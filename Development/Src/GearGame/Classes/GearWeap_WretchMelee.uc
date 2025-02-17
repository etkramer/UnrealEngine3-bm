/**
 * Wretch melee weapon
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_WretchMelee extends GearWeap_Melee
	config(Weapon)
	hidedropdown;


simulated function StartFire( byte FireModeNum );
simulated function bool ShouldRefire();


function DoMeleeDamage( Actor Target, Vector HitLocation, float DamageScale )
{
	local GearPawn P;
	local int ModeIdx;

	P = GearPawn(Instigator);
	if( P != None )
	{
		ModeIdx = P.SpecialMove - GSM_PounceAttack;
		Target.TakeDamage( AttackDamage[ModeIdx] * DamageScale, Instigator.Controller, HitLocation, Normal(Target.Location-Instigator.Location) * DamageScale, class'GDT_Wretch_Melee',, self );
		GearPawn_LocustWretchBase(Instigator).PlaySwipeAttackHitSound();

		//`log( "Wretch Damage" $ AttackDamage[ModeIdx] );
	}
}

defaultproperties
{
	bCanThrow=FALSE
}
