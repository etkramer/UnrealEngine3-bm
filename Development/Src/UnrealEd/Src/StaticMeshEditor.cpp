/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "Factories.h"
#include "MouseDeltaTracker.h"
#include "EnginePhysicsClasses.h"
#include "ConvexDecompTool.h"
#include "StaticMeshEditor.h"
#include "..\..\Launch\Resources\resource.h"
#include "MeshUtils.h"
#include "Properties.h"
#include "DlgGenericComboEntry.h"
#include "DlgGenerateUVOptions.h"
#include "GeomTools.h"
#include "UnFracturedStaticMesh.h"
#include "EngineMeshClasses.h"
#include "GenericBrowser.h"
#include "MeshSimplificationWindow.h"

IMPLEMENT_CLASS(UStaticMeshEditorComponent);

static const FColor StaticMeshEditor_SelectedChunkColor(255, 220, 0);
static const FColor StaticMeshEditor_FilterTrueColor(255,60,60);
static const FColor StaticMeshEditor_FilterFalseColor(60,60,255);
static const FLOAT	StaticMeshEditor_TranslateSpeed = 0.25f;
static const FLOAT	StaticMeshEditor_RotateSpeed = 0.01f;
static const FLOAT	StaticMeshEditor_ScaleSpeed = 0.02f;

static const FLOAT	StaticMeshEditor_LightRotSpeed = 40.0f;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WxStaticMeshEditMenu
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class WxStaticMeshEditMenu : public wxMenuBar
{
public:
	WxStaticMeshEditMenu(UStaticMesh* InStaticMesh)
	{
		// Determine if the mesh is from a cooked package.
		const UPackage* MeshPackage = InStaticMesh->GetOutermost();
		const UBOOL bInCookedPackage = ( MeshPackage->PackageFlags & PKG_Cooked ) ? TRUE : FALSE;
		
		if ( !bInCookedPackage )
		{
			// File Menu
			wxMenu* FileMenu = new wxMenu();
			FileMenu->Append( IDM_SME_IMPORTMESHLOD, *LocalizeUnrealEd("ImportMeshLOD"), TEXT("") );
			FileMenu->Append( IDM_SME_REMOVELOD, *LocalizeUnrealEd("RemoveLOD"), TEXT("") );
			FileMenu->Append( IDM_SME_GENERATELOD, *LocalizeUnrealEd("GenerateLOD"), TEXT("") );
			FileMenu->Append( IDM_SME_GENERATEUVS, *LocalizeUnrealEd("GenerateUVs"), TEXT("") );
			//FileMenu->Append( IDM_SME_MERGESM, *LocalizeUnrealEd("MergeStaticMesh"), TEXT("") );
			FileMenu->AppendSeparator();
			FileMenu->Append( IDM_SME_CHANGEMESH, *LocalizeUnrealEd("ChangeMesh"), TEXT("") );
			Append( FileMenu, *LocalizeUnrealEd("MeshMenu") );
		}

		// View menu
		wxMenu* ViewMenu = new wxMenu();
		ViewMenu->AppendCheckItem( ID_SHOW_OPEN_EDGES, *LocalizeUnrealEd("OpenEdges"), TEXT("") );
		ViewMenu->AppendCheckItem( ID_SHOW_UV_OVERLAY, *LocalizeUnrealEd("UVOverlay"), TEXT("") );
		ViewMenu->AppendCheckItem( ID_SHOW_WIREFRAME, *LocalizeUnrealEd("Wireframe"), TEXT("") );
		ViewMenu->AppendCheckItem( ID_SHOW_BOUNDS, *LocalizeUnrealEd("Bounds"), TEXT("") );
		ViewMenu->AppendCheckItem( IDMN_COLLISION, *LocalizeUnrealEd("Collision"), TEXT("") );
		ViewMenu->AppendSeparator();
		ViewMenu->AppendCheckItem( ID_LOCK_CAMERA, *LocalizeUnrealEd("LockCamera"), TEXT("") );
		Append( ViewMenu, *LocalizeUnrealEd("View") );

		if ( !bInCookedPackage )
		{
			// Tool menu
			wxMenu* ToolMenu = new wxMenu();
			ToolMenu->Append( ID_SAVE_THUMBNAIL, *LocalizeUnrealEd("SaveThumbnailAngle"), TEXT("") );
			ToolMenu->AppendSeparator();
			ToolMenu->AppendCheckItem( IDM_SME_FRACTURETOOL, *LocalizeUnrealEd("FractureTool"), TEXT("") );
			Append( ToolMenu, *LocalizeUnrealEd("Tool") );

			// Collision menu
			wxMenu* CollisionMenu = new wxMenu();
			CollisionMenu->Append( IDMN_SME_COLLISION_6DOP, *LocalizeUnrealEd("6DOP"), TEXT("") );
			CollisionMenu->Append( IDMN_SME_COLLISION_10DOPX, *LocalizeUnrealEd("10DOPX"), TEXT("") );
			CollisionMenu->Append( IDMN_SME_COLLISION_10DOPY, *LocalizeUnrealEd("10DOPY"), TEXT("") );
			CollisionMenu->Append( IDMN_SME_COLLISION_10DOPZ, *LocalizeUnrealEd("10DOPZ"), TEXT("") );
			CollisionMenu->Append( IDMN_SME_COLLISION_18DOP, *LocalizeUnrealEd("18DOP"), TEXT("") );
			CollisionMenu->Append( IDMN_SME_COLLISION_26DOP, *LocalizeUnrealEd("26DOP"), TEXT("") );
			CollisionMenu->Append( IDMN_SME_COLLISION_SPHERE, *LocalizeUnrealEd("SphereSimplifiedCollision"), TEXT("") );
			CollisionMenu->Append( IDMN_SME_COLLISION_DECOMP, *LocalizeUnrealEd("AutoConvexCollision"), TEXT("") );
			CollisionMenu->Append( IDMN_SME_COLLISION_REMOVE, *LocalizeUnrealEd("RemoveCollision"), TEXT("") );	
			CollisionMenu->Append( IDMN_SME_COLLISION_CONVERTBOX, *LocalizeUnrealEd("ConvertBoxCollision"), TEXT("") );	
			Append( CollisionMenu, *LocalizeUnrealEd("Collision") );
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WxStaticMeshEditorBar
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class WxStaticMeshEditorBar : public wxToolBar
{
public:
	WxStaticMeshEditorBar( wxWindow* InParent, UStaticMesh* InStaticMesh, wxWindowID InID )
		: wxToolBar( InParent, InID, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_FLAT | wxTB_NODIVIDER )
	{
		// Bitmaps

		OpenEdgesB.Load( TEXT("OpenEdges") );
		UVOverlayB.Load( TEXT("UVOverlay") );
		WireframeB.Load( TEXT("Wireframe") );
		BoundsB.Load( TEXT("Bounds") );
		CollisionB.Load( TEXT("Collision") );
		LockB.Load( TEXT("Lock") );
		CameraB.Load( TEXT("Camera") );
		FracToolB.Load(TEXT("FracTool"));

		LODAutoB.Load(TEXT("AnimTree_LOD_Auto"));
		LODBaseB.Load(TEXT("AnimTree_LOD_Base"));
		LOD1B.Load(TEXT("AnimTree_LOD_1"));
		LOD2B.Load(TEXT("AnimTree_LOD_2"));
		LOD3B.Load(TEXT("AnimTree_LOD_3"));


		// Set up the ToolBar

		AddCheckTool( ID_SHOW_OPEN_EDGES, TEXT(""), OpenEdgesB, OpenEdgesB, *LocalizeUnrealEd("ToolTip_68") );
		AddCheckTool( ID_SHOW_UV_OVERLAY, TEXT(""), UVOverlayB, UVOverlayB, *LocalizeUnrealEd("ToolTip_UVOverlay") );
		AddCheckTool( ID_SHOW_WIREFRAME, TEXT(""), WireframeB, WireframeB, *LocalizeUnrealEd("ToolTip_69") );
		AddCheckTool( ID_SHOW_BOUNDS, TEXT(""), BoundsB, BoundsB, *LocalizeUnrealEd("ToolTip_70") );
		AddCheckTool( IDMN_COLLISION, TEXT(""), CollisionB, CollisionB, *LocalizeUnrealEd("ToolTip_71") );
		AddSeparator();
		AddCheckTool( ID_LOCK_CAMERA, TEXT(""), LockB, LockB, *LocalizeUnrealEd("ToolTip_72") );

		// Disallow thumbnail saving on cooked content.
		if ( !(InStaticMesh->GetOutermost()->PackageFlags & PKG_Cooked) )
		{
			AddSeparator();
			AddTool( ID_SAVE_THUMBNAIL, TEXT(""), CameraB, *LocalizeUnrealEd("ToolTip_73") );
		}

		AddSeparator();
		AddCheckTool( IDM_SME_FRACTURETOOL, TEXT(""), FracToolB, FracToolB, *LocalizeUnrealEd("FractureTool") );
		AddSeparator();
		wxStaticText* LODLabel = new wxStaticText(this, -1 ,*LocalizeUnrealEd("LODLabel") );
		AddControl(LODLabel);
		AddRadioTool(IDM_SME_LOD_AUTO, *LocalizeUnrealEd("SetLODAuto"), LODAutoB, wxNullBitmap, *LocalizeUnrealEd("SetLODAuto") );
		AddRadioTool(IDM_SME_LOD_BASE, *LocalizeUnrealEd("ForceLODBaseMesh"), LODBaseB, wxNullBitmap, *LocalizeUnrealEd("ForceLODBaseMesh") );
		AddRadioTool(IDM_SME_LOD_1, *LocalizeUnrealEd("ForceLOD1"), LOD1B, wxNullBitmap, *LocalizeUnrealEd("ForceLOD1") );
		AddRadioTool(IDM_SME_LOD_2, *LocalizeUnrealEd("ForceLOD2"), LOD2B, wxNullBitmap, *LocalizeUnrealEd("ForceLOD2") );
		AddRadioTool(IDM_SME_LOD_3, *LocalizeUnrealEd("ForceLOD3"), LOD3B, wxNullBitmap, *LocalizeUnrealEd("ForceLOD3") );
		ToggleTool(IDM_SME_LOD_AUTO, true);

		Realize();
	}

private:
	WxMaskedBitmap OpenEdgesB, UVOverlayB, WireframeB, BoundsB, CollisionB, LockB, CameraB, FracToolB;
	WxMaskedBitmap LODAutoB, LODBaseB, LOD1B, LOD2B, LOD3B;
};

/** Hit proxy for a chunk. */
struct HSMEChunkProxy : public HHitProxy
{
	DECLARE_HIT_PROXY( HSMEChunkProxy, HHitProxy );

	INT	 ChunkIndex;

	HSMEChunkProxy(INT InChunkIndex)
		:	HHitProxy()
		,	ChunkIndex( InChunkIndex )
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	FStaticMeshEditorViewportClient
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FStaticMeshEditorViewportClient: public FEditorLevelViewportClient, private FPreviewScene
{
	class WxStaticMeshEditor*	StaticMeshEditor;
	UStaticMeshComponent*	StaticMeshComponent;
	UEditorComponent*			EditorComponent;

	UMaterialInterface*  SolidChunkMaterial;
	UMaterialInterface*  FracMeshMaterial;

	UStaticMeshComponent*	CorePreviewComponent;

	// Used for when the viewport is rotating around a locked position

	UBOOL bLock;
	FRotator LockRot;
	FVector LockLocation;

	INT			DistanceDragged;

	UBOOL		bManipulating;
	EAxis		ManipulateAxis;
	FLOAT		DragDirX;
	FLOAT		DragDirY;
	FVector		WorldManDir;
	FVector		LocalManDir;
	FLOAT		ManipulateAccumulator;

	// Constructor.

	FStaticMeshEditorViewportClient( class WxStaticMeshEditor* InStaticMeshEditor );

	void UpdatePreviewComponent();

	/** Turn on/off the core preview mesh */
	void SetCorePreview(UStaticMesh* CoreMesh);

	/** Move the core preview mesh */
	void UpdateCorePreview();


	// FEditorLevelViewportClient interface.
	virtual FSceneInterface* GetScene() { return FPreviewScene::GetScene(); }
	virtual FLinearColor GetBackgroundColor() { return FColor(64,64,64); }

	virtual void Draw(FViewport* Viewport,FCanvas* Canvas);
	virtual void Draw(const FSceneView* View,FPrimitiveDrawInterface* PDI);
	virtual void DrawUVs(FViewport* Viewport,FCanvas* Canvas);
	virtual UBOOL InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed = 1.f,UBOOL bGamepad=FALSE);
	virtual UBOOL InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime, UBOOL bGamepad=FALSE);
	FVector2D ClampUVRange(float U, float V);

	virtual void Serialize(FArchive& Ar) { Ar << Input << (FPreviewScene&)*this; }
};


FStaticMeshEditorViewportClient::FStaticMeshEditorViewportClient( WxStaticMeshEditor* InStaticMeshEditor )
	:	StaticMeshEditor(InStaticMeshEditor)
	,	StaticMeshComponent(NULL)
	,	bLock( TRUE )
	,	CorePreviewComponent(NULL)
{
	// Disable widget and grid rendering in this viewport.
	ShowFlags &= ~SHOW_ModeWidgets;
	ShowFlags &= ~SHOW_Grid;

	EditorComponent = ConstructObject<UEditorComponent>(UEditorComponent::StaticClass());
	EditorComponent->bDrawPivot = FALSE;
	EditorComponent->bDrawGrid = FALSE;
	FPreviewScene::AddComponent(EditorComponent,FMatrix::Identity);

	UpdatePreviewComponent();

	bLock = TRUE;
	bDrawAxes = FALSE;
	NearPlane = 1.0f;
	bAllowMayaCam = FALSE;//TRUE;
	SetViewLocationForOrbiting( FVector(0.f,0.f,0.f) );

	DistanceDragged = 0;

	SolidChunkMaterial = LoadObject<UMaterialInterface>(NULL, TEXT("EditorMaterials.Frac_ElementMaterial"), NULL, LOAD_None, NULL);
	check(SolidChunkMaterial);

	FracMeshMaterial = LoadObject<UMaterialInterface>(NULL, TEXT("EditorMaterials.PhAT_NoCollisionMaterial"), NULL, LOAD_None, NULL);
	check(FracMeshMaterial);

	bManipulating = false;
	ManipulateAxis = AXIS_None;
	DragDirX = 0.f;
	DragDirY = 0.f;
	WorldManDir = FVector(0.f);
	LocalManDir = FVector(0.f);
	ManipulateAccumulator = 0.f;
}

void FStaticMeshEditorViewportClient::UpdatePreviewComponent()
{
	if (StaticMeshComponent)
	{
		FPreviewScene::RemoveComponent(StaticMeshComponent);
		StaticMeshComponent = NULL;
	}

	UFracturedStaticMesh* FracturedMesh = Cast<UFracturedStaticMesh>(StaticMeshEditor->StaticMesh);

	if (FracturedMesh)
	{
		StaticMeshComponent = ConstructObject<UFracturedStaticMeshComponent>(UFracturedStaticMeshComponent::StaticClass());
	}
	else 
	{
		StaticMeshComponent = ConstructObject<UStaticMeshComponent>(UStaticMeshComponent::StaticClass());
	}

	StaticMeshComponent->SetStaticMesh(StaticMeshEditor->StaticMesh);
	StaticMeshComponent->bUsePrecomputedShadows = FALSE;
	FPreviewScene::AddComponent(StaticMeshComponent,FMatrix::Identity);

	ViewLocation = -FVector(0,StaticMeshEditor->StaticMesh->Bounds.SphereRadius / (75.0f * (FLOAT)PI / 360.0f),0);
	ViewRotation = FRotator(0,16384,0);

	LockLocation = FVector(0,StaticMeshEditor->StaticMesh->ThumbnailDistance,0);
	LockRot = StaticMeshEditor->StaticMesh->ThumbnailAngle;

	const FMatrix LockMatrix = FRotationMatrix( FRotator(0,LockRot.Yaw,0) ) * FRotationMatrix( FRotator(0,0,LockRot.Pitch) ) * FTranslationMatrix( LockLocation );
	StaticMeshComponent->ConditionalUpdateTransform(FTranslationMatrix(-StaticMeshEditor->StaticMesh->Bounds.Origin) * LockMatrix);
	UpdateCorePreview();
}

/** Turn on/off the core preview mesh */
void FStaticMeshEditorViewportClient::SetCorePreview(UStaticMesh* CoreMesh)
{
	if(CoreMesh && !CorePreviewComponent)
	{
		CorePreviewComponent = ConstructObject<UStaticMeshComponent>(UStaticMeshComponent::StaticClass());
		CorePreviewComponent->SetStaticMesh(CoreMesh);
		CorePreviewComponent->bUsePrecomputedShadows = FALSE;
		CorePreviewComponent->WireframeColor = FColor(255,255,255);
		FPreviewScene::AddComponent(CorePreviewComponent, FMatrix::Identity);
		UpdateCorePreview();
	}
	else if(!CoreMesh && CorePreviewComponent)
	{
		FPreviewScene::RemoveComponent(CorePreviewComponent);
		CorePreviewComponent = NULL;
	}
}

/** Move the core preview mesh */
void FStaticMeshEditorViewportClient::UpdateCorePreview()
{
	if(CorePreviewComponent)
	{
		FMatrix TM = FScaleRotationTranslationMatrix(StaticMeshEditor->CurrentCoreScale3D*StaticMeshEditor->CurrentCoreScale, StaticMeshEditor->CurrentCoreRotation, StaticMeshEditor->CurrentCoreOffset);
		CorePreviewComponent->ConditionalUpdateTransform(TM * StaticMeshComponent->LocalToWorld);
	}
}

void FStaticMeshEditorViewportClient::Draw(FViewport* Viewport,FCanvas* Canvas)
{
	FEditorLevelViewportClient::Draw(Viewport,Canvas);
	INT YPos = 6;

	FSceneViewFamilyContext ViewFamily(Viewport,GetScene(),ShowFlags,GWorld->GetTimeSeconds(),GWorld->GetDeltaSeconds(),GWorld->GetRealTimeSeconds(),NULL);
	FSceneView* View = CalcSceneView(&ViewFamily);

	const INT HalfX = Viewport->GetSizeX()/2;
	const INT HalfY = Viewport->GetSizeY()/2;

	DrawShadowedString(Canvas,
		6,
		YPos,
		*FString::Printf(LocalizeSecure(LocalizeUnrealEd("Triangles_F"),StaticMeshEditor->NumTriangles)),
		GEngine->SmallFont,
		FLinearColor::White
		);
	YPos += 18;

	DrawShadowedString(Canvas,
		6,
		YPos,
		*FString::Printf(LocalizeSecure(LocalizeUnrealEd("Vertices_F"),StaticMeshEditor->NumVertices)),
		GEngine->SmallFont,
		FLinearColor::White
		);
	YPos += 18;

	DrawShadowedString(Canvas,
		6,
		YPos,
		*FString::Printf(LocalizeSecure(LocalizeUnrealEd("UVChannels_F"),StaticMeshEditor->NumUVChannels)),
		GEngine->SmallFont,
		FLinearColor::White
		);
	YPos += 18;

	// Show the number of collision primitives if we are drawing collision.
	if((ShowFlags & SHOW_Collision) && StaticMeshEditor->StaticMesh->BodySetup)
	{
		DrawShadowedString(Canvas,
			6,
			YPos,
			*FString::Printf(LocalizeSecure(LocalizeUnrealEd("NumPrimitives_F"),StaticMeshEditor->StaticMesh->BodySetup->AggGeom.GetElementCount())),
			GEngine->SmallFont,
			FLinearColor::White
			);
		YPos += 18;
	}

	// FSM info
	if(StaticMeshEditor->FractureOptions)
	{
		FColor FracInfoColor(255,255,200);
		YPos += 5;

		// Show graphics info - number of actually drawn triangles, num graphics chunks
		UFracturedStaticMeshComponent* FracturedMeshComponent = Cast<UFracturedStaticMeshComponent>(StaticMeshComponent);
		if(FracturedMeshComponent)
		{
			INT ShowNum = FracturedMeshComponent->GetNumFragments();
			if(FracturedMeshComponent->GetCoreFragmentIndex() != INDEX_NONE)
			{
				ShowNum--;
			}

			DrawShadowedString(Canvas,
				6,
				YPos,
				*FString::Printf(LocalizeSecure(LocalizeUnrealEd("FracMeshInfo_F"), ShowNum, FracturedMeshComponent->GetNumVisibleTriangles())),
				GEngine->SmallFont,
				FracInfoColor
				);
			YPos += 18;
		}

		// Show tool info - num chunks in slice pattern
		FString ToolInfo = FString::Printf(LocalizeSecure(LocalizeUnrealEd("FracToolInfo_F"), StaticMeshEditor->FractureChunks.Num()));

		// Show if we have not yet applied slice pattern to mesh.
		if(StaticMeshEditor->FinalToToolMap.Num() == 0 && StaticMeshEditor->FractureChunks.Num() > 0)
		{
			ToolInfo += LocalizeUnrealEd("NeedsSlicing");
		}

		DrawShadowedString(Canvas,
			6,
			YPos,
			*ToolInfo,
			GEngine->SmallFont,
			FracInfoColor
			);
		YPos += 18;


		// Draw fracture lines if open.
		if(	StaticMeshEditor->FractureOptions->bShowCuts && 
			!StaticMeshEditor->bAdjustingCore && 
			StaticMeshEditor->SelectedChunks.Num() > 0 )
		{
			FVoronoiRegion& SelectedRegion = StaticMeshEditor->FractureChunks(StaticMeshEditor->SelectedChunks(0));

			// Draw neighbour connection area
			for(INT NIdx=0; NIdx<SelectedRegion.Neighbours.Num(); NIdx++)
			{
				if(SelectedRegion.Neighbours(NIdx) != INDEX_NONE)
				{
					FVector LocalNPos = StaticMeshEditor->FractureCenters(SelectedRegion.Neighbours(NIdx));
					FVector WorldNPos = StaticMeshComponent->LocalToWorld.TransformFVector(LocalNPos);
					const FPlane Proj = View->Project(WorldNPos);
					if(Proj.W > 0.f)
					{
						const INT XPos = HalfX + ( HalfX * Proj.X );
						const INT YPos = HalfY + ( HalfY * (Proj.Y * -1) );

						FLOAT NeighbourDim = SelectedRegion.NeighbourDims(NIdx);
						DrawString(Canvas, XPos, YPos, *FString::Printf(TEXT("%3.1f"), NeighbourDim), GEngine->SmallFont, FColor(255,196,196));
					}
				}
			}
		}
	}

	FStaticMeshEditorViewportClient* vp = (FStaticMeshEditorViewportClient*)Viewport->GetClient();
	if( vp->bLock )
	{
		FRotator DrawRotation( vp->LockRot.Pitch, -(vp->LockRot.Yaw - 16384), vp->LockRot.Roll );
		vp->DrawAxes( Viewport, Canvas, &DrawRotation );
	}

	if (StaticMeshEditor->DrawUVOverlay)
	{
		DrawUVs(Viewport, Canvas);
	}
}

/** 3D draw function, use to draw debug fracture ovelay. */
void FStaticMeshEditorViewportClient::Draw(const FSceneView* View,FPrimitiveDrawInterface* PDI)
{
	FEditorLevelViewportClient::Draw(View, PDI);

	// Get pointer to FSMC
	UFracturedStaticMeshComponent* FracturedMeshComponent = Cast<UFracturedStaticMeshComponent>(StaticMeshComponent);

	// Get pointer to FSM
	UFracturedStaticMesh* FracMesh = NULL;
	if(FracturedMeshComponent)
	{
		 FracMesh = Cast<UFracturedStaticMesh>(StaticMeshEditor->StaticMesh);
	}

	// Update chunk visibilities if slider was moved.
	if (StaticMeshEditor->bViewChunkChanged)
	{
		StaticMeshEditor->bViewChunkChanged = FALSE;
		if (FracturedMeshComponent)
		{
			TArray<BYTE> VisibleFragments = FracturedMeshComponent->GetVisibleFragments();
			if (StaticMeshEditor->CurrentChunkViewMode == ECVM_ViewOnly)
			{
				for (INT i = 0; i < VisibleFragments.Num(); i++)
				{
					VisibleFragments(i) = 0;
				}

				if (StaticMeshEditor->CurrentViewChunk >= 0 && StaticMeshEditor->CurrentViewChunk < VisibleFragments.Num())
				{
					VisibleFragments(StaticMeshEditor->CurrentViewChunk) = 1;
				}
			}
			else if (StaticMeshEditor->CurrentChunkViewMode == ECVM_ViewAllBut)
			{
				for (INT i = 0; i < VisibleFragments.Num(); i++)
				{
					VisibleFragments(i) = 1;
				}

				if (StaticMeshEditor->CurrentViewChunk >= 0 && StaticMeshEditor->CurrentViewChunk < VisibleFragments.Num())
				{
					VisibleFragments(StaticMeshEditor->CurrentViewChunk) = 0;
				}
			}
			else if (StaticMeshEditor->CurrentChunkViewMode == ECVM_ViewUpTo)
			{
				for (INT i = 0; i < VisibleFragments.Num(); i++)
				{
					if (i <= StaticMeshEditor->CurrentViewChunk)
					{
						VisibleFragments(i) = 1;
					}
					else
					{
						VisibleFragments(i) = 0;
					}
				}
			}

			// Make sure core is visible/invisible as desired
			if(FracMesh && StaticMeshEditor->FractureOptions)
			{
				INT CoreIndex = FracMesh->GetCoreFragmentIndex();
				if(CoreIndex != INDEX_NONE)
				{
					check(CoreIndex < VisibleFragments.Num());
					VisibleFragments(CoreIndex) = (StaticMeshEditor->FractureOptions->bShowCore) ? 1 : 0;
				}
			}

			FracturedMeshComponent->SetVisibleFragments(VisibleFragments);
		}
	}

	// Draw fracture lines if open.
	if(	StaticMeshEditor->FractureOptions &&
		StaticMeshEditor->FractureOptions->bShowCuts && 
		!StaticMeshEditor->bAdjustingCore )
	{
		// Get set of visible final graphics chunk visibility.
		TArray<BYTE> VisibleFragments;
		if (FracturedMeshComponent)
		{
			VisibleFragments = FracturedMeshComponent->GetVisibleFragments();
		}

		for(INT i=0; i<StaticMeshEditor->FractureChunks.Num(); i++)
		{
			UBOOL bIsSelected = StaticMeshEditor->SelectedChunks.ContainsItem(i);

			// If desired, see if the corresponding graphics chunk is visible
			INT GraphicFragmentIndex = StaticMeshEditor->FinalToToolMap.FindItemIndex(i);
			if( StaticMeshEditor->FractureOptions->bShowOnlyVisibleCuts && 
				!bIsSelected &&
				StaticMeshEditor->FinalToToolMap.Num() > 0 && 
				((GraphicFragmentIndex == INDEX_NONE) || (VisibleFragments(GraphicFragmentIndex) == 0)) )
			{
				continue;
			}

			FVoronoiRegion& Region = StaticMeshEditor->FractureChunks(i);

			FColor UseColor = StaticMeshEditor->GetChunkColor(i);
			UseColor.A = 128;

			PDI->SetHitProxy( new HSMEChunkProxy(i) );

			Region.ConvexElem.DrawElemWire(PDI, StaticMeshComponent->LocalToWorld, FVector(1,1,1), UseColor);

			// If selected, or we want to draw all chunks as solid, draw solid mesh
			if(StaticMeshEditor->FractureOptions->bShowCutsSolid || bIsSelected)
			{
				// Get vertex/index buffer from elem
				TArray<FDynamicMeshVertex> ElemVertexBuffer;
				TArray<INT> ElemIndexBuffer;
				Region.ConvexElem.AddCachedSolidConvexGeom(ElemVertexBuffer, ElemIndexBuffer, UseColor);

				// Draw it
				{
					FDynamicMeshBuilder MeshBuilder;
					MeshBuilder.AddVertices(ElemVertexBuffer);
					MeshBuilder.AddTriangles(ElemIndexBuffer);
					MeshBuilder.Draw(PDI, StaticMeshComponent->LocalToWorld, SolidChunkMaterial->GetRenderProxy(0), SDPG_World);
				}
			}

			FVector WCenter = StaticMeshComponent->LocalToWorld.TransformFVector( StaticMeshEditor->FractureCenters(i) );
			DrawWireStar(PDI, WCenter, 0.5f, UseColor, SDPG_World);
		}

		for(INT i=0; i<StaticMeshEditor->FractureCenters.Num(); i++)
		{
			UBOOL bIsSelected = StaticMeshEditor->SelectedChunks.ContainsItem(i);

			// If desired, see if the corresponding graphics chunk is visible
			INT GraphicFragmentIndex = StaticMeshEditor->FinalToToolMap.FindItemIndex(i);
			if( StaticMeshEditor->FractureOptions->bShowOnlyVisibleCuts && 
				!bIsSelected &&
				StaticMeshEditor->FinalToToolMap.Num() > 0 && 
				((GraphicFragmentIndex == INDEX_NONE) || (VisibleFragments(GraphicFragmentIndex) == 0)) )
			{
				continue;
			}

			FVector WCenter = StaticMeshComponent->LocalToWorld.TransformFVector( StaticMeshEditor->FractureCenters(i) );
			FColor UseColor = StaticMeshEditor->GetChunkColor(i);
			PDI->SetHitProxy( new HSMEChunkProxy(i) );
			DrawWireStar(PDI, WCenter, 0.5f, UseColor, SDPG_World);
			PDI->SetHitProxy( NULL );
		}

		if(StaticMeshEditor->SelectedChunks.Num() > 0)
		{
			for(INT SelIndex = 0; SelIndex<StaticMeshEditor->SelectedChunks.Num(); SelIndex++)
			{
				INT SelectedChunk = StaticMeshEditor->SelectedChunks(SelIndex);

				check(SelectedChunk < StaticMeshEditor->FractureCenters.Num());

				// Draw dragging widget only for first chunk in selection set
				if(SelIndex == 0)
				{
					FMatrix WidgetMatrix = FTranslationMatrix(StaticMeshEditor->FractureCenters(SelectedChunk)) * StaticMeshComponent->LocalToWorld;
					FUnrealEdUtils::DrawWidget(View, PDI, WidgetMatrix, 0, 0, ManipulateAxis, WMM_Translate);
				}

				// Draw selected chunk as solid geometry
				FVoronoiRegion& SelectedRegion = StaticMeshEditor->FractureChunks(SelectedChunk);

				// Draw neighbour/exterior normal info just for first chunk in selection
				if(SelIndex == 0)
				{
					// Draw neighbour info.
					FVector Start = StaticMeshEditor->FractureCenters(SelectedChunk);
					for(INT NIdx=0; NIdx<SelectedRegion.Neighbours.Num(); NIdx++)
					{
						if(SelectedRegion.Neighbours(NIdx) != INDEX_NONE)
						{
							FVector End = StaticMeshEditor->FractureCenters(SelectedRegion.Neighbours(NIdx));
							PDI->DrawLine(	StaticMeshComponent->LocalToWorld.TransformFVector(Start), 
								StaticMeshComponent->LocalToWorld.TransformFVector(End), 
								FColor(255,255,255), SDPG_World );
						}
					}

					// Draw average exterior normal as vector from chunk center
					if(FracMesh && StaticMeshEditor->FinalToToolMap.Num() > 0)
					{
						INT FinalChunkIndex = StaticMeshEditor->FinalToToolMap.FindItemIndex(SelectedChunk);
						if(FinalChunkIndex != INDEX_NONE)
						{
							FVector WorldStart = StaticMeshComponent->LocalToWorld.TransformFVector(Start);

							FVector LocalNormal = FracMesh->GetFragmentAverageExteriorNormal(FinalChunkIndex);
							FVector WorldNormal = StaticMeshComponent->LocalToWorld.TransformNormal(LocalNormal).SafeNormal();

							FLOAT LineLen = FracMesh->Bounds.SphereRadius * 0.35f;

							PDI->DrawLine(WorldStart, WorldStart+(LineLen*WorldNormal), FColor(255,0,255), SDPG_World);
						}
					}
				}
			}
		}
	}

	// Draw core adjustment widget
	if(StaticMeshEditor->bAdjustingCore)
	{	
		FMatrix WidgetMatrix = FRotationTranslationMatrix(StaticMeshEditor->CurrentCoreRotation, StaticMeshEditor->CurrentCoreOffset) * StaticMeshComponent->LocalToWorld;
		FUnrealEdUtils::DrawWidget(View, PDI, WidgetMatrix, 0, 0, ManipulateAxis, StaticMeshEditor->CoreEditMode);
	}

	// Draw open edges if that option is enabled
	if( StaticMeshEditor->DrawOpenEdges )
	{
		UStaticMesh* StaticMesh = StaticMeshEditor->StaticMesh;
		FStaticMeshRenderData& LODModel = StaticMesh->LODModels( 0 );
		for( INT EdgeIndex = 0;EdgeIndex < LODModel.Edges.Num(); ++EdgeIndex )
		{
			FMeshEdge& Edge = LODModel.Edges( EdgeIndex );
			if( Edge.Faces[ 1 ] != INDEX_NONE && LODModel.ShadowTriangleDoubleSided( Edge.Faces[ 0 ] ) )
			{
				PDI->DrawLine(
					StaticMeshComponent->LocalToWorld.TransformFVector( LODModel.PositionVertexBuffer.VertexPosition( Edge.Vertices[ 0 ] ) ),
					StaticMeshComponent->LocalToWorld.TransformFVector( LODModel.PositionVertexBuffer.VertexPosition( Edge.Vertices[ 1 ] ) ),
					FColor( 128, 0, 0 ),
					SDPG_World );
			}

			if( Edge.Faces[ 1 ] == INDEX_NONE )
			{
				PDI->DrawLine(
					StaticMeshComponent->LocalToWorld.TransformFVector( LODModel.PositionVertexBuffer.VertexPosition( Edge.Vertices[ 0 ] ) ),
					StaticMeshComponent->LocalToWorld.TransformFVector( LODModel.PositionVertexBuffer.VertexPosition( Edge.Vertices[ 1 ] ) ),
					FColor( 255, 255, 0 ),
					SDPG_World );
			}
		}
	}
}

void FStaticMeshEditorViewportClient::DrawUVs(FViewport* Viewport,FCanvas* Canvas)
{
	//use the overriden LOD level
	UINT LODLevel = Clamp(StaticMeshComponent->ForcedLodModel - 1, 0, StaticMeshEditor->StaticMesh->LODModels.Num() - 1);

	//draw a string showing what UV channel and LOD is being displayed
	DrawShadowedString(Canvas,
		6,
		60,
		*FString::Printf(LocalizeSecure(LocalizeUnrealEd("UVOverlay_F"),StaticMeshEditor->StaticMesh->LightMapCoordinateIndex,LODLevel)),
		GEngine->SmallFont,
		FLinearColor::White
		);

	FStaticMeshRenderData* RenderData = &StaticMeshEditor->StaticMesh->LODModels(LODLevel);
	if (RenderData && UINT(StaticMeshEditor->StaticMesh->LightMapCoordinateIndex) < RenderData->VertexBuffer.GetNumTexCoords())
	{
		//calculate scaling
		const UINT BorderWidth = 5;
		const UINT MinY = 96 + BorderWidth;
		const UINT MinX = BorderWidth;
		const FVector2D Origin(MinX, MinY);
		const UINT SizeOfSquare = Min(Viewport->GetSizeX() - MinX, Viewport->GetSizeY() - MinY) - BorderWidth;

		//draw texture border
		DrawLine2D(Canvas, Origin, FVector2D(MinX + SizeOfSquare, MinY), FLinearColor::White);
		DrawLine2D(Canvas, FVector2D(MinX + SizeOfSquare, MinY), FVector2D(MinX + SizeOfSquare, MinY + SizeOfSquare), FLinearColor::White);
		DrawLine2D(Canvas, FVector2D(MinX + SizeOfSquare, MinY + SizeOfSquare), FVector2D(MinX, MinY + SizeOfSquare), FLinearColor::White);
		DrawLine2D(Canvas, FVector2D(MinX, MinY + SizeOfSquare), Origin, FLinearColor::White);   

		//draw triangles
		WORD* Indices = (WORD*)RenderData->IndexBuffer.Indices.GetData();
		UINT NumIndices = RenderData->IndexBuffer.Indices.Num();
		for (UINT i = 0; i < NumIndices - 2; i += 3)
		{
			FVector2D UV1(RenderData->VertexBuffer.GetVertexUV(Indices[i], StaticMeshEditor->StaticMesh->LightMapCoordinateIndex)); 
			FVector2D UV2(RenderData->VertexBuffer.GetVertexUV(Indices[i + 1], StaticMeshEditor->StaticMesh->LightMapCoordinateIndex)); 
			FVector2D UV3(RenderData->VertexBuffer.GetVertexUV(Indices[i + 2], StaticMeshEditor->StaticMesh->LightMapCoordinateIndex)); 

			UV1 = ClampUVRange(UV1.X, UV1.Y) * SizeOfSquare + Origin;
			UV2 = ClampUVRange(UV2.X, UV2.Y) * SizeOfSquare + Origin;
			UV3 = ClampUVRange(UV3.X, UV3.Y) * SizeOfSquare + Origin;

			DrawLine2D(Canvas, UV1, UV2, FLinearColor::Black);
			DrawLine2D(Canvas, UV2, UV3, FLinearColor::Black);
			DrawLine2D(Canvas, UV3, UV1, FLinearColor::Black);
		}
	}
}

FVector2D FStaticMeshEditorViewportClient::ClampUVRange(float U, float V)
{
	// This handles both the + and - values correctly by putting all values between positive 0 and 1
	if(abs(U) > 1.0f)
	{
		U -= floorf(U);
	}

	if(abs(V) > 1.0f)
	{
		V -= floorf(V);
	}

	return FVector2D(U, V);
}

UBOOL FStaticMeshEditorViewportClient::InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed,UBOOL Gamepad)
{
	// Hide and lock mouse cursor if we're capturing mouse input
	Viewport->ShowCursor( !Viewport->HasMouseCapture() );
	Viewport->LockMouseToWindow( Viewport->HasMouseCapture() );

	const INT HitX = Viewport->GetMouseX();
	const INT HitY = Viewport->GetMouseY();
	const UBOOL bAltDown = Viewport->KeyState(KEY_LeftAlt) || Viewport->KeyState(KEY_RightAlt);
	const UBOOL bCtrlDown = Viewport->KeyState(KEY_LeftControl) || Viewport->KeyState(KEY_RightControl);

	if( Key == KEY_LeftMouseButton )
	{
		HHitProxy* HitResult = Viewport->GetHitProxy(HitX,HitY);

		// ON PRESS - Check for hitting a widget
		if(Event == IE_Pressed)
		{
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

					// If holding ALT when starting to manipulate, duplicate point.
					if( bAltDown && StaticMeshEditor->SelectedChunks.Num() > 0 )
					{
						for(INT SelIndex = 0; SelIndex<StaticMeshEditor->SelectedChunks.Num(); SelIndex++)
						{
							INT SelectedChunk = StaticMeshEditor->SelectedChunks(SelIndex);
							FVector SelectedCenter = StaticMeshEditor->FractureCenters(SelectedChunk);
							StaticMeshEditor->FractureCenters.AddItem(SelectedCenter);
						}
					}

					bManipulating = true;
					ManipulateAccumulator = 0.f;
					Viewport->Invalidate();
				}
			}

			DistanceDragged = 0;
		}
		// ON RELEASE - check for hitting 
		else if(Event == IE_Released)
		{
			// Don't check if we were using widget or dragged at all
			if(!bManipulating && DistanceDragged < 4)
			{
				if(HitResult)
				{
					if(HitResult->IsA(HSMEChunkProxy::StaticGetType()))
					{
						HSMEChunkProxy* ChunkProxy = (HSMEChunkProxy*)HitResult;
						
						// If ctrl is down, toggle selection
						if(bCtrlDown)
						{
							if(StaticMeshEditor->SelectedChunks.ContainsItem(ChunkProxy->ChunkIndex))
							{
								StaticMeshEditor->RemoveChunkFromSelection(ChunkProxy->ChunkIndex);
							}
							else
							{
								StaticMeshEditor->AddChunkToSelection(ChunkProxy->ChunkIndex);
							}
						}
						// If not, clear selection and add just this
						else
						{
							StaticMeshEditor->ClearChunkSelection();
							StaticMeshEditor->AddChunkToSelection(ChunkProxy->ChunkIndex);
						}
					}
				}
				else
				{
					if(!bCtrlDown)
					{
						StaticMeshEditor->ClearChunkSelection();
					}
				}
			}

			// Stop manipulating and update regions.
			if(bManipulating && StaticMeshEditor->FractureOptions)
			{
				bManipulating = FALSE;
				ManipulateAccumulator = 0.f;
				ManipulateAxis = AXIS_None;

				// If moving chunks, regen regions now.
				if(!StaticMeshEditor->bAdjustingCore)
				{
					StaticMeshEditor->FractureOptions->RegenRegions(FALSE);
				}
			}

			Viewport->Invalidate();
		}
	}

	if( Event == IE_Pressed) 
	{
		if( Key == KEY_LeftMouseButton || Key == KEY_RightMouseButton )
		{
			MouseDeltaTracker->StartTracking( this, HitX, HitY );
		}
		else if(Key == KEY_Delete)
		{
			// Make sure fracture tool is up, we have something valid selected, and more than one piece left.
			if( StaticMeshEditor->FractureOptions &&
				StaticMeshEditor->SelectedChunks.Num() > 0 &&
				StaticMeshEditor->FractureCenters.Num() > 1)
			{
				// Create new set of centers by iterating over current set, adding all that are _not_ in the selection set.
				TArray<FVector> NewCenters;
				for(INT i=0; i<StaticMeshEditor->FractureCenters.Num(); i++)
				{
					if(!StaticMeshEditor->SelectedChunks.ContainsItem(i))
					{
						NewCenters.AddItem(StaticMeshEditor->FractureCenters(i));
					}
				}
				StaticMeshEditor->FractureCenters = NewCenters;
				StaticMeshEditor->FractureOptions->RegenRegions(TRUE);
				// Clear selection
				StaticMeshEditor->ClearChunkSelection();
				Viewport->Invalidate();
			}
		}
		else if(Key == KEY_G)
		{
			StaticMeshEditor->GrowChunkSelection();
		}
		else if(Key == KEY_S)
		{
			StaticMeshEditor->ShrinkChunkSelection();
		}
		else if(Key == KEY_SpaceBar)
		{
			if(StaticMeshEditor->bAdjustingCore)
			{
				// Cycle editing mode
				if(StaticMeshEditor->CoreEditMode == WMM_Translate)
				{
					StaticMeshEditor->CoreEditMode = WMM_Rotate;
				}
				else if(StaticMeshEditor->CoreEditMode == WMM_Rotate)
				{
					StaticMeshEditor->CoreEditMode = WMM_Scale;
				}
				else
				{
					StaticMeshEditor->CoreEditMode = WMM_Translate;
				}
			}
		}
	}
	else if( Event == IE_Released )
	{
		if( Key == KEY_LeftMouseButton || Key == KEY_RightMouseButton )
		{
			MouseDeltaTracker->EndTracking( this );
		}
	}

	// Handle viewport screenshot.
	InputTakeScreenshot( Viewport, Key, Event );

	return TRUE;
	//return FEditorLevelViewportClient::InputKey( Viewport, ControllerId, Key, Event, AmountDepressed );
}

