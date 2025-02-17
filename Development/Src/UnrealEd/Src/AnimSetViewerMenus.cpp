/*=============================================================================
	AnimSetViewerMenus.cpp: AnimSet viewer menus
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "EngineAnimClasses.h"
#include "AnimSetViewer.h"
#include "Properties.h"
#include "DlgGenericComboEntry.h"
#include "DlgRename.h"
#include "BusyCursor.h"
#include "SoftBodyEditor.h"

///////////////////// MENU HANDLERS

/**
 *	Called when a SIZE event occurs on the window
 *
 *	@param	In		The size event information
 */
void WxAnimSetViewer::OnSize( wxSizeEvent& In )
{
	In.Skip();
	Refresh();
	UpdatePreviewWindow();
}

void WxAnimSetViewer::OnSkelMeshComboChanged(wxCommandEvent& In)
{
	const INT SelIndex = SkelMeshCombo->GetSelection();
	// Should not be possible to select no skeletal mesh!
	if( SelIndex != -1 )
	{
		USkeletalMesh* NewSelectedSkelMesh = (USkeletalMesh*)SkelMeshCombo->GetClientData(SelIndex);
		SetSelectedSkelMesh( NewSelectedSkelMesh, TRUE );
	}
}

void WxAnimSetViewer::OnAuxSkelMeshComboChanged(wxCommandEvent& In)
{
	USkeletalMeshComponent* AuxComp = NULL;
	USkeletalMesh* NewMesh = NULL;

	if(In.GetId() == IDM_ANIMSET_SKELMESHAUX1COMBO)
	{
		AuxComp = PreviewSkelCompAux1;

		if(SkelMeshAux1Combo->GetSelection() != -1)
		{
			NewMesh = (USkeletalMesh*)SkelMeshAux1Combo->GetClientData( SkelMeshAux1Combo->GetSelection() );
		}
	}
	else if(In.GetId() == IDM_ANIMSET_SKELMESHAUX2COMBO)
	{
		AuxComp = PreviewSkelCompAux2;

		if(SkelMeshAux2Combo->GetSelection() != -1)
		{
			NewMesh = (USkeletalMesh*)SkelMeshAux2Combo->GetClientData( SkelMeshAux2Combo->GetSelection() );
		}
	}
	else if(In.GetId() == IDM_ANIMSET_SKELMESHAUX3COMBO)
	{
		AuxComp = PreviewSkelCompAux3;

		if(SkelMeshAux3Combo->GetSelection() != -1)
		{
			NewMesh = (USkeletalMesh*)SkelMeshAux3Combo->GetClientData( SkelMeshAux3Combo->GetSelection() );
		}
	}

	if(AuxComp)
	{
		FASVViewportClient* ASVPreviewVC = GetPreviewVC();
		check( ASVPreviewVC );

		AuxComp->SetSkeletalMesh(NewMesh);
		AuxComp->UpdateParentBoneMap();
		ASVPreviewVC->PreviewScene.AddComponent(AuxComp,FMatrix::Identity);
	}
}

void WxAnimSetViewer::OnAnimSetComboChanged(wxCommandEvent& In)
{
	const INT SelIndex = AnimSetCombo->GetSelection();
	if( SelIndex != -1)
	{
		UAnimSet* NewSelectedAnimSet = (UAnimSet*)AnimSetCombo->GetClientData(SelIndex);
		SetSelectedAnimSet( NewSelectedAnimSet, true );
	}
}

void WxAnimSetViewer::OnSkelMeshUse( wxCommandEvent& In )
{
	USkeletalMesh* SelectedSkelMesh = GEditor->GetSelectedObjects()->GetTop<USkeletalMesh>();
	if(SelectedSkelMesh)
	{
		SetSelectedSkelMesh(SelectedSkelMesh, true);
	}
}

void WxAnimSetViewer::OnAnimSetUse( wxCommandEvent& In )
{
	UAnimSet* SelectedAnimSet = GEditor->GetSelectedObjects()->GetTop<UAnimSet>();
	if(SelectedAnimSet)
	{
		SetSelectedAnimSet(SelectedAnimSet, true);
	}
}

void WxAnimSetViewer::OnAuxSkelMeshUse( wxCommandEvent& In )
{
	USkeletalMesh* SelectedSkelMesh = GEditor->GetSelectedObjects()->GetTop<USkeletalMesh>();
	if(SelectedSkelMesh)
	{
		USkeletalMeshComponent* AuxComp = NULL;
		wxComboBox* AuxCombo = NULL;

		if(In.GetId() == IDM_ANIMSET_SKELMESH_AUX1USE)
		{
			AuxComp = PreviewSkelCompAux1;
			AuxCombo = SkelMeshAux1Combo;
		}
		else if(In.GetId() == IDM_ANIMSET_SKELMESH_AUX2USE)
		{
			AuxComp = PreviewSkelCompAux2;
			AuxCombo = SkelMeshAux2Combo;
		}
		else if(In.GetId() == IDM_ANIMSET_SKELMESH_AUX3USE)
		{
			AuxComp = PreviewSkelCompAux3;
			AuxCombo = SkelMeshAux3Combo;
		}

		if(AuxComp && AuxCombo)
		{
			// Set the skeletal mesh component to the new mesh.
			AuxComp->SetSkeletalMesh(SelectedSkelMesh);
			AuxComp->UpdateParentBoneMap();

			// Update combo box to point at new selected mesh.

			// First, refresh the contents, in case we have loaded a package since opening the AnimSetViewer.
			AuxCombo->Freeze();
			AuxCombo->Clear();
			AuxCombo->Append( *LocalizeUnrealEd("-None-"), (void*)NULL );
			for(TObjectIterator<USkeletalMesh> It; It; ++It)
			{
				USkeletalMesh* ItSkelMesh = *It;
				AuxCombo->Append( *ItSkelMesh->GetName(), ItSkelMesh );
			}
			AuxCombo->Thaw();

			// Then look for the skeletal mesh.
			for(UINT i=0; i<AuxCombo->GetCount(); i++)
			{
				if(AuxCombo->GetClientData(i) == SelectedSkelMesh)
				{
					AuxCombo->SetSelection(i);
				}
			}
		}
	}
}

void WxAnimSetViewer::OnAnimSeqListChanged(wxCommandEvent& In)
{
	const INT SelIndex = In.GetInt();// AnimSeqList->GetSelection();
	if( SelIndex != -1 )
	{
		UAnimSequence* NewSelectedAnimSeq = (UAnimSequence*)AnimSeqList->GetClientData(SelIndex);
		SetSelectedAnimSequence(NewSelectedAnimSeq);
	}
}

/** Create a new AnimSet, defaulting to in the package of the selected SkeletalMesh. */
void WxAnimSetViewer::OnNewAnimSet(wxCommandEvent& In)
{
	CreateNewAnimSet();
}

void WxAnimSetViewer::OnImportPSA(wxCommandEvent& In)
{
	ImportPSA();
}

void WxAnimSetViewer::OnImportCOLLADAAnim(wxCommandEvent& In)
{
	ImportCOLLADAAnim();
}

void WxAnimSetViewer::OnImportMeshLOD( wxCommandEvent& In )
{
	ImportMeshLOD();
}

void WxAnimSetViewer::OnImportMeshWeights( wxCommandEvent& In )
{
	ImportMeshWeights();
}

void WxAnimSetViewer::OnToggleMeshWeights( wxCommandEvent& In )
{
	if( PreviewSkelComp &&
		PreviewSkelComp->SkeletalMesh )
	{
		bPreviewInstanceWeights = !bPreviewInstanceWeights;
		if( bPreviewInstanceWeights )
		{
			// enable usage of instanced weights
			PreviewSkelComp->ToggleInstanceVertexWeights(TRUE);
			// use all ref skeleton bones so that all vertices use the instanced weights
			TArray<FBonePair> BonePairs;
			BonePairs.Empty(PreviewSkelComp->SkeletalMesh->RefSkeleton.Num());
			BonePairs.Add(PreviewSkelComp->SkeletalMesh->RefSkeleton.Num());
			for (INT i = 0; i < PreviewSkelComp->SkeletalMesh->RefSkeleton.Num(); i++)
			{
				FBonePair& BonePair = BonePairs(i);
				BonePair.Bones[0] = PreviewSkelComp->SkeletalMesh->RefSkeleton(i).Name;
				BonePair.Bones[1] = NAME_None;
			}
			PreviewSkelComp->UpdateInstanceVertexWeightBones(BonePairs);
		}
		else
		{
			// disable usage of instanced weights
			PreviewSkelComp->ToggleInstanceVertexWeights(FALSE);
		}
		PreviewSkelComp->BeginDeferredReattach();

		// toggle label
		MenuBar->Check(IDM_ANIMSET_TOGGLEMESHWEIGHTS, bPreviewInstanceWeights == TRUE );
	}
}

//////////////////////////////////////////////////////////////////////////
// MORPH TARGETS

