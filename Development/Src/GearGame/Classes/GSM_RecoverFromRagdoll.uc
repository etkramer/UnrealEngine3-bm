
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_RecoverFromRagdoll extends GearSpecialMove
	native(SpecialMoves);

var()	GearPawn.BodyStance	BS_GetUpFront, BS_GetUpBack;

var bool bBlendToGetUp;
var bool bGetUpFromBack;

var		float	GetUpBlendStartTime;
var()	float	GetUpBlendTime;
var()	float	GetUpAnimRate;
var()	float	GetUpAnimStartPos;

var()	Name	UpDownBoneName;
var()	EAxis	UpDownAxis;
var()	bool	bInvertUpDownBoneAxis;
var()	Name	OrientationBoneName;
var()	EAxis	OrientationAxis;

var()	INT		GetUpFromBackYawOffset;

/** If TRUE, ignore pawns when looking for a spot to get up */
var()	bool	bIgnorePawnsOnRecover;

var		GearPawn.BodyStance BSToPlay;

// C++ functions
cpptext
{
	virtual void TickSpecialMove(FLOAT DeltaTime);
}

/** Trace, but ignore volumes and triggers */
simulated function Actor TraceNoPawns(Pawn TraceOwner, vector End, vector Start, out vector HitLocation)
{
	local Actor HitActor;
	local vector HitLoc, HitNorm;

	// Iterate over each actor along trace...
	foreach TraceOwner.TraceActors(class'Actor', HitActor, HitLoc, HitNorm, End, Start, vect(0,0,0))
	{
		// .. if it's not a pawn, use it!
		if(Pawn(HitActor) != None)
		{
			HitLocation = HitLoc;
			return HitActor;
		}
	}
	return None;
}


