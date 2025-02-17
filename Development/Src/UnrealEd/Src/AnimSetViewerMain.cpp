/*=============================================================================
	AnimSetViewerMain.cpp: AnimSet viewer main
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "EngineAnimClasses.h"
#include "AnimSetViewer.h"
#include "MouseDeltaTracker.h"
#include "Properties.h"
#include "DlgGenericComboEntry.h"
#include "SocketManager.h"
#include "DlgAnimationCompression.h"
#include "AnimationUtils.h"

IMPLEMENT_CLASS(UASVSkelComponent);

static const FColor ExtraMesh1WireColor = FColor(255,215,215,255);
static const FColor ExtraMesh2WireColor = FColor(215,255,215,255);
static const FColor ExtraMesh3WireColor = FColor(215,215,255,255);

static const FLOAT	AnimSetViewer_TranslateSpeed = 0.25f;
static const FLOAT	AnimSetViewer_RotateSpeed = 0.02f;
static const FLOAT	AnimSetViewer_LightRotSpeed = 40.0f;
static const FLOAT	AnimSetViewer_WindStrengthSpeed = 0.1f;

/**
 * A hit proxy class for sockets.
 */
struct HASVSocketProxy : public HHitProxy
{
	DECLARE_HIT_PROXY( HASVSocketProxy, HHitProxy );

	INT		SocketIndex;

	HASVSocketProxy(INT InSocketIndex)
		:	HHitProxy( HPP_UI )
		,	SocketIndex( InSocketIndex )
	{}
};

FASVViewportClient::FASVViewportClient( WxAnimSetViewer* InASV ):
	PreviewScene(FRotator(-8192,-8192,0),0.25f,1.f,TRUE)
{
	AnimSetViewer = InASV;

	// Setup defaults for the common draw helper.
	DrawHelper.bDrawPivot = false;
	DrawHelper.bDrawWorldBox = false;
	DrawHelper.bDrawKillZ = false;
	DrawHelper.GridColorHi = FColor(80,80,80);
	DrawHelper.GridColorLo = FColor(72,72,72);
	DrawHelper.PerspectiveGridSize = 32767;

	// Create AnimNodeSequence for previewing animation playback
	InASV->PreviewAnimNode = ConstructObject<UAnimNodeSequence>(UAnimNodeSequence::StaticClass());
	InASV->PreviewAnimNode->bEditorOnlyAddRefPoseToAdditiveAnimation = TRUE;
	InASV->PreviewAnimNodeRaw = ConstructObject<UAnimNodeSequence>(UAnimNodeSequence::StaticClass());
	InASV->PreviewAnimNodeRaw->bEditorOnlyAddRefPoseToAdditiveAnimation = TRUE;

	// Mirror node for testing that.
	InASV->PreviewAnimMirror = ConstructObject<UAnimNodeMirror>(UAnimNodeMirror::StaticClass());
	InASV->PreviewAnimMirror->Children(0).Anim = InASV->PreviewAnimNode;
	InASV->PreviewAnimMirror->bEnableMirroring = false; // Default to mirroring off.

	InASV->PreviewAnimMirrorRaw = ConstructObject<UAnimNodeMirror>(UAnimNodeMirror::StaticClass());
	InASV->PreviewAnimMirrorRaw->Children(0).Anim = InASV->PreviewAnimNodeRaw;
	InASV->PreviewAnimMirrorRaw->bEnableMirroring = false; // Default to mirroring off.

	// Morph pose node for previewing morph targets.
	InASV->PreviewMorphPose = ConstructObject<UMorphNodePose>(UMorphNodePose::StaticClass());

	// Finally, AnimTree to hold them all
	InASV->PreviewAnimTree = ConstructObject<UAnimTree>(UAnimTree::StaticClass());
	InASV->PreviewAnimTree->Children(0).Anim = InASV->PreviewAnimMirror;
	InASV->PreviewAnimTree->RootMorphNodes.AddItem(InASV->PreviewMorphPose);

	InASV->PreviewAnimTreeRaw = ConstructObject<UAnimTree>(UAnimTree::StaticClass());
	InASV->PreviewAnimTreeRaw->Children(0).Anim = InASV->PreviewAnimMirrorRaw;
	InASV->PreviewAnimTreeRaw->RootMorphNodes.AddItem(InASV->PreviewMorphPose);

	// Create SkeletalMeshComponent for rendering skeletal mesh
	InASV->PreviewSkelComp = ConstructObject<UASVSkelComponent>(UASVSkelComponent::StaticClass());
	InASV->PreviewSkelComp->Animations = InASV->PreviewAnimTree;
	InASV->PreviewSkelComp->AnimSetViewerPtr = InASV;
	// Setting bUpdateSkelWhenNotRendered to true below forces updates to the renderer thread on each frame.
	// [[ When this is not set, the preview rendering in the ASV may fail because the code in
	//    USkeletalMeshComponent::Tick that tries to determine whether the component was recently rendered
	//    seems to fail due to bad timing data. ]]
	InASV->PreviewSkelComp->bUpdateSkelWhenNotRendered = TRUE;
	PreviewScene.AddComponent(InASV->PreviewSkelComp,FMatrix::Identity);

	InASV->PreviewSkelCompRaw = ConstructObject<UASVSkelComponent>(UASVSkelComponent::StaticClass());
	InASV->PreviewSkelCompRaw->Animations = InASV->PreviewAnimTreeRaw;
	InASV->PreviewSkelCompRaw->AnimSetViewerPtr = InASV;
	InASV->PreviewSkelCompRaw->bUseRawData = TRUE;
	InASV->PreviewSkelCompRaw->BoneColor = FColor(255, 128, 0);
	InASV->PreviewSkelCompRaw->bDrawMesh = FALSE;
	PreviewScene.AddComponent(InASV->PreviewSkelCompRaw,FMatrix::Identity);

	// Create 3 more SkeletalMeshComponents to show extra 'overlay' meshes.
	InASV->PreviewSkelCompAux1 = ConstructObject<USkeletalMeshComponent>(USkeletalMeshComponent::StaticClass());
	InASV->PreviewSkelCompAux1->ParentAnimComponent = InASV->PreviewSkelComp;
	InASV->PreviewSkelCompAux1->WireframeColor = ExtraMesh1WireColor;
	PreviewScene.AddComponent(InASV->PreviewSkelCompAux1,FMatrix::Identity);

	InASV->PreviewSkelCompAux2 = ConstructObject<USkeletalMeshComponent>(USkeletalMeshComponent::StaticClass());
	InASV->PreviewSkelCompAux2->ParentAnimComponent = InASV->PreviewSkelComp;
	InASV->PreviewSkelCompAux2->WireframeColor = ExtraMesh2WireColor;
	PreviewScene.AddComponent(InASV->PreviewSkelCompAux2,FMatrix::Identity);

	InASV->PreviewSkelCompAux3 = ConstructObject<USkeletalMeshComponent>(USkeletalMeshComponent::StaticClass());
	InASV->PreviewSkelCompAux3->ParentAnimComponent = InASV->PreviewSkelComp;
	InASV->PreviewSkelCompAux3->WireframeColor = ExtraMesh3WireColor;
	PreviewScene.AddComponent(InASV->PreviewSkelCompAux3,FMatrix::Identity);

	ShowFlags = SHOW_DefaultEditor;

	// Set the viewport to be fully lit.
	ShowFlags &= ~SHOW_ViewMode_Mask;
	ShowFlags |= SHOW_ViewMode_Lit;

	bShowBoneNames = false;
	bShowFloor = false;
	bShowSockets = false;

	bManipulating = false;
	ManipulateAxis = AXIS_None;
	DragDirX = 0.f;
	DragDirY = 0.f;
	WorldManDir = FVector(0.f);
	LocalManDir = FVector(0.f);

	NearPlane = 1.0f;

	// For making sound notifies work.
	bAudioRealTime = true;

	SetRealtime( true );

	bAllowMayaCam = TRUE;
}

FASVViewportClient::~FASVViewportClient()
{
}

void FASVViewportClient::Serialize(FArchive& Ar)
{ 
	Ar << Input; 
	Ar << PreviewScene;
}

FLinearColor FASVViewportClient::GetBackgroundColor()
{
	return FColor(64,64,64);
}

void FASVViewportClient::Draw(const FSceneView* View,FPrimitiveDrawInterface* PDI)
{
	// Draw common controls.
	DrawHelper.Draw(View, PDI);

	// Generate matrices for all sockets if we are drawing sockets.
	if(bShowSockets || AnimSetViewer->SocketMgr)
	{
		for(INT i=0; i<AnimSetViewer->SelectedSkelMesh->Sockets.Num(); i++)
		{
			USkeletalMeshSocket* Socket = AnimSetViewer->SelectedSkelMesh->Sockets(i);

			FMatrix SocketTM;
			if( Socket->GetSocketMatrix(SocketTM, AnimSetViewer->PreviewSkelComp) )
			{
				PDI->SetHitProxy( new HASVSocketProxy(i) );
				DrawWireDiamond(PDI, SocketTM, 2.f, FColor(255,128,128), SDPG_Foreground );
				PDI->SetHitProxy( NULL );

				// Draw widget for selected socket if Socket Manager is up.
				if(AnimSetViewer->SocketMgr && AnimSetViewer->SelectedSocket && AnimSetViewer->SelectedSocket == Socket)
				{
					const EAxis HighlightAxis = ManipulateAxis;

					// Info1 & Info2 are unused
					if(AnimSetViewer->SocketMgr->MoveMode == SMM_Rotate)
					{
						FUnrealEdUtils::DrawWidget(View, PDI, SocketTM, 0, 0, HighlightAxis, WMM_Rotate);
					}
					else
					{
						FUnrealEdUtils::DrawWidget(View, PDI, SocketTM, 0, 0, HighlightAxis, WMM_Translate);
					}
				}
			}
		}
	}

	//FEditorLevelViewportClient::Draw(View, PDI);
}

