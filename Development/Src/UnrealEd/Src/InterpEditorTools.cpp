/*=============================================================================
	InterpEditorTools.cpp: Interpolation editing support tools
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "EngineSequenceClasses.h"
#include "EngineInterpolationClasses.h"
#include "EngineAnimClasses.h"
#include "UnLinkedObjDrawUtils.h"
#include "InterpEditor.h"
#include "Properties.h"
#include "DlgGenericComboEntry.h"

static const FColor ActiveCamColor(255, 255, 0);
static const FColor SelectedCurveColor(255, 255, 0);
static const INT	DuplicateKeyOffset(10);
static const INT	KeySnapPixels(5);

///// UTILS

void WxInterpEd::TickInterp(FLOAT DeltaTime)
{
	// Don't tick if a windows close request was issued.
	if( !bClosed && Interp->bIsPlaying )
	{
		// When in 'fixed time step' playback, we may need to constrain the frame rate (by sleeping!)
		ConstrainFixedTimeStepFrameRate();

		// Make sure particle replay tracks have up-to-date editor-only transient state
		UpdateParticleReplayTracks();

		// Modify playback rate by desired speed.
		FLOAT TimeDilation = GWorld->GetWorldInfo()->TimeDilation;
		Interp->StepInterp(DeltaTime * PlaybackSpeed * TimeDilation, true);
		
		// If we are looping the selected section, when we pass the end, place it back to the beginning 
		if(bLoopingSection)
		{
			if(Interp->Position >= IData->EdSectionEnd)
			{
				Interp->UpdateInterp(IData->EdSectionStart, TRUE, TRUE);
				Interp->Play();
			}
		}

		UpdateCameraToGroup();
		UpdateCamColours();
		CurveEd->SetPositionMarker(true, Interp->Position, PosMarkerColor );
	}

	// Make sure fixed time step mode is set correctly based on whether we're currently 'playing' or not
	// We need to do this here because interp sequences can stop without us ever telling them to (and
	// we won't find out about it!)
	UpdateFixedTimeStepPlayback();
}



/** Constrains the maximum frame rate to the fixed time step rate when playing back in that mode */
void WxInterpEd::ConstrainFixedTimeStepFrameRate()
{
	// Don't allow the fixed time step playback to run faster than real-time
	if( bSnapToFrames && bFixedTimeStepPlayback )
	{
		// NOTE: Its important that PlaybackStartRealTime and NumContinuousFixedTimeStepFrames are reset
		//    when anything timing-related changes, like GFixedDeltaTime or playback direction.

		DOUBLE CurRealTime = appSeconds();

		// Minor hack to handle changes to world TimeDilation.  We reset our frame rate gate state
		// when we detect a change to time dilation.
		static FLOAT s_LastTimeDilation = GWorld->GetWorldInfo()->TimeDilation;
		if( s_LastTimeDilation != GWorld->GetWorldInfo()->TimeDilation )
		{
			// Looks like time dilation has changed!
			NumContinuousFixedTimeStepFrames = 0;
			PlaybackStartRealTime = CurRealTime;

			s_LastTimeDilation = GWorld->GetWorldInfo()->TimeDilation;
		}

		// How long should have it taken to get to the current frame?
		const DOUBLE ExpectedPlaybackTime =
			NumContinuousFixedTimeStepFrames * GFixedDeltaTime * PlaybackSpeed;

		// How long has it been (in real-time) since we started playback?
		DOUBLE RealTimeSincePlaybackStarted = CurRealTime - PlaybackStartRealTime;

		// If we're way ahead of schedule (more than 5 ms), then we'll perform a long sleep
		FLOAT WaitTime = ExpectedPlaybackTime - RealTimeSincePlaybackStarted;
		if( WaitTime > 5 / 1000.0f )
		{
			appSleep( WaitTime - 3 / 1000.0f );

			// Update timing info after our little snooze
			CurRealTime = appSeconds();
			RealTimeSincePlaybackStarted = CurRealTime - PlaybackStartRealTime;
			WaitTime = ExpectedPlaybackTime - RealTimeSincePlaybackStarted;
		}

		while( RealTimeSincePlaybackStarted < ExpectedPlaybackTime )
		{
			// OK, we're running ahead of schedule so we need to wait a bit before the next frame
			appSleep( 0.0f );

			// Check the time again
			CurRealTime = appSeconds();
			RealTimeSincePlaybackStarted = CurRealTime - PlaybackStartRealTime;
			WaitTime = ExpectedPlaybackTime - RealTimeSincePlaybackStarted;
		}

		// Increment number of continuous fixed time step frames
		++NumContinuousFixedTimeStepFrames;
	}
}



static UBOOL bIgnoreActorSelection = false;

void WxInterpEd::SetSelectedFilter(class UInterpFilter* InFilter)
{
	if ( IData->SelectedFilter != InFilter )
	{
		IData->SelectedFilter = InFilter;

		if(InFilter != NULL)
		{
			// Start by hiding all groups and tracks
			for(INT GroupIdx=0; GroupIdx<IData->InterpGroups.Num(); GroupIdx++)
			{
				UInterpGroup* CurGroup = IData->InterpGroups( GroupIdx );
				CurGroup->bVisible = FALSE;

				for( INT CurTrackIndex = 0; CurTrackIndex < CurGroup->InterpTracks.Num(); ++CurTrackIndex )
				{
					UInterpTrack* CurTrack = CurGroup->InterpTracks( CurTrackIndex );
					CurTrack->bVisible = FALSE;
				}
			}


			// Apply the filter.  This will mark certain groups and tracks as visible.
			InFilter->FilterData( Interp );


			// Make sure folders that are parents to visible groups are ALSO visible!
			for(INT GroupIdx=0; GroupIdx<IData->InterpGroups.Num(); GroupIdx++)
			{
				UInterpGroup* CurGroup = IData->InterpGroups( GroupIdx );
				if( CurGroup->bVisible )
				{
					// Make sure my parent folder group is also visible!
					if( CurGroup->bIsParented )
					{
						UInterpGroup* ParentFolderGroup = FindParentGroupFolder( CurGroup );
						if( ParentFolderGroup != NULL )
						{
							ParentFolderGroup->bVisible = TRUE;
						}
					}
				}
			}
		}
		else
		{
			// No filter, so show all groups and tracks
			for(INT GroupIdx=0; GroupIdx<IData->InterpGroups.Num(); GroupIdx++)
			{
				UInterpGroup* CurGroup = IData->InterpGroups( GroupIdx );
				CurGroup->bVisible = TRUE;

				// Hide tracks
				for( INT CurTrackIndex = 0; CurTrackIndex < CurGroup->InterpTracks.Num(); ++CurTrackIndex )
				{
					UInterpTrack* CurTrack = CurGroup->InterpTracks( CurTrackIndex );
					CurTrack->bVisible = TRUE;
				}

			}
		}
		

		// The selected group filter may have changed which directly affects the vertical size of the content
		// in the track window, so we'll need to update our scroll bars.
		UpdateTrackWindowScrollBars();

		// Update scroll position
		ScrollToGroup(ActiveGroup);
	}
}

void WxInterpEd::SetActiveTrack(UInterpGroup* InGroup, INT InTrackIndex)
{
	// Do nothing if already active.
	if( InGroup == ActiveGroup && InTrackIndex == ActiveTrackIndex)
		return;

	// Un-highlight curve in curve editor
	if(ActiveGroup && ActiveTrackIndex != INDEX_NONE && ActiveGroup->InterpTracks.IsValidIndex(ActiveTrackIndex))
	{
		UInterpTrack* Track = ActiveGroup->InterpTracks(ActiveTrackIndex);
		IData->CurveEdSetup->ChangeCurveColor(Track, ActiveGroup->GroupColor);
		CurveEd->UpdateDisplay();
	}

	// If nothing selected.
	if( !InGroup )
	{
		check( InTrackIndex == INDEX_NONE );

		ActiveGroup = NULL;
		ActiveTrackIndex = INDEX_NONE;

		PropertyWindow->SetObject(NULL,0,0,0);
		GUnrealEd->SelectNone( TRUE, TRUE );

		ClearKeySelection();

		return;
	}

	// We have selected a group

	ActiveGroup = InGroup;

	// If this track has an Actor, select it (if not already).
	UInterpGroupInst* GrInst = Interp->FindFirstGroupInst(ActiveGroup);
	if( GrInst != NULL )
	{
		check( GrInst->TrackInst.Num() == ActiveGroup->InterpTracks.Num() );

		AActor* Actor = GrInst->GroupActor;
		if( Actor && !Actor->IsSelected() )
		{
			bIgnoreActorSelection = true;
			GUnrealEd->SelectNone( TRUE, TRUE );
			GUnrealEd->SelectActor( Actor, TRUE, NULL, TRUE );
			bIgnoreActorSelection = false;
		}
	}

	// If a track is selected as well, put its properties in the Property Window
	if(InTrackIndex != INDEX_NONE)
	{
		check( InTrackIndex >= 0 && InTrackIndex < ActiveGroup->InterpTracks.Num() );
		ActiveTrackIndex = InTrackIndex;

		UInterpTrack* Track = ActiveGroup->InterpTracks(ActiveTrackIndex);

		// Set the property window to the selected track.
		PropertyWindow->SetObject( Track, 1,1,0 );

		// Highlight the selected curve.
		IData->CurveEdSetup->ChangeCurveColor(Track, SelectedCurveColor);
		CurveEd->UpdateDisplay();
	}
	else
	{
		ActiveTrackIndex = INDEX_NONE;

		// If just selecting the group - show its properties
		PropertyWindow->SetObject( ActiveGroup, 1,1,0 );
	}
}

void WxInterpEd::ClearKeySelection()
{
	Opt->SelectedKeys.Empty();
	bAdjustingKeyframe = false;

	// Dirty the track window viewports
	InvalidateTrackWindowViewports();
}

void WxInterpEd::AddKeyToSelection(UInterpGroup* InGroup, INT InTrackIndex, INT InKeyIndex, UBOOL bAutoWind)
{
	check(InGroup);

	UInterpTrack* Track = InGroup->InterpTracks(InTrackIndex);
	check(Track);

	check( InKeyIndex >= 0 && InKeyIndex < Track->GetNumKeyframes() );

	// If the sequence is currently playing, stop it before selecting the key.
	// This check is necessary because calling StopPlaying if playback is stopped will zero
	// the playback position, which we don't want to do.
	if ( Interp->bIsPlaying )
	{
		StopPlaying();
	}

	// If key is not already selected, add to selection set.
	if( !KeyIsInSelection(InGroup, InTrackIndex, InKeyIndex) )
	{
		// Add to array of selected keys.
		Opt->SelectedKeys.AddItem( FInterpEdSelKey(InGroup, InTrackIndex, InKeyIndex) );
	}

	// If this is the first and only keyframe selected, make track active and wind to it.
	if(Opt->SelectedKeys.Num() == 1 && bAutoWind)
	{
		SetActiveTrack( InGroup, InTrackIndex );

		FLOAT KeyTime = Track->GetKeyframeTime(InKeyIndex);
		SetInterpPosition(KeyTime);

		// When jumping to keyframe, update the pivot so the widget is in the right place.
		UInterpGroupInst* GrInst = Interp->FindFirstGroupInst(InGroup);
		if(GrInst && GrInst->GroupActor)
		{
			GEditor->SetPivot( GrInst->GroupActor->Location, FALSE, FALSE, TRUE );
		}

		bAdjustingKeyframe = true;
	}

	if(Opt->SelectedKeys.Num() != 1)
	{
		bAdjustingKeyframe = false;
	}

	// Dirty the track window viewports
	InvalidateTrackWindowViewports();
}

void WxInterpEd::RemoveKeyFromSelection(UInterpGroup* InGroup, INT InTrackIndex, INT InKeyIndex)
{
	for(INT i=0; i<Opt->SelectedKeys.Num(); i++)
	{
		if( Opt->SelectedKeys(i).Group == InGroup && 
			Opt->SelectedKeys(i).TrackIndex == InTrackIndex && 
			Opt->SelectedKeys(i).KeyIndex == InKeyIndex )
		{
			Opt->SelectedKeys.Remove(i);

			bAdjustingKeyframe = false;

			// Dirty the track window viewports
			InvalidateTrackWindowViewports();

			return;
		}
	}
}

UBOOL WxInterpEd::KeyIsInSelection(UInterpGroup* InGroup, INT InTrackIndex, INT InKeyIndex)
{
	for(INT i=0; i<Opt->SelectedKeys.Num(); i++)
	{
		if( Opt->SelectedKeys(i).Group == InGroup && 
			Opt->SelectedKeys(i).TrackIndex == InTrackIndex && 
			Opt->SelectedKeys(i).KeyIndex == InKeyIndex )
			return true;
	}

	return false;
}

/** Clear selection and then select all keys within the gree loop-section. */
void WxInterpEd::SelectKeysInLoopSection()
{
	ClearKeySelection();

	// Add keys that are within current section to selection
	for(INT i=0; i<IData->InterpGroups.Num(); i++)
	{
		UInterpGroup* Group = IData->InterpGroups(i);
		for(INT j=0; j<Group->InterpTracks.Num(); j++)
		{
			UInterpTrack* Track = Group->InterpTracks(j);
			Track->Modify();

			for(INT k=0; k<Track->GetNumKeyframes(); k++)
			{
				// Add keys in section to selection for deletion.
				FLOAT KeyTime = Track->GetKeyframeTime(k);
				if(KeyTime >= IData->EdSectionStart && KeyTime <= IData->EdSectionEnd)
				{
					// Add to selection for deletion.
					AddKeyToSelection(Group, j, k, false);
				}
			}
		}
	}
}

/** Calculate the start and end of the range of the selected keys. */
void WxInterpEd::CalcSelectedKeyRange(FLOAT& OutStartTime, FLOAT& OutEndTime)
{
	if(Opt->SelectedKeys.Num() == 0)
	{
		OutStartTime = 0.f;
		OutEndTime = 0.f;
	}
	else
	{
		OutStartTime = BIG_NUMBER;
		OutEndTime = -BIG_NUMBER;

		for(INT i=0; i<Opt->SelectedKeys.Num(); i++)
		{
			UInterpTrack* Track = Opt->SelectedKeys(i).Group->InterpTracks( Opt->SelectedKeys(i).TrackIndex );
			FLOAT KeyTime = Track->GetKeyframeTime( Opt->SelectedKeys(i).KeyIndex );

			OutStartTime = ::Min(KeyTime, OutStartTime);
			OutEndTime = ::Max(KeyTime, OutEndTime);
		}
	}
}

void WxInterpEd::DeleteSelectedKeys(UBOOL bDoTransaction)
{
	if(bDoTransaction)
	{
		InterpEdTrans->BeginSpecial( *LocalizeUnrealEd("DeleteSelectedKeys") );
		Interp->Modify();
		Opt->Modify();
	}

	TArray<UInterpTrack*> ModifiedTracks;

	UBOOL bRemovedEventKeys = false;
	for(INT i=0; i<Opt->SelectedKeys.Num(); i++)
	{
		FInterpEdSelKey& SelKey = Opt->SelectedKeys(i);
		UInterpTrack* Track = SelKey.Group->InterpTracks( SelKey.TrackIndex );

		check(Track);
		check(SelKey.KeyIndex >= 0 && SelKey.KeyIndex < Track->GetNumKeyframes());

		if(bDoTransaction)
		{
			// If not already done so, call Modify on this track now.
			if( !ModifiedTracks.ContainsItem(Track) )
			{
				Track->Modify();
				ModifiedTracks.AddItem(Track);
			}
		}

		// If this is an event key - we update the connectors later.
		if(Track->IsA(UInterpTrackEvent::StaticClass()))
		{
			bRemovedEventKeys = true;
		}
			
		Track->RemoveKeyframe(SelKey.KeyIndex);

		// If any other keys in the selection are on the same track but after the one we just deleted, decrement the index to correct it.
		for(INT j=0; j<Opt->SelectedKeys.Num(); j++)
		{
			if( Opt->SelectedKeys(j).Group == SelKey.Group &&
				Opt->SelectedKeys(j).TrackIndex == SelKey.TrackIndex &&
				Opt->SelectedKeys(j).KeyIndex > SelKey.KeyIndex &&
				j != i)
			{
				Opt->SelectedKeys(j).KeyIndex--;
			}
		}
	}

	// If we removed some event keys - ensure all Matinee actions are up to date.
	if(bRemovedEventKeys)
	{
		UpdateMatineeActionConnectors();
	}

	// Update positions at current time, in case removal of the key changed things.
	RefreshInterpPosition();

	// Select no keyframe.
	ClearKeySelection();

	if(bDoTransaction)
	{
		InterpEdTrans->EndSpecial();
	}
}

