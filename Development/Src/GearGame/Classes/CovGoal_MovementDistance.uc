
/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class CovGoal_MovementDistance extends CoverGoalConstraint
	native;

var() float BestCoverDist; // desired distance to move
var() float MaxCoverDist;  // maximum distance to move
var() float MinCoverDist;  // minimum distance to move

var() bool	bMoveTowardGoal;
var() float MinDistTowardGoal;

cpptext
{
	virtual UBOOL EvaluateCoverMarker(ACoverSlotMarker* Marker, APawn* SeekingPawn, AGearAI_Cover* SeekingAI, AActor* Goal, FLOAT& Rating);
};