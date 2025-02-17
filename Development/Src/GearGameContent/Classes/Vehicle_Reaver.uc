/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Vehicle_Reaver extends Vehicle_Reaver_Base
	notplaceable;

defaultproperties
{
	DriverClass=class'GearPawn_LocustReaverDriver'
	GunnerClass=class'GearPawn_LocustReaverPassenger'
	GunnerOffset=(Z=80.0)

	Seats(0)={(	GunClass=class'GearGameContent.GearVWeap_ReaverCannon',
				GunSocket=("Cannon"),
				GunPivotPoints=(b_Gun_Front),
				bSeatVisible=true,
				SeatRotation=(Yaw=16384),
				SeatOffset=(Z=0.0f),
				SeatBone="b_Main",
				CameraTag="",
				CameraViewOffsetHigh=(X=-1000,Y=0,Z=100),
				CameraViewOffsetMid=(X=-750,Y=0,Z=-150),
				CameraViewOffsetLow=(X=--750,Y=0,Z=0),
				WorstCameraLocOffset=(X=0,Y=0,Z=0),
				bDisableOffsetZAdjust=TRUE,
				DriverDamageMult=1.0
				)}

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
		bUseSingleBodyPhysics=1
		bAcceptsDynamicDecals=FALSE
	End Object

	ReaverRagdollPhysicsAsset=PhysicsAsset'Locust_Reaver_Anim.ReaverTest_Phys_Welded'
	GorePhysicsAsset=PhysicsAsset'Locust_Reaver.Locust_Reaver_Gore_Physics'
	//GoreSkeletalMesh=SkeletalMesh'Locust_Reaver.Locust_Gore_Chunks'
	GoreSkeletalMesh=SkeletalMesh'Locust_Reaver.Locust_Reaver_LOD1_Gore'
	GoreExplosionRadius=500.0
	GoreExplosionVel=1000.0
	ReaverDeathSound=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_DeathCue'
	ReaverExplodeEffect=ParticleSystem'Locust_Reaver.Effects.P_Reaver_Gib_Explosion_Main'
	ReaverExplodeNoGoreEffect=ParticleSystem'Locust_Reaver.Effects.P_Reaver_Gib_Explosion_Main_NOGORE'
	ReaverPreExplodeEffect=ParticleSystem'Locust_Reaver.Effects.P_Reaver_Body_Pre_Explosion'
	ReaverGibDeathSound=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_DeathExplosionCue'
	GibKillDistance=1500.0

	GoreImpactParticle=ParticleSystem'Effects_Gameplay.Blood.P_Blood_groundimpact_bodypart'

	//GoreBreakableJoints=("Gore_1","Gore_09","Gore_06","Gore_11","b_Tentacle_A_L_01","b_Tentacle_A_L_08","b_Tentacle_C_L_05","b_Tentacle_C_L_10","b_Tentacle_B_L_06","b_Tentacle_B_R_01","b_Tentacle_A_R_06","b_Tentacle_C_R_01","b_Tentacle_C_L_06","b_Neck","b_Head","b_Tongue_01","b_Jaw","b_Throat","b_Arm_L_01","b_Arm_L_02","b_Arm_R_01","b_Arm_R_02","Gore_12","Gore_08","Gore_07","Gore_05","Gore_04");

	Begin Object Class=AudioComponent Name=MyRandomSound
		bStopWhenOwnerDestroyed=TRUE
	End Object
	RandomSoundComp=MyRandomSound
	Components.Add(MyRandomSound)

	bCanBeBaseForPawns=FALSE
	CollisionDamageMult=0.0

	COMOffset=(X=0,Y=0.0,Z=150)
	bCanFlip=FALSE

	AirSpeed=275.0
	GroundSpeed=275.0

	bUseSuspensionAxis=TRUE

	bStayUpright=TRUE
	StayUprightRollResistAngle=0.0			// will be "locked"
	StayUprightPitchResistAngle=0.0

	Begin Object Class=GearVehicleSimHover Name=SimObject
		WheelSuspensionStiffness=100.0
		WheelSuspensionDamping=40.0
		WheelSuspensionBias=0.0
		MaxThrustForce=300.0
		MaxReverseForce=300.0
		LongDamping=0.3
		MaxStrafeForce=300.0
		LatDamping=0.3
		MaxRiseForce=0.0
		UpDamping=0.0
		TurnTorqueFactor=20000.0
		TurnTorqueMax=5000.0
		TurnDamping=1.0
		MaxYawRate=0.6
		PitchTorqueMax=35.0
		PitchDamping=0.1
		RollTorqueMax=50.0
		RollDamping=0.1
		MaxRandForce=0.0
		RandForceInterval=1000.0
		bCanClimbSlopes=TRUE
		PitchTorqueFactor=0.0
		RollTorqueTurnFactor=0.0
		RollTorqueStrafeFactor=0.0
		bAllowZThrust=FALSE
		bStabilizeStops=TRUE
		StabilizationForceMultiplier=1.0
		bFullThrustOnDirectionChange=TRUE
		bDisableWheelsWhenOff=FALSE
		HardLimitAirSpeedScale=1.5

		// Longitudinal tire model based on 10% slip ratio peak
		WheelLongExtremumSlip=0.1
		WheelLongExtremumValue=0
		WheelLongAsymptoteSlip=2.0
		WheelLongAsymptoteValue=0

		// Lateral tire model based on slip angle (radians)
		WheelLatExtremumSlip=0.2
		WheelLatExtremumValue=0
		WheelLatAsymptoteSlip=1.4
		WheelLatAsymptoteValue=0
	End Object
	SimObj=SimObject
	Components.Add(SimObject)

	ExitPositions(0)=(Y=-400,Z=60)
	ExitPositions(1)=(Y=400,Z=60)

	Begin Object Class=SVehicleWheel Name=RThruster
		BoneName="b_Main"
		BoneOffset=(X=0,Y=0,Z=-250)
		WheelRadius=50
		SuspensionTravel=260
		bPoweredWheel=FALSE
		SteerFactor=1.0
		LongSlipFactor=0.0
		LatSlipFactor=0.0
		HandbrakeLongSlipFactor=0.0
		HandbrakeLatSlipFactor=0.0
		bCollidesVehicles=FALSE
		bCollidesPawns=FALSE
	End Object
	Wheels(0)=RThruster

	Begin Object Class=GearExplosion Name=ExploTemplate0
		MyDamageType=class'GDT_ReaverLegStrike'
		MomentumTransferScale=20.f	// Scale momentum defined in DamageType

		ParticleEmitterTemplate=ParticleSystem'Locust_Reaver.Particles.P_Reaver_Ground_Hit'
		ExplosionSound=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_ArmStabImpactCue'

		FractureMeshRadius=120.0
		FracturePartVel=500.0

		bDoExploCameraShake=TRUE
		ExploAnimShake=(Anim=CameraAnim'Effects_Camera.Explosions.CA_Reaver_Tentacle_Strike',AnimBlendInTime=0.000000)
		// ExploShakeInnerRadius/ExploShakeOuterRadius set in LegAttackImpact()
	End Object
	StrikeTemplate=ExploTemplate0

	LandingViewShake=(TimeDuration=2.f,FOVAmplitude=0,LocAmplitude=(X=0,Y=0,Z=0),RotAmplitude=(X=1000,Y=400,Z=600),RotFrequency=(X=80,Y=40,Z=50))
	LandingShakeInnerRadius=512.f
	LandingShakeOuterRadius=2048.f
	LandingShakeFalloff=1.f

	Begin Object Class=CylinderComponent Name=CylComp0
		CollisionHeight=15.000000
		CollisionRadius=15.000000
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=TRUE
		BlockActors=FALSE
		CollideActors=TRUE
		AlwaysLoadOnServer=TRUE
	End Object
	Components.Add(CylComp0)
	MouthComp=CylComp0

	Begin Object Class=CylinderComponent Name=CylComp1
		CollisionHeight=20.000000
		CollisionRadius=30.000000
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=TRUE
		BlockActors=FALSE
		CollideActors=TRUE
		AlwaysLoadOnServer=TRUE
		Translation=(Y=40,Z=-10)
	End Object
	Components.Add(CylComp1)
	BellyComp=CylComp1


	ReaverAttackSound=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_AttackCue'

	FlyingSoundLoop=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_FlyLoop03Cue'
//	FlyingSoundLoop_Player=SoundCue'Locust_Reaver_Efforts.ReaverPlayer.ReaverPlayer_FlyLoopCue'

	ReaverScreamSound=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_ScreamACue'
	ReaverPainSound=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_PainCue'
//	ReaverPainSound_Player=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_VocalPainCue'
	ReaverSeePlayerSound=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_AlertCue'
	ReaverHowlSound=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_HowlCue'
	ReaverLandSound=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_FlyLandCue'
//	ReaverLandSound_Player=SoundCue'Locust_Reaver_Efforts.ReaverPlayer.ReaverPlayer_FlyLandCue'
	ReaverTakeOffSound=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_FlyAwayCue'
//	ReaverTakeOffSound_Player=SoundCue'Locust_Reaver_Efforts.ReaverPlayer.ReaverPlayer_FlyAwayCue'
	LegAttackSound=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_ArmStabCue'
	ReaverFootstepSound=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_FootstepsCue'
	//ReaverChainsawLegPainSound=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_ScreamACue'

	MinHitAnimInterval=1.5

//	ReaverRandomSounds(0)=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_BreathingCue'
//	ReaverRandomSounds(1)=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_HissCue'
//	ReaverRandomSounds(2)=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_SniffingCue'
//	ReaverRandomSounds(3)=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_GroanCue'
//	ReaverRandomSounds(4)=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_CacklingCue'
//	ReaverRandomSoundInterval=(X=3.0,Y=6.0)

	CollisionSound=SoundCue'Vehicle_APC.Vehicles.VehicleAPCCollideCue'
}
