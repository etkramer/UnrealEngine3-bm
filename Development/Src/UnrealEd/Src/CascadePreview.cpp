/*=============================================================================
	CascadePreview.cpp: 'Cascade' particle editor preview pane
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "Cascade.h"
#include "MouseDeltaTracker.h"
#include "EngineMaterialClasses.h"
#include "ImageUtils.h"

static FColor CASC_GridColorHi = FColor(0,0,80);
static FColor CASC_GridColorLo = FColor(0,0,72);

/*-----------------------------------------------------------------------------
FCascadePreviewViewportClient
-----------------------------------------------------------------------------*/

FCascadePreviewViewportClient::FCascadePreviewViewportClient(WxCascade* InCascade):
	PreviewScene( FRotator(-45.f*(65535.f/360.f), 180.f*(65535.f/360.f), 0.f), 0.25f, 1.f)
{
	Cascade = InCascade;
	check(Cascade);

	if (Cascade && Cascade->EditorOptions)
	{
		CASC_GridColorHi = Cascade->EditorOptions->GridColor_Hi;
		CASC_GridColorLo = Cascade->EditorOptions->GridColor_Low;
	}

	//@todo. Put these in the options class for Cascade...
	DrawHelper.bDrawGrid = Cascade->EditorOptions->bShowGrid;
	DrawHelper.GridColorHi = CASC_GridColorHi;
	DrawHelper.GridColorLo = CASC_GridColorLo;
	DrawHelper.bDrawKillZ = FALSE;
	DrawHelper.bDrawWorldBox = FALSE;
	DrawHelper.bDrawPivot = FALSE;
	if (Cascade && Cascade->EditorOptions)
	{
		DrawHelper.PerspectiveGridSize = Cascade->EditorOptions->GridPerspectiveSize;
	}
	else
	{
		DrawHelper.PerspectiveGridSize = 32767;
	}
	DrawHelper.DepthPriorityGroup = SDPG_World;
	//@todo. END - Put these in the options class for Cascade...
	//
	check(Cascade->EditorOptions);
	if (Cascade->EditorOptions->FloorMesh == TEXT(""))
	{
		if (Cascade->PartSys != NULL)
		{
			Cascade->EditorOptions->FloorMesh = Cascade->PartSys->FloorMesh;
			Cascade->EditorOptions->FloorScale = Cascade->PartSys->FloorScale;
			Cascade->EditorOptions->FloorScale3D = Cascade->PartSys->FloorScale3D;
		}
		else
		{
			Cascade->EditorOptions->FloorMesh = FString::Printf(TEXT("EditorMeshes.AnimTreeEd_PreviewFloor"));
			Cascade->EditorOptions->FloorScale = 1.0f;
			Cascade->EditorOptions->FloorScale3D = FVector(1.0f, 1.0f, 1.0f);
		}
		Cascade->EditorOptions->bShowFloor = FALSE;
	}

	UStaticMesh* Mesh = NULL;
	FloorComponent = NULL;
	if (Cascade->PartSys)
	{
		Mesh = (UStaticMesh*)UObject::StaticLoadObject(UStaticMesh::StaticClass(),NULL,
			*(Cascade->PartSys->FloorMesh),NULL,LOAD_None,NULL);
	}
	if ((Mesh == NULL) && (Cascade->EditorOptions->FloorMesh != TEXT("")))
	{
		Mesh = (UStaticMesh*)UObject::StaticLoadObject(UStaticMesh::StaticClass(),NULL,
			*(Cascade->EditorOptions->FloorMesh),NULL,LOAD_None,NULL);
	}
	if (Mesh == NULL)
	{
		// Safety catch...
		Cascade->EditorOptions->FloorMesh = FString::Printf(TEXT("EditorMeshes.AnimTreeEd_PreviewFloor"));
		Mesh = (UStaticMesh*)UObject::StaticLoadObject(UStaticMesh::StaticClass(),NULL,
			*(Cascade->EditorOptions->FloorMesh),NULL,LOAD_None,NULL);
	}

	if (Mesh)
	{
		FloorComponent = ConstructObject<UStaticMeshComponent>(UStaticMeshComponent::StaticClass());
		check(FloorComponent);
		FloorComponent->StaticMesh = Mesh;
		FloorComponent->DepthPriorityGroup = SDPG_World;

		// Hide it for now...
		FloorComponent->HiddenEditor = !Cascade->EditorOptions->bShowFloor;
		FloorComponent->HiddenGame = !Cascade->EditorOptions->bShowFloor;
		if (Cascade->PartSys)
		{
			FloorComponent->Translation = Cascade->PartSys->FloorPosition;
			FloorComponent->Rotation = Cascade->PartSys->FloorRotation;
			FloorComponent->Scale = Cascade->PartSys->FloorScale;
			FloorComponent->Scale3D = Cascade->PartSys->FloorScale3D;
		}
		else
		{
			FloorComponent->Translation = Cascade->EditorOptions->FloorPosition;
			FloorComponent->Rotation = Cascade->EditorOptions->FloorRotation;
			FloorComponent->Scale = Cascade->EditorOptions->FloorScale;
			FloorComponent->Scale3D = Cascade->EditorOptions->FloorScale3D;
		}
		PreviewScene.AddComponent(FloorComponent,FMatrix::Identity);
	}

	// Create the post process chain
	SetupPostProcessChain();

	// Create ParticleSystemComponent to use for preview.
	Cascade->PartSysComp = ConstructObject<UCascadeParticleSystemComponent>(UCascadeParticleSystemComponent::StaticClass());
	Cascade->PartSysComp->CascadePreviewViewportPtr = this;
	PreviewScene.AddComponent(Cascade->PartSysComp,FMatrix::Identity);
	Cascade->PartSysComp->SetFlags(RF_Transactional);

	// Create CascadePreviewComponent for drawing extra useful info
	Cascade->CascPrevComp = ConstructObject<UCascadePreviewComponent>(UCascadePreviewComponent::StaticClass());
	Cascade->CascPrevComp->CascadePtr = InCascade;
	PreviewScene.AddComponent(Cascade->CascPrevComp,FMatrix::Identity);

	LineBatcher = ConstructObject<ULineBatchComponent>(ULineBatchComponent::StaticClass());
	PreviewScene.AddComponent(LineBatcher,FMatrix::Identity);

	TimeScale = 1.f;

	BackgroundColor = FColor(0, 0, 0);

	// Update light components from parameters now
	UpdateLighting();

	// Default view position
	// TODO: Store in ParticleSystem
	ViewLocation = FVector(-200.f, 0.f, 0.f);
	ViewRotation = FRotator(0, 0, 0);	

	PreviewAngle = FRotator(0, 0, 0);
	PreviewDistance = 0.f;

	TotalTime = 0.f;

	// Use game defaults to hide emitter sprite etc.
	ShowFlags = SHOW_DefaultGame | SHOW_Grid;

	//@todo. Temp hack - unlit mode forced
	ShowFlags &= ~SHOW_Lighting;

	bDrawOriginAxes = FALSE;
	bDrawParticleCounts = TRUE;
	bDrawParticleEvents = FALSE;
	bDrawParticleTimes = TRUE;
	bDrawSystemDistance = FALSE;

	SetRealtime( 1 );

	bWireframe	= FALSE;
	bBounds = FALSE;
	bAllowMayaCam = TRUE;
	bDrawWireSphere = FALSE;
	WireSphereRadius = 150.f;

	bCaptureScreenShot = FALSE;
}

