/*=============================================================================
	UnGame.cpp: Unreal game engine.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineUserInterfaceClasses.h"
#include "EngineAnimClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineParticleClasses.h"
#include "EngineAudioDeviceClasses.h"
#include "UnNet.h"
#include "DemoRecording.h"
#include "RemoteControl.h"
#include "ChartCreation.h"
#include "DownloadableContent.h"
#include "AllocatorFixedSizeFreeList.h"
#include "Database.h"
#include "ProfilingHelpers.h"
#include "ScenePrivate.h"

#if !CONSOLE && defined(_MSC_VER)
#include "..\Debugger\UnDebuggerCore.h"
#endif

UBOOL GDisallowNetworkTravel = FALSE;

IMPLEMENT_COMPARE_CONSTREF( FString, UnGame, { return appStricmp(*A,*B); } )

//
//	Object class implementation.
//

/*-----------------------------------------------------------------------------
	cleanup!!
-----------------------------------------------------------------------------*/

void UGameEngine::RedrawViewports()
{
	SCOPE_CYCLE_COUNTER(STAT_RedrawViewports);

	if ( GameViewport != NULL )
	{
		GameViewport->eventLayoutPlayers();
		if ( GameViewport->Viewport != NULL )
		{
			GameViewport->Viewport->Draw();
		}
	}
}

/*-----------------------------------------------------------------------------
	Game init and exit.
-----------------------------------------------------------------------------*/

//
// Construct the game engine.
//
UGameEngine::UGameEngine()
: LastURL(TEXT(""))
{}

//
// Initialize the game engine.
//
void UGameEngine::Init()
{
	// Call base.
	UEngine::Init();

	// Sanity checking.
	VERIFY_CLASS_OFFSET( A, Actor, Owner );
	VERIFY_CLASS_OFFSET( A, PlayerController,	ViewTarget );
	VERIFY_CLASS_OFFSET( A, Pawn, Health );
    VERIFY_CLASS_SIZE( UEngine );
    VERIFY_CLASS_SIZE( UGameEngine );
	
	VERIFY_CLASS_SIZE( UTexture );
	VERIFY_CLASS_OFFSET( U, Texture, UnpackMax );

	VERIFY_CLASS_SIZE( USkeletalMeshComponent );
	VERIFY_CLASS_OFFSET( U, SkeletalMeshComponent, SkeletalMesh );

	VERIFY_CLASS_SIZE( USkeletalMesh );
	VERIFY_CLASS_OFFSET( U, SkeletalMesh, RefBasesInvMatrix );

	VERIFY_CLASS_SIZE( UPlayer );
	VERIFY_CLASS_SIZE( ULocalPlayer );
	VERIFY_CLASS_SIZE( UAudioComponent );

	debugf(TEXT("Object size..............: %u"),sizeof(UObject));
	debugf(TEXT("Actor size...............: %u"),sizeof(AActor));
	debugf(TEXT("ActorComponent size......: %u"),sizeof(UActorComponent));
	debugf(TEXT("PrimitiveComponent size..: %u"),sizeof(UPrimitiveComponent));

#if !CONSOLE && _MSC_VER
	if( ParseParam(appCmdLine(),TEXT("AUTODEBUG")) )
	{
		if ( ParseParam(appCmdLine(), TEXT("VADEBUG")) )
		{
			debugf(TEXT("Attaching script debugger (Visual Studio interface)"));
		}
		else
		{
			debugf(TEXT("Attaching script debugger (UDE interface)"));
		}
		UDebuggerCore* Debugger = new UDebuggerCore();
		GDebugger 				= Debugger;

		// we want the UDebugger to break on the first bytecode it processes
		Debugger->SetBreakASAP(TRUE);
		Debugger->EnableDebuggerProcessing(TRUE);
	}
	else if ( ParseParam(appCmdLine(),TEXT("VADEBUG")) )
	{
		debugf(TEXT("Attaching script debugger (Visual Studio interface)"));

		// otherwise, if we are running the script debugger from within VS.NET, we need to create the UDebugger (but not attach it to the game)
		// so that we can receive log messages from the game and user input events (e.g. set breakpoint) from VS.NET
		GDebugger = new UDebuggerCore();
	}
#endif


	// load the guid cache if it exists
	FString Filename;
	if (GPackageFileCache->FindPackageFile(TEXT("GuidCache"), NULL, Filename))
	{
		UGuidCache::CreateInstance(*Filename);
	}

#if PS3
	// update the guid cache with patch files (must be done between loading global guid cache
	// and installing any DLC
	extern void appPS3PatchGuidCache();
	appPS3PatchGuidCache();
#endif

	// potentially (platform-dependent) look for DLC before loading initial map
	GPlatformDownloadableContent->FindDownloadableContent();


	// If not a dedicated server.
	if( GIsClient )
	{	
		// Init client.
		UClass* ClientClass = StaticLoadClass( UClient::StaticClass(), NULL, TEXT("engine-ini:Engine.Engine.Client"), NULL, LOAD_None, NULL );
		Client = ConstructObject<UClient>( ClientClass );
		Client->Init( this );
	}


	// Initialize the viewport client.
	UGameViewportClient* ViewportClient = NULL;
	if(Client)
	{
		ViewportClient = ConstructObject<UGameViewportClient>(GameViewportClientClass,this);
		GameViewport = ViewportClient;
	}
	else
	{
		// dedicated server - create a data store client and reference through the UIInteraction CDO
		UUIInteraction* DefaultUIController = UUIInteraction::StaticClass()->GetDefaultObject<UUIInteraction>();
		if ( DefaultUIController != NULL )
		{
			DefaultUIController->InitializeGlobalDataStore();
		}
	}

	// Attach the viewport client to a new viewport.
	if(ViewportClient)
	{
		Parse(appCmdLine(), TEXT("ResX="), GSystemSettings.ResX);
		Parse(appCmdLine(), TEXT("ResY="), GSystemSettings.ResY);
		if (ParseParam(appCmdLine(),TEXT("WINDOWED")))
		{
			GSystemSettings.bFullscreen = FALSE;
		}
		else if (ParseParam(appCmdLine(),TEXT("FULLSCREEN")))
		{
			GSystemSettings.bFullscreen = TRUE;
		}

		FViewportFrame* ViewportFrame = Client->CreateViewportFrame(
			ViewportClient,
			*LocalizeGeneral("Product",GPackage),
			GSystemSettings.ResX,
			GSystemSettings.ResY,
			GSystemSettings.bFullscreen
			);

		// Start up the networking layer before the UI and after
		// rendering has been set up (important!)
		InitOnlineSubsystem();

		FString Error;
		if(!ViewportClient->eventInit(Error))
		{
			appErrorf(TEXT("%s"),*Error);
		}

		GameViewport->SetViewportFrame(ViewportFrame);
	}
	else
	{
#if WITH_PANORAMA
		extern void appPanoramaHookInit();
		// Initialize Panorama without rendering (dedicated server mode)
		appPanoramaHookInit();
#endif
		// Start up the networking layer that was specified
		InitOnlineSubsystem();
	}


	// Create default URL.
	// @note:  if we change how we determine the valid start up map update LaunchEngineLoop's GetStartupMap()
	FURL DefaultURL;
	DefaultURL.LoadURLConfig( TEXT("DefaultPlayer"), GGameIni );

	// Enter initial world.
	FString Error;
	TCHAR Parm[4096]=TEXT("");
	const TCHAR* Tmp = appCmdLine();
	if(	!ParseToken( Tmp, Parm, ARRAY_COUNT(Parm), 0 ) || Parm[0]=='-' )
	{
		appStrcpy( Parm, *FURL::DefaultLocalMap );
	}
	FURL URL( &DefaultURL, Parm, TRAVEL_Partial );
	if( !URL.Valid )
	{
		appErrorf( LocalizeSecure(LocalizeError(TEXT("InvalidUrl"),TEXT("Engine")), Parm) );
	}
	UBOOL Success = Browse( URL, Error );

	// If waiting for a network connection, go into the starting level.
	if (!Success && appStricmp(Parm, *FURL::DefaultLocalMap) != 0)
	{
		// the map specified on the command-line couldn't be loaded.  ask the user if we should load the default map instead
		if ( appStricmp( *URL.Map, *FURL::DefaultLocalMap )!=0 &&
			appMsgf(AMT_OKCancel, LocalizeSecure(LocalizeError(TEXT("FailedMapload_NotFound"),TEXT("Engine")), *URL.Map)) == 0 )
		{
			// user cancelled (maybe a typo while attempting to run a commandlet)
			appRequestExit( FALSE );
			return;
		}
		else
		{
			Success = Browse( FURL(&DefaultURL,*FURL::DefaultLocalMap,TRAVEL_Partial), Error );
		}
	}

	// Handle failure.
	if( !Success )
	{
		appErrorf( LocalizeSecure(LocalizeError(TEXT("FailedBrowse"),TEXT("Engine")), Parm, *Error) );
	}

#if USING_REMOTECONTROL
	extern UBOOL GUsewxWindows;
	if( GUsewxWindows && RemoteControlExec && !GIsEditor )
	{
		// Suppress remote control for dedicated servers.
		if ( Client )
		{
			UBOOL bSuppressRemoteControl = FALSE;
			GConfig->GetBool( TEXT("RemoteControl"), TEXT("SuppressRemoteControlAtStartup"), bSuppressRemoteControl, GEngineIni );
			if ( ParseParam(appCmdLine(), TEXT("NORC")) || ParseParam(appCmdLine(), TEXT("NOREMOTECONTROL")) )
			{
				bSuppressRemoteControl = TRUE;
			}
			if ( !bSuppressRemoteControl )
			{
				RemoteControlExec->Show( TRUE );
				RemoteControlExec->SetFocusToGame();
			}
		}
	}
#endif

	debugf( NAME_Init, TEXT("Game engine initialized") );
}

//
// Game exit.
//
void UGameEngine::PreExit()
{
	Super::PreExit();

	if (OnlineSubsystem)
	{
		// Let the online subsystem clean up any resources and finish any async tasks
		OnlineSubsystem->eventExit();
	}

	if( GPendingLevel )
	{
		CancelPending();
	}

	// Clean up world.
	if ( GWorld != NULL )
	{
		// notify the gameinfo
		AGameInfo *Info = GWorld->GetGameInfo();
		if (Info != NULL)
		{
			Info->eventPreExit();
		}
		GWorld->FlushLevelStreaming( NULL, TRUE );
		GWorld->TermWorldRBPhys();
		GWorld->CleanupWorld();
	}
}

void UGameEngine::FinishDestroy()
{
	if ( !HasAnyFlags(RF_ClassDefaultObject) )
	{
		// Game exit.
		debugf( NAME_Exit, TEXT("Game engine shut down") );
	}

	Super::FinishDestroy();
}

//
// Progress text.
//
void UGameEngine::SetProgress( EProgressMessageType MessageType, const FString& Title, const FString& Message )
{
	if (GamePlayers.Num() && GamePlayers(0)->Actor && !GamePlayers(0)->Actor->ActorIsPendingKill())
	{
		GamePlayers(0)->Actor->eventSetProgressMessage(MessageType, Message, Title);
	}
	// if we failed to find a PlayerController to handle a connection failure, perform the default behavior of returning to the entry level
	else if (GIsClient && MessageType == PMT_ConnectionFailure && TravelURL == TEXT(""))
	{
		SetClientTravel(TEXT("?failed"), TRAVEL_Absolute);
	}
}

/*-----------------------------------------------------------------------------
	Command line executor.
-----------------------------------------------------------------------------*/

class ParticleSystemUsage
{
public:
	UParticleSystem* Template;
	INT	Count;
	INT	ActiveTotal;
	INT	MaxActiveTotal;
	// Reported whether the emitters are instanced or not...
	INT	StoredMaxActiveTotal;

	TArray<INT>		EmitterActiveTotal;
	TArray<INT>		EmitterMaxActiveTotal;
	// Reported whether the emitters are instanced or not...
	TArray<INT>		EmitterStoredMaxActiveTotal;

	ParticleSystemUsage() :
		Template(NULL),
		Count(0),
		ActiveTotal(0),
		MaxActiveTotal(0),
		StoredMaxActiveTotal(0)
	{
	}

	~ParticleSystemUsage()
	{
	}
};

IMPLEMENT_COMPARE_POINTER( UStaticMesh, UnGame, { return B->GetResourceSize() - A->GetResourceSize(); } )

