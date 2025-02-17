/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GearAI_FlameDrone extends GearAI_Hunter;


defaultproperties
{

	// defaultcover goal evaluation profiles
	// ---->Enemies constraint
	Begin Object Class=CovGoal_Enemies Name=CovGoal_Enemies0
	End Object
	// ---->Enemies too close
	Begin object Class=CovGoal_EnemyProximity Name=CovGoal_EnemyProx0
	End object
	// ---->Weapon range constraint
	Begin object Class=CovGoal_WithinWeaponRange Name=CovGoal_WeaponRange0
		bHardConstraint=true
	End Object
	// ---->Movement distance
	Begin Object Class=CovGoal_MovementDistance Name=CovGoal_MovDist0
		BestCoverDist=768.0f
		MinCoverDist=256.0f
		MaxCoverDist=1024.0f
	End object
	// ---->Squad leader proximity
	Begin Object Class=CovGoal_SquadLeaderProximity name=CovGoal_SquadLeadProx0
	End Object
	// ---->Teammate Proximity
	Begin Object Class=CovGoal_TeammateProximity name=CovGoal_TeammateProx0
	End Object

	// ** TOWARDS
	Begin Object Class=Goal_AtCover name=AtCov_Towards1
		CoverGoalConstraints.Add(CovGoal_Enemies0)
		CoverGoalConstraints.Add(CovGoal_EnemyProx0)
		CoverGoalConstraints.Add(CovGoal_WeaponRange0)
		CoverGoalConstraints.Add(CovGoal_MovDist0)
		CoverGoalConstraints.Add(CovGoal_SquadLeadProx0)
		CoverGoalConstraints.Add(CovGoal_TeammateProx0)
	End object
	AtCover_Toward=AtCov_Towards1
}