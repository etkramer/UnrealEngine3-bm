/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTUITabPage_MidGameSettings extends UTTabPage_MidGame
	dependson(UTUIScene_MessageBox);


var UTUIOptionList SettingsList;
var UILabel DescriptionLabel;

/** Sound cue to play when the user changes the SFX volume slider. */
var transient SoundCue		SFXVolumeCue;

/** Sound cue to play when the user changes the Voice volume slider. */
var transient SoundCue		VoiceVolumeCue;

/** Sound cue to play when the user changes the Ambiance volume slider. */
var transient SoundCue		AmbianceVolumeCue;

/** Sound cue to play when the user changes the Announcer volume slider. */
var transient SoundCue		AnnouncerVolumeCue;

event PostInitialize( )
{
	Super.PostInitialize();

	DescriptionLabel = UILabel( FindChild('lblDescription',true));
	SettingsList = UTUIOptionList( FindChild('OptionList',true));
	if (SettingsList!=none)
	{
		SettingsList.OnOptionChanged = OnOptionChanged;
		SettingsList.OnOptionFocused = OnOptionFocused;
	}

	OnInitialSceneUpdate=none;

}

/**
 * Sets a flag in the parent scene which indicates that the profile should be saved before the scene can be closed.
 */
function MarkProfileDirty( optional bool bIsDirty=true )
{
	// could also call NotifyValueChanged here and have the scene assign this tab page's OnValueChanged delegate to a function which sets that variable
	UTUIScene_MidGameMenu(GetScene()).bNeedsProfileSave = bIsDirty;
}

/** Delegate for when the user changes one of the options an option list. */
function OnOptionChanged(UIScreenObject InObject, name OptionName, int PlayerIndex)
{
	local UTUIScene Scene;
	local UIDataStorePublisher Publisher;
	local array<UIDataStore> Unused;
	local bool bRefresh;
	local UTPlayerController UTPC;

	Scene = UTUIScene(GetScene());
	if ( Scene != none )
	{
		UTPC = Scene.GetUTPlayerOwner();
		// save this change to the player's profile.
		Publisher = UIDataStorePublisher(InObject);
		if ( Publisher != None )
		{
			Publisher.SaveSubscriberValue(UnUsed);
		}
	}

	bRefresh = false;

	if(OptionName=='Brightness')
	{
		bRefresh=true;
	}
	else if(OptionName=='SFXVolume')
	{
		bRefresh=true;
	}
	else if(OptionName=='MusicVolume')
	{
		bRefresh=true;
	}
	else if(OptionName=='VoiceVolume')
	{
		bRefresh=true;
	}
	else if(OptionName=='AmbianceVolume')
	{
		bRefresh=true;
	}
	else if(OptionName=='AnnouncerVolume')
	{
		bRefresh=true;
	}

	if(bRefresh)
	{
		UTPC.UpdateVolumeAndBrightness();
	}

	MarkProfileDirty();
}

function OnOptionFocused(UIScreenObject InObject, UIDataProvider OptionProvider)
{
	local UTUIDataProvider_MenuOption MenuOptionProvider;

	MenuOptionProvider = UTUIDataProvider_MenuOption(OptionProvider);

	if(DescriptionLabel != None && MenuOptionProvider != None)
	{
		DescriptionLabel.SetDataStoreBinding(MenuOptionProvider.Description);
	}
}



function SetupButtonBar(UTUIButtonBar ButtonBar)
{

	Super.SetupButtonBar(ButtonBar);

	if ( !IsConsole() )
	{
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Advanced>",OnButtonBar_Advanced) ;
	}

	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.ResetToDefaults>",OnButtonBar_Defaults) ;
}

function bool OnButtonBar_Advanced(UIScreenObject InButton, int InPlayerIndex)
{
	AdvancedOptions();
	return true;
}

function bool OnButtonBar_Defaults(UIScreenObject InButton, int InPlayerIndex)
{
	local UTUIScene_MessageBox MB;
	local UTUIScene Scene;

    Scene = UTUIScene(GetScene());

	MB = Scene.GetMessageBoxScene();
	if (MB!=none)
	{
		MB.DisplayAcceptCancelBox("<Strings:UTGameUI.MidGameMenu.ResetToDefaultsWarning>","<Strings:UTGameUI.Campaign.Confirmation", MB_Selection);
	}
	return true;
}