/** Util to clamp a vert to a box */
void ClampVertToBox(const FBox& Box, FVector& Vert)
{
	Vert.X = Clamp<FLOAT>(Vert.X, Box.Min.X, Box.Max.X)	;
	Vert.Y = Clamp<FLOAT>(Vert.Y, Box.Min.Y, Box.Max.Y)	;
	Vert.Z = Clamp<FLOAT>(Vert.Z, Box.Min.Z, Box.Max.Z)	;
}

UBOOL FStaticMeshEditorViewportClient::InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime, UBOOL bGamepad)
{
	if( Key == KEY_MouseX || Key == KEY_MouseY )
	{
		DistanceDragged += Abs(Delta);
	}

	// See if we are holding down the 'rotate light' key
	const UBOOL bLightMoveDown = Viewport->KeyState( KEY_L );
	const UBOOL bHoldingShift = Viewport->KeyState(KEY_RightShift) || Viewport->KeyState(KEY_LeftShift);

	// See if we are moving a widget.
	if(bManipulating)
	{
		// Look at which axis is being dragged and by how much
		FLOAT DragX = (Key == KEY_MouseX) ? Delta : 0.f;
		FLOAT DragY = (Key == KEY_MouseY) ? Delta : 0.f;
		FLOAT DragMag = (DragX * DragDirX) + (DragY * DragDirY);

		// Core editing
		if(StaticMeshEditor->bAdjustingCore)
		{
			// TRANSLATION //////////////////////////////////////////////////////////////////////////
			if(StaticMeshEditor->CoreEditMode == WMM_Translate)
			{
				ManipulateAccumulator += DragMag * StaticMeshEditor_TranslateSpeed;

				FLOAT ApplyMan = 0.f;

				if(GUnrealEd->Constraints.GridEnabled)
				{
					FLOAT LinSnap = GUnrealEd->Constraints.GetGridSize();

					while(ManipulateAccumulator > LinSnap)
					{
						ManipulateAccumulator -= LinSnap;
						ApplyMan += LinSnap;
					}

					while(ManipulateAccumulator < -LinSnap)
					{
						ManipulateAccumulator += LinSnap;
						ApplyMan -= LinSnap;
					}
				}
				else
				{
					ApplyMan = ManipulateAccumulator;
					ManipulateAccumulator = 0.f;
				}
			
				// Actually apply translation to core mesh
				FVector UseManDir = FRotationMatrix(StaticMeshEditor->CurrentCoreRotation).TransformNormal(LocalManDir);
				StaticMeshEditor->CurrentCoreOffset += (UseManDir * ApplyMan);
			}
			// ROTATION //////////////////////////////////////////////////////////////////////////
			else if(StaticMeshEditor->CoreEditMode == WMM_Rotate)
			{
				ManipulateAccumulator += DragMag * StaticMeshEditor_RotateSpeed;

				// Rotation to actually apply this frame
				FLOAT ApplyMan = 0.f;
				
				// Handle snap
				if(GUnrealEd->Constraints.RotGridEnabled)
				{
					FLOAT AngSnap = GUnrealEd->Constraints.RotGridSize.Euler().X * (PI / 180.f);

					while(ManipulateAccumulator > AngSnap)
					{
						ManipulateAccumulator -= AngSnap;
						ApplyMan += AngSnap;
					}

					while(ManipulateAccumulator < -AngSnap)
					{
						ManipulateAccumulator += AngSnap;
						ApplyMan -= AngSnap;
					}
				}
				// No snap - just apply rotation
				else
				{
					ApplyMan = ManipulateAccumulator;
					ManipulateAccumulator = 0.f;
				}

				FQuat CurrentQuat = StaticMeshEditor->CurrentCoreRotation.Quaternion();
				FQuat DeltaQuat(LocalManDir, ApplyMan);
				FQuat NewQuat = CurrentQuat * DeltaQuat;

				// Actually apply rotation to core mesh
				StaticMeshEditor->CurrentCoreRotation = FRotator(NewQuat);
			}
			// SCALING //////////////////////////////////////////////////////////////////////////
			else
			{
				FVector ManVec = (LocalManDir * DragMag * StaticMeshEditor_ScaleSpeed);
				// Hold shift -> scale all axes
				if(bHoldingShift)
				{
					// Use component with greatest magnitude (preserve sign)
					FLOAT M = (Abs(ManVec.GetMax()) > Abs(ManVec.GetMin())) ? ManVec.GetMax() : ManVec.GetMin();
					ManVec = FVector(M);
				}
				StaticMeshEditor->CurrentCoreScale3D += ManVec;
				// Make sure its not negative
				StaticMeshEditor->CurrentCoreScale3D.X = ::Max(0.f, StaticMeshEditor->CurrentCoreScale3D.X); 
				StaticMeshEditor->CurrentCoreScale3D.Y = ::Max(0.f, StaticMeshEditor->CurrentCoreScale3D.Y); 
				StaticMeshEditor->CurrentCoreScale3D.Z = ::Max(0.f, StaticMeshEditor->CurrentCoreScale3D.Z); 
			}

			UpdateCorePreview();
		}
		// Chunk editing
		else
		{
			if(StaticMeshEditor->SelectedChunks.Num() > 0)
			{
				for(INT SelIndex=0; SelIndex<StaticMeshEditor->SelectedChunks.Num(); SelIndex++)
				{
					INT SelectedChunk = StaticMeshEditor->SelectedChunks(SelIndex);
					StaticMeshEditor->FractureCenters(SelectedChunk) += (LocalManDir * DragMag * StaticMeshEditor_TranslateSpeed);

					// Make sure it doesn't go outside box
					FBox MeshBox = StaticMeshEditor->StaticMesh->Bounds.GetBox();
					ClampVertToBox(MeshBox, StaticMeshEditor->FractureCenters(SelectedChunk));
				}
			}
		}
	}
	// If so, use mouse movement to rotate light direction,
	else if( bLightMoveDown )
	{
		// Look at which axis is being dragged and by how much
		const FLOAT DragX = (Key == KEY_MouseX) ? Delta : 0.f;
		const FLOAT DragY = (Key == KEY_MouseY) ? Delta : 0.f;

		FRotator LightDir = GetLightDirection();

		LightDir.Yaw += -DragX * StaticMeshEditor_LightRotSpeed;
		LightDir.Pitch += -DragY * StaticMeshEditor_LightRotSpeed;

		SetLightDirection( LightDir );
	}
	// If we are not moving light, use the MouseDeltaTracker to update camera.
	else
	{
		if( (Key == KEY_MouseX || Key == KEY_MouseY) && Delta != 0.0f )
		{
			const UBOOL LeftMouseButton = Viewport->KeyState(KEY_LeftMouseButton);
			const UBOOL MiddleMouseButton = Viewport->KeyState(KEY_MiddleMouseButton);
			const UBOOL RightMouseButton = Viewport->KeyState(KEY_RightMouseButton);

			MouseDeltaTracker->AddDelta( this, Key, Delta, 0 );
			const FVector DragDelta = MouseDeltaTracker->GetDelta();

			if( !DragDelta.IsZero() )
			{
				GEditor->MouseMovement += DragDelta;

				if ( bAllowMayaCam && GEditor->bUseMayaCameraControls ) 
				{
					if ( bLock )
					{
						//StaticMeshEditor->LockCamera( FALSE ); // unlock 
					}
					FVector Drag;
					FRotator Rot;
					InputAxisMayaCam( Viewport, DragDelta, Drag, Rot );
					if ( bLock )
					{
						LockRot += FRotator( Rot.Pitch, -Rot.Yaw, Rot.Roll );
						LockLocation.Y -= Drag.Y;

						const FMatrix LockMatrix = FRotationMatrix( FRotator(0,LockRot.Yaw,0) ) * FRotationMatrix( FRotator(0,0,LockRot.Pitch) ) * FTranslationMatrix( LockLocation );
						StaticMeshComponent->ConditionalUpdateTransform(FTranslationMatrix(-StaticMeshEditor->StaticMesh->Bounds.Origin) * LockMatrix);
						UpdateCorePreview();
					}
				}
				else
				{
					// Convert the movement delta into drag/rotation deltas

					FVector Drag;
					FRotator Rot;
					FVector Scale;
					MouseDeltaTracker->ConvertMovementDeltaToDragRot( this, DragDelta, Drag, Rot, Scale );

					if( bLock )
					{
						LockRot += FRotator( Rot.Pitch, -Rot.Yaw, Rot.Roll );
						LockLocation.Y -= Drag.Y;

						const FMatrix LockMatrix = FRotationMatrix( FRotator(0,LockRot.Yaw,0) ) * FRotationMatrix( FRotator(0,0,LockRot.Pitch) ) * FTranslationMatrix( LockLocation );
						StaticMeshComponent->ConditionalUpdateTransform(FTranslationMatrix(-StaticMeshEditor->StaticMesh->Bounds.Origin) * LockMatrix);
						UpdateCorePreview();
					}
					else
					{
						MoveViewportCamera( Drag, Rot );
					}
				}

				MouseDeltaTracker->ReduceBy( DragDelta );
			}
		}
	}

	Viewport->Invalidate();

	return TRUE;
}


