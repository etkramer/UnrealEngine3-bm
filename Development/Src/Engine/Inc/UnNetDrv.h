/*=============================================================================
	UnNetDrv.h: Unreal network driver base class.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _UNNETDRV_H_
#define _UNNETDRV_H_

/*-----------------------------------------------------------------------------
	UPackageMapLevel.
-----------------------------------------------------------------------------*/

class UPackageMapLevel : public UPackageMap
{
	DECLARE_CLASS(UPackageMapLevel,UPackageMap,CLASS_Transient,Engine);

	UNetConnection* Connection;

	UBOOL CanSerializeObject( UObject* Obj );
	UBOOL SerializeObject( FArchive& Ar, UClass* Class, UObject*& Obj );

	UPackageMapLevel()
	{}
	UPackageMapLevel( UNetConnection* InConnection )
	: Connection( InConnection )
	{}

	/**
	 * Static constructor called once per class during static initialization via IMPLEMENT_CLASS
	 * macro. Used to e.g. emit object reference tokens for realtime garbage collection or expose
	 * properties for native- only classes.
	 */
	void StaticConstructor();
};

/**
 * Temporary hack to allow seekfree loading to work in the absence of linkers or a package map
 * replacement by serializing objects and names as strings. This is NOT meant for shipping due to
 * the obvious performance and bandwidth impact of the approach.
 */
class UPackageMapSeekFree : public UPackageMapLevel
{
	DECLARE_CLASS(UPackageMapSeekFree,UPackageMapLevel,CLASS_Transient,Engine);
	
	virtual UBOOL SerializeObject( FArchive& Ar, UClass* Class, UObject*& Obj );
	virtual UBOOL SerializeName( FArchive& Ar, FName& Name );

	virtual UBOOL SupportsPackage( UObject* InOuter );
	virtual UBOOL SupportsObject( UObject* Obj );

	// Dummies stubbed out.
	virtual void Compute() {}
	virtual void Copy( UPackageMap* Other ) {}
	virtual void AddNetPackages() {}
	virtual INT AddPackage(UPackage* Package) { return INDEX_NONE; }
	
	// Dummies that should never be called.
	virtual INT ObjectToIndex( UObject* Object )
	{
		appErrorf(TEXT("This should never have been called with UPackageMapSeekFree!"));
		return INDEX_NONE;
	}
	virtual UObject* IndexToObject( INT InIndex, UBOOL Load )
	{
		appErrorf(TEXT("This should never have been called with UPackageMapSeekFree!"));
		return NULL;
	}

	UPackageMapSeekFree()
	{}

	UPackageMapSeekFree( UNetConnection* InConnection )
	: UPackageMapLevel( InConnection )
	{}
};

/**
 * Network stats counters
 */
enum ENetStats
{
	STAT_Ping = STAT_NetFirstStat,
	STAT_Channels,
	STAT_InRate,
	STAT_OutRate,
	STAT_InPackets,
	STAT_OutPackets,
	STAT_InBunches,
	STAT_OutBunches,
	STAT_OutLoss,
	STAT_InLoss,
	STAT_VoiceBytesSent,
	STAT_VoiceBytesRecv,
	STAT_VoicePacketsSent,
	STAT_VoicePacketsRecv,
	STAT_PercentInVoice,
	STAT_PercentOutVoice
};

#include "VoiceDataCommon.h"

/*-----------------------------------------------------------------------------
	UNetDriver.
-----------------------------------------------------------------------------*/

//
// Base class of a network driver attached to an active or pending level.
//
class UNetDriver : public USubsystem, public FNetObjectNotify
{
	DECLARE_ABSTRACT_CLASS(UNetDriver,USubsystem,CLASS_Transient|CLASS_Config|CLASS_Intrinsic,Engine)

