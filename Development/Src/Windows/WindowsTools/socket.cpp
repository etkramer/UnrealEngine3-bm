/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "Socket.h"
#include <MSWSock.h>

FSocket::FSocket() : Socket(INVALID_SOCKET), bAssociatedWithIOCP(false)
{
	ZeroMemory(&Address, sizeof(Address));
	ZeroMemory(&ProtocolInfo, sizeof(ProtocolInfo));
}

FSocket::FSocket(ESocketFamily SocketFamily, ESocketType SocketType, ESocketProtocol SocketProtocol) : Socket(INVALID_SOCKET), bAssociatedWithIOCP(false)
{
	ZeroMemory(&Address, sizeof(Address));
	ZeroMemory(&ProtocolInfo, sizeof(ProtocolInfo));

	SetAttributes(SocketFamily, SocketType, SocketProtocol);
}

FSocket::~FSocket()
{
	Close();
}

/**
 * Associates the socket with an IO completion port.
 *
 * @param	IOCP	The IO completion port handle.
 * @return	True if the operation succeeds.
 */
bool FSocket::AssociateWithIOCP(HANDLE IOCP)
{
	if(IsValid() && IOCP && IOCP != INVALID_HANDLE_VALUE)
	{
		return bAssociatedWithIOCP = CreateIoCompletionPort((HANDLE)Socket, IOCP, NULL, 0) != NULL;
	}

	return false;
}

/**
 * Wrapper for the winsock recv().
 *
 * @param	Buffer		Buffer to save the recv'd data into.
 * @param	BufLength	The size of Buffer.
 * @param	Flags		Flags controlling the operation.
 */
int FSocket::Recv(char* Buffer, int BufLength, ESocketFlags Flags)
{
	return recv(Socket, Buffer, BufLength, (int)Flags);
}

/**
 * Wrapper for the winsock recvfrom().
 *
 * @param	Buffer		Buffer to save the recv'd data into.
 * @param	BufLength	The size of Buffer.
 * @param	FromAddress	The address the data was recv'd from.
 * @param	Flags		Flags controlling the operation.
 */
int FSocket::RecvFrom(char* Buffer, int BufLength, sockaddr_in &FromAddress, ESocketFlags Flags)
{
	int FromLen = sizeof(sockaddr_in);
	return recvfrom(Socket, Buffer, BufLength, (int)Flags, (sockaddr*)&FromAddress, &FromLen);
}

/**
 * Begins an asynchronous recv() operation.
 *
 * @param	Buffers		Array of buffers for the incoming data.
 * @param	BufferCount	The number of buffers.
 * @param	BytesRecvd	Receives the number of bytes written to the buffers.
 * @param	EventArgs	Information about the event.
 * @param	Flags		Flags controlling the operation.
 * @return	True if the operation succeeded.
 */
bool FSocket::RecvAsync(LPWSABUF Buffers, DWORD BufferCount, DWORD &BytesRecvd, WSAOVERLAPPED *EventArgs, ESocketFlags Flags)
{
	bool ret = WSARecv(Socket, Buffers, BufferCount, &BytesRecvd, (LPDWORD)&Flags, EventArgs, NULL) != SOCKET_ERROR;
	
	if(!ret && WSAGetLastError() == WSA_IO_PENDING)
	{
		ret = true;
	}

	return ret;
}

/**
 * Wrapper for the winsock send().
 *
 * @param	Buffer		Buffer to save the recv'd data into.
 * @param	BufLength	The size of Buffer.
 * @param	Flags		Flags controlling the operation.
 * @return	SOCKET_ERROR if the operation failed, otherwise the number of bytes sent.
 */
int FSocket::Send(char* Buffer, int BufLength, ESocketFlags Flags)
{
	return send(Socket, Buffer, BufLength, (int)Flags);
}

/**
 * Wrapper for the winsock sendto().
 *
 * @param	Buffer		Buffer to save the recv'd data into.
 * @param	BufLength	The size of Buffer.
 * @param	Flags		Flags controlling the operation.
 * @return	SOCKET_ERROR if the operation failed, otherwise the number of bytes sent.
 */
int FSocket::SendTo(char* Buffer, int BufLength, ESocketFlags Flags)
{
	return sendto(Socket, Buffer, BufLength, (int)Flags, (sockaddr*)&Address, sizeof(Address));
}

/**
 * Creates a new internal socket.
 *
 * @return	True if the operation succeeds.
 */
bool FSocket::CreateSocket()
{
	Close();

	return (Socket = WSASocket(ProtocolInfo.iAddressFamily, ProtocolInfo.iSocketType, ProtocolInfo.iProtocol, NULL, NULL, WSA_FLAG_OVERLAPPED)) != INVALID_SOCKET;
}

/**
 * Sets attributes used when creating a new socket via CreateSocket().
 *
 * @param	SocketFamily	The family of the socket.
 * @param	SocketType		The type of socket to be created.
 * @param	SocketProtocol	The protocol of the socket.
 */
void FSocket::SetAttributes(ESocketFamily SocketFamily, ESocketType SocketType, ESocketProtocol SocketProtocol)
{
	ProtocolInfo.iAddressFamily = (int)SocketFamily;
	ProtocolInfo.iSocketType = (int)SocketType;
	ProtocolInfo.iProtocol = (int)SocketProtocol;
	Address.sin_family = (short)SocketFamily;
}

/**
 * Binds the socket to the current address.
 *
 * @return	True if the operation succeeds.
 */
bool FSocket::Bind()
{
	return bind(Socket, (sockaddr*)&Address, sizeof(Address)) != SOCKET_ERROR;
}

/**
 * Wraps the winsock connect() by connecting to the current address.
 *
 * @return	True if the operation succeeds.
 */
bool FSocket::Connect()
{
	return connect(Socket, (sockaddr*)&Address, sizeof(Address)) == 0;
}

/**
 * Sets the blocking state of the socket.
 *
 * @param	IsNonBlocking	True if the socket is set to non-blocking.
 * @return	True if the operation succeeds.
 */
bool FSocket::SetNonBlocking(bool IsNonBlocking)
{
	u_long arg = IsNonBlocking ? 1 : 0;
	return ioctlsocket(Socket, FIONBIO, &arg) == 0;
}

/**
 * Sets the broadcasting state of the socket.
 *
 * @param	bEnable	True if the socket is set to broadcast.
 * @return	True if the operation succeeds.
 */
bool FSocket::SetBroadcasting(bool bEnable)
{
	static const unsigned int BroadcastIP = inet_addr("255.255.255.255");

	if(bEnable)
	{
		Address.sin_addr.s_addr = BroadcastIP;
	}

	INT bEnableBroadcast = bEnable ? TRUE : FALSE;
	return setsockopt(Socket, SOL_SOCKET, SO_BROADCAST, (const char*)&bEnableBroadcast, sizeof(bEnableBroadcast)) == 0;
}

/**
 * Closes the socket.
 */
void FSocket::Close()
{
	if(IsValid())
	{
		closesocket(Socket);
		Socket = INVALID_SOCKET;
	}

	bAssociatedWithIOCP = false;
}
