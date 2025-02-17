/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class LadderMarker extends NavigationPoint
	native;

cpptext
{
	virtual void addReachSpecs(AScout* Scout, UBOOL bOnlyChanged);
	virtual void AddForcedSpecs( AScout *Scout );
	virtual UBOOL CanConnectTo(ANavigationPoint* Nav, UBOOL bCheckDistance);
	virtual UClass* GetReachSpecClass( ANavigationPoint* Nav, UClass* ReachSpecClass );
	virtual UBOOL CanPrunePath(INT index);
}

var Trigger_LadderInteraction LadderTrigger;
/** Indicates if this trigger is hooked up to the top of the ladder */
var() bool bIsTopOfLadder;

simulated event string GetDebugAbbrev()
{
	return "LM";
}

defaultproperties
{
	bCollideWhenPlacing=FALSE
	bSpecialMove=TRUE

	Components.Remove(Sprite)
	Components.Remove(Sprite2)
	Components.Remove(Arrow)

	Begin Object Name=CollisionCylinder
		CollisionRadius=40.f
		CollisionHeight=40.f
	End Object
}