void WxAnimSetViewer::OnNewMorphTargetSet( wxCommandEvent& In )
{
	if(!SelectedSkelMesh)
	{
		return;
	}

	FString Package = TEXT("");
	FString Group = TEXT("");

	// Use the selected skeletal mesh to find the 'default' package for the new MorphTargetSet.
	// Bit yucky this...
	check( SelectedSkelMesh->GetOuter() );

	// If there are 2 levels above this mesh - top is packages, then group
	if( SelectedSkelMesh->GetOuter()->GetOuter() )
	{
		Group = SelectedSkelMesh->GetOuter()->GetFullGroupName(0);
		Package = SelectedSkelMesh->GetOuter()->GetOuter()->GetName();
	}
	else // Otherwise, just a package.
	{
		Group = TEXT("");
		Package = SelectedSkelMesh->GetOuter()->GetName();
	}

	WxDlgRename RenameDialog;
	RenameDialog.SetTitle( *LocalizeUnrealEd("NewMorphTargetSet") );
	if( RenameDialog.ShowModal( Package, Group, TEXT("NewMorphTargetSet") ) == wxID_OK )
	{
		if( RenameDialog.GetNewName().Len() == 0 || RenameDialog.GetNewPackage().Len() == 0 )
		{
			appMsgf( AMT_OK, *LocalizeUnrealEd("Error_MustSpecifyNewMorphSetName") );
			return;
		}

		UPackage* Pkg = GEngine->CreatePackage(NULL,*RenameDialog.GetNewPackage());
		if( RenameDialog.GetNewGroup().Len() )
		{
			Pkg = GEngine->CreatePackage(Pkg,*RenameDialog.GetNewGroup());
		}

		UMorphTargetSet* NewMorphSet = ConstructObject<UMorphTargetSet>( UMorphTargetSet::StaticClass(), Pkg, FName( *RenameDialog.GetNewName() ), RF_Public|RF_Standalone );

		// Remember this as the base mesh that the morph target will modify
		NewMorphSet->BaseSkelMesh = SelectedSkelMesh;

		// Will update MorphTargetSet list, which will include new AnimSet.
		SetSelectedSkelMesh(SelectedSkelMesh, false);
		SetSelectedMorphSet(NewMorphSet, false);

		SelectedMorphSet->MarkPackageDirty();
	}
}

void WxAnimSetViewer::OnImportMorphTarget( wxCommandEvent& In )
{
	ImportMorphTarget(FALSE);
}

void WxAnimSetViewer::OnImportMorphTargetLOD( wxCommandEvent& In )
{
	ImportMorphTarget(TRUE);
}

void WxAnimSetViewer::OnMorphSetComboChanged( wxCommandEvent& In )
{
	const INT SelIndex = MorphSetCombo->GetSelection();
	if( SelIndex != -1 )
	{
		UMorphTargetSet* NewSelectedMorphSet = (UMorphTargetSet*)MorphSetCombo->GetClientData(SelIndex);
		SetSelectedMorphSet( NewSelectedMorphSet, true );
	}
}

void WxAnimSetViewer::OnMorphSetUse( wxCommandEvent& In )
{
	UMorphTargetSet* SelectedMorphSet = GEditor->GetSelectedObjects()->GetTop<UMorphTargetSet>();
	if(SelectedMorphSet)
	{
		SetSelectedMorphSet(SelectedMorphSet, true);
	}
}

void WxAnimSetViewer::OnMorphTargetListChanged( wxCommandEvent& In )
{
	const INT SelIndex = MorphTargetList->GetSelection();
	if( SelIndex != -1 )
	{
		UMorphTarget* NewSelectedMorphTarget = (UMorphTarget*)MorphTargetList->GetClientData(SelIndex);
		SetSelectedMorphTarget(NewSelectedMorphTarget);
	}
}

void WxAnimSetViewer::OnRenameMorphTarget( wxCommandEvent& In )
{
    RenameSelectedMorphTarget();
}

void WxAnimSetViewer::OnDeleteMorphTarget( wxCommandEvent& In )
{
    DeleteSelectedMorphTarget();
}

//
//////////////////////////////////////////////////////////////////////////

void WxAnimSetViewer::OnTimeScrub(wxScrollEvent& In)
{
	if(SelectedAnimSeq)
	{
		check(SelectedAnimSeq->SequenceName == PreviewAnimNode->AnimSeqName && SelectedAnimSeq->SequenceName == PreviewAnimNodeRaw->AnimSeqName);
		const INT NewIntPosition = In.GetPosition();
		const FLOAT NewPosition = (FLOAT)NewIntPosition/(FLOAT)ASV_SCRUBRANGE;
		PreviewAnimNode->SetPosition( NewPosition * SelectedAnimSeq->SequenceLength, false );
		PreviewAnimNodeRaw->SetPosition( NewPosition * SelectedAnimSeq->SequenceLength, false );
	}
}

void WxAnimSetViewer::OnViewBones(wxCommandEvent& In)
{
	PreviewSkelComp->bDisplayBones = In.IsChecked();
	PreviewSkelComp->BeginDeferredReattach();
	MenuBar->Check( IDM_ANIMSET_VIEWBONES, PreviewSkelComp->bDisplayBones == TRUE );
	ToolBar->ToggleTool( IDM_ANIMSET_VIEWBONES, PreviewSkelComp->bDisplayBones == TRUE );
}

void WxAnimSetViewer::OnShowRawAnimation(wxCommandEvent& In)
{
	PreviewSkelCompRaw->bDisplayBones = In.IsChecked();
	PreviewSkelCompRaw->BeginDeferredReattach();
	MenuBar->Check( IDM_ANIMSET_ShowRawAnimation, PreviewSkelCompRaw->bDisplayBones == TRUE );
	ToolBar->ToggleTool( IDM_ANIMSET_ShowRawAnimation, PreviewSkelCompRaw->bDisplayBones == TRUE );
}

void WxAnimSetViewer::OnViewBoneNames(wxCommandEvent& In)
{
	FASVViewportClient* ASVPreviewVC = GetPreviewVC();
	check( ASVPreviewVC );

	ASVPreviewVC->bShowBoneNames = In.IsChecked();
	MenuBar->Check( IDM_ANIMSET_VIEWBONENAMES, ASVPreviewVC->bShowBoneNames == TRUE );
	ToolBar->ToggleTool( IDM_ANIMSET_VIEWBONENAMES, ASVPreviewVC->bShowBoneNames == TRUE );
}

void WxAnimSetViewer::OnViewFloor(wxCommandEvent& In)
{
	FASVViewportClient* ASVPreviewVC = GetPreviewVC();
	check( ASVPreviewVC );

	ASVPreviewVC->bShowFloor = In.IsChecked();
}

void WxAnimSetViewer::OnViewRefPose(wxCommandEvent& In)
{
	PreviewSkelComp->bForceRefpose = In.IsChecked();
	PreviewSkelCompRaw->bForceRefpose = In.IsChecked();
	MenuBar->Check( IDM_ANIMSET_VIEWREFPOSE, PreviewSkelComp->bForceRefpose == TRUE );
	ToolBar->ToggleTool( IDM_ANIMSET_VIEWREFPOSE, PreviewSkelComp->bForceRefpose == TRUE );
}

void WxAnimSetViewer::OnViewMirror(wxCommandEvent& In)
{
	PreviewAnimMirror->bEnableMirroring = In.IsChecked();
	MenuBar->Check( IDM_ANIMSET_VIEWMIRROR, PreviewAnimMirror->bEnableMirroring );
	ToolBar->ToggleTool( IDM_ANIMSET_VIEWMIRROR, PreviewAnimMirror->bEnableMirroring );
}

void WxAnimSetViewer::OnViewBounds(wxCommandEvent& In)
{
	FASVViewportClient* ASVPreviewVC = GetPreviewVC();
	check( ASVPreviewVC );

	if( In.IsChecked() )
	{
		ASVPreviewVC->ShowFlags |= SHOW_Bounds;
	}
	else
	{
		ASVPreviewVC->ShowFlags &= ~SHOW_Bounds;
	}
}

void WxAnimSetViewer::OnViewCollision(wxCommandEvent& In)
{
	FASVViewportClient* ASVPreviewVC = GetPreviewVC();
	check( ASVPreviewVC );

	if( In.IsChecked() )
	{
		ASVPreviewVC->ShowFlags |= SHOW_Collision;
	}
	else
	{
		ASVPreviewVC->ShowFlags &= ~SHOW_Collision;
	}
}

void WxAnimSetViewer::OnViewSoftBodyTetra(wxCommandEvent& In)
{
	check( PreviewSkelComp );

	PreviewSkelComp->bShowSoftBodyTetra = In.IsChecked();
	PreviewSkelComp->BeginDeferredReattach();
	MenuBar->Check(IDM_ANIMSET_VIEWSOFTBODYTETRA, PreviewSkelComp->bShowSoftBodyTetra == TRUE);
}

/** Handler for forcing the rendering of the preview model to use a particular LOD. */
void WxAnimSetViewer::OnForceLODLevel( wxCommandEvent& In)
{
	if(In.GetId() == IDM_ANIMSET_LOD_AUTO)
	{
		PreviewSkelComp->ForcedLodModel = 0;
		PreviewSkelCompRaw->ForcedLodModel = 0;
	}
	else if(In.GetId() == IDM_ANIMSET_LOD_BASE)
	{
		PreviewSkelComp->ForcedLodModel = 1;
		PreviewSkelCompRaw->ForcedLodModel = 1;
	}
	else if(In.GetId() == IDM_ANIMSET_LOD_1)
	{
		PreviewSkelComp->ForcedLodModel = 2;
		PreviewSkelCompRaw->ForcedLodModel = 2;
	}
	else if(In.GetId() == IDM_ANIMSET_LOD_2)
	{
		PreviewSkelComp->ForcedLodModel = 3;
		PreviewSkelCompRaw->ForcedLodModel = 3;
	}
	else if(In.GetId() == IDM_ANIMSET_LOD_3)
	{
		PreviewSkelComp->ForcedLodModel = 4;
		PreviewSkelCompRaw->ForcedLodModel = 4;
	}

	MenuBar->Check( In.GetId(), true );
	ToolBar->ToggleTool( In.GetId(), true );
}

