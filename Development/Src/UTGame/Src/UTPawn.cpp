//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
// Confidential.
//=============================================================================

#include "UTGame.h"
#include "EngineAnimClasses.h"
#include "GameFrameworkAnimClasses.h"
#include "UTGameAIClasses.h"
#include "EngineMaterialClasses.h"
#include "UnPath.h"

IMPLEMENT_CLASS(AUTPawn);

/** 
 * CalcVelocity()
 * Calculates new velocity and acceleration for pawn for this tick
 * bounds acceleration and velocity, adds effects of friction and momentum
 * @param	AccelDir	Acceleration direction. (Normalized vector).
 * @param	DeltaTime	time elapsed since last frame.
 * @param	MaxSpeed	Maximum speed Pawn can go at. (f.e. Pawn.GroundSpeed for walking physics).
 * @param	Friction	friction
 * @param	bFluid		bFluid
 * @param	bBrake		if should brake to a stop when acceleration is zero.
 * @param	bBuoyant	Apply buoyancy for swimming physics
 */
void AUTPawn::CalcVelocity(FVector &AccelDir, FLOAT DeltaTime, FLOAT MaxSpeed, FLOAT Friction, INT bFluid, INT bBrake, INT bBuoyant)
{
	if ( !bForceRegularVelocity && (bForceRMVelocity || (Mesh && Mesh->RootMotionMode != RMM_Ignore)) || (Controller && Controller->bPreciseDestination) )
	{
		// let super version handle any root motion driven velocity calculation
		Super::CalcVelocity(AccelDir,DeltaTime,MaxSpeed,Friction,bFluid,bBrake,bBuoyant);
		return;
	}

	// always force UTPawn acceleration to maximum AccelRate
	Acceleration = AccelDir * AccelRate;

	if( bBrake && Acceleration.IsZero() )
	{
		const	FVector OldVel = Velocity;
				FVector SumVel = FVector(0);

		// subdivide braking to get reasonably consistent results at lower frame rates
		// (important for packet loss situations w/ networking)
		FLOAT RemainingTime = DeltaTime;
		const FLOAT TimeStep = 0.03f;

		while( RemainingTime > 0.f )
		{
			const FLOAT dt = ((RemainingTime > TimeStep) ? TimeStep : RemainingTime);
			RemainingTime -= dt;

			// don't drift to a stop, brake
			Velocity = Velocity - (2.f * Velocity) * dt * Friction; 
			if( (Velocity | OldVel) > 0.f )
			{
				SumVel += dt * Velocity/DeltaTime;
			}
		}

		Velocity = SumVel;
		
		// brake to a stop, not backwards
		if( ((OldVel | Velocity) < 0.f)	|| (Velocity.SizeSquared() < SLOWVELOCITYSQUARED) )
		{
			Velocity = FVector(0.f);
		}
	}
	else
	{
		Velocity = Velocity - (Velocity - AccelDir * Velocity.Size()) * DeltaTime * Friction;
	}

	Velocity = Velocity * (1 - bFluid * Friction * DeltaTime) + Acceleration * DeltaTime;

	if( bBuoyant )
	{
		Velocity.Z += GetGravityZ() * DeltaTime * (1.f - Buoyancy);
	}

	// Scale max speed by modifier
	MaxSpeed *= MaxSpeedModifier();

	if( Velocity.SizeSquared() > MaxSpeed * MaxSpeed )
	{
		Velocity = Velocity.SafeNormal() * MaxSpeed;
	}
}

/**
@RETURN true if pawn is invisible to AI
*/
UBOOL AUTPawn::IsInvisible()
{
	return bIsInvisible;
}

/* Dampen loudness of a noise instigated by this pawn
@PARAM NoiseMaker	is the actual actor who made the noise
@PARAM Loudness		is the loudness passed in (1.0 is typical value)
@PARAM NoiseType	is additional optional information for game specific implementations
@RETURN		dampening factor
*/
FLOAT AUTPawn::DampenNoise( AActor* NoiseMaker, FLOAT Loudness, FName NoiseType )
{
	FLOAT DampeningFactor = 1.f;
	if ( (NoiseMaker == this) ||(NoiseMaker->Owner == this) )
		DampeningFactor *= Instigator->SoundDampening;

	// don't hear sounds from invisible players as well
	if ( IsInvisible() )
		DampeningFactor *= 0.3f;

	return DampeningFactor;
}

UBOOL AUTPawn::HasAudibleAmbientSound(const FVector& SrcLocation)
{
	INT bIsOccluded;
	return ( (PawnAmbientSoundCue != NULL && PawnAmbientSoundCue->IsAudible(Location, SrcLocation, this, bIsOccluded, FALSE)) ||
			(WeaponAmbientSoundCue != NULL && WeaponAmbientSoundCue->IsAudible(Location, SrcLocation, this, bIsOccluded, FALSE)) );
}

