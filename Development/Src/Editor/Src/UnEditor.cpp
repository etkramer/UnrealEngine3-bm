/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "Factories.h"
#include "BSPOps.h"

// needed for the RemotePropagator
#include "UnIpDrv.h"
#include "EnginePrefabClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"
#include "EngineDecalClasses.h"
#include "EngineAudioDeviceClasses.h"
#include "EngineSoundClasses.h"

#include "UnConsoleSupportContainer.h"
#include "UnAudioCompress.h"
#include "Database.h"

// For WAVEFORMATEXTENSIBLE
#pragma pack(push,8)
#include <mmreg.h>
#pragma pack(pop)

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

UEditorEngine*	GEditor;

static inline USelection*& PrivateGetSelectedActors()
{
	static USelection* SSelectedActors = NULL;
	return SSelectedActors;
};

static inline USelection*& PrivateGetSelectedObjects()
{
	static USelection* SSelectedObjects = NULL;
	return SSelectedObjects;
};

static void PrivateInitSelectedSets()
{
	PrivateGetSelectedActors() = new( UObject::GetTransientPackage(), TEXT("SelectedActors"), RF_Transactional ) USelection;
	PrivateGetSelectedActors()->AddToRoot();

	PrivateGetSelectedObjects() = new( UObject::GetTransientPackage(), TEXT("SelectedObjects"), RF_Transactional ) USelection;
	PrivateGetSelectedObjects()->AddToRoot();
}

static void PrivateDestroySelectedSets()
{
#if 0
	PrivateGetSelectedActors()->RemoveFromRoot();
	PrivateGetSelectedActors() = NULL;
	PrivateGetSelectedObjects()->RemoveFromRoot();
	PrivateGetSelectedObjects() = NULL;
#endif
}

/*-----------------------------------------------------------------------------
	UEditorEngine.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(UEditorEngine);
IMPLEMENT_CLASS(ULightingChannelsObject);
IMPLEMENT_CLASS(UEditorUserSettings);

/**
 * Returns the number of currently selected actors.
 *
 */
UBOOL UEditorEngine::GetSelectedActorCount() const
{
	int NumSelectedActors = 0;
	for(FSelectionIterator It(GetSelectedActorIterator()); It; ++It)
	{
		++NumSelectedActors;
	}

	return NumSelectedActors;
}

/**
 * Returns the set of selected actors.
 */
USelection* UEditorEngine::GetSelectedActors() const
{
	return PrivateGetSelectedActors();
}

/**
 * Returns an FSelectionIterator that iterates over the set of selected actors.
 */
FSelectionIterator UEditorEngine::GetSelectedActorIterator() const
{
	return FSelectionIterator( *GetSelectedActors() );
};

/**
 * Returns the set of selected non-actor objects.
 */
USelection* UEditorEngine::GetSelectedObjects() const
{
	return PrivateGetSelectedObjects();
}

/**
 * Returns the appropriate selection set for the specified object class.
 */
USelection* UEditorEngine::GetSelectedSet( const UClass* Class ) const
{
	if ( Class->IsChildOf( AActor::StaticClass() ) )
	{
		return GetSelectedActors();
	}
	else
	{
		return GetSelectedObjects();
	}
}

/*-----------------------------------------------------------------------------
	Init & Exit.
-----------------------------------------------------------------------------*/

//
// Construct the UEditorEngine class.
//
void UEditorEngine::StaticConstructor()
{
}

//
// Editor early startup.
//
void UEditorEngine::InitEditor()
{
	// Call base.
	UEngine::Init();

	// Create selection sets.
	if( !GIsUCCMake )
	{
		PrivateInitSelectedSets();
	}

	// Make sure properties match up.
	VERIFY_CLASS_OFFSET(A,Actor,Owner);

	// Allocate temporary model.
	TempModel = new UModel( NULL, 1 );

	// Settings.
	FBSPOps::GFastRebuild	= 0;
	Bootstrapping			= 0;

	// Setup up particle count clamping values...
	if (GEngine)
	{
		GEngine->MaxParticleSpriteCount = GEngine->MaxParticleVertexMemory / (4 * sizeof(FParticleSpriteVertex));
		GEngine->MaxParticleSubUVCount = GEngine->MaxParticleVertexMemory / (4 * sizeof(FParticleSpriteSubUVVertex));
	}
	else
	{
		warnf(NAME_Warning, TEXT("Failed to set GEngine particle counts!"));
	}
}

UBOOL UEditorEngine::ShouldDrawBrushWireframe( AActor* InActor )
{
	UBOOL bResult = TRUE;

	if(GEditorModeTools().GetCurrentMode())
	{
		bResult = GEditorModeTools().GetCurrentMode()->ShouldDrawBrushWireframe( InActor );
	}

	return bResult;
}

// Used for sorting ActorFactory classes.
IMPLEMENT_COMPARE_POINTER( UActorFactory, UnEditor, { return B->MenuPriority - A->MenuPriority; } )

//
// Init the editor.
//
void UEditorEngine::Init()
{
	check(!HasAnyFlags(RF_ClassDefaultObject));

	// Register for 'packaged dirtied' events.  We'll need these for some PIE stuff.  We also need to listen
	// for the 'Map Change' event so we can purge PIE data when needed.
	GCallbackEvent->Register( CALLBACK_MapChange, this );
	GCallbackEvent->Register( CALLBACK_PreEngineShutdown, this );

	// Init editor.
	GEditor = this;
	InitEditor();

	// Init transactioning.
	Trans = CreateTrans();

	// Load classes for editing.
	BeginLoad();
	for( INT i=0; i<EditPackages.Num(); i++ )
	{
		if( !LoadPackage( NULL, *EditPackages(i), LOAD_NoWarn ) )
		{
			debugf( LocalizeSecure(LocalizeUnrealEd("Error_CantFindEditPackage"), *EditPackages(i)) );
		}
	}

	// Automatically load mod classes unless -nomodautoload is specified.
	if ( !ParseParam(appCmdLine(), TEXT("nomodautoload")) )
	{
		TArray<FString> ModScriptPackageNames;
		TMultiMap<FString,FString>* ModSec = GConfig->GetSectionPrivate( TEXT("ModPackages"), 0, 1, GEditorIni );
		ModSec->MultiFind( FString(TEXT("ModPackages")), ModScriptPackageNames );

		for( INT PackageIndex=0; PackageIndex<ModScriptPackageNames.Num(); ++PackageIndex )
		{
			if( !LoadPackage( NULL, *ModScriptPackageNames(PackageIndex), LOAD_NoWarn ) )
			{
				debugf( LocalizeSecure(LocalizeUnrealEd("Error_CantFindEditPackage"), *ModScriptPackageNames(PackageIndex)) );
			}
		}
	}
	EndLoad();

	// Init the client.
	UClass* ClientClass = StaticLoadClass( UClient::StaticClass(), NULL, TEXT("engine-ini:Editor.EditorEngine.Client"), NULL, LOAD_None, NULL );
	Client = (UClient*)StaticConstructObject( ClientClass );
	Client->Init( this );
	check(Client);

	// Objects.
	Results  = new( GetTransientPackage(), TEXT("Results") )UTextBuffer;

	// Create array of ActorFactory instances.
	for(TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Cls = *It;
		UObject* DefaultObject = Cls->GetDefaultObject();

		if(  Cls->IsChildOf(UActorFactory::StaticClass()) &&
		   !(Cls->ClassFlags & CLASS_Abstract) &&
		    (DefaultObject != NULL && ((UActorFactory*)DefaultObject)->bPlaceable) &&
			(appStricmp( *((UActorFactory*)DefaultObject)->SpecificGameName, TEXT("") ) == 0 ||
			 appStricmp( *((UActorFactory*)DefaultObject)->SpecificGameName, appGetGameName() ) == 0) )
		{
			UActorFactory* NewFactory = ConstructObject<UActorFactory>( Cls );
			ActorFactories.AddItem(NewFactory);
		}
	}

	// Sort by menu priority.
	Sort<USE_COMPARE_POINTER(UActorFactory,UnEditor)>( &ActorFactories(0), ActorFactories.Num() );

	// Purge garbage.
	Cleanse( FALSE, 0, TEXT("startup") );

	// Subsystem init messsage.
	debugf( NAME_Init, TEXT("Editor engine initialized") );

	// create the possible propagators
	InEditorPropagator = new FEdObjectPropagator;
#if WITH_UE3_NETWORKING
	RemotePropagator = new FRemotePropagator;
#endif	//#if WITH_UE3_NETWORKING
};

/**
 * Constructs a default cube builder brush, this function MUST be called at the AFTER UEditorEngine::Init in order to guarantee builder brush and other required subsystems exist.
 */
void UEditorEngine::InitBuilderBrush()
{
	const UBOOL bOldDirtyState = GWorld->CurrentLevel->GetOutermost()->IsDirty();

	// For additive geometry mode, make the builder brush a small 256x256x256 cube so its visible.
	const INT CubeSize = 256;
	UCubeBuilder* CubeBuilder = ConstructObject<UCubeBuilder>( UCubeBuilder::StaticClass() );
	CubeBuilder->X = CubeSize;
	CubeBuilder->Y = CubeSize;
	CubeBuilder->Z = CubeSize;
	CubeBuilder->eventBuild();

	// Restore the level's dirty state, so that setting a builder brush won't mark the map as dirty.
	GWorld->CurrentLevel->MarkPackageDirty( bOldDirtyState );
}

void UEditorEngine::FinishDestroy()
{
	if ( !HasAnyFlags(RF_ClassDefaultObject) )
	{
		// this needs to be already cleaned up
		check(PlayWorld == NULL);

		// Unregister events
		GCallbackEvent->Unregister( CALLBACK_MapChange, this );
		GCallbackEvent->Unregister( CALLBACK_PreEngineShutdown, this );


		// free the propagators
		delete InEditorPropagator;
		delete RemotePropagator;

		// GWorld == NULL if we're compiling script.
		if( !GIsUCCMake )
		{
			ClearComponents();
			GWorld->CleanupWorld();
		}

		// Shut down transaction tracking system.
		if( Trans )
		{
			if( GUndo )
			{
				debugf( NAME_Warning, TEXT("Warning: A transaction is active") );
			}
			ResetTransaction( TEXT("shutdown") );
		}

		// Destroy selection sets.
		PrivateDestroySelectedSets();

		// Remove editor array from root.
		debugf( NAME_Exit, TEXT("Editor shut down") );
	}

	Super::FinishDestroy();
}
void UEditorEngine::Serialize( FArchive& Ar )
{
	Super::Serialize(Ar);
	if(!Ar.IsLoading() && !Ar.IsSaving())
	{
		// Serialize viewport clients.

		for(UINT ViewportIndex = 0;ViewportIndex < (UINT)ViewportClients.Num();ViewportIndex++)
			Ar << *ViewportClients(ViewportIndex);

		// Serialize ActorFactories
		Ar << ActorFactories;

		// Serialize components used in UnrealEd modes

		GEditorModeTools().Serialize( Ar );
	}
}

