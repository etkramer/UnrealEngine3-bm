/*=============================================================================
	InterpEditorMenus.cpp: Interpolation editing menus
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "EngineSequenceClasses.h"
#include "EngineInterpolationClasses.h"
#include "InterpEditor.h"
#include "UnLinkedObjDrawUtils.h"
#include "UnColladaExporter.h"
#include "UnColladaImporter.h"
#include "Properties.h"
#include "GenericBrowser.h"
#include "DlgGenericComboEntry.h"
#include "ScopedTransaction.h"

///// MENU CALLBACKS

// Add a new keyframe on the selected track 
void WxInterpEd::OnMenuAddKey( wxCommandEvent& In )
{
	AddKey();
}

void WxInterpEd::OnContextNewGroup( wxCommandEvent& In )
{
	// Find out if we want to make a 'Director' group.
	const UBOOL bIsNewFolder = ( In.GetId() == IDM_INTERP_NEW_FOLDER );
	const UBOOL bDirGroup = ( In.GetId() == IDM_INTERP_NEW_DIRECTOR_GROUP );
	const UBOOL bDuplicateGroup = ( In.GetId() == IDM_INTERP_GROUP_DUPLICATE );

	if(bDuplicateGroup && !ActiveGroup)
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("NoGroupToDuplicate") );
		return;
	}

	// This is temporary - need a unified way to associate tracks with components/actors etc... hmm..
	AActor* GroupActor = NULL;

	if(!bIsNewFolder && !bDirGroup && !bDuplicateGroup)
	{
		for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
		{
			AActor* Actor = static_cast<AActor*>( *It );
			checkSlow( Actor->IsA(AActor::StaticClass()) );

			GroupActor = Actor;
			break;
		}

		if(GroupActor)
		{
			// Check that the Outermost of both the Matinee action and the Actor are the same
			// We can't create a group for an Actor that is not in the same level as the sequence
			UObject* SequenceOutermost = Interp->GetOutermost();
			UObject* ActorOutermost = GroupActor->GetOutermost();
			if(ActorOutermost != SequenceOutermost)
			{
				appMsgf(AMT_OK, *LocalizeUnrealEd("Error_ActorNotInSequenceLevel"));
				return;
			}

			// Check we do not already have a group acting on this actor.
			for(INT i=0; i<Interp->GroupInst.Num(); i++)
			{
				if( GroupActor == Interp->GroupInst(i)->GroupActor )
				{
					appMsgf(AMT_OK, *LocalizeUnrealEd("Error_GroupAlreadyAssocWithActor"));
					return;
				}
			}
		}
	}

	FName NewGroupName;
	// If not a director group - ask for a name.
	if(!bDirGroup)
	{
		FString DialogName;
		FString DefaultNewGroupName;
		switch( In.GetId() )
		{
			case IDM_INTERP_NEW_CAMERA_GROUP:
				DialogName = TEXT( "NewGroupName" );
				DefaultNewGroupName = TEXT( "NewCameraGroup" );
				break;

			case IDM_INTERP_NEW_PARTICLE_GROUP:
				DialogName = TEXT( "NewGroupName" );
				DefaultNewGroupName = TEXT( "NewParticleGroup" );
				break;

			case IDM_INTERP_NEW_SKELETAL_MESH_GROUP:
				DialogName = TEXT( "NewGroupName" );
				DefaultNewGroupName = TEXT( "NewSkeletalMeshGroup" );
				break;

			case IDM_INTERP_NEW_FOLDER:
				DialogName = TEXT( "NewFolderName" );
				DefaultNewGroupName = TEXT( "NewFolder" );
				break;

			default:
				DialogName = TEXT( "NewGroupName" );
				DefaultNewGroupName = TEXT( "NewGroup" );
				break;
		}

		// Otherwise, pop up dialog to enter name.
		WxDlgGenericStringEntry dlg;
		const INT Result = dlg.ShowModal( *DialogName, *DialogName, *DefaultNewGroupName );
		if( Result != wxID_OK )
		{
			return;
		}

		FString TempString = dlg.GetEnteredString();
		TempString = TempString.Replace(TEXT(" "),TEXT("_"));
		NewGroupName = FName( *TempString );
	}

	// Begin undo transaction
	InterpEdTrans->BeginSpecial( *LocalizeUnrealEd("NewGroup") );
	Interp->Modify();
	IData->Modify();

	// Create new InterpGroup.
	UInterpGroup* NewGroup = NULL;
	if(bDirGroup)
	{
		NewGroup = ConstructObject<UInterpGroupDirector>( UInterpGroupDirector::StaticClass(), IData, NAME_None, RF_Transactional );
	}
	else if(bDuplicateGroup)
	{
		NewGroup = (UInterpGroup*)UObject::StaticDuplicateObject( ActiveGroup, ActiveGroup, IData, TEXT("None"), RF_Transactional );
		NewGroup->GroupName = NewGroupName;
	}
	else
	{
		NewGroup = ConstructObject<UInterpGroup>( UInterpGroup::StaticClass(), IData, NAME_None, RF_Transactional );
		NewGroup->GroupName = NewGroupName;
	}
	IData->InterpGroups.AddItem(NewGroup);


	// All groups must have a unique name.
	NewGroup->EnsureUniqueName();

	// Randomly generate a group colour for the new group.
	NewGroup->GroupColor = FColor::MakeRandomColor();

	// Set whether this is a folder or not
	NewGroup->bIsFolder = bIsNewFolder;


	NewGroup->Modify();

	// Folders don't need a group instance
	UInterpGroupInst* NewGroupInst = NULL;
	if( !bIsNewFolder )
	{
		// Create new InterpGroupInst
		if(bDirGroup)
		{
			NewGroupInst = ConstructObject<UInterpGroupInstDirector>( UInterpGroupInstDirector::StaticClass(), Interp, NAME_None, RF_Transactional );
			NewGroupInst->InitGroupInst(NewGroup, NULL);
		}
		else
		{
			NewGroupInst = ConstructObject<UInterpGroupInst>( UInterpGroupInst::StaticClass(), Interp, NAME_None, RF_Transactional );
			// Initialise group instance, saving ref to actor it works on.
			NewGroupInst->InitGroupInst(NewGroup, GroupActor);
		}

		const INT NewGroupInstIndex = Interp->GroupInst.AddItem(NewGroupInst);

		NewGroupInst->Modify();
	}


	// Don't need to save state here - no tracks!

	// If a director group, create a director track for it now.
	if(bDirGroup)
	{
		UInterpTrack* NewDirTrack = ConstructObject<UInterpTrackDirector>( UInterpTrackDirector::StaticClass(), NewGroup, NAME_None, RF_Transactional );
		NewGroup->InterpTracks.AddItem(NewDirTrack);

		UInterpTrackInst* NewDirTrackInst = ConstructObject<UInterpTrackInstDirector>( UInterpTrackInstDirector::StaticClass(), NewGroupInst, NAME_None, RF_Transactional );
		NewGroupInst->TrackInst.AddItem(NewDirTrackInst);

		NewDirTrackInst->InitTrackInst(NewDirTrack);
		NewDirTrackInst->SaveActorState(NewDirTrack);

		// Save for undo then redo.
		NewDirTrack->Modify();
		NewDirTrackInst->Modify();
	}
	// If regular track, create a new object variable connector, and variable containing selected actor if there is one.
	else
	{

		// Folder's don't need to be bound to actors
		if( !bIsNewFolder )
		{
			Interp->InitGroupActorForGroup(NewGroup, GroupActor);
		}

		// For Camera or Skeletal Mesh groups, add a Movement track
		if( In.GetId() == IDM_INTERP_NEW_CAMERA_GROUP ||
			  In.GetId() == IDM_INTERP_NEW_SKELETAL_MESH_GROUP )
		{
			INT NewTrackIndex = INDEX_NONE;
			AddTrackToGroup( NewGroup, UInterpTrackMove::StaticClass(), NULL, FALSE, NewTrackIndex );
		}

		// For Camera groups, add a Float Property track for FOV
		if( In.GetId() == IDM_INTERP_NEW_CAMERA_GROUP )
		{
			// Set the property name for the new track.  This is a global that will be used when setting everything up.
			SetTrackAddPropName( FName( TEXT( "FOVAngle" ) ) );

			INT NewTrackIndex = INDEX_NONE;
			UInterpTrack* NewTrack = AddTrackToGroup( NewGroup, UInterpTrackFloatProp::StaticClass(), NULL, FALSE, NewTrackIndex );
		}

		// For Skeletal Mesh groups, add an Anim track
		if( In.GetId() == IDM_INTERP_NEW_SKELETAL_MESH_GROUP )
		{
			INT NewTrackIndex = INDEX_NONE;
			AddTrackToGroup( NewGroup, UInterpTrackAnimControl::StaticClass(), NULL, FALSE, NewTrackIndex );
		}

		// For Particle groups, add a Toggle track
		if( In.GetId() == IDM_INTERP_NEW_PARTICLE_GROUP )
		{
			INT NewTrackIndex = INDEX_NONE;
			AddTrackToGroup( NewGroup, UInterpTrackToggle::StaticClass(), NULL, FALSE, NewTrackIndex );
		}
	}


	// If we have a custom filter tab currently selected, then add the new group to that filter tab
	{
		UInterpFilter_Custom* CustomFilter = Cast< UInterpFilter_Custom >( IData->SelectedFilter );
		if( CustomFilter != NULL && IData->InterpFilters.ContainsItem( CustomFilter ) )
		{
			check( !CustomFilter->GroupsToInclude.ContainsItem( NewGroup ) );

			// Add the new group to the custom filter tab!
			CustomFilter->GroupsToInclude.AddItem( NewGroup );
		}
	}


	InterpEdTrans->EndSpecial();

	// Make sure particle replay tracks have up-to-date editor-only transient state
	UpdateParticleReplayTracks();

	// Make sure the director track window is only visible if we have a director group!
	UpdateDirectorTrackWindowVisibility();

	// A new group or track may have been added, so we'll update the group list scroll bar
	UpdateTrackWindowScrollBars();

	// Dirty the track window viewports
	InvalidateTrackWindowViewports();

	// If adding a camera- make sure its frustum colour is updated.
	UpdateCamColours();

	// Reimage actor world locations.  This must happen after the group was created.
	Interp->RecaptureActorState();
}



void WxInterpEd::OnContextNewTrack( wxCommandEvent& In )
{
	if( !ActiveGroup )
		return;

	// Find the class of the new track we want to add.
	const INT NewTrackClassIndex = In.GetId() - IDM_INTERP_NEW_TRACK_START;
	check( NewTrackClassIndex >= 0 && NewTrackClassIndex < InterpTrackClasses.Num() );

	UClass* NewInterpTrackClass = InterpTrackClasses(NewTrackClassIndex);
	check( NewInterpTrackClass->IsChildOf(UInterpTrack::StaticClass()) );

	AddTrackToSelectedGroup(NewInterpTrackClass, NULL);
}



/**
 * Called when the user selects the 'Expand All Groups' option from a menu.  Expands every group such that the
 * entire hierarchy of groups and tracks are displayed.
 */
void WxInterpEd::OnExpandAllGroups( wxCommandEvent& In )
{
	const UBOOL bWantExpand = TRUE;
	ExpandOrCollapseAllVisibleGroups( bWantExpand );
}



/**
 * Called when the user selects the 'Collapse All Groups' option from a menu.  Collapses every group in the group
 * list such that no tracks are displayed.
 */
void WxInterpEd::OnCollapseAllGroups( wxCommandEvent& In )
{
	const UBOOL bWantExpand = FALSE;
	ExpandOrCollapseAllVisibleGroups( bWantExpand );
}



/**
 * Expands or collapses all visible groups in the track editor
 *
 * @param bExpand TRUE to expand all groups, or FALSE to collapse them all
 */
void WxInterpEd::ExpandOrCollapseAllVisibleGroups( const UBOOL bExpand )
{
	// We'll keep track of whether or not something changes
	UBOOL bAnythingChanged = FALSE;

	// Iterate over each group
	for( INT CurGroupIndex = 0; CurGroupIndex < IData->InterpGroups.Num(); ++CurGroupIndex	)
	{
		UInterpGroup* CurGroup = IData->InterpGroups( CurGroupIndex );
		assert( CurGroup != NULL );
															 
		// Only expand/collapse visible groups
		const UBOOL bIsCollapsing = !bExpand;
		if( CurGroup->bVisible )
		{
			if( CurGroup->bCollapsed != bIsCollapsing )
			{
				// Expand or collapse this group!
				CurGroup->bCollapsed = bIsCollapsing;
			}
		}
	}

	if( bAnythingChanged )
	{
		// @todo: Should we re-scroll to the currently selected group if needed?

		// At least one group has been expanded or collapsed, so we need to update our scroll bar
		UpdateTrackWindowScrollBars();
	}
}


void WxInterpEd::OnMenuPlay( wxCommandEvent& In )
{
	const UBOOL bShouldLoop = ( In.GetId() == IDM_INTERP_PLAYLOOPSECTION );
	const UBOOL bPlayForward = ( In.GetId() != IDM_INTERP_PlayReverse );
	StartPlaying( bShouldLoop, bPlayForward );
}

void WxInterpEd::OnMenuStop( wxCommandEvent& In )
{
	StopPlaying();
}

void WxInterpEd::OnOpenBindKeysDialog(wxCommandEvent &In)
{
	GApp->DlgBindHotkeys->Show(TRUE);
	GApp->DlgBindHotkeys->SetFocus();
}

void WxInterpEd::OnChangePlaySpeed( wxCommandEvent& In )
{
	PlaybackSpeed = 1.f;

	switch( In.GetId() )
	{
	case IDM_INTERP_SPEED_1:
		PlaybackSpeed = 0.01f;
		break;
	case IDM_INTERP_SPEED_10:
		PlaybackSpeed = 0.1f;
		break;
	case IDM_INTERP_SPEED_25:
		PlaybackSpeed = 0.25f;
		break;
	case IDM_INTERP_SPEED_50:
		PlaybackSpeed = 0.5f;
		break;
	case IDM_INTERP_SPEED_100:
		PlaybackSpeed = 1.0f;
		break;
	}

	// Playback speed changed, so reset our playback start time so fixed time step playback can
	// gate frame rate properly
	PlaybackStartRealTime = appSeconds();
	NumContinuousFixedTimeStepFrames = 0;
}

void WxInterpEd::OnMenuStretchSection(wxCommandEvent& In)
{
	// Edit section markers should always be within sequence...

	const FLOAT CurrentSectionLength = IData->EdSectionEnd - IData->EdSectionStart;
	if(CurrentSectionLength < 0.01f)
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Error_HighlightNonZeroLength") );
		return;
	}


	const FString CurrentLengthStr = FString::Printf( TEXT("%3.3f"), CurrentSectionLength );

	// Display dialog and let user enter new length for this section.
	WxDlgGenericStringEntry dlg;
	const INT Result = dlg.ShowModal( TEXT("StretchSection"), TEXT("NewLength"), *CurrentLengthStr);
	if( Result != wxID_OK )
		return;

	double dNewSectionLength;
	const UBOOL bIsNumber = dlg.GetStringEntry().GetValue().ToDouble(&dNewSectionLength);
	if(!bIsNumber)
		return;

	const FLOAT NewSectionLength = (FLOAT)dNewSectionLength;
	if(NewSectionLength <= 0.f)
		return;

	InterpEdTrans->BeginSpecial( *LocalizeUnrealEd("StretchSection") );

	IData->Modify();
	Opt->Modify();

	const FLOAT LengthDiff = NewSectionLength - CurrentSectionLength;
	const FLOAT StretchRatio = NewSectionLength/CurrentSectionLength;

	// Iterate over all tracks.
	for(INT i=0; i<IData->InterpGroups.Num(); i++)
	{
		UInterpGroup* Group = IData->InterpGroups(i);
		for(INT j=0; j<Group->InterpTracks.Num(); j++)
		{
			UInterpTrack* Track = Group->InterpTracks(j);

			Track->Modify();

			for(INT k=0; k<Track->GetNumKeyframes(); k++)
			{
				const FLOAT KeyTime = Track->GetKeyframeTime(k);

				// Key is before start of stretched section
				if(KeyTime < IData->EdSectionStart)
				{
					// Leave key as it is
				}
				// Key is in section being stretched
				else if(KeyTime < IData->EdSectionEnd)
				{
					// Calculate new key time.
					const FLOAT FromSectionStart = KeyTime - IData->EdSectionStart;
					const FLOAT NewKeyTime = IData->EdSectionStart + (StretchRatio * FromSectionStart);

					Track->SetKeyframeTime(k, NewKeyTime, FALSE);
				}
				// Key is after stretched section
				else
				{
					// Move it on by the increase in sequence length.
					Track->SetKeyframeTime(k, KeyTime + LengthDiff, FALSE);
				}
			}
		}
	}

	// Move the end of the interpolation to account for changing the length of this section.
	SetInterpEnd(IData->InterpLength + LengthDiff);

	// Move end marker of section to new, stretched position.
	MoveLoopMarker( IData->EdSectionEnd + LengthDiff, FALSE );

	InterpEdTrans->EndSpecial();
}

/** Remove the currernt section, reducing the length of the sequence and moving any keys after the section earlier in time. */
void WxInterpEd::OnMenuDeleteSection(wxCommandEvent& In)
{
	const FLOAT CurrentSectionLength = IData->EdSectionEnd - IData->EdSectionStart;
	if(CurrentSectionLength < 0.01f)
		return;

	InterpEdTrans->BeginSpecial( *LocalizeUnrealEd("DeleteSection") );

	IData->Modify();
	Opt->Modify();

	// Add keys that are within current section to selection
	SelectKeysInLoopSection();

	// Delete current selection
	DeleteSelectedKeys(FALSE);

	// Then move any keys after the current section back by the length of the section.
	for(INT i=0; i<IData->InterpGroups.Num(); i++)
	{
		UInterpGroup* Group = IData->InterpGroups(i);
		for(INT j=0; j<Group->InterpTracks.Num(); j++)
		{
			UInterpTrack* Track = Group->InterpTracks(j);
			Track->Modify();

			for(INT k=0; k<Track->GetNumKeyframes(); k++)
			{
				// Move keys after section backwards by length of the section
				FLOAT KeyTime = Track->GetKeyframeTime(k);
				if(KeyTime > IData->EdSectionEnd)
				{
					// Add to selection for deletion.
					Track->SetKeyframeTime(k, KeyTime - CurrentSectionLength, FALSE);
				}
			}
		}
	}

	// Move the end of the interpolation to account for changing the length of this section.
	SetInterpEnd(IData->InterpLength - CurrentSectionLength);

	// Move section end marker on top of section start marker (section has vanished).
	MoveLoopMarker( IData->EdSectionStart, FALSE );

	InterpEdTrans->EndSpecial();
}

