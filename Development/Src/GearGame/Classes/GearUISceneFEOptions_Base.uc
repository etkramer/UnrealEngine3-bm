/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUISceneFEOptions_Base extends GearUISceneFrontEnd_Base
	abstract
	Config(UI);


/************************************************************************/
/* Consts, structs, enums, etc.                                         */
/************************************************************************/

/** Storage place for the data needed to handle the settings */
struct GearSettingsData
{
	/** The ProfileSettingId of the setting in the profile */
	var int ProfileSettingId;
	/** Name of the widget this setting is using */
	var name WidgetName;
	/** Reference to the widget for this setting */
	var UIScreenObject SettingWidget;
	/** Whether this is using a raw value in the profile or not */
	var bool bUsesRawValue;
	/** Indicates that this option can only be set by the primary player */
	var bool bIsGlobalValue;
	/** indicates that this setting should be hidden if mature language isn't allowed */
	var bool bRequiresMatureLanguageSupport;
};


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** The setting data we'll need for the scene */
var transient array<GearSettingsData> SettingsData;

/** Cache of profile reference */
var transient GearProfileSettings MyProfile;

/** Weapon button bar */
var transient UICalloutButtonPanel SettingsButtonBar;

/** Whether we need to save the profile or not */
var transient bool bSaveProfile;

/** Whether we've reset defaults or not (needed since we must set the profile in this case and can't properly test if things changed */
var transient bool bResettedDefaults;


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/**
* Called after this screen object's children have been initialized
* Overloaded to set the deactivated callback
*/
event PostInitialize()
{
	local GearPC MyGearPC;

	// Cache the profile
	MyGearPC = GetGearPlayerOwner();
	if ( MyGearPC != None )
	{
		MyProfile = MyGearPC.ProfileSettings;
	}

	// Initialize references to widgets
	InitializeWidgetReferences();

	// Setup any callbacks need, such as button bars
	SetupCallbacks();

	Super.PostInitialize();
}

/** Initialize references to widgets */
function InitializeWidgetReferences()
{
	local int Idx, PlayerOwnerIndex;
	local bool bIsPrimaryPlayer;
	local GearUIInteraction GearUIController;
	local UIObject Parent, OwnerPanel, PreviousWidget, NextWidget;

	PlayerOwnerIndex = GetPlayerOwnerIndex();
	bIsPrimaryPlayer = PlayerOwnerIndex == 0;
	GearUIController = GearUIInteraction(GetCurrentUIController());

	// Initialize the settings button bar
	SettingsButtonBar = UICalloutButtonPanel(FindChild('pnlOptionButtonBar', true));
	PreviousWidget = None;

	// Initialize the setting specific widgets
	for ( Idx = 0; Idx < SettingsData.length; Idx ++ )
	{
		SettingsData[Idx].SettingWidget = FindChild(SettingsData[Idx].WidgetName, true);
		if ( SettingsData[Idx].SettingWidget != None && !IsEditor() )
		{
			if ( SettingsData[Idx].bRequiresMatureLanguageSupport
			&&	!GearUIController.IsMatureLanguageSupported() )
			{
				Parent = UIObject(SettingsData[Idx].SettingWidget.GetParent());
				OwnerPanel = UIObject(Parent.GetParent());

				if ( SettingsButtonBar != None && SettingsButtonBar.IsDockedTo(Parent, UIFACE_Top) )
				{
					SettingsButtonBar.SetDockTarget(UIFACE_Top, PreviousWidget, UIFACE_Bottom);
				}

				if ( Idx < SettingsData.Length - 1 && SettingsData[Idx+1].SettingWidget != None )
				{
					NextWidget = UIObject(SettingsData[Idx+1].SettingWidget.GetParent());
				}
				else if ( Idx == SettingsData.Length - 1 && SettingsData.Length > 1 && SettingsData[0].SettingWidget != None )
				{
					NextWidget = UIObject(SettingsData[0].SettingWidget.GetParent());
				}
				if ( PreviousWidget != None )
				{
					PreviousWidget.SetForcedNavigationTarget(UIFACE_Bottom, NextWidget, true);
				}

				if ( NextWidget != None )
				{
					NextWidget.SetForcedNavigationTarget(UIFACE_Top, PreviousWidget, true);
				}

				SettingsData.Remove(Idx--,1);
				OwnerPanel.RemoveChild(Parent);
			}
			else if ( !bIsPrimaryPlayer && SettingsData[Idx].bIsGlobalValue )
			{
				SettingsData[Idx].SettingWidget.SetEnabled(false, PlayerOwnerIndex);
			}
			else
			{
				PreviousWidget = UIObject(SettingsData[Idx].SettingWidget.GetParent());
			}
		}
	}
}