/*-----------------------------------------------------------------------------
	Tick.
-----------------------------------------------------------------------------*/

//
// Time passes...
//
void UEditorEngine::Tick( FLOAT DeltaSeconds )
{
	check( GWorld );
	check( GWorld != PlayWorld );

	// Tick client code.
	if( Client )
	{
		Client->Tick(DeltaSeconds);
	}

	// Clean up the game viewports that have been closed.
	CleanupGameViewport();

	// If all viewports closed, close the current play level.
	if( GameViewport == NULL && PlayWorld )
	{
		EndPlayMap();
	}

	// Update subsystems.
	{
		// This assumes that UObject::StaticTick only calls ProcessAsyncLoading.	
		UObject::StaticTick( DeltaSeconds );
	}

	// Look for realtime flags.
	UBOOL IsRealtime = FALSE;
	UBOOL IsAudioRealTime = FALSE;
	for( INT i = 0; i < ViewportClients.Num(); i++ )
	{
		FEditorLevelViewportClient* ViewportClient = ViewportClients( i );
		if( ViewportClient->GetScene() == GWorld->Scene )
		{
			if( ViewportClient->IsRealtime() )
			{
				IsRealtime = TRUE;
				IsAudioRealTime = TRUE;
			}
		}

		if( ViewportClient->bAudioRealTime )
		{
			IsAudioRealTime = TRUE;
		}
	}

	// Tick level.
	GWorld->Tick( IsRealtime ? LEVELTICK_ViewportsOnly : LEVELTICK_TimeOnly, DeltaSeconds );

	// Perform editor level streaming previs if no PIE session is currently in progress.
	if( !PlayWorld )
	{
		FEditorLevelViewportClient* PerspectiveViewportClient = NULL;
		for ( INT i = 0 ; i < ViewportClients.Num() ; ++i )
		{
			FEditorLevelViewportClient* ViewportClient = ViewportClients(i);

			// Previs level streaming volumes in the Editor.
			if ( ViewportClient->bLevelStreamingVolumePrevis )
			{
				UBOOL bProcessViewer = FALSE;
				const FVector& ViewLocation = ViewportClient->ViewLocation;

				// Iterate over streaming levels and compute whether the ViewLocation is in their associated volumes.
				TMap<ALevelStreamingVolume*, UBOOL> VolumeMap;

				AWorldInfo*	WorldInfo = GWorld->GetWorldInfo();
				for( INT LevelIndex = 0 ; LevelIndex < WorldInfo->StreamingLevels.Num() ; ++LevelIndex )
				{
					ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
					if( StreamingLevel )
					{
						// Assume the streaming level is invisible until we find otherwise.
						UBOOL bStreamingLevelShouldBeVisible = FALSE;

						// We're not going to change level visibility unless we encounter at least one
						// volume associated with the level.
						UBOOL bFoundValidVolume = FALSE;

						// For each streaming volume associated with this level . . .
						for ( INT i = 0 ; i < StreamingLevel->EditorStreamingVolumes.Num() ; ++i )
						{
							ALevelStreamingVolume* StreamingVolume = StreamingLevel->EditorStreamingVolumes(i);
							if ( StreamingVolume && !StreamingVolume->bDisabled )
							{
								bFoundValidVolume = TRUE;

								UBOOL bViewpointInVolume;
								UBOOL* bResult = VolumeMap.Find(StreamingVolume);
								if ( bResult )
								{
									// This volume has already been considered for another level.
									bViewpointInVolume = *bResult;
								}
								else
								{
									// Compute whether the viewpoint is inside the volume and cache the result.
									bViewpointInVolume = StreamingVolume->Encompasses( ViewLocation );
									VolumeMap.Set( StreamingVolume, bViewpointInVolume );
								}

								// Halt when we find a volume associated with the level that the viewpoint is in.
								if ( bViewpointInVolume )
								{
									bStreamingLevelShouldBeVisible = TRUE;
									break;
								}
							}
						}

						// Set the streaming level visibility status if we encountered at least one volume.
						if ( bFoundValidVolume && StreamingLevel->bShouldBeVisibleInEditor != bStreamingLevelShouldBeVisible )
						{
							StreamingLevel->bShouldBeVisibleInEditor = bStreamingLevelShouldBeVisible;
							bProcessViewer = TRUE;
						}
					}
				}

				// Call UpdateLevelStreaming if the visibility of any streaming levels was modified.
				if ( bProcessViewer )
				{
					GWorld->UpdateLevelStreaming();
					GCallbackEvent->Send( CALLBACK_RefreshEditor_LevelBrowser );
				}
				break;
			}
		}
	}

	// kick off a "Play From Here" if we got one
	if (bIsPlayWorldQueued)
	{
		StartQueuedPlayMapRequest();
	}

	// if we have the side-by-side world for "Play From Here", tick it!
	if(PlayWorld)
	{
		// Use the PlayWorld as the GWorld, because who knows what will happen in the Tick.
		UWorld* OldGWorld = SetPlayInEditorWorld( PlayWorld );

		// Release mouse if the game is paused. The low level input code might ignore the request when e.g. in fullscreen mode.
		if ( GameViewport != NULL && GameViewport->Viewport != NULL )
		{
			// Decide whether to drop high detail because of frame rate
			GameViewport->SetDropDetail(DeltaSeconds);
		}

		// Update the level.
		GameCycles=0;
		CLOCK_CYCLES(GameCycles);

		{
			// So that hierarchical stats work in PIE
			SCOPE_CYCLE_COUNTER(STAT_FrameTime);
			// tick the level
			PlayWorld->Tick( LEVELTICK_All, DeltaSeconds );
		}

		UNCLOCK_CYCLES(GameCycles);

		// Tick the viewports.
		if ( GameViewport != NULL )
		{
			GameViewport->Tick(DeltaSeconds);
		}

//#if defined(WITH_FACEFX) && defined(FX_TRACK_MEMORY_STATS)
#if OLD_STATS
		if( GFaceFXStats.Enabled )
		{
			GFaceFXStats.NumCurrentAllocations.Value  += OC3Ent::Face::FxGetCurrentNumAllocations();
			GFaceFXStats.CurrentAllocationsSize.Value += OC3Ent::Face::FxGetCurrentBytesAllocated() / 1024.0f;
			GFaceFXStats.PeakAllocationsSize.Value    += OC3Ent::Face::FxGetMaxBytesAllocated() / 1024.0f;
		}
#endif // WITH_FACEFX && FX_TRACK_MEMORY_STATS

		// Pop the world
		RestoreEditorWorld( OldGWorld );
	}

	// Clean up any game viewports that may have been closed during the level tick (eg by Kismet).
	CleanupGameViewport();

	// If all viewports closed, close the current play level.
	if( GameViewport == NULL && PlayWorld )
	{
		EndPlayMap();
	}

	// Handle decal update requests.
	if ( bDecalUpdateRequested )
	{
		bDecalUpdateRequested = FALSE;
		for( FActorIterator It; It; ++It )
		{
			ADecalActorBase* DecalActor = Cast<ADecalActorBase>( *It );
			if ( DecalActor && DecalActor->Decal )
			{
				FComponentReattachContext ReattachContext( DecalActor->Decal );
			}
		}
	}

	// Update viewports.
	for(INT ViewportIndex = 0;ViewportIndex < ViewportClients.Num();ViewportIndex++)
	{
		ViewportClients(ViewportIndex)->Tick(DeltaSeconds);
	}

	// Commit changes to the BSP model.
	GWorld->CommitModelSurfaces();

	/////////////////////////////
	// Redraw viewports.

	// Render view parents, then view children.
	for(UBOOL bRenderingChildren = 0;bRenderingChildren < 2;bRenderingChildren++)
	{
		for(INT ViewportIndex=0; ViewportIndex<ViewportClients.Num(); ViewportIndex++ )
		{
			FEditorLevelViewportClient* ViewportClient = ViewportClients(ViewportIndex);
			const UBOOL bIsViewParent = ViewportClient->ViewState->IsViewParent();
			if(	(bRenderingChildren && !bIsViewParent) ||
				(!bRenderingChildren && bIsViewParent))
			{
				// Add view information for perspective viewports.
				if( ViewportClient->ViewportType == LVT_Perspective )
				{
					GStreamingManager->AddViewInformation( ViewportClient->ViewLocation, ViewportClient->Viewport->GetSizeX(), ViewportClient->Viewport->GetSizeX() / appTan(ViewportClient->ViewFOV) );
				}
				// Redraw the viewport if it's realtime.
				if( ViewportClient->IsRealtime() )
				{
					ViewportClient->Viewport->Draw();
				}
				// Redraw the viewport if there are pending redraw.
				if(ViewportClient->NumPendingRedraws > 0)
				{
					ViewportClient->Viewport->Draw();
					ViewportClient->NumPendingRedraws--;
				}
			}
		}
	}

	// Render playworld. This needs to happen after the other viewports for screenshots to work correctly in PIE.
	if(PlayWorld)
	{
		// Use the PlayWorld as the GWorld, because who knows what will happen in the Tick.
		UWorld* OldGWorld = SetPlayInEditorWorld( PlayWorld );
		
		// Render everything.
		if ( GameViewport != NULL )
		{
			GameViewport->eventLayoutPlayers();
			check(GameViewport->Viewport);
			GameViewport->Viewport->Draw();
		}

		// Pop the world
		RestoreEditorWorld( OldGWorld );
	}

	// Update resource streaming after both regular Editor viewports and PIE had a chance to add viewers.
	GStreamingManager->UpdateResourceStreaming( DeltaSeconds );

	// Update Audio. This needs to occur after rendering as the rendering code updates the listener position.
	if( Client && Client->GetAudioDevice() )
	{
		UWorld* OldGWorld = NULL;
		if( PlayWorld )
		{
			// Use the PlayWorld as the GWorld if we're using PIE.
			OldGWorld = SetPlayInEditorWorld( PlayWorld );
		}

		// Update audio device.
		Client->GetAudioDevice()->Update( IsAudioRealTime || ( PlayWorld && !PlayWorld->IsPaused() ) );

		if( PlayWorld )
		{
			// Pop the world.
			RestoreEditorWorld( OldGWorld );
		}
	}

	if( GScriptCallGraph )
	{
		GScriptCallGraph->Tick(DeltaSeconds);
	}

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

	// Track Editor startup time.
	static UBOOL bIsFirstTick = TRUE;
	if( bIsFirstTick )
	{
		GTaskPerfTracker->AddTask( TEXT("Editor startup"), TEXT(""), appSeconds() - GStartTime );
		bIsFirstTick = FALSE;
	}
}

