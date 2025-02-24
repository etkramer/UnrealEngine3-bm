/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*
* Defines an seeder for Nemacysts to shoot out of
*/
class GearSpawner_SireSpawnTube extends GearSpawner
	DependsOn(GearPawn)
	placeable;

/** Static mesh component for tank itself*/
var()	StaticMeshComponent Mesh;
var()   StaticMeshComponent UnBrokenGlassMesh;
var()   StaticMeshComponent BrokenGlassMesh;
/** mesh of idle sire floating in tank */
var() SkeletalMeshComponent IdleSireMesh;
/** cached custom anim node sequence reference */
var GearAnim_Slot FullBodySlot;

var ParticleSystem			BreakEffect;
var ParticleSystemComponent BreakEffectComp;

var() const LightEnvironmentComponent LightEnvironment;

var repnotify bool bGlassBroken;

var SoundCue GlassBreakNoise;
var SoundCue SireAwakeningNoise;

replication
{
	if (bNetDirty)
		bGlassBroken;
}

simulated event ReplicatedEvent(name VarName)
{
	if (VarName == 'bGlassBroken')
	{
		if(bGlassBroken)
		{
			PlayBreakEffects();
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

simulated function PlayBreakEffects()
{
	// PS
	BreakEffectComp.SetTemplate(BreakEffect);
	BreakEffectComp.SetHidden(FALSE);
	BreakEffectComp.ActivateSystem();

	// swap meshes
	UnBrokenGlassMesh.SetHidden(TRUE);
	UnBrokenGlassMesh.DetachFromAny();
	UnBrokenGlassMesh=none;

	BrokenGlassMesh.SetHidden(FALSE);


	// remove sire from inside
	DetachComponent(IdleSireMesh);
	IdleSireMesh = None;

	PlaySound(GlassBreakNoise);
	SetTimer(1.0,FALSE,nameof(PlayAwakeningNoise));
}

simulated function PlayAwakeningNoise()
{
	PlaySound(SireAwakeningNoise);
}

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	SpawnSlots[0].LocationOffset *= DrawScale;
	// cache fullbody slot
	FullBodySlot = GearAnim_Slot(IdleSireMesh.FindAnimNode('Custom_FullBody'));
	PlayIdleTankAnim();
}

event HandleSpawn(GearPawn NewSpawn, int SlotIdx)
{
	local Gearpawn_LocustSireBase Sire;

	Super.HandleSpawn(NewSpawn,SlotIdx);
	Sire = Gearpawn_LocustSireBase(NewSpawn);
	Sire.SpawnerDrawScale = DrawScale;
	NewSpawn.ServerDoSpecialMove(GSM_Sire_TankFall,true);
	bGlassBroken = true;
	if(WorldInfo.NetMode != NM_DedicatedServer)
	{
		PlayBreakEffects();
	}
}

simulated function PlayIdleTankAnim()
{
	//`log(GetFuncName()@FullBodySlot);
	FullBodySlot.PlayCustomAnim( 'tank_idle', 1.f, 0.2f, 0.2f, TRUE );
}

defaultproperties
{
	bEdShouldSnap=TRUE
	bBlockActors=TRUE
	bCollideActors=TRUE
	bWorldGeometry=FALSE
	bGameRelevant=TRUE
	bProjTarget=TRUE
	RemoteRole=ROLE_SimulatedProxy
	bAlwaysRelevant=true
	bMovable=false
	bRouteBeginPlayEvenIfStatic=true

	Begin Object class=StaticMeshComponent Name=TankMesh0
		StaticMesh=StaticMesh'COG_Outpost_Walls.SM.Mesh.S_Outpost_Tank01Body_SM'
		bCastDynamicShadow=FALSE
		bUsePrecomputedShadows=TRUE
		LightingChannels=(bInitialized=True,Dynamic=True,CompositeDynamic=TRUE)
	End Object
	Mesh=TankMesh0
	Components.Add(TankMesh0)

	Begin Object class=StaticMeshComponent Name=glassmesh
		StaticMesh=StaticMesh'COG_Outpost_Walls.SM.Mesh.S_Outpost_Tank01_Glass_nothick'
		bCastDynamicShadow=FALSE
		bUsePrecomputedShadows=TRUE
		LightingChannels=(bInitialized=True,Dynamic=True,CompositeDynamic=TRUE)
	End Object
	UnBrokenGlassMesh=glassmesh
	Components.Add(glassmesh)

	Begin Object class=StaticMeshComponent Name=brokenglassmesh0
		StaticMesh=StaticMesh'COG_Outpost_Walls.SM.Mesh.S_Outpost_Tank01Glass_Shatter_Section_SM_LOW'
		bCastDynamicShadow=FALSE
		bUsePrecomputedShadows=TRUE
		LightingChannels=(bInitialized=True,Dynamic=True,CompositeDynamic=TRUE)
		HiddenGame=TRUE
		HiddenEditor=TRUE
	End Object
	BrokenGlassMesh=brokenglassmesh0
	Components.Add(brokenglassmesh0)


	SpawnSlots(0)=(LocationOffset=(X=0,Y=0,Z=100),RotationOffset=(yaw=16384))

	Begin Object Class=DynamicLightEnvironmentComponent Name=MyLightEnvironment
	    bDynamic=FALSE
		bCastShadows=FALSE
	    TickGroup=TG_DuringAsyncWork
	End Object
	LightEnvironment=MyLightEnvironment

	Begin Object Class=SkeletalMeshComponent Name=SireSkelMesh
		SkeletalMesh=SkeletalMesh'Locust_Sire.Locust_Sire'
		PhysicsAsset=PhysicsAsset'Locust_Sire.Locust_Sire_Physics'
		AnimTreeTemplate=AnimTree'Locust_Sire.Locust_Sire_Animtree'
		AnimSets.Empty()
		AnimSets(0)=AnimSet'Locust_Sire.Anims.Locust_Sire_Animset'
		CollideActors=FALSE
		BlockActors=FALSE
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=FALSE
		bCastDynamicShadow=FALSE
		bUsePrecomputedShadows=TRUE
		LightingChannels=(bInitialized=True,Dynamic=True,CompositeDynamic=TRUE)
		Rotation=(yaw=16384)
		LightEnvironment=MyLightEnvironment

	End Object
	Components.Add(SireSkelMesh)
	IdleSireMesh=SireSkelMesh



	Begin Object Class=ArrowComponent Name=Arrow
		ArrowColor=(R=150,G=200,B=255)
		ArrowSize=2.5
		bTreatAsASprite=True
		HiddenGame=true
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
		Rotation=(Yaw=16384)
	End Object
	Components.Add(Arrow)

	Begin Object Class=ParticleSystemComponent Name=BreakEffect0
		Translation=(z=80)
		HiddenGame=TRUE
		HiddenEditor=TRUE
	End Object
	BreakEffectComp=BreakEffect0
	Components.Add(BreakEffect0)

	BreakEffect=ParticleSystem'COG_Outpost_Walls.Effects.P_Tank_Shatter'

	GlassBreakNoise=SoundCue'Locust_Sire_Efforts.Sire.Sire_GlassBreakCue'
	SireAwakeningNoise=SoundCue'Locust_Sire_Efforts.Sire.SiresChatter_ScreamGetupandRunCue'
}
