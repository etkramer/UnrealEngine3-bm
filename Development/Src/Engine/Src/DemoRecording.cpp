/*=============================================================================
	DemoRecDrv.cpp: Unreal demo recording network driver.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.

Revision history:
	* Created by Jack Porter.
=============================================================================*/

#include "EnginePrivate.h"
#include "DemoRecording.h"
#define PACKETSIZE 512

/*-----------------------------------------------------------------------------
	UDemoRecConnection.
-----------------------------------------------------------------------------*/

UDemoRecConnection::UDemoRecConnection()
{
	MaxPacket   = PACKETSIZE;
	InternalAck = 1;
}

/**
 * Intializes an "addressless" connection with the passed in settings
 *
 * @param InDriver the net driver associated with this connection
 * @param InState the connection state to start with for this connection
 * @param InURL the URL to init with
 * @param InConnectionSpeed Optional connection speed override
 */
void UDemoRecConnection::InitConnection(UNetDriver* InDriver, EConnectionState InState, const FURL& InURL, INT InConnectionSpeed)
{
	// default implementation
	Super::InitConnection(InDriver, InState, InURL, InConnectionSpeed);

	// the driver must be a DemoRecording driver (GetDriver makes assumptions to avoid Cast'ing each time)
	check(InDriver->IsA(UDemoRecDriver::StaticClass()));
}

FString UDemoRecConnection::LowLevelGetRemoteAddress()
{
	return TEXT("");
}

void UDemoRecConnection::LowLevelSend( void* Data, INT Count )
{
	if (!GetDriver()->ServerConnection && GetDriver()->FileAr)
	{
		*GetDriver()->FileAr << GetDriver()->LastDeltaTime << GetDriver()->FrameNum << Count;
		GetDriver()->FileAr->Serialize( Data, Count );
		//@todo demorec: if GetDriver()->GetFileAr()->IsError(), print error, cancel demo recording
	}
}

FString UDemoRecConnection::LowLevelDescribe()
{
	return TEXT("Demo recording driver connection");
}

INT UDemoRecConnection::IsNetReady( UBOOL Saturate )
{
	return 1;
}

void UDemoRecConnection::FlushNet()
{
	// in playback, there is no data to send except
	// channel closing if an error occurs.
	if( !GetDriver()->ServerConnection )
	{
		Super::FlushNet();
	}
}

void UDemoRecConnection::HandleClientPlayer( APlayerController* PC )
{
	Super::HandleClientPlayer(PC);
	PC->bDemoOwner = TRUE;
}

UBOOL UDemoRecConnection::ClientHasInitializedLevelFor(UObject* TestObject)
{
	// there is no way to know when demo playback will load levels,
	// so we assume it always is anytime after initial replication and rely on the client to load it in time to make the reference work
	return (GetDriver()->FrameNum > 2 || Super::ClientHasInitializedLevelFor(TestObject));
}

IMPLEMENT_CLASS(UDemoRecConnection);

/*-----------------------------------------------------------------------------
	UDemoRecDriver.
-----------------------------------------------------------------------------*/

UDemoRecDriver::UDemoRecDriver()
{}
UBOOL UDemoRecDriver::InitBase( UBOOL Connect, FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error )
{
	DemoFilename	= ConnectURL.Map;
	Time			= 0;
	FrameNum	    = 0;
	bHasDemoEnded	= FALSE;

	return TRUE;
}