FLinearColor FCascadePreviewViewportClient::GetBackgroundColor()
{
	return BackgroundColor;
}

void FCascadePreviewViewportClient::UpdateLighting()
{
	// TODO
}

void FCascadePreviewViewportClient::SetupPostProcessChain()
{
    Cascade->DefaultPostProcessName = Cascade->EditorOptions->PostProcessChainName;
	ShowPPFlags = Cascade->EditorOptions->ShowPPFlags;
	if (Cascade->DefaultPostProcessName != FString(TEXT("")))
	{
		Cascade->DefaultPostProcess = LoadObject<UPostProcessChain>(
			NULL,*(Cascade->DefaultPostProcessName),NULL,LOAD_None,NULL);

		if (Cascade->DefaultPostProcess)
		{
			Cascade->UpdatePostProcessChain();
		}
		else
		{
			warnf(TEXT("CASCADE: Failed to load default post process chain."));
		}
	}
}

void FCascadePreviewViewportClient::Draw(FViewport* Viewport, FCanvas* Canvas)
{
	// We make sure the background is black because some of it will show through (black bars).
	Clear(Canvas,BackgroundColor);

	// clear any lines rendered the previous frame
	LineBatcher->BatchedLines.Empty();
	PreviewScene.RemoveComponent(LineBatcher);

	const FVector XAxis(1,0,0); 
	const FVector YAxis(0,1,0); 
	const FVector ZAxis(0,0,1);
	if (bDrawOriginAxes)
	{
		FMatrix ArrowMatrix = FMatrix(XAxis, YAxis, ZAxis, FVector(0.f));
		DrawDirectionalArrow(LineBatcher, ArrowMatrix, FColor(255,0,0), 10.f, 1.0f, SDPG_World);

		ArrowMatrix = FMatrix(YAxis, ZAxis, XAxis, FVector(0.f));
		DrawDirectionalArrow(LineBatcher, ArrowMatrix, FColor(0,255,0), 10.f, 1.0f, SDPG_World);

		ArrowMatrix = FMatrix(ZAxis, XAxis, YAxis, FVector(0.f));
		DrawDirectionalArrow(LineBatcher, ArrowMatrix, FColor(0,0,255), 10.f, 1.0f, SDPG_World);
	}
	if (bDrawWireSphere)
	{
		for (INT i = 0; i < 16; i++)
		{
			FVector Base = FVector(0.f, 0.f, 0.f);
			FVector RotXAxis;
			FVector RotYAxis;
			FVector RotZAxis;
			FColor WireColor = FColor(255, 0, 0);

			FRotationMatrix RotMatrix(FRotator(i * 4096, 0, 0));
			RotXAxis = RotMatrix.TransformFVector(XAxis);
			RotYAxis = RotMatrix.TransformFVector(YAxis);
			RotZAxis = RotMatrix.TransformFVector(ZAxis);
			DrawCircle(LineBatcher,Base, RotXAxis, RotYAxis, WireColor, WireSphereRadius, 32, SDPG_World);
			DrawCircle(LineBatcher,Base, RotXAxis, RotZAxis, WireColor, WireSphereRadius, 32, SDPG_World);
			DrawCircle(LineBatcher,Base, RotYAxis, RotZAxis, WireColor, WireSphereRadius, 32, SDPG_World);

			RotMatrix = FRotationMatrix(FRotator(0, i * 4096, 0));
			RotXAxis = RotMatrix.TransformFVector(XAxis);
			RotYAxis = RotMatrix.TransformFVector(YAxis);
			RotZAxis = RotMatrix.TransformFVector(ZAxis);
			DrawCircle(LineBatcher,Base, RotXAxis, RotYAxis, WireColor, WireSphereRadius, 32, SDPG_World);
			DrawCircle(LineBatcher,Base, RotXAxis, RotZAxis, WireColor, WireSphereRadius, 32, SDPG_World);
			DrawCircle(LineBatcher,Base, RotYAxis, RotZAxis, WireColor, WireSphereRadius, 32, SDPG_World);
		}
	}

	PreviewScene.AddComponent(LineBatcher,FMatrix::Identity);

	EShowFlags SavedShowFlags = ShowFlags;
	if (bWireframe)
	{
		ShowFlags &= ~SHOW_ViewMode_Mask;
		ShowFlags |= SHOW_ViewMode_Wireframe;
	}
	if (bBounds)
	{
		ShowFlags |= SHOW_Bounds;
	}
	else
	{
		ShowFlags &= ~SHOW_Bounds;
	}
	if (ShowPPFlags != 0)
	{
		ShowFlags |= SHOW_PostProcess;
	}
	else
	{
		ShowFlags &= ~SHOW_PostProcess;
	}

	FEditorLevelViewportClient::Draw(Viewport, Canvas);
	ShowFlags = SavedShowFlags;

	if (bDrawParticleCounts || bDrawParticleTimes || bDrawParticleEvents)
	{
		// 'Up' from the lower left...
		FString strOutput;
		const INT XPosition = Viewport->GetSizeX() - 5;
		INT YPosition = Viewport->GetSizeY() - 5;

		UParticleSystemComponent* PartComp = Cascade->PartSysComp;

		INT iWidth, iHeight;

		if (PartComp->EmitterInstances.Num())
		{
			for (INT i = 0; i < PartComp->EmitterInstances.Num(); i++)
			{
				strOutput = TEXT("");
				FParticleEmitterInstance* Instance = PartComp->EmitterInstances(i);
				//@todo Instance->SpriteTemplate being NULL indicates a higher level problem that needs to be looked at
				if (Instance && Instance->SpriteTemplate && Instance->SpriteTemplate->EmitterRenderMode != ERM_None)
				{
					//@todo. Add appropriate editor support for LOD here!
					UParticleLODLevel* LODLevel = Instance->SpriteTemplate->GetCurrentLODLevel(Instance);
					UParticleLODLevel* HighLODLevel = Instance->SpriteTemplate->GetLODLevel(0);
					if (LODLevel && (LODLevel->RequiredModule->EmitterRenderMode != ERM_None))
					{
						if (bDrawParticleCounts)
						{
							strOutput += FString::Printf(TEXT("%4d/%4d"), 
								Instance->ActiveParticles, HighLODLevel->PeakActiveParticles);
						}
						if (bDrawParticleTimes)
						{
							if (bDrawParticleCounts)
							{
								strOutput += TEXT("/");
							}
							strOutput += FString::Printf(TEXT("%8.4f/%8.4f"), 
								Instance->EmitterTime, Instance->SecondsSinceCreation);
						}
#if !FINAL_RELEASE
						if (bDrawParticleEvents)
						{
							if (bDrawParticleCounts || bDrawParticleTimes)
							{
								strOutput += TEXT("/");
							}
							strOutput += FString::Printf(TEXT("Evts: %4d/%4d"), Instance->EventCount, Instance->MaxEventCount);
						}
#endif	//#if !FINAL_RELEASE
						UCanvas::ClippedStrLen(GEngine->TinyFont, 1.0f, 1.0f, iWidth, iHeight, *strOutput);
						DrawString(Canvas,XPosition - iWidth, YPosition - iHeight, *strOutput, GEngine->TinyFont, LODLevel->RequiredModule->EmitterEditorColor);
						YPosition -= iHeight - 2;
					}
				}
			}
		}
		else
		{
			for (INT i = 0; i < PartComp->Template->Emitters.Num(); i++)
			{
				strOutput = TEXT("");
				UParticleEmitter* Emitter = PartComp->Template->Emitters(i);
				//@todo. Add appropriate editor support for LOD here!
				UParticleLODLevel* LODLevel = Emitter->GetLODLevel(0);
				if (LODLevel && LODLevel->bEnabled && (LODLevel->RequiredModule->EmitterRenderMode != ERM_None))
				{
					if (bDrawParticleCounts)
					{
						strOutput += FString::Printf(TEXT("%4d/%4d"), 
							0, LODLevel->PeakActiveParticles);
					}
					if (bDrawParticleTimes)
					{
						if (bDrawParticleCounts)
						{
							strOutput += TEXT("/");
						}
						strOutput += FString::Printf(TEXT("%8.4f/%8.4f"), 0.f, 0.f);
					}
#if !FINAL_RELEASE
					if (bDrawParticleEvents)
					{
						if (bDrawParticleCounts || bDrawParticleTimes)
						{
							strOutput += TEXT("/");
						}
						strOutput += FString::Printf(TEXT("Evts: %4d/%4d"), 0, 0);
					}
#endif	//#if !FINAL_RELEASE
					UCanvas::ClippedStrLen(GEngine->TinyFont, 1.0f, 1.0f, iWidth, iHeight, *strOutput);
					DrawString(Canvas,XPosition - iWidth, YPosition - iHeight, *strOutput, GEngine->TinyFont, 
						LODLevel->RequiredModule->EmitterEditorColor);
					YPosition -= iHeight - 2;
				}
			}
		}
	}

	if (bDrawSystemDistance)
	{
	}

	if (Cascade->ToolBar)
	{
		if (IsRealtime() && !Cascade->ToolBar->bRealtime)
		{
			Cascade->ToolBar->ToggleTool(IDM_CASCADE_REALTIME, TRUE);
			Cascade->ToolBar->bRealtime	= TRUE;
		}
		else
		if (!IsRealtime() && Cascade->ToolBar->bRealtime)
		{
			Cascade->ToolBar->ToggleTool(IDM_CASCADE_REALTIME, FALSE);
			Cascade->ToolBar->bRealtime	= FALSE;
		}
	}

	if (Viewport && bCaptureScreenShot)
	{
		INT SrcWidth = Viewport->GetSizeX();
		INT SrcHeight = Viewport->GetSizeY();
		// Read the contents of the viewport into an array.
		TArray<FColor> OrigBitmap;
		if (Viewport->ReadPixels(OrigBitmap))
		{
			check(OrigBitmap.Num() == SrcWidth * SrcHeight);

			// Resize image to enforce max size.
			TArray<FColor> ScaledBitmap;
			INT ScaledWidth	 = 512;
			INT ScaledHeight = 512;
			FImageUtils::ImageResize( SrcWidth, SrcHeight, OrigBitmap, ScaledWidth, ScaledHeight, ScaledBitmap );

			// Compress.
			EObjectFlags ObjectFlags = RF_NotForClient|RF_NotForServer;
			Cascade->PartSys->ThumbnailImage = FImageUtils::ConstructTexture2D( ScaledWidth, ScaledHeight, ScaledBitmap, Cascade->PartSys, TEXT("ThumbnailTexture"), ObjectFlags );

			Cascade->PartSys->ThumbnailImageOutOfDate = FALSE;
			Cascade->PartSys->MarkPackageDirty();
		}

		bCaptureScreenShot = FALSE;
	}
}