void FASVViewportClient::Draw(FViewport* Viewport, FCanvas* Canvas)
{
	// Do main viewport drawing stuff.
	FEditorLevelViewportClient::Draw(Viewport, Canvas);

	INT XL, YL;
	StringSize( GEngine->SmallFont,  XL, YL, TEXT("L") );

	FSceneViewFamilyContext ViewFamily(Viewport,GetScene(),ShowFlags,GWorld->GetTimeSeconds(),GWorld->GetDeltaSeconds(),GWorld->GetRealTimeSeconds(),NULL);
	FSceneView* View = CalcSceneView(&ViewFamily);
	USkeletalMeshComponent* MeshComp = AnimSetViewer->PreviewSkelComp;
	USkeletalMesh*			SkelMesh = MeshComp ? MeshComp->SkeletalMesh : NULL;

	const INT HalfX = Viewport->GetSizeX()/2;
	const INT HalfY = Viewport->GetSizeY()/2;

	check(AnimSetViewer->PreviewSkelComp);
	const INT LODIndex = ::Clamp(AnimSetViewer->PreviewSkelComp->PredictedLODLevel, 0, AnimSetViewer->SelectedSkelMesh->LODModels.Num()-1);

	FStaticLODModel& LODModel = AnimSetViewer->SelectedSkelMesh->LODModels( LODIndex );

	if( bShowBoneNames && SkelMesh )
	{
		for(INT i=0; i< LODModel.RequiredBones.Num(); i++)
		{
			const INT BoneIndex	= LODModel.RequiredBones(i);

			FColor& BoneColor = SkelMesh->RefSkeleton(BoneIndex).BoneColor;
			if( BoneColor.A != 0 )
			{
				const FVector BonePos = MeshComp->LocalToWorld.TransformFVector(MeshComp->SpaceBases(BoneIndex).GetOrigin());

				const FPlane proj = View->Project(BonePos);
				if( proj.W > 0.f )
				{
					const INT XPos = HalfX + ( HalfX * proj.X );
					const INT YPos = HalfY + ( HalfY * (proj.Y * -1) );

					const FName BoneName		= SkelMesh->RefSkeleton(BoneIndex).Name;
					const FString BoneString	= FString::Printf( TEXT("%d: %s"), BoneIndex, *BoneName.ToString() );

					DrawString(Canvas,XPos, YPos, *BoneString, GEngine->SmallFont, BoneColor);
				}
			}
		}
	}

	// Draw socket names if desired.
	if( bShowSockets  || AnimSetViewer->SocketMgr )
	{
		for(INT i=0; i<AnimSetViewer->SelectedSkelMesh->Sockets.Num(); i++)
		{
			USkeletalMeshSocket* Socket = AnimSetViewer->SelectedSkelMesh->Sockets(i);

			FMatrix SocketTM;
			if( Socket->GetSocketMatrix(SocketTM, MeshComp) )
			{
				const FVector SocketPos	= SocketTM.GetOrigin();
				const FPlane proj		= View->Project( SocketPos );
				if(proj.W > 0.f)
				{
					const INT XPos = HalfX + ( HalfX * proj.X );
					const INT YPos = HalfY + ( HalfY * (proj.Y * -1) );
					DrawString(Canvas,XPos, YPos, *Socket->SocketName.ToString(), GEngine->SmallFont, FColor(255,196,196));
				}

			}
		}
	}


	// Draw stats about the mesh
	const FBoxSphereBounds& SkelBounds = AnimSetViewer->PreviewSkelComp->Bounds;
	const FPlane ScreenPosition = View->Project(SkelBounds.Origin);
	const FLOAT ScreenRadius = Max((FLOAT)HalfX * View->ProjectionMatrix.M[0][0], (FLOAT)HalfY * View->ProjectionMatrix.M[1][1]) * SkelBounds.SphereRadius / Max(ScreenPosition.W,1.0f);
	const FLOAT LODFactor = ScreenRadius / 320.0f;

	FString InfoString = FString::Printf( TEXT("LOD [%d] Bones:%d Polys:%d (Display Factor:%3.2f)"), 
		LODIndex, 
		LODModel.RequiredBones.Num(), 
		LODModel.GetTotalFaces(), 
		LODFactor );

	INT CurYOffset=5;
	DrawString(Canvas, 5, CurYOffset, *InfoString, GEngine->SmallFont, FColor(255,255,255) );

	UINT NumRigidVertices = 0;
	UINT NumSoftVertices = 0;
	for(INT ChunkIndex = 0;ChunkIndex < LODModel.Chunks.Num();ChunkIndex++)
	{
		NumRigidVertices += LODModel.Chunks(ChunkIndex).GetNumRigidVertices();
		NumSoftVertices += LODModel.Chunks(ChunkIndex).GetNumSoftVertices();
	}

	InfoString = FString::Printf( TEXT("Verts:%d (Rigid:%d Soft:%d)"), 
		LODModel.NumVertices, 
		NumRigidVertices,
		NumSoftVertices );

	CurYOffset += YL + 2;
	DrawString(Canvas, 5, CurYOffset, *InfoString, GEngine->SmallFont, FColor(255,255,255) );

	InfoString = FString::Printf( TEXT("Chunks:%d Sections:%d"), 
		LODModel.Chunks.Num(), 
		LODModel.Sections.Num() );

	CurYOffset += YL + 2;
	DrawString(Canvas, 5, CurYOffset, *InfoString, GEngine->SmallFont, FColor(255,255,255) );

	if( AnimSetViewer->SelectedAnimSeq && AnimSetViewer->PreviewAnimNode && LODModel.RequiredBones.Num() > 0 )
	{
		const INT RootBoneIndex	= LODModel.RequiredBones(0);

		if( MeshComp && MeshComp->SpaceBases.Num() > RootBoneIndex )
		{
			const UAnimSequence* AnimSeq = AnimSetViewer->SelectedAnimSeq;
			UAnimSet* AnimSet = AnimSeq->GetAnimSet();
			check(AnimSet);
			FAnimSetMeshLinkup* AnimLinkup = &AnimSet->LinkupCache(AnimSetViewer->PreviewAnimNode->AnimLinkupIndex);
			check(AnimLinkup);
			const INT RootBoneTrackIndex = AnimLinkup->BoneToTrackTable(RootBoneIndex);
			if(RootBoneTrackIndex != INDEX_NONE)
			{
				FBoneAtom FirstFrameAtom;
				AnimSeq->GetBoneAtom(FirstFrameAtom, RootBoneTrackIndex, 0.f, FALSE, MeshComp->bUseRawData);

				FBoneAtom CurrentFrameAtom;
				AnimSeq->GetBoneAtom(CurrentFrameAtom, RootBoneTrackIndex, AnimSetViewer->PreviewAnimNode->CurrentTime, FALSE, MeshComp->bUseRawData);

				const FVector RootBoneDelta = MeshComp->LocalToWorld.TransformNormal(CurrentFrameAtom.Translation - FirstFrameAtom.Translation);
				InfoString = FString::Printf( TEXT("Root Bone Delta: %s"), *RootBoneDelta.ToString());

				CurYOffset += YL + 2;
				DrawString(Canvas, 5, CurYOffset, *InfoString, GEngine->SmallFont, FColor(255,255,255) );
			}
		}
	}

	// Draw anim notify viewer.
	if( AnimSetViewer->SelectedAnimSeq )
	{
		const INT NotifyViewBorderX = 10;
		const INT NotifyViewBorderY = 10;
		const INT NotifyViewEndHeight = 10;

		const INT SizeX = Viewport->GetSizeX();
		const INT SizeY = Viewport->GetSizeY();

		const UAnimSequence* AnimSeq = AnimSetViewer->SelectedAnimSeq;

		const FLOAT PixelsPerSecond = (FLOAT)(SizeX - (2*NotifyViewBorderX))/AnimSeq->SequenceLength;

		// Draw ends of track
		DrawLine2D(Canvas, FVector2D(NotifyViewBorderX, SizeY - NotifyViewBorderY - NotifyViewEndHeight), FVector2D(NotifyViewBorderX, SizeY - NotifyViewBorderY), FColor(255,255,255) );
		DrawLine2D(Canvas, FVector2D(SizeX - NotifyViewBorderX, SizeY - NotifyViewBorderY - NotifyViewEndHeight), FVector2D(SizeX - NotifyViewBorderX, SizeY - NotifyViewBorderY), FColor(255,255,255) );

		// Draw track itself
		const INT TrackY = SizeY - NotifyViewBorderY - (0.5f*NotifyViewEndHeight);
		DrawLine2D(Canvas, FVector2D(NotifyViewBorderX, TrackY), FVector2D(SizeX - NotifyViewBorderX, TrackY), FColor(255,255,255) );

		// Draw notifies on the track
		for(INT i=0; i<AnimSeq->Notifies.Num(); i++)
		{
			const INT NotifyX = NotifyViewBorderX + (PixelsPerSecond * AnimSeq->Notifies(i).Time);

			DrawLine2D(Canvas, FVector2D(NotifyX, TrackY - (0.5f*NotifyViewEndHeight)), FVector2D(NotifyX, TrackY), FColor(255,200,200) );

			if( AnimSeq->Notifies(i).Comment != NAME_None )
			{
				DrawString(Canvas, NotifyX - 2, TrackY + 2, *AnimSeq->Notifies(i).Comment.ToString(), GEngine->SmallFont, FColor(255,200,200) );
			}
		}
			
		// Draw current position on the track.
		const INT CurrentPosX = NotifyViewBorderX + (PixelsPerSecond * AnimSetViewer->PreviewAnimNode->CurrentTime);
		DrawTriangle2D(Canvas, 
			FVector2D(CurrentPosX, TrackY-1), FVector2D(0.f, 0.f), 
			FVector2D(CurrentPosX+5, TrackY-6), FVector2D(0.f, 0.f), 
			FVector2D(CurrentPosX-5, TrackY-6), FVector2D(0.f, 0.f), 
			FColor(255,255,255) );

		// Draw anim position
		InfoString = FString::Printf(TEXT("Pos: %3.1f pct, Time: %4.4fs, Len: %4.4fs"), 
						100.f*AnimSetViewer->PreviewAnimNode->CurrentTime/AnimSeq->SequenceLength, 
						AnimSetViewer->PreviewAnimNode->CurrentTime, 
						AnimSeq->SequenceLength);

		INT XS, YS;
		StringSize(GEngine->SmallFont, XS, YS, *InfoString);
		DrawString(Canvas,SizeX - XS - XL - NotifyViewBorderX, SizeY - YS - YL - NotifyViewBorderY - NotifyViewEndHeight, *InfoString, GEngine->SmallFont, FColor(255,255,255));

		// Draw if it's an ADDITIVE animation
		if( AnimSetViewer->SelectedAnimSeq->bIsAdditive )
		{
			InfoString = FString::Printf(TEXT("ADDITIVE ANIMATION (Reference: %s)"), *AnimSetViewer->SelectedAnimSeq->AdditiveRefName.ToString());
			CurYOffset += YL + 2;
			DrawString(Canvas, 5, CurYOffset, *InfoString, GEngine->SmallFont, FColor(255,0,0) );
		}
	}

	// If doing cloth sim, show wind direction
	if(AnimSetViewer->PreviewSkelComp->bEnableClothSimulation)
	{
		INT NumClothVerts = AnimSetViewer->SelectedSkelMesh->ClothToGraphicsVertMap.Num();
		INT NumFixedClothVerts = NumClothVerts - AnimSetViewer->SelectedSkelMesh->NumFreeClothVerts;

		InfoString = FString::Printf( TEXT("Cloth Verts:%d (%d Fixed)"), 
			NumClothVerts, 
			NumFixedClothVerts );

		CurYOffset += YL + 2;
		DrawString(Canvas, 5, CurYOffset, *InfoString, GEngine->SmallFont, FColor(255,255,255) );

		DrawWindDir(Viewport, Canvas);
	}

		
	if(AnimSetViewer->SelectedSkelMesh && AnimSetViewer->SelectedSkelMesh->SoftBodySurfaceToGraphicsVertMap.Num() > 0)
	{
		INT NumSurfaceVerts	= AnimSetViewer->SelectedSkelMesh->SoftBodySurfaceToGraphicsVertMap.Num();
		INT NumSurfaceTris	= AnimSetViewer->SelectedSkelMesh->SoftBodySurfaceIndices.Num() / 3;
		INT NumTetraVerts	= AnimSetViewer->SelectedSkelMesh->SoftBodyTetraVertsUnscaled.Num();				
		INT NumTetras		= AnimSetViewer->SelectedSkelMesh->SoftBodyTetraIndices.Num() / 4;		

		InfoString = FString::Printf( TEXT("Soft-Body Surface Triangles: %d, Surface Verts: %d, Tetras: %d, Tetra Verts: %d"), 
			NumSurfaceTris, NumSurfaceVerts, NumTetras, NumTetraVerts);

		CurYOffset += YL + 2;
		DrawString(Canvas, 5, CurYOffset, *InfoString, GEngine->SmallFont, FColor(255,255,255) );
	}

}

/** Util for drawing wind direction. */
void FASVViewportClient::DrawWindDir(FViewport* Viewport, FCanvas* Canvas)
{
	FRotationMatrix ViewTM( this->ViewRotation );

	const INT SizeX = Viewport->GetSizeX();
	const INT SizeY = Viewport->GetSizeY();

	const FIntPoint AxisOrigin( 80, SizeY - 30 );
	const FLOAT AxisSize = 25.f;

	FVector AxisVec = AxisSize * ViewTM.InverseTransformNormal( AnimSetViewer->WindRot.Vector() );
	FIntPoint AxisEnd = AxisOrigin + FIntPoint( AxisVec.Y, -AxisVec.Z );
	DrawLine2D( Canvas, AxisOrigin, AxisEnd, FColor(255,255,128) );

	FString WindString = FString::Printf( TEXT("Wind: %4.1f"), AnimSetViewer->WindStrength );
	DrawString( Canvas, 65, SizeY - 15, *WindString, GEngine->SmallFont, FColor(255,255,128) );
}

void FASVViewportClient::Tick(FLOAT DeltaSeconds)
{
	FEditorLevelViewportClient::Tick(DeltaSeconds);
	AnimSetViewer->TickViewer(DeltaSeconds * AnimSetViewer->PlaybackSpeed);
}

