/*=============================================================================
	DlgUISkinEditor.cpp : Specialized dialog for editing UI Skins.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "PropertyWindowManager.h"
#include "ScopedTransaction.h"

#include "EngineUIPrivateClasses.h"
#include "UnrealEdPrivateClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"

#include "UnObjectEditor.h"
#include "UnLinkedObjEditor.h"
#include "Kismet.h"
#include "UnUIEditor.h"
#include "UnUIStyleEditor.h"

#include "DlgUICursorEditor.h"
#include "DlgUISkinEditor.h"
#include "DlgUISoundCue.h"

/* ==========================================================================================================
WxDlgUISkinEditor_StyleContextMenu
========================================================================================================== */
class WxDlgUISkinEditor_StyleContextMenu : public wxMenu
{
public:
	WxDlgUISkinEditor_StyleContextMenu();

};

WxDlgUISkinEditor_StyleContextMenu::WxDlgUISkinEditor_StyleContextMenu()
: wxMenu()
{
	Append( IDM_UI_EDITSTYLE, *LocalizeUI(TEXT("UIEditor_MenuItem_EditStyle")), TEXT("") );
	AppendSeparator();
	Append( IDM_UI_CREATESTYLE, *LocalizeUI(TEXT("UIEditor_MenuItem_CreateStyle")), TEXT("") );
	Append( IDM_UI_CREATESTYLE_FROMTEMPLATE, *LocalizeUI(TEXT("UIEditor_MenuItem_CreateStyle_Selected")), TEXT("") );
	AppendSeparator();
	Append( IDM_UI_REPLACESTYLE, *LocalizeUI(TEXT("UIEditor_MenuItem_ReplaceStyle")), TEXT("") );	
	Append( IDM_UI_DELETESTYLE, *LocalizeUI(TEXT("UIEditor_MenuItem_DeleteStyle")), TEXT("") );	

}

/* ==========================================================================================================
WxDlgUISkinEditor_CursorContextMenu
========================================================================================================== */
class WxDlgUISkinEditor_CursorContextMenu : public wxMenu
{
public:
	WxDlgUISkinEditor_CursorContextMenu(UBOOL bCursorSelected);

};

WxDlgUISkinEditor_CursorContextMenu::WxDlgUISkinEditor_CursorContextMenu(UBOOL bCursorSelected)
: wxMenu()
{
	Append( IDM_UI_ADDCURSOR, *LocalizeUI(TEXT("UIEditor_MenuItem_AddCursor")), TEXT("") );

	if(bCursorSelected == TRUE)
	{
		AppendSeparator();
		Append( IDM_UI_EDITCURSOR, *LocalizeUI(TEXT("UIEditor_MenuItem_EditCursor")), TEXT("") );
		AppendSeparator();
		Append( IDM_UI_RENAMECURSOR, *LocalizeUI(TEXT("UIEditor_MenuItem_RenameCursor")), TEXT("") );	
		Append( IDM_UI_DELETECURSOR, *LocalizeUI(TEXT("UIEditor_MenuItem_DeleteCursor")), TEXT("") );	
	}
}

/* ==========================================================================================================
	WxUICursorList
========================================================================================================== */

/**
 * Derived list class that displays a context menu when right clicked.
 */
class WxUICursorList : public WxListView
{
public:
	WxUICursorList(wxWindow* InParent, WxDlgUISkinEditor *InSkinEditor);
protected:

	/** Callback for when the user right clicks on the list, displays a context menu. */
	void OnRightDown(wxMouseEvent &Event);

	/** Pointer to the skin editor that owns this object. */
	WxDlgUISkinEditor* SkinEditor;

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WxUICursorList, WxListView)
	EVT_RIGHT_DOWN(OnRightDown)
END_EVENT_TABLE()

WxUICursorList::WxUICursorList(wxWindow* Parent, WxDlgUISkinEditor *InSkinEditor) : 
WxListView(Parent,  ID_UI_LIST_CURSORS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL),
SkinEditor(InSkinEditor)
{
	
}

void WxUICursorList::OnRightDown(wxMouseEvent &Event)
{
	// Display a context menu for the list.
	WxDlgUISkinEditor_CursorContextMenu Menu(GetFirstSelected() != -1);

	FTrackPopupMenu Tracker( SkinEditor, &Menu );
	Tracker.Show();
}

/* ==========================================================================================================
	WxUISoundCueList
========================================================================================================== */
class WxDlgUISkinEditor_SoundCueContextMenu : public wxMenu
{
public:

	/**
	* Constructor
	*
	* @param	bSoundCueSelected	indicates whether an existing UI sound cue is selected
	*/
	WxDlgUISkinEditor_SoundCueContextMenu( UBOOL bSoundCueSelected )
	{
		Append( IDM_UI_ADDUISOUNDCUE, *LocalizeUI(TEXT("UIEditor_MenuItem_AddSoundCue")), TEXT("") );

		if ( bSoundCueSelected == TRUE )
		{
			Append( IDM_UI_EDITUISOUNDCUE, *LocalizeUI(TEXT("UIEditor_MenuItem_EditSoundCue")), TEXT("") );
			AppendSeparator();
			Append( IDM_UI_DELETEUISOUNDCUE, *LocalizeUI(TEXT("UIEditor_MenuItem_DeleteSoundCue")), TEXT("") );
		}
	}
};

class WxUISoundCueList : public WxListView
{
public:
	WxUISoundCueList( wxWindow* InParent, WxDlgUISkinEditor* InSkinEditor )
	: WxListView(InParent, ID_UI_LIST_SOUNDCUES, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL)
	, SkinEditor(InSkinEditor)
	{
	}

protected:

	/** Callback for when the user right clicks on the list, displays a context menu. */
	void OnRightDown(wxMouseEvent &Event)
	{
		// Display a context menu for the list.
		WxDlgUISkinEditor_SoundCueContextMenu Menu(GetFirstSelected() != -1);

		FTrackPopupMenu Tracker( SkinEditor, &Menu );
		Tracker.Show();
	}

	/** Pointer to the skin editor that owns this object. */
	WxDlgUISkinEditor* SkinEditor;

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WxUISoundCueList, WxListView)
	EVT_RIGHT_DOWN(OnRightDown)
END_EVENT_TABLE()

/* ==========================================================================================================
	WxUIStyleGroupsList
========================================================================================================== */
class WxDlgUISkinEditor_StyleGroupsContextMenu : public wxMenu
{
public:

	/**
	* Constructor
	*
	* @param	bSelected	indicates whether an existing UI group is selected
	*/
	WxDlgUISkinEditor_StyleGroupsContextMenu( UBOOL bSelected )
	{
		Append( IDM_UI_ADDGROUP, *LocalizeUI(TEXT("UIEditor_MenuItem_AddNewStyleGroup")), TEXT("") );
		if( bSelected )
		{
			Append( IDM_UI_DELETEGROUP, *LocalizeUI(TEXT("UIEditor_MenuItem_DeleteNewStyleGroup")), TEXT("") );
			Append( IDM_UI_RENAMEGROUP, *LocalizeUI(TEXT("UIEditor_MenuItem_RenameNewStyleGroup")), TEXT("") );
		}
	}
};

class WxUIStyleGroupsList : public WxListView
{
public:
	WxUIStyleGroupsList( wxWindow* InParent, WxDlgUISkinEditor* InSkinEditor )
	: WxListView(InParent, ID_UI_LIST_STYLEGROUPS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL)
	, SkinEditor(InSkinEditor)
	{
	}

	/**
	 * Displays a context menu for this list
	 *
	 * @param	SelectedItem	if valid, adds "Remove" and "Rename" items to the context menu
	 */
	void ShowContextMenu( LONG SelectedItem )
	{
		UBOOL bAllowRemoval=FALSE;
		if ( SelectedItem != -1 )
		{
			WxDlgUISkinEditor::FStyleGroupNameSortData* SortData = (WxDlgUISkinEditor::FStyleGroupNameSortData*)GetItemData(SelectedItem);
			if ( SortData == NULL || !SortData->bInherited )
			{
				bAllowRemoval = TRUE;
			}
		}
		WxDlgUISkinEditor_StyleGroupsContextMenu Menu(bAllowRemoval);

		FTrackPopupMenu Tracker( SkinEditor, &Menu );
		Tracker.Show();
	}
protected:

