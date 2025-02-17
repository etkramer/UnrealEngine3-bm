/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "NetworkManager.h"
#include "..\..\IpDrv\Inc\DebugServerDefs.h"
#include "..\..\IpDrv\Inc\GameType.h"
#include "ByteStreamConverter.h"
#include "StringUtils.h"

#define WORKER_THREAD_HEURISTIC 2

extern void DebugOutput(const char* Buffer);

/**Returns platform type name for enum value. */
static const char* ToString(EPlatformType type)
{
	switch (type)
	{
	case EPlatformType_Windows:
		{
			return "Windows";
		}
	case EPlatformType_Xenon:
		{
			return "Xenon";
		}
	case EPlatformType_PS3:
		{
			return "PS3";
		}
	case EPlatformType_Linux:
		{
			return "Linux";
		}
	/*case EPlatformType_Mac:
		{
			return "Mac";
		}*/
	}
	return "Unknown";
}

FNetworkManager::FNetworkManager() : IOCompletionPort(INVALID_HANDLE_VALUE), bInitialized(false), TTYCallback(NULL)
{
	GetSystemInfo(&SysInfo);
	WorkerThreads.resize(SysInfo.dwNumberOfProcessors * WORKER_THREAD_HEURISTIC);
	ZeroMemory(&WorkerThreads[0], sizeof(HANDLE) * WorkerThreads.size());
}

FNetworkManager::~FNetworkManager()
{
	Cleanup();
}

/**
 * Initalizes winsock and the FNetworkManager instance.
 */
void FNetworkManager::Initialize()
{
	if(bInitialized)
	{
		return;
	}

	DebugOutput("Creating shutdown event...\n");
	ThreadCleanupEvent = CreateEvent(NULL, TRUE, FALSE, "FNetworkManager::ThreadCleanupEvent");
	if(!ThreadCleanupEvent)
	{
		DebugOutput("Creating shutdown event failed.\n");
		return;
	}

	DebugOutput("Shutdown event created.\n");

	DebugOutput("Initializing Network Manager...\n");

	// Init Windows Sockets
	WSADATA WSAData;
	WORD WSAVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(WSAVersionRequested, &WSAData) != 0)
	{
		return;
	}

	DebugOutput("WSA Sockets Initialized.\n");

	DebugOutput("Creating IO Completion Port...\n");

	// By providing 0 to the last argument a IOCP thread is created for every processor on the machine
	IOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if(!IOCompletionPort)
	{
		DebugOutput("Creating IO Completion Port Failed.\n");
		WSACleanup();
		return;
	}

	DebugOutput("IO Completion Port Created.\n");

	DebugOutput("Creating IO Worker Threads...\n");

	size_t NumThreadsCreated = 0;
	for(size_t ThreadIndex = 0; ThreadIndex < WorkerThreads.size(); ++ThreadIndex, ++NumThreadsCreated)
	{
		WorkerThreads[ThreadIndex] = (HANDLE)_beginthreadex(NULL, 0, WorkerThreadProc, this, 0, NULL);
		if(!WorkerThreads[ThreadIndex])
		{
			break;
		}
	}

	if(NumThreadsCreated != WorkerThreads.size())
	{
		DebugOutput("Creating IO Worker Threads Failed.\n");
		
		Cleanup();

		return;
	}

	DebugOutput("IO Worker Threads Created.\n");

	// Init broadcaster
	Broadcaster.SetPort(DefaultDebugChannelReceivePort);
	Broadcaster.SetAttributes(SF_IPv4, ST_Datagram, SP_UDP);
	if(!Broadcaster.CreateSocket())
	{
		DebugOutput("Failed to create broadcast socket.\n");
		return;
	}
	Broadcaster.SetBroadcasting(true);
}

/**
 * Cleans up winsock and all of the resources allocated by the FNetworkManager instance.
 */
