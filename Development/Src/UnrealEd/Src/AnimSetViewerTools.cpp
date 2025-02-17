/*=============================================================================
	AnimSetViewerMain.cpp: AnimSet viewer main
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "EngineAnimClasses.h"
#include "AnimSetViewer.h"
#include "UnColladaImporter.h"
#include "Properties.h"
#include "DlgGenericComboEntry.h"
#include "DlgRename.h"
#include "DlgRotateAnimSequence.h"
#include "BusyCursor.h"
#include "AnimationUtils.h"
#include "SkelImport.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Static helpers
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {
	/**
	 * Recompresses the sequences in specified anim set by applying each sequence's compression scheme.
	 * 
	 */
	static void RecompressSet(UAnimSet* AnimSet, USkeletalMesh* SkeletalMesh)
	{
		for( INT SequenceIndex = 0 ; SequenceIndex < AnimSet->Sequences.Num() ; ++SequenceIndex )
		{
			UAnimSequence* Seq = AnimSet->Sequences( SequenceIndex );
			FAnimationUtils::CompressAnimSequence(Seq, SkeletalMesh, FALSE, FALSE);
		}
	}

	/**
	 * Recompresses the specified sequence in anim set by applying the sequence's compression scheme.
	 * 
	 */
	static void RecompressSequence(UAnimSequence* Seq, USkeletalMesh* SkeletalMesh)
	{
		FAnimationUtils::CompressAnimSequence(Seq, SkeletalMesh, FALSE, FALSE);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WxAnimSetViewer
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WxAnimSetViewer::SetSelectedSkelMesh(USkeletalMesh* InSkelMesh, UBOOL bClearMorphTarget)
{
	check(InSkelMesh);

	// Before we change mesh, clear any component we using for previewing attachments.
	ClearSocketPreviews();

	SelectedSkelMesh = InSkelMesh;

	SkelMeshCombo->Freeze();
	SkelMeshCombo->Clear();
	for(TObjectIterator<USkeletalMesh> It; It; ++It)
	{
		USkeletalMesh* SkelMesh = *It;

		if( !(SkelMesh->GetOutermost()->PackageFlags & PKG_Trash) )
		{
			SkelMeshCombo->Append( *SkelMesh->GetName(), SkelMesh );
		}
	}
	SkelMeshCombo->Thaw();

	// Set combo box to reflect new selection.
	for(UINT i=0; i<SkelMeshCombo->GetCount(); i++)
	{
		if( SkelMeshCombo->GetClientData(i) == SelectedSkelMesh )
		{
			SkelMeshCombo->SetSelection(i); // This won't cause a new IDM_ANIMSET_SKELMESHCOMBO event.
		}
	}

	PreviewSkelComp->SetSkeletalMesh(SelectedSkelMesh);
	PreviewSkelCompRaw->SetSkeletalMesh(SelectedSkelMesh);

	// When changing primary skeletal mesh - reset all extra ones.
	SkelMeshAux1Combo->SetSelection(0);
	PreviewSkelCompAux1->SetSkeletalMesh(NULL);
	PreviewSkelCompAux1->UpdateParentBoneMap();

	SkelMeshAux2Combo->SetSelection(0);
	PreviewSkelCompAux2->SetSkeletalMesh(NULL);
	PreviewSkelCompAux2->UpdateParentBoneMap();

	SkelMeshAux3Combo->SetSelection(0);
	PreviewSkelCompAux3->SetSkeletalMesh(NULL);
	PreviewSkelCompAux3->UpdateParentBoneMap();

	MeshProps->SetObject( NULL, true, false, true );
	MeshProps->SetObject( SelectedSkelMesh, true, false, true );

	// Update the AnimSet combo box to only show AnimSets that can be played on the new skeletal mesh.
	UpdateAnimSetCombo();

	// Update the Socket Manager
	UpdateSocketList();
	SetSelectedSocket(NULL);
	RecreateSocketPreviews();

	// Try and re-select the select AnimSet with the new mesh.
	SetSelectedAnimSet(SelectedAnimSet, false);

	// Select no morph target.
	if(bClearMorphTarget)
	{
		SetSelectedMorphSet(NULL, false);
	}

	// Reset LOD to Auto mode.
	PreviewSkelComp->ForcedLodModel = 0;
	PreviewSkelCompRaw->ForcedLodModel = 0;
	MenuBar->Check( IDM_ANIMSET_LOD_AUTO, true );
	ToolBar->ToggleTool( IDM_ANIMSET_LOD_AUTO, true );

	// Update the buttons used for changing the desired LOD.
	UpdateForceLODButtons();

	// Turn off cloth sim when we change mesh.
	ToolBar->ToggleTool( IDM_ANIMSET_TOGGLECLOTH, false );
	PreviewSkelComp->TermClothSim(NULL);
	PreviewSkelComp->bEnableClothSimulation = FALSE;
	
	// Turn off soft-body sim when we change mesh. Reinitialize data buffers for previewing if we already have generated tetras.
	ToolBar->ToggleTool(IDM_ANIMSET_SOFTBODYTOGGLESIM, FALSE);
	PreviewSkelComp->TermSoftBodySim(NULL);
	PreviewSkelComp->bEnableSoftBodySimulation = FALSE;
	if(InSkelMesh->SoftBodyTetraVertsUnscaled.Num() > 0)
	{
		FlushRenderingCommands();
		PreviewSkelComp->InitSoftBodySimBuffers();
	}
	


	UpdateStatusBar();

	FASVViewportClient* ASVPreviewVC = GetPreviewVC();
	check( ASVPreviewVC );
	ASVPreviewVC->Viewport->Invalidate();

	// Remember we want to use this mesh with the current anim/morph set.
	if(SelectedSkelMesh && SelectedAnimSet)
	{
		FString MeshName = SelectedSkelMesh->GetPathName();
		SelectedAnimSet->PreviewSkelMeshName = FName( *MeshName );
	}

	// Fill Skeleton Tree.
	FillSkeletonTree();
}

// If trying to set the AnimSet to one that cannot be played on selected SkelMesh, show message and just select first instead.
void WxAnimSetViewer::SetSelectedAnimSet(UAnimSet* InAnimSet, UBOOL bAutoSelectMesh)
{
	// Only allow selection of compatible AnimSets. 
	// AnimSetCombo should contain all AnimSets that can played on SelectedSkelMesh.

	// In case we have loaded some new packages, rebuild the list before finding the entry.
	UpdateAnimSetCombo();

	INT AnimSetIndex = INDEX_NONE;
	for(UINT i=0; i<AnimSetCombo->GetCount(); i++)
	{
		if(AnimSetCombo->GetClientData(i) == InAnimSet)
		{
			AnimSetIndex = i;
		}
	}

	// If specified AnimSet is not compatible with current skeleton, show message and pick the first one.
	if(AnimSetIndex == INDEX_NONE)
	{
		if(InAnimSet)
		{
			appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("Error_AnimSetCantBePlayedOnSM"), *InAnimSet->GetName(), *SelectedSkelMesh->GetName()) );
		}

		AnimSetIndex = 0;
	}

	// Handle case where combo is empty - select no AnimSet
	if(AnimSetCombo->GetCount() > 0)
	{
		AnimSetCombo->SetSelection(AnimSetIndex);

		// Assign selected AnimSet
		SelectedAnimSet = (UAnimSet*)AnimSetCombo->GetClientData(AnimSetIndex);

		// Add newly selected AnimSet to the AnimSets array of the preview skeletal mesh.
		PreviewSkelComp->AnimSets.Empty();
		PreviewSkelCompRaw->AnimSets.Empty();
		PreviewSkelComp->AnimSets.AddItem(SelectedAnimSet);
		PreviewSkelCompRaw->AnimSets.AddItem(SelectedAnimSet);

		AnimSetProps->SetObject( NULL, true, false, true );
		AnimSetProps->SetObject( SelectedAnimSet, true, false, true );
	}
	else
	{
		SelectedAnimSet = NULL;

		PreviewSkelComp->AnimSets.Empty();
		PreviewSkelCompRaw->AnimSets.Empty();

		AnimSetProps->SetObject( NULL, true, false, true );
	}

	// Refresh the animation sequence list.
	UpdateAnimSeqList();

	// Select the first sequence (if present) - or none at all.
	if( AnimSeqList->GetCount() > 0 )
	{
		SetSelectedAnimSequence( (UAnimSequence*)(AnimSeqList->GetClientData(0)) );
	}
	else
	{
		SetSelectedAnimSequence( NULL );
	}

	UpdateStatusBar();

	// The menu title bar displays the full path name of the selected AnimSet
	FString WinTitle;
	if(SelectedAnimSet)
	{
		WinTitle = FString::Printf( LocalizeSecure(LocalizeUnrealEd("AnimSetEditor_F"), *SelectedAnimSet->GetPathName()) );
	}
	else
	{
		WinTitle = FString::Printf( *LocalizeUnrealEd("AnimSetEditor") );
	}

	SetTitle( *WinTitle );


	// See if there is a skeletal mesh we would like to use with this AnimSet
	if(bAutoSelectMesh && SelectedAnimSet->PreviewSkelMeshName != NAME_None)
	{
		USkeletalMesh* PreviewSkelMesh = (USkeletalMesh*)UObject::StaticFindObject( USkeletalMesh::StaticClass(), ANY_PACKAGE, *SelectedAnimSet->PreviewSkelMeshName.ToString() );
		if(PreviewSkelMesh)
		{
			SetSelectedSkelMesh(PreviewSkelMesh, false);
		}
	}
}

void WxAnimSetViewer::SetSelectedAnimSequence(UAnimSequence* InAnimSeq)
{
	if(!InAnimSeq)
	{
		PreviewAnimNode->SetAnim(NAME_None);
		PreviewAnimNodeRaw->SetAnim(NAME_None);
		SelectedAnimSeq = PreviewAnimNode->AnimSeq;
		AnimSeqProps->SetObject( NULL, true, false, true );
	}
	else
	{
		INT AnimSeqIndex = INDEX_NONE;
		for(UINT i=0; i<AnimSeqList->GetCount(); i++)
		{
			if( AnimSeqList->GetClientData(i) == InAnimSeq )
			{
				AnimSeqIndex = i;
			}
		}		

		if(AnimSeqIndex == INDEX_NONE)
		{
			PreviewAnimNode->SetAnim(NAME_None);
			PreviewAnimNodeRaw->SetAnim(NAME_None);
			SelectedAnimSeq = NULL;
			AnimSeqProps->SetObject( NULL, true, false, true );
		}
		else
		{
			AnimSeqList->SetSelection( AnimSeqIndex );
			PreviewAnimNode->SetAnim( InAnimSeq->SequenceName );
			PreviewAnimNodeRaw->SetAnim( InAnimSeq->SequenceName );
			SelectedAnimSeq = InAnimSeq;

			AnimSeqProps->SetObject( NULL, true, false, true );
			AnimSeqProps->SetObject( SelectedAnimSeq, true, false, true );
		}
	}

	UpdateStatusBar();
}

/** Set the supplied MorphTargetSet as the selected one. */
void WxAnimSetViewer::SetSelectedMorphSet(UMorphTargetSet* InMorphSet, UBOOL bAutoSelectMesh)
{
	// In case we have loaded some new packages, rebuild the list before finding the entry.
	UpdateMorphSetCombo();

	INT MorphSetIndex = INDEX_NONE;
	for(UINT i=0; i<MorphSetCombo->GetCount(); i++)
	{
		if(MorphSetCombo->GetClientData(i) == InMorphSet)
		{
			MorphSetIndex = i;
		}
	}

	// Combo should never be empty - there is always a -None- slot.
	check(MorphSetCombo->GetCount() > 0);
	MorphSetCombo->SetSelection(MorphSetIndex);

	SelectedMorphSet = (UMorphTargetSet*)MorphSetCombo->GetClientData(MorphSetIndex);
	// Note, it may be NULL at this point, because combo contains a 'None' entry.

	// assign MorphTargetSet to array in SkeletalMeshComponent. 
	PreviewSkelComp->MorphSets.Empty();
	PreviewSkelCompRaw->MorphSets.Empty();
	if(SelectedMorphSet)
	{
		PreviewSkelComp->MorphSets.AddItem(SelectedMorphSet);
		PreviewSkelCompRaw->MorphSets.AddItem(SelectedMorphSet);
	}

	// Refresh the animation sequence list.
	UpdateMorphTargetList();

	// Select the first sequence (if present) - or none at all.
	if( MorphTargetList->GetCount() > 0 )
	{
		SetSelectedMorphTarget( (UMorphTarget*)(MorphTargetList->GetClientData(0)) );
	}
	else
	{
		SetSelectedMorphTarget( NULL );
	}

	UpdateStatusBar();

	// Set the BaseSkelMesh as the selected skeletal mesh.
	if( bAutoSelectMesh && 
		SelectedMorphSet && 
		SelectedMorphSet->BaseSkelMesh && 
		SelectedMorphSet->BaseSkelMesh != SelectedSkelMesh)
	{
		SetSelectedSkelMesh(SelectedMorphSet->BaseSkelMesh, false);
	}
}

/** Set the selected MorphTarget as the desired. */
void WxAnimSetViewer::SetSelectedMorphTarget(UMorphTarget* InMorphTarget)
{
	if(!InMorphTarget)
	{
		SelectedMorphTarget = NULL;
		PreviewMorphPose->SetMorphTarget(NAME_None);
	}
	else
	{
		INT MorphTargetIndex = INDEX_NONE;
		for(UINT i=0; i<MorphTargetList->GetCount(); i++)
		{
			if( MorphTargetList->GetClientData(i) == InMorphTarget )
			{
				MorphTargetIndex = i;
			}
		}		

		if(MorphTargetIndex == INDEX_NONE)
		{
			SelectedMorphTarget = NULL;
			PreviewMorphPose->SetMorphTarget(NAME_None);
		}
		else
		{
			MorphTargetList->SetSelection( MorphTargetIndex );
			SelectedMorphTarget = InMorphTarget;
			PreviewMorphPose->SetMorphTarget(InMorphTarget->GetFName());
		}
	}
}

