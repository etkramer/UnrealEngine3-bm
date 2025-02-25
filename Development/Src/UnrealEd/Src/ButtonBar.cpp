/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "UnrealEd.h"
#include "Properties.h"

extern class WxDlgAddSpecial* GDlgAddSpecial;

/*-----------------------------------------------------------------------------
	WxDlgBrushBuilder.
-----------------------------------------------------------------------------*/

class WxDlgBrushBuilder : public wxDialog
{
public:
	WxDlgBrushBuilder()
	{
		const bool bSuccess = wxXmlResource::Get()->LoadDialog( this, GApp->EditorFrame, TEXT("ID_DLG_BRUSH_BUILDER") );
		check( bSuccess );

		wxWindow* win = (wxWindow*)FindWindow( XRCID( "ID_PROPERTY_WINDOW" ) );
		check( win );

		// Attach our property window to the placeholder window
		wxRect rc = win->GetRect();
		rc.SetX( 0 );
		rc.SetY( 0 );
		PropertyWindow = new WxPropertyWindow;
		PropertyWindow->Create(win, GUnrealEd);
		PropertyWindow->SetSize( rc );

		// For Brush building dialogs, we want the ENTER key to build the brush, so we disallow the property
		// window from intercepting the ENTER key itself
		PropertyWindow->SetAllowEnterKeyToApplyChanges( FALSE );

		FWindowUtil::LoadPosSize( TEXT("DlgBrushBuilder"), this );
		FLocalizeWindow( this );
	}

	~WxDlgBrushBuilder()
	{
		FWindowUtil::SavePosSize( TEXT("DlgBrushBuilder"), this );
		delete PropertyWindow;
	}

	UBOOL Show( UBrushBuilder* InBrushBuilder, UBOOL bInShow = TRUE )
	{
		BrushBuilder = InBrushBuilder;
		PropertyWindow->SetObject( BrushBuilder, 1,0,0 );
		SetTitle( *FString::Printf( LocalizeSecure(LocalizeUnrealEd("BrushBuilderCaption"), *BrushBuilder->ToolTip) ) );

		const bool bShouldShow = (bInShow == TRUE);
		return wxDialog::Show( bShouldShow );
	}


private:
	UBrushBuilder* BrushBuilder;
	WxPropertyWindow* PropertyWindow;

	void OnOK( wxCommandEvent& In )
	{
		PropertyWindow->FinalizeValues();
		BrushBuilder->eventBuild();
		GEditorModeTools().GetCurrentMode()->MapChangeNotify();
	}

	void OnClose( wxCommandEvent& In )
	{
		Destroy();
	}

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WxDlgBrushBuilder, wxDialog)
	EVT_BUTTON( wxID_OK, WxDlgBrushBuilder::OnOK )
	EVT_BUTTON( wxID_CLOSE, WxDlgBrushBuilder::OnClose )
END_EVENT_TABLE()

/*-----------------------------------------------------------------------------
	WxButtonGroupButton.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxButtonGroupButton, wxEvtHandler )
END_EVENT_TABLE()

WxButtonGroupButton::WxButtonGroupButton()
{
	check(0);		// wrong ctor
}

WxButtonGroupButton::WxButtonGroupButton( wxWindow* parent, wxWindowID id, WxBitmap& InBitmap, EButtonType InButtonType, UClass* InClass, INT InStartMenuID )
	: wxEvtHandler()
{
	Button = new WxBitmapButton( parent, id, InBitmap );
	ButtonType = InButtonType;
	Class = InClass;
	StartMenuID = InStartMenuID;
	BrushBuilder = NULL;
}

WxButtonGroupButton::WxButtonGroupButton( wxWindow* parent, wxWindowID id, WxBitmap& InBitmapOff, WxBitmap& InBitmapOn, EButtonType InButtonType, UClass* InClass, INT InStartMenuID )
	: wxEvtHandler()
{
	Button = new WxBitmapCheckButton( parent, parent, id, &InBitmapOff, &InBitmapOn );
	ButtonType = InButtonType;
	Class = InClass;
	StartMenuID = InStartMenuID;
	BrushBuilder = NULL;
}

WxButtonGroupButton::~WxButtonGroupButton()
{
}

/** 
 * Serializes the BrushBuilder reference so it doesn't get garbage collected.
 *
 * @param Ar	FArchive to serialize with
 */
void WxButtonGroupButton::Serialize(FArchive& Ar)
{
	Ar << BrushBuilder;
}

