/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "LevelViewportToolBar.h"
#include "UnTerrain.h"

BEGIN_EVENT_TABLE( WxLevelViewportToolBar, wxToolBar )
	EVT_TOOL( IDM_REALTIME, WxLevelViewportToolBar::OnRealTime )
	EVT_TOOL( IDM_MOVEUNLIT, WxLevelViewportToolBar::OnMoveUnlit )
	EVT_TOOL( IDM_LevelStreamingVolumePreVis, WxLevelViewportToolBar::OnLevelStreamingVolumePreVis )
	EVT_TOOL( IDM_PostProcessVolumePreVis, WxLevelViewportToolBar::OnPostProcessVolumePreVis )
	EVT_TOOL( IDM_SQUINTBLURMODE, WxLevelViewportToolBar::OnSquintModeChange )
	EVT_TOOL( IDM_VIEWPORTLOCKED, WxLevelViewportToolBar::OnViewportLocked )
	EVT_TOOL( IDM_BRUSHWIREFRAME, WxLevelViewportToolBar::OnBrushWireframe )
	EVT_TOOL( IDM_WIREFRAME, WxLevelViewportToolBar::OnWireframe )
	EVT_TOOL( IDM_UNLIT, WxLevelViewportToolBar::OnUnlit )
	EVT_TOOL( IDM_LIT, WxLevelViewportToolBar::OnLit )
	EVT_TOOL( IDM_LIGHTINGONLY, WxLevelViewportToolBar::OnLightingOnly )
	EVT_TOOL( IDM_LIGHTCOMPLEXITY, WxLevelViewportToolBar::OnLightComplexity )
	EVT_TOOL( IDM_TEXTUREDENSITY, WxLevelViewportToolBar::OnTextureDensity )
	EVT_TOOL( IDM_SHADERCOMPLEXITY, WxLevelViewportToolBar::OnShaderComplexity )
	EVT_TOOL( IDM_PERSPECTIVE, WxLevelViewportToolBar::OnPerspective )
	EVT_TOOL( IDM_TOP, WxLevelViewportToolBar::OnTop )
	EVT_TOOL( IDM_FRONT, WxLevelViewportToolBar::OnFront )
	EVT_TOOL( IDM_SIDE, WxLevelViewportToolBar::OnSide )
	EVT_TOOL( IDM_MAXIMIZE_VIEWPORT, WxLevelViewportToolBar::OnMaximizeViewport )
	EVT_UPDATE_UI( IDM_MAXIMIZE_VIEWPORT, WxLevelViewportToolBar::UpdateMaximizeViewportUI )
	EVT_TOOL( IDM_TearOffNewFloatingViewport, WxLevelViewportToolBar::OnTearOffNewFloatingViewport )
	EVT_TOOL( IDM_AllowMatineePreview, WxLevelViewportToolBar::OnToggleAllowMatineePreview )
	EVT_TOOL( IDM_LOCK_SELECTED_TO_CAMERA, WxLevelViewportToolBar::OnLockSelectedToCamera )
	EVT_TOOL( IDM_LevelViewportToolbar_PlayInViewport, WxLevelViewportToolBar::OnPlayInViewport )
	EVT_MENU( IDM_MakeOcclusionParent, WxLevelViewportToolBar::OnMakeParentViewport )
	EVT_MENU( IDM_SHOW_FLAGS, WxLevelViewportToolBar::OnShowFlags )
	EVT_MENU( IDM_VIEWPORT_SHOW_DEFAULTS, WxLevelViewportToolBar::OnShowDefaults )
	EVT_MENU( IDM_VIEWPORT_SHOW_STATICMESHES, WxLevelViewportToolBar::OnShowStaticMeshes )
	EVT_MENU( IDM_VIEWPORT_SHOW_SkeletalMeshes, WxLevelViewportToolBar::OnShowSkeletalMeshes )
	EVT_MENU( IDM_VIEWPORT_SHOW_SpeedTrees, WxLevelViewportToolBar::OnShowSpeedTrees )
	EVT_MENU( IDM_VIEWPORT_SHOW_TERRAIN, WxLevelViewportToolBar::OnShowTerrain )
	EVT_MENU( IDM_VIEWPORT_SHOW_FOLIAGE, WxLevelViewportToolBar::OnShowFoliage )
	EVT_MENU( IDM_VIEWPORT_SHOW_BSP, WxLevelViewportToolBar::OnShowBSP )
	EVT_MENU( IDM_VIEWPORT_SHOW_BSPSPLIT, WxLevelViewportToolBar::OnShowBSPSplit )
	EVT_MENU( IDM_VIEWPORT_SHOW_COLLISION, WxLevelViewportToolBar::OnShowCollision )
	EVT_MENU( IDM_VIEWPORT_SHOW_GRID, WxLevelViewportToolBar::OnShowGrid )
	EVT_MENU( IDM_VIEWPORT_SHOW_BOUNDS, WxLevelViewportToolBar::OnShowBounds )
	EVT_MENU( IDM_VIEWPORT_SHOW_PATHS, WxLevelViewportToolBar::OnShowPaths )
	EVT_MENU( IDM_VIEWPORT_SHOW_NAVNODES, WxLevelViewportToolBar::OnShowNavigationNodes )
	EVT_MENU( IDM_VIEWPORT_SHOW_MESHEDGES, WxLevelViewportToolBar::OnShowMeshEdges )
	EVT_MENU( IDM_VIEWPORT_SHOW_LARGEVERTICES, WxLevelViewportToolBar::OnShowLargeVertices )
	EVT_MENU( IDM_VIEWPORT_SHOW_PORTALS, WxLevelViewportToolBar::OnShowPortals )
	EVT_MENU( IDM_VIEWPORT_SHOW_HITPROXIES, WxLevelViewportToolBar::OnShowHitProxies )
	EVT_MENU( IDM_VIEWPORT_SHOW_SHADOWFRUSTUMS, WxLevelViewportToolBar::OnShowShadowFrustums )
	EVT_MENU( IDM_VIEWPORT_SHOW_KISMETREFS, WxLevelViewportToolBar::OnShowKismetRefs )
	EVT_MENU( IDM_VIEWPORT_SHOW_VOLUMES, WxLevelViewportToolBar::OnShowVolumes )
	EVT_MENU( IDM_VIEWPORT_SHOW_CAMFRUSTUMS, WxLevelViewportToolBar::OnShowCamFrustums )
	EVT_MENU( IDM_VIEWPORT_SHOW_FOG, WxLevelViewportToolBar::OnShowFog )
	EVT_MENU( IDM_VIEWPORT_SHOW_SELECTION, WxLevelViewportToolBar::OnShowSelection )
	EVT_MENU( IDM_VIEWPORT_SHOW_PARTICLES, WxLevelViewportToolBar::OnShowParticles )
	EVT_MENU( IDM_VIEWPORT_SHOW_LIGHTINFLUENCES, WxLevelViewportToolBar::OnShowLightInfluences )
	EVT_MENU( IDM_VIEWPORT_SHOW_BUILDERBRUSH, WxLevelViewportToolBar::OnShowBuilderBrush )
	EVT_MENU( IDM_VIEWPORT_SHOW_ACTORTAGS, WxLevelViewportToolBar::OnShowActorTags )
	EVT_MENU( IDM_VIEWPORT_SHOW_MISSINGCOLLISION, WxLevelViewportToolBar::OnShowMissingCollision )
	EVT_MENU( IDM_VIEWPORT_SHOW_DECALS, WxLevelViewportToolBar::OnShowDecals )
	EVT_MENU( IDM_VIEWPORT_SHOW_DECALINFO, WxLevelViewportToolBar::OnShowDecalInfo )
	EVT_MENU( IDM_VIEWPORT_SHOW_LIGHTRADIUS, WxLevelViewportToolBar::OnShowLightRadius )
	EVT_MENU( IDM_VIEWPORT_SHOW_AUDIORADIUS, WxLevelViewportToolBar::OnShowAudioRadius )
	EVT_MENU( IDM_VIEWPORT_SHOW_TERRAIN_PATCHES, WxLevelViewportToolBar::OnShowTerrainPatches )
	EVT_MENU( IDM_VIEWPORT_SHOW_TERRAIN_COLLISION, WxLevelViewportToolBar::OnShowTerrainCollision )
	EVT_MENU( IDM_VIEWPORT_SHOW_POSTPROCESS, WxLevelViewportToolBar::OnShowPostProcess )
	EVT_MENU( IDM_VIEWPORT_SHOW_UNLITTRANSLUCENCY, WxLevelViewportToolBar::OnShowUnlitTranslucency )
	EVT_MENU( IDM_VIEWPORT_SHOW_SCENECAPTUREUPDATES, WxLevelViewportToolBar::OnShowSceneCaptureUpdates )
	EVT_MENU( IDM_VIEWPORT_SHOW_LEVELCOLORATION, WxLevelViewportToolBar::OnShowLevelColoration )
	EVT_MENU( IDM_VIEWPORT_SHOW_PROPERTYCOLORATION, WxLevelViewportToolBar::OnShowPropertyColoration )
	EVT_MENU( IDM_VIEWPORT_SHOW_SPRITES, WxLevelViewportToolBar::OnShowSprites )
	EVT_MENU( IDM_VIEWPORT_SHOW_CONSTRAINTS, WxLevelViewportToolBar::OnShowConstraints )
	EVT_MENU( IDM_VIEWPORT_SHOW_MODEWIDGETS, WxLevelViewportToolBar::OnShowModeWidgets )
	EVT_MENU( IDM_VIEWPORT_SHOW_COLLISION_ZEROEXTENT, WxLevelViewportToolBar::OnChangeCollisionMode )
	EVT_MENU( IDM_VIEWPORT_SHOW_COLLISION_NONZEROEXTENT, WxLevelViewportToolBar::OnChangeCollisionMode )
	EVT_MENU( IDM_VIEWPORT_SHOW_COLLISION_RIGIDBODY, WxLevelViewportToolBar::OnChangeCollisionMode )
	EVT_MENU( IDM_VIEWPORT_SHOW_COLLISION_NONE, WxLevelViewportToolBar::OnChangeCollisionMode )
	EVT_MENU( IDM_VIEWPORT_SHOW_DYNAMICSHADOWS, WxLevelViewportToolBar::OnShowDynamicShadows )
	EVT_MENU( IDM_VIEWPORT_SHOW_LENSFLARES, WxLevelViewportToolBar::OnShowLensFlares )
	EVT_MENU( IDM_VIEWPORT_SHOW_SENTINELSTATS, WxLevelViewportToolBar::OnShowSentinelStats )

