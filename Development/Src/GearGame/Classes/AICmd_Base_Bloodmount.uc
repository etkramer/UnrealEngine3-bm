/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AICmd_Base_Bloodmount extends AICommand_Base_Combat
	within GearAI_Bloodmount;

var AIReactCond_GenericCallDelegate SeeEnemyStopCond;

function Pushed()
{
	Super.Pushed();
	RoamStartTime=WorldInfo.TimeSeconds;
	GotoState('InCombat');
}

state InCombat
{
Begin:
	
	if( !SelectTarget() )
	{
		`AILog(self@"Could not select target!");
		CheckCombatTransition();
		Sleep(0.5f);
	}

	if(Enemy == none)
	{
		Sleep(0.25f);
		Goto('Begin');
	}

	if(ShouldCharge())
	{
		Charge();
	}
	else
	{
		GotoState('Roaming');
	}

	Sleep(0.1f);
	Goto('Begin');
}

state Roaming `DEBUGSTATE
{
	function Actor GenerateRoamPath( Actor Goal, optional float Distance, optional bool bAllowPartialPath )
	{
		local float DistToGoal;
		local Actor Ret;

		DistToGoal = VSize(Pawn.Location - Goal.Location);
		class'Path_TowardGoal'.static.TowardGoal( Pawn, Goal );
		class'Path_AvoidanceVolumes'.static.AvoidThoseVolumes( Pawn );
		class'Path_WithinTraversalDist'.static.DontExceedMaxDist(Pawn,MaxRoamDist,FALSE);
		
		// real one with soft distances
		class'Path_WithinDistanceEnvelope'.static.StayWithinEnvelopeToLoc(Pawn, Goal.Location,RoamEnvelopeOuter,Min(DistToGoal,RoamEnvelopeInner),FALSE,,TRUE);
		
		// don't go somewhere we can't get out of
		class'Path_AvoidInEscapableNodes'.static.DontGetStuck( Pawn );
		class'Goal_Null'.static.GoUntilBust( Pawn, 1024 );
		
		Ret = FindPathToward( Goal );
		Pawn.ClearConstraints();
		return Ret;
	}

	function bool RoamTowardEnemy()
	{
		local actor Path;
		Path = GenerateRoamPath(Enemy,100.f,TRUE);
		if(Path != none)
		{
			SetMoveGoal(RouteGoal,,,,TRUE);
			return true;
		}

		return false;
	}
Begin:

	`AILog("Roaming toward enemy...");
	if(Enemy == none || !RoamTowardEnemy())
	{
		`AILog("Failed to find roam location for "$Enemy);
		Sleep(1.5f);
		GotoState('Reacquire');
	}

	`AILog("Done roaming");

	// if we can't see our enemy from here, roam again!
	if(!IsPawnVisibleViaTrace(Enemy))
	{
		`AILog("Enemy wasn't visible, reacquiring");
		GotoState('Reacquire');
	}

	`AILog("Enemy was visible, rotating toward him");
	DesiredRotation = rotator(Enemy.Location - Pawn.Location);
	Focus = Enemy;
	SetFocalPoint(vect(0,0,0));
	Focus = MoveFocus;
	Pawn.DesiredRotation = DesiredRotation;
	
	FinishRotation();
	`AILog("Finished rotating, waiting a bit to let our driver pwn his arse");
	Sleep(RandRange(RoamWaitMin,RoamWaitMax));
	GotoState('InCombat');

}

function SawEnemyAgain(Actor EventInstigator, AIReactChannel OriginatingChannel)
{
	`AILog("Saw enemy, aborting move");
	AbortCommand(none,class'AICmd_MoveToEnemy');
}

state Reacquire `DEBUGSTATE
{
	function BeginState(Name PreviousStateName)
	{
		Super.BeginState(PreviousStateName);
		SeeEnemyStopCond = new(outer) class'AIReactCond_GenericCallDelegate';
		SeeEnemyStopCond.AutoSubscribeChannels[0]='sight';
		SeeEnemyStopCond.OutputFunction = SawEnemyAgain;
		SeeEnemyStopCond.bOneTimeOnly = TRUE;
		SeeEnemyStopCond.Initialize();
		RoamStartTime=WorldInfo.TimeSeconds;
	}

	function EndState(Name NextStateName)
	{
		Super.EndState(NextStateName);
		SeeEnemyStopCond.UnsubscribeAll();
		SeeEnemyStopCond = none;
	}

Begin:
	`AILog("Moving toward enemy until we see him");
	SetEnemyMoveGoal(TRUE);
	Sleep(0.1f);
	`AILog("SetEnemyMoveGoal finished.. going back to incombat");
	GotoState('InCombat');

};

defaultproperties
{
}
