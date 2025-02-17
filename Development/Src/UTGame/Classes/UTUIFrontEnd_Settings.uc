/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Settings scene for UT3.
 */
class UTUIFrontEnd_Settings extends UTUIFrontEnd_BasicMenu;

// Menu options
const SETTINGS_OPTION_VIDEO = 0;
const SETTINGS_OPTION_AUDIO = 1;
const SETTINGS_OPTION_INPUT = 2;
const SETTINGS_OPTION_NETWORK = 3;
const SETTINGS_OPTION_WEAPONS = 4;
const SETTINGS_OPTION_HUD = 5;
const SETTINGS_OPTION_CREDITS = 6;
const SETTINGS_OPTION_INSTALL = 7;

/** Reference to credits screen. */
var string	CreditsScene;

/** Reference to the actual scene that holds all of the settings panels. */
var string SettingsPanelsScene;

/** Index of the tabpage to show after the settings panels scene is opened. */
var transient int SelectedPage;

/** TRUE if the value for any options in the scene have changed */
var	private	bool	bDirty;

event SceneActivated(bool bInitialActivation)
{
	Super.SceneActivated(bInitialActivation);

	if (bInitialActivation)
	{
		SelectedPage = 0;
		OpenSceneByName(SettingsPanelsScene, false, OnPanelsSceneOpened);
	}
	else
	{
		ConditionalSaveProfile();
	}
}

final function MarkDirty( optional bool bIsDirty=true )
{
	bDirty = bIsDirty;
}

final function bool IsDirty()
{
	return bDirty;
}

/**
 * Executes a action based on the currently selected menu item.
 */
function OnSelectItem(int PlayerIndex=0)
{
	local int SelectedItem;
	SelectedItem = MenuList.GetCurrentItem();

	switch(SelectedItem)
	{
	case SETTINGS_OPTION_VIDEO:
	case SETTINGS_OPTION_AUDIO:
	case SETTINGS_OPTION_INPUT:
	case SETTINGS_OPTION_NETWORK:
	case SETTINGS_OPTION_WEAPONS:
	case SETTINGS_OPTION_HUD:
		SelectedPage = SelectedItem-SETTINGS_OPTION_VIDEO;
		OpenSceneByName(SettingsPanelsScene, false, OnPanelsSceneOpened);
		break;

	case SETTINGS_OPTION_CREDITS:
		OpenSceneByName(CreditsScene);
		break;

	case SETTINGS_OPTION_INSTALL:
		InstallPS3();
		break;
	}
}

/** Callback for when the settings scene has opened after its intro animation. */
function OnPanelsSceneOpened(UIScene OpenedScene, bool bInitialActivation)
{
	local UTUIFrontEnd_SettingsPanels PanelsSceneInst;

	if ( bInitialActivation )
	{
		PanelsSceneInst = UTUIFrontEnd_SettingsPanels(OpenedScene);
		if ( PanelsSceneInst != None )
		{
			PanelsSceneInst.OnNotifyOptionChanged = OnSettingValueChanged;
			PanelsSceneInst.OnMarkProfileDirty = MarkDirty;
			PanelsSceneInst.TabControl.ActivatePage(PanelsSceneInst.TabControl.GetPageAtIndex(SelectedPage), GetBestPlayerIndex());
		}
	}
}

/** Handler for the OnOptionChanged delegate of each panel; called when the user changes the value for an option in any panel */
function OnSettingValueChanged(UIScreenObject InObject, name OptionName, int PlayerIndex)
{
	local UIDataStorePublisher Publisher;
	local array<UIDataStore> Unused;

	Publisher = UIDataStorePublisher(InObject);
	if ( Publisher != None )
	{
		bDirty = Publisher.SaveSubscriberValue(Unused) || bDirty;
	}
}

function MidGameMenuSetup()
{
	// Remove any special options here.
}

function UILabel GetTitleLabel()
{
	local UILabel T;

	T = Super.GetTitleLabel();
	if ( T == none )
	{
		T = UILabel(FindChild('lblTitle',true));
	}
	return t;
}

/** override this to save the profile if the user changed any settings */
function OnBack()
{
	ConditionalSaveProfile();
}

/**
 * Saves and reloads the player profile if any values were changed.
 */
function ConditionalSaveProfile()
{
	local UIDataStore_OnlinePlayerData	PlayerDataStore;
	local UTUIScene_SaveProfile SaveProfileScene;
	local UTGameUISceneClient UTSceneClient;

	if ( bDirty )
	{
		bDirty = false;

		ConsoleCommand("RetrieveSettingsFromProfile");

		// Save profile
		PlayerDataStore = UIDataStore_OnlinePlayerData(FindDataStore('OnlinePlayerData', GetPlayerOwner()));
		if(PlayerDataStore != none)
		{
			UTSceneClient = UTGameUISceneClient(GetSceneClient());
			if ( UTSceneClient != None )
			{
				`Log(`location@"- Saving player profile.");
				SaveProfileScene = UTSceneClient.ShowSaveProfileScene(GetUTPlayerOwner());
				if(SaveProfileScene != None)
				{
					SaveProfileScene.OnSaveFinished = OnSaveProfileCompleted;
				}
				else
				{
					OnSaveProfileCompleted();
				}
			}
		}
		else
		{
			`Log(`location@" - Unable to locate OnlinePlayerData datastore for saving out profile.");
			OnSaveProfileCompleted();
		}
	}
	else
	{
		CloseScene(Self);
	}
}

/** Callback for when the profile save has completed. */
function OnSaveProfileCompleted()
{
	CloseScene(self);
}


defaultproperties
{
	// actually, we do save scene values on close, but only if we're marked dirty
	bSaveSceneValuesOnClose=false

	CreditsScene="UI_Scenes_FrontEnd.Scenes.Credits"
	SettingsPanelsScene="UI_Scenes_ChrisBLayout.Scenes.SettingsPanels"
}
