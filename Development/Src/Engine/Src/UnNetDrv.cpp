/*=============================================================================
	UnNetDrv.cpp: Unreal network driver base class.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnNet.h"

DECLARE_STATS_GROUP(TEXT("Net"),STATGROUP_Net);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Ping"),STAT_Ping,STATGROUP_Net);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Channels"),STAT_Channels,STATGROUP_Net);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("In Rate (bytes)"),STAT_InRate,STATGROUP_Net);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Out Rate (bytes)"),STAT_OutRate,STATGROUP_Net);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("In Packets"),STAT_InPackets,STATGROUP_Net);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Out Packets"),STAT_OutPackets,STATGROUP_Net);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("In Bunches"),STAT_InBunches,STATGROUP_Net);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Out Bunches"),STAT_OutBunches,STATGROUP_Net);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Out Loss"),STAT_OutLoss,STATGROUP_Net);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("In Loss"),STAT_InLoss,STATGROUP_Net);
// Voice specific stats
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Voice bytes sent"),STAT_VoiceBytesSent,STATGROUP_Net);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Voice bytes recv"),STAT_VoiceBytesRecv,STATGROUP_Net);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Voice packets sent"),STAT_VoicePacketsSent,STATGROUP_Net);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Voice packets recv"),STAT_VoicePacketsRecv,STATGROUP_Net);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("In % Voice"),STAT_PercentInVoice,STATGROUP_Net);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Out % Voice"),STAT_PercentOutVoice,STATGROUP_Net);

/*-----------------------------------------------------------------------------
	UPackageMapLevel implementation.
-----------------------------------------------------------------------------*/

/**
 * Static constructor called once per class during static initialization via IMPLEMENT_CLASS
 * macro. Used to e.g. emit object reference tokens for realtime garbage collection or expose
 * properties for native- only classes.
 */
void UPackageMapLevel::StaticConstructor()
{
	UClass* TheClass = GetClass();
	TheClass->EmitObjectReference( STRUCT_OFFSET( UPackageMapLevel, Connection ) );
}

