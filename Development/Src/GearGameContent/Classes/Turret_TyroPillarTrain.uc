/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Turret_TyroPillarTrain extends Turret
	config(Pawn);

/** 
 * Tyro Pillar train turret
 *
 */

/** @See Pawn::FaceRotation */
simulated function FaceRotation( rotator NewRotation, float DeltaTime )
{
	SetRotation( NewRotation );
}

/*
simulated function Tick( float DeltaTime )
{
	local Rotator BaseRotation, CannonRotation;

	// Control bones rotation, to match desired aim
	if ( Mesh != None )
	{
		// first set base rotation (only Yaw)
		BaseRotation.Pitch	= 0;
		BaseRotation.Yaw	= DesiredRelativeAim.Yaw;
		BaseRotation.Roll	= 0;
		Mesh.SetBoneRotation(BaseBone, BaseRotation); // FIXME!

		// set cannon rotation: only pitch, as Yaw is inherited from parent bone.
		CannonRotation.Pitch	= -1 * DesiredRelativeAim.Pitch;
		CannonRotation.Yaw		= 0;
		CannonRotation.Roll		= 0;
		Mesh.SetBoneRotation(PitchBone, CannonRotation); // FIXME!
	}
}

	Begin Object Class=SkeletalMeshComponent Name=SkelMeshComponent0
		SkeletalMesh=SkeletalMesh'TyroPillar.prototype-train-turret'
		AnimSets(0)=AnimSet'TyroPillar.TyroTrainTurretAnims'
		Animations=anIdleAnim
		BlockZeroExtent=true
		CollideActors=true
		BlockRigidBody=true
		Scale=1.0
        Translation=(Z=-597.000000) //-Y,+Z=-597
		Rotation=(Yaw=32768)
	End Object
	Mesh=SkelMeshComponent0
	Components.Add(SkelMeshComponent0)

*/

defaultproperties
{
	bRelativeExitPos=false

	InventoryManagerClass=class'GearInventoryManager'
	DefaultInventory(0)=class'GearWeap_TyroTurret'

	Begin Object Name=CollisionCylinder
        CollisionHeight=200.000000
        CollisionRadius=160.000000
		BlockZeroExtent=false
	End Object

	Begin Object Class=AnimNodeSequence Name=anIdleAnim
    	AnimSeqName=still
    End object
	//ExitPositions(0)=(Y=-400)
	//ExitPositions(1)=(Z=400)
	PitchBone=train-turret-gun
	BaseBone=train-turret-body
	ViewPitchMin=-4096
	ViewPitchMax=8192
	POV=(DirOffset=(X=-5,Y=0,Z=5),Distance=100,fZAdjust=-350)
	CannonFireOffset=(X=160,Y=0,Z=24)

}
