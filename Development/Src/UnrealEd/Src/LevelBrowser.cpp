/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "LevelBrowser.h"
#include "EngineSequenceClasses.h"
#include "LevelUtils.h"
#include "SceneManager.h"
#include "FileHelpers.h"
#include "BusyCursor.h"
#include "ScopedTransaction.h"
#include "Kismet.h"
#include "Properties.h"
#include "DlgGenericComboEntry.h"

/** Implements FString sorting for LevelBrowser.cpp */
IMPLEMENT_COMPARE_CONSTREF( FString, LevelBrowser, { return appStricmp(*A,*B); } );

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WxLockBitmapsWindow
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class WxLockBitmapsWindow : public wxWindow
{
public:
	WxLockBitmapsWindow(wxWindow* InParent)
		:	wxWindow( InParent, -1, wxDefaultPosition, wxDefaultSize )
	{
		if ( !bBitmapsLoaded )
		{
			bBitmapsLoaded = TRUE;
			const UBOOL bLockBitmapLoaded = LockBitmap.Load( TEXT("LockLB") );
			check( bLockBitmapLoaded );
			const UBOOL bUnlockBitmapLoaded = UnlockBitmap.Load( TEXT("UnlockLB") );
			check( bUnlockBitmapLoaded );
		}
	}

protected:
	void DrawCurLockBitmap(wxDC& InDC, const wxRect& InRect)
	{
		InDC.DrawBitmap( *CurLockBitmap, InRect.x + BitmapOffsetX, InRect.y + BitmapOffsetY, 1 );
	}

	void SetCurLockBitmap(UBOOL bLocked)
	{
		if ( bLocked )
		{
			CurLockBitmap = &LockBitmap;
		}
		else
		{
			CurLockBitmap = &UnlockBitmap;
		}
	}

	UBOOL ClickedOnLock(wxMouseEvent& In) const
	{
		const int w = LockBitmap.GetWidth();
		const int h = LockBitmap.GetHeight();
		const wxRect BitmapRect( BitmapOffsetX, BitmapOffsetY, w, h );

		return BitmapRect.Contains( In.GetPosition() ) ? TRUE : FALSE;
	}

private:
	WxMaskedBitmap* CurLockBitmap;

	static const INT		BitmapOffsetX;
	static const INT		BitmapOffsetY;
	static WxMaskedBitmap	LockBitmap;
	static WxMaskedBitmap	UnlockBitmap;
	static UBOOL			bBitmapsLoaded;
};