void FNetworkManager::Cleanup()
{
	DebugOutput("Cleaning up Network Manager...\n");

	// Shut down all of the sockets and let the IOCP worker threads remove the active ones themselves
	TargetsLock.Lock();
	{
		for(TargetMap::iterator Iter = Targets.begin(); Iter != Targets.end(); ++Iter)
		{
			TargetPtr Target = (*Iter).second;
			Target->TCPClient.Close();
			Target->UDPClient.Close();
		}
	}
	TargetsLock.Unlock();

	// Close broadcaster
	Broadcaster.Close();

	// Close 'server announce' listener
	ServerResponseListener.Close();

	SetEvent(ThreadCleanupEvent);

	WaitForMultipleObjects((DWORD)WorkerThreads.size(), &WorkerThreads[0], TRUE, 5000);

	for(size_t ThreadIndex = 0; ThreadIndex < WorkerThreads.size(); ++ThreadIndex)
	{
		CloseHandle(WorkerThreads[ThreadIndex]);
		WorkerThreads[ThreadIndex] = 0;
	}

	CloseHandle(ThreadCleanupEvent);
	CloseHandle(IOCompletionPort);

	WSACleanup();

	// Clean up any remaining targets that were not on an IOCP worker thread
	TargetsLock.Lock();
	{
		Targets.clear();
	}
	TargetsLock.Unlock();

	DebugOutput("Finished cleaning up Network Manager.\n");
	bInitialized = false;
}

/**
 * Retrieves a target with the specified IP Address.
 *
 * @param	Address		The address of the target to retrieve.
 * @return	NULL if the target could not be found, otherwise a valid reference pointer.
 */
TargetPtr FNetworkManager::GetTarget(const sockaddr_in &Address)
{
	TargetPtr Ret;

	TargetsLock.Lock();
	for(TargetMap::iterator Iter = Targets.begin(); Iter != Targets.end(); ++Iter)
	{
		if(memcmp(&(*Iter).second->RemoteAddress, &Address, sizeof(sockaddr_in)) == 0)
		{
			Ret = (*Iter).second;
			break;
		}
	}
	TargetsLock.Unlock();

	return Ret;
}

/**
 * Retrieves a target with the specified IP Address.
 *
 * @param	Handle		The handle of the target to retrieve.
 * @return	NULL if the target could not be found, otherwise a valid reference pointer.
 */
TargetPtr FNetworkManager::GetTarget(const TARGETHANDLE Handle)
{
	TargetsLock.Lock();
	TargetMap::iterator Iter = Targets.find(Handle);
	TargetsLock.Unlock();

	if(Iter != Targets.end())
	{
		return (*Iter).second;
	}

	return TargetPtr();
}

/**
 * Adds a target to the list of targets.
 *
 * @param	Address		The address of the target being added.
 * @return	A reference pointer to the new target.
 */
TargetPtr FNetworkManager::AddTarget(const sockaddr_in& Address)
{
	TargetPtr Ret;

	TargetsLock.Lock();
	{
		Ret = GetTarget(Address);

		if(!Ret)
		{
			Ret = new CWindowsTarget();
			Ret->RemoteAddress = Address;

			Targets[Ret.GetHandle()] = Ret;
		}
		else
		{
			if(!Ret->UDPClient.IsValid())
			{
				Ret->UDPClient.CreateSocket();
			}

			if(!Ret->TCPClient.IsValid())
			{
				Ret->TCPClient.CreateSocket();
			}
		}
	}
	TargetsLock.Unlock();

	return Ret;
}

/**
 * Removes the target with the specified address.
 *
 * @param	Handle		The handle of the target to be removed.
 */
void FNetworkManager::RemoveTarget(TARGETHANDLE Handle)
{
	TargetsLock.Lock();
	{
		TargetMap::iterator Iter = Targets.find(Handle);

		if(Iter != Targets.end())
		{
			TargetPtr Target = (*Iter).second;

			Targets.erase(Iter);

			Target->UDPClient.Close();
			Target->TCPClient.Close();
			Target->bConnected = false;
		}
	}
	TargetsLock.Unlock();
}

/**
 * Triggers the TTY callback for outputting a message.
 *
 * @param	Data	The message to be output.
 * @param	Length	The size in bytes of the buffer pointed to by Data.
 * @param	Target	The target to send the TTY text to.
 */
void FNetworkManager::NotifyTTYCallback(TargetPtr &Target, const char *Txt)
{
	if(Target)
	{
		wchar_t Buffer[1024];
		swprintf_s(Buffer, 1024, L"%S", Txt);

		Target->SendTTY(Buffer);
	}
}

/**
 * Triggers the TTY callback for outputting a message.
 *
 * @param	Channel	The channel the message is occuring on.
 * @param	Text	The message to be displayed.
 * @param	Target	The target to send the TTY text to.
 */
