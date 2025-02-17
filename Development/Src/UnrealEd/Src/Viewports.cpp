/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "UnrealEd.h"
#include "LevelViewportToolBar.h"
#include "ViewportsContainer.h"


namespace EditorViewportDefs
{
	/** Default camera position for level editor perspective viewports */
	const FVector DefaultPerspectiveViewLocation( -1024.0f, 0.0f, 512.0f );

	/** Default camera orientation for level editor perspective viewports */
	const FRotator DefaultPerspectiveViewRotation( appRound( -15.0f * (32768.f / 180.f) ), 0.0f, 0.0f );
}



/*-----------------------------------------------------------------------------
	FViewportConfig_Viewport.
-----------------------------------------------------------------------------*/

FViewportConfig_Viewport::FViewportConfig_Viewport()
{
	ViewportType = LVT_Perspective;
	ShowFlags = (SHOW_DefaultEditor&~SHOW_ViewMode_Mask) | SHOW_ViewMode_BrushWireframe | SHOW_ModeWidgets;
	bEnabled = 0;
	bSetListenerPosition = 0;
}

FViewportConfig_Viewport::~FViewportConfig_Viewport()
{
}

/*-----------------------------------------------------------------------------
	FViewportConfig_Template.
-----------------------------------------------------------------------------*/

FViewportConfig_Template::FViewportConfig_Template()
{
}

FViewportConfig_Template::~FViewportConfig_Template()
{
}

void FViewportConfig_Template::Set( EViewportConfig InViewportConfig )
{
	ViewportConfig = InViewportConfig;

	switch( ViewportConfig )
	{
		case VC_2_2_Split:

			Desc = *LocalizeUnrealEd("2x2Split");

			ViewportTemplates[0].bEnabled = 1;
			ViewportTemplates[0].ViewportType = LVT_OrthoXY;
			ViewportTemplates[0].ShowFlags = (SHOW_DefaultEditor&~SHOW_ViewMode_Mask) | SHOW_ViewMode_BrushWireframe;

			ViewportTemplates[1].bEnabled = 1;
			ViewportTemplates[1].ViewportType = LVT_OrthoXZ;
			ViewportTemplates[1].ShowFlags = (SHOW_DefaultEditor&~SHOW_ViewMode_Mask) | SHOW_ViewMode_BrushWireframe;

			ViewportTemplates[2].bEnabled = 1;
			ViewportTemplates[2].ViewportType = LVT_OrthoYZ;
			ViewportTemplates[2].ShowFlags = (SHOW_DefaultEditor&~SHOW_ViewMode_Mask) | SHOW_ViewMode_BrushWireframe;

			ViewportTemplates[3].bEnabled = 1;
			ViewportTemplates[3].bSetListenerPosition = 1;
			ViewportTemplates[3].ViewportType = LVT_Perspective;
			ViewportTemplates[3].ShowFlags = (SHOW_DefaultEditor&~SHOW_ViewMode_Mask) | SHOW_ViewMode_Lit;

			break;

		case VC_1_2_Split:

			Desc = *LocalizeUnrealEd("1x2Split");

			ViewportTemplates[0].bEnabled = 1;
			ViewportTemplates[0].bSetListenerPosition = 1;
			ViewportTemplates[0].ViewportType = LVT_Perspective;
			ViewportTemplates[0].ShowFlags = (SHOW_DefaultEditor&~SHOW_ViewMode_Mask) | SHOW_ViewMode_Lit;

			ViewportTemplates[1].bEnabled = 1;
			ViewportTemplates[1].ViewportType = LVT_OrthoXY;
			ViewportTemplates[1].ShowFlags = (SHOW_DefaultEditor&~SHOW_ViewMode_Mask) | SHOW_ViewMode_BrushWireframe;

			ViewportTemplates[2].bEnabled = 1;
			ViewportTemplates[2].ViewportType = LVT_OrthoXZ;
			ViewportTemplates[2].ShowFlags = (SHOW_DefaultEditor&~SHOW_ViewMode_Mask) | SHOW_ViewMode_BrushWireframe;

			break;

		case VC_1_1_SplitH:

			Desc = *LocalizeUnrealEd("1x1SplitH");

			ViewportTemplates[0].bEnabled = 1;
			ViewportTemplates[0].bSetListenerPosition = 1;
			ViewportTemplates[0].ViewportType = LVT_Perspective;
			ViewportTemplates[0].ShowFlags = (SHOW_DefaultEditor&~SHOW_ViewMode_Mask) | SHOW_ViewMode_Lit;

			ViewportTemplates[1].bEnabled = 1;
			ViewportTemplates[1].ViewportType = LVT_OrthoXY;
			ViewportTemplates[1].ShowFlags = (SHOW_DefaultEditor&~SHOW_ViewMode_Mask) | SHOW_ViewMode_BrushWireframe;

			break;

		case VC_1_1_SplitV:

			Desc = *LocalizeUnrealEd("1x1SplitV");

			ViewportTemplates[0].bEnabled = 1;
			ViewportTemplates[0].bSetListenerPosition = 1;
			ViewportTemplates[0].ViewportType = LVT_Perspective;
			ViewportTemplates[0].ShowFlags = (SHOW_DefaultEditor&~SHOW_ViewMode_Mask) | SHOW_ViewMode_Lit;

			ViewportTemplates[1].bEnabled = 1;
			ViewportTemplates[1].ViewportType = LVT_OrthoXY;
			ViewportTemplates[1].ShowFlags = (SHOW_DefaultEditor&~SHOW_ViewMode_Mask) | SHOW_ViewMode_BrushWireframe;

			break;

		default:
			check(0);	// Unknown viewport config
			break;
	}
}

/*-----------------------------------------------------------------------------
	FVCD_Viewport.
-----------------------------------------------------------------------------*/

FVCD_Viewport::FVCD_Viewport()
{
	ViewportWindow = NULL;
	FloatingViewportFrame = NULL;
	PIEContainerWindow = NULL;
	ShowFlags = (SHOW_DefaultEditor&~SHOW_ViewMode_Mask) | SHOW_ViewMode_BrushWireframe | SHOW_ModeWidgets;

	SashPos = 0;
}

FVCD_Viewport::~FVCD_Viewport()
{
}



/*-----------------------------------------------------------------------------
	FViewportConfig_Data.
-----------------------------------------------------------------------------*/

FViewportConfig_Data::FViewportConfig_Data()
{
	MaximizedViewport = -1;
}

FViewportConfig_Data::~FViewportConfig_Data()
{
	// Clean up floating viewports
	for( INT FloatingViewportIndex = 0; FloatingViewportIndex < FloatingViewports.Num(); ++FloatingViewportIndex )
	{
		FVCD_Viewport* FloatingViewport = FloatingViewports( FloatingViewportIndex );
		if( FloatingViewport != NULL )
		{
			// Unbind callback interface
			if( FloatingViewport->FloatingViewportFrame != NULL )
			{
				FloatingViewport->FloatingViewportFrame->SetCallbackInterface( NULL );
			}

			// NOTE: No other real cleanup work to do here; the floating windows will be destroyed by their parent
			//   window naturally
		}
	}
}

