/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUISceneFEOptions_Video extends GearUISceneFEOptions_Base
	Config(UI);


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
	local UIOptionList SettingList;

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
			case MyProfile.const.GammaSetting:
				SettingSlider = UISlider(Sender);
				if ( SettingSlider != None )
				{
					MyGearPC.SetGamma( SettingSlider.GetValue() );
				}
				break;

			case MyProfile.const.TelevisionType:
				SettingList = UIOptionList(Sender);
				if ( SettingList != None )
				{
					MyGearPC.SetPostProcessValues( ETVTYPE(SettingList.GetCurrentIndex()) );
				}
				break;
		}
	}
}


defaultproperties
{
	SceneRenderMode=SPLITRENDER_Fullscreen
	SceneInputMode=INPUTMODE_Locked
	bRenderParentScenes=false

	SettingsData(0)=(ProfileSettingId=GammaSetting,WidgetName="slideBrightness",bIsGlobalValue=true)
	SettingsData(1)=(ProfileSettingId=TelevisionType,WidgetName="olistDisplay",bIsGlobalValue=true)
}
