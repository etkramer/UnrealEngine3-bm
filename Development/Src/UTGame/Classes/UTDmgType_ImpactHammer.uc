/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTDmgType_ImpactHammer extends UTDamageType
	abstract;


/** Return the DeathCameraEffect that will be played on the instigator that was caused by this damagetype and the Pawn type (e.g. robot) */
simulated static function class<UTEmitCameraEffect> GetDeathCameraEffectInstigator( UTPawn UTP )
{
		// robots need to splatter oil instead of blood
		if( ( UTP != none ) && ( ClassIsChildOf( UTP.GetFamilyInfo(), class'UTFamilyInfo_Liandri' ) == TRUE ) )
		{
			return class'UTEmitCameraEffect_OilSplatter';
		}
		else
		{
			return default.DeathCameraEffectInstigator;
		}
}


defaultproperties
{
	RewardCount=15
	RewardEvent=REWARD_JACKHAMMER
	RewardAnnouncementSwitch=5
	bAlwaysGibs=true
	GibPerterbation=+0.5
	KillStatsName=KILLS_IMPACTHAMMER
	DeathStatsName=DEATHS_IMPACTHAMMER
	SuicideStatsName=SUICIDES_IMPACTHAMMER
	DamageWeaponClass=class'UTWeap_ImpactHammer'
	DamageWeaponFireMode=2
	VehicleDamageScaling=0.2
	VehicleMomentumScaling=+1.0
	KDamageImpulse=10000
	CustomTauntIndex=5

	DamageCameraAnim=CameraAnim'Camera_FX.ImpactHammer.C_WP_ImpactHammer_Primary_Fire_GetHit_Shake'
	DeathCameraEffectInstigator=class'UTEmitCameraEffect_BloodSplatter'
}
