/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUISceneFEOptions_Game extends GearUISceneFEOptions_Base
	Config(UI);

/** Whether the tutorials were reset or not */
var transient bool bTutorialsReset;

/** Override to add the reset tutorials button */
function SetupCallbacks()
{
	if ( SettingsButtonBar != None )
	{
		SettingsButtonBar.SetButtonCallback( 'ResetTutorials', OnResetTutorials );
	}
	Super.SetupCallbacks();
}

/** Ask if they are sure */
function bool OnResetTutorials(UIScreenObject EventObject, int PlayerIndex)
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	if (TutorialsCanBeReset())
	{
		ButtonAliases.AddItem('GenericCancel');
		ButtonAliases.AddItem('ResetAccept');
		GameSceneClient = GetSceneClient();
		GameSceneClient.ShowUIMessage(
			'ConfirmTutorialReset',
			"<Strings:GearGameUI.MessageBoxStrings.ResetTutorials_Title>",
			"<Strings:GearGameUI.MessageBoxStrings.ResetTutorials_Message>",
			"<Strings:GearGameUI.MessageBoxStrings.ResetTutorials_Question>",
			ButtonAliases,
			OnResetTutorials_Confirm,
			GetPlayerOwner(PlayerIndex) );
	}

	return true;
}

/** Whether the tutorials can be reset or not */
function bool TutorialsCanBeReset()
{
	local int CurrValue;

	if (MyProfile != none)
	{
		if (MyProfile.GetProfileSettingValueInt(MyProfile.SPTutorials1, CurrValue) && CurrValue != 0)
		{
			return true;
		}

		if (MyProfile.GetProfileSettingValueInt(MyProfile.SPTutorials2, CurrValue) && CurrValue != 0)
		{
			return true;
		}

		if (MyProfile.GetProfileSettingValueInt(MyProfile.SPTutorials3, CurrValue) && CurrValue != 0)
		{
			return true;
		}

		if (MyProfile.GetProfileSettingValueInt(MyProfile.MPTutorials1, CurrValue) && CurrValue != 0)
		{
			return true;
		}

		if (MyProfile.GetProfileSettingValueInt(MyProfile.MPTutorials2, CurrValue) && CurrValue != 0)
		{
			return true;
		}

		if (MyProfile.GetProfileSettingValueInt(MyProfile.MPTutorials3, CurrValue) && CurrValue != 0)
		{
			return true;
		}

		if (MyProfile.GetProfileSettingValueInt(MyProfile.TrainTutorials1, CurrValue) && CurrValue != 0)
		{
			return true;
		}

		if (MyProfile.GetProfileSettingValueInt(MyProfile.TrainTutorials2, CurrValue) && CurrValue != 0)
		{
			return true;
		}

		if (MyProfile.GetProfileSettingValueInt(MyProfile.TrainTutorials3, CurrValue) && CurrValue != 0)
		{
			return true;
		}
	}

	return false;
}

/** Callback from asking if the player wants to reset tutorials */
function bool OnResetTutorials_Confirm( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	if (SelectedInputAlias == 'ResetAccept')
	{
		if (MyProfile != none)
		{
			MyProfile.SetProfileSettingValueInt(MyProfile.SPTutorials1, 0);
			MyProfile.SetProfileSettingValueInt(MyProfile.SPTutorials2, 0);
			MyProfile.SetProfileSettingValueInt(MyProfile.SPTutorials3, 0);
			MyProfile.SetProfileSettingValueInt(MyProfile.MPTutorials1, 0);
			MyProfile.SetProfileSettingValueInt(MyProfile.MPTutorials2, 0);
			MyProfile.SetProfileSettingValueInt(MyProfile.MPTutorials3, 0);
			MyProfile.SetProfileSettingValueInt(MyProfile.TrainTutorials1, 0);
			MyProfile.SetProfileSettingValueInt(MyProfile.TrainTutorials2, 0);
			MyProfile.SetProfileSettingValueInt(MyProfile.TrainTutorials3, 0);
			bTutorialsReset = true;
		}
	}
	return true;
}

/** Overloaded to support resetting tutorials */
function bool SettingChangesHaveBeenMade()
{
	return (bTutorialsReset || Super.SettingChangesHaveBeenMade());
}

defaultproperties
{
	SettingsData(0)=(ProfileSettingId=HudOnOff,WidgetName="olistFadingHud")
	SettingsData(1)=(ProfileSettingId=Subtitles,WidgetName="olistSubtitles",bIsGlobalValue=true)
	SettingsData(2)=(ProfileSettingId=PictogramTooltips,WidgetName="olistToolTips")
	SettingsData(3)=(ProfileSettingId=WeapTutorialsOn,WidgetName="olistTutorials")
	SettingsData(4)=(ProfileSettingId=Gore,WidgetName="olistGore",bIsGlobalValue=true)
	SettingsData(5)=(ProfileSettingId=MatureLang,WidgetName="olistExtremeContent",bIsGlobalValue=true,bRequiresMatureLanguageSupport=true)
}