UBOOL UDemoRecDriver::InitConnect( FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error )
{
#if !PS3
	// handle default initialization
	if (!Super::InitConnect(InNotify, ConnectURL, Error))
	{
		return FALSE;
	}
	if (!InitBase(1, InNotify, ConnectURL, Error))
	{
		return FALSE;
	}

	// Playback, local machine is a client, and the demo stream acts "as if" it's the server.
	ServerConnection = ConstructObject<UNetConnection>(UDemoRecConnection::StaticClass());
	ServerConnection->InitConnection(this, USOCK_Pending, ConnectURL, 1000000);

	// open the pre-recorded demo file
	FileAr = GFileManager->CreateFileReader(*DemoFilename);
	if( !FileAr )
	{
		Error = FString::Printf( TEXT("Couldn't open demo file %s for reading"), *DemoFilename );//@todo demorec: localize
		return 0;
	}

	// use the same byte format regardless of platform so that the demos are cross platform
	//@note: swap on non console platforms as the console archives have byte swapping compiled out by default
#if !XBOX && !PS3
	FileAr->SetByteSwapping(TRUE);
#endif

	LoopURL								= ConnectURL;
	bNoFrameCap							= ConnectURL.HasOption(TEXT("timedemo"));
	bAllowInterpolation					= !ConnectURL.HasOption(TEXT("disallowinterp"));
	bShouldExitAfterPlaybackFinished	= ConnectURL.HasOption(TEXT("exitafterplayback"));
	PlayCount							= appAtoi( ConnectURL.GetOption(TEXT("playcount="), TEXT("1")) );
	if( PlayCount == 0 )
	{
		PlayCount = INT_MAX;
	}
	bShouldSkipPackageChecking			= ConnectURL.HasOption(TEXT("skipchecks"));
	
	PlaybackStartTime					= appSeconds();
	LastFrameTime						= appSeconds();
#endif

	return TRUE;
}

/** sets the RemoteGeneration and LocalGeneration as appopriate for the given package info
 * so that the demo driver can correctly record object references into the demo
 */
void UDemoRecDriver::SetDemoPackageGeneration(FPackageInfo& Info)
{
	// content is always recorded as generation 1 so playing back the demo in a different language works
	// but we allow code to be the current version so that any new features/etc get recorded into the demo
	// (however, this means patches break old demos)
	if (Info.PackageFlags & PKG_ContainsScript)
	{
		Info.RemoteGeneration = Info.LocalGeneration;
	}
	else
	{
		Info.LocalGeneration = 1;
		Info.RemoteGeneration = 1;
	}
}

UBOOL UDemoRecDriver::InitListen( FNetworkNotify* InNotify, FURL& ConnectURL, FString& Error )
{
#if !PS3
	if( !Super::InitListen( InNotify, ConnectURL, Error ) )
	{
		return 0;
	}
	if( !InitBase( 0, InNotify, ConnectURL, Error ) )
	{
		return 0;
	}

	class AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
	if ( !WorldInfo )
	{
		Error = TEXT("No WorldInfo!!");
		return FALSE;
	}

	// Recording, local machine is server, demo stream acts "as if" it's a client.
	UDemoRecConnection* Connection = ConstructObject<UDemoRecConnection>(UDemoRecConnection::StaticClass());
	Connection->InitConnection(this, USOCK_Open, ConnectURL, 1000000);
	Connection->InitOut();

	FileAr = GFileManager->CreateFileWriter( *DemoFilename );
	ClientConnections.AddItem( Connection );

	if( !FileAr )
	{
		Error = FString::Printf( TEXT("Couldn't open demo file %s for writing"), *DemoFilename );//@todo demorec: localize
		return 0;
	}

	// use the same byte format regardless of platform so that the demos are cross platform
	//@note: swap on non console platforms as the console archives have byte swapping compiled out by default
#if !XBOX && !PS3
	FileAr->SetByteSwapping(TRUE);
#endif

	// Setup
	UGameEngine* GameEngine = CastChecked<UGameEngine>(GEngine);

	// Build package map.
	MasterMap->AddNetPackages();
	// fixup the RemoteGeneration to be LocalGeneration
	for (INT InfoIndex = 0; InfoIndex < MasterMap->List.Num(); InfoIndex++)
	{
		SetDemoPackageGeneration(MasterMap->List(InfoIndex));
	}
	MasterMap->Compute();

	UPackage::NetObjectNotifies.AddItem(this);

	// Create the control channel.
	Connection->CreateChannel( CHTYPE_Control, 1, 0 );

	// Send initial message.
	Connection->Logf( TEXT("HELLO P=%i REVISION=0 MINVER=%i VER=%i"),(INT)appGetPlatformType(), GEngineMinNetVersion, GEngineVersion );
	Connection->FlushNet();

	// Welcome the player to the level.
	FString WelcomeExtra;

	if( WorldInfo->NetMode == NM_Client )
	{
		WelcomeExtra = TEXT("NETCLIENTDEMO");
	}
	else if( WorldInfo->NetMode == NM_Standalone )
	{
		WelcomeExtra = TEXT("CLIENTDEMO");
	}
	else
	{
		WelcomeExtra = TEXT("SERVERDEMO");
	}

	WelcomeExtra = WelcomeExtra + FString::Printf(TEXT(" TICKRATE=%d"), WorldInfo->NetMode == NM_DedicatedServer ? appRound(GameEngine->GetMaxTickRate(0, FALSE)) : appRound(NetServerMaxTickRate) );

	GWorld->WelcomePlayer(Connection, *WelcomeExtra);

	// Spawn the demo recording spectator.
	SpawnDemoRecSpectator(Connection);
#endif
	return 1;
}

