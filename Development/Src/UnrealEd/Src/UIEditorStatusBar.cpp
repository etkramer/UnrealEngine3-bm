/*=============================================================================
	UIEditorStatusBar.cpp: Status Bar Class for the UI Editor Frame
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "EngineUIPrivateClasses.h"
#include "UnrealEdPrivateClasses.h"
#include "UnObjectEditor.h"
#include "UnUIEditor.h"
#include "UIEditorStatusBar.h"


IMPLEMENT_DYNAMIC_CLASS(WxUIEditorStatusBar,wxStatusBar)

namespace UIEditorStatusBar
{
	enum EStatusBarFields
	{
		SBF_Help = 0,
		SBF_MouseDeltaTracker,
		SBF_MouseTracker,
		SBF_SelectedObjects,
		SBF_DragGrid,
		SBF_Buffer,

		SBF_Count	// Must be the last field.
	};
};

BEGIN_EVENT_TABLE(WxUIEditorStatusBar, WxStatusBar)
	EVT_SIZE(OnSize)
	EVT_COMMAND( IDCB_DRAG_GRID_TOGGLE, wxEVT_COMMAND_CHECKBOX_CLICKED, WxUIEditorStatusBar::OnDragGridToggleClick )
END_EVENT_TABLE()


/**
 *	WxUIEditorStatusBar - Status bar for the UI editor window.
 */
WxUIEditorStatusBar::WxUIEditorStatusBar() : 
Editor(NULL),
DragGridCB(NULL),
DragGridSB(NULL),
DragGridMB(NULL),
DragGridST(NULL)
{

}

/** 2nd part of the 2 step widget creation process. */
void WxUIEditorStatusBar::Create( WxUIEditorBase* InParent, wxWindowID InID, const FString& Name/* = TEXT("StatusBar")*/ )
{
	Editor = InParent;

	const bool bSuccess = wxStatusBar::Create((wxWindow*)InParent, InID);
	check( bSuccess );

	/** Create bitmaps. */
	wxImage TempDragGrid;
	TempDragGrid.LoadFile( *FString::Printf( TEXT("%swxres\\DragGrid.bmp"), appBaseDir() ), wxBITMAP_TYPE_BMP );
	DragGridB = wxBitmap(TempDragGrid);
	DragGridB.SetMask( new wxMask( DragGridB, wxColor(192,192,192) ) );

	/** Create a set of controls to let the user toggle snap to grid and the grid size from the status bar. */
	DragGridST = new wxStaticText( this, IDST_DRAG_GRID, TEXT("XXXX"), wxDefaultPosition, wxSize(-1,-1), wxST_NO_AUTORESIZE );
	DragGridCB = new wxCheckBox( this, IDCB_DRAG_GRID_TOGGLE, TEXT("") );
	DragGridSB = new wxStaticBitmap( this, IDSB_DRAG_GRID, DragGridB );
	DragGridMB = new WxMenuButton( this, IDPB_DRAG_GRID, &GApp->EditorFrame->DownArrowB, Editor->GetDragGridMenu() );
	DragGridCB->SetToolTip( *LocalizeUnrealEd("ToolTip_16") );
	DragGridMB->SetToolTip( *LocalizeUnrealEd("ToolTip_18") );
	DragTextWidth = DragGridST->GetRect().GetWidth();

	const INT DragGridPane = 2+ DragGridB.GetWidth() +2+ DragTextWidth +2+ DragGridCB->GetRect().GetWidth() +2+ DragGridMB->GetRect().GetWidth();	
	INT Widths[UIEditorStatusBar::SBF_Count] = {-1, 400, 150, 200, DragGridPane, 20};

	SetFieldsCount(UIEditorStatusBar::SBF_Count, Widths);
}

WxUIEditorStatusBar::~WxUIEditorStatusBar()
{

}

/** Called when the UI editor frame wants the status bar to update. */
void WxUIEditorStatusBar::UpdateUI()
{	
	// Snap to grid check box
	DragGridCB->SetValue( Editor->EditorOptions->bSnapToGrid == TRUE );

	// Grid size status text
	FString GridSizeLabel = FString::Printf( TEXT("%i"), Editor->EditorOptions->GridSize ) ;
	const UBOOL bGridSizeLabelChanged = (DragGridST->GetLabel() != *GridSizeLabel);

	if( bGridSizeLabelChanged )
	{
		DragGridST->SetLabel( *GridSizeLabel );
	}
}

/**
 * Updates the status bar to reflect which widgets are currently selected.
 *
 * @param SelectedWidgets		A array of the currently selected widgets.
 */