void WxAnimSetViewer::ImportCOLLADAAnim()
{
	if(!SelectedAnimSet)
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Error_SelectAnimSetForImport") );
		return;
	}

	WxFileDialog ImportFileDialog( this, 
		*LocalizeUnrealEd("ImportCOLLADA_Anim_File"), 
		*(GApp->LastDir[LD_COLLADA_ANIM]),
		TEXT(""),
		TEXT("COLLADA .dae files|*.dae"),
		wxOPEN | wxFILE_MUST_EXIST | wxMULTIPLE,
		wxDefaultPosition);

	if( ImportFileDialog.ShowModal() == wxID_OK )
	{
		// Remember if this AnimSet was brand new
		const UBOOL bSetWasNew = SelectedAnimSet->TrackBoneNames.Num() == 0 ? TRUE : FALSE;

		wxArrayString ImportFilePaths;
		ImportFileDialog.GetPaths(ImportFilePaths);

		for(UINT FileIndex = 0; FileIndex < ImportFilePaths.Count(); FileIndex++)
		{
			const FFilename Filename( (const TCHAR*) ImportFilePaths[FileIndex] );
			GApp->LastDir[LD_COLLADA_ANIM] = Filename.GetPath(); // Save path as default for next time.

			UEditorEngine::ImportCOLLADAANIMIntoAnimSet( SelectedAnimSet, *Filename, SelectedSkelMesh );

			SelectedAnimSet->MarkPackageDirty();
		}

		// If this set was new - we have just created the bone-track-table.
		// So we now need to check if we can still play it on our selected SkeletalMesh.
		// If we can't, we look for a few one which can play it.
		// If we can't find any mesh to play this new set on, we fail the import and reset the AnimSet.
		if(bSetWasNew)
		{
			if( !SelectedAnimSet->CanPlayOnSkeletalMesh(SelectedSkelMesh) )
			{
				USkeletalMesh* NewSkelMesh = NULL;
				for(TObjectIterator<USkeletalMesh> It; It && !NewSkelMesh; ++It)
				{
					USkeletalMesh* TestSkelMesh = *It;
					if( SelectedAnimSet->CanPlayOnSkeletalMesh(TestSkelMesh) )
					{
						NewSkelMesh = TestSkelMesh;
					}
				}

				if(NewSkelMesh)
				{
					SetSelectedSkelMesh(NewSkelMesh, TRUE);
				}
				else
				{
					appMsgf( AMT_OK, *LocalizeUnrealEd("Error_PSACouldNotBePlayed") );
					SelectedAnimSet->ResetAnimSet();
					return;
				}
			}
		}

		// Refresh AnimSet combo to show new number of sequences in this AnimSet.
		const INT CurrentSelection = AnimSetCombo->GetSelection();
		UpdateAnimSetCombo();
		AnimSetCombo->SetSelection(CurrentSelection);

		// Refresh AnimSequence list box to show any new anims.
		UpdateAnimSeqList();

		// Reselect current animation sequence. If none selected, pick the first one in the box (if not empty).
		if(SelectedAnimSeq)
		{
			SetSelectedAnimSequence(SelectedAnimSeq);
		}
		else
		{
			if(AnimSeqList->GetCount() > 0)
			{
				SetSelectedAnimSequence( (UAnimSequence*)(AnimSeqList->GetClientData(0)) );
			}
		}

		SelectedAnimSet->MarkPackageDirty();
	}
	return; 	
}

void WxAnimSetViewer::ImportPSA()
{
	if(!SelectedAnimSet)
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Error_SelectAnimSetForImport") );
		return;
	}

	// Prevent animations from being imported into a cooked anim set.
	UPackage* Package = SelectedAnimSet->GetOutermost();
	if( Package->PackageFlags & PKG_Cooked )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Error_OperationDisallowedOnCookedContent") );
		return;
	}

	WxFileDialog ImportFileDialog( this, 
		*LocalizeUnrealEd("ImportPSAFile"), 
		*(GApp->LastDir[LD_PSA]),
		TEXT(""),
		TEXT("PSA files|*.psa"),
		wxOPEN | wxFILE_MUST_EXIST | wxMULTIPLE,
		wxDefaultPosition);

	if( ImportFileDialog.ShowModal() == wxID_OK )
	{
		// Remember if this AnimSet was brand new
		const UBOOL bSetWasNew = SelectedAnimSet->TrackBoneNames.Num() == 0 ? TRUE : FALSE;

		wxArrayString ImportFilePaths;
		ImportFileDialog.GetPaths(ImportFilePaths);

		for(UINT FileIndex = 0; FileIndex < ImportFilePaths.Count(); FileIndex++)
		{
			const FFilename Filename( (const TCHAR*)ImportFilePaths[FileIndex] );
			GApp->LastDir[LD_PSA] = Filename.GetPath(); // Save path as default for next time.

			UEditorEngine::ImportPSAIntoAnimSet( SelectedAnimSet, *Filename, SelectedSkelMesh );

			SelectedAnimSet->MarkPackageDirty();
		}

		// If this set was new - we have just created the bone-track-table.
		// So we now need to check if we can still play it on our selected SkeletalMesh.
		// If we can't, we look for a few one which can play it.
		// If we can't find any mesh to play this new set on, we fail the import and reset the AnimSet.
		if(bSetWasNew)
		{
			if( !SelectedAnimSet->CanPlayOnSkeletalMesh(SelectedSkelMesh) )
			{
				USkeletalMesh* NewSkelMesh = NULL;
				for(TObjectIterator<USkeletalMesh> It; It && !NewSkelMesh; ++It)
				{
					USkeletalMesh* TestSkelMesh = *It;
					if( SelectedAnimSet->CanPlayOnSkeletalMesh(TestSkelMesh) )
					{
						NewSkelMesh = TestSkelMesh;
					}
				}

				if(NewSkelMesh)
				{
					SetSelectedSkelMesh(NewSkelMesh, true);
				}
				else
				{
					appMsgf( AMT_OK, *LocalizeUnrealEd("Error_PSACouldNotBePlayed") );
					SelectedAnimSet->ResetAnimSet();
					return;
				}
			}
		}

		// Refresh AnimSet combo to show new number of sequences in this AnimSet.
		const INT CurrentSelection = AnimSetCombo->GetSelection();
		UpdateAnimSetCombo();
		AnimSetCombo->SetSelection(CurrentSelection);
			
		// Refresh AnimSequence list box to show any new anims.
		UpdateAnimSeqList();

		// Reselect current animation sequence. If none selected, pick the first one in the box (if not empty).
		if(SelectedAnimSeq)
		{
			SetSelectedAnimSequence(SelectedAnimSeq);
		}
		else
		{
			if(AnimSeqList->GetCount() > 0)
			{
				SetSelectedAnimSequence( (UAnimSequence*)(AnimSeqList->GetClientData(0)) );
			}
		}

		SelectedAnimSet->MarkPackageDirty();
	}
}

IMPLEMENT_COMPARE_CONSTREF( BYTE, AnimSetViewerTools, { return (A - B); } )

/**
* Import the temporary skeletal mesh into the specified LOD of the currently selected skeletal mesh
*
* @param InSkeletalMesh - newly created mesh used as LOD
* @param DesiredLOD - the LOD index to import into. A new LOD entry is created if one doesn't exist
*/
void WxAnimSetViewer::ImportMeshLOD(USkeletalMesh* InSkeletalMesh, INT DesiredLOD)
{
	check(InSkeletalMesh);
	// Now we copy the base FStaticLODModel from the imported skeletal mesh as the new LOD in the selected mesh.
	check(InSkeletalMesh->LODModels.Num() == 1);

	// Names of root bones must match.
	if(InSkeletalMesh->RefSkeleton(0).Name != SelectedSkelMesh->RefSkeleton(0).Name)
	{
		appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("LODRootNameIncorrect"), *InSkeletalMesh->RefSkeleton(0).Name.ToString(), *SelectedSkelMesh->RefSkeleton(0).Name.ToString()) );
		return;
	}

	// We do some checking here that for every bone in the mesh we just imported, it's in our base ref skeleton, and the parent is the same.
	for(INT i=0; i<InSkeletalMesh->RefSkeleton.Num(); i++)
	{
		INT LODBoneIndex = i;
		FName LODBoneName = InSkeletalMesh->RefSkeleton(LODBoneIndex).Name;
		INT BaseBoneIndex = SelectedSkelMesh->MatchRefBone(LODBoneName);
		if( BaseBoneIndex == INDEX_NONE )
		{
			// If we could not find the bone from this LOD in base mesh - we fail.
			appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("LODBoneDoesNotMatch"), *LODBoneName.ToString(), *SelectedSkelMesh->GetName()) );
			return;
		}

		if(i>0)
		{
			INT LODParentIndex = InSkeletalMesh->RefSkeleton(LODBoneIndex).ParentIndex;
			FName LODParentName = InSkeletalMesh->RefSkeleton(LODParentIndex).Name;

			INT BaseParentIndex = SelectedSkelMesh->RefSkeleton(BaseBoneIndex).ParentIndex;
			FName BaseParentName = SelectedSkelMesh->RefSkeleton(BaseParentIndex).Name;

			if(LODParentName != BaseParentName)
			{
				// If bone has different parents, display an error and don't allow import.
				appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("LODBoneHasIncorrectParent"), *LODBoneName.ToString(), *LODParentName.ToString(), *BaseParentName.ToString()) );
				return;
			}
		}
	}

	FStaticLODModel& NewLODModel = InSkeletalMesh->LODModels(0);

	// Enforce LODs having only single-influence vertices.
	UBOOL bCheckSingleInfluence;
	GConfig->GetBool( TEXT("AnimSetViewer"), TEXT("CheckSingleInfluenceLOD"), bCheckSingleInfluence, GEditorIni );
	if( bCheckSingleInfluence && 
		DesiredLOD > 0 )
	{
		for(INT ChunkIndex = 0;ChunkIndex < NewLODModel.Chunks.Num();ChunkIndex++)
		{
			if(NewLODModel.Chunks(ChunkIndex).SoftVertices.Num() > 0)
			{
				appMsgf( AMT_OK, *LocalizeUnrealEd("LODHasSoftVertices") );
			}
		}
	}

	// If this LOD is going to be the lowest one, we check all bones we have sockets on are present in it.
	if( DesiredLOD == SelectedSkelMesh->LODModels.Num() || 
		DesiredLOD == SelectedSkelMesh->LODModels.Num()-1 )
	{
		for(INT i=0; i<SelectedSkelMesh->Sockets.Num(); i++)
		{
			// Find bone index the socket is attached to.
			USkeletalMeshSocket* Socket = SelectedSkelMesh->Sockets(i);
			INT SocketBoneIndex = InSkeletalMesh->MatchRefBone( Socket->BoneName );

			// If this LOD does not contain the socket bone, abort import.
			if( SocketBoneIndex == INDEX_NONE )
			{
				appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("LODMissingSocketBone"), *Socket->BoneName.ToString(), *Socket->SocketName.ToString()) );
				return;
			}
		}
	}

	// Fix up the ActiveBoneIndices array.
	for(INT i=0; i<NewLODModel.ActiveBoneIndices.Num(); i++)
	{
		INT LODBoneIndex = NewLODModel.ActiveBoneIndices(i);
		FName LODBoneName = InSkeletalMesh->RefSkeleton(LODBoneIndex).Name;
		INT BaseBoneIndex = SelectedSkelMesh->MatchRefBone(LODBoneName);
		NewLODModel.ActiveBoneIndices(i) = BaseBoneIndex;
	}

	// Fix up the chunk BoneMaps.
	for(INT ChunkIndex = 0;ChunkIndex < NewLODModel.Chunks.Num();ChunkIndex++)
	{
		FSkelMeshChunk& Chunk = NewLODModel.Chunks(ChunkIndex);
		for(INT i=0; i<Chunk.BoneMap.Num(); i++)
		{
			INT LODBoneIndex = Chunk.BoneMap(i);
			FName LODBoneName = InSkeletalMesh->RefSkeleton(LODBoneIndex).Name;
			INT BaseBoneIndex = SelectedSkelMesh->MatchRefBone(LODBoneName);
			Chunk.BoneMap(i) = BaseBoneIndex;
		}
	}

	// Create the RequiredBones array in the LODModel from the ref skeleton.
	NewLODModel.RequiredBones.Empty();
	for(INT i=0; i<InSkeletalMesh->RefSkeleton.Num(); i++)
	{
		FName LODBoneName = InSkeletalMesh->RefSkeleton(i).Name;
		INT BaseBoneIndex = SelectedSkelMesh->MatchRefBone(LODBoneName);
		if(BaseBoneIndex != INDEX_NONE)
		{
			NewLODModel.RequiredBones.AddItem(BaseBoneIndex);
		}
	}

	// Also sort the RequiredBones array to be strictly increasing.
	Sort<USE_COMPARE_CONSTREF(BYTE, AnimSetViewerTools)>( &NewLODModel.RequiredBones(0), NewLODModel.RequiredBones.Num() );

	// To be extra-nice, we apply the difference between the root transform of the meshes to the verts.
	FMatrix LODToBaseTransform = InSkeletalMesh->GetRefPoseMatrix(0).Inverse() * SelectedSkelMesh->GetRefPoseMatrix(0);

	for(INT ChunkIndex = 0;ChunkIndex < NewLODModel.Chunks.Num();ChunkIndex++)
	{
		FSkelMeshChunk& Chunk = NewLODModel.Chunks(ChunkIndex);
		// Fix up rigid verts.
		for(INT i=0; i<Chunk.RigidVertices.Num(); i++)
		{
			Chunk.RigidVertices(i).Position = LODToBaseTransform.TransformFVector( Chunk.RigidVertices(i).Position );
			Chunk.RigidVertices(i).TangentX = LODToBaseTransform.TransformNormal( Chunk.RigidVertices(i).TangentX );
			Chunk.RigidVertices(i).TangentY = LODToBaseTransform.TransformNormal( Chunk.RigidVertices(i).TangentY );
			Chunk.RigidVertices(i).TangentZ = LODToBaseTransform.TransformNormal( Chunk.RigidVertices(i).TangentZ );
		}

		// Fix up soft verts.
		for(INT i=0; i<Chunk.SoftVertices.Num(); i++)
		{
			Chunk.SoftVertices(i).Position = LODToBaseTransform.TransformFVector( Chunk.SoftVertices(i).Position );
			Chunk.SoftVertices(i).TangentX = LODToBaseTransform.TransformNormal( Chunk.SoftVertices(i).TangentX );
			Chunk.SoftVertices(i).TangentY = LODToBaseTransform.TransformNormal( Chunk.SoftVertices(i).TangentY );
			Chunk.SoftVertices(i).TangentZ = LODToBaseTransform.TransformNormal( Chunk.SoftVertices(i).TangentZ );
		}
	}

	// Shut down the skeletal mesh component that is previewing this mesh.
	{
		FComponentReattachContext ReattachContextPreview(PreviewSkelComp);
		FComponentReattachContext ReattachContextPreviewRaw(PreviewSkelCompRaw);

		// If we want to add this as a new LOD to this mesh - add to LODModels/LODInfo array.
		if(DesiredLOD == SelectedSkelMesh->LODModels.Num())
		{
			new(SelectedSkelMesh->LODModels)FStaticLODModel();

			// Add element to LODInfo array.
			SelectedSkelMesh->LODInfo.AddZeroed();
			check( SelectedSkelMesh->LODInfo.Num() == SelectedSkelMesh->LODModels.Num() );
			SelectedSkelMesh->LODInfo(DesiredLOD) = InSkeletalMesh->LODInfo(0);
		}

		// Set up LODMaterialMap to number of materials in new mesh.
		FSkeletalMeshLODInfo& LODInfo = SelectedSkelMesh->LODInfo(DesiredLOD);
		LODInfo.LODMaterialMap.Empty();

		// Now set up the material mapping array.
		for(INT MatIdx = 0; MatIdx < InSkeletalMesh->Materials.Num(); MatIdx++)
		{
			// Try and find the auto-assigned material in the array.
			INT LODMatIndex = SelectedSkelMesh->Materials.FindItemIndex( InSkeletalMesh->Materials(MatIdx) );

			// If we didn't just use the index - but make sure its within range of the Materials array.
			if(LODMatIndex == INDEX_NONE)
			{
				LODMatIndex = ::Clamp(MatIdx, 0, SelectedSkelMesh->Materials.Num() - 1);
			}

			LODInfo.LODMaterialMap.AddItem(LODMatIndex);
		}

		// Assign new FStaticLODModel to desired slot in selected skeletal mesh.
		SelectedSkelMesh->LODModels(DesiredLOD) = NewLODModel;
		// rebuild vertex buffers and reinit RHI resources
		SelectedSkelMesh->PreEditChange(NULL);
		SelectedSkelMesh->PostEditChange(NULL);								

		// ReattachContexts go out of scope here, reattaching skel components to the scene.
	}
}

