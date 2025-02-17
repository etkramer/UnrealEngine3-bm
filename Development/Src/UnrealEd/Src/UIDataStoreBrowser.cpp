/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "FConfigCacheIni.h"
#include "SourceControlIntegration.h"

#include "EngineUIPrivateClasses.h"
#include "UnrealEdPrivateClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"

#include "UnObjectEditor.h"
#include "UnUIEditor.h"
#include "UnUIStyleEditor.h"

#include "UIDataStoreBrowser.h"


/* ==========================================================================================================
WxDlgUICreateConfigDataTag
========================================================================================================== */
/**
 * Dialog that lets the user add a new data tag to a ConfigFileDataProvider
 */
class WxDlgUICreateConfigDataTag : public wxDialog
{
public:
	WxDlgUICreateConfigDataTag(WxDlgUIDataStoreBrowser* Parent, UUIConfigFileProvider* DataProvider, const FString &InSection);
	
	/** @return Returns the section name that the user entered. */
	const FString& GetSectionName() const
	{
		return SectionName;
	}

	/** @return Returns the tag name that the user entered. */
	const FString& GetTagName() const
	{
		return TagName;
	}

protected:

	/** Creates a new tag in the specified config section with the value provided. */
	void OnOK(wxCommandEvent& Event);

	/** Textbox that holds the section name of the new data tag. */
	wxTextCtrl *SectionText;

	/** Textbox that holds the key of the new data tag. */
	wxTextCtrl *TagText;

	/** Data provider that we are adding the new tag to. */
	UUIConfigFileProvider* DataProvider;

	/** String to store resulting section name in. */
	FString SectionName;

	/** String to store resulting tag in. */
	FString TagName;
		

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WxDlgUICreateConfigDataTag, wxDialog)
	EVT_BUTTON(wxID_OK, OnOK)
END_EVENT_TABLE()

