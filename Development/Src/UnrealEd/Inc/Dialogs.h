/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DIALOGS_H__
#define __DIALOGS_H__

/*-----------------------------------------------------------------------------
	WxDlgBindHotkeys
-----------------------------------------------------------------------------*/

/**
 * This class is used to bind hotkeys for the editor.
 */
class WxDlgBindHotkeys : public wxFrame
{
public:
	WxDlgBindHotkeys(wxWindow* InParent);
	~WxDlgBindHotkeys();

	/** Saves settings for this dialog to the INI. */
	void SaveSettings();

	/** Loads settings for this dialog from the INI. */
	void LoadSettings();

	/** Starts the binding process for the current command. */
	void StartBinding(INT CommandIdx);

	/** Finishes the binding for the current command using the provided event. */
	void FinishBinding(wxKeyEvent &Event);

	/** Stops the binding process for the current command. */
	void StopBinding();

	/** @return Whether or not we are currently binding a command. */
	UBOOL IsBinding() 
	{
		return CurrentlyBindingIdx != -1;
	}

	/**
	 * @return Generates a descriptive binding string based on the key combinations provided.
	 */
	FString GenerateBindingText(UBOOL bAltDown, UBOOL bCtrlDown, UBOOL bShiftDown, FName Key);

private:
	
	/** Builds the category tree. */
	void BuildCategories();

	/** Builds the command list using the currently selected category. */
	void BuildCommands();

	/** Refreshes the binding text for the currently visible binding widgets. */
	void RefreshBindings();

	/** Updates the scrollbar for the command view. */
	void UpdateScrollbar();

	/** Window closed event handler. */
	void OnClose(wxCloseEvent& Event);

	/** Category selected handler. */
	void OnCategorySelected(wxTreeEvent& Event);

	/** Handler to let the user load a config from a file. */
	void OnLoadConfig(wxCommandEvent& Event);
	
	/** Handler to let the user save the current config to a file. */
	void OnSaveConfig(wxCommandEvent& Event);
	
	/** Handler to reset bindings to default. */
	void OnResetToDefaults(wxCommandEvent& Event);

	/** Bind key button pressed handler. */
	void OnBindKey(wxCommandEvent& Event);

	/** OK Button pressed handler. */
	void OnOK(wxCommandEvent &Event);

	/** Handler for key binding events. */
	void OnKeyDown(wxKeyEvent& Event);

	/** Which command we are currently binding a key to. */
	INT CurrentlyBindingIdx;

	/** Tree control to display all of the available command categories to bind to. */
	WxTreeCtrl* CommandCategories;

	/** Splitter to separate tree and command views. */
	wxSplitterWindow* MainSplitter;

	/** Panel to store commands. */
	wxScrolledWindow* CommandPanel;

	/** Currently binding label text. */
	wxStaticText* BindLabel;

	/** Size of 1 command item. */
	INT ItemSize;

	/** Number of currently visible commands. */
	INT NumVisibleItems;

	/** The list of currently visible commands. */
	TArray<FEditorCommand> VisibleCommands;

	/** Struct of controls in a command panel. */
	struct FCommandPanel
	{
		wxTextCtrl* BindingWidget;
		wxButton* BindingButton;
		wxPanel* BindingPanel;
		FName CommandName;
	};

	/** Array of currently visible binding controls. */
	TArray<FCommandPanel> VisibleBindingControls;

	/** A mapping of category names to their array of commands. */
	TMap< FName, TArray<FEditorCommand> >	CommandMap;

	/** Mapping of category names to tree id's. */
	TMap<FName, wxTreeItemId> ParentMap;

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxDlgPackageGroupName.
-----------------------------------------------------------------------------*/

class WxDlgPackageGroupName : public wxDialog
{
public:
	WxDlgPackageGroupName();
	virtual ~WxDlgPackageGroupName();

	const FString& GetPackage() const
	{
		return Package;
	}
	const FString& GetGroup() const
	{
		return Group;
	}
	const FString& GetObjectName() const
	{
		return Name;
	}

	int ShowModal(const FString& InPackage, const FString& InGroup, const FString& InName );
	virtual bool Validate();
	UBOOL ProcessNewAssetDlg(UPackage** NewObjPackage, FString* NewObjName, UBOOL bAllowCreateOverExistingOfSameType, UClass* NewObjClass);

protected:
	FString Package, Group, Name;
	wxBoxSizer* PGNSizer;
	wxPanel* PGNPanel;
	WxPkgGrpNameCtrl* PGNCtrl;


private:
	void OnOK( wxCommandEvent& In );

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxDlgNewArchetype.
-----------------------------------------------------------------------------*/

class WxDlgNewArchetype : public WxDlgPackageGroupName
{
public:
	/////////////////////////
	// wxWindow interface.