/** Handler for removing a particular LOD from the SkeletalMesh. */
void WxAnimSetViewer::OnRemoveLOD(wxCommandEvent &In)
{
	if( SelectedSkelMesh->LODModels.Num() == 1 )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("NoLODToRemove") );
		return;
	}

	// Now display combo to choose which LOD to remove.
	TArray<FString> LODStrings;
	LODStrings.AddZeroed( SelectedSkelMesh->LODModels.Num()-1 );
	for(INT i=0; i<SelectedSkelMesh->LODModels.Num()-1; i++)
	{
		LODStrings(i) = FString::Printf( TEXT("%d"), i+1 );
	}

	// pop up dialog
	WxDlgGenericComboEntry dlg;
	if( dlg.ShowModal( TEXT("ChooseLODLevel"), TEXT("LODLevel:"), LODStrings, 0, TRUE ) == wxID_OK )
	{
		check( SelectedSkelMesh->LODInfo.Num() == SelectedSkelMesh->LODModels.Num() );

		// If its a valid LOD, kill it.
		INT DesiredLOD = dlg.GetComboBox().GetSelection() + 1;
		if( DesiredLOD > 0 && DesiredLOD < SelectedSkelMesh->LODModels.Num() )
		{
			// Release rendering resources before deleting LOD
			SelectedSkelMesh->LODModels(DesiredLOD).ReleaseResources();

			// Block until this is done
			FlushRenderingCommands();

			SelectedSkelMesh->LODModels.Remove(DesiredLOD);
			SelectedSkelMesh->LODInfo.Remove(DesiredLOD);

			// Set the forced LOD to Auto.
			PreviewSkelComp->ForcedLodModel = 0;
			PreviewSkelCompRaw->ForcedLodModel = 0;
			MenuBar->Check( IDM_ANIMSET_LOD_AUTO, true );
			ToolBar->ToggleTool( IDM_ANIMSET_LOD_AUTO, true );

			UpdateForceLODButtons();
		}
	}
}

void WxAnimSetViewer::OnSpeed(wxCommandEvent& In)
{
	const INT Id = In.GetId();
	switch (Id)
	{
	case IDM_ANIMSET_SPEED_1:
		PlaybackSpeed = 0.01f;
		break;
	case IDM_ANIMSET_SPEED_10:
		PlaybackSpeed = 0.1f;
		break;
	case IDM_ANIMSET_SPEED_25:
		PlaybackSpeed = 0.25f;
		break;
	case IDM_ANIMSET_SPEED_50:
		PlaybackSpeed = 0.5f;
		break;
	case IDM_ANIMSET_SPEED_100:
	default:
		PlaybackSpeed = 1.0f;
		break;
	};
}

void WxAnimSetViewer::OnViewWireframe(wxCommandEvent& In)
{
	FASVViewportClient* ASVPreviewVC = GetPreviewVC();
	check( ASVPreviewVC );

	if( In.IsChecked() )
	{
		ASVPreviewVC->ShowFlags &= ~SHOW_ViewMode_Mask;
		ASVPreviewVC->ShowFlags |= SHOW_ViewMode_Wireframe;
	}
	else
	{
		ASVPreviewVC->ShowFlags &= ~SHOW_ViewMode_Mask;
		ASVPreviewVC->ShowFlags |= SHOW_ViewMode_Lit;
	}

	MenuBar->Check( IDM_ANIMSET_VIEWWIREFRAME, In.IsChecked() );
	ToolBar->ToggleTool( IDM_ANIMSET_VIEWWIREFRAME, In.IsChecked() );
}

void WxAnimSetViewer::OnViewGrid( wxCommandEvent& In )
{
	FASVViewportClient* ASVPreviewVC = GetPreviewVC();
	check( ASVPreviewVC );

	if( In.IsChecked() )
	{
		ASVPreviewVC->ShowFlags |= SHOW_Grid;
	}
	else
	{
		ASVPreviewVC->ShowFlags &= ~SHOW_Grid;
	}
}

void WxAnimSetViewer::OnViewSockets( wxCommandEvent& In )
{
	FASVViewportClient* ASVPreviewVC = GetPreviewVC();
	check( ASVPreviewVC );

	ASVPreviewVC->bShowSockets = In.IsChecked();
}

void WxAnimSetViewer::OnLoopAnim(wxCommandEvent& In)
{
	PreviewAnimNode->bLooping = !(PreviewAnimNode->bLooping);
	PreviewAnimNodeRaw->bLooping = !(PreviewAnimNodeRaw->bLooping);

	RefreshPlaybackUI();
}

void WxAnimSetViewer::OnPlayAnim(wxCommandEvent& In)
{
	if(!PreviewAnimNode->bPlaying)
	{
		PreviewAnimNode->PlayAnim(PreviewAnimNode->bLooping, 1.f);
		PreviewAnimNodeRaw->PlayAnim(PreviewAnimNodeRaw->bLooping, 1.f);
	}
	else
	{
		PreviewAnimNode->StopAnim();
		PreviewAnimNodeRaw->StopAnim();
	}

	RefreshPlaybackUI();
}

void WxAnimSetViewer::OnEmptySet(wxCommandEvent& In)
{
	EmptySelectedSet();
}

void WxAnimSetViewer::OnDeleteTrack(wxCommandEvent& In)
{
	DeleteTrackFromSelectedSet();
}

void WxAnimSetViewer::OnCopyTranslationBoneNames(wxCommandEvent& In)
{
	CopyTranslationBoneNamesToAnimSet();
}

void WxAnimSetViewer::OnRenameSequence(wxCommandEvent& In)
{
	RenameSelectedSeq();
}

void WxAnimSetViewer::OnDeleteSequence(wxCommandEvent& In)
{
	DeleteSelectedSequence();
}

void WxAnimSetViewer::OnCopySequence(wxCommandEvent& In)
{
	CopySelectedSequence();
}

void WxAnimSetViewer::OnMoveSequence(wxCommandEvent& In)
{
	MoveSelectedSequence();
}

void WxAnimSetViewer::OnMakeSequencesAdditive(wxCommandEvent& In)
{
	MakeSelectedSequencesAdditive();
}

void WxAnimSetViewer::OnSequenceApplyRotation(wxCommandEvent& In)
{
	SequenceApplyRotation();
}

void WxAnimSetViewer::OnSequenceReZero(wxCommandEvent& In)
{
	SequenceReZeroToCurrent();
}

void WxAnimSetViewer::OnSequenceCrop(wxCommandEvent& In)
{
	UBOOL bCropFromStart = FALSE;
	if(In.GetId() == IDM_ANIMSET_SEQDELBEFORE)
	{
		bCropFromStart = TRUE;
	}

	SequenceCrop(bCropFromStart);
}

/** Copies anim notifies from the selected sequence to a user-named destination sequence. */
void WxAnimSetViewer::OnNotifyCopy( wxCommandEvent& In )
{
	if( SelectedAnimSeq )
	{
		check( SelectedAnimSet );

		// If the source sequence doesn't contain any notifies, abort.
		UAnimSequence* SrcSeq = SelectedAnimSeq;
		if ( SrcSeq->Notifies.Num() == 0 )
		{
			appMsgf( AMT_OK, *LocalizeUnrealEd("SelectedSeqHasNoNotifies") );
			return;
		}

		// Prompt the user for the name of the destination sequence.
		WxDlgGenericStringEntry dlg;
		const INT Result = dlg.ShowModal( TEXT("CopyNotifies"), TEXT("DestAnimSequenceName"), *SelectedAnimSeq->SequenceName.ToString() );
		if( Result == wxID_OK )
		{
			// Find the named sequence.
			UAnimSequence* DestSeq = NULL;
			{
				FString EnteredString = dlg.GetEnteredString();

				// parse off part before the dot and see if it's an AnimSet name
				FString AnimSetName = TEXT("");
				FString AnimName = TEXT("");
				
				if (EnteredString.Split(TEXT("."), &AnimSetName, &AnimName) == 0)
				{
					// split failed, probably because there's no dot, which is ok.  AnimName should be the whole string.
					DestSeq = SelectedAnimSet->FindAnimSequence( *EnteredString );
				}
				else
				{
					// try to find the user-specified AnimSet
					UAnimSet* DestAnimSet = FindObject<UAnimSet>(ANY_PACKAGE, *AnimSetName);
					if (DestAnimSet)
					{
						DestSeq = DestAnimSet->FindAnimSequence(*AnimName);
					}
					else
					{
						// error, couldn't find specified AnimSet
						appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("AnimSetNotFound"), *AnimSetName) );
						return;
					}
				}
			}

			// If the named dest sequence couldn't be found, abort.
			if ( !DestSeq )
			{
				appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("DestSeqNotFoundF"), *dlg.GetEnteredString()) );
				return;
			}

			// If the source and dest sequences are the same, abort.
			if (SrcSeq == DestSeq)
			{
				appMsgf( AMT_OK, *LocalizeUnrealEd("DestAndSrcSeqsMatch") );
				return;
			}

			// Copy notifies
			CopyNotifies(SrcSeq, DestSeq);

			// Make sure UI shows the destination anim info.
			SetSelectedAnimSequence( DestSeq );

			// Select the sequence properties page.
			PropNotebook->SetSelection(2);

			// Make sure its up to date.
			AnimSeqProps->SetObject( NULL, TRUE, FALSE, TRUE );
			AnimSeqProps->SetObject( DestSeq, TRUE, FALSE, TRUE );

