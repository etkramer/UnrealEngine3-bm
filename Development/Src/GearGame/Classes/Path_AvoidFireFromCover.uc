/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Path_AvoidFireFromCover extends PathConstraint
	native(AI);

cpptext
{
	// Interface
	virtual UBOOL EvaluatePath( UReachSpec* Spec, APawn* Pawn, INT& out_PathCost, INT& out_HeuristicCost );
}

var GearAI AI;

struct native EnemyCoverInfo
{
	var GearPawn Enemy;
	var CoverInfo Cover;
};

var array<EnemyCoverInfo> EnemyList;

static function bool AvoidFireFromCover( Pawn P )
{
	local Path_AvoidFireFromCover Con;
	local GearAI TheAI;
	local GearPawn Enemy;
	local EnemyCoverInfo EnemyInfo;

	TheAI = GearAI(P.Controller);
	if (TheAI != None && TheAI.Squad != None)
	{
		Con = Path_AvoidFireFromCover(P.CreatePathConstraint(default.class));
		Con.AI = TheAI;
		foreach TheAI.Squad.AllEnemies(class'GearPawn', Enemy, TheAI)
		{
			if (Enemy.IsPlayerOwned() && TheAI.GetPlayerCover(Enemy, EnemyInfo.Cover, false))
			{
				EnemyInfo.Enemy = Enemy;
				Con.EnemyList.AddItem(EnemyInfo);
			}
		}
		if (Con.EnemyList.length > 0)
		{
			P.AddPathConstraint(Con);
		}
		return TRUE;
	}

	return FALSE;
}

function Recycle()
{
	Super.Recycle();
	AI = None;
	EnemyList.length = 0;
}
defaultproperties
{
	CacheIdx=6
}