/** Import a .PSK file as an LOD of the selected SkeletalMesh. */
void WxAnimSetViewer::ImportMeshLOD()
{
	if(!SelectedSkelMesh)
	{
		return;
	}

	const UPackage* SkelMeshPackage = SelectedSkelMesh->GetOutermost();
	if( SkelMeshPackage->PackageFlags & PKG_Cooked )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Error_OperationDisallowedOnCookedContent") );
		return;
	}

	// First, display the file open dialog for selecting the .PSK file.
	WxFileDialog ImportFileDialog( this, 
		*LocalizeUnrealEd("ImportMeshLOD"), 
		*(GApp->LastDir[LD_PSA]),
		TEXT(""),
		TEXT("PSK files|*.psk"),
		wxOPEN | wxFILE_MUST_EXIST,
		wxDefaultPosition);

	// Only continue if we pressed OK and have only one file selected.
	if( ImportFileDialog.ShowModal() == wxID_OK )
	{
		wxArrayString ImportFilePaths;
		ImportFileDialog.GetPaths(ImportFilePaths);

		if(ImportFilePaths.Count() == 0)
		{
			appMsgf( AMT_OK, *LocalizeUnrealEd("NoFileSelectedForLOD") );
		}
		else if(ImportFilePaths.Count() > 1)
		{
			appMsgf( AMT_OK, *LocalizeUnrealEd("MultipleFilesSelectedForLOD") );
		}
		else
		{
			// Now display combo to choose which LOD to import this mesh as.
			TArray<FString> LODStrings;
			LODStrings.AddZeroed( SelectedSkelMesh->LODModels.Num() );
			for(INT i=0; i<SelectedSkelMesh->LODModels.Num(); i++)
			{
				LODStrings(i) = FString::Printf( TEXT("%d"), i+1 );
			}

			WxDlgGenericComboEntry dlg;
			if( dlg.ShowModal( TEXT("ChooseLODLevel"), TEXT("LODLevel:"), LODStrings, 0, TRUE ) == wxID_OK )
			{
				INT DesiredLOD = dlg.GetComboBox().GetSelection() + 1;

				// If the LOD we want
				if( DesiredLOD > 0 && DesiredLOD <= SelectedSkelMesh->LODModels.Num() )
				{
					FFilename Filename = (const TCHAR*)ImportFilePaths[0];
					GApp->LastDir[LD_PSA] = Filename.GetPath(); // Save path as default for next time.

					// Load the data from the file into a byte array.
					TArray<BYTE> Data;
					if( appLoadFileToArray( Data, *Filename ) )
					{
						Data.AddItem( 0 );
						const BYTE* Ptr = &Data( 0 );

						// Use the SkeletalMeshFactory to load this SkeletalMesh into a temporary SkeletalMesh.
						USkeletalMeshFactory* SkelMeshFact = new USkeletalMeshFactory();
						USkeletalMesh* TempSkelMesh = (USkeletalMesh*)SkelMeshFact->FactoryCreateBinary( 
							USkeletalMesh::StaticClass(), UObject::GetTransientPackage(), NAME_None, 0, NULL, NULL, Ptr, Ptr+Data.Num()-1, GWarn );
						if( TempSkelMesh )
						{
							ImportMeshLOD(TempSkelMesh,DesiredLOD);

							// Update buttons to reflect new LOD.
							UpdateForceLODButtons();

							// Pop up a box saying it worked.
							appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("LODImportSuccessful"), DesiredLOD) );

							// Mark package containing skeletal mesh as dirty.
							SelectedSkelMesh->MarkPackageDirty();
						}
					}
				}
			}
		}
	}
}

/**
* Import a .psk file as a skeletal mesh and copy its bone influence weights 
* to the existing skeletal mesh.  The psk for the base skeletal mesh must also be specified
*/
void WxAnimSetViewer::ImportMeshWeights()
{
	if( !SelectedSkelMesh )
	{
		return;
	}

	const UPackage* SkelMeshPackage = SelectedSkelMesh->GetOutermost();
	if( SkelMeshPackage->PackageFlags & PKG_Cooked )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Error_OperationDisallowedOnCookedContent") );
		return;
	}

	// make a list of existing LOD levels to choose from
	TArray<FString> LODStrings;
	LODStrings.AddZeroed( SelectedSkelMesh->LODModels.Num() );
	for(INT i=0; i<SelectedSkelMesh->LODModels.Num(); i++)
	{
		LODStrings(i) = FString::Printf( TEXT("%d"), i );
	}

	// show dialog for choosing which LOD level to import into
	WxDlgGenericComboEntry dlg;
	if( dlg.ShowModal( TEXT("ChooseLODLevel"), TEXT("ExistingLODLevel:"), LODStrings, 0, TRUE ) == wxID_OK )
	{
		INT DesiredLOD = dlg.GetComboBox().GetSelection();

		// Only import if the LOD we want exists
		if( SelectedSkelMesh->LODModels.IsValidIndex(DesiredLOD) )
		{
			// First, display the file open dialog for selecting the base mesh .PSK file.
			WxFileDialog ImportFileDialogBaseMesh( this, 
				*LocalizeUnrealEd("ImportMeshWeightsFilenameBase"), 
				*(GApp->LastDir[LD_PSA]),
				TEXT(""),
				TEXT("PSK files|*.psk"),
				wxOPEN | wxFILE_MUST_EXIST,
				wxDefaultPosition);

			// First, display the file open dialog for selecting the weights mesh .PSK file.
			WxFileDialog ImportFileDialogWeightsMesh( this, 
				*LocalizeUnrealEd("ImportMeshWeightsFilename"), 
				*(GApp->LastDir[LD_PSA]),
				TEXT(""),
				TEXT("PSK files|*.psk"),
				wxOPEN | wxFILE_MUST_EXIST,
				wxDefaultPosition);

			wxArrayString ImportFilePathsBaseMesh;
			FFilename BaseMeshFilename;
			// Only continue if we pressed OK and have only one file selected. for the base mesh
			if( ImportFileDialogBaseMesh.ShowModal() == wxID_OK )
			{			
				ImportFileDialogBaseMesh.GetPaths(ImportFilePathsBaseMesh);

				if( ImportFilePathsBaseMesh.Count() == 0 )
				{
					appMsgf( AMT_OK, *LocalizeUnrealEd("NoFileSelectedForLOD") );
				}
				else if( ImportFilePathsBaseMesh.Count() > 1 )
				{
					appMsgf( AMT_OK, *LocalizeUnrealEd("MultipleFilesSelectedForLOD") );
				}
				else
				{
					BaseMeshFilename = (const TCHAR*)ImportFilePathsBaseMesh[0];
					GApp->LastDir[LD_PSA] = BaseMeshFilename.GetPath(); // Save path as default for next time.
				}
			}

			wxArrayString ImportFilePathsWeightsMesh;
			FFilename WeightsMeshFilename;
			// Only continue if we pressed OK and have only one file selected. for the base mesh			
			if( ImportFileDialogWeightsMesh.ShowModal() == wxID_OK )
			{				
				ImportFileDialogWeightsMesh.GetPaths(ImportFilePathsWeightsMesh);

				if( ImportFilePathsWeightsMesh.Count() == 0 )
				{
					appMsgf( AMT_OK, *LocalizeUnrealEd("NoFileSelectedForLOD") );
				}
				else if( ImportFilePathsWeightsMesh.Count() > 1 )
				{
					appMsgf( AMT_OK, *LocalizeUnrealEd("MultipleFilesSelectedForLOD") );
				}
				else
				{
					WeightsMeshFilename = (const TCHAR*)ImportFilePathsWeightsMesh[0];
					GApp->LastDir[LD_PSA] = WeightsMeshFilename.GetPath(); // Save path as default for next time.
				}
			}

			if( BaseMeshFilename != TEXT("") && 
				WeightsMeshFilename != TEXT("") )
			{
				// Load the data from the files into byte arrays
				TArray<BYTE> BaseMeshData;
				TArray<BYTE> WeightsMeshData;

				if( appLoadFileToArray(BaseMeshData, *BaseMeshFilename) &&
					appLoadFileToArray(WeightsMeshData, *WeightsMeshFilename) )
				{
					BaseMeshData.AddItem(0);
					WeightsMeshData.AddItem(0);

					const BYTE* BaseMeshDataPtr = &BaseMeshData(0);
					BYTE* WeightsMeshDataPtr = &WeightsMeshData(0);

					// import raw mesh data and use it as optional data for importing mesh weights
					FSkelMeshOptionalImportData MeshImportData;
					MeshImportData.RawMeshInfluencesData.ImportFromFile( WeightsMeshDataPtr, WeightsMeshDataPtr + WeightsMeshData.Num()-1 );

					GWarn->BeginSlowTask( TEXT("Importing Mesh Model Weights"), TRUE);

					// Use the SkeletalMeshFactory to load this SkeletalMesh into a temporary SkeletalMesh.
					// pass in the optional mesh weight import data 
					USkeletalMeshFactory* SkelMeshFact = new USkeletalMeshFactory(&MeshImportData);
					USkeletalMesh* TempSkelMesh = (USkeletalMesh*)SkelMeshFact->FactoryCreateBinary( 
						USkeletalMesh::StaticClass(), 
						UObject::GetTransientPackage(), 
						NAME_None, 
						RF_Transient, 
						NULL, 
						NULL, 
						BaseMeshDataPtr, 
						BaseMeshDataPtr + BaseMeshData.Num()-1, 
						GWarn 
						);

					if( TempSkelMesh )
					{
						ImportMeshLOD(TempSkelMesh,DesiredLOD);
					}

					GWarn->EndSlowTask();
					
				}
			}
		}
	}

}