	/** Called when the user right-clicks an item in the list */
	void OnRightClick( wxListEvent& Event )
	{
		ShowContextMenu(Event.GetIndex());
	}

	/** Callback for when the user right clicks on an empty area of the list. */
	void OnRightUp( wxMouseEvent& Event )
	{
		ShowContextMenu(GetFirstSelected());
		Event.Skip();
	}

	/** Pointer to the skin editor that owns this object. */
	WxDlgUISkinEditor* SkinEditor;

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WxUIStyleGroupsList, WxListView)
	EVT_RIGHT_UP(OnRightUp)
	EVT_LIST_ITEM_RIGHT_CLICK(ID_UI_LIST_STYLEGROUPS, WxUIStyleGroupsList::OnRightClick )
END_EVENT_TABLE()

/* ==========================================================================================================
	WxDlgUISkinEditor
========================================================================================================== */
namespace
{
	enum ECursorField
	{
		CF_Name,
		CF_Cursor,
		CF_Style
	};

	enum EUISoundCueField
	{
		SCF_Name,
		SCF_SoundCue,
	};

	enum EUIStyleGroupsField
	{
		SGF_Name,
	};
};

IMPLEMENT_DYNAMIC_CLASS(WxDlgUISkinEditor,wxDialog)

BEGIN_EVENT_TABLE(WxDlgUISkinEditor, wxDialog)
	EVT_TREE_SEL_CHANGED(ID_UI_TREE_SKINS, WxDlgUISkinEditor::OnSkinSelected)
	EVT_TREE_ITEM_RIGHT_CLICK(ID_UI_TREE_SKINS, WxDlgUISkinEditor::OnSkinRightClick)
	
	EVT_TREE_ITEM_ACTIVATED(ID_UI_TREE_STYLE, WxDlgUISkinEditor::OnStyleActivated)
	EVT_TREE_ITEM_RIGHT_CLICK(ID_UI_TREE_STYLE, WxDlgUISkinEditor::OnStyleRightClick)

	EVT_MENU(IDM_UI_CREATESTYLE, WxDlgUISkinEditor::OnContext_CreateStyle)
	EVT_MENU(IDM_UI_CREATESTYLE_FROMTEMPLATE, WxDlgUISkinEditor::OnContext_CreateStyle)
	EVT_MENU(IDM_UI_EDITSTYLE, WxDlgUISkinEditor::OnContext_EditStyle)
	EVT_MENU(IDM_UI_REPLACESTYLE, WxDlgUISkinEditor::OnContext_ReplaceStyle)
	EVT_MENU(IDM_UI_DELETESTYLE, WxDlgUISkinEditor::OnContext_DeleteStyle)

	EVT_LIST_ITEM_ACTIVATED(ID_UI_LIST_CURSORS,  WxDlgUISkinEditor::OnCursorActivated )
	EVT_LIST_ITEM_RIGHT_CLICK(ID_UI_LIST_CURSORS, WxDlgUISkinEditor::OnCursorRightClick )

	EVT_MENU(IDM_UI_ADDCURSOR, WxDlgUISkinEditor::OnContext_AddCursor)
	EVT_MENU(IDM_UI_EDITCURSOR, WxDlgUISkinEditor::OnContext_EditCursor)
	EVT_MENU(IDM_UI_RENAMECURSOR, WxDlgUISkinEditor::OnContext_RenameCursor)
	EVT_MENU(IDM_UI_DELETECURSOR, WxDlgUISkinEditor::OnContext_DeleteCursor)

	EVT_LIST_ITEM_ACTIVATED(ID_UI_LIST_SOUNDCUES,  WxDlgUISkinEditor::OnSoundCueActivated )
	EVT_LIST_ITEM_RIGHT_CLICK(ID_UI_LIST_SOUNDCUES, WxDlgUISkinEditor::OnSoundCueRightClick )
	EVT_MENU(IDM_UI_ADDUISOUNDCUE, WxDlgUISkinEditor::OnContext_AddSoundCue)
	EVT_MENU(IDM_UI_EDITUISOUNDCUE, WxDlgUISkinEditor::OnContext_EditSoundCue)
	EVT_MENU(IDM_UI_DELETEUISOUNDCUE, WxDlgUISkinEditor::OnContext_DeleteSoundCue)

	EVT_LIST_COL_CLICK(ID_UI_LIST_STYLEGROUPS, WxDlgUISkinEditor::OnClickStyleGroupListColumn)
	EVT_MENU(IDM_UI_ADDGROUP, WxDlgUISkinEditor::OnContext_AddGroup)
	EVT_MENU(IDM_UI_RENAMEGROUP, WxDlgUISkinEditor::OnContext_RenameGroup)
	EVT_MENU(IDM_UI_DELETEGROUP, WxDlgUISkinEditor::OnContext_DeleteGroup)

END_EVENT_TABLE()

namespace
{
	int wxCALLBACK WxDlgUISkinEditor_StyleGroupName_ListSort( long InItem1, long InItem2, long InSortData )
	{
		WxDlgUISkinEditor::FStyleGroupNameSortData* A = (WxDlgUISkinEditor::FStyleGroupNameSortData*)InItem1;
		WxDlgUISkinEditor::FStyleGroupNameSortData* B = (WxDlgUISkinEditor::FStyleGroupNameSortData*)InItem2;
		WxDlgUISkinEditor::FSortOptions* SortOptions = (WxDlgUISkinEditor::FSortOptions*)InSortData;

		INT Result = B->bInherited - A->bInherited;
		if ( Result == 0 )
		{
			Result = appStricmp(*A->StyleGroupName, *B->StyleGroupName);

			// If we are sorting backwards, invert the string comparison result.
			if( !SortOptions->bSortAscending )
			{
				Result *= -1;
			}
		}

		return Result;
	}
}


WxDlgUISkinEditor::WxDlgUISkinEditor()
: SkinTree(NULL), StyleTree(NULL), CursorList(NULL)
, GroupsList(NULL), SoundCueList(NULL), SplitterWnd(NULL)
, SelectedSkin(NULL)
{
}

/**
 * Initialize this dialog.  Must be the first function called after creating the dialog.
 *
 * @param	InParent	the window that opened this dialog
 * @param	InSkin		the skin to edit
 */
