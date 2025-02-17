/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
#include "UTGame.h"
#include "EngineAnimClasses.h"
#include "UTGameAnimationClasses.h"
#include "UTGameVehicleClasses.h"

IMPLEMENT_CLASS(UUTSkelControl_MantaBlade);
IMPLEMENT_CLASS(UUTSkelControl_MantaFlaps);
IMPLEMENT_CLASS(UUTSkelControl_HugGround);
IMPLEMENT_CLASS(UUTSkelControl_LockRotation);
IMPLEMENT_CLASS(UUTSkelControl_Oscillate);
IMPLEMENT_CLASS(UUTSkelControl_Damage);
IMPLEMENT_CLASS(UUTSkelControl_DamageHinge);
IMPLEMENT_CLASS(UUTSkelControl_DamageSpring);
IMPLEMENT_CLASS(UUTSkelControl_CantileverBeam);
IMPLEMENT_CLASS(UUTSkelControl_MassBoneScaling);
IMPLEMENT_CLASS(UUTSkelControl_CicadaEngine);
IMPLEMENT_CLASS(UUTSkelControl_JetThruster);
IMPLEMENT_CLASS(UUTSkelControl_Rotate);
IMPLEMENT_CLASS(UUTSkelControl_LookAt);
IMPLEMENT_CLASS(UUTSkelControl_HoverboardSuspension);
IMPLEMENT_CLASS(UUTSkelControl_HoverboardSwing);
IMPLEMENT_CLASS(UUTSkelControl_HoverboardVibration);
IMPLEMENT_CLASS(UUTSkelControl_SpinControl);

static INT RotationalTurn(INT Current, INT Desired, FLOAT DeltaRatePerSecond)
{
	const INT DeltaRate = appTrunc( DeltaRatePerSecond );

	if (DeltaRate == 0)
		return (Current & 65535);

	INT result = Current & 65535;
	Current = result;
	Desired = Desired & 65535;

	if (Current > Desired)
	{
		if (Current - Desired < 32768)
			result -= Min((Current - Desired), Abs(DeltaRate));
		else
			result += Min((Desired + 65536 - Current), Abs(DeltaRate));
	}
	else
	{
		if (Desired - Current < 32768)
			result += Min((Desired - Current), Abs(DeltaRate));
		else
			result -= Min((Current + 65536 - Desired), Abs(DeltaRate));
	}
	return (result & 65535);
}

/**********************************************************************************
 * UTSkelControl_MantaBlade
 *
 * Spins the manta blades
 **********************************************************************************/

void UUTSkelControl_MantaBlade::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	AUTVehicle* OwnerVehicle = Cast<AUTVehicle>(SkelComp->GetOwner());
	if (OwnerVehicle && OwnerVehicle->bDriving)
		DesiredRotationsPerSecond=MaxRotationsPerSecond;
	else
		DesiredRotationsPerSecond=0;

	if (SpinUpTime>0)
	{
		if ( RotationsPerSecond < DesiredRotationsPerSecond )
		{
			RotationsPerSecond += MaxRotationsPerSecond * SpinUpTime * DeltaSeconds;
			if (RotationsPerSecond>DesiredRotationsPerSecond)
				RotationsPerSecond = DesiredRotationsPerSecond;
		}
		else if (RotationsPerSecond > DesiredRotationsPerSecond)
		{
			RotationsPerSecond -= MaxRotationsPerSecond * SpinUpTime * DeltaSeconds;
			if (RotationsPerSecond<DesiredRotationsPerSecond)
				RotationsPerSecond = DesiredRotationsPerSecond;
		}
	}
	else
		RotationsPerSecond = DesiredRotationsPerSecond;

	if (bCounterClockwise)
		DeltaSeconds *= -1;

	if (RotationsPerSecond > 0)
	{
		BoneRotation.Yaw = BoneRotation.Yaw + INT(RotationsPerSecond * 65536 * DeltaSeconds);
	}

	Super::TickSkelControl(DeltaSeconds, SkelComp);
}

/**********************************************************************************
 * UTSkelControl_MantaFlags
 **********************************************************************************/


