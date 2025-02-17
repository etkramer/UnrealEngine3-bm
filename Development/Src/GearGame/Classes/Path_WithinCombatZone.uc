/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Path_WithinCombatZone extends PathConstraint
	native(AI);

// if this is non-null we will check to see if the nodes are in this combat zone only, rather than checking the list
var CombatZone SpecificCombatZone;
cpptext
{
	// Interface
	virtual UBOOL EvaluatePath( UReachSpec* Spec, APawn* Pawn, INT& out_PathCost, INT& out_HeuristicCost );
}

var GearAI AI;

/** penalty to apply to edges which start inside a desired combat zone and then leave (to encourage traversal within combat zones first) **/
var() int LeavingCombatZonePenalty;

static function bool WithinCombatZone( Pawn P, optional int inPenalty, optional CombatZone InSpecificCombatZone )
{
	local Path_WithinCombatZone Constraint;
	local GearAI C;

	if( P != None )
	{
		C = GearAI(P.Controller);
		if( C != None && C.CombatZoneList.Length > 0 )
		{
			Constraint = Path_WithinCombatZone(P.CreatePathConstraint(default.class));
			if( Constraint != None )
			{
				if( inPenalty > 0 )
				{
					Constraint.LeavingCombatZonePenalty = inPenalty;
				}
				Constraint.AI = C;
				Constraint.SpecificCombatZone = InSpecificCombatZone;
				P.AddPathConstraint( Constraint );
				return TRUE;
			}
		}
	}

	return FALSE;
}

function Recycle()
{
	AI=none;
	Super.Recycle();
}

defaultproperties
{
	LeavingCombatZonePenalty=100
	CacheIdx=9
}

