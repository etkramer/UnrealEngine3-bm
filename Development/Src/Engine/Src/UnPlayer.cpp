/*=============================================================================
	UnPlayer.cpp: Unreal player implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
 
#include "EnginePrivate.h"
#include "CanvasScene.h"
#include "EngineUserInterfaceClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineUIPrivateClasses.h"
#include "EngineParticleClasses.h"
#include "EngineInterpolationClasses.h"
#include "EngineAnimClasses.h"
#include "EngineAudioDeviceClasses.h"
#include "EngineSoundClasses.h"
#include "SceneRenderTargets.h"

// needed for adding components when typing "show paths" in game
#include "EngineAIClasses.h"

#include "UnStatChart.h"
#include "UnTerrain.h"
#include "UnSubtitleManager.h"
#include "UnNet.h"
#include "PerfMem.h"
#include "ProfilingHelpers.h"

IMPLEMENT_CLASS(UPlayer);
IMPLEMENT_CLASS(ULocalPlayer);

/** High-res screenshot variables */
UBOOL		GIsTiledScreenshot = FALSE;
INT			GScreenshotResolutionMultiplier = 2;
INT			GScreenshotMargin = 64;	// In pixels
INT			GScreenshotTile = 0;
FIntRect	GScreenshotRect;		// Rendered tile
/** TRUE if we should grab a screenshot at the end of the frame */
UBOOL		GScreenShotRequest = FALSE;
FString     GScreenShotName = TEXT("");

/** Set to >0 when taking a screenshot, then counts down to 0.  When 0, do normal processing. */
INT			GGameScreenshotCounter = 0;

/** Whether to tick and render the UI. */
UBOOL		GTickAndRenderUI = TRUE;

#if PS3
/** Show GCM memory stat graphs (host,local) */
UBOOL GShowPS3GcmMemoryStats=FALSE;
#endif

UBOOL		ULocalPlayer::bOverrideView = FALSE;
FVector		ULocalPlayer::OverrideLocation;
FRotator	ULocalPlayer::OverrideRotation;

IMPLEMENT_CLASS(UGameViewportClient);
IMPLEMENT_CLASS(UPlayerManagerInteraction);

/** Whether to show the FPS counter */
UBOOL GShowFpsCounter = FALSE;
/** Whether to show the level stats */
static UBOOL GShowLevelStats = FALSE;
/** Whether to show the CPU thread and GPU frame times */
UBOOL GShowUnitTimes = FALSE;
/** WHether to show active sound waves */
UBOOL GShowSoundWaves = FALSE;

/**
 *	Renders the FPS counter
 *
 *	@param Viewport	The viewport to render to
 *	@param Canvas	Canvas object to use for rendering
 *	@param X		Suggested X coordinate for where to start drawing
 *	@param Y		Suggested Y coordinate for where to start drawing
 *	@return			Y coordinate of the next line after this output
 */
INT DrawFPSCounter( FViewport* Viewport, FCanvas* Canvas, INT X, INT Y );

/** This will set the StreamingLevels TMap with the current Streaming Level Status and also set which level the player is in **/
void GetLevelStremingStatus( TMap<FName,INT>& StreamingLevels, FString& LevelPlayerIsInName );


/**
 *	Draws frame times for the overall frame, gamethread, renderthread and GPU.
 *	The gamethread time excludes idle time while it's waiting for the render thread.
 *	The renderthread time excludes idle time while it's waiting for more commands from the gamethread or waiting for the GPU to swap backbuffers.
 *	The GPU time is a measurement on the GPU from the beginning of the first drawcall to the end of the last drawcall. It excludes
 *	idle time while waiting for VSYNC. However, it will include any starvation time between drawcalls.
 *
 *	@param Viewport	The viewport to render to
 *	@param Canvas	Canvas object to use for rendering
 *	@param X		Suggested X coordinate for where to start drawing
 *	@param Y		Suggested Y coordinate for where to start drawing
 *	@return			Y coordinate of the next line after this output
 */
INT DrawUnitTimes( FViewport* Viewport, FCanvas* Canvas, INT X, INT Y );

/**
 *	Render the level stats
 *
 *	@param Viewport	The viewport to render to
 *	@param Canvas	Canvas object to use for rendering
 *	@param X		Suggested X coordinate for where to start drawing
 *	@param Y		Suggested Y coordinate for where to start drawing
 *	@return			Y coordinate of the next line after this output
 */
INT DrawLevelStats( FViewport* Viewport, FCanvas* Canvas, INT X, INT Y );

/**
*	Render active sound waves
*
*	@param Viewport	The viewport to render to
*	@param Canvas	Canvas object to use for rendering
*	@param X		Suggested X coordinate for where to start drawing
*	@param Y		Suggested Y coordinate for where to start drawing
*	@return			Y coordinate of the next line after this output
*/
INT DrawSoundWaves( FViewport* Viewport, FCanvas* Canvas, INT X, INT Y );

UBOOL GShouldLogOutAFrameOfSkelCompTick = FALSE;
UBOOL GShouldLofOutAFrameOfLightEnvTick = FALSE;
UBOOL GShouldLogOutAFrameOfIsOverlapping = FALSE;
UBOOL GShouldLogOutAFrameOfMoveActor = FALSE;
UBOOL GShouldLogOutAFrameOfPhysAssetBoundsUpdate = FALSE;
UBOOL GShouldLogOutAFrameOfComponentUpdates = FALSE;

UBOOL GShouldTraceOutAFrameOfPain = FALSE;
UBOOL GShouldLogOutAFrameOfSkelCompLODs = FALSE;
UBOOL GShouldLogOutAFrameOfSkelMeshLODs = FALSE;
UBOOL GShouldLogOutAFrameOfFaceFXDebug = FALSE;
UBOOL GShouldLogOutAFrameOfFaceFXBones = FALSE;

UBOOL GShouldTraceFaceFX = FALSE;

/** UPlayerManagerInteraction */
/**
 * Routes an input key event to the player's interactions array
 *
 * @param	Viewport - The viewport which the key event is from.
 * @param	ControllerId - The controller which the key event is from.
 * @param	Key - The name of the key which an event occured for.
 * @param	Event - The type of event which occured.
 * @param	AmountDepressed - For analog keys, the depression percent.
 * @param	bGamepad - input came from gamepad (ie xbox controller)
 *
 * @return	True to consume the key event, false to pass it on.
 */
UBOOL UPlayerManagerInteraction::InputKey(INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed/*=1.f*/,UBOOL bGamepad/*=FALSE*/)
{
	UBOOL bResult = FALSE;

	INT PlayerIndex = UUIInteraction::GetPlayerIndex(ControllerId);
	if ( GEngine->GamePlayers.IsValidIndex(PlayerIndex) )
	{
		ULocalPlayer* TargetPlayer = GEngine->GamePlayers(PlayerIndex);
		if ( TargetPlayer != NULL && TargetPlayer->Actor != NULL )
		{
			APlayerController* PC = TargetPlayer->Actor;
			for ( INT InteractionIndex = 0; !bResult && InteractionIndex < PC->Interactions.Num(); InteractionIndex++ )
			{
				UInteraction* PlayerInteraction = PC->Interactions(InteractionIndex);
				if ( OBJ_DELEGATE_IS_SET(PlayerInteraction,OnReceivedNativeInputKey) )
				{
					bResult = PlayerInteraction->delegateOnReceivedNativeInputKey(ControllerId, Key, Event, AmountDepressed, bGamepad);
				}

				bResult = bResult || PlayerInteraction->InputKey(ControllerId, Key, Event, AmountDepressed, bGamepad);
			}
		}
	}

	return bResult;
}

/**
 * Routes an axis input event to the player's interactions array.
 *
 * @param	Viewport - The viewport which the axis movement is from.
 * @param	ControllerId - The controller which the axis movement is from.
 * @param	Key - The name of the axis which moved.
 * @param	Delta - The axis movement delta.
 * @param	DeltaTime - The time since the last axis update.
 *
 * @return	True to consume the axis movement, false to pass it on.
 */
UBOOL UPlayerManagerInteraction::InputAxis( INT ControllerId, FName Key, FLOAT Delta, FLOAT DeltaTime, UBOOL bGamepad/*=FALSE*/ )
{
	UBOOL bResult = FALSE;

	INT PlayerIndex = UUIInteraction::GetPlayerIndex(ControllerId);
	if ( GEngine->GamePlayers.IsValidIndex(PlayerIndex) )
	{
		ULocalPlayer* TargetPlayer = GEngine->GamePlayers(PlayerIndex);
		if ( TargetPlayer != NULL && TargetPlayer->Actor != NULL )
		{
			APlayerController* PC = TargetPlayer->Actor;
			for ( INT InteractionIndex = 0; !bResult && InteractionIndex < PC->Interactions.Num(); InteractionIndex++ )
			{
				UInteraction* PlayerInteraction = PC->Interactions(InteractionIndex);
				if ( OBJ_DELEGATE_IS_SET(PlayerInteraction,OnReceivedNativeInputAxis) )
				{
					bResult = PlayerInteraction->delegateOnReceivedNativeInputAxis(ControllerId, Key, Delta, DeltaTime, bGamepad);
				}

				bResult = bResult || PlayerInteraction->InputAxis(ControllerId, Key, Delta, DeltaTime, bGamepad);
			}
		}
	}

	return bResult;
}

/**
 * Routes a character input to the player's Interaction array.
 *
 * @param	Viewport - The viewport which the axis movement is from.
 * @param	ControllerId - The controller which the axis movement is from.
 * @param	Character - The character.
 *
 * @return	True to consume the character, false to pass it on.
 */
UBOOL UPlayerManagerInteraction::InputChar( INT ControllerId, TCHAR Character )
{
	UBOOL bResult = FALSE;

	INT PlayerIndex = UUIInteraction::GetPlayerIndex(ControllerId);
	if ( GEngine->GamePlayers.IsValidIndex(PlayerIndex) )
	{
		ULocalPlayer* TargetPlayer = GEngine->GamePlayers(PlayerIndex);
		if ( TargetPlayer != NULL && TargetPlayer->Actor != NULL )
		{
			APlayerController* PC = TargetPlayer->Actor;
			for ( INT InteractionIndex = 0; !bResult && InteractionIndex < PC->Interactions.Num(); InteractionIndex++ )
			{
				UInteraction* PlayerInteraction = PC->Interactions(InteractionIndex);
				if ( OBJ_DELEGATE_IS_SET(PlayerInteraction,OnReceivedNativeInputChar) )
				{
					TCHAR CharString[2] = { Character, 0 };
					bResult = PlayerInteraction->delegateOnReceivedNativeInputChar(ControllerId, CharString);
				}

				bResult = bResult || PlayerInteraction->InputChar(ControllerId, Character);
			}
		}
	}

	return bResult;
}

UGameViewportClient::UGameViewportClient():
	ShowFlags((SHOW_DefaultGame&~SHOW_ViewMode_Mask)|SHOW_ViewMode_Lit)
{
	bUIMouseCaptureOverride = FALSE;
	bDisplayingUIMouseCursor = FALSE;
}

/**
 * Cleans up all rooted or referenced objects created or managed by the GameViewportClient.  This method is called
 * when this GameViewportClient has been disassociated with the game engine (i.e. is no longer the engine's GameViewport).
 */
void UGameViewportClient::DetachViewportClient()
{
	// notify all interactions to clean up their own references
	eventGameSessionEnded();

	// if we have a UIController, tear it down now
	if ( UIController != NULL )
	{
		UIController->TearDownUI();
	}

	UIController = NULL;
	ViewportConsole = NULL;
	RemoveFromRoot();
}

/**
 * Called every frame to allow the game viewport to update time based state.
 * @param	DeltaTime - The time since the last call to Tick.
 */
void UGameViewportClient::Tick( FLOAT DeltaTime )
{
	// first call the unrealscript tick
	eventTick(DeltaTime);

	// now tick all interactions
	for ( INT i = 0; i < GlobalInteractions.Num(); i++ )
	{
		UInteraction* Interaction = GlobalInteractions(i);
		Interaction->Tick(DeltaTime);
	}

	// Reset the flag. It will be turned on again if we actually draw our cursor.
	bDisplayingUIMouseCursor = FALSE;
}

FString UGameViewportClient::ConsoleCommand(const FString& Command)
{
	FString TruncatedCommand = Command.Left(1000);
	FConsoleOutputDevice ConsoleOut(ViewportConsole);
	Exec(*TruncatedCommand,ConsoleOut);
	return *ConsoleOut;
}

/**
 * Routes an input key event received from the viewport to the Interactions array for processing.
 *
 * @param	Viewport		the viewport the input event was received from
 * @param	ControllerId	gamepad/controller that generated this input event
 * @param	Key				the name of the key which an event occured for (KEY_Up, KEY_Down, etc.)
 * @param	EventType		the type of event which occured (pressed, released, etc.)
 * @param	AmountDepressed	(analog keys only) the depression percent.
 * @param	bGamepad - input came from gamepad (ie xbox controller)
 *
 * @return	TRUE to consume the key event, FALSE to pass it on.
 */
UBOOL UGameViewportClient::InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent EventType,FLOAT AmountDepressed,UBOOL bGamepad)
{

#if !CONSOLE
	// if a movie is playing then handle input key
	if( GFullScreenMovie && 
		GFullScreenMovie->GameThreadIsMoviePlaying(TEXT("")) )
	{
		if ( GFullScreenMovie->InputKey(Viewport,ControllerId,Key,EventType,AmountDepressed,bGamepad) )
		{
			return TRUE;
		}
	}
#endif

	UBOOL bResult = FALSE;

	if ( DELEGATE_IS_SET(HandleInputKey) )
	{
		bResult = delegateHandleInputKey(ControllerId, Key, EventType, AmountDepressed, bGamepad);
	}

	// if it wasn't handled by script, route to the interactions array
	for ( INT InteractionIndex = 0; !bResult && InteractionIndex < GlobalInteractions.Num(); InteractionIndex++ )
	{
		UInteraction* Interaction = GlobalInteractions(InteractionIndex);
		if ( OBJ_DELEGATE_IS_SET(Interaction,OnReceivedNativeInputKey) )
		{
			bResult = Interaction->delegateOnReceivedNativeInputKey(ControllerId, Key, EventType, AmountDepressed, bGamepad);
		}

		bResult = bResult || Interaction->InputKey(ControllerId, Key, EventType, AmountDepressed, bGamepad);
	}

	return bResult;
}

/**
 * Routes an input axis (joystick, thumbstick, or mouse) event received from the viewport to the Interactions array for processing.
 *
 * @param	Viewport		the viewport the input event was received from
 * @param	ControllerId	the controller that generated this input axis event
 * @param	Key				the name of the axis that moved  (KEY_MouseX, KEY_XboxTypeS_LeftX, etc.)
 * @param	Delta			the movement delta for the axis
 * @param	DeltaTime		the time (in seconds) since the last axis update.
 *
 * @return	TRUE to consume the axis event, FALSE to pass it on.
 */
UBOOL UGameViewportClient::InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime, UBOOL bGamepad)
{
	UBOOL bResult = FALSE;

	// give script the chance to process this input first
	if ( DELEGATE_IS_SET(HandleInputAxis) )
	{
		bResult = delegateHandleInputAxis(ControllerId, Key, Delta, DeltaTime, bGamepad);
	}

	// if it wasn't handled by script, route to the interactions array
	for ( INT InteractionIndex = 0; !bResult && InteractionIndex < GlobalInteractions.Num(); InteractionIndex++ )
	{
		UInteraction* Interaction = GlobalInteractions(InteractionIndex);
		if ( OBJ_DELEGATE_IS_SET(Interaction,OnReceivedNativeInputAxis) )
		{
			bResult = Interaction->delegateOnReceivedNativeInputAxis(ControllerId, Key, Delta, DeltaTime, bGamepad);
		}

		bResult = bResult || Interaction->InputAxis(ControllerId, Key, Delta, DeltaTime, bGamepad);
	}

	return bResult;
}

/**
 * Routes a character input event (typing) received from the viewport to the Interactions array for processing.
 *
 * @param	Viewport		the viewport the input event was received from
 * @param	ControllerId	the controller that generated this character input event
 * @param	Character		the character that was typed
 *
 * @return	TRUE to consume the key event, FALSE to pass it on.
 */
UBOOL UGameViewportClient::InputChar(FViewport* Viewport,INT ControllerId,TCHAR Character)
{
	UBOOL bResult = FALSE;

	// should probably just add a ctor to FString that takes a TCHAR
	FString CharacterString;
	CharacterString += Character;

	if ( DELEGATE_IS_SET(HandleInputChar) )
	{
		bResult = delegateHandleInputChar(ControllerId, CharacterString);
	}

	// if it wasn't handled by script, route to the interactions array
	for ( INT InteractionIndex = 0; !bResult && InteractionIndex < GlobalInteractions.Num(); InteractionIndex++ )
	{
		UInteraction* Interaction = GlobalInteractions(InteractionIndex);
		if ( OBJ_DELEGATE_IS_SET(Interaction,OnReceivedNativeInputChar) )
		{
			bResult = Interaction->delegateOnReceivedNativeInputChar(ControllerId, CharacterString);
		}

		bResult = bResult || Interaction->InputChar(ControllerId, Character);
	}

	return bResult;
}

/**
 * Returns the forcefeedback manager associated with the PlayerController.
 */
class UForceFeedbackManager* UGameViewportClient::GetForceFeedbackManager(INT ControllerId)
{
	for(INT PlayerIndex = 0;PlayerIndex < GEngine->GamePlayers.Num();PlayerIndex++)
	{
		ULocalPlayer* Player = GEngine->GamePlayers(PlayerIndex);
		if(Player->ViewportClient == this && Player->ControllerId == ControllerId)
		{
			// Only play force feedback on gamepad
			if( Player->Actor && 
				Player->Actor->ForceFeedbackManager )
			{
				return Player->Actor->ForceFeedbackManager;
			}
			break;
		}
	}

	return NULL;
}


/** @returns whether the controller is active **/
UBOOL UGameViewportClient::IsControllerTiltActive( INT ControllerID ) const
{
	//warnf( TEXT( "UGameViewportClient::SetControllerTiltActive" ) );
	return FALSE;
}

void UGameViewportClient::SetControllerTiltDesiredIfAvailable( INT ControllerID, UBOOL bActive )
{
	//warnf( TEXT( "UGameViewportClient::SetControllerTiltDesiredIfAvailable" ) );
}

void UGameViewportClient::SetControllerTiltActive( INT ControllerID, UBOOL bActive )
{
	//warnf( TEXT( "UGameViewportClient::SetControllerTiltActive" ) );
}

void UGameViewportClient::SetOnlyUseControllerTiltInput( INT ControllerID, UBOOL bActive )
{
	//warnf( TEXT( "UGameViewportClient::SetOnlyUseControllerTiltInput" ) );
}

void UGameViewportClient::SetUseTiltForwardAndBack( INT ControllerID, UBOOL bActive )
{
	//warnf( TEXT( "UGameViewportClient::SetOnlyUseControllerTiltInput" ) );
}

/** @return whether or not this Controller has a keyboard available to be used **/
UBOOL UGameViewportClient::IsKeyboardAvailable( INT ControllerID ) const
{
	//warnf( TEXT( "UGameViewportClient::IsKeyboardAvailable" ) );
	return Viewport == NULL || Viewport->IsKeyboardAvailable(ControllerID);
}

/** @return whether or not this Controller has a mouse available to be used **/
UBOOL UGameViewportClient::IsMouseAvailable( INT ControllerID ) const
{
	//warnf( TEXT( "UGameViewportClient::IsMouseAvailable" ) );
	return Viewport == NULL || Viewport->IsMouseAvailable(ControllerID);
}


/**
 * Determines whether this viewport client should receive calls to InputAxis() if the game's window is not currently capturing the mouse.
 * Used by the UI system to easily receive calls to InputAxis while the viewport's mouse capture is disabled.
 */
UBOOL UGameViewportClient::RequiresUncapturedAxisInput() const
{
	return Viewport != NULL && bDisplayingUIMouseCursor == TRUE && Viewport->HasFocus();
}

/**
 * Retrieves the cursor that should be displayed by the OS
 *
 * @param	Viewport	the viewport that contains the cursor
 * @param	X			the client x position of the cursor
 * @param	Y			the client Y position of the cursor
 * 
 * @return	the cursor that the OS should display
 */