/**
 * Tells this viewport configuration to create its windows and apply its settings.
 */
void FViewportConfig_Data::Apply( WxViewportsContainer* InParent )
{
	// NOTE: Floating viewports currently won't be affected by this function.  Any floating windows will just
	// be left intact.

	for( INT CurViewportIndex = 0; CurViewportIndex < 4; ++CurViewportIndex )
	{
		FVCD_Viewport& CurViewport = Viewports[ CurViewportIndex ];
		if( CurViewport.bEnabled && CurViewport.ViewportWindow != NULL )
		{
			CurViewport.ViewportWindow->DestroyChildren();
			CurViewport.ViewportWindow->Destroy();
			CurViewport.ViewportWindow = NULL;
		}

		if( CurViewport.PIEContainerWindow != NULL )
		{
			CurViewport.PIEContainerWindow->DestroyChildren();
			CurViewport.PIEContainerWindow->Destroy();
		}
	}

	MaximizedViewport = -1;
	for( int CurSplitterIndex = 0; CurSplitterIndex < SplitterWindows.Num(); ++CurSplitterIndex )
	{
		SplitterWindows( CurSplitterIndex )->DestroyChildren();
		SplitterWindows( CurSplitterIndex )->Destroy();
	}
	SplitterWindows.Empty();


	wxRect rc = InParent->GetClientRect();

	// Set up the splitters and viewports as per the template defaults.

	wxSplitterWindow *MainSplitter = NULL;
	INT SashPos = 0;
	FString Key;

	switch( Template )
	{
		case VC_2_2_Split:
		{
			wxSplitterWindow *TopSplitter, *BottomSplitter;

			MainSplitter = new wxSplitterWindow( InParent, ID_SPLITTERWINDOW, wxPoint(0,0), wxSize(rc.GetWidth(), rc.GetHeight()), wxSP_3D );
			TopSplitter = new wxSplitterWindow( MainSplitter, ID_SPLITTERWINDOW+1, wxPoint(0,0), wxSize(rc.GetWidth(), rc.GetHeight()/2), wxSP_3D );
			BottomSplitter = new wxSplitterWindow( MainSplitter, ID_SPLITTERWINDOW+2, wxPoint(0,0), wxSize(rc.GetWidth(), rc.GetHeight()/2), wxSP_3D );

			SplitterWindows.AddItem( MainSplitter );
			SplitterWindows.AddItem( TopSplitter );
			SplitterWindows.AddItem( BottomSplitter );

			// Connect splitter events so we can link the top and bottom splitters if the ViewportResizeTogether option is set.
			InParent->ConnectSplitterEvents( TopSplitter, BottomSplitter );

			for( INT x = 0 ; x < 4 ; ++x )
			{
				if( Viewports[x].bEnabled )
				{
					Viewports[x].ViewportWindow = new WxLevelViewportWindow;
					Viewports[x].ViewportWindow->Create( MainSplitter, -1 );
					Viewports[x].ViewportWindow->SetUp( Viewports[x].ViewportType, Viewports[x].bSetListenerPosition, Viewports[x].ShowFlags );
					Viewports[x].ViewportWindow->SetLabel( wxT("LevelViewport") );
				}
			}

			Viewports[0].ViewportWindow->Reparent( TopSplitter );
			Viewports[1].ViewportWindow->Reparent( TopSplitter );
			Viewports[2].ViewportWindow->Reparent( BottomSplitter );
			Viewports[3].ViewportWindow->Reparent( BottomSplitter );

			MainSplitter->SetLabel( wxT("MainSplitter") );
			TopSplitter->SetLabel( wxT("TopSplitter") );
			BottomSplitter->SetLabel( wxT("BottomSplitter") );

			GConfig->GetInt( TEXT("ViewportConfig"), TEXT("Splitter0"), SashPos, GEditorUserSettingsIni );
			MainSplitter->SplitHorizontally( TopSplitter, BottomSplitter, SashPos );
			GConfig->GetInt( TEXT("ViewportConfig"), TEXT("Splitter1"), SashPos, GEditorUserSettingsIni );
			TopSplitter->SplitVertically( Viewports[0].ViewportWindow, Viewports[1].ViewportWindow, SashPos );
			GConfig->GetInt( TEXT("ViewportConfig"), TEXT("Splitter2"), SashPos, GEditorUserSettingsIni );
			BottomSplitter->SplitVertically( Viewports[3].ViewportWindow, Viewports[2].ViewportWindow, SashPos );
		}
		break;

		case VC_1_2_Split:
		{
			wxSplitterWindow *RightSplitter;

			MainSplitter = new wxSplitterWindow( InParent, ID_SPLITTERWINDOW, wxPoint(0,0), wxSize(rc.GetWidth(), rc.GetHeight()), wxSP_3D );
			RightSplitter = new wxSplitterWindow( MainSplitter, ID_SPLITTERWINDOW+1, wxPoint(0,0), wxSize(rc.GetWidth(), rc.GetHeight()/2), wxSP_3D );

			SplitterWindows.AddItem( MainSplitter );
			SplitterWindows.AddItem( RightSplitter );

			// Disconnect Splitter Events
			InParent->DisconnectSplitterEvents();

			for( INT x = 0 ; x < 4 ; ++x )
			{
				if( Viewports[x].bEnabled )
				{
					Viewports[x].ViewportWindow = new WxLevelViewportWindow;
					Viewports[x].ViewportWindow->Create( MainSplitter, -1 );
					Viewports[x].ViewportWindow->SetUp( Viewports[x].ViewportType, Viewports[x].bSetListenerPosition, Viewports[x].ShowFlags );
					Viewports[x].ViewportWindow->SetLabel( wxT("LevelViewport") );
				}
			}

			Viewports[0].ViewportWindow->Reparent( MainSplitter );
			Viewports[1].ViewportWindow->Reparent( RightSplitter );
			Viewports[2].ViewportWindow->Reparent( RightSplitter );

			MainSplitter->SetLabel( wxT("MainSplitter") );
			RightSplitter->SetLabel( wxT("RightSplitter") );

			GConfig->GetInt( TEXT("ViewportConfig"), TEXT("Splitter0"), SashPos, GEditorUserSettingsIni );
			MainSplitter->SplitVertically( Viewports[0].ViewportWindow, RightSplitter, SashPos );
			GConfig->GetInt( TEXT("ViewportConfig"), TEXT("Splitter1"), SashPos, GEditorUserSettingsIni );
			RightSplitter->SplitHorizontally( Viewports[1].ViewportWindow, Viewports[2].ViewportWindow, SashPos );
		}
		break;

		case VC_1_1_SplitH:
		case VC_1_1_SplitV:
		{
			MainSplitter = new wxSplitterWindow( InParent, ID_SPLITTERWINDOW, wxPoint(0,0), wxSize(rc.GetWidth(), rc.GetHeight()), wxSP_3D );

			SplitterWindows.AddItem( MainSplitter );

			// Disconnect Splitter Events
			InParent->DisconnectSplitterEvents();

			for( INT x = 0 ; x < 4 ; ++x )
			{
				if( Viewports[x].bEnabled )
				{
					Viewports[x].ViewportWindow = new WxLevelViewportWindow;
					Viewports[x].ViewportWindow->Create( MainSplitter, -1 );
					Viewports[x].ViewportWindow->SetUp( Viewports[x].ViewportType, Viewports[x].bSetListenerPosition, Viewports[x].ShowFlags );
					Viewports[x].ViewportWindow->SetLabel( wxT("LevelViewport") );
				}
			}

			Viewports[0].ViewportWindow->Reparent( MainSplitter );
			Viewports[1].ViewportWindow->Reparent( MainSplitter );

			MainSplitter->SetLabel( wxT("MainSplitter") );

			GConfig->GetInt( TEXT("ViewportConfig"), TEXT("Splitter0"), SashPos, GEditorUserSettingsIni );

			if(Template == VC_1_1_SplitH)
			{
				MainSplitter->SplitHorizontally( Viewports[0].ViewportWindow, Viewports[1].ViewportWindow, SashPos );
			}
			else
			{
				MainSplitter->SplitVertically( Viewports[0].ViewportWindow, Viewports[1].ViewportWindow, SashPos );
			}
		}
		break;
	}

	// Make sure the splitters will resize with the editor
	wxBoxSizer* WkSizer = new wxBoxSizer( wxHORIZONTAL );
	WkSizer->Add( MainSplitter, 1, wxEXPAND | wxALL, 0 );

	Sizer = new wxBoxSizer( wxVERTICAL );
	Sizer->Add( WkSizer, 1, wxEXPAND | wxALL, 0 );
	InParent->SetSizer( Sizer );
	
	// Apply the custom settings contained in this instance.

	for( INT x = 0 ; x < 4 ; ++x )
	{
		if( Viewports[x].bEnabled )
		{
			check(Viewports[x].ViewportWindow);
			FString Key = FString::Printf( TEXT("Viewport%d"), x );
			GConfig->GetFloat( TEXT("ViewportConfig"), *(Key+TEXT("_CameraSpeed")), Viewports[x].ViewportWindow->CameraSpeed, GEditorUserSettingsIni );
			Viewports[x].ViewportWindow->ViewportType = Viewports[x].ViewportType;
			Viewports[x].ViewportWindow->ShowFlags = Viewports[x].ShowFlags;
			Viewports[x].ViewportWindow->ToolBar->UpdateUI();

			if( Viewports[x].ViewportType == LVT_Perspective )
			{
				// Only perspective windows will have Matinee preview features turned on by default
				Viewports[x].ViewportWindow->SetAllowMatineePreview( TRUE );

				// Assign default camera location/rotation for perspective camera
				Viewports[x].ViewportWindow->ViewLocation = EditorViewportDefs::DefaultPerspectiveViewLocation;
				Viewports[x].ViewportWindow->ViewRotation = EditorViewportDefs::DefaultPerspectiveViewRotation;
			}
		}
	}
}