// 			// Select the new entires in the property window.
// 			for ( INT NewNotify = 0 ; NewNotify < NewNotifyIndices.Num() ; ++NewNotify )
// 			{
// 				AnimSeqProps->ExpandItem( TEXT("Notifies"), NewNotifyIndices(NewNotify) );
// 			}
		}
	}
}

void WxAnimSetViewer::OnNotifySort(wxCommandEvent& In)
{
	AnimSeqProps->SetObject( NULL, TRUE, FALSE, TRUE );

	if(SelectedAnimSeq)
	{
		SelectedAnimSeq->SortNotifies();
		SelectedAnimSeq->MarkPackageDirty();
		AnimSeqProps->SetObject( SelectedAnimSeq, TRUE, FALSE, TRUE );
	}
}

void WxAnimSetViewer::OnNewNotify( wxCommandEvent& In )
{
	if(SelectedAnimSeq)
	{
		FLOAT NewTime = ::Clamp(PreviewAnimNode->CurrentTime, 0.f, SelectedAnimSeq->SequenceLength);
		INT NewNotifyIndex = 0;
		while( NewNotifyIndex < SelectedAnimSeq->Notifies.Num() && 
			SelectedAnimSeq->Notifies(NewNotifyIndex).Time <= NewTime )
		{
			NewNotifyIndex++;
		}

		SelectedAnimSeq->Notifies.InsertZeroed(NewNotifyIndex);
		SelectedAnimSeq->Notifies(NewNotifyIndex).Time = NewTime;
		SelectedAnimSeq->MarkPackageDirty();

		// Select the sequence properties page.
		PropNotebook->SetSelection(2);

		// Make sure its up to date
		AnimSeqProps->SetObject( NULL, TRUE, FALSE, TRUE );
		AnimSeqProps->SetObject( SelectedAnimSeq, TRUE, FALSE, TRUE );

		// Select the new entry
		AnimSeqProps->ExpandItem( FString(TEXT("Notifies")), NewNotifyIndex );
	}
}

void WxAnimSetViewer::OnCopySequenceName( wxCommandEvent& In )
{
	if(SelectedAnimSeq)
	{
		appClipboardCopy(*SelectedAnimSeq->SequenceName.ToString());
	}
}


void WxAnimSetViewer::OnCopySequenceNameList( wxCommandEvent& In )
{
	FString AnimSeqNameListStr;

	for(INT i=0; i<SelectedAnimSet->Sequences.Num(); i++)
	{
		UAnimSequence* Seq = SelectedAnimSet->Sequences(i);

		AnimSeqNameListStr += FString::Printf( TEXT("%s\r\n"), *Seq->SequenceName.ToString());
	}

	appClipboardCopy( *AnimSeqNameListStr );
}


void WxAnimSetViewer::OnCopyMeshBoneNames( wxCommandEvent& In )
{
	FString BoneNames;

	if(SelectedSkelMesh)
	{
		for(INT i=0; i<SelectedSkelMesh->RefSkeleton.Num(); i++)
		{
			FMeshBone& MeshBone = SelectedSkelMesh->RefSkeleton(i);

			INT Depth = 0;
			INT TmpBoneIndex = i;
			while( TmpBoneIndex != 0 )
			{
				TmpBoneIndex = SelectedSkelMesh->RefSkeleton(TmpBoneIndex).ParentIndex;
				Depth++;
			}

			FString LeadingSpace;
			for(INT j=0; j<Depth; j++)
			{
				LeadingSpace += TEXT(" ");
			}
	
			if( i==0 )
			{
				BoneNames += FString::Printf( TEXT("%3d: %s%s\r\n"), i, *LeadingSpace, *MeshBone.Name.ToString());
			}
			else
			{
				BoneNames += FString::Printf( TEXT("%3d: %s%s (ParentBoneID: %d)\r\n"), i, *LeadingSpace, *MeshBone.Name.ToString(), MeshBone.ParentIndex );
			}
		}
	}

	appClipboardCopy( *BoneNames );
}

/**
 * Fixes up names by removing tabs and replacing spaces with underscores.
 *
 * @param	InOldName	The name to fixup.
 * @param	OutResult	[out] The fixed up name.
 * @return				TRUE if the name required fixup, FALSE otherwise.
 */
static UBOOL FixupName(const TCHAR* InOldName, FString& OutResult)
{
	// Replace spaces with underscores.
	OutResult = FString( InOldName ).Replace( TEXT(" "), TEXT("_") );

	// Remove tabs by recursively splitting the string at a tab and merging halves.
	FString LeftStr;
	FString RightStr;

	while ( TRUE )
	{
		const UBOOL bSplit = OutResult.Split( TEXT("\t"), &LeftStr, &RightStr );
		if ( !bSplit )
		{
			break;
		}
		OutResult = LeftStr + RightStr;
	}

	const UBOOL bNewNameDiffersFromOld = ( OutResult != InOldName );
	return bNewNameDiffersFromOld;
}

void WxAnimSetViewer::OnFixupMeshBoneNames( wxCommandEvent& In )
{
	if( !SelectedSkelMesh )
	{
		return;
	}

	// Fix up bone names for the reference skeleton.
	TArray<FMeshBone>& RefSkeleton = SelectedSkelMesh->RefSkeleton;

	TArray<FName> OldNames;
	TArray<FName> NewNames;

	for( INT BoneIndex = 0 ; BoneIndex < RefSkeleton.Num() ; ++BoneIndex )
	{
		FMeshBone& MeshBone = RefSkeleton( BoneIndex );
	
		FString FixedName;
		const UBOOL bNeededFixup = FixupName( *MeshBone.Name.ToString(), FixedName );
		if ( bNeededFixup )
		{
			// Point the skeleton to the new bone name.
			const FName NewName( *FixedName );
			MeshBone.Name = NewName;

			// Store off the old and new bone names for animset fixup.
			NewNames.AddItem( NewName );
			OldNames.AddItem( MeshBone.Name );

			debugf(TEXT("Fixed \"%s\" --> \"%s\""), *MeshBone.Name.ToString(), *NewName.ToString() );
		}
	}

	check( OldNames.Num() == NewNames.Num() );

	// Go back over the list of names that were changed and update the selected animset.
	if( SelectedAnimSet && OldNames.Num() > 0 )
	{
		// Flag for tracking whether or not the anim set referred to bones that were fixed up.
		UBOOL bFixedUpAnimSet = FALSE;

		for ( INT TrackIndex = 0 ; TrackIndex < SelectedAnimSet->TrackBoneNames.Num() ; ++TrackIndex )
		{
			const FName& TrackName = SelectedAnimSet->TrackBoneNames( TrackIndex );

			// See if this track was fixed up.
			for ( INT NameIndex = 0 ; NameIndex < OldNames.Num() ; ++NameIndex )
			{
				const FName& OldName = OldNames( NameIndex );

				if ( OldName == TrackName )
				{
					// Point the track to the new bone name.
					const FName& NewName = NewNames( NameIndex );
					SelectedAnimSet->TrackBoneNames( TrackIndex ) = NewName;

					bFixedUpAnimSet = TRUE;

					break;
				}
			}
		}

		// Mark things for saving.
		SelectedSkelMesh->MarkPackageDirty();

		// Mark the anim set dirty if it refered to bones that were fixed up.
		if ( bFixedUpAnimSet )
		{
			SelectedAnimSet->MarkPackageDirty();
		}
	}
}

void WxAnimSetViewer::OnSwapLODSections(wxCommandEvent& In)
{
	if(!SelectedSkelMesh || PreviewSkelComp->ForcedLodModel == 0)
	{
		return;
	}

	{
		FComponentReattachContext ReattachContext(PreviewSkelComp);
		SelectedSkelMesh->ReleaseResources();
		SelectedSkelMesh->ReleaseResourcesFence.Wait();

		INT LODIndex = PreviewSkelComp->ForcedLodModel - 1;
		FStaticLODModel& LODModel = SelectedSkelMesh->LODModels(LODIndex);
		if(LODModel.Sections.Num() == 2)
		{
			FSkelMeshSection TempSection = LODModel.Sections(0);
			LODModel.Sections(0) = LODModel.Sections(1);
			LODModel.Sections(0).MaterialIndex = 0;
			LODModel.Sections(1) = TempSection;
			LODModel.Sections(1).MaterialIndex = 1;
		}

		SelectedSkelMesh->InitResources();
	}
}

