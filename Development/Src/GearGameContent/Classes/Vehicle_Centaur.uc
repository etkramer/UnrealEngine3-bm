/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Vehicle_Centaur extends Vehicle_Centaur_Base;

var ForceFeedbackWaveform WeaponFiredFF, BoostFF, IdleFF, SquishedFF;

simulated function VehicleWeaponFireEffects(vector HitLocation, int SeatIndex)
{
	local GearPC PC;
	PC = GearPC(Seats[SeatIndex].SeatPawn.Controller);
	if (PC != None && PC.IsLocalPlayerController())
	{
		PC.ClientPlayForceFeedbackWaveform(WeaponFiredFF);
	}
	Super.VehicleWeaponFireEffects(HitLocation,SeatIndex);
}

event SquishedSomething()
{
	local int SeatIndex;
	local GearPC PC;
	for (SeatIndex = 0; SeatIndex < Seats.Length; SeatIndex++)
	{
		PC = GearPC(Seats[SeatIndex].SeatPawn.Controller);
		if (PC != None && PC.IsLocalPlayerController())
		{
			PC.ClientPlayForceFeedbackWaveform(SquishedFF);
		}
	}
}

simulated event StartBoost()
{
	local GearPC PC;
	local int SeatIndex;
	for (SeatIndex = 0; SeatIndex < Seats.Length; SeatIndex++)
	{
		PC = GearPC(Seats[SeatIndex].SeatPawn.Controller);
		if (PC != None && PC.IsLocalPlayerController())
		{
			PC.ClientPlayForceFeedbackWaveform(BoostFF);
		}
	}
	Super.StartBoost();
}

simulated event EndBoost()
{
	local GearPC PC;
	local int SeatIndex;
	for (SeatIndex = 0; SeatIndex < Seats.Length; SeatIndex++)
	{
		PC = GearPC(Seats[SeatIndex].SeatPawn.Controller);
		if (PC != None && PC.IsLocalPlayerController())
		{
			PC.ClientStopForceFeedbackWaveform(BoostFF);
		}
	}
	Super.EndBoost();
}