/**
* Callback for when a editor property changed.
*
* @param	PropertyThatChanged	Property that changed and initiated the callback.
*/
void UEditorEngine::PostEditChange(UProperty* PropertyThatChanged)
{
	if( PropertyThatChanged )
	{
		// Check to see if the FOVAngle property changed, if so loop through all viewports and update their fovs.
		const UBOOL bFOVAngleChanged = (appStricmp( *PropertyThatChanged->GetName(), TEXT("FOVAngle") ) == 0) ? TRUE : FALSE;
		
		if( bFOVAngleChanged )
		{
			// Clamp the FOV value to valid ranges.
			GEditor->FOVAngle = Clamp<FLOAT>( GEditor->FOVAngle, 1.0f, 179.0f );

			// Loop through all viewports and update their FOV angles and invalidate them, forcing them to redraw.
			for( INT ViewportIndex = 0 ; ViewportIndex < ViewportClients.Num() ; ++ViewportIndex )
			{
				if (ViewportClients(ViewportIndex) && ViewportClients(ViewportIndex)->Viewport)
				{
					ViewportClients( ViewportIndex )->ViewFOV = GEditor->FOVAngle;
					ViewportClients( ViewportIndex )->Invalidate();
				}
			}
		}
	}

	// Propagate the callback up to the superclass.
	Super::PostEditChange(PropertyThatChanged);
}

FViewport* UEditorEngine::GetAViewport()
{
	if(GameViewport && GameViewport->Viewport)
	{
		return GameViewport->Viewport;
	}

	for(INT i=0; i<ViewportClients.Num(); i++)
	{
		if(ViewportClients(i) && ViewportClients(i)->Viewport)
		{
			return ViewportClients(i)->Viewport;
		}
	}

	return NULL;
}

/*-----------------------------------------------------------------------------
	Cleanup.
-----------------------------------------------------------------------------*/

/**
 * Cleans up after major events like e.g. map changes.
 *
 * @param	ClearSelection	Whether to clear selection
 * @param	Redraw			Whether to redraw viewports
 * @param	TransReset		Human readable reason for resetting the transaction system
 */
void UEditorEngine::Cleanse( UBOOL ClearSelection, UBOOL Redraw, const TCHAR* TransReset )
{
	check(TransReset);
	if( GIsRunning && !Bootstrapping )
	{
		if( ClearSelection )
		{
			// Clear selection sets.
			GetSelectedActors()->DeselectAll();
			GetSelectedObjects()->DeselectAll();
		}

		// Reset the transaction tracking system.
		ResetTransaction( TransReset );

		// Redraw the levels.
		if( Redraw )
		{
			RedrawLevelEditingViewports();
		}

		// Collect garbage.
		CollectGarbage( GARBAGE_COLLECTION_KEEPFLAGS );
	}
}

/*---------------------------------------------------------------------------------------
	Components.
---------------------------------------------------------------------------------------*/

void UEditorEngine::ClearComponents()
{
	FEdMode* CurrentMode = GEditorModeTools().GetCurrentMode();
	if(CurrentMode)
	{
		CurrentMode->ClearComponent();
	}
	GWorld->ClearComponents();
}

void UEditorEngine::UpdateComponents()
{
	FEdMode* CurrentMode = GEditorModeTools().GetCurrentMode();
	if(CurrentMode)
	{
		CurrentMode->UpdateComponent();
	}
	GWorld->UpdateComponents( FALSE );
}

/** 
 * Returns an audio component linked to the current scene that it is safe to play a sound on
 *
 * @param	SoundCue	A sound cue to attach to the audio component
 * @param	SoundNode	A sound node that is attached to the audio component when the sound cue is NULL
 */
UAudioComponent* UEditorEngine::GetPreviewAudioComponent( USoundCue* SoundCue, USoundNode* SoundNode )
{
	if( Client && Client->GetAudioDevice() )
	{
		if( PreviewAudioComponent == NULL )
		{
			PreviewSoundCue = ConstructObject<USoundCue>( USoundCue::StaticClass() );
			PreviewAudioComponent = Client->GetAudioDevice()->CreateComponent( PreviewSoundCue, GWorld->Scene, NULL, FALSE );
		}

		check( PreviewAudioComponent );
		// Mark as a preview component so the distance calculations can be ignored
		PreviewAudioComponent->bPreviewComponent = TRUE;

		if( SoundNode != NULL )
		{
			PreviewSoundCue->FirstNode = SoundNode;
			PreviewAudioComponent->SoundCue = PreviewSoundCue;
		}
		else
		{
			PreviewAudioComponent->SoundCue = SoundCue;
		}
	}

	return( PreviewAudioComponent );
}

/** 
 * Stop any sounds playing on the preview audio component and allowed it to be garbage collected
 */
void UEditorEngine::ClearPreviewAudioComponents( void )
{
	if( PreviewAudioComponent )
	{
		PreviewAudioComponent->Stop();

		// Just null out so they get GC'd
		PreviewSoundCue->FirstNode = NULL;
		PreviewSoundCue = NULL;
		PreviewAudioComponent->SoundCue = NULL;
		PreviewAudioComponent = NULL;
	}
}

/*---------------------------------------------------------------------------------------
	Misc.
---------------------------------------------------------------------------------------*/

/**
 * Issued by code reuqesting that decals be reattached.
 */
void UEditorEngine::IssueDecalUpdateRequest()
{
	bDecalUpdateRequested = TRUE;
}

void UEditorEngine::SetObjectPropagationDestination(INT Destination)
{
	// Layout of the ComboBox:
	//   No Propagation
	//   Local Standalone (a game running, but not In Editor)
	//   Console 0
	//   Console 1
	//   ...

	// remember out selection for when we restart the editor
	GConfig->SetInt(TEXT("ObjectPropagation"), TEXT("Destination"), Destination, GEditorIni);

	// first one is no propagation
	if (Destination == OPD_None)
	{
		FObjectPropagator::ClearPropagator();
	}
	else
	{
		// the rest are network propagations
		FObjectPropagator::SetPropagator(RemotePropagator);
		GObjectPropagator->ClearTargets();

		// the first one of these is a local standalone game (so we use 127.0.0.1)
		if (Destination == OPD_LocalStandalone)
		{
			GObjectPropagator->AddTarget(INVALID_TARGETHANDLE, 0x7F000001, TRUE); // 127.0.0.1
		}
		else
		{
			FConsoleSupport *CurPlatform = FConsoleSupportContainer::GetConsoleSupportContainer()->GetConsoleSupport(Destination - OPD_ConsoleStart);

			INT NumTargets = CurPlatform->GetNumTargets();
			TArray<TARGETHANDLE> SelectedMenuItems(NumTargets);

			CurPlatform->GetMenuSelectedTargets(&SelectedMenuItems(0), NumTargets);

			for(INT CurTargetIndex = 0; CurTargetIndex < NumTargets; ++CurTargetIndex)
			{
				GObjectPropagator->AddTarget(SelectedMenuItems(CurTargetIndex), htonl(CurPlatform->GetIPAddress(SelectedMenuItems(CurTargetIndex))), CurPlatform->GetIntelByteOrder());
			}
		}
	}
}

/**
 *	Returns pointer to a temporary 256x256 render target.
 *	If it has not already been created, does so here.
 */
UTextureRenderTarget2D* UEditorEngine::GetScratchRenderTarget()
{
	if(!ScratchRenderTarget)
	{
		UTextureRenderTargetFactoryNew* NewFactory = CastChecked<UTextureRenderTargetFactoryNew>( StaticConstructObject(UTextureRenderTargetFactoryNew::StaticClass()) );
		NewFactory->Width = 256;
		NewFactory->Height = 256;

		UObject* NewObj = NewFactory->FactoryCreateNew( UTextureRenderTarget2D::StaticClass(), UObject::GetTransientPackage(), NAME_None, RF_Transient, NULL, GWarn );
		ScratchRenderTarget = CastChecked<UTextureRenderTarget2D>(NewObj);
	}
	check(ScratchRenderTarget);
	return ScratchRenderTarget;
}

/**
 * Check for any PrefabInstances which are out of date.  For any PrefabInstances which have a TemplateVersion less than its PrefabTemplate's
 * PrefabVersion, propagates the changes to the source Prefab to the PrefabInstance.
 */