UBOOL UPackageMapLevel::CanSerializeObject( UObject* Obj )
{
	AActor* Actor = Cast<AActor>(Obj);
	if (Actor != NULL && !Actor->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		if (!Actor->bStatic && !Actor->bNoDelete)
		{
			// dynamic actors can be serialized if they have a channel
			UActorChannel* Ch = Connection->ActorChannels.FindRef(Actor);
			//old: induces a bit of lag. return Ch && Ch->OpenAcked;
			return Ch!=NULL; //new: reduces lag, increases bandwidth slightly.
		}
		else
		{
			// static actors can always be serialized on the client and can be on the server if the client has initialized the level it's in
			return (!GWorld->IsServer() || Connection->ClientHasInitializedLevelFor(Actor));
		}
	}
	// for others, we can if we're the client, it's not in a level, or the client has initialized the level it's in
	else if (Obj == NULL || !GWorld->IsServer() || Connection->ClientHasInitializedLevelFor(Obj))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

UBOOL UPackageMapLevel::SerializeObject( FArchive& Ar, UClass* Class, UObject*& Object )
{
	DWORD Index=0;
	if( Ar.IsLoading() )
	{
		Object = NULL;
		BYTE B=0; Ar.SerializeBits( &B, 1 );
		if( B )
		{
			// Dynamic actor or None.
			Ar.SerializeInt( Index, UNetConnection::MAX_CHANNELS );
			if( Index==0 )
			{
				Object = NULL;
			}
			else if
			(	!Ar.IsError()
			&&	Index<UNetConnection::MAX_CHANNELS
			&&	Connection->Channels[Index]
			&&	Connection->Channels[Index]->ChType==CHTYPE_Actor 
			&&	!Connection->Channels[Index]->Closing )
				Object = ((UActorChannel*)Connection->Channels[Index])->GetActor();
		}
		else
		{
			// Static object.
			Ar.SerializeInt( Index, MAX_OBJECT_INDEX );
			if( !Ar.IsError() )
				Object = IndexToObject( Index, 1 );

			// ignore it if it's an object inside a level we haven't finished initializing
			if (Object != NULL && GWorld != NULL)
			{
				// get the level for the object
				ULevel* Level = NULL;
				for (UObject* Obj = Object; Obj != NULL; Obj = Obj->GetOuter())
				{
					Level = Cast<ULevel>(Obj);
					if (Level != NULL)
					{
						break;
					}
				}
				if (Level != NULL && Level != GWorld->PersistentLevel)
				{
					AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
					UBOOL bLevelVisible = FALSE;
					for (INT i = 0; i < WorldInfo->StreamingLevels.Num(); i++)
					{
						if (WorldInfo->StreamingLevels(i)->LoadedLevel == Level)
						{
							bLevelVisible = WorldInfo->StreamingLevels(i)->bIsVisible;
							break;
						}
					}
					if (!bLevelVisible)
					{
						debugfSuppressed(NAME_DevNetTraffic, TEXT("Using None instead of replicated reference to %s because the level it's in has not been made visible"), *Object->GetFullName());
						Object = NULL;
					}
				}
			}
		}
		if( Object && !Object->IsA(Class) )
		{
			debugf(TEXT("Forged object: got %s, expecting %s"),*Object->GetFullName(),*Class->GetFullName());
			Object = NULL;
		}
		return 1;
	}
	else
	{
		AActor* Actor = Cast<AActor>(Object);
		if (Actor != NULL && !Actor->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
		{
			if (!Actor->bStatic && !Actor->bNoDelete)
			{
				// Map dynamic actor through channel index.
				BYTE B=1; Ar.SerializeBits( &B, 1 );
				UActorChannel* Ch = Connection->ActorChannels.FindRef(Actor);
				UBOOL Mapped = 0;
				if( Ch )
				{
					Index  = Ch->ChIndex;
					Mapped = Ch->OpenAcked;
				}
				Ar.SerializeInt( Index, UNetConnection::MAX_CHANNELS );
				return Mapped;
			}
			else if (GWorld->IsServer() && !Connection->ClientHasInitializedLevelFor(Object))
			{
				// send NULL for static actor because the client has not initialized its level yet
				BYTE B=1; Ar.SerializeBits( &B, 1 );
				Ar.SerializeInt( Index, UNetConnection::MAX_CHANNELS );
				return 0;
			}
		}
		else if (Object != NULL && GWorld->IsServer() && !Connection->ClientHasInitializedLevelFor(Object))
		{
			// send NULL for object in level because the client has not initialized that level yet
			BYTE B=1; Ar.SerializeBits( &B, 1 );
			Ar.SerializeInt( Index, UNetConnection::MAX_CHANNELS );
			return 0;
		}
		if( !Object || (Index=ObjectToIndex(Object))==INDEX_NONE )
		{
			// NULL object or we cannot serialize it
			BYTE B=1; Ar.SerializeBits( &B, 1 );
			Ar.SerializeInt( Index, UNetConnection::MAX_CHANNELS );
			return 1;
		}
		else
		{
			// Map regular object.
			// Since mappability doesn't change dynamically, there is no advantage to setting Result!=0.
			BYTE B=0; Ar.SerializeBits( &B, 1 );
			Ar.SerializeInt( Index, MAX_OBJECT_INDEX );
			return 1;
		}
	}
}
IMPLEMENT_CLASS(UPackageMapLevel);

/*-----------------------------------------------------------------------------
	UPackageMapSeekFree implementation.
-----------------------------------------------------------------------------*/

UBOOL UPackageMapSeekFree::SerializeObject( FArchive& Ar, UClass* Class, UObject*& Object )
{
	if( Ar.IsLoading() )
	{
		INT ChannelIndex;
		Ar << ChannelIndex;
		if( Ar.IsError() )
		{
			ChannelIndex = 0;
		}
		
		// Static object.
		if( ChannelIndex == INDEX_NONE )
		{
			FString ObjectPathName;
			Ar << ObjectPathName;
			if( !Ar.IsError() )
			{
				Object = UObject::StaticFindObject( Class, NULL, *ObjectPathName, FALSE );
			}
		}
		// Null object
		else if( ChannelIndex == 0 )
		{
			Object = NULL;
		}
		// Dynamic object.
		else
		{
			if( ChannelIndex<UNetConnection::MAX_CHANNELS
			&&	Connection->Channels[ChannelIndex]
			&&	Connection->Channels[ChannelIndex]->ChType==CHTYPE_Actor 
			&&	!Connection->Channels[ChannelIndex]->Closing )
			{
				Object = ((UActorChannel*)Connection->Channels[ChannelIndex])->GetActor();
			}
		}
	}
	else if( Ar.IsSaving() )
	{
		AActor* Actor = Cast<AActor>(Object);
		if (Actor != NULL && !Actor->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject) && !Actor->bStatic && !Actor->bNoDelete)
		{
			// Map dynamic actor through channel index.
			UActorChannel* ActorChannel = Connection->ActorChannels.FindRef(Actor);
			UBOOL bIsMapped		= 0;
			INT ChannelIndex	= 0;
			if( ActorChannel )
			{
				ChannelIndex	= ActorChannel->ChIndex;
				bIsMapped		= ActorChannel->OpenAcked;
			}

			Ar << ChannelIndex;
			return bIsMapped;
		}
		else if( !Object )
		{
			INT ChannelIndex = 0;
			Ar << ChannelIndex;
		}
		else
		{
			INT ChannelIndex = INDEX_NONE;
			Ar << ChannelIndex;
			FString ObjectPathName = Object->GetPathName();
			Ar << ObjectPathName;
		}
	}
	return TRUE;
}
UBOOL UPackageMapSeekFree::SerializeName( FArchive& Ar, FName& Name )
{
	if( Ar.IsLoading() )
	{
		FString InString;
		INT Number;
		Ar << InString << Number;
		Name = FName(*InString, Number);
	}
	else if( Ar.IsSaving() )
	{
		FString OutString = Name.GetNameString();
		INT Number = Name.GetNumber();
		Ar << OutString << Number;
	}
	return TRUE;
}

UBOOL UPackageMapSeekFree::SupportsPackage( UObject* InOuter )
{
	return TRUE;
}
UBOOL UPackageMapSeekFree::SupportsObject( UObject* Obj )
{
	return TRUE;
}
IMPLEMENT_CLASS(UPackageMapSeekFree);

/*-----------------------------------------------------------------------------
	UNetDriver implementation.
-----------------------------------------------------------------------------*/

UNetDriver::UNetDriver()
:	ClientConnections()
,	Time( 0.f )
,	DownloadManagers( E_NoInit )
,	InBytes(0)
,	OutBytes(0)
,	InPackets(0)
,	OutPackets(0)
,	InBunches(0)
,	OutBunches(0)
,	InPacketsLost(0)
,	OutPacketsLost(0)
,	InOutOfOrderPackets(0)
,	OutOutOfOrderPackets(0)
,	StatUpdateTime(0.0)
,	StatPeriod(1.f)
{
	if ( !HasAnyFlags(RF_ClassDefaultObject) )
	{
#if DO_ENABLE_NET_TEST
		// Parse the command-line parameters.
		PacketSimulationSettings.ParseSettings(appCmdLine());
#endif
		RoleProperty       = FindObjectChecked<UProperty>( AActor::StaticClass(), TEXT("Role"      ) );
		RemoteRoleProperty = FindObjectChecked<UProperty>( AActor::StaticClass(), TEXT("RemoteRole") );
		MasterMap          = new UPackageMap;
		ProfileStats	   = ParseParam(appCmdLine(),TEXT("profilestats"));
	}
}
void UNetDriver::StaticConstructor()
{
	// Expose CPF_Config properties to be loaded from .ini.
	new(GetClass(),TEXT("ConnectionTimeout"),    RF_Public)UFloatProperty(CPP_PROPERTY(ConnectionTimeout    ), TEXT("Client"), CPF_Config );
	new(GetClass(),TEXT("InitialConnectTimeout"),RF_Public)UFloatProperty(CPP_PROPERTY(InitialConnectTimeout), TEXT("Client"), CPF_Config );
	new(GetClass(),TEXT("KeepAliveTime"),        RF_Public)UFloatProperty(CPP_PROPERTY(KeepAliveTime        ), TEXT("Client"), CPF_Config );
	new(GetClass(),TEXT("RelevantTimeout"),      RF_Public)UFloatProperty(CPP_PROPERTY(RelevantTimeout      ), TEXT("Client"), CPF_Config );
	new(GetClass(),TEXT("SpawnPrioritySeconds"), RF_Public)UFloatProperty(CPP_PROPERTY(SpawnPrioritySeconds ), TEXT("Client"), CPF_Config );
	new(GetClass(),TEXT("ServerTravelPause"),    RF_Public)UFloatProperty(CPP_PROPERTY(ServerTravelPause    ), TEXT("Client"), CPF_Config );
	new(GetClass(),TEXT("MaxClientRate"),		 RF_Public)UIntProperty  (CPP_PROPERTY(MaxClientRate        ), TEXT("Client"), CPF_Config );
	new(GetClass(),TEXT("MaxInternetClientRate"),RF_Public)UIntProperty  (CPP_PROPERTY(MaxInternetClientRate), TEXT("Client"), CPF_Config );
	new(GetClass(),TEXT("NetServerMaxTickRate"), RF_Public)UIntProperty  (CPP_PROPERTY(NetServerMaxTickRate ), TEXT("Client"), CPF_Config );
	new(GetClass(),TEXT("bClampListenServerTickRate"),RF_Public)UBoolProperty(CPP_PROPERTY(bClampListenServerTickRate), TEXT("Client"), CPF_Config );
	new(GetClass(),TEXT("AllowDownloads"),       RF_Public)UBoolProperty (CPP_PROPERTY(AllowDownloads       ), TEXT("Client"), CPF_Config );
	new(GetClass(),TEXT("MaxDownloadSize"),	     RF_Public)UIntProperty  (CPP_PROPERTY(MaxDownloadSize      ), TEXT("Client"), CPF_Config );
	new(GetClass(),TEXT("NetConnectionClassName"),RF_Public)UStrProperty(CPP_PROPERTY(NetConnectionClassName), TEXT("Client"), CPF_Config );

	UArrayProperty* B = new(GetClass(),TEXT("DownloadManagers"),RF_Public)UArrayProperty( CPP_PROPERTY(DownloadManagers), TEXT("Client"), CPF_Config );
	B->Inner = new(B,TEXT("StrProperty0"),RF_Public)UStrProperty;

	UClass* TheClass = GetClass();
	TheClass->EmitObjectArrayReference( STRUCT_OFFSET( UNetDriver, ClientConnections ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UNetDriver, ServerConnection ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UNetDriver, MasterMap ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UNetDriver, RoleProperty ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UNetDriver, RemoteRoleProperty ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UNetDriver, NetConnectionClass ) );
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UNetDriver::InitializeIntrinsicPropertyValues()
{
	// Default values.
	MaxClientRate = 15000;
	MaxInternetClientRate = 10000;
}

void UNetDriver::AssertValid()
{
}
void UNetDriver::TickFlush()
{
	// Update network stats
	if (Time - StatUpdateTime > StatPeriod)
	{
		FLOAT RealTime = Time - StatUpdateTime;
		// Use the elapsed time to keep things scaled to one measured unit
		InBytes = appTrunc(InBytes / RealTime);
		OutBytes = appTrunc(OutBytes / RealTime);
		InPackets = appTrunc(InPackets / RealTime);
		OutPackets = appTrunc(OutPackets / RealTime);
		InBunches = appTrunc(InBunches / RealTime);
		OutBunches = appTrunc(OutBunches / RealTime);
		OutPacketsLost = appTrunc(100.f * OutPacketsLost / ::Max((FLOAT)OutPackets,1.f));
		InPacketsLost = appTrunc(100.f * InPacketsLost / ::Max((FLOAT)InPackets + InPacketsLost,1.f));
#if STATS
		// Copy the net status values over
		if (ServerConnection != NULL && ServerConnection->Actor != NULL && ServerConnection->Actor->PlayerReplicationInfo != NULL)
		{
			SET_DWORD_STAT(STAT_Ping, appTrunc(1000.0f * ServerConnection->Actor->PlayerReplicationInfo->ExactPing));
		}
		else
		{
			SET_DWORD_STAT(STAT_Ping, 0);
		}
		SET_DWORD_STAT(STAT_Channels, 0);
		if (ServerConnection != NULL)
		{
			INC_DWORD_STAT_BY(STAT_Channels, ServerConnection->OpenChannels.Num());
		}
		for (INT i = 0; i < ClientConnections.Num(); i++)
		{
			INC_DWORD_STAT_BY(STAT_Channels, ClientConnections(i)->OpenChannels.Num());
		}
		SET_DWORD_STAT(STAT_OutLoss,OutPacketsLost);
		SET_DWORD_STAT(STAT_InLoss,InPacketsLost);
		SET_DWORD_STAT(STAT_InRate,InBytes);
		SET_DWORD_STAT(STAT_OutRate,OutBytes);
		SET_DWORD_STAT(STAT_InPackets,InPackets);
		SET_DWORD_STAT(STAT_OutPackets,OutPackets);
		SET_DWORD_STAT(STAT_InBunches,InBunches);
		SET_DWORD_STAT(STAT_OutBunches,OutBunches);
		// Use the elapsed time to keep things scaled to one measured unit
		VoicePacketsSent = appTrunc(VoicePacketsSent / RealTime);
		SET_DWORD_STAT(STAT_VoicePacketsSent,VoicePacketsSent);
		VoicePacketsRecv = appTrunc(VoicePacketsRecv / RealTime);
		SET_DWORD_STAT(STAT_VoicePacketsRecv,VoicePacketsRecv);
		VoiceBytesSent = appTrunc(VoiceBytesSent / RealTime);
		SET_DWORD_STAT(STAT_VoiceBytesSent,VoiceBytesSent);
		VoiceBytesRecv = appTrunc(VoiceBytesRecv / RealTime);
		SET_DWORD_STAT(STAT_VoiceBytesRecv,VoiceBytesRecv);
		// Determine voice percentages
		if (InBytes > 0)
		{
			VoiceInPercent = appTrunc(100.f * (FLOAT)VoiceBytesRecv / (FLOAT)InBytes);
		}
		else
		{
			VoiceInPercent = 0;
		}
		if (OutBytes > 0)
		{
			VoiceOutPercent = appTrunc(100.f * (FLOAT)VoiceBytesSent / (FLOAT)OutBytes);
		}
		else
		{
			VoiceOutPercent = 0;
		}
		SET_DWORD_STAT(STAT_PercentInVoice,VoiceInPercent);
		SET_DWORD_STAT(STAT_PercentOutVoice,VoiceOutPercent);
#endif
		// Reset everything
		InBytes = 0;
		OutBytes = 0;
		InPackets = 0;
		OutPackets = 0;
		InBunches = 0;
		OutBunches = 0;
		OutPacketsLost = 0;
		InPacketsLost = 0;
		VoicePacketsSent = 0;
		VoiceBytesSent = 0;
		VoicePacketsRecv = 0;
		VoiceBytesRecv = 0;
		VoiceInPercent = 0;
		VoiceOutPercent = 0;
		StatUpdateTime = Time;
	}

	// Poll all sockets.
	if( ServerConnection )
	{
		// Queue client voice packets in the server's voice channel
		ProcessLocalClientPackets();
		ServerConnection->Tick();
	}
	else
	{
		// Queue up any voice packets the server has locally
		ProcessLocalServerPackets();
	}
	for( INT i=0; i<ClientConnections.Num(); i++ )
	{
		ClientConnections(i)->Tick();
	}
}

/**
 * Initializes the net connection class to use for new connections
 */
UBOOL UNetDriver::InitConnectionClass(void)
{
	if (NetConnectionClass == NULL && NetConnectionClassName != TEXT(""))
	{
		NetConnectionClass = LoadClass<UNetConnection>(NULL,*NetConnectionClassName,NULL,LOAD_None,NULL);
		if (NetConnectionClass == NULL)
		{
			debugf(NAME_Error,TEXT("Failed to load class '%s'"),*NetConnectionClassName);
		}
	}
	return NetConnectionClass != NULL;
}


UBOOL UNetDriver::InitConnect( FNetworkNotify* InNotify, const FURL& URL, FString& Error )
{
	InitConnectionClass();
	Notify = InNotify;

	return 1;
}
UBOOL UNetDriver::InitListen( FNetworkNotify* InNotify, FURL& URL, FString& Error )
{
	InitConnectionClass();
	Notify = InNotify;
	return 1;
}
void UNetDriver::TickDispatch( FLOAT DeltaTime )
{
	SendCycles=RecvCycles=0;

	// Get new time.
	Time += DeltaTime;

	// Delete any straggler connections.
	if( !ServerConnection )
	{
		for( INT i=ClientConnections.Num()-1; i>=0; i-- )
		{
			if( ClientConnections(i)->State==USOCK_Closed )
			{
				ClientConnections(i)->CleanUp();
			}
		}
	}
}
void UNetDriver::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );

	// Prevent referenced objects from being garbage collected.
	Ar << ClientConnections << ServerConnection << MasterMap << RoleProperty << RemoteRoleProperty;

}
void UNetDriver::FinishDestroy()
{
	if ( !HasAnyFlags(RF_ClassDefaultObject) )
	{
		// Destroy server connection.
		if( ServerConnection )
		{
			ServerConnection->CleanUp();
		}
		// Destroy client connections.
		while( ClientConnections.Num() )
		{
			UNetConnection* ClientConnection = ClientConnections(0);
			ClientConnection->CleanUp();
		}
		// Low level destroy.
		LowLevelDestroy();

		// remove from net object notify
		UPackage::NetObjectNotifies.RemoveItem(this);

		// Delete the master package map.
		MasterMap = NULL;
	}
	else
	{
		check(ServerConnection==NULL);
		check(ClientConnections.Num()==0);
		check(MasterMap==NULL);
	}
	Super::FinishDestroy();
}
UBOOL UNetDriver::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	if( ParseCommand(&Cmd,TEXT("SOCKETS")) )
	{
		// Print list of open connections.
		Ar.Logf( TEXT("Connections:") );
		if( ServerConnection )
		{
			Ar.Logf( TEXT("   Server %s"), *ServerConnection->LowLevelDescribe() );
			for( INT i=0; i<ServerConnection->OpenChannels.Num(); i++ )
				Ar.Logf( TEXT("      Channel %i: %s"), ServerConnection->OpenChannels(i)->ChIndex, *ServerConnection->OpenChannels(i)->Describe() );
		}
		for( INT i=0; i<ClientConnections.Num(); i++ )
		{
			UNetConnection* Connection = ClientConnections(i);
			Ar.Logf( TEXT("   Client %s"), *Connection->LowLevelDescribe() );
			for( INT j=0; j<Connection->OpenChannels.Num(); j++ )
				Ar.Logf( TEXT("      Channel %i: %s"), Connection->OpenChannels(j)->ChIndex, *Connection->OpenChannels(j)->Describe() );
		}
		return TRUE;
	}
	else if (ParseCommand(&Cmd, TEXT("PACKAGEMAP")))
	{
		// Print packagemap for open connections
		Ar.Logf(TEXT("Package Map:"));
		if (ServerConnection != NULL)
		{
			Ar.Logf(TEXT("   Server %s"), *ServerConnection->LowLevelDescribe());
			ServerConnection->PackageMap->LogDebugInfo(Ar);
		}
		for (INT i = 0; i < ClientConnections.Num(); i++)
		{
			UNetConnection* Connection = ClientConnections(i);
			Ar.Logf( TEXT("   Client %s"), *Connection->LowLevelDescribe() );
			Connection->PackageMap->LogDebugInfo(Ar);
		}
		return TRUE;
	}
