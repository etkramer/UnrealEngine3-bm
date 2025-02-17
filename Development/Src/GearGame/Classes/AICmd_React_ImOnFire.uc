/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class AICmd_React_ImOnFire extends AICommand
	within GearAI;

var Controller EventInstigator;

var() float FreakOutDist;

static function bool InitCommandUserActor( GearAI AI, Actor UserActor )
{
	local AICmd_React_ImOnFire Cmd;

	if( AI != None )
	{
		Cmd = new(AI) class'AICmd_React_ImOnFire';
		if( Cmd != None )
		{
			Cmd.EventInstigator = Controller(UserActor);
			AI.PushCommand(Cmd);
			return TRUE;
		}
	}

	return FALSE;
}

function Pushed()
{
	bAllowCombatTransitions=false;
	StopFiring();
	GotoState('RunSomeWhere');
}

function bool AllowTransitionTo( class<AICommand> AttemptCommand )
{
	return false; // one way ticket
}

function bool ShouldRunSomewhereElse()
{
	return frand() < 0.6f;
}

state FreakOut
{
	function vector GetFreakOutPoint()
	{
		local vector TryPt;
		local vector TryDir;
		local int i;

		//FlushPersistentDebugLines();
		TryDir = VRand();
		TryDir.Z = 0.f;
		TryDir = Normal(TryDir);
		TryPt = Pawn.Location + TryDir * FreakOutDist;
		for(i=0;i<15;i++)
		{
			TryDir = VRand();
			TryDir.Z = 0.f;
			TryDir = Normal(TryDir);
			// don't bother testing if this is too close to the last direction we tested
			if((TryDir*FreakOutDist) dot (TryPt - Pawn.Location) > 0.8f)
			{
				continue;
			}
			TryPt = Pawn.Location + TryDir * FreakOutDist;

			if(PointReachable(TryPt))
			{
				//DrawDebugCoordinateSystem(TryPt,rot(0,0,0),100.f,true);
				return TryPt;
			}
		}

		//DrawDebugCoordinateSystem(TryPt,rot(0,0,0),100.f,true);
		return TryPt;
	}
Begin:
	InvalidateCover();
	StopFiring();
	bShouldRoadieRun=true;
	Pawn.GroundSpeed *= 1.1f;
	MoveTo(GetFreakOutPoint(),none,false);
	if(ShouldRunSomewhereElse())
	{
		GotoState('RunSomewhere');
	}
	Sleep(RandRange(0.1f,0.5f));
	Goto('Begin');
}

state RunSomewhere
{
	function GearPawn GetClosestEnemy(out float BestDistSq)
	{
		Local GearPawn CurEnemy;
		local float CurDistSq;
		local GearPawn BestEnemy;

		foreach Squad.AllEnemies(class'GearPawn',CurEnemy)
		{
			CurDistSq = VSizeSq(CurEnemy.Location - Pawn.Location);
			if(IsEnemyVisible(CurEnemy) && (BestEnemy == none || CurDistSq < BestDistSq))
			{
				BestEnemy = CurEnemy;
				BestDistSq = CurDistSq;
			}
		}
		return BestEnemy;

	}

	function Actor GetRunDest()
	{
		Local gearAI CurAI;
		local float CurDistSq;
		local GearAI BestAI;
		local float  BestDistSq;
		local GearPawn ClosestEnemy;

		BestDistSq = 65535.f;
		foreach Squad.AllMembers(class'GearAI',CurAI)
		{
			if(CurAI == Outer)
			{
				continue;
			}
			CurDistSq = VSizeSq(CurAI.Pawn.Location - Pawn.Location);
			if(BestAI == none || CurDistSq < BestDistSq)
			{
				BestAI = CurAI;
				BestDistSq = CurDistSq;
			}
		}

		// now see if there is an enemy closer
		ClosestEnemy = GetClosestEnemy(BestDistSq);
		if(ClosestEnemy != none)
		{
			return ClosestEnemy;
		}

		return BestAI.Pawn;
	}

	function bool RunToDest()
	{
		Local Actor Dest;

		Dest = GetRunDest();

		if(Dest != none)
		{
			SetMoveGoal(Dest);
			return true;
		}

		return false;

	}
Begin:
	InvalidateCover();
	StopFiring();
	bShouldRoadieRun=true;
	Pawn.GroundSpeed *= 1.1f;
	if(RunToDest())
	{
		Sleep(RandRange(0.1f,0.5f));
	}
	GotoState('FreakOut');
}

DefaultProperties
{
	FreakOutDist=350.f
}