/*-----------------------------------------------------------------------------
	WxButtonGroup.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxButtonGroup, wxWindow )
	EVT_SIZE( WxButtonGroup::OnSize )
	EVT_RIGHT_DOWN( WxButtonGroup::OnRightButtonDown )
	EVT_COMMAND_RANGE( IDM_VolumeClasses_START, IDM_VolumeClasses_END, wxEVT_COMMAND_MENU_SELECTED, WxButtonGroup::OnAddVolumeClass )
	EVT_BUTTON( IDM_DEFAULT, WxButtonGroup::OnModeCamera )
	EVT_BUTTON( IDM_GEOMETRY, WxButtonGroup::OnModeGeometry )
	EVT_BUTTON( IDM_TERRAIN, WxButtonGroup::OnModeTerrain )
	EVT_BUTTON( IDM_TEXTURE, WxButtonGroup::OnModeTexture )
	EVT_BUTTON( IDM_COVEREDIT, WxButtonGroup::OnModeCoverEdit )
	EVT_BUTTON( IDM_BRUSH_ADD, WxButtonGroup::OnBrushAdd )
	EVT_BUTTON( IDM_BRUSH_SUBTRACT, WxButtonGroup::OnBrushSubtract )
	EVT_BUTTON( IDM_BRUSH_INTERSECT, WxButtonGroup::OnBrushIntersect )
	EVT_BUTTON( IDM_BRUSH_DEINTERSECT, WxButtonGroup::OnBrushDeintersect )
	EVT_BUTTON( IDM_BRUSH_ADD_SPECIAL, WxButtonGroup::OnAddSpecial )
	EVT_BUTTON( IDM_BRUSH_ADD_VOLUME, WxButtonGroup::OnAddVolume )
	EVT_BUTTON( IDM_SELECT_SHOW, WxButtonGroup::OnSelectShow )
	EVT_BUTTON( IDM_SELECT_HIDE, WxButtonGroup::OnSelectHide )
	EVT_BUTTON( IDM_SELECT_INVERT, WxButtonGroup::OnSelectInvert )
	EVT_BUTTON( IDM_SHOW_ALL, WxButtonGroup::OnShowAll )
	EVT_COMMAND_RANGE( IDM_BrushBuilder_START, IDM_BrushBuilder_END, wxEVT_COMMAND_BUTTON_CLICKED, WxButtonGroup::OnBrushBuilder )
	EVT_PAINT( WxButtonGroup::OnPaint )
	EVT_LEFT_DOWN( WxButtonGroup::OnLeftButtonDown )
END_EVENT_TABLE()

WxButtonGroup::WxButtonGroup( wxWindow* InParent )
	: wxWindow( InParent, -1 )
{
	bExpanded = 1;
}

WxButtonGroup::~WxButtonGroup()
{
}

WxButtonGroupButton* WxButtonGroup::AddButton( INT InID, FString InBitmapFilename, FString InToolTip, EButtonType InButtonType, UClass* InClass, INT InStartMenuID )
{
	WxAlphaBitmap* bmp = new WxAlphaBitmap( InBitmapFilename, 0 );
	WxButtonGroupButton* bb = new WxButtonGroupButton( this, InID, *bmp, InButtonType, InClass, InStartMenuID );
	Buttons.AddItem( bb );
	bb->Button->SetToolTip( *InToolTip );

	return bb;
}

WxButtonGroupButton* WxButtonGroup::AddButtonChecked( INT InID, FString InBitmapFilename, FString InToolTip, EButtonType InButtonType, UClass* InClass, INT InStartMenuID )
{
	WxAlphaBitmap* bmp = new WxAlphaBitmap( InBitmapFilename, 0 );
	WxAlphaBitmap* bmpHi = new WxAlphaBitmap( InBitmapFilename, 1 );
	WxButtonGroupButton* bb = new WxButtonGroupButton( this, InID, *bmp, *bmpHi, InButtonType, InClass, InStartMenuID );
	Buttons.AddItem( bb );
	bb->Button->SetToolTip( *InToolTip );

	return bb;
}

void WxButtonGroup::OnSize( wxSizeEvent& InEvent )
{
	wxRect rc = GetClientRect();

	INT XPos = 0;
	INT YPos = WxButtonGroup::TITLEBAR_H;

	for( INT x = 0 ; x < Buttons.Num() ; ++x )
	{
		wxBitmapButton* bmb = Buttons(x)->Button;

		bmb->SetSize( XPos,YPos, WxButtonGroup::BUTTON_SZ,WxButtonGroup::BUTTON_SZ );

		XPos += WxButtonGroup::BUTTON_SZ;
		if( XPos > WxButtonGroup::BUTTON_SZ )
		{
			XPos = 0;
			YPos += WxButtonGroup::BUTTON_SZ;
		}
	}
}

INT WxButtonGroup::GetHeight()
{
	INT NumButtons = Buttons.Num();

	if( !NumButtons || !bExpanded )
	{
		return WxButtonGroup::TITLEBAR_H;
	}

	int ExpandedHeight = WxButtonGroup::TITLEBAR_H + (WxButtonGroup::BUTTON_SZ + (((NumButtons-1)/2)*WxButtonGroup::BUTTON_SZ));

	return ExpandedHeight;
}

void WxButtonGroup::OnModeCamera( wxCommandEvent& In )			{	GUnrealEd->Exec(TEXT("MODE CAMERAMOVE")); UpdateUI(); }
void WxButtonGroup::OnModeGeometry( wxCommandEvent& In )		{	GUnrealEd->Exec(TEXT("MODE GEOMETRY")); UpdateUI(); }
void WxButtonGroup::OnModeTexture( wxCommandEvent& In )			{	GUnrealEd->Exec(TEXT("MODE TEXTURE")); UpdateUI(); }
void WxButtonGroup::OnModeTerrain( wxCommandEvent& In )			{	GUnrealEd->Exec(TEXT("MODE TERRAINEDIT")); UpdateUI(); }
void WxButtonGroup::OnModeCoverEdit( wxCommandEvent& In )		{	GUnrealEd->Exec(TEXT("MODE COVEREDIT")); UpdateUI(); }
void WxButtonGroup::OnBrushAdd( wxCommandEvent& In )			{	GUnrealEd->Exec(TEXT("BRUSH ADD"));	}
void WxButtonGroup::OnBrushSubtract( wxCommandEvent& In )		{	GUnrealEd->Exec(TEXT("BRUSH SUBTRACT"));	}
void WxButtonGroup::OnBrushIntersect( wxCommandEvent& In )		{	GUnrealEd->Exec(TEXT("BRUSH FROM INTERSECTION"));	}
void WxButtonGroup::OnBrushDeintersect( wxCommandEvent& In )	{	GUnrealEd->Exec(TEXT("BRUSH FROM DEINTERSECTION"));	}
void WxButtonGroup::OnAddSpecial( wxCommandEvent& In )			{	GDlgAddSpecial->Show(); }
void WxButtonGroup::OnAddVolume( wxCommandEvent& In )			{	GUnrealEd->Exec(TEXT("FIXME"));	}
void WxButtonGroup::OnSelectShow( wxCommandEvent& In )			{	GUnrealEd->Exec(TEXT("ACTOR HIDE UNSELECTED"));	}
void WxButtonGroup::OnSelectHide( wxCommandEvent& In )			{	GUnrealEd->Exec(TEXT("ACTOR HIDE SELECTED"));	}
void WxButtonGroup::OnSelectInvert( wxCommandEvent& In )		{	GUnrealEd->Exec(TEXT("ACTOR SELECT INVERT"));	}
void WxButtonGroup::OnShowAll( wxCommandEvent& In )				{	GUnrealEd->Exec(TEXT("ACTOR UNHIDE ALL"));	}

void WxButtonGroup::OnRightButtonDown( wxMouseEvent& In )
{
	WxButtonGroupButton* Button = FindButtonFromId( In.GetId() );
	if( !Button )	return;

	if( Button->ButtonType == BUTTONTYPE_ClassMenu )
	{
		wxMenu Menu;
		PopupMenuClasses.Empty();

		// Get a sorted array of Volume Classes then add each item in the array to the menu.
		TArray< UClass* > VolumeClasses;

		GApp->EditorFrame->GetSortedVolumeClasses( &VolumeClasses );

		
		INT ID = IDM_VolumeClasses_START;

		for( INT VolumeIdx = 0; VolumeIdx < VolumeClasses.Num(); VolumeIdx++ )
		{
			PopupMenuClasses.AddItem( VolumeClasses( VolumeIdx ) );
			Menu.Insert( 0, ID, *VolumeClasses( VolumeIdx )->GetName(), TEXT(""), 0 );

			ID++;
		}

		FTrackPopupMenu tpm( this, &Menu );
		tpm.Show();
	}

	if( Button->ButtonType == BUTTONTYPE_BrushBuilder )
	{
		WxDlgBrushBuilder* dlg = new WxDlgBrushBuilder;
		dlg->Show( Button->BrushBuilder );
	}
}

void WxButtonGroup::OnAddVolumeClass( wxCommandEvent& In )
{
	WxButtonGroupButton* Button = FindButtonFromId( In.GetId() );
	if( !Button )	return;

	INT idx = In.GetId() - IDM_VolumeClasses_START;
	UClass* Class = PopupMenuClasses(idx);

	GUnrealEd->Exec( *FString::Printf( TEXT("BRUSH ADDVOLUME CLASS=%s"), *Class->GetName() ) );
}

WxButtonGroupButton* WxButtonGroup::FindButtonFromId( INT InID )
{
	if( InID >= IDM_VolumeClasses_START && InID <= IDM_VolumeClasses_END )
		InID = IDM_BRUSH_ADD_VOLUME;

	for( INT x = 0 ; x < Buttons.Num() ; ++x )
	{
		if( Buttons(x)->Button->GetId() == InID )
			return Buttons(x);
	}

	return NULL;
}

void WxButtonGroup::OnPaint( wxPaintEvent& In )
{
	wxPaintDC dc(this);
	wxRect rc = GetClientRect();
	rc.height = WxButtonGroup::TITLEBAR_H;

	dc.SetBrush( *wxTRANSPARENT_BRUSH );
	dc.SetPen( *wxGREY_PEN );

	dc.DrawBitmap( bExpanded ? GApp->EditorFrame->ArrowUp : GApp->EditorFrame->ArrowDown, rc.width-18,rc.y+4, 1 );
	dc.DrawRoundedRectangle( rc.x+1,rc.y+2, rc.width-1,rc.height-6, 5 );
}

void WxButtonGroup::OnLeftButtonDown( wxMouseEvent& In )
{
	wxRect rc = GetClientRect();
	rc.height = WxButtonGroup::TITLEBAR_H;
	if( rc.Contains( In.GetPosition() ) )
	{
		bExpanded = !bExpanded;
		GApp->EditorFrame->ButtonBar->PositionChildControls();
	}
}

void WxButtonGroup::OnBrushBuilder( wxCommandEvent& In )
{
	INT idx = In.GetId() - IDM_BrushBuilder_START;
	UBrushBuilder* bb = Buttons(idx)->BrushBuilder;

	bb->eventBuild();
	GEditorModeTools().GetCurrentMode()->MapChangeNotify();
}

void WxButtonGroup::UpdateUI()
{
	for( INT x = 0 ; x < Buttons.Num() ; ++x )
	{
		WxButtonGroupButton* bgb = Buttons(x);

		switch( bgb->Button->GetId() )
		{
			case IDM_DEFAULT:
				((WxBitmapCheckButton*)(bgb->Button))->SetCurrentState( GEditorModeTools().GetCurrentModeID() == EM_Default );
				break;
			case IDM_GEOMETRY:
				((WxBitmapCheckButton*)(bgb->Button))->SetCurrentState( GEditorModeTools().GetCurrentModeID() == EM_Geometry );
				break;
			case IDM_TERRAIN:
				((WxBitmapCheckButton*)(bgb->Button))->SetCurrentState( GEditorModeTools().GetCurrentModeID() == EM_TerrainEdit );
				break;
			case IDM_TEXTURE:
				((WxBitmapCheckButton*)(bgb->Button))->SetCurrentState( GEditorModeTools().GetCurrentModeID() == EM_Texture );
				break;
			case IDM_COVEREDIT:
				((WxBitmapCheckButton*)(bgb->Button))->SetCurrentState( GEditorModeTools().GetCurrentModeID() == EM_CoverEdit );
				break;
		}
	}
}

/*-----------------------------------------------------------------------------
	WxButtonBar.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxButtonBar, wxPanel )
	EVT_SIZE( WxButtonBar::OnSize )
END_EVENT_TABLE()

WxButtonBar::WxButtonBar()
{
}

WxButtonBar::~WxButtonBar()
{
}

void WxButtonBar::UpdateUI()
{
	for( INT x = 0 ; x < ButtonGroups.Num() ; ++x )
		ButtonGroups(x)->UpdateUI();
}

UBOOL WxButtonBar::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const FString& name )
{
	UBOOL bRet = wxPanel::Create( parent, id, pos, size, style, *name );

	WxButtonGroup* bg;

	// Modes

	bg = new WxButtonGroup( (wxWindow*)this );
	ButtonGroups.AddItem( bg );
	bg->AddButtonChecked( IDM_DEFAULT, TEXT("Btn_Camera"), *LocalizeUnrealEd("CameraMode") );
	bg->AddButtonChecked( IDM_GEOMETRY, TEXT("Btn_Geometry"), *LocalizeUnrealEd("GeometryMode") );
	bg->AddButtonChecked( IDM_TERRAIN, TEXT("Btn_TerrainMode"), *LocalizeUnrealEd("TerrainEditingMode") );
	bg->AddButtonChecked( IDM_TEXTURE, TEXT("Btn_TextureMode"), *LocalizeUnrealEd("TextureAlignmentMode") );

	bg = new WxButtonGroup( (wxWindow*)this );
	ButtonGroups.AddItem( bg );

	INT ID = IDM_BrushBuilder_START;

	for( TObjectIterator<UClass> ItC; ItC; ++ItC )
		if( ItC->IsChildOf(UBrushBuilder::StaticClass()) && !(ItC->ClassFlags&CLASS_Abstract) )
		{
			UBrushBuilder* ubb = ConstructObject<UBrushBuilder>( *ItC );
			if( ubb )
			{
				WxButtonGroupButton* bgb = bg->AddButton( ID, ubb->BitmapFilename, ubb->ToolTip, BUTTONTYPE_BrushBuilder, *ItC );
				bgb->BrushBuilder = ubb;
				ID++;
			}
		}

	bg = new WxButtonGroup( (wxWindow*)this );
	ButtonGroups.AddItem( bg );
	bg->AddButton( IDM_BRUSH_ADD, TEXT("Btn_Add"), *LocalizeUnrealEd("CSGAdd") );
	bg->AddButton( IDM_BRUSH_SUBTRACT, TEXT("Btn_Subtract"), *LocalizeUnrealEd("CSGSubtract") );
	bg->AddButton( IDM_BRUSH_INTERSECT, TEXT("Btn_Intersect"), *LocalizeUnrealEd("CSGIntersect") );
	bg->AddButton( IDM_BRUSH_DEINTERSECT, TEXT("Btn_Deintersect"), *LocalizeUnrealEd("CSGDeintersect") );

	bg = new WxButtonGroup( (wxWindow*)this );
	ButtonGroups.AddItem( bg );
	bg->AddButton( IDM_BRUSH_ADD_SPECIAL, TEXT("Btn_AddSpecial"), *LocalizeUnrealEd("AddSpecialBrush") );
	bg->AddButton( IDM_BRUSH_ADD_VOLUME, TEXT("Btn_AddVolume"), *LocalizeUnrealEd("AddVolume"), BUTTONTYPE_ClassMenu, AVolume::StaticClass(), IDM_VolumeClasses_START );

	bg = new WxButtonGroup( (wxWindow*)this );
	ButtonGroups.AddItem( bg );
	bg->AddButton( IDM_SELECT_SHOW, TEXT("Btn_ShowSelected"), *LocalizeUnrealEd("ShowSelectedActors") );
	bg->AddButton( IDM_SELECT_HIDE, TEXT("Btn_HideSelected"), *LocalizeUnrealEd("HideSelectedActors") );
	bg->AddButton( IDM_SELECT_INVERT, TEXT("Btn_InvertSelection"), *LocalizeUnrealEd("InvertSelections") );
	bg->AddButton( IDM_SHOW_ALL, TEXT("Btn_ShowAll"), *LocalizeUnrealEd("ShowAllActors") );

	return bRet;
}

void WxButtonBar::OnSize( wxSizeEvent& InEvent )
{
	PositionChildControls();
}

void WxButtonBar::PositionChildControls()
{
	wxRect rc = GetClientRect();

	INT Top = 0;

	// Size button groups to fit inside

	for( INT x = 0 ; x < ButtonGroups.Num() ; ++x )
	{
		WxButtonGroup* bg = ButtonGroups(x);

		INT YSz = bg->GetHeight();

		bg->SetSize( 0,Top, rc.GetWidth(),YSz );
		bg->Refresh();

		Top += YSz;
	}
}

