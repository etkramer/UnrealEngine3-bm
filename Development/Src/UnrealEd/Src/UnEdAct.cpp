/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "Factories.h"
#include "EngineSequenceClasses.h"
#include "EnginePrefabClasses.h"
#include "EnginePhysicsClasses.h"
#include "EngineMeshClasses.h"
#include "EngineAnimClasses.h"
#include "UnTerrain.h"
#include "UnLinkedObjEditor.h"
#include "Kismet.h"
#include "BusyCursor.h"
#include "DlgGenericComboEntry.h"
#include "SpeedTree.h"
#include "BSPOps.h"

#pragma DISABLE_OPTIMIZATION /* Not performance-critical */

static INT RecomputePoly( ABrush* InOwner, FPoly* Poly )
{
	// force recalculation of normal, and texture U and V coordinates in FPoly::Finalize()
	Poly->Normal = FVector(0,0,0);

	// catch normalization exceptions to warn about non-planar polys
	try
	{
		return Poly->Finalize( InOwner, 0 );
	}
	catch(...)
	{
		debugf( TEXT("WARNING: FPoly::Finalize() failed (You broke the poly!)") );
	}

	return 0;
}

/*-----------------------------------------------------------------------------
   Actor adding/deleting functions.
-----------------------------------------------------------------------------*/

/**
 * Copy selected actors to the clipboard.  Does not copy PrefabInstance actors or parts of Prefabs.
 *
 * @param	bReselectPrefabActors	If TRUE, reselect any actors that were deselected prior to export as belonging to prefabs.
 * @param	bClipPadCanBeUsed		If TRUE, the clip pad is available for use if the user is holding down SHIFT.
 */
void UUnrealEdEngine::edactCopySelected(UBOOL bReselectPrefabActors, UBOOL bClipPadCanBeUsed)
{
	// Before copying, deselect:
	//		- Actors belonging to prefabs unless all actors in the prefab are selected.
	//		- Builder brushes.
	TArray<AActor*> ActorsToDeselect;

	UBOOL bSomeSelectedActorsNotInCurrentLevel = FALSE;
	for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		// Deselect any selected builder brushes.
		const UBOOL bActorIsBuilderBrush = (Actor->IsABrush() && Actor == GWorld->GetBrush());
		if( bActorIsBuilderBrush || Actor->IsInPrefabInstance() )
		{
			ActorsToDeselect.AddItem(Actor);
		}

		// If any selected actors are not in the current level, warn the user that some actors will not be copied.
		if ( !bSomeSelectedActorsNotInCurrentLevel && Actor->GetLevel() != GWorld->CurrentLevel )
		{
			bSomeSelectedActorsNotInCurrentLevel = TRUE;
			appMsgf( AMT_OK, *LocalizeUnrealEd("CopySelectedActorsInNonCurrentLevel") );
		}
	}

	const FScopedBusyCursor BusyCursor;
	for( INT ActorIndex = 0 ; ActorIndex < ActorsToDeselect.Num() ; ++ActorIndex )
	{
		AActor* Actor = ActorsToDeselect(ActorIndex);
		GetSelectedActors()->Deselect( Actor );
	}

	// Export the actors.
	FStringOutputDevice Ar;
	const FExportObjectInnerContext Context;
	UExporter::ExportToOutputDevice( &Context, GWorld, NULL, Ar, TEXT("copy"), 0, PPF_DeepCompareInstances | PPF_ExportsNotFullyQualified);
	appClipboardCopy( *Ar );

	// If the clip pad is being used...
	if( bClipPadCanBeUsed && (GetAsyncKeyState(VK_SHIFT) & 0x8000) )
	{	
		PasteClipboardIntoClipPad();
	}

	// Reselect actors that were deselected for being or belonging to prefabs.
	if ( bReselectPrefabActors )
	{
		for( INT ActorIndex = 0 ; ActorIndex < ActorsToDeselect.Num() ; ++ActorIndex )
		{
			AActor* Actor = ActorsToDeselect(ActorIndex);
			GetSelectedActors()->Select( Actor );
		}
	}
}



/**
 * Creates offsets for locations based on the editor grid size and active viewport.
 */
static FVector CreateLocationOffset(UBOOL bDuplicate, UBOOL bOffsetLocations)
{
	const FLOAT Offset = static_cast<FLOAT>( bOffsetLocations ? GEditor->Constraints.GetGridSize() : 0 );
	FVector LocationOffset(Offset,Offset,Offset);
	if ( bDuplicate && GCurrentLevelEditingViewportClient )
	{
		switch( GCurrentLevelEditingViewportClient->ViewportType )
		{
		case LVT_OrthoXZ:
			LocationOffset = FVector(Offset,0.f,Offset);
			break;
		case LVT_OrthoYZ:
			LocationOffset = FVector(0.f,Offset,Offset);
			break;
		default:
			LocationOffset = FVector(Offset,Offset,0.f);
			break;
		}
	}
	return LocationOffset;
}

/**
 * Paste selected actors from the clipboard.
 *
 * @param	bDuplicate			Is this a duplicate operation (as opposed to a real paste)?
 * @param	bOffsetLocations	Should the actor locations be offset after they are created?
 * @param	bClipPadCanBeUsed	If TRUE, the clip pad is available for use if the user is holding down SHIFT.
*/
void UUnrealEdEngine::edactPasteSelected(UBOOL bDuplicate, UBOOL bOffsetLocations, UBOOL bClipPadCanBeUsed)
{
	// If the clip pad is the source of the pasted text...

	if( bClipPadCanBeUsed && (GetAsyncKeyState(VK_SHIFT) & 0x8000) )
	{	
		AWorldInfo* Info = GWorld->GetWorldInfo();
		FString ClipboardText = appClipboardPaste();

		WxDlgGenericComboEntry* dlg = new WxDlgGenericComboEntry( FALSE, TRUE );

		// Gather up the existing clip pad entry names

		TArray<FString> ClipPadNames;

		for( int x = 0 ; x < Info->ClipPadEntries.Num() ; ++x )
		{
			ClipPadNames.AddItem( Info->ClipPadEntries(x)->Title );
		}

		// Ask where the user which clip pad entry they want to paste from

		if( dlg->ShowModal( TEXT("ClipPadSelect"), TEXT("PasteFrom:"), ClipPadNames, 0, TRUE ) == wxID_OK )
		{
			FString ClipPadName = dlg->GetComboBoxString();

			// Find the requested entry and copy it to the clipboard

			for( int x = 0 ; x < Info->ClipPadEntries.Num() ; ++x )
			{
				if( Info->ClipPadEntries(x)->Title == ClipPadName )
				{
					appClipboardCopy( *(Info->ClipPadEntries(x)->Text) );
				}
			}
		}
		else
		{
			return;
		}
	}

	const FScopedBusyCursor BusyCursor;

	// Create a location offset.
	const FVector LocationOffset = CreateLocationOffset( bDuplicate, bOffsetLocations );

	// Save off visible groups.
	TArray<FString> VisibleGroupArray;
	GWorld->GetWorldInfo()->VisibleGroups.ParseIntoArray( &VisibleGroupArray, TEXT(","), 0 );

	// Transact the current selection set.
	USelection* SelectedActors = GetSelectedActors();
	SelectedActors->Modify();

	// Get pasted text.
	const FString PasteString = appClipboardPaste();
	const TCHAR* Paste = *PasteString;

	// Import the actors.
	ULevelFactory* Factory = new ULevelFactory;
	Factory->FactoryCreateText( ULevel::StaticClass(), GWorld->CurrentLevel, GWorld->CurrentLevel->GetFName(), RF_Transactional, NULL, TEXT("paste"), Paste, Paste+appStrlen(Paste), GWarn );

	// Fire CALLBACK_LevelDirtied and CALLBACK_RefreshEditor_LevelBrowser when falling out of scope.
	FScopedLevelDirtied					LevelDirtyCallback;
	FScopedRefreshEditor_LevelBrowser	LevelBrowserRefresh;

	// Update the actors' locations and update the global list of visible groups.
	for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		// Offset the actor's location.
		Actor->Location += LocationOffset;

		// Add any groups this actor belongs to into the visible array.
		TArray<FString> ActorGroups;
		Actor->Group.ToString().ParseIntoArray( &ActorGroups, TEXT(","), 0 );

		for( INT GroupIndex = 0 ; GroupIndex < ActorGroups.Num() ; ++GroupIndex )
		{
			const FString& GroupName = ActorGroups(GroupIndex);
			VisibleGroupArray.AddUniqueItem( GroupName );
		}

		// Call PostEditMove to update components, notify decals, etc.
		Actor->PostEditMove( TRUE );

		// Request saves/refreshes.
		Actor->MarkPackageDirty();
		LevelDirtyCallback.Request();
		LevelBrowserRefresh.Request();
	}

	// Create the new list of visible groups and set it
	FString NewVisibleGroups;
	for( INT GroupIndex = 0 ; GroupIndex < VisibleGroupArray.Num() ; ++GroupIndex )
	{
		if( NewVisibleGroups.Len() )
		{
			NewVisibleGroups += TEXT(",");
		}
		NewVisibleGroups += VisibleGroupArray(GroupIndex);
	}
	GWorld->GetWorldInfo()->VisibleGroups = NewVisibleGroups;
	GCallbackEvent->Send( CALLBACK_RefreshEditor_GroupBrowser );

	// Note the selection change.  This will also redraw level viewports and update the pivot.
	NoteSelectionChange();
}

