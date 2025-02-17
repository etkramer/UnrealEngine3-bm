/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTHoldSpot extends UTDefensePoint
	native
	notplaceable;

var UTVehicle HoldVehicle;

/** since HoldSpots aren't part of the prebuilt nav network we need to hook them to another NavigationPoint */
var NavigationPoint LastAnchor;

cpptext
{
	virtual ANavigationPoint* SpecifyEndAnchor(APawn* RouteFinder);
	virtual void NotifyAnchorFindingResult(ANavigationPoint* EndAnchor, APawn* RouteFinder);
	virtual UBOOL AnchorNeedNotBeReachable();
}

function PreBeginPlay()
{
	Super(NavigationPoint).PreBeginPlay();
}

function Actor GetMoveTarget()
{
	if ( HoldVehicle != None )
	{
		if ( HoldVehicle.Health <= 0 )
			HoldVehicle = None;
		if ( HoldVehicle != None )
			return HoldVehicle.GetMoveTargetFor(None);
	}

	return self;
}

function FreePoint()
{
	Destroy();
}

defaultproperties
{
	bCollideWhenPlacing=false
	bStatic=false
	bNoDelete=false
}
