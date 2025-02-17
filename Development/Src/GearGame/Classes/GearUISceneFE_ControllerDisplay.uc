/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUISceneFE_ControllerDisplay extends GearUISceneFrontEnd_Base
	Config(UI);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** The labels of the scene */
var transient UILabel AButtonLabel;
var transient UILabel BButtonLabel;
var transient UILabel XButtonLabel;
var transient UILabel YButtonLabel;
var transient UILabel LeftTriggerLabel;
var transient UILabel RightTriggerLabel;
var transient UILabel LeftBumperLabel;
var transient UILabel RightBumperLabel;
var transient UILabel DPadLabel;
var transient UILabel LeftStickXLabel;
var transient UILabel LeftStickYLabel;
var transient UILabel LeftStickClickLabel;
var transient UILabel RightStickXLabel;
var transient UILabel RightStickYLabel;
var transient UILabel RightStickClickLabel;
var transient UILabel BackButtonLabel;
var transient UILabel StartButtonLabel;
var transient UILabel PlayerNameLabel;
var transient UILabel ControllerIconLabel;
var transient UILabel ConfigLabel;
var transient UICalloutButtonPanel ButtonBar;

/** Localized strings */
var localized string AButtonString;
var localized string AButtonAltString;
var localized string BButtonString;
var localized string XButtonString;
var localized string XButtonAltString;
var localized string YButtonString;
var localized string YButtonAltString;
var localized string LeftTriggerString;
var localized string RightTriggerString;
var localized string LeftBumperString;
var localized string RightBumperString;
var localized string DPadString;
var localized string LeftStickXString;
var localized string LeftStickYString;
var localized string LeftStickClickString;
var localized string RightStickXString;
var localized string RightStickYString;
var localized string RightStickClickString;
var localized string RightStickClickAltString;
var localized string RightStickAltString;
var localized string BackButtonString;
var localized string StartButtonString;


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/**
 * Called after this screen object's children have been initialized
 * Overloaded to initialized the references to this scene's widgets
 */
event PostInitialize()
{
	InitializeWidgetReferences();

	Super.PostInitialize();
}

