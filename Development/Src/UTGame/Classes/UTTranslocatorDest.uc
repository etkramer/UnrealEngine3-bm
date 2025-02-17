/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/** builds UTTranslocatorReachSpecs towards this node for each entry in the StartPoints array  */
class UTTranslocatorDest extends NavigationPoint
	native
	placeable;

cpptext
{
	virtual void addReachSpecs(AScout* Scout, UBOOL bOnlyChanged);
	virtual void PostEditChange(UProperty* PropertyThatChanged);
}

struct native TranslocatorSource
{
	/** the actual source point */
	var() NavigationPoint Point;
	/** if set, try to determine target translocator disc velocity/special jump Z automatically */
	var() bool bAutoDetectVelocity;
	/** translocator disc velocity the AI should use */
	var() vector RequiredTransVelocity;
	/** Jump Z velocity AI must be able to attain to jump here, or <= 0 if this point cannot be reached by jumping */
	var() float RequiredJumpZ;

	structdefaultproperties
	{
		bAutoDetectVelocity=true
	}
};

/** list of points that the AI can translocate/specialjump to this node from */
var() array<TranslocatorSource> StartPoints;