namespace DuplicateSelectedActors {
/**
 * A collection of actors to duplicate and prefabs to instance that all belong to the same level.
 */
class FDuplicateJob
{
public:
	/** A list of actors to duplicate. */
	TArray<AActor*>	Actors;

	/** A list of prefabs to instance. */
	TArray<APrefabInstance*> PrefabInstances;

	/** The source level that all actors in the Actors array come from. */
	ULevel*			SrcLevel;

	/**
	 * Duplicate the job's actors to the specified destination level.  The new actors
	 * are appended to the specified output lists of actors.
	 *
	 * @param	OutNewActors			[out] Newly created actors are appended to this list.
	 * @param	OutNewPrefabInstances	[out] Newly created prefab instances are appended to this list.
	 * @param	DestLevel				The level to duplicate the actors in this job to.
	 * @param	bOffsetLocations		Passed to edactPasteSelected; TRUE if new actor locations should be offset.
	 */
	void DuplicateActorsToLevel(TArray<AActor*>& OutNewActors, TArray<APrefabInstance*>& OutNewPrefabInstances, ULevel* DestLevel, UBOOL bOffsetLocations)
	{
		// Set the selection set to be precisely the actors belonging to this job.
		GWorld->CurrentLevel = SrcLevel;
		GEditor->SelectNone( FALSE, TRUE );
		for ( INT ActorIndex = 0 ; ActorIndex < Actors.Num() ; ++ActorIndex )
		{
			AActor* Actor = Actors( ActorIndex );
			GEditor->SelectActor( Actor, TRUE, NULL, FALSE );
		}

		// Copy actors from src level.
		GEditor->edactCopySelected( TRUE, FALSE );

		// Paste to the dest level.
		GWorld->CurrentLevel = DestLevel;
		GEditor->edactPasteSelected( TRUE, bOffsetLocations, FALSE );

		// The selection set will be the newly created actors; copy them over to the output array.
		for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
		{
			AActor* Actor = static_cast<AActor*>( *It );
			checkSlow( Actor->IsA(AActor::StaticClass()) );
			OutNewActors.AddItem( Actor );
		}

		// Create new prefabs in the destination level.
		const FVector LocationOffset = CreateLocationOffset( TRUE, bOffsetLocations );
		for ( INT PrefabIndex = 0 ; PrefabIndex < PrefabInstances.Num() ; ++PrefabIndex )
		{
			APrefabInstance* SrcPrefabInstance = PrefabInstances(PrefabIndex);
			UPrefab* TemplatePrefab = SrcPrefabInstance->TemplatePrefab;
			const FVector NewLocation( SrcPrefabInstance->Location + LocationOffset );

			APrefabInstance* NewPrefabInstance = TemplatePrefab
				? GEditor->Prefab_InstancePrefab( TemplatePrefab, NewLocation, FRotator(0,0,0) )
				: NULL;

			if ( NewPrefabInstance )
			{
				OutNewPrefabInstances.AddItem( NewPrefabInstance );
			}
			else
			{
				debugf( TEXT("Failed to instance prefab %s into level %s"), *SrcPrefabInstance->GetPathName(), *GWorld->CurrentLevel->GetName() );
			}
		}
	}
};
}

/** 
 * Duplicates selected actors.  Handles the case where you are trying to duplicate PrefabInstance actors.
 *
 * @param	bOffsetLocations		Should the actor locations be offset after they are created?
 */