void WxInterpEd::DuplicateSelectedKeys()
{
	InterpEdTrans->BeginSpecial( *LocalizeUnrealEd("DuplicateSelectedKeys") );
	Interp->Modify();
	Opt->Modify();

	TArray<UInterpTrack*> ModifiedTracks;

	for(INT i=0; i<Opt->SelectedKeys.Num(); i++)
	{
		FInterpEdSelKey& SelKey = Opt->SelectedKeys(i);
		UInterpTrack* Track = SelKey.Group->InterpTracks( SelKey.TrackIndex );

		check(Track);
		check(SelKey.KeyIndex >= 0 && SelKey.KeyIndex < Track->GetNumKeyframes());

		// If not already done so, call Modify on this track now.
		if( !ModifiedTracks.ContainsItem(Track) )
		{
			Track->Modify();
			ModifiedTracks.AddItem(Track);
		}
		
		FLOAT CurrentKeyTime = Track->GetKeyframeTime(SelKey.KeyIndex);
		FLOAT NewKeyTime = CurrentKeyTime + (FLOAT)DuplicateKeyOffset/PixelsPerSec;

		INT DupKeyIndex = Track->DuplicateKeyframe(SelKey.KeyIndex, NewKeyTime);

		// Change selection to select the new keyframe instead.
		SelKey.KeyIndex = DupKeyIndex;

		// If any other keys in the selection are on the same track but after the new key, increase the index to correct it.
		for(INT j=0; j<Opt->SelectedKeys.Num(); j++)
		{
			if( Opt->SelectedKeys(j).Group == SelKey.Group &&
				Opt->SelectedKeys(j).TrackIndex == SelKey.TrackIndex &&
				Opt->SelectedKeys(j).KeyIndex >= DupKeyIndex &&
				j != i)
			{
				Opt->SelectedKeys(j).KeyIndex++;
			}
		}
	}

	InterpEdTrans->EndSpecial();
}

/** Adjust the view so the entire sequence fits into the viewport. */
void WxInterpEd::ViewFitSequence()
{
	ViewStartTime = 0.f;
	ViewEndTime = IData->InterpLength;

	SyncCurveEdView();
}



/** Adjust the view so the selected keys fit into the viewport. */
void WxInterpEd::ViewFitToSelected()
{
	if( Opt->SelectedKeys.Num() > 0 )
	{
		FLOAT NewStartTime = BIG_NUMBER;
		FLOAT NewEndTime = -BIG_NUMBER;

		for( INT CurKeyIndex = 0; CurKeyIndex < Opt->SelectedKeys.Num(); ++CurKeyIndex )
		{
			FInterpEdSelKey& CurSelKey = Opt->SelectedKeys( CurKeyIndex );
			
			UInterpTrack* Track = CurSelKey.Group->InterpTracks( CurSelKey.TrackIndex );
			check( Track != NULL );
			check( CurSelKey.KeyIndex >= 0 && CurSelKey.KeyIndex < Track->GetNumKeyframes() );

			NewStartTime = Min( Track->GetKeyframeTime( CurSelKey.KeyIndex ), NewStartTime );
			NewEndTime = Max( Track->GetKeyframeTime( CurSelKey.KeyIndex ), NewEndTime );
		}

		// Clamp the minimum size
		if( NewStartTime - NewEndTime < 0.001f )
		{
			NewStartTime -= 0.005f;
			NewEndTime += 0.005f;
		}

		ViewStartTime = NewStartTime;
		ViewEndTime = NewEndTime;

		SyncCurveEdView();
	}
}



/** Adjust the view so the looped section fits into the viewport. */
void WxInterpEd::ViewFitLoop()
{
	// Do nothing if loop section is too small!
	FLOAT LoopRange = IData->EdSectionEnd - IData->EdSectionStart;
	if(LoopRange > 0.01f)
	{
		ViewStartTime = IData->EdSectionStart;
		ViewEndTime = IData->EdSectionEnd;

		SyncCurveEdView();
	}
}

/** Adjust the view so the looped section fits into the entire sequence. */
void WxInterpEd::ViewFitLoopSequence()
{
	// Adjust the looped section
	IData->EdSectionStart = 0.0f;
	IData->EdSectionEnd = IData->InterpLength;

	// Adjust the view
	ViewStartTime = IData->EdSectionStart;
	ViewEndTime = IData->EdSectionEnd;

	SyncCurveEdView();
}

/** Adjust the view by the defined range. */
void WxInterpEd::ViewFit(FLOAT StartTime, FLOAT EndTime)
{
	ViewStartTime = StartTime;
	ViewEndTime = EndTime;

	SyncCurveEdView();
}

/** Iterate over keys changing their interpolation mode and adjusting tangents appropriately. */
void WxInterpEd::ChangeKeyInterpMode(EInterpCurveMode NewInterpMode/*=CIM_Unknown*/)
{
	for(INT i=0; i<Opt->SelectedKeys.Num(); i++)
	{
		FInterpEdSelKey& SelKey = Opt->SelectedKeys(i);
		UInterpTrack* Track = SelKey.Group->InterpTracks(SelKey.TrackIndex);

		UInterpTrackMove* MoveTrack = Cast<UInterpTrackMove>(Track);
		if(MoveTrack)
		{
			MoveTrack->PosTrack.Points(SelKey.KeyIndex).InterpMode = NewInterpMode;
			MoveTrack->EulerTrack.Points(SelKey.KeyIndex).InterpMode = NewInterpMode;

			MoveTrack->PosTrack.AutoSetTangents(MoveTrack->LinCurveTension);
			MoveTrack->EulerTrack.AutoSetTangents(MoveTrack->AngCurveTension);
		}

		UInterpTrackFloatBase* FloatTrack = Cast<UInterpTrackFloatBase>(Track);
		if(FloatTrack)
		{
			FloatTrack->FloatTrack.Points(SelKey.KeyIndex).InterpMode = NewInterpMode;

			FloatTrack->FloatTrack.AutoSetTangents(FloatTrack->CurveTension);
		}

		UInterpTrackVectorBase* VectorTrack = Cast<UInterpTrackVectorBase>(Track);
		if(VectorTrack)
		{
			VectorTrack->VectorTrack.Points(SelKey.KeyIndex).InterpMode = NewInterpMode;

			VectorTrack->VectorTrack.AutoSetTangents(VectorTrack->CurveTension);
		}
	}

	CurveEd->UpdateDisplay();
}

/** Increments the cursor or selected keys by 1 interval amount, as defined by the toolbar combo. */
void WxInterpEd::IncrementSelection()
{
	UBOOL bMoveMarker = TRUE;

	if(Opt->SelectedKeys.Num())
	{
		BeginMoveSelectedKeys();
		{
			MoveSelectedKeys(SnapAmount);
		}
		EndMoveSelectedKeys();
		bMoveMarker = FALSE;
	}


	// Move the interp marker if there are no keys selected.
	if(bMoveMarker)
	{
		FLOAT StartTime = Interp->Position;
		if( bSnapToFrames && bSnapTimeToFrames )
		{
			StartTime = SnapTimeToNearestFrame( Interp->Position );
		}

		SetInterpPosition( StartTime + SnapAmount );
	}
}

/** Decrements the cursor or selected keys by 1 interval amount, as defined by the toolbar combo. */
void WxInterpEd::DecrementSelection()
{
	UBOOL bMoveMarker = TRUE;

	if(Opt->SelectedKeys.Num())
	{
		BeginMoveSelectedKeys();
		{
			MoveSelectedKeys(-SnapAmount);
		}
		EndMoveSelectedKeys();
		bMoveMarker = FALSE;
	}

	// Move the interp marker if there are no keys selected.
	if(bMoveMarker)
	{
		FLOAT StartTime = Interp->Position;
		if( bSnapToFrames && bSnapTimeToFrames )
		{
			StartTime = SnapTimeToNearestFrame( Interp->Position );
		}

		SetInterpPosition( StartTime - SnapAmount );
	}
}

void WxInterpEd::SelectNextKey()
{
	if(ActiveTrackIndex != INDEX_NONE)
	{
		check(ActiveGroup);
		UInterpTrack* Track = ActiveGroup->InterpTracks(ActiveTrackIndex);
		check(Track);

		INT NumKeys = Track->GetNumKeyframes();

		if(NumKeys)
		{
			INT i;
			for(i=0; i < NumKeys-1 && Track->GetKeyframeTime(i) < (Interp->Position + KINDA_SMALL_NUMBER); i++);
	
			ClearKeySelection();
			AddKeyToSelection(ActiveGroup, ActiveTrackIndex, i, true);
		}
	}
}

void WxInterpEd::SelectPreviousKey()
{
	if(ActiveTrackIndex != INDEX_NONE)
	{
		check(ActiveGroup);
		UInterpTrack* Track = ActiveGroup->InterpTracks(ActiveTrackIndex);
		check(Track);

		INT NumKeys = Track->GetNumKeyframes();

		if(NumKeys)
		{
			INT i;
			for(i=NumKeys-1; i > 0 && Track->GetKeyframeTime(i) > (Interp->Position - KINDA_SMALL_NUMBER); i--);

			ClearKeySelection();
			AddKeyToSelection(ActiveGroup, ActiveTrackIndex, i, true);
		}
	}
}

/** Turns snap on and off in Matinee. Updates state of snap button as well. */
void WxInterpEd::SetSnapEnabled(UBOOL bInSnapEnabled)
{
	bSnapEnabled = bInSnapEnabled;

	if(bSnapToKeys)
	{
		CurveEd->SetInSnap(false, SnapAmount, bSnapToFrames);
	}
	else
	{
		CurveEd->SetInSnap(bSnapEnabled, SnapAmount, bSnapToFrames);
	}

	// Update button status.
	ToolBar->ToggleTool(IDM_INTERP_TOGGLE_SNAP, bSnapEnabled == TRUE);
	// Save to ini when it changes.
	GConfig->SetBool( TEXT("Matinee"), TEXT("SnapEnabled"), bSnapEnabled, GEditorUserSettingsIni );
}


/** Toggles snapping the current timeline position to 'frames' in Matinee. */
void WxInterpEd::SetSnapTimeToFrames( UBOOL bInValue )
{
	bSnapTimeToFrames = bInValue;

	// Update button status.
	ToolBar->ToggleTool( IDM_INTERP_TOGGLE_SNAP_TIME_TO_FRAMES, ( bSnapToFrames && bSnapTimeToFrames ) );
	ToolBar->EnableTool( IDM_INTERP_TOGGLE_SNAP_TIME_TO_FRAMES, bSnapToFrames == TRUE );
	
	// Save to ini when it changes.
	GConfig->SetBool( TEXT("Matinee"), TEXT("SnapTimeToFrames"), bSnapTimeToFrames, GEditorUserSettingsIni );

	// Go ahead and apply the change right now if we need to
	if( IsInitialized() && bSnapToFrames && bSnapTimeToFrames )
	{
		SetInterpPosition( SnapTimeToNearestFrame( Interp->Position ) );
	}
}



/** Toggles fixed time step mode */
void WxInterpEd::SetFixedTimeStepPlayback( UBOOL bInValue )
{
	bFixedTimeStepPlayback = bInValue;

	// Update button status.
	ToolBar->ToggleTool( IDM_INTERP_FixedTimeStepPlayback, ( bSnapToFrames && bFixedTimeStepPlayback ) );
	ToolBar->EnableTool( IDM_INTERP_FixedTimeStepPlayback, bSnapToFrames == TRUE );
	
	// Save to ini when it changes.
	GConfig->SetBool( TEXT("Matinee"), TEXT("FixedTimeStepPlayback"), bFixedTimeStepPlayback, GEditorUserSettingsIni );

	// Update fixed time step state
	UpdateFixedTimeStepPlayback();
}



/** Updates 'fixed time step' mode based on current playback state and user preferences */
void WxInterpEd::UpdateFixedTimeStepPlayback()
{
	// Turn on 'benchmarking' mode if we're using a fixed time step
	GIsBenchmarking = Interp->bIsPlaying && bSnapToFrames && bFixedTimeStepPlayback;

	// Set the time interval between fixed ticks
	GFixedDeltaTime = SnapAmount;
}



/** Toggles 'prefer frame numbers' setting */
void WxInterpEd::SetPreferFrameNumbers( UBOOL bInValue )
{
	bPreferFrameNumbers = bInValue;

	// Update button status.
	ToolBar->ToggleTool( IDM_INTERP_PreferFrameNumbers, ( bSnapToFrames && bPreferFrameNumbers ) );
	ToolBar->EnableTool( IDM_INTERP_PreferFrameNumbers, bSnapToFrames == TRUE );
	
	// Save to ini when it changes.
	GConfig->SetBool( TEXT("Matinee"), TEXT("PreferFrameNumbers"), bPreferFrameNumbers, GEditorUserSettingsIni );
}



/** Toggles 'show time cursor pos for all keys' setting */
void WxInterpEd::SetShowTimeCursorPosForAllKeys( UBOOL bInValue )
{
	bShowTimeCursorPosForAllKeys = bInValue;

	// Update button status.
	ToolBar->ToggleTool( IDM_INTERP_ShowTimeCursorPosForAllKeys, bShowTimeCursorPosForAllKeys ? TRUE : FALSE );
	ToolBar->EnableTool( IDM_INTERP_ShowTimeCursorPosForAllKeys, TRUE );
	
	// Save to ini when it changes.
	GConfig->SetBool( TEXT("Matinee"), TEXT("ShowTimeCursorPosForAllKeys"), bShowTimeCursorPosForAllKeys, GEditorUserSettingsIni );
}



/** Snaps the specified time value to the closest frame */
FLOAT WxInterpEd::SnapTimeToNearestFrame( FLOAT InTime ) const
{
	// Compute the new time value by rounding
	const INT InterpPositionInFrames = appRound( InTime / SnapAmount );
	const FLOAT NewTime = InterpPositionInFrames * SnapAmount;

	return NewTime;
}



/** Take the InTime and snap it to the current SnapAmount. Does nothing if bSnapEnabled is false */
FLOAT WxInterpEd::SnapTime(FLOAT InTime, UBOOL bIgnoreSelectedKeys)
{
	if(bSnapEnabled)
	{
		if(bSnapToKeys)
		{
			// Iterate over all tracks finding the closest snap position to the supplied time.

			UBOOL bFoundSnap = false;
			FLOAT BestSnapPos = 0.f;
			FLOAT BestSnapDist = BIG_NUMBER;
			for(INT i=0; i<IData->InterpGroups.Num(); i++)
			{
				UInterpGroup* Group = IData->InterpGroups(i);
				for(INT j=0; j<Group->InterpTracks.Num(); j++)
				{
					UInterpTrack* Track = Group->InterpTracks(j);

					// If we are ignoring selected keys - build an array of the indices of selected keys on this track.
					TArray<INT> IgnoreKeys;
					if(bIgnoreSelectedKeys)
					{
						for(INT SelIndex=0; SelIndex<Opt->SelectedKeys.Num(); SelIndex++)
						{
							if( Opt->SelectedKeys(SelIndex).Group == Group && 
								Opt->SelectedKeys(SelIndex).TrackIndex == j )
							{
								IgnoreKeys.AddUniqueItem( Opt->SelectedKeys(SelIndex).KeyIndex );
							}
						}
					}

					FLOAT OutPos = 0.f;
					UBOOL bTrackSnap = Track->GetClosestSnapPosition(InTime, IgnoreKeys, OutPos);
					if(bTrackSnap) // If we found a snap location
					{
						// See if its closer than the closest location so far.
						FLOAT SnapDist = Abs(InTime - OutPos);
						if(SnapDist < BestSnapDist)
						{
							BestSnapPos = OutPos;
							BestSnapDist = SnapDist;
							bFoundSnap = true;
						}
					}
				}
			}

			// Find how close we have to get to snap, in 'time' instead of pixels.
			FLOAT SnapTolerance = (FLOAT)KeySnapPixels/(FLOAT)PixelsPerSec;

			// If we are close enough to snap position - do it.
			if(bFoundSnap && (BestSnapDist < SnapTolerance))
			{
				bDrawSnappingLine = true;
				SnappingLinePosition = BestSnapPos;

				return BestSnapPos;
			}
			else
			{
				bDrawSnappingLine = false;

				return InTime;
			}
		}
		else
		{	
			// Don't draw snapping line when just snapping to grid.
			bDrawSnappingLine = false;

			return SnapTimeToNearestFrame( InTime );
		}
	}
	else
	{
		bDrawSnappingLine = false;

		return InTime;
	}
}