/** Utility for shrinking the Materials array in the SkeletalMesh, removed any duplicates. */
void WxAnimSetViewer::OnMergeMaterials( wxCommandEvent& In )
{
	if(!SelectedSkelMesh)
	{
		return;
	}

	TArray<UMaterialInterface*>	NewMaterialArray;

	TArray<INT> OldToNewMaterialMap;
	OldToNewMaterialMap.Add( SelectedSkelMesh->Materials.Num() );

	for(INT i=0; i<SelectedSkelMesh->Materials.Num(); i++)
	{
		UMaterialInterface* Inst = SelectedSkelMesh->Materials(i);

		INT NewIndex = NewMaterialArray.FindItemIndex(Inst);
		if(NewIndex == INDEX_NONE)
		{
			NewIndex = NewMaterialArray.AddItem(Inst);
			OldToNewMaterialMap(i) = NewIndex;
		}
		else
		{
			OldToNewMaterialMap(i) = NewIndex;
		}
	}

	// Assign new materials array to SkeletalMesh
	SelectedSkelMesh->Materials = NewMaterialArray;

	// For the base mesh - remap the sections.
	for(INT i=0; i<SelectedSkelMesh->LODModels(0).Sections.Num(); i++)
	{
		FSkelMeshSection& Section = SelectedSkelMesh->LODModels(0).Sections(i);
		Section.MaterialIndex = OldToNewMaterialMap( Section.MaterialIndex );
	}

	// Now remap each LOD's LODMaterialMap to take this into account.
	for(INT i=0; i<SelectedSkelMesh->LODInfo.Num(); i++)
	{
		FSkeletalMeshLODInfo& LODInfo = SelectedSkelMesh->LODInfo(i);
		for(INT j=0; j<LODInfo.LODMaterialMap.Num(); j++)
		{
			LODInfo.LODMaterialMap(j) = OldToNewMaterialMap( LODInfo.LODMaterialMap(j) );
		}
	}

	// Display success box.
	appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("MergeMaterialSuccess"), OldToNewMaterialMap.Num(), SelectedSkelMesh->Materials.Num()) );
}


static FLOAT DistFromPlane(FVector& Point, BYTE Axis)
{
	if(Axis == AXIS_X)
	{
		return Point.X;
	}
	else if(Axis == AXIS_Y)
	{
		return Point.Y;
	}
	else if(Axis == AXIS_Z)
	{
		return Point.Z;
	}
	else
	{
		return 0.f;
	}
}

static void MirrorPointByPlane(FVector& Point, BYTE Axis)
{
	if(Axis == AXIS_X)
	{
		Point.X *= -1.f;
	}
	else if(Axis == AXIS_Y)
	{
		Point.Y *= -1.f;
	}
	else if(Axis == AXIS_Z)
	{
		Point.Z *= -1.f;
	}
}

static const FLOAT NoSwapThreshold = 0.2f;
static const FLOAT MatchBoneThreshold = 0.1f;


/** Utility tool for constructing the mirror table automatically. */
void WxAnimSetViewer::OnAutoMirrorTable( wxCommandEvent& In )
{
	if( SelectedSkelMesh->SkelMirrorAxis != AXIS_X &&
		SelectedSkelMesh->SkelMirrorAxis != AXIS_Y &&
		SelectedSkelMesh->SkelMirrorAxis != AXIS_Z )
	{
		return;
	}

	TArray<BYTE> NewMirrorTable;
	NewMirrorTable.Add(SelectedSkelMesh->RefSkeleton.Num());

	// First we build all the mesh-space reference pose transforms.
	TArray<FMatrix> BoneTM;
	BoneTM.Add(SelectedSkelMesh->RefSkeleton.Num());

	for(INT i=0; i<SelectedSkelMesh->RefSkeleton.Num(); i++)
	{
		BoneTM(i) = FQuatRotationTranslationMatrix( SelectedSkelMesh->RefSkeleton(i).BonePos.Orientation, SelectedSkelMesh->RefSkeleton(i).BonePos.Position );

		if(i>0)
		{
			INT ParentIndex = SelectedSkelMesh->RefSkeleton(i).ParentIndex;
			BoneTM(i) = BoneTM(i) * BoneTM(ParentIndex);
		}

		// Init table to 255, so we can see where we are missing bones later on.
		NewMirrorTable(i) = 255;
	}

	// Then we go looking for pairs of bones.
	for(INT i=0; i<SelectedSkelMesh->RefSkeleton.Num(); i++)
	{
		// First, find distance of point from mirror plane.
		FVector BonePos = BoneTM(i).GetOrigin();
		FLOAT DistFromMirrorPlane = DistFromPlane(BonePos, SelectedSkelMesh->SkelMirrorAxis);

		// If its suitable small (eg. for spine) we don't look for its mirror twin.
		if(Abs(DistFromMirrorPlane) > NoSwapThreshold)
		{
			// Only search for bones on positive side of plane.
			if(DistFromMirrorPlane > 0.f)
			{
				INT ClosestBoneIndex = INDEX_NONE;
				FLOAT ClosestDist = 10000000000.f;

				// Mirror point, to compare distances against.
				MirrorPointByPlane(BonePos, SelectedSkelMesh->SkelMirrorAxis);

				for(INT j=0; j<SelectedSkelMesh->RefSkeleton.Num(); j++)
				{
					FVector TestPos = BoneTM(j).GetOrigin();
					FLOAT TestDistFromPlane = DistFromPlane(TestPos, SelectedSkelMesh->SkelMirrorAxis);

					// If point is on negative side of plane, test against it.
					if(TestDistFromPlane < 0.f)
					{
						// If this is closer than best match so far, remember it.
						FLOAT DistToBone = (BonePos - TestPos).Size();
						if(DistToBone < ClosestDist)
						{
							ClosestBoneIndex = j;
							ClosestDist = DistToBone;
						}
					}
				}

				// If below our match threshold..
				if(ClosestDist < MatchBoneThreshold)
				{
					UBOOL bDoAssignment = true;

					// If one of these slots is already being used, fail.
					if(NewMirrorTable(i) != 255)
					{
						//appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("AutoMirrorMultiMatch"), *(SelectedSkelMesh->RefSkeleton(i).Name)) );
						NewMirrorTable(i) = 255;
						bDoAssignment = false;
					}

					if( NewMirrorTable(ClosestBoneIndex) != 255)
					{
						//appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("AutoMirrorMultiMatch"), *(SelectedSkelMesh->RefSkeleton(ClosestBoneIndex).Name)) );
						NewMirrorTable(ClosestBoneIndex) = 255;
						bDoAssignment = false;
					}

					// If not, set these into the mirror table.
					if(bDoAssignment)
					{
						NewMirrorTable(i) = ClosestBoneIndex;
						NewMirrorTable(ClosestBoneIndex) = i;
					}
				}
				else
				{					
					//appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("AutoMirrorNoMatch"), *(SelectedSkelMesh->RefSkeleton(i).Name)) );
					debugf( TEXT("NoMatch: %d:%s (closest %d:%s %f)"), 
						i, *(SelectedSkelMesh->RefSkeleton(i).Name.ToString()), 
						ClosestBoneIndex, *(SelectedSkelMesh->RefSkeleton(ClosestBoneIndex).Name.ToString()), ClosestDist );
				}
			}
		}
	}

	// Look for bones that are not present.
	INT ProblemCount = 0;
	for(INT i=0; i<NewMirrorTable.Num(); i++)
	{
		if(NewMirrorTable(i) == 255)
		{
			ProblemCount++;
			NewMirrorTable(i) = i;
		}
	}

	// Clear old mapping table and assign new one.
	SelectedSkelMesh->SkelMirrorTable.Empty();
	SelectedSkelMesh->SkelMirrorTable.AddZeroed(NewMirrorTable.Num());
	for(INT i=0; i<NewMirrorTable.Num(); i++)
	{
		SelectedSkelMesh->SkelMirrorTable(i).SourceIndex = NewMirrorTable(i);
	}

	SelectedSkelMesh->MarkPackageDirty();

	appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("AutoMirrorProblemCount"), ProblemCount) );
}

/** Check the current mirror table is ok */
void WxAnimSetViewer::OnCheckMirrorTable( wxCommandEvent& In )
{
	FString ProblemBones;
	UBOOL bIsOK = SelectedSkelMesh->MirrorTableIsGood(ProblemBones);
	if(!bIsOK)
	{
		appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("MirrorTableBad"), *ProblemBones) );
	}
	else
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("MirrorTableOK") );
	}
}

/**
 * Copy Mirror table to clipboard
 */
void WxAnimSetViewer::OnCopyMirrorTable( wxCommandEvent& In )
{
	FString	Table;
	UEnum*	AxisEnum = FindObject<UEnum>(UObject::StaticClass(), TEXT("EAxis"));

	if( SelectedSkelMesh )
	{
		for(INT i=0; i<SelectedSkelMesh->SkelMirrorTable.Num(); i++)
		{
			FMeshBone& MeshBone = SelectedSkelMesh->RefSkeleton(i);

			Table += FString::Printf( TEXT("%3d: %d %s \r\n"), i, SelectedSkelMesh->SkelMirrorTable(i).SourceIndex, *AxisEnum->GetEnum(SelectedSkelMesh->SkelMirrorTable(i).BoneFlipAxis).ToString() );
		}
	}

	appClipboardCopy( *Table );
}

