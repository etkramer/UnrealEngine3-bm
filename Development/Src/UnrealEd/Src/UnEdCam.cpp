/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "ScopedTransaction.h"
#include "DlgMapCheck.h"
#include "EnginePrefabClasses.h"
#include "DlgActorSearch.h"

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

// Used for actor rotation gizmo.
INT GARGAxis = -1;
UBOOL GbARG = 0;

// Click flags.
enum EViewportClick
{
	CF_MOVE_ACTOR	= 1,	// Set if the actors have been moved since first click
	CF_MOVE_TEXTURE = 2,	// Set if textures have been adjusted since first click
	CF_MOVE_ALL     = (CF_MOVE_ACTOR | CF_MOVE_TEXTURE),
};

/*-----------------------------------------------------------------------------
   Change transacting.
-----------------------------------------------------------------------------*/

//
// If this is the first time called since first click, note all selected actors.
//
void UUnrealEdEngine::NoteActorMovement()
{
	if( !GUndo && !(GEditor->ClickFlags & CF_MOVE_ACTOR) )
	{
		GEditor->ClickFlags |= CF_MOVE_ACTOR;

		const FScopedTransaction Transaction( *LocalizeUnrealEd("ActorMovement") );
		GEditorModeTools().Snapping=0;
		
		AActor* SelectedActor = NULL;
		for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
		{
			AActor* Actor = static_cast<AActor*>( *It );
			checkSlow( Actor->IsA(AActor::StaticClass()) );

			SelectedActor = Actor;
			break;
		}

		if( SelectedActor == NULL )
		{
			USelection* SelectedActors = GetSelectedActors();
			SelectedActors->Modify();
			SelectActor( GWorld->GetBrush(), TRUE, NULL, TRUE );
		}

		// Look for an actor that requires snapping.
		for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
		{
			AActor* Actor = static_cast<AActor*>( *It );
			checkSlow( Actor->IsA(AActor::StaticClass()) );

			if ( Actor->bEdShouldSnap )
			{
				GEditorModeTools().Snapping = 1;
				break;
			}
		}

		// Modify selected actors.
		for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
		{
			AActor* Actor = static_cast<AActor*>( *It );
			checkSlow( Actor->IsA(AActor::StaticClass()) );

			Actor->Modify();
			if ( Actor->IsABrush() )
			{
				ABrush* Brush = (ABrush*)Actor;
				if( Brush->Brush )
				{
					Brush->Brush->Polys->Element.ModifyAllItems();
				}
			}
		}
	}
}

/** Finish snapping all actors. */
void UUnrealEdEngine::FinishAllSnaps()
{
	if(!GIsUCC)
	{
		if( ClickFlags & CF_MOVE_ACTOR )
		{
			ClickFlags &= ~CF_MOVE_ACTOR;

			for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
			{
				AActor* Actor = static_cast<AActor*>( *It );
				checkSlow( Actor->IsA(AActor::StaticClass()) );

				Actor->Modify();
				Actor->InvalidateLightingCache();
				Actor->PostEditMove( TRUE );
			}
		}
	}
}

/**
 * Cleans up after major events like e.g. map changes.
 *
 * @param	ClearSelection	Whether to clear selection
 * @param	Redraw			Whether to redraw viewports
 * @param	TransReset		Human readable reason for resetting the transaction system
 */
void UUnrealEdEngine::Cleanse( UBOOL ClearSelection, UBOOL Redraw, const TCHAR* TransReset )
{
	if( GIsRunning && !Bootstrapping )
	{
		// Release any object references held by editor dialogs.
		GApp->DlgActorSearch->Clear();
		GApp->DlgMapCheck->ClearMessageList();
	}

	Super::Cleanse( ClearSelection, Redraw, TransReset );
}

//
// Get the editor's pivot location
//
FVector UUnrealEdEngine::GetPivotLocation()
{
	return GEditorModeTools().PivotLocation;
}

/**
 * Sets the editor's pivot location, and optionally the pre-pivots of actors.
 *
 * @param	NewPivot				The new pivot location
 * @param	bSnapPivotToGrid		If TRUE, snap the new pivot location to the grid.
 * @param	bAssignToPrePivot		If TRUE, assign the new pivot location to the pre-pivot of selected actors.
 * @param	bIgnoreAxis				If TRUE, leave the existing pivot unaffected for components of NewPivot that are 0.
 */