const INT WxLockBitmapsWindow::BitmapOffsetX		= 3;
const INT WxLockBitmapsWindow::BitmapOffsetY		= 3;
WxMaskedBitmap WxLockBitmapsWindow::LockBitmap;
WxMaskedBitmap WxLockBitmapsWindow::UnlockBitmap;
UBOOL WxLockBitmapsWindow::bBitmapsLoaded			= FALSE;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WxLevelBrowserMenuBar
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class WxLevelBrowserMenuBar : public wxMenuBar
{
public:
	WxLevelBrowserMenuBar()
	{
		// Level menu.
		wxMenu* LevelMenu = new wxMenu();
		LevelMenu->Append( IDM_LB_NewLevel, *LocalizeUnrealEd("NewLevel"), TEXT("") );
		LevelMenu->Append( IDM_LB_NewLevelFromSelectedActors, *LocalizeUnrealEd("NewLevelFromSelectedActors"), TEXT("") );
		LevelMenu->Append( IDM_LB_ImportLevel, *LocalizeUnrealEd("ImportLevelE"), TEXT("") );
		LevelMenu->AppendSeparator();
		LevelMenu->Append( IDM_LB_MakeCurrentLevel, *LocalizeUnrealEd("MakeCurrent"), TEXT("") );
		LevelMenu->AppendSeparator();
		LevelMenu->Append( IDM_LB_MergeVisibleLevels, *LocalizeUnrealEd("MergeVisibleLevels"), TEXT("") );
		LevelMenu->AppendSeparator();
		LevelMenu->Append( IDM_LB_RemoveLevelFromWorld, *LocalizeUnrealEd("RemoveLevelFromWorld"), TEXT("") );

		// View menu.
		wxMenu* ViewMenu = new wxMenu();
		ViewMenu->Append( IDM_RefreshBrowser, *LocalizeUnrealEd("RefreshWithHotkey"), TEXT("") );

		Append( LevelMenu, *LocalizeUnrealEd("Level") );
		Append( ViewMenu, *LocalizeUnrealEd("View") );

		WxBrowser::AddDockingMenu( this );
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WxScrollablePane
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class WxScrollablePane : public wxScrolledWindow
{
public:
	WxScrollablePane(wxWindow* InParent, WxLevelBrowser* InLevelBrowser)
		:	wxScrolledWindow( InParent, -1, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL )
		,	LevelBrowser( InLevelBrowser )
	{
		SetScrollbars(1,1,0,0);
		// Set the x and y scrolling increments.  This is necessary for the scrolling to work!
		SetScrollRate( 1, 1 );
	}

protected:
	/**
	 * Does the layout for the pane's child windows.
	 */
	template<class ChildWindowType>
	void LayoutWindows(TArray<ChildWindowType*>& ChildWindows, INT ChildWindowHeight)
	{
		Freeze();

		const wxRect rc( GetClientRect() );
		const int ScrollPos = GetScrollPos( wxVERTICAL );
		INT Top = -ScrollPos;

		for ( INT ChildWindowIndex = 0 ; ChildWindowIndex < ChildWindows.Num() ; ++ChildWindowIndex )
		{
			ChildWindowType* CurChildWindow = ChildWindows( ChildWindowIndex );
			CurChildWindow->SetSize( 0, Top, rc.GetWidth(), ChildWindowHeight );
			Top += ChildWindowHeight;
		}

		// Tell the scrolling window how large the virtual area is.
		SetVirtualSize( rc.GetWidth(), ChildWindows.Num() * ChildWindowHeight );

		Thaw();
	}

	/** Handle to the level browser that owns this level pane. */
	WxLevelBrowser*			LevelBrowser;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WxLevelPane
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace LevelBrowser {

/**
 * Deselects all BSP surfaces in the specified level.
 *
 * @param	Level		The level for which to deslect all levels.
 */
static void LevelBrowser_DeselectAllSurfacesForLevel(ULevel* Level)
{
	UModel* Model = Level->Model;
	for( INT SurfaceIndex = 0 ; SurfaceIndex < Model->Surfs.Num() ; ++SurfaceIndex )
	{
		FBspSurf& Surf = Model->Surfs(SurfaceIndex);
		if( Surf.PolyFlags & PF_Selected )
		{
			Model->ModifySurf( SurfaceIndex, FALSE );
			Surf.PolyFlags &= ~PF_Selected;
		}
	}
}

/**
 * Encapsulates calls to setting level visibility so that selected actors
 * in levels that are being hidden can be deselected.
 */
static void SetLevelVisibility(ULevel* Level, UBOOL bVisible)
{
	FLevelUtils::SetLevelVisibility( Level, bVisible, TRUE );

	// If the level is being hidden, deselect actors and surfaces that belong to this level.
	if ( !bVisible )
	{
		USelection* SelectedActors = GEditor->GetSelectedActors();
		SelectedActors->Modify();
		TTransArray<AActor*>& Actors = Level->Actors;
		for ( INT ActorIndex = 0 ; ActorIndex < Actors.Num() ; ++ActorIndex )
		{
			AActor* Actor = Actors( ActorIndex );
			if ( Actor )
			{
				SelectedActors->Deselect( Actor );
			}
		}
		LevelBrowser_DeselectAllSurfacesForLevel( Level );

		// Tell the editor selection status was changed.
		GEditor->NoteSelectionChange();
	}
}

/**
 * Encapsulates calls to setting level visibility so that selected actors
 * in levels that are being hidden can be deselected.
 */
static void SetLevelVisibility(ULevelStreaming* StreamingLevel, UBOOL bVisible)
{
	FLevelUtils::SetLevelVisibility( StreamingLevel, bVisible, TRUE );

	// If the level is being hidden, deselect actors and surfaces that belong to this level.
	if ( !bVisible )
	{
		ULevel *Level = StreamingLevel->LoadedLevel;
		if ( Level )
		{
			USelection* SelectedActors = GEditor->GetSelectedActors();
			SelectedActors->Modify();
			TTransArray<AActor*>& Actors = Level->Actors;
			for ( INT ActorIndex = 0 ; ActorIndex < Actors.Num() ; ++ActorIndex )
			{
				AActor* Actor = Actors( ActorIndex );
				if ( Actor )
				{
					SelectedActors->Deselect( Actor );
				}
			}
			LevelBrowser_DeselectAllSurfacesForLevel( Level );

			// Tell the editor selection status was changed.
			GEditor->NoteSelectionChange();
		}
	}
}

} // namespace LevelBrowser

/**
 * The panel on the left side of the level browser that contains a scrollable
 * list of levels in the world.
 */
class WxLevelPane : public WxScrollablePane
{
public:
	class WxLevelWindow : public WxLockBitmapsWindow
	{
	public:
		WxLevelWindow(wxWindow* InParent, WxLevelBrowser* InLevelBrowser, ULevel* InLevel, const TCHAR* InDescription);

		ULevel* GetLevel()				{ return Level; }
		const ULevel* GetLevel() const	{ return Level; }

		/**
		 * Updates UI elements from the level managed by this level window.  Refreshes.
		 */
		void UpdateUI()
		{
			if ( VisibleCheck )
			{
				const UBOOL bLevelVisible = FLevelUtils::IsLevelVisible( Level );
				VisibleCheck->SetValue( bLevelVisible == TRUE );
			}
			SetCurLockBitmap( FLevelUtils::IsLevelLocked( Level ) );
			//LevelBrowser->RequestUpdate();
			//Refresh();
		}

		/**
		 * Sets this level's visibility to the specified value.  Refreshes indirectly.
		 */
		void SetLevelVisibility(UBOOL bVisible)
		{
			const UBOOL bLevelCurrentlyVisible = FLevelUtils::IsLevelVisible( Level );
			if ( bLevelCurrentlyVisible != bVisible )
			{
				LevelBrowser::SetLevelVisibility( Level, bVisible );
				LevelBrowser->UpdateStreamingLevelVolumeVisibility();
				UpdateUI();
			}
		}

		static INT GetHeight()
		{
			return 64;
		}

	private:
		ULevel*				Level;
		WxLevelBrowser*		LevelBrowser;
		FString				Description;

		wxCheckBox*			VisibleCheck;
		WxBitmapButton*		KismetButton;
		WxBitmapButton*		PickColorButton;
		WxBitmapButton*		OpenButton;

		///////////////////
		// Wx events.

		void OnPaint(wxPaintEvent& In);
		void OnLeftButtonDown(wxMouseEvent& In);
		void OnRightButtonDown(wxMouseEvent& In);
		void OnVisibleCheckChanged(wxCommandEvent& In);
		void OnLeftDoubleClick(wxMouseEvent& In);
		void OnSaveLevel(wxCommandEvent& In);
		void OnOpenKismet(wxCommandEvent& In);
		void OnPickColor(wxCommandEvent& In);
		// Declare an empty erase background callback to prevent flicker.
		void OnEraseBackground(wxEraseEvent& event) {};

		DECLARE_EVENT_TABLE();
	};

	WxLevelPane(wxWindow* InParent, WxLevelBrowser* InLevelBrowser)
		:	WxScrollablePane( InParent, InLevelBrowser )
	{}

	/**
	 * Destroys child windows and empties out the list of managed levels.
	 */
	void Clear()
	{
		DestroyChildren();
		LevelWindows.Empty();
	}

	void LayoutLevelWindows()
	{
		LayoutWindows( LevelWindows, 64 );
	}

	/**
	 * Adds the specified level to the level list.  Does nothing if the specified level already exists.
	 *
	 * @param	InLevel			The level to add.
	 * @param	Description		A level description, displayed at the top of the level window.
	 */
	void AddLevel(ULevel* InLevel, const TCHAR* InDescription)
	{
		// Do nothing if the level already exists.
		for ( INT LevelWindowIndex = 0 ; LevelWindowIndex < LevelWindows.Num() ; ++LevelWindowIndex )
		{
			WxLevelWindow* CurLevelWindow = LevelWindows( LevelWindowIndex );
			if ( CurLevelWindow->GetLevel() == InLevel )
			{
				return;
			}
		}

		// Add the level to the list of levels appearing in the level pane.
		LevelWindows.AddItem( new WxLevelWindow( this, LevelBrowser, InLevel, InDescription ) );
	}

	/**
	 * Sync's the specified level's level window.  Refreshes indirectly.
	 */
	void UpdateUIForLevel(const ULevel* InLevel)
	{
		for( INT LevelWindowIndex = 0 ; LevelWindowIndex < LevelWindows.Num() ; ++LevelWindowIndex )
		{
			WxLevelWindow* CurLevelWindow = LevelWindows( LevelWindowIndex );
			if ( InLevel == CurLevelWindow->GetLevel() )
			{
				CurLevelWindow->UpdateUI();
				break;
			}
		}
	}

	/**
	 * Sets the specified level's visibility.
	 */
	void SetLevelVisibility(const ULevel* InLevel, UBOOL bVisible)
	{
		for ( INT LevelWindowIndex = 0 ; LevelWindowIndex < LevelWindows.Num() ; ++LevelWindowIndex )
		{
			WxLevelWindow* CurLevelWindow = LevelWindows( LevelWindowIndex );
			if ( CurLevelWindow->GetLevel() == InLevel )
			{
				CurLevelWindow->SetLevelVisibility( bVisible );
			}
		}
	}

	/**
	 * Sets the specified level's visibility.
	 */
	void SetAllLevelVisibility(UBOOL bVisible)
	{
		const FScopedBusyCursor BusyCursor;
		for ( INT LevelWindowIndex = 0 ; LevelWindowIndex < LevelWindows.Num() ; ++LevelWindowIndex )
		{
			WxLevelWindow* CurLevelWindow = LevelWindows( LevelWindowIndex );
			CurLevelWindow->SetLevelVisibility( bVisible );
		}
	}

	////////////////////////////////
	// Level window iterator

	typedef TArray<WxLevelWindow*>::TIterator TLevelWindowIterator;

	TLevelWindowIterator LevelWindowIterator()
	{
		return TLevelWindowIterator( LevelWindows );
	}


private:

	/** A list of levels appearing in this pane. */
	TArray<WxLevelWindow*>	LevelWindows;

	void OnSize(wxSizeEvent& In)
	{
		LayoutLevelWindows();
		//Refresh();//////////////////////
	}

	/**
	 * Clicking off any levels deselects all levels.
	 */
	void OnLeftButtonDown(wxMouseEvent& In)
	{
		LevelBrowser->DeselectAllLevels();
	}

	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE( WxLevelPane, wxPanel )
	EVT_SIZE( WxLevelPane::OnSize )
	EVT_LEFT_DOWN( WxLevelPane::OnLeftButtonDown )
END_EVENT_TABLE()

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WxLevelPane::WxLevelWindow
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE( WxLevelPane::WxLevelWindow, wxWindow )
	EVT_PAINT( WxLevelPane::WxLevelWindow::OnPaint )
	EVT_LEFT_DOWN( WxLevelPane::WxLevelWindow::OnLeftButtonDown )
	EVT_RIGHT_DOWN( WxLevelPane::WxLevelWindow::OnRightButtonDown )
	EVT_LEFT_DCLICK( WxLevelPane::WxLevelWindow::OnLeftDoubleClick )
	EVT_CHECKBOX( IDCK_LB_LevelVisible, WxLevelPane::WxLevelWindow::OnVisibleCheckChanged )
	EVT_BUTTON( IDM_LB_SaveLevel, WxLevelPane::WxLevelWindow::OnSaveLevel )
	EVT_BUTTON( IDM_LB_OpenKismet, WxLevelPane::WxLevelWindow::OnOpenKismet )
	EVT_BUTTON( IDM_LB_PickColor, WxLevelPane::WxLevelWindow::OnPickColor )
	EVT_ERASE_BACKGROUND( WxLevelPane::WxLevelWindow::OnEraseBackground )
END_EVENT_TABLE()

WxLevelPane::WxLevelWindow::WxLevelWindow(wxWindow* InParent,
										  WxLevelBrowser* InLevelBrowser,
										  ULevel* InLevel,
										  const TCHAR* InDescription)
										  :	WxLockBitmapsWindow( InParent )
										  ,	Level( InLevel )
										  ,	LevelBrowser( InLevelBrowser )
										  ,	Description( InDescription )
										  ,	VisibleCheck( NULL )
										  , KismetButton( NULL )
										  , PickColorButton( NULL )
										  , OpenButton( NULL )
{
	if ( GWorld->PersistentLevel != Level )
	{
		VisibleCheck = new wxCheckBox( this, IDCK_LB_LevelVisible, *LocalizeUnrealEd("VisibleQ"), wxPoint(6,26), wxSize(64,12) );
	}

	WxBitmap* KismetBitmap = new WxMaskedBitmap( TEXT("Kismet") );
	KismetButton = new WxBitmapButton( this, IDM_LB_OpenKismet, *KismetBitmap, wxPoint(220,26), wxSize(18,18) );
	KismetButton->SetToolTip( *LocalizeUnrealEd("ToolTip_OpenKismetForThisLevel") );
	delete KismetBitmap;

	// Don't display the color picker button for the persistent level tab.
	if ( Level != GWorld->PersistentLevel )
	{
		WxBitmap* ColorBitmap = new WxMaskedBitmap( TEXT("Prop_Pick") );
		PickColorButton = new WxBitmapButton( this, IDM_LB_PickColor, *ColorBitmap, wxPoint(240,26), wxSize(18,18) );
		PickColorButton->SetToolTip( *LocalizeUnrealEd("ToolTip_PickColorForThisLevel") );
		delete ColorBitmap;
	}

	WxBitmap* SaveBitmap = new WxMaskedBitmap( TEXT("Save") );
	OpenButton = new WxBitmapButton( this, IDM_LB_SaveLevel, *SaveBitmap, wxPoint(260,26), wxSize(18,18) );
	OpenButton->SetToolTip( *LocalizeUnrealEd("ToolTip_SaveLevel") );
	delete SaveBitmap;

	UpdateUI();
}

void WxLevelPane::WxLevelWindow::OnPaint(wxPaintEvent& In)
{
	wxBufferedPaintDC dc(this);
	const wxRect rc( GetClientRect() );

	// Clear the background.
	dc.Clear();

	dc.SetFont( *wxNORMAL_FONT );

	const wxColour BaseColor( wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE) );

	// Get the level color.
	wxColour LevelColor( BaseColor );
	ULevelStreaming* LevelStreaming = FLevelUtils::FindStreamingLevel( Level );
	if ( LevelStreaming )
	{
		const FColor& LevelDrawColor = LevelStreaming->DrawColor;
		LevelColor = wxColour( LevelDrawColor.R, LevelDrawColor.G, LevelDrawColor.B );
	}

	if( LevelBrowser->IsSelected( Level ) )
	{
		dc.SetPen( *wxMEDIUM_GREY_PEN );
		dc.SetBrush( wxBrush( wxColour(64,64,64), wxSOLID ) );
		dc.DrawRoundedRectangle( rc.x+1,rc.y+1, rc.width-2,rc.height-2, 5 );
		dc.SetPen( *wxTRANSPARENT_PEN );
		dc.SetBrush( wxBrush( LevelColor, wxSOLID ) );
		dc.DrawRectangle( rc.x+2,rc.y+20, rc.width-4,rc.height-27 );

		if ( VisibleCheck )
		{
			VisibleCheck->SetBackgroundColour( LevelColor );
		}

		dc.SetTextForeground( *wxWHITE );
		dc.SetTextBackground( LevelColor );
	}
	else
	{
		const wxColour DarkerColor( BaseColor.Red()-32, BaseColor.Green()-32, BaseColor.Blue()-32 );

		dc.SetPen( *wxMEDIUM_GREY_PEN );
		dc.SetBrush( wxBrush( DarkerColor, wxSOLID ) );
		dc.DrawRoundedRectangle( rc.x+1,rc.y+1, rc.width-2,rc.height-2, 5 );
		dc.SetPen( *wxMEDIUM_GREY_PEN );
		dc.SetBrush( wxBrush( LevelColor, wxSOLID ) );
		dc.DrawRectangle( rc.x+2,rc.y+20, rc.width-4,rc.height-27 );

		if ( VisibleCheck )
		{
			VisibleCheck->SetBackgroundColour( LevelColor );
		}
	}

	// Create a string to use as the level description.
	FString DescriptionString( FString::Printf( LocalizeSecure(LocalizeUnrealEd("LayerDesc_F"), *Description, Level->Actors.Num()-2) ) );

	// Append an asterisk if the level's package is unsaved.
	UPackage* Package = CastChecked<UPackage>( Level->GetOutermost() );
	if ( Package->IsDirty() )
	{
		DescriptionString += TEXT("*");
		OpenButton->Enable();
	}
	else
	{
		OpenButton->Disable();
	}

	// Append a string indicating the level is the current level.
	if ( GWorld->CurrentLevel == Level )
	{
		wxFont Font( dc.GetFont() );
		Font.SetWeight( wxBOLD );
		dc.SetFont( Font );
		DescriptionString += TEXT("  - CURRENT -");
	}

	// Draw the loc bitmap.
	DrawCurLockBitmap( dc, rc );

	// Draw the level description text.
	dc.DrawText( *DescriptionString, rc.x+20,rc.y+3 );
}

void WxLevelPane::WxLevelWindow::OnLeftButtonDown(wxMouseEvent& In)
{
	if( ClickedOnLock( In ) )
	{
		FLevelUtils::ToggleLevelLock( Level );
		const UBOOL bIsLevelLocked = FLevelUtils::IsLevelLocked( Level );
		SetCurLockBitmap( bIsLevelLocked );

		// If the user just locked the current level, make the persistent level current.
		if ( bIsLevelLocked && Level == GWorld->CurrentLevel )
		{
			LevelBrowser->MakeLevelCurrent( GWorld->PersistentLevel );
			LevelBrowser->RequestUpdate();
		}

		// The level window was clicked on, so select this level.
		LevelBrowser->SelectSingleLevel( Level );
	}
	else
	{
		if ( !In.ControlDown() )
		{
			// Clicking sets this level as the selected level.
			LevelBrowser->SelectSingleLevel( Level );
		}
		else
		{
			// Ctrl clicking toggles this level's selection status.
			const UBOOL bIsSelected = LevelBrowser->IsSelected( Level );
			if ( bIsSelected )
			{
				LevelBrowser->DeselectLevel( Level );
			}
			else
			{
				LevelBrowser->SelectLevel( Level );
			}
		}
	}

	Refresh();
}

void WxLevelPane::WxLevelWindow::OnRightButtonDown(wxMouseEvent& In)
{
	class WxLevelPopupMenu : public wxMenu
	{
	public:
		WxLevelPopupMenu(ULevel* Level)
		{
			INT NumItems = 0;

			// Don't offer the option to load/unload if the level is locked.
			if ( !FLevelUtils::IsLevelLocked( Level ) )
			{
				Append( IDM_LB_MakeCurrentLevel, *LocalizeUnrealEd("MakeCurrent"), TEXT("") );
				++NumItems;
			}

			if ( NumItems )
			{
				AppendSeparator();
				NumItems = 0;
			}
			Append( IDM_LB_ShowSelectedLevelsInSceneManager, *LocalizeUnrealEd("ShowSelectedLevelsInSceneManager"), TEXT("") );
			Append( IDM_LB_SelectAllActors, *LocalizeUnrealEd("SelectAllActors"), TEXT("") );
			++NumItems;

			if ( NumItems )
			{
				AppendSeparator();
				NumItems = 0;
			}
			Append( IDM_LB_SaveSelectedLevels, *LocalizeUnrealEd("SaveSelectedLevels"), TEXT("") );
			++NumItems;

			AppendSeparator();

			// Don't offer the option to load/unload or edit properties if this is the persistent level.
			if ( Level != GWorld->PersistentLevel )
			{
				Append( IDM_LB_LevelProperties, *LocalizeUnrealEd("Properties"), TEXT("") );
			}

			// Self contained lighting check.
			AppendCheckItem( IDM_LB_SelfContainedLighting, *LocalizeUnrealEd("SelfContainedLighting") );
			const UBOOL bHasSelfContainedLighting = FLevelUtils::HasSelfContainedLighting( Level );
			Check( IDM_LB_SelfContainedLighting, bHasSelfContainedLighting == TRUE );

			// Operations based on selected levels.
			AppendSeparator();
			Append( IDM_LB_ShowSelectedLevels, *LocalizeUnrealEd("ShowSelectedLevels"), TEXT("") );
			Append( IDM_LB_HideSelectedLevels, *LocalizeUnrealEd("HideSelectedLevels"), TEXT("") );
			AppendSeparator();
			Append( IDM_LB_ShowOnlySelectedLevels, *LocalizeUnrealEd("ShowOnlySelectedLevels"), TEXT("") );
			Append( IDM_LB_ShowOnlyUnselectedLevels, *LocalizeUnrealEd("ShowOnlyUnselectedLevels"), TEXT("") );
			AppendSeparator();
			Append( IDM_LB_ShowAllLevels, *LocalizeUnrealEd("ShowAllLevels"), TEXT("") );
			Append( IDM_LB_HideAllLevels, *LocalizeUnrealEd("HideAllLevels"), TEXT("") );
			AppendSeparator();
			Append( IDM_LB_SelectAllLevels, *LocalizeUnrealEd("SelectAllLevels"), TEXT("") );
			Append( IDM_LB_DeselectAllLevels, *LocalizeUnrealEd("DeselectAllLevels"), TEXT("") );
			Append( IDM_LB_InvertLevelSelection, *LocalizeUnrealEd("InvertLevelSelection"), TEXT("") );

			// For options only presented when actors are selected.
			UBOOL bActorSelected = FALSE;
			UBOOL bStreamingLevelVolumeSelected = FALSE;
			for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
			{
				bActorSelected = TRUE;
				if ( Cast<ALevelStreamingVolume>( *It ) )
				{
					bStreamingLevelVolumeSelected = TRUE;
					break;
				}
			}

			AppendSeparator();
			// If any level streaming volumes are selected, present the option to associate the volumes with selected levels.
			if ( bStreamingLevelVolumeSelected )
			{
				Append( IDM_LB_AddSelectedLevelsToLevelStreamingVolumes, *LocalizeUnrealEd("AddStreamingVolumes"), TEXT("") );
				Append( IDM_LB_SetSelectedLevelsToLevelStreamingVolumes, *LocalizeUnrealEd("SetStreamingVolumes"), TEXT("") );
			}
			Append( IDM_LB_ClearLevelStreamingVolumeAssignments, *LocalizeUnrealEd("ClearStreamingVolumes"), TEXT("") );
			Append( IDM_LB_SelectAssociatedStreamingVolumes, *LocalizeUnrealEd("SelectAssociatedStreamingVolumes"), TEXT("") );

			// If any actors are selected, present the option to move actors between levels.
			if ( bActorSelected )
			{
				AppendSeparator();
				Append( IDM_LB_MoveSelectedActorsToCurrentLevel, *LocalizeUnrealEd("MoveSelectedActorsToCurrentLevel"), TEXT("") );
			}
		}
	};

	WxLevelPopupMenu Menu( Level );
	FTrackPopupMenu tpm( this, &Menu );
	tpm.Show();
}

void WxLevelPane::WxLevelWindow::OnVisibleCheckChanged(wxCommandEvent& In)
{
	const UBOOL bNewLevelVisibility = !FLevelUtils::IsLevelVisible( Level );
	SetLevelVisibility( bNewLevelVisibility );
	LevelBrowser->SelectSingleLevel( Level );
}

void WxLevelPane::WxLevelWindow::OnLeftDoubleClick(wxMouseEvent& In)
{
	// Double clicking on the level window makes the level associated with this window current.
	LevelBrowser->MakeLevelCurrent( Level );
}

void WxLevelPane::WxLevelWindow::OnSaveLevel(wxCommandEvent& In)
{
	if ( !FLevelUtils::IsLevelVisible( Level ) )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("UnableToSaveInvisibleLevels") );
		return;
	}

	LevelBrowser->SelectSingleLevel( Level );
	FEditorFileUtils::SaveLevel( Level );
}

void WxLevelPane::WxLevelWindow::OnOpenKismet(wxCommandEvent& In)
{
	LevelBrowser->SelectSingleLevel( Level );
	// Open this level's kismet sequence, creating one if it does not exist.
	USequence* Sequence = Level->GetGameSequence();
	if ( !Sequence )
	{
		// The level has no sequence -- create a new one.
		Sequence = ConstructObject<USequence>( USequence::StaticClass(), Level, TEXT("Main_Sequence"), RF_Transactional );
		GWorld->SetGameSequence( Sequence, Level );
		Sequence->MarkPackageDirty();
		GCallbackEvent->Send( CALLBACK_LevelDirtied );
		GCallbackEvent->Send( CALLBACK_RefreshEditor_Kismet );
	}
	WxKismet::OpenSequenceInKismet( Sequence, GApp->EditorFrame );
}

void WxLevelPane::WxLevelWindow::OnPickColor(wxCommandEvent& In)
{
	LevelBrowser->SelectSingleLevel( Level );
	if ( GWorld->PersistentLevel != Level )
	{
		// Initialize the color data for the picker window.
		ULevelStreaming* StreamingLevel = FLevelUtils::FindStreamingLevel( Level );
		check( StreamingLevel );

		wxColourData ColorData;
		ColorData.SetChooseFull( true );
		const FColor PrevColor( StreamingLevel->DrawColor );
		ColorData.SetColour(wxColour(PrevColor.R,PrevColor.G,PrevColor.B));

		// Get topmost window for modal dialog to prevent clicking away
		wxWindow* TopWindow = this;
		while( TopWindow->GetParent() )
		{
			TopWindow = TopWindow->GetParent();
		}

		WxDlgColor ColorDialog;
		ColorDialog.Create( TopWindow, &ColorData );
		if( ColorDialog.ShowModal() == wxID_OK )
		{
			const wxColour clr( ColorDialog.GetColourData().GetColour() );
			StreamingLevel->DrawColor = FColor(clr.Red(), clr.Green(), clr.Blue() );
			StreamingLevel->MarkPackageDirty();

			LevelBrowser->UpdateLevelPropertyWindow();
			GCallbackEvent->Send( CALLBACK_LevelDirtied );
			LevelBrowser->RequestUpdate();
			GCallbackEvent->Send( CALLBACK_RedrawAllViewports );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WxGroupPane
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * The panel on the right side of the level browser that contains a scrollable
 * list of groups corresponding to selected levels.
 */
class WxGroupPane : public WxScrollablePane
{
public:
	WxGroupPane(wxWindow* InParent, WxLevelBrowser* InLevelBrowser)
		:	WxScrollablePane( InParent, InLevelBrowser )
	{}

	/**
	 * Destroys child windows and empties out the list of managed groups.
	 */
	void Clear()
	{
		DestroyChildren();
		//GroupWindows.Empty();
	}

	void SetLevel(ULevel* InLevel)
	{

	}

	/**
	 * Does the layout for group windows in this pane.
	 */
	void LayoutGroupWindows()
	{
		//LayoutWindows( GroupWindows, 64 );
	}

private:

	/** A list of groupss appearing in this pane. */
	//TArray<WxGroupWindow*>	GroupWindows;

	void OnSize(wxSizeEvent& In)
	{
		LayoutGroupWindows();
		//Refresh();
		//In.Skip();
	}

	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE( WxGroupPane, wxPanel )
	EVT_SIZE( WxGroupPane::OnSize )
END_EVENT_TABLE()

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WxLevelBrowser
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * The toolbar appearing at the top of the level browser.
 */
class WxLevelBrowserToolBar : public wxToolBar
{
public:
	WxLevelBrowserToolBar(wxWindow* InParent, wxWindowID InID)
		:	wxToolBar( InParent, InID, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_3DBUTTONS )
	{
		ReimportB.Load( TEXT("Reimport") );
		AddTool( IDM_LB_LevelProperties, TEXT(""), ReimportB, *LocalizeUnrealEd("Properties") );
		AddSeparator();
		Realize();
	}
private:
	WxMaskedBitmap ReimportB;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WxLevelBrowser
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE( WxLevelBrowser, WxBrowser )
	EVT_SIZE( WxLevelBrowser::OnSize )
	EVT_MENU( IDM_RefreshBrowser, WxLevelBrowser::OnRefresh )

	EVT_MENU( IDM_LB_NewLevel, WxLevelBrowser::OnNewLevel )
	EVT_MENU( IDM_LB_NewLevelFromSelectedActors, WxLevelBrowser::OnNewLevelFromSelectedActors )
	EVT_MENU( IDM_LB_ImportLevel, WxLevelBrowser::OnImportLevel )
	EVT_MENU( IDM_LB_RemoveLevelFromWorld, WxLevelBrowser::OnRemoveLevelFromWorld )

	EVT_MENU( IDM_LB_MakeCurrentLevel, WxLevelBrowser::OnMakeLevelCurrent )
	EVT_MENU( IDM_LB_MergeVisibleLevels, WxLevelBrowser::OnMergeVisibleLevels )
	EVT_MENU( IDM_LB_SaveSelectedLevels, WxLevelBrowser::OnSaveSelectedLevels )
	EVT_MENU( IDM_LB_ShowSelectedLevelsInSceneManager, WxLevelBrowser::ShowSelectedLevelsInSceneManager )
	EVT_MENU( IDM_LB_SelectAllActors, WxLevelBrowser::OnSelectAllActors )
	EVT_MENU( IDM_LB_LevelProperties, WxLevelBrowser::OnLevelProperties )
	EVT_MENU( IDM_LB_SelfContainedLighting, WxLevelBrowser::OnToggleSelfContainedLighting )

	EVT_MENU( IDM_LB_ShowOnlySelectedLevels, WxLevelBrowser::OnShowOnlySelectedLevels )
	EVT_MENU( IDM_LB_ShowOnlyUnselectedLevels, WxLevelBrowser::OnShowOnlyUnselectedLevels )

	EVT_MENU( IDM_LB_ShowSelectedLevels, WxLevelBrowser::OnShowSelectedLevels )
	EVT_MENU( IDM_LB_HideSelectedLevels, WxLevelBrowser::OnHideSelectedLevels )
	EVT_MENU( IDM_LB_ShowAllLevels, WxLevelBrowser::OnShowAllLevels )
	EVT_MENU( IDM_LB_HideAllLevels, WxLevelBrowser::OnHideAllLevels )

	EVT_MENU( IDM_LB_SelectAllLevels, WxLevelBrowser::OnSelectAllLevels )
	EVT_MENU( IDM_LB_DeselectAllLevels, WxLevelBrowser::OnDeselectAllLevels )
	EVT_MENU( IDM_LB_InvertLevelSelection, WxLevelBrowser::OnInvertSelection )

	EVT_MENU( IDM_LB_MoveSelectedActorsToCurrentLevel, WxLevelBrowser::OnMoveSelectedActorsToCurrentLevel )

	EVT_MENU( IDM_LB_AddSelectedLevelsToLevelStreamingVolumes, WxLevelBrowser::OnAddStreamingLevelVolumes )
	EVT_MENU( IDM_LB_SetSelectedLevelsToLevelStreamingVolumes, WxLevelBrowser::OnSetStreamingLevelVolumes )
	EVT_MENU( IDM_LB_ClearLevelStreamingVolumeAssignments, WxLevelBrowser::OnClearStreamingLevelVolumes )
	EVT_MENU( IDM_LB_SelectAssociatedStreamingVolumes, WxLevelBrowser::OnSelectStreamingLevelVolumes )

	EVT_MENU( IDM_LB_ShowSelectedLevels, WxLevelBrowser::OnShowSelectedLevels )

	EVT_MENU( IDM_LB_ShiftLevelUp, WxLevelBrowser::ShiftLevelUp )
	EVT_MENU( IDM_LB_ShiftLevelDown, WxLevelBrowser::ShiftLevelDown )
END_EVENT_TABLE()

WxLevelBrowser::WxLevelBrowser()
	:	LevelPane( NULL )
	,	GroupPane( NULL )
	,	SplitterWnd( NULL )
	,	LevelPropertyWindow( NULL )
	,	bDoFullUpdate( FALSE )
	,	bUpdateOnActivated( FALSE )
	,	SuppressUpdateMutex( 0 )
{
	// Register which events we want
	GCallbackEvent->Register(CALLBACK_LevelDirtied,this);
	GCallbackEvent->Register(CALLBACK_WorldChange,this);
	GCallbackEvent->Register(CALLBACK_RefreshEditor_LevelBrowser,this);
}

/**
* Forwards the call to our base class to create the window relationship.
* Creates any internally used windows after that
*
* @param DockID the unique id to associate with this dockable window
* @param FriendlyName the friendly name to assign to this window
* @param Parent the parent of this window (should be a Notebook)
*/
void WxLevelBrowser::Create(INT DockID, const TCHAR* FriendlyName, wxWindow* Parent)
{
	WxBrowser::Create( DockID, FriendlyName, Parent );

	// Create the sizer.
	wxBoxSizer* Sizer = new wxBoxSizer( wxVERTICAL );
	SetSizer( Sizer );

	// Create the menu bar that sits at the top of the browser window.
	MenuBar = new WxLevelBrowserMenuBar();

	// @todo DB: figure out why toolbar registration isn't working for WxBrowser-derived classes.
	//ToolBar = new WxLevelBrowserToolBar( (wxWindow*)this, -1 );

	// Create the splitter separating the level and group panes.
	//const long SplitterStyles = wxSP_LIVE_UPDATE;// | wxSP_FULLSASH;
	//SplitterWnd = new wxSplitterWindow( this, -1 );//, wxDefaultPosition, wxDefaultSize, SplitterStyles );
	SplitterWnd = new wxSplitterWindow( this, -1, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE );//wxSP_3DBORDER|wxSP_3DSASH|wxNO_BORDER );

	// Indicate the minimum number of pixels a pane can be reduced to by dragging the sash.
	// This prevents either pane from being reduced to size zero.
	//const INT MinimumPaneSizeInPixels = 80;
	//SplitterWnd->SetMinimumPaneSize( MinimumPaneSizeInPixels );

	// Create the level and group planes.
	LevelPane = new WxLevelPane( SplitterWnd, this );
	GroupPane = new WxGroupPane( SplitterWnd, this );

	Sizer->Add( SplitterWnd, 1, wxGROW|wxALL, 0 );

	// Initialize the left and right panes of the splitter window.
	const INT InitialSashPosition = 300;
	SplitterWnd->SplitVertically( LevelPane, GroupPane, InitialSashPosition );

	// Tell the sizer to match the sizer's minimum size.
	GetSizer()->Fit( this );

	// Set size hints to honour minimum size.
	GetSizer()->SetSizeHints( this );

	// Do an update.
	//RequestUpdate( TRUE );
}

void WxLevelBrowser::OnSize(wxSizeEvent& In)
{
	// During the creation process a sizing message can be sent so don't
	// handle it until we are initialized
	if ( bAreWindowsInitialized )
	{
		const wxRect rc( GetClientRect() );
		SplitterWnd->SetSize( rc );
	}
}

/**
 * Disassociates all objects from the level property window, then hides it.
 */
void WxLevelBrowser::ClearPropertyWindow()
{
	if ( LevelPropertyWindow )
	{
		LevelPropertyWindow->GetPropertyWindow()->GetRoot()->RemoveAllObjects();
		LevelPropertyWindow->Show( FALSE );
	}
}

void WxLevelBrowser::Send(ECallbackEventType Event)
{
	// if we are currently in the PIE world, there's no need to update the level browser
	if (GIsPlayInEditorWorld)
	{
		return;
	}

	if ( Event == CALLBACK_LevelDirtied )
	{
		// Level dirties require only a redraw.
		RequestUpdate();
	}
	else
	{
		if ( Event == CALLBACK_WorldChange )
		{
			ClearPropertyWindow();
		}

		// Reset the scrolling position, so that child levels will redraw into the correct position.
		LevelPane->Scroll( 0, 0 );

		// Do a full rebuild.
		Update();//RequestUpdate( TRUE );
	}
}

/**
 * Adds entries to the browser's accelerator key table.  Derived classes should call up to their parents.
 */
void WxLevelBrowser::AddAcceleratorTableEntries(TArray<wxAcceleratorEntry>& Entries)
{
	WxBrowser::AddAcceleratorTableEntries( Entries );
	Entries.AddItem( wxAcceleratorEntry( wxACCEL_NORMAL, WXK_UP, IDM_LB_ShiftLevelUp ) );
	Entries.AddItem( wxAcceleratorEntry( wxACCEL_NORMAL, WXK_DOWN, IDM_LB_ShiftLevelDown ) );
}

/** Handler for IDM_RefreshBrowser events; updates the browser contents. */
void WxLevelBrowser::OnRefresh(wxCommandEvent& In)
{
	// Rebuild the level, layer and actor panes.
	Update();
}

void WxLevelBrowser::RequestUpdate(UBOOL bFullUpdate)
{
	bDoFullUpdate |= bFullUpdate;
	Refresh();
}

void WxLevelBrowser::Update()
{
	//if ( bDoFullUpdate )
	if ( IsShown() && SuppressUpdateMutex == 0 )
	{
		BeginUpdate();

		UpdateLevelList();
		UpdateLevelPane();
		UpdateGroupPane();
		bDoFullUpdate = FALSE;
		bUpdateOnActivated = FALSE;

		EndUpdate();
	}
	else
	{
		bUpdateOnActivated = TRUE;
	}
}

void WxLevelBrowser::Activated()
{
	WxBrowser::Activated();
	if ( bUpdateOnActivated && SuppressUpdateMutex == 0 )
	{
		Update();
	}
}

void WxLevelBrowser::AddLevel(ULevel* Level, const TCHAR* Description)
{
	LevelPane->AddLevel( Level, Description );
}

void WxLevelBrowser::UpdateLevelList()
{
	// Clear level list.
	LevelPane->Clear();
	SelectedLevels.Empty();
	UpdateStreamingLevelVolumeVisibility( FALSE );

	if ( GWorld )
	{
		// Add main level.
		AddLevel( GWorld->PersistentLevel, *LocalizeUnrealEd( "PersistentLevel" ) );

		// Add secondary levels.
		AWorldInfo*	WorldInfo = GWorld->GetWorldInfo();
		for( INT LevelIndex = 0 ; LevelIndex < WorldInfo->StreamingLevels.Num() ; ++LevelIndex )
		{
			ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
			if( StreamingLevel && StreamingLevel->LoadedLevel )
			{
				AddLevel( StreamingLevel->LoadedLevel, *StreamingLevel->PackageName.ToString() );
			}
		}
	}
}

void WxLevelBrowser::UpdateLevelPane()
{
	LevelPane->LayoutLevelWindows();
}

void WxLevelBrowser::UpdateGroupPane()
{
	// Notify the group pane of the selected level and refresh it.
	GroupPane->SetLevel( GetSelectedLevel() );
	GroupPane->LayoutGroupWindows();
}

/**
 * Clears the level selection, then sets the specified level.  Refreshes.
 */
void WxLevelBrowser::SelectSingleLevel(ULevel* InLevel)
{
	SelectedLevels.Empty();
	SelectedLevels.AddItem( InLevel );
	UpdateStreamingLevelVolumeVisibility();
	RequestUpdate();
}

/**
 * Adds the specified level to the selection set.  Refreshes.
 */
void WxLevelBrowser::SelectLevel(ULevel* InLevel)
{
	SelectedLevels.AddUniqueItem( InLevel );
	UpdateStreamingLevelVolumeVisibility();
	RequestUpdate();
}

/**
 * Selects all selected levels.  Refreshes.
 */
void WxLevelBrowser::SelectAllLevels()
{
	for ( WxLevelPane::TLevelWindowIterator It( LevelPane->LevelWindowIterator() ) ; It ; ++It )
	{
		WxLevelPane::WxLevelWindow* CurWindow = *It;
		SelectedLevels.AddUniqueItem( CurWindow->GetLevel() );
	}
	UpdateStreamingLevelVolumeVisibility();
	RequestUpdate();
}

/**
 * Deselects the specified level.  Refreshes.
 */	
void WxLevelBrowser::DeselectLevel(const ULevel* InLevel)
{
	for ( INT LevelIndex = 0 ; LevelIndex < SelectedLevels.Num() ; ++LevelIndex )
	{
		const ULevel* CurLevel = SelectedLevels( LevelIndex );
		if ( CurLevel == InLevel )
		{
			SelectedLevels.Remove( LevelIndex );
			UpdateStreamingLevelVolumeVisibility();
			RequestUpdate();
			break;
		}
	}
}

/**
 * Deselects all selected levels.  Refreshes.
 */
void WxLevelBrowser::DeselectAllLevels()
{
	SelectedLevels.Empty();
	UpdateStreamingLevelVolumeVisibility();
	RequestUpdate();
}

/**
 * Inverts the level selection.  Refreshes.
 */
void WxLevelBrowser::InvertLevelSelection()
{
	TArray<ULevel*> LevelsToBeSelected;

	// Iterate over all levels and mark unselected levels for selection.
	for ( WxLevelPane::TLevelWindowIterator It( LevelPane->LevelWindowIterator() ) ; It ; ++It )
	{
		WxLevelPane::WxLevelWindow* CurWindow = *It;
		ULevel* CurLevel = CurWindow->GetLevel();
		if ( !IsSelected( CurLevel ) )
		{
			LevelsToBeSelected.AddItem( CurLevel );
		}
	}

	// Clear out current selections.
	SelectedLevels.Empty();

	// Select marked levels.
	for ( INT LevelIndex = 0 ; LevelIndex < LevelsToBeSelected.Num() ; ++LevelIndex )
	{
		ULevel* CurLevel = LevelsToBeSelected( LevelIndex );
		SelectedLevels.AddItem( CurLevel );
	}
	UpdateStreamingLevelVolumeVisibility();
	RequestUpdate();
}

/**
 * @return		TRUE if the specified level is selected in the level browser.
 */
UBOOL WxLevelBrowser::IsSelected(ULevel* InLevel) const
{
	return SelectedLevels.ContainsItem( InLevel );
}

/**
 * Returns the head of the selection list, or NULL if nothing is selected.
 */
ULevel* WxLevelBrowser::GetSelectedLevel()
{
	TSelectedLevelIterator It( SelectedLevelIterator() );
	ULevel* SelectedLevel = It ? *It : NULL;
	return SelectedLevel;
}

/**
 * Returns the head of the selection list, or NULL if nothing is selected.
 */
const ULevel* WxLevelBrowser::GetSelectedLevel() const
{
	TSelectedLevelConstIterator It( SelectedLevelConstIterator() );
	const ULevel* SelectedLevel = It ? *It : NULL;
	return SelectedLevel;
}

/**
 * Returns NULL if the number of selected level is zero or more than one.  Otherwise,
 * returns the singly selected level.
 */
ULevel* WxLevelBrowser::GetSingleSelectedLevel()
{
	// See if there is a single level selected.
	ULevel* SingleSelectedLevel = NULL;
	for ( TSelectedLevelIterator It = SelectedLevelIterator() ; It ; ++It )
	{
		ULevel* Level = *It;
		if ( Level )
		{
			if ( !SingleSelectedLevel )
			{
				SingleSelectedLevel = Level;
			}
			else
			{
				// Multiple levels are selected.
				return NULL;
			}
		}
	}
	return SingleSelectedLevel;
}

WxLevelBrowser::TSelectedLevelIterator WxLevelBrowser::SelectedLevelIterator()
{
	return TSelectedLevelIterator( SelectedLevels );
}

WxLevelBrowser::TSelectedLevelConstIterator WxLevelBrowser::SelectedLevelConstIterator() const
{
	return TSelectedLevelConstIterator( SelectedLevels );
}

/**
 * Returns the number of selected levels.
 */
INT WxLevelBrowser::GetNumSelectedLevels() const
{
	return SelectedLevels.Num();
}

/**
 * Displays a property window for the selected levels.
 */
void WxLevelBrowser::ShowSelectedLevelPropertyWindow()
{
	TArray<ULevelStreaming*> StreamingLevels;

	for ( TSelectedLevelIterator It = SelectedLevelIterator() ; It ; ++It )
	{
		ULevel* Level = *It;
		ULevelStreaming* StreamingLevel = FLevelUtils::FindStreamingLevel( Level ); 
		if ( StreamingLevel )
		{
			StreamingLevels.AddItem( StreamingLevel );
		}
	}

	if ( StreamingLevels.Num() )
	{
		// Allocate a new property window if none exists.
		if ( !LevelPropertyWindow )
		{
			LevelPropertyWindow = new WxPropertyWindowFrame;
			LevelPropertyWindow->Create( GApp->EditorFrame, -1, 0, this );
		}

		// Display the level in the property window.
		//LevelPropertyWindow->AllowClose();
		LevelPropertyWindow->SetObjectArray( StreamingLevels, 1,1,1 );
		LevelPropertyWindow->Show();
	}
}

/**
 * Refreshes the level browser if any level properties were modified.  Refreshes indirectly.
 */
void WxLevelBrowser::NotifyPostChange(void* Src, UProperty* PropertyThatChanged)
{
	// Get the level that was modified from the property window, if it was specified
	if( Src )
	{
		// Check if ULevelStreaming::bShouldBeVisibleInEditor was the property that changed.
		UBOOL bVisibilityChanged = FALSE;
		if ( PropertyThatChanged )
		{
			bVisibilityChanged = appStricmp( *PropertyThatChanged->GetName(), TEXT("bShouldBeVisibleInEditor") ) == 0;
		}

		WxPropertyWindow_Base* PropertyWindowItem = static_cast<WxPropertyWindow_Base*>( Src );
		WxPropertyWindow* PropertyWindow = PropertyWindowItem->GetPropertyWindow();
		for ( WxPropertyWindow::TObjectIterator Itor( PropertyWindow->ObjectIterator() ) ; Itor ; ++Itor )
		{
			ULevelStreaming* LevelStreaming = CastChecked<ULevelStreaming>( *Itor );
			if ( bVisibilityChanged )
			{
				LevelBrowser::SetLevelVisibility( LevelStreaming->LoadedLevel, LevelStreaming->bShouldBeVisibleInEditor );
			}
			UpdateUIForLevel( LevelStreaming->LoadedLevel );
		}
		if ( bVisibilityChanged )
		{
			UpdateStreamingLevelVolumeVisibility();
		}
	}
	else
	{
		// We can't get at the level.  Just do a simple refresh.
		RequestUpdate();
	}
}

/**
 * Used to track when the level browser's level property window is destroyed.
 */
void WxLevelBrowser::NotifyDestroy(void* Src)
{
	if ( Src == LevelPropertyWindow )
	{
		LevelPropertyWindow = NULL;
	}
}

/**
 * Clear out object references to allow levels to be GCed.
 *
 * @param	Ar			The archive to serialize with.
 */
void WxLevelBrowser::Serialize(FArchive& Ar)
{
	if ( !GIsPlayInEditorWorld )
	{
		LevelPane->Clear();
		SelectedLevels.Empty();
	}
}

/**
 * Updates the property window that contains level streaming objects, if it exists.
 */
void WxLevelBrowser::UpdateLevelPropertyWindow()
{
	if ( LevelPropertyWindow )
	{
		LevelPropertyWindow->Rebuild();
	}
}

/**
 * Creates a new streaming level.
 *
 * @param	bMoveSelectedActorsIntoNewLevel		If TRUE, move any selected actors into the new level.
 */
void WxLevelBrowser::CreateNewLevel(UBOOL bMoveSelectedActorsIntoNewLevel)
{
	// Disallow for cooked packages.
	if( GWorld && GWorld->GetOutermost()->PackageFlags & PKG_Cooked )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Error_OperationDisallowedOnCookedContent") );
		return;
	}

	DisableUpdate();

	// Matinee cannot be opened when any level saving occurs.
	GEditorModeTools().SetCurrentMode( EM_Default );

	// Cache the current GWorld and clear it, then allocate a new one, and finally restore it.
	UWorld* OldGWorld = GWorld;
	GWorld = NULL;
	UWorld::CreateNew();
	UWorld* NewGWorld = GWorld;

	check(NewGWorld);
	// Save the new world to disk.
	const UBOOL bNewWorldSaved = FEditorFileUtils::SaveLevel( NewGWorld->PersistentLevel );
	FString NewPackageName;
	if ( bNewWorldSaved )
	{
		NewPackageName = GWorld->GetOutermost()->GetName();
	}

	// Restore the old GWorld and GC the new one.
	GWorld = OldGWorld;

	NewGWorld->RemoveFromRoot();
	UObject::CollectGarbage( GARBAGE_COLLECTION_KEEPFLAGS );

	EnableUpdate();

	// If the new world was saved successfully, import it as a streaming level.
	if ( bNewWorldSaved )
	{
		ULevel* NewLevel = AddLevelToWorld( *NewPackageName );

		if ( bMoveSelectedActorsIntoNewLevel )
		{
			WxLevelBrowser::MakeLevelCurrent( NewLevel );
			GEditor->MoveSelectedActorsToCurrentLevel();
		}
	}
}

/**
 * Presents an "Open File" dialog to the user and adds the selected level(s) to the world.  Refreshes.
 */
void WxLevelBrowser::ImportLevelsFromFile()
{
	// Disallow for cooked packages.
	if( GWorld && GWorld->GetOutermost()->PackageFlags & PKG_Cooked )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Error_OperationDisallowedOnCookedContent") );
		return;
	}

	WxFileDialog FileDialog( GApp->EditorFrame,
								*LocalizeUnrealEd("Open"),
								*GApp->LastDir[LD_UNR],
								TEXT(""),
								*FString::Printf( TEXT("Map files (*.%s)|*.%s|All files (*.*)|*.*"), *GApp->MapExt, *GApp->MapExt ),
								wxOPEN | wxFILE_MUST_EXIST | wxMULTIPLE );

	if( FileDialog.ShowModal() == wxID_OK )
	{
		// Get the set of selected paths from the dialog
		wxArrayString FilePaths;
		FileDialog.GetPaths( FilePaths );

		TArray<FString> Filenames;
		for( UINT FileIndex = 0 ; FileIndex < FilePaths.Count() ; ++FileIndex )
		{
			// Strip paths from to get the level package names.
			const FFilename FilePath( FilePaths[FileIndex] );
			Filenames.AddItem( FilePath.GetBaseFilename() );
		}

		// Sort the level packages alphabetically by name.
		Sort<USE_COMPARE_CONSTREF(FString, LevelBrowser)>( &Filenames(0), Filenames.Num() );

		// Fire CALLBACK_LevelDirtied when falling out of scope.
		FScopedLevelDirtied LevelDirtyCallback;

		// Try to add the levels that were specified in the dialog.
		for( INT FileIndex = 0 ; FileIndex < Filenames.Num() ; ++FileIndex )
		{
			const FString& BaseFilename	= Filenames(FileIndex);
			if ( AddLevelToWorld( *BaseFilename ) )
			{
				LevelDirtyCallback.Request();
			}
		} // for each file

		// refresh editor windows
		GCallbackEvent->Send( CALLBACK_RefreshEditor_AllBrowsers );
	}
}

static TArray<FString> GStreamingMethodStrings;
static TArray<UClass*> GStreamingMethodClassList;

/**
 * Initializes the list of possible level streaming methods. 
 * Does nothing if the lists are already initialized.
 */
static void InitializeStreamingMethods()
{
	check( GStreamingMethodStrings.Num() == GStreamingMethodClassList.Num() );
	if ( GStreamingMethodClassList.Num() == 0 )
	{
		// Assemble a list of possible level streaming methods.
		for ( TObjectIterator<UClass> It ; It ; ++It )
		{
			if ( It->IsChildOf( ULevelStreaming::StaticClass() ) &&
				(It->ClassFlags & CLASS_EditInlineNew) &&
				!(It->ClassFlags & CLASS_Hidden) &&
				!(It->ClassFlags & CLASS_Abstract) &&
				!(It->ClassFlags & CLASS_Deprecated) &&
				!(It->ClassFlags & CLASS_Transient) )
			{
				const FString ClassName( It->GetName() );
				// Strip the leading "LevelStreaming" text from the class name.
				// @todo DB: This assumes the names of all ULevelStreaming-derived types begin with the string "LevelStreaming".
				GStreamingMethodStrings.AddItem( ClassName.Mid( 14 ) );
				GStreamingMethodClassList.AddItem( *It );
			}
		}
	}
}

/**
 * Adds the named level package to the world.  Does nothing if the level already exists in the world.
 *
 * @param	LevelPackageBaseFilename	The base filename of the level package to add.
 * @return								The new level, or NULL if the level couldn't added.
 */
ULevel* WxLevelBrowser::AddLevelToWorld(const TCHAR* LevelPackageBaseFilename)
{
	ULevel* NewLevel = NULL;

	if ( FLevelUtils::FindStreamingLevel( LevelPackageBaseFilename ) )
	{
		// Do nothing if the level already exists in the world.
		appMsgf( AMT_OK, *LocalizeUnrealEd("LevelAlreadyExistsInWorld") );
	}
	else
	{
		// Ensure the set of available level streaming methods is initialized.
		InitializeStreamingMethods();

		// Display a dialog prompting the user to choose streaming method for this level.
		const FString TitleText( FString::Printf( TEXT("%s - %s"), *LocalizeUnrealEd("SelectStreamingMethod"), LevelPackageBaseFilename ) );

		WxDlgGenericComboEntry dlg( TRUE );
		dlg.SetTitleAndCaption( *TitleText, *LocalizeUnrealEd("StreamingMethod:"), FALSE );
		dlg.PopulateComboBox( GStreamingMethodStrings );
		dlg.SetSelection( TEXT("Kismet") );

		if ( dlg.ShowModal() == wxID_OK )
		{
			const FScopedBusyCursor BusyCursor;

			// Use the selection index to look up the selected level streaming type.
			UClass* SelectedClass = GStreamingMethodClassList( dlg.GetComboBox().GetSelection() );
			ULevelStreaming* StreamingLevel = static_cast<ULevelStreaming*>( UObject::StaticConstructObject( SelectedClass, GWorld, NAME_None, 0, NULL) );

			// Associate a package name.
			StreamingLevel->PackageName = LevelPackageBaseFilename;

			// Seed the level's draw color.
			StreamingLevel->DrawColor = FColor::MakeRandomColor();

			// Add the new level to worldinfo.
			GWorld->GetWorldInfo()->StreamingLevels.AddItem( StreamingLevel );
			GWorld->GetWorldInfo()->PostEditChange( NULL );
			GWorld->MarkPackageDirty();

			LevelBrowser::SetLevelVisibility( StreamingLevel, TRUE );

			// Create a kismet sequence for this level if one does not already exist.
			NewLevel = StreamingLevel->LoadedLevel;
			if ( NewLevel )
			{
				USequence* ExistingSequence = NewLevel->GetGameSequence();
				if ( !ExistingSequence )
				{
					// The newly added level contains no sequence -- create a new one.
					USequence* NewSequence = ConstructObject<USequence>( USequence::StaticClass(), NewLevel, TEXT("Main_Sequence"), RF_Transactional );
					GWorld->SetGameSequence( NewSequence, NewLevel );
					NewSequence->MarkPackageDirty();

					// Fire CALLBACK_RefreshEditor_Kismet when falling out of scope.
					FScopedRefreshEditor_Kismet RefreshEditor_KismetCallback;
					RefreshEditor_KismetCallback.Request();
				}
			} 
		} // if streamingDlg.ok
	} // if world already exists

	return NewLevel;
}

/**
 * Removes the specified level from the world.  Refreshes.
 *
 * @return	TRUE	If a level was removed.
 */
UBOOL WxLevelBrowser::RemoveLevelFromWorld(ULevel* InLevel)
{
	// Disallow for cooked packages.
	if( GWorld && GWorld->GetOutermost()->PackageFlags & PKG_Cooked )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Error_OperationDisallowedOnCookedContent") );
		return FALSE;
	}

	const UBOOL bRemovingCurrentLevel	= InLevel && InLevel == GWorld->CurrentLevel;
	const UBOOL bRemoveSuccessful		= FLevelUtils::RemoveLevelFromWorld( InLevel );
	if ( bRemoveSuccessful )
	{
		// If the current level was removed, make the persistent level current.
		if( bRemovingCurrentLevel )
		{
			MakeLevelCurrent( GWorld->PersistentLevel );
		}

		// Do a full level browser redraw.
		RequestUpdate( TRUE );

		// Redraw the main editor viewports.
		GCallbackEvent->Send( CALLBACK_RedrawAllViewports );

		// refresh editor windows
		GCallbackEvent->Send( CALLBACK_RefreshEditor_AllBrowsers );
	}
	return bRemoveSuccessful;
}

/**
 * Makes the specified level the current level.  Refreshes.
 */
void WxLevelBrowser::MakeLevelCurrent(ULevel* InLevel)
{
	// If something is selected and not already current . . .
	if ( InLevel && InLevel != GWorld->CurrentLevel )
	{
		// Locked levels can't be made current.
		if ( !FLevelUtils::IsLevelLocked( InLevel ) )
		{
			// Make current.
			GWorld->CurrentLevel = InLevel;

			// Fire off an event
			GCallbackEvent->Send( CALLBACK_NewCurrentLevel );


			// Deselect all selected builder brushes.
			UBOOL bDeselectedSomething = FALSE;
			for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
			{
				AActor* Actor = static_cast<AActor*>( *It );
				checkSlow( Actor->IsA(AActor::StaticClass()) );

				if ( Actor->IsBrush() && Actor->IsABuilderBrush() )
				{
					GEditor->SelectActor( Actor, FALSE, NULL, FALSE );
					bDeselectedSomething = TRUE;
				}
			}

			// Send a selection change callback if necessary.
			if ( bDeselectedSomething )
			{
				GEditor->NoteSelectionChange();
			}

			// Force the level to be visible.
			LevelPane->SetLevelVisibility( GWorld->CurrentLevel, TRUE );

			// Refresh the level browser.
			RequestUpdate();
		}
	}
}

/**
 * Merges all visible levels into the persistent level at the top.  All included levels are then removed.
 */
void WxLevelBrowser::MergeVisibleLevels()
{
	// Disallow for a cooked GWorld or any visible levels that are cooked.
	UBOOL bFoundCookedPackage = ( GWorld->GetOutermost()->PackageFlags & PKG_Cooked ) ? TRUE : FALSE;
	AWorldInfo*	WorldInfo = GWorld->GetWorldInfo();
	for( INT LevelIndex = 0 ; LevelIndex < WorldInfo->StreamingLevels.Num() && !bFoundCookedPackage ; ++LevelIndex )
	{
		const ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
		if ( StreamingLevel && StreamingLevel->bShouldBeVisibleInEditor )
		{
			if ( StreamingLevel->LoadedLevel->GetOutermost()->PackageFlags & PKG_Cooked )
			{
				bFoundCookedPackage = TRUE;
			}
		}
	}

	if( bFoundCookedPackage )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Error_OperationDisallowedOnCookedContent") );
		return;
	}

	// Make the persistent level current
	MakeLevelCurrent( GWorld->PersistentLevel );

	// Move all actors from visible sublevels into the persistent level
	for( INT LevelIndex = 0 ; LevelIndex < WorldInfo->StreamingLevels.Num() ; ++LevelIndex )
	{
		ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);

		// If the level is visible, it needs to have all of its actors moved to the current one

		if ( StreamingLevel && StreamingLevel->bShouldBeVisibleInEditor )
		{
			for ( INT ActorIndex = 2 ; ActorIndex < StreamingLevel->LoadedLevel->Actors.Num() ; ++ActorIndex )
			{
				AActor* Actor = StreamingLevel->LoadedLevel->Actors( ActorIndex );
				if ( Actor )
				{
					GEditor->SelectActor( Actor, TRUE, NULL, FALSE, FALSE );
				}
			}

			GEditor->MoveSelectedActorsToCurrentLevel();
		}
	}

	// Remove all sublevels from the persistent level

	while( WorldInfo->StreamingLevels.Num() )
	{
		ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(0);
		if( StreamingLevel && StreamingLevel->LoadedLevel )
		{
			if ( RemoveLevelFromWorld( StreamingLevel->LoadedLevel ) )
			{
				DeselectLevel( StreamingLevel->LoadedLevel );
			}
		}
	}

	// Ask the user to save the flattened file

	GCallbackEvent->Send( CALLBACK_RedrawAllViewports );
	GCallbackEvent->Send( CALLBACK_RefreshEditor_LevelBrowser );

	// Prompt user to save newly merged level into a different filename

	FEditorFileUtils::SaveAs( GWorld );
}

/**
 * Selects all actors in the selected levels.
 */
void WxLevelBrowser::SelectActorsOfSelectedLevels()
{
	const FScopedBusyCursor BusyCursor;

	const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("SelectAllActors")) );
	GEditor->GetSelectedActors()->Modify();
	GEditor->SelectNone( FALSE, TRUE );

	for ( TSelectedLevelIterator It = SelectedLevelIterator() ; It ; ++It )
	{
		ULevel* Level = *It;

		ULevel* LevelToSelect = NULL;
		if ( Level == GWorld->PersistentLevel)
		{
			LevelToSelect = Level;
		}
		else
		{
			ULevelStreaming* StreamingLevel = FLevelUtils::FindStreamingLevel( Level ); 
			if ( StreamingLevel && StreamingLevel->LoadedLevel )
			{
				LevelToSelect = StreamingLevel->LoadedLevel;
			}
		}

		if ( LevelToSelect )
		{
			for ( INT ActorIndex = 2 ; ActorIndex < LevelToSelect->Actors.Num() ; ++ActorIndex )
			{
				AActor* Actor = LevelToSelect->Actors( ActorIndex );
				if ( Actor )
				{
					GEditor->SelectActor( Actor, TRUE, NULL, FALSE, FALSE );
				}
			}
		}
	}

	GEditor->NoteSelectionChange();
}