void WxInterpEd::BeginMoveMarker()
{
	if(GrabbedMarkerType == ISM_SeqEnd)
	{
		UnsnappedMarkerPos = IData->InterpLength;
		InterpEdTrans->BeginSpecial( *LocalizeUnrealEd("MoveEndMarker") );
		IData->Modify();
	}
	else if(GrabbedMarkerType == ISM_LoopStart)
	{
		UnsnappedMarkerPos = IData->EdSectionStart;
		InterpEdTrans->BeginSpecial( *LocalizeUnrealEd("MoveLoopStartMarker") );
		IData->Modify();
	}
	else if(GrabbedMarkerType == ISM_LoopEnd)
	{
		UnsnappedMarkerPos = IData->EdSectionEnd;
		InterpEdTrans->BeginSpecial( *LocalizeUnrealEd("MoveLoopEndMarker") );
		IData->Modify();
	}
}

void WxInterpEd::EndMoveMarker()
{
	if(	GrabbedMarkerType == ISM_SeqEnd || 
		GrabbedMarkerType == ISM_LoopStart || 
		GrabbedMarkerType == ISM_LoopEnd)
	{
		InterpEdTrans->EndSpecial();
	}
}

void WxInterpEd::SetInterpEnd(FLOAT NewInterpLength)
{
	// Ensure non-negative end time.
	IData->InterpLength = ::Max(NewInterpLength, 0.f);
	
	CurveEd->SetEndMarker(true, IData->InterpLength);

	// Ensure the current position is always inside the valid sequence area.
	if(Interp->Position > IData->InterpLength)
	{
		SetInterpPosition(IData->InterpLength);
	}

	// Ensure loop points are inside sequence.
	IData->EdSectionStart = ::Clamp(IData->EdSectionStart, 0.f, IData->InterpLength);
	IData->EdSectionEnd = ::Clamp(IData->EdSectionEnd, 0.f, IData->InterpLength);
	CurveEd->SetRegionMarker(true, IData->EdSectionStart, IData->EdSectionEnd, RegionFillColor);
}

void WxInterpEd::MoveLoopMarker(FLOAT NewMarkerPos, UBOOL bIsStart)
{
	if(bIsStart)
	{
		IData->EdSectionStart = NewMarkerPos;
		IData->EdSectionEnd = ::Max(IData->EdSectionStart, IData->EdSectionEnd);				
	}
	else
	{
		IData->EdSectionEnd = NewMarkerPos;
		IData->EdSectionStart = ::Min(IData->EdSectionStart, IData->EdSectionEnd);
	}

	// Ensure loop points are inside sequence.
	IData->EdSectionStart = ::Clamp(IData->EdSectionStart, 0.f, IData->InterpLength);
	IData->EdSectionEnd = ::Clamp(IData->EdSectionEnd, 0.f, IData->InterpLength);

	CurveEd->SetRegionMarker(true, IData->EdSectionStart, IData->EdSectionEnd, RegionFillColor);
}

void WxInterpEd::BeginMoveSelectedKeys()
{
	InterpEdTrans->BeginSpecial( *LocalizeUnrealEd("MoveSelectedKeys") );
	Opt->Modify();

	TArray<UInterpTrack*> ModifiedTracks;
	for(INT i=0; i<Opt->SelectedKeys.Num(); i++)
	{
		FInterpEdSelKey& SelKey = Opt->SelectedKeys(i);

		UInterpTrack* Track = SelKey.Group->InterpTracks(SelKey.TrackIndex);
		check(Track);

		// If not already done so, call Modify on this track now.
		if( !ModifiedTracks.ContainsItem(Track) )
		{
			Track->Modify();
			ModifiedTracks.AddItem(Track);
		}

		SelKey.UnsnappedPosition = Track->GetKeyframeTime(SelKey.KeyIndex);
	}

	// When moving a key in time, turn off 'recording', so we dont end up assigning an objects location at one time to a key at another time.
	bAdjustingKeyframe = FALSE;
}

void WxInterpEd::EndMoveSelectedKeys()
{
	InterpEdTrans->EndSpecial();
}

void WxInterpEd::MoveSelectedKeys(FLOAT DeltaTime)
{
	for(INT i=0; i<Opt->SelectedKeys.Num(); i++)
	{
		FInterpEdSelKey& SelKey = Opt->SelectedKeys(i);

		UInterpTrack* Track = SelKey.Group->InterpTracks(SelKey.TrackIndex);
		check(Track);

		SelKey.UnsnappedPosition += DeltaTime;
		FLOAT NewTime = SnapTime(SelKey.UnsnappedPosition, true);

		// Do nothing if already at target time.
		if( Track->GetKeyframeTime(SelKey.KeyIndex) != NewTime )
		{
			INT OldKeyIndex = SelKey.KeyIndex;
			INT NewKeyIndex = Track->SetKeyframeTime(SelKey.KeyIndex, NewTime);
			SelKey.KeyIndex = NewKeyIndex;

			// If the key changed index we need to search for any other selected keys on this track that may need their index adjusted because of this change.
			INT KeyMove = NewKeyIndex - OldKeyIndex;
			if(KeyMove > 0)
			{
				for(INT j=0; j<Opt->SelectedKeys.Num(); j++)
				{
					if( j == i ) // Don't look at one we just changed.
						continue;

					FInterpEdSelKey& TestKey = Opt->SelectedKeys(j);
					if( TestKey.TrackIndex == SelKey.TrackIndex && 
						TestKey.Group == SelKey.Group &&
						TestKey.KeyIndex > OldKeyIndex && 
						TestKey.KeyIndex <= NewKeyIndex)
					{
						TestKey.KeyIndex--;
					}
				}
			}
			else if(KeyMove < 0)
			{
				for(INT j=0; j<Opt->SelectedKeys.Num(); j++)
				{
					if( j == i )
						continue;

					FInterpEdSelKey& TestKey = Opt->SelectedKeys(j);
					if( TestKey.TrackIndex == SelKey.TrackIndex && 
						TestKey.Group == SelKey.Group &&
						TestKey.KeyIndex < OldKeyIndex && 
						TestKey.KeyIndex >= NewKeyIndex)
					{
						TestKey.KeyIndex++;
					}
				}
			}
		}

	} // FOR each selected key

	// Update positions at current time but with new keyframe times.
	RefreshInterpPosition();

	CurveEd->UpdateDisplay();
}


void WxInterpEd::BeginDrag3DHandle(UInterpGroup* Group, INT TrackIndex)
{
	if(TrackIndex < 0 || TrackIndex >= Group->InterpTracks.Num())
	{
		return;
	}

	UInterpTrackMove* MoveTrack = Cast<UInterpTrackMove>( Group->InterpTracks(TrackIndex) );
	if(MoveTrack)
	{
		InterpEdTrans->BeginSpecial( *LocalizeUnrealEd("Drag3DTrajectoryHandle") );
		MoveTrack->Modify();
		bDragging3DHandle = true;
	}
}

void WxInterpEd::Move3DHandle(UInterpGroup* Group, INT TrackIndex, INT KeyIndex, UBOOL bArriving, const FVector& Delta)
{
	if(!bDragging3DHandle)
	{
		return;
	}

	if(TrackIndex < 0 || TrackIndex >= Group->InterpTracks.Num())
	{
		return;
	}

	UInterpTrackMove* MoveTrack = Cast<UInterpTrackMove>( Group->InterpTracks(TrackIndex) );
	if(MoveTrack)
	{
		if(KeyIndex < 0 || KeyIndex >= MoveTrack->PosTrack.Points.Num())
		{
			return;
		}

		UInterpGroupInst* GrInst = Interp->FindFirstGroupInst(Group);
		check(GrInst);
		check(GrInst->TrackInst.Num() == Group->InterpTracks.Num());
		UInterpTrackInstMove* MoveInst = CastChecked<UInterpTrackInstMove>( GrInst->TrackInst(TrackIndex) );

		FMatrix InvRefTM = MoveTrack->GetMoveRefFrame( MoveInst ).Inverse();
		FVector LocalDelta = InvRefTM.TransformNormal(Delta);

		BYTE InterpMode = MoveTrack->PosTrack.Points(KeyIndex).InterpMode;

		if(bArriving)
		{
			MoveTrack->PosTrack.Points(KeyIndex).ArriveTangent -= LocalDelta;

			// If keeping tangents smooth, update the LeaveTangent
			if(InterpMode != CIM_CurveBreak)
			{
				MoveTrack->PosTrack.Points(KeyIndex).LeaveTangent = MoveTrack->PosTrack.Points(KeyIndex).ArriveTangent;
			}
		}
		else
		{
			MoveTrack->PosTrack.Points(KeyIndex).LeaveTangent += LocalDelta;

			// If keeping tangents smooth, update the ArriveTangent
			if(InterpMode != CIM_CurveBreak)
			{
				MoveTrack->PosTrack.Points(KeyIndex).ArriveTangent = MoveTrack->PosTrack.Points(KeyIndex).LeaveTangent;
			}
		}

		// If adjusting an 'Auto' keypoint, switch it to 'User'
		if(InterpMode == CIM_CurveAuto || InterpMode == CIM_CurveAutoClamped)
		{
			MoveTrack->PosTrack.Points(KeyIndex).InterpMode = CIM_CurveUser;
			MoveTrack->EulerTrack.Points(KeyIndex).InterpMode = CIM_CurveUser;
		}

		// Update the curve editor to see curves change.
		CurveEd->UpdateDisplay();
	}
}

void WxInterpEd::EndDrag3DHandle()
{
	if(bDragging3DHandle)
	{
		InterpEdTrans->EndSpecial();
	}
}

void WxInterpEd::MoveInitialPosition(const FVector& Delta, const FRotator& DeltaRot)
{
	// If no group selected, do nothing
	if(!ActiveGroup)
	{
		return;
	}

	FRotationTranslationMatrix RotMatrix(DeltaRot, FVector(0));
	FTranslationMatrix TransMatrix(Delta);

	// Find all instances of selected group
	for(INT i=0; i<Interp->GroupInst.Num(); i++)
	{
		UInterpGroupInst* GrInst = Interp->GroupInst(i);
		if(GrInst->Group == ActiveGroup)
		{
			// Look for an instance of a movement track
			for(INT j=0; j<GrInst->TrackInst.Num(); j++)
			{
				UInterpTrackInstMove* MoveInst = Cast<UInterpTrackInstMove>(GrInst->TrackInst(j));
				if(MoveInst)
				{
					// Apply to reference frame of movement track.

					// Translate to origin and rotate, then apply translation.
					FVector InitialOrigin = MoveInst->InitialTM.GetOrigin();
					MoveInst->InitialTM.SetOrigin(FVector(0));
					MoveInst->InitialTM = MoveInst->InitialTM * RotMatrix;
					MoveInst->InitialTM.SetOrigin(InitialOrigin);
					MoveInst->InitialTM = MoveInst->InitialTM * TransMatrix;

					FMatrix ResetTM = FRotationTranslationMatrix(MoveInst->ResetRotation, MoveInst->ResetLocation);

					// Apply to reset information as well.
					
					FVector ResetOrigin = ResetTM.GetOrigin();
					ResetTM.SetOrigin(FVector(0));
					ResetTM = ResetTM * RotMatrix;
					ResetTM.SetOrigin(ResetOrigin);
					ResetTM = ResetTM * TransMatrix;

					MoveInst->ResetLocation = ResetTM.GetOrigin();
					MoveInst->ResetRotation = ResetTM.Rotator();
				}
			}
		}
	}

	RefreshInterpPosition();

	// Dirty the track window viewports
	InvalidateTrackWindowViewports();
}

void WxInterpEd::AddKey()
{
	if(ActiveTrackIndex == INDEX_NONE)
		appMsgf(AMT_OK,*LocalizeUnrealEd("NoActiveTrack"));
	else
	{
		check(ActiveGroup);
		UInterpTrack* Track = ActiveGroup->InterpTracks(ActiveTrackIndex);
		check(Track);

		UInterpGroupInst* GrInst = Interp->FindFirstGroupInst(ActiveGroup);
		check(GrInst);

		UInterpTrackInst* TrInst = GrInst->TrackInst(ActiveTrackIndex);
		check(TrInst);

		UInterpTrackHelper* TrackHelper = NULL;
		UClass	*Class = LoadObject<UClass>( NULL, *Track->GetEdHelperClassName(), NULL, LOAD_None, NULL );
		if ( Class != NULL )
		{
			TrackHelper = CastChecked<UInterpTrackHelper>(Class->GetDefaultObject());
		}

		FLOAT	fKeyTime = SnapTime( Interp->Position, false );
		if ( (TrackHelper == NULL) || !TrackHelper->PreCreateKeyframe(Track, fKeyTime) )
		{
			return;
		}

		InterpEdTrans->BeginSpecial( *LocalizeUnrealEd("AddKey") );
		Track->Modify();
		Opt->Modify();

		// Add key at current time, snapped to the grid if its on.
		INT NewKeyIndex = Track->AddKeyframe( fKeyTime, TrInst, InitialInterpMode );

		// If we failed to add the keyframe - bail out now.
		if(NewKeyIndex == INDEX_NONE)
		{
			appMsgf( AMT_OK, *LocalizeUnrealEd("NothingToKeyframe") );
		}
		else
		{
			// Select the newly added keyframe.
			ClearKeySelection();
			AddKeyToSelection(ActiveGroup, ActiveTrackIndex, NewKeyIndex, true); // Probably don't need to auto-wind - should already be there!

			TrackHelper->PostCreateKeyframe( Track, NewKeyIndex );

			// Update to current time, in case new key affects state of scene.
			RefreshInterpPosition();

			// Dirty the track window viewports
			InvalidateTrackWindowViewports();
		}

		InterpEdTrans->EndSpecial();
	}
}

/** Call utitlity to split an animtion in ths selected AnimControl track. */
void WxInterpEd::SplitAnimKey()
{
	// Check we have a group and track selected
	if(!ActiveGroup)
	{
		return;
	}

	// Check its an AnimControlTrack
	UInterpTrackAnimControl* AnimTrack = Cast<UInterpTrackAnimControl>(ActiveGroup->InterpTracks(ActiveTrackIndex));
	if(!AnimTrack)
	{
		return;
	}

	// Call split utility.
	INT NewKeyIndex = AnimTrack->SplitKeyAtPosition(Interp->Position);

	// If we created a new key - select it by default.
	if(NewKeyIndex != INDEX_NONE)
	{
		ClearKeySelection();
		AddKeyToSelection(ActiveGroup, ActiveTrackIndex, NewKeyIndex, false);
	}
}

/**
 * Copies the currently selected track.
 *
 * @param bCut	Whether or not we should cut instead of simply copying the track.
 */
void WxInterpEd::CopySelectedGroupOrTrack(UBOOL bCut)
{
	if(!ActiveGroup)
	{
		appMsgf(AMT_OK, *LocalizeUnrealEd("NeedToSelectGroupToCopy"));
		return;
	}

	// If no tracks are selected, copy the group.
	if(ActiveTrackIndex == INDEX_NONE)
	{
		GUnrealEd->MatineeCopyPasteBuffer = (UObject*)UObject::StaticDuplicateObject(ActiveGroup, ActiveGroup, 
			UObject::GetTransientPackage(), NULL);

		// Delete the active group if we are doing a cut operation.
		if(bCut)
		{
			InterpEdTrans->BeginSpecial(  *LocalizeUnrealEd(TEXT("CutActiveTrackOrGroup")) );
			{
				ActiveGroup->Modify();
				DeleteSelectedGroup();
			}
			InterpEdTrans->EndSpecial();
		}
	}
	else
	{
		UInterpTrack* SelectedTrack = ActiveGroup->InterpTracks(ActiveTrackIndex);
		GUnrealEd->MatineeCopyPasteBuffer = (UObject*)UObject::StaticDuplicateObject(SelectedTrack, SelectedTrack, 
			UObject::GetTransientPackage(), NULL);

		// Delete the originating track if we are cutting.
		if(bCut)
		{
			InterpEdTrans->BeginSpecial(  *LocalizeUnrealEd(TEXT("CutActiveTrackOrGroup")) );
			{
				ActiveGroup->Modify();
				DeleteSelectedTrack();
			}
			InterpEdTrans->EndSpecial();
		}
	}
}

/**
 * Pastes the previously copied track.
 */