void UUTSkelControl_MantaFlaps::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	AUTVehicle* OwnerVehicle = Cast<AUTVehicle>(SkelComp->GetOwner());
	if (OwnerVehicle && OwnerVehicle->bDriving)
	{

		const FLOAT ZForce = Abs(OwnerVehicle->Velocity.Z);
		FLOAT NewPitch = 0.0f;

		// Get body angular velocity
		FRigidBodyState rbState;
		OwnerVehicle->GetCurrentRBState(rbState);


		if ( ZForce > 100.f )
		{
			NewPitch += maxPitch * (ZForce - 100.f) / 900.f * 182.044f;
			if (OwnerVehicle->Velocity.Z > 0.f)
			{
				NewPitch *= -1;
			}

			// smooth change
			if ( Abs(NewPitch - OldZPitch) > MaxPitchChange * DeltaSeconds )
			{
				NewPitch = (NewPitch > OldZPitch) ? OldZPitch + MaxPitchChange*DeltaSeconds : OldZPitch - MaxPitchChange*DeltaSeconds;
			}
		}
		else
		{
			// let pitch reduce smoothly over time
			NewPitch = OldZPitch * (1.f - MaxPitchTime * DeltaSeconds);
		}
		OldZPitch = NewPitch;

		if (Abs(rbState.AngVel.Z) > 100.0f)
		{
			if (ControlName == RightFlapControl)
				NewPitch += -maxPitch * (rbState.AngVel.Z / 4000.0f) * 182.044f;			

			if (ControlName == LeftFlapControl)
				NewPitch += maxPitch * (rbState.AngVel.Z / 4000.0f) * 182.044f;		
		}

		BoneRotation.Pitch = Clamp<INT>(appTrunc(NewPitch), appTrunc(-maxPitch * 182.044f), appTrunc(maxPitch * 182.044f));
	}
	else
		BoneRotation.Pitch = 0;

	Super::TickSkelControl(DeltaSeconds, SkelComp);
}

/************************************************************************************
 * UTSkelControl_HugGround
 ************************************************************************************/

void UUTSkelControl_HugGround::CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms)
{
	checkSlow(SkelComp != NULL);
	checkSlow(SkelComp->SkeletalMesh != NULL);

	AActor* Owner = SkelComp->GetOwner();
	if (Owner != NULL && (SkelComp->FramesPhysicsAsleep < 30))
	{
		const FVector& OldBoneTranslation = BoneTranslation;

		// calculate the trace start (the bone's reference location) and the trace end (MaxDist units in the Actor's negative Z axis) in world space
		FMatrix ActorToWorld = Owner->LocalToWorld();
		ActorToWorld.RemoveScaling();

		FMatrix CompToWorld = SkelComp->LocalToWorld;
		CompToWorld.RemoveScaling();

		const FVector& BoneLocation = CompToWorld.TransformFVector(SkelComp->SpaceBases(BoneIndex).GetOrigin());
		const FVector& TraceStart = BoneLocation + ActorToWorld.TransformNormal(FVector(0.f, 0.f, MaxDist));
		const FVector& TraceEnd = BoneLocation - ActorToWorld.TransformNormal(FVector(0.f, 0.f, MaxDist + DesiredGroundDist));
	
		FLOAT CurrentGroundDist = 1000.f; 
		FCheckResult Hit(1.0f);		
		if (!GWorld->SingleLineCheck(Hit, Owner, TraceEnd, TraceStart, TRACE_World ))
		{
			CurrentGroundDist = (Hit.Time * ((2.f * MaxDist) + DesiredGroundDist)) - MaxDist;
		}

		BoneTranslation = Clamp(CurrentGroundDist - DesiredGroundDist, -MaxDist, MaxDist) * FVector(0,0,-1);

		if (ParentBone != NAME_None)
		{
			INT ParentBoneIndex = SkelComp->MatchRefBone(ParentBone);
			if (ParentBoneIndex == INDEX_NONE)
			{
				debugf(NAME_Warning, TEXT("Invalid ParentBone for %s on mesh %s - setting to 'None'"), *GetName(), *SkelComp->SkeletalMesh->GetName());
				ParentBone = NAME_None;
			}
			else
			{
				const FMatrix& ParentMatrix = SkelComp->GetBoneMatrix(ParentBoneIndex);
				// rotate the bone towards the parent bone
				bApplyRotation = TRUE;
				FVector Dir = ParentMatrix.GetOrigin() - (BoneLocation + BoneTranslation);
				// move the translation along the rotation to stay the desired distance from the parent bone
				if (XYDistFromParentBone > 0.f)
				{
					const FVector DirNoZ(Dir.X, Dir.Y, 0.f);
					const FLOAT Dist = DirNoZ.Size();
					if (Dist > XYDistFromParentBone)
					{
						FVector Change = DirNoZ.UnsafeNormal() * (Dist - XYDistFromParentBone);
						BoneTranslation += Change;
						Dir -= Change;
					}
				}

				// pull back the parent location half our distance along the opposite direction of its rotation
				Dir -= ParentMatrix.Rotator().Vector() * (Dir.Size() * 0.5);

				if (ZDistFromParentBone > 0.f && Dir.Z > ZDistFromParentBone)
				{
					BoneTranslation.Z += Dir.Z - ZDistFromParentBone;
				}

				if (bOppositeFromParent)
				{
					Dir = (BoneLocation + BoneTranslation) - ParentMatrix.GetOrigin();
				}
				else
				{
					Dir = ParentMatrix.GetOrigin() - (BoneLocation + BoneTranslation);
				}
				BoneRotation = SkelComp->SpaceBases(BoneIndex).TransformNormal(Dir).Rotation();
				BoneRotation.Yaw = 0;
			}
		}
		else
		{
			bApplyRotation = FALSE;
		}

		// cap BoneTranslation change, if necessary
		if (MaxTranslationPerSec > 0.0f)
		{
			const FLOAT MaxChange = (Owner->WorldInfo->TimeSeconds - LastUpdateTime) * MaxTranslationPerSec;
			const FVector& Diff = BoneTranslation - OldBoneTranslation;
			if (Diff.Size() > MaxChange)
			{
				BoneTranslation = OldBoneTranslation + (Diff.SafeNormal() * MaxChange);
			}
			LastUpdateTime = Owner->WorldInfo->TimeSeconds;
		}
	}

	Super::CalculateNewBoneTransforms(BoneIndex, SkelComp, OutBoneTransforms);
}

