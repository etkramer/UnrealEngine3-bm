/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTDmgType_BioGoo_Charged extends UTDmgType_BioGoo
	abstract;

defaultproperties
{
	DamageWeaponFireMode=1
	DamageCameraAnim=CameraAnim'Camera_FX.BioRifle.C_WP_Bio_Alt_Hit'
    AlwaysGibDamageThreshold=99
	GibPerterbation=0.75
	CustomTauntIndex=11
}
