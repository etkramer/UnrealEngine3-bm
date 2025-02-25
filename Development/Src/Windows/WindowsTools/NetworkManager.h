/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#ifndef _NETWORKMANAGER_H_
#define _NETWORKMANAGER_H_

#include <map>
#include "Common.h"
#include "CriticalSection.h"
#include "Socket.h"
#include "WindowsTarget.h"

//forward declarations
enum EDebugServerMessageType;

typedef FReferenceCountPtr<CWindowsTarget> TargetPtr;
typedef map<TARGETHANDLE, TargetPtr> TargetMap;

/**
 * This class contains all of the network code for interacting with windows targets.
 */
class FNetworkManager
{
private:
	/** Handle to the IO completion port. */
	HANDLE IOCompletionPort;
	/** Handle for triggering the cleanup event for IOCP worker threads. */
	HANDLE ThreadCleanupEvent;
	/** True if the FNetworkManager instance has been initialized. */
	bool bInitialized;
	/** Information about the system. */
	SYSTEM_INFO SysInfo;
	/** The callback for TTY notifications. */
	TTYEventCallbackPtr TTYCallback;
	/** A map of targets. */
	TargetMap Targets;
	/** Synch object for accessing the target map/list. */
	FCriticalSection TargetsLock;
	/** Array of handles to IOCP worker threads. */
	vector<HANDLE> WorkerThreads;

	/** Used to broadcast 'server announce' message. */
	FSocket Broadcaster;
	/** Used to receive 'server response' from server. */
	FSocket ServerResponseListener;

	/**
	 * The callback for IOCP worker threads. Handles IO events for the targets.
	 *
	 * @param	Data	A pointer to the owning FNetworkManager instance.
	 */
	static unsigned int __stdcall WorkerThreadProc(void *Data);

	/**
	 * Adds a target to the list of targets.
	 *
	 * @param	Address		The address of the target being added.
	 * @return	A reference pointer to the new target.
	 */
	TargetPtr AddTarget(const sockaddr_in& Address);

	/**
	 * Removes the target with the specified address.
	 *
	 * @param	Handle		The handle of the target to be removed.
	 */
	void RemoveTarget(TARGETHANDLE Handle);

	/**
	 * Removes the targets that have not been active for a period of time.
	 */
	void RemoveTimeExpiredTargets();

	/**
	 * Triggers the TTY callback for outputting a message.
	 *
	 * @param	Data	The message to be output.
	 * @param	Length	The size in bytes of the buffer pointed to by Data.
	 * @param	Target	The target to send the TTY text to.
	 */
	void NotifyTTYCallback(TargetPtr &Target, const char *Txt);

	/**
	 * Triggers the TTY callback for outputting a message.
	 *
	 * @param	Channel	The channel the message is occuring on.
	 * @param	Text	The message to be displayed.
	 * @param	Target	The target to send the TTY text to.
	 */
	void NotifyTTYCallback(TargetPtr &Target, const string &Channel, const string &Text);

	/**
	 * Handles a packet.
	 *
	 * @param	Data				The packet data.
	 * @param	BytesRecv			The size of the packet data.
	 * @param	Target				The target that received the packet.
	 */
	void HandlePacket(char* Data, const int BytesRecv, TargetPtr& Target);

	/**
	 * Parses packet data for a message.
	 *
	 * @param	TCPData				The packet data.
	 * @param	TCPDataSize			The size of the packet data.
	 * @param	ChannelEnum			Receives the channel the packet is operating on.
	 * @param	Text				Receives the text of the message.
	 * @param	NumBytesConsumed	The number of bytes read.
	 */
	bool CheckForCompleteMessage(char* TCPData, const int TCPDataSize, int& ChannelEnum, string& Text, int& NumBytesConsumed) const;

	/**
	*	Attempts to receive (and dispatch) messages from given client.
	*	@param Client client to attempt to receive message from
	*	@param AttemptsCount no. attempts (between each fixed miliseconds waiting happens)
	*/
	void ReceiveMessages(FSocket& UDPClient, int AttemptsCount);

	/**
	*	Attempts to receive message from given UDP client.
	*	@param Client client to receive message from
	*	@param MessageType received message type
	*	@param Data received message data
	*	@param DataSize received message size
	*	@return true on success, false otherwise
	*/
	bool ReceiveFromConsole(FSocket& Client, EDebugServerMessageType& MessageType, char*& Data, int& DataSize, sockaddr_in& SenderAddress);

	/**
	*	Sends message using given UDP client.
	*	@param Client client to send message to
	*	@param MessageType sent message type
	*	@param Message actual text of the message
	*	@return true on success, false otherwise
	*/
	bool SendToConsole(FSocket& Client, const EDebugServerMessageType MessageType, const string& Message = "");

	/**
	 * Connects to the target.
	 *
	 * @param	Handle		The handle of the target to connect to.
	 */
	bool ConnectToTarget(TargetPtr &Target);

public:
	FNetworkManager();
	~FNetworkManager();

	/**
	 * Initalizes winsock and the FNetworkManager instance.
	 */
	void Initialize();

	/**
	 * Cleans up winsock and all of the resources allocated by the FNetworkManager instance.
	 */
	void Cleanup();

	/**
	 * Retrieves a target with the specified IP Address.
	 *
	 * @param	Address		The address of the target to retrieve.
	 * @return	NULL if the target could not be found, otherwise a valid reference pointer.
	 */
	TargetPtr GetTarget(const sockaddr_in &Address);

	/**
	 * Retrieves a target with the specified IP Address.
	 *
	 * @param	Handle		The handle of the target to retrieve.
	 * @return	NULL if the target could not be found, otherwise a valid reference pointer.
	 */
	TargetPtr GetTarget(const TARGETHANDLE Handle);

	/**
	 * Connects to the target with the specified name.
	 *
	 * @param	TargetName		The name of the target to connect to.
	 * @return	Handle to the target that was connected or else INVALID_TARGETHANDLE.
	 */
	TARGETHANDLE ConnectToTarget(const wchar_t* TargetName);

	/**
	 * Connects to the target with the specified handle.
	 *
	 * @param	Handle		The handle of the target to connect to.
	 */
	bool ConnectToTarget(TARGETHANDLE Handle);

	/**
	 * Returns the number of targets available.
	 */
	int GetNumberOfTargets();

	/**
	 * Exists for compatability with UnrealConsole. Index is disregarded and CurrentTarget is disconnected if it contains a valid pointer.
	 *
	 * @param	Handle		Handle to the target to disconnect.
	 */
	void DisconnectTarget(const TARGETHANDLE Handle);
	
	/**
	 *	Sends message using given UDP client.
	 *	@param Handle			client to send message to
	 *	@param MessageType		sent message type
	 *	@param Message			actual text of the message
	 *	@return true on success, false otherwise
	 */
	bool SendToConsole(const TARGETHANDLE Handle, const EDebugServerMessageType MessageType, const string& Message = "");

	/**
	 *	Attempts to determine available targets.
	 */
	void DetermineTargets();

	/**
	 * Gets the default target.
	 */
	TargetPtr GetDefaultTarget();

	/**
	 * Retrieves a handle to each available target.
	 *
	 * @param	OutTargetList			An array to copy all of the target handles into.
	 * @param	InOutTargetListSize		This variable needs to contain the size of OutTargetList. When the function returns it will contain the number of target handles copied into OutTargetList.
	 */
	void GetTargets(TARGETHANDLE *OutTargetList, int *InOutTargetListSize);

	// accessors
	inline void SetTTYCallback(TTYEventCallbackPtr Callback) { TTYCallback = Callback; }
};

#endif
