/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GDT_Immulsion extends GDT_Environment
	dependson(GearSoundGroup);

var() protected const float MediumScreamHeatThreshold;

/** Which lens effect to play when damaged by fire. */
var() protected const class<Emit_CameraLensEffectBase> LensEffectClass;

/** Overridden to add sound effect, skin effects. */
static function HandleDamageFX(GearPawn DamagedPawn, const out TakeHitInfo HitInfo)
{
	local GearPC PC;
	local float HeatPct, DamagePct, OldLastPainTime;
	local bool bDoPainFX;

	// do this before the super so we can handle pain sounds ourself
	bDoPainFX = (DamagedPawn.WorldInfo.TimeSeconds - DamagedPawn.LastPainTime > 0.95f);
	if (bDoPainFX)
	{
		OldLastPainTime = DamagedPawn.LastPainTime;
		DamagedPawn.LastPainTime = DamagedPawn.WorldInfo.TimeSeconds;
	}

	Super.HandleDamageFX(DamagedPawn,HitInfo);

	// camera fx
	if (DamagedPawn.IsHumanControlled())
	{
		// spawn camera lens effect
		PC = DamagedPawn.GetPC();
		if (PC != None)
		{
			PC.ClientSpawnCameraLensEffect(default.LensEffectClass);
		}
	}

	// scream!
	if ( bDoPainFX && (DamagedPawn.Health > 0) )
	{
		if (DamagedPawn.CurrentSkinHeat > default.MediumScreamHeatThreshold)
		{
			DamagedPawn.SoundGroup.PlayEffort(DamagedPawn, GearEffort_OnFirePain);
		}
		else
		{
			// reset LastPainTime to old value -- not enough sustained damage to scream, wait a sec
			DamagedPawn.LastPainTime = OldLastPainTime;
		}
	}

	// fire damage causes skin "heating" effect
	DamagePct = HitInfo.Damage / float(DamagedPawn.default.DefaultHealth);
	HeatPct =  DamagePct / DamagedPawn.default.SkinHeatDamagePctToFullyHeat;
	DamagedPawn.HeatSkin(HeatPct);
	DamagedPawn.CharSkin(HeatPct*2.f);		// char double, since it's less noticeable

	// Start the imulsion effects
	DamagedPawn.StartImulsion(2.0f);
}

static function bool ShouldIgnoreExecutionRules( Pawn TestPawn, Pawn Instigator )
{
	return TRUE;
}

defaultproperties
{
	LensEffectClass=class'Emit_CameraScorch'
	MediumScreamHeatThreshold=0.2f
}