/**
 * Sync's window elements to the specified level.  Refreshes indirectly.
 */
void WxLevelBrowser::UpdateUIForLevel(ULevel* InLevel)
{
	LevelPane->UpdateUIForLevel( InLevel );
}

/**
 * Sets visibility for the selected levels.
 */
void WxLevelBrowser::SetSelectedLevelVisibility(UBOOL bVisible)
{
	const FScopedBusyCursor BusyCursor;
	for ( TSelectedLevelIterator It = SelectedLevelIterator() ; It ; ++It )
	{
		ULevel* Level = *It;
		LevelPane->SetLevelVisibility( Level, bVisible );
	}
	RequestUpdate();
}

/**
 * @param bShowSelected		If TRUE, show only selected levels; if FALSE show only unselected levels.
 */
void WxLevelBrowser::ShowOnlySelectedLevels(UBOOL bShowSelected)
{
	const FScopedBusyCursor BusyCursor;
	for ( WxLevelPane::TLevelWindowIterator It( LevelPane->LevelWindowIterator() ) ; It ; ++It )
	{
		WxLevelPane::WxLevelWindow* CurWindow = *It;
		ULevel* CurLevel = CurWindow->GetLevel();
		if ( IsSelected(CurLevel) )
		{
			CurWindow->SetLevelVisibility( bShowSelected );
		}
		else
		{
			CurWindow->SetLevelVisibility( !bShowSelected );
		}
	}
}

