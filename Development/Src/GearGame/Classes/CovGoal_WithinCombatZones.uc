
/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class CovGoal_WithinCombatZones extends CoverGoalConstraint
	native;

cpptext
{
	virtual UBOOL EvaluateCoverMarker(ACoverSlotMarker* Marker, APawn* SeekingPawn, AGearAI_Cover* SeekingAI, AActor* Goal, FLOAT& Rating);
};

DefaultProperties
{
	ConstraintEvaluationPriority=0
}