END_EVENT_TABLE()

WxLevelViewportToolBar::WxLevelViewportToolBar( wxWindow* InParent, wxWindowID InID, FEditorLevelViewportClient* InViewportClient )
	: wxToolBar( InParent, InID, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_3DBUTTONS )
	,	ViewportClient( InViewportClient )
{
	// Bitmaps
	MakeViewOcclusionParentB.Load( TEXT("LVT_MakeViewOcclusionParent") );
	LockSelectedToCameraB.Load( TEXT("LVT_LockSelectedToCamera") );
	LockViewportB.Load( TEXT("LVT_LockViewport") );
	RealTimeB.Load( TEXT("Realtime") );
	StreamingPrevisB.Load( TEXT("LVT_StreamingPrevis") );
	PostProcessPrevisB.Load( TEXT("LVT_PPPrevis") );
	UnlitMovementB.Load( TEXT("UnlitMovement") );
	ShowFlagsB.Load( TEXT("Showflags") );
	SquintButtonB.Load( TEXT("Squint") );
	CamSlowB.Load( TEXT("CamSlow") );
	CamNormalB.Load( TEXT("CamNormal") );
	CamFastB.Load( TEXT("CamFast") );

	//SetToolBitmapSize( wxSize( 16, 16 ) );

	BrushWireframeB.Load( TEXT("LVT_BrushWire") );
	WireframeB.Load( TEXT("LVT_Wire") );
	UnlitB.Load( TEXT("LVT_Unlit") );
	LitB.Load( TEXT("LVT_Lit") );
	LightingOnlyB.Load( TEXT("LVT_LightingOnly") );
	LightComplexityB.Load( TEXT("LVT_LightingComplexity") );
	TextureDensityB.Load( TEXT("LVT_TextureDensity") );
	ShaderComplexityB.Load( TEXT("LVT_ShaderComplexity") );

	PerspectiveB.Load( TEXT("LVT_Perspective") );
	TopB.Load( TEXT("LVT_Top") );
	FrontB.Load( TEXT("LVT_Front") );
	SideB.Load( TEXT("LVT_Side") );
	MaximizeB.Load( TEXT("LVT_Maximize") );
	TearOffNewFloatingViewportB.Load( TEXT( "LVT_TearOffNewFloatingViewport" ) );
	AllowMatineePreviewB.Load( TEXT( "LVT_AllowMatineePreview" ) );

	// Play-In-Viewport button
	PlayInViewportB = new WxAlphaBitmap( TEXT( "PlayViewport" ), 0 );

	// Set up the ToolBar
	AddSeparator();
	AddTool( IDM_SHOW_FLAGS, TEXT(""), ShowFlagsB, *LocalizeUnrealEd("ToolTip_60") );
	AddSeparator();
	AddCheckTool( IDM_REALTIME, TEXT(""), RealTimeB, RealTimeB, *LocalizeUnrealEd("ToolTip_57") );
	AddCheckTool( IDM_AllowMatineePreview, TEXT(""), AllowMatineePreviewB, AllowMatineePreviewB, *LocalizeUnrealEd( "ViewportToolbar_AllowMatineePreview" ) );
	AddCheckTool( IDM_MOVEUNLIT, TEXT(""), UnlitMovementB, UnlitMovementB, *LocalizeUnrealEd("ToolTip_UnlitMovement") );
	AddSeparator();
	AddCheckTool( IDM_BRUSHWIREFRAME, TEXT(""), BrushWireframeB, BrushWireframeB, *LocalizeUnrealEd("BrushWireframe") );
	AddCheckTool( IDM_WIREFRAME, TEXT(""), WireframeB, WireframeB, *LocalizeUnrealEd("Wireframe") );
	AddCheckTool( IDM_UNLIT, TEXT(""), UnlitB, UnlitB, *LocalizeUnrealEd("Unlit") );
	AddCheckTool( IDM_LIT, TEXT(""), LitB, LitB, *LocalizeUnrealEd("Lit") );
	AddCheckTool( IDM_LIGHTINGONLY, TEXT(""), LightingOnlyB, LightingOnlyB, *LocalizeUnrealEd("LightingOnly") );
	AddCheckTool( IDM_LIGHTCOMPLEXITY, TEXT(""), LightComplexityB, LightComplexityB, *LocalizeUnrealEd("LightComplexity") );
	AddCheckTool( IDM_TEXTUREDENSITY, TEXT(""), TextureDensityB, TextureDensityB, *LocalizeUnrealEd("TextureDensity") );
	AddCheckTool( IDM_SHADERCOMPLEXITY, TEXT(""), ShaderComplexityB, ShaderComplexityB, *LocalizeUnrealEd("ShaderComplexity") );
	AddSeparator();
	AddCheckTool( IDM_PERSPECTIVE, TEXT(""), PerspectiveB, PerspectiveB, *LocalizeUnrealEd("Perspective") );
	AddCheckTool( IDM_TOP, TEXT(""), TopB, TopB, *LocalizeUnrealEd("Top") );
	AddCheckTool( IDM_FRONT, TEXT(""), FrontB, FrontB, *LocalizeUnrealEd("Front") );
 	AddCheckTool( IDM_SIDE, TEXT(""), SideB, SideB, *LocalizeUnrealEd("Side") );
	AddSeparator();
	AddCheckTool( IDM_VIEWPORTLOCKED, TEXT(""), LockViewportB, LockViewportB, *LocalizeUnrealEd("ToolTip_ViewportLocked") );
	AddCheckTool( IDM_LOCK_SELECTED_TO_CAMERA, TEXT(""), LockSelectedToCameraB, LockSelectedToCameraB, *LocalizeUnrealEd("ToolTip_159") );
	AddSeparator();
	AddCheckTool( IDM_MakeOcclusionParent, TEXT(""), MakeViewOcclusionParentB, MakeViewOcclusionParentB, *LocalizeUnrealEd("ToolTip_MakeViewOcclusionParent") );
	AddSeparator();
	AddCheckTool( IDM_LevelStreamingVolumePreVis, TEXT(""), StreamingPrevisB, StreamingPrevisB, *LocalizeUnrealEd("ToolTip_LevelStreamingVolumePrevis") );
	AddCheckTool( IDM_PostProcessVolumePreVis, TEXT(""), PostProcessPrevisB, PostProcessPrevisB, *LocalizeUnrealEd("ToolTip_PostProcessVolumePrevis") );
	AddCheckTool( IDM_SQUINTBLURMODE, TEXT(""), SquintButtonB, SquintButtonB, *LocalizeUnrealEd("TollTip_SquintMode") );
	AddSeparator();
	AddCheckTool( ID_CAMSPEED_SLOW, TEXT(""), CamSlowB, CamSlowB, *LocalizeUnrealEd("ToolTip_28") );
	AddCheckTool( ID_CAMSPEED_NORMAL, TEXT(""), CamNormalB, CamNormalB, *LocalizeUnrealEd("ToolTip_29") );
	AddCheckTool( ID_CAMSPEED_FAST, TEXT(""), CamFastB, CamFastB, *LocalizeUnrealEd("ToolTip_30") );
	AddSeparator();
	AddTool( IDM_LevelViewportToolbar_PlayInViewport, TEXT(""), *PlayInViewportB, *LocalizeUnrealEd( "LevelViewportToolbar_PlayInViewport" ) );


	if( !ViewportClient->IsFloatingViewport() )
	{
		AddSeparator();

		// Only add the 'tear off' button for non-floating viewports
		AddTool( IDM_TearOffNewFloatingViewport, TEXT( "" ), TearOffNewFloatingViewportB, *LocalizeUnrealEd( "ViewportToolbar_TearOffNewFloatingViewport" ) );

		// Don't show the 'maximize' button for floating viewport.  They'll have their own maximize button on the window frame
		AddCheckTool( IDM_MAXIMIZE_VIEWPORT, TEXT(""), MaximizeB, MaximizeB, *LocalizeUnrealEd("Maximize_Viewport") );
	}

	Realize();
	// UpdateUI must be called *after* Realize.
	UpdateUI();
}