void UUnrealEdEngine::edactDuplicateSelected( UBOOL bOffsetLocations )
{
	using namespace DuplicateSelectedActors;

	const FScopedBusyCursor BusyCursor;
	GetSelectedActors()->Modify();

	// Get the list of selected prefabs.
	TArray<APrefabInstance*> SelectedPrefabInstances;
	DeselectActorsBelongingToPrefabs( SelectedPrefabInstances, FALSE );

	// Create per-level job lists.
	typedef TMap<ULevel*, FDuplicateJob*>	DuplicateJobMap;
	DuplicateJobMap							DuplicateJobs;

	// Add selected actors to the per-level job lists.
	for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		ULevel* OldLevel = Actor->GetLevel();
		FDuplicateJob** Job = DuplicateJobs.Find( OldLevel );
		if ( Job )
		{
			(*Job)->Actors.AddItem( Actor );
		}
		else
		{
			// Allocate a new job for the level.
			FDuplicateJob* NewJob = new FDuplicateJob;
			NewJob->SrcLevel = OldLevel;
			NewJob->Actors.AddItem( Actor );
			DuplicateJobs.Set( OldLevel, NewJob );
		}
	}

	// Add prefab instances to the per-level job lists.
	for ( INT Index = 0 ; Index < SelectedPrefabInstances.Num() ; ++Index )
	{
		APrefabInstance* PrefabInstance = SelectedPrefabInstances(Index);
		ULevel* PrefabLevel = PrefabInstance->GetLevel();
		FDuplicateJob** Job = DuplicateJobs.Find( PrefabLevel );
		if ( Job )
		{
			(*Job)->PrefabInstances.AddItem( PrefabInstance );
		}
		else
		{
			// Allocate a new job for the level.
			FDuplicateJob* NewJob = new FDuplicateJob;
			NewJob->SrcLevel = PrefabLevel;
			NewJob->PrefabInstances.AddItem( PrefabInstance );
			DuplicateJobs.Set( PrefabLevel, NewJob );
		}
	}

	// Copy off the destination level.
	ULevel* DestLevel = GWorld->CurrentLevel;

	// For each level, select the actors in that level and copy-paste into the destination level.
	TArray<AActor*>	NewActors;
	TArray<APrefabInstance*> NewPrefabInstances;
	for ( DuplicateJobMap::TIterator It( DuplicateJobs ) ; It ; ++It )
	{
		FDuplicateJob* Job = It.Value();
		check( Job );
		Job->DuplicateActorsToLevel( NewActors, NewPrefabInstances, DestLevel, bOffsetLocations );
	}

	// Return the current world to the destination package.
	GWorld->CurrentLevel = DestLevel;

	// Select any newly created actors and prefabs.
	SelectNone( FALSE, TRUE );
	for ( INT ActorIndex = 0 ; ActorIndex < NewActors.Num() ; ++ActorIndex )
	{
		AActor* Actor = NewActors( ActorIndex );
		SelectActor( Actor, TRUE, NULL, FALSE );
	}
	for ( INT PrefabIndex = 0 ; PrefabIndex < NewPrefabInstances.Num() ; ++PrefabIndex )
	{
		APrefabInstance* PrefabInstance = NewPrefabInstances( PrefabIndex );
		SelectActor( PrefabInstance, TRUE, NULL, FALSE );
	}
	NoteSelectionChange();

	// Finally, cleanup.
	for ( DuplicateJobMap::TIterator It( DuplicateJobs ) ; It ; ++It )
	{
		FDuplicateJob* Job = It.Value();
		delete Job;
	}
}


/**
 * Deletes all selected actors.  bIgnoreKismetReferenced is ignored when bVerifyDeletionCanHappen is TRUE.
 *
 * @param		bVerifyDeletionCanHappen	[opt] If TRUE (default), verify that deletion can be performed.
 * @param		bIgnoreKismetReferenced		[opt] If TRUE, don't delete actors referenced by Kismet.
 * @return									TRUE unless the delete operation was aborted.
 */
UBOOL UUnrealEdEngine::edactDeleteSelected(UBOOL bVerifyDeletionCanHappen, UBOOL bIgnoreKismetReferenced)
{
	if ( bVerifyDeletionCanHappen )
	{
		// Provide the option to abort the delete if e.g. Kismet-referenced actors are selected.
		bIgnoreKismetReferenced = FALSE;
		if ( ShouldAbortActorDeletion(bIgnoreKismetReferenced) )
		{
			return FALSE;
		}
	}

	const DOUBLE StartSeconds = appSeconds();

	GetSelectedActors()->Modify();

	// Fire CALLBACK_LevelDirtied and CALLBACK_RefreshEditor_LevelBrowser when falling out of scope.
	FScopedLevelDirtied					LevelDirtyCallback;
	FScopedRefreshEditor_LevelBrowser	LevelBrowserRefresh;

	// Iterate over all levels and create a list of world infos.
	TArray<AWorldInfo*> WorldInfos;
	for ( INT LevelIndex = 0 ; LevelIndex < GWorld->Levels.Num() ; ++LevelIndex )
	{
		ULevel* Level = GWorld->Levels( LevelIndex );
		WorldInfos.AddItem( Level->GetWorldInfo() );
	}

	// Iterate over selected actors and assemble a list of actors to delete.
	TArray<AActor*> ActorsToDelete;
	for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor		= static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		// Only delete transactional actors that aren't a level's builder brush or worldinfo.
		UBOOL bDeletable	= FALSE;
		if ( Actor->HasAllFlags( RF_Transactional ) )
		{
			const UBOOL bRejectBecauseOfKismetReference = bIgnoreKismetReferenced && Actor->IsReferencedByKismet();
			if ( !bRejectBecauseOfKismetReference )
			{
				const UBOOL bIsDefaultBrush = Actor->IsBrush() && Actor->IsABuilderBrush();
				if ( !bIsDefaultBrush )
				{
					const UBOOL bIsWorldInfo =
						Actor->IsA( AWorldInfo::StaticClass() ) && WorldInfos.ContainsItem( static_cast<AWorldInfo*>(Actor) );
					if ( !bIsWorldInfo )
					{
						bDeletable = TRUE;
					}
				}
			}
		}

		if ( bDeletable )
		{
			ActorsToDelete.AddItem( Actor );
		}
		else
		{
			debugf( LocalizeSecure(LocalizeUnrealEd("CannotDeleteSpecialActor"), *Actor->GetFullName()) );
		}
	}

	// Maintain a list of levels that have already been Modify()'d so that each level
	// is modify'd only once.
	TArray<ULevel*> LevelsAlreadyModified;

	UBOOL	bTerrainWasDeleted = FALSE;
	INT		DeleteCount = 0;

	USelection* SelectedActors = GetSelectedActors();

	for ( INT ActorIndex = 0 ; ActorIndex < ActorsToDelete.Num() ; ++ActorIndex )
	{
		AActor* Actor = ActorsToDelete( ActorIndex );

		// Track whether or not a terrain actor was deleted.
		// Avoid the IsA call if we already know a terrain was deleted.
		if ( !bTerrainWasDeleted && Actor->IsA(ATerrain::StaticClass()) )
		{
			bTerrainWasDeleted = TRUE;
		}

		// If the actor about to be deleted is a PrefabInstance, need to do a couple of additional steps.
		// check to see whether the user would like to delete the PrefabInstance's members
		// remove the prefab's sequence from the level's sequence
		APrefabInstance* PrefabInstance = Cast<APrefabInstance>(Actor);
		if ( PrefabInstance != NULL )
		{
			TArray<AActor*> PrefabMembers;
			PrefabInstance->GetActorsInPrefabInstance(PrefabMembers);

			UBOOL bAllPrefabMembersSelected = TRUE;	// remains TRUE if all members of the prefab are selected
			for ( INT MemberIndex = 0 ; MemberIndex < PrefabMembers.Num() ; ++MemberIndex )
			{
				AActor* PrefabActor = PrefabMembers(MemberIndex);
				if ( !PrefabActor->IsSelected() )
				{
					// setting to FALSE ends the loop
					bAllPrefabMembersSelected = FALSE;
					break;
				}
			}

			// if not all of the PrefabInstance's members were selected, ask the user whether they should also be deleted
			if ( !bAllPrefabMembersSelected )
			{
				const UBOOL bShouldDeleteEntirePrefab = appMsgf(AMT_YesNo, LocalizeSecure(LocalizeUnrealEd(TEXT("Prefab_DeletePrefabMembersPrompt")), *PrefabInstance->GetPathName()));
				if ( bShouldDeleteEntirePrefab )
				{
					for ( INT MemberIndex = 0; MemberIndex < PrefabMembers.Num(); MemberIndex++ )
					{
						AActor* PrefabActor = PrefabMembers(MemberIndex);

						// add the actor to the selection set (probably not necessary, but do it just in case something assumes that all prefab actors would be selected)
						SelectedActors->Select(PrefabActor, TRUE);

						// now add this actor to the list of actors being deleted if it isn't already there
						ActorsToDelete.AddUniqueItem(PrefabActor);
					}
				}
			}

			// remove the prefab's sequence from the level's sequence
			PrefabInstance->DestroyKismetSequence();
		}

		// Let the object propagator report the deletion
		GObjectPropagator->OnActorDelete(Actor);

		// Mark the actor's level as dirty.
		Actor->MarkPackageDirty();
		LevelDirtyCallback.Request();
		LevelBrowserRefresh.Request();

		// Deselect the Actor.
		SelectedActors->Deselect(Actor);

		// Modify the level.  Each level is modified only once.
		// @todo DB: Shouldn't this be calling UWorld::ModifyLevel?
		ULevel* Level = Actor->GetLevel();
		if ( LevelsAlreadyModified.FindItemIndex( Level ) == INDEX_NONE )
		{
			LevelsAlreadyModified.AddItem( Level );
			Level->Modify();
		}

		// Destroy actor and clear references.
		GWorld->EditorDestroyActor( Actor, FALSE );

		DeleteCount++;
	}

	// Propagate RF_IsPendingKill till count doesn't change.
	INT CurrentObjectsPendingKillCount	= 0;
	INT LastObjectsPendingKillCount		= -1;
	while( CurrentObjectsPendingKillCount != LastObjectsPendingKillCount )
	{
		LastObjectsPendingKillCount		= CurrentObjectsPendingKillCount;
		CurrentObjectsPendingKillCount	= 0;

		for( TObjectIterator<UObject> It; It; ++It )
		{	
			UObject* Object = *It;
			if( Object->HasAnyFlags( RF_PendingKill ) )
			{
				// Object has RF_PendingKill set.
				CurrentObjectsPendingKillCount++;
			}
			else if( Object->IsPendingKill() )
			{
				// Make sure that setting pending kill is undoable.
				if( !Object->HasAnyFlags( RF_PendingKill ) )
				{
					Object->Modify();

					// Object didn't have RF_PendingKill set but IsPendingKill returned TRUE so we manually set the object flag.
					Object->MarkPendingKill();
				}
				CurrentObjectsPendingKillCount++;
			}
		}
	}

	// Remove all references to destroyed actors once at the end, instead of once for each Actor destroyed..
	UObject::CollectGarbage( GARBAGE_COLLECTION_KEEPFLAGS );

	NoteSelectionChange();

	if (bTerrainWasDeleted)
	{
		GCallbackEvent->Send(CALLBACK_RefreshEditor_TerrainBrowser);
	}

	debugf( TEXT("Deleted %d Actors (%3.3f secs)"), DeleteCount, appSeconds() - StartSeconds );

	return TRUE;
}

