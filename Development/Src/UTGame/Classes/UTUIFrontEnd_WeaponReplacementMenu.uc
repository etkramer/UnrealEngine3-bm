/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTUIFrontEnd_WeaponReplacementMenu extends UTUIFrontEnd
	dependson(UTMutator_WeaponReplacement);


/** Replacement info. */
var transient array<ReplacementInfo> WeaponsToReplace;
var transient array<ReplacementInfo> AmmoToReplace;

/** Reference to option list. */
var transient UTUIOptionList		OptionList;

/** Reference ot the options datastore. */
var transient UTUIDataStore_Options OptionsDataStore;

/** Reference to the string list datastore. */
var transient UTUIDataStore_StringList StringListDataStore;

/** Reference to the menu item datastore. */
var transient UTUIDataStore_MenuItems MenuItemDataStore;

/** List of possible weapon classes. */
var transient array<string>	WeaponClassNames;
var transient array<string>	WeaponFriendlyNames;
var transient array<UTUIDataProvider_Weapon> WeaponProviders;

event PostInitialize()
{
	Super.PostInitialize();

	WeaponsToReplace = class'UTMutator_WeaponReplacement'.default.WeaponsToReplace;
	AmmoToReplace = class'UTMutator_WeaponReplacement'.default.AmmoToReplace;

	OptionList = UTUIOptionList(FindChild('lstOptions', true));
	OptionList.OnAcceptOptions=OnAcceptOptions;

	// Get datastore references
	OptionsDataStore = UTUIDataStore_Options(FindDataStore('UTOptions'));
	StringListDataStore = UTUIDataStore_StringList(FindDataStore('UTStringList'));
	MenuItemDataStore = UTUIDataStore_MenuItems(FindDataStore('UTMenuItems'));
	
	BuildWeaponOptions();	
}

/** Actiated event for the scene. */
event SceneActivated(bool bInitialActivation)
{
	Super.SceneActivated(bInitialActivation);

	OptionList.SetFocus(none);
}

/** Callback for when the option list is accepted. */
function OnAcceptOptions(UIScreenObject InObj, int PlayerIndex)
{
	OnAccept();
}

/** Builds the weapons option lists. */
function BuildWeaponOptions()
{
	local int WeaponIdx;
	local int SelectedIdx;
	local int NameIdx;
	local array<UTUIResourceDataProvider> OutProviders;
	local array<UTUIResourceDataProvider> WeaponOptions;
	local bool bAddWeapon;
	local string ClassPath;

	// Build a list of weapons
	WeaponProviders.length = 0;
	WeaponClassNames.length = 0;
	WeaponFriendlyNames.length = 0;

	if(MenuItemDataStore.GetProviderSet('Weapons', OutProviders))
	{
		for(WeaponIdx=0; WeaponIdx<OutProviders.length; WeaponIdx++)
		{
			bAddWeapon=true;

			
			ClassPath = UTUIDataProvider_Weapon(OutProviders[WeaponIdx]).ClassName;

			// Remove content weapons from console
			if (class'WorldInfo'.static.IsConsoleBuild() && Left(ClassPath, 14) ~= "UTGameContent.")
			{
				bAddWeapon=false;
			}

			if(bAddWeapon)
			{
				WeaponProviders.AddItem(UTUIDataProvider_Weapon(OutProviders[WeaponIdx]));
				WeaponClassNames.AddItem(ClassPath);
				WeaponFriendlyNames.AddItem(UTUIDataProvider_Weapon(OutProviders[WeaponIdx]).FriendlyName);
			}
		}
	}

	// Generate weapon set if we havent already.
	OptionsDataStore.GetSet('ReplacementWeapons', WeaponOptions);

	if(WeaponOptions.length==0)
	{
		OptionsDataStore.AppendToSet('ReplacementWeapons', WeaponFriendlyNames.length);
		OptionsDataStore.GetSet('ReplacementWeapons', WeaponOptions);

		// Generate set of options and set of possible values
		for(WeaponIdx=0; WeaponIdx<WeaponFriendlyNames.length; WeaponIdx++)
		{
			ClassPath = GetClassFieldName(WeaponClassNames[WeaponIdx]);
			
			UTUIDataProvider_MenuOption(WeaponOptions[WeaponIdx]).DataStoreMarkup = "<UTStringList:"$ClassPath$">";
			UTUIDataProvider_MenuOption(WeaponOptions[WeaponIdx]).CustomFriendlyName = WeaponFriendlyNames[WeaponIdx];

			StringListDataStore.Empty(name(ClassPath));
			for(NameIdx=0; NameIdx<WeaponFriendlyNames.length; NameIdx++)
			{
				StringListDataStore.AddStr(name(ClassPath), WeaponFriendlyNames[NameIdx]);
			}

			StringListDataStore.SetCurrentValueIndex(name(ClassPath), WeaponIdx);
		}

		OptionList.bRegenOptions=true;
	}
	else
	{
		for(WeaponIdx=0; WeaponIdx<WeaponFriendlyNames.length; WeaponIdx++)
		{
			ClassPath = GetClassFieldName(WeaponClassNames[WeaponIdx]);
			StringListDataStore.SetCurrentValueIndex(name(ClassPath), WeaponIdx);
		}
	}

	// Set defaults
	for(WeaponIdx=0; WeaponIdx<WeaponsToReplace.length; WeaponIdx++)
	{
		for(SelectedIdx=0; SelectedIdx<WeaponClassNames.length; SelectedIdx++)
		{
			if(WeaponsToReplace[WeaponIdx].NewClassPath==WeaponClassNames[SelectedIdx])
			{
				ClassPath = GetClassFieldName(string(WeaponsToReplace[WeaponIdx].OldClassName));
				StringListDataStore.SetCurrentValueIndex(name(ClassPath), SelectedIdx);
			}
		}
	}

	OptionList.RefreshAllOptions();
}

