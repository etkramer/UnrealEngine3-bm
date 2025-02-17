/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GDT_TorqueBow_Explosion extends GDT_Explosive;

defaultproperties
{
	ImpactTypeExplosionID=ITE_TorqueBowArrow
	KilledByIcon=(Texture=Texture2D'Warfare_HUD.HUD.Gears_DeathIcons',U=156,V=30,UL=77,VL=29)
	bEnvironmentalDamage=FALSE
	bSuppressPlayExplosiveRadialDamageEffects=TRUE
	WeaponID=WC_TorqueBow
}