/**
 * Updates streaming level volume visibility status.  A streaming level volume is visible only if a level
 * is both visible and selected in the level browser.
 *
 * @param	bNoteSelectionChange		[opt] If TRUE (the default), call GEditor->NoteSelectionChange() if streaming volume selection changed.
 */
void WxLevelBrowser::UpdateStreamingLevelVolumeVisibility(UBOOL bNoteSelectionChange)
{
	if ( GWorld )
	{
		TArray<ALevelStreamingVolume*>	LevelStreamingVolumes;
		TArray<UBOOL>					States;

		AWorldInfo*	WorldInfo		= GWorld->GetWorldInfo();
		for( INT LevelIndex = 0 ; LevelIndex < WorldInfo->StreamingLevels.Num() ; ++LevelIndex )
		{
			ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
			if ( StreamingLevel )
			{
				const UBOOL bVolumesShouldBeVisible = FLevelUtils::IsLevelVisible( StreamingLevel ) && SelectedLevels.ContainsItem( StreamingLevel->LoadedLevel );
				for ( INT i = 0 ; i < StreamingLevel->EditorStreamingVolumes.Num() ; ++i )
				{
					ALevelStreamingVolume* LevelStreamingVolume = StreamingLevel->EditorStreamingVolumes(i);
					if ( LevelStreamingVolume == NULL )
					{
						// Take this opportunity to clear out null entries.
						StreamingLevel->EditorStreamingVolumes.Remove(i--);
					}
					else
					{
						const INT Index = LevelStreamingVolumes.FindItemIndex( LevelStreamingVolume );
						if ( Index == INDEX_NONE )
						{
							LevelStreamingVolumes.AddItem( LevelStreamingVolume );
							States.AddItem( bVolumesShouldBeVisible );
						}
						else
						{
							States(Index) |= bVolumesShouldBeVisible ;
						}
					}
				}
			}
		}

		for ( INT i = 0 ; i < LevelStreamingVolumes.Num() ; ++i )
		{
			ALevelStreamingVolume* LevelStreamingVolume = LevelStreamingVolumes(i);
			const UBOOL bShouldBeVisible = States(i);
			if ( bShouldBeVisible )
			{
				LevelStreamingVolume->bHiddenEd = FALSE;
			}
			else
			{
				LevelStreamingVolume->bHiddenEd = TRUE;
				GEditor->SelectActor( LevelStreamingVolume, FALSE, NULL, FALSE );
			}
		}
		if( bNoteSelectionChange )
		{
			GEditor->NoteSelectionChange();
		}
		GEditor->RedrawLevelEditingViewports();
	}
}