EMouseCursor UGameViewportClient::GetCursor( FViewport* Viewport, INT X, INT Y )
{
	UBOOL bIsPlayingMovie = GFullScreenMovie && GFullScreenMovie->GameThreadIsMoviePlaying( TEXT("") );

#if CONSOLE || PLATFORM_UNIX
	UBOOL bIsWithinTitleBar = FALSE;
#else
	POINT CursorPos = { X, Y };
	RECT WindowRect;
	ClientToScreen( (HWND)Viewport->GetWindow(), &CursorPos );
	GetWindowRect( (HWND)Viewport->GetWindow(), &WindowRect );
	UBOOL bIsWithinWindow = ( CursorPos.x >= WindowRect.left && CursorPos.x <= WindowRect.right &&
							  CursorPos.y >= WindowRect.top && CursorPos.y <= WindowRect.bottom );

	// The user is mousing over the title bar if Y is less than zero and within the window rect
	UBOOL bIsWithinTitleBar = Y < 0 && bIsWithinWindow;
#endif

	if ( (bDisplayingUIMouseCursor || !bIsPlayingMovie) && (Viewport->IsFullscreen() || !bIsWithinTitleBar) )
	{
		//@todo - returning MC_None causes the OS to not render a mouse...might want to change
		// this so that UI can indicate which mouse cursor should be displayed
		return MC_None;
	}

	return FViewportClient::GetCursor(Viewport, X, Y);
}

/**
 * Callback to let the game engine know the UI software mouse cursor is being rendered.
 *
 * @param	bVisible	Whether the UI software mouse cursor is visible or not
 */
void UGameViewportClient::OnShowUIMouseCursor( UBOOL bVisible )
{
	bDisplayingUIMouseCursor = bVisible;
}


void UGameViewportClient::SetDropDetail(FLOAT DeltaSeconds)
{
	if (GEngine->Client)
	{
#if CONSOLE
		// Externs to detailed frame stats, split by render/ game thread CPU time and GPU time.
		extern DWORD GRenderThreadTime;
		extern DWORD GGameThreadTime;

#if XBOX
		// GPU frame times is not supported in final release. We don't want to change how this works across configurations
		// so we simply disable looking at GPU frame time on Xbox 360.
		const DWORD GGPUFrameTime = 0;
#else
		extern DWORD GGPUFrameTime;
#endif // XBOX

		// Calculate the maximum time spent, EXCLUDING idle time. We don't use DeltaSeconds as it includes idle time waiting
		// for VSYNC so we don't know how much "buffer" we have.
		FLOAT FrameTime	= Max3<DWORD>( GRenderThreadTime, GGameThreadTime, GGPUFrameTime ) * GSecondsPerCycle;
		// If DeltaSeconds is bigger than 34 ms we can take it into account as we're not VSYNCing in that case.
		if( DeltaSeconds > 0.034 )
		{
			FrameTime = Max( FrameTime, DeltaSeconds );
		}
		const FLOAT FrameRate	= FrameTime > 0 ? 1 / FrameTime : 0;
#else
		FLOAT FrameTime = DeltaSeconds;
		const FLOAT FrameRate	= DeltaSeconds > 0 ? 1 / DeltaSeconds : 0;
#endif // CONSOLE

		// Drop detail if framerate is below threshold.
		AWorldInfo* WorldInfo		= GWorld->GetWorldInfo();
		WorldInfo->bDropDetail		= FrameRate < Clamp(GEngine->Client->MinDesiredFrameRate, 1.f, 100.f) && !GIsBenchmarking;
		WorldInfo->bAggressiveLOD	= FrameRate < Clamp(GEngine->Client->MinDesiredFrameRate - 5.f, 1.f, 100.f) && !GIsBenchmarking;


// this is slick way to be able to do something based on the frametime and whether we are bound by one thing or another
#if 0 
		// so where we check to see if we are above some threshold and below 150 ms (any thing above that is usually blocking loading of some sort)
		// also we don't want to do the auto trace when we are blocking on async loading
		if( ( 0.070 < FrameTime ) && ( FrameTime < 0.150 ) && GIsAsyncLoading == FALSE && GWorld->GetWorldInfo()->bRequestedBlockOnAsyncLoading == FALSE && (GWorld->GetTimeSeconds() > 30.0f )  )
		{
			// now check to see if we have done a trace in the last 30 seconds otherwise we will just trace ourselves to death
			static FLOAT LastTraceTime = -9999.0f;
			if( (LastTraceTime+30.0f < GWorld->GetTimeSeconds()))
			{
				LastTraceTime = GWorld->GetTimeSeconds();
				warnf(TEXT("Auto Trace initiated!! FrameTime: %f"), FrameTime );

				// do what ever action you want here (e.g. trace <type>, GShouldLogOutAFrameOfSkelCompTick = TRUE, c.f. UnLevTic.cpp for more)
				//GShouldLogOutAFrameOfSkelCompTick = TRUE;
				//GShouldLogOutAFrameOfIsOverlapping = TRUE;
				//GShouldLogOutAFrameOfMoveActor = TRUE;
				//GShouldLogOutAFrameOfPhysAssetBoundsUpdate = TRUE;
				//GShouldLogOutAFrameOfComponentUpdates = TRUE;
			
#if CONSOLE
				warnf(TEXT("    GGameThreadTime: %d GRenderThreadTime: %d "), GGameThreadTime, GRenderThreadTime );
				if( GGameThreadTime > GRenderThreadTime )
				{
					//GShouldTraceOutAFrameOfPain = TRUE;
					//appStartCPUTrace( NAME_Game, FALSE, FALSE, 40, NULL );
					//appStopCPUTrace( NAME_Game );
				}
				else
				{
					//appStartCPUTrace( TEXT("Render"), FALSE, FALSE, 40, NULL );
					//appStopCPUTrace( TEXT("Render") );
				}
#endif // CONSOLE
			}
		}
#endif // 0 
	}
}

/**
 * Set this GameViewportClient's viewport and viewport frame to the viewport specified
 */
void UGameViewportClient::SetViewportFrame( FViewportFrame* InViewportFrame )
{
	ViewportFrame = InViewportFrame;
	SetViewport( ViewportFrame ? ViewportFrame->GetViewport() : NULL );
}

/**
 * Set this GameViewportClient's viewport to the viewport specified
 */
void UGameViewportClient::SetViewport( FViewport* InViewport )
{
	FViewport* PreviousViewport = Viewport;
	Viewport = InViewport;

	if ( PreviousViewport == NULL && Viewport != NULL )
	{
		// ensure that the player's Origin and Size members are initialized the moment we get a viewport
		eventLayoutPlayers();
	}

	if ( UIController != NULL )
	{
		UIController->SceneClient->SetRenderViewport(Viewport);
	}

}


/**
 * Retrieve the size of the main viewport.
 *
 * @param	out_ViewportSize	[out] will be filled in with the size of the main viewport
 */
void UGameViewportClient::GetViewportSize( FVector2D& out_ViewportSize )
{
	if ( Viewport != NULL )
	{
		out_ViewportSize.X = Viewport->GetSizeX();
		out_ViewportSize.Y = Viewport->GetSizeY();
	}
}

/** @return Whether or not the main viewport is fullscreen or windowed. */
UBOOL UGameViewportClient::IsFullScreenViewport()
{
	return Viewport->IsFullscreen();
}


/**
 * Determine whether a fullscreen viewport should be used in cases where there are multiple players.
 *
 * @return	TRUE to use a fullscreen viewport; FALSE to allow each player to have their own area of the viewport.
 */
UBOOL UGameViewportClient::ShouldForceFullscreenViewport() const
{
	UBOOL bResult = FALSE;

	if ( GetOuterUEngine()->GamePlayers.Num() == 0 )
	{
		bResult = TRUE;
	}
	else if ( GWorld != NULL && GWorld->GetWorldInfo() != NULL && GWorld->GetWorldInfo()->IsMenuLevel() )
	{
		bResult = TRUE;
	}
	else
	{
		ULocalPlayer* FirstPlayer = GetOuterUEngine()->GamePlayers(0);
		if ( FirstPlayer != NULL && FirstPlayer->Actor != NULL && FirstPlayer->Actor->bCinematicMode )
		{
			bResult = TRUE;
		}
	}

	return bResult;
}

/** Whether we should precache during the next frame. */
UBOOL GPrecacheNextFrame = FALSE;
/** Whether texture memory has been corrupted because we ran out of memory in the pool. */
UBOOL GIsTextureMemoryCorrupted = FALSE;

#if !FINAL_RELEASE
/** Whether PrepareMapChange is attempting to load a map that doesn't exist */
UBOOL GIsPrepareMapChangeBroken = FALSE;

/** draws a property of the given object on the screen similarly to stats */
static void DrawProperty(UCanvas* CanvasObject, UObject* Obj, const FDebugDisplayProperty& PropData, UProperty* Prop, INT X, INT& Y)
{
	checkSlow(PropData.bSpecialProperty || Prop != NULL);
	checkSlow(Prop == NULL || Obj->GetClass()->IsChildOf(Prop->GetOwnerClass()));

	FCanvas* Canvas = CanvasObject->Canvas;
	FString PropText, ValueText;
	if (!PropData.bSpecialProperty)
	{
		PropText = FString::Printf(TEXT("%s.%s.%s = "), *Obj->GetOutermost()->GetName(), *Obj->GetName(), *Prop->GetName());
		if (Prop->ArrayDim == 1)
		{
			Prop->ExportText(0, ValueText, (BYTE*)Obj, (BYTE*)Obj, Obj, PPF_Localized);
		}
		else
		{
			ValueText += TEXT("(");
			for (INT i = 0; i < Prop->ArrayDim; i++)
			{
				Prop->ExportText(i, ValueText, (BYTE*)Obj, (BYTE*)Obj, Obj, PPF_Localized);
				if (i + 1 < Prop->ArrayDim)
				{
					ValueText += TEXT(",");
				}
			}
			ValueText += TEXT(")");
		}
	}
	else
	{
		PropText = FString::Printf(TEXT("%s.%s.(%s) = "), *Obj->GetOutermost()->GetName(), *Obj->GetName(), *PropData.PropertyName.ToString());
		if (PropData.PropertyName == NAME_State)
		{
			ValueText = (Obj->GetStateFrame() != NULL) ? *Obj->GetStateFrame()->StateNode->GetName() : TEXT("None");
		}
	}
	INT XL, YL;
	CanvasObject->ClippedStrLen(GEngine->SmallFont, 1.0f, 1.0f, XL, YL, *PropText);
	FRenderParameters DrawParams(X, Y, CanvasObject->SizeX - X, 0, GEngine->SmallFont);
	TArray<FWrappedStringElement> TextLines;
	UUIString::WrapString(DrawParams, X + XL, *ValueText, TextLines);
	INT XL2 = XL;
	if (TextLines.Num() > 0)
	{
		XL2 += appTrunc(TextLines(0).LineExtent.X);
		for (INT i = 1; i < TextLines.Num(); i++)
		{
			XL2 = Max<INT>(XL2, appTrunc(TextLines(i).LineExtent.X));
		}
	}
	DrawTile( Canvas, X, Y, XL2 + 1, YL * Max<INT>(TextLines.Num(), 1), 0, 0, CanvasObject->DefaultTexture->SizeX, CanvasObject->DefaultTexture->SizeY,
				FLinearColor(0.5f, 0.5f, 0.5f, 0.5f), CanvasObject->DefaultTexture->Resource );
	DrawShadowedString(Canvas, X, Y, *PropText, GEngine->SmallFont, FLinearColor(0.0f, 1.0f, 0.0f));
	if (TextLines.Num() > 0)
	{
		DrawShadowedString(Canvas, X + XL, Y, *TextLines(0).Value, GEngine->SmallFont, FLinearColor(1.0f, 0.0f, 0.0f));
		for (INT i = 1; i < TextLines.Num(); i++)
		{
			DrawShadowedString(Canvas, X, Y + YL * i, *TextLines(i).Value, GEngine->SmallFont, FLinearColor(1.0f, 0.0f, 0.0f));
		}
		Y += YL * TextLines.Num();
	}
	else
	{
		Y += YL;
	}
}
#endif

