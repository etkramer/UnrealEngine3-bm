/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Defines an emergence hole for AI to climb out of
 */
class GearSpawner_EmergenceHoleCircular extends GearSpawner_EmergenceHoleBase;


defaultproperties
{
	bDoReminders=TRUE

	SpawnSlots(0)=(LocationOffset=(X=90,Y=0,Z=-150),RotationOffset=(Yaw=0))
	SpawnSlots(1)=(LocationOffset=(X=60,Y=60,Z=-150),RotationOffset=(Yaw=8192))
	SpawnSlots(2)=(LocationOffset=(X=0,Y=90,Z=-150),RotationOffset=(Yaw=16384))
	SpawnSlots(3)=(LocationOffset=(X=-60,Y=60,Z=-150),RotationOffset=(Yaw=24596))
	SpawnSlots(4)=(LocationOffset=(X=-90,Y=0,Z=-150),RotationOffset=(Yaw=32768))
	SpawnSlots(5)=(LocationOffset=(X=-60,Y=-60,Z=-150),RotationOffset=(Yaw=41000))
	SpawnSlots(6)=(LocationOffset=(X=0,Y=-90,Z=-150),RotationOffset=(Yaw=49152))
	SpawnSlots(7)=(LocationOffset=(X=60,Y=-60,Z=-150),RotationOffset=(Yaw=57000))

	bEdShouldSnap=TRUE
	bBlockActors=TRUE
	bCollideActors=TRUE
	bWorldGeometry=TRUE
	bGameRelevant=TRUE
	bMovable=FALSE
	RemoteRole=ROLE_SimulatedProxy
	bAlwaysRelevant=true
	bOnlyDirtyReplication=true

	bSpawnPOI=TRUE

	SourceTexture=Texture2D'Ephyra.Ephyra_Pavement1_Tex'
	SourceMaterial=Material'Ephyra.Ephyra_Pavement1_Mat'

	OpenAnimationName=Locust_E_Hole_Burst2_seq

	OpenSounds(0)=(Time=4.216216,Volume=0.800000,Pitch=1.000000,Sound=SoundCue'Ambient_NonLoop.AmbientNonLoop.EarthDebrisMediumClosesCue')
	OpenSounds(1)=(Time=4.203262,Volume=0.800000,Pitch=1.000000,Sound=SoundCue'Foley_Crashes.SoundCues.Impact_Stone_LargeLouder')
	OpenSounds(2)=(Time=6.892540,Volume=0.800000,Pitch=1.000000,Sound=SoundCue'Foley_Crashes.SoundCues.Impact_Stone_HugeLouder')
	OpenSounds(3)=(Time=4.197124,Volume=0.800000,Pitch=1.000000,Sound=SoundCue'Foley_Crashes.SoundCues.Impact_Stone_HugeLouder')
	OpenSounds(4)=(Time=6.872089,Volume=0.800000,Pitch=1.000000,Sound=SoundCue'Ambient_NonLoop.AmbientNonLoop.EarthDebrisSmallClosesCue')
	OpenSounds(5)=(Volume=0.800000,Pitch=1.000000,Sound=SoundCue'Ambient_NonLoop.AmbientNonLoop.EarthCrackingDistantsCue')
	OpenSounds(6)=(Time=6.874696,Volume=0.800000,Pitch=1.000000,Sound=SoundCue'Foley_Crashes.SoundCues.Impact_Stone_LargeLouder')
	OpenSounds(7)=(Time=1.463931,Volume=0.800000,Pitch=1.000000,Sound=SoundCue'Ambient_NonLoop.AmbientNonLoop.EarthLiftingClosesCue')
	OpenSounds(8)=(Time=2.189321,Volume=0.800000,Pitch=1.000000,Sound=SoundCue'Ambient_NonLoop.AmbientNonLoop.EarthRippingsCue')
	OpenSounds(9)=(Time=1.081960,Volume=0.800000,Pitch=1.000000,Sound=SoundCue'Music_Stingers.stinger_ring03Cue')
	OpenSounds(10)=(Time=4.140577,Volume=0.800000,Pitch=1.000000,Sound=SoundCue'Music_Stingers.stinger_suspense15Cue')
	OpenSounds(11)=(Time=9.068769,Volume=1.600000,Pitch=1.000000,Sound=SoundCue'Locust_Drone_Chatter_Cue.LocustBreathCue')

	FirstShake=(TimeDuration=3.000000,RotAmplitude=(X=56,Y=56,Z=112),RotFrequency=(X=21,Y=21,Z=50),LocAmplitude=(X=0,Y=1.4,Z=2.8),LocFrequency=(X=2.1,Y=21.0,Z=41.0),FOVAmplitude=1.1,FOVFrequency=11.1)
	SecondShake=(TimeDuration=3.000000,RotAmplitude=(X=90,Y=90,Z=180),RotFrequency=(X=29,Y=29,Z=70),LocAmplitude=(X=0,Y=2.8,Z=4.5),LocFrequency=(X=2.8,Y=28.0,Z=53.0),FOVAmplitude=1.9,FOVFrequency=15.1)

	Components.Empty

	Begin Object Class=DynamicLightEnvironmentComponent Name=MyLightEnvironment
	    LightShadowMode=LightShadow_Modulate
	    bDynamic=FALSE
	    bCastShadows=FALSE
	    TickGroup=TG_DuringAsyncWork
	    MinTimeBetweenFullUpdates=1.0f
	    InvisibleUpdateTime=5.0f
	End Object
	LightEnvironment=MyLightEnvironment
	Components.Add(MyLightEnvironment)

	// defaults for the static meshes
	LightingChannels=(bInitialized=TRUE,BSP=TRUE,Static=TRUE,Dynamic=FALSE)


	Begin Object Class=MaterialInstanceConstant Name=CoverMeshInst
		Parent=MaterialInterface'E_Hole_Test.E_Hole_Mat'
		ScalarParameterValues(0)=(ParameterName=Ramp,ParameterValue=0.f)
		TextureParameterValues(0)=(ParameterName=SourceTexture,ParameterValue=Texture'EngineResources.WhiteSquareTexture')
	End Object
	MatInst=CoverMeshInst

	// the base hole box with collision
	Begin Object Class=StaticMeshComponent Name=HoleBox
		StaticMesh=StaticMesh'E_Hole_Test.S_Locust_E_Hole_Box'
		CollideActors=TRUE
		BlockActors=TRUE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=TRUE
		BlockRigidBody=TRUE
		bCastDynamicShadow=FALSE
		bUsePrecomputedShadows=TRUE
		Translation=(Z=-16)
		LightingChannels=(bInitialized=TRUE,BSP=TRUE,Static=TRUE,Dynamic=FALSE)
		CastShadow=FALSE
	End Object
	Components.Add(HoleBox)
	BaseMesh=HoleBox

	Begin Object Class=StaticMeshComponent Name=HoleCover
		StaticMesh=StaticMesh'E_Hole_Test.S_Ehole_Cover'
		CollideActors=TRUE
		BlockActors=TRUE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=TRUE
		BlockRigidBody=TRUE
		bCastDynamicShadow=FALSE
		bUsePrecomputedShadows=TRUE
		Translation=(Z=497)
		Scale3D=(X=1.0856,Y=1.0856,Z=1.0856)
		LightingChannels=(bInitialized=TRUE,BSP=TRUE,Static=TRUE,Dynamic=FALSE)
		CastShadow=FALSE
	End Object
	Components.Add(HoleCover)
	CoverMesh=HoleCover

	Begin Object Class=StaticMeshComponent Name=BrokenHole
		StaticMesh=StaticMesh'E_Hole_Test.S_Locust_E_Hole_Box_Broken'
		HiddenGame=TRUE
		CollideActors=FALSE
		BlockActors=FALSE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=TRUE
		BlockRigidBody=TRUE
		bCastDynamicShadow=FALSE
		bUsePrecomputedShadows=TRUE
		Translation=(Z=-16)
		LightingChannels=(bInitialized=TRUE,BSP=TRUE,Static=TRUE,Dynamic=FALSE)
	CastShadow=FALSE
    End Object
	Components.Add(BrokenHole)
	BrokenMesh=BrokenHole

	Begin Object Class=StaticMeshComponent Name=HoleFog
		StaticMesh=StaticMesh'UN_Volumetrics.Fogsheet.Mesh.S_UN_Volumetrics_Fogsheet_01'
		Materials(0)=MaterialInstanceConstant'Locust_EHole.Materials.Locust_Ehole_Smokey_01_MI_02'
		CollideActors=FALSE
		BlockActors=FALSE
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=FALSE
		bCastDynamicShadow=FALSE
		bUsePrecomputedShadows=TRUE
		Translation=(Z=-108)
		Scale3D=(X=1.511054,Y=1.511054,Z=1.511054)
		bAcceptsLights=FALSE
		CastShadow=FALSE
		LightingChannels=(bInitialized=TRUE,BSP=TRUE,Static=TRUE,Dynamic=FALSE)
	End Object
	FogMesh=HoleFog
	Components.Add(HoleFog)

	Begin Object Class=StaticMeshComponent Name=HoleTunnel
		StaticMesh=StaticMesh'E_Hole_Test.S_Locust_E_Hole_Tunnel'
		CollideActors=TRUE
		BlockActors=TRUE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=TRUE
		BlockRigidBody=TRUE
		Translation=(Z=-16)
		bAcceptsLights=TRUE
		LightingChannels=(bInitialized=TRUE,BSP=TRUE,Static=TRUE,Dynamic=FALSE)
		CastShadow=FALSE
		bCastDynamicShadow=FALSE
		bUsePrecomputedShadows=TRUE
	End Object
	Components.Add(HoleTunnel)

	Begin Object Class=CylinderComponent Name=HoleCollision
		CollisionRadius=256
		CollisionHeight=16
		CollideActors=TRUE
		BlockActors=TRUE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=TRUE
		BlockRigidBody=TRUE
		Translation=(Z=-192)
	End Object
	Components.Add(HoleCollision)
	CollisionComponent=HoleCollision

	Begin Object Class=CylinderComponent Name=HoleCork
		CollisionRadius=256
		CollisionHeight=16
		CollideActors=FALSE
		BlockActors=TRUE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=TRUE
		BlockRigidBody=TRUE
		Translation=(Z=-24)
	End Object
	Components.Add(HoleCork)
	CorkCylinder=HoleCork

	// the animated mesh
	// note - copied same basic functionality from SkeletalMeshActor as we don't need a full tree
	Begin Object Class=AnimNodeSequence Name=AnimNodeSeq0
	End Object

	Begin Object Class=SkeletalMeshComponent Name=HoleSkelMesh
		SkeletalMesh=SkeletalMesh'E_Hole_Test.SK_Locust_E_Hole'
		Animations=AnimNodeSeq0
		AnimSets=(AnimSet'E_Hole_Test.Locust_E_Hole_anim_set')
		CollideActors=FALSE
		BlockActors=FALSE
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=FALSE
		bCastDynamicShadow=FALSE
    	Translation=(Z=-15)
		LightEnvironment=MyLightEnvironment
		LightingChannels=(bInitialized=TRUE,Dynamic=FALSE)
		CastShadow=FALSE
	End Object
	Mesh=HoleSkelMesh
	Components.Add(HoleSkelMesh)

	MeshTranslationAmount=(Z=13)
	MeshTranslationStartTime=4.31f
	MeshTranslationEndTime=4.44f

	Begin Object Class=ParticleSystemComponent Name=InitDust
		Template=ParticleSystem'War_Level_Effects.E_Hole.P_E_Hole_Init_Dust'
		bAutoActivate=False
		bAcceptsLights=FALSE
    End Object
	Components.Add(InitDust)

	Begin Object Class=ParticleSystemComponent Name=PreDust
	Template=ParticleSystem'War_Level_Effects.E_Hole.P_E_Hole_Pre_Dust'
	bAutoActivate=False
		Translation=(Z=64)
		bAcceptsLights=FALSE
    End Object
	Components.Add(PreDust)

	Begin Object Class=ParticleSystemComponent Name=HoleDust
		Template=ParticleSystem'War_Level_Effects.E_Hole.P_E_Hole_Dust'
		bAutoActivate=False
		Translation=(Z=24)
		bAcceptsLights=FALSE
    End Object
	Components.Add(HoleDust)

    Begin Object Class=ParticleSystemComponent Name=HoleClose
		Template=ParticleSystem'War_Level_Effects.E_Hole.P_E_Hole_Close'
		bAutoActivate=False
		bAcceptsLights=FALSE
    End Object
	Components.Add(HoleClose)

	Begin Object Class=ParticleSystemComponent Name=Vapors
		Template=ParticleSystem'War_Level_Effects.Emergence_Vapor.P_Emergence_surface'
		bAutoActivate=False
		Translation=(Z=-80)
		bAcceptsLights=FALSE
    End Object
	Components.Add(Vapors)

    Begin Object Class=PointLightComponent Name=ExplosionLightComp
		Radius=384.000000
		Brightness=0.000000
		LightColor=(B=62,G=153,R=236,A=0)
		LightingChannels=(BSP=False,Dynamic=True,Static=FALSE)
		LightAffectsClassification=LAC_USER_SELECTED
		Translation=(Z=-36)
    End Object
	ExplosionLight=ExplosionLightComp
	Components.Add(ExplosionLightComp)

	Begin Object Class=PointLightComponent Name=ImulsionLightComp
	    bEnabled=FALSE  // this gets turned on in PostBeginPlay
	    Radius=512.000000
		Brightness=2.000000
		LightColor=(B=123,G=253,R=250,A=0)
		LightingChannels=(BSP=False,Dynamic=True,Static=TRUE)
		LightAffectsClassification=LAC_USER_SELECTED
		Translation=(Z=-84)
		bForceDynamicLight=TRUE
		CastDynamicShadows=FALSE
		CastShadows=FALSE
		CastStaticShadows=FALSE
		bCastCompositeShadow=FALSE
		bAffectCompositeShadowDirection=FALSE
    End Object
	ImulsionLight=ImulsionLightComp
	Components.Add(ImulsionLightComp)

	OpenParticles(0)=(Time=4.23f,ParticleSystem=InitDust)
	OpenParticles(1)=(Time=1.5f,ParticleSystem=PreDust)
	OpenParticles(2)=(Time=6.73f,ParticleSystem=HoleDust)
	OpenParticles(3)=(Time=8.06f,ParticleSystem=Vapors)

	CloseParticles(0)=(Time=0.f,ParticleSystem=HoleClose)

	RampInterpCurve=(Points=((InVal=1.654155,ArriveTangent=0.513605,LeaveTangent=0.513605,InterpMode=CIM_CurveBreak),(InVal=4.005690,OutVal=0.400012,ArriveTangent=0.854549,LeaveTangent=4.946238,InterpMode=CIM_CurveBreak),(InVal=5.082332,OutVal=1.507296,ArriveTangent=0.246090,LeaveTangent=10.990778,InterpMode=CIM_CurveBreak)))
	RampStartTime=2.f
	RampEndTime=5.f

	Begin Object Class=EmergenceHoleRenderingComponent Name=HoleRenderer
	End Object
	Components.Add(HoleRenderer)

	Begin Object Class=StaticMeshComponent Name=DebrisMesh0
		StaticMesh=StaticMesh'E_Hole_Test.E_Hole_rubble_Smesh'
		Materials(0)=Material'E_Hole_Test.E_Hole_rubble_Mat'
		CollideActors=False
		BlockActors=TRUE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=TRUE
		BlockRigidBody=FALSE
		HiddenGame=TRUE
		bAcceptsLights=TRUE
		LightingChannels=(bInitialized=TRUE,BSP=TRUE,Static=TRUE,Dynamic=FALSE)
		CastShadow=FALSE
		bCastDynamicShadow=FALSE
		bUsePrecomputedShadows=TRUE
	End Object
	Components.Add(DebrisMesh0)
	RubbleMeshes.Add(DebrisMesh0)

	OpenDelegates(0)=(Time=2.5f,Delegate=GearSpawner_EmergenceHoleBase.PlayFirstCameraShake)
	OpenDelegates(1)=(Time=3.5f,Delegate=GearSpawner_EmergenceHoleBase.PlaySecondCameraShake)
	OpenDelegates(2)=(Time=3.85f,Delegate=GearSpawner_EmergenceHoleBase.PlayOpenAnimation)
	OpenDelegates(8)=(Time=4.f,Delegate=GearSpawner_EmergenceHoleBase.BumpPlayers)
	OpenDelegates(3)=(Time=4.23f,Delegate=GearSpawner_EmergenceHoleBase.UnhideBrokenMesh)
	OpenDelegates(4)=(Time=6.85f,Delegate=GearSpawner_EmergenceHoleBase.DoOpeningEHoleRadialDamage)
	OpenDelegates(5)=(Time=7.f,Delegate=GearSpawner_EmergenceHoleBase.TurnOffMeshCollision)
	OpenDelegates(6)=(Time=7.2f,Delegate=GearSpawner_EmergenceHoleBase.ClearAttached)
	OpenDelegates(7)=(Time=9.3f,Delegate=GearSpawner_EmergenceHoleBase.FinishedOpening)


	// explosion when the ehole opens
// 	Begin Object Class=GearExplosion Name=ExploTemplate0
// 	   MyDamageType=class'GDT_EmergenceHoleOpening'
// 	   MomentumTransferScale=0.f	// Scale momentum defined in DamageType
// 	   ActorClassToIgnoreForDamage=class'GearGame.GearPawn_LocustBase'
// 	   ActorClassToIgnoreForKnockdownsAndCringes=class'GearGame.GearPawn_LocustBase'
// 
// 	   FractureMeshRadius=100.0
// 	   FracturePartVel=300.0
// 
// 	   Damage=700
// 	   DamageRadius=384
// 	   DamageFalloffExponent=1.0f
// 
// 		KnockDownRadius=700
// 		CringeRadius=1024
// 	End Object
// 	ExplosionTemplate=ExploTemplate0

}
