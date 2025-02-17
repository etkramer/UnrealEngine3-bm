/**
 * This class is the base class for the shotgun damage type.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GDT_Shotgun extends GDT_Ballistic
	config(Weapon);

/** Overridden to prevent COGs being gibbed in Horde */
static function bool ShouldGib(Pawn TestPawn, Pawn Instigator)
{
	// COGs in Horde can't be shotgun gibbed
	if (ClassIsChildOf(TestPawn.WorldInfo.GRI.GameClass,class'GearGameHorde_Base') && TestPawn.GetTeamNum() == 0)
	{
		return FALSE;
	}
	return Super.ShouldGib(TestPawn,Instigator);
}

/** Overridden to eliminate headshots at a distance */
static function ModifyDamage(Pawn Victim, Controller InstigatedBy, out int Damage, TraceHitInfo HitInfo, vector HitLocation, vector Momentum)
{
	local int PreModifyDamage;
	local GearPawn GP;
	PreModifyDamage = Damage;
	Super.ModifyDamage(Victim,InstigatedBy,Damage,HitInfo,HitLocation,Momentum);
	GP = GearPawn(Victim);
	if (GP != None && GP.bLastHitWasHeadShot)
	{
		if (InstigatedBy == None || InstigatedBy.Pawn == None || VSize2D(InstigatedBy.Pawn.Location - GP.Location) > default.MaxGibDistance)
		{
			GP.bLastHitWasHeadShot = FALSE;
			Damage = PreModifyDamage;
		}
	}
}


/** Overridden to add a blood effect to the victim's camera */
static function HandleDamageFX(GearPawn DamagedPawn, const out TakeHitInfo HitInfo)
{
	local GearPC PC;
	Super.HandleDamageFX(DamagedPawn,HitInfo);
	PC = DamagedPawn.GetPC();
	if (PC != None && DamagedPawn.IsHumanControlled() && DamagedPawn.WorldInfo.GRI.ShouldShowGore() && HitInfo.InstigatedBy != None && VSize(HitInfo.InstigatedBy.Location - DamagedPawn.Location) <= 256.f)
	{
		PC.ClientSpawnCameraLensEffect( class'Emit_CameraBlood_ShotGun' );
	}
}

static function int GetChainsawInterruptValue(int Damage)
{
	// interrupt more if we hit them with enough pellets
	if (Damage > class'GearWeap_Shotgun'.default.WeaponDamage * 0.4)
	{
		return default.ChainsawInterruptValue * 2;
	}
	else
	{
		return default.ChainsawInterruptValue;
	}
}

defaultproperties
{
	ImpactTypeBallisticID=ITB_Gnasher
	KDamageImpulse=200
	KDeathUpKick=100
	KilledByIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=78,V=90,UL=77,VL=29)
	WeaponID=WC_Gnasher
}