/**
* Displays and handles dialogs necessary for importing 
* a new morph target mesh from file. The new morph
* target is placed in the currently selected MorphTargetSet
*
* @param bImportToLOD - if TRUE then new files will be treated as morph target LODs 
* instead of new morph target resources
*/
void WxAnimSetViewer::ImportMorphTarget(UBOOL bImportToLOD)
{
	if( !SelectedMorphSet )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Error_SelectMorphSetForImport") );
		return;
	}

	// The MorphTargetSet should be bound to a skel mesh on creation
	if( !SelectedMorphSet->BaseSkelMesh )
	{	
		return;
	}

	// Disallow on cooked content.
	const UPackage* MorphSetPackage = SelectedMorphSet->GetOutermost();
	if( MorphSetPackage->PackageFlags & PKG_Cooked )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Error_OperationDisallowedOnCookedContent") );
		return;
	}

	// First, display the file open dialog for selecting the .PSK files.
	WxFileDialog ImportFileDialog( this, 
		*LocalizeUnrealEd("ImportMorphTarget"), 
		*(GApp->LastDir[LD_PSA]),
		TEXT(""),
		TEXT("PSK files|*.psk|COLLADA document|*.dae|All files|*.*"),
		wxOPEN | wxFILE_MUST_EXIST | wxMULTIPLE,
		wxDefaultPosition);
	
	// show dialog and handle OK
	if( ImportFileDialog.ShowModal() == wxID_OK )
	{
		// get list of files from dialog
		wxArrayString ImportFilePaths;
		ImportFileDialog.GetPaths(ImportFilePaths);

		// iterate over all selected files
		for( UINT FileIdx=0; FileIdx < ImportFilePaths.Count(); FileIdx++ )
		{
			// path and filename to import from
			FFilename Filename = (const TCHAR*)ImportFilePaths[FileIdx];
			// Check the file extension for PSK/DAE switch. Anything that isn't .DAE is considered a PSK file.
			const FString FileExtension = Filename.GetExtension();
			const UBOOL bIsCOLLADA = appStricmp(*FileExtension, TEXT("DAE")) == 0;
			// keep track of error reported
			EMorphImportError ImportError=MorphImport_OK;

			// handle importing to LOD level of an existing morph target
			if( bImportToLOD &&
				SelectedMorphSet->Targets.Num() > 0 )
			{
				// create a list of morph targets to display in the dlg
				TArray<UObject*> MorphList;
				for( INT Idx=0; Idx < SelectedMorphSet->Targets.Num(); Idx++)
				{
					MorphList.AddItem(SelectedMorphSet->Targets(Idx));
				}
				// only allow up to same number of LODs as the base skel mesh
				INT MaxLODIdx = SelectedMorphSet->BaseSkelMesh->LODModels.Num()-1;
				check(MaxLODIdx >= 0);
				// show dialog for importing an LOD level
				WxDlgMorphLODImport Dlg(this,MorphList,MaxLODIdx,*Filename.GetBaseFilename());
				if( Dlg.ShowModal() == wxID_OK )
				{
					// get user input from dlg
					INT SelectedMorphIdx = Dlg.GetSelectedMorph();
					INT SelectedLODIdx = Dlg.GetLODLevel();

					// import mesh to LOD
					if( !bIsCOLLADA )
					{
						// create the morph target importer
						FMorphTargetBinaryImport MorphImport( SelectedMorphSet->BaseSkelMesh, SelectedLODIdx, GWarn );
						// import the mesh LOD
						if( SelectedMorphSet->Targets.IsValidIndex(SelectedMorphIdx) )
						{
							MorphImport.ImportMorphLODModel(
								SelectedMorphSet->Targets(SelectedMorphIdx),
								*Filename,
								SelectedLODIdx,
								&ImportError
								);			
						}
						else
						{
							ImportError = MorphImport_MissingMorphTarget;
						}
					}
					else
					{
						UnCollada::CImporter* ColladaImporter = UnCollada::CImporter::GetInstance();
						if ( !ColladaImporter->ImportFromFile(*Filename) )
						{
							// Log the error message and fail the import.
							GWarn->Log( NAME_Error, ColladaImporter->GetErrorMessage() );
							ImportError = MorphImport_CantLoadFile;
						}
						else
						{
							//@todo sz - handle importing morph LOD from collada file
							GWarn->Log( TEXT("Morph target LOD importing from Collada format not supported at this time.") );
						}
					}					
				}
			}
			// handle importing a new morph target object
			else
			{
				// show dialog to specify the new morph target's name
				WxDlgGenericStringEntry NameDlg;
				if( NameDlg.ShowModal(TEXT("NewMorphTargetName"), TEXT("NewMorphTargetName"), *Filename.GetBaseFilename()) == wxID_OK )
				{
					// create the morph target importer
					FMorphTargetBinaryImport MorphImport( SelectedMorphSet->BaseSkelMesh, 0, GWarn );

					// get the name entered
					FName NewTargetName( *NameDlg.GetEnteredString() );

					// import the mesh
					if ( !bIsCOLLADA )
					{
						MorphImport.ImportMorphMeshToSet( 
							SelectedMorphSet, 
							NewTargetName, 
							*Filename,
							FALSE,
							&ImportError );
					}
					else
					{
						UnCollada::CImporter* ColladaImporter = UnCollada::CImporter::GetInstance();
						if ( !ColladaImporter->ImportFromFile(*Filename) )
						{
							// Log the error message and fail the import.
							GWarn->Log( NAME_Error, ColladaImporter->GetErrorMessage() );
							ImportError = MorphImport_CantLoadFile;
						}
						else
						{
							// Log the import message and import the animation.
							GWarn->Log( ColladaImporter->GetErrorMessage() );

							// Retrieve the list of all the meshes within the COLLADA document
							TArray<const TCHAR*> ColladaMeshes;
							ColladaImporter->RetrieveEntityList(ColladaMeshes, TRUE);
							ColladaImporter->RetrieveEntityList(ColladaMeshes, FALSE);
							if ( ColladaMeshes.Num() > 0 )
							{
								// TODO: Fill these arrays with only the wanted names to import.
								// For now: pick the first (up to) three possibilities and import them.
								TArray<FName> MorphTargetNames;
								TArray<const TCHAR*> ColladaImportNames;
								for( INT MeshIndex = 0; MeshIndex < ColladaMeshes.Num(); ++MeshIndex )
								{
									MorphTargetNames.AddItem( FName( ColladaMeshes( MeshIndex ) ) );
									ColladaImportNames.AddItem( ColladaMeshes( MeshIndex) );
								}

								ColladaImporter->ImportMorphTarget( SelectedMorphSet->BaseSkelMesh, SelectedMorphSet, MorphTargetNames, ColladaImportNames, TRUE, &ImportError );
							}
							else
							{
								ImportError = MorphImport_CantLoadFile;
							}
						}
					}

					// handle an existing target by asking the user to replace the existing one
					if( ImportError == MorphImport_AlreadyExists &&
						appMsgf( AMT_YesNoCancel, *LocalizeUnrealEd("Prompt_29") )==0 )
					{
						MorphImport.ImportMorphMeshToSet( 
							SelectedMorphSet, 
							NewTargetName, 
							*Filename,
							TRUE,
							&ImportError );
					}					
				}
			}

			// handle errors
			switch( ImportError )
			{
			case MorphImport_AlreadyExists:
				//appMsgf( AMT_OK, *LocalizeUnrealEd("Error_MorphImport_AlreadyExists") );
				break;
			case MorphImport_CantLoadFile:
				appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("Error_MorphImport_CantLoadFile"), *Filename) );
				break;
			case MorphImport_InvalidMeshFormat:
				appMsgf( AMT_OK, *LocalizeUnrealEd("Error_MorphImport_InvalidMeshFormat") );
				break;
			case MorphImport_MismatchBaseMesh:
				appMsgf( AMT_OK, *LocalizeUnrealEd("Error_MorphImport_MismatchBaseMesh") );
				break;
			case MorphImport_ReimportBaseMesh:
				appMsgf( AMT_OK, *LocalizeUnrealEd("Error_MorphImport_ReimportBaseMesh") );
				break;
			case MorphImport_MissingMorphTarget:
				appMsgf( AMT_OK, *LocalizeUnrealEd("Error_MorphImport_MissingMorphTarget") );
				break;
			case MorphImport_InvalidLODIndex:
				appMsgf( AMT_OK, *LocalizeUnrealEd("Error_MorphImport_InvalidLODIndex") );
				break;
			}
		}        
	}

	// Refresh combo to show new number of targets in this MorphTargetSet.
	INT CurrentSelection = MorphSetCombo->GetSelection();
	UpdateMorphSetCombo();
	MorphSetCombo->SetSelection(CurrentSelection);

	// Refresh morph target list box to show any new anims.
	UpdateMorphTargetList();

	// Reselect current morph target. If none selected, pick the first one in the box (if not empty).
	if(SelectedMorphTarget)
	{
		SetSelectedMorphTarget(SelectedMorphTarget);
	}
	else
	{
		if(MorphTargetList->GetCount() > 0)
		{
			SetSelectedMorphTarget( (UMorphTarget*)(MorphTargetList->GetClientData(0)) );
		}
		else
		{
			SetSelectedMorphTarget(NULL);
		}
	}

	// Mark as dirty.
	SelectedMorphSet->MarkPackageDirty();
}

/** rename the currently selected morph target */
void WxAnimSetViewer::RenameSelectedMorphTarget()
{
	if( SelectedMorphTarget )
	{
		check( SelectedMorphSet );
		check( SelectedMorphTarget->GetOuter() == SelectedMorphSet );
		check( SelectedMorphSet->Targets.ContainsItem(SelectedMorphTarget) );

		WxDlgGenericStringEntry dlg;
		const INT Result = dlg.ShowModal( TEXT("RenameMorph"), TEXT("NewMorphTargetName"), *SelectedMorphTarget->GetName() );
		if( Result == wxID_OK )
		{
			const FString NewItemString = dlg.GetEnteredString();
			const FName NewItemName =  FName( *NewItemString );

			if( SelectedMorphSet->FindMorphTarget( NewItemName ) )
			{
				// if we are trying to rename to an already existing name then prompt
				appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("Prompt_28"), *NewItemName.ToString()) );
			}
			else
			{
				// rename the object
				SelectedMorphTarget->Rename( *NewItemName.ToString() );
				// refresh the list
				UpdateMorphTargetList();
				// reselect the morph target
				SetSelectedMorphTarget( SelectedMorphTarget );
				// mark as dirty so a resave is required
				SelectedMorphSet->MarkPackageDirty();
			}
		}
	}
}

/** delete the currently selected morph target */
void WxAnimSetViewer::DeleteSelectedMorphTarget()
{
	if( SelectedMorphTarget )
	{
		check( SelectedMorphSet );
		check( SelectedMorphTarget->GetOuter() == SelectedMorphSet );
		check( SelectedMorphSet->Targets.ContainsItem(SelectedMorphTarget) );

		// show confirmation message dialog
		UBOOL bDoDelete = appMsgf( AMT_YesNo, *FString::Printf( LocalizeSecure(LocalizeUnrealEd("Prompt_27"), *SelectedMorphTarget->GetName()) ) );
		if( bDoDelete )
		{
			SelectedMorphSet->Targets.RemoveItem( SelectedMorphTarget );

			// Refresh list
			UpdateMorphTargetList();

			// Select the first item (if present) - or none at all.
			if( MorphTargetList->GetCount() > 0 )
			{
				SetSelectedMorphTarget( (UMorphTarget*)(MorphTargetList->GetClientData(0)) );
			}
			else
			{
				SelectedMorphTarget = NULL;
				SetSelectedAnimSequence( NULL );
			}
		}

		// mark as dirty
		SelectedMorphSet->MarkPackageDirty();
	}
}


void WxAnimSetViewer::CreateNewAnimSet()
{
	FString Package = TEXT("");
	FString Group = TEXT("");

	if(SelectedSkelMesh)
	{
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
	}

	WxDlgRename RenameDialog;
	RenameDialog.SetTitle( *LocalizeUnrealEd("NewAnimSet") );
	if( RenameDialog.ShowModal( Package, Group, TEXT("NewAnimSet") ) == wxID_OK )
	{
		if( RenameDialog.GetNewName().Len() == 0 || RenameDialog.GetNewPackage().Len() == 0 )
		{
			appMsgf( AMT_OK, *LocalizeUnrealEd("Error_MustSpecifyNewAnimSetName") );
			return;
		}

		UPackage* Pkg = GEngine->CreatePackage(NULL,*RenameDialog.GetNewPackage());
		if( RenameDialog.GetNewGroup().Len() )
		{
			Pkg = GEngine->CreatePackage(Pkg,*RenameDialog.GetNewGroup());
		}

		UAnimSet* NewAnimSet = ConstructObject<UAnimSet>( UAnimSet::StaticClass(), Pkg, FName( *RenameDialog.GetNewName() ), RF_Public|RF_Standalone );

		// Will update AnimSet list, which will include new AnimSet.
		SetSelectedSkelMesh(SelectedSkelMesh, true);
		SetSelectedAnimSet(NewAnimSet, false);

		SelectedAnimSet->MarkPackageDirty();
	}
}

IMPLEMENT_COMPARE_POINTER( UAnimSequence, AnimSetViewer, { return appStricmp(*A->SequenceName.ToString(),*B->SequenceName.ToString()); } );

void WxAnimSetViewer::UpdateAnimSeqList()
{
	AnimSeqList->Freeze();
	AnimSeqList->Clear();

	if(!SelectedAnimSet)
		return;

	Sort<USE_COMPARE_POINTER(UAnimSequence,AnimSetViewer)>(&SelectedAnimSet->Sequences(0),SelectedAnimSet->Sequences.Num());

	for(INT i=0; i<SelectedAnimSet->Sequences.Num(); i++)
	{
		UAnimSequence* Seq = SelectedAnimSet->Sequences(i);

		FString SeqString = FString::Printf( TEXT("%s [%d]"), *Seq->SequenceName.ToString(), Seq->NumFrames );
		AnimSeqList->Append( *SeqString, Seq );
	}
	AnimSeqList->Thaw();
}

// Note - this will clear the current selection in the combo- must manually select an AnimSet afterwards.
void WxAnimSetViewer::UpdateAnimSetCombo()
{
	AnimSetCombo->Freeze();
	AnimSetCombo->Clear();

	for(TObjectIterator<UAnimSet> It; It; ++It)
	{
		UAnimSet* ItAnimSet = *It;
		
		if( ItAnimSet->CanPlayOnSkeletalMesh(SelectedSkelMesh) )
		{
			FString AnimSetString = FString::Printf( TEXT("%s [%d]"), *ItAnimSet->GetName(), ItAnimSet->Sequences.Num() );
			AnimSetCombo->Append( *AnimSetString, ItAnimSet );
		}
	}
	AnimSetCombo->Thaw();
}

IMPLEMENT_COMPARE_POINTER( UMorphTarget, AnimSetViewer, { return appStricmp(*A->GetName(), *B->GetName()); } );


/** Update the list of MorphTargets in the list box. */
void WxAnimSetViewer::UpdateMorphTargetList()
{
	MorphTargetList->Freeze();
	MorphTargetList->Clear();

	if(!SelectedMorphSet)
		return;

	Sort<USE_COMPARE_POINTER(UMorphTarget,AnimSetViewer)>(&SelectedMorphSet->Targets(0),SelectedMorphSet->Targets.Num());

	for(INT i=0; i<SelectedMorphSet->Targets.Num(); i++)
	{
		UMorphTarget* Target = SelectedMorphSet->Targets(i);

		FString TargetString = FString::Printf( TEXT("%s"), *Target->GetName() );
		MorphTargetList->Append( *TargetString, Target );
	}
	MorphTargetList->Thaw();
}

/**
 *	Update the list of MorphTargetSets in the combo box.
 *	Note - this will clear the current selection in the combo- must manually select an AnimSet afterwards.
 */
void WxAnimSetViewer::UpdateMorphSetCombo()
{
	MorphSetCombo->Freeze();
	MorphSetCombo->Clear();

	// Add 'none' entry, for not using any morph target. This guarantees there is always an element zero in the MorphSetCombo.
	MorphSetCombo->Append( *LocalizeUnrealEd("-None-"), (void*)NULL );

	for(TObjectIterator<UMorphTargetSet> It; It; ++It)
	{
		UMorphTargetSet* ItMorphSet = *It;

		FString MorphSetString = FString::Printf( TEXT("%s [%d]"), *ItMorphSet->GetName(), ItMorphSet->Targets.Num() );
		MorphSetCombo->Append( *MorphSetString, ItMorphSet );
	}
	MorphSetCombo->Thaw();
}


// Update the UI to match the current animation state (position, playing, looping)
void WxAnimSetViewer::RefreshPlaybackUI()
{
	// Update scrub bar (can only do if we have an animation selected.
	if(SelectedAnimSeq)
	{
		check(PreviewAnimNode->AnimSeq == SelectedAnimSeq);

		FLOAT CurrentPos = PreviewAnimNode->CurrentTime / SelectedAnimSeq->SequenceLength;

		TimeSlider->SetValue( appRound(CurrentPos * (FLOAT)ASV_SCRUBRANGE) );
	}


	// Update Play/Stop button
	if(PreviewAnimNode->bPlaying)
	{
		// Only set the bitmap label if we actually are changing state.
		const wxBitmap& CurrentBitmap = PlayButton->GetBitmapLabel();


		if(CurrentBitmap.GetHandle() != StopB.GetHandle())
		{
			PlayButton->SetBitmapLabel( StopB );
			PlayButton->Refresh();
		}
	}
	else
	{
		// Only set the bitmap label if we actually are changing state.
		const wxBitmap& CurrentBitmap = PlayButton->GetBitmapLabel();

		if(CurrentBitmap.GetHandle() != PlayB.GetHandle())
		{
			PlayButton->SetBitmapLabel( PlayB );
			PlayButton->Refresh();
		}
	}



	// Update Loop toggle
	if(PreviewAnimNode->bLooping)
	{
		// Only set the bitmap label if we actually are changing state.
		const wxBitmap& CurrentBitmap = LoopButton->GetBitmapLabel();

		if(CurrentBitmap.GetHandle() != LoopB.GetHandle())
		{
			LoopButton->SetBitmapLabel( LoopB );
			LoopButton->Refresh();
		}
	}
	else
	{
		// Only set the bitmap label if we actually are changing state.
		const wxBitmap& CurrentBitmap = LoopButton->GetBitmapLabel();

		if(CurrentBitmap.GetHandle() != NoLoopB.GetHandle())
		{
			LoopButton->SetBitmapLabel( NoLoopB );
			LoopButton->Refresh();
		}
	}

	
}