void UDemoRecDriver::NotifyNetPackageAdded(UPackage* Package)
{
	// overridden to force the Local/RemoteGeneration for demo playback
	if (!GIsRequestingExit && ServerConnection == NULL && !GUseSeekFreePackageMap)
	{
		// updating the master map is probably unnecessary after the connection has been created, but it doesn't hurt to keep it in sync
		SetDemoPackageGeneration(MasterMap->List(MasterMap->AddPackage(Package)));

		if (ClientConnections.Num() > 0 && ClientConnections(0) != NULL && ClientConnections(0)->bWelcomed)
		{
			INT Index = ClientConnections(0)->PackageMap->AddPackage(Package);
			SetDemoPackageGeneration(ClientConnections(0)->PackageMap->List(Index));
			ClientConnections(0)->SendPackageInfo(ClientConnections(0)->PackageMap->List(Index));
		}
	}
}

void UDemoRecDriver::StaticConstructor()
{
	new(GetClass(),TEXT("DemoSpectatorClass"), RF_Public)UStrProperty(CPP_PROPERTY(DemoSpectatorClass), TEXT("Client"), CPF_Config);
}

void UDemoRecDriver::LowLevelDestroy()
{
	debugf( TEXT("Closing down demo driver.") );

	// Shut down file.
	if( FileAr )
	{	
		delete FileAr;
		FileAr = NULL;
	}
}

UBOOL UDemoRecDriver::UpdateDemoTime( FLOAT* DeltaTime, FLOAT TimeDilation )
{
	UBOOL Result = 0;
	bNoRender = FALSE;

	// Playback.
	if( ServerConnection )
	{
		// skip play back if in player only mode
		if (GWorld->GetWorldInfo()->bPlayersOnly)
		{
			return 0;
		}

		// This will be triggered several times during initial handshake.
		if( FrameNum == 0 )
		{
			PlaybackStartTime = appSeconds();
		}

		if (ShouldInterpolate())
		{
			if (ServerConnection->State == USOCK_Open)
			{
				if (!FileAr->AtEnd() && !FileAr->IsError())
				{
					// peek at next delta time.
					FLOAT NewDeltaTime;
					INT NewFrameNum;

					*FileAr << NewDeltaTime << NewFrameNum;
					FileAr->Seek(FileAr->Tell() - sizeof(NewDeltaTime) - sizeof(NewFrameNum));

					// only increment frame if enough time has passed
					DemoRecMultiFrameDeltaTime += *DeltaTime * TimeDilation;
					while (DemoRecMultiFrameDeltaTime >= NewDeltaTime)
					{
						FrameNum++;
						DemoRecMultiFrameDeltaTime -= NewDeltaTime;
					}
				}
			}
			else
			{
				// increment the current frame every client frame until we're fully initialized
				FrameNum++;
			}
		}
		else
		{
			// Ensure LastFrameTime is inside a valid range, so we don't lock up if things get very out of sync.
			LastFrameTime = Clamp<DOUBLE>( LastFrameTime, appSeconds() - 1.0, appSeconds() );

			FrameNum++;
			if( ServerConnection->State==USOCK_Open ) 
			{
				if( !FileAr->AtEnd() && !FileAr->IsError() )
				{
					// peek at next delta time.
					FLOAT NewDeltaTime;
					INT NewFrameNum;

					*FileAr << NewDeltaTime << NewFrameNum;
					FileAr->Seek(FileAr->Tell() - sizeof(NewDeltaTime) - sizeof(NewFrameNum));

					// If the real delta time is too small, sleep for the appropriate amount.
					if( !bNoFrameCap )
					{
						if ( (appSeconds() > LastFrameTime+(DOUBLE)NewDeltaTime/TimeDilation) )
						{
							bNoRender = TRUE;
						}
						else
						{
							while(appSeconds() < LastFrameTime+(DOUBLE)NewDeltaTime/TimeDilation)
							{
								appSleep(0);
							}
						}
					}
					// Lie to the game about the amount of time which has passed.
					*DeltaTime = NewDeltaTime;
				}
			}
	 		LastFrameTime = appSeconds();
		}
	}
	// Recording.
	else
	{
		BYTE NetMode = GWorld->GetWorldInfo()->NetMode;

		// Accumulate the current DeltaTime for the real frames this demo frame will represent.
		DemoRecMultiFrameDeltaTime += *DeltaTime;

		// Cap client demo recording rate (but not framerate).
		if( NetMode==NM_DedicatedServer || ( (appSeconds()-LastClientRecordTime) >= (DOUBLE)(1.f/NetServerMaxTickRate) ) )
		{
			// record another frame.
			FrameNum++;
			LastClientRecordTime		= appSeconds();
			LastDeltaTime				= DemoRecMultiFrameDeltaTime;
			DemoRecMultiFrameDeltaTime	= 0.f;
			Result						= 1;

			// Save the new delta-time and frame number, with no data, in case there is nothing to replicate.
			INT Count = 0;
			*FileAr << LastDeltaTime << FrameNum << Count;
		}
	}

	return Result;
}

