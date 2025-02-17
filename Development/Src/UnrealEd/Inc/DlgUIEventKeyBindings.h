/*=============================================================================
	DlgUIEventKeyBindings.h : This is a dialog that lets the user set defaults for which keys trigger built in UI Events for each widget type.  
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DLGUIEVENTKEYBINDINGS_H__
#define __DLGUIEVENTKEYBINDINGS_H__

/*-----------------------------------------------------------------------------
WxDlgGenericStringEntry
-----------------------------------------------------------------------------*/

/**
 * A custom input key handler class which is used to intercept key and mouse input events
 * from the Choose Input Key dialog's list box.
 */
class WxRawInputKeyHandler : public wxEvtHandler
{
public:
	/** Constructor */
	WxRawInputKeyHandler( class WxDlgChooseInputKey* inOwnerDialog );

private:

	/** Reference to the dialog which contains the list */
	class WxDlgChooseInputKey* InputKeyDialog;

	/** called when the user presses a key while the list is active */
	void OnInputKeyPressed( wxKeyEvent& Event );

	/**
	 * Called when the user presses a mouse button or scrolls the mouse wheel while
	 * the list is active
	 */
	void OnMouseInput( wxMouseEvent& Event );

	DECLARE_EVENT_TABLE()
};

/**
 * A dialog which allows the user to choose an input key either by pressing the desired key
 * on the keyboard, or holding a modifier key (ctrl,alt,shift) and clicking with the mouse.
 */
class WxDlgChooseInputKey : public wxDialog
{
	friend class WxRawInputKeyHandler;

public:
	/** Ctor/dtor */
	WxDlgChooseInputKey( wxWindow* inParent );
	virtual ~WxDlgChooseInputKey();

	/** Returns the key selected by the user */
	FName GetSelectedKey() const;

	/** show the dialog */
	int ShowModal( const TArray<FName>& AvailableKeys );

private:
	/** called when the user selects an item in the list */
	void OnInputKeySelected( wxCommandEvent& Event );

	/** called when the user presses a key while the list is active */
	void OnInputKeyPressed( wxKeyEvent& Event );

	/** Called when the user clicks the mouse on the list or uses the mouse wheel */
	void OnMouseInput( wxMouseEvent& Event );

	/**
	 * Determines whether the specified code corresponds to a key in the list
	 * and selects the appropriate element if found.
	 *
	 * @param	KeyCode		a windows virtual input key code
	 *
	 * @return	true if the specified code maps to a key which can be bound to an input alias
	 */
	bool InterceptInputKey( WORD KeyCode );

	/** the list of available keys */
	wxListBox* KeyList;

	/** the currently selected key */
	FName SelectedKey;

	/** the input key handler which routes input events from the list to the dialog */
	WxRawInputKeyHandler* ListInputKeyHandler;

	DECLARE_EVENT_TABLE()
};

/**
 * This is a dialog that lets the user set defaults for which keys trigger built in UI Events for each widget type.  
 */
class WxDlgUIEventKeyBindings : public wxDialog, public FSerializableObject
{
public:

	/** Enums for the fields of the listview. */
	enum EFields
	{
		Field_Key,
		Field_Count // Must be last element.
	};

	enum EBoundKeyListColumnHeaders
	{
		COLHEADER_KeyName,
		COLHEADER_StateNames,
		COLHEADER_Modifiers,
	};

	/** Options for the sorting process, tells whether to sort ascending or descending and which column to sort by. */
	struct FSortOptions
	{
		UBOOL bSortAscending;
		INT Column;
	};

	/**
	 * Stores information about the state and modifier flags for a single input key
	 */
	struct FInputKeyMappingData
	{
		/** the name of the input alias associated with this key */
		FName	InputAliasName;

		/** the state that the associated input key is mapped for */
		UClass*	LinkedState;

		/** the modifier flags set for the associated input key mapping */
		BYTE	ModifierFlags;

		FInputKeyMappingData( FName InAliasName, UClass* InState, BYTE InFlags )
		: InputAliasName(InAliasName), LinkedState(InState), ModifierFlags(InFlags)
		{}
		FInputKeyMappingData( const FInputKeyMappingData& Other )
		: InputAliasName(Other.InputAliasName), LinkedState(Other.LinkedState), ModifierFlags(Other.ModifierFlags)
		{}
		/** Comparison operators */
		FORCEINLINE UBOOL operator==(  const FInputKeyMappingData& Other ) const
		{
			return InputAliasName == Other.InputAliasName && LinkedState == Other.LinkedState && ModifierFlags == Other.ModifierFlags;
		}
		FORCEINLINE UBOOL operator!=(  const FInputKeyMappingData& Other ) const
		{
			return InputAliasName != Other.InputAliasName || LinkedState != Other.LinkedState || ModifierFlags != Other.ModifierFlags;
		}
		friend DWORD GetTypeHash( const FInputKeyMappingData& Data )
		{
			return GetTypeHash(Data.InputAliasName) + PointerHash(Data.LinkedState);
		}
	};

