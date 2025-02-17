/*=============================================================================
	UnTcpNetDriver.h: Unreal TCP/IP driver.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#if WITH_UE3_NETWORKING

/*-----------------------------------------------------------------------------
	UTcpipConnection.
-----------------------------------------------------------------------------*/

//
// Windows socket class.
//
class UTcpipConnection : public UNetConnection
{
	DECLARE_CLASS(UTcpipConnection,UNetConnection,CLASS_Config|CLASS_Transient,IpDrv)

	// Variables.
	FInternetIpAddr		RemoteAddr;
	FSocket*			Socket;
	UBOOL				OpenedLocally;
	FResolveInfo*		ResolveInfo;

	/**
	 * Initializes a connection with the passed in settings
	 *
	 * @param InDriver the net driver associated with this connection
	 * @param InSocket the socket associated with this connection
	 * @param InRemoteAddr the remote address for this connection
	 * @param InState the connection state to start with for this connection
	 * @param InOpenedLocally whether the connection was a client/server
	 * @param InURL the URL to init with
	 * @param InMaxPacket the max packet size that will be used for sending
	 * @param InPacketOverhead the packet overhead for this connection type
	 */
	virtual void InitConnection(UNetDriver* InDriver,FSocket* InSocket,const FInternetIpAddr& InRemoteAddr,EConnectionState InState,UBOOL InOpenedLocally,const FURL& InURL,INT InMaxPacket = 0,INT InPacketOverhead = 0);

	/**
	 * Sends a byte stream to the remote endpoint using the underlying socket
	 *
	 * @param Data the byte stream to send
	 * @param Count the length of the stream to send
	 */
	virtual void LowLevelSend(void* Data,INT Count);

	FString LowLevelGetRemoteAddress();
	FString LowLevelDescribe();

	/** @return Returns the address of the connection as an integer */
	virtual INT GetAddrAsInt(void)
	{
		DWORD OutAddr = 0;
		// Get the network byte order ip addr
		RemoteAddr.GetIp(OutAddr);
		return (INT)OutAddr;
	}
};

/*-----------------------------------------------------------------------------
	UTcpNetDriver.
-----------------------------------------------------------------------------*/

//
// BSD sockets network driver.
//
class UTcpNetDriver : public UNetDriver
{
	DECLARE_CLASS(UTcpNetDriver,UNetDriver,CLASS_Transient|CLASS_Config,IpDrv)

	UBOOL AllowPlayerPortUnreach;
	UBOOL LogPortUnreach;

	// Variables.
	FInternetIpAddr LocalAddr;
	FSocket* Socket;

	// Constructor.
	void StaticConstructor();
	UTcpNetDriver()
	{}

	// UNetDriver interface.
	UBOOL InitConnect( FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error );
	UBOOL InitListen( FNetworkNotify* InNotify, FURL& LocalURL, FString& Error );
	void TickDispatch( FLOAT DeltaTime );
	FString LowLevelGetNetworkNumber();
	void LowLevelDestroy();

	// UTcpNetDriver interface.
	UBOOL InitBase( UBOOL Connect, FNetworkNotify* InNotify, const FURL& URL, FString& Error );
	UTcpipConnection* GetServerConnection();
	FSocketData GetSocketData();
};

#endif	//#if WITH_UE3_NETWORKING