/*-----------------------------------------------------------------------------
	WxStaticMeshEditor
-----------------------------------------------------------------------------*/

IMPLEMENT_DYNAMIC_CLASS(WxStaticMeshEditor, WxTrackableFrame);

BEGIN_EVENT_TABLE( WxStaticMeshEditor, WxTrackableFrame )
	EVT_SIZE( WxStaticMeshEditor::OnSize )
	EVT_PAINT( WxStaticMeshEditor::OnPaint )
	EVT_UPDATE_UI( ID_SHOW_OPEN_EDGES, WxStaticMeshEditor::UI_ShowEdges )
	EVT_UPDATE_UI( ID_SHOW_UV_OVERLAY, WxStaticMeshEditor::UI_ShowUVOverlay )
	EVT_UPDATE_UI( ID_SHOW_WIREFRAME, WxStaticMeshEditor::UI_ShowWireframe )
	EVT_UPDATE_UI( ID_SHOW_BOUNDS, WxStaticMeshEditor::UI_ShowBounds )
	EVT_UPDATE_UI( IDMN_COLLISION, WxStaticMeshEditor::UI_ShowCollision )
	EVT_UPDATE_UI( ID_LOCK_CAMERA, WxStaticMeshEditor::UI_LockCamera )
	EVT_MENU( ID_SHOW_OPEN_EDGES, WxStaticMeshEditor::OnShowEdges )
	EVT_MENU( ID_SHOW_UV_OVERLAY, WxStaticMeshEditor::OnShowUVOverlay )
	EVT_MENU( ID_SHOW_WIREFRAME, WxStaticMeshEditor::OnShowWireframe )
	EVT_MENU( ID_SHOW_BOUNDS, WxStaticMeshEditor::OnShowBounds )
	EVT_MENU( IDMN_COLLISION, WxStaticMeshEditor::OnShowCollision )
	EVT_MENU( ID_LOCK_CAMERA, WxStaticMeshEditor::OnLockCamera )
	EVT_MENU( ID_SAVE_THUMBNAIL, WxStaticMeshEditor::OnSaveThumbnailAngle )
	EVT_MENU( IDMN_SME_COLLISION_6DOP, WxStaticMeshEditor::OnCollision6DOP )
	EVT_MENU( IDMN_SME_COLLISION_10DOPX, WxStaticMeshEditor::OnCollision10DOPX )
	EVT_MENU( IDMN_SME_COLLISION_10DOPY, WxStaticMeshEditor::OnCollision10DOPY )
	EVT_MENU( IDMN_SME_COLLISION_10DOPZ, WxStaticMeshEditor::OnCollision10DOPZ )
	EVT_MENU( IDMN_SME_COLLISION_18DOP, WxStaticMeshEditor::OnCollision18DOP )
	EVT_MENU( IDMN_SME_COLLISION_26DOP, WxStaticMeshEditor::OnCollision26DOP )
	EVT_MENU( IDMN_SME_COLLISION_SPHERE, WxStaticMeshEditor::OnCollisionSphere )
	EVT_MENU( IDM_SME_LOD_AUTO, WxStaticMeshEditor::OnForceLODLevel )
	EVT_MENU( IDM_SME_LOD_BASE, WxStaticMeshEditor::OnForceLODLevel )
	EVT_MENU( IDM_SME_LOD_1, WxStaticMeshEditor::OnForceLODLevel )
	EVT_MENU( IDM_SME_LOD_2, WxStaticMeshEditor::OnForceLODLevel )
	EVT_MENU( IDM_SME_LOD_3, WxStaticMeshEditor::OnForceLODLevel )
	EVT_MENU( IDM_SME_IMPORTMESHLOD, WxStaticMeshEditor::OnImportMeshLOD )
	EVT_MENU( IDM_SME_REMOVELOD,  WxStaticMeshEditor::OnRemoveLOD )
	EVT_MENU( IDM_SME_GENERATELOD,  WxStaticMeshEditor::OnGenerateLOD )
	EVT_MENU( IDM_SME_GENERATEUVS,  WxStaticMeshEditor::OnGenerateUVs )
	EVT_MENU( IDM_SME_CHANGEMESH,   WxStaticMeshEditor::OnChangeMesh )
	EVT_MENU( IDMN_SME_COLLISION_REMOVE, WxStaticMeshEditor::OnCollisionRemove )
	EVT_MENU( IDMN_SME_COLLISION_CONVERTBOX, WxStaticMeshEditor::OnConvertBoxToConvexCollision )
	EVT_MENU( IDMN_SME_COLLISION_DECOMP, WxStaticMeshEditor::OnCollisionConvexDecomp )
	EVT_MENU( IDM_SME_FRACTURETOOL, WxStaticMeshEditor::OnFractureTool )
	EVT_MENU( IDM_SME_MERGESM, WxStaticMeshEditor::OnMergeStaticMesh )
END_EVENT_TABLE()

WxStaticMeshEditor::WxStaticMeshEditor() : 
FDockingParent(this)
{}

WxStaticMeshEditor::WxStaticMeshEditor( wxWindow* Parent, wxWindowID id, UStaticMesh* InStaticMesh, UBOOL bInSimplifyMode ) :	
        WxTrackableFrame( Parent, id, TEXT(""), wxDefaultPosition, wxDefaultSize, wxFRAME_FLOAT_ON_PARENT | wxDEFAULT_FRAME_STYLE | wxFRAME_NO_TASKBAR )
	,   FDockingParent(this)
	,	StaticMesh( InStaticMesh )
	,	DrawOpenEdges(FALSE)
	,	DrawUVOverlay(FALSE)
	,	DecompOptions(NULL)
	,	FractureOptions(NULL)
	,   MeshSimplificationWindow(NULL)
	,	CurrentViewChunk(0)
	,	CurrentChunkViewMode(ECVM_ViewOnly)
	,	bViewChunkChanged(FALSE)
	,	bAdjustingCore(FALSE)
	,	CoreEditMode(WMM_Translate)
	,	CurrentCoreOffset(0.f,0.f,0.f)
	,	CurrentCoreRotation(0,0,0)
	,	CurrentCoreScale(1.f)
	,	CurrentCoreScale3D(1.f,1.f,1.f)
	,	PendingCoreMesh(NULL)
	,   bSimplifyMode( bInSimplifyMode )
{
	// Set the static mesh editor window title to include the static mesh being edited.
	SetTitle( *FString::Printf( LocalizeSecure(LocalizeUnrealEd("StaticMeshEditorCaption_F"), *StaticMesh->GetPathName()) ) );

	FStaticMeshRenderData* LODModel = &StaticMesh->LODModels(0);
	check(LODModel);

	// When loading slice data from mesh, check its ok. If it isn't we regenerate it.
	UBOOL bBadInfo = FALSE;

	// If a fractured mesh - copy chunk info for preview
	UFracturedStaticMesh* FracMesh = Cast<UFracturedStaticMesh>(InStaticMesh);
	if(FracMesh)
	{
		const TArray<FFragmentInfo>& FragInfo = FracMesh->GetFragments();

		// Number of 'real' fragments - disregarding 'core'
		INT NumToolFrags = FragInfo.Num();
		if(FracMesh->GetCoreFragmentIndex() != INDEX_NONE)
		{
			NumToolFrags--;
		}

		FractureChunks.AddZeroed(NumToolFrags);
		FractureCenters.AddZeroed(NumToolFrags);
		FractureInfos.AddZeroed(NumToolFrags);
		FinalToToolMap.AddZeroed(NumToolFrags);

		// Copy data from mesh into editor structures
		INT CIndex = 0;
		for(INT i=0; i<FragInfo.Num(); i++)
		{
			if(i != FracMesh->GetCoreFragmentIndex())
			{
				// Copy convex hull
				FractureChunks(CIndex).ConvexElem = FragInfo(i).ConvexHull;

				// Copy neighbour info
				for(INT NeighIdx=0; NeighIdx<FragInfo(i).Neighbours.Num(); NeighIdx++)
				{
					BYTE N = FragInfo(i).Neighbours(NeighIdx);
					if(N == 255)
					{
						FractureChunks(CIndex).Neighbours.AddItem( INDEX_NONE );
					}
					else
					{
						FractureChunks(CIndex).Neighbours.AddItem( FragInfo(i).Neighbours(NeighIdx) );
					}

					FractureChunks(CIndex).NeighbourDims.AddItem( FragInfo(i).NeighbourDims(NeighIdx) );
				}

				// In old versions, FacePlaneData did not match Neighbours - need to regen in that case.
				if(FractureChunks(CIndex).ConvexElem.FacePlaneData.Num() != FractureChunks(CIndex).Neighbours.Num())
				{
					bBadInfo = TRUE;
				}

				// Copy center
				FractureCenters(CIndex) = FragInfo(i).Center;

				FractureInfos(CIndex).bCanBeDestroyed = FragInfo(i).bCanBeDestroyed;
				FractureInfos(CIndex).bRootFragment = FragInfo(i).bRootFragment;
				FractureInfos(CIndex).bNeverSpawnPhysics = FragInfo(i).bNeverSpawnPhysicsChunk;

				// Fill in chunk mapping table
				FinalToToolMap(CIndex) = i;

				CIndex++;
			}
		}
	}

	// Create property window
	PropertyWindow = new WxPropertyWindow;
	PropertyWindow->Create( this, NULL );
	PropertyWindow->SetObject( StaticMesh, 1,1,0 );
	
	// Create viewport.
	ViewportHolder = new WxViewportHolder( this, -1, 0 );
	ViewportClient = new FStaticMeshEditorViewportClient( this );
	ViewportClient->Viewport = GEngine->Client->CreateWindowChildViewport( ViewportClient, (HWND)ViewportHolder->GetHandle() );
	ViewportClient->Viewport->CaptureJoystickInput( FALSE );
	ViewportHolder->SetViewport( ViewportClient->Viewport );
	ViewportHolder->Show();

	
	// Create mesh simplification window
	MeshSimplificationWindow = new WxMeshSimplificationWindow( this );

	if( bSimplifyMode )
	{
		FWindowUtil::LoadPosSize( GetDockingParentName(), this, 64,64,800,650 );
	}
	else
	{
		FWindowUtil::LoadPosSize( GetDockingParentName(), this, 64,64,800,450 );
	}

	// Add docking windows.
	{
		AddDockingWindow( ViewportHolder, FDockingParent::DH_None, NULL );
		SetDockHostSize(FDockingParent::DH_Right, 300);

		AddDockingWindow(PropertyWindow, FDockingParent::DH_Right, *FString::Printf(LocalizeSecure(LocalizeUnrealEd("PropertiesCaption_F"), *StaticMesh->GetPathName())), *LocalizeUnrealEd("Properties"));

		AddDockingWindow(
			MeshSimplificationWindow,
			FDockingParent::DH_Right,
			*LocalizeUnrealEd( "MeshSimp_WindowTitle" ),	// Docking window title
			TEXT( "MeshSimplification" ) );	// Window name, used for saving docked window state

		// In 'simplify mode', we hide the property window initially
		ShowDockingWindow( PropertyWindow, ( bSimplifyMode == FALSE ) );

		// Try to load a existing layout for the docking windows.
		LoadDockingLayout();

		// Even after we've loaded our layout, make sure that Simplify window is hidden while in regular mode,
		// and visible while in simplify mode
		ShowDockingWindow( MeshSimplificationWindow, ( bSimplifyMode == TRUE ) );
	}

	ToolBar = new WxStaticMeshEditorBar( this, InStaticMesh, -1 );
	SetToolBar( ToolBar );

	MenuBar = new WxStaticMeshEditMenu( InStaticMesh );
	AppendWindowMenu(MenuBar);
	SetMenuBar( MenuBar );

	UpdateToolbars();

	// Reset view position to something reasonable
	LockCamera( ViewportClient->bLock );

	// Recalc regions if we could not load them correctly
	if(bBadInfo)
	{
		ComputeRegions(FVector(1,1,1), FALSE);
	}

	if( MeshSimplificationWindow != NULL && bSimplifyMode )
	{
		// Check for existing simplified copy of this mesh in the level's package
		MeshSimplificationWindow->CheckForAlreadySimplifiedDuplicate();
	}
}

WxStaticMeshEditor::~WxStaticMeshEditor()
{
	FWindowUtil::SavePosSize( GetDockingParentName(), this );
	
	SaveDockingLayout();

	GEngine->Client->CloseViewport( ViewportClient->Viewport );
	ViewportClient->Viewport = NULL;
	delete ViewportClient;

	if( PropertyWindow != NULL )
	{
		UnbindDockingWindow( PropertyWindow );
		PropertyWindow->SetObject( NULL, 0,0,0 );
		PropertyWindow->Destroy();
		PropertyWindow = NULL;
	}

	if( MeshSimplificationWindow != NULL )
	{
		UnbindDockingWindow( MeshSimplificationWindow );
		MeshSimplificationWindow->Destroy();
		MeshSimplificationWindow = NULL;
	}
}

/** Changes the StaticMeshEditor to look at this new mesh instead. */
void WxStaticMeshEditor::SetEditorMesh(UStaticMesh* NewMesh)
{
	StaticMesh = NewMesh;
	ViewportClient->UpdatePreviewComponent();
	SetTitle( *FString::Printf(LocalizeSecure(LocalizeUnrealEd("StaticMeshEditorCaption_F"), *StaticMesh->GetPathName())) );
	this->UpdatePropertyView();

	if( MeshSimplificationWindow != NULL )
	{
		// Tell the simplification window to invalidate any meshes it's cached
		MeshSimplificationWindow->InvalidateHighResSourceStaticMesh();
	}

	// We're switching the mesh out, so make sure the forced LOD settings get reset
	ViewportClient->StaticMeshComponent->ForcedLodModel = 0;
	ToolBar->ToggleTool( IDM_SME_LOD_AUTO, true );

	SetDockingWindowTitle( PropertyWindow, *FString::Printf(LocalizeSecure(LocalizeUnrealEd("PropertiesCaption_F"), *StaticMesh->GetPathName())) );
	UpdateToolbars();
	ViewportClient->Viewport->Invalidate();
}

/**
 * This function is called when the window has been selected from within the ctrl + tab dialog.
 */
void WxStaticMeshEditor::OnSelected()
{
	Raise();
}


/** Invalidates the viewport */
void WxStaticMeshEditor::InvalidateViewport()
{
	if( ViewportClient != NULL )
	{
		ViewportClient->Viewport->Invalidate();
	}
}


/**
 *	This function returns the name of the docking parent.  This name is used for saving and loading the layout files.
 *  @return A string representing a name to use for this docking parent.
 */
const TCHAR* WxStaticMeshEditor::GetDockingParentName() const
{
	// We use a different name for the window layout when we're running in 'Simplify Mode'
	return bSimplifyMode ? TEXT("StaticMeshEditor") : TEXT("StaticMeshEditor");
}

/**
 * @return The current version of the docking parent, this value needs to be increased every time new docking windows are added or removed.
 */
const INT WxStaticMeshEditor::GetDockingParentVersion() const
{
	return 1;
}

/**
 * Updates the property view.
 */
void WxStaticMeshEditor::UpdatePropertyView()
{
	PropertyWindow->SetObject( StaticMesh, TRUE, TRUE, FALSE );
}

void WxStaticMeshEditor::OnSize( wxSizeEvent& In )
{
	wxFrame::OnSize(In);
}

void WxStaticMeshEditor::Serialize(FArchive& Ar)
{
	Ar << StaticMesh;
	check(ViewportClient);
	ViewportClient->Serialize(Ar);

	if( MeshSimplificationWindow != NULL )
	{
		MeshSimplificationWindow->Serialize( Ar );
	}
}

void WxStaticMeshEditor::OnPaint( wxPaintEvent& In )
{
	wxPaintDC dc( this );
	ViewportClient->Viewport->Invalidate();
}

void WxStaticMeshEditor::UI_ShowEdges( wxUpdateUIEvent& In )
{
	const bool bCheckStatus = DrawOpenEdges == TRUE;
	In.Check( bCheckStatus );
}

void WxStaticMeshEditor::UI_ShowUVOverlay( wxUpdateUIEvent& In )
{
	const bool bCheckStatus = DrawUVOverlay == TRUE;
	In.Check( bCheckStatus );
}

void WxStaticMeshEditor::UI_ShowWireframe( wxUpdateUIEvent& In )
{
	const bool bCheckStatus = (ViewportClient->ShowFlags & SHOW_ViewMode_Mask) == SHOW_ViewMode_Wireframe;
	In.Check( bCheckStatus );
}

void WxStaticMeshEditor::UI_ShowBounds( wxUpdateUIEvent& In )
{
	const bool bCheckStatus = ViewportClient->ShowFlags & SHOW_Bounds ? true : false;
	In.Check( bCheckStatus );
}

void WxStaticMeshEditor::UI_ShowCollision( wxUpdateUIEvent& In )
{
	const bool bCheckStatus = ViewportClient->ShowFlags & SHOW_Collision ? true : false;
	In.Check( bCheckStatus );
}

void WxStaticMeshEditor::UI_LockCamera( wxUpdateUIEvent& In )
{
	const bool bCheckStatus = ViewportClient->bLock == TRUE;
	In.Check( bCheckStatus );
}

void WxStaticMeshEditor::OnShowEdges( wxCommandEvent& In )
{
	DrawOpenEdges = !DrawOpenEdges;
	ViewportClient->Viewport->Invalidate();
}

void WxStaticMeshEditor::OnShowUVOverlay( wxCommandEvent& In )
{
	DrawUVOverlay = !DrawUVOverlay;
	ViewportClient->Viewport->Invalidate();
}

void WxStaticMeshEditor::OnShowWireframe( wxCommandEvent& In )
{
	if((ViewportClient->ShowFlags & SHOW_ViewMode_Mask) == SHOW_ViewMode_Wireframe)
	{
		ViewportClient->ShowFlags &= ~SHOW_ViewMode_Mask;
		ViewportClient->ShowFlags |= SHOW_ViewMode_Lit;
	}
	else
	{
		ViewportClient->ShowFlags &= ~SHOW_ViewMode_Mask;
		ViewportClient->ShowFlags |= SHOW_ViewMode_Wireframe;
	}
	ViewportClient->Viewport->Invalidate();
}

void WxStaticMeshEditor::OnShowBounds( wxCommandEvent& In )
{
	ViewportClient->ShowFlags ^= SHOW_Bounds;
	ViewportClient->Viewport->Invalidate();
}

void WxStaticMeshEditor::OnShowCollision( wxCommandEvent& In )
{
	ViewportClient->ShowFlags ^= SHOW_Collision;
	ViewportClient->Viewport->Invalidate();
}

