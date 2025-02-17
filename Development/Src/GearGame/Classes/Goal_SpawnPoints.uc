/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Goal_SpawnPoints extends PathGoalEvaluator
	native(AI);

cpptext
{
	// Interface
	virtual UBOOL EvaluateGoal(ANavigationPoint*& PossibleGoal, APawn* Pawn);
	virtual UBOOL DetermineFinalGoal( ANavigationPoint*& out_GoalNav );
}

/** AI looking for cover */
var GearAI_Cover AI;
var array<NavigationPoint> PickedSpawnPoints;
// list of spawn points to use in case we didn't find any really good ones
var array<NavigationPoint> BackupSpawnPoints;

var int NumSpawnpointsNeeded;
var float MinDistToEnemy;

static function Goal_SpawnPoints FindSpawnPoints(Pawn P, GearAI_Cover InAI, int InNumSpawnPointsNeeded,float InMinDistToEnemy, optional int InMaxpathvisits=-1)
{
	local Goal_SpawnPoints Eval;
	if(P != none && InAI != none)
	{
		Eval = Goal_SpawnPoints(P.CreatePathGoalEvaluator(default.class));
		if(Eval != none)
		{
			Eval.AI = InAI;
			Eval.NumSpawnpointsNeeded = InNumSpawnPointsNeeded;
			Eval.MinDistToEnemy = InMinDistToEnemy;

			if(InMaxPathVisits > 0)
			{
				Eval.MaxPathVisits = InMaxPathVisits;
			}
			P.AddGoalEvaluator(Eval);
			return Eval;
		}
	}

	return none;
}

function Recycle()
{
	BackupSpawnPoints.length=0;
	AI=none;
	NumSpawnpointsNeeded=default.NumSpawnpointsNeeded;
	maxPathVisits=default.MaxPathVisits;
	MinDistToEnemy=default.MinDistToEnemy;
	Super.Recycle();
}

function ClearFoundSpawns()
{
	PickedSpawnPoints.length=0;
}
defaultproperties
{
	NumSpawnpointsNeeded=1
	MinDistToEnemy=1024
	MaxPathVisits=750
	CacheIdx=6
}
