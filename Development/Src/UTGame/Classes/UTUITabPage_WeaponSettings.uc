/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Tab page for a user's weapon settings.
 */

class UTUITabPage_WeaponSettings extends UTUITabPage_Options
	placeable;


/** Reference to the weapon preference scene. */
var string	WeaponPreferenceScene;

/** Post initialization event - Setup widget delegates.*/
event PostInitialize()
{
	Super.PostInitialize();

	// Set the button tab caption.
	SetDataStoreBinding("<Strings:UTGameUI.Settings.Weapons>");
}


/** Callback allowing the tabpage to setup the button bar for the current scene. */
function SetupButtonBar(UTUIButtonBar ButtonBar)
{
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.WeaponPreference>", OnButtonBar_WeaponPreference);
	ConditionallyAppendDefaultsButton(ButtonBar);
}


/**
* Callback for when the weapon preference scene is opened.
*/
function OnShowWeaponPreferenceScene_Opened(UIScene OpenedScene, bool bInitialActivation)
{
	if(OpenedScene != None)
	{
		UTUIFrontEnd_WeaponPreference(OpenedScene).MarkDirty = WeaponPreferencesChanged;
	}
}

/** Displays the key binding scene. */
function OnShowWeaponPreferenceScene()
{
	UTUIScene(GetScene()).OpenSceneByName(WeaponPreferenceScene, false, OnShowWeaponPreferenceScene_Opened);
}

function WeaponPreferencesChanged()
{
	// total hackery - kids, don't do this at home
	local UTUIFrontEnd_Settings SettingsScene;

	SettingsScene = UTUIFrontEnd_Settings(GetScene().GetPreviousScene());
	if ( SettingsScene != None )
	{
		SettingsScene.MarkDirty();
	}
}


function bool OnButtonBar_WeaponPreference(UIScreenObject InButton, int PlayerIndex)
{
	OnShowWeaponPreferenceScene();

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
		if(EventParms.InputKeyName=='XboxTypeS_Y')
		{
			OnShowWeaponPreferenceScene();
			bResult=true;
		}
	}

	return bResult;
}

defaultproperties
{
	bAllowResetToDefaults=true
	WeaponPreferenceScene="UI_Scenes_ChrisBLayout.Scenes.WeaponPreference"
}

