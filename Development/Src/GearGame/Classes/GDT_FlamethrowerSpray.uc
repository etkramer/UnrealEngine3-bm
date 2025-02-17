/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GDT_FlamethrowerSpray extends GDT_Fire;


/** Overridden to add camera effect, skin effects. */
static function HandleDamageFX(GearPawn DamagedPawn, const out TakeHitInfo HitInfo)
{
	local SpawnedGearEmitter SpawnedEmitter;

	super.HandleDamageFX(DamagedPawn, HitInfo);

	if ( (DamagedPawn.DamagedByFlamethrowerEmitter == None) && DamagedPawn.CurrentSkinHeat > 0.6f )
	{
		// this is ok to not use the GOP as the pawn will have it on him for a while and then will destroy it
		SpawnedEmitter = DamagedPawn.Spawn(class'SpawnedGearEmitter', DamagedPawn,, DamagedPawn.Location);
		if (SpawnedEmitter != None)
		{
			SpawnedEmitter.SetTemplate(ParticleSystem'COG_Flamethrower.Effects.P_Player_smoke', TRUE);
			SpawnedEmitter.SetBase(DamagedPawn,, DamagedPawn.Mesh, DamagedPawn.TorsoBoneName);
			SpawnedEmitter.ParticleSystemComponent.ActivateSystem(TRUE);
			DamagedPawn.DamagedByFlamethrowerEmitter = SpawnedEmitter;
		}
	}
}

defaultproperties
{
	KillGUDEvent=GUDEvent_KilledEnemyFlamethrower
	DamageHeavyGUDEvent=GUDEvent_DamagedEnemyHeavy
	bSuppressImpactFX=TRUE
	bSuppressBloodDecals=TRUE
	bEnvironmentalDamage=FALSE
	ImpactTypeBallisticID=ITB_Scorcher
	WeaponID=WC_Scorcher
}

