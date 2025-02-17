
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "GearGame.h"

#include "GearGameSpecialMovesClasses.h"
#include "GearGameWeaponClasses.h"
#include "GearGameAnimClasses.h"


// Base special moves
IMPLEMENT_CLASS(UGearSpecialMove)
IMPLEMENT_CLASS(UGSM_BaseVariableFall)
IMPLEMENT_CLASS(UGSM_BasePawnToPawnInteractionLeader)
IMPLEMENT_CLASS(UGSM_BasePlaySingleAnim)
IMPLEMENT_CLASS(UGSM_InteractionPawnLeader_Base)

IMPLEMENT_CLASS(UGSM_Run2Cover)
IMPLEMENT_CLASS(UGSM_CoverSlip)
IMPLEMENT_CLASS(UGSM_StdLvlSwatTurn)
IMPLEMENT_CLASS(UGSM_RoadieRun)
IMPLEMENT_CLASS(UGSM_RecoverFromRagdoll)
IMPLEMENT_CLASS(UGSM_PushButton)
IMPLEMENT_CLASS(UGSM_PushObject)
IMPLEMENT_CLASS(UGSM_MantleOverCoverBase)
IMPLEMENT_CLASS(UGSM_MidLvlJumpOver)
IMPLEMENT_CLASS(UGSM_MantleOverGears)
IMPLEMENT_CLASS(UGSM_MantleOverLocust)
IMPLEMENT_CLASS(UGSM_GrabWretch)
IMPLEMENT_CLASS(UGSM_RaiseShieldOverHead)
IMPLEMENT_CLASS(UGSM_Engage)
IMPLEMENT_CLASS(UGSM_EngageStart)
IMPLEMENT_CLASS(UGSM_DBNO)

IMPLEMENT_CLASS(UGSM_Berserker_Charge)
IMPLEMENT_CLASS(UGSM_Berserker_Smash)

IMPLEMENT_CLASS(UGSM_Brumak_BasePlaySingleAnim)
IMPLEMENT_CLASS(UGSM_Brumak_MeleeAttack)
IMPLEMENT_CLASS(UGSM_Brumak_OverlayBite)

IMPLEMENT_CLASS(AMantlePlaceholderCylinder);

UBOOL AMantlePlaceholderCylinder::IgnoreBlockingBy(const AActor *Other) const
{
	return Other == PawnToIgnore;
}

void UGearSpecialMove::SetBasedPosition( FBasedPosition& BP, FVector inLoc )
{
	if( PawnOwner != NULL )
	{
		PawnOwner->SetBasedPosition( BP, inLoc, PawnOwner->ClampedBase );
	}	
}
FVector UGearSpecialMove::GetBasedPosition( FBasedPosition& BP )
{
	if( PawnOwner != NULL )
	{
		return PawnOwner->GetBasedPosition( BP );
	}

	debugf(TEXT("Tried to get base position w/ no pawn %s"), *GetName() );
	return FVector(0,0,0);
}


FVector UGSM_MantleOverCoverBase::GetIdealCameraOrigin()
{
	// for the duration of a mantle, we base the camera on a bone to get better motion
	return PawnOwner->Mesh->GetBoneLocation(PawnOwner->PelvisBoneName) + FVector(0,0,16.f);
}

FVector UGSM_MantleOverCoverBase::GetMantleDir( FVector InStartLoc, FVector InEndLoc )
{
	return (InEndLoc - InStartLoc).SafeNormal();
}

void UGSM_MantleOverCoverBase::TickSpecialMove(float DeltaTime)
{
	if( bJumpToFallPhysicsTransition )
	{
		bJumpToFallPhysicsTransition = FALSE;

		// Also simulate physics for the rest of the frame time.
		//PawnOwner->startNewPhysics(JumpToFallTransitionTime, 0);
		eventDoingJumpToFallPhysicsTransition();
	}
	Super::TickSpecialMove(DeltaTime);
}

FVector UGSM_MantleOverGears::GetIdealCameraOrigin()
{
	// for the duration of a mantle, we base the camera on a bone to get better motion
	FVector IdealCameraOrigin = Super::GetIdealCameraOrigin();

	if (VariationIndex == 1)
	{
		IdealCameraOrigin.Z += RollCameraOriginZOffset;
	}

	return IdealCameraOrigin;
}

FVector UGSM_MantleOverLocust::GetIdealCameraOrigin()
{
	// for the duration of a mantle, we base the camera on a bone to get better motion
	FVector IdealCameraOrigin = Super::GetIdealCameraOrigin();

	if (VariationIndex == 1)
	{
		IdealCameraOrigin.Z += 16.f;
	}

	return IdealCameraOrigin;
}


/************************************************************************************
 * UGearSpecialMove
 ***********************************************************************************/

FLOAT UGearSpecialMove::GetSpeedModifier()
{
	// Have another speed modifier for HeavyWeapons.
	if( PawnOwner && PawnOwner->MyGearWeapon && PawnOwner->MyGearWeapon->WeaponType == WT_Heavy )
	{
		return HeavyWeaponSpeedModifier;
	}
	return SpeedModifier;
}

/**
 * Turn a World Space Offset into an Rotation relative Offset.
 */
FVector UGearSpecialMove::WorldToRelativeOffset(FRotator InRotation, FVector WorldSpaceOffset) const
{
	FRotationMatrix	RotM(InRotation);
	return FVector(WorldSpaceOffset | RotM.GetAxis(0), WorldSpaceOffset | RotM.GetAxis(1), WorldSpaceOffset | RotM.GetAxis(2));
}

/** Do the opposite as above. Get a Rotation relative offset, and turn it into a world space offset */
FVector UGearSpecialMove::RelativeToWorldOffset(FRotator InRotation, FVector RelativeSpaceOffset) const
{
	FRotationMatrix	RotM(InRotation);
	return RelativeSpaceOffset.X * RotM.GetAxis(0) + RelativeSpaceOffset.Y * RotM.GetAxis(1) + RelativeSpaceOffset.Z * RotM.GetAxis(2);
}