	struct FStateAliasPair
	{
		FName	AliasName;
		UClass* AliasState;

		FStateAliasPair( FName InAliasName, UClass* InState )
		: AliasName(InAliasName), AliasState(InState)
		{
		}
		FStateAliasPair( const FStateAliasPair& Other )
		: AliasName(Other.AliasName), AliasState(Other.AliasState)
		{}

		/** Comparison operators */
		FORCEINLINE UBOOL operator==(  const FStateAliasPair& Other ) const
		{
			return AliasName == Other.AliasName && AliasState == Other.AliasState;
		}
		FORCEINLINE UBOOL operator!=(  const FStateAliasPair& Other ) const
		{
			return AliasName != Other.AliasName || AliasState != Other.AliasState;
		}
		friend DWORD GetTypeHash( const FStateAliasPair& Pair )
		{
			return GetTypeHash(Pair.AliasName) + PointerHash(Pair.AliasState);
		}
	};

	WxDlgUIEventKeyBindings();
	WxDlgUIEventKeyBindings(wxWindow* InParent);

	UBOOL Create( wxWindow* InParent );
	void CreateControls();

	virtual ~WxDlgUIEventKeyBindings();

	/** 
	 * Callback for the Serialize function of FSerializableObject. 
	 *
	 * @param	Ar	Archive to serialize to.
	 */
	virtual void Serialize(FArchive& Ar);

protected:

	void Init();

	/**
	 * Loads the window position and other options for the dialog.
	 */
	void LoadDialogOptions();

	/**
	 * Saves the window position and other options for the dialog.
	 */
	void SaveDialogOptions() const;

	/** Builds a list of available input keys. */
	void BuildKeyList();

	/**
	 * Returns the widget state alias struct given the widget class and state class to use for the search.
	 *
	 * @param	WidgetClass		Widget class to use for the search.
	 * @param	StateClass		The state to we are looking for.
	 *
	 * @return	A pointer to the UIInputAliasStateMap for the specified widget and state classes.
 	 */
	FUIInputAliasStateMap* GetStateMapFromClass(UClass* WidgetClass, const UClass* StateClass);

	/**
	 * Returns the action alias map for the specified state using the selected widget class and event alias.
	 *
	 * @param StateClass	The state class to look in for the selected event alias struct.
	 * @return A pointer to the inputactionalias struct for the state specified.
	 */
	TArray<FUIInputActionAlias*> GetSelectedActionAliasMaps(const UClass* StateClass);

	/** Generates a bitmask of modifier flags based on the currently checked modifier checkboxes */
	BYTE GetCurrentModifierFlags() const;

	/**
	 * Returns the list of states which can be contain input alias bindings for the specified widget's class.  This is usually
	 * the classes in the widget's DefaultStates array, but might contain more states if the widget class is abstract.
	 */
	TArray<UClass*> GetSupportedStates( UUIScreenObject* WidgetCDO ) const;

	/**
	 * Searches all widget input alias bindings and searches for input keys/modifier combinations that are bound to more than one input
	 * alias for a single widget's state.  If duplicates are found, prompts the user for confirmation to continue.
	 *
	 * @return	TRUE if there were no duplicate bindings or the user chose to ignore them.  FALSE if duplicate bindings were detected and
	 *			the user wants to correct them.
	 */
	UBOOL VerifyUniqueBindings();

	/**
	 * Finds all aliases in a specific state that are bound to the specified key/modifier combination.
	 *
	 * @param	SearchWidget		the widget to search for matching bindings
	 * @param	SearchStateClass	the state to search for bound input keys
	 * @param	SearchKey			the input key / modifier combination to search for
	 * @param	out_Aliases			receives the list of existing mappings.
	 */
	UBOOL FindInputKeyMappings( class UUIScreenObject* SearchWidget, class UClass* SearchStateClass, const struct FRawInputKeyEventData& SearchKey, TArray<FName>& out_Aliases );