/** Utility for making sure rotation component is between +/- 32768 */
static INT UnwindRot(INT InRot)
{
	INT OutRot = InRot;

	while(OutRot > 32768)
	{
		OutRot -= 65536;
	}

	while(OutRot < -32768)
	{
		OutRot += 65536;
	}

	return OutRot;
}

void AUTPawn::TickSpecial( FLOAT DeltaSeconds )
{
	Super::TickSpecial(DeltaSeconds);

	if (WorldInfo->NetMode != NM_DedicatedServer && SaturationParameterName != NAME_None)
	{
		FLOAT SaturationMult = 1.0f;
		for (INT i = 0; i < GEngine->GamePlayers.Num(); i++)
		{
			if (GEngine->GamePlayers(i)->Actor != NULL && GEngine->GamePlayers(i)->Actor->LODDistanceFactor < 1.0f)
			{
				SaturationMult = Max<FLOAT>(SaturationMult, 1.0f / GEngine->GamePlayers(i)->Actor->LODDistanceFactor);
			}
		}

		for (INT i = 0; i < BodyMaterialInstances.Num(); i++)
		{
			if (BodyMaterialInstances(i) != NULL)
			{
				BodyMaterialInstances(i)->SetScalarParameterValue(SaturationParameterName, SaturationMult);
			}
		}
	}

	// Get current aiming pitch and yaw
	INT PawnAimPitch;
	if(Controller)
	{
		PawnAimPitch = Controller->Rotation.Pitch;
	}
	else
	{
		PawnAimPitch = Rotation.Pitch;
		if( PawnAimPitch == 0 )
		{
			PawnAimPitch = RemoteViewPitch << 8;
		}
	}
	PawnAimPitch = UnwindRot(PawnAimPitch);

	INT PawnAimYaw = UnwindRot(Rotation.Yaw);
	INT AimYaw = 0;

	// If not moving, do turn-in-place calculation (offset of aim angle from pawn)
	// Check Physics mode to avoid doing this when in vehicles etc.
	if(Physics == PHYS_Walking && Velocity.Size() < KINDA_SMALL_NUMBER)
	{
		INT CurrentAimYaw = UnwindRot(PawnAimYaw - RootYaw);
		INT RootRot = 0;
		if(CurrentAimYaw > MaxYawAim)
		{
			RootRot = (CurrentAimYaw - MaxYawAim);
		}
		else if(CurrentAimYaw < -MaxYawAim)
		{
			RootRot = (CurrentAimYaw - (-MaxYawAim));
		}
		RootYaw += RootRot;
		RootYawSpeed = ((FLOAT)RootRot)/DeltaSeconds;

		AimYaw = UnwindRot(PawnAimYaw - RootYaw);
	}
	else
	{
		RootYaw = Rotation.Yaw;
		RootYawSpeed = 0.f;
		AimYaw = 0;
	}

	// Update aiming direction (leave where it is when firing disabled)
	if(!bNoWeaponFiring)
	{
		CurrentSkelAim.X = Clamp<FLOAT>(((FLOAT)AimYaw/16384.f), -1.f, 1.f);
		CurrentSkelAim.Y = Clamp<FLOAT>(((FLOAT)PawnAimPitch/16384.f), -1.f, 1.f);
	}

	// Push info to nodes.
	if(RootRotControl)
	{
		RootRotControl->BoneRotation.Yaw = -AimYaw;
	}

	if(AimNode)
	{
		AimNode->Aim = CurrentSkelAim;
	}

	// Because recoil is applied in component space, and component rotates with aim not with hips, we want its X aim to be zero
	CurrentSkelAim.X = 0;

	if(GunRecoilNode)
	{
		GunRecoilNode->Aim = CurrentSkelAim;
	}

	if(LeftRecoilNode)
	{
		LeftRecoilNode->Aim = CurrentSkelAim;
	}

	if(RightRecoilNode)
	{
		RightRecoilNode->Aim = CurrentSkelAim;
	}

	if (bKilledByBio && Mesh != NULL)
	{
		// first, how far into the burn are we: (scale of 0-9.9)
		FLOAT BurnLevel = Min(((WorldInfo->TimeSeconds-DeathTime)/BioBurnAwayTime*10.0f),9.9f);
		for (INT i = 0; i < Mesh->Materials.Num(); i++)
		{
			UMaterialInstanceConstant* MyMIC = Cast<UMaterialInstanceConstant>(Mesh->Materials(i));
			if (MyMIC != NULL)
			{
				MyMIC->SetScalarParameterValue(BioEffectName, BurnLevel);
			}
		}
		if (BurnLevel >= 9.9f)
		{
			BioBurnAway->DeactivateSystem();
			bKilledByBio = FALSE; // no need to loop this in anymore.
		}
	}

	// if being tracked, update trackers
	if ( Trackers.Num() > 0 )
	{
		FEnemyPosition NewPosition;
		NewPosition.Position = GetTargetLocation();
		NewPosition.Velocity = Velocity;
		NewPosition.Time = WorldInfo->TimeSeconds;

		for ( INT i=0; i<Trackers.Num(); i++ )
		{
			if ( !Trackers(i) || Trackers(i)->bDeleteMe || (Trackers(i)->CurrentlyTrackedEnemy != this) )
			{
				Trackers.Remove(i,1);
				i--;
			}
			else
			{
				Trackers(i)->SavedPositions.AddItem(NewPosition);
			}
		}
	}

	// update eyeheight if someone is viewing through this pawn's eyes
	if ( bUpdateEyeheight )
	{
		UpdateEyeHeight(DeltaSeconds);
	}

	if ( bIsHero )
	{
		if ( Controller && (Role == ROLE_Authority) && (WorldInfo->TimeSeconds - LastAuraCheckTime > 1.0) )
		{
			for ( APawn *P=WorldInfo->PawnList; P!=NULL; P=P->NextPawn )
			{
				if ( P->Controller && ((P->Location - Location).SizeSquared() < Square(AuraRadius)) )
				{
					eventProvideAuraEffect(P);
				}
			}
		}

		if ( Mesh && (Mesh->Scale != DesiredMeshScale) )
		{
			if ( Mesh->Scale < DesiredMeshScale )
			{
				Mesh->Scale = ::Min(DesiredMeshScale, Mesh->Scale + 5.f*DeltaSeconds);
			}
			else
			{
				Mesh->Scale = ::Max(DesiredMeshScale, Mesh->Scale - 5.f*DeltaSeconds);
			}
			Mesh->BeginDeferredUpdateTransform();
		}
	}

	// check overlays
	UBOOL bFadeOut = FALSE;
	if ( RemainingBodyMatDuration > 0.f )
	{
		if (appTrunc(ClientBodyMatDuration) != appTrunc(RemainingBodyMatDuration))
		{
			// replicate updated duration once per second
			ClientBodyMatDuration = RemainingBodyMatDuration;
			bNetDirty = TRUE;
		}
		if ( RemainingBodyMatDuration < BodyMatFadeDuration)
		{
			if ( RemainingBodyMatDuration < 0.f )
			{
				BodyMatColor = FLinearColor(0.f,0.f,0.f,0.f);
				CompressedBodyMatColor = FRotator(0,0,0);
			}
			else
			{
				bFadeOut = TRUE;
				CurrentBodyMatColor = FLinearColor(0.f,0.f,0.f,0.f);
			}
		}
		RemainingBodyMatDuration -= DeltaSeconds;
	}
	else
	{
		if (ClientBodyMatDuration != 0.f)
		{
			ClientBodyMatDuration = 0.f;
			bNetDirty = TRUE;
		}
		BodyMatColor = FLinearColor(0.f,0.f,0.f,0.f);
		CompressedBodyMatColor = FRotator(0,0,0);
	}

	if (!(CurrentBodyMatColor == BodyMatColor) && BodyMaterialInstances.Num() > 0)
	{
		CurrentBodyMatColor = BodyMatColor;
		if ( bFadeOut )
		{
			// fade out overlay
			const FLOAT FadeScale = RemainingBodyMatDuration/BodyMatFadeDuration;
			CurrentBodyMatColor.R *= FadeScale;
			CurrentBodyMatColor.G *= FadeScale;
			CurrentBodyMatColor.B *= FadeScale;
			CurrentBodyMatColor.A *= FadeScale;
		}
		
		for (INT i = 0; i < BodyMaterialInstances.Num(); i++)
		{
			if (BodyMaterialInstances(i) != NULL)
			{
				BodyMaterialInstances(i)->SetVectorParameterValue(DamageParameterName, CurrentBodyMatColor);
			}
		}
	}

	LookYaw = Rotation.Yaw;

	if (Mesh != NULL && Mesh->PhysicsWeight > 0.0f)
	{
		// if recovering from feign death, interpolate animations to max weight
		if (bPlayingFeignDeathRecovery)
		{
			Mesh->PhysicsWeight -= DeltaSeconds * FeignDeathPhysicsBlendOutSpeed;
			if (Mesh->PhysicsWeight <= 0.0f)
			{
				Mesh->PhysicsWeight = 0.0f;
				bPlayingFeignDeathRecovery = FALSE;
				eventStartFeignDeathRecoveryAnim();
			}
			return;
		}
		else if (bBlendOutTakeHitPhysics)
		{
			Mesh->PhysicsWeight -= DeltaSeconds * TakeHitPhysicsBlendOutSpeed;
			if (Mesh->PhysicsWeight <= 0.0f)
			{
				Mesh->PhysicsWeight = 0.0f;
				bBlendOutTakeHitPhysics = FALSE;
				eventTakeHitBlendedOut();
			}
		}
	}

	if ( (WorldInfo->NetMode != NM_DedicatedServer) && Mesh && (Physics != PHYS_RigidBody) )
	{
		// Should we use foot placement on this tick?
		const UBOOL bUseFootPlacement = UseFootPlacementThisTick();

		// enable/disable foot placement skeletal controllers and handle mesh translation if needed
		EnableFootPlacement(bUseFootPlacement);

		if (bUseFootPlacement)
		{
			DoFootPlacement(DeltaSeconds);
		}
		else if(Physics != PHYS_None)
		{
			// If swimming, translate mesh down so its not above water surface
			FLOAT TargetZOffset = (Physics == PHYS_Swimming) ? SwimmingZOffset : BaseTranslationOffset;
			TargetZOffset += CrouchMeshZOffset;

			if(Abs(Mesh->Translation.Z - TargetZOffset) > KINDA_SMALL_NUMBER)
			{
				FLOAT MaxZDelta = (SwimmingZOffsetSpeed * DeltaSeconds);
				Mesh->Translation.Z += ::Clamp<FLOAT>(TargetZOffset - Mesh->Translation.Z, -MaxZDelta, MaxZDelta);
				Mesh->ConditionalUpdateTransform();
			}
		}

		// Position the 1st person weapon
		AUTWeapon* Weap = Cast<AUTWeapon>(Weapon);
		if (Weap != NULL)
		{
			Weap->eventSetPosition(this);
		}
	}

	if(bCanPlayFallingImpacts)
	{
		URB_BodyInstance* BodyInst = NULL;
		if(Mesh)
		{
			BodyInst = Mesh->FindBodyInstanceNamed(TorsoBoneName);
		}
		if(BodyInst && BodyInst->IsValidBodyInstance()) // if we have a torso:
		{
			BodyInst->PreviousVelocity = BodyInst->Velocity;
			BodyInst->Velocity = BodyInst->GetUnrealWorldVelocity();
			if(BodyInst->PreviousVelocity.SizeSquared() != 0.0f)
			{
				FLOAT Speedsq = (BodyInst->Velocity - BodyInst->PreviousVelocity).SizeSquared(); // set up speeds and check for:
				
				if((((BodyInst->Velocity).SizeSquared() == 0.0f) ||
					 WorldInfo->TimeSeconds -  StartFallImpactTime > 1.5f) && CylinderComponent)
				{
					FCheckResult Hit(1.f);
					GWorld->SingleLineCheck(Hit,this,Location-FVector(0,0,(CylinderComponent->CollisionHeight)),Location,TRACE_World|TRACE_StopAtAnyHit);
					if(Hit.Time != 1.f)
					{
						Speedsq = FallSpeedThreshold*FallSpeedThreshold + 1.f; // force a collision
					}
					else // in the air, so let's reset the system.
					{
						StartFallImpactTime = WorldInfo->TimeSeconds;
					}
				}
				if(  Speedsq > FallSpeedThreshold*FallSpeedThreshold)
				{
					// Play the sound and then cancel the sound out so we don't play it again.
					PlaySound( FallImpactSound );
					bCanPlayFallingImpacts=false;
				}
			}
		}
		else
		{
			// we don't have a torso, so lets just cut this off at the pass in the future.
			bCanPlayFallingImpacts = false;
		}
	}
}

