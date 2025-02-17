/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Vehicle_RideReaver extends Vehicle_RideReaver_Base;

var array<ForceFeedbackWaveform> WeaponFiredFFs;

simulated function VehicleWeaponFireEffects(vector HitLocation, int SeatIndex)
{
	local GearPC PC;
	Super.VehicleWeaponFireEffects(HitLocation,SeatIndex);
	PC = GearPC(Seats[SeatIndex].SeatPawn.Controller);
	if (PC != None && PC.IsLocalPlayerController())
	{
		PC.ClientPlayForceFeedbackWaveform(WeaponFiredFFs[SeatIndex]);
	}
}

simulated function SitDriver( GearPawn WP, int SeatIndex)
{
	Super.SitDriver(WP, SeatIndex);

	if(GearPawn_COGDom(WP) != None)
	{
		DriverComp.SetSkeletalMesh(SkeletalMesh'COG_Baird.COG_Baird_Goggles');
		DriverComp.SetPhysicsAsset(PhysicsAsset'COG_Baird.COG_Baird_Physics');
	}
}

defaultproperties
{
	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformShooting0
		Samples(0)=(LeftAmplitude=100,RightAmplitude=100,LeftFunction=WF_LinearDecreasing,RightFunction=WF_LinearDecreasing,Duration=0.300)
	End Object
	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformShooting1
		Samples(0)=(LeftAmplitude=60,RightAmplitude=60,LeftFunction=WF_LinearDecreasing,RightFunction=WF_LinearDecreasing,Duration=0.200)
	End Object
	WeaponFiredFFs(0)=ForceFeedbackWaveformShooting0
	WeaponFiredFFs(1)=ForceFeedbackWaveformShooting1

	Seats(0)={(	GunClass=class'GearGameContent.GearVWeap_RideReaverCannon',
		GunSocket=("RideCannonLight"),
		GunPivotPoints=(b_Gun_Front),
		TurretControls=(RideCannon_Aim),
		bSeatVisible=true,
		SeatRotation=(Yaw=16384),
		SeatOffset=(Z=0.0f),
		SeatBone="b_Main",
		CameraTag="b_Main",
		CameraBaseOffset=(X=0,Y=0,Z=200),
		CameraViewOffsetHigh=(X=-80,Y=0,Z=120),
		CameraViewOffsetMid=(X=-800,Y=0,Z=40),
		CameraViewOffsetLow=(X=-800,Y=0,Z=150),
		WorstCameraLocOffset=(X=0,Y=0,Z=0),
		bDisableOffsetZAdjust=TRUE,
		DriverDamageMult=1.0
		)}

	Seats(1)={(	GunClass=class'GearGameContent.GearVWeap_ReaverMinigun',
		GunSocket=("RideMinigunLight"),
		GunPivotPoints=(b_Gun_Rear),
		TurretControls=(RideMinigun_Aim),
		bSeatVisible=true,
		SeatRotation=(Yaw=16384),
		SeatOffset=(Z=0.0f),
		SeatBone="b_Main",
		CameraTag="b_Main",
		CameraBaseOffset=(X=-170,Y=0,Z=200),
		CameraViewOffsetHigh=(X=-80,Y=0,Z=120),
		CameraViewOffsetMid=(X=-800,Y=0,Z=40),
		CameraViewOffsetLow=(X=-800,Y=0,Z=150),
		WorstCameraLocOffset=(X=0,Y=0,Z=0),
		bDisableOffsetZAdjust=TRUE,
		DriverDamageMult=1.0,
		TurretVarPrefix="Turret"
		)}

	ViewPitchMax=8192
	ViewPitchMin=-8192
	CameraLag=0.05

	DefaultRideReaverHealth=1000.0
	bDrawHealthOnHUD=TRUE
	bAlwaysViewFriction=TRUE
	bShowWeaponOnHUD=FALSE

	DodgeSpeed=3.0
	MaxDodgeAmount=512.0
	DodgeRotationAmount=-0.0002

	TentacleTrailRelaxation=40.0

	Begin Object Name=SVehicleMesh
		SkeletalMesh=SkeletalMesh'Locust_Reaver.ReaverTest'
		PhysicsAsset=PhysicsAsset'Locust_Reaver_Anim.ReaverTest_Phys'
		AnimTreeTemplate=AnimTree'Locust_Reaver_Anim.ReaverTestAnimTree'
		AnimSets(0)=AnimSet'Locust_Reaver.ReaverTestAnims'
		AnimSets(1)=AnimSet'Locust_Reaver.Locust_Reaver_Flying'
		AnimSets(2)=AnimSet'Locust_Reaver.Locust_Reaver_Overlay_Head'
		RBCollideWithChannels=(GameplayPhysics=TRUE,EffectPhysics=TRUE)
		LightEnvironment=MyLightEnvironment
		bCastDynamicShadow=TRUE
		bAllowAmbientOcclusion=FALSE
		bSelfShadowOnly=TRUE
		bNotifyRigidBodyCollision=FALSE // not needed for this
	End Object

	GorePhysicsAsset=PhysicsAsset'Locust_Reaver.Locust_Reaver_Gore_Physics'
	GoreSkeletalMesh=SkeletalMesh'Locust_Reaver.Locust_Reaver_LOD1_Gore'
	GoreExplosionRadius=500.0
	GoreExplosionVel=1000.0
	ReaverDeathSound=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_DeathCue'
	ReaverExplodeEffect=ParticleSystem'Locust_Reaver.Effects.P_Reaver_Gib_Explosion_Main'
	ReaverExplodeNoGoreEffect=ParticleSystem'Locust_Reaver.Effects.P_Reaver_Gib_Explosion_Main_NOGORE'

	Begin Object Name=DriverSkel
		SkeletalMesh=SkeletalMesh'COG_Gus.COG_Gus'
		PhysicsAsset=PhysicsAsset'COG_Gus.COG_Gus_Physics'
		AnimTreeTemplate=AnimTree'Locust_Reaver_Anim.AT_Locust_ReaverDriver'
		AnimSets(0)=AnimSet'Locust_Grunt.Locust_Grunt_OnReaver'
		bAllowAmbientOcclusion=FALSE
		bSelfShadowOnly=TRUE
	End Object

	Begin Object Name=TurretSkel
		SkeletalMesh=SkeletalMesh'Locust_Reaver.Locust_Reaver_Turrets'
		bAllowAmbientOcclusion=FALSE
		bSelfShadowOnly=TRUE
	End Object

	Begin Object Class=LightFunction Name=LightFunction_0
		SourceMaterial=Material'MS_Escape.Materials.Light_Functions.Flashlight_LFM'
	End Object

	Begin Object Name=FrontSpot
		LightAffectsClassification=LAC_DYNAMIC_AND_STATIC_AFFECTING
		CastShadows=TRUE
		CastStaticShadows=TRUE
		CastDynamicShadows=FALSE
		bForceDynamicLight=FALSE
		UseDirectLightMap=FALSE
		LightingChannels=(BSP=TRUE,Static=TRUE,Dynamic=TRUE,bInitialized=TRUE)
		OuterConeAngle=20.000000
		Radius=5000.000000
		FalloffExponent=0.100000
		Brightness=30.000000
		LightColor=(B=147,G=200,R=215,A=0)
		Function=LightFunction'LightFunction_0'
		LightEnv_BouncedLightBrightness=0.114913
		LightEnv_BouncedModulationColor=(B=52,G=239,R=255,A=221)
	End Object

	Begin Object Class=LightFunction Name=LightFunction_1
		SourceMaterial=Material'MS_Escape.Materials.Light_Functions.Flashlight_LFM'
	End Object

	Begin Object Name=RearSpot
		LightAffectsClassification=LAC_DYNAMIC_AND_STATIC_AFFECTING
		CastShadows=TRUE
		CastStaticShadows=TRUE
		CastDynamicShadows=FALSE
		bForceDynamicLight=FALSE
		UseDirectLightMap=FALSE
		LightingChannels=(BSP=TRUE,Static=TRUE,Dynamic=TRUE,bInitialized=TRUE)
		OuterConeAngle=20.000000
		Radius=5000.000000
		FalloffExponent=0.100000
		Brightness=30.000000
		LightColor=(B=147,G=200,R=215,A=0)
		Function=LightFunction'LightFunction_1'
		LightEnv_BouncedLightBrightness=0.114913
		LightEnv_BouncedModulationColor=(B=52,G=239,R=255,A=221)
	End Object

	Begin Object Name=GlowLight
		//LightAffectsClassification=LAC_DYNAMIC_AND_STATIC_AFFECTING
		//CastShadows=FALSE
		//CastStaticShadows=FALSE
		//CastDynamicShadows=FALSE
		//bForceDynamicLight=FALSE
		//UseDirectLightMap=FALSE
		//LightingChannels=(BSP=TRUE,Static=TRUE,Dynamic=TRUE,bInitialized=TRUE)
		Radius=2000.000000
		Brightness=2.000000
		LightColor=(B=234,G=198,R=121,A=0)
		LightEnv_BouncedLightBrightness=0.141115
		LightEnv_BouncedModulationColor=(B=255,G=232,R=0,A=180)
	End Object

	// rocket muzzle light
	Begin Object Class=PointLightComponent Name=MuzzleLight0
		bEnabled=FALSE
		LightColor=(B=35,G=185,R=255,A=255)
		Brightness=10
		Radius=256
	End Object
	RocketMuzzleLight=MuzzleLight0
	RocketMuzzleLightTime=0.4f
	RocketMuzzleEffect=ParticleSystem'Locust_Reaver.Effects.P_Locust_Reaver_Muzzleflash_Cannon'

	// Minigun muzzle light
	Begin Object Class=PointLightComponent Name=MuzzleLight1
		bEnabled=FALSE
		LightColor=(B=35,G=185,R=255,A=255)
		Brightness=10
		Radius=256
	End Object
	MinigunMuzzleLight=MuzzleLight1
	MinigunMuzzleLightTime=0.4f
	MinigunMuzzleEffect=ParticleSystem'Locust_Reaver.Effects.P_Locust_Reaver_Muzzleflash_Gun'

	Physics=PHYS_Interpolating

	GunnerAnimTree=AnimTree'Locust_Reaver_Anim.AT_RideReaverGunner'
	GunnerAnimSets.Add(AnimSet'COG_MarcusFenix.Animations.Reaver_Riding')

	PS_StrikeTemplate=ParticleSystem'Locust_Reaver.Particles.P_Reaver_Ground_Hit'

	FlyingSoundLoop=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_FlyLoop03Cue'
	FlyingSoundLoop_Player=SoundCue'Locust_Reaver_Efforts.ReaverPlayer.ReaverPlayer_FlyLoopCue'

	RocketFireSound=SoundCue'Weapon_Boomer.Firing.BoomerFireCue'
	RocketFireSound_Player=SoundCue'Locust_Reaver_Efforts.ReaverPlayer.ReaverPlayer_RocketFireCue'

	MinigunFireLoop=SoundCue'Weapon_Troika.TroikaV2.Troika_FirePlayerLoopReaverCue'		// @fixme, needs spatializable loop
	MinigunFireLoop_Player=SoundCue'Weapon_Troika.TroikaV2.Troika_FirePlayerLoopReaverCue'
	MinigunFireStopCue=SoundCue'Weapon_Troika.TroikaV2.Troika_FireStopEnemyCue'			// @fixme, temp sound
	MinigunFireStopCue_Player=SoundCue'Weapon_Troika.TroikaV2.Troika_FireStopPlayerCue'	// @fixme, temp sound
	HeatBuildupCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingFireHeatBuildUpCue'

	ReaverHowlSound=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_HowlCue'
	ReaverLandSound=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_FlyLandCue'
	ReaverLandSound_Player=SoundCue'Locust_Reaver_Efforts.ReaverPlayer.ReaverPlayer_FlyLandCue'
	ReaverTakeOffSound=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_FlyAwayCue'
	ReaverTakeOffSound_Player=SoundCue'Locust_Reaver_Efforts.ReaverPlayer.ReaverPlayer_FlyAwayCue'
	ReaverScreamSound=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_ScreamACue'
	ReaverPainSound=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_VocalPainCue'
	ReaverMovementCue=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_FlyMovementCue'
	ReaverDodgeCue_Player=SoundCue'Locust_Reaver_Efforts.ReaverPlayer.ReaverPlayer_FlyDodgeCue'
	ReaverLeanCue_Player=SoundCue'Locust_Reaver_Efforts.ReaverPlayer.ReaverPlayer_FlyLeanCue'

//	RandomReaverGrowls(0)=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_AmbientGrowlACue'
//	RandomReaverGrowls(1)=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_AmbientGrowlBCue'
//	RandomReaverGrowls(2)=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_SniffingCue'
//	RandomReaverGrowls(3)=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_BreathingCue'

	OverlayEffect=PostProcessChain'War_PostProcess.ReaverOverlayPostProcess'
	OverlaySpreadFactor=-0.15
	OverlayEffectFactor=0.5
}