UBOOL FASVViewportClient::InputKey(FViewport* Viewport, INT ControllerId, FName Key, EInputEvent Event,FLOAT AmountDepressed,UBOOL Gamepad)
{
	// Hide and lock mouse cursor if we're capturing mouse input
	Viewport->ShowCursor( !Viewport->HasMouseCapture() );
	Viewport->LockMouseToWindow( Viewport->HasMouseCapture() );

	const INT HitX = Viewport->GetMouseX();
	const INT HitY = Viewport->GetMouseY();

	if(Key == KEY_LeftMouseButton)
	{
		if(Event == IE_Pressed)
		{
			HHitProxy*	HitResult = Viewport->GetHitProxy(HitX,HitY);

			if(HitResult)
			{
				if(HitResult->IsA(HWidgetUtilProxy::StaticGetType()))
				{
					HWidgetUtilProxy* WidgetProxy = (HWidgetUtilProxy*)HitResult;

					ManipulateAxis = WidgetProxy->Axis;

					// Calculate the scree-space directions for this drag.
					FSceneViewFamilyContext ViewFamily(Viewport,GetScene(),ShowFlags,GWorld->GetTimeSeconds(),GWorld->GetDeltaSeconds(),GWorld->GetRealTimeSeconds(),NULL);
					FSceneView* View = CalcSceneView(&ViewFamily);
					WidgetProxy->CalcVectors(View, FViewportClick(View, this, Key, Event, HitX, HitY), LocalManDir, WorldManDir, DragDirX, DragDirY);

					bManipulating = true;
				}
				else if(HitResult->IsA(HASVSocketProxy::StaticGetType()))
				{
					HASVSocketProxy* SocketProxy = (HASVSocketProxy*)HitResult;

					USkeletalMeshSocket* Socket = NULL;
					if(SocketProxy->SocketIndex < AnimSetViewer->SelectedSkelMesh->Sockets.Num())
					{
						Socket = AnimSetViewer->SelectedSkelMesh->Sockets(SocketProxy->SocketIndex);
					}

					if(Socket)
					{
						AnimSetViewer->SetSelectedSocket(Socket);
					}
				}
			}
		}
		else if(Event == IE_Released)
		{
			if( bManipulating )
			{
				ManipulateAxis = AXIS_None;
				bManipulating = false;
			}
		}
	}

	if( Event == IE_Pressed )
	{
		if(Key == KEY_SpaceBar)
		{
			if(AnimSetViewer->SocketMgr)
			{
				if(AnimSetViewer->SocketMgr->MoveMode == SMM_Rotate)
				{
					AnimSetViewer->SetSocketMoveMode( SMM_Translate );
				}
				else
				{
					AnimSetViewer->SetSocketMoveMode( SMM_Rotate );
				}
			}
		}
	}	

	// Do stuff for FEditorLevelViewportClient camera movement.
	if( Event == IE_Pressed) 
	{
		if( Key == KEY_LeftMouseButton || Key == KEY_RightMouseButton || Key == KEY_MiddleMouseButton )
		{
			MouseDeltaTracker->StartTracking( this, HitX, HitY );
		}
	}
	else if( Event == IE_Released )
	{
		if( Key == KEY_LeftMouseButton || Key == KEY_RightMouseButton || Key == KEY_MiddleMouseButton )
		{
			MouseDeltaTracker->EndTracking( this );
		}
	}

	// Handle viewport screenshot.
	InputTakeScreenshot( Viewport, Key, Event );

	return TRUE;
}

/** Handles mouse being moved while input is captured - in this case, while a mouse button is down. */
UBOOL FASVViewportClient::InputAxis(FViewport* Viewport, INT ControllerId, FName Key, FLOAT Delta, FLOAT DeltaTime, UBOOL bGamepad)
{
	// Get some useful info about buttons being held down
	UBOOL bCtrlDown = Viewport->KeyState(KEY_LeftControl) || Viewport->KeyState(KEY_RightControl);
	UBOOL bShiftDown = Viewport->KeyState(KEY_LeftShift) || Viewport->KeyState(KEY_RightShift);
	UBOOL bLightMoveDown = Viewport->KeyState(KEY_L);
	UBOOL bWindMoveDown = Viewport->KeyState(KEY_W);
	UBOOL bRightMouseDown = Viewport->KeyState(KEY_RightMouseButton);

	// Look at which axis is being dragged and by how much
	FLOAT DragX = (Key == KEY_MouseX) ? Delta : 0.f;
	FLOAT DragY = (Key == KEY_MouseY) ? Delta : 0.f;

	if(bManipulating)
	{
		if(AnimSetViewer->SocketMgr && AnimSetViewer->SelectedSocket)
		{
			FLOAT DragMag = (DragX * DragDirX) + (DragY * DragDirY);

			if(AnimSetViewer->SocketMgr->MoveMode == SMM_Rotate)
			{
				FQuat CurrentQuat = AnimSetViewer->SelectedSocket->RelativeRotation.Quaternion();
				FQuat DeltaQuat( LocalManDir, -DragMag * AnimSetViewer_RotateSpeed );
				FQuat NewQuat = CurrentQuat * DeltaQuat;

				AnimSetViewer->SelectedSocket->RelativeRotation = FRotator(NewQuat);

				AnimSetViewer->SocketMgr->UpdateTextEntry();
			}
			else
			{
				FRotationMatrix SocketRotTM( AnimSetViewer->SelectedSocket->RelativeRotation );
				FVector SocketMove = SocketRotTM.TransformNormal( LocalManDir * DragMag * AnimSetViewer_TranslateSpeed );

				AnimSetViewer->SelectedSocket->RelativeLocation += SocketMove;
			}

			AnimSetViewer->UpdateSocketPreviews();

			AnimSetViewer->SelectedSkelMesh->MarkPackageDirty();
		}
	}
	else if(bLightMoveDown)
	{
		FRotator LightDir = PreviewScene.GetLightDirection();
		
		LightDir.Yaw += -DragX * AnimSetViewer_LightRotSpeed;
		LightDir.Pitch += -DragY * AnimSetViewer_LightRotSpeed;

		PreviewScene.SetLightDirection(LightDir);
	}
	else if(bWindMoveDown)
	{
		// If right mouse button is down, turn Y movement into modifying strength
		if(bRightMouseDown)
		{
			AnimSetViewer->WindStrength = ::Max(AnimSetViewer->WindStrength + (DragY * AnimSetViewer_WindStrengthSpeed), 0.f);			
		}
		// Otherwise, orbit the wind
		else
		{
			AnimSetViewer->WindRot.Yaw += -DragX * AnimSetViewer_LightRotSpeed;
			AnimSetViewer->WindRot.Pitch += -DragY * AnimSetViewer_LightRotSpeed;
		}

		AnimSetViewer->UpdateClothWind();
	}
	// If we are not manipulating an axis, use the MouseDeltaTracker to update camera.
	else
	{
		MouseDeltaTracker->AddDelta( this, Key, Delta, 0 );
		const FVector DragDelta = MouseDeltaTracker->GetDelta();

		GEditor->MouseMovement += DragDelta;

		if( !DragDelta.IsZero() )
		{
			// Convert the movement delta into drag/rotation deltas
			if ( bAllowMayaCam && GEditor->bUseMayaCameraControls )
			{
				FVector TempDrag;
				FRotator TempRot;
				InputAxisMayaCam( Viewport, DragDelta, TempDrag, TempRot );
			}
			else
			{
				FVector Drag;
				FRotator Rot;
				FVector Scale;
				MouseDeltaTracker->ConvertMovementDeltaToDragRot( this, DragDelta, Drag, Rot, Scale );
				MoveViewportCamera( Drag, Rot );
			}

			MouseDeltaTracker->ReduceBy( DragDelta );
		}
	}

	Viewport->Invalidate();

	return TRUE;
}

void FASVViewportClient::MouseMove(FViewport* Viewport,INT x, INT y)
{
	// If we are not currently moving the widget - update the ManipulateAxis to the one we are mousing over.
	if(!bManipulating)
	{
		INT	HitX = Viewport->GetMouseX();
		INT HitY = Viewport->GetMouseY();

		HHitProxy*	HitResult = Viewport->GetHitProxy(HitX,HitY);
		if(HitResult && HitResult->IsA(HWidgetUtilProxy::StaticGetType()))
		{
			HWidgetUtilProxy* WidgetProxy = (HWidgetUtilProxy*)HitResult;
			ManipulateAxis = WidgetProxy->Axis;
		}
		else 
		{
			ManipulateAxis = AXIS_None;
		}
	}
}


/*-----------------------------------------------------------------------------
FASVSkelSceneProxy
-----------------------------------------------------------------------------*/

/**
* A skeletal mesh component scene proxy.
*/
class FASVSkelSceneProxy : public FSkeletalMeshSceneProxy
{
public:
	/** 
	 * Constructor. 
	 * @param	Component - skeletal mesh primitive being added
	 */
	FASVSkelSceneProxy(const UASVSkelComponent* InComponent, const FColor& Color = FColor(230, 230, 255)) :
	FSkeletalMeshSceneProxy( InComponent, Color ),
	bHasBoneLine(FALSE)
	{
		SkeletalMeshComponent = const_cast<UASVSkelComponent*>(InComponent);
		WxAnimSetViewer* ASV = (WxAnimSetViewer*)SkeletalMeshComponent->AnimSetViewerPtr;
		const FASVViewportClient* ASVPreviewVC = ASV->GetPreviewVC();

		// If showing bones, then show root motion delta
		if( bDisplayBones  )
		{
			if( ASV->SelectedAnimSet && ASV->SelectedAnimSeq && ASV->PreviewAnimNode )
			{
				UAnimSet* AnimSet = ASV->SelectedAnimSet;
				FAnimSetMeshLinkup* AnimLinkup = &AnimSet->LinkupCache(ASV->PreviewAnimNode->AnimLinkupIndex);

				// Find which track in the sequence we look in for the Root Bone data
				const INT	TrackIndex	= AnimLinkup->BoneToTrackTable(0);
				FBoneAtom RootBoneAtom;

				// If there is no track for this bone, we just use the reference pose.
				if( TrackIndex == INDEX_NONE )
				{
					TArray<FMeshBone>& RefSkel = SkeletalMeshComponent->SkeletalMesh->RefSkeleton;
					RootBoneAtom = FBoneAtom(RefSkel(0).BonePos.Orientation, RefSkel(0).BonePos.Position, 1.f);					
				}
				else 
				{
					// get the exact translation of the root bone on the first frame of the animation
					ASV->SelectedAnimSeq->GetBoneAtom(RootBoneAtom, TrackIndex, 0.f, FALSE, SkeletalMeshComponent->bUseRawData);
				}

				// convert mesh space root loc to world space
				BoneRootMotionLine[0] = SkeletalMeshComponent->LocalToWorld.TransformFVector(RootBoneAtom.Translation);
				BoneRootMotionLine[1] = SkeletalMeshComponent->GetBoneMatrix(0).GetOrigin();

				bHasBoneLine = TRUE;
			}
		}

		bShowSoftBodyTetra = InComponent->bShowSoftBodyTetra;		
	}

	virtual ~FASVSkelSceneProxy() {}

	// FPrimitiveSceneProxy interface.

	/** 
	* Draw the scene proxy as a dynamic element
	*
	* @param	PDI - draw interface to render to
	* @param	View - current view
	* @param	DPGIndex - current depth priority 
	* @param	Flags - optional set of flags from EDrawDynamicElementFlags
	*/
	virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex,DWORD Flags)
	{
		if(SkeletalMeshComponent->bDrawMesh)
		{
			FSkeletalMeshSceneProxy::DrawDynamicElements(PDI, View, DPGIndex,Flags);
		}

		// Render a wireframe skeleton of the raw animation data.
		else if(this->bDisplayBones)
		{	
			INT LODIndex = GetCurrentLODIndex();
			const FStaticLODModel& LODModel = SkeletalMesh->LODModels(LODIndex);
			DebugDrawBones(PDI, View, SkeletalMeshComponent->SpaceBases, LODModel, BoneColor);
		}


		// Draw root motion delta for bones.
		if(bHasBoneLine && (this->bDisplayBones || SkeletalMeshComponent->bDrawMesh))
		{
			PDI->DrawLine(BoneRootMotionLine[0], BoneRootMotionLine[1], FColor(255, 0, 0), SDPG_Foreground);
		}

		if(bShowSoftBodyTetra)
		{
			DebugDrawSoftBodyTetras(PDI, View);
		}
	}

	/** Ensure its always in the foreground DPG. */
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View)
	{
		FPrimitiveViewRelevance Result = FSkeletalMeshSceneProxy::GetViewRelevance(View);
		Result.SetDPG(SDPG_Foreground,TRUE);
		return Result;
	}

	virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocOther ); }
	virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	DWORD GetAllocatedSize( void ) const { return( FSkeletalMeshSceneProxy::GetAllocatedSize() ); }

private:
	/** Holds onto the skeletal mesh component that created it so it can handle the rendering of bones properly. */
	UASVSkelComponent* SkeletalMeshComponent;
	BITFIELD bHasBoneLine : 1;
	FVector BoneRootMotionLine[2];
	BITFIELD bShowSoftBodyTetra : 1;
};

/*-----------------------------------------------------------------------------
	UASVSkelComponent
-----------------------------------------------------------------------------*/

/** Create the scene proxy needed for rendering a ASV skeletal mesh */
FPrimitiveSceneProxy* UASVSkelComponent::CreateSceneProxy()
{
	FASVSkelSceneProxy* Result = NULL;

	// only create a scene proxy for rendering if
	// properly initialized
	if( SkeletalMesh && 
		SkeletalMesh->LODModels.IsValidIndex(PredictedLODLevel) &&
		!bHideSkin &&
		MeshObject )
	{
		Result = ::new FASVSkelSceneProxy(this, BoneColor);
	}

	return Result;
}

