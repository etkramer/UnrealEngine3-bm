
/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class CovGoal_AvoidanceVolumes extends CoverGoalConstraint
	native;

cpptext
{
	virtual UBOOL EvaluateCoverMarker(ACoverSlotMarker* Marker, APawn* SeekingPawn, AGearAI_Cover* SeekingAI, AActor* Goal, FLOAT& Rating);
};