void UGameViewportClient::Draw(FViewport* Viewport,FCanvas* Canvas)
{
	GSystemSettings.UpdateSplitScreenSettings((ESplitScreenType)ActiveSplitscreenType);

	if(GPrecacheNextFrame)
	{
#if !CONSOLE
		FSceneViewFamilyContext PrecacheViewFamily(Viewport,GWorld->Scene,ShowFlags,GWorld->GetTimeSeconds(),GWorld->GetDeltaSeconds(),GWorld->GetRealTimeSeconds(),NULL);
		PrecacheViewFamily.Views.AddItem(
			new FSceneView(
				&PrecacheViewFamily,
				NULL,
				-1,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				0,
				0,
				1,
				1,
				FMatrix(
					FPlane(1.0f / WORLD_MAX,0.0f,0.0f,0.0f),
					FPlane(0.0f,1.0f / WORLD_MAX,0.0f,0.0f),
					FPlane(0.0f,0.0f,1.0f / WORLD_MAX,0.0f),
					FPlane(0.5f,0.5f,0.5f,1.0f)
					),
				FMatrix::Identity,
				FLinearColor::Black,
				FLinearColor::Black,
				FLinearColor::White,
				TSet<UPrimitiveComponent*>()
				)
			);
		BeginRenderingViewFamily(Canvas,&PrecacheViewFamily);
#endif

		GPrecacheNextFrame = FALSE;
	}

	// update ScriptedTextures
	for (INT i = 0; i < UScriptedTexture::GScriptedTextures.Num(); i++)
	{
		UScriptedTexture::GScriptedTextures(i)->CheckUpdate();
	}

	// Create a temporary canvas if there isn't already one.
	UCanvas* CanvasObject = FindObject<UCanvas>(UObject::GetTransientPackage(),TEXT("CanvasObject"));
	if( !CanvasObject )
	{
		CanvasObject = ConstructObject<UCanvas>(UCanvas::StaticClass(),UObject::GetTransientPackage(),TEXT("CanvasObject"));
		CanvasObject->AddToRoot();
	}
	CanvasObject->Canvas = Canvas;	

	// Setup a FSceneViewFamily for the player.
	if( GWorld->bGatherDynamicShadowStatsSnapshot )
	{
		// Start with clean slate.
		GWorld->DynamicShadowStats.Empty();
	}

	// see if the UI has post processing
	UUISceneClient* UISceneClient = UUIRoot::GetSceneClient(); 
	
	UBOOL bUIDisableWorldRendering = FALSE;
	if (UISceneClient)
	{
		UGameUISceneClient* GameSceneClient = Cast<UGameUISceneClient>(UISceneClient);
		const INT NumScenes = GameSceneClient->GetActiveSceneCount();
		for (INT i=0;i<NumScenes;i++)
		{
			UUIScene* Scene = GameSceneClient->GetSceneAtIndex(i);
			if ( Scene != NULL )
			{
				bUIDisableWorldRendering |= Scene->bDisableWorldRendering;
			}
		}
	}

	// If the UI should be rendered with post processing all of the UI elements render 
	// to the scene color RT instead of directly to the viewport RT.  The final copy to
	// the viewport RT will occur after UI post processing has finished.
	UBOOL bUISceneHasPostProcess = UISceneClient 
		&& UISceneClient->UsesPostProcess()
		// Disallow UI post processing with upscaling as the UI text will be rendered 
		// before the upscale in this case which will result in low res unreadable text.
		&& !GSystemSettings.NeedsUpscale();

	// see if canvas scene has any primitives
	FCanvasScene* CanvasScene = UIController->UsesUIPrimitiveScene() ? UIController->GetUIPrimitiveScene() : NULL;
	UBOOL bUISceneHasPrimitives = (CanvasScene && CanvasScene->GetNumPrimitives() > 0);
	// disable final copy to viewport RT if there are canvas prims or the ui scene needs to be post processed
	UBOOL bDisableViewportCopy = GTickAndRenderUI && (bUISceneHasPostProcess || bUISceneHasPrimitives);

	// create the view family for rendering the world scene to the viewport's render target
	FSceneViewFamilyContext ViewFamily(
		Viewport,
		GWorld->Scene,
		ShowFlags,
		GWorld->GetTimeSeconds(),
		GWorld->GetDeltaSeconds(),
		GWorld->GetRealTimeSeconds(), 
		GWorld->bGatherDynamicShadowStatsSnapshot ? &GWorld->DynamicShadowStats : NULL, 
		TRUE,
		TRUE,
		FALSE,
		GEngine->bEnableColorClear,		// disable the initial scene color clear in game
		!bDisableViewportCopy			// disable the final copy to the viewport RT if still rendering to scene color
		);
	TMap<INT,FSceneView*> PlayerViewMap;
	UBOOL bHaveReverbSettingsBeenSet = FALSE;
	for(INT PlayerIndex = 0;PlayerIndex < GEngine->GamePlayers.Num();PlayerIndex++)
	{
		ULocalPlayer* Player = GEngine->GamePlayers(PlayerIndex);
		if (Player->Actor)
		{
			// Calculate the player's view information.
			FVector		ViewLocation;
			FRotator	ViewRotation;

			FSceneView* View = Player->CalcSceneView( &ViewFamily, ViewLocation, ViewRotation, Viewport );
			if(View)
			{
				// Save the location of the view.
				Player->LastViewLocation = ViewLocation;
				
				// Tiled rendering for high-res screenshots.
				if ( GIsTiledScreenshot )
				{
					// Calculate number of overlapping tiles:
					INT TileWidth	= Viewport->GetSizeX();
					INT TileHeight	= Viewport->GetSizeY();
					INT TotalWidth	= GScreenshotResolutionMultiplier * TileWidth;
					INT TotalHeight	= GScreenshotResolutionMultiplier * TileHeight;
					INT NumColumns	= appCeil( FLOAT(TotalWidth) / FLOAT(TileWidth - 2*GScreenshotMargin) );
					INT NumRows		= appCeil( FLOAT(TotalHeight) / FLOAT(TileHeight - 2*GScreenshotMargin) );
					TileWidth		= appTrunc(View->SizeX);
					TileHeight		= appTrunc(View->SizeY);
					TotalWidth		= GScreenshotResolutionMultiplier * TileWidth;
					TotalHeight		= GScreenshotResolutionMultiplier * TileHeight;

					// Report back to UD3DRenderDevice::TiledScreenshot():
					GScreenshotRect.Min.X = appTrunc(View->X);
					GScreenshotRect.Min.Y = appTrunc(View->Y);
					GScreenshotRect.Max.X = appTrunc(View->X + View->SizeX);
					GScreenshotRect.Max.Y = appTrunc(View->Y + View->SizeY);

					// Calculate tile position (upper-left corner, screen space):
					INT TileRow		= GScreenshotTile / NumColumns;
					INT TileColumn	= GScreenshotTile % NumColumns;
					INT PosX		= TileColumn*TileWidth - (2*TileColumn + 1)*GScreenshotMargin;
					INT PosY		= TileRow*TileHeight - (2*TileRow + 1)*GScreenshotMargin;

					// Calculate offsets to center tile (screen space):
					INT OffsetX		= (TotalWidth - TileWidth) / 2 - PosX;
					INT OffsetY		= (TotalHeight - TileHeight) / 2 - PosY;

					// Convert to projection space:
					FLOAT Scale		= FLOAT(GScreenshotResolutionMultiplier);
					FLOAT OffsetXp	= 2.0f * FLOAT(OffsetX) / FLOAT(TotalWidth);
					FLOAT OffsetYp	= -2.0f * FLOAT(OffsetY) / FLOAT(TotalHeight);

					// Apply offsets and scales:
					FTranslationMatrix OffsetMtx( FVector( OffsetXp, OffsetYp, 0.0f) );
					FScaleMatrix ScaleMtx( FVector(Scale, Scale, 1.0f) );
					View->ProjectionMatrix = View->ProjectionMatrix * OffsetMtx * ScaleMtx;
					View->InvProjectionMatrix = View->ProjectionMatrix.Inverse();
					View->ViewProjectionMatrix = View->ViewMatrix * View->ProjectionMatrix;
					View->InvViewProjectionMatrix = View->ViewProjectionMatrix.Inverse();
					View->TranslatedViewMatrix = FTranslationMatrix(-View->PreViewTranslation) * View->ViewMatrix;
					View->TranslatedViewProjectionMatrix = View->TranslatedViewMatrix * View->ProjectionMatrix;
					View->InvTranslatedViewProjectionMatrix = View->TranslatedViewProjectionMatrix.Inverse();
					//RI->SetOrigin2D( -PosX, -PosY );
					//RI->SetZoom2D( Scale );
				}

				PlayerViewMap.Set(PlayerIndex,View);

				// Update the listener.
				check(GEngine->Client);
				UAudioDevice* AudioDevice = GEngine->Client->GetAudioDevice();
				if( AudioDevice )
				{
					FMatrix CameraToWorld		= View->ViewMatrix.Inverse();

					FVector ProjUp				= CameraToWorld.TransformNormal(FVector(0,1000,0));
					FVector ProjRight			= CameraToWorld.TransformNormal(FVector(1000,0,0));
					FVector ProjFront			= ProjRight ^ ProjUp;

					ProjUp.Z = Abs( ProjUp.Z ); // Don't allow flipping "up".

					ProjUp.Normalize();
					ProjRight.Normalize();
					ProjFront.Normalize();

					AudioDevice->SetListener(PlayerIndex, GEngine->GamePlayers.Num(), ViewLocation, ProjUp, ProjRight, ProjFront);

					// Update reverb settings based on the view of the first player we encounter.
					if ( !bHaveReverbSettingsBeenSet )
					{
						bHaveReverbSettingsBeenSet = TRUE;
						FReverbSettings ReverbSettings;
						AReverbVolume* ReverbVolume = GWorld->GetWorldInfo()->GetReverbSettings( ViewLocation, TRUE/*bReverbVolumePrevis*/, ReverbSettings );
						AudioDevice->SetReverbSettings( ReverbSettings );
					}
				}

				if (!bDisableWorldRendering && !bUIDisableWorldRendering)
				{
					// Set the canvas transform for the player's view rectangle.
					CanvasObject->Init();
					CanvasObject->SizeX = appTrunc(View->SizeX);
					CanvasObject->SizeY = appTrunc(View->SizeY);
					CanvasObject->SceneView = View;
					CanvasObject->Update();
					Canvas->PushAbsoluteTransform(FTranslationMatrix(FVector(View->X,View->Y,0)));

					// PreRender the player's view.
					Player->Actor->eventPreRender(CanvasObject);

					Canvas->PopTransform();
				}

				// Add view information for resource streaming.
				GStreamingManager->AddViewInformation( View->ViewOrigin, View->SizeX, View->SizeX * View->ProjectionMatrix.M[0][0] );

				// Add scene captures - if their streaming is not disabled (SceneCaptureStreamingMultiplier == 0.0f)
				if (View->Family->Scene && (GSystemSettings.SceneCaptureStreamingMultiplier > 0.0f))
				{
					View->Family->Scene->AddSceneCaptureViewInformation(GStreamingManager, View);
				}
			}
		}
	}

	// Update level streaming.
	GWorld->UpdateLevelStreaming( &ViewFamily );

	// Draw the player views.
	if (!bDisableWorldRendering && !bUIDisableWorldRendering && PlayerViewMap.Num() > 0)
	{
		BeginRenderingViewFamily(Canvas,&ViewFamily);
	}

	// Clear areas of the rendertarget (backbuffer) that aren't drawn over by the views.
	{
		// Find largest rectangle bounded by all rendered views.
		UINT MinX=Viewport->GetSizeX(),	MinY=Viewport->GetSizeY(), MaxX=0, MaxY=0;
		UINT TotalArea = 0;
		for( INT ViewIndex = 0; ViewIndex < ViewFamily.Views.Num(); ++ViewIndex )
		{
			const FSceneView* View = ViewFamily.Views(ViewIndex);
			INT UnscaledViewX = 0;
			INT UnscaledViewY = 0;
			UINT UnscaledViewSizeX = 0;
			UINT UnscaledViewSizeY = 0;

			//unscale the view coordinates before calculating extents
			GSystemSettings.UnScaleScreenCoords(
				UnscaledViewX, UnscaledViewY, 
				UnscaledViewSizeX, UnscaledViewSizeY, 
				View->X, View->Y, 
				View->SizeX, View->SizeY);

			MinX = Min<UINT>(UnscaledViewX, MinX);
			MinY = Min<UINT>(UnscaledViewY, MinY);
			MaxX = Max<UINT>(UnscaledViewX + UnscaledViewSizeX, MaxX);
			MaxY = Max<UINT>(UnscaledViewY + UnscaledViewSizeY, MaxY);
			TotalArea += UnscaledViewSizeX * UnscaledViewSizeY;
		}
		// If the views don't cover the entire bounding rectangle, clear the entire buffer.
		if ( ViewFamily.Views.Num() == 0 || TotalArea != (MaxX-MinX)*(MaxY-MinY) || bDisableWorldRendering )
		{
			DrawTile(Canvas,0,0,Viewport->GetSizeX(),Viewport->GetSizeY(),0.0f,0.0f,1.0f,1.f,FLinearColor::Black,NULL,FALSE);
		}
		else
		{
			// clear left
			if( MinX > 0 )
			{
				DrawTile(Canvas,0,0,MinX,Viewport->GetSizeY(),0.0f,0.0f,1.0f,1.f,FLinearColor::Black,NULL,FALSE);
			}
			// clear right
			if( MaxX < Viewport->GetSizeX() )
			{
				DrawTile(Canvas,MaxX,0,Viewport->GetSizeX(),Viewport->GetSizeY(),0.0f,0.0f,1.0f,1.f,FLinearColor::Black,NULL,FALSE);
			}
			// clear top
			if( MinY > 0 )
			{
				DrawTile(Canvas,MinX,0,MaxX,MinY,0.0f,0.0f,1.0f,1.f,FLinearColor::Black,NULL,FALSE);
			}
			// clear bottom
			if( MaxY < Viewport->GetSizeY() )
			{
				DrawTile(Canvas,MinX,MaxY,MaxX,Viewport->GetSizeY(),0.0f,0.0f,1.0f,1.f,FLinearColor::Black,NULL,FALSE);
			}
		}
	}

	// Remove temporary debug lines.
	if (GWorld->LineBatcher != NULL && GWorld->LineBatcher->BatchedLines.Num())
	{
		GWorld->LineBatcher->BatchedLines.Empty();
		GWorld->LineBatcher->BeginDeferredReattach();
	}

	// End dynamic shadow stats gathering now that snapshot is collected.
	if( GWorld->bGatherDynamicShadowStatsSnapshot )
	{
		GWorld->bGatherDynamicShadowStatsSnapshot = FALSE;
		// Update the dynamic shadow stats browser.
		if( GCallbackEvent )
		{
			GCallbackEvent->Send( CALLBACK_RefreshEditor_DynamicShadowStatsBrowser );
		}
		// Dump stats to the log outside the editor.
		if( !GIsEditor )
		{
			for( INT RowIndex=0; RowIndex<GWorld->DynamicShadowStats.Num(); RowIndex++ )
			{
				const FDynamicShadowStatsRow& Row = GWorld->DynamicShadowStats(RowIndex);
				FString RowStringified;
				for( INT ColumnIndex=0; ColumnIndex<Row.Columns.Num(); ColumnIndex++ )
				{
					RowStringified += FString::Printf(TEXT("%s,"),*Row.Columns(ColumnIndex));
				}
				debugf(TEXT("%s"),*RowStringified);
			}
		}
	}

	// Render the UI if enabled.
	if( GTickAndRenderUI )
	{
		SCOPE_CYCLE_COUNTER(STAT_UIDrawingTime);

		// render the canvas scene if needed
		if( bUISceneHasPrimitives )
		{
			// Use the same show flags for the canvas scene as the main scene
			QWORD CanvasSceneShowFlags = ShowFlags;

			// Create a FSceneViewFamilyContext for the canvas scene
			FSceneViewFamilyContext CanvasSceneViewFamily(
				Viewport,
				CanvasScene,
				CanvasSceneShowFlags,
				GWorld->GetTimeSeconds(),
				GWorld->GetDeltaSeconds(),
				GWorld->GetRealTimeSeconds(), 
				NULL, 
				TRUE,
				FALSE,
				FALSE,			
				FALSE,					// maintain scene color from player scene rendering
				!bUISceneHasPostProcess	// disable the final copy to the viewport RT if still rendering to scene color
				);

			// Generate the view for the canvas scene
			const FVector2D ViewOffsetScale(0,0);
			const FVector2D ViewSizeScale(1,1);
			CanvasScene->CalcSceneView(
				&CanvasSceneViewFamily,
				ViewOffsetScale,
				ViewSizeScale,
				Viewport,
				NULL,
				NULL
				);

			// Render the canvas scene
			BeginRenderingViewFamily(Canvas,&CanvasSceneViewFamily);		
		}

		// create a proxy of the scene color buffer to render HUD/UI to 
		// so that they can be post processed
		static FSceneRenderTargetProxy SceneColorTarget;
		if( bUISceneHasPostProcess )
		{
			// the viewport size will always be smaller than the render target size
			SceneColorTarget.SetSizes(Viewport->GetSizeX(),Viewport->GetSizeY());
			// set the scene color render target
			Canvas->SetRenderTarget(&SceneColorTarget);
		}

		// render HUD
		for(INT PlayerIndex = 0;PlayerIndex < GEngine->GamePlayers.Num();PlayerIndex++)
		{
			ULocalPlayer* Player = GEngine->GamePlayers(PlayerIndex);
			if(Player->Actor)
			{
				FSceneView* View = PlayerViewMap.FindRef(PlayerIndex);
				if (View != NULL)
				{
					INT UnscaledViewX = 0;
					INT UnscaledViewY = 0;
					UINT UnscaledViewSizeX = 0;
					UINT UnscaledViewSizeY = 0;

					// Unscale the view coordinates if needed
					GSystemSettings.UnScaleScreenCoords(
						UnscaledViewX, UnscaledViewY, 
						UnscaledViewSizeX, UnscaledViewSizeY, 
						View->X, View->Y, 
						View->SizeX, View->SizeY);

					// Set the canvas transform for the player's view rectangle.
					CanvasObject->Init();
					CanvasObject->SizeX = UnscaledViewSizeX;
					CanvasObject->SizeY = UnscaledViewSizeY;
					CanvasObject->SceneView = View;
					CanvasObject->Update();
					Canvas->PushAbsoluteTransform(FTranslationMatrix(FVector(UnscaledViewX,UnscaledViewY,0)));

					// Render the player's HUD.
					if( Player->Actor->myHUD )
					{
						SCOPE_CYCLE_COUNTER(STAT_HudTime);			

						Player->Actor->myHUD->Canvas = CanvasObject;
						Player->Actor->myHUD->eventPostRender();
						Player->Actor->myHUD->Canvas = NULL;
					}

					Canvas->PopTransform();

					// draw subtitles
					if (PlayerIndex == 0)
					{
						FVector2D MinPos(0.f, 0.f);
						FVector2D MaxPos(1.f, 1.f);
						eventGetSubtitleRegion(MinPos, MaxPos);

						UINT SizeX = Canvas->GetRenderTarget()->GetSizeX();
						UINT SizeY = Canvas->GetRenderTarget()->GetSizeY();
						FIntRect SubtitleRegion(appTrunc(SizeX * MinPos.X), appTrunc(SizeY * MinPos.Y), appTrunc(SizeX * MaxPos.X), appTrunc(SizeY * MaxPos.Y));
						FSubtitleManager::GetSubtitleManager()->DisplaySubtitles( Canvas, SubtitleRegion );
					}
				}
			}
		}

		// Reset the canvas for rendering to the full viewport.
		CanvasObject->Init();
		CanvasObject->SizeX = Viewport->GetSizeX();
		CanvasObject->SizeY = Viewport->GetSizeY();
		CanvasObject->SceneView = NULL;
		CanvasObject->Update();		
		
		// render the global UIScenes 
		UIController->RenderUI(Canvas);

		if( bUISceneHasPostProcess )
		{
			// Compute the view's screen rectangle.
			INT X = 0;
			INT Y = 0;
			UINT SizeX = Viewport->GetSizeX();
			UINT SizeY = Viewport->GetSizeY();

			// Take screen percentage option into account if percentage != 100.
			GSystemSettings.ScaleScreenCoords(X,Y,SizeX,SizeY);

			// restore viewport RT
			Canvas->SetRenderTarget(Viewport);

			// Create a FSceneViewFamilyContext for rendering the post process
			FSceneViewFamilyContext PostProcessViewFamily(
				Viewport,
				NULL,
				ShowFlags,
				GWorld->GetTimeSeconds(),
				GWorld->GetDeltaSeconds(),
				GWorld->GetRealTimeSeconds(), 
				NULL, 
				TRUE,
				TRUE,
				FALSE,			
				FALSE,	// maintain scene color from player scene rendering
				TRUE	// this is the final post process so enable the final copy to the viewport RT
				);

			const FLOAT fFOV = 90.0f;

			// add the new view to the scene
			FSceneView* View = new FSceneView(
				&PostProcessViewFamily,
				NULL,
				-1,
				NULL,
				NULL,
				NULL,
				UISceneClient->UIScenePostProcess,
				NULL,
				NULL,
				NULL,
				X,
				Y,
				SizeX,
				SizeY,
				FCanvas::CalcViewMatrix(SizeX,SizeY,fFOV),
				FCanvas::CalcProjectionMatrix(SizeX,SizeY,fFOV,NEAR_CLIPPING_PLANE),
				FLinearColor::Black,
				FLinearColor(0,0,0,0),
				FLinearColor::White,
				TSet<UPrimitiveComponent*>()
				);
			PostProcessViewFamily.Views.AddItem(View);
			// Render the post process pass. 
			// Always need to render this when any UI post processing was enabled 
			// even if the UIScenePostProcess=NULL because need to copy from 
			// scene color to the viewport RT
			BeginRenderingViewFamily(Canvas,&PostProcessViewFamily);
		}	

		// Allow the viewport to render additional stuff
		eventPostRender(CanvasObject);
	}

#if STATS
	DWORD DrawStatsBeginTime = appCycles();
#endif

	//@todo joeg: Move this stuff to a function, make safe to use on consoles by
	// respecting the various safe zones, and make it compile out.
#if CONSOLE
		const INT FPSXOffset	= 250;
		const INT StatsXOffset	= 100;
#else
		const INT FPSXOffset	= 90;
		const INT StatsXOffset	= 4;
#endif

#if !FINAL_RELEASE
	if( !GIsTiledScreenshot && !GIsDumpingMovie )
	{
		AWorldInfo* WorldInfo = GWorld->GetWorldInfo();

		if( GIsTextureMemoryCorrupted )
		{
			DrawShadowedString(Canvas,
				100,
				200,
				TEXT("RAN OUT OF TEXTURE MEMORY, EXPECT CORRUPTION AND GPU HANGS!"),
				GEngine->MediumFont,
				FColor(255,0,0)
				);
		}

		if( WorldInfo->bMapNeedsLightingFullyRebuilt )
		{
			FColor Color = FColor(128,128,128);
			// Color unbuilt lighting red if encountered within the last second
			if( GCurrentTime - WorldInfo->LastTimeUnbuiltLightingWasEncountered < 1 )
			{
				Color = FColor(255,0,0);
			}
			DrawShadowedString(Canvas,
				10,
				100,
				TEXT("LIGHTING NEEDS TO BE REBUILT"),
				GEngine->SmallFont,
				Color
				);
		}

		if( WorldInfo->bMapHasPathingErrors )
		{
			DrawShadowedString(Canvas,
				10,
				140,
				TEXT("MAP HAS PATHING ERRORS"),
				GEngine->SmallFont,
				FColor(128,128,128)
				);
		}

		if (GWorld->bIsLevelStreamingFrozen)
		{
			DrawShadowedString(Canvas,
				10,
				180,
				TEXT("Level streaming frozen..."),
				GEngine->SmallFont,
				FColor(128,128,128)
				);
		}

		if (GIsPrepareMapChangeBroken)
		{
			DrawShadowedString(Canvas,
				10,
				240,
				TEXT("PrepareMapChange had a bad level name! Check the log (tagged with PREPAREMAPCHANGE) for info!"),
				GEngine->SmallFont,
				FColor(128,128,128)
				);
		}

		INT XPos = 10;
		INT YPos = 220;

		if (WorldInfo->PriorityScreenMessages.Num() > 0)
		{
			for (INT PrioIndex = WorldInfo->PriorityScreenMessages.Num() - 1; PrioIndex >= 0; PrioIndex--)
			{
				FScreenMessageString& Message = WorldInfo->PriorityScreenMessages(PrioIndex);
				if (YPos < 700)
				{
					DrawShadowedString(Canvas,
						XPos,
						YPos,
						*(Message.ScreenMessage),
						GEngine->SmallFont,
						Message.DisplayColor
						);
					YPos += 20;
				}
				Message.CurrentTimeDisplayed += GWorld->GetDeltaSeconds();
				if (Message.CurrentTimeDisplayed >= Message.TimeToDisplay)
				{
					WorldInfo->PriorityScreenMessages.Remove(PrioIndex);
				}
			}
		}

		if (WorldInfo->ScreenMessages.Num() > 0)
		{
			for (TMap<INT, FScreenMessageString>::TIterator MsgIt(WorldInfo->ScreenMessages); MsgIt; ++MsgIt)
			{
				FScreenMessageString& Message = MsgIt.Value();
				if (YPos < 700)
				{
					DrawShadowedString(Canvas,
						XPos,
						YPos,
						*(Message.ScreenMessage),
						GEngine->SmallFont,
						Message.DisplayColor
						);
					YPos += 20;
				}
				Message.CurrentTimeDisplayed += GWorld->GetDeltaSeconds();
				if (Message.CurrentTimeDisplayed >= Message.TimeToDisplay)
				{
					MsgIt.RemoveCurrent();
				}
			}
		}
	}
#endif

	{
		INT X = Viewport->GetSizeX() - FPSXOffset;
		INT	Y = appTrunc(Viewport->GetSizeY() * 0.20f);

		// Render the FPS counter.
		Y = DrawFPSCounter( Viewport, Canvas, X, Y );
		// Render CPU thread and GPU frame times.
		Y = DrawUnitTimes( Viewport, Canvas, X, Y );

		// Reset Y as above stats are rendered on right hand side.
		Y = 20;

		// Level stats.
		Y = DrawLevelStats( Viewport, Canvas, StatsXOffset, Y );

#if !FINAL_RELEASE
		// Active sound waves
		Y = DrawSoundWaves( Viewport, Canvas, StatsXOffset, Y );
#endif

#if STATS
		// Render the Stats System UI
		GStatManager.Render( Canvas, StatsXOffset, Y );
#endif
	}

	// Render the stat chart.
	if(GStatChart)
	{
		GStatChart->Render(Viewport, Canvas);
	}

	// draw debug properties
#if !FINAL_RELEASE
#if SHIPPING_PC_GAME
	if (GWorld != NULL && GWorld->GetNetMode() == NM_Standalone)
#endif
	{
		// construct a list of objects relevant to "getall" type elements, so that we only have to do the object iterator once
		// we do the iterator each frame so that new objects will show up immediately
		TArray<UClass*> DebugClasses;
		DebugClasses.Reserve(DebugProperties.Num());
		for (INT i = 0; i < DebugProperties.Num(); i++)
		{
			if (DebugProperties(i).Obj != NULL && !DebugProperties(i).Obj->IsPendingKill())
			{
				UClass* Cls = Cast<UClass>(DebugProperties(i).Obj);
				if (Cls != NULL)
				{
					DebugClasses.AddItem(Cls);
				}
			}
			else
			{
				// invalid, object was destroyed, etc. so remove the entry
				DebugProperties.Remove(i--, 1);
			}
		}
		TArray<UObject*> RelevantObjects;
		if (DebugClasses.Num() > 0)
		{
			for (TObjectIterator<UObject> It(TRUE); It; ++It)
			{
				for (INT i = 0; i < DebugClasses.Num(); i++)
				{
					if (It->IsA(DebugClasses(i)))
					{
						RelevantObjects.AddItem(*It);
						break;
					}
				}
			}
		}
		// draw starting in the top left
		INT X = StatsXOffset;
		INT Y = 20;
		INT MaxY = INT(Canvas->GetRenderTarget()->GetSizeY());
		for (INT i = 0; i < DebugProperties.Num() && Y < MaxY; i++)
		{
			// we removed entries with invalid Obj above so no need to check for that here
			UClass* Cls = Cast<UClass>(DebugProperties(i).Obj);
			if (Cls != NULL)
			{
				UProperty* Prop = FindField<UProperty>(Cls, DebugProperties(i).PropertyName);
				if (Prop != NULL || DebugProperties(i).bSpecialProperty)
				{
					// getall
					for (INT j = 0; j < RelevantObjects.Num(); j++)
					{
						if (RelevantObjects(j)->IsA(Cls) && !RelevantObjects(j)->IsPendingKill())
						{
							DrawProperty(CanvasObject, RelevantObjects(j), DebugProperties(i), Prop, X, Y);
						}
					}
				}
				else
				{
					// invalid entry
					DebugProperties.Remove(i--, 1);
				}
			}
			else
			{
				UProperty* Prop = FindField<UProperty>(DebugProperties(i).Obj->GetClass(), DebugProperties(i).PropertyName);
				if (Prop != NULL || DebugProperties(i).bSpecialProperty)
				{
					DrawProperty(CanvasObject, DebugProperties(i).Obj, DebugProperties(i), Prop, X, Y);
				}
				else
				{
					DebugProperties.Remove(i--, 1);
				}
			}
		}
	}
#endif

	if( GIsDumpingMovie || GScreenShotRequest )
	{
		Canvas->Flush();

		// Read the contents of the viewport into an array.
		TArray<FColor>	Bitmap;
		if(Viewport->ReadPixels(Bitmap))
		{
			check(Bitmap.Num() == Viewport->GetSizeX() * Viewport->GetSizeY());

			// Create screenshot folder if not already present.
			GFileManager->MakeDirectory( *GSys->ScreenShotPath, TRUE );

			const FString ScreenFileName( GSys->ScreenShotPath * (GIsDumpingMovie ? TEXT("MovieFrame") : GScreenShotName == TEXT( "") ? TEXT("ScreenShot") : GScreenShotName ));

			// Save the contents of the array to a bitmap file.
			appCreateBitmap(*ScreenFileName,Viewport->GetSizeX(),Viewport->GetSizeY(),&Bitmap(0),GFileManager);			
		}
		// reset any G vars that deal with screenshots.  We need to expressly reset the GScreenShotName here as there is code that will just keep
		// writing to the same .bmp if that GScreenShotName has an .bmp extension
		GScreenShotRequest=FALSE;
		GScreenShotName = TEXT("");
	}

#if PS3
	extern void PrintFrameRatePS3();
	PrintFrameRatePS3();

#if !USE_NULL_RHI
	if( GShowPS3GcmMemoryStats )
	{
		const FLOAT BorderSize = 0.05f; //pct
		FLOAT X = Viewport->GetSizeX()*BorderSize;
		FLOAT Y = Viewport->GetSizeY()*BorderSize;
		FLOAT SizeX = Viewport->GetSizeX() - Viewport->GetSizeX()*BorderSize*2;
		FLOAT SizeY = GEngine ? GEngine->TinyFont->GetMaxCharHeight()*2 : 80.f;
		// draw graph of all current local allocs and free regions
		GPS3Gcm->GetLocalAllocator()->DepugDraw(Canvas,X,Y,SizeX,SizeY, TEXT("Local"));
		// draw graph of all current host allocs and free regions
		Y += SizeY*2;
		for( INT HostMemIdx=0; HostMemIdx < HostMem_MAX; HostMemIdx++ )
		{
			const EHostMemoryHeapType HostMemType = (EHostMemoryHeapType)HostMemIdx; 
			if( GPS3Gcm->HasHostAllocator(HostMemType) )
			{
				GPS3Gcm->GetHostAllocator(HostMemType)->DepugDraw(Canvas,X,Y,SizeX,SizeY, GPS3Gcm->GetHostAllocatorDesc(HostMemType), HostMemIdx==(HostMem_MAX-1));
			}
		}
		Canvas->Flush();
	}
#endif	//#if !USE_NULL_RHI
#endif

#if STATS
	DWORD DrawStatsEndTime = appCycles();
	SET_CYCLE_COUNTER(STAT_DrawStats, DrawStatsEndTime - DrawStatsBeginTime, 1);
#endif
}

