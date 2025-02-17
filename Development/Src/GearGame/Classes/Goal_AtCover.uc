/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Goal_AtCover extends PathGoalEvaluator
	native(AI);

cpptext
{
	// Interface
	virtual UBOOL EvaluateGoal(ANavigationPoint*& PossibleGoal, APawn* Pawn);
	virtual UBOOL DetermineFinalGoal( ANavigationPoint*& out_GoalNav );
	void RateSlotMarker(ACoverSlotMarker* Marker, APawn* Pawn, INT BaseRating);
	virtual void NotifyExceededMaxPathVisits( ANavigationPoint* BestGuess );
	UBOOL IsValid(ACoverSlotMarker* Marker, APawn* Pawn);
}

/** AI looking for cover */
var GearAI_Cover AI;

/** Best valid slot marker and cost that was found */
var CoverSlotMarker	BestMarker;
var INT				BestRating;

/** maximum number of nodes to rate before picking the best one **/
var() INT				MaxToRate;

/** number of markers tested so far */
var int NumMarkersTested;

/** Actor that we desired to be near */
var Actor			TetherActor;

/** list of constraints used to evaluate cover as we find it */
var instanced array<CoverGoalConstraint> CoverGoalConstraints;

final function Init(GearAI_Cover GAIC, Actor GoalActor)
{
	local int i;

	if (GAIC.Squad != None && GAIC.MyGearPawn != None)
	{
		BestMarker = None;
		BestRating = class'NavigationPoint'.const.INFINITE_PATH_COST;
		NumMarkersTested = 0;
		AI = GAIC;
		TetherActor = GoalActor;

		for (i = 0; i < CoverGoalConstraints.length; i++)
		{
			CoverGoalConstraints[i].Init(self);
		}
		InitNative();
		GAIC.Pawn.AddGoalEvaluator(self);
	}
	else
	{
		`Warn("Invalid AI - Squad" @ GAIC.Squad @ "GearPawn" @ GAIC.MyGearPawn);
	}


}

private native function InitNative();


native final noexport function RateSlotMarker(CoverSlotMarker Marker, Pawn Pawn, int BaseRating);
event AddCoverGoalConstraint(CoverGoalConstraint Constraint)
{
	CoverGoalConstraints[CoverGoalConstraints.length] = Constraint;
}

event String GetDumpString()
{
	local int Idx;
	local String Str;

	if( CoverGoalConstraints.Length == 0 )
	{
		return "Empty Cov Goal Constraints";
	}
	
	for( Idx = 0; Idx < CoverGoalConstraints.length; Idx++ )
	{
		Str = Str@CoverGoalConstraints[Idx].GetDumpString()@"\n";
	}
	return Str;
}

defaultproperties
{
	MaxToRate=40
	MaxPathVisits=500
}