void WxStaticMeshEditor::LockCamera(UBOOL bInLock)
{
	ViewportClient->bLock = bInLock;
	ViewportClient->Viewport->Invalidate();

	// Only reset view location/rotation when the user turns ON locking.
	if( ViewportClient->bLock )
	{
		// Disable grid drawing when view is locked.
		ViewportClient->ShowFlags &= ~SHOW_Grid;

		ViewportClient->ViewLocation = -FVector(0,StaticMesh->Bounds.SphereRadius / (75.0f * (FLOAT)PI / 360.0f),0);
		ViewportClient->ViewRotation = FRotator(0,16384,0);

		ViewportClient->LockLocation = FVector(0,0,0);
		ViewportClient->LockRot = StaticMesh->ThumbnailAngle;

		FMatrix LockMatrix = FRotationMatrix( FRotator(0,ViewportClient->LockRot.Yaw,0) ) * FRotationMatrix( FRotator(0,0,ViewportClient->LockRot.Pitch) );
		ViewportClient->StaticMeshComponent->ConditionalUpdateTransform(FTranslationMatrix(-StaticMesh->Bounds.Origin) * LockMatrix);
		ViewportClient->UpdateCorePreview();
	}
	else
	{
		// Enable grid drawing when view is free.
		ViewportClient->ShowFlags |= SHOW_Grid;

		ViewportClient->LockLocation = FVector(0,0,0);
		ViewportClient->LockRot = FRotator(0,0,0);
	}
}

void WxStaticMeshEditor::OnLockCamera( wxCommandEvent& In )
{
	if ( ViewportClient->bAllowMayaCam && GEditor->bUseMayaCameraControls )
	{
		return;
	}

	// Toggle camera locking.
	LockCamera( !ViewportClient->bLock );
}

void WxStaticMeshEditor::OnSaveThumbnailAngle( wxCommandEvent& In )
{
	ViewportClient->StaticMeshComponent->StaticMesh->ThumbnailAngle = ViewportClient->LockRot;
	ViewportClient->StaticMeshComponent->StaticMesh->ThumbnailDistance = ViewportClient->LockLocation.Y;
	ViewportClient->StaticMeshComponent->StaticMesh->MarkPackageDirty();
	GCallbackEvent->Send(CALLBACK_RefreshEditor_GenericBrowser);
}

void WxStaticMeshEditor::OnCollision6DOP( wxCommandEvent& In )
{
	GenerateKDop(KDopDir6,6);
}

void WxStaticMeshEditor::OnCollision10DOPX( wxCommandEvent& In )
{
	GenerateKDop(KDopDir10X,10);
}

void WxStaticMeshEditor::OnCollision10DOPY( wxCommandEvent& In )
{
	GenerateKDop(KDopDir10Y,10);
}

void WxStaticMeshEditor::OnCollision10DOPZ( wxCommandEvent& In )
{
	GenerateKDop(KDopDir10Z,10);
}

void WxStaticMeshEditor::OnCollision18DOP( wxCommandEvent& In )
{
	GenerateKDop(KDopDir18,18);
}

void WxStaticMeshEditor::OnCollision26DOP( wxCommandEvent& In )
{
	GenerateKDop(KDopDir26,26);
}

void WxStaticMeshEditor::OnCollisionSphere( wxCommandEvent& In )
{
	GenerateSphere();
}

/**
 * Handles the user selection of the remove collision option. Uses the
 * common routine for clearing the collision
 *
 * @param In the command event to handle
 */
void WxStaticMeshEditor::OnCollisionRemove( wxCommandEvent& In )
{
	RemoveCollision();
}

/**
* Handles the user selection of the convert collision option. 
*
* @param In the command event to handle
*/
void WxStaticMeshEditor::OnConvertBoxToConvexCollision( wxCommandEvent& In )
{
	ConvertBoxToConvexCollision();
}

/** When menu item is selected, pop up options dialog for convex decomposition util */
void WxStaticMeshEditor::OnCollisionConvexDecomp( wxCommandEvent& In )
{
	if(!DecompOptions)
	{
		DecompOptions = new WxConvexDecompOptions( this, this );
		DecompOptions->Show();
		ViewportClient->ShowFlags |= SHOW_Collision;
	}
}

/** This is called when Apply is pressed in the dialog. Does the actual processing. */
void WxStaticMeshEditor::DoDecomp(INT Depth, INT MaxVerts, FLOAT CollapseThresh)
{
	// Check we have a selected StaticMesh
	if(StaticMesh)
	{
		// Make vertex buffer
		INT NumVerts = StaticMesh->LODModels(0).NumVertices;
		TArray<FVector> Verts;
		for(INT i=0; i<NumVerts; i++)
		{
			FVector Vert = StaticMesh->LODModels(0).PositionVertexBuffer.VertexPosition(i);
			Verts.AddItem(Vert);
		}
	
		// Make index buffer
		INT NumIndices = StaticMesh->LODModels(0).IndexBuffer.Indices.Num();
		TArray<INT> Indices;
		for(INT i=0; i<NumIndices; i++)
		{
			INT Index = StaticMesh->LODModels(0).IndexBuffer.Indices(i);
			Indices.AddItem(Index);
		}

		// Make sure rendering is done - so we are not changing data being used by collision drawing.
		FlushRenderingCommands();

		// Get the BodySetup we are going to put the collision into
		URB_BodySetup* bs = StaticMesh->BodySetup;
		if(bs)
		{
			bs->AggGeom.EmptyElements();
			bs->ClearShapeCache();
		}
		else
		{
			// Otherwise, create one here.
			StaticMesh->BodySetup = ConstructObject<URB_BodySetup>(URB_BodySetup::StaticClass(), StaticMesh);
			bs = StaticMesh->BodySetup;
		}

		// Run actual util to do the work
		DecomposeMeshToHulls(&(bs->AggGeom), Verts, Indices, Depth, 0.1f, CollapseThresh, MaxVerts);		

		// Mark mesh as dirty
		StaticMesh->MarkPackageDirty();

		// Update screen.
		ViewportClient->Viewport->Invalidate();
		this->UpdatePropertyView();
	}
}

/** When options window is closed - clear pointer. */
void WxStaticMeshEditor::DecompOptionsClosed()
{
	DecompOptions = NULL;
	ViewportClient->ShowFlags &= ~SHOW_Collision;
}


/**
 * Clears the collision data for the static mesh
 */
void WxStaticMeshEditor::RemoveCollision(void)
{
	// If we have a collision model for this staticmesh, ask if we want to replace it.
	if (StaticMesh->BodySetup != NULL)
	{
		UBOOL bShouldReplace = appMsgf(AMT_YesNo, *LocalizeUnrealEd("RemoveCollisionPrompt"));
		if (bShouldReplace == TRUE)
		{
			StaticMesh->BodySetup = NULL;
			// Mark staticmesh as dirty, to help make sure it gets saved.
			StaticMesh->MarkPackageDirty();
			// Update views/property windows
			ViewportClient->Viewport->Invalidate();
			this->UpdatePropertyView();
		}
	}
}

/** Util for adding vertex to an array if it is not already present. */
static void AddVertexIfNotPresent(TArray<FVector>& Vertices, const FVector& NewVertex)
{
	UBOOL bIsPresent = FALSE;

	for(INT i=0; i<Vertices.Num(); i++)
	{
		FLOAT diffSqr = (NewVertex - Vertices(i)).SizeSquared();
		if(diffSqr < 0.01f * 0.01f)
		{
			bIsPresent = 1;
			break;
		}
	}

	if(!bIsPresent)
	{
		Vertices.AddItem(NewVertex);
	}
}

/*
 * Quick and dirty way of creating box vertices from a box collision representation
 * Grossly inefficient, but not time critical
 * @param BoxElem - Box collision to get the vertices for
 * @param Verts - output listing of vertex data from the box collision
 * @param Scale - scale to create this box at
 */
void CreateBoxVertsFromBoxCollision(const FKBoxElem& BoxElem, TArray<FVector>& Verts, FLOAT Scale)
{
	FVector	B[2], P, Q, Radii;

	// X,Y,Z member variables are LENGTH not RADIUS
	Radii.X = Scale*0.5f*BoxElem.X;
	Radii.Y = Scale*0.5f*BoxElem.Y;
	Radii.Z = Scale*0.5f*BoxElem.Z;

	B[0] = Radii; // max
	B[1] = -1.0f * Radii; // min

	for( INT i=0; i<2; i++ )
	{
		for( INT j=0; j<2; j++ )
		{
			P.X=B[i].X; Q.X=B[i].X;
			P.Y=B[j].Y; Q.Y=B[j].Y;
			P.Z=B[0].Z; Q.Z=B[1].Z;
			AddVertexIfNotPresent(Verts, BoxElem.TM.TransformFVector(P));
			AddVertexIfNotPresent(Verts, BoxElem.TM.TransformFVector(Q));

			P.Y=B[i].Y; Q.Y=B[i].Y;
			P.Z=B[j].Z; Q.Z=B[j].Z;
			P.X=B[0].X; Q.X=B[1].X;
			AddVertexIfNotPresent(Verts, BoxElem.TM.TransformFVector(P));
			AddVertexIfNotPresent(Verts, BoxElem.TM.TransformFVector(Q));

			P.Z=B[i].Z; Q.Z=B[i].Z;
			P.X=B[j].X; Q.X=B[j].X;
			P.Y=B[0].Y; Q.Y=B[1].Y;
			AddVertexIfNotPresent(Verts, BoxElem.TM.TransformFVector(P));
			AddVertexIfNotPresent(Verts, BoxElem.TM.TransformFVector(Q));
		}
	}
}


/**
* Converts the collision data for the static mesh
*/
void WxStaticMeshEditor::ConvertBoxToConvexCollision(void)
{
	// If we have a collision model for this staticmesh, ask if we want to replace it.
	if (StaticMesh->BodySetup != NULL)
	{
		UBOOL bShouldReplace = appMsgf(AMT_YesNo, *LocalizeUnrealEd("ConvertBoxCollisionPrompt"));
		if (bShouldReplace == TRUE)
		{
			URB_BodySetup* BodySetup = StaticMesh->BodySetup;

			INT NumBoxElems = BodySetup->AggGeom.BoxElems.Num();
			if (NumBoxElems > 0)
			{
				// Make sure rendering is done - so we are not changing data being used by collision drawing.
				FlushRenderingCommands();

				FKConvexElem* NewConvexColl = NULL;

				//For each box elem, calculate the new convex collision representation
				//Stored in a temp array so we can undo on failure.
				TArray<FKConvexElem> TempArray;

				for (INT i=0; i<NumBoxElems; i++)
				{
					const FKBoxElem& BoxColl = BodySetup->AggGeom.BoxElems(i);

					//Create a new convex collision element
					NewConvexColl = new(TempArray) FKConvexElem;
					appMemzero(NewConvexColl, sizeof(FKConvexElem));
					NewConvexColl->Reset();

					//Fill the convex verts from the box elem collision and generate the convex hull
					CreateBoxVertsFromBoxCollision(BoxColl, NewConvexColl->VertexData, 1.0f);
					if (NewConvexColl->GenerateHullData() == FALSE)
					{
						appMsgf(AMT_OK, *LocalizeUnrealEd("ConvertBoxCollisionFailedPrompt"));
						return;
					}
				}

				//Clear the cache (PIE may have created some data)
				BodySetup->ClearShapeCache();

				//Copy the new data into the static mesh
				BodySetup->AggGeom.ConvexElems.Append(TempArray);

				//Clear out what we just replaced
				BodySetup->AggGeom.BoxElems.Empty();

				// Mark static mesh as dirty, to help make sure it gets saved.
				StaticMesh->MarkPackageDirty();

				// Update views/property windows
				ViewportClient->Viewport->Invalidate();
				this->UpdatePropertyView();
			}
		}
	}
}


void WxStaticMeshEditor::GenerateKDop(const FVector* Directions,UINT NumDirections)
{
	TArray<FVector>	DirArray;

	for(UINT DirectionIndex = 0;DirectionIndex < NumDirections;DirectionIndex++)
		DirArray.AddItem(Directions[DirectionIndex]);

	GenerateKDopAsCollisionModel( StaticMesh, DirArray );

	ViewportClient->Viewport->Invalidate();
	this->UpdatePropertyView();
}

void WxStaticMeshEditor::GenerateSphere()
{
	GenerateSphereAsKarmaCollision( StaticMesh );

	ViewportClient->Viewport->Invalidate();
	this->UpdatePropertyView();
}

/** Handler for forcing the rendering of the preview model to use a particular LOD. */
void WxStaticMeshEditor::OnForceLODLevel( wxCommandEvent& In)
{
	if(In.GetId() == IDM_SME_LOD_AUTO)
	{
		UpdateLODStats(0);
		ViewportClient->StaticMeshComponent->ForcedLodModel = 0;
	}
	else if(In.GetId() == IDM_SME_LOD_BASE)
	{
		UpdateLODStats(0);
		ViewportClient->StaticMeshComponent->ForcedLodModel = 1;
	}
	else if(In.GetId() == IDM_SME_LOD_1)
	{
		UpdateLODStats(1);
		ViewportClient->StaticMeshComponent->ForcedLodModel = 2;
	}
	else if(In.GetId() == IDM_SME_LOD_2)
	{
		UpdateLODStats(2);
		ViewportClient->StaticMeshComponent->ForcedLodModel = 3;
	}
	else if(In.GetId() == IDM_SME_LOD_3)
	{
		UpdateLODStats(3);
		ViewportClient->StaticMeshComponent->ForcedLodModel = 4;
	}

	//MenuBar->Check( In.GetId(), true );
	ToolBar->ToggleTool( In.GetId(), true );
	{FComponentReattachContext ReattachContext(ViewportClient->StaticMeshComponent);}
	ViewportClient->Viewport->Invalidate();
}

/** Handler for removing a particular LOD from the SkeletalMesh. */
void WxStaticMeshEditor::OnRemoveLOD(wxCommandEvent &In)
{
	if( StaticMesh->LODModels.Num() == 1 )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("NoLODToRemove") );
		return;
	}

	// Now display combo to choose which LOD to remove.
	TArray<FString> LODStrings;
	LODStrings.AddZeroed( StaticMesh->LODModels.Num()-1 );
	for(INT i=0; i<StaticMesh->LODModels.Num()-1; i++)
	{
		LODStrings(i) = FString::Printf( TEXT("%d"), i+1 );
	}

	// pop up dialog
	WxDlgGenericComboEntry dlg;
	if( dlg.ShowModal( TEXT("ChooseLODLevel"), TEXT("LODLevel:"), LODStrings, 0, TRUE ) == wxID_OK )
	{
		check( StaticMesh->LODInfo.Num() == StaticMesh->LODModels.Num() );

		// If its a valid LOD, kill it.
		INT DesiredLOD = dlg.GetComboBox().GetSelection() + 1;
		if( DesiredLOD > 0 && DesiredLOD < StaticMesh->LODModels.Num() )
		{
			// Detach all instances of the static mesh while the LOD is being removed.
			FStaticMeshComponentReattachContext	ComponentReattachContext(StaticMesh);

			StaticMesh->ReleaseResources();
			StaticMesh->ReleaseResourcesFence.Wait();
			StaticMesh->LODModels.Remove(DesiredLOD);
			StaticMesh->LODInfo.Remove(DesiredLOD);
			StaticMesh->InitResources();

			// Set the forced LOD to Auto.
			ViewportClient->StaticMeshComponent->ForcedLodModel = 0;
			ToolBar->ToggleTool( IDM_SME_LOD_AUTO, true );
			UpdateToolbars();
		}
	}
}


/** Event for generating an LOD */
void WxStaticMeshEditor::OnGenerateLOD( wxCommandEvent& In )
{
	// Combo options to choose target LOD
	TArray<FString> LODStrings;
	FillOutLODAlterationStrings(LODStrings);


	WxDlgGenericComboEntry dlg;	
	if( dlg.ShowModal( TEXT("ChooseLODLevel"), TEXT("LODLevel:"), LODStrings, 0, TRUE ) == wxID_OK )
	{
		const INT DesiredLOD = dlg.GetComboBox().GetSelection()+1;

		// Dialog to select target triangle count
		WxDlgGenericStringEntry dlg2;

		FStaticMeshRenderData& BaseLOD = StaticMesh->LODModels(0);

		FString NumFacesString = appItoa(BaseLOD.IndexBuffer.Indices.Num() / 6);
		if( dlg2.ShowModal(TEXT("GenerateLOD"), TEXT("LODTriCount"), *NumFacesString) == wxID_OK )
		{
			const FString Tris = dlg2.GetEnteredString();
			const INT NumTris = appAtoi(*Tris);

			if( GenerateLOD( StaticMesh, StaticMesh, DesiredLOD, NumTris ) )
			{
				UpdateToolbars();
				appMsgf( AMT_OK, *LocalizeUnrealEd("LODGenerationSuccessful") );
			}
		}
	}
}

/** Change the mesh the editor is viewing. */
void WxStaticMeshEditor::OnChangeMesh( wxCommandEvent& In )
{
	UStaticMesh* SelectedMesh = GEditor->GetSelectedObjects()->GetTop<UStaticMesh>();
	if(SelectedMesh && SelectedMesh != StaticMesh)
	{
		SetEditorMesh(SelectedMesh);
	}
}

/** Event for generating UVs */
void WxStaticMeshEditor::OnGenerateUVs( wxCommandEvent& In )
{
	//an entry for each LOD
	TArray<FString> LODStrings;
	//the number of texture coordinates each LOD has 
	//used for updating the tex coord combo box when a different LOD is chosen
	TArray<INT> LODNumTexcoords;
	LODStrings.AddZeroed(StaticMesh->LODModels.Num());
	LODNumTexcoords.AddZeroed(StaticMesh->LODModels.Num());
	for(INT i = 0; i < StaticMesh->LODModels.Num(); i++)
	{
		//populate the arrays
		LODStrings(i) = FString::Printf( TEXT("%d"), i );
		LODNumTexcoords(i) = StaticMesh->LODModels(i).VertexBuffer.GetNumTexCoords();
	}

	//create a dialog that exposes the options needed to generate UV's
	WxDlgGenerateUVOptions dlg(this, LODStrings, LODNumTexcoords, 0.9f);
	if (dlg.ShowModal() == wxID_OK)
	{
		GWarn->BeginSlowTask( *LocalizeUnrealEd("GenerateUVsProgressText"), TRUE );

		// Detach all instances of the static mesh while generating the UVs, then reattach them.
		FStaticMeshComponentReattachContext	ComponentReattachContext(StaticMesh);

		UINT NumGeneratedPatches = 0;
		//call the utility helper with the user supplied parameters
		if (GenerateUVs(StaticMesh, dlg.ChosenLODIndex, dlg.ChosenTexIndex, dlg.bKeepExistingUVs, dlg.MaxStretch, NumGeneratedPatches))
		{
			//update toolbar states
			UpdateToolbars();
			appMsgf( AMT_OK, *FString::Printf(LocalizeSecure(LocalizeUnrealEd("UVGenerationSuccessful"), NumGeneratedPatches, dlg.MaxStretch)));
		}

		GWarn->EndSlowTask();
	}
}

void WxStaticMeshEditor::OnImportMeshLOD( wxCommandEvent& In )
{
	WxFileDialog ImportFileDialog( this, 
		*LocalizeUnrealEd("ImportMeshLOD"), 
		*(GApp->LastDir[LD_GENERIC_IMPORT]),
		TEXT(""),
		TEXT("ASE files|*.ase"),
		wxOPEN | wxFILE_MUST_EXIST,
		wxDefaultPosition);

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
			FillOutLODAlterationStrings(LODStrings);

			WxDlgGenericComboEntry dlg;
			if( dlg.ShowModal( TEXT("ChooseLODLevel"), TEXT("LODLevel:"), LODStrings, 0, TRUE ) == wxID_OK )
			{
				INT DesiredLOD = dlg.GetComboBox().GetSelection()+1;
				// If the LOD we want
				if( DesiredLOD > 0  )
				{
					FFilename Filename = (const TCHAR*)ImportFilePaths[0];
					GApp->LastDir[LD_GENERIC_IMPORT] = Filename.GetPath(); // Save path as default for next time.

					// Load the data from the file into a byte array.
					FString Data;
					if( appLoadFileToString( Data, *Filename ) )
					{
						const TCHAR* Ptr = *Data;

						debugf(TEXT("LOD %d loading (%x)"),DesiredLOD,(INT)&DesiredLOD);

						// Use the StaticMeshFactory to load this StaticMesh into a temporary StaticMesh.
						UStaticMeshFactory* StaticMeshFact = new UStaticMeshFactory();
						UStaticMesh* TempStaticMesh = (UStaticMesh*)StaticMeshFact->FactoryCreateText( 
							UStaticMesh::StaticClass(), UObject::GetTransientPackage(), NAME_None, 0, NULL, TEXT("ASE"), Ptr, Ptr+Data.Len(), GWarn );

						debugf(TEXT("LOD %d loaded"),DesiredLOD);

						// Extract data to LOD
						if(TempStaticMesh)
						{
							// Detach all instances of the static mesh from the scene while we're merging the LOD data.
							FStaticMeshComponentReattachContext	ComponentReattachContext(StaticMesh);

							// Overwriting existing LOD
							if(DesiredLOD < StaticMesh->LODModels.Num())
							{
								StaticMesh->LODModels(DesiredLOD).ReleaseResources();
								StaticMesh->LODModels(DesiredLOD).PositionVertexBuffer.CleanUp();
								StaticMesh->LODModels(DesiredLOD).VertexBuffer.CleanUp();
								StaticMesh->LODModels(DesiredLOD) = TempStaticMesh->LODModels(0);
							}
							// Adding new LOD
							else
							{
								// Add dummy LODs if the LOD being inserted is not the next one in the array
								while(StaticMesh->LODModels.Num() < DesiredLOD)
								{
									new(StaticMesh->LODModels) FStaticMeshRenderData();
								}

								// Add dummy LODs if the LOD being inserted is not the next one in the array
								while(StaticMesh->LODInfo.Num() <= DesiredLOD)
								{
									StaticMesh->LODInfo.AddZeroed();
								}

								StaticMesh->LODInfo(DesiredLOD) = FStaticMeshLODInfo();

								FStaticMeshRenderData* Data = new(StaticMesh->LODModels) FStaticMeshRenderData();
								*Data = TempStaticMesh->LODModels(0);
							}

							// Copy the LOD Info (materials, collision, shadow casting) from the LOD0 to the newly imported LOD
							FStaticMeshRenderData& SourceLODRenderData = StaticMesh->LODModels(0);
							FStaticMeshRenderData& TargetLODRenderData = StaticMesh->LODModels(DesiredLOD);
							
							for(INT MatIndex = 0; MatIndex < TargetLODRenderData.Elements.Num(); MatIndex++)
							{
								// Copy as many elements as exist in LOD0, and fill the rest with the last element of LOD0.
								INT SrcIndex = ( MatIndex >= SourceLODRenderData.Elements.Num() ) ? ( SourceLODRenderData.Elements.Num() ) - 1 : ( MatIndex );

								TargetLODRenderData.Elements(MatIndex).Material = SourceLODRenderData.Elements(SrcIndex).Material;
								TargetLODRenderData.Elements(MatIndex).bEnableShadowCasting = SourceLODRenderData.Elements(SrcIndex).bEnableShadowCasting;
								TargetLODRenderData.Elements(MatIndex).EnableCollision = SourceLODRenderData.Elements(SrcIndex).EnableCollision;
							}

							// Rebuild the static mesh.
							StaticMesh->Build();
						}

						// Update mesh component
						StaticMesh->MarkPackageDirty();
						UpdateToolbars();
						appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("LODImportSuccessful"), DesiredLOD) );
					}
				}
			}
		}

	}
}