void UEditorEngine::UpdatePrefabs()
{
	TArray<UPrefab*> UpdatedPrefabs;
	TArray<APrefabInstance*> RemovedPrefabs;
	UBOOL bResetInvalidPrefab = FALSE;

	USelection* SelectedActors = GetSelectedActors();
	for( FActorIterator It; It; ++It )
	{
		APrefabInstance* PrefabInst = Cast<APrefabInstance>(*It);

		// If this is a valid PrefabInstance
		if(	PrefabInst && !PrefabInst->bDeleteMe && !PrefabInst->IsPendingKill() )
		{
			// first, verify that this PrefabInstance is still bound to a valid prefab
			UPrefab* SourcePrefab = PrefabInst->TemplatePrefab;
			if ( SourcePrefab != NULL )
			{
				// first, verify that all archetypes in the prefab's ArchetypeToInstanceMap exist.
				if ( !PrefabInst->VerifyMemberArchetypes() )
				{
					bResetInvalidPrefab = TRUE;
				}

				// If the PrefabInstance's version number is less than the source Prefab's version (ie there is 
				// a newer version of the Prefab), update it now.
				if ( PrefabInst->TemplateVersion < SourcePrefab->PrefabVersion )
				{
					PrefabInst->UpdatePrefabInstance( GetSelectedActors() );

					// Mark the level package as dirty, so we are prompted to save the map.
					PrefabInst->MarkPackageDirty();

					// Add prefab to list of ones that we needed to update instances of.
					UpdatedPrefabs.AddUniqueItem(SourcePrefab);
				}
				else if ( PrefabInst->TemplateVersion > SourcePrefab->PrefabVersion )
				{
					bResetInvalidPrefab = TRUE;
					warnf(NAME_Warning, TEXT("PrefabInstance '%s' has a version number that is higher than the source Prefab's version.  Resetting existing PrefabInstance from source: '%s'"), *PrefabInst->GetPathName(), *SourcePrefab->GetPathName());

					// this PrefabInstance's version number is higher than the source Prefab's version number,
					// this is normally the result of updating a prefab, then saving the map but not the package containing
					// the prefab.  If this has occurred, we'll need to replace the existing PrefabInstance with a new copy 
					// of the older version of the Prefab, but we must warn the user when we do this!
					PrefabInst->DestroyPrefab(SelectedActors);
					PrefabInst->InstancePrefab(SourcePrefab);
				}
			}
			else
			{
				// if the PrefabInstance's TemplatePrefab is NULL, it probably means that the user created a prefab,
				// then reloaded the map containing the prefab instance without saving the package containing the prefab.
				PrefabInst->DestroyPrefab(SelectedActors);
				GWorld->DestroyActor(PrefabInst);
				SelectedActors->Deselect(PrefabInst);

				RemovedPrefabs.AddItem(PrefabInst);
			}
		}
	}

	// If we updated some prefab instances, display it in a dialog.
	if(UpdatedPrefabs.Num() > 0)
	{
		FString UpdatedPrefabList;
		for(INT i=0; i<UpdatedPrefabs.Num(); i++)
		{
			// Add name to list of updated Prefabs.
			UpdatedPrefabList += FString::Printf( TEXT("%s\n"), *(UpdatedPrefabs(i)->GetPathName()) );
		}

		appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("Prefab_OldPrefabInstancesUpdated"), *UpdatedPrefabList) );
	}

	if ( RemovedPrefabs.Num() )
	{
		FString RemovedPrefabList;
		for(INT i=0; i<RemovedPrefabs.Num(); i++)
		{
			// Add name to list of updated Prefabs.
			RemovedPrefabList += FString::Printf( TEXT("%s") LINE_TERMINATOR, *RemovedPrefabs(i)->GetPathName());
		}

		appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("Prefab_MissingSourcePrefabs"), *RemovedPrefabList) );
	}

	if ( bResetInvalidPrefab )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Prefab_ResetInvalidPrefab") );
	}
}


/** 
 *	Create a new instance of a prefab in the level. 
 *
 *	@param	Prefab		The prefab to create an instance of.
 *	@param	Location	Location to create the new prefab at.
 *	@param	Rotation	Rotation to create the new prefab at.
 *	@return				Pointer to new PrefabInstance actor in the level, or NULL if it fails.
 */
APrefabInstance* UEditorEngine::Prefab_InstancePrefab(UPrefab* Prefab, const FVector& Location, const FRotator& Rotation) const
{
	// First, create a new PrefabInstance actor.
	APrefabInstance* NewInstance = CastChecked<APrefabInstance>( GWorld->SpawnActor(APrefabInstance::StaticClass(), NAME_None, Location, Rotation) );
	if(NewInstance)
	{
		NewInstance->InstancePrefab(Prefab);
		return NewInstance;
	}
	else
	{
		return NULL;
	}
}

/** Util that looks for and fixes any incorrect ParentSequence pointers in Kismet objects in memory. */
void UEditorEngine::FixKismetParentSequences()
{
	TArray<FString> ParentSequenceFixed;

	for( TObjectIterator<USequenceObject> It; It; ++It )
	{
		USequenceObject* SeqObj = *It;

		// if this sequence object is a subobject template, skip it as it won't be initialized (no ParentSequence, etc.)
		if ( SeqObj->IsTemplate() || SeqObj->IsA(UUIStateSequence::StaticClass()) )
		{
			// leave UI sequences alone!
			continue;
		}

		// if there is no parent sequence look for a sequence that contains this obj
		if (SeqObj->ParentSequence == NULL)
		{
			UBOOL bFoundSeq = FALSE;
			for (TObjectIterator<USequence> SeqIt; SeqIt; ++SeqIt)
			{
				if (SeqIt->SequenceObjects.ContainsItem(SeqObj))
				{
					warnf(TEXT("Found parent sequence for object %s"),*SeqObj->GetPathName());
					bFoundSeq = TRUE;
					SeqObj->ParentSequence = *SeqIt;
					SeqObj->MarkPackageDirty( TRUE );
					ParentSequenceFixed.AddItem( SeqObj->GetPathName() );
					break;
				}
			}
			if (!bFoundSeq && !SeqObj->IsA(USequence::StaticClass()))
			{
				SeqObj->MarkPackageDirty( TRUE );
				USequence *Seq = Cast<USequence>(SeqObj->GetOuter());
				if (Seq != NULL)
				{
					warnf(TEXT("No containing sequence for object %s, placing in outer"),*SeqObj->GetPathName());
					Seq->SequenceObjects.AddItem(SeqObj);
					SeqObj->ParentSequence = Seq;
				}
				else if ( !SeqObj->IsA(UUIEvent::StaticClass()) && !SeqObj->IsA(UUIAction::StaticClass()) )
				{
					warnf(TEXT("No parent for object %s, placing in the root"),*SeqObj->GetPathName());
					Seq = GWorld->GetGameSequence();
					Seq->SequenceObjects.AddItem(SeqObj);
					SeqObj->ParentSequence = Seq;
				}
				else
				{
					continue;
				}

				ParentSequenceFixed.AddItem( SeqObj->GetPathName() );
			}
		}

		USequence* RootSeq = SeqObj->GetRootSequence();
		const UBOOL bIsUISequenceObject = RootSeq != NULL && RootSeq->IsA(UUISequence::StaticClass());
		if ( bIsUISequenceObject )
		{
			// leave UI sequences alone!
			continue;
		}
		// check it for general errors
		SeqObj->CheckForErrors();
		
		// If we have a ParentSequence, but it does not contain me - thats a bug. Fix it.
		if( SeqObj->ParentSequence && 
			!SeqObj->ParentSequence->SequenceObjects.ContainsItem(SeqObj) )
		{
			UBOOL bFoundCorrectParent = FALSE;

			// Find the sequence that _does_ contain SeqObj
			for( FObjectIterator TestIt; TestIt; ++TestIt )
			{
				UObject* TestObj = *TestIt;
				USequence* TestSeq = Cast<USequence>(TestObj);
				if(TestSeq && TestSeq->SequenceObjects.ContainsItem(SeqObj))
				{
					// If we have already found a sequence that contains SeqObj - warn about that too!
					if(bFoundCorrectParent)
					{
						warnf( TEXT("Multiple Sequences Contain '%s'"), *SeqObj->GetName() );
					}
					else
					{
						SeqObj->MarkPackageDirty( TRUE );
						ParentSequenceFixed.AddItem( SeqObj->GetPathName() );

						// Change ParentSequence pointer to correct sequence
						SeqObj->ParentSequence = TestSeq;

						// Mark package as dirty
						SeqObj->MarkPackageDirty();
						bFoundCorrectParent = TRUE;
					}
				}
			}

			// It's also bad if we couldn't find the correct parent!
			if(!bFoundCorrectParent)
			{
				debugf( TEXT("No correct parent found for '%s'.  Try entering 'OBJ REFS class=%s name=%s' into the command window to determine how this sequence object is being referenced."),
					*SeqObj->GetName(), *SeqObj->GetClass()->GetName(), *SeqObj->GetPathName() );
			}
		}
	}

	// If we corrected some things - tell the user
	if(ParentSequenceFixed.Num() > 0)
	{	
		appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("FixedKismetParentSequences"), ParentSequenceFixed.Num()));
	}
}

/**
 * Warns the user of any hidden streaming levels, and prompts them with a Yes/No dialog
 * for whether they wish to continue with the operation.  No dialog is presented if all
 * streaming levels are visible.  The return value is TRUE if no levels are hidden or
 * the user selects "Yes", or FALSE if the user selects "No".
 *
 * @param	AdditionalMessage		An additional message to include in the dialog.  Can be NULL.
 * @return							FALSE if the user selects "No", TRUE otherwise.
 */
UBOOL UEditorEngine::WarnAboutHiddenLevels(const TCHAR* AdditionalMessage) const
{
	UBOOL bResult = TRUE;

	// Make a list of all hidden levels.
	AWorldInfo*	WorldInfo = GWorld->GetWorldInfo();
	TArray<FString> HiddenLevels;
	for( INT LevelIndex = 0 ; LevelIndex< WorldInfo->StreamingLevels.Num() ; ++LevelIndex )
	{
		ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels( LevelIndex );
		if( StreamingLevel && !StreamingLevel->bShouldBeVisibleInEditor )
		{
			HiddenLevels.AddItem(StreamingLevel->PackageName.ToString() );
		}
	}

	// Warn the user that some levels are hidden and prompt for continue.
	if ( HiddenLevels.Num() > 0 )
	{
		FString ContinueMessage( LocalizeUnrealEd(TEXT("TheFollowingLevelsAreHidden")) );
		for ( INT LevelIndex = 0 ; LevelIndex < HiddenLevels.Num() ; ++LevelIndex )
		{
			ContinueMessage += FString::Printf( TEXT("\n    %s"), *HiddenLevels(LevelIndex) );
		}
		if ( AdditionalMessage )
		{
			ContinueMessage += FString::Printf( TEXT("\n%s"), AdditionalMessage );
		}
		else
		{
			ContinueMessage += FString::Printf( TEXT("\n%s"), *LocalizeUnrealEd(TEXT("ContinueQ")) );
		}
		bResult = appMsgf( AMT_YesNo, TEXT("%s"), *ContinueMessage );
	}

	return bResult;
}

/**
 *	Sets the texture to use for displaying StreamingBounds.
 *
 *	@param	InTexture	The source texture for displaying StreamingBounds.
 *						Pass in NULL to disable displaying them.
 */
void UEditorEngine::SetStreamingBoundsTexture(UTexture2D* InTexture)
{
	if (StreamingBoundsTexture != InTexture)
	{
		// Clear the currently stored streaming bounds information

		// Set the new texture
		StreamingBoundsTexture = InTexture;
		if (StreamingBoundsTexture != NULL)
		{
			// Fill in the new streaming bounds info
			for (TObjectIterator<ULevel> It; It; ++It)
			{
				ULevel* Level = *It;
				Level->BuildStreamingData(InTexture);
			}

			// Turn on the StreamingBounds show flag
			for(UINT ViewportIndex = 0; ViewportIndex < (UINT)ViewportClients.Num(); ViewportIndex++)
			{
				FEditorLevelViewportClient* ViewportClient = ViewportClients(ViewportIndex);
				if (ViewportClient)
				{
					ViewportClient->ShowFlags |= SHOW_StreamingBounds;
					ViewportClient->Invalidate(FALSE,TRUE);
				}
			}
		}
		else
		{
			// Clear the streaming bounds info
			// Turn off the StreamingBounds show flag
			for(UINT ViewportIndex = 0; ViewportIndex < (UINT)ViewportClients.Num(); ViewportIndex++)
			{
				FEditorLevelViewportClient* ViewportClient = ViewportClients(ViewportIndex);
				if (ViewportClient)
				{
					ViewportClient->ShowFlags &= ~SHOW_StreamingBounds;
					ViewportClient->Invalidate(FALSE,TRUE);
				}
			}
		}
	}
}