/************************************************************************************
 * UTSkelControl_LockRotation
 ************************************************************************************/

void UUTSkelControl_LockRotation::GetAffectedBones(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<INT>& OutBoneIndices)
{
	checkSlow(OutBoneIndices.Num() == 0);
	OutBoneIndices.AddItem(BoneIndex);
}

void UUTSkelControl_LockRotation::CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms)
{
	checkSlow(SkelComp != NULL);

	AActor* Owner = SkelComp->GetOwner();
	if (Owner != NULL)
	{
		// SpaceBases are in component space - so we need to calculate the BoneRotationSpace -> Component transform
		FMatrix ComponentToFrame = SkelComp->CalcComponentToFrameMatrix(BoneIndex, LockRotationSpace, RotationSpaceBoneName);
		ComponentToFrame.RemoveScaling();
		ComponentToFrame.SetOrigin(FVector(0.f));
		
		// get the desired rotation
		FRotator DesiredRotation = (SkelComp->SpaceBases(BoneIndex) * ComponentToFrame).Rotator();
		if (bLockPitch)
		{
			DesiredRotation.Pitch = Owner->fixedTurn(DesiredRotation.Pitch, LockRotation.Pitch, MaxDelta.Pitch);
		}
		if (bLockYaw)
		{
			DesiredRotation.Yaw = Owner->fixedTurn(DesiredRotation.Yaw, LockRotation.Yaw, MaxDelta.Yaw);
		}
		if (bLockRoll)
		{
			DesiredRotation.Roll = Owner->fixedTurn(DesiredRotation.Roll, LockRotation.Roll, MaxDelta.Roll);
		}

		// calculate the transform of the bone to get to the desired rotation
		FRotationMatrix RotInFrame(DesiredRotation);
		FMatrix FrameToComponent = ComponentToFrame.InverseSafe();
		FMatrix RotInComp = RotInFrame * FrameToComponent;
		RotInComp.SetOrigin(SkelComp->SpaceBases(BoneIndex).GetOrigin());

		OutBoneTransforms.AddItem(RotInComp);
	}
}
/************************************************************************************
 * UTSkelControl_Oscillate
 *
 * Controls the treads of the Necris Tank
 ************************************************************************************/

void UUTSkelControl_Oscillate::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	if (GWorld->HasBegunPlay())
	{
		if (bReverseDirection)
		{
			CurrentTime -= DeltaSeconds;
			if (CurrentTime <= -Period)
			{
				CurrentTime = -Period - (CurrentTime + Period);
				bReverseDirection = FALSE;
			}
		}
		else
		{
			CurrentTime += DeltaSeconds;
			if (CurrentTime >= Period)
			{
				CurrentTime = Period - (CurrentTime - Period);
				bReverseDirection = TRUE;
			}
		}
	}

	BoneTranslation = MaxDelta * (CurrentTime / Period);

	Super::TickSkelControl(DeltaSeconds, SkelComp);
}
/************************************************************************************
 * UTSkelControl_Damage
 *
 * The root of the skel_controls for simulating vehicle damage
 ************************************************************************************/

/**
 * Precache links to the UTVehicle as well as any associated DamageMorphTarget
 *
 * @param	SkelComp	The SkeletalMeshComponent that owns this SkelControl
 * @returns TRUE if we have successfully initialized this control
 */

UBOOL UUTSkelControl_Damage::InitializeControl(USkeletalMeshComponent* SkelComp)
{
	if ( !bInitialized )
	{
		if ( SkelComp && SkelComp->GetOwner() )
		{
			OwnerVehicle = Cast<AUTVehicle>( SkelComp->GetOwner() );
		}

		return (OwnerVehicle != NULL);
	}
	return TRUE;
}

/**
 * The root SkelControl_Damage is responsible for controlling it's ControlStrength by querying the 
 * vehicle (and/or the vehicle's DamageMorphTarget) to check to see if it needs to activate.
 *
 * @See USkelControlBase::TickSkelControl for params
 */