/**
* Updates the UI
*/
void WxStaticMeshEditor::UpdateToolbars()
{
	if(StaticMesh)
	{
		UpdateLODStats(0);

		// Update LOD info with the materials
		check(StaticMesh->LODInfo.Num() == StaticMesh->LODModels.Num());
		for( INT LODIndex = 0; LODIndex < StaticMesh->LODModels.Num(); LODIndex++)
		{
			FStaticMeshRenderData& LODData = StaticMesh->LODModels(LODIndex);
			FStaticMeshLODInfo& LODInfo = StaticMesh->LODInfo(LODIndex);

			LODInfo.Elements.Empty();
			LODInfo.Elements.AddZeroed( LODData.Elements.Num() );

			for(INT MatIndex = 0; MatIndex < LODData.Elements.Num(); MatIndex++)
			{
				LODInfo.Elements(MatIndex).Material = LODData.Elements(MatIndex).Material;
				LODInfo.Elements(MatIndex).bEnableShadowCasting = LODData.Elements(MatIndex).bEnableShadowCasting;
				LODInfo.Elements(MatIndex).bEnableCollision = LODData.Elements(MatIndex).EnableCollision;
			}
		}

		this->UpdatePropertyView();

		ToolBar->EnableTool( IDM_SME_LOD_1, StaticMesh->LODModels.Num() > 1 );
		ToolBar->EnableTool( IDM_SME_LOD_2, StaticMesh->LODModels.Num() > 2 );
		ToolBar->EnableTool( IDM_SME_LOD_3, StaticMesh->LODModels.Num() > 3 );
	}
}


/** 
* Updates NumTriangles, NumVertices and NumUVChannels for the given LOD 
*/
void WxStaticMeshEditor::UpdateLODStats(INT CurrentLOD) 
{

	check(CurrentLOD >= 0 && CurrentLOD < StaticMesh->LODModels.Num());
	FStaticMeshRenderData* LODModel = &StaticMesh->LODModels(CurrentLOD);
	check(LODModel);

	NumTriangles = LODModel->IndexBuffer.Indices.Num() / 3;

	NumVertices = LODModel->NumVertices;

	NumUVChannels = LODModel->VertexBuffer.GetNumTexCoords();
}


//////////////////////////////////////////////////////////////////////////
// FRACTURE TOOL
//////////////////////////////////////////////////////////////////////////

/** Show the fracture tool options. */
void WxStaticMeshEditor::OnFractureTool(wxCommandEvent& In)
{
	UBOOL bShowFracTool = In.IsChecked();

	if(bShowFracTool && !FractureOptions)
	{
		if(!StaticMesh->BodySetup)
		{
			appMsgf( AMT_OK, *LocalizeUnrealEd("NeedSimpCollisionForFracture") );
			return;
		}

		FractureOptions = new WxFractureToolOptions( this );
		FractureOptions->Show();
		ViewportClient->Viewport->Invalidate();

		MenuBar->Check(IDM_SME_FRACTURETOOL, TRUE);
		ToolBar->ToggleTool(IDM_SME_FRACTURETOOL, TRUE);
	}
	else if(!bShowFracTool && FractureOptions)
	{
		FractureOptions->Close();
	}
}

/** Called when chunk view options change. */
void WxStaticMeshEditor::ChangeChunkView(INT ChunkIndex, EChunkViewMode ViewMode)
{
	CurrentViewChunk = ChunkIndex;
	CurrentChunkViewMode = ViewMode;
	bViewChunkChanged = TRUE;

	// Redraw the viewport
	ViewportClient->Viewport->Invalidate();
}

/** Called when options window is closed, to release pointer. */
void WxStaticMeshEditor::FractureOptionsClosed()
{
	FWindowUtil::SavePosSize(TEXT("FractureTool"), FractureOptions);

	// If in the middle of adjusting when we close the window - cancel.
	if(bAdjustingCore)
	{
		CancelAddCore();
	}

	FractureOptions = NULL;

	// Set all fragments to be visible when closing dialog
	UFracturedStaticMeshComponent* FracturedMeshComponent = Cast<UFracturedStaticMeshComponent>(ViewportClient->StaticMeshComponent);
	if(FracturedMeshComponent)
	{
		TArray<BYTE> VisibleFragments = FracturedMeshComponent->GetVisibleFragments();
		for(INT i=0; i<VisibleFragments.Num(); i++)
		{
			VisibleFragments(i) = 1;
		}
		FracturedMeshComponent->SetVisibleFragments(VisibleFragments);
		ViewportClient->Viewport->Invalidate();
	}

	MenuBar->Check(IDM_SME_FRACTURETOOL, FALSE);
	ToolBar->ToggleTool(IDM_SME_FRACTURETOOL, FALSE);
}

/** Generate points randomly for regions. */
void WxStaticMeshEditor::GenerateRandomPoints(INT NumChunks)
{
	UFracturedStaticMesh* ExistingFracturedMesh = Cast<UFracturedStaticMesh>(StaticMesh);
	UStaticMesh* SourceMesh = StaticMesh;
	if (ExistingFracturedMesh)
	{
		SourceMesh = ExistingFracturedMesh->SourceStaticMesh;
	}

	if(!SourceMesh)
	{
		return;
	}

	// Find bounding boxes of collision shapes
	FKAggregateGeom& Geom = SourceMesh->BodySetup->AggGeom;
	FLOAT TotalVolume = 0.f;
	TArray<FBox> Boxes;
	for(INT i=0; i<Geom.ConvexElems.Num(); i++)
	{
		FBox ConvexBox = Geom.ConvexElems(i).ElemBox;
		Boxes.AddItem( ConvexBox );
		TotalVolume += ConvexBox.GetVolume();
	}

	for(INT i=0; i<Geom.BoxElems.Num(); i++)
	{
		FBox BoxBox = Geom.BoxElems(i).CalcAABB(FMatrix::Identity, 1.f);
		Boxes.AddItem( BoxBox );
		TotalVolume += BoxBox.GetVolume();
	}

	check(Boxes.Num() > 0);

	// For each box, add points within it.
	FractureCenters.Empty();
	for(INT BoxIdx=0; BoxIdx<Boxes.Num(); BoxIdx++)
	{
		const FBox& Box = Boxes(BoxIdx);
		INT NumChunksInBox = appRound((FLOAT)NumChunks * (Box.GetVolume()/TotalVolume));

		for(INT i=0; i<NumChunksInBox; i++)
		{
			FVector NewCenter;
			NewCenter.X = RandRange( Box.Min.X + 0.1f, Box.Max.X - 0.1f );
			NewCenter.Y = RandRange( Box.Min.Y + 0.1f, Box.Max.Y - 0.1f );
			NewCenter.Z = RandRange( Box.Min.Z + 0.1f, Box.Max.Z - 0.1f );

			FractureCenters.AddItem(NewCenter);
		}
	}

	// Clear selection
	ClearChunkSelection();

	// Redraw the viewport to show result.
	ViewportClient->Viewport->Invalidate();
}

/** Util to get a set of unique vertex positions from  a set of triangles. */
static void GetUniqueVertsFromSMTris(const FStaticMeshTriangle* RawTriangleData, INT NumSMTris, TArray<FVector>& OutVerts)
{
	for(INT i=0; i<NumSMTris; i++)
	{
		AddVertexIfNotPresent(OutVerts, RawTriangleData[i].Vertices[0]);
		AddVertexIfNotPresent(OutVerts, RawTriangleData[i].Vertices[1]);
		AddVertexIfNotPresent(OutVerts, RawTriangleData[i].Vertices[2]);
	}
}

/** Cluster existing points based on the vertices of the graphics mesh. */
void WxStaticMeshEditor::ClusterPoints()
{
	UFracturedStaticMesh* ExistingFracturedMesh = Cast<UFracturedStaticMesh>(StaticMesh);
	UStaticMesh* SourceMesh = StaticMesh;

	if (ExistingFracturedMesh)
	{
		SourceMesh = ExistingFracturedMesh->SourceStaticMesh;
	}

	if(!SourceMesh)
	{
		return;
	}

	// Make set of all verts in static mesh (removing duplicates)
	INT NumSMTris = SourceMesh->LODModels(0).RawTriangles.GetElementCount();
	const FStaticMeshTriangle* RawTriangleData = (FStaticMeshTriangle*)SourceMesh->LODModels(0).RawTriangles.Lock(LOCK_READ_ONLY);
	TArray<FVector> AllSMVerts;
	GetUniqueVertsFromSMTris(RawTriangleData, NumSMTris, AllSMVerts);
	SourceMesh->LODModels(0).RawTriangles.Unlock();

	// Use clustering to move chunk centers to where verts are.
	GenerateClusterCenters(FractureCenters, AllSMVerts, 10);
}

/** Generate regions that will be used to slice mesh. */
void WxStaticMeshEditor::ComputeRegions(const FVector& PlaneBias, UBOOL bResetInfos)
{
	UFracturedStaticMesh* ExistingFracturedMesh = Cast<UFracturedStaticMesh>(StaticMesh);
	UStaticMesh* SourceMesh = StaticMesh;

	if (ExistingFracturedMesh)
	{
		SourceMesh = ExistingFracturedMesh->SourceStaticMesh;
	}

	if(!SourceMesh)
	{
		return;
	}

	// Find bounding box of mesh
	FBox MeshBox = SourceMesh->Bounds.GetBox();
	// .. expand a little, to make sure the planes don't intersect the mesh
	MeshBox = MeshBox.ExpandBy(1.f);

	// Create convex voronoi regions for the mesh
	CreateVoronoiRegions(MeshBox, FractureCenters, PlaneBias, FractureChunks);

	if(bResetInfos || FractureInfos.Num() != FractureChunks.Num())
	{
		FractureInfos.Empty();
		FractureInfos.AddZeroed(FractureChunks.Num());
		for(INT i=0; i<FractureInfos.Num(); i++)
		{
			FractureInfos(i).bCanBeDestroyed = TRUE;
		}
	}

	// Mapping no longer valid.
	FinalToToolMap.Empty();

	// Redraw the viewport to show result.
	ViewportClient->Viewport->Invalidate();
}

/** Add some random noise to cluster centers */
void WxStaticMeshEditor::RandomizePoints(const FVector& RandScale)
{
	// Make set of points to move
	TArray<INT> PointsToMove;
	if(SelectedChunks.Num() == 0)
	{
		// If nothing selected, move all
		for(INT i=0; i<FractureCenters.Num(); i++)
		{
			PointsToMove.AddItem(i);
		}
	}
	else
	{
		// Move only selected set
		PointsToMove = SelectedChunks;
	}

	// Get box to keep points in
	FBox MeshBox = StaticMesh->Bounds.GetBox();

	// Move desired points
	for(INT i=0; i<PointsToMove.Num(); i++)
	{
		FVector RandOffset(0,0,0);

		RandOffset.X = -RandScale.X + ((2.f * RandScale.X) * appFrand());
		RandOffset.Y = -RandScale.Y + ((2.f * RandScale.Y) * appFrand());
		RandOffset.Z = -RandScale.Z + ((2.f * RandScale.Z) * appFrand());

		INT FracIndex = PointsToMove(i);
		FractureCenters(FracIndex) += RandOffset;

		// Ensure points are still inside mesh bounds
		ClampVertToBox(MeshBox, FractureCenters(FracIndex));
	}
}

/** Util to find the normal of the box face nearest to Vec */
static FVector FindNearestBoxFaceNormal(const FBox& Box, const FVector& Vec, FLOAT& OutDist)
{
	OutDist = BIG_NUMBER;
	FVector BestNormal(0,0,0);

	// X
	if((Box.Max.X - Vec.X) < OutDist)
	{
		BestNormal = FVector(1,0,0);
		OutDist = (Box.Max.X - Vec.X);
	}
	
	if((Vec.X - Box.Min.X) < OutDist)
	{
		BestNormal = FVector(-1,0,0);
		OutDist = (Vec.X - Box.Min.X);
	}

	// Y
	if((Box.Max.Y - Vec.Y) < OutDist)
	{
		BestNormal = FVector(0,1,0);
		OutDist = (Box.Max.Y - Vec.Y);
	}

	if((Vec.Y - Box.Min.Y) < OutDist)
	{
		BestNormal = FVector(0,-1,0);
		OutDist = (Vec.Y - Box.Min.Y);
	}

	// Z
	if((Box.Max.Z - Vec.Z) < OutDist)
	{
		BestNormal = FVector(0,0,1);
		OutDist = (Box.Max.Z - Vec.Z);
	}

	if((Vec.Z - Box.Min.Z) < OutDist)
	{
		BestNormal = FVector(0,0,-1);
		OutDist = (Vec.Z - Box.Min.Z);
	}

	return BestNormal;
}

/** Move points towards nearest bounds face */
void WxStaticMeshEditor::MovePointsTowardsNearestFace(const FVector& MoveAmount)
{
	// Make set of points to move
	TArray<INT> PointsToMove;
	if(SelectedChunks.Num() == 0)
	{
		// If nothing selected, move all
		for(INT i=0; i<FractureCenters.Num(); i++)
		{
			PointsToMove.AddItem(i);
		}
	}
	else
	{
		// Move only selected set
		PointsToMove = SelectedChunks;
	}

	// Get box to keep points in
	FBox MeshBox = StaticMesh->Bounds.GetBox();

	// Move desired points
	for(INT i=0; i<PointsToMove.Num(); i++)
	{
		INT FracIndex = PointsToMove(i);

		FLOAT Dist = 0.f;
		FVector Norm = FindNearestBoxFaceNormal( MeshBox, FractureCenters(FracIndex), Dist );

		FractureCenters(FracIndex) += (Norm * MoveAmount * Dist);
	}
}

static FLOAT SMTriArea(const FStaticMeshTriangle& Tri)
{
	FVector Side1 = Tri.Vertices[1] - Tri.Vertices[0];
	FVector Side2 = Tri.Vertices[2] - Tri.Vertices[0];
	FLOAT Area = (Side1 ^ Side2).Size() * 0.5f;
	return Area;
}

/** Util for doing work of clipping mesh against hull and generating new faces. */
void ClipMeshWithHull(	const TArray<FClipSMTriangle>& StaticMeshTriangles,
						const TArray<FPlane>& HullPlanes,
						const TArray<INT>& InNeighbours,
						INT InteriorMatIndex,
						TArray<FStaticMeshTriangle>& OutExteriorTris, 
						TArray<FStaticMeshTriangle>& OutInteriorTris,
						TArray<INT>& OutNeighbours,
						TArray<FLOAT>& OutNeighbourDims)
{
	check(InNeighbours.Num() == HullPlanes.Num());

	OutNeighbours.Empty();
	OutNeighbourDims.Empty();

	// Polys for each face of geom.
	TArray<FUtilPoly2DSet> HullPlanePolys;
	HullPlanePolys.AddZeroed(HullPlanes.Num());

	// Clip the input triangles against each of the hull's bounding planes.
	TArray<FClipSMTriangle> ClippedExteriorTriangles = StaticMeshTriangles;
	for(INT PlaneIdx=0; PlaneIdx<HullPlanes.Num(); PlaneIdx++)
	{
		FPlane SplitPlane = HullPlanes(PlaneIdx);

		// Clip exterior triangles with plane
		{
			TArray<FUtilEdge3D> TempEdges;
			TArray<FClipSMTriangle> NewClippedExteriorTriangles;
			// Clip graphics triangles to plane - accumulate edges from both sets of clipping
			ClipMeshWithPlane(NewClippedExteriorTriangles, TempEdges, ClippedExteriorTriangles, SplitPlane);
			// Now update ClippedExteriorTriangles to be this set of triangles and repeat.
			ClippedExteriorTriangles = NewClippedExteriorTriangles;
		}

		// Going to generate some new interior polys now, based on edges generated by clipping original full mesh
		TArray<FUtilEdge3D> ClipEdges;
		TArray<FClipSMTriangle> TempTris;
		ClipMeshWithPlane(TempTris, ClipEdges, StaticMeshTriangles, SplitPlane);

		// Project edges generate by clipping plane into 2D
		TArray<FUtilEdge2D> Edges2D;
		FUtilPoly2DSet& PolySet = HullPlanePolys(PlaneIdx);
		ProjectEdges(Edges2D, PolySet.PolyToWorld, ClipEdges, SplitPlane);

		// Find 2D closed polygons from this edge soup
		Buid2DPolysFromEdges(PolySet.Polys, Edges2D, FColor(255,255,255,255));

		// Cut away at this polygon with all other planes.
		for(INT OtherPlaneIdx=0; OtherPlaneIdx < HullPlanes.Num(); OtherPlaneIdx++)
		{
			if(OtherPlaneIdx != PlaneIdx)
			{
				Split2DPolysWithPlane(PolySet, HullPlanes(OtherPlaneIdx), FColor(255,255,255,255), FColor(255,255,255,0));
			}
		}
	}

	// Remove the redundant exterior triangles.
	RemoveRedundantTriangles(ClippedExteriorTriangles);

	// Convert the clipped exterior triangles into raw static mesh triangles.
	OutExteriorTris.Empty();
	GetRawStaticMeshTrianglesFromClipped(OutExteriorTris, ClippedExteriorTriangles);


	// Interior tris starts empty.
	OutInteriorTris.Empty();

	// Triangulate the closed polygons into 2D triangles - accumulated into array
	for(INT PlaneIdx=0; PlaneIdx<HullPlanePolys.Num(); PlaneIdx++)
	{
		FUtilPoly2DSet& PolySet = HullPlanePolys(PlaneIdx);

		if(PolySet.Polys.Num() > 0)
		{
			FLOAT TotalPolyArea = 0.f;

			for(INT PolyIdx=0; PolyIdx<PolySet.Polys.Num(); PolyIdx++)
			{
				// Generate UVs for the 2D polygon.
				FUtilPoly2D InteriorPolygon2D = PolySet.Polys(PolyIdx);
				GeneratePolyUVs(InteriorPolygon2D);

				// Transform the 2D polygon into 3D.
				FClipSMPolygon InteriorPolygon = Transform2DPolygonToSMPolygon(InteriorPolygon2D,PolySet.PolyToWorld);
				InteriorPolygon.MaterialIndex = InteriorMatIndex;
				InteriorPolygon.NumUVs = 1;
				InteriorPolygon.SmoothingMask = 0;

				// Triangulate the polygon.
				TArray<FClipSMTriangle> InteriorTriangles;
				TriangulatePoly(InteriorTriangles, InteriorPolygon);

				// Convert the clip triangles into static mesh triangles.
				TArray<FStaticMeshTriangle> NewInteriorSMTriangles;
				GetRawStaticMeshTrianglesFromClipped(NewInteriorSMTriangles,InteriorTriangles);
				for(INT TriangleIndex = 0; TriangleIndex < NewInteriorSMTriangles.Num(); TriangleIndex++)
				{
					const FStaticMeshTriangle& Triangle = NewInteriorSMTriangles(TriangleIndex);
					OutInteriorTris.AddItem(Triangle);
					TotalPolyArea += SMTriArea(Triangle);
				}
			}

			// Remove neighbours that have no graphics polys
			OutNeighbours.AddItem( InNeighbours(PlaneIdx) );
			// Update neighbour connection area based on graphics polys.
			OutNeighbourDims.AddItem( TotalPolyArea );
		}
		else
		{
			OutNeighbours.AddItem( INDEX_NONE );
			OutNeighbourDims.AddItem( 0.f );
		}
	}

	check(OutNeighbours.Num() == HullPlanePolys.Num());
}

/** Util for determining if a set of verts are all outside plane. */
static UBOOL VertsAreOutsidePlane(const TArray<FVector>& Verts, const FPlane& Plane, FLOAT MinDist)
{
	for(INT i=0; i<Verts.Num(); i++)
	{
		// Calculate distance of each vertex from plane. 
		// If negative, point is behind plane, so return FALSE.
		const FLOAT Dist = Plane.PlaneDot(Verts(i));
		if(Dist < MinDist)
		{
			return FALSE;
		}
	}

	// No vertex behind plane - all in front
	return TRUE;
}

