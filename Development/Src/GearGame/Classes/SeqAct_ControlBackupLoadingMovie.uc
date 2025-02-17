/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_ControlBackupLoadingMovie extends SequenceAction;


/** On Disable, whether to pause the game for a few seconds (StreamByURL->PostLoadPause .ini setting). The loading movie keeps playing during this time. */
var() bool bPauseAfterDisable;

/** On Disable, after the game has been unpaused, wait an additional few seconds before stopping the movie. */
var() float StopMovieAfterUnpauseDuration;


event Activated()
{
	local GearGameSP_Base Game;
	local GearPC PC;

	Game = GearGameSP_Base(GetWorldInfo().Game);
	if (InputLinks[0].bHasImpulse)
	{
		Game.bNeedBackupLoadingMovie = true;
		foreach GetWorldInfo().AllControllers(class'GearPC', PC)
		{
			PC.ClientKeepPlayingLoadingMovie();
		}
	}
	else
	{
		Game.bNeedBackupLoadingMovie = false;
		foreach GetWorldInfo().AllControllers(class'GearPC', PC)
		{
			PC.ClientShowLoadingMovie(false, bPauseAfterDisable, 0.0, StopMovieAfterUnpauseDuration, true);
		}
	}
}

defaultproperties
{
	ObjName="Control Backup Loading Movie"
	ObjCategory="Cinematic"

	InputLinks(0)=(LinkDesc="Enable")
	InputLinks(1)=(LinkDesc="Disable")

	bPauseAfterDisable=true
	StopMovieAfterUnpauseDuration=0.0

	VariableLinks.Empty
}