	/**
	 * Adds an input key to the the list of keys mapped for the currently selected input aliases in the 
	 * specified state for the selected widgets.
	 *
	 * @param	KeyName		the input key to add to the widget's map.
	 * @param	StateClass  The state class to add the key binding to.
	 * @param	bUseDefaultModifierFlags
	 *						if TRUE, the default modifier flag mask will be assigned to this new alias instead
	 *						of the currently checked ones; useful when adding an entirely new key.
	 */
	void AddKeyToStateMap(const FName& KeyName, const UClass* StateClass, UBOOL bUseDefaultModifierFlags=FALSE );

	/**
	 * Removes a key from all of the selected alias's input maps for all states of the currently selected widget.
	 *
	 * @param	KeyName		Key to remove from the widget's maps.
	 */
	void RemoveKeyFromStateMaps(FName &KeyName);

	/** Updates the ModifierFlags value for the currently selected widget/input alias/input key's UIInputActionAlias */
	void SetModifierBitmask( BYTE NewModifierFlagMask );

	/** Gets the list of keys mapped to the selected input aliases for the specified widget CDO */
	void GetWidgetReverseKeyMap( UUIScreenObject* WidgetCDO, TMap<FRawInputKeyEventData,FStateAliasPair>& out_MappedKeys );

	/** Simple wrapper for updating all of the selection arrays */
	void UpdateSelections();

	/** Retrieves the currently selected widgets */
	void UpdateSelectedWidgets();

	/** Updates the list of currently selected input aliases */
	void UpdateSelectedAliases();

	/** Adds all of the widgets available to the widget combobox. */
	void PopulateWidgetClassList();

	/** Populates the EventList with the input aliases supported by the selected widgets */
	void PopulateSupportedAliasList();

	/** Populates the StateList with the names of the UIStates supported by the selected widgets */
	void PopulateSupportedStateList();

	/** Populates the BoundInputKeysList with the input keys bound to the currently selected input aliases */
	void PopulateBoundInputKeys();

	/** Updates the checked state of the ListStates list's checkboxes */
	void UpdateStateChecklistValues();

	/** Updates the checked state of all modifier checkboxes according to the currently selected items */
	void UpdateModifierValues();

	/** Wrapper method which updates the values of the state list and modifier checkboxes */
	void UpdateOptionsPanel()
	{
		UpdateStateChecklistValues();
		UpdateModifierValues();
	}

	/** Regenerates the MappedInputKeyData cache */
	void RebuildBoundInputKeyDataCache();

	/** Updates the list of available keys */
	void UpdateAvailableKeys();

	/**
	 * Returns a pointer to the state class most appropriate to handle input events for the specified input key.  If not all widgets support
	 * the most appropriate state, returns NULL.
	 */
	UClass* GetRecommendedStateClass( const FName& SelectedKey );

	/**
	 * Called when the list of bound input keys is invalidated; clears the list and all other
	 * cached data related to the set of input keys bound to the selected aliases.
	 */
	void ClearBoundInputKeyData();

	/** 
	 * Generates a local version of the Unreal Key to UI Event Key mapping tables for editing. 
	 *
	 * @param	Widget	Widget to generate the mapping tables for.
	 */
	void GenerateEventArrays(UUIScreenObject* Widget);

	/** Sorts the list based on which column was clicked. */
	void OnClickListColumn(wxListEvent &Event);

	/**
	 * Enables / disables the state list and modifier checkboxes depending on whether a valid input key is
	 * selected.
	 */
	void OnUpdateOptionsPanel_UI( wxUpdateUIEvent& Event );

	/**
	 * Enables/disables the "New Key" button and input key list depending on whether any input aliases
	 * are selected
	 */
	void OnUpdateInputKeys_UI( wxUpdateUIEvent& Event );

	/** Called when the selected widget changes. */
	void OnWidgetClassSelected(wxCommandEvent& Event);

	/** Called when the user selects an event in the event list. */
	void OnAliasListSelected(wxCommandEvent &Event);

	/** called when the user selects an item in the bound keys list */
	void OnInputKeyList_Selected( wxListEvent& Event );
	
	/** Called when the user deselects an item in the bound keys list */
	void OnInputKeyList_Deselected( wxListEvent& Event );

	/** called when the user right-clicks on the ListBoundInputKeys list */
	void OnInputKeyList_RightClick( wxListEvent& Event );

	/** called when the user presses a key with the bound input key list focused */
	void OnInputKeyList_KeyDown( wxListEvent& Event );

	/** Called when the user checks/unchecks a state in the state list */
	void OnStateCheckToggled( wxCommandEvent& Event );

	/** called when on of the modifier checkboxes is checked/unchecked */
	void OnModifierChecked( wxCommandEvent& Event );