void UEditorEngine::ApplyDeltaToActor(AActor* InActor,
									  UBOOL bDelta,
									  const FVector* InTrans,
									  const FRotator* InRot,
									  const FVector* InScale,
									  UBOOL bAltDown,
									  UBOOL bShiftDown,
									  UBOOL bControlDown) const
{
	InActor->Modify();
	if( InActor->IsBrush() )
	{
		ABrush* Brush = (ABrush*)InActor;
		if( Brush->BrushComponent && Brush->BrushComponent->Brush )
		{
			Brush->BrushComponent->Brush->Polys->Element.ModifyAllItems();
		}
	}

	///////////////////
	// Rotation

	// Unfortunately this can't be moved into ABrush::EditorApplyRotation, as that would
	// create a dependence in Engine on Editor.
	if ( InRot )
	{
		const FRotator& InDeltaRot = *InRot;
		const UBOOL bRotatingActor = !bDelta || !InDeltaRot.IsZero();
		if( bRotatingActor )
		{
			if( InActor->IsBrush() )
			{
				FBSPOps::RotateBrushVerts( (ABrush*)InActor, InDeltaRot, TRUE );
			}
			else
			{
				if ( bDelta )
				{
					InActor->EditorApplyRotation( InDeltaRot, bAltDown, bShiftDown, bControlDown );
				}
				else
				{
					InActor->SetRotation( InDeltaRot );
				}
			}

			if ( bDelta )
			{
				FVector NewActorLocation = InActor->Location;
				NewActorLocation -= GEditorModeTools().PivotLocation;
				NewActorLocation = FRotationMatrix( InDeltaRot ).TransformFVector( NewActorLocation );
				NewActorLocation += GEditorModeTools().PivotLocation;
				InActor->Location = NewActorLocation;
			}
		}
	}

	///////////////////
	// Translation
	if ( InTrans )
	{
		if ( bDelta )
		{
			InActor->EditorApplyTranslation( *InTrans, bAltDown, bShiftDown, bControlDown );
		}
		else
		{
			InActor->SetLocation( *InTrans );
		}
	}

	///////////////////
	// Scaling
	if ( InScale )
	{
		const FVector& InDeltaScale = *InScale;
		const UBOOL bScalingActor = !bDelta || !InDeltaScale.IsNearlyZero(0.000001f);
		if( bScalingActor )
		{
			// If the actor is a brush, update the vertices.
			if( InActor->IsBrush() )
			{
				ABrush* Brush = (ABrush*)InActor;
				FVector ModifiedScale = InDeltaScale;

				// Get Box Extents
				const FBox BrushBox = InActor->GetComponentsBoundingBox( TRUE );
				const FVector BrushExtents = BrushBox.GetExtent();

				// Make sure brushes are clamped to a minimum size.
				FLOAT MinThreshold = 1.0f;

				for (INT Idx=0; Idx<3; Idx++)
				{
					const UBOOL bBelowAllowableScaleThreshold = ((InDeltaScale[Idx] + 1.0f) * BrushExtents[Idx]) < MinThreshold;

					if(bBelowAllowableScaleThreshold)
					{
						ModifiedScale[Idx] = (MinThreshold / BrushExtents[Idx]) - 1.0f;
					}
				}

				// If we are uniformly scaling, make sure that the modified scale is always the same for all 3 axis.
				if(GEditorModeTools().GetWidgetMode() == FWidget::WM_Scale)
				{
					INT Min = 0;
					for(INT Idx=1; Idx < 3; Idx++)
					{
						if(Abs(ModifiedScale[Idx]) < Abs(ModifiedScale[Min]))
						{
							Min=Idx;
						}
					}

					for(INT Idx=0; Idx < 3; Idx++)
					{
						if(Min != Idx)
						{
							ModifiedScale[Idx] = ModifiedScale[Min];
						}
					}
				}

				// Scale all of the polygons of the brush.
				const FScaleMatrix matrix( FVector( ModifiedScale.X , ModifiedScale.Y, ModifiedScale.Z ) );
				
				if(Brush->BrushComponent->Brush && Brush->BrushComponent->Brush->Polys)
				{
					for( INT poly = 0 ; poly < Brush->BrushComponent->Brush->Polys->Element.Num() ; poly++ )
					{
						FPoly* Poly = &(Brush->BrushComponent->Brush->Polys->Element(poly));

						FBox bboxBefore(0);
						for( INT vertex = 0 ; vertex < Poly->Vertices.Num() ; vertex++ )
						{
							bboxBefore += Brush->LocalToWorld().TransformFVector( Poly->Vertices(vertex) );
						}

						// Scale the vertices

						for( INT vertex = 0 ; vertex < Poly->Vertices.Num() ; vertex++ )
						{
							FVector Wk = Brush->LocalToWorld().TransformFVector( Poly->Vertices(vertex) );
							Wk -= GEditorModeTools().PivotLocation;
							Wk += matrix.TransformFVector( Wk );
							Wk += GEditorModeTools().PivotLocation;
							Poly->Vertices(vertex) = Brush->WorldToLocal().TransformFVector( Wk );
						}

						FBox bboxAfter(0);
						for( INT vertex = 0 ; vertex < Poly->Vertices.Num() ; vertex++ )
						{
							bboxAfter += Brush->LocalToWorld().TransformFVector( Poly->Vertices(vertex) );
						}

						FVector Wk = Brush->LocalToWorld().TransformFVector( Poly->Base );
						Wk -= GEditorModeTools().PivotLocation;
						Wk += matrix.TransformFVector( Wk );
						Wk += GEditorModeTools().PivotLocation;
						Poly->Base = Brush->WorldToLocal().TransformFVector( Wk );

						// Scale the texture vectors

						for( INT a = 0 ; a < 3 ; ++a )
						{
							const FLOAT Before = bboxBefore.GetExtent()[a];
							const FLOAT After = bboxAfter.GetExtent()[a];

							if( After != 0.0 )
							{
								const FLOAT Pct = Before / After;

								if( Pct != 0.0 )
								{
									Poly->TextureU[a] *= Pct;
									Poly->TextureV[a] *= Pct;
								}
							}
						}

						// Recalc the normal for the poly

						Poly->Normal = FVector(0,0,0);
						Poly->Finalize((ABrush*)InActor,0);
					}

					Brush->BrushComponent->Brush->BuildBound();

					if( !Brush->IsStaticBrush() )
					{
						FBSPOps::csgPrepMovingBrush( Brush );
					}
				}
			}
			else
			{
				if ( bDelta )
				{
					const FScaleMatrix matrix( FVector( InDeltaScale.X , InDeltaScale.Y, InDeltaScale.Z ) );
					InActor->EditorApplyScale( InDeltaScale,
												matrix,
												&GEditorModeTools().PivotLocation,
												bAltDown,
												bShiftDown,
												bControlDown );
				}
				else
				{
					InActor->DrawScale3D = InDeltaScale;
				}
			}

			InActor->ClearComponents();
		}
	}

	// Update the actor before leaving.
	InActor->MarkPackageDirty();
	InActor->InvalidateLightingCache();
	InActor->PostEditMove( FALSE );
	InActor->ForceUpdateComponents();
}

/**
 * Handles freezing/unfreezing of rendering
 */
void UEditorEngine::ProcessToggleFreezeCommand()
{
#if !FINAL_RELEASE && !SHIPPING_PC_GAME
	if (GIsPlayInEditorWorld)
	{
		GamePlayers(0)->ViewportClient->Viewport->ProcessToggleFreezeCommand();
	}
	else
	{
		// pass along the freeze command to all perspective viewports
		for(INT ViewportIndex = 0; ViewportIndex < ViewportClients.Num(); ViewportIndex++)
		{
			if (ViewportClients(ViewportIndex)->ViewportType == LVT_Perspective)
			{
				ViewportClients(ViewportIndex)->Viewport->ProcessToggleFreezeCommand();
			}
		}
	}

	// tell editor to update views
	RedrawAllViewports();
#endif
}

/**
 * Handles frezing/unfreezing of streaming
 */
void UEditorEngine::ProcessToggleFreezeStreamingCommand()
{
#if !FINAL_RELEASE && !SHIPPING_PC_GAME
	// freeze vis in PIE
	if (GIsPlayInEditorWorld)
	{
		PlayWorld->bIsLevelStreamingFrozen = !PlayWorld->bIsLevelStreamingFrozen;
	}
#endif
}

/*-----------------------------------------------------------------------------
	Reimporting.
-----------------------------------------------------------------------------*/

/** Constructor */
FReimportManager::FReimportManager()
{
	// Create reimport handler for textures
	// NOTE: New factories can be created anywhere, inside or outside of editor
	// This is just here for convenience
	UReimportTextureFactory::StaticClass();
}


/** 
* Singleton function
* @return Singleton instance of this manager
*/
FReimportManager* FReimportManager::Instance()
{
	static FReimportManager Inst;
	return &Inst;
}

/**
* Reimports specified texture from its source material, if the meta-data exists
* @param Package texture to reimport
* @return TRUE if handled
*/
UBOOL FReimportManager::Reimport( UObject* Obj )
{
	for(INT I=0;I<Handlers.Num();I++)
	{
		if(Handlers(I)->Reimport(Obj))
			return TRUE;
	}
	return FALSE;
}

/*-----------------------------------------------------------------------------
	PIE helpers.
-----------------------------------------------------------------------------*/

/**
 * Sets GWorld to the passed in PlayWorld and sets a global flag indicating that we are playing
 * in the Editor.
 *
 * @param	PlayInEditorWorld		PlayWorld
 * @return	the original GWorld
 */
UWorld* SetPlayInEditorWorld( UWorld* PlayInEditorWorld )
{
	check(!GIsPlayInEditorWorld);
	UWorld* SavedWorld = GWorld;
	GIsPlayInEditorWorld = TRUE;
	GIsGame = (!GIsEditor || GIsPlayInEditorWorld);
	GWorld = PlayInEditorWorld;
	return SavedWorld;
}