void FNetworkManager::NotifyTTYCallback(TargetPtr &Target, const string &Channel, const string &Text)
{
	wchar_t Buffer[1024];
	swprintf_s(Buffer, 1024, L"%S: %S", Channel.c_str(), Text.c_str());

	if(Target)
	{
		Target->SendTTY(Buffer);
	}
}

/**
 *	Sends message using given UDP client.
 *	@param Handle			client to send message to
 *	@param MessageType		sent message type
 *	@param Message			actual text of the message
 *	@return true on success, false otherwise
 */
bool FNetworkManager::SendToConsole(const TARGETHANDLE Handle, const EDebugServerMessageType MessageType, const string& Message)
{
	TargetPtr Target = GetTarget(Handle);

	if(Target)
	{
		return SendToConsole(Target->UDPClient, MessageType, Message);
	}

	return false;
}

/**
 *	Attempts to determine available targets.
 */
void FNetworkManager::DetermineTargets()
{
	// Init 'server announce' listener
	ServerResponseListener.SetPort(DefaultDebugChannelSendPort);
	ServerResponseListener.SetAttributes(SF_IPv4, ST_Datagram, SP_UDP);
	if(!ServerResponseListener.CreateSocket() ||
		!ServerResponseListener.SetNonBlocking(true) ||
		!ServerResponseListener.Bind())
	{
		DebugOutput("Failed to create server response receiver.\n");
		return;
	}

	DebugOutput("Determining targets...\n");

	// Broadcast 'server announce' request.
	// Send three requests in case packets are dropped.
	bool bServerAnnounceResult = SendToConsole(Broadcaster, EDebugServerMessageType_ServerAnnounce, "");
	bServerAnnounceResult = SendToConsole(Broadcaster, EDebugServerMessageType_ServerAnnounce, "") || bServerAnnounceResult;
	bServerAnnounceResult = SendToConsole(Broadcaster, EDebugServerMessageType_ServerAnnounce, "") || bServerAnnounceResult;

	if(!bServerAnnounceResult)
	{
		return;
	}

	DebugOutput("Sent broadcast message...\n");

	// Give the servers some time to respond
	Sleep(100);

	// Attempt to receive 'server response'
	ReceiveMessages(ServerResponseListener, 50);

	RemoveTimeExpiredTargets();

	ServerResponseListener.Close();
}

/**
* Gets the default target.
*/
TargetPtr FNetworkManager::GetDefaultTarget()
{
	TargetPtr Ret;

	TargetsLock.Lock();
	TargetMap::iterator Iter = Targets.begin();

	if(Iter != Targets.end())
	{
		Ret = (*Iter).second;
	}

	TargetsLock.Unlock();

	return Ret;
}

/**
* Retrieves a handle to each available target.
*
* @param	OutTargetList			An array to copy all of the target handles into.
* @param	InOutTargetListSize		This variable needs to contain the size of OutTargetList. When the function returns it will contain the number of target handles copied into OutTargetList.
*/
void FNetworkManager::GetTargets(TARGETHANDLE *OutTargetList, int *InOutTargetListSize)
{
	TargetsLock.Lock();

	int TargetsCopied = 0;
	for(TargetMap::iterator Iter = Targets.begin(); Iter != Targets.end() && TargetsCopied < *InOutTargetListSize; ++Iter, ++TargetsCopied)
	{
		OutTargetList[TargetsCopied] = (*Iter).first;
	}

	TargetsLock.Unlock();

	*InOutTargetListSize = TargetsCopied;
}

/**
 * Removes the targets that have not been active for a period of time.
 */
void FNetworkManager::RemoveTimeExpiredTargets()
{
	// Invalidate targets older than fixed no. seconds
	__time64_t CurrentTime;
	_time64(&CurrentTime);

	TargetsLock.Lock();
	{
		vector<TARGETHANDLE> TargetsToRemove;

		for(TargetMap::iterator Iter = Targets.begin(); Iter != Targets.end(); ++Iter)
		{
			if(!(*Iter).second->bConnected && CurrentTime - (*Iter).second->TimeRegistered > 10)
			{
				TargetsToRemove.push_back((*Iter).first);
			}
		}

		for(size_t Index = 0; Index < TargetsToRemove.size(); ++Index)
		{
			TargetMap::iterator Iter = Targets.find(TargetsToRemove[Index]);
			TargetPtr Target = (*Iter).second;

			Targets.erase(Iter);

			Target->TCPClient.Close();
			Target->UDPClient.Close();
		}
	}
	TargetsLock.Unlock();
}