defaultproperties
{
	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformShooting0
		Samples(0)=(LeftAmplitude=100,RightAmplitude=100,LeftFunction=WF_LinearDecreasing,RightFunction=WF_LinearDecreasing,Duration=0.300)
	End Object
	WeaponFiredFF=ForceFeedbackWaveformShooting0

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformSquishing0
		Samples(0)=(LeftAmplitude=35,RightAmplitude=35,LeftFunction=WF_LinearDecreasing,RightFunction=WF_LinearDecreasing,Duration=0.300)
	End Object
	SquishedFF=ForceFeedbackWaveformSquishing0

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformBoosting0
		Samples(0)=(LeftAmplitude=100,RightAmplitude=100,LeftFunction=WF_Noise,RightFunction=WF_Noise,Duration=1.00)
		bIsLooping=TRUE
	End Object
	BoostFF=ForceFeedbackWaveformBoosting0

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformCentaurIdle0
		Samples(0)=(LeftAmplitude=50,RightAmplitude=50,LeftFunction=WF_Constant,RightFunction=WF_Noise,Duration=5.00)
		bIsLooping=TRUE
	End Object
	IdleFF=ForceFeedbackWaveformCentaurIdle0

	Begin Object Name=CollisionCylinder
		CollisionHeight=+65.0
		CollisionRadius=+140.0
		Translation=(Z=-15.0)
	End Object

	Begin Object Name=SVehicleMesh
		SkeletalMesh=SkeletalMesh'COG_Centaur.Meshes.COG_Centaur'
		PhysicsAsset=PhysicsAsset'COG_Centaur.COG_Centaur_Physics'
		AnimTreeTemplate=AnimTree'COG_Centaur.Anims.COG_Centaur_Animtree'
		AnimSets(0)=AnimSet'COG_Centaur.Anims.COG_Centaur_Animset'
		RBCollideWithChannels=(Default=TRUE,BlockingVolume=TRUE,GameplayPhysics=TRUE,EffectPhysics=TRUE,Vehicle=TRUE,Untitled4=TRUE)
		bAllowAmbientOcclusion=FALSE // does not really work on skelmeshes
		ForcedLodModel=1 // Force vehicle you are driving to always use highest LOD
		bForceMipStreaming=TRUE // Force textures to always be loaded
	End Object

	DrawScale=0.85

	Seats(0)={(	GunClass=class'GearGameContent.GearVWeap_RocketCannon',
		GunSocket=(GunnerFireSocket),
		TurretControls=(MainTurret_Pitch,MainTurret_Yaw),
		GunPivotPoints=(b_Turret_Yaw),
		//CameraTag=CameraOrigin,
		CameraOffset=-300,
		CameraBaseOffset=(X=-30.0,Z=20.0),
		CameraViewOffsetHigh=(X=-400,Y=0,Z=215),
		CameraViewOffsetMid=(X=-700,Y=0,Z=190),
		CameraViewOffsetLow=(X=-700,Y=0,Z=200),
		DriverDamageMult=0.0,
		bDisableOffsetZAdjust=TRUE
	)}
	Seats(1)={(	GunClass=class'GearGameContent.GearVWeap_RocketCannon',
		GunSocket=(GunnerFireSocket),
		TurretControls=(MainTurret_Pitch,MainTurret_Yaw),
		GunPivotPoints=(b_Turret_Yaw),
		//CameraTag=CameraOrigin,
		CameraOffset=-300,
		CameraBaseOffset=(X=-30.0,Z=20.0),
		CameraViewOffsetHigh=(X=-400,Y=0,Z=215),
		CameraViewOffsetMid=(X=-700,Y=0,Z=190),
		CameraViewOffsetLow=(X=-700,Y=0,Z=200),
		DriverDamageMult=0.0,
		bDisableOffsetZAdjust=TRUE
	)}

	SplitDriverCamViewOffsetHigh=(X=-500,Y=0.0,Z=160.0)
	SplitDriverCamViewOffsetMid=(X=-700,Y=0.0,Z=150.0)
	SplitDriverCamViewOffsetLow=(X=-800,Y=0.0,Z=200.0)

	CameraLag=0.15
	OverlayEffect=PostProcessChain'War_PostProcess.CentaurOverlayPostProcess'
	OverlaySpreadFactor=-0.2
	OverlayEffectFactor=0.78

	bVehicleSpaceCamera=TRUE
	bPassengerVehicleSpaceCamera=FALSE
	bOnlyInheritVehicleSpaceYaw=TRUE

	COMOffset=(x=0.0,y=0.0,z=-125.0)
	InertiaTensorMultiplier=(x=0.7,y=0.7,z=1.0)
	UprightLiftStrength=500.0
	UprightTorqueStrength=400.0
	bCanFlip=true
	bSeparateTurretFocus=true
	bHasHandbrake=FALSE
	GroundSpeed=800
	AirSpeed=1100
	MaxSpeed=2000
	ObjectiveGetOutDist=1500.0
	LookSteerSensitivity=2.2
	LookSteerDamping=0.04

	ConsoleSteerScale=1.2
	ConsoleThrottleScale=1.0
	DeflectionReverseThresh=-0.3
	bStickDeflectionThrottle=FALSE

	bAllowBoosting=TRUE
	BoostStartSound=SoundCue'Vehicle_Centaur.Centaur.Centaur_PowerBoostCue'
//	BoostEndSound=SoundCue'Ambient_NonLoop.AmbientNonLoop.MetalCranksCue'

	Stage1DamageEffect=ParticleSystem'COG_Centaur.Effects.P_COG_Centaur_Damaged_Stage1'
	Stage2DamageEffect=ParticleSystem'COG_Centaur.Effects.P_COG_Centaur_Damaged_Stage2'
	Stage3DamageEffect=ParticleSystem'COG_Centaur.Effects.P_COG_Centaur_Damaged_Stage3'

	DamageEffectSocketName=DamageSocket

	StartRepairDialog.Add(SoundCue'Human_Baird_Dialog_Cue.MountKadar.BAIRD_MK_04Cue')
	StartRepairDialog.Add(SoundCue'Human_Baird_Dialog_Cue.MountKadar.BAIRD_MK_05Cue')
	StartRepairDialog.Add(SoundCue'Human_Baird_Dialog_Cue.MountKadar.BAIRD_MK_06Cue')
	StartRepairDialog.Add(SoundCue'Human_Baird_Dialog_Cue.MountKadar.BAIRD_MK_07Cue')

	DoneRepairDialog.Add(SoundCue'Human_Baird_Dialog_Cue.MountKadar.BAIRD_MK_10Cue')
	DoneRepairDialog.Add(SoundCue'Human_Baird_Dialog_Cue.MountKadar.BAIRD_MK_13Cue')
	DoneRepairDialog.Add(SoundCue'Human_Baird_Dialog_Cue.MountKadar.BAIRD_MK_15Cue')

	Begin Object Class=AudioComponent Name=MyBairdAC
		bStopWhenOwnerDestroyed=TRUE
	End Object
	BairdRepairAC=MyBairdAC
	Components.Add(MyBairdAC);

	Begin Object Class=GearVehicleSimCar Name=SimObject
		WheelSuspensionStiffness=75.0
		WheelSuspensionDamping=2.5
		WheelSuspensionBias=0.5
		WheelInertia=1.0
		//ChassisTorqueScale=8.0
		ChassisTorqueScale=4.0
		LSDFactor=0.2
		TorqueVSpeedCurve=(Points=((InVal=-600.0,OutVal=0.0),(InVal=-400.0,OutVal=35.0),(InVal=0.0,OutVal=35.0),(InVal=800.0,OutVal=35.0),(InVal=1200.0,OutVal=0.0)))
		EngineBrakeFactor=0.03
		MaxBrakeTorque=20.0
		MaxSteerAngleCurve=(Points=((InVal=0,OutVal=35.0),(InVal=300.0,OutVal=38.0),(InVal=1100.0,OutVal=35.0)))
		SteerSpeed=200
		BoostSteerSpeed=80
		BoostMaxSteerAngleScale=0.4
		StopThreshold=200
		ThrottleSpeed=2.0

		bClampedFrictionModel=true

		// Longitudinal tire model based on 10% slip ratio peak
		WheelLongExtremumSlip=0.1
		WheelLongExtremumValue=1.0
		WheelLongAsymptoteSlip=2.0
		WheelLongAsymptoteValue=0.6

		// Lateral tire model based on slip angle (radians)
		WheelLatExtremumSlip=0.2
		WheelLatExtremumValue=0.56
		WheelLatAsymptoteSlip=1.4
		WheelLatAsymptoteValue=0.56

		InAirAngVelDamping=700.0
		InAirUprightMaxTorque=200000.0
		InAirUprightTorqueFactor=-1500.0

		BoostSpeed=4.0
		BoostAirSpeedScale=1.6
		BoostTorqueScale=1.1

		bDoTankSteering=True
		TankSteerThrottleThreshold=0.2
		MaxEngineTorque=100.0
		TurnInPlaceThrottle=0.25
		InsideTrackTorqueFactor=0.25
	End Object
	SimObj=SimObject
	Components.Add(SimObject)

	// RR, RL, RF, LF

	Begin Object Class=SVehicleWheel Name=RRWheel
		WheelRadius=78
		SuspensionTravel=60
		bPoweredWheel=true
		SteerFactor=-0.4
		WheelParticleSystem=ParticleSystem'COG_Centaur.Effects.P_COG_Centaur_Wheel_Dust'
		SlipParticleParamName="CentaurTireFXSpawnrate"
		//bUseMaterialSpecificEffects=true
		ParkedSlipFactor=10.0
		BoneName="b_Rear_Rt_Tire"
		BoneOffset=(X=0.0,Y=35.0,Z=30.0)
		SkelControlName="Rt_Rear_Tire"
		LongSlipFactor=2.0
		LatSlipFactor=1.5
		HandbrakeLongSlipFactor=0.6
		HandbrakeLatSlipFactor=0.2
		Side=SIDE_Right
	End Object
	Wheels(0)=RRWheel

	Begin Object Class=SVehicleWheel Name=LRWheel
		WheelRadius=78
		SuspensionTravel=60
		bPoweredWheel=true
		SteerFactor=-0.4
		WheelParticleSystem=ParticleSystem'COG_Centaur.Effects.P_COG_Centaur_Wheel_Dust'
		SlipParticleParamName="CentaurTireFXSpawnrate"
		//bUseMaterialSpecificEffects=true
		ParkedSlipFactor=10.0
		BoneName="b_Rear_Lt_Tire"
		BoneOffset=(X=0.0,Y=-35.0,Z=30.0)
		SkelControlName="Lt_Rear_Tire"
		LatSlipFactor=1.5
		LongSlipFactor=2.0
		HandbrakeLongSlipFactor=0.6
		HandbrakeLatSlipFactor=0.2
		Side=SIDE_Left
		End Object
	Wheels(1)=LRWheel

	Begin Object Class=SVehicleWheel Name=RFWheel
		WheelRadius=78
		SuspensionTravel=60
		bPoweredWheel=true
		WheelParticleSystem=ParticleSystem'COG_Centaur.Effects.P_COG_Centaur_Wheel_Dust'
		SlipParticleParamName="CentaurTireFXSpawnrate"
		//bUseMaterialSpecificEffects=true
		ParkedSlipFactor=10.0
		BoneName="b_Front_Rt_Tire"
		BoneOffset=(X=0.0,Y=35.0,Z=30.0)
		SteerFactor=0.66
		SkelControlName="Rt_Front_Tire"
		LongSlipFactor=2.0
		LatSlipFactor=2.0
		HandbrakeLongSlipFactor=0.8
		HandbrakeLatSlipFactor=0.8
		Side=SIDE_Right
	End Object
	Wheels(2)=RFWheel

	Begin Object Class=SVehicleWheel Name=LFWheel
		WheelRadius=78
		SuspensionTravel=60
		bPoweredWheel=true
		WheelParticleSystem=ParticleSystem'COG_Centaur.Effects.P_COG_Centaur_Wheel_Dust'
		SlipParticleParamName="CentaurTireFXSpawnrate"
		//bUseMaterialSpecificEffects=true
		ParkedSlipFactor=10.0
		BoneName="b_Front_Lt_Tire"
		BoneOffset=(X=0.0,Y=-35.0,Z=30.0)
		SteerFactor=0.66
		SkelControlName="Lt_Front_Tire"
		LongSlipFactor=2.0
		LatSlipFactor=2.0
		HandbrakeLongSlipFactor=0.8
		HandbrakeLatSlipFactor=0.8
		Side=SIDE_Left
	End Object
	Wheels(3)=LFWheel

	bReducedFallingCollisionDamage=true

	ViewPitchMin=-6000
	ViewPitchMax=4400

	BaseEyeheight=0
	Eyeheight=0
	HeavySuspensionShiftPercent=0.2

	WheelExtraGraphicalSpin=3.0
	PeelOutSound=SoundCue'Vehicle_Centaur.Centaur.Centaur_TireSquealCue'
	PeelOutCamAnim=CameraAnim'Effects_Camera.Gameplay_Movement.CA_COG_Centaur_WheelSlip'
	PeelOutMaxWheelVel=7.5
	PeelOutFrictionScale=1.0

	SnowWheelEffect=ParticleSystem'COG_Centaur.Effects.P_COG_Centaur_Wheel_Dust'
	SnowPeelOutEffect=ParticleSystem'COG_Centaur.Effects.P_COG_Centaur_Wheel_Wheelspin'
	SnowBoostEffect=ParticleSystem'COG_Centaur.Effects.P_COG_Centaur_Wheel_Boost'

	IceWheelEffect=ParticleSystem'COG_Centaur.Effects.P_COG_Centaur_Wheel_Dust_Ice'
	IcePeelOutEffect=ParticleSystem'COG_Centaur.Effects.P_COG_Centaur_Wheel_Water'
	IceBoostEffect=ParticleSystem'COG_Centaur.Effects.P_COG_Centaur_Wheel_Boost'

	CaveWheelEffect=ParticleSystem'COG_Centaur.Effects.P_COG_Centaur_Wheel_Dust_Caves'
	CavePeelOutEffect=ParticleSystem'COG_Centaur.Effects.P_COG_Centaur_Wheel_Wheelspin_Caves'
	CaveBoostEffect=ParticleSystem'COG_Centaur.Effects.P_COG_Centaur_Wheel_Boost_Caves'

	SpotlightConeEffect=ParticleSystem'COG_Centaur.Effects.P_Headlight_Cone'

	//Boost effect
	TailpipeSocketName=BoostThruster
	TailpipeBoostEffect=ParticleSystem'COG_Centaur.Effects.P_COG_Centaur_Boost_Thruster'
	BoostHeatEffectName=Pipe_Heat

	MomentumMult=1.0
	bEjectKilledBodies=FALSE
	bEjectPassengersWhenFlipped=FALSE

	RunOverCreatureCamAnim=CameraAnim'Effects_Camera.Gameplay_Movement.CA_COG_Centaur_Run_Over'

	CentaurCannonMuzzzleEffect=ParticleSystem'COG_Centaur.Effects.P_COG_Centaur_MuzzleFlash'
	CentaurCannonSound=SoundCue'Vehicle_Centaur.Centaur.Centaur_RocketFireACue'
	CentaurCannonCamAnim=CameraAnim'Effects_Camera.Weapons.CA_COG_Centaur_Weapon_Main'
	TargetingCannonCamAnimScale=0.3

	// muzzle light
	Begin Object Class=PointLightComponent Name=MuzzleLight0
		bEnabled=FALSE
		LightColor=(B=35,G=185,R=255,A=255)
		Brightness=10
		Radius=256
	End Object
	CentaurCannonMuzzleLight=MuzzleLight0
	CentaurCannonMuzzleLightTime=0.4f

	CentaurDeathSound=SoundCue'Vehicle_Centaur.Centaur.Centaur_ExplodesCue'
	CentaurDeathEffect=ParticleSystem'Effects_Gameplay.Explosions.P_Centaur_Death'
	CentaurDeathCamAnim=CameraAnim'COG_Centaur.Centaur_DeathCam'

	BoostCameraAnim=CameraAnim'COG_Centaur.Centaur_BoostCam'

	//TireSoundList(0)=(MaterialType=Dirt,Sound=SoundCue'A_Vehicle_Generic.Vehicle.VehicleSurface_TireDirt01Cue')
	//TireSoundList(1)=(MaterialType=Foliage,Sound=SoundCue'A_Vehicle_Generic.Vehicle.VehicleSurface_TireFoliage01Cue')
	//TireSoundList(2)=(MaterialType=Grass,Sound=SoundCue'A_Vehicle_Generic.Vehicle.VehicleSurface_TireGrass01Cue')
	//TireSoundList(3)=(MaterialType=Metal,Sound=SoundCue'A_Vehicle_Generic.Vehicle.VehicleSurface_TireMetal01Cue')
	//TireSoundList(4)=(MaterialType=Mud,Sound=SoundCue'A_Vehicle_Generic.Vehicle.VehicleSurface_TireMud01Cue')
	//TireSoundList(5)=(MaterialType=Snow,Sound=SoundCue'A_Vehicle_Generic.Vehicle.VehicleSurface_TireSnow01Cue')
	//TireSoundList(6)=(MaterialType=Stone,Sound=SoundCue'A_Vehicle_Generic.Vehicle.VehicleSurface_TireStone01Cue')
	//TireSoundList(7)=(MaterialType=Wood,Sound=SoundCue'A_Vehicle_Generic.Vehicle.VehicleSurface_TireWood01Cue')
	//TireSoundList(8)=(MaterialType=Water,Sound=SoundCue'A_Vehicle_Generic.Vehicle.VehicleSurface_TireWater01Cue')

	//WheelParticleEffects[0]=(MaterialType=Generic,ParticleTemplate=ParticleSystem'Envy_Level_Effects_2.Vehicle_Dust_Effects.P_Hellbender_Wheel_Dust')
	//WheelParticleEffects[1]=(MaterialType=Dirt,ParticleTemplate=ParticleSystem'Envy_Level_Effects_2.Vehicle_Dirt_Effects.P_Hellbender_Wheel_Dirt')
	//WheelParticleEffects[2]=(MaterialType=Water,ParticleTemplate=ParticleSystem'Envy_Level_Effects_2.Vehicle_Water_Effects.P_Hellbender_Water_Splash')
	//WheelParticleEffects[3]=(MaterialType=Snow,ParticleTemplate=ParticleSystem'Envy_Level_Effects_2.Vehicle_Snow_Effects.P_Hellbender_Wheel_Snow')

	// Sounds

	Begin Object Class=AudioComponent Name=ScorpionTireSound
	//SoundCue=SoundCue'A_Vehicle_Generic.Vehicle.VehicleSurface_TireDirt01Cue'
	End Object
	TireAudioComp=ScorpionTireSound
	Components.Add(ScorpionTireSound);

	CollisionSound=SoundCue'Vehicle_Centaur.Centaur.Centaur_BodyImpactLargeCue'

	// On Fire sound
	Begin Object Class=AudioComponent Name=MyOnFireSound
		SoundCue=SoundCue'Vehicle_Centaur.Centaur.Centaur_OnFireCue'
	End Object
	AC_OnFireSound=MyOnFireSound
	Components.Add(MyOnFireSound);

	// Spotlight sound.
	Begin Object Class=AudioComponent Name=MySpotlightSound
		SoundCue=SoundCue'Vehicle_Centaur.Centaur.Centaur_SpotlightLoopCue'
		bStopWhenOwnerDestroyed=true
		bAutoPlay=false
	End Object
	AC_SpotlightSound=MySpotlightSound

	SpotLightFlickerSound=SoundCue'Vehicle_Centaur.Centaur.Centaur_SpotlightStartCue'
	SpotLightOffSound=SoundCue'Vehicle_Centaur.Centaur.Centaur_SpotlightStopCue'

	EnterVehicleSound=SoundCue'Vehicle_APC.Vehicles.VehicleAPCStartCue'
	ExitVehicleSound=SoundCue'Vehicle_APC.Vehicles.VehicleAPCStopCue'

	// Squish sound
	SquishSound=SoundCue'Vehicle_Centaur.Centaur.Centaur_BodyCrunchCue'
	// Horn sound
	HornSound=SoundCue'Vehicle_Centaur.Centaur.Centaur_HornDoubleCue'
	//Vehicle_Centaur.Centaur.Centaur_HornLongCue
	//Vehicle_Centaur.Centaur.Centaur_HornDoubleCue
	//Vehicle_Centaur.Centaur.Centaur_HornShortCue

	// Turret rotation sound
	Begin Object Class=AudioComponent Name=MyTurretRotSound
		SoundCue=SoundCue'Vehicle_Centaur.Centaur.Centaur_TurretRotateLoopCue'
	End Object
	CentaurTurretRotationAC=MyTurretRotSound
	Components.Add(MyTurretRotSound);

	CentaurTurretRotationVolumeVelRange=(X=0.f,Y=32768.f);
	CentaurTurretRotationVolumeRange=(X=0.7f,Y=1.3f);
	CentaurTurretRotationPitchVelRange=(X=0.f,Y=32768.f);
	CentaurTurretRotationPitchRange=(X=0.9f,Y=1.1f);

	CentaurGear={(
		PitchRange=(X=0.6f,Y=1.8f),
		//PitchRPMRange=(X=0.f,Y=1100.f),
		PitchRPMRange=(X=5.f,Y=30.f),
		VolumeRange=(X=0.9f,Y=1.2f),
		VolumeRPMRange=(X=5.f,Y=30.f),
		//VolumeRPMRange=(X=0.f,Y=800.f),
		EngineLoopCue=SoundCue'Vehicle_Centaur.Centaur.Centaur_EngineRPMSlowLoopCue',
		)}

	EnginePlayerAmbientLoopCue=SoundCue'Vehicle_Centaur.Centaur.Centaur_EngineAmbientLoopCue'
	EngineStartSound=SoundCue'Vehicle_Centaur.Centaur.Centaur_EngineStartCue'
	EngineStopSound=SoundCue'Vehicle_Centaur.Centaur.Centaur_EngineStopCue'
	CentaurLandSound=SoundCue'Vehicle_Centaur.Centaur.Centaur_LandCue'

	CentaurSuspensionSound=SoundCue'Vehicle_Centaur.Centaur.Centaur_SuspensionCue'
	CentaurSuspensionSoundThreshold=30
	CentaurSuspensionSoundMinTimeBetween=3.f

	SnowTireAudioLoop=SoundCue'Vehicle_Centaur.Centaur.Centaur_TireLoopSnowCue'
	IceTireAudioLoop=SoundCue'Vehicle_Centaur.Centaur.Centaur_TireLoopIceCue'
	CaveTireAudioLoop=SoundCue'Vehicle_Centaur.Centaur.Centaur_TireLoopStoneCue'
	TireAudioLoopVolumeRange=(X=0.f,Y=1.4f)
	TireAudioLoopVolumeVelocityRange=(X=0.f,Y=300.f)


	// Initialize sound parameters.
	SquealThreshold=0.1
	SquealLatThreshold=0.02
	LatAngleVolumeMult=30.0

	EngineStartOffsetSecs=0.5
	EngineStopOffsetSecs=1.0

	SnowFieldSocketName=SnowMeshSpawnPoint

	TurnTooltipVelThresh=50.0
	TurnToolTipDelay=0.5
	ActionTurnOnSpot={(
		ActionName=TurnOnSpot,
		IconAnimationSpeed=0.1f,
		ActionIconDatas=(	(ActionIcons=(	(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=134,V=327,UL=93,VL=90), // Stick move icon
											(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=134,V=327,UL=93,VL=90) )),
							(ActionIcons=(	(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=177,V=429,UL=106,VL=83) )) ) // tank turn icon
		)}

	//SnowSprayTemplate=ParticleSystem'GOW_MountKismet.Effects.P_MK_Snow_VehicleBased_01'
}