// Foot placement - WIP!
void AUTPawn::DoFootPlacement(FLOAT DeltaSeconds)
{
	// translate - old loc, plus trace down (if rendered)
	const FLOAT BaseFloor = Location.Z - CylinderComponent->CollisionHeight - 2.f;
	FLOAT TracedFloor = BaseFloor;
	FLOAT FloorNormalZ = 0.f;
	FLOAT LeftTracedFloor = BaseFloor;
	FLOAT RightTracedFloor = BaseFloor;
	FVector NewTranslation = Mesh->Translation;

	if ( Physics == PHYS_Walking )
	{
		FCheckResult Hit(1.f);
		if ( LeftFootBone != NAME_None )
		{
			const FVector& TraceStart = Mesh->GetBoneLocation(LeftFootBone);
			FVector TraceEnd = TraceStart;
			TraceEnd.Z -= 5.f*(MaxStepHeight + 1.5f*CylinderComponent->CollisionRadius);
			GWorld->SingleLineCheck(Hit, this, TraceEnd , TraceStart, TRACE_AllBlocking  | TRACE_ComplexCollision);

			if ( Hit.Time < 0.2f )
			{
				LeftTracedFloor = ::Min(TracedFloor, Hit.Location.Z);
				FloorNormalZ = Hit.Normal.Z;
			}
		}
		if (RightFootBone != NAME_None )
		{
			const FVector& TraceStart = Mesh->GetBoneLocation(RightFootBone);
			FVector TraceEnd = TraceStart;
			TraceEnd.Z -= 5.f*(MaxStepHeight + 1.5f*CylinderComponent->CollisionRadius);
			GWorld->SingleLineCheck(Hit, this, TraceEnd , TraceStart, TRACE_AllBlocking  | TRACE_ComplexCollision);
			if ( Hit.Time < 0.2f )
			{
				RightTracedFloor = ::Min(TracedFloor, Hit.Location.Z);
				TracedFloor = ::Min(LeftTracedFloor, RightTracedFloor);
				FloorNormalZ = ::Max(Hit.Normal.Z, FloorNormalZ);
			}
		}

		if ( !Acceleration.IsZero() )
		{
			// translate mesh based on floor
			FloorNormalZ = ::Clamp<FLOAT>(FloorNormalZ, WalkableFloorZ, 1.f);
			TracedFloor = BaseFloor - CylinderComponent->CollisionRadius * appSqrt(1.f/Square(FloorNormalZ) - 1.f);
		}
		else if ( ::Max(LeftTracedFloor, RightTracedFloor) - BaseFloor < -1.f*MaxStepHeight )
		{
			TracedFloor = BaseFloor;
		}

		if ( (DeltaSeconds > 0.1f) || (Abs(OldLocationZ - Location.Z) > MaxStepHeight) )
		{
			ZSmoothingRate = 0.f;
		}
		else
		{
			ZSmoothingRate = (1.f - 10.f*DeltaSeconds)*ZSmoothingRate + 10.f*DeltaSeconds*(OldLocationZ - Location.Z);
			NewTranslation.Z += (OldLocationZ - Location.Z - ZSmoothingRate);
		}
	}

	OldLocationZ = Location.Z;

	if ( DeltaSeconds > 0.111f )
		NewTranslation.Z = ::Max(-1.f*MaxStepHeight, TracedFloor-BaseFloor) + (bIsCrouched ? CrouchTranslationOffset : BaseTranslationOffset);
	else
		NewTranslation.Z = (1.f - 9.f*DeltaSeconds)*NewTranslation.Z + 9.f*DeltaSeconds*(::Max(-1.f*MaxStepHeight, TracedFloor-BaseFloor) + (bIsCrouched ? CrouchTranslationOffset : BaseTranslationOffset));

	if (Mesh->Translation != NewTranslation)
	{
		Mesh->Translation = NewTranslation;
		Mesh->ConditionalUpdateTransform();
	}
}