void WxInterpEd::PasteSelectedGroupOrTrack()
{
	// See if we are pasting a track or group.
	if(GUnrealEd->MatineeCopyPasteBuffer)
	{
		if(GUnrealEd->MatineeCopyPasteBuffer->IsA(UInterpGroup::StaticClass()))
		{
			DuplicateGroup(Cast<UInterpGroup>(GUnrealEd->MatineeCopyPasteBuffer));
		}
		else if(GUnrealEd->MatineeCopyPasteBuffer->IsA(UInterpTrack::StaticClass()))
		{	
			if(!ActiveGroup)
			{
				appMsgf(AMT_OK, *LocalizeUnrealEd("NeedToSelectGroupToPaste"));
				return;
			}

			UInterpTrack* TrackToPaste = Cast<UInterpTrack>(GUnrealEd->MatineeCopyPasteBuffer);

			if(TrackToPaste)
			{
				InterpEdTrans->BeginSpecial(  *LocalizeUnrealEd(TEXT("PasteActiveTrackOrGroup")) );
				{
					ActiveGroup->Modify();
					AddTrackToSelectedGroup(TrackToPaste->GetClass(), TrackToPaste);
				}
				InterpEdTrans->EndSpecial();
			}
		}
	}
}

/**
 * @return Whether or not we can paste a track/group.
 */
UBOOL WxInterpEd::CanPasteGroupOrTrack()
{
	UBOOL bResult = FALSE;

	if(GUnrealEd->MatineeCopyPasteBuffer)
	{
		if(GUnrealEd->MatineeCopyPasteBuffer->IsA(UInterpGroup::StaticClass()))
		{
			bResult = TRUE;
		}
		else if(GUnrealEd->MatineeCopyPasteBuffer->IsA(UInterpTrack::StaticClass()) && ActiveGroup != NULL)
		{
			bResult = TRUE;
		}
	}

	return bResult;
}




/**
 * Adds a new track to the specified group.
 *
 * @param Group The group to add a track to
 * @param TrackClass The class of track object we are going to add.
 * @param TrackToCopy A optional track to copy instead of instantiating a new one.
 * @param bAllowPrompts TRUE if we should allow a dialog to be summoned to ask for initial information
 * @param OutNewTrackIndex [Out] The index of the newly created track in its parent group
 *
 * @return Returns newly created track (or NULL if failed)
 */
UInterpTrack* WxInterpEd::AddTrackToGroup( UInterpGroup* Group, UClass* TrackClass, UInterpTrack* TrackToCopy, UBOOL bAllowPrompts, INT& OutNewTrackIndex )
{
	OutNewTrackIndex = INDEX_NONE;

	if( Group == NULL )
	{
		return NULL;
	}

	UInterpGroupInst* GrInst = Interp->FindFirstGroupInst( Group );
	check(GrInst);

	UInterpTrack* TrackDef = TrackClass->GetDefaultObject<UInterpTrack>();

	UInterpTrackHelper* TrackHelper = NULL;
	UBOOL bCopyingTrack = (TrackToCopy!=NULL);
	UClass	*Class = LoadObject<UClass>( NULL, *TrackDef->GetEdHelperClassName(), NULL, LOAD_None, NULL );
	if ( Class != NULL )
	{
		TrackHelper = CastChecked<UInterpTrackHelper>(Class->GetDefaultObject());
	}

	if ( (TrackHelper == NULL) || !TrackHelper->PreCreateTrack( Group, TrackDef, bCopyingTrack, bAllowPrompts ) )
	{
		return NULL;
	}

	Group->Modify();

	// Construct track and track instance objects.
	UInterpTrack* NewTrack = NULL;
	if(TrackToCopy)
	{
		NewTrack = Cast<UInterpTrack>(UObject::StaticDuplicateObject(TrackToCopy, TrackToCopy, Group, NULL ));
	}
	else
	{
		NewTrack = ConstructObject<UInterpTrack>( TrackClass, Group, NAME_None, RF_Transactional );
	}

	check(NewTrack);

	OutNewTrackIndex = Group->InterpTracks.AddItem(NewTrack);

	check( NewTrack->TrackInstClass );
	check( NewTrack->TrackInstClass->IsChildOf(UInterpTrackInst::StaticClass()) );

	TrackHelper->PostCreateTrack( NewTrack, bCopyingTrack, OutNewTrackIndex );

	if(bCopyingTrack == FALSE)
	{
		NewTrack->SetTrackToSensibleDefault();
	}

	NewTrack->Modify();

	// We need to create a InterpTrackInst in each instance of the active group (the one we are adding the track to).
	for(INT i=0; i<Interp->GroupInst.Num(); i++)
	{
		UInterpGroupInst* GrInst = Interp->GroupInst(i);
		if(GrInst->Group == Group)
		{
			GrInst->Modify();

			UInterpTrackInst* NewTrackInst = ConstructObject<UInterpTrackInst>( NewTrack->TrackInstClass, GrInst, NAME_None, RF_Transactional );

			const INT NewInstIndex = GrInst->TrackInst.AddItem(NewTrackInst);
			check(NewInstIndex == OutNewTrackIndex);

			// Initialise track, giving selected object.
			NewTrackInst->InitTrackInst(NewTrack);

			// Save state into new track before doing anything else (because we didn't do it on ed mode change).
			NewTrackInst->SaveActorState(NewTrack);
			NewTrackInst->Modify();
		}
	}

	if(bCopyingTrack == FALSE)
	{
		// Bit of a hack here, but useful. Whenever you put down a movement track, add a key straight away at the start.
		// Should be ok to add at the start, because it should not be having its location (or relative location) changed in any way already as we scrub.
		UInterpTrackMove* MoveTrack = Cast<UInterpTrackMove>(NewTrack);
		if(MoveTrack)
		{
			UInterpGroupInst* GrInst = Interp->FindFirstGroupInst(Group);
			UInterpTrackInst* TrInst = GrInst->TrackInst(OutNewTrackIndex);
			MoveTrack->AddKeyframe( 0.0f, TrInst, InitialInterpMode );
		}
	}

	return NewTrack;
}



/**
 * Adds a new track to the selected group.
 *
 * @param TrackClass		The class of track object we are going to add.
 * @param TrackToCopy		A optional track to copy instead of instantiating a new one.
 */
void WxInterpEd::AddTrackToSelectedGroup(UClass* TrackClass, UInterpTrack* TrackToCopy)
{
	if(!ActiveGroup)
	{
		return;
	}


	UInterpGroupInst* GrInst = Interp->FindFirstGroupInst(ActiveGroup);
	check(GrInst);

	UInterpTrack* TrackDef = TrackClass->GetDefaultObject<UInterpTrack>();

	// If bOnePerGrouop - check we don't already have a track of this type in the group.
	if(TrackDef->bOnePerGroup)
	{
		for(INT i=0; i<ActiveGroup->InterpTracks.Num(); i++)
		{
			if( ActiveGroup->InterpTracks(i)->GetClass() == TrackClass )
			{
				appMsgf(AMT_OK, *LocalizeUnrealEd("Error_OnlyOneTrackTypePerGroup"));
				return;
			}
		}
	}

	// Warn when creating dynamic track on a static actor, warn and offer to bail out.
	if(TrackDef->AllowStaticActors()==FALSE)
	{
		AActor* GrActor = GrInst->GetGroupActor();
		if(GrActor && GrActor->bStatic)
		{
			const UBOOL bConfirm = appMsgf(AMT_YesNo, *LocalizeUnrealEd("WarnNewMoveTrackOnStatic"));
			if(!bConfirm)
			{
				return;
			}
		}
	}


	InterpEdTrans->BeginSpecial( *LocalizeUnrealEd("NewTrack") );

	// Add the track!
	INT NewTrackIndex = INDEX_NONE;
	AddTrackToGroup( ActiveGroup, TrackClass, TrackToCopy, TRUE, NewTrackIndex );

	InterpEdTrans->EndSpecial();

	// Select the new track
	ClearKeySelection();

	// Make sure particle replay tracks have up-to-date editor-only transient state
	UpdateParticleReplayTracks();

	SetActiveTrack(ActiveGroup, NewTrackIndex);

	// A new track may have been added, so we'll need to update the scroll bar
	UpdateTrackWindowScrollBars();

	// Update graphics to show new track!
	InvalidateTrackWindowViewports();

	// If we added a movement track to this group, we'll need to make sure that the actor's transformations are captured
	// so that we can restore them later.
	Interp->RecaptureActorState();
}



/** Deletes the currently active track. */
void WxInterpEd::DeleteSelectedTrack()
{
	if(ActiveTrackIndex == INDEX_NONE)
	{
		return;
	}

	check(ActiveGroup);

	InterpEdTrans->BeginSpecial( *LocalizeUnrealEd("TrackDelete") );
	Interp->Modify();
	IData->Modify();

	ActiveGroup->Modify();

	UInterpTrack* ActiveTrack = ActiveGroup->InterpTracks(ActiveTrackIndex);
	check(ActiveTrack);

	ActiveTrack->Modify();

	for(INT i=0; i<Interp->GroupInst.Num(); i++)
	{
		UInterpGroupInst* GrInst = Interp->GroupInst(i);
		if(GrInst->Group == ActiveGroup)
		{
			check( ActiveTrackIndex >= 0 && ActiveTrackIndex < GrInst->TrackInst.Num() );
			UInterpTrackInst* TrInst = GrInst->TrackInst(ActiveTrackIndex);

			GrInst->Modify();
			TrInst->Modify();

			// Before deleting this track - find each instance of it and restore state.
			TrInst->RestoreActorState( GrInst->Group->InterpTracks(ActiveTrackIndex) );

			GrInst->TrackInst.Remove(ActiveTrackIndex);
		}
	}

	// Remove from the InterpGroup and free.
	ActiveGroup->InterpTracks.Remove(ActiveTrackIndex);
	ActiveTrackIndex = INDEX_NONE;

	// Remove from the Curve editor, if its there.
	IData->CurveEdSetup->RemoveCurve(ActiveTrack);
	CurveEd->CurveChanged();

	// If this is an Event track, update the event outputs of any actions using this MatineeData.
	if(ActiveTrack->IsA(UInterpTrackEvent::StaticClass()))
	{
		UpdateMatineeActionConnectors();
	}

	InterpEdTrans->EndSpecial();

	// A track may have been deleted, so we'll need to update our track window scroll bar
	UpdateTrackWindowScrollBars();

	// Deselect current track and all keys as indices might be wrong.
	ClearKeySelection();
	SetActiveTrack(NULL, INDEX_NONE);

	Interp->RecaptureActorState();
}

/** Deletes the currently active group. */
void WxInterpEd::DeleteSelectedGroup()
{
	if(ActiveGroup==NULL)
	{
		return;
	}

	InterpEdTrans->BeginSpecial( *LocalizeUnrealEd("GroupDelete") );
	Interp->Modify();
	IData->Modify();

	// Mark InterpGroup and all InterpTracks as Modified.
	ActiveGroup->Modify();
	for(INT j=0; j<ActiveGroup->InterpTracks.Num(); j++)
	{
		ActiveGroup->InterpTracks(j)->Modify();

		// Remove from the Curve editor, if its there.
		IData->CurveEdSetup->RemoveCurve( ActiveGroup->InterpTracks(j) );
	}

	// Tell curve editor stuff might have changed.
	CurveEd->CurveChanged();

	// First, destroy any instances of this group.
	INT i=0;
	while( i<Interp->GroupInst.Num() )
	{
		UInterpGroupInst* GrInst = Interp->GroupInst(i);
		if(GrInst->Group == ActiveGroup)
		{
			// Mark InterpGroupInst and all InterpTrackInsts as Modified.
			GrInst->Modify();
			for(INT j=0; j<GrInst->TrackInst.Num(); j++)
			{
				GrInst->TrackInst(j)->Modify();
			}

			// Restore all state in this group before exiting
			GrInst->RestoreGroupActorState();

			// Clean up GroupInst
			GrInst->TermGroupInst(FALSE);
			// Don't actually delete the TrackInsts - but we do want to call TermTrackInst on them.

			// Remove from SeqAct_Interps list of GroupInsts
			Interp->GroupInst.Remove(i);
		}
		else
		{
			i++;
		}
	}

	// We're being deleted, so we need to unparent any child groups
	// @todo: Should we support optionally deleting all sub-groups when deleting the parent?
	for( INT CurGroupIndex = IData->InterpGroups.FindItemIndex( ActiveGroup ) + 1; CurGroupIndex < IData->InterpGroups.Num(); ++CurGroupIndex )
	{
		UInterpGroup* CurGroup = IData->InterpGroups( CurGroupIndex );
		if( CurGroup->bIsParented )
		{
			CurGroup->Modify();

			// Unparent this child
			CurGroup->bIsParented = FALSE;
		}
		else
		{
			// We've reached a root object, so we're done processing children.  Bail!
			break;
		}
	}

	// Then remove the group itself from the InterpData.
	IData->InterpGroups.RemoveItem(ActiveGroup);

	// Make sure the director track window is only visible if we have a director group!
	UpdateDirectorTrackWindowVisibility();

	// A group may have been deleted, so we'll need to update our track window scroll bar
	UpdateTrackWindowScrollBars();

	// Deselect everything.
	ClearKeySelection();

	// Also remove the variable connector that corresponded to this group in all SeqAct_Interps in the level.
	UpdateMatineeActionConnectors();	

	InterpEdTrans->EndSpecial();

	// Stop having the camera locked to this group if it currently is.
	if(CamViewGroup == ActiveGroup)
	{
		LockCamToGroup(NULL);
	}

	// Finally, deselect any groups.
	SetActiveTrack(NULL, INDEX_NONE);

	// Reimage actor world locations.  This must happen after the group was removed.
	Interp->RecaptureActorState();
}

/**
 * Duplicates the specified group
 *
 * @param GroupToDuplicate		Group we are going to duplicate.
 */
void WxInterpEd::DuplicateGroup(UInterpGroup* GroupToDuplicate)
{
	if(GroupToDuplicate==NULL)
	{
		return;
	}

	FName NewGroupName;

	// See if we are duplicating a director group.
	UBOOL bDirGroup = GroupToDuplicate->IsA(UInterpGroupDirector::StaticClass());

	// If we are a director group, make sure we don't have a director group yet in our interp data.
	if(bDirGroup)
	{
		 UInterpGroupDirector* DirGroup = IData->FindDirectorGroup();

		 if(DirGroup)
		 {
			appMsgf(AMT_OK, *LocalizeUnrealEd("UnableToPasteOnlyOneDirectorGroup"));
			return;
		 }
	}
	else
	{
		// Otherwise, pop up dialog to enter name.
		WxDlgGenericStringEntry dlg;
		const INT Result = dlg.ShowModal( TEXT("NewGroupName"), TEXT("NewGroupName"), TEXT("InterpGroup") );
		if( Result != wxID_OK )
		{
			return;
		}

		FString TempString = dlg.GetEnteredString();
		TempString = TempString.Replace(TEXT(" "),TEXT("_"));
		NewGroupName = FName( *TempString );
	}

	

	// Begin undo transaction
	InterpEdTrans->BeginSpecial( TEXT("NewGroup") );
	{
		Interp->Modify();
		IData->Modify();

		// Create new InterpGroup.
		UInterpGroup* NewGroup = NULL;

		NewGroup = (UInterpGroup*)UObject::StaticDuplicateObject( GroupToDuplicate, GroupToDuplicate, IData, TEXT("None"), RF_Transactional );

		if(!bDirGroup)
		{
			NewGroup->GroupName = NewGroupName;
		}
		IData->InterpGroups.AddItem(NewGroup);


		// All groups must have a unique name.
		NewGroup->EnsureUniqueName();

		// Randomly generate a group colour for the new group.
		NewGroup->GroupColor = FColor::MakeRandomColor();
		NewGroup->Modify();

		// Pasted groups are always unparented.  If we wanted to support pasting a group into a folder, we'd
		// need to be sure to insert the new group in the appropriate place in the group list.
		NewGroup->bIsParented = FALSE;

		// Create new InterpGroupInst
		UInterpGroupInst* NewGroupInst = NULL;

		if(bDirGroup)
		{
			NewGroupInst = ConstructObject<UInterpGroupInstDirector>( UInterpGroupInstDirector::StaticClass(), Interp, NAME_None, RF_Transactional );
		}
		else
		{
			NewGroupInst = ConstructObject<UInterpGroupInst>( UInterpGroupInst::StaticClass(), Interp, NAME_None, RF_Transactional );
		}

		// Initialise group instance, saving ref to actor it works on.
		NewGroupInst->InitGroupInst(NewGroup, NULL);

		const INT NewGroupInstIndex = Interp->GroupInst.AddItem(NewGroupInst);

		NewGroupInst->Modify();


		// If a director group, create a director track for it now.
		if(bDirGroup)
		{
			UInterpGroupDirector* DirGroup = Cast<UInterpGroupDirector>(NewGroup);
			check(DirGroup);

			// See if the director group has a director track yet, if not make one and make the corresponding track inst as well.
			UInterpTrackDirector* NewDirTrack = DirGroup->GetDirectorTrack();
			
			if(NewDirTrack==NULL)
			{
				NewDirTrack = ConstructObject<UInterpTrackDirector>( UInterpTrackDirector::StaticClass(), NewGroup, NAME_None, RF_Transactional );
				NewGroup->InterpTracks.AddItem(NewDirTrack);

				UInterpTrackInst* NewDirTrackInst = ConstructObject<UInterpTrackInstDirector>( UInterpTrackInstDirector::StaticClass(), NewGroupInst, NAME_None, RF_Transactional );
				NewGroupInst->TrackInst.AddItem(NewDirTrackInst);

				NewDirTrackInst->InitTrackInst(NewDirTrack);
				NewDirTrackInst->SaveActorState(NewDirTrack);

				// Save for undo then redo.
				NewDirTrackInst->Modify();
				NewDirTrack->Modify();
			}
		}
		else
		{	
			// Create a new variable connector on all Matinee's using this data.
			UpdateMatineeActionConnectors();

			// Find the newly created connector on this SeqAct_Interp. Should always have one now!
			const INT NewLinkIndex = Interp->FindConnectorIndex(NewGroup->GroupName.ToString(), LOC_VARIABLE );
			check(NewLinkIndex != INDEX_NONE);
			FSeqVarLink* NewLink = &(Interp->VariableLinks(NewLinkIndex));

			// Find the sequence that this SeqAct_Interp is in.
			USequence* Seq = Cast<USequence>( Interp->GetOuter() );
			check(Seq);
		}

		// Select the group we just duplicated
		SetActiveTrack(NewGroup, INDEX_NONE);
	}
	InterpEdTrans->EndSpecial();

	// Make sure the director track window is only visible if we have a director group!
	UpdateDirectorTrackWindowVisibility();

	// A new group may have been added (via duplication), so we'll need to update our scroll bar
	UpdateTrackWindowScrollBars();

	// Update graphics to show new group.
	InvalidateTrackWindowViewports();

	// If adding a camera- make sure its frustum colour is updated.
	UpdateCamColours();

	// Reimage actor world locations.  This must happen after the group was created.
	Interp->RecaptureActorState();
}