void WxAnimSetViewer::TickViewer(FLOAT DeltaSeconds)
{
	// Tick the PreviewSkelComp to move animation forwards, then Update to update bone locations.
	PreviewSkelComp->TickAnimNodes(DeltaSeconds);
	PreviewSkelComp->UpdateLODStatus();
	PreviewSkelComp->UpdateSkelPose();

	PreviewSkelCompRaw->TickAnimNodes(DeltaSeconds);
	PreviewSkelCompRaw->UpdateLODStatus();
	PreviewSkelCompRaw->UpdateSkelPose();

	// Apply wind forces.
	PreviewSkelComp->UpdateClothWindForces(DeltaSeconds);

	// Run physics
	check( GWorld && GWorld->GetWorldInfo() );
	AWorldInfo * Info = GWorld->GetWorldInfo();
	FLOAT MaxSubstep = Info->PhysicsProperties.PrimaryScene.TimeStep;
	FLOAT MaxDeltaTime = Info->MaxPhysicsDeltaTime;
	FLOAT PhysDT = ::Min(DeltaSeconds, MaxDeltaTime);
	TickRBPhysScene(RBPhysScene, PhysDT, MaxSubstep, FALSE);
	WaitRBPhysScene(RBPhysScene);

	// Update components
	PreviewSkelComp->ConditionalUpdateTransform(FMatrix::Identity);
	PreviewSkelCompRaw->ConditionalUpdateTransform(FMatrix::Identity);
	PreviewSkelCompAux1->ConditionalUpdateTransform(FMatrix::Identity);
	PreviewSkelCompAux2->ConditionalUpdateTransform(FMatrix::Identity);
	PreviewSkelCompAux3->ConditionalUpdateTransform(FMatrix::Identity);

	// Move scrubber to reflect current animation position etc.
	RefreshPlaybackUI();
}

void WxAnimSetViewer::UpdateSkelComponents()
{
	FComponentReattachContext ReattachContext1(PreviewSkelComp);
	FComponentReattachContext ReattachContext1Raw(PreviewSkelCompRaw);
	FComponentReattachContext ReattachContext2(PreviewSkelCompAux1);
	FComponentReattachContext ReattachContext3(PreviewSkelCompAux2);
	FComponentReattachContext ReattachContext4(PreviewSkelCompAux3);
}

void WxAnimSetViewer::UpdateForceLODButtons()
{
	if(SelectedSkelMesh)
	{
		ToolBar->EnableTool( IDM_ANIMSET_LOD_1, SelectedSkelMesh->LODModels.Num() > 1 );
		ToolBar->EnableTool( IDM_ANIMSET_LOD_2, SelectedSkelMesh->LODModels.Num() > 2 );
		ToolBar->EnableTool( IDM_ANIMSET_LOD_3, SelectedSkelMesh->LODModels.Num() > 3 );
	}
}

void WxAnimSetViewer::EmptySelectedSet()
{
	if(SelectedAnimSet)
	{
		UBOOL bDoEmpty = appMsgf( AMT_YesNo, *LocalizeUnrealEd("Prompt_1") );
		if( bDoEmpty )
		{
			SelectedAnimSet->ResetAnimSet();
			UpdateAnimSeqList();
			SetSelectedAnimSequence(NULL);

			SelectedAnimSet->MarkPackageDirty();
		}
	}
}

void WxAnimSetViewer::DeleteTrackFromSelectedSet()
{
	if( SelectedAnimSet )
	{
		// Make a list of all tracks in the animset.
		TArray<FString> Options;
		for ( INT TrackIndex = 0 ; TrackIndex < SelectedAnimSet->TrackBoneNames.Num() ; ++TrackIndex )
		{
			const FName& TrackName = SelectedAnimSet->TrackBoneNames(TrackIndex);
			Options.AddItem(TrackName.ToString());
		}

		// Present the user with a list of potential tracks to delete.
		WxDlgGenericComboEntry dlg;
		const INT Result = dlg.ShowModal( TEXT("DeleteTrack"), TEXT("DeleteTrack"), Options, 0, TRUE );

		if ( Result == wxID_OK )
		{
			const FName SelectedTrackName( *dlg.GetSelectedString() );

			const INT SelectedTrackIndex = SelectedAnimSet->TrackBoneNames.FindItemIndex( SelectedTrackName );
			if ( SelectedTrackIndex != INDEX_NONE )
			{
				UAnimNodeMirror* Pre = PreviewAnimMirror;
				// Eliminate the selected track from all sequences in the set.
				for( INT SequenceIndex = 0 ; SequenceIndex < SelectedAnimSet->Sequences.Num() ; ++SequenceIndex )
				{
					UAnimSequence* Seq = SelectedAnimSet->Sequences( SequenceIndex );
					check( Seq->RawAnimData.Num() == SelectedAnimSet->TrackBoneNames.Num() );
					Seq->RawAnimData.Remove( SelectedTrackIndex );
				}
				UAnimNodeMirror* Post = PreviewAnimMirror;

				// Element the track bone name.
				SelectedAnimSet->TrackBoneNames.Remove( SelectedTrackIndex );

				// Flush the linkup cache.
				SelectedAnimSet->LinkupCache.Empty();

				// Flag the animset for resaving.
				SelectedAnimSet->MarkPackageDirty();

				// Reinit anim trees so that eg new linkup caches are created.
				for(TObjectIterator<USkeletalMeshComponent> It;It;++It)
				{
					USkeletalMeshComponent* SkelComp = *It;
					if(!SkelComp->IsPendingKill() && !SkelComp->IsTemplate())
					{
						SkelComp->InitAnimTree();
					}
				}

				// Recompress the anim set if compression was applied.
				RecompressSet( SelectedAnimSet, SelectedSkelMesh );
			}
		}
	}
}

/** Copy TranslationBoneNames to new AnimSet. */
void WxAnimSetViewer::CopyTranslationBoneNamesToAnimSet()
{
	// Get destination AnimSet, and check we have one!
	UAnimSet* DestAnimSet = GEditor->GetSelectedObjects()->GetTop<UAnimSet>();
	if( !DestAnimSet )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("NoDestinationAnimSetSelected") );
		return;
	}

	// Check we're not moving to same AnimSet... duh!
	if( DestAnimSet == SelectedAnimSet )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("AnimSetSelectedIsSourceAnimSet") );
		return;
	}

	DestAnimSet->UseTranslationBoneNames = SelectedAnimSet->UseTranslationBoneNames;
	DestAnimSet->ForceMeshTranslationBoneNames = SelectedAnimSet->ForceMeshTranslationBoneNames;
	DestAnimSet->MarkPackageDirty();
}

void WxAnimSetViewer::RenameSelectedSeq()
{
	if(SelectedAnimSeq)
	{
		check(SelectedAnimSet);

		WxDlgGenericStringEntry dlg;
		const INT Result = dlg.ShowModal( TEXT("RenameAnimSequence"), TEXT("NewSequenceName"), *SelectedAnimSeq->SequenceName.ToString() );
		if( Result == wxID_OK )
		{
			const FString NewSeqString = dlg.GetEnteredString();
			const FName NewSeqName =  FName( *NewSeqString );

			// If there is a sequence with that name already, see if we want to over-write it.
			UAnimSequence* FoundSeq = SelectedAnimSet->FindAnimSequence(NewSeqName);
			if(FoundSeq)
			{
				const FString ConfirmMessage = FString::Printf( LocalizeSecure(LocalizeUnrealEd("Prompt_2"), *NewSeqName.ToString()) );
				const UBOOL bDoDelete = appMsgf( AMT_YesNo, *ConfirmMessage );
				if(!bDoDelete)
					return;

				SelectedAnimSet->Sequences.RemoveItem(FoundSeq);
			}

			SelectedAnimSeq->SequenceName = NewSeqName;

			UpdateAnimSeqList();

			// This will reselect this sequence, update the AnimNodeSequence, status bar etc.
			SetSelectedAnimSequence(SelectedAnimSeq);


			SelectedAnimSet->MarkPackageDirty();
		}
	}
}

void WxAnimSetViewer::DeleteSelectedSequence()
{
	// Get the list of animations selected
	wxArrayInt Selection;
	AnimSeqList->GetSelections(Selection);

	// Build list of animations to delete
	TArray<class UAnimSequence*> Sequences;
	FString SeqListString = TEXT(" ");
	for (UINT i=0; i<Selection.Count(); i++)
	{
		INT SelIndex = Selection.Item(i);

		UAnimSequence* AnimSeq = (UAnimSequence*)AnimSeqList->GetClientData(SelIndex);
		if( AnimSeq )
		{
			check( AnimSeq->GetOuter() == SelectedAnimSet );
			check( SelectedAnimSet->Sequences.ContainsItem(AnimSeq) );

			// Checks out, so add item...
			Sequences.AddItem(AnimSeq);
			SeqListString = FString::Printf( TEXT("%s\n %s"), *SeqListString, *AnimSeq->SequenceName.ToString());
		}
	}

	// If no sequences found, just abort
	if( Sequences.Num() == 0 )
	{
		return;
	}

	// Pop up a message to make sure the user really wants to do this
	FString ConfirmMessage = FString::Printf( LocalizeSecure(LocalizeUnrealEd("Prompt_3"), *SeqListString) );
	UBOOL bDoDelete = appMsgf( AMT_YesNo, *ConfirmMessage );
	if( !bDoDelete )
	{
		return;
	}

	// Now go through all sequences and delete!
	INT LastIndex = INDEX_NONE;
	for(INT i=0; i<Sequences.Num(); i++)
	{
		UAnimSequence* AnimSeq = Sequences(i);
		if( AnimSeq )
		{
			LastIndex = SelectedAnimSet->Sequences.FindItemIndex(AnimSeq);
			SelectedAnimSet->Sequences.Remove(LastIndex, 1);
		}
	}

	// Refresh list
	UpdateAnimSeqList();

	// Select previous or next animation in the list (if present) - or none at all.
	if( SelectedAnimSet->Sequences.Num() > 0 )
	{
		INT SelectedIndex = Clamp<INT>(LastIndex, 0, SelectedAnimSet->Sequences.Num() - 1);
		SetSelectedAnimSequence( SelectedAnimSet->Sequences(SelectedIndex) );
	}
	else
	{
		SetSelectedAnimSequence( NULL );
	}

	SelectedAnimSet->MarkPackageDirty();
}

/** Copy animation selection to new animation set. */
void WxAnimSetViewer::CopySelectedSequence()
{
	// Get the list of animations selected
	wxArrayInt Selection;
	AnimSeqList->GetSelections(Selection);

	// Build list of animations to move
	TArray<class UAnimSequence*> Sequences;
	FString SeqListString = TEXT(" ");
	for (UINT i=0; i<Selection.Count(); i++)
	{
		INT SelIndex = Selection.Item(i);

		UAnimSequence* AnimSeq = (UAnimSequence*)AnimSeqList->GetClientData(SelIndex);
		if( AnimSeq )
		{
			check( AnimSeq->GetOuter() == SelectedAnimSet );
			check( SelectedAnimSet->Sequences.ContainsItem(AnimSeq) );

			// Checks out, so add item...
			Sequences.AddItem(AnimSeq);
			SeqListString = FString::Printf( TEXT("%s\n %s"), *SeqListString, *AnimSeq->SequenceName.ToString());
		}
	}

	// If no sequences found, just abort
	if( Sequences.Num() == 0 )
	{
		return;
	}

	// Get selected AnimSet, and check we have one!
	UAnimSet* DestAnimSet = GEditor->GetSelectedObjects()->GetTop<UAnimSet>();
	if( !DestAnimSet )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("NoDestinationAnimSetSelected") );
		return;
	}

	// Check we're not moving to same AnimSet... duh!
	if( DestAnimSet == SelectedAnimSet )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("AnimSetSelectedIsSourceAnimSet") );
		return;
	}

	// Pop up a message to make sure the user really wants to do this
	FString ConfirmMessage = FString::Printf( LocalizeSecure(LocalizeUnrealEd("Prompt_34"), *SeqListString, *DestAnimSet->GetName()) );
	UBOOL bProceed = appMsgf( AMT_YesNo, *ConfirmMessage );
	if( !bProceed )
	{
		return;
	}

	// Now go through all sequences and move them!
	INT LastIndex = INDEX_NONE;
	for(INT i=0; i<Sequences.Num(); i++)
	{
		UAnimSequence* AnimSeq = Sequences(i);
		if( AnimSeq )
		{
			// See if this sequence already exists in destination
			UAnimSequence* DestSeq = DestAnimSet->FindAnimSequence(AnimSeq->SequenceName);
			// If not, create new one now.
			if( !DestSeq )
			{
				DestSeq = ConstructObject<UAnimSequence>( UAnimSequence::StaticClass(), DestAnimSet );
			}
			else
			{
				UBOOL bDoReplace = appMsgf( AMT_YesNo, LocalizeSecure(LocalizeUnrealEd("Prompt_25"), *AnimSeq->SequenceName.ToString()) );
				if( !bDoReplace )
				{
					continue; // Move on to next sequence...
				}
			}
			
			// Copy AnimSeq information who belongs to SelectedAnimSet, into DestAnimSeq to be put into DestAnimSet.
			if( !CopyAnimSequence(AnimSeq, DestSeq, SelectedAnimSet, DestAnimSet, SelectedSkelMesh) )
			{		
				// Abort
				return;
			}
		}
	}

	// Mark destination package dirty
	DestAnimSet->MarkPackageDirty();
}


