/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Path_PreferCover extends PathConstraint
	native(AI);

cpptext
{
	// Interface
	virtual UBOOL EvaluatePath( UReachSpec* Spec, APawn* Pawn, INT& out_PathCost, INT& out_HeuristicCost );
}

static function bool PreferCover( Pawn P )
{
	if( P != None )
	{
		P.AddPathConstraint( P.CreatePathConstraint(default.class) );
		return TRUE;
	}

	return FALSE;
}

	
defaultproperties
{
	CacheIdx=8
}