void UGearSpecialMove::SetReachPreciseDestination(FVector DestinationToReach, UBOOL bCancel)
{
	// Cancel Precise Destination move
	if( bCancel )
	{
		bReachPreciseDestination	= FALSE;
		bReachedPreciseDestination	= FALSE;
		PreciseDestBase				= NULL;
	}
	// Start a new Precise Destination move
	else
	{
		PreciseDestination			= DestinationToReach;
		bReachPreciseDestination	= TRUE;
		bReachedPreciseDestination	= FALSE;

		// If PawnOwner is based on something that is not world geometry, then make destination relative to that.
		// For cases when PawnOwner is on a moving vehicle
		if( PawnOwner->Base && !PawnOwner->Base->bWorldGeometry )
		{
			PreciseDestBase = PawnOwner->Base;
			PreciseDestRelOffset = WorldToRelativeOffset(PreciseDestBase->Rotation, PreciseDestination - PreciseDestBase->Location);
		}
	}
}


void UGearSpecialMove::SetFacePreciseRotation(FRotator RotationToFace, FLOAT InterpolationTime)
{
	PreciseRotation						= RotationToFace;
	PreciseRotationInterpolationTime	= InterpolationTime;
	bReachPreciseRotation				= TRUE;
	bReachedPreciseRotation				= FALSE;
}

void UGearSpecialMove::PrePerformPhysics(FLOAT DeltaTime)
{
	// Below, we require a locally controlled Pawn, unless bForcePrecisePosition.
	if( !PawnOwner || (!bForcePrecisePosition && !PawnOwner->IsLocallyControlled()) )
	{
		return;
	}

	// Reach for a precise destination
	// Controller.bPreciseDestination is not precise enough, so we have this alternate method to get as close as possible to destination.
	if( bReachPreciseDestination && !bReachedPreciseDestination )
	{
		// Update Precise Destination if we're based on a mover
		if( PreciseDestBase )
		{
			PreciseDestination = PreciseDestBase->Location + RelativeToWorldOffset(PreciseDestBase->Rotation, PreciseDestRelOffset);
		}

		// Distance to Destination
		const FLOAT Distance = (PreciseDestination - PawnOwner->Location).Size2D();

		if( Abs(Distance) > 1.f )
		{
			// Push Pawn at full speed
			const FLOAT		PushMagnitude	= Min( (Distance / DeltaTime), PawnOwner->GroundSpeed * PawnOwner->MaxSpeedModifier());
			const FVector	Direction		= (PreciseDestination - PawnOwner->Location).SafeNormal2D();

			PawnOwner->Velocity		= Direction * PushMagnitude;
			PawnOwner->Acceleration	= (PawnOwner->Velocity / DeltaTime).SafeNormal();

			//debugf(TEXT("Distance: %f, Vect %s, Vel: %s, Accel: %s, DeltaTime: %f"), Distance, *(PreciseDestination - PawnOwner->Location).ToString(), *PawnOwner->Velocity.ToString(), *PawnOwner->Acceleration.ToString(), DeltaTime );
		}
		else
		{
			// PawnOwner is close enough, consider position reached
			PawnOwner->Velocity			= FVector(0.f);
			PawnOwner->Acceleration		= FVector(0.f);
			bReachedPreciseDestination	= TRUE;
		}
	}

	// Precise Rotation interpolation
	if( bReachPreciseRotation && !bReachedPreciseRotation )
	{
		FRotator NewRotation = PawnOwner->Rotation;

		if( PreciseRotationInterpolationTime > DeltaTime )
		{
			// Delta rotation
			const FRotator RotDelta	= (PreciseRotation.GetNormalized() - PawnOwner->Rotation.GetNormalized()).GetNormalized();
			NewRotation.Yaw = (PawnOwner->Rotation + RotDelta * (DeltaTime / PreciseRotationInterpolationTime)).GetNormalized().Yaw;
			PreciseRotationInterpolationTime -= DeltaTime;
		}
		else
		{
			NewRotation.Yaw			= PreciseRotation.Yaw;
			bReachedPreciseRotation	= TRUE;
		}

		ForcePawnRotation(PawnOwner, NewRotation);
	}

	// Send event once Pawn has reached precise position
	if( bReachedPreciseRotation || bReachedPreciseDestination )
	{
		UBOOL bDelay = FALSE;

		if( bReachPreciseDestination && !bReachedPreciseDestination )
		{
			bDelay = TRUE;
		}

		if( bReachPreciseRotation && !bReachedPreciseRotation )
		{
			bDelay = TRUE;
		}

		if( !bDelay )
		{
			bReachPreciseRotation		= FALSE;
			bReachedPreciseRotation		= FALSE;
			bReachPreciseDestination	= FALSE;
			bReachedPreciseDestination	= FALSE;
			eventReachedPrecisePosition();
		}
	}
}

/** Forces Pawn's rotation to a given Rotator */
void UGearSpecialMove::ForcePawnRotation(APawn* P, FRotator NewRotation)
{
	if( !P || P->Rotation == NewRotation )
	{
		return;
	}

	P->SetRotation(NewRotation);
	P->DesiredRotation = NewRotation;

	// Update AI Controller as well.
	// Don't change controller rotation if human player is in free cam
	if( P->Controller && (!P->IsHumanControlled() || !P->eventInFreeCam()) )
	{
		P->Controller->DesiredRotation.Yaw = P->Rotation.Yaw;
		FRotator ControllerRot = P->Controller->Rotation;
		ControllerRot.Yaw = P->Rotation.Yaw;
		P->Controller->SetRotation(ControllerRot);
		P->Controller->SetFocalPoint( P->Location + NewRotation.Vector() * 1024.f );	// only needed when bLockPawnRotation is not set.
	}
}


void UGearSpecialMove::TickSpecialMove(float DeltaTime)
{
	// See if we should be notifying special move that we're mirror transition safe
	if( bMirrorTransitionSafeNotify )
	{
		if( eventIsMirrorTransitionSafe() )
		{
			eventOnMirrorTransitionSafeNotifyInternal();
		}
	}
}

/************************************************************************************
 * UGSM_InteractionPawnLeader_Base
 ***********************************************************************************/

