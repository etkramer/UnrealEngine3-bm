/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Base_Kantus extends AICommand_Base_Combat
	within GearAI_Kantus;

var AIReactCond_GenericCallDelegate SeeEnemyStopCond;

function Pushed()
{
	Super.Pushed();
	GotoState('InCombat');
}

function Popped()
{
	Super.Popped();

	bShouldRoadieRun = FALSE;
}


state Roaming `DEBUGSTATE
{
	function Actor GenerateRoamPath( Actor Goal, optional float Distance, optional bool bAllowPartialPath )
	{
		local Actor Ret;
		local Vector SearchStartPt;

		// take squad centroid and project back away from enemy so we go "behind" the squad
		SearchStartPt = Squad.GetSquadCentroid();
		SearchStartPt += Normal(SearchStartPt - Goal.Location) * EnemyDistance_Short;

		// move toward squad centroid, but be sure to move a bit each time
		class'Path_TowardPoint'.static.TowardPoint(Pawn,SearchStartPt);
		
		//Pawn.FlushPersistentDebugLines();
		//Pawn.DrawDebugCoordinateSystem(SearchStartPt,rot(0,0,0),100.f,TRUE);
		`AILog("trying to search toward"@SearchStartPt@"MaxRoamDist:"@MaxRoamDist@EnemyDistance_Short@EnemyDistance_Medium);

		class'Path_AvoidanceVolumes'.static.AvoidThoseVolumes( Pawn );
		class'Path_WithinTraversalDist'.static.DontExceedMaxDist(Pawn,MaxRoamDist,FALSE);

		// envelope to keep us in our desired range to enemy
		class'Path_WithinDistanceEnvelope'.static.StayWithinEnvelopeToloc(Pawn, Enemy.Location,EnemYDistance_Long,EnemyDistance_Short,TRUE,500.0);

		class'Goal_Null'.static.GoUntilBust(Pawn,512);

		Ret = FindPathToward( Goal );
		Pawn.ClearConstraints();
		return Ret;
	}

	function bool RoamTowardEnemy()
	{
		local actor Path;
		// skip path search if we're not allowed to move
		if(!AllowedToMove())
		{
			`AILog("Not roaming because AllowedToMove() returned false");
			return false;
		}

		Path = GenerateRoamPath(Enemy,0,TRUE);
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
	}

	`AILog("Done roaming");

	// if we can't see our enemy from here, roam again!
	if(!IsEnemyVisible(Enemy))
	{
		`AILog("Enemy wasn't visible, reacquiring");
		GotoState('Reacquire');
	}
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

state InCombat
{
	function BeginState(Name PreviousStateName)
	{
		InvalidateCover();
	}

Begin:
	//debug
	`AILog( "BEGIN TAG", 'State' );

	// check for any interrupt transitions
	CheckInterruptCombatTransitions();
	// select the best enemy
	SelectEnemy();
	FireTarget = Enemy;

	if (!HasValidEnemy())
	{
		//debug
		`AILog("No enemy");

		Sleep(1.f);
		Goto('Begin');
	}

	while( IsReloading() )
	{
		//debug
		`AILog("Waiting on reload",'Loop');

		Sleep(0.25f);
	}

	// Fire from open
	FireFromOpen();

	Sleep( 0.25f );
	
	CheckInterruptCombatTransitions();
	GotoState('Roaming');
}

defaultproperties
{
}

