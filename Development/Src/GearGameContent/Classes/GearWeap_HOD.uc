
/**
 * Hammer of Dawn marker
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_HOD extends GearWeap_HODBase
	dependson(HOD_BeamManager);

defaultproperties
{
	BeamManagerClass=class'HOD_BeamManager'

	TargetingBeamFeedbackTimeSec=0.75f
	TargetingBeamFireSound=SoundCue'Weapon_HammerDawn.Weapons.HODFire01Cue'
	TargetingBeamPosFeedbackSound=SoundCue'Weapon_HammerDawn.Weapons.HODTargetPositive01Cue'
	TargetingBeamNegFeedbackSound=SoundCue'Weapon_HammerDawn.Weapons.HODTargetNegative01Cue'
	TargetingBeamLockedFeedbackSound=SoundCue'Weapon_HammerDawn.Weapons.HODTargetLocked01Cue'

	Begin Object Name=TargetingBeamLoopSound0
		SoundCue=SoundCue'Weapon_HammerDawn.Weapons.HODFireBeam01Cue'
	End Object

	bBlindFirable=FALSE
	InstantHitDamageTypes(0)=class'GDT_HOD'
	WeaponFireTypes(0)=EWFT_Custom

	// Weapon Mesh
	Begin Object Name=WeaponMesh
		SkeletalMesh=SkeletalMesh'COG_HOD.COG_HammerofDawn'
		PhysicsAsset=PhysicsAsset'COG_HOD.COG_HammerofDawn_Physics'
	End Object

	// valid target effect
	PS_ValidTarget=ParticleSystem'COG_HOD.Effects.COG_HOD_Target_EndPoint'

	// muzzle flash emitter
	Begin Object Name=PSC_WeaponMuzzleMuzFlashComp
		Template=ParticleSystem'COG_HOD.Effects.COG_HOD_Muzzle_Flash'
	End Object

	// targeting beam emitter
	Begin Object Name=TBParticleSystemComponent0
		Template=ParticleSystem'COG_HOD.Effects.COG_HOD_Scope_Beam'
	End Object

	DamageTypeClassForUI=class'GDT_HOD'
	WeaponIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=367,V=193,UL=143,VL=46)
	AnnexWeaponIcon=(U=0,V=260,UL=128,VL=45)
	HUDDrawData			= (DisplayCount=106,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=494,UL=106,VL=7),ULPerAmmo=0)
	HUDDrawDataSuper	= (DisplayCount=106,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=494,UL=106,VL=7),ULPerAmmo=0)

	MeleeImpactSound=SoundCue'Weapon_Boomer.Reloads.BoomerHitCue'

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformShooting1
	    Samples(0)=(LeftAmplitude=40,RightAmplitude=40,LeftFunction=WF_LinearDecreasing,RightFunction=WF_LinearDecreasing,Duration=0.600)
	End Object
	WeaponFireWaveForm=ForceFeedbackWaveformShooting1

	bCanNegateMeatShield=true
	bIgnoresExecutionRules=true
}