/**
 *	Attempts to receive (and dispatch) messages from given client.
 *	@param Client client to attempt to receive message from
 *	@param AttemptsCount no. attempts (between each fixed miliseconds waiting happens)
 */
void FNetworkManager::ReceiveMessages(FSocket& UDPClient, int AttemptsCount)
{
	while(AttemptsCount > 0)
	{
		// Attempt to receive message
		char* Data = NULL;
		int DataSize = 0;
		EDebugServerMessageType MessageType;
		sockaddr_in ServerAddress;

		if(!ReceiveFromConsole(UDPClient, MessageType, Data, DataSize, ServerAddress))
		{
			// See if we should stop attempting to receive messages
			AttemptsCount--;
			if(AttemptsCount == 0)
			{
				return;
			}

			// Try again
			Sleep(10);
			continue;
		}

		int CurDataSize = DataSize;

		// Handle each supported message type
		switch(MessageType)
		{
		case EDebugServerMessageType_ServerResponse:
			{
				// See if target already exists
				TargetPtr Target = GetTarget(ServerAddress);

				// Add target
				if(!Target)
				{
					Target = AddTarget(ServerAddress);
				}

				// Set up target info
				Target->Configuration = L"Unknown configuration";
				Target->ComputerName = ToWString(CByteStreamConverter::LoadStringBE(Data, CurDataSize));

				if(Target->ComputerName.length() == 0)
				{
					DebugOutput("Corrupt packet received\n");
					continue;
				}

				Target->GameName = ToWString(CByteStreamConverter::LoadStringBE(Data, CurDataSize));
				Target->GameTypeName = ToWString(ToString(Target->GameType));
				Target->PlatformType = (EPlatformType) *(Data++);
				Target->ListenPortNo = CByteStreamConverter::LoadIntBE(Data, CurDataSize);
				_time64(&Target->TimeRegistered);

				// Set up target connection
				Target->UDPClient.SetAddress(ServerAddress.sin_addr.s_addr);
				Target->UDPClient.SetPort(DefaultDebugChannelReceivePort);

				Target->TCPClient.SetAddress(ServerAddress.sin_addr.s_addr);
				Target->TCPClient.SetPort(DefaultDebugChannelListenPort);

				Target->UpdateName();

				break;
			}

		case EDebugServerMessageType_ServerDisconnect:
			{
				// Figure out target
				TargetPtr Target = GetTarget(ServerAddress);
				if(!Target)
				{
					DebugOutput("Received 'server disconnect' from unknown server.\n");
					break;
				}

				// Disconnect from server
				Target->UDPClient.Close();
				Target->TCPClient.Close();

				break;
			}

		case EDebugServerMessageType_ServerTransmission:
			{
				DebugOutput("Server TEXT transmission.\n");

				// Figure out target
				TargetPtr Target = GetTarget(ServerAddress);
				if(!Target)
				{
					DebugOutput("Received TEXT transmission from unknown server.\n");
					break;
				}

				// Retrieve channel and text
				const int ChannelEnum = CByteStreamConverter::LoadIntBE(Data, CurDataSize);
				string Channel;

				// Enums mirrored from FDebugServer!
				if(ChannelEnum == 0)
				{
					Channel="DEBUG";
				}
				else if(ChannelEnum == 1)
				{
					Channel="REMOTE";
				}
				else if(ChannelEnum == 2)
				{
					Channel="MEM";
				}
				else
				{
					Channel="UNKNOWN";
				}

				const string Text = CByteStreamConverter::LoadStringBE(Data, CurDataSize);

				DebugOutput("Server TEXT transmission decoded.\n");
				DebugOutput("Channel: ");
				DebugOutput(Channel.c_str());
				DebugOutput("\n");
				DebugOutput("Text: ");
				DebugOutput(Text.c_str());
				DebugOutput("\n");

				NotifyTTYCallback(Target, Channel, Text);
				break;
			}

		default:
			{
				DebugOutput("Some server sent message of unknown type.\n");
			}
		}
	}
}

