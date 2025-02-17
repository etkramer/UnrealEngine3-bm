/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
#include "GearGame.h"
#include "GearGameAnimClasses.h"
#include "EngineMaterialClasses.h"

IMPLEMENT_CLASS(UGearAnim_GoreSystem);

/** Used to cache index to gore bones */
void UGearAnim_GoreSystem::InitAnim( USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent )
{
	Super::InitAnim(MeshComp, Parent);

	SortGoreSetup();
	CalcGoreIndexInfo();
}	

void UGearAnim_GoreSystem::CalcGoreIndexInfo()
{
	if(SkelComponent && SkelComponent->SkeletalMesh)
	{
		for(INT i=0; i<GoreSetup.Num(); i++)
		{
			GoreSetup(i).GoreBoneIndex = INDEX_NONE;
			GoreSetup(i).ChildEntryIndex = INDEX_NONE;
		}

		// Cache index of each gore bone
		for(INT i=0; i<GoreSetup.Num(); i++)
		{
			GoreSetup(i).GoreBoneIndex = SkelComponent->MatchRefBone(GoreSetup(i).GoreBoneName);

			// If we couldn't find gore bone - warn
			if(GoreSetup(i).GoreBoneIndex == INDEX_NONE)
			{
				debugf(TEXT("GoreSystem: GoreBoneName '%s' not found in SkelMesh '%s'"), *GoreSetup(i).GoreBoneName.ToString(), *SkelComponent->SkeletalMesh->GetName());
			}
		}

		// Now look to see if a gore bone has a parent which is also a gore bone, and inform it if so
		for(INT i=0; i<GoreSetup.Num(); i++)
		{
			if(GoreSetup(i).GoreBoneIndex != INDEX_NONE)
			{
				// Now we want to tell this gore bone's parent (if its also a gore bone) about this entry
				INT ParentBoneIndex = SkelComponent->SkeletalMesh->RefSkeleton(GoreSetup(i).GoreBoneIndex).ParentIndex;
				check(ParentBoneIndex != INDEX_NONE); // should never happen
				for(INT j=0; j<GoreSetup.Num(); j++)
				{
					if(GoreSetup(j).GoreBoneIndex == ParentBoneIndex)
					{
						GoreSetup(j).ChildEntryIndex = i;
						//debugf(TEXT("Gore bone '%s' added as child of '%s'"), *GoreSetup(i).GoreBoneName.ToString(), *GoreSetup(j).GoreBoneName.ToString());
					}
				}
			}
		}
	}
	else
	{
		// No skel mesh or component - reset all bone indices
		for(INT i=0; i<GoreSetup.Num(); i++)
		{
			GoreSetup(i).GoreBoneIndex = INDEX_NONE;
			GoreSetup(i).ChildEntryIndex = INDEX_NONE;
		}
	}
}


/** Used to update falling gore bones */
void UGearAnim_GoreSystem::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	Super::TickAnim(DeltaSeconds, TotalWeight);

	UBOOL bRBPhysics = FALSE;
	if( SkelComponent && 
		SkelComponent->GetOwner() && 
		SkelComponent->GetOwner()->Physics == PHYS_RigidBody )
	{
		bRBPhysics = TRUE;
	}

	for(INT i=0; i<GoreSetup.Num(); i++)
	{
		FGearGoreEntry& GoreEntry = GoreSetup(i);
		if(GoreEntry.GoreBoneState == EGBS_Falling)
		{
			// gore bone animation doesn't work right with rigid body physics - so kill parts if this is the case
			if(bRBPhysics)
			{
				GoreEntry.GoreBoneState = EGBS_Hidden;
			}

			// Update velocity
			GoreEntry.GoreBoneVelocity += FVector(0, 0, GWorld->GetGravityZ()) * GoreFallGravScale * DeltaSeconds;
			// Update location
			GoreEntry.GoreBoneLocation += (GoreEntry.GoreBoneVelocity * DeltaSeconds);

			// Update rotation
			GoreEntry.GoreBoneRotation += (GoreEntry.GoreBoneAngVelocity * DeltaSeconds);

			// Update lifetime
			GoreEntry.FallingLifetime -= DeltaSeconds;
			// If we hit zero - swith to hidden
			if(GoreEntry.FallingLifetime <= 0.f)
			{
				GoreEntry.GoreBoneState = EGBS_Hidden;
			}
		}
	}
}

