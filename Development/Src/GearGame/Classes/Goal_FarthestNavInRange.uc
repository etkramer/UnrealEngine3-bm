/** returns as the goal the farthest NavigationPoint of the given list that is within the specified range
 * if none found within range, keeps searching until it finds one or it runs out of paths
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Goal_FarthestNavInRange extends PathGoalEvaluator
	native;

cpptext
{
	virtual UBOOL EvaluateGoal(ANavigationPoint*& PossibleGoal, APawn* Pawn);
	virtual UBOOL DetermineFinalGoal(ANavigationPoint*& out_GoalNav);
}

/** the maximum distance to search - will abort when this distance is reached unless none of the goal nodes have been encountered */
var int OptimalMaxDist;
/** list of goals to search for */
var array<NavigationPoint> GoalList;

/** current best result */
var NavigationPoint CurrentBest;

static function bool FarthestNavInRange(Pawn P, int InOptimalMaxDist, const out array<NavigationPoint> InGoalList)
{
	local Goal_FarthestNavInRange Eval;

	Eval = Goal_FarthestNavInRange(P.CreatePathGoalEvaluator(default.class));

	Eval.OptimalMaxDist = InOptimalMaxDist;
	Eval.GoalList = InGoalList;
	P.AddGoalEvaluator(Eval);
	return true;
}

function Recycle()
{
	OptimalMaxDist=default.OptimalMaxDist;
	GoalList.length=0;
	CurrentBest=none;
	Super.Recycle();
}

defaultproperties
{
	CacheIdx=1
	MaxPathVisits=10000 // don't want it to fail for this reason, reduce OptimalMaxDist to improve performance instead
}