	// Variables.
	TArray<UNetConnection*>		ClientConnections;
	UNetConnection*				ServerConnection;
	FNetworkNotify*				Notify;
	UPackageMap*				MasterMap;
	FLOAT						Time;
	FLOAT						ConnectionTimeout;
	FLOAT						InitialConnectTimeout;
	FLOAT						KeepAliveTime;
	FLOAT						RelevantTimeout;
	FLOAT						SpawnPrioritySeconds;
	FLOAT						ServerTravelPause;
	INT							MaxClientRate;
	INT							MaxInternetClientRate;
	INT							NetServerMaxTickRate;
	UBOOL						bClampListenServerTickRate;
	UBOOL						AllowDownloads;
	UBOOL						ProfileStats;
	UProperty*					RoleProperty;
	UProperty*					RemoteRoleProperty;
	INT							SendCycles, RecvCycles;
	INT							MaxDownloadSize;
	TArray<FString>				DownloadManagers;
	/** Stats for network perf */
	DWORD						InBytes;
	DWORD						OutBytes;
	DWORD						InPackets;
	DWORD						OutPackets;
	DWORD						InBunches;
	DWORD						OutBunches;
	DWORD						InPacketsLost;
	DWORD						OutPacketsLost;
	DWORD						InOutOfOrderPackets;
	DWORD						OutOutOfOrderPackets;
	/** Tracks the total number of voice packets sent */
	DWORD						VoicePacketsSent;
	/** Tracks the total number of voice bytes sent */
	DWORD						VoiceBytesSent;
	/** Tracks the total number of voice packets received */
	DWORD						VoicePacketsRecv;
	/** Tracks the total number of voice bytes received */
	DWORD						VoiceBytesRecv;
	/** Tracks the voice data percentage of in bound bytes */
	DWORD						VoiceInPercent;
	/** Tracks the voice data percentage of out bound bytes */
	DWORD						VoiceOutPercent;
	/** map of Actors to properties they need to be forced to replicate when the Actor is bNetInitial */
	TMap< AActor*, TArray<UProperty*> >	ForcedInitialReplicationMap;
	/** Time of last stat update */
	DOUBLE						StatUpdateTime;
	/** Interval between gathering stats */
	FLOAT						StatPeriod;
	/** Used to specify the class to use for connections */
	FStringNoInit				NetConnectionClassName;
	/** The loaded UClass of the net connection type to use */
	UClass*						NetConnectionClass;

#if DO_ENABLE_NET_TEST
	FPacketSimulationSettings	PacketSimulationSettings;
#endif

	// Constructors.
	UNetDriver();
	void StaticConstructor();
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	// UObject interface.
	void FinishDestroy();
	void Serialize( FArchive& Ar );
	/**
	 * Initializes the net connection class to use for new connections
	 */
	virtual UBOOL InitConnectionClass(void);

	// UNetDriver interface.
	virtual void LowLevelDestroy() PURE_VIRTUAL(UNetDriver::LowLevelDestroy,);
	virtual FString LowLevelGetNetworkNumber() PURE_VIRTUAL(UNetDriver::LowLevelGetNetworkNumber,return TEXT(""););
	virtual void AssertValid();
	virtual UBOOL InitConnect( FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error );
	virtual UBOOL InitListen( FNetworkNotify* InNotify, FURL& ListenURL, FString& Error );
	virtual void TickFlush();
	virtual void TickDispatch( FLOAT DeltaTime );
	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar=*GLog );
	virtual void NotifyActorDestroyed( AActor* Actor );
	/** creates a child connection and adds it to the given parent connection */
	virtual class UChildConnection* CreateChild(UNetConnection* Parent);

	/** notification when a package is added to the NetPackages list */
	virtual void NotifyNetPackageAdded(UPackage* Package);
	/** notification when a package is removed from the NetPackages list */
	virtual void NotifyNetPackageRemoved(UPackage* Package);
	/** notification when an object is removed from a net package's NetObjects list */
	virtual void NotifyNetObjectRemoved(UObject* Object);

	/**
	 * Process any local talker packets that need to be sent to clients
	 */
	virtual void ProcessLocalServerPackets(void);

	/**
	 * Process any local talker packets that need to be sent to the server
	 */
	virtual void ProcessLocalClientPackets(void);

	/**
	 * Determines which other connections should receive the voice packet and
	 * queues the packet for those connections
	 *
	 * @param VoicePacket the packet to be queued
	 * @param CameFromConn the connection this packet came from (obviously skip it)
	 */
	virtual void ReplicateVoicePacket(FVoicePacket* VoicePacket,UNetConnection* CameFromConn);
};

#endif
