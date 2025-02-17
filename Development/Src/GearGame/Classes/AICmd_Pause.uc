/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class AICmd_Pause extends AICommand
	within GearAI;

var() float Delay;

static function bool Pause( GearAI AI, float InDelay )
{
	local AICmd_Pause Cmd;

	if( AI != None )
	{
		Cmd = new(AI) Default.Class;
		if( Cmd != None )
		{
			Cmd.Delay = InDelay;
			AI.PushCommand(Cmd);
			return TRUE;
		}
	}

	return FALSE;
}

function Pushed()
{
	super.Pushed();
	`AILog("Pausing for "$Delay$" second(s).");
	GotoState('Wait');
}


state Wait `DEBUGSTATE
{
Begin:
	Sleep(Delay);
	GotoState('DelaySuccess');
};


defaultproperties
{
	Delay=10.0f
}