/**
 * this is used for the extra damage over time applied due to skin heat from the flamethrower
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GDT_FireDOT extends GDT_Fire;

defaultproperties
{
	ImpactTypeBallisticID=ITB_Scorcher
	KillGUDEvent=GUDEvent_KilledEnemyFlamethrower

	KilledByIcon=(Texture=Texture2D'Warfare_HUD.HUD.Gears_DeathIcons',U=0,V=30,UL=77,VL=29)

	bSuppressImpactFX=TRUE
	bSuppressBloodDecals=TRUE

	bHeatSkin=FALSE

	bEnvironmentalDamage=FALSE
	WeaponID=WC_Scorcher
}