/** Apply regions to cut actual mesh up */
void WxStaticMeshEditor::SliceMesh()
{
	if(bAdjustingCore)
	{
		return;
	}

	// Make sure the property window applies pending changes *before* we go and create the new mesh.  Otherwise,
	// when we switch to viewing the new mesh afterwards, the changes would be applied unexpectedly (crash!)
	PropertyWindow->FlushLastFocused();
	PropertyWindow->ClearLastFocused();


	UFracturedStaticMesh* ExistingFracturedMesh = Cast<UFracturedStaticMesh>(StaticMesh);
	UStaticMesh* SourceMesh = StaticMesh;
	UFracturedStaticMesh* NewFracturedMesh = NULL;

	if (ExistingFracturedMesh)
	{
		SourceMesh = ExistingFracturedMesh->SourceStaticMesh;
	}

	check(SourceMesh);
	if(SourceMesh->LODModels.Num() != 1)
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("FractureFailedInvalidLOD") );
		return;
	}

	if(FractureChunks.Num() == 0)
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("NoFractureInfo") );
		return;
	}

	// Backup info
	INT NumElems = StaticMesh->LODModels(0).Elements.Num();
	TArray<FStaticMeshLODElement> BackupElements;
	BackupElements.AddZeroed(NumElems);
	for(INT ElemIndex=0; ElemIndex<NumElems; ElemIndex++)
	{
		BackupElements(ElemIndex).Material				= StaticMesh->LODModels(0).Elements(ElemIndex).Material;
		BackupElements(ElemIndex).bEnableShadowCasting	= StaticMesh->LODModels(0).Elements(ElemIndex).bEnableShadowCasting;
		BackupElements(ElemIndex).bEnableCollision		= StaticMesh->LODModels(0).Elements(ElemIndex).EnableCollision;
	}


	UBOOL bHaveBackupData = FALSE;
	UParticleSystem* BackupFragmentDestroyEffect = NULL;
	TArray<UParticleSystem*> BackupFragmentDestroyEffects;
	FLOAT BackupFragmentDestroyEffectScale = 0.f;
	FLOAT BackupFragmentHealthScale = 0.f;
	FLOAT BackupFragmentMinHealth = 0.f;
	FLOAT BackupFragmentMaxHealth = 0.f;
	UBOOL bBackupUniformFragmentHealth = FALSE;
	FLOAT BackupChunkLinVel = 0.f;
	FLOAT BackupChunkAngVel = 0.f;
	FLOAT BackupChunkLinHorizontalScale = 0.f;
	UBOOL bBackupCompositeChunksExplodeOnImpact = FALSE;
	FLOAT BackupMinConnectionSupportArea = 0.f;
	FLOAT BackupNormalChanceOfPhysicsChunk = 1.f;
	FLOAT BackupExplosionChanceOfPhysicsChunk = 1.f;
	FVector2D BackupNormalPhysicsChunkScaleMinMax(1.0f, 1.0f);
	FVector2D BackupExplosionPhysicsChunkScaleMinMax(1.0f, 1.0f);
	UBOOL bBackupSliceUsingCoreCollision = FALSE;
	UMaterialInterface* BackupDynamicOutsideMaterial = NULL;
	UMaterialInterface* BackupLoseChunkOutsideMaterial = NULL;
	INT BackupOutsideMaterialIndex = 0;
	INT BackupLightMapResolution = 0;
	INT BackupLightMapCoordinateIndex = 0;
	FVector BackupPlaneBias(1,1,1);

	FVector BackupCoreOffset(0,0,0);
	FRotator BackupCoreRotation(0,0,0);
	FVector BackupCoreScale3D(1,1,1);
	FLOAT BackupCoreScale = 1.f;

	if(ExistingFracturedMesh)
	{
		BackupFragmentDestroyEffect = ExistingFracturedMesh->FragmentDestroyEffect;
		BackupFragmentDestroyEffects = ExistingFracturedMesh->FragmentDestroyEffects;
		BackupFragmentDestroyEffectScale = ExistingFracturedMesh->FragmentDestroyEffectScale;
		BackupFragmentHealthScale = ExistingFracturedMesh->FragmentHealthScale;
		BackupFragmentMinHealth = ExistingFracturedMesh->FragmentMinHealth;
		BackupFragmentMaxHealth = ExistingFracturedMesh->FragmentMaxHealth;
		bBackupUniformFragmentHealth = ExistingFracturedMesh->bUniformFragmentHealth;
		BackupChunkLinVel = ExistingFracturedMesh->ChunkLinVel;
		BackupChunkAngVel = ExistingFracturedMesh->ChunkAngVel;
		BackupChunkLinHorizontalScale = ExistingFracturedMesh->ChunkLinHorizontalScale;
		bBackupCompositeChunksExplodeOnImpact = ExistingFracturedMesh->bCompositeChunksExplodeOnImpact;
		BackupMinConnectionSupportArea = ExistingFracturedMesh->MinConnectionSupportArea;
		BackupNormalChanceOfPhysicsChunk = ExistingFracturedMesh->NormalChanceOfPhysicsChunk;
		BackupExplosionChanceOfPhysicsChunk = ExistingFracturedMesh->ExplosionChanceOfPhysicsChunk;
		BackupNormalPhysicsChunkScaleMinMax = FVector2D(ExistingFracturedMesh->NormalPhysicsChunkScaleMin, ExistingFracturedMesh->NormalPhysicsChunkScaleMax);
		BackupExplosionPhysicsChunkScaleMinMax = FVector2D(ExistingFracturedMesh->ExplosionPhysicsChunkScaleMin, ExistingFracturedMesh->ExplosionPhysicsChunkScaleMax);
		BackupPlaneBias = ExistingFracturedMesh->PlaneBias;
		bBackupSliceUsingCoreCollision = ExistingFracturedMesh->bSliceUsingCoreCollision;

		BackupDynamicOutsideMaterial = ExistingFracturedMesh->DynamicOutsideMaterial;
		BackupLoseChunkOutsideMaterial = ExistingFracturedMesh->LoseChunkOutsideMaterial;
		BackupOutsideMaterialIndex = ExistingFracturedMesh->OutsideMaterialIndex;
		
		BackupCoreOffset = ExistingFracturedMesh->CoreMeshOffset;
		BackupCoreRotation = ExistingFracturedMesh->CoreMeshRotation;
		BackupCoreScale3D = ExistingFracturedMesh->CoreMeshScale3D;
		BackupCoreScale = ExistingFracturedMesh->CoreMeshScale;

		BackupLightMapResolution = ExistingFracturedMesh->LightMapResolution;
		BackupLightMapCoordinateIndex = ExistingFracturedMesh->LightMapCoordinateIndex;

		bHaveBackupData = TRUE;
	}

	{
		// Need to re-attach all components using this mesh after this
		FStaticMeshComponentReattachContext ComponentReattachContext(ExistingFracturedMesh);

		// Source of existing static mesh triangles.
		TArray<FClipSMTriangle> StaticMeshTriangles;
		GetClippableStaticMeshTriangles(StaticMeshTriangles,SourceMesh);

		GWarn->BeginSlowTask( *LocalizeUnrealEd("FracturingMesh"), TRUE );


		// Set of all triangles from all chunks
		TArray<FStaticMeshTriangle> NewTotalTris;

		// Find the current max material number
		INT MaxMatIndex = 0;
		for(INT i=0; i<StaticMeshTriangles.Num(); i++)
		{
			MaxMatIndex = Max(StaticMeshTriangles(i).MaterialIndex, MaxMatIndex);
		}
		const INT InteriorMatIndex = MaxMatIndex + 1;

		// Create a new element for this material
		TArray<FStaticMeshElement> MeshElements = SourceMesh->LODModels(0).Elements;
		MeshElements.AddItem(FStaticMeshElement(NULL, InteriorMatIndex));

		INT NumChunks = FractureChunks.Num();

		// Find the convex hulls for the 'core' geometry, if present.
		TArray<FPlane> CoreConvexPlanes;
		if( bBackupSliceUsingCoreCollision &&
			ExistingFracturedMesh && 
			ExistingFracturedMesh->SourceCoreMesh && 
			ExistingFracturedMesh->SourceCoreMesh->BodySetup )
		{
			const FMatrix CoreTM = FScaleRotationTranslationMatrix( ExistingFracturedMesh->CoreMeshScale * ExistingFracturedMesh->CoreMeshScale3D, ExistingFracturedMesh->CoreMeshRotation, ExistingFracturedMesh->CoreMeshOffset );
			const FMatrix CoreTA = CoreTM.TransposeAdjoint();
			const FLOAT CoreDet = CoreTM.Determinant();

			// Iterate over each convex in the core.
			for(INT ConvexIdx=0; ConvexIdx<ExistingFracturedMesh->SourceCoreMesh->BodySetup->AggGeom.ConvexElems.Num(); ConvexIdx++)
			{
				const FKConvexElem& CoreConvex = ExistingFracturedMesh->SourceCoreMesh->BodySetup->AggGeom.ConvexElems(ConvexIdx);
				for(INT i=0; i<CoreConvex.FacePlaneData.Num(); i++)
				{
					// We need to scale the plane based on core mesh scaling.
					FPlane ScaledPlane = CoreConvex.FacePlaneData(i);
					ScaledPlane = ScaledPlane.TransformByUsingAdjointT(CoreTM, CoreDet, CoreTA);
					CoreConvexPlanes.AddItem(ScaledPlane);
				}
			}
		}

		TArray<FRawFragmentInfo> RawFragments; 

		// Reset table from tool chunks to final chunks.
		FinalToToolMap.Empty();

		// Iterate over each chunk clipping original static mesh triangles against it
		for(INT ChunkIdx=0; ChunkIdx<NumChunks; ChunkIdx++)
		{
			GWarn->StatusUpdatef( ChunkIdx, NumChunks,  *LocalizeUnrealEd(TEXT("Choppingizing")) );

			// Get the convex region to clip against.
			FKConvexElem& ConvexElem = FractureChunks(ChunkIdx).ConvexElem;
			const TArray<INT>& Neighbours = FractureChunks(ChunkIdx).Neighbours;

			TArray<FStaticMeshTriangle> ExteriorTris, InteriorTris;
			TArray<INT> NewNeighbours;
			TArray<FLOAT> NewNeighbourDims;
			ClipMeshWithHull(StaticMeshTriangles, ConvexElem.FacePlaneData, Neighbours, InteriorMatIndex, ExteriorTris, InteriorTris, NewNeighbours, NewNeighbourDims);

			// Discard any chunks which have no exterior triangles!
			if(ExteriorTris.Num() > 0)
			{
				// If we have a core, see if we can cut away a bit more of the chunks based on planes of its collision model
				// We only do this if a plane does not intersect the exterior geom verts.
				if(CoreConvexPlanes.Num() > 0)
				{
					// Get set of verts making up the exterior geom of this chunk
					TArray<FVector> ExteriorVerts;
					GetUniqueVertsFromSMTris((FStaticMeshTriangle*)ExteriorTris.GetData(), ExteriorTris.Num(), ExteriorVerts);

					// For each core plane, 
					TArray<FPlane> CoreClipPlanes;
					for(INT PIndex=0; PIndex<CoreConvexPlanes.Num(); PIndex++)
					{
						const FPlane& CorePlane = CoreConvexPlanes(PIndex);

						//.. see if plane intersects the convex, but all exterior verts are outside it.
						if(!ConvexElem.IsOutsidePlane(CorePlane) && VertsAreOutsidePlane(ExteriorVerts, CorePlane, 1.f))
						{
							// If so, flip (we want the area 'inside' this plane) add to set.
							CoreClipPlanes.AddItem(CorePlane.Flip());
						}
					}

					// If we have additional planes, re-slice mesh
					if(CoreClipPlanes.Num() > 0)
					{
						// New hull is old set of planes plus the new ones
						TArray<FPlane> ClippedConvexElemPlanes = ConvexElem.FacePlaneData;
						ClippedConvexElemPlanes += CoreClipPlanes;

						// Make neighbours array match with null entries
						TArray<INT> ClippedElemNeighbours = Neighbours;
						for(INT AddNIndex=0; AddNIndex<CoreClipPlanes.Num(); AddNIndex++)
						{
							ClippedElemNeighbours.AddItem(INDEX_NONE);
						}

						TArray<INT> ClippedNeighbours;
						TArray<FLOAT> ClippedNeighbourDims;

						// Re-clip using this hull - replace old results.
						ClipMeshWithHull(StaticMeshTriangles, ClippedConvexElemPlanes, ClippedElemNeighbours, InteriorMatIndex, ExteriorTris, InteriorTris, ClippedNeighbours, ClippedNeighbourDims);
					}
				}

				// Calculate average surface normal of exterior tris
				FVector ExteriorNormal = CalcAverageTriNormal(ExteriorTris);

				// Add all triangles from this chunk as a new fragment
				TArray<FStaticMeshTriangle> AllTris;
				AllTris += ExteriorTris;
				AllTris += InteriorTris;

				// Add fragment
				FRawFragmentInfo RawFrag(	FractureCenters(ChunkIdx), 
											ConvexElem, 
											NewNeighbours,
											NewNeighbourDims,
											FractureInfos(ChunkIdx).bCanBeDestroyed, 
											FractureInfos(ChunkIdx).bRootFragment, 
											FractureInfos(ChunkIdx).bNeverSpawnPhysics, 
											ExteriorNormal, 
											AllTris	);
				RawFragments.AddItem(RawFrag);

				// Record which element of FractureChunks this came from.
				FinalToToolMap.AddItem(ChunkIdx);

				// Update tool with more accurate neighour info
				FractureChunks(ChunkIdx).Neighbours = NewNeighbours;
				FractureChunks(ChunkIdx).NeighbourDims = NewNeighbourDims;
			}
		}

		// Fix up neighbour tables - currently is working in 'tool' chunks, not final 'graphics' chunks
		for(INT FragIdx=0; FragIdx<RawFragments.Num(); FragIdx++)
		{
			FRawFragmentInfo& RawFrag = RawFragments(FragIdx);
			// Copy and empty fragments neighbours array
			TArray<BYTE> ToolNeighbours = RawFrag.Neighbours;
			RawFrag.Neighbours.Empty();
			// See if each one is present in graphics version, fix up index if so.
			for(INT i=0; i<ToolNeighbours.Num(); i++)
			{
				BYTE ToolN = ToolNeighbours(i);
				if(ToolN == INDEX_NONE)
				{
					RawFrag.Neighbours.AddItem(255);
				}
				else
				{
					INT FinalN = FinalToToolMap.FindItemIndex(ToolN);
					if(FinalN != INDEX_NONE)
					{
						RawFrag.Neighbours.AddItem((BYTE)FinalN);
					}
					else
					{
						RawFrag.Neighbours.AddItem(255); // Indicates 'null' neighbour
					}
				}
			}
		}

		GWarn->StatusUpdatef( 0, 1,  *LocalizeUnrealEd(TEXT("BuildingMesh")) );

		// Grab a pointer to the BodySetup to use for the new fractured mesh
		URB_BodySetup* BodySetup = NULL;
		if(SourceMesh->BodySetup)
		{
			BodySetup = SourceMesh->BodySetup;
		}

		if (ExistingFracturedMesh)
		{
			// Back up pointer to core mesh before clobbering FracturedStaticMesh
			UStaticMesh* CoreMesh = ExistingFracturedMesh->SourceCoreMesh;
			FLOAT CoreMeshScale = ExistingFracturedMesh->CoreMeshScale;
			FVector CoreMeshScale3D = ExistingFracturedMesh->CoreMeshScale3D;
			FVector CoreMeshOffset = ExistingFracturedMesh->CoreMeshOffset;
			FRotator CoreMeshRotation = ExistingFracturedMesh->CoreMeshRotation;

			// Overwrite the existing fractured mesh by creating a new one with the same name
			NewFracturedMesh = UFracturedStaticMesh::CreateFracturedStaticMesh(
				ExistingFracturedMesh->GetOuter(), 
				*ExistingFracturedMesh->GetName(), 
				ExistingFracturedMesh->GetFlags(),
				RawFragments,
				SourceMesh->LODInfo(0),
				InteriorMatIndex,
				MeshElements,
				ExistingFracturedMesh);

			// Re-merge the core mesh in.
			if(CoreMesh)
			{
				DoMergeStaticMesh(CoreMesh, CoreMeshOffset, CoreMeshRotation, CoreMeshScale, CoreMeshScale3D);
			}
		}
		else
		{
			// pop up the factory new dialog so the user can pick name/package
			FString DefaultPackage = *StaticMesh->GetOutermost()->GetName();
			FString DefaultGroup = StaticMesh->GetOuter()->GetOuter() ? StaticMesh->GetFullGroupName(TRUE) : TEXT("");
			FString DefaultName = StaticMesh->GetName() + TEXT("_FRACTURED");

			WxDlgPackageGroupName dlg;
			if( dlg.ShowModal(DefaultPackage, DefaultGroup, DefaultName) == wxID_OK )
			{
				UPackage* NewPackage = NULL;
				FString NewObjName;
				UBOOL bValidPackage = dlg.ProcessNewAssetDlg(&NewPackage, &NewObjName, FALSE, UFracturedStaticMesh::StaticClass());
				if(bValidPackage)
				{
					NewFracturedMesh = UFracturedStaticMesh::CreateFracturedStaticMesh(
						NewPackage, 
						*NewObjName, 
						StaticMesh->GetFlags(),
						RawFragments,
						StaticMesh->LODInfo(0),
						InteriorMatIndex,
						MeshElements,
						NULL);

					// Update the generic browser since we just created an asset
					WxGenericBrowser* GenericBrowser = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT("GenericBrowser") );
					check(GenericBrowser);
					GenericBrowser->Update();
				}
			}
		}

		// Copy simplified collision from source mesh
		if(NewFracturedMesh)
		{
			NewFracturedMesh->SourceStaticMesh = SourceMesh;

			if(BodySetup)
			{
				NewFracturedMesh->BodySetup = CastChecked<URB_BodySetup>( UObject::StaticDuplicateObject(BodySetup, BodySetup, NewFracturedMesh, TEXT("None")) );
			}

			// Copy section info (material etc)
			INT NumCopyElems = Min(NewFracturedMesh->LODModels(0).Elements.Num(), BackupElements.Num());
			for(INT ElemIndex = 0; ElemIndex<NumCopyElems; ElemIndex++)
			{
				NewFracturedMesh->LODModels(0).Elements(ElemIndex).Material				= BackupElements(ElemIndex).Material;
				NewFracturedMesh->LODModels(0).Elements(ElemIndex).bEnableShadowCasting	= BackupElements(ElemIndex).bEnableShadowCasting;
				NewFracturedMesh->LODModels(0).Elements(ElemIndex).EnableCollision		= BackupElements(ElemIndex).bEnableCollision;	
			}

			// Restore info if present
			if(bHaveBackupData)
			{
				NewFracturedMesh->FragmentDestroyEffect = BackupFragmentDestroyEffect;
				NewFracturedMesh->FragmentDestroyEffects = BackupFragmentDestroyEffects;
				NewFracturedMesh->FragmentDestroyEffectScale = BackupFragmentDestroyEffectScale;
				NewFracturedMesh->FragmentHealthScale = BackupFragmentHealthScale;
				NewFracturedMesh->FragmentMinHealth = BackupFragmentMinHealth;
				NewFracturedMesh->FragmentMaxHealth = BackupFragmentMaxHealth;
				NewFracturedMesh->bUniformFragmentHealth = bBackupUniformFragmentHealth;
				NewFracturedMesh->ChunkLinVel = BackupChunkLinVel;
				NewFracturedMesh->ChunkAngVel = BackupChunkAngVel;
				NewFracturedMesh->ChunkLinHorizontalScale = BackupChunkLinHorizontalScale;
				NewFracturedMesh->bCompositeChunksExplodeOnImpact = bBackupCompositeChunksExplodeOnImpact;
				NewFracturedMesh->MinConnectionSupportArea = BackupMinConnectionSupportArea;
				NewFracturedMesh->NormalChanceOfPhysicsChunk = BackupNormalChanceOfPhysicsChunk;
				NewFracturedMesh->ExplosionChanceOfPhysicsChunk = BackupExplosionChanceOfPhysicsChunk;
				NewFracturedMesh->NormalPhysicsChunkScaleMin = BackupNormalPhysicsChunkScaleMinMax.X;
				NewFracturedMesh->NormalPhysicsChunkScaleMax = BackupNormalPhysicsChunkScaleMinMax.Y;
				NewFracturedMesh->ExplosionPhysicsChunkScaleMin = BackupExplosionPhysicsChunkScaleMinMax.X;
				NewFracturedMesh->ExplosionPhysicsChunkScaleMax = BackupExplosionPhysicsChunkScaleMinMax.Y;
				NewFracturedMesh->PlaneBias = BackupPlaneBias;
				NewFracturedMesh->bSliceUsingCoreCollision = bBackupSliceUsingCoreCollision;

				NewFracturedMesh->DynamicOutsideMaterial = BackupDynamicOutsideMaterial;
				NewFracturedMesh->LoseChunkOutsideMaterial = BackupLoseChunkOutsideMaterial;
				NewFracturedMesh->OutsideMaterialIndex = BackupOutsideMaterialIndex;

				NewFracturedMesh->CoreMeshOffset = BackupCoreOffset;
				NewFracturedMesh->CoreMeshRotation = BackupCoreRotation;
				NewFracturedMesh->CoreMeshScale3D = BackupCoreScale3D;
				NewFracturedMesh->CoreMeshScale = BackupCoreScale;
				
				NewFracturedMesh->LightMapResolution =  BackupLightMapResolution;
				NewFracturedMesh->LightMapCoordinateIndex = BackupLightMapCoordinateIndex;
			}
			else
			{
				NewFracturedMesh->LightMapResolution = SourceMesh->LightMapResolution;
				NewFracturedMesh->LightMapCoordinateIndex = SourceMesh->LightMapCoordinateIndex;
			}
			
			// Copy light-map info.
			if(NewFracturedMesh->LightMapResolution > 0)
			{
				GWarn->StatusUpdatef( 0, 1,  *LocalizeUnrealEd(TEXT("GenerateUVsProgressText")) );

				// Generate unique UV coordinates for the new triangles if the source mesh had light-map coordinates.
				FLOAT MaxStretch = 0.9f;
				UINT NumGeneratedCharts = 0;
				GenerateUVs(NewFracturedMesh, 0, NewFracturedMesh->LightMapCoordinateIndex, TRUE, MaxStretch, NumGeneratedCharts);
			}

			// Update slicing version number
			NewFracturedMesh->NonCriticalBuildVersion = FSMNonCriticalBuildVersion;
			NewFracturedMesh->LicenseeNonCriticalBuildVersion = LicenseeFSMNonCriticalBuildVersion;
		}
	}

	GWarn->EndSlowTask();

	// Bail out if no mesh created
	if(!NewFracturedMesh)
	{
		return;
	}

	// Make sure view stays the same when doing this
	NewFracturedMesh->ThumbnailAngle = SourceMesh->ThumbnailAngle;
	NewFracturedMesh->ThumbnailDistance = SourceMesh->ThumbnailDistance;

	FVector ViewLoc = ViewportClient->ViewLocation;
	FRotator ViewRot = ViewportClient->ViewRotation;
	FVector BackupLockLoc = ViewportClient->LockLocation;
	FRotator BackupLockRot = ViewportClient->LockRot;
	FMatrix BackupTM = ViewportClient->StaticMeshComponent->LocalToWorld;
	SetEditorMesh(NewFracturedMesh);
	ViewportClient->StaticMeshComponent->ConditionalUpdateTransform(BackupTM);
	ViewportClient->ViewLocation = ViewLoc;
	ViewportClient->ViewRotation = ViewRot;
	ViewportClient->LockLocation = BackupLockLoc;
	ViewportClient->LockRot = BackupLockRot;

	// Mark package dirty 
	NewFracturedMesh->MarkPackageDirty();

	// Redraw the viewport to show result.
	ViewportClient->Viewport->Invalidate();
	UpdateToolbars();
}

//////////////////////////////////////////////////////////////////////////
// Fracture Options

BEGIN_EVENT_TABLE( WxFractureToolOptions, wxDialog )
	EVT_TEXT_ENTER( ID_FRAC_XPLANEBIAS, WxFractureToolOptions::OnPlaneBiasChangeX )
	EVT_TEXT_ENTER( ID_FRAC_YPLANEBIAS, WxFractureToolOptions::OnPlaneBiasChangeY )
	EVT_TEXT_ENTER( ID_FRAC_ZPLANEBIAS, WxFractureToolOptions::OnPlaneBiasChangeZ )
	EVT_BUTTON( ID_FRAC_GENERATE, WxFractureToolOptions::OnGeneratePoints )
	EVT_BUTTON( ID_FRAC_RANDOMIZE, WxFractureToolOptions::OnRandomize )
	EVT_BUTTON( ID_FRAC_MOVETOFACES, WxFractureToolOptions::OnMoveToFaces )
	EVT_BUTTON( ID_FRAC_SLICE, WxFractureToolOptions::OnSlice )
	EVT_CHECKBOX( ID_FRAC_SHOWCUTS, WxFractureToolOptions::OnShowCutsChange )
	EVT_CHECKBOX( ID_FRAC_SHOWCUTSSOLID, WxFractureToolOptions::OnShowCutsSolidChange )
	EVT_CHECKBOX( ID_FRAC_SHOWONLYVISCUTS, WxFractureToolOptions::OnShowOnlyVisibleCuts )
	EVT_COMBOBOX( ID_FRAC_COLORMODE, WxFractureToolOptions::OnColorModeChange )
	EVT_CHECKBOX( ID_FRAC_SHOWCORE, WxFractureToolOptions::OnShowCoreChange )
	EVT_BUTTON( wxID_CLOSE, WxFractureToolOptions::OnPressClose )
	EVT_COMMAND_SCROLL( ID_FRAC_VIEWCHUNK, WxFractureToolOptions::OnViewChunkChange )
	EVT_COMBOBOX( ID_FRAC_VIEWCHUNKMODE, WxFractureToolOptions::OnViewChunkModeChange )
	EVT_CHECKBOX( ID_FRAC_CHANGEOPT, WxFractureToolOptions::OnChunkOptChange )
	EVT_BUTTON( ID_FRAC_GROWSEL, WxFractureToolOptions::OnGrowSelection )
	EVT_BUTTON( ID_FRAC_SHRINKSEL, WxFractureToolOptions::OnShrinkSelection )
	EVT_BUTTON( ID_FRAC_SELECTTOP, WxFractureToolOptions::OnSelectTop )
	EVT_BUTTON( ID_FRAC_SELECTBOTTOM, WxFractureToolOptions::OnSelectBottom )
	EVT_BUTTON( ID_FRAC_INVERTSELECTION, WxFractureToolOptions::OnInvertSelection )
	EVT_BUTTON( ID_FRAC_ADDCORE, WxFractureToolOptions::OnAddCore )
	EVT_BUTTON( ID_FRAC_REMOVECORE, WxFractureToolOptions::OnRemoveCore )
	EVT_BUTTON( ID_FRAC_ACCEPTCORE, WxFractureToolOptions::OnAcceptCore )
	EVT_BUTTON( ID_FRAC_CANCELCORE, WxFractureToolOptions::OnCancelCore )
	EVT_CLOSE( WxFractureToolOptions::OnClose )
END_EVENT_TABLE()

