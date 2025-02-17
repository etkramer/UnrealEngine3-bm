
/**
 * GearInv_GrapplingHook
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearInv_GrapplingHook extends GearMeleeTarget
	config(Pawn);

var SkeletalMeshComponent		GrapplingHookMesh;

/** How much damage it takes to detach. */
var config protected float		Health;

/** Tweakable params for controlling how the hook looks when it detaches. */
var() protected const vector	DetachImpulseExtra;
var() protected const float		DetachImpulseMag;

function TakeDamage(int DamageAmount, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	if (WorldInfo.NetMode != NM_Client)
	{
		Health -= DamageAmount;

		if (Health <= 0 && Physics != PHYS_RigidBody)
		{
			KnockOff(EventInstigator);
		}
	}
}

simulated final function KnockOff(Controller EventInstigator)
{
	local GearPawn OwnerGP;
	local GearAnim_Slot AnimSlot;
	local vector DetachMomentum;

	// go "ragdoll"
	GrapplingHookMesh.PhysicsWeight = 1.0f;
	GrapplingHookMesh.SetHasPhysicsAssetInstance(TRUE);

	SetPhysics(PHYS_RigidBody);

	// Make sure all motors are off on joints.
	GrapplingHookMesh.PhysicsAssetInstance.SetAllMotorsAngularPositionDrive(FALSE, FALSE, GrapplingHookMesh, TRUE);
	GrapplingHookMesh.PhysicsAssetInstance.SetAllBodiesFixed(FALSE);

	// Set physics to NOT be updated from Kinematic data
	GrapplingHookMesh.bUpdateJointsFromAnimation = FALSE;
	GrapplingHookMesh.bUpdateKinematicBonesFromAnimation = FALSE;

	OwnerGP = GearPawn(Owner);
	if (OwnerGP != None)
	{
		AnimSlot = OwnerGP.GetGrapplingHookAnimSlot();

		// stop any animations
		if (AnimSlot != None)
		{
			AnimSlot.StopAnim();
		}

		if (OwnerGP.Role == ROLE_Authority)
		{
			// kill the climbing pawn, let him fall
			OwnerGP.TakeDamage(99999.f, EventInstigator, OwnerGP.Location, -vector(Rotation), class'GDT_FallOffGrappleRope');
		}
	}

	// Wake up rigid body
	GrapplingHookMesh.WakeRigidBody();

	DetachMomentum = -vector(Rotation) * DetachImpulseMag + DetachImpulseExtra;
	GrapplingHookMesh.AddImpulse(DetachMomentum);

	// die soon
	LifeSpan = 5.f;
}

defaultproperties
{
	DetachImpulseMag=50
	DetachImpulseExtra=(Z=30)

	bNoEncroachCheck=TRUE

	CollisionType=COLLIDE_BlockWeapons
	bCollideActors=TRUE
	bProjTarget=TRUE

	Begin Object Class=SkeletalMeshComponent Name=GrapplingHookMesh0
		SkeletalMesh=SkeletalMesh'Locust_GrappleHook.Mesh.Locust_GrappleHook'
		AnimSets(0)=AnimSet'Locust_GrappleHook.Anims.Locust_GrappleHook_Animset'
		AnimTreeTemplate=AnimTree'Locust_GrappleHook.Locust_GrappleHookAnimTree'
		PhysicsAsset=PhysicsAsset'Locust_GrappleHook.Mesh.Locust_GrappleHook_Physics'

		RBChannel=RBCC_Default
		RBCollideWithChannels=(Default=TRUE,BlockingVolume=TRUE,EffectPhysics=TRUE,GameplayPhysics=TRUE,Pawn=TRUE)

		bUpdateSkelWhenNotRendered=TRUE
		CollideActors=TRUE
		BlockActors=TRUE
		BlockNonZeroExtent=TRUE // so you can melee it
		BlockZeroExtent=TRUE
		BlockRigidBody=FALSE
	End Object
	Components.Add(GrapplingHookMesh0)
	GrapplingHookMesh=GrapplingHookMesh0
	CollisionComponent=GrapplingHookMesh0
}
