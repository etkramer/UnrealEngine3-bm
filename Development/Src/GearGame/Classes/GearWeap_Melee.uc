/**
* Base melee weapon
*
*  Weapon used for melee only
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GearWeap_Melee extends GearWeapon
	config(Weapon)
	hidedropdown;

var() config array<float>	AttackDamage;

function DoMeleeDamage( Actor Target, Vector HitLocation, float DamageScale )
{
	local GearPawn P;

	P = GearPawn(Instigator);
	if( P != None )
	{
		Target.TakeDamage( AttackDamage[0] * DamageScale, Instigator.Controller, HitLocation, Normal(HitLocation-Instigator.Location), class'GDT_Melee',, self );
	}
}

/** this weapon has no team based colors **/
simulated function SetMaterialBasedOnTeam( MeshComponent TheMesh, int TeamNum );

simulated function TimeWeaponEquipping()
{
	SetTimer( 0.0001f, FALSE, 'WeaponEquipped' );
	return;
}

defaultproperties
{
	bCanThrowActiveWeapon=FALSE

	WeaponEquipSound=None
	WeaponDeEquipSound=None
	WeaponReloadSound=None
	FireSound=None

	InstantHitDamageTypes(0)=class'GDT_Melee'
}