/*
 * Scales the splitter windows proportionally.
 * Used when switching between windowed and maximized mode.
 */
void FViewportConfig_Data::ResizeProportionally( FLOAT ScaleX, FLOAT ScaleY, UBOOL bRedraw/*=TRUE*/ )
{
	if ( MaximizedViewport >= 0 )
	{
		Layout();
	}
	else
	{
		for( INT x = 0 ; x < SplitterWindows.Num() ; ++x )
		{
			wxSplitterWindow* Splitter = SplitterWindows(x);
			wxSize WindowSize = Splitter->GetSize();
			FLOAT Scale, OldSize;
			if (Splitter->GetSplitMode() == wxSPLIT_HORIZONTAL)
			{
				Scale = ScaleY;
				OldSize = WindowSize.y;
			}
			else
			{
				Scale = ScaleX;
				OldSize = WindowSize.x;
			}
			FLOAT NewSize = FLOAT(OldSize) * Scale;
			FLOAT Proportion = FLOAT(Splitter->GetSashPosition()) / OldSize;
			INT Sash = appTrunc( Proportion * NewSize );
			Splitter->SetSashPosition( Sash, bRedraw == TRUE );
		}
	}
}

/**
 * Sets up this instance with the data from a template.
 */
void FViewportConfig_Data::SetTemplate( EViewportConfig InTemplate )
{
	Template = InTemplate;

	// Find the template

	FViewportConfig_Template* vct = NULL;
	for( INT x = 0 ; x < GApp->EditorFrame->ViewportConfigTemplates.Num() ; ++x )
	{
		FViewportConfig_Template* vctWk = GApp->EditorFrame->ViewportConfigTemplates(x);
		if( vctWk->ViewportConfig == InTemplate )
		{
			vct = vctWk;
			break;
		}
	}

	check( vct );	// If NULL, the template config type is unknown

	// Copy the templated data into our local vars

	*this = *vct;
}

/**
 * Updates custom data elements.
 */

void FViewportConfig_Data::Save()
{
	for( INT x = 0 ; x < GetViewportCount(); ++x )
	{
		FVCD_Viewport* viewport = &AccessViewport( x );

		if( viewport->bEnabled )
		{
			viewport->ViewportType = (ELevelViewportType)viewport->ViewportWindow->ViewportType;

			if(viewport->ViewportWindow->LastSpecialMode != 0)
			{
				viewport->ShowFlags = viewport->ViewportWindow->LastShowFlags;
			}
			else
			{
				viewport->ShowFlags = viewport->ViewportWindow->ShowFlags;
			}
		}
	}
}

/**
 * Loads the custom data elements from InData to this instance.
 *
 * @param	InData	The instance to load the data from.
 */

void FViewportConfig_Data::Load( FViewportConfig_Data* InData, UBOOL bTransferFloatingViewports )
{
	if( bTransferFloatingViewports )
{
		// We expect the destination floating viewport array to be empty at this point
		if( FloatingViewports.Num() == 0 )
		{
			// Transfer ownership of floating viewports
			FloatingViewports.Add( InData->FloatingViewports.Num() );
			for( INT FloatingViewportIndex = 0; FloatingViewportIndex < InData->FloatingViewports.Num(); ++FloatingViewportIndex )
			{
				FloatingViewports( FloatingViewportIndex ) = InData->FloatingViewports( FloatingViewportIndex );
			}

			// Kill the source pointer array so that it doesn't get cleaned up
			InData->FloatingViewports.Reset();
		}
	}


	for( INT x = 0 ; x < InData->GetViewportCount(); ++x )
	{
		const FVCD_Viewport* Src = &InData->GetViewport( x );

		if( Src->bEnabled )
		{
			// Find a matching viewport to copy the data into

			for( INT y = 0 ; y < GetViewportCount(); ++y )
			{
				FVCD_Viewport* Dst = &AccessViewport( y );

				if( Dst->bEnabled && Dst->ViewportType == Src->ViewportType )
				{
					Dst->ViewportType = Src->ViewportType;
					Dst->ShowFlags = Src->ShowFlags;
				}
			}
		}
	}
}

