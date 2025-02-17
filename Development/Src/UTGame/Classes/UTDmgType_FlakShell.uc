/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTDmgType_FlakShell extends UTDmgType_FlakShard
	abstract;


defaultproperties
{
	GibPerterbation=0.25
	bThrowRagdoll=true
	bKRadialImpulse=true
	KDamageImpulse=1000
	VehicleDamageScaling=0.8
	VehicleMomentumScaling=3.0

	DamageCameraAnim=CameraAnim'Camera_FX.Flak.C_WP_Flak_Alt_Hit_Shake'
}