/** Move animation selection to new animation set. */
void WxAnimSetViewer::MoveSelectedSequence()
{
	// Get the list of animations selected
	wxArrayInt Selection;
	AnimSeqList->GetSelections(Selection);

	// Build list of animations to move
	TArray<class UAnimSequence*> Sequences;
	FString SeqListString = TEXT(" ");
	for (UINT i=0; i<Selection.Count(); i++)
	{
		INT SelIndex = Selection.Item(i);

		UAnimSequence* AnimSeq = (UAnimSequence*)AnimSeqList->GetClientData(SelIndex);
		if( AnimSeq )
		{
			check( AnimSeq->GetOuter() == SelectedAnimSet );
			check( SelectedAnimSet->Sequences.ContainsItem(AnimSeq) );

			// Checks out, so add item...
			Sequences.AddItem(AnimSeq);
			SeqListString = FString::Printf( TEXT("%s\n %s"), *SeqListString, *AnimSeq->SequenceName.ToString());
		}
	}

	// If no sequences found, just abort
	if( Sequences.Num() == 0 )
	{
		return;
	}

	// Get selected AnimSet, and check we have one!
	UAnimSet* DestAnimSet = GEditor->GetSelectedObjects()->GetTop<UAnimSet>();
	if( !DestAnimSet )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("NoDestinationAnimSetSelected") );
		return;
	}

	// Check we're not moving to same AnimSet... duh!
	if( DestAnimSet == SelectedAnimSet )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("AnimSetSelectedIsSourceAnimSet") );
		return;
	}

	// Pop up a message to make sure the user really wants to do this
	FString ConfirmMessage = FString::Printf( LocalizeSecure(LocalizeUnrealEd("Prompt_32"), *SeqListString, *DestAnimSet->GetName()) );
	UBOOL bDoDelete = appMsgf( AMT_YesNo, *ConfirmMessage );
	if( !bDoDelete )
	{
		return;
	}

	// Now go through all sequences and move them!
	INT LastIndex = INDEX_NONE;
	for(INT i=0; i<Sequences.Num(); i++)
	{
		UAnimSequence* AnimSeq = Sequences(i);
		if( AnimSeq )
		{
			// See if this sequence already exists in destination
			UAnimSequence* DestSeq = DestAnimSet->FindAnimSequence(AnimSeq->SequenceName);
			// If not, create new one now.
			if( !DestSeq )
			{
				DestSeq = ConstructObject<UAnimSequence>( UAnimSequence::StaticClass(), DestAnimSet );
			}
			else
			{
				UBOOL bDoReplace = appMsgf( AMT_YesNo, LocalizeSecure(LocalizeUnrealEd("Prompt_25"), *AnimSeq->SequenceName.ToString()) );
				if( !bDoReplace )
				{
					continue; // Move on to next sequence...
				}
			}
			
			// Copy AnimSeq information who belongs to SelectedAnimSet, into DestAnimSeq to be put into DestAnimSet.
			if( CopyAnimSequence(AnimSeq, DestSeq, SelectedAnimSet, DestAnimSet, SelectedSkelMesh) )
			{		
				// now delete AnimSeq from source AnimSet
				LastIndex = SelectedAnimSet->Sequences.FindItemIndex(AnimSeq);
				SelectedAnimSet->Sequences.Remove(LastIndex, 1);
			}
			// Abort
			else
			{
				return;
			}
		}
	}

	// Refresh list
	UpdateAnimSeqList();

	// Select previous or next animation in the list (if present) - or none at all.
	if( SelectedAnimSet->Sequences.Num() > 0 )
	{
		INT SelectedIndex = Clamp<INT>(LastIndex, 0, SelectedAnimSet->Sequences.Num() - 1);
		SetSelectedAnimSequence( SelectedAnimSet->Sequences(SelectedIndex) );
	}
	else
	{
		SetSelectedAnimSequence( NULL );
	}

	SelectedAnimSet->MarkPackageDirty();
	DestAnimSet->MarkPackageDirty();
}

/** 
 * Copy SourceAnimSeq in SourceAnimSet to DestAnimSeq in DestAnimSet.
 * Returns TRUE for success, FALSE for failure.
 */
UBOOL WxAnimSetViewer::CopyAnimSequence(UAnimSequence* SourceAnimSeq, UAnimSequence *DestAnimSeq, UAnimSet* SourceAnimSet, UAnimSet* DestAnimSet, USkeletalMesh* FillInMesh)
{
	// Make sure input is valid
	check( SourceAnimSeq && DestAnimSeq && SourceAnimSet && DestAnimSet && FillInMesh );

	// Calculate track mapping from target tracks in DestAnimSet to the source amoung those we are importing.
	TArray<INT> TrackMap;

	// If Destination is an empty AnimSet, we copy this information from the source animset.
	if( DestAnimSet->TrackBoneNames.Num() == 0)
	{
		DestAnimSet->TrackBoneNames = SourceAnimSet->TrackBoneNames;
		TrackMap.Add( DestAnimSet->TrackBoneNames.Num() );

		for(INT i=0; i<DestAnimSet->TrackBoneNames.Num(); i++)
		{
			TrackMap(i) = i;
		}
	}
	else
	{
		// Otherwise, ensure right track goes to right place.
		// If we are missing a track, we give a warning and refuse to import into this set.
		TrackMap.Add( DestAnimSet->TrackBoneNames.Num() );

		// For each track in the AnimSet, find its index in the source data
		for(INT i=0; i<DestAnimSet->TrackBoneNames.Num(); i++)
		{
			FName TrackName = DestAnimSet->TrackBoneNames(i);
			TrackMap(i) = INDEX_NONE;

			for(INT j=0; j<SourceAnimSet->TrackBoneNames.Num(); j++)
			{
				FName TestName = SourceAnimSet->TrackBoneNames(j);
				if( TestName == TrackName )
				{	
					if( TrackMap(i) != INDEX_NONE )
					{
						debugf( TEXT(" DUPLICATE TRACK IN INCOMING DATA: %s"), *TrackName.ToString() );
					}
					TrackMap(i) = j;
				}
			}

			// If we failed to find a track we need in the imported data - see if we want to patch it using the skeletal mesh ref pose.
			if( TrackMap(i) == INDEX_NONE )
			{
				UBOOL bDoPatching = appMsgf(AMT_YesNo, LocalizeSecure(LocalizeUnrealEd("Error_CouldNotFindTrack"), *TrackName.ToString()));
				
				if( bDoPatching )
				{
					// Check the selected skel mesh has a bone called that. If we can't find it - fail.
					INT PatchBoneIndex = FillInMesh->MatchRefBone(TrackName);
					if( PatchBoneIndex == INDEX_NONE )
					{
						appMsgf(AMT_OK, LocalizeSecure(LocalizeUnrealEd("Error_CouldNotFindPatchBone"), *TrackName.ToString()));
						return FALSE;
					}
				}
				else
				{
					return FALSE;
				}
			}
		}
	}

	// Flag to indicate that the AnimSet has had its TrackBoneNames array changed, and therefore its LinkupCache needs to be flushed.
	UBOOL bAnimSetTrackChanged = FALSE;

	// Now we see if there are any tracks in the incoming data which do not have a use in the DestAnimSet. 
	// These will be thrown away unless we extend the DestAnimSet name table.
	TArray<FName> AnimSetMissingNames;
	TArray<UAnimSequence*> SequencesRequiringRecompression;
	for(INT i=0; i<SourceAnimSet->TrackBoneNames.Num(); i++)
	{
		if( !TrackMap.ContainsItem(i) )
		{
			FName ExtraTrackName = SourceAnimSet->TrackBoneNames(i);
			UBOOL bDoExtension = appMsgf(AMT_YesNo, LocalizeSecure(LocalizeUnrealEd("Error_ExtraInfoInPSA"), *ExtraTrackName.ToString()));
			if( bDoExtension )
			{
				INT PatchBoneIndex = FillInMesh->MatchRefBone(ExtraTrackName);

				// If we can't find the extra track in the SkelMesh to create an animation from, warn and fail.
				if( PatchBoneIndex == INDEX_NONE )
				{
					appMsgf(AMT_OK, LocalizeSecure(LocalizeUnrealEd("Error_CouldNotFindPatchBone"), *ExtraTrackName.ToString()));
					return FALSE;
				}
				// If we could, add to all animations in the AnimSet, and add to track table.
				else
				{
					DestAnimSet->TrackBoneNames.AddItem(ExtraTrackName);
					bAnimSetTrackChanged = TRUE;

					// Iterate over all existing sequences in this set and add an extra track to the end.
					for(INT SetAnimIndex=0; SetAnimIndex<DestAnimSet->Sequences.Num(); SetAnimIndex++)
					{
						UAnimSequence* ExtendSeq = DestAnimSet->Sequences(SetAnimIndex);

						// Remove any compression on the sequence so that it will be recomputed with the new track.
						if( ExtendSeq->CompressedTrackOffsets.Num() > 0 )
						{
							ExtendSeq->CompressedTrackOffsets.Empty();
							// Mark the sequence as requiring recompression.
							SequencesRequiringRecompression.AddUniqueItem( ExtendSeq );
						}

						// Add an extra track to the end, based on the ref skeleton.
						ExtendSeq->RawAnimData.AddZeroed();
						FRawAnimSequenceTrack& RawTrack = ExtendSeq->RawAnimData( ExtendSeq->RawAnimData.Num()-1 );

						// Create 1-frame animation from the reference pose of the skeletal mesh.
						// This is basically what the compression does, so should be fine.
						if( ExtendSeq->bIsAdditive )
						{
							RawTrack.PosKeys.AddItem(FVector(0.f));

							FQuat RefOrientation = FQuat::Identity;
							// To emulate ActorX-exported animation quat-flipping, we do it here.
							if( PatchBoneIndex > 0 )
							{
								RefOrientation.W *= -1.f;
							}
							RawTrack.RotKeys.AddItem(RefOrientation);
							RawTrack.KeyTimes.AddItem( 0.f );

							// Extend AdditiveRefPose
							const FMeshBone& RefSkelBone = FillInMesh->RefSkeleton(PatchBoneIndex);

							FBoneAtom RefBoneAtom = FBoneAtom::Identity;
							RefBoneAtom.Translation = RefSkelBone.BonePos.Position;
							RefBoneAtom.Rotation    = RefSkelBone.BonePos.Orientation;
							if( PatchBoneIndex > 0)
							{
								RefBoneAtom.Rotation.W *= -1.f; // As above - flip if necessary
							}

							// Save off RefPose into destination AnimSequence
							ExtendSeq->AdditiveRefPose.AddItem(RefBoneAtom);
						}
						else
						{
							const FVector RefPosition = FillInMesh->RefSkeleton(PatchBoneIndex).BonePos.Position;
							RawTrack.PosKeys.AddItem(RefPosition);

							FQuat RefOrientation = FillInMesh->RefSkeleton(PatchBoneIndex).BonePos.Orientation;
							// To emulate ActorX-exported animation quat-flipping, we do it here.
							if( PatchBoneIndex > 0 )
							{
								RefOrientation.W *= -1.f;
							}
							RawTrack.RotKeys.AddItem(RefOrientation);
							RawTrack.KeyTimes.AddItem( 0.f );
						}
					}

					// So now the new incoming track 'i' maps to the last track in the AnimSet.
					TrackMap.AddItem(i);
				}
			}
		}
	}

	// Make sure DestAnimSeq's outer is the DestAnimSet
	if( DestAnimSeq->GetOuter() != DestAnimSet )
	{
		DestAnimSeq->Rename( *DestAnimSeq->GetName(), DestAnimSet );
	}

	// Make sure DestAnimSeq belongs to DestAnimSet
	if( DestAnimSet->Sequences.FindItemIndex(DestAnimSeq) == INDEX_NONE )
	{
		DestAnimSet->Sequences.AddItem( DestAnimSeq );
	}

	// Initialize DestAnimSeq
	DestAnimSeq->RecycleAnimSequence();

	// Copy all parameters from source to destination
	CopyAnimSequenceProperties(SourceAnimSeq, DestAnimSeq);

	// Make sure data is zeroed
	DestAnimSeq->RawAnimData.AddZeroed( DestAnimSet->TrackBoneNames.Num() );

	// Structure of data is this:
	// RawAnimKeys contains all keys. 
	// Sequence info FirstRawFrame and NumRawFrames indicate full-skel frames (NumPSATracks raw keys). Ie number of elements we need to copy from RawAnimKeys is NumRawFrames * NumPSATracks.

	// Import each track.
	for(INT TrackIdx = 0; TrackIdx < DestAnimSet->TrackBoneNames.Num(); TrackIdx++)
	{
		check( DestAnimSet->TrackBoneNames.Num() == DestAnimSeq->RawAnimData.Num() );
		FRawAnimSequenceTrack& RawTrack = DestAnimSeq->RawAnimData(TrackIdx);

		// Find the source track for this one in the AnimSet
		INT SourceTrackIdx = TrackMap( TrackIdx );

		// If bone was not found in incoming data, use SkeletalMesh to create the track.
		if( SourceTrackIdx == INDEX_NONE )
		{
			FName TrackName = DestAnimSet->TrackBoneNames(TrackIdx);
			INT PatchBoneIndex = FillInMesh->MatchRefBone(TrackName);
			check(PatchBoneIndex != INDEX_NONE); // Should have checked for this case above!

			// Create 1-frame animation from the reference pose of the skeletal mesh.
			// This is basically what the compression does, so should be fine.

			FVector RefPosition = FillInMesh->RefSkeleton(PatchBoneIndex).BonePos.Position;
			RawTrack.PosKeys.AddItem(RefPosition);

			FQuat RefOrientation = FillInMesh->RefSkeleton(PatchBoneIndex).BonePos.Orientation;
			// To emulate ActorX-exported animation quat-flipping, we do it here.
			if( PatchBoneIndex > 0 )
			{
				RefOrientation.W *= -1.f;
			}
			RawTrack.RotKeys.AddItem(RefOrientation);
			RawTrack.KeyTimes.AddItem( 0.f );
		}
		else
		{
			check(SourceTrackIdx >= 0 && SourceTrackIdx < SourceAnimSet->TrackBoneNames.Num());

			INT SrcTrackIndex = SourceAnimSet->FindTrackWithName(DestAnimSet->TrackBoneNames(TrackIdx));
			if( SrcTrackIndex != INDEX_NONE )
			{
				// Direct copy
				DestAnimSeq->RawAnimData(TrackIdx) = SourceAnimSeq->RawAnimData(SrcTrackIndex);

				// Remap the additive ref pose
				if( DestAnimSeq->bIsAdditive )
				{
					DestAnimSeq->AdditiveRefPose(TrackIdx) = SourceAnimSeq->AdditiveRefPose(SrcTrackIndex);
				}
			}
			else
			{
				appMsgf(AMT_OK, *FString::Printf(TEXT("Couldn't find track in source animset. %s"), *DestAnimSet->TrackBoneNames(TrackIdx).ToString()) );
				return FALSE;
			}
		}
	}

	// See if SourceAnimSeq had a compression Scheme
	FAnimationUtils::CompressAnimSequence(DestAnimSeq, NULL, FALSE, FALSE);

	// If we need to, flush the LinkupCache.
	if( bAnimSetTrackChanged )
	{
		DestAnimSet->LinkupCache.Empty();

		// We need to re-init any skeletal mesh components now, because they might still have references to linkups in this set.
		for(TObjectIterator<USkeletalMeshComponent> It;It;++It)
		{
			USkeletalMeshComponent* SkelComp = *It;
			if( !SkelComp->IsPendingKill() && !SkelComp->IsTemplate() )
			{
				SkelComp->InitAnimTree();
			}
		}

		// Recompress any sequences that need it.
		for( INT SequenceIndex = 0 ; SequenceIndex < SequencesRequiringRecompression.Num() ; ++SequenceIndex )
		{
			UAnimSequence* AnimSeq = SequencesRequiringRecompression( SequenceIndex );
			FAnimationUtils::CompressAnimSequence(AnimSeq, NULL, FALSE, FALSE);
		}
	}

	return TRUE;
}

/** 
 * Utility function to copy all UAnimSequence properties from Source to Destination.
 * Does not copy however RawAnimData and CompressedAnimData.
 */
