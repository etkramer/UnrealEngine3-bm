/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTDmgType_SniperPrimary extends UTDamageType
	abstract;

static function float VehicleDamageScalingFor(Vehicle V)
{
	if ( (UTVehicle(V) != None) && UTVehicle(V).bLightArmor )
		return 1.5 * Default.VehicleDamageScaling;

	return Default.VehicleDamageScaling;
}

defaultproperties
{
	KillStatsName=KILLS_SNIPERRIFLE
	DeathStatsName=DEATHS_SNIPERRIFLE
	SuicideStatsName=SUICIDES_SNIPERRIFLE
	DamageWeaponClass=class'UTWeap_SniperRifle'
	DamageWeaponFireMode=0
	GibPerterbation=0.25
	VehicleDamageScaling=0.4
	NodeDamageScaling=0.4
	bNeverGibs=true
	CustomTauntIndex=2
}