void UGSM_InteractionPawnLeader_Base::PrePerformPhysics(FLOAT DeltaTime)
{
	Super::PrePerformPhysics(DeltaTime);

	//debugf(TEXT("UGSM_InteractionPawnLeader_Base::PrePerformPhysics bAlignPawns: %d, PawnOwner: %s, Follower: %s"), bAlignPawns, *PawnOwner->GetName(), *Follower->GetName());

	// Align Pawns. Do that if locally controlled only
	if( bAlignPawns && PawnOwner && Follower )
	{
		//debugf(TEXT(" bAlignPawns == TRUE"));
		const FVector VectLeaderToFollower = Follower->Location - PawnOwner->Location;
		const FLOAT DistanceBetweenPawns2D = VectLeaderToFollower.Size2D();
		const FVector LeaderToFollowerDir = VectLeaderToFollower.SafeNormal2D();

		if( Abs(DistanceBetweenPawns2D - AlignDistance) > 0.01f )
		{
			const FVector PushVelocity = (LeaderToFollowerDir * (DistanceBetweenPawns2D - AlignDistance) * 0.5f) / DeltaTime;
			//debugf(TEXT(" PrePerformPhysics AlignDistance: %f, DistanceBetweenPawns2D: %f, PushVelocity: %s"), AlignDistance, DistanceBetweenPawns2D, *PushVelocity.ToString());

			PawnOwner->Velocity		= PushVelocity;
			PawnOwner->Acceleration	= PushVelocity.SafeNormal2D();

			Follower->Velocity		= -PushVelocity;
			Follower->Acceleration	= -PushVelocity.SafeNormal2D();
		}
		// Position reached
		else
		{
			PawnOwner->Velocity			= FVector(0.f);
			PawnOwner->Acceleration		= FVector(0.f);

			Follower->Velocity			= FVector(0.f);
			Follower->Acceleration		= FVector(0.f);
		}
		
		// Take care of rotation now
		FRotator NewPawnRotation = LeaderToFollowerDir.Rotation();
		if( NewPawnRotation != PawnOwner->Rotation )
		{
			//debugf(TEXT(" PrePerformPhysics NewPawnRotation: %s, PawnOwnerRotation: %s"), *NewPawnRotation.ToString(), *PawnOwner->Rotation.ToString());
			NewPawnRotation = RInterpTo(PawnOwner->Rotation, NewPawnRotation, DeltaTime, 12.f);
			ForcePawnRotation(PawnOwner, NewPawnRotation);
		}

		NewPawnRotation = bAlignFollowerLookSameDirAsMe ? LeaderToFollowerDir.Rotation() : (-LeaderToFollowerDir).Rotation();
		if( NewPawnRotation != Follower->Rotation )
		{
			//debugf(TEXT(" PrePerformPhysics NewPawnRotation: %s, FollowerRotation: %s"), *NewPawnRotation.ToString(), *Follower->Rotation.ToString());
			NewPawnRotation = RInterpTo(Follower->Rotation, NewPawnRotation, DeltaTime, 12.f);
			ForcePawnRotation(Follower, NewPawnRotation);
		}
	}
}


/************************************************************************************
 * UGSM_BaseVariableFall
 ***********************************************************************************/

void UGSM_BaseVariableFall::TickSpecialMove(FLOAT DeltaTime)
{
	Super::TickSpecialMove(DeltaTime);

	// if in the variable fall part detect point of impact
	if( MoveType == EMT_Fall )
	{
#if 0
		debugf(TEXT("%3.2f Pawn [%s] PhysicMode: %d"), GWorld->GetTimeSeconds(), *PawnOwner->GetName(), int(PawnOwner->Physics));
#endif
		// Did Pawn already land?
		if( PawnOwner->Physics != PHYS_Falling && // falling
			PawnOwner->Physics != PHYS_Flying && // jumping? (root motion accel?)
			PawnOwner->Physics != PHYS_None )	// jump w/ root motion translation
		{
#if 0
			debugf(TEXT("%3.2f Pawn [%s] had already landed."), GWorld->GetTimeSeconds(), *PawnOwner->GetName());
#endif
			eventLanded(0.f, 0.f);
		}
		// Otherwise, trace along velocity to predict point of impact.
		else if( PreImpactTime > 0.f )
		{
			const FLOAT		VelSize			= PawnOwner->Velocity.Size();

			if( VelSize > 0.f )
			{
				// Location of collision cylinder
				const FVector	ColLocation		= PawnOwner->CollisionComponent ? (PawnOwner->Location + PawnOwner->CollisionComponent->Translation) : PawnOwner->Location;
				// Start location of trace. Use velocity to predict where Pawn will be on next frame.
				const FVector	StartLocation	= ColLocation + PawnOwner->Velocity * DeltaTime;
				// Distance to check from point of impact
				const FLOAT		TraceDist		= 1024.f;
				const FVector	CheckVector		= PawnOwner->Velocity.SafeNormal() * TraceDist;
				// Collision box extent
				const FVector	Extent(PawnOwner->CylinderComponent->CollisionRadius, PawnOwner->CylinderComponent->CollisionRadius, PawnOwner->CylinderComponent->CollisionHeight);
#if 0
				debugf(TEXT("%3.2f Check for Impact, %s, Check Vect: %s"), GWorld->GetTimeSeconds(), *PawnOwner->GetName(), *CheckVector.ToString());
#endif
				// Trace to see if we hit something.
				FCheckResult Hit(1.f);
				GWorld->SingleLineCheck(Hit, PawnOwner, StartLocation + CheckVector, StartLocation, TRACE_World|TRACE_StopAtAnyHit, Extent);

				// If hit something, then we can trigger the pre-land animation.
				if( Hit.Time < 1.f )
				{
					FLOAT TimeToImpact = (Hit.Time * TraceDist) / VelSize;

//					debugf(TEXT("TimeToImpact %f Thresh %f Dist %f"), TimeToImpact, PreImpactTime, (Hit.Location - ColLocation).Size() );

					if( TimeToImpact <= PreImpactTime )
					{
						const FLOAT DistanceToImpact = (Hit.Location - ColLocation).Size();
						eventLanded(DistanceToImpact, TimeToImpact);
					}
				}
			}
		}
	}

	// Disable Gravity simulation when jumping, because it's using PHYS_Flying or PHYS_None.
	PawnOwner->bSimulateGravity = (MoveType != EMT_Jump);
}


/************************************************************************************
 * UGSM_StdLvlSwatTurn
 ***********************************************************************************/