/** Insert an amount of space (specified by user in dialog) at the current position in the sequence. */
void WxInterpEd::OnMenuInsertSpace( wxCommandEvent& In )
{
	WxDlgGenericStringEntry dlg;
	INT Result = dlg.ShowModal( TEXT("InsertEmptySpace"), TEXT("Seconds"), TEXT("1.0"));
	if( Result != wxID_OK )
		return;

	double dAddTime;
	UBOOL bIsNumber = dlg.GetStringEntry().GetValue().ToDouble(&dAddTime);
	if(!bIsNumber)
		return;

	FLOAT AddTime = (FLOAT)dAddTime;

	// Ignore if adding a negative amount of time!
	if(AddTime <= 0.f)
		return;

	InterpEdTrans->BeginSpecial( *LocalizeUnrealEd("InsertSpace") );

	IData->Modify();
	Opt->Modify();

	// Move the end of the interpolation on by the amount we are adding.
	SetInterpEnd(IData->InterpLength + AddTime);

	// Iterate over all tracks.
	for(INT i=0; i<IData->InterpGroups.Num(); i++)
	{
		UInterpGroup* Group = IData->InterpGroups(i);
		for(INT j=0; j<Group->InterpTracks.Num(); j++)
		{
			UInterpTrack* Track = Group->InterpTracks(j);

			Track->Modify();

			for(INT k=0; k<Track->GetNumKeyframes(); k++)
			{
				FLOAT KeyTime = Track->GetKeyframeTime(k);
				if(KeyTime > Interp->Position)
				{
					Track->SetKeyframeTime(k, KeyTime + AddTime, FALSE);
				}
			}
		}
	}

	InterpEdTrans->EndSpecial();
}

void WxInterpEd::OnMenuSelectInSection(wxCommandEvent& In)
{
	SelectKeysInLoopSection();
}

void WxInterpEd::OnMenuDuplicateSelectedKeys(wxCommandEvent& In)
{
	DuplicateSelectedKeys();
}

void WxInterpEd::OnSavePathTime( wxCommandEvent& In )
{
	IData->PathBuildTime = Interp->Position;
}

void WxInterpEd::OnJumpToPathTime( wxCommandEvent& In )
{
	SetInterpPosition(IData->PathBuildTime);
}

void WxInterpEd::OnViewHide3DTracks( wxCommandEvent& In )
{
	bHide3DTrackView = !bHide3DTrackView;
	MenuBar->ViewMenu->Check( IDM_INTERP_VIEW_Draw3DTrajectories, bHide3DTrackView == FALSE );

	// Save to ini when it changes.
	GConfig->SetBool( TEXT("Matinee"), TEXT("Hide3DTracks"), bHide3DTrackView, GEditorUserSettingsIni );
}

void WxInterpEd::OnViewZoomToScrubPos( wxCommandEvent& In )
{
	bZoomToScrubPos = !bZoomToScrubPos;
	MenuBar->ViewMenu->Check( IDM_INTERP_VIEW_ZoomToTimeCursorPosition, bZoomToScrubPos == TRUE );

	// Save to ini when it changes.
	GConfig->SetBool( TEXT("Matinee"), TEXT("ZoomToScrubPos"), bZoomToScrubPos, GEditorUserSettingsIni );
}


void WxInterpEd::OnToggleViewportFrameStats( wxCommandEvent& In )
{
	bViewportFrameStatsEnabled = !bViewportFrameStatsEnabled;
	MenuBar->ViewMenu->Check( IDM_INTERP_VIEW_ViewportFrameStats, bViewportFrameStatsEnabled == TRUE );

	// Save to ini when it changes.
	GConfig->SetBool( TEXT("Matinee"), TEXT("ViewportFrameStats"), bViewportFrameStatsEnabled, GEditorUserSettingsIni );
}


/** Called when the "Toggle Gore Preview" button is pressed */
void WxInterpEd::OnToggleGorePreview( wxCommandEvent& In )
{
	Interp->bShouldShowGore = !Interp->bShouldShowGore;
}


/** Called when the "Toggle Gore Preview" UI should be updated */
void WxInterpEd::OnToggleGorePreview_UpdateUI( wxUpdateUIEvent& In )
{
	In.Check( Interp->bShouldShowGore == TRUE );
}



void WxInterpEd::OnContextTrackRename( wxCommandEvent& In )
{
	if(ActiveTrackIndex == INDEX_NONE)
		return;

	check(ActiveGroup);

	UInterpTrack* Track = ActiveGroup->InterpTracks(ActiveTrackIndex);
	check(Track);

	WxDlgGenericStringEntry dlg;
	INT Result = dlg.ShowModal( TEXT("RenameTrack"), TEXT("NewTrackName"), *Track->TrackTitle );
	if( Result != wxID_OK )
		return;

	Track->TrackTitle = dlg.GetEnteredString();

	// In case this track is being displayed on the curve editor, update its name there too.
	FString CurveName = FString::Printf( TEXT("%s_%s"), *ActiveGroup->GroupName.ToString(), *Track->TrackTitle);
	IData->CurveEdSetup->ChangeCurveName(Track, CurveName);
	CurveEd->CurveChanged();
}

void WxInterpEd::OnContextTrackDelete( wxCommandEvent& In )
{
	DeleteSelectedTrack();
}

void WxInterpEd::OnContextTrackChangeFrame( wxCommandEvent& In  )
{
	check(ActiveGroup);

	UInterpTrack* Track = ActiveGroup->InterpTracks(ActiveTrackIndex);
	UInterpTrackMove* MoveTrack = Cast<UInterpTrackMove>(Track);
	if(!MoveTrack)
		return;

	// Find the frame we want to convert to.
	INT Id = In.GetId();
	BYTE DesiredFrame = 0;
	if(Id == IDM_INTERP_TRACK_FRAME_WORLD)
	{
		DesiredFrame = IMF_World;
	}
	else if(Id == IDM_INTERP_TRACK_FRAME_RELINITIAL)
	{
		DesiredFrame = IMF_RelativeToInitial;
	}
	else
	{
		check(0);
	}

	// Do nothing if already in desired frame
	if(DesiredFrame == MoveTrack->MoveFrame)
	{
		return;
	}
	
	// Find the first instance of this group. This is the one we are going to use to store the curve relative to.
	UInterpGroupInst* GrInst = Interp->FindFirstGroupInst(ActiveGroup);
	check(GrInst);

	AActor* Actor = GrInst->GroupActor;
	if(!Actor)
	{
		appMsgf(AMT_OK, *LocalizeUnrealEd("Error_NoActorForThisGroup"));
		return;
	}

	// Get instance of movement track, for the initial TM.
	UInterpTrackInstMove* MoveTrackInst = CastChecked<UInterpTrackInstMove>( GrInst->TrackInst(ActiveTrackIndex) );

	// Find the frame to convert key-frame from.
	FMatrix FromFrameTM = MoveTrack->GetMoveRefFrame(MoveTrackInst);

	// Find the frame to convert the key-frame into.
	AActor* BaseActor = Actor->GetBase();

	FMatrix BaseTM = FMatrix::Identity;
	if(BaseActor)
	{
		BaseTM = FRotationTranslationMatrix( BaseActor->Rotation, BaseActor->Location );
	}

	FMatrix ToFrameTM = FMatrix::Identity;
	if( DesiredFrame == IMF_World )
	{
		if(BaseActor)
		{
			ToFrameTM = BaseTM;
		}
		else
		{
			ToFrameTM = FMatrix::Identity;
		}
	}
	else if( DesiredFrame == IMF_RelativeToInitial )
	{
		if(BaseActor)
		{
			ToFrameTM = MoveTrackInst->InitialTM * BaseTM;
		}
		else
		{
			ToFrameTM = MoveTrackInst->InitialTM;
		}
	}
	FMatrix InvToFrameTM = ToFrameTM.Inverse();


	// Iterate over each keyframe. Convert key into world reference frame, then into new desired reference frame.
	check( MoveTrack->PosTrack.Points.Num() == MoveTrack->EulerTrack.Points.Num() );
	for(INT i=0; i<MoveTrack->PosTrack.Points.Num(); i++)
	{
		FQuat KeyQuat = FQuat::MakeFromEuler( MoveTrack->EulerTrack.Points(i).OutVal );
		FQuatRotationTranslationMatrix KeyTM( KeyQuat, MoveTrack->PosTrack.Points(i).OutVal );

		FMatrix WorldKeyTM = KeyTM * FromFrameTM;

		FVector WorldArriveTan = FromFrameTM.TransformNormal( MoveTrack->PosTrack.Points(i).ArriveTangent );
		FVector WorldLeaveTan = FromFrameTM.TransformNormal( MoveTrack->PosTrack.Points(i).LeaveTangent );

		FMatrix RelKeyTM = WorldKeyTM * InvToFrameTM;

		MoveTrack->PosTrack.Points(i).OutVal = RelKeyTM.GetOrigin();
		MoveTrack->PosTrack.Points(i).ArriveTangent = ToFrameTM.InverseTransformNormal( WorldArriveTan );
		MoveTrack->PosTrack.Points(i).LeaveTangent = ToFrameTM.InverseTransformNormal( WorldLeaveTan );

		MoveTrack->EulerTrack.Points(i).OutVal = FQuat(RelKeyTM).Euler();
	}

	MoveTrack->MoveFrame = DesiredFrame;

	//PropertyWindow->Refresh(); // Don't know why this doesn't work...

	PropertyWindow->SetObject(NULL, 1, 1, 0);
	PropertyWindow->SetObject(Track, 1, 1, 0);

	// We changed the interp mode, so dirty the Matinee sequence
	Interp->MarkPackageDirty();
}



/**
 * Toggles visibility of the trajectory for the selected movement track
 */
void WxInterpEd::OnContextTrackShow3DTrajectory( wxCommandEvent& In )
{
	check( ActiveGroup != NULL );

	// Grab the movement track for the selected group
	UInterpTrack* Track = ActiveGroup->InterpTracks( ActiveTrackIndex );
	UInterpTrackMove* MoveTrack = Cast<UInterpTrackMove>( Track );
	if( MoveTrack != NULL )
	{
		InterpEdTrans->BeginSpecial( *LocalizeUnrealEd( "InterpEd_Undo_ToggleTrajectory" ) );
		MoveTrack->Modify();

		MoveTrack->bHide3DTrack = !MoveTrack->bHide3DTrack;

		InterpEdTrans->EndSpecial();
	}
}



/**
 * Shows or hides all movement track trajectories in the Matinee sequence
 */
void WxInterpEd::OnViewShowOrHideAll3DTrajectories( wxCommandEvent& In )
{
	// Are we showing or hiding track trajectories?
	const UBOOL bShouldHideTrajectories = ( In.GetId() == IDM_INTERP_VIEW_HideAll3DTrajectories );

	UBOOL bAnyTracksModified = FALSE;

	// Iterate over each group
	for( INT CurGroupIndex = 0; CurGroupIndex < IData->InterpGroups.Num(); ++CurGroupIndex	)
	{
		UInterpGroup* CurGroup = IData->InterpGroups( CurGroupIndex );
		assert( CurGroup != NULL );

		// Iterate over tracks in this group
		for( INT CurTrackIndex = 0; CurTrackIndex < CurGroup->InterpTracks.Num(); ++CurTrackIndex )
		{
			UInterpTrack* CurTrack = CurGroup->InterpTracks( CurTrackIndex );
			assert( CurTrack != NULL );

			// Is this a movement track?  Only movement tracks have trajectories
			UInterpTrackMove* MovementTrack = Cast<UInterpTrackMove>( CurTrack );
			if( MovementTrack != NULL )
			{
				if( bShouldHideTrajectories != MovementTrack->bHide3DTrack )
				{
					// Begin our undo transaction if we haven't started on already
					if( !bAnyTracksModified )
					{
						InterpEdTrans->BeginSpecial( *LocalizeUnrealEd( "InterpEd_Undo_ShowOrHideAllTrajectories" ) );
						bAnyTracksModified = TRUE;
					}

					// Show or hide the trajectory for this movement track
					MovementTrack->Modify();
					MovementTrack->bHide3DTrack = bShouldHideTrajectories;
				}
			}
		}
	}

	// End our undo transaction, but only if we actually modified something
	if( bAnyTracksModified )
	{
		InterpEdTrans->EndSpecial();
	}
}



/** Toggles 'capture mode' for particle replay tracks */
void WxInterpEd::OnParticleReplayTrackContext_ToggleCapture( wxCommandEvent& In )
{
	check( ActiveGroup != NULL );

	UInterpTrack* Track = ActiveGroup->InterpTracks( ActiveTrackIndex );
	UInterpTrackParticleReplay* ParticleReplayTrack = Cast<UInterpTrackParticleReplay>( Track );
	if( ParticleReplayTrack != NULL )
	{
		const UBOOL bEnableCapture =
			( In.GetId() == IDM_INTERP_ParticleReplayTrackContext_StartRecording );

		// Toggle capture mode
		ParticleReplayTrack->bIsCapturingReplay = bEnableCapture;

		// Dirty the track window viewports
		InvalidateTrackWindowViewports();
	}
}



void WxInterpEd::OnContextGroupRename( wxCommandEvent& In )
{
	if(!ActiveGroup)
	{
		return;
	}

	WxDlgGenericStringEntry dlg;
	FName NewName = ActiveGroup->GroupName;
	UBOOL bValidName = FALSE;

	while(!bValidName)
	{
		FString DialogName = ActiveGroup->bIsFolder ? TEXT( "RenameFolder" ) : TEXT( "RenameGroup" );
		FString PromptName = ActiveGroup->bIsFolder ? TEXT( "NewFolderName" ) : TEXT( "NewGroupName" );
		INT Result = dlg.ShowModal( *DialogName, *PromptName, *NewName.ToString() );
		if( Result != wxID_OK )
		{
			return;
		}

		NewName = FName(*dlg.GetEnteredString());
		bValidName = TRUE;

		// Check this name does not already exist.
		for(INT i=0; i<IData->InterpGroups.Num() && bValidName; i++)
		{
			if(IData->InterpGroups(i)->GroupName == NewName)
			{
				bValidName = FALSE;
			}
		}

		if(!bValidName)
		{
			appMsgf( AMT_OK, *LocalizeUnrealEd("Error_NameAlreadyExists") );
		}
	}
	
	// We also need to change the name of the variable connector on all SeqAct_Interps in this level using this InterpData
	USequence* RootSeq = Interp->GetRootSequence();
	check(RootSeq);

	TArray<USequenceObject*> MatineeActions;
	RootSeq->FindSeqObjectsByClass( USeqAct_Interp::StaticClass(), MatineeActions );

	for(INT i=0; i<MatineeActions.Num(); i++)
	{
		USeqAct_Interp* TestAction = CastChecked<USeqAct_Interp>( MatineeActions(i) );
		check(TestAction);
	
		UInterpData* TestData = TestAction->FindInterpDataFromVariable();
		if(TestData && TestData == IData)
		{
			INT VarIndex = TestAction->FindConnectorIndex( ActiveGroup->GroupName.ToString(), LOC_VARIABLE );
			if(VarIndex != INDEX_NONE && VarIndex >= 1) // Ensure variable index is not the reserved first one.
			{
				TestAction->VariableLinks(VarIndex).LinkDesc = NewName.ToString();
			}
		}
	}

	// Update any camera cuts to point to new group name
	UInterpGroupDirector* DirGroup = IData->FindDirectorGroup();
	if(DirGroup)
	{
		UInterpTrackDirector* DirTrack = DirGroup->GetDirectorTrack();
		if(DirTrack)
		{
			for(INT i=0; i<DirTrack->CutTrack.Num(); i++)
			{
				FDirectorTrackCut& Cut = DirTrack->CutTrack(i);
				if(Cut.TargetCamGroup == ActiveGroup->GroupName)
				{
					Cut.TargetCamGroup = NewName;
				}	
			}
		}
	}

	// Change the name of the InterpGroup.
	ActiveGroup->GroupName = NewName;
}

void WxInterpEd::OnContextGroupDelete( wxCommandEvent& In )
{
	if(!ActiveGroup)
	{
		return;
	}

	if( ActiveGroup->bIsFolder )
	{
		// Check we REALLY want to do this.
		UBOOL bDoDestroy = appMsgf(AMT_YesNo, LocalizeSecure(LocalizeUnrealEd( "InterpEd_DeleteSelectedFolder" ), *ActiveGroup->GroupName.ToString()));
		if(!bDoDestroy)
		{
			return;
		}
	}

	DeleteSelectedGroup();
}

/** Prompts the user for a name for a new filter and creates a custom filter. */
void WxInterpEd::OnContextGroupCreateTab( wxCommandEvent& In )
{
	// Display dialog and let user enter new time.
	FString TabName;

	WxDlgGenericStringEntry Dlg;
	const INT Result = Dlg.ShowModal( TEXT("CreateGroupTab_Title"), TEXT("CreateGroupTab_Caption"), TEXT(""));
	if( Result == wxID_OK )
	{
		// Create a new tab.
		if(ActiveGroup != NULL)
		{
			UInterpFilter_Custom* Filter = ConstructObject<UInterpFilter_Custom>(UInterpFilter_Custom::StaticClass(), IData, NAME_None, RF_Transactional);

			if(Dlg.GetEnteredString().Len())
			{
				Filter->Caption = Dlg.GetEnteredString();
			}
			else
			{
				Filter->Caption = Filter->GetName();
			}
	
			Filter->GroupsToInclude.AddItem(ActiveGroup);
			IData->InterpFilters.AddItem(Filter);
		}
	}
}

/** Sends the selected group to the tab the user specified.  */
void WxInterpEd::OnContextGroupSendToTab( wxCommandEvent& In )
{
	INT TabIndex = (In.GetId() - IDM_INTERP_GROUP_SENDTOTAB_START);

	if(TabIndex >=0 && TabIndex < IData->InterpFilters.Num() && ActiveGroup != NULL)
	{
		// Make sure the active group isnt already in the filter's set of groups.
		UInterpFilter_Custom* Filter = Cast<UInterpFilter_Custom>(IData->InterpFilters(TabIndex));

		if(Filter != NULL && Filter->GroupsToInclude.ContainsItem(ActiveGroup) == FALSE)
		{
			Filter->GroupsToInclude.AddItem(ActiveGroup);
		}
	}
}

/** Removes the group from the current tab.  */
void WxInterpEd::OnContextGroupRemoveFromTab( wxCommandEvent& In )
{
	// Make sure the active group exists in the selected filter and that the selected filter isn't a default filter.
	UInterpFilter_Custom* Filter = Cast<UInterpFilter_Custom>(IData->SelectedFilter);

	if(Filter != NULL && Filter->GroupsToInclude.ContainsItem(ActiveGroup) == TRUE && IData->InterpFilters.ContainsItem(Filter) == TRUE)
	{
		Filter->GroupsToInclude.RemoveItem(ActiveGroup);
		ActiveGroup->bVisible = FALSE;

		// Dirty the track window viewports
		InvalidateTrackWindowViewports();
	}
}

/** Deletes the currently selected group tab.  */
void WxInterpEd::OnContextDeleteGroupTab( wxCommandEvent& In )
{
	// Make sure the active group exists in the selected filter and that the selected filter isn't a default filter.
	UInterpFilter_Custom* Filter = Cast<UInterpFilter_Custom>(IData->SelectedFilter);

	if(Filter != NULL)
	{
		IData->InterpFilters.RemoveItem(Filter);

		// Set the selected filter back to the all filter.
		if(IData->DefaultFilters.Num())
		{
			SetSelectedFilter(IData->DefaultFilters(0));
		}
		else
		{
			SetSelectedFilter(NULL);
		}
	}
}