/**
 * Restores GWorld to the passed in one and reset the global flag indicating whether we are a PIE
 * world or not.
 *
 * @param EditorWorld	original world being edited
 */
void RestoreEditorWorld( UWorld* EditorWorld )
{
	check(GIsPlayInEditorWorld);
	GIsPlayInEditorWorld = FALSE;
	GIsGame = (!GIsEditor || GIsPlayInEditorWorld);
	GWorld = EditorWorld;
}


/*-----------------------------------------------------------------------------
	Cooking helpers.
-----------------------------------------------------------------------------*/

/**
 * Decompresses sound data compressed for a specific platform
 *
 * @param	SourceData				Compressed sound data
 * @param	SoundCooker				Platform specific cooker object to cook with
 * @param	Destination				Where to put the decompressed sounds
 */
void ThawSoundNodeWave( FConsoleSoundCooker* SoundCooker, USoundNodeWave* Wave, SWORD*& Destination )
{
	Destination = NULL;

	if( SoundCooker->Decompress( ( BYTE* )Wave->ResourceData, Wave->ResourceSize, Wave ) )
	{
		Destination = ( SWORD* )appMalloc( SoundCooker->GetRawDataSize() );
		SoundCooker->GetRawData( ( BYTE* )Destination );
	}
}

/*
 * Create a struct to pass to tools from the parsed wave data
 */
void CreateWaveFormat( FWaveModInfo& WaveInfo, WAVEFORMATEXTENSIBLE& WaveFormat )
{
	appMemzero( &WaveFormat, sizeof( WAVEFORMATEXTENSIBLE ) );

	WaveFormat.Format.nAvgBytesPerSec = *WaveInfo.pAvgBytesPerSec;
	WaveFormat.Format.nBlockAlign = *WaveInfo.pBlockAlign;
	WaveFormat.Format.nChannels = *WaveInfo.pChannels;
	WaveFormat.Format.nSamplesPerSec = *WaveInfo.pSamplesPerSec;
	WaveFormat.Format.wBitsPerSample = *WaveInfo.pBitsPerSample;
	WaveFormat.Format.wFormatTag = WAVE_FORMAT_PCM;
}

/**
 * Read a wave file header from bulkdata
 */
UBOOL ReadWaveHeader( FWaveModInfo& WaveInfo, BYTE* RawWaveData, INT Size, INT Offset )
{
	if( Size == 0 )
	{
		return( FALSE );
	}

	// Parse wave info.
	if( !WaveInfo.ReadWaveInfo( RawWaveData + Offset, Size ) )
	{
		return( FALSE );
	}

	// Validate the info
	if( ( *WaveInfo.pChannels != 1 && *WaveInfo.pChannels != 2 ) || *WaveInfo.pBitsPerSample != 16 )
	{
		return( FALSE );
	}

	return( TRUE );
}

/**
 * Cook a simple mono or stereo wave
 */
void CookSimpleWave( USoundNodeWave* SoundNodeWave, FConsoleSoundCooker* SoundCooker, FByteBulkData& DestinationData )
{
	WAVEFORMATEXTENSIBLE	WaveFormat;						

	// did we lock existing bulk data?
	UBOOL bWasRawDataLocked = FALSE;

	// do we have good wave data and WaveInfo?
	UBOOL bHasGoodWaveData = FALSE;

	// sound params
	void* SampleDataStart = NULL;
	INT SampleDataSize = 0;

	// check if there is any raw sound data
	if( SoundNodeWave->RawData.GetBulkDataSize() > 0 )
	{
		// Lock raw wave data.
		BYTE* RawWaveData = ( BYTE * )SoundNodeWave->RawData.Lock( LOCK_READ_ONLY );
		INT RawDataSize = SoundNodeWave->RawData.GetBulkDataSize();

		// mark that we locked the data
		bWasRawDataLocked = TRUE;

		FWaveModInfo			WaveInfo;
	
		// parse the wave data
		if( !ReadWaveHeader( WaveInfo, RawWaveData, RawDataSize, 0 ) )
		{
			warnf( TEXT( "Only mono or stereo 16 bit wavs allowed: %s" ), *SoundNodeWave->GetFullName() );
		}
		else
		{
			// mark that we succeeded
			bHasGoodWaveData = TRUE;

			// Create wave format structure for encoder. Set up like a regular WAVEFORMATEX.
			CreateWaveFormat( WaveInfo, WaveFormat );

			// copy out some values
			SampleDataStart = WaveInfo.SampleDataStart;
			SampleDataSize = WaveInfo.SampleDataSize;
		}
	}
	// if the raw data didn't exist, try to uncompress the PC compressed data
	// @todo ship: Duplicate this for surround sound!
	else
	{
		if( SoundNodeWave->CompressedPCData.GetBulkDataSize() == 0 )
		{
			warnf( TEXT( "Can't cook %s because there is no source compressed or uncompressed PC sound data" ), *SoundNodeWave->GetFullName() );
		}
		else
		{
			SoundNodeWave->RemoveAudioResource();
			SoundNodeWave->InitAudioResource( SoundNodeWave->CompressedPCData );

			BYTE* PCMData;
			if( GetPCSoundCooker()->Decompress( ( BYTE* )SoundNodeWave->ResourceData, SoundNodeWave->ResourceSize, SoundNodeWave ) )
			{
				// retrieve uncompressed data
				INT PCMDataSize = GetPCSoundCooker()->GetRawDataSize();
				PCMData = ( BYTE* )appMalloc( PCMDataSize );
				SoundCooker->GetRawData( PCMData );

				// get size of uncompressed data
				SampleDataSize = PCMDataSize;

				// use the PCM Data as the sample data to compress
				SampleDataStart = ( BYTE* )PCMData;

				// fill out wave info
				WaveFormat.Format.nAvgBytesPerSec = SoundNodeWave->NumChannels * SoundNodeWave->SampleRate * sizeof( SWORD );
				WaveFormat.Format.nBlockAlign = SoundNodeWave->NumChannels * sizeof( SWORD );
				WaveFormat.Format.nChannels = SoundNodeWave->NumChannels;
				WaveFormat.Format.nSamplesPerSec = SoundNodeWave->SampleRate;
				WaveFormat.Format.wBitsPerSample = 16;
				WaveFormat.Format.wFormatTag = WAVE_FORMAT_PCM;

				// mark that we succeeded
				bHasGoodWaveData = TRUE;
			}
			else
			{
				warnf( TEXT( "Failed to decompress PC Vorbis data for %s" ), *SoundNodeWave->GetFullName() );
			}
		}
	}

	// we good to go?
	if( bHasGoodWaveData )
	{
		debugf( TEXT( "Cooking: %s" ), *SoundNodeWave->GetFullName() );

		// Cook the data.
		if( SoundCooker->Cook( ( short* )SampleDataStart, SampleDataSize, &WaveFormat, SoundNodeWave->CompressionQuality, *SoundNodeWave->GetFullName() ) == TRUE ) 
		{
			// Make place for cooked data.
			DestinationData.Lock( LOCK_READ_WRITE );
			BYTE* RawCompressedData = ( BYTE* )DestinationData.Realloc( SoundCooker->GetCookedDataSize() );

			// Retrieve cooked data.
			SoundCooker->GetCookedData( RawCompressedData );
			DestinationData.Unlock();

			SoundNodeWave->SampleRate = WaveFormat.Format.nSamplesPerSec;
			SoundNodeWave->NumChannels = WaveFormat.Format.nChannels;
			SoundNodeWave->SampleDataSize = SampleDataSize;
			SoundNodeWave->Duration = ( FLOAT )SoundNodeWave->SampleDataSize / ( SoundNodeWave->SampleRate * sizeof( SWORD ) * SoundNodeWave->NumChannels );
		}
		else
		{
			const char* ErrorMessages = SoundCooker->GetCookErrorMessages();
			if( ErrorMessages != NULL )
			{
				const INT MsgLength = strlen( ErrorMessages ) + 1;
				TCHAR* TCErrorMessages = new TCHAR[ MsgLength ];
				MultiByteToWideChar( CP_ACP, 0, ErrorMessages, MsgLength, TCErrorMessages, MsgLength );

				warnf( NAME_Warning, TEXT( "Cooking simple sound failed: %s\nReason: %s\n" ),
					*SoundNodeWave->GetPathName(), TCErrorMessages );

				delete[] TCErrorMessages;
			}
			else
			{
				warnf( NAME_Warning, TEXT( "Cooking simple sound failed (unknown reason): %s\n" ),
					*SoundNodeWave->GetPathName() );
			}

			// Empty data and set invalid format token.
			DestinationData.RemoveBulkData();
		}
	}

	// handle freeing/unlocking temp buffers
	if( bWasRawDataLocked )
	{
		// Unlock source as we no longer need the data
		SoundNodeWave->RawData.Unlock();
	}
	else
	{
		// if we didn't lock, we potentially allocated memory
		appFree( SampleDataStart );
	}
}

/**
 * Cook a multistream (normally 5.1) wave
 */