void WxUIEditorStatusBar::UpdateSelectedWidgets(const TArray<UUIObject*> &SelectedWidgets)
{
	// Update selected objects text.
	const INT NumWidgets = SelectedWidgets.Num();
	FString SelectedStr;

	// We display different text depending if 0, 1, or N items are selected.
	switch(NumWidgets)
	{
	case 0:
		{
			UUIScene* OwnerScene = Editor ? Editor->OwnerScene : NULL;
			if ( OwnerScene )
			{
				SelectedStr = OwnerScene->GetClass()->GetDescription() + TEXT(": ") + OwnerScene->GetTag().ToString();
			}
			else
			{
				SelectedStr = LocalizeUI(TEXT("UIEditor_WidgetsNoneSelected"));		
			}
		}
		break;

	case 1:
		{
			const UUIObject* Widget = SelectedWidgets(0);
			SelectedStr = Widget->GetClass()->GetDescription() + TEXT(": ") + Widget->GetTag().ToString();
		}
		break;

	default:
		{
			/** 
			 * Loop through and see if the widgets are of the same type,
			 * if so, then display a string like "2 UIButtons selected",
			 * If not, display a string like "2 widgets selected"
			 */

			UBOOL bHomogenousSet = TRUE;
			const UUIObject* MatchWidget = SelectedWidgets(0);
			const UClass* MatchClass = MatchWidget->GetClass();

			for(INT WidgetIdx = 1; WidgetIdx < NumWidgets; WidgetIdx++)
			{
				const UUIObject* Widget = SelectedWidgets(WidgetIdx);
				const UBOOL bSameClass = (Widget->GetClass() == MatchClass);

				if( bSameClass == FALSE)
				{
					bHomogenousSet = FALSE;
					break;
				}
			}

			if( bHomogenousSet == TRUE )
			{
				SelectedStr = FString::Printf(LocalizeSecure(LocalizeUI(TEXT("UIEditor_WidgetsSameSelected")), NumWidgets, *MatchClass->GetName()));
			}
			else
			{
				SelectedStr = FString::Printf(LocalizeSecure(LocalizeUI(TEXT("UIEditor_WidgetsDifferentSelected")), NumWidgets));
			}
		}
		break;
	}

	SetStatusText(*SelectedStr,UIEditorStatusBar::SBF_SelectedObjects);
}

/**
 * Updates the mouse delta tracker status field with new text.
 *
 * @param InText	New text for the mouse delta tracker field.
 */
void WxUIEditorStatusBar::UpdateMouseDelta(const TCHAR* InText)
{
	if(InText != NULL)
	{
		SetStatusText(InText, UIEditorStatusBar::SBF_MouseDeltaTracker);
	}
	else
	{
		SetStatusText(TEXT(""), UIEditorStatusBar::SBF_MouseDeltaTracker);
	}
}

/** 
 * Updates the mouse tracker status field with new text.
 *
 * @param	ViewportClient	the viewport client containing the viewport that the mouse is moving across
 * @param	MouseX/MouseY	the position of the mouse cursor, relative to the viewport's upper-left corner
 */
void WxUIEditorStatusBar::UpdateMousePosition( FEditorObjectViewportClient* ViewportClient, INT MouseX, INT MouseY )
{
	check(ViewportClient);

	FLOAT ViewportPosX;
	FLOAT ViewportPosY;
	ViewportClient->GetViewportPositionFromMousePosition(MouseX, MouseY, ViewportPosX, ViewportPosY);

	FString MousePosText = FString::Printf(TEXT("( X: %.2f, Y: %.2f )"), ViewportPosX, ViewportPosY);
	SetStatusText(*MousePosText, UIEditorStatusBar::SBF_MouseTracker);
}

/** Event handler for when the status bar is resized. */ 
void WxUIEditorStatusBar::OnSize(wxSizeEvent& event)
{
	if( DragGridSB )
	{
		wxRect Rect;

		GetFieldRect( UIEditorStatusBar::SBF_DragGrid, Rect );

		INT Left = Rect.x + 2;
		DragGridSB->SetSize( Left, Rect.y+2, DragGridB.GetWidth(), Rect.height-4 );
		Left += DragGridB.GetWidth() + 2;
		DragGridST->SetSize( Left, Rect.y+2, DragTextWidth, Rect.height-4 );
		Left += DragTextWidth + 2;
		DragGridCB->SetSize( Left, Rect.y+2, -1, Rect.height-4 );
		Left += DragGridCB->GetSize().GetWidth() + 2;
		DragGridMB->SetSize( Left, Rect.y+1, 13, Rect.height-2 );
	}
}



/** Event handler for when the user clicks on the drag grid toggle check box. */
void WxUIEditorStatusBar::OnDragGridToggleClick(wxCommandEvent& event)
{
	Editor->EditorOptions->bSnapToGrid = !Editor->EditorOptions->bSnapToGrid;
}

