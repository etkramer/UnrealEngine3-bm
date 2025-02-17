/*=============================================================================
	UnPhysAnim.cpp: Code for supporting animation/physics blending
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#include "EnginePrivate.h"
#include "EnginePhysicsClasses.h"
#include "EngineAnimClasses.h"

#if WITH_NOVODEX
#include "UnNovodexSupport.h"
#endif // WITH_NOVODEX

/** Used for drawing pre-phys skeleton if bShowPrePhysBones is true */
static const FColor AnimSkelDrawColor(255, 64, 64);


// Temporary workspace for caching world-space matrices.
struct FAssetWorldBoneTM
{
	FMatrix	TM; // Should never contain scaling.
	INT		UpdateNum; // If this equals PhysAssetUpdateNum, then the matrix is up to date.
};

static INT PhysAssetUpdateNum = 0;
static TArray<FAssetWorldBoneTM> WorldBoneTMs;

// Use current pose to calculate world-space position of this bone without physics now.
static void UpdateWorldBoneTM(INT BoneIndex, USkeletalMeshComponent* skelComp, FLOAT Scale)
{
	// If its already up to date - do nothing
	if(	WorldBoneTMs(BoneIndex).UpdateNum == PhysAssetUpdateNum )
		return;

	FMatrix ParentTM, RelTM;
	if(BoneIndex == 0)
	{
		// If this is the root bone, we use the mesh component LocalToWorld as the parent transform.
		ParentTM = skelComp->LocalToWorld;
		ParentTM.RemoveScaling();
	}
	else
	{
		// If not root, use our cached world-space bone transforms.
		INT ParentIndex = skelComp->SkeletalMesh->RefSkeleton(BoneIndex).ParentIndex;
		UpdateWorldBoneTM(ParentIndex, skelComp, Scale);
		ParentTM = WorldBoneTMs(ParentIndex).TM;
	}

	skelComp->LocalAtoms(BoneIndex).ToTransform(RelTM);
	RelTM.ScaleTranslation( FVector(Scale) );

	WorldBoneTMs(BoneIndex).TM = RelTM * ParentTM;
	WorldBoneTMs(BoneIndex).UpdateNum = PhysAssetUpdateNum;
}

/** 
 *	Calculates the local-space transforms needed by the animation system from the world-space transforms we get from the physics, 
 *	and blends them with the current LocalAtoms.
 *	If the parent of a physics bone is not physical, we work out its world space position by walking up the heirarchy until we reach 
 *	another physics bone (or the root) and then construct back down using the current LocalAtoms.
 */