void UGameViewportClient::Precache()
{
	if(!GIsEditor)
	{
		// Precache sounds...
		UAudioDevice* AudioDevice = GEngine->Client ? GEngine->Client->GetAudioDevice() : NULL;
		if( AudioDevice )
		{
			debugf(TEXT("Precaching sounds..."));
			for(TObjectIterator<USoundNodeWave> It;It;++It)
			{
				USoundNodeWave* SoundNodeWave = *It;
				AudioDevice->Precache( SoundNodeWave );
			}
			debugf(TEXT("Precaching sounds completed..."));
		}
	}

	// Log time till first precache is finished.
	static UBOOL bIsFirstCallOfFunction = TRUE;
	if( bIsFirstCallOfFunction )
	{
		debugf(TEXT("%5.2f seconds passed since startup."),appSeconds()-GStartTime);
		bIsFirstCallOfFunction = FALSE;
	}
}

void UGameViewportClient::LostFocus(FViewport* Viewport)
{
}

void UGameViewportClient::ReceivedFocus(FViewport* Viewport)
{
}

UBOOL UGameViewportClient::IsFocused(FViewport* Viewport)
{
	return Viewport->HasFocus() || Viewport->HasMouseCapture();
}

void UGameViewportClient::CloseRequested(FViewport* Viewport)
{
	check(Viewport == this->Viewport);
	if( GFullScreenMovie )
	{
		// force movie playback to stop
		GFullScreenMovie->GameThreadStopMovie(0,FALSE,TRUE);
	}
	GEngine->Client->CloseViewport(this->Viewport);
	SetViewportFrame(NULL);
}

UBOOL UGameViewportClient::Exec(const TCHAR* Cmd,FOutputDevice& Ar)
{
	if( ParseCommand(&Cmd,TEXT("SHOW")) )
	{
#if SHIPPING_PC_GAME || FINAL_RELEASE
		// don't allow show flags in net games
		if ( GWorld->GetNetMode() != NM_Standalone || (Cast<UGameEngine>(GEngine) && Cast<UGameEngine>(GEngine)->GPendingLevel != NULL) )
		{
			return TRUE;
		}
		// the effects of this cannot be easily reversed, so prevent the user from playing network games without restarting to avoid potential exploits
		GDisallowNetworkTravel = TRUE;
#endif

		struct { const TCHAR* Name; EShowFlags Flag; }	Flags[] =
		{
			{ TEXT("BOUNDS"),				SHOW_Bounds					},
			{ TEXT("BSP"),					SHOW_BSP					},
			{ TEXT("BSPSPLIT"),				SHOW_BSPSplit				},
			{ TEXT("CAMFRUSTUMS"),			SHOW_CamFrustums			},
			{ TEXT("COLLISION"),			SHOW_Collision				},
			{ TEXT("CONSTRAINTS"),			SHOW_Constraints			},
			{ TEXT("COVER"),				SHOW_Cover					},
			{ TEXT("DECALINFO"),			SHOW_DecalInfo				},
			{ TEXT("DECALS"),				SHOW_Decals					},
			{ TEXT("DYNAMICSHADOWS"),		SHOW_DynamicShadows			},
			{ TEXT("FOG"),					SHOW_Fog					},
			{ TEXT("FOLIAGE"),				SHOW_Foliage				},
			{ TEXT("HITPROXIES"),			SHOW_HitProxies				},
			{ TEXT("LENSFLARES"),			SHOW_LensFlares				},
			{ TEXT("LEVELCOLORATION"),		SHOW_LevelColoration		},
			{ TEXT("MESHEDGES"),			SHOW_MeshEdges				},
			{ TEXT("MISSINGCOLLISION"),		SHOW_MissingCollisionModel	},
			{ TEXT("NAVNODES"),				SHOW_NavigationNodes		},
			{ TEXT("NONZEROEXTENT"),		SHOW_CollisionNonZeroExtent	},
			{ TEXT("PARTICLES"),			SHOW_Particles				},
			{ TEXT("PATHS"),				SHOW_Paths					},
			{ TEXT("PORTALS"),				SHOW_Portals				},
			{ TEXT("POSTPROCESS"),			SHOW_PostProcess			},
			{ TEXT("RIGIDBODY"),			SHOW_CollisionRigidBody		},
			{ TEXT("SCENECAPTURE"),			SHOW_SceneCaptureUpdates	},
			{ TEXT("SHADOWFRUSTUMS"),		SHOW_ShadowFrustums			},
			{ TEXT("SKELETALMESHES"),		SHOW_SkeletalMeshes			},
			{ TEXT("SKELMESHES"),			SHOW_SkeletalMeshes			},
			{ TEXT("SPEEDTREES"),			SHOW_SpeedTrees				},
			{ TEXT("SPRITES"),				SHOW_Sprites				},
			{ TEXT("STATICMESHES"),			SHOW_StaticMeshes			},
			{ TEXT("TERRAIN"),				SHOW_Terrain				},
			{ TEXT("TERRAINCOLLISION"),		SHOW_TerrainCollision		},
			{ TEXT("TERRAINPATCHES"),		SHOW_TerrainPatches			},
			{ TEXT("UNLITTRANSLUCENCY"),	SHOW_UnlitTranslucency		},
			{ TEXT("ZEROEXTENT"),			SHOW_CollisionZeroExtent	},
			{ TEXT("VOLUMES"),				SHOW_Volumes				},
		};

		// First, look for skeletal mesh show commands

		UBOOL bUpdateSkelMeshCompDebugFlags = FALSE;
		static UBOOL bShowSkelBones = FALSE;
		static UBOOL bShowPrePhysSkelBones = FALSE;

		if(ParseCommand(&Cmd,TEXT("BONES")))
		{
			bShowSkelBones = !bShowSkelBones;
			bUpdateSkelMeshCompDebugFlags = TRUE;
		}
		else if(ParseCommand(&Cmd,TEXT("PREPHYSBONES")))
		{
			bShowPrePhysSkelBones = !bShowPrePhysSkelBones;
			bUpdateSkelMeshCompDebugFlags = TRUE;
		}

		// If we changed one of the skel mesh debug show flags, set it on each of the components in the GWorld.
		if(bUpdateSkelMeshCompDebugFlags)
		{
			for (TObjectIterator<USkeletalMeshComponent> It; It; ++It)
			{
				USkeletalMeshComponent* SkelComp = *It;
				if( SkelComp->GetScene() == GWorld->Scene )
				{
					SkelComp->bDisplayBones = bShowSkelBones;
					SkelComp->bShowPrePhysBones = bShowPrePhysSkelBones;
					SkelComp->BeginDeferredReattach();
				}
			}

			// Now we are done.
			return TRUE;
		}

		// Search for a specific show flag and toggle it if found.
		for(UINT FlagIndex = 0;FlagIndex < ARRAY_COUNT(Flags);FlagIndex++)
		{
			if(ParseCommand(&Cmd,Flags[FlagIndex].Name))
			{
				// Don't let the user toggle editoronly showflags.
				const UBOOL bCanBeToggled = GIsEditor || !(Flags[FlagIndex].Flag & SHOW_EditorOnly_Mask);
				if ( !bCanBeToggled )
				{
					continue;
				}

				ShowFlags ^= Flags[FlagIndex].Flag;
				// special case: for the SHOW_Collision flag, we need to un-hide any primitive components that collide so their collision geometry gets rendered
				if (Flags[FlagIndex].Flag == SHOW_Collision ||
					Flags[FlagIndex].Flag == SHOW_CollisionNonZeroExtent || 
					Flags[FlagIndex].Flag == SHOW_CollisionZeroExtent || 
					Flags[FlagIndex].Flag == SHOW_CollisionRigidBody )
				{
					// Ensure that all flags, other than the one we just typed in, is off.
					ShowFlags &= ~(Flags[FlagIndex].Flag ^ (SHOW_Collision_Any | SHOW_Collision));

					for (TObjectIterator<UPrimitiveComponent> It; It; ++It)
					{
						UPrimitiveComponent* PrimitiveComponent = *It;
						if( PrimitiveComponent->HiddenGame && PrimitiveComponent->ShouldCollide() && PrimitiveComponent->GetScene() == GWorld->Scene )
						{
							check( !GIsEditor || (PrimitiveComponent->GetOutermost()->PackageFlags & PKG_PlayInEditor) );
							PrimitiveComponent->SetHiddenGame(false);
						}
					}
				}
				else
				if (Flags[FlagIndex].Flag == SHOW_Paths || (GIsGame && Flags[FlagIndex].Flag == SHOW_Cover))
				{
					UBOOL bShowPaths = (ShowFlags & SHOW_Paths) != 0;
					UBOOL bShowCover = (ShowFlags & SHOW_Cover) != 0;
					// make sure all nav points have path rendering components
					for (FActorIterator It; It; ++It)
					{
						ACoverLink *Link = Cast<ACoverLink>(*It);
						if (Link != NULL)
						{
							UBOOL bHasComponent = FALSE;
							for (INT Idx = 0; Idx < Link->Components.Num(); Idx++)
							{
								UCoverMeshComponent *PathRenderer = Cast<UCoverMeshComponent>(Link->Components(Idx));
								if (PathRenderer != NULL)
								{
									PathRenderer->SetHiddenGame(!(bShowPaths || bShowCover));
									bHasComponent = TRUE;
									break;
								}
							}
							if (!bHasComponent)
							{
								UClass *MeshCompClass = FindObject<UClass>(ANY_PACKAGE,*GEngine->DynamicCoverMeshComponentName);
								if (MeshCompClass == NULL)
								{
									MeshCompClass = UCoverMeshComponent::StaticClass();
								}
								UCoverMeshComponent *PathRenderer = ConstructObject<UCoverMeshComponent>(MeshCompClass,Link);
								PathRenderer->SetHiddenGame(!(bShowPaths || bShowCover));
								Link->AttachComponent(PathRenderer);
							}
						}
						else
						{
							ANavigationPoint *Nav = Cast<ANavigationPoint>(*It);
							if (Nav != NULL)
							{
								UBOOL bHasComponent = FALSE;
								for (INT Idx = 0; Idx < Nav->Components.Num(); Idx++)
								{
									UPathRenderingComponent *PathRenderer = Cast<UPathRenderingComponent>(Nav->Components(Idx));
									if (PathRenderer != NULL)
									{
										bHasComponent = TRUE;
										PathRenderer->SetHiddenGame(!bShowPaths);
										break;
									}
								}
								if (!bHasComponent)
								{
									UPathRenderingComponent *PathRenderer = ConstructObject<UPathRenderingComponent>(UPathRenderingComponent::StaticClass(),Nav);
									PathRenderer->SetHiddenGame(!bShowPaths);
									Nav->AttachComponent(PathRenderer);
								}
							}
						}
					}
				}
				else if( Flags[FlagIndex].Flag == SHOW_Volumes )
				{
					// Iterate over all brushes
					for( TObjectIterator<UBrushComponent> It; It; ++It )
					{
						UBrushComponent* BrushComponent = *It;
						AVolume* Owner = Cast<AVolume>( BrushComponent->GetOwner() );

						// Only bother with volume brushes that belong to the world's scene
						if( Owner && BrushComponent->GetScene() == GWorld->Scene )
						{
							// We're expecting this to be in the game at this point
							check( !GIsEditor || ( BrushComponent->GetOutermost()->PackageFlags & PKG_PlayInEditor ) );

							// Toggle visibility of this volume
							if( BrushComponent->HiddenGame )
							{
								Owner->bHidden = false;
								BrushComponent->SetHiddenGame( false );
							}
							else
							{
								Owner->bHidden = true;
								BrushComponent->SetHiddenGame( true );
							}
						}
					}
				}

				return TRUE;
			}
		}

		// The specified flag wasn't found -- list all flags and their current value.
		for(UINT FlagIndex = 0;FlagIndex < ARRAY_COUNT(Flags);FlagIndex++)
		{
			Ar.Logf(TEXT("%s : %s"),
				Flags[FlagIndex].Name,
				(ShowFlags & Flags[FlagIndex].Flag) ? TEXT("TRUE") :TEXT("FALSE"));
		}
		return TRUE;
	}
	else if (ParseCommand(&Cmd,TEXT("VIEWMODE")))
	{
#ifndef _DEBUG
		// If there isn't a cheat manager, exit out
		UBOOL bCheatsEnabled = FALSE;
		for (FPlayerIterator It((UEngine*)GetOuter()); It; ++It)
		{
			if (It->Actor != NULL && It->Actor->CheatManager != NULL)
			{
				bCheatsEnabled = TRUE;
				break;
			}
		}
		if (!bCheatsEnabled)
		{
			return TRUE;
		}
#endif

		if( ParseCommand(&Cmd,TEXT("WIREFRAME")) )
		{
			ShowFlags &= ~SHOW_ViewMode_Mask;
			ShowFlags |= SHOW_ViewMode_Wireframe;
		}
		else if( ParseCommand(&Cmd,TEXT("BRUSHWIREFRAME")) )
		{
			ShowFlags &= ~SHOW_ViewMode_Mask;
			ShowFlags |= SHOW_ViewMode_BrushWireframe;
		}
		else if( ParseCommand(&Cmd,TEXT("UNLIT")) )
		{
			ShowFlags &= ~SHOW_ViewMode_Mask;
			ShowFlags |= SHOW_ViewMode_Unlit;
		}
		else if( ParseCommand(&Cmd,TEXT("LIGHTINGONLY")) )
		{
			ShowFlags &= ~SHOW_ViewMode_Mask;
			ShowFlags |= SHOW_ViewMode_LightingOnly;
		}			
		else if( ParseCommand(&Cmd,TEXT("LIGHTCOMPLEXITY")) )
		{
			ShowFlags &= ~SHOW_ViewMode_Mask;
			ShowFlags |= SHOW_ViewMode_LightComplexity;
		}
		else if( ParseCommand(&Cmd,TEXT("SHADERCOMPLEXITY")) )
		{
			ShowFlags &= ~SHOW_ViewMode_Mask;
			ShowFlags |= SHOW_ViewMode_ShaderComplexity;
		}
		else if( ParseCommand(&Cmd,TEXT("TEXTUREDENSITY")))
		{
			ShowFlags &= ~SHOW_ViewMode_Mask;
			ShowFlags |= SHOW_ViewMode_TextureDensity;
		}
		else
		{
			ShowFlags &= ~SHOW_ViewMode_Mask;
			ShowFlags |= SHOW_ViewMode_Lit;
		}

		return TRUE;
	}
	else if (ParseCommand(&Cmd, TEXT("NEXTVIEWMODE")))
	{
#ifndef _DEBUG
		// If there isn't a cheat manager, exit out
		UBOOL bCheatsEnabled = FALSE;
		for (FPlayerIterator It((UEngine*)GetOuter()); It; ++It)
		{
			if (It->Actor != NULL && It->Actor->CheatManager != NULL)
			{
				bCheatsEnabled = TRUE;
				break;
			}
		}
		if (!bCheatsEnabled)
		{
			return TRUE;
		}
#endif

		QWORD OldShowFlags = ShowFlags;
		ShowFlags &= ~SHOW_ViewMode_Mask;
		switch (OldShowFlags & SHOW_ViewMode_Mask)
		{
			case SHOW_ViewMode_Lit:
				ShowFlags |= SHOW_ViewMode_LightingOnly;
				break;
			case SHOW_ViewMode_LightingOnly:
				ShowFlags |= SHOW_ViewMode_LightComplexity;
				break;
			case SHOW_ViewMode_LightComplexity:
				ShowFlags |= SHOW_ViewMode_Wireframe;
				break;
			case SHOW_ViewMode_Wireframe:
				ShowFlags |= SHOW_ViewMode_BrushWireframe;
				break;
			case SHOW_ViewMode_BrushWireframe:
				ShowFlags |= SHOW_ViewMode_Unlit;
				break;
			case SHOW_ViewMode_Unlit:
				ShowFlags |= SHOW_ViewMode_TextureDensity;
				break;
			case SHOW_ViewMode_TextureDensity:
				ShowFlags |= SHOW_ViewMode_Lit;
				break;
			default:
				ShowFlags |= SHOW_ViewMode_Lit;
				break;
		}

		return TRUE;
	}
	else if (ParseCommand(&Cmd, TEXT("PREVVIEWMODE")))
	{
#ifndef _DEBUG
		// If there isn't a cheat manager, exit out
		UBOOL bCheatsEnabled = FALSE;
		for (FPlayerIterator It((UEngine*)GetOuter()); It; ++It)
		{
			if (It->Actor != NULL && It->Actor->CheatManager != NULL)
			{
				bCheatsEnabled = TRUE;
				break;
			}
		}
		if (!bCheatsEnabled)
		{
			return TRUE;
		}
#endif

		QWORD OldShowFlags = ShowFlags;
		ShowFlags &= ~SHOW_ViewMode_Mask;
		switch (OldShowFlags & SHOW_ViewMode_Mask)
		{
			case SHOW_ViewMode_Lit:
				ShowFlags |= SHOW_ViewMode_TextureDensity;
				break;
			case SHOW_ViewMode_LightingOnly:
				ShowFlags |= SHOW_ViewMode_Lit;
				break;
			case SHOW_ViewMode_LightComplexity:
				ShowFlags |= SHOW_ViewMode_LightingOnly;
				break;
			case SHOW_ViewMode_Wireframe:
				ShowFlags |= SHOW_ViewMode_LightComplexity;
				break;
			case SHOW_ViewMode_BrushWireframe:
				ShowFlags |= SHOW_ViewMode_Wireframe;
				break;
			case SHOW_ViewMode_Unlit:
				ShowFlags |= SHOW_ViewMode_BrushWireframe;
				break;
			case SHOW_ViewMode_TextureDensity:
				ShowFlags |= SHOW_ViewMode_Unlit;
				break;
			default:
				ShowFlags |= SHOW_ViewMode_Lit;
				break;
		}

		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("STAT")) )
	{
		const TCHAR* Temp = Cmd;
		// Check for FPS counter
		if (ParseCommand(&Temp,TEXT("FPS")))
		{
			GShowFpsCounter ^= TRUE;
			return TRUE;
		}
		// Check for Level stats.
		else if (ParseCommand(&Temp,TEXT("LEVELS")))
		{
			GShowLevelStats ^= TRUE;
			return TRUE;
		}
		// Check for idle times.
		else if (ParseCommand(&Temp,TEXT("UNIT")))
		{
			GShowUnitTimes ^= TRUE;
			return TRUE;
		}
		else if(ParseCommand(&Temp,TEXT("SOUNDWAVES")))
		{
			GShowSoundWaves ^= TRUE;
			return TRUE;
		}
#if PS3
		else if( ParseCommand(&Temp,TEXT("GPUMEMALLOCS")) )
		{
			GShowPS3GcmMemoryStats = !GShowPS3GcmMemoryStats;
			return TRUE;
		}		
#endif
#if STATS
		// Forward the call to the stat manager
		else
		{
			return GStatManager.Exec(Cmd,Ar);
		}
#else
		return FALSE;
#endif
	}
	if( ParseCommand(&Cmd,TEXT("DUMPDYNAMICSHADOWSTATS")) )
	{
		GWorld->bGatherDynamicShadowStatsSnapshot = TRUE;
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("PRECACHE")) )
	{
		Precache();
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("SETRES")) )
	{
		if(Viewport && ViewportFrame)
		{
			INT X=appAtoi(Cmd);
			const TCHAR* CmdTemp = appStrchr(Cmd,'x') ? appStrchr(Cmd,'x')+1 : appStrchr(Cmd,'X') ? appStrchr(Cmd,'X')+1 : TEXT("");
			INT Y=appAtoi(CmdTemp);
			Cmd = CmdTemp;
			UBOOL	Fullscreen = Viewport->IsFullscreen();
			if(appStrchr(Cmd,'w') || appStrchr(Cmd,'W'))
				Fullscreen = 0;
			else if(appStrchr(Cmd,'f') || appStrchr(Cmd,'F'))
				Fullscreen = 1;
			if( X && Y )
				ViewportFrame->Resize(X,Y,Fullscreen);
		}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("TILEDSHOT")) )
	{
		FlushRenderingCommands();
		GIsTiledScreenshot = TRUE;
		GScreenshotResolutionMultiplier = appAtoi(Cmd);
		GScreenshotResolutionMultiplier = Clamp<INT>( GScreenshotResolutionMultiplier, 2, 128 );
		const TCHAR* CmdTemp = appStrchr(Cmd, ' ');
		GScreenshotMargin = CmdTemp ? Clamp<INT>(appAtoi(CmdTemp), 0, 320) : 64;
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("SHOT")) || ParseCommand(&Cmd,TEXT("SCREENSHOT")) )
	{
		if(Viewport)
		{
			GScreenShotRequest=TRUE;
		}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("LOGOUTSTATLEVELS")) )
	{
		TMap<FName,INT> StreamingLevels;	
		FString LevelPlayerIsInName;
		GetLevelStremingStatus( StreamingLevels, LevelPlayerIsInName );

		Ar.Logf( TEXT( "Level Streaming:" ) );

		// now draw the "map" name
		FString MapName	= GWorld->CurrentLevel->GetOutermost()->GetName();

		if( LevelPlayerIsInName == MapName )
		{
			MapName = *FString::Printf( TEXT("->  %s"), *MapName );
		}
		else
		{
			MapName = *FString::Printf( TEXT("    %s"), *MapName );
		}

		Ar.Logf( TEXT( "%s" ), *MapName );

		// now log the levels
		for( TMap<FName,INT>::TIterator It(StreamingLevels); It; ++It )
		{
			FString LevelName = It.Key().ToString();
			const INT Status = It.Value();
			FString StatusName;

			switch( Status )
			{
			case LEVEL_Visible:
				StatusName = TEXT( "red loaded and visible" );
				break;
			case LEVEL_MakingVisible:
				StatusName = TEXT( "orange, in process of being made visible" );
				break;
			case LEVEL_Loaded:
				StatusName = TEXT( "yellow loaded but not visible" );
				break;
			case LEVEL_UnloadedButStillAround:
				StatusName = TEXT( "blue  (GC needs to occur to remove this)" );
				break;
			case LEVEL_Unloaded:
				StatusName = TEXT( "green Unloaded" );
				break;
			case LEVEL_Preloading:
				StatusName = TEXT( "purple (preloading)" );
				break;
			default:
				break;
			};


			UPackage* LevelPackage = FindObject<UPackage>( NULL, *LevelName );

			if( LevelPackage 
				&& (LevelPackage->GetLoadTime() > 0) 
				&& (Status != LEVEL_Unloaded) )
			{
				LevelName += FString::Printf(TEXT(" - %4.1f sec"), LevelPackage->GetLoadTime());
			}
			else if( UObject::GetAsyncLoadPercentage( *LevelName ) >= 0 )
			{
				const INT Percentage = appTrunc( UObject::GetAsyncLoadPercentage( *LevelName ) );
				LevelName += FString::Printf(TEXT(" - %3i %%"), Percentage ); 
			}

			if( LevelPlayerIsInName == LevelName )
			{
				LevelName = *FString::Printf( TEXT("->  %s"), *LevelName );
			}
			else
			{
				LevelName = *FString::Printf( TEXT("    %s"), *LevelName );
			}

			LevelName = FString::Printf( TEXT("%s \t\t%s"), *LevelName, *StatusName );

			Ar.Logf( TEXT( "%s" ), *LevelName );

		}

		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("BUGSCREENSHOT")) )
	{
		// find where these are really defined
#if PS3
		const static INT MaxFilenameLen = 42;
#elif XBOX
		const static INT MaxFilenameLen = 42;
#else
		const static INT MaxFilenameLen = 100;
#endif

		if( Viewport != NULL )
		{
			TCHAR File[MAX_SPRINTF] = TEXT("");
			for( INT TestBitmapIndex = 0; TestBitmapIndex < 9; ++TestBitmapIndex )
			{ 
				const FString DescPlusExtension = FString::Printf( TEXT("%s%i.bmp"), Cmd, TestBitmapIndex );
				const FString SSFilename = CreateProfileFilename( DescPlusExtension, FALSE );
				const FString OutputDir = appGameDir() + TEXT("ScreenShots") + PATH_SEPARATOR;
				//warnf( TEXT( "BugIt Looking: %s" ), *(OutputDir + SSFilename) );
				appSprintf( File, TEXT("%s"), *(OutputDir + SSFilename) );
				if( GFileManager->FileSize(File) == INDEX_NONE )
				{
					GSceenShotBitmapIndex = TestBitmapIndex; // this is safe as the UnMisc.cpp ScreenShot code will test each number before writing a file
					GScreenShotName = SSFilename; 
					GScreenShotRequest = TRUE;

					break;
				}
			}
		}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("KILLPARTICLES")) )
	{
		// Don't kill in the Editor to avoid potential content clobbering.
		if( !GIsEditor )
		{
			extern UBOOL GIsAllowingParticles;
			// Deactivate system and kill existing particles.
			for( TObjectIterator<UParticleSystemComponent> It; It; ++It )
			{
				UParticleSystemComponent* ParticleSystemComponent = *It;
				ParticleSystemComponent->DeactivateSystem();
				ParticleSystemComponent->KillParticlesForced();
			}
			// No longer initialize particles from here on out.
			GIsAllowingParticles = FALSE;
		}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("FORCESKELLOD")) )
	{
		INT ForceLod = 0;
		if(Parse(Cmd,TEXT("LOD="),ForceLod))
		{
			ForceLod++;
		}

		for (TObjectIterator<USkeletalMeshComponent> It; It; ++It)
		{
			USkeletalMeshComponent* SkelComp = *It;
			if( SkelComp->GetScene() == GWorld->Scene && !SkelComp->IsTemplate())
			{
				SkelComp->ForcedLodModel = ForceLod;
			}
		}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("TOGGLEOCCLUSION")) )
	{
		extern UBOOL GIgnoreAllOcclusionQueries;
		GIgnoreAllOcclusionQueries = !GIgnoreAllOcclusionQueries;
		debugf(TEXT("Occlusion queries are now %s"),GIgnoreAllOcclusionQueries ? TEXT("disabled") : TEXT("enabled")); 
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("TOGGLEUI")) )
	{
		GTickAndRenderUI = !GTickAndRenderUI;
		return TRUE;
	}