/** Called when the user selects to move a group to another group folder */
void WxInterpEd::OnContextGroupChangeGroupFolder( wxCommandEvent& In )
{
	// Figure out if we're moving the active group to a new group, or if we simply want to unparent it
	const UBOOL bIsParenting = ( In.GetId() != IDM_INTERP_GROUP_RemoveFromGroupFolder );


	// Figure out which direction we're moving things: A group to the selected folder?  Or, the selected group
	// to a folder?
	UBOOL bIsMovingActiveGroupToFolder = FALSE;
	UBOOL bIsMovingGroupToActiveFolder = FALSE;
	if( bIsParenting )
	{
		bIsMovingActiveGroupToFolder =
			( In.GetId() >= IDM_INTERP_GROUP_MoveActiveGroupToFolder_Start && In.GetId() <= IDM_INTERP_GROUP_MoveActiveGroupToFolder_End );
		bIsMovingGroupToActiveFolder = !bIsMovingActiveGroupToFolder;
	}

	// Make sure we're dealing with a valid group index
	INT MenuGroupIndex = INDEX_NONE;
	UBOOL bIsValidGroupIndex = TRUE;
	if( bIsParenting )
	{
		MenuGroupIndex =
			bIsMovingActiveGroupToFolder ?
				( In.GetId() - IDM_INTERP_GROUP_MoveActiveGroupToFolder_Start ) :
				( In.GetId() - IDM_INTERP_GROUP_MoveGroupToActiveFolder_Start );
	}
	else
	{
		// If we're unparenting, then use ourselves as the destination index
		MenuGroupIndex = IData->InterpGroups.FindItemIndex( ActiveGroup );

		// Make sure we're not already in the desired state; this would be a UI error
		check( ActiveGroup->bIsParented );
	}

	bIsValidGroupIndex = ( MenuGroupIndex >= 0 && MenuGroupIndex < IData->InterpGroups.Num() );
	check( bIsValidGroupIndex );
	if( !bIsValidGroupIndex || ActiveGroup == NULL )
	{
		return;
	}


	// Figure out what our source and destination groups are for this operation
	UInterpGroup* SourceGroup = NULL;
	INT DestGroupIndex = INDEX_NONE;
	if( !bIsParenting || bIsMovingActiveGroupToFolder )
	{
		// We're moving the active group to a group, or unparenting a group
		DestGroupIndex = MenuGroupIndex;
		SourceGroup = ActiveGroup;
	}
	else
	{
		// We're moving a group to our active group
		DestGroupIndex = IData->InterpGroups.FindItemIndex( ActiveGroup );
		SourceGroup = IData->InterpGroups( MenuGroupIndex );
	}



	// OK, to pull this off we need to do two things.  First, we need to relocate the source group such that
	// it's at the bottom of the destination group's children in our list.  Then, we'll need to mark the
	// group as 'parented'!

	// We're about to modify stuff!
	InterpEdTrans->BeginSpecial( *LocalizeUnrealEd( "InterpEd_ChangeGroupFolder" ) );
	Interp->Modify();
	IData->Modify();

	// First, remove ourselves from the group list
	{
		INT SourceGroupIndex = IData->InterpGroups.FindItemIndex( SourceGroup );
		IData->InterpGroups.Remove( SourceGroupIndex );

		// Adjust destination group index if we've removed an item earlier than it in the list
		if( SourceGroupIndex <= DestGroupIndex )
		{
			--DestGroupIndex;
		}
	}

	INT TargetGroupIndex = DestGroupIndex + 1;
	for( INT OtherGroupIndex = TargetGroupIndex; OtherGroupIndex < IData->InterpGroups.Num(); ++OtherGroupIndex )
	{
		UInterpGroup* OtherGroup = IData->InterpGroups( OtherGroupIndex );

		// Is this group parented?
		if( OtherGroup->bIsParented )
		{
			// OK, this is a child group of the destination group.  We want to append our new group to the end of
			// the destination group's list of children, so we'll just keep on iterating.
			++TargetGroupIndex;
		}
		else
		{
			// This group isn't the destination group or a child of the destination group.  We now have the index
			// we're looking for!
			break;
		}
	}

	// OK, now we know where we need to place the source group to in the list.  Let's do it!
	IData->InterpGroups.InsertItem( SourceGroup, TargetGroupIndex );

	// OK, now mark the group as parented!  Note that if we're relocating a group from one folder to another, it
	// may already be tagged as parented.
	if( SourceGroup->bIsParented != bIsParenting )
	{
		SourceGroup->Modify();
		SourceGroup->bIsParented = bIsParenting;
	}

	// Complete undo state
	InterpEdTrans->EndSpecial();

	// Dirty the track window viewports
	InvalidateTrackWindowViewports();
}



// Iterate over keys changing their interpolation mode and adjusting tangents appropriately.
void WxInterpEd::OnContextKeyInterpMode( wxCommandEvent& In )
{

	for(INT i=0; i<Opt->SelectedKeys.Num(); i++)
	{
		FInterpEdSelKey& SelKey = Opt->SelectedKeys(i);
		UInterpTrack* Track = SelKey.Group->InterpTracks(SelKey.TrackIndex);

		if(In.GetId() == IDM_INTERP_KEYMODE_LINEAR)
		{
			Track->SetKeyInterpMode( SelKey.KeyIndex, CIM_Linear );
		}
		else if(In.GetId() == IDM_INTERP_KEYMODE_CURVE_AUTO)
		{
			Track->SetKeyInterpMode( SelKey.KeyIndex, CIM_CurveAuto );
		}
		else if(In.GetId() == IDM_INTERP_KEYMODE_CURVE_AUTO_CLAMPED)
		{
			Track->SetKeyInterpMode( SelKey.KeyIndex, CIM_CurveAutoClamped );
		}
		else if(In.GetId() == IDM_INTERP_KEYMODE_CURVEBREAK)
		{
			Track->SetKeyInterpMode( SelKey.KeyIndex, CIM_CurveBreak );
		}
		else if(In.GetId() == IDM_INTERP_KEYMODE_CONSTANT)
		{
			Track->SetKeyInterpMode( SelKey.KeyIndex, CIM_Constant );
		}
	}

	CurveEd->UpdateDisplay();
}

/** Pops up menu and lets you set the time for the selected key. */
void WxInterpEd::OnContextSetKeyTime( wxCommandEvent& In )
{
	// Only works if one key is selected.
	if(Opt->SelectedKeys.Num() != 1)
	{
		return;
	}

	// Get the time the selected key is currently at.
	FInterpEdSelKey& SelKey = Opt->SelectedKeys(0);
	UInterpTrack* Track = SelKey.Group->InterpTracks(SelKey.TrackIndex);

	FLOAT CurrentKeyTime = Track->GetKeyframeTime(SelKey.KeyIndex);
	const FString CurrentTimeStr = FString::Printf( TEXT("%3.3f"), CurrentKeyTime );

	// Display dialog and let user enter new time.
	WxDlgGenericStringEntry dlg;
	const INT Result = dlg.ShowModal( TEXT("NewKeyTime"), TEXT("NewTime"), *CurrentTimeStr);
	if( Result != wxID_OK )
		return;

	double dNewTime;
	const UBOOL bIsNumber = dlg.GetStringEntry().GetValue().ToDouble(&dNewTime);
	if(!bIsNumber)
		return;

	const FLOAT NewKeyTime = (FLOAT)dNewTime;

	// Move the key. Also update selected to reflect new key index.
	SelKey.KeyIndex = Track->SetKeyframeTime( SelKey.KeyIndex, NewKeyTime );

	// Update positions at current time but with new keyframe times.
	RefreshInterpPosition();
	CurveEd->UpdateDisplay();
}

/** Pops up a menu and lets you set the value for the selected key. Not all track types are supported. */
void WxInterpEd::OnContextSetValue( wxCommandEvent& In )
{
	// Only works if one key is selected.
	if(Opt->SelectedKeys.Num() != 1)
	{
		return;
	}

	// Get the time the selected key is currently at.
	FInterpEdSelKey& SelKey = Opt->SelectedKeys(0);
	UInterpTrack* Track = SelKey.Group->InterpTracks(SelKey.TrackIndex);

	// If its a float track - pop up text entry dialog.
	UInterpTrackFloatBase* FloatTrack = Cast<UInterpTrackFloatBase>(Track);
	if(FloatTrack)
	{
		// Get current float value of the key
		FLOAT CurrentKeyVal = FloatTrack->FloatTrack.Points(SelKey.KeyIndex).OutVal;
		const FString CurrentValStr = FString::Printf( TEXT("%f"), CurrentKeyVal );

		// Display dialog and let user enter new value.
		WxDlgGenericStringEntry dlg;
		const INT Result = dlg.ShowModal( TEXT("NewKeyValue"), TEXT("NewValue"), *CurrentValStr);
		if( Result != wxID_OK )
			return;

		double dNewVal;
		const UBOOL bIsNumber = dlg.GetStringEntry().GetValue().ToDouble(&dNewVal);
		if(!bIsNumber)
			return;

		// Set new value, and update tangents.
		const FLOAT NewVal = (FLOAT)dNewVal;
		FloatTrack->FloatTrack.Points(SelKey.KeyIndex).OutVal = NewVal;
		FloatTrack->FloatTrack.AutoSetTangents(FloatTrack->CurveTension);
	}

	// Update positions at current time but with new keyframe times.
	RefreshInterpPosition();
	CurveEd->UpdateDisplay();
}


/** Pops up a menu and lets you set the color for the selected key. Not all track types are supported. */
void WxInterpEd::OnContextSetColor( wxCommandEvent& In )
{
	// Only works if one key is selected.
	if(Opt->SelectedKeys.Num() != 1)
	{
		return;
	}

	// Get the time the selected key is currently at.
	FInterpEdSelKey& SelKey = Opt->SelectedKeys(0);
	UInterpTrack* Track = SelKey.Group->InterpTracks(SelKey.TrackIndex);

	// If its a color prop track - pop up color dialog.
	UInterpTrackColorProp* ColorPropTrack = Cast<UInterpTrackColorProp>(Track);
	if(ColorPropTrack)
	{
		// Get the current color and show a color picker dialog.
		FVector CurrentColorVector = ColorPropTrack->VectorTrack.Points(SelKey.KeyIndex).OutVal;
		FColor CurrentColor(FLinearColor(CurrentColorVector.X, CurrentColorVector.Y, CurrentColorVector.Z));
		
		wxColour FinalColor = ::wxGetColourFromUser(this, wxColour(CurrentColor.R, CurrentColor.G, CurrentColor.B));
		
		if(FinalColor.Ok())
		{
			// The user chose a color so set the keyframe color to the color they picked.
			FLinearColor VectorColor(FColor(FinalColor.Red(), FinalColor.Green(), FinalColor.Blue()));

			ColorPropTrack->VectorTrack.Points(SelKey.KeyIndex).OutVal = FVector(VectorColor.R, VectorColor.G, VectorColor.B);
			ColorPropTrack->VectorTrack.AutoSetTangents(ColorPropTrack->CurveTension);
		}
	}

	// Update positions at current time but with new keyframe times.
	RefreshInterpPosition();
	CurveEd->UpdateDisplay();
}


/** Pops up menu and lets the user set a group to use to lookup transform info for a movement keyframe. */
void WxInterpEd::OnSetMoveKeyLookupGroup( wxCommandEvent& In )
{
	// Only works if one key is selected.
	if(Opt->SelectedKeys.Num() != 1)
	{
		return;
	}

	// Get the time the selected key is currently at.
	FInterpEdSelKey& SelKey = Opt->SelectedKeys(0);
	UInterpTrack* Track = SelKey.Group->InterpTracks(SelKey.TrackIndex);

	// Only perform work if we are on a movement track.
	UInterpTrackMove* MoveTrack = Cast<UInterpTrackMove>(Track);
	if(MoveTrack)
	{
		// Make array of group names
		TArray<FString> GroupNames;
		for ( INT GroupIdx = 0; GroupIdx < IData->InterpGroups.Num(); GroupIdx++ )
		{
			// Skip folder groups
			if( !IData->InterpGroups( GroupIdx )->bIsFolder )
			{
				if(IData->InterpGroups(GroupIdx) != SelKey.Group)
				{
					GroupNames.AddItem( *(IData->InterpGroups(GroupIdx)->GroupName.ToString()) );
				}
			}
		}

		WxDlgGenericComboEntry	dlg;
		const INT	Result = dlg.ShowModal( TEXT("SelectGroup"), TEXT("SelectGroupToLookupDataFrom"), GroupNames, 0, TRUE );
		if ( Result == wxID_OK )
		{
			FName KeyframeLookupGroup = FName( *dlg.GetSelectedString() );
			MoveTrack->SetLookupKeyGroupName(SelKey.KeyIndex, KeyframeLookupGroup);
		}

		
	}
}

/** Clears the lookup group for a currently selected movement key. */
void WxInterpEd::OnClearMoveKeyLookupGroup( wxCommandEvent& In )
{
	// Only works if one key is selected.
	if(Opt->SelectedKeys.Num() != 1)
	{
		return;
	}

	// Get the time the selected key is currently at.
	FInterpEdSelKey& SelKey = Opt->SelectedKeys(0);
	UInterpTrack* Track = SelKey.Group->InterpTracks(SelKey.TrackIndex);

	// Only perform work if we are on a movement track.
	UInterpTrackMove* MoveTrack = Cast<UInterpTrackMove>(Track);
	if(MoveTrack)
	{
		MoveTrack->ClearLookupKeyGroupName(SelKey.KeyIndex);
	}
}


/** Rename an event. Handle removing/adding connectors as appropriate. */
void WxInterpEd::OnContextRenameEventKey(wxCommandEvent& In)
{
	// Only works if one Event key is selected.
	if(Opt->SelectedKeys.Num() != 1)
	{
		return;
	}

	// Find the EventNames of selected key
	FName EventNameToChange;
	FInterpEdSelKey& SelKey = Opt->SelectedKeys(0);
	UInterpTrack* Track = SelKey.Group->InterpTracks(SelKey.TrackIndex);
	UInterpTrackEvent* EventTrack = Cast<UInterpTrackEvent>(Track);
	if(EventTrack)
	{
		EventNameToChange = EventTrack->EventTrack(SelKey.KeyIndex).EventName; 
	}
	else
	{
		return;
	}

	// Pop up dialog to ask for new name.
	WxDlgGenericStringEntry dlg;
	INT Result = dlg.ShowModal( TEXT("EnterNewEventName"), TEXT("NewEventName"), *EventNameToChange.ToString() );
	if( Result != wxID_OK )
		return;		

	FString TempString = dlg.GetEnteredString();
	TempString = TempString.Replace(TEXT(" "),TEXT("_"));
	FName NewEventName = FName( *TempString );

	// If this Event name is already in use- disallow it
	TArray<FName> CurrentEventNames;
	IData->GetAllEventNames(CurrentEventNames);
	if( CurrentEventNames.ContainsItem(NewEventName) )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Error_EventNameInUse") );
		return;
	}
	
	// Then go through all keys, changing those with this name to the new one.
	for(INT i=0; i<IData->InterpGroups.Num(); i++)
	{
		UInterpGroup* Group = IData->InterpGroups(i);
		for(INT j=0; j<Group->InterpTracks.Num(); j++)
		{
			UInterpTrackEvent* EventTrack = Cast<UInterpTrackEvent>( Group->InterpTracks(j) );
			if(EventTrack)
			{
				for(INT k=0; k<EventTrack->EventTrack.Num(); k++)
				{
					if(EventTrack->EventTrack(k).EventName == EventNameToChange)
					{
						EventTrack->EventTrack(k).EventName = NewEventName;
					}	
				}
			}			
		}
	}

	// We also need to change the name of the output connector on all SeqAct_Interps using this InterpData
	USequence* RootSeq = Interp->GetRootSequence();
	check(RootSeq);

	TArray<USequenceObject*> MatineeActions;
	RootSeq->FindSeqObjectsByClass( USeqAct_Interp::StaticClass(), MatineeActions );

	for(INT i=0; i<MatineeActions.Num(); i++)
	{
		USeqAct_Interp* TestAction = CastChecked<USeqAct_Interp>( MatineeActions(i) );
		check(TestAction);
	
		UInterpData* TestData = TestAction->FindInterpDataFromVariable();
		if(TestData && TestData == IData)
		{
			INT OutputIndex = TestAction->FindConnectorIndex( EventNameToChange.ToString(), LOC_OUTPUT );
			if(OutputIndex != INDEX_NONE && OutputIndex >= 2) // Ensure Output index is not one of the reserved first 2.
			{
				TestAction->OutputLinks(OutputIndex).LinkDesc = NewEventName.ToString();
			}
		}
	}
}

void WxInterpEd::OnSetAnimKeyLooping( wxCommandEvent& In )
{
	UBOOL bNewLooping = (In.GetId() == IDM_INTERP_ANIMKEY_LOOP);

	for(INT i=0; i<Opt->SelectedKeys.Num(); i++)
	{
		FInterpEdSelKey& SelKey = Opt->SelectedKeys(i);
		UInterpTrack* Track = SelKey.Group->InterpTracks(SelKey.TrackIndex);
		UInterpTrackAnimControl* AnimTrack = Cast<UInterpTrackAnimControl>(Track);
		if(AnimTrack)
		{
			AnimTrack->AnimSeqs(SelKey.KeyIndex).bLooping = bNewLooping;
		}
	}
}

void WxInterpEd::OnSetAnimOffset( wxCommandEvent& In )
{
	UBOOL bEndOffset = (In.GetId() == IDM_INTERP_ANIMKEY_SETENDOFFSET);

	if(Opt->SelectedKeys.Num() != 1)
	{
		return;
	}

	FInterpEdSelKey& SelKey = Opt->SelectedKeys(0);
	UInterpTrack* Track = SelKey.Group->InterpTracks(SelKey.TrackIndex);
	UInterpTrackAnimControl* AnimTrack = Cast<UInterpTrackAnimControl>(Track);
	if(!AnimTrack)
	{
		return;
	}

	FLOAT CurrentOffset = 0.f;
	if(bEndOffset)
	{
		CurrentOffset = AnimTrack->AnimSeqs(SelKey.KeyIndex).AnimEndOffset;
	}
	else
	{
		CurrentOffset = AnimTrack->AnimSeqs(SelKey.KeyIndex).AnimStartOffset;
	}

	const FString CurrentOffsetStr = FString::Printf( TEXT("%3.3f"), CurrentOffset );

	// Display dialog and let user enter new offste.
	WxDlgGenericStringEntry dlg;
	const INT Result = dlg.ShowModal( TEXT("NewAnimOffset"), TEXT("NewOffset"), *CurrentOffsetStr);
	if( Result != wxID_OK )
		return;

	double dNewOffset;
	const UBOOL bIsNumber = dlg.GetStringEntry().GetValue().ToDouble(&dNewOffset);
	if(!bIsNumber)
		return;

	const FLOAT NewOffset = ::Max( (FLOAT)dNewOffset, 0.f );

	if(bEndOffset)
	{
		AnimTrack->AnimSeqs(SelKey.KeyIndex).AnimEndOffset = NewOffset;
	}
	else
	{
		AnimTrack->AnimSeqs(SelKey.KeyIndex).AnimStartOffset = NewOffset;
	}


	// Update stuff in case doing this has changed it.
	RefreshInterpPosition();
}