function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local vector	HitLocation, HitNormal, TraceStart, TraceEnd, Extent;
	local vector	HeightVec;
	local Actor		HitActor;

	//DebugDrawBones();

	Super.SpecialMoveStarted(bForced,PrevMove);

	// Switch to new AnimTree if we have to
	if( PawnOwner.RecoverFromRagdollAnimTree != None )
	{
		TransitionToAnimTree(PawnOwner.RecoverFromRagdollAnimTree);
	}
	// restore default animations. Bloodmount rider switches it to fall from beast. He's on foot now.
	if( PawnOwner.SpecialMoveClasses.Length > SM_DeathAnim )
	{
		PawnOwner.SpecialMoveClasses[SM_DeathAnim] = PawnOwner.default.SpecialMoveClasses[SM_DeathAnim];
	} 

	if( PawnOwner.SpecialMoveClasses.Length > SM_DeathAnimFire )
	{
		PawnOwner.SpecialMoveClasses[SM_DeathAnimFire] = PawnOwner.default.SpecialMoveClasses[SM_DeathAnimFire];
	}

	if( PawnOwner.SpecialMoveClasses.Length > SM_StumbleFromMelee )
	{
		PawnOwner.SpecialMoveClasses[SM_StumbleFromMelee] = PawnOwner.default.SpecialMoveClasses[SM_StumbleFromMelee];
	}

	if( PawnOwner.SpecialMoveClasses.Length > SM_StumbleFromMelee2 )
	{
		PawnOwner.SpecialMoveClasses[SM_StumbleFromMelee2] = PawnOwner.default.SpecialMoveClasses[SM_StumbleFromMelee2];
	}

	PawnOwner.bDisableMeshTranslationChanges = PawnOwner.default.bDisableMeshTranslationChanges;

	// This is a fix for bloodmount attached characters, they have their default mesh translation reset to zero.
	// We don't want this when they get up from rag doll.
	PawnOwner.CheckDefaultMeshTranslation();
	PawnOwner.SetMeshTranslationOffset( vect(0,0,0) );
	PawnOwner.bCanDoStepsSmoothing = FALSE;

	// Stop updating the physics bones to match the animation
	PawnOwner.Mesh.bUpdateKinematicBonesFromAnimation = FALSE;
	PawnOwner.Mesh.PhysicsWeight = 1.f;
	// Ensure we are always updating kinematic
	PawnOwner.Mesh.MinDistFactorForKinematicUpdate = PawnOwner.default.Mesh.MinDistFactorForKinematicUpdate;


	PawnOwner.Mesh.SetTickGroup(TG_PreAsyncWork);
	PawnOwner.SetTickGroup(TG_PreAsyncWork);

	// This will fix all bones, and stop Actor location being changed.
	PawnOwner.SetPhysics(PHYS_None);

	// Figure out Actor Rotation and Recovery animation to play
	SetupRotationAndAnimation();

	// Move Actor origin so its feet will be on the ground.
	
	// Move Pawn off walls
	Extent = vect(0,0,0);
	// Trace down to floor level
	HeightVec	= vect(0,0,1) * PawnOwner.CylinderComponent.CollisionHeight;
	TraceStart	= PawnOwner.Location + HeightVec * 1.1f;	// Don't go to high, so we don't start trace inside a ceiling.
	TraceEnd	= PawnOwner.Location - HeightVec * 3.f;

	if(bIgnorePawnsOnRecover)
	{
		HitActor = TraceNoPawns(PawnOwner, TraceEnd, TraceStart, HitLocation);
	}
	else
	{
		HitActor = PawnOwner.Trace(HitLocation, HitNormal, TraceEnd, TraceStart, TRUE, Extent);
	}

	if( HitActor != None )
	{
		// Move off geometry by collision extent.
		HitLocation += HeightVec - vect(0,0,1) * Extent.Z;
		Extent = Vect(1,1,0) * PawnOwner.CylinderComponent.CollisionRadius + HeightVec;
		if( PawnOwner.FindSpot(Extent, HitLocation) )
		{		
			if( PawnOwner.Location != HitLocation )
			{
				PawnOwner.SetLocation(HitLocation);
			}
		}
		else
		{
			PawnOwner.SetLocation(HitLocation);
// `if(`notdefined(FINAL_RELEASE))
// 			`Log(class @ "Failed findspot");
// 			PawnOwner.DrawDebugCylinder(PawnOwner.Location - HeightVec, PawnOwner.Location + HeightVec, PawnOwner.CylinderComponent.CollisionRadius, 12, 255, 0, 0, TRUE);
// `endif
		}
	}
	else
	{
// `if(`notdefined(FINAL_RELEASE))
// 		`Log(class @ "Failed trace");
// 		PawnOwner.DrawDebugCylinder(TraceStart + vect(0,0,1) * Extent.Z, TraceEnd - vect(0,0,1) * Extent.Z, Extent.X, 4, 0, 255, 0, TRUE);
// 		PawnOwner.DrawDebugLine(TraceStart, TraceEnd, 0, 0, 255, TRUE);
// `endif
	}

	// Start changing PhysicsWeight from 1.f (showing ragdoll position) to 0.f (showing first frame of get up anim)
	bBlendToGetUp = TRUE;
	GetUpBlendStartTime = PawnOwner.WorldInfo.TimeSeconds;
}

function TransitionToAnimTree(AnimTree NewAnimTree)
{
	// Make sure we're setting a different template, 
	// as this reinitializes the tree and looses the state the animations are in.
	if( PawnOwner.Mesh.AnimTreeTemplate != NewAnimTree )
	{
		// Set driver AnimTree
		PawnOwner.Mesh.SetAnimTreeTemplate(NewAnimTree);
		// Update AnimNodes
		PawnOwner.ClearAnimNodes();
		PawnOwner.CacheAnimNodes();
	}
}

protected function bool InternalCanDoSpecialMove()
{
	return (PawnOwner.Physics == PHYS_RigidBody);
}

/** 
 * Figure out what the Actor's Rotation should be.
 * And which recovery animation should be played.
 */
