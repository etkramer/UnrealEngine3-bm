/**
 * Flamethrower spray!
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class FlameThrowerSpray extends FlameThrowerSprayBase;

defaultproperties
{
	//@fixme, find out why mesh is becoming detached from gun occasionally when in PHYS_None!
	Physics=PHYS_Interpolating
//	Physics=PHYS_None
	TickGroup=TG_PostAsyncWork

	bStatic=false
	bCollideActors=TRUE
	bBlockActors=false
	bWorldGeometry=false
	bCollideWorld=TRUE
	bProjTarget=false
	bIgnoreEncroachers=FALSE
	bNoEncroachCheck=TRUE

	// client handles his own
	RemoteRole=ROLE_None

	Begin Object Class=SkeletalMeshComponent Name=FlameCore0
		CollideActors=true
		BlockActors=false
		BlockZeroExtent=true
		BlockNonZeroExtent=true
		bUseAsOccluder=FALSE
		bUpdateSkelWhenNotRendered=TRUE
		bIgnoreControllersWhenNotRendered=FALSE		
		CastShadow=FALSE
		bAcceptsStaticDecals=FALSE
		bAcceptsDynamicDecals=FALSE
		bAcceptsFoliage=FALSE

//		HiddenGame=TRUE

		Rotation=(Roll=-16384)
		SkeletalMesh=SkeletalMesh'COG_Flamethrower.Mesh.SK_Flamethrower_FireWhip'
		AnimTreeTemplate=AnimTree'COG_Flamethrower.Animations.AT_Flamethrower_FireWhip'
		AnimSets(0)=AnimSet'COG_Flamethrower.Animations.COG_Flamethrower_FireWhip_Anims'
	End Object
	SkeletalSprayMesh=FlameCore0
	CollisionComponent=FlameCore0
	Components.Add(FlameCore0)

	GravityScaleRange=(X=0.f,Y=-15.f)
	GravityScaleInTime=0.5f

	MomentumScale=0.15f
	MyDamageType=class'GDT_FlamethrowerSpray'

	FireSprayStartSound=SoundCue'Weapon_FlameThrower.FlameThrower.FlameThrower_Fire01Cue'
	FireSprayStopSound=SoundCue'Weapon_FlameThrower.FlameThrower.FlameThrower_FireStop01Cue'
	FireSprayLoopSound=SoundCue'Weapon_FlameThrower.FlameThrower.FlameThrower_FireLoop01Cue'

	SprayAudioAdjInterpSpeed=20.f
	CurrentSprayPitchMult=1.f
	CurrentSprayVolumeMult=1.f
	FireAudioAdj_RotVelRange=(X=0.f,Y=200.f)				// degrees/sec
	FireAudioAdj_PitchRange=(X=0.8f,Y=1.f)
	FireAudioAdj_VolumeRange=(X=0.f,Y=3.f)

	Begin Object Class=ParticleSystemComponent Name=SplashGlancingPSC0
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_Firewhip_Impact'
	End Object
	SplashGlancingPSC=SplashGlancingPSC0
	Components.Add(SplashGlancingPSC0)

	Begin Object Class=ParticleSystemComponent Name=SplashDirectPSC0
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_Firewhip_Impact_Flat'
	End Object
	SplashDirectPSC=SplashDirectPSC0
	Components.Add(SplashDirectPSC0)

	Begin Object Class=ParticleSystemComponent Name=SplashPawnPSC0
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_Firewhip_Impact_Player'
	End Object
	SplashPawnPSC=SplashPawnPSC0
	Components.Add(SplashPawnPSC0)

	Begin Object Class=ParticleSystemComponent Name=SplashMaterialBasedPSC0
		bAutoActivate=FALSE
		Template=None		// filled in by the code based on what was hit
	End Object
	SplashMaterialBasedPSC=SplashMaterialBasedPSC0
	Components.Add(SplashMaterialBasedPSC0)

	// splash vars
	SplashGlancingDotLimit=-0.9f
	SplashRotInterpSpeed=8.f
	SplashLocInterpSpeed=40.f

	PruneSkelControlName=FlamePrune

	// per-bone fire fx
	PerBoneFireFXGlobalScale=1.f
	bDoPerBoneFireFX=TRUE
	LastBoneChainIndexThatCanSpawnSplashEffects=11
	//PerBoneFireFXActivationDelayRange=(X=0.f,Y=0.08f)
	//PerBoneFireFXActivationDist=160.f



	// bone01
	Begin Object Class=ParticleSystemComponent Name=Bone01_PSC0
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_beam_Spawn'
		Rotation=(Yaw=32768)
	End Object
	// bone02
	Begin Object Class=ParticleSystemComponent Name=Bone02_PSC0
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_beam_Spawn'
		Rotation=(Yaw=32768)
	End Object
	// bone03
	Begin Object Class=ParticleSystemComponent Name=Bone03_PSC0
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_beam_Spawn'
		Rotation=(Yaw=32768)
	End Object
	// bone04
	Begin Object Class=ParticleSystemComponent Name=Bone04_PSC0
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_beam_Spawn'
		Rotation=(Yaw=32768)
	End Object
	// bone05
	Begin Object Class=ParticleSystemComponent Name=Bone05_PSC0
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_beam_Spawn'
		Rotation=(Yaw=32768)
	End Object

	// bone06
	Begin Object Class=ParticleSystemComponent Name=Bone06_PSC0
		bOwnerNoSee=TRUE		// 3rd person
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_Firewhip_BoneSpawns'
	End Object
	Begin Object Class=ParticleSystemComponent Name=Bone06_PSC1
		bOnlyOwnerSee=TRUE		// first person
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_Firewhip_BoneSpawns_Player'
	End Object

	// bone07
	Begin Object Class=ParticleSystemComponent Name=Bone07_PSC0
		bOwnerNoSee=TRUE		// 3rd person
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_Firewhip_BoneSpawns'
	End Object
	Begin Object Class=ParticleSystemComponent Name=Bone07_PSC1
		bOnlyOwnerSee=TRUE		// first person
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_Firewhip_BoneSpawns_Player'
	End Object

	// bone08
	Begin Object Class=ParticleSystemComponent Name=Bone08_PSC0
		bOwnerNoSee=TRUE		// 3rd person
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_Firewhip_BoneSpawns'
	End Object
	Begin Object Class=ParticleSystemComponent Name=Bone08_PSC1
		bOnlyOwnerSee=TRUE		// first person
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_Firewhip_BoneSpawns_Player'
	End Object

	// bone09
	Begin Object Class=ParticleSystemComponent Name=Bone09_PSC0
		bOwnerNoSee=TRUE		// 3rd person
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_Firewhip_BoneSpawns'
	End Object
	Begin Object Class=ParticleSystemComponent Name=Bone09_PSC1
		bOnlyOwnerSee=TRUE		// first person
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_Firewhip_BoneSpawns_Player'
	End Object

	// bone10
	Begin Object Class=ParticleSystemComponent Name=Bone10_PSC0
		bOwnerNoSee=TRUE		// 3rd person
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_Firewhip_BoneSpawns'
	End Object
	Begin Object Class=ParticleSystemComponent Name=Bone10_PSC1
		bOnlyOwnerSee=TRUE		// first person
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_Firewhip_BoneSpawns_Player'
	End Object

	// bone11
	Begin Object Class=ParticleSystemComponent Name=Bone11_PSC0
		bOwnerNoSee=TRUE		// 3rd person
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_Firewhip_BoneSpawns'
	End Object
	Begin Object Class=ParticleSystemComponent Name=Bone11_PSC1
		bOnlyOwnerSee=TRUE		// first person
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_Firewhip_BoneSpawns_Player'
	End Object

	// bone12
	Begin Object Class=ParticleSystemComponent Name=Bone12_PSC0
		bOwnerNoSee=TRUE		// 3rd person
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_Firewhip_BoneSpawns'
	End Object
	Begin Object Class=ParticleSystemComponent Name=Bone12_PSC1
		bOnlyOwnerSee=TRUE		// first person
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_Firewhip_BoneSpawns_Player'
	End Object

	// bone13
	Begin Object Class=ParticleSystemComponent Name=Bone13_PSC0
		bOwnerNoSee=TRUE		// 3rd person
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_Firewhip_BoneSpawns'
	End Object
	Begin Object Class=ParticleSystemComponent Name=Bone13_PSC1
		bOnlyOwnerSee=TRUE		// first person
		bAutoActivate=FALSE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_Firewhip_BoneSpawns_Player'
	End Object

	BoneChain(0)=(BoneName=Bone01)
	BoneChain(1)=(BoneName=Bone02,ParticleActivationDelay=0.0f,EffectScale=0.4f,BonePSC0=Bone01_PSC0)
	BoneChain(2)=(BoneName=Bone03,ParticleActivationDelay=0.0f,EffectScale=0.6f,BonePSC0=Bone02_PSC0)
	BoneChain(3)=(BoneName=Bone04,ParticleActivationDelay=0.0f,EffectScale=0.8f,BonePSC0=Bone03_PSC0)
	BoneChain(4)=(BoneName=Bone05,ParticleActivationDelay=0.0f,EffectScale=1.0f,BonePSC0=Bone04_PSC0)
	BoneChain(5)=(BoneName=Bone06,ParticleActivationDelay=0.0f,EffectScale=1.2f,BonePSC0=Bone05_PSC0)
	BoneChain(6)=(BoneName=Bone07,ParticleActivationDelay=0.0f,EffectScale=1.8f,BonePSC0=Bone06_PSC0,BonePSC1=Bone06_PSC1)
	BoneChain(7)=(BoneName=Bone08,ParticleActivationDelay=0.0f,EffectScale=2.0f,BonePSC0=Bone07_PSC0,BonePSC1=Bone07_PSC1)
	BoneChain(8)=(BoneName=Bone09,ParticleActivationDelay=0.0f,EffectScale=2.3f,BonePSC0=Bone08_PSC0,BonePSC1=Bone08_PSC1)
	BoneChain(9)=(BoneName=Bone10,ParticleActivationDelay=0.0f,EffectScale=2.7f,BonePSC0=Bone09_PSC0,BonePSC1=Bone09_PSC1)
	BoneChain(10)=(BoneName=Bone11,ParticleActivationDelay=0.f,EffectScale=3.0f,BonePSC0=Bone10_PSC0,BonePSC1=Bone10_PSC1)
	BoneChain(11)=(BoneName=Bone12,ParticleActivationDelay=0.f,EffectScale=3.3f,BonePSC0=Bone11_PSC0,BonePSC1=Bone11_PSC1)
	BoneChain(12)=(BoneName=Bone13,ParticleActivationDelay=0.f,EffectScale=3.7f,BonePSC0=Bone12_PSC0,BonePSC1=Bone12_PSC1)
	BoneChain(13)=(BoneName=Bone14,ParticleActivationDelay=0.f,EffectScale=4.0f,BonePSC0=Bone13_PSC0,BonePSC1=Bone13_PSC1)
	BoneChain(14)=(BoneName=Bone15)


	// pointlight at far end of spray
	Begin Object Class=PointLightComponent Name=FlamePointLight2
	    LightColor=(R=245,G=190,B=140,A=255)
		Brightness=4.f
		Radius=450.f
		CastShadows=FALSE
		CastStaticShadows=FALSE
		CastDynamicShadows=FALSE
		bForceDynamicLight=FALSE
		bCastCompositeShadow=FALSE
		bEnabled=FALSE
	End Object

	// Muzzle Flash point light
	// want this light to illuminate characters only, so Marcus gets the glow
    Begin Object Class=PointLightComponent Name=FlamePointLight0
		LightColor=(R=250,G=150,B=85,A=255)
		Brightness=6.0f
		FalloffExponent=1.f
		Radius=150.f
		CastShadows=FALSE
		CastStaticShadows=FALSE
		CastDynamicShadows=FALSE
		bForceDynamicLight=FALSE
		bCastCompositeShadow=FALSE
		bEnabled=FALSE
		LightingChannels=(BSP=FALSE,Static=FALSE,Dynamic=FALSE,CompositeDynamic=TRUE)
	End Object

	FlameLights(0)=(Light=FlamePointLight0,BoneChainIndex=0,FlickerIntensity=5.f,FlickerInterpSpeed=15.f)
	FlameLights(1)=(Light=FlamePointLight2,BoneChainIndex=9,FlickerIntensity=5.f,FlickerInterpSpeed=15.f)

	Begin Object Class=ParticleSystemComponent Name=OwnerGlow0
		bAutoActivate=FALSE
		bOnlyOwnerSee=TRUE		// only the flame owner gets to see this effect
		Template=ParticleSystem'COG_Flamethrower.Effects.P_Fire_Glow'
		Scale=0.f				// starts at zero, ramps in
	End Object
	PSC_OwnerGlow=OwnerGlow0
	OwnerGlowBoneChainIndex=12
	OwnerGlowScaleInTime=0.5f

	MaterialHeatRampTime=0.65f
	MaterialHeatRange=(X=0.f,Y=0.8f)
	MaterialFadeOutTime=0.2f
	MaterialCurrentFadeVal=1.f
	MatFadePow=2.f

//	EndFireParticleSystem=ParticleSystem'COG_Flamethrower.Effects.P_Firewhip_End_Spawn'
//	EndFireParticleSystem=ParticleSystem'COG_Flamethrower.Effects.P_FireWhip_Init'
//	StartFireParticleSystem=ParticleSystem'COG_Flamethrower.Effects.P_FireWhip_Init'

	Begin Object Class=ParticleSystemComponent Name=StartFirePSC0
		bAutoActivate=TRUE
		Template=ParticleSystem'COG_Flamethrower.Effects.P_FireWhip_Init'
	End Object
	StartFirePSC=StartFirePSC0


	//	bDebugShowBones=TRUE
	//	bHidden=TRUE
	bTestCollideComplex=TRUE

	// Splash audio
	Begin Object Class=AudioComponent Name=SplashPawnAC0
		SoundCue=SoundCue'Weapon_FlameThrower.FlameThrower.FlameThrower_ImpactFleshLoop01Cue'
		bStopWhenOwnerDestroyed=TRUE
		bUseOwnerLocation=FALSE
	End Object
	SplashPawnAC=SplashPawnAC0

	Begin Object Class=AudioComponent Name=SplashDirectAC0
		SoundCue=SoundCue'Weapon_FlameThrower.FlameThrower.FlameThrower_ImpactSmoulderLoop05Cue'
		bStopWhenOwnerDestroyed=TRUE
		bUseOwnerLocation=FALSE
	End Object
	SplashDirectAC=SplashDirectAC0

	Begin Object Class=AudioComponent Name=SplashGlancingAC0
		SoundCue=SoundCue'Weapon_FlameThrower.FlameThrower.FlameThrower_ImpactSmoulderLoop04Cue'
		bStopWhenOwnerDestroyed=TRUE
		bUseOwnerLocation=FALSE
	End Object
	SplashGlancingAC=SplashGlancingAC0

	Begin Object Class=AudioComponent Name=SplashMaterialBasedAC0
		// cue will be filled in from physicalmaterial data
		SoundCue=None
		bStopWhenOwnerDestroyed=TRUE
		bUseOwnerLocation=FALSE
	End Object
	SplashMaterialBasedAC=SplashMaterialBasedAC0

}