class WxHandleKismetReferencedActors : public wxDialog
{
public:
	WxHandleKismetReferencedActors()
		:	wxDialog( GApp->EditorFrame, -1, *LocalizeUnrealEd("ActorsReferencedByKismet"), wxDefaultPosition, wxDefaultSize)//, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER )
	{
		wxBoxSizer* UberSizer = new wxBoxSizer(wxVERTICAL);
		{
			// Text string
			wxSizer *TextSizer = new wxBoxSizer(wxHORIZONTAL);
			{
				wxStaticText* QuestionText = new wxStaticText( this, -1, *LocalizeUnrealEd("PromptSomeActorsAreReferencedByKismet") );
				TextSizer->Add(QuestionText, 0, /*wxALIGN_LEFT|*/wxALL|wxADJUST_MINSIZE, 5);
			}
			UberSizer->Add(TextSizer, 0, wxALL, 5);

			// OK/Cancel Buttons
			wxSizer *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
			{
				wxButton* ButtonMoveAll = new wxButton(this, ID_Continue, *LocalizeUnrealEd("Continue"));
				ButtonMoveAll->SetDefault();
				ButtonSizer->Add(ButtonMoveAll, 0, wxEXPAND | wxALL, 5);

				wxButton* ButtonMoveNonKismet = new wxButton(this, ID_IgnoreReferenced, *LocalizeUnrealEd("IgnoreKismetReferencedActors"));
				ButtonSizer->Add(ButtonMoveNonKismet, 0, wxEXPAND | wxALL, 5);

				wxButton* ButtonCancel = new wxButton(this, wxID_CANCEL, *LocalizeUnrealEd("Cancel"));
				ButtonSizer->Add(ButtonCancel, 0, wxEXPAND | wxALL, 5);
			}
			UberSizer->Add(ButtonSizer, 0, wxALL, 5);
		}

		SetSizer( UberSizer );
		SetAutoLayout( true );
		GetSizer()->Fit( this );
	}
private:
	DECLARE_EVENT_TABLE();
	void OnContinue(wxCommandEvent& In)
	{
		EndModal( ID_Continue );
	}
	void OnIgnoreReferenced(wxCommandEvent& In)
	{
		EndModal( ID_IgnoreReferenced );
	}
};
BEGIN_EVENT_TABLE( WxHandleKismetReferencedActors, wxDialog )
	EVT_BUTTON(ID_Continue, WxHandleKismetReferencedActors::OnContinue)
	EVT_BUTTON(ID_IgnoreReferenced, WxHandleKismetReferencedActors::OnIgnoreReferenced)
END_EVENT_TABLE()

/**
 * Checks the state of the selected actors and notifies the user of any potentially unknown destructive actions which may occur as
 * the result of deleting the selected actors.  In some cases, displays a prompt to the user to allow the user to choose whether to
 * abort the deletion.
 *
 * @param	bOutIgnoreKismetReferenced		[out] Set only if it's okay to delete actors; specifies if the user wants Kismet-refernced actors not deleted.
 * @return									FALSE to allow the selected actors to be deleted, TRUE if the selected actors should not be deleted.
 */
UBOOL UUnrealEdEngine::ShouldAbortActorDeletion(UBOOL& bOutIgnoreKismetReferenced) const
{
	UBOOL bResult = FALSE;

	// Can't delete actors if Matinee is open.
	if ( !GEditorModeTools().EnsureNotInMode( EM_InterpEdit, TRUE ) )
	{
		bResult = TRUE;
	}

	if ( !bResult )
	{
		for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
		{
			AActor* Actor = static_cast<AActor*>( *It );
			checkSlow( Actor->IsA(AActor::StaticClass()) );
			if ( Actor->IsReferencedByKismet() )
			{
				WxHandleKismetReferencedActors Dlg;
				const INT DlgResult = Dlg.ShowModal();
				if ( DlgResult == wxID_CANCEL )
				{
					bResult = TRUE;
				}
				else
				{
					// Some actors are referenced by Kismet, but the user wants to proceed.
					// Record whether or not they wanted to ignore referenced actors.
					bOutIgnoreKismetReferenced = (DlgResult == ID_IgnoreReferenced);
				}
				break;
			}
		}
	}

	if( !bResult )
	{
		UBOOL bHasRouteList = FALSE;
		TArray<ARoute*> RouteList;

		for( FSelectionIterator It( GetSelectedActorIterator() ); It && !bResult; ++It )
		{
			ANavigationPoint* DelNav = Cast<ANavigationPoint>(*It);
			if( DelNav )
			{
				if( !bHasRouteList )
				{
					bHasRouteList = TRUE;
					for( FActorIterator It; It; ++It )
					{
						ARoute* Route = Cast<ARoute>(*It);
						if( Route )
						{
							RouteList.AddItem( Route );
						}
					}
				}

				for( INT RouteIdx = 0; RouteIdx < RouteList.Num(); RouteIdx++ )
				{
					ARoute* Route = RouteList(RouteIdx);
					for( INT NavIdx = 0; NavIdx < Route->RouteList.Num(); NavIdx++ )
					{
						ANavigationPoint* Nav = Cast<ANavigationPoint>(~Route->RouteList(NavIdx));
						if( Nav == DelNav )
						{
							const UBOOL bDeleteNav = appMsgf( AMT_YesNo, *LocalizeUnrealEd("Prompt_30") );
							bResult = !bDeleteNav;
							break;
						}
					}
				}
			}
		}
	}

	return bResult;
}