/** Util to build mesh-space bone transforms. */
static void BuildMeshSpaceMatrices(TArray<FMatrix>& BoneTM, const USkeletalMesh* SkelMesh, const FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones)
{
	BoneTM.Add(SkelMesh->RefSkeleton.Num());

	for(INT i=0; i<DesiredBones.Num(); i++)
	{	
		const INT BoneIndex = DesiredBones(i);

		if( BoneIndex == 0 )
		{
			Atoms(0).ToTransform(BoneTM(0));
		}
		else
		{
			FMatrix LocalBoneTM;
			Atoms(BoneIndex).ToTransform(LocalBoneTM);

			const INT ParentIndex	= SkelMesh->RefSkeleton(BoneIndex).ParentIndex;
			BoneTM(BoneIndex)		= LocalBoneTM * BoneTM(ParentIndex);
		}
	}
}

/** Scale gore bones that have been destroyed. */
void UGearAnim_GoreSystem::GetBoneAtoms(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion)
{
	START_GETBONEATOM_TIMER

	// First grab animation data from children
	if(Children(0).Anim)
	{
		EXCLUDE_CHILD_TIME
		Children(0).Anim->GetBoneAtoms(Atoms, DesiredBones, RootMotionDelta, bHasRootMotion);
	}
	else
	{
		RootMotionDelta = FBoneAtom::Identity;
		bHasRootMotion	= 0;
		FillWithRefPose(Atoms, DesiredBones, SkelComponent->SkeletalMesh->RefSkeleton);
	}

	// Only do gore stuff if we have valid SkeletalMesh(Component)
	if(SkelComponent && SkelComponent->SkeletalMesh)
	{
		// CAlc world->component
		FMatrix WorldToLocal = SkelComponent->LocalToWorld.Inverse();

		// Component-space bone transforms - needed if we are moving bones in world space
		TArray<FMatrix> BoneTM;

		// Modify any gore bones that need it
		for(INT i=0; i<GoreSetup.Num(); i++)
		{
			FGearGoreEntry& GoreEntry = GoreSetup(i);

			// Check gore bone index is valid
			if(GoreEntry.GoreBoneIndex != INDEX_NONE)
			{
				// If hidden, scale to zero
				if(GoreEntry.GoreBoneState == EGBS_Hidden)
				{
					check(GoreEntry.GoreBoneIndex < Atoms.Num());
					Atoms(GoreEntry.GoreBoneIndex).Scale = 0.f;
				}
				// If falling, update location
				else if(GoreEntry.GoreBoneState == EGBS_Falling)
				{
					// If we have not done so yet, calc component-space bone transforms, so we can go from world to parent-bone space
					if(BoneTM.Num() == 0)
					{
						BuildMeshSpaceMatrices(BoneTM, SkelComponent->SkeletalMesh, Atoms, DesiredBones);
					}

					// First calc mesh-space bone matrix.
					FMatrix BoneWorldTM = FRotationTranslationMatrix(FRotator::MakeFromEuler(GoreEntry.GoreBoneRotation), GoreEntry.GoreBoneLocation);

					// Update mesh space transforms for gore children of this bone
					BoneTM(GoreEntry.GoreBoneIndex) = BoneWorldTM * WorldToLocal;

					// Find parent bone of this gore bone
					INT ParentIndex = SkelComponent->SkeletalMesh->RefSkeleton(GoreEntry.GoreBoneIndex).ParentIndex;
					// Get its world-space TM
					FMatrix ParentWorldTM = BoneTM(ParentIndex) * SkelComponent->LocalToWorld;
					// Find new gore bone transform in that ref frame.
					FMatrix ParentSpaceBoneTM = BoneWorldTM * ParentWorldTM.InverseSafe();
					ParentSpaceBoneTM.RemoveScaling();

					// Set for bone atom.
					Atoms(GoreEntry.GoreBoneIndex).Translation = ParentSpaceBoneTM.GetOrigin();
					Atoms(GoreEntry.GoreBoneIndex).Rotation = FQuat(ParentSpaceBoneTM);
				}
			}
		}
	}
}

