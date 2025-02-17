/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class Path_AvoidanceVolumes extends PathConstraint
	native(AI);

var GearAI MyGearAI;

var array<AIAvoidanceCylinderComponent> AffectingCylinders;

cpptext
{
	// Interface
	virtual UBOOL EvaluatePath( UReachSpec* Spec, APawn* Pawn, INT& out_PathCost, INT& out_HeuristicCost );
}

static final function bool AvoidThoseVolumes( Pawn P )
{
	local Path_AvoidanceVolumes Con;
	local Array<AIAvoidanceCylinderComponent>  LocalAffectingCylinders;

	local GearAI GAI;
	GAI = GearAI(P.Controller);
	
	if (GAI != none && class'AIAvoidanceCylinderComponent'.static.BuildListOfAffectingCylinders(GAI,LocalAffectingCylinders))
	{
		Con = Path_AvoidanceVolumes(P.CreatePathConstraint(default.class));
		Con.MyGearAI = GAI;
		Con.AffectingCylinders = LocalAffectingCylinders;
		P.AddPathConstraint(Con);
		return TRUE;
	}

	return FALSE;
}

function Recycle()
{
	Super.Recycle();
	AffectingCylinders.length=0;
	MyGearAI=none;
}
defaultproperties
{
	CacheIdx=5
}