void UUTSkelControl_Damage::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	Super::TickSkelControl(DeltaSeconds, SkelComp);

	if ( !bInitialized )
	{
		bInitialized = InitializeControl(SkelComp);
	}

	if ( bInitialized && OwnerVehicle )
	{

		// Get the health value to test against.  If we have a linked damage morph target, then use the current
		// health of that target, otherwise use the main vehicle health
		if (HealthPerc >= 1.0)
		{
			bIsBreaking = FALSE;
			bIsBroken = FALSE;
		}

		if ( !bIsBroken )
		{
			// Check to see if we meet that activation threshold.  If we do and it's appropriate, 
			// set the control strength to be a factor of the remaining health, otherwise come on strong.
			if ( HealthPerc < ActivationThreshold )
			{
				ControlStrength = 1 - HealthPerc;
			}
			else
			{
				// Turn it off
				ControlStrength = 0.0;
			}
		}
		else if (bIsBroken || bIsBreaking)
		{
			ControlStrength = 1.0;
		}
	}
}

/**
 * Check to see if this control should break 
 *
 * @See USkelControlBase::CalculateNewbonetransforms for params
 */

void UUTSkelControl_Damage::CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms)
{
	if (bInitialized && ControlStrength >= BreakThreshold)
	{
		if (!bIsBroken && !bIsBreaking )
		{
			const FLOAT TargetPct = (ControlStrength - BreakThreshold) / (1.0 - BreakThreshold);
			if ( appFrand() < TargetPct )
			{
				BreakTimer = GWorld->GetTimeSeconds() + BreakTime;
				bIsBreaking = TRUE;
			}
		}

		if ( bIsBreaking )
		{
			if (GWorld->GetTimeSeconds() >= BreakTimer)
			{
				if( HealthPerc >= 0.0f )
				{
					UBOOL bIsVisible = FALSE;
					AActor* OwnerActor = SkelComp->GetOwner();
					if ( OwnerActor && (OwnerActor->WorldInfo->NetMode != NM_DedicatedServer) )
					{
						for( INT iPlayers=0; iPlayers<GEngine->GamePlayers.Num(); iPlayers++ )
						{
							if ( GEngine->GamePlayers(iPlayers) && GEngine->GamePlayers(iPlayers)->Actor )
							{
								APlayerController* PC = GEngine->GamePlayers(iPlayers)->Actor;
								ULocalPlayer* MyLocalPlayer = PC ? Cast<ULocalPlayer>(PC->Player) : NULL;
								if ( MyLocalPlayer && MyLocalPlayer->GetActorVisibility(OwnerActor) )
								{
									bIsVisible = TRUE;
									break;
								}
							}
						}
					}
					eventBreakApart( SkelComp->GetBoneMatrix(BoneIndex).GetOrigin(), bIsVisible );
				}
			}
		}
	}

	Super::CalculateNewBoneTransforms(BoneIndex, SkelComp, OutBoneTransforms);
}

/**
 * Looks to see if this control is broken and returns the bone scale if it is 
 */
FLOAT UUTSkelControl_Damage::GetBoneScale(INT BoneIndex, USkeletalMeshComponent* SkelComp)
{
	if( GIsEditor && !GIsGame )
	{
		return BoneScale;
	}
	else
	{
		// depending on if the vehicle is dead
		if( HealthPerc == 0.0f )
		{
			return DeathBoneScale;
		}
		// damaged
		else if( bIsBroken == TRUE )
		{
			return DamageBoneScale;
		}
		// alive
		else
		{
			return BoneScale;
		}
	}
}

/************************************************************************************
 * UTSkelControl_DamageHinge
 *
 * Used for Hoods and such
 ************************************************************************************/

/**
 * Each frame, look at the owning vehicle and adjust the hinge velocity.  
 *
 * @See USkelControlBase::TickSkelControl for params
 */

void UUTSkelControl_DamageHinge::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	Super::TickSkelControl(DeltaSeconds, SkelComp);

	if ( !bIsBroken && bInitialized )
	{
		FLOAT AngVel;

		// Get the angular velocity given the set pivot axis.  We will use this to adjust the angle of the hinge

		switch (PivotAxis)
		{
			case AXIS_X: 
				AngVel = OwnerVehicle->VState.RBState.AngVel.X;
				break;

			case AXIS_Z: 
				AngVel = OwnerVehicle->VState.RBState.AngVel.Z;
				break;

			default: 
				AngVel = OwnerVehicle->VState.RBState.AngVel.Y;
				break;

		}

		AngVel *= AVModifier;

		// FIXME: Should I make this configurable?
		if (AngVel > -2.0 && AngVel < 2.0)
		{
			AngVel = 0;
		}

		// Adjust the Hinge

		const FLOAT Force = SpringStiffness * CurrentAngle;
		const FLOAT MaxUAngle = MaxAngle * 182.0444;
		CurrentAngle += AngVel + Force * 0.95; 
		CurrentAngle = Clamp<FLOAT>(CurrentAngle,0, MaxUAngle );
		BoneRotation.Pitch = appTrunc( CurrentAngle );
	}
}