/** Copy the mirror table from the selected SkeletalMesh in the browser to the current mesh in the AnimSet Viewer */
void WxAnimSetViewer::OnCopyMirroTableFromMesh( wxCommandEvent& In )
{
	// Get selected mesh, and check we have one!
	USkeletalMesh* SrcMesh = GEditor->GetSelectedObjects()->GetTop<USkeletalMesh>();
	if(!SrcMesh)
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("NoSourceSkelMeshSelected") );
		return;
	}

	// Check source mesh has a mirror table
	if(SrcMesh->SkelMirrorTable.Num() == 0)
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("SrcMeshNoMirrorTable") );
		return;
	}

	// Let the user confirm they want to do this - will clobber existing table in this mesh.
	UBOOL bConfirm = appMsgf( AMT_YesNo, *LocalizeUnrealEd("SureCopyMirrorTable") );
	if(!bConfirm)
	{
		return;
	}

	// Do the copy...
	SelectedSkelMesh->CopyMirrorTableFrom(SrcMesh);

	SelectedSkelMesh->MarkPackageDirty();

	// Run a check, just to be sure, and let user fix up and existing problems.
	FString ProblemBones;
	UBOOL bIsOK = SelectedSkelMesh->MirrorTableIsGood(ProblemBones);
	if(!bIsOK)
	{
		appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("MirrorTableBad"), *ProblemBones) );
	}
}

void WxAnimSetViewer::OnUpdateBounds( wxCommandEvent& In )
{
	UpdateMeshBounds();
}

void WxAnimSetViewer::OnToggleClothSim( wxCommandEvent& In )
{
	if(In.IsChecked())
	{
		PreviewSkelComp->InitClothSim(RBPhysScene);
		PreviewSkelComp->bEnableClothSimulation = TRUE;
	}
	else
	{
		PreviewSkelComp->TermClothSim(NULL);
		PreviewSkelComp->bEnableClothSimulation = FALSE;
	}
}
void WxAnimSetViewer::OnSoftBodyGenerate( wxCommandEvent& In )
{
	if(PreviewSkelComp->SkeletalMesh)
	{
		PreviewSkelComp->TermSoftBodySim(NULL);
		FlushRenderingCommands();
#if WITH_NOVODEX
		PreviewSkelComp->SkeletalMesh->ClearSoftBodyMeshCache();
		BuildSoftBodyMapping(*PreviewSkelComp->SkeletalMesh);	
#endif	//#if WITH_NOVODEX
		PreviewSkelComp->InitSoftBodySimBuffers();
		if(PreviewSkelComp->bEnableSoftBodySimulation)
		{
			PreviewSkelComp->InitSoftBodySim(RBPhysScene, TRUE);
		}
	}
}

void WxAnimSetViewer::OnSoftBodyToggleSim( wxCommandEvent& In )
{
	if(In.IsChecked())
	{
		PreviewSkelComp->InitSoftBodySim(RBPhysScene, TRUE);
		PreviewSkelComp->bEnableSoftBodySimulation = TRUE;
		// TODO: Is there a reason to simulate the "Raw" mesh as well?
	}
	else
	{
		PreviewSkelComp->TermSoftBodySim(NULL);
		PreviewSkelComp->bEnableSoftBodySimulation = FALSE;
		// Reset mesh to position based on original tetra-mesh.
		PreviewSkelComp->InitSoftBodySimBuffers();
	}
}

/*-----------------------------------------------------------------------------
	WxASVToolBar
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxASVToolBar, wxToolBar )
END_EVENT_TABLE()

WxASVToolBar::WxASVToolBar( wxWindow* InParent, wxWindowID InID )
: wxToolBar( InParent, InID, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_3DBUTTONS )
{
	SocketMgrB.Load(TEXT("ASV_SocketMgr"));
	ShowBonesB.Load(TEXT("AnimTree_ShowBones"));
	ShowRawAnimationB.Load(TEXT("ASV_ShowRawAnimation"));
	ShowBoneNamesB.Load(TEXT("AnimTree_ShowBoneNames"));
	ShowWireframeB.Load(TEXT("AnimTree_ShowWireframe"));
	ShowRefPoseB.Load(TEXT("ASV_ShowRefPose"));
	ShowMirrorB.Load(TEXT("ASV_ShowMirror"));

	NewNotifyB.Load(TEXT("ASV_NewNotify"));
	ClothB.Load(TEXT("ASV_Cloth"));
	SoftBodyGenerateB.Load(TEXT("ASV_SoftBodyGenerate"));
	SoftBodyToggleSimB.Load(TEXT("ASV_SoftBodyToggleSim"));

	LODAutoB.Load(TEXT("AnimTree_LOD_Auto"));
	LODBaseB.Load(TEXT("AnimTree_LOD_Base"));
	LOD1B.Load(TEXT("AnimTree_LOD_1"));
	LOD2B.Load(TEXT("AnimTree_LOD_2"));
	LOD3B.Load(TEXT("AnimTree_LOD_3"));

	Speed1B.Load(TEXT("ASV_Speed_1"));
	Speed10B.Load(TEXT("ASV_Speed_10"));
	Speed25B.Load(TEXT("ASV_Speed_25"));
	Speed50B.Load(TEXT("ASV_Speed_50"));
	Speed100B.Load(TEXT("ASV_Speed_100"));

	SetToolBitmapSize( wxSize( 16, 16 ) );

	wxStaticText* LODLabel = new wxStaticText(this, -1 ,*LocalizeUnrealEd("LODLabel") );

	AddSeparator();
	AddCheckTool(IDM_ANIMSET_VIEWBONES, *LocalizeUnrealEd("ToolTip_3"), ShowBonesB, wxNullBitmap, *LocalizeUnrealEd("ToolTip_3") );
	AddCheckTool(IDM_ANIMSET_VIEWBONENAMES, *LocalizeUnrealEd("ToolTip_4"), ShowBoneNamesB, wxNullBitmap, *LocalizeUnrealEd("ToolTip_4") );
	AddCheckTool(IDM_ANIMSET_VIEWWIREFRAME, *LocalizeUnrealEd("ToolTip_5"), ShowWireframeB, wxNullBitmap, *LocalizeUnrealEd("ToolTip_5") );
	AddCheckTool(IDM_ANIMSET_VIEWREFPOSE, *LocalizeUnrealEd("ShowReferencePose"), ShowRefPoseB, wxNullBitmap, *LocalizeUnrealEd("ShowReferencePose") );
	AddCheckTool(IDM_ANIMSET_VIEWMIRROR, *LocalizeUnrealEd("ShowMirror"), ShowMirrorB, wxNullBitmap, *LocalizeUnrealEd("ShowMirror") );
	AddSeparator();
	AddCheckTool(IDM_ANIMSET_OPENSOCKETMGR, *LocalizeUnrealEd("ToolTip_6"), SocketMgrB, wxNullBitmap, *LocalizeUnrealEd("ToolTip_6") );
	AddSeparator();
	AddTool(IDM_ANIMSET_NOTIFYNEW, NewNotifyB, *LocalizeUnrealEd("NewNotify") );
	AddSeparator();
	AddCheckTool(IDM_ANIMSET_TOGGLECLOTH, *LocalizeUnrealEd("ToggleCloth"), ClothB, wxNullBitmap, *LocalizeUnrealEd("ToggleCloth") );
	AddTool(IDM_ANIMSET_SOFTBODYGENERATE, *LocalizeUnrealEd("SoftBodyGenerate"), SoftBodyGenerateB, *LocalizeUnrealEd("SoftBodyGenerate") );
	AddCheckTool(IDM_ANIMSET_SOFTBODYTOGGLESIM, *LocalizeUnrealEd("SoftBodyToggleSim"), SoftBodyToggleSimB, wxNullBitmap, *LocalizeUnrealEd("SoftBodyToggleSim") );
	AddSeparator();
	AddCheckTool(IDM_ANIMSET_ShowRawAnimation, *LocalizeUnrealEd("ShowUncompressedAnimation"), ShowRawAnimationB, wxNullBitmap, *LocalizeUnrealEd("ShowUncompressedAnimation") );
	AddCheckTool(IDM_ANIMSET_OpenAnimationCompressionDlg, *LocalizeUnrealEd("AnimationCompressionE"), SocketMgrB, wxNullBitmap, *LocalizeUnrealEd("AnimationCompressionE") );
	AddSeparator();
	AddControl(LODLabel);
	AddRadioTool(IDM_ANIMSET_LOD_AUTO, *LocalizeUnrealEd("SetLODAuto"), LODAutoB, wxNullBitmap, *LocalizeUnrealEd("SetLODAuto") );
	AddRadioTool(IDM_ANIMSET_LOD_BASE, *LocalizeUnrealEd("ForceLODBaseMesh"), LODBaseB, wxNullBitmap, *LocalizeUnrealEd("ForceLODBaseMesh") );
	AddRadioTool(IDM_ANIMSET_LOD_1, *LocalizeUnrealEd("ForceLOD1"), LOD1B, wxNullBitmap, *LocalizeUnrealEd("ForceLOD1") );
	AddRadioTool(IDM_ANIMSET_LOD_2, *LocalizeUnrealEd("ForceLOD2"), LOD2B, wxNullBitmap, *LocalizeUnrealEd("ForceLOD2") );
	AddRadioTool(IDM_ANIMSET_LOD_3, *LocalizeUnrealEd("ForceLOD3"), LOD3B, wxNullBitmap, *LocalizeUnrealEd("ForceLOD3") );
	ToggleTool(IDM_ANIMSET_LOD_AUTO, true);
	AddSeparator();
	AddRadioTool(IDM_ANIMSET_SPEED_100,	*LocalizeUnrealEd("FullSpeed"), Speed100B, wxNullBitmap, *LocalizeUnrealEd("FullSpeed") );
	AddRadioTool(IDM_ANIMSET_SPEED_50,	*LocalizeUnrealEd("50Speed"), Speed50B, wxNullBitmap, *LocalizeUnrealEd("50Speed") );
	AddRadioTool(IDM_ANIMSET_SPEED_25,	*LocalizeUnrealEd("25Speed"), Speed25B, wxNullBitmap, *LocalizeUnrealEd("25Speed") );
	AddRadioTool(IDM_ANIMSET_SPEED_10,	*LocalizeUnrealEd("10Speed"), Speed10B, wxNullBitmap, *LocalizeUnrealEd("10Speed") );
	AddRadioTool(IDM_ANIMSET_SPEED_1,	*LocalizeUnrealEd("1Speed"), Speed1B, wxNullBitmap, *LocalizeUnrealEd("1Speed") );
	ToggleTool(IDM_ANIMSET_SPEED_100, true);

	Realize();
}

/*-----------------------------------------------------------------------------
	WxASVMenuBar
-----------------------------------------------------------------------------*/