void UDemoRecDriver::DemoPlaybackEnded()
{
	ServerConnection->State = USOCK_Closed;
	bHasDemoEnded = TRUE;
	PlayCount--;

	FLOAT Seconds = appSeconds()-PlaybackStartTime;
	if( bNoFrameCap )
	{
		FString Result = FString::Printf(TEXT("Demo %s ended: %d frames in %lf seconds (%.3f fps)"), *DemoFilename, FrameNum, Seconds, FrameNum/Seconds );
		debugf(TEXT("%s"),*Result);
		if (ServerConnection->Actor != NULL)
		{
			ServerConnection->Actor->eventClientMessage( *Result, NAME_None );//@todo demorec: localize
		}
	}
	else
	{
		if (ServerConnection->Actor != NULL)
		{
			ServerConnection->Actor->eventClientMessage( *FString::Printf(TEXT("Demo %s ended: %d frames in %f seconds"), *DemoFilename, FrameNum, Seconds ), NAME_None );//@todo demorec: localize
		}
	}

	// Exit after playback of last loop iteration has finished.
	if( bShouldExitAfterPlaybackFinished && PlayCount == 0 )
	{
		GIsRequestingExit = TRUE;
	}

	if( PlayCount > 0 )
	{
		// Play while new loop count.
		LoopURL.AddOption( *FString::Printf(TEXT("playcount=%i"),PlayCount) );
		
		// Start over again.
		GWorld->Exec( *(FString(TEXT("DEMOPLAY "))+(*LoopURL.String())), *GLog );
	}
}

void UDemoRecDriver::TickDispatch( FLOAT DeltaTime )
{
	Super::TickDispatch( DeltaTime );

	if( ServerConnection && (ServerConnection->State==USOCK_Pending || ServerConnection->State==USOCK_Open) )
	{	
		BYTE Data[PACKETSIZE + 8];
		// Read data from the demo file
		DWORD PacketBytes;
		INT PlayedThisTick = 0;
		for( ; ; )
		{
			// At end of file?
			if( FileAr->AtEnd() || FileAr->IsError() )
			{
				DemoPlaybackEnded();
				return;
			}
	
			INT ServerFrameNum;
			FLOAT ServerDeltaTime;

			*FileAr << ServerDeltaTime;
			*FileAr << ServerFrameNum;
			if( ServerFrameNum > FrameNum )
			{
				FileAr->Seek(FileAr->Tell() - sizeof(ServerFrameNum) - sizeof(ServerDeltaTime));
				break;
			}
			*FileAr << PacketBytes;

			if( PacketBytes )
			{
				// Read data from file.
				FileAr->Serialize( Data, PacketBytes );
				if( FileAr->IsError() )
				{
					debugf( NAME_DevNet, TEXT("Failed to read demo file packet") );
					DemoPlaybackEnded();
					return;
				}

				// Update stats.
				PlayedThisTick++;

				// Process incoming packet.
				ServerConnection->ReceivedRawPacket( Data, PacketBytes );
			}

			if (ServerConnection == NULL || ServerConnection->State == USOCK_Closed)
			{
				// something we received resulted in the demo being stopped
				DemoPlaybackEnded();
				return;
			}

			// Only play one packet per tick on demo playback, until we're 
			// fully connected.  This is like the handshake for net play.
			if(ServerConnection->State == USOCK_Pending)
			{
				FrameNum = ServerFrameNum;
				break;
			}
		}
	}
}