/** Initialize the widget references */
final function InitializeWidgetReferences()
{
	local GearPC PC;
	local bool bUsingAlt;
	local bool bUsingTrigFlip;
	local EGearStickConfigOpts StickConfig;
	local EGearTriggerConfigOpts TrigConfig;
	local LocalPlayer LP;
	local OnlineSubsystem OnlineSub;
	local string ControllerString;
	local string PlayerName;

	PC = GetGearPlayerOwner(GetBestPlayerIndex());

	AButtonLabel = UILabel(FindChild('lblA', true));
	BButtonLabel = UILabel(FindChild('lblB', true));
	XButtonLabel = UILabel(FindChild('lblX', true));
	YButtonLabel = UILabel(FindChild('lblY', true));
	LeftTriggerLabel = UILabel(FindChild('lblLT', true));
	RightTriggerLabel = UILabel(FindChild('lblRT', true));
	LeftBumperLabel = UILabel(FindChild('lblLB', true));
	RightBumperLabel = UILabel(FindChild('lblRB', true));
	DPadLabel = UILabel(FindChild('lblDpad', true));
	LeftStickXLabel = UILabel(FindChild('lblLStickX', true));
	LeftStickYLabel = UILabel(FindChild('lblLStickY', true));
	LeftStickClickLabel = UILabel(FindChild('lblLStickClick', true));
	RightStickXLabel = UILabel(FindChild('lblRStickX', true));
	RightStickYLabel = UILabel(FindChild('lblRStickY', true));
	RightStickClickLabel = UILabel(FindChild('lblRStickClick', true));
	BackButtonLabel = UILabel(FindChild('lblBack', true));
	StartButtonLabel = UILabel(FindChild('lblStart', true));
	PlayerNameLabel = UILabel(FindChild('lblName', true));
	PlayerNameLabel.SetVisibility(false);
	ControllerIconLabel = UILabel(FindChild('lblProfile', true));
	ControllerIconLabel.SetVisibility(false);
	ConfigLabel = UILabel(FindChild('lblConfiguration', true));
	if (ConfigLabel != none)
	{
		ConfigLabel.SetVisibility(false);
	}
	ButtonBar = UICalloutButtonPanel(FindChild('pnlButtonBar', true));


	// Setup button bar
	ButtonBar.SetButtonCallback('GenericBack', OnBackClicked);

	if (PC != none && PC.ProfileSettings != none)
	{
		// Handle playername and controller icon
		LP = LocalPlayer(PC.Player);
		if (LP != None)
		{
			// Get the widget references and set the values for the player's name
			OnlineSub = class'GameEngine'.static.GetOnlineSubSystem();
			if (OnlineSub != None && OnlineSub.PlayerInterface != None)
			{
				PlayerName = OnlineSub.PlayerInterface.GetPlayerNickname(LP.ControllerId);
			}
			else if ( LP != None )
			{
				PlayerName = LP.Actor.PlayerReplicationInfo.PlayerName;
			}
			PlayerNameLabel.SetVisibility(true);
			PlayerNameLabel.SetDataStoreBinding(PlayerName);

			// Set the player's profile icon
			ControllerString = GetControllerIconString(LP.ControllerId);
			ControllerIconLabel.SetVisibility(true);
			ControllerIconLabel.SetDataStoreBinding(ControllerString);
		}

		bUsingAlt = PC.ProfileSettings.GetControlScheme() == GCS_Alternate;
		TrigConfig = PC.ProfileSettings.GetTrigConfigOption();
		bUsingTrigFlip = (TrigConfig == WTCO_SouthPaw);
		StickConfig = PC.ProfileSettings.GetStickConfigOption();

		// A Button
		AButtonLabel.SetDataStoreBinding(bUsingAlt ? AButtonAltString : AButtonString);
		// B Button
		BButtonLabel.SetDataStoreBinding(BButtonString);
		// X Button
		XButtonLabel.SetDataStoreBinding(bUsingAlt ? XButtonAltString : XButtonString);
		// Y Button
		YButtonLabel.SetDataStoreBinding(bUsingAlt ? YButtonAltString : YButtonString);
		// Left and Right Stick Click
		if (StickConfig == WSCO_Legacy || StickConfig == WSCO_SouthPaw)
		{
			RightStickClickLabel.SetDataStoreBinding(LeftStickClickString);
			LeftStickClickLabel.SetDataStoreBinding(bUsingAlt ? RightStickClickAltString : RightStickClickString);
		}
		else
		{
			RightStickClickLabel.SetDataStoreBinding(bUsingAlt ? RightStickClickAltString : RightStickClickString);
			LeftStickClickLabel.SetDataStoreBinding(LeftStickClickString);
		}
		// Left Stick X
		if (StickConfig == WSCO_Legacy || StickConfig == WSCO_SouthPaw)
		{
			LeftStickXLabel.SetDataStoreBinding(RightStickXString);
		}
		else
		{
			LeftStickXLabel.SetDataStoreBinding(LeftStickXString);
		}
		// Left Stick Y
		if (StickConfig == WSCO_SouthPaw || StickConfig == WSCO_LegacySouthpaw)
		{
			LeftStickYLabel.SetDataStoreBinding(RightStickYString);
		}
		else
		{
			LeftStickYLabel.SetDataStoreBinding(LeftStickYString);
		}
		// Right Stick X
		if (StickConfig == WSCO_Legacy || StickConfig == WSCO_SouthPaw)
		{
			RightStickXLabel.SetDataStoreBinding(LeftStickXString);
		}
		else
		{
			RightStickXLabel.SetDataStoreBinding(RightStickXString);
		}
		// Right Stick Y
		if (StickConfig == WSCO_SouthPaw || StickConfig == WSCO_LegacySouthpaw)
		{
			RightStickYLabel.SetDataStoreBinding(LeftStickYString);
		}
		else
		{
			RightStickYLabel.SetDataStoreBinding(RightStickYString);
		}
		// Triggers
		LeftTriggerLabel.SetDataStoreBinding(bUsingTrigFlip ? RightTriggerString : LeftTriggerString);
		RightTriggerLabel.SetDataStoreBinding(bUsingTrigFlip ? LeftTriggerString : RightTriggerString);
		// Bumpers
		LeftBumperLabel.SetDataStoreBinding(bUsingTrigFlip ? RightBumperString : LeftBumperString);
		RightBumperLabel.SetDataStoreBinding(bUsingTrigFlip ? LeftBumperString : RightBumperString);
		// DPad
		DPadLabel.SetDataStoreBinding(DPadString);
		// Back Button
		BackButtonLabel.SetDataStoreBinding(BackButtonString);
		// Start Button
		StartButtonLabel.SetDataStoreBinding(StartButtonString);
	}
}

/** Back clicked */
function bool OnBackClicked(UIScreenObject EventObject, int PlayerIndex)
{
	CloseScene();
	return true;
}

defaultproperties
{
	Begin Object Name=SceneEventComponent
		DisabledEventAliases.Add(CloseScene)
	End Object
}
