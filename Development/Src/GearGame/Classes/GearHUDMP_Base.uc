/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearHUDMP_Base extends GearHUD_Base
	abstract;

var GearGRI GRI;

var localized String RoundString;
var localized String FinalRoundString;
var localized String COGString;
var localized String LocustString;
var localized String VsString;
var localized String WinRoundString;
var localized String WinMatchString;
var localized String FinalKillString;
var localized String FinalSuicideString;
var localized String LeaderKillsString, LeaderPointsString, LeaderDeathsString, LeaderGoalString, LeaderRoundTime;
var localized String LeaderMatchStatus, LeaderMatchTime;
var localized String RoundWasADrawString;

var const float TotalScoreTime;
var float EndRoundSceneDelayTime;
var const float TotalEndRoundTime;

var float DisplayNameTime;
var const float DisplayNameTotalTime;
var bool bDisplayingName;

var localized string ViewingPlayerMsg;

/** The content reference to the UI scene for the scoreboard */
var const GearUIScene_Scoreboard ScoreBoardUISceneReference;
/** The instance of the UI scene for the scoreboard */
var GearUIScene_Scoreboard ScoreBoardUISceneInstance;

/** The content reference to the UI scene for the end-of-round scene */
var const GearUISceneMP_Base EndRoundUISceneReference;
/** The instance reference to the UI scene for the end-of-round scene */
var GearUISceneMP_Base EndRoundUISceneInstance;

function SpawnScoreBoard(class<Scoreboard> ScoringType)
{
}

function GearGRI GetGRI()
{
	if (GRI == None)
	{
		GRI = GearGRI(WorldInfo.GRI);
	}
	return GRI;
}

function DrawHUD()
{
	local GearPC PC;

	PC = GearPC(PlayerOwner);

	if ( !DrawScoreboard() )
	{
		Super.DrawHUD();
	}

	if ( bInMultiplayerMode )
	{
		if ( PC.IsInState('Reviving') )
		{
			// make sure we can draw the button mash action icon
			DrawActions(RenderDelta);
		}
	}
}

/** See if we should open or close the scoreboard */
function bool DrawScoreboard()
{
	local bool bShowScoreboard, bDrewEndRound, bDrewScoreboard;

	// First try and draw the start/end round UI
	if ( GetGRI() != None )
	{
		bDrewEndRound = EndOfRoundSceneIsOpen();
		bDrewScoreboard = bDrewEndRound;
	}

	// See if the scoreboard should be gone
	if ( (GetGRI() == None) || bRestrictScoreboard || bDrewEndRound || (PauseUISceneInstance != None)
	||	(GRI.GameStatus == GS_EndMatch && GRI.RoundTime == 0) )
	{
		bShowScoreboard = false;
	}
	// See if the scoreboard should be shown
	else if ( ShouldShowScoreboardBasedOnGameStatus() || bShowScores )
	{
		bShowScoreboard = true;
	}

	// Open or close the scoreboard
	if ( bShowScoreboard && (WorldInfo.GRI != None) && (WorldInfo.GRI.PRIArray.length > 0) )
	{
		OpenScoreboardScene();
		bDrewScoreboard = true;
	}
	else
	{
		CloseScoreboardScene();
	}

	return bDrewScoreboard;
}

/** Uses the game status to determine if the scoreboard should be showing or not */
function bool ShouldShowScoreboardBasedOnGameStatus()
{
	if ( GearGRI(WorldInfo.GRI).GameStatus != GS_RoundInProgress )
	{
		return true;
	}

	return false;
}

/**
* bUseAtoContinueVersion - whether to show the "Press A to Continue" string or not... if not
* then the "B Back" string shows instead.
*/

function ToggleScoreboard(bool bOn)
{
	bShowScores = bOn;
}

/** Whether the show the hud or not based on screens showing in front of it */
function bool HideHUDFromScenes()
{
	return (Super.HideHUDFromScenes() ||
			ScoreBoardUISceneInstance != none ||
			EndRoundUISceneInstance != none);
}

/** Open the scoreboard scene */
final function OpenScoreboardScene()
{
	local GearPC MyGearPC;

	if ( ScoreBoardUISceneInstance == None )
	{
		MyGearPC = GearPC(PlayerOwner);
		ScoreBoardUISceneInstance = GearUIScene_Scoreboard(MyGearPC.ClientOpenScene( ScoreBoardUISceneReference ));
	}
}

/** Close the scoreboard scene */
final function CloseScoreboardScene()
{
	if ( ScoreBoardUISceneInstance != None )
	{
		ScoreboardUISceneInstance.CloseScene(ScoreboardUISceneInstance, false, true);
		ScoreBoardUISceneInstance = None;
	}
}

/** Open the EndOfRound scene */
final function OpenEndOfRoundScene()
{
	local GearPC MyGearPC;

	// Make sure the GRI has been replicated to this player before opening this scene,
	// this could happen if a player comes in late to the game and the EndOfRound screen opens before replication of the GRI happens
	if ( (EndRoundUISceneInstance == None) && ((GearGRI(WorldInfo.GRI) != None && GearGRI(WorldInfo.GRI).GameStatus == GS_EndMatch) || (WorldInfo.GRI != None && WorldInfo.GRI.PRIArray.length >= 1)) )
	{
		MyGearPC = GearPC(PlayerOwner);
		CloseScoreboardScene();
		EndRoundUISceneInstance = GearUISceneMP_Base(MyGearPC.ClientOpenScene( EndRoundUISceneReference ));
	}
}

/** Close the EndOfRound scene */
function CloseEndOfRoundScene()
{
	if ( EndRoundUISceneInstance != None )
	{
		EndRoundUISceneInstance.CloseScene(EndRoundUISceneInstance, false, true);
		EndRoundUISceneInstance = None;
	}
}

/** Whether the EndOfRound scene is drawing or not */
final function bool EndOfRoundSceneIsOpen()
{
	return (EndRoundUISceneInstance != None);
}

function SignalEndofRoundOrMatch(GearGRI CurGRI)
{
	// Don't show the who won on the server if there was a network error
	if (WorldInfo.Game != None && WorldInfo.Game.bHasNetworkError)
	{
		return;
	}

	if ( SpectatorUISceneInstance != None )
	{
		SpectatorUISceneInstance.CloseScene(SpectatorUISceneInstance, false, true);
		SpectatorUISceneInstance = None;
	}

	if (CurGRI.GameStatus == GS_EndMatch || EndRoundSceneDelayTime <= 0.0)
	{
		OpenEndOfRoundScene();
	}
	else
	{
		SetTimer(EndRoundSceneDelayTime, false, nameof(OpenEndOfRoundScene));
	}
	SetTimer( TotalEndRoundTime, FALSE, nameof(CloseEndOfRoundScene) );
}

function SignalStartofRoundOrMatch(GearGRI CurGRI)
{
	// Don't show if there's a network error
	if (WorldInfo.Game != None && WorldInfo.Game.bHasNetworkError)
	{
		return;
	}
}


defaultproperties
{
	TotalScoreTime=3.f

	TotalEndRoundTime=5.f;

	DisplayNameTime=0.f
	DisplayNameTotalTime=2.f
	bDisplayingName=false
}

