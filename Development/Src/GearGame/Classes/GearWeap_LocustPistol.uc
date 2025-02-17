/**
 * Locust Boltock pistol
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearWeap_LocustPistol extends GearWeap_PistolBase;

simulated function float GetWeaponRating()
{
	local GearAI_TDM AI;

	// always equip pistol when kidnapping
	if (GearAIController != None && GearAIController.MyGearPawn != None && GearAIController.MyGearPawn.IsAKidnapper())
	{
		return 10.0;
	}
	if (Instigator != None)
	{
		// more desireable around medium range
		AI = GearAI_TDM(Instigator.Controller);
		if ( AI != None && AI.Enemy != None && VSize(AI.GetEnemyLocation() - AI.Pawn.Location) < AI.EnemyDistance_Medium * 1.5 &&
			!AI.IsShortRange(AI.GetEnemyLocation()) )
		{
			return 1.0 + 0.3 * FRand();
		}
	}
	return Super.GetWeaponRating();
}

simulated function EjectClip()
{
	WeaponPlaySound(SoundCue'Weapon_Pistol.Reloads.CogPistolClipEjectCue');
}
simulated function CockOpen()
{
	WeaponPlaySound(SoundCue'Weapon_Pistol.Reloads.CogPistolCockOpenCue');
}
simulated function CockClose()
{
	WeaponPlaySound(SoundCue'Weapon_Pistol.Reloads.CogPistolCockCloseCue');
}

simulated function InsertClip()
{
	WeaponPlaySound(SoundCue'Weapon_Pistol.Reloads.CogPistolClipReloadInsertCue');
}

simulated function Jammed()
{
	WeaponPlaySound(SoundCue'Weapon_Pistol.Actions.CogPistolDropCue');
}

simulated function HandSlam()
{
	WeaponPlaySound(SoundCue'Weapon_AssaultRifle.Reloads.CogRifleJammedHandCue');
}
simulated function ClipImpact()
{
	WeaponPlaySound(SoundCue'Weapon_Pistol.Reloads.CogPistolClipEjectImpactCue');
}

defaultproperties
{
	AIRating=0.5

	FireAnimRateScale=2.5

	bWeaponCanBeReloaded=TRUE

	InstantHitDamageTypes(0)=class'GDT_LocustPistol'
	AmmoTypeClass=class'GearAmmoType_PistolHighPowered'

	FireSound=SoundCue'Weapon_Pistol.Boltok.BoltokPistolFireEnemyCue'
	FireSound_Player=SoundCue'Weapon_Pistol.Boltok.BoltokPistolFirePlayerCue'
	FireNoAmmoSound=SoundCue'Weapon_Pistol.Firing.LocustPistolFireEmptyCue'

	WeaponWhipSound=SoundCue'Weapon_Pistol.Firing.LocustPistolWhipCue'

	WeaponReloadSound=None
	TracerType=WTT_Boltok

	WeaponEquipSound=SoundCue'Weapon_Pistol.Actions.LocustPistolRaiseCue'
	WeaponDeEquipSound=SoundCue'Weapon_Pistol.Actions.LocustPistolLowerCue'
	PickupSound=SoundCue'Weapon_Pistol.Actions.LocustPistolPickupCue'
	WeaponDropSound=SoundCue'Weapon_Pistol.Actions.LocustPistolDropCue'

	// Weapon Mesh
	Begin Object Name=WeaponMesh
	    SkeletalMesh=SkeletalMesh'Locust_Pistol.Boltock_Pistol'
		PhysicsAsset=PhysicsAsset'Locust_Pistol.Boltock_Pistol_Physics'
	    AnimSets(0)=AnimSet'Locust_Pistol.animset_Boltock_Pistol'
		Animations=WeaponAnimNode
    End Object
	Mesh=WeaponMesh
	DroppedPickupMesh=WeaponMesh

	WeaponFireAnim=""
	WeaponReloadAnim="Reload"
	WeaponReloadAnimFail="Reload_Fail"

	CustomAnimSets.Add(AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_CamSkel_Boltok')

	PSC_ShellEject=none // locust pistol doesn't eject shells!

	// muzzle flash emitter
	Begin Object Name=PSC_WeaponMuzzleMuzFlashComp
		Template=ParticleSystem'Locust_Pistol.EffectS.P_Geist_Pistol_Muzzleflash'
	End Object
	MuzFlashEmitter=PSC_WeaponMuzzleMuzFlashComp

	// reload barrel smoke
	Begin Object Name=PSC_WeaponReloadBarrelSmokeComp
		Template=ParticleSystem'Locust_Pistol.Effects.P_Geist_Pistol_Smoke'
	End Object
	PSC_ReloadBarrelSmoke=PSC_WeaponReloadBarrelSmokeComp

	// Muzzle Flash point light
	Begin Object Name=WeaponMuzzleFlashLightComp
		LightColor=(R=255,G=112,B=48,A=255)
	End Object
	MuzzleFlashLight=WeaponMuzzleFlashLightComp
	MuzzleLightDuration=0.3f
	MuzzleLightPulseFreq=15
	MuzzleLightPulseExp=1.5

	Recoil_Hand={(
		LocAmplitude=(X=-15,Y=2,Z=9),
		LocFrequency=(X=8,Y=20,Z=15),
		LocParams=(X=ERS_Zero,Y=ERS_Random,Y=ERS_Zero),
		RotAmplitude=(X=10000,Y=500,Z=500),
		RotFrequency=(X=8,Y=20,Z=8),
		RotParams=(X=ERS_Zero,Y=ERS_Random,Y=ERS_Random),
		TimeDuration=0.45f,
	)}

	Recoil_Spine={(
		RotAmplitude=(X=1000,Y=000,Z=0),
		RotFrequency=(X=13,Y=10,Z=0),
		RotParams=(X=ERS_Zero,Y=ERS_Random,Y=ERS_Zero),
		TimeDuration=0.9f,
	)}

	bInstantHit=true
	bIsSuppressive=FALSE

	NeedReloadNotifyThreshold=2

	MuzFlashParticleSystem=ParticleSystem'Locust_Pistol.EffectS.P_Geist_Pistol_Muzzleflash'
	MuzFlashParticleSystemActiveReload=ParticleSystem'Locust_Pistol.Effects.P_Geist_Pistol_Muzzleflash_AR'

	DamageTypeClassForUI=class'GDT_LocustPistol'
	WeaponIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=367,V=99,UL=143,VL=46)
	AnnexWeaponIcon=(U=0,V=79,UL=128,VL=40)
	CrosshairIcon=Texture2D'Warfare_HUD.HUD.HUD_Xhair_COG_Snub'
	HUDDrawData			= (DisplayCount=6,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=281,UL=106,VL=7),ULPerAmmo=17)
	HUDDrawDataSuper	= (DisplayCount=6,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=281,UL=106,VL=7),ULPerAmmo=17)

	MeleeImpactSound=SoundCue'Weapon_TorqueBow.Weapons.Crossbow_HitCue';

	LC_EmisDefaultCOG=(R=0.0,G=2.0,B=25.0,A=1.0)
	LC_EmisDefaultLocust=(R=15.0,G=2.0,B=0.0,A=1.0)

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformShooting1
		Samples(0)=(LeftAmplitude=50,RightAmplitude=80,LeftFunction=WF_LinearDecreasing,RightFunction=WF_LinearDecreasing,Duration=0.600)
	End Object
	WeaponFireWaveForm=ForceFeedbackWaveformShooting1

	WeaponID=WC_Boltock

	// CQC Long Executions
	CQC_Long_KillerAnim="CTRL_Handgun_A"
	CQC_Long_VictimAnim="DBNO_Handgun_A"
	CQC_Long_CameraAnims.Empty
	CQC_Long_CameraAnims(0)=CameraAnim'COG_MarcusFenix.Camera_Anims.DBNO_Handgun_A_Cam01'
	CQC_Long_CameraAnims(1)=CameraAnim'COG_MarcusFenix.Camera_Anims.DBNO_Handgun_A_Cam02'
	CQC_Long_CameraAnims(2)=CameraAnim'COG_MarcusFenix.Camera_Anims.DBNO_Handgun_A_Cam03'
	CQC_Long_bHolsterWeapon=FALSE
	CQC_Long_VictimDeathTime=1.36f
	CQC_Long_VictimRotStartTime=0.f
	CQC_Long_VictimRotInterpSpeed=0.f
	CQC_Long_EffortID=GearEffort_BoltokLongExecutionEffort
	CQC_Long_DamageType=class'GDT_LongExecution_Boltok'

	BS_PawnWeaponReload={(
		AnimName[BS_Hostage_Upper]		="Hostage_Pistol_reload",
		AnimName[BS_Kidnapper_Upper]	="Kidnapper_Pistol_reload"
	)}

	BS_PawnWeaponReloadFail={(
		AnimName[BS_Hostage_Upper]		="Hostage_Pistol_reload_fail",
		AnimName[BS_Kidnapper_Upper]	="Kidnapper_Pistol_reload_fail"
	)}
}