/** @return string	Returns a fieldname given a weapon class name. */
function string GetClassFieldName(string ClassName)
{
	ClassName = Right(ClassName, Len(ClassName) - InStr(ClassName, ".") - 1);
	ClassName = Repl(ClassName, ".", "_");
	ClassName = "ReplaceWeapon_"$ClassName;
	return ClassName;
}

/** Sets up the scene's button bar. */
function SetupButtonBar()
{
	ButtonBar.Clear();
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Back>", OnButtonBar_Back);
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Accept>", OnButtonBar_Accept);
}

/** Callback for when the user wants to back out of this screen. */
function OnBack()
{
	CloseScene(self);
}

/** Callback for when the user accepts the changes. */
function OnAccept()
{
	local int WeaponIdx;
	local string ClassPath;
	local int SelectedIdx;

	WeaponsToReplace.length=0;
	AmmoToReplace.length=0;

	for(WeaponIdx=0; WeaponIdx<WeaponClassNames.length; WeaponIdx++)
	{
		ClassPath=GetClassFieldName(WeaponClassNames[WeaponIdx]);
		SelectedIdx=StringListDataStore.GetCurrentValueIndex(name(ClassPath));
		if(SelectedIdx!=WeaponIdx)
		{
			WeaponsToReplace.length=WeaponsToReplace.length+1;
			WeaponsToReplace[WeaponsToReplace.length-1].NewClassPath = WeaponProviders[SelectedIdx].ClassName;
			WeaponsToReplace[WeaponsToReplace.length-1].OldClassName = name(Right(WeaponProviders[WeaponIdx].ClassName, Len(WeaponProviders[WeaponIdx].ClassName) - InStr(WeaponProviders[WeaponIdx].ClassName, ".") - 1));
			
			AmmoToReplace.length=AmmoToReplace.length+1;
			AmmoToReplace[AmmoToReplace.length-1].NewClassPath = WeaponProviders[SelectedIdx].AmmoClassPath;
			AmmoToReplace[AmmoToReplace.length-1].OldClassName = name(Right(WeaponProviders[WeaponIdx].AmmoClassPath, Len(WeaponProviders[WeaponIdx].AmmoClassPath) - InStr(WeaponProviders[WeaponIdx].AmmoClassPath, ".") - 1));
		}
	}

	class'UTMutator_WeaponReplacement'.default.WeaponsToReplace = WeaponsToReplace;
	class'UTMutator_WeaponReplacement'.default.AmmoToReplace = AmmoToReplace;
	class'UTMutator_WeaponReplacement'.static.StaticSaveConfig();

	CloseScene(self);
}

/** Buttonbar Callbacks. */
function bool OnButtonBar_Back(UIScreenObject InButton, int InPlayerIndex)
{
	OnBack();

	return true;
}

function bool OnButtonBar_Accept(UIScreenObject InButton, int InPlayerIndex)
{
	OnAccept();

	return true;
}


/**
 * Provides a hook for unrealscript to respond to input using actual input key names (i.e. Left, Tab, etc.)
 *
 * Called when an input key event is received which this widget responds to and is in the correct state to process.  The
 * keys and states widgets receive input for is managed through the UI editor's key binding dialog (F8).
 *
 * This delegate is called BEFORE kismet is given a chance to process the input.
 *
 * @param	EventParms	information about the input event.
 *
 * @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
 */
function bool HandleInputKey( const out InputEventParameters EventParms )
{
	local bool bResult;

	bResult=false;

	if(EventParms.EventType==IE_Released)
	{
		if(EventParms.InputKeyName=='XboxTypeS_B' || EventParms.InputKeyName=='Escape')
		{
			OnBack();
			bResult=true;
		}
	}

	return bResult;
}