void FCascadePreviewViewportClient::Draw(const FSceneView* View,FPrimitiveDrawInterface* PDI)
{
	DrawHelper.Draw(View, PDI);
	FEditorLevelViewportClient::Draw(View, PDI);
	// Can now iterate over the modules on this system...
    for (INT i = 0; i < Cascade->PartSys->Emitters.Num(); i++)
	{
		UParticleEmitter* Emitter = Cascade->PartSys->Emitters(i);
		if (Emitter == NULL)
		{
			//@todo.SAS. Handle 'empty' emitter slots!
			continue;
		}

		// Emitters may have a set number of loops.
		// After which, the system will kill them off
		if (i < Cascade->PartSysComp->EmitterInstances.Num())
		{
			FParticleEmitterInstance* EmitterInst = Cascade->PartSysComp->EmitterInstances(i);
			if (EmitterInst && EmitterInst->SpriteTemplate)
			{
				check(EmitterInst->SpriteTemplate == Emitter);

				UParticleLODLevel* LODLevel = Emitter->GetCurrentLODLevel(EmitterInst);
				for( INT j=0; j< LODLevel->Modules.Num(); j++)
				{
					UParticleModule* Module = LODLevel->Modules(j);
					if (Module && Module->bSupported3DDrawMode && Module->b3DDrawMode)
					{
						Module->Render3DPreview( EmitterInst, View, PDI);
					}
				}
			}
		}
	}

//	Cascade->PartSysComp->SetLODLevel(0);
}

