/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUISceneFEOptions_AV extends GearUISceneFEOptions_Base
	Config(UI);


/** The audio component used for playing a sample sound on volume change */
var transient AudioComponent MySoundAC;

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Initialize references to widgets */
function InitializeWidgetReferences()
{
	local int Idx;

	Super.InitializeWidgetReferences();

	// Initialize the setting specific widgets
	for ( Idx = 0; Idx < SettingsData.length; Idx ++ )
	{
		if ( SettingsData[Idx].SettingWidget != None )
		{
			UIObject(SettingsData[Idx].SettingWidget).OnValueChanged = OnSettingValueChanged;
		}
	}
}

/**
* Called when the value of this UIObject is changed.  Only called for widgets that contain data values.
*
* @param	Sender			the UIObject whose value changed
* @param	PlayerIndex		the index of the player that generated the call to this method; used as the PlayerIndex when activating
*							UIEvents; if not specified, the value of GetBestPlayerIndex() is used instead.
*/
function OnSettingValueChanged( UIObject Sender, int PlayerIndex )
{
	local GearPC MyGearPC;
	local int FocusIndex;
	local UISlider SettingSlider;

	MyGearPC = GetGearPlayerOwner(PlayerIndex);
	FocusIndex = GetSettingIndexOfFocusedWidget();
	if ( FocusIndex == INDEX_NONE )
	{
		// user clicked 'reset to defaults'
		FocusIndex = FindSettingIndexOfWidget(Sender);
	}

	// Update the audio and video in real-time as they change values
	if ( FocusIndex != -1 && MyGearPC != None )
	{
		switch ( SettingsData[FocusIndex].ProfileSettingId )
		{
			case MyProfile.const.MusicVolume:
				SettingSlider = UISlider(Sender);
				if ( SettingSlider != None )
				{
					MyGearPC.SetAudioGroupVolume( 'Music', SettingSlider.GetValue() );
				}
				break;

			case MyProfile.const.FxVolume:
				SettingSlider = UISlider(Sender);
				if ( SettingSlider != None )
				{
					MyGearPC.SetAudioGroupVolume( 'Effects', SettingSlider.GetValue() );
					StartSound(MyGearPC, "Interface_Audio.Firing.MenuCogRifleFireCue");
				}
				break;

			case MyProfile.const.DialogueVolume:
				SettingSlider = UISlider(Sender);
				if ( SettingSlider != None )
				{
					MyGearPC.SetAudioGroupVolume( 'Voice', SettingSlider.GetValue() );
					StartSound(MyGearPC, "Interface_Audio.Menu.MenuTestDialogCue");
				}
				break;
		}
	}
}


/**
 * Play a sound.
 *
 * @param MyGearPC	the Player Controller which owns the Audio Component
 * @param SoundCuePath	the path to which sound to play
 */
function StartSound(GearPC MyGearPC, string SoundCuePath)
{
	local SoundCue MusicCue;

	StopSound();

	MusicCue = SoundCue(DynamicLoadObject(SoundCuePath, class'SoundCue'));
	if (MusicCue != None)
	{
		MySoundAC = MyGearPC.CreateAudioComponent(MusicCue, false, true);
		if (MySoundAC != none)
		{
			MySoundAC.bAllowSpatialization = false;
			MySoundAC.bAutoDestroy = true;
			MySoundAC.Play();
		}
	}
}

/** Stop the sound */
function StopSound()
{
	if (MySoundAC != none)
	{
		MySoundAC.FadeOut(0.5f, 0.0f);
		MySoundAC = none;
	}
}

defaultproperties
{
	SettingsData(0)=(ProfileSettingId=MusicVolume,WidgetName="slideMusic",bIsGlobalValue=true)
	SettingsData(1)=(ProfileSettingId=FxVolume,WidgetName="slideSFX",bIsGlobalValue=true)
	SettingsData(2)=(ProfileSettingId=DialogueVolume,WidgetName="slideDialogue",bIsGlobalValue=true)
}
