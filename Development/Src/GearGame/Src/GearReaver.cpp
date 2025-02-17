/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
#include "GearGame.h"
#include "UnNet.h"

#include "GearGameVehicleClasses.h"
#include "GearGameAnimClasses.h"


IMPLEMENT_CLASS(AVehicle_Reaver_Base);
IMPLEMENT_CLASS(AVehicle_RideReaver_Base);
IMPLEMENT_CLASS(AReaverLandingPoint);
IMPLEMENT_CLASS(UGearAnimNotify_ReaverStep);

extern FVector CalcAngularVelocity(FRotator const& OldRot, FRotator const& NewRot, FLOAT DeltaTime);

/** Util to calculate position of foot at a point in the step. */
FVector FReaverLegInfo::CalcFootPos(FLOAT Alpha) const
{
	const FVector StartTangent = FVector(0,0,ZTangent);
	const FVector EndTangent = FVector(0,0,-ZTangent) + (EndSlope * (StepEndPosition - StepStartPosition));


	const FVector CurrentFootPos = CubicInterp(StepStartPosition, StartTangent, StepEndPosition, EndTangent, Alpha);
	return CurrentFootPos;
}

/** Util for getting current InterpGroup used by this Reaver. */
UInterpGroup* AVehicle_Reaver_Base::GetCurrentInterpGroup()
{
	// First, get the InterpData we want to be using currently.
	UInterpData* CurrentIData = NULL;

	if(CurrentFlightIndex > 0)
	{
		INT UseIndex = CurrentFlightIndex - 1;
		check(UseIndex < 16);
		CurrentIData = FlightPaths[UseIndex];
	}
	else
	{
		CurrentIData = InitialFlightPath;
	}

	// Ok - we have some data - currently just return first group (TODO: use a name specifier?)
	if(CurrentIData && CurrentIData->InterpGroups.Num() > 0)
	{
		return CurrentIData->InterpGroups(0);
	}

	return NULL;
}

/** Moves Reaver along a matinee path. */
void AVehicle_Reaver_Base::ReaverFlying(FLOAT DeltaTime)
{
	UInterpGroup* Group = GetCurrentInterpGroup();
	UInterpTrackMove* MoveTrack = NULL;
	UInterpData* IData = NULL;
	if(Group)
	{
		// Get the current InterpData as well.
		IData = CastChecked<UInterpData>(Group->GetOuter());

		// Iterate over tracks
		for(INT TrIndex=0; TrIndex<Group->InterpTracks.Num(); TrIndex++)
		{
			// See if its a movement track
			UInterpTrackMove* TestMove = Cast<UInterpTrackMove>(Group->InterpTracks(TrIndex));
			if(TestMove)
			{
				MoveTrack = TestMove;
				break;
			}
		}
	}

	// Ok - we have a movement track
	if(MoveTrack)
	{
		// Update current position.
		CurrentInterpTime += DeltaTime;

		// Get new position from track
		FVector NewPos = Location;
		FRotator NewRot = Rotation;
		MoveTrack->GetKeyTransformAtTime(NULL, CurrentInterpTime, NewPos, NewRot);

		FVector OldLocation = Location;
		FRotator OldRotation = Rotation;

		// set velocity for our desired move (so any events that get triggered, such as encroachment, know where we were trying to go)
		if(DeltaTime > KINDA_SMALL_NUMBER)
		{
			Velocity = (NewPos - Location) / DeltaTime;
			AngularVelocity = CalcAngularVelocity(OldRotation, NewRot, DeltaTime);
		}
		else
		{
			Velocity = FVector(0.f);
			AngularVelocity = FVector(0.f);
		}

		// Do actual move
		FCheckResult Hit(1.f);
		GWorld->MoveActor(this, NewPos - Location, NewRot, 0, Hit);

		// set velocity for the movement that actually happened
		if(DeltaTime > KINDA_SMALL_NUMBER)
		{
			Velocity = (Location - OldLocation) / DeltaTime;
		}
	}

	// On server, see if we are past end of the movement - and if so, pick a new one.
	if((Role == ROLE_Authority) && IData && (CurrentInterpTime > IData->InterpLength) && !bDeleteMe)
	{
		TArray<INT> FlightIndices;
		for(INT i=0; i<16; i++)
		{
			if(FlightPaths[i])
			{
				FlightIndices.AddItem(i);
			}
		}

		// If flying off with both legs removed - destroy it now
		if( bLeavingDefeated || bDestroyOnNextPassEnd || FlightIndices.Num() == 0 )
		{
			eventReaverFlownOff();
		}
		else
		{
			// Pick randomly from valid paths
			CurrentFlightIndex = 1 + FlightIndices( appRand() % FlightIndices.Num() );

			// Increment flight count to replicate to client.
			FlightCount++;
			bNetDirty = TRUE;
			//debugf(TEXT("INCREMENT FLIGHT: %d"), FlightCount);

			// reset position to start
			CurrentInterpTime = 0.f;

			// We certainly are not on the first flight any more!
			bHasPlayedInitialFlight = TRUE;

			// Call script event to indicate starting a new pass
			eventReaverPassEnded();
		}
	}
}