/**
 *	Attempts to receive message from given UDP client.
 *	@param Client client to receive message from
 *	@param MessageType received message type
 *	@param Data received message data
 *	@param DataSize received message size
 *	@return true on success, false otherwise
 */
bool FNetworkManager::ReceiveFromConsole(FSocket& Client, EDebugServerMessageType& MessageType, char*& Data, int& DataSize, sockaddr_in& SenderAddress)
{
	// Attempt to receive bytes
#define MAX_BUFFER_SIZE (1 << 12)
	char Buffer[MAX_BUFFER_SIZE];
	const int NumReceivedBytes = Client.RecvFrom(Buffer, MAX_BUFFER_SIZE, SenderAddress);

	// The message should contain at least 2 characters identyfing message type
	if (NumReceivedBytes < 2)
	{
		return false;
	}

	// Determine message type
	char MessageTypeString[3];
	sprintf_s(MessageTypeString, 3, "%c%c", Buffer[0], Buffer[1]);
	MessageType = ToDebugServerMessageType(MessageTypeString);

	DebugOutput("Received message of type: ");
	DebugOutput(MessageTypeString);
	DebugOutput("\n");

	// Determine message content
	// Skip the two bytes that pad out the message header.
	DataSize = NumReceivedBytes - 2;
	Data = new char[DataSize];
	memcpy(Data, Buffer + 2, DataSize);

	return true;
}

/**
 *	Sends message using given UDP client.
 *	@param Client client to send message to
 *	@param MessageType sent message type
 *	@param Message actual text of the message
 *	@return true on success, false otherwise
 */
bool FNetworkManager::SendToConsole(FSocket& Client, const EDebugServerMessageType MessageType, const string& Message)
{
	// Compose message to be sent (header, followed by actual message)
	const int BufferLength = DebugServerMessageTypeNameLength + (Message.length() > 0 ? (sizeof(int) + (int) Message.length()) : 0);
	char* Buffer = new char[BufferLength];

	memcpy(Buffer, ToString(MessageType), DebugServerMessageTypeNameLength);
	if (Message.length() > 0)
	{
		CByteStreamConverter::StoreIntBE(Buffer + DebugServerMessageTypeNameLength, (int) Message.length());
		memcpy(Buffer + DebugServerMessageTypeNameLength + sizeof(int), Message.c_str(), Message.length());
	}

	// Send the message
	const int NumSentBytes = Client.SendTo(Buffer, BufferLength);

	// Clean up
	delete[] Buffer;

	return NumSentBytes == BufferLength;
}

/**
 * Connects to the target with the specified name.
 *
 * @param	TargetName		The name of the target to connect to.
 * @return	Handle to the target that was connected or else INVALID_TARGETHANDLE.
 */
TARGETHANDLE FNetworkManager::ConnectToTarget(const wchar_t* TargetName)
{
	// Find the target
	TargetPtr CurrentTarget;

	for(TargetMap::iterator Iter = Targets.begin(); Iter != Targets.end(); ++Iter)
	{
		if((*Iter).second->Name == TargetName)
		{
			CurrentTarget = (*Iter).second;
			break;
		}
	}

	if(ConnectToTarget(CurrentTarget))
	{
		CurrentTarget.GetHandle();
	}

	return INVALID_TARGETHANDLE;
}

/**
 * Connects to the target with the specified handle.
 *
 * @param	Handle		The handle of the target to connect to.
 */
bool FNetworkManager::ConnectToTarget(TARGETHANDLE Handle)
{
	TargetPtr Target = GetTarget(Handle);

	return ConnectToTarget(Target);
}

/**
 * Connects to the target.
 *
 * @param	Handle		The handle of the target to connect to.
 */
