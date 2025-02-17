/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTEntryGame extends UTTeamGame;

function bool NeedPlayers()
{
	return false;
}

exec function AddBots(int num) {}

function StartMatch()
{}

// Parse options for this game...
event InitGame( string Options, out string ErrorMessage )
{
	ParseAutomatedTestingOptions(Options);
}

auto State PendingMatch
{
	function RestartPlayer(Controller aPlayer)
	{
	}

	function Timer()
    {
    }

    function BeginState(Name PreviousStateName)
    {
		bWaitingToStartMatch = true;
		UTGameReplicationInfo(GameReplicationInfo).bWarmupRound = false;
		StartupStage = 0;
		bQuickStart = false;
    }

	function EndState(Name NextStateName)
	{
		UTGameReplicationInfo(GameReplicationInfo).bWarmupRound = false;
	}
}

defaultproperties
{
	HUDType=class'UTGame.UTEntryHUD'
	PlayerControllerClass=class'UTGame.UTEntryPlayerController'
	ConsolePlayerControllerClass=class'UTGame.UTEntryPlayerController'

	bExportMenuData=false
}
