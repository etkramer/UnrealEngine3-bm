/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GDT_HOD extends GDT_Explosive;

/** Which lens effect to play when damaged by fire. */
var() protected const class<Emit_CameraLensEffectBase> LensEffectClass;

/** Overridden to attach a burn effect to the victim */
static function HandleDamagedPawn(Pawn DamagedPawn, Pawn Instigator, int DamageAmt, vector Momentum)
{
	local GearPawn GP;
	local GearGRI GRI;
	GP = GearPawn(DamagedPawn);
	if (GP != None)
	{
		GRI = GearGRI(GP.WorldInfo.GRI);
		if (GRI != None && GP.BurnEffectPSC == None && GP.Mesh != None)
		{
			GP.BurnEffectPSC = GRI.GOP.GetImpactParticleSystemComponent( GP.PS_BurnEffect );
			GP.Mesh.AttachComponent( GP.BurnEffectPSC, GP.MeleeDamageBoneName, vect(0,0,0), rot(0,0,0));
			GP.BurnEffectPSC.SetLODLevel( GRI.GetLODLevelToUse(GP.BurnEffectPSC.Template, GP.Location) );
			GP.BurnEffectPSC.ActivateSystem();
		}
	}
	Super.HandleDamagedPawn(DamagedPawn,Instigator,DamageAmt,Momentum);
}

/** Overridden to add camera effect, skin effects. */
static function HandleDamageFX(GearPawn DamagedPawn, const out TakeHitInfo HitInfo)
{
	local GearPC PC;

	Super.HandleDamageFX(DamagedPawn, HitInfo);

	// camera lens fx
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

/** Overridden to ignite players on death */
static function HandleKilledPawn(Pawn KilledPawn, Pawn Instigator)
{
	local GearPawn GP;
	GP = GearPawn(KilledPawn);
	if (GP != None)
	{
		GP.IgnitePawn(POF_Blazing, 10.f);
	}
	Super.HandleKilledPawn(KilledPawn,Instigator);
}

static function HandleDBNOPawn(Pawn DBNOPawn)
{
	local GearPawn GP;
	GP = GearPawn(DBNOPawn);
	if (GP != None)
	{
		GP.IgnitePawn(POF_Smoldering);
	}
	Super.HandleDBNOPawn(DBNOPawn);
}


defaultproperties
{
	ImpactTypeExplosionID=ITE_HOD
	LensEffectClass=class'Emit_CameraHOD'

	KilledByIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=150,UL=77,VL=29)
	bCausesFracture=TRUE

	KillGUDEvent=GUDEvent_KilledEnemyHOD

	bEnvironmentalDamage=FALSE
	WeaponID=WC_HOD
}
