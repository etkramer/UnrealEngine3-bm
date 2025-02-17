/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustBeastRider extends GearPawn_LocustHunterArmorNoGrenades
	config(Pawn);


/** This will return whether or not this mob can be special melee attacked **/
simulated function bool CanBeSpecialMeleeAttacked( GearPawn Attacker )
{
	// no chainsawing when we're on a bloodmount
	if(GearPawn_LocustBloodmount(Base) != none)
	{
		return false;
	}

	return Super.CanBeSpecialMeleeAttacked(Attacker);
}

DefaultProperties
{
	Begin Object Name=GearPawnMesh
		AnimSets.Add(AnimSet'Locust_Theron_Guard.Animations.Bloodmount_Animset')
		Translation=(X=0,Y=0,Z=0)
	End Object

	DefaultInventory.Empty()
	DefaultInventory(0)=class'GearGame.GearWeap_LocustAssaultRifle'

	HelmetType=class'Item_Helmet_LocustBeastRider'

	bCanDBNO=false
}