void CookSurroundWave( USoundNodeWave* SoundNodeWave, FConsoleSoundCooker* SoundCooker, FByteBulkData& DestinationData )
{
	INT						i, ChannelCount;
	DWORD					SampleDataSize;
	FWaveModInfo			WaveInfo;
	WAVEFORMATEXTENSIBLE	WaveFormat;						
	short *					SourceBuffers[SPEAKER_Count] = { NULL };

	BYTE* RawWaveData = ( BYTE * )SoundNodeWave->RawData.Lock( LOCK_READ_ONLY );

	// Front left channel is the master
	ChannelCount = 1;
	if( ReadWaveHeader( WaveInfo, RawWaveData, SoundNodeWave->ChannelSizes( SPEAKER_FrontLeft ), SoundNodeWave->ChannelOffsets( SPEAKER_FrontLeft ) ) )
	{
		SampleDataSize = WaveInfo.SampleDataSize;
		SourceBuffers[SPEAKER_FrontLeft] = ( short * )WaveInfo.SampleDataStart;

		// Create wave format structure for encoder. Set up like a regular WAVEFORMATEX.
		CreateWaveFormat( WaveInfo, WaveFormat );

		// Extract all the info for the other channels (may be blank)
		for( i = 1; i < SPEAKER_Count; i++ )
		{
			if( ReadWaveHeader( WaveInfo, RawWaveData, SoundNodeWave->ChannelSizes( i ), SoundNodeWave->ChannelOffsets( i ) ) )
			{
				// Only mono files allowed
				if( *WaveInfo.pChannels == 1 )
				{
					SourceBuffers[i] = ( short * )WaveInfo.SampleDataStart;
					ChannelCount++;

					// Truncating to the shortest sound
					if( WaveInfo.SampleDataSize < SampleDataSize )
					{
						SampleDataSize = WaveInfo.SampleDataSize;
					}
				}
			}
		}
	
		// Only allow the formats that can be played back through
		if( ChannelCount == 4 || ChannelCount == 6 || ChannelCount == 7 || ChannelCount == 8 )
		{
			debugf( TEXT( "Cooking %d channels for: %s" ), ChannelCount, *SoundNodeWave->GetFullName() );

			if( SoundCooker->CookSurround( SourceBuffers, SampleDataSize, &WaveFormat, SoundNodeWave->CompressionQuality, *SoundNodeWave->GetFullName() ) == TRUE ) 
			{
				// Make place for cooked data.
				DestinationData.Lock( LOCK_READ_WRITE );
				BYTE * RawCompressedData = ( BYTE * )DestinationData.Realloc( SoundCooker->GetCookedDataSize() );

				// Retrieve cooked data.
				SoundCooker->GetCookedData( RawCompressedData );
				DestinationData.Unlock();

				SoundNodeWave->SampleRate = *WaveInfo.pSamplesPerSec;
				SoundNodeWave->NumChannels = ChannelCount;
				SoundNodeWave->SampleDataSize = SampleDataSize * ChannelCount;
				SoundNodeWave->Duration = ( FLOAT )SoundNodeWave->SampleDataSize / ( SoundNodeWave->SampleRate * sizeof( SWORD ) );
			}
			else
			{
				const char* ErrorMessages = SoundCooker->GetCookErrorMessages();
				if( ErrorMessages != NULL )
				{
					const INT MsgLength = strlen( ErrorMessages ) + 1;
					TCHAR* TCErrorMessages = new TCHAR[ MsgLength ];
					MultiByteToWideChar( CP_ACP, 0, ErrorMessages, MsgLength, TCErrorMessages, MsgLength );

					warnf( NAME_Warning, TEXT( "Cooking surround sound failed: %s\nReason: %s\n" ),
						*SoundNodeWave->GetPathName(), TCErrorMessages );
				}
				else
				{
					warnf( NAME_Warning, TEXT( "Cooking surround sound failed (unknown reason): %s\n" ),
						*SoundNodeWave->GetPathName() );
				}

				// Empty data and set invalid format token.
				DestinationData.RemoveBulkData();
			}
		}
		else
		{
			warnf( NAME_Warning, TEXT( "No format available for a %d channel surround sound: %s" ), ChannelCount, *SoundNodeWave->GetFullName() );
		}
	}
	else
	{
		warnf( NAME_Warning, TEXT( "Cooking surround sound failed: %s" ), *SoundNodeWave->GetPathName() );
	}

	SoundNodeWave->RawData.Unlock();
}

/**
 * Cooks SoundNodeWave to a specific platform
 *
 * @param	SoundNodeWave			Wave file to cook
 * @param	SoundCooker				Platform specific cooker object to cook with
 * @param	DestinationData			Destination bulk data
 */
UBOOL CookSoundNodeWave( USoundNodeWave* SoundNodeWave, FConsoleSoundCooker* SoundCooker, FByteBulkData& DestinationData )
{
	check( SoundCooker );

	if( DestinationData.GetBulkDataSize() > 0 && SoundNodeWave->NumChannels > 0 )
	{
		// Already cooked for this platform
		return( FALSE );
	}

	// Compress the sound using the platform specific cooker compression (if not already compressed)
	if( SoundNodeWave->ChannelSizes.Num() == 0 )
	{
		check( SoundNodeWave->ChannelOffsets.Num() == 0 );
		check( SoundNodeWave->ChannelSizes.Num() == 0 );
		CookSimpleWave( SoundNodeWave, SoundCooker, DestinationData );
	}
	else if( SoundNodeWave->ChannelSizes.Num() > 0 )
	{
		check( SoundNodeWave->ChannelOffsets.Num() == SPEAKER_Count );
		check( SoundNodeWave->ChannelSizes.Num() == SPEAKER_Count );
		CookSurroundWave( SoundNodeWave, SoundCooker, DestinationData );
	}

	return( TRUE );
}

/**
 * Cooks SoundNodeWave to all available platforms
 *
 * @param	SoundNodeWave			Wave file to cook
 * @param	Platform				Platform to cook for - PLATFORM_Unknown for all
 */
UBOOL CookSoundNodeWave( USoundNodeWave* SoundNodeWave, UE3::EPlatformType Platform )
{
	UBOOL bDirty = FALSE;

	if( Platform == UE3::PLATFORM_Unknown || Platform == UE3::PLATFORM_Xenon )
	{
		FConsoleSupport* XenonSupport = FConsoleSupportContainer::GetConsoleSupportContainer()->GetConsoleSupport( TEXT( "Xenon" ) );
		if( XenonSupport )
		{
			bDirty |= CookSoundNodeWave( SoundNodeWave, XenonSupport->GetGlobalSoundCooker(), SoundNodeWave->CompressedXbox360Data );
		}
	}

	if( Platform == UE3::PLATFORM_Unknown || Platform == UE3::PLATFORM_PS3 )
	{
		FConsoleSupport* PS3Support = FConsoleSupportContainer::GetConsoleSupportContainer()->GetConsoleSupport( TEXT( "PS3" ) );
		if( PS3Support )
		{
			bDirty |= CookSoundNodeWave( SoundNodeWave, PS3Support->GetGlobalSoundCooker(), SoundNodeWave->CompressedPS3Data );
		}
	}

	if( Platform == UE3::PLATFORM_Unknown || Platform == UE3::PLATFORM_Windows )
	{
		bDirty |= CookSoundNodeWave( SoundNodeWave, GetPCSoundCooker(), SoundNodeWave->CompressedPCData );
	}

	return( bDirty );
}

/**
 * Compresses SoundNodeWave for all available platforms, and then decompresses to PCM 
 *
 * @param	SoundNodeWave			Wave file to compress
 * @param	PreviewInfo				Compressed stats and decompressed data
 */
void SoundNodeWaveQualityPreview( USoundNodeWave* SoundNode, FPreviewInfo* PreviewInfo )
{
	// Compress to all platforms
	SoundNode->CompressionQuality = PreviewInfo->QualitySetting;

	SoundNode->CompressedPCData.RemoveBulkData();
	SoundNode->CompressedXbox360Data.RemoveBulkData();
	SoundNode->CompressedPS3Data.RemoveBulkData();
	SoundNode->NumChannels = 0;

	CookSoundNodeWave( SoundNode );

	// Extract the stats
	PreviewInfo->OriginalSize = SoundNode->SampleDataSize;
	PreviewInfo->OggVorbisSize = SoundNode->CompressedPCData.GetBulkDataSize();
	PreviewInfo->XMASize = SoundNode->CompressedXbox360Data.GetBulkDataSize();
	PreviewInfo->PS3Size = SoundNode->CompressedPS3Data.GetBulkDataSize();

	// Expand back to PCM data
	SoundNode->RemoveAudioResource();
	SoundNode->InitAudioResource( SoundNode->CompressedPCData );
	ThawSoundNodeWave( GetPCSoundCooker(), SoundNode, PreviewInfo->DecompressedOggVorbis );

	if( PreviewInfo->DecompressedOggVorbis == NULL )
	{
		debugfSuppressed( NAME_DevAudio, TEXT( "PC decompression failed" ) );
	}
	
	FConsoleSupport* XenonSupport = FConsoleSupportContainer::GetConsoleSupportContainer()->GetConsoleSupport( TEXT( "Xenon" ) );
	if( XenonSupport )
	{
		SoundNode->RemoveAudioResource();
		SoundNode->InitAudioResource( SoundNode->CompressedXbox360Data );
		ThawSoundNodeWave( XenonSupport->GetGlobalSoundCooker(), SoundNode, PreviewInfo->DecompressedXMA );
		if( PreviewInfo->DecompressedXMA == NULL )
		{
			debugfSuppressed( NAME_DevAudio, TEXT( "Xenon decompression failed" ) );
		}
	}

	FConsoleSupport* PS3Support = FConsoleSupportContainer::GetConsoleSupportContainer()->GetConsoleSupport( TEXT( "PS3" ) );
	if( PS3Support )
	{
		SoundNode->RemoveAudioResource();
		SoundNode->InitAudioResource( SoundNode->CompressedPS3Data );
		ThawSoundNodeWave( PS3Support->GetGlobalSoundCooker(), SoundNode, PreviewInfo->DecompressedPS3 );
		if( PreviewInfo->DecompressedPS3 == NULL )
		{
			debugfSuppressed( NAME_DevAudio, TEXT( "PS3 decompression failed" ) );
		}
	}
}

/** 
 * Makes sure ogg vorbis data is available for this sound node by converting on demand
 */
UBOOL USoundNodeWave::ValidateData( void )
{
	return( CookSoundNodeWave( this, GetPCSoundCooker(), CompressedPCData ) );
}

/** 
 * Makes sure ogg vorbis data is available for all sound nodes in this cue by converting on demand
 */
UBOOL USoundCue::ValidateData( void )
{
	TArray<USoundNodeWave*> Waves;
	RecursiveFindWaves( FirstNode, Waves );

	UBOOL Converted = FALSE;
	for( INT WaveIndex = 0; WaveIndex < Waves.Num(); ++WaveIndex )
	{
		USoundNodeWave* Sound = Waves( WaveIndex );
		if( !Sound->bUseTTS )
		{
			Converted |= Sound->ValidateData();
		}
	}

	return( Converted );
}

// Content tag index interface

FTagInfo* UContentTagIndex::FindDefaultTagInfoForClass(UClass *ClassToFind)
{
	for (TArray<FTagInfo>::TIterator It(DefaultTags); It; ++It)
	{
		if ((*It).AssetType == NULL)
		{
			(*It).AssetType = Cast<UClass>(UObject::StaticLoadObject(UClass::StaticClass(),NULL,*((*It).AssetTypeName),NULL,0,NULL));
		}
		if ((*It).AssetType != NULL && (*It).AssetType == ClassToFind)
		{
			return &(*It);
		}
	}
	return NULL;
}

FTagInfo& UContentTagIndex::GetTagInfo(UClass *ClassToFind)
{
	// search for existing entry first
	for (INT Idx = 0; Idx < Tags.Num(); Idx++)
	{
		if (Tags(Idx).AssetType == ClassToFind)
		{
			return Tags(Idx);
		}
	}
	// create new one
	INT Idx = Tags.AddZeroed();
	Tags(Idx).AssetType = ClassToFind;
	Tags(Idx).AssetTypeName = Tags(Idx).AssetType->GetName();
	return Tags(Idx);
}