void WxInterpEd::OnSetAnimPlayRate( wxCommandEvent& In )
{
	if(Opt->SelectedKeys.Num() != 1)
	{
		return;
	}

	FInterpEdSelKey& SelKey = Opt->SelectedKeys(0);
	UInterpTrack* Track = SelKey.Group->InterpTracks(SelKey.TrackIndex);
	UInterpTrackAnimControl* AnimTrack = Cast<UInterpTrackAnimControl>(Track);
	if(!AnimTrack)
	{
		return;
	}

	FLOAT CurrentRate = AnimTrack->AnimSeqs(SelKey.KeyIndex).AnimPlayRate;
	const FString CurrentRateStr = FString::Printf( TEXT("%3.3f"), CurrentRate );

	// Display dialog and let user enter new rate.
	WxDlgGenericStringEntry dlg;
	const INT Result = dlg.ShowModal( TEXT("NewAnimRate"), TEXT("PlayRate"), *CurrentRateStr);
	if( Result != wxID_OK )
		return;

	double dNewRate;
	const UBOOL bIsNumber = dlg.GetStringEntry().GetValue().ToDouble(&dNewRate);
	if(!bIsNumber)
		return;

	const FLOAT NewRate = ::Clamp( (FLOAT)dNewRate, 0.01f, 100.f );

	AnimTrack->AnimSeqs(SelKey.KeyIndex).AnimPlayRate = NewRate;

	// Update stuff in case doing this has changed it.
	RefreshInterpPosition();
}

/** Handler for the toggle animation reverse menu item. */
void WxInterpEd::OnToggleReverseAnim( wxCommandEvent& In )
{
	if(Opt->SelectedKeys.Num() != 1)
	{
		return;
	}

	FInterpEdSelKey& SelKey = Opt->SelectedKeys(0);
	UInterpTrack* Track = SelKey.Group->InterpTracks(SelKey.TrackIndex);
	UInterpTrackAnimControl* AnimTrack = Cast<UInterpTrackAnimControl>(Track);
	if(!AnimTrack)
	{
		return;
	}

	AnimTrack->AnimSeqs(SelKey.KeyIndex).bReverse = !AnimTrack->AnimSeqs(SelKey.KeyIndex).bReverse;
}

/** Handler for UI update requests for the toggle anim reverse menu item. */
void WxInterpEd::OnToggleReverseAnim_UpdateUI( wxUpdateUIEvent& In )
{
	if(Opt->SelectedKeys.Num() != 1)
	{
		return;
	}

	FInterpEdSelKey& SelKey = Opt->SelectedKeys(0);
	UInterpTrack* Track = SelKey.Group->InterpTracks(SelKey.TrackIndex);
	UInterpTrackAnimControl* AnimTrack = Cast<UInterpTrackAnimControl>(Track);
	if(!AnimTrack)
	{
		return;
	}

	In.Check(AnimTrack->AnimSeqs(SelKey.KeyIndex).bReverse==TRUE);
}

/** Handler for the save as camera animation menu item. */
void WxInterpEd::OnContextSaveAsCameraAnimation( wxCommandEvent& In )
{
	if (ActiveGroup == NULL)
	{
		return;
	}

	WxDlgPackageGroupName dlg;
	dlg.SetTitle( *LocalizeUnrealEd("ExportCameraAnim") );
	FString Reason;

	// pre-populate with whatever is selected in Generic Browser
	WxGenericBrowser* GBrowser = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT("GenericBrowser") );
	FString PackageName, GroupName, ObjName;
	{
		UPackage* SelectedPkg = GBrowser->GetTopPackage();
		PackageName = SelectedPkg ? SelectedPkg->GetName() : TEXT("");

		UPackage* SelectedGrp = GBrowser->GetGroup();
		GroupName = SelectedGrp ? SelectedGrp->GetName() : TEXT("");

		UObject* const SelectedCamAnim = GEditor->GetSelectedObjects()->GetTop<UCameraAnim>();
		ObjName = SelectedCamAnim ? *SelectedCamAnim->GetName() : TEXT("");
	}

	if( dlg.ShowModal( PackageName, GroupName, ObjName ) == wxID_OK )
	{
		FString Pkg;
		if( dlg.GetGroup().Len() > 0 )
		{
			Pkg = FString::Printf(TEXT("%s.%s"), *dlg.GetPackage(), *dlg.GetGroup());
		}
		else
		{
			Pkg = FString::Printf(TEXT("%s"), *dlg.GetPackage());
		}
		UObject* ExistingPackage = UObject::FindPackage(NULL, *Pkg);
		if( ExistingPackage == NULL )
		{
			// Create the package
			ExistingPackage = UObject::CreatePackage(NULL,*(GroupName != TEXT("") ? (PackageName+TEXT(".")+GroupName) : PackageName));
		}

		// Make sure packages objects are duplicated into are fully loaded.
		TArray<UPackage*> TopLevelPackages;
		if( ExistingPackage )
		{
			TopLevelPackages.AddItem( ExistingPackage->GetOutermost() );
		}

		if(!dlg.GetPackage().Len() || !dlg.GetObjectName().Len())
		{
			appMsgf(AMT_OK,*LocalizeUnrealEd("Error_InvalidInput"));
		}
// 		else if( !FIsValidObjectName( *dlg.GetObjectName(), Reason ))
// 		{
// 			appMsgf( AMT_OK, *Reason );
// 		}
		else
		{
			UBOOL bNewObject = FALSE, bSavedSuccessfully = FALSE;

			UObject* ExistingObject = GEditor->StaticFindObject(UCameraAnim::StaticClass(), ExistingPackage, *dlg.GetObjectName(), TRUE);

			if (ExistingObject == NULL)
			{
				// attempting to create a new object, need to handle fully loading
				if( GBrowser->HandleFullyLoadingPackages( TopLevelPackages, TEXT("ExportCameraAnim") ) )
				{
					// make sure name of new object is unique
					if (ExistingPackage && !FIsUniqueObjectName(*dlg.GetObjectName(), ExistingPackage, Reason))
					{
						appMsgf(AMT_OK, *Reason);
					}
					else
					{
						// create it, then copy params into it
						ExistingObject = GEditor->StaticConstructObject(UCameraAnim::StaticClass(), ExistingPackage, *dlg.GetObjectName(), RF_Public|RF_Standalone);
						bNewObject = TRUE;
						GBrowser->Update();
					}
				}
			}

			if (ExistingObject)
			{
				// copy params into it
				UCameraAnim* CamAnim = Cast<UCameraAnim>(ExistingObject);
				if (CamAnim->CreateFromInterpGroup(ActiveGroup, Interp))
				{
					bSavedSuccessfully = TRUE;
					CamAnim->MarkPackageDirty();

					// set generic browser selection to the newly-saved object
					GBrowser->GetSelection()->Select(ExistingObject, TRUE);
				}
			}

			if (bNewObject)
			{
				if (bSavedSuccessfully)
				{
					GBrowser->Update();
				}
				else
				{
					// delete the new object
					ExistingObject->MarkPendingKill();
				}
			}
		}
	}
}

/**
 * Prompts the user to edit volumes for the selected sound keys.
 */
void WxInterpEd::OnSetSoundVolume(wxCommandEvent& In)
{
	TArray<INT> SoundTrackKeyIndices;
	UBOOL bFoundVolume = FALSE;
	UBOOL bKeysDiffer = FALSE;
	FLOAT Volume = 1.0f;

	// Make a list of all keys and what their volumes are.
	for( INT i = 0 ; i < Opt->SelectedKeys.Num() ; ++i )
	{
		const FInterpEdSelKey& SelKey		= Opt->SelectedKeys(i);
		UInterpTrack* Track					= SelKey.Group->InterpTracks(SelKey.TrackIndex);
		UInterpTrackSound* SoundTrack		= Cast<UInterpTrackSound>( Track );

		if( SoundTrack )
		{
			SoundTrackKeyIndices.AddItem(i);
			const FSoundTrackKey& SoundTrackKey	= SoundTrack->Sounds(SelKey.KeyIndex);
			if ( !bFoundVolume )
			{
				bFoundVolume = TRUE;
				Volume = SoundTrackKey.Volume;
			}
			else
			{
				if ( Abs(Volume-SoundTrackKey.Volume) > KINDA_SMALL_NUMBER )
				{
					bKeysDiffer = TRUE;
				}
			}
		}
	}

	if ( SoundTrackKeyIndices.Num() )
	{
		// Display dialog and let user enter new rate.
		const FString VolumeStr( FString::Printf( TEXT("%2.2f"), bKeysDiffer ? 1.f : Volume ) );
		WxDlgGenericStringEntry dlg;
		const INT Result = dlg.ShowModal( TEXT("SetSoundVolume"), TEXT("Volume"), *VolumeStr );
		if( Result == wxID_OK )
		{
			double NewVolume;
			const UBOOL bIsNumber = dlg.GetStringEntry().GetValue().ToDouble( &NewVolume );
			if( bIsNumber )
			{
				const FLOAT ClampedNewVolume = ::Clamp( (FLOAT)NewVolume, 0.f, 100.f );
				for ( INT i = 0 ; i < SoundTrackKeyIndices.Num() ; ++i )
				{
					const INT Index						= SoundTrackKeyIndices(i);
					const FInterpEdSelKey& SelKey		= Opt->SelectedKeys(Index);
					UInterpTrack* Track					= SelKey.Group->InterpTracks(SelKey.TrackIndex);
					UInterpTrackSound* SoundTrack		= CastChecked<UInterpTrackSound>( Track );
					FSoundTrackKey& SoundTrackKey		= SoundTrack->Sounds(SelKey.KeyIndex);
					SoundTrackKey.Volume				= ClampedNewVolume;
				}
			}
		}

		Interp->MarkPackageDirty();

		// Update stuff in case doing this has changed it.
		RefreshInterpPosition();
	}
}

/**
 * Prompts the user to edit pitches for the selected sound keys.
 */
void WxInterpEd::OnSetSoundPitch(wxCommandEvent& In)
{
	TArray<INT> SoundTrackKeyIndices;
	UBOOL bFoundPitch = FALSE;
	UBOOL bKeysDiffer = FALSE;
	FLOAT Pitch = 1.0f;

	// Make a list of all keys and what their pitches are.
	for( INT i = 0 ; i < Opt->SelectedKeys.Num() ; ++i )
	{
		const FInterpEdSelKey& SelKey		= Opt->SelectedKeys(i);
		UInterpTrack* Track					= SelKey.Group->InterpTracks(SelKey.TrackIndex);
		UInterpTrackSound* SoundTrack		= Cast<UInterpTrackSound>( Track );

		if( SoundTrack )
		{
			SoundTrackKeyIndices.AddItem(i);
			const FSoundTrackKey& SoundTrackKey	= SoundTrack->Sounds(SelKey.KeyIndex);
			if ( !bFoundPitch )
			{
				bFoundPitch = TRUE;
				Pitch = SoundTrackKey.Pitch;
			}
			else
			{
				if ( Abs(Pitch-SoundTrackKey.Pitch) > KINDA_SMALL_NUMBER )
				{
					bKeysDiffer = TRUE;
				}
			}
		}
	}

	if ( SoundTrackKeyIndices.Num() )
	{
		// Display dialog and let user enter new rate.
		const FString PitchStr( FString::Printf( TEXT("%2.2f"), bKeysDiffer ? 1.f : Pitch ) );
		WxDlgGenericStringEntry dlg;
		const INT Result = dlg.ShowModal( TEXT("SetSoundPitch"), TEXT("Pitch"), *PitchStr );
		if( Result == wxID_OK )
		{
			double NewPitch;
			const UBOOL bIsNumber = dlg.GetStringEntry().GetValue().ToDouble( &NewPitch );
			if( bIsNumber )
			{
				const FLOAT ClampedNewPitch = ::Clamp( (FLOAT)NewPitch, 0.f, 100.f );
				for ( INT i = 0 ; i < SoundTrackKeyIndices.Num() ; ++i )
				{
					const INT Index						= SoundTrackKeyIndices(i);
					const FInterpEdSelKey& SelKey		= Opt->SelectedKeys(Index);
					UInterpTrack* Track					= SelKey.Group->InterpTracks(SelKey.TrackIndex);
					UInterpTrackSound* SoundTrack		= CastChecked<UInterpTrackSound>( Track );
					FSoundTrackKey& SoundTrackKey		= SoundTrack->Sounds(SelKey.KeyIndex);
					SoundTrackKey.Pitch					= ClampedNewPitch;
				}
			}
		}

		Interp->MarkPackageDirty();

		// Update stuff in case doing this has changed it.
		RefreshInterpPosition();
	}
}



/** Syncs the generic browser to the currently selected sound track key */
void WxInterpEd::OnKeyContext_SyncGenericBrowserToSoundCue( wxCommandEvent& In )
{
	if( Opt->SelectedKeys.Num() > 0 )
	{
		// Does this key have a sound cue set?
		FInterpEdSelKey& SelKey = Opt->SelectedKeys( 0 );
		UInterpTrackSound* SoundTrack = Cast<UInterpTrackSound>( SelKey.Group->InterpTracks( SelKey.TrackIndex ) );
		USoundCue* KeySoundCue = SoundTrack->Sounds( SelKey.KeyIndex ).Sound;
		if( KeySoundCue != NULL )
		{
			TArray< UObject* > Objects;
			Objects.AddItem( KeySoundCue );

			// Sync the generic browser!
			WxGenericBrowser* GenericBrowser = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT("GenericBrowser") );
			if( GenericBrowser )
			{
				// Make sure the window is visible.  The window needs to be visible *before*
				// the browser is sync'd to objects so that redraws actually happen!
				GUnrealEd->GetBrowserManager()->ShowWindow( GenericBrowser->GetDockID(), TRUE );

				// Sync.
				GenericBrowser->SyncToObjects( Objects );
			}
		}
	}
}



/** Called when the user wants to set the master volume on Audio Master track keys */
void WxInterpEd::OnKeyContext_SetMasterVolume( wxCommandEvent& In )
{
	TArray<INT> SoundTrackKeyIndices;
	UBOOL bFoundVolume = FALSE;
	UBOOL bKeysDiffer = FALSE;
	FLOAT Volume = 1.0f;

	// Make a list of all keys and what their volumes are.
	for( INT i = 0 ; i < Opt->SelectedKeys.Num() ; ++i )
	{
		const FInterpEdSelKey& SelKey		= Opt->SelectedKeys(i);
		UInterpTrack* Track					= SelKey.Group->InterpTracks(SelKey.TrackIndex);
		UInterpTrackAudioMaster* AudioMasterTrack = Cast<UInterpTrackAudioMaster>( Track );

		if( AudioMasterTrack != NULL )
		{
			// SubIndex 0 = Volume
			const FLOAT CurKeyVolume = AudioMasterTrack->GetKeyOut( 0, SelKey.KeyIndex );

			SoundTrackKeyIndices.AddItem(i);
			if ( !bFoundVolume )
			{
				bFoundVolume = TRUE;
				Volume = CurKeyVolume;
			}
			else
			{
				if ( Abs(Volume-CurKeyVolume) > KINDA_SMALL_NUMBER )
				{
					bKeysDiffer = TRUE;
				}
			}
		}
	}

	if ( SoundTrackKeyIndices.Num() )
	{
		// Display dialog and let user enter new rate.
		const FString VolumeStr( FString::Printf( TEXT("%2.2f"), bKeysDiffer ? 1.f : Volume ) );
		WxDlgGenericStringEntry dlg;
		const INT Result = dlg.ShowModal( TEXT("SetSoundVolume"), TEXT("Volume"), *VolumeStr );
		if( Result == wxID_OK )
		{
			double NewVolume;
			const UBOOL bIsNumber = dlg.GetStringEntry().GetValue().ToDouble( &NewVolume );
			if( bIsNumber )
			{
				const FLOAT ClampedNewVolume = ::Clamp( (FLOAT)NewVolume, 0.f, 100.f );
				for ( INT i = 0 ; i < SoundTrackKeyIndices.Num() ; ++i )
				{
					const INT Index						= SoundTrackKeyIndices(i);
					const FInterpEdSelKey& SelKey		= Opt->SelectedKeys(Index);
					UInterpTrack* Track					= SelKey.Group->InterpTracks(SelKey.TrackIndex);
					UInterpTrackAudioMaster* AudioMasterTrack = Cast<UInterpTrackAudioMaster>( Track );

					// SubIndex 0 = Volume
					AudioMasterTrack->SetKeyOut( 0, SelKey.KeyIndex, ClampedNewVolume );
				}
			}
		}

		Interp->MarkPackageDirty();

		// Update stuff in case doing this has changed it.
		RefreshInterpPosition();
	}
}



/** Called when the user wants to set the master pitch on Audio Master track keys */
void WxInterpEd::OnKeyContext_SetMasterPitch( wxCommandEvent& In )
{
	TArray<INT> SoundTrackKeyIndices;
	UBOOL bFoundPitch = FALSE;
	UBOOL bKeysDiffer = FALSE;
	FLOAT Pitch = 1.0f;

	// Make a list of all keys and what their pitches are.
	for( INT i = 0 ; i < Opt->SelectedKeys.Num() ; ++i )
	{
		const FInterpEdSelKey& SelKey		= Opt->SelectedKeys(i);
		UInterpTrack* Track					= SelKey.Group->InterpTracks(SelKey.TrackIndex);
		UInterpTrackAudioMaster* AudioMasterTrack = Cast<UInterpTrackAudioMaster>( Track );

		if( AudioMasterTrack != NULL )
		{
			// SubIndex 1 = Pitch
			const FLOAT CurKeyPitch = AudioMasterTrack->GetKeyOut( 1, SelKey.KeyIndex );

			SoundTrackKeyIndices.AddItem(i);
			if ( !bFoundPitch )
			{
				bFoundPitch = TRUE;
				Pitch = CurKeyPitch;
			}
			else
			{
				if ( Abs(Pitch-CurKeyPitch) > KINDA_SMALL_NUMBER )
				{
					bKeysDiffer = TRUE;
				}
			}
		}
	}

	if ( SoundTrackKeyIndices.Num() )
	{
		// Display dialog and let user enter new rate.
		const FString PitchStr( FString::Printf( TEXT("%2.2f"), bKeysDiffer ? 1.f : Pitch ) );
		WxDlgGenericStringEntry dlg;
		const INT Result = dlg.ShowModal( TEXT("SetSoundPitch"), TEXT("Pitch"), *PitchStr );
		if( Result == wxID_OK )
		{
			double NewPitch;
			const UBOOL bIsNumber = dlg.GetStringEntry().GetValue().ToDouble( &NewPitch );
			if( bIsNumber )
			{
				const FLOAT ClampedNewPitch = ::Clamp( (FLOAT)NewPitch, 0.f, 100.f );
				for ( INT i = 0 ; i < SoundTrackKeyIndices.Num() ; ++i )
				{
					const INT Index						= SoundTrackKeyIndices(i);
					const FInterpEdSelKey& SelKey		= Opt->SelectedKeys(Index);
					UInterpTrack* Track					= SelKey.Group->InterpTracks(SelKey.TrackIndex);
					UInterpTrackAudioMaster* AudioMasterTrack = Cast<UInterpTrackAudioMaster>( Track );

					// SubIndex 1 = Pitch
					AudioMasterTrack->SetKeyOut( 1, SelKey.KeyIndex, ClampedNewPitch );
				}
			}
		}

		Interp->MarkPackageDirty();

		// Update stuff in case doing this has changed it.
		RefreshInterpPosition();
	}
}