#if !FINAL_RELEASE
	else if (ParseCommand(&Cmd, TEXT("DISPLAY")))
	{
		TCHAR ObjectName[256];
		TCHAR PropStr[256];
		if ( ParseToken(Cmd, ObjectName, ARRAY_COUNT(ObjectName), TRUE) &&
			ParseToken(Cmd, PropStr, ARRAY_COUNT(PropStr), TRUE) )
		{
			UObject* Obj = FindObject<UObject>(ANY_PACKAGE, ObjectName);
			if (Obj != NULL)
			{
				FName PropertyName(PropStr, FNAME_Find);
				if (PropertyName != NAME_None && FindField<UProperty>(Obj->GetClass(), PropertyName) != NULL)
				{
					FDebugDisplayProperty& NewProp = DebugProperties(DebugProperties.AddZeroed());
					NewProp.Obj = Obj;
					NewProp.PropertyName = PropertyName;
				}
				else
				{
					Ar.Logf(TEXT("Property '%s' not found on object '%s'"), PropStr, *Obj->GetName());
				}
			}
			else
			{
				Ar.Logf(TEXT("Object not found"));
			}
		}

		return TRUE;
	}
	else if (ParseCommand(&Cmd, TEXT("DISPLAYALL")))
	{
		TCHAR ClassName[256];
		TCHAR PropStr[256];
		if ( ParseToken(Cmd, ClassName, ARRAY_COUNT(ClassName), TRUE) &&
			ParseToken(Cmd, PropStr, ARRAY_COUNT(PropStr), TRUE) )
		{
			UClass* Cls = FindObject<UClass>(ANY_PACKAGE, ClassName);
			if (Cls != NULL)
			{
				FName PropertyName(PropStr, FNAME_Find);
				if (PropertyName != NAME_None && FindField<UProperty>(Cls, PropertyName) != NULL)
				{
					// add all un-GCable things immediately as that list is static
					// so then we only have to iterate over dynamic things each frame
					for (TObjectIterator<UObject> It; It; ++It)
					{
						if (!It->HasAnyFlags(RF_DisregardForGC))
						{
							break;
						}
						else if (It->IsA(Cls))
						{
							FDebugDisplayProperty& NewProp = DebugProperties(DebugProperties.AddZeroed());
							NewProp.Obj = *It;
							NewProp.PropertyName = PropertyName;
						}
					}
					FDebugDisplayProperty& NewProp = DebugProperties(DebugProperties.AddZeroed());
					NewProp.Obj = Cls;
					NewProp.PropertyName = PropertyName;
				}
				else
				{
					Ar.Logf(TEXT("Property '%s' not found on object '%s'"), PropStr, *Cls->GetName());
				}
			}
			else
			{
				Ar.Logf(TEXT("Object not found"));
			}
		}

		return TRUE;
	}
	else if (ParseCommand(&Cmd, TEXT("DISPLAYALLSTATE")))
	{
		TCHAR ClassName[256];
		if (ParseToken(Cmd, ClassName, ARRAY_COUNT(ClassName), TRUE))
		{
			UClass* Cls = FindObject<UClass>(ANY_PACKAGE, ClassName);
			if (Cls != NULL)
			{
				// add all un-GCable things immediately as that list is static
				// so then we only have to iterate over dynamic things each frame
				for (TObjectIterator<UObject> It; It; ++It)
				{
					if (!It->HasAnyFlags(RF_DisregardForGC))
					{
						break;
					}
					else if (It->IsA(Cls))
					{
						FDebugDisplayProperty& NewProp = DebugProperties(DebugProperties.AddZeroed());
						NewProp.Obj = *It;
						NewProp.PropertyName = NAME_State;
						NewProp.bSpecialProperty = TRUE;
					}
				}
				FDebugDisplayProperty& NewProp = DebugProperties(DebugProperties.AddZeroed());
				NewProp.Obj = Cls;
				NewProp.PropertyName = NAME_State;
				NewProp.bSpecialProperty = TRUE;
			}
			else
			{
				Ar.Logf(TEXT("Object not found"));
			}
		}

		return TRUE;
	}
	else if (ParseCommand(&Cmd, TEXT("DISPLAYCLEAR")))
	{
		DebugProperties.Empty();

		return TRUE;
	}
#endif
	else if (ParseCommand(&Cmd, TEXT("TOGGLEFLUIDS")))
	{
		extern UBOOL GForceFluidDeactivation;
		GForceFluidDeactivation = !GForceFluidDeactivation;
		Ar.Logf(TEXT("Forcing deactivation of all fluids: %s"), GForceFluidDeactivation ? TEXT("ON") : TEXT("OFF"));
		return TRUE;
	}
	else if(ParseCommand(&Cmd, TEXT("TEXTUREDEFRAG")))
	{
		extern void appDefragmentTexturePool();
		appDefragmentTexturePool();
		return TRUE;
	}
	else if (ParseCommand(&Cmd, TEXT("TOGGLEMIPFADE")))
	{
		GEnableMipLevelFading = (GEnableMipLevelFading >= 0.0f) ? -1.0f : 1.0f;
		Ar.Logf(TEXT("Mip-fading is now: %s"), (GEnableMipLevelFading >= 0.0f) ? TEXT("ENABLED") : TEXT("DISABLED"));
		return TRUE;
	}
	else if (ParseCommand(&Cmd, TEXT("PAUSERENDERCLOCK")))
	{
		GPauseRenderingRealtimeClock = !GPauseRenderingRealtimeClock;
		Ar.Logf(TEXT("The global realtime rendering clock is now: %s"), GPauseRenderingRealtimeClock ? TEXT("PAUSED") : TEXT("RUNNING"));
		return TRUE;
	}
	else if ( UIController->Exec(Cmd,Ar) )
	{
		return TRUE;
	}
	else if(ScriptConsoleExec(Cmd,Ar,NULL))
	{
		return TRUE;
	}
	else if( GEngine->Exec(Cmd,Ar) )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