bool FNetworkManager::ConnectToTarget(TargetPtr &Target)
{
	// Target not found
	if(!Target)
	{
		return false;
	}

	if(!Target->UDPClient.IsValid())
	{
		// Create the socket used to send messages to server
		if(!Target->UDPClient.CreateSocket())
		{
			return false;
		}

		// Inform server about connection
		const bool connectionRequestResult = SendToConsole(Target->UDPClient, EDebugServerMessageType_ClientConnect);
		if(!connectionRequestResult)
		{
			Target->UDPClient.Close();
			return false;
		}
	}


	if(!Target->TCPClient.IsValid() || !Target->TCPClient.IsAssociatedWithIOCP())
	{
		if(!Target->TCPClient.CreateSocket())
		{
			return false;
		}

		if(!Target->TCPClient.AssociateWithIOCP(IOCompletionPort))
		{
			return false;
		}

		//Target->TCPClient.SetNonBlocking(true);
		if(!Target->TCPClient.Connect())
		{
			return false;
		}

		OverlappedEventArgs *Args = new OverlappedEventArgs(OVT_Recv);
		Args->Owner = Target;

		DWORD BytesRecvd = 0;
		if(!Target->TCPClient.RecvAsync(&Args->WSABuffer, 1, BytesRecvd, Args))
		{
			return false;
		}
	}

	// Add initial info for the user
	const unsigned int IP = Target->UDPClient.GetIP();
	NotifyTTYCallback(Target, "CONNECTION",
		string("Connected to server:") +
		"\n\tIP: " + inet_ntoa(*(in_addr*) &IP) +
		"\n\tComputer name: " + ToString(Target->ComputerName) +
		"\n\tGame name: " + ToString(Target->GameName) +
		"\n\tGame type: " + ToString((EGameType)Target->GameType) +
		"\n\tPlatform: " + ToString(Target->PlatformType) + "\n");

	Target->bConnected = true;

	return true;
}

/**
 * Returns the number of targets available.
 */
int FNetworkManager::GetNumberOfTargets()
{
	int Num = 0;

	TargetsLock.Lock();
	{
		Num = (int)Targets.size();
	}
	TargetsLock.Unlock();

	return Num;
}

/**
 * Exists for compatability with UnrealConsole. Index is disregarded and CurrentTarget is disconnected if it contains a valid pointer.
 *
 * @param	Handle		Handle to the target to disconnect.
 */
void FNetworkManager::DisconnectTarget(const TARGETHANDLE Handle)
{
	TargetPtr Target = GetTarget(Handle);

	if(Target)
	{
		Target->bConnected = false;

		// Inform server about disconnection
		// Send three messages in case packets are dropped.
		SendToConsole(Target->UDPClient, EDebugServerMessageType_ClientDisconnect);
		SendToConsole(Target->UDPClient, EDebugServerMessageType_ClientDisconnect);
		SendToConsole(Target->UDPClient, EDebugServerMessageType_ClientDisconnect);

		RemoveTarget(Handle);
	}
}

/**
 * Handles a packet.
 *
 * @param	Data				The packet data.
 * @param	BytesRecv			The size of the packet data.
 * @param	Target				The target that received the packet.
 */
void FNetworkManager::HandlePacket(char* Data, const int BytesRecv, TargetPtr& Target)
{
	bool bAllocated = false;
	char *Buf = Data;
	int ActualBufSize = BytesRecv;

	if(Target->PartialPacketBuffer && Target->PartialPacketBufferSize > 0)
	{
		bAllocated = true;
		ActualBufSize = Target->PartialPacketBufferSize + BytesRecv;
		Buf = new char[ActualBufSize];
		memcpy_s(Buf, ActualBufSize, Target->PartialPacketBuffer, Target->PartialPacketBufferSize);
		memcpy_s(&Buf[Target->PartialPacketBufferSize], ActualBufSize - Target->PartialPacketBufferSize, Data, BytesRecv);

		delete [] Target->PartialPacketBuffer;
		Target->PartialPacketBuffer = NULL;
		Target->PartialPacketBufferSize = 0;
	}

	int ChannelEnum = 0;
	string Text;
	string ChannelText;
	int BytesConsumed = 0;
	char *BufPtr = Buf;

	while(CheckForCompleteMessage(BufPtr, ActualBufSize, ChannelEnum, Text, BytesConsumed))
	{
		BufPtr += BytesConsumed;
		ActualBufSize -= BytesConsumed;

		if(Target->bConnected)
		{
			switch(ChannelEnum)
			{
			case 0:
				{
					ChannelText = "DEBUG";
					break;
				}
			case 1:
				{
					ChannelText = "REMOTE";
					break;
				}
			case 2:
				{
					ChannelText = "MEM";
					break;
				}
			default:
				{
					ChannelText = "UNKNOWN";
					break;
				}
			}

			NotifyTTYCallback(Target, ChannelText, Text);
		}
	}

	if(ActualBufSize > 0)
	{
		Target->PartialPacketBuffer = new char[ActualBufSize];
		Target->PartialPacketBufferSize = ActualBufSize;

		memcpy_s(Target->PartialPacketBuffer, Target->PartialPacketBufferSize, BufPtr, ActualBufSize);
	}

	if(bAllocated)
	{
		delete [] Buf;
	}
}