/**
 * Saves out the current viewport configuration to the editors INI file.
 */

void FViewportConfig_Data::SaveToINI()
{
	Save();

	GConfig->EmptySection( TEXT("ViewportConfig"), GEditorUserSettingsIni );
	GConfig->SetInt( TEXT("ViewportConfig"), TEXT("Template"), Template, GEditorUserSettingsIni );

	// Save MaximizedViewport setting
	GConfig->SetInt( TEXT( "ViewportConfig" ), TEXT( "MaximizedViewport" ), MaximizedViewport, GEditorUserSettingsIni );

	if ( !IsViewportMaximized() )
	{
		for( INT x = 0 ; x < SplitterWindows.Num() ; ++x )
		{
			FString Key = FString::Printf( TEXT("Splitter%d"), x );
			GConfig->SetInt( TEXT("ViewportConfig"), *Key, SplitterWindows(x)->GetSashPosition(), GEditorUserSettingsIni );
		}
	}

	// NOTE: Configuration of floating viewport windows is currently not loaded or saved to .ini files

	for( INT x = 0 ; x < 4 ; ++x )
	{
		FVCD_Viewport* viewport = &Viewports[x];

		FString Key = FString::Printf( TEXT("Viewport%d"), x );

		GConfig->SetBool( TEXT("ViewportConfig"), *(Key+TEXT("_Enabled")), viewport->bEnabled, GEditorUserSettingsIni );

		if( viewport->bEnabled )
		{
			GConfig->SetInt( TEXT("ViewportConfig"), *(Key+TEXT("_ViewportType")), viewport->ViewportType, GEditorUserSettingsIni );

			INT LowerFlags = viewport->ShowFlags & 0xFFFFFFFF;
			INT UpperFlags = viewport->ShowFlags >> 32;
			GConfig->SetInt( TEXT("ViewportConfig"), *(Key+TEXT("_ShowFlagsLower")), LowerFlags, GEditorUserSettingsIni );
			GConfig->SetInt( TEXT("ViewportConfig"), *(Key+TEXT("_ShowFlagsUpper")), UpperFlags, GEditorUserSettingsIni );
			GConfig->SetFloat(TEXT("ViewportConfig"), *(Key+TEXT("_CameraSpeed")), viewport->ViewportWindow->CameraSpeed, GEditorUserSettingsIni);
		}
	}
}

/**
 * Attempts to load the viewport configuration from the editors INI file.  If unsuccessful,
 * it returns 0 to the caller.
 */
UBOOL FViewportConfig_Data::LoadFromINI()
{
	INT Wk = VC_None;
	GConfig->GetInt( TEXT("ViewportConfig"), TEXT("Template"), Wk, GEditorUserSettingsIni );

	if( Wk == VC_None )
	{
		return 0;
	}

	Template = (EViewportConfig)Wk;
	GApp->EditorFrame->ViewportConfigData->SetTemplate( Template );

	// NOTE: Configuration of floating viewport windows is currently not loaded or saved to .ini files

	for( INT x = 0 ; x < 4 ; ++x )
	{
		FString Key = FString::Printf( TEXT("Viewport%d"), x );

		UBOOL bIsEnabled = FALSE;
		UBOOL bFoundViewport = GConfig->GetBool( TEXT("ViewportConfig"), *(Key+TEXT("_Enabled")), bIsEnabled, GEditorUserSettingsIni );

		if( bFoundViewport && bIsEnabled )
		{
			FVCD_Viewport* viewport = &Viewports[x];

			viewport->bEnabled = bIsEnabled;

			GConfig->GetInt( TEXT("ViewportConfig"), *(Key+TEXT("_ViewportType")), Wk, GEditorUserSettingsIni );
			viewport->ViewportType = (ELevelViewportType)Wk;

			DWORD UpperFlags, LowerFlags;
			GConfig->GetInt( TEXT("ViewportConfig"), *(Key+TEXT("_ShowFlagsUpper")), (INT&) UpperFlags, GEditorUserSettingsIni );
			GConfig->GetInt( TEXT("ViewportConfig"), *(Key+TEXT("_ShowFlagsLower")), (INT&) LowerFlags, GEditorUserSettingsIni );
			viewport->ShowFlags = ((QWORD)LowerFlags) | (((QWORD)UpperFlags) << 32);
			viewport->ShowFlags |= SHOW_ModeWidgets;

			// Remove postprocess on non-perspective viewports
			if(viewport->ShowFlags & SHOW_PostProcess && viewport->ViewportType != LVT_Perspective)
				viewport->ShowFlags &= ~SHOW_PostProcess;

			// Always clear the StreamingBounds flag at start-up
			viewport->ShowFlags &= ~SHOW_StreamingBounds;
		}
	}

	// No need to transfer floating viewports since we're not reinstantiating the object
	const UBOOL bTransferFloatingViewports = FALSE;
	GApp->EditorFrame->ViewportConfigData->Load( this, bTransferFloatingViewports );
	GApp->EditorFrame->ViewportConfigData->Apply( GApp->EditorFrame->ViewportContainer );

	// Load and apply MaximizedViewport setting
	INT ViewportToMaximize = -1;
	GConfig->GetInt( TEXT( "ViewportConfig" ), TEXT( "MaximizedViewport" ), ViewportToMaximize, GEditorUserSettingsIni );
	if( ViewportToMaximize >= 0 && ViewportToMaximize < 4 )
	{
		if( Viewports[ ViewportToMaximize ].bEnabled && MaximizedViewport != ViewportToMaximize )
		{
			ToggleMaximize( Viewports[ ViewportToMaximize ].ViewportWindow->Viewport );
			if( MaximizedViewport != -1 )
			{
				Viewports[ MaximizedViewport ].ViewportWindow->Invalidate();
			}
		}
	}

	return 1;
}

/**
 * Either resizes all viewports so that the specified Viewport is fills the entire editor window,
 * or restores all viewports to their previous sizes.
 * WxEditorFrame will lock all splitter drag bars when a viewport is maximized.
 * This function is called by clicking a button on the viewport toolbar.
 */