//
// This always going to be the last exec handler in the chain. It
// handles passing the command to all other global handlers.
//
UBOOL UGameEngine::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	const TCHAR* Str=Cmd;
	if( ParseCommand( &Str, TEXT("MEMTAG_UPDATE") ) )
	{
#if !FINAL_RELEASE
		{
			static INT NumRounds = -1;			

			// parse the various command line options we have
			INT NumWarmUpRounds = 0;
			INT RoundsBetweenTagging = 1;

			INT FromCommandLine = 0;
			Parse( appCmdLine(), TEXT("-MemTaggingNumWarmUpRounds="), FromCommandLine );
			if( FromCommandLine != 0 )
			{
				NumWarmUpRounds = FromCommandLine;
			}

			FromCommandLine = 0;
			Parse( appCmdLine(), TEXT("-MemTaggingRoundsBetweenTagging="), FromCommandLine );
			if( FromCommandLine != 0 )
			{
				RoundsBetweenTagging = FromCommandLine;
			}

			// test to see if the number of warm up rounds has passed
			NumRounds++;
			if( NumRounds < NumWarmUpRounds )
			{
				debugf( TEXT( "MemTagging:  Still Warming Up %d out of %d before we start" ), NumRounds, NumWarmUpRounds );
				return TRUE;
			}


			// start and stop tracking based on number of rounds that have passed or if this is the round immediately after warming up
			if( ( (NumRounds%RoundsBetweenTagging) == 0 ) || ( NumRounds == (NumWarmUpRounds) ) )
			{
				// for where we stop tracking check GameInfo.uc  GetNextAutomatedTestingMap()

				// snapshot time!
				GMalloc->Exec(*FString::Printf(TEXT("SNAPSHOTMEMORY")),*GLog);
			}
			else
			{
				debugf( TEXT( "MemTagging:  This round did not qualify for a new mem tag. %d rounds until next tag" ), RoundsBetweenTagging-(NumRounds%RoundsBetweenTagging) );
			}
		}
#endif // !FINAL_RELEASE

		return TRUE;
	}
	else if( ParseCommand(&Str,TEXT("REATTACHCOMPONENTS")) )
	{
		UClass* Class=NULL;
		if( ParseObject<UClass>( Str, TEXT("CLASS="), Class, ANY_PACKAGE ) &&
			Class->IsChildOf(UActorComponent::StaticClass()) )
		{
			for( FObjectIterator It(Class); It; ++It )
			{
				UActorComponent* ActorComponent = Cast<UActorComponent>(*It);
				if( ActorComponent )
				{
					FComponentReattachContext Reattach(ActorComponent);
				}
			}
		}
		return TRUE;
	}
	// exec to start/stop all movies
	else if( ParseCommand(&Str,TEXT("MOVIE")) )
	{
		FString MovieCmd = ParseToken(Str,0);
		if( MovieCmd.InStr(TEXT("PLAY"),FALSE,TRUE) != INDEX_NONE )
		{
			for( TObjectIterator<UTextureMovie> It; It; ++It )
			{
				UTextureMovie* Movie = *It;
				Movie->Play();
			}
		}
		else if( MovieCmd.InStr(TEXT("PAUSE"),FALSE,TRUE) != INDEX_NONE )
		{
			for( TObjectIterator<UTextureMovie> It; It; ++It )
			{
				UTextureMovie* Movie = *It;
				Movie->Pause();
			}
		}
		else if( MovieCmd.InStr(TEXT("STOP"),FALSE,TRUE) != INDEX_NONE )
		{
			for( TObjectIterator<UTextureMovie> It; It; ++It )
			{
				UTextureMovie* Movie = *It;
				Movie->Stop();
			}
		}
		return TRUE;
	}
	else if( ParseCommand( &Str, TEXT("DEFERRED_STOPMEMTRACKING_AND_DUMP") ) )
	{
		new(GEngine->DeferredCommands) FString( TEXT( "SNAPSHOTMEMORY" ) );
		new(GEngine->DeferredCommands) FString( TEXT( "STOPTRACKING" ) );
		new(GEngine->DeferredCommands) FString( TEXT( "DUMPALLOCSTOFILE" ) );

		return TRUE;
	}
	else if( ParseCommand( &Str, TEXT("OPEN") ) )
	{
		// make sure the file exists if we are opening a local file
		FURL TestURL(&LastURL, Str, TRAVEL_Partial);
		if (TestURL.IsLocalInternal())
		{
			FString PackageFilename;
			if (!GPackageFileCache->FindPackageFile(*TestURL.Map, NULL, PackageFilename))
			{
				Ar.Logf(TEXT("ERROR: The map '%s' does not exist."), *TestURL.Map);
				return TRUE;
			}
		}
		else if ( TestURL.IsInternal() )
		{
			GEngine->SetProgress(PMT_Information, TEXT(""), LocalizeProgress(TEXT("Connecting"), TEXT("Engine")));
		}

		SetClientTravel( Str, TRAVEL_Partial );
		return TRUE;
	}
	else if( ParseCommand( &Str, TEXT("STREAMMAP")) )
	{
		// make sure the file exists if we are opening a local file
		FURL TestURL(&LastURL, Str, TRAVEL_Partial);
		if (TestURL.IsLocalInternal())
		{
			FString PackageFilename;
			if (GPackageFileCache->FindPackageFile(*TestURL.Map, NULL, PackageFilename))
			{
				TArray<FName> LevelNames;
				LevelNames.AddItem(*TestURL.Map);
				PrepareMapChange(LevelNames);

				bShouldCommitPendingMapChange				= TRUE;
				ConditionalCommitMapChange();
			}
			else
			{
				Ar.Logf(TEXT("ERROR: The map '%s' does not exist."), *TestURL.Map);
			}
		}
		else
		{
			Ar.Logf(TEXT("ERROR: Can only perform streaming load for local URLs."));
		}
		return TRUE;
	}
	else if( ParseCommand( &Str, TEXT("START") ) )
	{
		// make sure the file exists if we are opening a local file
		FURL TestURL(&LastURL, Str, TRAVEL_Absolute);
		if (TestURL.IsLocalInternal())
		{
			FString PackageFilename;
			if (!GPackageFileCache->FindPackageFile(*TestURL.Map, NULL, PackageFilename))
			{
				Ar.Logf(TEXT("ERROR: The map '%s' does not exist."), *TestURL.Map);
				return TRUE;
			}
		}
		else if ( TestURL.IsInternal() )
		{
			GEngine->SetProgress(PMT_Information, TEXT(""), LocalizeProgress(TEXT("Connecting"), TEXT("Engine")));
		}

		SetClientTravel( Str, TRAVEL_Absolute );
		return TRUE;
	}
	else if (ParseCommand(&Str, TEXT("SERVERTRAVEL")) && GWorld->IsServer())
	{
		GWorld->GetWorldInfo()->eventServerTravel(Str);
		return TRUE;
	}
	else if( (GIsServer && !GIsClient) && ParseCommand( &Str, TEXT("SAY") ) )
	{
		GWorld->GetWorldInfo()->Game->eventBroadcast(NULL, Str, NAME_None);
		return TRUE;
	}
	else if( ParseCommand(&Str, TEXT("DISCONNECT")) )
	{
		// Remove ?Listen parameter, if it exists
		LastURL.RemoveOption( TEXT("Listen") );
		LastURL.RemoveOption( TEXT("LAN") ) ;

		UNetDriver* NetDriver = GWorld->GetNetDriver();
		// we're the client
		if( NetDriver && NetDriver->ServerConnection && NetDriver->ServerConnection->Channels[0] )
		{
			NetDriver->ServerConnection->Channels[0]->Close();
			NetDriver->ServerConnection->FlushNet();
		}
		if( GPendingLevel && GPendingLevel->NetDriver && GPendingLevel->NetDriver->ServerConnection && GPendingLevel->NetDriver->ServerConnection->Channels[0] )
		{
			GPendingLevel->NetDriver->ServerConnection->Channels[0]->Close();
			GPendingLevel->NetDriver->ServerConnection->FlushNet();
		}

		if ( ParseCommand(&Str, TEXT("LOCAL")) )
		{
			// shut down netdriver only; don't travel back to entry yet
			// Clean up the socket so that the pending level succeeds
			UNetDriver* NetDriver = GWorld->GetNetDriver();
			if (NetDriver)
			{
				if ( NetDriver->ServerConnection )
				{
					NetDriver->ServerConnection->Close();
					NetDriver->ServerConnection->FlushNet();
				}

				//@todo - should we do anything if we have ClientConnections ?   currently handling this in gamecode...

				NetDriver->LowLevelDestroy();
			}

			GWorld->SetNetDriver(NULL);
		}
		else
		{
			// we're the server - immediately notify all connected clients that we're closing down
			if ( NetDriver && NetDriver->ClientConnections.Num() > 0 )
			{
				for ( INT ClientIndex = 0; ClientIndex < NetDriver->ClientConnections.Num(); ClientIndex++ )
				{
					NetDriver->ClientConnections(ClientIndex)->Logf(TEXT("FAILURE FORCED"));
					NetDriver->ClientConnections(ClientIndex)->FlushNet();
				}
			}

			SetClientTravel( TEXT("?closed"), TRAVEL_Absolute );
		}

		return TRUE;
	}
	else if( ParseCommand(&Str, TEXT("RECONNECT")) )
	{
		if (LastRemoteURL.Valid && LastRemoteURL.Host != TEXT(""))
		{
			SetClientTravel(*LastRemoteURL.String(), TRAVEL_Absolute);
		}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("EXIT")) || ParseCommand(&Cmd,TEXT("QUIT")))
	{
		// Dump FPS chart before exiting.
		DumpFPSChart();
		// Dump distance factor chart before exiting.
		DumpDistanceFactorChart();
		// Dump Memory chart before exiting.
		DumpMemoryChart();

		if(( FString(appCmdLine()).InStr( TEXT( "DoingASentinelRun=1" ), FALSE, TRUE ) != -1 )
			|| ( GSentinelRunID != -1 )
			)
		{
			const FString EndRun = FString::Printf(TEXT("EXEC EndRun @RunID=%i, @ResultDescription='%s'")
				, GSentinelRunID
				, *PerfMemRunResultStrings[ARR_Passed] 
			);

			//warnf( TEXT("%s"), *EndRun );
			GTaskPerfMemDatabase->SendExecCommand( *EndRun );
		}

#if !CONSOLE && defined(_MSC_VER)
		if ( GDebugger != NULL )
		{
			GDebugger->Close(TRUE);
		}
#endif // !CONSOLE && defined(_MSC_VER)

		if( GWorld->GetWorldInfo() && GWorld->GetWorldInfo()->Game )
		{
			GWorld->GetWorldInfo()->Game->eventGameEnding();
		}

		UNetDriver* NetDriver = GWorld->GetNetDriver();
		if( NetDriver && NetDriver->ServerConnection && NetDriver->ServerConnection->Channels[0] )
		{
			NetDriver->ServerConnection->Channels[0]->Close();
			NetDriver->ServerConnection->FlushNet();
		}
		if( GPendingLevel && GPendingLevel->NetDriver && GPendingLevel->NetDriver->ServerConnection && GPendingLevel->NetDriver->ServerConnection->Channels[0] )
		{
			GPendingLevel->NetDriver->ServerConnection->Channels[0]->Close();
			GPendingLevel->NetDriver->ServerConnection->FlushNet();
		}

		Ar.Log( TEXT("Closing by request") );
		appRequestExit( 0 );
		return TRUE;
	}
	else if( ParseCommand( &Str, TEXT("GETMAXTICKRATE") ) )
	{
		Ar.Logf( TEXT("%f"), GetMaxTickRate(0,FALSE) );
		return TRUE;
	}
	else if( ParseCommand( &Cmd, TEXT("CANCEL") ) )
	{
		static UBOOL InCancel = FALSE;
		if( !InCancel )	
		{
			//!!Hack for broken Input subsystem.  JP.
			//!!Inside LoadMap(), ResetInput() is called,
			//!!which can retrigger an Exec call.
			InCancel = TRUE;
			if( GPendingLevel )
			{
				if( GPendingLevel->TrySkipFile() )
				{
					InCancel = 0;
					return TRUE;
				}

				SetProgress( PMT_Information, *LocalizeProgress(TEXT("CancelledConnect"),TEXT("Engine")), TEXT("") );
			}
			else
			{
				SetProgress( PMT_Clear, TEXT(""), TEXT("") );
			}

			CancelPending();
			InCancel = FALSE;
		}

		return TRUE;
	}
	// This will list out the packages which are in the precache list and have not been "loaded" out.  (e.g. could be just there taking up memory!)
	else if( ParseCommand( &Cmd, TEXT("ListPrecacheMapPackages") ) )
	{
		TArray<FString> Packages;
		ULinkerLoad::GetListOfPackagesInPackagePrecacheMap( Packages );

		Sort<USE_COMPARE_CONSTREF(FString,UnGame)>( &Packages(0), Packages.Num() );

		Ar.Logf( TEXT( "Total Number Of Packages In PrecacheMap: %i " ), Packages.Num() );

		for( INT i = 0; i < Packages.Num(); ++i )
		{
			Ar.Logf( TEXT( "%i %s" ), i, *Packages(i) );
		}
		Ar.Logf( TEXT( "Total Number Of Packages In PrecacheMap: %i " ), Packages.Num() );

		return TRUE;

	}
	// we can't always do an obj linkers, as cooked games have their linkers tossed out.  So we need to look at the actual packages which are loaded
	else if( ParseCommand( &Cmd, TEXT("ListLoadedPackages") ) )
	{
		TArray<FString> Packages;

		for( TObjectIterator<UPackage> It; It; ++It )
		{
			UPackage* Package = *It;

			const UBOOL bIsARootPackage = Package->GetOuter() == NULL;

			if( bIsARootPackage == TRUE )
			{
				Packages.AddItem( Package->GetFullName() );
				//warnf( TEXT("Package %s"), *Package->GetFullName() );
			}
		}

		Sort<USE_COMPARE_CONSTREF(FString,UnGame)>( &Packages(0), Packages.Num() );

		Ar.Logf( TEXT( "Total Number Of Packages Loaded: %i " ), Packages.Num() );

		for( INT i = 0; i < Packages.Num(); ++i )
		{
			Ar.Logf( TEXT( "%i %s" ), i, *Packages(i) );
		}
		Ar.Logf( TEXT( "Total Number Of Packages Loaded: %i " ), Packages.Num() );

		return TRUE;
	}
	else if( ParseCommand( &Cmd, TEXT("MemReport") ) )
	{
#if !FINAL_RELEASE
		const FString Token = ParseToken( Str, 0 );
		const UBOOL bCheckFrag = ( Token == TEXT("FRAG") );

		new(GEngine->DeferredCommands) FString( TEXT( "MEM DETAILED" ) );

		Exec( TEXT( "MEMORYSPLIT ACTUAL" ) );

		FMemoryChartEntry NewMemoryEntry = FMemoryChartEntry();
		NewMemoryEntry.UpdateMemoryChartStats();
		Ar.Logf( *NewMemoryEntry.ToString() );


		Exec( TEXT( "OBJ LIST -ALPHASORT" ) );
		Exec( TEXT( "LISTTEXTURES" ) );
		Exec( TEXT( "LISTSOUNDS" ) );

		Exec( TEXT( "LISTAUDIOCOMPONENTS" ) );
		
		Exec( TEXT( "ListLoadedPackages" ) );
		Exec( TEXT( "ListPrecacheMapPackages" ) );

		// print out some mem stats
		GConfig->ShowMemoryUsage(Ar);

		if (bCheckFrag == TRUE)
		{
			Exec( TEXT( "MemFragCheck" ) );
		}

#endif // !FINAL_RELEASE

		return TRUE;
	}
	else if ( ParseCommand( &Cmd, TEXT("MemFragCheck") ) )
	{
        // this will defer the MemFragCheck to the end of the frame so we can force a GC and get a real frag with no gc'able objects
        new(GEngine->DeferredCommands) FString( TEXT( "MemFragCheckPostGC" ) );

		FColor ErrorColor(255,0,0);
		GWorld->GetWorldInfo()->AddOnScreenDebugMessage((INT)this, 1.0f, ErrorColor, TEXT("MemFragCheck is Active!!!!") );

		return TRUE;
	}
    // do not call this from script as this will do a GC which can crash and is unsafe.  Instead call:  MemFragCheck
	else if ( ParseCommand( &Cmd, TEXT("MemFragCheckPostGC") ) )
    {
		FlushAsyncLoading();
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS,TRUE);
		FlushRenderingCommands();

		GMalloc->CheckMemoryFragmentationLevel( Ar );

		return TRUE;
    }
	else if( ParseCommand( &Cmd, TEXT("MemLeakCheck") ) )
	{
		// this will defer the MemFragCheck to the end of the frame so we can force a GC and get a real frag with no gc'able objects
		new(GEngine->DeferredCommands) FString( TEXT( "MemLeakCheckPostGC" ) );

		return TRUE;
	}
	else if( ParseCommand( &Cmd, TEXT("MemLeakCheckPostGC") ) )
	{
		FlushAsyncLoading();
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS,TRUE);
		FlushRenderingCommands();

		const FString PathName = *(appProfilingDir() + TEXT("MemLeaks") PATH_SEPARATOR );
		GFileManager->MakeDirectory( *PathName );

		const FString Filename = CreateProfileFilename( TEXT(".memlk"), TRUE );
		const FString FilenameFull = PathName + Filename;
		FOutputDeviceFile FileAr(*FilenameFull);

		debugf(TEXT("MemLeakCheckPostGC: saving to %s"), *FilenameFull);

#ifdef ENABLE_MEM_TAGGING
		// for now don't do this and only do SNAPSHOTMEMORY Manually as the snapshots eat up precious mprof size and you will oom when analyzing them
		//Exec(TEXT("MEMTAG_UPDATE"));
#endif	//#ifdef ENABLE_MEM_TAGGING

		Exec( TEXT( "MEMORYSPLIT ACTUAL" ), FileAr );

		FMemoryChartEntry NewMemoryEntry = FMemoryChartEntry();
		NewMemoryEntry.UpdateMemoryChartStats();
		FileAr.Logf( *NewMemoryEntry.GetHeaders() );
		FileAr.Logf( *NewMemoryEntry.ToString() );

		FileAr.Logf( LINE_TERMINATOR );

		Exec( TEXT("OBJ LIST -ALPHASORT"), FileAr );

		FileAr.Logf( LINE_TERMINATOR );

		Exec( TEXT("OBJ LIST class=SkeletalMesh -ALPHASORT"), FileAr );

		FileAr.Logf( LINE_TERMINATOR );

		Exec( TEXT("LISTANIMSETS -ALPHASORT"), FileAr );
		
		FileAr.Logf( LINE_TERMINATOR );

		Exec( TEXT( "LISTTEXTURES -ALPHASORT" ), FileAr );

		FileAr.Logf( LINE_TERMINATOR );

		Exec( TEXT( "LISTSOUNDS -ALPHASORT" ), FileAr );

		FileAr.Logf( LINE_TERMINATOR );

		Exec( TEXT( "LISTAUDIOCOMPONENTS" ), FileAr );

		FileAr.Logf( LINE_TERMINATOR );

		GEngine->GameViewport->Exec( TEXT("LOGOUTSTATLEVELS"), FileAr );

		FileAr.Logf( LINE_TERMINATOR );

		Exec( TEXT( "ListLoadedPackages" ), FileAr );

		FileAr.Logf( LINE_TERMINATOR );

		Exec( TEXT( "ListPrecacheMapPackages" ), FileAr );

		FileAr.Logf( LINE_TERMINATOR );

		GConfig->ShowMemoryUsage(FileAr);

		FileAr.Logf( LINE_TERMINATOR );

		Exec( TEXT( "MEM DETAILED" ), FileAr );

		FileAr.Logf( LINE_TERMINATOR );

		Exec( TEXT( "GAMESPECIFIC_MEMORY" ), FileAr );

		FileAr.Logf( LINE_TERMINATOR );

		Exec( TEXT( "LISTSPAWNEDACTORS" ), FileAr );

		FileAr.Logf( LINE_TERMINATOR );
		
		FileAr.Logf( LINE_TERMINATOR );

		Exec( TEXT( "RENDERTARGET_MEM_USAGE" ), FileAr );

		FileAr.Logf( LINE_TERMINATOR );

		Exec( TEXT( "GPURESOURCEDUMP" ), FileAr );
		
		FileAr.Logf( LINE_TERMINATOR );

#if PS3
		Exec( TEXT( "PS3DUMPGPU2AR" ), FileAr);
#endif	//PS3

		FileAr.TearDown();

		SendDataToPCViaUnrealConsole( TEXT("UE_PROFILER!MEMLEAK:"), *(FilenameFull) );

		return TRUE;
	}
	else if( ParseCommand(&Str,TEXT("MEM")) )
	{
		const FString Token = ParseToken( Str, 0 );
		const UBOOL bSummaryOnly = ( Token != TEXT("STAT") );
		const UBOOL bDetailed	 = ( Token == TEXT("DETAILED") );	

#if PS3 && (!FINAL_RELEASE || FINAL_RELEASE_DEBUGCONSOLE) && !USE_NULL_RHI
		ENQUEUE_UNIQUE_RENDER_COMMAND(Bah2, 
		{ 
			extern void DEBUG_DumpFixedSizePoolAllocatedSizes();
			DEBUG_DumpFixedSizePoolAllocatedSizes();
		});
#endif	

		FlushAsyncLoading();
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS,TRUE);
		FlushRenderingCommands();

		if( !bSummaryOnly || bDetailed )
		{
			if( bSummaryOnly )
			{
				GMalloc->Exec( TEXT("DUMPALLOCS SUMMARYONLY -AlphaSort"), Ar );
			}
			else
			{
				GMalloc->Exec( TEXT("DUMPALLOCS -AlphaSort"), Ar );
			}
			Ar.Logf( TEXT("GMainThreadMemStack allocation size [used/ unused] = [%i / %i] KByte"), GMainThreadMemStack.GetByteCount() / 1024, GMainThreadMemStack.GetUnusedByteCount() / 1024  );
		}

#if PS3
		appPS3DumpDetailedMemoryInformation( Ar );
