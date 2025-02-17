
/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class CovGoal_WithinWeaponRange extends CoverGoalConstraint
	native;

/** if this is ON and the cover is outside the envelope it will be thrown the eph out */
var() bool bHardConstraint;

/** cached value indicating whether the searching AI is a leader (affects distance bias) */
var transient bool bIsLeader;

cpptext
{
	virtual UBOOL EvaluateCoverMarker(ACoverSlotMarker* Marker, APawn* SeekingPawn, AGearAI_Cover* SeekingAI, AActor* Goal, FLOAT& Rating);
};

function Init(Goal_AtCover GoalEvaluator)
{
	bIsLeader = GearPRI(GoalEvaluator.AI.PlayerReplicationInfo).bIsLeader;
}