/*-----------------------------------------------------------------------------
	WxASVPreview
-----------------------------------------------------------------------------*/

/**
 * A wxWindows container for FCascadePreviewViewportClient.
 */
class WxASVPreview : public wxWindow
{
public:
	FASVViewportClient* ASVPreviewVC;

	WxASVPreview(wxWindow* InParent, wxWindowID InID, class WxAnimSetViewer* InASV)
		:	wxWindow( InParent, InID )
		,	ASVPreviewVC( NULL )
	{
		CreateViewport( InASV );
	}

	~WxASVPreview()
	{
		DestroyViewport();
	}

	/**
	 * Calls DestoryViewport(), then creates a new viewport client and associated viewport.
	 */
	void CreateViewport(class WxAnimSetViewer* InASV)
	{
		DestroyViewport();

		ASVPreviewVC = new FASVViewportClient(InASV);
		ASVPreviewVC->Viewport = GEngine->Client->CreateWindowChildViewport(ASVPreviewVC, (HWND)GetHandle());
		check( ASVPreviewVC->Viewport );
		ASVPreviewVC->Viewport->CaptureJoystickInput(false);
	}

	/**
	 * Destroys any existing viewport client and associated viewport.
	 */
	void DestroyViewport()
	{
		if ( ASVPreviewVC )
		{
			GEngine->Client->CloseViewport( ASVPreviewVC->Viewport );
			ASVPreviewVC->Viewport = NULL;

			delete ASVPreviewVC;
			ASVPreviewVC = NULL;
		}
	}

private:
	void OnSize( wxSizeEvent& In )
	{
		if ( ASVPreviewVC )
		{
			checkSlow( ASVPreviewVC->Viewport );
			const wxRect rc = GetClientRect();
			::MoveWindow( (HWND)ASVPreviewVC->Viewport->GetWindow(), 0, 0, rc.GetWidth(), rc.GetHeight(), 1 );

			ASVPreviewVC->Viewport->Invalidate();
		}
	}

	DECLARE_EVENT_TABLE()
};


BEGIN_EVENT_TABLE( WxASVPreview, wxWindow )
	EVT_SIZE( WxASVPreview::OnSize )
END_EVENT_TABLE()

/*-----------------------------------------------------------------------------
	WxAnimSetViewer
-----------------------------------------------------------------------------*/


BEGIN_EVENT_TABLE( WxAnimSetViewer, WxTrackableFrame )
	EVT_SIZE(WxAnimSetViewer::OnSize)
	EVT_COMBOBOX( IDM_ANIMSET_SKELMESHCOMBO, WxAnimSetViewer::OnSkelMeshComboChanged )
	EVT_BUTTON( IDM_ANIMSET_SKELMESHUSE, WxAnimSetViewer::OnSkelMeshUse )
	EVT_COMBOBOX( IDM_ANIMSET_ANIMSETCOMBO, WxAnimSetViewer::OnAnimSetComboChanged )
	EVT_BUTTON( IDM_ANIMSET_ANIMSETUSE, WxAnimSetViewer::OnAnimSetUse )
	EVT_COMBOBOX( IDM_ANIMSET_SKELMESHAUX1COMBO, WxAnimSetViewer::OnAuxSkelMeshComboChanged )
	EVT_COMBOBOX( IDM_ANIMSET_SKELMESHAUX2COMBO, WxAnimSetViewer::OnAuxSkelMeshComboChanged )
	EVT_COMBOBOX( IDM_ANIMSET_SKELMESHAUX3COMBO, WxAnimSetViewer::OnAuxSkelMeshComboChanged )
	EVT_BUTTON( IDM_ANIMSET_SKELMESH_AUX1USE, WxAnimSetViewer::OnAuxSkelMeshUse )
	EVT_BUTTON( IDM_ANIMSET_SKELMESH_AUX2USE, WxAnimSetViewer::OnAuxSkelMeshUse )
	EVT_BUTTON( IDM_ANIMSET_SKELMESH_AUX3USE, WxAnimSetViewer::OnAuxSkelMeshUse )
	EVT_LISTBOX( IDM_ANIMSET_ANIMSEQLIST, WxAnimSetViewer::OnAnimSeqListChanged )
	EVT_MENU( IDM_ANIMSET_IMPORTPSA, WxAnimSetViewer::OnImportPSA )
	EVT_MENU( IDM_ANIMSET_IMPORTCOLLADAANIM, WxAnimSetViewer::OnImportCOLLADAAnim )
	EVT_MENU( IDM_ANIMSET_IMPORTMESHLOD, WxAnimSetViewer::OnImportMeshLOD )
	EVT_MENU( IDM_ANIMSET_IMPORTMESHWEIGHTS, WxAnimSetViewer::OnImportMeshWeights )	
	EVT_MENU( IDM_ANIMSET_TOGGLEMESHWEIGHTS, WxAnimSetViewer::OnToggleMeshWeights )	
	EVT_MENU( IDM_ANIMSET_NEWANISMET, WxAnimSetViewer::OnNewAnimSet )
	EVT_MENU( IDM_ANIMSET_NEWMORPHSET, WxAnimSetViewer::OnNewMorphTargetSet )
	EVT_MENU( IDM_ANIMSET_IMPORTMORPHTARGET, WxAnimSetViewer::OnImportMorphTarget )
	EVT_MENU( IDM_ANIMSET_IMPORTMORPHTARGET_LOD, WxAnimSetViewer::OnImportMorphTargetLOD )		
	EVT_COMBOBOX( IDM_ANIMSET_MORPHSETCOMBO, WxAnimSetViewer::OnMorphSetComboChanged )
	EVT_BUTTON( IDM_ANIMSET_MORPHSETUSE, WxAnimSetViewer::OnMorphSetUse )
	EVT_LISTBOX( IDM_ANIMSET_MORPHTARGETLIST, WxAnimSetViewer::OnMorphTargetListChanged )
	EVT_COMMAND_SCROLL( IDM_ANIMSET_TIMESCRUB, WxAnimSetViewer::OnTimeScrub )
	EVT_MENU( IDM_ANIMSET_VIEWBONES, WxAnimSetViewer::OnViewBones )
	EVT_MENU( IDM_ANIMSET_ShowRawAnimation, WxAnimSetViewer::OnShowRawAnimation )
	EVT_MENU( IDM_ANIMSET_VIEWBONENAMES, WxAnimSetViewer::OnViewBoneNames )
	EVT_MENU( IDM_ANIMSET_VIEWFLOOR, WxAnimSetViewer::OnViewFloor )
	EVT_MENU( IDM_ANIMSET_VIEWWIREFRAME, WxAnimSetViewer::OnViewWireframe )
	EVT_MENU( IDM_ANIMSET_VIEWGRID, WxAnimSetViewer::OnViewGrid )
	EVT_MENU( IDM_ANIMSET_VIEWSOCKETS, WxAnimSetViewer::OnViewSockets )
	EVT_MENU( IDM_ANIMSET_VIEWREFPOSE, WxAnimSetViewer::OnViewRefPose )
	EVT_MENU( IDM_ANIMSET_VIEWMIRROR, WxAnimSetViewer::OnViewMirror )
	EVT_MENU( IDM_ANIMSET_VIEWBOUNDS, WxAnimSetViewer::OnViewBounds )
	EVT_MENU( IDM_ANIMSET_VIEWCOLLISION, WxAnimSetViewer::OnViewCollision )
	EVT_MENU( IDM_ANIMSET_VIEWSOFTBODYTETRA, WxAnimSetViewer::OnViewSoftBodyTetra )
	EVT_BUTTON( IDM_ANIMSET_LOOPBUTTON, WxAnimSetViewer::OnLoopAnim )
	EVT_BUTTON( IDM_ANIMSET_PLAYBUTTON, WxAnimSetViewer::OnPlayAnim )
	EVT_MENU( IDM_ANIMSET_EMPTYSET, WxAnimSetViewer::OnEmptySet )

	EVT_MENU( IDM_ANIMSET_DELETETRACK, WxAnimSetViewer::OnDeleteTrack )
	EVT_MENU( IDM_ANIMSET_COPYTRANSLATIONBONENAMES, WxAnimSetViewer::OnCopyTranslationBoneNames )
	EVT_MENU( IDM_ANIMSET_RENAMESEQ, WxAnimSetViewer::OnRenameSequence )
	EVT_MENU( IDM_ANIMSET_DELETESEQ, WxAnimSetViewer::OnDeleteSequence )
	EVT_MENU( IDM_ANIMSET_COPYSEQ, WxAnimSetViewer::OnCopySequence )
	EVT_MENU( IDM_ANIMSET_MOVESEQ, WxAnimSetViewer::OnMoveSequence )
	EVT_MENU( IDM_ANIMSET_MAKESEQADDITIVE, WxAnimSetViewer::OnMakeSequencesAdditive )
	EVT_MENU( IDM_ANIMSET_SEQAPPLYROT, WxAnimSetViewer::OnSequenceApplyRotation )
	EVT_MENU( IDM_ANIMSET_SEQREZERO, WxAnimSetViewer::OnSequenceReZero )
	EVT_MENU( IDM_ANIMSET_SEQDELBEFORE, WxAnimSetViewer::OnSequenceCrop )
	EVT_MENU( IDM_ANIMSET_SEQDELAFTER, WxAnimSetViewer::OnSequenceCrop )
	EVT_MENU( IDM_ANIMSET_NOTIFYNEW, WxAnimSetViewer::OnNewNotify )
	EVT_MENU( IDM_ANIMSET_NOTIFYSORT, WxAnimSetViewer::OnNotifySort )
	EVT_MENU( IDM_ANIMSET_COPYSEQNAME, WxAnimSetViewer::OnCopySequenceName )
	EVT_MENU( IDM_ANIMSET_COPYSEQNAMELIST, WxAnimSetViewer::OnCopySequenceNameList )
	EVT_MENU( IDM_ANIMSET_UPDATEBOUNDS, WxAnimSetViewer::OnUpdateBounds )	
	EVT_MENU( IDM_ANIMSET_COPYMESHBONES, WxAnimSetViewer::OnCopyMeshBoneNames )	
	EVT_MENU( IDM_ANIMSET_FIXUPMESHBONES, WxAnimSetViewer::OnFixupMeshBoneNames )	
	EVT_MENU( IDM_ANIMSET_SWAPLODSECTIONS, WxAnimSetViewer::OnSwapLODSections )	
	EVT_MENU( IDM_ANIMSET_MERGEMATERIALS, WxAnimSetViewer::OnMergeMaterials )	
	EVT_MENU( IDM_ANIMSET_AUTOMIRRORTABLE, WxAnimSetViewer::OnAutoMirrorTable )	
	EVT_MENU( IDM_ANIMSET_CHECKMIRRORTABLE, WxAnimSetViewer::OnCheckMirrorTable )	
	EVT_MENU( IDM_ANIMSET_COPYMIRRORTABLE, WxAnimSetViewer::OnCopyMirrorTable )	
	EVT_MENU( IDM_ANIMSET_COPYMIRRORTABLEFROM, WxAnimSetViewer::OnCopyMirroTableFromMesh )
	EVT_MENU( IDM_ANIMSET_NOTIFYCOPY, WxAnimSetViewer::OnNotifyCopy )
	EVT_TOOL( IDM_ANIMSET_OPENSOCKETMGR, WxAnimSetViewer::OnOpenSocketMgr )
	EVT_TOOL( IDM_ANIMSET_OpenAnimationCompressionDlg, WxAnimSetViewer::OnOpenAnimationCompressionDlg )
	EVT_BUTTON( IDM_ANIMSET_NEWSOCKET, WxAnimSetViewer::OnNewSocket )
	EVT_BUTTON( IDM_ANIMSET_DELSOCKET, WxAnimSetViewer::OnDeleteSocket )
	EVT_LISTBOX( IDM_ANIMSET_SOCKETLIST, WxAnimSetViewer::OnClickSocket )
	EVT_TOOL( IDM_ANIMSET_SOCKET_TRANSLATE, WxAnimSetViewer::OnSocketMoveMode )
	EVT_TOOL( IDM_ANIMSET_SOCKET_ROTATE, WxAnimSetViewer::OnSocketMoveMode )
	EVT_TOOL( IDM_ANIMSET_CLEARPREVIEWS, WxAnimSetViewer::OnClearPreviewMeshes )
	EVT_TOOL( IDM_ANIMSET_COPYSOCKETS, WxAnimSetViewer::OnCopySockets )
	EVT_TOOL( IDM_ANIMSET_PASTESOCKETS, WxAnimSetViewer::OnPasteSockets )
	EVT_MENU( IDM_ANIMSET_LOD_AUTO, WxAnimSetViewer::OnForceLODLevel )
	EVT_MENU( IDM_ANIMSET_LOD_BASE, WxAnimSetViewer::OnForceLODLevel )
	EVT_MENU( IDM_ANIMSET_LOD_1, WxAnimSetViewer::OnForceLODLevel )
	EVT_MENU( IDM_ANIMSET_LOD_2, WxAnimSetViewer::OnForceLODLevel )
	EVT_MENU( IDM_ANIMSET_LOD_3, WxAnimSetViewer::OnForceLODLevel )
	EVT_MENU( IDM_ANIMSET_REMOVELOD,  WxAnimSetViewer::OnRemoveLOD )
	EVT_TOOL( IDM_ANIMSET_SPEED_100,	WxAnimSetViewer::OnSpeed )
	EVT_TOOL( IDM_ANIMSET_SPEED_50, WxAnimSetViewer::OnSpeed )
	EVT_TOOL( IDM_ANIMSET_SPEED_25, WxAnimSetViewer::OnSpeed )
	EVT_TOOL( IDM_ANIMSET_SPEED_10, WxAnimSetViewer::OnSpeed )
	EVT_TOOL( IDM_ANIMSET_SPEED_1, WxAnimSetViewer::OnSpeed )
	EVT_MENU( IDM_ANIMSET_RENAME_MORPH, WxAnimSetViewer::OnRenameMorphTarget )
	EVT_MENU( IDM_ANIMSET_DELETE_MORPH, WxAnimSetViewer::OnDeleteMorphTarget )
	EVT_TOOL( IDM_ANIMSET_TOGGLECLOTH, WxAnimSetViewer::OnToggleClothSim )
	EVT_TOOL( IDM_ANIMSET_SOFTBODYGENERATE, WxAnimSetViewer::OnSoftBodyGenerate )
	EVT_TOOL( IDM_ANIMSET_SOFTBODYTOGGLESIM, WxAnimSetViewer::OnSoftBodyToggleSim )


	EVT_TREE_ITEM_RIGHT_CLICK( IDM_ANIMSET_SKELETONTREECTRL, WxAnimSetViewer::OnSkeletonTreeItemRightClick )
	EVT_MENU( IDM_ANIMSET_SKELETONTREE_SHOWBONE, WxAnimSetViewer::OnSkeletonTreeMenuHandleCommand)
	EVT_MENU( IDM_ANIMSET_SKELETONTREE_HIDEBONE, WxAnimSetViewer::OnSkeletonTreeMenuHandleCommand)
	EVT_MENU( IDM_ANIMSET_SKELETONTREE_SETBONECOLOR, WxAnimSetViewer::OnSkeletonTreeMenuHandleCommand)
	EVT_MENU( IDM_ANIMSET_SKELETONTREE_SHOWCHILDBONE, WxAnimSetViewer::OnSkeletonTreeMenuHandleCommand)
	EVT_MENU( IDM_ANIMSET_SKELETONTREE_HIDECHILDBONE, WxAnimSetViewer::OnSkeletonTreeMenuHandleCommand)
	EVT_MENU( IDM_ANIMSET_SKELETONTREE_SETCHILDBONECOLOR, WxAnimSetViewer::OnSkeletonTreeMenuHandleCommand)
	EVT_MENU( IDM_ANIMSET_SKELETONTREE_COPYBONENAME, WxAnimSetViewer::OnSkeletonTreeMenuHandleCommand)