/** Call utility to crop the current key in the selected track. */
void WxInterpEd::CropAnimKey(UBOOL bCropBeginning)
{
	// Check we have a group and track selected
	if(!ActiveGroup || ActiveTrackIndex == INDEX_NONE)
	{
		return;
	}

	// Check its an AnimControlTrack
	UInterpTrackAnimControl* AnimTrack = Cast<UInterpTrackAnimControl>(ActiveGroup->InterpTracks(ActiveTrackIndex));
	if(!AnimTrack)
	{
		return;
	}

	InterpEdTrans->BeginSpecial( *LocalizeUnrealEd(TEXT("CropAnimationKey")) );
	{
		// Call crop utility.
		AnimTrack->Modify();
		AnimTrack->CropKeyAtPosition(Interp->Position, bCropBeginning);
	}
	InterpEdTrans->EndSpecial();
}


/** Go over all Matinee Action (SeqAct_Interp) making sure that their connectors (variables for group and outputs for event tracks) are up to date. */
void WxInterpEd::UpdateMatineeActionConnectors()
{
	USequence* RootSeq = Interp->GetRootSequence();
	check(RootSeq);
	RootSeq->UpdateInterpActionConnectors();
}


/** Jump the position of the interpolation to the current time, updating Actors. */
void WxInterpEd::SetInterpPosition( FLOAT NewPosition )
{
	UBOOL bTimeChanged = (NewPosition != Interp->Position);

	// Make sure particle replay tracks have up-to-date editor-only transient state
	UpdateParticleReplayTracks();

	// Move preview position in interpolation to where we want it, and update any properties
	Interp->UpdateInterp( NewPosition, TRUE, bTimeChanged );

	// When playing/scrubbing, we release the current keyframe from editing
	if(bTimeChanged)
	{
		bAdjustingKeyframe = FALSE;
	}

	// If we are locking the camera to a group, update it here
	UpdateCameraToGroup();

	// Set the camera frustum colours to show which is being viewed.
	UpdateCamColours();

	// Redraw viewport.
	InvalidateTrackWindowViewports();

	// Update the position of the marker in the curve view.
	CurveEd->SetPositionMarker( TRUE, Interp->Position, PosMarkerColor );
}



/** Make sure particle replay tracks have up-to-date editor-only transient state */
void WxInterpEd::UpdateParticleReplayTracks()
{
	for( INT CurGroupIndex = 0; CurGroupIndex < Interp->InterpData->InterpGroups.Num(); ++CurGroupIndex )
	{
		UInterpGroup* CurGroup = Interp->InterpData->InterpGroups( CurGroupIndex );
		if( CurGroup != NULL )
		{
			for( INT CurTrackIndex = 0; CurTrackIndex < CurGroup->InterpTracks.Num(); ++CurTrackIndex )
			{
				UInterpTrack* CurTrack = CurGroup->InterpTracks( CurTrackIndex );
				if( CurTrack != NULL )
				{
					UInterpTrackParticleReplay* ParticleReplayTrack = Cast< UInterpTrackParticleReplay >( CurTrack );
					if( ParticleReplayTrack != NULL )
					{
						// Copy time step
						ParticleReplayTrack->FixedTimeStep = SnapAmount;
					}
				}
			}
		}
	}
}



/** Refresh the Matinee position marker and viewport state */
void WxInterpEd::RefreshInterpPosition()
{
	SetInterpPosition( Interp->Position );
}



/** Ensure the curve editor is synchronised with the track editor. */
void WxInterpEd::SyncCurveEdView()
{
	CurveEd->StartIn = ViewStartTime;
	CurveEd->EndIn = ViewEndTime;
	CurveEd->CurveEdVC->Viewport->Invalidate();
}

/** Add the property being controlled by this track to the graph editor. */
void WxInterpEd::AddTrackToCurveEd(class UInterpGroup* InGroup, INT InTrackIndex)
{
	UInterpTrack* InterpTrack = InGroup->InterpTracks(InTrackIndex);

	FString CurveName = FString::Printf( TEXT("%s_%s"), *InGroup->GroupName.ToString(), *InterpTrack->TrackTitle);

	// Toggle whether this curve is edited in the Curve editor.
	if( IData->CurveEdSetup->ShowingCurve(InterpTrack) )
	{
		IData->CurveEdSetup->RemoveCurve(InterpTrack);
	}
	else
	{
		FColor CurveColor = InGroup->GroupColor;

		// If we are adding selected curve - highlight it.
		if(InGroup == ActiveGroup && InTrackIndex == ActiveTrackIndex)
		{
			CurveColor = SelectedCurveColor;
		}

		// Add track to the curve editor.
		UBOOL bColorTrack = FALSE;

		if(InterpTrack->IsA(UInterpTrackColorProp::StaticClass()))
		{
			bColorTrack = TRUE;
		}

		IData->CurveEdSetup->AddCurveToCurrentTab(InterpTrack, CurveName, CurveColor, bColorTrack, bColorTrack);
	}

	CurveEd->CurveChanged();
}


/** 
 *	Get the actor that the camera should currently be viewed through.
 *	We look here to see if the viewed group has a Director Track, and if so, return that Group.
 */
AActor* WxInterpEd::GetViewedActor()
{
	if( CamViewGroup != NULL )
	{
		UInterpGroupDirector* DirGroup = Cast<UInterpGroupDirector>(CamViewGroup);
		if(DirGroup)
		{
			return Interp->FindViewedActor();
		}
		else
		{
			UInterpGroupInst* GroupInst = Interp->FindFirstGroupInst(CamViewGroup);
			if( GroupInst != NULL )
			{
				return GroupInst->GroupActor;
			}
		}
	}

	return NULL;
}

/** Can input NULL to unlock camera from all group. */
void WxInterpEd::LockCamToGroup(class UInterpGroup* InGroup)
{
	// If different from current locked group - release current.
	if(CamViewGroup && (CamViewGroup != InGroup))
	{
		// Re-show the actor (if present)
		//UInterpGroupInst* GrInst = Interp->FindFirstGroupInst(ActiveGroup);
		//check(GrInst);
		//if(GrInst->GroupActor)
		//	GrInst->GroupActor->bHiddenEd = false;

		// Reset viewports (clear roll etc).
		for(INT i=0; i<GApp->EditorFrame->ViewportConfigData->GetViewportCount(); i++)
		{
			WxLevelViewportWindow* LevelVC = GApp->EditorFrame->ViewportConfigData->AccessViewport(i).ViewportWindow;
			if(LevelVC && LevelVC->ViewportType == LVT_Perspective && LevelVC->AllowMatineePreview() )
			{
				LevelVC->ViewRotation.Roll = 0;
				LevelVC->bConstrainAspectRatio = FALSE;
				LevelVC->bOverridePostProcessSettings = FALSE;
				LevelVC->bAdditivePostProcessSettings = FALSE;
				LevelVC->ViewFOV = GEditor->FOVAngle;
				LevelVC->bEnableFading = FALSE;
				LevelVC->bEnableColorScaling = FALSE;
			}
		}

		CamViewGroup = NULL;
	}

	// If non-null new group - switch to it now.
	if(InGroup)
	{
		// Hide the actor when viewing through it.
		//UInterpGroupInst* GrInst = Interp->FindFirstGroupInst(InGroup);
		//check(GrInst);
		//GrInst->GroupActor->bHiddenEd = true;

		CamViewGroup = InGroup;

		// Move camera to track now.
		UpdateCameraToGroup();
	}
}

/** Update the colours of any CameraActors we are manipulating to match their group colours, and indicate which is 'active'. */
void WxInterpEd::UpdateCamColours()
{
	AActor* ViewedActor = Interp->FindViewedActor();

	for(INT i=0; i<Interp->GroupInst.Num(); i++)
	{
		ACameraActor* Cam = Cast<ACameraActor>(Interp->GroupInst(i)->GroupActor);
		if(Cam && Cam->DrawFrustum)
		{
			if(Interp->GroupInst(i)->GroupActor == ViewedActor)
			{
				Cam->DrawFrustum->FrustumColor = ActiveCamColor;
			}
			else
			{
				Cam->DrawFrustum->FrustumColor = Interp->GroupInst(i)->Group->GroupColor;
			}
		}
	}
}

/** 
 *	If we are viewing through a particular group - move the camera to correspond. 
 */
void WxInterpEd::UpdateCameraToGroup()
{
	UBOOL bEnableColorScaling = false;
	FVector ColorScale(1.f,1.f,1.f);

	// If viewing through the director group, see if we have a fade track, and if so see how much fading we should do.
	FLOAT FadeAmount = 0.f;
	if(CamViewGroup)
	{
		UInterpGroupDirector* DirGroup = Cast<UInterpGroupDirector>(CamViewGroup);
		if(DirGroup)
		{
			UInterpTrackFade* FadeTrack = DirGroup->GetFadeTrack();
			if(FadeTrack && !FadeTrack->bDisableTrack)
			{
				FadeAmount = FadeTrack->GetFadeAmountAtTime(Interp->Position);
			}

			// Set TimeDilation in the LevelInfo based on what the Slomo track says (if there is one).
			UInterpTrackSlomo* SlomoTrack = DirGroup->GetSlomoTrack();
			if(SlomoTrack && !SlomoTrack->bDisableTrack)
			{
				GWorld->GetWorldInfo()->TimeDilation = SlomoTrack->GetSlomoFactorAtTime(Interp->Position);
			}

			UInterpTrackColorScale* ColorTrack = DirGroup->GetColorScaleTrack();
			if(ColorTrack && !ColorTrack->bDisableTrack)
			{
				bEnableColorScaling = true;
				ColorScale = ColorTrack->GetColorScaleAtTime(Interp->Position);
			}
		}
	}

	AActor* ViewedActor = GetViewedActor();
	if(ViewedActor)
	{
		ACameraActor* Cam = Cast<ACameraActor>(ViewedActor);

		// Move any perspective viewports to coincide with moved actor.
		for(INT i=0; i<GApp->EditorFrame->ViewportConfigData->GetViewportCount(); i++)
		{
			WxLevelViewportWindow* LevelVC = GApp->EditorFrame->ViewportConfigData->AccessViewport(i).ViewportWindow;
			if(LevelVC && LevelVC->ViewportType == LVT_Perspective && LevelVC->AllowMatineePreview() )
			{
				LevelVC->ViewLocation = ViewedActor->Location;
				LevelVC->ViewRotation = ViewedActor->Rotation;				

				LevelVC->FadeAmount = FadeAmount;
				LevelVC->bEnableFading = true;

				LevelVC->bEnableColorScaling = bEnableColorScaling;
				LevelVC->ColorScale = ColorScale;

				// If viewing through a camera - enforce aspect ratio and PP settings of camera.
				if(Cam)
				{
					LevelVC->bOverridePostProcessSettings = Cam->bCamOverridePostProcess;
					LevelVC->OverrideProcessSettings = Cam->CamOverridePostProcess;

					LevelVC->bConstrainAspectRatio = Cam->bConstrainAspectRatio;
					LevelVC->AspectRatio = Cam->AspectRatio;
					LevelVC->ViewFOV = Cam->FOVAngle;
				}
				else
				{
					LevelVC->bOverridePostProcessSettings = FALSE;
					LevelVC->bAdditivePostProcessSettings = FALSE;

					LevelVC->bConstrainAspectRatio = FALSE;
					LevelVC->ViewFOV = GEditor->FOVAngle;
				}
			}
		}
	}
	// If not bound to anything at this point of the cinematic - leave viewports at default.
	else
	{
		for(INT i=0; i<GApp->EditorFrame->ViewportConfigData->GetViewportCount(); i++)
		{
			WxLevelViewportWindow* LevelVC = GApp->EditorFrame->ViewportConfigData->AccessViewport(i).ViewportWindow;
			if(LevelVC && LevelVC->ViewportType == LVT_Perspective && LevelVC->AllowMatineePreview() )
			{
				LevelVC->bOverridePostProcessSettings = FALSE;

				LevelVC->bConstrainAspectRatio = false;
				LevelVC->ViewFOV = GEditor->FOVAngle;

				LevelVC->FadeAmount = FadeAmount;
				LevelVC->bEnableFading = true;
			}
		}
	}
}

// Notification from the EdMode that a perspective camera has moves. 
// If we are locking the camera to a particular actor - we update its location to match.
void WxInterpEd::CamMoved(const FVector& NewCamLocation, const FRotator& NewCamRotation)
{
	// If cam not locked to something, do nothing.
	AActor* ViewedActor = GetViewedActor();
	if(ViewedActor)
	{
		// Update actors location/rotation from camera
		ViewedActor->Location = NewCamLocation;
		ViewedActor->Rotation = NewCamRotation;

		ViewedActor->InvalidateLightingCache();
		ViewedActor->ForceUpdateComponents();

		// In case we were modifying a keyframe for this actor.
		ActorModified();
	}
}


void WxInterpEd::ActorModified()
{
	// We only see if we need to update a track if we have a keyframe selected.
	if(bAdjustingKeyframe)
	{
		check(Opt->SelectedKeys.Num() == 1);
    FInterpEdSelKey& SelKey = Opt->SelectedKeys(0);
  
    // Find the actor controlled by the selected group.
    UInterpGroupInst* GrInst = Interp->FindFirstGroupInst(SelKey.Group);
		if( GrInst == NULL || GrInst->GroupActor == NULL )
		{
			return;
		}
  
    // See if this is one of the actors that was just moved.
    UBOOL bTrackActorModified = FALSE;

			TArray<UObject*> NewObjects;
			for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
			{
				AActor* Actor = static_cast<AActor*>( *It );
				checkSlow( Actor->IsA(AActor::StaticClass()) );

				if ( Actor == GrInst->GroupActor )
		        {
			        bTrackActorModified = TRUE;
					break;
		        }
	        }
        
	        // If so, update the selected keyframe on the selected track to reflect its new position.
	        if(bTrackActorModified)
	        {
		        UInterpTrack* Track = SelKey.Group->InterpTracks(SelKey.TrackIndex);
        
		        InterpEdTrans->BeginSpecial( *LocalizeUnrealEd("UpdateKeyframe") );
		        Track->Modify();
        
		        UInterpTrackInst* TrInst = GrInst->TrackInst(SelKey.TrackIndex);
        
		        Track->UpdateKeyframe( SelKey.KeyIndex, TrInst);
        
		        InterpEdTrans->EndSpecial();
	        }
	}

	// This might have been a camera propety - update cameras.
	UpdateCameraToGroup();
}