void WxLevelViewportToolBar::UpdateUI()
{
	ToggleTool( IDM_REALTIME, ViewportClient->IsRealtime()==TRUE );
	ToggleTool( IDM_MOVEUNLIT, ViewportClient->bMoveUnlit==TRUE );

	INT ViewportTypeID = -1;
	switch( ViewportClient->ViewportType )
	{
		case LVT_Perspective:	ViewportTypeID = 0; break;
		case LVT_OrthoXY:		ViewportTypeID = 1; break;
		case LVT_OrthoYZ:		ViewportTypeID = 2; break;
		case LVT_OrthoXZ:		ViewportTypeID = 3; break;
		default:
			break;
	}
	SetViewportTypeUI( ViewportTypeID );

	INT ViewModeID = -1;
	EShowFlags ViewModeShowFlags = ViewportClient->ShowFlags & SHOW_ViewMode_Mask;
	switch( ViewModeShowFlags )
	{
		case SHOW_ViewMode_BrushWireframe:	ViewModeID = 0; break;
		case SHOW_ViewMode_Wireframe:		ViewModeID = 1; break;
		case SHOW_ViewMode_Unlit:			ViewModeID = 2; break;
		case SHOW_ViewMode_Lit:				ViewModeID = 3; break;
		case SHOW_ViewMode_LightingOnly:	ViewModeID = 4; break;
		case SHOW_ViewMode_LightComplexity:	ViewModeID = 5; break;
		case SHOW_ViewMode_TextureDensity:	ViewModeID = 6; break;
		case SHOW_ViewMode_ShaderComplexity:ViewModeID = 7; break;
		default:
			break;
	}
	SetViewModeUI( ViewModeID );
}

void WxLevelViewportToolBar::OnSquintModeChange( wxCommandEvent& In )
{
	ViewportClient->SetSquintMode( In.IsChecked() );
	ViewportClient->Viewport->Invalidate();
}

void WxLevelViewportToolBar::OnRealTime( wxCommandEvent& In )
{
	ViewportClient->SetRealtime( In.IsChecked() );
	ViewportClient->Invalidate( FALSE );
}

