/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Vehicle_Jack extends Vehicle_Jack_Base;

var array<MaterialInstanceConstant>	CloakMaterialList;
var MaterialInterface				AnyaMonitorMatierial;
var float							CloakTimer;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	// Create cloaking/decloaking material instances
	CloakMaterialList.Length = 2;

	CloakMaterialList[0] = new(Outer) class'MaterialInstanceConstant';
	CloakMaterialList[0].SetParent( MaterialInterface'COG_BOT.Materials.COG_BOT_CP_Mat_New_INST_AI' );
	Mesh.SetMaterial( 0, CloakMaterialList[0] );

	CloakMaterialList[1] = new(Outer) class'MaterialInstanceConstant';
	CloakMaterialList[1].SetParent( MaterialInterface'COG_BOT.Materials.cog_jackbot_screen_M_INST_AI' );
	Mesh.SetMaterial( 1, CloakMaterialList[1] );
}

simulated function Tick( float DeltaTime )
{
	local float Duration, Pct;
	local int Idx;

	Super.Tick( DeltaTime );

	if( bCloaking )
	{
		Duration = Mesh.GetAnimLength( IsCloaked() ? CloakedAnimName : JackUnfoldAnimName ) * 0.95;
		CloakTimer = FMin( CloakTimer + DeltaTime, Duration );

		Pct = CloakTimer / Duration;
		if( !IsCloaked() )
		{
			Pct = 1.f - Pct;
		}

		for( Idx = 0; Idx < 2; Idx++ )
		{
			Mesh.SetMaterial( Idx, CloakMaterialList[Idx] );
			CloakMaterialList[Idx].SetScalarParameterValue( 'res', Pct );
		}
	}
}

simulated function PlayCloakAnim()
{
	local int Idx;

	//debug
	`AILog_Ext( self@GetFuncName()@IsCloaked(), '', MyGearAI );

	Super.PlayCloakAnim();

	CloakTimer = 0.f;
	for( Idx = 0; Idx < 2; Idx++ )
	{
		Mesh.SetMaterial( Idx, CloakMaterialList[Idx] );
	}

	ShowRightArmAttachment( None );
}

simulated function FinishedCloaking( bool bHide, optional bool bForceResUpdate )
{
	local int Idx;

	//debug
	`AILog_Ext( self@GetFuncName()@bHide@bForceResUpdate, '', MyGearAI );

	Super.FinishedCloaking( bHide, bForceResUpdate );

	if( !bHide )
	{
		// Fixup materials properly... make sure Anya is showing if monitor is down
		for( Idx = 0; Idx < 2; Idx++ )
		{
			CloakMaterialList[Idx].SetScalarParameterValue( 'res', 0.f );
		}
		Mesh.SetMaterial( 1, bUnfoldMonitor ? AnyaMonitorMatierial : CloakMaterialList[1] );

		ShowRightArmAttachment( RightArmAttach_Hand );
	}
}

simulated function PlayMonitorAnim()
{
	Super.PlayMonitorAnim();

	Mesh.SetMaterial( 1, bUnfoldMonitor ? AnyaMonitorMatierial : CloakMaterialList[1] );
}

