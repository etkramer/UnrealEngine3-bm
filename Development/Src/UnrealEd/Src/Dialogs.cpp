/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "UnTexAlignTools.h"
#include "ScopedTransaction.h"
#include "Properties.h"
#include "BusyCursor.h"

///////////////////////////////////////////////////////////////////////////////
//
// Local classes.
//
///////////////////////////////////////////////////////////////////////////////

class WxSurfacePropertiesPanel : public wxPanel, public FNotifyHook, public FSerializableObject
{
public:
	WxSurfacePropertiesPanel( wxWindow* InParent );

	/**
	 * Called by WxDlgSurfaceProperties::RefreshPages().
	 */
	void RefreshPage();

	////////////////////////////
	// FNotifyHook interface
	virtual void NotifyPostChange( void* Src, UProperty* PropertyThatChanged );

	////////////////////////////
	// FSerializableObject interface
	/**
	 * Since this class holds onto an object reference, it needs to be serialized
	 * so that the objects aren't GCed out from underneath us.
	 *
	 * @param	Ar			The archive to serialize with.
	 */
	virtual void Serialize(FArchive& Ar)
	{
		Ar << LightingChannelsObject;
	}

private:
	void Pan( INT InU, INT InV );
	void Scale( FLOAT InScaleU, FLOAT InScaleV, UBOOL InRelative );

	void OnU1( wxCommandEvent& In );
	void OnU4( wxCommandEvent& In );
	void OnU16( wxCommandEvent& In );
	void OnU64( wxCommandEvent& In );
	void OnUCustom( wxCommandEvent& In );

	void OnV1( wxCommandEvent& In );
	void OnV4( wxCommandEvent& In );
	void OnV16( wxCommandEvent& In );
	void OnV64( wxCommandEvent& In );
	void OnVCustom( wxCommandEvent& In );

	void OnFlipU( wxCommandEvent& In );
	void OnFlipV( wxCommandEvent& In );
	void OnRot45( wxCommandEvent& In );
	void OnRot90( wxCommandEvent& In );
	void OnRotCustom( wxCommandEvent& In );

	void OnApply( wxCommandEvent& In );
	void OnScaleSimple( wxCommandEvent& In );
	void OnScaleCustom( wxCommandEvent& In );
	void OnLightMapResSelChange( wxCommandEvent& In );
	void OnAcceptsLightsChange( wxCommandEvent& In );
	void OnAcceptsDynamicLightsChange( wxCommandEvent& In );
	void OnForceLightmapChange( wxCommandEvent& In );
	void OnAlignSelChange( wxCommandEvent& In );
	void OnApplyAlign( wxCommandEvent& In );

	/**
	 * Sets passed in poly flag on selected surfaces.
	 *
 	 * @param PolyFlag	PolyFlag to toggle on selected surfaces
	 * @param Value		Value to set the flag to.
	 */
	void SetPolyFlag( DWORD PolyFlag, UBOOL Value );

	/** Sets lighting channels for selected surfaces to the specified value. */
	void SetLightingChannelsForSelectedSurfaces(DWORD NewBitfield);

	wxPanel* Panel;

	wxRadioButton *SimpleScaleButton;
	wxRadioButton *CustomScaleButton;

	wxComboBox *SimpleCB;

	wxStaticText *CustomULabel;
	wxStaticText *CustomVLabel;

	wxTextCtrl *CustomUEdit;
	wxTextCtrl *CustomVEdit;

	wxCheckBox *RelativeCheck;
	wxComboBox *LightMapResCombo;

	/** Checkbox for PF_AcceptsLights */
	wxCheckBox*	AcceptsLightsCheck;
	/** Checkbox for PF_AcceptsDynamicLights */
	wxCheckBox*	AcceptsDynamicLightsCheck;
	/** Checkbox for PF_ForceLigthMap */
	wxCheckBox* ForceLightMapCheck;

	UBOOL bUseSimpleScaling;
	/** Property window containg texture alignment options. */
	WxPropertyWindow* PropertyWindow;
	wxListBox *AlignList;

	/** Property window containing surface lighting channels. */
	WxPropertyWindow* ChannelsPropertyWindow;
	/** The lighting channels object embedded in this window. */
	ULightingChannelsObject* LightingChannelsObject;
};


/**
 * WxDlgBindHotkeys
 */
namespace
{
	static const wxColour DlgBindHotkeys_SelectedBackgroundColor(255,218,171);
	static const wxColour DlgBindHotkeys_LightColor(227, 227, 239);
	static const wxColour DlgBindHotkeys_DarkColor(200, 200, 212);
}


class WxKeyBinder : public wxTextCtrl
{
public:
	WxKeyBinder(wxWindow* Parent, class WxDlgBindHotkeys* InParentDlg) : 
	  wxTextCtrl(Parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY),
	  ParentDlg(InParentDlg)
	{

	}

private:

	/** Handler for when a key is pressed or released. */
	void OnKeyEvent(wxKeyEvent& Event);

	/** Handler for when focus is lost */
	void OnKillFocus(wxFocusEvent& Event);

	/** Pointer to the parent of this class. */
	class WxDlgBindHotkeys* ParentDlg;

	DECLARE_EVENT_TABLE()
};


BEGIN_EVENT_TABLE(WxKeyBinder, wxTextCtrl)
	EVT_KEY_DOWN(OnKeyEvent)
	EVT_KEY_UP(OnKeyEvent)
	EVT_KILL_FOCUS(OnKillFocus)
END_EVENT_TABLE()


/** Handler for when a key is pressed or released. */
void WxKeyBinder::OnKeyEvent(wxKeyEvent& Event)
{
	if(ParentDlg->IsBinding())
	{
		if(Event.GetKeyCode() != WXK_SHIFT && Event.GetKeyCode() != WXK_CONTROL && Event.GetKeyCode() != WXK_ALT && Event.GetEventType() == wxEVT_KEY_DOWN)
		{
			FName Key = GApp->GetKeyName(Event);

			if(Key==KEY_Escape)
			{
				ParentDlg->StopBinding();
			}
			else
			{
				ParentDlg->FinishBinding(Event);
			}
		}
		else
		{
			FString BindString = ParentDlg->GenerateBindingText(Event.AltDown(), Event.ControlDown(), Event.ShiftDown(), NAME_None);
			SetValue(*BindString);
		}
	}
}

/** Handler for when focus is lost */
void WxKeyBinder::OnKillFocus(wxFocusEvent& Event)
{
	ParentDlg->StopBinding();
}

BEGIN_EVENT_TABLE(WxDlgBindHotkeys, wxFrame)
	EVT_CLOSE(OnClose)
	EVT_BUTTON(ID_DLGBINDHOTKEYS_LOAD_CONFIG, OnLoadConfig)
	EVT_BUTTON(ID_DLGBINDHOTKEYS_SAVE_CONFIG, OnSaveConfig)
	EVT_BUTTON(ID_DLGBINDHOTKEYS_RESET_TO_DEFAULTS, OnResetToDefaults)
	EVT_BUTTON(wxID_OK, OnOK)
	EVT_TREE_SEL_CHANGED(ID_DLGBINDHOTKEYS_TREE, OnCategorySelected)
	EVT_COMMAND_RANGE(ID_DLGBINDHOTKEYS_BIND_KEY_START, ID_DLGBINDHOTKEYS_BIND_KEY_END, wxEVT_COMMAND_BUTTON_CLICKED, OnBindKey)
	EVT_KEY_DOWN(OnKeyDown)
	EVT_CHAR(OnKeyDown)
END_EVENT_TABLE()