UBOOL UPlayer::Exec(const TCHAR* Cmd,FOutputDevice& Ar)
{
	if(Actor)
	{
		// Since UGameViewportClient calls Exec on UWorld, we only need to explicitly
		// call UWorld::Exec if we either have a null GEngine or a null ViewportClient
		UBOOL bWorldNeedsExec = GEngine == NULL || Cast<ULocalPlayer>(this) == NULL || static_cast<ULocalPlayer*>(this)->ViewportClient == NULL;
		if( bWorldNeedsExec && GWorld->Exec(Cmd,Ar) )
		{
			return TRUE;
		}
		else if( Actor->PlayerInput && Actor->PlayerInput->ScriptConsoleExec(Cmd,Ar,Actor->Pawn) )
		{
			return TRUE;
		}
		else if( Actor->ScriptConsoleExec(Cmd,Ar,Actor->Pawn) )
		{
			return TRUE;
		}
		else if( Actor->Pawn )
		{
			if( Actor->Pawn->ScriptConsoleExec(Cmd,Ar,Actor->Pawn) )
			{
				return TRUE;
			}
			else if( Actor->Pawn->InvManager && Actor->Pawn->InvManager->ScriptConsoleExec(Cmd,Ar,Actor->Pawn) )
			{
				return TRUE;
			}
			else if( Actor->Pawn->Weapon && Actor->Pawn->Weapon->ScriptConsoleExec(Cmd,Ar,Actor->Pawn) )
			{
				return TRUE;
			}
		}
		if( Actor->myHUD && Actor->myHUD->ScriptConsoleExec(Cmd,Ar,Actor->Pawn) )
		{
			return TRUE;
		}
		else if( GWorld->GetGameInfo() && GWorld->GetGameInfo()->ScriptConsoleExec(Cmd,Ar,Actor->Pawn) )
		{
			return TRUE;
		}
		else if( Actor->CheatManager && Actor->CheatManager->ScriptConsoleExec(Cmd,Ar,Actor->Pawn) )
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	return FALSE;
}


/**
 * Dynamically assign Controller to Player and set viewport.
 *
 * @param    PC - new player controller to assign to player
 **/
void UPlayer::SwitchController(class APlayerController* PC)
{
	// Detach old player.
	if( this->Actor )
	{
		this->Actor->Player = NULL;
	}

	// Set the viewport.
	PC->Player = this;
	this->Actor = PC;
}

ULocalPlayer::ULocalPlayer()
{
	if ( !IsTemplate() )
	{
		ViewState = AllocateViewState();

		if( !PlayerPostProcess )
		{
			// initialize to global post process if one is not set
			if (InsertPostProcessingChain(GEngine->DefaultPostProcess, 0, TRUE) == FALSE)
			{
				warnf(TEXT("LocalPlayer %d - Failed to setup default post process..."), ControllerId);
			}
		}

		// Initialize the actor visibility history.
		ActorVisibilityHistory.Init();
	}
}

UBOOL ULocalPlayer::GetActorVisibility(AActor* TestActor) const
{
	return ActorVisibilityHistory.GetActorVisibility(TestActor);
}

UBOOL ULocalPlayer::SpawnPlayActor(const FString& URL,FString& OutError)
{
	if ( GWorld->IsServer() )
	{
		Actor = GWorld->SpawnPlayActor(this,ROLE_SimulatedProxy,FURL(NULL,*URL,TRAVEL_Absolute),OutError,GEngine->GamePlayers.FindItemIndex(this));
	}
	else
	{
		UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
		// Statically bind to the specified player controller
		UClass* PCClass = GameEngine != NULL ?
			LoadClass<APlayerController>(NULL, *GameEngine->PendingLevelPlayerControllerClassName, NULL, LOAD_None, NULL) :
			NULL;
		if (PCClass == NULL)
		{
			// This failed to load so use the engine one as default
			PCClass = APlayerController::StaticClass();
		}
		debugf(TEXT("PlayerController class for the pending level is %s"),*PCClass->GetFName().ToString());
		// The PlayerController gets replicated from the client though the engine assumes that every Player always has
		// a valid PlayerController so we spawn a dummy one that is going to be replaced later.
		Actor = CastChecked<APlayerController>(GWorld->SpawnActor(PCClass));
		const INT PlayerIndex=GEngine->GamePlayers.FindItemIndex(this);
		Actor->NetPlayerIndex = PlayerIndex;
		if ( Actor->PlayerReplicationInfo != NULL )
		{
			Actor->PlayerReplicationInfo->SplitscreenIndex = PlayerIndex;
		}
	}
	return Actor != NULL;
}

void ULocalPlayer::SendSplitJoin()
{
	if (GWorld == NULL || GWorld->GetNetDriver() == NULL || GWorld->GetNetDriver()->ServerConnection == NULL || GWorld->GetNetDriver()->ServerConnection->State != USOCK_Open)
	{
		debugf(NAME_Warning, TEXT("SendSplitJoin(): Not connected to a server"));
	}
	else if (!bSentSplitJoin)
	{
		// make sure we don't already have a connection
		UBOOL bNeedToSendJoin = FALSE;
		if (Actor == NULL)
		{
			bNeedToSendJoin = TRUE;
		}
		else if (GWorld->GetNetDriver()->ServerConnection->Actor != Actor)
		{
			UNetDriver* NetDriver = GWorld->GetNetDriver();
			bNeedToSendJoin = TRUE;
			for (INT i = 0; i < NetDriver->ServerConnection->Children.Num(); i++)
			{
				if (NetDriver->ServerConnection->Children(i)->Actor == Actor)
				{
					bNeedToSendJoin = FALSE;
					break;
				}
			}
		}

		if (bNeedToSendJoin)
		{
			//@todo: send a separate URL?
			GWorld->GetNetDriver()->ServerConnection->Logf(TEXT("JOINSPLIT"));
			bSentSplitJoin = TRUE;
		}
	}
}

void ULocalPlayer::FinishDestroy()
{
	if ( !IsTemplate() )
	{
		ViewState->Destroy();
		ViewState = NULL;
	}
	Super::FinishDestroy();
}

/**
 * Add the given post process chain to the chain at the given index.
 *
 *	@param	InChain		The post process chain to insert.
 *	@param	InIndex		The position to insert the chain in the complete chain.
 *						If -1, insert it at the end of the chain.
 *	@param	bInClone	If TRUE, create a deep copy of the chains effects before insertion.
 */
UBOOL ULocalPlayer::InsertPostProcessingChain(class UPostProcessChain* InChain, INT InIndex, UBOOL bInClone)
{
	if (InChain == NULL)
	{
		return FALSE;
	}

	// Create a new chain...
	UPostProcessChain* ClonedChain = Cast<UPostProcessChain>(StaticDuplicateObject(InChain, InChain, UObject::GetTransientPackage(), TEXT("None"), RF_AllFlags & (~RF_Standalone)));
	if (ClonedChain)
	{
		INT InsertIndex = 0;
		if ((InIndex == -1) || (InIndex >= PlayerPostProcessChains.Num()))
		{
			InsertIndex = PlayerPostProcessChains.Num();
		}
		else
		{
			InsertIndex = InIndex;
		}

		PlayerPostProcessChains.InsertItem(ClonedChain, InsertIndex);

		RebuildPlayerPostProcessChain();

		return TRUE;
	}

	return FALSE;
}

/**
 * Remove the post process chain at the given index.
 *
 *	@param	InIndex		The position to insert the chain in the complete chain.
 */
UBOOL ULocalPlayer::RemovePostProcessingChain(INT InIndex)
{
	if ((InIndex >= 0) && (InIndex < PlayerPostProcessChains.Num()))
	{
		PlayerPostProcessChains.Remove(InIndex);
		RebuildPlayerPostProcessChain();
		return TRUE;
	}

	return FALSE;
}

/**
 * Remove all post process chains.
 *
 *	@return	boolean		TRUE if the chain array was cleared
 *						FALSE if not
 */
UBOOL ULocalPlayer::RemoveAllPostProcessingChains()
{
	PlayerPostProcessChains.Empty();
	RebuildPlayerPostProcessChain();
	return TRUE;
}

/**
 *	Get the PPChain at the given index.
 *
 *	@param	InIndex				The index of the chain to retrieve.
 *
 *	@return	PostProcessChain	The post process chain if found; NULL if not.
 */
class UPostProcessChain* ULocalPlayer::GetPostProcessChain(INT InIndex)
{
	if ((InIndex >= 0) && (InIndex < PlayerPostProcessChains.Num()))
	{
		return PlayerPostProcessChains(InIndex);
	}
	return NULL;
}

/**
 *	Forces the PlayerPostProcess chain to be rebuilt.
 *	This should be called if a PPChain is retrieved using the GetPostProcessChain,
 *	and is modified directly.
 */
void ULocalPlayer::TouchPlayerPostProcessChain()
{
	RebuildPlayerPostProcessChain();
}

/**
 *	Rebuilds the PlayerPostProcessChain.
 *	This should be called whenever the chain array has items inserted/removed.
 */
void ULocalPlayer::RebuildPlayerPostProcessChain()
{
	// Release the current PlayerPostProcessChain.
	if (PlayerPostProcessChains.Num() == 0)
	{
		PlayerPostProcess = NULL;
		return;
	}

	PlayerPostProcess = ConstructObject<UPostProcessChain>(UPostProcessChain::StaticClass(), UObject::GetTransientPackage());
	check(PlayerPostProcess);
	
	UBOOL bUberEffectInserted = FALSE;
	for (INT ChainIndex = 0; ChainIndex < PlayerPostProcessChains.Num(); ChainIndex++)
	{
		UPostProcessChain* PPChain = PlayerPostProcessChains(ChainIndex);
		if (PPChain)
		{
			for (INT EffectIndex = 0; EffectIndex < PPChain->Effects.Num(); EffectIndex++)
			{
				UPostProcessEffect* PPEffect = PPChain->Effects(EffectIndex);
				if (PPEffect)
				{
					if (PPEffect->IsA(UUberPostProcessEffect::StaticClass())== TRUE)
					{
						if (bUberEffectInserted == FALSE)
						{
							PlayerPostProcess->Effects.AddItem(PPEffect);
							bUberEffectInserted = TRUE;
						}
						else
						{
							warnf(TEXT("LocalPlayer %d - Multiple UberPostProcessEffects present..."), ControllerId);
						}
					}
					else
					{
						PlayerPostProcess->Effects.AddItem(PPEffect);
					}
				}
			}
		}
	}
}

//
void ULocalPlayer::UpdatePostProcessSettings(const FVector& ViewLocation)
{
	// Find the post-process settings for the view.
	FPostProcessSettings NewSettings;
	APostProcessVolume* NewVolume;

	// Give priority to local PP override flag
	if ( bOverridePostProcessSettings )
	{
		NewVolume = NULL;
		NewSettings = PostProcessSettingsOverride;
		CurrentPPInfo.BlendStartTime = PPSettingsOverrideStartBlend;
	}
	// If not forcing an override on the LocalPlayer, see if we have Camera that wants to override PP settings
	// If so, we just grab those settings straight away and return - no blending.
	else if(Actor && Actor->PlayerCamera && Actor->PlayerCamera->bCamOverridePostProcess)
	{
		NewVolume = NULL;
 		CurrentPPInfo.LastSettings = Actor->PlayerCamera->CamPostProcessSettings;
		return;
	}
	else
	{
		NewVolume = GWorld->GetWorldInfo()->GetPostProcessSettings(ViewLocation,TRUE,NewSettings);
	}
	
	// Give the camera an opportunity to do any non-overriding modifications (e.g. additive effects)
	if (Actor != NULL)
	{
		Actor->ModifyPostProcessSettings(NewSettings);
	}


	const FLOAT CurrentWorldTime = GWorld->GetRealTimeSeconds();

	// Update info for when a new volume goes into use
	if( CurrentPPInfo.LastVolumeUsed != NewVolume )
	{
		CurrentPPInfo.LastVolumeUsed = NewVolume;
		CurrentPPInfo.BlendStartTime = CurrentWorldTime;
	}

	// Calculate the blend factors.
	const FLOAT DeltaTime = Max(CurrentWorldTime - CurrentPPInfo.LastBlendTime,0.f);
	const FLOAT ElapsedBlendTime = Max(CurrentPPInfo.LastBlendTime - CurrentPPInfo.BlendStartTime,0.f);

	// Calculate the blended settings.
	FPostProcessSettings BlendedSettings;
	const FPostProcessSettings& CurrentSettings = CurrentPPInfo.LastSettings;

	// toggles
	BlendedSettings.bEnableBloom = NewSettings.bEnableBloom;
	BlendedSettings.bEnableDOF = NewSettings.bEnableDOF;
	BlendedSettings.bEnableMotionBlur = NewSettings.bEnableMotionBlur;
	BlendedSettings.bEnableSceneEffect = NewSettings.bEnableSceneEffect;
	BlendedSettings.bAllowAmbientOcclusion = NewSettings.bAllowAmbientOcclusion;

	// calc bloom lerp amount
	FLOAT BloomFade = 1.f;
	const FLOAT RemainingBloomBlendTime = Max(NewSettings.Bloom_InterpolationDuration - ElapsedBlendTime,0.f);
	if(RemainingBloomBlendTime > DeltaTime)
	{
		BloomFade = Clamp<FLOAT>(DeltaTime / RemainingBloomBlendTime,0.f,1.f);
	}
	// bloom values
	BlendedSettings.Bloom_Scale = Lerp<FLOAT>(CurrentSettings.Bloom_Scale,NewSettings.Bloom_Scale,BloomFade);

	// calc dof lerp amount
	FLOAT DOFFade = 1.f;
	const FLOAT RemainingDOFBlendTime = Max(NewSettings.DOF_InterpolationDuration - ElapsedBlendTime,0.f);
	if(RemainingDOFBlendTime > DeltaTime)
	{
		DOFFade = Clamp<FLOAT>(DeltaTime / RemainingDOFBlendTime,0.f,1.f);
	}
	// dof values		
	BlendedSettings.DOF_FalloffExponent = Lerp<FLOAT>(CurrentSettings.DOF_FalloffExponent,NewSettings.DOF_FalloffExponent,DOFFade);
	BlendedSettings.DOF_BlurKernelSize = Lerp<FLOAT>(CurrentSettings.DOF_BlurKernelSize,NewSettings.DOF_BlurKernelSize,DOFFade);
	BlendedSettings.DOF_MaxNearBlurAmount = Lerp<FLOAT>(CurrentSettings.DOF_MaxNearBlurAmount,NewSettings.DOF_MaxNearBlurAmount,DOFFade);
	BlendedSettings.DOF_MaxFarBlurAmount = Lerp<FLOAT>(CurrentSettings.DOF_MaxFarBlurAmount,NewSettings.DOF_MaxFarBlurAmount,DOFFade);
	BlendedSettings.DOF_ModulateBlurColor = FColor(Lerp<FLinearColor>(CurrentSettings.DOF_ModulateBlurColor,NewSettings.DOF_ModulateBlurColor,DOFFade));
	BlendedSettings.DOF_FocusType = NewSettings.DOF_FocusType;
	BlendedSettings.DOF_FocusInnerRadius = Lerp<FLOAT>(CurrentSettings.DOF_FocusInnerRadius,NewSettings.DOF_FocusInnerRadius,DOFFade);
	BlendedSettings.DOF_FocusDistance = Lerp<FLOAT>(CurrentSettings.DOF_FocusDistance,NewSettings.DOF_FocusDistance,DOFFade);
	BlendedSettings.DOF_FocusPosition = Lerp<FVector>(CurrentSettings.DOF_FocusPosition,NewSettings.DOF_FocusPosition,DOFFade);

	// calc motion blur lerp amount
	FLOAT MotionBlurFade = 1.f;
	const FLOAT RemainingMotionBlurBlendTime = Max(NewSettings.MotionBlur_InterpolationDuration - ElapsedBlendTime,0.f);
	if(RemainingMotionBlurBlendTime > DeltaTime)
	{
		MotionBlurFade = Clamp<FLOAT>(DeltaTime / RemainingMotionBlurBlendTime,0.f,1.f);
	}
	// motion blur values
	BlendedSettings.MotionBlur_MaxVelocity = Lerp<FLOAT>(CurrentSettings.MotionBlur_MaxVelocity,NewSettings.MotionBlur_MaxVelocity,MotionBlurFade);
	BlendedSettings.MotionBlur_Amount = Lerp<FLOAT>(CurrentSettings.MotionBlur_Amount,NewSettings.MotionBlur_Amount,MotionBlurFade);
	BlendedSettings.MotionBlur_CameraRotationThreshold = Lerp<FLOAT>(CurrentSettings.MotionBlur_CameraRotationThreshold,NewSettings.MotionBlur_CameraRotationThreshold,MotionBlurFade);
	BlendedSettings.MotionBlur_CameraTranslationThreshold = Lerp<FLOAT>(CurrentSettings.MotionBlur_CameraTranslationThreshold,NewSettings.MotionBlur_CameraTranslationThreshold,MotionBlurFade);
	BlendedSettings.MotionBlur_FullMotionBlur = NewSettings.MotionBlur_FullMotionBlur;

	// calc scene material lerp amount
	FLOAT SceneMaterialFade = 1.f;
	const FLOAT RemainingSceneBlendTime = Max(NewSettings.Scene_InterpolationDuration - ElapsedBlendTime,0.f);
	if(RemainingSceneBlendTime > DeltaTime)
	{
		SceneMaterialFade = Clamp<FLOAT>(DeltaTime / RemainingSceneBlendTime,0.f,1.f);
	}
	// scene material values
	BlendedSettings.Scene_Desaturation	= Lerp<FLOAT>(CurrentSettings.Scene_Desaturation,NewSettings.Scene_Desaturation*PP_DesaturationMultiplier,SceneMaterialFade);
	BlendedSettings.Scene_HighLights	= Lerp<FVector>(CurrentSettings.Scene_HighLights,NewSettings.Scene_HighLights*PP_HighlightsMultiplier,SceneMaterialFade);
	BlendedSettings.Scene_MidTones		= Lerp<FVector>(CurrentSettings.Scene_MidTones,NewSettings.Scene_MidTones*PP_MidTonesMultiplier,SceneMaterialFade);
	BlendedSettings.Scene_Shadows		= Lerp<FVector>(CurrentSettings.Scene_Shadows,NewSettings.Scene_Shadows*PP_ShadowsMultiplier,SceneMaterialFade);

	// Clamp desaturation to 0..1 range to allow desaturation multipliers > 1 without color shifts at high desaturation.
	BlendedSettings.Scene_Desaturation = Clamp( BlendedSettings.Scene_Desaturation, 0.f, 1.f );

	// the scene material only needs to be enabled if the values don't match the default
	// as it should be setup to not have any affect in the default case
	if( BlendedSettings.bEnableSceneEffect )
	{
		if( BlendedSettings.Scene_Desaturation == 0.0f &&
			BlendedSettings.Scene_HighLights.Equals(FVector(1,1,1)) &&
			BlendedSettings.Scene_MidTones.Equals(FVector(1,1,1)) &&
			BlendedSettings.Scene_Shadows.Equals(FVector(0,0,0)) )
		{
			BlendedSettings.bEnableSceneEffect = FALSE;
		}
	}

	// Update the current settings and timer.
	CurrentPPInfo.LastSettings = BlendedSettings;
	CurrentPPInfo.LastBlendTime = CurrentWorldTime;
}

/**
 * Calculate the view settings for drawing from this view actor
 *
 * @param	View - output view struct
 * @param	ViewLocation - output actor location
 * @param	ViewRotation - output actor rotation
 * @param	Viewport - current client viewport
 */
FSceneView* ULocalPlayer::CalcSceneView( FSceneViewFamily* ViewFamily, FVector& ViewLocation, FRotator& ViewRotation, FViewport* Viewport )
{
	if( !Actor )
	{
		return NULL;
	}

	// do nothing if the viewport size is zero - this allows the viewport client the capability to temporarily remove a viewport without actually destroying and recreating it
	if (Size.X <= 0.f || Size.Y <= 0.f)
	{
		return NULL;
	}

	check(Viewport);

	// Compute the view's screen rectangle.
	INT X = appTrunc(Origin.X * Viewport->GetSizeX());
	INT Y = appTrunc(Origin.Y * Viewport->GetSizeY());
	UINT SizeX = appTrunc(Size.X * Viewport->GetSizeX());
	UINT SizeY = appTrunc(Size.Y * Viewport->GetSizeY());
	
	// if the object propagtor is pushing us new values, use them instead of the player
	if (bOverrideView)
	{
		ViewLocation = OverrideLocation;
		ViewRotation = OverrideRotation;
	}
	else
	{
		Actor->eventGetPlayerViewPoint( ViewLocation, ViewRotation );
	}
	FLOAT fFOV = Actor->eventGetFOVAngle();
	// scale distances for cull distance purposes by the ratio of our current FOV to the default FOV
	Actor->LODDistanceFactor = fFOV / Max<FLOAT>(0.01f, (Actor->PlayerCamera != NULL) ? Actor->PlayerCamera->DefaultFOV : Actor->DefaultFOV);
	
	FMatrix ViewMatrix = FTranslationMatrix(-ViewLocation);
	ViewMatrix = ViewMatrix * FInverseRotationMatrix(ViewRotation);
	ViewMatrix = ViewMatrix * FMatrix(
		FPlane(0,	0,	1,	0),
		FPlane(1,	0,	0,	0),
		FPlane(0,	1,	0,	0),
		FPlane(0,	0,	0,	1));

	UGameUISceneClient* SceneClient = UUIRoot::GetSceneClient();

	FMatrix ProjectionMatrix;
	if( Actor && Actor->PlayerCamera != NULL && Actor->PlayerCamera->bConstrainAspectRatio )
	{
		ProjectionMatrix = FPerspectiveMatrix(
			fFOV * (FLOAT)PI / 360.0f,
			Actor->PlayerCamera->ConstrainedAspectRatio,
			1.0f,
			NEAR_CLIPPING_PLANE
			);

		// Enforce a particular aspect ratio for the render of the scene. 
		// Results in black bars at top/bottom etc.
		Viewport->CalculateViewExtents( 
				Actor->PlayerCamera->ConstrainedAspectRatio, 
				X, Y, SizeX, SizeY );
	}
	else 
	{
		FLOAT CurViewAspectRatio = ((FLOAT)Viewport->GetSizeX()) / ((FLOAT)Viewport->GetSizeY());
		ProjectionMatrix = FPerspectiveMatrix(
			fFOV * (FLOAT)PI / 360.0f,
			SizeX * Viewport->GetDesiredAspectRatio() / CurViewAspectRatio,
			SizeY,
			NEAR_CLIPPING_PLANE
			);
	}

	// Take screen percentage option into account if percentage != 100.
	// Note: this needs to be done after the view size and position are final
	GSystemSettings.ScaleScreenCoords(X,Y,SizeX,SizeY);

	FLinearColor OverlayColor(0,0,0,0);
	FLinearColor ColorScale(FLinearColor::White);

	if( Actor && Actor->PlayerCamera )
	{
		// Apply screen fade effect to screen.
		if(Actor->PlayerCamera->bEnableFading)
		{
			OverlayColor = Actor->PlayerCamera->FadeColor.ReinterpretAsLinear();
			OverlayColor.A = Clamp(Actor->PlayerCamera->FadeAmount,0.0f,1.0f);
		}

		// Do color scaling if desired.
		if(Actor->PlayerCamera->bEnableColorScaling)
		{
			ColorScale = FLinearColor(
				Actor->PlayerCamera->ColorScale.X,
				Actor->PlayerCamera->ColorScale.Y,
				Actor->PlayerCamera->ColorScale.Z
				);
		}
	}

	// Update the player's post process settings.
	UpdatePostProcessSettings(ViewLocation);

	TSet<UPrimitiveComponent*> HiddenPrimitives;

	// Translate the camera's hidden actors list to a hidden primitive list.
	Actor->UpdateHiddenActors(ViewLocation);
	const TArray<AActor*>& HiddenActors = Actor->HiddenActors;
	for(INT ActorIndex = 0; ActorIndex < HiddenActors.Num(); ActorIndex++)
	{
		AActor* HiddenActor = HiddenActors(ActorIndex);
		if(HiddenActor)
		{
			for(INT ComponentIndex = 0; ComponentIndex < HiddenActor->AllComponents.Num(); ComponentIndex++)
			{
				UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(HiddenActor->AllComponents(ComponentIndex));
				if(PrimitiveComponent && !PrimitiveComponent->bIgnoreHiddenActorsMembership)
				{
					HiddenPrimitives.Add(PrimitiveComponent);
				}
			}
		}
	}

	FSceneView* View = new FSceneView(
		ViewFamily,
		ViewState,
		-1,
		NULL,
		&ActorVisibilityHistory,
		Actor->GetViewTarget(),
		PlayerPostProcess,
		&CurrentPPInfo.LastSettings,
		NULL,
		NULL,
		X,
		Y,
		SizeX,
		SizeY,
		ViewMatrix,
		ProjectionMatrix,
		FLinearColor::Black,
		OverlayColor,
		ColorScale,
		HiddenPrimitives,
		Actor->LODDistanceFactor
		);
	ViewFamily->Views.AddItem(View);

	return View;
}

//
//	ULocalPlayer::Exec
//

UBOOL ULocalPlayer::Exec(const TCHAR* Cmd,FOutputDevice& Ar)
{
	// Create a pending Note actor (only in PIE)
	if( ParseCommand(&Cmd,TEXT("DN")) )
	{
		// Do nothing if not in editor
		if(GIsEditor && Actor && Actor->Pawn)
		{
			FString Comment = FString(Cmd);
			INT NewNoteIndex = GEngine->PendingDroppedNotes.AddZeroed();
			FDropNoteInfo& NewNote = GEngine->PendingDroppedNotes(NewNoteIndex);
			NewNote.Location = Actor->Pawn->Location;
			NewNote.Rotation = Actor->Rotation;
			NewNote.Comment = Comment;
			debugf(TEXT("Note Dropped: (%3.2f,%3.2f,%3.2f) - '%s'"), NewNote.Location.X, NewNote.Location.Y, NewNote.Location.Z, *NewNote.Comment);
		}
		return TRUE;
	}
    // This will show all of the SkeletalMeshComponents that were ticked for one frame
	else if( ParseCommand(&Cmd,TEXT("SHOWSKELCOMPTICKTIME")) )
	{
		GShouldLogOutAFrameOfSkelCompTick = TRUE;
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("SHOWLIGHTENVS")) )
	{
		GShouldLofOutAFrameOfLightEnvTick = TRUE;
		return TRUE;
	}
	// This will show all IsOverlapping calls for one frame
	else if( ParseCommand(&Cmd,TEXT("SHOWISOVERLAPPING")) )
	{
		GShouldLogOutAFrameOfIsOverlapping = TRUE;
		return TRUE;
	}
	// This will list all awake rigid bodies
	else if( ParseCommand(&Cmd,TEXT("LISTAWAKEBODIES")) )
	{
		ListAwakeRigidBodies();
		return TRUE;
	}
	// Allow crowds to be toggled on and off.
	else if( ParseCommand(&Cmd, TEXT("TOGGLECROWDS")) )
	{
		GWorld->bDisableCrowds = !GWorld->bDisableCrowds;
		return TRUE;
	}
	else if( ParseCommand(&Cmd, TEXT("MOVEACTORTIMES")) )
	{
		GShouldLogOutAFrameOfMoveActor = TRUE;
		return TRUE;
	}
	else if( ParseCommand(&Cmd, TEXT("PHYSASSETBOUNDS")) )
	{
		GShouldLogOutAFrameOfPhysAssetBoundsUpdate = TRUE;
		return TRUE;
	}
	else if( ParseCommand(&Cmd, TEXT("FRAMECOMPUPDATES")) )
	{
		GShouldLogOutAFrameOfComponentUpdates = TRUE;
		return TRUE;
	}
	else if( ParseCommand(&Cmd, TEXT("FRAMEOFPAIN")) )
	{
		GShouldTraceOutAFrameOfPain = TRUE;
		return TRUE;
	}
	else if( ParseCommand(&Cmd, TEXT("SHOWSKELCOMPLODS")) )
	{
		GShouldLogOutAFrameOfSkelCompLODs = TRUE;
		return TRUE;
	}
	else if( ParseCommand(&Cmd, TEXT("SHOWSKELMESHLODS")) )
	{
		debugf(TEXT("============================================================"));
		debugf(TEXT("Verifying SkeleltalMesh : STARTING"));
		debugf(TEXT("============================================================"));

		GShouldLogOutAFrameOfSkelMeshLODs = TRUE;
		return TRUE;
	}
	else if ( ParseCommand(&Cmd, TEXT("SHOWFACEFXBONES")) )
	{
		debugf(TEXT("============================================================"));
		debugf(TEXT("Verifying FaceFX Bones of SkeletalMeshComp : STARTING"));
		debugf(TEXT("============================================================"));

		GShouldLogOutAFrameOfFaceFXBones = TRUE;
		return TRUE;
	}
	else if ( ParseCommand(&Cmd, TEXT("SHOWFACEFXDEBUG")) )
	{
		debugf(TEXT("============================================================"));

		GShouldLogOutAFrameOfFaceFXDebug = TRUE;
		return TRUE;
	}
	else if ( ParseCommand(&Cmd, TEXT("TRACEFACEFX")) )
	{
		GShouldTraceFaceFX = TRUE;
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("LISTSKELMESHES")) )
	{
		// Iterate over all skeletal mesh components and create mapping from skeletal mesh to instance.
		TMultiMap<USkeletalMesh*,USkeletalMeshComponent*> SkeletalMeshToInstancesMultiMap;
		for( TObjectIterator<USkeletalMeshComponent> It; It; ++It )
		{
			USkeletalMeshComponent* SkeletalMeshComponent = *It;
			USkeletalMesh* SkeletalMesh = SkeletalMeshComponent->SkeletalMesh;

			if( !SkeletalMeshComponent->IsTemplate() )
			{
				SkeletalMeshToInstancesMultiMap.Add( SkeletalMesh, SkeletalMeshComponent );
			}
		}

		// Retrieve player location for distance checks.
		FVector PlayerLocation = FVector(0,0,0);
		if( Actor && Actor->Pawn )
		{
			PlayerLocation = Actor->Pawn->Location;
		}

		// Iterate over multi-map and dump information sorted by skeletal mesh.
		for( TObjectIterator<USkeletalMesh> It; It; ++It )
		{
			// Look up array of instances associated with this key/ skeletal mesh.
			USkeletalMesh* SkeletalMesh = *It;
			TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
			SkeletalMeshToInstancesMultiMap.MultiFind( SkeletalMesh, SkeletalMeshComponents );

			if( SkeletalMesh && SkeletalMeshComponents.Num() )
			{
				// Dump information about skeletal mesh.
				check(SkeletalMesh->LODModels.Num());
				debugf(TEXT("%5i Vertices for LOD 0 of %s"),SkeletalMesh->LODModels(0).NumVertices,*SkeletalMesh->GetFullName());

				// Dump all instances.
				for( INT InstanceIndex=0; InstanceIndex<SkeletalMeshComponents.Num(); InstanceIndex++ )
				{
					USkeletalMeshComponent* SkeletalMeshComponent = SkeletalMeshComponents(InstanceIndex);
					FLOAT TimeSinceLastRender = GWorld->GetTimeSeconds() - SkeletalMeshComponent->LastRenderTime;

					debugf(TEXT("%s%2i  Component    : %s"), 
						(TimeSinceLastRender > 0.5) ? TEXT(" ") : TEXT("*"), 
						InstanceIndex,
						*SkeletalMeshComponent->GetFullName() );
					if( SkeletalMeshComponent->GetOwner() )
					{
						debugf(TEXT("     Owner        : %s"),*SkeletalMeshComponent->GetOwner()->GetFullName());
					}
					debugf(TEXT("     LastRender   : %f"), TimeSinceLastRender);
					debugf(TEXT("     CullDistance : %f   Distance: %f   Location: (%7.1f,%7.1f,%7.1f)"), 
						SkeletalMeshComponent->CachedMaxDrawDistance,	
						FDist( PlayerLocation, SkeletalMeshComponent->Bounds.Origin ),
						SkeletalMeshComponent->Bounds.Origin.X,
						SkeletalMeshComponent->Bounds.Origin.Y,
						SkeletalMeshComponent->Bounds.Origin.Z );
				}
			}
		}
		return TRUE;
	}
	else if ( ParseCommand(&Cmd,TEXT("LISTPAWNCOMPONENTS")) )
	{
		for (APawn *Pawn = GWorld->GetWorldInfo()->PawnList; Pawn != NULL; Pawn = Pawn->NextPawn)
		{
			debugf(TEXT("Components for pawn: %s (collision component: %s)"),*Pawn->GetName(),*Pawn->CollisionComponent->GetName());
			for (INT CompIdx = 0; CompIdx < Pawn->Components.Num(); CompIdx++)
			{
				UActorComponent *Comp = Pawn->Components(CompIdx);
				if (Comp != NULL)
				{
					debugf(TEXT("  %d: %s"),CompIdx,*Comp->GetName());
				}
			}
		}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("EXEC")) )
	{
		TCHAR Filename[512];
		if( ParseToken( Cmd, Filename, ARRAY_COUNT(Filename), 0 ) )
		{
			ExecMacro( Filename, Ar );
		}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("TOGGLEDRAWEVENTS")) )
	{
#if XBOX && !_DEBUG
		debugf(TEXT("Draw events are automatically enabled on Xbox 360 in Release when PIX is attached; no need to use TOGGLEDRAWEVENTS"));
#else
		if( GEmitDrawEvents )
		{
			GEmitDrawEvents = FALSE;
			debugf(TEXT("Draw events are now DISABLED"));
		}
		else
		{
			GEmitDrawEvents = TRUE;
			debugf(TEXT("Draw events are now ENABLED"));
		}
#endif
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("TOGGLESTREAMINGVOLUMES")) )
	{
		if (ParseCommand(&Cmd, TEXT("ON")))
		{
			GWorld->DelayStreamingVolumeUpdates( 0 );
		}
		else if (ParseCommand(&Cmd, TEXT("OFF")))
		{
			GWorld->DelayStreamingVolumeUpdates( INDEX_NONE );
		}
		else
		{
			if( GWorld->StreamingVolumeUpdateDelay == INDEX_NONE )
			{
				GWorld->DelayStreamingVolumeUpdates( 0 );
			}
			else
			{
				GWorld->DelayStreamingVolumeUpdates( INDEX_NONE );
			}
		}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("PUSHVIEW")) )
	{
		if (ParseCommand(&Cmd, TEXT("START")))
		{
			bOverrideView = TRUE;
		}
		else if (ParseCommand(&Cmd, TEXT("STOP")))
		{
			bOverrideView = FALSE;
		}
		else if (ParseCommand(&Cmd, TEXT("SYNC")))
		{
			if (bOverrideView)
			{
				// @todo: with PIE, this maybe be the wrong PlayWorld!
				GWorld->FarMoveActor(Actor->Pawn ? (AActor*)Actor->Pawn : Actor, OverrideLocation, FALSE, TRUE, TRUE);
				Actor->SetRotation(OverrideRotation);
			}
		}
		else
		{
			OverrideLocation.X = appAtof(*ParseToken(Cmd, 0));
			OverrideLocation.Y = appAtof(*ParseToken(Cmd, 0));
			OverrideLocation.Z = appAtof(*ParseToken(Cmd, 0));

			OverrideRotation.Pitch = appAtoi(*ParseToken(Cmd, 0));
			OverrideRotation.Yaw   = appAtoi(*ParseToken(Cmd, 0));
			OverrideRotation.Roll  = appAtoi(*ParseToken(Cmd, 0));
		}
		return TRUE;
	}
	// @hack: This is a test matinee skipping function, quick and dirty to see if it's good enough for
	// gameplay. Will fix up better when we have some testing done!
	else if (ParseCommand(&Cmd, TEXT("CANCELMATINEE")))
	{
		UBOOL bMatineeSkipped = FALSE;

		// allow optional parameter for initial time in the matinee that this won't work (ie, 
		// 'cancelmatinee 5' won't do anything in the first 5 seconds of the matinee)
		FLOAT InitialNoSkipTime = appAtof(Cmd);

		// is the player in cinematic mode?
		if (Actor->bCinematicMode)
		{
			// if so, look for all active matinees that has this Player in a director group
			for (TObjectIterator<USeqAct_Interp> It; It; ++It)
			{
				// isit currently playing (and skippable)?
				if (It->bIsPlaying && It->bIsSkippable && (It->bClientSideOnly || GWorld->IsServer()))
				{
					for (INT GroupIndex = 0; GroupIndex < It->GroupInst.Num(); GroupIndex++)
					{
						// is the PC the group actor?
						if (It->GroupInst(GroupIndex)->GetGroupActor() == Actor)
						{
							const FLOAT RightBeforeEndTime = 0.1f;
							// make sure we aren';t already at the end (or before the allowed skip time)
							if ((It->Position < It->InterpData->InterpLength - RightBeforeEndTime) && 
								(It->Position >= InitialNoSkipTime))
							{
								// skip to end
								It->SetPosition(It->InterpData->InterpLength - RightBeforeEndTime, TRUE);

								// send a callback that this was skipped
								GCallbackEvent->Send(CALLBACK_MatineeCanceled, *It);

								bMatineeSkipped = TRUE;

								extern FLOAT HACK_DelayAfterSkip;
								// for 2 seconds after actually skipping a matinee, don't allow savegame loadng
								HACK_DelayAfterSkip = 2.0f;
							}
						}
					}
				}
			}

			if(bMatineeSkipped && GWorld && GWorld->GetGameInfo())
			{
				GWorld->GetGameInfo()->eventMatineeCancelled();
			}
		}
		return TRUE;
	}
	else if(ViewportClient && ViewportClient->Exec(Cmd,Ar))
	{
		return TRUE;
	}
	else if ( Super::Exec( Cmd, Ar ) )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void ULocalPlayer::ExecMacro( const TCHAR* Filename, FOutputDevice& Ar )
{
	// make sure Binaries is specified in the filename
	FString FixedFilename;
	if (!appStristr(Filename, TEXT("Binaries")))
	{
		FixedFilename = FString(TEXT("..\\Binaries\\")) + Filename;
		Filename = *FixedFilename;
	}

	FString Text;
	if (appLoadFileToString(Text, Filename))
	{
		debugf(TEXT("Execing %s"), Filename);
		const TCHAR* Data = *Text;
		FString Line;
		while( ParseLine(&Data, Line) )
		{
			Exec(*Line, Ar);
		}
	}
	else
	{
		Ar.Logf( NAME_ExecWarning, LocalizeSecure(LocalizeError("FileNotFound",TEXT("Core")), Filename) );
	}
}