FString UDemoRecDriver::LowLevelGetNetworkNumber()
{
	return TEXT("");
}

UBOOL UDemoRecDriver::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	if( bHasDemoEnded )
	{
		return 0;
	}
	if( ParseCommand(&Cmd,TEXT("DEMOREC")) || ParseCommand(&Cmd,TEXT("DEMOPLAY")) )
	{
		if( ServerConnection )
		{
			Ar.Logf( TEXT("Demo playback currently active: %s"), *DemoFilename );//@todo demorec: localize
		}
		else
		{
			Ar.Logf( TEXT("Demo recording currently active: %s"), *DemoFilename );//@todo demorec: localize
		}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("DEMOSTOP")) )
	{
		PlayCount = 0;
		Ar.Logf( TEXT("Demo %s stopped at frame %d"), *DemoFilename, FrameNum );//@todo demorec: localize
		if( !ServerConnection )
		{
			// let GC cleanup the object
			if (ClientConnections.Num() > 0 && ClientConnections(0) != NULL)
			{
				ClientConnections(0)->Close();
				ClientConnections(0)->CleanUp(); // make sure DemoRecSpectator gets destroyed immediately
			}

			GWorld->DemoRecDriver=NULL;
		}
		else
		{
			// flush out any pending network traffic
			ServerConnection->FlushNet();
			ServerConnection->State = USOCK_Closed;
			GEngine->SetClientTravel(TEXT("?closed"), TRAVEL_Absolute);
		}

		delete FileAr;
		FileAr = NULL;
		return TRUE;
	}
	else 
	{
		return Super::Exec(Cmd, Ar);
	}
}

void UDemoRecDriver::SpawnDemoRecSpectator( UNetConnection* Connection )
{
	UClass* C = StaticLoadClass( AActor::StaticClass(), NULL, *DemoSpectatorClass, NULL, LOAD_None, NULL );
	APlayerController* Controller = CastChecked<APlayerController>(GWorld->SpawnActor( C ));

	for (FActorIterator It; It; ++It)
	{
		if (It->IsA(APlayerStart::StaticClass()))
		{
			Controller->Location = It->Location;
			Controller->Rotation = It->Rotation;
			break;
		}
	}

	Controller->SetPlayer(Connection);
}
IMPLEMENT_CLASS(UDemoRecDriver);


/*-----------------------------------------------------------------------------
	UDemoPlayPendingLevel implementation.
-----------------------------------------------------------------------------*/

//
// Constructor.
//
UDemoPlayPendingLevel::UDemoPlayPendingLevel(const FURL& InURL)
:	UPendingLevel( InURL )
{
	NetDriver = NULL;

	// Try to create demo playback driver.
	UClass* DemoDriverClass = StaticLoadClass( UDemoRecDriver::StaticClass(), NULL, TEXT("engine-ini:Engine.Engine.DemoRecordingDevice"), NULL, LOAD_None, NULL );
	DemoRecDriver = ConstructObject<UDemoRecDriver>( DemoDriverClass );
	if( DemoRecDriver->InitConnect( this, URL, ConnectionError ) )
	{
	}
	else
	{
		//@todo ronp connection
		// make sure this failure is propagated to the game
		DemoRecDriver = NULL;
	}
}