/**
 * Replace all selected brushes with the default brush.
 */
void UUnrealEdEngine::edactReplaceSelectedBrush()
{
	// Make a list of brush actors to replace.
	ABrush* DefaultBrush = GWorld->GetBrush();

	TArray<ABrush*> BrushesToReplace;
	for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );
		if ( Actor->IsBrush() && Actor->HasAnyFlags(RF_Transactional) && Actor != DefaultBrush )
		{
			BrushesToReplace.AddItem( static_cast<ABrush*>(Actor) );
		}
	}

	// Fire CALLBACK_LevelDirtied and CALLBACK_RefreshEditor_LevelBrowser when falling out of scope.
	FScopedLevelDirtied					LevelDirtyCallback;
	FScopedRefreshEditor_LevelBrowser	LevelBrowserRefresh;

	GetSelectedActors()->Modify();

	// Replace brushes.
	for ( INT BrushIndex = 0 ; BrushIndex < BrushesToReplace.Num() ; ++BrushIndex )
	{
		ABrush* SrcBrush = BrushesToReplace(BrushIndex);
		ABrush* NewBrush = FBSPOps::csgAddOperation( DefaultBrush, SrcBrush->PolyFlags, (ECsgOper)SrcBrush->CsgOper );
		if( NewBrush )
		{
			SrcBrush->MarkPackageDirty();
			NewBrush->MarkPackageDirty();

			LevelDirtyCallback.Request();
			LevelBrowserRefresh.Request();

			NewBrush->Modify();
			NewBrush->Group = SrcBrush->Group;
			NewBrush->CopyPosRotScaleFrom( SrcBrush );
			NewBrush->PostEditMove( TRUE );
			SelectActor( SrcBrush, FALSE, NULL, FALSE );
			SelectActor( NewBrush, TRUE, NULL, FALSE );
			GWorld->EditorDestroyActor( SrcBrush, TRUE );
		}
	}

	NoteSelectionChange();
}

static void CopyActorProperties( AActor* Dest, const AActor *Src )
{
	// Events
	Dest->Tag	= Src->Tag;

	// Object
	Dest->Group	= Src->Group;
}

/**
 * Replaces the specified actor with a new actor of the specified class.  The new actor
 * will be selected if the current actor was selected.
 * 
 * @param	CurrentActor			The actor to replace.
 * @param	NewActorClass			The class for the new actor.
 * @param	Archetype				The template to use for the new actor.
 * @param	bNoteSelectionChange	If TRUE, call NoteSelectionChange if the new actor was created successfully.
 * @return							The new actor.
 */
AActor* UUnrealEdEngine::ReplaceActor( AActor* CurrentActor, UClass* NewActorClass, UObject* Archetype, UBOOL bNoteSelectionChange )
{
	AActor* NewActor = GWorld->SpawnActor
	(
		NewActorClass,
		NAME_None,
		CurrentActor->Location,
		CurrentActor->Rotation,
		Cast<AActor>(Archetype),
		1
	);

	if( NewActor )
	{
		NewActor->Modify();

		const UBOOL bCurrentActorSelected = GetSelectedActors()->IsSelected( CurrentActor );
		if ( bCurrentActorSelected )
		{
			// The source actor was selected, so deselect the old actor and select the new one.
			GetSelectedActors()->Modify();
			SelectActor( NewActor, bCurrentActorSelected, NULL, FALSE );
			SelectActor( CurrentActor, FALSE, NULL, FALSE );
		}

		CopyActorProperties( NewActor, CurrentActor );
		GWorld->EditorDestroyActor( CurrentActor, TRUE );

		// Note selection change if necessary and requested.
		if ( bCurrentActorSelected && bNoteSelectionChange )
		{
			NoteSelectionChange();
		}
	}

	return NewActor;
}

/**
 * Replace all selected non-brush actors with the specified class.
 */
void UUnrealEdEngine::edactReplaceSelectedNonBrushWithClass(UClass* Class)
{
	// Make a list of actors to replace.
	TArray<AActor*> ActorsToReplace;
	for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );
		if ( !Actor->IsBrush() && Actor->HasAnyFlags(RF_Transactional) )
		{
			ActorsToReplace.AddItem( Actor );
		}
	}

	// Fire CALLBACK_LevelDirtied and CALLBACK_RefreshEditor_LevelBrowser when falling out of scope.
	FScopedLevelDirtied					LevelDirtyCallback;
	FScopedRefreshEditor_LevelBrowser	LevelBrowserRefresh;

	// Replace actors.
	for ( INT i = 0 ; i < ActorsToReplace.Num() ; ++i )
	{
		AActor* SrcActor = ActorsToReplace(i);
		AActor* NewActor = ReplaceActor( SrcActor, Class, NULL, FALSE );
		if ( NewActor )
		{
			NewActor->MarkPackageDirty();
			LevelDirtyCallback.Request();
			LevelBrowserRefresh.Request();
		}
	}

	NoteSelectionChange();
}

/**
 * Replace all actors of the specified source class with actors of the destination class.
 *
 * @param	SrcClass	The class of actors to replace.
 * @param	DstClass	The class to replace with.
 */
void UUnrealEdEngine::edactReplaceClassWithClass(UClass* SrcClass, UClass* DstClass)
{
	// Make a list of actors to replace.
	TArray<AActor*> ActorsToReplace;
	for( FActorIterator It; It; ++It )
	{
		AActor* Actor = *It;
		if ( Actor->IsA( SrcClass ) && Actor->HasAnyFlags(RF_Transactional) )
		{
			ActorsToReplace.AddItem( Actor );
		}
	}

	// Fires CALLBACK_LevelDirtied when falling out of scope.
	FScopedLevelDirtied					LevelDirtyCallback;
	FScopedRefreshEditor_LevelBrowser	LevelBrowserRefresh;

	// Replace actors.
	for ( INT i = 0 ; i < ActorsToReplace.Num() ; ++i )
	{
		AActor* SrcActor = ActorsToReplace(i);
		AActor* NewActor = ReplaceActor( SrcActor, DstClass, NULL, FALSE );
		if ( NewActor )
		{
			NewActor->MarkPackageDirty();
			LevelDirtyCallback.Request();
			LevelBrowserRefresh.Request();
		}
	}

	NoteSelectionChange();
}

/*-----------------------------------------------------------------------------
   Actor hiding functions.
-----------------------------------------------------------------------------*/