void WxLevelViewportToolBar::OnMoveUnlit( wxCommandEvent& In )
{
	ViewportClient->bMoveUnlit = In.IsChecked();
	ViewportClient->Invalidate( FALSE );
}


void WxLevelViewportToolBar::OnToggleAllowMatineePreview( wxCommandEvent& In )
{
	ViewportClient->SetAllowMatineePreview( In.IsChecked() );
	ViewportClient->Invalidate( FALSE );
}



/** Called from the window event handler to launch Play-In-Editor for this viewport */
void WxLevelViewportToolBar::OnPlayInViewport( wxCommandEvent& In )
{
	if ( GApp && GApp->EditorFrame && GApp->EditorFrame->ViewportConfigData )
	{
		FVector* StartLocation = NULL;
		FRotator* StartRotation = NULL;

		// If this is a perspective viewport, then we'll Play From Here
		if( ViewportClient->ViewportType == LVT_Perspective )
		{
			// Start PIE from the camera's location and orientation!
			StartLocation = &ViewportClient->ViewLocation;
			StartRotation = &ViewportClient->ViewRotation;
		}

		// Figure out which viewport index we are
		INT MyViewportIndex = -1;
		for( INT CurViewportIndex = 0; CurViewportIndex < GApp->EditorFrame->ViewportConfigData->GetViewportCount(); ++CurViewportIndex )
		{
			FVCD_Viewport& CurViewport = GApp->EditorFrame->ViewportConfigData->AccessViewport( CurViewportIndex );
			if( CurViewport.bEnabled && CurViewport.ViewportWindow->Viewport == ViewportClient->Viewport )
			{
				MyViewportIndex = CurViewportIndex;
				break;
			}
		}

		// Queue a PIE session for this viewport
		GUnrealEd->PlayMap( StartLocation, StartRotation, -1, MyViewportIndex );
	}
}



void WxLevelViewportToolBar::OnLevelStreamingVolumePreVis( wxCommandEvent& In )
{
	// Level streaming volume previews is only possible with perspective level viewports.
	check( ViewportClient->IsPerspective() );
	ViewportClient->bLevelStreamingVolumePrevis = In.IsChecked();
	ViewportClient->Invalidate();
}

void WxLevelViewportToolBar::OnPostProcessVolumePreVis( wxCommandEvent& In )
{
	// Post process volume previews is only possible with perspective level viewports.
	check( ViewportClient->IsPerspective() );
	ViewportClient->bPostProcessVolumePrevis = In.IsChecked();
	ViewportClient->Invalidate( FALSE );
}

void WxLevelViewportToolBar::OnViewportLocked( wxCommandEvent& In )
{
	ViewportClient->bViewportLocked = In.IsChecked();
}

void WxLevelViewportToolBar::SetViewModeUI(INT ViewModeID)
{
	ToggleTool( IDM_BRUSHWIREFRAME, false );
	ToggleTool( IDM_WIREFRAME, false );
	ToggleTool( IDM_UNLIT, false );
	ToggleTool( IDM_LIT, false );
	ToggleTool( IDM_LIGHTINGONLY, false );
	ToggleTool( IDM_LIGHTCOMPLEXITY, false );
	ToggleTool( IDM_TEXTUREDENSITY, false );
	ToggleTool( IDM_SHADERCOMPLEXITY, false );
	switch( ViewModeID )
	{
	case 0:					ToggleTool( IDM_BRUSHWIREFRAME, true );break;
	case 1:					ToggleTool( IDM_WIREFRAME, true );break;
	case 2:					ToggleTool( IDM_UNLIT, true );break;
	case 3:					ToggleTool( IDM_LIT, true );break;
	case 4:					ToggleTool( IDM_LIGHTINGONLY, true );break;
	case 5:					ToggleTool( IDM_LIGHTCOMPLEXITY, true );break;
	case 6:					ToggleTool( IDM_TEXTUREDENSITY, true );break;
	case 7:					ToggleTool( IDM_SHADERCOMPLEXITY, true );break;
	default:
		break;
	}
}

void WxLevelViewportToolBar::SetViewportTypeUI(INT ViewportTypeID)
{
	ToggleTool( IDM_PERSPECTIVE, false );
	ToggleTool( IDM_TOP, false );
	ToggleTool( IDM_FRONT, false );
	ToggleTool( IDM_SIDE, false );
	switch( ViewportTypeID )
	{
	case 0:					ToggleTool( IDM_PERSPECTIVE, true );break;
	case 1:					ToggleTool( IDM_TOP, true );break;
	case 2:					ToggleTool( IDM_FRONT, true );break;
	case 3:					ToggleTool( IDM_SIDE, true );break;
	default:
		break;
	}

	// Level streaming and post process volume previs is only possible with perspective level viewports.
	const bool bIsPerspectiveView = ViewportClient->ViewportType == LVT_Perspective;
	EnableTool( IDM_LevelStreamingVolumePreVis, bIsPerspectiveView );
	EnableTool( IDM_PostProcessVolumePreVis, bIsPerspectiveView );
	EnableTool( IDM_MakeOcclusionParent, bIsPerspectiveView );
	EnableTool( IDM_AllowMatineePreview, bIsPerspectiveView );

	if ( bIsPerspectiveView )
	{
		ToggleTool( IDM_LevelStreamingVolumePreVis, ViewportClient->bLevelStreamingVolumePrevis==TRUE );
		ToggleTool( IDM_PostProcessVolumePreVis, ViewportClient->bPostProcessVolumePrevis==TRUE );
		ToggleTool( IDM_MakeOcclusionParent, ViewportClient->ViewState->IsViewParent()==TRUE );
	}

	ToggleTool( IDM_AllowMatineePreview, bIsPerspectiveView && ViewportClient->AllowMatineePreview() );
}

void WxLevelViewportToolBar::OnBrushWireframe( wxCommandEvent& In )
{
	ViewportClient->ShowFlags &= ~SHOW_ViewMode_Mask;
	ViewportClient->ShowFlags |= SHOW_ViewMode_BrushWireframe;
	SetViewModeUI( 0 );
	ViewportClient->Invalidate();
}

void WxLevelViewportToolBar::OnWireframe( wxCommandEvent& In )
{
	ViewportClient->ShowFlags &= ~SHOW_ViewMode_Mask;
	ViewportClient->ShowFlags |= SHOW_ViewMode_Wireframe;
	SetViewModeUI( 1 );
	ViewportClient->Invalidate();
}

void WxLevelViewportToolBar::OnUnlit( wxCommandEvent& In )
{
	ViewportClient->ShowFlags &= ~SHOW_ViewMode_Mask;
	ViewportClient->ShowFlags |= SHOW_ViewMode_Unlit;
	SetViewModeUI( 2 );
	ViewportClient->Invalidate();
}