void UGSM_StdLvlSwatTurn::TickSpecialMove(FLOAT DeltaTime)
{
	Super::TickSpecialMove(DeltaTime);

	// If haven't played the end animation, look for when we are close enough to start it.
	if( !bPlayedEndAnim )
	{
		const FLOAT DistToTarget = (*SwatTurnDestination - PawnOwner->Location).Size2D();
		
		if( DistToTarget < EndTurnAnimDist )
		{
			bPlayedEndAnim = TRUE;
			eventPlayEndAnimation(DistToTarget);
		}
	}
}


/************************************************************************************
 * UGSM_RoadieRun
 ***********************************************************************************/

void UGSM_RoadieRun::TickSpecialMove(FLOAT DeltaTime)
{
	Super::TickSpecialMove(DeltaTime);

	if( PawnOwner != NULL && PawnOwner->IsLocallyControlled() )
	{
		if( PawnOwner->Velocity.Size() < PawnOwner->DefaultGroundSpeed * 0.3f )
		{
			RunAbortTimer += DeltaTime;
			if( RunAbortTimer > 0.3f )
			{
				PawnOwner->eventLocalEndSpecialMove(PawnOwner->SpecialMove);	
			}
		}
		else
		{
			RunAbortTimer = 0.f;
		}
	}
}

FLOAT UGSM_RoadieRun::GetSpeedModifier()
{
	FLOAT BoostModifier = 0.f;
	FLOAT ShieldMultiplier = 1.f;

	if (PawnOwner != NULL)
	{
		// adjustment for shield
		ShieldMultiplier = PawnOwner->eventIsCarryingShield() ? PawnOwner->ShieldMovementSpeedPercentage : 1.f;

		if (PawnOwner->WorldInfo->TimeSeconds - PawnOwner->RoadieRunBoostTime < 1.5f)
		{
			// apply a small speed boost
			BoostModifier = ((1.f - ((PawnOwner->WorldInfo->TimeSeconds - PawnOwner->RoadieRunBoostTime)/1.5f)) * 0.5f);
		}
	}
	return (Super::GetSpeedModifier() + BoostModifier) * ShieldMultiplier;
}


/************************************************************************************
 * UGSM_PushButton
 ***********************************************************************************/

void UGSM_PushButton::TickSpecialMove(FLOAT DeltaTime)
{
	Super::TickSpecialMove(DeltaTime);

	USkelControlBase* Ctrl = PawnOwner->IKCtrl_LeftHand;

	if(Ctrl)
	{
		// If we have finished blending in, but the target is still non-zero - blend back
		if(Ctrl->BlendTimeToGo < KINDA_SMALL_NUMBER && Ctrl->StrengthTarget > KINDA_SMALL_NUMBER)
		{
			// Look at the animation and see how long it has left.
			FLOAT AnimTimeLeft = 0.f;
			UGearAnim_Slot* UpSlot = PawnOwner->BodyStanceNodes(BS_Std_Up);
			if(UpSlot)
			{
				UAnimNodeSequence* PressSeqNode = UpSlot->GetCustomAnimNodeSeq();
				AnimTimeLeft = PressSeqNode->GetTimeLeft();
			}

			Ctrl->BlendOutTime = AnimTimeLeft * 0.4f;
			Ctrl->SetSkelControlActive(FALSE);
		}
	}
}


/************************************************************************************
 * UGSM_Run2Cover
 ***********************************************************************************/

void UGSM_Run2Cover::TickSpecialMove(FLOAT DeltaTime)
{
	Super::TickSpecialMove(DeltaTime);

	if( bEnableJoystickMonitoring )
	{
		if( PCOwner )
		{
			// Remap controls so they remain relative to cover normal.
			FVector		CamLoc(0.0f);
			FRotator	CamRot(0,0,0);
			PCOwner->eventGetPlayerViewPoint(CamLoc, CamRot);
			PCOwner->ControlsRemapRotation = CamRot;

			const UBOOL bJoyRightDominant = Abs(PCOwner->RemappedJoyRight) > Abs(PCOwner->RemappedJoyUp);

			if( bJoyRightDominant && PCOwner->RemappedJoyRight > PCOwner->DeadZoneThreshold )
			{
				HoldingRight	+= DeltaTime;
				HoldingLeft		= 0.f;
				HoldingBack		= 0.f;

				if( HoldingRight > MinHoldTimeToSwitchMirroring )
				{
					eventJoystickHeldLongEnough(PCOwner);
				}
			}
			else if( bJoyRightDominant && PCOwner->RemappedJoyRight < -PCOwner->DeadZoneThreshold )
			{
				HoldingRight	= 0.f;
				HoldingLeft		+= DeltaTime;
				HoldingBack		= 0.f;

				if( HoldingLeft > MinHoldTimeToSwitchMirroring )
				{
					eventJoystickHeldLongEnough(PCOwner);
				}
			}
			else
			{
				HoldingRight	= 0.f;
				HoldingLeft		= 0.f;
				if (PCOwner->RemappedJoyUp < -PCOwner->DeadZoneThreshold)
				{
					HoldingBack += DeltaTime;
					if (HoldingBack >= PCOwner->BreakFromCoverHoldTime)
					{
						eventJoystickHeldLongEnough(PCOwner);
					}
				}
			}
		}
	}
}

static FVector LocalToWorld(FVector const& LocalVect, FRotator const& SystemRot)
{
	return FRotationMatrix(SystemRot).TransformNormal( LocalVect );
}

/************************************************************************************
 * UGSM_Engage
 ***********************************************************************************/

FVector UGSM_Engage::GetTargetPosition()
{
	if (PawnOwner != NULL && PawnOwner->EngageTrigger != NULL && PawnOwner->EngageTrigger->ENGAGE_Actor != NULL)
	{
		FVector RotatedOffset = PawnOwner->EngageTrigger->ENGAGE_OffsetFromActor;
		return PawnOwner->EngageTrigger->ENGAGE_Actor->Location + ::LocalToWorld( RotatedOffset, PawnOwner->EngageTrigger->ENGAGE_Actor->Rotation );
	}
	else
	{
		return PawnOwner->Location;
	}
}

