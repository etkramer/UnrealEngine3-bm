/**
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUIScene_HordeOptions extends GearUIScene_Base
	Config(UI);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Label describing which wave you just died on */
var transient UILabel WaveDescLabel;

/** Label displaying the final team score */
var transient UILabel TeamScoreLabel;

/** Button to press for replaying the current wave */
var transient UILabelButton ContinueButton;

/** Button to press for restarting back to wave 1 */
var transient UILabelButton RestartButton;

/** Button to press to return to the lobby */
var transient UILabelButton QuitToLobbyButton;

/** Button to press to return to the mainmenu */
var transient UILabelButton QuitButton;

/** Localized strings */
var localized string WaveTitleString;

/** Is ready to close */
var transient bool bIsReadyToClose;

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/**
 * Called after this screen object's children have been initialized.  While the Initialized event is only called when
 * a widget is initialized for the first time, PostInitialize() will be called every time this widget receives a call
 * to Initialize(), even if the widget was already initialized.  Examples would be reparenting a widget.
 */
event PostInitialize()
{
	Super.PostInitialize();

	if ( !IsEditor() )
	{
		InitializeWidgetReferences();
	}
}

/** Assigns all member variables to appropriate child widget from this scene */
function InitializeWidgetReferences()
{
	local string StringToDraw;
	local GearGRI GRI;

	GRI = GetGRI();

	WaveDescLabel = UILabel(FindChild('lblWave', true));
	TeamScoreLabel = UILabel(FindChild('lblTeamScore', true));

	ContinueButton = UILabelButton(FindChild('btnContinue', true));
	RestartButton = UILabelButton(FindChild('btnRestart', true));
	QuitButton = UILabelButton(FindChild('btnQuit', true));
	QuitToLobbyButton = UILabelButton(FindChild('btnQuitLobby', true));

	ContinueButton.OnClicked = OnContinueClicked;
	RestartButton.OnClicked = OnRestartClicked;
	QuitButton.OnClicked = OnQuitClicked;
	QuitToLobbyButton.OnClicked = OnQuitToLobbyClicked;

	StringToDraw = WaveTitleString @ (GRI.ExtendedRestartCount * class'GearGameHorde_Base'.default.MaxWaves) + GRI.InvasionCurrentWaveIndex;
	WaveDescLabel.SetDataStoreBinding( StringToDraw );
	TeamScoreLabel.SetDataStoreBinding( string(GearTeamInfo(GRI.Teams[0]).TotalScore) );

	if (IsArbitrated())
	{
		QuitButton.DisableWidget(GetBestPlayerIndex());
		ContinueButton.SetForcedNavigationTarget(UIFACE_Top, QuitToLobbyButton);
		QuitToLobbyButton.SetForcedNavigationTarget(UIFACE_Bottom, ContinueButton);
	}
}

/** @return true if the return to lobby button can be used in this pause menu */
function bool IsArbitrated()
{
	local OnlineSubsystem OnlineSub;
	local OnlineGameSettings GameSettings;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None && OnlineSub.GameInterface != None)
	{
		GameSettings = OnlineSub.GameInterface.GetGameSettings('Game');
	}

	// Don't let arbitrated matches quit
	return (GameSettings != None && GameSettings.bUsesArbitration);
}

/** Restart the current wave */
function bool OnContinueClicked(UIScreenObject EventObject, int PlayerIndex)
{
	GearGameHorde_Base(GetWorldInfo().Game).RestartFromCheckpoint();
	CloseScene();
	return true;
}

/** Restart the whole game */
function bool OnRestartClicked(UIScreenObject EventObject, int PlayerIndex)
{
	GearGameHorde_Base(GetWorldInfo().Game).RestartFromBeginning();
	CloseScene();
	return true;
}

/** Quit to the main menu */
function bool OnQuitClicked(UIScreenObject EventObject, int PlayerIndex)
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;
	local WorldInfo WI;

	WI = GetWorldInfo();
	if ( WI != None )
	{
		ButtonAliases.AddItem('GenericCancel');
		ButtonAliases.AddItem('GenericContinue');
		GameSceneClient = GetSceneClient();
		GameSceneClient.ShowUIMessage('ConfirmQuit',
			"<Strings:GearGameUI.MessageBoxStrings.ReturnToMainMenu_Title>",
			"<Strings:GearGameUI.MessageBoxStrings.ReturnToMainMenu_LeaderMessage>",
			"",
			ButtonAliases, OnReturnToMainMenuConfirmed, GetPlayerOwner(PlayerIndex));
	}
	return true;
}