	virtual bool Validate();
};

/*-----------------------------------------------------------------------------
	WxDlgAddSpecial.
-----------------------------------------------------------------------------*/

class WxDlgAddSpecial : public wxDialog
{
public:
	WxDlgAddSpecial();
	virtual ~WxDlgAddSpecial();

private:
	wxCheckBox *PortalCheck, *InvisibleCheck, *TwoSidedCheck;
	wxRadioButton *SolidRadio, *SemiSolidRadio, *NonSolidRadio;

	void OnOK( wxCommandEvent& In );

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxDlgGenericStringEntry
-----------------------------------------------------------------------------*/

class WxDlgGenericStringEntry : public wxDialog
{
public:
	WxDlgGenericStringEntry();
	virtual ~WxDlgGenericStringEntry();

	const FString& GetEnteredString() const
	{
		return EnteredString;
	}
	wxTextCtrl& GetStringEntry() const
	{
		return *StringEntry;
	}
	wxStaticText& GetStringCaption() const
	{
		return *StringCaption;
	}

	int ShowModal( const TCHAR* DialogTitle, const TCHAR* Caption, const TCHAR* DefaultString );

	void Init();

private:
	FString			EnteredString;
	wxTextCtrl		*StringEntry;
	wxStaticText	*StringCaption;

	void OnOK( wxCommandEvent& In );

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxDlgGenericStringWrappedEntry
-----------------------------------------------------------------------------*/

class WxDlgGenericStringWrappedEntry : public wxDialog
{
public:
	WxDlgGenericStringWrappedEntry();
	virtual ~WxDlgGenericStringWrappedEntry();

	const FString& GetEnteredString() const
	{
		return EnteredString;
	}
	const wxTextCtrl& GetStringEntry() const
	{
		return *StringEntry;
	}

	int ShowModal( const TCHAR* DialogTitle, const TCHAR* Caption, const TCHAR* DefaultString );

	void Init();

private:
	FString			EnteredString;
	wxTextCtrl		*StringEntry;
	wxStaticText	*StringCaption;

	void OnOK( wxCommandEvent& In );

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxDlgMorphLODImport
-----------------------------------------------------------------------------*/

class WxDlgMorphLODImport : public wxDialog
{
public:
	WxDlgMorphLODImport( wxWindow* InParent, const TArray<UObject*>& InMorphObjects, INT MaxLODIdx, const TCHAR* SrcFileName );
	virtual ~WxDlgMorphLODImport();

	const INT GetSelectedMorph() const
	{
		return MorphObjectsList->GetSelection();
	}

	const INT GetLODLevel() const
	{
		return LODLevelList->GetSelection();
	}

private:
	wxComboBox*	MorphObjectsList;
	wxComboBox* LODLevelList;

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxDlgSurfaceProperties
-----------------------------------------------------------------------------*/

class WxDlgSurfaceProperties : public wxDialog
{
public:
	WxDlgSurfaceProperties();
	virtual ~WxDlgSurfaceProperties();

	void RefreshPages();

private:
	class WxSurfacePropertiesPanel* PropertiesPanel;

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxDlgColor.
-----------------------------------------------------------------------------*/

class WxDlgColor : public wxColourDialog
{
public:
	WxDlgColor();
	virtual ~WxDlgColor();

	void LoadColorData( wxColourData* InData );
	void SaveColorData( wxColourData* InData );

	bool Create( wxWindow* InParent, wxColourData* InData );

	int ShowModal();

  	void OnOK( wxCommandEvent& event );

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxDlgLoadErrors
-----------------------------------------------------------------------------*/

class WxDlgLoadErrors : public wxDialog
{
public:
	WxDlgLoadErrors( wxWindow* InParent );

	wxListBox *PackageList, *ObjectList;

	void Update();
};

/**
* Helper method for popping up a directory dialog for the user.  OutDirectory will be 
* set to the empty string if the user did not select the OK button.
*
* @param	OutDirectory	[out] The resulting path.
* @param	Message			A message to display in the directory dialog.
* @param	DefaultPath		An optional default path.
* @return					TRUE if the user selected the OK button, FALSE otherwise.
*/
UBOOL PromptUserForDirectory(FString& OutDirectory, const FName& Message, const FName& DefaultPath = NAME_None);

#endif // __DIALOGS_H__
