/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "DlgGeometryTools.h"


/*-----------------------------------------------------------------------------
WxModeGeometrySelBar.
-----------------------------------------------------------------------------*/

class WxModeGeometrySelBar : public wxToolBar
{
public:
	WxModeGeometrySelBar( wxWindow* InParent, wxWindowID InID )
		: wxToolBar( InParent, InID, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_FLAT | wxTB_NODIVIDER )
	{
		// Bitmaps

		WindowB.Load( TEXT("Window") );
		ObjectB.Load( TEXT("Geom_Object") );
		PolyB.Load( TEXT("Geom_Poly") );
		EdgeB.Load( TEXT("Geom_Edge") );
		VertexB.Load( TEXT("Geom_Vertex") );

		// Set up the ToolBar

		AddCheckTool( ID_TOGGLE_WINDOW, TEXT(""), WindowB, WindowB, *LocalizeUnrealEd("ToolTip_67") );
		AddSeparator();
		AddRadioTool( ID_GEOM_OBJECT, TEXT(""), ObjectB, ObjectB, *LocalizeUnrealEd("Object") );
		AddRadioTool( ID_GEOM_POLY, TEXT(""), PolyB, PolyB, *LocalizeUnrealEd("Poly") );
		AddRadioTool( ID_GEOM_EDGE, TEXT(""), EdgeB, EdgeB, *LocalizeUnrealEd("Edge") );
		AddRadioTool( ID_GEOM_VERTEX, TEXT(""), VertexB, VertexB, *LocalizeUnrealEd("Vertex") );

		Realize();
	}

private:
	WxMaskedBitmap WindowB, ObjectB, PolyB, EdgeB, VertexB;

	//DECLARE_EVENT_TABLE()
};


// WxDlgGeometry Tools Event Table
BEGIN_EVENT_TABLE(WxDlgGeometryTools, wxDialog)
	EVT_TOOL( ID_TOGGLE_WINDOW,			WxDlgGeometryTools::OnToggleWindow )
	EVT_TOOL( ID_GEOM_OBJECT,			WxDlgGeometryTools::OnSelObject )
	EVT_TOOL( ID_GEOM_POLY,				WxDlgGeometryTools::OnSelPoly )
	EVT_TOOL( ID_GEOM_EDGE,				WxDlgGeometryTools::OnSelEdge )
	EVT_TOOL( ID_GEOM_VERTEX,			WxDlgGeometryTools::OnSelVertex )
	EVT_UPDATE_UI( ID_TOGGLE_WINDOW,	WxDlgGeometryTools::UI_ToggleWindow )
	EVT_UPDATE_UI( ID_GEOM_OBJECT,		WxDlgGeometryTools::UI_SelObject )
	EVT_UPDATE_UI( ID_GEOM_POLY,		WxDlgGeometryTools::UI_SelPoly )
	EVT_UPDATE_UI( ID_GEOM_EDGE,		WxDlgGeometryTools::UI_SelEdge )
	EVT_UPDATE_UI( ID_GEOM_VERTEX,		WxDlgGeometryTools::UI_SelVertex )

	EVT_CHECKBOX( IDCK_SHOW_NORMALS,		WxDlgGeometryTools::OnShowNormals )
	EVT_CHECKBOX( IDCK_WIDGET_ONLY,		WxDlgGeometryTools::OnAffectWidgetOnly )
	EVT_CHECKBOX( IDCK_SOFT_SELECTION,	WxDlgGeometryTools::OnUseSoftSelection )
	EVT_TEXT( IDEC_SS_RADIUS,		WxDlgGeometryTools::OnSoftSelectionRadius )
	EVT_UPDATE_UI( IDCK_SHOW_NORMALS,	WxDlgGeometryTools::UI_ShowNormals )
	EVT_UPDATE_UI( IDCK_WIDGET_ONLY,	WxDlgGeometryTools::UI_AffectWidgetOnly )
	EVT_UPDATE_UI( IDCK_SOFT_SELECTION, WxDlgGeometryTools::UI_UseSoftSelection )
	EVT_UPDATE_UI( IDEC_SS_RADIUS,		WxDlgGeometryTools::UI_SoftSelectionRadius )
	EVT_UPDATE_UI( IDSC_SS_RADIUS,		WxDlgGeometryTools::UI_SoftSelectionLabel )