void USkeletalMeshComponent::BlendPhysicsBones( TArray<BYTE>& RequiredBones, FLOAT PhysicsWeight )
{
#if WITH_NOVODEX
	check(PhysicsAssetInstance);

	// Get drawscale from Owner (if there is one)
	FVector TotalScale3D = Scale * Scale3D;
	if (Owner != NULL)
	{
		TotalScale3D *= Owner->DrawScale * Owner->DrawScale3D;
	}

	if( !TotalScale3D.IsUniform() )
	{
		debugf(TEXT("UAnimNodePhysAsset::AddAnimationData : Non-uniform scale factor (%s). %s  SkelMesh: %s"), *TotalScale3D.ToString(), *GetFullName(), *SkeletalMesh->GetFullName());
		return;
	}

	FLOAT TotalScale = TotalScale3D.X;
	FLOAT RecipScale = 1.0f/TotalScale;

	// If there isn't one - we just use the base-class to put in ref pose.
	if(!PhysicsAssetInstance)
	{
		return;
	}

	check( PhysicsAsset );

	FVector PhysDelta(0,0,0);
	if(PhysicsBlendZeroDriftBoneName != NAME_None)
	{
		INT ZeroDriftBIIndex = PhysicsAsset->FindBodyIndex(PhysicsBlendZeroDriftBoneName);
		if(ZeroDriftBIIndex != INDEX_NONE)
		{
			URB_BodyInstance* ZeroDriftBI = PhysicsAssetInstance->Bodies(ZeroDriftBIIndex);
			FMatrix PhysTM = ZeroDriftBI->GetUnrealWorldTM();
			INT ZeroDriftBoneIndex = MatchRefBone(PhysicsBlendZeroDriftBoneName);
			if(ZeroDriftBoneIndex != INDEX_NONE)
			{
				FMatrix GraphicsTM = GetBoneMatrix(ZeroDriftBoneIndex);
				PhysDelta = GraphicsTM.GetOrigin() - PhysTM.GetOrigin();
			}
		}
	}

	// Make sure scratch space is big enough.
	if(WorldBoneTMs.Num() < SpaceBases.Num())
	{
		WorldBoneTMs.AddZeroed( SpaceBases.Num() - WorldBoneTMs.Num() );
	}

	check( WorldBoneTMs.Num() >= SpaceBases.Num() );

	PhysAssetUpdateNum++;

	// For each bone - see if we need to provide some data for it.
	for(INT i=0; i<RequiredBones.Num(); i++)
	{
		BYTE BoneIndex = RequiredBones(i);
		FBoneAtom ThisAtom;

		// See if this is a physics bone..
		INT BodyIndex = PhysicsAsset->FindBodyIndex( SkeletalMesh->RefSkeleton(BoneIndex).Name );

		// If so - get its world space matrix and its parents world space matrix and calc relative atom.
		if(BodyIndex != INDEX_NONE )
		{	
			URB_BodyInstance* bi = PhysicsAssetInstance->Bodies(BodyIndex);
			URB_BodySetup* bs = PhysicsAsset->BodySetup(BodyIndex);
			if(bi->IsValidBodyInstance())
			{
				FMatrix BoneTM = bi->GetUnrealWorldTM();
				BoneTM.SetOrigin( BoneTM.GetOrigin() + PhysDelta );

				// Store this world-space transform in cache.
				WorldBoneTMs(BoneIndex).TM = BoneTM;
				WorldBoneTMs(BoneIndex).UpdateNum = PhysAssetUpdateNum;

				// Find this bones parent matrix.
				FMatrix ParentTM;
				if(BoneIndex == 0)
				{
					// If root bone is physical - use skeletal LocalToWorld as parent.
					ParentTM = LocalToWorld;
					ParentTM.RemoveScaling();
				}
				else
				{
					// If not root, get parent TM from cache (making sure its up-to-date).
					INT ParentIndex = SkeletalMesh->RefSkeleton(BoneIndex).ParentIndex;
					UpdateWorldBoneTM(ParentIndex, this, TotalScale);
					ParentTM = WorldBoneTMs(ParentIndex).TM;
				}

				// Then calc rel TM and convert to atom.
				FMatrix RelTM = BoneTM * ParentTM.InverseSafe();
				FQuat RelRot(RelTM);
				FVector RelPos = RecipScale * RelTM.GetOrigin();
				ThisAtom = FBoneAtom(RelRot, RelPos, LocalAtoms(BoneIndex).Scale);

				// Now blend in this atom. See if we are forcing this bone to always be blended in
				FLOAT UsePhysWeight = PhysicsWeight;
				if(bEnableFullAnimWeightBodies && bs->bAlwaysFullAnimWeight)
				{
					UsePhysWeight = 1.f;
				}

				LocalAtoms(BoneIndex).Blend( LocalAtoms(BoneIndex), ThisAtom, UsePhysWeight );
			}
#if 0 // Since we support terminating subsets of a PhysicsAssetInstance, this isn't a valid warning any more...
			else
			{
				// In the destructible we intentionally remove bone physics.
				// In those cases bInitBodies will be false and we don't want warnings.
				if ( PhysicsAssetInstance->bInitBodies )
				{
					debugf( TEXT("USkeletalMeshComponent::GetPhysicsBoneAtoms : Invalid BodyInstance (%s) from SkelMesh: %s"), *SkeletalMesh->RefSkeleton(BoneIndex).Name.ToString(), *SkeletalMesh->GetFullName() );
				}
			}
#endif
		}
	}
#endif
}

