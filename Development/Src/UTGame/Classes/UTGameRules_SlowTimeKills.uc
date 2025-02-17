// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.

class UTGameRules_SlowTimeKills extends GameRules;

var float SlowTime;
var float RampUpTime;
var float SlowSpeed;

function ScoreKill(Controller Killer, Controller Killed)
{
	if ( PlayerController(Killer) != None )
	{
		WorldInfo.Game.SetGameSpeed(SlowSpeed);
		SetTimer(SlowTime, false);
	}
	if ( NextGameRules != None )
	{
		NextGameRules.ScoreKill(Killer,Killed);
	}
}

function timer()
{
	GotoState('rampup');
}

state Rampup
{
	function Tick(float DeltaTime)
	{
		local float NewGameSpeed;

		NewGameSpeed = WorldInfo.Game.GameSpeed + DeltaTime/RampUpTime;
		if ( NewGameSpeed >= 1 )
		{
			WorldInfo.Game.SetGameSpeed(1.0);
			GotoState('');
		}
		else
		{
			WorldInfo.Game.SetGameSpeed(NewGameSpeed);
		}
	}
}

defaultproperties
{
	RampUpTime=0.1
	SlowTime=0.3
	SlowSpeed=0.25
}