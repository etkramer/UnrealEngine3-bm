//=============================================================================
// ScoreBoard
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================

class ScoreBoard extends Hud
	config(Game);

var bool bDisplayMessages;

/**
 * The Main Draw loop for the hud.  Get's called before any messaging.  Should be subclassed
 */
function DrawHUD()
{
	UpdateGRI();
    UpdateScoreBoard();
}

function bool UpdateGRI()
{
    if (WorldInfo.GRI == None)
    {
		return false;
	}
    WorldInfo.GRI.SortPRIArray();
    return true;
}

function UpdateScoreBoard();

function ChangeState(bool bIsVisible);

defaultproperties
{
	TickGroup=TG_DuringAsyncWork
}
