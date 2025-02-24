/*=============================================================================
	UnWorld.cpp: UWorld implementation
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineDecalClasses.h"
#include "UnNet.h"
#include "EngineSequenceClasses.h"
#include "UnStatChart.h"
#include "UnPath.h"
#include "EngineAudioDeviceClasses.h"
#include "EnginePhysicsClasses.h"
#include "EngineAnimClasses.h"
#include "DemoRecording.h"
#include "EngineUserInterfaceClasses.h"
#include "PerfMem.h"
#include "Database.h"

/*-----------------------------------------------------------------------------
	UWorld implementation.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(UWorld);

/** Global world pointer */
UWorld* GWorld = NULL;
extern FParticleDataManager	GParticleDataManager;

/**
 * UWorld constructor called at game startup and when creating a new world in the Editor.
 * Please note that this constructor does NOT get called when a world is loaded from disk.
 *
 * @param	InURL	URL associated with this world.
 */
UWorld::UWorld( const FURL& InURL )
:	URL(InURL)
{
	SetFlags( RF_Transactional );
}

/**
 * Static constructor, called once during static initialization of global variables for native 
 * classes. Used to e.g. register object references for native- only classes required for realtime
 * garbage collection or to associate UProperties.
 */
void UWorld::StaticConstructor()
{
	UClass* TheClass = GetClass();
	TheClass->EmitObjectReference( STRUCT_OFFSET( UWorld, PersistentLevel ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UWorld, PersistentFaceFXAnimSet ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UWorld, SaveGameSummary ) );
	TheClass->EmitObjectArrayReference( STRUCT_OFFSET( UWorld, Levels ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UWorld, CurrentLevel ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UWorld, CurrentLevelPendingVisibility ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UWorld, NetDriver ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UWorld, DemoRecDriver ) );
	TheClass->EmitObjectArrayReference( STRUCT_OFFSET( UWorld, NewlySpawned ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UWorld, LineBatcher ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UWorld, PersistentLineBatcher ) );
	TheClass->EmitObjectArrayReference( STRUCT_OFFSET( UWorld, ExtraReferencedObjects ) );
	TheClass->EmitObjectArrayReference( STRUCT_OFFSET( UWorld, BodyInstancePool ) );
	TheClass->EmitObjectArrayReference( STRUCT_OFFSET( UWorld, ConstraintInstancePool ) );
}

/**
 * Serialize function.
 *
 * @param Ar	Archive to use for serialization
 */
void UWorld::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );

	Ar << PersistentLevel;
	if (Ar.Ver() >= VER_WORLD_PERSISTENT_FACEFXANIMSET)
	{
		Ar << PersistentFaceFXAnimSet;
	}

	Ar << EditorViews[0];
	Ar << EditorViews[1];
	Ar << EditorViews[2];
	Ar << EditorViews[3];

	Ar << SaveGameSummary;

	if (Ar.Ver() < VER_REMOVED_DECAL_MANAGER_FROM_UWORLD)
	{
		UObject* DecalManager;
		Ar << DecalManager;
	}

	if( !Ar.IsLoading() && !Ar.IsSaving() )
	{
		Ar << Levels;
		Ar << CurrentLevel;
		Ar << URL;

		Ar << NetDriver;
		Ar << DemoRecDriver;
		
		Ar << LineBatcher;
		Ar << PersistentLineBatcher;

		Ar << BodyInstancePool;
		Ar << ConstraintInstancePool;
	}

	Ar << ExtraReferencedObjects;

	// Mark archive and package as containing a map if we're serializing to disk.
	if( !HasAnyFlags( RF_ClassDefaultObject ) && Ar.IsPersistent() )
	{
		Ar.ThisContainsMap();
		GetOutermost()->ThisContainsMap();
	}
}

/**
 * Destroy function, cleaning up world components, delete octree, physics scene, ....
 */
void UWorld::FinishDestroy()
{
	// Avoid cleanup if the world hasn't been initialized. E.g. the default object or a world object that got loaded
	// due to level streaming.
	if( bIsWorldInitialized )
	{
		// Delete octree.
		delete Hash;
		Hash = NULL;

		// Delete navigation octree.
		delete NavigationOctree;
		NavigationOctree = NULL;

		// Release scene.
		Scene->Release();
		Scene = NULL;
	}
	else
	{
		check(Hash==NULL);
	}

	// Free crowd octree
	DeleteCrowdOctree();

	Super::FinishDestroy();
}

/**
 * Called after the object has been serialized. Currently ensures that CurrentLevel gets initialized as
 * it is required for saving secondary levels in the multi- level editing case.
 */
void UWorld::PostLoad()
{
	Super::PostLoad();
	CurrentLevel = PersistentLevel;
}

/**
 * Called from within SavePackage on the passed in base/ root. The return value of this function will be passed to
 * PostSaveRoot. This is used to allow objects used as base to perform required actions before saving and cleanup
 * afterwards.
 * @param Filename: Name of the file being saved to (includes path)
	 * @param AdditionalPackagesToCook [out] Array of other packages the Root wants to make sure are cooked when this is cooked
 *
 * @return	Whether PostSaveRoot needs to perform internal cleanup
 */
UBOOL UWorld::PreSaveRoot(const TCHAR* Filename, TArray<FString>& AdditionalPackagesToCook)
{
	// allow default gametype an opportunity to modify the WorldInfo's GameTypesSupportedOnThisMap array before we save it
	UClass* GameClass = StaticLoadClass(AGameInfo::StaticClass(), NULL, TEXT("game-ini:Engine.GameInfo.DefaultGame"), NULL, LOAD_None, NULL);
	if (GameClass != NULL)
	{
		GameClass->GetDefaultObject<AGameInfo>()->AddSupportedGameTypes(GetWorldInfo(), Filename);
	}

	// add any streaming sublevels to the list of extra packages to cook
	AWorldInfo* WorldInfo = GetWorldInfo();
	for (INT LevelIndex = 0; LevelIndex < WorldInfo->StreamingLevels.Num(); LevelIndex++)
	{
		ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
		if (StreamingLevel)
		{
			// Load package if found.
			FString PackageFilename;
			if (GPackageFileCache->FindPackageFile(*StreamingLevel->PackageName.ToString(), NULL, PackageFilename))
			{
				AdditionalPackagesToCook.AddItem(StreamingLevel->PackageName.ToString());
			}
		}
	}

	// construct the list of objects referenced by Actors that will be destroyed on the client because they have bStatic and bNoDelete == FALSE
	WorldInfo->ClientDestroyedActorContent.Reset();
	for (INT i = 0; i < PersistentLevel->Actors.Num(); i++)
	{
		if (PersistentLevel->Actors(i) != NULL && !PersistentLevel->Actors(i)->bStatic && !PersistentLevel->Actors(i)->bNoDelete)
		{
			// add the archetype of this Actor
			UObject* Archetype = PersistentLevel->Actors(i)->GetArchetype();
			WorldInfo->ClientDestroyedActorContent.AddUniqueItem(Archetype);

			// collect the objects referenced by this Actor
			TArray<UObject*> InstanceReferencedObjects;
			{
				FArchiveObjectReferenceCollector Ar(&InstanceReferencedObjects, NULL, TRUE, FALSE, FALSE, TRUE);
				PersistentLevel->Actors(i)->Serialize(Ar);
				// second pass, go through stuff inside the Actor to get content referenced by components and such
				INT FirstPassNum = InstanceReferencedObjects.Num();
				for (INT j = 0; j < FirstPassNum; j++)
				{
					if (InstanceReferencedObjects(j)->IsIn(PersistentLevel->Actors(i)))
					{
						InstanceReferencedObjects(j)->Serialize(Ar);
					}
				}
			}

			// now do the same with the archetype
			TArray<UObject*> ArchetypeReferencedObjects;
			{
				FArchiveObjectReferenceCollector Ar(&ArchetypeReferencedObjects, NULL, TRUE, FALSE, FALSE, TRUE);
				Archetype->Serialize(Ar);
				// second pass, go through stuff inside the Actor to get content referenced by components and such
				INT FirstPassNum = ArchetypeReferencedObjects.Num();
				for (INT j = 0; j < FirstPassNum; j++)
				{
					if (ArchetypeReferencedObjects(j)->IsIn(Archetype))
					{
						ArchetypeReferencedObjects(j)->Serialize(Ar);
					}
				}
			}

			// add content not in the level and not referenced by the archetype to main array
			for (INT j = 0; j < InstanceReferencedObjects.Num(); j++)
			{
				UObject* Obj = InstanceReferencedObjects(j);
				if ( !Obj->HasAnyFlags(RF_Native | RF_Transient) && Obj->HasAnyFlags(RF_Public) && Obj != PersistentLevel &&
					!Obj->IsIn(PersistentLevel) && !Obj->IsIn(GetTransientPackage()) && !ArchetypeReferencedObjects.ContainsItem(Obj) )
				{
					WorldInfo->ClientDestroyedActorContent.AddUniqueItem(Obj);
				}
			}
		}
	}

	// Update components and keep track off whether we need to clean them up afterwards.
	UBOOL bCleanupIsRequired = PersistentLevel->bAreComponentsCurrentlyAttached == FALSE;
	PersistentLevel->UpdateComponents();
	return bCleanupIsRequired;
}

/**
 * Called from within SavePackage on the passed in base/ root. This function is being called after the package
 * has been saved and can perform cleanup.
 *
 * @param	bCleanupIsRequired	Whether PreSaveRoot dirtied state that needs to be cleaned up
 */
void UWorld::PostSaveRoot( UBOOL bCleanupIsRequired )
{
	if( bCleanupIsRequired )
	{
		PersistentLevel->ClearComponents();
	}
}

/**
 * Saves this world.  Safe to call on non-GWorld worlds.
 *
 * @param	Filename					The filename to use.
 * @param	bForceGarbageCollection		Whether to force a garbage collection before saving.
 * @param	bAutosaving					If TRUE, don't perform optional caching tasks.
 * @param	bPIESaving					If TRUE, don't perform tasks that will conflict with editor state.
 */
UBOOL UWorld::SaveWorld( const FString& Filename, UBOOL bForceGarbageCollection, UBOOL bAutosaving, UBOOL bPIESaving )
{
	check(PersistentLevel);
	check(GIsEditor);

	UBOOL bWasSuccessful = false;

	// pause propagation
	GObjectPropagator->Pause();

	// Don't bother with static mesh physics data or shrinking when only autosaving.
	if ( bAutosaving )
	{
		PersistentLevel->ClearPhysStaticMeshCache();
	}
	else
	{
		if( GIsEditor && !bPIESaving )
		{
			FString MapFileName = FFilename( Filename ).GetCleanFilename();
			const FString LocalizedSavingMap(
				*FString::Printf( LocalizeSecure( LocalizeUnrealEd( TEXT("SavingMap_F") ), *MapFileName ) ) );
			GWarn->PushStatus();
			GWarn->StatusUpdatef( -1, -1, *FString( LocalizedSavingMap + TEXT( " " ) + LocalizeUnrealEd( TEXT( "SavingMapStatus_CachingPhysStaticMeshes" ) ) ) );
		}

		PersistentLevel->BuildPhysStaticMeshCache();

		if( GIsEditor && !bPIESaving )
		{
			GWarn->PopStatus();
		}
	}

	// Shrink model and clean up deleted actors.
	// Don't do this when autosaving or PIE saving so that actor adds can still undo.
	if ( !bPIESaving && !bAutosaving )
	{
		ShrinkLevel();
	}

	// Reset actor creation times.
	for( FActorIterator It; It; ++It )
	{
		AActor* Actor = *It;
		Actor->CreationTime = 0.0f;
	}

	if( bForceGarbageCollection )
	{
		if( GIsEditor && !bPIESaving )
		{
			FString MapFileName = FFilename( Filename ).GetCleanFilename();
			const FString LocalizedSavingMap(
				*FString::Printf( LocalizeSecure( LocalizeUnrealEd( TEXT("SavingMap_F") ), *MapFileName ) ) );
			GWarn->PushStatus();
			GWarn->StatusUpdatef( -1, -1, *FString( LocalizedSavingMap + TEXT( " " ) + LocalizeUnrealEd( TEXT( "SavingMapStatus_CollectingGarbage" ) ) ) );
		}

		// NULL empty or "invalid" entries (e.g. bDeleteMe) in actors array.
		UObject::CollectGarbage( GARBAGE_COLLECTION_KEEPFLAGS );

		if( GIsEditor && !bPIESaving )
		{
			GWarn->PopStatus();
		}
	}

	// Compact and sort actors array to remove empty entries.
	// Don't do this when autosaving or PIE saving so that actor adds can still undo.
	if ( !bPIESaving && !bAutosaving )
	{
		PersistentLevel->SortActorList();
	}

	// Temporarily flag packages saved under a PIE filename as PKG_PlayInEditor for serialization so loading
	// them will have the flag set. We need to undo this as the object flagged isn't actually the PIE package, 
	// but rather only the loaded one will be.
	UPackage*	WorldPackage			= GetOutermost();
	DWORD		OriginalPIEFlagValue	= WorldPackage->PackageFlags & PKG_PlayInEditor;
	// PIE prefix detected, mark package.
	if( Filename.InStr( PLAYWORLD_PACKAGE_PREFIX ) != INDEX_NONE )
	{
		WorldPackage->PackageFlags |= PKG_PlayInEditor;
	}

	// Save package.
	const UBOOL bWarnOfLongFilename = !(bAutosaving | bPIESaving);
	bWasSuccessful = SavePackage(WorldPackage, this, 0, *Filename, GWarn, NULL, FALSE, bWarnOfLongFilename);
	if (!bWasSuccessful)
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Error_CouldntSavePackage") );
	}
	
	// Restore original value of PKG_PlayInEditor.
	WorldPackage->PackageFlags &= ~PKG_PlayInEditor;
	WorldPackage->PackageFlags |= OriginalPIEFlagValue;

	// Empty the static mesh physics cache if necessary.
	if ( !bAutosaving )
	{
		PersistentLevel->ClearPhysStaticMeshCache();
	}

	// resume propagation
	GObjectPropagator->Unpause();

	return bWasSuccessful;
}

/**
 * Initializes the world, associates the persistent level and sets the proper zones.
 */
void UWorld::Init()
{
	if( PersistentLevel->GetOuter() != this )
	{
		// Move persistent level into world so the world object won't get garbage collected in the multi- level
		// case as it is still referenced via the level's outer. This is required for multi- level editing to work.
		PersistentLevel->Rename( *PersistentLevel->GetName(), this );
	}

	// Allocate the world's hash, navigation octree and scene.
	Hash				= new FPrimitiveOctree();
	NavigationOctree	= new FNavigationOctree();
	Scene				= AllocateScene( this, FALSE, TRUE );

	URL					= PersistentLevel->URL;
	CurrentLevel		= PersistentLevel;

	// See whether we're missing the default brush. It was possible in earlier builds to accidentally delete the default
	// brush of sublevels so we simply spawn a new one if we encounter it missing.
	ABrush* DefaultBrush = PersistentLevel->Actors.Num()<2 ? NULL : Cast<ABrush>(PersistentLevel->Actors(1));
	if( GIsEditor && (!DefaultBrush || !DefaultBrush->IsStaticBrush() || !DefaultBrush->CsgOper==CSG_Active || !DefaultBrush->BrushComponent || !DefaultBrush->Brush) )
	{
		debugf( TEXT("Encountered missing default brush - spawning new one") );

		// Spawn the default brush.
		DefaultBrush = SpawnBrush();
		check(DefaultBrush->BrushComponent);
		DefaultBrush->Brush = new( GetOuter(), TEXT("Brush") )UModel( DefaultBrush, 1 );
		DefaultBrush->BrushComponent->Brush = DefaultBrush->Brush;
		DefaultBrush->SetFlags( RF_NotForClient | RF_NotForServer | RF_Transactional );
		DefaultBrush->Brush->SetFlags( RF_NotForClient | RF_NotForServer | RF_Transactional );
		DefaultBrush->Brush->Polys->SetFlags( RF_NotForClient | RF_NotForServer | RF_Transactional );

		// Find the index in the array the default brush has been spawned at. Not necessarily
		// the last index as the code might spawn the default physics volume afterwards.
		const INT DefaultBrushActorIndex = PersistentLevel->Actors.FindItemIndex( DefaultBrush );

		// The default brush needs to reside at index 1.
		Exchange(PersistentLevel->Actors(1),PersistentLevel->Actors(DefaultBrushActorIndex));

		// Re-sort actor list as we just shuffled things around.
		PersistentLevel->SortActorList();
	}

	Levels.Empty(1);
	Levels.AddItem( PersistentLevel );

	AWorldInfo* WorldInfo = GetWorldInfo();
	for( INT ActorIndex=0; ActorIndex<PersistentLevel->Actors.Num(); ActorIndex++ )
	{
		AActor* Actor = PersistentLevel->Actors(ActorIndex);
		if( Actor )
		{
			// We can't do this in PostLoad as GetWorldInfo() will point to the old WorldInfo.
			Actor->WorldInfo = WorldInfo;
			Actor->SetZone( 0, 1 );
		}
	}

	// If in the editor, load secondary levels.
	if( GIsEditor )
	{
		WorldInfo->LoadSecondaryLevels();
	}

	// update it's bIsMenuLevel
	WorldInfo->bIsMenuLevel = (FFilename(GetMapName()).GetBaseFilename() == FFilename(FURL::DefaultLocalMap).GetBaseFilename());

	// Find the PersistentFaceFXAnimSet and set it
	FindAndSetPersistentFaceFXAnimSet();

	// We're initialized now.
	bIsWorldInitialized = TRUE;
}

/**
 * Static function that creates a new UWorld and replaces GWorld with it.
 */
void UWorld::CreateNew()
{
	// Clean up existing world and remove it from root set so it can be garbage collected.
	if( GWorld )
	{
		GWorld->FlushLevelStreaming( NULL, TRUE );
		GWorld->TermWorldRBPhys();
		GWorld->CleanupWorld();
		GWorld->RemoveFromRoot();
		GWorld = NULL;
	}

	// Create a new package unless we're a commandlet in which case we keep the dummy world in the transient package.
	UPackage*	WorldPackage		= GIsUCC ? UObject::GetTransientPackage() : CreatePackage( NULL, NULL );

	// Mark the package as containing a world.  This has to happen here rather than at serialization time,
	// so that e.g. the referenced assets browser will work correctly.
	if ( WorldPackage != UObject::GetTransientPackage() )
	{
		WorldPackage->ThisContainsMap();
	}

	// Create new UWorld, ULevel and UModel.
	GWorld							= new( WorldPackage				, TEXT("TheWorld")			) UWorld(FURL(NULL));
	GWorld->PersistentLevel			= new( GWorld					, TEXT("PersistentLevel")	) ULevel(FURL(NULL));
	GWorld->PersistentLevel->Model	= new( GWorld->PersistentLevel								) UModel( NULL, 1 );

	// Mark objects are transactional for undo/ redo.
	GWorld->SetFlags( RF_Transactional );
	GWorld->PersistentLevel->SetFlags( RF_Transactional );
	GWorld->PersistentLevel->Model->SetFlags( RF_Transactional );
	
	// Need to associate current level so SpawnActor doesn't complain.
	GWorld->CurrentLevel			= GWorld->PersistentLevel;

	// Spawn the level info.
	UClass* WorldInfoClass = StaticLoadClass( AWorldInfo::StaticClass(), AWorldInfo::StaticClass()->GetOuter(), TEXT("WorldInfo"), NULL, LOAD_None, NULL);
	check(WorldInfoClass);

	GWorld->SpawnActor( WorldInfoClass );
	check(GWorld->GetWorldInfo());

	// Initialize.
	GWorld->Init();

	// Update components.
	GWorld->UpdateComponents( FALSE );

	// Add to root set so it doesn't get garbage collected.
	GWorld->AddToRoot();
}

/**
 * Removes the passed in actor from the actor lists. Please note that the code actually doesn't physically remove the
 * index but rather clears it so other indices are still valid and the actors array size doesn't change.
 *
 * @param	Actor					Actor to remove.
 * @param	bShouldModifyLevel		If TRUE, Modify() the level before removing the actor if in the editor.
 * @return							Number of actors array entries cleared.
 */
void UWorld::RemoveActor(AActor* Actor, UBOOL bShouldModifyLevel)
{
	UBOOL	bSuccessfulRemoval	= FALSE;
	ULevel* CheckLevel			= Actor->GetLevel();
	
	if (HasBegunPlay())
	{
		// If we're in game, then only search dynamic actors.
		for (INT ActorIdx = CheckLevel->iFirstDynamicActor; ActorIdx < CheckLevel->Actors.Num(); ActorIdx++)
		{
			if (CheckLevel->Actors(ActorIdx) == Actor)
			{
				CheckLevel->Actors(ActorIdx)	= NULL;
				bSuccessfulRemoval				= TRUE;
				break;
			}
		}
	}
	else
	{
		// Otherwise search the entire list.
		for (INT ActorIdx = 0; ActorIdx < CheckLevel->Actors.Num(); ActorIdx++)
		{
			if (CheckLevel->Actors(ActorIdx) == Actor)
			{
				if ( bShouldModifyLevel && GUndo )
				{
					ModifyLevel( CheckLevel );
				}
				CheckLevel->Actors.ModifyItem(ActorIdx);
				CheckLevel->Actors(ActorIdx)	= NULL;
				bSuccessfulRemoval				= TRUE;
				break;
			}
		}
	}

	check(bSuccessfulRemoval);
}

/**
 * Returns whether the passed in actor is part of any of the loaded levels actors array.
 *
 * @param	Actor	Actor to check whether it is contained by any level
 *	
 * @return	TRUE if actor is contained by any of the loaded levels, FALSE otherwise
 */