/** Take the results of the physics and blend them with the animation state (based on the PhysicsWeight parameter), and update the SpaceBases array. */
void USkeletalMeshComponent::BlendInPhysics()
{
	SCOPE_CYCLE_COUNTER(STAT_BlendInPhysics);

#if WITH_NOVODEX
	// Can't do anything without a SkeletalMesh
	if( !SkeletalMesh )
	{
		return;
	}

	// We now have all the animations blended together and final relative transforms for each bone.
	// If we don't have or want any physics, we do nothing.
	if( PhysicsAssetInstance && (PhysicsWeight > ZERO_ANIMWEIGHT_THRESH || (bEnableFullAnimWeightBodies && !bNotUpdatingKinematicDueToDistance)) )
	{
		if(bSkipAllUpdateWhenPhysicsAsleep && FramesPhysicsAsleep > 5 && !RigidBodyIsAwake() && Owner && Owner->Physics == PHYS_RigidBody)
		{
			return;
		}

		BlendPhysicsBones( RequiredBones, PhysicsWeight );

		// now compose SpaceBases
		for(INT i=0; i<RequiredBones.Num(); i++)
		{
			const INT BoneIndex = RequiredBones(i);

			if( BoneIndex == 0 )
			{
				LocalAtoms(0).ToTransform(SpaceBases(0));
			}
			else
			{
				FMatrix LocalBoneTM;
				LocalAtoms(BoneIndex).ToTransform(LocalBoneTM);

				const INT ParentIndex	= SkeletalMesh->RefSkeleton(BoneIndex).ParentIndex;
				SpaceBases(BoneIndex)	= LocalBoneTM * SpaceBases(ParentIndex);
			}
		}
	}
#endif
}

/** 
 *	Iterate over each physics body in the physics for this mesh, and for each 'kinematic' (ie fixed) one, update its
 *	transform based on the animated transform.
 */
void USkeletalMeshComponent::UpdateRBBonesFromSpaceBases(const FMatrix& CurrentLocalToWorld, UBOOL bMoveUnfixedBodies, UBOOL bTeleport)
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateRBBones);

	if(CurrentLocalToWorld.ContainsNaN())
	{
		return;
	}

#if !FINAL_RELEASE
	// Check to see if this physics call is illegal during this tick group
	if (GWorld->InTick && GWorld->TickGroup == TG_DuringAsyncWork)
	{
		debugf(NAME_Error,TEXT("Can't call UpdateRBBonesFromSpaceBases() on (%s)->(%s) during async work!"), *Owner->GetName(), *GetName());
	}
#endif

	// If desired, draw the skeleton at the point where we pass it to the physics.
	if(bShowPrePhysBones && SpaceBases.Num() == SkeletalMesh->RefSkeleton.Num())
	{
		for(INT i=1; i<SpaceBases.Num(); i++)
		{
			FVector ThisPos = CurrentLocalToWorld.TransformFVector( SpaceBases(i).GetOrigin() );

			INT ParentIndex = SkeletalMesh->RefSkeleton(i).ParentIndex;
			FVector ParentPos = CurrentLocalToWorld.TransformFVector( SpaceBases(ParentIndex).GetOrigin() );

			GWorld->LineBatcher->DrawLine(ThisPos, ParentPos, AnimSkelDrawColor, SDPG_Foreground);
		}
	}

