/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_MiniGun extends GearWeap_Troika
	hidedropdown;

/*
simulated function LinearColor GetWeaponEmisColor_COG()
{
	return WorldInfo.Gears_COGWeapons.MiniGun;
}

simulated function LinearColor GetWeaponEmisColor_Locust()
{
	return WorldInfo.Gears_LocustWeapons.MiniGun;
}
*/

defaultproperties
{
	InstantHitMomentum(1)=1

	LC_EmisDefaultCOG=(R=5.0,G=5.0,B=5.0,A=1.0)
	LC_EmisDefaultLocust=(R=50.0,G=2.0,B=0.2,A=1.0)
}