void WxInterpEd::ActorSelectionChange()
{
	// Ignore this selection notification if desired.
	if(bIgnoreActorSelection)
	{
		return;
	}

	// Look at currently selected actors. We only want one.
	AActor* SingleActor = NULL;
	for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );
		if(!SingleActor)
		{
			SingleActor = Actor;
		}
		else
		{
			// If more than one thing selected - select no tracks.
			SetActiveTrack(NULL, INDEX_NONE);
			return;
		}
	}

	if(!SingleActor)
	{
		SetActiveTrack(NULL, INDEX_NONE);
		return;
	}

	UInterpGroupInst* GrInst = Interp->FindGroupInst(SingleActor);
	if(GrInst)
	{
		check(GrInst->Group);
		SetActiveTrack(GrInst->Group, INDEX_NONE);
		return;
	}

	SetActiveTrack(NULL, INDEX_NONE);
}

UBOOL WxInterpEd::ProcessKeyPress(FName Key, UBOOL bCtrlDown, UBOOL bAltDown)
{
	return false;
}



/**
 * Zooms the curve editor and track editor in or out by the specified amount
 *
 * @param ZoomAmount			Amount to zoom in or out
 * @param bZoomToTimeCursorPos	True if we should zoom to the time cursor position, otherwise mouse cursor position
 */
void WxInterpEd::ZoomView( FLOAT ZoomAmount, UBOOL bZoomToTimeCursorPos )
{
	// Proportion of interp we are currently viewing
	const FLOAT OldTimeRange = ViewEndTime - ViewStartTime;
	FLOAT CurrentZoomFactor = OldTimeRange / TrackViewSizeX;

	FLOAT NewZoomFactor = Clamp<FLOAT>(CurrentZoomFactor * ZoomAmount, 0.0003f, 1.0f);
	FLOAT NewTimeRange = NewZoomFactor * TrackViewSizeX;

	// zoom into scrub position
	if(bZoomToScrubPos)
	{
		FLOAT ViewMidTime = Interp->Position;
		ViewStartTime = ViewMidTime - 0.5*NewTimeRange;
		ViewEndTime = ViewMidTime + 0.5*NewTimeRange;
	}
	else
	{
		UBOOL bZoomedToCursorPos = FALSE;
		
		if( TrackWindow != NULL && IsMouseInWindow() )
		{
			// Figure out where the mouse cursor is over the Matinee track editor timeline
			wxPoint ScreenMousePos = wxGetMousePosition();
			wxPoint ClientMousePos =  TrackWindow->ScreenToClient( ScreenMousePos );
			INT ViewportClientAreaX = ClientMousePos.x;
			INT MouseXOverTimeline = ClientMousePos.x - LabelWidth;

  			if( MouseXOverTimeline >= 0 && MouseXOverTimeline < TrackViewSizeX )
			{
				// zoom into the mouse cursor's position over the view
				const FLOAT CursorPosInTime = ViewStartTime + ( MouseXOverTimeline / PixelsPerSec );
				const FLOAT CursorPosScalar = ( CursorPosInTime - ViewStartTime ) / OldTimeRange;

				ViewStartTime = CursorPosInTime - CursorPosScalar * NewTimeRange;
				ViewEndTime = CursorPosInTime + ( 1.0f - CursorPosScalar ) * NewTimeRange;

				bZoomedToCursorPos = TRUE;
			}
		}
		
		
		// We'll only zoom to the middle if we weren't already able to zoom to the cursor position.  Useful
		// if the mouse is outside of the window but the window still has focus for the zoom event
		if( !bZoomedToCursorPos )
		{
			// zoom into middle of view
			FLOAT ViewMidTime = ViewStartTime + 0.5f*(ViewEndTime - ViewStartTime);
			ViewStartTime = ViewMidTime - 0.5*NewTimeRange;
			ViewEndTime = ViewMidTime + 0.5*NewTimeRange;
		}
	}

	SyncCurveEdView();
}

void WxInterpEd::MoveActiveBy(INT MoveBy)
{
	if(!ActiveGroup)
	{
		return;
	}

	// We only support moving 1 unit in either direction
	check( Abs( MoveBy ) == 1 );
	
	InterpEdTrans->BeginSpecial( *LocalizeUnrealEd("MoveActiveTrackOrGroup") );

	// If no track selected, move group
	if(ActiveTrackIndex == INDEX_NONE)
	{
		INT ActiveGroupIndex = IData->InterpGroups.FindItemIndex(ActiveGroup);

		// Is this a root group or a child group?  We'll only allow navigation through groups within the current scope.
		const UBOOL bIsChildGroup = ActiveGroup->bIsParented;

		// If we're moving a child group, then don't allow it to move outside of it's current folder's sub-group list
		if( bIsChildGroup )
		{
			INT TargetGroupIndex = ActiveGroupIndex + MoveBy;

			if( TargetGroupIndex >= 0 && TargetGroupIndex < IData->InterpGroups.Num() )
			{
				UInterpGroup* GroupToCheck = IData->InterpGroups( TargetGroupIndex );
				if( !GroupToCheck->bIsParented )
				{
					// Uh oh, we've reached the end of our parent group's list.  We'll deny movement.
					TargetGroupIndex = ActiveGroupIndex;
				}
			}

			if(TargetGroupIndex != ActiveGroupIndex && TargetGroupIndex >= 0 && TargetGroupIndex < IData->InterpGroups.Num())
			{
				IData->Modify();

				UInterpGroup* TempGroup = IData->InterpGroups(TargetGroupIndex);
				IData->InterpGroups(TargetGroupIndex) = IData->InterpGroups(ActiveGroupIndex);
				IData->InterpGroups(ActiveGroupIndex) = TempGroup;
			}
		}
		else
		{
			// We're moving a root group.  This is a bit tricky.  Our (single level) 'heirarchy' of groups is really just
			// a flat list of elements with a bool that indicates whether the element is a child of the previous non-child
			// element, so we need to be careful to skip over all child groups when reordering things.

			// Also, we'll also skip over the director group if we find one, since those will always appear immutable to the
			// user in the GUI.  The director group draws at the top of the group list and never appears underneath
			// another group or track, so we don't want to consider it when rearranging groups through the UI.

			struct TopLevelGroupInfo
			{
				/** Index in original list */
				INT GroupIndex;

				/** Number of children */
				INT ChildCount;
			};


			// Digest information about the group list
			TArray< TopLevelGroupInfo > TopLevelGroups;
			INT ActiveGroupTLIndex = INDEX_NONE;
			{
				INT LastParentListIndex = INDEX_NONE;
				for( INT CurGroupIndex = 0; CurGroupIndex < IData->InterpGroups.Num(); ++CurGroupIndex )
				{
					UInterpGroup* CurGroup = IData->InterpGroups( CurGroupIndex );

					if( CurGroup->bIsParented )
					{
						// Add a new child to the last top level group
						check( LastParentListIndex != INDEX_NONE );
						++TopLevelGroups( LastParentListIndex ).ChildCount;
					}
					else
					{
						// A new top level group!
						TopLevelGroupInfo NewTopLevelGroup;
						NewTopLevelGroup.GroupIndex = CurGroupIndex;

						// Start at zero; we'll count these as we go along
						NewTopLevelGroup.ChildCount = 0;

						LastParentListIndex = TopLevelGroups.AddItem( NewTopLevelGroup );

						// If this is the active group, then keep track of that
						if( CurGroup == ActiveGroup )
						{
							ActiveGroupTLIndex = LastParentListIndex;
						}
					}
				}
			}

			// Make sure we found ourselves in the list
			check( ActiveGroupTLIndex != INDEX_NONE );



			// Determine our top-level list target
			INT TargetTLIndex = ActiveGroupTLIndex + MoveBy;
			if( TargetTLIndex >= 0 && TargetTLIndex < TopLevelGroups.Num() )
			{
				// Skip over director groups if we need to
				if( IData->InterpGroups( TopLevelGroups( TargetTLIndex ).GroupIndex )->IsA( UInterpGroupDirector::StaticClass() ) )
				{
					TargetTLIndex += MoveBy;
				}
			}

			// Make sure we're still in range
			if( TargetTLIndex >= 0 && TargetTLIndex < TopLevelGroups.Num() )
			{
				// Compute the list index that we'll be 'inserting before'
				INT InsertBeforeTLIndex = TargetTLIndex;
				if( MoveBy > 0 )
				{
					++InsertBeforeTLIndex;
				}

				// Compute our list destination
				INT TargetGroupIndex;
				if( InsertBeforeTLIndex < TopLevelGroups.Num() )
				{
					// Grab the top-level target group
					UInterpGroup* TLTargetGroup = IData->InterpGroups( TopLevelGroups( InsertBeforeTLIndex ).GroupIndex );

					// Setup 'insert' target group index
					TargetGroupIndex = TopLevelGroups( InsertBeforeTLIndex ).GroupIndex;
				}
				else
				{
					// We need to be at the very end of the list!
					TargetGroupIndex = IData->InterpGroups.Num();
				}


				// OK, time to move!
				const INT NumChildGroups = CountGroupFolderChildren( ActiveGroup );
				const INT NumGroupsToMove = NumChildGroups + 1;


				// We're about to modify stuff 
				IData->Modify();


				// Remove source groups from master list
				TArray< UInterpGroup* > GroupsToMove;
				for( INT GroupToMoveIndex = 0; GroupToMoveIndex < NumGroupsToMove; ++GroupToMoveIndex )
				{
					GroupsToMove.AddItem( IData->InterpGroups( ActiveGroupIndex ) );
					IData->InterpGroups.Remove( ActiveGroupIndex );

					// Adjust our target index for removed groups
					if( TargetGroupIndex >= ActiveGroupIndex )
					{
						--TargetGroupIndex;
					}
				};


				// Reinsert source groups at destination index
				for( INT GroupToMoveIndex = 0; GroupToMoveIndex < NumGroupsToMove; ++GroupToMoveIndex )
				{
					INT DestGroupIndex = TargetGroupIndex + GroupToMoveIndex;
					IData->InterpGroups.InsertItem( GroupsToMove( GroupToMoveIndex ), DestGroupIndex );
				};

			}
			else
			{
				// Out of range, we can't move any further
			}
		}
	}
	// If a track is selected, move it instead.
	else
	{
		// Move the track itself.
		INT TargetTrackIndex = ActiveTrackIndex + MoveBy;

		ActiveGroup->Modify();

		if(TargetTrackIndex >= 0 && TargetTrackIndex < ActiveGroup->InterpTracks.Num())
		{
			UInterpTrack* TempTrack = ActiveGroup->InterpTracks(TargetTrackIndex);
			ActiveGroup->InterpTracks(TargetTrackIndex) = ActiveGroup->InterpTracks(ActiveTrackIndex);
			ActiveGroup->InterpTracks(ActiveTrackIndex) = TempTrack;

			// Now move any track instances inside their group instance.
			for(INT i=0; i<Interp->GroupInst.Num(); i++)
			{
				UInterpGroupInst* GrInst = Interp->GroupInst(i);
				if(GrInst->Group == ActiveGroup)
				{
					check(GrInst->TrackInst.Num() == ActiveGroup->InterpTracks.Num());

					GrInst->Modify();

					UInterpTrackInst* TempTrInst = GrInst->TrackInst(TargetTrackIndex);
					GrInst->TrackInst(TargetTrackIndex) = GrInst->TrackInst(ActiveTrackIndex);
					GrInst->TrackInst(ActiveTrackIndex) = TempTrInst;
				}
			}

			// Update selection to keep same track selected.
			ActiveTrackIndex = TargetTrackIndex;

			// Selection stores keys by track index - safest to invalidate here.
			ClearKeySelection();
		}
	}

	InterpEdTrans->EndSpecial();

	// Dirty the track window viewports
	InvalidateTrackWindowViewports();
}

void WxInterpEd::MoveActiveUp()
{
	MoveActiveBy(-1);
}

void WxInterpEd::MoveActiveDown()
{
	MoveActiveBy(+1);
}

void WxInterpEd::InterpEdUndo()
{
	GEditor->Trans->Undo();
	UpdateMatineeActionConnectors();
	CurveEd->SetRegionMarker(true, IData->EdSectionStart, IData->EdSectionEnd, RegionFillColor);
	CurveEd->SetEndMarker(true, IData->InterpLength);
	bAdjustingKeyframe = FALSE;

	// Make sure the director track window is only visible if we have a director group!
	UpdateDirectorTrackWindowVisibility();

	// A new group may have been added (via duplication), so we'll need to update our scroll bar
	UpdateTrackWindowScrollBars();

	// Make sure that the viewports get updated after the Undo operation
	InvalidateTrackWindowViewports();
}

void WxInterpEd::InterpEdRedo()
{
	GEditor->Trans->Redo();
	UpdateMatineeActionConnectors();
	CurveEd->SetRegionMarker(true, IData->EdSectionStart, IData->EdSectionEnd, RegionFillColor);
	CurveEd->SetEndMarker(true, IData->InterpLength);
	bAdjustingKeyframe = FALSE;

	// Make sure the director track window is only visible if we have a director group!
	UpdateDirectorTrackWindowVisibility();

	// A new group may have been added (via duplication), so we'll need to update our scroll bar
	UpdateTrackWindowScrollBars();

	// Make sure that the viewports get updated after the Undo operation
	InvalidateTrackWindowViewports();
}

/*******************************************************************************
* InterpTrack methods used only in the editor.
*******************************************************************************/

// Common FName used just for storing name information while adding Keyframes to tracks.
static FName		KeyframeAddDataName = NAME_None;
static USoundCue	*KeyframeAddSoundCue = NULL;
static FName		TrackAddPropName = NAME_None;
static FName		AnimSlotName = NAME_None;
static FString		FaceFXGroupName = FString(TEXT(""));
static FString		FaceFXAnimName = FString(TEXT(""));


/**
 * Sets the global property name to use for newly created property tracks
 *
 * @param NewName The property name
 */
void WxInterpEd::SetTrackAddPropName( const FName NewName )
{
	TrackAddPropName = NewName;
}


IMPLEMENT_CLASS(UInterpTrackHelper);

/**
 * @return Returns the actor for the group's track if one exists, NULL otherwise.
 */
AActor* UInterpTrackHelper::GetGroupActor() const
{
	FEdModeInterpEdit* mode = (FEdModeInterpEdit*)GEditorModeTools().GetCurrentMode();
	check(mode != NULL);

	WxInterpEd	*InterpEd = mode->InterpEd;
	check(InterpEd != NULL);

	UInterpGroupInst* GrInst = InterpEd->Interp->FindFirstGroupInst(InterpEd->ActiveGroup);

	return ( GrInst != NULL ) ? GrInst->GroupActor : NULL;
}


/** Checks track-dependent criteria prior to adding a new track.
 * Responsible for any message-boxes or dialogs for selecting track-specific parameters.
 * Called on default object.
 *
 * @param Group The group that this track is being added to
 * @param	Trackdef Pointer to default object for this UInterpTrackClass.
 * @param	bDuplicatingTrack Whether we are duplicating this track or creating a new one from scratch.
 * @param bAllowPrompts When TRUE, we'll prompt for more information from the user with a dialog box if we need to
 * @return Returns true if this track can be created and false if some criteria is not met (i.e. A named property is already controlled for this group).
 */
UBOOL UInterpTrackAnimControlHelper::PreCreateTrack( UInterpGroup* Group, const UInterpTrack *TrackDef, UBOOL bDuplicatingTrack, UBOOL bAllowPrompts ) const
{
	// For AnimControl tracks - pop up a dialog to choose slot name.
	AnimSlotName = NAME_None;

	FEdModeInterpEdit* mode = (FEdModeInterpEdit*)GEditorModeTools().GetCurrentMode();
	check(mode != NULL);

	WxInterpEd	*InterpEd = mode->InterpEd;
	check(InterpEd != NULL);

	UInterpGroupInst* GrInst = InterpEd->Interp->FindFirstGroupInst(Group);
	check(GrInst);

	AActor* Actor = GrInst->GroupActor;
	if ( Actor != NULL )
	{
		// If this is the first AnimControlTrack, then init anim control now.
		// We need that before calling GetAnimControlSlotDesc
		if( !Group->HasAnimControlTrack() )
		{
			Actor->PreviewBeginAnimControl(Group->GroupAnimSets);
		}

		if( bAllowPrompts )
		{
			TArray<FAnimSlotDesc> SlotDescs;
			Actor->GetAnimControlSlotDesc(SlotDescs);

			// If we get no information - just allow it to be created with empty slot.
			if( SlotDescs.Num() == 0 )
			{
				return TRUE;
			}

			// Build combo to let you pick a slot. Don't put any names in that have already used all their channels. */

			TArray<FString> SlotStrings;
			for(INT i=0; i<SlotDescs.Num(); i++)
			{
				INT ChannelsUsed = GrInst->Group->GetAnimTracksUsingSlot( SlotDescs(i).SlotName );
				if(ChannelsUsed < SlotDescs(i).NumChannels)
				{
					SlotStrings.AddItem(*SlotDescs(i).SlotName.ToString());
				}
			}

			// If no slots free - we fail to create track.
			if(SlotStrings.Num() == 0)
			{
				appMsgf( AMT_OK, *LocalizeUnrealEd("Error_NoAnimChannelsLeft") );
				return FALSE;
			}

			WxDlgGenericComboEntry dlg;
			if( dlg.ShowModal( TEXT("ChooseAnimSlot"), TEXT("SlotName"), SlotStrings, 0, TRUE ) == wxID_OK )
			{
				AnimSlotName = FName( *dlg.GetSelectedString() );
				if ( AnimSlotName != NAME_None )
				{
					return TRUE;
				}
			}
		}
		else
		{
			// Prompts aren't allowed, so just succeed with defaults
			return TRUE;
		}
	}

	return FALSE;
}