/** Damage has been done to a body in the physicsasset - update any gore state associated with it */
void UGearAnim_GoreSystem::UpdateGoreDamage(FName ShotBoneName, FVector HitLocation, INT Damage)
{
	INT ClosestBoundGoreSetupIndex = INDEX_NONE;
	FLOAT ClosestBoundGoreBoneDistSqr = BIG_NUMBER;

	INT ClosestGoreSetupIndex = INDEX_NONE;
	FLOAT ClosestGoreBoneDistSqr = BIG_NUMBER;

	// Iterate over gore entries, to find one for this body - could be multiple, we pick the one with the closest gore bone
	for(INT i=0; i<GoreSetup.Num(); i++)
	{
		FGearGoreEntry& GoreEntry = GoreSetup(i);

//		debugf(TEXT("GoreEntry %s / %s"), *GoreEntry.HitBodyName.ToString(), *GoreEntry.GoreBoneName.ToString());

		// Desired body, and it has a valid gore bone
		if(GoreEntry.GoreBoneIndex != INDEX_NONE)
		{
			// See how far gore bone is from hit location - remember if closest
			FVector GoreBoneLocation = SkelComponent->GetBoneMatrix(GoreEntry.GoreBoneIndex).GetOrigin();
			FLOAT GoreBoneDistSqr = (GoreBoneLocation - HitLocation).SizeSquared();

			// Closest bound to the physics body we hit
			if(GoreEntry.HitBodyName == ShotBoneName)
			{
				if(GoreBoneDistSqr < ClosestBoundGoreBoneDistSqr)
				{
					ClosestBoundGoreSetupIndex = i;
					ClosestBoundGoreBoneDistSqr = GoreBoneDistSqr;
				}
			}
			// Closest overall
			else
			{
				if(GoreBoneDistSqr < ClosestGoreBoneDistSqr)
				{
					ClosestGoreSetupIndex = i;
					ClosestGoreBoneDistSqr = GoreBoneDistSqr;
				}
			}
		}
	}

	// If we found some gore bound to body we shot - use that, otherwise use closest overall
	INT RelevantGoreSetupIndex = (ClosestBoundGoreSetupIndex != INDEX_NONE) ? ClosestBoundGoreSetupIndex : ClosestGoreSetupIndex;

	// See if we found a gore entry
	if(RelevantGoreSetupIndex != INDEX_NONE)
	{
		FGearGoreEntry& GoreEntry = GoreSetup(RelevantGoreSetupIndex);

		// If we have a child which is still attached, change damage to that entry instead
		while(GoreEntry.ChildEntryIndex != INDEX_NONE && GoreSetup(GoreEntry.ChildEntryIndex).GoreBoneState == EGBS_Attached)
		{
			GoreEntry = GoreSetup(GoreEntry.ChildEntryIndex);
		}

//		debugf(TEXT("Relevant Gore Entry %s / %s -- enabled? %d"), *GoreEntry.HitBodyName.ToString(), *GoreEntry.GoreBoneName.ToString(), GoreEntry.bEnabled );

		// Only do damage if not already at zero!
		if(GoreEntry.bEnabled && GoreEntry.Health > 0)
		{
			GoreEntry.Health -= Damage;
			if(GoreEntry.Health <= 0)
			{
				// Get matrix of this bone in world space now
				const FMatrix GoreBoneMatrix = SkelComponent->GetBoneMatrix(GoreEntry.GoreBoneIndex);

				LoseGorePiece(GoreEntry, GoreBoneMatrix);
			}
		}
	}
}