UBOOL WxAnimSetViewer::CopyAnimSequenceProperties(UAnimSequence* SourceAnimSeq, UAnimSequence* DestAnimSeq)
{
	// Copy parameters
	DestAnimSeq->SequenceName				= SourceAnimSeq->SequenceName;
	DestAnimSeq->SequenceLength				= SourceAnimSeq->SequenceLength;
	DestAnimSeq->NumFrames					= SourceAnimSeq->NumFrames;
	DestAnimSeq->RateScale					= SourceAnimSeq->RateScale;
	DestAnimSeq->bNoLoopingInterpolation	= SourceAnimSeq->bNoLoopingInterpolation;
	DestAnimSeq->bIsAdditive				= SourceAnimSeq->bIsAdditive;
	DestAnimSeq->AdditiveRefPose			= SourceAnimSeq->AdditiveRefPose;
	DestAnimSeq->AdditiveRefName			= SourceAnimSeq->AdditiveRefName;
	DestAnimSeq->bDoNotOverrideCompression	= SourceAnimSeq->bDoNotOverrideCompression;

	// Copy Compression Settings
	DestAnimSeq->CompressionScheme				= SourceAnimSeq->CompressionScheme;
	DestAnimSeq->TranslationCompressionFormat	= SourceAnimSeq->TranslationCompressionFormat;
	DestAnimSeq->RotationCompressionFormat		= SourceAnimSeq->RotationCompressionFormat;

	// Copy notifies
	return CopyNotifies(SourceAnimSeq, DestAnimSeq);
}

/** 
 * Copy AnimNotifies from one UAnimSequence to another.
 */
UBOOL WxAnimSetViewer::CopyNotifies(UAnimSequence* SourceAnimSeq, UAnimSequence* DestAnimSeq)
{
	// Abort if source == destination.
	if( SourceAnimSeq == DestAnimSeq )
	{
		return TRUE;
	}

	// If the destination sequence is shorter than the source sequence, we'll be dropping notifies that
	// occur at later times than the dest sequence is long.  Give the user a chance to abort if we
	// find any notifies that won't be copied over.
	if( DestAnimSeq->SequenceLength < SourceAnimSeq->SequenceLength )
	{
		for(INT NotifyIndex=0; NotifyIndex<SourceAnimSeq->Notifies.Num(); ++NotifyIndex)
		{
			// If a notify is found which occurs off the end of the destination sequence, prompt the user to continue.
			const FAnimNotifyEvent& SrcNotifyEvent = SourceAnimSeq->Notifies(NotifyIndex);
			if( SrcNotifyEvent.Time > DestAnimSeq->SequenceLength )
			{
				const UBOOL bProceed = appMsgf( AMT_YesNo, *LocalizeUnrealEd("SomeNotifiesWillNotBeCopiedQ") );
				if( !bProceed )
				{
					return FALSE;
				}
				else
				{
					break;
				}
			}
		}
	}

	// If the destination sequence contains any notifies, ask the user if they'd like
	// to delete the existing notifies before copying over from the source sequence.
	if( DestAnimSeq->Notifies.Num() > 0 )
	{
		const UBOOL bDeleteExistingNotifies = appMsgf( AMT_YesNo, LocalizeSecure(LocalizeUnrealEd("DestSeqAlreadyContainsNotifiesMergeQ"), DestAnimSeq->Notifies.Num()) );
		if( bDeleteExistingNotifies )
		{
			DestAnimSeq->Notifies.Empty();
			DestAnimSeq->MarkPackageDirty();
		}
	}

	// Do the copy.
	TArray<INT> NewNotifyIndices;
	INT NumNotifiesThatWereNotCopied = 0;

	const FScopedBusyCursor BusyCursor;
	for(INT NotifyIndex=0; NotifyIndex<SourceAnimSeq->Notifies.Num(); ++NotifyIndex)
	{
		const FAnimNotifyEvent& SrcNotifyEvent = SourceAnimSeq->Notifies(NotifyIndex);

		// Skip notifies which occur at times later than the destination sequence is long.
		if( SrcNotifyEvent.Time > DestAnimSeq->SequenceLength )
		{
			continue;
		}

		// Do a linear-search through existing notifies to determine where
		// to insert the new notify.
		INT NewNotifyIndex = 0;
		while( NewNotifyIndex < DestAnimSeq->Notifies.Num()
			&& DestAnimSeq->Notifies(NewNotifyIndex).Time <= SrcNotifyEvent.Time )
		{
			++NewNotifyIndex;
		}

		// Track the location of the new notify.
		NewNotifyIndices.AddItem(NewNotifyIndex);

		// Create a new empty on in the array.
		DestAnimSeq->Notifies.InsertZeroed(NewNotifyIndex);

		// Copy time and comment.
		DestAnimSeq->Notifies(NewNotifyIndex).Time = SrcNotifyEvent.Time;
		DestAnimSeq->Notifies(NewNotifyIndex).Comment = SrcNotifyEvent.Comment;

		// Copy the notify itself, and point the new one at it.
		if( SrcNotifyEvent.Notify )
		{
			FObjectDuplicationParameters DupParams( SrcNotifyEvent.Notify, SrcNotifyEvent.Notify->GetOuter() );
			DestAnimSeq->Notifies(NewNotifyIndex).Notify = CastChecked<UAnimNotify>( UObject::StaticDuplicateObjectEx(DupParams) );
		}
		else
		{
			DestAnimSeq->Notifies(NewNotifyIndex).Notify = NULL;
		}

		// Make sure editor knows we've changed something.
		DestAnimSeq->MarkPackageDirty();
	}

	// Inform the user if some notifies weren't copied.
	if( SourceAnimSeq->Notifies.Num() > NewNotifyIndices.Num() )
	{
		appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("SomeNotifiesWereNotCopiedF"), SourceAnimSeq->Notifies.Num() - NewNotifyIndices.Num()) );
	}

	return TRUE;
}

void WxAnimSetViewer::MakeSelectedSequencesAdditive()
{
	// Get the list of selected animations
	wxArrayInt Selection;
	AnimSeqList->GetSelections(Selection);

	// Build list of selected AnimSequences
	TArray<class UAnimSequence*> Sequences;
	FString SeqListString = TEXT(" ");
	for (UINT i=0; i<Selection.Count(); i++)
	{
		INT SelIndex = Selection.Item(i);

		UAnimSequence* AnimSeq = (UAnimSequence*)AnimSeqList->GetClientData(SelIndex);
		if( AnimSeq )
		{
			check( AnimSeq->GetOuter() == SelectedAnimSet );
			check( SelectedAnimSet->Sequences.ContainsItem(AnimSeq) );

			// Checks out, so add item...
			Sequences.AddItem(AnimSeq);
			SeqListString = FString::Printf( TEXT("%s\n %s"), *SeqListString, *AnimSeq->SequenceName.ToString());
		}
	}

	// If no sequences found, just abort
	if( Sequences.Num() == 0 )
	{
		return;
	}

	// Now go through all sequences and convert them!
	INT LastIndex = INDEX_NONE;
	for(INT i=0; i<Sequences.Num(); i++)
	{
		UAnimSequence* AnimSeq = Sequences(i);
		if( AnimSeq )
		{
			FString NewNameString = FString::Printf(TEXT("ADD_%s"), *AnimSeq->SequenceName.ToString());

			// See if this sequence already exists in destination
			UAnimSequence* DestSeq = SelectedAnimSet->FindAnimSequence(FName(*NewNameString));

			// If not, create new one now.
			if( !DestSeq )
			{
				DestSeq = ConstructObject<UAnimSequence>(UAnimSequence::StaticClass(), SelectedAnimSet);
				// Add AnimSequence to AnimSet
				SelectedAnimSet->Sequences.AddItem( DestSeq );
			}
			else
			{
				UBOOL bDoReplace = appMsgf( AMT_YesNo, LocalizeSecure(LocalizeUnrealEd("Prompt_25"), *NewNameString) );
				if( !bDoReplace )
				{
					continue; // Move on to next sequence...
				}
			}

			// Build a list of reference animations from the selected anim set to choose from, do not show additive animations
			wxArrayString RefAnimChoices;
			RefAnimChoices.Add( *AnimSeq->SequenceName.ToString() );		// First: Source Animation
			RefAnimChoices.Add( TEXT("Reference Pose") );					// Then reference pose
			for(INT i=0; i<SelectedAnimSet->Sequences.Num(); i++)
			{
				if( !SelectedAnimSet->Sequences(i)->bIsAdditive )
				{
					RefAnimChoices.Add( *SelectedAnimSet->Sequences(i)->SequenceName.ToString() );
				}
			}

			wxString DlgResult = wxGetSingleChoice(TEXT("Reference Pose Sequence Name - First frame of animation will be used"),		// message
				*FString::Printf(TEXT("Select Reference Animation Sequence for %s"), *AnimSeq->SequenceName.ToString()),				// title
				RefAnimChoices, // choices
				this // parent
				);

			// Cancel button pressed
			if( DlgResult == TEXT("") )
			{
				return;
			}

			// See from which animation we should get the ref pose from
			const FName RefSeqName = FName( DlgResult );
			UAnimSequence* RefAnimSeq = SelectedAnimSet->FindAnimSequence(RefSeqName);

			// Skip if we didn't find the animation we wanted.
			if( RefAnimSeq  || RefSeqName == TEXT("Reference Pose") )
			{
				// Convert source animsequence to an additive animation into destination animsequence
				ConvertAnimSeqToAdditive(AnimSeq, DestSeq, RefAnimSeq, SelectedSkelMesh);
			}
		}
	}

	// Refresh list
	UpdateAnimSeqList();
	// Mark Current package as dirty
	SelectedAnimSet->MarkPackageDirty();
}

UBOOL WxAnimSetViewer::ConvertAnimSeqToAdditive(UAnimSequence* SourceAnimSeq, UAnimSequence* DestAnimSeq, UAnimSequence* RefAnimSeq, USkeletalMesh* SkelMesh)
{
	// Make sure all anim sequences belong to the same AnimSet, this makes our life easier as they share the same tracks.
	check(SourceAnimSeq->GetOuter() == DestAnimSeq->GetOuter());
	if( RefAnimSeq )
	{
		check(SourceAnimSeq->GetOuter() == RefAnimSeq->GetOuter());
	}

	// Make sure source anim sequence is not already additive.
	if( SourceAnimSeq->bIsAdditive )
	{
		appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("AnimSequenceAlreadyAdditive"), *SourceAnimSeq->SequenceName.ToString()) );
		return FALSE;
	}

	// AnimSet those animsequences belong to
	UAnimSet* AnimSet = SourceAnimSeq->GetAnimSet();

	// Make sure destination is setup correctly
	DestAnimSeq->RecycleAnimSequence();

	// Copy properties of Source into Dest.
	CopyAnimSequenceProperties(SourceAnimSeq, DestAnimSeq);

	// New name
	DestAnimSeq->SequenceName		= FName( *FString::Printf(TEXT("ADD_%s"), *SourceAnimSeq->SequenceName.ToString()) );
	DestAnimSeq->bIsAdditive		= TRUE;
	DestAnimSeq->AdditiveRefName	= RefAnimSeq ? RefAnimSeq->SequenceName : FName( TEXT("Reference Pose") );

	// Make sure data is zeroed
	DestAnimSeq->RawAnimData.AddZeroed( AnimSet->TrackBoneNames.Num() );
	DestAnimSeq->AdditiveRefPose.Add( AnimSet->TrackBoneNames.Num() );

	// Verify that number of tracks are matching.
	check( (AnimSet->TrackBoneNames.Num() == SourceAnimSeq->RawAnimData.Num()) && (AnimSet->TrackBoneNames.Num() == DestAnimSeq->RawAnimData.Num()) );
	if( RefAnimSeq )
	{
		check(AnimSet->TrackBoneNames.Num() == RefAnimSeq->RawAnimData.Num());
	}

	FBoneAtom RefBoneAtom = FBoneAtom::Identity;

	// Import each track.
	for(INT TrackIdx=0; TrackIdx<AnimSet->TrackBoneNames.Num(); TrackIdx++)
	{
		// Figure out which bone this track is mapped to
		const INT BoneIndex = SkelMesh->MatchRefBone(AnimSet->TrackBoneNames(TrackIdx));

		// Get ref pose for this track, if no sequence passed use the SkelMesh reference skeleton
		if( RefAnimSeq != NULL )
		{
			RefBoneAtom.Translation = RefAnimSeq->RawAnimData(TrackIdx).PosKeys(0);
			RefBoneAtom.Rotation = RefAnimSeq->RawAnimData(TrackIdx).RotKeys(0);
		}
		else
		{
			const FMeshBone& RefSkelBone = SkelMesh->RefSkeleton(BoneIndex);
			RefBoneAtom.Translation = RefSkelBone.BonePos.Position;
			RefBoneAtom.Rotation    = RefSkelBone.BonePos.Orientation;
			RefBoneAtom.Rotation.W *= -1.f; // See note below about "quarternion fix for ActorX exported quaternions"
		}

		// Save off RefPose into destination AnimSequence
		DestAnimSeq->AdditiveRefPose(TrackIdx) = RefBoneAtom;

		// Go through all keys and turns them into additive
		// That is the difference to the Reference pose.
		FRawAnimSequenceTrack& DestRawTrack = DestAnimSeq->RawAnimData(TrackIdx);
		FRawAnimSequenceTrack& SourceRawTrack = SourceAnimSeq->RawAnimData(TrackIdx);

		DestRawTrack.PosKeys.Reset();
		DestRawTrack.RotKeys.Reset();
		DestRawTrack.KeyTimes.Reset();

		// Do tracks separatey as they can have different number of keys(!)
		DestRawTrack.KeyTimes.Add( SourceRawTrack.KeyTimes.Num() );
		for(INT KeyIdx=0; KeyIdx<SourceRawTrack.KeyTimes.Num(); KeyIdx++)
		{
			DestRawTrack.KeyTimes(KeyIdx) = SourceRawTrack.KeyTimes(KeyIdx);
		}

		DestRawTrack.PosKeys.Add( SourceRawTrack.PosKeys.Num() );
		for(INT KeyIdx=0; KeyIdx<SourceRawTrack.PosKeys.Num(); KeyIdx++)
		{
			DestRawTrack.PosKeys(KeyIdx) = SourceRawTrack.PosKeys(KeyIdx) - RefBoneAtom.Translation;
		}

		DestRawTrack.RotKeys.Add( SourceRawTrack.RotKeys.Num() );
		for(INT KeyIdx=0; KeyIdx<SourceRawTrack.RotKeys.Num(); KeyIdx++)
		{
			// For rotation part. We have this annoying thing to work around...
			// See UAnimNodeSequence::GetAnimationPose()
			// Make delta with "quarternion fix for ActorX exported quaternions". Then revert back.
			if( BoneIndex > 0 || BoneIndex == INDEX_NONE )
			{
				SourceRawTrack.RotKeys(KeyIdx).W *= -1.f;
				RefBoneAtom.Rotation.W *= -1.f;
			}

			// Actual delta.
			DestRawTrack.RotKeys(KeyIdx) = SourceRawTrack.RotKeys(KeyIdx) * (-RefBoneAtom.Rotation);
			
			// Convert back to non "quarternion fix for ActorX exported quaternions".
			if( BoneIndex > 0 || BoneIndex == INDEX_NONE )
			{
				DestRawTrack.RotKeys(KeyIdx).W *= -1.f;
				SourceRawTrack.RotKeys(KeyIdx).W *= -1.f;
				RefBoneAtom.Rotation.W *= -1.f;
			}

			// Normalize resulting quaternion.
			DestRawTrack.RotKeys(KeyIdx).Normalize();
		}
	}

	// See if SourceAnimSeq had a compression Scheme
	FAnimationUtils::CompressAnimSequence(DestAnimSeq, NULL, FALSE, FALSE);

	return TRUE;
}

