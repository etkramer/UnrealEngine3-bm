/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearUISceneFETran_Campaign extends GearUISceneFETran_Base
	Config(UI);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** The button bar for the scene */
var transient UICalloutButtonPanel ButtonBar;

/** Host-Coop button */
var transient UILabelButton HostCoopButton;
/** Join-Coop button */
var transient UILabelButton JoinCoopButton;
/** Splitscreen button */
var transient UILabelButton SplitButton;

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
	// Button bar
	ButtonBar = UICalloutButtonPanel(FindChild('pnlButtonBar', true));
	ButtonBar.SetButtonCallback('GenericBack', OnBackClicked);

	// Menu buttons
	HostCoopButton = UILabelButton(FindChild('btnHost', true));
	HostCoopButton.OnClicked = OnHostCampaign;
	JoinCoopButton = UILabelButton(FindChild('btnJoin', true));
	JoinCoopButton.OnClicked = OnJoinCampaign;
	SplitButton = UILabelButton(FindChild('btnSplit', true));
	SplitButton.OnClicked = OnSplitCampaign;
}

/** Get the memory slot availability that was cached in the main menu */
function GetMemorySlotAvailability( out int EmptySlots, out int UsedSlots )
{
	local UIScene MainMenuScene;
	local GameUISceneClient GameSceneClient;

	GameSceneClient = GetSceneClient();
	if (GameSceneClient != none)
	{
		MainMenuScene = GameSceneClient.FindSceneByTag('UI_FE_MainMenu');
		if (MainMenuScene != none)
		{
			GearUISceneFETran_MainMenu(MainMenuScene).GetMemorySlotAvailability(EmptySlots, UsedSlots);
		}
	}
}

/** Called when the host coop campaign button is clicked */
function bool OnHostCampaign(UIScreenObject EventObject, int PlayerIndex)
{
	// Clear all the navigation values we store to know where we are in the FE
	ClearAllTransitionValues();

	// Check for a network connection first
	if (!class'UIInteraction'.static.HasLinkConnection())
	{
		DisplayErrorMessage("NoLinkConnection_Message", "NoLinkConnection_Title", "GenericContinue", GetPlayerOwner(GetBestPlayerIndex()));
	}
	// We're good, go to the lobby
	else
	{
		// Leave a breadcrumb of where we came from
		SetTransitionValue("Campaign", "Host");
		// Attempt to go to the lobby
		AttemptCampaignLobby(PlayerIndex);
	}
	return true;
}

/** Called when the join coop campaign button is clicked */
function bool OnJoinCampaign(UIScreenObject EventObject, int PlayerIndex)
{

	// Clear all the navigation values we store to know where we are in the FE
	ClearAllTransitionValues();

	// Check for a network connection first
	if (!class'UIInteraction'.static.HasLinkConnection())
	{
		DisplayErrorMessage("NoLinkConnection_Message", "NoLinkConnection_Title", "GenericContinue", GetPlayerOwner(GetBestPlayerIndex()));
	}
	// We're good, go to the lobby
	else
	{
		// Leave a breadcrumb of where we came from
		SetTransitionValue("Campaign", "Join");
		// Attempt to go to the lobby
		AttemptCampaignLobby(PlayerIndex);
	}
	return true;
}

/** Back clicked */
function bool OnBackClicked(UIScreenObject EventObject, int PlayerIndex)
{
	// Clear all the navigation values we store to know where we are in the FE
	ClearAllTransitionValues();
	CloseScene();
	return true;
}

defaultproperties
{
	Begin Object Name=SceneEventComponent
		DisabledEventAliases.Add(CloseScene)
	End Object
}