/** Callback when the user confirms leaving */
function bool OnReturnToMainMenuConfirmed( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	local WorldInfo WI;
	local GearPC PC;

	if ( SelectedInputAlias == 'GenericContinue' )
	{
		PC = GetGearPlayerOwner(0);
		WI = GetWorldInfo();
		if ( PC != None && WI != None )
		{
			// Make sure that Horde writes the stats in this case
			EndHordeMatch(WI.Game);
			PC.ReturnToMainMenu();
			// Save the profiles
			foreach WI.LocalPlayerControllers(class'GearPC', PC)
			{
				PC.SaveProfile();
			}
		}
		CloseScene(Self);
	}
	return true;
}

/** Quit to the party lobby */
function bool OnQuitToLobbyClicked(UIScreenObject EventObject, int PlayerIndex)
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	ButtonAliases.AddItem('GenericCancel');
	ButtonAliases.AddItem('GenericContinue');
	GameSceneClient = GetSceneClient();
	GameSceneClient.ShowUIMessage('ConfirmQuit',
		"<Strings:GearGameUI.MessageBoxStrings.ReturnToLobby_Title>",
		"<Strings:GearGameUI.MessageBoxStrings.ReturnToLobby_Message>",
		"<Strings:GearGameUI.MessageBoxStrings.ReturnToLobby_Question>",
		ButtonAliases, OnReturnToLobbyConfirmed, GetPlayerOwner(PlayerIndex));

	return true;
}

/** Callback when the user confirms leaving */
function bool OnReturnToLobbyConfirmed( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	local WorldInfo WI;
	local GearPC PC;

	if ( SelectedInputAlias == 'GenericContinue' )
	{
		PC = GetGearPlayerOwner(0);
		WI = GetWorldInfo();
		if ( PC != None && WI != None )
		{
			// Make sure that Horde writes the stats in this case
			EndHordeMatch(WI.Game);
			// Now that the game has completed properly, return to party
			WI.Game.TellClientsToReturnToPartyHost();
			// Save the profiles
			foreach WI.LocalPlayerControllers(class'GearPC', PC)
			{
				PC.SaveProfile();
			}
		}
		CloseScene(Self);
	}
	return true;
}

/**
 * Has Horde clean up properly before traveling
 *
 * @param Game the game object to wrap up
 */
function EndHordeMatch(GameInfo Game)
{
	if (Game != None)
	{
		Game.WriteOnlinePlayerScores();
		Game.EndOnlineGame();
	}
}

/**
 * Handler for the scene's OnSceneActivated delegate.  Called after scene focus has been initialized, or when the scene becomes active as
 * the result of another scene closing.
 *
 * @param	ActivatedScene			the scene that was activated
 * @param	bInitialActivation		TRUE if this is the first time this scene is being activated; FALSE if this scene has become active
 *									as a result of closing another scene or manually moving this scene in the stack.
 */
function SceneActivationComplete( UIScene ActivatedScene, bool bInitialActivation )
{
	local GearPC PC;

	PC = GetGearPlayerOwner(GetBestPlayerIndex());
	PC.SetPause(true, CanUnpauseHordeOptions);
}

function bool CanUnpauseHordeOptions()
{
	return bIsReadyToClose;
}

/** Called when the scene is closed */
function OnSceneDeactivatedCallback( UIScene DeactivatedScene )
{
	local GearPC PC;

	bIsReadyToClose = true;
	PC = GetGearPlayerOwner(GetBestPlayerIndex());
	PC.SetPause(false);
}

defaultproperties
{
	OnSceneActivated=SceneActivationComplete
	OnSceneDeactivated=OnSceneDeactivatedCallback

	Begin Object Name=SceneEventComponent
		DisabledEventAliases.Add(CloseScene)
	End Object

}