/**
 * Increases the mutex which controls whether Update() will be executed.  Should be called just before beginning an operation which might result in a
 * call to Update, if the calling code must manually call Update immediately afterwards.
 */
void WxLevelBrowser::DisableUpdate()
{
	SuppressUpdateMutex++;
}

/**
 * Decreases the mutex which controls whether Update() will be executed.
 */
void WxLevelBrowser::EnableUpdate()
{
	SuppressUpdateMutex--;
	check(SuppressUpdateMutex>=0);
}

namespace LevelBrowser {

/**
 * Shifts the specified level up or down in the level ordering, as determined by the bUp flag.
 *
 * @param	InLevel		Level to shift.
 * @param	bUp			TRUE to shift level up one position, FALSe to shife level down one position.
 * @return				TRUE if the shift occurred, FALSE if the shift did not occur.
 */
static UBOOL ShiftLevel(const ULevel* InLevel, UBOOL bUp)
{
	UBOOL bResult = FALSE;
	if ( InLevel )
	{
		INT PrevFoundLevelIndex = -1;
		INT FoundLevelIndex = -1;
		INT PostFoundLevelIndex = -1;
		AWorldInfo*	WorldInfo = GWorld->GetWorldInfo();
		for( INT LevelIndex = 0 ; LevelIndex < WorldInfo->StreamingLevels.Num() ; ++LevelIndex )
		{
			ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
			if( StreamingLevel && StreamingLevel->LoadedLevel )
			{
				if ( FoundLevelIndex > -1 )
				{
					// Mark the first valid index after the found level and stop searching.
					PostFoundLevelIndex = LevelIndex;
					break;
				}
				else
				{
					if ( StreamingLevel->LoadedLevel == InLevel )
					{
						// We've found the level.
						FoundLevelIndex = LevelIndex;
					}
					else
					{
						// Mark this level as being the index before the found level.
						PrevFoundLevelIndex = LevelIndex;
					}
				}
			}
		}

		// If we found the level . . .
		if ( FoundLevelIndex > -1 )
		{
			// Check if we found a destination index to swap it to.
			const INT DestIndex = bUp ? PrevFoundLevelIndex : PostFoundLevelIndex;
			const UBOOL bFoundPrePost = DestIndex > -1;
			if ( bFoundPrePost )
			{
				// Swap the level into position.
				const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("ShiftLevelInLevelBrowser")) );
				WorldInfo->Modify();
				WorldInfo->StreamingLevels.SwapItems( FoundLevelIndex, DestIndex );
				WorldInfo->MarkPackageDirty();
				bResult = TRUE;
			}
		}
	}
	return bResult;
}

} // namespace LevelBrowser