void FConsoleOutputDevice::Serialize(const TCHAR* Text,EName Event)
{
	FStringOutputDevice::Serialize(Text,Event);
	FStringOutputDevice::Serialize(TEXT("\n"),Event);
	GLog->Serialize(Text,Event);

	if( Console != NULL )
	{
		Console->eventOutputText(Text);
	}
}

// We expose these variables to everyone as we need to access them in other files via an extern
FLOAT GAverageFPS = 0.0f;
FLOAT GAverageMS = 0.0f;

 /**
 *	Renders the FPS counter
 *
 *	@param Viewport	The viewport to render to
 *	@param Canvas	Canvas object to use for rendering
 *	@param X		Suggested X coordinate for where to start drawing
 *	@param Y		Suggested Y coordinate for where to start drawing
 *	@return			Y coordinate of the next line after this output
 */
INT DrawFPSCounter( FViewport* Viewport, FCanvas* Canvas, INT X, INT Y )
{
	// Render the FPS counter.
	if( GShowFpsCounter )
	{
		// Pick a larger font on console.
#if CONSOLE
		UFont* Font = GEngine->MediumFont;
#else
		UFont* Font = GEngine->SmallFont;
#endif

#if STATS
		// Calculate the average frame time by using the stats system.
		GAverageMS	= (FLOAT)GFPSCounter.GetAverage() * 1000.0;
#else
		// Calculate the average frame time via continued averaging.
		static DOUBLE LastTime	= 0;
		DOUBLE CurrentTime		= appSeconds();
		FLOAT FrameTime			= (CurrentTime - LastTime) * 1000;
		GAverageMS				= GAverageMS * 0.1 + FrameTime * 0.9;
		LastTime				= CurrentTime;
#endif
		// Choose the counter color based on the average framerate.
		GAverageFPS = 1000.f / GAverageMS;
		FColor FPSColor = GAverageFPS < 20.0f ? FColor(255,0,0) : (GAverageFPS < 29.5f ? FColor(255,255,0) : FColor(0,255,0));

		// Start drawing the various counters.
		const INT RowHeight = appTrunc( Font->GetMaxCharHeight() * 1.1f );
#if STATS
		// Draw the FPS counter.
		DrawShadowedString(Canvas,
			X,
			Y,
			*FString::Printf(TEXT("%5.2f FPS"), GAverageFPS),
			Font,
			FPSColor
			);
		Y += RowHeight;
#endif
		// Draw the frame time.
		DrawShadowedString(Canvas,
			X,
			Y,
			*FString::Printf(TEXT("%5.2f ms"), GAverageMS),
			Font,
			FPSColor
			);
		Y += RowHeight;
	}

	return Y;
}


// We expose these variables to everyone as we need to access them in other files via an extern
FLOAT GUnit_RenderThreadTime = 0.0f;
FLOAT GUnit_GameThreadTime = 0.0f;
FLOAT GUnit_GPUFrameTime = 0.0f;
FLOAT GUnit_FrameTime = 0.0f;

/**
 *	Draws frame times for the overall frame, gamethread, renderthread and GPU.
 *	The gamethread time excludes idle time while it's waiting for the render thread.
 *	The renderthread time excludes idle time while it's waiting for more commands from the gamethread or waiting for the GPU to swap backbuffers.
 *	The GPU time is a measurement on the GPU from the beginning of the first drawcall to the end of the last drawcall. It excludes
 *	idle time while waiting for VSYNC. However, it will include any starvation time between drawcalls.
 *
 *	@param Viewport	The viewport to render to
 *	@param Canvas	Canvas object to use for rendering
 *	@param X		Suggested X coordinate for where to start drawing
 *	@param Y		Suggested Y coordinate for where to start drawing
 *	@return			Y coordinate of the next line after this output
 */
