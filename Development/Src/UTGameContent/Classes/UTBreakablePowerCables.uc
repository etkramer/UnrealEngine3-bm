/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTBreakablePowerCables extends Actor
	placeable
	hidecategories(Collision);


var()	SkeletalMeshComponent	Mesh;
var()	StaticMeshComponent		BoxComp;
var		ParticleSystemComponent	SparkComponent0;
var		ParticleSystemComponent	SparkComponent1;
var		PointLightComponent		SparkLight;
var		AudioComponent			SparkNoise;

var		AnimNodeSequence		AnimNode;

var		ParticleSystem			BreakEffect;

var		bool					bBroken;

var		vector2D				ReSparkInterval;
var		float					TimeToRespawn;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	// If this is on dedicated server, detach all components and basically shut down
	if(WorldInfo.NetMode == NM_DedicatedServer)
	{
		SetCollision(FALSE, FALSE);
		DetachComponent(Mesh);
		DetachComponent(BoxComp);
		DetachComponent(SparkLight);
		DetachComponent(SparkNoise);
		BeginState('IgnoreItAll');
	}
	// If not dedicated - attach particle components to ends of cable
	else
	{
		Mesh.AttachComponent(SparkComponent0, 'BoneEndL');
		Mesh.AttachComponent(SparkLight, 'BoneEndL');
		Mesh.AttachComponent(SparkNoise, 'BoneEndL');
		Mesh.AttachComponent(SparkComponent1, 'BoneEndR');

		// Cache pointer to AnimNodeSequence
		AnimNode = AnimNodeSequence(Mesh.Animations);
	}
}

/** State used to stop anything from happening on dedicated server. */
state IgnoreItAll
{
	ignores Touch, TakeDamage, Tick;
}

/** Something has touched the cable - do all the effects and stuff! */
simulated function BreakCable()
{
	local vector BreakPos;
	local float SparkTime;

	// Play animation of cable breaking
	AnimNode.PlayAnim(FALSE, 1.0, 0.0);

	// Get location in middle of rope
	BreakPos = Mesh.GetBoneLocation('BoneEndL');

	// Play sparky sound
	SparkNoise.SoundCue = SoundCue'A_Ambient_NonLoops.HiTech.spark_large_LargeRadius_Cue';
	SparkNoise.Play();

	// Play big spark effect
	WorldInfo.MyEmitterPool.SpawnEmitter(BreakEffect, BreakPos);

	// Make emitter on each end of the cable spark
	SparkComponent0.SetActive(true);
	SparkComponent1.SetActive(true);
	SetTimer(0.3, FALSE, 'TurnOffSparkEffects');

	// Turn on light, and set timer to turn it off again
	SparkLight.SetEnabled(TRUE);
	SetTimer(0.2, FALSE, 'TurnOffSparkLight');

	// Set flag to indicate cables are broken
	bBroken = TRUE;

	// Start a timer for causing extra sparks from the cables
	SparkTime = RandRange(ReSparkInterval.X, ReSparkInterval.Y);
	SetTimer(SparkTime, FALSE, 'SparkCable');

	// Start a countdown to resetting the cables.
	TimeToRespawn = 30.0;
	SetTimer(1.0, TRUE, 'CheckRespawn');

	// Turn off collision now
	SetCollision(FALSE, FALSE);
}

/** */
simulated function TurnOffSparkLight()
{
	SparkLight.SetEnabled(FALSE);
}

/** */
simulated function TurnOffSparkEffects()
{
	SparkComponent0.DeactivateSystem();
	SparkComponent1.DeactivateSystem();
}

/** */
simulated function SparkCable()
{
	local float SparkTime;

	SparkComponent0.SetActive(true);
	SparkComponent1.SetActive(true);
	SetTimer(0.3, FALSE, 'TurnOffSparkEffects');

	SparkNoise.Stop();
	SparkNoise.SoundCue = SoundCue'A_Ambient_NonLoops.HiTech.spark_small_Cue';
	SparkNoise.Play();

	AnimNode.SetAnim( (FRand() < 0.5) ? 'Spark1' : 'Spark2' );
	AnimNode.PlayAnim(FALSE, 1.0, 0.0);

	SparkTime = RandRange(ReSparkInterval.X, ReSparkInterval.Y);
	SetTimer(SparkTime, FALSE, 'SparkCable');
}