WxDlgBindHotkeys::WxDlgBindHotkeys(wxWindow* InParent) : 
wxFrame(InParent, wxID_ANY, *LocalizeUnrealEd("DlgBindHotkeys_Title"),wxDefaultPosition, wxSize(640,480), wxDEFAULT_FRAME_STYLE | wxWANTS_CHARS),
ItemSize(20),
NumVisibleItems(0),
CommandPanel(NULL),
CurrentlyBindingIdx(-1)
{
	wxSizer* PanelSizer = new wxBoxSizer(wxHORIZONTAL);
	{		
		wxPanel* MainPanel = new wxPanel(this);
		{
			wxSizer* MainSizer = new wxBoxSizer(wxVERTICAL);
			{
				MainSplitter = new wxSplitterWindow(MainPanel);
				{
					CommandPanel = new wxScrolledWindow(MainSplitter);
					CommandCategories = new WxTreeCtrl(MainSplitter, ID_DLGBINDHOTKEYS_TREE, NULL, wxTR_HIDE_ROOT | wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT);
					
					MainSplitter->SplitVertically(CommandCategories, CommandPanel);
					MainSplitter->SetMinimumPaneSize(128);
				}
				MainSizer->Add(MainSplitter, 1, wxEXPAND | wxALL, 2);

				wxBoxSizer* HSizer = new wxBoxSizer(wxHORIZONTAL);
				{
					wxButton* LoadButton = new wxButton(MainPanel, ID_DLGBINDHOTKEYS_LOAD_CONFIG, *LocalizeUnrealEd("LoadConfig"));
					wxButton* SaveButton = new wxButton(MainPanel, ID_DLGBINDHOTKEYS_SAVE_CONFIG, *LocalizeUnrealEd("SaveConfig"));
					wxButton* ResetToDefaults = new wxButton(MainPanel, ID_DLGBINDHOTKEYS_RESET_TO_DEFAULTS, *LocalizeUnrealEd("ResetToDefaults"));
					BindLabel = new wxStaticText(MainPanel, wxID_ANY, TEXT(""));
					BindLabel->GetFont().SetWeight(wxFONTWEIGHT_BOLD);

					HSizer->Add(LoadButton, 0, wxEXPAND | wxALL, 2);
					HSizer->Add(SaveButton, 0, wxEXPAND | wxALL, 2);
					HSizer->Add(ResetToDefaults, 0, wxEXPAND | wxALL, 2);
					HSizer->Add(BindLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
					
					wxBoxSizer* VSizer = new wxBoxSizer(wxVERTICAL);
					{
						wxButton* OKButton = new wxButton(MainPanel, wxID_OK, *LocalizeUnrealEd("OK"));
						VSizer->Add(OKButton, 0, wxALIGN_RIGHT | wxALL, 2);
					}
					HSizer->Add(VSizer,1, wxEXPAND);
				}
				MainSizer->Add(HSizer, 0, wxEXPAND | wxALL, 2);
			}
			MainPanel->SetSizer(MainSizer);
		}
		PanelSizer->Add(MainPanel, 1, wxEXPAND);
	}
	SetSizer(PanelSizer);
	SetAutoLayout(TRUE);

	// Build category tree
	BuildCategories();

	// Load window options.
	LoadSettings();

	// Initially select the first item in the category list
	CommandCategories->SelectItem( CommandCategories->GetFirstVisibleItem() );
}

/** Saves settings for this dialog to the INI. */
void WxDlgBindHotkeys::SaveSettings()
{
	FWindowUtil::SavePosSize(TEXT("DlgBindHotkeys"), this);

	GConfig->SetInt(TEXT("DlgBindHotkeys"), TEXT("SplitterPos"), MainSplitter->GetSashPosition(), GEditorUserSettingsIni);

	// Save the key bindings object
	UUnrealEdOptions* Options = GUnrealEd->GetUnrealEdOptions();
	
	if(Options)
	{
		UUnrealEdKeyBindings* KeyBindings = Options->EditorKeyBindings;
		
		if(KeyBindings)
		{
			FString UserKeybindings = appGameConfigDir() + GGameName;
			UserKeybindings += TEXT("EditorUserKeybindings.ini");

			KeyBindings->SaveConfig(CPF_Config, *UserKeybindings);
		}
	}
}

/** Loads settings for this dialog from the INI. */
void WxDlgBindHotkeys::LoadSettings()
{
	FWindowUtil::LoadPosSize(TEXT("DlgBindHotkeys"), this, -1, -1, 640, 480);

	INT SashPos = 256;

	GConfig->GetInt(TEXT("DlgBindHotkeys"), TEXT("SplitterPos"), SashPos, GEditorUserSettingsIni);
	MainSplitter->SetSashPosition(SashPos);

	// Load the user's settings
	UUnrealEdOptions* Options = GUnrealEd->GetUnrealEdOptions();

	if(Options)
	{
		UUnrealEdKeyBindings* KeyBindings = Options->EditorKeyBindings;

		if(KeyBindings)
		{
			// Load user bindings first.
			FString UserKeyBindings = appGameConfigDir() + GGameName;
			UserKeyBindings += TEXT("EditorUserKeyBindings.ini");
			KeyBindings->ReloadConfig(NULL, *UserKeyBindings);

			TArray<FEditorKeyBinding> UserKeys = Options->EditorKeyBindings->KeyBindings;

			// Reload Defaults
			KeyBindings->ReloadConfig();

			// Generate a binding map for quick lookup.
			TMap<FName, INT> BindingMap;

			BindingMap.Empty();

			for(INT BindingIdx=0; BindingIdx<KeyBindings->KeyBindings.Num(); BindingIdx++)
			{
				FEditorKeyBinding &KeyBinding = KeyBindings->KeyBindings(BindingIdx);

				BindingMap.Set(KeyBinding.CommandName, BindingIdx);
			}

			// Merge in user keys
			for(INT KeyIdx=0; KeyIdx<UserKeys.Num();KeyIdx++)
			{
				FEditorKeyBinding &UserKey = UserKeys(KeyIdx);

				INT* BindingIdx = BindingMap.Find(UserKey.CommandName);

				if(BindingIdx && KeyBindings->KeyBindings.IsValidIndex(*BindingIdx))
				{
					FEditorKeyBinding &DefaultBinding = KeyBindings->KeyBindings(*BindingIdx);
					DefaultBinding = UserKey;
				}
				else
				{
					KeyBindings->KeyBindings.AddItem(UserKey);
				}
			}
		}
	}
}

/** Builds the category tree. */
void WxDlgBindHotkeys::BuildCategories()
{
	ParentMap.Empty();
	CommandCategories->DeleteAllItems();
	CommandCategories->AddRoot(TEXT("Root"));
	
	// Add all of the categories to the tree.
	UUnrealEdOptions* Options = GUnrealEd->GetUnrealEdOptions();
	Options->GenerateCommandMap();

	if(Options)
	{
		TArray<FEditorCommandCategory> &Categories = Options->EditorCategories;

		for(INT CategoryIdx=0; CategoryIdx<Categories.Num(); CategoryIdx++)
		{
			FEditorCommandCategory &Category = Categories(CategoryIdx);
			wxTreeItemId ParentId = CommandCategories->GetRootItem();

			if(Category.Parent != NAME_None)
			{
				wxTreeItemId* ParentIdPtr = ParentMap.Find(Category.Parent);

				if(ParentIdPtr)
				{
					ParentId = *ParentIdPtr;
				}
			}

			FString CategoryName = Localize(TEXT("CommandCategoryNames"),*Category.Name.ToString(),TEXT("UnrealEd"));
			wxTreeItemId TreeItem = CommandCategories->AppendItem(ParentId, *CategoryName);
			ParentMap.Set(Category.Name, TreeItem);

			// Create the command map entry for this category.
			TArray<FEditorCommand> Commands;
			for(INT CmdIdx=0; CmdIdx<Options->EditorCommands.Num(); CmdIdx++)
			{
				FEditorCommand &Command = Options->EditorCommands(CmdIdx);

				if(Command.Parent == Category.Name)
				{
					Commands.AddItem(Command);
				}
			}

			if(Commands.Num())
			{
				CommandMap.Set(Category.Name, Commands);
			}
		}
	}
}

/** Builds the command list using the currently selected category. */
void WxDlgBindHotkeys::BuildCommands()
{
	UUnrealEdOptions* Options = GUnrealEd->GetUnrealEdOptions();

	CommandPanel->Freeze();

	// Remove all children from the panel.
	CommandPanel->DestroyChildren();
	
	VisibleBindingControls.Empty();

	FName ParentName;
	const FName* ParentNamePtr = ParentMap.FindKey(CommandCategories->GetSelection());

	if(ParentNamePtr != NULL)
	{
		ParentName = *ParentNamePtr;
		if(ParentName != NAME_None)
		{
			TArray<FEditorCommand> *CommandsPtr = CommandMap.Find(ParentName);

			if(CommandsPtr)
			{
				wxBoxSizer* PanelSizer = new wxBoxSizer(wxVERTICAL);
				{
					// Loop through all commands and add a panel for each one.
					TArray<FEditorCommand> &Commands = *CommandsPtr;

					for(INT CmdIdx=0; CmdIdx<Commands.Num(); CmdIdx++)
					{
						FEditorCommand &Command = Commands(CmdIdx);

						// Add a button, label, and binding box for each command.
						wxPanel* ItemPanel = new wxPanel(CommandPanel);
						{
							FCommandPanel PanelWidgets;
							PanelWidgets.BindingPanel = ItemPanel;
							PanelWidgets.CommandName = Command.CommandName;

							ItemPanel->SetBackgroundColour((CmdIdx%2==0) ? DlgBindHotkeys_LightColor : DlgBindHotkeys_DarkColor);

							FString CommandName = Localize(TEXT("CommandNames"),*Command.CommandName.ToString(),TEXT("UnrealEd"));
							wxBoxSizer* ItemSizer = new wxBoxSizer(wxHORIZONTAL);
							{
								wxBoxSizer* VSizer = new wxBoxSizer(wxVERTICAL);
								{
									wxStaticText* CommandLabel = new wxStaticText(ItemPanel, wxID_ANY, *CommandName);
									CommandLabel->GetFont().SetWeight(wxFONTWEIGHT_BOLD);
									VSizer->Add(CommandLabel, 0, wxEXPAND | wxTOP, 2);

									wxTextCtrl* CurrentBinding = new WxKeyBinder(ItemPanel, this);
									VSizer->Add(CurrentBinding, 0, wxEXPAND | wxBOTTOM, 6);

									// Store reference to the binding widget.
									PanelWidgets.BindingWidget = CurrentBinding;
								}
								ItemSizer->Add(VSizer, 1, wxEXPAND | wxALL, 4);

								wxButton* SetBindingButton = new wxButton(ItemPanel, ID_DLGBINDHOTKEYS_BIND_KEY_START+CmdIdx, *LocalizeUnrealEd("Bind"));
								ItemSizer->Add(SetBindingButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 8);

								PanelWidgets.BindingButton = SetBindingButton;
							}
							ItemPanel->SetSizer(ItemSizer);			

							// Store pointers to each of the controls we created for future use.
							VisibleBindingControls.AddItem(PanelWidgets);
						}
						PanelSizer->Add(ItemPanel, 0, wxEXPAND);
						PanelSizer->RecalcSizes();

						ItemSize = ItemPanel->GetSize().GetHeight();
					}

					VisibleCommands = Commands;
					NumVisibleItems = Commands.Num();
				}
				CommandPanel->SetSizer(PanelSizer);
			}
		}
	}

	// Refresh binding text
	RefreshBindings();

	// Start drawing the command panel again
	CommandPanel->Thaw();

	// Updates the scrollbar and refreshes this window.
	CommandPanel->Layout();
	CommandPanel->Refresh();
	UpdateScrollbar();
	CommandPanel->Refresh();
}

/** Refreshes the binding text for the currently visible binding widgets. */
void WxDlgBindHotkeys::RefreshBindings()
{
	UUnrealEdOptions* Options = GUnrealEd->GetUnrealEdOptions();

	// Loop through all visible items and update the current binding text.
	for(INT BindingIdx=0; BindingIdx<VisibleBindingControls.Num(); BindingIdx++)
	{
		FCommandPanel &BindingWidgets = VisibleBindingControls(BindingIdx);
		wxTextCtrl* CurrentBinding = BindingWidgets.BindingWidget;

		// Set the key binding text.
		FEditorKeyBinding* KeyBinding = Options->GetKeyBinding(BindingWidgets.CommandName);

		FString BindingText;

		if(KeyBinding)
		{
			BindingText = GenerateBindingText(KeyBinding->bAltDown, KeyBinding->bCtrlDown, KeyBinding->bShiftDown, KeyBinding->Key);								
		}
		else
		{
			BindingText = *LocalizeUnrealEd("NotBound");
		}

		CurrentBinding->SetValue(*BindingText);
	}
}

/** Updates the scrollbar for the command view. */
void WxDlgBindHotkeys::UpdateScrollbar()
{
	if(CommandPanel)
	{
		CommandPanel->EnableScrolling(FALSE, TRUE);
		CommandPanel->SetScrollbars(0, ItemSize, 0, NumVisibleItems);
	}
}

/** Starts the binding process for the specified command index. */
void WxDlgBindHotkeys::StartBinding(INT CommandIdx)
{
	CurrentlyBindingIdx = CommandIdx;

	// Set focus to the binding control
	FCommandPanel &BindingWidgets = VisibleBindingControls(CommandIdx);

	// Change the background of the current command
	BindingWidgets.BindingPanel->SetBackgroundColour(DlgBindHotkeys_SelectedBackgroundColor);
	BindingWidgets.BindingPanel->Refresh();

	// Set focus to the binding widget
	wxTextCtrl* BindingControl = BindingWidgets.BindingWidget;
	BindingControl->SetFocus();

	// Disable all binding buttons.
	for(INT ButtonIdx=0; ButtonIdx<VisibleBindingControls.Num(); ButtonIdx++)
	{
		VisibleBindingControls(ButtonIdx).BindingButton->Disable();
	}

	// Show binding text
	BindLabel->SetLabel(*LocalizeUnrealEd("PressKeysToBind"));
}

/** Finishes the binding for the current command using the provided event. */
void WxDlgBindHotkeys::FinishBinding(wxKeyEvent &Event)
{
	// Finish binding the key.
	UUnrealEdOptions* Options = GUnrealEd->GetUnrealEdOptions();

	FName KeyName = GApp->GetKeyName(Event);

	if(KeyName != NAME_None && Options->EditorCommands.IsValidIndex(CurrentlyBindingIdx))
	{
		FName Command = Options->EditorCommands(CurrentlyBindingIdx).CommandName;
		
		Options->BindKey(KeyName, Event.AltDown(), Event.ControlDown(), Event.ShiftDown(), Command);
	}

	StopBinding();
}

/** Stops the binding process for the current command. */
void WxDlgBindHotkeys::StopBinding()
{
	if(CurrentlyBindingIdx != -1)
	{		
		// Reset the background color
		FCommandPanel &BindingWidgets = VisibleBindingControls(CurrentlyBindingIdx);
		BindingWidgets.BindingPanel->SetBackgroundColour((CurrentlyBindingIdx%2==0) ? DlgBindHotkeys_LightColor : DlgBindHotkeys_DarkColor);
		BindingWidgets.BindingPanel->Refresh();

		// Enable all binding buttons.
		for(INT ButtonIdx=0; ButtonIdx<VisibleBindingControls.Num(); ButtonIdx++)
		{
			VisibleBindingControls(ButtonIdx).BindingButton->Enable();
		}

		// Hide binding text
		BindLabel->SetLabel(TEXT(""));

		// Refresh binding text
		RefreshBindings();

		CurrentlyBindingIdx = -1;
	}
}

/**
 * @return Generates a descriptive binding string based on the key combinations provided.
 */
FString WxDlgBindHotkeys::GenerateBindingText(UBOOL bAltDown, UBOOL bCtrlDown, UBOOL bShiftDown, FName Key)
{
	// Build a string describing this key binding.
	FString BindString;

	if(bCtrlDown)
	{
		BindString += TEXT("Ctrl + ");
	}

	if(bAltDown)
	{
		BindString += TEXT("Alt + ");
	}

	if(bShiftDown)
	{
		BindString += TEXT("Shift + ");
	}

	if(Key != NAME_None)
	{
		BindString += Key.ToString();
	}

	return BindString;
}

/** Window closed event handler. */
void WxDlgBindHotkeys::OnClose(wxCloseEvent& Event)
{
	SaveSettings();

	// Hide the dialog
	Hide();

	// Veto the close
	Event.Veto();
}

/** Category selected handler. */
void WxDlgBindHotkeys::OnCategorySelected(wxTreeEvent& Event)
{
	BuildCommands();
}

/** Handler to let the user load a config from a file. */
void WxDlgBindHotkeys::OnLoadConfig(wxCommandEvent& Event)
{
	// Get the filename
	const wxChar* FileTypes = TEXT("INI Files (*.ini)|*.ini|All Files (*.*)|*.*");

	WxFileDialog Dlg( this, 
		*LocalizeUnrealEd("LoadKeyConfig"), 
		*appGameConfigDir(),
		TEXT(""),
		FileTypes,
		wxOPEN | wxFILE_MUST_EXIST,
		wxDefaultPosition);

	if(Dlg.ShowModal()==wxID_OK)
	{
		wxString Filename = Dlg.GetPath();
		UUnrealEdOptions* Options = GUnrealEd->GetUnrealEdOptions();

		if(Options)
		{
			UUnrealEdKeyBindings* KeyBindings = Options->EditorKeyBindings;

			if(KeyBindings)
			{
				KeyBindings->ReloadConfig(NULL, Filename);

				// Refresh binding text
				RefreshBindings();
			}
		}
	}
}

/** Handler to let the user save the current config to a file. */
void WxDlgBindHotkeys::OnSaveConfig(wxCommandEvent& Event)
{
	// Get the filename
	const wxChar* FileTypes = TEXT("INI Files (*.ini)|*.ini|All Files (*.*)|*.*");

	WxFileDialog Dlg( this, 
		*LocalizeUnrealEd("SaveKeyConfig"), 
		*appGameConfigDir(),
		TEXT(""),
		FileTypes,
		wxSAVE,
		wxDefaultPosition);

	if(Dlg.ShowModal()==wxID_OK)
	{
		wxString Filename = Dlg.GetPath();
		UUnrealEdOptions* Options = GUnrealEd->GetUnrealEdOptions();

		if(Options)
		{
			UUnrealEdKeyBindings* KeyBindings = Options->EditorKeyBindings;

			if(KeyBindings)
			{
				KeyBindings->SaveConfig(CPF_Config, Filename);
			}
		}
	}
}

/** Handler to reset bindings to default. */
void WxDlgBindHotkeys::OnResetToDefaults(wxCommandEvent& Event)
{
	if(wxMessageBox(*LocalizeUnrealEd("AreYouSureYouWantDefaults"), TEXT(""), wxYES_NO | wxCENTRE, this))
	{
		// Load the user's settings
		UUnrealEdOptions* Options = GUnrealEd->GetUnrealEdOptions();

		if(Options)
		{
			UUnrealEdKeyBindings* KeyBindings = Options->EditorKeyBindings;

			if(KeyBindings)
			{
				KeyBindings->ReloadConfig();

				// Refresh binding text
				RefreshBindings();
			}
		}
	}
}

/** Bind key button pressed handler. */
void WxDlgBindHotkeys::OnBindKey(wxCommandEvent& Event)
{
	INT CommandIdx = Event.GetId() - ID_DLGBINDHOTKEYS_BIND_KEY_START;

	if(CommandIdx >=0 && CommandIdx < VisibleCommands.Num())
	{
		StartBinding(CommandIdx);
	}
	else
	{
		CurrentlyBindingIdx = -1;
	}
}

/** OK Button pressed handler. */
void WxDlgBindHotkeys::OnOK(wxCommandEvent &Event)
{
	SaveSettings();

	// Hide the dialog
	Hide();
}

/** Handler for key binding events. */
void WxDlgBindHotkeys::OnKeyDown(wxKeyEvent& Event)
{
	Event.Skip();
}

WxDlgBindHotkeys::~WxDlgBindHotkeys()
{
	
}

/*-----------------------------------------------------------------------------
	WxDlgPackageGroupName.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE(WxDlgPackageGroupName, wxDialog)
	EVT_BUTTON( wxID_OK, WxDlgPackageGroupName::OnOK )
END_EVENT_TABLE()

WxDlgPackageGroupName::WxDlgPackageGroupName()
{
	wxDialog::Create(NULL, wxID_ANY, TEXT("PackageGroupName"), wxDefaultPosition, wxDefaultSize );

	wxBoxSizer* HorizontalSizer = new wxBoxSizer(wxHORIZONTAL);
	{
		wxBoxSizer* InfoStaticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Info"));
		{
			
			PGNCtrl = new WxPkgGrpNameCtrl( this, -1, NULL );
			PGNCtrl->SetSizer(PGNCtrl->FlexGridSizer);
			InfoStaticBoxSizer->Add(PGNCtrl, 1, wxEXPAND);
		}
		HorizontalSizer->Add(InfoStaticBoxSizer, 1, wxALIGN_TOP|wxALL|wxEXPAND, 5);
		
		wxBoxSizer* ButtonSizer = new wxBoxSizer(wxVERTICAL);
		{
			wxButton* ButtonOK = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
			ButtonOK->SetDefault();
			ButtonSizer->Add(ButtonOK, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

			wxButton* ButtonCancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
			ButtonSizer->Add(ButtonCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
		}
		HorizontalSizer->Add(ButtonSizer, 0, wxALIGN_TOP|wxALL, 5);
		
	}
	SetSizer(HorizontalSizer);
	
	FWindowUtil::LoadPosSize( TEXT("DlgPackageGroupName"), this );
	GetSizer()->Fit(this);

	FLocalizeWindow( this );
}

WxDlgPackageGroupName::~WxDlgPackageGroupName()
{
	FWindowUtil::SavePosSize( TEXT("DlgPackageGroupName"), this );
}

int WxDlgPackageGroupName::ShowModal(const FString& InPackage, const FString& InGroup, const FString& InName )
{
	Package = InPackage;
	Group = InGroup;
	Name = InName;

	PGNCtrl->PkgCombo->SetValue( *InPackage );
	PGNCtrl->GrpEdit->SetValue( *InGroup );
	PGNCtrl->NameEdit->SetValue( *InName );

	return wxDialog::ShowModal();
}

bool WxDlgPackageGroupName::Validate()
{
	bool bResult = wxDialog::Validate();

	if ( bResult )
	{
		// validate that the object name is valid
		//@todo ronp - move this functionality to a WxTextValidator
		FString Reason;
		if( !FIsValidObjectName( *Name, Reason )
		||	!FIsValidGroupName( *Package, Reason )
		||	!FIsValidGroupName( *Group, Reason, TRUE ) )
		{
			appMsgf( AMT_OK, *Reason );
			bResult = false;
		}
	}

	return bResult;
}

void WxDlgPackageGroupName::OnOK( wxCommandEvent& In )
{
	Package = PGNCtrl->PkgCombo->GetValue();
	Group = PGNCtrl->GrpEdit->GetValue();
	Name = PGNCtrl->NameEdit->GetValue();

	wxDialog::AcceptAndClose();
}

/** Util for  */
UBOOL WxDlgPackageGroupName::ProcessNewAssetDlg(UPackage** NewObjPackage, FString* NewObjName, UBOOL bAllowCreateOverExistingOfSameType, UClass* NewObjClass)
{
	check(NewObjPackage);
	check(NewObjName);

	FString Pkg;
	FString PName = GetPackage(), GName = GetGroup(), OName = GetObjectName();

	// Was a group specified?
	if( GName.Len() > 0 )
	{
		Pkg = FString::Printf(TEXT("%s.%s"),*PName,*GName);
	}
	else
	{
		Pkg = FString::Printf(TEXT("%s"),*PName);
	}
	UObject* ExistingPackage = UObject::FindPackage(NULL, *Pkg);
	FString Reason;

	// Verify the package an object name.
	if(!PName.Len() || !OName.Len())
	{
		appMsgf(AMT_OK,*LocalizeUnrealEd("Error_InvalidInput"));
		return FALSE;
	}
	// Verify the object name.
	else if(!FIsValidObjectName( *OName, Reason )
		||	!FIsValidGroupName(*GName, Reason, TRUE)
		||	!FIsValidGroupName(*PName, Reason) )
	{
		appMsgf( AMT_OK, *Reason );
		return FALSE;
	}

	*NewObjPackage = UObject::CreatePackage(NULL,*Pkg);

	// See if the name is already in use.
	UObject* FindObj = UObject::StaticFindObject( UObject::StaticClass(), *NewObjPackage, *GetObjectName() );
	if(FindObj && (!bAllowCreateOverExistingOfSameType || FindObj->GetClass() != NewObjClass))
	{
		appMsgf(AMT_OK, TEXT("Invalid object name.  An object already exists with the class name '%s'"), *FindObj->GetName());
		return FALSE;
	}

	// Verify the object name is unique withing the package.
	*NewObjName = OName;
	return TRUE;
}

/*-----------------------------------------------------------------------------
	WxDlgNewArchetype.
-----------------------------------------------------------------------------*/

bool WxDlgNewArchetype::Validate()
{
	bool bResult = wxDialog::Validate();
	if ( bResult )
	{
		FString	QualifiedName;
		if( Group.Len() )
		{
			QualifiedName = Package + TEXT(".") + Group + TEXT(".") + Name;
		}
		else
		{
			QualifiedName = Package + TEXT(".") + Name;
		}

		// validate that the object name is valid
		//@todo ronp - move this functionality to a WxTextValidator
		FString Reason;
		if( !FIsValidObjectName( *Name, Reason )
		||	!FIsValidGroupName( *Package, Reason )
		||	!FIsValidGroupName( *Group, Reason, TRUE )
		||	!FIsUniqueObjectName( *QualifiedName, ANY_PACKAGE, Reason ) )
		{
			appMsgf( AMT_OK, *Reason );
			bResult = false;
		}
	}

	return bResult;
}

/*-----------------------------------------------------------------------------
	WxDlgAddSpecial.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE(WxDlgAddSpecial, wxDialog)
	EVT_BUTTON( wxID_OK, WxDlgAddSpecial::OnOK )
END_EVENT_TABLE()

WxDlgAddSpecial::WxDlgAddSpecial()
{
	const bool bSuccess = wxXmlResource::Get()->LoadDialog( this, GApp->EditorFrame, TEXT("ID_DLG_ADDSPECIAL") );
	check( bSuccess );

	PortalCheck = (wxCheckBox*)FindWindow( XRCID( "IDCK_PORTAL" ) );
	check( PortalCheck != NULL );
	InvisibleCheck = (wxCheckBox*)FindWindow( XRCID( "IDCK_INVISIBLE" ) );
	check( InvisibleCheck != NULL );
	TwoSidedCheck = (wxCheckBox*)FindWindow( XRCID( "IDCK_TWOSIDED" ) );
	check( TwoSidedCheck != NULL );
	SolidRadio = (wxRadioButton*)FindWindow( XRCID( "IDRB_SOLID" ) );
	check( SolidRadio != NULL );
	SemiSolidRadio = (wxRadioButton*)FindWindow( XRCID( "IDRB_SEMISOLID" ) );
	check( SemiSolidRadio != NULL );
	NonSolidRadio = (wxRadioButton*)FindWindow( XRCID( "IDRB_NONSOLID" ) );
	check( NonSolidRadio != NULL );

	FWindowUtil::LoadPosSize( TEXT("DlgAddSpecial"), this );
	FLocalizeWindow( this );
}

WxDlgAddSpecial::~WxDlgAddSpecial()
{
	FWindowUtil::SavePosSize( TEXT("DlgAddSpecial"), this );
}

void WxDlgAddSpecial::OnOK( wxCommandEvent& In )
{
	INT Flags = 0;

	if( PortalCheck->GetValue() )		Flags |= PF_Portal;
	if( InvisibleCheck->GetValue() )	Flags |= PF_Invisible;
	if( TwoSidedCheck->GetValue() )		Flags |= PF_TwoSided;
	if( SemiSolidRadio->GetValue() )	Flags |= PF_Semisolid;
	if( NonSolidRadio->GetValue() )		Flags |= PF_NotSolid;

	GUnrealEd->Exec( *FString::Printf(TEXT("BRUSH ADD FLAGS=%d"), Flags));

	wxDialog::AcceptAndClose();
}

/*-----------------------------------------------------------------------------
	WxDlgGenericStringEntry.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE(WxDlgGenericStringEntry, wxDialog)
	EVT_BUTTON( wxID_OK, WxDlgGenericStringEntry::OnOK )
END_EVENT_TABLE()

WxDlgGenericStringEntry::WxDlgGenericStringEntry()
{
	const bool bSuccess = wxXmlResource::Get()->LoadDialog( this, GApp->EditorFrame, TEXT("ID_GENERICSTRINGENTRY") );
	check( bSuccess );

	StringEntry = (wxTextCtrl*)FindWindow( XRCID( "IDEC_STRINGENTRY" ) );
	check( StringEntry != NULL );
	StringCaption = (wxStaticText*)FindWindow( XRCID( "IDEC_STRINGCAPTION" ) );
	check( StringCaption != NULL );

	ADDEVENTHANDLER( XRCID("IDEC_STRINGENTRY"), wxEVT_COMMAND_TEXT_ENTER, &WxDlgGenericStringEntry::OnOK );

	EnteredString = FString( TEXT("") );

	FWindowUtil::LoadPosSize( TEXT("DlgGenericStringEntry"), this );
}

WxDlgGenericStringEntry::~WxDlgGenericStringEntry()
{
	FWindowUtil::SavePosSize( TEXT("DlgGenericStringEntry"), this );
}

int WxDlgGenericStringEntry::ShowModal(const TCHAR* DialogTitle, const TCHAR* Caption, const TCHAR* DefaultString)
{
	SetTitle( DialogTitle );
	StringCaption->SetLabel( Caption );

	FLocalizeWindow( this );

	StringEntry->SetValue( DefaultString );

	return wxDialog::ShowModal();
}

void WxDlgGenericStringEntry::OnOK( wxCommandEvent& In )
{
	EnteredString = StringEntry->GetValue();

	wxDialog::AcceptAndClose();
}

/*-----------------------------------------------------------------------------
	WxDlgGenericStringWrappedEntry.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE(WxDlgGenericStringWrappedEntry, wxDialog)
EVT_BUTTON( wxID_OK, WxDlgGenericStringWrappedEntry::OnOK )
END_EVENT_TABLE()

WxDlgGenericStringWrappedEntry::WxDlgGenericStringWrappedEntry()
{
	const bool bSuccess = wxXmlResource::Get()->LoadDialog( this, GApp->EditorFrame, TEXT("ID_GENERICSTRINGWRAPPEDENTRY") );
	check( bSuccess );

	StringEntry = (wxTextCtrl*)FindWindow( XRCID( "IDEC_STRINGENTRY" ) );
	check( StringEntry != NULL );
	StringCaption = (wxStaticText*)FindWindow( XRCID( "IDEC_STRINGCAPTION" ) );
	check( StringCaption != NULL );

	ADDEVENTHANDLER( XRCID("IDEC_STRINGENTRY"), wxEVT_COMMAND_TEXT_ENTER, &WxDlgGenericStringWrappedEntry::OnOK );

	EnteredString = FString( TEXT("") );

	FWindowUtil::LoadPosSize( TEXT("DlgGenericStringWrappedEntry"), this );
}

WxDlgGenericStringWrappedEntry::~WxDlgGenericStringWrappedEntry()
{
	FWindowUtil::SavePosSize( TEXT("DlgGenericStringWrappedEntry"), this );
}

int WxDlgGenericStringWrappedEntry::ShowModal(const TCHAR* DialogTitle, const TCHAR* Caption, const TCHAR* DefaultString)
{
	SetTitle( DialogTitle );
	StringCaption->SetLabel( Caption );

	FLocalizeWindow( this );

	StringEntry->SetValue( DefaultString );

	return wxDialog::ShowModal();
}

void WxDlgGenericStringWrappedEntry::OnOK( wxCommandEvent& In )
{
	EnteredString = StringEntry->GetValue();

	wxDialog::AcceptAndClose();
}

/*-----------------------------------------------------------------------------
WxDlgMorphLODImport.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE(WxDlgMorphLODImport, wxDialog)
END_EVENT_TABLE()

/**
* Find the number of matching characters in-order between two strings
* @param StrA - string to compare
* @param StrB - string to compare
* @return number of matching characters
*/
static INT NumMatchingChars( const FString& StrA, const FString& StrB )
{
	INT MaxLength = Min(StrA.Len(),StrB.Len());
	INT NumMatching=0;
	if( MaxLength > 0 )
	{
		// find start index for first character 
		INT StrANumMatching=0;
		INT StrAStartIdx = StrA.InStr( FString::Printf(TEXT("%c"),StrB[0]) );
		if( StrAStartIdx != INDEX_NONE )
		{
			for( INT Idx=StrAStartIdx; Idx < MaxLength; Idx++ )
			{
				if( StrA[Idx] != StrB[Idx-StrAStartIdx] ) 
				{
					break;
				}
				else 
				{
					StrANumMatching++;
				}
			}
		}

		// find start index for first character 
		INT StrBNumMatching=0;
		INT StrBStartIdx = StrB.InStr( FString::Printf(TEXT("%c"),StrA[0]) );
		if( StrBStartIdx != INDEX_NONE )
		{
			for( INT Idx=StrBStartIdx; Idx < MaxLength; Idx++ )
			{
				if( StrB[Idx] != StrA[Idx-StrBStartIdx] ) 
				{
					break;
				}
				else 
				{
					StrBNumMatching++;
				}
			}
		}

		NumMatching = Max(StrANumMatching, StrBNumMatching);
	}
	return NumMatching;
}

WxDlgMorphLODImport::WxDlgMorphLODImport( wxWindow* InParent, const TArray<UObject*>& InMorphObjects, INT MaxLODIdx, const TCHAR* SrcFileName )
:	wxDialog(InParent,wxID_ANY,*LocalizeUnrealEd("WxDlgMorphLODImport_Title"),wxDefaultPosition,wxDefaultSize,wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
	wxBoxSizer* MainSizer = new wxBoxSizer(wxVERTICAL);
	{
		// filename text
		wxStaticText* FileNameText = new wxStaticText(
			this,
			wxID_ANY,
			*FString::Printf(TEXT("%s %s"),*LocalizeUnrealEd("WxDlgMorphLODImport_Filename"),SrcFileName)
			);

		wxBoxSizer* MorphListHorizSizer = new wxBoxSizer(wxHORIZONTAL);
		{
			// label for morph target list
			wxStaticText* MorphNameText = new wxStaticText(
				this,
				wxID_ANY,
				*LocalizeUnrealEd("WxDlgMorphLODImport_MorphObject")
				);
			// fill in the text for each entry of the morph list dropdown
			// and also find the best matching morph target for the given filename
			wxArrayString MorphListText;
			FString BestMatch;
			INT BestMatchNumChars=-1;
			for( INT MorphIdx=0; MorphIdx < InMorphObjects.Num(); MorphIdx++ )
			{
				UObject* Morph = InMorphObjects(MorphIdx);
				check(Morph);
				if( Morph )
				{
					MorphListText.Add( *Morph->GetName() );
				}
				INT NumMatching = NumMatchingChars(FString(SrcFileName),Morph->GetName());
				if( NumMatching > BestMatchNumChars )
				{
					BestMatch = Morph->GetName();
					BestMatchNumChars = NumMatching;
				}				
			}

			// create the morph list dropdown
			MorphObjectsList = new wxComboBox(
				this,
				wxID_ANY,
				wxString(*BestMatch),
				wxDefaultPosition,
				wxDefaultSize,
				MorphListText,
				wxCB_DROPDOWN|wxCB_READONLY
				);
			// add the lable and dropdown to the sizer
			MorphListHorizSizer->Add( MorphNameText, 0, wxEXPAND|wxALL, 4 );
			MorphListHorizSizer->Add( MorphObjectsList, 1, wxEXPAND|wxALL, 4 );

		}

		wxBoxSizer* LODHorizSizer = new wxBoxSizer(wxHORIZONTAL);
		{
			// label for LOD list
			wxStaticText* LODNameText = new wxStaticText(
				this,
				wxID_ANY,
				*LocalizeUnrealEd("WxDlgMorphLODImport_LODLevel")
				);
			// fill in the text for each LOD index
			wxArrayString LODListText;
			for( INT LODIdx=0; LODIdx <= MaxLODIdx; LODIdx++ )
			{
				LODListText.Add( *FString::Printf(TEXT("%d"),LODIdx) );
			}
			// create the LOD index dropdown
			LODLevelList = new wxComboBox(
				this,
				wxID_ANY,
				wxString(*FString::Printf(TEXT("%d"),MaxLODIdx)),
				wxDefaultPosition,
				wxDefaultSize,
				LODListText,
				wxCB_DROPDOWN|wxCB_READONLY
				);

			LODHorizSizer->Add( LODNameText, 0, wxEXPAND|wxALL, 4 );
			LODHorizSizer->Add( LODLevelList, 1, wxEXPAND|wxALL, 4 );
		}

		MainSizer->Add( FileNameText, 0, wxEXPAND|wxALL, 4 );
		MainSizer->Add( MorphListHorizSizer, 0, wxEXPAND );
		MainSizer->Add( LODHorizSizer, 0, wxEXPAND );

		wxBoxSizer* ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
		{
			// ok button
			wxButton* OkButton = new wxButton(this, wxID_OK, *LocalizeUnrealEd("OK"));
			ButtonSizer->Add(OkButton, 0, wxCENTER | wxALL, 5);
			// cancel button
			wxButton* CancelButton = new wxButton(this, wxID_CANCEL, *LocalizeUnrealEd("Cancel"));
			ButtonSizer->Add(CancelButton, 0, wxCENTER | wxALL, 5);
		}
		MainSizer->Add(ButtonSizer, 0, wxALIGN_RIGHT | wxALL, 2);
	}

	SetSizer(MainSizer);
	FWindowUtil::LoadPosSize(TEXT("DlgMorphLODImport"),this,-1,-1,350,150);
}

WxDlgMorphLODImport::~WxDlgMorphLODImport()
{
	FWindowUtil::SavePosSize(TEXT("DlgMorphLODImport"),this);
}

/*-----------------------------------------------------------------------------
	XDlgSurfPropPage1.
-----------------------------------------------------------------------------*/

WxSurfacePropertiesPanel::WxSurfacePropertiesPanel( wxWindow* InParent )
	:	wxPanel( InParent, -1 ),
		bUseSimpleScaling( TRUE )
{
	Panel = (wxPanel*)wxXmlResource::Get()->LoadPanel( this, TEXT("ID_SURFPROP_PANROTSCALE") );
	check( Panel != NULL );
	SimpleCB = (wxComboBox*)FindWindow( XRCID( "IDCB_SIMPLE" ) );
	check( SimpleCB != NULL );
	CustomULabel = (wxStaticText*)FindWindow( XRCID( "IDSC_CUSTOM_U" ) );
	check( CustomULabel != NULL );
	CustomVLabel = (wxStaticText*)FindWindow( XRCID( "IDSC_CUSTOM_V" ) );
	check( CustomVLabel != NULL );
	CustomUEdit = (wxTextCtrl*)FindWindow( XRCID( "IDEC_CUSTOM_U" ) );
	check( CustomUEdit != NULL );
	CustomVEdit = (wxTextCtrl*)FindWindow( XRCID( "IDEC_CUSTOM_V" ) );
	check( CustomVEdit != NULL );
	SimpleScaleButton = (wxRadioButton*)FindWindow( XRCID( "IDRB_SIMPLE" ) );
	check( SimpleScaleButton != NULL );
	CustomScaleButton = (wxRadioButton*)FindWindow( XRCID( "IDRB_CUSTOM" ) );
	check( CustomScaleButton != NULL );
	RelativeCheck = (wxCheckBox*)FindWindow( XRCID( "IDCK_RELATIVE" ) );
	check( RelativeCheck != NULL );
	LightMapResCombo = (wxComboBox*)FindWindow( XRCID( "IDCB_LIGHTMAPRES" ) );
	check( LightMapResCombo != NULL );
	AcceptsLightsCheck = (wxCheckBox*)FindWindow( XRCID( "IDCK_ACCEPTSLIGHTS" ) );
	check( AcceptsLightsCheck != NULL );
	AcceptsDynamicLightsCheck = (wxCheckBox*)FindWindow( XRCID( "IDCK_ACCEPTSDYNAMICLIGHTS" ) );
	check( AcceptsDynamicLightsCheck != NULL );
	ForceLightMapCheck = (wxCheckBox*)FindWindow( XRCID( "IDCK_FORCELIGHTMAP" ) );
	check( ForceLightMapCheck != NULL );

	SimpleScaleButton->SetValue( 1 );

	ADDEVENTHANDLER( XRCID("IDPB_U_1"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnU1 );
	ADDEVENTHANDLER( XRCID("IDPB_U_4"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnU4 );
	ADDEVENTHANDLER( XRCID("IDPB_U_16"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnU16 );
	ADDEVENTHANDLER( XRCID("IDPB_U_64"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnU64 );
	ADDEVENTHANDLER( XRCID("IDPB_U_CUSTOM"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnUCustom );

	ADDEVENTHANDLER( XRCID("IDPB_V_1"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnV1 );
	ADDEVENTHANDLER( XRCID("IDPB_V_4"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnV4 );
	ADDEVENTHANDLER( XRCID("IDPB_V_16"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnV16 );
	ADDEVENTHANDLER( XRCID("IDPB_V_64"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnV64 );
	ADDEVENTHANDLER( XRCID("IDPB_V_CUSTOM"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnVCustom );

	ADDEVENTHANDLER( XRCID("IDPB_ROT_45"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnRot45 );
	ADDEVENTHANDLER( XRCID("IDPB_ROT_90"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnRot90 );
	ADDEVENTHANDLER( XRCID("IDPB_ROT_CUSTOM"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnRotCustom );

	ADDEVENTHANDLER( XRCID("IDPB_FLIP_U"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnFlipU );
	ADDEVENTHANDLER( XRCID("IDPB_FLIP_V"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnFlipV );
	
	ADDEVENTHANDLER( XRCID("IDPB_APPLY"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnApply );
	ADDEVENTHANDLER( XRCID("IDRB_SIMPLE"), wxEVT_COMMAND_RADIOBUTTON_SELECTED, &WxSurfacePropertiesPanel::OnScaleSimple );
	ADDEVENTHANDLER( XRCID("IDRB_CUSTOM"), wxEVT_COMMAND_RADIOBUTTON_SELECTED, &WxSurfacePropertiesPanel::OnScaleCustom );
	ADDEVENTHANDLER( XRCID("IDCB_LIGHTMAPRES"), wxEVT_COMMAND_COMBOBOX_SELECTED, &WxSurfacePropertiesPanel::OnLightMapResSelChange );
	ADDEVENTHANDLER( XRCID("IDCK_ACCEPTSLIGHTS"), wxEVT_COMMAND_CHECKBOX_CLICKED, &WxSurfacePropertiesPanel::OnAcceptsLightsChange );
	ADDEVENTHANDLER( XRCID("IDCK_ACCEPTSDYNAMICLIGHTS"), wxEVT_COMMAND_CHECKBOX_CLICKED, &WxSurfacePropertiesPanel::OnAcceptsDynamicLightsChange );
	ADDEVENTHANDLER( XRCID("IDCK_FORCELIGHTMAP"), wxEVT_COMMAND_CHECKBOX_CLICKED, &WxSurfacePropertiesPanel::OnForceLightmapChange );

	CustomULabel->Disable();
	CustomVLabel->Disable();
	CustomUEdit->Disable();
	CustomVEdit->Disable();

	
	// Setup alignment properties.
	AlignList = (wxListBox*)FindWindow( XRCID( "IDLB_ALIGNMENT" ) );
	check( AlignList );

	ADDEVENTHANDLER( XRCID("IDLB_ALIGNMENT"), wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, &WxSurfacePropertiesPanel::OnApplyAlign );
	ADDEVENTHANDLER( XRCID("IDLB_ALIGNMENT"), wxEVT_COMMAND_LISTBOX_SELECTED, &WxSurfacePropertiesPanel::OnAlignSelChange );
	ADDEVENTHANDLER( XRCID("IDPB_APPLYALIGN"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnApplyAlign);

	// Initialize controls.
	for( INT x = 0 ; x < GTexAlignTools.Aligners.Num() ; ++x )
	{
		AlignList->Append( *GTexAlignTools.Aligners(x)->Desc );
	}
	AlignList->SetSelection( 0 );

	// Property window for the texture alignment tools.
	{
		wxPanel* PropertyPanel;
		PropertyPanel = (wxPanel*)FindWindow( XRCID( "ID_PROPERTYWINDOW_PANEL" ) );
		check( PropertyPanel );

		wxStaticBoxSizer* PropertySizer = new wxStaticBoxSizer(wxVERTICAL, PropertyPanel, TEXT("Options"));
		{
			PropertyWindow = new WxPropertyWindow;
			PropertyWindow->Create( PropertyPanel, GUnrealEd );
			
			wxSize PanelSize = PropertyPanel->GetSize();
			PanelSize.SetWidth(PanelSize.GetWidth() - 5);
			PanelSize.SetHeight(PanelSize.GetHeight() - 15);
			PropertyWindow->SetMinSize(PanelSize);

			PropertySizer->Add(PropertyWindow, 1, wxEXPAND);
		}
		PropertyPanel->SetSizer(PropertySizer);
		PropertySizer->Fit(PropertyPanel);
	}

	// Property window for the surface lighting channels.
	{
		wxPanel* PropertyPanel;
		PropertyPanel = (wxPanel*)FindWindow( XRCID( "ID_ChannelsPropertyWindow" ) );
		check( PropertyPanel );

		wxStaticBoxSizer* PropertySizer = new wxStaticBoxSizer(wxVERTICAL, PropertyPanel, TEXT("LightingChannels"));
		{
			ChannelsPropertyWindow = new WxPropertyWindow;
			ChannelsPropertyWindow->Create( PropertyPanel, this );
			
			wxSize PanelSize = PropertyPanel->GetSize();
			PanelSize.SetWidth(PanelSize.GetWidth() - 5);
			PanelSize.SetHeight(PanelSize.GetHeight() - 15);
			ChannelsPropertyWindow->SetMinSize(PanelSize);

			PropertySizer->Add(ChannelsPropertyWindow, 1, wxEXPAND);
		}
		PropertyPanel->SetSizer(PropertySizer);
		PropertySizer->Fit(PropertyPanel);
	}

	// Allocate a new surface lighting channels object.
	LightingChannelsObject = ConstructObject<ULightingChannelsObject>( ULightingChannelsObject::StaticClass() );
}

void WxSurfacePropertiesPanel::Pan( INT InU, INT InV )
{
	const FLOAT Mod = GetAsyncKeyState(VK_SHIFT) & 0x8000 ? -1.f : 1.f;
	GUnrealEd->Exec( *FString::Printf( TEXT("POLY TEXPAN U=%f V=%f"), InU * Mod, InV * Mod ) );
}

void WxSurfacePropertiesPanel::Scale( FLOAT InScaleU, FLOAT InScaleV, UBOOL InRelative )
{
	if( InScaleU == 0.f )
	{
		InScaleU = 1.f;
	}
	if( InScaleV == 0.f )
	{
		InScaleV = 1.f;
	}

	InScaleU = 1.0f / InScaleU;
	InScaleV = 1.0f / InScaleV;

	GUnrealEd->Exec( *FString::Printf( TEXT("POLY TEXSCALE %s UU=%f VV=%f"), InRelative?TEXT("RELATIVE"):TEXT(""), InScaleU, InScaleV ) );
}

void WxSurfacePropertiesPanel::RefreshPage()
{
	// Find the shadowmap scale on the selected surfaces.

	FLOAT	ShadowMapScale				= 0.0f;
	UBOOL	bValidScale					= FALSE;
	// Keep track of how many surfaces are selected and how many have the options set.
	INT		AcceptsLightsCount			= 0;
	INT		AcceptsDynamicLightsCount	= 0;
	INT		ForceLightMapCount			= 0;
	INT		SelectedSurfaceCount		= 0;

	LightingChannelsObject->LightingChannels.Bitfield = 0;
	LightingChannelsObject->LightingChannels.bInitialized = TRUE;

	UBOOL bSawSelectedSurface = FALSE;
	if ( GWorld )
	{
		for ( INT LevelIndex = 0 ; LevelIndex < GWorld->Levels.Num() ; ++LevelIndex )
		{
			const ULevel* Level = GWorld->Levels(LevelIndex);
			const UModel* Model = Level->Model;
			for(INT SurfaceIndex = 0; SurfaceIndex < Model->Surfs.Num();SurfaceIndex++)
			{
				const FBspSurf&	Surf =  Model->Surfs(SurfaceIndex);

				if(Surf.PolyFlags & PF_Selected)
				{
					bSawSelectedSurface = TRUE;
					LightingChannelsObject->LightingChannels.Bitfield |= Surf.LightingChannels.Bitfield;

					if(SelectedSurfaceCount == 0)
					{
						ShadowMapScale = Surf.ShadowMapScale;
						bValidScale = TRUE;
					}
					else if(bValidScale && ShadowMapScale != Surf.ShadowMapScale)
					{
						bValidScale = FALSE;
					}

					// Keep track of selected surface count and how many have the particular options set so we can display
					// the correct state of checked, unchecked or mixed.
					SelectedSurfaceCount++;
					if( Surf.PolyFlags & PF_AcceptsLights )
					{
						AcceptsLightsCount++;
					}
					if( Surf.PolyFlags & PF_AcceptsDynamicLights )
					{
						AcceptsDynamicLightsCount++;
					}
					if( Surf.PolyFlags & PF_ForceLightMap )
					{
						ForceLightMapCount++;
					}
				}
			}
		}
	}

	// Select the appropriate scale.
	INT	ScaleItem = INDEX_NONE;

	if( bValidScale )
	{
		const FString ScaleString = FString::Printf(TEXT("%.1f"),ShadowMapScale);

		ScaleItem = LightMapResCombo->FindString(*ScaleString);

		if(ScaleItem == INDEX_NONE)
		{
			ScaleItem = LightMapResCombo->GetCount();
			LightMapResCombo->Append( *ScaleString );
		}
	}

	LightMapResCombo->SetSelection( ScaleItem );

	// Set AcceptsLights state.
	if( AcceptsLightsCount == 0 )
	{
		AcceptsLightsCheck->Set3StateValue( wxCHK_UNCHECKED );
	}
	else if( AcceptsLightsCount == SelectedSurfaceCount )
	{
		AcceptsLightsCheck->Set3StateValue( wxCHK_CHECKED );
	}
	else
	{
		AcceptsLightsCheck->Set3StateValue( wxCHK_UNDETERMINED );
	}

	// Set AcceptsDynamicLights state.
	if( AcceptsDynamicLightsCount == 0 )
	{
		AcceptsDynamicLightsCheck->Set3StateValue( wxCHK_UNCHECKED );
	}
	else if( AcceptsDynamicLightsCount == SelectedSurfaceCount )
	{
		AcceptsDynamicLightsCheck->Set3StateValue( wxCHK_CHECKED );
	}
	else
	{
		AcceptsDynamicLightsCheck->Set3StateValue( wxCHK_UNDETERMINED );
	}

	// Set ForceLightMap state.
	if( ForceLightMapCount == 0 )
	{
		ForceLightMapCheck->Set3StateValue( wxCHK_UNCHECKED );
	}
	else if( ForceLightMapCount == SelectedSurfaceCount )
	{
		ForceLightMapCheck->Set3StateValue( wxCHK_CHECKED );
	}
	else
	{
		ForceLightMapCheck->Set3StateValue( wxCHK_UNDETERMINED );
	}

	// Refresh property windows.
	PropertyWindow->Freeze();
	PropertyWindow->SetObject( GTexAlignTools.Aligners(AlignList->GetSelection()), TRUE, FALSE, FALSE );
	PropertyWindow->Thaw();
	ChannelsPropertyWindow->Freeze();
	if ( bSawSelectedSurface )
	{
		ChannelsPropertyWindow->SetObject( LightingChannelsObject, FALSE, FALSE, FALSE );
		ChannelsPropertyWindow->ExpandAllItems();
	}
	else
	{
		ChannelsPropertyWindow->GetRoot()->RemoveAllObjects();
	}
	ChannelsPropertyWindow->Thaw();
}

/** Sets lighting channels for selected surfaces to the specified value. */
void WxSurfacePropertiesPanel::SetLightingChannelsForSelectedSurfaces(DWORD NewBitfield)
{
	UBOOL bSawLightingChannelsChange = FALSE;
	for ( INT LevelIndex = 0 ; LevelIndex < GWorld->Levels.Num() ; ++LevelIndex )
	{
		ULevel* Level = GWorld->Levels(LevelIndex);
		UModel* Model = Level->Model;
		for ( INT SurfaceIndex = 0 ; SurfaceIndex < Model->Surfs.Num() ; ++SurfaceIndex )
		{
			FBspSurf& Surf = Model->Surfs(SurfaceIndex);
			if ( Surf.PolyFlags & PF_Selected )
			{
				if ( Surf.LightingChannels.Bitfield != NewBitfield )
				{
					Surf.LightingChannels.Bitfield = NewBitfield;
					Surf.Actor->Brush->Polys->Element(Surf.iBrushPoly).LightingChannels = NewBitfield;
					bSawLightingChannelsChange = TRUE;
				}
			}
		}
	}
	if ( bSawLightingChannelsChange )
	{
		GWorld->MarkPackageDirty();
		GCallbackEvent->Send( CALLBACK_LevelDirtied );
	}
}

void WxSurfacePropertiesPanel::NotifyPostChange( void* Src, UProperty* PropertyThatChanged )
{
	SetLightingChannelsForSelectedSurfaces( LightingChannelsObject->LightingChannels.Bitfield );
}

void WxSurfacePropertiesPanel::OnU1( wxCommandEvent& In )
{
	Pan( 1, 0 );
}

void WxSurfacePropertiesPanel::OnU4( wxCommandEvent& In )
{
	Pan( 4, 0 );
}

void WxSurfacePropertiesPanel::OnU16( wxCommandEvent& In )
{
	Pan( 16, 0 );
}

void WxSurfacePropertiesPanel::OnU64( wxCommandEvent& In )
{
	Pan( 64, 0 );
}

void WxSurfacePropertiesPanel::OnUCustom( wxCommandEvent& In )
{
	wxTextEntryDialog Dlg(this, *LocalizeUnrealEd(TEXT("UPanAmount")), *LocalizeUnrealEd(TEXT("PanU")));

	if(Dlg.ShowModal() == wxID_OK)
	{
		const wxString StrValue = Dlg.GetValue();
		const INT PanValue = appAtoi(StrValue);

		Pan( PanValue, 0 );
	}

}

void WxSurfacePropertiesPanel::OnV1( wxCommandEvent& In )
{
	Pan( 0, 1 );
}

void WxSurfacePropertiesPanel::OnV4( wxCommandEvent& In )
{
	Pan( 0, 4 );
}

void WxSurfacePropertiesPanel::OnV16( wxCommandEvent& In )
{
	Pan( 0, 16 );
}

void WxSurfacePropertiesPanel::OnV64( wxCommandEvent& In )
{
	Pan( 0, 64 );
}

void WxSurfacePropertiesPanel::OnVCustom( wxCommandEvent& In )
{
	wxTextEntryDialog Dlg(this, *LocalizeUnrealEd(TEXT("VPanAmount")), *LocalizeUnrealEd(TEXT("PanV")));

	if(Dlg.ShowModal() == wxID_OK)
	{
		const wxString StrValue = Dlg.GetValue();
		const INT PanValue = appAtoi(StrValue);

		Pan( 0, PanValue );
	}
}

void WxSurfacePropertiesPanel::OnFlipU( wxCommandEvent& In )
{
	GUnrealEd->Exec( TEXT("POLY TEXMULT UU=-1 VV=1") );
}

void WxSurfacePropertiesPanel::OnFlipV( wxCommandEvent& In )
{
	GUnrealEd->Exec( TEXT("POLY TEXMULT UU=1 VV=-1") );
}

void WxSurfacePropertiesPanel::OnRot45( wxCommandEvent& In )
{
	const FLOAT Mod = GetAsyncKeyState(VK_SHIFT) & 0x8000 ? -1.f : 1.f;
	const FLOAT UU = 1.0f / appSqrt(2.f);
	const FLOAT VV = 1.0f / appSqrt(2.f);
	const FLOAT UV = (1.0f / appSqrt(2.f)) * Mod;
	const FLOAT VU = -(1.0f / appSqrt(2.f)) * Mod;
	GUnrealEd->Exec( *FString::Printf( TEXT("POLY TEXMULT UU=%f VV=%f UV=%f VU=%f"), UU, VV, UV, VU ) );
}

void WxSurfacePropertiesPanel::OnRot90( wxCommandEvent& In )
{
	const FLOAT Mod = GetAsyncKeyState(VK_SHIFT) & 0x8000 ? -1 : 1;
	const FLOAT UU = 0.f;
	const FLOAT VV = 0.f;
	const FLOAT UV = 1.f * Mod;
	const FLOAT VU = -1.f * Mod;
	GUnrealEd->Exec( *FString::Printf( TEXT("POLY TEXMULT UU=%f VV=%f UV=%f VU=%f"), UU, VV, UV, VU ) );
}

void WxSurfacePropertiesPanel::OnRotCustom( wxCommandEvent& In )
{
	wxTextEntryDialog Dlg(this, *LocalizeUnrealEd(TEXT("RotationAmount")), *LocalizeUnrealEd(TEXT("Rotation")));

	if(Dlg.ShowModal() == wxID_OK)
	{
		wxString StrValue = Dlg.GetValue();
		const FLOAT RotateDegrees = appAtof(StrValue);
		const FLOAT RotateRadians = RotateDegrees / 180.0f * PI;

		const FLOAT UU = cos(RotateRadians);
		const FLOAT VV = UU;
		const FLOAT UV = -sin(RotateRadians);
		const FLOAT VU = sin(RotateRadians);
		GUnrealEd->Exec( *FString::Printf( TEXT("POLY TEXMULT UU=%f VV=%f UV=%f VU=%f"), UU, VV, UV, VU ) );
	}


}

void WxSurfacePropertiesPanel::OnApply( wxCommandEvent& In )
{
	FLOAT UScale, VScale;

	if( bUseSimpleScaling )
	{
		UScale = VScale = appAtof( SimpleCB->GetValue() );
	}
	else
	{
		UScale = appAtof( CustomUEdit->GetValue() );
		VScale = appAtof( CustomVEdit->GetValue() );
	}

	const UBOOL bRelative = RelativeCheck->GetValue();
	Scale( UScale, VScale, bRelative );
}

void WxSurfacePropertiesPanel::OnScaleSimple( wxCommandEvent& In )
{
	bUseSimpleScaling = TRUE;

	CustomULabel->Disable();
	CustomVLabel->Disable();
	CustomUEdit->Disable();
	CustomVEdit->Disable();
	SimpleCB->Enable();
}

void WxSurfacePropertiesPanel::OnScaleCustom( wxCommandEvent& In )
{
	bUseSimpleScaling = FALSE;

	CustomULabel->Enable();
	CustomVLabel->Enable();
	CustomUEdit->Enable();
	CustomVEdit->Enable();
	SimpleCB->Disable();
}

void WxSurfacePropertiesPanel::OnLightMapResSelChange( wxCommandEvent& In )
{
	const FLOAT ShadowMapScale = appAtof(LightMapResCombo->GetString(LightMapResCombo->GetSelection()));

	for ( INT LevelIndex = 0 ; LevelIndex < GWorld->Levels.Num() ; ++LevelIndex )
	{
		ULevel* Level = GWorld->Levels(LevelIndex);
		UModel* Model = Level->Model;
		for(INT SurfaceIndex = 0;SurfaceIndex < Model->Surfs.Num();SurfaceIndex++)
		{
			FBspSurf&	Surf = Model->Surfs(SurfaceIndex);

			if(Surf.PolyFlags & PF_Selected)
			{
				Surf.Actor->Brush->Polys->Element(Surf.iBrushPoly).ShadowMapScale = ShadowMapScale;
				Surf.ShadowMapScale = ShadowMapScale;
			}
		}
	}

	GWorld->MarkPackageDirty();
	GCallbackEvent->Send( CALLBACK_LevelDirtied );
}

/**
 * Sets passed in poly flag on selected surfaces.
 *
 * @param PolyFlag	PolyFlag to toggle on selected surfaces
 * @param Value		Value to set the flag to.
 */
void WxSurfacePropertiesPanel::SetPolyFlag( DWORD PolyFlag, UBOOL Value )
{
	check( PolyFlag & PF_ModelComponentMask );
	{
		const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("ToggleSurfaceFlags")) );
		for ( INT LevelIndex = 0 ; LevelIndex < GWorld->Levels.Num() ; ++LevelIndex )
		{
			ULevel* Level = GWorld->Levels(LevelIndex);
			UModel* Model = Level->Model;
			Model->ModifySelectedSurfs( TRUE );

			for( INT SurfaceIndex=0; SurfaceIndex<Model->Surfs.Num(); SurfaceIndex++ )
			{
				FBspSurf& Surf = Model->Surfs(SurfaceIndex);
				if( Surf.PolyFlags & PF_Selected )
				{
					if(Value)
					{
						Surf.PolyFlags |= PolyFlag;
					}
					else
					{
						Surf.PolyFlags &= ~PolyFlag;
					}

					// Propagate toggled flags to poly.
					GEditor->polyUpdateMaster( Model, SurfaceIndex, TRUE );
				}
			}
		}
	}

	GWorld->MarkPackageDirty();
	GCallbackEvent->Send( CALLBACK_LevelDirtied );
}

void WxSurfacePropertiesPanel::OnAcceptsLightsChange( wxCommandEvent& In )
{
	SetPolyFlag( PF_AcceptsLights,  In.IsChecked());
}
void WxSurfacePropertiesPanel::OnAcceptsDynamicLightsChange( wxCommandEvent& In )
{
	SetPolyFlag( PF_AcceptsDynamicLights,  In.IsChecked() );
}
void WxSurfacePropertiesPanel::OnForceLightmapChange( wxCommandEvent& In )
{
	SetPolyFlag( PF_ForceLightMap,  In.IsChecked() );
}



void WxSurfacePropertiesPanel::OnAlignSelChange( wxCommandEvent& In )
{
	RefreshPage();
}

void WxSurfacePropertiesPanel::OnApplyAlign( wxCommandEvent& In )
{
	UTexAligner* SelectedAligner = GTexAlignTools.Aligners( AlignList->GetSelection() );
	for ( INT LevelIndex = 0 ; LevelIndex < GWorld->Levels.Num() ; ++LevelIndex )
	{
		ULevel* Level = GWorld->Levels(LevelIndex);
		SelectedAligner->Align( TEXALIGN_None, Level->Model );
	}
}

/*-----------------------------------------------------------------------------
	WxDlgSurfaceProperties.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE(WxDlgSurfaceProperties, wxDialog)
END_EVENT_TABLE()

WxDlgSurfaceProperties::WxDlgSurfaceProperties() : 
wxDialog(GApp->EditorFrame, wxID_ANY, TEXT("SurfaceProperties"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxSYSTEM_MENU | wxCAPTION)
{

	wxBoxSizer* DialogSizer = new wxBoxSizer(wxVERTICAL);
	{
		PropertiesPanel = new WxSurfacePropertiesPanel( this );
		DialogSizer->Add( PropertiesPanel, 1, wxEXPAND );
	}
	SetSizer(DialogSizer);

	
	FWindowUtil::LoadPosSize( TEXT("SurfaceProperties"), this );

	Fit();
	RefreshPages();
	FLocalizeWindow( this );
}

WxDlgSurfaceProperties::~WxDlgSurfaceProperties()
{
	FWindowUtil::SavePosSize( TEXT("SurfaceProperties"), this );
}

void WxDlgSurfaceProperties::RefreshPages()
{
	PropertiesPanel->RefreshPage();
}

/*-----------------------------------------------------------------------------
	WxDlgColor.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE(WxDlgColor, wxColourDialog)
EVT_BUTTON( wxID_OK, WxDlgColor::OnOK )
END_EVENT_TABLE()

WxDlgColor::WxDlgColor()
{
}

WxDlgColor::~WxDlgColor()
{
	wxColourData cd = GetColourData();
	SaveColorData( &cd );
}

bool WxDlgColor::Create( wxWindow* InParent, wxColourData* InData )
{
	LoadColorData( InData );

	return wxColourDialog::Create( InParent, InData );
}

/**
* Loads custom color data from the INI file.
*
* @param	InData	The color data structure to load into
*/

void WxDlgColor::LoadColorData( wxColourData* InData )
{
	for( INT x = 0 ; x < 16 ; ++x )
	{
		const FString Key = FString::Printf( TEXT("Color%d"), x );
		INT Color = wxColour(255,255,255).GetPixel();
		GConfig->GetInt( TEXT("ColorDialog"), *Key, Color, GEditorUserSettingsIni );
		InData->SetCustomColour( x, wxColour( Color ) );
	}
}

int WxDlgColor::ShowModal()
{
	MakeModal(true);
	const int ReturnCode = wxColourDialog::ShowModal();
	MakeModal(false);
	return ReturnCode;
}

/**
* Saves custom color data to the INI file.
*
* @param	InData	The color data structure to save from
*/

void WxDlgColor::SaveColorData( wxColourData* InData )
{
	for( INT x = 0 ; x < 16 ; ++x )
	{
		const FString Key = FString::Printf( TEXT("Color%d"), x );
		GConfig->SetInt( TEXT("ColorDialog"), *Key, InData->GetCustomColour(x).GetPixel(), GEditorUserSettingsIni );
	}
}

/*-----------------------------------------------------------------------------
	WxDlgLoadErrors.
-----------------------------------------------------------------------------*/

WxDlgLoadErrors::WxDlgLoadErrors( wxWindow* InParent )
{
	const bool bSuccess = wxXmlResource::Get()->LoadDialog( this, InParent, TEXT("ID_DLG_LOAD_ERRORS") );
	check( bSuccess );

	PackageList = wxDynamicCast( FindWindow( XRCID( "IDLB_FILES" ) ), wxListBox );
	check( PackageList != NULL );
	ObjectList = wxDynamicCast( FindWindow( XRCID( "IDLB_OBJECTS" ) ), wxListBox );
	check( ObjectList != NULL );

	FLocalizeWindow( this );
}

void WxDlgLoadErrors::Update()
{
	PackageList->Clear();
	ObjectList->Clear();

	// Use Windows methods since wxWindows doesn't give us what we need
	HWND hWnd = (HWND)GetHandle();
	HDC hDC = ::GetDC(hWnd);
	DWORD MaxWidth = 0;

	for( INT x = 0 ; x < GEdLoadErrors.Num() ; ++x )
	{
		if( GEdLoadErrors(x).Type == FEdLoadError::TYPE_FILE )
		{
			PackageList->Append( *GEdLoadErrors(x).Desc );
		}
		else
		{
			ObjectList->Append( *GEdLoadErrors(x).Desc );
			// Figure out the size of the string
			DWORD StrWidth = LOWORD(::GetTabbedTextExtent(hDC,*GEdLoadErrors(x).Desc,
				GEdLoadErrors(x).Desc.Len(),0,NULL));
			if (MaxWidth < StrWidth)
			{
				MaxWidth = StrWidth;
			}
		}
	}
	// Done with the DC so let it go
	::ReleaseDC(hWnd,hDC);
	// Get the object list rect so we know how much to change the dialog size by
	wxRect ObjectListRect = ObjectList->GetRect();
	wxSize DialogSize = GetClientSize();
	// If the dialog's right edge is less than our needed size, grow the dialog
	INT AdjustBy = (ObjectListRect.GetLeft() + MaxWidth + 8) - DialogSize.GetWidth();
	if (AdjustBy > 0)
	{
		SetSize(DialogSize.GetWidth() + AdjustBy,DialogSize.GetHeight());
	}
	Center();
}


void WxDlgColor::OnOK(wxCommandEvent& event)
{
	wxDialog::AcceptAndClose();
}

UBOOL PromptUserForDirectory(FString& OutDirectory, const FName& Message, const FName& DefaultPath)
{
	UBOOL bResult = TRUE;

	wxString strDefPath;
	if (DefaultPath != NAME_None)
	{
		strDefPath = *DefaultPath.ToString();
	}

	wxDirDialog	DirDialog(GApp->EditorFrame, wxString(*Message.ToString()), strDefPath);
	if (DirDialog.ShowModal() == wxID_OK)
	{
		OutDirectory = FString(DirDialog.GetPath());
	}
	else
	{
		OutDirectory = TEXT("");
		bResult = FALSE;
	}

	return bResult;
}
