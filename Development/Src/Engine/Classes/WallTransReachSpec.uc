/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class WallTransReachSpec extends ForcedReachSpec
	native;

cpptext
{
	virtual INT CostFor(APawn* P);
}

defaultproperties
{
	bSkipPrune=TRUE
}