/************************************************************************************
 * UTSkelControl_DamageSpring
 *
 * Used for fenders/etc
 ************************************************************************************/

INT UUTSkelControl_DamageSpring::CalcAxis(INT &InAngle, FLOAT CurVelocity, FLOAT MinUAngle, FLOAT MaxUAngle)
{
	FLOAT CurAngle = FLOAT(InAngle);
	
	if (CurVelocity > -2.0 && CurVelocity < 2.0)
	{
		CurVelocity = 0.0;
	}

	CurAngle += ( CurVelocity * AVModifier );

	const FLOAT Force = SpringStiffness * CurAngle;  

	CurAngle = (CurAngle + Force) * Falloff; 
	CurAngle = Clamp<FLOAT>(CurAngle, MinUAngle, MaxUAngle );
	InAngle = appTrunc(CurAngle);
	return InAngle;
	
}

/**
 * @See USkelControlBase::TickSkelControl for params
 */

void UUTSkelControl_DamageSpring::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	Super::TickSkelControl(DeltaSeconds, SkelComp);

	if ( !bIsBroken )
	{
		if ( bInitialized )
		{
			FVector AngVel = OwnerVehicle->VState.RBState.AngVel;
			BoneRotation.Pitch = CalcAxis(CurrentAngle.Pitch, AngVel.Y, FLOAT(MinAngle.Pitch), FLOAT(MaxAngle.Pitch) );
			BoneRotation.Roll =  CalcAxis(CurrentAngle.Roll,  AngVel.X, FLOAT(MinAngle.Roll),  FLOAT(MaxAngle.Roll) );
			BoneRotation.Yaw =   CalcAxis(CurrentAngle.Yaw,   AngVel.Z, FLOAT(MinAngle.Yaw),   FLOAT(MaxAngle.Yaw) );
		}
	}
}

/**
 * Give each control a slightly random AVModifer +/- 10% 
 */

UBOOL UUTSkelControl_DamageSpring::InitializeControl(USkeletalMeshComponent* SkelComp)
{
	const UBOOL Res = Super::InitializeControl(SkelComp);
	if (Res)
	{
		AVModifier += ( (AVModifier * 0.2) * appFrand() ) - (AVModifier * 0.1);
	}

	return Res;
}

void UUTSkelControl_CantileverBeam::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	Super::TickSkelControl(DeltaSeconds, SkelComp);

	WorldSpaceGoal = (SkelComp->LocalToWorld.GetOrigin())+InitialWorldSpaceGoalOffset;
	FVector DistFromGoal = TargetLocation - WorldSpaceGoal;

	FVector Force = (DistFromGoal *-SpringStiffness);
	Force -= (SpringDamping * Velocity);
	// apply force to Velocity:
	Velocity += (Force*DeltaSeconds);
	TargetLocation += (Velocity*DeltaSeconds);
}


void UUTSkelControl_MassBoneScaling::SetBoneScale(FName BoneName, FLOAT Scale)
{
	USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(GetOuter()->GetOuter());
	if (SkelComp != NULL)
	{
		const INT BoneIndex = SkelComp->MatchRefBone(BoneName);
		if (BoneIndex == INDEX_NONE)
		{
			debugf(NAME_Warning, TEXT("Failed to find bone named %s in mesh %s"), *BoneName.ToString(), *SkelComp->SkeletalMesh->GetName());
		}
		else
		{
			const INT NumToAdd = BoneIndex - (BoneScales.Num() - 1);
			if (NumToAdd > 0)
			{
				BoneScales.Add(NumToAdd);
				for (INT i = 0; i < NumToAdd; i++)
				{
					BoneScales(BoneScales.Num() - i - 1) = 1.0f;
				}
			}
			BoneScales(BoneIndex) = Scale;
		}
	}
}

FLOAT UUTSkelControl_MassBoneScaling::GetBoneScale(FName BoneName)
{
	USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(GetOuter()->GetOuter());
	if (SkelComp != NULL)
	{
		const INT BoneIndex = SkelComp->MatchRefBone(BoneName);
		if (BoneIndex == INDEX_NONE)
		{
			debugf(NAME_Warning, TEXT("Failed to find bone named %s in mesh %s"), *BoneName.ToString(), *SkelComp->SkeletalMesh->GetName());
			return 0.f;
		}
		else
		{
			return GetBoneScale(BoneIndex, SkelComp);
		}
	}
	else
	{
		return 0.f;
	}
}

void UUTSkelControl_MassBoneScaling::GetAffectedBones(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<INT>& OutBoneIndices)
{
	if (BoneIndex < BoneScales.Num() && Abs(BoneScales(BoneIndex) - 1.0f) > KINDA_SMALL_NUMBER)
	{
		OutBoneIndices.AddItem(BoneIndex);
	}
}

