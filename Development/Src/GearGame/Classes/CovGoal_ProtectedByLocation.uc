class CovGoal_ProtectedByLocation extends CoverGoalConstraint
	native;

var Vector	ThreatLocation;
var bool	bOnlySlotsWithFireLinks;
var bool	bForceMoveTowardGoal;			

cpptext
{
	virtual UBOOL EvaluateCoverMarker( ACoverSlotMarker* Marker, APawn* SeekingPawn, AGearAI_Cover* SeekingAI, AActor* Goal, FLOAT& Rating );
};