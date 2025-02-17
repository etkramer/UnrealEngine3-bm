
/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class CovGoal_Enemies extends CoverGoalConstraint
	native;

struct native ValidEnemyCacheDatum
{
	var GearPawn EnemyPawn;
	var CoverInfo EnemyCover;
};

var array<ValidEnemyCacheDatum> ValidEnemyCache;

cpptext
{
	virtual UBOOL EvaluateCoverMarker(ACoverSlotMarker* Marker, APawn* SeekingPawn, AGearAI_Cover* SeekingAI, AActor* Goal, FLOAT& Rating);
};

// overidden to cache the valid enemy list
event native Init(Goal_AtCover GoalEvaluator);

DefaultProperties
{
	// expensive, do this last
	ConstraintEvaluationPriority=10
}