#endif

		SIZE_T VirtualAllocationSize	= 0;
		SIZE_T PhysicalAllocationSize	= 0;
		GMalloc->GetAllocationInfo( VirtualAllocationSize, PhysicalAllocationSize );

		Ar.Logf( TEXT("Virtual  memory allocation size: %6.2f MByte (%i Bytes)"), VirtualAllocationSize  / 1024.f / 1024.f, VirtualAllocationSize );
		Ar.Logf( TEXT("Physical memory allocation size: %6.2f MByte (%i Bytes)"), PhysicalAllocationSize / 1024.f / 1024.f, PhysicalAllocationSize );

		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("RENDERTARGET_MEM_USAGE")) )
	{
		FlushRenderingCommands();
		GSceneRenderTargets.DumpMemoryUsage( Ar );
		return TRUE;
	}

	else if( ParseCommand(&Str,TEXT("TRIMMEMORY")) )
	{
		GMalloc->TrimMemory(0,TRUE);
		return TRUE;
	}
	else if( ParseCommand(&Str,TEXT("MALLOC")) )
	{
		const FString Token = ParseToken(Str,0);
		Ar.Logf(TEXT("MALLOC %s"),*Token);
		return GMalloc->Exec(*Token,Ar);
	}
	else if( ParseCommand( &Cmd, TEXT("MEMORYSPLIT") ) )
	{
#if !FINAL_RELEASE
		// if this is set, this will be closer to what's actually allocated
		UBOOL bShowActualAllocated = (ParseToken(Cmd, 0) == TEXT("ACTUAL"));

		//  Gather various object resource sizes.
		INT		LevelDataSize			= 0;
		INT		BSPDataSize				= 0;
		INT		ParticlePeakSize		= 0;
		INT		ParticleActiveSize		= 0;

		// get a list of generic classes to track 
		TMultiMap<FString, FString>* ClassesToTrack;
		ClassesToTrack = GConfig->GetSectionPrivate(TEXT("MemorySplitClassesToTrack"), 0, 1, GEngineIni);

		// set up a map to store usage, with 0 initial mem used
		TMap<UClass*, INT> MemUsageByClass;
		if (ClassesToTrack)
		{
			for (TMultiMap<FString, FString>::TIterator It(*ClassesToTrack); It; ++It)
			{
				// find class by name
				UClass* Class = FindObject<UClass>(ANY_PACKAGE, *It.Value());

				if (Class)
				{
					MemUsageByClass.Set(Class, 0);
				}
			}
		}

		for( TObjectIterator<UObject> It; It; ++It )
		{
			UObject* Object = *It;

			INT ObjectSize = Object->GetResourceSize();
			if( ObjectSize == 0 )
			{
				FArchiveCountMem CountBytesSize( Object );
				// if we want accurate results, count the max (actual amount allocated)
				ObjectSize = bShowActualAllocated ? CountBytesSize.GetMax() : CountBytesSize.GetNum();
			}

			UBOOL bMemoryWasAccountedFor = FALSE;

			// look in the generic class tracking map for the exact class
			INT* MemUsage = MemUsageByClass.Find(Object->GetClass());

			// if not found, look for subclasses with fake IsA
			if (MemUsage == NULL)
			{
				for (TMap<UClass*, INT>::TIterator It(MemUsageByClass); It; ++It)
				{
					if (Object->IsA(It.Key()))
					{
						MemUsage = &It.Value();

						break;
					}
				}
			}

			// if we found the class in the map, update it's memory
			if (MemUsage)
			{
				*MemUsage += ObjectSize;

				bMemoryWasAccountedFor = TRUE;
			}

			// don't count the memory twice if we already counted it and we want
			// to show as accurate results as possibl
			if (bMemoryWasAccountedFor && bShowActualAllocated)
			{
				continue;
			}


			// handle objects that may be counted in two sections
			if(	Object->IsA(UModelComponent::StaticClass())
			||	Object->IsA(UBrushComponent::StaticClass()) )
			{
				LevelDataSize += ObjectSize;

				// don't count the memory twice if we want to be accurate
				if (!bShowActualAllocated)
				{
					BSPDataSize += ObjectSize;
				}
			}


			if( Object->IsA(ULevel::StaticClass())
			||	Object->IsA(UStaticMeshComponent::StaticClass())
			||	Object->IsA(USkeletalMeshComponent::StaticClass()) 
			||	Object->IsA(UBrushComponent::StaticClass()) )
			{
				LevelDataSize += ObjectSize;
			}

			if( Object->IsA(UModel::StaticClass()) )
			{
				BSPDataSize += ObjectSize;
			}

			if( Object->IsA(UParticleSystemComponent::StaticClass()) )
			{
				FArchiveCountMem CountBytesSize( Object);
				ParticlePeakSize    += CountBytesSize.GetMax();
				ParticleActiveSize += CountBytesSize.GetNum();
			}
		}

		// Gather audio stats.
		INT		SoundDataSize			= 0;

		// Gather texture memory stats.
		INT		TexturePoolAllocSize	= 0;
		INT		TexturePoolAvailSize	= 0;
		RHIGetTextureMemoryStats( TexturePoolAllocSize, TexturePoolAvailSize );

		// Gather malloc stats.
		SIZE_T	VirtualMemorySize		= 0;
		SIZE_T	PhysicalMemorySize		= 0;
		GMalloc->GetAllocationInfo( VirtualMemorySize, PhysicalMemorySize );

		Ar.Logf( TEXT("Memory split (in KByte)") );
		Ar.Logf( TEXT("  BSP                 : %6i"), BSPDataSize	/ 1024 );
		Ar.Logf( TEXT("  Level               : %6i"), LevelDataSize / 1024 );
		Ar.Logf( TEXT("  Particles (Active)  : %6i"), ParticleActiveSize / 1024 );
		Ar.Logf( TEXT("  Particles (Peak)    : %6i"), ParticlePeakSize / 1024 );
		
		for (TMap<UClass*, INT>::TIterator It(MemUsageByClass); It; ++It)
		{
#if (PS3 || PLATFORM_UNIX)
			Ar.Logf(TEXT("  %20ls: %6i"), *It.Key()->GetName(), It.Value() / 1024);
#else
			Ar.Logf(TEXT("  %20s: %6i"), *It.Key()->GetName(), It.Value() / 1024);
#endif
		}
#endif
		return TRUE;
	}
	else if( ParseCommand( &Cmd, TEXT("PARTICLEMESHUSAGE") ) )
	{
		// Mapping from static mesh to particle systems using it.
		TMultiMap<UStaticMesh*,UParticleSystem*> StaticMeshToParticleSystemMap;
		// Unique array of referenced static meshes, used for sorting and index into map.
		TArray<UStaticMesh*> UniqueReferencedMeshes;

		// Iterate over all mesh modules to find and keep track of mesh to system mappings.
		for( TObjectIterator<UParticleModuleTypeDataMesh> It; It; ++It )
		{
			UStaticMesh* StaticMesh = It->Mesh;
			if( StaticMesh )
			{
				// Find particle system in outer chain.
				UParticleSystem* ParticleSystem = NULL;
				UObject* Outer = It->GetOuter();
				while( Outer && !ParticleSystem )
				{
					ParticleSystem = Cast<UParticleSystem>(Outer);
					Outer = Outer->GetOuter();
				}

				// Add unique mapping from static mesh to particle system.
				if( ParticleSystem )
				{
					StaticMeshToParticleSystemMap.AddUnique( StaticMesh, ParticleSystem );
					UniqueReferencedMeshes.AddUniqueItem( StaticMesh );
				}
			}
		}

		// Sort by resource size.
		Sort<USE_COMPARE_POINTER(UStaticMesh,UnGame)>( UniqueReferencedMeshes.GetTypedData(), UniqueReferencedMeshes.Num() );
		
		// Calculate total size for summary.
		INT TotalSize = 0;
		for( INT StaticMeshIndex=0; StaticMeshIndex<UniqueReferencedMeshes.Num(); StaticMeshIndex++ )
		{
			UStaticMesh* StaticMesh	= UniqueReferencedMeshes(StaticMeshIndex);
			TotalSize += StaticMesh->GetResourceSize();
		}		

		// Log sorted summary.
		Ar.Logf(TEXT("%5i KByte of static meshes referenced by particle systems:"),TotalSize / 1024);
		for( INT StaticMeshIndex=0; StaticMeshIndex<UniqueReferencedMeshes.Num(); StaticMeshIndex++ )
		{
			UStaticMesh* StaticMesh	= UniqueReferencedMeshes(StaticMeshIndex);
			
			// Find all particle systems using this static mesh.
			TArray<UParticleSystem*> ParticleSystems;
			StaticMeshToParticleSystemMap.MultiFind( StaticMesh, ParticleSystems );

			// Log meshes including resource size and referencing particle systems.
			Ar.Logf(TEXT("%5i KByte  %s"), StaticMesh->GetResourceSize() / 1024, *StaticMesh->GetFullName());
			for( INT ParticleSystemIndex=0; ParticleSystemIndex<ParticleSystems.Num(); ParticleSystemIndex++ )
			{
				UParticleSystem* ParticleSystem = ParticleSystems(ParticleSystemIndex);
				Ar.Logf(TEXT("             %s"),*ParticleSystem->GetFullName());
			}
		}
		
		return TRUE;
	}
	else if( ParseCommand( &Cmd, TEXT("PARTICLEMEMORY") ) )
	{
		//  Gather various object resource sizes.
		INT		PSysCompCount			= 0;
		INT		ParticlePeakSize		= 0;
		INT		ParticleActiveSize		= 0;
		INT		ParticleMaxPeakSize		= 0;	// The single PSysComp that consumes the most memory
		FString ParticleMaxPeakSizeComp;

		for( TObjectIterator<UParticleSystemComponent> It; It; ++It )
		{
			UParticleSystemComponent* CheckPSysComp = *It;
			PSysCompCount++;

			FArchiveCountMem CountBytesSize(CheckPSysComp);
			INT	CheckParticleMaxPeakSize = CountBytesSize.GetMax();
			if (CheckParticleMaxPeakSize > ParticleMaxPeakSize)
			{
				ParticleMaxPeakSize	= CheckParticleMaxPeakSize;
				if (CheckPSysComp)
				{
					if (CheckPSysComp->Template)
					{
                        ParticleMaxPeakSizeComp = CheckPSysComp->Template->GetPathName();
					}
					else
					{
						ParticleMaxPeakSizeComp = FString(TEXT("No Template!"));
					}
				}
				else
				{
					ParticleMaxPeakSizeComp = FString(TEXT("No Component!"));
				}
			}
			ParticlePeakSize    += CheckParticleMaxPeakSize;
			ParticleActiveSize += CountBytesSize.GetNum();
		}

		// Gather malloc stats.
		Ar.Logf( TEXT("PSysComp Count     : %6i"), PSysCompCount );
		Ar.Logf( TEXT("Particles (Active) : %6i"), ParticleActiveSize / 1024 );
		Ar.Logf( TEXT("Particles (Peak)   : %6i"), ParticlePeakSize / 1024 );
		Ar.Logf( TEXT("Particles (Single) : %6i"), ParticleMaxPeakSize / 1024 );
		Ar.Logf( TEXT("    PSys           : %s"), *ParticleMaxPeakSizeComp );

		return TRUE;
	}
	else if( ParseCommand( &Cmd, TEXT("DUMPPARTICLECOUNTS") ) )
	{
		TMap<UParticleSystem*, ParticleSystemUsage> UsageMap;

		UBOOL bTrackUsage = ParseCommand(&Cmd, TEXT("USAGE"));
		UBOOL bTrackUsageOnly = ParseCommand(&Cmd, TEXT("USAGEONLY"));
		for( TObjectIterator<UObject> It; It; ++It )
		{
			UParticleSystemComponent* PSysComp = Cast<UParticleSystemComponent>(*It);
			if (PSysComp)
			{
				ParticleSystemUsage* Usage = NULL;

				if (bTrackUsageOnly == FALSE)
				{
					debugf(NAME_DevParticle, TEXT("ParticleSystemComponent %s"), *(PSysComp->GetName()));
				}

				UParticleSystem* PSysTemplate = PSysComp->Template;
				if (PSysTemplate != NULL)
				{
					if (bTrackUsage || bTrackUsageOnly)
					{
						ParticleSystemUsage* pUsage = UsageMap.Find(PSysTemplate);
						if (pUsage == NULL)
						{
							ParticleSystemUsage TempUsage;
							TempUsage.Template = PSysTemplate;
							TempUsage.Count = 1;

							UsageMap.Set(PSysTemplate, TempUsage);
							Usage = UsageMap.Find(PSysTemplate);
							check(Usage);
						}					
						else
						{
							Usage = pUsage;
							Usage->Count++;
						}
					}
					if (bTrackUsageOnly == FALSE)
					{
						debugf(NAME_DevParticle, TEXT("\tTemplate         : %s"), *(PSysTemplate->GetPathName()));
					}
				}
				else
				{
					if (bTrackUsageOnly == FALSE)
					{
						debugf(NAME_DevParticle, TEXT("\tTemplate         : %s"), TEXT("NULL"));
					}
				}
				
				// Dump each emitter
				INT TotalActiveCount = 0;
				if (bTrackUsageOnly == FALSE)
				{
					debugf(NAME_DevParticle, TEXT("\tEmitterCount     : %d"), PSysComp->EmitterInstances.Num());
				}

				if (PSysComp->EmitterInstances.Num() > 0)
				{
					for (INT EmitterIndex = 0; EmitterIndex < PSysComp->EmitterInstances.Num(); EmitterIndex++)
					{
						FParticleEmitterInstance* EmitInst = PSysComp->EmitterInstances(EmitterIndex);
						if (EmitInst)
						{
							UParticleLODLevel* LODLevel = EmitInst->SpriteTemplate ? EmitInst->SpriteTemplate->LODLevels(0) : NULL;
							if (bTrackUsageOnly == FALSE)
							{
								debugf(NAME_DevParticle, TEXT("\t\tEmitter %2d:\tActive = %4d\tMaxActive = %4d"), 
									EmitterIndex, EmitInst->ActiveParticles, EmitInst->MaxActiveParticles);
							}
							TotalActiveCount += EmitInst->MaxActiveParticles;
							if (bTrackUsage || bTrackUsageOnly)
							{
								check(Usage);
								Usage->ActiveTotal += EmitInst->ActiveParticles;
								Usage->MaxActiveTotal += EmitInst->MaxActiveParticles;
								Usage->StoredMaxActiveTotal += EmitInst->MaxActiveParticles;
								if (Usage->EmitterActiveTotal.Num() <= EmitterIndex)
								{
									INT CheckIndex;
									CheckIndex = Usage->EmitterActiveTotal.AddZeroed(1);
									check(CheckIndex == EmitterIndex);
									CheckIndex = Usage->EmitterMaxActiveTotal.AddZeroed(1);
									check(CheckIndex == EmitterIndex);
				                    CheckIndex = Usage->EmitterStoredMaxActiveTotal.AddZeroed(1);
									check(CheckIndex == EmitterIndex);
								}
								Usage->EmitterActiveTotal(EmitterIndex) = Usage->EmitterActiveTotal(EmitterIndex) + EmitInst->ActiveParticles;
								Usage->EmitterMaxActiveTotal(EmitterIndex) = Usage->EmitterMaxActiveTotal(EmitterIndex) + EmitInst->MaxActiveParticles;
			                    Usage->EmitterStoredMaxActiveTotal(EmitterIndex) = Usage->EmitterStoredMaxActiveTotal(EmitterIndex) + EmitInst->MaxActiveParticles;
							}
						}
						else
						{
							if (bTrackUsageOnly == FALSE)
							{
								debugf(NAME_DevParticle, TEXT("\t\tEmitter %2d:\tActive = %4d\tMaxActive = %4d"), EmitterIndex, 0, 0);
							}
						}
					}
				}
				else
				if (PSysTemplate != NULL)
				{
					for (INT EmitterIndex = 0; EmitterIndex < PSysTemplate->Emitters.Num(); EmitterIndex++)
					{
						UParticleEmitter* Emitter = PSysTemplate->Emitters(EmitterIndex);
						if (Emitter)
						{
							INT MaxActive = 0;

							for (INT LODIndex = 0; LODIndex < Emitter->LODLevels.Num(); LODIndex++)
							{
                                UParticleLODLevel* LODLevel = Emitter->LODLevels(LODIndex);
								if (LODLevel)
								{
									if (LODLevel->PeakActiveParticles > MaxActive)
									{
										MaxActive = LODLevel->PeakActiveParticles;
									}
								}
							}

							if (bTrackUsage || bTrackUsageOnly)
							{
								check(Usage);
								Usage->StoredMaxActiveTotal += MaxActive;
								if (Usage->EmitterStoredMaxActiveTotal.Num() <= EmitterIndex)
								{
									INT CheckIndex;
									CheckIndex = Usage->EmitterActiveTotal.AddZeroed(1);
									check(CheckIndex == EmitterIndex);
									CheckIndex = Usage->EmitterMaxActiveTotal.AddZeroed(1);
									check(CheckIndex == EmitterIndex);
				                    CheckIndex = Usage->EmitterStoredMaxActiveTotal.AddZeroed(1);
									check(CheckIndex == EmitterIndex);
								}
								// Don't update the non-stored entries...
			                    Usage->EmitterStoredMaxActiveTotal(EmitterIndex) = Usage->EmitterStoredMaxActiveTotal(EmitterIndex) + MaxActive;
							}
						}
					}
				}
				if (bTrackUsageOnly == FALSE)
				{
					debugf(NAME_DevParticle, TEXT("\tTotalActiveCount : %d"), TotalActiveCount);
				}
			}
		}

		if (bTrackUsage || bTrackUsageOnly)
		{
			debugf(NAME_DevParticle, TEXT("PARTICLE USAGE DUMP:"));
			for (TMap<UParticleSystem*, ParticleSystemUsage>::TIterator It(UsageMap); It; ++It)
			{
				ParticleSystemUsage& Usage = It.Value();
				UParticleSystem* Template = Usage.Template;
				check(Template);

				debugf(NAME_DevParticle, TEXT("\tParticleSystem..%s"), *(Usage.Template->GetPathName()));
				debugf(NAME_DevParticle, TEXT("\t\tCount.....................%d"), Usage.Count);
				debugf(NAME_DevParticle, TEXT("\t\tActiveTotal...............%5d"), Usage.ActiveTotal);
				debugf(NAME_DevParticle, TEXT("\t\tMaxActiveTotal............%5d (%4d per instance)"), Usage.MaxActiveTotal, (Usage.MaxActiveTotal / Usage.Count));
				debugf(NAME_DevParticle, TEXT("\t\tPotentialMaxActiveTotal...%5d (%4d per instance)"), Usage.StoredMaxActiveTotal, (Usage.StoredMaxActiveTotal / Usage.Count));
				debugf(NAME_DevParticle, TEXT("\t\tEmitters..................%d"), Usage.EmitterActiveTotal.Num());
				check(Usage.EmitterActiveTotal.Num() == Usage.EmitterMaxActiveTotal.Num());
				for (INT EmitterIndex = 0; EmitterIndex < Usage.EmitterActiveTotal.Num(); EmitterIndex++)
				{
					INT EActiveTotal = Usage.EmitterActiveTotal(EmitterIndex);
					INT EMaxActiveTotal = Usage.EmitterMaxActiveTotal(EmitterIndex);
					INT EStoredMaxActiveTotal = Usage.EmitterStoredMaxActiveTotal(EmitterIndex);
					debugf(NAME_DevParticle, TEXT("\t\t\tEmitter %2d - AT = %5d, MT = %5d (%4d per emitter), Potential MT = %5d (%4d per emitter)"),
						EmitterIndex, EActiveTotal,
						EMaxActiveTotal, (EMaxActiveTotal / Usage.Count),
						EStoredMaxActiveTotal, (EStoredMaxActiveTotal / Usage.Count)
						);
				}
			}
		}
		return TRUE;
	}
	// This will look at all static meshes loaded and print out whether they have collision or not.  This is useful for checking to see which meshes have collision when it is not needed (e.g. they are doing:  CollisionComponent=CollisionCylinder )
	else if( ParseCommand( &Cmd, TEXT("MeshesWithCollision") ) )
	{
		for( FObjectIterator It; It; ++It )
		{
			UStaticMesh* SM = Cast<UStaticMesh>( *It );
			if( SM != NULL )
			{
				if( SM->BodySetup != NULL )
				{
					debugf( TEXT( "Collision: %s" ), *It->GetFullName() );
				}
				else
				{
					debugf( TEXT( "No Collision: %s" ), *It->GetFullName() );
				}
			}
		}

		return TRUE;
	}
	else if( GWorld && GWorld->Exec( Cmd, Ar ) )
	{
		return TRUE;
	}
	else if( GWorld && GWorld->GetGameInfo() && GWorld->GetGameInfo()->ScriptConsoleExec(Cmd,Ar,NULL) )
	{
		return TRUE;
	}
	else
	{
#if SHIPPING_PC_GAME
		// disallow set of actor properties if network game
		if ((ParseCommand(&Str, TEXT("SET")) || ParseCommand(&Str, TEXT("SETNOPEC"))))
		{
			if (GPendingLevel != NULL || GWorld->GetNetMode() != NM_Standalone)
			{
				return TRUE;
			}
			// the effects of this cannot be easily reversed, so prevent the user from playing network games without restarting to avoid potential exploits
			GDisallowNetworkTravel = TRUE;
		}
#endif //SHIPPING_PC_GAME		
		if (UEngine::Exec(Cmd, Ar))
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
}

/*-----------------------------------------------------------------------------
	Game entering.
-----------------------------------------------------------------------------*/
//
// Cancel pending level.
//
void UGameEngine::CancelPending()
{
	if( GPendingLevel )
	{
		if( GPendingLevel->NetDriver && GPendingLevel->NetDriver->ServerConnection && GPendingLevel->NetDriver->ServerConnection->Channels[0] )
		{
			GPendingLevel->NetDriver->ServerConnection->Channels[0]->Close();
			GPendingLevel->NetDriver->ServerConnection->FlushNet();
		}
		GPendingLevel = NULL;
	}
}