END_EVENT_TABLE()


static INT AnimSetViewer_ControlBorder = 4;

WxAnimSetViewer::WxAnimSetViewer(wxWindow* InParent, wxWindowID InID, USkeletalMesh* InSkelMesh, UAnimSet* InAnimSet, UMorphTargetSet* InMorphSet)
	:	WxTrackableFrame( InParent, InID, *LocalizeUnrealEd("AnimSetViewer"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR )
	,	FDockingParent( this )
	,	PlaybackSpeed( 1.0f )
	,	DlgAnimationCompression( NULL )
	,	bPreviewInstanceWeights(FALSE)
{
	wxEvtHandler::Connect(wxID_ANY, IDM_ANIMSET_FILLSKELETONTREE, wxCommandEventHandler(WxAnimSetViewer::OnFillSkeletonTree));

	SelectedSkelMesh = NULL;
	SelectedAnimSet = NULL;
	SelectedAnimSeq = NULL;
	SelectedMorphSet = NULL;
	SelectedMorphTarget = NULL;
	SelectedSocket = NULL;

	SocketMgr = NULL;

	/////////////////////////////////
	// Create all the GUI stuff.

	PlayB.Load( TEXT("ASV_Play") );
	StopB.Load( TEXT("ASV_Stop") );
	LoopB.Load( TEXT("ASV_Loop") );
	NoLoopB.Load( TEXT("ASV_NoLoop") );
	UseButtonB.Load( TEXT("Prop_Use") );

	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));

	//////// LEFT PANEL ////////
	wxPanel* LeftPanel = new wxPanel( this, -1 );
	wxBoxSizer* LeftSizerV = new wxBoxSizer( wxVERTICAL );
	LeftPanel->SetSizer(LeftSizerV);
	{
		ResourceNotebook = new wxNotebook( LeftPanel, -1, wxDefaultPosition, wxSize(350, -1) );
		ResourceNotebook->SetPageSize(wxSize(350, -1));
		LeftSizerV->Add( ResourceNotebook, 1, wxGROW|wxALL, AnimSetViewer_ControlBorder );

		// SkeletalMesh
		{
			wxPanel* SkeletalMeshPanel = new wxPanel( ResourceNotebook, -1 );
			ResourceNotebook->AddPage( SkeletalMeshPanel, *LocalizeUnrealEd("SkelMesh"), true );

			wxBoxSizer* SkeletalMeshPanelSizer = new wxBoxSizer(wxVERTICAL);
			SkeletalMeshPanel->SetSizer(SkeletalMeshPanelSizer);
			SkeletalMeshPanel->SetAutoLayout(true);

			wxStaticText* SkelMeshLabel = new wxStaticText( SkeletalMeshPanel, -1, *LocalizeUnrealEd("SkeletalMesh") );
			SkeletalMeshPanelSizer->Add( SkelMeshLabel, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, AnimSetViewer_ControlBorder );

			wxBoxSizer* SkelMeshSizer = new wxBoxSizer( wxHORIZONTAL );
			SkeletalMeshPanelSizer->Add( SkelMeshSizer, 0, wxGROW|wxALL, 0 );

			SkelMeshCombo = new WxComboBox( SkeletalMeshPanel, IDM_ANIMSET_SKELMESHCOMBO, TEXT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY | wxCB_SORT );
			SkelMeshSizer->Add( SkelMeshCombo, 1, wxGROW|wxALL, AnimSetViewer_ControlBorder );

			wxBitmapButton* SkelMeshUseButton = new wxBitmapButton( SkeletalMeshPanel, IDM_ANIMSET_SKELMESHUSE, UseButtonB );
			SkelMeshUseButton->SetToolTip( *LocalizeUnrealEd("ToolTip_1" ) );
			SkelMeshSizer->Add( SkelMeshUseButton, 0, wxGROW|wxALL, AnimSetViewer_ControlBorder );


			// Additional skeletal meshes
			{
				wxStaticText* Aux1SkelMeshLabel = new wxStaticText( SkeletalMeshPanel, -1, *LocalizeUnrealEd("ExtraMesh1") );
				SkeletalMeshPanelSizer->Add( Aux1SkelMeshLabel, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, AnimSetViewer_ControlBorder );

				wxBoxSizer* Aux1Sizer = new wxBoxSizer( wxHORIZONTAL );
				SkeletalMeshPanelSizer->Add( Aux1Sizer, 0, wxGROW|wxALL, 0 );

				SkelMeshAux1Combo = new WxComboBox( SkeletalMeshPanel, IDM_ANIMSET_SKELMESHAUX1COMBO, TEXT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY | wxCB_SORT );
				Aux1Sizer->Add( SkelMeshAux1Combo, 1, wxGROW|wxALL, AnimSetViewer_ControlBorder );

				wxBitmapButton* Aux1UseButton = new wxBitmapButton( SkeletalMeshPanel, IDM_ANIMSET_SKELMESH_AUX1USE, UseButtonB );
				Aux1UseButton->SetToolTip( *LocalizeUnrealEd("ToolTip_2") );
				Aux1Sizer->Add( Aux1UseButton, 0, wxGROW|wxALL, AnimSetViewer_ControlBorder );
			}

			{
				wxStaticText* Aux2SkelMeshLabel = new wxStaticText( SkeletalMeshPanel, -1, *LocalizeUnrealEd("ExtraMesh2") );
				SkeletalMeshPanelSizer->Add( Aux2SkelMeshLabel, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, AnimSetViewer_ControlBorder );

				wxBoxSizer* Aux2Sizer = new wxBoxSizer( wxHORIZONTAL );
				SkeletalMeshPanelSizer->Add( Aux2Sizer, 0, wxGROW|wxALL, 0 );

				SkelMeshAux2Combo = new WxComboBox( SkeletalMeshPanel, IDM_ANIMSET_SKELMESHAUX2COMBO, TEXT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY | wxCB_SORT );
				Aux2Sizer->Add( SkelMeshAux2Combo, 1, wxGROW|wxALL, AnimSetViewer_ControlBorder );

				wxBitmapButton* Aux2UseButton = new wxBitmapButton( SkeletalMeshPanel, IDM_ANIMSET_SKELMESH_AUX2USE, UseButtonB );
				Aux2UseButton->SetToolTip( *LocalizeUnrealEd("ToolTip_2") );
				Aux2Sizer->Add( Aux2UseButton, 0, wxGROW|wxALL, AnimSetViewer_ControlBorder );
			}

			{
				wxStaticText* Aux3SkelMeshLabel = new wxStaticText( SkeletalMeshPanel, -1, *LocalizeUnrealEd("ExtraMesh3") );
				SkeletalMeshPanelSizer->Add( Aux3SkelMeshLabel, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, AnimSetViewer_ControlBorder );

				wxBoxSizer* Aux3Sizer = new wxBoxSizer( wxHORIZONTAL );
				SkeletalMeshPanelSizer->Add( Aux3Sizer, 0, wxGROW|wxALL, 0 );

				SkelMeshAux3Combo = new WxComboBox( SkeletalMeshPanel, IDM_ANIMSET_SKELMESHAUX3COMBO, TEXT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY | wxCB_SORT );
				Aux3Sizer->Add( SkelMeshAux3Combo, 1, wxGROW|wxALL, AnimSetViewer_ControlBorder );

				wxBitmapButton* Aux3UseButton = new wxBitmapButton( SkeletalMeshPanel, IDM_ANIMSET_SKELMESH_AUX3USE, UseButtonB );
				Aux3UseButton->SetToolTip( *LocalizeUnrealEd("ToolTip_2") );
				Aux3Sizer->Add( Aux3UseButton, 0, wxGROW|wxALL, AnimSetViewer_ControlBorder );
			}
		}

		// AnimSet
		{
			wxPanel* AnimPanel = new wxPanel( ResourceNotebook, -1 );
			ResourceNotebook->AddPage( AnimPanel, *LocalizeUnrealEd("Anim"), true );

			wxBoxSizer* AnimPanelSizer = new wxBoxSizer(wxVERTICAL);
			AnimPanel->SetSizer(AnimPanelSizer);
			AnimPanel->SetAutoLayout(true);

			wxStaticText* AnimSetLabel = new wxStaticText( AnimPanel, -1, *LocalizeUnrealEd("AnimSet") );
			AnimPanelSizer->Add( AnimSetLabel, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, AnimSetViewer_ControlBorder );

			wxBoxSizer* AnimSetSizer = new wxBoxSizer( wxHORIZONTAL );
			AnimPanelSizer->Add( AnimSetSizer, 0, wxGROW|wxALL, 0 );

			AnimSetCombo = new WxComboBox( AnimPanel, IDM_ANIMSET_ANIMSETCOMBO, TEXT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY | wxCB_SORT );
			AnimSetSizer->Add( AnimSetCombo, 1, wxGROW|wxALL, AnimSetViewer_ControlBorder );

			wxBitmapButton* AnimSetUseButton = new wxBitmapButton( AnimPanel, IDM_ANIMSET_ANIMSETUSE, UseButtonB );
			AnimSetUseButton->SetToolTip( *LocalizeUnrealEd("ToolTip_1") );
			AnimSetSizer->Add( AnimSetUseButton, 0, wxGROW|wxALL, AnimSetViewer_ControlBorder );


			// AnimSequence list
			wxStaticText* AnimSeqLabel = new wxStaticText( AnimPanel, -1, *LocalizeUnrealEd("AnimationSequences") );
			AnimPanelSizer->Add( AnimSeqLabel, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, AnimSetViewer_ControlBorder );

			AnimSeqList = new wxListBox( AnimPanel, IDM_ANIMSET_ANIMSEQLIST, wxDefaultPosition, wxSize(250, -1), 0, NULL, wxLB_EXTENDED | wxLB_SORT);
			AnimPanelSizer->Add( AnimSeqList, 1, wxGROW|wxALL, AnimSetViewer_ControlBorder  );
		}


		// Morph target
		{
			wxPanel* MorphPanel = new wxPanel( ResourceNotebook, -1 );
			ResourceNotebook->AddPage( MorphPanel, *LocalizeUnrealEd("Morph"), true );

			wxBoxSizer* MorphPanelSizer = new wxBoxSizer(wxVERTICAL);
			MorphPanel->SetSizer(MorphPanelSizer);
			MorphPanel->SetAutoLayout(true);

			wxStaticText* MorphSetLabel = new wxStaticText( MorphPanel, -1, *LocalizeUnrealEd("MorphTargetSet") );
			MorphPanelSizer->Add( MorphSetLabel, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, AnimSetViewer_ControlBorder );

			wxBoxSizer* MorphSetSizer = new wxBoxSizer( wxHORIZONTAL );
			MorphPanelSizer->Add( MorphSetSizer, 0, wxGROW|wxALL, 0 );

			MorphSetCombo = new WxComboBox( MorphPanel, IDM_ANIMSET_MORPHSETCOMBO, TEXT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY | wxCB_SORT );
			MorphSetSizer->Add( MorphSetCombo, 1, wxGROW|wxALL, AnimSetViewer_ControlBorder );

			wxBitmapButton* MorphSetUseButton = new wxBitmapButton( MorphPanel, IDM_ANIMSET_MORPHSETUSE, UseButtonB );
			MorphSetUseButton->SetToolTip( *LocalizeUnrealEd("ToolTip_1") );
			MorphSetSizer->Add( MorphSetUseButton, 0, wxGROW|wxALL, AnimSetViewer_ControlBorder );


			// MorphTarget list
			wxStaticText* MorphTargetLabel = new wxStaticText( MorphPanel, -1, *LocalizeUnrealEd("MorphTargets") );
			MorphPanelSizer->Add( MorphTargetLabel, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, AnimSetViewer_ControlBorder );

			MorphTargetList = new wxListBox( MorphPanel, IDM_ANIMSET_MORPHTARGETLIST, wxDefaultPosition, wxSize(250, -1), 0, NULL, wxLB_SINGLE );
			MorphPanelSizer->Add( MorphTargetList, 1, wxGROW|wxALL, AnimSetViewer_ControlBorder  );
		}
	}

	//////// MIDDLE PANEL ////////
	wxPanel* MidPanel = new wxPanel( this, -1 );
	wxBoxSizer* MidSizerV = new wxBoxSizer( wxVERTICAL );
	MidPanel->SetSizer(MidSizerV);
	{
		// 3D Preview
		PreviewWindow = new WxASVPreview( MidPanel, -1, this );
		MidSizerV->Add( PreviewWindow, 1, wxGROW|wxALL, AnimSetViewer_ControlBorder );

		// Sizer for controls at bottom
		wxBoxSizer* ControlSizerH = new wxBoxSizer( wxHORIZONTAL );
		MidSizerV->Add( ControlSizerH, 0, wxGROW|wxALL, AnimSetViewer_ControlBorder );

		// Stop/Play/Loop buttons
		LoopButton = new wxBitmapButton( MidPanel, IDM_ANIMSET_LOOPBUTTON, NoLoopB, wxDefaultPosition, wxDefaultSize, 0 );
		ControlSizerH->Add( LoopButton, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

		PlayButton = new wxBitmapButton( MidPanel, IDM_ANIMSET_PLAYBUTTON, PlayB, wxDefaultPosition, wxDefaultSize, 0 );
		ControlSizerH->Add( PlayButton, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

		// Time scrubber
		TimeSlider = new wxSlider( MidPanel, IDM_ANIMSET_TIMESCRUB, 0, 0, ASV_SCRUBRANGE );
		ControlSizerH->Add( TimeSlider, 1, wxGROW|wxALL, 0 );
	}

	//////// RIGHT PANEL ////////
	wxPanel* RightPanel = new wxPanel( this, -1 );
	wxBoxSizer* RightSizerV = new wxBoxSizer( wxVERTICAL );
	RightPanel->SetSizer(RightSizerV);
	{
		PropNotebook = new wxNotebook( RightPanel, -1, wxDefaultPosition, wxSize(350, -1) );
		PropNotebook->SetPageSize(wxSize(350, -1));
		RightSizerV->Add( PropNotebook, 1, wxGROW|wxALL, AnimSetViewer_ControlBorder );

		// Mesh properties
		wxPanel* MeshPropsPanel = new wxPanel( PropNotebook, -1 );
		PropNotebook->AddPage( MeshPropsPanel, *LocalizeUnrealEd("Mesh"), true );

		wxBoxSizer* MeshPropsPanelSizer = new wxBoxSizer(wxVERTICAL);
		MeshPropsPanel->SetSizer(MeshPropsPanelSizer);
		MeshPropsPanel->SetAutoLayout(true);

		MeshProps = new WxPropertyWindow;
		MeshProps->Create( MeshPropsPanel, this );
		MeshPropsPanelSizer->Add( MeshProps, 1, wxGROW|wxALL, 0 );


		// AnimSet properties
		wxPanel* AnimSetPropsPanel = new wxPanel( PropNotebook, -1 );
		PropNotebook->AddPage( AnimSetPropsPanel, *LocalizeUnrealEd("AnimSet"), true );

		wxBoxSizer* AnimSetPropsPanelSizer = new wxBoxSizer(wxVERTICAL);
		AnimSetPropsPanel->SetSizer(AnimSetPropsPanelSizer);
		AnimSetPropsPanel->SetAutoLayout(true);

		AnimSetProps = new WxPropertyWindow;
		AnimSetProps->Create( AnimSetPropsPanel, this );
		AnimSetPropsPanelSizer->Add( AnimSetProps, 1, wxGROW|wxALL, 0 );


		// AnimSequence properties
		wxPanel* AnimSeqPropsPanel = new wxPanel( PropNotebook, -1 );
		PropNotebook->AddPage( AnimSeqPropsPanel, *LocalizeUnrealEd("AnimSequence"), true );

		wxBoxSizer* AnimSeqPropsPanelSizer = new wxBoxSizer(wxVERTICAL);
		AnimSeqPropsPanel->SetSizer(AnimSeqPropsPanelSizer);
		AnimSeqPropsPanel->SetAutoLayout(true);

		AnimSeqProps = new WxPropertyWindow;
		AnimSeqProps->Create( AnimSeqPropsPanel, this );
		AnimSeqPropsPanelSizer->Add( AnimSeqProps, 1, wxGROW|wxALL, 0 );

		//wxPanel* TempPanel = new wxPanel( RightPanel, -1, wxDefaultPosition, wxSize(350, -1) );
		//RightSizerV->Add(TempPanel, 0, wxGROW, 0 );
	}

	// Create tree control
	SkeletonTreeCtrl = new WxTreeCtrl;
	SkeletonTreeCtrl->Create( this, IDM_ANIMSET_SKELETONTREECTRL, NULL, wxTR_HAS_BUTTONS|wxTR_MULTIPLE|wxTR_LINES_AT_ROOT );

	SetSize(1200,800);
	// Load Window Options.
	LoadSettings();

	// Add docking windows.
	{
		AddDockingWindow( LeftPanel, FDockingParent::DH_Left, *FString::Printf(LocalizeSecure(LocalizeUnrealEd("BrowserCaption_F"), *InAnimSet->GetPathName())), *LocalizeUnrealEd("Preview") );
		AddDockingWindow( RightPanel, FDockingParent::DH_Left, *FString::Printf(LocalizeSecure(LocalizeUnrealEd("PropertiesCaption_F"), *InAnimSet->GetPathName())), *LocalizeUnrealEd("Properties") );
		AddDockingWindow( SkeletonTreeCtrl, FDockingParent::DH_Left, *FString::Printf(LocalizeSecure(LocalizeUnrealEd("SkeletonTreeCaption_F"), *InAnimSet->GetPathName())), *LocalizeUnrealEd(TEXT("SkeletonTree")) );
		AddDockingWindow( MidPanel, FDockingParent::DH_None, NULL );

		// Try to load a existing layout for the docking windows.
		LoadDockingLayout();
	}

	// Create menu bar
	MenuBar = new WxASVMenuBar();
	AppendWindowMenu(MenuBar);
	SetMenuBar( MenuBar );

	// Create tool bar
	ToolBar = new WxASVToolBar( this, -1 );
	SetToolBar( ToolBar );

	// Create status bar
	StatusBar = new WxASVStatusBar( this, -1 );
	SetStatusBar( StatusBar );

	// Set camera location
	// TODO: Set the automatically based on bounds? Update when changing mesh? Store per-mesh?
	FASVViewportClient* ASVPreviewVC = GetPreviewVC();
	check( ASVPreviewVC );

	ASVPreviewVC->ViewLocation = FVector(0,-256,0);
	ASVPreviewVC->ViewRotation = FRotator(0,16384,0);	

	// Init wind dir.
	WindStrength = 10.f;
	WindRot = FRotator(0,0,0);
	UpdateClothWind();


	// Use default WorldInfo to define the gravity and stepping params.
	AWorldInfo* Info = (AWorldInfo*)(AWorldInfo::StaticClass()->GetDefaultObject());
	check(Info);
	FVector Gravity(0, 0, Info->DefaultGravityZ);
	RBPhysScene = CreateRBPhysScene( Gravity );
	
	// Fill in skeletal mesh combos box with all loaded skeletal meshes.
	SkelMeshCombo->Freeze();
	SkelMeshAux1Combo->Freeze();
	SkelMeshAux2Combo->Freeze();
	SkelMeshAux3Combo->Freeze();

	SkelMeshAux1Combo->Append( *LocalizeUnrealEd("-None-"), (void*)NULL );
	SkelMeshAux2Combo->Append( *LocalizeUnrealEd("-None-"), (void*)NULL );
	SkelMeshAux3Combo->Append( *LocalizeUnrealEd("-None-"), (void*)NULL );

	for(TObjectIterator<USkeletalMesh> It; It; ++It)
	{
		USkeletalMesh* ItSkelMesh = *It;
		SkelMeshCombo->Append( *ItSkelMesh->GetName(), ItSkelMesh );
		SkelMeshAux1Combo->Append( *ItSkelMesh->GetName(), ItSkelMesh );
		SkelMeshAux2Combo->Append( *ItSkelMesh->GetName(), ItSkelMesh );
		SkelMeshAux3Combo->Append( *ItSkelMesh->GetName(), ItSkelMesh );
	}

	SkelMeshCombo->Thaw();
	SkelMeshAux1Combo->Thaw();
	SkelMeshAux2Combo->Thaw();
	SkelMeshAux3Combo->Thaw();

	// Fill in MorphTargetSet combo.
	UpdateMorphSetCombo();

	// Select the skeletal mesh we were passed in.
	if(InSkelMesh)
	{
		SetSelectedSkelMesh(InSkelMesh, false);
	}
	// If none passed in, pick first in combo.
	//@todo: Should probably iterate to find mesh that is compatible.
	else
	{
		if( SkelMeshCombo->GetCount() > 0 )
		{
			USkeletalMesh* DefaultMesh = (USkeletalMesh*)SkelMeshCombo->GetClientData(0);
			SetSelectedSkelMesh(DefaultMesh, false);
		}
	}

	// If an animation set was passed in, try and select it. If not, select first one.
	if(InAnimSet)
	{
		SetSelectedAnimSet(InAnimSet, true);

		// If AnimSet supplied, set to Anim page.
		ResourceNotebook->SetSelection(1);
	}
	else
	{
		if( AnimSetCombo->GetCount() > 0)
		{
			UAnimSet* DefaultAnimSet = (UAnimSet*)AnimSetCombo->GetClientData(0);
			SetSelectedAnimSet(DefaultAnimSet, true);
		}

		// If no AnimSet, set to Mesh page.
		ResourceNotebook->SetSelection(0);
	}

	// If a morphtargetset was passed in, try and select it.
	if(InMorphSet)
	{
		SetSelectedMorphSet(InMorphSet, true);

		// If AnimSet supplied, set to Anim page.
		ResourceNotebook->SetSelection(2);
	}
	else
	{
		if( MorphSetCombo->GetCount() > 0)
		{
			UMorphTargetSet* DefaultMorphSet = (UMorphTargetSet*)MorphSetCombo->GetClientData(0);
			SetSelectedMorphSet(DefaultMorphSet, true);
		}

		// If no morph set, set to Mesh page.
		ResourceNotebook->SetSelection(0);
	}

	// Fill Skeleton Tree.
	FillSkeletonTree();
}

WxAnimSetViewer::~WxAnimSetViewer()
{
	SaveSettings();

	DestroyRBPhysScene(RBPhysScene);

	PreviewWindow->DestroyViewport();
}

/**
 * This function is called when the window has been selected from within the ctrl + tab dialog.
 */
void WxAnimSetViewer::OnSelected()
{
	Raise();
}

/**
* Called once to load AnimSetViewer settings, including window position.
*/

void WxAnimSetViewer::LoadSettings()
{
	// Load Window Position.
	FWindowUtil::LoadPosSize(TEXT("AnimSetViewer"), this, 256,256,1200,800);
}

/**
* Writes out values we want to save to the INI file.
*/

void WxAnimSetViewer::SaveSettings()
{
	SaveDockingLayout();

	// Save Window Position.
	FWindowUtil::SavePosSize(TEXT("AnimSetViewer"), this);
}



/**
 * Returns a handle to the anim set viewer's preview viewport client.
 */
FASVViewportClient* WxAnimSetViewer::GetPreviewVC()
{
	checkSlow( PreviewWindow );
	return PreviewWindow->ASVPreviewVC;
}

/**
 * Serializes the preview scene so it isn't garbage collected
 *
 * @param Ar The archive to serialize with
 */
void WxAnimSetViewer::Serialize(FArchive& Ar)
{
	FASVViewportClient* ASVPreviewVC = GetPreviewVC();
	check( ASVPreviewVC );

	ASVPreviewVC->Serialize(Ar);
	Ar << SocketPreviews;
}

///////////////////////////////////////////////////////////////////////////////////////
// Properties window NotifyHook stuff

void WxAnimSetViewer::NotifyDestroy( void* Src )
{

}

void WxAnimSetViewer::NotifyPreChange( void* Src, UProperty* PropertyAboutToChange )
{
	if( PropertyAboutToChange )
	{
		// If it is the FaceFXAsset, NULL out the DefaultSkelMesh.
		if( PropertyAboutToChange->GetFName() == FName(TEXT("FaceFXAsset")) )
		{
			if( SelectedSkelMesh && SelectedSkelMesh->FaceFXAsset )
			{
				SelectedSkelMesh->FaceFXAsset->DefaultSkelMesh = NULL;
#if WITH_FACEFX
				OC3Ent::Face::FxActor* Actor = SelectedSkelMesh->FaceFXAsset->GetFxActor();
				if( Actor )
				{
					Actor->SetShouldClientRelink(FxTrue);
				}
#endif
			}
		}
	}
}

void WxAnimSetViewer::NotifyPostChange( void* Src, UProperty* PropertyThatChanged )
{
	if( PropertyThatChanged )
	{
		// If it is the FaceFXAsset, NULL out the DefaultSkelMesh.
		if( PropertyThatChanged->GetFName() == FName(TEXT("FaceFXAsset")) )
		{
			if( SelectedSkelMesh && SelectedSkelMesh->FaceFXAsset )
			{
				SelectedSkelMesh->FaceFXAsset->DefaultSkelMesh = SelectedSkelMesh;
#if WITH_FACEFX
				OC3Ent::Face::FxActor* Actor = SelectedSkelMesh->FaceFXAsset->GetFxActor();
				if( Actor )
				{
					Actor->SetShouldClientRelink(FxTrue);
				}
#endif
				SelectedSkelMesh->FaceFXAsset->MarkPackageDirty();
			}
		}
		else if(PropertyThatChanged->GetFName() == FName(TEXT("bForceCPUSkinning")))
		{
			FComponentReattachContext ReattachContext(PreviewSkelComp);
			FComponentReattachContext ReattachContextRaw(PreviewSkelCompRaw);
			FComponentReattachContext ReattachContext1(PreviewSkelCompAux1);
			FComponentReattachContext ReattachContext2(PreviewSkelCompAux2);
			FComponentReattachContext ReattachContext3(PreviewSkelCompAux3);
		}
	}

	// Might have changed UseTranslationBoneNames array. We have to fix any existing FAnimSetMeshLinkup objects in selected AnimSet.
	// Would be nice to not do this all the time, but PropertyThatChanged seems flaky when editing arrays (sometimes NULL)
	if(SelectedAnimSet)
	{
		INT LinkupIndex = 0;
		while(LinkupIndex < SelectedAnimSet->LinkupCache.Num())
		{
			FAnimSetMeshLinkup& Linkup = SelectedAnimSet->LinkupCache(LinkupIndex);

			// Find the SkeletalMesh with this Linkup's GUID.
			USkeletalMesh* SkelMesh = NULL;
			for(TObjectIterator<USkeletalMesh> It;It;++It)
			{
				USkeletalMesh* TestMesh = *It;
				if(TestMesh->SkelMeshGUID == Linkup.SkelMeshLinkupGUID)
				{
					SkelMesh = TestMesh;
					continue;
				}
			}

			// If we found the mesh, update the linkup
			if(SkelMesh)
			{
				Linkup.BuildLinkup(SkelMesh, SelectedAnimSet);
				LinkupIndex++;
			}
			// If we didn't, remove it
			else
			{
				SelectedAnimSet->LinkupCache.Remove(LinkupIndex);
			}
		}
	}

	// Might have changed ClothBones array - so need to rebuild cloth data.
	// Would be nice to not do this all the time, but PropertyThatChanged seems flaky when editing arrays (sometimes NULL)
	{
		PreviewSkelComp->TermClothSim(NULL);
		check(SelectedSkelMesh);
		SelectedSkelMesh->ClearClothMeshCache();
		SelectedSkelMesh->BuildClothMapping();
		if(PreviewSkelComp->bEnableClothSimulation)
		{
			PreviewSkelComp->InitClothSim(RBPhysScene);
		}
		// Push any changes to cloth param to the running cloth sim.
		PreviewSkelComp->UpdateClothParams();

		PreviewSkelComp->TermSoftBodySim(NULL);
		if(PreviewSkelComp->bEnableSoftBodySimulation)
		{
			PreviewSkelComp->InitSoftBodySim(RBPhysScene, TRUE);
		}
		PreviewSkelComp->UpdateSoftBodyParams();
	}


	// In case we change Origin/RotOrigin, update the components.
	UpdateSkelComponents();
}

void WxAnimSetViewer::NotifyExec( void* Src, const TCHAR* Cmd )
{
	GUnrealEd->NotifyExec(Src, Cmd);
}

/** Toggle the 'Socket Manager' window. */
void WxAnimSetViewer::OnOpenSocketMgr(wxCommandEvent &In)
{
	if(!SocketMgr)
	{
		SocketMgr = new WxSocketManager( this, -1 );
		SocketMgr->Show();

		ToolBar->ToggleTool(IDM_ANIMSET_OPENSOCKETMGR, true);

		UpdateSocketList();
		SetSelectedSocket(NULL);

		FASVViewportClient* ASVPreviewVC = GetPreviewVC();
		check( ASVPreviewVC );
		ASVPreviewVC->Viewport->Invalidate();
	}
	else
	{
		SocketMgr->Close();

		ToolBar->ToggleTool(IDM_ANIMSET_OPENSOCKETMGR, false);
	}
}

/** Toggles the modal animation Animation compression dialog. */
void WxAnimSetViewer::OnOpenAnimationCompressionDlg(wxCommandEvent &In)
{
	if ( !DlgAnimationCompression )
	{
		DlgAnimationCompression = new WxDlgAnimationCompression( this, -1 );
		DlgAnimationCompression->Show();

		ToolBar->ToggleTool( IDM_ANIMSET_OpenAnimationCompressionDlg, true );
		MenuBar->Check( IDM_ANIMSET_OpenAnimationCompressionDlg, true );
	}
	else
	{
		DlgAnimationCompression->Close();
		DlgAnimationCompression = NULL;
		ToolBar->ToggleTool( IDM_ANIMSET_OpenAnimationCompressionDlg, false );
		MenuBar->Check( IDM_ANIMSET_OpenAnimationCompressionDlg, false );
	}
}

/** Updates the contents of the status bar, based on e.g. the selected set/sequence. */
void WxAnimSetViewer::UpdateStatusBar()
{
	StatusBar->UpdateStatusBar( this );
}

/** Create a new Socket object and add it to the selected SkeletalMesh's Sockets array. */
void WxAnimSetViewer::OnNewSocket( wxCommandEvent& In )
{
	if(SocketMgr)
	{
		// We only allow sockets to be created to bones that are in all LODs.

		// Get the bones we'll be updating if using the lowest LOD.
		FStaticLODModel& LODModel = SelectedSkelMesh->LODModels( SelectedSkelMesh->LODModels.Num()-1 );
		TArray<BYTE> LODBones = LODModel.RequiredBones;

		// Make list of bone names in lowest LOD.
		TArray<FString> BoneNames;
		BoneNames.AddZeroed( LODBones.Num() );
		for(INT i=0; i<LODBones.Num(); i++)
		{
			FName BoneName = SelectedSkelMesh->RefSkeleton( LODBones(i) ).Name;
			BoneNames(i) = BoneName.ToString();
		}

		// Display dialog and let user pick which bone to create a Socket for.
		WxDlgGenericComboEntry BoneDlg;
		if( BoneDlg.ShowModal( TEXT("NewSocket"), TEXT("BoneName"), BoneNames, 0, TRUE ) == wxID_OK )
		{
			FName BoneName = FName( *BoneDlg.GetSelectedString() );
			if(BoneName != NAME_None)
			{
				WxDlgGenericStringEntry NameDlg;
				UBOOL bLoopAddSocket = TRUE;

				while(bLoopAddSocket)
				{
					if( NameDlg.ShowModal( TEXT("NewSocket"), TEXT("SocketName"), TEXT("") ) == wxID_OK )
					{
						UBOOL bNameAlreadyExists = FALSE;
						for(INT SocketIndex = 0; SocketIndex < SelectedSkelMesh->Sockets.Num(); ++SocketIndex)
						{
							if(NameDlg.GetEnteredString() == SelectedSkelMesh->Sockets(SocketIndex)->SocketName.GetNameString())
							{
								bNameAlreadyExists = TRUE;
								break;
							}
						}

						if(bNameAlreadyExists)
						{
							appMsgf(AMT_OK, LocalizeSecure(LocalizeUnrealEd("SocketAlreadyExists"), *NameDlg.GetEnteredString()));
							continue;
						}

						FName NewSocketName = FName( *NameDlg.GetEnteredString() );
						if(NewSocketName != NAME_None)
						{
							USkeletalMeshSocket* NewSocket = ConstructObject<USkeletalMeshSocket>( USkeletalMeshSocket::StaticClass(), SelectedSkelMesh );
							check(NewSocket);

							NewSocket->SocketName = NewSocketName;
							NewSocket->BoneName = BoneName;

							SelectedSkelMesh->Sockets.AddItem(NewSocket);
							SelectedSkelMesh->MarkPackageDirty();

							UpdateSocketList();
							SetSelectedSocket(NewSocket);
						}
					}

					// If a valid name wasn't entered then continue was called to loop over which means you can't get to this point
					bLoopAddSocket = FALSE;
				}
			}
		}
	}
}

/** Delete the currently selected Socket. */
void WxAnimSetViewer::OnDeleteSocket( wxCommandEvent& In )
{
	if(SocketMgr)
	{
		if(SelectedSocket)
		{
			SelectedSkelMesh->Sockets.RemoveItem(SelectedSocket);
			SelectedSkelMesh->MarkPackageDirty();

			UpdateSocketList();
			SetSelectedSocket(NULL);
			RecreateSocketPreviews();

			FASVViewportClient* ASVPreviewVC = GetPreviewVC();
			check( ASVPreviewVC );
			ASVPreviewVC->Viewport->Invalidate();
		}
	}
}

/** 
*	Handler called when clicking on a socket from the list. 
*	Sets the socket properties into the property window, and set the SelectedSocket variable.
*/
void WxAnimSetViewer::OnClickSocket( wxCommandEvent& In )
{
	if(SocketMgr)
	{
		// Get the index of the item we clicked on.
		INT SocketIndex = SocketMgr->SocketList->GetSelection();		
		if(SocketIndex >= 0 && SocketIndex < SelectedSkelMesh->Sockets.Num())
		{
			SetSelectedSocket( SelectedSkelMesh->Sockets(SocketIndex) );
		}
	}
}

/** Called when clicking any of the socket movement mode buttons, to change the current mode. */
void WxAnimSetViewer::OnSocketMoveMode( wxCommandEvent& In )
{
	if(SocketMgr)
	{
		INT Id = In.GetId();
		if(Id == IDM_ANIMSET_SOCKET_TRANSLATE)
		{
			SetSocketMoveMode(SMM_Translate);
		}
		else if(Id == IDM_ANIMSET_SOCKET_ROTATE)
		{
			SetSocketMoveMode(SMM_Rotate);
		}
	}
}

/** Iterate over all sockets clearing their preview skeletal/static mesh. */
void WxAnimSetViewer::OnClearPreviewMeshes( wxCommandEvent& In )
{
	ClearSocketPreviews();

	for(INT i=0; i<SelectedSkelMesh->Sockets.Num(); i++)
	{
		USkeletalMeshSocket* Socket = SelectedSkelMesh->Sockets(i);
		Socket->PreviewSkelMesh = NULL;
		Socket->PreviewStaticMesh = NULL;
		Socket->PreviewSkelComp = NULL;
	}

	if(SocketMgr)
	{
		SocketMgr->SocketProps->SetObject( SelectedSocket, true, false, false );
	}
}

/** Copy socket info from mesh to clipboard. */
void WxAnimSetViewer::OnCopySockets( wxCommandEvent& In )
{
	// First, clear existing buffer
	GUnrealEd->SkelSocketPasteBuffer.Empty();

	// Then copy data into it.
	GUnrealEd->SkelSocketPasteBuffer.AddZeroed(SelectedSkelMesh->Sockets.Num());
	for(INT i=0; i<SelectedSkelMesh->Sockets.Num(); i++)
	{
		USkeletalMeshSocket* Socket = SelectedSkelMesh->Sockets(i);
		GUnrealEd->SkelSocketPasteBuffer(i).SocketName			= Socket->SocketName;
		GUnrealEd->SkelSocketPasteBuffer(i).BoneName			= Socket->BoneName;
		GUnrealEd->SkelSocketPasteBuffer(i).RelativeLocation	= Socket->RelativeLocation;
		GUnrealEd->SkelSocketPasteBuffer(i).RelativeRotation	= Socket->RelativeRotation;
		GUnrealEd->SkelSocketPasteBuffer(i).RelativeScale		= Socket->RelativeScale;
	}
}

/** Paste socket info from clipboard to current mesh. */
void WxAnimSetViewer::OnPasteSockets( wxCommandEvent& In )
{
	// First check buffer is not empty.
	if(GUnrealEd->SkelSocketPasteBuffer.Num() == 0)
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("NoSocketsToPaste") );
		return;
	}

	// Then iterate over each one
	for(INT i=0; i<GUnrealEd->SkelSocketPasteBuffer.Num(); i++)
	{
		FName SocketName = GUnrealEd->SkelSocketPasteBuffer(i).SocketName;

		// See if we have a socket with this name.
		UBOOL bNameAlreadyExists = FALSE;
		for(INT SocketIndex = 0; SocketIndex < SelectedSkelMesh->Sockets.Num(); ++SocketIndex)
		{
			if(SocketName == SelectedSkelMesh->Sockets(SocketIndex)->SocketName)
			{
				bNameAlreadyExists = TRUE;
				break;
			}
		}

		if(bNameAlreadyExists)
		{
			appMsgf(AMT_OK, LocalizeSecure(LocalizeUnrealEd("SocketAlreadyExists"), *SocketName.ToString()));
			continue;
		}

		// Check there is a bone with the bone name.
		FName BoneName = GUnrealEd->SkelSocketPasteBuffer(i).BoneName;
		INT BoneIndex = SelectedSkelMesh->MatchRefBone( BoneName );
		if(BoneIndex == INDEX_NONE)
		{
			appMsgf(AMT_OK, LocalizeSecure(LocalizeUnrealEd("SocketBoneNotFound"), *BoneName.ToString(), *SocketName.ToString()));
			continue;
		}

		// Things look good - create socket, and add to skel mesh
		USkeletalMeshSocket* NewSocket = ConstructObject<USkeletalMeshSocket>( USkeletalMeshSocket::StaticClass(), SelectedSkelMesh );
		if(NewSocket)
		{
			NewSocket->SocketName = SocketName;
			NewSocket->BoneName = BoneName;

			NewSocket->RelativeLocation = GUnrealEd->SkelSocketPasteBuffer(i).RelativeLocation;
			NewSocket->RelativeRotation = GUnrealEd->SkelSocketPasteBuffer(i).RelativeRotation;
			NewSocket->RelativeScale = GUnrealEd->SkelSocketPasteBuffer(i).RelativeScale;

			SelectedSkelMesh->Sockets.AddItem(NewSocket);
		}
	}

	SelectedSkelMesh->MarkPackageDirty();
	UpdateSocketList();
}