/** Drop given gore piece */
void UGearAnim_GoreSystem::ForceLoseGorePiece( FName GoreBoneName )
{
	for( INT i = 0; i < GoreSetup.Num(); i++ )
	{
		FGearGoreEntry& GoreEntry = GoreSetup(i);

		if( GoreEntry.GoreBoneName == GoreBoneName )
		{
			// If we have a child which is still attached, force that piece off also
			while(GoreEntry.ChildEntryIndex != INDEX_NONE && GoreSetup(GoreEntry.ChildEntryIndex).GoreBoneState == EGBS_Attached)
			{
				ForceLoseGorePiece( GoreSetup(GoreEntry.ChildEntryIndex).GoreBoneName );
			}

			//debugf(TEXT("ForceLoseGorePiece %s %s"), *GoreBoneName.ToString(), *GoreEntry.GoreBoneName.ToString());

			GoreEntry.bEnabled = TRUE;

			// Get matrix of this bone in world space now
			const FMatrix GoreBoneMatrix = SkelComponent->GetBoneMatrix(GoreEntry.GoreBoneIndex);

			LoseGorePiece( GoreEntry, GoreBoneMatrix );
		}
	}
}

/** Start the supplied gore piece falling */
void UGearAnim_GoreSystem::LoseGorePiece(FGearGoreEntry& GoreEntry, const FMatrix& GoreBoneMatrix)
{
	// In PHYS_RigidBody, just hide parts
	if( SkelComponent && 
		SkelComponent->GetOwner() && 
		SkelComponent->GetOwner()->Physics == PHYS_RigidBody )
	{
		GoreEntry.GoreBoneState = EGBS_Hidden;
	}
	else
	{
		GoreEntry.GoreBoneState = EGBS_Falling;
	}

	// Get matrix of parent of this bone in world space
	INT GoreBoneParentIndex = SkelComponent->SkeletalMesh->RefSkeleton(GoreEntry.GoreBoneIndex).ParentIndex;
	FMatrix GoreBoneParentMatrix = SkelComponent->GetBoneMatrix(GoreBoneParentIndex);

	// Save initial location/rotation
	GoreEntry.GoreBoneLocation = GoreBoneMatrix.GetOrigin();
	GoreEntry.GoreBoneRotation = GoreBoneMatrix.Rotator().Euler();

	// Set velocity to take bone away from parent bone
	GoreEntry.GoreBoneVelocity = (GoreEntry.GoreBoneLocation - GoreBoneParentMatrix.GetOrigin()).SafeNormal() * GoreInitialVel;
	// Random angular velocity
	GoreEntry.GoreBoneAngVelocity = VRand() * GoreInitialAngVel;

	// If desired, add additional velocity (specified in actor space)
	if(!ActorSpaceAdditionalVel.IsZero())
	{
		FVector WorldSpaceAddVel = -ActorSpaceAdditionalVel;
#if 0
		AActor* Owner = SkelComponent->GetOwner();
		if(Owner)
		{
			const FMatrix ActorToWorld = FRotationTranslationMatrix(Owner->Rotation, Owner->Location);
			WorldSpaceAddVel = ActorToWorld.TransformNormal(ActorSpaceAdditionalVel);
		}
#endif

		GoreEntry.GoreBoneVelocity += WorldSpaceAddVel;
	}

	GoreEntry.FallingLifetime = FallingLifetime;

	// Turn on rigid weighting for verts weighted to parent of gore bone
	SkelComponent->AddInstanceVertexWeightBoneParented(SkelComponent->GetParentBone(GoreEntry.GoreBoneName), FALSE);
	SkelComponent->AddInstanceVertexWeightBoneParented(GoreEntry.GoreBoneName, FALSE);
	//SkelComponent->AddInstanceVertexWeightBoneParented(GoreEntry.GoreBoneName);

	// If we have an owner, and want some effects, call util function in AGearGame
	if(	SkelComponent->GetOwner() && 
		GoreEntry.EffectInfoIndex != INDEX_NONE &&
		GoreEntry.EffectInfoIndex < GoreEffectInfos.Num() )
	{
		// We need the parent bone name
		FName ParentName = SkelComponent->SkeletalMesh->RefSkeleton(GoreBoneParentIndex).Name;

		// Get the game info
		AGameInfo* GameInfo = GWorld->GetGameInfo();
		AGearGame* GearGame = Cast<AGearGame>(GameInfo);

		if(GearGame)
		{
			// Bone location plus offset in parent-bone space
			FVector EffectLocation = GoreEntry.GoreBoneLocation + GoreBoneParentMatrix.TransformNormal(GoreEntry.EffectOffset);

			// This will spawn effects and play sound
			UBOOL bSpawnedGib = (GoreEntry.GoreBoneState != EGBS_Hidden);
			GearGame->eventGoreSystemSpawnGibEffects(SkelComponent, GoreEntry.GoreBoneName, EffectLocation, GoreEntry.GoreBoneVelocity, ParentName, GoreEffectInfos(GoreEntry.EffectInfoIndex), bSpawnedGib);
		}
	}

	// Modify material parameter
	if(GoreEntry.MaterialParamIndex < GoreMaterialParamInfos.Num())
	{
		FGearGoreMaterialParam& ParamInfo = GoreMaterialParamInfos(GoreEntry.MaterialParamIndex);

		// Iterate over materials looking for MIs which are not in content package
		INT NumMaterials = SkelComponent->SkeletalMesh->Materials.Num();
		for(INT i=0; i<NumMaterials; i++)
		{
			UMaterialInstance* MI = Cast<UMaterialInstance>(SkelComponent->GetMaterial(i));
			if(MI)
			{
				if(MI->IsInMapOrTransientPackage())
				{
					FLOAT CurrentVal = 0.f;
					UBOOL bFoundParam = MI->GetScalarParameterValue(ParamInfo.ScalarParamName, CurrentVal);
					if(bFoundParam)
					{
						// Increase up to a max of 1.0
						FLOAT NewVal = Min(CurrentVal + ParamInfo.IncreasePerGore, 1.f);
						MI->SetScalarParameterValue(ParamInfo.ScalarParamName, NewVal);
					}
				}
				else
				{
					debugf( TEXT("LoseGorePiece : MI is not in map or transient package: %s (Owner: %s)"), *MI->GetName(), (SkelComponent->GetOwner() ? *SkelComponent->GetOwner()->GetName() : TEXT("NONE")) );
				}
			}
			else
			{
				debugf( TEXT("LoseGorePiece : No MI to modify (Owner: %s)"), (SkelComponent->GetOwner() ? *SkelComponent->GetOwner()->GetName() : TEXT("NONE")) );
			}
		}
	}
}