void UUTSkelControl_MassBoneScaling::CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms)
{
	OutBoneTransforms.AddItem(SkelComp->SpaceBases(BoneIndex));
}

FLOAT UUTSkelControl_MassBoneScaling::GetBoneScale(INT BoneIndex, USkeletalMeshComponent* SkelComp)
{
	return (BoneIndex < BoneScales.Num()) ? BoneScales(BoneIndex) : 1.0f;
}

void UUTSkelControl_CicadaEngine::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	AUTVehicle* OwnerVehicle = Cast<AUTVehicle>(SkelComp->GetOwner());
	PitchTime = PitchRate;
	if (OwnerVehicle != NULL && OwnerVehicle->bDriving && SkelComp->LastRenderTime > OwnerVehicle->WorldInfo->TimeSeconds - 1.0f)
	{
		const FLOAT Thrust = OwnerVehicle->OutputGas;
		if ( Thrust != LastThrust )
		{
			if ( Thrust > 0 )
			{
				DesiredPitch = appTrunc(ForwardPitch * 182.0444);
			}
			else if ( Thrust < 0 )
			{
				DesiredPitch = appTrunc(BackPitch * 182.0444);
			}
			else
			{
				DesiredPitch = 0;
			}

			// Use the Speed to determine the rate at which it moves
			const FLOAT Speed = Clamp<FLOAT>( OwnerVehicle->Velocity.Size2D(), MinVelocity, MaxVelocity );
			const FLOAT Pct = (Speed - MinVelocity) / (MaxVelocity - MinVelocity);
			PitchTime *= ( 1 + ( (MaxVelocityPitchRateMultiplier - 1) * Pct) );
		}
		LastThrust = Thrust;
	}
	else
	{
		DesiredPitch = 0;
	}

	if ( BoneRotation.Pitch != DesiredPitch )
	{
		BoneRotation.Pitch += appTrunc((DesiredPitch - BoneRotation.Pitch) * (DeltaSeconds / PitchTime));
		PitchTime -= DeltaSeconds;
		if ( PitchTime <= 0 || DesiredPitch == BoneRotation.Pitch )
		{
			PitchTime = 0.0;
			BoneRotation.Pitch = DesiredPitch;
		}
	}

	Super::TickSkelControl(DeltaSeconds,SkelComp);

}

void UUTSkelControl_JetThruster::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	AUTVehicle* OwnerVehicle = Cast<AUTVehicle>(SkelComp->GetOwner());
	
	if ( !GIsEditor )
	{
		if (OwnerVehicle != NULL && SkelComp->LastRenderTime > OwnerVehicle->WorldInfo->TimeSeconds - 1.0f)
		{
			FLOAT NewDesiredStrength = 0.0f;

			if (OwnerVehicle->bDriving)
			{
				const FLOAT DP = OwnerVehicle->Velocity.SafeNormal() | OwnerVehicle->Rotation.Vector().SafeNormal();
				if ( DP > 0.0f )
				{
					const FLOAT Thrust = Clamp<FLOAT>(OwnerVehicle->Velocity.Size2D(), 0.0f, MaxForwardVelocity);
					NewDesiredStrength =  1 - (Thrust / MaxForwardVelocity);
				}
				else
				{
					NewDesiredStrength = 1.0f;
				}
			}
			else
			{
				NewDesiredStrength = 1.0;
			}

			if (NewDesiredStrength != DesiredStrength)
			{
				BlendTimeToGo += ( NewDesiredStrength - DesiredStrength ) * BlendRate;
				BlendTimeToGo = Clamp<FLOAT>(BlendTimeToGo, 0.0f, BlendRate);
				StrengthTarget = NewDesiredStrength;
			}
		}
	}

	Super::TickSkelControl(DeltaSeconds, SkelComp);
}

void UUTSkelControl_Rotate::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	if ( DesiredBoneRotation.Pitch != BoneRotation.Pitch )
	{
		BoneRotation.Pitch = RotationalTurn(BoneRotation.Pitch, DesiredBoneRotation.Pitch, FLOAT(DesiredBoneRotationRate.Pitch) *  DeltaSeconds);
	}

	if ( DesiredBoneRotation.Yaw != BoneRotation.Yaw )
	{
		BoneRotation.Yaw = RotationalTurn(BoneRotation.Yaw, DesiredBoneRotation.Yaw, FLOAT(DesiredBoneRotationRate.Yaw) *  DeltaSeconds);
	}

	if ( DesiredBoneRotation.Roll != BoneRotation.Roll )
	{
		BoneRotation.Roll = RotationalTurn(BoneRotation.Roll, DesiredBoneRotation.Roll, FLOAT(DesiredBoneRotationRate.Roll) *  DeltaSeconds);
	}

	Super::TickSkelControl(DeltaSeconds, SkelComp);
}