//////////////////////////////////////////////////////////////////////////

/** Set the movement mode for the selected socket to the one supplied. */
void WxAnimSetViewer::SetSocketMoveMode( EASVSocketMoveMode NewMode )
{
	if(SocketMgr)
	{
		SocketMgr->MoveMode = NewMode;

		if(SocketMgr->MoveMode == SMM_Rotate)
		{
			SocketMgr->ToolBar->ToggleTool( IDM_ANIMSET_SOCKET_TRANSLATE, false );
			SocketMgr->ToolBar->ToggleTool( IDM_ANIMSET_SOCKET_ROTATE, true );
		}
		else
		{
			SocketMgr->ToolBar->ToggleTool( IDM_ANIMSET_SOCKET_TRANSLATE, true );
			SocketMgr->ToolBar->ToggleTool( IDM_ANIMSET_SOCKET_ROTATE, false );
		}

		SocketMgr->UpdateTextEntry();

		// Flush hit proxies so we get them for the new widget.
		FASVViewportClient* ASVPreviewVC = GetPreviewVC();
		check( ASVPreviewVC );
		ASVPreviewVC->Viewport->Invalidate();
	}
}

/** Set the currently selected socket to the supplied one */
void WxAnimSetViewer::SetSelectedSocket(USkeletalMeshSocket* InSocket)
{
	if(InSocket && SocketMgr)
	{
		UINT SocketIndex = static_cast<UINT>(SelectedSkelMesh->Sockets.FindItemIndex(InSocket));
		if(SocketIndex != INDEX_NONE)
		{
			SelectedSocket = InSocket;
			SocketMgr->SocketProps->SetObject( SelectedSocket, true, false, false );
			if(SocketIndex >= 0 && SocketIndex < SocketMgr->SocketList->GetCount())
			{
				SocketMgr->SocketList->SetSelection(SocketIndex);
			}

			// Flush hit proxies so we get them for the new widget.
			FASVViewportClient* ASVPreviewVC = GetPreviewVC();
			check( ASVPreviewVC );
			ASVPreviewVC->Viewport->Invalidate();

			return;
		}
	}

	SelectedSocket = NULL;

	if(SocketMgr)
	{
		SocketMgr->SocketProps->SetObject(NULL, true, false, false );
		SocketMgr->SocketList->DeselectAll();
	}
}