void WxLevelViewportToolBar::OnLit( wxCommandEvent& In )
{
	ViewportClient->ShowFlags &= ~SHOW_ViewMode_Mask;
	ViewportClient->ShowFlags |= SHOW_ViewMode_Lit;
	SetViewModeUI( 3 );
	ViewportClient->Invalidate();
}

void WxLevelViewportToolBar::OnLightingOnly( wxCommandEvent& In )
{
	ViewportClient->ShowFlags &= ~SHOW_ViewMode_Mask;
	ViewportClient->ShowFlags |= SHOW_ViewMode_LightingOnly;
	SetViewModeUI( 4 );
	ViewportClient->Invalidate();
}

void WxLevelViewportToolBar::OnLightComplexity( wxCommandEvent& In )
{
	ViewportClient->ShowFlags &= ~SHOW_ViewMode_Mask;
	ViewportClient->ShowFlags |= SHOW_ViewMode_LightComplexity;
	SetViewModeUI( 5 );
	ViewportClient->Invalidate();
}

void WxLevelViewportToolBar::OnTextureDensity( wxCommandEvent& In )
{
	ViewportClient->ShowFlags &= ~SHOW_ViewMode_Mask;
	ViewportClient->ShowFlags |= SHOW_ViewMode_TextureDensity;
	SetViewModeUI( 6 );
	ViewportClient->Invalidate();
}

void WxLevelViewportToolBar::OnShaderComplexity( wxCommandEvent& In )
{
	ViewportClient->ShowFlags &= ~SHOW_ViewMode_Mask;
	ViewportClient->ShowFlags |= SHOW_ViewMode_ShaderComplexity;;
	SetViewModeUI( 7 );
	ViewportClient->Invalidate();
}

void WxLevelViewportToolBar::OnPerspective( wxCommandEvent& In )
{
	ViewportClient->ViewportType = LVT_Perspective;
	SetViewportTypeUI( 0 );
	ViewportClient->Invalidate();
}

void WxLevelViewportToolBar::OnTop( wxCommandEvent& In )
{
	ViewportClient->ViewportType = LVT_OrthoXY;
	SetViewportTypeUI( 1 );
	ViewportClient->Invalidate();
}

void WxLevelViewportToolBar::OnFront( wxCommandEvent& In )
{
	ViewportClient->ViewportType = LVT_OrthoYZ;
	SetViewportTypeUI( 2 );
	ViewportClient->Invalidate();
}

void WxLevelViewportToolBar::OnSide( wxCommandEvent& In )
{
	ViewportClient->ViewportType = LVT_OrthoXZ;
	SetViewportTypeUI( 3 );
	ViewportClient->Invalidate();
}

void WxLevelViewportToolBar::OnMaximizeViewport( wxCommandEvent& In )
{
	if ( GApp && GApp->EditorFrame && GApp->EditorFrame->ViewportConfigData )
	{
		FViewportConfig_Data *ViewportConfig = GApp->EditorFrame->ViewportConfigData;
		ViewportConfig->ToggleMaximize( ViewportClient->Viewport );
		ViewportClient->Invalidate();
	}
}


/** Updates UI state for the 'Maximize Viewport' toolbar button */
void WxLevelViewportToolBar::UpdateMaximizeViewportUI( wxUpdateUIEvent& In )
{
	UBOOL bIsMaximized = FALSE;

	if ( GApp && GApp->EditorFrame && GApp->EditorFrame->ViewportConfigData )
	{
		if( GApp->EditorFrame->ViewportConfigData->IsViewportMaximized() )
		{
			const INT MaximizedViewportIndex = GApp->EditorFrame->ViewportConfigData->MaximizedViewport;
			if( MaximizedViewportIndex >= 0 && MaximizedViewportIndex < 4 )
			{
				if( GApp->EditorFrame->ViewportConfigData->Viewports[ MaximizedViewportIndex ].ViewportWindow == ViewportClient )
				{
					// This viewport is maximized, so check the button!
					bIsMaximized = TRUE;
				}
			}
		}
	}

	In.Check( bIsMaximized ? true : false );
}


void WxLevelViewportToolBar::OnTearOffNewFloatingViewport( wxCommandEvent& In )
{
	if ( GApp && GApp->EditorFrame && GApp->EditorFrame->ViewportConfigData )
	{
		FViewportConfig_Data *ViewportConfig = GApp->EditorFrame->ViewportConfigData;

		// The 'torn off copy' will have the same dimensions and settings as the original window
		const INT ViewportWidth = ViewportClient->Viewport->GetSizeX();
		const INT ViewportHeight = ViewportClient->Viewport->GetSizeY();
		ELevelViewportType ViewportType = ViewportClient->ViewportType;
		EShowFlags ShowFlags = ViewportClient->ShowFlags;

		// Create the new floating viewport
		INT NewViewportIndex = INDEX_NONE;
		UBOOL bResultValue = 
			ViewportConfig->OpenNewFloatingViewport(
				( wxWindow* )GApp->EditorFrame,	// Parent window
				ViewportType,					// Viewport type
				ShowFlags,						// Show flags
				ViewportWidth,					// Width
				ViewportHeight,					// Height
				NewViewportIndex );				// [out] New viewport index

		if( bResultValue )
		{
			// OK, now copy various settings from our viewport into the newly created viewport
			FVCD_Viewport& NewViewport = ViewportConfig->AccessViewport( NewViewportIndex );
			WxLevelViewportWindow* NewViewportWin = NewViewport.ViewportWindow;
			if( NewViewportWin != NULL )
			{
				NewViewportWin->CopyLayoutFromViewport( *ViewportClient );
				NewViewportWin->Invalidate();
				NewViewportWin->ToolBar->UpdateUI();
			}
		}
		else
		{
			appMsgf( AMT_OK, *LocalizeUnrealEd( "OpenNewFloatingViewport_Error" ) );
		}
	}
}



void WxLevelViewportToolBar::OnLockSelectedToCamera( wxCommandEvent& In )
{
	if ( GApp && GApp->EditorFrame && GApp->EditorFrame->ViewportConfigData )
	{
		FViewportConfig_Data *ViewportConfig = GApp->EditorFrame->ViewportConfigData;
		ViewportClient->bLockSelectedToCamera = In.IsChecked();

		if( !ViewportClient->bLockSelectedToCamera )
		{
			ViewportClient->ViewRotation.Pitch = 0;
			ViewportClient->ViewRotation.Roll = 0;
		}
	}
}