/**
 * Helper function for shifting level ordering; if a single level is selected, the
 * level is shifted up or down one position in the WorldInfo streaming level ordering,
 * depending on the value of the bUp argument.
 *
 * @param	bUp		TRUE to shift level up one position, FALSe to shift level down one position.
 */
void WxLevelBrowser::ShiftSingleSelectedLevel(UBOOL bUp)
{
	// Shift a singly selected level the requested direction.
	ULevel* SingleSelectedLevel = GetSingleSelectedLevel();
	const UBOOL bShiftOccurred = LevelBrowser::ShiftLevel( SingleSelectedLevel, bUp );

	// Redraw window contents if necessary.
	if ( bShiftOccurred )
	{
		Update();
		// Make sure the level is still selected.
		SelectLevel( SingleSelectedLevel );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Wx events.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when the user selects the "New Level" option from the level browser's file menu.
 */
void WxLevelBrowser::OnNewLevel(wxCommandEvent& In)
{
	CreateNewLevel( FALSE );
}

/**
 * Called when the user selects the "New Level From Selected Actors" option from the level browser's file menu.
 */
void WxLevelBrowser::OnNewLevelFromSelectedActors(wxCommandEvent& In)
{
	CreateNewLevel( TRUE );
}

/**
 * Called when the user selects the "Import Level" option from the level browser's file menu.
 */
void WxLevelBrowser::OnImportLevel(wxCommandEvent& In)
{
	ImportLevelsFromFile();
}

void WxLevelBrowser::OnRemoveLevelFromWorld(wxCommandEvent& In)
{
	ULevel* SelectedLevel = GetSelectedLevel();
	if ( RemoveLevelFromWorld( SelectedLevel ) )
	{
		DeselectLevel( SelectedLevel );
	}
}

void WxLevelBrowser::OnMakeLevelCurrent(wxCommandEvent& In)
{
	MakeLevelCurrent( GetSelectedLevel() );
}

void WxLevelBrowser::OnMergeVisibleLevels(wxCommandEvent& In)
{
	MergeVisibleLevels();
}

void WxLevelBrowser::OnSaveSelectedLevels(wxCommandEvent& In)
{
	// NOTE: We'll build a list of levels to save here.  We don't want to use the SelectedLevels member
	//   since that list will be reset when Serialize is called
	TArray< ULevel* > LevelsToSave;
	FString NamesOfLevelsNotSaved;
	for ( TSelectedLevelIterator It = SelectedLevelIterator() ; It ; ++It )
	{
		ULevel* Level = *It;
		if ( FLevelUtils::IsLevelVisible( Level ) )
		{
			LevelsToSave.AddItem( Level );
		}
		else
		{
			ULevelStreaming* StreamingLevel = FLevelUtils::FindStreamingLevel( Level ); 
			if ( StreamingLevel )
			{
				NamesOfLevelsNotSaved += *StreamingLevel->PackageName.ToString();
			}
			else
			{
				NamesOfLevelsNotSaved += *Level->GetName();
			}
			NamesOfLevelsNotSaved += '\n';
		}
	}

	for ( TSelectedLevelIterator It = TSelectedLevelIterator( LevelsToSave ) ; It ; ++It )
	{
		ULevel* Level = *It;
		FEditorFileUtils::SaveLevel( Level );
	}

	if( NamesOfLevelsNotSaved.Len() > 0 )
	{
		appMsgf( AMT_OK, *FString::Printf( LocalizeSecure( LocalizeUnrealEd( "UnableToSaveSomeInvisibleLevels_F" ), *NamesOfLevelsNotSaved ) ) );
	}

	// Select the levels that were saved successfully
	DeselectAllLevels();
	for ( TSelectedLevelIterator It = TSelectedLevelIterator( LevelsToSave ) ; It ; ++It )
	{
		ULevel* Level = *It;
		SelectLevel( Level );
	}

	RequestUpdate();
}

void WxLevelBrowser::ShowSelectedLevelsInSceneManager(wxCommandEvent& In)
{
	WxSceneManager* SceneManager = GUnrealEd->GetBrowser<WxSceneManager>( TEXT("SceneManager") );
	if ( SceneManager )
	{
		SceneManager->SetActiveLevels( SelectedLevels );
		GUnrealEd->GetBrowserManager()->ShowWindow( SceneManager->GetDockID(), TRUE );
	}
}

/**
 * Selects all actors in the selected levels.
 */
void WxLevelBrowser::OnSelectAllActors(wxCommandEvent& In)
{
	SelectActorsOfSelectedLevels();
}

void WxLevelBrowser::ShiftLevelUp(wxCommandEvent& In)
{
	ShiftSingleSelectedLevel( TRUE );
}

void WxLevelBrowser::ShiftLevelDown(wxCommandEvent& In)
{
	ShiftSingleSelectedLevel( FALSE );
}

/**
 * Shows the selected level in a property window.
 */
void WxLevelBrowser::OnLevelProperties(wxCommandEvent& In)
{
	ShowSelectedLevelPropertyWindow();
}

/**
 * Shows the selected level in a property window.
 */
void WxLevelBrowser::OnToggleSelfContainedLighting(wxCommandEvent& In)
{
	ULevel* SingleSelectedLevel = GetSingleSelectedLevel();
	if ( SingleSelectedLevel )
	{
		const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("SelfContainedLighting")) );

		GWorld->ModifyLevel( SingleSelectedLevel );
		SingleSelectedLevel->MarkPackageDirty();

		const UBOOL bHasSelfContainedLighting = FLevelUtils::HasSelfContainedLighting( SingleSelectedLevel );
		FLevelUtils::SetSelfContainedLighting( SingleSelectedLevel, !bHasSelfContainedLighting );
		SelectSingleLevel( SingleSelectedLevel );
		if( FLevelUtils::IsLevelVisible( SingleSelectedLevel ) )
		{
			SingleSelectedLevel->UpdateComponents();
		}
	}
}