/** Moves Reaver along spline to its landing point. */
void AVehicle_Reaver_Base::ReaverTransitioning(FLOAT DeltaTime)
{
	// Update transition progress
	if(TransitionInfo.bLanding || bCurrentlyTakingOff) 
	{
		CurrentTransitionTime += DeltaTime;
	}

	FLOAT TransitionTime = TransitionInfo.bTakingOff ? TakeOffTime : LandingTime;
	FLOAT Alpha = Clamp(CurrentTransitionTime/TransitionTime, 0.f, 1.f);

#if !FINAL_RELEASE
	// DEBUG DRAWING
	// Draw step trajectory
	if(bDrawTransitionDebug)
	{
		for(INT i=0; i<20; i++)
		{
			FLOAT StartA = FLOAT(i)/21.f;
			FVector StartPos = CubicInterp(TransitionStartPosition, TransitionStartTangent, TransitionInfo.EndPosition, TransitionInfo.EndTangent, StartA);

			FLOAT EndA = FLOAT(i+1)/21.f;
			FVector EndPos = CubicInterp(TransitionStartPosition, TransitionStartTangent, TransitionInfo.EndPosition, TransitionInfo.EndTangent, EndA);

			GWorld->LineBatcher->DrawLine(StartPos, EndPos, FColor(0,255,255), SDPG_World);
		}
	}
	// END DEBUG DRAWING
#endif

	// Inerpolate to find new position
	FVector NewPos = CubicInterp(TransitionStartPosition, TransitionStartTangent, TransitionInfo.EndPosition, TransitionInfo.EndTangent, Alpha);

#if 0
	// Euler interp
	FVector NewRotEuler = CubicInterp(TransitionStartRotation.Euler(), FVector(0,0,0), TransitionInfo.EndRotation.Euler(), FVector(0,0,0), Alpha);
	FRotator NewRot = FRotator::MakeFromEuler(NewRotEuler);
#elif 0
	// Quat interp
	FQuat NewQuat = QuatSlerp( FQuat(FRotationMatrix(TransitionStartRotation)), FQuat(FRotationMatrix(TransitionInfo.EndRotation)), Alpha );
	FRotator NewRot = FQuatRotationTranslationMatrix( NewQuat, FVector(0.f) ).Rotator();
#else
	// Jump to end
	FRotator NewRot = TransitionInfo.EndRotation;
#endif

	// Update position
	FVector OldLocation = Location;
	FRotator OldRotation = Rotation;

	// set velocity for our desired move (so any events that get triggered, such as encroachment, know where we were trying to go)
	if(DeltaTime > KINDA_SMALL_NUMBER)
	{
		Velocity = (NewPos - Location) / DeltaTime;
		AngularVelocity = CalcAngularVelocity(OldRotation, NewRot, DeltaTime);
	}
	else
	{
		Velocity = FVector(0.f);
		AngularVelocity = FVector(0.f);
	}

	// Do actual move
	FCheckResult Hit(1.f);
	GWorld->MoveActor(this, NewPos - Location, NewRot, 0, Hit);

	// set velocity for the movement that actually happened
	Velocity = (Location - OldLocation) / DeltaTime;


	// If Alpha is 1.0, then we have finished landing, call script.
	if((Role == ROLE_Authority) && Abs(Alpha - 1.f) < KINDA_SMALL_NUMBER)
	{
		if(TransitionInfo.bTakingOff)
		{
			eventServerFinishedTakingOff();
		}
		else
		{
			eventServerFinishedLanding();
		}
	}
}

/** Special case interpolation for reavers following matinee paths and landing. */
void AVehicle_Reaver_Base::physInterpolating(FLOAT DeltaTime)
{
	if(TransitionInfo.bLanding || TransitionInfo.bTakingOff)
	{
		ReaverTransitioning(DeltaTime);
	}
	else
	{
		ReaverFlying(DeltaTime);
	}
}