/**
 * Hide selected actors (set their bHiddenEd=true).
 */
void UUnrealEdEngine::edactHideSelected()
{
	// Assemble a list of actors to hide.
	TArray<AActor*> ActorsToHide;
	for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );
		if ( !Actor->IsABuilderBrush() && !Actor->bHiddenEd )
		{
			ActorsToHide.AddItem( Actor );
		}
	}

	if ( ActorsToHide.Num() > 0 )
	{
		// Fires CALLBACK_LevelDirtied when falling out of scope.
		FScopedLevelDirtied LevelDirtyCallback;

		USelection* SelectedActors = GetSelectedActors();
		SelectedActors->Modify();

		for( INT ActorIndex = 0 ; ActorIndex < ActorsToHide.Num() ; ++ActorIndex )
		{
			AActor* Actor = ActorsToHide( ActorIndex );
			Actor->Modify();
			Actor->bHiddenEd = TRUE;
			Actor->ForceUpdateComponents(FALSE,FALSE);
			SelectedActors->Deselect( Actor );
			Actor->MarkPackageDirty();
			LevelDirtyCallback.Request();
		}

		NoteSelectionChange();
	}
}

/**
 * Hide unselected actors (set their bHiddenEd=true).
 */
void UUnrealEdEngine::edactHideUnselected()
{
	// Fires CALLBACK_LevelDirtied when falling out of scope.
	FScopedLevelDirtied LevelDirtyCallback;

	for( FActorIterator It; It; ++It )
	{
		AActor* Actor = *It;
		if( !Actor->IsABuilderBrush() && !Actor->IsSelected() && !Actor->bHiddenEd )
		{
			Actor->Modify();
			Actor->bHiddenEd = TRUE;
			Actor->ForceUpdateComponents(FALSE,FALSE);
			Actor->MarkPackageDirty();
			LevelDirtyCallback.Request();
		}
	}
}

/**
 * UnHide selected actors (set their bHiddenEd=true).
 */
void UUnrealEdEngine::edactUnHideAll()
{
	// Fires CALLBACK_LevelDirtied when falling out of scope.
	FScopedLevelDirtied LevelDirtyCallback;

	for( FActorIterator It; It; ++It )
	{
		AActor* Actor = *It;
		if( !Actor->IsABuilderBrush() && Actor->bHiddenEd && Actor->GetClass()->GetDefaultActor()->bHiddenEd==0 )
		{
			Actor->Modify();
			Actor->bHiddenEd = FALSE;
			Actor->ForceUpdateComponents(FALSE,FALSE);
			Actor->MarkPackageDirty();
			LevelDirtyCallback.Request();
		}
	}
	RedrawLevelEditingViewports();
}

/*-----------------------------------------------------------------------------
   Actor selection functions.
-----------------------------------------------------------------------------*/

/**
 * Select all actors except hidden actors.
 */
void UUnrealEdEngine::edactSelectAll()
{
	TArray<FName> GroupArray;

	// Add all selected actors' group name to the GroupArray.
	USelection* SelectedActors = GetSelectedActors();

	for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		if( !Actor->IsHiddenEd() && Actor->Group!=NAME_None )
		{
			GroupArray.AddUniqueItem( Actor->Group );
		}
	}

	SelectedActors->Modify();

	if( GroupArray.Num() == 0 ) 
	{
		for( FActorIterator It; It; ++It )
		{
			AActor* Actor = *It;
			if( !Actor->IsSelected() && !Actor->IsHiddenEd() )
			{
				SelectActor( Actor, 1, NULL, 0 );
			}
		}

	} 
	// otherwise, select all actors that match one of the groups,
	else 
	{
		// use appStrfind() to allow selection based on hierarchically organized group names
		for( FActorIterator It; It; ++It )
		{
			AActor* Actor = *It;
			if( !Actor->IsSelected() && !Actor->IsHiddenEd() )
			{
				for( INT j=0; j<GroupArray.Num(); j++ ) 
				{
					if( appStrfind( *Actor->Group.ToString(), *GroupArray(j).ToString() ) != NULL ) 
					{
						SelectActor( Actor, 1, NULL, 0 );
						break;
					}
				}
			}
		}
	}

	NoteSelectionChange();
}

/**
 * Invert the selection of all actors.
 */
void UUnrealEdEngine::edactSelectInvert()
{
	GetSelectedActors()->Modify();

	for( FActorIterator It; It; ++It )
	{
		AActor* Actor = *It;
		if( !Actor->IsABuilderBrush() && !Actor->IsHiddenEd() )
		{
			SelectActor( Actor, !Actor->IsSelected(), NULL, FALSE );
		}
	}
	NoteSelectionChange();
}

/**
 * Select any actors based on currently selected actors 
 */
