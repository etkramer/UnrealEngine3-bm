/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class LeapReachSpec extends AdvancedReachSpec
	native;

cpptext
{
	virtual INT CostFor(APawn* P);
	virtual UBOOL CanBeSkipped(APawn* P)
	{
		return FALSE;
	}
}

var() editconst Vector	CachedVelocity;
var() editconst float	RequiredJumpZ;

defaultproperties
{
	bSkipPrune=TRUE
}
