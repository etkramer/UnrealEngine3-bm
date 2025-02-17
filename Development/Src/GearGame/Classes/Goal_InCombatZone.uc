/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Goal_InCombatZone extends PathGoalEvaluator
	native(AI);

cpptext
{
	// Interface
	virtual UBOOL EvaluateGoal(ANavigationPoint*& PossibleGoal, APawn* Pawn);
}

var GearAI	AI;

static function bool InCombatZone( Pawn P )
{
	local Goal_InCombatZone	Eval;
	local GearAI C;

	if( P != None )
	{
		C = GearAI(P.Controller);
		if( C != None && C.CombatZoneList.Length > 0 )
		{
			Eval = Goal_InCombatZone(P.CreatePathGoalEvaluator(default.class));

			if( Eval != None )
			{
				Eval.AI = C;
				P.AddGoalEvaluator( Eval );
				return TRUE;
			}
		}
	}

	return FALSE;
}

function Recycle()
{
	AI=none;
	Super.Recycle();
}

defaultproperties
{
	CacheIdx=2
}