#if WITH_NOVODEX
	if(PhysicsAsset && PhysicsAssetInstance && SkeletalMesh)
	{
		check( PhysicsAsset->BodySetup.Num() == PhysicsAssetInstance->Bodies.Num() );

		// Iterate over each body
		for(int i = 0; i < PhysicsAssetInstance->Bodies.Num(); i++)
		{
			// If we have a Novodex body, and its not frozen...
			URB_BodyInstance* BodyInst = PhysicsAssetInstance->Bodies(i);
			check(BodyInst);
			NxActor* nActor = BodyInst->GetNxActor();
			if(nActor && nActor->isDynamic() && !nActor->readBodyFlag(NX_BF_FROZEN))
			{
				// Find the graphics bone index that corresponds to this physics body.
				FName BodyName = PhysicsAsset->BodySetup(i)->BoneName;
				INT BoneIndex = SkeletalMesh->MatchRefBone(BodyName);
				
				// If we could not find it - warn.
				if(BoneIndex == INDEX_NONE)
				{
					debugf( TEXT("UpdateRBBones: WARNING: Failed to find bone '%s' need by PhysicsAsset '%s' in SkeletalMesh '%s'."), *BodyName.ToString(), *PhysicsAsset->GetName(), *SkeletalMesh->GetName() );
				}
				else
				{
					// If its kinematic, move it  to correspond with graphics bone
					if(nActor->readBodyFlag(NX_BF_KINEMATIC) || bMoveUnfixedBodies)
					{
						FMatrix uTM = SpaceBases(BoneIndex) * CurrentLocalToWorld;
						uTM.RemoveScaling();

#if !FINAL_RELEASE && !SHIPPING_PC_GAME
						if(SpaceBases(BoneIndex).ContainsNaN())
						{
							FMatrix TM = SpaceBases(BoneIndex);
							debugf(TEXT("(UpdateRBBonesFromSpaceBases) Bad SpaceBases - %s %d %s"), *SkeletalMesh->GetName(), BoneIndex, *BodyName.ToString());
							debugf(TEXT("%f %f %f %f"), TM.M[0][0], TM.M[0][1], TM.M[0][2], TM.M[0][3]);
							debugf(TEXT("%f %f %f %f"), TM.M[1][0], TM.M[1][1], TM.M[1][2], TM.M[1][3]);
							debugf(TEXT("%f %f %f %f"), TM.M[2][0], TM.M[2][1], TM.M[2][2], TM.M[2][3]);
							debugf(TEXT("%f %f %f %f"), TM.M[3][0], TM.M[3][1], TM.M[3][2], TM.M[3][3]);
							check(!SpaceBases(BoneIndex).ContainsNaN());
						}

						if(uTM.ContainsNaN())
						{
							FMatrix TM = uTM;
							debugf(TEXT("(UpdateRBBonesFromSpaceBases) Bad uTM - %s %d %s"), *SkeletalMesh->GetName(), BoneIndex, *BodyName.ToString());
							debugf(TEXT("%f %f %f %f"), TM.M[0][0], TM.M[0][1], TM.M[0][2], TM.M[0][3]);
							debugf(TEXT("%f %f %f %f"), TM.M[1][0], TM.M[1][1], TM.M[1][2], TM.M[1][3]);
							debugf(TEXT("%f %f %f %f"), TM.M[2][0], TM.M[2][1], TM.M[2][2], TM.M[2][3]);
							debugf(TEXT("%f %f %f %f"), TM.M[3][0], TM.M[3][1], TM.M[3][2], TM.M[3][3]);
							check(!uTM.ContainsNaN());
						}
#endif

						NxMat34 nNewPose = U2NTransform(uTM);

						// Don't call moveGlobalPose if we are already in the correct pose, or nNewPose is invalid.
						NxMat34 nCurrentPose = nActor->getGlobalPose();
						if( nNewPose.M.determinant() > (FLOAT)KINDA_SMALL_NUMBER && 
							!MatricesAreEqual(nNewPose, nCurrentPose, (FLOAT)KINDA_SMALL_NUMBER) )
						{
							// If we are updating a dynamic body, or we want to teleport all the bodies, use 'set'
							if(!nActor->readBodyFlag(NX_BF_KINEMATIC) || bTeleport)
							{
								nActor->setGlobalPose( nNewPose );
							}
							// When smoothly updating kinematic bodies, use 'move'
							else
							{
								nActor->moveGlobalPose( nNewPose );
							}
						}
					}

					// If its not kinematic (ie its dynamics), and it has a bone spring on, update the spring target.
					if(!nActor->readBodyFlag(NX_BF_KINEMATIC) && (BodyInst->bEnableBoneSpringAngular || BodyInst->bEnableBoneSpringLinear))
					{
						FMatrix TM = SpaceBases(BoneIndex) * CurrentLocalToWorld;

						// If bMakeSpringToBaseCollisionComponent is TRUE, transform needs to be relative to base component body.
						if(BodyInst->bMakeSpringToBaseCollisionComponent)
						{
							if( GetOwner() &&
								GetOwner()->Base &&
								GetOwner()->Base->CollisionComponent )
							{
								FMatrix BaseL2W = GetOwner()->Base->CollisionComponent->LocalToWorld;
								FMatrix InvBaseL2W = BaseL2W.Inverse();
								TM = TM * InvBaseL2W;
							}
						}

						BodyInst->SetBoneSpringTarget( TM, bTeleport );
					}
				}
			}
		}
	}
#endif // WITH_NOVODEX
}

/** Script version of UpdateRBBonesFromSpaceBases */
void USkeletalMeshComponent::execUpdateRBBonesFromSpaceBases( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(bMoveUnfixedBodies);
	P_GET_UBOOL(bTeleport);
	P_FINISH;

	UpdateRBBonesFromSpaceBases(LocalToWorld, bMoveUnfixedBodies, bTeleport);
}
IMPLEMENT_FUNCTION(USkeletalMeshComponent,INDEX_NONE,execUpdateRBBonesFromSpaceBases);

