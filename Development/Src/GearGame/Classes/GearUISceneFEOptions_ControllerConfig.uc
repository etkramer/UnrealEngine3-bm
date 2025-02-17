/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUISceneFEOptions_ControllerConfig extends GearUISceneFEOptions_Base
	Config(UI);


/** Scene to open for Controller Display */
var	transient GearUISceneFE_ControllerDisplay ControllerDisplayScene;

/** Assigns delegates in important child widgets to functions in this scene class */
function SetupCallbacks()
{
	Super.SetupCallbacks();

	if ( SettingsButtonBar != None )
	{
		SettingsButtonBar.SetButtonCallback('Controller', OnDisplayController);
		SettingsButtonBar.EnableButton('Controller', GetPlayerOwnerIndex(), class'WorldInfo'.static.IsMenuLevel() || class'UIInteraction'.static.GetPlayerCount() == 1, true);
	}
}

/** Open the Controller screen */
function bool OnDisplayController(UIScreenObject EventObject, int PlayerIndex)
{
	SaveSceneDataValues();
	OpenScene(ControllerDisplayScene, GetPlayerOwner());
	return true;
}


/**
 * Update the hint text and related widgets. Do this with a delay depending on whether the label is stationaty.
 * If the hint text moves around to follow the currently selected widget use a 1 second delay before switching to a new label.
 */
function UpdateDescriptionLabel( string StringToBind, UIScreenObject WidgetToDockTo, EGearDescriptionDockType GearDockType, float DockPadding )
{
	if ( class'WorldInfo'.static.IsMenuLevel() || class'UIInteraction'.static.GetPlayerCount() == 1 )
	{
		Super.UpdateDescriptionLabel(StringToBind, WidgetToDockTo, GearDockType, DockPadding);
	}
}

defaultproperties
{
	ControllerDisplayScene=GearUISceneFE_ControllerDisplay'UI_Scenes_Common.UI_ControllerDisplay'
	SettingsData(0)=(ProfileSettingId=PSI_YInversion,WidgetName="olistInvertY")
	SettingsData(1)=(ProfileSettingId=TurnInversion,WidgetName="olistInvertX")
	SettingsData(2)=(ProfileSettingId=PSI_ControllerSensitivity,WidgetName="olistSensitivity")
	SettingsData(3)=(ProfileSettingId=TargetSensitivity,WidgetName="olistTargetSensitivity")
	SettingsData(4)=(ProfileSettingId=ZoomSensitivity,WidgetName="olistZoomSensitivity")
	SettingsData(5)=(ProfileSettingId=PSI_ControllerVibration,WidgetName="olistVibration")
	SettingsData(6)=(ProfileSettingId=GearStickConfig,WidgetName="olistSticks")
	SettingsData(7)=(ProfileSettingId=GearTriggerConfig,WidgetName="olistTriggers")
	SettingsData(8)=(ProfileSettingId=GearControlScheme,WidgetName="olistControlScheme")
}