void FViewportConfig_Data::ToggleMaximize( FViewport* Viewport )
{
	for( INT x = 0; x < 4 && Viewports[x].ViewportWindow; ++x )
	{
		if ( Viewport == Viewports[x].ViewportWindow->Viewport )
		{
			// Already maximized?
			if ( MaximizedViewport == x )
			{
				// Restore all viewports:

				// Restore all sash positions:
				for( INT n = 0; n < SplitterWindows.Num(); ++n )
				{
					INT SashPos;
					FString Key = FString::Printf( TEXT("Splitter%d"), n );
					if ( GConfig->GetInt( TEXT("ViewportConfig"), *Key, SashPos, GEditorUserSettingsIni ) )
					{
						SplitterWindows(n)->SetSashPosition( SashPos );
					}
				}
				MaximizedViewport = -1;
			}
			else
			{
				// Maximize this viewport:

				// Save sash positions if no other viewport was maximized (should never happen, but anyway)
				if ( MaximizedViewport < 0 )
				{
					for( INT n = 0; n < SplitterWindows.Num(); ++n )
					{
						FString Key = FString::Printf( TEXT("Splitter%d"), n);
						INT SashPos = SplitterWindows(n)->GetSashPosition();
						GConfig->SetInt( TEXT("ViewportConfig"), *Key, SashPos, GEditorUserSettingsIni );
					}
				}

				MaximizedViewport = x;
				Layout();
			}
			break;
		}
	}
}

/**
 * Lets the ViewportConfig layout the splitter windows.
 * If a viewport is maximized, set up all sash positions so that the maximized viewport covers the entire window.
 */
void FViewportConfig_Data::Layout()
{
	if ( IsViewportMaximized() )
	{
		INT TreePath[3] = {0,0,0};	// Maximum 3 splitters
		check( SplitterWindows.Num() <= 3 );
		
		wxWindow* ContainedWindow = Viewports[MaximizedViewport].ViewportWindow;

		// If we have a PIE container window, then we should always be looking for that window
		if( Viewports[MaximizedViewport].PIEContainerWindow != NULL )
		{
			ContainedWindow = Viewports[MaximizedViewport].PIEContainerWindow;
		}

		INT WhichWindow;

		INT SplitterIndex = FindSplitter( ContainedWindow, &WhichWindow );
		while( SplitterIndex >= 0 )
		{
			TreePath[SplitterIndex] = WhichWindow;
			ContainedWindow = SplitterWindows(SplitterIndex);
			SplitterIndex = FindSplitter( ContainedWindow, &WhichWindow );
		}
		for( INT n=0; n < SplitterWindows.Num(); ++n )
		{
			wxSplitterWindow* Splitter = SplitterWindows(n);
			wxSize Size = Splitter->GetClientSize();
			INT MaxPos = (Splitter->GetSplitMode() == wxSPLIT_HORIZONTAL) ? Size.y : Size.x;
			if ( TreePath[n] == 1 )
			{
				Splitter->SetSashPosition(MaxPos, FALSE);
			}
			else if ( TreePath[n] == 2 )
			{
				Splitter->SetSashPosition(-MaxPos, FALSE);
			}
			else
			{
				Splitter->SetSashPosition(0, FALSE);
			}
			Splitter->UpdateSize();
		}
	}
}

/**
 * Returns TRUE if a viewport is maximized, otherwise FALSE.
 */
UBOOL FViewportConfig_Data::IsViewportMaximized()
{
	return (MaximizedViewport >= 0);
}

/**
 * Finds which SplitterWindow contains the specified window.
 * Returns the index in the SplitterWindow array, or -1 if not found.
 * It also returns which window it was:
 *    WindowID == 1 if it was the first window (GetWindow1)
 *    WindowID == 2 if it was the second window (GetWindow2)
 * WindowID may be NULL.
 */
INT FViewportConfig_Data::FindSplitter( wxWindow* ContainedWindow, INT *WhichWindow/*=NULL*/ )
{
	// First, find the ViewportWindow:
	INT i;
	wxSplitterWindow* Splitter;
	INT Found=0;
	for( i=0; i < SplitterWindows.Num(); ++i )
	{
		Splitter = SplitterWindows(i);
		if ( ContainedWindow == Splitter->GetWindow1() )
		{
			Found = 1;
			break;
		}
		if ( ContainedWindow == Splitter->GetWindow2() )
		{
			Found = 2;
			break;
		}
	}
	if ( !Found )
	{
		return -1;
	}
	if ( WhichWindow )
	{
		*WhichWindow = Found;
	}
	return i;
}



/**
 * Opens a new floating viewport window
 *
 * @param ParentWxWindow The parent window
 * @param InViewportType Type of viewport window
 * @param InShowFlags Show flags for the new window
 * @param InWidth Width of the window
 * @param InHeight Height of the window
 * @param OutViewportIndex [out] Index of newly created viewport
 *
 * @return Returns TRUE if everything went OK
 */
UBOOL FViewportConfig_Data::OpenNewFloatingViewport( wxWindow* ParentWxWindow,
													 const ELevelViewportType InViewportType,
													 const EShowFlags InShowFlags,
													 const INT InWidth,
													 const INT InHeight,
													 INT& OutViewportIndex )
{
	// This will be filled in later if everything goes OK
	OutViewportIndex = INDEX_NONE;

	// Floating viewports will never default to being listeners
	const UBOOL bSetListenerPosition = FALSE;

	// Setup window size
	wxSize WindowSize( InWidth, InHeight );
	if( WindowSize.GetX() <= 0 || WindowSize.GetY() <= 0 )
	{
		WindowSize = wxSize( 1280, 720 );
	}

	// Create a container window for the floating viewport
	WxFloatingViewportFrame* NewViewportFrame = NULL;
	{
		// Start off with a default frame style (caption, resizable, minimize, maximize, close, system menu, child clip)
		INT ContainerWindowWxStyle = wxDEFAULT_FRAME_STYLE;

		// We don't want the window to appear in the task bar on platforms that support that
		ContainerWindowWxStyle |= wxFRAME_NO_TASKBAR;

		// We never want the window to be hidden by the main UnrealEd window
		ContainerWindowWxStyle |= wxFRAME_FLOAT_ON_PARENT;

		// Setup window title
		FString WindowTitle = FString::Printf( LocalizeSecure( LocalizeUnrealEd( "FloatingViewportWindowTitle_F" ), FloatingViewports.Num() ) );

		// Create the viewport frame!
		NewViewportFrame = new WxFloatingViewportFrame( ParentWxWindow, -1, WindowTitle, wxDefaultPosition, WindowSize, ContainerWindowWxStyle );
		check( NewViewportFrame != NULL );
	}


	// Create and initialize viewport window
	WxLevelViewportWindow* NewLevelViewportWindow = new WxLevelViewportWindow();
	check( NewLevelViewportWindow != NULL );
	{
		if( !NewLevelViewportWindow->Create( NewViewportFrame, -1, wxDefaultPosition, WindowSize, 0 ) )
		{
			NewLevelViewportWindow->Destroy();
			NewViewportFrame->Destroy();
			return FALSE;
		}
		
		// Let the viewport window know that it's going to be floating.  Must be called before we call SetUp!
		NewLevelViewportWindow->SetFloatingViewport( TRUE );

		NewLevelViewportWindow->SetLabel( wxT( "FloatingLevelViewport" ) );
		NewLevelViewportWindow->SetUp( InViewportType, bSetListenerPosition, InShowFlags );
	}


	// Create and initialize viewport window container
	FVCD_Viewport* NewViewport = new FVCD_Viewport();
	check( NewViewport != NULL );
	{
		NewViewport->bEnabled = TRUE;
		NewViewport->ViewportType = NewLevelViewportWindow->ViewportType = InViewportType;
		NewViewport->ShowFlags = NewLevelViewportWindow->ShowFlags = InShowFlags;
		NewViewport->bSetListenerPosition = NewLevelViewportWindow->bSetListenerPosition = bSetListenerPosition;
		NewViewport->ViewportWindow = NewLevelViewportWindow;
		NewViewport->FloatingViewportFrame = NewViewportFrame;
	}


	if( NewLevelViewportWindow->ViewportType == LVT_Perspective )
	{
		// Only perspective windows will have Matinee preview features turned on by default
		NewLevelViewportWindow->SetAllowMatineePreview( TRUE );

		// Assign default camera location/rotation for perspective camera
		NewLevelViewportWindow->ViewLocation = EditorViewportDefs::DefaultPerspectiveViewLocation;
		NewLevelViewportWindow->ViewRotation = EditorViewportDefs::DefaultPerspectiveViewRotation;
	}

	// @todo: Support loading/saving floating viewport camera speed pref?
	// 	FString Key = FString::Printf( TEXT("Viewport%d"), x );
	// 	GConfig->GetFloat( TEXT("ViewportConfig"), *(Key+TEXT("_CameraSpeed")), Viewports[x].ViewportWindow->CameraSpeed, GEditorUserSettingsIni );


	// Add new viewport to floating viewport list
	OutViewportIndex = 4 + FloatingViewports.Num();
	FloatingViewports.AddItem( NewViewport );

	// Assign callback interface to viewport frame so we'll find out about OnClose events
	NewViewportFrame->SetCallbackInterface( this );

	// OK, now show the window!
	NewViewportFrame->Show( TRUE );


	return TRUE;
}