function SetupRotationAndAnimation()
{
	local Rotator NewRotation;
	local int OrientationBodyIndex, UpDownBodyIndex;
	local matrix BodyTM;

	// restart any animations on the pawn
	PawnOwner.ResumeAllAnimations();

	bGetUpFromBack = FALSE;
	BSToPlay = BS_GetUpFront;

	// Make sure we have a bone set to do these things.
	OrientationBodyIndex = PawnOwner.Mesh.PhysicsAsset.FindBodyIndex(OrientationBoneName);
	if( OrientationBodyIndex != INDEX_NONE )
	{
		// force rotation to match the body's direction so the blend to the getup animation looks more natural
		BodyTM = PawnOwner.Mesh.PhysicsAssetInstance.Bodies[OrientationBodyIndex].GetUnrealWorldTM();
		NewRotation.Yaw = rotator(MatrixGetAxis(BodyTM, OrientationAxis)).Yaw;
		//PawnOwner.DrawDebugCoordinateSystem(MatrixGetOrigin(BodyTM), MatrixGetRotator(BodyTM), 5.f, TRUE);

		UpDownBodyIndex = PawnOwner.Mesh.PhysicsAsset.FindBodyIndex(UpDownBoneName);

		if( UpDownBodyIndex != INDEX_NONE )
		{
			BodyTM = PawnOwner.Mesh.PhysicsAssetInstance.Bodies[UpDownBodyIndex].GetUnrealWorldTM();
			bGetUpFromBack = (MatrixGetAxis(BodyTM, UpDownAxis).Z > 0.f);
			//PawnOwner.DrawDebugCoordinateSystem(MatrixGetOrigin(BodyTM), MatrixGetRotator(BodyTM), 5.f, TRUE);
			if( bInvertUpDownBoneAxis )
			{
				bGetUpFromBack = !bGetUpFromBack;
			}
		}

		// flip it around if the head is facing upwards, since the animation for that makes the character
		// end up facing in the opposite direction that its body is pointing on the ground
		if( bGetUpFromBack )
		{
			NewRotation.Yaw += GetUpFromBackYawOffset;
			NewRotation = Normalize(NewRotation);
		}
		PawnOwner.SetRotation(NewRotation);
 		//PawnOwner.DrawDebugCoordinateSystem(PawnOwner.Location, PawnOwner.Rotation, 50.f, TRUE);

		// Pick get-up anim
		BSToPlay = GetGetUpAnim();
	}
	else
	{
		NewRotation.Yaw = PawnOwner.Rotation.Yaw;
		PawnOwner.SetRotation(NewRotation);
	}

	// Choose correct get-up animation and rewind get up animation to start
	// The play rate is zero - we just want it frozen on the first frame.
	PawnOwner.BS_Play(BSToPlay, GetUpAnimRate, 0.f, 0.33f, FALSE, TRUE);
	PawnOwner.BS_SetPosition(BSToPlay, GetUpAnimStartPos);
	PawnOwner.BS_SetRootBoneAxisOptions(BSToPlay, RBA_Translate, RBA_Translate, RBA_Translate);
	PawnOwner.BS_SetAnimEndNotify(BSToPlay, TRUE);
}

function DebugDrawBones()
{
	local int					i;
	local Name					BoneName;
	local SkeletalMeshComponent	SkelComp;
	local Array<Name>			BoneList;

	SkelComp = PawnOwner.Mesh;
	SkelComp.GetBoneNames(BoneList);
	for(i=0; i<BoneList.Length; i++)
	{
		BoneName = BoneList[i];
		if( SkelComp.MatchRefBone(BoneName) != INDEX_NONE )
		{	
			PawnOwner.DrawDebugCoordinateSystem(SkelComp.GetBoneLocation(BoneName), QuatToRotator(SkelComp.GetBoneQuaternion(BoneName)), 5.f, TRUE);
		}
	}
}

simulated function protected GearPawn.BodyStance GetGetUpAnim()
{
	if( bGetUpFromBack )
	{
		return BS_GetUpBack;
	}
	else
	{
		return BS_GetUpFront;
	}	
}