/** Called when the user wants to set the clip ID number for Particle Replay track keys */
void WxInterpEd::OnParticleReplayKeyContext_SetClipIDNumber( wxCommandEvent& In )
{
	if( Opt->SelectedKeys.Num() > 0 )
	{
		const FInterpEdSelKey& FirstSelectedKey = Opt->SelectedKeys( 0 );

		// We only support operating on one key at a time, we'll use the first selected key.
		UInterpTrackParticleReplay* ParticleReplayTrack =
			Cast< UInterpTrackParticleReplay >( FirstSelectedKey.Group->InterpTracks( FirstSelectedKey.TrackIndex ) );
		if( ParticleReplayTrack != NULL )
		{
			FParticleReplayTrackKey& ParticleReplayKey =
				ParticleReplayTrack->TrackKeys( FirstSelectedKey.KeyIndex );

			WxDlgGenericStringEntry StringEntryDialog;
			const INT DlgResult =
				StringEntryDialog.ShowModal(
					TEXT( "InterpEd_SetParticleReplayKeyClipIDNumber_DialogTitle" ),	// Title
					TEXT( "InterpEd_SetParticleReplayKeyClipIDNumber_DialogCaption" ),	// Caption
					*appItoa( ParticleReplayKey.ClipIDNumber ) );						// Initial value

			if( DlgResult == wxID_OK )
			{
				long NewClipIDNumber;	// 'long', for WxWidgets
				const UBOOL bIsNumber = StringEntryDialog.GetStringEntry().GetValue().ToLong( &NewClipIDNumber );
				if( bIsNumber )
				{
					// Store the new value!
					ParticleReplayKey.ClipIDNumber = NewClipIDNumber;

					// Mark the package as dirty
					Interp->MarkPackageDirty();

					// Refresh Matinee
					RefreshInterpPosition();
				}
			}
		}
	}
}



/** Called when the user wants to set the duration of Particle Replay track keys */
void WxInterpEd::OnParticleReplayKeyContext_SetDuration( wxCommandEvent& In )
{
	if( Opt->SelectedKeys.Num() > 0 )
	{
		const FInterpEdSelKey& FirstSelectedKey = Opt->SelectedKeys( 0 );

		// We only support operating on one key at a time, we'll use the first selected key.
		UInterpTrackParticleReplay* ParticleReplayTrack =
			Cast< UInterpTrackParticleReplay >( FirstSelectedKey.Group->InterpTracks( FirstSelectedKey.TrackIndex ) );
		if( ParticleReplayTrack != NULL )
		{
			FParticleReplayTrackKey& ParticleReplayKey =
				ParticleReplayTrack->TrackKeys( FirstSelectedKey.KeyIndex );

			WxDlgGenericStringEntry StringEntryDialog;
			const INT DlgResult =
				StringEntryDialog.ShowModal(
					TEXT( "InterpEd_SetParticleReplayKeyDuration_DialogTitle" ),	// Title
					TEXT( "InterpEd_SetParticleReplayKeyDuration_DialogCaption" ),	// Caption
					*FString::Printf( TEXT( "%2.2f" ), ParticleReplayKey.Duration ) );			// Initial value

			if( DlgResult == wxID_OK )
			{
				double NewDuration;		// 'double', for WxWidgets
				const UBOOL bIsNumber = StringEntryDialog.GetStringEntry().GetValue().ToDouble( &NewDuration );
				if( bIsNumber )
				{
					// Store the new value!
					ParticleReplayKey.Duration = ( FLOAT )NewDuration;

					// Mark the package as dirty
					Interp->MarkPackageDirty();

					// Refresh Matinee
					RefreshInterpPosition();
				}
			}
		}
	}
}


	
/** Called to delete the currently selected keys */
void WxInterpEd::OnDeleteSelectedKeys( wxCommandEvent& In )
{
	const UBOOL bWantTransactions = TRUE;
	DeleteSelectedKeys( bWantTransactions );
}



void WxInterpEd::OnContextDirKeyTransitionTime( wxCommandEvent& In )
{
	if(Opt->SelectedKeys.Num() != 1)
	{
		return;
	}

	FInterpEdSelKey& SelKey = Opt->SelectedKeys(0);
	UInterpTrack* Track = SelKey.Group->InterpTracks(SelKey.TrackIndex);
	UInterpTrackDirector* DirTrack = Cast<UInterpTrackDirector>(Track);
	if(!DirTrack)
	{
		return;
	}

	FLOAT CurrentTime = DirTrack->CutTrack(SelKey.KeyIndex).TransitionTime;
	const FString CurrentTimeStr = FString::Printf( TEXT("%3.3f"), CurrentTime );

	// Display dialog and let user enter new time.
	WxDlgGenericStringEntry dlg;
	const INT Result = dlg.ShowModal( TEXT("NewTransitionTime"), TEXT("Time"), *CurrentTimeStr);
	if( Result != wxID_OK )
		return;

	double dNewTime;
	const UBOOL bIsNumber = dlg.GetStringEntry().GetValue().ToDouble(&dNewTime);
	if(!bIsNumber)
		return;

	const FLOAT NewTime = (FLOAT)dNewTime;

	DirTrack->CutTrack(SelKey.KeyIndex).TransitionTime = NewTime;

	// Update stuff in case doing this has changed it.
	RefreshInterpPosition();
}

void WxInterpEd::OnFlipToggleKey(wxCommandEvent& In)
{
	for (INT KeyIndex = 0; KeyIndex < Opt->SelectedKeys.Num(); KeyIndex++)
	{
		FInterpEdSelKey& SelKey = Opt->SelectedKeys(KeyIndex);
		UInterpTrack* Track = SelKey.Group->InterpTracks(SelKey.TrackIndex);

		UInterpTrackToggle* ToggleTrack = Cast<UInterpTrackToggle>(Track);
		if (ToggleTrack)
		{
			FToggleTrackKey& ToggleKey = ToggleTrack->ToggleTrack(SelKey.KeyIndex);
			ToggleKey.ToggleAction = (ToggleKey.ToggleAction == ETTA_Off) ? ETTA_On : ETTA_Off;
			Track->MarkPackageDirty();
		}

		UInterpTrackVisibility* VisibilityTrack = Cast<UInterpTrackVisibility>(Track);
		if (VisibilityTrack)
		{
			FVisibilityTrackKey& VisibilityKey = VisibilityTrack->VisibilityTrack(SelKey.KeyIndex);
			VisibilityKey.Action = (VisibilityKey.Action == EVTA_Hide) ? EVTA_Show : EVTA_Hide;
			Track->MarkPackageDirty();
		}
	}
}



/** Called when a new key condition is selected in a track keyframe context menu */
void WxInterpEd::OnKeyContext_SetCondition( wxCommandEvent& In )
{
	for (INT KeyIndex = 0; KeyIndex < Opt->SelectedKeys.Num(); KeyIndex++)
	{
		FInterpEdSelKey& SelKey = Opt->SelectedKeys(KeyIndex);
		UInterpTrack* Track = SelKey.Group->InterpTracks(SelKey.TrackIndex);

		UInterpTrackVisibility* VisibilityTrack = Cast<UInterpTrackVisibility>(Track);
		if (VisibilityTrack)
		{
			FVisibilityTrackKey& VisibilityKey = VisibilityTrack->VisibilityTrack(SelKey.KeyIndex);

			switch( In.GetId() )
			{
				case IDM_INTERP_KeyContext_SetCondition_Always:
					VisibilityKey.ActiveCondition = EVTC_Always;
					break;

				case IDM_INTERP_KeyContext_SetCondition_GoreEnabled:
					VisibilityKey.ActiveCondition = EVTC_GoreEnabled;
					break;

				case IDM_INTERP_KeyContext_SetCondition_GoreDisabled:
					VisibilityKey.ActiveCondition = EVTC_GoreDisabled;
					break;
			}

			Track->MarkPackageDirty();
		}
	}
}



void WxInterpEd::OnMenuUndo(wxCommandEvent& In)
{
	InterpEdUndo();
}

void WxInterpEd::OnMenuRedo(wxCommandEvent& In)
{
	InterpEdRedo();
}

/** Menu handler for cut operations. */
void WxInterpEd::OnMenuCut( wxCommandEvent& In )
{
	CopySelectedGroupOrTrack(TRUE);
}

/** Menu handler for copy operations. */
void WxInterpEd::OnMenuCopy( wxCommandEvent& In )
{
	CopySelectedGroupOrTrack(FALSE);
}

/** Menu handler for paste operations. */
void WxInterpEd::OnMenuPaste( wxCommandEvent& In )
{
	PasteSelectedGroupOrTrack();
}

/** Update UI handler for edit menu items. */
void WxInterpEd::OnMenuEdit_UpdateUI( wxUpdateUIEvent& In )
{
	switch(In.GetId())
	{
	case IDM_INTERP_EDIT_UNDO:
		if(InterpEdTrans->CanUndo())
		{
			FString Label = FString::Printf(TEXT("%s %s"), *LocalizeUnrealEd("Undo"), *InterpEdTrans->GetUndoDesc());
			In.SetText(*Label);
			In.Enable(TRUE);
		}
		else
		{
			In.Enable(FALSE);
		}
		break;
	case IDM_INTERP_EDIT_REDO:
		if(InterpEdTrans->CanRedo())
		{
			FString Label = FString::Printf(TEXT("%s %s"), *LocalizeUnrealEd("Redo"), *InterpEdTrans->GetRedoDesc());
			In.SetText(*Label);
			In.Enable(TRUE);
		}
		else
		{
			In.Enable(FALSE);
		}
		break;
	case IDM_INTERP_EDIT_PASTE:
		{
			UBOOL bCanPaste = CanPasteGroupOrTrack();
			In.Enable(bCanPaste==TRUE);
		}
		break;
	}
}

void WxInterpEd::OnMenuImport( wxCommandEvent& )
{
	if( Interp != NULL )
	{
		WxFileDialog ImportFileDialog(this, *LocalizeUnrealEd("ImportMatineeSequence"), *(GApp->LastDir[LD_GENERIC_IMPORT]), TEXT(""), TEXT("COLLADA document|*.dae|All files|*.*"), wxOPEN | wxFILE_MUST_EXIST, wxDefaultPosition);

		// Show dialog and execute the import if the user did not cancel out
		if( ImportFileDialog.ShowModal() == wxID_OK )
		{
			// Get the filename from dialog
			wxString ImportFilename = ImportFileDialog.GetPath();
			FFilename FileName = ImportFilename.c_str();
			GApp->LastDir[LD_GENERIC_IMPORT] = FileName.GetPath(); // Save path as default for next time.
		
			// Import the Matinee information from the COLLADA document.
			UnCollada::CImporter* ColladaImporter = UnCollada::CImporter::GetInstance();
			ColladaImporter->ImportFromFile( ImportFilename.c_str() );
			if (ColladaImporter->IsParsingSuccessful())
			{
				if ( ColladaImporter->HasUnknownCameras( Interp ) )
				{
					// Ask the user whether to create any missing cameras.
					int LResult = wxMessageBox(*LocalizeUnrealEd("ImportMatineeSequence_MissingCameras"), *LocalizeUnrealEd("ImportMatineeSequence"), wxICON_QUESTION | wxYES_NO | wxCENTRE);
					ColladaImporter->SetProcessUnknownCameras(LResult == wxYES);
				}

				// Re-create the Matinee sequence.
				ColladaImporter->ImportMatineeSequence(Interp);

				// We have modified the sequence, so update its UI.
				NotifyPostChange(NULL, NULL);
			}
			else
			{
				GWarn->Log( NAME_Error, ColladaImporter->GetErrorMessage() );
			}
			ColladaImporter->CloseDocument();
		}
	}
}

void WxInterpEd::OnMenuExport( wxCommandEvent& In )
{
	if( Interp != NULL )
	{
		WxFileDialog ExportFileDialog(this, *LocalizeUnrealEd("ExportMatineeSequence"), *(GApp->LastDir[LD_GENERIC_EXPORT]), TEXT(""), TEXT("COLLADA document|*.dae"), wxSAVE | wxOVERWRITE_PROMPT, wxDefaultPosition);

		// Show dialog and execute the import if the user did not cancel out
		if( ExportFileDialog.ShowModal() == wxID_OK )
		{
			// Get the filename from dialog
			wxString ExportFilename = ExportFileDialog.GetPath();
			FFilename FileName = ExportFilename.c_str();
			GApp->LastDir[LD_GENERIC_EXPORT] = FileName.GetPath(); // Save path as default for next time.

			// Export the Matinee information to a COLLADA document.
			UnCollada::CExporter* ColladaExporter = UnCollada::CExporter::GetInstance();
			ColladaExporter->CreateDocument();

			// Export the persistent level and all of it's actors
			ColladaExporter->ExportLevelMesh(GWorld->PersistentLevel, Interp );

			// Export streaming levels and actors
			for( INT CurLevelIndex = 0; CurLevelIndex < GWorld->Levels.Num(); ++CurLevelIndex )
			{
				ULevel* CurLevel = GWorld->Levels( CurLevelIndex );
				if( CurLevel != NULL && CurLevel != GWorld->PersistentLevel )
				{
					ColladaExporter->ExportLevelMesh( CurLevel, Interp );
				}
			}

			// Export Matinee
			ColladaExporter->ExportMatinee( Interp );

			// Save to disk
			ColladaExporter->WriteToFile( ExportFilename.c_str() );
		}
	}
}



void WxInterpEd::OnExportSoundCueInfoCommand( wxCommandEvent& )
{
	if( Interp == NULL )
	{
		return;
	}


	WxFileDialog ExportFileDialog(
		this,
		*LocalizeUnrealEd( "InterpEd_ExportSoundCueInfoDialogTitle" ),
		*(GApp->LastDir[LD_GENERIC_EXPORT]),
		TEXT( "" ),
		TEXT( "CSV file|*.csv" ),
		wxSAVE | wxOVERWRITE_PROMPT,
		wxDefaultPosition );

	// Show dialog and execute the import if the user did not cancel out
	if( ExportFileDialog.ShowModal() == wxID_OK )
	{
		// Get the filename from dialog
		wxString ExportFilename = ExportFileDialog.GetPath();
		FFilename FileName = ExportFilename.c_str();

		// Save path as default for next time.
		GApp->LastDir[ LD_GENERIC_EXPORT ] = FileName.GetPath();

		
		FArchive* CSVFile = GFileManager->CreateFileWriter( *FileName );
		if( CSVFile != NULL )
		{
			// Write header
			{
				FString TextLine( TEXT( "Group,Track,SoundCue,Time,Frame,Anim,AnimTime,AnimFrame" ) LINE_TERMINATOR );
				CSVFile->Serialize( TCHAR_TO_ANSI( *TextLine ), TextLine.Len() );
			}

			for( INT CurGroupIndex = 0; CurGroupIndex < Interp->InterpData->InterpGroups.Num(); ++CurGroupIndex )
			{
				const UInterpGroup* CurGroup = Interp->InterpData->InterpGroups( CurGroupIndex );
				if( CurGroup != NULL )
				{
					for( INT CurTrackIndex = 0; CurTrackIndex < CurGroup->InterpTracks.Num(); ++CurTrackIndex )
					{
						const UInterpTrack* CurTrack = CurGroup->InterpTracks( CurTrackIndex );
						if( CurTrack != NULL )
						{
							const UInterpTrackSound* SoundTrack = ConstCast< UInterpTrackSound >( CurTrack );
							if( SoundTrack != NULL )
							{
								for( INT CurSoundIndex = 0; CurSoundIndex < SoundTrack->Sounds.Num(); ++CurSoundIndex )
								{
									const FSoundTrackKey& CurSound = SoundTrack->Sounds( CurSoundIndex );
									if( CurSound.Sound != NULL )
									{
										FString FoundAnimName;
										FLOAT FoundAnimTime = 0.0f;

										// Search for an animation track in this group that overlaps this sound's start time
										for( TArrayNoInit< UInterpTrack* >::TConstIterator TrackIter( CurGroup->InterpTracks ); TrackIter != NULL; ++TrackIter )
										{
											const UInterpTrackAnimControl* AnimTrack = ConstCast< UInterpTrackAnimControl >( *TrackIter );
											if( AnimTrack != NULL )
											{
												// Iterate over animations in this anim track
												for( TArrayNoInit< FAnimControlTrackKey >::TConstIterator AnimKeyIter( AnimTrack->AnimSeqs ); AnimKeyIter != NULL; ++AnimKeyIter )
												{
													const FAnimControlTrackKey& CurAnimKey = *AnimKeyIter;

													// Does this anim track overlap the sound's start time?
													if( CurSound.Time >= CurAnimKey.StartTime )
													{
														FoundAnimName = CurAnimKey.AnimSeqName.ToString();
														
														// Compute the time the sound exists at within this animation
														FoundAnimTime = ( CurSound.Time - CurAnimKey.StartTime ) + CurAnimKey.AnimStartOffset;

														// NOTE: The array is ordered, so we'll take the LAST anim we find that overlaps the sound!
													}
												}
											}
										}


										// Also store values as frame numbers instead of time values if a frame rate is selected
										const INT SoundFrameIndex = bSnapToFrames ? appTrunc( CurSound.Time / SnapAmount ) : 0;

										FString TextLine = FString::Printf(
											TEXT( "%s,%s,%s,%0.2f,%i" ),
											*CurGroup->GroupName.ToString(),
											*CurTrack->TrackTitle,
											*CurSound.Sound->GetName(),
											CurSound.Time,
											SoundFrameIndex );

										// Did we find an animation that overlaps this sound?  If so, we'll emit that info
										if( FoundAnimName.Len() > 0 )
										{
											// Also store values as frame numbers instead of time values if a frame rate is selected
											const INT AnimFrameIndex = bSnapToFrames ? appTrunc( FoundAnimTime / SnapAmount ) : 0;

											TextLine += FString::Printf(
												TEXT( ",%s,%.2f,%i" ),
												*FoundAnimName,
												FoundAnimTime,
												AnimFrameIndex );
										}

										TextLine += LINE_TERMINATOR;

										CSVFile->Serialize( TCHAR_TO_ANSI( *TextLine ), TextLine.Len() );
									}
								}
							}
						}
					}
				}
			}

			// Close and delete archive
			CSVFile->Close();
			delete CSVFile;
		}
		else
		{
			debugf( NAME_Warning, TEXT("Could not create CSV file %s for writing."), *FileName );
		}
	}
}



void WxInterpEd::OnMenuReduceKeys(wxCommandEvent& In)
{
	ReduceKeys();
}