/** Constructor - create sliders/buttons. */
WxFractureToolOptions::WxFractureToolOptions( WxStaticMeshEditor* InSME )
: wxDialog( InSME, -1, wxString(*LocalizeUnrealEd("FractureTool")), wxDefaultPosition, wxSize(530, 900), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER )
{
	FWindowUtil::LoadPosSize( TEXT("FractureTool"), this, -1, -1, 530, 900 );

	// Save pointer to StaticMeshEditor
	Editor = InSME;

	UFracturedStaticMesh* FracMesh = Cast<UFracturedStaticMesh>(Editor->StaticMesh);

	bShowCuts = TRUE;
	bShowCutsSolid = FALSE;
	bShowOnlyVisibleCuts = FALSE;
	ColorMode = ESCM_Random;
	bShowCore = TRUE;

	// Create all the widgets
	wxBoxSizer* TopVSizer = new wxBoxSizer(wxVERTICAL);
	{
		////// GENERATE //////
		wxStaticBoxSizer* GenerateSizerH = new wxStaticBoxSizer(wxHORIZONTAL, this, *LocalizeUnrealEd("GenerateChunks"));
		{
			wxFlexGridSizer* SliderGrid = new wxFlexGridSizer(2, 2, 0, 0);
			{
				SliderGrid->AddGrowableCol(1);

				wxStaticText* NumChunkText = new wxStaticText( this, wxID_STATIC, *LocalizeUnrealEd("NumChunks"), wxDefaultPosition, wxDefaultSize, 0 );
				SliderGrid->Add(NumChunkText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

				NumChunkSlider = new wxSlider( this, ID_FRAC_NUMCHUNKS, 5, 1, 150, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS );
				SliderGrid->Add(NumChunkSlider, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

				// Set slider to current number of chunks
				if( Editor->FractureCenters.Num() > 0)
				{
					NumChunkSlider->SetValue(Editor->FractureCenters.Num());
				}

				SliderGrid->AddStretchSpacer(0);

				ComputeButton = new wxButton( this, ID_FRAC_GENERATE, *LocalizeUnrealEd("Generate"), wxDefaultPosition, wxDefaultSize, 0 );
				SliderGrid->Add(ComputeButton, 0, wxALIGN_LEFT|wxALL, 5);
			}
			GenerateSizerH->Add(SliderGrid, 1, wxGROW|wxALL, 5);
		}
		TopVSizer->Add(GenerateSizerH, 0, wxGROW|wxALL, 5);

		////// PLANE BIAS //////
		wxStaticBoxSizer* ShapeSizerH = new wxStaticBoxSizer(wxHORIZONTAL, this, *LocalizeUnrealEd("ChunkShape"));
		{
			wxFlexGridSizer* SliderGrid = new wxFlexGridSizer(1, 2, 0, 0);
			{
				SliderGrid->AddGrowableCol(1);

				wxStaticText* PlaneBiasText = new wxStaticText( this, wxID_STATIC, *LocalizeUnrealEd("PlaneBias"), wxDefaultPosition, wxDefaultSize, 0 );
				SliderGrid->Add(PlaneBiasText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

				wxBoxSizer* PlaneBiasSizer = new wxBoxSizer(wxHORIZONTAL);
				{
					FVector CurrentPlaneBias = FVector(1,1,1);
					if(FracMesh)
					{
						CurrentPlaneBias = FracMesh->PlaneBias;
					}

					FString CompText = FString::Printf(TEXT("%3.1f"), CurrentPlaneBias.X);
					XBiasEntry = new wxTextCtrl( this, ID_FRAC_XPLANEBIAS, *CompText, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
					PlaneBiasSizer->Add(XBiasEntry, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 2);
					XBiasEntry->Connect(wxEVT_KILL_FOCUS, wxCommandEventHandler(WxFractureToolOptions::OnPlaneBiasLoseFocus), NULL, this);

					CompText = FString::Printf(TEXT("%3.1f"), CurrentPlaneBias.Y);
					YBiasEntry = new wxTextCtrl( this, ID_FRAC_YPLANEBIAS, *CompText, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
					PlaneBiasSizer->Add(YBiasEntry, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 2);
					YBiasEntry->Connect(wxEVT_KILL_FOCUS, wxCommandEventHandler(WxFractureToolOptions::OnPlaneBiasLoseFocus), NULL, this);

					CompText = FString::Printf(TEXT("%3.1f"), CurrentPlaneBias.Z);
					ZBiasEntry = new wxTextCtrl( this, ID_FRAC_ZPLANEBIAS, *CompText, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
					PlaneBiasSizer->Add(ZBiasEntry, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 2);
					ZBiasEntry->Connect(wxEVT_KILL_FOCUS, wxCommandEventHandler(WxFractureToolOptions::OnPlaneBiasLoseFocus), NULL, this);
				}
				SliderGrid->Add(PlaneBiasSizer, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 1);
			}
			ShapeSizerH->Add(SliderGrid, 1, wxGROW|wxALL, 5);
		}
		TopVSizer->Add(ShapeSizerH, 0, wxGROW|wxALL, 5);

		////// RANDOMIZER //////
		wxStaticBoxSizer* NoiseSizerH = new wxStaticBoxSizer(wxHORIZONTAL, this, *LocalizeUnrealEd("ModifyPoints"));
		{
			wxFlexGridSizer* SliderGrid = new wxFlexGridSizer(2, 2, 0, 0);
			{
				SliderGrid->AddGrowableCol(1);

				wxStaticText* RandomizeText = new wxStaticText( this, wxID_STATIC, *LocalizeUnrealEd("Amount"), wxDefaultPosition, wxDefaultSize, 0 );
				SliderGrid->Add(RandomizeText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

				wxBoxSizer* RandomScaleSizer = new wxBoxSizer(wxHORIZONTAL);
				{
					XRandScaleEntry = new wxTextCtrl( this, -1, TEXT("1.0") );
					RandomScaleSizer->Add(XRandScaleEntry, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 2);

					YRandScaleEntry = new wxTextCtrl( this, -1, TEXT("1.0") );
					RandomScaleSizer->Add(YRandScaleEntry, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 2);

					ZRandScaleEntry = new wxTextCtrl( this, -1, TEXT("1.0") );
					RandomScaleSizer->Add(ZRandScaleEntry, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 2);
				}
				SliderGrid->Add(RandomScaleSizer, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 1);

				SliderGrid->AddStretchSpacer(0);

				wxBoxSizer* ButtonBoxSizer = new wxBoxSizer(wxHORIZONTAL);
				{
					RandomizerButton = new wxButton( this, ID_FRAC_RANDOMIZE, *LocalizeUnrealEd("Randomize"), wxDefaultPosition, wxDefaultSize, 0 );
					ButtonBoxSizer->Add(RandomizerButton, 0, wxALIGN_LEFT|wxALL, 5);

					MoveToFaceButton = new wxButton( this, ID_FRAC_MOVETOFACES, *LocalizeUnrealEd("MoveToFaces"), wxDefaultPosition, wxDefaultSize, 0 );
					ButtonBoxSizer->Add(MoveToFaceButton, 0, wxALIGN_LEFT|wxALL, 5);
				}
				SliderGrid->Add(ButtonBoxSizer, 1, wxGROW|wxALL, 5);
			}
			NoiseSizerH->Add(SliderGrid, 1, wxGROW|wxALL, 5);
		}
		TopVSizer->Add(NoiseSizerH, 0, wxGROW|wxALL, 5);

		////// CHUNK OPTIONS //////
		wxStaticBoxSizer* ChunkOptSizerH = new wxStaticBoxSizer(wxHORIZONTAL, this, *LocalizeUnrealEd("ChunkOptions"));
		{
			wxFlexGridSizer* SliderGrid = new wxFlexGridSizer(2, 2, 0, 0);
			{
				SliderGrid->AddGrowableCol(1);

				// DEstroyable
				wxStaticText* DestroyableText = new wxStaticText( this, wxID_STATIC, *LocalizeUnrealEd("Destroyable"), wxDefaultPosition, wxDefaultSize, 0 );
				SliderGrid->Add(DestroyableText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

				DestroyableBox = new wxCheckBox( this, ID_FRAC_CHANGEOPT, TEXT(""), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE );
				DestroyableBox->SetValue(TRUE);
				DestroyableBox->Enable(FALSE);
				SliderGrid->Add(DestroyableBox, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

				// Root chunk
				wxStaticText* RootText = new wxStaticText( this, wxID_STATIC, *LocalizeUnrealEd("RootChunk"), wxDefaultPosition, wxDefaultSize, 0 );
				SliderGrid->Add(RootText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

				RootChunkBox = new wxCheckBox( this, ID_FRAC_CHANGEOPT, TEXT(""), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE );
				RootChunkBox->Enable(FALSE);
				SliderGrid->Add(RootChunkBox, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

				// NoPhysics
				wxStaticText* NoPhysText = new wxStaticText( this, wxID_STATIC, *LocalizeUnrealEd("NoPhysChunk"), wxDefaultPosition, wxDefaultSize, 0 );
				SliderGrid->Add(NoPhysText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

				NoPhysBox = new wxCheckBox( this, ID_FRAC_CHANGEOPT, TEXT(""), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE );
				NoPhysBox->Enable(FALSE);
				SliderGrid->Add(NoPhysBox, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
			}
			ChunkOptSizerH->Add(SliderGrid, 1, wxGROW|wxALL, 5);
		}
		TopVSizer->Add(ChunkOptSizerH, 0, wxGROW|wxALL, 5);

		////// SELECTION //////
		wxStaticBoxSizer* SelectionSizerH = new wxStaticBoxSizer(wxHORIZONTAL, this, *LocalizeUnrealEd("ChunkSelection"));
		{
			wxBoxSizer* ButtonBoxSizer = new wxBoxSizer(wxHORIZONTAL);
			{
				GrowSelButton = new wxButton( this, ID_FRAC_GROWSEL, *LocalizeUnrealEd("GrowSelection"), wxDefaultPosition, wxDefaultSize, 0 );
				ButtonBoxSizer->Add(GrowSelButton, 0, wxALIGN_BOTTOM|wxALL, 5);

				ShrinkSelButton = new wxButton( this, ID_FRAC_SHRINKSEL, *LocalizeUnrealEd("ShrinkSelection"), wxDefaultPosition, wxDefaultSize, 0 );
				ButtonBoxSizer->Add(ShrinkSelButton, 0, wxALIGN_BOTTOM|wxALL, 5);

				SelectTopButton = new wxButton( this, ID_FRAC_SELECTTOP, *LocalizeUnrealEd("SelectTop"), wxDefaultPosition, wxDefaultSize, 0 );
				ButtonBoxSizer->Add(SelectTopButton, 0, wxALIGN_BOTTOM|wxALL, 5);

				SelectBottomButton = new wxButton( this, ID_FRAC_SELECTBOTTOM, *LocalizeUnrealEd("SelectBottom"), wxDefaultPosition, wxDefaultSize, 0 );
				ButtonBoxSizer->Add(SelectBottomButton, 0, wxALIGN_BOTTOM|wxALL, 5);

				InvertSelectionButton = new wxButton( this, ID_FRAC_INVERTSELECTION, *LocalizeUnrealEd("InvertSelection"), wxDefaultPosition, wxDefaultSize, 0 );
				ButtonBoxSizer->Add(InvertSelectionButton, 0, wxALIGN_BOTTOM|wxALL, 5);
			}
			SelectionSizerH->Add(ButtonBoxSizer, 1, wxGROW|wxALL, 5);
		}
		TopVSizer->Add(SelectionSizerH, 0, wxGROW|wxALL, 5);

		////// CHUNK VIEWING OPTIONS //////
		wxStaticBoxSizer* ChunkViewSizerH = new wxStaticBoxSizer(wxHORIZONTAL, this, *LocalizeUnrealEd("Viewing"));
		{
			wxFlexGridSizer* SliderGrid = new wxFlexGridSizer(7, 2, 0, 0);
			{
				SliderGrid->AddGrowableCol(1);

				// View chunk slider
				wxStaticText* ShowChunkText = new wxStaticText( this, wxID_STATIC, *LocalizeUnrealEd("ViewChunk"), wxDefaultPosition, wxDefaultSize, 0 );
				SliderGrid->Add(ShowChunkText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

				ViewChunkSlider = new wxSlider( this, ID_FRAC_VIEWCHUNK, 0, 0, 1, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS );
				SliderGrid->Add(ViewChunkSlider, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

				// View chunk mode
				wxStaticText* ChunkModeText = new wxStaticText( this, wxID_STATIC, *LocalizeUnrealEd("ViewChunkMode"), wxDefaultPosition, wxDefaultSize, 0 );
				SliderGrid->Add(ChunkModeText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

				ChunkViewModeCombo = new wxComboBox( this, ID_FRAC_VIEWCHUNKMODE, TEXT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
				ChunkViewModeCombo->Append( *LocalizeUnrealEd("ViewOnly") );
				ChunkViewModeCombo->Append( *LocalizeUnrealEd("ViewAllBut") );
				ChunkViewModeCombo->Append( *LocalizeUnrealEd("ViewUpTo") );
				ChunkViewModeCombo->SetSelection(0);
				SliderGrid->Add(ChunkViewModeCombo, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

				// Show cuts
				wxStaticText* ShowCutsText = new wxStaticText( this, wxID_STATIC, *LocalizeUnrealEd("ShowCuts"), wxDefaultPosition, wxDefaultSize, 0 );
				SliderGrid->Add(ShowCutsText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

				ShowCutsBox = new wxCheckBox( this, ID_FRAC_SHOWCUTS, TEXT("") );
				ShowCutsBox->SetValue(TRUE);
				SliderGrid->Add(ShowCutsBox, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

				// Show cuts solid
				wxStaticText* ShowCutsSolidText = new wxStaticText( this, wxID_STATIC, *LocalizeUnrealEd("ShowCutsSolid"), wxDefaultPosition, wxDefaultSize, 0 );
				SliderGrid->Add(ShowCutsSolidText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

				ShowCutsSolidBox = new wxCheckBox( this, ID_FRAC_SHOWCUTSSOLID, TEXT("") );
				SliderGrid->Add(ShowCutsSolidBox, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

				// Show visible cuts
				wxStaticText* ShowVisibleCutsText = new wxStaticText( this, wxID_STATIC, *LocalizeUnrealEd("ShowVisibleCuts"), wxDefaultPosition, wxDefaultSize, 0 );
				SliderGrid->Add(ShowVisibleCutsText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

				ShowVisibleCutsBox = new wxCheckBox( this, ID_FRAC_SHOWONLYVISCUTS, TEXT("") );
				SliderGrid->Add(ShowVisibleCutsBox, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

				// Chunk color mode
				wxStaticText* ColorModeText = new wxStaticText( this, wxID_STATIC, *LocalizeUnrealEd("SliceColorMode"), wxDefaultPosition, wxDefaultSize, 0 );
				SliderGrid->Add(ColorModeText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

				ColorModeCombo = new wxComboBox( this, ID_FRAC_COLORMODE, TEXT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
				ColorModeCombo->Append( *LocalizeUnrealEd("Random") );
				ColorModeCombo->Append( *LocalizeUnrealEd("SupportChunks") );
				ColorModeCombo->Append( *LocalizeUnrealEd("DestroyableChunks") );
				ColorModeCombo->Append( *LocalizeUnrealEd("NoPhysChunks") );
				ColorModeCombo->SetSelection(0);
				SliderGrid->Add(ColorModeCombo, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

				// Show core 
				wxStaticText* ShowCoreText = new wxStaticText( this, wxID_STATIC, *LocalizeUnrealEd("ShowCore"), wxDefaultPosition, wxDefaultSize, 0 );
				SliderGrid->Add(ShowCoreText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

				ShowCoreBox = new wxCheckBox( this, ID_FRAC_SHOWCORE, TEXT("") );
				ShowCoreBox->SetValue(TRUE);
				SliderGrid->Add(ShowCoreBox, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
			}
			ChunkViewSizerH->Add(SliderGrid, 1, wxGROW|wxALL, 5);
		}
		TopVSizer->Add(ChunkViewSizerH, 0, wxGROW|wxALL, 5);

		////// CORE CONTROL //////
		wxStaticBoxSizer* CoreControlSizerH = new wxStaticBoxSizer(wxHORIZONTAL, this, *LocalizeUnrealEd("Core"));
		{
			wxBoxSizer* ButtonBoxSizer = new wxBoxSizer(wxHORIZONTAL);
			{
				AddCoreButton = new wxButton( this, ID_FRAC_ADDCORE, *LocalizeUnrealEd("AddCore"), wxDefaultPosition, wxDefaultSize, 0 );
				ButtonBoxSizer->Add(AddCoreButton, 0, wxALIGN_BOTTOM|wxALL, 5);

				RemoveCoreButton = new wxButton( this, ID_FRAC_REMOVECORE, *LocalizeUnrealEd("RemoveCore"), wxDefaultPosition, wxDefaultSize, 0 );
				ButtonBoxSizer->Add(RemoveCoreButton, 0, wxALIGN_BOTTOM|wxALL, 5);

				AcceptCoreButton = new wxButton( this, ID_FRAC_ACCEPTCORE, *LocalizeUnrealEd("AcceptCore"), wxDefaultPosition, wxDefaultSize, 0 );
				ButtonBoxSizer->Add(AcceptCoreButton, 0, wxALIGN_BOTTOM|wxALL, 5);

				CancelCoreButton = new wxButton( this, ID_FRAC_CANCELCORE, *LocalizeUnrealEd("CancelCore"), wxDefaultPosition, wxDefaultSize, 0 );
				ButtonBoxSizer->Add(CancelCoreButton, 0, wxALIGN_BOTTOM|wxALL, 5);
			}
			CoreControlSizerH->Add(ButtonBoxSizer, 1, wxGROW|wxALL, 5);
		}
		TopVSizer->Add(CoreControlSizerH, 0, wxGROW|wxALL, 5);

		////// BUTTONS //////
		wxBoxSizer* ButtonBoxSizer = new wxBoxSizer(wxHORIZONTAL);
		{
			ApplyButton = new wxButton( this, ID_FRAC_SLICE, *LocalizeUnrealEd("Slice"), wxDefaultPosition, wxDefaultSize, 0 );
			ButtonBoxSizer->Add(ApplyButton, 0, wxALIGN_BOTTOM|wxALL, 5);

			CloseButton = new wxButton( this, wxID_CLOSE, *LocalizeUnrealEd("Close"), wxDefaultPosition, wxDefaultSize, 0 );
			ButtonBoxSizer->Add(CloseButton, 0, wxALIGN_BOTTOM|wxALL, 5);
		}
		TopVSizer->Add(ButtonBoxSizer, 1, wxGROW|wxALL, 5);
	}
	SetSizer(TopVSizer);

	UpdateCoreButtonStates();

	UFracturedStaticMesh* FracturedMesh = Cast<UFracturedStaticMesh>(InSME->StaticMesh);
	if (FracturedMesh)
	{
		UpdateViewChunkSlider(FracturedMesh->GetNumFragments());
	}
	else
	{
		UpdateViewChunkSlider(100);
	}
}

void WxFractureToolOptions::OnPlaneBiasChangeX(wxCommandEvent& In)
{
	RegenRegions(FALSE);
	XBiasEntry->SetSelection(-1,-1);
}

void WxFractureToolOptions::OnPlaneBiasChangeY(wxCommandEvent& In)
{
	RegenRegions(FALSE);
	YBiasEntry->SetSelection(-1,-1);
}

void WxFractureToolOptions::OnPlaneBiasChangeZ(wxCommandEvent& In)
{
	RegenRegions(FALSE);
	ZBiasEntry->SetSelection(-1,-1);
}

void WxFractureToolOptions::OnPlaneBiasLoseFocus(wxCommandEvent& In)
{
	RegenRegions(FALSE);
}

/** Generate regions to let you preview before carving mesh */
void WxFractureToolOptions::OnGeneratePoints(wxCommandEvent& In)
{
	check(Editor);

	INT NumChunks = NumChunkSlider->GetValue();
	Editor->GenerateRandomPoints(NumChunks);
	Editor->ClusterPoints();
	RegenRegions(TRUE);
}

/** Add some random noise to cluster centers */
void WxFractureToolOptions::OnRandomize(wxCommandEvent& In)
{
	check(Editor);
	FVector RandScale(0.f,0.f,0.f);
	RandScale.X = appAtof( XRandScaleEntry->GetValue() );
	RandScale.Y = appAtof( YRandScaleEntry->GetValue() );
	RandScale.Z = appAtof( ZRandScaleEntry->GetValue() );

	Editor->RandomizePoints(RandScale);
	RegenRegions(FALSE);
}

void WxFractureToolOptions::OnMoveToFaces(wxCommandEvent& In)
{
	check(Editor);
	FVector MoveAmount(0.f,0.f,0.f);
	MoveAmount.X = appAtof( XRandScaleEntry->GetValue() );
	MoveAmount.Y = appAtof( YRandScaleEntry->GetValue() );
	MoveAmount.Z = appAtof( ZRandScaleEntry->GetValue() );

	Editor->MovePointsTowardsNearestFace(MoveAmount);
	RegenRegions(FALSE);
}

/** */
void WxFractureToolOptions::OnCluster(wxCommandEvent& In )
{
	check(Editor);
	Editor->ClusterPoints();
	RegenRegions(FALSE);
}

/** Recompute regions based on set of FracturePoints and PlaneBias entries. */
void WxFractureToolOptions::RegenRegions(UBOOL bResetInfos)
{
	check(Editor);
	Editor->ComputeRegions(GetPlaneBiasFromDialog(), bResetInfos);
}

/** Called when the Apply button is pressed */
void WxFractureToolOptions::OnSlice( wxCommandEvent& In )
{
	check(Editor);
	if(Editor->StaticMesh)
	{
		Editor->SliceMesh();

		UFracturedStaticMesh* FracMesh = Cast<UFracturedStaticMesh>(Editor->StaticMesh);
		if(FracMesh)
		{
			UpdateViewChunkSlider(FracMesh->GetNumFragments());

			// Update 'add core' button enabledness
			UpdateCoreButtonStates();

			// Save off plane bias
			FracMesh->PlaneBias = GetPlaneBiasFromDialog();
		}
	}
}

/** Called when show cuts box changed. */
void WxFractureToolOptions::OnShowCutsChange( wxCommandEvent& In )
{
	check(Editor);
	bShowCuts = ShowCutsBox->GetValue();
	Editor->ViewportClient->Viewport->Invalidate();
}

void WxFractureToolOptions::OnShowCutsSolidChange( wxCommandEvent& In )
{
	check(Editor);
	bShowCutsSolid = ShowCutsSolidBox->GetValue();
	Editor->ViewportClient->Viewport->Invalidate();
}

void WxFractureToolOptions::OnShowOnlyVisibleCuts( wxCommandEvent& In )
{
	check(Editor);
	bShowOnlyVisibleCuts = ShowVisibleCutsBox->GetValue();
	Editor->ViewportClient->Viewport->Invalidate();
}

void WxFractureToolOptions::OnColorModeChange( wxCommandEvent& In )
{
	check(Editor);

	INT NewMode = ColorModeCombo->GetSelection();

	if(NewMode == 0)
	{
		ColorMode = ESCM_Random;
	}
	else if(NewMode == 1)
	{
		ColorMode = ESCM_FixedChunks;
	}
	else if(NewMode == 2)
	{
		ColorMode = ESCM_DestroyableChunks;
	}
	else if(NewMode == 3)
	{
		ColorMode = ESCM_NoPhysChunks;
	}

	Editor->ViewportClient->Viewport->Invalidate();
}

void WxFractureToolOptions::OnShowCoreChange( wxCommandEvent& In )
{
	check(Editor);
	bShowCore = ShowCoreBox->GetValue();
	Editor->bViewChunkChanged = TRUE;
	Editor->ViewportClient->Viewport->Invalidate();
}


/** Called when Close button is pressed. */
void WxFractureToolOptions::OnPressClose( wxCommandEvent& In )
{
	Close();
}

/** Called when window is closed. */
void WxFractureToolOptions::OnClose( wxCloseEvent& In )
{
	check(Editor);
	Editor->FractureOptionsClosed();
	Destroy();
}

/** Called when chunk view slider is moved. */
void WxFractureToolOptions::OnViewChunkChange(wxScrollEvent& In)
{
	INT ChunkIndex = ViewChunkSlider->GetValue();

	Editor->ChangeChunkView(ChunkIndex, Editor->CurrentChunkViewMode);
}

/** Called when chunk view mode combo is changed. */
void WxFractureToolOptions::OnViewChunkModeChange(wxCommandEvent& In )
{
	INT ViewMode = ChunkViewModeCombo->GetSelection();

	if(ViewMode == 0)
	{
		Editor->ChangeChunkView(Editor->CurrentViewChunk, ECVM_ViewOnly);
	}
	else if(ViewMode == 1)
	{
		Editor->ChangeChunkView(Editor->CurrentViewChunk, ECVM_ViewAllBut);
	}
	else if(ViewMode == 2)
	{
		Editor->ChangeChunkView(Editor->CurrentViewChunk, ECVM_ViewUpTo);
	}
}

/** Called when a chunk option is changed. */
void WxFractureToolOptions::OnChunkOptChange( wxCommandEvent& In )
{
	if(Editor->SelectedChunks.Num() > 0)
	{
		for(INT SelIndex=0; SelIndex<Editor->SelectedChunks.Num(); SelIndex++)
		{
			INT SelectedChunk = Editor->SelectedChunks(SelIndex);

			// Copy data into 'info' struct
			check(SelectedChunk < Editor->FractureInfos.Num());

			wxCheckBoxState DestroyableState = DestroyableBox->Get3StateValue();
			if(DestroyableState != wxCHK_UNDETERMINED)
			{
				Editor->FractureInfos(SelectedChunk).bCanBeDestroyed = (DestroyableState == wxCHK_CHECKED) ? TRUE : FALSE;
			}

			wxCheckBoxState RootChunkState = RootChunkBox->Get3StateValue();
			if(RootChunkState != wxCHK_UNDETERMINED)
			{
				Editor->FractureInfos(SelectedChunk).bRootFragment = (RootChunkState == wxCHK_CHECKED) ? TRUE : FALSE;
			}

			wxCheckBoxState NoPhysChunkState = NoPhysBox->Get3StateValue();
			if(NoPhysChunkState != wxCHK_UNDETERMINED)
			{
				Editor->FractureInfos(SelectedChunk).bNeverSpawnPhysics = (NoPhysChunkState == wxCHK_CHECKED) ? TRUE : FALSE;
			}

			// Propagate to actual mesh if possible (if mapping is still present)
			UFracturedStaticMesh* FracMesh = Cast<UFracturedStaticMesh>(Editor->StaticMesh);
			if(FracMesh && Editor->FinalToToolMap.Num() != 0)
			{
				INT FinalChunkIndex = Editor->FinalToToolMap.FindItemIndex(SelectedChunk);
				if(FinalChunkIndex != INDEX_NONE)
				{
					FracMesh->SetFragmentDestroyable(FinalChunkIndex, Editor->FractureInfos(SelectedChunk).bCanBeDestroyed);
					FracMesh->SetIsRootFragment(FinalChunkIndex, Editor->FractureInfos(SelectedChunk).bRootFragment);
					FracMesh->SetIsNoPhysFragment(FinalChunkIndex, Editor->FractureInfos(SelectedChunk).bNeverSpawnPhysics);
				}
			}
		}
	}
}

void WxFractureToolOptions::OnGrowSelection( wxCommandEvent& In )
{
	check(Editor);
	Editor->GrowChunkSelection();
}

void WxFractureToolOptions::OnShrinkSelection( wxCommandEvent& In )
{
	check(Editor);
	Editor->ShrinkChunkSelection();
}

void WxFractureToolOptions::OnSelectTop( wxCommandEvent& In )
{
	check(Editor);
	Editor->SelectTopChunks();
}

void WxFractureToolOptions::OnSelectBottom( wxCommandEvent& In )
{
	check(Editor);
	Editor->SelectBottomChunks();
}

void WxFractureToolOptions::OnInvertSelection(wxCommandEvent& In)
{
	check(Editor);
	Editor->InvertSelection();
}

void WxFractureToolOptions::OnAddCore( wxCommandEvent& In)
{
	check(Editor);
	if(!Editor->bAdjustingCore)
	{
		UFracturedStaticMesh* FracMesh = Cast<UFracturedStaticMesh>(Editor->StaticMesh);
		if(FracMesh->SourceCoreMesh)
		{
			Editor->BeginAdjustCore();
		}
		else
		{
			Editor->BeginAddCore(FALSE, NULL);
		}
	}
}

void WxFractureToolOptions::OnRemoveCore( wxCommandEvent& In )
{
	check(Editor);
	Editor->RemoveCore();
}

void WxFractureToolOptions::OnAcceptCore( wxCommandEvent& In )
{
	check(Editor);
	Editor->AcceptAddCore();
}

void WxFractureToolOptions::OnCancelCore( wxCommandEvent& In )
{
	check(Editor);
	Editor->CancelAddCore();
}

/** Util to change range of chunk view slider. */
void  WxFractureToolOptions::UpdateViewChunkSlider(INT NumChunks)
{
	ViewChunkSlider->SetRange(0, NumChunks-1);
	ViewChunkSlider->SetValue( ::Clamp(Editor->CurrentViewChunk, 0, NumChunks-1) );
}

void WxStaticMeshEditor::OnMergeStaticMesh(wxCommandEvent& In)
{
	//MergeStaticMeshOpt();
}

/** Clear core mesh ref and re-slice to remove geom from this mesh. */
void WxStaticMeshEditor::RemoveCore()
{
	UFracturedStaticMesh* FracMesh = Cast<UFracturedStaticMesh>(StaticMesh);
	if(FracMesh && FracMesh->SourceCoreMesh)
	{
		FracMesh->SourceCoreMesh = NULL;
		SliceMesh();

		if(FractureOptions)
		{
			FractureOptions->UpdateCoreButtonStates();

			// Save off plane bias
			FracMesh->PlaneBias = FractureOptions->GetPlaneBiasFromDialog();
		}
	}
}

void WxStaticMeshEditor::BeginAddCore(UBOOL bUseExistingCoreTransform, UStaticMesh* CoreMesh)
{
	UStaticMesh* SelectedStaticMesh = NULL;

	if(!CoreMesh)
	{
		WxGenericBrowser* GenericBrowser = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT("GenericBrowser") );
		check(GenericBrowser);
		USelection* CurrentSelection = GenericBrowser->GetSelection();
		if (CurrentSelection)
		{
			SelectedStaticMesh = CurrentSelection->GetTop<UStaticMesh>();
		}

		if (!SelectedStaticMesh 
			|| SelectedStaticMesh->IsA(UFracturedStaticMesh::StaticClass())
			|| SelectedStaticMesh == StaticMesh)
		{
			appMsgf( AMT_OK, *LocalizeUnrealEd("SMMergeNoMeshSelected") );
			return;
		}

		// Don't allow adding a core if there is already one present.
		UFracturedStaticMesh* FracMesh = Cast<UFracturedStaticMesh>(StaticMesh);
		if(FracMesh)
		{
			if(FracMesh->SourceCoreMesh)
			{
				appMsgf( AMT_OK, *LocalizeUnrealEd("SMMergeFracMeshAlreadyHasCore") );
				return;
			}

			if(!SelectedStaticMesh->BodySetup)
			{
				appMsgf( AMT_OK, *LocalizeUnrealEd("SMMergeFracMeshCoreNoCollision") );
				return;
			}
		}
	}
	else
	{
		SelectedStaticMesh = CoreMesh;
	}
	
	bAdjustingCore = TRUE;
	PendingCoreMesh = SelectedStaticMesh;

	if(bUseExistingCoreTransform)
	{
		UFracturedStaticMesh* FracMesh = Cast<UFracturedStaticMesh>(StaticMesh);
		check(FracMesh);
		CurrentCoreOffset = FracMesh->CoreMeshOffset;
		CurrentCoreRotation = FracMesh->CoreMeshRotation;
		CurrentCoreScale = FracMesh->CoreMeshScale;
		CurrentCoreScale3D = FracMesh->CoreMeshScale3D;
	}
	else
	{
		CurrentCoreOffset = FVector(0,0,0);
		CurrentCoreRotation = FRotator(0,0,0);
		CurrentCoreScale = 1.f;
		CurrentCoreScale3D = FVector(1,1,1);
	}

	CoreEditMode = WMM_Translate;
	ViewportClient->SetCorePreview(PendingCoreMesh);
	ViewportClient->UpdateCorePreview();

	// Change material on outer mesh
	INT NumMaterials = ViewportClient->StaticMeshComponent->StaticMesh->LODInfo(0).Elements.Num();
	for(INT i=0; i<NumMaterials; i++)
	{
		ViewportClient->StaticMeshComponent->SetMaterial(i, ViewportClient->FracMeshMaterial);
	}


	ViewportClient->Viewport->Invalidate();
	
	if(FractureOptions)
	{
		FractureOptions->UpdateCoreButtonStates();
	}
}

/** Basically does a RemoveCore, then BeginAddCore using the current mesh and offset. */
void WxStaticMeshEditor::BeginAdjustCore()
{
	UFracturedStaticMesh* FracMesh = Cast<UFracturedStaticMesh>(StaticMesh);
	check(FracMesh);
	check(FracMesh->SourceCoreMesh);
	UStaticMesh* CoreMesh = FracMesh->SourceCoreMesh;

	RemoveCore();
	BeginAddCore(TRUE, CoreMesh);
}

/** Use current core settings to actually add core to the existing mesh. */
void WxStaticMeshEditor::AcceptAddCore()
{
	if(!bAdjustingCore || !PendingCoreMesh)
	{
		return;
	}

	//UBOOL bShouldMerge = appMsgf(AMT_YesNo, *FString::Printf(LocalizeSecure(LocalizeUnrealEd("SMMergeConfirmMerge"), *PendingCoreMesh->GetPathName())));
	//if (bShouldMerge)
	{
		DoMergeStaticMesh(PendingCoreMesh, CurrentCoreOffset, CurrentCoreRotation, CurrentCoreScale, CurrentCoreScale3D);
	}

	CancelAddCore();
}

/** Abort the core-adjusting state. */
void WxStaticMeshEditor::CancelAddCore()
{
	bAdjustingCore = FALSE;
	PendingCoreMesh = NULL;
	ViewportClient->SetCorePreview(NULL);

	// Restore original material
	INT NumMaterials = ViewportClient->StaticMeshComponent->Materials.Num();
	for(INT i=0; i<NumMaterials; i++)
	{
		ViewportClient->StaticMeshComponent->SetMaterial(i, NULL);
	}


	ViewportClient->Viewport->Invalidate();

	if(FractureOptions)
	{
		FractureOptions->UpdateCoreButtonStates();
	}
}

void WxStaticMeshEditor::AddChunkToSelection(INT ChunkIndex)
{
	SelectedChunks.AddUniqueItem(ChunkIndex);
	UpdateChunkUI();
}

void WxStaticMeshEditor::RemoveChunkFromSelection(INT ChunkIndex)
{
	SelectedChunks.RemoveItem(ChunkIndex);
	UpdateChunkUI();
}

void WxStaticMeshEditor::ClearChunkSelection()
{
	SelectedChunks.Empty();
	UpdateChunkUI();
}

void WxStaticMeshEditor::GrowChunkSelection()
{
	// Do nothing if tool isn't open or nothing selected
	if(!FractureOptions || SelectedChunks.Num() == 0)
	{
		return;
	}

	// Init new selection set with current selection set
	TArray<INT> NewSelectedChunks = SelectedChunks;
	// Iterate over each chunk..
	for(INT SelIndex=0; SelIndex<SelectedChunks.Num(); SelIndex++)
	{
		INT SelectedChunk = SelectedChunks(SelIndex);
		FVoronoiRegion& SelectedV = FractureChunks(SelectedChunk);
		// ..adding all its neighbours
		for(INT NIndex=0; NIndex<SelectedV.Neighbours.Num(); NIndex++)
		{
			if(SelectedV.Neighbours(NIndex) != INDEX_NONE)
			{
				NewSelectedChunks.AddUniqueItem( SelectedV.Neighbours(NIndex) );
			}
		}
	}
	// Update selection
	SelectedChunks = NewSelectedChunks;
	UpdateChunkUI();
	ViewportClient->Viewport->Invalidate();
}

void WxStaticMeshEditor::ShrinkChunkSelection()
{
	// Do nothing if tool isn't open or nothing selected
	if(!FractureOptions || SelectedChunks.Num() == 0)
	{
		return;
	}

	// Init new selection set with current selection set
	TArray<INT> NewSelectedChunks = SelectedChunks;
	// Iterate over each chunk..
	for(INT SelIndex=0; SelIndex<SelectedChunks.Num(); SelIndex++)
	{
		INT SelectedChunk = SelectedChunks(SelIndex);
		FVoronoiRegion& SelectedV = FractureChunks(SelectedChunk);
		// .. see if it has any neighbours which are not selected
		UBOOL bUnselectedNeighbour = FALSE;
		for(INT NIndex=0; NIndex<SelectedV.Neighbours.Num(); NIndex++)
		{
			if(SelectedV.Neighbours(NIndex) != INDEX_NONE)
			{
				if( !SelectedChunks.ContainsItem(SelectedV.Neighbours(NIndex)) )
				{
					// Found an unselected neighbour - remove from new selection set, and move on.
					NewSelectedChunks.RemoveItem(SelectedChunk);
					break;
				}
			}
		}
	}
	// Update selection
	SelectedChunks = NewSelectedChunks;
	UpdateChunkUI();
	ViewportClient->Viewport->Invalidate();
}

/** Select all chunks at the top of the mesh. */
void WxStaticMeshEditor::SelectTopChunks()
{
	// Do nothing if tool isn't open or nothing selected
	if(FractureChunks.Num() == 0)
	{
		return;
	}

	// First determine max box
	FLOAT FracMaxZ = -BIG_NUMBER;
	for(INT i=0; i<FractureChunks.Num(); i++)
	{
		FVoronoiRegion& V = FractureChunks(i);
		FracMaxZ = Max(V.ConvexElem.ElemBox.Max.Z, FracMaxZ);
	}

	// Now find all that touch the top
	for(INT i=0; i<FractureChunks.Num(); i++)
	{
		FVoronoiRegion& V = FractureChunks(i);
		if(Abs(V.ConvexElem.ElemBox.Max.Z - FracMaxZ) < 0.1f)
		{
			SelectedChunks.AddUniqueItem(i);
		}
	}

	// Update selection
	UpdateChunkUI();
	ViewportClient->Viewport->Invalidate();
}

/** Select all chunks at the bottom of the mesh. */
void WxStaticMeshEditor::SelectBottomChunks()
{
	// Do nothing if tool isn't open or nothing selected
	if(!FractureOptions || FractureChunks.Num() == 0)
	{
		return;
	}

	// First determine min box
	FLOAT FracMinZ = BIG_NUMBER;
	for(INT i=0; i<FractureChunks.Num(); i++)
	{
		FVoronoiRegion& V = FractureChunks(i);
		FracMinZ = Min(V.ConvexElem.ElemBox.Min.Z, FracMinZ);
	}

	// Now find all that touch the top
	for(INT i=0; i<FractureChunks.Num(); i++)
	{
		FVoronoiRegion& V = FractureChunks(i);
		if(Abs(V.ConvexElem.ElemBox.Min.Z - FracMinZ) < 0.1f)
		{
			SelectedChunks.AddUniqueItem(i);
		}
	}

	// Update selection
	UpdateChunkUI();
	ViewportClient->Viewport->Invalidate();
}

/** Invert current selection set */
void WxStaticMeshEditor::InvertSelection()
{
	// Build set of all chunks that are not currently selected.
	TArray<INT> NewSelection;

	for(INT i=0; i<FractureChunks.Num(); i++)
	{
		if(!SelectedChunks.ContainsItem(i))
		{
			NewSelection.AddItem(i);
		}
	}

	SelectedChunks = NewSelection;

	// Update selection
	UpdateChunkUI();
	ViewportClient->Viewport->Invalidate();
}

/** Util for finding color of a slice chunks, based on color mode and selection state. */
FColor WxStaticMeshEditor::GetChunkColor(INT ChunkIndex)
{
	FColor UseColor;

	// If no fracture options, or out of range - use random
	if(!FractureOptions || ChunkIndex >= FractureInfos.Num())
	{
		return DebugUtilColor[ChunkIndex%NUM_DEBUG_UTIL_COLORS];
	}
	// Selected case
	else if(SelectedChunks.ContainsItem(ChunkIndex))
	{
		return StaticMeshEditor_SelectedChunkColor;
	}
	// Want random colours
	else if(FractureOptions->ColorMode == ESCM_Random)
	{
		return DebugUtilColor[ChunkIndex%NUM_DEBUG_UTIL_COLORS];
	}
	// Colorise fixed chunks
	else if(FractureOptions->ColorMode == ESCM_FixedChunks)
	{
		return FractureInfos(ChunkIndex).bRootFragment ? StaticMeshEditor_FilterTrueColor : StaticMeshEditor_FilterFalseColor;
	}
	// Colorise destroyable chunks
	else if(FractureOptions->ColorMode == ESCM_DestroyableChunks)
	{
		return FractureInfos(ChunkIndex).bCanBeDestroyed ? StaticMeshEditor_FilterTrueColor : StaticMeshEditor_FilterFalseColor;
	}
	// Colorise no-physics chunks
	else if(FractureOptions->ColorMode == ESCM_NoPhysChunks)
	{
		return FractureInfos(ChunkIndex).bNeverSpawnPhysics ? StaticMeshEditor_FilterTrueColor : StaticMeshEditor_FilterFalseColor;
	}
	// Fallback case
	else
	{
		return StaticMeshEditor_FilterFalseColor;
	}
}

/**
 * Fill the array with strings representing the available options
 * when altering LODs for a static mesh.
 *
 * Note: We should not allow the user to skip an LOD (e.g. add LOD2 before LOD1 exists).
 *
 * @param	Strings	The TArray of strings to be populated; this array will be emptied.
 */
void WxStaticMeshEditor::FillOutLODAlterationStrings( TArray<FString>& Strings ) const
{
	Strings.Empty();
	
	// We are adding a new LOD, so the new max is the current highest LOD level + 1,
	// which happens to be LODModels.Num() due to arrays being 0-indexed.
	// However, we'll never have anything higher than MexAllowedLOD (by design)
	INT CurrentMaxLOD = StaticMesh->LODModels.Num() - 1;
	INT NewMaxLOD = Min(CurrentMaxLOD + 1, MaxAllowedLOD);

	// Now fill the array with user options
	Strings.AddZeroed( NewMaxLOD );
	for( INT i=0; i<NewMaxLOD; i++ )
	{
		if ( i+1 > CurrentMaxLOD )
		{
			Strings(i) = LocalizeUnrealEd("StaticMeshEditor_AddNewLOD") + FString::Printf( TEXT(" %d"), i+1 );
		}
		else
		{
			Strings(i) = LocalizeUnrealEd("StaticMeshEditor_ReplaceLOD") + FString::Printf( TEXT(" %d"), i+1 );
		}
	}
}

/** Change the selected chunk. */
void WxStaticMeshEditor::UpdateChunkUI()
{
	if(FractureOptions)
	{
		// When selecting no chunk, disable check boxes
		if(SelectedChunks.Num() == 0)
		{
			FractureOptions->DestroyableBox->Enable(FALSE);
			FractureOptions->RootChunkBox->Enable(FALSE);
			FractureOptions->NoPhysBox->Enable(FALSE);
		}
		// When selecting chunk, update info.
		else
		{
			// Look through selection to find state of check boxes
			INT SelectedChunk = SelectedChunks(0);
			wxCheckBoxState DestroyableState = FractureInfos(SelectedChunk).bCanBeDestroyed ? wxCHK_CHECKED : wxCHK_UNCHECKED;
			wxCheckBoxState RootChunkState = FractureInfos(SelectedChunk).bRootFragment ? wxCHK_CHECKED : wxCHK_UNCHECKED;
			wxCheckBoxState NoPhysChunkState = FractureInfos(SelectedChunk).bNeverSpawnPhysics ? wxCHK_CHECKED : wxCHK_UNCHECKED;

			for(INT SelIndex=1; SelIndex<SelectedChunks.Num(); SelIndex++)
			{
				SelectedChunk = SelectedChunks(SelIndex);
				wxCheckBoxState ThisDestroyableState = FractureInfos(SelectedChunk).bCanBeDestroyed ? wxCHK_CHECKED : wxCHK_UNCHECKED;
				if(DestroyableState != ThisDestroyableState)
				{
					DestroyableState = wxCHK_UNDETERMINED;
				}

				wxCheckBoxState ThisRootChunkState = FractureInfos(SelectedChunk).bRootFragment ? wxCHK_CHECKED : wxCHK_UNCHECKED;
				if(RootChunkState != ThisRootChunkState)
				{
					RootChunkState = wxCHK_UNDETERMINED;
				}

				wxCheckBoxState ThisNoPhysChunkState = FractureInfos(SelectedChunk).bNeverSpawnPhysics ? wxCHK_CHECKED : wxCHK_UNCHECKED;
				if(NoPhysChunkState != ThisNoPhysChunkState)
				{
					NoPhysChunkState = wxCHK_UNDETERMINED;
				}
			}

			// Enable and set state on check boxes
			FractureOptions->DestroyableBox->Enable(TRUE);
			FractureOptions->DestroyableBox->Set3StateValue(DestroyableState);

			FractureOptions->RootChunkBox->Enable(TRUE);
			FractureOptions->RootChunkBox->Set3StateValue(RootChunkState);

			FractureOptions->NoPhysBox->Enable(TRUE);
			FractureOptions->NoPhysBox->Set3StateValue(NoPhysChunkState);
		}
	}
}

/** Do the actual merging work. */
void WxStaticMeshEditor::DoMergeStaticMesh(UStaticMesh* OtherMesh, const FVector& InOffset, const FRotator& InRotation, FLOAT InScale, const FVector& InScale3D)
{
	MergeStaticMesh(StaticMesh, OtherMesh, InOffset, InRotation, InScale, InScale3D);

	UpdateToolbars();

	// Update the generic browser since the poly count will have changed
	WxGenericBrowser* GenericBrowser = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT("GenericBrowser") );
	check(GenericBrowser);
	GenericBrowser->Update();

	//Update the fragment view slider if the fracture tools dialog is open, since the number of fragments may have changed
	UFracturedStaticMesh* FracturedMesh = Cast<UFracturedStaticMesh>(StaticMesh);
	if (FracturedMesh && FractureOptions)
	{
		FractureOptions->UpdateViewChunkSlider(FracturedMesh->GetNumFragments());

		// Disable 'add core' button
		FractureOptions->UpdateCoreButtonStates();
	}

	ViewportClient->Viewport->Invalidate();
}

/** Util for updating state of core control buttons. */
void WxFractureToolOptions::UpdateCoreButtonStates()
{
	UFracturedStaticMesh* FracMesh = Cast<UFracturedStaticMesh>(Editor->StaticMesh);
	if(FracMesh)
	{
		if(Editor->bAdjustingCore)
		{
			AddCoreButton->Enable(FALSE);
			RemoveCoreButton->Enable(FALSE);
			AcceptCoreButton->Enable(TRUE);
			CancelCoreButton->Enable(TRUE);
		}
		else
		{
			if(FracMesh->SourceCoreMesh)
			{
				AddCoreButton->SetLabel(*LocalizeUnrealEd("AdjustCore"));
			}
			else
			{
				AddCoreButton->SetLabel(*LocalizeUnrealEd("AddCore"));
			}

			AddCoreButton->Enable(TRUE);
			RemoveCoreButton->Enable(FracMesh->SourceCoreMesh != NULL);
			AcceptCoreButton->Enable(FALSE);
			CancelCoreButton->Enable(FALSE);
		}
	}
	else
	{
		AddCoreButton->Enable(FALSE);
		RemoveCoreButton->Enable(FALSE);
		AcceptCoreButton->Enable(FALSE);
		CancelCoreButton->Enable(FALSE);
	}
}

/** Read info from Plane Bias dialog */
FVector WxFractureToolOptions::GetPlaneBiasFromDialog()
{
	FVector PlaneBias(1.f,1.f,1.f);
	PlaneBias.X = appAtof( XBiasEntry->GetValue() );
	PlaneBias.Y = appAtof( YBiasEntry->GetValue() );
	PlaneBias.Z = appAtof( ZBiasEntry->GetValue() );
	return PlaneBias;
}

