/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class CoverSlotMarker extends NavigationPoint
	native;

cpptext
{
	virtual void addReachSpecs(AScout* Scout, UBOOL bOnlyChanged);
	virtual void AddForcedSpecs( AScout *Scout );
	virtual UBOOL CanConnectTo(ANavigationPoint* Nav, UBOOL bCheckDistance);
	virtual UBOOL ShouldBeBased()
	{
		// don't base since we're set directly to the slot location
		return FALSE;
	}

	UBOOL PlaceScout(AScout *Scout);
	virtual UBOOL CanPrunePath(INT index);
	virtual UClass* GetReachSpecClass( ANavigationPoint* Nav, UClass* ReachSpecClass );
}

var() editconst CoverInfo	OwningSlot;

/** AI cover selection ignores slots with this flag set unless they are the only slots available */
var bool bLastChoice;
/** Ignore size limits for path building */
var transient bool bIgnoreSizeLimits;

simulated native function Vector  GetSlotLocation();
simulated native function Rotator GetSlotRotation();
simulated native function SetSlotEnabled( bool bEnable );

/** Returns true if the specified pawn is able to claim this slot. */
final native function bool IsValidClaim( Pawn ChkClaim, optional bool bSkipTeamCheck, optional bool bSkipOverlapCheck );

simulated event string GetDebugAbbrev()
{
	return "CSM";
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