/** 
 *	Done with blend, start the get-up animation, start updating physics bones again, and show result.
 *	Bones should all be fixed at this point.
 */
simulated event FinishedBlendToGetUp()
{
	bBlendToGetUp = FALSE;

	PawnOwner.Mesh.RootMotionMode = RMM_Accel;
	PawnOwner.SetPhysics(PHYS_Falling);
	PawnOwner.CollisionComponent = PawnOwner.CylinderComponent;
	//PawnOwner.PreRagdollCollisionComponent = None;

	// Retore params to defaults.
	if( !PawnOwner.default.Mesh.bHasPhysicsAssetInstance )
	{
		PawnOwner.Mesh.SetHasPhysicsAssetInstance(FALSE);
	}
	PawnOwner.Mesh.bUpdateKinematicBonesFromAnimation = PawnOwner.default.Mesh.bUpdateKinematicBonesFromAnimation;
	PawnOwner.Mesh.bUpdateJointsFromAnimation = PawnOwner.default.Mesh.bUpdateJointsFromAnimation;
	PawnOwner.Mesh.PhysicsWeight = PawnOwner.default.Mesh.PhysicsWeight;

	// Turn on physics collision again - you are a pawn again, and can collide with other live pawns (for flappy bits), and corpses, but not knocked down alive people
	PawnOwner.Mesh.SetRBChannel(RBCC_Pawn);
	PawnOwner.Mesh.SetRBCollidesWithChannel(RBCC_Untitled3, FALSE);
	PawnOwner.Mesh.SetRBCollidesWithChannel(RBCC_Pawn, TRUE);

	// Check flag before letting you kick corpses again
	if(!PawnOwner.bDisableKicksCorpseAfterRecovery)
	{
		PawnOwner.Mesh.SetRBCollidesWithChannel(RBCC_DeadPawn, TRUE);
	}

	// restore danglers
	if( PawnOwner.Mesh != None && PawnOwner.Mesh.PhysicsAssetInstance != None )
	{
		PawnOwner.Mesh.PhysicsAssetInstance.SetFullAnimWeightBonesFixed(FALSE, PawnOwner.Mesh);
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	local GearInventoryManager GIM;

	bBlendToGetUp = FALSE;

	// Restore things we turned off when we went into rag doll
	PawnOwner.PostRagDollRecovery();
	PawnOwner.Mesh.RootMotionMode = PawnOwner.default.Mesh.RootMotionMode;
	PawnOwner.BS_SetAnimEndNotify(BSToPlay, FALSE);

	PawnOwner.bCanDoStepsSmoothing = PawnOwner.default.bCanDoStepsSmoothing;
	PawnOwner.bInvalidMeleeTarget  = PawnOwner.default.bInvalidMeleeTarget;

	Super.SpecialMoveEnded(PrevMove, NextMove);

	// make sure we have a weapon.	this handles the case when we dropped our heavy weapon because we ragdolled
	if (PawnOwner.Weapon == None)
	{
		GIM = GearInventoryManager(PawnOwner.InvManager);
		GIM.SetCurrentWeapon(GIM.PreviousEquippedWeapon);
	}
}


defaultproperties
{
	BS_GetUpFront=(AnimName[BS_FullBody]="AR_Getup_Front")
	BS_GetUpBack=(AnimName[BS_FullBody]="AR_Getup_Back")

	UpDownBoneName="b_MF_Spine_01"
	UpDownAxis=AXIS_Y
	bInvertUpDownBoneAxis=TRUE
	OrientationBoneName="b_MF_Spine_01"
	OrientationAxis=AXIS_X
	GetUpFromBackYawOffset=+32768

	GetUpBlendTime=0.42f
	GetUpAnimRate=0.55f
	GetUpAnimStartPos=0.f

	bDisableMovement=TRUE
	bLockPawnRotation=TRUE
	bCanFireWeapon=FALSE
	bShouldAbortWeaponReload=TRUE
}