//
// Browse to a specified URL, relative to the current one.
//
UBOOL UGameEngine::Browse( FURL URL, FString& Error )
{
	Error = TEXT("");
	TravelURL = TEXT("");
	const TCHAR* Option;

	// Convert .unreal link files.
	const TCHAR* LinkStr = TEXT(".unreal");//!!
	if( appStrstr(*URL.Map,LinkStr)-*URL.Map==appStrlen(*URL.Map)-appStrlen(LinkStr) )
	{
		debugf( TEXT("Link: %s"), *URL.Map );
		FString NewUrlString;
		if( GConfig->GetString( TEXT("Link")/*!!*/, TEXT("Server"), NewUrlString, *URL.Map ) )
		{
			// Go to link.
			URL = FURL( NULL, *NewUrlString, TRAVEL_Absolute );//!!
		}
		else
		{
			// Invalid link.
			Error = FString::Printf( LocalizeSecure(LocalizeError(TEXT("InvalidLink"),TEXT("Engine")), *URL.Map) );
			return FALSE;
		}
	}

	// Crack the URL.
	debugf( TEXT("Browse: %s"), *URL.String() );

	// Handle it.
	if( !URL.Valid )
	{
		// Unknown URL.
		Error = FString::Printf( LocalizeSecure(LocalizeError(TEXT("InvalidUrl"),TEXT("Engine")), *URL.String()) );
		return FALSE;
	}
	else if (URL.HasOption(TEXT("failed")) || URL.HasOption(TEXT("closed")))
	{
		UBOOL bHadPendingLevel = FALSE;
		if (GPendingLevel)
		{
			bHadPendingLevel = TRUE;
			CancelPending();
		}
		// Handle failure URL.
		debugf( NAME_Log, *LocalizeError(TEXT("AbortToEntry"),TEXT("Engine")) );
		if (GWorld != NULL)
		{
			ResetLoaders( GWorld->GetOuter() );
		}
		LoadMap( FURL(&URL,*FURL::DefaultLocalMap,TRAVEL_Partial), NULL, Error );
		CollectGarbage( GARBAGE_COLLECTION_KEEPFLAGS );
		if( URL.HasOption(TEXT("failed")) )
		{
			if( !bHadPendingLevel )
			{
				SetProgress( PMT_ConnectionFailure, LocalizeError(TEXT("ConnectionFailed_Title"), TEXT("Engine")), LocalizeError(TEXT("ConnectionFailed"),TEXT("Engine")) );
			}
		}

		// now remove "failed" and "closed" options from LastURL so it doesn't get copied on to future URLs
		LastURL.RemoveOption(TEXT("failed"));
		LastURL.RemoveOption(TEXT("closed"));
		return TRUE;
	}
	else if( URL.HasOption(TEXT("restart")) )
	{
		// Handle restarting.
		URL = LastURL;
	}
	else if( (Option=URL.GetOption(TEXT("load="),NULL))!=NULL )
	{
		// Handle loadgame.
		FString Error2, Temp=FString::Printf( TEXT("%s") PATH_SEPARATOR TEXT("Save%i.usa?load"), *GSys->SavePath, appAtoi(Option) );
		debugf(TEXT("Loading save game %s"),*Temp);
		if( LoadMap(FURL(&LastURL,*Temp,TRAVEL_Partial),NULL,Error2) )
		{
			LastURL = GWorld->URL;
			return TRUE;
		}
		else return FALSE;
	}

	// Handle normal URL's.
	if (GDisallowNetworkTravel && URL.HasOption(TEXT("listen")))
	{
#if OLD_CONNECTION_FAILURE_CODE
		SetProgress(TEXT("Networking Failed"), TEXT("CheatCommandsUsed"), 6.f);
#else
		SetProgress( PMT_ConnectionFailure, LocalizeError(TEXT("CreateMatchError"), TEXT("Engine")), LocalizeError(TEXT("UsedCheatCommands"), TEXT("Engine")) );
#endif

		return FALSE;
	}
	if( URL.IsLocalInternal() )
	{
		// Local map file.
		return LoadMap( URL, NULL, Error );
	}
	else if( URL.IsInternal() && GIsClient )
	{
		// Force a demo stop on level change.
		if (GWorld && GWorld->DemoRecDriver != NULL)
		{
			GWorld->DemoRecDriver->Exec( TEXT("DEMOSTOP"), *GLog );
		}

		// Network URL.
		if( GPendingLevel )
		{
			CancelPending();
		}

		// Clean up the socket so that the pending level succeeds
		if (GWorld)
		{
			UNetDriver* NetDriver = GWorld->GetNetDriver();
			if (NetDriver)
			{
				if (NetDriver->ServerConnection)
				{
					NetDriver->ServerConnection->Close();
					NetDriver->ServerConnection->FlushNet();
				}

				NetDriver->LowLevelDestroy();
			}
			GWorld->SetNetDriver(NULL);
		}

		GPendingLevel = new UNetPendingLevel( URL );
		if( !GPendingLevel->NetDriver )
		{
			// UNetPendingLevel will set the appropriate error code and connectionlost type, so
			// we just have to propagate that message to the game.
#if OLD_CONNECTION_FAILURE_CODE
			SetProgress( TEXT("Networking Failed"), *GPendingLevel->ConnectionError, 6.f );
#else
			SetProgress( PMT_ConnectionFailure, LocalizeError(TEXT("NetworkInit"), TEXT("Engine")), GPendingLevel->ConnectionError );
#endif
			GPendingLevel = NULL;
		}
		return FALSE;
	}
	else if( URL.IsInternal() )
	{
		// Invalid.
		Error = LocalizeError(TEXT("ServerOpen"),TEXT("Engine"));
		return FALSE;
	}
	else
	{
		// External URL.
		//Client->Viewports(0)->Exec(TEXT("ENDFULLSCREEN"));
		appLaunchURL( *URL.String(), TEXT(""), &Error );
		return FALSE;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MPSounds_PackageName		TEXT("MPSounds")
#define MPSounds_ReferencerName		MPSounds_PackageName TEXT("ObjRef")

/** Enum entries represent index to global object referencer stored in UGameEngine */
enum EMPContentReferencerTypes
{
	MPSounds_ReferencerIndex=0,
	MPSounds_LocalizedReferencerIndex,
	MPGameContent_ReferencerIndex,
	MPGameContent_LocalizedReferencerIndex,
	MAX_ReferencerIndex
};

/**
* Finds object referencer in the content package and sets it in the global referencer list
*
* @param MPContentPackage - content package which has an obj referencer
* @param GameEngine - current game engine instance
* @param ContentType - EMPContentReferencerTypes entry for global content package type
*/
static void SetMPContentObjectReferencers(UObject* MPContentPackage,UGameEngine* GameEngine,EMPContentReferencerTypes ContentType)
{
	check(GameEngine == GEngine);

	// Make sure to allocate enough referencer entries
	if ( GameEngine->ObjectReferencers.Num() < MAX_ReferencerIndex )
	{
		GameEngine->ObjectReferencers.AddZeroed( MAX_ReferencerIndex );
	}
	// Release any previous object referencer
	GameEngine->ObjectReferencers(ContentType) = NULL;

	if( MPContentPackage )
	{	
		// Find the object referencer in the content package. There should only be one
		UObjectReferencer* ObjectReferencer = NULL;		
		for( TObjectIterator<UObjectReferencer> It; It; ++It )
		{
			if( It->IsIn(MPContentPackage) )
			{
				ObjectReferencer = *It;
				break;
			}
		}
		// Keep a reference to it in the game engine
		if( ObjectReferencer )
		{
			GameEngine->ObjectReferencers(ContentType) = ObjectReferencer;
		}
		else
		{
			debugf( NAME_Warning, TEXT("MPContentObjectReferencers: Couldn't find object referencer in %s"), 
				*MPContentPackage->GetPathName() );
		}
	}
	else
	{
		debugf( NAME_Warning, TEXT("MPContentObjectReferencers: package load failed") );
	}
}

/**
* Callback function for when the localized MP sounds package is loaded.
*
* @param	ContentPackage		The package that was loaded.
* @param	GameEngine			The GameEngine.
*/
static void AsyncLoadLocalizedMPSoundsCompletionCallback(UObject* ContentPackage, void* InGameEngine)
{
	SetMPContentObjectReferencers(ContentPackage,(UGameEngine*)InGameEngine,MPSounds_LocalizedReferencerIndex);
}

/**
* Callback function for when the MP sounds package is loaded.
*
* @param	ContentPackage		The package that was loaded.
* @param	GameEngine			The GameEngine.
*/
static void AsyncLoadMPSoundsCompletionCallback(UObject* ContentPackage, void* InGameEngine)
{
	SetMPContentObjectReferencers(ContentPackage,(UGameEngine*)InGameEngine,MPSounds_ReferencerIndex);
}

/**
* Releases any existing handles to MP sounds.
*/
void FreeMPSounds(UEngine* InGameEngine)
{
	debugf( TEXT("Freeing MPSounds") );

	UGameEngine* GameEngine = Cast<UGameEngine>( InGameEngine );
	check( GameEngine );

	if ( GameEngine->ObjectReferencers.Num() > 0 )
	{
		GameEngine->ObjectReferencers(MPSounds_ReferencerIndex) = NULL;
		GameEngine->ObjectReferencers(MPSounds_LocalizedReferencerIndex) = NULL;
	}
}

/**
* Loads the multiplayer sound packages.
*/
void LoadMPSounds(UEngine* GameEngine)
{
	// Release any existing handles to MP sounds.
	FreeMPSounds( GEngine );

	// Load localized MP sounds.
	const TCHAR* Language = UObject::GetLanguage();
	const FString LocalizedPreloadName( FString(MPSounds_PackageName) + LOCALIZED_SEEKFREE_SUFFIX + TEXT("_") + Language );

	FString LocalizedPreloadFilename;
	if( GPackageFileCache->FindPackageFile( *LocalizedPreloadName, NULL, LocalizedPreloadFilename ) )
	{
		debugf( TEXT("Issuing preload for %s"), *LocalizedPreloadFilename );
		UObject::LoadPackageAsync( LocalizedPreloadFilename, AsyncLoadLocalizedMPSoundsCompletionCallback, GameEngine );
	}
	else
	{
		debugf( TEXT("Couldn't find localized MP sound package %s"), *LocalizedPreloadName );
	}

	// Load non-localized MP sounds.
	const FString PreloadName( MPSounds_PackageName );
	FString PreloadFilename;
	if( GPackageFileCache->FindPackageFile( *PreloadName, NULL, PreloadFilename ) )
	{
		debugf( TEXT("Issuing preload for %s"), *PreloadFilename );
		UObject::LoadPackageAsync( PreloadFilename, AsyncLoadMPSoundsCompletionCallback, GameEngine );
	}
	else
	{
		debugf( TEXT("Couldn't find MP sound package %s"), *PreloadName );
	}
}

/**
* Callback function for when the localized MP game package is loaded.
*
* @param	ContentPackage		The package that was loaded.
* @param	GameEngine			The GameEngine.
*/
static void AsyncLoadLocalizedMapGameTypeContentCallback(UObject* ContentPackage, void* InGameEngine)
{
	SetMPContentObjectReferencers(ContentPackage,(UGameEngine*)InGameEngine,MPGameContent_LocalizedReferencerIndex);
}

/**
* Callback function for when the MP game package is loaded.
*
* @param	ContentPackage		The package that was loaded.
* @param	GameEngine			The GameEngine.
*/
static void AsyncLoadMapGameTypeContentCallback(UObject* ContentPackage, void* InGameEngine)
{
	SetMPContentObjectReferencers(ContentPackage,(UGameEngine*)InGameEngine,MPGameContent_ReferencerIndex);
}

/**
* Parse game type from URL and return standalone seekfree package name for it
*
* @param URL - current URL containing map and game type we are browsing to
*/
FString GetMPGameContentPackageStr(const FURL& URL)
{
	static const FString GAME_CONTENT_PKG_PREFIX(TEXT(""));

	// get game from URL
	FString GameTypeClassName( URL.GetOption(TEXT("Game="), TEXT("")) );
	if (GameTypeClassName == TEXT(""))
	{
		// ask the default gametype what we should use
		UClass* DefaultGameClass = UObject::StaticLoadClass(AGameInfo::StaticClass(), NULL, TEXT("game-ini:Engine.GameInfo.DefaultGame"), NULL, LOAD_None, NULL);
        if (DefaultGameClass != NULL)
		{
			FString Options(TEXT(""));
			for (INT i = 0; i < URL.Op.Num(); i++)
			{
				Options += TEXT("?");
				Options += URL.Op(i);
			}
			GameTypeClassName = DefaultGameClass->GetDefaultObject<AGameInfo>()->eventGetDefaultGameClassPath(URL.Map, Options, *URL.Portal);
		}
	}
	// parse game class from full path
	INT FoundIdx = FString(GameTypeClassName).InStr(TEXT("."),TRUE);
	FString GameClassStr = FString(GameTypeClassName).Right(FString(GameTypeClassName).Len()-1 - FoundIdx);
	
	return GAME_CONTENT_PKG_PREFIX + GameClassStr + FString(TEXT("_SF"));
}

/**
* Remove object referencer entries for the game content packages
*/
void FreeMPGameContent(UEngine* InGameEngine)
{
	debugf( TEXT("Freeing MP Game Content") );
	UGameEngine* GameEngine = Cast<UGameEngine>( InGameEngine );
	check( GameEngine );
	if ( GameEngine->ObjectReferencers.Num() > 0 )
	{
		GameEngine->ObjectReferencers(MPGameContent_ReferencerIndex) = NULL;
		GameEngine->ObjectReferencers(MPGameContent_LocalizedReferencerIndex) = NULL;
	}
}

/**
* Async load the game content standalone seekfree packages for the current game 
*
* @param GameEngine - current game engine instance
* @param URL - current URL containing map and game type we are browsing to
*/
void LoadMPGameContent(UEngine* GameEngine, const FURL& URL)
{
	FreeMPGameContent(GameEngine);

	const TCHAR* Language = UObject::GetLanguage();
	const FString LocalizedPreloadName( GetMPGameContentPackageStr(URL) + LOCALIZED_SEEKFREE_SUFFIX + TEXT("_") + Language );

	FString LocalizedPreloadFilename;
	if( GPackageFileCache->FindPackageFile( *LocalizedPreloadName, NULL, LocalizedPreloadFilename ) )
	{
		debugf( TEXT("Issuing preload for %s"), *LocalizedPreloadFilename );
		UObject::LoadPackageAsync( LocalizedPreloadFilename, AsyncLoadLocalizedMapGameTypeContentCallback, GameEngine );
	}
	else
	{
		debugf( TEXT("Couldn't find localized map content package %s"), *LocalizedPreloadName );
	}

	FString PreloadFilename;
	if( GPackageFileCache->FindPackageFile( *GetMPGameContentPackageStr(URL), NULL, PreloadFilename ) )
	{
		debugf( TEXT("Issuing preload for %s"), *PreloadFilename );
		UObject::LoadPackageAsync( PreloadFilename, AsyncLoadMapGameTypeContentCallback, GameEngine );
	}
	else
	{
		debugf( TEXT("Couldn't find map content package %s"), *PreloadFilename );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern UBOOL GPrecacheNextFrame;

//
// Load a map.
//
UBOOL UGameEngine::LoadMap( const FURL& URL, UPendingLevel* Pending, FString& Error )
{
	// make sure level streaming isn't frozen
	if (GWorld)
	{
		GWorld->bIsLevelStreamingFrozen = FALSE;
	}

#if PS3
	extern UBOOL GNoRecovery;
	// if we are in a no recovery state, we can't call LoadMap, as it will [b]lock
	// @todo: this may need to change if we are able to "recover from NoRecovery"
	if (GNoRecovery)
	{
		return FALSE;
	}
#endif

	// Force a demo stop on level change.
	if (GWorld && GWorld->DemoRecDriver != NULL)
	{
		GWorld->DemoRecDriver->Exec( TEXT("DEMOSTOP"), *GLog );
	}

	// send a callback message
	GCallbackEvent->Send(CALLBACK_PreLoadMap);

	// Cancel any pending texture streaming requests.  This avoids a significant delay on consoles 
	// when loading a map and there are a lot of outstanding texture streaming requests from the previous map.
	UTexture2D::CancelPendingTextureStreaming();

	// play a load map movie if specified in ini
	UBOOL bStartedLoadMapMovie = FALSE;

	// Dump and reset the FPS chart on map changes.
	DumpFPSChart();
	ResetFPSChart();

	DumpMemoryChart();
	ResetMemoryChart();

	// clean up any per-map loaded packages for the map we are leaving
	if (GWorld && GWorld->PersistentLevel)
	{
		CleanupPackagesToFullyLoad(FULLYLOAD_Map, GWorld->PersistentLevel->GetOutermost()->GetName());
	}

	// cleanup the existing per-game pacakges
	// @todo: It should be possible to not unload/load packages if we are going from/to the same gametype.
	//        would have to save the game pathname here and pass it in to SetGameInfo below
	CleanupPackagesToFullyLoad(FULLYLOAD_Game_PreLoadClass, TEXT(""));
	CleanupPackagesToFullyLoad(FULLYLOAD_Game_PostLoadClass, TEXT(""));
	CleanupPackagesToFullyLoad(FULLYLOAD_Mutator, TEXT(""));


	// Cancel any pending async map changes after flushing async loading. We flush async loading before canceling the map change
	// to avoid completion after cancelation to not leave references to the "to be changed to" level around. Async loading is
	// implicitly flushed again later on during garbage collection.
	UObject::FlushAsyncLoading();
	CancelPendingMapChange();
	GSeamlessTravelHandler.CancelTravel();

	DOUBLE	StartTime = appSeconds();

	Error = TEXT("");
	debugf( NAME_Log, TEXT("LoadMap: %s"), *URL.String() );
	GInitRunaway();

	// Get network package map.
	UPackageMap* PackageMap = NULL;
	if( Pending )
	{
		PackageMap = Pending->GetDriver()->ServerConnection->PackageMap;
	}

	if( GWorld )
	{
		// Display loading screen.
		if( !URL.HasOption(TEXT("quiet")) )
		{
			TransitionType = TT_Loading;
			TransitionDescription = URL.Map;
			if (URL.HasOption(TEXT("Game=")))
			{
				TransitionGameType = URL.GetOption(TEXT("Game="), TEXT(""));
			}
			else
			{
				TransitionGameType = TEXT("");
			}
			RedrawViewports();
			TransitionType = TT_None;
		}

		bStartedLoadMapMovie = PlayLoadMapMovie();

		// If desired, clear all AnimSet LinkupCaches.
		if(bClearAnimSetLinkupCachesOnLoadMap)
		{
			UAnimSet::ClearAllAnimSetLinkupCaches();
		}

		// Clean up game state.
		GWorld->SetNetDriver(NULL);
		GWorld->FlushLevelStreaming( NULL, TRUE );
		GWorld->TermWorldRBPhys();
		GWorld->CleanupWorld();
		
		// send a message that all levels are going away (NULL means every sublevel is being removed
		// without a call to RemoveFromWorld for each)
		GCallbackEvent->Send(CALLBACK_LevelRemovedFromWorld, (UObject*)NULL);

		// Disassociate the players from their PlayerControllers.
		for(FPlayerIterator It(this);It;++It)
		{
			if(It->Actor)
			{
				if(It->Actor->Pawn)
				{
					GWorld->DestroyActor(It->Actor->Pawn, TRUE);
				}
				GWorld->DestroyActor(It->Actor, TRUE);
				It->Actor = NULL;
			}
			// clear post process volume so it doesn't prevent the world from being unloaded
			It->CurrentPPInfo.LastVolumeUsed = NULL;
			// reset split join info so we'll send one after loading the new map if necessary
			It->bSentSplitJoin = FALSE;
		}

		// clear any "DISPLAY" properties referencing level objects
		if (GEngine->GameViewport != NULL)
		{
			for (INT i = 0; i < GEngine->GameViewport->DebugProperties.Num(); i++)
			{
				if (GEngine->GameViewport->DebugProperties(i).Obj == NULL)
				{
					GEngine->GameViewport->DebugProperties.Remove(i, 1);
					i--;
				}
				else
				{
					for (UObject* TestObj = GEngine->GameViewport->DebugProperties(i).Obj; TestObj != NULL; TestObj = TestObj->GetOuter())
					{
						if (TestObj->IsA(ULevel::StaticClass()) || TestObj->IsA(UWorld::StaticClass()) || TestObj->IsA(AActor::StaticClass()))
						{
							GEngine->GameViewport->DebugProperties.Remove(i, 1);
							i--;
							break;
						}
					}
				}
			}
		}

		GWorld->RemoveFromRoot();
		GWorld = NULL;
	}

	// Stop all audio to remove references to current level.
	if( GEngine->Client && GEngine->Client->GetAudioDevice() )
	{
		GEngine->Client->GetAudioDevice()->Flush( NULL );
		// reset transient volume
		GEngine->Client->GetAudioDevice()->TransientMasterVolume = 1.0;
	}

	// Figure out relevant data for per gametype MP packages.
	UBOOL bIsMultiplayerMap = URL.Map.StartsWith(TEXT("MP_"));
	UBOOL bSeparateSharedMPSoundContent = FALSE;
	UBOOL bSeparateSharedMPGameContent = FALSE;
	GConfig->GetBool( TEXT("Engine.Engine"), TEXT("bCookSeparateSharedMPSoundContent"), bSeparateSharedMPSoundContent, GEngineIni );
	GConfig->GetBool( TEXT("Engine.Engine"), TEXT("bCookSeparateSharedMPGameContent"), bSeparateSharedMPGameContent, GEngineIni );
	// Free any previously loaded MP specific content if we're not loading a MP map.
	if( GUseSeekFreeLoading 
	&& (bSeparateSharedMPSoundContent || bSeparateSharedMPGameContent) 
	&& !bIsMultiplayerMap )
	{
		debugf( NAME_Log, TEXT("LoadMap: %s: freeing any shared MP resources"), *URL.String() );
		FreeMPSounds(GEngine);
		FreeMPGameContent(GEngine);
	}

	// Clean up the previous level out of memory.
	UObject::CollectGarbage( GARBAGE_COLLECTION_KEEPFLAGS, TRUE );

	// remove any prestream requests from textures still in memory
	for (TObjectIterator<UTexture2D> It; It; ++It)
	{
		if (It->ForceMipLevelsToBeResidentTimestamp > 0.0f)
		{
			It->SetForceMipLevelsToBeResident(-1.0f);
		}
	}

#if CONSOLE
	appDefragmentTexturePool();
	appDumpTextureMemoryStats(TEXT(""));
#endif

#if !FINAL_RELEASE || FINAL_RELEASE_DEBUGCONSOLE
	// Dump info
	Exec(TEXT("MEM"));
#endif

#if !FINAL_RELEASE
	// There should be no UWorld instances at this point!
	for( TObjectIterator<UWorld> It; It; ++It )
	{
		UWorld* World = *It;
		// Print some debug information...
		debugf(TEXT("%s not cleaned up by garbage collection! "), *World->GetFullName());
		UObject::StaticExec(*FString::Printf(TEXT("OBJ REFS CLASS=WORLD NAME=%s.TheWorld"), *World->GetOutermost()->GetName()));
		TMap<UObject*,UProperty*>	Route		= FArchiveTraceRoute::FindShortestRootPath( World, TRUE, GARBAGE_COLLECTION_KEEPFLAGS );
		FString						ErrorString	= FArchiveTraceRoute::PrintRootPath( Route, World );
		debugf(TEXT("%s"),*ErrorString);
		// before asserting.
		appErrorf( TEXT("%s not cleaned up by garbage collection!") LINE_TERMINATOR TEXT("%s") , *World->GetFullName(), *ErrorString );
	}
#endif

	if( GUseSeekFreeLoading )
	{
		// Load MP specific data.
		if( (bSeparateSharedMPSoundContent || bSeparateSharedMPGameContent) && bIsMultiplayerMap )
		{			
			debugf( NAME_Log, TEXT("LoadMap: %s: issuing load request for shared MP resources"), *URL.String() );
			if( bSeparateSharedMPSoundContent )
			{
				LoadMPSounds(GEngine);
			}
			if( bSeparateSharedMPGameContent )
			{
				LoadMPGameContent(GEngine,URL);
			}
		}

		// Load localized part of level first in case it exists.
		FString LocalizedMapPackageName	= URL.Map + LOCALIZED_SEEKFREE_SUFFIX;
		FString LocalizedMapFilename;
		if( GPackageFileCache->FindPackageFile( *LocalizedMapPackageName, NULL, LocalizedMapFilename ) )
		{
			LoadPackage( NULL, *LocalizedMapPackageName, LOAD_NoWarn );
		}
	}

	UPackage* MapOuter = NULL;
	// in the seekfree case (which hasn't already loaded anything), get linkers for any downloaded packages here,
	// so that any dependenant packages will correctly find them as they will not search the cache by default
	if (Pending && Pending->NetDriver && Pending->NetDriver->ServerConnection)
	{
		// cache map name
		FName MapName(*GPendingLevel->URL.Map);

		// first, look in the package map to get the guid for the map, so we can open by guid for package downloading
		for (INT PackageIndex = 0; PackageIndex < Pending->NetDriver->ServerConnection->PackageMap->List.Num(); PackageIndex++)
		{
			FPackageInfo& Info = Pending->NetDriver->ServerConnection->PackageMap->List(PackageIndex);

			// is this package entry the map?
			if (MapName == Info.PackageName)
			{
				// if we haven't loaded the map yet (seekfree loading, for instance), load it now
				// using the Guid, so we can open a downloaded map
				if (Info.Parent == NULL || !GUseSeekFreeLoading)
				{
					// make the package, and use this for the new linker (and to load the map from)
					MapOuter = CreatePackage(NULL, *GPendingLevel->URL.Map);

					// create the linker with the map name, and use the Guid so we find the downloaded version
					BeginLoad();
					GetPackageLinker(MapOuter, NULL, LOAD_NoWarn | LOAD_NoVerify | LOAD_Quiet, NULL, &Info.Guid);
					EndLoad();
				}
			}
			else if (Info.Parent == NULL)
			{
				FString PackageName = Info.PackageName.ToString();
				FString RealFilename;
				if (GPackageFileCache->FindPackageFile(*PackageName, &Info.Guid, RealFilename) && RealFilename.StartsWith(GSys->CachePath))
				{
					BeginLoad();
					UPackage* NewPackage = CreatePackage(NULL, *PackageName);
					UObject* Linker = GetPackageLinker(NewPackage, *PackageName, LOAD_NoWarn | LOAD_NoVerify, NULL, &Info.Guid);
					EndLoad();
				}
			}
		}
	}

	// Load level.
	UPackage* WorldPackage = LoadPackage(MapOuter, *URL.Map, LOAD_None);
	if( WorldPackage == NULL )
	{
		// it is now the responsibility of the caller to deal with a NULL return value and alert the user if necessary
		//@todo ronp connection
		Error = FString::Printf(TEXT("Failed to load package '%s'"), *URL.Map);
		return FALSE;
	}

#if CONSOLE
	if( GUseSeekFreeLoading && !(WorldPackage->PackageFlags & PKG_DisallowLazyLoading) )
	{
		appErrorf(TEXT("Map '%s' has not been cooked correctly! Most likely stale version on the XDK."),*WorldPackage->GetName());
	}
#endif

	GWorld = FindObjectChecked<UWorld>( WorldPackage, TEXT("TheWorld") );
	GWorld->AddToRoot();
	GWorld->Init();

	// If pending network level.
	if( Pending )
	{
		// Setup network package info.
		if (GUseSeekFreeLoading && PackageMap->List.Num() > 0)
		{
			// verify that we loaded everything we need here after loading packages required by the map itself to minimize extraneous loading/seeks
			// first, attempt to find or load any packages required, then try to find any we couldn't find the first time again (to catch forced exports inside packages loaded by the first loop)
			for (INT PackageIndex = 0, Pass = 0; Pass < 2; PackageIndex++)
			{
				FPackageInfo& Info = PackageMap->List(PackageIndex);
				UPackage* Package = Info.Parent;

				// ignore forced export packages in the first pass
				if (Package == NULL && (Info.ForcedExportBasePackageName == NAME_None || Pass > 0))
				{
					// cache the package's name
					FString PackageName = Info.PackageName.ToString();

					// attempt to find it
					Package = FindPackage(NULL, *PackageName);
					if (Package == NULL)
					{
						// if that fails, load it
						// skip files server said had a base package as that means they are forced exports and we shouldn't attempt to load the "real" package file (if it even exists)
						if (Info.ForcedExportBasePackageName == NAME_None)
						{
							// check for localized package first as that may be required to load the base package
							FString LocalizedPackageName = PackageName + LOCALIZED_SEEKFREE_SUFFIX;
							FString LocalizedFileName;
							if (GPackageFileCache->FindPackageFile(*LocalizedPackageName, NULL, LocalizedFileName))
							{
								LoadPackage(NULL, *LocalizedPackageName, LOAD_None);
							}
							// load the base package
							Package = LoadPackage(NULL, *PackageName, LOAD_None);
						}

						if (Package == NULL && Info.ForcedExportBasePackageName != NAME_None)
						{
							// this package is a forced export inside another package, so try loading that other package
							FString BasePackageNameString(Info.ForcedExportBasePackageName.ToString());
							FString FileName;
							// don't check the Guid because we don't know the Guid of the base package, and it will have
							// been already downloaded. This can break in the case of downloading partial pieces of a cooked package
							if (GPackageFileCache->FindPackageFile(*BasePackageNameString, NULL, FileName))
							{
								debugf(NAME_DevNet, TEXT("Fully loading base package %s to get reference to internal package %s"), *BasePackageNameString, *PackageName);

								// find the guid of the localized base package if it exists
								/* @FIXME: maybe use this to get GUID, otherwise use NULL?
								INT BaseLocPackageIndex = INDEX_NONE;
								FName BaseLocPackageName = FName(*(BasePackageNameString + LOCALIZED_SEEKFREE_SUFFIX));
								for (INT PackageIndex3 = 0; PackageIndex3 < PackageMap->List.Num(); PackageIndex3++)
								{
									if (PackageMap->List(PackageIndex3).PackageName == BaseLocPackageName)
									{
										BaseLocPackageIndex = PackageIndex3;
										break;
									}
								}

								// if the localized base package exists in the package map, load it
								if (BaseLocPackageIndex != INDEX_NONE)
								{
									// check for localized package first as that may be required to load the base package
									FString LocalizedFileName;
									if (GPackageFileCache->FindPackageFile(*BaseLocPackageName.ToString(), NULL, LocalizedFileName))
									{
										LoadPackage(NULL, *LocalizedFileName, LOAD_None);
									}
								}
								*/
								FString LocalizedBasePackageName = BasePackageNameString + LOCALIZED_SEEKFREE_SUFFIX;
								FString LocalizedFilename;
								if (GPackageFileCache->FindPackageFile(*LocalizedBasePackageName, NULL, LocalizedFilename))
								{
									LoadPackage(NULL, *LocalizedBasePackageName, LOAD_None);
								}
								// load base package
								LoadPackage(NULL, *BasePackageNameString, LOAD_None);
								// now try to find the original package
								Package = FindPackage(NULL, *PackageName);
							}
						}
					}

					// if we have a valid package
					if (Package != NULL)
					{
						// check GUID
						if (Package->GetGuid() != Info.Guid)
						{
							Error = FString::Printf(TEXT("Package '%s' version mismatch [package %s, info %s]"), *Package->GetName(), *Package->GetGuid().String(), *Info.Guid.String());
							debugf(NAME_Error, *Error);
							return FALSE;
						}
						// record info in package map
						Info.Parent = Package;
						Info.LocalGeneration = Package->GetGenerationNetObjectCount().Num();
						if (Info.LocalGeneration < Info.RemoteGeneration && Cast<UDemoPlayPendingLevel>(Pending) != NULL)
						{
							// the indices will be mismatched in this case as there's no real server to adjust them for our older package version
							Error = FString::Printf(TEXT("Package '%s' version mismatch"), *Info.PackageName.ToString());
							debugf(NAME_Error, *Error);
							return FALSE;
						}
						// tell the server what we have
						Pending->GetDriver()->ServerConnection->Logf(TEXT("HAVE GUID=%s GEN=%i"), *Info.Guid.String(), Info.LocalGeneration);
					}
					else
					{
						//@todo ronp connection
						Error = FString::Printf(TEXT("Required package '%s' not found (download was already attempted)"), *Info.PackageName.ToString());
						debugf(NAME_Error, *Error);
						return FALSE;
					}
				}

				// if we reached the end, start the next pass
				if (PackageIndex == PackageMap->List.Num() - 1)
				{
					Pass++;
					PackageIndex = -1;
				}
			}
		}

		PackageMap->Compute();
	}

	// Handle pending level.
	if( Pending )
	{
		check(Pending==GPendingLevel);

		// Hook network driver up to level.
		GWorld->SetNetDriver(Pending->NetDriver);
		if( GWorld->GetNetDriver() )
		{
			GWorld->GetNetDriver()->Notify = GWorld;
			UPackage::NetObjectNotifies.AddItem(GWorld->GetNetDriver());
		}

		// Hook up the DemoRecDriver from the pending level
		GWorld->DemoRecDriver = Pending->DemoRecDriver;
		if (GWorld->DemoRecDriver)
		{
			GWorld->DemoRecDriver->Notify = GWorld;
		}

		// Setup level.
		GWorld->GetWorldInfo()->NetMode = NM_Client;
	}
	else
	{
		check(!GWorld->GetNetDriver());
	}

	// GEMINI_TODO: A nicer precaching scheme.
	GPrecacheNextFrame = TRUE;

	GWorld->SetGameInfo(URL);

	// Listen for clients.
	UBOOL bHadListenError = FALSE;
	if( !Client || URL.HasOption(TEXT("Listen")) )
	{
		if( GPendingLevel )
		{
			check(!Pending);
			GPendingLevel = NULL;
		}
		FString Error2;
		if (!GWorld->Listen(URL, Error2))
		{
			// Log why the listen couldn't happen
			//Error = FString::Printf(LocalizeSecure(LocalizeError(TEXT("ServerListen"),TEXT("Engine")),*Error2));
			Error = FString::Printf(LocalizeSecure(LocalizeError(TEXT("NetworkInit"),TEXT("Engine")),*Error2));
			debugf(NAME_Error, *Error);
			bHadListenError = TRUE;
		}
 	}

	const TCHAR* MutatorString = URL.GetOption(TEXT("Mutator="), TEXT(""));
	if (MutatorString)
	{
		TArray<FString> Mutators;
		FString(MutatorString).ParseIntoArray(&Mutators, TEXT(","), TRUE);

		for (INT MutatorIndex = 0; MutatorIndex < Mutators.Num(); MutatorIndex++)
		{
			LoadPackagesFully(FULLYLOAD_Mutator, Mutators(MutatorIndex));
		}
	}

	// Initialize gameplay for the level.
	GWorld->BeginPlay(URL);

	// Remember the URL. Put this before spawning player controllers so that
	// a player controller can get the map name during initialization and
	// have it be correct
	LastURL = URL;

	if (GWorld->GetNetMode() == NM_Client)
	{
		// remember last server URL
		LastRemoteURL = URL;
	}

	// Client init.
	for(FPlayerIterator It(this);It;++It)
	{
		FString Error2;
		if(!It->SpawnPlayActor(URL.String(1),Error2))
		{
			appErrorf(TEXT("%s"),*Error2);
		}
	}

	// notify player about errors starting listen server here
	if (bHadListenError)
	{
		SetProgress(PMT_ConnectionFailure, Error, TEXT(""));
	}

	// Remember DefaultPlayer options.
	if( GIsClient )
	{
		URL.SaveURLConfig( TEXT("DefaultPlayer"), TEXT("Name" ), GGameIni );
		URL.SaveURLConfig( TEXT("DefaultPlayer"), TEXT("Team" ), GGameIni );
		URL.SaveURLConfig( TEXT("DefaultPlayer"), TEXT("Class"), GGameIni );
		URL.SaveURLConfig( TEXT("DefaultPlayer"), TEXT("Skin" ), GGameIni );
		URL.SaveURLConfig( TEXT("DefaultPlayer"), TEXT("Face" ), GGameIni );
		URL.SaveURLConfig( TEXT("DefaultPlayer"), TEXT("Voice" ), GGameIni );
		URL.SaveURLConfig( TEXT("DefaultPlayer"), TEXT("OverrideClass" ), GGameIni );
	}

	// Prime texture streaming.
	GStreamingManager->NotifyLevelChange();

	debugf(TEXT("########### Finished loading level: %f seconds"),appSeconds() - StartTime);

	// load any per-map packages
	check(GWorld->PersistentLevel);
	LoadPackagesFully(FULLYLOAD_Map, GWorld->PersistentLevel->GetOutermost()->GetName());

	// send a callback message
	GCallbackEvent->Send(CALLBACK_PostLoadMap);

	UBOOL bShouldStopMovieAtEndOfLoadMap = FALSE;
	if (GFullScreenMovie
	&&	bStartedLoadMapMovie
	&&	GConfig->GetBool(TEXT("FullScreenMovie"), TEXT("bShouldStopMovieAtEndOfLoadMap"), bShouldStopMovieAtEndOfLoadMap, GEngineIni)
	&&	bShouldStopMovieAtEndOfLoadMap == TRUE)
	{
		// this will stop the movie we started at the start of loadmap
		// passing TRUE so that the movie will be delayed being stopped until the game has rendered a frame
		StopMovie(TRUE);
	}

	bWorldWasLoadedThisTick = TRUE;

	// Successfully started local level.
	return TRUE;
}

//
// Jumping viewport.
//
void UGameEngine::SetClientTravel( const TCHAR* NextURL, ETravelType InTravelType )
{
	// set TravelURL.  Will be processed safely on the next tick in UGameEngine::Tick().
	TravelURL    = NextURL;
	TravelType   = InTravelType;

	// Prevent crashing the game by attempting to connect to own listen server
	if ( LastURL.HasOption(TEXT("Listen")) )
	{
		LastURL.RemoveOption(TEXT("Listen"));
	}
}

/*-----------------------------------------------------------------------------
	Async persistent level map change.
-----------------------------------------------------------------------------*/

/**
 * Callback function used in UGameEngine::PrepareMapChange to pass to LoadPackageAsync.
 *
 * @param	LevelPackage	level package that finished async loading
 * @param	InGameEngine	pointer to game engine object to associated loaded level with so it won't be GC'ed
 */
static void AsyncMapChangeLevelLoadCompletionCallback( UObject* LevelPackage, void* InGameEngine )
{
	UGameEngine* GameEngine = (UGameEngine*) InGameEngine;
	check( GameEngine == GEngine );

	if( LevelPackage )
	{	
		// Try to find a UWorld object in the level package.
		UWorld* World = FindObject<UWorld>( LevelPackage, TEXT("TheWorld") );
		ULevel* Level = World ? World->PersistentLevel : NULL;	
		
		// Print out a warning and set the error if we couldn't find a level in this package.
		if( !Level )
		{
			// NULL levels can happen if existing package but not level is specified as a level name.
			GameEngine->PendingMapChangeFailureDescription = FString::Printf(TEXT("Couldn't find level in package %s"), *LevelPackage->GetName());
			debugf( NAME_Error, TEXT( "ERROR ERROR %s was not found in the PackageCache It must exist or the Level Loading Action will FAIL!!!! " ), *LevelPackage->GetName() );
			debugf( NAME_Warning, *GameEngine->PendingMapChangeFailureDescription );
			debugf( NAME_Error, TEXT( "ERROR ERROR %s was not found in the PackageCache It must exist or the Level Loading Action will FAIL!!!! " ), *LevelPackage->GetName() );
		}

		// Add loaded level to array to prevent it from being garbage collected.
		GameEngine->LoadedLevelsForPendingMapChange.AddItem( Level );
	}
	else
	{
		// Add NULL entry so we don't end up waiting forever on a level that is never going to be loaded.
		GameEngine->LoadedLevelsForPendingMapChange.AddItem( NULL );
		debugf( NAME_Warning, TEXT("NULL LevelPackage as argument to AsyncMapChangeLevelCompletionCallback") );
	}
}

/**
 * Prepares the engine for a map change by pre-loading level packages in the background.
 *
 * @param	LevelNames	Array of levels to load in the background; the first level in this
 *						list is assumed to be the new "persistent" one.
 *
 * @return	TRUE if all packages were in the package file cache and the operation succeeded, 
 *			FALSE otherwise. FALSE as a return value also indicates that the code has given
 *			up.
 */
UBOOL UGameEngine::PrepareMapChange(const TArray<FName>& LevelNames)
{
	// make sure level streaming isn't frozen
	GWorld->bIsLevelStreamingFrozen = FALSE;

	// Make sure we don't interrupt a pending map change in progress.
	if( !IsPreparingMapChange() )
	{
		LevelsToLoadForPendingMapChange.Empty();
		LevelsToLoadForPendingMapChange += LevelNames;

#if !FINAL_RELEASE
		// Verify that all levels specified are in the package file cache.
		FString Dummy;
		for( INT LevelIndex=0; LevelIndex<LevelsToLoadForPendingMapChange.Num(); LevelIndex++ )
		{
			const FName LevelName = LevelsToLoadForPendingMapChange(LevelIndex);
			if( !GPackageFileCache->FindPackageFile( *LevelName.ToString(), NULL, Dummy ) )
			{
				LevelsToLoadForPendingMapChange.Empty();
				PendingMapChangeFailureDescription = FString::Printf(TEXT("Couldn't find package for level '%s'"), *LevelName.ToString());
				// write it out immediately so make sure it's in the log even without a CommitMapChange happening
				debugf(NAME_Warning, TEXT("PREPAREMAPCHANGE: %s"), *PendingMapChangeFailureDescription);

				// tell user on screen!
				extern UBOOL GIsPrepareMapChangeBroken;
				GIsPrepareMapChangeBroken = TRUE;

				return FALSE;
			}
			//@todo streaming: make sure none of the maps are already loaded/ being loaded?
		}
#endif

		// copy LevelNames into the WorldInfo's array to keep track of the map change that we're preparing (primarily for servers so clients that join in progress can be notified)
		if (GWorld != NULL)
		{
			GWorld->GetWorldInfo()->PreparingLevelNames = LevelNames;
		}

		// Kick off async loading of packages.
		for( INT LevelIndex=0; LevelIndex<LevelsToLoadForPendingMapChange.Num(); LevelIndex++ )
		{
			const FName LevelName = LevelsToLoadForPendingMapChange(LevelIndex);
			if( GUseSeekFreeLoading )
			{
				// Only load localized package if it exists as async package loading doesn't handle errors gracefully.
				FString LocalizedPackageName = LevelName.ToString() + LOCALIZED_SEEKFREE_SUFFIX;
				FString LocalizedFileName;
				if( GPackageFileCache->FindPackageFile( *LocalizedPackageName, NULL, LocalizedFileName ) )
				{
					// Load localized part of level first in case it exists. We don't need to worry about GC or completion 
					// callback as we always kick off another async IO for the level below.
					UObject::LoadPackageAsync( *LocalizedPackageName, NULL, NULL );
				}
			}
			UObject::LoadPackageAsync( *LevelName.ToString(), AsyncMapChangeLevelLoadCompletionCallback, this );
		}

		return TRUE;
	}
	else
	{
		PendingMapChangeFailureDescription = TEXT("Current map change still in progress");
		return FALSE;
	}
}

/**
 * Returns the failure description in case of a failed map change request.
 *
 * @return	Human readable failure description in case of failure, empty string otherwise
 */
FString UGameEngine::GetMapChangeFailureDescription()
{
	return PendingMapChangeFailureDescription;
}
	
/**
 * Returns whether we are currently preparing for a map change or not.
 *
 * @return TRUE if we are preparing for a map change, FALSE otherwise
 */
UBOOL UGameEngine::IsPreparingMapChange()
{
	return LevelsToLoadForPendingMapChange.Num() > 0;
}
	
/**
 * Returns whether the prepared map change is ready for commit having called.
 *
 * @return TRUE if we're ready to commit the map change, FALSE otherwise
 */
UBOOL UGameEngine::IsReadyForMapChange()
{
	return IsPreparingMapChange() && (LevelsToLoadForPendingMapChange.Num() == LoadedLevelsForPendingMapChange.Num());
}

/**
 * Commit map change if requested and map change is pending. Called every frame.
 */
void UGameEngine::ConditionalCommitMapChange()
{
	// Check whether there actually is a pending map change and whether we want it to be committed yet.
	if( bShouldCommitPendingMapChange && IsPreparingMapChange() )
	{
		// Block on remaining async data.
		if( !IsReadyForMapChange() )
		{
			FlushAsyncLoading();
			check( IsReadyForMapChange() );
		}
		
		// Perform map change.
		if( !CommitMapChange(FALSE, FALSE) )
		{
			debugf(NAME_Warning, TEXT("Committing map change via %s was not successful: %s"), *GetFullName(), *GetMapChangeFailureDescription());
		}
		// No pending map change - called commit without prepare.
		else
		{
			debugf(TEXT("Committed map change via %s"), *GetFullName());
		}

		// We just commited, so reset the flag.
		bShouldCommitPendingMapChange = FALSE;
	}
}

/** struct to temporarily hold on to already loaded but unbound levels we're going to make visible at the end of CommitMapChange() while we first trigger GC */
struct FPendingStreamingLevelHolder : public FSerializableObject
{
public:
	TArray<ULevel*> Levels;

	virtual void Serialize(FArchive& Ar)
	{
		Ar << Levels;
	}
};

/**
 * Finalizes the pending map change that was being kicked off by PrepareMapChange.
 *
 * @param bShouldSkipLevelStartupEvent TRUE if this function NOT fire the SeqEvent_LevelBStartup event.
 * @param bShouldSkipLevelBeginningEvent TRUE if this function NOT fire the SeqEvent_LevelBeginning event. Useful for when skipping to the middle of a map by savegame or something
 *
 * @return	TRUE if successful, FALSE if there were errors (use GetMapChangeFailureDescription 
 *			for error description)
 */
UBOOL UGameEngine::CommitMapChange(UBOOL bShouldSkipLevelStartupEvent, UBOOL bShouldSkipLevelBeginningEvent)
{
	if (!IsPreparingMapChange())
	{
		PendingMapChangeFailureDescription = TEXT("No map change is being prepared");
		return FALSE;
	}
	else if (!IsReadyForMapChange())
	{
		PendingMapChangeFailureDescription = TEXT("Map change is not ready yet");
		return FALSE;
	}
	else
	{
		// Dump and reset the FPS chart on map changes.
		DumpFPSChart();
		ResetFPSChart();

		// Dump and reset the Memory chart on map changes.
		DumpMemoryChart();
		ResetMemoryChart();

		// tell the game we are about to switch levels
        if (GWorld->GetGameInfo())
		{
			// get the actual persistent level's name
			FString PreviousMapName = GWorld->PersistentLevel->GetOutermost()->GetName();
			FString NextMapName = LevelsToLoadForPendingMapChange(0).ToString();

			// look for a persistent streamed in sublevel
			for (INT LevelIndex = 0; LevelIndex < GWorld->GetWorldInfo()->StreamingLevels.Num(); LevelIndex++)
			{
				ULevelStreamingPersistent* PersistentLevel = Cast<ULevelStreamingPersistent>(GWorld->GetWorldInfo()->StreamingLevels(LevelIndex));
				if (PersistentLevel)
				{
					PreviousMapName = PersistentLevel->PackageName.ToString();
					// only one persistent level
					break;
				}
			}
            GWorld->GetGameInfo()->eventPreCommitMapChange(PreviousMapName, NextMapName); 
		}

		// on the client, check if we already loaded pending levels to be made visible due to e.g. the PackageMap
		FPendingStreamingLevelHolder LevelHolder;
		if (PendingLevelStreamingStatusUpdates.Num() > 0)
		{
			for (TObjectIterator<ULevel> It(TRUE); It; ++It)
			{
				for (INT i = 0; i < PendingLevelStreamingStatusUpdates.Num(); i++)
				{
					if ( It->GetOutermost()->GetFName() == PendingLevelStreamingStatusUpdates(i).PackageName && 
						(PendingLevelStreamingStatusUpdates(i).bShouldBeLoaded || PendingLevelStreamingStatusUpdates(i).bShouldBeVisible) )
					{
						LevelHolder.Levels.AddItem(*It);
						break;
					}
				}
			}
		}

		// we are no longer preparing this change
		GWorld->GetWorldInfo()->PreparingLevelNames.Empty();

		// Iterate over level collection, marking them to be forcefully unloaded.
		AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
		for( INT LevelIndex=0; LevelIndex<WorldInfo->StreamingLevels.Num(); LevelIndex++ )
		{
			ULevelStreaming* StreamingLevel	= WorldInfo->StreamingLevels(LevelIndex);
			if( StreamingLevel )
			{
				StreamingLevel->bIsRequestingUnloadAndRemoval = TRUE;
			}
		}

		// Collect garbage. @todo streaming: make sure that this doesn't stall due to async loading in the background
		UObject::CollectGarbage( GARBAGE_COLLECTION_KEEPFLAGS, TRUE );

		// The new fake persistent level is first in the LevelsToLoadForPendingMapChange array.
		FName	FakePersistentLevelName = LevelsToLoadForPendingMapChange(0);
		ULevel*	FakePersistentLevel		= NULL;
		// copy to WorldInfo to keep track of the last map change we performed (primarily for servers so clients that join in progress can be notified)
		// we don't need to remember secondary levels as the join code iterates over all streaming levels and updates them
		GWorld->GetWorldInfo()->CommittedPersistentLevelName = FakePersistentLevelName;

		// Find level package in loaded levels array.
		for( INT LevelIndex=0; LevelIndex<LoadedLevelsForPendingMapChange.Num(); LevelIndex++ )
		{
			ULevel* Level = LoadedLevelsForPendingMapChange(LevelIndex);

			// NULL levels can happen if existing package but not level is specified as a level name.
			if( Level && (FakePersistentLevelName == Level->GetOutermost()->GetFName()) )
			{
				FakePersistentLevel = Level;
				break;
			}
		}
		check( FakePersistentLevel );
#if WITH_FACEFX
		FString PersistentPackageName = FakePersistentLevel->GetOutermost()->GetName();
		FString PFFXAnimSetName = FakePersistentLevel->GetOutermost()->GetName() + TEXT("_FaceFXAnimSet");
		FString FullyQualName = PersistentPackageName + TEXT(".") + PFFXAnimSetName;
		UFaceFXAnimSet* NewPersistentFaceFXAnimSet = FindObject<UFaceFXAnimSet>(FakePersistentLevel->GetOutermost(), *PFFXAnimSetName);
		if (NewPersistentFaceFXAnimSet)
		{
			// Prevent if from getting GC'd until we can set it on the World.
			// We can't do it yet as we need to clear the last one first.
			//debugf(TEXT("************* FOUND PersisitentFaceFXAnimSet: %s"), *(NewPersistentFaceFXAnimSet->GetPathName()));
			NewPersistentFaceFXAnimSet->AddToRoot();
#if 0 // Make this '1' to spew out all the animations contained in the animset
			const OC3Ent::Face::FxAnimSet* TempFFXAnimSet = NewPersistentFaceFXAnimSet->GetFxAnimSet();
			const OC3Ent::Face::FxAnimGroup& TempFFXAnimGroup = TempFFXAnimSet->GetAnimGroup();
			OC3Ent::Face::FxAnimGroup* FFXAnimGroup = (OC3Ent::Face::FxAnimGroup*)&TempFFXAnimGroup;
			if ( FFXAnimGroup != NULL )
			{
				OC3Ent::Face::FxSize AnimCount = FFXAnimGroup->GetNumAnims();
				debugf(TEXT("\tGroup %s has %d animations"), ANSI_TO_TCHAR(TempFFXAnimGroup.GetNameAsCstr()), (INT)AnimCount);
				for (OC3Ent::Face::FxSize AnimIndex = 0; AnimIndex < AnimCount; AnimIndex++)
				{
					const OC3Ent::Face::FxAnim& FXAnim = FFXAnimGroup->GetAnim(AnimIndex);
					
					debugf(TEXT("\t\t%3d - %s"), (INT)AnimIndex, ANSI_TO_TCHAR(FXAnim.GetNameAsCstr()));
				}
			}
#endif
		}
		else
		{
			//debugf(TEXT("************* Didn't find PersisitentFaceFXAnimSet: %s - in %s"), *PFFXAnimSetName, *(FakePersistentLevel->GetPathName()));
		}
#endif	//#if WITH_FACEFX

		// Construct a new ULevelStreamingPersistent for the new persistent level.
		ULevelStreamingPersistent* LevelStreamingPersistent = ConstructObject<ULevelStreamingPersistent>(
			ULevelStreamingPersistent::StaticClass(),
			UObject::GetTransientPackage(),
			*FString::Printf(TEXT("LevelStreamingPersistent_%s"), *FakePersistentLevel->GetOutermost()->GetName()) );

		// Propagate level and name to streaming object.
		LevelStreamingPersistent->LoadedLevel	= FakePersistentLevel;
		LevelStreamingPersistent->PackageName	= FakePersistentLevelName;
		// And add it to the world info's list of levels.
		WorldInfo->StreamingLevels.AddItem( LevelStreamingPersistent );

		// Add secondary levels to the world info levels array.
		WorldInfo->StreamingLevels += FakePersistentLevel->GetWorldInfo()->StreamingLevels;

		// fixup up any kismet streaming objects to force them to be loaded if they were preloaded, this
		// will keep streaming volumes from immediately unloading the levels that were just loaded
		for( INT LevelIndex=0; LevelIndex<WorldInfo->StreamingLevels.Num(); LevelIndex++ )
		{
			ULevelStreaming* StreamingLevel	= WorldInfo->StreamingLevels(LevelIndex);
			// mark any kismet streamers to force be loaded
			if (StreamingLevel)
			{
				UBOOL bWasFound = FALSE;
				// was this one of the packages we wanted to load?
				for (INT LoadLevelIndex = 0; LoadLevelIndex < LevelsToLoadForPendingMapChange.Num(); LoadLevelIndex++)
				{
					if (LevelsToLoadForPendingMapChange(LoadLevelIndex) == StreamingLevel->PackageName)
					{
						bWasFound = TRUE;
						break;
					}
				}

				// if this level was preloaded, mark it as to be loaded and visible
				if (bWasFound)
				{
					StreamingLevel->bShouldBeLoaded		= TRUE;
					StreamingLevel->bShouldBeVisible	= TRUE;

					if (GWorld->IsServer())
					{
						// notify players of the change
						for (AController *Controller = GWorld->GetWorldInfo()->ControllerList; Controller != NULL; Controller = Controller->NextController)
						{
							APlayerController *PC = Cast<APlayerController>(Controller);
							if (PC != NULL)
							{
								PC->eventLevelStreamingStatusChanged( 
									StreamingLevel, 
									StreamingLevel->bShouldBeLoaded, 
									StreamingLevel->bShouldBeVisible,
									StreamingLevel->bShouldBlockOnLoad );
							}
						}
					}
				}
			}
		}
		
		// Kill actors we are supposed to remove reference to during seamless map transitions.
		GWorld->CleanUpBeforeLevelTransition();

		// Update level streaming, forcing existing levels to be unloaded and their streaming objects 
		// removed from the world info.	We can't kick off async loading in this update as we want to 
		// collect garbage right below.
		GWorld->FlushLevelStreaming( NULL, TRUE );
		
		// make sure any looping sounds, etc are stopped
		if (Client != NULL && Client->GetAudioDevice() != NULL)
		{
			GEngine->Client->GetAudioDevice()->StopAllSounds();
		}

		// Remove all unloaded levels from memory and perform full purge.
		UObject::CollectGarbage( GARBAGE_COLLECTION_KEEPFLAGS, TRUE );
		
		// if there are pending streaming changes replicated from the server, apply them immediately
		if (PendingLevelStreamingStatusUpdates.Num() > 0)
		{
			for (INT i = 0; i < PendingLevelStreamingStatusUpdates.Num(); i++)
			{
				ULevelStreaming* LevelStreamingObject = NULL;
				for (INT j = 0; j < WorldInfo->StreamingLevels.Num(); j++)
				{
					if (WorldInfo->StreamingLevels(j) != NULL && WorldInfo->StreamingLevels(j)->PackageName == PendingLevelStreamingStatusUpdates(i).PackageName)
					{
						LevelStreamingObject = WorldInfo->StreamingLevels(j);
						if (LevelStreamingObject != NULL)
						{
							LevelStreamingObject->bShouldBeLoaded	= PendingLevelStreamingStatusUpdates(i).bShouldBeLoaded;
							LevelStreamingObject->bShouldBeVisible	= PendingLevelStreamingStatusUpdates(i).bShouldBeVisible;
						}
						else
						{
							check(LevelStreamingObject);
							debugfSuppressed(NAME_DevStreaming, TEXT("Unable to handle streaming object %s"),*LevelStreamingObject->GetName());
						}

						// break out of object iterator if we found a match
						break;
					}
				}

				if (LevelStreamingObject == NULL)
				{
					debugfSuppressed(NAME_DevStreaming, TEXT("Unable to find streaming object %s"), *PendingLevelStreamingStatusUpdates(i).PackageName.ToString());
				}
			}

			PendingLevelStreamingStatusUpdates.Empty();

			GWorld->FlushLevelStreaming(NULL, FALSE);
		}
		else
		{
			// This will cause the newly added persistent level to be made visible and kick off async loading for others.
			GWorld->FlushLevelStreaming( NULL, TRUE );
		}

		// delay the use of streaming volumes for a few frames
		GWorld->DelayStreamingVolumeUpdates(3);

		// notify the new levels that they are starting up
		for( INT LevelIndex=0; LevelIndex<LoadedLevelsForPendingMapChange.Num(); LevelIndex++ )
		{
			ULevel* Level = LoadedLevelsForPendingMapChange(LevelIndex);
			if( Level )
			{
				for (INT SeqIdx = 0; SeqIdx < Level->GameSequences.Num(); SeqIdx++)
				{
					USequence* Seq = Level->GameSequences(SeqIdx);
					if(Seq)
					{
						Seq->NotifyMatchStarted(!bShouldSkipLevelStartupEvent, !bShouldSkipLevelBeginningEvent);
					}
				}
			}
		}

		// Empty intermediate arrays.
		LevelsToLoadForPendingMapChange.Empty();
		LoadedLevelsForPendingMapChange.Empty();
		PendingMapChangeFailureDescription = TEXT("");

		// Prime texture streaming.
		GStreamingManager->NotifyLevelChange();

		// tell the game we are done switching levels
        if (GWorld->GetGameInfo())
		{
            GWorld->GetGameInfo()->eventPostCommitMapChange(); 
		}

#if WITH_FACEFX
		if (NewPersistentFaceFXAnimSet)
		{
			NewPersistentFaceFXAnimSet->RemoveFromRoot();
			GWorld->SetPersistentFaceFXAnimSet(NewPersistentFaceFXAnimSet);
		}
#endif	//#if WITH_FACEFX

		return TRUE;
	}
}

/**
 * Cancels pending map change.
 */
void UGameEngine::CancelPendingMapChange()
{
	// Empty intermediate arrays.
	LevelsToLoadForPendingMapChange.Empty();
	LoadedLevelsForPendingMapChange.Empty();

	// Reset state and make sure conditional map change doesn't fire.
	PendingMapChangeFailureDescription	= TEXT("");
	bShouldCommitPendingMapChange		= FALSE;
	
	// Reset array of levels to prepare for client.
	if( GWorld )
	{
		GWorld->GetWorldInfo()->PreparingLevelNames.Empty();
	}
}

/*-----------------------------------------------------------------------------
	Tick.
-----------------------------------------------------------------------------*/

//
// Get tick rate limitor.
//
FLOAT UGameEngine::GetMaxTickRate( FLOAT DeltaTime, UBOOL bAllowFrameRateSmoothing )
{
	FLOAT MaxTickRate = 0;

#if CONSOLE
	// Limit framerate on console if VSYNC is enabled to avoid jumps from 30 to 60 and back.
	if( GSystemSettings.bUseVSync )
	{
		MaxTickRate = MaxSmoothedFrameRate;
	}
#else
	if( GWorld )
	{
		UNetDriver* NetDriver = GWorld->GetNetDriver();
		// Demo recording or playback.
		if( GWorld->DemoRecDriver )
		{
			// Demo recording from dedicated server.
			if( !GWorld->DemoRecDriver->ServerConnection && NetDriver && !GIsClient )
			{
				// We're a dedicated server recording a demo, use the high framerate demo tick.
				MaxTickRate = Clamp( GWorld->DemoRecDriver->NetServerMaxTickRate, 20, 60 );
			}
			// Respect bNoFrameCap otherwise by disabling framerate smoothing
			else if( GWorld->DemoRecDriver->bNoFrameCap )
			{
				bAllowFrameRateSmoothing = FALSE;
			}
		}
		// In network games, limit framerate to not saturate bandwidth.
		else if( NetDriver && (!GIsClient || NetDriver->bClampListenServerTickRate))
		{
			// We're a dedicated server, use the LAN or Net tick rate.
			MaxTickRate = Clamp( NetDriver->NetServerMaxTickRate, 10, 120 );
		}
		else if( NetDriver && NetDriver->ServerConnection )
		{
			//@todo FIXMESTEVE: take voice bandwidth into account.
			MaxTickRate = NetDriver->ServerConnection->CurrentNetSpeed / GWorld->GetWorldInfo()->MoveRepSize;
			if( NetDriver->ServerConnection->CurrentNetSpeed <= 10000 )
			{
				MaxTickRate = Clamp( MaxTickRate, 10.f, 90.f );
			}
		}
	}

	// Smooth the framerate if wanted. The code uses a simplistic running average. Other approaches, like reserving
	// a percentage of time, ended up creating negative feedback loops in conjunction with GPU load and were abandonend.
	if( bSmoothFrameRate && bAllowFrameRateSmoothing && GIsClient )
	{
		if( DeltaTime < 0.0f )
		{
#if SHIPPING_PC_GAME
			// End users don't have access to the secure parts of UDN. The localized string points to the release notes,
			// which should include a link to the AMD CPU drivers download site.
			appErrorf( NAME_FriendlyError, *LocalizeError("NegativeDeltaTime",TEXT("Engine")) );
#else
			// Send developers to the support list thread.
			appErrorf(TEXT("Negative delta time! Please see https://udn.epicgames.com/lists/showpost.php?list=ue3bugs&id=4364"));
#endif
		}

		// Running average delta time, initial value at 100 FPS so fast machines don't have to creep up
		// to a good frame rate due to code limiting upward "mobility".
		static FLOAT RunningAverageDeltaTime = 1 / 100.f;

		// Keep track of running average over 300 frames, clamping at min of 5 FPS for individual delta times.
		RunningAverageDeltaTime = Lerp<FLOAT>( RunningAverageDeltaTime, Min<FLOAT>( DeltaTime, 0.2f ), 1 / 300.f );

		// Work in FPS domain as that is what the function will return.
		FLOAT AverageFPS = 1.f / RunningAverageDeltaTime;

		// Clamp FPS into ini defined min/ max range.
		MaxTickRate = Clamp<FLOAT>( AverageFPS, MinSmoothedFrameRate, MaxSmoothedFrameRate );
	}
#endif

	return MaxTickRate;
}

//
// Update everything.
//

void UGameEngine::Tick( FLOAT DeltaSeconds )
{
	SCOPE_CYCLE_COUNTER(STAT_GameEngineTick);

	INT LocalTickCycles=0;
	CLOCK_CYCLES(LocalTickCycles);

	check(GWorld);

    if( DeltaSeconds < 0.0f )
	{
#if SHIPPING_PC_GAME
		// End users don't have access to the secure parts of UDN.  Regardless, they won't
		// need the warning because the game ships with AMD drivers that address the issue.
		appErrorf(TEXT("Negative delta time!"));
#else
		// Send developers to the support list thread.
		appErrorf(TEXT("Negative delta time! Please see https://udn.epicgames.com/lists/showpost.php?list=ue3bugs&id=4364"));
#endif
	}

	// Tick allocator
	if( GMalloc != NULL )
	{
		GMalloc->Tick( DeltaSeconds );
	}
	
	// Ticks the FPS chart.
	TickFPSChart( DeltaSeconds );

	// Ticks the Memory chart.
	TickMemoryChart( DeltaSeconds );

	if ( GDebugger != NULL )
	{
		GDebugger->NotifyBeginTick();
	}

	// Tick the client code.
	INT LocalClientCycles=0;
	if( Client )
	{
		CLOCK_CYCLES(LocalClientCycles);
		Client->Tick( DeltaSeconds );
		UNCLOCK_CYCLES(LocalClientCycles);
	}
	ClientCycles=LocalClientCycles;

#if !CONSOLE && defined(_MSC_VER)
	if ( GDebugger != NULL && GIsRequestingExit )
	{
		static_cast<UDebuggerCore*>(GDebugger)->Close(TRUE);
		GDebugger = NULL;
	}
#endif // !CONSOLE && defined(_MSC_VER)

	// Clean up the game viewports that have been closed.
	CleanupGameViewport();

	// If all viewports closed, time to exit.
	if(GIsClient && GameViewport == NULL )
	{
#if !CONSOLE && defined(_MSC_VER)
		if ( GDebugger != NULL )
		{
			static_cast<UDebuggerCore*>(GDebugger)->Close(TRUE);
		}
#endif // !CONSOLE && defined(_MSC_VER)

		debugf( TEXT("All Windows Closed") );
		appRequestExit( 0 );
		return;
	}

	if ( GameViewport != NULL )
	{
		// Decide whether to drop high detail because of frame rate.
		GameViewport->SetDropDetail(DeltaSeconds);
	}

	// Update subsystems.
	{
		// This assumes that UObject::StaticTick only calls ProcessAsyncLoading.
		UObject::StaticTick( DeltaSeconds );
	}

	// Handle seamless traveling
	if (GSeamlessTravelHandler.IsInTransition())
	{
		GSeamlessTravelHandler.Tick();
	}

	// Tick the world.
	GameCycles=0;
	CLOCK_CYCLES(GameCycles);
	GWorld->Tick( LEVELTICK_All, DeltaSeconds );
	UNCLOCK_CYCLES(GameCycles);

	// Issue cause event after first tick to provide a chance for the game to spawn the player and such.
	if( bWorldWasLoadedThisTick )
	{
		const TCHAR* InitialExec = LastURL.GetOption(TEXT("causeevent="),NULL);
		if( InitialExec && GamePlayers.Num() && GamePlayers(0) )
		{
			debugf(TEXT("Issuing initial cause event passed from URL: %s"), InitialExec);
			GamePlayers(0)->Exec( *(FString("CAUSEEVENT ") + InitialExec), *GLog );
		}
	}

	// Tick the viewports.
	if ( GameViewport != NULL )
	{
		SCOPE_CYCLE_COUNTER(STAT_GameViewportTick);
		GameViewport->Tick(DeltaSeconds);
	}

	// Handle server travelling.
	if( GWorld->GetWorldInfo()->NextURL!=TEXT("") )
	{
		GWorld->GetWorldInfo()->NextSwitchCountdown -= DeltaSeconds;
		if( GWorld->GetWorldInfo()->NextSwitchCountdown <= 0.f )
		{
			debugf( TEXT("Server switch level: %s"), *GWorld->GetWorldInfo()->NextURL );
			if (GWorld->GetGameInfo() != NULL)
			{
				GWorld->GetGameInfo()->eventGameEnding(); 
			}
			FString Error;
			Browse( FURL(&LastURL,*GWorld->GetWorldInfo()->NextURL,(ETravelType)GWorld->GetWorldInfo()->NextTravelType), Error );
			GWorld->GetWorldInfo()->NextURL = TEXT("");
			return;
		}
	}

	// Handle client travelling.
	if( TravelURL != TEXT("") )
	{
		// Make sure we stop any vibration before loading next map.
		if(Client)
		{
			Client->ForceClearForceFeedback();
		}

		if (GWorld->GetGameInfo() != NULL)
		{
            GWorld->GetGameInfo()->eventGameEnding(); 
		}

		FString Error, TravelURLCopy = TravelURL;
		Browse( FURL(&LastURL,*TravelURLCopy,(ETravelType)TravelType), Error );
		return;
	}

	// Update the pending level.
	if( GPendingLevel )
	{
		GPendingLevel->Tick( DeltaSeconds );
		if ( GPendingLevel->ConnectionError.Len() > 0 )
		{
			// stop the connecting movie
			StopMovie(FALSE);

			// Pending connect failed.
			SetProgress( PMT_ConnectionFailure, LocalizeError(TEXT("ConnectionFailed_Title"), TEXT("Engine")), GPendingLevel->ConnectionError );
			debugf( NAME_Log, LocalizeSecure(LocalizeError(TEXT("Pending"),TEXT("Engine")), *GPendingLevel->URL.String(), *GPendingLevel->ConnectionError) );

			GPendingLevel = NULL;
		}
		else if( GPendingLevel->bSuccessfullyConnected && GPendingLevel->FilesNeeded == 0 && !GPendingLevel->bSentJoinRequest )
		{
			// Attempt to load the map.
			FString Error;

			const UBOOL bLoadedMapSuccessfully = LoadMap( GPendingLevel->URL, GPendingLevel, Error );
			if( !bLoadedMapSuccessfully || Error != TEXT("") )
			{
				// make sure there's a valid GWorld and PlayerController available to handle the error message
				// errors here are fatal; there's nothing left to fall back to
				check(GamePlayers.Num() > 0);
				if (GamePlayers(0)->Actor == NULL)
				{
					if (GWorld != NULL)
					{
						// need a PlayerController for loading screen rendering
						GamePlayers(0)->Actor = CastChecked<APlayerController>(GWorld->SpawnActor(APlayerController::StaticClass()));
					}
					// we can't guarantee the current GWorld is in a valid state, so travel to the default map
#if OLD_CONNECTION_FAILURE_CODE
					APlayerController* PC = GEngine->GamePlayers.Num() > 0 && GEngine->GamePlayers(0) != NULL
						? GEngine->GamePlayers(0)->Actor
						: NULL;

					if ( PC != NULL )
					{
						//@todo ronp connection
						PC->eventNotifyConnectionError(TRUE, FCT_Unknown);
					}
#else
					//@todo ronp connection - pass the value of Error to Browse somehow....
#endif

					FString NewError;
					verify(Browse(FURL(&LastURL, TEXT("?failed"), TRAVEL_Relative), NewError));

					check(GWorld != NULL);
					//@todo: should have special code/error message for this
					check(GamePlayers(0)->Actor != NULL);
				}

				// we call SetProgress here because the handler for ?failed in UGameEngine::Browse() only calls
				// SetProgress if GPendingLevel is NULL.
				SetProgress( PMT_ConnectionFailure, LocalizeError(TEXT("ConnectionFailed_Title"), TEXT("Engine")), Error );

				//@todo - here is where we probably want to handle match full errors
			}
			else
			{
				// Show connecting message, cause precaching to occur.
				TransitionType = TT_Connecting;
				RedrawViewports();

				// Send join.
				GPendingLevel->SendJoin();
				GPendingLevel->NetDriver = NULL;
				GPendingLevel->DemoRecDriver = NULL;
			}

			// Kill the pending level.
			GPendingLevel = NULL;
		}
	}

#if STATS	// Can't do this within the malloc classes as they get initialized before the stats
	FStatGroup* MemGroup = GStatManager.GetGroup(STATGROUP_Memory);
	// Update memory stats.
	if (MemGroup->bShowGroup == TRUE)
	{
#if PS3
		// handle special memory regions of PS3
		extern void UpdateMemStats();
		UpdateMemStats();
#else
		SIZE_T Virtual  = 0; 
		SIZE_T Physical = 0;
		GMalloc->GetAllocationInfo( Virtual, Physical );
		SET_DWORD_STAT(STAT_VirtualAllocSize,Virtual);
		SET_DWORD_STAT(STAT_PhysicalAllocSize,Physical);
#endif
		
#if WITH_FACEFX
		SET_DWORD_STAT(STAT_FaceFXCurrentAllocSize,OC3Ent::Face::FxGetCurrentBytesAllocated());
		SET_DWORD_STAT(STAT_FaceFXPeakAllocSize,OC3Ent::Face::FxGetPeakBytesAllocated());
#endif

	}

	static QWORD LastMallocCalls		= 0;
	static QWORD LastReallocCalls		= 0;
	static QWORD LastFreeCalls			= 0;
	static QWORD LastPhysicalAllocCalls = 0;
	static QWORD LastPhysicalFreeCalls	= 0;

	DWORD CurrentFrameMallocCalls		= FMalloc::TotalMallocCalls - LastMallocCalls;
	DWORD CurrentFrameReallocCalls		= FMalloc::TotalReallocCalls - LastReallocCalls;
	DWORD CurrentFrameFreeCalls			= FMalloc::TotalFreeCalls - LastFreeCalls;
	DWORD CurrentFramePhysicalAllocCalls= FMalloc::TotalPhysicalAllocCalls - LastPhysicalAllocCalls;
	DWORD CurrentFramePhysicalFreeCalls	= FMalloc::TotalPhysicalFreeCalls - LastPhysicalFreeCalls;
	DWORD CurrentFrameAllocatorCalls	= CurrentFrameMallocCalls + CurrentFrameReallocCalls + CurrentFrameFreeCalls + CurrentFramePhysicalAllocCalls + CurrentFramePhysicalFreeCalls;

	SET_DWORD_STAT( STAT_MallocCalls, CurrentFrameMallocCalls );
	SET_DWORD_STAT( STAT_ReallocCalls, CurrentFrameReallocCalls );
	SET_DWORD_STAT( STAT_FreeCalls, CurrentFrameFreeCalls );
	SET_DWORD_STAT( STAT_PhysicalAllocCalls, CurrentFramePhysicalAllocCalls );
	SET_DWORD_STAT( STAT_PhysicalFreeCalls, CurrentFramePhysicalFreeCalls );
	SET_DWORD_STAT( STAT_TotalAllocatorCalls, CurrentFrameAllocatorCalls );

	LastMallocCalls			= FMalloc::TotalMallocCalls;
	LastReallocCalls		= FMalloc::TotalReallocCalls;
	LastFreeCalls			= FMalloc::TotalFreeCalls;
	LastPhysicalAllocCalls	= FMalloc::TotalPhysicalAllocCalls;
	LastPhysicalFreeCalls	= FMalloc::TotalPhysicalFreeCalls;
#endif

	// Update the transition screen.
	if(TransitionType == TT_Connecting)
	{
		// Check to see if all players have finished connecting.
		TransitionType = TT_None;
		for(FPlayerIterator PlayerIt(this);PlayerIt;++PlayerIt)
		{
			if(!PlayerIt->Actor)
			{
				// This player has not received a PlayerController from the server yet, so leave the connecting screen up.
				TransitionType = TT_Connecting;
				break;
			}
		}
	}
	else if(TransitionType == TT_None || TransitionType == TT_Paused)
	{
		// Display a paused screen if the game is paused.
		TransitionType = (GWorld->GetWorldInfo()->Pauser != NULL) ? TT_Paused : TT_None;
	}

#if !CONSOLE
	// Hide the splashscreen and show the game window
	static UBOOL bFirstTime = TRUE;
	if ( GIsGame && bFirstTime )
	{
		extern void appHideSplash();
		bFirstTime = FALSE;
		appHideSplash();
		extern void appShowGameWindow();
		appShowGameWindow();
	}
#endif

	// Render everything.
	RedrawViewports();
	
	// Block on async loading if requested.
	if( GWorld->GetWorldInfo()->bRequestedBlockOnAsyncLoading )
	{
		// Only perform work if there is anything to do. This ensures we are not syncronizing with the GPU
		// and suspending the device needlessly.
		UBOOL bWorkToDo = UObject::IsAsyncLoading();
		if (!bWorkToDo)
		{
			GWorld->UpdateLevelStreaming();
			bWorkToDo = GWorld->IsVisibilityRequestPending();
		}
		if (bWorkToDo)
		{
			// Make sure vibration stops when blocking on load.
			if(GEngine->Client)
			{
				GEngine->Client->ForceClearForceFeedback();
			}

			// tell clients to do the same so they don't fall behind
			for (AController* C = GWorld->GetFirstController(); C != NULL; C = C->NextController)
			{
				APlayerController* PC = C->GetAPlayerController();
				if (PC != NULL)
				{
					UNetConnection* Conn = Cast<UNetConnection>(PC->Player);
					if (Conn != NULL && Conn->GetUChildConnection() == NULL)
					{
						// call the event to replicate the call
						PC->eventClientSetBlockOnAsyncLoading();
						// flush the connection to make sure it gets sent immediately
						Conn->FlushNet();
					}
				}
			}

			// Enqueue command to suspend rendering during blocking load.
			ENQUEUE_UNIQUE_RENDER_COMMAND( SuspendRendering, { extern UBOOL GGameThreadWantsToSuspendRendering; GGameThreadWantsToSuspendRendering = TRUE; } );
			// Flush rendering commands to ensure we don't have any outstanding work on rendering thread
			// and rendering is indeed suspended.
			FlushRenderingCommands();

			// Flushes level streaming requests, blocking till completion.
			GWorld->FlushLevelStreaming();

			// Resume rendering again now that we're done loading.
			ENQUEUE_UNIQUE_RENDER_COMMAND( ResumeRendering, { extern UBOOL GGameThreadWantsToSuspendRendering; GGameThreadWantsToSuspendRendering = FALSE; RHIResumeRendering(); } );
		}
		GWorld->GetWorldInfo()->bRequestedBlockOnAsyncLoading = FALSE;
	}

	// streamingServer
    if( GIsServer == TRUE )
	{
		SCOPE_CYCLE_COUNTER(STAT_UpdateLevelStreaming);
		GWorld->UpdateLevelStreaming();
	}

	// Update Audio. This needs to occur after rendering as the rendering code updates the listener position.
	if( Client && Client->GetAudioDevice() )
	{
		Client->GetAudioDevice()->Update( !GWorld->IsPaused() );
	}

	if( GIsClient )
	{
		// Update resource streaming after viewports have had a chance to update view information.
		GStreamingManager->UpdateResourceStreaming( DeltaSeconds );
	}

	bWorldWasLoadedThisTick = FALSE;

	UNCLOCK_CYCLES(LocalTickCycles);
	TickCycles=LocalTickCycles;

	if( GScriptCallGraph )
	{
		GScriptCallGraph->Tick(DeltaSeconds);
	}

#if USING_REMOTECONTROL
	if ( RemoteControlExec )
	{
		RemoteControlExec->RenderInGame();
	}
#endif

	// See whether any map changes are pending and we requested them to be committed.
	ConditionalCommitMapChange();

	// Tick the GRenderingRealtimeClock, unless it's paused
	if ( GPauseRenderingRealtimeClock == FALSE )
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			TickRenderingTimer,
			FTimer*, Timer, &GRenderingRealtimeClock,
			FLOAT, DeltaTime, DeltaSeconds,
		{
			Timer->Tick(DeltaTime);
		});
	}
}

/**
 * Handles freezing/unfreezing of rendering
 */
void UGameEngine::ProcessToggleFreezeCommand()
{
	if (GameViewport)
	{
		GameViewport->Viewport->ProcessToggleFreezeCommand();
	}
}

/**
 * Handles frezing/unfreezing of streaming
 */
void UGameEngine::ProcessToggleFreezeStreamingCommand()
{
	// if not already frozen, then flush async loading before we freeze so that we don't bollocks up any in-process streaming
	if (!GWorld->bIsLevelStreamingFrozen)
	{
		UObject::FlushAsyncLoading();
	}

	// toggle the frozen state
	GWorld->bIsLevelStreamingFrozen = !GWorld->bIsLevelStreamingFrozen;
}

/**
 * Returns the online subsystem object. Returns null if GEngine isn't a
 * game engine
 */
UOnlineSubsystem* UGameEngine::GetOnlineSubsystem(void)
{
	UOnlineSubsystem* OnlineSubsystem = NULL;
	// Make sure this is the correct type (not PIE)
	UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
	if (GameEngine != NULL)
	{
		OnlineSubsystem = GameEngine->OnlineSubsystem;
	}
	return OnlineSubsystem;
}

/**
 * Script to C++ thunking code
 */
void UGameEngine::execGetOnlineSubsystem( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	*(class UOnlineSubsystem**)Result=UGameEngine::GetOnlineSubsystem();
}

/**
 * Creates the online subsystem that was specified in UEngine's
 * OnlineSubsystemClass. This function is virtual so that licensees
 * can provide their own version without modifying Epic code.
 */
void UGameEngine::InitOnlineSubsystem(void)
{
	// See if a class was loaded
	if (OnlineSubsystemClass != NULL)
	{
		// Create an instance of the object
		OnlineSubsystem = ConstructObject<UOnlineSubsystem>(OnlineSubsystemClass);
		if (OnlineSubsystem)
		{
			// Try to initialize the subsystem
			// NOTE: The subsystem is responsible for fixing up its interface pointers
			if (OnlineSubsystem->eventInit() == TRUE)
			{
				if (OnlineSubsystem->eventPostInit() == FALSE)
				{
					debugf(NAME_Error,TEXT("Failed to PostInit() online subsytem (%s)"),
						*OnlineSubsystem->GetFullName());
					// Remove the ref so it will get gc-ed
					OnlineSubsystem = NULL;
				}
			}
			else
			{
				debugf(NAME_Error,TEXT("Failed to init online subsytem (%s)"),
					*OnlineSubsystem->GetFullName());
				// Remove the ref so it will get gc-ed
				OnlineSubsystem = NULL;
			}
		}
		else
		{
			debugf(NAME_Error,TEXT("Failed to create online subsytem (%s)"),
				*OnlineSubsystemClass->GetFullName());
		}
	}
}

/**
 * Spawns all of the registered server actors
 */
void UGameEngine::SpawnServerActors(void)
{
	for( INT i=0; i < ServerActors.Num(); i++ )
	{
		TCHAR Str[240];
		const TCHAR* Ptr = * ServerActors(i);
		if( ParseToken( Ptr, Str, ARRAY_COUNT(Str), 1 ) )
		{
			debugf(NAME_DevNet, TEXT("Spawning: %s"), Str );
			UClass* HelperClass = StaticLoadClass( AActor::StaticClass(), NULL, Str, NULL, LOAD_None, NULL );
			AActor* Actor = GWorld->SpawnActor( HelperClass );
			while( Actor && ParseToken(Ptr,Str,ARRAY_COUNT(Str),1) )
			{
				TCHAR* Value = appStrchr(Str,'=');
				if( Value )
				{
					*Value++ = 0;
					for( TFieldIterator<UProperty,CASTCLASS_UProperty> It(Actor->GetClass()); It; ++It )
						if
						(	appStricmp(*It->GetName(),Str)==0
						&&	(It->PropertyFlags & CPF_Config) )
							It->ImportText( Value, (BYTE*)Actor + It->Offset, 0, Actor );
				}
			}
		}
	}
}

/**
 * Adds a map/package array pair for pacakges to load at LoadMap
 *
 * @param FullyLoadType When to load the packages (based on map, gametype, etc)
 * @param Tag Map/game for which the packages need to be loaded
 * @param Packages List of package names to fully load when the map is loaded
 * @param bLoadPackagesForCurrentMap If TRUE, the packages for the currently loaded map will be loaded now
 */
void UGameEngine::AddPackagesToFullyLoad(EFullyLoadPackageType FullyLoadType, const FString& Tag, const TArray<FName>& Packages, UBOOL bLoadPackagesForCurrentMap)
{
	/*
	for ( INT i = 0 ; i < Packages.Num() ; ++i )
	{
		debugf(TEXT("------------- AddPackagesToFullyLoad: fullyloadtype %i for %s: %s"), (INT)FullyLoadType, *Tag, *Packages(i).ToString() );
	}
	*/

	// make a new entry
	const INT NewIndex = PackagesToFullyLoad.AddZeroed(1);

	// get the newly added entry
	FFullyLoadedPackagesInfo& PackagesInfo = PackagesToFullyLoad(NewIndex);

	// fill it out
	PackagesInfo.FullyLoadType = FullyLoadType;
	PackagesInfo.Tag = Tag;
	PackagesInfo.PackagesToLoad = Packages;

	// if desired, load the packages for the current map now
	if (bLoadPackagesForCurrentMap && GWorld && GWorld->PersistentLevel)
	{
		LoadPackagesFully(FullyLoadType, GWorld->PersistentLevel->GetOutermost()->GetName());
	}

	// if the type is to always loading, load them now, they will be unloaded in CleanupAllPackagesToFullyLoad()
	if (FullyLoadType == FULLYLOAD_Always)
	{
		LoadPackagesFully(FullyLoadType, TEXT("___TAILONLY___"));
	}
}

/**
 * Empties the PerMapPackages array, and removes any currently loaded packages from the Root
 */
void UGameEngine::CleanupAllPackagesToFullyLoad()
{
	// make sure the packages for the current map are cleaned
	CleanupPackagesToFullyLoad(FULLYLOAD_Map, GWorld->PersistentLevel->GetOutermost()->GetName());
	// all pergame type packages need to be unloaded
	CleanupPackagesToFullyLoad(FULLYLOAD_Game_PreLoadClass, TEXT(""));
	CleanupPackagesToFullyLoad(FULLYLOAD_Game_PostLoadClass, TEXT(""));
	CleanupPackagesToFullyLoad(FULLYLOAD_Always, TEXT(""));
	CleanupPackagesToFullyLoad(FULLYLOAD_Mutator, TEXT(""));

	// empty the array
	PackagesToFullyLoad.Empty();
}

/**
 * Loads the PerMapPackages for the given map, and adds them to the RootSet
 *
 * @param FullyLoadType When to load the packages (based on map, gametype, etc)
 * @param Tag Name of the map/game to load packages for ("" will match any)
 */
void UGameEngine::LoadPackagesFully(EFullyLoadPackageType FullyLoadType, const FString& Tag)
{
	//debugf(TEXT("------------------ LoadPackagesFully: %i, %s"),(INT)FullyLoadType, *Tag);

	// look for all entries for the given map
	for (INT MapIndex = ((Tag == TEXT("___TAILONLY___")) ? PackagesToFullyLoad.Num() - 1 : 0); MapIndex < PackagesToFullyLoad.Num(); MapIndex++)
	{
		FFullyLoadedPackagesInfo& PackagesInfo = PackagesToFullyLoad(MapIndex);
		/*
		for (INT PackageIndex = 0; PackageIndex < PackagesInfo.PackagesToLoad.Num(); PackageIndex++)
		{
			debugf(TEXT("--------------------- Considering: %i, %s, %s"),(INT)PackagesInfo.FullyLoadType, *PackagesInfo.Tag, *PackagesInfo.PackagesToLoad(PackageIndex).ToString());
		}
		*/

		// is this entry for the map/game?
		if (PackagesInfo.FullyLoadType == FullyLoadType && (PackagesInfo.Tag == Tag || Tag == TEXT("") || Tag == TEXT("___TAILONLY___")))
		{
			// go over all packages that need loading
			for (INT PackageIndex = 0; PackageIndex < PackagesInfo.PackagesToLoad.Num(); PackageIndex++)
			{
				//debugf(TEXT("------------------------ looking for %s"),*PackagesInfo.PackagesToLoad(PackageIndex).ToString());

				// look for the package in the package cache
				FString SFPackageName = PackagesInfo.PackagesToLoad(PackageIndex).ToString() + STANDALONE_SEEKFREE_SUFFIX;
				UBOOL bFoundFile = FALSE;
				FString PackagePath;
				if (GPackageFileCache->FindPackageFile(*SFPackageName, NULL, PackagePath))
				{
					bFoundFile = TRUE;
				}
				else if ( (GPackageFileCache->FindPackageFile(*PackagesInfo.PackagesToLoad(PackageIndex).ToString(), NULL, PackagePath)) )
				{
					bFoundFile = TRUE;
				}
				if (bFoundFile)
				{
					// load the package
					// @todo: This would be nice to be async probably, but how would we add it to the root? (LOAD_AddPackageToRoot?)
					//debugf(TEXT("------------------ Fully loading %s"), *PackagePath);
					UPackage* Package = UObject::LoadPackage(NULL, *PackagePath, 0);

					// add package to root so we can find it
					Package->AddToRoot();

					// remember the object for unloading later
					PackagesInfo.LoadedObjects.AddItem(Package);

					// add the objects to the root set so that it will not be GC'd
					for (TObjectIterator<UObject> It; It; ++It)
					{
						if (It->IsIn(Package))
						{
//							debugf(TEXT("Adding %s to root"), *It->GetFullName());
							It->AddToRoot();

							// remember the object for unloading later
							PackagesInfo.LoadedObjects.AddItem(*It);
						}
					}
				}
				else
				{
					debugf(TEXT("Failed to find Package %s to FullyLoad [FullyLoadType = %d, Tag = %s]"), *PackagesInfo.PackagesToLoad(PackageIndex).ToString(), (INT)FullyLoadType, *Tag);
				}
			}
		}
		/*
		else
		{
			debugf(TEXT("DIDN't MATCH!!!"));
			for (INT PackageIndex = 0; PackageIndex < PackagesInfo.PackagesToLoad.Num(); PackageIndex++)
			{
				debugf(TEXT("DIDN't MATCH!!! %i, \"%s\"(\"%s\"), %s"),(INT)PackagesInfo.FullyLoadType, *PackagesInfo.Tag, *Tag, *PackagesInfo.PackagesToLoad(PackageIndex).ToString());
			}
		}
		*/
	}
}

/**
 * Removes the PerMapPackages from the RootSet
 *
 * @param FullyLoadType When to load the packages (based on map, gametype, etc)
 * @param Tag Name of the map/game to cleanup packages for ("" will match any)
 */
void UGameEngine::CleanupPackagesToFullyLoad(EFullyLoadPackageType FullyLoadType, const FString& Tag)
{
	//debugf(TEXT("------------------ CleanupPackagesToFullyLoad: %i, %s"),(INT)FullyLoadType, *Tag);
	for (INT MapIndex = 0; MapIndex < PackagesToFullyLoad.Num(); MapIndex++)
	{
		FFullyLoadedPackagesInfo& PackagesInfo = PackagesToFullyLoad(MapIndex);
		// is this entry for the map/game?
		if (PackagesInfo.FullyLoadType == FullyLoadType && (PackagesInfo.Tag == Tag || Tag == TEXT("")))
		{
			// mark all objects from this map as unneeded
			for (INT ObjectIndex = 0; ObjectIndex < PackagesInfo.LoadedObjects.Num(); ObjectIndex++)
			{
//				debugf(TEXT("Removing %s from root"), *PackagesInfo.LoadedObjects(ObjectIndex)->GetFullName());
				PackagesInfo.LoadedObjects(ObjectIndex)->RemoveFromRoot();	
			}
			// empty the array of pointers to the objects
			PackagesInfo.LoadedObjects.Empty();
		}
	}
}

/**
* Creates a UNetDriver and associates a name with it.
*
* @param NetDriverName The name to associate with the driver.
*
* @return True if the driver was created successfully, false if there was an error.
*/
UBOOL UGameEngine::CreateNamedNetDriver(FName NetDriverName)
{
	// Try to create network driver.
	UClass* NetDriverClass = StaticLoadClass( UNetDriver::StaticClass(), NULL, TEXT("engine-ini:Engine.Engine.NetworkDevice"), NULL, LOAD_None, NULL );
	UNetDriver* NetDriver = ConstructObject<UNetDriver>( NetDriverClass );
	check(NetDriver);
	// Add it to the array.
	if(NetDriver)
	{
		const INT NewIndex = NamedNetDrivers.AddZeroed();
		FNamedNetDriver& NamedNetDriver = NamedNetDrivers(NewIndex);
		NamedNetDriver.NetDriverName = NetDriverName;
		NamedNetDriver.NetDriver = NetDriver;
		return TRUE;
	}
	return FALSE;
}

/**
* Destroys a UNetDriver based on its name.
*
* @param NetDriverName The name associated with the driver to destroy.
*/
void UGameEngine::DestroyNamedNetDriver(FName NetDriverName)
{
	for(INT Index = 0; Index < NamedNetDrivers.Num(); Index++)
	{
		FNamedNetDriver& NamedNetDriver = NamedNetDrivers(Index);
		if(NamedNetDriver.NetDriverName == NetDriverName)
		{
			// Close all connections.
			UNetDriver* NetDriver = NamedNetDriver.NetDriver;
			if(NetDriver->ServerConnection)
			{
				NetDriver->ServerConnection->Close();
			}
			for(INT ClientIndex = 0; ClientIndex < NetDriver->ClientConnections.Num(); ClientIndex++)
			{
				NetDriver->ClientConnections(ClientIndex)->Close();
			}
			// Remove it from the array.
			NamedNetDrivers.Remove(Index);
			return;
		}
	}
	check(0);
}

/**
* Finds a UNetDriver based on its name.
*
* @param NetDriverName The name associated with the driver to find.
*
* @return A pointer to the UNetDriver that was found, or NULL if it wasn't found.
*/
UNetDriver* UGameEngine::FindNamedNetDriver(FName NetDriverName)
{
	for(INT Index = 0; Index < NamedNetDrivers.Num(); Index++)
	{
		FNamedNetDriver& NamedNetDriver = NamedNetDrivers(Index);
		if(NamedNetDriver.NetDriverName == NetDriverName)
		{
			return NamedNetDriver.NetDriver;
		}
	}
	return NULL;
}
