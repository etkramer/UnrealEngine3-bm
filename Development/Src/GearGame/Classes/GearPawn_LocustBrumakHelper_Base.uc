/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
**/
class GearPawn_LocustBrumakHelper_Base extends GearPawn
	native(Pawn);

cpptext
{
	virtual UBOOL IsAliveAndWell() const
	{
		return (!bDeleteMe);
	}
};

simulated function UpdateAnimSetList();

simulated function PlayWeaponSwitch(Weapon OldWeapon, Weapon NewWeapon)
{
	MyGearWeapon = GearWeapon(Weapon);
	AttachWeapon();
}

simulated native event Vector GetPawnViewLocation();

defaultproperties
{
	bTranslateMeshByCollisionHeight=FALSE
}