void UGSM_Engage::TickSpecialMove(FLOAT DeltaTime)
{
	Super::TickSpecialMove(DeltaTime);

	// Turn towards wheel
	if( PawnOwner != NULL && PawnOwner->EngageTrigger != NULL && (PawnOwner->Role == ROLE_Authority || PawnOwner->IsLocallyControlled()) )
	{
		// move the pawn to the right spot
		if( PawnOwner->EngageTrigger->ENGAGE_Actor != NULL )
		{
			// Desired rotation: Facing valve.
			FRotator TargetRotation = PawnOwner->EngageTrigger->ENGAGE_Actor->Rotation;
			TargetRotation.Yaw -= 16384;
			TargetRotation.Pitch = 0;
			TargetRotation.Roll = 0;
			TargetRotation = TargetRotation.GetNormalized();

			// New rotation, smoothly interpolated.
			FRotator NewRotation = RInterpTo(PawnOwner->Rotation.GetNormalized(), TargetRotation.GetNormalized(), DeltaTime, PawnOwner->EngageTrigger->ENGAGE_TurnTowardSpeed);
			NewRotation = NewRotation.GetNormalized();

			// set the rotation
			if( PawnOwner->Rotation != NewRotation )
			{
				PawnOwner->SetRotation( NewRotation );
				PawnOwner->DesiredRotation = NewRotation;
			}

			// Desired position.
			const FVector TargetPosition = GetTargetPosition();

			// New position, smoothly interpolated.
			const FLOAT Distance = (TargetPosition - PawnOwner->Location).Size2D();

			if( Distance > 1.f )
			{
				const FLOAT		PushMagnitude	= Min( (Distance / DeltaTime), PawnOwner->GroundSpeed * PawnOwner->MaxSpeedModifier());
				const FVector Direction		= (TargetPosition - PawnOwner->Location).SafeNormal2D();

				//debugf(TEXT("dist %2.3f mag %2.3f"),Distance,PushMagnitude);
				PawnOwner->Velocity		= Direction * PushMagnitude;
				PawnOwner->Acceleration	= (PawnOwner->Velocity / DeltaTime).SafeNormal();
			}
			else
			{
				PawnOwner->Velocity		= FVector(0.f);
				PawnOwner->Acceleration	= FVector(0.f);
			}
		}
	}
}

void UGSM_EngageStart::TickSpecialMove(FLOAT DeltaTime)
{
	Super::TickSpecialMove(DeltaTime);

	// Turn towards wheel
	if( !bHasReachedDestination )
	{
		// move the pawn to the right spot
		if( PawnOwner != NULL && PawnOwner->EngageTrigger != NULL && PawnOwner->EngageTrigger->ENGAGE_Actor != NULL )
		{
			// Desired rotation: Facing valve.
			FRotator TargetRotation = PawnOwner->EngageTrigger->ENGAGE_Actor->Rotation;
			TargetRotation.Yaw -= 16384;
			TargetRotation.Pitch = 0;
			TargetRotation.Roll = 0;
			TargetRotation = TargetRotation.GetNormalized();

			// if we've reached our destination, send the event.
			if( FRotator::NormalizeAxis(abs(PawnOwner->Rotation.Yaw - TargetRotation.Yaw)) < 256 )
			{
				// check the target position as well
				if( (GetTargetPosition() - PawnOwner->Location).Size2D() <= 2.0f )
				{
					bHasReachedDestination = TRUE;
					eventStartGrabAnim();
				}
			}
		}
	}
}

void UGSM_Berserker_Smash::TickSpecialMove(FLOAT DeltaTime)
{
	Super::TickSpecialMove(DeltaTime);

	if( bDamageActive && PawnOwner && PawnOwner->Mesh )
	{
		// Trace along Right Hand
		FCheckResult Hit;
		FVector Extent(1.f);
		FVector SocketLocation;

		PawnOwner->Mesh->GetSocketWorldLocationAndRotation( PawnOwner->RightHandSocketName, SocketLocation, NULL );
		if( RHand_OldLocation != FVector(0,0,0) )
		{
			//debug
			if( bDebugLines )
				PawnOwner->DrawDebugLine(SocketLocation, RHand_OldLocation, 255, 0, 0, TRUE );

			FMemMark Mark(GMainThreadMemStack);
			/*
			UBOOL bHitSomething = !GWorld->SingleLineCheck( Hit, PawnOwner, SocketLocation, RHand_OldLocation, TRACE_ProjTargets, Extent );
			if( bHitSomething && Hit.Actor )
			{
				eventMeleeDamageTo( Hit.Actor, Hit.Location );
			} 
			*/
			FCheckResult *Hits = GWorld->MultiLineCheck(GMainThreadMemStack, SocketLocation, RHand_OldLocation, Extent, TRACE_ProjTargets, PawnOwner);
			for (FCheckResult* CheckHit = Hits; CheckHit != NULL; CheckHit = CheckHit->GetNext())
			{
				if (CheckHit->Actor != NULL)
				{
					eventMeleeDamageTo(CheckHit->Actor,CheckHit->Location);
				}
			}
			Mark.Pop();
		}
		RHand_OldLocation = SocketLocation;

		// Trace along Left Hand
		PawnOwner->Mesh->GetSocketWorldLocationAndRotation( PawnOwner->LeftHandSocketName, SocketLocation, NULL );
		if( LHand_OldLocation != FVector(0,0,0) )
		{
			//debug
			if( bDebugLines )
				PawnOwner->DrawDebugLine(SocketLocation, LHand_OldLocation, 0, 255, 0, TRUE );

			/*
			UBOOL bHitSomething = !GWorld->SingleLineCheck( Hit, PawnOwner, SocketLocation, LHand_OldLocation, TRACE_ProjTargets, Extent );
			if( bHitSomething && Hit.Actor )
			{
				eventMeleeDamageTo( Hit.Actor, Hit.Location );
			} 
			*/ 
			FMemMark Mark(GMainThreadMemStack);
			FCheckResult *Hits = GWorld->MultiLineCheck(GMainThreadMemStack, SocketLocation, LHand_OldLocation, Extent, TRACE_ProjTargets, PawnOwner);
			for (FCheckResult* CheckHit = Hits; CheckHit != NULL; CheckHit = CheckHit->GetNext())
			{
				if (CheckHit->Actor != NULL)
				{
					eventMeleeDamageTo(CheckHit->Actor,CheckHit->Location);
				}
			}
			Mark.Pop();
		}
		LHand_OldLocation = SocketLocation;
	}
}