/**
 * Configures the specified FSceneView object with the view and projection matrices for this viewport.
 * @param	View		The view to be configured.  Must be valid.
 * @return	A pointer to the view within the view family which represents the viewport's primary view.
 */
FSceneView* FCascadePreviewViewportClient::CalcSceneView(FSceneViewFamily* ViewFamily)
{
	FSceneView* View = FEditorLevelViewportClient::CalcSceneView(ViewFamily);
	if (View)
	{
		View->PostProcessChain = Cascade->DefaultPostProcess;
	}

	return View;
}

UBOOL FCascadePreviewViewportClient::InputKey(FViewport* Viewport, INT ControllerId, FName Key, EInputEvent Event,FLOAT /*AmountDepressed*/,UBOOL /*Gamepad*/)
{
	// Hide and lock mouse cursor if we're capturing mouse input
	Viewport->ShowCursor( !Viewport->HasMouseCapture() );
	Viewport->LockMouseToWindow( Viewport->HasMouseCapture() );

	if (Event == IE_Pressed)
	{
		if (Key == KEY_SpaceBar)
		{
			//
			if (Cascade->EditorOptions->bUseSpaceBarReset)
			{
				wxCommandEvent In;
				if (Cascade->EditorOptions->bUseSpaceBarResetInLevel == FALSE)
				{
					Cascade->OnResetSystem(In);
				}
				else
				{
					Cascade->OnResetInLevel(In);
				}
			}
		}
	}

	// Handle viewport screenshot.
	InputTakeScreenshot( Viewport, Key, Event );

	return TRUE;
}