function MB_Selection(UTUIScene_MessageBox MessageBox, int SelectedOption, int PlayerIndex)
{
	local int OptionIdx;
	local int DataPosition;
	local string ProfileOptionName;
	local UTUIDataProvider_MenuOption MenuProvider;
	local UTProfileSettings Profile;
	local UTUIScene UTScene;
	local int ProfileId;

	if ( SelectedOption == 0 && SettingsList.GeneratedObjects.Length > 0 )
	{
		SettingsList.SetFocus(none);
		UTScene = UTUIScene(GetScene());
		if(UTScene != None)
		{
			Profile = UTScene.GetPlayerProfile();

			if(Profile != None)
			{
				for(OptionIdx=0; OptionIdx<SettingsList.GeneratedObjects.length; OptionIdx++)
				{
					MenuProvider = UTUIDataProvider_MenuOption(SettingsList.GeneratedObjects[OptionIdx].OptionProvider);

					if(MenuProvider != None)
					{
						DataPosition = InStr(MenuProvider.DataStoreMarkup, "<OnlinePlayerData:ProfileData.");
						if(DataPosition != INDEX_NONE)
						{
							DataPosition += Len("<OnlinePlayerData:ProfileData.");
							ProfileOptionName=Mid(MenuProvider.DataStoreMarkup, DataPosition, Len(MenuProvider.DataStoreMarkup)-DataPosition-1);
							if(Profile.GetProfileSettingId(name(ProfileOptionName), ProfileId))
							{
								Profile.ResetToDefault(ProfileId);
								UIDataStoreSubscriber(SettingsList.GeneratedObjects[OptionIdx].OptionObj).RefreshSubscriberValue();
							}
						}
					}
				}
			}

			UTScene.ConsoleCommand("RetrieveSettingsFromProfile");
		}

		MarkProfileDirty();
	}

}

function bool HandleInputKey( const out InputEventParameters EventParms )
{
	if (EventParms.EventType == IE_Released)
	{
		if (EventParms.InputKeyName == 'XboxtypeS_X')
		{
			OnButtonBar_Advanced(none, EventParms.PlayerIndex);
			return true;
		}
		else if (EventParms.InputKeyName =='XboxtypeS_LeftTrigger')
		{
			OnButtonBar_Defaults(none, EventParms.PlayerIndex);
			return true;
		}
	}
	return true;
}





function AdjustSkin(bool bHudSkin)
{
	local UISkin Skin;

	// make sure we're using the right skin
	Skin = bHudSkin ? UISkin'UI_InGameHud.UTHUDSkin' : UISkin(DynamicLoadObject("UI_Skin_Derived.UTDerivedSkin",class'UISkin'));
	if ( Skin != none )
	{
		GetSceneClient().ChangeActiveSkin(Skin);
	}
}


function AdvancedOptions()
{
	local UTUIScene SceneOwner;
	local UIScene S;
	local UTUIFrontEnd_Settings SettingsScene;

	SceneOwner = UTUIScene(GetScene());
	if ( SceneOwner != none )
	{
		SceneOwner.FindChild('MidGameSafeRegion',true).SetVisibility(false);
		AdjustSkin(false);
		S = SceneOwner.OpenSceneByName(class'UTUIFrontEnd_MainMenu'.default.SettingsScene,true);
		if ( S!= none )
		{
			S.OnSceneDeactivated = AdvancedClosed;
			SettingsScene = UTUIFrontEnd_Settings(S);
			if ( SettingsScene != none )
			{
				SettingsScene.MidGameMenuSetup();
			}
		}
	}
}
function AdvancedClosed( UIScene DeactivatedScene )
{
	local UTUIScene SceneOwner;
	local UTUIFrontEnd_Settings SettingsScene;

	SceneOwner = UTUIScene(GetScene());
	if ( SceneOwner != none )
	{
		SceneOwner.FindChild('MidGameSafeRegion',true).SetVisibility(true);
		AdjustSkin(true);
		SettingsList.SetFocus(none);

		// the settings scene would have saved the profile if it was marked dirty.
		SettingsScene = UTUIFrontEnd_Settings(DeactivatedScene);
		if ( SettingsScene != None && SettingsScene.IsDirty() )
		{
			MarkProfileDirty(false);
		}
	}
}

defaultproperties
{
	SFXVolumeCue=SoundCue'A_interface.Default.SFXVolume_Cue';
	VoiceVolumeCue=SoundCue'A_interface.Default.VoiceVolume_Cue';
	AmbianceVolumeCue=SoundCue'A_interface.Default.AmbientVolume_Cue';
	AnnouncerVolumeCue=SoundCue'A_interface.Default.AnnouncerVolume_Cue';
}