void UUnrealEdEngine::SetPivot( FVector NewPivot, UBOOL bSnapPivotToGrid, UBOOL bAssignToPrePivot, UBOOL bIgnoreAxis )
{
	FEditorModeTools& EditorModeTools = GEditorModeTools();

	if( !bIgnoreAxis )
	{
		// Don't stomp on orthonormal axis.
		if( NewPivot.X==0 ) NewPivot.X=EditorModeTools.PivotLocation.X;
		if( NewPivot.Y==0 ) NewPivot.Y=EditorModeTools.PivotLocation.Y;
		if( NewPivot.Z==0 ) NewPivot.Z=EditorModeTools.PivotLocation.Z;
	}

	// Set the pivot.
	EditorModeTools.PivotLocation   = NewPivot;
	EditorModeTools.GridBase        = FVector(0,0,0);
	EditorModeTools.SnappedLocation = NewPivot;

	if( bSnapPivotToGrid )
	{
		FRotator DummyRotator(0,0,0);
		Constraints.Snap( EditorModeTools.SnappedLocation, EditorModeTools.GridBase, DummyRotator );
		EditorModeTools.PivotLocation = EditorModeTools.SnappedLocation;
	}

	// Check all actors.
	INT Count=0, SnapCount=0;

	for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		Count++;
		SnapCount += Actor->bEdShouldSnap;

		if ( bAssignToPrePivot && Actor->IsBrush() )
		{
			Actor->Modify();
			const FVector Delta( EditorModeTools.SnappedLocation - Actor->Location );
			Actor->Location += Delta;
			Actor->PrePivot += Delta;
			Actor->PostEditMove( TRUE );
		}
	}

	// Update showing.
	EditorModeTools.PivotShown = SnapCount>0 || Count>1;
}

//
// Reset the editor's pivot location.
//
void UUnrealEdEngine::ResetPivot()
{
	GEditorModeTools().PivotShown = 0;
	GEditorModeTools().Snapping   = 0;
}

/*-----------------------------------------------------------------------------
	Selection.
-----------------------------------------------------------------------------*/

//
// Selection change.
//
void UUnrealEdEngine::NoteSelectionChange()
{
	// Notify the editor.
	GCallbackEvent->Send( CALLBACK_SelChange );

	// Pick a new common pivot, or not.
	INT Count=0;
	AActor* SingleActor=NULL;

	for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		SingleActor = Actor;
		Count++;
	}
	
	if( Count==0 ) 
	{
		ResetPivot();
	}
	else
	{
		SetPivot( SingleActor->Location, FALSE, FALSE, TRUE );
	}

	GEditorModeTools().GetCurrentMode()->ActorSelectionChangeNotify();

	UpdatePropertyWindows();

	RedrawLevelEditingViewports();
}

/**
 * Selects/deselects and actor.
 */
void UUnrealEdEngine::SelectActor(AActor* Actor, UBOOL bInSelected, FViewportClient* InViewportClient, UBOOL bNotify, UBOOL bSelectEvenIfHidden)
{
	// Recursion guard, set when selecting parts of prefabs.
	static UBOOL bIteratingOverPrefabActors = FALSE;

	// If selections are globally locked, leave.
	if( GEdSelectionLock )
	{
		return;
	}

	// Only abort from hidden actors if we are selecting. You can deselect hidden actors without a problem.
	if ( bInSelected && !bSelectEvenIfHidden )
	{
		// If the actor is NULL or hidden, leave.
		if( !Actor || Actor->IsHiddenEd() )
		{
			return;
		}
	}

	// Select the actor and update its internals.
	if( !GEditorModeTools().GetCurrentMode()->Select( Actor, bInSelected ) )
	{
		// There's no need to recursively look for parts of prefabs, because actors can belong to a single prefab only.
		if ( !bIteratingOverPrefabActors )
		{
			// Select parts of prefabs if "prefab lock" is enabled, or if this actor is a prefab.
			if ( bPrefabsLocked || Cast<APrefabInstance>(Actor) )
			{
				APrefabInstance* PrefabInstance = Actor->FindOwningPrefabInstance();
				if ( PrefabInstance )
				{
					bIteratingOverPrefabActors = TRUE;

					// Select the prefab instance itself, if we're not already selecting it.
					if ( Actor != PrefabInstance )
					{
						SelectActor( PrefabInstance, bInSelected, InViewportClient, FALSE, TRUE );
					}

					// Select the prefab parts.
					TArray<AActor*> ActorsInPrefabInstance;
					PrefabInstance->GetActorsInPrefabInstance( ActorsInPrefabInstance );
					for( INT ActorIndex = 0 ; ActorIndex < ActorsInPrefabInstance.Num() ; ++ActorIndex )
					{
						AActor* PrefabActor = ActorsInPrefabInstance(ActorIndex);
						SelectActor( PrefabActor, bInSelected, InViewportClient, FALSE, TRUE );
					}

					bIteratingOverPrefabActors = FALSE;
				}
			}
		}

		// Don't do any work if the actor's selection state is already the selected state.
		const UBOOL bActorSelected = Actor->IsSelected();
		if ( (bActorSelected && !bInSelected) || (!bActorSelected && bInSelected) )
		{
			GetSelectedActors()->Select( Actor, bInSelected );
			Actor->ForceUpdateComponents(FALSE,FALSE);
			if( bNotify )
			{
				NoteSelectionChange();
			}
		}
	}

	// If we are locking selections to the camera, move the viewport camera to the selected actors location.
	if( bInSelected && GCurrentLevelEditingViewportClient && GCurrentLevelEditingViewportClient->bLockSelectedToCamera )
	{
		GCurrentLevelEditingViewportClient->ViewLocation = Actor->Location;
		GCurrentLevelEditingViewportClient->ViewRotation = Actor->Rotation;
	}
}

