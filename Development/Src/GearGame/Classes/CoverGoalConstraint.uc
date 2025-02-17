class CoverGoalConstraint extends Object
	native
	abstract;

// constraints will be evaluate roughly in the order of their priority, that is cover goal constraints with a lower evaluation priority will
// be executed before constraints with a higher priority.. this is useful for tweaking the order
// that constraints are executed for performance.. e.g. to execute the slowest constraint last
var int ConstraintEvaluationPriority;

cpptext
{
	virtual UBOOL EvaluateCoverMarker(ACoverSlotMarker* Marker, APawn* SeekingPawn, AGearAI_Cover* SeekingAI, AActor* Goal, FLOAT& Rating){return TRUE;};
};

/** called when adding our owning Goal_AtCover to an AI's goal evaluators, giving us a chance to cache stuff */
event Init(Goal_AtCover GoalEvaluator);

event String GetDumpString()
{
	return String(self);
}

DefaultProperties
{
	ConstraintEvaluationPriority=5
}