UBOOL AUTPawn::UseFootPlacementThisTick()
{
	// Foot placement must be enabled for this pawn
	if (!bEnableFootPlacement)
	{
		return FALSE;
	}

	// Pawn must be walking
	if (Physics != PHYS_Walking)
	{
		return FALSE;
	}

	// Pawn can't be crouched
	if (bIsCrouched)
	{
		return FALSE;
	}

	// Is pawn recently rendered?
	if (WorldInfo->TimeSeconds - Mesh->LastRenderTime >= 0.2f)
	{
		return FALSE;
	}

	// is the pawn stopped?
	if (Velocity.SizeSquared() > KINDA_SMALL_NUMBER)
	{
		return FALSE;
	}

	// Is pawn close enough to another player?
	UBOOL bDoFootPlacement = FALSE;
	for (FPlayerIterator It(GEngine); It; ++It)
	{
		if (It->Actor != NULL && It->Actor->ViewTarget != NULL && (Location - It->Actor->ViewTarget->Location).SizeSquared() < Square(It->Actor->LODDistanceFactor) * MaxFootPlacementDistSquared)
		{
			bDoFootPlacement = TRUE;
			break;
		}
	}
	if (!bDoFootPlacement)
	{
		return FALSE;
	}

	// Passed all tests, go ahead and do it...
	return TRUE;
}