/**
 * Parses packet data for a message.
 *
 * @param	TCPData				The packet data.
 * @param	TCPDataSize			The size of the packet data.
 * @param	ChannelEnum			Receives the channel the packet is operating on.
 * @param	Text				Receives the text of the message.
 * @param	NumBytesConsumed	The number of bytes read.
 */
bool FNetworkManager::CheckForCompleteMessage(char* TCPData, const int TCPDataSize, int& ChannelEnum, string& Text, int& NumBytesConsumed) const
{
	Text.clear();
	NumBytesConsumed = 0;

	// We've stopped receiving data for now.
	// Have a look at the buffered data and see if there's enough to start parsing.
	const int PacketHeaderSize =
		//2							// Message header padding
		+ sizeof(char) * 2		// Message header
		+ 1//4							// Channel enum
		+ sizeof(int);				// TextLength

	if(TCPDataSize < PacketHeaderSize)
	{
		return false;
	}

	// Check that the message type is a 'server transmission'.
	char MessageTypeString[3];
	sprintf_s(MessageTypeString, 3, "%c%c", TCPData[0], TCPData[1]);
	const EDebugServerMessageType MessageType = ToDebugServerMessageType(MessageTypeString);
	if(MessageType != EDebugServerMessageType_ServerTransmission)
	{
		DebugOutput("UNKNOWN MESSAGE TYPE\n");
		return false;
	}

	// Retrieve the channel enum.
	ChannelEnum = (int)TCPData[2];

	// Copy the data over to a temp buffer, skipping the first HEADER_OFFSET bytes.
	const int HEADER_OFFSET = 3;//4;
	char* Data = &TCPData[HEADER_OFFSET];

	int CurDataSize = TCPDataSize - HEADER_OFFSET;

	// Retrieve the channel enum.
	//ChannelEnum = CByteStreamConverter::LoadIntBE(Data);
	// Retrieve the text buffer.  This will advance the Data pointer by sizeof(int).
	const int StringLen = CByteStreamConverter::LoadIntBE(Data, CurDataSize);

	// At this point, we're at the end of the header.  See if the entire text buffer has been received.
	if(TCPDataSize - PacketHeaderSize < StringLen)
	{
		return false;
	}

	Text.assign(Data, StringLen);
	NumBytesConsumed = PacketHeaderSize + StringLen;

	assert((int)Text.length() == StringLen);
	assert(NumBytesConsumed <= TCPDataSize);

	return true;
}

/**
 * The callback for IOCP worker threads. Handles IO events for the targets.
 *
 * @param	Data	A pointer to the owning FNetworkManager instance.
 */
unsigned int __stdcall FNetworkManager::WorkerThreadProc(void *Data)
{
	FNetworkManager *Mgr = (FNetworkManager*)Data;

	ULONG_PTR CompletionKey = NULL;
	OverlappedEventArgs *Overlapped = NULL;
	DWORD BytesTransferred  = 0;

	while(WaitForSingleObject(Mgr->ThreadCleanupEvent, 0) != WAIT_OBJECT_0)
	{
		BOOL bReturn = GetQueuedCompletionStatus(Mgr->IOCompletionPort, &BytesTransferred, &CompletionKey, (LPOVERLAPPED*)&Overlapped, 100);

		if(bReturn)
		{
			if(BytesTransferred > 0)
			{
				Mgr->HandlePacket(Overlapped->Buffer, BytesTransferred, Overlapped->Owner);
			}

			ZeroMemory(Overlapped, sizeof(OVERLAPPED));
			if(!Overlapped->Owner->TCPClient.RecvAsync(&Overlapped->WSABuffer, 1, BytesTransferred, Overlapped))
			{
				Mgr->NotifyTTYCallback(Overlapped->Owner, "Error", "Could not being an asynchronous recv on the current target!");
			}
		}
		else if(Overlapped)
		{
			Mgr->RemoveTarget(Overlapped->Owner.GetHandle());
			delete Overlapped;
		}

		Overlapped = NULL;
	}

	return 0;
}