UBOOL UWorld::ContainsActor( AActor* Actor )
{
	for( INT LevelIndex=0; LevelIndex<Levels.Num(); LevelIndex++ )
	{
		ULevel* Level = Levels(LevelIndex);
		if( Level->Actors.ContainsItem( Actor ) )
		{
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Returns whether audio playback is allowed for this scene.
 *
 * @return TRUE if current world is GWorld, FALSE otherwise
 */
UBOOL UWorld::AllowAudioPlayback()
{
	return GWorld == this;
}

void UWorld::NotifyProgress( EProgressMessageType MessageType, const FString& Title, const FString& Message )
{
	GEngine->SetProgress( MessageType, Title, Message );
}

void UWorld::ShrinkLevel()
{
	GetModel()->ShrinkModel();
}

/**
 * Clears all level components and world components like e.g. line batcher.
 */
void UWorld::ClearComponents()
{
	GParticleDataManager.Clear();

	for( INT LevelIndex=0; LevelIndex<Levels.Num(); LevelIndex++ )
	{
		ULevel* Level = Levels(LevelIndex);
		Level->ClearComponents();
	}

	if(LineBatcher)
	{
		LineBatcher->ConditionalDetach();
	}
	if(PersistentLineBatcher)
	{
		PersistentLineBatcher->ConditionalDetach();
	}
}

/**
 * Updates world components like e.g. line batcher and all level components.
 *
 * @param	bCurrentLevelOnly		If TRUE, update only the current level.
 */
void UWorld::UpdateComponents(UBOOL bCurrentLevelOnly)
{
	if(!LineBatcher)
	{
		LineBatcher = ConstructObject<ULineBatchComponent>(ULineBatchComponent::StaticClass());
	}

	if( LineBatcher->BatchedLines.Num() > 0 ) 
	{	
		LineBatcher->ConditionalDetach();
		LineBatcher->ConditionalAttach(Scene,NULL,FMatrix::Identity);
	}

	if(!PersistentLineBatcher)
	{
		PersistentLineBatcher = ConstructObject<ULineBatchComponent>(ULineBatchComponent::StaticClass());
	}

	if( PersistentLineBatcher->BatchedLines.Num() > 0 ) 
	{
		PersistentLineBatcher->ConditionalDetach();
		PersistentLineBatcher->ConditionalAttach(Scene,NULL,FMatrix::Identity);
	}

	if ( bCurrentLevelOnly )
	{
		check( CurrentLevel );
		CurrentLevel->UpdateComponents();
	}
	else
	{
		for( INT LevelIndex=0; LevelIndex<Levels.Num(); LevelIndex++ )
		{
			ULevel* Level = Levels(LevelIndex);
			Level->UpdateComponents();
		}
	}
}

/**
 * Updates all cull distance volumes.
 */
void UWorld::UpdateCullDistanceVolumes()
{
	// Keep track of time spent.
	DOUBLE Duration = 0;
	{
		SCOPE_SECONDS_COUNTER(Duration);

		// Global reattach/ update context to propagate cull distance changes. This cannot be done on the individual
		// component level as we might have changed the component to no longer be affectable by volumes.
		FGlobalPrimitiveSceneAttachmentContext PropagatePrimitiveChanges;

		// Reset cached cull distance for all primitive components affectable by cull distance volumes.
		for( TObjectIterator<UPrimitiveComponent> It; It; ++It )
		{
			UPrimitiveComponent*	PrimitiveComponent	= *It;
	
			// Check whether primitive can be affected by cull distance volumes and reset the cached cull
			// distance to the level designer specified value so we have a clear baseline before iterating
			// over all volumes.
			if(	ACullDistanceVolume::CanBeAffectedByVolumes( PrimitiveComponent ) )
			{
				PrimitiveComponent->CachedMaxDrawDistance = PrimitiveComponent->LDMaxDrawDistance;
			}
		}

		// Iterate over all cull distance volumes and have them update associated primitives.
		for( FActorIterator It; It; ++It )
		{
			ACullDistanceVolume* CullDistanceVolume = Cast<ACullDistanceVolume>(*It);
			if( CullDistanceVolume )
			{
				CullDistanceVolume->UpdateVolume();
			}
		}
	}
	if( Duration > 1.f )
	{
		debugf(TEXT("Updating cull distance volumes took %5.2f seconds"),Duration);
	}
}

/**
 * Modifies all levels.
 *
 * @param	bAlwaysMarkDirty	if TRUE, marks the package dirty even if we aren't
 *								currently recording an active undo/redo transaction
 */
void UWorld::Modify( UBOOL bAlwaysMarkDirty/*=FALSE*/ )
{
	appErrorf( TEXT("UWorldModify is deprecated!") );
}

/**
 * Transacts the specified level -- the correct way to modify a level
 * as opposed to calling Level->Modify.
 */
void UWorld::ModifyLevel(ULevel* Level)
{
	if( Level )
	{
		Level->Modify( FALSE );
		check( Level->HasAnyFlags(RF_Transactional) );
		//Level->Actors.ModifyAllItems();
		Level->Model->Modify( FALSE );
	}
}

/**
 * Invalidates the cached data used to render the levels' UModel.
 *
 * @param	bCurrentLevelOnly		If TRUE, affect only the current level.
 */
void UWorld::InvalidateModelGeometry(UBOOL bCurrentLevelOnly)
{
	if ( bCurrentLevelOnly )
	{
		check( bCurrentLevelOnly );
		CurrentLevel->InvalidateModelGeometry();
	}
	else
	{
		for( INT LevelIndex=0; LevelIndex<Levels.Num(); LevelIndex++ )
		{
			ULevel* Level = Levels(LevelIndex);
			Level->InvalidateModelGeometry();
		}
	}
}

/**
 * Discards the cached data used to render the levels' UModel.  Assumes that the
 * faces and vertex positions haven't changed, only the applied materials.
 *
 * @param	bCurrentLevelOnly		If TRUE, affect only the current level.
 */
void UWorld::InvalidateModelSurface(UBOOL bCurrentLevelOnly)
{
	if ( bCurrentLevelOnly )
	{
		check( bCurrentLevelOnly );
		CurrentLevel->InvalidateModelSurface();
	}
	else
	{
		for( INT LevelIndex=0; LevelIndex<Levels.Num(); LevelIndex++ )
		{
			ULevel* Level = Levels(LevelIndex);
			Level->InvalidateModelSurface();
		}
	}
}

/**
 * Commits changes made to the surfaces of the UModels of all levels.
 */
void UWorld::CommitModelSurfaces()
{
	for( INT LevelIndex=0; LevelIndex<Levels.Num(); LevelIndex++ )
	{
		ULevel* Level = Levels(LevelIndex);
		Level->CommitModelSurfaces();
	}
}

void DebugPrintCrossLevelRefInfo( UWorld* World )
{
	for( INT LevelIdx = 0; LevelIdx < World->Levels.Num(); LevelIdx++ )
	{
		ULevel* Level = World->Levels(LevelIdx);
		debugf(TEXT("Level %s has..."), *Level->GetName());
		debugf(TEXT("\t\t\t %d CrossLevelActors"), Level->CrossLevelActors.Num() );
//		debugf(TEXT("\t\t\t %d CrossLevelTargets"), Level->CrossLevelTargets.Num() );
	}
}

/**
 * Fixes up any cross level paths. Called from e.g. UpdateLevelStreaming when associating a new level with the world.
 *
 * @param	bIsRemovingLevel	Whether we are adding or removing a level
 * @param	Level				Level to add or remove
 */
void UWorld::FixupCrossLevelRefs( UBOOL bIsRemovingLevel, ULevel* Level )
{
	if (Level->HasPathNodes())
	{
		// tell controllers to clear any cross level path refs they have
		for (AController *Controller = GetFirstController(); Controller != NULL; Controller = Controller->NextController)
		{
			if (!Controller->IsPendingKill() && !Controller->bDeleteMe)
			{
				Controller->ClearCrossLevelPaths(Level);
			}
		}

		// grab a list of all actor ref's
		TArray<FActorReference*> ActorRefs;
		for( INT LevelIdx = 0; LevelIdx < Levels.Num(); LevelIdx++ )
		{
			ULevel *ChkLevel = Levels(LevelIdx);
			for (INT Idx = 0; Idx < ChkLevel->CrossLevelActors.Num(); Idx++)
			{
				AActor *Actor = ChkLevel->CrossLevelActors(Idx);
				if( Actor != NULL )
				{
					//INT PrevNum = ActorRefs.Num();
					Actor->GetActorReferences(ActorRefs,bIsRemovingLevel);
					//debugf(TEXT("%s added %i references!"),*Actor->GetName(),ActorRefs.Num() - PrevNum);
				}
			}
		}

		//debugf(TEXT("%i total cross level actors after getactorreferences!"),ActorRefs.Num());
#if 0
		DebugPrintCrossLevelRefInfo( this );
#endif
		// if removing the level then just null all ref's to actors in level being unloaded
		if (bIsRemovingLevel)
		{
			for (INT Idx = 0; Idx < ActorRefs.Num(); Idx++)
			{
				AActor *Actor = ActorRefs(Idx)->Actor;
				if( Actor != NULL && Actor->IsInLevel( Level ) )
				{
					ActorRefs(Idx)->Actor = NULL;
				}
			}
			// remove the level's nav list from the world
			RemoveLevelNavList( Level, TRUE );
			// for each nav in the level nav list
			for( ANavigationPoint *Nav = Level->NavListStart; Nav != NULL; Nav = Nav->nextNavigationPoint )
			{
				// remove the nav from the octree
				Nav->RemoveFromNavigationOctree();
			}
		}
		else
		{
#if !FINAL_RELEASE
			DWORD Time =0;
			CLOCK_CYCLES(Time);
			INT Total = ActorRefs.Num();
#endif

			TMap<FGuid, AActor*> GuidHash;
			#define MAX_GUID_HASH_SIZE 5000

			FActorIterator It;
			while(It)
			{
				// limit the size of the hash table to keep memory usage down
				INT CountThisitt = 0;
				GuidHash.Empty(MAX_GUID_HASH_SIZE+8);
				for( ; It && CountThisitt < MAX_GUID_HASH_SIZE; ++It ) 
				{
					AActor* Actor = *It;
					FGuid* Guid = Actor->GetGuid();
					if (Guid != NULL && Guid->IsValid())
					{
						CountThisitt++;
						GuidHash.Set(*Guid,Actor);
					}
				}

				for (INT Idx = ActorRefs.Num()-1; Idx >=0; Idx--)
				{
					AActor** FoundActor = GuidHash.Find(ActorRefs(Idx)->Guid);
					if(FoundActor != NULL)
					{
						// assign the actor
						ActorRefs(Idx)->Actor = *FoundActor;
						ActorRefs.RemoveSwap(Idx,1);
					}
				}

			}
#if !FINAL_RELEASE
			UNCLOCK_CYCLES(Time);
			debugf(TEXT("%s has %i cross-level refs! took %0.3f ms to fixup refs! Couldn't match %i(%0.2f%%)"),*Level->GetOutermost()->GetName(),Total,Time *GSecondsPerCycle * 1000.f,ActorRefs.Num(),((FLOAT)ActorRefs.Num()/(FLOAT)Total)*100.f);
#endif

		}

#if !FINAL_RELEASE
		if (GIsGame && !bIsRemovingLevel)
		{
			// check to see if show paths/cover is enabled
			if (GEngine->GameViewport != NULL && (GEngine->GameViewport->ShowFlags & SHOW_Paths))
			{
				GEngine->GameViewport->ShowFlags &= ~SHOW_Paths;
				GEngine->GameViewport->Exec(TEXT("SHOW PATHS"),*GLog);
			}
		}
#endif
	}
}

/** verifies that the navlist pointers have not been corrupted.
 *	helps track down errors w/ dynamic pathnodes being added/removed from the navigation list
 */
void UWorld::VerifyNavList( const TCHAR* DebugTxt, ANavigationPoint* IngoreNav )
{
	INT ListErrorCount = 0;
	// for all navigation points
	for (FActorIterator It; It; ++It)
	{
		ANavigationPoint *Nav = Cast<ANavigationPoint>(*It);
		if( Nav == NULL || Nav == IngoreNav || appStricmp( *Nav->GetClass()->GetName(), TEXT("FauxPathNode") ) == 0 )
		{
			continue;
		}
		if (Nav->nextOrdered != NULL ||
			Nav->prevOrdered != NULL ||
			Nav->previousPath	 != NULL ||
			Nav->bAlreadyVisited == TRUE )
		{
			debugf(NAME_Warning,TEXT("%s has transient properties that haven't been cleared!"),*Nav->GetPathName());
			ListErrorCount++;
		}
		UBOOL bInList = FALSE;
		// validate it is in the world list
		for (ANavigationPoint *TestNav = GWorld->GetFirstNavigationPoint(); TestNav != NULL; TestNav = TestNav->nextNavigationPoint)
		{
			if (Nav == TestNav)
			{
				bInList = TRUE;
				break;
			}
		}
		if (!bInList)
		{
			debugf(NAME_Warning,TEXT("%s is not in the nav list!"),*Nav->GetPathName());
			ListErrorCount++;
		}
		else
		if( Nav->IsPendingKill() )
		{
			debugf( NAME_Warning,TEXT("%s is in the nav list but about to be GC'd!!"), *Nav->GetPathName() );
			ListErrorCount++;
		}

		// validate cover list as well
		ACoverLink *TestLink = Cast<ACoverLink>(Nav);
		if (TestLink != NULL)
		{
			bInList = FALSE;
			for (ACoverLink *Link = GWorld->GetWorldInfo()->CoverList; Link != NULL; Link = Link->NextCoverLink)
			{
				if (Link == TestLink)
				{
					bInList = TRUE;
					break;
				}
			}
			if (!bInList)
			{
				debugf(NAME_Warning,TEXT("%s is not in the cover list!"),*TestLink->GetPathName());
				ListErrorCount++;
			}
			else
			if( TestLink->IsPendingKill() )
			{
				debugf( NAME_Warning,TEXT("%s is in the nav list but about to be GC'd!!"), *TestLink->GetPathName());
				ListErrorCount++;
			}
		}
	}
	if (ListErrorCount != 0)
	{
		debugf( DebugTxt );
		debugf(NAME_Warning,TEXT("%d errors found, currently loaded levels:"),ListErrorCount);
		for (INT Idx = 0; Idx < GWorld->Levels.Num(); Idx++)
		{
			debugf(NAME_Warning,TEXT("- %s"),*GWorld->Levels(Idx)->GetPathName());
		}
	}
}


DECLARE_CYCLE_STAT(TEXT("AddToWorld Time"),STAT_AddToWorldTime,STATGROUP_Streaming);
DECLARE_CYCLE_STAT(TEXT("RemoveFromWorld Time"),STAT_RemoveFromWorldTime,STATGROUP_Streaming);
DECLARE_CYCLE_STAT(TEXT("UpdateLevelStreming Time"),STAT_UpdateLevelStreamingTime,STATGROUP_Streaming);

/**
 * Static helper function for AddToWorld to determine whether we've already spent all the allotted time.
 *
 * @param	CurrentTask		Description of last task performed
 * @param	StartTime		StartTime, used to calculate time passed
 * @param	Level			Level work has been performed on
 *
 * @return TRUE if time limit has been exceeded, FALSE otherwise
 */
static UBOOL IsTimeLimitExceeded( const TCHAR* CurrentTask, DOUBLE StartTime, ULevel* Level )
{
	UBOOL bIsTimeLimitExceed = FALSE;
	// We don't spread work across several frames in the Editor to avoid potential side effects.
	if( GIsEditor == FALSE )
	{
		DOUBLE CurrentTime	= appSeconds();
		// Delta time in ms.
		DOUBLE DeltaTime	= (CurrentTime - StartTime) * 1000;
		if( DeltaTime > 5 )
		{
			// Log if a single event took way too much time.
			if( DeltaTime > 20 )
			{
				debugfSuppressed( NAME_DevStreaming, TEXT("UWorld::AddToWorld: %s for %s took (less than) %5.2f ms"), CurrentTask, *Level->GetOutermost()->GetName(), DeltaTime );
			}
			bIsTimeLimitExceed = TRUE;
		}
	}
	return bIsTimeLimitExceed;
}

extern UBOOL GPrecacheNextFrame;

/**
 * Associates the passed in level with the world. The work to make the level visible is spread across several frames and this
 * function has to be called till it returns TRUE for the level to be visible/ associated with the world and no longer be in
 * a limbo state.
 *
 * @param StreamingLevel	Level streaming object whose level we should add
 * @param RelativeOffset	Relative offset to move actors
 */
void UWorld::AddToWorld( ULevelStreaming* StreamingLevel, const FVector& RelativeOffset )
{
	SCOPE_CYCLE_COUNTER(STAT_AddToWorldTime);
	check(StreamingLevel);
	
	ULevel* Level = StreamingLevel->LoadedLevel;
	check(Level);
	check(!Level->IsPendingKill());
	check(!Level->HasAnyFlags(RF_Unreachable));
	check(!StreamingLevel->bIsVisible);

	// Set flags to indicate that we are associating a level with the world to e.g. perform slower/ better octree insertion 
	// and such, as opposed to the fast path taken for run-time/ gameplay objects.
	GIsAssociatingLevel					= TRUE;
	DOUBLE	StartTime					= appSeconds();
	UBOOL	bExecuteNextStep			= (CurrentLevelPendingVisibility == Level) || CurrentLevelPendingVisibility == NULL;
	UBOOL	bPerformedLastStep			= FALSE;
	// We're not done yet, aka we have a pending request in flight.
	Level->bHasVisibilityRequestPending	= TRUE;

	if( bExecuteNextStep && !Level->bAlreadyMovedActors )
	{
		// Mark level as being the one in process of being made visible.
		CurrentLevelPendingVisibility = Level;

		// We're adding the level to the world so we implicitly have a request pending.
		Level->bHasVisibilityRequestPending = TRUE;

		// Add to the UWorld's array of levels, which causes it to be rendered et al.
		Levels.AddUniqueItem( Level );

		// Don't bother moving if there isn't anything to do.
		UBOOL		bMoveActors	= RelativeOffset.Size() > KINDA_SMALL_NUMBER;
		AWorldInfo*	WorldInfo	= GetWorldInfo();
		// Iterate over all actors int he level and move them if necessary, associate them with the right world info, set the
		// proper zone and clear their Kismet events.
		for( INT ActorIndex=0; ActorIndex<Level->Actors.Num(); ActorIndex++ )
		{
			AActor* Actor = Level->Actors(ActorIndex);
			if( Actor )
			{
				// Shift actors by specified offset.
				if( bMoveActors )
				{
					Actor->Location += RelativeOffset;
					Actor->PostEditMove( TRUE );
				}

				// Associate with persistent world info actor and update zoning.
				Actor->WorldInfo = WorldInfo;
				Actor->SetZone( 0, 1 );

				// Clear any events for Kismet as well
				if( GIsGame )
				{
					Actor->GeneratedEvents.Empty();
				}
			}
		}
	
		// We've moved actors so mark the level package as being dirty.
		if( bMoveActors )
		{
			Level->MarkPackageDirty();
		}

		Level->bAlreadyMovedActors = TRUE;
		bExecuteNextStep = !IsTimeLimitExceeded( TEXT("moving actors"), StartTime, Level );
	}

	// Updates the level components (Actor components and UModelComponents).
	if( bExecuteNextStep && !Level->bAlreadyUpdatedComponents )
	{
		// Make sure code thinks components are not currently attached.
		Level->bAreComponentsCurrentlyAttached = FALSE;

		// Incrementally update components.
		do
		{
			Level->IncrementalUpdateComponents( (GIsEditor || GIsUCC) ? 0 : 50 );
		}
		while( !IsTimeLimitExceeded( TEXT("updating components"), StartTime, Level ) && !Level->bAreComponentsCurrentlyAttached );

		// We are done once all components are attached.
		Level->bAlreadyUpdatedComponents	= Level->bAreComponentsCurrentlyAttached;
		bExecuteNextStep					= Level->bAreComponentsCurrentlyAttached;
	}

	if( GIsGame )
	{
		// Initialize physics for level BSP mesh.
		if( bExecuteNextStep && !Level->bAlreadyCreateBSPPhysMesh )
		{
			Level->InitLevelBSPPhysMesh();
			Level->bAlreadyCreateBSPPhysMesh = TRUE;
			bExecuteNextStep = !IsTimeLimitExceeded( TEXT("initializing level bsp physics mesh"), StartTime, Level );
		}

		// Initialize physics for each actor.
		if( bExecuteNextStep && !Level->bAlreadyInitializedAllActorRBPhys )
		{
			// Incrementally initialize physics for actors.
			do
			{
				// This function will set bAlreadyInitializedAllActorRBPhys when it does the final Actor.
				Level->IncrementalInitActorsRBPhys( (GIsEditor || GIsUCC) ? 0 : 50 );
			}
			while( !IsTimeLimitExceeded( TEXT("initializing actor physics"), StartTime, Level ) && !Level->bAlreadyInitializedAllActorRBPhys );

			// Go on to next step if we are done here.
			bExecuteNextStep = Level->bAlreadyInitializedAllActorRBPhys;
		}

		// send a callback that a level was added to the world
		GCallbackEvent->Send(CALLBACK_LevelAddedToWorld, Level);

		// Initialize all actors and start execution.
		if( bExecuteNextStep && !Level->bAlreadyInitializedActors )
		{
			Level->InitializeActors();
			Level->bAlreadyInitializedActors = TRUE;
			bExecuteNextStep = !IsTimeLimitExceeded( TEXT("initializing actors"), StartTime, Level );
		}

		// Route various begin play functions and set volumes.
		if( bExecuteNextStep && !Level->bAlreadyRoutedActorBeginPlay )
		{
			AWorldInfo* Info = GetWorldInfo();
			Info->bStartup = 1;
			Level->RouteBeginPlay();
			Level->bAlreadyRoutedActorBeginPlay = TRUE;
			Info->bStartup = 0;

			bExecuteNextStep = !IsTimeLimitExceeded( TEXT("routing BeginPlay on actors"), StartTime, Level );
		}

		// Fixup any cross level paths.
		if( bExecuteNextStep && !Level->bAlreadyFixedUpCrossLevelRefs )
		{
			FixupCrossLevelRefs( FALSE, Level );
			Level->bAlreadyFixedUpCrossLevelRefs = TRUE;
			bExecuteNextStep = !IsTimeLimitExceeded( TEXT("fixing up cross-level paths"), StartTime, Level );
		}

		// Handle kismet scripts, if there are any.
		if( bExecuteNextStep && !Level->bAlreadyRoutedSequenceBeginPlay )
		{
			// find the parent persistent level to add this as the parent 
			USequence* RootSequence = PersistentLevel->GetGameSequence();
			if( RootSequence )
			{
				// at runtime, we could have multiple FakePersistentLevels and one of them could be the
				// parent sequence for this loaded object.
				// Instead of this level hierarchy:
				//     HappyLevel_P
				//       \-> HappySubLevel1
				//       \-> HappySubLevel2
				//     SadLevel_P
				//       \-> SadSubLevel1
				//       \-> SadSubLevel2
				// We could have this:
				//     MyGame_P
				//       \-> HappyLevel_P
				//       \-> HappySubLevel1
				//       \-> HappySubLevel2
				//       \-> SadLevel_P
				//       \-> SadSubLevel1
				//       \-> SadSubLevel2
				//
				// HappySubLevel1 needs HappyLevel_P's GameSequence to be the RootSequence for it's sequences, etc
				if (GIsGame)
				{
					// loop over all the loaded World objects, looking to find the one that has this level 
					// in it's streaming level list
					AWorldInfo* WorldInfo = GetWorldInfo();
					// get package name for the loaded level
					FName LevelPackageName = Level->GetOutermost()->GetFName();
					UBOOL bIsDone = FALSE;
					for (INT LevelIndex = 0; LevelIndex < WorldInfo->StreamingLevels.Num() && !bIsDone; LevelIndex++)
					{
						ULevelStreamingPersistent* LevelStreaming = Cast<ULevelStreamingPersistent>(WorldInfo->StreamingLevels(LevelIndex));
						// if the level streaming object has a loaded world that isn't this one, check it's
						// world's streaming levels
						if (LevelStreaming != NULL && LevelStreaming->LoadedLevel != NULL && LevelStreaming->LoadedLevel != Level)
						{
                            AWorldInfo* SubLevelWorldInfo = CastChecked<AWorldInfo>(LevelStreaming->LoadedLevel->Actors(0));
							if (SubLevelWorldInfo)
							{
								for (INT SubLevelIndex=0; SubLevelIndex < SubLevelWorldInfo->StreamingLevels.Num() && !bIsDone; SubLevelIndex++ )
								{
									ULevelStreaming* SubLevelStreaming = SubLevelWorldInfo->StreamingLevels(SubLevelIndex);
									// look to see if the sublevelWorld was the "owner" of this level
									if (SubLevelStreaming->PackageName == LevelPackageName && LevelStreaming->LoadedLevel->GetGameSequence() != NULL)
									{
										RootSequence = LevelStreaming->LoadedLevel->GetGameSequence();
										// mark as as done to get out of nested loop
										bIsDone = TRUE;
									}
								}
							}
						}
					}
				}
				check(RootSequence != NULL);
				for( INT SequenceIndex=0; SequenceIndex<Level->GameSequences.Num(); SequenceIndex++ )
				{
					USequence* Sequence = Level->GameSequences(SequenceIndex);
					if( Sequence )
					{
						// Add the game sequences to the persistent level's root sequence.
						RootSequence->SequenceObjects.AddUniqueItem(Sequence);
						RootSequence->NestedSequences.AddUniqueItem(Sequence);
						// Set the parent to the current root sequence.
						Sequence->ParentSequence = RootSequence;
						// And initialize the sequence, unless told not to
						if (!bDisallowRoutingSequenceBeginPlay)
						{
							Sequence->BeginPlay();
						}
					}
				}
			}
			Level->bAlreadyRoutedSequenceBeginPlay = TRUE;
			bExecuteNextStep = !IsTimeLimitExceeded( TEXT("routing BeginPlay on sequences"), StartTime, Level );
		}

		// Sort the actor list. @todo optimization: we could actually just serialize the indices
		if( bExecuteNextStep && !Level->bAlreadySortedActorList )
		{
			Level->SortActorList();
			Level->bAlreadySortedActorList = TRUE;
			bExecuteNextStep = !IsTimeLimitExceeded( TEXT("sorting actor list"), StartTime, Level );
			bPerformedLastStep = TRUE;
		}
	}
	// !GIsGame
	else
	{
		bPerformedLastStep = TRUE;
	}

	GIsAssociatingLevel = FALSE;

	// We're done.
	if( bPerformedLastStep )
	{
		// Reset temporary level properties for next time.
		Level->bHasVisibilityRequestPending				= FALSE;
		Level->bAlreadyMovedActors						= FALSE;
		Level->bAlreadyUpdatedComponents				= FALSE;
		Level->bAlreadyCreateBSPPhysMesh				= FALSE;
		Level->bAlreadyInitializedAllActorRBPhys		= FALSE;
		Level->bAlreadyInitializedActors				= FALSE;
		Level->bAlreadyRoutedActorBeginPlay				= FALSE;
		Level->bAlreadyFixedUpCrossLevelRefs			= FALSE;
		Level->bAlreadyRoutedSequenceBeginPlay			= FALSE;
		Level->bAlreadySortedActorList					= FALSE;

		// Finished making level visible - allow other levels to be added to the world.
		CurrentLevelPendingVisibility					= NULL;

		// notify server that the client has finished making this level visible
		for (FPlayerIterator It(GEngine); It; ++It)
		{
			if (It->Actor != NULL)
			{
				It->Actor->eventServerUpdateLevelVisibility(Level->GetOutermost()->GetFName(), TRUE);
			}
		}

		// GEMINI_TODO: A nicer precaching scheme.
		GPrecacheNextFrame = TRUE;
	}

	StreamingLevel->bIsVisible = !Level->bHasVisibilityRequestPending;
}

/** 
 * Dissociates the passed in level from the world. The removal is blocking.
 *
 * @param LevelStreaming	Level streaming object whose level we should remove
 */
void UWorld::RemoveFromWorld( ULevelStreaming* StreamingLevel )
{
	SCOPE_CYCLE_COUNTER(STAT_RemoveFromWorldTime);
	check(StreamingLevel);

	ULevel* Level = StreamingLevel->LoadedLevel;
	check(Level);
	check(!Level->IsPendingKill());
	check(!Level->HasAnyFlags(RF_Unreachable));
	check(StreamingLevel->bIsVisible);

	// let the universe know we removed a level
	GCallbackEvent->Send(CALLBACK_LevelRemovedFromWorld, Level);

	if( CurrentLevelPendingVisibility == NULL )
	{
		// Keep track of timing.
		DOUBLE StartTime = appSeconds();	

		if( GIsGame )
		{
			// Clean up cross level paths.
			FixupCrossLevelRefs( TRUE, Level );

			// Clear out kismet refs from the root sequence if there is one.
			for (INT Idx = 0; Idx < Level->GameSequences.Num(); Idx++) 
			{ 
				USequence *Seq = Level->GameSequences(Idx); 
				if (Seq != NULL) 
				{ 
					Seq->CleanUp(); 
					// This can happen if the persistent level never had the Kismet window opened.
					if( Seq->ParentSequence )
					{
						Seq->ParentSequence->SequenceObjects.RemoveItem(Seq); 
						Seq->ParentSequence->NestedSequences.RemoveItem(Seq); 
					}
				} 
			}

			// Shut down physics for the level object (ie BSP collision).
			Level->TermLevelRBPhys(NULL);

			// Shut down physics for all the Actors in the level.
			for(INT ActorIdx=0; ActorIdx<Level->Actors.Num(); ActorIdx++)
			{
				AActor* Actor = Level->Actors(ActorIdx);
				if(Actor)
				{
					Actor->TermRBPhys(NULL);
					Actor->bScriptInitialized = FALSE;
				}
			}

			// Remove any pawns from the pawn list that are about to be streamed out
			for (APawn *Pawn = GetFirstPawn(); Pawn != NULL; Pawn = Pawn->NextPawn)
			{
				if (Pawn->IsInLevel(Level))
				{
					RemovePawn(Pawn);
				}
				else
				{
					// otherwise force floor check in case the floor was streamed out from under it
					Pawn->bForceFloorCheck = TRUE;
				}
			}
		}

		// Remove from the world's level array and destroy actor components.
		Levels.RemoveItem(Level );
		Level->ClearComponents();

		// notify server that the client has removed this level
		for (FPlayerIterator It(GEngine); It; ++It)
		{
			if (It->Actor != NULL)
			{
				It->Actor->eventServerUpdateLevelVisibility(Level->GetOutermost()->GetFName(), FALSE);
			}
		}

		StreamingLevel->bIsVisible = FALSE;
		debugfSuppressed( NAME_DevStreaming, TEXT("UWorld::RemoveFromWorld for %s took %5.2f ms"), *Level->GetOutermost()->GetName(), (appSeconds()-StartTime) * 1000.0 );
	}
}


/**
 * Helper structure encapsulating functionality used to defer marking actors and their components as pending
 * kill till right before garbage collection by registering a callback.
 */
struct FLevelStreamingGCHelper
{
	/**
	 * Request level associated with level streaming object to be unloaded.
	 *
	 * @param LevelStreaming	Level streaming object whose level should be unloaded
	 */
	static void RequestUnload( ULevelStreaming* LevelStreaming )
	{
		check( LevelStreaming->LoadedLevel );
		check( LevelStreamingObjects.FindItemIndex( LevelStreaming ) == INDEX_NONE );
		LevelStreamingObjects.AddItem( LevelStreaming );
		LevelStreaming->bHasUnloadRequestPending = TRUE;
	}

	/**
	 * Cancel any pending unload requests for passed in LevelStreaming object.
	 */
	static void CancelUnloadRequest( ULevelStreaming* LevelStreaming )
	{
		verify( LevelStreamingObjects.RemoveItem( LevelStreaming ) <= 1 );
		LevelStreaming->bHasUnloadRequestPending = FALSE;
	}

	/** 
	 * Prepares levels that are marked for unload for the GC call by marking their actors and components as
	 * pending kill.
	 */
	static void PrepareStreamedOutLevelsForGC()
	{
		// This can never ever be called during tick; same goes for GC in general.
		check( !GWorld || !GWorld->InTick );

		// Iterate over all streaming level objects that want their levels unloaded.
		for( INT LevelStreamingIndex=0; LevelStreamingIndex<LevelStreamingObjects.Num(); LevelStreamingIndex++ )
		{
			ULevelStreaming*	LevelStreaming	= LevelStreamingObjects(LevelStreamingIndex);
			ULevel*				Level			= LevelStreaming->LoadedLevel;
			check(Level);

			if( !GIsEditor || (Level->GetOutermost()->PackageFlags & PKG_PlayInEditor) )
			{
				debugfSuppressed( NAME_DevStreaming, TEXT("PrepareStreamedOutLevelsForGC called on '%s'"), *Level->GetOutermost()->GetName() );

				// Make sure that this package has been unloaded after GC pass.
				LevelPackageNames.AddItem( Level->GetOutermost()->GetFName() );

				// Mark level as pending kill so references to it get deleted.
				Level->MarkPendingKill();

				// Mark all model components as pending kill so GC deletes references to them.
				for( INT ModelComponentIndex=0; ModelComponentIndex<Level->ModelComponents.Num(); ModelComponentIndex++ )
				{
					UModelComponent* ModelComponent = Level->ModelComponents(ModelComponentIndex);
					if( ModelComponent )
					{
						ModelComponent->MarkPendingKill();
					}
				}

				// Mark all actors and their components as pending kill so GC will delete references to them.
				for( INT ActorIndex=0; ActorIndex<Level->Actors.Num(); ActorIndex++ )
				{
					AActor* Actor = Level->Actors(ActorIndex);
					if( Actor )
					{
						if (GWorld)
						{
							// notify both net drivers about the actor destruction
							for (INT NetDriverIndex = 0; NetDriverIndex < 2; NetDriverIndex++)
							{
								UNetDriver* NetDriver = NetDriverIndex ? GWorld->DemoRecDriver : GWorld->GetNetDriver();
								// close any channels for this actor
								if (NetDriver != NULL)
								{
									// server
									NetDriver->NotifyActorDestroyed(Actor);
									// client
									if (NetDriver->ServerConnection != NULL)
									{
										// we can't kill the channel until the server says so, so just clear the actor ref and break the channel
										UActorChannel* Channel = NetDriver->ServerConnection->ActorChannels.FindRef(Actor);
										if (Channel != NULL)
										{
											NetDriver->ServerConnection->ActorChannels.Remove(Actor);
											Channel->Actor = NULL;
											Channel->Broken = TRUE;
										}
									}
								}
							}
						}
						Actor->MarkComponentsAsPendingKill( FALSE );
						Actor->MarkPendingKill();
					}
				}

				for (INT SeqIdx = 0; SeqIdx < Level->GameSequences.Num(); SeqIdx++)
				{
					USequence* Sequence = Level->GameSequences(SeqIdx);
					if( Sequence != NULL )
					{
						Sequence->MarkSequencePendingKill();
					}
				}

				// Remove reference so GC can delete it.
				LevelStreaming->LoadedLevel					= NULL;
				// We're no longer having an unload request pending.
				LevelStreaming->bHasUnloadRequestPending	= FALSE;
			}
		}
		LevelStreamingObjects.Empty();
	}

	/**
	 * Verify that the level packages are no longer around.
	 */
	static void VerifyLevelsGotRemovedByGC()
	{
		if( !GIsEditor )
		{
#if DO_GUARD_SLOW
			INT FailCount = 0;
			// Iterate over all objects and find out whether they reside in a GC'ed level package.
			for( FObjectIterator It; It; ++It )
			{
				UObject* Object = *It;
				// Check whether object's outermost is in the list.
				if( LevelPackageNames.FindItemIndex( Object->GetOutermost()->GetFName() ) != INDEX_NONE
				// But disregard package object itself.
				&&	!Object->IsA(UPackage::StaticClass()) )
				{
					debugf(TEXT("%s didn't get garbage collected! Trying to find culprit, though this might crash. Try increasing stack size if it does."), *Object->GetFullName());
					UObject::StaticExec(*FString::Printf(TEXT("OBJ REFS CLASS=%s NAME=%s"),*Object->GetClass()->GetName(), *Object->GetPathName()));
					TMap<UObject*,UProperty*>	Route		= FArchiveTraceRoute::FindShortestRootPath( Object, TRUE, GARBAGE_COLLECTION_KEEPFLAGS );
					FString						ErrorString	= FArchiveTraceRoute::PrintRootPath( Route, Object );
					// Print out error message. We don't assert here as there might be multiple culprits.
					warnf( TEXT("%s didn't get garbage collected!") LINE_TERMINATOR TEXT("%s"), *Object->GetFullName(), *ErrorString );
					FailCount++;
				}
			}
			if( FailCount > 0 )
			{
				appErrorf(TEXT("Streamed out levels were not completely garbage collected! Please see previous log entries."));
			}
#endif
			LevelPackageNames.Empty();
		}
	}

	/**
	 * @return	The number of levels pending a purge by the garbage collector
	 */
	static INT GetNumLevelsPendingPurge()
	{
		return LevelStreamingObjects.Num();
	}

private:
	/** Static array of level streaming objects that want their levels to be unloaded */
	static TArray<ULevelStreaming*> LevelStreamingObjects;
	/** Static array of level packages that have been marked by PrepareStreamedOutLevelsForGC */
	static TArray<FName> LevelPackageNames;
};
/** Static array of level streaming objects that want their levels to be unloaded */
TArray<ULevelStreaming*> FLevelStreamingGCHelper::LevelStreamingObjects;
/** Static array of level packages that have been marked by PrepareStreamedOutLevelsForGC */
TArray<FName> FLevelStreamingGCHelper::LevelPackageNames;

IMPLEMENT_PRE_GARBAGE_COLLECTION_CALLBACK( DUMMY_PrepareStreamedOutLevelsForGC, FLevelStreamingGCHelper::PrepareStreamedOutLevelsForGC, GCCB_PRE_PrepareLevelsForGC );
IMPLEMENT_POST_GARBAGE_COLLECTION_CALLBACK( DUMMY_VerifyLevelsGotRemovedByGC, FLevelStreamingGCHelper::VerifyLevelsGotRemovedByGC, GCCB_POST_VerifyLevelsGotRemovedByGC );

/**
 * Callback function used in UpdateLevelStreaming to pass to LoadPackageAsync. Sets LoadedLevel to map found in LinkerRoot.
 * @param	LevelPackage	level package that finished async loading
 * @param	Unused
 */
static void AsyncLevelLoadCompletionCallback( UObject* LevelPackage, void* /*Unused*/ )
{
	if( !GWorld )
	{
		// This happens if we change levels while there are outstanding load requests.
	}
	else if( LevelPackage )
	{
		// Try to find a UWorld object in the level package.
		UWorld* World = (UWorld*) UObject::StaticFindObjectFast( UWorld::StaticClass(), LevelPackage, NAME_TheWorld );
		ULevel* Level = World ? World->PersistentLevel : NULL;	
		if( Level )
		{
			// Iterate over level streaming objects to find the ones that have a package name matching the name of the LinkerRoot.
			AWorldInfo*	WorldInfo = GWorld->GetWorldInfo();
			for( INT LevelIndex=0; LevelIndex<WorldInfo->StreamingLevels.Num(); LevelIndex++ )
			{
				ULevelStreaming* StreamingLevel	= WorldInfo->StreamingLevels(LevelIndex);
				if( StreamingLevel && StreamingLevel->PackageName == LevelPackage->GetFName() )
				{
					// Propagate loaded level so garbage collection doesn't delete it in case async loading was flushed by GC.
					StreamingLevel->LoadedLevel = Level;
					
					// Cancel any pending unload requests.
					FLevelStreamingGCHelper::CancelUnloadRequest( StreamingLevel );
				}
			}
		}
		else
		{
			debugf( NAME_Warning, TEXT("Couldn't find ULevel object in package '%s'"), *LevelPackage->GetName() );
		}
	}
	else
	{
		debugf( NAME_Warning, TEXT("NULL LevelPackage as argument to AsyncLevelCompletionCallback") );
	}
}
 

/**
 * Updates the world based on the current view location of the player and sets level LODs accordingly.
 *
 * @param ViewFamily	Optional collection of views to take into account
 */
void UWorld::UpdateLevelStreaming( FSceneViewFamily* ViewFamily )
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateLevelStreamingTime);
	// do nothing if level streaming is frozen
	if (bIsLevelStreamingFrozen)
	{
		return;
	}

	AWorldInfo*	WorldInfo						= GetWorldInfo();
	UBOOL		bLevelsHaveBeenUnloaded			= FALSE;
	// whether one or more levels has a pending load request
	UBOOL		bLevelsHaveLoadRequestPending	= FALSE;

	// Iterate over level collection to find out whether we need to load/ unload any levels.
	for( INT LevelIndex=0; LevelIndex<WorldInfo->StreamingLevels.Num(); LevelIndex++ )
	{
		ULevelStreaming* StreamingLevel	= WorldInfo->StreamingLevels(LevelIndex);
		if( !StreamingLevel )
		{
			// This can happen when manually adding a new level to the array via the property editing code.
			continue;
		}
		
		//@todo seamless: add ability to abort load request.

		// Work performed to make a level visible is spread across several frames and we can't unload/ hide a level that is currently pending
		// to be made visible, so we fulfill those requests first.
		UBOOL bHasVisibilityRequestPending	= StreamingLevel->LoadedLevel && StreamingLevel->LoadedLevel->bHasVisibilityRequestPending;
		
		// Figure out whether level should be loaded, visible and block on load if it should be loaded but currently isn't.
		UBOOL bShouldBeLoaded				= bHasVisibilityRequestPending || (!GEngine->bUseBackgroundLevelStreaming && !StreamingLevel->bIsRequestingUnloadAndRemoval);
		UBOOL bShouldBeVisible				= bHasVisibilityRequestPending;
		UBOOL bShouldBlockOnLoad			= StreamingLevel->bShouldBlockOnLoad;

		// Don't update if the code requested this level object to be unloaded and removed.
		if( !StreamingLevel->bIsRequestingUnloadAndRemoval )
		{
			// Take all views into account if any were passed in.
			if( ViewFamily )
			{
				// Iterate over all views in collection.
				for( INT ViewIndex=0; ViewIndex<ViewFamily->Views.Num(); ViewIndex++ )
				{
					const FSceneView* View		= ViewFamily->Views(ViewIndex);
					const FVector& ViewLocation	= View->ViewOrigin;
					bShouldBeLoaded	= bShouldBeLoaded  || ( StreamingLevel && (!GIsGame || StreamingLevel->ShouldBeLoaded(ViewLocation)) );
					bShouldBeVisible= bShouldBeVisible || ( bShouldBeLoaded && StreamingLevel && StreamingLevel->ShouldBeVisible(ViewLocation) );
				}
			}
			// Or default to view location of 0,0,0
			else
			{
				FVector ViewLocation(0,0,0);
				bShouldBeLoaded		= bShouldBeLoaded  || ( StreamingLevel && (!GIsGame || StreamingLevel->ShouldBeLoaded(ViewLocation)) );
				bShouldBeVisible	= bShouldBeVisible || ( bShouldBeLoaded && StreamingLevel && StreamingLevel->ShouldBeVisible(ViewLocation) );
			}
		}

		// Figure out whether there are any levels we haven't collected garbage yet.
		UBOOL bAreLevelsPendingPurge	=	FLevelStreamingGCHelper::GetNumLevelsPendingPurge() > 0;
		// We want to give the garbage collector a chance to remove levels before we stream in more. We can't do this in the
		// case of a blocking load as it means those requests should be fulfilled right away. By waiting on GC before kicking
		// off new levels we potentially delay streaming in maps, but AllowLevelLoadRequests already looks and checks whether
		// async loading in general is active. E.g. normal package streaming would delay loading in this case. This is done
		// on purpose as well so the GC code has a chance to execute between consecutive loads of maps.
		//
		// NOTE: AllowLevelLoadRequests not an invariant as streaming might affect the result, do NOT pulled out of the loop.
		UBOOL bAllowLevelLoadRequests	=	(AllowLevelLoadRequests() && !bAreLevelsPendingPurge) || bShouldBlockOnLoad;

		// Request a 'soft' GC if there are levels pending purge and there are levels to be loaded. In the case of a blocking
		// load this is going to guarantee GC firing first thing afterwards and otherwise it is going to sneak in right before
		// kicking off the async load.
		if( bAreLevelsPendingPurge && bShouldBeLoaded )
		{
			GWorld->GetWorldInfo()->ForceGarbageCollection( FALSE );
		}

		// See whether level is already loaded
		if(	bShouldBeLoaded 
		&&	!StreamingLevel->LoadedLevel )
		{
			if( !StreamingLevel->bHasLoadRequestPending )
			{
				// Try to find the [to be] loaded package.
				UPackage* LevelPackage = (UPackage*) UObject::StaticFindObjectFast( UPackage::StaticClass(), NULL, StreamingLevel->PackageName );
				
				// Package is already or still loaded.
				UBOOL bNeedToLoad = TRUE;
				if( LevelPackage )
				{
					// Find world object and use its PersistentLevel pointer.
					UWorld* World = (UWorld*) UObject::StaticFindObjectFast( UWorld::StaticClass(), LevelPackage, NAME_TheWorld );
					if (World != NULL)
					{
#if !FINAL_RELEASE
						if (World->PersistentLevel == NULL)
						{
							debugf(TEXT("World exists but PersistentLevel doesn't for %s, most likely caused by reference to world of unloaded level and GC setting reference to NULL while keeping world object"), *World->GetOutermost()->GetName());
							// print out some debug information...
							UObject::StaticExec(*FString::Printf(TEXT("OBJ REFS CLASS=WORLD NAME=%s.TheWorld"), *World->GetOutermost()->GetName()));
							TMap<UObject*,UProperty*> Route = FArchiveTraceRoute::FindShortestRootPath( World, TRUE, GARBAGE_COLLECTION_KEEPFLAGS );
							FString ErrorString = FArchiveTraceRoute::PrintRootPath( Route, World );
							debugf(TEXT("%s"), *ErrorString);
							// before asserting
							checkMsg(World->PersistentLevel,TEXT("Most likely caused by reference to world of unloaded level and GC setting reference to NULL while keeping world object"));
						}
#endif
						StreamingLevel->LoadedLevel	= World->PersistentLevel;
						bNeedToLoad = FALSE;
					}
				}
				// Async load package if world object couldn't be found and we are allowed to request a load.
				if( bNeedToLoad && bAllowLevelLoadRequests  )
				{
					if( GUseSeekFreeLoading )
					{
						// Only load localized package if it exists as async package loading doesn't handle errors gracefully.
						FString LocalizedPackageName = StreamingLevel->PackageName.ToString() + LOCALIZED_SEEKFREE_SUFFIX;
						FString LocalizedFileName;
						if( GPackageFileCache->FindPackageFile( *LocalizedPackageName, NULL, LocalizedFileName ) )
						{
							// Load localized part of level first in case it exists. We don't need to worry about GC or completion 
							// callback as we always kick off another async IO for the level below.
							UObject::LoadPackageAsync( *LocalizedPackageName, NULL, NULL );
						}
					}
						
					// Kick off async load request.
					UObject::LoadPackageAsync( *StreamingLevel->PackageName.ToString(), AsyncLevelLoadCompletionCallback, NULL );
					
					// streamingServer: server loads everything?
					// Editor immediately blocks on load and we also block if background level streaming is disabled.
					if( GIsEditor || !GEngine->bUseBackgroundLevelStreaming )
					{
						// Finish all async loading.
						UObject::FlushAsyncLoading();

						// Completion callback should have associated level by now.
						if( StreamingLevel->LoadedLevel == NULL )
						{
							debugfSuppressed( NAME_DevStreaming, TEXT("Failed to load %s"), *StreamingLevel->PackageName.ToString() );
						}
					}

					// Load request is still pending if level isn't set.
					StreamingLevel->bHasLoadRequestPending = (StreamingLevel->LoadedLevel == NULL) ? TRUE : FALSE;
				}
			}

			// We need to tell the game to bring up a loading screen and flush async loading during this or the next tick.
			if( StreamingLevel->bHasLoadRequestPending && bShouldBlockOnLoad && !GIsEditor )
			{
				GWorld->GetWorldInfo()->bRequestedBlockOnAsyncLoading = TRUE;
				debugfSuppressed( NAME_DevStreaming, TEXT("Requested blocking on load for level %s"), *StreamingLevel->PackageName.ToString() );
			}
		}

		// See whether we have a loaded level.
		if( StreamingLevel->LoadedLevel )
		{
			// Cache pointer for convenience. This cannot happen before this point as e.g. flushing async loaders
			// or such will modify StreamingLevel->LoadedLevel.
			ULevel* Level = StreamingLevel->LoadedLevel;

			// We have a level so there is no load request pending.
			StreamingLevel->bHasLoadRequestPending = FALSE;

			// Associate level if it should be visible and hasn't been in the past.
			if( bShouldBeVisible && !StreamingLevel->bIsVisible )
			{
				// Calculate relative offset and keep track of it so we can move maps around in the Editor.
				FVector RelativeOffset		= StreamingLevel->Offset - StreamingLevel->OldOffset;
				StreamingLevel->OldOffset	= StreamingLevel->Offset;

				// Make level visible.  Updates bIsVisible if level is finished being made visible.
				AddToWorld( StreamingLevel, RelativeOffset );
			}
			// Level was visible before but no longer should be.
			else if( !bShouldBeVisible && StreamingLevel->bIsVisible )
			{
				// Hide this level/ remove from world. Updates bIsVisible.
				RemoveFromWorld( StreamingLevel );
			}

			// Make sure level is referenced if it should be loaded or is currently visible.
			if( bShouldBeLoaded || StreamingLevel->bIsVisible )
			{
				// Cancel any pending requests to unload this level.
				FLevelStreamingGCHelper::CancelUnloadRequest( StreamingLevel );
			}
			else if( !StreamingLevel->bHasUnloadRequestPending )
			{
				// Request unloading this level.
				FLevelStreamingGCHelper::RequestUnload( StreamingLevel );
				bLevelsHaveBeenUnloaded	= TRUE;
			}
		}
		// Editor is always blocking on load and if we don't have a level now it is because it couldn't be found. Ditto if background
		// level streaming is disabled.
		else if( GIsEditor || !GEngine->bUseBackgroundLevelStreaming )
		{
			StreamingLevel->bHasLoadRequestPending = FALSE;
		}

		// If requested, remove this level from iterated over array once it is unloaded.
		if( StreamingLevel->bIsRequestingUnloadAndRemoval 
		&&	!bShouldBeLoaded 
		&&	!StreamingLevel->bIsVisible )
		{
			// The -- is required as we're forward iterating over the StreamingLevels array.
			WorldInfo->StreamingLevels.Remove( LevelIndex-- );
		}

		bLevelsHaveLoadRequestPending = (bLevelsHaveLoadRequestPending || StreamingLevel->bHasLoadRequestPending);
	}

	// Force initial loading to be "bShouldBlockOnLoad".
	if (bLevelsHaveLoadRequestPending && (!HasBegunPlay() || (GEngine->IsA(UGameEngine::StaticClass()) ? ((UGameEngine*)GEngine)->bWorldWasLoadedThisTick : GetTimeSeconds() < 1.f)))
	{
		// Block till all async requests are finished.
		UObject::FlushAsyncLoading();
	}

	if( bLevelsHaveBeenUnloaded && !GIsEditor )
	{
		//@todo seamless: we need to request GC to unload
	}
}

/**
 * Flushes level streaming in blocking fashion and returns when all levels are loaded/ visible/ hidden
 * so further calls to UpdateLevelStreaming won't do any work unless state changes. Basically blocks
 * on all async operation like updating components.
 *
 * @param ViewFamily	Optional collection of views to take into account
 * @param bOnlyFlushVisibility		Whether to only flush level visibility operations (optional)
 */
void UWorld::FlushLevelStreaming( FSceneViewFamily* ViewFamily, UBOOL bOnlyFlushVisibility )
{
	// Can't happen from within Tick as we are adding/ removing actors to the world.
	check( !InTick );

	AWorldInfo* WorldInfo = GetWorldInfo();

	// Allow queuing multiple load requests if we're performing a full flush and disallow if we're just
	// flushing level visibility.
	INT OldAllowLevelLoadOverride = AllowLevelLoadOverride;
	AllowLevelLoadOverride = bOnlyFlushVisibility ? 0 : 1;

	// Update internals with current loaded/ visibility flags.
	GWorld->UpdateLevelStreaming();

	// Only flush async loading if we're performing a full flush.
	if( !bOnlyFlushVisibility )
	{
		// Make sure all outstanding loads are taken care of
		UObject::FlushAsyncLoading();
	}

	// Kick off making levels visible if loading finished by flushing.
	GWorld->UpdateLevelStreaming();

	// Making levels visible is spread across several frames so we simply loop till it is done.
	UBOOL bLevelsPendingVisibility = TRUE;
	while( bLevelsPendingVisibility )
	{
		bLevelsPendingVisibility = IsVisibilityRequestPending();

		// Tick level streaming to make levels visible.
		if( bLevelsPendingVisibility )
		{
			// Only flush async loading if we're performing a full flush.
			if( !bOnlyFlushVisibility )
			{
				// Make sure all outstanding loads are taken care of...
				UObject::FlushAsyncLoading();
			}
	
			// Update level streaming.
			GWorld->UpdateLevelStreaming( ViewFamily );
		}
	}
	check( CurrentLevelPendingVisibility == NULL );

	// Update level streaming one last time to make sure all RemoveFromWorld requests made it.
	GWorld->UpdateLevelStreaming( ViewFamily );

	// We already blocked on async loading.
	if( !bOnlyFlushVisibility )
	{
		GWorld->GetWorldInfo()->bRequestedBlockOnAsyncLoading = FALSE;
	}

	AllowLevelLoadOverride = OldAllowLevelLoadOverride;
}

/** @return whether there is at least one level with a pending visibility request */
UBOOL UWorld::IsVisibilityRequestPending()
{
	// Iterate over all levels to find out whether they have a request pending.
	AWorldInfo* WorldInfo = GetWorldInfo();
	for (INT LevelIndex = 0; LevelIndex < WorldInfo->StreamingLevels.Num(); LevelIndex++)
	{
		ULevelStreaming* LevelStreaming = WorldInfo->StreamingLevels(LevelIndex);
		
		// See whether there's a level with a pending request.
		if (LevelStreaming != NULL && LevelStreaming->LoadedLevel != NULL && LevelStreaming->LoadedLevel->bHasVisibilityRequestPending)
		{	
			// Bingo. We can early out at this point as we only care whether there is at least
			// one level in this phase.
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Returns whether the level streaming code is allowed to issue load requests.
 *
 * @return TRUE if level load requests are allowed, FALSE otherwise.
 */
UBOOL UWorld::AllowLevelLoadRequests()
{
	UBOOL bAllowLevelLoadRequests = FALSE;
	// Hold off requesting new loads if there is an active async load request.
	if( !GIsEditor )
	{
		// Let code choose.
		if( AllowLevelLoadOverride == 0 )
		{
			// There are pending queued requests and gameplay has already started so hold off queueing.
			if( UObject::IsAsyncLoading() && GetTimeSeconds() > 1.f )
			{
				bAllowLevelLoadRequests = FALSE;
			}
			// No load requests or initial load so it's save to queue.
			else
			{
				bAllowLevelLoadRequests = TRUE;
			}
		}
		// Use override, < 0 == don't allow, > 0 == allow
		else
		{
			bAllowLevelLoadRequests = AllowLevelLoadOverride > 0 ? TRUE : FALSE;
		}
	}
	// Always allow load request in the Editor.
	else
	{
		bAllowLevelLoadRequests = TRUE;
	}
	return bAllowLevelLoadRequests;
}

UBOOL UWorld::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	//@todo seamless: move debug execs into UDebugManager
	if( NetDriver && NetDriver->Exec( Cmd, Ar ) )
	{
		return 1;
	}
	else if( DemoRecDriver && DemoRecDriver->Exec( Cmd, Ar ) )
	{
		return 1;
	}
	else if( ParseCommand( &Cmd, TEXT("SHOWEXTENTLINECHECK") ) )
	{
		bShowExtentLineChecks = !bShowExtentLineChecks;
		return 1;
	}
	else if( ParseCommand( &Cmd, TEXT("SHOWLINECHECK") ) )
	{
		bShowLineChecks = !bShowLineChecks;
		return 1;
	}
	else if( ParseCommand( &Cmd, TEXT("SHOWPOINTCHECK") ) )
	{
		bShowPointChecks = !bShowPointChecks;
		return 1;
    }
#if LINE_CHECK_TRACING
    else if( ParseCommand( &Cmd, TEXT("DUMPLINECHECKS") ) )
    {            
        LINE_CHECK_DUMP();
        return TRUE;
    }
    else if( ParseCommand( &Cmd, TEXT("RESETLINECHECKS") ) )
    {
        LINE_CHECK_RESET();
        return TRUE;
    }
    else if( ParseCommand( &Cmd, TEXT("TOGGLELINECHECKS") ) )
    {
        LINE_CHECK_TOGGLE();
        return TRUE;
    }
	else if( ParseCommand( &Cmd, TEXT("TOGGLELINECHECKSPIKES") ) )
	{
		FString NumChecksStr;
		ParseToken(Cmd, NumChecksStr, 0);
		if (NumChecksStr != TEXT(""))
		{
			INT NumLineChecksPerFrame = appAtoi(*NumChecksStr);
			if (NumLineChecksPerFrame > 0)
			{
				LINE_CHECK_TOGGLESPIKES(NumLineChecksPerFrame);
			}
		}		   

		return TRUE;
	}
#endif //LINE_CHECK_TRACING
	else if( ParseCommand( &Cmd, TEXT("FLUSHPERSISTENTDEBUGLINES") ) )
	{
		PersistentLineBatcher->BatchedLines.Empty();
		PersistentLineBatcher->BeginDeferredReattach();
		return 1;
	}
	else if (ParseCommand(&Cmd, TEXT("DEMOREC")))
	{
		// Attempt to make the dir if it doesn't exist.
		FString DemoDir = appGameDir() + TEXT("Demos");
		
		GFileManager->MakeDirectory(*DemoDir, TRUE);

		FURL URL;
		FString DemoName;
		if( !ParseToken( Cmd, DemoName, 0 ) )
		{
#if NEED_THIS
			if ( !GConfig->GetString( TEXT("DemoRecording"), TEXT("DemoMask"), DemoName, TEXT("user.ini")) )
#endif
			{
				DemoName=TEXT("%m-%t");
			}
		}

		DemoName = DemoName.Replace(TEXT("%m"), *URL.Map);

		INT Year, Month, DayOfWeek, Day, Hour, Min,Sec,MSec;
		appSystemTime(Year, Month, DayOfWeek, Day, Hour, Min,Sec,MSec);

		//DemoName = DemoName.Replace(TEXT("%td"), *FString::Printf(TEXT("%i-%i-%i-%i"),Year,Month,Day,((Hour*3600)+(Min*60)+Sec)*1000+MSec));
		DemoName = DemoName.Replace(TEXT("%td"), *appSystemTimeString());
		DemoName = DemoName.Replace(TEXT("%d"),  *FString::Printf(TEXT("%i-%i-%i"),Month,Day,Year));
		DemoName = DemoName.Replace(TEXT("%t"),  *FString::Printf(TEXT("%i"),((Hour*3600)+(Min*60)+Sec)*1000+MSec));

//		UGameEngine* GameEngine = CastChecked<UGameEngine>( Engine );

		if ( GEngine && GEngine->GamePlayers.Num() > 0 && GEngine->GamePlayers(0) != NULL && 
			GEngine->GamePlayers(0)->Actor != NULL && GEngine->GamePlayers(0)->Actor->PlayerReplicationInfo != NULL )
		{
			DemoName=DemoName.Replace( TEXT("%p"), *GEngine->GamePlayers(0)->Actor->PlayerReplicationInfo->PlayerName);
		}

		// replace bad characters with underscores
		DemoName = DemoName.Replace( TEXT("\\"), TEXT("_"));
		DemoName = DemoName.Replace( TEXT("/"),TEXT("_") );
		DemoName = DemoName.Replace( TEXT("."),TEXT("_") );
		DemoName = DemoName.Replace( TEXT(" "),TEXT("_") );

		// replace the current URL's map with a demo extension
		URL.Map = DemoDir * DemoName + TEXT(".demo");

		UClass* DemoDriverClass = StaticLoadClass(UDemoRecDriver::StaticClass(), NULL, TEXT("engine-ini:Engine.Engine.DemoRecordingDevice"), NULL, LOAD_None, NULL);
		check(DemoDriverClass);
		DemoRecDriver           = ConstructObject<UDemoRecDriver>(DemoDriverClass);
		check(DemoRecDriver);
		FString Error;
		if (!DemoRecDriver->InitListen(this, URL, Error))
		{
			Ar.Logf( TEXT("Demo recording failed: %s"), *Error );//!!localize!!
			DemoRecDriver = NULL;
		}
		else
		{
			Ar.Logf( TEXT("Demo recording started to %s"), *URL.Map );
		}
		return 1;
	}
	else if( ParseCommand( &Cmd, TEXT("DEMOPLAY") ) )
	{
		FString Temp;
		if( ParseToken( Cmd, Temp, 0 ) )
		{
			UGameEngine* GameEngine = CastChecked<UGameEngine>(GEngine);

			FURL URL( NULL, *Temp, TRAVEL_Absolute );			
			debugf( TEXT("Attempting to play demo %s"), *Temp );
			URL.Map = appGameDir() * TEXT("Demos") * FFilename(URL.Map).GetBaseFilename() + TEXT(".demo");

			if( GameEngine->GPendingLevel )
			{
				GameEngine->CancelPending();
			}

			GameEngine->GPendingLevel = new UDemoPlayPendingLevel( URL );
			if( !GameEngine->GPendingLevel->DemoRecDriver )
			{
				//@todo ronp connection
				// UDemoPlayPendingLevel will set the appropriate error code and connectionlost type, so
				// we just have to propagate that message to the game.
				Ar.Logf( TEXT("Demo playback failed: %s"), *GameEngine->GPendingLevel->ConnectionError );//!!localize!!
				GameEngine->GPendingLevel = NULL;
			}
		}
		else
		{
			Ar.Log( TEXT("You must specify a filename") );//!!localize!!
		}
		return 1;
	}
	else if ( ParseCommand( &Cmd, TEXT("HIDELOGDETAILEDSTATS") ) )
	{
		extern UBOOL GLogDetailedTickStats;
		extern UBOOL GLogDetailedDumpStats;
		extern UBOOL GLogDetailedActorUpdateStats;
		extern UBOOL GLogDetailedComponentUpdateStats;
		GLogDetailedTickStats = FALSE;
		GLogDetailedDumpStats = FALSE;
		GLogDetailedActorUpdateStats = FALSE;
		GLogDetailedComponentUpdateStats = FALSE;
		return 1;
	}
	else if ( ParseCommand( &Cmd, TEXT("TOGGLELOGDETAILEDTICKSTATS") ) )
	{
		extern UBOOL GLogDetailedTickStats;
		GLogDetailedTickStats = !GLogDetailedTickStats;
		return 1;
	}
	else if ( ParseCommand( &Cmd, TEXT("TOGGLELOGDETAILEDDUMPSTATS") ) )
	{
		extern UBOOL GLogDetailedDumpStats;
		// If dump stat is the only one on, then it's not going to work
		// it needs any of the other stats to be on
		// TOGGLELOGDETAILEDTICKSTATS or TOGGLELOGDETAILEDACTORUPDATESTATS or 
		// TOGGLELOGDETAILEDCOMPONENTUPDATESTATS
		GLogDetailedDumpStats = !GLogDetailedDumpStats;
		return 1;
	}
	else if ( ParseCommand( &Cmd, TEXT("TOGGLELOGDETAILEDACTORUPDATESTATS") ) )
	{
		extern UBOOL GLogDetailedActorUpdateStats;
		GLogDetailedActorUpdateStats = !GLogDetailedActorUpdateStats;
		return 1;
	}
	else if ( ParseCommand( &Cmd, TEXT("TOGGLELOGDETAILEDCOMPONENTUPDATESTATS") ) )
	{
		extern UBOOL GLogDetailedComponentUpdateStats;
		GLogDetailedComponentUpdateStats = !GLogDetailedComponentUpdateStats;
		return 1;
	}
	else if( Hash->Exec(Cmd,Ar) )
	{
		return 1;
	}
	else if (NavigationOctree->Exec(Cmd, Ar))
	{
		return 1;
	}
	else if( ExecRBCommands( Cmd, &Ar ) )
	{
		return 1;
	}
	else 
	{
		return 0;
	}
}

void UWorld::SetGameInfo(const FURL& InURL)
{
	AWorldInfo* Info = GetWorldInfo();

	if( IsServer() && !Info->Game )
	{
		// Init the game info.
		FString Options(TEXT(""));
		TCHAR GameClassName[256]=TEXT("");
		FString	Error=TEXT("");
		for( INT i=0; i<InURL.Op.Num(); i++ )
		{
			Options += TEXT("?");
			Options += InURL.Op(i);
			Parse( *InURL.Op(i), TEXT("GAME="), GameClassName, ARRAY_COUNT(GameClassName) );
		}

		UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);

		// Get the GameInfo class.
		UClass* GameClass=NULL;
		if ( GameClassName[0] )
		{
			// if the gamename was specified, we can use it to fully load the pergame PreLoadClass packages
			if (GameEngine)
			{
				GameEngine->LoadPackagesFully(FULLYLOAD_Game_PreLoadClass, GameClassName);
			}

			GameClass = StaticLoadClass( AGameInfo::StaticClass(), NULL, GameClassName, NULL, LOAD_None, NULL);
		}
		if ( !GameClass )
		{
			GameClass = StaticLoadClass( AGameInfo::StaticClass(), NULL, (GEngine->Client != NULL && !InURL.HasOption(TEXT("Listen"))) ? TEXT("game-ini:Engine.GameInfo.DefaultGame") : TEXT("game-ini:Engine.GameInfo.DefaultServerGame"), NULL, LOAD_None, NULL);
		}
        if ( !GameClass ) 
			GameClass = AGameInfo::StaticClass();
		else
			GameClass = Cast<AGameInfo>(GameClass->GetDefaultActor())->eventSetGameType(*InURL.Map, Options, *InURL.Portal);

		// no matter how the game was specified, we can use it to load the PostLoadClass packages
		if (GameEngine)
		{
			GameEngine->LoadPackagesFully(FULLYLOAD_Game_PostLoadClass, GameClass->GetPathName());
			GameEngine->LoadPackagesFully(FULLYLOAD_Game_PostLoadClass, TEXT("LoadForAllGameTypes"));
		}

		// Spawn the GameInfo.
		debugf( NAME_Log, TEXT("Game class is '%s'"), *GameClass->GetName() );
		Info->Game = (AGameInfo*)SpawnActor( GameClass );
		check(Info->Game!=NULL);
	}
}

//#define DEBUG_CHECKCOLLISIONCOMPONENTS 1
// used to check if actors have more than one collision component (most should not, but can easily happen accidentally)

/** BeginPlay - Begins gameplay in the level.
 * @param InURL commandline URL
 * @param bResetTime (optional) whether the WorldInfo's TimeSeconds should be reset to zero
 */
void UWorld::BeginPlay(const FURL& InURL, UBOOL bResetTime)
{
	check(bIsWorldInitialized);
	DOUBLE StartTime = appSeconds();

	AWorldInfo* Info = GetWorldInfo();

	// Don't reset time for seamless world transitions.
	if (bResetTime)
	{
		GetWorldInfo()->TimeSeconds = 0.0f;
		GetWorldInfo()->RealTimeSeconds = 0.0f;
		GetWorldInfo()->AudioTimeSeconds = 0.0f;
	}

	// Get URL Options
	FString Options(TEXT(""));
	FString	Error(TEXT(""));
	for( INT i=0; i<InURL.Op.Num(); i++ )
	{
		Options += TEXT("?");
		Options += InURL.Op(i);
	}

	// Set level info.
	if( !InURL.GetOption(TEXT("load"),NULL) )
		URL = InURL;
	Info->EngineVersion = FString::Printf( TEXT("%i"), GEngineVersion );
	Info->MinNetVersion = FString::Printf( TEXT("%i"), GEngineMinNetVersion );
	Info->ComputerName = appComputerName();

	// Update world and the components of all levels.
	UpdateComponents( TRUE );

	// Clear any existing stat charts.
	if(GStatChart)
	{
		GStatChart->Reset();
	}

	// Reset indices till we have a chance to rearrange actor list at the end of this function.
	for( INT LevelIndex=0; LevelIndex<Levels.Num(); LevelIndex++ )
	{
		ULevel* Level = Levels(LevelIndex);
		Level->iFirstDynamicActor		= 0;
		Level->iFirstNetRelevantActor	= 0;
	}

	// Initialize rigid body physics.
	InitWorldRBPhys();

	// Initialize physics engine for persistent level. 
	// This creates physics engine BSP representation and iterates over actors calling InitRBPhys for each.
	PersistentLevel->InitLevelBSPPhysMesh();
	PersistentLevel->IncrementalInitActorsRBPhys(0);

	// Init level gameplay info.
	if( !HasBegunPlay() )
	{
		// allow any post-ship patching
		GGamePatchHelper->FixupWorld(this);

		// Check that paths are valid
		if ( !GetWorldInfo()->bPathsRebuilt )
		{
			warnf(TEXT("*** WARNING - PATHS MAY NOT BE VALID ***"));
		}

		// Lock the level.
		debugf( NAME_Log, TEXT("Bringing %s up for play (%i) at %s"), *GetFullName(), appRound(GEngine->GetMaxTickRate(0,FALSE)), *appSystemTimeString() );
		GetWorldInfo()->GetDefaultPhysicsVolume()->bNoDelete = true;

		// Initialize all actors and start execution.
		PersistentLevel->InitializeActors();

		// Enable actor script calls.
		Info->bBegunPlay	= 1;
		Info->bStartup		= 1;

		// Init the game.
		if (Info->Game != NULL && !Info->Game->bScriptInitialized)
		{
			Info->Game->eventInitGame( Options, Error );
		}

		// Route various begin play functions and set volumes.
		PersistentLevel->RouteBeginPlay();

		// Initialize any scripting sequences
		if (GetGameSequence() != NULL)
		{
			GetGameSequence()->BeginPlay();
		}

		Info->bStartup = 0;
	}

	// Rearrange actors: static not net relevant actors first, then static net relevant actors and then others.
	check( Levels.Num() );
	check( PersistentLevel );
	check( Levels(0) == PersistentLevel );
	for( INT LevelIndex=0; LevelIndex<Levels.Num(); LevelIndex++ )
	{
		ULevel*	Level = Levels(LevelIndex);
		Level->SortActorList();
	}

	debugf(TEXT("Bringing up level for play took: %f"), appSeconds() - StartTime );

	// do memory tracking if we are looking at mem
	AGameInfo* GameInfo = GetGameInfo();
	if ( GameInfo != NULL ) 
	{
		if ( GameInfo->bCheckingForMemLeaks == TRUE ) 
		{
			if (GameInfo->bUsingAutomatedTestingMapList == TRUE)
			{
				if (GameInfo->AutomatedMapTestingTransitionMap != TEXT(""))
				{
					if ( GetFullName().InStr(GameInfo->AutomatedMapTestingTransitionMap) != -1 )
					{
						Info->DoMemoryTracking();
					}
				}
				else
				if ( GetFullName().InStr(InURL.DefaultTransitionMap) != -1 )  // if we are in the transition map
				{
					Info->DoMemoryTracking();
				}
			}
			else
			{
				if ( GetFullName().InStr(InURL.DefaultTransitionMap) != -1 )  // if we are in the transition map
				{
					Info->DoMemoryTracking();
				}
			}
		}

		if (GameInfo->bUsingAutomatedTestingMapList == TRUE)
		{
			// Start the transition timer...
			GameInfo->eventStartAutomatedMapTestTimer();
		}
	}
}


/** 
 * This function will do what ever memory tracking we have enabled.  Basically there are a myriad of memory tracking/leak detection
 * methods and this function abstracts all of that.
 **/
void AWorldInfo::DoMemoryTracking()
{
#if ENABLE_MEM_TAGGING
		debugf(TEXT("**** MemTagUpdate ****"));
		GEngine->Exec(TEXT("MEMTAG_UPDATE"));
#endif  // ENABLE_MEM_TAGGING

		// we always want to dump out the data
		debugf(TEXT("**** MemLeakCheck ****"));
		GEngine->Exec( TEXT("MemLeakCheck") );

}

/**
 *	This function will add a debug message to the onscreen message list.
 *	It will be displayed for FrameCount frames.
 *
 *	@param	Key				A unique key to prevent the same message from being added multiple times.
 *	@param	TimeToDisplay	How long to display the message, in seconds.
 *	@param	DisplayColor	The color to display the text in.
 *	@param	DebugMessage	The message to display.
 */
void AWorldInfo::AddOnScreenDebugMessage(INT Key,FLOAT TimeToDisplay,FColor DisplayColor,const FString& DebugMessage)
{
#if !FINAL_RELEASE
	if (Key == -1)
	{
		FScreenMessageString* NewMessage = new(PriorityScreenMessages)FScreenMessageString();
		check(NewMessage);
		NewMessage->Key = Key;
		NewMessage->ScreenMessage = DebugMessage;
		NewMessage->DisplayColor = DisplayColor;
		NewMessage->TimeToDisplay = TimeToDisplay;
		NewMessage->CurrentTimeDisplayed = 0.0f;
	}
	else
	{
		FScreenMessageString* Message = ScreenMessages.Find(Key);
		if (Message == NULL)
		{
			FScreenMessageString NewMessage;
			NewMessage.CurrentTimeDisplayed = 0.0f;
			NewMessage.Key = Key;
			NewMessage.DisplayColor = DisplayColor;
			NewMessage.TimeToDisplay = TimeToDisplay;
			NewMessage.ScreenMessage = DebugMessage;
			ScreenMessages.Set(Key, NewMessage);
		}
		else
		{
			// Set the message, and update the time to display and reset the current time.
			Message->ScreenMessage = DebugMessage;
			Message->DisplayColor = DisplayColor;
			Message->TimeToDisplay = TimeToDisplay;
			Message->CurrentTimeDisplayed = 0.0f;
		}
	}
#endif
}

UBOOL AWorldInfo::OnScreenDebugMessageExists(INT Key)
{
	if (Key == -1)
	{
		// Priority messages assumed to always exist...
		// May want to check for there being none.
		return TRUE;
	}

	if (ScreenMessages.Find(Key) != NULL)
	{
		return TRUE;
	}

	return FALSE;
}

/**
 * Cleans up components, streaming data and assorted other intermediate data.
 * @param bSessionEnded whether to notify the viewport that the game session has ended
 */
void UWorld::CleanupWorld(UBOOL bSessionEnded)
{
	check(CurrentLevelPendingVisibility == NULL);

	if (bSessionEnded)
	{
		if (GEngine != NULL && GEngine->GameViewport != NULL)
		{
			GEngine->GameViewport->eventGameSessionEnded();
		}
		else if (!GIsRequestingExit)
		{
			UDataStoreClient* DSC = UUIInteraction::GetDataStoreClient();
			if (DSC != NULL)
			{
				DSC->eventNotifyGameSessionEnded();
			}
		}
	}

	// Tell actors to remove their components from the scene.
	ClearComponents();
	
	// Remove all objects from octree.
	if( NavigationOctree )
	{
		NavigationOctree->RemoveAllObjects();
	}

	// Clear standalone flag when switching maps in the Editor. This causes resources placed in the map
	// package to be garbage collected together with the world.
	if( GIsEditor && !IsTemplate() )
	{
		UPackage* WorldPackage = GetOutermost();
		// Iterate over all objects to find ones that reside in the same package as the world.
		for( FObjectIterator It; It; ++It )
		{
			UObject* Object = *It;
			if( Object->IsIn( WorldPackage ) )
			{
				Object->ClearFlags( RF_Standalone );
			}
		}
	}
}

/*-----------------------------------------------------------------------------
	Accessor functions.
-----------------------------------------------------------------------------*/

/**
 * Returns the current netmode
 *
 * @return current netmode
 */
ENetMode UWorld::GetNetMode() const
{
	return (ENetMode) GetWorldInfo()->NetMode;
}

/**
 * Returns the current game info object.
 *
 * @return current game info object
 */
AGameInfo* UWorld::GetGameInfo() const
{
	return GetWorldInfo()->Game;
}

/**
 * Returns the first navigation point. Note that ANavigationPoint contains
 * a pointer to the next navigation point so this basically returns a linked
 * list of navigation points in the world.
 *
 * @return first navigation point
 */
ANavigationPoint* UWorld::GetFirstNavigationPoint() const
{
	return GetWorldInfo()->NavigationPointList;
}

/**
 * Returns the first controller. Note that AController contains a pointer to
 * the next controller so this basically returns a linked list of controllers
 * associated with the world.
 *
 * @return first controller
 */
AController* UWorld::GetFirstController() const
{
	return GetWorldInfo()->ControllerList;
}

/**
 * Returns the first pawn. Note that APawn contains a pointer to
 * the next pawn so this basically returns a linked list of pawns
 * associated with the world.
 *
 * @return first pawn
 */
APawn* UWorld::GetFirstPawn() const
{
	return GetWorldInfo()->PawnList;
}

//debug
//pathdebug
#if 0 && !PS3 && !FINAL_RELEASE
#define CHECKNAVLIST(b, x) \
		if( !GIsEditor && ##b ) \
		{ \
			debugf(*##x); \
			for (ANavigationPoint *T = GWorld->GetFirstNavigationPoint(); T != NULL; T = T->nextNavigationPoint) \
			{ \
				T->ClearForPathFinding(); \
			} \
			UWorld::VerifyNavList(*##x); \
		}
#else
#define CHECKNAVLIST(b, x)
#endif

void UWorld::AddLevelNavList( ULevel *Level, UBOOL bDebugNavList )
{
	if (Level != NULL && Level->NavListStart != NULL && Level->NavListEnd != NULL)
	{
		// don't add if we're in the editor
		//if (GIsGame) // MT-> DO add if we're in editor :D
		{
			// for each nav in the level nav list
			for (ANavigationPoint *Nav = Level->NavListStart; Nav != NULL; Nav = Nav->nextNavigationPoint)
			{
				// add the nav to the octree
				Nav->AddToNavigationOctree();
			}
		}
		AWorldInfo *Info = GetWorldInfo();
		// insert the level at the beginning of the nav list
		Level->NavListEnd->nextNavigationPoint = Info->NavigationPointList;
		Info->NavigationPointList = Level->NavListStart;

		// insert the cover list as well
		if (Level->CoverListStart != NULL && Level->CoverListEnd != NULL)
		{
			Level->CoverListEnd->NextCoverLink = Info->CoverList;
			Info->CoverList = Level->CoverListStart; 
		}

		//debug
		CHECKNAVLIST(bDebugNavList , FString::Printf(TEXT("AddLevelNavList %s"), *Level->GetFullName()));
	}
}

void UWorld::RemoveLevelNavList( ULevel *Level, UBOOL bDebugNavList )
{
	if (Level != NULL && Level->NavListStart != NULL && Level->NavListEnd != NULL)
	{
		//debug
		CHECKNAVLIST(bDebugNavList , FString::Printf(TEXT("RemoveLevelNavList %s"), *Level->GetFullName()));

		AWorldInfo *Info = GetWorldInfo();
		// if this level is at the start of the list,
		if (Level->NavListStart == Info->NavigationPointList)
		{
			// then just move the start of the end to the next level
			Info->NavigationPointList = Level->NavListEnd->nextNavigationPoint;
		}
		else
		{
			// otherwise find the nav that is referencing the start of this level's list
			ANavigationPoint *End = NULL;
			for (INT LevelIdx = 0; LevelIdx < Levels.Num(); LevelIdx++)
			{
				ULevel *ChkLevel = Levels(LevelIdx);
				if (ChkLevel != Level &&
					ChkLevel->NavListEnd != NULL &&
					ChkLevel->NavListEnd->nextNavigationPoint == Level->NavListStart)
				{
					End = ChkLevel->NavListEnd;
					break;
				}
			}
			if (End != NULL)
			{
				// point the current end to the level's end's next point
				End->nextNavigationPoint = Level->NavListEnd->nextNavigationPoint;
			}
		}
		// and clear the level's end
		Level->NavListEnd->nextNavigationPoint = NULL;

		// update the cover list as well
		if (Level->CoverListStart != NULL && Level->CoverListEnd != NULL)
		{
			if (Level->CoverListStart == Info->CoverList)
			{
				Info->CoverList = Level->CoverListEnd->NextCoverLink;
			}
			else
			{
				ACoverLink *End = NULL;
				for (INT LevelIdx = 0; LevelIdx < Levels.Num(); LevelIdx++)
				{
					ULevel *ChkLevel = Levels(LevelIdx);
					if (ChkLevel != Level &&
						ChkLevel->CoverListEnd != NULL &&
						ChkLevel->CoverListEnd->NextCoverLink == Level->CoverListStart)
					{
						End = ChkLevel->CoverListEnd;
						break;
					}
				}
				if (End != NULL)
				{
					End->NextCoverLink = Level->CoverListEnd->NextCoverLink;
				}
			}
			if (Level->CoverListEnd != NULL)
			{
				Level->CoverListEnd->NextCoverLink = NULL;
			}
		}
	}
}

#undef CHECKNAVLIST

void UWorld::ResetNavList()
{
	// tada!
	GetWorldInfo()->NavigationPointList = NULL;
	GetWorldInfo()->CoverList = NULL;
}

AActor* UWorld::FindActorByGuid(FGuid &Guid, UClass* InClass)
{
	AActor *Result = NULL;
	UBOOL bFullSearch = GIsEditor;

	if( InClass == ANavigationPoint::StaticClass() )
	{
		for( INT LevelIdx = 0; LevelIdx < Levels.Num() && Result == NULL; LevelIdx++ )
		{
			ULevel *Level = Levels(LevelIdx);
			for( ANavigationPoint *Nav = Level->NavListStart; Nav != NULL; Nav = Nav->nextNavigationPoint )
			{
				if( *Nav->GetGuid() == Guid )
				{
					Result = Nav;
					break;
				}
			}
		}
	}
	else
	{
		bFullSearch = TRUE;
	}

	if( bFullSearch && Result == NULL )
	{
		// use an actor iterator in the editor since paths may not be rebuilt
		for( FActorIterator It; It; ++It )
		{
			AActor* Actor = *It;
			if(  Actor->GetGuid() != NULL && 
				*Actor->GetGuid() == Guid )
			{
				Result = Actor;
				break;
			}
		}
	}
	return Result;
}

/**
 * Inserts the passed in controller at the front of the linked list of controllers.
 *
 * @param	Controller	Controller to insert, use NULL to clear list
 */
void UWorld::AddController( AController* Controller )
{
	if( Controller )
	{
		Controller->NextController = GetWorldInfo()->ControllerList;
	}
	GetWorldInfo()->ControllerList = Controller;
}

/**
 * Removes the passed in controller from the linked list of controllers.
 *
 * @param	Controller	Controller to remove
 */
void UWorld::RemoveController( AController* Controller )
{
	AController* Next = GetFirstController();
	if( Next == Controller )
	{
		GetWorldInfo()->ControllerList = Controller->NextController;
	}
	else
	{
		while( Next != NULL )
		{
			if( Next->NextController == Controller )
			{
				Next->NextController = Controller->NextController;
				break;
			}
			Next = Next->NextController;
		}
	}
}

/**
 * Inserts the passed in pawn at the front of the linked list of pawns.
 *
 * @param	Pawn	Pawn to insert, use NULL to clear list
 */
void UWorld::AddPawn( APawn* Pawn )
{
	if( Pawn )
	{
		// make sure it's not already in the list
		RemovePawn(Pawn);
		// and add to the beginning
		Pawn->NextPawn = GetWorldInfo()->PawnList;
	}
	GetWorldInfo()->PawnList = Pawn;
}

/**
 * Removes the passed in pawn from the linked list of pawns.
 *
 * @param	Pawn	Pawn to remove
 */
void UWorld::RemovePawn( APawn* Pawn )
{
	APawn* Next = GetFirstPawn();
	if( Next == Pawn )
	{
		GetWorldInfo()->PawnList = Pawn->NextPawn;
	}
	else
	{
		while( Next != NULL )
		{
			if( Next->NextPawn == Pawn )
			{
				Next->NextPawn = Pawn->NextPawn;
				break;
			}
			Next = Next->NextPawn;
		}
	}
}

/**
 * Returns the default brush.
 *
 * @return default brush
 */
ABrush* UWorld::GetBrush() const
{
	check(PersistentLevel);
	return PersistentLevel->GetBrush();
}

/**
 * Returns whether game has already begun play.
 *
 * @return TRUE if game has already started, FALSE otherwise
 */
UBOOL UWorld::HasBegunPlay() const
{
	return PersistentLevel && PersistentLevel->Actors.Num() && GetWorldInfo() && GetWorldInfo()->bBegunPlay;
}

/**
 * Returns whether gameplay has already begun and we are not associating a level
 * with the world.
 *
 * @return TRUE if game has already started and we're not associating a level, FALSE otherwise
 */
UBOOL UWorld::HasBegunPlayAndNotAssociatingLevel() const
{
	return HasBegunPlay() && !GIsAssociatingLevel;
}

/**
 * Returns time in seconds since world was brought up for play, IS stopped when game pauses, IS dilated/clamped
 *
 * @return time in seconds since world was brought up for play
 */
FLOAT UWorld::GetTimeSeconds() const
{
	return GetWorldInfo()->TimeSeconds;
}

/**
* Returns time in seconds since world was brought up for play, is NOT stopped when game pauses, NOT dilated/clamped
*
* @return time in seconds since world was brought up for play
*/
FLOAT UWorld::GetRealTimeSeconds() const
{
	return GetWorldInfo()->RealTimeSeconds;
}

/**
* Returns time in seconds since world was brought up for play, IS stopped when game pauses, NOT dilated/clamped
*
* @return time in seconds since world was brought up for play
*/
FLOAT UWorld::GetAudioTimeSeconds() const
{
	return GetWorldInfo()->AudioTimeSeconds;
}

/**
 * Returns the frame delta time in seconds adjusted by e.g. time dilation.
 *
 * @return frame delta time in seconds adjusted by e.g. time dilation
 */
FLOAT UWorld::GetDeltaSeconds() const
{
	return GetWorldInfo()->DeltaSeconds;
}

/**
 * Returns the default physics volume and creates it if necessary.
 * 
 * @return default physics volume
 */
APhysicsVolume* UWorld::GetDefaultPhysicsVolume() const
{
	return GetWorldInfo()->GetDefaultPhysicsVolume();
}

/**
 * Returns the physics volume a given actor is in.
 *
 * @param	Location
 * @param	Actor
 * @param	bUseTouch
 *
 * @return physics volume given actor is in.
 */
APhysicsVolume* UWorld::GetPhysicsVolume(FVector Loc, AActor *A, UBOOL bUseTouch) const
{
	return GetWorldInfo()->GetPhysicsVolume(Loc,A,bUseTouch);
}

/**
 * Returns the global/ persistent Kismet sequence for the specified level.
 *
 * @param	OwnerLevel		the level to get the sequence from - must correspond to one of the levels in GWorld's Levels array;
 *							thus, only applicable when editing a multi-level map.  Defaults to the level currently being edited.
 *
 * @return	a pointer to the sequence located at the specified element of the specified level's list of sequences, or
 *			NULL if that level doesn't exist or the index specified is invalid for that level's list of sequences
 */
USequence* UWorld::GetGameSequence(ULevel* OwnerLevel/* =NULL  */) const
{
	if( OwnerLevel == NULL )
	{
		OwnerLevel = CurrentLevel;
	}
	check(OwnerLevel);
	return OwnerLevel->GetGameSequence();
}

/**
 * Sets the current (or specified) level's kismet sequence to the sequence specified.
 *
 * @param	GameSequence	the sequence to add to the level
 * @param	OwnerLevel		the level to add the sequence to - must correspond to one of the levels in GWorld's Levels array;
 *							thus, only applicable when editing a multi-level map.  Defaults to the level currently being edited.
 */
void UWorld::SetGameSequence(USequence* GameSequence, ULevel* OwnerLevel/* =NULL  */)
{
	if( OwnerLevel == NULL )
	{
		OwnerLevel = CurrentLevel;
	}

	if( OwnerLevel->GameSequences.Num() == 0 )
	{
		OwnerLevel->GameSequences.AddItem( GameSequence );
	}
	else
	{
		OwnerLevel->GameSequences(0) = GameSequence;
	}
	check( OwnerLevel->GameSequences.Num() == 1 );
}


/**
 * Returns the AWorldInfo actor associated with this world.
 *
 * @return AWorldInfo actor associated with this world
 */
AWorldInfo* UWorld::GetWorldInfo( UBOOL bCheckStreamingPesistent ) const
{
	checkSlow(PersistentLevel);
	checkSlow(PersistentLevel->Actors.Num());
	checkSlow(PersistentLevel->Actors(0));
	checkSlow(PersistentLevel->Actors(0)->IsA(AWorldInfo::StaticClass()));

	AWorldInfo* WorldInfo = (AWorldInfo*)PersistentLevel->Actors(0);
	if( bCheckStreamingPesistent )
	{
		if( WorldInfo->StreamingLevels.Num() > 0 &&
			WorldInfo->StreamingLevels(0) &&
			WorldInfo->StreamingLevels(0)->LoadedLevel && 
			WorldInfo->StreamingLevels(0)->IsA(ULevelStreamingPersistent::StaticClass()) )
		{
			WorldInfo = WorldInfo->StreamingLevels(0)->LoadedLevel->GetWorldInfo();
		}
	}
	return WorldInfo;
}

/**
 * Returns the current levels BSP model.
 *
 * @return BSP UModel
 */
UModel* UWorld::GetModel() const
{
	check(CurrentLevel);
	return CurrentLevel->Model;
}

/**
 * Returns the Z component of the current world gravity.
 *
 * @return Z component of current world gravity.
 */
FLOAT UWorld::GetGravityZ() const
{
	return GetWorldInfo()->GetGravityZ();
}

/**
 * Returns the Z component of the default world gravity.
 *
 * @return Z component of the default world gravity.
 */
FLOAT UWorld::GetDefaultGravityZ() const
{
	return GetWorldInfo()->DefaultGravityZ;
}

/**
 * Returns the Z component of the current world gravity scaled for rigid body physics.
 *
 * @return Z component of the current world gravity scaled for rigid body physics.
 */
FLOAT UWorld::GetRBGravityZ() const
{
	return GetWorldInfo()->GetRBGravityZ();
}


/** This is our global function for retrieving the current MapName **/
const FString GetMapNameStatic()
{
	FString Retval;

	if( GWorld != NULL )
	{
		Retval = GWorld->GetMapName();
	}
	else
	{	
		extern FString GetStartupMap(const TCHAR* CommandLine);
		Retval = GetStartupMap( appCmdLine() );
	}

	return Retval;
}


/** This is our global function for retrieving the current non persistent MapName **/
const FString GetNonPersistentMapNameStatic()
{
	FString NonPersistentMapName = TEXT(" ");
	UINT NumNonPersistentMapLoaded = 0;
	// In the case of a seamless world check to see whether there are any persistent levels in the levels
	// array and use its name if there is one.
	if( GWorld != NULL )
	{
		AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
		if( WorldInfo != NULL )
		{
			for( INT LevelIndex=0; LevelIndex < WorldInfo->StreamingLevels.Num(); LevelIndex++ )
			{
				ULevelStreamingPersistent* PersistentLevel = Cast<ULevelStreamingPersistent>( WorldInfo->StreamingLevels(LevelIndex) );
				// Use the name of the first 
				if( ( PersistentLevel == NULL ) && ( WorldInfo->StreamingLevels(LevelIndex)->LoadedLevel != NULL ) )
				{
					NonPersistentMapName = WorldInfo->StreamingLevels(LevelIndex)->PackageName.ToString();
					++NumNonPersistentMapLoaded;
				}
			}
		}
	}

	// if there is more than one level loaded then we do not not to pass in anything as streaming is occurring / we are not
	// doing a single level memory look at.  OR there is only a single level loaded (probably MP map)
	if( NumNonPersistentMapLoaded > 1 )
	{
		NonPersistentMapName = TEXT(" ");
	}

	return NonPersistentMapName;
}

/**
 * Returns the name of the current map, taking into account using a dummy persistent world
 * and loading levels into it via PrepareMapChange.
 *
 * @return	name of the current map
 */
const FString UWorld::GetMapName() const
{
	// Default to the world's package as the map name.
	FString MapName = GetOutermost()->GetName();
	
	// In the case of a seamless world check to see whether there are any persistent levels in the levels
	// array and use its name if there is one.
	AWorldInfo* WorldInfo = GetWorldInfo();
	for( INT LevelIndex=0; LevelIndex<WorldInfo->StreamingLevels.Num(); LevelIndex++ )
	{
		ULevelStreamingPersistent* PersistentLevel = Cast<ULevelStreamingPersistent>( WorldInfo->StreamingLevels(LevelIndex) );
		// Use the name of the first found persistent level.
		if( PersistentLevel )
		{
			MapName = PersistentLevel->PackageName.ToString();
			break;
		}
	}

	return MapName;
}

/*-----------------------------------------------------------------------------
	Network interface.
-----------------------------------------------------------------------------*/

//
// The network driver is about to accept a new connection attempt by a
// connectee, and we can accept it or refuse it.
//
EAcceptConnection UWorld::NotifyAcceptingConnection()
{
	check(NetDriver);
	if( NetDriver->ServerConnection )
	{
		// We are a client and we don't welcome incoming connections.
		debugf( NAME_DevNet, TEXT("NotifyAcceptingConnection: Client refused") );
		return ACCEPTC_Reject;
	}
	else if( GetWorldInfo()->NextURL!=TEXT("") )
	{
		// Server is switching levels.
		debugf( NAME_DevNet, TEXT("NotifyAcceptingConnection: Server %s refused"), *GetName() );
		return ACCEPTC_Ignore;
	}
	else
	{
		// Server is up and running.
		debugf( NAME_DevNet, TEXT("NotifyAcceptingConnection: Server %s accept"), *GetName() );
		return ACCEPTC_Accept;
	}
}

//
// This server has accepted a connection.
//
void UWorld::NotifyAcceptedConnection( UNetConnection* Connection )
{
	check(NetDriver!=NULL);
	check(NetDriver->ServerConnection==NULL);
	debugf( NAME_NetComeGo, TEXT("Open %s %s %s"), *GetName(), appTimestamp(), *Connection->LowLevelGetRemoteAddress() );
}

//
// The network interface is notifying this level of a new channel-open
// attempt by a connectee, and we can accept or refuse it.
//
UBOOL UWorld::NotifyAcceptingChannel( UChannel* Channel )
{
	check(Channel);
	check(Channel->Connection);
	check(Channel->Connection->Driver);
	UNetDriver* Driver = Channel->Connection->Driver;

	if( Driver->ServerConnection )
	{
		// We are a client and the server has just opened up a new channel.
		//debugf( "NotifyAcceptingChannel %i/%i client %s", Channel->ChIndex, Channel->ChType, *GetName() );
		if( Channel->ChType==CHTYPE_Actor )
		{
			// Actor channel.
			//debugf( "Client accepting actor channel" );
			return 1;
		}
		else
		{
			// Unwanted channel type.
			debugf( NAME_DevNet, TEXT("Client refusing unwanted channel of type %i"), (BYTE)Channel->ChType );
			return 0;
		}
	}
	else
	{
		// We are the server.
		if( Channel->ChIndex==0 && Channel->ChType==CHTYPE_Control )
		{
			// The client has opened initial channel.
			debugf( NAME_DevNet, TEXT("NotifyAcceptingChannel Control %i server %s: Accepted"), Channel->ChIndex, *GetFullName() );
			return 1;
		}
		else if( Channel->ChType==CHTYPE_File )
		{
			// The client is going to request a file.
			debugf( NAME_DevNet, TEXT("NotifyAcceptingChannel File %i server %s: Accepted"), Channel->ChIndex, *GetFullName() );
			return 1;
		}
		else
		{
			// Client can't open any other kinds of channels.
			debugf( NAME_DevNet, TEXT("NotifyAcceptingChannel %i %i server %s: Refused"), (BYTE)Channel->ChType, Channel->ChIndex, *GetFullName() );
			return 0;
		}
	}
}

//
// Welcome a new player joining this server.
//
void UWorld::WelcomePlayer( UNetConnection* Connection, const TCHAR* Optional )
{
	check(CurrentLevel);
	Connection->PackageMap->Copy( Connection->Driver->MasterMap );
	Connection->SendPackageMap();

	FString LevelName = CurrentLevel->GetOutermost()->GetName();
	Connection->ClientWorldPackageName = GetOutermost()->GetFName();
	if( Optional[0] )
	{
		Connection->Logf( TEXT("WELCOME LEVEL=%s %s"), *LevelName, Optional );
	}
	else
	{
		Connection->Logf( TEXT("WELCOME LEVEL=%s"), *LevelName );
	}
	Connection->FlushNet();
	// don't count initial join data for netspeed throttling
	// as it's unnecessary, since connection won't be fully open until it all gets received, and this prevents later gameplay data from being delayed to "catch up"
	Connection->QueuedBytes = 0;
}

/** verifies that the client has loaded or can load the package with the specified information
 * if found, sets the Info's Parent to the package and notifies the server of our generation of the package
 * if not, handles downloading the package, skipping it, or disconnecting, depending on the requirements of the package
 * @param Info the info on the package the client should have
 */
UBOOL UWorld::VerifyPackageInfo(FPackageInfo& Info)
{
	check(!GWorld->IsServer());

	if( GUseSeekFreeLoading )
	{
		FString PackageNameString = Info.PackageName.ToString();
		// try to find the package in memory
		Info.Parent = FindPackage(NULL, *PackageNameString);
		if (Info.Parent == NULL)
		{
			if (IsAsyncLoading())
			{
				// delay until async loading is complete
				return FALSE;
			}
			else if (Info.ForcedExportBasePackageName == NAME_None)
			{
				// attempt to async load the package
				FString Filename;
				if (GPackageFileCache->FindPackageFile(*PackageNameString, &Info.Guid, Filename))
				{
					// check for localized package first as that may be required to load the base package
					FString LocalizedPackageName = PackageNameString + LOCALIZED_SEEKFREE_SUFFIX;
					FString LocalizedFileName;
					if (GPackageFileCache->FindPackageFile(*LocalizedPackageName, NULL, LocalizedFileName))
					{
						LoadPackageAsync(*LocalizedPackageName, NULL, NULL);
					}
#if !CONSOLE
					// if this is a downloaded package, just load the linker
					// as we may not be able to load its contents without dependencies that are later in the package list
					if (Filename.StartsWith(GSys->CachePath))
					{
						BeginLoad();
						Info.Parent = CreatePackage(NULL, *PackageNameString);
						GetPackageLinker(Info.Parent, *Filename, LOAD_NoWarn | LOAD_NoVerify | LOAD_Quiet, NULL, &Info.Guid);
						EndLoad();
					}
					else
#endif
					{
						// load base package
						LoadPackageAsync(PackageNameString, NULL, NULL, &Info.Guid);
					}
					return FALSE;
				}
			}
			if (Info.ForcedExportBasePackageName != NAME_None)
			{
				//@FIXME: we may need the GUID of the base package here...

				// this package is a forced export inside another package, so try loading that other package
				FString BasePackageNameString(Info.ForcedExportBasePackageName.ToString());
				FString FileName;
				if (GPackageFileCache->FindPackageFile(*BasePackageNameString, NULL, FileName))
				{
					// check for localized package first as that may be required to load the base package
					FString LocalizedPackageName = BasePackageNameString + LOCALIZED_SEEKFREE_SUFFIX;
					FString LocalizedFileName;
					if (GPackageFileCache->FindPackageFile(*LocalizedPackageName, NULL, LocalizedFileName))
					{
						LoadPackageAsync(*LocalizedPackageName, NULL, NULL);
					}
					// load base package
					LoadPackageAsync(*BasePackageNameString, NULL, NULL);
					return FALSE;
				}
			}
			debugf(NAME_DevNet, TEXT("Failed to find required package '%s'"), *PackageNameString);

#if OLD_CONNECTION_FAILURE_CODE
			GEngine->SetClientTravel(TEXT("?failed"), TRAVEL_Absolute);
			GEngine->SetProgress(*FString::Printf(TEXT("Failed to find required package '%s'"), *PackageNameString), TEXT(""), 4.0f);
#else
			GEngine->SetProgress(PMT_ConnectionFailure, *FString::Printf(LocalizeSecure(LocalizeError(TEXT("FailedFindPackage"),TEXT("Engine")), *PackageNameString)), TEXT("") );
			
			//@todo ronp connection
			// should we rely on TravelURL.Len() to determine whether we should call SetClientTravel??
			UGameEngine* GE = Cast<UGameEngine>(GEngine);
			if ( GE == NULL || GE->TravelURL.Len() == 0 )
			{
				GEngine->SetClientTravel(TEXT("?failed"), TRAVEL_Absolute);
			}
#endif
			NetDriver->ServerConnection->Close();
		}
		else if (Info.Parent->GetGuid() != Info.Guid)
		{
			debugf(NAME_DevNet, TEXT("Package '%s' mismatched - Server GUID: %s Client GUID: %s"), *Info.Parent->GetName(), *Info.Guid.String(), *Info.Parent->GetGuid().String());

#if OLD_CONNECTION_FAILURE_CODE
			GEngine->SetClientTravel(TEXT("?failed"), TRAVEL_Absolute);
			GEngine->SetProgress(*FString::Printf(TEXT("Package '%s' version mismatch"), *Info.Parent->GetName()), TEXT(""), 4.0f);
#else
			GEngine->SetProgress( PMT_ConnectionFailure, *FString::Printf(LocalizeSecure(LocalizeError(TEXT("PackageVersion"), TEXT("Core")), *Info.Parent->GetName())), TEXT("") );

			//@todo ronp connection
			// should we rely on TravelURL.Len() to determine whether we should call SetClientTravel??
			UGameEngine* GE = Cast<UGameEngine>(GEngine);
			if ( GE == NULL || GE->TravelURL.Len() == 0 )
			{
				GEngine->SetClientTravel(TEXT("?failed"), TRAVEL_Absolute);
			}
#endif
			NetDriver->ServerConnection->Close();
		}
		else
		{
			Info.LocalGeneration = Info.Parent->GetGenerationNetObjectCount().Num();
			// tell the server what we have
			NetDriver->ServerConnection->Logf(TEXT("HAVE GUID=%s GEN=%i"), *Info.Parent->GetGuid().String(), Info.LocalGeneration);
		}
	}
	else
	{
		// verify that we have this package, or it is downloadable
		FString Filename;
		if (GPackageFileCache->FindPackageFile(*Info.PackageName.ToString(), &Info.Guid, Filename))
		{
			Info.Parent = FindPackage(NULL, *Info.PackageName.ToString());
			if (Info.Parent == NULL)
			{
				if (IsAsyncLoading())
				{
					// delay until async loading is complete
					return FALSE;
				}
				Info.Parent = CreatePackage(NULL, *Info.PackageName.ToString());
			}
			// check that the GUID matches (meaning it is the same package or it has been conformed)
			if (!Info.Parent->GetGuid().IsValid() || Info.Parent->GetGenerationNetObjectCount().Num() == 0)
			{
				if (IsAsyncLoading())
				{
					// delay until async loading is complete
					return FALSE;
				}
				// we need to load the linker to get the info
				BeginLoad();
				GetPackageLinker(Info.Parent, NULL, LOAD_NoWarn | LOAD_NoVerify | LOAD_Quiet, NULL, &Info.Guid);
				EndLoad();
			}
			if (Info.Parent->GetGuid() != Info.Guid)
			{
				// incompatible files
				//@todo FIXME: we need to be able to handle this better - have the client ignore this version of the package and download the correct one
				debugf(NAME_DevNet, TEXT("Package '%s' mismatched - Server GUID: %s Client GUID: %s"), *Info.Parent->GetName(), *Info.Guid.String(), *Info.Parent->GetGuid().String());

#if OLD_CONNECTION_FAILURE_CODE
				GEngine->SetClientTravel(TEXT("?failed"), TRAVEL_Absolute);
				GEngine->SetProgress(*FString::Printf(TEXT("Package '%s' version mismatch"), *Info.Parent->GetName()), TEXT(""), 4.0f);
#else
				GEngine->SetProgress( PMT_ConnectionFailure, *FString::Printf(LocalizeSecure(LocalizeError(TEXT("PackageVersion"), TEXT("Core")), *Info.Parent->GetName())), TEXT("") );

			    //@todo ronp connection
			    // should we rely on TravelURL.Len() to determine whether we should call SetClientTravel??
				UGameEngine* GE = Cast<UGameEngine>(GEngine);
				if ( GE == NULL || GE->TravelURL.Len() == 0 )
				{
					GEngine->SetClientTravel(TEXT("?failed"), TRAVEL_Absolute);
				}
#endif
				NetDriver->ServerConnection->Close();
			}
			else
			{
				Info.LocalGeneration = Info.Parent->GetGenerationNetObjectCount().Num();
				// tell the server what we have
				NetDriver->ServerConnection->Logf(TEXT("HAVE GUID=%s GEN=%i"), *Info.Parent->GetGuid().String(), Info.LocalGeneration);
			}
		}
		else
		{
			// we need to download this package
			//@fixme: FIXME: handle
			debugf(NAME_DevNet, TEXT("Failed to find required package '%s'"), *Info.Parent->GetName());

#if OLD_CONNECTION_FAILURE_CODE
			GEngine->SetClientTravel(TEXT("?failed"), TRAVEL_Absolute);
			GEngine->SetProgress(*FString::Printf(TEXT("Downloading '%s' not allowed"), *Info.Parent->GetName()), TEXT(""), 4.0f);
#else
			GEngine->SetProgress( PMT_ConnectionFailure, *FString::Printf(LocalizeSecure(LocalizeError(TEXT("NoDownload"), TEXT("Engine")), *Info.Parent->GetName())), TEXT("") );

			//@todo ronp connection
			// should we rely on TravelURL.Len() to determine whether we should call SetClientTravel??
			UGameEngine* GE = Cast<UGameEngine>(GEngine);
			if ( GE == NULL || GE->TravelURL.Len() == 0 )
			{
				GEngine->SetClientTravel(TEXT("?failed"), TRAVEL_Absolute);
			}
#endif
			NetDriver->ServerConnection->Close();
		}
	}

	// on success, update the entry in the package map
	if (NetDriver->ServerConnection->State != USOCK_Closed)
	{
		NetDriver->ServerConnection->PackageMap->AddPackageInfo(Info);
	}

	return TRUE;
}

/** looks for a PlayerController that was being swapped by the given NetConnection and, if found, destroys it
 * (because the swap is complete or the connection was closed)
 * @param Connection - the connection that performed the swap
 * @return whether a PC waiting for a swap was found
 */
UBOOL UWorld::DestroySwappedPC(UNetConnection* Connection)
{
	for (AController* C = GetFirstController(); C != NULL; C = C->NextController)
	{
		APlayerController* PC = C->GetAPlayerController();
		if (PC != NULL && PC->Player == NULL && PC->PendingSwapConnection == Connection)
		{
			DestroyActor(PC);
			return TRUE;
		}
	}

	return FALSE;
}

//
// Received text on the control channel.
//
void UWorld::NotifyReceivedText( UNetConnection* Connection, const TCHAR* Text )
{
	if( ParseCommand(&Text,TEXT("USERFLAG")) )
	{
		Connection->UserFlags = appAtoi(Text);
	}
	else if (DemoRecDriver != NULL && DemoRecDriver->ServerConnection != NULL)
	{
		check(Connection == DemoRecDriver->ServerConnection);
		
		// we are playing back a demo
#if !SHIPPING_PC_GAME
		debugf(NAME_DevNet, TEXT("Demo playback received: %s"), Text);
#endif
		if (ParseCommand(&Text, TEXT("USES")))
		{
			// Dependency information.
			FPackageInfo Info(NULL);
			Connection->ParsePackageInfo(Text, Info);
			Connection->PackageMap->AddPackageInfo(Info);
			// force it to be verified, flushing async loading if necessary
			checkSlow(Connection->PendingPackageInfos.Num() == 0);
			NetDriver = DemoRecDriver; //@FIXME: hack
			if (!VerifyPackageInfo(Info))
			{
				FlushAsyncLoading();
				if (!VerifyPackageInfo(Info))
				{
					// something else went wrong, give up and stop playback
					debugf(NAME_Warning, TEXT("Aborting demo playback because unable to synchronize '%s'"), *Info.PackageName.ToString());
					Connection->Close();
				}
			}
			NetDriver = NULL; //@FIXME: hack
			if (Info.LocalGeneration > 0 && Info.LocalGeneration < Info.RemoteGeneration)
			{
				// the indices will be mismatched in this case as there's no real server to adjust them for our older package version
				FString Error = FString::Printf(TEXT("Package '%s' version mismatch"), *Info.PackageName.ToString());
				debugf(NAME_Error, *Error);
				NotifyProgress(PMT_ConnectionFailure, LocalizeError(TEXT("ConnectionFailed_Title"), TEXT("Engine")), Error);
				Connection->Close();
			}
		}
		else if (ParseCommand(&Text, TEXT("UNLOAD")))
		{
			// remove a package from the package map
			FGuid Guid;
			Parse(Text, TEXT("GUID="), Guid);
			// demo playback should never have pending packages
			checkSlow(Connection->PendingPackageInfos.Num() == 0);
			// now remove from package map itself
			Connection->PackageMap->RemovePackageByGuid(Guid);
		}
	}
	else if( NetDriver->ServerConnection )
	{
		check(Connection == NetDriver->ServerConnection);

		// We are the client, travelling to a new map with the same server
#if !SHIPPING_PC_GAME
		debugf( NAME_DevNet, TEXT("Level client received: %s"), Text );
#endif
		if( ParseCommand(&Text,TEXT("FAILURE")) )
		{
			//@todo ronp - here we can further parse the string to determine the reason that the server closed our connection (disconnect?, kicked?, switched maps while we were still loading?)
			// Report problem to user.
			// 1. We had a package with an incorrect GUID (UWorld::NotifyReceivedText, parsing HAVE)
			// 2. The challenge response we sent to the server didn't match the server's version (same, parsing LOGIN)
			// 3. eventPreLogin returned FALSE (same, same, just before WelcomePlayer is called)
			// 4. SpawnPlayActor failed (UWorld::NotifyReceivedText (same), parsing JOIN)
			// 5. SpawnPlayActor failed (same, parsing JOINSPLIT)
			FString EntryURL = TEXT("?failed");

			// Let the client PC know the host shut down
			if (Connection != NULL && Connection->Actor != NULL)
			{
				while (*Text != 0 && appIsWhitespace(*Text))
				{
					Text++;
				}
				FString Error = Text;
				if (Error != TEXT(""))
				{
					// try to get localized error message
					TArray<FString> Pieces;
					Error.ParseIntoArray(&Pieces, TEXT("."), TRUE);
					if (Pieces.Num() >= 3)
					{
						Error = Localize(*Pieces(1), *Pieces(2), *Pieces(0), NULL, TRUE);
					}
				}
				NotifyProgress(PMT_ConnectionFailure, LocalizeError(TEXT("ConnectionFailed_Title"), TEXT("Engine")), Error.Len() > 0 ? Error : LocalizeError(TEXT("ConnectionFailed"), TEXT("Engine")));
// 				Connection->Actor->eventSetProgressMessage( PMT_ConnectionFailure,
// 															(Error != TEXT("")) ? Error : LocalizeError(TEXT("ConnectionFailed"), TEXT("Engine")),
// 															LocalizeError(TEXT("ConnectionFailed"), TEXT("Engine")) );

				// calling SetProgressMessage will cause an error message to be displayed to the user, probably with more detailed information...so
				// use ?closed rather than ?failed so that the user doesn't get the generic "Connection failed" message
				EntryURL = TEXT("?closed");
			}

			// pass the value of Text to Browse somehow
			GEngine->SetClientTravel(*EntryURL, TRAVEL_Absolute);
		}
		else if (ParseCommand(&Text, TEXT("USES")))
		{
			// Dependency information.
			FPackageInfo Info(NULL);
			Connection->ParsePackageInfo(Text, Info);
			// add to the packagemap immediately even if we can't verify it to guarantee its place in the list as it needs to match the server
			Connection->PackageMap->AddPackageInfo(Info);
			// it's important to verify packages in order so that we're not reshuffling replicated indices during gameplay, so don't try if there are already others pending
			if (Connection->PendingPackageInfos.Num() > 0 || !VerifyPackageInfo(Info))
			{
				// we can't verify the package until we have finished async loading
				Connection->PendingPackageInfos.AddItem(Info);
			}
		}
		else if (ParseCommand(&Text, TEXT("UNLOAD")))
		{
			// remove a package from the package map
			FGuid Guid;
			Parse(Text, TEXT("GUID="), Guid);
			// remove ref from pending list, if present
			for (INT i = 0; i < Connection->PendingPackageInfos.Num(); i++)
			{
				if (Connection->PendingPackageInfos(i).Guid == Guid)
				{
					Connection->PendingPackageInfos.Remove(i);
					// if the package was pending, the server is expecting a response, so tell it we aborted as requested
					Connection->Logf(TEXT("ABORT GUID=%s"), *Guid.String());
					break;
				}
			}
			// now remove from package map itself
			Connection->PackageMap->RemovePackageByGuid(Guid);
		}
	}
	else
	{
		// We are the server.
#if !SHIPPING_PC_GAME
		debugf( NAME_DevNet, TEXT("Level server received: %s"), Text );
#endif
		if( ParseCommand(&Text,TEXT("HELLO")) )
		{
			// fail the connection if we are transitioning, as this could cause the client to load 2 maps at once
			if (GSeamlessTravelHandler.IsInTransition())
			{
				debugf( NAME_DevNet, TEXT("Client can't connect, in transition! Closing socket"));
				Connection->Close();
				return;
			}

			// Versions.
			INT RemoteMinVer=219, RemoteVer=219;
			Parse( Text, TEXT("MINVER="), RemoteMinVer );
			Parse( Text, TEXT("VER="),    RemoteVer    );
			if( RemoteVer<GEngineMinNetVersion || RemoteMinVer>GEngineVersion )
			{
				Connection->Logf( TEXT("UPGRADE MINVER=%i VER=%i"), GEngineMinNetVersion, GEngineVersion );
				Connection->FlushNet();
				Connection->State = USOCK_Closed;
				return;
			}
			Connection->NegotiatedVer = Min(RemoteVer,GEngineVersion);

			Connection->Challenge = FString::Printf(TEXT("%08X"),appCycles());
			Connection->Logf( TEXT("CHALLENGE VER=%i CHALLENGE=%s"), Connection->NegotiatedVer, *Connection->Challenge);
			Connection->FlushNet();
		}
		else if ( ParseCommand(&Text,TEXT("DEBUG")) )
		{
			// rjp -- simulate various network conditions
			// to test, type 'inject debug <subcommand>' at the command prompt on the client

			// simulate being disconnected as the result of attempting to connect to a full server
			if ( ParseCommand(&Text,TEXT("FULLSERVER")) )
			{
				// @fixme - doesn't exist
				FString Error = TEXT("Engine.GameMessage.MaxedOutMessage");

				debugf( NAME_DevNet, TEXT("Join failure: %s"), *Error );
				Connection->Logf( TEXT("FAILURE %s"), *Error );
				Connection->FlushNet();
				Connection->State = USOCK_Closed;
			}
			else if ( ParseCommand(&Text,TEXT("CHALLENGE")) )
			{
				debugf( NAME_DevNet, TEXT("Client %s failed CHALLENGE."), *Connection->LowLevelGetRemoteAddress() );
				Connection->Logf( TEXT("FAILURE CHALLENGE") );
				Connection->FlushNet();
				Connection->State = USOCK_Closed;
			}
		}
		else if( ParseCommand(&Text,TEXT("NETSPEED")) )
		{
			INT Rate = appAtoi(Text);
			Connection->CurrentNetSpeed = Clamp( Rate, 1800, NetDriver->MaxClientRate );
			debugf( TEXT("Client netspeed is %i"), Connection->CurrentNetSpeed );
		}
		else if( ParseCommand(&Text,TEXT("HAVE")) )
		{
			// Client specifying his generation.
			FGuid Guid(0,0,0,0);
			Parse( Text, TEXT("GUID=" ), Guid );
			UBOOL bFound = FALSE;
			for (TArray<FPackageInfo>::TIterator It(Connection->PackageMap->List); It; ++It)
			{
				if (It->Guid == Guid)
				{
					Parse( Text, TEXT("GEN=" ), It->RemoteGeneration );
					//@warning: it's important we compute here before executing any pending removal, so we're sure all object counts have been set
					Connection->PackageMap->Compute();
					// if the package was pending removal, we can do that now that we're synchronized
					if (Connection->PendingRemovePackageGUIDs.RemoveItem(Guid) > 0)
					{
						Connection->PackageMap->RemovePackage(It->Parent, FALSE);
					}
					bFound = TRUE;
					break;
				}
			}
			if (!bFound)
			{
				// the client specified a package with an incorrect GUID or one that it should not be using; kick it out
				Connection->Logf(TEXT("FAILURE"));
				Connection->Close();
			}
		}
		else if (ParseCommand(&Text, TEXT("ABORT")))
		{
			// client was verifying a package, but received an "UNLOAD" and aborted it
			FGuid Guid(0,0,0,0);
			Parse(Text, TEXT("GUID="), Guid);
			// if the package was pending removal, we can do that now that we're synchronized
			if (Connection->PendingRemovePackageGUIDs.RemoveItem(Guid) > 0)
			{
				Connection->PackageMap->RemovePackageByGuid(Guid);
			}
			else
			{
				/* @todo: this can happen and be valid if the server loads a package, unloads a package, then reloads that package again within a short span of time
							(e.g. streaming) - the package then won't be in the PendingRemovePackageGUIDs list, but the client may send an ABORT for the UNLOAD in between the two USES
							possibly can resolve by keeping track of how many responses we're expecting... or somethin
				debugf(NAME_DevNet, TEXT("Received ABORT with invalid GUID %s, disconnecting client"), *Guid.String());
				Connection->Logf(TEXT("FAILURE"));
				Connection->Close();
				*/
				debugf(NAME_DevNet, TEXT("Received ABORT with invalid GUID %s"), *Guid.String());
			}
		}
		else if( ParseCommand( &Text, TEXT("SKIP") ) )
		{
			FGuid Guid(0,0,0,0);
			Parse( Text, TEXT("GUID=" ), Guid );
			if( Connection->PackageMap )
			{
				for( INT i=0;i<Connection->PackageMap->List.Num();i++ )
					if( Connection->PackageMap->List(i).Guid == Guid )
					{
						debugf(NAME_DevNet, TEXT("User skipped download of '%s'"), *Connection->PackageMap->List(i).PackageName.ToString());
						Connection->PackageMap->List.Remove(i);
						break;
					}
			}
		}
		else if( ParseCommand(&Text,TEXT("LOGIN")) )
		{
			// Admit or deny the player here.
			if (!Parse(Text,TEXT("RESPONSE="), Connection->ClientResponse))
			{
				// Challenge failed, abort right now

				debugf( NAME_DevNet, TEXT("Client %s failed CHALLENGE."), *Connection->LowLevelGetRemoteAddress() );
				Connection->Logf( TEXT("FAILURE CHALLENGE") );
				Connection->FlushNet();
				Connection->State = USOCK_Closed;
				return;
			}
#if WITH_GAMESPY
			extern void appSubmitGameSpyAuthRequest(UNetConnection*);
			// Submit the async auth request
			appSubmitGameSpyAuthRequest(Connection);
#endif
			TCHAR Str[1024]=TEXT("");
			FString Error;
			Parse( Text, TEXT("URL="), Str, ARRAY_COUNT(Str) );
			Connection->RequestURL = Str;
			debugf( NAME_DevNet, TEXT("Login request: %s"), *Connection->RequestURL );
			const TCHAR* Tmp;
			for( Tmp=Str; *Tmp && *Tmp!='?'; Tmp++ );
			GetGameInfo()->eventPreLogin( Tmp, Connection->LowLevelGetRemoteAddress(), Error);
			if( Error!=TEXT("") )
			{
				debugf(NAME_DevNet, TEXT("PreLogin failure: %s"), *Error);
				Connection->Logf( TEXT("FAILURE %s"), *Error );
				Connection->Close();
				return;
			}
			WelcomePlayer( Connection );
		}
		else if( ParseCommand(&Text,TEXT("JOIN")) && !Connection->Actor )
		{
			// verify that the client informed us about all its packages
			/* @fixme: can't do this because if we're in the middle of loading, we could've recently sent some packages and therefore there's no way the client would pass this
			   @fixme: need another way to verify (timelimit?)
			for (INT i = 0; i < Connection->PackageMap->List.Num(); i++)
			{
				if (Connection->PackageMap->List(i).RemoteGeneration == 0)
				{
					FString Error = FString::Printf(TEXT("Join failure: failed to match package '%s'"), *Connection->PackageMap->List(i).PackageName.ToString());
					debugf(NAME_DevNet, *Error);
					Connection->Logf(TEXT("FAILURE %s"), *Error);
					Connection->FlushNet();
					Connection->State = USOCK_Closed;
					return;
				}
			}
			*/

			// Finish computing the package map.
			Connection->PackageMap->Compute();

			// Spawn the player-actor for this network player.
			FString Error;
			debugf( NAME_DevNet, TEXT("Join request: %s"), *Connection->RequestURL );
			Connection->Actor = SpawnPlayActor( Connection, ROLE_AutonomousProxy, FURL(NULL,*Connection->RequestURL,TRAVEL_Absolute), Error );
			if( !Connection->Actor )
			{
				// Failed to connect.
				debugf( NAME_DevNet, TEXT("Join failure: %s"), *Error );
				Connection->Logf( TEXT("FAILURE %s"), *Error );
				Connection->FlushNet();
				Connection->State = USOCK_Closed;
			}
			else
			{
				// Successfully in game.
				debugf( NAME_DevNet, TEXT("Join succeeded: %s"), *Connection->Actor->PlayerReplicationInfo->PlayerName );
				// if we're in the middle of a transition or the client is in the wrong world, tell it to travel
				FString LevelName;
				if (GSeamlessTravelHandler.IsInTransition())
				{
					// tell the client to go to the destination map
					LevelName = GSeamlessTravelHandler.GetDestinationMapName();
				}
				else if (!Connection->Actor->HasClientLoadedCurrentWorld())
				{
					// tell the client to go to our current map
					LevelName = FString(GWorld->GetOutermost()->GetName());
				}
				if (LevelName != TEXT(""))
				{
					Connection->Actor->eventClientTravel(LevelName, TRAVEL_Relative, TRUE);
				}
			}
		}
		// Handle server-side request for spawning a new controller using a child connection.
		else if (ParseCommand(&Text, TEXT("JOINSPLIT")))
		{
			FString SplitRequestURL = Connection->RequestURL;

			// create a child network connection using the existing connection for its parent
			check(Connection->GetUChildConnection() == NULL);
			UChildConnection* ChildConn = NetDriver->CreateChild(Connection);

			// create URL from string
			FURL URL(NULL, *SplitRequestURL, TRAVEL_Absolute);

			FString Error;
			debugf(NAME_DevNet, TEXT("JOINSPLIT: Join request: URL=%s"), *URL.String());
			APlayerController* PC = SpawnPlayActor(ChildConn, ROLE_AutonomousProxy, URL, Error, BYTE(Connection->Children.Num()));
			if (PC == NULL)
			{
				// Failed to connect.
				debugf(NAME_DevNet, TEXT("JOINSPLIT: Join failure: %s"), *Error);
				// remove the child connection
				Connection->Children.RemoveItem(ChildConn);
				// if any splitscreen viewport fails to join, all viewports on that client also fail
				Connection->Logf(TEXT("FAILURE %s"), *Error);
				Connection->FlushNet();
				Connection->State = USOCK_Closed;
			}
			else
			{
				// Successfully spawned in game.
				debugf(NAME_DevNet, TEXT("JOINSPLIT: Succeeded: %s"), *ChildConn->Actor->PlayerReplicationInfo->PlayerName);
			}
		}
		// PlayerController swap completion message
		else if (ParseCommand(&Text, TEXT("PCSWAP")))
		{
			UNetConnection* SwapConnection = Connection;
			INT ChildIndex = INDEX_NONE;
			if (Parse(Text, TEXT("CHILD="), ChildIndex) && ChildIndex >= 0)
			{
				SwapConnection = Connection->Children.IsValidIndex(ChildIndex) ? Connection->Children(ChildIndex) : NULL;
			}
			UBOOL bSuccess = FALSE;
			if (SwapConnection != NULL)
			{
				bSuccess = DestroySwappedPC(SwapConnection);
			}
			
			if (!bSuccess)
			{
				debugf(NAME_Warning, TEXT("Received invalid swap message with child index %i"), ChildIndex);
			}
		}
	}
}

//
// Called when a file receive is about to begin.
//
void UWorld::NotifyReceivedFile( UNetConnection* Connection, INT PackageIndex, const TCHAR* Error, UBOOL Skipped )
{
	appErrorf( TEXT("Level received unexpected file") );
}

//
// Called when other side requests a file.
//
UBOOL UWorld::NotifySendingFile( UNetConnection* Connection, FGuid Guid )
{
	if( NetDriver->ServerConnection )
	{
		// We are the client.
		debugf( NAME_DevNet, TEXT("Server requested file: Refused") );
		return 0;
	}
	else
	{
		// We are the server.
		debugf( NAME_DevNet, TEXT("Client requested file: Allowed") );
		return 1;
	}
}

//
// Start listening for connections.
//
UBOOL UWorld::Listen( FURL InURL, FString& Error)
{
#if WITH_UE3_NETWORKING
	if( NetDriver )
	{
		Error = LocalizeError(TEXT("NetAlready"),TEXT("Engine"));
		return FALSE;
	}

	// Create net driver.
	UClass* NetDriverClass = StaticLoadClass( UNetDriver::StaticClass(), NULL, TEXT("engine-ini:Engine.Engine.NetworkDevice"), NULL, LOAD_None, NULL );
	if (NetDriverClass == NULL)
	{
		debugf( TEXT("No driver class.") );
		return FALSE;
	}

	NetDriver = (UNetDriver*)StaticConstructObject( NetDriverClass );
	if( !NetDriver->InitListen( this, InURL, Error ) )
	{
		debugf( TEXT("Failed to listen: %s"), *Error );
		NetDriver = NULL;
		return FALSE;
	}
	static UBOOL LanPlay = ParseParam(appCmdLine(),TEXT("lanplay"));
	if ( !LanPlay && (NetDriver->MaxInternetClientRate < NetDriver->MaxClientRate) && (NetDriver->MaxInternetClientRate > 2500) )
	{
		NetDriver->MaxClientRate = NetDriver->MaxInternetClientRate;
	}

	if ( GetGameInfo() && GetGameInfo()->MaxPlayers > 16)
	{
		NetDriver->MaxClientRate = ::Min(NetDriver->MaxClientRate, 10000);
	}

	// Load everything required for network server support.
	if( !GUseSeekFreePackageMap )
	{
		BuildServerMasterMap();
	}
	else
	{
		UPackage::NetObjectNotifies.AddItem(NetDriver);
	}

	// Spawn network server support.
	GEngine->SpawnServerActors();

	// Set WorldInfo properties.
	GetWorldInfo()->NetMode = GEngine->Client ? NM_ListenServer : NM_DedicatedServer;
	GetWorldInfo()->NextSwitchCountdown = NetDriver->ServerTravelPause;
	debugf(TEXT("NetMode is now %d"),GetWorldInfo()->NetMode);
	return TRUE;
#else	//#if WITH_UE3_NETWORKING
	return TRUE;
#endif	//#if WITH_UE3_NETWORKING
}

/** @return whether this level is a client */
UBOOL UWorld::IsClient()
{
	return (GEngine->Client != NULL);
}

//
// Return whether this level is a server.
//
UBOOL UWorld::IsServer()
{
	return (!NetDriver || !NetDriver->ServerConnection) && (!DemoRecDriver || !DemoRecDriver->ServerConnection);
}

/**
 * Builds the master package map from the loaded levels, server packages,
 * and game type
 */
void UWorld::BuildServerMasterMap(void)
{
	check(NetDriver);
	NetDriver->MasterMap->AddNetPackages();
	UPackage::NetObjectNotifies.AddItem(NetDriver);
}

/**
 *	Sets the persistent FaceFXAnimSet to the given one...
 *
 *	@param	InPersistentFaceFXAnimSet	The anim set to set as the persistent one
 */
void UWorld::SetPersistentFaceFXAnimSet(UFaceFXAnimSet* InPersistentFaceFXAnimSet)
{
	if (InPersistentFaceFXAnimSet != PersistentFaceFXAnimSet)
	{
//		debugf(TEXT(">>>>>>>> SetPersistentFaceFXAnimSet> Was %s, now %s"),
//			PersistentFaceFXAnimSet ? *(PersistentFaceFXAnimSet->GetPathName()) : TEXT("NULL"),
//			InPersistentFaceFXAnimSet ? *(InPersistentFaceFXAnimSet->GetPathName()) : TEXT("NULL")
//			);
		if (PersistentFaceFXAnimSet)
		{
			UnmountPersistentFaceFXAnimSet();
		}
		PersistentFaceFXAnimSet = InPersistentFaceFXAnimSet;
		if (PersistentFaceFXAnimSet)
		{
			MountPersistentFaceFXAnimSet();
		}
	}
}

/**
 *	Finds the persistent FaceFXAnimSet and sets it if founc...
 */
void UWorld::FindAndSetPersistentFaceFXAnimSet()
{
	if (PersistentLevel)
	{
//		debugf(TEXT(">>>>>>>> FindAndSetPersistentFaceFXAnimSet called..."));
		// Find the PersistentFaceFXAnimSet and set it
		UObject* OutermostObj = PersistentLevel->GetOutermost();
		FString PFFXAnimSetName = OutermostObj->GetName() + TEXT("_FaceFXAnimSet");
		UFaceFXAnimSet* PFXXAnimSet = (UFaceFXAnimSet*) UObject::StaticFindObjectFast(UFaceFXAnimSet::StaticClass(), OutermostObj, FName(*PFFXAnimSetName));
/***
		if (PFXXAnimSet)
		{
			debugf(TEXT("\tFound FaceFXAnimSet %s"), *(PFXXAnimSet->GetPathName()));
		}
		else
		{
			debugf(TEXT("\tDid not find FaceFXAnimSet %s"), *PFFXAnimSetName);
		}
***/
		GWorld->SetPersistentFaceFXAnimSet(PFXXAnimSet);
	}
	else
	{
//		debugf(TEXT("\tNo FaceFXAnimSet found"));
	}
}

/**
 *	Iterates over the existing Pawns and mounts the current PersistentFaceFXAnimSet
 */
void UWorld::MountPersistentFaceFXAnimSet()
{
	if (PersistentFaceFXAnimSet != NULL)
	{
//		debugf(TEXT(">>>>>>>> MountPersistentFaceFXAnimSet> Mounting %s"), *(PersistentFaceFXAnimSet->GetPathName()));
		for (TObjectIterator<APawn> It; It; ++It)
		{
			APawn* Pawn = *It;

			if (Pawn->Mesh && Pawn->Mesh->SkeletalMesh && Pawn->Mesh->SkeletalMesh->FaceFXAsset)
			{
				Pawn->Mesh->SkeletalMesh->FaceFXAsset->MountFaceFXAnimSet(PersistentFaceFXAnimSet);
//				debugf(TEXT("Mounted %s on %s"), *(PersistentFaceFXAnimSet->GetName()), *(Pawn->GetName()));
			}
		}
	}
}

/**
 *	Iterates over the existing Pawns and unmounts the current PersistentFaceFXAnimSet
 */
void UWorld::UnmountPersistentFaceFXAnimSet()
{
	if (PersistentFaceFXAnimSet != NULL)
	{
//		debugf(TEXT(">>>>>>>> UnmountPersistentFaceFXAnimSet> Unmounting %s"), *(PersistentFaceFXAnimSet->GetPathName()));
		for (TObjectIterator<APawn> It; It; ++It)
		{
			APawn* Pawn = *It;

			if (Pawn->Mesh && Pawn->Mesh->SkeletalMesh && Pawn->Mesh->SkeletalMesh->FaceFXAsset)
			{
				Pawn->Mesh->SkeletalMesh->FaceFXAsset->UnmountFaceFXAnimSet(PersistentFaceFXAnimSet);
//				debugf(TEXT("Unmounted %s on %s"), *(PersistentFaceFXAnimSet->GetName()), *(Pawn->GetName()));
			}
		}
	}
}

/**
 *	Sets the persistent FaceFX animset on the given pawn...
 *
 *	@param	InPawn		The pawn to set it on.
 */
void UWorld::MountPersistentFaceFXAnimSetOnPawn(APawn* InPawn)
{
	if (PersistentFaceFXAnimSet != NULL)
	{
		if (InPawn->Mesh && InPawn->Mesh->SkeletalMesh && InPawn->Mesh->SkeletalMesh->FaceFXAsset)
		{
			InPawn->Mesh->SkeletalMesh->FaceFXAsset->MountFaceFXAnimSet(PersistentFaceFXAnimSet);
//			debugf(TEXT(">>>>>>>> MountPersistentFaceFXAnimSetOnPawn> Mounted %s on %s (%s)"), 
//				*(PersistentFaceFXAnimSet->GetName()), *(InPawn->GetName()),
//				*(InPawn->Mesh->SkeletalMesh->GetPathName()));
		}
	}
}

/** asynchronously loads the given levels in preparation for a streaming map transition.
 * @param LevelNames the names of the level packages to load. LevelNames[0] will be the new persistent (primary) level
 */
void AWorldInfo::PrepareMapChange(const TArray<FName>& LevelNames)
{
	// Only the game can do async map changes.
	UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
	if (GameEngine != NULL)
	{
		// Kick off async loading request for those maps.
		if( !GameEngine->PrepareMapChange(LevelNames) )
		{
			debugf(NAME_Warning,TEXT("Preparing map change via %s was not successful: %s"), *GetFullName(), *GameEngine->GetMapChangeFailureDescription() );
		}
	}
}

/** returns whether there's a map change currently in progress */
UBOOL AWorldInfo::IsPreparingMapChange()
{
	UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
	return (GameEngine != NULL) ? GameEngine->IsPreparingMapChange() : FALSE;
}

/** if there is a map change being prepared, returns whether that change is ready to be committed
 * (if no change is pending, always returns false)
 */
UBOOL AWorldInfo::IsMapChangeReady()
{
	UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
	return (GameEngine != NULL) ? GameEngine->IsReadyForMapChange() : FALSE;
}

void AWorldInfo::CancelPendingMapChange()
{
	UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
	if (GameEngine != NULL)
	{
		GameEngine->CancelPendingMapChange();
	}
}

/**
 * Actually performs the map transition prepared by PrepareMapChange().
 *
 * @param bShouldSkipLevelStartupEvent TRUE if this function NOT fire the SeqEvent_LevelBStartup event.
 * @param bShouldSkipLevelBeginningEvent TRUE if this function NOT fire the SeqEvent_LevelBeginning event. Useful for when skipping to the middle of a map by savegame or something
 */
void AWorldInfo::CommitMapChange(UBOOL bShouldSkipLevelStartupEvent, UBOOL bShouldSkipLevelBeginningEvent)
{
	// Only the game can do async map changes.
	UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
	if( GameEngine != NULL )
	{
		if( IsPreparingMapChange() )
		{
			GameEngine->bShouldCommitPendingMapChange				= TRUE;
		}
		else
		{
			debugf(TEXT("AWorldInfo::CommitMapChange being called without a pending map change!"));
		}
	}
}

/*-----------------------------------------------------------------------------
	Seamless world traveling
-----------------------------------------------------------------------------*/

extern void LoadMPSounds(UEngine* GameEngine);
extern void FreeMPSounds(UEngine* GameEngine);

extern void LoadMPGameContent(UEngine* GameEngine, const FURL& URL);
extern void FreeMPGameContent(UEngine* InGameEngine);

FSeamlessTravelHandler GSeamlessTravelHandler;

/** callback sent to async loading code to inform us when the level package is complete */
void FSeamlessTravelHandler::SeamlessTravelLoadCallback(UObject* LevelPackage, void* UserData)
{
	// defer until tick when it's safe to perform the transition
	FSeamlessTravelHandler* Handler = (FSeamlessTravelHandler*)UserData;
	if (Handler->IsInTransition())
	{
		Handler->LoadedPackage = LevelPackage;
		Handler->LoadedWorld = (UWorld*)UObject::StaticFindObjectFast(UWorld::StaticClass(), LevelPackage, NAME_TheWorld);
		if (Handler->LoadedWorld != NULL)
		{
			Handler->LoadedWorld->AddToRoot();
		}
	}
}

UBOOL FSeamlessTravelHandler::StartTravel(const FURL& InURL, const FGuid& InGuid)
{
	if (!InURL.Valid)
	{
		debugf(NAME_Error, TEXT("Invalid travel URL specified"));
		return FALSE;
	}
	else
	{
		debugf(TEXT("SeamlessTravel to: %s"), *InURL.Map);
		FString FileName;
		if (!GPackageFileCache->FindPackageFile(*InURL.Map, InGuid.IsValid() ? &InGuid : NULL, FileName))
		{
			debugf(NAME_Error, TEXT("Unable to travel to '%s' - file not found"), *InURL.Map);
			return FALSE;
			// @todo: might have to handle this more gracefully to handle downloading (might also need to send GUID and check it here!)
		}
		else
		{
			UBOOL bCancelledExisting = FALSE;
			if (IsInTransition())
			{
				if (PendingTravelURL.Map == InURL.Map)
				{
					// we are going to the same place so just replace the options
					PendingTravelURL = InURL;
					return TRUE;
				}
				debugf(NAME_Warning, TEXT("Cancelling travel to '%s' to go to '%s' instead"), *PendingTravelURL.Map, *InURL.Map);
				CancelTravel();
				bCancelledExisting = TRUE;
			}

			// Force a demo stop on level change.
			if (GWorld->DemoRecDriver != NULL)
			{
				GWorld->DemoRecDriver->Exec(TEXT("DEMOSTOP"), *GLog);
			}

			checkSlow(LoadedPackage == NULL);
			checkSlow(LoadedWorld == NULL);

			PendingTravelURL = InURL;
			PendingTravelGuid = InGuid;
			bSwitchedToDefaultMap = FALSE;
			bTransitionInProgress = TRUE;
			bPauseAtMidpoint = FALSE;
			bNeedCancelCleanUp = FALSE;

			FString DefaultMapName = FFilename(FURL::DefaultTransitionMap).GetBaseFilename();
			FName DefaultMapFinalName(*DefaultMapName);
			// if we're already in the default map, skip loading it and just go to the destination
			if (DefaultMapFinalName == GWorld->GetOutermost()->GetFName() ||
				DefaultMapFinalName == FName(*PendingTravelURL.Map))
			{
				debugf(TEXT("Already in default map or the default map is the destination, continuing to destination"));
				bSwitchedToDefaultMap = TRUE;
				if (bCancelledExisting)
				{
					// we need to fully finishing loading the old package and GC it before attempting to load the new one
					bPauseAtMidpoint = TRUE;
					bNeedCancelCleanUp = TRUE;
				}
				else
				{
					StartLoadingDestination();
				}
			}
			else
			{
				// first, load the entry level package
				UObject::LoadPackageAsync(DefaultMapName, &SeamlessTravelLoadCallback, this);
			}

			return TRUE;
		}
	}
}

/** cancels transition in progress */
void FSeamlessTravelHandler::CancelTravel()
{
	LoadedPackage = NULL;
	if (LoadedWorld != NULL)
	{
		LoadedWorld->RemoveFromRoot();
		LoadedWorld = NULL;
	}
	bTransitionInProgress = FALSE;
}

void FSeamlessTravelHandler::SetPauseAtMidpoint(UBOOL bNowPaused)
{
	if (!bTransitionInProgress)
	{
		debugf(NAME_Warning, TEXT("Attempt to pause seamless travel when no transition is in progress"));
	}
	else if (bSwitchedToDefaultMap && bNowPaused)
	{
		debugf(NAME_Warning, TEXT("Attempt to pause seamless travel after started loading final destination"));
	}
	else
	{
		bPauseAtMidpoint = bNowPaused;
		if (!bNowPaused && bSwitchedToDefaultMap)
		{
			// load the final destination now that we're done waiting
			StartLoadingDestination();
		}
	}
}

void FSeamlessTravelHandler::StartLoadingDestination()
{
	if (bTransitionInProgress && bSwitchedToDefaultMap)
	{
		if (GUseSeekFreeLoading)
		{
			// load gametype specific resources
			UBOOL bSeparateSharedMPSoundContent = FALSE;
			GConfig->GetBool( TEXT("Engine.Engine"), TEXT("bCookSeparateSharedMPSoundContent"), bSeparateSharedMPSoundContent, GEngineIni );
			UBOOL bSeparateSharedMPGameContent = FALSE;
			GConfig->GetBool( TEXT("Engine.Engine"), TEXT("bCookSeparateSharedMPGameContent"), bSeparateSharedMPGameContent, GEngineIni );
			if( bSeparateSharedMPSoundContent || bSeparateSharedMPGameContent )
			{
				const UBOOL bIsMultiplayerMap = PendingTravelURL.Map.StartsWith(TEXT("MP_"));
				if ( bIsMultiplayerMap )
				{
					debugf( NAME_Log, TEXT("LoadMap: %s: issuing load request for shared MP resources"), *PendingTravelURL.String() );
					if( bSeparateSharedMPSoundContent )
					{
						LoadMPSounds(GEngine);
					}
					if( bSeparateSharedMPGameContent )
					{
						LoadMPGameContent(GEngine,PendingTravelURL);
					}
				}
				else
				{
					debugf( NAME_Log, TEXT("SeamlessTravel: %s: freeing any shared MP resources"), *PendingTravelURL.String() );
					FreeMPSounds(GEngine);
					FreeMPGameContent(GEngine);
				}
			}

			// Only load localized package if it exists as async package loading doesn't handle errors gracefully.
			FString LocalizedPackageName = PendingTravelURL.Map + LOCALIZED_SEEKFREE_SUFFIX;
			FString LocalizedFileName;
			if (GPackageFileCache->FindPackageFile(*LocalizedPackageName, NULL, LocalizedFileName))
			{
				// Load localized part of level first in case it exists. We don't need to worry about GC or completion 
				// callback as we always kick off another async IO for the level below.
				UObject::LoadPackageAsync(*LocalizedPackageName, NULL, NULL);
			}
		}

		UObject::LoadPackageAsync(PendingTravelURL.Map, SeamlessTravelLoadCallback, this, PendingTravelGuid.IsValid() ? &PendingTravelGuid : NULL);
	}
	else
	{
		debugf(NAME_Error, TEXT("Called StartLoadingDestination() when not ready! bTransitionInProgress: %u bSwitchedToDefaultMap: %u"), bTransitionInProgress, bSwitchedToDefaultMap);
		checkSlow(0);
	}
}

void FSeamlessTravelHandler::Tick()
{
	if (bNeedCancelCleanUp)
	{
		if (UObject::IsAsyncLoading())
		{
			// allocate more time for async loading so we can clean up faster
			UObject::ProcessAsyncLoading(TRUE, 0.003f);
		}
		if (!UObject::IsAsyncLoading())
		{
			UObject::CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS, TRUE);
			bNeedCancelCleanUp = FALSE;
			SetPauseAtMidpoint(FALSE);
		}
	}
	else if (IsInTransition())
	{
		FLOAT ExtraTime = 0.003f;

		// on console, we're locked to 30 FPS, so if we actually spent less time than that put it all into loading faster
#if CONSOLE
		// Externs to detailed frame stats, split by render/ game thread CPU time and GPU time.
		extern DWORD GRenderThreadTime;
		extern DWORD GGameThreadTime;

		// Calculate the maximum time spent, EXCLUDING idle time. We don't use DeltaSeconds as it includes idle time waiting
		// for VSYNC so we don't know how much "buffer" we have.
		FLOAT FrameTime	= Max<DWORD>(GRenderThreadTime, GGameThreadTime) * GSecondsPerCycle;

		ExtraTime = Max<FLOAT>(ExtraTime, 0.0333f - FrameTime);
#endif

		// allocate more time for async loading during transition
		UObject::ProcessAsyncLoading(TRUE, ExtraTime);
	}
	//@fixme: wait for client to verify packages before finishing transition. Is this the right fix?
	if (LoadedPackage != NULL && (GWorld->GetNetDriver() == NULL || GWorld->GetNetDriver()->ServerConnection == NULL || GWorld->GetNetDriver()->ServerConnection->PendingPackageInfos.Num() == 0))
	{
		// find the new world
		UWorld* NewWorld = LoadedWorld;
		UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
		if (NewWorld == NULL || NewWorld->PersistentLevel == NULL)
		{
			debugf(NAME_Error, TEXT("Unable to travel to '%s' - package is not a level"), *LoadedPackage->GetName());
			// abort
			CancelTravel();
			if (GameEngine != NULL)
			{
				GameEngine->SetProgress(PMT_ConnectionFailure, LocalizeError(TEXT("NetOpen"),TEXT("Engine")), TEXT(""));
			}
		}
		else
		{
			// Dump and reset the FPS chart on map changes.
			GEngine->DumpFPSChart();
			GEngine->ResetFPSChart();

			// Dump and reset the Memory chart on map changes.
			GEngine->DumpMemoryChart();
			GEngine->ResetMemoryChart();

			AWorldInfo* OldWorldInfo = GWorld->GetWorldInfo();
			AWorldInfo* NewWorldInfo = NewWorld->GetWorldInfo();

			// clean up the old world
			for (INT i = 0; i < GWorld->PersistentLevel->GameSequences.Num(); i++)
			{
				// note that sequences for streaming levels are attached to the persistent level's sequence in AddToWorld(), so this should get everything
				USequence* Seq = GWorld->PersistentLevel->GameSequences(i);
				if (Seq != NULL)
				{
					Seq->CleanUp();
				}
			}

			// If desired, clear all AnimSet LinkupCaches.
			if(GameEngine && GameEngine->bClearAnimSetLinkupCachesOnLoadMap)
			{
				UAnimSet::ClearAllAnimSetLinkupCaches();
			}

			// Unmount the PersistentFaceFXAnimSet from all pawns
			GWorld->SetPersistentFaceFXAnimSet(NULL);

			// Make sure there are no pending visibility requests.
			GWorld->FlushLevelStreaming( NULL, TRUE );
			GWorld->TermWorldRBPhys();
			// only consider session ended if we're making the final switch so that HUD, scoreboard, etc. UI elements stay around until the end
			GWorld->CleanupWorld(bSwitchedToDefaultMap); 
			GWorld->RemoveFromRoot();
			// Stop all audio to remove references to old world
			if (GEngine->Client != NULL && GEngine->Client->GetAudioDevice() != NULL)
			{
				GEngine->Client->GetAudioDevice()->Flush( NULL );
			}

			// mark actors we want to keep

			// keep GameInfo if traveling to entry
			if (!bSwitchedToDefaultMap && OldWorldInfo->Game != NULL)
			{
				OldWorldInfo->Game->SetFlags(RF_Marked);
			}
			// always keep Controllers that belong to players
			if (GWorld->GetNetMode() == NM_Client)
			{
				for (FPlayerIterator It(GEngine); It; ++It)
				{
					if (It->Actor != NULL)
					{
						It->Actor->SetFlags(RF_Marked);
					}
				}
			}
			else
			{
				for (AController* C = OldWorldInfo->ControllerList; C != NULL; C = C->NextController)
				{
					if (C->bIsPlayer || C->GetAPlayerController() != NULL)
					{
						C->SetFlags(RF_Marked);
					}
				}
			}

			// ask the game class what else we should keep
			TArray<AActor*> KeepActors;
			if (OldWorldInfo->Game != NULL)
			{
				OldWorldInfo->Game->eventGetSeamlessTravelActorList(!bSwitchedToDefaultMap, KeepActors);
			}
			// ask players what else we should keep
			for (FPlayerIterator It(GEngine); It; ++It)
			{
				if (It->Actor != NULL)
				{
					It->Actor->eventGetSeamlessTravelActorList(!bSwitchedToDefaultMap, KeepActors);
				}
			}
			// mark all valid actors specified
			for (INT i = 0; i < KeepActors.Num(); i++)
			{
				if (KeepActors(i) != NULL)
				{
					KeepActors(i)->SetFlags(RF_Marked);
				}
			}

			// rename dynamic actors in the old world's PersistentLevel that we want to keep into the new world
			for (FActorIterator It; It; ++It)
			{
				// keep if it's dynamic and has been marked or we don't control it
				if (It->GetLevel() == GWorld->PersistentLevel && !It->bStatic && !It->bNoDelete && (It->HasAnyFlags(RF_Marked) || It->Role < ROLE_Authority))
				{
					It->ClearFlags(RF_Marked);
					It->Rename(NULL, NewWorld->PersistentLevel);
					It->WorldInfo = NewWorldInfo;
					// if it's a Controller or a Pawn, add it to the appopriate list in the new world's WorldInfo
					if (It->GetAController())
					{
						NewWorld->AddController((AController*)*It);
					}
					else if (It->GetAPawn())
					{
						NewWorld->AddPawn((APawn*)*It);
					}
					// add to new world's actor list and remove from old
					NewWorld->PersistentLevel->Actors.AddItem(*It);
					It.ClearCurrent();
				}
				else
				{
					// otherwise, set to be deleted
					It->ClearFlags(RF_Marked);
					It->MarkPendingKill();
					It->MarkComponentsAsPendingKill(FALSE);
					UNetDriver* NetDriver = GWorld->GetNetDriver();
					// close any channels for this actor
					if (NetDriver != NULL)
					{
						// server
						NetDriver->NotifyActorDestroyed(*It);
						// client
						if (NetDriver->ServerConnection != NULL)
						{
							// we can't kill the channel until the server says so, so just clear the actor ref and break the channel
							UActorChannel* Channel = NetDriver->ServerConnection->ActorChannels.FindRef(*It);
							if (Channel != NULL)
							{
								NetDriver->ServerConnection->ActorChannels.Remove(*It);
								Channel->Actor = NULL;
								Channel->Broken = TRUE;
							}
						}
					}
				}
			}

			// mark everything else contained in the world to be deleted
			for (TObjectIterator<UObject> It; It; ++It)
			{
				if (It->IsIn(GWorld))
				{
					It->MarkPendingKill();
				}
			}

			// copy over other info
			NewWorld->SetNetDriver(GWorld->GetNetDriver());
			if (NewWorld->GetNetDriver() != NULL)
			{
				NewWorld->GetNetDriver()->Notify = NewWorld;
			}
			NewWorldInfo->NetMode = GWorld->GetNetMode();
			NewWorldInfo->Game = OldWorldInfo->Game;
			NewWorldInfo->GRI = OldWorldInfo->GRI;
			NewWorldInfo->TimeSeconds = OldWorldInfo->TimeSeconds;
			NewWorldInfo->RealTimeSeconds = OldWorldInfo->RealTimeSeconds;
			NewWorldInfo->AudioTimeSeconds = OldWorldInfo->AudioTimeSeconds;

			// the new world should not be garbage collected
			NewWorld->AddToRoot();

			// collect garbage to delete the old world
			// because we marked everything in it pending kill, references will be NULL'ed so we shouldn't end up with any dangling pointers
			GWorld = NULL;
			UObject::CollectGarbage( GARBAGE_COLLECTION_KEEPFLAGS, TRUE );

#if CONSOLE
			appDefragmentTexturePool();
			appDumpTextureMemoryStats(TEXT(""));
#endif

			// set GWorld to the new world and initialize it
			GWorld = NewWorld;
			GWorld->Init();

#if !FINAL_RELEASE
			// verify that we successfully cleaned up the old world
			for (TObjectIterator<UWorld> It; It; ++It)
			{
				UWorld* World = *It;
				if (World != NewWorld)
				{
					// Print some debug information...
					UObject::StaticExec(*FString::Printf(TEXT("OBJ REFS CLASS=WORLD NAME=%s.TheWorld"), *World->GetOutermost()->GetName()));
					TMap<UObject*,UProperty*>	Route		= FArchiveTraceRoute::FindShortestRootPath( World, TRUE, GARBAGE_COLLECTION_KEEPFLAGS );
					FString						ErrorString	= FArchiveTraceRoute::PrintRootPath( Route, World );
					debugf(TEXT("%s"),*ErrorString);
					// before asserting.
					appErrorf( TEXT("%s not cleaned up by garbage collection!") LINE_TERMINATOR TEXT("%s") , *World->GetFullName(), *ErrorString );
				}
			}
#endif

			// GEMINI_TODO: A nicer precaching scheme.
			GPrecacheNextFrame = TRUE;

			// if we've already switched to entry before and this is the transition to the new map, re-create the gameinfo
			if (bSwitchedToDefaultMap && GWorld->GetNetMode() != NM_Client)
			{
				GWorld->SetGameInfo(PendingTravelURL);
			}

			// call begin play functions on everything that wasn't carried over from the old world
			GWorld->BeginPlay(PendingTravelURL, FALSE);

			// send loading complete notifications for all local players
			for (FPlayerIterator It(GEngine); It; ++It)
			{
				if (It->Actor != NULL)
				{
					It->Actor->eventNotifyLoadedWorld(GWorld->GetOutermost()->GetFName(), bSwitchedToDefaultMap);
					It->Actor->eventServerNotifyLoadedWorld(GWorld->GetOutermost()->GetFName());
				}
			}

			// we've finished the transition
			if (GameEngine != NULL)
			{
				GameEngine->bWorldWasLoadedThisTick = TRUE;
			}
			LoadedPackage = NULL;
			LoadedWorld = NULL; // don't need to remove from root as this is now GWorld
			if (bSwitchedToDefaultMap)
			{
				// we've now switched to the final destination, so we're done
				UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
				if (GameEngine != NULL)
				{
					// remember the last used URL
					GameEngine->LastURL = PendingTravelURL;
				}

				// Flag our transition as completed before we call PostSeamlessTravel.  This 
				// allows for chaining of maps.

				bTransitionInProgress = FALSE;

				if (NewWorldInfo->Game != NULL)
				{
					// inform the new GameInfo so it can handle players that persisted
					NewWorldInfo->Game->eventPostSeamlessTravel();
				}

				if ( GCallbackEvent )
				{
					GCallbackEvent->Send(CALLBACK_PostLoadMap);
				}
			}
			else
			{
				bSwitchedToDefaultMap = TRUE;
				if (!bPauseAtMidpoint)
				{
					StartLoadingDestination();
				}
			}
		}
	}
}

/** seamlessly travels to the given URL by first loading the entry level in the background,
 * switching to it, and then loading the specified level. Does not disrupt network communication or disconnet clients.
 * You may need to implement GameInfo::GetSeamlessTravelActorList(), PlayerController::GetSeamlessTravelActorList(),
 * GameInfo::PostSeamlessTravel(), and/or GameInfo::HandleSeamlessTravelPlayer() to handle preserving any information
 * that should be maintained (player teams, etc)
 * This codepath is designed for worlds that use little or no level streaming and gametypes where the game state
 * is reset/reloaded when transitioning. (like UT)
 * @param URL the URL to travel to; must be relative to the current URL (same server)
 * @param bAbsolute (opt) - if true, URL is absolute, otherwise relative
 * @param MapPackageGuid (opt) - the GUID of the map package to travel to - this is used to find the file when it has been autodownloaded,
 * 				so it is only needed for clients
 */
void AWorldInfo::SeamlessTravel(const FString& URL, UBOOL bAbsolute, FGuid MapPackageGuid)
{
	UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
	if (GameEngine != NULL)
	{
		// construct the URL
		FURL NewURL(&GameEngine->LastURL, *URL, bAbsolute ? TRAVEL_Absolute : TRAVEL_Relative);
		if (!NewURL.Valid)
		{
			FString Error = FString::Printf(LocalizeSecure(LocalizeError(TEXT("InvalidUrl"),TEXT("Engine")), *URL));
			debugf(NAME_Error, TEXT("SeamlessTravel(): %s"), *Error);
			GameEngine->SetProgress(PMT_ConnectionFailure, Error, TEXT(""));
		}
		else
		{
			if (NewURL.HasOption(TEXT("Restart")))
			{
				//@todo url cleanup - we should merge the two URLs, not completely replace it
				NewURL = GameEngine->LastURL;
			}
			// tell the handler to start the transition
			if (!GSeamlessTravelHandler.StartTravel(NewURL, MapPackageGuid) && !GSeamlessTravelHandler.IsInTransition())
			{
				GameEngine->SetProgress(PMT_ConnectionFailure, FString::Printf(LocalizeSecure(LocalizeError(TEXT("InvalidUrl"),TEXT("Engine")), *URL, TEXT(""))), TEXT(""));
			}
		}
	}
}

/** @return whether we're currently in a seamless transition */
UBOOL AWorldInfo::IsInSeamlessTravel()
{
	return GSeamlessTravelHandler.IsInTransition();
}

/** this function allows pausing the seamless travel in the middle,
 * right before it starts loading the destination (i.e. while in the transition level)
 * this gives the opportunity to perform any other loading tasks before the final transition
 * this function has no effect if we have already started loading the destination (you will get a log warning if this is the case)
 * @param bNowPaused - whether the transition should now be paused
 */
void AWorldInfo::SetSeamlessTravelMidpointPause(UBOOL bNowPaused)
{
	GSeamlessTravelHandler.SetPauseAtMidpoint(bNowPaused);
}

/**
 * @return the name of the map.  If Title isn't assigned, it will attempt to create the name from the full name
 */

FString AWorldInfo::GetMapName(UBOOL bIncludePrefix)
{
	if ( Title.Len() > 0 && !bIncludePrefix )
	{
		return Title;
	}
	else
	{
		// use the name of the map package
		FString MapName = GWorld->GetMapName();
		if ( !bIncludePrefix )
		{
			INT pos = MapName.InStr(TEXT("-"));
			if (pos>=0)
			{
				MapName = MapName.Mid(pos + 1);
			}
		}

// 		MapName = MapName.ToUpper();
		return MapName;
	}
}

BYTE AWorldInfo::GetDetailMode()
{
	return EDetailMode(GSystemSettings.DetailMode);
}

/** @return whether a demo is being recorded */
UBOOL AWorldInfo::IsRecordingDemo()
{
	return (GWorld->DemoRecDriver != NULL && GWorld->DemoRecDriver->ServerConnection == NULL);
}

UBOOL AWorldInfo::IsPlayingDemo()
{
	return (GWorld->DemoRecDriver != NULL && GWorld->DemoRecDriver->ServerConnection != NULL);
}

/** @return the current MapInfo that should be used. May return one of the inner StreamingLevels' MapInfo if a streaming level
 *	transition has occurred via PrepareMapChange()
 */
UMapInfo* AWorldInfo::GetMapInfo()
{
	AWorldInfo* CurrentWorldInfo = this;
	if ( StreamingLevels.Num() > 0 &&
		StreamingLevels(0)->LoadedLevel && 
		Cast<ULevelStreamingPersistent>(StreamingLevels(0)) )
	{
		CurrentWorldInfo = StreamingLevels(0)->LoadedLevel->GetWorldInfo();
	}
	return CurrentWorldInfo->MyMapInfo;
}

/** sets the current MapInfo to the passed in one */
void AWorldInfo::SetMapInfo(UMapInfo* NewMapInfo)
{
	AWorldInfo* CurrentWorldInfo = this;
	if ( StreamingLevels.Num() > 0 &&
		StreamingLevels(0)->LoadedLevel && 
		Cast<ULevelStreamingPersistent>(StreamingLevels(0)) )
	{
		CurrentWorldInfo = StreamingLevels(0)->LoadedLevel->GetWorldInfo();
	}
	CurrentWorldInfo->MyMapInfo = NewMapInfo;
}

void AWorldInfo::UpdateMusicTrack(FMusicTrackStruct NewMusicTrack)
{
	// if not attempting to play the same track
	if (MusicComp == NULL || NewMusicTrack.TheSoundCue != CurrentMusicTrack.TheSoundCue)
	{
		// if already playing a track
		if (MusicComp != NULL)
		{
			// otherwise fade out the current track
			MusicComp->FadeOut(CurrentMusicTrack.FadeOutTime,CurrentMusicTrack.FadeOutVolumeLevel);
		}
		// create a new audio component to play music
		MusicComp = UAudioDevice::CreateComponent( NewMusicTrack.TheSoundCue, GWorld->Scene, NULL, FALSE );
		if (MusicComp != NULL)
		{
			// update the new component with the correct settings
			MusicComp->bAutoDestroy = TRUE;
			MusicComp->bShouldRemainActiveIfDropped = TRUE;
			MusicComp->bIsMusic = TRUE;
			MusicComp->bAutoPlay = NewMusicTrack.bAutoPlay;
			MusicComp->bIgnoreForFlushing = NewMusicTrack.bPersistentAcrossLevels;

			// and finally fade in the new track
			MusicComp->FadeIn( NewMusicTrack.FadeInTime, NewMusicTrack.FadeInVolumeLevel );
		}

		// set the properties for future fades as well as replication to clients
		CurrentMusicTrack = NewMusicTrack;
		ReplicatedMusicTrack = NewMusicTrack;
		bNetDirty = TRUE;
	}
}


/**
 * @return Whether or not we can spawn more fractured chunks this frame
 **/
UBOOL AWorldInfo::CanSpawnMoreFracturedChunksThisFrame() const
{
	//if( NumFacturedChunksSpawnedThisFrame > 1 )
	//{
	//	warnf( TEXT( "Too Many Fractured Mesh Spawned@!!!!! %d" ), NumFacturedChunksSpawnedThisFrame );
	//}

	FWorldFractureSettings Settings = GetWorldFractureSettings();

	return NumFacturedChunksSpawnedThisFrame <= Settings.MaxNumFacturedChunksToSpawnInAFrame;
}

/** Get the current fracture settings for the loaded world - handles streaming correctly. */
FWorldFractureSettings AWorldInfo::GetWorldFractureSettings() const
{
	// If first level is a FakePersistentLevel (see CommitMapChange for more info)
	// then use its world info for fracture settings
	const AWorldInfo* CurrentWorldInfo = this;
	if( StreamingLevels.Num() > 0 &&
		StreamingLevels(0) &&
		StreamingLevels(0)->LoadedLevel && 
		StreamingLevels(0)->IsA(ULevelStreamingPersistent::StaticClass()) )
	{
		CurrentWorldInfo = StreamingLevels(0)->LoadedLevel->GetWorldInfo();
	}

	// Copy settings from relevant WorldInfo
	FWorldFractureSettings Settings;
	Settings.ChanceOfPhysicsChunkOverride			= CurrentWorldInfo->ChanceOfPhysicsChunkOverride;
	Settings.bEnableChanceOfPhysicsChunkOverride	= CurrentWorldInfo->bEnableChanceOfPhysicsChunkOverride;
	Settings.bLimitExplosionChunkSize				= CurrentWorldInfo->bLimitExplosionChunkSize;
	Settings.MaxExplosionChunkSize					= CurrentWorldInfo->MaxExplosionChunkSize;
	Settings.bLimitDamageChunkSize					= CurrentWorldInfo->bLimitDamageChunkSize;
	Settings.MaxDamageChunkSize						= CurrentWorldInfo->MaxDamageChunkSize;
	Settings.MaxNumFacturedChunksToSpawnInAFrame	= CurrentWorldInfo->MaxNumFacturedChunksToSpawnInAFrame;
	Settings.FractureExplosionVelScale				= CurrentWorldInfo->FractureExplosionVelScale;
	return Settings;
}


/**
* Determines whether a map is the default local map.
*
* @param	MapName	if specified, checks whether MapName is the default local map; otherwise, checks the currently loaded map.
*
* @return	TRUE if the map is the default local (or front-end) map.
*/
UBOOL AWorldInfo::IsMenuLevel( FString MapName/*=TEXT("")*/ )
{
	UBOOL bResult = FALSE;
	
	if (!GIsPlayInEditorWorld)
	{
		if ( MapName.Len() == 0 )
		{
			bResult = bIsMenuLevel;
		}
		else
		{
			bResult = FFilename(MapName).GetBaseFilename() == FFilename(FURL::DefaultLocalMap).GetBaseFilename();
		}
	}

	return bResult;
}

void AWorldInfo::execIsMenuLevel( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(MapName);
	P_FINISH;

	*(UBOOL*)Result = GWorld? GWorld->GetWorldInfo()->IsMenuLevel() : FALSE;
}

/*-----------------------------------------------------------------------------
	ULevelStreaming* implementation.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(ULevelStreaming);
IMPLEMENT_CLASS(ULevelStreamingKismet);
IMPLEMENT_CLASS(ULevelStreamingDistance);
IMPLEMENT_CLASS(ULevelStreamingPersistent);

/**
 * Returns whether this level should be present in memory which in turn tells the 
 * streaming code to stream it in. Please note that a change in value from FALSE 
 * to TRUE only tells the streaming code that it needs to START streaming it in 
 * so the code needs to return TRUE an appropriate amount of time before it is 
 * needed.
 *
 * @param ViewLocation	Location of the viewer
 * @return TRUE if level should be loaded/ streamed in, FALSE otherwise
 */
UBOOL ULevelStreaming::ShouldBeLoaded( const FVector& ViewLocation )
{
	return TRUE;
}

/**
 * Returns whether this level should be visible/ associated with the world if it
 * is loaded.
 * 
 * @param ViewLocation	Location of the viewer
 * @return TRUE if the level should be visible, FALSE otherwise
 */
UBOOL ULevelStreaming::ShouldBeVisible( const FVector& ViewLocation )
{
	if( GIsGame )
	{
		// Game and play in editor viewport codepath.
		return ShouldBeLoaded( ViewLocation );
	}
	else
	{
		// Editor viewport codepath.
		return bShouldBeVisibleInEditor;
	}
}

void ULevelStreaming::PostEditChange(FEditPropertyChain& PropertyThatChanged)
{
	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* OutermostProperty = PropertyThatChanged.GetHead()->GetValue();
		if ( OutermostProperty != NULL )
		{
			const FName PropertyName = OutermostProperty->GetFName();
			if ( PropertyName == TEXT("Offset") )
			{
				GWorld->UpdateLevelStreaming();
			}
		}
	}

	Super::PostEditChange( PropertyThatChanged );
}


/**
 * Returns whether this level should be visible/ associated with the world if it
 * is loaded.
 * 
 * @param ViewLocation	Location of the viewer
 * @return TRUE if the level should be visible, FALSE otherwise
 */
UBOOL ULevelStreamingKismet::ShouldBeVisible( const FVector& ViewLocation )
{
	return bShouldBeVisible || (bShouldBeVisibleInEditor && !GIsGame);
}

/**
 * Returns whether this level should be present in memory which in turn tells the 
 * streaming code to stream it in. Please note that a change in value from FALSE 
 * to TRUE only tells the streaming code that it needs to START streaming it in 
 * so the code needs to return TRUE an appropriate amount of time before it is 
 * needed.
 *
 * @param ViewLocation	Location of the viewer
 * @return TRUE if level should be loaded/ streamed in, FALSE otherwise
 */
UBOOL ULevelStreamingKismet::ShouldBeLoaded( const FVector& ViewLocation )
{
	return bShouldBeLoaded;
}


/**
 * Returns whether this level should be present in memory which in turn tells the 
 * streaming code to stream it in. Please note that a change in value from FALSE 
 * to TRUE only tells the streaming code that it needs to START streaming it in 
 * so the code needs to return TRUE an appropriate amount of time before it is 
 * needed.
 *
 * @param ViewLocation	Location of the viewer
 * @return TRUE if level should be loaded/ streamed in, FALSE otherwise
 */
UBOOL ULevelStreamingDistance::ShouldBeLoaded( const FVector& ViewLocation )
{
	return FDist( ViewLocation, Origin ) <= MaxDistance;
}


/** 
 * This will start a SentinelRun in the DB.  Setting up the Run table with all of metadata that a run has.
 * This will also set the GSentinelRunID so that the engine knows what the current run is.
 *
 * @param TaskDescription The name/description of the task that we are running
 * @param TaskParameter Any Parameters that the task needs
 * @param TagDesc A specialized tag (e.g. We are doing Task A with Param B and then "MapWithParticlesAdded" so it can be found)
 **/
void AGameInfo::BeginSentinelRun( const FString& TaskDescription, const FString& TaskParameter, const FString& TagDesc )
{
	extern const FString GetMapNameStatic();
	extern INT GScreenWidth;
	extern INT GScreenHeight;


	const FString BeginRun = FString::Printf( TEXT("EXEC BeginRun @PlatformName='%s', @MachineName='%s', @UserName='%s', @Changelist='%d', @GameName='%s', @ResolutionName='%s', @ConfigName='%s', @CmdLine='%s', @GameType='%s', @LevelName='%s', @TaskDescription='%s', @TaskParameter='%s', @Tag='%s'")
		//, appTimestamp() // need to pass in the date for when the compile was done  format:  03/18/08 20:35:48
		, *appGetPlatformString()
		, appComputerName()
		, appUserName() 
		, GetChangeListNumberForPerfTesting()
		, appGetGameName()
		, *FString::Printf(TEXT("%dx%d"), GScreenWidth, GScreenHeight ) // resolution is:  width by height
		, *GetConfigName()
		, appCmdLine()
		, *this->GetName()
		, *GetMapNameStatic()
		//	BeginSentinelRun( "TravelTheWorld", "TaskParameter", "TagDesc" );
		, *TaskDescription
		, *TaskParameter
		, *TagDesc
		);

	FDataBaseRecordSet* RecordSet = NULL;
	if( GTaskPerfMemDatabase->SendExecCommandRecordSet( *BeginRun, RecordSet ) && RecordSet )
	{
		// Retrieve RunID from recordset. It's the return value of the EXEC.
		GSentinelRunID = RecordSet->GetInt(TEXT("Return Value"));
		//warnf( TEXT("RunID: %d"), GSentinelRunID );
	}

	warnf( TEXT("%s %d"), *BeginRun, GSentinelRunID );

	delete RecordSet;
	RecordSet = NULL;
}


/**
 * This will output some set of data that we care about when we are doing Sentinel runs while we are
 * doing a MP test or a BVT. 
 * Prob just stat unit and some other random stats (streaming fudge factor and such)
 **/
void AGameInfo::AddSentinelPerTimePeriodStats( const FVector InLocation, const FRotator InRotation )
{
	if( GSentinelRunID != - 1 )
	{
		PerfMem Datum( InLocation, InRotation );
		Datum.AddPerfStatsForLocationRotation_TimePeriod();
	}
}

/** 
 * Add the audio related stats to the database
 */
void AGameInfo::HandlePerLoadedMapAudioStats()
{
	// Get the memory used by each sound group
	if( GEngine && GEngine->Client )
	{
		TMap<FName, FAudioGroupInfo> AudioGroupInfos;

		UAudioDevice* AudioDevice = GEngine->Client->GetAudioDevice();
		AudioDevice->GetSoundGroupInfo( AudioGroupInfos );

		for( TMap<FName, FAudioGroupInfo>::TIterator AGIIter( AudioGroupInfos ); AGIIter; ++AGIIter )
		{
			FName SoundClassFName = AGIIter.Key();
			FString SoundClassName = FString::Printf( TEXT( "SC_%s" ), *SoundClassFName.ToString() );
			FAudioGroupInfo* AGI = AudioGroupInfos.Find( SoundClassFName );

			extern const FString GetNonPersistentMapNameStatic();
			FString AddRunData = FString::Printf( TEXT( "EXEC AddRunData @RunID=%i, @StatGroupName='%s', @StatName='%s', @StatValue=%f, @SubLevelName='%s'" )  
				, GSentinelRunID  
				, TEXT( "SoundClass" )  
				, *SoundClassName
				, AGI->SizeResident / 1024.0f 
				, *GetNonPersistentMapNameStatic()
				);  

			GTaskPerfMemDatabase->SendExecCommand( *AddRunData ); 
		}
	}
}

/** 
 * This will run on every map load.  (e.g. You have P map which consists of N sublevels.  For each SubLevel this will run. 
 **/
void AGameInfo::DoSentinelActionPerLoadedMap()
{
	HandlePerLoadedMapAudioStats();
}

/** 
 * This will tell the DB to end the current Sentinel run (i.e. GSentinelRunID) and set that Run's RunResult to the passed in var.
 *
 * @param RunResult The result of this Sentinel run (e.g. OOM, Passed, etc.)
 **/
void AGameInfo::EndSentinelRun( BYTE RunResult )
{
	if( GSentinelRunID != - 1 )
	{
		const FString EndRun = FString::Printf(TEXT("EXEC EndRun @RunID=%i, @ResultDescription='%s'")
			, GSentinelRunID
			, *PerfMemRunResultStrings[RunResult] 
		);

		//warnf( TEXT("%s"), *EndRun );
		GTaskPerfMemDatabase->SendExecCommand( *EndRun );
	}
}

// TODO:  need to convert this into .ini for stats to gather so we don't have to have this code madness and LDs could do some sweeet sweetness of choosing which stats they want to see
// pain:  need to have some scaling factor attached to each  (e.g. for the memory stats so we don't jsut see bytes which are impossible to understand

void AGameInfo::DoSentinel_MemoryAtSpecificLocation( const FVector InLocation, const FRotator InRotation )
{
	PerfMem Datum( InLocation, InRotation );
	//Datum.WriteInsertSQLToBatFile();
	Datum.AddMemoryStatsForLocationRotation();
}


void AGameInfo::DoSentinel_PerfAtSpecificLocation( const FVector InLocation, const FRotator InRotation )
{
	PerfMem Datum( InLocation, InRotation );
	//Datum.WriteInsertSQLToBatFile();
	Datum.AddPerfStatsForLocationRotation();
}

namespace
{
	TArray<FVector> SentinelCoverLinks;
	TArray<FVector> SentinelPlayerStarts;
	TArray<FVector> SentinelPathNodes;
	TArray<FVector> SentinelPickUpFactories;
}

void AGameInfo::GetTravelLocations( FName LevelName, APlayerController* PC, TArray<FVector>& TravelPoints )
{
	warnf( TEXT("GetTravelLocations %d"), GWorld->Levels.Num() );

	AWorldInfo* WorldInfo = GWorld->GetWorldInfo();



	for( INT RealLevelIdx = 0; RealLevelIdx < GWorld->Levels.Num(); ++RealLevelIdx )
	{
		warnf( TEXT("Level: %d %s %d"), GWorld->Levels.Num(), *GWorld->Levels(RealLevelIdx)->GetOutermost()->GetName(), GWorld->Levels(RealLevelIdx)->Actors.Num() );


		for( INT ActorIdx = 0; ActorIdx < GWorld->Levels(RealLevelIdx)->Actors.Num(); ++ActorIdx )
		{
			AActor* Actor = GWorld->Levels(RealLevelIdx)->Actors(ActorIdx);

			//ANavigationPoint* NavPoint = Cast<ANavigationPoint>(Actor);
			ACoverLink* CoverLink = Cast<ACoverLink>(Actor);
			APlayerStart* PlayerStart = Cast<APlayerStart>(Actor);
			APathNode* PathNode = Cast<APathNode>(Actor);
			APickupFactory* PickUpFactory = Cast<APickupFactory>(Actor);

			if( ( CoverLink != NULL ) || ( PathNode != NULL ) || ( PickUpFactory != NULL ) || ( PlayerStart != NULL ) )
			{
				//warnf( TEXT("Adding NavPoint: %s"), *Actor->GetName() );
				TravelPoints.AddUniqueItem( Actor->Location );
			}

			if( CoverLink != NULL )
			{
				SentinelCoverLinks.AddUniqueItem( Actor->Location );
			}

			if( PlayerStart != NULL ) 
			{
				SentinelPathNodes.AddUniqueItem( Actor->Location );
			}

			if( PathNode != NULL ) 
			{
				SentinelPathNodes.AddUniqueItem( Actor->Location );
				//TravelPoints.AddUniqueItem( Actor->Location );
			}

			if( PickUpFactory != NULL ) 
			{
				SentinelPickUpFactories.AddUniqueItem( Actor->Location );
			}

		}
	}

	// when we return to the .uc land we will set our PC to bIsUsingStreamingVolumes = TRUE 
	warnf( TEXT( "Total Number of TravelPoints: %d  SentinelCoverLinks: %d  SentinelPlayerStarts: %d  SentinelPathNodes: %d  SentinelPickUpFactories: %d" ), TravelPoints.Num(), SentinelCoverLinks.Num(), SentinelPlayerStarts.Num(), SentinelPathNodes.Num(), SentinelPickUpFactories.Num() );

}
