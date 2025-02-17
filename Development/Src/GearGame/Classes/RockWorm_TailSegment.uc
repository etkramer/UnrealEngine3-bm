/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class RockWorm_TailSegment extends Actor
	dependsOn(Gearpawn_RockWormBase)
	native;
var repnotify GearPawn_RockWormBase WormOwner;
var RockWorm_TailSegment NextSegment;
var RockWorm_TailSegment PrevSegment;
var float AttachOffset;

var() SkeletalMeshComponent Mesh;
var() name FrontPivotBoneName;
var() name RearPivotBoneName;
var bool bThisSegmentIsTheTail;
var float LastZeroSpeedTime;

var repnotify vector InitialLocation;
var repnotify rotator InitialRotation;


cpptext
{
		virtual UBOOL ShouldTrace(UPrimitiveComponent* Primitive,AActor *SourceActor, DWORD TraceFlags);
		virtual void NotifyBump(AActor *Other, UPrimitiveComponent* OtherComp, const FVector &HitNormal);
};
replication
{
	if(ROLE==ROLE_Authority && bNetDirty)
		WormOwner,NextSegment,PrevSegment,InitialRotation,InitialLocation;
}

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	AttachOffset = VSize(Mesh.GetBoneLocation(FrontPivotBoneName) - Mesh.GetBoneLocation(RearPivotBoneName));

}

event TakeDamage(int DamageAmount, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	Super.TakeDamage(DamageAmount,EventInstigator,HitLocation,Momentum,DamageType,HitInfo,DamageCauser);

	if(bThisSegmentIsTheTail && WormOwner != none)
	{
		WormOwner.OuchYouShotMyBum();
	}
}

function AttachToChain(Actor PrevSeg)
{
	local vector PtToSnapTo;
	local vector MySnapPt;
	local GearPawn_RockWormBase Worm;
	local rotator rot;
	PrevSegment = RockWorm_TailSegment(prevSeg);
	
	if(PrevSegment != none)
	{
		PrevSegment.NextSegment = self;
	}	

	Worm = Gearpawn_RockWormBase(PrevSeg);
	if( Worm != none)
	{
		PtToSnapTo = Worm.Mesh.GetBoneLocation(Worm.RearPivotBoneName);
	}
	else
	{	
		PtToSnapTo = PrevSegment.Mesh.GetBoneLocation(PrevSegment.RearPivotBoneName);
	}

	MySnapPt = Mesh.GetBoneLocation(FrontPivotBoneName);
	rot = PrevSeg.Rotation;
	//rot.pitch=0;
	SetCollision(false);
	SetRotation(rot);
	SetLocation(PtToSnapTo + (Location-MySnapPt));
	SetCollision(true);
	InitialRotation=Rotation;
	InitialLocation=Location;
}

function SetupMesh(bool bIsTail)
{
	bThisSegmentIsTheTail=bIsTail;
	//`log(GetFuncName()@self@WormOwner@WormOwner.Mesh);
	Mesh.SetShadowParent(WormOwner.Mesh);
	Mesh.SetLightEnvironment(WormOwner.LightEnvironment);
}

simulated function ClientSetupMesh(SkeletalMeshComponent OwnerMesh, int SegmentIdx)
{
	//`log(GetFuncName()@OwnerMesh@OwnerMesh.LightEnvironment);
	bThisSegmentIsTheTail=(SegmentIdx < 0);
	Mesh.SetShadowParent(OwnerMesh);
	Mesh.SetLightEnvironment(OwnerMesh.LightEnvironment);
}

//simulated event Tick(float DeltaTime)
//{
//	local int i;
//	local vector ReachStart,ReachEnd;
//	for(i=0;i<ReachSpecsImBlocking.length;i++)
//	{
//		if(ReachSpecsImBlocking[i].IsA('MantleReachSpec'))
//		{
//			`log(GetFuncName()@self@"blocking mantle"@ReachSpecsImBlocking[i]);
//		}
//
//		ReachStart = ReachSpecsImBlocking[i].Start.Location;
//		ReachEnd = ReachSpecsImBlocking[i].End.Nav.Location;
//		DrawDebugLine(ReachStart,ReachEnd,255,0,0);
//		DrawDebugLine(ReachSpecsImBlocking[i].BlockedBy.Location,(ReachStart+ReachEnd)/2.0f,255,255,255);
//	}
//	Super.Tick(DeltaTime);
//}

defaultproperties
{
	bBlockActors=true
	bCollideActors=true
	bPathColliding=true
	bWorldGeometry=false
	bBlocksNavigation=true
	bReplicateMovement=false
	RemoteRole=ROLE_SimulatedProxy
	bAlwaysRelevant=true

	Begin Object Class=CylinderComponent Name=CollisionComp0
		CollisionRadius=45.f
		Collisionheight=50.f
		BlockNonZeroExtent=true
		BlockZeroExtent=true
		BlockActors=true
		CollideActors=true
		Translation=(z=46)
	End Object
	Components.Add(CollisionComp0)
	CollisionComponent=CollisionComp0
}
