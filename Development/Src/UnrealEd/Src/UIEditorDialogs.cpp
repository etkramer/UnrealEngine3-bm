/*=============================================================================
	UIEditorDialogs.cpp: UI editor dialog window class implementations.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Includes
#include "UnrealEd.h"

#include "EngineUIPrivateClasses.h"
#include "UnrealEdPrivateClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"

//#include "Properties.h"
#include "UnObjectEditor.h"
#include "UnUIEditor.h"
#include "UIDockingPanel.h"
#include "ScopedObjectStateChange.h"


/* ==========================================================================================================
	WxDlgCreateUIStyle
========================================================================================================== */

/** Constructor */
FStyleSelectionData::FStyleSelectionData( const FUIStyleResourceInfo& InResourceInfo )
: ResourceInfo(InResourceInfo), CurrentTemplate(INDEX_NONE)
{
}

/**
 * Adds the specified style to this selection's list of available templates
 */
void FStyleSelectionData::AddStyleTemplate( UUIStyle* InTemplate )
{
	checkSlow(InTemplate->StyleDataClass->IsChildOf(ResourceInfo.UIResource->GetClass()));

	StyleTemplates.AddItem(InTemplate);
}

/**
 * Serializes the references to the style templates for this selection
 */
void FStyleSelectionData::Serialize( FArchive& Ar )
{
	Ar << StyleTemplates;
}


IMPLEMENT_DYNAMIC_CLASS(WxDlgCreateUIStyle,wxDialog)

BEGIN_EVENT_TABLE(WxDlgCreateUIStyle,wxDialog)
	EVT_RADIOBOX(ID_UI_NEWSTYLE_RADIO, WxDlgCreateUIStyle::OnStyleTypeSelected)
	EVT_CHOICE(ID_UI_NEWSTYLE_TEMPLATECOMBO, WxDlgCreateUIStyle::OnStyleTemplateSelected)
END_EVENT_TABLE()

/**
 * Constructor
 * Initializes the available list of style type selections.
 */
WxDlgCreateUIStyle::WxDlgCreateUIStyle()
: StyleClassSelect(NULL), TagEditbox(NULL), FriendlyNameEditbox(NULL), TemplateCombo(NULL)
{
	SceneManager = GUnrealEd->GetBrowserManager()->UISceneManager;
	InitializeStyleCollection();
}

/**
 * Destructor
 * Saves the window position and size to the ini
 */
WxDlgCreateUIStyle::~WxDlgCreateUIStyle()
{
	FWindowUtil::SavePosSize(TEXT("CreateStyleDialog"), this);
}

/**
 * Initialize this dialog.  Must be the first function called after creating the dialog.
 */
void WxDlgCreateUIStyle::Create( wxWindow* InParent, wxWindowID InID/* = ID_UI_NEWSTYLE_DLG*/ )
{
	verify(wxDialog::Create(
		InParent, InID, 
		*LocalizeUI(TEXT("DlgNewStyle_Title")), 
		wxDefaultPosition, wxDefaultSize, 
		wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxDIALOG_MODAL|wxCLIP_CHILDREN
		));

	SetExtraStyle(GetExtraStyle()|wxWS_EX_TRANSIENT|wxWS_EX_BLOCK_EVENTS);

	CreateControls();

	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
	Centre();

	wxRect DefaultPos = GetRect();
	FWindowUtil::LoadPosSize( TEXT("CreateStyleDialog"), this, DefaultPos.GetLeft(), DefaultPos.GetTop(), 400, DefaultPos.GetHeight() );
}

/**
 * Creates the controls and sizers for this dialog.
 */
