/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <WinSock2.h>

enum ESocketFamily
{
	SF_Unspecified = AF_UNSPEC,
	SF_IPv4 = AF_INET,
	SF_IPv6 = AF_INET6,
	SF_Infrared = AF_IRDA,
	SF_Bluetooth = 32, //AF_BTM
};

enum ESocketType
{
	ST_Invalid = 0,
	ST_Stream = SOCK_STREAM,
	ST_Datagram = SOCK_DGRAM,
	ST_Raw = SOCK_RAW,
	ST_ReliableDatagram = SOCK_RDM,
	ST_PsuedoStreamDatagram = SOCK_SEQPACKET,
};

enum ESocketProtocol
{
	SP_Invalid = 0,
	SP_TCP = IPPROTO_TCP,
	SP_UDP = IPPROTO_UDP,
	SP_ReliableMulticast = 113, //IPPROTO_RM
};

enum ESocketFlags
{
	SF_None = 0,
	SF_Peek = MSG_PEEK,
	SF_OutOfBand = MSG_OOB,
	SF_WaitAll = MSG_WAITALL,
	SF_DontRoute = MSG_DONTROUTE,
	SF_Partial = MSG_PARTIAL,
};

/**
 * Wraps a windows socket.
 */
class FSocket
{
public:
	FSocket();
	FSocket(ESocketFamily SocketFamily, ESocketType SocketType, ESocketProtocol SocketProtocol);
	virtual ~FSocket();

	/**
	 * Associates the socket with an IO completion port.
	 *
	 * @param	IOCP	The IO completion port handle.
	 * @return	True if the operation succeeds.
	 */
	bool AssociateWithIOCP(HANDLE IOCP);

	/**
	 * Wrapper for the winsock recv().
	 *
	 * @param	Buffer		Buffer to save the recv'd data into.
	 * @param	BufLength	The size of Buffer.
	 * @param	Flags		Flags controlling the operation.
	 * @return	SOCKET_ERROR if the operation failed, otherwise the number of bytes recv'd.
	 */
	int Recv(char* Buffer, int BufLength, ESocketFlags Flags = SF_None);

	/**
	 * Wrapper for the winsock recvfrom().
	 *
	 * @param	Buffer		Buffer to save the recv'd data into.
	 * @param	BufLength	The size of Buffer.
	 * @param	FromAddress	The address the data was recv'd from.
	 * @param	Flags		Flags controlling the operation.
	 * @return	SOCKET_ERROR if the operation failed, otherwise the number of bytes recv'd.
	 */
	int RecvFrom(char* Buffer, int BufLength, sockaddr_in &FromAddress, ESocketFlags Flags = SF_None);

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
	bool RecvAsync(LPWSABUF Buffers, DWORD BufferCount, DWORD &BytesRecvd, WSAOVERLAPPED *EventArgs, ESocketFlags Flags = SF_None);

	/**
	 * Wrapper for the winsock send().
	 *
	 * @param	Buffer		Buffer to save the recv'd data into.
	 * @param	BufLength	The size of Buffer.
	 * @param	Flags		Flags controlling the operation.
	 * @return	SOCKET_ERROR if the operation failed, otherwise the number of bytes sent.
	 */
	int Send(char* Buffer, int BufLength, ESocketFlags Flags = SF_None);

	/**
	 * Wrapper for the winsock sendto().
	 *
	 * @param	Buffer		Buffer to save the recv'd data into.
	 * @param	BufLength	The size of Buffer.
	 * @param	Flags		Flags controlling the operation.
	 * @return	SOCKET_ERROR if the operation failed, otherwise the number of bytes sent.
	 */
	int SendTo(char* Buffer, int BufLength, ESocketFlags Flags = SF_None);

	/**
	 * Creates a new internal socket.
	 *
	 * @return	True if the operation succeeds.
	 */
	bool CreateSocket();

	/**
	 * Sets attributes used when creating a new socket via CreateSocket().
	 *
	 * @param	SocketFamily	The family of the socket.
	 * @param	SocketType		The type of socket to be created.
	 * @param	SocketProtocol	The protocol of the socket.
	 */
	void SetAttributes(ESocketFamily SocketFamily, ESocketType SocketType, ESocketProtocol SocketProtocol);

	/**
	 * Closes the socket.
	 */
	void Close();

	/**
	 * Sets the blocking state of the socket.
	 *
	 * @param	IsNonBlocking	True if the socket is set to non-blocking.
	 * @return	True if the operation succeeds.
	 */
	bool SetNonBlocking(bool IsNonBlocking);
	
	/**
	 * Sets the broadcasting state of the socket.
	 *
	 * @param	bEnable	True if the socket is set to broadcast.
	 * @return	True if the operation succeeds.
	 */
	bool SetBroadcasting(bool bEnable);

	/**
	 * Binds the socket to the current address.
	 *
	 * @return	True if the operation succeeds.
	 */
	bool Bind();

	/**
	 * Wraps the winsock connect() by connecting to the current address.
	 *
	 * @return	True if the operation succeeds.
	 */
	bool Connect();
	
	// functions for checking state
	inline bool IsValid() const { return Socket != INVALID_SOCKET; }
	inline bool IsAssociatedWithIOCP() const { return bAssociatedWithIOCP; }

	//mutators
	inline void SetAddress(const sockaddr_in Addr) { Address = Addr; }
	inline void SetAddress(const u_long Addr) { Address.sin_addr.s_addr = Addr; }
	inline void SetAddress(const char* Addr) { Address.sin_addr.s_addr = inet_addr(Addr); }
	inline void SetPort(const u_short Port) { Address.sin_port = htons(Port); }

	//accessors
	inline u_short GetPort() const { return ntohs(Address.sin_port); }
	inline u_long GetIP() const { return Address.sin_addr.s_addr; }
	inline ESocketProtocol GetProtocol() const { return (ESocketProtocol)ProtocolInfo.iProtocol; }
	inline ESocketFamily GetFamily() const { return (ESocketFamily)ProtocolInfo.iAddressFamily; }
	inline ESocketType GetType() const { return (ESocketType)ProtocolInfo.iSocketType; }

	//operator overloads
	inline operator bool() const { return IsValid(); }
	inline operator SOCKET() const { return Socket; }

private:
	/** Socket used. */
	SOCKET Socket;
	/** IP we're connecting to. */
	sockaddr_in Address;
	WSAPROTOCOL_INFO ProtocolInfo;
	bool bAssociatedWithIOCP;
};

#endif