	/**
	 * Called when the user clicks the "New Key" button - shows a dialog which allows
	 * them to choose a key from the list of all available keys
	 */
	void OnNewKeyButton( wxCommandEvent& Event );

	/** Called when the user chooses to remove the current input key from the alias mappings */
	void OnRemoveInputKey( wxCommandEvent& Event );

	/** Called when the user double-clicks an item or presses enter with an item selected */
	void OnInputKeyList_ReplaceKey( wxListEvent& Event );

	/** Called when the user chooses a different value from the platform combo */
	void OnPlatformSelected( wxCommandEvent& Event );

	/** When the user clicks the OK button, we create/remove output links for the sequence object based on the state of the lists. */
	void OnOK(wxCommandEvent &Event);

	/**
	 * Returns the filename to the default input.ini file for the specified platform.
	 */
	FString GetDefaultIniFilename( const FString& PlatformName );

	/**
	 * Reads the input alias mappings from the default .ini for the specified platform and applies the new mappings to the UIInputConfiguration singleton.
	 *
	 * @param	PlatformName	the name of the platform (as returned from the FConsoleSupport::GetConsoleName object for that platform), or PC.
	 *
	 * @return	TRUE if the new values were successfully loaded and applied
	 */
	UBOOL LoadSpecifiedPlatformValues( const FString& PlatformName );

	/**
	 * Saves the input alias mappings to the default ini for the currently selected platform
	 *
	 * @param	PlatformName	the name of the platform (as returned from the FConsoleSupport::GetConsoleName object for that platform), or PC.
	 *
	 * @return	TRUE if the save was successful, FALSE if the file was read-only or couldn't be written for some reason
	 */
	UBOOL SaveSpecifiedPlatform( const FString& PlatformName );

	/**
	 * Saves the input alias mappings to the default ini for all platforms
	 *
	 * @return	TRUE if the all platforms were saved successfully, FALSE if any platforms failed
	 */
	UBOOL SaveAll();

	/**
	 * Displays the "Choose input key" dialog, then adds the selected key to the list of bound keys.
	 *
	 * @return	the key that the user selected, or NAME_None if the user didn't choose a key or the key couldn't be added to the input key list
	 */
	FName ShowInputKeySelectionDialog();

	wxSplitterWindow* MainSplitter;
	wxSplitterWindow* LeftSplitter;

	wxListBox* ListWidgets;
	wxListBox* ListAliases;
	wxListCtrl* ListBoundInputKeys;
	wxCheckListBox* ListStates;

	wxButton* btn_NewKey;
	WxComboBox* combo_Platform;

	/**
	 * Used as indexes for the array of modifier flag checkboxes.  Using an array
	 * makes it much easier to perform operations on the entire group; the order of
	 * the values in the enum is significant - they should occur in the same order as
	 * the values in the EInputKeyModifierFlag enum
	 */
	enum EModifierFlagIndex
	{
		INDEX_AltReq,
		INDEX_CtrlReq,
		INDEX_ShiftReq,
		INDEX_AltExcl,
		INDEX_CtrlExcl,
		INDEX_ShiftExcl,
		INDEX_MAX,
	};

	wxCheckBox* chk_ModifierFlag[INDEX_MAX];

	/** Reference to the currently selected widget. */
	TArray<UUIScreenObject*> SelectedWidgets;

	/** Name of the currently selected event alias. */
	TArray<FName> SelectedAliases;

	/** The input key currently selected in the list of bound input keys */
	FName SelectedInputKey;

	/**
	 * the name of the currently selected platform; needed because from within a "combo-box item selected" event, we won't know
	 * we otherwise couldn't know the previously selected platform in order to save the current values.
	 */
	FString CurrentPlatformName;

	/** A mapping of UUIObject to a struct containing a map of key bindings. */
	typedef TMap<UClass*, TArray<FUIInputAliasStateMap> > FClassToUIKeyMap;
	FClassToUIKeyMap WidgetLookupMap;

	/** List of available keys to use for binding. */
	TArray<FName> KeyList;

	/** List of keys which are not bound to an existing input alias */
	TArray<FName> AvailableKeys;

	/** Mapping of unreal key names to their event types. */
	TMap<FName, INT> KeyEventMap;

	/** A mapping of UIState class friendly name to the UClass* for that state */
	TMap<FString,UClass*> StateNameToClassMap;

	/** cache of the alias mapping information for each input key in the ListBoundInputKeys list */
	TMap<FName,FInputKeyMappingData> MappedInputKeyData;
	
	/** Options for sorting the lists, one for each list. */
	FSortOptions SortOptions;

	DECLARE_EVENT_TABLE()
};


#endif

