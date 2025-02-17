/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GDT_InkGrenade extends GearDamageType;

var() protected const class<Emit_CameraLensEffectBase> LensEffectClass;

/** Overridden to add camera effect. */
static function HandleDamageFX(GearPawn DamagedPawn, const out TakeHitInfo HitInfo)
{
	local GearPC PC;
	Super.HandleDamageFX(DamagedPawn,HitInfo);

	if (DamagedPawn.IsHumanControlled())
	{
		// spawn camera lens effect
		PC = DamagedPawn.GetPC();
		if (PC != None)
		{
			PC.ClientSpawnCameraLensEffect(default.LensEffectClass);
		}
	}
}

static function bool ShouldIgnoreExecutionRules( Pawn TestPawn, Pawn Instigator )
{
	return TRUE;
}

defaultproperties
{
	ImpactTypeExplosionID=ITE_GrenadeInk
	LensEffectClass=class'Emit_CameraInk'
	KilledByIcon=(Texture=Texture2D'Warfare_HUD.HUD.Gears_DeathIcons',U=0,V=90,UL=77,VL=29)
	KDamageImpulse=0.0

	bSuppressImpactFX=TRUE
	WeaponID=WC_InkGrenade
}