void AUTPawn::EnableFootPlacement(UBOOL bEnabled)
{
	if ( !LeftLegControl || !RightLegControl )
	{
		return;
	}

	const UBOOL bLeftLegControlActive	= (LeftLegControl->StrengthTarget == 1.f);
	const UBOOL bRightLegControlActive	= (RightLegControl->StrengthTarget == 1.f);
	
	// If there is a change from last tick...
	if ( bEnabled != bLeftLegControlActive || bEnabled != bRightLegControlActive )
	{
		LeftLegControl->SetSkelControlActive(bEnabled);
		RightLegControl->SetSkelControlActive(bEnabled);

		// If we have just disabled then handle mesh translation
		if ( !bEnabled )
		{
			Mesh->Translation = FVector(Mesh->Translation.X,Mesh->Translation.Y, bIsCrouched? CrouchTranslationOffset : BaseTranslationOffset);
			Mesh->ConditionalUpdateTransform();
		}
	}

	if ( bEnabled )
	{
		LeftLegControl->MaxUpAdjustment = MaxStepHeight; // FIXMESTEVE - set all of these in anim tree controller default properties
		RightLegControl->MaxUpAdjustment = MaxStepHeight;
		LeftLegControl->MaxDownAdjustment = MaxStepHeight;
		RightLegControl->MaxDownAdjustment = MaxStepHeight;
	}
}

