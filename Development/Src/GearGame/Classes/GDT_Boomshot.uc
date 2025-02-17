/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GDT_Boomshot extends GDT_Explosive;

defaultproperties
{
	ImpactTypeExplosionID=ITE_Boomshot
	KDamageImpulse=100
	RadialDamageImpulse=400
	bRadialDamageVelChange=TRUE
	KDeathUpKick=400
	KilledByIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=180,UL=77,VL=29)
	bEnvironmentalDamage=FALSE
	bSuppressPlayExplosiveRadialDamageEffects=TRUE
	WeaponID=WC_Boomshot
}
