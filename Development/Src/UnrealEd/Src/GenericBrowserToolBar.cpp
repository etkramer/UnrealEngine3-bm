/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "GenericBrowserToolBar.h"

/*-----------------------------------------------------------------------------
	WxGBViewComboBox.
-----------------------------------------------------------------------------*/

/**
 * The combobox that sits on the generic browser toolbar that controls the size of
 * objects in the generic browser.  Subclassed from wxComboBox so that mouse
 * wheeling can be deactivated for this box.
 */
class WxGBViewComboBox : public wxComboBox
{
private:
	void OnMouseWheel( wxMouseEvent& In ) {}

	friend class WxGBViewToolBar;
	WxGBViewComboBox(wxWindow* InParent, const wxString DummyChoices[])
		:	wxComboBox( InParent, ID_ZOOM_FACTOR, TEXT(""), wxDefaultPosition, wxSize(75,-1), 0, DummyChoices, wxCB_READONLY )
	{}

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE( WxGBViewComboBox, wxComboBox )
	EVT_MOUSEWHEEL( WxGBViewComboBox::OnMouseWheel )
END_EVENT_TABLE()

/*-----------------------------------------------------------------------------
	WxGBViewToolBar.
-----------------------------------------------------------------------------*/

WxGBViewToolBar::WxGBViewToolBar( wxWindow* InParent, wxWindowID InID )
	: wxToolBar( InParent, InID, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_3DBUTTONS )
{
	// Bitmaps

	ListB.Load( TEXT("ListView") );
	PreviewB.Load( TEXT("PreviewView") );
	ThumbnailB.Load( TEXT("ThumbnailView") );
	SphereB.Load( TEXT("Sphere") );
	CubeB.Load( TEXT("Cube") );
	CylinderB.Load( TEXT("Cylinder") );
	PlaneB.Load( TEXT("Plane") );
	SearchB.Load( TEXT("Search") );
	GroupByClassB.Load( TEXT("GroupByClass") );
	PSysRealTimeB.Load( TEXT("PsysThumbs") );
	ReimportB.Load( TEXT("Reimport") );
	UsageFilterB.Load(TEXT("UsageFilter"));

	ZoomST = new wxStaticText( this, -1, *LocalizeUnrealEd("View") );

	wxString DummyChoices[1];
	ViewCB = new WxGBViewComboBox( this, DummyChoices );

	ViewCB->Append( TEXT("300%") );
	ViewCB->Append( TEXT("200%") );
	ViewCB->Append( TEXT("100%") );
	ViewCB->Append( TEXT("50%") );
	ViewCB->Append( TEXT("25%") );
	ViewCB->Append( TEXT("10%") );
	ViewCB->Append( *LocalizeUnrealEd("64x64") );
	ViewCB->Append( *LocalizeUnrealEd("128x128") );
	ViewCB->Append( *LocalizeUnrealEd("256x256") );
	ViewCB->Append( *LocalizeUnrealEd("512x512") );
	ViewCB->Append( *LocalizeUnrealEd("1024x1024") );
	ViewCB->SetToolTip( *LocalizeUnrealEd("ThumbnailFormat") );

	ViewCB->SetSelection( 2 );

	NameFilterST = new wxStaticText( this, -1, *LocalizeUnrealEd("Filter") );
	NameFilterTC = new wxTextCtrl( this, ID_NAME_FILTER, TEXT(""), wxDefaultPosition, wxSize(100,-1) );//, wxTE_LEFT | wxTE_DONTWRAP );

	// Set up the ToolBar

	AddRadioTool( ID_VIEW_LIST, TEXT(""), ListB, ListB, *LocalizeUnrealEd("List") );
	AddRadioTool( ID_VIEW_PREVIEW, TEXT(""), PreviewB, PreviewB, *LocalizeUnrealEd("Preview") );
	AddRadioTool( ID_VIEW_THUMBNAIL, TEXT(""), ThumbnailB, ThumbnailB, *LocalizeUnrealEd("Thumbnails") );
	AddSeparator();
	AddCheckTool( IDM_GROUPBYCLASS, TEXT(""), GroupByClassB, GroupByClassB, *LocalizeUnrealEd("ToolTip_61") );
	AddSeparator();
	AddCheckTool( IDM_USAGEFILTER, TEXT(""), UsageFilterB, UsageFilterB, *LocalizeUnrealEd("ToolTip_UsageFilter") );
	AddSeparator();
	AddCheckTool( IDM_PSYSREALTIME, TEXT(""), PSysRealTimeB, PSysRealTimeB, *LocalizeUnrealEd("PSysRealTime") );
	AddSeparator();
	AddRadioTool( ID_PRIMTYPE_SPHERE, TEXT(""), SphereB, SphereB, *LocalizeUnrealEd("ToolTip_62") );
	AddRadioTool( ID_PRIMTYPE_CUBE, TEXT(""), CubeB, CubeB, *LocalizeUnrealEd("ToolTip_63") );
	AddRadioTool( ID_PRIMTYPE_CYLINDER, TEXT(""), CylinderB, CylinderB, *LocalizeUnrealEd("ToolTip_64") );
	AddRadioTool( ID_PRIMTYPE_PLANE, TEXT(""), PlaneB, PlaneB, *LocalizeUnrealEd("ToolTip_65") );
	AddSeparator();
	AddTool( IDM_SEARCH, TEXT(""), SearchB, *LocalizeUnrealEd("ToolTip_66") );
	AddSeparator();
	AddTool( IDM_ATOMIC_RESOURCE_REIMPORT, TEXT(""), ReimportB, *LocalizeUnrealEd("ToolTip_AtomicResourceImport") );
	AddSeparator();
	AddControl( ZoomST );
	AddControl( ViewCB );
	AddSeparator();
	AddControl( NameFilterST );
	AddControl( NameFilterTC );

	Realize();
}

/**
* Sets the text for the NameFilter TextCtrl.
*
* @param String to set the TextCtrl value to.
*
*/
void WxGBViewToolBar::SetNameFilterText( const TCHAR* Str )
{
	NameFilterTC->SetValue( Str );
}