/** Used to countdown to respawn. */
simulated event CheckRespawn()
{
	// If destroyed, countdown to respawn.
	if(bBroken)
	{
		TimeToRespawn -= 1.0;

		if(TimeToRespawn < 0.f && (Mesh.LastRenderTime < WorldInfo.TimeSeconds - 1.0f))
		{
			// Put cable back to start position
			AnimNode.SetAnim('Break');
			AnimNode.SetPosition(0.0, FALSE);

			// Turn off sparkinf and respawning timers
			ClearTimer('SparkCable');
			ClearTimer('CheckRespawn');

			// Turn collision back on
			SetCollision(TRUE, FALSE);

			// Reset flag
			bBroken = FALSE;
		}
	}
}

/** Look for vehicles touching the cable. */
simulated function Touch(Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal)
{
	if(bBroken)
	{
		return;
	}

	if( Vehicle(Other) != None )
	{
		BreakCable();
	}
};


defaultproperties
{
	bCollideActors=TRUE
	bBlockActors=FALSE
	bPathColliding=FALSE
	bNoDelete=TRUE
	RemoteRole=ROLE_SimulatedProxy

	Begin Object Class=StaticMeshComponent Name=MyCollisionComp
		StaticMesh=StaticMesh'UN_SimpleMeshes.TexPropCube_Dup'
		BlockRigidBody=FALSE
		HiddenGame=TRUE
		CollideActors=TRUE
		BlockActors=FALSE
		Scale3D=(X=4.0,Y=0.05,Z=0.13)
		Translation=(Z=244.0)
	End Object
	Components.Add(MyCollisionComp)
	BoxComp=MyCollisionComp
	CollisionComponent=MyCollisionComp


	Begin Object Class=PointLightComponent Name=MySparkLight
		Radius=1024.0
		Brightness=8.0
		CastShadows=FALSE
		bEnabled=FALSE
		FalloffExponent=4.0
		LightingChannels=(BSP=TRUE,Static=TRUE,Dynamic=TRUE,bInitialized=TRUE)
	End Object
	SparkLight=MySparkLight

	Begin Object Class=AudioComponent Name=MySparkNoise
		SoundCue=SoundCue'A_Ambient_NonLoops.HiTech.spark_large_LargeRadius_Cue'
		VolumeMultiplier=5.0
	End Object
	Components.Add(MySparkNoise)
	SparkNoise=MySparkNoise


	Begin Object Class=DynamicLightEnvironmentComponent Name=MyLightEnvironment
		bEnabled=TRUE
		bDynamic=FALSE
	End Object
	Components.Add(MyLightEnvironment)

	Begin Object Class=AnimNodeSequence Name=MySeqNode
		AnimSeqName=Break
	End Object

	Begin Object Class=SkeletalMeshComponent Name=MySkeletalMeshComponent
		SkeletalMesh=SkeletalMesh'Envy_Level_Effects_2.DM_HeatRay.SK_PowerWire'
		AnimSets.Add(AnimSet'Envy_Level_Effects_2.DM_HeatRay.K_PowerWire_Anims')
		Animations=MySeqNode
		LightEnvironment=MyLightEnvironment
		BlockRigidBody=FALSE
		CollideActors=FALSE
		BlockActors=FALSE
		CastShadow=FALSE
		bUseAsOccluder=FALSE
	End Object
	Components.Add(MySkeletalMeshComponent)
	Mesh=MySkeletalMeshComponent


	Begin Object Class=ParticleSystemComponent Name=MySparkComponent0
		Template=ParticleSystem'Envy_Level_Effects_2.DM_HeatRay.P_PowerWire_Sparks_01'
		bAutoActivate=FALSE;
	End Object
	SparkComponent0=MySparkComponent0


	Begin Object Class=ParticleSystemComponent Name=MySparkComponent1
		Template=ParticleSystem'Envy_Level_Effects_2.DM_HeatRay.P_PowerWire_Sparks_01'
		bAutoActivate=FALSE;
	End Object
	SparkComponent1=MySparkComponent1

	ReSparkInterval=(X=1.0,Y=2.0)

	BreakEffect=ParticleSystem'Envy_Level_Effects_2.DM_HeatRay.P_PowerWireBreak_Spark'
}
