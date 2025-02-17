
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_DeathAnimBase extends GearSpecialMove
	abstract;

function bool PlayDeathAnimation(class<GearDamageType> DamageType, vector HitLoc)
{
	return FALSE;
}

function DeathAnimBlendToMotors()
{
	// If we're not in rag doll physics, then we need to setup a few things.
	if( PawnOwner.Physics != PHYS_RigidBody )
	{
		// Unfix all bodies from kinematic data.
		// So now we switch to being fully physical
		PawnOwner.Mesh.PhysicsAssetInstance.SetAllBodiesFixed(FALSE);

		// Add velocity of base upon death.
		PawnOwner.AddBaseLinearVelocityUponWhenGoingRagDoll();

		// Turn off collision on cylinder component and switch to skeleton physics
		PawnOwner.CollisionComponent = PawnOwner.Mesh;

		// Allow all ragdoll bodies to collide with all physics objects (ie allow collision with things marked RigidBodyIgnorePawns)
		PawnOwner.Mesh.SetRBChannel(RBCC_DeadPawn);
		PawnOwner.Mesh.SetRBCollidesWithChannel(RBCC_Pawn, FALSE);

		// Wake up physics
		PawnOwner.Mesh.WakeRigidBody();
	}

	// Set Physics weight to 1
	PawnOwner.Mesh.PhysicsWeight = 1.f;

	// Set physics to be updated from Kinematic data
	PawnOwner.Mesh.bUpdateJointsFromAnimation = TRUE;
	PawnOwner.Mesh.bUpdateKinematicBonesFromAnimation = TRUE;

	// Turn on motors
	PawnOwner.Mesh.PhysicsAssetInstance.SetAllMotorsAngularDriveParams(5000.f, 250.f, 0.f, PawnOwner.Mesh, TRUE);
	PawnOwner.Mesh.PhysicsAssetInstance.SetAllMotorsAngularPositionDrive(TRUE, TRUE, PawnOwner.Mesh, TRUE);

	// If Pawn had broken constraints, then we need to make sure child bodies and joints are not turned back into kinematic state
	if( PawnOwner.bHasBrokenConstraints )
	{
		PawnOwner.Mesh.UpdateMeshForBrokenConstraints();
	}
}

function TurnToRagDollPhysics()
{
	// Turning to Rag Doll
	if( PawnOwner.Physics != PHYS_RigidBody )
	{
		PawnOwner.SetPhysics(PHYS_RigidBody);

		// Add velocity of base upon death.
		PawnOwner.AddBaseLinearVelocityUponWhenGoingRagDoll();
	}

	// Turn off collision on cylinder component and switch to skeleton physics
	PawnOwner.CollisionComponent = PawnOwner.Mesh;

	// disable unreal collision on ragdolls
	PawnOwner.SetCollision(TRUE, FALSE);

	if( PawnOwner.bEnableEncroachCheckOnRagdoll )
	{
		PawnOwner.bNoEncroachCheck = FALSE;
	}

	// Set Physics weight to 1
	PawnOwner.Mesh.PhysicsWeight = 1.f;

	// Allow all ragdoll bodies to collide with all physics objects (ie allow collision with things marked RigidBodyIgnorePawns)
	PawnOwner.Mesh.SetRBChannel(RBCC_DeadPawn);
	PawnOwner.Mesh.SetRBCollidesWithChannel(RBCC_Pawn, FALSE);

	// Make sure all bodies are unfixed
	PawnOwner.Mesh.PhysicsAssetInstance.SetAllBodiesFixed(FALSE);

	// Set physics to NOT be updated from Kinematic data
	PawnOwner.Mesh.bUpdateJointsFromAnimation = FALSE;
	PawnOwner.Mesh.bUpdateKinematicBonesFromAnimation = FALSE;

	// Wake up physics
	PawnOwner.Mesh.WakeRigidBody();

	// If Pawn had broken constraints, then we need to make sure child bodies and joints are not turned back into kinematic state
	if( PawnOwner.bHasBrokenConstraints )
	{
		PawnOwner.Mesh.UpdateMeshForBrokenConstraints();
	}
}

function DeathAnimRagDoll()
{
	// Setup Pawn to be in rag doll physics
	TurnToRagDollPhysics();

	// Turn off position motors
	PawnOwner.Mesh.PhysicsAssetInstance.SetAllMotorsAngularPositionDrive(FALSE, FALSE, PawnOwner.Mesh, TRUE);

	// Turn on velocity motors, to add some joint friction
	PawnOwner.Mesh.PhysicsAssetInstance.SetAllMotorsAngularDriveParams(0.f, 0.f, 0.02f, PawnOwner.Mesh, TRUE);
	PawnOwner.Mesh.PhysicsAssetInstance.SetAllMotorsAngularVelocityDrive(TRUE, TRUE, PawnOwner.Mesh, TRUE);

	PawnOwner.StopAllAnimations();

	// Spawn a blood pool
	if( !PawnOwner.IsDBNO() && PawnOwner.WorldInfo.NetMode != NM_DedicatedServer )
	{
		PawnOwner.SetTimer( 3.0f, FALSE, nameof(PawnOwner.DoSpawnABloodPool) );
	}

	// Decrease joint limits, to prevent nasty poses once in complete rag doll.
	if( !PawnOwner.bIsGore && PawnOwner.ScaleLimitTimeToGo <= 0 )
	{
		PawnOwner.ReduceConstraintLimits();
	}
}

/** 
 * Creates a spring between physics reprensation of bone and animated position.
 * To have physics match animation.
 */
final function SetSpringForBone(Name InBoneName, bool bEnable)
{
	local int				BoneIndex;
	local matrix			BoneMatrix;
	local RB_BodyInstance	BoneBody;

	// Filter through sockets and bones. Make sure we have a valid bone name to use.
	InBoneName = PawnOwner.Mesh.GetSocketBoneName(InBoneName);

	if( InBoneName == '' )
	{
		`Warn(PawnOwner.WorldInfo.TimeSeconds @ PawnOwner @ class @ GetFuncName() @ "Has non existing Bone or Socket named:" @ InBoneName @ "Unable to create HipBodyInstance!");
		return;
	}

	BoneIndex = PawnOwner.Mesh.MatchRefBone(InBoneName);
	if( BoneIndex != INDEX_NONE )
	{
		BoneMatrix = PawnOwner.Mesh.GetBoneMatrix(BoneIndex);
		BoneBody = PawnOwner.Mesh.FindBodyInstanceNamed(InBoneName);
		
		if( BoneBody != None )
		{
			BoneBody.EnableBoneSpring(bEnable, bEnable, BoneMatrix);
			if( bEnable )
			{
				BoneBody.OverextensionThreshold	= 50.f;
				BoneBody.SetBoneSpringParams(10000.f, 500.f, 10000.f, 500.f);
				BoneBody.bDisableOnOverExtension = TRUE;
			}
		}
		else
		{
			`Warn(PawnOwner.WorldInfo.TimeSeconds @ PawnOwner @ class @ GetFuncName() @ "BodyInstance not found for BoneName:" @ InBoneName);
		}
	}
	else
	{
		`Warn(PawnOwner.WorldInfo.TimeSeconds @ PawnOwner @ class @ GetFuncName() @ "Has non existing Index for BoneName:" @ InBoneName @ "Unable to create BodyInstance!");
	}
}

defaultproperties
{
	bShouldAbortWeaponReload=TRUE
	bCanFireWeapon=FALSE
	bDisableMovement=TRUE
	bLockPawnRotation=TRUE
}