void WxInterpEd::OnToggleCurveEd(wxCommandEvent& In)
{
	// Check to see if the Curve Editor is currently visible
	UBOOL bCurveEditorVisible = false;

	FDockingParent::FDockWindowState CurveEdDockState;
	if( GetDockingWindowState( CurveEd, CurveEdDockState ) )
	{
		bCurveEditorVisible = CurveEdDockState.bIsVisible;
	}

	// Toggle the curve editor
	if( CurveEdToggleMenuItem != NULL )
	{
		CurveEdToggleMenuItem->Toggle();
	}
	bCurveEditorVisible = !bCurveEditorVisible;

	// Now actually show or hide the window
	ShowDockingWindow( CurveEd, bCurveEditorVisible );

	// Update button status.
	ToolBar->ToggleTool( IDM_INTERP_TOGGLE_CURVEEDITOR, bCurveEditorVisible ? true : false );
	MenuBar->ViewMenu->Check( IDM_INTERP_TOGGLE_CURVEEDITOR, bCurveEditorVisible ? true : false );

	// OK, we need to make sure our track window's scroll bar gets repositioned correctly if the docking
	// layout changes
	if( TrackWindow != NULL )
	{
		TrackWindow->UpdateWindowLayout();
	}
	if( DirectorTrackWindow != NULL )
	{
		DirectorTrackWindow->UpdateWindowLayout();
	}
}



/** Called when sash position changes so we can remember the sash position. */
void WxInterpEd::OnGraphSplitChangePos(wxSplitterEvent& In)
{
	GraphSplitPos = GraphSplitterWnd->GetSashPosition();
}


/**
 * Called when a docking window state has changed
 */
void WxInterpEd::OnWindowDockingLayoutChanged()
{
	// Check to see if the Curve Editor is currently visible
	UBOOL bCurveEditorVisible = false;

	FDockingParent::FDockWindowState CurveEdDockState;
	if( GetDockingWindowState( CurveEd, CurveEdDockState ) )
	{
		bCurveEditorVisible = CurveEdDockState.bIsVisible;
	}

	// Update button status.
	if( ToolBar != NULL )
	{
		ToolBar->ToggleTool( IDM_INTERP_TOGGLE_CURVEEDITOR, bCurveEditorVisible ? true : false );
	}

	if( MenuBar != NULL )
	{
		MenuBar->ViewMenu->Check( IDM_INTERP_TOGGLE_CURVEEDITOR, bCurveEditorVisible ? true : false );
	}

	// OK, we need to make sure our track window's scroll bar gets repositioned correctly if the docking
	// layout changes
	if( TrackWindow != NULL )
	{
		TrackWindow->UpdateWindowLayout();
	}
	if( DirectorTrackWindow != NULL )
	{
		DirectorTrackWindow->UpdateWindowLayout();
	}
}



/** Turn keyframe snap on/off. */
void WxInterpEd::OnToggleSnap(wxCommandEvent& In)
{
	SetSnapEnabled( In.IsChecked() );
}



/** Updates UI state for 'snap keys' option */
void WxInterpEd::OnToggleSnap_UpdateUI( wxUpdateUIEvent& In )
{
	In.Check( bSnapEnabled ? TRUE : FALSE );
}	



/** Called when the 'snap time to frames' command is triggered from the GUI */
void WxInterpEd::OnToggleSnapTimeToFrames( wxCommandEvent& In )
{
	SetSnapTimeToFrames( In.IsChecked() );
}



/** Updates UI state for 'snap time to frames' option */
void WxInterpEd::OnToggleSnapTimeToFrames_UpdateUI( wxUpdateUIEvent& In )
{
	In.Enable( bSnapToFrames ? TRUE : FALSE );
	In.Check( bSnapToFrames && bSnapTimeToFrames );
}



/** Called when the 'fixed time step playback' command is triggered from the GUI */
void WxInterpEd::OnFixedTimeStepPlaybackCommand( wxCommandEvent& In )
{
	SetFixedTimeStepPlayback( In.IsChecked() );
}



/** Updates UI state for 'fixed time step playback' option */
void WxInterpEd::OnFixedTimeStepPlaybackCommand_UpdateUI( wxUpdateUIEvent& In )
{
	In.Enable( bSnapToFrames ? TRUE : FALSE );
	In.Check( bSnapToFrames && bFixedTimeStepPlayback );
}



/** Called when the 'prefer frame numbers' command is triggered from the GUI */
void WxInterpEd::OnPreferFrameNumbersCommand( wxCommandEvent& In )
{
	SetPreferFrameNumbers( In.IsChecked() );
}



/** Updates UI state for 'prefer frame numbers' option */
void WxInterpEd::OnPreferFrameNumbersCommand_UpdateUI( wxUpdateUIEvent& In )
{
	In.Enable( bSnapToFrames ? TRUE : FALSE );
	In.Check( bSnapToFrames && bPreferFrameNumbers );
}



/** Called when the 'show time cursor pos for all keys' command is triggered from the GUI */
void WxInterpEd::OnShowTimeCursorPosForAllKeysCommand( wxCommandEvent& In )
{
	SetShowTimeCursorPosForAllKeys( In.IsChecked() );
}



/** Updates UI state for 'show time cursor pos for all keys' option */
void WxInterpEd::OnShowTimeCursorPosForAllKeysCommand_UpdateUI( wxUpdateUIEvent& In )
{
	In.Enable( TRUE );
	In.Check( bShowTimeCursorPosForAllKeys ? TRUE : FALSE );
}



/** The snap resolution combo box was changed. */
void WxInterpEd::OnChangeSnapSize(wxCommandEvent& In)
{
	const INT NewSelection = In.GetInt();
	check(NewSelection >= 0 && NewSelection <= ARRAY_COUNT(InterpEdSnapSizes)+ARRAY_COUNT(InterpEdFPSSnapSizes));

	if(NewSelection == ARRAY_COUNT(InterpEdSnapSizes)+ARRAY_COUNT(InterpEdFPSSnapSizes))
	{
		bSnapToFrames = FALSE;
		bSnapToKeys = TRUE;
		SnapAmount = 1.0f / 30.0f;	// Shouldn't be used
		CurveEd->SetInSnap(FALSE, SnapAmount, bSnapToFrames);
	}
	else if(NewSelection<ARRAY_COUNT(InterpEdSnapSizes))	// see if they picked a second snap amount
	{
		bSnapToFrames = FALSE;
		bSnapToKeys = FALSE;
		SnapAmount = InterpEdSnapSizes[NewSelection];
		CurveEd->SetInSnap(bSnapEnabled, SnapAmount, bSnapToFrames);
	}
	else if(NewSelection<ARRAY_COUNT(InterpEdFPSSnapSizes)+ARRAY_COUNT(InterpEdSnapSizes))	// See if they picked a FPS snap amount.
	{
		bSnapToFrames = TRUE;
		bSnapToKeys = FALSE;
		SnapAmount = InterpEdFPSSnapSizes[NewSelection-ARRAY_COUNT(InterpEdSnapSizes)];
		CurveEd->SetInSnap(bSnapEnabled, SnapAmount, bSnapToFrames);
	}

	// Enable or disable the 'Snap Time To Frames' button based on whether or not we have a 'frame rate' selected
	ToolBar->EnableTool( IDM_INTERP_TOGGLE_SNAP_TIME_TO_FRAMES, bSnapToFrames ? true : false );

	// Save selected snap mode to INI.
	GConfig->SetInt(TEXT("Matinee"), TEXT("SelectedSnapMode"), NewSelection, GEditorUserSettingsIni );

	// Snap time to frames right now if we need to
	SetSnapTimeToFrames( bSnapTimeToFrames );

	// If 'fixed time step playback' is turned on, we also need to make sure the benchmarking time step
	// is set when this changes
	SetFixedTimeStepPlayback( bFixedTimeStepPlayback );

	// The 'prefer frame numbers' option requires bSnapToFrames to be enabled, so update it's state
	SetPreferFrameNumbers( bPreferFrameNumbers );

	// Make sure any particle replay tracks are filled in with the correct state
	UpdateParticleReplayTracks();
}



/**
 * Called when the initial curve interpolation mode for newly created keys is changed
 */
void WxInterpEd::OnChangeInitialInterpMode( wxCommandEvent& In )
{
	const INT NewSelection = In.GetInt();

	// Store new interp mode
	InitialInterpMode = ( EInterpCurveMode )NewSelection;
	check( InitialInterpMode >= 0 && InitialInterpMode < CIM_Unknown );

	// Save selected mode to user's preference file
	GConfig->SetInt( TEXT( "Matinee" ), TEXT( "InitialInterpMode2" ), NewSelection, GEditorUserSettingsIni );
}



/** Adjust the view so the entire sequence fits into the viewport. */
void WxInterpEd::OnViewFitSequence(wxCommandEvent& In)
{
	ViewFitSequence();
}

/** Adjust the view so the selected keys fit into the viewport. */
void WxInterpEd::OnViewFitToSelected(wxCommandEvent& In)
{
	ViewFitToSelected();
}

/** Adjust the view so the looped section fits into the viewport. */
void WxInterpEd::OnViewFitLoop(wxCommandEvent& In)
{
	ViewFitLoop();
}

/** Adjust the view so the looped section fits into the entire sequence. */
void WxInterpEd::OnViewFitLoopSequence(wxCommandEvent& In)
{
	ViewFitLoopSequence();
}

/*-----------------------------------------------------------------------------
	WxInterpEdToolBar
-----------------------------------------------------------------------------*/

WxInterpEdToolBar::WxInterpEdToolBar( wxWindow* InParent, wxWindowID InID )
: wxToolBar( InParent, InID, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_3DBUTTONS )
{
	AddB.Load( TEXT("MAT_AddKey") );
	PlayReverseB.Load( TEXT("MAT_PlayReverse") );
	PlayB.Load( TEXT("MAT_Play") );
	LoopSectionB.Load( TEXT("MAT_PlayLoopSection") );
	StopB.Load( TEXT("MAT_Stop") );
	UndoB.Load( TEXT("MAT_Undo") );
	RedoB.Load( TEXT("MAT_Redo") );
	CurveEdB.Load( TEXT("MAT_CurveEd") );
	SnapB.Load( TEXT("MAT_ToggleSnap") );
	FitSequenceB.Load( TEXT("MAT_FitSequence") );
	FitToSelectedB.Load( TEXT( "MAT_FitViewToSelected" ) );
	FitLoopB.Load( TEXT("MAT_FitLoop") );
	FitLoopSequenceB.Load( TEXT("MAT_FitLoopSequence") );

	Speed1B.Load(TEXT("CASC_Speed_1"));
	Speed10B.Load(TEXT("CASC_Speed_10"));
	Speed25B.Load(TEXT("CASC_Speed_25"));
	Speed50B.Load(TEXT("CASC_Speed_50"));
	Speed100B.Load(TEXT("CASC_Speed_100"));
	SnapTimeToFramesB.Load( TEXT( "MAT_SnapTimeToFrames" ) );
	FixedTimeStepPlaybackB.Load( TEXT( "MAT_FixedTimeStepPlayback" ) );
	GorePreviewB.Load( TEXT( "MAT_GorePreview" ) );

	SetToolBitmapSize( wxSize( 18, 18 ) );

	AddTool( IDM_INTERP_ADDKEY, AddB, *LocalizeUnrealEd("AddKey") );

	// Create combo box that allows the user to select the initial curve interpolation mode for newly created keys
	{
		InitialInterpModeComboBox = new WxComboBox( this, IDM_INTERP_InitialInterpMode_ComboBox, TEXT(""), wxDefaultPosition, wxSize(140, -1), 0, NULL, wxCB_READONLY );

		InitialInterpModeComboBox->SetToolTip( *LocalizeUnrealEd( "InterpEd_InitialInterpModeComboBox_Desc" ) );

		// NOTE: These must be in the same order as the definitions in UnMath.h for EInterpCurveMode
		InitialInterpModeComboBox->Append( *LocalizeUnrealEd( "Linear" ) );                   // CIM_Linear
		InitialInterpModeComboBox->Append( *LocalizeUnrealEd( "CurveAuto" ) );                // CIM_CurveAuto
		InitialInterpModeComboBox->Append( *LocalizeUnrealEd( "Constant" ) );                 // CIM_Constant
		InitialInterpModeComboBox->Append( *LocalizeUnrealEd( "CurveUser" ) );                // CIM_CurveUser
		InitialInterpModeComboBox->Append( *LocalizeUnrealEd( "CurveBreak" ) );               // CIM_CurveBreak
		InitialInterpModeComboBox->Append( *LocalizeUnrealEd( "CurveAutoClamped" ) );         // CIM_CurveAutoClamped

		AddControl( InitialInterpModeComboBox );
	}

	AddSeparator();

	AddTool( IDM_INTERP_PLAY, PlayB, *LocalizeUnrealEd("Play") );
	AddTool( IDM_INTERP_PLAYLOOPSECTION, LoopSectionB, *LocalizeUnrealEd("LoopSection") );
	AddTool( IDM_INTERP_STOP, StopB, *LocalizeUnrealEd("Stop") );
	AddTool( IDM_INTERP_PlayReverse, PlayReverseB, *LocalizeUnrealEd( "InterpEd_ToolBar_PlayReverse_Desc" ) );

	AddSeparator();

	AddRadioTool(IDM_INTERP_SPEED_100,	*LocalizeUnrealEd("FullSpeed"), Speed100B, wxNullBitmap, *LocalizeUnrealEd("FullSpeed") );
	AddRadioTool(IDM_INTERP_SPEED_50,	*LocalizeUnrealEd("50Speed"), Speed50B, wxNullBitmap, *LocalizeUnrealEd("50Speed") );
	AddRadioTool(IDM_INTERP_SPEED_25,	*LocalizeUnrealEd("25Speed"), Speed25B, wxNullBitmap, *LocalizeUnrealEd("25Speed") );
	AddRadioTool(IDM_INTERP_SPEED_10,	*LocalizeUnrealEd("10Speed"), Speed10B, wxNullBitmap, *LocalizeUnrealEd("10Speed") );
	AddRadioTool(IDM_INTERP_SPEED_1,	*LocalizeUnrealEd("1Speed"), Speed1B, wxNullBitmap, *LocalizeUnrealEd("1Speed") );
	ToggleTool(IDM_INTERP_SPEED_100, TRUE);

	AddSeparator();

	AddTool( IDM_INTERP_EDIT_UNDO, UndoB, *LocalizeUnrealEd("Undo") );
	AddTool( IDM_INTERP_EDIT_REDO, RedoB, *LocalizeUnrealEd("Redo") );

	AddSeparator();

	AddCheckTool( IDM_INTERP_TOGGLE_CURVEEDITOR, *LocalizeUnrealEd("ToggleCurveEditor"), CurveEdB, wxNullBitmap, *LocalizeUnrealEd("ToggleCurveEditor") );

	AddSeparator();

	AddCheckTool( IDM_INTERP_TOGGLE_SNAP, *LocalizeUnrealEd("ToggleSnap"), SnapB, wxNullBitmap, *LocalizeUnrealEd("ToggleSnap") );
	AddCheckTool( IDM_INTERP_TOGGLE_SNAP_TIME_TO_FRAMES, *LocalizeUnrealEd( "InterpEd_ToggleSnapTimeToFrames_Desc" ), SnapTimeToFramesB, wxNullBitmap, *LocalizeUnrealEd("InterpEd_ToggleSnapTimeToFrames_Desc") );
	AddCheckTool( IDM_INTERP_FixedTimeStepPlayback, *LocalizeUnrealEd( "InterpEd_FixedTimeStepPlayback_Desc" ), FixedTimeStepPlaybackB, wxNullBitmap, *LocalizeUnrealEd("InterpEd_FixedTimeStepPlayback_Desc") );
	
	// Create snap-size combo
	{
		SnapCombo = new WxComboBox( this, IDM_INTERP_SNAPCOMBO, TEXT(""), wxDefaultPosition, wxSize(110, -1), 0, NULL, wxCB_READONLY );

		SnapCombo->SetToolTip( *LocalizeUnrealEd( "InterpEd_SnapComboBox_Desc" ) );

		// Append Second Snap Times
		for(INT i=0; i<ARRAY_COUNT(InterpEdSnapSizes); i++)
		{
			FString SnapCaption = FString::Printf( TEXT("%1.2f"), InterpEdSnapSizes[i] );
			SnapCombo->Append( *SnapCaption );
		}

		// Append FPS Snap Times
		for(INT i=0; i<ARRAY_COUNT(InterpEdFPSSnapSizes); i++)
		{
			FString SnapCaption = LocalizeUnrealEd( InterpEdFPSSnapSizeLocNames[ i ] );
			SnapCombo->Append( *SnapCaption );
		}

		SnapCombo->Append( *LocalizeUnrealEd( TEXT("InterpEd_Snap_Keys") ) ); // Add option for snapping to other keys.
		SnapCombo->SetSelection(2);
	}

	AddControl(SnapCombo);

	AddSeparator();
	AddTool( IDM_INTERP_VIEW_FITSEQUENCE, FitSequenceB, *LocalizeUnrealEd("ViewFitSequence") );
	AddTool( IDM_INTERP_VIEW_FitViewToSelected, FitToSelectedB, *LocalizeUnrealEd("ViewFitToSelected") );
	AddTool( IDM_INTERP_VIEW_FITLOOP, FitLoopB, *LocalizeUnrealEd("ViewFitLoop") );
	AddTool( IDM_INTERP_VIEW_FITLOOPSEQUENCE, FitLoopSequenceB, *LocalizeUnrealEd("ViewFitLoopSequence") );

	AddSeparator();
	AddCheckTool( IDM_INTERP_ToggleGorePreview, *LocalizeUnrealEd( "InterpEd_ToggleGorePreview"), GorePreviewB, wxNullBitmap, *LocalizeUnrealEd( "InterpEd_ToggleGorePreview" ) );

	Realize();
}

WxInterpEdToolBar::~WxInterpEdToolBar()
{
}


/*-----------------------------------------------------------------------------
	WxInterpEdMenuBar
-----------------------------------------------------------------------------*/