void WxLevelBrowser::OnShowOnlySelectedLevels(wxCommandEvent& In)
{
	ShowOnlySelectedLevels( TRUE );
}

void WxLevelBrowser::OnShowOnlyUnselectedLevels(wxCommandEvent& In)
{
	ShowOnlySelectedLevels( FALSE );
}

void WxLevelBrowser::OnShowSelectedLevels(wxCommandEvent& In)
{
	SetSelectedLevelVisibility( TRUE );
}

void WxLevelBrowser::OnHideSelectedLevels(wxCommandEvent& In)
{
	SetSelectedLevelVisibility( FALSE );
}

void WxLevelBrowser::OnShowAllLevels(wxCommandEvent& In)
{
	LevelPane->SetAllLevelVisibility( TRUE );
}

void WxLevelBrowser::OnHideAllLevels(wxCommandEvent& In)
{
	LevelPane->SetAllLevelVisibility( FALSE );
}

void WxLevelBrowser::OnSelectAllLevels(wxCommandEvent& In)
{
	SelectAllLevels();
}

void WxLevelBrowser::OnDeselectAllLevels(wxCommandEvent& In)
{
	DeselectAllLevels();
}

void WxLevelBrowser::OnInvertSelection(wxCommandEvent& In)
{
	InvertLevelSelection();
}