void AUTPawn::UpdateEyeHeight(FLOAT DeltaSeconds)
{
	if ( !IsLocallyControlled() )
	{
		EyeHeight = BaseEyeHeight;
		return;
	}

	if ( !Controller )
	{
		EyeHeight = 0;
		return;
	}
	eventUpdateEyeHeight(DeltaSeconds);
}

FLOAT AUTPawn::GetGravityZ()
{
	if ( (Physics == PHYS_RigidBody) && PhysicsVolume->bWaterVolume )
	{
		return (1.f - Buoyancy) * Super::GetGravityZ() * CustomGravityScaling;
	}
	return Super::GetGravityZ() * CustomGravityScaling;
}

void AUTPawn::setPhysics(BYTE NewPhysics, AActor *NewFloor, FVector NewFloorV)
{
	// When in full ragdoll, want in post async work so that any traces in Pre (eg weapons) check against last frames physics bone positions.
	if(NewPhysics == PHYS_RigidBody && Physics != PHYS_RigidBody)
	{
		Mesh->SetTickGroup(TG_PostAsyncWork);
	}
	else if(NewPhysics == PHYS_RigidBody && Physics != PHYS_RigidBody)
	{
		Mesh->SetTickGroup(TG_PreAsyncWork);
	}

	if (Physics == PHYS_Falling)
	{
		StartedFallingTime = WorldInfo->TimeSeconds;
	}

	const UBOOL bShouldNotifyStopFalling = ( bNotifyStopFalling && (Physics == PHYS_Falling) );
	Super::setPhysics(NewPhysics, NewFloor, NewFloorV);
	if ( bShouldNotifyStopFalling && (Physics != PHYS_Falling) )
		eventStoppedFalling();
}

void AUTPawn::physFalling(FLOAT DeltaTime, INT Iterations)
{
	Super::physFalling(DeltaTime, Iterations);

	if (!Velocity.IsZero())
	{
		StartedFallingTime = WorldInfo->TimeSeconds;
	}
	else if (WorldInfo->TimeSeconds - StartedFallingTime > 5.0f)
	{
		if (WorldInfo->TimeSeconds - StartedFallingTime - DeltaTime < 5.0f)
		{
			// try slight teleport to get out of whatever we're stuck in
			GWorld->FarMoveActor(this, Location + FVector(1.f, 1.f, 1.f));
		}
		else if (WorldInfo->TimeSeconds - StartedFallingTime > 10.0f)
		{
			eventStuckFalling();
		}
	}
}

FVector AUTPawn::CalculateSlopeSlide(const FVector& Adjusted, const FCheckResult& Hit)
{
	FVector Result = (Adjusted - Hit.Normal * (Adjusted | Hit.Normal)) * (1.f - Hit.Time);
	
	// check if hit material allows slope boosting
	if ( (SlopeBoostFriction == 0.f) || (Hit.Material && Hit.Material->GetPhysicalMaterial() && (Hit.Material->GetPhysicalMaterial()->Friction < SlopeBoostFriction)) )
		return Result;

	// prevent boosting up slopes
	if ( Result.Z > 0.f )
    	Result.Z = ::Min(Result.Z, Adjusted.Z * (1.f - Hit.Time));
	return Result;
}


/** Get height/radius of big cylinder around this actors colliding components.
*/
void AUTPawn::GetBoundingCylinder(FLOAT& CollisionRadius, FLOAT& CollisionHeight) const
{
	if ( CylinderComponent && (CylinderComponent==CollisionComponent) )
	{
		CollisionRadius = CylinderComponent->CollisionRadius;
		CollisionHeight = CylinderComponent->CollisionHeight;
	}
	else
	{
		Super::GetBoundingCylinder(CollisionRadius, CollisionHeight);
	}
}