/** Assigns delegates in important child widgets to functions in this scene class */
function SetupCallbacks()
{
	if ( SettingsButtonBar != None )
	{
		SettingsButtonBar.SetButtonCallback( 'Defaults', OnSetDefaultsClicked );
		SettingsButtonBar.SetButtonCallback( 'GenericBack', OnBackClicked );
	}
}

/** Sets a flag in the main options screen to save the profile when it closes */
function MarkProfileDirty()
{
	local UIScene MainOptionScene;
	local GameUISceneClient GameSceneClient;

	GameSceneClient = GetSceneClient();
	if (GameSceneClient != none)
	{
		MainOptionScene = GameSceneClient.FindSceneByTag('UI_FETran_Options', GetPlayerOwner());
		if (MainOptionScene != none)
		{
			GearUISceneFETran_Options(MainOptionScene).bOptionsNeedSaved = true;
		}
	}
}

/** Called when the scene is closed so we can stop the music */
function OnSceneDeactivatedCallback( UIScene DeactivatedScene )
{
	if ( bSaveProfile )
	{
		MarkProfileDirty();
	}
}

/** Perform any actions that need to take place before the determining if the profile should be saved or not */
function PreProfileSaveCheck( int PlayerIndex );

/** Accepts the changes and closes the scene */
function bool OnBackClicked(UIScreenObject EventObject, int PlayerIndex)
{
	if ( SettingChangesHaveBeenMade() )
	{
		PreProfileSaveCheck( PlayerIndex );
		bSaveProfile = true;
	}
	else
	{
		bSaveProfile = false;
	}

	CloseScene( self );

	return true;
}

/** Ask if they are sure */
function bool OnSetDefaultsClicked(UIScreenObject EventObject, int PlayerIndex)
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	ButtonAliases.AddItem('GenericCancel');
	ButtonAliases.AddItem('ResetAccept');
	GameSceneClient = GetSceneClient();
	GameSceneClient.ShowUIMessage(
		'ConfirmTutorialReset',
		"<Strings:GearGameUI.MessageBoxStrings.ResetOptions_Title>",
		"<Strings:GearGameUI.MessageBoxStrings.ResetOptions_Message>",
		"<Strings:GearGameUI.MessageBoxStrings.ResetOptions_Question>",
		ButtonAliases,
		OnResetOptions_Confirm,
		GetPlayerOwner(PlayerIndex) );

	return true;
}

/** Callback from asking if the player wants to reset tutorials */
function bool OnResetOptions_Confirm( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	if (SelectedInputAlias == 'ResetAccept')
	{
		SetAllSettingsToDefault(PlayerIndex);
	}
	return true;
}

/** Set all of the weapon settings back to their defaults (NOTE: Any raw profile values must be handled by hand in the child scene) */
function SetAllSettingsToDefault( int PlayerIndex )
{
	local int Idx, ProfileValue, ProfileIndex;
	local float ProfileFloatValue;
	local UIScreenObject SettingWidget;
	local UIOptionList OptionList;
	local UISlider SettingSlider;
	local array<UIDataStore> Unused;

	if ( MyProfile != None )
	{
		for ( Idx = 0; Idx < SettingsData.length; Idx ++ )
		{
			SettingWidget = SettingsData[Idx].SettingWidget;
			// Let the scene handle any raw values
			if ( SettingWidget != None && !SettingsData[Idx].bUsesRawValue )
			{
				OptionList = UIOptionList(SettingsData[Idx].SettingWidget);
				if ( OptionList != None )
				{
					if (MyProfile.GetProfileSettingDefaultId(SettingsData[Idx].ProfileSettingId, ProfileValue, ProfileIndex)
					&&	OptionList.GetCurrentIndex() != ProfileIndex )
					{
						OptionList.SetCurrentIndex( ProfileIndex );

						// publish to profile
						OptionList.SaveSubscriberValue(Unused);
						OptionList.OnValueChanged(OptionList, PlayerIndex);
					}
				}
				// Assumes UISlider
				else
				{
					SettingSlider = UISlider(SettingsData[Idx].SettingWidget);
					if ( SettingSlider != None )
					{
						if (MyProfile.GetProfileSettingDefaultFloat(SettingsData[Idx].ProfileSettingId, ProfileFloatValue)
						&&	SettingSlider.GetValue() != ProfileFloatValue )
						{
							SettingSlider.SetValue( ProfileFloatValue );
							SettingSlider.SaveSubscriberValue(Unused);
							SettingSlider.OnValueChanged(SettingSlider, PlayerIndex);
						}

						`log(`location @ `showvar(Idx) @ `showvar(SettingSlider) @ `showvar(ProfileFloatValue) @ `showvar(SettingSlider.GetValue()));
					}
				}
			}
		}

		// Set the data to the profile
		bResettedDefaults = true;
	}
}