void FViewportConfig_Data::OnFloatingViewportClosed( WxFloatingViewportFrame* InViewportFrame )
{
	// Clean up floating viewports
	for( INT FloatingViewportIndex = 0; FloatingViewportIndex < FloatingViewports.Num(); ++FloatingViewportIndex )
	{
		FVCD_Viewport* FloatingViewport = FloatingViewports( FloatingViewportIndex );
		if( FloatingViewport != NULL )
		{
			if( FloatingViewport->FloatingViewportFrame == InViewportFrame )
			{
				// OK, this viewport is being destroyed

				// NOTE: The viewport windows will take care of destroying themselves after this function is called
				FloatingViewport->FloatingViewportFrame = NULL;
				FloatingViewport->ViewportWindow = NULL;

				// Kill our wrapper object
				delete FloatingViewport;

				// Remove from the list
				FloatingViewports.Remove( FloatingViewportIndex );
				break;
			}
		}
	}
}



/*-----------------------------------------------------------------------------
	WxLevelViewportWindow.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxLevelViewportWindow, wxWindow )
	EVT_SIZE( WxLevelViewportWindow::OnSize )
	EVT_SET_FOCUS( WxLevelViewportWindow::OnSetFocus )
	EVT_MENU( ID_CAMSPEED_SLOW, WxLevelViewportWindow::OnCameraSlow )
	EVT_MENU( ID_CAMSPEED_NORMAL, WxLevelViewportWindow::OnCameraNormal )
	EVT_MENU( ID_CAMSPEED_FAST, WxLevelViewportWindow::OnCameraFast )
	EVT_UPDATE_UI( ID_CAMSPEED_SLOW, WxLevelViewportWindow::OnCameraSlowUIUpdate )
	EVT_UPDATE_UI( ID_CAMSPEED_NORMAL, WxLevelViewportWindow::OnCameraNormalUIUpdate )
	EVT_UPDATE_UI( ID_CAMSPEED_FAST, WxLevelViewportWindow::OnCameraFastUIUpdate )
END_EVENT_TABLE()

WxLevelViewportWindow::WxLevelViewportWindow()
	: FEditorLevelViewportClient()
{
	bAllowAmbientOcclusion = TRUE;
	ToolBar = NULL;
	Viewport = NULL;
	bVariableFarPlane = TRUE;
	bDrawVertices = TRUE;
}

WxLevelViewportWindow::~WxLevelViewportWindow()
{
	GEngine->Client->CloseViewport(Viewport);
	Viewport = NULL;
}

void WxLevelViewportWindow::SetUp( ELevelViewportType InViewportType, UBOOL bInSetListenerPosition, EShowFlags InShowFlags )
{
	// Set viewport parameters first.  These may be queried by the level viewport toolbar, etc.
	ViewportType = InViewportType;
	bSetListenerPosition = bInSetListenerPosition;
	ShowFlags = InShowFlags;
	LastShowFlags = InShowFlags;

	// Create viewport
	Viewport = GEngine->Client->CreateWindowChildViewport( (FViewportClient*)this, (HWND)GetHandle() );
	if( Viewport != NULL )
	{
		Viewport->CaptureJoystickInput(false);
		::SetWindowText( (HWND)Viewport->GetWindow(), TEXT("Viewport") );
	}

	// ToolBar
	ToolBar = new WxLevelViewportToolBar( this, -1, this );
	ToolBar->SetLabel( wxT("ToolBar") );

	wxSizeEvent DummyEvent;
	OnSize( DummyEvent );

	ToolBar->UpdateUI();
}

UBOOL WxLevelViewportWindow::InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime)
{
	return FEditorLevelViewportClient::InputAxis( Viewport, ControllerId, Key, Delta, DeltaTime );
}

void WxLevelViewportWindow::OnSize( wxSizeEvent& InEvent )
{
	if( ToolBar )
	{
		wxRect rc = GetClientRect();
		rc.y += WxLevelViewportToolBar::GetToolbarHeight();
		rc.height -= WxLevelViewportToolBar::GetToolbarHeight();

		ToolBar->SetSize( rc.GetWidth(), WxLevelViewportToolBar::GetToolbarHeight() );
		::SetWindowPos( (HWND)Viewport->GetWindow(), HWND_TOP, rc.GetLeft()+1, rc.GetTop()+1, rc.GetWidth()-2, rc.GetHeight()-2, SWP_SHOWWINDOW );
	}
	InEvent.Skip();
}

void WxLevelViewportWindow::OnSetFocus(wxFocusEvent& In)
{
	if ( Viewport )
	{
		::SetFocus( (HWND) Viewport->GetWindow() );
	}
}

void WxLevelViewportWindow::OnCameraSlow(wxCommandEvent& Event)
{
	CameraSpeed = MOVEMENTSPEED_SLOW;
}

void WxLevelViewportWindow::OnCameraNormal(wxCommandEvent& Event)
{
	CameraSpeed = MOVEMENTSPEED_NORMAL;
}

void WxLevelViewportWindow::OnCameraFast(wxCommandEvent& Event)
{
	CameraSpeed = MOVEMENTSPEED_FAST;
}

void WxLevelViewportWindow::OnCameraSlowUIUpdate(wxUpdateUIEvent& Event)
{
	Event.Check(CameraSpeed == MOVEMENTSPEED_SLOW);
}

void WxLevelViewportWindow::OnCameraNormalUIUpdate(wxUpdateUIEvent& Event)
{
	Event.Check(CameraSpeed == MOVEMENTSPEED_NORMAL);
}

void WxLevelViewportWindow::OnCameraFastUIUpdate(wxUpdateUIEvent& Event)
{
	Event.Check(CameraSpeed == MOVEMENTSPEED_FAST);
}



/*-----------------------------------------------------------------------------
	WxFloatingViewportFrame
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxFloatingViewportFrame, WxTrackableFrame )
	EVT_CLOSE( WxFloatingViewportFrame::OnClose )
	EVT_SIZE( WxFloatingViewportFrame::OnSize )
END_EVENT_TABLE()


WxFloatingViewportFrame::WxFloatingViewportFrame( wxWindow* InParent, wxWindowID InID, const FString& InTitle, const wxPoint& pos, const wxSize& size, long style )
	: WxTrackableFrame( InParent, InID, *InTitle, pos, size, style ),
	  CallbackInterface( NULL )
{
}



WxFloatingViewportFrame::~WxFloatingViewportFrame()
{
	CallbackInterface = NULL;
}



/** Sets the callback interface for this viewport */
void WxFloatingViewportFrame::SetCallbackInterface( IFloatingViewportCallback* NewCallbackInterface )
{
	CallbackInterface = NewCallbackInterface;
}