WxInterpEdMenuBar::WxInterpEdMenuBar(WxInterpEd* InEditor)
{
	FileMenu = new wxMenu();
	Append( FileMenu, *LocalizeUnrealEd("File") );

	FileMenu->Append( IDM_INTERP_FILE_IMPORT, *LocalizeUnrealEd("InterpEd_FileMenu_Import"), TEXT("") );
	FileMenu->Append( IDM_INTERP_FILE_EXPORT, *LocalizeUnrealEd("InterpEd_FileMenu_ExportAll"), TEXT("") );
	FileMenu->Append( IDM_INTERP_ExportSoundCueInfo, *LocalizeUnrealEd( "InterpEd_FileMenu_ExportSoundCueInfo" ), TEXT("") );

	EditMenu = new wxMenu();
	Append( EditMenu, *LocalizeUnrealEd("Edit") );

	EditMenu->Append( IDM_INTERP_EDIT_UNDO, *LocalizeUnrealEd("Undo"), TEXT("") );
	EditMenu->Append( IDM_INTERP_EDIT_REDO, *LocalizeUnrealEd("Redo"), TEXT("") );
	EditMenu->AppendSeparator();
	EditMenu->Append( IDM_INTERP_DeleteSelectedKeys, *LocalizeUnrealEd( "InterpEd_EditMenu_DeleteSelectedKeys" ), TEXT("") );
	EditMenu->Append( IDM_INTERP_EDIT_DUPLICATEKEYS, *LocalizeUnrealEd("DuplicateSelectedKeys"), TEXT("") );
	EditMenu->AppendSeparator();
	EditMenu->Append( IDM_INTERP_EDIT_INSERTSPACE, *LocalizeUnrealEd("InsertSpaceCurrent"), TEXT("") );
	EditMenu->Append( IDM_INTERP_EDIT_STRETCHSECTION, *LocalizeUnrealEd("StretchSection"), TEXT("") );
	EditMenu->Append( IDM_INTERP_EDIT_DELETESECTION, *LocalizeUnrealEd("DeleteSection"), TEXT("") );
	EditMenu->Append( IDM_INTERP_EDIT_SELECTINSECTION, *LocalizeUnrealEd("SelectKeysSection"), TEXT("") );
	EditMenu->AppendSeparator();
	EditMenu->Append( IDM_INTERP_EDIT_REDUCEKEYS, *LocalizeUnrealEd("ReduceKeys"), TEXT("") );
	EditMenu->AppendSeparator();
	EditMenu->Append( IDM_INTERP_EDIT_SAVEPATHTIME, *LocalizeUnrealEd("SavePathBuildingPositions"), TEXT("") );
	EditMenu->Append( IDM_INTERP_EDIT_JUMPTOPATHTIME, *LocalizeUnrealEd("JumpPathBuildingPositions"), TEXT("") );
	EditMenu->AppendSeparator();
	EditMenu->Append( IDM_OPEN_BINDKEYS_DIALOG,	*LocalizeUnrealEd("BindEditorHotkeys"), TEXT("") );

	ViewMenu = new wxMenu();
	Append( ViewMenu, *LocalizeUnrealEd("View") );

	ViewMenu->AppendCheckItem( IDM_INTERP_VIEW_Draw3DTrajectories, *LocalizeUnrealEd("InterpEd_ViewMenu_Show3DTrajectories"), TEXT("") );
	bool bShowTrack = (InEditor->bHide3DTrackView == FALSE);
	ViewMenu->Check( IDM_INTERP_VIEW_Draw3DTrajectories, bShowTrack );
	ViewMenu->Append( IDM_INTERP_VIEW_ShowAll3DTrajectories, *LocalizeUnrealEd( "InterpEd_MovementTrackContext_ShowAll3DTracjectories" ), TEXT( "" ) );
	ViewMenu->Append( IDM_INTERP_VIEW_HideAll3DTrajectories, *LocalizeUnrealEd( "InterpEd_MovementTrackContext_HideAll3DTracjectories" ), TEXT( "" ) );
	ViewMenu->AppendSeparator();

	ViewMenu->AppendCheckItem( IDM_INTERP_TOGGLE_SNAP, *LocalizeUnrealEd( "InterpEd_ViewMenu_ToggleSnap" ), TEXT( "" ) );
	ViewMenu->AppendCheckItem( IDM_INTERP_TOGGLE_SNAP_TIME_TO_FRAMES, *LocalizeUnrealEd( "InterpEd_ViewMenu_SnapTimeToFrames" ), TEXT( "" ) );
	ViewMenu->AppendCheckItem( IDM_INTERP_FixedTimeStepPlayback, *LocalizeUnrealEd( "InterpEd_ViewMenu_FixedTimeStepPlayback" ), TEXT( "" ) );
	ViewMenu->AppendCheckItem( IDM_INTERP_PreferFrameNumbers, *LocalizeUnrealEd( "InterpEd_ViewMenu_PreferFrameNumbers" ), TEXT( "" ) );
	ViewMenu->AppendCheckItem( IDM_INTERP_ShowTimeCursorPosForAllKeys, *LocalizeUnrealEd( "InterpEd_ViewMenu_ShowTimeCursorPosForAllKeys" ), TEXT( "" ) );
	ViewMenu->AppendSeparator();

	ViewMenu->AppendCheckItem( IDM_INTERP_VIEW_ZoomToTimeCursorPosition, *LocalizeUnrealEd("InterpEd_ViewMenu_ZoomToTimeCursorPosition"), TEXT("") );
	bool bZoomToScub = (InEditor->bZoomToScrubPos == TRUE);
	ViewMenu->Check( IDM_INTERP_VIEW_ZoomToTimeCursorPosition, bZoomToScub );

	ViewMenu->AppendCheckItem( IDM_INTERP_VIEW_ViewportFrameStats, *LocalizeUnrealEd("InterpEd_ViewMenu_ViewportFrameStats"), TEXT("") );
	ViewMenu->Check( IDM_INTERP_VIEW_ViewportFrameStats, InEditor->IsViewportFrameStatsEnabled() == TRUE );

	ViewMenu->AppendSeparator();
	ViewMenu->Append( IDM_INTERP_VIEW_FITSEQUENCE,	*LocalizeUnrealEd("ViewFitSequence"), TEXT(""));
	ViewMenu->Append( IDM_INTERP_VIEW_FitViewToSelected, *LocalizeUnrealEd( "ViewFitToSelected" ), TEXT( "" ) );
	ViewMenu->Append( IDM_INTERP_VIEW_FITLOOP,	*LocalizeUnrealEd("ViewFitLoop"), TEXT(""));
	ViewMenu->Append( IDM_INTERP_VIEW_FITLOOPSEQUENCE,	*LocalizeUnrealEd("ViewFitLoopSequence"), TEXT(""));

	ViewMenu->AppendSeparator();
	ViewMenu->AppendCheckItem( IDM_INTERP_ToggleGorePreview, *LocalizeUnrealEd( "InterpEd_ToggleGorePreview"), TEXT( "" ) );

	ViewMenu->AppendSeparator();
	ViewMenu->AppendCheckItem( IDM_INTERP_TOGGLE_CURVEEDITOR,	*LocalizeUnrealEd("ToggleCurveEditor"), TEXT(""));

	// Check to see if the Curve Editor is currently visible
	UBOOL bCurveEditorVisible = false;

	FDockingParent::FDockWindowState CurveEdDockState;
	if( InEditor->GetDockingWindowState( InEditor->CurveEd, CurveEdDockState ) )
	{
		bCurveEditorVisible = CurveEdDockState.bIsVisible;
	}

	// Update button state
	ViewMenu->Check( IDM_INTERP_TOGGLE_CURVEEDITOR, bCurveEditorVisible ? true : false );
}

WxInterpEdMenuBar::~WxInterpEdMenuBar()
{

}


/*-----------------------------------------------------------------------------
	WxMBInterpEdTabMenu
-----------------------------------------------------------------------------*/

WxMBInterpEdTabMenu::WxMBInterpEdTabMenu(WxInterpEd* InterpEd)
{
	UInterpFilter_Custom* Filter = Cast<UInterpFilter_Custom>(InterpEd->IData->SelectedFilter);
	if(Filter != NULL)
	{
		// make sure this isn't a default filter.
		if(InterpEd->IData->InterpFilters.ContainsItem(Filter))
		{
			Append(IDM_INTERP_GROUP_DELETETAB, *LocalizeUnrealEd("DeleteGroupTab"));
		}
	}
}

WxMBInterpEdTabMenu::~WxMBInterpEdTabMenu()
{

}


/*-----------------------------------------------------------------------------
	WxMBInterpEdGroupMenu
-----------------------------------------------------------------------------*/

WxMBInterpEdGroupMenu::WxMBInterpEdGroupMenu(WxInterpEd* InterpEd)
{
	// Nothing to do if no group selected
	if(!InterpEd->ActiveGroup)
		return;

	const UBOOL bIsFolder = InterpEd->ActiveGroup->bIsFolder;
	const UBOOL bIsDirGroup = InterpEd->ActiveGroup->IsA(UInterpGroupDirector::StaticClass());

	if( !bIsFolder )
	{
		for(INT i=0; i<InterpEd->InterpTrackClasses.Num(); i++)
		{
			UInterpTrack* DefTrack = (UInterpTrack*)InterpEd->InterpTrackClasses(i)->GetDefaultObject();
			if(!DefTrack->bDirGroupOnly)
			{
				FString NewTrackString = FString::Printf( LocalizeSecure(LocalizeUnrealEd("AddNew_F"), *InterpEd->InterpTrackClasses(i)->GetDescription()) );
				Append( IDM_INTERP_NEW_TRACK_START+i, *NewTrackString, TEXT("") );
			}
		}
	
		AppendSeparator();
	}


	// Add Director-group specific tracks to separate menu underneath.
	if(bIsDirGroup)
	{
		for(INT i=0; i<InterpEd->InterpTrackClasses.Num(); i++)
		{
			UInterpTrack* DefTrack = (UInterpTrack*)InterpEd->InterpTrackClasses(i)->GetDefaultObject();
			if(DefTrack->bDirGroupOnly)
			{
				FString NewTrackString = FString::Printf( LocalizeSecure(LocalizeUnrealEd("AddNew_F"), *InterpEd->InterpTrackClasses(i)->GetDescription()) );
				Append( IDM_INTERP_NEW_TRACK_START+i, *NewTrackString, TEXT("") );
			}
		}

		AppendSeparator();
	}

	// Add CameraAnim export option if appropriate
	if (!bIsDirGroup && !bIsFolder)
	{
		UInterpGroupInst* GrInst = InterpEd->Interp->FindFirstGroupInst(InterpEd->ActiveGroup);
		check(GrInst);
		if (GrInst)
		{
			AActor* const GroupActor = GrInst->GetGroupActor();
			UBOOL bControllingACameraActor = GroupActor && GroupActor->IsA(ACameraActor::StaticClass());
			if (bControllingACameraActor)
			{
				// add strings to unrealed.int
				Append(IDM_INTERP_CAMERA_ANIM_EXPORT, *LocalizeUnrealEd("ExportCameraAnim"), *LocalizeUnrealEd("ExportCameraAnim_Desc"));
				AppendSeparator();
			}
		}
	}

	// Copy/Paste not supported on folders yet
	if( !bIsFolder )
	{
		Append (IDM_INTERP_EDIT_CUT, *LocalizeUnrealEd("CutGroup"), *LocalizeUnrealEd("CutGroup_Desc"));
		Append (IDM_INTERP_EDIT_COPY, *LocalizeUnrealEd("CopyGroup"), *LocalizeUnrealEd("CopyGroup_Desc"));
		Append (IDM_INTERP_EDIT_PASTE, *LocalizeUnrealEd("PasteGroupOrTrack"), *LocalizeUnrealEd("PasteGroupOrTrack_Desc"));

		AppendSeparator();
	}

	Append( IDM_INTERP_GROUP_RENAME, *LocalizeUnrealEd( bIsFolder ? TEXT( "RenameFolder" ) : TEXT( "RenameGroup" ) ), TEXT("") );

	// Cannot duplicate Director groups or folders
	if(!bIsDirGroup && !bIsFolder)
	{
		Append( IDM_INTERP_GROUP_DUPLICATE, *LocalizeUnrealEd("DuplicateGroup"), TEXT("") );
	}

	Append( IDM_INTERP_GROUP_DELETE, *LocalizeUnrealEd( bIsFolder ? TEXT( "DeleteFolder" ) : TEXT( "DeleteGroup" ) ), TEXT("") );


	UBOOL bNeedSeparator = TRUE;

	// Director groups aren't allowed to be parented to other group folders
	if( !bIsDirGroup )
	{
		// Grab the selected group's parent group
		const UInterpGroup* MyParentGroupFolder = InterpEd->FindParentGroupFolder( InterpEd->ActiveGroup );

		// Count the selected group's children
		INT MyGroupChildCount = InterpEd->CountGroupFolderChildren( InterpEd->ActiveGroup );

		// These variables will be allocated on demand, later
		wxMenu* PotentialParentFoldersMenu = NULL;
		wxMenu* PotentialChildGroupsMenu = NULL;

		for( INT CurGroupIndex = 0; CurGroupIndex < InterpEd->IData->InterpGroups.Num(); ++CurGroupIndex )
		{
			UInterpGroup* const CurGroup = InterpEd->IData->InterpGroups( CurGroupIndex );

			// Grab the parent group for this group, if it has one
			UInterpGroup* CurGroupParentGroupFolder = InterpEd->FindParentGroupFolder( CurGroup );

			// Obviously, we don't want to include ourselves in the list!  We can't become our own child!
			if( CurGroup != InterpEd->ActiveGroup )
			{
				// There's no point allowing us to be re-parented to the same group!
				if( ( MyParentGroupFolder == NULL && CurGroupParentGroupFolder == NULL ) ||
					  ( MyParentGroupFolder != CurGroupParentGroupFolder ) )
				{
					// Director groups aren't allowed to contain other groups (only tracks!)
					if( !CurGroup->IsA( UInterpGroupDirector::StaticClass() ) )
					{
						// We can't allow the user to reparent groups that already have children, since we currently don't
						// support multi-level nesting
						if( MyGroupChildCount == 0 )
						{
							// Child groups currently aren't allowed to contain other groups
							if( !bIsFolder && CurGroup->bIsFolder && !CurGroup->bIsParented )
							{
								// Don't allow us to be reparented to the group we're already parented to!
								if( MyParentGroupFolder != CurGroup )
								{
									// @todo: If more than 1000 groups exist in the data set, this limit will start to cause us problems
									const INT MaxAllowedGroupIndex =
										( IDM_INTERP_GROUP_MoveActiveGroupToFolder_End - IDM_INTERP_GROUP_MoveActiveGroupToFolder_Start );

									if( CurGroupIndex <= MaxAllowedGroupIndex )
									{
										// Construct on demand!
										if( PotentialParentFoldersMenu == NULL )
										{
											PotentialParentFoldersMenu = new wxMenu();
										}

										// OK, this is a candidate parent folder!
										PotentialParentFoldersMenu->Append( IDM_INTERP_GROUP_MoveActiveGroupToFolder_Start + CurGroupIndex, *CurGroup->GroupName.GetNameString() );
									}
									else
									{
										// We've run out of space in the sub menu (no more resource IDs!).  Oh well, we won't display these items.
									}
								}
							}
						}

						// If the selected group is allowed to have children, then we'll search for those too!
						if( bIsFolder && !CurGroup->bIsFolder && MyParentGroupFolder == NULL )
						{
							// Skip existing child groups
							if( CurGroupParentGroupFolder != InterpEd->ActiveGroup )
							{
								// If the current group has no children, then it's a candidate for adding to our folder
								INT NumChildGroups = InterpEd->CountGroupFolderChildren( CurGroup );
								if( NumChildGroups == 0 )
								{
									// @todo: If more than 1000 groups exist in the data set, this limit will start to cause us problems
									const INT MaxAllowedGroupIndex =
										( IDM_INTERP_GROUP_MoveGroupToActiveFolder_End - IDM_INTERP_GROUP_MoveGroupToActiveFolder_Start );

									if( CurGroupIndex <= MaxAllowedGroupIndex )
									{
										// Construct on demand!
										if( PotentialChildGroupsMenu == NULL )
										{
											PotentialChildGroupsMenu = new wxMenu();
										}

										// OK, this is a candidate child group!
										PotentialChildGroupsMenu->Append( IDM_INTERP_GROUP_MoveGroupToActiveFolder_Start + CurGroupIndex, *CurGroup->GroupName.GetNameString() );
									}
									else
									{
										// We've run out of space in the sub menu (no more resource IDs!).  Oh well, we won't display these items.
									}
								}
							}
						}
					}
				}
			}
		}

		UBOOL bAddedFolderMenuItem = FALSE;
		if( PotentialParentFoldersMenu != NULL )
		{
			if( bNeedSeparator )
			{
				AppendSeparator();
				bNeedSeparator = FALSE;
			}
			Append( IDM_INTERP_GROUP_MoveActiveGroupToFolder_SubMenu, *LocalizeUnrealEd( "InterpEd_MoveActiveGroupToFolder" ), PotentialParentFoldersMenu );
			bAddedFolderMenuItem = TRUE;
		}

		if( PotentialChildGroupsMenu != NULL )
		{
			if( bNeedSeparator )
			{
				AppendSeparator();
				bNeedSeparator = FALSE;
			}
			Append( IDM_INTERP_GROUP_MoveGroupToActiveFolder_SubMenu, *LocalizeUnrealEd( "InterpEd_MoveGroupToActiveFolder" ), PotentialChildGroupsMenu );
			bAddedFolderMenuItem = TRUE;
		}

		// If the group is parented, then add an option to remove it from the group folder its in
		if( InterpEd->ActiveGroup->bIsParented )
		{
			if( bNeedSeparator )
			{
				AppendSeparator();
				bNeedSeparator = FALSE;
			}
			Append( IDM_INTERP_GROUP_RemoveFromGroupFolder, *LocalizeUnrealEd( "InterpEd_RemoveFromGroupFolder" ) );
			bAddedFolderMenuItem = TRUE;
		}

		if( bAddedFolderMenuItem )
		{
			bNeedSeparator = TRUE;
		}
	}


	if( !bIsFolder )
	{
		if( bNeedSeparator )
		{
			AppendSeparator();
			bNeedSeparator = FALSE;
		}

		// Add entries for creating and sending to tabs.
		Append(IDM_INTERP_GROUP_CREATETAB, *LocalizeUnrealEd("CreateNewGroupTab"));

		// See if the user can remove this group from the current tab.
		UInterpFilter* Filter = Cast<UInterpFilter_Custom>(InterpEd->IData->SelectedFilter);
		if(Filter != NULL && InterpEd->ActiveGroup != NULL && InterpEd->IData->InterpFilters.ContainsItem(Filter))
		{
			Append(IDM_INTERP_GROUP_REMOVEFROMTAB, *LocalizeUnrealEd("RemoveFromGroupTab"));
		}

		if(InterpEd->Interp->InterpData->InterpFilters.Num())
		{
			wxMenu* TabMenu = new wxMenu();
			for(INT FilterIdx=0; FilterIdx<InterpEd->IData->InterpFilters.Num(); FilterIdx++)
			{
				UInterpFilter* Filter = InterpEd->IData->InterpFilters(FilterIdx);
				TabMenu->Append(IDM_INTERP_GROUP_SENDTOTAB_START+FilterIdx, *Filter->Caption);
			}

			Append(IDM_INTERP_GROUP_SENDTOTAB_SUBMENU, *LocalizeUnrealEd("SendToGroupTab"), TabMenu);
		}
	}

}

WxMBInterpEdGroupMenu::~WxMBInterpEdGroupMenu()
{

}

/*-----------------------------------------------------------------------------
	WxMBInterpEdTrackMenu
-----------------------------------------------------------------------------*/