INT DrawUnitTimes( FViewport* Viewport, FCanvas* Canvas, INT X, INT Y )
{
	/** How many cycles the renderthread used (excluding idle time). It's set once per frame in FViewport::Draw. */
	extern DWORD GRenderThreadTime;
	/** How many cycles the gamethread used (excluding idle time). It's set once per frame in FViewport::Draw. */
	extern DWORD GGameThreadTime;

	static DOUBLE LastTime = 0.0;
	const DOUBLE CurrentTime = appSeconds();
	if ( LastTime == 0 )
	{
		LastTime = CurrentTime;
	}
	GUnit_FrameTime			= 0.9 * GUnit_FrameTime + 0.1 * (CurrentTime - LastTime) * 1000.0f;
	LastTime				= CurrentTime;

	/** Number of milliseconds the gamethread was used last frame. */
	GUnit_GameThreadTime = 0.9 * GUnit_GameThreadTime + 0.1 * GGameThreadTime * GSecondsPerCycle * 1000.0;
	appSetCounterValue( TEXT("Game thread time"), GGameThreadTime * GSecondsPerCycle * 1000.0 );

	/** Number of milliseconds the renderthread was used last frame. */
	GUnit_RenderThreadTime = 0.9 * GUnit_RenderThreadTime + 0.1 * GRenderThreadTime * GSecondsPerCycle * 1000.0;
	appSetCounterValue( TEXT("Render thread time"), GRenderThreadTime * GSecondsPerCycle * 1000.0 );

#if CONSOLE
	/** Number of milliseconds the GPU was busy last frame. */
	const DWORD GPUCycles = RHIGetGPUFrameCycles();
	GUnit_GPUFrameTime = 0.9 * GUnit_GPUFrameTime + 0.1 * GPUCycles * GSecondsPerCycle * 1000.0;
	appSetCounterValue( TEXT("GPU time"), GPUCycles * GSecondsPerCycle * 1000.0 );
#endif


	// Render CPU thread and GPU frame times.
	if( GShowUnitTimes )
	{
#if CONSOLE
		UFont* Font		= GEngine->MediumFont;
		const INT SafeZone	= appTrunc(Viewport->GetSizeX() * 0.05f);
#else
		UFont* Font		= GEngine->SmallFont;
		const INT SafeZone	= 0;
#endif
		
		FColor Color;
		const INT X2			= Viewport->GetSizeX() - Font->GetStringSize( TEXT(" 000.00 ms ") ) - SafeZone;
		const INT X1			= X2 - Font->GetStringSize( TEXT("Frame: ") );
		const INT RowHeight		= appTrunc( Font->GetMaxCharHeight() * 1.1f );

		// 0-34 ms: Green, 34-50 ms: Yellow, 50+ ms: Red
		Color = GUnit_FrameTime < 34.0f ? FColor(0,255,0) : (GUnit_FrameTime < 50.0f ? FColor(255,255,0) : FColor(255,0,0));
		DrawShadowedString(Canvas,X1,Y, *FString::Printf(TEXT("Frame:")),Font,FColor(255,255,255));
		DrawShadowedString(Canvas,X2,Y, *FString::Printf(TEXT("%3.2f ms"), GUnit_FrameTime),Font,Color);
		Y += RowHeight;

		Color = GUnit_GameThreadTime < 34.0f ? FColor(0,255,0) : (GUnit_GameThreadTime < 50.0f ? FColor(255,255,0) : FColor(255,0,0));
		DrawShadowedString(Canvas,X1,Y, *FString::Printf(TEXT("Game:")),Font,FColor(255,255,255));
		DrawShadowedString(Canvas,X2,Y, *FString::Printf(TEXT("%3.2f ms"), GUnit_GameThreadTime),Font,Color);
		Y += RowHeight;

		Color = GUnit_RenderThreadTime < 34.0f ? FColor(0,255,0) : (GUnit_RenderThreadTime < 50.0f ? FColor(255,255,0) : FColor(255,0,0));
		DrawShadowedString(Canvas,X1,Y, *FString::Printf(TEXT("Draw:")),Font,FColor(255,255,255));
		DrawShadowedString(Canvas,X2,Y, *FString::Printf(TEXT("%3.2f ms"), GUnit_RenderThreadTime),Font,Color);
		Y += RowHeight;

#if CONSOLE
		if ( GPUCycles > 0 )
		{
			Color = GUnit_GPUFrameTime < 34.0f ? FColor(0,255,0) : (GUnit_GPUFrameTime < 50.0f ? FColor(255,255,0) : FColor(255,0,0));
			DrawShadowedString(Canvas,X1,Y, *FString::Printf(TEXT("GPU:")),Font,FColor(255,255,255));
			DrawShadowedString(Canvas,X2,Y, *FString::Printf(TEXT("%3.2f ms"), GUnit_GPUFrameTime),Font,Color);
			Y += RowHeight;
		}
#endif
	}
	return Y;
}


/** This will set the StreamingLevels TMap with the current Streaming Level Status and also set which level the player is in **/
void GetLevelStremingStatus( TMap<FName,INT>& StreamingLevels, FString& LevelPlayerIsInName )
{
	// Iterate over the world info's level streaming objects to find and see whether levels are loaded, visible or neither.
	AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
	for( INT LevelIndex=0; LevelIndex<WorldInfo->StreamingLevels.Num(); LevelIndex++ )
	{
		ULevelStreaming* LevelStreaming = WorldInfo->StreamingLevels(LevelIndex);
		if( LevelStreaming 
			&&  LevelStreaming->PackageName != NAME_None 
			&&	LevelStreaming->PackageName != GWorld->GetOutermost()->GetFName() )
		{
			if( LevelStreaming->LoadedLevel && !LevelStreaming->bHasUnloadRequestPending )
			{
				if( GWorld->Levels.FindItemIndex( LevelStreaming->LoadedLevel ) != INDEX_NONE )
				{
					if( LevelStreaming->LoadedLevel->bHasVisibilityRequestPending )
					{
						StreamingLevels.Set( LevelStreaming->PackageName, LEVEL_MakingVisible );
					}
					else
					{
						StreamingLevels.Set( LevelStreaming->PackageName, LEVEL_Visible );
					}
				}
				else
				{
					StreamingLevels.Set( LevelStreaming->PackageName, LEVEL_Loaded );
				}
			}
			else
			{
				// See whether the level's world object is still around.
				UPackage* LevelPackage	= Cast<UPackage>(UGameViewportClient::StaticFindObjectFast( UPackage::StaticClass(), NULL, LevelStreaming->PackageName ));
				UWorld*	  LevelWorld	= NULL;
				if( LevelPackage )
				{
					LevelWorld = Cast<UWorld>(UGameViewportClient::StaticFindObjectFast( UWorld::StaticClass(), LevelPackage, NAME_TheWorld ));
				}

				if( LevelWorld )
				{
					StreamingLevels.Set( LevelStreaming->PackageName, LEVEL_UnloadedButStillAround );
				}
				else
				{
					StreamingLevels.Set( LevelStreaming->PackageName, LEVEL_Unloaded );
				}
			}
		}
	}

	UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
	if (GameEngine != NULL)
	{
		// toss in the levels being loaded by PrepareMapChange
		for( INT LevelIndex=0; LevelIndex < GameEngine->LevelsToLoadForPendingMapChange.Num(); LevelIndex++ )
		{
			const FName LevelName = GameEngine->LevelsToLoadForPendingMapChange(LevelIndex);
			StreamingLevels.Set(LevelName, LEVEL_Preloading);
		}
	}


	ULevel* LevelPlayerIsIn = NULL;

	for( AController* Controller = GWorld->GetWorldInfo()->ControllerList; 
		Controller != NULL; 
		Controller = Controller->NextController
		)
	{
		APlayerController* PC = Cast<APlayerController>( Controller );

		if( ( PC != NULL )
			&&( PC->Pawn != NULL )
			)
		{
			// need to do a trace down here
			//TraceActor = Trace( out_HitLocation, out_HitNormal, TraceDest, TraceStart, false, TraceExtent, HitInfo, true );
			FCheckResult Hit(1.0f);
			DWORD TraceFlags;
			TraceFlags = TRACE_World;

			FVector TraceExtent(0,0,0);

			// this will not work for flying around :-(
			GWorld->SingleLineCheck( Hit, PC->Pawn, (PC->Pawn->Location-FVector(0, 0, 256 )), PC->Pawn->Location, TraceFlags, TraceExtent );

			if( Hit.Level != NULL )
			{
				LevelPlayerIsIn = Hit.Level;
			}
			else if( Hit.Actor != NULL )
			{
				LevelPlayerIsIn = Hit.Actor->GetLevel();
			}
			else if( Hit.Component != NULL )
			{
				LevelPlayerIsIn = Hit.Component->GetOwner()->GetLevel();
			}
		}
	}

	// this no longer seems to be getting the correct level name :-(
	LevelPlayerIsInName = LevelPlayerIsIn != NULL ? LevelPlayerIsIn->GetOutermost()->GetName() : TEXT("None");
}



/**
 *	Render the level stats
 *
 *	@param Viewport	The viewport to render to
 *	@param Canvas	Canvas object to use for rendering
 *	@param X		Suggested X coordinate for where to start drawing
 *	@param Y		Suggested Y coordinate for where to start drawing
 *	@return			Y coordinate of the next line after this output
 */
INT DrawLevelStats( FViewport* Viewport, FCanvas* Canvas, INT X, INT Y )
{
	// Render level stats.
	if( GShowLevelStats )
	{
		TMap<FName,INT> StreamingLevels;	
		FString LevelPlayerIsInName;
		GetLevelStremingStatus( StreamingLevels, LevelPlayerIsInName );

		// now do drawing to the screen

		// Render unloaded levels in red, loaded ones in yellow and visible ones in green. Blue signifies that a level is unloaded but
		// hasn't been garbage collected yet.
		DrawShadowedString(Canvas, X, Y, TEXT("Level streaming"), GEngine->SmallFont, FLinearColor::White );
		Y+=12;

		// now draw the "map" name
		FString MapName	= GWorld->CurrentLevel->GetOutermost()->GetName();

		if( LevelPlayerIsInName == MapName )
		{
			MapName = *FString::Printf( TEXT("->  %s"), *MapName );
		}
		else
		{
			MapName = *FString::Printf( TEXT("    %s"), *MapName );
		}

		DrawShadowedString(Canvas, X, Y, *MapName, GEngine->SmallFont, FColor(127,127,127) );
		Y+=12;

		// now draw the levels
		for( TMap<FName,INT>::TIterator It(StreamingLevels); It; ++It )
		{
			FString	LevelName	= It.Key().ToString();
			INT		Status		= It.Value();
			FColor	Color		= FColor(255,255,255);
			switch( Status )
			{
			case LEVEL_Visible:
				Color = FColor(255,0,0);	// red  loaded and visible
				break;
			case LEVEL_MakingVisible:
				Color = FColor(255,128,0);	// orange, in process of being made visible
				break;
			case LEVEL_Loaded:
				Color = FColor(255,255,0);	// yellow loaded but not visible
				break;
			case LEVEL_UnloadedButStillAround:
				Color = FColor(0,0,255);	// blue  (GC needs to occur to remove this)
				break;
			case LEVEL_Unloaded:
				Color = FColor(0,255,0);	// green
				break;
			case LEVEL_Preloading:
				Color = FColor(255,0,255);	// purple (preloading)
				break;
			default:
				break;
			};

			UPackage* LevelPackage = FindObject<UPackage>( NULL, *LevelName );

			if( LevelPackage 
			&& (LevelPackage->GetLoadTime() > 0) 
			&& (Status != LEVEL_Unloaded) )
			{
				LevelName += FString::Printf(TEXT(" - %4.1f sec"), LevelPackage->GetLoadTime());
			}
			else if( UObject::GetAsyncLoadPercentage( *LevelName ) >= 0 )
			{
				const INT Percentage = appTrunc( UObject::GetAsyncLoadPercentage( *LevelName ) );
				LevelName += FString::Printf(TEXT(" - %3i %%"), Percentage ); 
			}

			if( LevelPlayerIsInName == LevelName )
			{
				LevelName = *FString::Printf( TEXT("->  %s"), *LevelName );
			}
			else
			{
				LevelName = *FString::Printf( TEXT("    %s"), *LevelName );
			}

			DrawShadowedString(Canvas, X + 4, Y, *LevelName, GEngine->SmallFont, Color );
			Y+=12;
		}
	}
	return Y;
}

/**
*	Render Active sound waves
*
*	@param Viewport	The viewport to render to
*	@param Canvas	Canvas object to use for rendering
*	@param X		Suggested X coordinate for where to start drawing
*	@param Y		Suggested Y coordinate for where to start drawing
*	@return			Y coordinate of the next line after this output
*/
INT DrawSoundWaves( FViewport* Viewport, FCanvas* Canvas, INT X, INT Y )
{
	// Render level stats.
	if( GShowSoundWaves )
	{
		DrawShadowedString(Canvas, X, Y, TEXT("Active Sound Waves:"), GEngine->SmallFont, FLinearColor::White );
		Y+=12;

		UAudioDevice* AudioDevice = GEngine->Client->GetAudioDevice();
		if( AudioDevice )
		{
			TArray<FWaveInstance*> WaveInstances;
			INT FirstActiveIndex = AudioDevice->GetSortedActiveWaveInstances( WaveInstances, false );

			for( INT InstanceIndex = FirstActiveIndex; InstanceIndex < WaveInstances.Num(); InstanceIndex++ )
			{
				FWaveInstance* WaveInstance = WaveInstances( InstanceIndex );
				FSoundSource* Source = AudioDevice->WaveInstanceSourceMap.FindRef( WaveInstance );
				AActor* SoundOwner = WaveInstance->AudioComponent ? WaveInstance->AudioComponent->GetOwner() : NULL;

				FString TheString = *FString::Printf(TEXT( "%4i.    %s %6.2f  %s   %s"),
															InstanceIndex,
															Source ? TEXT( "Yes" ) : TEXT( " No" ),
															WaveInstance->Volume,
															*WaveInstance->WaveData->GetPathName(),
															SoundOwner ? *SoundOwner->GetName() : TEXT("None") );

				DrawShadowedString(Canvas, X, Y, *TheString, GEngine->SmallFont, FColor(255,255,255) );
				Y+=12;
			}
			INT ActiveInstances = WaveInstances.Num() - FirstActiveIndex;
			INT R,G,B;
			R=G=B=0;
			INT Max = AudioDevice->MaxChannels/2;
			FLOAT f = Clamp<FLOAT>((FLOAT)(ActiveInstances-Max) / (FLOAT)Max,0.f,1.f);			
			R = appTrunc(f * 255);
			if(ActiveInstances > Max)
			{
				f = Clamp<FLOAT>((FLOAT)(Max-ActiveInstances) / (FLOAT)Max,0.5f,1.f);
			}
			else
			{
				f = 1.0f;
			}
			G = appTrunc(f * 255);
			
			DrawShadowedString(Canvas,X,Y,*FString::Printf(TEXT(" Total: %i"),ActiveInstances),GEngine->SmallFont,FColor(R,G,B));
			Y+=12;
		}
	}
	return Y;
}

/** This will return a vector from a passed in string in form:  (X=8141.9819,Y=7483.3872,Z=2093.4136) **/
FVector APlayerController::GetFVectorFromString( const FString& InStr )
{
	FVector Retval;

	FLOAT Temp = 0.0f;

	if( Parse(*InStr,TEXT("X="), Temp) ) {Retval.X = Temp;}
	if( Parse(*InStr,TEXT("Y="), Temp) ) {Retval.Y   = Temp;}
	if( Parse(*InStr,TEXT("Z="), Temp) ) {Retval.Z  = Temp;}


	return Retval;
}

/** This will return a rotator from a passed in string in form:  (Pitch=100,Yaw=13559,Roll=0) **/
FRotator APlayerController::GetFRotatorFromString( const FString& InStr )
{
	FRotator Retval;

	INT Temp = 0;

	if( Parse(*InStr,TEXT("PITCH="),Temp) ) {Retval.Pitch = Temp;}
	if( Parse(*InStr,TEXT("YAW="),  Temp) ) {Retval.Yaw   = Temp;}
	if( Parse(*InStr,TEXT("ROLL="), Temp) ) {Retval.Roll  = Temp;}

	return Retval;
}


void APlayerController::LogOutBugItGoToLogFile( const FString& InScreenShotDesc, const FString& InGoString, const FString& InLocString )
{
#if ALLOW_DEBUG_FILES
	// Create folder if not already there
	const FString OutputDir = GSys->ScreenShotPath + PATH_SEPARATOR;
	GFileManager->MakeDirectory( *OutputDir );
	// Create archive for log data.
	// we have to +1 on the GSceenShotBitmapIndex as it will be incremented by the bugitscreenshot which is processed next tick

	const FString DescPlusExtension = FString::Printf( TEXT("%s%i.txt"), *InScreenShotDesc, GSceenShotBitmapIndex );
	const FString TxtFileName = CreateProfileFilename( DescPlusExtension, FALSE );

	//FString::Printf( TEXT("BugIt%i-%s%05i"), GBuiltFromChangeList, *InScreenShotDesc, GSceenShotBitmapIndex+1 ) + TEXT( ".txt" );
	const FString FullFileName = OutputDir + TxtFileName;

	FOutputDeviceFile OutputFile(*FullFileName);
	//FArchive* OutputFile = GFileManager->CreateDebugFileWriter( *(FullFileName), FILEWRITE_Append );


	OutputFile.Logf( TEXT("Dumping BugIt data chart at %s using build %i built from changelist %i"), *appSystemTimeString(), GEngineVersion, GetChangeListNumberForPerfTesting() );

	extern const FString GetMapNameStatic();
	const FString MapNameStr = GetMapNameStatic();

	OutputFile.Logf( TEXT("MapName: %s"), *MapNameStr );

	OutputFile.Logf( TEXT("Description: %s"), *InScreenShotDesc );
	OutputFile.Logf( TEXT("%s"), *InGoString );
	OutputFile.Logf( TEXT("%s"), *InLocString );

	OutputFile.Logf( TEXT(" ---=== GameSpecificData ===--- ") );
	// can add some other more detailed info here
	GEngine->Exec( TEXT( "GAMESPECIFIC_BUGIT" ), OutputFile );

	// Flush, close and delete.
	//delete OutputFile;
	OutputFile.TearDown();

	// so here we want to send this bad boy back to the PC
	SendDataToPCViaUnrealConsole( TEXT("UE_PROFILER!BUGIT:"), *(FullFileName) );


	

#endif // ALLOW_DEBUG_FILES
}

void APlayerController::LogOutBugItAIGoToLogFile( const FString& InScreenShotDesc, const FString& InGoString, const FString& InLocString )
{
#if ALLOW_DEBUG_FILES
	extern const FString GetMapNameStatic();
	const FString MapNameStr = GetMapNameStatic();
	const FString PlatformStr = FString(
#if PS3
		TEXT("PS3")
#elif XBOX
		TEXT("Xe")
#else
		TEXT("PC")
#endif // PS3
		);

	FString FinalDir = FString::Printf( TEXT("%s-%s-%i"), *MapNameStr, *PlatformStr, GetChangeListNumberForPerfTesting() );
	FString SubDir = 
	FinalDir = FinalDir.Right(42);
	FinalDir = GSys->ScreenShotPath + PATH_SEPARATOR + FinalDir +  PATH_SEPARATOR + FString::Printf(TEXT("BugItAI-%s%i"),*InScreenShotDesc,GSceenShotBitmapIndex) + PATH_SEPARATOR;
	//debugf(TEXT("%s"),*FinalDir);

	// Create folder if not already there
	GFileManager->MakeDirectory( *FinalDir );


	const FString TxtFileName = FString::Printf( TEXT("BugitAI-%s%i.txt"), *InScreenShotDesc, GSceenShotBitmapIndex );


	TCHAR File[MAX_SPRINTF] = TEXT("");
	for( INT TestBitmapIndex = 0; TestBitmapIndex < 9; ++TestBitmapIndex )
	{ 
		FString BmpFileName = FString::Printf( TEXT("BugitAI-%s%i.bmp"), *InScreenShotDesc, TestBitmapIndex );
		BmpFileName = BmpFileName.Right(42);
		//warnf( TEXT( "BugIt Looking: %s" ), *(FinalDir + BmpFileName) );
		appSprintf( File, TEXT("%s"), *(FinalDir + BmpFileName) );
		if( GFileManager->FileSize(File) == INDEX_NONE )
		{
			GSceenShotBitmapIndex = TestBitmapIndex; // this is safe as the UnMisc.cpp ScreenShot code will test each number before writing a file
			FString SSFull = FinalDir + BmpFileName;
			
			// make relative to SS dir
			SSFull = SSFull.Replace(*(GSys->ScreenShotPath+PATH_SEPARATOR),TEXT(""),TRUE);
			GScreenShotName = SSFull; 
			GScreenShotRequest = TRUE;
			break;
		}
	}
	//FString::Printf( TEXT("BugIt%i-%s%05i"), GBuiltFromChangeList, *InScreenShotDesc, GSceenShotBitmapIndex+1 ) + TEXT( ".txt" );
	const FString FullFileName = FinalDir + TxtFileName;
	
	// Create archive for log data.
	FOutputDeviceFile OutputFile(*FullFileName);
	//FArchive* OutputFile = GFileManager->CreateDebugFileWriter( *(FullFileName), FILEWRITE_Append );


	OutputFile.Logf( TEXT("Dumping BugItAI data chart at %s using build %i built from changelist %i"), *appSystemTimeString(), GEngineVersion, GetChangeListNumberForPerfTesting() );
	OutputFile.Logf( TEXT("MapName: %s"), *MapNameStr );

	OutputFile.Logf( TEXT("Description: %s"), *InScreenShotDesc );
	OutputFile.Logf( TEXT("%s"), *InGoString );
	OutputFile.Logf( TEXT("%s"), *InLocString );

	OutputFile.Logf( TEXT(" ---=== GameSpecificData ===--- ") );
	// can add some other more detailed info here
	GEngine->Exec( *FString::Printf( TEXT("GAMESPECIFIC_BUGITAI %s" ), *FinalDir), OutputFile );

	// Flush, close and delete.
	//delete OutputFile;
	OutputFile.TearDown();

	// so here we want to send this bad boy back to the PC
	SendDataToPCViaUnrealConsole( TEXT("UE_PROFILER!BUGIT:"), *(FullFileName) );




#endif // ALLOW_DEBUG_FILES
}




