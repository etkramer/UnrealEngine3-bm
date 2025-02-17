/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class RockWorm_TailSegmentSM extends RockWorm_TailSegment;

struct EMeshData
{
	var SkeletalMesh Mesh;
	var name		 DustSocketName;
};

var array<EMeshData> SegmentMeshes;
var name CurrentDustSocketName;

var AnimSet SegmentAnimSet;

var SkeletalMesh TailMesh;
var AnimSet TailAnimSet;
var name TailFrontPivotName;
var Name TailRearPivotName;
var PhysicsAsset TailPhysAsset;

/** Index of the tailsegment mesh we should use */
var repnotify int SegIdx;

var ParticleSystem DustPS;
var ParticleSystemComponent DustPSC;

replication
{
	if(ROLE==ROLE_Authority && bNetDirty)
		SegIdx;
}

simulated function Destroyed()
{
	Super.Destroyed();
	if(DustPSC != none)
	{
		DustPSC.DeactivateSystem();
		Mesh.DetachComponent(DustPSC);		
		DustPSC=none;
	}
}

simulated function UpdateDust()
{
	if(VSizeSq(Velocity) > 100.f)
	{
		if(!DustPSC.bIsActive)
		{
			DustPSC.ActivateSystem();
		}
	}
	else
	{
		if(DustPSC.bIsActive)
		{
			DustPSC.DeactivateSystem();
		}
	}
	SetTimer(RandRange(0.75,1.0),FALSE,nameof(UpdateDust));
}

