/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Tab page for a user's Audio settings.
 */

class UTUITabPage_AudioSettings extends UTUITabPage_Options
	placeable;

/** Sound cue to play when the user changes the SFX volume slider. */
var transient SoundCue		SFXVolumeCue;

/** Sound cue to play when the user changes the Voice volume slider. */
var transient SoundCue		VoiceVolumeCue;

/** Sound cue to play when the user changes the Ambiance volume slider. */
var transient SoundCue		AmbianceVolumeCue;

/** Sound cue to play when the user changes the Announcer volume slider. */
var transient SoundCue		AnnouncerVolumeCue;

/** Whether or not we have been fully initialized. */
var transient bool			bFullyInitialized;

/** Post initialization event - Setup widget delegates.*/
event PostInitialize()
{
	Super.PostInitialize();

	// Set the button tab caption.
	SetDataStoreBinding("<Strings:UTGameUI.Settings.Audio>");

	// Pull possible audio options
	if(!IsConsole())
	{
		// Audio Devices
		StringListDataStore.Empty('AudioDevices', true);
		StringListDataStore.AddStr('AudioDevices', "Generic Software");
		StringListDataStore.AddStr('AudioDevices', "Hardware");

		if(UTUIScene(GetScene()).GetCurrentAudioDevice()=="Generic Software")
		{
			StringListDataStore.SetCurrentValueIndex('AudioDevices', 0);
		}
		else
		{
			StringListDataStore.SetCurrentValueIndex('AudioDevices', 1);
		}
	}

	bFullyInitialized=true;
}


/** Pass through the option callback. */
function OnOptionList_OptionChanged(UIScreenObject InObject, name OptionName, int PlayerIndex)
{
	local UTUIScene UTScene;
	local bool bSaveAndRefresh;
	local array<UIDataStore> OutDataStores;

	bSaveAndRefresh = false;

	UTScene = UTUIScene(GetScene());

	Super.OnOptionList_OptionChanged(InObject, OptionName, PlayerIndex);

	if(UTScene != None && bFullyInitialized)
	{
		// Check to see if they changed a volume option.  If so, play a sound.
		if(OptionName=='SFXVolume')
		{
			bSaveAndRefresh=true;
			UTScene.PlaySound(SFXVolumeCue);
		}
		else if(OptionName=='MusicVolume')
		{
			bSaveAndRefresh=true;
		}
		else if(OptionName=='VoiceVolume')
		{
			bSaveAndRefresh=true;
			UTScene.PlaySound(VoiceVolumeCue);
		}
		else if(OptionName=='AmbianceVolume')
		{
			bSaveAndRefresh=true;
			UTScene.PlaySound(AmbianceVolumeCue);
		}
		else if(OptionName=='AnnouncerVolume')
		{
			bSaveAndRefresh=true;
			UTScene.PlaySound(AnnouncerVolumeCue);
		}
		else if(OptionName=='HardwareOpenAL')
		{
			bSaveAndRefresh=true;
			if(UICheckBox(InObject).IsChecked())
			{
				UTUIScene(GetScene()).SetAudioDeviceToUse("");
			}
			else
			{
				UTUIScene(GetScene()).SetAudioDeviceToUse("Generic Software");
			}
		}
	}

	if(bSaveAndRefresh)
	{
		UIDataStorePublisher(InObject).SaveSubscriberValue(OutDataStores);
		UTScene.GetUTPlayerOwner().UpdateVolumeAndBrightness();
	}

	Super.OnOptionList_OptionChanged(InObject, OptionName, PlayerIndex);
}


/** Callback allowing the tabpage to setup the button bar for the current scene. */
function SetupButtonBar(UTUIButtonBar ButtonBar)
{
	ConditionallyAppendDefaultsButton(ButtonBar);
}


/** Applies the current audio settings. */
function OnApplySettings()
{
	/* - Not used
	local string NewDevice;

	if(!IsConsole())
	{
		// Apply audio device change.
		if(StringListDataStore.GetCurrentValue('AudioDevices', NewDevice))
		{
			if(NewDevice != Localize("Generic","None","UTGameUI"))
			{
				`Log("Audio Device Changed: " $ NewDevice);

				UTUIScene(GetScene()).SetAudioDeviceToUse(NewDevice);
			}
		}
	}
	*/
}

/* - Not used
function bool OnButtonBar_Apply(UIScreenObject InButton, int PlayerIndex)
{
	OnApplySettings();

	return true;
}
*/

defaultproperties
{
	SFXVolumeCue=SoundCue'A_interface.Default.SFXVolume_Cue';
	VoiceVolumeCue=SoundCue'A_interface.Default.VoiceVolume_Cue';
	AmbianceVolumeCue=SoundCue'A_interface.Default.AmbientVolume_Cue';
	AnnouncerVolumeCue=SoundCue'A_interface.Default.AnnouncerVolume_Cue';
	bAllowResetToDefaults=true;
}