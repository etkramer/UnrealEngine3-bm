/** rates goals based on how far away they are from the target */
class Goal_AwayFromPosition extends PathGoalEvaluator
	native;

cpptext
{
	virtual UBOOL EvaluateGoal(ANavigationPoint*& PossibleGoal, APawn* Pawn);
	virtual UBOOL DetermineFinalGoal( ANavigationPoint*& out_GoalNav );
}

/** location to flee from */
var vector AvoidPos;
/** cached direction from AvoidPos to Pawn.Location */
var vector AvoidDir;
/** exit with the best result so far if the travel distance to the nodes being evaluated exceeds this */
var int MaxDist;

/** best node we've found so far */
var NavigationPoint BestNode;
var int BestRating;

static function bool FleeFrom(Pawn P, vector InAvoidPos, int InMaxDist)
{
	local Goal_AwayFromPosition Eval;

	Eval = Goal_AwayFromPosition(P.CreatePathGoalEvaluator(default.class));
	Eval.AvoidPos = InAvoidPos;
	Eval.AvoidDir = Normal(InAvoidPos - P.Location);
	Eval.MaxDist = InMaxDist;
	P.AddGoalEvaluator(Eval);
	return true;
}

event Recycle()
{
	Super.Recycle();
	BestNode = None;
	BestRating = 0;
}

defaultproperties
{
	CacheIdx=7
}