simulated event ReplicatedEvent(name VarName)
{
	if (VarName == 'SegIdx')
	{
		if(WormOwner != none)
		{
			ClientSetupMesh(WormOwner.Mesh,SegIdx);
		}
		
	}
	else if (VarName == 'WormOwner')
	{
		if(WormOwner != none && SegIdx == -2)
		{
			ClientSetupMesh(WormOwner.Mesh,SegIdx);
		}

	}
	else if(VarName == 'InitialRotation')
	{
		SetRotation(InitialRotation);
	}
	else if(VarName == 'InitialLocation')
	{
		SetLocation(InitialLocation);
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

function SetupMesh(bool bIsTail)
{
	if(bIsTail)
	{
		Mesh.SetSkeletalMesh(TailMesh);
		Mesh.AnimSets[0]=TailAnimSet;
		Mesh.UpdateAnimations();
		Mesh.SetPhysicsAsset(TailPhysAsset);
		FrontPivotBoneName=TailFrontPivotName;
		RearPivotBoneName=TailRearPivotName;
		SegIdx=-1;
		Mesh.ForceSkelUpdate();
		CurrentDustSocketName='Dust7';
		CollisionComponent.SetTranslation(CollisionComponent.Translation + vect(-50,0,0));
	}
	else
	{
		SegIdx = rand(SegmentMeshes.length-1);
		Mesh.SetSkeletalMesh(SegmentMeshes[SegIdx].Mesh,true);
		Mesh.AnimSets[0]=SegmentAnimSet;
		Mesh.UpdateAnimations();
		Mesh.ForceSkelUpdate();
		CurrentDustSocketName=SegmentMeshes[SegIdx].DustSocketName;
	}
	
	if(WorldInfo.NetMode != NM_DedicatedServer)
	{
		DustPSC = new(self) class'ParticleSystemComponent';
		DustPSC.SetTemplate(DustPS);
		Mesh.AttachComponentToSocket(DustPSC,CurrentDustSocketName);
		UpdateDust();
	}
	Super.SetupMesh(bIsTail);
}

simulated function ClientSetupMesh(SkeletalMeshComponent OwnerMesh, int SegmentIdx)
{
	Super.ClientSetupMesh(OwnerMesh,SegmentIdx);

	if(SegmentIdx < 0)
	{
		Mesh.SetSkeletalMesh(TailMesh);
		Mesh.AnimSets[0]=TailAnimSet;
		Mesh.UpdateAnimations();
		Mesh.SetPhysicsAsset(TailPhysAsset);
		FrontPivotBoneName=TailFrontPivotName;
		RearPivotBoneName=TailRearPivotName;
		Mesh.ForceSkelUpdate();
		CurrentDustSocketName='Dust7';
	}
	else
	{
		Mesh.SetSkeletalMesh(SegmentMeshes[SegmentIdx].Mesh);
		Mesh.AnimSets[0]=SegmentAnimSet;
		Mesh.UpdateAnimations();
		Mesh.ForceSkelUpdate();
		CurrentDustSocketName=SegmentMeshes[SegmentIdx].DustSocketName;
		CollisionComponent.SetTranslation(CollisionComponent.Translation + vect(-50,0,0));
	}

	DustPSC = new(self) class'ParticleSystemComponent';
	DustPSC.SetTemplate(DustPS);
	Mesh.AttachComponentToSocket(DustPSC,CurrentDustSocketName);
	UpdateDust();
}

defaultproperties
{

	SegIdx=-2
	TailFrontPivotName=b_Tail_FrontPivot
	TailRearPivotName=b_Tail_Spine00
	FrontPivotBoneName=b_Segment_FrontPivot
	RearPivotBoneName=b_Segment_RearPivot
	TailPhysAsset=PhysicsAsset'Locust_Rockworm.Rockworm_F_Tail_Physics'
	
	SegmentMeshes(0)=(Mesh=SkeletalMesh'Locust_Rockworm.Mesh.Rockworm_F_Segment01',DustSocketName=Dust1)
	SegmentMeshes(1)=(Mesh=SkeletalMesh'Locust_Rockworm.Mesh.Rockworm_F_Segment02',DustSocketName=Dust2)
	SegmentMeshes(2)=(Mesh=SkeletalMesh'Locust_Rockworm.Mesh.Rockworm_F_Segment03',DustSocketName=Dust3)
	SegmentMeshes(3)=(Mesh=SkeletalMesh'Locust_Rockworm.Mesh.Rockworm_F_Segment04',DustSocketName=Dust4)
	SegmentMeshes(4)=(Mesh=SkeletalMesh'Locust_Rockworm.Mesh.Rockworm_F_Segment05',DustSocketName=Dust5)
	SegmentMeshes(5)=(Mesh=SkeletalMesh'Locust_Rockworm.Mesh.Rockworm_F_Segment06',DustSocketName=Dust6)
	SegmentAnimSet=AnimSet'Locust_Rockworm.Anims.Animset_Rockworm_Segment'
	TailMesh=SkeletalMesh'Locust_Rockworm.Mesh.Rockworm_F_Tail'
	TailAnimSet=AnimSet'Locust_Rockworm.Anims.Animset_Rockworm_Tail'



	Begin Object Class=SkeletalMeshComponent Name=PawnMesh
		SkeletalMesh=SkeletalMesh'Locust_Rockworm.Mesh.Rockworm_F_Segment01'
		PhysicsAsset=PhysicsAsset'Locust_Rockworm.Rockworm_F_Segment01_Physics'
		//AnimTreeTemplate=AnimTree'Locust_Rockworm.AnimTree.Rockworm_AnimTree'
		AnimTreeTemplate=AnimTree'Locust_Rockworm.AnimTree.Rockworm_AnimTree_Tailsegments'
		AnimSets(0)=AnimSet'Locust_Rockworm.Anims.Animset_Rockworm_Segment'
		BlockZeroExtent=TRUE
		//CollideActors=TRUE
		BlockRigidBody=TRUE
		//BlockActors=TRUE
		//BlockNonZeroExtent=TRUE
		RBChannel=RBCC_Pawn
		RBCollideWithChannels=(Default=TRUE,BlockingVolume=TRUE,EffectPhysics=TRUE,Pawn=TRUE)
		bIgnoreControllersWhenNotRendered=TRUE
		MinDistFactorForKinematicUpdate=0.0
		bAcceptsStaticDecals=FALSE
		bAcceptsDynamicDecals=FALSE
		bCullModulatedShadowOnBackfaces=FALSE
		bAllowAmbientOcclusion=FALSE
		bHasPhysicsAssetInstance=TRUE
		Translation=(z=-5)
		Rotation=(Yaw=-16384)
	End Object
	Mesh=PawnMesh
	//CollisionComponent=PawnMesh
	Components.Add(PawnMesh)

	DustPS=ParticleSystem'Locust_Rockworm.Effects.P_Rockworm_Ground_Dust2'
}