WxDlgUICreateConfigDataTag::WxDlgUICreateConfigDataTag(WxDlgUIDataStoreBrowser* Parent, UUIConfigFileProvider* InDataProvider, const FString &InSection) : 
wxDialog(Parent, wxID_ANY, *LocalizeUI(TEXT("DlgUICreateConfigDataTag_Title")), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
DataProvider(InDataProvider)
{
	SetMinSize(wxSize(400,-1));

	// Get a string path to the data provider.
	FString ProviderPath;

	UUIDataStore* ConfigDataStore = Parent->GetOwningDataStore(DataProvider);
	if ( ConfigDataStore != NULL )
	{
		UUIDataProvider* OwnerProvider = NULL;
		verify(ConfigDataStore->ContainsProvider(DataProvider, OwnerProvider));

		ProviderPath = OwnerProvider->BuildDataFieldPath(ConfigDataStore, *DataProvider->GetName());
	}

	// Create controls
	wxSizer* MainSizer = new wxBoxSizer(wxHORIZONTAL);
	{
		// Text boxes describing the new data tag to add.
		wxSizer* TextBoxSizer = new wxBoxSizer(wxVERTICAL);
		{
			// Data store path.
			wxStaticText* ConfigFilePath = new wxStaticText(this, wxID_ANY, *FString::Printf(LocalizeSecure(LocalizeUI(TEXT("DlgUICreateConfigDataTag_DataStorePath")), *ProviderPath)));
			TextBoxSizer->Add(ConfigFilePath, 0, wxEXPAND | wxALL, 5);

			// Section
			wxStaticBoxSizer* SectionSizer = new wxStaticBoxSizer(wxVERTICAL, this, *LocalizeUI(TEXT("DlgUICreateConfigDataTag_Section")));
			{
				SectionText = new wxTextCtrl(this, wxID_ANY);
				SectionText->SetValue(*InSection);
				SectionName = InSection;

				SectionSizer->Add(SectionText, 0, wxEXPAND | wxALL, 5);
			}
			TextBoxSizer->Add(SectionSizer, 0, wxEXPAND | wxALL, 3);

			// Tag
			wxStaticBoxSizer* TagSizer = new wxStaticBoxSizer(wxVERTICAL, this, *LocalizeUI(TEXT("DlgUICreateConfigDataTag_Tag")));
			{
				TagText = new wxTextCtrl(this, wxID_ANY);

				// Set focus to the tag field if the section field was already filled in.
				if(SectionName.Len() > 0)
				{
					TagText->SetFocus();
				}
				TagSizer->Add(TagText, 0, wxEXPAND | wxALL, 5);
			}
			TextBoxSizer->Add(TagSizer, 0, wxEXPAND | wxALL, 3);
		}
		MainSizer->Add(TextBoxSizer, 1, wxEXPAND | wxALL, 4);

		// Add OK/Cancel buttons.
		wxSizer *ButtonSizer = new wxBoxSizer(wxVERTICAL);
		{
			wxButton* ButtonOK = new wxButton(this, wxID_OK, *LocalizeUnrealEd("OK"));
			ButtonOK->SetDefault();

			ButtonSizer->Add(ButtonOK, 0, wxEXPAND | wxALL, 5);

			wxButton* ButtonCancel = new wxButton(this, wxID_CANCEL, *LocalizeUnrealEd("Cancel"));
			ButtonSizer->Add(ButtonCancel, 0, wxEXPAND | wxALL, 5);
		}
		MainSizer->Add(ButtonSizer, 0, wxALL, 5);
	}
	SetSizer(MainSizer);
}

/** Creates a new tag in the specified config section with the value provided. */
void WxDlgUICreateConfigDataTag::OnOK(wxCommandEvent& Event)
{
	const wxString Section = SectionText->GetValue();
	const wxString Tag = TagText->GetValue();

	if(Section.Len() && Tag.Len())
	{
		SectionName = Section;
		TagName = Tag;
		EndModal(wxID_OK);
	}
	else
	{
		wxMessageBox(*LocalizeUI(TEXT("DlgUICreateConfigDataTag_NeedText")), *LocalizeUI(TEXT("DlgUICreateConfigDataTag_Error")), wxICON_ERROR);
	}
}


/* ==========================================================================================================
	WxUIDlgDataBrowser
========================================================================================================== */
BEGIN_EVENT_TABLE(WxDlgUIDataStoreBrowser, wxFrame)
	EVT_CLOSE(OnClose)
	EVT_MENU(IDM_UI_ADDDATATAG,		WxDlgUIDataStoreBrowser::OnAddDatastoreTag)
	EVT_BUTTON(IDM_UI_ADDDATATAG,	WxDlgUIDataStoreBrowser::OnAddDatastoreTag)
	EVT_MENU(IDM_UI_COPYDATAMARKUP,	WxDlgUIDataStoreBrowser::OnCopyDataStoreMarkup)
END_EVENT_TABLE()

WxDlgUIDataStoreBrowser::WxDlgUIDataStoreBrowser(wxWindow* InParent, UUISceneManager* InSceneManager) : 
wxFrame(InParent, wxID_ANY, (wxString)*LocalizeUI("UIEditor_DataStoreBrowser_Title"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT)
{
	wxSizer* MainSizer = new wxBoxSizer(wxVERTICAL);
	{
		DataStoreNotebook = new wxNotebook(this, wxID_ANY);
		{
			StringsPanel = new WxUIStringDataStorePanel(DataStoreNotebook, InSceneManager);
			OthersPanel = new WxUIDataStorePanel(DataStoreNotebook, InSceneManager);

			DataStoreNotebook->AddPage(StringsPanel, *LocalizeUI("UIEditor_DataStoreBrowser_Strings"), TRUE);
			DataStoreNotebook->AddPage(OthersPanel, *LocalizeUI("UIEditor_DataStoreBrowser_Other"), TRUE);

			wxColour BGColor = DataStoreNotebook->GetThemeBackgroundColour();
			if (BGColor.Ok())
			{
				StringsPanel->SetBackgroundColour(BGColor);
				OthersPanel->SetBackgroundColour(BGColor);
			}
		}
		MainSizer->Add(DataStoreNotebook, 1, wxEXPAND);
	}
	SetSizer(MainSizer);
	Layout();

	wxAcceleratorEntry entries[1];
	entries[0].Set(wxACCEL_CTRL, (int)'C', IDM_UI_COPYDATAMARKUP);
	wxAcceleratorTable accel(1, entries);
	SetAcceleratorTable(accel);

	LoadSettings();

	DataStoreNotebook->SetSelection(0);

	// Generate data provider trees
	OthersPanel->RefreshTree();
	StringsPanel->RefreshTree();
}

WxDlgUIDataStoreBrowser::~WxDlgUIDataStoreBrowser()
{
	SaveSettings();
}

/** Shows the dialog. */
bool WxDlgUIDataStoreBrowser::Show(bool Show)
{
	bool Result = wxFrame::Show(Show);

	if(IsIconized())
	{
		Iconize(FALSE);
	}

	SetFocus();
	WxUIDataStorePanel* Panel = (WxUIDataStorePanel*)DataStoreNotebook->GetCurrentPage();
	Panel->ResetFocus();

	return Result;
}

/**
 * @return	TRUE if there is a valid tag selected.
 */
UBOOL WxDlgUIDataStoreBrowser::IsTagSelected() const
{
	UBOOL bResult = FALSE;

	WxUIDataStorePanel* Panel = static_cast<WxUIDataStorePanel*>(DataStoreNotebook->GetCurrentPage());
	if ( Panel )
	{
		bResult = Panel->IsTagSelected();
	}
	
	return bResult;
}

/** 
 * Returns the currently selected tag in OutTagName. 
 * @return TRUE if there is a tag selected, FALSE otherwise.
 */
UBOOL WxDlgUIDataStoreBrowser::GetSelectedTag(FName &OutTagName)
{
	WxUIDataStorePanel* Panel = (WxUIDataStorePanel*)DataStoreNotebook->GetCurrentPage();
	return Panel->GetSelectedTag(OutTagName);
}

/** 
 * @return the path to the current tag if one is selected, or a blank string if one is not selected.
 */
FString WxDlgUIDataStoreBrowser::GetSelectedTagPath() const
{
	WxUIDataStorePanel* Panel = (WxUIDataStorePanel*)DataStoreNotebook->GetCurrentPage();
	return Panel->GetSelectedTagPath();
}

/**
 * @return	the provider field type for the currently selected tag, or DATATYPE_MAX if no tags are selected
 */
EUIDataProviderFieldType WxDlgUIDataStoreBrowser::GetSelectedTagType() const
{
	WxUIDataStorePanel* Panel = (WxUIDataStorePanel*)DataStoreNotebook->GetCurrentPage();
	return Panel->GetSelectedTagType();
}

/** 
 * @return Returns the selected dataprovider if there is one, otherwise NULL.
 */
UUIDataProvider* WxDlgUIDataStoreBrowser::GetSelectedTagProvider()
{
	WxUIDataStorePanel* Panel = (WxUIDataStorePanel*)DataStoreNotebook->GetCurrentPage();
	return Panel->GetSelectedTagProvider();
}

/** 
 * @return Returns the selected dataprovider using the tree control if there is one, otherwise NULL.
 */
UUIDataProvider* WxDlgUIDataStoreBrowser::GetSelectedTreeProvider()
{
	WxUIDataStorePanel* Panel = (WxUIDataStorePanel*)DataStoreNotebook->GetCurrentPage();
	return Panel->GetSelectedTreeProvider();
}


/**
 * @param	DataProvider		Provider to find the owning datastore for.
 * @return	Returns the data store that contains the specified data provider.
 */
UUIDataStore* WxDlgUIDataStoreBrowser::GetOwningDataStore(UUIDataProvider* DataProvider)
{
	WxUIDataStorePanel* Panel = (WxUIDataStorePanel*)DataStoreNotebook->GetCurrentPage();
	return Panel->GetOwningDataStore(DataProvider);
}

/**
 * Copies the markup string for the selected tag to the clipboard 
 */
void WxDlgUIDataStoreBrowser::CopySelectedMarkupToClipboard()
{
	// Get the selected datastore
	UUIDataProvider* CurrentDataProvider = GetSelectedTagProvider();
	if ( CurrentDataProvider != NULL )
	{
		FName Tag = NAME_None;
		const UBOOL bTagSelected = GetSelectedTag(Tag);
		if ( bTagSelected )
		{
			UUIDataStore* OwningDataStore = GetOwningDataStore(CurrentDataProvider);

			// Get the markup for the current tag.
			const FString TagMarkup = CurrentDataProvider->GenerateDataMarkupString(OwningDataStore,Tag);
			if ( TagMarkup.Len() > 0 )
			{
				appClipboardCopy(*TagMarkup);
			}
		}
	}
}

/** Handler for 'Copy Selected Markup' context menu option - calls CopySelectedMarkupToClipboard */
void WxDlgUIDataStoreBrowser::OnCopyDataStoreMarkup(wxCommandEvent& Event)
{
	CopySelectedMarkupToClipboard();
}


/** Save window position and other settings for the dialog. */
void WxDlgUIDataStoreBrowser::SaveSettings() const
{
	StringsPanel->SaveSettings();
	OthersPanel->SaveSettings();

	// Save Options
	FWindowUtil::SavePosSize(TEXT("DlgUIDataStoreBrowser"), this);
}

/** Load window position and other settings for the dialog. */
void WxDlgUIDataStoreBrowser::LoadSettings()
{
	// Load Options
	FWindowUtil::LoadPosSize(TEXT("DlgUIDataStoreBrowser"), this, -1, -1, 800, 600);


	StringsPanel->LoadSettings();
	OthersPanel->LoadSettings();
}

void WxDlgUIDataStoreBrowser::OnClose(wxCloseEvent &Event)
{
	SaveSettings();

	Hide();
}

/** Adds a new datastore tag to the selected provider. */
void WxDlgUIDataStoreBrowser::OnAddDatastoreTag( wxCommandEvent& Event )
{
	WxUIDataStorePanel* Panel = (WxUIDataStorePanel*)DataStoreNotebook->GetCurrentPage();
	Panel->AddTag();
}

/* ==========================================================================================================
	WxUIDataStorePanel
========================================================================================================== */
BEGIN_EVENT_TABLE(WxUIDataStorePanel, wxPanel)
	EVT_TREE_SEL_CHANGED(ID_UI_TREE_DATASTORE, WxUIDataStorePanel::OnTreeSelectionChanged)
	EVT_TEXT(ID_UI_DATASTORE_FILTER, WxUIDataStorePanel::OnFilterTextChanged)
END_EVENT_TABLE()

namespace
{
	enum EPropertiesFields
	{
		PF_Name,
		PF_Path,
		PF_Type,
		PF_Markup
	};
};

WxUIDataStorePanel::WxUIDataStorePanel(wxWindow* InParent,UBOOL bInDisplayNestedTags/*=FALSE*/)
: wxPanel(InParent)
, bDisplayNestedTags(bInDisplayNestedTags)
{

}

WxUIDataStorePanel::WxUIDataStorePanel(wxWindow* InParent, UUISceneManager* InSceneManager,UBOOL bInDisplayNestedTags/*=FALSE*/)
: wxPanel(InParent)
, SceneManager(InSceneManager)
, bDisplayNestedTags(bInDisplayNestedTags)
{
	wxSizer* MainSizer = new wxBoxSizer(wxVERTICAL);
	{
		// Filter
		wxSizer* FilterSizer = new wxBoxSizer(wxHORIZONTAL);
		{
			wxStaticText* ST = new wxStaticText(this, wxID_ANY, *LocalizeUI("UIEditor_DataStoreBrowser_Filter"));
			FilterSizer->Add(ST,0, wxRIGHT | wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT, 4);

			FilterText = new wxTextCtrl(this, ID_UI_DATASTORE_FILTER);
			FilterSizer->Add(FilterText,1, wxEXPAND);
		}
		MainSizer->Add(FilterSizer, 0, wxEXPAND | wxALL, 3);

		SplitterWnd = new wxSplitterWindow(this);
		{
			DataStoreTree = new WxTreeCtrl(SplitterWnd, ID_UI_TREE_DATASTORE, NULL, wxTR_HAS_BUTTONS|wxTR_LINES_AT_ROOT|wxTR_EXTENDED|wxTR_HIDE_ROOT|wxTR_SINGLE);

			wxPanel* TempPanel = new wxPanel(SplitterWnd);
			wxStaticBoxSizer* PropertySizer = new wxStaticBoxSizer(wxVERTICAL, TempPanel, *LocalizeUI(TEXT("UIEditor_DataStoreBrowser_DataStoreTags")));
			{
				TagsList = new WxListView(TempPanel, ID_UI_LIST_DATASTOREPROPERTIES, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
				{
					TagsList->InsertColumn(0, *LocalizeUI("UIEditor_DataStoreBrowser_Tag"));
					TagsList->InsertColumn(1, *LocalizeUI("UIEditor_DataStoreBrowser_Path"));
					TagsList->InsertColumn(2, *LocalizeUI("UIEditor_DataStoreBrowser_Type"));
					TagsList->InsertColumn(3, *LocalizeUI("UIEditor_DataStoreBrowser_Markup"));
				}
				PropertySizer->Add(TagsList, 1, wxEXPAND | wxALL, 5);
			}
			TempPanel->SetSizer(PropertySizer);

			// Split the window and set default properties for the splitter.
			const INT MinPaneSize = 20;
			SplitterWnd->SetMinimumPaneSize(MinPaneSize);
			SplitterWnd->SplitVertically(DataStoreTree, TempPanel);
		}
		MainSizer->Add(SplitterWnd, 1, wxEXPAND);
	}
	SetSizer(MainSizer);
	Layout();

	// Always default focus to the filter text.
	FilterText->SetFocus();
}

/** Loads settings for this panel. */
void WxUIDataStorePanel::LoadSettings()
{
	// Load Sash Position
	INT SashPos = 300;
	GConfig->GetInt(TEXT("UIDataStorePanel"), TEXT("SashPos"), SashPos, GEditorUserSettingsIni);
	SplitterWnd->SetSashPosition(SashPos);

	FString Key;

	for(INT ColumnIdx = 0; ColumnIdx < TagsList->GetColumnCount(); ColumnIdx++)
	{
		Key = FString::Printf(TEXT("TagCol_%i"), ColumnIdx);
		INT ColumnWidth = 250;
		GConfig->GetInt(TEXT("UIDataStorePanel"), *Key, ColumnWidth, GEditorUserSettingsIni);

		TagsList->SetColumnWidth(ColumnIdx, ColumnWidth);
	}
}

/** Saves settings for this panel. */
void WxUIDataStorePanel::SaveSettings()
{
	// Save Sash Position
	GConfig->SetInt(TEXT("UIDataStorePanel"), TEXT("SashPos"), SplitterWnd->GetSashPosition(), GEditorUserSettingsIni);

	FString Key;

	for(INT ColumnIdx = 0; ColumnIdx < TagsList->GetColumnCount(); ColumnIdx++)
	{
		Key = FString::Printf(TEXT("TagCol_%i"), ColumnIdx);
		const INT ColWidth = TagsList->GetColumnWidth(ColumnIdx);
		GConfig->SetInt(TEXT("UIDataStorePanel"), *Key, ColWidth, GEditorUserSettingsIni);
	}
}


/**
 * @return	TRUE if there is a valid tag selected.
 */
UBOOL WxUIDataStorePanel::IsTagSelected() const
{
	return TagsList != NULL && TagsList->GetSelectedItemCount() > 0;
}

/** 
 * Returns the currently selected tag in OutTagName. 
 * @return TRUE if there is a tag selected, FALSE otherwise.
 */
UBOOL WxUIDataStorePanel::GetSelectedTag(FName &OutTagName)
{
	UBOOL bResult = FALSE;
	long Selection = TagsList->GetFirstSelected();

	if(Selection > -1)
	{
		OutTagName = FName(TagsList->GetItemText(Selection));
		bResult = TRUE;
	}

	return bResult;
}

/** 
 * @return the path to the current tag if one is selected, or a blank string if one is not selected.
 */
FString WxUIDataStorePanel::GetSelectedTagPath() const
{
	FString Result;
	long Selection = TagsList->GetFirstSelected();

	if(Selection > -1)
	{
		wxString Path = TagsList->GetColumnItemText(Selection, PF_Path);
		wxString Tag = TagsList->GetItemText(Selection);
		Result = FString::Printf(TEXT("%s.%s"), (const TCHAR*)Path, (const TCHAR*)Tag);
	}

	return Result;
}

/**
 * Returns the provider field type that corresponds to the name specified
 *
 * @param	FieldTypeText	a name which corresponds to one of the values of the EUIDataProviderFieldType enum
 *
 * @return	the enum value that matches the name specified
 */
static EUIDataProviderFieldType GetTagTypeFromText( FName FieldTypeText )
{
	EUIDataProviderFieldType Result = DATATYPE_MAX;

	static UEnum* ProviderFieldTypeEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EUIDataProviderFieldType"), TRUE);
	if ( ProviderFieldTypeEnum != NULL )
	{
		INT EnumValue = ProviderFieldTypeEnum->FindEnumIndex(FieldTypeText);
		if ( EnumValue != INDEX_NONE )
		{
			Result = (EUIDataProviderFieldType)EnumValue;
		}
	}

	return Result;
}

/** 
 * @return the path to the current tag if one is selected, or a blank string if one is not selected.
 */
EUIDataProviderFieldType WxUIDataStorePanel::GetSelectedTagType() const
{
	EUIDataProviderFieldType Result = DATATYPE_MAX;

	long Selection = TagsList->GetFirstSelected();
	if(Selection > -1)
	{
		FString TypeText = TEXT("DATATYPE_");
		TypeText += TagsList->GetColumnItemText(Selection, PF_Type).c_str();

		Result = GetTagTypeFromText(*TypeText);
	}

	return Result;
}

/** 
 * @return Returns the selected dataprovider if there is one, otherwise NULL.
 */
UUIDataProvider* WxUIDataStorePanel::GetSelectedTagProvider()
{
	UUIDataProvider* Result = NULL;
	long Selection = TagsList->GetFirstSelected();

	if(Selection > -1)
	{
		Result = (UUIDataProvider*)TagsList->GetItemData(Selection);
	}

	return Result;
}

/** 
 * @return Returns the selected dataprovider using the tree control if there is one, otherwise NULL.
 */
UUIDataProvider* WxUIDataStorePanel::GetSelectedTreeProvider()
{
	UUIDataProvider* DataProvider = NULL;
	wxTreeItemId SelectedId = DataStoreTree->GetSelection();

	if(SelectedId.IsOk())
	{
		DataProvider = GetProviderFromTreeId(SelectedId);
	}

	return DataProvider;
}


/** @return Returns whether or not this panel supports a datastore and should display it in its datastore tree. */
UBOOL WxUIDataStorePanel::DataStoreSupported(const UUIDataProvider* DataStore) const
{
	return (DataStore->IsA(UUIDataStore_Strings::StaticClass()) == FALSE);
}


/**
 * Iterates over all of the available datastores and adds them to the tree.
 */
void WxUIDataStorePanel::RefreshTree()
{
	// remove all existing items
	DataStoreTree->DeleteAllItems();
	
	wxTreeItemId FirstItem;

	TagMap.Empty();
	ProviderIDMap.Empty();
	DataStoreTree->Freeze();
	{
		wxTreeItemId RootID = DataStoreTree->AddRoot (*LocalizeUI(TEXT("Root")), -1, -1, NULL);

		// Add all of the available data stores under the root item.
		const UDataStoreClient *DataClient = SceneManager->DataStoreManager;
		TArray<UUIDataStore*> AvailableDataStores;

		DataClient->GetAvailableDataStores(NULL, AvailableDataStores);

		for (INT DataStoreIndex = 0; DataStoreIndex < AvailableDataStores.Num(); DataStoreIndex++)
		{
			UUIDataProvider* DataStore = AvailableDataStores(DataStoreIndex);
			if( DataStoreSupported(DataStore) )
			{
				// Don't add the strings data
				const FName DataStoreName = AvailableDataStores(DataStoreIndex)->GetDataStoreID();

				wxTreeItemId ItemID = DataStoreTree->AppendItem (RootID, *DataStoreName.ToString(), -1, -1, new WxTreeObjectWrapper(DataStore));
				ProviderIDMap.Set(DataStore, ItemID);

				if(FirstItem.IsOk() == FALSE)
				{
					FirstItem = ItemID;
				}

				// Loop through all of the data provders for this data store and add them to the tree.
				FString CurrentPath = DataStoreName.ToString();
				TArray<FUIDataProviderField> DataFields;
				DataStore->GetSupportedDataFields(DataFields);

				for(INT ProviderIdx = 0; ProviderIdx < DataFields.Num(); ProviderIdx++)
				{
					FUIDataProviderField& DataField = DataFields(ProviderIdx);

					if( DataField.FieldType ==  DATATYPE_Provider || DataField.FieldType == DATATYPE_ProviderCollection )
					{
						AddDataProviderToTree(ItemID, DataField, CurrentPath);
					}
				}

				// Generate tags for this datastore.
				GenerateTags(DataStore, CurrentPath);
				GWarn->UpdateProgress(DataStoreIndex, AvailableDataStores.Num());
			}
		}
	}
	DataStoreTree->Thaw();

	TagListRoot = DataStoreTree->GetRootItem();
	RefreshList();
}

void WxUIDataStorePanel::RefreshList()
{
	TagsList->Freeze();
	{
		TagsList->DeleteAllItems();

		if(TagListRoot.IsOk())
		{
			// get the total number of providers in our map
			TLookupMap<UUIDataProvider*> Unused;
			const INT TotalProviderCount = TagMap.GetKeys(Unused);
			INT CurrentProgress = 0;

			AddDataProviderTagsToList(TagListRoot, TotalProviderCount, CurrentProgress);
		}	
	}
	TagsList->Thaw();
}

/**
 * Wrapper for getting the data provider associated with the specified tree id.
 */
UUIDataProvider* WxUIDataStorePanel::GetProviderFromTreeId( const wxTreeItemId& ProviderId )
{
	UUIDataProvider* Result = NULL;

	if ( DataStoreTree != NULL && ProviderId.IsOk() )
	{
		WxTreeObjectWrapper* DataWrapper = static_cast<WxTreeObjectWrapper*>(DataStoreTree->GetItemData(ProviderId));

		if(DataWrapper)
		{
			Result = DataWrapper->GetObject<UUIDataProvider>();
		}
	}

	return Result;
}

/** 
 * Adds the data provider and all of its children to the tree. 
 * @param Provider	Provider to add to the tree ctrl.
 */
void WxUIDataStorePanel::AddDataProviderToTree(wxTreeItemId &ParentID, FUIDataProviderField& ProviderData, const FString &CurrentPath)
{
	//@fixme - this needs to work correctly with data provider collections.
	TArray<UUIDataProvider*> FieldProviders;
	verify(ProviderData.GetProviders(FieldProviders));

	if(FieldProviders.Num())
	{
		UUIDataProvider* Provider = FieldProviders(0);
		wxTreeItemId ItemID = DataStoreTree->AppendItem(ParentID, *ProviderData.FieldTag.ToString(), -1, -1, new WxTreeObjectWrapper(Provider));
		ProviderIDMap.Set(Provider, ItemID);

		// Generate a new path that includes this dataprovider.
		FString NewPath;

		if ( CurrentPath.Len() )
		{
			if ( CurrentPath.InStr(TEXT(".")) == INDEX_NONE )
			{
				NewPath = CurrentPath + TEXT(":") + *ProviderData.FieldTag.ToString();
			}
			else
			{
				NewPath = CurrentPath + TEXT(".") + *ProviderData.FieldTag.ToString();
			}
		}
		else
		{
			NewPath = ProviderData.FieldTag.ToString();
		}


		// Loop through all of the child data providers for this data provider and add them to the tree.
		TArray<FUIDataProviderField> DataFields;
		Provider->GetSupportedDataFields(DataFields);

		for(INT ProviderIdx = 0; ProviderIdx < DataFields.Num(); ProviderIdx++)
		{
			FUIDataProviderField& DataField = DataFields(ProviderIdx);

			if( DataField.FieldType ==  DATATYPE_Provider || DataField.FieldType == DATATYPE_ProviderCollection )
			{
				AddDataProviderToTree(ItemID, DataField, NewPath);
			}
		}

		// Generate tag map entries for this provider.
		GenerateTags(Provider, NewPath);
	}
}


/**
 * Generates a list of tags using the provided data store. Adds the tags to the list and then recursively adds the tags for the children of this data provider to the list.
 *
 * @param	ProviderId		DataProvider to use as a source for generating properties.
 * @param	TotalProviders	the total number of providers that we need to add tags for; used to update the status bar
 * @param	ProgressCount	the number of providers that have been added to the list so far; used to update the status bar
 */
void WxUIDataStorePanel::AddDataProviderTagsToList(const wxTreeItemId& ProviderId, const INT TotalProviders, INT& ProgressCount )
{
	// If we were given a valid data store, loop through all of its available tags and add them to the UI editor.
	UUIDataProvider* DataProvider = GetProviderFromTreeId(ProviderId);
	if(DataProvider != NULL)
	{
		FString ProviderName = DataStoreTree->GetItemText(ProviderId).c_str();
		FString StatusString = FString::Printf(LocalizeSecure(LocalizeUI(TEXT("UIEditor_SlowTask_AddingProviderTags")), *ProviderName));
		//GWarn->StatusUpdatef( ++ProgressCount, TotalProviders, *StatusString );
		GWarn->UpdateProgress(++ProgressCount, TotalProviders);

		TArray<FTagInfo> Tags;
		TagMap.MultiFind(DataProvider, Tags, TRUE);

		for(INT FieldIdx=0; FieldIdx < Tags.Num(); FieldIdx++)
		{
			const FTagInfo &Tag = Tags(FieldIdx);
			UBOOL bAddTag = TRUE;

			// Check against filter.
			if(CurrentFilter.Len())
			{
				FString StrName = Tag.TagName.ToString();
				StrName = StrName.ToLower();

				
				if(appStrstr(*StrName, *CurrentFilter) != *StrName)
				{
					bAddTag = FALSE;
				}
			}

			if(bAddTag)
			{
				long ItemIdx = TagsList->InsertItem(TagsList->GetItemCount(), *Tag.TagName.ToString());

				// Keep a pointer to the tag's dataprovider.
				TagsList->SetItemData(ItemIdx, (long)DataProvider);

				// Name Field
				TagsList->SetItem(ItemIdx, PF_Name, *Tag.TagName.ToString());

				// Path Field
				TagsList->SetItem(ItemIdx, PF_Path, *(Tag.TagPath));
	
				// type Field
				TagsList->SetItem(ItemIdx, PF_Type, *Tag.TagTypeString);

				// Markup Field
				TagsList->SetItem(ItemIdx, PF_Markup, *(Tag.TagMarkup));
			}
		}
	}

	if ( bDisplayNestedTags == TRUE )
	{
		// Loop through all of the children for this provider and add their tags to the tree as well.
		wxTreeItemIdValue Cookie;
		wxTreeItemId Child = DataStoreTree->GetFirstChild(ProviderId, Cookie);

		while(Child.IsOk())
		{
			AddDataProviderTagsToList(Child, TotalProviders, ProgressCount);
			Child = DataStoreTree->GetNextChild(ProviderId, Cookie);
		}
	}
}

static void GetStatusString( FString ProviderPath, FString& out_StatusString )
{
	INT delimPos = ProviderPath.InStr(TEXT("."), TRUE);
	if ( delimPos == INDEX_NONE )
	{
		delimPos = ProviderPath.InStr(TEXT(":"), TRUE);
	}

	if ( delimPos != INDEX_NONE )
	{
		ProviderPath = ProviderPath.Mid(delimPos+1);
	}

	static FString GenertingTagsLocString = LocalizeUI(TEXT("UIEditor_SlowTask_GeneratingTags"));

	out_StatusString = FString::Printf(LocalizeSecure(GenertingTagsLocString, *ProviderPath));
}

/**
 * Generates tags for a dataprovider and stores them in a lookup map.
 *
 * @param DataProvider		DataProvider to generate tags for.
 */
void WxUIDataStorePanel::GenerateTags(UUIDataProvider* DataProvider, const FString &CurrentPath)
{
	// Erase all previous instances that use this provider.
	TagMap.RemoveKey(DataProvider);

	// Generate tags.
	TArray<FUIDataProviderField> DataFields;
	DataProvider->GetSupportedDataFields(DataFields);

	UUIDataStore* OwningDataStore = GetOwningDataStore(DataProvider);

	if(OwningDataStore && DataFields.Num())
	{
		FString StatusTag;
		GetStatusString(CurrentPath, StatusTag);

		const FString FieldDelim = CurrentPath.InStr(TEXT(":")) != INDEX_NONE ? TEXT(".") : TEXT(":");

		// Insert tags into map.
		for(INT FieldIdx=0; FieldIdx < DataFields.Num(); FieldIdx++)
		{
			FUIDataProviderField& ProviderField = DataFields(FieldIdx);
			if ( ProviderField.FieldType != DATATYPE_Provider )
			{
//				GWarn->UpdateProgress(FieldIdx, DataFields.Num());
				//GWarn->StatusUpdatef(FieldIdx, DataFields.Num(), *StatusTag);

				// @todo: Generate tag markup and save them.
				const FString FieldMarkup = FString(TEXT("<")) + CurrentPath + FieldDelim + ProviderField.FieldTag.ToString() + TEXT(">");
				TagMap.Add(DataProvider, FTagInfo(ProviderField.FieldTag, CurrentPath, (EUIDataProviderFieldType)ProviderField.FieldType, FieldMarkup) );
			}
		}
	}
}

/**
 * @param	DataProvider		Provider to find the owning datastore for.
 * @return	Returns the data store that contains the specified data provider.
 */
UUIDataStore* WxUIDataStorePanel::GetOwningDataStore( UUIDataProvider* DataProvider )
{
	UUIDataStore* Result = NULL;

	if ( DataProvider != NULL )
	{
		// get the tree item id for the currently selected
		wxTreeItemId* ProviderId = ProviderIDMap.Find(DataProvider);
		if ( ProviderId != NULL )
		{
			wxTreeItemId CurrentId = *ProviderId;
			UUIDataProvider* CurrentProvider = DataProvider;

			while ( CurrentProvider != NULL )
			{
				// if this provider is a data store, we've found the owning data store
				Result = Cast<UUIDataStore>(CurrentProvider);
				if ( Result != NULL )
				{
					break;
				}

				// otherwise, get this tree item parent's ItemId
				CurrentId = DataStoreTree->GetItemParent(CurrentId);

				// now get the provider associated with that id
				CurrentProvider = GetProviderFromTreeId(CurrentId);
			}
		}
	}

	return Result;
}

/**
 * wxWidgets Event Handler for when the tree selection changes.  This updates the property list.
 */
void WxUIDataStorePanel::OnTreeSelectionChanged(wxTreeEvent& Event)
{
	TagListRoot = DataStoreTree->GetSelection();
	RefreshList();
}

/** Reapplies a filter to the set of visible tags. */
void WxUIDataStorePanel::OnFilterTextChanged(wxCommandEvent& Event)
{
	CurrentFilter = Event.GetString();
	CurrentFilter = CurrentFilter.ToLower();
	RefreshList();
}

/* ==========================================================================================================
WxUIStringDataStorePanel
========================================================================================================== */
BEGIN_EVENT_TABLE(WxUIStringDataStorePanel, WxUIDataStorePanel)
	EVT_LIST_ITEM_SELECTED(ID_UI_LIST_DATASTOREPROPERTIES,	WxUIStringDataStorePanel::OnListSelectionChanged)
	EVT_BUTTON(ID_UI_APPLY_CHANGES,						WxUIStringDataStorePanel::OnApplyChanges)
	EVT_BUTTON(ID_UI_CANCEL_CHANGES,						WxUIStringDataStorePanel::OnCancelChanges)
	EVT_TEXT(ID_UI_DATASTORE_VALUE,							WxUIStringDataStorePanel::OnValueTextChanged)

	EVT_UPDATE_UI(ID_UI_APPLY_CHANGES,						WxUIStringDataStorePanel::OnUpdateUI)
	EVT_UPDATE_UI(ID_UI_CANCEL_CHANGES,						WxUIStringDataStorePanel::OnUpdateUI)
	EVT_UPDATE_UI(IDM_UI_ADDDATATAG,						WxUIStringDataStorePanel::OnUpdateUI)
END_EVENT_TABLE()

WxUIStringDataStorePanel::WxUIStringDataStorePanel(wxWindow* InParent, UBOOL bInDisplayNestedTags/*=TRUE*/)
: WxUIDataStorePanel(InParent, bInDisplayNestedTags)
{}

WxUIStringDataStorePanel::WxUIStringDataStorePanel(wxWindow* InParent, UUISceneManager* InSceneManager,UBOOL bInDisplayNestedTags/*=TRUE*/)
: WxUIDataStorePanel(InParent,InSceneManager,bInDisplayNestedTags)
{
	wxSizer* MainSizer = new wxBoxSizer(wxVERTICAL);
	{
		// Filter
		wxSizer* FilterSizer = new wxBoxSizer(wxHORIZONTAL);
		{
			wxStaticText* ST = new wxStaticText(this, wxID_ANY, *LocalizeUI("UIEditor_DataStoreBrowser_Filter"));
			FilterSizer->Add(ST,0, wxRIGHT | wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT, 4);

			FilterText = new wxTextCtrl(this, ID_UI_DATASTORE_FILTER);
			FilterSizer->Add(FilterText,1, wxEXPAND);
		}
		MainSizer->Add(FilterSizer, 0, wxEXPAND | wxALL, 3);

		SplitterWnd = new wxSplitterWindow(this);
		{
			DataStoreTree = new WxTreeCtrl(SplitterWnd, ID_UI_TREE_DATASTORE, NULL, wxTR_HAS_BUTTONS|wxTR_LINES_AT_ROOT|wxTR_EXTENDED|wxTR_HIDE_ROOT|wxTR_SINGLE);

			wxPanel* TempPanel = new wxPanel(SplitterWnd);

			wxSizer *VSizer = new wxBoxSizer(wxVERTICAL);
			{
				SplitterValue = new wxSplitterWindow(TempPanel);
				{
					wxPanel* PropertyPanel = new wxPanel(SplitterValue);
					{
						wxStaticBoxSizer* PropertySizer = new wxStaticBoxSizer(wxVERTICAL, PropertyPanel, *LocalizeUI(TEXT("UIEditor_DataStoreBrowser_DataStoreTags")));
						{
							TagsList = new WxListView(PropertyPanel, ID_UI_LIST_DATASTOREPROPERTIES, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
							{
								TagsList->InsertColumn(0, *LocalizeUI("UIEditor_DataStoreBrowser_Tag"));
								TagsList->InsertColumn(1, *LocalizeUI("UIEditor_DataStoreBrowser_Path"));
								TagsList->InsertColumn(2, *LocalizeUI("UIEditor_DataStoreBrowser_Type"));
								TagsList->InsertColumn(3, *LocalizeUI("UIEditor_DataStoreBrowser_Markup"));
							}
							PropertySizer->Add(TagsList, 1, wxEXPAND | wxALL, 5);
						}
						PropertyPanel->SetSizer(PropertySizer);
					}

					wxPanel* ValuePanel = new wxPanel(SplitterValue);
					{
						wxStaticBoxSizer* ValueSizer = new wxStaticBoxSizer(wxVERTICAL, ValuePanel, *LocalizeUI(TEXT("UIEditor_DataStoreBrowser_Value")));
						{
							Value = new wxTextCtrl(ValuePanel, ID_UI_DATASTORE_VALUE);
							ValueSizer->Add(Value, 1, wxEXPAND | wxALL, 5);


							wxSizer* ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
							{
								{
									wxSizer* TempSizer = new wxBoxSizer(wxVERTICAL);
									wxButton* AddTag = new wxButton(ValuePanel, IDM_UI_ADDDATATAG, *LocalizeUI("UIEditor_AddTag"));
									TempSizer->Add(AddTag, 1, wxALIGN_CENTER_HORIZONTAL);
									ButtonSizer->Add(TempSizer, 1, wxEXPAND);
								}



								{
									wxSizer* TempSizer = new wxBoxSizer(wxVERTICAL);
									wxButton* CancelChanges = new wxButton(ValuePanel, ID_UI_CANCEL_CHANGES, *LocalizeUI("UIEditor_DataStoreBrowser_Cancel"));
									TempSizer->Add(CancelChanges, 1, wxALIGN_CENTER_HORIZONTAL);
									ButtonSizer->Add(TempSizer, 1, wxEXPAND);
								}

								{
									wxSizer* TempSizer = new wxBoxSizer(wxVERTICAL);
									wxButton* AcceptChanges = new wxButton(ValuePanel, ID_UI_APPLY_CHANGES, *LocalizeUI("UIEditor_DataStoreBrowser_Apply"));
									TempSizer->Add(AcceptChanges, 1, wxALIGN_CENTER_HORIZONTAL);
									ButtonSizer->Add(TempSizer, 1, wxEXPAND);
								}
							}
							ValueSizer->Add(ButtonSizer,0,wxEXPAND);
						}
						ValuePanel->SetSizer(ValueSizer);
					}
					

					// Split the window and set default properties for the splitter.
					const INT MinPaneSize = 20;
					SplitterValue->SetMinimumPaneSize(MinPaneSize);
					SplitterValue->SplitHorizontally(PropertyPanel, ValuePanel);
					SplitterValue->SetSashPosition(300);
				}
				VSizer->Add(SplitterValue,1,wxEXPAND);
			}
			TempPanel->SetSizer(VSizer);


			// Split the window and set default properties for the splitter.
			const INT MinPaneSize = 20;
			SplitterWnd->SetMinimumPaneSize(MinPaneSize);
			SplitterWnd->SplitVertically(DataStoreTree, TempPanel);
		}
		MainSizer->Add(SplitterWnd, 1, wxEXPAND);
	}
	SetSizer(MainSizer);
	Layout();

	// Always default focus to the filter text.
	FilterText->SetFocus();
}

/** @return Returns whether or not this panel supports a datastore and should display it in its datastore tree. */
UBOOL WxUIStringDataStorePanel::DataStoreSupported(const UUIDataProvider* DataStore) const
{
	return (DataStore->IsA(UUIDataStore_Strings::StaticClass()) == TRUE);
}

/** Loads settings for this panel. */
void WxUIStringDataStorePanel::LoadSettings()
{
	// Load Sash Position
	INT SashPos = 300;
	GConfig->GetInt(TEXT("UIStringDataStorePanel"), TEXT("SashPos"), SashPos, GEditorUserSettingsIni);
	SplitterWnd->SetSashPosition(SashPos);

	SashPos=400;
	GConfig->GetInt(TEXT("UIStringDataStorePanel"), TEXT("ValueSashPos"), SashPos, GEditorUserSettingsIni);
	SplitterValue->SetSashPosition(SashPos);

	FString Key;

	for(INT ColumnIdx = 0; ColumnIdx < TagsList->GetColumnCount(); ColumnIdx++)
	{
		Key = FString::Printf(TEXT("TagCol_%i"), ColumnIdx);
		INT ColumnWidth = 250;
		GConfig->GetInt(TEXT("UIStringDataStorePanel"), *Key, ColumnWidth, GEditorUserSettingsIni);

		TagsList->SetColumnWidth(ColumnIdx, ColumnWidth);
	}
}

/** Saves settings for this panel. */
void WxUIStringDataStorePanel::SaveSettings()
{
	// Save Sash Position
	GConfig->SetInt(TEXT("UIStringDataStorePanel"), TEXT("SashPos"), SplitterWnd->GetSashPosition(), GEditorUserSettingsIni);
	GConfig->SetInt(TEXT("UIStringDataStorePanel"), TEXT("ValueSashPos"), SplitterValue->GetSashPosition(), GEditorUserSettingsIni);
	

	FString Key;

	for(INT ColumnIdx = 0; ColumnIdx < TagsList->GetColumnCount(); ColumnIdx++)
	{
		Key = FString::Printf(TEXT("TagCol_%i"), ColumnIdx);
		const INT ColWidth = TagsList->GetColumnWidth(ColumnIdx);
		GConfig->SetInt(TEXT("UIStringDataStorePanel"), *Key, ColWidth, GEditorUserSettingsIni);
	}
}

/** Adds a tag to the datastore. */
void WxUIStringDataStorePanel::AddTag()
{
	UUIDataProvider* DataProvider = GetSelectedTreeProvider();
	UUIConfigFileProvider* ConfigProvider = Cast<UUIConfigFileProvider>(DataProvider);
	FString Section;

	if(ConfigProvider == NULL)
	{
		UUIConfigSectionProvider* SectionProvider = Cast<UUIConfigSectionProvider>(DataProvider);

		if(SectionProvider)
		{
			ConfigProvider = SectionProvider->GetOuterUUIConfigFileProvider();
			Section = SectionProvider->SectionName;
		}
	}

	if(ConfigProvider)
	{
		WxDlgUICreateConfigDataTag Dlg(SceneManager->GetDataStoreBrowser(), ConfigProvider, Section);

		if(Dlg.ShowModal() == wxID_OK)
		{
			FString Section = Dlg.GetSectionName();
			FString Tag = Dlg.GetTagName();


			SaveTag(*Section, *Tag, *LocalizeUI("UIEditor_DefaultTagValue"), ConfigProvider);

			DataStoreTree->SaveSelectionExpansionState();
			{
				RefreshTree();
			}	
			DataStoreTree->RestoreSelectionExpansionState();


			// Try to find the section we just added a tag to so we can select our new tag.
			UUIConfigSectionProvider* SectionProvider = NULL;
			for(INT SectionIdx=0; SectionIdx < ConfigProvider->Sections.Num(); SectionIdx++)
			{
				UUIConfigSectionProvider* CurrentSection = ConfigProvider->Sections(SectionIdx);

				if(CurrentSection->SectionName == Section)
				{
					SectionProvider = CurrentSection;
					break;
				}
			}
			
			wxTreeItemId *SectionIdPtr = ProviderIDMap.Find(SectionProvider);

			if(SectionIdPtr)
			{
				wxTreeItemId SectionId = *SectionIdPtr;
				DataStoreTree->UnselectAll();
				DataStoreTree->SelectItem(SectionId);
				DataStoreTree->EnsureVisible(SectionId);
				TagListRoot = DataStoreTree->GetSelection();
				RefreshList();


				// Loop through all visible tags and find the tag we just added.
				long ItemCount = TagsList->GetItemCount();
				Tag = Tag.ToLower();

				for(INT TagIdx=0; TagIdx<ItemCount; TagIdx++)
				{
					wxString TestTag = TagsList->GetItemText(TagIdx);
					TestTag.MakeLower();

					if(TestTag==*Tag)
					{
						TagsList->Select(TagIdx);
						TagsList->EnsureVisible(TagIdx);
						break;
					}
				}

				ResetValue();
			}
		}
	}
}

/** Saves a config tag, checks out the loc file if necessary. */
UBOOL WxUIStringDataStorePanel::SaveTag(const TCHAR *InSection, const TCHAR *InTag, const TCHAR *InValue, UUIConfigFileProvider* ConfigProvider)
{
	const TCHAR* InFilename = *ConfigProvider->ConfigFileName;
	const UBOOL bReadOnly = GFileManager->IsReadOnly(InFilename);
	UBOOL bSaveString = FALSE;
	

	if(bReadOnly == TRUE)
	{
#if HAVE_SCC
		if ( GApp->SCC )
		{
			const UBOOL ReturnVal = appMsgf(AMT_YesNo, LocalizeSecure(LocalizeUnrealEd("CheckoutPerforce"), InFilename));

			if(ReturnVal == TRUE)
			{
				// See if the file is writable, if it isn't, try to check it out using SCC.
				if(GApp->SCC->IsOpen() == FALSE)
				{
					GApp->SCC->Open();
				}

				if(GApp->SCC->IsOpen())
				{
					GApp->SCC->CheckOut(InFilename);

					if(GFileManager->IsReadOnly(InFilename) == FALSE)
					{
						bSaveString = TRUE;
					}
				}
			}
		}
#endif // HAVE_SCC

		if(bSaveString == FALSE)
		{
			wxMessageBox( *FString::Printf(LocalizeSecure(LocalizeUI(TEXT("DlgUICreateConfigDataTag_ReadOnlyError")), InFilename)), *LocalizeUI(TEXT("DlgUICreateConfigDataTag_Error")), wxICON_ERROR);
		}
	}
	else
	{
		bSaveString = TRUE;
	}

	if(bSaveString == TRUE)
	{
		GConfig->SetString(InSection, InTag, InValue, InFilename);

		// Reinitialize the config provider since it changed.
		FConfigFile* File = GConfig->FindConfigFile(InFilename);

		if(File)
		{
			File->Write(InFilename);
			ConfigProvider->InitializeProvider(File);
		}


		bMadeChange = FALSE;
	}

	return bSaveString;
}

/** Resets the value of the value text box using the currently selected tag. */
void WxUIStringDataStorePanel::ResetValue()
{
	long SelectedItem = TagsList->GetFirstSelected();
	UBOOL bEnableValue = FALSE;
	FString ValueText;

	if(SelectedItem != -1)
	{
		UUIDataProvider* DataProvider = (UUIDataProvider*)TagsList->GetItemData(SelectedItem);

		if(DataProvider)
		{
			// See if the current dataprovider is a config provider.
			if( DataProvider->IsA(UUIConfigFileProvider::StaticClass()) || DataProvider->IsA(UUIConfigSectionProvider::StaticClass()) )
			{
				UUIConfigFileProvider* ConfigProvider = NULL;

				// Make sure that we have a section selected
				UUIConfigSectionProvider* SectionProvider = Cast<UUIConfigSectionProvider>(DataProvider);
				if( SectionProvider != NULL )
				{
					CurrentSection = SectionProvider->SectionName;

					ConfigProvider = SectionProvider->GetOuterUUIConfigFileProvider();

					// Retrieve the previous value for the data tag then display the data tag dialog with all of the values already filled in.
					if(ConfigProvider != NULL)
					{
						wxString Str = TagsList->GetItemText(SelectedItem);
						FName TagName((const TCHAR*)Str);

						const FString Filename = *ConfigProvider->ConfigFileName;
						GConfig->GetString(*CurrentSection, *TagName.ToString(), ValueText, *Filename);

						bEnableValue = TRUE;
					}
				}
			}
		}
	}

	if(bEnableValue)
	{
		Value->Enable();
		Value->SetValue(*ValueText);

	}
	else
	{
		Value->SetValue(TEXT(""));
		Value->Disable();
	}

	bMadeChange = FALSE;
}

/** Called when the user clicks on a tag in the list. */
void WxUIStringDataStorePanel::OnListSelectionChanged(wxListEvent &Event)
{
	ResetValue();
}

/** Called when the user clicks on the accept change button. */
void WxUIStringDataStorePanel::OnApplyChanges(wxCommandEvent &Event)
{


	UUIDataProvider* SelectedDataProvider = GetSelectedTagProvider();
	UUIConfigSectionProvider* SectionProvider = Cast<UUIConfigSectionProvider>(SelectedDataProvider);
	if(SectionProvider)
	{
		UUIConfigFileProvider* ConfigProvider = SectionProvider->GetOuterUUIConfigFileProvider();
		
		FName TagName;
		if(GetSelectedTag(TagName))
		{

			const wxString Section = *CurrentSection;
			const wxString Tag = *TagName.ToString();
			const wxString ValueText = Value->GetValue();
			const FFilename Filename = ConfigProvider->ConfigFileName;

			if(Section.Len() && Tag.Len() && ValueText.Len())
			{
				UBOOL bResult = SaveTag(Section, Tag, ValueText, ConfigProvider);

				if(bResult)
				{
					// Update all widgets subscribing to this datastore.
					UUIDataStore* DataStore = GetOwningDataStore(SelectedDataProvider);
					DataStore->eventRefreshSubscribers();
				}
			}
			else
			{
				wxMessageBox(*LocalizeUI(TEXT("DlgUICreateConfigDataTag_NeedText")), *LocalizeUI(TEXT("DlgUICreateConfigDataTag_Error")), wxICON_ERROR);
			}
		}
	}
}

/** Cancels changes that were made to the value for a tag. */
void WxUIStringDataStorePanel::OnCancelChanges(wxCommandEvent& Event)
{
	ResetValue();
}

void WxUIStringDataStorePanel::OnValueTextChanged(wxCommandEvent& Event)
{
	bMadeChange = TRUE;
}

/** Update UI callback for this panel. */
void WxUIStringDataStorePanel::OnUpdateUI(wxUpdateUIEvent &Event)
{
	switch(Event.GetId())
	{
	case ID_UI_APPLY_CHANGES: case ID_UI_CANCEL_CHANGES:
		Event.Enable(bMadeChange==TRUE);
		break;
	case IDM_UI_ADDDATATAG:
		{
			UBOOL bEnable = FALSE;

			UUIConfigFileProvider* ConfigProvider = Cast<UUIConfigFileProvider>(GetProviderFromTreeId(DataStoreTree->GetSelection()));

			if(ConfigProvider != NULL)
			{
				bEnable = TRUE;
			}
			else
			{
				UUIConfigSectionProvider* SectionProvider = Cast<UUIConfigSectionProvider>(GetProviderFromTreeId(DataStoreTree->GetSelection()));

				if(SectionProvider != NULL)
				{
					bEnable = TRUE;
				}
			}
			

			Event.Enable(bEnable==TRUE);
		}
		break;
	}
}

/* ==========================================================================================================
WxUIDataProviderTagsMenu
========================================================================================================== */

/**
 * Context menu for when the user right clicks on the tags list.
 */
class WxUIDataProviderTagsMenu : public wxMenu
{
public:

	WxUIDataProviderTagsMenu(UUIDataStore* ContainerDataStore, UUIDataProvider* DataProvider, const FString& Tag)
	{
		FString DataPath = DataProvider->BuildDataFieldPath(ContainerDataStore,FName(*Tag));

		FString MenuString = FString::Printf(LocalizeSecure(LocalizeUI(TEXT("UIEditor_BindDataStore_F")), *DataPath));
		Append(IDM_UI_BINDWIDGETSTODATASTORE, *MenuString);

		// This is a config file type, allow the user to add a new string to it.
		if( DataProvider->IsA(UUIConfigFileProvider::StaticClass()) || DataProvider->IsA(UUIConfigSectionProvider::StaticClass()) )
		{
			Append(IDM_UI_EDITDATATAG, *LocalizeUI( TEXT("UIEditor_EditTag") ) );
		}
	}
};

/* ==========================================================================================================
WxUIDataProviderContextMenu
========================================================================================================== */

/**
 * Context menu for when the user right clicks on the data provider tree.
 */
class WxUIDataProviderContextMenu : public wxMenu
{
public:

	WxUIDataProviderContextMenu(UUIDataProvider* DataProvider)
	{
		//@todo: This is hardcoded for the strings datastore, for phase 2 it needs to be abstracted.
		UUIDataProvider* OwnerProvider = Cast<UUIDataProvider>(DataProvider->GetOuter());
		if ( OwnerProvider == NULL )
		{
			/* @todo: Enable this again later.
			// we are the outermost data store and the strings type.
			if(DataProvider->IsA(UUIDataStore_Strings::StaticClass()))
			{
				Append(IDM_UI_ADDDATAPROVIDER, *LocalizeUI( TEXT("UIEditor_AddDataProvider") ) );
			}
			*/
		}
		else
		{
			// This is a config file type, allow the user to add a new string to it.
			if( DataProvider->IsA(UUIConfigFileProvider::StaticClass()) || DataProvider->IsA(UUIConfigSectionProvider::StaticClass()) )
			{
				Append(IDM_UI_ADDDATATAG, *LocalizeUI( TEXT("UIEditor_AddTag") ) );
			}
		}

		
	}
};



