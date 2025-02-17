/** 
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Goal_SquadFormation extends PathGoalEvaluator
	native;

cpptext
{
	virtual UBOOL EvaluateGoal(ANavigationPoint*& PossibleGoal, APawn* Pawn);
	virtual UBOOL DetermineFinalGoal(ANavigationPoint*& out_GoalNav);

	AActor* GetBestPosition( TArray<AActor*> &UsedList, UBOOL bRetKey, INT& out_PosIdx );
	void	WeightPositionsBySquadMembers();
}

struct native FormationEvalInfo
{
	var int		PosIdx;
	var Actor	QueryActor;
	var native	Map{AActor*,FLOAT} ActorList;
};
var private const native array<FormationEvalInfo> PositionList; 

/** AI that is asking for squad formation search */
var GearAI	SeekerAI;
/** Position index of the squad we are looking for */
var int	PositionIdx;
/** Squad formation that is being searched */
var GearSquadFormation Formation;


private native function ClearPositionList();

static function bool SquadFormation( Pawn P, GearAI inSeekerAI, GearSquadFormation inFormation, optional int inPositionIdx = -1 )
{
	local Goal_SquadFormation Eval;

	if( P != None && inFormation != None && inSeekerAI != None )
	{
		Eval = Goal_SquadFormation(P.CreatePathGoalEvaluator(default.class));

		if( Eval != None )
		{
			Eval.SeekerAI	 = inSeekerAI;
			Eval.PositionIdx = inPositionIdx;
			Eval.Formation	 = inFormation;
			P.AddGoalEvaluator( Eval );
			return TRUE;
		}
	}

	return FALSE;
}

function Recycle()
{
	SeekerAI=none;
	PositionIdx=default.PositionIdx;
	Formation=none;
	ClearPositionList();
	Super.Recycle();
}

defaultproperties
{
	CacheIdx=4
}