void WxLevelViewportToolBar::OnMakeParentViewport( wxCommandEvent& In )
{
	if ( GApp && GApp->EditorFrame && GApp->EditorFrame->ViewportConfigData )
	{
		// Allow only perspective views to be view parents
		const UBOOL bIsPerspectiveView = ViewportClient->IsPerspective();
		if ( bIsPerspectiveView )
		{
			FlushRenderingCommands();
			const UBOOL bIsViewParent = ViewportClient->ViewState->IsViewParent();

			// First, clear all existing view parent status.
			for( INT ViewportIndex = 0 ; ViewportIndex < GApp->EditorFrame->ViewportConfigData->GetViewportCount() ; ++ViewportIndex )
			{
				FVCD_Viewport& CurViewport = GApp->EditorFrame->ViewportConfigData->AccessViewport( ViewportIndex );
				if( CurViewport.bEnabled )
				{
					CurViewport.ViewportWindow->ViewState->SetViewParent( NULL );
					CurViewport.ViewportWindow->Invalidate();
				}
			}

			// If the view was not a parent, we're toggling occlusion parenting 'on' for this viewport.
			if ( !bIsViewParent )
			{
				for( INT ViewportIndex = 0 ; ViewportIndex < GApp->EditorFrame->ViewportConfigData->GetViewportCount() ; ++ViewportIndex )
				{
					FVCD_Viewport& CurViewport = GApp->EditorFrame->ViewportConfigData->AccessViewport( ViewportIndex );
					if( CurViewport.bEnabled )
					{
						CurViewport.ViewportWindow->ViewState->SetViewParent( ViewportClient->ViewState );
					}
				}
			}

			// Finally, update level viewport toolbar UI.
			for( INT ViewportIndex = 0 ; ViewportIndex < GApp->EditorFrame->ViewportConfigData->GetViewportCount() ; ++ViewportIndex )
			{
				FVCD_Viewport& CurViewport = GApp->EditorFrame->ViewportConfigData->AccessViewport( ViewportIndex );
				if( CurViewport.bEnabled )
				{
					CurViewport.ViewportWindow->ToolBar->UpdateUI();
				}
			}
		}
	}
}

void WxLevelViewportToolBar::OnViewportTypeSelChange( wxCommandEvent& In )
{
	switch( In.GetInt() )
	{
		case 0:					ViewportClient->ViewportType = LVT_Perspective;		break;
		case 1:					ViewportClient->ViewportType = LVT_OrthoXY;			break;
		case 2:					ViewportClient->ViewportType = LVT_OrthoYZ;			break;
		case 3:					ViewportClient->ViewportType = LVT_OrthoXZ;			break;
		default:
			break;
	}
	SetViewportTypeUI( In.GetInt() );
	ViewportClient->Invalidate();
}

void WxLevelViewportToolBar::OnSceneViewModeSelChange( wxCommandEvent& In )
{
	ViewportClient->ShowFlags &= ~SHOW_ViewMode_Mask;
	switch( In.GetInt() )
	{
		case 0: ViewportClient->ShowFlags |= SHOW_ViewMode_BrushWireframe; break;
		case 1: ViewportClient->ShowFlags |= SHOW_ViewMode_Wireframe; break;
		case 2: ViewportClient->ShowFlags |= SHOW_ViewMode_Unlit; break;
		default:
		case 3: ViewportClient->ShowFlags |= SHOW_ViewMode_Lit; break;
		case 4:	ViewportClient->ShowFlags |= SHOW_ViewMode_LightingOnly; break;
		case 5:	ViewportClient->ShowFlags |= SHOW_ViewMode_LightComplexity; break;
		case 6:	ViewportClient->ShowFlags |= SHOW_ViewMode_TextureDensity; break;
		case 7:	ViewportClient->ShowFlags |= SHOW_ViewMode_ShaderComplexity; break;
			break;
	}
	SetViewModeUI( In.GetInt() );
	ViewportClient->Invalidate();
}

class FShowFlagData
{
public:
	INT			ID;
	FString		Name;
	EShowFlags	Mask;

	FShowFlagData(INT InID, const FString& InName, const EShowFlags& InMask)
		:	ID( InID )
		,	Name( InName )
		,	Mask( InMask )
	{}
};

IMPLEMENT_COMPARE_CONSTREF( FShowFlagData, LevelViewportToolBar, { return appStricmp(*A.Name, *B.Name); } )

/**
 * Present the user with an alphabetical listing of available show flags.
 */