/************************************************************************************
 * UGSM_Berserker_Charge
 ***********************************************************************************/

void UGSM_Berserker_Charge::TickSpecialMove(FLOAT DeltaTime)
{
	Super::TickSpecialMove(DeltaTime);

	if( bInterpolateRotation && PawnOwner )
	{
		FRotator NewRotation;
	
		if( RotationInterpolationTime > DeltaTime )
		{
			// Delta rotation
			const FRotator RotDelta	= (DesiredChargeRot.GetNormalized() - PawnOwner->Rotation.GetNormalized()).GetNormalized();

			NewRotation	= (PawnOwner->Rotation + RotDelta * (DeltaTime / RotationInterpolationTime)).GetNormalized();
			
			RotationInterpolationTime	-= DeltaTime;

			//debugf(TEXT("%3.2f, PawnRot: %s, DesiredRot: %s, DeltaRot: %s"), GWorld->GetTimeSeconds(), *Pawn->Rotation.ToString(), *DesiredChargeRot.ToString(), *RotDelta.ToString());
		}
		else
		{
			NewRotation				= DesiredChargeRot;
			bInterpolateRotation	= FALSE;
		}

		if( NewRotation != PawnOwner->Rotation )
		{
			// Rotate Pawn to new Rotation.
			FCheckResult Hit(1.f);
			GWorld->MoveActor(PawnOwner, FVector(0.f), NewRotation, 0, Hit);

			// Orient velocity to match actor's rotation.
			if( !PawnOwner->Velocity.IsNearlyZero() )
			{
				PawnOwner->Velocity = PawnOwner->Rotation.Vector() * PawnOwner->Velocity.Size();
			}

			// Set desired rotation, to prevent any interpolation dictated by controller.
			PawnOwner->DesiredRotation = PawnOwner->Rotation;
			if( PawnOwner->Controller )
			{
				PawnOwner->Controller->DesiredRotation = PawnOwner->Rotation;
				PawnOwner->Controller->SetRotation(PawnOwner->Rotation);
			}
		}
	}
}

/************************************************************************************
 * UGSM_RecoverFromRagdoll
 ***********************************************************************************/

void UGSM_RecoverFromRagdoll::TickSpecialMove(FLOAT DeltaTime)
{
	Super::TickSpecialMove(DeltaTime);

	if( bBlendToGetUp && PawnOwner )
	{
		// If we are in process of blending to get up animation, modify physics weight between ragdoll finish pose and get-up start pose.
		if( GetUpBlendStartTime + GetUpBlendTime > GWorld->GetTimeSeconds() )
		{
			const FLOAT Alpha = 1.f - ((GWorld->GetTimeSeconds() - GetUpBlendStartTime)/GetUpBlendTime);
			PawnOwner->Mesh->PhysicsWeight = Clamp<FLOAT>(CubicInterp<FLOAT>(0.f, 0.f, 1.f, 0.f, Alpha), 0.f, 1.f);
		}
		// Done with blend, start the get-up animation, start updating physics bones again, and show result.
		// Bones should all be fixed at this point.
		else
		{
			eventFinishedBlendToGetUp();
		}
	}
}


/************************************************************************************
 * UGSM_GrabWretch
 ***********************************************************************************/

// Turn on box fitting debugging
#define DEBUG_BOX_FITTING 1

/** 
 * builds position AABB from Follower location and rotation.
 * Uses Marker to place leader, and build an AABB from there.
 */
static void BuildAABBFromFollower(AGearPawn* Leader, AGearPawn* Follower, FBox& OutAABB, FRotator TestRotation, FVector& RelativeAlignOffset)
{
	// Follower AABB
	FVector FollowerExtent(Follower->CylinderComponent->CollisionRadius, Follower->CylinderComponent->CollisionRadius, Follower->CylinderComponent->CollisionHeight);
	FBox	FollowerAABB = FBox::BuildAABB(Follower->Location, FollowerExtent);
	
#if DEBUG_BOX_FITTING
	Follower->DrawDebugBox(FollowerAABB.GetCenter(), FollowerAABB.GetExtent(), 000, 255, 000, TRUE);
#endif

	// Marker AABB
	FRotationMatrix FollowerRotM(Follower->Rotation);
	FVector AlignOffset2D	= FollowerRotM.GetAxis(0) * RelativeAlignOffset.X + FollowerRotM.GetAxis(1) * RelativeAlignOffset.Y;
	FVector LeaderMarker	= Follower->Location - AlignOffset2D;
	FVector MarkerHalfLoc	= (Follower->Location + LeaderMarker) / 2.f;
	FVector	MarkerExtent	= (Follower->Location - LeaderMarker) / 2.f;
	FBox	MarkerAABB = FBox::BuildAABB(MarkerHalfLoc, MarkerExtent);

#if DEBUG_BOX_FITTING
	Follower->DrawDebugBox(MarkerAABB.GetCenter(), MarkerAABB.GetExtent(), 255, 000, 255, TRUE);
#endif

	// Leader AABB
	FVector LeaderExtent(Leader->CylinderComponent->CollisionRadius, Leader->CylinderComponent->CollisionRadius, Leader->CylinderComponent->CollisionHeight);
	FVector LeaderLoc = LeaderMarker + FVector(0, 0, LeaderExtent.Z - FollowerExtent.Z);
	FBox	LeaderAABB = FBox::BuildAABB(LeaderLoc, LeaderExtent);

#if DEBUG_BOX_FITTING
	Follower->DrawDebugBox(LeaderAABB.GetCenter(), LeaderAABB.GetExtent(), 000, 255, 000, TRUE);
#endif

	OutAABB.Init();
	OutAABB += FollowerAABB;
	OutAABB += MarkerAABB;
	OutAABB += LeaderAABB;

#if DEBUG_BOX_FITTING
	// Draw resulting AABB
	Follower->DrawDebugBox(OutAABB.GetCenter(), OutAABB.GetExtent(), 255, 000, 000, TRUE);

	// Draw resulting Marker
	FVector FollowerGroundLoc = Follower->Location + FVector(0,0,-FollowerExtent.Z);
	Follower->DrawDebugLine(FollowerGroundLoc, FollowerGroundLoc-AlignOffset2D, 255, 000, 000, TRUE);
	Follower->DrawDebugCoordinateSystem(FollowerGroundLoc, Follower->Rotation, 20.f, TRUE);
#endif
}


