/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*
*/
class GearSpawner_Pimple extends GearSpawner
	DependsOn(GearPawn)
	placeable;

var() SkeletalMeshComponent Mesh;
var() StaticMeshComponent ClosedSM;

var ParticleSystem			SpawnParticleEffect;
var ParticleSystemComponent SpawnParticleComp;

var() const LightEnvironmentComponent LightEnvironment;

var repnotify bool bOpen;
var repnotify int  SpawnCount;

var SoundCue SpawnNoise;

/** cached custom anim node sequence reference */
var GearAnim_Slot FullBodySlot;

var() ScreenShakeStruct CamShake;
var() bool bPlaySpawnAudio;

replication
{
	if (ROLE==ROLE_Authority && bNetDirty)
		bOpen,SpawnCount;
}

simulated event ReplicatedEvent(name VarName)
{
	if (VarName == 'bOpen')
	{
		if(bOpen)
		{
			PlayOpenEffects();
		}
	}
	else if(VarName == 'SpawnCount')
	{
		PlaySpawnEffects();
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

simulated function PlaySpawnEffects()
{
	// PS
	SpawnParticleComp.SetTemplate(SpawnParticleEffect);
	SpawnParticleComp.SetHidden(FALSE);
	SpawnParticleComp.ActivateSystem();
	if(bPlaySpawnAudio)
	{
		PlaySound(SpawnNoise);
	}
}

simulated function PlayOpenEffects()
{
	`log(GetFuncName()@FullBodySlot);
	// swap meshes
	ClosedSM.SetHidden(TRUE);
	FullBodySlot.PlayCustomAnim( 'Burst', 1.f, 0.2f, 0.2f, FALSE );
	Mesh.SetHidden(FALSE);
	PlayCameraShake();

}

function OnToggle(SeqAct_Toggle Action)
{
	Super.OnToggle(Action);
	Open();
}
function Open()
{
	bOpen=true;
	PlayOpenEffects();
	SetCollision(TRUE,TRUE);
	SetTimer(0.5f,FALSE,nameof(BumpPlayers));
}


simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	SpawnSlots[0].LocationOffset *= DrawScale;
	// cache fullbody slot
	FullBodySlot = GearAnim_Slot(Mesh.FindAnimNode('Custom_FullBody'));
}

event HandleSpawn(GearPawn NewSpawn, int SlotIdx)
{
	local rotator Rot;
	Super.HandleSpawn(NewSpawn,SlotIdx);
	NewSpawn.ServerDoSpecialMove(SM_Emerge_Type1,true);
	SpawnCount++;

	Rot = NewSpawn.Rotation;
	//Rot.yaw = Rand(65535);
	NewSpawn.SetRotation( Rot );
	if(WorldInfo.NetMode != NM_DedicatedServer && !bOpen)
	{
		Open();
	}
	PlaySpawnEffects();

}

simulated final function PlayCameraShake()
{
	local GearPC PC;

	foreach WorldInfo.AllControllers(class'GearPC',PC)
	{
		PC.ClientPlayCameraShake(CamShake, true);
	}
}

simulated final function BumpPlayers()
{
	local GearPawn_COGGear Victim;
	foreach VisibleCollidingActors( class'GearPawn_COGGear', Victim, 384, Location )
	{
		if (Victim.Base == self)
		{
			Victim.SetBase(None);
			Victim.AddVelocity(vector(Rotation) * 200  + vect(0,0,200),Victim.Location,class'GDT_EmergenceHoleOpening');
		}
	}
}

defaultproperties
{
	bEdShouldSnap=TRUE
	bBlockActors=FALSE
	bCollideActors=TRUE
	bWorldGeometry=FALSE
	bGameRelevant=TRUE
	bProjTarget=TRUE
	RemoteRole=ROLE_SimulatedProxy
	bAlwaysRelevant=true
	bMovable=false
	bRouteBeginPlayEvenIfStatic=true

	SpawnSlots(0)=(LocationOffset=(X=0,Y=0,Z=50))

	Begin Object Class=DynamicLightEnvironmentComponent Name=MyLightEnvironment
	    LightShadowMode=LightShadow_Modulate
		bSynthesizeSHLight=FALSE
		bDynamic=FALSE
		bCastShadows=FALSE
		TickGroup=TG_DuringAsyncWork
		MinTimeBetweenFullUpdates=1.0f
		InvisibleUpdateTime=5.0f
	End Object
	LightEnvironment=MyLightEnvironment
	Components.Add(MyLightEnvironment)

	Begin Object Class=SkeletalMeshComponent Name=PimpleMesh
		SkeletalMesh=SkeletalMesh'Locust_Nemaslug.Slug_Hole.Mesh.SK_Slug_Hole'
		PhysicsAsset=PhysicsAsset'Locust_Nemaslug.Slug_Hole.Mesh.SK_Slug_Hole_Physics'
		bHasPhysicsAssetInstance=true
		AnimTreeTemplate=AnimTree'Locust_Nemaslug.Anim_Slug_Hole_AT'
		AnimSets.Empty()
		AnimSets(0)=AnimSet'Locust_Nemaslug.Slug_Hole.Mesh.Anim_Slug_Hole'
		CollideActors=TRUE
		BlockActors=FALSE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=TRUE
		bCastDynamicShadow=FALSE
		bUsePrecomputedShadows=TRUE
		LightingChannels=(bInitialized=True,Dynamic=True,CompositeDynamic=TRUE)
		LightEnvironment=MyLightEnvironment
		HiddenGame=TRUE
		HiddenEditor=TRUE
		Translation=(z=-15)
	End Object
	Components.Add(PimpleMesh)
	Mesh=PimpleMesh

	Begin Object Class=CylinderComponent Name=CollisionCylinder
		CollisionRadius=+085.000000
		CollisionHeight=+070.000000
		BlockNonZeroExtent=TRUE
		BlockZeroExtent=FALSE
		BlockActors=TRUE
		CollideActors=TRUE
	End Object
	Components.Add(CollisionCylinder)
	CollisionComponent=CollisionCylinder



	Begin Object Class=ParticleSystemComponent Name=SpawnEffect0
		Translation=(z=50)
		HiddenGame=TRUE
		HiddenEditor=TRUE
		Scale=1.2f
	End Object
	SpawnParticleComp=SpawnEffect0
	Components.Add(SpawnEffect0)

	SpawnParticleEffect=ParticleSystem'Locust_Nemaslug.Slug_Hole.Effects.P_Burst'

	SpawnNoise=SoundCue'Foley_Flesh.Flesh.GibBodyChunkHugeCue'

	Begin Object Class=ArrowComponent Name=Arrow
		ArrowColor=(R=150,G=200,B=255)
		ArrowSize=2.5
		bTreatAsASprite=True
		HiddenGame=true
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
		Translation=(z=100)
	End Object
	Components.Add(Arrow)

	Begin object class=StaticMeshComponent Name=ClosedMesh0
		StaticMesh=StaticMesh'Locust_Nemaslug.Slug_Hole.Mesh.SM_Slughole_Closed'
		bAllowApproximateOcclusion=TRUE
		bCastDynamicShadow=FALSE
		bForceDirectLightMap=TRUE
		bUsePrecomputedShadows=TRUE
	End Object
	Components.Add(ClosedMesh0)
	ClosedSM=ClosedMesh0

	DrawScale=1.2

	CamShake=(TimeDuration=3.000000,RotAmplitude=(X=56,Y=56,Z=112),RotFrequency=(X=21,Y=21,Z=50),LocAmplitude=(X=0,Y=1.4,Z=2.8),LocFrequency=(X=2.1,Y=21.0,Z=41.0),FOVAmplitude=1.1,FOVFrequency=11.1)

	bPlaySpawnAudio=true
}