void UUnrealEdEngine::edactSelectBased()
{
	USelection* SelectedActors = GetSelectedActors();

	if( SelectedActors != NULL )
	{
		SelectedActors->Modify();

		TArray<AActor*> BasedActors;
		for( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
		{
			AActor* Actor = static_cast<AActor*>( *It );
			checkSlow( Actor->IsA(AActor::StaticClass()) );

			for( INT AttachIdx = 0; AttachIdx < Actor->Attached.Num(); AttachIdx++ )
			{
				AActor* Child = Actor->Attached(AttachIdx);
				if( Child == NULL )
					continue;

				BasedActors.AddUniqueItem( Child );
			}
		}

		for( INT BasedIdx = 0; BasedIdx < BasedActors.Num(); BasedIdx++ )
		{
			SelectActor( BasedActors( BasedIdx ), TRUE, NULL, TRUE );
		}

		NoteSelectionChange();
	}
}

/**
 * Select all actors in a particular class.
 */
void UUnrealEdEngine::edactSelectOfClass( UClass* Class )
{
	GetSelectedActors()->Modify();

	for( FActorIterator It; It; ++It )
	{
		AActor* Actor = *It;
		if( Actor->GetClass()==Class && !Actor->IsSelected() && !Actor->IsHiddenEd() )
		{
			// Selection by class not permitted for actors belonging to prefabs.
			// Selection by class not permitted for builder brushes.
			if ( !Actor->IsInPrefabInstance() && !Actor->IsABuilderBrush() )
			{
				SelectActor( Actor, 1, NULL, 0 );
			}
		}
	}
	NoteSelectionChange();
}

/**
 * Select all actors in a particular class and its subclasses.
 */
void UUnrealEdEngine::edactSelectSubclassOf( UClass* Class )
{
	GetSelectedActors()->Modify();

	for( FActorIterator It; It; ++It )
	{
		AActor* Actor = *It;
		if( !Actor->IsSelected() && !Actor->IsHiddenEd() && Actor->GetClass()->IsChildOf(Class) )
		{
			// Selection by class not permitted for actors belonging to prefabs.
			// Selection by class not permitted for builder brushes.
			if ( !Actor->IsInPrefabInstance() && !Actor->IsABuilderBrush() )
			{
				SelectActor( Actor, 1, NULL, 0 );
			}
		}
	}
	NoteSelectionChange();
}

/**
 * Select all actors in a level that are marked for deletion.
 */
void UUnrealEdEngine::edactSelectDeleted()
{
	GetSelectedActors()->Modify();

	UBOOL bSelectionChanged = FALSE;
	for( FActorIterator It; It; ++It )
	{
		AActor* Actor = *It;
		if( !Actor->IsSelected() && !Actor->IsHiddenEd() )
		{
			if( Actor->bDeleteMe )
			{
				bSelectionChanged = TRUE;
				SelectActor( Actor, 1, NULL, 0 );
			}
		}
	}
	if ( bSelectionChanged )
	{
		NoteSelectionChange();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Select matching static meshes.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {

/**
 * Information about an actor and its static mesh.
 */
class FStaticMeshActor
{
public:
	/** Non-NULL if the actor is a static mesh. */
	AStaticMeshActor* StaticMeshActor;
	/** Non-NULL if the actor is an InterpActor actor. */
	AInterpActor* InterpActor;
	/** Non-NULL if the actor is a KActor actor. */
	AKActor* KActor;
	/** Non-NULL if the actor is a FracturedStaticMeshActor actor. */
	AFracturedStaticMeshActor* FracMeshActor;
	/** Non-NULL if the actor has a static mesh. */
	UStaticMesh* StaticMesh;

	FStaticMeshActor()
		: StaticMeshActor(NULL)
		, InterpActor(NULL)
		, KActor(NULL)
		, FracMeshActor(NULL)
		, StaticMesh(NULL)
	{}

	UBOOL IsStaticMeshActor() const
	{
		return StaticMeshActor != NULL;
	}

	UBOOL IsInterpActor() const
	{
		return InterpActor != NULL;
	}

	UBOOL IsKActor() const
	{
		return KActor != NULL;
	}

	UBOOL IsFracMeshActor() const
	{
		return FracMeshActor != NULL;
	}

	UBOOL HasStaticMesh() const
	{
		return StaticMesh != NULL;
	}

	/**
	 * Extracts the static mesh information from the specified actor.
	 */
	static UBOOL GetStaticMeshInfoFromActor(AActor* Actor, FStaticMeshActor& OutStaticMeshActor)
	{
		OutStaticMeshActor.StaticMeshActor = Cast<AStaticMeshActor>( Actor );
		OutStaticMeshActor.InterpActor = Cast<AInterpActor>( Actor );
		OutStaticMeshActor.KActor = Cast<AKActor>( Actor );
		OutStaticMeshActor.FracMeshActor = Cast<AFracturedStaticMeshActor>( Actor );

		if( OutStaticMeshActor.IsStaticMeshActor() )
		{
			if ( OutStaticMeshActor.StaticMeshActor->StaticMeshComponent )
			{
				OutStaticMeshActor.StaticMesh = OutStaticMeshActor.StaticMeshActor->StaticMeshComponent->StaticMesh;
			}
		}
		else if ( OutStaticMeshActor.IsInterpActor() )
		{
			if ( OutStaticMeshActor.InterpActor->StaticMeshComponent )
			{
				OutStaticMeshActor.StaticMesh = OutStaticMeshActor.InterpActor->StaticMeshComponent->StaticMesh;
			}
		}
		else if ( OutStaticMeshActor.IsKActor() )
		{
			if ( OutStaticMeshActor.KActor->StaticMeshComponent )
			{
				OutStaticMeshActor.StaticMesh = OutStaticMeshActor.KActor->StaticMeshComponent->StaticMesh;
			}
		}
		else if ( OutStaticMeshActor.IsFracMeshActor() )
		{
			if ( OutStaticMeshActor.FracMeshActor->FracturedStaticMeshComponent )
			{
				OutStaticMeshActor.StaticMesh = OutStaticMeshActor.FracMeshActor->FracturedStaticMeshComponent->StaticMesh;
			}
		}
		return OutStaticMeshActor.HasStaticMesh();
	}
};

} // namespace

/**
 * Select all actors that have the same static mesh assigned to them as the selected ones.
 *
 * @param bAllClasses		If TRUE, also select non-AStaticMeshActor actors whose meshes match.
 */
void UUnrealEdEngine::edactSelectMatchingStaticMesh(UBOOL bAllClasses)
{
	TArray<FStaticMeshActor> StaticMeshActors;
	TArray<FStaticMeshActor> InterpActors;
	TArray<FStaticMeshActor> KActors;
	TArray<FStaticMeshActor> FracMeshActors;

	// Make a list of selected actors with static meshes.
	for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		FStaticMeshActor ActorInfo;
		if ( FStaticMeshActor::GetStaticMeshInfoFromActor( Actor, ActorInfo ) )
		{
			if ( ActorInfo.IsStaticMeshActor() )
			{
				StaticMeshActors.AddItem( ActorInfo );
			}
			else if ( ActorInfo.IsInterpActor() )
			{
				InterpActors.AddItem( ActorInfo );
			}
			else if ( ActorInfo.IsKActor() )
			{
				KActors.AddItem( ActorInfo );
			}
			else if ( ActorInfo.IsFracMeshActor() )
			{
				FracMeshActors.AddItem( ActorInfo );
			}
		}
	}

	USelection* SelectedActors = GetSelectedActors();
	SelectedActors->Modify();

	// Loop through all non-hidden actors in visible levels, selecting those that have one of the
	// static meshes in the list.
	for( FActorIterator It; It; ++It )
	{
		AActor* Actor = *It;
		if ( !Actor->IsHiddenEd() )
		{
			FStaticMeshActor ActorInfo;
			if ( FStaticMeshActor::GetStaticMeshInfoFromActor( Actor, ActorInfo ) )
			{
				UBOOL bSelectActor = FALSE;
				if ( !bSelectActor && (bAllClasses || ActorInfo.IsStaticMeshActor()) )
				{
					for ( INT i = 0 ; i < StaticMeshActors.Num() ; ++i )
					{
						if ( StaticMeshActors(i).StaticMesh == ActorInfo.StaticMesh )
						{
							bSelectActor = TRUE;
							break;
						}
					}
				}
				if ( !bSelectActor && (bAllClasses || ActorInfo.IsInterpActor()) )
				{
					for ( INT i = 0 ; i < InterpActors.Num() ; ++i )
					{
						if ( InterpActors(i).StaticMesh == ActorInfo.StaticMesh )
						{
							bSelectActor = TRUE;
							break;
						}
					}
				}
				if ( !bSelectActor && (bAllClasses || ActorInfo.IsKActor()) )
				{
					for ( INT i = 0 ; i < KActors.Num() ; ++i )
					{
						if ( KActors(i).StaticMesh == ActorInfo.StaticMesh )
						{
							bSelectActor = TRUE;
							break;
						}
					}
				}
				if ( !bSelectActor && (bAllClasses || ActorInfo.IsFracMeshActor()) )
				{
					for ( INT i = 0 ; i < FracMeshActors.Num() ; ++i )
					{
						if ( FracMeshActors(i).StaticMesh == ActorInfo.StaticMesh )
						{
							bSelectActor = TRUE;
							break;
						}
					}
				}

				if ( bSelectActor )
				{
					SelectActor( Actor, TRUE, NULL, FALSE );
				}
			}
		}
	}

	NoteSelectionChange();
}

/**
* Select all actors that have the same skeletal mesh assigned to them as the selected ones.
*
* @param bAllClasses		If TRUE, also select all actors whose meshes match.
*/
void UUnrealEdEngine::edactSelectMatchingSkeletalMesh(UBOOL bAllClasses)
{
	TArray<USkeletalMesh*> SelectedMeshes;
	UBOOL bSelectSkelMeshActors = FALSE;
	UBOOL bSelectKAssets = FALSE;
	UBOOL bSelectPawns = FALSE;

	// Make a list of skeletal meshes of selected actors, and note what classes we have selected.
	for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		// Look for SkelMeshActor
		ASkeletalMeshActor* SkelMeshActor = Cast<ASkeletalMeshActor>(Actor);
		if(SkelMeshActor && SkelMeshActor->SkeletalMeshComponent)
		{
			bSelectSkelMeshActors = TRUE;
			SelectedMeshes.AddUniqueItem(SkelMeshActor->SkeletalMeshComponent->SkeletalMesh);
		}

		// Look for Kasset
		AKAsset* KAsset = Cast<AKAsset>(Actor);
		if(KAsset && KAsset->SkeletalMeshComponent)
		{
			bSelectKAssets = TRUE;
			SelectedMeshes.AddUniqueItem(KAsset->SkeletalMeshComponent->SkeletalMesh);
		}

		// Look for Pawn
		APawn* Pawn = Cast<APawn>(Actor);
		if(Pawn && Pawn->Mesh)
		{
			bSelectPawns = TRUE;
			SelectedMeshes.AddUniqueItem(Pawn->Mesh->SkeletalMesh);
		}
	}

	// If desired, select all class types
	if(bAllClasses)
	{
		bSelectSkelMeshActors = TRUE;
		bSelectKAssets = TRUE;
		bSelectPawns = TRUE;
	}

	USelection* SelectedActors = GetSelectedActors();
	SelectedActors->Modify();

	// Loop through all non-hidden actors in visible levels, selecting those that have one of the skeletal meshes in the list.
	for( FActorIterator It; It; ++It )
	{
		AActor* Actor = *It;
		if ( !Actor->IsHiddenEd() )
		{
			UBOOL bSelectActor = FALSE;

			if(bSelectSkelMeshActors)
			{
				ASkeletalMeshActor* SkelMeshActor = Cast<ASkeletalMeshActor>(Actor);
				if( SkelMeshActor && 
					SkelMeshActor->SkeletalMeshComponent && 
					SelectedMeshes.ContainsItem(SkelMeshActor->SkeletalMeshComponent->SkeletalMesh) )
				{
					bSelectActor = TRUE;
				}
			}

			if(bSelectKAssets)
			{
				AKAsset* KAsset = Cast<AKAsset>(Actor);
				if( KAsset && 
					KAsset->SkeletalMeshComponent &&
					SelectedMeshes.ContainsItem(KAsset->SkeletalMeshComponent->SkeletalMesh) )
				{
					bSelectActor = TRUE;
				}
			}

			if(bSelectPawns)
			{
				APawn* Pawn = Cast<APawn>(Actor);
				if( Pawn && 
					Pawn->Mesh &&
					SelectedMeshes.ContainsItem(Pawn->Mesh->SkeletalMesh) )
				{
					bSelectActor = TRUE;
				}
			}

			if ( bSelectActor )
			{
				SelectActor( Actor, TRUE, NULL, FALSE );
			}
		}
	}

	NoteSelectionChange();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Selects actors in the current level based on whether or not they're referenced by Kismet.
 *
 * @param	bReferenced		If TRUE, select actors referenced by Kismet; if FALSE, select unreferenced actors.
 */
void UUnrealEdEngine::edactSelectKismetReferencedActors(UBOOL bReferenced)
{
	USelection* SelectedActors = GetSelectedActors();
	SelectedActors->Modify();

	SelectNone( FALSE, TRUE );
	for( FActorIterator It; It; ++It )
	{
		AActor* Actor = *It;
		if ( Actor->GetLevel() == GWorld->CurrentLevel )
		{
			const UBOOL bReferencedByKismet = Actor->IsReferencedByKismet();
			if ( bReferencedByKismet == bReferenced )
			{
				SelectActor( Actor, TRUE, NULL, FALSE );
			}
		}
	}

	NoteSelectionChange();
}

void UUnrealEdEngine::SelectMatchingSpeedTrees()
{
	TArray<USpeedTree*> SelectedSpeedTrees;

	// Make a list of selected SpeedTrees.
	for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
	{
		ASpeedTreeActor* SpeedTreeActor = Cast<ASpeedTreeActor>(*It);
		if(SpeedTreeActor)
		{
			if(SpeedTreeActor->SpeedTreeComponent->SpeedTree)
			{
				SelectedSpeedTrees.AddUniqueItem(SpeedTreeActor->SpeedTreeComponent->SpeedTree);
			}
		}
	}

	USelection* SelectedActors = GetSelectedActors();
	SelectedActors->Modify();

	// Loop through all non-hidden actors in visible levels, selecting those that have one of the
	// SpeedTrees in the list.
	for( FActorIterator It; It; ++It )
	{
		ASpeedTreeActor* SpeedTreeActor = Cast<ASpeedTreeActor>(*It);
		if(SpeedTreeActor && !SpeedTreeActor->IsHiddenEd())
		{
			if(SelectedSpeedTrees.ContainsItem(SpeedTreeActor->SpeedTreeComponent->SpeedTree))
			{
				SelectActor(SpeedTreeActor,TRUE,NULL,FALSE);
			}
		}
	}

	NoteSelectionChange();
}

/**
 * Align all vertices with the current grid.
 */
void UUnrealEdEngine::edactAlignVertices()
{
	// Fires CALLBACK_LevelDirtied when falling out of scope.
	FScopedLevelDirtied LevelDirtyCallback;

	// Apply transformations to all selected brushes.
	for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		if ( Actor->IsABrush() )
		{
			ABrush* Brush = Cast<ABrush>( Actor );
			LevelDirtyCallback.Request();

			Brush->PreEditChange(NULL);
			Brush->Modify();

			// Snap each vertex in the brush to an integer grid.
			UPolys* Polys = Brush->Brush->Polys;
			for( INT j=0; j<Polys->Element.Num(); j++ )
			{
				FPoly* Poly = &Polys->Element(j);
				for( INT k=0; k<Poly->Vertices.Num(); k++ )
				{
					// Snap each vertex to the nearest grid.
					Poly->Vertices(k).X = appRound( ( Poly->Vertices(k).X + Brush->Location.X )  / Constraints.GetGridSize() ) * Constraints.GetGridSize() - Brush->Location.X;
					Poly->Vertices(k).Y = appRound( ( Poly->Vertices(k).Y + Brush->Location.Y )  / Constraints.GetGridSize() ) * Constraints.GetGridSize() - Brush->Location.Y;
					Poly->Vertices(k).Z = appRound( ( Poly->Vertices(k).Z + Brush->Location.Z )  / Constraints.GetGridSize() ) * Constraints.GetGridSize() - Brush->Location.Z;
				}

				// If the snapping resulted in an off plane polygon, triangulate it to compensate.
				if( !Poly->IsCoplanar() )
				{
					Poly->Triangulate( Brush );
				}

				if( RecomputePoly( Brush, &Polys->Element(j) ) == -2 )
				{
					j = -1;
				}
			}

			Brush->Brush->BuildBound();

			Brush->PostEditChange(NULL);
		}
	}
}