/** Update the list of sockets shown in the Socket Manager list box to match that of the selected SkeletalMesh. */
void WxAnimSetViewer::UpdateSocketList()
{
	if(SocketMgr)
	{
		// Remember the current selection.
		UINT CurrentSelection = static_cast<UINT>(SocketMgr->SocketList->GetSelection());

		SocketMgr->SocketList->Clear();

		// Remove empty sockets
		for(INT i=0; i<SelectedSkelMesh->Sockets.Num(); i++)
		{
			USkeletalMeshSocket* Socket = SelectedSkelMesh->Sockets(i);
			if (NULL == Socket)
			{
				SelectedSkelMesh->Sockets.Remove(i);
				i--;
			}
		}

		for(INT i=0; i<SelectedSkelMesh->Sockets.Num(); i++)
		{
			USkeletalMeshSocket* Socket = SelectedSkelMesh->Sockets(i);
			SocketMgr->SocketList->Append( *(Socket->SocketName.ToString()), Socket );
		}

		// Restore the selection we had.
		if(CurrentSelection >= 0 && CurrentSelection < SocketMgr->SocketList->GetCount())
		{
			SocketMgr->SocketList->SetSelection(CurrentSelection);
		}
	}
}

/** Destroy all components being used to preview sockets. */
void WxAnimSetViewer::ClearSocketPreviews()
{
	FASVViewportClient* ASVPreviewVC = GetPreviewVC();
	check( ASVPreviewVC );

	for(INT i=0; i<SocketPreviews.Num(); i++)
	{
		UPrimitiveComponent* PrimComp = SocketPreviews(i).PreviewComp;
		PreviewSkelComp->DetachComponent(PrimComp);
		ASVPreviewVC->PreviewScene.RemoveComponent(PrimComp);
	}
	SocketPreviews.Empty();
}


