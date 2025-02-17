/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GDT_Headshot extends GDT_Ballistic;



/** force a headshot and force the pawn to take enough damage to kil them hopefully **/
static function ModifyDamage(Pawn Victim, Controller InstigatedBy, out int Damage, TraceHitInfo HitInfo, vector HitLocation, vector Momentum)
{
	local GearPawn	GP;

	Super.ModifyDamage( Victim, InstigatedBy, Damage, HitInfo, HitLocation, Momentum );

	// always say we were hit with a head shot
	GP = GearPawn(Victim);
	if( GP != None )
	{
		Damage = GP.default.DefaultHealth * 10.0f; // kill them!!
		GP.bLastHitWasHeadShot = TRUE;
	}
}


static function bool ShouldHeadShotGib(Pawn TestPawn, Pawn Instigator)
{
	return TRUE;
}

static function bool IsScriptedDamageType()
{
	return TRUE;
}

defaultproperties
{
	KDamageImpulse=100
	WeaponID=WC_Longshot
}