/** Damage done radially from a location - find  all gore bones affected and damage them */
void UGearAnim_GoreSystem::UpdateGoreDamageRadial(FVector HitLocation, INT Damage, UBOOL bForceRemoveAll)
{
	// Iterate over gore entries, to find one for this body - could be multiple, we pick the one with the closest gore bone
	for(INT i=0; i<GoreSetup.Num(); i++)
	{
		FGearGoreEntry& GoreEntry = GoreSetup(i);

		// Check if it has a valid gore bone
		if(GoreEntry.GoreBoneIndex != INDEX_NONE)
		{
			// See how far gore bone is from hit location - damage if within Radius
			const FMatrix GoreBoneMatrix = SkelComponent->GetBoneMatrix(GoreEntry.GoreBoneIndex);

			const FLOAT GoreBoneDistSqr = (GoreBoneMatrix.GetOrigin() - HitLocation).SizeSquared();
			if(GoreBoneDistSqr < (ExplosiveGoreRadius*ExplosiveGoreRadius) || bForceRemoveAll)
			{
				// Don't apply damage to bones that have attached children
				UBOOL bHasAttachedChild = (	GoreEntry.ChildEntryIndex != INDEX_NONE && 
											GoreEntry.ChildEntryIndex < GoreSetup.Num() &&
											GoreSetup(GoreEntry.ChildEntryIndex).GoreBoneState == EGBS_Attached );

				// Only do damage if not already at zero!
				if( GoreEntry.bEnabled && ((GoreEntry.Health > 0 && !bHasAttachedChild) || bForceRemoveAll))
				{
					GoreEntry.Health -= Damage;
					if(GoreEntry.Health <= 0 || bForceRemoveAll)
					{
						LoseGorePiece(GoreEntry, GoreBoneMatrix);
					}
				}
			}
		}
	}
}