/** Called when the window is closed */
void WxFloatingViewportFrame::OnClose( wxCloseEvent& InEvent )
{
	if( CallbackInterface != NULL )
	{
		CallbackInterface->OnFloatingViewportClosed( this );
	}

	// Kill the window's children, and the window itself
	DestroyChildren();
	Destroy();
}



/** Called when the window is resized */
void WxFloatingViewportFrame::OnSize( wxSizeEvent& InEvent )
{
	// Update size of child windows
	for( UINT CurChildWindowIndex = 0; CurChildWindowIndex < GetChildren().size(); ++CurChildWindowIndex )
	{
		wxWindow* CurChildWindow = GetChildren()[ CurChildWindowIndex ];
		CurChildWindow->SetSize( GetClientRect().GetSize() );
	}
	InEvent.Skip();
}



/** This function is called when the WxTrackableDialog has been selected from within the ctrl + tab dialog. */
void WxFloatingViewportFrame::OnSelected()
{
	// This is kind of weird right here.  Ideally, WxTrackableFrame would be doing this instead of our derived class
	// (and similar derived classes), just like WxTrackableDialog does.  However, it seems that would introduce 
	// window focus problems with very specific frame windows, so we do it here in our derived class.
	Raise();
}



/*-----------------------------------------------------------------------------
	WxViewportHolder
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxViewportHolder, wxPanel )
	EVT_SIZE(WxViewportHolder::OnSize)
END_EVENT_TABLE()

WxViewportHolder::WxViewportHolder( wxWindow* InParent, wxWindowID InID, bool InWantScrollBar, const wxPoint& pos, const wxSize& size, long style)
	: wxPanel( InParent, InID, pos, size, style ), bAutoDestroy(FALSE)
{
	Viewport = NULL;
	ScrollBar = NULL;
	SBPos = SBRange = 0;

	if( InWantScrollBar )
		ScrollBar = new wxScrollBar( this, ID_BROWSER_SCROLL_BAR, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL );
}

WxViewportHolder::~WxViewportHolder()
{
	if ( bAutoDestroy )
	{
		GEngine->Client->CloseViewport(Viewport);
		SetViewport(NULL);
	}
}

void WxViewportHolder::SetViewport( FViewport* InViewport )
{
	Viewport = InViewport;
}

void WxViewportHolder::OnSize( wxSizeEvent& InEvent )
{
	if( Viewport )
	{
		wxRect rc = GetClientRect();
		wxRect rcSB;
		if( ScrollBar )
			rcSB = ScrollBar->GetClientRect();

		SetWindowPos( (HWND)Viewport->GetWindow(), HWND_TOP, rc.GetLeft(), rc.GetTop(), rc.GetWidth()-rcSB.GetWidth(), rc.GetHeight(), SWP_SHOWWINDOW );

		if( ScrollBar )
			ScrollBar->SetSize( rc.GetLeft()+rc.GetWidth()-rcSB.GetWidth(), rc.GetTop(), rcSB.GetWidth(), rc.GetHeight() );
	}

	InEvent.Skip();
}

// Updates the scrollbar so it looks right and is in the right position

void WxViewportHolder::UpdateScrollBar( INT InPos, INT InRange )
{
	if( ScrollBar )
		ScrollBar->SetScrollbar( InPos, Viewport->GetSizeY(), InRange, Viewport->GetSizeY() );
}

INT WxViewportHolder::GetScrollThumbPos()
{
	return ( ScrollBar ? ScrollBar->GetThumbPosition() : 0 );
}

void WxViewportHolder::SetScrollThumbPos( INT InPos )
{
	if( ScrollBar )
		ScrollBar->SetThumbPosition( InPos );
}




/*-----------------------------------------------------------------------------
	WxPIEContainerWindow
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxPIEContainerWindow, wxPanel )
	EVT_SIZE( WxPIEContainerWindow::OnSize )
END_EVENT_TABLE()


/** Constructor.  Should only be called from within CreatePIEContainerWindow() */
WxPIEContainerWindow::WxPIEContainerWindow( wxWindow* InParent, wxWindowID InID, const wxPoint& pos, const wxSize& size, long style, const FString& InTitle )
	: wxPanel( InParent, InID, pos, size, style, *InTitle ),
	  Viewport( NULL ),
	  bIsEmbeddedInFloatingWindow( FALSE )
{
	GCallbackEvent->Register( CALLBACK_EndPIE, this );
}



/** Destructor */
WxPIEContainerWindow::~WxPIEContainerWindow()
{
	GCallbackEvent->UnregisterAll( this );

	UBOOL bWasEmbeddedInFloatingWindow  = bIsEmbeddedInFloatingWindow;

	// Make sure everything's been cleaned up
	ClosePIEWindowAndRestoreViewport();

	// Make sure viewport client gets a chance to clean up if the user closed the entire floating window
	if( bWasEmbeddedInFloatingWindow )
	{
		if( Viewport != NULL && Viewport->GetClient() != NULL )
		{
			Viewport->GetClient()->CloseRequested( Viewport );
		}
	}

	// NOTE: We don't need to destroy our Viewport object here since either the window itself, because
	//   the viewport client will take care of that for us.
	Viewport = NULL;
}



/**
 * Creates a Play In Editor viewport window and embeds it into a level editor viewport (if possible)
 *
 * NOTE: This is a static method
 *
 * @param ViewportClient The viewport client the new viewport will be associated with
 * @param TargetViewport The viewport window to possess
 *
 * @return Newly created WxPIEContainerWindow if successful, otherwise NULL
 */