WxMBInterpEdTrackMenu::WxMBInterpEdTrackMenu(WxInterpEd* InterpEd)
{
	if(InterpEd->ActiveTrackIndex == INDEX_NONE)
		return;


	Append (IDM_INTERP_EDIT_CUT, *LocalizeUnrealEd("CutTrack"), *LocalizeUnrealEd("CutTrack_Desc"));
	Append (IDM_INTERP_EDIT_COPY, *LocalizeUnrealEd("CopyTrack"), *LocalizeUnrealEd("CopyTrack_Desc"));
	Append (IDM_INTERP_EDIT_PASTE, *LocalizeUnrealEd("PasteGroupOrTrack"), *LocalizeUnrealEd("PasteGroupOrTrack_Desc"));

	AppendSeparator();

	Append( IDM_INTERP_TRACK_RENAME, *LocalizeUnrealEd("RenameTrack"), TEXT("") );
	Append( IDM_INTERP_TRACK_DELETE, *LocalizeUnrealEd("DeleteTrack"), TEXT("") );

	check(InterpEd->ActiveGroup);
	UInterpTrack* Track = InterpEd->ActiveGroup->InterpTracks( InterpEd->ActiveTrackIndex );

	UInterpTrackMove* MoveTrack = Cast<UInterpTrackMove>(Track);
	if( MoveTrack )
	{
		AppendSeparator();

		// Trajectory settings for movement tracks
		AppendCheckItem( IDM_INTERP_TRACK_Show3DTrajectory, *LocalizeUnrealEd( "InterpEd_MovementTrackContext_Show3DTracjectory" ), TEXT( "" ) );
		Check( IDM_INTERP_TRACK_Show3DTrajectory, MoveTrack->bHide3DTrack == FALSE );
		Append( IDM_INTERP_VIEW_ShowAll3DTrajectories, *LocalizeUnrealEd( "InterpEd_MovementTrackContext_ShowAll3DTracjectories" ), TEXT( "" ) );
		Append( IDM_INTERP_VIEW_HideAll3DTrajectories, *LocalizeUnrealEd( "InterpEd_MovementTrackContext_HideAll3DTracjectories" ), TEXT( "" ) );

		AppendSeparator();

		AppendCheckItem( IDM_INTERP_TRACK_FRAME_WORLD, *LocalizeUnrealEd("WorldFrame"), TEXT("") );
		AppendCheckItem( IDM_INTERP_TRACK_FRAME_RELINITIAL, *LocalizeUnrealEd("RelativeInitial"), TEXT("") );

		// Check the currently the selected movement frame
		if( MoveTrack->MoveFrame == IMF_World )
		{
			Check(IDM_INTERP_TRACK_FRAME_WORLD, TRUE);
		}
		else if( MoveTrack->MoveFrame == IMF_RelativeToInitial )
		{
			Check(IDM_INTERP_TRACK_FRAME_RELINITIAL, TRUE);
		}
	}


	// If this is a Particle Replay track, add buttons for toggling Capture Mode
	UInterpTrackParticleReplay* ParticleReplayTrack = Cast< UInterpTrackParticleReplay >( Track );
	if( ParticleReplayTrack != NULL )
	{
		AppendSeparator();

		// Are we capturing already?  Display the appropriate menu option for toggling
		if( ParticleReplayTrack->bIsCapturingReplay )
		{
			AppendCheckItem( IDM_INTERP_ParticleReplayTrackContext_StopRecording, *LocalizeUnrealEd( "InterpEd_ParticleReplayTrackContext_StopRecording" ), TEXT( "" ) );
		}
		else
		{
			AppendCheckItem( IDM_INTERP_ParticleReplayTrackContext_StartRecording, *LocalizeUnrealEd( "InterpEd_ParticleReplayTrackContext_StartRecording" ), TEXT( "" ) );
		}
	}
}

WxMBInterpEdTrackMenu::~WxMBInterpEdTrackMenu()
{

}

/*-----------------------------------------------------------------------------
	WxMBInterpEdBkgMenu
-----------------------------------------------------------------------------*/

WxMBInterpEdBkgMenu::WxMBInterpEdBkgMenu(WxInterpEd* InterpEd)
{
	Append (IDM_INTERP_EDIT_PASTE, *LocalizeUnrealEd("PasteGroup"), *LocalizeUnrealEd("PasteGroup_Desc"));

	AppendSeparator();

	Append( IDM_INTERP_NEW_FOLDER, *LocalizeUnrealEd("InterpEd_AddNewFolder"), TEXT("") );

	AppendSeparator();

	Append( IDM_INTERP_NEW_EMPTY_GROUP, *LocalizeUnrealEd("AddNewEmptyGroup"), TEXT("") );

	// Prefab group types
	Append( IDM_INTERP_NEW_CAMERA_GROUP, *LocalizeUnrealEd("AddNewCameraGroup"), TEXT("") );
	Append( IDM_INTERP_NEW_PARTICLE_GROUP, *LocalizeUnrealEd("AddNewParticleGroup"), TEXT("") );
	Append( IDM_INTERP_NEW_SKELETAL_MESH_GROUP, *LocalizeUnrealEd("AddNewSkeletalMeshGroup"), TEXT("") );

	TArray<UInterpTrack*> Results;
	InterpEd->IData->FindTracksByClass( UInterpTrackDirector::StaticClass(), Results );
	if(Results.Num() == 0)
	{
		Append( IDM_INTERP_NEW_DIRECTOR_GROUP, *LocalizeUnrealEd("AddNewDirectorGroup"), TEXT("") );
	}
}

WxMBInterpEdBkgMenu::~WxMBInterpEdBkgMenu()
{

}

/*-----------------------------------------------------------------------------
	WxMBInterpEdKeyMenu
-----------------------------------------------------------------------------*/

WxMBInterpEdKeyMenu::WxMBInterpEdKeyMenu(WxInterpEd* InterpEd)
{
	UBOOL bHaveMoveKeys = FALSE;
	UBOOL bHaveFloatKeys = FALSE;
	UBOOL bHaveVectorKeys = FALSE;
	UBOOL bHaveColorKeys = FALSE;
	UBOOL bHaveEventKeys = FALSE;
	UBOOL bHaveAnimKeys = FALSE;
	UBOOL bHaveDirKeys = FALSE;
	UBOOL bAnimIsLooping = FALSE;
	UBOOL bHaveToggleKeys = FALSE;
	UBOOL bHaveVisibilityKeys = FALSE;
	UBOOL bHaveAudioMasterKeys = FALSE;
	UBOOL bHaveParticleReplayKeys = FALSE;

	// TRUE if at least one sound key is selected.
	UBOOL bHaveSoundKeys = FALSE;


	// Keep track of the conditions required for all selected visibility keys to fire
	UBOOL bAllKeyConditionsAreSetToAlways = TRUE;
	UBOOL bAllKeyConditionsAreGoreEnabled = TRUE;
	UBOOL bAllKeyConditionsAreGoreDisabled = TRUE;


	for(INT i=0; i<InterpEd->Opt->SelectedKeys.Num(); i++)
	{
		FInterpEdSelKey& SelKey = InterpEd->Opt->SelectedKeys(i);
		UInterpTrack* Track = SelKey.Group->InterpTracks(SelKey.TrackIndex);

		if( Track->IsA(UInterpTrackMove::StaticClass()) )
		{
			bHaveMoveKeys = TRUE;
		}
		else if( Track->IsA(UInterpTrackEvent::StaticClass()) )
		{
			bHaveEventKeys = TRUE;
		}
		else if( Track->IsA(UInterpTrackDirector::StaticClass()) )
		{
			bHaveDirKeys = TRUE;
		}
		else if( Track->IsA(UInterpTrackAnimControl::StaticClass()) )
		{
			bHaveAnimKeys = TRUE;

			UInterpTrackAnimControl* AnimTrack = (UInterpTrackAnimControl*)Track;
			bAnimIsLooping = AnimTrack->AnimSeqs(SelKey.KeyIndex).bLooping;
		}
		else if( Track->IsA(UInterpTrackFloatBase::StaticClass()) )
		{
			bHaveFloatKeys = TRUE;
		}
		else if( Track->IsA(UInterpTrackColorProp::StaticClass()) )
		{
			bHaveColorKeys = TRUE;
		}
		else if( Track->IsA(UInterpTrackVectorBase::StaticClass()) )
		{
			bHaveVectorKeys = TRUE;
		}

		if( Track->IsA(UInterpTrackSound::StaticClass()) )
		{
			bHaveSoundKeys = TRUE;
		}

		if( Track->IsA( UInterpTrackToggle::StaticClass() ) )
		{
			bHaveToggleKeys = TRUE;
		}

		if( Track->IsA( UInterpTrackVisibility::StaticClass() ) )
		{
			bHaveVisibilityKeys = TRUE;

			UInterpTrackVisibility* VisibilityTrack = CastChecked<UInterpTrackVisibility>(Track);
			FVisibilityTrackKey& VisibilityKey = VisibilityTrack->VisibilityTrack( SelKey.KeyIndex );

			if( VisibilityKey.ActiveCondition != EVTC_Always )
			{
				bAllKeyConditionsAreSetToAlways = FALSE;
			}

			if( VisibilityKey.ActiveCondition != EVTC_GoreEnabled )
			{
				bAllKeyConditionsAreGoreEnabled = FALSE;
			}

			if( VisibilityKey.ActiveCondition != EVTC_GoreDisabled )
			{
				bAllKeyConditionsAreGoreDisabled = FALSE;
			}
		}

		if( Track->IsA( UInterpTrackAudioMaster::StaticClass() ) )
		{
			bHaveAudioMasterKeys = TRUE;
		}

		if( Track->IsA( UInterpTrackParticleReplay::StaticClass() ) )
		{
			bHaveParticleReplayKeys = TRUE;
		}
	}

	if(bHaveMoveKeys || bHaveFloatKeys || bHaveVectorKeys)
	{
		wxMenu* MoveMenu = new wxMenu();
		MoveMenu->Append( IDM_INTERP_KEYMODE_CURVE_AUTO, *LocalizeUnrealEd("CurveAuto"), TEXT("") );
		MoveMenu->Append( IDM_INTERP_KEYMODE_CURVE_AUTO_CLAMPED, *LocalizeUnrealEd("CurveAutoClamped"), TEXT("") );
		MoveMenu->Append( IDM_INTERP_KEYMODE_CURVEBREAK, *LocalizeUnrealEd("CurveBreak"), TEXT("") );
		MoveMenu->Append( IDM_INTERP_KEYMODE_LINEAR, *LocalizeUnrealEd("Linear"), TEXT("") );
		MoveMenu->Append( IDM_INTERP_KEYMODE_CONSTANT, *LocalizeUnrealEd("Constant"), TEXT("") );
		Append( IDM_INTERP_MOVEKEYMODEMENU, *LocalizeUnrealEd("InterpMode"), MoveMenu );
	}

	if(InterpEd->Opt->SelectedKeys.Num() == 1)
	{
		Append( IDM_INTERP_KEY_SETTIME, *LocalizeUnrealEd("SetTime"), TEXT("") );

		if(bHaveMoveKeys)
		{
			AppendSeparator();

			wxMenuItem* SetLookupSourceItem = Append(IDM_INTERP_MOVEKEY_SETLOOKUP, *LocalizeUnrealEd("GetPositionFromAnotherGroup"), TEXT(""));
			wxMenuItem* ClearLookupSourceItem = Append(IDM_INTERP_MOVEKEY_CLEARLOOKUP, *LocalizeUnrealEd("ClearGroupLookup"), TEXT(""));

			FInterpEdSelKey& SelKey = InterpEd->Opt->SelectedKeys(0);
			UInterpTrackMove* MoveTrack = Cast<UInterpTrackMove>(SelKey.Group->InterpTracks(SelKey.TrackIndex));

			if( MoveTrack )
			{
				FName GroupName = MoveTrack->GetLookupKeyGroupName(SelKey.KeyIndex);

				if(GroupName == NAME_None)
				{
					ClearLookupSourceItem->Enable(FALSE);
				}
				else
				{
					ClearLookupSourceItem->SetText(*FString::Printf(LocalizeSecure(LocalizeUnrealEd("ClearGroupLookup_F"), *GroupName.ToString())));
				}
			}
		}

		if(bHaveFloatKeys)
		{
			Append( IDM_INTERP_KEY_SETVALUE, *LocalizeUnrealEd("SetValue"), TEXT("") );
		}

		if(bHaveColorKeys)
		{
			Append( IDM_INTERP_KEY_SETCOLOR, *LocalizeUnrealEd("SetColor"), TEXT("") );
		}

		if(bHaveEventKeys)
		{
			Append( IDM_INTERP_EVENTKEY_RENAME, *LocalizeUnrealEd("RenameEvent"), TEXT("") );
		}

		if(bHaveDirKeys)
		{
			Append(IDM_INTERP_DIRKEY_SETTRANSITIONTIME, *LocalizeUnrealEd("SetTransitionTime"));
		}

		if( bHaveAudioMasterKeys )
		{
			Append( IDM_INTERP_KeyContext_SetMasterVolume, *LocalizeUnrealEd( "InterpEd_KeyContext_SetMasterVolume" ) );
			Append( IDM_INTERP_KeyContext_SetMasterPitch, *LocalizeUnrealEd( "InterpEd_KeyContext_SetMasterPitch" ) );
		}
	}

	if( bHaveToggleKeys || bHaveVisibilityKeys )
	{
		Append(IDM_INTERP_TOGGLEKEY_FLIP, *LocalizeUnrealEd("FlipToggle"));
	}

	if( bHaveVisibilityKeys )
	{
		wxMenu* ConditionMenu = new wxMenu();

		// Key condition: Always
		wxMenuItem* AlwaysItem = ConditionMenu->AppendCheckItem( IDM_INTERP_KeyContext_SetCondition_Always, *LocalizeUnrealEd( "InterpEd_KeyContext_SetCondition_Always" ) );
		AlwaysItem->Check( bAllKeyConditionsAreSetToAlways ? true : false );

		// Key condition: Gore Enabled
		wxMenuItem* GoreEnabledItem = ConditionMenu->AppendCheckItem( IDM_INTERP_KeyContext_SetCondition_GoreEnabled, *LocalizeUnrealEd( "InterpEd_KeyContext_SetCondition_GoreEnabled" ) );
		GoreEnabledItem->Check( bAllKeyConditionsAreGoreEnabled ? true : false );

		// Key condition: Gore Disabled
		wxMenuItem* GoreDisabledItem = ConditionMenu->AppendCheckItem( IDM_INTERP_KeyContext_SetCondition_GoreDisabled, *LocalizeUnrealEd( "InterpEd_KeyContext_SetCondition_GoreDisabled" ) );
		GoreDisabledItem->Check( bAllKeyConditionsAreGoreDisabled ? true : false );

		Append( IDM_INTERP_KeyContext_ConditionMenu, *LocalizeUnrealEd( "InterpEd_KeyContext_ConditionMenu" ), ConditionMenu );
	}

	if(bHaveAnimKeys)
	{
		Append(IDM_INTERP_ANIMKEY_LOOP, *LocalizeUnrealEd("SetAnimLooping"));
		Append(IDM_INTERP_ANIMKEY_NOLOOP, *LocalizeUnrealEd("SetAnimNoLooping"));

		if(InterpEd->Opt->SelectedKeys.Num() == 1)
		{
			Append(IDM_INTERP_ANIMKEY_SETSTARTOFFSET,  *LocalizeUnrealEd("SetStartOffset"));
			Append(IDM_INTERP_ANIMKEY_SETENDOFFSET,  *LocalizeUnrealEd("SetEndOffset"));
			Append(IDM_INTERP_ANIMKEY_SETPLAYRATE,  *LocalizeUnrealEd("SetPlayRate"));
			AppendCheckItem(IDM_INTERP_ANIMKEY_TOGGLEREVERSE,  *LocalizeUnrealEd("Reverse"));
		}
	}

	if ( bHaveSoundKeys )
	{
		Append( IDM_INTERP_SoundKey_SetVolume, *LocalizeUnrealEd("SetSoundVolume") );
		Append( IDM_INTERP_SoundKey_SetPitch, *LocalizeUnrealEd("SetSoundPitch") );


		// Does this key have a sound cue set?
		FInterpEdSelKey& SelKey = InterpEd->Opt->SelectedKeys( 0 );
		UInterpTrackSound* SoundTrack = Cast<UInterpTrackSound>( SelKey.Group->InterpTracks( SelKey.TrackIndex ) );
		USoundCue* KeySoundCue = SoundTrack->Sounds( SelKey.KeyIndex ).Sound;
		if( KeySoundCue != NULL )
		{
			AppendSeparator();

			Append( IDM_INTERP_KeyContext_SyncGenericBrowserToSoundCue,
					*FString::Printf( LocalizeSecure( LocalizeUnrealEd( "InterpEd_KeyContext_SyncGenericBrowserToSoundCue_F" ),
									*KeySoundCue->GetName() ) ) );
		}
	}


	if( bHaveParticleReplayKeys)
	{
		Append( IDM_INTERP_ParticleReplayKeyContext_SetClipIDNumber, *LocalizeUnrealEd( "InterpEd_ParticleReplayKeyContext_SetClipIDNumber" ) );
		Append( IDM_INTERP_ParticleReplayKeyContext_SetDuration, *LocalizeUnrealEd( "InterpEd_ParticleReplayKeyContext_SetDuration" ) );
	}


	if( InterpEd->Opt->SelectedKeys.Num() > 0 )
	{
		AppendSeparator();
		Append( IDM_INTERP_DeleteSelectedKeys, *LocalizeUnrealEd( "InterpEd_KeyContext_DeleteSelected" ) );
	}
}

WxMBInterpEdKeyMenu::~WxMBInterpEdKeyMenu()
{

}



/*-----------------------------------------------------------------------------
	WxMBInterpEdCollapseExpandMenu
-----------------------------------------------------------------------------*/

WxMBInterpEdCollapseExpandMenu::WxMBInterpEdCollapseExpandMenu( WxInterpEd* InterpEd )
{
	Append( IDM_INTERP_ExpandAllGroups, *LocalizeUnrealEd( "InterpEdExpandAllGroups" ), *LocalizeUnrealEd( "InterpEdExpandAllGroups_Desc" ) );
	Append( IDM_INTERP_CollapseAllGroups, *LocalizeUnrealEd( "InterpEdCollapseAllGroups" ), *LocalizeUnrealEd( "InterpEdCollapseAllGroups_Desc" ) );
}


WxMBInterpEdCollapseExpandMenu::~WxMBInterpEdCollapseExpandMenu()
{
}



/*-----------------------------------------------------------------------------
	WxMBCameraAnimEdGroupMenu
-----------------------------------------------------------------------------*/

WxMBCameraAnimEdGroupMenu::WxMBCameraAnimEdGroupMenu(WxCameraAnimEd* InterpEd)
{
	// Nothing to do if no group selected
	if(!InterpEd->ActiveGroup)
	{
		return;
	}

	for(INT i=0; i<InterpEd->InterpTrackClasses.Num(); i++)
	{
		UInterpTrack* DefTrack = (UInterpTrack*)InterpEd->InterpTrackClasses(i)->GetDefaultObject();
		if(!DefTrack->bDirGroupOnly)
		{
			FString NewTrackString = FString::Printf( LocalizeSecure(LocalizeUnrealEd("AddNew_F"), *InterpEd->InterpTrackClasses(i)->GetDescription()) );
			Append( IDM_INTERP_NEW_TRACK_START+i, *NewTrackString, TEXT("") );
		}
	}

	AppendSeparator();

	// Add CameraAnim export option if appropriate
	UInterpGroupInst* GrInst = InterpEd->Interp->FindFirstGroupInst(InterpEd->ActiveGroup);
	if (GrInst)
	{
		AActor* const GroupActor = GrInst->GetGroupActor();
		UBOOL bControllingACameraActor = GroupActor && GroupActor->IsA(ACameraActor::StaticClass());
		if (bControllingACameraActor)
		{
			// add strings to unrealed.int
			Append(IDM_INTERP_CAMERA_ANIM_EXPORT, *LocalizeUnrealEd("ExportCameraAnim"), *LocalizeUnrealEd("ExportCameraAnim_Desc"));
			AppendSeparator();
		}
	}

	Append( IDM_INTERP_GROUP_RENAME, *LocalizeUnrealEd("RenameGroup"), TEXT("") );
}

WxMBCameraAnimEdGroupMenu::~WxMBCameraAnimEdGroupMenu()
{
}