#if DO_ENABLE_NET_TEST
	// This will allow changing the Pkt* options at runtime
	else if (PacketSimulationSettings.ParseSettings(Cmd))
	{
		if (ServerConnection)
		{
			// Notify the server connection of the change
			ServerConnection->UpdatePacketSimulationSettings();
		}
		else
		{
			// Notify all client connections that the settings have changed
			for (INT Index = 0; Index < ClientConnections.Num(); Index++)
			{
				ClientConnections(Index)->UpdatePacketSimulationSettings();
			}
		}
		return TRUE;
	}
#endif
	else
	{
		return FALSE;
	}
}
void UNetDriver::NotifyActorDestroyed( AActor* ThisActor )
{
	ForcedInitialReplicationMap.Remove(ThisActor);
	for( INT i=ClientConnections.Num()-1; i>=0; i-- )
	{
		UNetConnection* Connection = ClientConnections(i);
		if( ThisActor->bNetTemporary )
			Connection->SentTemporaries.RemoveItem( ThisActor );
		UActorChannel* Channel = Connection->ActorChannels.FindRef(ThisActor);
		if( Channel )
		{
			check(Channel->OpenedLocally);
			Channel->bClearRecentActorRefs = FALSE;
			Channel->Close();
		}
	}
}

/** creates a child connection and adds it to the given parent connection */
UChildConnection* UNetDriver::CreateChild(UNetConnection* Parent)
{
	debugf(NAME_DevNet, TEXT("Creating child connection with %s parent"), *Parent->GetName());
	UChildConnection* Child = new UChildConnection();
	Child->Driver = this;
	Child->URL = FURL();
	Child->State = Parent->State;
	Child->URL.Host = Parent->URL.Host;
	Child->Parent = Parent;
	Child->PackageMap = Parent->PackageMap;
	Child->CurrentNetSpeed = Parent->CurrentNetSpeed;
	Parent->Children.AddItem(Child);
	return Child;
}

