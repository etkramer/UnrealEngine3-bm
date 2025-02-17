/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTJumpPadReachSpec extends UTTrajectoryReachSpec
	native;

cpptext
{
	virtual FVector GetInitialVelocity();
	virtual INT CostFor(APawn* P);
}