/** Looks to see if reaver can currently land (is in valid section of flight path). */
UBOOL AVehicle_Reaver_Base::CanLand()
{
	const FName LandOnName = FName(TEXT("LAND_ON"));
	const FName LandOffName = FName(TEXT("LAND_OFF"));

	// Get current interp group
	UInterpGroup* Group = GetCurrentInterpGroup();
	if(!Group)
	{
		return FALSE;
	}

	// Iterate over tracks finding LAND_ event before current position
	FLOAT NearestLandTime = BIG_NUMBER;
	UBOOL bIsLandOnEvent = FALSE;
	UBOOL bFoundALandingKey = FALSE;

	for(INT TrIndex=0; TrIndex<Group->InterpTracks.Num(); TrIndex++)
	{
		// See if its a movement track
		UInterpTrackEvent* EventTrack = Cast<UInterpTrackEvent>(Group->InterpTracks(TrIndex));
		if(EventTrack)
		{
			// Iterate over each key
			for(INT i=0; i<EventTrack->EventTrack.Num(); i++)
			{
				const FEventTrackKey& Key = EventTrack->EventTrack(i);

				// See if this is a LAND_ON or LAND_OFF event
				UBOOL bIsLandKey = ((Key.EventName == LandOnName) || (Key.EventName == LandOffName));
				bFoundALandingKey |= bIsLandKey;

				// If past current position - discard
				if(Key.Time > CurrentInterpTime)
				{
					break;
				}

				// See if this is the cloeset before current time.
				if(bIsLandKey)
				{
					FLOAT LandTime = (CurrentInterpTime - Key.Time);
					if(LandTime < NearestLandTime)
					{
						NearestLandTime = LandTime;
						bIsLandOnEvent = (Key.EventName == LandOnName);
					}
				}
			}
		}
	}

	// If we found no LAND_ events, or the nearest event before current time is LAND_ON, allow the landing
	if(!bFoundALandingKey || bIsLandOnEvent)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void AVehicle_Reaver_Base::CalcDefaultLegPositions()
{
	if(WalkAnimNode && WalkAnimNode->AnimSeq)
	{
		// Force animation to first frame.
		FName OldAnimName = WalkAnimNode->AnimSeqName;
		WalkAnimNode->SetAnim(FName(TEXT("Idle_Stand")));
		WalkAnimNode->SetPosition(0.f, FALSE);

		INT NumBones = Mesh->SkeletalMesh->RefSkeleton.Num();
		// Want all bones - add all to array
		TArray<BYTE> RequiredBones;
		RequiredBones.Add(NumBones);
		for(INT i=0; i<NumBones; i++)
		{
			RequiredBones(i) = i;
		}

		// Get anim info (local space)
		FBoneAtom RootMotionDelta;
		INT bHasRootMotion;

		FMemMark Mark(GMainThreadMemStack);

		FBoneAtomArray Atoms;
		Atoms.Add(NumBones);
		WalkAnimNode->GetAnimationPose(WalkAnimNode->AnimSeq, WalkAnimNode->AnimLinkupIndex, Atoms, RequiredBones, RootMotionDelta, bHasRootMotion);

		// We build the mesh-space matrices of the source bones.
		TArray<FMatrix> BoneTM;
		BoneTM.Add(Mesh->SkeletalMesh->RefSkeleton.Num());

		for(INT BoneIndex=0; BoneIndex<NumBones; BoneIndex++)
		{	
			if( BoneIndex == 0 )
			{
				//Atoms(BoneIndex).ToTransform(BoneTM(0));
				BoneTM(0) = FMatrix::Identity;
			}
			else
			{
				FMatrix LocalBoneTM;
				Atoms(BoneIndex).ToTransform(LocalBoneTM);

				const INT ParentIndex	= Mesh->SkeletalMesh->RefSkeleton(BoneIndex).ParentIndex;
				BoneTM(BoneIndex)		= LocalBoneTM * BoneTM(ParentIndex);
			}
		}

		// For each leg...
		for(INT LegIdx=0; LegIdx<6; LegIdx++)
		{
			FReaverLegInfo& Leg = LegInfo[LegIdx];
			// ..get the socket at the tip..
			USkeletalMeshSocket* Socket = Mesh->SkeletalMesh->FindSocket(Leg.TipSocketName);
			if(Socket)
			{
				// .. and calculate where it it, in component space, now.
				INT BoneIndex = Mesh->MatchRefBone(Socket->BoneName);
				if(BoneIndex != INDEX_NONE)
				{
					const FMatrix BoneMatrix = BoneTM(BoneIndex);
					FRotationTranslationMatrix SocketMatrix(Socket->RelativeRotation, Socket->RelativeLocation);
					FMatrix ComponentSocketMatrix = SocketMatrix * BoneTM(BoneIndex);
					Leg.DefaultLegPos = ComponentSocketMatrix.GetOrigin();
					debugf(TEXT("-- %d = %s"), LegIdx, *Leg.DefaultLegPos.ToString());
				}
			}
		}

		WalkAnimNode->SetAnim(OldAnimName);

		Mark.Pop();
	}
}

void AVehicle_Reaver_Base::InitLegs()
{
	for(INT LegIdx=0; LegIdx<6; LegIdx++)
	{
		FReaverLegInfo& Leg = LegInfo[LegIdx];
		FVector TipLocation;
		Mesh->GetSocketWorldLocationAndRotation(Leg.TipSocketName, TipLocation, NULL);

		FVector DesiredFootLocation = TipLocation;

		FCheckResult Hit(1.f);
		const FVector CheckStart = TipLocation + FVector(0,0,100);
		const FVector CheckEnd = TipLocation - FVector(0,0,200);
		UBOOL bHit = !GWorld->SingleLineCheck(Hit, this, CheckEnd, CheckStart, TRACE_World);
		if(bHit)
		{
			DesiredFootLocation = Hit.Location;
		}
		else
		{
			DesiredFootLocation = CheckEnd;
		}

		if(Leg.IKControl)
		{
			Leg.IKControl->SetSkelControlActive(TRUE);
			Leg.IKControl->EffectorLocation = DesiredFootLocation;
		}
	}
}

void AVehicle_Reaver_Base::TakeStep(INT FootIndex, FLOAT RandomOffset, FLOAT StepTime, FLOAT ZTangentScale)
{
	// Ignore stepping if foot raised for attack.
	if((FootIndex == 2 && AttackInfo.bLLegAttacking) || (FootIndex == 5 && AttackInfo.bRLegAttacking))
	{
		return;
	}

	check((FootIndex < 6) && (FootIndex >= 0));
	FReaverLegInfo& Leg = LegInfo[FootIndex];
	// Project forwards by current velocity in XY plane.
	FVector XYVel = Velocity;
	// Clamp to max AirSpeed to avoid weirdness
	if(XYVel.Size() > AirSpeed)
	{
		XYVel.SafeNormal() * AirSpeed;
	}
	XYVel.Z = 0.f;

	FVector WorldDefaultLegPos = Mesh->LocalToWorld.TransformFVector(Leg.DefaultLegPos);
	FVector StepEndPos = WorldDefaultLegPos + (StepAdvanceFactor * XYVel * WalkAnimNode->GetAnimPlaybackLength()) + (AngularVelocity ^ (WorldDefaultLegPos - Location));

	// Add additional random offset
	if(RandomOffset > 0.f)
	{
		FVector Offset = VRand();
		Offset.Z = 0.f;
		Offset = Offset.SafeNormal();
		StepEndPos += (Offset * RandomOffset);
	}

	// Line check to match world height at that point
	FCheckResult Hit(1.f);
	const FVector CheckStart = StepEndPos + FVector(0,0,100);
	const FVector CheckEnd = StepEndPos - FVector(0,0,200);
	UBOOL bHit = !GWorld->SingleLineCheck(Hit, this, CheckEnd, CheckStart, TRACE_World);
	if(bHit)
	{
		StepEndPos = Hit.Location;
	}

	MoveFootToPos(FootIndex, StepEndPos, StepTime, StepZTangent, StepEndSlope);
}

void AVehicle_Reaver_Base::MoveFootToPos(INT FootIndex, FVector EndPos, FLOAT Time, FLOAT InZTangent, FLOAT InEndSlope)
{
	if((FootIndex < 6) && (FootIndex >= 0))
	{
		FReaverLegInfo& Leg = LegInfo[FootIndex];
		if(Leg.IKControl)
		{
			Leg.StepStartPosition = Leg.IKControl->EffectorLocation;
			Leg.StepEndPosition = EndPos;

			Leg.CurrentStepTime = 0.f;
			Leg.TotalStepTime = Time;
			Leg.bStepping = TRUE;
			Leg.ZTangent = InZTangent;
			Leg.EndSlope = InEndSlope;
		}
	}
}

UBOOL AVehicle_Reaver_Base::FindNearestPathAndTime(INT& OutFlightIndex,FLOAT& OutInterpTime, FVector& OutFlightPos, FRotator& OutFlightRot, FVector& OutFlightVel)
{
	OutFlightIndex = INDEX_NONE;
	OutInterpTime = 0.f;

	UBOOL bFoundCorrectDir = FALSE;

	FVector ForwardDir = LocalToWorld().GetAxis(0);

	FLOAT ClosestDistSq = BIG_NUMBER;
	for(INT FlightIndex=0; FlightIndex<16; FlightIndex++)
	{
		UInterpData* IData = FlightPaths[FlightIndex];
		if(IData && IData->InterpGroups.Num() > 0)
		{
			UInterpGroup* Group = IData->InterpGroups(0);
			check(Group);
			UInterpTrackMove* MoveTrack = NULL;
			// Iterate over tracks
			for(INT TrIndex=0; TrIndex<Group->InterpTracks.Num(); TrIndex++)
			{
				// See if its a movement track
				UInterpTrackMove* TestMove = Cast<UInterpTrackMove>(Group->InterpTracks(TrIndex));
				if(TestMove)
				{
					MoveTrack = TestMove;
					break;
				}
			}

			// See if we have a movement track
			if(MoveTrack)
			{
				FLOAT TestTime = 0.f;
				while(TestTime < IData->InterpLength)
				{
					FVector NewPos(0,0,0);
					FRotator NewRot(0,0,0);
					MoveTrack->GetKeyTransformAtTime(NULL, TestTime, NewPos, NewRot);

					// See if this direction will have us pointing
					FRotationMatrix NewRotTM(NewRot);
					FVector NewDir = NewRotTM.GetAxis(0);
					UBOOL bCorrectDir = (ForwardDir | NewDir) > 0.2f;

					FLOAT TestDistSq = (NewPos - Location).SizeSquared();
					// If this is the closest so far, and either its going the right way, or we haven't found 
					if((!bFoundCorrectDir && bCorrectDir) || ((TestDistSq < ClosestDistSq) && (!bFoundCorrectDir || bCorrectDir)))
					{
						OutFlightIndex = FlightIndex;
						OutInterpTime = TestTime;
						OutFlightPos = NewPos;
						OutFlightRot = NewRot;

						// Estimate velocity based on position 1/10th sec later.
						FVector NextPos(0,0,0);
						FRotator NextRot(0,0,0);
						MoveTrack->GetKeyTransformAtTime(NULL, TestTime + 0.1f, NextPos, NextRot);					
						OutFlightVel = (NextPos - NewPos)/0.1f;

						ClosestDistSq = TestDistSq;

						// Remeber if we have found a point going the right way
						if(bCorrectDir)
						{
							bFoundCorrectDir = TRUE;
						}
					}

					// Search every quarter of a second of the path.
					TestTime += 0.25f;
				}
			}
		}
	}

#if !FINAL_RELEASE
	if(!bFoundCorrectDir)
	{
		FString Msg = TEXT("REAVER: COULD NOT FIND PATH POINTING CORRECT WAY");
		debugf(*Msg);
		GWorld->GetWorldInfo()->AddOnScreenDebugMessage(INT(this), 3.f, FColor(0,255,255), Msg);
	}
#endif

	if(OutFlightIndex != INDEX_NONE)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

UBOOL AVehicle_Reaver_Base::ShouldTrace(UPrimitiveComponent* Primitive, AActor* SourceActor, DWORD TraceFlags)
{
	// don't allow gunner's traces or projectiles to hit us
	if (Gunner != NULL && SourceActor != NULL)
	{
		if (SourceActor == Gunner || (SourceActor->Instigator == Gunner && SourceActor->GetAProjectile() != NULL))
		{
			return FALSE;
		}
		else
		{
			return Super::ShouldTrace(Primitive, SourceActor, TraceFlags);
		}
	}
	else
	{
		return Super::ShouldTrace(Primitive, SourceActor, TraceFlags);
	}
}

void AVehicle_Reaver_Base::TickSpecial(FLOAT DeltaSeconds)
{
	Super::TickSpecial(DeltaSeconds);

	// If we are walking...
	if(Physics == PHYS_RigidBody && Health > 0)
	{
		// Find positions of local players
		TArray<FVector>	LocalPlayerPawnLocations;
		for (INT Idx = 0; Idx < GEngine->GamePlayers.Num(); Idx++) 
		{
			if (GEngine->GamePlayers(Idx) != NULL &&
				GEngine->GamePlayers(Idx)->Actor != NULL &&
				GEngine->GamePlayers(Idx)->Actor->Pawn != NULL)
			{
				LocalPlayerPawnLocations.AddItem(GEngine->GamePlayers(Idx)->Actor->Pawn->Location);
			}
		}

		// For each leg
		for(INT LegIdx=0; LegIdx<6; LegIdx++)
		{
			FReaverLegInfo& Leg = LegInfo[LegIdx];
			// Check we have a leg IK controller
			if(!Leg.IKControl || Leg.bCutOff)
			{
				continue;
			}

			// If taking a step - interpolate IK target
			if(Leg.bStepping)
			{
				// Update leg time
				Leg.CurrentStepTime += DeltaSeconds;

				// Work out new position of foot
				FLOAT Alpha = Clamp(Leg.CurrentStepTime/Leg.TotalStepTime, 0.f, 1.f);
				FVector CurrentFootPos = Leg.CalcFootPos(Alpha);
				Leg.IKControl->EffectorLocation = CurrentFootPos;

#if !FINAL_RELEASE
				// DEBUG DRAWING
				// Draw step trajectory
				if(bDrawStepDebug)
				{
					for(INT i=0; i<20; i++)
					{
						FLOAT StartA = FLOAT(i)/21.f;
						FVector StartPos = Leg.CalcFootPos(StartA);

						FLOAT EndA = FLOAT(i+1)/21.f;
						FVector EndPos = Leg.CalcFootPos(EndA);

						GWorld->LineBatcher->DrawLine(StartPos, EndPos, FColor(0,255,255), SDPG_World);
					}
				}
				// END DEBUG DRAWING
#endif
				// Stop the step when time is up
				if(Leg.CurrentStepTime >= Leg.TotalStepTime)
				{
					Leg.bStepping = FALSE;

					// step finished!
					eventNotifyStepFinished(LegIdx);
				}
			}
			// Not taking a step - see if we need to take an 'emergency' step
			// This happen if stretched too long, or too close to player
			else
			{
				FVector TipLocation;
				Mesh->GetSocketWorldLocationAndRotation(Leg.TipSocketName, TipLocation, NULL);
				UBOOL bForceTakeStep = FALSE;
				FLOAT RandomOffset = 0.f;
				FLOAT StepTime = 0.33f;
				FLOAT TangentZScale = 1.f;
				for(INT PIndex=0; PIndex<LocalPlayerPawnLocations.Num(); PIndex++)
				{
					if((LocalPlayerPawnLocations(PIndex) - TipLocation).SizeSquared2D() < (LegForceStepPlayerDist*LegForceStepPlayerDist))
					{
						//debugf(TEXT("PLAYER CLOSE! %d %f"), LegIdx, (LocalPlayerPawnLocations(PIndex) - TipLocation).Size2D());
						bForceTakeStep = TRUE;
						RandomOffset = LegForceStepPlayerRand;
						StepTime = 0.66f;
						TangentZScale = 3.f;
						break;
					}
				}

				if(!bForceTakeStep)
				{
					const FVector LegDesiredPosition = Mesh->LocalToWorld.TransformFVector(Leg.DefaultLegPos);
					if((LegDesiredPosition - TipLocation).SizeSquared() > (LegForceStepErrorDist*LegForceStepErrorDist))
					{
						//GWorld->LineBatcher->DrawLine(LegDesiredPosition, TipLocation, FColor(255,0,0), SDPG_World);
						//debugf(TEXT("LEG STRETCH! %d %f"), LegIdx, (LegDesiredPosition - TipLocation).Size());
						bForceTakeStep = TRUE;
					}
				}

				if(bForceTakeStep)
				{
					TakeStep(LegIdx, RandomOffset, StepTime, TangentZScale);
				}
			}

#if !FINAL_RELEASE
			// DEBUG DRAWING
			// Draw current control target
			if(bDrawStepDebug && Leg.IKControl)
			{
				// Draw current leg control target
				
				DrawWireStar(GWorld->LineBatcher, Leg.IKControl->EffectorLocation, 10.f, FColor(0,255,0), SDPG_World);
				// Draw default position
				DrawWireStar(GWorld->LineBatcher, Mesh->LocalToWorld.TransformFVector(Leg.DefaultLegPos), 10.f, FColor(255,0,0), SDPG_World);
			}
			// END DEBUG DRAWING
#endif
		}
	}
	else if(Physics == PHYS_Interpolating)
	{
		FVector UseAngVel = AngularVelocity;
		// Use base angular velocity if there is some
		if(Base && !Base->AngularVelocity.IsNearlyZero())
		{
			UseAngVel += Base->AngularVelocity;
		}

		// Project angvel to find component around local z and y vectors
		FMatrix L2W = LocalToWorld();
		FLOAT YawVel = L2W.GetAxis(2) | UseAngVel;
		FLOAT PitchVel = L2W.GetAxis(1) | UseAngVel;

		// Limit how quickly it can change
		FLOAT MaxDelta = DeltaSeconds * AngVelAimOffsetChangeSpeed;

		FLOAT TargetYawOffset = Clamp(YawVel * AngVelAimOffsetScale, -1.f, 1.f);
		FLOAT Delta = Clamp(TargetYawOffset - VehicleAimOffset.X, -MaxDelta, MaxDelta);
		VehicleAimOffset.X += Delta;

		FLOAT TargetPitchOffset = Clamp(PitchVel * AngVelAimOffsetScale, -1.f, 1.f);
		Delta = Clamp(TargetPitchOffset - VehicleAimOffset.Y, -MaxDelta, MaxDelta);
		VehicleAimOffset.Y += Delta;

		//debugf(TEXT("R ANGVEL: %s AIM: %f %f"), *UseAngVel.ToString(), VehicleAimOffset.X, VehicleAimOffset.Y);
	}

	// When gored - force lighting bounds to be around actor location
	if(bIsGore)
	{
		LightEnvironment->bOverrideOwnerBounds = TRUE;
		LightEnvironment->OverriddenBounds = FBoxSphereBounds(Location, FVector(100.f,100.f,100.f), 100.f);
	}
	else
	{
		LightEnvironment->bOverrideOwnerBounds = FALSE;
	}

	// If in ragdoll 
	if( Physics == PHYS_RigidBody && 
		bPlayedDeath &&
		WorldInfo->TimeSeconds - LastRenderTime < 1.0f &&
		GWorld &&
		Mesh->PhysicsAsset &&
		Mesh->PhysicsAssetInstance )
	{
		const FLOAT GibThreshSqr = GibEffectsSpeedThreshold * GibEffectsSpeedThreshold;

		URB_BodyInstance* BodyInst;

		// and gibbed up - look to see if some parts are moving and we should fire some gib effects, otherwise let's do fall effects (see 'else')
		if(bIsGore)
		{
			// Set of bones to kill as they are too far away 
			TArray<INT> BonesToKill;
			const FLOAT GibKillDistSqr = GibKillDistance*GibKillDistance;

			// Iterate over bodies in gore to see if any are moving.
			for(INT i=0;i<Mesh->PhysicsAsset->BodySetup.Num();++i)
			{
				BodyInst = (Mesh->PhysicsAssetInstance->Bodies(i));
				if(BodyInst && BodyInst->IsValidBodyInstance())
				{
					// See how far this gib has moved from the actor location
					const FMatrix TM = BodyInst->GetUnrealWorldTM();
					if((TM.GetOrigin() - Location).SizeSquared() > GibKillDistSqr)
					{
						// If too far - mark it for killing
						URB_BodySetup* BS = Mesh->PhysicsAsset->BodySetup(i);
						INT BoneIndex = Mesh->MatchRefBone(BS->BoneName);
						if(BoneIndex != INDEX_NONE)
						{
							BonesToKill.AddUniqueItem(BoneIndex);
						}
						// No need to do more stuff
						continue;
					}

					// Look for changes in velocity
					const FVector Speed = BodyInst->GetUnrealWorldVelocity();
					if(BodyInst->Velocity != Speed)
					{
						BodyInst->PreviousVelocity = BodyInst->Velocity;
						BodyInst->Velocity = Speed;
					}

					const FLOAT SpeedSq = (BodyInst->Velocity - BodyInst->PreviousVelocity).SizeSquared();
					// If vel change is big enough..
					if(  SpeedSq > GibThreshSqr )
					{
						const FMatrix Transform = BodyInst->GetUnrealWorldTM();
						// .. and it has been long enough since we last fired an effect..
						if( (BodyInst->LastEffectPlayedTime + TimeBetweenGibEffects) < GWorld->GetTimeSeconds() )
						{
							// ..call script event.
							eventPlayGibEffect(Transform.GetOrigin(),FVector(0.0f,0.0f,0.0f), SpeedSq);
							BodyInst->LastEffectPlayedTime = GWorld->GetTimeSeconds();
						}
					} 
				}
			}

			// Kill bones that need it
			for(INT i=0; i<BonesToKill.Num(); i++)
			{
				Mesh->HideBone(BonesToKill(i), TRUE);
			}
		}
	}

}

void AVehicle_Reaver_Base::CheckForErrors()
{
	Super::CheckForErrors();

	GWarn->MapCheck_Add( MCTYPE_ERROR, this, *FString::Printf(TEXT("%s : Should not place Vehicle_Reavers directly into map - use AI FActory instead."), *GetName() ), MCACTION_DELETE, TEXT("") );
}

static inline UBOOL NEQ(const FReaverTransitionInfo& A, const FReaverTransitionInfo& B, UPackageMap* Map, UActorChannel* Channel)
{
	return	((A.bLanding != B.bLanding) || (A.bTakingOff != B.bTakingOff) || 
			 (A.EndPosition != B.EndPosition) || (A.EndTangent != B.EndTangent) || (A.EndRotation != B.EndRotation));
}

static inline UBOOL NEQ(const FReaverLegAttackInfo& A, const FReaverLegAttackInfo& B, UPackageMap* Map, UActorChannel* Channel)
{
	return	((A.bLLegAttacking != B.bLLegAttacking) || (A.bRLegAttacking != B.bRLegAttacking) || (A.LegAttackTarget != B.LegAttackTarget));
}

static inline UBOOL NEQ(const FReaverPlayAnimInfo& A, const FReaverPlayAnimInfo& B, UPackageMap* Map, UActorChannel* Channel)
{
	return	((A.bNewData != B.bNewData) || (A.AnimName != B.AnimName));
}

/** Native rep function to ensure Physics gets replicated even though bUpdateSimulatedPosition is FALSE. */
INT* AVehicle_Reaver_Base::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	Ptr = Super::GetOptimizedRepList(Recent, Retire, Ptr, Map, Channel);

	if(bNetDirty)
	{
		if(bNetInitial)
		{
			DOREP(Vehicle_Reaver_Base, CurrentInterpTime);
			DOREP(Vehicle_Reaver_Base, bWalking);
			((AVehicle_Reaver_Base*)Recent)->FlightCount = FlightCount;
			DOREP(Vehicle_Reaver_Base, Gunner);
		}
		else
		{
			DOREP(Vehicle_Reaver_Base, FlightCount);
		}

		DOREPARRAY(Vehicle_Reaver_Base, FlightPaths);
		DOREP(Vehicle_Reaver_Base, InitialFlightPath);
		DOREP(Vehicle_Reaver_Base, CurrentFlightIndex);
		DOREP(Vehicle_Reaver_Base, RestartMainBoneAnimCount);
		DOREP(Vehicle_Reaver_Base, AnimRepInfo);
		DOREP(Vehicle_Reaver_Base, TransitionInfo);
		DOREP(Vehicle_Reaver_Base, TakeOffTargetInterpTime);
		DOREP(Vehicle_Reaver_Base, AttackInfo);

		DOREP(Actor, Physics);
	}

	return Ptr;
}


/** Called when foot goes up or down. */
void UGearAnimNotify_ReaverStep::Notify( class UAnimNodeSequence* NodeSeq )
{
	if(StepAction == ERST_FootUp)
	{
		USkeletalMeshComponent* SkelComp = NodeSeq->SkelComponent;
		if(SkelComp)
		{
			AVehicle_Reaver_Base* Reaver = Cast<AVehicle_Reaver_Base>(SkelComp->GetOwner());
			if(Reaver)
			{
				Reaver->TakeStep(LegIndex, 0.f, StepTime, 1.f);
			}
		}
	}
}


void AVehicle_RideReaver_Base::TickSpecial(FLOAT DeltaSeconds)
{
	Super::TickSpecial(DeltaSeconds);

	// Update dodge offset CurrentDodgeAmount towards TargetDodge.
	FVector OldDodgeAmount = CurrentDodgeAmount;
	CurrentDodgeAmount = VInterpTo(CurrentDodgeAmount, TargetDodge, DeltaSeconds, DodgeSpeed);
	// Update dodge velocity
	CurrentDodgeVel = (CurrentDodgeAmount - OldDodgeAmount)/DeltaSeconds;

	// If we want to turn off spotlight, check here
	if(bSpotlightsDisabled)
	{
		if(FrontSpotlightComp->bEnabled)
		{
			FrontSpotlightComp->SetEnabled(FALSE);
		}

		if(RearSpotlightComp->bEnabled)
		{
			RearSpotlightComp->SetEnabled(FALSE);
		}
	}
	// We want spotlight on - see which seat we are in
	else
	{
		// Front seat - front light on, rear off
		if(SeatMask & (1 << 0))
		{
			if(!FrontSpotlightComp->bEnabled)
			{
				FrontSpotlightComp->SetEnabled(TRUE);
			}

			if(RearSpotlightComp->bEnabled)
			{
				RearSpotlightComp->SetEnabled(FALSE);
			}
		}
		// Rear seat - rear light on, front off
		else
		{
			if(FrontSpotlightComp->bEnabled)
			{
				FrontSpotlightComp->SetEnabled(FALSE);
			}

			if(!RearSpotlightComp->bEnabled)
			{
				RearSpotlightComp->SetEnabled(TRUE);
			}
		}
	}
}

static FVector GetAverageVel(FVector* History, INT NumElements)
{
	FVector Total(0,0,0);
	for(INT i=0; i<NumElements; i++)
	{
		Total += History[i];
	}
	return Total/(FLOAT(NumElements));
}

void AVehicle_RideReaver_Base::AdjustInterpTrackMove(FVector& Pos, FRotator& Rot, FLOAT DeltaTime)
{
	LastOrigInterpLocation = Pos;

	FMatrix L2W = LocalToWorld();

	FVector LocalDodgeOffset(0.f, CurrentDodgeAmount.X, CurrentDodgeAmount.Y);
	FVector WorldDodgeOffset = L2W.TransformNormal(LocalDodgeOffset);
	Pos += WorldDodgeOffset;

	// Add some rotation based on dodging velocity

	// We smooth out the dodge velocity
	FVector LocalDodgeVel(0.f, CurrentDodgeVel.X, CurrentDodgeVel.Y);

	DodgeRotVelHistory[DodgeRotVelSlot] = LocalDodgeVel;
	if(++DodgeRotVelSlot >= 10)
	{
		DodgeRotVelSlot = 0;
	}

	FVector DodgeRotUseVel = GetAverageVel(DodgeRotVelHistory, 10);

	// Cross to get unit axis
	FVector LocalRotAxis = DodgeRotUseVel ^ FVector(1,0,0);
	LocalRotAxis = LocalRotAxis.SafeNormal();

	// Get dodge vel mag
	FLOAT DodgeVelMag = DodgeRotUseVel.Size();

	// Calc desired rotation, and move towards it as fast as allowed
	FLOAT TargetRotAngle = DodgeVelMag * DodgeRotationAmount;

	// Then build quat and apply
	FQuat DodgeRotQuat(LocalRotAxis, TargetRotAngle);
	FQuat RotQuat = FQuat( FRotationMatrix(Rot) );
	FQuat FinalQuat = RotQuat * DodgeRotQuat;

	// Convert back to rotator
	Rot = FQuatRotationTranslationMatrix( FinalQuat, FVector(0.f) ).Rotator();
}