void AUTPawn::RestorePreRagdollCollisionComponent()
{
	// reset physics/etc
	if (PreRagdollCollisionComponent != NULL && PreRagdollCollisionComponent->IsAttached() && PreRagdollCollisionComponent->GetOwner() == this)
	{
		CollisionComponent = PreRagdollCollisionComponent;
	}
	else
	{
		CollisionComponent = CylinderComponent;
	}
}

/** Util that makes sure the overlay component is last in the AllComponents and Components arrays. */
void AUTPawn::EnsureOverlayComponentLast()
{
	// Iterate over Components array looking for OverlayMesh
	for(INT i=0; i<Components.Num(); i++)
	{
		if(Components(i) == OverlayMesh)
		{
			// When/if we find it, remove it and add to the end instead.
			Components.Remove(i);
			Components.AddItem(OverlayMesh);
			continue;
		}
	}

	// Iterate over AllComponents array looking for OverlayMesh
	for(INT i=0; i<AllComponents.Num(); i++)
	{
		if(AllComponents(i) == OverlayMesh)
		{
			// When/if we find it, remove it and add to the end instead.
			AllComponents.Remove(i);
			AllComponents.AddItem(OverlayMesh);
			continue;
		}
	}
}

FVector AUTPawn::GetTargetLocation(AActor* RequestedBy, UBOOL bRequestAlternateLoc) const
{
	if (bFeigningDeath && Physics == PHYS_RigidBody)
	{
		return Mesh->LocalToWorld.GetOrigin();
	}
	else
	{
		return Super::GetTargetLocation(RequestedBy, bRequestAlternateLoc);
	}
}

void AUTPawn::physicsRotation(FLOAT deltaTime, FVector OldVelocity)
{
	if ( !Controller )
		return;
	if( !bCrawler && (Rotation == DesiredRotation) && (MaxLeanRoll <= 0) && !IsHumanControlled() )
		return;

	// Accumulate a desired new rotation.
	FRotator NewRotation = Rotation;	
	const FRotator& deltaRot = Controller->SetRotationRate(deltaTime);

	if( (Physics == PHYS_Ladder) && OnLadder )
	{
		// must face ladder
		NewRotation = OnLadder->WallDir;
	}
	else
	{
		//YAW
		if( DesiredRotation.Yaw != NewRotation.Yaw )
		{
			NewRotation.Yaw = fixedTurn(NewRotation.Yaw, DesiredRotation.Yaw, deltaRot.Yaw);
		}

		// PITCH
		if( !bRollToDesired && ((Physics == PHYS_Walking) || (Physics == PHYS_Falling)) )
		{
			DesiredRotation.Pitch = 0;
		}
		if( (!bCrawler || (Physics != PHYS_Walking)) && (DesiredRotation.Pitch != NewRotation.Pitch) )
		{
			NewRotation.Pitch = fixedTurn(NewRotation.Pitch, DesiredRotation.Pitch, deltaRot.Pitch);
		}
	}

	// ROLL
	if( bRollToDesired )
	{
		if( DesiredRotation.Roll != NewRotation.Roll )
		{
			NewRotation.Roll = fixedTurn(NewRotation.Roll, DesiredRotation.Roll, deltaRot.Roll);
		}
	}
	else if( bCrawler  )
	{
		if( Physics != PHYS_Walking )
		{
			// Straighten out
			NewRotation.Pitch = fixedTurn(NewRotation.Pitch, 0, deltaRot.Pitch);
			NewRotation.Roll = fixedTurn(NewRotation.Roll, 0, deltaRot.Roll);
		}
		else
		{
			NewRotation = FindSlopeRotation(Floor,NewRotation);
		}
	}
	else if( MaxLeanRoll > 0 )
	{
		NewRotation.Roll = NewRotation.Roll & 65535;
		if( NewRotation.Roll < 32768 )
		{
			if( NewRotation.Roll > MaxLeanRoll )
			{
				NewRotation.Roll = MaxLeanRoll;
			}
		}
		else if( NewRotation.Roll < 65536 - MaxLeanRoll )
		{
			NewRotation.Roll = 65536 - MaxLeanRoll;
		}

		// Pawns roll based on physics
		if( (Physics == PHYS_Walking) && (Velocity.SizeSquared() < 40000.f) )
		{
			const FLOAT SmoothRoll = Min(1.f, 8.f * deltaTime);
			if( NewRotation.Roll < 32768 )
			{
				NewRotation.Roll = appTrunc(NewRotation.Roll * (1 - SmoothRoll));
			}
			else
			{
				NewRotation.Roll = appTrunc(NewRotation.Roll + (65536 - NewRotation.Roll) * SmoothRoll);
			}
		}
		else
		{
			FVector RealAcceleration = (Velocity - OldVelocity)/deltaTime;
			if( RealAcceleration.SizeSquared() > 10000.f )
			{
				NewRotation.Roll = 0;

				RealAcceleration = FRotationMatrix(NewRotation).Transpose().TransformNormal(RealAcceleration); //y component will affect roll
				if( RealAcceleration.Y > 0 )
				{
					NewRotation.Roll = Min( MaxLeanRoll, appTrunc( 2.f * RealAcceleration.Y * MaxLeanRoll/AccelRate ) );
				}
				else
				{
					NewRotation.Roll = ::Max( 65536 - MaxLeanRoll, appTrunc( 65536.f + 2.f * RealAcceleration.Y * MaxLeanRoll/AccelRate ) );
				}

				//smoothly change rotation
				Rotation.Roll = Rotation.Roll & 65535;
				if (NewRotation.Roll > 32768)
				{
					if (Rotation.Roll < 32768)
					{
						Rotation.Roll += 65536;
					}
				}
				else if (Rotation.Roll > 32768)
				{
					Rotation.Roll -= 65536;
				}
	
				const FLOAT SmoothRoll = Min( 1.f, 5.f * deltaTime );
				NewRotation.Roll = appTrunc( NewRotation.Roll * SmoothRoll + Rotation.Roll * (1 - SmoothRoll) );
			}
			else
			{
				const FLOAT SmoothRoll = Min( 1.f, 8.f * deltaTime );
				if( NewRotation.Roll < 32768 )
				{
					NewRotation.Roll = appTrunc( NewRotation.Roll * (1 - SmoothRoll) );
				}
				else
				{
					NewRotation.Roll = appTrunc( NewRotation.Roll + (65536 - NewRotation.Roll) * SmoothRoll );
				}
			}
		}
	}
	else
	{
		NewRotation.Roll = 0;
	}

	// Set the new rotation.
	// fixedTurn() returns denormalized results so we must convert Rotation to prevent negative values in Rotation from causing unnecessary MoveActor() calls
	if( NewRotation != Rotation.GetDenormalized() )
	{
		FCheckResult Hit(1.f);
		GWorld->MoveActor( this, FVector(0,0,0), NewRotation, 0, Hit );
	}
}