void UContentTagIndex::AddContentReference(FTagInfo &TagInfo, TArray<FName> &AssetTags, FString AssetReference)
{
	// iterate each content tag and add it to the index
	for (INT TagIdx = 0; TagIdx < AssetTags.Num(); TagIdx++)
	{
		FName Tag = AssetTags(TagIdx);
		INT NewIdx = TagInfo.Tags.AddUniqueItem(Tag);
		// fill in the references array to match the tags list in case we added a new tag
		if (NewIdx >= TagInfo.Assets.Num())
		{
			INT NewEntriesToAdd = TagInfo.Assets.Num() - NewIdx + 1;
			TagInfo.Assets.AddZeroed(NewEntriesToAdd);
		}
		TagInfo.Assets(NewIdx).AssetReference.AddItem(AssetReference);
		if (GIsUCC && ParseParam(appCmdLine(),TEXT("verbose")))
		{
			warnf(TEXT("...added content tags for asset %s, type %s"),*AssetReference,*TagInfo.AssetTypeName);
		}
	}
}

#include "EngineParticleClasses.h"

void UContentTagIndex::ApplyTagsToObjects(TArray<UObject*> &Objects, UClass* ObjectsClass, TArray<FName> &TagsToApply)
{
	for (TArray<UObject*>::TIterator It(Objects); It; ++It)
	{
		UObject *TargetObject = *It;
		TArray<FName> *TagsArrayToUpdate = NULL;
		if (TargetObject->IsA(UStaticMesh::StaticClass()))
		{
			TagsArrayToUpdate = &((UStaticMesh*)TargetObject)->ContentTags;
		}
		else if (TargetObject->IsA(UParticleSystem::StaticClass()))
		{
			TagsArrayToUpdate = &((UParticleSystem*)TargetObject)->ContentTags;
		}
		else if (TargetObject->IsA(UMaterial::StaticClass()))
		{
			TagsArrayToUpdate = &((UMaterial*)TargetObject)->ContentTags;
		}
		// update the actual array here
		if (TagsArrayToUpdate != NULL)
		{
			TagsArrayToUpdate->Empty();
			TagsArrayToUpdate->Append(TagsToApply);
			// and add to the local index
			UContentTagIndex *LocalTagIndex = GetLocalContentTagIndex();
			LocalTagIndex->AddContentReference(LocalTagIndex->GetTagInfo(ObjectsClass),TagsToApply,TargetObject->GetPathName());
			// mark the package as dirty
			TargetObject->MarkPackageDirty();
		}
	}
}

void UContentTagIndex::GetTagsFromObjects(TArray<UObject*> &Objects, UClass* ObjectsClass, TArray<FName> &OutTags)
{
	for (TArray<UObject*>::TIterator It(Objects); It; ++It)
	{
		UObject *TargetObject = *It;
		TArray<FName> *TagsArrayToRead = NULL;
		if (TargetObject->IsA(UStaticMesh::StaticClass()))
		{
			TagsArrayToRead = &((UStaticMesh*)TargetObject)->ContentTags;
		}
		else if (TargetObject->IsA(UParticleSystem::StaticClass()))
		{
			TagsArrayToRead = &((UParticleSystem*)TargetObject)->ContentTags;
		}
		else if (TargetObject->IsA(UMaterial::StaticClass()))
		{
			TagsArrayToRead = &((UMaterial*)TargetObject)->ContentTags;
		}
		// update the actual array here
		if (TagsArrayToRead != NULL)
		{
			OutTags.Append(*TagsArrayToRead);
		}
	}
}

void UContentTagIndex::BuildAssetTypeToAssetMap(AssetMapType &OutAssetTypeToAssetMap)
{
	for (TArray<FTagInfo>::TIterator TagIter(Tags); TagIter; ++TagIter)
	{
		FTagInfo &TagInfo = *TagIter;
		// grab/create entry in asset type to assets map
		TMap<FString,TArray<FName> > *AssetToTagsMap = OutAssetTypeToAssetMap.Find(TagInfo.AssetType);
		if (AssetToTagsMap == NULL)
		{
			AssetToTagsMap = &OutAssetTypeToAssetMap.Set(TagInfo.AssetType,TMap<FString,TArray<FName> >());
		}
		for (INT TagIdx = 0; TagIdx < TagInfo.Tags.Num(); TagIdx++)
		{
			FName Tag = TagInfo.Tags(TagIdx);
			for (TArray<FString>::TIterator AssetIter(TagInfo.Assets(TagIdx).AssetReference); AssetIter; ++AssetIter)
			{
				FString Asset = *AssetIter;
				//grab create entry in assets to tags map
				TArray<FName> *AssetTags = AssetToTagsMap->Find(Asset);
				if (AssetTags == NULL)
				{
					AssetTags = &AssetToTagsMap->Set(Asset,TArray<FName>());
				}
				AssetTags->AddItem(Tag);
			}
		}
	}
}

void UContentTagIndex::MergeFromRefContentTagIndex()
{
	debugf(TEXT("Updating from reference content tag index..."));
	const FFilename Filename = FString::Printf(TEXT("%sContent\\RefContentTagsIndex.upk"),*appGameDir());
	UPackage *Pkg = UObject::LoadPackage(NULL,*Filename,0);
	if (Pkg != NULL)
	{
		UContentTagIndex *RefTagIndex = FindObject<UContentTagIndex>(Pkg,TEXT("TagIndex"));
		// if the ref version is newer then merge
		if (RefTagIndex->VersionInfo > VersionInfo)
		{
			debugf(TEXT("...reference version newer than local, merging..."));
			Tags.Empty();
			// grab the data from the local and reference indices
			AssetMapType AssetTypeToAssetMap;
			BuildAssetTypeToAssetMap(AssetTypeToAssetMap);
			AssetMapType RefAssetTypeToAssetMap;
			RefTagIndex->BuildAssetTypeToAssetMap(RefAssetTypeToAssetMap);
			// add any asset refs not existing in the ref index
			for (AssetMapType::TIterator RefTypeIter(RefAssetTypeToAssetMap); RefTypeIter; ++RefTypeIter)
			{
				FTagInfo &TagInfo = GetTagInfo(RefTypeIter.Key());
				TMap<FString,TArray<FName> > &RefAssetToTagsMap = RefTypeIter.Value();
				for (AssetMapType::TIterator TypeIter(AssetTypeToAssetMap); TypeIter; ++TypeIter)
				{
					if (TypeIter.Key() == RefTypeIter.Key())
					{
						TMap<FString,TArray<FName> > &AssetToTagsMap = TypeIter.Value();
						// iterate through local index looking for assets not in ref
						for (TMap<FString,TArray<FName> >::TIterator AssetIter(AssetToTagsMap); AssetIter; ++AssetIter)
						{
							FString Asset = AssetIter.Key();
							// if it doesn't exist in the ref add it here
							if (RefAssetToTagsMap.Find(Asset) == NULL)
							{
								RefAssetToTagsMap.Set(Asset,AssetIter.Value());
							}
						}
					}
				}
				// now add the contents of the updated ref as the new index
				for (TMap<FString,TArray<FName> >::TIterator AssetIter(RefAssetToTagsMap); AssetIter; ++AssetIter)
				{
					AddContentReference(TagInfo,AssetIter.Value(),AssetIter.Key());
				}
			}
			// update version and request a save
			VersionInfo = RefTagIndex->VersionInfo;
			MarkPackageDirty(TRUE);
		}
		else
		{
			debugf(TEXT("...reference version matches, ignoring."));
		}
	}
	else
	{
		debugf(TEXT("...failed to load reference content tag index!"));
	}
}

UContentTagIndex* UContentTagIndex::GetLocalContentTagIndex()
{
	static UContentTagIndex* LocalTagIndex = NULL;
	if (LocalTagIndex == NULL)
	{
		// first try to load it
		const FFilename Filename = FString::Printf(TEXT("%sContent\\LocalContentTagsIndex.upk"),*appGameDir());
		UPackage *Pkg = UObject::LoadPackage(NULL,*Filename,0);
		if (Pkg != NULL)
		{
			LocalTagIndex = FindObject<UContentTagIndex>(Pkg,TEXT("TagIndex"));
		}
		else
		{
			Pkg = CreatePackage(NULL,TEXT("LocalContentTagsIndex"));
		}
		if (LocalTagIndex == NULL)
		{
			// create a new one now
			LocalTagIndex = ConstructObject<UContentTagIndex>(UContentTagIndex::StaticClass(),Pkg,FName(TEXT("TagIndex")),RF_Public|RF_Standalone);
			// invalidate the version so that the ref is properly merged regardless
			LocalTagIndex->VersionInfo = -1;
		}
		// and update from the ref
		LocalTagIndex->MergeFromRefContentTagIndex();
		// filter out any duplicate references within a single tag
		INT NumDuplicates = 0;
		for (TArray<FTagInfo>::TIterator TagInfoIter(LocalTagIndex->Tags); TagInfoIter; ++TagInfoIter)
		{
			FTagInfo &TagInfo = *TagInfoIter;
			for (TArray<FAssetReferences>::TIterator AssetIter(TagInfo.Assets); AssetIter; ++AssetIter)
			{
				TArray<FString> &Assets = (*AssetIter).AssetReference;
				for (INT Idx = 0; Idx < Assets.Num(); Idx++)
				{
					FString &Asset = Assets(Idx);
					for (INT SubIdx = Idx + 1; SubIdx < Assets.Num(); SubIdx++)
					{
						if (Assets(SubIdx) == Asset)
						{
							NumDuplicates++;
							Assets.Remove(SubIdx--,1);
						}
					}
				}
			}
		}
		if (NumDuplicates > 0)
		{
			debugf(TEXT("Found %d duplicates in tag index"),NumDuplicates);
		}
		// and save the updated version
		SaveLocalContentTagIndex();
	}
	return LocalTagIndex;
}

void UContentTagIndex::SaveLocalContentTagIndex()
{
	debugf(TEXT("Saving local content tag index..."));
	UContentTagIndex *LocalTagIndex = GetLocalContentTagIndex();
	if (LocalTagIndex != NULL)
	{
		FString IndexFilename = FString::Printf(TEXT("%sContent\\LocalContentTagsIndex.upk"),*appGameDir());
		SavePackage(LocalTagIndex->GetOutermost(),LocalTagIndex,0,*IndexFilename,GWarn);
		debugf(TEXT("...finished!"));
	}
}

// end