defaultproperties
{
	Seats(0)={(	//GunClass=class'GearGameContent.GearVWeap_ReaverCannon',
		//GunSocket=("Cannon"),
		//GunPivotPoints=(b_Neck),
		bSeatVisible=FALSE,
		SeatRotation=(Yaw=16384),
		SeatOffset=(Z=65.0f),
		SeatBone="bone01",
		CameraTag="",
		CameraViewOffsetHigh=(X=-1000,Y=0,Z=100),
		CameraViewOffsetMid=(X=-750,Y=0,Z=-150),
		CameraViewOffsetLow=(X=--750,Y=0,Z=0),
		WorstCameraLocOffset=(X=0,Y=0,Z=0),
		bDisableOffsetZAdjust=TRUE
		)}

	Begin Object Name=SVehicleMesh
		SkeletalMesh=SkeletalMesh'COG_BOT.Jackbot'
		PhysicsAsset=PhysicsAsset'COG_BOT.PA_Jackbot_PhysicsV'
		AnimTreeTemplate=AnimTree'COG_BOT.AT_COG_Jack'
		AnimSets(0)=AnimSet'COG_BOT.Jackbot_animset'
		RBCollideWithChannels=(Default=TRUE,BlockingVolume=TRUE,GameplayPhysics=TRUE,EffectPhysics=TRUE,Vehicle=TRUE,Untitled4=TRUE)
		Translation=(X=-25,Y=0,Z=0)
	End Object

	Begin Object Name=CollisionCylinder
		CollisionRadius=+0048.000000
		CollisionHeight=+0060.000000
		BlockNonZeroExtent=TRUE
		BlockZeroExtent=FALSE
		BlockActors=TRUE
		CollideActors=TRUE
	End Object

	bStayUpright=TRUE
	StayUprightRollResistAngle=5.0
	StayUprightPitchResistAngle=5.0
	StayUprightStiffness=400
	StayUprightDamping=20

	GroundSpeed=800
	AirSpeed=800
	MaxSpeed=1500

	Begin Object Class=GearVehicleSimChopper Name=SimObject
		MaxThrustForce=20.0
		MaxReverseForce=12.0
		LongDamping=0.3
		MaxStrafeForce=20.0
		LatDamping=0.07
		MaxRiseForce=8.0
		UpDamping=0.25
		TurnTorqueFactor=100.0
		TurnTorqueMax=1000.0
		TurnDamping=0.01
		MaxYawRate=3.0
		PitchTorqueFactor=1.0
		PitchTorqueMax=60.0
		PitchDamping=0.001
		RollTorqueTurnFactor=0.0
		RollTorqueStrafeFactor=1.0
		RollTorqueMax=50.0
		RollDamping=0.001
		MaxRandForce=0.0
		RandForceInterval=0.5
		StopThreshold=100
		bFullThrustOnDirectionChange=TRUE
	End Object
	SimObj=SimObject
	Components.Add(SimObject)

	JackIdleStillVocalSound=SoundCue'Human_Jack.Jack.Jack_Arcade_IdleStill01Cue'
	JackIdleMovingVocalSound=SoundCue'Human_Jack.Jack.Jack_Arcade_IdleMoving01Cue'
	JackCloakSound=SoundCue'Human_Jack.Jack.Jack_Mechanical_FadeOut01Cue'
	JackDecloakSound=SoundCue'Human_Jack.Jack.Jack_Mechanical_FadeIn01Cue'
	JackAcquiredPOISound=SoundCue'Human_Jack.Jack.Jack_Arcade_ConfirmAccept02Cue'
	JackInspectPOISound=SoundCue'Human_Jack.Jack.Jack_Arcade_CommunicateA02Cue'
	JackHappySound=SoundCue'Human_Jack.Jack.Jack_Arcade_CuteA01Cue'
	JackSadSound=SoundCue'Human_Jack.Jack.Jack_Arcade_BeepNegative01Cue'
	JackRecoilSound=SoundCue'Human_Jack.Jack.Jack_Mechanical_Recoil01Cue'
	JackArmUnfoldSound=SoundCue'Human_Jack.Jack.Jack_Mechanical_ArmMoveIn01Cue'
	JackArmFoldSound=SoundCue'Human_Jack.Jack.Jack_Mechanical_ArmMoveOut01Cue'
	JackFlyStartSound=SoundCue'Human_Jack.Jack.Jack_Mechanical_FlyStart01Cue'
	JackFlyLoopSound=SoundCue'Human_Jack.Jack.Jack_Mechanical_FlyLoop01Cue'
	JackFlyStopSound=SoundCue'Human_Jack.Jack.Jack_Mechanical_FlyStop01Cue'
	JackFlyChangeDirSound=SoundCue'Human_Jack.Jack.Jack_Mechanical_FlyChangeDirection02Cue'
	JackMonitorUnfoldSound=SoundCue'Human_Jack.Jack.Jack_Mechanical_TVExtend01Cue'
	JackMonitorFoldSound=SoundCue'Human_Jack.Jack.Jack_Mechanical_TVRetract01Cue'
	JackWeldingIntroSound=SoundCue'Human_Jack.Jack.Jack_Mechanical_WeldingIntro01Cue'
	JackWeldingOutroSound=SoundCue'Human_Jack.Jack.Jack_Mechanical_WeldingOutro01Cue'
	JackPointSound=SoundCue'Human_Jack.Jack.Jack_PointingCue'

	// Hover sound
	Begin Object Class=AudioComponent Name=AudioComponent0
		SoundCue=SoundCue'Human_Jack.Jack.Jack_Mechanical_IdleLoop01Cue'
		bAutoPlay=true
		bStopWhenOwnerDestroyed=true
		bShouldRemainActiveIfDropped=true
	End Object
	AC_Idle=AudioComponent0
	Components.Add(AudioComponent0)

	// Welding sound
	Begin Object Class=AudioComponent Name=AudioComponent1
		SoundCue=SoundCue'Human_Jack.Jack.Jack_Mechanical_RippingDoor01Cue'
		bAutoPlay=FALSE
		bStopWhenOwnerDestroyed=TRUE
		bShouldRemainActiveIfDropped=TRUE
	End Object
	AC_Welding=AudioComponent1
	Components.Add(AudioComponent1)

	// Scanning sound
	Begin Object Class=AudioComponent Name=AudioComponent2
		SoundCue=SoundCue'Human_Jack.Jack.Jack_ScanningCue'
		bAutoPlay=FALSE
		bStopWhenOwnerDestroyed=TRUE
		bShouldRemainActiveIfDropped=TRUE
	End Object
	AC_Scanning=AudioComponent2
	Components.Add(AudioComponent2)

	Begin Object Class=LightFunction Name=LightFunction_0
		SourceMaterial=Material'COG_BOT.Materials.Jack_Headlight_LFM'
	End Object

	Begin Object Class=SpotLightComponent Name=FrontSpot
		LightAffectsClassification=LAC_DYNAMIC_AND_STATIC_AFFECTING
		CastShadows=TRUE
		CastStaticShadows=TRUE
		CastDynamicShadows=TRUE
		bForceDynamicLight=FALSE
		UseDirectLightMap=FALSE
		LightingChannels=(BSP=TRUE,Static=TRUE,Dynamic=TRUE,bInitialized=TRUE)
		FalloffExponent=0.500000
		Brightness=10.000000
		InnerConeAngle=18
		OuterConeAngle=45
		Radius=512
		LightColor=(B=244,G=172,R=157,A=0)
		Function=LightFunction'LightFunction_0'
		LightEnv_BouncedLightBrightness=0.114913
		LightEnv_BouncedModulationColor=(B=52,G=239,R=255,A=221)
		bEnabled=FALSE
	End Object
	FrontSpotlightComp=FrontSpot

	PS_Eye=ParticleSystem'COG_BOT.Effects.P_Jack_Eye'
	PS_Jets=ParticleSystem'COG_BOT.Effects.P_Jack_Jets'
	PS_Cloak=ParticleSystem'COG_BOT.Effects.P_Jack_ResOUT'
	PS_Welding=ParticleSystem'COG_BOT.Effects.P_Jack_Laser'
	PS_WeldingImpact=ParticleSystem'COG_BOT.Effects.P_Jack_Torch_Impact'

	WeldingImpactOffset=34.f

	// hand
	Begin Object Class=StaticMeshComponent Name=RightArmItem0
		CollideActors=FALSE
		BlockRigidBody=FALSE
		StaticMesh=StaticMesh'COG_BOT.Meshes.cog_jackbot_hand_claw'
		LightEnvironment=MyLightEnvironment
		Scale=1.1f
	End Object
	// welder
	Begin Object Class=StaticMeshComponent Name=RightArmItem1
		CollideActors=FALSE
		BlockRigidBody=FALSE
		StaticMesh=StaticMesh'COG_BOT.Meshes.cog_jackbot_hand_weld'
		LightEnvironment=MyLightEnvironment
		Scale=1.1f
	End Object
	RightArmAttach_Hand=RightArmItem0
	RightArmAttach_Welder=RightArmItem1

	AnyaMonitorMatierial=MaterialInterface'COG_BOT.Materials.cog_jackbot_screen_ON_M'

	JackFoldAnimName=Jackbot_Fold
	JackUnfoldAnimName=Jackbot_Unfold
	JackRecoilAnimName=Jackbot_Recoil
}