/** 
 *	Iterate over each joint in the physics for this mesh, setting its AngularPositionTarget based on the animation information.
 */
void USkeletalMeshComponent::UpdateRBJointMotors()
{
#if !FINAL_RELEASE
	// Check to see if this physics call is illegal during this tick group
	if (GWorld->InTick && GWorld->TickGroup == TG_DuringAsyncWork)
	{
		debugf(NAME_Error,TEXT("Can't call UpdateRBJointMotors() on (%s)->(%s) during async work!"), *Owner->GetName(), *GetName());
	}
#endif

#if WITH_NOVODEX
	if(PhysicsAsset && PhysicsAssetInstance && SkeletalMesh)
	{
		check( PhysicsAsset->ConstraintSetup.Num() == PhysicsAssetInstance->Constraints.Num() );


		// Iterate over the constraints.
		for(INT i=0; i<PhysicsAssetInstance->Constraints.Num(); i++)
		{
			URB_ConstraintSetup* CS = PhysicsAsset->ConstraintSetup(i);
			URB_ConstraintInstance* CI = PhysicsAssetInstance->Constraints(i);

			FName JointName = CS->JointName;
			INT BoneIndex = SkeletalMesh->MatchRefBone(JointName);

			// If we found this bone, and its not the root, and its joint is motorised in some way..
			if( BoneIndex != INDEX_NONE && 
				BoneIndex != 0 && 
				(CI->bSwingPositionDrive || CI->bTwistPositionDrive) )
			{
				check(BoneIndex < LocalAtoms.Num());

				// If we find the joint - get the local-space animation between this bone and its parent.
				FQuat LocalQuat = LocalAtoms(BoneIndex).Rotation;
				FQuatRotationTranslationMatrix LocalRot(LocalQuat, FVector(0.f));

				// We loop from the graphics parent bone up to the bone that has the body which the joint is attached to, to calculate the relative transform.
				// We need this to compensate for welding, where graphics and physics parents may not be the same.
				FMatrix ControlBodyToParentBoneTM = FMatrix::Identity;

				INT TestBoneIndex = SkeletalMesh->RefSkeleton(BoneIndex).ParentIndex; // This give the 'graphics' parent of this bone
				UBOOL bFoundControlBody = (SkeletalMesh->RefSkeleton(TestBoneIndex).Name == CS->ConstraintBone2); // ConstraintBone2 is the 'physics' parent of this joint.

				while(!bFoundControlBody)
				{
					// Abort if we find a bone scaled to zero.
					if(LocalAtoms(TestBoneIndex).Scale < KINDA_SMALL_NUMBER)
					{
						break;
					}

					// Add the current animated local transform into the overall controlling body->parent bone TM
					FMatrix RelTM;
					LocalAtoms(TestBoneIndex).ToTransform(RelTM);
					RelTM.SetOrigin(FVector(0.f));
					ControlBodyToParentBoneTM = ControlBodyToParentBoneTM * RelTM;

					// Move on to parent
					TestBoneIndex = SkeletalMesh->RefSkeleton(TestBoneIndex).ParentIndex;

					// If we are at the root - bail out.
					if(TestBoneIndex == 0)
					{
						break;
					}

					// See if this is the controlling body
					bFoundControlBody = (SkeletalMesh->RefSkeleton(TestBoneIndex).Name == CS->ConstraintBone2);
				}

				// If after that we didn't find a parent body, we can' do this, so skip.
				if(bFoundControlBody)
				{
#if WITH_NOVODEX
					// The animation rotation is between the two bodies. We need to supply the joint with the relative orientation between
#endif
					// the constraint ref frames. So we work out each body->joint transform

					FMatrix Body1TM = CS->GetRefFrameMatrix(0);
					Body1TM.SetOrigin(FVector(0.f));
					FMatrix Body1TMInv = Body1TM.Inverse();

					FMatrix Body2TM = CS->GetRefFrameMatrix(1);
					Body2TM.SetOrigin(FVector(0.f));
					FMatrix Body2TMInv = Body2TM.Inverse();

					FMatrix JointRot = Body1TM * (LocalRot * ControlBodyToParentBoneTM) * Body2TMInv;
					FQuat JointQuat(JointRot);

					// Then pass new quaternion to the joint!
					CI->SetAngularPositionTarget(JointQuat);
				}
			}
		}
	}
#endif // WITH_NOVODEX
}