void WxLevelViewportToolBar::OnShowFlags( wxCommandEvent& In )
{
	TArray<FShowFlagData> ShowFlags;
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_BSP, *LocalizeUnrealEd("BSPSF"), SHOW_BSP ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_BSPSPLIT, *LocalizeUnrealEd("BSPSplitSF"), SHOW_BSPSplit ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_ACTORTAGS, *LocalizeUnrealEd("ActorTagsSF"), SHOW_ActorTags ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_BOUNDS, *LocalizeUnrealEd("BoundsSF"), SHOW_Bounds ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_AUDIORADIUS, *LocalizeUnrealEd("AudioRadiusSF"), SHOW_AudioRadius ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_BUILDERBRUSH, *LocalizeUnrealEd("BuilderBrushSF"), SHOW_BuilderBrush ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_CAMFRUSTUMS, *LocalizeUnrealEd("CameraFrustumsSF"), SHOW_CamFrustums ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_COLLISION, *LocalizeUnrealEd("CollisionSF"), SHOW_Collision ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_DECALS, *LocalizeUnrealEd("DecalsSF"), SHOW_Decals ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_DECALINFO, *LocalizeUnrealEd("DecalInfoSF"), SHOW_DecalInfo ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_FOG, *LocalizeUnrealEd("FogSF"), SHOW_Fog ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_FOLIAGE, *LocalizeUnrealEd("FoliageSF"), SHOW_Foliage ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_GRID, *LocalizeUnrealEd("GridSF"), SHOW_Grid ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_HITPROXIES, *LocalizeUnrealEd("HitProxiesSF"), SHOW_HitProxies ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_KISMETREFS, *LocalizeUnrealEd("KismetReferencesSF"), SHOW_KismetRefs ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_LARGEVERTICES, *LocalizeUnrealEd("LargeVerticesSF"), SHOW_LargeVertices ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_LENSFLARES, *LocalizeUnrealEd("LensFlareSF"), SHOW_LensFlares ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_LEVELCOLORATION, *LocalizeUnrealEd("LevelColorationSF"), SHOW_LevelColoration ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_PROPERTYCOLORATION, *LocalizeUnrealEd("PropertyColorationSF"), SHOW_PropertyColoration ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_LIGHTINFLUENCES, *LocalizeUnrealEd("LightInfluencesSF"), SHOW_LightInfluences ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_LIGHTRADIUS, *LocalizeUnrealEd("LightRadiusSF"), SHOW_LightRadius ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_MISSINGCOLLISION, *LocalizeUnrealEd("MissingCollisionSF"), SHOW_MissingCollisionModel ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_MESHEDGES, *LocalizeUnrealEd("MeshEdgesSF"), SHOW_MeshEdges ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_MODEWIDGETS, *LocalizeUnrealEd("ModeWidgetsSF"), SHOW_ModeWidgets ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_NAVNODES, *LocalizeUnrealEd("NavigationNodesSF"), SHOW_NavigationNodes ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_PATHS, *LocalizeUnrealEd("PathsSF"), SHOW_Paths ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_PORTALS, *LocalizeUnrealEd("PortalsSF"), SHOW_Portals ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_POSTPROCESS, *LocalizeUnrealEd("PostProcessSF"), SHOW_PostProcess ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_SELECTION, *LocalizeUnrealEd("SelectionSF"), SHOW_Selection ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_SHADOWFRUSTUMS, *LocalizeUnrealEd("ShadowFrustumsSF"), SHOW_ShadowFrustums ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_SPRITES, *LocalizeUnrealEd("SpritesSF"), SHOW_Sprites ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_CONSTRAINTS, *LocalizeUnrealEd("ConstraintsSF"), SHOW_Constraints ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_DYNAMICSHADOWS, *LocalizeUnrealEd("DynamicShadowsSF"), SHOW_DynamicShadows ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_PARTICLES, *LocalizeUnrealEd("ParticlesSF"), SHOW_Particles ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_STATICMESHES, *LocalizeUnrealEd("StaticMeshesSF"), SHOW_StaticMeshes ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_SkeletalMeshes, *LocalizeUnrealEd("SkeletalMeshesSF"), SHOW_SkeletalMeshes ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_SpeedTrees, *LocalizeUnrealEd("SpeedTreesSF"), SHOW_SpeedTrees ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_TERRAIN, *LocalizeUnrealEd("TerrainSF"), SHOW_Terrain ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_TERRAIN_PATCHES, *LocalizeUnrealEd("TerrainPatchesSF"), SHOW_TerrainPatches ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_TERRAIN_COLLISION, *LocalizeUnrealEd("TerrainCollisionSF"), SHOW_TerrainCollision ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_UNLITTRANSLUCENCY, *LocalizeUnrealEd("UnlitTranslucencySF"), SHOW_UnlitTranslucency ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_VOLUMES, *LocalizeUnrealEd("VolumesSF"), SHOW_Volumes ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_SCENECAPTUREUPDATES, *LocalizeUnrealEd("SceneCaptureUpdatesSF"), SHOW_SceneCaptureUpdates ) );
	ShowFlags.AddItem( FShowFlagData( IDM_VIEWPORT_SHOW_SENTINELSTATS, *LocalizeUnrealEd("SentinelStatsSF"), SHOW_SentinelStats ) );

	// Sort the show flags alphabetically by string.
	Sort<USE_COMPARE_CONSTREF(FShowFlagData,LevelViewportToolBar)>( ShowFlags.GetTypedData(), ShowFlags.Num() );

	// Create a new menu and add the flags in alphabetical order.
	wxMenu* menu = new wxMenu( *LocalizeUnrealEd("SHOWFLAGS") );

	menu->Append(IDM_VIEWPORT_SHOW_DEFAULTS, *LocalizeUnrealEd("DefaultShowFlags"));

	wxMenu* CollisionMenu = new wxMenu();

	CollisionMenu->AppendCheckItem(IDM_VIEWPORT_SHOW_COLLISION_ZEROEXTENT, *LocalizeUnrealEd("ZeroExtent") );
	CollisionMenu->Check(IDM_VIEWPORT_SHOW_COLLISION_ZEROEXTENT, (ViewportClient->ShowFlags & SHOW_CollisionZeroExtent) ? true : false);

	CollisionMenu->AppendCheckItem(IDM_VIEWPORT_SHOW_COLLISION_NONZEROEXTENT, *LocalizeUnrealEd("NonZeroExtent") );
	CollisionMenu->Check(IDM_VIEWPORT_SHOW_COLLISION_NONZEROEXTENT, (ViewportClient->ShowFlags & SHOW_CollisionNonZeroExtent) ? true : false);

	CollisionMenu->AppendCheckItem(IDM_VIEWPORT_SHOW_COLLISION_RIGIDBODY, *LocalizeUnrealEd("RigidBody") );
	CollisionMenu->Check(IDM_VIEWPORT_SHOW_COLLISION_RIGIDBODY, (ViewportClient->ShowFlags & SHOW_CollisionRigidBody) ? true : false);

	CollisionMenu->AppendCheckItem(IDM_VIEWPORT_SHOW_COLLISION_NONE, *LocalizeUnrealEd("Normal") );
	CollisionMenu->Check(IDM_VIEWPORT_SHOW_COLLISION_NONE, !(ViewportClient->ShowFlags & SHOW_Collision_Any));

	menu->Append( IDM_VIEWPORT_SHOW_COLLISIONMENU, *LocalizeUnrealEd("CollisionModes"), CollisionMenu );

	menu->AppendSeparator();

	for ( INT i = 0 ; i < ShowFlags.Num() ; ++i )
	{
		const FShowFlagData& ShowFlagData = ShowFlags(i);

		menu->AppendCheckItem( ShowFlagData.ID, *ShowFlagData.Name );
		const UBOOL bShowFlagEnabled = (ViewportClient->ShowFlags & ShowFlagData.Mask) ? TRUE : FALSE;
		menu->Check( ShowFlagData.ID, bShowFlagEnabled == TRUE );
	}

	FTrackPopupMenu tpm( this, menu );
	tpm.Show();
	delete menu;
}

void WxLevelViewportToolBar::OnShowDefaults( wxCommandEvent& In )
{
	// Setting show flags to the defaults should not stomp on the current viewmode settings.
	const EShowFlags ViewModeShowFlags = ViewportClient->ShowFlags &= SHOW_ViewMode_Mask;
	ViewportClient->ShowFlags = SHOW_DefaultEditor;
	ViewportClient->ShowFlags |= ViewModeShowFlags;

	ViewportClient->Invalidate();
}

