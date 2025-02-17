/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GDT_RocketCannon extends GDT_Explosive;

defaultproperties
{
	ImpactTypeExplosionID=ITE_CentaurCannon
	KDamageImpulse=100
	RadialDamageImpulse=400
	bRadialDamageVelChange=TRUE
	KDeathUpKick=400
	bEnvironmentalDamage=FALSE
	KilledByIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=180,UL=77,VL=29)
}