void FCascadePreviewViewportClient::MouseMove(FViewport* Viewport, INT X, INT Y)
{

}

UBOOL FCascadePreviewViewportClient::InputAxis(FViewport* Viewport, INT ControllerId, FName Key, FLOAT Delta, FLOAT DeltaTime, UBOOL bGamepad)
{
/***
	if((Key == KEY_MouseX || Key == KEY_MouseY) && Delta != 0.0f)
	{
		UBOOL LeftMouseButton = Viewport->KeyState(KEY_LeftMouseButton);
		UBOOL MiddleMouseButton = Viewport->KeyState(KEY_MiddleMouseButton);
		UBOOL RightMouseButton = Viewport->KeyState(KEY_RightMouseButton);

		FLOAT DX = (Key == KEY_MouseX) ? Delta : 0.0f;
		FLOAT DY = (Key == KEY_MouseY) ? Delta : 0.0f;

		FLOAT YawSpeed = 20.0f;
		FLOAT PitchSpeed = 20.0f;
		FLOAT ZoomSpeed = 1.0f;

		FRotator NewPreviewAngle = PreviewAngle;
		FLOAT NewPreviewDistance = PreviewDistance;

		if(LeftMouseButton != RightMouseButton)
		{
			NewPreviewAngle.Yaw += DX * YawSpeed;
			NewPreviewAngle.Pitch += DY * PitchSpeed;
			NewPreviewAngle.Clamp();
		}
		else if(LeftMouseButton && RightMouseButton)
		{
			NewPreviewDistance += DY * ZoomSpeed;
			NewPreviewDistance = Clamp( NewPreviewDistance, 0.f, 100000.f);
		}
		
		SetPreviewCamera(NewPreviewAngle, NewPreviewDistance);
	}
***/

	if(Input->InputAxis(ControllerId,Key,Delta,DeltaTime))
		return TRUE;

	if((Key == KEY_MouseX || Key == KEY_MouseY) && Delta != 0.0f)
	{
		const UBOOL LeftMouseButton = Viewport->KeyState(KEY_LeftMouseButton);
		const UBOOL MiddleMouseButton = Viewport->KeyState(KEY_MiddleMouseButton);
		const UBOOL RightMouseButton = Viewport->KeyState(KEY_RightMouseButton);

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

				if(Cascade->bOrbitMode)
				{
					const FLOAT DX = (Key == KEY_MouseX) ? Delta : 0.0f;
					const FLOAT DY = (Key == KEY_MouseY) ? Delta : 0.0f;

					const FLOAT YawSpeed = 20.0f;
					const FLOAT PitchSpeed = 20.0f;
					const FLOAT ZoomSpeed = 1.0f;

					FRotator NewPreviewAngle = PreviewAngle;
					FLOAT NewPreviewDistance = PreviewDistance;

					if(LeftMouseButton != RightMouseButton)
					{
						NewPreviewAngle.Yaw += DX * YawSpeed;
						NewPreviewAngle.Pitch += DY * PitchSpeed;
						NewPreviewAngle.Clamp();
					}
					else if(LeftMouseButton && RightMouseButton)
					{
						NewPreviewDistance += DY * ZoomSpeed;
						NewPreviewDistance = Clamp( NewPreviewDistance, 0.f, 100000.f);
					}
					
					SetPreviewCamera(NewPreviewAngle, NewPreviewDistance);
				}
				else
				{
					MoveViewportCamera(Drag, Rot);
				}
			}

			MouseDeltaTracker->ReduceBy( DragDelta );
		}
	}

	if (!IsRealtime())
	{
		Viewport->Invalidate();
	}

	return TRUE;
}