/** notification when a package is added to the NetPackages list */
void UNetDriver::NotifyNetPackageAdded(UPackage* Package)
{
	if (!GIsRequestingExit && ServerConnection == NULL)
	{
		MasterMap->AddPackage(Package);
		for (INT i = 0; i < ClientConnections.Num(); i++)
		{
			if (ClientConnections(i) != NULL)
			{
				ClientConnections(i)->AddNetPackage(Package);
			}
		}
	}
}

/** notification when a package is removed from the NetPackages list */
void UNetDriver::NotifyNetPackageRemoved(UPackage* Package)
{
	if (!GIsRequestingExit && ServerConnection == NULL)
	{
		// we actually delete the entry in the MasterMap as otherwise when it gets sent to new clients those clients will attempt to load packages that it shouldn't have around
		MasterMap->RemovePackage(Package, TRUE);
		// we DO NOT delete the entry in client PackageMaps so that indices are not reshuffled during play as that can potentially result in mismatches
		for (INT i = 0; i < ClientConnections.Num(); i++)
		{
			if (ClientConnections(i) != NULL)
			{
				ClientConnections(i)->RemoveNetPackage(Package);
			}
		}
	}
}

/** notification when an object is removed from a net package's NetObjects list */
void UNetDriver::NotifyNetObjectRemoved(UObject* Object)
{
	UClass* RemovedClass = Cast<UClass>(Object);
	if (RemovedClass != NULL)
	{
		MasterMap->RemoveClassNetCache(RemovedClass);
		for (INT i = 0; i < ClientConnections.Num(); i++)
		{
			ClientConnections(i)->PackageMap->RemoveClassNetCache(RemovedClass);
		}
		if (ServerConnection != NULL)
		{
			ServerConnection->PackageMap->RemoveClassNetCache(RemovedClass);
		}
	}
}