/** 
 * Utility function, turns rotation relative marker to world space.
 */
static FVector GetWorldMarker(const FMatrix& RotationMatrix, const FVector& RelativeMarker)
{
	return RotationMatrix.GetAxis(0) * RelativeMarker.X + RotationMatrix.GetAxis(1) * RelativeMarker.Y;
}


/** 
 * Utility function to turn a reference Marker (world loc and rotation), to match a given direction vector.
 * It then outputs the new WorldMarker aligned to the direction vector, and the new resulting rotation matrix derived from that.
 */
static void GetMarkerFromDir
(
	const FVector&	RefMarker, 
	const FMatrix&	RefRotMatrix, 
	const FVector&	DesiredDir, 
	FVector&		Out_NewWorldMarker, 
	FMatrix&		Out_NewRotMatrix
)
{
	// Make sure DesiredDir is normalized in 2D space
	FVector SafeDir2D = DesiredDir.SafeNormal2D();

	// Find delta angle between our reference and target.
	FQuat DeltaQuat = FQuatFindBetween(RefMarker.SafeNormal2D(), SafeDir2D);

	// Resulting Rotation
	Out_NewRotMatrix = FQuatRotationTranslationMatrix(DeltaQuat * FQuat(RefRotMatrix), FVector(0));

	Out_NewWorldMarker = SafeDir2D * RefMarker.Size2D();
}


/** 
 */
void UGSM_GrabWretch::PrePositioning_MRLeader_RFollower(AGearPawn* Leader, AGearPawn* Follower, FVector& out_LeaderLoc, FVector& out_FollowerLoc, FRotator& out_FacingRot)
{
	// The tough part is moving the 2 actors in the right sport, turn adjustments are trivial.
	// So first let's try to minimize movement, that is keeping the same alignment direction between both actors
	// and just moving them closer or further from each other. We derive facing direction from there.

	// Reference Rotation matrix
	FRotationMatrix RefRotMatrix(Leader->Rotation);
	// Ref marker
	FVector RefMarker = GetWorldMarker(RefRotMatrix, AlignmentOffset);

	// Delta between both actors.
	FVector Delta = Follower->Location - Leader->Location;

	// Get Marker aligned to vector between both Leader and Follower.
	FMatrix	TestRotMatrix;
	FVector	TestMarker;
	GetMarkerFromDir(RefMarker, RefRotMatrix, Delta, TestMarker, TestRotMatrix);

	// Both need to face this direction.
	out_FacingRot	= TestRotMatrix.Rotator();
	// Leader is offset from Follower 
	out_LeaderLoc	= Follower->Location - TestMarker;
	// Follower doesn't move
	out_FollowerLoc	= FVector(0.f);

#if 0
	// Debug transformation
	
	debugf(TEXT("LeaderLoc: %s, FollowerLoc: %s, FacingRot: %s"), *out_LeaderLoc.ToString(), *out_FollowerLoc.ToString(), *out_FacingRot.ToString());
	
	// Draw line connecting both actors.
	Leader->DrawDebugLine(Leader->Location, Follower->Location, 000, 000, 255, TRUE);

	// This is our reference marker, based from the leader's rotation.
	Leader->DrawDebugLine(Leader->Location, Leader->Location + RefMarker, 000, 255, 000, TRUE);
	Leader->DrawDebugCoordinateSystem(Leader->Location + RefMarker, Leader->Rotation, 20.f, TRUE);

	// This is the adjusted marker based of the new rotation.
	Leader->DrawDebugLine(Leader->Location, Leader->Location + TestMarker, 255, 000, 000, TRUE);
	Leader->DrawDebugCoordinateSystem(Leader->Location + TestMarker, TestRotMatrix.Rotator(), 20.f, TRUE);
#endif

	//FBox TestAABB;
	//BuildAABBFromFollower(Leader, Follower, TestAABB, FacingRotation, AlignmentOffset);

}

FLOAT UGSM_RaiseShieldOverHead::GetSpeedModifier()
{
	FLOAT Ret = SpeedModifier;
	if (PawnOwner)
	{
		// still carrying shield, incorporate the shield speed modifier values as well
		Ret *= PawnOwner->eventIsInAimingPose() ? PawnOwner->ShieldFiringMovementSpeedPercentage : PawnOwner->ShieldMovementSpeedPercentage;
	}
	return Ret;
}


/************************************************************************************
* UGSM_Brumak_MeleeAttack
***********************************************************************************/