/** Returns whether any changes were made or not (NOTE: Any raw profile values must be handled by hand in the child scene) */
function bool SettingChangesHaveBeenMade()
{
	local int Idx, ProfileValue, ListIndex;
	local float ProfileFloatValue;
	local UIScreenObject SettingWidget;
	local UIOptionList OptionList;
	local UISlider SettingSlider;

	if (bResettedDefaults)
	{
		return true;
	}

	if ( MyProfile != None )
	{
		for ( Idx = 0; Idx < SettingsData.length; Idx ++ )
		{
			SettingWidget = SettingsData[Idx].SettingWidget;
			if ( SettingWidget != None && !SettingsData[Idx].bUsesRawValue )
			{
				OptionList = UIOptionList(SettingsData[Idx].SettingWidget);
				if ( OptionList != None )
				{
					if ( MyProfile.GetProfileSettingValueId(SettingsData[Idx].ProfileSettingId, ProfileValue, ListIndex)
					&&	OptionList.GetCurrentIndex() != ListIndex )
					{
						return true;
					}
				}
				// Assumes UISlider
				else
				{
					SettingSlider = UISlider(SettingsData[Idx].SettingWidget);
					if ( SettingSlider != None )
					{
						if (MyProfile.GetProfileSettingValueFloat(SettingsData[Idx].ProfileSettingId, ProfileFloatValue)
						&&	SettingSlider.GetValue() != ProfileFloatValue )
						{
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

/**
 * Find the index into the SettingData array for the specified widget.
 */
final function int FindSettingIndexOfWidget( UIScreenObject TargetWidget )
{
	local int Idx, Result;

	Result = INDEX_NONE;
	for ( Idx = 0; Idx < SettingsData.Length; Idx++ )
	{
		if ( SettingsData[Idx].SettingWidget == TargetWidget )
		{
			Result = Idx;
			break;
		}
	}

	return Result;
}

/** Returns the index of the SettingsData of the setting that is currently focused */
final function int GetSettingIndexOfFocusedWidget()
{
	local int Idx;

	for ( Idx = 0; Idx < SettingsData.length; Idx ++ )
	{
		if ( SettingsData[Idx].SettingWidget != None && SettingsData[Idx].SettingWidget.IsFocused() )
		{
			return Idx;
		}
	}

	return -1;
}

/** Returns the index of the SettingsData of the setting that matching the ProfileId */
final function int GetSettingIndexUsingProfileId( int ProfileId )
{
	local int Idx;

	for ( Idx = 0; Idx < SettingsData.length; Idx ++ )
	{
		if ( SettingsData[Idx].ProfileSettingId == ProfileId )
		{
			return Idx;
		}
	}

	return -1;
}

/**
 * Handler for this scene's GetSceneInputModeOverride delegate.  Forces INPUTMODE_Locked if in the front-end.
 */
function EScreenInputMode OverrideSceneInputMode()
{
	local EScreenInputMode Result;

	Result = GetSceneInputMode(true);
	if ( class'WorldInfo'.static.IsMenuLevel() )
	{
		Result = INPUTMODE_Locked;
	}

	return Result;
}

defaultproperties
{
	OnSceneDeactivated=OnSceneDeactivatedCallback
	GetSceneInputModeOverride=OverrideSceneInputMode
	bMenuLevelRestoresScene=true
	SceneRenderMode=SPLITRENDER_PlayerOwner
	SceneInputMode=INPUTMODE_MatchingOnly
	bRenderParentScenes=true
}