void WxDlgUISkinEditor::Create( wxWindow* InParent, UUISkin* InSkin )
{
	SelectedSkin = InSkin;
	verify(wxDialog::Create(InParent, wxID_ANY, *LocalizeUI(TEXT("DlgUISkinEditor_Title")),wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX | wxSYSTEM_MENU | wxRESIZE_BORDER));

	// Create controls for this dialog.
	wxSizer* MainSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(MainSizer);	
	{
		// Create a splitter window with a skin and style tree.
		SplitterWnd = new wxSplitterWindow(this);
		SplitterWnd->SetMinimumPaneSize(20);
		{
			// Skin Tree
			wxPanel* SkinPanel = new wxPanel(SplitterWnd);
			{
				wxStaticBoxSizer *SkinSizer = new wxStaticBoxSizer(wxVERTICAL, SkinPanel, *LocalizeUI(TEXT("DlgUISkinEditor_Skin")));
				{
					SkinTree = new WxTreeCtrl(SkinPanel, ID_UI_TREE_SKINS, NULL, wxTR_HAS_BUTTONS|wxTR_LINES_AT_ROOT);
					SkinSizer->Add(SkinTree, 1, wxEXPAND | wxALL, 3);
				}
				SkinPanel->SetSizer(SkinSizer);
			}
	
			// Skin Notebook
			wxNotebook* Notebook = new wxNotebook(SplitterWnd, wxID_ANY);
			{
				// Style Tree Page
				wxPanel* StylePanel = new wxPanel(Notebook);
				{
					wxSizer *StyleSizer = new wxBoxSizer(wxVERTICAL);
					{
						StyleTree = new WxTreeCtrl(StylePanel, ID_UI_TREE_STYLE, NULL, wxTR_HAS_BUTTONS|wxTR_LINES_AT_ROOT);

						wxImageList *ImageList = new wxImageList (16, 15);
						{
							// These match order of EStyleIcon
							char *ImageNames[] = 
							{
								"Style_Text",
									"Style_Image",
									"Style_Combo",
									0
							};

							for (INT NameIdx = 0; ImageNames[NameIdx]; ++NameIdx)
							{
								ImageList->Add (WxBitmap (ImageNames[NameIdx]), wxColor (192, 192, 192));
							}
						}
						StyleTree->AssignImageList(ImageList);

						StyleSizer->Add(StyleTree, 1, wxEXPAND | wxALL, 3);
					}
					StylePanel->SetSizer(StyleSizer);
				}
				Notebook->AddPage(StylePanel, *LocalizeUI(TEXT("DlgUISkinEditor_Styles")));

				// Style Groups Panel
				wxPanel* StyleGroupsPanel = new wxPanel(Notebook);
				{

					wxSizer *GroupsSizer = new wxBoxSizer(wxVERTICAL);
					{
						GroupsList = new WxUIStyleGroupsList(StyleGroupsPanel, this);
						GroupsSizer->Add(GroupsList, 1, wxEXPAND | wxALL, 3);
					}
					StyleGroupsPanel->SetSizer(GroupsSizer);
				}
				Notebook->AddPage(StyleGroupsPanel, *LocalizeUI("DlgUISkinEditor_StyleGroups"));

				// Cursor Page
				wxPanel* CursorPanel = new wxPanel(Notebook);
				{
					wxSizer *CursorSizer = new wxBoxSizer(wxVERTICAL);
					{
						CursorList = new WxUICursorList(CursorPanel, this);
						CursorSizer->Add(CursorList, 1, wxEXPAND | wxALL, 3);
					}
					CursorPanel->SetSizer(CursorSizer);
				}
				Notebook->AddPage(CursorPanel, *LocalizeUI(TEXT("DlgUISkinEditor_Cursors")));

				// Sound Cue Panel
				wxPanel* SoundCuePanel = new wxPanel(Notebook);
				{
					wxSizer* SoundCueSizer = new wxBoxSizer(wxVERTICAL);
					{
						SoundCueList = new WxUISoundCueList(SoundCuePanel, this);
						SoundCueSizer->Add(SoundCueList, 1, wxEXPAND|wxALL, 3);
					}
					SoundCuePanel->SetSizer(SoundCueSizer);
				}
				Notebook->AddPage(SoundCuePanel, *LocalizeUI("DlgUISkinEditor_SoundCues"));
			}
			
			SplitterWnd->SplitVertically(SkinPanel, Notebook);
		}
		MainSizer->Add(SplitterWnd, 1, wxEXPAND | wxALL, 5);
		
		// OK/Cancel Buttons
		wxSizer *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
		{
			wxButton* ButtonOK = new wxButton(this, wxID_OK, *LocalizeUnrealEd("&OK"));
			ButtonOK->SetDefault();

			ButtonSizer->Add(ButtonOK, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

			wxButton* ButtonCancel = new wxButton(this, wxID_CANCEL, *LocalizeUnrealEd("&Cancel"));
			ButtonSizer->Add(ButtonCancel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
		}
		MainSizer->Add(ButtonSizer, 0, wxALIGN_RIGHT|wxALL, 2);
	}

	InitializeValues(SelectedSkin);

	// we need to know about undo/redo operations
	GCallbackEvent->Register(CALLBACK_Undo, this);

	// Load the window size/pos.
	FWindowUtil::LoadPosSize(TEXT("DlgUISkinEditor"), this, 256, 256, 500, 400);

	// splitter positions
	{
		INT SplitterPos = 160;
		GConfig->GetInt(TEXT("DlgUISkinEditor"), TEXT("MainSplitterPos"), SplitterPos, GEditorUserSettingsIni);
		SplitterWnd->SetSashPosition(SplitterPos);
	}
}

WxDlgUISkinEditor::~WxDlgUISkinEditor()
{
	CleanupSortData();

	GCallbackEvent->UnregisterAll(this);

	// Save the window size/pos.
	FWindowUtil::SavePosSize(TEXT("DlgUISkinEditor"), this);
	GConfig->SetInt(TEXT("DlgUISkinEditor"), TEXT("MainSplitterPos"), SplitterWnd->GetSashPosition(), GEditorUserSettingsIni);
}

void WxDlgUISkinEditor::Send( ECallbackEventType EventType )
{
	switch ( EventType )
	{
	case CALLBACK_Undo:
        UpdateSkinProperties();
		break;
	}
}

void WxDlgUISkinEditor::InitializeValues(UUISkin* InSelectedSkin)
{
	// Create the skin combo and set the selected skin.
	SelectedSkin = InSelectedSkin;
	RebuildSkinTree();

	// Update skin properties using the currently selected skin.
	UpdateSkinProperties();
}

/** Updates the widgets that represent skin properties to reflect the properties of the currently selected skin. */
void WxDlgUISkinEditor::UpdateSkinProperties()
{
	StyleTree->SaveSelectionExpansionState();
	{
		RebuildStyleTree();
	}
	StyleTree->RestoreSelectionExpansionState();

	RebuildStyleGroups();
	RebuildCursorList();
	RebuildSoundCueList();
}

/** Rebuilds the skin tree with all of the currently available skins. */
void WxDlgUISkinEditor::RebuildSkinTree()
{
	SkinTree->DeleteAllItems();
	SkinIDMap.Empty();

	SkinTree->Freeze();
	{
		wxTreeItemId RootId = SkinTree->AddRoot(*LocalizeUI(TEXT("DlgUISkinEditor_Skin")));

		// Loop through and all of the available skins to the combo box.
		for ( TObjectIterator<UUISkin> It; It; ++It )
		{
			UUISkin* Skin = *It;

			// add this skin to the list
			if ( Skin != NULL && !Skin->IsTemplate() )
			{
				AddSkin(Skin);
			}
		}

		SkinTree->Expand(RootId);
	}
	SkinTree->Thaw();
}

/** 
 * Adds a skin and all of its children to the tree. 
 * @param Skin		Skin to add to the tree.
 * @return			The TreeItemId of the item that was added to the tree.
 */
wxTreeItemId WxDlgUISkinEditor::AddSkin(UUISkin* Skin)
{
	wxTreeItemId SkinId;
	
	wxTreeItemId* SearchId = SkinIDMap.Find(Skin);
	if(SearchId == NULL)
	{
		wxTreeItemId ParentId;
		UUISkin* SkinTemplate = CastChecked<UUISkin>(Skin->GetArchetype());

		// if this style is not based on any other style, add it to the tree as a child of root
		if ( SkinTemplate->HasAnyFlags(RF_ClassDefaultObject) )
		{
			ParentId = SkinTree->GetRootItem();
		}
		else
		{
			// if this style is based on another style, add it to the tree first.
			ParentId = AddSkin(SkinTemplate);
		}

		// Initialize the skin's lookup tables.
		Skin->Initialize();


		// Add the skin to the tree control.
		SkinId = SkinTree->AppendItem(ParentId, *Skin->GetPathName(), -1, -1,new WxTreeObjectWrapper( Skin ) );
		SkinIDMap.Set(Skin, SkinId);

		if(Skin == SelectedSkin)
		{
			SkinTree->SelectItem(SkinId);
			SkinTree->EnsureVisible(SkinId);
			SkinTree->Expand(SkinId);
		}
	}
	else
	{
		SkinId = *SearchId;
	}

	return SkinId;
}

/** 
 * Adds a style group to the group list.
 *
 * @param GroupName	Name of the group being added.
 */
void WxDlgUISkinEditor::AddGroup(const FString& GroupName, UBOOL bInherited, UBOOL bAutosizeColumns/*=FALSE*/ )
{
	long ItemId = GroupsList->InsertItem( GroupsList->GetItemCount(), *GroupName );

	// if this group is inherited from a parent style, make it italic
	wxFont Font = GroupsList->GetItemFont(ItemId);
	if ( bInherited )
	{
		Font.SetStyle(wxFONTSTYLE_ITALIC);
		Font.SetWeight(wxFONTWEIGHT_LIGHT);
	}
	else
	{
		Font.SetStyle(wxFONTSTYLE_NORMAL);
		Font.SetWeight(wxFONTWEIGHT_BOLD);
	}
	GroupsList->SetItemFont(ItemId, Font);

	// this memory will be cleaned up by CleanupSortData()
	FStyleGroupNameSortData* SortData = new FStyleGroupNameSortData(GroupName, bInherited);
	GroupsList->SetItemData(ItemId, (LONG)SortData);

	if ( bAutosizeColumns )
	{
		GroupsList->SetColumnWidth(SGF_Name, GroupsList->GetItemCount() > 0 ? wxLIST_AUTOSIZE : wxLIST_AUTOSIZE_USEHEADER);
	}
}

/** 
 * Adds a cursor to the cursor list.
 *
 * @param CursorName	Name of the cursor being added.
 * @param CursorInfo	Info about the cursor being added, including its texture and style.
 */
void WxDlgUISkinEditor::AddCursor(const FName& CursorName, const FUIMouseCursor* CursorInfo, UBOOL bAutosizeColumns/*=FALSE*/)
{
	long ItemIndex = CursorList->InsertItem(CursorList->GetItemCount(), *CursorName.ToString());
	USurface* Surface = CursorInfo->Cursor->ImageTexture;

	if ( Surface != NULL )
	{
		CursorList->SetItem(ItemIndex, CF_Cursor, *Surface->GetPathName());
	}

	CursorList->SetItem(ItemIndex, CF_Style, *CursorInfo->CursorStyle.ToString());

	if ( bAutosizeColumns )
	{
		int ColAutoSizeMode = wxLIST_AUTOSIZE;
		if ( CursorList->GetItemCount() == 0 )
		{
			ColAutoSizeMode = wxLIST_AUTOSIZE_USEHEADER;
		}

		CursorList->SetColumnWidth(CF_Cursor, ColAutoSizeMode);
		CursorList->SetColumnWidth(CF_Style, ColAutoSizeMode);
	}
}

/** 
 * Adds a new UISoundCue to the list of sound cues
 *
 * @param	SoundCueName		the alias used for referencing the sound cue specified
 * @param	SoundToPlay			the actual sound cue associated with this alias
 * @param	bAutosizeColumns	specify TRUE to force the column widths to be resized to fit the width of the text
 */
void WxDlgUISkinEditor::AddSoundCue(const FName& SoundCueName, USoundCue* SoundToPlay, UBOOL bAutosizeColumns/*=FALSE*/)
{
	long ItemIndex = SoundCueList->FindItem(-1, *SoundCueName.ToString());
	if ( ItemIndex == -1 )
	{
		ItemIndex = SoundCueList->InsertItem(SoundCueList->GetItemCount(), *SoundCueName.ToString());
	}

	SoundCueList->SetItemData(ItemIndex, (long)SoundToPlay);
	SoundCueList->SetItem(ItemIndex, SCF_SoundCue, SoundToPlay != NULL ? *SoundToPlay->GetPathName() : *LocalizeUnrealEd(TEXT("None")));

	if ( bAutosizeColumns )
	{
		int ColAutoSizeMode = wxLIST_AUTOSIZE;
		if ( SoundCueList->GetItemCount() == 0 )
		{
			ColAutoSizeMode = wxLIST_AUTOSIZE_USEHEADER;
		}

		SoundCueList->SetColumnWidth(SCF_Name, ColAutoSizeMode);
		SoundCueList->SetColumnWidth(SCF_SoundCue, ColAutoSizeMode);
	}
}

/** Rebuilds the tree control with all of the styles that this skin supports. */
void WxDlgUISkinEditor::RebuildStyleTree()
{
	StyleTree->Freeze();

	StyleTree->DeleteAllItems();
	StyleIDMap.Empty();
	{
		wxTreeItemId RootId = StyleTree->AddRoot(*LocalizeUI(TEXT("DlgUISkinEditor_Styles")));

		// Add style nodes.
		for(TMap< FSTYLE_ID, UUIStyle* >::TIterator It(SelectedSkin->StyleLookupTable); It; ++It)
		{
			AddStyle(It.Value());
		}

		StyleTree->Expand(RootId);
	}
	StyleTree->Thaw();
}

void WxDlgUISkinEditor::RebuildStyleGroups()
{
	GroupsList->Freeze();
	{
		CleanupSortData();

		GroupsList->DeleteAllItems();
		GroupsList->DeleteAllColumns();

		GroupsList->InsertColumn(SGF_Name, *LocalizeUI(TEXT("DlgUISkinEditor_Name")));

		// Add groups
		for ( INT GroupIndex = 0; GroupIndex < SelectedSkin->StyleGroupMap.Num(); GroupIndex++ )
		{
			const FString& GroupName = SelectedSkin->StyleGroupMap(GroupIndex);
			const UBOOL bIsInherited = SelectedSkin->IsInheritedGroupName(GroupName);
			AddGroup(GroupName, bIsInherited);
		}
		GroupsList->SetColumnWidth(SGF_Name, GroupsList->GetItemCount() > 0 ? wxLIST_AUTOSIZE : wxLIST_AUTOSIZE_USEHEADER);

		//@todo ronp - sorting support
		GroupsList->SortItems(WxDlgUISkinEditor_StyleGroupName_ListSort, (LONG)&StyleGroupSortOptions);
	}
	GroupsList->Thaw();
}

/**
 * Cleans up all memory allocated for sorting data which was stored in the list directly.
 *
 * @param	ItemIndex	the index of the item to cleanup; if INDEX_NONE, cleans up the memory for all item data
 */
void WxDlgUISkinEditor::CleanupSortData( INT ItemIndex/*=INDEX_NONE*/ )
{
	if ( GroupsList != NULL )
	{
		const INT GroupCount = GroupsList->GetItemCount();
		if ( ItemIndex == INDEX_NONE )
		{
			for ( INT Idx = 0; Idx < GroupCount; Idx++ )
			{
				FStyleGroupNameSortData* SortData = (FStyleGroupNameSortData*)GroupsList->GetItemData(Idx);
				if ( SortData != NULL )
				{
					delete SortData;
				}

				GroupsList->SetItemData(Idx, 0);
			}
		}
		else if ( ItemIndex >= 0 && ItemIndex < GroupCount )
		{
			FStyleGroupNameSortData* SortData = (FStyleGroupNameSortData*)GroupsList->GetItemData(ItemIndex);
			if ( SortData != NULL )
			{
				delete SortData;
			}

			GroupsList->SetItemData(ItemIndex, 0);
		}
	}
}

/** Rebuilds the cursor list with all of the cursors that this skin supports. */
void WxDlgUISkinEditor::RebuildCursorList()
{
	CursorList->Freeze();
	{
		CursorList->DeleteAllItems();
		CursorList->DeleteAllColumns();

		CursorList->InsertColumn(CF_Name, *LocalizeUI(TEXT("DlgUISkinEditor_Name")));
		CursorList->InsertColumn(CF_Cursor, *LocalizeUI(TEXT("DlgUISkinEditor_CursorTexture")));
		CursorList->InsertColumn(CF_Style, *LocalizeUI(TEXT("DlgUISkinEditor_Style")));

		CursorList->SetColumnWidth(CF_Name, 75);
		CursorList->SetColumnWidth(CF_Cursor, 100);
		CursorList->SetColumnWidth(CF_Style, 150);

		// Add cursors
		for(TMap< FName, FUIMouseCursor >::TIterator It(SelectedSkin->CursorMap); It; ++It)
		{
			const FName Name = It.Key();
			const FUIMouseCursor *Cursor = &It.Value();
			
			AddCursor(Name, Cursor);
		}

		int ColAutoSizeMode = wxLIST_AUTOSIZE;
		if ( CursorList->GetItemCount() == 0 )
		{
			ColAutoSizeMode = wxLIST_AUTOSIZE_USEHEADER;
		}

		CursorList->SetColumnWidth(CF_Name, ColAutoSizeMode);
		CursorList->SetColumnWidth(CF_Cursor, ColAutoSizeMode);
		CursorList->SetColumnWidth(CF_Style, ColAutoSizeMode);
	}
	CursorList->Thaw();
}

/** Rebuilds the sound cue list with all of the UISoundCues that this skin supports */
void WxDlgUISkinEditor::RebuildSoundCueList()
{
	SoundCueList->Freeze();
	{
		SoundCueList->DeleteAllItems();
		SoundCueList->DeleteAllColumns();

		SoundCueList->InsertColumn(SCF_Name, *LocalizeUI(TEXT("DlgUISkinEditor_Name")));
		SoundCueList->InsertColumn(SCF_SoundCue, *LocalizeUI(TEXT("DlgUISkinEditor_SoundCueSound")));

		SoundCueList->SetColumnWidth(SCF_Name, 75);
		SoundCueList->SetColumnWidth(SCF_SoundCue, 100);

		TArray<FUISoundCue> SkinSoundCues;
		SelectedSkin->GetSkinSoundCues(SkinSoundCues);

		for ( INT SoundCueIndex = 0; SoundCueIndex < SkinSoundCues.Num(); SoundCueIndex++ )
		{
			FUISoundCue& SoundCue = SkinSoundCues(SoundCueIndex);
			AddSoundCue(SoundCue.SoundName, SoundCue.SoundToPlay);
		}

		int ColAutoSizeMode = wxLIST_AUTOSIZE;
		if ( SoundCueList->GetItemCount() == 0 )
		{
			ColAutoSizeMode = wxLIST_AUTOSIZE_USEHEADER;
		}

		SoundCueList->SetColumnWidth(SCF_Name, ColAutoSizeMode);
		SoundCueList->SetColumnWidth(SCF_SoundCue, ColAutoSizeMode);

	}
	SoundCueList->Thaw();
}

/** 
 * Adds a style and all of its children to the tree. 
 * @param Style		Style to add to the tree.
 * @return			The TreeItemId of the item that was added to the tree.
 */
wxTreeItemId WxDlgUISkinEditor::AddStyle(UUIStyle* Style)
{
	checkSlow(Style);

	// skip it if this style has been added (which might occur if another style was based on this one).
	wxTreeItemId StyleId;
	wxTreeItemId* ItemId = StyleIDMap.Find(Style);

	if ( ItemId == NULL )
	{
		wxTreeItemId ParentId;
		UUIStyle* StyleTemplate = CastChecked<UUIStyle>(Style->GetArchetype());

		// If this style has the same ID as its template then its parent is a style we replaced, so add the style as a child of its grandparent.
		if(StyleTemplate->StyleID == Style->StyleID)
		{
			StyleTemplate = Cast<UUIStyle>(StyleTemplate->GetArchetype());
		}

		// if this style is not based on any other style, add it to the tree as a child of root
		if ( StyleTemplate->HasAnyFlags(RF_ClassDefaultObject) )
		{
			ParentId = StyleTree->GetRootItem();
		}
		else
		{
			ParentId = AddStyle(StyleTemplate);
		}

		// Find a icon for this style depending on its type. 
		INT StyleIcon = -1;

		if(Style->StyleDataClass->IsChildOf(UUIStyle_Combo::StaticClass()) == TRUE)
		{
			StyleIcon = SI_Combo;
		}
		else if(Style->StyleDataClass->IsChildOf(UUIStyle_Image::StaticClass()) == TRUE)
		{
			StyleIcon = SI_Image;
		}
		else if(Style->StyleDataClass->IsChildOf(UUIStyle_Text::StaticClass()) == TRUE)
		{
			StyleIcon = SI_Text;
		}


		StyleId = StyleTree->AppendItem(ParentId, *Style->GetStyleName(), StyleIcon, StyleIcon, new WxTreeObjectWrapper(Style));
		StyleIDMap.Set(Style, StyleId);
		

		// See if this style has been especially created for this skin, if it has, display normal text, otherwise display italicized text.
		if(Style->GetOuter() != SelectedSkin)
		{
			wxFont Font = StyleTree->GetItemFont(StyleId);
			Font.SetStyle(wxFONTSTYLE_ITALIC);
			Font.SetWeight(wxFONTWEIGHT_LIGHT);
			StyleTree->SetItemFont(StyleId, Font);
		}
		else
		{
			wxFont Font = StyleTree->GetItemFont(StyleId);
			Font.SetStyle(wxFONTSTYLE_NORMAL);
			Font.SetWeight(wxFONTWEIGHT_BOLD);
			StyleTree->SetItemFont(StyleId, Font);
		}
	}
	else
	{
		StyleId = *ItemId;
	}

	return StyleId;
}


/**
 * @return Returns the currently selected style.
 */
UUIStyle* WxDlgUISkinEditor::GetSelectedStyle() const
{
	wxTreeItemId SelectedItem = StyleTree->GetSelection();
	UUIStyle* Style = NULL;
	if(SelectedItem.IsOk())
	{
		WxTreeObjectWrapper* ItemData = (WxTreeObjectWrapper*)SkinTree->GetItemData(SelectedItem);
		if ( ItemData != NULL )
		{
			Style = ItemData->GetObject<UUIStyle>();
		}
	}
	
	return Style;
}


/**
 * @return Returns the currently selected skin.
 */
UUISkin* WxDlgUISkinEditor::GetSelectedSkin() const
{
	return SelectedSkin;
}

UBOOL WxDlgUISkinEditor::IsImplementedStyleSelected() const
{
	UUIStyle* Style = GetSelectedStyle();
	UUISkin* Skin = GetSelectedSkin();

	if(Style != NULL && Skin != NULL)
	{
		if(Style->GetOuter() == Skin)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Displays the "edit style" dialog to allow the user to modify the specified style.
 * @param StyleToEdit	Style to edit in the dialog we spawn.
 */
void WxDlgUISkinEditor::DisplayEditStyleDialog( UUIStyle* StyleToEdit )
{
	WxDlgEditUIStyle* Dialog = new WxDlgEditUIStyle;
	Dialog->Create(StyleToEdit, this);
	Dialog->Show();
}

/**
 * Displays the "edit cursor" dialog to allow the user to modify the specified cursor.
 * @param CursorName	Name of the cursor to modify.
 */
void WxDlgUISkinEditor::DisplayEditCursorDialog( FName& CursorName )
{
	if(SelectedSkin != NULL)
	{
		FUIMouseCursor* CursorInfo = SelectedSkin->CursorMap.Find(CursorName);

		if(CursorInfo != NULL)
		{
			WxDlgUICursorEditor *Dlg = new WxDlgUICursorEditor(this, SelectedSkin, *CursorName.ToString(), CursorInfo);
			Dlg->Show();
		}
	}
}

/** Reinitializes the style pointers for all currently active widgets. */
void WxDlgUISkinEditor::ReinitializeAllWidgetStyles() const
{
	// Reset the active skin for all clients, this will force all widgets to reinitialize their style pointers.
	UUISceneManager *SceneManager = GUnrealEd->GetBrowserManager()->UISceneManager;
	for ( INT SceneIndex = 0; SceneIndex < SceneManager->SceneClients.Num(); SceneIndex++ )
	{
		UEditorUISceneClient* Client = SceneManager->SceneClients(SceneIndex);
		Client->OnActiveSkinChanged();
	}
}

/** Updates the skin options when the user changes the selected skin. */
void WxDlgUISkinEditor::OnSkinSelected( wxTreeEvent &Event )
{
	UUISkin* Result = NULL;
	if ( SkinTree->GetRootItem().IsOk() )
	{
		wxTreeItemId SelectedItemId = SkinTree->GetSelection();
		if ( SelectedItemId.IsOk() )
		{
			WxTreeObjectWrapper* ItemData = (WxTreeObjectWrapper*)SkinTree->GetItemData(SelectedItemId);
			if ( ItemData != NULL )
			{
				Result = ItemData->GetObject<UUISkin>();

				if(Result != NULL && Result != SelectedSkin)
				{
					SelectedSkin = Result;
					UpdateSkinProperties();
				}
			}
		}
	}
}

/** Shows a skin tree control context menu. */
void WxDlgUISkinEditor::OnSkinRightClick( wxTreeEvent &Event )
{
	
}

/** Displays the edit style dialog if the style is implemented in the selected skin. */
void WxDlgUISkinEditor::OnStyleActivated ( wxTreeEvent &Event )
{
	UUIStyle* StyleToEdit = GetSelectedStyle();

	if ( StyleToEdit != NULL && IsImplementedStyleSelected() == TRUE )
	{
		DisplayEditStyleDialog(StyleToEdit);
	}
}

/** Shows a style tree control context menu. */
void WxDlgUISkinEditor::OnStyleRightClick( wxTreeEvent &Event )
{
	WxDlgUISkinEditor_StyleContextMenu Menu;

	UUIStyle* Style = GetSelectedStyle();
	if(Style != NULL)
	{
		const UBOOL bImplementedStyle = IsImplementedStyleSelected();
		const UBOOL bDefaultStyleSelected = Style->IsDefaultStyle();

		// Don't let the user edit the style if it is inherited.  Don't let them replace a style if it is implemented.
		if(bImplementedStyle == FALSE)
		{
			Menu.Enable(IDM_UI_EDITSTYLE, false);
		}
		else
		{
			Menu.Enable(IDM_UI_REPLACESTYLE, false);
		}

		// Don't let the user delete inherited or default styles.
		if(bDefaultStyleSelected == TRUE || bImplementedStyle == FALSE)
		{
			Menu.Enable(IDM_UI_DELETESTYLE, false);
		}

		// Change the caption for the create and replace options to something more specific.
		Menu.SetLabel(IDM_UI_CREATESTYLE_FROMTEMPLATE, *FString::Printf( LocalizeSecure(LocalizeUI(TEXT("UIEditor_MenuItem_CreateStyle_TemplateF")), *Style->GetStyleName()) ) );
		Menu.SetLabel(IDM_UI_REPLACESTYLE, *FString::Printf( LocalizeSecure(LocalizeUI(TEXT("UIEditor_MenuItem_ReplaceStyleF")), *Style->GetStyleName()) ) );

		FTrackPopupMenu Tracker( this, &Menu );
		Tracker.Show();
	}

}

/** Called when the user clicks a column in the Style Groups list */
void WxDlgUISkinEditor::OnClickStyleGroupListColumn( wxListEvent& Event )
{
// 	WxListView* List = wxDynamicCast(Event.GetEventObject(), WxListView);

/*

	INT SortIdx = 0;

	if(Event.GetId() == ID_UI_AVAILABLELIST)
	{
		SortIdx = 0;
	}
	else
	{
		SortIdx = 1;
	}
*/

	if ( GroupsList != NULL )
	{
		// Make it so that if the user clicks on the same column twice, that it reverses the way the list is sorted.
		if(StyleGroupSortOptions.Column == Event.GetColumn())
		{
			StyleGroupSortOptions.bSortAscending = !StyleGroupSortOptions.bSortAscending;
		}
		else
		{
			StyleGroupSortOptions.Column = Event.GetColumn();
		}

		GroupsList->SortItems(WxDlgUISkinEditor_StyleGroupName_ListSort, (LONG)&StyleGroupSortOptions);
	}
}


/** Called when the user selects the "Create Style" context menu option */
void WxDlgUISkinEditor::OnContext_CreateStyle( wxCommandEvent& Event )
{
	WxDlgCreateUIStyle dlg;
	dlg.Create(this);

	UUIStyle* StyleTemplate = Event.GetId() == IDM_UI_CREATESTYLE_FROMTEMPLATE
		? GetSelectedStyle()
		: NULL;

	if ( dlg.ShowModal(TEXT(""),TEXT(""), StyleTemplate) == wxID_OK )
	{
		// create the new style
		UClass* StyleClass = dlg.GetStyleClass();
		UUIStyle* StyleTemplate = dlg.GetStyleTemplate();
		FString StyleFriendlyName = dlg.GetFriendlyName();
		FString StyleTag = dlg.GetStyleTag();
		UUISkin* CurrentSkin = GetSelectedSkin();

		check(CurrentSkin);
		check(StyleClass);

		CurrentSkin->SetFlags(RF_Transactional);

		FScopedTransaction Transaction( *LocalizeUI(TEXT("TransCreateStyle")) );

		UUIStyle* NewStyle = CurrentSkin->CreateStyle(StyleClass, *StyleTag, StyleTemplate, TRUE);
		if ( NewStyle != NULL )
		{
			NewStyle->StyleName = StyleFriendlyName;

			// Regenerates the style tree.
			UpdateSkinProperties();

			// get the wxTreeItemId associated with the new style
			wxTreeItemId* ItemId = StyleIDMap.Find(NewStyle);
			check(ItemId);

			// ensure it's visible in the style browser
			StyleTree->SelectItem(*ItemId);
			StyleTree->EnsureVisible(*ItemId);
		}
		else
		{
			Transaction.Cancel();
		}
	}
}

/** Called when the user selects the "Edit Style" context menu option */
void WxDlgUISkinEditor::OnContext_EditStyle( wxCommandEvent& Event )
{
	UUIStyle* StyleToEdit = GetSelectedStyle();
	if ( StyleToEdit != NULL )
	{
		// Only let the user edit the style if it is not inherited.
		if(IsImplementedStyleSelected() == TRUE)
		{
			DisplayEditStyleDialog(StyleToEdit);
		}
	}
	else
	{
		Event.Skip();
	}
}

/** Called when the user selects the "Replace Style" context menu option */
void WxDlgUISkinEditor::OnContext_ReplaceStyle( wxCommandEvent& Event )
{	
	UUIStyle* StyleTemplate = GetSelectedStyle();
	UUISkin* CurrentSkin = GetSelectedSkin();
	
	if(StyleTemplate && CurrentSkin)
	{
		// Make sure that we should delete this style by checking to see if any widgets are currently referencing this style.
		// If so, display a dialog and let the user confirm that they want to delete the style.
		const UUISceneManager *SceneManager = GUnrealEd->GetBrowserManager()->UISceneManager;
		const UBOOL bReplaceStyle = SceneManager->ShouldDeleteStyle(StyleTemplate, TRUE);

		if(bReplaceStyle == TRUE)
		{
			CurrentSkin->SetFlags(RF_Transactional);

			FScopedTransaction Transaction( *LocalizeUI(TEXT("TransReplaceStyle")) );

			UUIStyle* NewStyle = CurrentSkin->ReplaceStyle(StyleTemplate);
			if ( NewStyle != NULL )
			{
				// Since we deleted a style, we need to tell widgets to reinitialize their styles.
				ReinitializeAllWidgetStyles();

				// Regenerates the style tree.
				UpdateSkinProperties();

				// get the wxTreeItemId associated with the new style
				wxTreeItemId* ItemId = StyleIDMap.Find(NewStyle);
				check(ItemId);

				// ensure it's visible in the style browser
				StyleTree->SelectItem(*ItemId);
				StyleTree->EnsureVisible(*ItemId);
			}
			else
			{
				Transaction.Cancel();
			}
		}
	}
}	

/** Called when the user selects the "Delete Style" context menu option */
void WxDlgUISkinEditor::OnContext_DeleteStyle( wxCommandEvent& Event )
{
	UUISkin* Skin = GetSelectedSkin();
	UUIStyle* Style = GetSelectedStyle();

	if(Skin && Style)
	{
		const UBOOL bImplementedStyle = IsImplementedStyleSelected();
		wxTreeItemId* StyleItemId = StyleIDMap.Find(Style);

		if(StyleItemId && bImplementedStyle == TRUE)
		{	
			// Make sure that we should delete this style by checking to see if any widgets are currently referencing this style.
			// If so, display a dialog and let the user confirm that they want to delete the style.
			const UUISceneManager *SceneManager = GUnrealEd->GetBrowserManager()->UISceneManager;
			const UBOOL bDeleteStyle = SceneManager->ShouldDeleteStyle(Style, FALSE);

			if(bDeleteStyle)
			{
				UUISkin *ActiveSkin = SceneManager->ActiveSkin;

				FScopedTransaction Transaction( *LocalizeUI(TEXT("UIEditor_MenuItem_DeleteStyle")) );
				if(Skin->DeleteStyle(Style))
				{
					// Since we deleted a style, we need to tell widgets to reinitialize their styles.
					ReinitializeAllWidgetStyles();

					// Store the parent of the item that will be disappearing and update the property view.
					wxTreeItemId ParentItemId = StyleTree->GetItemParent(*StyleItemId);
					UpdateSkinProperties();

					// Make sure we place the selection close to where the user deleted the style from.
					StyleTree->SelectItem(ParentItemId);
					StyleTree->EnsureVisible(ParentItemId);
					StyleTree->Expand(ParentItemId);
				}
				else
				{
					Transaction.Cancel();
				}
			}
		}
	}

}

/** Displays the the edit cursor dialog. */
void WxDlgUISkinEditor::OnCursorActivated(wxListEvent &Event)
{
	FName CursorName(Event.GetItem().GetText());
	
	DisplayEditCursorDialog(CursorName);
}

/** Displays a cursor specific context menu. */
void WxDlgUISkinEditor::OnCursorRightClick( wxListEvent &Event )
{
	WxDlgUISkinEditor_CursorContextMenu Menu(CursorList->GetFirstSelected() != -1);

	FTrackPopupMenu Tracker( this, &Menu );
	Tracker.Show();
}

/** Called when the user selects the "Add Cursor" context menu option */
void WxDlgUISkinEditor::OnContext_AddCursor( wxCommandEvent& Event )
{
	if(SelectedSkin != NULL)
	{

		// Get a name for this new cursor.
		wxTextEntryDialog Dlg(this, *LocalizeUI(TEXT("DlgUISkinEditor_RenameCursor")), *LocalizeUI(TEXT("UIEditor_MenuItem_RenameCursor")));

		if(Dlg.ShowModal() == wxID_OK)
		{
			wxString NewNameStr = Dlg.GetValue();

			if(NewNameStr.Len())
			{
				FName NewName(NewNameStr);

				//Make sure a cursor with this name doesn't already exist.
				if(SelectedSkin->CursorMap.Find(NewName) == NULL)
				{

					FScopedTransaction Transaction( *LocalizeUI(TEXT("UIEditor_MenuItem_AddCursor")) );

					FUIMouseCursor CursorInfo;
					UBOOL bFoundImageStyle = FALSE;

					// Loop through the style map and pull the first ImageStyle we find.
					for(TMap<FName, UUIStyle*>::TIterator It(SelectedSkin->StyleNameMap); It; ++It)
					{
						const UUIStyle* Style = It.Value();
		
						if(Style->StyleDataClass->IsChildOf(UUIStyle_Image::StaticClass()) == TRUE)
						{
							CursorInfo.CursorStyle = It.Key();
							bFoundImageStyle = TRUE;
							break;
						}
					}

					check(bFoundImageStyle);

					// Create a UUITexture for this cursor.
					UUITexture* CursorTexture = ConstructObject<UUITexture>(UUITexture::StaticClass(), SelectedSkin);
					CursorTexture->ImageTexture = LoadObject<UMaterial>(NULL, *GEngine->DefaultMaterialName, NULL, LOAD_None, NULL);
					CursorInfo.Cursor = CursorTexture;

					// Finally add the cursor.
					if ( SelectedSkin->AddCursorResource(NewName, CursorInfo) == TRUE )
					{
						AddCursor(NewName, &CursorInfo, TRUE);
					}
					else
					{
						Transaction.Cancel();
					}
				}
				else
				{
					wxMessageBox( *FString::Printf(LocalizeSecure(LocalizeUI(TEXT("DlgUISkinEditor_ErrorCursorExists")), *NewName.ToString())), *LocalizeUnrealEd(TEXT("Error")), wxICON_ERROR);
				}
			}
		}
	}
}

/** Called when the user selects the "Edit Cursor" context menu option */
void WxDlgUISkinEditor::OnContext_EditCursor( wxCommandEvent& Event )
{
	long SelectedItem = CursorList->GetFirstSelected();

	if(SelectedItem != -1 && SelectedSkin != NULL)
	{
		FName CursorName(CursorList->GetItemText(SelectedItem));
		DisplayEditCursorDialog(CursorName);
	}
}

/** Called when the user selects the "Rename Cursor" context menu option */
void WxDlgUISkinEditor::OnContext_RenameCursor( wxCommandEvent& Event )
{
	long SelectedItem = CursorList->GetFirstSelected();

	if(SelectedItem != -1 && SelectedSkin != NULL)
	{
		// Find the old name of this cursor and make sure its still valid.
		FName OldName(CursorList->GetItemText(SelectedItem));
		FUIMouseCursor* CursorInfo = SelectedSkin->CursorMap.Find(OldName);

		if(CursorInfo != NULL)
		{
			// Get a new name for this cursor.
			wxTextEntryDialog Dlg(this, *LocalizeUI(TEXT("DlgUISkinEditor_RenameCursor")), *LocalizeUI(TEXT("UIEditor_MenuItem_RenameCursor")));

			if(Dlg.ShowModal() == wxID_OK)
			{
				wxString NewNameStr = Dlg.GetValue();

				if(NewNameStr.Len())
				{
					FScopedTransaction Transaction( *LocalizeUI(TEXT("UIEditor_MenuItem_RenameCursor")) );

					FName NewName(NewNameStr);

					//Make sure a cursor with this name doesn't already exist.
					if(SelectedSkin->CursorMap.Find(NewName) == NULL)
					{
						SelectedSkin->CursorMap.Remove(OldName);
						SelectedSkin->CursorMap.Set(NewName, *CursorInfo);

						CursorList->SetItemText(SelectedItem, *NewName.ToString());
					}
					else
					{
						wxMessageBox( *FString::Printf(LocalizeSecure(LocalizeUI(TEXT("DlgUISkinEditor_ErrorCursorExists")), *NewName.ToString())), *LocalizeUnrealEd(TEXT("Error")), wxICON_ERROR);
					}
				}
			}
		}
	}

}

/** Called when the user selects the "Delete Cursor" context menu option */
void WxDlgUISkinEditor::OnContext_DeleteCursor( wxCommandEvent& Event )
{
	long SelectedItem = CursorList->GetFirstSelected();

	if(SelectedItem != -1 && SelectedSkin != NULL)
	{
		FName CursorName(CursorList->GetItemText(SelectedItem));

		FScopedTransaction Transaction( *LocalizeUI(TEXT("UIEditor_MenuItem_DeleteCursor")) );

		SelectedSkin->CursorMap.Remove(CursorName);
		CursorList->DeleteItem(SelectedItem);
	}
}

/** Called when the user selects the "Add New Group" context menu option */
void WxDlgUISkinEditor::OnContext_AddGroup( wxCommandEvent& Event )
{
	if ( SelectedSkin != NULL )
	{
		wxTextEntryDialog dlg(
			this, 
			*LocalizeUI(TEXT("UIEditor_MenuItem_AskNewName")), 
			TEXT("Rename"), TEXT(""), 
			wxOK | wxCANCEL
			);

		if ( dlg.ShowModal() == wxID_OK )
		{
			FScopedTransaction Transaction(*LocalizeUI(TEXT("TransAddStyleGroup")));

			const FString NewGroupName = dlg.GetValue().c_str();
			if ( SelectedSkin->AddStyleGroupName(NewGroupName) )
			{
				AddGroup(NewGroupName, FALSE, TRUE);
				GroupsList->SortItems(WxDlgUISkinEditor_StyleGroupName_ListSort, (LONG)&StyleGroupSortOptions);
			}
			else
			{
				Transaction.Cancel();
			}
		}
	}
}

/** Called when the user selects the "Rename Group" context menu option */
void WxDlgUISkinEditor::OnContext_RenameGroup( wxCommandEvent& Event )
{
	long SelectedItem = GroupsList->GetFirstSelected();
	if(SelectedItem != -1 && SelectedSkin != NULL)
	{
		const FString OldName = GroupsList->GetItemText(SelectedItem).c_str();

		wxTextEntryDialog dialog(this, 
			*LocalizeUI(TEXT("UIEditor_MenuItem_AskNewName")), 
			TEXT("Rename"), GroupsList->GetItemText(SelectedItem), wxOK | wxCANCEL);

		if ( dialog.ShowModal() == wxID_OK )
		{
			FScopedTransaction Transaction(*LocalizeUI(TEXT("TransRenameStyleGroup")));

			const FString NewName = dialog.GetValue().c_str();
			if ( SelectedSkin->RenameStyleGroup(OldName, NewName) )
			{
				// now update the list with the new name
				GroupsList->SetItemText(SelectedItem, *NewName);

				FStyleGroupNameSortData* SortData = (FStyleGroupNameSortData*)GroupsList->GetItemData(SelectedItem);
				if ( SortData )
				{
					delete SortData;
					SortData = new FStyleGroupNameSortData(NewName, FALSE);
					GroupsList->SetItemData(SelectedItem, (LONG)SortData);
				}
			}
			else
			{
				Transaction.Cancel();
			}
		}

	}
}

/** Called when the user selects the "Delete Group" context menu option */
void WxDlgUISkinEditor::OnContext_DeleteGroup( wxCommandEvent& Event )
{
	long SelectedItem = GroupsList->GetFirstSelected();
	if ( SelectedItem != -1 && SelectedSkin != NULL)
	{
		const FString SelectedGroupName = GroupsList->GetItemText(SelectedItem).c_str();
		wxMessageDialog DeserializationWarning(
			this, 
			*FString::Printf(LocalizeSecure(LocalizeUI(TEXT("UIEditor_MenuItem_AcceptDeletion")), *SelectedGroupName)),
			TEXT("Message"),
			wxOK|wxCANCEL|wxICON_EXCLAMATION
			);

		if ( DeserializationWarning.ShowModal() == wxID_OK )
		{
			FScopedTransaction Transaction(*LocalizeUI(TEXT("TransRemoveStyleGroup")));

			if ( SelectedSkin->RemoveStyleGroupName(SelectedGroupName) )
			{
				// remove the item from the list
				CleanupSortData(SelectedItem);
				GroupsList->DeleteItem(SelectedItem);

				// if there are more items in the list, select the next one in the list (or the last item if the last item was deleted)
				const INT GroupCount = GroupsList->GetItemCount();
				if ( GroupCount > 0 )
				{
					GroupsList->Select( Clamp<INT>(SelectedItem, 0, GroupCount - 1) );
				}
			}
			else
			{
				Transaction.Cancel();
			}
		}
	}
}

/** Displays the edit sound cue dialog. */
void WxDlgUISkinEditor::OnSoundCueActivated(wxListEvent &Event)
{
	DisplayEditSoundCueDialog(Event.GetItem().GetId());
}

/** Displays a sound cue specific context menu. */
void WxDlgUISkinEditor::OnSoundCueRightClick( wxListEvent &Event )
{
	WxDlgUISkinEditor_SoundCueContextMenu Menu(SoundCueList->GetFirstSelected() != -1);

	FTrackPopupMenu Tracker( this, &Menu );
	Tracker.Show();
}


/** Called when the user selects the "Add Sound Cue" context menu option */
void WxDlgUISkinEditor::OnContext_AddSoundCue( wxCommandEvent& Event )
{
	if(SelectedSkin != NULL)
	{
		WxDlgEditSoundCue* Dlg = new WxDlgEditSoundCue;
		Dlg->Create(SelectedSkin, this);
		Dlg->Show();
	}
}

/** Called when the user selects the "Edit Sound Cue" context menu option */
void WxDlgUISkinEditor::OnContext_EditSoundCue( wxCommandEvent& Event )
{
	long SelectedItem = SoundCueList->GetFirstSelected();
	if( SelectedItem != -1 )
	{
		DisplayEditSoundCueDialog(SelectedItem);
	}
}

/** Called when the user selects the "Delete Sound Cue" context menu option */
void WxDlgUISkinEditor::OnContext_DeleteSoundCue( wxCommandEvent& Event )
{
	long SelectedItem = SoundCueList->GetFirstSelected();
	if( SelectedItem != -1 && SelectedSkin != NULL )
	{
		FUISoundCue SelectedSoundCue(EC_EventParm);
		if ( SelectedItem != INDEX_NONE )
		{
			SelectedSoundCue.SoundName = SoundCueList->GetItemText(SelectedItem).c_str();
			SelectedSoundCue.SoundToPlay = (USoundCue*)SoundCueList->GetItemData(SelectedItem);
		}

		if ( SelectedSoundCue.SoundName != NAME_None )
		{
			FScopedTransaction Transaction( *LocalizeUI(TEXT("UIEditor_MenuItem_DeleteSoundCue")) );

			if ( SelectedSkin->RemoveUISoundCue(SelectedSoundCue.SoundName) )
			{
				SoundCueList->DeleteItem(SelectedItem);
				
				int ColAutoSizeMode = wxLIST_AUTOSIZE;
				if ( SoundCueList->GetItemCount() == 0 )
				{
					ColAutoSizeMode = wxLIST_AUTOSIZE_USEHEADER;
				}

				SoundCueList->SetColumnWidth(SCF_Name, ColAutoSizeMode);
				SoundCueList->SetColumnWidth(SCF_SoundCue, ColAutoSizeMode);
			}
			else
			{
				Transaction.Cancel();
			}
		}
	}
}


/**
 * Displays the "edit sound cue" dialog to allow the user to modify the selected UISoundCue
 *
 * @param	SoundCueIndex	the index of the UISoundCue that the user wants to edit
 */
void WxDlgUISkinEditor::DisplayEditSoundCueDialog( INT SoundCueIndex/*=INDEX_NONE*/ )
{
	if( SelectedSkin != NULL && SoundCueList != NULL )
	{
		FUISoundCue SelectedSoundCue(EC_EventParm);
		if ( SoundCueIndex != INDEX_NONE )
		{
			SelectedSoundCue.SoundName = SoundCueList->GetItemText(SoundCueIndex).c_str();
			SelectedSoundCue.SoundToPlay = (USoundCue*)SoundCueList->GetItemData(SoundCueIndex);
		}

		WxDlgEditSoundCue* Dlg = new WxDlgEditSoundCue;
		Dlg->Create(SelectedSkin, this, ID_UI_EDITUISOUNDCUE_DLG, SelectedSoundCue.SoundName, SelectedSoundCue.SoundToPlay);
		Dlg->Show();
	}
}

