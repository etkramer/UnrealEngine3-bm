
/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class CovGoal_GoalProximity extends CoverGoalConstraint
	native;

var() float BestGoalDist;
var() float MinGoalDist;
var() float MaxGoalDist;
/** if set markers are completely rejected if they are outside the min and max distance */
var() bool bHardLimits;

cpptext
{
	virtual UBOOL EvaluateCoverMarker(ACoverSlotMarker* Marker, APawn* SeekingPawn, AGearAI_Cover* SeekingAI, AActor* Goal, FLOAT& Rating);
};