//
// FNetworkNotify interface.
//
void UDemoPlayPendingLevel::NotifyReceivedText( UNetConnection* Connection, const TCHAR* Text )
{
#if !SHIPPING_PC_GAME
	debugf(NAME_DevNet, TEXT("DemoPlayPendingLevel received: %s"), Text);
#endif
	if( ParseCommand( &Text, TEXT("USES") ) )
	{
		// Dependency information.
		FPackageInfo& Info = *new(Connection->PackageMap->List)FPackageInfo(NULL);
		Connection->ParsePackageInfo(Text, Info);

		// in the seekfree loading case, we load the requested map first and then attempt to load requested packages that haven't been loaded yet
		// as packages referenced by the map might be forced exports and not actually have a file associated with them
		//@see UGameEngine::LoadMap()
		//@todo: figure out some early-out code to detect when missing downloadable content, etc so we don't have to load the level first
		if( !GUseSeekFreeLoading )
		{
			// verify that we have this package, or it is downloadable
			FString Filename;
			if (GPackageFileCache->FindPackageFile(*Info.PackageName.ToString(), DemoRecDriver->bShouldSkipPackageChecking ? NULL : &Info.Guid, Filename))
			{
				Info.Parent = CreatePackage(NULL, *Info.PackageName.ToString());
				// check that the GUID matches (meaning it is the same package or it has been conformed)
				BeginLoad();
				ULinkerLoad* Linker = GetPackageLinker(Info.Parent, NULL, LOAD_NoWarn | LOAD_NoVerify | LOAD_Quiet, NULL, DemoRecDriver->bShouldSkipPackageChecking ? NULL : &Info.Guid);
				EndLoad();
				if (Linker == NULL || (!DemoRecDriver->bShouldSkipPackageChecking && Linker->Summary.Guid != Info.Guid))
				{
					// incompatible files
					debugf(NAME_DevNet, TEXT("Package '%s' mismatched - Server GUID: %s Client GUID: %s"), *Info.Parent->GetName(), *Info.Guid.String(), (Linker != NULL) ? *Linker->Summary.Guid.String() : TEXT("None"));
					ConnectionError = FString::Printf(TEXT("Package '%s' version mismatch"), *Info.Parent->GetName());
					DemoRecDriver->ServerConnection->Close();
					return;
				}
				else
				{
					Info.LocalGeneration = Linker->Summary.Generations.Num();
					if (Info.LocalGeneration < Info.RemoteGeneration)
					{
						// the indices will be mismatched in this case as there's no real server to adjust them for our older package version
						debugf(NAME_DevNet, TEXT("Package '%s' mismatched for demo playback - local version: %i, demo version: %i"), *Info.Parent->GetName(), Info.LocalGeneration, Info.RemoteGeneration);
						ConnectionError = FString::Printf(TEXT("Package '%s' version mismatch"), *Info.Parent->GetName());
						DemoRecDriver->ServerConnection->Close();
						return;
					}
					// tell the server what we have
					DemoRecDriver->ServerConnection->Logf(TEXT("HAVE GUID=%s GEN=%i"), *Linker->Summary.Guid.String(), Info.LocalGeneration);
				}
			}
			else
			{
				// we need to download this package
				FilesNeeded++;
				Info.PackageFlags |= PKG_Need;
				/*@todo:
				if (DemoRecDriver->ClientRedirectURLs.Num()==0 || !DemoRecDriver->AllowDownloads || !(Info.PackageFlags & PKG_AllowDownload))
				*/
				if (TRUE)
				{
					ConnectionError = FString::Printf(TEXT("Demo requires missing/mismatched package '%s'"), *Info.PackageName.ToString());
					DemoRecDriver->ServerConnection->Close();
					return;
				}
			}
		}
	}
	else if( ParseCommand( &Text, TEXT("WELCOME") ) )
	{
		// Parse welcome message.
		Parse( Text, TEXT("LEVEL="), URL.Map );

		/*@todo:
		if (FilesNeeded > 0)
		{
			// Send first download request.
			ReceiveNextFile( Connection, 0 );
		}*/

		DemoRecDriver->Time = 0;
		bSuccessfullyConnected = TRUE;
	}
}

//
// UPendingLevel interface.
//
void UDemoPlayPendingLevel::Tick( FLOAT DeltaTime )
{
	check(DemoRecDriver);
	check(DemoRecDriver->ServerConnection);

	if( DemoRecDriver->ServerConnection && DemoRecDriver->ServerConnection->Download )
	{
		DemoRecDriver->ServerConnection->Download->Tick();
	}

	if( !FilesNeeded )
	{
		// Update demo recording driver.
		DemoRecDriver->UpdateDemoTime( &DeltaTime, 1.f );
		DemoRecDriver->TickDispatch( DeltaTime );
		DemoRecDriver->TickFlush();
	}
}

UNetDriver* UDemoPlayPendingLevel::GetDriver()
{
	return DemoRecDriver;
}

IMPLEMENT_CLASS(UDemoPlayPendingLevel);