void  UInterpTrackAnimControlHelper::PostCreateTrack( UInterpTrack *Track, UBOOL bDuplicatingTrack, INT TrackIndex ) const
{
	UInterpTrackAnimControl* AnimTrack = CastChecked<UInterpTrackAnimControl>(Track);
	AnimTrack->SlotName = AnimSlotName;

	// When you change the SlotName, change the TrackTitle to reflect that.
	UInterpTrackAnimControl* DefAnimTrack = CastChecked<UInterpTrackAnimControl>(AnimTrack->GetClass()->GetDefaultObject());
	FString DefaultTrackTitle = DefAnimTrack->TrackTitle;

	if(AnimTrack->SlotName == NAME_None)
	{
		AnimTrack->TrackTitle = DefaultTrackTitle;
	}
	else
	{
		AnimTrack->TrackTitle = FString::Printf( TEXT("%s:%s"), *DefaultTrackTitle, *AnimTrack->SlotName.ToString() );
	}
}


/** Checks track-dependent criteria prior to adding a new keyframe.
* Responsible for any message-boxes or dialogs for selecting key-specific parameters.
* Optionally creates/references a key-specific data object to be used in PostCreateKeyframe.
*
* @param	Track	Pointer to the currently selected track.
* @param	KeyTime	The time that this Key becomes active.
* @return	Returns true if this key can be created and false if some criteria is not met (i.e. No related item selected in browser).
*/
UBOOL UInterpTrackAnimControlHelper::PreCreateKeyframe( UInterpTrack *Track, FLOAT fTime ) const
{
	KeyframeAddDataName = NAME_None;
	UInterpTrackAnimControl	*AnimTrack = CastChecked<UInterpTrackAnimControl>(Track);
	UInterpGroup* Group = CastChecked<UInterpGroup>(Track->GetOuter());

	if ( Group->GroupAnimSets.Num() > 0 )
	{
		// Make array of AnimSequence names.
		TArray<FString> AnimNames;
		for ( INT i = 0; i < Group->GroupAnimSets.Num(); i++ )
		{
			UAnimSet *Set = Group->GroupAnimSets(i);
			if ( Set )
			{
				for ( INT j = 0; j < Set->Sequences.Num(); j++ )
				{
					AnimNames.AddUniqueItem(Set->Sequences(j)->SequenceName.ToString());
				}
			}
		}

		// If we couldn't find any AnimSequence names, don't bother continuing.
		if ( AnimNames.Num() > 0 )
		{
			// Show the dialog.
			WxDlgGenericComboEntry	dlg;
			const INT Result = dlg.ShowModal( TEXT("NewSeqKey"), TEXT("SeqKeyName"), AnimNames, 0, TRUE );
			if ( Result == wxID_OK )
			{
				KeyframeAddDataName = FName( *dlg.GetSelectedString() );
				return true;
			}
		}
		else
		{
			appMsgf( AMT_OK, *LocalizeUnrealEd("NoAnimSeqsFound") );
		}
	}

	return false;
}

/** Uses the key-specific data object from PreCreateKeyframe to initialize the newly added key.
* @param	Track		Pointer to the currently selected track.
* @param	KeyIndex	The index of the keyframe that as just added.  This is the index returned by AddKeyframe.
*/
void  UInterpTrackAnimControlHelper::PostCreateKeyframe( UInterpTrack *Track, INT KeyIndex ) const
{
	UInterpTrackAnimControl	*AnimTrack = CastChecked<UInterpTrackAnimControl>(Track);
	FAnimControlTrackKey& NewSeqKey = AnimTrack->AnimSeqs( KeyIndex );
	NewSeqKey.AnimSeqName = KeyframeAddDataName;
	KeyframeAddDataName = NAME_None;
}

IMPLEMENT_CLASS(UInterpTrackAnimControlHelper);

/** Checks track-dependent criteria prior to adding a new keyframe.
* Responsible for any message-boxes or dialogs for selecting key-specific parameters.
* Optionally creates/references a key-specific data object to be used in PostCreateKeyframe.
*
* @param	Track	Pointer to the currently selected track.
* @param	KeyTime	The time that this Key becomes active.
* @return	Returns true if this key can be created and false if some criteria is not met (i.e. No related item selected in browser).
*/
UBOOL UInterpTrackDirectorHelper::PreCreateKeyframe( UInterpTrack *Track, FLOAT KeyTime ) const
{
	// If adding a cut, bring up combo to let user choose group to cut to.
	KeyframeAddDataName = NAME_None;

	FEdModeInterpEdit* mode = (FEdModeInterpEdit*)GEditorModeTools().GetCurrentMode();
	check(mode != NULL);
	check(mode->InterpEd != NULL);

	if ( (mode != NULL) && (mode->InterpEd != NULL) )
	{
		// Make array of group names
		TArray<FString> GroupNames;
		for ( INT i = 0; i < mode->InterpEd->IData->InterpGroups.Num(); i++ )
		{
			// Skip folder groups
			if( !mode->InterpEd->IData->InterpGroups(i)->bIsFolder)
			{
				GroupNames.AddItem( *(mode->InterpEd->IData->InterpGroups(i)->GroupName.ToString()) );
			}
		}

		WxDlgGenericComboEntry	dlg;
		const INT	Result = dlg.ShowModal( TEXT("NewCut"), TEXT("CutToGroup"), GroupNames, 0, TRUE );
		if ( Result == wxID_OK )
		{
			KeyframeAddDataName = FName( *dlg.GetSelectedString() );
			return true;
		}
	}
	else
	{
	}

	return false;
}

/** Uses the key-specific data object from PreCreateKeyframe to initialize the newly added key.
*
* @param	Track	Pointer to the currently selected track.
* @param KeyIndex	The index of the keyframe that as just added.  This is the index returned by AddKeyframe.
*/
void  UInterpTrackDirectorHelper::PostCreateKeyframe( UInterpTrack *Track, INT KeyIndex ) const
{
	UInterpTrackDirector	*DirectorTrack = CastChecked<UInterpTrackDirector>(Track);
	FDirectorTrackCut& NewDirCut = DirectorTrack->CutTrack( KeyIndex );
	NewDirCut.TargetCamGroup = KeyframeAddDataName;
	KeyframeAddDataName = NAME_None;
}

IMPLEMENT_CLASS(UInterpTrackDirectorHelper);

/** Checks track-dependent criteria prior to adding a new keyframe.
* Responsible for any message-boxes or dialogs for selecting key-specific parameters.
* Optionally creates/references a key-specific data object to be used in PostCreateKeyframe.
*
* @param	Track	Pointer to the currently selected track.
* @param	KeyTime	The time that this Key becomes active.
* @return	Returns true if this key can be created and false if some criteria is not met (i.e. No related item selected in browser).
*/
UBOOL UInterpTrackEventHelper::PreCreateKeyframe( UInterpTrack *Track, FLOAT KeyTime ) const
{
	KeyframeAddDataName = NAME_None;

	// Prompt user for name of new event.
	WxDlgGenericStringEntry	dlg;
	const INT Result = dlg.ShowModal( TEXT("NewEventKey"), TEXT("NewEventName"), TEXT("Event0") );
	if( Result == wxID_OK )
	{
		FString TempString = dlg.GetEnteredString();
		TempString = TempString.Replace(TEXT(" "),TEXT("_"));
		KeyframeAddDataName = FName( *TempString );
		return true;
	}

	return false;
}

/** Uses the key-specific data object from PreCreateKeyframe to initialize the newly added key.
*
* @param	Track		Pointer to the currently selected track.
* @param	KeyIndex	The index of the keyframe that as just added.  This is the index returned by AddKeyframe.
*/
void  UInterpTrackEventHelper::PostCreateKeyframe( UInterpTrack *Track, INT KeyIndex ) const
{
	UInterpTrackEvent	*EventTrack = CastChecked<UInterpTrackEvent>(Track);
	FEventTrackKey& NewEventKey = EventTrack->EventTrack( KeyIndex );
	NewEventKey.EventName = KeyframeAddDataName;

	// Now ensure all Matinee actions (including 'Interp') are updated with a new connector to match the data.
	FEdModeInterpEdit* mode = (FEdModeInterpEdit*)GEditorModeTools().GetCurrentMode();
	check(mode != NULL);
	check(mode->InterpEd != NULL);

	if ( (mode != NULL) && (mode->InterpEd != NULL) )
	{
		mode->InterpEd->UpdateMatineeActionConnectors( );	
	}
	else
	{
	}

	KeyframeAddDataName = NAME_None;
}

IMPLEMENT_CLASS(UInterpTrackEventHelper);

/** Checks track-dependent criteria prior to adding a new keyframe.
* Responsible for any message-boxes or dialogs for selecting key-specific parameters.
* Optionally creates/references a key-specific data object to be used in PostCreateKeyframe.
*
* @param	Track	Pointer to the currently selected track.
* @param	KeyTime	The time that this Key becomes active.
* @return	Returns true if this key can be created and false if some criteria is not met (i.e. No related item selected in browser).
*/
UBOOL UInterpTrackSoundHelper::PreCreateKeyframe( UInterpTrack *Track, FLOAT KeyTime ) const
{
	KeyframeAddSoundCue = GEditor->GetSelectedObjects()->GetTop<USoundCue>();
	if ( KeyframeAddSoundCue )
	{
		return true;
	}

	appMsgf( AMT_OK, *LocalizeUnrealEd("NoSoundCueSelected") );
	return false;
}

/** Uses the key-specific data object from PreCreateKeyframe to initialize the newly added key.
*
* @param	Track		Pointer to the currently selected track.
* @param	KeyIndex	The index of the keyframe that as just added.  This is the index returned by AddKeyframe.
*/
void  UInterpTrackSoundHelper::PostCreateKeyframe( UInterpTrack *Track, INT KeyIndex ) const
{
	UInterpTrackSound	*SoundTrack = CastChecked<UInterpTrackSound>(Track);

	// Assign the chosen SoundCue to the new key.
	FSoundTrackKey& NewSoundKey = SoundTrack->Sounds( KeyIndex );
	NewSoundKey.Sound = KeyframeAddSoundCue;
	KeyframeAddSoundCue = NULL;
}

IMPLEMENT_CLASS(UInterpTrackSoundHelper);

/** Checks track-dependent criteria prior to adding a new track.
 * Responsible for any message-boxes or dialogs for selecting track-specific parameters.
 * Called on default object.
 *
 * @param Group The group that this track is being added to
 * @param	Trackdef Pointer to default object for this UInterpTrackClass.
 * @param	bDuplicatingTrack Whether we are duplicating this track or creating a new one from scratch.
 * @param bAllowPrompts When TRUE, we'll prompt for more information from the user with a dialog box if we need to
 * @return Returns true if this track can be created and false if some criteria is not met (i.e. A named property is already controlled for this group).
 */