END_EVENT_TABLE()

/**
* UnrealEd dialog with various geometry mode related tools on it.
*/
WxDlgGeometryTools::WxDlgGeometryTools(wxWindow* InParent) : 
wxDialog(InParent, wxID_ANY, TEXT("DlgGeometryToolsTitle"), wxDefaultPosition, wxDefaultSize, wxCAPTION )
{
	wxBoxSizer* VerticalSizer = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticBoxSizer* ToolBarSizer = new wxStaticBoxSizer(wxVERTICAL, this, TEXT("SelectionMode"));
		{
			ToolBar = new WxModeGeometrySelBar( this, -1 );
			ToolBarSizer->Add(ToolBar, 1, wxEXPAND | wxALL, 5);
		}
		VerticalSizer->Add(ToolBarSizer, 0, wxALL | wxEXPAND, 5);

		wxStaticBoxSizer* SoftSelectSizer = new wxStaticBoxSizer(wxVERTICAL, this, TEXT("SoftSelection"));
		{
			UseSoftSelectionCheck = new wxCheckBox( this, IDCK_SOFT_SELECTION, TEXT("EnabledQ"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
			UseSoftSelectionCheck->SetValue(false);
			SoftSelectSizer->Add(UseSoftSelectionCheck, 1, wxALIGN_LEFT);

			
			wxBoxSizer* HorizontalSizer = new wxBoxSizer(wxHORIZONTAL);
			{
				
				SoftSelectionLabel = new wxStaticText( this, IDSC_SS_RADIUS, TEXT("Radius"), wxDefaultPosition, wxDefaultSize, 0 );
				HorizontalSizer->Add(SoftSelectionLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL | wxADJUST_MINSIZE, 5);

				SoftSelectionRadiusText = new wxTextCtrl( this, IDEC_SS_RADIUS, TEXT(""), wxDefaultPosition, wxDefaultSize, 0 );
				HorizontalSizer->Add(SoftSelectionRadiusText, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
				
			}
			SoftSelectSizer->Add(HorizontalSizer, 0);
		}
		VerticalSizer->Add(SoftSelectSizer, 0, wxALL, 5);

		wxStaticBoxSizer* OptionsSizer = new wxStaticBoxSizer(wxVERTICAL, this, TEXT("Options"));
		{
			ShowNormalsCheck= new wxCheckBox( this, IDCK_SHOW_NORMALS, TEXT("ShowNormalsQ"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
			ShowNormalsCheck->SetValue(false);
			OptionsSizer->Add(ShowNormalsCheck, 1, wxALIGN_LEFT | wxALL, 5);

			AffectWidgetOnlyCheck = new wxCheckBox( this, IDCK_WIDGET_ONLY, TEXT("AffectWidgetOnlyQ"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
			AffectWidgetOnlyCheck->SetValue(false);
			OptionsSizer->Add(AffectWidgetOnlyCheck, 1, wxALIGN_LEFT | wxALL, 5);
		}
		VerticalSizer->Add(OptionsSizer, 0, wxALL | wxEXPAND, 5);
	}
	SetSizer(VerticalSizer);

	
	VerticalSizer->Fit(this);
	
	FWindowUtil::LoadPosSize( TEXT("DlgGeometryTools"), this );
	FLocalizeWindow( this );

	Show ( FALSE );
}

WxDlgGeometryTools::~WxDlgGeometryTools()
{
	FWindowUtil::SavePosSize( TEXT("DlgGeometryTools"), this );
}

/**
* Toggles the geometry mode modifers window.
*
* @param In	wxCommandEvent object with information about the event.
*/
void WxDlgGeometryTools::OnToggleWindow( wxCommandEvent& In )
{
	if( GEditorModeTools().GetCurrentModeID() != EM_Geometry )
	{
		return;
	}

	FGeometryToolSettings* settings = (FGeometryToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();
	settings->bShowModifierWindow = In.IsChecked();

	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
	mode->ShowModifierWindow( settings->bShowModifierWindow );

	GCallbackEvent->Send( CALLBACK_RedrawAllViewports );
}

/**
* Select Toolbar - Sets selection mode to objects.
*
* @param In	wxCommandEvent object with information about the event.
*/
void WxDlgGeometryTools::OnSelObject( wxCommandEvent& In )
{
	FGeometryToolSettings* settings = (FGeometryToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();
	settings->SelectionType = GST_Object;
	GCallbackEvent->Send( CALLBACK_RedrawAllViewports );
}

/**
* Select Toolbar - Sets selection mode to polygons.
*
* @param In	wxCommandEvent object with information about the event.
*/
void WxDlgGeometryTools::OnSelPoly( wxCommandEvent& In )
{
	FGeometryToolSettings* settings = (FGeometryToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();
	settings->SelectionType = GST_Poly;
	GCallbackEvent->Send( CALLBACK_RedrawAllViewports );
}

/**
* Select Toolbar - Sets selection mode to edges.
*
* @param In	wxCommandEvent object with information about the event.
*/
void WxDlgGeometryTools::OnSelEdge( wxCommandEvent& In )
{
	FGeometryToolSettings* settings = (FGeometryToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();
	settings->SelectionType = GST_Edge;
	GCallbackEvent->Send( CALLBACK_RedrawAllViewports );
}

/**
* Select Toolbar - Sets selection mode to vertices.
*
* @param In	wxCommandEvent object with information about the event.
*/
void WxDlgGeometryTools::OnSelVertex( wxCommandEvent& In )
{
	FGeometryToolSettings* settings = (FGeometryToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();
	settings->SelectionType = GST_Vertex;
	GCallbackEvent->Send( CALLBACK_RedrawAllViewports );
}


/**
* Updates the wxWidgets ToggleWindow check button object.
*
* @param In	wxUpdateUIEvent object with information about the event.
*/
void WxDlgGeometryTools::UI_ToggleWindow( wxUpdateUIEvent& In )
{
	FGeometryToolSettings* settings = (FGeometryToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();
	if( settings )
	{
		In.Check( settings->bShowModifierWindow == TRUE );
	}
}

/**
* Updates the wxWidgets SelObject radio button object.
*
* @param In	wxUpdateUIEvent object with information about the event.
*/
void WxDlgGeometryTools::UI_SelObject( wxUpdateUIEvent& In )
{
	FGeometryToolSettings* settings = (FGeometryToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();
	if( settings )
	{
		In.Check( settings->SelectionType == GST_Object );
	}
}

/**
* Updates the wxWidgets SelPoly radio button object.
*
* @param In	wxUpdateUIEvent object with information about the event.
*/
void WxDlgGeometryTools::UI_SelPoly( wxUpdateUIEvent& In )
{
	FGeometryToolSettings* settings = (FGeometryToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();
	if( settings )
	{
		In.Check( settings->SelectionType == GST_Poly );
	}
}

/**
* Updates the wxWidgets SelEdge radio button object.
*
* @param In	wxUpdateUIEvent object with information about the event.
*/
void WxDlgGeometryTools::UI_SelEdge( wxUpdateUIEvent& In )
{
	FGeometryToolSettings* settings = (FGeometryToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();
	if( settings )
	{
		In.Check( settings->SelectionType == GST_Edge );
	}
}

/**
* Updates the wxWidgets SelVertex radio button object.
*
* @param In	wxUpdateUIEvent object with information about the event.
*/
void WxDlgGeometryTools::UI_SelVertex( wxUpdateUIEvent& In )
{
	FGeometryToolSettings* settings = (FGeometryToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();
	if( settings )
	{
		In.Check( settings->SelectionType == GST_Vertex );
	}
}

/**
* Toggles the displaying of normals.
*
* @param In	wxCommandEvent object with information about the event.
*/
void WxDlgGeometryTools::OnShowNormals( wxCommandEvent& In )
{
	FGeometryToolSettings* settings = (FGeometryToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();
	settings->bShowNormals = In.IsChecked();
	GCallbackEvent->Send( CALLBACK_RedrawAllViewports );
}

/**
* Toggles whether or not to only affect widgets.
*
* @param In	wxCommandEvent object with information about the event.
*/
void WxDlgGeometryTools::OnAffectWidgetOnly( wxCommandEvent& In )
{
	FGeometryToolSettings* settings = (FGeometryToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();
	settings->bAffectWidgetOnly = In.IsChecked();
	GCallbackEvent->Send( CALLBACK_RedrawAllViewports );
}

/**
* Toggles whether or not soft selection mode is enabled.
*
* @param In	wxCommandEvent object with information about the event.
*/
void WxDlgGeometryTools::OnUseSoftSelection( wxCommandEvent& In )
{
	FGeometryToolSettings* settings = (FGeometryToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();
	settings->bUseSoftSelection = In.IsChecked();

	if( settings->bUseSoftSelection )
	{
		GEditorModeTools().GetCurrentMode()->SoftSelect();
	}
	else
	{
		GEditorModeTools().GetCurrentMode()->SoftSelectClear();
	}

	GCallbackEvent->Send( CALLBACK_RedrawAllViewports );
}

/**
* Called when the radius text box changes, updates the soft selection radius.
*
* @param In	wxCommandEvent object with information about the event.
*/
void WxDlgGeometryTools::OnSoftSelectionRadius( wxCommandEvent& In )
{
	FGeometryToolSettings* settings = (FGeometryToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();
	settings->SoftSelectionRadius = appAtoi( In.GetString().c_str() );
	GEditorModeTools().GetCurrentMode()->SoftSelect();
	GCallbackEvent->Send( CALLBACK_RedrawAllViewports );
}

/**
* Updates the wxWidgets ShowNormals check box object.
*
* @param In	wxUpdateUIEvent object with information about the event.
*/
void WxDlgGeometryTools::UI_ShowNormals( wxUpdateUIEvent& In )
{
	if( GEditorModeTools().GetCurrentModeID() != EM_Geometry )
	{
		return;
	}

	FGeometryToolSettings* settings = (FGeometryToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();
	In.Check( settings->bShowNormals == TRUE );
}

/**
* Updates the wxWidgets AffectWidgetOnly check box object.
*
* @param In	wxUpdateUIEvent object with information about the event.
*/
void WxDlgGeometryTools::UI_AffectWidgetOnly( wxUpdateUIEvent& In )
{
	if( GEditorModeTools().GetCurrentModeID() != EM_Geometry )
	{
		return;
	}

	FGeometryToolSettings* settings = (FGeometryToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();
	In.Check( settings->bAffectWidgetOnly == TRUE );
}

/**
* Updates the wxWidgets UseSoftSelection check box object.
*
* @param In	wxUpdateUIEvent object with information about the event.
*/
void WxDlgGeometryTools::UI_UseSoftSelection( wxUpdateUIEvent& In )
{
	if( GEditorModeTools().GetCurrentModeID() != EM_Geometry )
	{
		return;
	}

	FGeometryToolSettings* settings = (FGeometryToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();
	In.Check( settings->bUseSoftSelection == TRUE );
}

/**
* Updates the wxWidgets SoftSelectionRadius TextCtrl object.
*
* @param In	wxUpdateUIEvent object with information about the event.
*/
void WxDlgGeometryTools::UI_SoftSelectionRadius( wxUpdateUIEvent& In )
{
	if( GEditorModeTools().GetCurrentModeID() != EM_Geometry )
	{
		return;
	}

	FGeometryToolSettings* settings = (FGeometryToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();

	In.SetText( *appItoa( settings->SoftSelectionRadius ) );
	In.Enable( settings->bUseSoftSelection == TRUE );
}

/**
* Updates the wxWidgets SoftSelectionLabel label object.
*
* @param In	wxUpdateUIEvent object with information about the event.
*/
void WxDlgGeometryTools::UI_SoftSelectionLabel( wxUpdateUIEvent& In )
{
	if( GEditorModeTools().GetCurrentModeID() != EM_Geometry )
	{
		return;
	}

	FGeometryToolSettings* settings = (FGeometryToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();
	In.Enable( settings->bUseSoftSelection == TRUE );
}