void WxDlgCreateUIStyle::CreateControls()
{
	wxBoxSizer* BaseVSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(BaseVSizer);

    wxBoxSizer* PropertiesVSizer = new wxBoxSizer(wxVERTICAL);
    BaseVSizer->Add(PropertiesVSizer, 0, wxGROW|wxALL, 5);

	wxArrayString StyleClassNames;

	// get the labels for the radio button options
	for ( INT i = 0; i < StyleChoices.Num(); i++ )
	{
		FStyleSelectionData& StyleSelection = StyleChoices(i);
		StyleClassNames.Add(*StyleSelection.ResourceInfo.FriendlyName);
	}

	// create the "Select style class" radio button control
	FString StyleClassLabelText = Localize(TEXT("UIEditor"), TEXT("DlgNewStyle_Label_StyleClass"));
	StyleClassSelect = new wxRadioBox( 
		this,								// parent
		ID_UI_NEWSTYLE_RADIO,				// id
		*StyleClassLabelText,				// title
		wxDefaultPosition,					// pos
		wxDefaultSize,						// size
		StyleClassNames,					// choice labels
		1,									// major dimension
		wxRA_SPECIFY_ROWS					// style
		);
	StyleClassSelect->SetToolTip( *Localize(TEXT("UIEditor"), TEXT("DlgNewStyle_Tooltip_StyleClass")) );
	PropertiesVSizer->Add(StyleClassSelect, 0, wxGROW|wxALL, 5);

	// create the properties grouping box
    wxStaticBox* PropertiesGroupBox = new wxStaticBox(this, wxID_ANY, *LocalizeUnrealEd(TEXT("Properties")));
    wxStaticBoxSizer* PropertiesStaticVSizer = new wxStaticBoxSizer(PropertiesGroupBox, wxVERTICAL);
    PropertiesVSizer->Add(PropertiesStaticVSizer, 1, wxGROW|wxALL, 5);

	// create the sizer that will contain the options for creating the new style
    wxFlexGridSizer* PropertiesGridSizer = new wxFlexGridSizer(0, 2, 0, 0);
    PropertiesGridSizer->AddGrowableCol(1);
    PropertiesStaticVSizer->Add(PropertiesGridSizer, 1, wxGROW|wxALL, 5);

	// create the style tag label
    wxStaticText* TagLabel = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("DlgNewStyle_Label_StyleTag")), wxDefaultPosition, wxDefaultSize, 0 );
    PropertiesGridSizer->Add(TagLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	// create the style tag editbox
    TagEditbox = new wxTextCtrl( this, -1, TEXT(""), wxDefaultPosition, wxDefaultSize, 0 );
    TagEditbox->SetMaxLength(64);
	TagEditbox->SetToolTip(*LocalizeUI(TEXT("DlgNewStyle_Tooltip_StyleTag")));
	PropertiesGridSizer->Add(TagEditbox, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	// create the style friendly name label
    wxStaticText* NameLabel = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("DlgNewStyle_Label_StyleName")), wxDefaultPosition, wxDefaultSize, 0 );
    PropertiesGridSizer->Add(NameLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	// create the style friendly name editbox
    FriendlyNameEditbox = new wxTextCtrl( this, -1, TEXT(""), wxDefaultPosition, wxDefaultSize, 0 );
	FriendlyNameEditbox->SetToolTip(*LocalizeUI(TEXT("DlgNewStyle_Tooltip_StyleName")));
    PropertiesGridSizer->Add(FriendlyNameEditbox, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	// create the template combo label
    wxStaticText* TemplateLabel = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("DlgNewStyle_Label_StyleTemplate")), wxDefaultPosition, wxDefaultSize, 0 );
    PropertiesGridSizer->Add(TemplateLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxArrayString TemplateComboStrings;
	TemplateComboStrings.Add(*LocalizeUnrealEd(TEXT("None")));

	// create the combo box that will display the list of UIStyles that can be used as the archetype for this new style
    TemplateCombo = new wxChoice( this, ID_UI_NEWSTYLE_TEMPLATECOMBO, wxDefaultPosition, wxDefaultSize, TemplateComboStrings );
	TemplateCombo->SetToolTip(*LocalizeUI(TEXT("DlgNewStyle_Tooltip_StyleTemplate")));
    PropertiesGridSizer->Add(TemplateCombo, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	// setup the sizer for the OK/Cancel buttons
    wxBoxSizer* ButtonHSizer = new wxBoxSizer(wxHORIZONTAL);
    PropertiesVSizer->Add(ButtonHSizer, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* OKButton = new wxButton( this, wxID_OK, *LocalizeUnrealEd(TEXT("&OK")), wxDefaultPosition, wxDefaultSize, 0 );
    ButtonHSizer->Add(OKButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* CancelButton = new wxButton( this, wxID_CANCEL, *LocalizeUnrealEd(TEXT("&Cancel")), wxDefaultPosition, wxDefaultSize, 0 );
    ButtonHSizer->Add(CancelButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	TagEditbox->SetValidator( WxNameTextValidator(&StyleTagString,VALIDATE_ObjectName) );

	FLocalizeWindow(this,TRUE);
}

/**
 * Initializes the list of available styles, along with the templates available to each one.
 */
void WxDlgCreateUIStyle::InitializeStyleCollection()
{
	check(SceneManager != NULL);

	for ( INT i = 0; i < SceneManager->UIStyleResources.Num(); i++ )
	{
		FStyleSelectionData* StyleSelection = new FStyleSelectionData(SceneManager->UIStyleResources(i));
		StyleChoices.AddRawItem(StyleSelection);
	}

	TArray<UUIStyle*> SkinStyles;
	SceneManager->ActiveSkin->GetAvailableStyles(SkinStyles, FALSE);

	// iterate through the list of available styles in the currently active skin
	//@fixme - this should actually use the style lookup table (by calling GetAvailableStyles(SkinStyles,TRUE)),
	// but then users would be able to use styles from base skins as the archetype for new skins....this is fine,
	// but the ReplaceStyle functionality needs to then fixup those style references to point the corresponding style
	// in the current skin (i.e. the style that is replacing the style that is about to be used for this new style's archetype)
	for ( INT i = 0; i < SkinStyles.Num(); i++ )
	{
		UUIStyle* StyleObj = SkinStyles(i);
		if ( !StyleObj->IsTemplate(RF_ClassDefaultObject) )
		{
			INT StyleDataIndex;

			// if this style object belongs to this style type's class, add the object to the style type's list of available templates
			if ( GetSelectionData(StyleObj,StyleDataIndex) )
			{
				StyleChoices(StyleDataIndex).AddStyleTemplate(StyleObj);
			}
		}
	}
}

/**
 * Retrieves the style selection data associated with the specified style object's class
 *
 * @param	StyleObject			the style object to find selection data for
 * @param	out_SelectionIndex	if the return value is TRUE, will be filled with the index corresponding to the
 *								selection data for the specified style object's class.  If the return value is FALSE,
 *								this value is left unchanged.
 *
 * @return	TRUE if selection data was successfully found for the specified style class
 */
UBOOL WxDlgCreateUIStyle::GetSelectionData( UUIStyle* StyleObject, INT& out_SelectionIndex )
{
	UBOOL bResult = FALSE;
	if ( StyleObject != NULL )
	{
		for ( INT i = 0; i < StyleChoices.Num(); i++ )
		{
			FStyleSelectionData& Choice = StyleChoices(i);
			if ( StyleObject->StyleDataClass->IsChildOf(Choice.ResourceInfo.UIResource->GetClass()) )
			{
				out_SelectionIndex = i;
				bResult = TRUE;
				break;
			}
		}
	}

	return bResult;
}

/**
 * Displays this dialog.
 *
 * @param	InStyleTag		the default value for the "Unique Name" field
 * @param	InFriendlyName	the default value for the "Friendly Name" field
 * @param	InTemplate		the style template to use for the new style
 */
int WxDlgCreateUIStyle::ShowModal( const FString& InStyleTag, const FString& InFriendlyName, UUIStyle* InTemplate/*=NULL*/ )
{
	TagEditbox->SetValue( *InStyleTag );
	FriendlyNameEditbox->SetValue( *InFriendlyName );

	INT StyleTypeIndex=0;
	if ( InTemplate != NULL )
	{
		// if InTemplate is not NULL, it means the user selected the "create style from selected" option...
		// so disable the style class radio button since we must use the class of the template
		if ( GetSelectionData(InTemplate,StyleTypeIndex) )
		{
			// disable the style class radio button if the user specified a t
			StyleClassSelect->SetSelection(StyleTypeIndex);
			StyleClassSelect->Enable(false);
			StyleChoices(StyleTypeIndex).SetCurrentTemplate( InTemplate );
		}
	}

	PopulateStyleTemplateCombo(StyleTypeIndex);

	return wxDialog::ShowModal();
}

/**
 * Populates the style template combo with the list of UIStyle objects which can be used as an archetype for the new style.
 * Only displays the styles that correspond to the selected style type.
 *
 * @param	SelectedStyleIndex	the index of the selected style; corresponds to an element in the StyleChoices array
 */
void WxDlgCreateUIStyle::PopulateStyleTemplateCombo( INT SelectedStyleIndex/*=INDEX_NONE*/ )
{
	// clear all existing items
	TemplateCombo->Clear();

	// add the "None" item
	TemplateCombo->Append( *LocalizeUnrealEd(TEXT("None")) );

	INT TemplateIndex = INDEX_NONE;
	if ( StyleChoices.IsValidIndex(SelectedStyleIndex) )
	{
		TArray<UUIStyle*> StyleTemplates;
		StyleChoices(SelectedStyleIndex).GetStyleTemplates(StyleTemplates);
		for ( INT i = 0; i < StyleTemplates.Num(); i++ )
		{
			// add the data for each template for this style type to the template combo
			TemplateCombo->Append( *StyleTemplates(i)->GetPathName(), StyleTemplates(i) );
		}

		TemplateIndex = StyleChoices(SelectedStyleIndex).GetCurrentTemplate();
	}

	TemplateCombo->SetSelection( TemplateIndex + 1 );
}

/**
 * This handler is called when the user selects an item in the radiobox.
 */
void WxDlgCreateUIStyle::OnStyleTypeSelected( wxCommandEvent& Event )
{
	PopulateStyleTemplateCombo(Event.GetSelection());
}

/**
 * This handler is called when the user selects an item in the choice control.
 */
void WxDlgCreateUIStyle::OnStyleTemplateSelected( wxCommandEvent& Event )
{
	INT StyleTypeIndex = StyleClassSelect->GetSelection();
	check(StyleChoices.IsValidIndex(StyleTypeIndex));

	// set the currently selected style type's CurrentTemplate index to the newly selected template
	StyleChoices(StyleTypeIndex).SetCurrentTemplate( (UUIStyle*)Event.GetClientData() );
}

/**
 * Retrieve the style class that was selected by the user
 */
UClass* WxDlgCreateUIStyle::GetStyleClass() const
{
	INT StyleTypeIndex = StyleClassSelect->GetSelection();
	check(StyleChoices.IsValidIndex(StyleTypeIndex));

	return StyleChoices(StyleTypeIndex).ResourceInfo.UIResource->GetClass();
}

/**
 * Retrieve the style template that was selected by the user
 */
UUIStyle* WxDlgCreateUIStyle::GetStyleTemplate() const
{
	UUIStyle* Result = NULL;
	if ( TemplateCombo != NULL && TemplateCombo->GetCount() > 1 )
	{
		Result = (UUIStyle*)TemplateCombo->GetClientData(TemplateCombo->GetSelection());
	}

	return Result;
}

/* ==========================================================================================================
	WxDlgCreateUISkin
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxDlgCreateUISkin,wxDialog)

BEGIN_EVENT_TABLE(WxDlgCreateUISkin,wxDialog)
	EVT_TEXT(ID_UI_NEWSKIN_EDIT_PKGNAME,WxDlgCreateUISkin::OnTextEntered)
	EVT_TEXT(ID_UI_NEWSKIN_EDIT_SKINNAME,WxDlgCreateUISkin::OnTextEntered)
END_EVENT_TABLE()

void WxDlgCreateUISkin::Create( wxWindow* InParent, wxWindowID InID )
{
	verify(wxDialog::Create(InParent,InID,*LocalizeUI(TEXT("DlgNewSkin_Title")),wxDefaultPosition,wxSize(400,178),wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxCLIP_CHILDREN));

	SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);

	CreateControls();

	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
	Centre();
}

/**
 * Creates the controls for this window
 */
void WxDlgCreateUISkin::CreateControls()
{
	wxBoxSizer* BaseVSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(BaseVSizer);

	wxStaticBox* OptionsGroupBox = new wxStaticBox(this, wxID_ANY, TEXT("Options"));
	wxStaticBoxSizer* OptionsVSizer = new wxStaticBoxSizer(OptionsGroupBox, wxVERTICAL);
	BaseVSizer->Add(OptionsVSizer, 0, wxGROW|wxALL, 5);

	wxFlexGridSizer* OptionsGridSizer = new wxFlexGridSizer(0, 2, 0, 0);
	OptionsGridSizer->AddGrowableCol(1);
	OptionsVSizer->Add(OptionsGridSizer, 1, wxGROW|wxALL, 5);

	//@todo - localize
	wxStaticText* PackageNameLabel = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("DlgNewSkin_Label_PackageName")), wxDefaultPosition, wxDefaultSize, 0 );
	OptionsGridSizer->Add(PackageNameLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	PackageNameEdit = new WxTextCtrl( this, ID_UI_NEWSKIN_EDIT_PKGNAME, TEXT(""), wxDefaultPosition, wxDefaultSize, 0 );
	OptionsGridSizer->Add(PackageNameEdit, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* SkinNameLabel = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("DlgNewSkin_Label_SkinName")), wxDefaultPosition, wxDefaultSize, 0 );
	OptionsGridSizer->Add(SkinNameLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	SkinNameEdit = new WxTextCtrl( this, ID_UI_NEWSKIN_EDIT_SKINNAME, TEXT(""), wxDefaultPosition, wxDefaultSize, 0 );
	OptionsGridSizer->Add(SkinNameEdit, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* TemplateLabel = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("DlgNewStyle_Label_StyleTemplate")), wxDefaultPosition, wxDefaultSize, 0 );
	OptionsGridSizer->Add(TemplateLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	SkinTemplateCombo = new wxChoice( this, ID_UI_NEWSKIN_COMBO_TEMPLATE, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	OptionsGridSizer->Add(SkinTemplateCombo, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxBoxSizer* ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	BaseVSizer->Add(ButtonSizer, 0, wxALIGN_RIGHT|wxALL, 2);

	OKButton = new wxButton( this, wxID_OK, *LocalizeUnrealEd(TEXT("&OK")) );
	ButtonSizer->Add(OKButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxButton* CancelButton = new wxButton( this, wxID_CANCEL, *LocalizeUnrealEd(TEXT("&Cancel")) );
	ButtonSizer->Add(CancelButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	PackageNameEdit->SetValidator( WxNameTextValidator(&PackageNameString,VALIDATE_PackageName) );
	SkinNameEdit->SetValidator( WxNameTextValidator(&SkinNameString,VALIDATE_ObjectName) );
}

/**
 * Displays this dialog.
 *
 * @param	InPackageName	the default value for the "Package Name" field
 * @param	InSkinName		the default value for the "Skin Name" field
 * @param	InTemplate		the template to use for the new skin
 */
INT WxDlgCreateUISkin::ShowModal( const FString& InPackageName, const FString& InSkinName, UUISkin* InTemplate )
{
	PackageNameString = *InPackageName;
	SkinNameString = *InSkinName;

	// for now, only add the specified template
	INT Index = SkinTemplateCombo->Append( *InTemplate->Tag.ToString(), InTemplate );
	SkinTemplateCombo->SetSelection(Index);
	SkinTemplateCombo->Enable(FALSE);

	return wxDialog::ShowModal();
}

/** This handler is called when the user types text in the text control */
void WxDlgCreateUISkin::OnTextEntered( wxCommandEvent& Event )
{
	if ( OKButton != NULL )
	{
		// only enable the OK button if both edit boxes contain text
		OKButton->Enable( PackageNameEdit->GetLineLength(0) > 0 && SkinNameEdit->GetLineLength(0) > 0 );
	}

	Event.Skip();
}

bool WxDlgCreateUISkin::Validate()
{
	bool bResult = wxDialog::Validate();
	if ( bResult )
	{
		FString PackageName = PackageNameString.c_str();
		FString SkinName = SkinNameString.c_str();

		FString QualifiedSkinName = PackageName + TEXT(".") + SkinName;
		FString Reason;
		if ( !FIsUniqueObjectName(*QualifiedSkinName,ANY_PACKAGE,Reason) )
		{
			//@todo - don't rely on FIsUniqueObjectName....search through all packages
			appMsgf(AMT_OK, *Reason);
			bResult = false;
		}
	}

	return bResult;
}

/**
 * Returns the UUISkin corresponding to the selected item in the template combobox
 */
UUISkin* WxDlgCreateUISkin::GetSkinTemplate()
{
	return (UUISkin*)SkinTemplateCombo->GetClientData(SkinTemplateCombo->GetSelection());
}

/* ==========================================================================================================
	WxDlgDockingEditor
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxDlgDockingEditor,wxDialog)

/**
 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
 *
 * @param	InParent				the window that opened this dialog
 * @param	InID					the ID to use for this dialog
 * @param	AdditionalButtonIds		additional buttons that should be added to the dialog....currently only ID_CANCEL_ALL is supported
 */
void WxDlgDockingEditor::Create( wxWindow* InParent, wxWindowID InID/*=ID_UI_DOCKINGEDITOR_DLG*/, long AdditionalButtonIds/*=0*/ )
{
	SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
	verify(wxDialog::Create( InParent, InID, *LocalizeUI(TEXT("DlgDockingEditor_Title")), wxDefaultPosition, wxSize(600,300), wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL|wxCLIP_CHILDREN|wxTAB_TRAVERSAL ));

	CreateControls(AdditionalButtonIds);

	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
	Centre();

	wxRect DefaultPos = GetRect();
	FWindowUtil::LoadPosSize( TEXT("DockingEditorDialog"), this, DefaultPos.GetLeft(), DefaultPos.GetTop(), 400, DefaultPos.GetHeight() );

	// in case the previous size of the dialog is too small to display all items in the combos, update the minimum size of the dialog
	// to what the sizers report as the minimum size to display everything
	GetSizer()->SetSizeHints(this);
}

/**
 * Destructor
 * Saves the window position
 */
WxDlgDockingEditor::~WxDlgDockingEditor()
{
	FWindowUtil::SavePosSize(TEXT("DockingEditorDialog"), this);
}

/**
 * Creates the controls for this window
 */
void WxDlgDockingEditor::CreateControls( long AdditionalButtonIds/*=0*/ )
{
	wxBoxSizer* BaseVSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(BaseVSizer);

	EditorPanel = new WxUIDockingSetEditorPanel(this);
	BaseVSizer->Add(EditorPanel, 0, wxGROW|wxALL, 0);

	wxBoxSizer* ButtonHSizer = new wxBoxSizer(wxHORIZONTAL);
	BaseVSizer->Add(ButtonHSizer, 0, wxALIGN_RIGHT|wxALL, 5);

	// add the OK and cancel buttons
	btn_OK = new wxButton( this, wxID_OK, *LocalizeUnrealEd(TEXT("&OK")) );
	btn_OK->SetDefault();
	ButtonHSizer->Add(btn_OK, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxButton* btn_Cancel = new wxButton( this, wxID_CANCEL, *LocalizeUnrealEd(TEXT("&Cancel")) );
	ButtonHSizer->Add(btn_Cancel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

/**
 * Initializes the values of the controls in this dialog, the displays the dialog.
 *
 * @param	Container	the object which contains InWidget
 * @param	InWidget	the widget to edit the docking sets for
 */
INT WxDlgDockingEditor::ShowModal( UUIScreenObject* Container, UUIObject* InWidget )
{
	check(Container);
	check(InWidget);

	FString WindowTitle = LocalizeUI(TEXT("DlgDockingEditor_Title")) + TEXT(": ") + InWidget->GetTag().ToString();
	SetTitle(*WindowTitle);

	// InWidget must be contained within Container
	verify(Container==InWidget->GetScene() || InWidget->IsContainedBy(Cast<UUIObject>(Container)));

	EditorPanel->RefreshEditorPanel(InWidget);

	// display the dialog
	return wxDialog::ShowModal();
}

/**
 * Applies changes made in the dialog to the widget being edited.  Called from the owner window when ShowModal()
 * returns the OK code.
 *
 * @return	TRUE if all changes were successfully applied
 */
UBOOL WxDlgDockingEditor::SaveChanges()
{
	UBOOL bSuccess=TRUE;

	FScopedObjectStateChange DockingChangeNotifier(EditorPanel->CurrentWidget);

	// apply the changes
	for ( INT FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
	{
		UUIScreenObject* TargetWidget = EditorPanel->GetSelectedDockTarget((EUIWidgetFace)FaceIndex);
		EUIWidgetFace TargetFace = EditorPanel->GetSelectedDockFace((EUIWidgetFace)FaceIndex);
		FLOAT Padding = EditorPanel->GetSelectedDockPadding((EUIWidgetFace)FaceIndex);
		EUIDockPaddingEvalType PaddingType = EditorPanel->GetSelectedDockPaddingType((EUIWidgetFace)FaceIndex);

		bSuccess = EditorPanel->CurrentWidget->SetDockParameters((BYTE)FaceIndex, TargetWidget, TargetFace, Padding, PaddingType, TRUE) && bSuccess;
	}

	if ( !bSuccess )
	{
		DockingChangeNotifier.CancelEdit();
	}

	return bSuccess;
}








// EOF