void FCascadePreviewViewportClient::SetPreviewCamera(const FRotator& NewPreviewAngle, FLOAT NewPreviewDistance)
{
	PreviewAngle = NewPreviewAngle;
	PreviewDistance = NewPreviewDistance;

	ViewLocation = PreviewAngle.Vector() * -PreviewDistance;
	ViewRotation = PreviewAngle;

	Viewport->Invalidate();
}


/*-----------------------------------------------------------------------------
WxCascadePreview
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxCascadePreview, wxWindow )
	EVT_SIZE( WxCascadePreview::OnSize )
END_EVENT_TABLE()

WxCascadePreview::WxCascadePreview( wxWindow* InParent, wxWindowID InID, class WxCascade* InCascade  )
: wxWindow( InParent, InID )
{
	CascadePreviewVC = new FCascadePreviewViewportClient(InCascade);
	CascadePreviewVC->Viewport = GEngine->Client->CreateWindowChildViewport(CascadePreviewVC, (HWND)GetHandle());
	CascadePreviewVC->Viewport->CaptureJoystickInput(FALSE);
}

WxCascadePreview::~WxCascadePreview()
{

}

void WxCascadePreview::OnSize( wxSizeEvent& In )
{
	wxRect rc = GetClientRect();

	::MoveWindow( (HWND)CascadePreviewVC->Viewport->GetWindow(), 0, 0, rc.GetWidth(), rc.GetHeight(), 1 );
}

//
//	CascadePreviewComponent
//
void UCascadePreviewComponent::Render(const FSceneView* View,FPrimitiveDrawInterface* PDI)
{
	// Draw any module 'scene widgets'
	WxCascade* Cascade = CascadePtr;

	// Can now iterate over the modules on this system...
    for (INT i = 0; i < Cascade->PartSys->Emitters.Num(); i++)
	{
		UParticleEmitter* Emitter = Cascade->PartSys->Emitters(i);
		if (Emitter == NULL)
		{
			//@todo.SAS. Handle 'empty' emitter slots!
			continue;
		}

		// Emitters may have a set number of loops.
		// After which, the system will kill them off
		if (i < Cascade->PartSysComp->EmitterInstances.Num())
		{
			FParticleEmitterInstance* EmitterInst = Cascade->PartSysComp->EmitterInstances(i);
			if (EmitterInst && EmitterInst->SpriteTemplate)
			{
				check(EmitterInst->SpriteTemplate == Emitter);

				UParticleLODLevel* LODLevel = Emitter->GetCurrentLODLevel(EmitterInst);
				for (INT j=0; j< LODLevel->Modules.Num(); j++)
				{
					UParticleModule* Module = LODLevel->Modules(j);

					for( INT j=0; j< LODLevel->Modules.Num(); j++)
					{
						UParticleModule* Module = LODLevel->Modules(j);

						if (Module && Module->bSupported3DDrawMode && Module->b3DDrawMode)
						{
							Module->Render3DPreview( EmitterInst, View, PDI);
						}
					}
				}
			}
		}
	}

	Cascade->PartSysComp->SetLODLevel(0);
}