/** The global structure for managing voice data */
FVoiceData GVoiceData;

/**
 * Process any local talker packets that need to be sent to clients
 */
void UNetDriver::ProcessLocalServerPackets(void)
{
	// Process all of the local packets
	for (DWORD Index = 0; Index < MAX_SPLITSCREEN_TALKERS; Index++)
	{
		FVoicePacket& LocalPacket = GVoiceData.LocalPackets[Index];
		// Check for something to send for this local talker
		if (LocalPacket.Length > 0)
		{
			// Create a new ref counted packet that is a copy of this local packet
			FVoicePacket* VoicePacket = new FVoicePacket(LocalPacket,1);
			// See if anyone wants this packet
			ReplicateVoicePacket(VoicePacket,NULL);
			// Relase our copy of the packet
			VoicePacket->DecRef();
			// Mark the local packet as processed
			LocalPacket.Length = 0;
		}
	}
}

/**
 * Process any local talker packets that need to be sent to the server
 */
void UNetDriver::ProcessLocalClientPackets(void)
{
	UVoiceChannel* VoiceChannel = ServerConnection->GetVoiceChannel();
	// Process all of the local packets
	for (DWORD Index = 0; Index < MAX_SPLITSCREEN_TALKERS; Index++)
	{
		FVoicePacket& LocalPacket = GVoiceData.LocalPackets[Index];
		// Check for something to send for this local talker
		if (LocalPacket.Length > 0)
		{
			// If there is a voice channel to the server, submit the packets
			if (VoiceChannel != NULL)
			{
				// Create a new ref counted packet that is a copy of this local packet
				FVoicePacket* VoicePacket = new FVoicePacket(LocalPacket,1);
				// Add the voice packet for network sending
				VoiceChannel->AddVoicePacket(VoicePacket);
				// Relase our copy of the packet
				VoicePacket->DecRef();
			}
			// Mark the local packet as processed
			LocalPacket.Length = 0;
		}
	}
}