UBOOL UInterpTrackFloatPropHelper::PreCreateTrack( UInterpGroup* Group, const UInterpTrack *TrackDef, UBOOL bDuplicatingTrack, UBOOL bAllowPrompts ) const
{
	if( bAllowPrompts && bDuplicatingTrack == FALSE )
	{
		// For Property tracks - pop up a dialog to choose property name.
		TrackAddPropName = NAME_None;

		FEdModeInterpEdit* mode = (FEdModeInterpEdit*)GEditorModeTools().GetCurrentMode();
		check(mode != NULL);

		WxInterpEd	*InterpEd = mode->InterpEd;
		check(InterpEd != NULL);

		UInterpGroupInst* GrInst = InterpEd->Interp->FindFirstGroupInst(Group);
		check(GrInst);

		AActor* Actor = GrInst->GroupActor;
		if ( Actor != NULL )
		{
			TArray<FName> PropNames;
			Actor->GetInterpFloatPropertyNames(PropNames);

			TArray<FString> PropStrings;
			PropStrings.AddZeroed( PropNames.Num() );
			for(INT i=0; i<PropNames.Num(); i++)
			{
				PropStrings(i) = PropNames(i).ToString();
			}

			WxDlgGenericComboEntry dlg;
			if( dlg.ShowModal( TEXT("ChooseProperty"), TEXT("PropertyName"), PropStrings, 0, TRUE ) == wxID_OK )
			{
				TrackAddPropName = FName( *dlg.GetSelectedString() );
				if ( TrackAddPropName != NAME_None )
				{
					// Check we don't already have a track controlling this property.
					for(INT i=0; i<Group->InterpTracks.Num(); i++)
					{
						UInterpTrackFloatProp* TestFloatTrack = Cast<UInterpTrackFloatProp>( Group->InterpTracks(i) );
						if(TestFloatTrack && TestFloatTrack->PropertyName == TrackAddPropName)
						{
							appMsgf(AMT_OK, *LocalizeUnrealEd("Error_PropertyAlreadyControlled"));
							return FALSE;
						}

						UInterpTrackVectorProp* TestVectorTrack = Cast<UInterpTrackVectorProp>( Group->InterpTracks(i) );
						if(TestVectorTrack && TestVectorTrack->PropertyName == TrackAddPropName)
						{
							appMsgf(AMT_OK, *LocalizeUnrealEd("Error_VectorPropertyAlreadyControlled"));
							return FALSE;
						}
					}

					return TRUE;
				}
			}
		}

		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/** Uses the track-specific data object from PreCreateTrack to initialize the newly added Track.
 * @param Track				Pointer to the track that was just created.
 * @param bDuplicatingTrack	Whether we are duplicating this track or creating a new one from scratch.
 * @param TrackIndex			The index of the Track that as just added.  This is the index returned by InterpTracks.AddItem.
 */
void  UInterpTrackFloatPropHelper::PostCreateTrack( UInterpTrack *Track, UBOOL bDuplicatingTrack, INT TrackIndex ) const
{
	if(bDuplicatingTrack == FALSE)
	{
		UInterpTrackFloatProp	*PropTrack = CastChecked<UInterpTrackFloatProp>(Track);

		// Set track title to property name (cut off component name if there is one).
		FString PropString = TrackAddPropName.ToString();
		INT PeriodPos = PropString.InStr(TEXT("."));
		if(PeriodPos != INDEX_NONE)
		{
			PropString = PropString.Mid(PeriodPos+1);
		}

		PropTrack->PropertyName = TrackAddPropName;
		PropTrack->TrackTitle = *PropString;

		TrackAddPropName = NAME_None;
	}
}

IMPLEMENT_CLASS(UInterpTrackFloatPropHelper);

//////////////////////////////////////////////////////////////////////////
// UInterpTrackToggleHelper
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UInterpTrackToggleHelper);

/** 
 * Checks track-dependent criteria prior to adding a new keyframe.
 * Responsible for any message-boxes or dialogs for selecting key-specific parameters.
 * Optionally creates/references a key-specific data object to be used in PostCreateKeyframe.
 *
 * @param Track		Pointer to the currently selected track.
 * @param KeyTime	The time that this Key becomes active.
 * @return			Returns true if this key can be created and false if some 
 *					criteria is not met (i.e. No related item selected in browser).
 */
UBOOL UInterpTrackToggleHelper::PreCreateKeyframe( UInterpTrack *Track, FLOAT KeyTime ) const
{
	UBOOL bResult = FALSE;

	FEdModeInterpEdit* mode = (FEdModeInterpEdit*)GEditorModeTools().GetCurrentMode();
	check(mode != NULL);

	WxInterpEd	*InterpEd = mode->InterpEd;
	check(InterpEd != NULL);

	TArray<FString> PropStrings;
	PropStrings.AddZeroed( 3 );
	PropStrings(0) = TEXT("Trigger");
	PropStrings(1) = TEXT("On");
	PropStrings(2) = TEXT("Off");

	WxDlgGenericComboEntry dlg;
	if( dlg.ShowModal( TEXT("ChooseToggleAction"), TEXT("ToggleAction"), PropStrings, 0, TRUE ) == wxID_OK )
	{
		KeyframeAddDataName = FName(*dlg.GetSelectedString());
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Uses the key-specific data object from PreCreateKeyframe to initialize the newly added key.
 *
 * @param Track		Pointer to the currently selected track.
 * @param KeyIndex	The index of the keyframe that as just added.  This is the index returned by AddKeyframe.
 */
void  UInterpTrackToggleHelper::PostCreateKeyframe( UInterpTrack *Track, INT KeyIndex ) const
{
	UInterpTrackToggle* ToggleTrack = CastChecked<UInterpTrackToggle>(Track);

	FToggleTrackKey& NewToggleKey = ToggleTrack->ToggleTrack(KeyIndex);
	if (KeyframeAddDataName == FName(TEXT("On")))
	{
		NewToggleKey.ToggleAction = ETTA_On;
	}
	else
	if (KeyframeAddDataName == FName(TEXT("Trigger")))
	{
		NewToggleKey.ToggleAction = ETTA_Trigger;
	}
	else
	{
		NewToggleKey.ToggleAction = ETTA_Off;
	}

	KeyframeAddDataName = NAME_None;
}

//////////////////////////////////////////////////////////////////////////
// UInterpTrackVectorPropHelper
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UInterpTrackVectorPropHelper);

UBOOL UInterpTrackVectorPropHelper::ChooseProperty(TArray<FName> &PropNames) const
{
	UBOOL bResult = FALSE;

	FEdModeInterpEdit* mode = (FEdModeInterpEdit*)GEditorModeTools().GetCurrentMode();
	check(mode != NULL);

	WxInterpEd	*InterpEd = mode->InterpEd;
	check(InterpEd != NULL);

	TArray<FString> PropStrings;
	PropStrings.AddZeroed( PropNames.Num() );
	for(INT i=0; i<PropNames.Num(); i++)
	{
		PropStrings(i) = PropNames(i).ToString();
	}

	WxDlgGenericComboEntry dlg;
	if( dlg.ShowModal( TEXT("ChooseProperty"), TEXT("PropertyName"), PropStrings, 0, TRUE ) == wxID_OK )
	{
		TrackAddPropName = FName( *dlg.GetSelectedString() );
		if ( TrackAddPropName != NAME_None )
		{
			bResult = TRUE;

			// Check we don't already have a track controlling this property.
			for(INT i=0; i<InterpEd->ActiveGroup->InterpTracks.Num(); i++)
			{
				UInterpTrackFloatProp* TestFloatTrack = Cast<UInterpTrackFloatProp>( InterpEd->ActiveGroup->InterpTracks(i) );
				if(TestFloatTrack && TestFloatTrack->PropertyName == TrackAddPropName)
				{
					appMsgf(AMT_OK, *LocalizeUnrealEd("Error_PropertyAlreadyControlled"));
					bResult = FALSE;
				}

				UInterpTrackVectorProp* TestVectorTrack = Cast<UInterpTrackVectorProp>( InterpEd->ActiveGroup->InterpTracks(i) );
				if(TestVectorTrack && TestVectorTrack->PropertyName == TrackAddPropName)
				{
					appMsgf(AMT_OK, *LocalizeUnrealEd("Error_VectorPropertyAlreadyControlled"));
					bResult = FALSE;
				}
			}
		}
	}

	return bResult;
}

/** Checks track-dependent criteria prior to adding a new track.
 * Responsible for any message-boxes or dialogs for selecting track-specific parameters.
 * Called on default object.
 *
 * @param Group The group that this track is being added to
 * @param	Trackdef Pointer to default object for this UInterpTrackClass.
 * @param	bDuplicatingTrack Whether we are duplicating this track or creating a new one from scratch.
 * @param bAllowPrompts When TRUE, we'll prompt for more information from the user with a dialog box if we need to
 * @return Returns true if this track can be created and false if some criteria is not met (i.e. A named property is already controlled for this group).
 */
UBOOL UInterpTrackVectorPropHelper::PreCreateTrack( UInterpGroup* Group, const UInterpTrack *TrackDef, UBOOL bDuplicatingTrack, UBOOL bAllowPrompts ) const
{
	UBOOL bResult = TRUE;

	if( bAllowPrompts && bDuplicatingTrack == FALSE )
	{
		bResult = FALSE;

		// For Property tracks - pop up a dialog to choose property name.
		TrackAddPropName = NAME_None;

		FEdModeInterpEdit* mode = (FEdModeInterpEdit*)GEditorModeTools().GetCurrentMode();
		check(mode != NULL);

		WxInterpEd	*InterpEd = mode->InterpEd;
		check(InterpEd != NULL);

		UInterpGroupInst* GrInst = InterpEd->Interp->FindFirstGroupInst(Group);
		check(GrInst);

		AActor* Actor = GrInst->GroupActor;
		if ( Actor != NULL )
		{
			TArray<FName> PropNames;
			Actor->GetInterpVectorPropertyNames(PropNames);
			bResult = ChooseProperty(PropNames);
		}
	}

	return bResult;
}

/** Uses the track-specific data object from PreCreateTrack to initialize the newly added Track.
 * @param Track				Pointer to the track that was just created.
 * @param bDuplicatingTrack	Whether we are duplicating this track or creating a new one from scratch.
 * @param TrackIndex			The index of the Track that as just added.  This is the index returned by InterpTracks.AddItem.
 */
void  UInterpTrackVectorPropHelper::PostCreateTrack( UInterpTrack *Track, UBOOL bDuplicatingTrack, INT TrackIndex ) const
{
	if(bDuplicatingTrack == FALSE)
	{
		UInterpTrackVectorProp	*PropTrack = CastChecked<UInterpTrackVectorProp>(Track);

		// Set track title to property name (cut off component name if there is one).
		FString PropString = TrackAddPropName.ToString();
		INT PeriodPos = PropString.InStr(TEXT("."));
		if(PeriodPos != INDEX_NONE)
		{
			PropString = PropString.Mid(PeriodPos+1);
		}

		PropTrack->PropertyName = TrackAddPropName;
		PropTrack->TrackTitle = *PropString;
		
		TrackAddPropName = NAME_None;
	}
}

//////////////////////////////////////////////////////////////////////////
// UInterpTrackColorPropHelper
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UInterpTrackColorPropHelper);

/** Checks track-dependent criteria prior to adding a new track.
 * Responsible for any message-boxes or dialogs for selecting track-specific parameters.
 * Called on default object.
 *
 * @param Group The group that this track is being added to
 * @param	Trackdef Pointer to default object for this UInterpTrackClass.
 * @param	bDuplicatingTrack Whether we are duplicating this track or creating a new one from scratch.
 * @param bAllowPrompts When TRUE, we'll prompt for more information from the user with a dialog box if we need to
 * @return Returns true if this track can be created and false if some criteria is not met (i.e. A named property is already controlled for this group).
 */
UBOOL UInterpTrackColorPropHelper::PreCreateTrack( UInterpGroup* Group, const UInterpTrack *TrackDef, UBOOL bDuplicatingTrack, UBOOL bAllowPrompts ) const
{
	UBOOL bResult = TRUE;

	if( bAllowPrompts && bDuplicatingTrack == FALSE )
	{
		bResult = FALSE;

		// For Property tracks - pop up a dialog to choose property name.
		TrackAddPropName = NAME_None;

		FEdModeInterpEdit* mode = (FEdModeInterpEdit*)GEditorModeTools().GetCurrentMode();
		check(mode != NULL);

		WxInterpEd	*InterpEd = mode->InterpEd;
		check(InterpEd != NULL);

		UInterpGroupInst* GrInst = InterpEd->Interp->FindFirstGroupInst(Group);
		check(GrInst);

		AActor* Actor = GrInst->GroupActor;
		if ( Actor != NULL )
		{
			TArray<FName> PropNames;
			Actor->GetInterpColorPropertyNames(PropNames);
			bResult = ChooseProperty(PropNames);
		}
	}

	return bResult;
}

/** Uses the track-specific data object from PreCreateTrack to initialize the newly added Track.
 * @param Track				Pointer to the track that was just created.
 * @param bDuplicatingTrack	Whether we are duplicating this track or creating a new one from scratch.
 * @param TrackIndex			The index of the Track that as just added.  This is the index returned by InterpTracks.AddItem.
 */
void  UInterpTrackColorPropHelper::PostCreateTrack( UInterpTrack *Track, UBOOL bDuplicatingTrack, INT TrackIndex ) const
{
	if(bDuplicatingTrack == FALSE)
	{
		UInterpTrackColorProp	*PropTrack = CastChecked<UInterpTrackColorProp>(Track);

		// Set track title to property name (cut off component name if there is one).
		FString PropString = TrackAddPropName.ToString();
		INT PeriodPos = PropString.InStr(TEXT("."));
		if(PeriodPos != INDEX_NONE)
		{
			PropString = PropString.Mid(PeriodPos+1);
		}

		PropTrack->PropertyName = TrackAddPropName;
		PropTrack->TrackTitle = *PropString;
		
		TrackAddPropName = NAME_None;
	}
}

//////////////////////////////////////////////////////////////////////////
// UInterpTrackFaceFXHelper
//////////////////////////////////////////////////////////////////////////

/** Offer choice of FaceFX sequences. */
UBOOL UInterpTrackFaceFXHelper::PreCreateKeyframe( UInterpTrack *Track, FLOAT fTime ) const
{
	FaceFXGroupName = FString(TEXT(""));
	FaceFXAnimName = FString(TEXT(""));

	// Get the Actor for the active Group - we need this to get a list of FaceFX animations to choose from.
	FEdModeInterpEdit* mode = (FEdModeInterpEdit*)GEditorModeTools().GetCurrentMode();
	check(mode != NULL);

	WxInterpEd	*InterpEd = mode->InterpEd;
	check(InterpEd != NULL);

	UInterpGroupInst* GrInst = InterpEd->Interp->FindFirstGroupInst(InterpEd->ActiveGroup);
	check(GrInst);

	AActor* Actor = GrInst->GroupActor;

	// If no Actor, warn and fail
	if(!Actor)
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Error_NoActorForFaceFXTrack") );
		return FALSE;
	}

	UInterpTrackFaceFX* FaceFXTrack = CastChecked<UInterpTrackFaceFX>(Track);
	if(!FaceFXTrack->CachedActorFXAsset)
	{
		//appMsgf( AMT_OK, *LocalizeUnrealEd("Error_NoActorForFaceFXTrack") ); // @todo - warning here
		return FALSE;
	}

	// Get array of sequence names. Will be in form 'GroupName.SequenceName'
	TArray<FString> SeqNames;
	FaceFXTrack->CachedActorFXAsset->GetSequenceNames(TRUE, SeqNames);

	// Get from each mounted AnimSet.
	for(INT i=0; i<FaceFXTrack->FaceFXAnimSets.Num(); i++)
	{
		UFaceFXAnimSet* Set = FaceFXTrack->FaceFXAnimSets(i);
		if(Set)
		{
			Set->GetSequenceNames(SeqNames);
		}
	}

	// If we got none, warn and fail.
	if(SeqNames.Num() == 0)
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Error_NoFaceFXSequencesFound") );
		return FALSE;
	}

	// Show a dialog to let user pick sequence.
	WxDlgGenericComboEntry dlg;
	const INT Result = dlg.ShowModal( TEXT("SelectFaceFXAnim"), TEXT("FaceFXAnim"), SeqNames, 0, TRUE );
	if(Result == wxID_OK)
	{
		// Get full name
		FString SelectedFullName = dlg.GetSelectedString();

		// Split it on the dot
		FString GroupName, SeqName;
		if( SelectedFullName.Split(TEXT("."), &GroupName, &SeqName) )
		{
			FaceFXGroupName = GroupName;
			FaceFXAnimName = SeqName;

			return TRUE;
		}
	}

	return FALSE;
}

/** Set the group/sequence name we chose in the newly created key. */
void  UInterpTrackFaceFXHelper::PostCreateKeyframe( UInterpTrack *Track, INT KeyIndex ) const
{
	UInterpTrackFaceFX* FaceFXTrack = CastChecked<UInterpTrackFaceFX>(Track);
	FFaceFXTrackKey& NewSeqKey = FaceFXTrack->FaceFXSeqs( KeyIndex );
	NewSeqKey.FaceFXGroupName = FaceFXGroupName;
	NewSeqKey.FaceFXSeqName = FaceFXAnimName;
}

IMPLEMENT_CLASS(UInterpTrackFaceFXHelper);



//////////////////////////////////////////////////////////////////////////
// UInterpTrackVisibilityHelper
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UInterpTrackVisibilityHelper);

/** 
 * Checks track-dependent criteria prior to adding a new keyframe.
 * Responsible for any message-boxes or dialogs for selecting key-specific parameters.
 * Optionally creates/references a key-specific data object to be used in PostCreateKeyframe.
 *
 * @param Track		Pointer to the currently selected track.
 * @param KeyTime	The time that this Key becomes active.
 * @return			Returns true if this key can be created and false if some 
 *					criteria is not met (i.e. No related item selected in browser).
 */
UBOOL UInterpTrackVisibilityHelper::PreCreateKeyframe( UInterpTrack *Track, FLOAT KeyTime ) const
{
	UBOOL bResult = FALSE;

	FEdModeInterpEdit* mode = (FEdModeInterpEdit*)GEditorModeTools().GetCurrentMode();
	check(mode != NULL);

	WxInterpEd	*InterpEd = mode->InterpEd;
	check(InterpEd != NULL);

	TArray<FString> PropStrings;
	PropStrings.AddZeroed( 3 );
	PropStrings(0) = TEXT("Show");
	PropStrings(1) = TEXT("Hide");
	PropStrings(2) = TEXT("Toggle");

	WxDlgGenericComboEntry dlg;
	if( dlg.ShowModal( TEXT("InterpEd_VisibilityActionDialogTitle"), TEXT("InterpEd_VisibilityActionDialogText"), PropStrings, 0, TRUE ) == wxID_OK )
	{
		KeyframeAddDataName = FName(*dlg.GetSelectedString());
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Uses the key-specific data object from PreCreateKeyframe to initialize the newly added key.
 *
 * @param Track		Pointer to the currently selected track.
 * @param KeyIndex	The index of the keyframe that as just added.  This is the index returned by AddKeyframe.
 */
void  UInterpTrackVisibilityHelper::PostCreateKeyframe( UInterpTrack *Track, INT KeyIndex ) const
{
	UInterpTrackVisibility* VisibilityTrack = CastChecked<UInterpTrackVisibility>(Track);

	FVisibilityTrackKey& NewVisibilityKey = VisibilityTrack->VisibilityTrack(KeyIndex);

	if (KeyframeAddDataName == FName(TEXT("Show")))
	{
		NewVisibilityKey.Action = EVTA_Show;
	}
	else
	if (KeyframeAddDataName == FName(TEXT("Toggle")))
	{
		NewVisibilityKey.Action = EVTA_Toggle;
	}
	else	// "Hide"
	{
		NewVisibilityKey.Action = EVTA_Hide;
	}


	// Default to Always firing this event.  The user can change it later by right clicking on the
	// track keys in the editor.
	NewVisibilityKey.ActiveCondition = EVTC_Always;

	KeyframeAddDataName = NAME_None;
}





//////////////////////////////////////////////////////////////////////////
// UInterpTrackParticleReplayHelper
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UInterpTrackParticleReplayHelper);

/** 
 * Checks track-dependent criteria prior to adding a new keyframe.
 * Responsible for any message-boxes or dialogs for selecting key-specific parameters.
 * Optionally creates/references a key-specific data object to be used in PostCreateKeyframe.
 *
 * @param Track		Pointer to the currently selected track.
 * @param KeyTime	The time that this Key becomes active.
 * @return			Returns true if this key can be created and false if some 
 *					criteria is not met (i.e. No related item selected in browser).
 */
UBOOL UInterpTrackParticleReplayHelper::PreCreateKeyframe( UInterpTrack *Track, FLOAT KeyTime ) const
{
	// We don't currently need to do anything here

	// @todo: It would be nice to pop up a dialog where the user can select a clip ID number
	//        from a list of replay clips that exist in emitter actor.

	return TRUE;
	UBOOL bResult = TRUE;
}

/**
 * Uses the key-specific data object from PreCreateKeyframe to initialize the newly added key.
 *
 * @param Track		Pointer to the currently selected track.
 * @param KeyIndex	The index of the keyframe that as just added.  This is the index returned by AddKeyframe.
 */
void  UInterpTrackParticleReplayHelper::PostCreateKeyframe( UInterpTrack *Track, INT KeyIndex ) const
{
	// We don't currently need to do anything here
}