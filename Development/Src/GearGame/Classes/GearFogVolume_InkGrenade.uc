/** 
 * Spawnable ink grenade fog volume.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearFogVolume_InkGrenade extends GearFogVolume_SmokeGrenade;



defaultproperties
{
	FogVolumeArchetype=FogVolumeSphericalDensityInfo'Effects_FogVolumes.FogVolumes.FV_Spherical_InkGrenade'
	DamageTypeToUseForPerLevelMaterialEffects=class'GDT_InkGrenade'
}
