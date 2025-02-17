/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GDT_ReaverLegStrike extends GDT_Explosive;


/** Overridden to add a blood effect to the victim's camera */
static function HandleDamageFX(GearPawn DamagedPawn, const out TakeHitInfo HitInfo)
{
	local GearPC PC;
	Super.HandleDamageFX(DamagedPawn,HitInfo);
	PC = DamagedPawn.GetPC();
	if (PC != None && PC.IsLocalPlayerController() && DamagedPawn.WorldInfo.GRI.ShouldShowGore() && HitInfo.InstigatedBy != None && VSize(HitInfo.InstigatedBy.Location - DamagedPawn.Location) <= 256.f)
	{
		PC.ClientSpawnCameraLensEffect( class'Emit_CameraBlood_ShotGun' );
	}
}

defaultproperties
{
	bEnvironmentalDamage=FALSE
}