UBOOL UUTSkelControl_LookAt::ApplyLookDirectionLimits(FVector& DesiredLookDir, const FVector &CurrentLookDir, INT BoneIndex, USkeletalMeshComponent* SkelComp)
{
	const FRotator BaseLookDirRot = CurrentLookDir.Rotation();
	FRotator DesiredLookDirRot = DesiredLookDir.Rotation();
	FRotator DeltaRot = (DesiredLookDirRot - BaseLookDirRot).GetNormalized();	

	//FRotator DeltaRotTest = (DesiredLookDir - CurrentLookDir).Rotation().Normalize();

	// clamp the delta
	if (bLimitPitch)
	{
		const INT PitchLimitUnr = appTrunc(Abs(PitchLimit) * (65536.f/360.f));
		DeltaRot.Pitch = Clamp(DeltaRot.Pitch, -PitchLimitUnr, PitchLimitUnr);
	}
	if (bLimitYaw)
	{
		const INT YawLimitUnr = appTrunc(Abs(YawLimit) * (65536.f/360.f));
		DeltaRot.Yaw = Clamp(DeltaRot.Yaw, -YawLimitUnr, YawLimitUnr);
	}
	if (bLimitRoll)
	{
		const INT RollLimitUnr = appTrunc(Abs(RollLimit) * (65536.f/360.f));
		DeltaRot.Roll = Clamp(DeltaRot.Roll, -RollLimitUnr, RollLimitUnr);
	}

	// re-apply the clamped delta
	//FVector DesiredLookDirTest = DeltaRot.Vector() + CurrentLookDir;
	DesiredLookDirRot = BaseLookDirRot + DeltaRot;
	DesiredLookDir = DesiredLookDirRot.Vector();

	// let super do any limiting it wants to as well
	return Super::ApplyLookDirectionLimits(DesiredLookDir, CurrentLookDir, BoneIndex, SkelComp);
}


void UUTSkelControl_LookAt::DrawSkelControl3D(const FSceneView* View, FPrimitiveDrawInterface* PDI, USkeletalMeshComponent* SkelComp, INT BoneIndex)
{
	if (bShowPerAxisLimits && (bLimitYaw || bLimitPitch || bLimitRoll) )
	{
		// Calculate transform for cone.
		FVector YAxis, ZAxis;
		LimitLookDir.FindBestAxisVectors(YAxis, ZAxis);
		const FVector	ConeOrigin		= SkelComp->SpaceBases(BoneIndex).GetOrigin();
		const FLOAT		MaxAngle1Radians = YawLimit * ((FLOAT)PI/180.f);
		const FLOAT		MaxAngle2Radians = PitchLimit * ((FLOAT)PI/180.f);
		const FMatrix	ConeToWorld		= FScaleMatrix(FVector(30.f)) * FMatrix(LimitLookDir, YAxis, ZAxis, ConeOrigin) * SkelComp->LocalToWorld;

		UMaterialInterface* LimitMaterial = LoadObject<UMaterialInterface>(NULL, TEXT("EditorMaterials.PhAT_JointLimitMaterial"), NULL, LOAD_None, NULL);

		DrawCone(PDI, ConeToWorld, MaxAngle1Radians, MaxAngle2Radians, 40, TRUE, FColor(64,255,64), LimitMaterial->GetRenderProxy(FALSE), SDPG_World);
	}

	Super::DrawSkelControl3D(View, PDI, SkelComp, BoneIndex);
}

void UUTSkelControl_LookAt::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	// UT version automatically does interpolation
	TargetLocation = VInterpTo(TargetLocation, DesiredTargetLocation, TargetLocationInterpSpeed, DeltaSeconds);

	Super::TickSkelControl(DeltaSeconds, SkelComp);
}

void UUTSkelControl_HoverboardSuspension::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	AUTVehicle_Hoverboard* Hoverboard = Cast<AUTVehicle_Hoverboard>(SkelComp->GetOwner());
	// Check we have a hoverboard and it has two valid wheels.
	if(!Hoverboard || Hoverboard->Wheels.Num() != 2 || !Hoverboard->Wheels(0) || !Hoverboard->Wheels(1))
	{
		return;
	}

	const FLOAT FrontWheelHeight = Hoverboard->Wheels(0)->SuspensionPosition;
	const FLOAT RearWheelHeight = Hoverboard->Wheels(1)->SuspensionPosition;
	const FLOAT AverageHeight = (FrontWheelHeight + RearWheelHeight) * 0.5f;
	BoneTranslation = FVector(0,0,1) * Max<FLOAT>((TransOffset + Clamp<FLOAT>((AverageHeight - TransIgnore) * TransScale, 0.f, MaxTrans)), MinTrans);

	const FLOAT HeightDiff = (FrontWheelHeight - RearWheelHeight);
	const FLOAT TargetRot = Clamp<FLOAT>(HeightDiff * RotScale * 2 * (FLOAT)PI, -MaxRot, MaxRot);
	
	FLOAT MaxDeltaRot = DeltaSeconds * MaxRotRate;
	CurrentRot += Clamp(TargetRot - CurrentRot, -MaxDeltaRot, MaxDeltaRot);

	BoneRotation  = FQuat( FVector(0,1,0), CurrentRot);

	Super::TickSkelControl(DeltaSeconds, SkelComp);
}