/** Util to reset all gore (reattach/unhide all pieces) */
void UGearAnim_GoreSystem::ResetAllGore()
{
	// Iterate over gore entries, attaching pieces again
	for(INT i=0; i<GoreSetup.Num(); i++)
	{
		FGearGoreEntry& GoreEntry = GoreSetup(i);
		GoreEntry.GoreBoneState = EGBS_Attached;
	}

	// Turn on soft weighting again
	SkelComponent->ToggleInstanceVertexWeights(FALSE);
}

void UGearAnim_GoreSystem::PreEditChange(UProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);

	for(INT i=0; i<GoreSetup.Num(); i++)
	{
		GoreSetup(i).GoreBoneIndex = INDEX_NONE;
		GoreSetup(i).ChildEntryIndex = INDEX_NONE;
	}
}

/** Use to handle bTestAllGore flag in ATE */
void UGearAnim_GoreSystem::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);

	if( PropertyThatChanged->GetFName() == FName(TEXT("bAutoFillGoreSetup")) )
	{
		if(SkelComponent)
		{
			AutoFillGoreSetup(SkelComponent->SkeletalMesh);
		}

		bAutoFillGoreSetup = FALSE;
	}

	// Recalc bone/effect infos, in case arrays have changed size
	SortGoreSetup();
	CalcGoreIndexInfo();

	// Turn on all gore!
	if(bTestAllGore && !bOldTestAllGore)
	{
		UpdateGoreDamageRadial(FVector(0,0,0), 0, TRUE);
	}
	// Turn off all gore
	else if(!bTestAllGore && bOldTestAllGore)
	{
		ResetAllGore();
	}

	bOldTestAllGore = bTestAllGore;
}

void UGearAnim_GoreSystem::AutoFillGoreSetup(USkeletalMesh* SkelMesh)
{
	// Check we want to discard current setup
	if(GoreSetup.Num() != 0)
	{
		if( appMsgf(AMT_YesNo, TEXT("This will replace current GoreSetup, is that ok?")) == FALSE )
		{
			return;
		}
	}
	GoreSetup.Empty();

	// Look for bones containing word 'gore'
	for(INT i=0; i<SkelMesh->RefSkeleton.Num(); i++)
	{
		FMeshBone& Bone = SkelMesh->RefSkeleton(i);
		if( appStristr(*Bone.Name.ToString(), TEXT("gore")) )
		{
			// Create new entry
			INT NewIndex = GoreSetup.AddZeroed();
			GoreSetup(NewIndex).GoreBoneName = Bone.Name;
			GoreSetup(NewIndex).GoreBoneIndex = INDEX_NONE;
			GoreSetup(NewIndex).Health = 100;
			GoreSetup(NewIndex).bEnabled = TRUE;
		}
	}

	// Cache bone indices
	InitAnim(SkelComponent, NULL);

	// output info
	appMsgf(AMT_OK, TEXT("Found: %d gore bones!"), GoreSetup.Num());
}

/** Hack pointer to get mesh to sort function  */
static USkeletalMesh* SortUsingMesh;

IMPLEMENT_COMPARE_CONSTREF(FGearGoreEntry, GearAnimGore, { return(( SortUsingMesh->MatchRefBone(A.GoreBoneName) -  SortUsingMesh->MatchRefBone(B.GoreBoneName) )); } );

/** Sorts the setup array so that parent bones are before child bones */
void UGearAnim_GoreSystem::SortGoreSetup()
{
	if(SkelComponent && SkelComponent->SkeletalMesh)
	{
		SortUsingMesh = SkelComponent->SkeletalMesh; // Hack to pass skel mesh to sort function
		Sort<USE_COMPARE_CONSTREF(FGearGoreEntry, GearAnimGore)>( &GoreSetup(0), GoreSetup.Num() );
		SortUsingMesh = NULL;
	}
}