WxPIEContainerWindow* WxPIEContainerWindow::CreatePIEWindowAndPossessViewport( UGameViewportClient* ViewportClient,
																			   FVCD_Viewport* TargetViewport )
{
	// Viewport should never already have a PIE container at this point
	check( TargetViewport->PIEContainerWindow == NULL );

	// Try to create an embedded PIE viewport
	if( TargetViewport->ViewportWindow != NULL )
	{
		// Is this a floating viewport?  If so, we need to handle it differently
		if( TargetViewport->FloatingViewportFrame != NULL )
		{
			// Hide actual viewport window
			TargetViewport->ViewportWindow->Hide();

			// Create and initialize container window
			TargetViewport->PIEContainerWindow =
				new WxPIEContainerWindow(
					TargetViewport->FloatingViewportFrame,
					-1,
					wxPoint( 0, 0 ),
					wxSize( TargetViewport->FloatingViewportFrame->GetClientRect().GetSize() ) );

			// NOTE: We call Initialize after ReplaceWindow so that the size of this window will be correct
			//   before creating the child window
			if( TargetViewport->PIEContainerWindow->Initialize( ViewportClient ) )
			{
				TargetViewport->PIEContainerWindow->bIsEmbeddedInFloatingWindow = TRUE;

				// Done!
				return TargetViewport->PIEContainerWindow;
			}
			else
			{
				// Undo our changes so far
				TargetViewport->PIEContainerWindow->Destroy();
				TargetViewport->PIEContainerWindow = NULL;

				// Restore the viewport window
				TargetViewport->ViewportWindow->Show();
			}
		}
		else
		{
			// Figure out which splitter window is the parent of the current viewport
			INT SplitterIndex = GApp->EditorFrame->ViewportConfigData->FindSplitter( TargetViewport->ViewportWindow );
			if( SplitterIndex >= 0 )
			{
				wxSplitterWindow* ParentSplitterWindow =
					GApp->EditorFrame->ViewportConfigData->SplitterWindows( SplitterIndex );

				// Hide actual viewport window
				TargetViewport->ViewportWindow->Hide();

				// Create and initialize container window
				TargetViewport->PIEContainerWindow = new WxPIEContainerWindow( ParentSplitterWindow, -1 );

				// Replace the viewport window associated with the splitter with the new PIE container window
				ParentSplitterWindow->ReplaceWindow( TargetViewport->ViewportWindow, TargetViewport->PIEContainerWindow );

				// NOTE: We call Initialize after ReplaceWindow so that the size of this window will be correct
				//   before creating the child window
				if( TargetViewport->PIEContainerWindow->Initialize( ViewportClient ) )
				{
					// Done!
					return TargetViewport->PIEContainerWindow;
				}
				else
				{
					// Undo our changes so far
					ParentSplitterWindow->ReplaceWindow( TargetViewport->PIEContainerWindow, TargetViewport->ViewportWindow );

					TargetViewport->PIEContainerWindow->Destroy();
					TargetViewport->PIEContainerWindow = NULL;

					// Restore the viewport window
					TargetViewport->ViewportWindow->Show();
				}
			}
		}
	}

	// Failed
	return NULL;
}



UBOOL WxPIEContainerWindow::Initialize( UGameViewportClient* ViewportClient )
{
	SetLabel( TEXT( "PlayInEditorContainerWindow" ) );

	// Now create a child viewport for this window
	Viewport = GEngine->Client->CreateWindowChildViewport(
		ViewportClient,
		( HWND )GetHandle(),
		GetClientRect().width,
		GetClientRect().height,
		0,
		0 );
	if( Viewport != NULL )
	{
		::SetWindowText( ( HWND )Viewport->GetWindow(), TEXT( "PlayInEditorViewport" ) );

		// Update the viewport client's window pointer
		ViewportClient->SetViewport( Viewport );

		// Set focus to the PIE viewport
		::ShowWindow( ( HWND )Viewport->GetWindow(), SW_SHOW );
		::SetFocus( ( HWND )Viewport->GetWindow() );

		// Done!
		return TRUE;
	}

	return FALSE;
}



/** Closes the the Play-In-Editor window and restores any previous viewport */
void WxPIEContainerWindow::ClosePIEWindowAndRestoreViewport()
{
	// Find the window for this viewport client
	FVCD_Viewport* ViewportWithPIE = NULL;
	{
		const UINT NumViewports = GApp->EditorFrame->ViewportConfigData->GetViewportCount();
		for( UINT CurViewportIndex = 0; CurViewportIndex < NumViewports; ++CurViewportIndex )
		{
			FVCD_Viewport* CurrentViewport = &GApp->EditorFrame->ViewportConfigData->AccessViewport( CurViewportIndex );
			if( CurrentViewport->PIEContainerWindow == this )
			{
				ViewportWithPIE = CurrentViewport;
				break;
			}
		}
	}

	if( ViewportWithPIE != NULL )
	{
		// We'll only need to restore the original viewport if one actually exists
		if( ViewportWithPIE->ViewportWindow != NULL )
		{
			// Is this a floating viewport?  Those are handled a bit differently
			if( ViewportWithPIE->FloatingViewportFrame != NULL )
			{
				// Show the original viewport window that we hid before launching PIE
				ViewportWithPIE->ViewportWindow->Show();
			}
			else
			{
				// Find the splitter window that's the parent of the PIE container window
				INT SplitterIndex = GApp->EditorFrame->ViewportConfigData->FindSplitter( ViewportWithPIE->PIEContainerWindow );
				if( SplitterIndex >= 0 )
				{
					wxSplitterWindow* ParentSplitterWindow =
						GApp->EditorFrame->ViewportConfigData->SplitterWindows( SplitterIndex );

					// Show the original viewport window that we hid before launching PIE
					ViewportWithPIE->ViewportWindow->Show();

					// Replace the PIE container window associated with the splitter with the original viewport
					ParentSplitterWindow->ReplaceWindow( ViewportWithPIE->PIEContainerWindow, ViewportWithPIE->ViewportWindow );
				}
			}
		}

		// Detach from the editor viewport completely
		ViewportWithPIE->PIEContainerWindow = NULL;
	}

	// No longer embedded
	bIsEmbeddedInFloatingWindow = FALSE;
}



void WxPIEContainerWindow::OnSize( wxSizeEvent& InEvent )
{
	if( Viewport != NULL )
	{
		wxRect rc = GetClientRect();

		// Update the viewport window's size
		::SetWindowPos( ( HWND )Viewport->GetWindow(), HWND_TOP, 0, 0, rc.GetWidth(), rc.GetHeight(), SWP_SHOWWINDOW );
	}

	InEvent.Skip();
}



/** Called from the global event handler when a registered event is fired */
void WxPIEContainerWindow::Send( ECallbackEventType InType )
{
	if( InType == CALLBACK_EndPIE )
	{
		// Close the Play-In-Editor window and restore a previous viewport if needed
		// NOTE: This will call Destroy on the window!
		ClosePIEWindowAndRestoreViewport();

		// Also, kill self
		Destroy();
	}
}