static FLOAT ArrayAverage(const TArray<FLOAT>& InArray)
{
	FLOAT Total = 0.f;
	for(INT i=0; i<InArray.Num(); i++)
	{
		Total += InArray(i);
	}
	return Total/((FLOAT)InArray.Num());
}

void UUTSkelControl_HoverboardSwing::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	AUTVehicle_Hoverboard* Hoverboard = Cast<AUTVehicle_Hoverboard>(SkelComp->GetOwner());
	// Check we have a hoverboard and it has two valid wheels.
	if(!Hoverboard || !Hoverboard->CollisionComponent || !Hoverboard->CollisionComponent->BodyInstance)
	{
		return;
	}

	// Ensure history window size is correct.
	if(SwingHistory.Num() != SwingHistoryWindow)
	{
		SwingHistory.Empty();
		SwingHistory.AddZeroed(SwingHistoryWindow);
		SwingHistorySlot=0;
	}

	// Look at yaw speed and use that to adjust 'swing out' of the board.
	const FLOAT UseVel = Clamp(Hoverboard->ForwardVel, -MaxUseVel, MaxUseVel);
	const FVector AngVel = Hoverboard->CollisionComponent->BodyInstance->GetUnrealWorldAngularVelocity();
	const FLOAT TargetSwing = Clamp<FLOAT>(UseVel * AngVel.Z * SwingScale * 2 * (FLOAT)PI, -MaxSwing, MaxSwing);

	// Update rolling history
	SwingHistory(SwingHistorySlot) = TargetSwing;
	SwingHistorySlot = (SwingHistorySlot+1)%SwingHistoryWindow;
	
	// Then set current 
	CurrentSwing = ArrayAverage(SwingHistory);
}

void UUTSkelControl_HoverboardSwing::CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms)
{
	BoneRotation = FQuat( FVector(1,0,0), CurrentSwing );

	Super::CalculateNewBoneTransforms(BoneIndex, SkelComp, OutBoneTransforms);
}

void UUTSkelControl_HoverboardVibration::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	Super::TickSkelControl(DeltaSeconds, SkelComp);

	AUTVehicle_Hoverboard* Hoverboard = Cast<AUTVehicle_Hoverboard>(SkelComp->GetOwner());
	if(Hoverboard)
	{
		FLOAT Speed = Hoverboard->Velocity.Size();
		VibInput += (VibFrequency * Speed * DeltaSeconds * 2 * (FLOAT)PI);
	}
}

void UUTSkelControl_HoverboardVibration::CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms)
{
	AUTVehicle_Hoverboard* Hoverboard = Cast<AUTVehicle_Hoverboard>(SkelComp->GetOwner());
	// Check we have a hoverboard and it has two valid wheels.
	if(!Hoverboard || Hoverboard->Wheels.Num() != 2 || !Hoverboard->Wheels(0) || !Hoverboard->Wheels(1) 
		|| !Hoverboard->CollisionComponent || !Hoverboard->CollisionComponent->BodyInstance)
	{
		return;
	}

	// Don't vibrate board if wheels are not on the ground
	if(!Hoverboard->Wheels(0)->bWheelOnGround || !Hoverboard->Wheels(1)->bWheelOnGround)
	{
		return;
	}

	FVector AngVel = Hoverboard->CollisionComponent->BodyInstance->GetUnrealWorldAngularVelocity();
	FLOAT Speed = Hoverboard->Velocity.Size();
	FLOAT VibeMag = (Speed * VibSpeedAmpScale) +  (Speed * Abs<FLOAT>(AngVel.Z) * VibTurnAmpScale);
	VibeMag = Min<FLOAT>(VibeMag, VibMaxAmplitude);
	FLOAT Vibe = VibeMag * appSin( VibInput );

	BoneTranslation = FVector(0,0,Vibe);

	Super::CalculateNewBoneTransforms(BoneIndex, SkelComp, OutBoneTransforms);
}

void UUTSkelControl_SpinControl::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent *SkelComp)
{
	const FLOAT RotationAmount = DeltaSeconds*DegreesPerSecond*182.0444;
	if(!Axis.IsZero())
	{
		Axis.Normalize();
		BoneRotation.Add(appTrunc(Axis.Y*RotationAmount),appTrunc(Axis.Z*RotationAmount),appTrunc(Axis.X*RotationAmount));
	}
	Super::TickSkelControl(DeltaSeconds,SkelComp);
}