WxASVMenuBar::WxASVMenuBar()
{
	// File Menu
	FileMenu = new wxMenu();
	Append( FileMenu, *LocalizeUnrealEd("File") );

	FileMenu->Append( IDM_ANIMSET_IMPORTMESHLOD, *LocalizeUnrealEd("ImportMeshLOD"), TEXT("") );
	FileMenu->AppendSeparator();
	FileMenu->Append( IDM_ANIMSET_NEWANISMET, *LocalizeUnrealEd("NewAnimSet"), TEXT("") );
	FileMenu->Append( IDM_ANIMSET_IMPORTPSA, *LocalizeUnrealEd("ImportPSA"), TEXT("") );
	FileMenu->Append( IDM_ANIMSET_IMPORTCOLLADAANIM, *LocalizeUnrealEd("ImportCOLLADAAnim"), TEXT("") );
	FileMenu->AppendSeparator();
	FileMenu->Append( IDM_ANIMSET_NEWMORPHSET, *LocalizeUnrealEd("NewMorphTargetSet"), TEXT("") );
	FileMenu->Append( IDM_ANIMSET_IMPORTMORPHTARGET, *LocalizeUnrealEd("ImportMorphTarget"), TEXT("") );
	FileMenu->Append( IDM_ANIMSET_IMPORTMORPHTARGET_LOD, *LocalizeUnrealEd("ImportMorphTargetLOD"), TEXT("") );	

	// View Menu
	ViewMenu = new wxMenu();
	Append( ViewMenu, *LocalizeUnrealEd("View") );

	ViewMenu->AppendCheckItem( IDM_ANIMSET_VIEWBONES, *LocalizeUnrealEd("ShowSkeleton"), TEXT("") );
	ViewMenu->AppendCheckItem( IDM_ANIMSET_VIEWBONENAMES, *LocalizeUnrealEd("ShowBoneNames"), TEXT("") );
	ViewMenu->AppendCheckItem( IDM_ANIMSET_VIEWREFPOSE, *LocalizeUnrealEd("ShowReferencePose"), TEXT("") );
	ViewMenu->AppendCheckItem( IDM_ANIMSET_VIEWWIREFRAME, *LocalizeUnrealEd("ShowWireframe"), TEXT("") );
	ViewMenu->AppendCheckItem( IDM_ANIMSET_VIEWMIRROR, *LocalizeUnrealEd("ShowMirror"), TEXT("") );
	ViewMenu->AppendCheckItem( IDM_ANIMSET_VIEWFLOOR, *LocalizeUnrealEd("ShowFloor"), TEXT("") );
	ViewMenu->AppendCheckItem( IDM_ANIMSET_VIEWGRID, *LocalizeUnrealEd("ShowGrid"), TEXT("") );
	ViewMenu->AppendCheckItem( IDM_ANIMSET_VIEWSOCKETS, *LocalizeUnrealEd("ShowSockets"), TEXT("") );
	ViewMenu->AppendCheckItem( IDM_ANIMSET_VIEWBOUNDS, *LocalizeUnrealEd("ShowBounds"), TEXT("") );
	ViewMenu->AppendCheckItem( IDM_ANIMSET_VIEWCOLLISION, *LocalizeUnrealEd("ShowCollision"), TEXT("") );
	ViewMenu->AppendCheckItem( IDM_ANIMSET_ShowRawAnimation, *LocalizeUnrealEd("ShowUncompressedAnimation"), TEXT("") );
	ViewMenu->AppendCheckItem( IDM_ANIMSET_VIEWSOFTBODYTETRA, *LocalizeUnrealEd("ShowSoftBodyTetra"), TEXT("") );
	ViewMenu->Check( IDM_ANIMSET_VIEWSOFTBODYTETRA, true );
	ViewMenu->Check( IDM_ANIMSET_VIEWGRID, true );


	// Mesh Menu
	MeshMenu = new wxMenu();
	Append( MeshMenu, *LocalizeUnrealEd("Mesh") );

	MeshMenu->AppendRadioItem( IDM_ANIMSET_LOD_AUTO, *LocalizeUnrealEd("SetLODAuto"), TEXT("") );
	MeshMenu->AppendRadioItem( IDM_ANIMSET_LOD_BASE, *LocalizeUnrealEd("ForceLODBaseMesh"), TEXT("") );
	MeshMenu->AppendRadioItem( IDM_ANIMSET_LOD_1, *LocalizeUnrealEd("ForceLOD1"), TEXT("") );
	MeshMenu->AppendRadioItem( IDM_ANIMSET_LOD_2, *LocalizeUnrealEd("ForceLOD2"), TEXT("") );
	MeshMenu->AppendRadioItem( IDM_ANIMSET_LOD_3, *LocalizeUnrealEd("ForceLOD3"), TEXT("") );
	MeshMenu->Check( IDM_ANIMSET_LOD_AUTO, true);
	MeshMenu->AppendSeparator();
	MeshMenu->Append( IDM_ANIMSET_REMOVELOD, *LocalizeUnrealEd("RemoveLOD"), TEXT("") );
	MeshMenu->AppendSeparator();
	MeshMenu->Append( IDM_ANIMSET_AUTOMIRRORTABLE, *LocalizeUnrealEd("AutoMirrorTable"), TEXT("") );
	MeshMenu->Append( IDM_ANIMSET_CHECKMIRRORTABLE, *LocalizeUnrealEd("CheckMirrorTable"), TEXT("") );
	MeshMenu->Append( IDM_ANIMSET_COPYMIRRORTABLE, *LocalizeUnrealEd("CopyMirrorTableToClipboard"), TEXT("") );
	MeshMenu->Append( IDM_ANIMSET_COPYMIRRORTABLEFROM, *LocalizeUnrealEd("CopyMirrorTableFromMesh"), TEXT("") );
	MeshMenu->AppendSeparator();
	MeshMenu->Append( IDM_ANIMSET_UPDATEBOUNDS, *LocalizeUnrealEd("UpdateBounds"), TEXT("") );
	MeshMenu->Append( IDM_ANIMSET_COPYMESHBONES, *LocalizeUnrealEd("CopyBoneNamesToClipboard"), TEXT("") );
	MeshMenu->Append( IDM_ANIMSET_MERGEMATERIALS, *LocalizeUnrealEd("MergeMeshMaterials"), TEXT("") );
	MeshMenu->AppendSeparator();
	MeshMenu->Append( IDM_ANIMSET_FIXUPMESHBONES, *LocalizeUnrealEd("FixupBoneNames"), TEXT("") );
	MeshMenu->Append( IDM_ANIMSET_SWAPLODSECTIONS, *LocalizeUnrealEd("SwapLODSections"), TEXT("") );
	MeshMenu->AppendSeparator();
	// Socket Manager
	MeshMenu->Append( IDM_ANIMSET_OPENSOCKETMGR, *LocalizeUnrealEd("ToolTip_6"), TEXT("") );
	MeshMenu->AppendSeparator();
	MeshMenu->Append( IDM_ANIMSET_IMPORTMESHWEIGHTS, *LocalizeUnrealEd("ImportMeshWeights"), TEXT("") );
	MeshMenu->AppendCheckItem( IDM_ANIMSET_TOGGLEMESHWEIGHTS, *LocalizeUnrealEd("ToggleMeshWeights"), TEXT("") );

	// AnimSet Menu
	AnimSetMenu = new wxMenu();
	Append( AnimSetMenu, *LocalizeUnrealEd("AnimSet") );

	AnimSetMenu->Append( IDM_ANIMSET_EMPTYSET, *LocalizeUnrealEd("ResetAnimSet"), TEXT("") );
	AnimSetMenu->AppendSeparator();
	AnimSetMenu->Append( IDM_ANIMSET_DELETETRACK, *LocalizeUnrealEd("DeleteTrack"), TEXT("") );
	AnimSetMenu->AppendSeparator();
	AnimSetMenu->Append( IDM_ANIMSET_COPYTRANSLATIONBONENAMES, *LocalizeUnrealEd("CopyTranslationBoneNames"), TEXT("") );
	

	// AnimSeq Menu
	AnimSeqMenu = new wxMenu();
	Append( AnimSeqMenu, *LocalizeUnrealEd("AnimSequence") );

	AnimSeqMenu->Append( IDM_ANIMSET_RENAMESEQ, *LocalizeUnrealEd("RenameSequence"), TEXT("") );
	AnimSeqMenu->Append( IDM_ANIMSET_DELETESEQ, *LocalizeUnrealEd("DeleteSequence"), TEXT("") );
	AnimSeqMenu->Append( IDM_ANIMSET_COPYSEQ,  *LocalizeUnrealEd("CopySequence"), TEXT("") );
	AnimSeqMenu->Append( IDM_ANIMSET_MOVESEQ,  *LocalizeUnrealEd("MoveSequence"), TEXT("") );
	AnimSeqMenu->AppendSeparator();
	AnimSeqMenu->Append( IDM_ANIMSET_MAKESEQADDITIVE,  *LocalizeUnrealEd("MakeSequencesAdditive"), TEXT("") );
	AnimSeqMenu->AppendSeparator();
	AnimSeqMenu->Append( IDM_ANIMSET_COPYSEQNAME, *LocalizeUnrealEd("CopySequenceName"), TEXT("") );
	AnimSeqMenu->Append( IDM_ANIMSET_COPYSEQNAMELIST, *LocalizeUnrealEd("CopySequenceNameList"), TEXT("") );
	AnimSeqMenu->AppendSeparator();
	AnimSeqMenu->Append( IDM_ANIMSET_SEQAPPLYROT, *LocalizeUnrealEd("ApplyRotation"), TEXT("") );
	AnimSeqMenu->Append( IDM_ANIMSET_SEQREZERO, *LocalizeUnrealEd("ReZeroCurrentPosition"), TEXT("") );
	AnimSeqMenu->AppendSeparator();
	AnimSeqMenu->Append( IDM_ANIMSET_SEQDELBEFORE, *LocalizeUnrealEd("RemoveBeforeCurrentPosition"), TEXT("") );
	AnimSeqMenu->Append( IDM_ANIMSET_SEQDELAFTER, *LocalizeUnrealEd("RemoveAfterCurrentPosition"), TEXT("") );

	// Notify Menu

	NotifyMenu = new wxMenu();
	Append( NotifyMenu, *LocalizeUnrealEd("Notifies") );

	NotifyMenu->Append( IDM_ANIMSET_NOTIFYNEW, *LocalizeUnrealEd("NewNotify"), TEXT("") );
	NotifyMenu->Append( IDM_ANIMSET_NOTIFYCOPY, *LocalizeUnrealEd("CopyNotifies"), TEXT("") );
	NotifyMenu->Append( IDM_ANIMSET_NOTIFYSORT, *LocalizeUnrealEd("SortNotifies"), TEXT("") );

	// MorphSet Menu
	MorphSetMenu = new wxMenu();
	Append( MorphSetMenu, *LocalizeUnrealEd("MorphSet") );

	// Morph Menu
	MorphTargetMenu = new wxMenu();
	Append( MorphTargetMenu, *LocalizeUnrealEd("MorphTarget") );

	MorphTargetMenu->Append( IDM_ANIMSET_RENAME_MORPH, *LocalizeUnrealEd("RenameMorph"), TEXT("") );
	MorphTargetMenu->Append( IDM_ANIMSET_DELETE_MORPH, *LocalizeUnrealEd("DeleteMorph"), TEXT("") );

	// AnimationCompresion Menu
	AnimationCompressionMenu = new wxMenu();
	Append( AnimationCompressionMenu, *LocalizeUnrealEd("AnimationCompression") );

	AnimationCompressionMenu->AppendCheckItem( IDM_ANIMSET_OpenAnimationCompressionDlg, *LocalizeUnrealEd("AnimationCompressionE"), TEXT("") );
}

