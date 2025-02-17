/** 
 * Spawnable ticker fog volume.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearFogVolume_Ticker extends GearFogVolume_SmokeGrenade;


defaultproperties
{
	FogVolumeArchetype=FogVolumeSphericalDensityInfo'Locust_Ticker.Effects.Explo_FOG'
	DamageTypeToUseForPerLevelMaterialEffects=class'GDT_TickerExplosion'
	DurationForMITV=6.0f
}