/**
 * Selects or deselects a BSP surface in the persistent level's UModel.  Does nothing if GEdSelectionLock is TRUE.
 *
 * @param	InModel					The model of the surface to select.
 * @param	iSurf					The index of the surface in the persistent level's UModel to select/deselect.
 * @param	bSelected				If TRUE, select the surface; if FALSE, deselect the surface.
 * @param	bNoteSelectionChange	If TRUE, call NoteSelectionChange().
 */
void UUnrealEdEngine::SelectBSPSurf(UModel* InModel, INT iSurf, UBOOL bSelected, UBOOL bNoteSelectionChange)
{
	if( GEdSelectionLock )
	{
		return;
	}

	FBspSurf& Surf = InModel->Surfs( iSurf );
	InModel->ModifySurf( iSurf, FALSE );

	if( bSelected )
	{
		Surf.PolyFlags |= PF_Selected;
	}
	else
	{
		Surf.PolyFlags &= ~PF_Selected;
	}

	if( bNoteSelectionChange )
	{
		NoteSelectionChange();
	}
}

/**
 * Deselects all BSP surfaces in the specified level.
 *
 * @param	Level		The level for which to deselect all levels.
 */
static void DeselectAllSurfacesForLevel(ULevel* Level)
{
	if ( Level )
	{
		UModel* Model = Level->Model;
		for( INT SurfaceIndex = 0 ; SurfaceIndex < Model->Surfs.Num() ; ++SurfaceIndex )
		{
			FBspSurf& Surf = Model->Surfs(SurfaceIndex);
			if( Surf.PolyFlags & PF_Selected )
			{
				Model->ModifySurf( SurfaceIndex, FALSE );
				Surf.PolyFlags &= ~PF_Selected;
			}
		}
	}
}

/**
 * Deselect all actors.  Does nothing if GEdSelectionLock is TRUE.
 *
 * @param	bNoteSelectionChange		If TRUE, call NoteSelectionChange().
 * @param	bDeselectBSPSurfs			If TRUE, also deselect all BSP surfaces.
 */
void UUnrealEdEngine::SelectNone(UBOOL bNoteSelectionChange, UBOOL bDeselectBSPSurfs)
{
	if( GEdSelectionLock )
	{
		return;
	}

	USelection* SelectedActors = GetSelectedActors();
	SelectedActors->Modify();

	// Make a list of selected actors . . .
	TArray<AActor*> ActorsToDeselect;
	for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		ActorsToDeselect.AddItem( Actor );
	}

	// . . . and deselect them.
	for ( INT ActorIndex = 0 ; ActorIndex < ActorsToDeselect.Num() ; ++ActorIndex )
	{
		AActor* Actor = ActorsToDeselect( ActorIndex );
		SelectActor( Actor, FALSE, NULL, FALSE );
	}

	if( bDeselectBSPSurfs )
	{
		// Unselect all surfaces in all levels.
		DeselectAllSurfacesForLevel( GWorld->PersistentLevel );
		AWorldInfo*	WorldInfo = GWorld->GetWorldInfo();
		for( INT LevelIndex = 0 ; LevelIndex < WorldInfo->StreamingLevels.Num() ; ++LevelIndex )
		{
			ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
			if( StreamingLevel )
			{
				DeselectAllSurfacesForLevel( StreamingLevel->LoadedLevel );
			}
		}
	}

	GetSelectedActors()->DeselectAll();

	if( bNoteSelectionChange )
	{
		NoteSelectionChange();
	}
}