/*-----------------------------------------------------------------------------
	WxASVStatusBar.
-----------------------------------------------------------------------------*/


WxASVStatusBar::WxASVStatusBar( wxWindow* InParent, wxWindowID InID )
	:	wxStatusBar( InParent, InID )
{
	INT Widths[3] = {-3, -3, -3};

	SetFieldsCount(3, Widths);
}

/**
 * @return		The size of the raw animation data in the specified animset.
 */
static INT GetRawSize(const UAnimSet& AnimSet)
{
	INT ResourceSize = 0;
	for( INT SeqIndex = 0 ; SeqIndex < AnimSet.Sequences.Num() ; ++SeqIndex )
	{
		ResourceSize += AnimSet.Sequences(SeqIndex)->GetApproxRawSize();
	}
	return ResourceSize;
}

/**
 * @return		The size of the compressed animation data in the specified animset.
 */
static INT GetCompressedSize(const UAnimSet& AnimSet)
{
	INT ResourceSize = 0;
	for( INT SeqIndex = 0 ; SeqIndex < AnimSet.Sequences.Num() ; ++SeqIndex )
	{
		ResourceSize += AnimSet.Sequences(SeqIndex)->GetApproxCompressedSize();
	}
	return ResourceSize;
}

void WxASVStatusBar::UpdateStatusBar(WxAnimSetViewer* AnimSetViewer)
{
	// SkeletalMesh status text
	FString MeshStatus( *LocalizeUnrealEd("MeshNone") );
	if(AnimSetViewer->SelectedSkelMesh && AnimSetViewer->SelectedSkelMesh->LODModels.Num() > 0)
	{
		const INT NumTris = AnimSetViewer->SelectedSkelMesh->LODModels(0).GetTotalFaces();
		const INT NumBones = AnimSetViewer->SelectedSkelMesh->RefSkeleton.Num();

		MeshStatus = FString::Printf( LocalizeSecure(LocalizeUnrealEd("MeshTrisBones_F"), *AnimSetViewer->SelectedSkelMesh->GetName(), NumTris, NumBones) );
	}
	SetStatusText( *MeshStatus, 0 );

	// AnimSet status text
	FString AnimSetStatus( *LocalizeUnrealEd("AnimSetNone") );
	if(AnimSetViewer->SelectedAnimSet)
	{
		const INT RawSize				= GetRawSize( *AnimSetViewer->SelectedAnimSet );
		const INT CompressedSize		= GetCompressedSize( *AnimSetViewer->SelectedAnimSet );
		const FLOAT RawSizeMB			= static_cast<FLOAT>( RawSize ) / (1024.f * 1024.f);
		const FLOAT CompressedSizeMB	= static_cast<FLOAT>( CompressedSize ) / (1024.f * 1024.f);

		AnimSetStatus = FString::Printf( LocalizeSecure(LocalizeUnrealEd("AnimSet_F"), *AnimSetViewer->SelectedAnimSet->GetName(), CompressedSizeMB, RawSizeMB) );
	}
	SetStatusText( *AnimSetStatus, 1 );

	// AnimSeq status text
	FString AnimSeqStatus( *LocalizeUnrealEd("AnimSeqNone") );
	if(AnimSetViewer->SelectedAnimSeq)
	{
		const INT RawSize				= AnimSetViewer->SelectedAnimSeq->GetApproxRawSize();
		const INT CompressedSize		= AnimSetViewer->SelectedAnimSeq->GetApproxCompressedSize();
		const FLOAT RawSizeKB			= static_cast<FLOAT>( RawSize ) / 1024.f;
		const FLOAT CompressedSizeKB	= static_cast<FLOAT>( CompressedSize ) / 1024.f;
		const FLOAT SeqLength			= AnimSetViewer->SelectedAnimSeq->SequenceLength;

		AnimSeqStatus = FString::Printf( LocalizeSecure(LocalizeUnrealEd("AnimSeq_F"), *AnimSetViewer->SelectedAnimSeq->SequenceName.ToString(), SeqLength, CompressedSizeKB, RawSizeKB) );
	}
	SetStatusText( *AnimSeqStatus, 2 );
}

/*-----------------------------------------------------------------------------
	WxSkeletonTreePopUpMenu
-----------------------------------------------------------------------------*/

WxSkeletonTreePopUpMenu::WxSkeletonTreePopUpMenu(WxAnimSetViewer* AnimSetViewer)
{
	check(AnimSetViewer);

	Append( IDM_ANIMSET_SKELETONTREE_SHOWBONE, *LocalizeUnrealEd("ShowBones"), TEXT("") );
	Append( IDM_ANIMSET_SKELETONTREE_HIDEBONE, *LocalizeUnrealEd("HideBones"), TEXT("") );
	Append( IDM_ANIMSET_SKELETONTREE_SETBONECOLOR, *LocalizeUnrealEd("SetBonesColor"), TEXT("") );
	AppendSeparator();
	Append( IDM_ANIMSET_SKELETONTREE_SHOWCHILDBONE, *LocalizeUnrealEd("ShowChildBones"), TEXT("") );
	Append( IDM_ANIMSET_SKELETONTREE_HIDECHILDBONE, *LocalizeUnrealEd("HideChildBones"), TEXT("") );
	Append( IDM_ANIMSET_SKELETONTREE_SETCHILDBONECOLOR, *LocalizeUnrealEd("SetChildBonesColor"), TEXT("") );
	AppendSeparator();
	Append( IDM_ANIMSET_SKELETONTREE_COPYBONENAME, *LocalizeUnrealEd("CopyBoneNameToClipBoard"), TEXT("") );
}

WxSkeletonTreePopUpMenu::~WxSkeletonTreePopUpMenu()
{
}