void UGSM_Brumak_MeleeAttack::TickSpecialMove(float DeltaTime)
{
	Super::TickSpecialMove(DeltaTime);

	if( bDamageActive && PawnOwner && PawnOwner->Mesh && GWorld->GetWorldInfo()->NetMode != NM_Client )
	{
		// Trace along Right Hand
		FVector Extent(125.f);
		FVector SocketLocation;

		//debug
		if( bDebugLines )
		{
			PawnOwner->FlushPersistentDebugLines();
		}

		PawnOwner->Mesh->GetSocketWorldLocationAndRotation(PawnOwner->RightHandSocketName, SocketLocation, NULL);
		if( !RHand_OldLocation.IsZero() )
		{
			FVector EndTrace = SocketLocation + (SocketLocation - RHand_OldLocation).SafeNormal() * 64.f;
			//debug
			if( bDebugLines )
			{
				PawnOwner->DrawDebugLine(EndTrace, RHand_OldLocation, 255, 0, 0, TRUE );
				PawnOwner->DrawDebugBox( RHand_OldLocation, Extent, 0, 255, 0, TRUE );
				PawnOwner->DrawDebugBox( EndTrace, Extent, 255, 0, 0, TRUE );
			}

			FMemMark Mark(GMainThreadMemStack);
			FCheckResult *Hits = GWorld->MultiLineCheck(GMainThreadMemStack, EndTrace, RHand_OldLocation, Extent, TRACE_AllBlocking, PawnOwner );//TRACE_ProjTargets|TRACE_ComplexCollision, PawnOwner);
			for (FCheckResult* CheckHit = Hits; CheckHit != NULL; CheckHit = CheckHit->GetNext())
			{
				if (CheckHit->Actor != NULL)
				{
					eventMeleeDamageTo(CheckHit->Actor,CheckHit->Location);
				}
			}
			Mark.Pop();
		}
		RHand_OldLocation = SocketLocation;

		// Trace along Left Hand
		PawnOwner->Mesh->GetSocketWorldLocationAndRotation( PawnOwner->LeftHandSocketName, SocketLocation, NULL );
		if( !LHand_OldLocation.IsZero() )
		{
			FVector EndTrace = SocketLocation + (SocketLocation - LHand_OldLocation).SafeNormal() * 64.f;
			//debug
			if( bDebugLines )
			{
				PawnOwner->DrawDebugLine(EndTrace, LHand_OldLocation, 0, 255, 0, TRUE );
				PawnOwner->DrawDebugBox( LHand_OldLocation, Extent, 0, 255, 0, TRUE );
				PawnOwner->DrawDebugBox( EndTrace, Extent, 255, 0, 0, TRUE );
			}

			FMemMark Mark(GMainThreadMemStack);
			FCheckResult *Hits = GWorld->MultiLineCheck(GMainThreadMemStack, EndTrace, LHand_OldLocation, Extent, TRACE_AllBlocking, PawnOwner );//TRACE_ProjTargets|TRACE_ComplexCollision, PawnOwner);
			for (FCheckResult* CheckHit = Hits; CheckHit != NULL; CheckHit = CheckHit->GetNext())
			{
				if (CheckHit->Actor != NULL)
				{
					eventMeleeDamageTo(CheckHit->Actor,CheckHit->Location);
				}
			}
			Mark.Pop();
		}
		LHand_OldLocation = SocketLocation;
	}
}


/************************************************************************************
* UGSM_Brumak_OverlayBite
***********************************************************************************/

void UGSM_Brumak_OverlayBite::TickSpecialMove(float DeltaTime)
{
	Super::TickSpecialMove(DeltaTime);

	if( bDamageActive && PawnOwner && PawnOwner->Mesh && GWorld->GetWorldInfo()->NetMode != NM_Client )
	{
		// Trace along Right Hand
		FCheckResult Hit;
		FVector Extent(75.f);
		FVector SocketLocation;

		//debug
		if( bDebugLines )
		{
			PawnOwner->FlushPersistentDebugLines();
		}

		PawnOwner->Mesh->GetSocketWorldLocationAndRotation( JawSocketName, SocketLocation, NULL );
		if( !Mouth_OldLocation.IsZero() )
		{
			//debug
			if( bDebugLines )
			{
				PawnOwner->DrawDebugLine(SocketLocation, Mouth_OldLocation, 255, 0, 0, TRUE );
				PawnOwner->DrawDebugBox( Mouth_OldLocation, Extent, 0, 255, 0, TRUE );
				PawnOwner->DrawDebugBox( SocketLocation, Extent, 255, 0, 0, TRUE );
			}

			UBOOL bHitSomething = !GWorld->SingleLineCheck( Hit, PawnOwner, SocketLocation, Mouth_OldLocation, TRACE_ProjTargets, Extent );
			if( bHitSomething && Hit.Actor )
			{
				eventMeleeDamageTo( Hit.Actor, Hit.Location );
			}
		}
		Mouth_OldLocation = SocketLocation;
	}
}

/************************************************************************************
 * UGSM_DBNO
 ***********************************************************************************/

#define SHOW_CheckForClipping_DEBUG 0

void UGSM_DBNO::CheckForClipping()
{
	// Do 45 degrees increment tests
	const INT	TestRotationIncrement = 4096;
	// How many units to test
	const FLOAT	TestDistance = 192.f;
	FRotator FinalRotation = PawnOwner->Rotation;

	FCheckResult Hit;
	// Our number of interations to do a full circle
	for(INT Count=0; Count<(32768/TestRotationIncrement); Count++)
	{
		FRotator TestRotation = FRotator(0, PawnOwner->Rotation.Yaw + TestRotationIncrement*Count, 0);
		FVector TestOffset = TestRotation.Vector() * TestDistance;
		
		// If we don't hit something then, we're good!
		if( GWorld->SingleLineCheck(Hit, PawnOwner, PawnOwner->Location + TestOffset, PawnOwner->Location, TRACE_World|TRACE_StopAtAnyHit, PawnOwner->GetCylinderExtent() * 0.5f) )
		{
#if SHOW_CheckForClipping_DEBUG
			PawnOwner->DrawDebugLine(PawnOwner->Location, PawnOwner->Location + TestOffset, 0, 255, 0, TRUE);
#endif
			FinalRotation = TestRotation;
			break;
		}
#if SHOW_CheckForClipping_DEBUG
		else
		{
			PawnOwner->DrawDebugLine(PawnOwner->Location, PawnOwner->Location + TestOffset, 255, 0, 0, TRUE);
		}
#endif
		// Test left
		if( Count > 0 )
		{
			FRotator TestRotation = FRotator(0, PawnOwner->Rotation.Yaw - TestRotationIncrement*Count, 0);
			TestOffset = TestRotation.Vector() * TestDistance;
			
			// If we don't hit something then, we're good!
			if( GWorld->SingleLineCheck(Hit, PawnOwner, PawnOwner->Location + TestOffset, PawnOwner->Location, TRACE_World|TRACE_StopAtAnyHit, PawnOwner->GetCylinderExtent() * 0.5f) )
			{
#if SHOW_CheckForClipping_DEBUG
				PawnOwner->DrawDebugLine(PawnOwner->Location, PawnOwner->Location + TestOffset, 0, 255, 0, TRUE);
#endif
				FinalRotation = TestRotation;
				break;
			}
#if SHOW_CheckForClipping_DEBUG
			else
			{
				PawnOwner->DrawDebugLine(PawnOwner->Location, PawnOwner->Location + TestOffset, 255, 0, 0, TRUE);
			}
#endif
		}
	}

	if( FinalRotation != PawnOwner->Rotation )
	{
		SetFacePreciseRotation(FinalRotation, 0.33f);
	}
}