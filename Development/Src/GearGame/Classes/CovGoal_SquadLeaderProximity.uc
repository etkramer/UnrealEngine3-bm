
/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class CovGoal_SquadLeaderProximity extends CoverGoalConstraint
	native;

/** cached location of searching AI's squadleader */
var transient vector SquadLeaderLocation;

cpptext
{
	virtual UBOOL EvaluateCoverMarker(ACoverSlotMarker* Marker, APawn* SeekingPawn, AGearAI_Cover* SeekingAI, AActor* Goal, FLOAT& Rating);
};

function Init(Goal_AtCover GoalEvaluator)
{
	SquadLeaderLocation = GoalEvaluator.AI.Squad.GetSquadLeaderLocation();
}