UBOOL AUTPawn::ShouldTrace(UPrimitiveComponent* Primitive, AActor* SourceActor, DWORD TraceFlags)
{
	// by default dead bodies will get hit by traces intended to hit only blocking collision because APawn::ShouldTrace() doesn't check TRACE_Blocking
	// so we specifically check for that here and ignore it
	return (bBlockActors || Physics != PHYS_RigidBody || Health > 0 || ConstCast<AUTPawn>(SourceActor) == NULL || !(TraceFlags & TRACE_Blocking)) && Super::ShouldTrace(Primitive, SourceActor, TraceFlags);
}

UBOOL AUTPawn::IgnoreBlockingBy(const AActor* Other) const
{
	// by default dead bodies will block living players because they become encroachers and check bCollideWorld (legacy thing that's scary to fix)
	// so we specifically check for that here and ignore it
	return (!bBlockActors && Physics == PHYS_RigidBody && Health <= 0 && ConstCast<AUTPawn>(Other) != NULL) || Super::IgnoreBlockingBy(Other);
}

/**
Hook to allow actors to render HUD overlays for themselves.
Assumes that appropriate font has already been set
*/
void AUTPawn::NativePostRenderFor(APlayerController *PC, UCanvas *Canvas, FVector CameraPosition, FVector CameraDir)
{
	ULocalPlayer *MyLocalPlayer = PC ? Cast<ULocalPlayer>(PC->Player) : NULL;
	if ( MyLocalPlayer 
		&& (bPostRenderIfNotVisible || MyLocalPlayer->GetActorVisibility(this))
		&& ((CameraDir | (Location - CameraPosition)) > 0.f) 
		&& (PC->ViewTarget != this) 
		&& ((CameraPosition - Location).SizeSquared() * Square(PC->LODDistanceFactor) < Square(TeamBeaconMaxDist))
		&& WorldInfo->GRI && PlayerReplicationInfo && PC->ViewTarget )
	{
		if ( !bPostRenderOtherTeam && !WorldInfo->GRI->OnSameTeam(this, PC) && (WorldInfo->TimeSeconds - LastPostRenderTraceTime < 0.5f) )
		{
			return;
		}

		eventPostRenderFor(PC, Canvas, CameraPosition, CameraDir);
	}
}