/**
 * Pop up a dialog asking for axis and angle (in debgrees), and apply that rotation to all keys in selected sequence.
 * Basically 
 */
void WxAnimSetViewer::SequenceApplyRotation()
{
	if(SelectedAnimSeq)
	{
		WxDlgRotateAnimSeq dlg;
		INT Result = dlg.ShowModal();
		if( Result == wxID_OK )
		{
			// Angle (in radians) to rotate AnimSequence by
			FLOAT RotAng = dlg.Degrees * (PI/180.f);

			// Axis to rotate AnimSequence about.
			FVector RotAxis;
			if( dlg.Axis == AXIS_X )
			{
				RotAxis = FVector(1.f, 0.f, 0.f);
			}
			else if( dlg.Axis == AXIS_Y )
			{
				RotAxis = FVector(0.f, 1.f, 0.0f);
			}
			else if( dlg.Axis == AXIS_Z )
			{
				RotAxis = FVector(0.f, 0.f, 1.0f);
			}
			else
			{
				check(0);
			}

			// Make transformation matrix out of rotation (via quaternion)
			const FQuat RotQuat( RotAxis, RotAng );

			// Hmm.. animations don't have any idea of heirarchy, so we just rotate the first track. Can we be sure track 0 is the root bone?
			FRawAnimSequenceTrack& RawTrack = SelectedAnimSeq->RawAnimData(0);
			for( INT r = 0 ; r < RawTrack.RotKeys.Num() ; ++r )
			{
				FQuat& Quat = RawTrack.RotKeys(r);
				Quat = RotQuat * Quat;
				Quat.Normalize();
			}
			for( INT p = 0 ; p < RawTrack.PosKeys.Num() ; ++p)
			{
				RawTrack.PosKeys(p) = RotQuat.RotateVector(RawTrack.PosKeys(p));
			}

			RecompressSequence(SelectedAnimSeq, SelectedSkelMesh);

			SelectedAnimSeq->MarkPackageDirty();
		}
	}
}

void WxAnimSetViewer::SequenceReZeroToCurrent()
{
	if(SelectedAnimSeq)
	{
		// Find vector that would translate current root bone location onto origin.
		FVector ApplyTranslation = -1.f * PreviewSkelComp->SpaceBases(0).GetOrigin();

		// Convert into world space and eliminate 'z' translation. Don't want to move character into ground.
		FVector WorldApplyTranslation = PreviewSkelComp->LocalToWorld.TransformNormal(ApplyTranslation);
		WorldApplyTranslation.Z = 0.f;
		ApplyTranslation = PreviewSkelComp->LocalToWorld.InverseTransformNormal(WorldApplyTranslation);

		// As above, animations don't have any idea of heirarchy, so we don't know for sure if track 0 is the root bone's track.
		FRawAnimSequenceTrack& RawTrack = SelectedAnimSeq->RawAnimData(0);
		for(INT i=0; i<RawTrack.PosKeys.Num(); i++)
		{
			RawTrack.PosKeys(i) += ApplyTranslation;
		}

		RecompressSequence(SelectedAnimSeq, SelectedSkelMesh);
		SelectedAnimSeq->MarkPackageDirty();
	}
}

/**
 * Crop a sequence either before or after the current position. This is made slightly more complicated due to the basic compression
 * we do where tracks which had all identical keyframes are reduced to just 1 frame.
 * 
 * @param bFromStart Should we remove the sequence before or after the selected position.
 */
void WxAnimSetViewer::SequenceCrop(UBOOL bFromStart)
{
	if(SelectedAnimSeq)
	{
		// Can't crop cooked animations.
		const UPackage* SeqPackage = SelectedAnimSeq->GetOutermost();
		if( SeqPackage->PackageFlags & PKG_Cooked )
		{
			appMsgf( AMT_OK, *LocalizeUnrealEd("Error_OperationDisallowedOnCookedContent") );
			return;
		}

		// Crop the raw anim data.
		SelectedAnimSeq->CropRawAnimData( PreviewAnimNode->CurrentTime, bFromStart );

		if(bFromStart)
		{
			PreviewAnimNode->CurrentTime = 0.f;
		}
		else
		{		
			PreviewAnimNode->CurrentTime = SelectedAnimSeq->SequenceLength;
		}

		UpdateAnimSeqList();
		SetSelectedAnimSequence(SelectedAnimSeq);
	}
}

/** Tool for updating the bounding sphere/box of the selected skeletal mesh. Shouldn't generally be needed, except for fixing stuff up possibly. */
void WxAnimSetViewer::UpdateMeshBounds()
{
	if(SelectedSkelMesh)
	{
		FBox BoundingBox(0);
		check(SelectedSkelMesh->LODModels.Num() > 0);
		FStaticLODModel& LODModel = SelectedSkelMesh->LODModels(0);

		for(INT ChunkIndex = 0;ChunkIndex < LODModel.Chunks.Num();ChunkIndex++)
		{
			const FSkelMeshChunk& Chunk = LODModel.Chunks(ChunkIndex);
			for(INT i=0; i<Chunk.RigidVertices.Num(); i++)
			{
				BoundingBox += Chunk.RigidVertices(i).Position;
			}
			for(INT i=0; i<Chunk.SoftVertices.Num(); i++)
			{
				BoundingBox += Chunk.SoftVertices(i).Position;
			}
		}

		const FBox Temp			= BoundingBox;
		const FVector MidMesh	= 0.5f*(Temp.Min + Temp.Max);
		BoundingBox.Min			= Temp.Min + 1.0f*(Temp.Min - MidMesh);
		BoundingBox.Max			= Temp.Max + 1.0f*(Temp.Max - MidMesh);

		// Tuck up the bottom as this rarely extends lower than a reference pose's (e.g. having its feet on the floor).
		BoundingBox.Min.Z	= Temp.Min.Z + 0.1f*(Temp.Min.Z - MidMesh.Z);
		SelectedSkelMesh->Bounds = FBoxSphereBounds(BoundingBox);

		SelectedSkelMesh->MarkPackageDirty();
	}
}

/** Update the external force on the cloth sim. */
void WxAnimSetViewer::UpdateClothWind()
{
	const FVector WindDir = WindRot.Vector();
	PreviewSkelComp->ClothWind = WindDir * WindStrength;
}

// Skeleton Tree Manager

/** Posts an event for regenerating the Skeleton Tree control. */
void WxAnimSetViewer::FillSkeletonTree()
{
	wxCommandEvent Event;
	Event.SetEventObject(this);
	Event.SetEventType(IDM_ANIMSET_FILLSKELETONTREE);
	GetEventHandler()->AddPendingEvent(Event);
}

static wxTreeItemId AnimSetViewer_CreateTreeItemAndParents(INT BoneIndex, TArray<FMeshBone>& Skeleton, wxTreeCtrl* TreeCtrl, TMap<FName,wxTreeItemId>& BoneTreeItemMap)
{
	wxTreeItemId* existingItem = BoneTreeItemMap.Find( Skeleton(BoneIndex).Name );
	if( existingItem != NULL )
	{
		return *existingItem;
	}

	FMeshBone& Bone = Skeleton(BoneIndex);
	wxTreeItemId newItem;
	if( BoneIndex == 0 )
	{
		newItem = TreeCtrl->AddRoot( *FString::Printf(TEXT("%s (%d)"), *Bone.Name.ToString(), BoneIndex) );
	}
	else
	{
		wxTreeItemId parentItem = AnimSetViewer_CreateTreeItemAndParents(Bone.ParentIndex, Skeleton, TreeCtrl, BoneTreeItemMap);
		newItem = TreeCtrl->AppendItem( parentItem, *FString::Printf(TEXT("%s (%d)"), *Bone.Name.ToString(), BoneIndex) );
		TreeCtrl->Expand( parentItem );
	}    

	BoneTreeItemMap.Set(Bone.Name, newItem);

	return newItem;
}

/**
 * Event handler for regenerating the tree view data.
 *
 * @param	In	Information about the event.
 */
void WxAnimSetViewer::OnFillSkeletonTree(wxCommandEvent &In)
{
	// We don't have a skeletal mesh, skip update.
	if( !SelectedSkelMesh )
	{
		return;
	}

	SkeletonTreeCtrl->Freeze();
	{
		SkeletonTreeCtrl->DeleteAllItems();

		SkeletonTreeItemBoneIndexMap.Empty();

		TMap<FName,wxTreeItemId>	BoneTreeItemMap;

		// Fill Tree with all bones...
		for(INT BoneIndex=0; BoneIndex<SelectedSkelMesh->RefSkeleton.Num(); BoneIndex++)
		{
			wxTreeItemId newItem = AnimSetViewer_CreateTreeItemAndParents(BoneIndex, SelectedSkelMesh->RefSkeleton, SkeletonTreeCtrl, BoneTreeItemMap);
			
			// Mapping between bone index and tree item.
			SkeletonTreeItemBoneIndexMap.Set(newItem, BoneIndex);
		}
	}
	SkeletonTreeCtrl->Thaw();
}

void WxAnimSetViewer::OnSkeletonTreeItemRightClick(wxTreeEvent& In)
{
	// Pop up menu
	WxSkeletonTreePopUpMenu Menu( this );
	FTrackPopupMenu TrackMenu( this, &Menu );
	TrackMenu.Show();
}

void WxAnimSetViewer::OnSkeletonTreeMenuHandleCommand(wxCommandEvent &In)
{
	if( !SkeletonTreeCtrl || !SelectedSkelMesh )
	{
		return;
	}

	wxArrayTreeItemIds SelectionSet;
	INT NumSelections = SkeletonTreeCtrl->GetSelections(SelectionSet);
	if( NumSelections == 0 )
	{
		return;
	}

	// Id of selection in popup menu.
	INT Command = In.GetId();
	debugf(TEXT("OnSkeletonTreeMenuHandleCommand. Command: %d"), Command);

	TArray<BYTE> BoneSelectionMask;
	BoneSelectionMask.AddZeroed(SelectedSkelMesh->RefSkeleton.Num());

	// Build a list of selected bones.
	debugf(TEXT("  BoneSelectionMask building."));
	for(INT SelectionIdx=0; SelectionIdx<NumSelections; SelectionIdx++)
	{
		wxTreeItemId SelectionItemId = SelectionSet[SelectionIdx];

		const INT* BoneIdxPtr = SkeletonTreeItemBoneIndexMap.Find( SelectionItemId );
		INT BoneIndex = *BoneIdxPtr;

		// Flag bone as affected
		BoneSelectionMask(BoneIndex) = 1;

		debugf(TEXT("    Bone: %s flagged as selected."), *SelectedSkelMesh->RefSkeleton(BoneIndex).Name.ToString());
	}

	// If we select child bones as well, then add those to the selection mask.
	if( Command == IDM_ANIMSET_SKELETONTREE_SHOWCHILDBONE || Command == IDM_ANIMSET_SKELETONTREE_HIDECHILDBONE || Command == IDM_ANIMSET_SKELETONTREE_SETCHILDBONECOLOR )
	{
		debugf(TEXT("  Adding children to selection mask."));
		// Skip root bone, he can't be a child.
		for(INT BoneIndex=1; BoneIndex<BoneSelectionMask.Num(); BoneIndex++)
		{
			BoneSelectionMask(BoneIndex) |= BoneSelectionMask(SelectedSkelMesh->RefSkeleton(BoneIndex).ParentIndex);
		}
	}

	debugf(TEXT("  Apply changes."));
	// Go through bone list and apply changes.
	FColor NewBoneColor;
	FString BoneNames;
	UBOOL bFirstBone = TRUE;
	for(INT BoneIndex=0; BoneIndex<BoneSelectionMask.Num(); BoneIndex++)
	{
		// Skip bones which were not selected.
		if( BoneSelectionMask(BoneIndex) == 0 )
		{
			continue;
		}

		debugf(TEXT("    Processing bone: %s."), *SelectedSkelMesh->RefSkeleton(BoneIndex).Name.ToString());

		FMeshBone& Bone = SelectedSkelMesh->RefSkeleton(BoneIndex);
		if( Command == IDM_ANIMSET_SKELETONTREE_SHOWBONE || Command == IDM_ANIMSET_SKELETONTREE_SHOWCHILDBONE )
		{
			Bone.BoneColor.A = 255;
		}
		else if( Command == IDM_ANIMSET_SKELETONTREE_HIDEBONE || Command == IDM_ANIMSET_SKELETONTREE_HIDECHILDBONE )
		{
			Bone.BoneColor.A = 0;
		}
		else if( Command == IDM_ANIMSET_SKELETONTREE_SETBONECOLOR || Command == IDM_ANIMSET_SKELETONTREE_SETCHILDBONECOLOR )
		{
			if( bFirstBone )
			{
				// Initialize the color data for the picker window.
				wxColourData ColorData;
				ColorData.SetChooseFull( true );
				ColorData.SetColour( wxColour(Bone.BoneColor.R, Bone.BoneColor.G, Bone.BoneColor.B) );

				WxDlgColor ColorDialog;
				ColorDialog.Create(this, &ColorData );
				if( ColorDialog.ShowModal() == wxID_OK)
				{
					const wxColour clr = ColorDialog.GetColourData().GetColour();
					NewBoneColor = FColor( clr.Red(), clr.Green(), clr.Blue() );
				}
				else
				{
					break;
				}
			}

			// Don't touch Alpha as that is used to toggle per bone drawing.
			Bone.BoneColor.R = NewBoneColor.R;
			Bone.BoneColor.G = NewBoneColor.G;
			Bone.BoneColor.B = NewBoneColor.B;
		}
		else if( Command == IDM_ANIMSET_SKELETONTREE_COPYBONENAME )
		{
			BoneNames += FString::Printf( TEXT("%s\r\n"), *Bone.Name.ToString());
		}

		bFirstBone = FALSE;
	}

	if( Command == IDM_ANIMSET_SKELETONTREE_COPYBONENAME )
	{
		appClipboardCopy( *BoneNames );
	}
}