void WxLevelViewportToolBar::OnShowStaticMeshes( wxCommandEvent& In )			{	ViewportClient->ShowFlags ^= SHOW_StaticMeshes;				ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowSkeletalMeshes( wxCommandEvent& In )			{	ViewportClient->ShowFlags ^= SHOW_SkeletalMeshes;			ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowSpeedTrees( wxCommandEvent& In )				{	ViewportClient->ShowFlags ^= SHOW_SpeedTrees;				ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowTerrain( wxCommandEvent& In )				{	ViewportClient->ShowFlags ^= SHOW_Terrain;					ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowFoliage( wxCommandEvent& In )				{	ViewportClient->ShowFlags ^= SHOW_Foliage;					ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowBSP( wxCommandEvent& In )					{	ViewportClient->ShowFlags ^= SHOW_BSP;						ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowBSPSplit( wxCommandEvent& In )				{	ViewportClient->ShowFlags ^= SHOW_BSPSplit;					ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowCollision( wxCommandEvent& In )				{	ViewportClient->ShowFlags ^= SHOW_Collision;				ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowGrid( wxCommandEvent& In )					{	ViewportClient->ShowFlags ^= SHOW_Grid;						ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowBounds( wxCommandEvent& In )					{	ViewportClient->ShowFlags ^= SHOW_Bounds;					ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowPaths( wxCommandEvent& In )					{	ViewportClient->ShowFlags ^= SHOW_Paths;					ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowNavigationNodes( wxCommandEvent& In )		{	ViewportClient->ShowFlags ^= SHOW_NavigationNodes;			ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowMeshEdges( wxCommandEvent& In )				{	ViewportClient->ShowFlags ^= SHOW_MeshEdges;				ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowModeWidgets( wxCommandEvent& In )			{	ViewportClient->ShowFlags ^= SHOW_ModeWidgets;				ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowLargeVertices( wxCommandEvent& In )			{	ViewportClient->ShowFlags ^= SHOW_LargeVertices;			ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowPortals( wxCommandEvent& In )				{	ViewportClient->ShowFlags ^= SHOW_Portals;					ViewportClient->Invalidate(); }
void WxLevelViewportToolBar::OnShowHitProxies( wxCommandEvent& In )				{	ViewportClient->ShowFlags ^= SHOW_HitProxies;				ViewportClient->Invalidate(); }
void WxLevelViewportToolBar::OnShowShadowFrustums( wxCommandEvent& In )			{	ViewportClient->ShowFlags ^= SHOW_ShadowFrustums;			ViewportClient->Invalidate(); }
void WxLevelViewportToolBar::OnShowKismetRefs( wxCommandEvent& In )				{	ViewportClient->ShowFlags ^= SHOW_KismetRefs;				ViewportClient->Invalidate(); }
void WxLevelViewportToolBar::OnShowVolumes( wxCommandEvent& In )				{	ViewportClient->ShowFlags ^= SHOW_Volumes;					ViewportClient->Invalidate(); }
void WxLevelViewportToolBar::OnShowFog( wxCommandEvent& In )					{	ViewportClient->ShowFlags ^= SHOW_Fog;						ViewportClient->Invalidate(); }
void WxLevelViewportToolBar::OnShowCamFrustums( wxCommandEvent& In )			{	ViewportClient->ShowFlags ^= SHOW_CamFrustums;				ViewportClient->Invalidate(); }
void WxLevelViewportToolBar::OnShowParticles( wxCommandEvent& In)				{	ViewportClient->ShowFlags ^= SHOW_Particles;				ViewportClient->Invalidate(); }
void WxLevelViewportToolBar::OnShowLightInfluences( wxCommandEvent& In)			{	ViewportClient->ShowFlags ^= SHOW_LightInfluences;			ViewportClient->Invalidate(); }
void WxLevelViewportToolBar::OnShowSelection( wxCommandEvent& In )				{	ViewportClient->ShowFlags ^= SHOW_Selection;				ViewportClient->Invalidate(); }
void WxLevelViewportToolBar::OnShowBuilderBrush( wxCommandEvent& In)			{	ViewportClient->ShowFlags ^= SHOW_BuilderBrush;				ViewportClient->Invalidate(); }
void WxLevelViewportToolBar::OnShowActorTags( wxCommandEvent& In)				{	ViewportClient->ShowFlags ^= SHOW_ActorTags;				ViewportClient->Invalidate(); }
void WxLevelViewportToolBar::OnShowMissingCollision( wxCommandEvent& In)		{	ViewportClient->ShowFlags ^= SHOW_MissingCollisionModel;	ViewportClient->Invalidate(); }
void WxLevelViewportToolBar::OnShowDecals( wxCommandEvent& In)					{	ViewportClient->ShowFlags ^= SHOW_Decals;					ViewportClient->Invalidate(); }
void WxLevelViewportToolBar::OnShowDecalInfo( wxCommandEvent& In)				{	ViewportClient->ShowFlags ^= SHOW_DecalInfo;				ViewportClient->Invalidate(); }
void WxLevelViewportToolBar::OnShowLightRadius( wxCommandEvent& In)				{	ViewportClient->ShowFlags ^= SHOW_LightRadius;				ViewportClient->Invalidate(); }
void WxLevelViewportToolBar::OnShowAudioRadius( wxCommandEvent& In)				{	ViewportClient->ShowFlags ^= SHOW_AudioRadius;				ViewportClient->Invalidate(); }
void WxLevelViewportToolBar::OnShowTerrainPatches( wxCommandEvent& In )			{	ViewportClient->ShowFlags ^= SHOW_TerrainPatches;			ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowTerrainCollision( wxCommandEvent& In )		{	ViewportClient->ShowFlags ^= SHOW_TerrainCollision;			ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowPostProcess( wxCommandEvent& In )			{	ViewportClient->ShowFlags ^= SHOW_PostProcess;				ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowUnlitTranslucency( wxCommandEvent& In )		{	ViewportClient->ShowFlags ^= SHOW_UnlitTranslucency;		ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowSceneCaptureUpdates( wxCommandEvent& In )	{	ViewportClient->ShowFlags ^= SHOW_SceneCaptureUpdates;		ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowLevelColoration( wxCommandEvent& In )		{	ViewportClient->ShowFlags ^= SHOW_LevelColoration;			ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowPropertyColoration( wxCommandEvent& In )		{	ViewportClient->ShowFlags ^= SHOW_PropertyColoration;		ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowSprites( wxCommandEvent& In )				{	ViewportClient->ShowFlags ^= SHOW_Sprites;					ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowConstraints( wxCommandEvent& In )			{	ViewportClient->ShowFlags ^= SHOW_Constraints;				ViewportClient->Invalidate();	}
void WxLevelViewportToolBar::OnShowDynamicShadows( wxCommandEvent& In )			{	ViewportClient->ShowFlags ^= SHOW_DynamicShadows;			ViewportClient->Invalidate(); }
void WxLevelViewportToolBar::OnShowLensFlares( wxCommandEvent& In )				{	ViewportClient->ShowFlags ^= SHOW_LensFlares;				ViewportClient->Invalidate(); }
void WxLevelViewportToolBar::OnShowSentinelStats( wxCommandEvent& In )			{	ViewportClient->ShowFlags ^= SHOW_SentinelStats;			ViewportClient->Invalidate(); }

/** Handle choosing one of the collision view mode options */
void WxLevelViewportToolBar::OnChangeCollisionMode(wxCommandEvent& In)
{
	INT Id = In.GetId();
	
	// Turn off all collision flags
	ViewportClient->ShowFlags &= ~SHOW_Collision_Any;

	// Then set the one we want
	if(Id == IDM_VIEWPORT_SHOW_COLLISION_ZEROEXTENT)
	{
		ViewportClient->ShowFlags |= SHOW_CollisionZeroExtent;
	}
	else if(Id == IDM_VIEWPORT_SHOW_COLLISION_NONZEROEXTENT)
	{
		ViewportClient->ShowFlags |= SHOW_CollisionNonZeroExtent;
	}
	else if(Id == IDM_VIEWPORT_SHOW_COLLISION_RIGIDBODY)
	{
		ViewportClient->ShowFlags |= SHOW_CollisionRigidBody;
	}

	ViewportClient->Invalidate();
}

/**
 * Returns the level viewport toolbar height, in pixels.
 */
INT WxLevelViewportToolBar::GetToolbarHeight()
{
	return 28;
}