/** Update the Skeletal and StaticMesh Components used to preview attachments in the editor. */
void WxAnimSetViewer::RecreateSocketPreviews()
{
	ClearSocketPreviews();

	if(SelectedSkelMesh)
	{
		FASVViewportClient* ASVPreviewVC = GetPreviewVC();
		check( ASVPreviewVC );

		// Then iterate over Sockets creating component for any preview skeletal/static meshes.
		for(INT i=0; i<SelectedSkelMesh->Sockets.Num(); i++)
		{
			USkeletalMeshSocket* Socket = SelectedSkelMesh->Sockets(i);
			
			if(Socket && Socket->PreviewSkelMesh)
			{
				// Create SkeletalMeshComponent and fill in mesh and scene.
				USkeletalMeshComponent* NewSkelComp = ConstructObject<USkeletalMeshComponent>(USkeletalMeshComponent::StaticClass());
				NewSkelComp->SkeletalMesh = Socket->PreviewSkelMesh;

				// Attach component to this socket.
				PreviewSkelComp->AttachComponentToSocket(NewSkelComp, Socket->SocketName);

				// Assign SkelComp, so we can edit its properties.
				Socket->PreviewSkelComp = NewSkelComp;
				// Create AnimNodeSequence for previewing animation playback
				Socket->PreviewSkelComp->Animations = ConstructObject<UAnimNodeSequence>(UAnimNodeSequence::StaticClass());

				// And keep track of it.
				new( SocketPreviews )FASVSocketPreview(Socket, NewSkelComp);
			}

			if(Socket && Socket->PreviewStaticMesh)
			{
				// Create StaticMeshComponent and fill in mesh and scene.
				UStaticMeshComponent* NewMeshComp = ConstructObject<UStaticMeshComponent>(UStaticMeshComponent::StaticClass());
				NewMeshComp->StaticMesh = Socket->PreviewStaticMesh;

				// Attach component to this socket.
				PreviewSkelComp->AttachComponentToSocket(NewMeshComp, Socket->SocketName);

				// And keep track of it
				new( SocketPreviews )FASVSocketPreview(Socket, NewMeshComp);
			}
		}
	}
}

/** Update the components being used to preview sockets. Need to call this when you change a Socket to see its affect. */
void WxAnimSetViewer::UpdateSocketPreviews()
{
	for(INT i=0; i<SocketPreviews.Num(); i++)
	{
		PreviewSkelComp->DetachComponent( SocketPreviews(i).PreviewComp );
		PreviewSkelComp->AttachComponentToSocket( SocketPreviews(i).PreviewComp, SocketPreviews(i).Socket->SocketName );
	}
}


/**
 *	This function returns the name of the docking parent.  This name is used for saving and loading the layout files.
 *  @return A string representing a name to use for this docking parent.
 */
const TCHAR* WxAnimSetViewer::GetDockingParentName() const
{
	return TEXT("AnimSetViewer");
}

/**
 * @return The current version of the docking parent, this value needs to be increased every time new docking windows are added or removed.
 */
const INT WxAnimSetViewer::GetDockingParentVersion() const
{
	return 0;
}

/**
 *	Update the preview window.
 */
void WxAnimSetViewer::UpdatePreviewWindow()
{
	if (PreviewWindow)
	{
		if (PreviewWindow->ASVPreviewVC)
		{
			if (PreviewWindow->ASVPreviewVC->Viewport)
			{
				PreviewWindow->ASVPreviewVC->Viewport->Invalidate();
			}
		}
	}
}