/**
 * Determines which other connections should receive the voice packet and
 * queues the packet for those connections
 *
 * @param VoicePacket the packet to be queued
 * @param CameFromConn the connection this packet came from (obviously skip it)
 */
void UNetDriver::ReplicateVoicePacket(FVoicePacket* VoicePacket,UNetConnection* CameFromConn)
{
	// Iterate the connections and see if they want the packet
	for (INT Index = 0; Index < ClientConnections.Num(); Index++)
	{
		UNetConnection* Conn = ClientConnections(Index);
		// Skip the originating connection
		if (CameFromConn != Conn)
		{
			// See if the sender is muted or not
			if (Conn->ShouldReplicateVoicePacketFrom(VoicePacket->Sender))
			{
				UVoiceChannel* VoiceChannel = Conn->GetVoiceChannel();
				if (VoiceChannel != NULL)
				{
					// Add the voice packet for network sending
					VoiceChannel->AddVoicePacket(VoicePacket);
				}
			}
		}
	}
}

IMPLEMENT_CLASS(UNetDriver);

#if DO_ENABLE_NET_TEST
/**
 * Reads the settings from a string: command line or an exec
 *
 * @param Stream the string to read the settings from
 */
UBOOL FPacketSimulationSettings::ParseSettings(const TCHAR* Cmd)
{
	// note that each setting is tested.
	// this is because the same function will be used to parse the command line as well
	UBOOL bParsed = FALSE;

	if( Parse(Cmd,TEXT("PktLoss="), PktLoss) )
	{
		bParsed = TRUE;
		Clamp<INT>( PktLoss, 0, 100 );
		debugf(TEXT("PktLoss set to %d"), PktLoss);
	}
	if( Parse(Cmd,TEXT("PktOrder="), PktOrder) )
	{
		bParsed = TRUE;
		Clamp<INT>( PktOrder, 0, 1 );
		debugf(TEXT("PktOrder set to %d"), PktOrder);
	}
	if( Parse(Cmd,TEXT("PktLag="), PktLag) )
	{
		bParsed = TRUE;
		debugf(TEXT("PktLag set to %d"), PktLag);
	}
	if( Parse(Cmd,TEXT("PktDup="), PktDup) )
	{
		bParsed = TRUE;
		Clamp<INT>( PktDup, 0, 100 );
		debugf(TEXT("PktDup set to %d"), PktDup);
	}
	if( Parse(Cmd,TEXT("PktLagVariance="), PktLagVariance) )
	{
		bParsed = TRUE;
		Clamp<INT>( PktLagVariance, 0, 100 );
		debugf(TEXT("PktLagVariance set to %d"), PktLagVariance);
	}
	return bParsed;
}

#endif