void WxLevelBrowser::OnMoveSelectedActorsToCurrentLevel(wxCommandEvent& In)
{
	GEditor->MoveSelectedActorsToCurrentLevel();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Iterates over selected levels and sets streaming volume associations.
void WxLevelBrowser::SetStreamingLevelVolumes(const TArray<ALevelStreamingVolume*>& LevelStreamingVolumes)
{
	ClearStreamingLevelVolumes();
	AddStreamingLevelVolumes( LevelStreamingVolumes );
}

// Iterates over selected levels and adds streaming volume associations.
void WxLevelBrowser::AddStreamingLevelVolumes(const TArray<ALevelStreamingVolume*>& LevelStreamingVolumes)
{
	const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("AddStreamingVolumes")) );

	for ( TSelectedLevelIterator It = SelectedLevelIterator() ; It ; ++It )
	{
		ULevelStreaming* StreamingLevel = FLevelUtils::FindStreamingLevel( *It ); 
		if ( StreamingLevel )
		{
			StreamingLevel->Modify();
			for ( INT i = 0 ; i < LevelStreamingVolumes.Num() ; ++i )
			{
				ALevelStreamingVolume* LevelStreamingVolume = LevelStreamingVolumes(i);

				// Associate the level to the volume.
				LevelStreamingVolume->Modify();
				LevelStreamingVolume->StreamingLevels.AddUniqueItem( StreamingLevel );

				// Associate the volume to the level.
				StreamingLevel->EditorStreamingVolumes.AddUniqueItem( LevelStreamingVolume );
			}
		}
	}
}

// Iterates over selected levels and clears all streaming volume associations.
void WxLevelBrowser::ClearStreamingLevelVolumes()
{
	const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("ClearStreamingVolumes")) );

	for ( TSelectedLevelIterator It = SelectedLevelIterator() ; It ; ++It )
	{
		ULevelStreaming* StreamingLevel = FLevelUtils::FindStreamingLevel( *It ); 
		if ( StreamingLevel )
		{
			StreamingLevel->Modify();

			// Disassociate the level from the volume.
			for ( INT i = 0 ; i < StreamingLevel->EditorStreamingVolumes.Num() ; ++i )
			{
				ALevelStreamingVolume* LevelStreamingVolume = StreamingLevel->EditorStreamingVolumes(i);
				if ( LevelStreamingVolume )
				{
					LevelStreamingVolume->Modify();
					LevelStreamingVolume->StreamingLevels.RemoveItem( StreamingLevel );
				}
			}

			// Disassociate the volumes from the level.
			StreamingLevel->EditorStreamingVolumes.Empty();
		}
	}
}

// Iterates over selected levels and selects all streaming volume associations.
void WxLevelBrowser::SelectStreamingLevelVolumes()
{
	// Iterate over selected levels and make a list of volumes to select.
	TArray<ALevelStreamingVolume*> LevelStreamingVolumesToSelect;
	for ( TSelectedLevelIterator It = SelectedLevelIterator() ; It ; ++It )
	{
		ULevelStreaming* StreamingLevel = FLevelUtils::FindStreamingLevel( *It ); 
		if ( StreamingLevel )
		{
			for ( INT i = 0 ; i < StreamingLevel->EditorStreamingVolumes.Num() ; ++i )
			{
				ALevelStreamingVolume* LevelStreamingVolume = StreamingLevel->EditorStreamingVolumes(i);
				if ( LevelStreamingVolume )
				{
					LevelStreamingVolumesToSelect.AddItem( LevelStreamingVolume );
				}
			}
		}
	}

	// Select the volumes.
	const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("SelectAssociatedStreamingVolumes")) );
	GEditor->GetSelectedActors()->Modify();
	GEditor->SelectNone( FALSE, TRUE );

	for ( INT i = 0 ; i < LevelStreamingVolumesToSelect.Num() ; ++i )
	{
		ALevelStreamingVolume* LevelStreamingVolume = LevelStreamingVolumesToSelect(i);
		GEditor->SelectActor( LevelStreamingVolume, TRUE, NULL, FALSE, TRUE );
	}


	GEditor->NoteSelectionChange();
}

static void AssembleSelectedLevelStreamingVolumes(TArray<ALevelStreamingVolume*>& LevelStreamingVolumes)
{
	LevelStreamingVolumes.Empty();
	for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		ALevelStreamingVolume* StreamingVolume = Cast<ALevelStreamingVolume>( *It );
		if ( StreamingVolume && StreamingVolume->GetLevel() == GWorld->PersistentLevel )
		{
			LevelStreamingVolumes.AddItem( StreamingVolume );
		}
	}

	if ( LevelStreamingVolumes.Num() == 0 )
	{
		appMsgf( AMT_OK, TEXT("Streaming volume assignment failed.  No LevelStreamingVolumes in the persistent level were selected.") );
	}
}
void WxLevelBrowser::OnSetStreamingLevelVolumes(wxCommandEvent& In)
{
	TArray<ALevelStreamingVolume*> LevelStreamingVolumes;
	AssembleSelectedLevelStreamingVolumes( LevelStreamingVolumes );
	if ( LevelStreamingVolumes.Num() > 0 )
	{
		SetStreamingLevelVolumes( LevelStreamingVolumes );
		UpdateLevelPropertyWindow();
		UpdateStreamingLevelVolumeVisibility();
	}
}

void WxLevelBrowser::OnAddStreamingLevelVolumes(wxCommandEvent& In)
{
	TArray<ALevelStreamingVolume*> LevelStreamingVolumes;
	AssembleSelectedLevelStreamingVolumes( LevelStreamingVolumes );
	if ( LevelStreamingVolumes.Num() > 0 )
	{
		AddStreamingLevelVolumes( LevelStreamingVolumes );
		UpdateLevelPropertyWindow();
		UpdateStreamingLevelVolumeVisibility();
	}
}

void WxLevelBrowser::OnClearStreamingLevelVolumes(wxCommandEvent& In)
{
	ClearStreamingLevelVolumes();
	UpdateLevelPropertyWindow();
	UpdateStreamingLevelVolumeVisibility();
}

void WxLevelBrowser::OnSelectStreamingLevelVolumes(wxCommandEvent& In)
{
	SelectStreamingLevelVolumes();
	UpdateLevelPropertyWindow();
	UpdateStreamingLevelVolumeVisibility();
}
