/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
#include "GearGame.h"
#include "EngineMaterialClasses.h"
#include "UnNet.h"

#include "GearGameVehicleClasses.h"
#include "GearGameSequenceClasses.h"
#include "GearGameWeaponClasses.h"
#include "GearGameAnimClasses.h"

IMPLEMENT_CLASS(AGearVehicleBase);
IMPLEMENT_CLASS(AGearVehicle);
IMPLEMENT_CLASS(AGearWeaponPawn);
IMPLEMENT_CLASS(AGearVehicleWeapon);
IMPLEMENT_CLASS(AVehicle_Centaur_Base);

IMPLEMENT_CLASS(UGearVehicleSimCar);
IMPLEMENT_CLASS(UGearVehicleSimChopper);
IMPLEMENT_CLASS(UGearVehicleSimHover);

IMPLEMENT_CLASS(ACinematicCentaur_Base);

IMPLEMENT_CLASS(AHydra_Base);

IMPLEMENT_CLASS(AVehicle_Crane_Base);

UBOOL AGearVehicle::IsValidEnemyTargetFor(const APlayerReplicationInfo* OtherPRI, UBOOL bNoPRIIsEnemy) const
{
	if( bNeverAValidEnemy )
	{
		return FALSE;
	}

	// if we're on the magical neutral team, then we're not a valid enemy for anyone
	if(((AGearVehicle*)this)->GetTeamNum() == 254)
	{
		return FALSE;
	}

	return Super::IsValidEnemyTargetFor( OtherPRI, bNoPRIIsEnemy );
}

/**
* Returns camera "no render" cylinder.
* @param bViewTarget TRUE to return the cylinder for use when this pawn is the camera target, false to return the non-viewtarget dimensions
*/
void AGearVehicle::GetCameraNoRenderCylinder(FLOAT& Radius, FLOAT& Height, UBOOL bViewTarget, UBOOL bHiddenLocally)
{
	if (bViewTarget)
	{
		Radius = CameraNoRenderCylinder_High_ViewTarget.Radius;
		Height = CameraNoRenderCylinder_High_ViewTarget.Height;
	}
	else
	{
		Radius = CameraNoRenderCylinder_High.Radius;
		Height = CameraNoRenderCylinder_High.Height;
	}

	// if already hidden, we expand the cylinder a little bit to prevent flickering caused
	// by the camera being right on the boundary.
	if (bHiddenLocally)
	{
		Radius += CameraNoRenderCylinder_FlickerBuffer.Radius;
		Height += CameraNoRenderCylinder_FlickerBuffer.Height;
	}
}


void AGearVehicle::OnRigidBodyCollision(const FRigidBodyCollisionInfo& Info0, const FRigidBodyCollisionInfo& Info1, const FCollisionImpactData& RigidCollisionData)
{
	Super::OnRigidBodyCollision(Info0, Info1, RigidCollisionData);

	AActor* OtherActor = (Info0.Actor != this) ? Info0.Actor : Info1.Actor;
	FLOAT ImpactMag = RigidCollisionData.TotalNormalForceVector.Size();

	// If we find a contact that is penetrating too much (and actually generating a force) - destroy vehicle.
	if(ImpactMag > 0.f)
	{
		for(INT i=0; i<RigidCollisionData.ContactInfos.Num(); i++)
		{
			const FRigidBodyContactInfo& ContactInfo = RigidCollisionData.ContactInfos(i);
			if(ContactInfo.ContactPenetration > DestroyOnPenetrationThreshold)
			{
				bIsInDestroyablePenetration = TRUE;
			}
		}
	}

	// If the impact force is non-zero
	if(ImpactMag > KINDA_SMALL_NUMBER)
	{
		FVector ImpactNorm = RigidCollisionData.TotalNormalForceVector/ImpactMag;
		FLOAT ForwardImpactMag = Abs(Mesh->LocalToWorld.GetAxis(0) | ImpactNorm);
		if(ForwardImpactMag > 0.7f)
		{
			bFrontalCollision = TRUE;

			if(OtherActor && OtherActor->Physics != PHYS_RigidBody)
			{
				bFrontalCollisionWithFixed = TRUE;
			}
		}
	}

	if(GWorld->GetNetMode() != NM_DedicatedServer && Health <= 0 && LastDeathImpactTime + 0.6 < GWorld->GetTimeSeconds() && Info0.Actor != NULL && Info1.Actor != NULL) // impact sounds on clients for dead vehicles
	{
		LastDeathImpactTime = GWorld->GetTimeSeconds();
		FVector ContactLoc = RigidCollisionData.ContactInfos(0).ContactPosition;
		// Notes to self: using consistent self destruct: Speedbike numbers: 1000-7000, Goliath numbers: all over 40k
		if(ImpactMag >= 20000.0f && LargeChunkImpactSound != NULL) // large chunk
		{
			PlaySound(LargeChunkImpactSound,TRUE,TRUE,TRUE,&ContactLoc);
		}
		else if(ImpactMag >= 4000.0f && MediumChunkImpactSound != NULL) // medium chunk
		{
			PlaySound(MediumChunkImpactSound,TRUE,TRUE,TRUE,&ContactLoc);
		}
		else if(ImpactMag >= 1000.0f && SmallChunkImpactSound != NULL) // small chunk
		{
			PlaySound(SmallChunkImpactSound,TRUE,TRUE,TRUE,&ContactLoc);
		}
	}
}

void AGearVehicle::PostEditChange( UProperty* PropertyThatChanged )
{
	if (!GIsEditor && !IsTemplate())
	{
		eventOnPropertyChange( *PropertyThatChanged->GetName() );
	}

	Super::PostEditChange(PropertyThatChanged);
}

/** 
* In order to have an easily extendable system, we use the following accessor function to perform property lookup based
* on the name of the property.  This name is precached when the vehicle is constructed.  For more information see the
* comments in UTVehicle.uc and https://udn.epicgames.com/Three/PropertyOverview
*
* @param	SeatIndex		The Seat in question
* @param	NewRot			If writing, the new rotation to write
* @param	bReadValue		If TRUE, we are reading this value, not writing it
*
* @return the value if bReadValue is TRUE
*/

FRotator AGearVehicle::SeatWeaponRotation(INT SeatIndex,FRotator NewRot,UBOOL bReadValue)
{
	FRotator Result = FRotator(0,0,0);
	if ( SeatIndex>=0 && SeatIndex < Seats.Num() )
	{
		if ( !Seats(SeatIndex).WeaponRotationProperty )
		{
			// Find the UProperty in question

			UProperty* Prop = FindField<UProperty>(GetClass(), Seats(SeatIndex).WeaponRotationName);
			if (Prop != NULL)
			{

				// check to make sure the property is an FRotator.  We do this by insuring it's a UStructProperty named
				// Rotator.

				if (Prop->GetClass() != UStructProperty::StaticClass() || ((UStructProperty*)Prop)->Struct->GetFName() != NAME_Rotator)
				{
					debugf( NAME_Warning, TEXT("WeaponRotation property type mismatch: %s is %s, expected Rotator"), *Seats(SeatIndex).WeaponRotationName.ToString(), 
						(Prop->GetClass() != UStructProperty::StaticClass()) ? *Prop->GetClass()->GetName() : *((UStructProperty*)Prop)->Struct->GetName() );
					Prop = NULL;
				}
			}

			// Short circut if we couldn't find the property

			if (Prop == NULL)
			{
				return Result;
			}

			Seats(SeatIndex).WeaponRotationProperty = Prop;
		}

		/*
		Process the value.  A property doesn't hold the value of the property, it describes where in its owner 
		struct's (or class's) _instance_ to find the value of the property. So, the code gets the offset of the 
		property that it found by name, adds that offset to the beginning of the memory used by the vehicle instance, 
		and then copies what that memory location is pointing to. 
		*/

		BYTE* PropLoc = (BYTE*) this + ((UProperty*) Seats(SeatIndex).WeaponRotationProperty)->Offset;

		if ( bReadValue )
		{
			((UProperty*) Seats(SeatIndex).WeaponRotationProperty)->CopySingleValue(&Result, PropLoc);
		}
		else
		{
			((UProperty*) Seats(SeatIndex).WeaponRotationProperty)->CopySingleValue(PropLoc, &NewRot);
			bNetDirty=TRUE;
		}
	}

	return Result;
}

/** 
* In order to have an easily extendable system, we use the following accessor function to perform property lookup based
* on the name of the property.  This name is precached when the vehicle is constructed.  For more information see the
* comments in UTVehicle.uc and https://udn.epicgames.com/Three/PropertyOverview
*
* @param	SeatIndex		The Seat in question
* @param	NewLoc			If writing, the new location to write
* @param	bReadValue		If TRUE, we are reading this value, not writing it
*
* @return the value if bReadValue is TRUE
*/


FVector AGearVehicle::SeatFlashLocation(INT SeatIndex,FVector NewLoc,UBOOL bReadValue)
{
	FVector Result = FVector(0,0,0);

	if ( SeatIndex>=0 && SeatIndex < Seats.Num() )
	{
		if ( !Seats(SeatIndex).FlashLocationProperty )
		{
			UProperty* Prop = FindField<UProperty>(GetClass(), Seats(SeatIndex).FlashLocationName);
			if (Prop != NULL)
			{
				if (Prop->GetClass() != UStructProperty::StaticClass() || ((UStructProperty*)Prop)->Struct->GetFName() != NAME_Vector)
				{
					debugf( NAME_Warning, TEXT("FlashLocation property type mismatch: %s is %s, expected Vector"), *Seats(SeatIndex).FlashLocationName.ToString(), 
						(Prop->GetClass() != UStructProperty::StaticClass()) ? *Prop->GetClass()->GetName() : *((UStructProperty*)Prop)->Struct->GetName() );
					Prop = NULL;
				}
			}

			if (Prop == NULL)
			{
				return Result;
			}

			Seats(SeatIndex).FlashLocationProperty = Prop;
		}

		BYTE* PropLoc = (BYTE*) this + ((UProperty*) Seats(SeatIndex).FlashLocationProperty)->Offset;
		if ( bReadValue )
		{
			((UProperty*) Seats(SeatIndex).FlashLocationProperty)->CopySingleValue(&Result, PropLoc);
		}
		else
		{
			((UProperty*) Seats(SeatIndex).FlashLocationProperty)->CopySingleValue(PropLoc, &NewLoc);
			bNetDirty=TRUE;
		}
	}
	return Result;
}

/** 
* In order to have an easily extendable system, we use the following accessor function to perform property lookup based
* on the name of the property.  This name is precached when the vehicle is constructed.  For more information see the
* comments in UTVehicle.uc and https://udn.epicgames.com/Three/PropertyOverview
*
* @param	SeatIndex		The Seat in question
* @param	NewCount		If writing, the new count to write
* @param	bReadValue		If TRUE, we are reading this value, not writing it
*
* @return the value if bReadValue is TRUE
*/


BYTE AGearVehicle::SeatFlashCount(INT SeatIndex, BYTE NewCount, UBOOL bReadValue)
{
	BYTE Result = 0;

	if ( SeatIndex>=0 && SeatIndex < Seats.Num() )
	{
		if ( !Seats(SeatIndex).FlashCountProperty)
		{
			UProperty* Prop = FindField<UProperty>(GetClass(), Seats(SeatIndex).FlashCountName);
			if (Prop != NULL)
			{
				if (Prop->GetClass() != UByteProperty::StaticClass())
				{
					debugf(NAME_Warning, TEXT("FlashCount property type mismatch: %s is %s, expected ByteProperty"), *Seats(SeatIndex).FlashCountName.ToString(), *Prop->GetClass()->GetName());
					Prop = NULL;
				}
			}

			if (Prop == NULL)
			{
				return Result;
			}

			Seats(SeatIndex).FlashCountProperty = Prop;
		}

		BYTE* PropLoc = (BYTE*) this + ((UProperty*) Seats(SeatIndex).FlashCountProperty)->Offset;
		if ( bReadValue )
		{
			((UProperty*) Seats(SeatIndex).FlashCountProperty)->CopySingleValue(&Result, PropLoc);
		}
		else
		{
			((UProperty*) Seats(SeatIndex).FlashCountProperty)->CopySingleValue(PropLoc, &NewCount);
			bNetDirty=TRUE;
		}
	}
	return Result;
}

/** 
* In order to have an easily extendable system, we use the following accessor function to perform property lookup based
* on the name of the property.  This name is precached when the vehicle is constructed.  For more information see the
* comments in UTVehicle.uc and https://udn.epicgames.com/Three/PropertyOverview
*
* @param	SeatIndex		The Seat in question
* @param	NewFireMode		writing, the new firing mode to write
* @param	bReadValue		If TRUE, we are reading this value, not writing it
*
* @return the value if bReadValue is TRUE
*/

BYTE AGearVehicle::SeatFiringMode(INT SeatIndex, BYTE NewFireMode, UBOOL bReadValue)
{
	BYTE Result = 0;

	if ( SeatIndex>=0 && SeatIndex < Seats.Num() )
	{
		if ( !Seats(SeatIndex).FiringModeProperty )
		{
			UProperty* Prop = FindField<UProperty>(GetClass(), Seats(SeatIndex).FiringModeName);
			if (Prop != NULL)
			{
				if (Prop->GetClass() != UByteProperty::StaticClass())
				{
					debugf(NAME_Warning, TEXT("FiringMode property type mismatch: %s is %s, expected ByteProperty"), *Seats(SeatIndex).FiringModeName.ToString(), *Prop->GetClass()->GetName());
					Prop = NULL;
				}
			}

			if (Prop == NULL)
			{
				return Result;
			}

			Seats(SeatIndex).FiringModeProperty = Prop;
		}

		BYTE* PropLoc = (BYTE*) this + ((UProperty*) Seats(SeatIndex).FiringModeProperty)->Offset;
		if ( bReadValue )
		{
			((UProperty*) Seats(SeatIndex).FiringModeProperty)->CopySingleValue(&Result, PropLoc);
		}
		else
		{
			((UProperty*) Seats(SeatIndex).FiringModeProperty)->CopySingleValue(PropLoc, &NewFireMode);
			bNetDirty=TRUE;
		}
	}
	return Result;
}

void AGearVehicle::execIsSeatControllerReplicationViewer(FFrame& Stack, RESULT_DECL)
{
	P_GET_INT(SeatIndex);
	P_FINISH;

	UBOOL bResult = FALSE;
	if (SeatIndex < Seats.Num() && Seats(SeatIndex).SeatPawn != NULL)
	{
		for (INT i = 0; i < WorldInfo->ReplicationViewers.Num(); i++)
		{
			if (WorldInfo->ReplicationViewers(i).InViewer == Seats(SeatIndex).SeatPawn->Controller)
			{
				bResult = TRUE;
				break;
			}
		}
	}

	*(UBOOL*)Result = bResult;
}

FGuid* AGearVehicle::GetGuid()
{
	MyGuid = FGuid(EC_EventParm);
	if (Controller != NULL)
	{
		FGuid* ControllerGuid = Controller->GetGuid();
		if (ControllerGuid != NULL)
		{
			MyGuid = *ControllerGuid;
			MyGuid.A += 1;
		}
	}

	return &MyGuid;
}

void AGearVehicle::TickSpecial( FLOAT DeltaSeconds )
{
	Super::TickSpecial(DeltaSeconds);

	// Reset frontal collision flags.
	bFrontalCollision = FALSE;
	bFrontalCollisionWithFixed = FALSE;

	// See if we are penetrating deeply enough to cause destruction. 
	// Update how long we've been in that situation if so.
	if(bIsInDestroyablePenetration)
	{
		TimeInDestroyablePenetration += DeltaSeconds;
	}
	else
	{
		TimeInDestroyablePenetration = 0.f;
	}
	bIsInDestroyablePenetration = FALSE;

	// If its been too long - blow up.
	if(TimeInDestroyablePenetration >= DestroyOnPenetrationDuration)
	{
		eventRBPenetrationDestroy();
		if (bDeleteMe || Health < 0)
		{
			// Do nothing further.
			return;
		}
	}

	// use appropriate physical material depending on whether being driven

	if ( WorldInfo->NetMode != NM_DedicatedServer )
	{
		if ( bDeadVehicle )
		{
			/*
			if (bIsBurning && BurnOutMaterialInstances.Num() > 0)
			{
				RemainingBurn -= DeltaSeconds;

				for (INT i = 0; i < BurnOutMaterialInstances.Num(); i++)
				{
					BurnOutMaterialInstances(i).CurrValue -= DeltaSeconds;

					if(RemainingBurn > 0.0f)
					{
						//	if (BurnOutMaterialInstances(i).MIC != NULL)
						//{
						//BurnOutMaterialInstances(i).MIC->SetScalarParameterValue(BurnTimeParameterName, BurnOutMaterialInstances(i).CurrValue - 1.0f); 
						//}
					}
					else
					{
						// @todo:  turn off collision here also
						SetHidden(TRUE);
					}
				}
			}
			*/
			return;
		}


		// deal with wheel sounds/effects
		if ( bVehicleOnGround ) // on the ground, with tire sounds, moving anywhere but straight up/down
		{
			UBOOL bFoundViewer = FALSE;
			// only do this if close enough to a local player
			for( INT iPlayers=0; iPlayers<GEngine->GamePlayers.Num(); iPlayers++ )
			{
				if ( GEngine->GamePlayers(iPlayers) && GEngine->GamePlayers(iPlayers)->Actor )
				{
					APlayerController *PC = GEngine->GamePlayers(iPlayers)->Actor;
					if ( PC->ViewTarget && ((Location - PC->ViewTarget->Location).SizeSquared()< MaxWheelEffectDistSq) )
					{
						bFoundViewer = TRUE;
						break;
					}
				}
			}

			/*
			if ( bFoundViewer && (TireAudioComp != NULL || WheelParticleEffects.Num() > 0) )
			{
				FLOAT curSpd = Velocity.Size2D();

				// while moving:
				if(curSpd > 10.f) // if we're moving or we're on the gas.
				{
					FCheckResult HitRes(1.0f);
					FTraceHitInfo HitInfo;
					FVector TraceStart(Location.X,Location.Y,Location.Z);
					if (CylinderComponent != NULL)
					{
						TraceStart.Z -= CylinderComponent->CollisionHeight - CylinderComponent->Translation.Z;
					}
					FVector EndPt = TraceStart;
					EndPt.Z -= 32.f;

					FName NewMaterialType = NAME_None;
					GWorld->SingleLineCheck(HitRes, this, EndPt, TraceStart, TRACE_World | TRACE_Material | TRACE_PhysicsVolumes);

					if ( HitRes.Actor )
					{
						APhysicsVolume* HitVolume = Cast<APhysicsVolume>(HitRes.Actor);
						if ( HitVolume && HitVolume->bWaterVolume )
						{
							NewMaterialType = WaterEffectType;
						}
						else
						{
							DetermineCorrectPhysicalMaterial<FCheckResult, FTraceHitInfo>( HitRes, HitInfo );
							// we now have a phys material so we can see if we need to update the sound
							HitInfo.Material = HitRes.Material ? HitRes.Material->GetMaterial() : NULL;
							UUTPhysicalMaterialProperty* UTPMP = (HitInfo.Material && HitInfo.Material->PhysMaterial)
								? Cast<UUTPhysicalMaterialProperty>(HitInfo.Material->PhysMaterial->PhysicalMaterialProperty)
								: NULL;
						}
					}

					// If the material we are over has changed
					if(NewMaterialType != CurrentTireMaterial)
					{
						// First make sure current sound is faded out
						if(TireAudioComp)
						{
							if(TireAudioComp->bWasPlaying)
							{
								TireAudioComp->FadeOut(0.3f,0.0f);
							}

							TireAudioComp = NULL; // Will be GCd
						}

						// If we have a sound list, look for a new one to play
						if(TireSoundList.Num() > 0)
						{
							// Now look for new sound to play
							USoundCue* NewSound = NULL;
							// Slight hack - if no name - use first element of array
							if(NewMaterialType == NAME_None)
							{
								NewSound = TireSoundList(0).Sound;
							}
							else
							{
								// Iterate over list looking for correct sound
								for(INT i=0; i<TireSoundList.Num(); i++)
								{
									if(TireSoundList(i).MaterialType == NewMaterialType)
									{
										NewSound = TireSoundList(i).Sound;
										break;
									}
								}
							}

							// Now play new sound
							if(NewSound)
							{
								TireAudioComp = CreateAudioComponent(NewSound, FALSE, TRUE, FALSE);
							}
						}

						// Update current material
						CurrentTireMaterial = NewMaterialType;
					}

					if ( WheelParticleEffects.Num() > 0 && HitRes.Time < 1.0f )
					{
						INT EffectIndex = 0;
						if ( NewMaterialType != NAME_None )
						{
							for (INT i = 0; i < WheelParticleEffects.Num(); i++)
							{
								if (WheelParticleEffects(i).MaterialType == NewMaterialType)
								{
									EffectIndex = i;
									break;
								}
							}
						}

						for (INT i = 0; i < Wheels.Num(); i++)
						{
							UUTVehicleWheel* Wheel = Cast<UUTVehicleWheel>(Wheels(i));
							if ( Wheel && Wheel->bUseMaterialSpecificEffects && Wheel->WheelParticleComp
								&& Wheel->WheelParticleComp->Template != WheelParticleEffects(EffectIndex).ParticleTemplate )
							{
								Wheel->eventSetParticleEffect(this, WheelParticleEffects(EffectIndex).ParticleTemplate);
							}
						}
					}
					if (TireAudioComp != NULL)
					{
						if(!TireAudioComp->bWasPlaying)
						{
							TireAudioComp->Play();
						}
						TireAudioComp->AdjustVolume(0.1f, Min<FLOAT>(1.0,curSpd/(AirSpeed*0.10f)) ); // go to full volume if >10%, else to the % of 10%
						TireAudioComp->PitchMultiplier = 0.5f + 1.25f*(curSpd/AirSpeed); // 0 = 0.5, 40% = 1.0, 80% = 1.5
					}
				}
				else if (TireAudioComp != NULL) // not moving, stop tires.
				{
					TireAudioComp->Stop();
				}
			}
			else if (TireAudioComp != NULL) 
			{
				TireAudioComp->Stop();
			}
			*/
		}
	}

	if ( Role == ROLE_Authority )
	{
		// check if vehicle is upside down and on ground
		if ( bIsInverted && bWasChassisTouchingGroundLastTick )
		{
			if ( WorldInfo->TimeSeconds - LastCheckUpsideDownTime > 0.5f )
			{
				if (WorldInfo->TimeSeconds - LastCheckUpsideDownTime > 1.f)
				{
					if ( bIsScraping && ScrapeSound )
					{
						ScrapeSound->Stop();
						bIsScraping = FALSE;
					}
				}

				// Check if we are upside down and touching the level every 0.5 seconds.
				if ( bEjectPassengersWhenFlipped )
				{
					FlippedCount++;
					if ( FlippedCount > 2 )
					{
						if (Driver)
							eventDriverLeave(TRUE);

						for ( INT i=0; i<Seats.Num(); i++ )
							if ( Seats(i).SeatPawn )
								Seats(i).SeatPawn->eventDriverLeave(TRUE);

						FlippedCount = 0;
					}
					LastCheckUpsideDownTime = WorldInfo->TimeSeconds;
				}
			}
		}	
		else
		{
			if ( ScrapeSound )
			{
				if ( bWasChassisTouchingGroundLastTick && (Velocity.SizeSquared() > 200000.f) && (WorldInfo->TimeSeconds - LastCollisionSoundTime > CollisionIntervalSecs) )
				{
					if ( !bIsScraping )
					{
						ScrapeSound->Play();
						bIsScraping = TRUE;
					}
				}
				else if ( bIsScraping )
				{
					ScrapeSound->Stop();
					bIsScraping = FALSE;
				}
			}
			FlippedCount = 0;
		}
	}

	if ( Controller && (Role == ROLE_Authority || IsLocallyControlled()) && Driver )
	{
		// Don't do anything if the pawn is in fixed view
		AGearPawn *GearDriverPawn = Cast<AGearPawn>(Driver);
		if ( GearDriverPawn )// && !GearDriverPawn->bFixedView)
		{
			FRotator Rot = Controller->Rotation;
			ApplyWeaponRotation(0, Rot);
		}
	}

	// Handle seats making noise when moving.
	if ( WorldInfo->NetMode != NM_DedicatedServer )	
	{
		for (INT i=0;i < Seats.Num(); i++)
		{
			if ( Seats(i).SeatMotionAudio )
			{
				// We have to check rotations as the TurretController might be set to instant
				// and we wouldn't see the motion.
				FRotator CurrentRotation = SeatWeaponRotation(i, FRotator(0,0,0), TRUE);
				UBOOL bInMotion = (CurrentRotation.Vector() - Seats(i).LastWeaponRotation.Vector()).Size() > KINDA_SMALL_NUMBER;

				// Now look at each controller directly and see if it's in motion
				for (INT TCIndex = 0 ; TCIndex  < Seats(i).TurretControllers.Num(); TCIndex++)
				{
					if  ( Seats(i).TurretControllers(TCIndex)->bIsInMotion )
					{
						bInMotion = TRUE;
						break;
					}
				}

				// Handle it
				if ( bInMotion )
				{
					if (!Seats(i).SeatMotionAudio->bWasPlaying || Seats(i).SeatMotionAudio->bFinished)
					{
						Seats(i).SeatMotionAudio->Play();
					}
				}
				else
				{
					// To avoid annoying sound-bites, we insure the sound has played for 150ms at least before stopping it
					if ( (Seats(i).SeatMotionAudio->bWasPlaying || !Seats(i).SeatMotionAudio->bFinished) && Seats(i).SeatMotionAudio->PlaybackTime > 0.15  )
					{
						Seats(i).SeatMotionAudio->Stop();
					}
				}
				Seats(i).LastWeaponRotation = CurrentRotation;
			}
		}
	}
}

static FTakeHitInfo OldLastTakeHitInfo;
static FLOAT OldHealth;

void AGearVehicle::PreNetReceive()
{
	Super::PreNetReceive();

	OldHealth = Health;
}

void AGearVehicle::PostNetReceive()
{
	Super::PostNetReceive();

	if (OldHealth != Health)
	{
		eventReceivedHealthChange(appTrunc(OldHealth));
	}
}

/** Do trace for weapon, ignoring hits between camera and vehicle, and things we don't want to aim at. */
AActor* AGearVehicle::DoTurretTrace(const FVector Start, const FVector End, INT SeatIndex, FVector& OutHitLocation)
{
	AActor* HitActor = NULL;

	FVector Pivot = GetSeatPivotPoint(SeatIndex);

	//GWorld->PersistentLineBatcher->DrawLine(AimPoint, CamLoc, FColor(255,0,0), 5.f, SDPG_World);
	FMemMark Mark(GMainThreadMemStack);
	FCheckResult* HitResult = GWorld->MultiLineCheck(GMainThreadMemStack, End, Start, FVector(0,0,0), TRACE_ProjTargets | TRACE_ComplexCollision, this );

	// Walk through results looking for one that is in front of the turret
	while(HitResult && !HitActor)
	{
		// Check we only aim at things that would block a player
		if(HitResult->Actor && !HitResult->Actor->bBlockActors)
		{
			HitResult = HitResult->GetNext();
			continue;
		}

		FVector HitLoc = HitResult->Location;

		// Dot product 'cam to hit' and 'gun to hit' to check they are in the same direction (within ~53 degrees)
		if(((HitLoc - Start).SafeNormal()|(HitLoc - Pivot).SafeNormal()) > 0.6f)
		{
			OutHitLocation = HitLoc;
			HitActor = HitResult->Actor;
		}

		HitResult = HitResult->GetNext();
	}

	Mark.Pop();

	return HitActor;
}

/**
* This function will calculate the current firing vector and create a turret rotation from it.  
*
* @param	SeatIndex	- The Seat we are calc'ing the rotation for
* @param	NewRotation	- The new Pawn Rotation.  This is ignored if there is a controlled
*/

void AGearVehicle::ApplyWeaponRotation(INT SeatIndex, FRotator NewRotation)
{
	if (Seats.IsValidIndex(SeatIndex) && Seats(SeatIndex).SeatPawn)
	{
		// @HACK - We don't want to have to replicate the entire seats array, so when we see that the 
		// vehicle has a gun, steal it if we are seat 0.
		if (SeatIndex == 0 && Weapon && !Seats(SeatIndex).Gun)
		{
			Seats(SeatIndex).Gun = Cast<AGearVehicleWeapon>(Weapon);
		}

		AController* C = Seats(SeatIndex).SeatPawn->Controller;

		Seats(SeatIndex).AimTarget = NULL;
		FVector CamLoc(0,0,0);

		if ( C )
		{
			APlayerController* PC = C->GetAPlayerController();
			FVector AimPoint;

			if ( PC )
			{
				if( !PC->eventIsLookInputIgnored() )
				{
					FRotator CamRot;
					PC->eventGetPlayerViewPoint(CamLoc, CamRot);

					FLOAT TraceRange;
					TArray<AActor*> IgnoredActors;
					if (Seats(SeatIndex).Gun != NULL)
					{
						TraceRange = Seats(SeatIndex).Gun->AimTraceRange;
						// turn off bProjTarget on Actors we should ignore for the aiming trace
						for (INT i = 0; i < Seats(SeatIndex).Gun->AimingTraceIgnoredActors.Num(); i++)
						{
							AActor* IgnoredActor = Seats(SeatIndex).Gun->AimingTraceIgnoredActors(i);
							if (IgnoredActor != NULL && IgnoredActor->bProjTarget)
							{
								IgnoredActor->bProjTarget = FALSE;
								IgnoredActors.AddItem(IgnoredActor);
							}
						}
					}
					else
					{
						TraceRange = 5000.0f;
					}

					AimPoint = CamLoc + CamRot.Vector() * TraceRange;

					FVector HitLocation = AimPoint;
					AActor* HitActor = DoTurretTrace(CamLoc, AimPoint, SeatIndex, HitLocation);

					// Cache who we are aiming at

					Seats(SeatIndex).AimPoint  = HitLocation;
					Seats(SeatIndex).AimTarget = HitActor;

					// restore bProjTarget on Actors we turned it off for
					for (INT i = 0; i < IgnoredActors.Num(); i++)
					{
						IgnoredActors(i)->bProjTarget = TRUE;
					}

					FVector Pivot = GetSeatPivotPoint(SeatIndex);
					FVector PivotTargetVec = AimPoint - Pivot;

					//GWorld->LineBatcher->DrawLine(AimPoint, Pivot, FColor(255,255,0), SDPG_World);
					NewRotation = PivotTargetVec.Rotation();

					if(!Seats(SeatIndex).bDisableOffsetZAdjust)
					{
						FLOAT PivotTargetMag = PivotTargetVec.Size();
						FLOAT AdjustAngle = ((FLOAT)PI * 0.5f) - appAcos(Seats(SeatIndex).PivotFireOffsetZ/PivotTargetMag);
						NewRotation.Pitch -= appTrunc(AdjustAngle * Rad2U);
					}
				}
			}
			else 
			{
				CamLoc = GetSeatPivotPoint(SeatIndex);
				AimPoint = C->GetFocalPoint();
				NewRotation = (AimPoint - CamLoc).Rotation();
			}

			// Set the value
			SeatWeaponRotation(SeatIndex, NewRotation, FALSE);
		}

		for (INT i=0;i<Seats(SeatIndex).TurretControllers.Num(); i++)
		{
			Seats(SeatIndex).TurretControllers(i)->DesiredBoneRotation = NewRotation;
		}
	}
}

UBOOL AGearVehicleBase::ReachedDesiredRotation()
{
	AGearVehicleWeapon* VWeap = Cast<AGearVehicleWeapon>(Weapon);
	return (VWeap != NULL) ? VWeap->eventIsAimCorrect() : Super::ReachedDesiredRotation();
}

/** 
* Returns the pivot point to use for a given turret
*
* @Param	SeatIndex	- The Seat to look up
* @returns a locational vector of the pivot point
*/
FVector AGearVehicle::GetSeatPivotPoint(INT SeatIndex)
{
	INT BarrelIndex = GetBarrelIndex(SeatIndex);
	INT ArrayLen = Seats(SeatIndex).GunPivotPoints.Num();

	if ( Mesh && ArrayLen > 0 )
	{
		if ( BarrelIndex >= ArrayLen )
		{
			BarrelIndex = ArrayLen - 1;
		}

		FName Pivot = Seats(SeatIndex).GunPivotPoints(BarrelIndex);
		return Mesh->GetBoneLocation(Pivot);
	}
	else
	{
		return Location;
	}
}

/** 
* Returns the index of the current "in use" barrel
*
* @Param	SeatIndex	- The Seat to look up
* @returns the index of the barrel that will be used for the next shot
*/
INT AGearVehicle::GetBarrelIndex(INT SeatIndex)
{
	if ( Seats(SeatIndex).GunSocket.Num() < 0 )
	{
		return 0;
	}
	else
	{
		return Seats(SeatIndex).GunSocket.Num() > 0 ? Seats(SeatIndex).BarrelIndex % Seats(SeatIndex).GunSocket.Num() : 0;
	}
}

/**
* This function is used by Vehicle Factories to force the rotation on a vehicle's turrets
*
* @param	SeatIndex	- The Seat we are calc'ing the rotation for
* @param	NewRotation	- The new Pawn Rotation.  
*/
void AGearVehicle::ForceWeaponRotation(INT SeatIndex,FRotator NewRotation)
{
	ApplyWeaponRotation(SeatIndex, NewRotation);
}

FLOAT AGearVehicle::GetGravityZ()
{
	return Super::GetGravityZ() * VehicleGravityZScale;
}

/**
*  Used by some vehicles to limit their maximum velocity
*	 @PARAM InForce is the force being applied to this vehicle from USVehicleSimBase::UpdateVehicle()
*  @RETURN damping force 
*/
FVector AGearVehicle::GetDampingForce(const FVector& InForce)
{
	checkSlow(AirSpeed > 0.f );

	FVector DampedVelocity = Velocity;
	// perhaps don't damp downward z velocity if vehicle isn't touching ground
	DampedVelocity.Z = (bNoZDamping || (bNoZDampingInAir && !HasWheelsOnGround())) ? 0.f : DampedVelocity.Z;

	return InForce.Size() * ::Min(DampedVelocity.SizeSquared()/Square(1.03f*AirSpeed), 2.f) * DampedVelocity.SafeNormal();
}

/************************************************************************************
 * UGearVehicleSimCar
 ***********************************************************************************/

//Handles in place turning of the vehicle only (not really tweaked for at speed maneuvering)
void UGearVehicleSimCar::ApplyTankSteering(ASVehicle* Vehicle, FLOAT DeltaTime)
{
	AGearVehicle* GearV = CastChecked<AGearVehicle>(Vehicle);
	check(GearV);

	FLOAT EngineTorque = Clamp<FLOAT>(Abs(TurnInPlaceThrottle * Vehicle->OutputSteering), -1.0, 1.0) * MaxEngineTorque;

	// Lose torque when climbing too steep
	FRotationMatrix R(Vehicle->Rotation);
	if ((R.GetAxis(2) | FVector(0.0f,0.0f,1.0f)) < Vehicle->WalkableFloorZ)
	{
		EngineTorque = 0.f;
	}

	//@todo Pretty sure OutputGas is never > 0 in this function
	FLOAT InsideTrackFactor = -0.5f;

	// Braking
	FLOAT BrakeTorque = Vehicle->OutputBrake * MaxBrakeTorque;
	FLOAT TotalBrakeTorque = BrakeTorque * Vehicle->NumPoweredWheels;

	// Determine how to split up the torque based on the InsideTrackTorqueCurve
	FLOAT InsideTrackTorque = EngineTorque * InsideTrackFactor;
	FLOAT OutsideTrackTorque = EngineTorque * (1.0f - Abs(InsideTrackFactor)); 

	FLOAT LeftTrackTorque, RightTrackTorque;
	if (Vehicle->OutputSteering < 0.f) // Turn Right
	{
		LeftTrackTorque = OutsideTrackTorque; 
		RightTrackTorque = InsideTrackTorque;
	}
	else // Turn Left
	{	
		LeftTrackTorque = InsideTrackTorque;
		RightTrackTorque = OutsideTrackTorque;
	}

	//debugf(TEXT("ET %f LT %f RT %f TF %f"), EngineTorque, LeftTrackTorque, RightTrackTorque, InsideTrackFactor);
	// Do model for each wheel.
	for(INT i=0; i<Vehicle->Wheels.Num(); i++)
	{
		USVehicleWheel* vw = Vehicle->Wheels(i);

		FLOAT ExtraGripScale = 1.f;

		if (vw->bPoweredWheel)
		{
			//0.25 and .5 values here came from UnTank.cpp UpdateVehicle
			vw->BrakeTorque = 0.25f * TotalBrakeTorque;
			if (vw->Side == SIDE_Left)
			{
				vw->MotorTorque = 0.5f * LeftTrackTorque;
			}
			else
			{
				vw->MotorTorque = 0.5f * RightTrackTorque;
			}
			
			// Calculate torque applied back to chassis if wheel is on the ground
			if (vw->bWheelOnGround)
				vw->ChassisTorque = -.5f * vw->MotorTorque * ChassisTorqueScale;
			else
				vw->ChassisTorque = 0.0f;
		}

#if WITH_NOVODEX // Update WheelShape in case it changed due to handbrake
		NxWheelShape* WheelShape = vw->GetNxWheelShape();
		check(WheelShape);	

		SetNxWheelShapeParams(WheelShape, vw, GearV->WheelLongGripScale * ExtraGripScale, GearV->WheelLatGripScale * ExtraGripScale);
#endif // WITH_NOVODEX


		/////////// STEERING  ///////////

		// Pass on steering to wheels that want it.
		vw->Steer = ActualSteering * vw->SteerFactor;
	}
}

void UGearVehicleSimCar::UpdateVehicle(ASVehicle* Vehicle, FLOAT DeltaTime)
{
	AGearVehicle* GearV = CastChecked<AGearVehicle>(Vehicle);

	UBOOL bDoBoost = ((Vehicle->OutputRise > 0.5f) || GearV->eventForceBoost() ) && GearV->eventCanBoost();

	/////////// STEERING ///////////
	
	FLOAT maxSteerAngle = MaxSteerAngleCurve.Eval(Vehicle->Velocity.Size(), 0.f);
	if(bDoBoost)
	{
		maxSteerAngle *= BoostMaxSteerAngleScale;
	}

	FLOAT maxSteer = DeltaTime * (bDoBoost ? BoostSteerSpeed : SteerSpeed);

	FLOAT deltaSteer;
	deltaSteer = (-Vehicle->OutputSteering * maxSteerAngle) - ActualSteering; // Amount we want to move (target - current)
	deltaSteer = Clamp<FLOAT>(deltaSteer, -maxSteer, maxSteer);
	ActualSteering += deltaSteer;

	/////////// FORCED STOP ///////////

 	if (bForceStop)
	{
		Vehicle->Throttle = 0.0f;
		Vehicle->OutputBrake = 1.0f;
	}

    //Tank steering can only be done at low speed (ie no throttle)
    //primarily for turning quick while stopped
	if (GearV->eventShouldDoTankSteer())
	{
		ApplyTankSteering(Vehicle, DeltaTime);
		return;
	}

	/////////// THROTTLE ///////////

	// Boost
	FLOAT DesiredBoost = bDoBoost ? 1.f : 0.f;
	FLOAT MaxBoostChange = BoostSpeed * DeltaTime;
	FLOAT DeltaBoost = DesiredBoost - ActualBoost;
	ActualBoost += Clamp<FLOAT>(DeltaBoost, -MaxBoostChange, MaxBoostChange);

	FLOAT TargetThrottle = (bDoBoost) ? 1.f : Vehicle->Throttle; // Force throttle to max when boosting.

	FLOAT DeltaThrottle = TargetThrottle - ActualThrottle; // Desired delta.
	FLOAT MaxDeltaThrottle = ThrottleSpeed * DeltaTime; // Amount it can change this frame.
	ActualThrottle += Clamp(DeltaThrottle, -MaxDeltaThrottle, MaxDeltaThrottle); // Update ActualThrottle.

	/////////// TORQUE CURVE APPROXIMATION //////////////

	FLOAT EffectiveForwardVel = Lerp(Vehicle->ForwardVel, Vehicle->ForwardVel/BoostAirSpeedScale, ActualBoost);

	// Braking
	FLOAT BrakeTorque = Vehicle->OutputBrake * MaxBrakeTorque;

	// Torque
	FLOAT TorqueEval = TorqueVSpeedCurve.Eval(EffectiveForwardVel, 0.0f);
	FLOAT MotorTorque = Lerp((ActualThrottle * TorqueEval), (ActualThrottle * TorqueEval * BoostTorqueScale), ActualBoost);

	//debugf(TEXT("AT: %f  VFV: %f"), ActualThrottle, Vehicle->ForwardVel);

	if ( (EffectiveForwardVel > Vehicle->AirSpeed) && ((Vehicle->Velocity | Vehicle->Rotation.Vector()) > 0.f) )
	{
		// force vehicle to slow down if above airspeed limit
		MotorTorque = 0.f;
		BrakeTorque = MaxBrakeTorque;
	}

	// Engine braking (inversely proportional to throttle).
	MotorTorque -= EngineBrakeFactor * Vehicle->ForwardVel * (1.f - Abs(ActualThrottle));

	// Lose torque when climbing too steep
	FRotationMatrix R(Vehicle->Rotation);
	if ((R.GetAxis(2) | FVector(0.0f,0.0f,1.0f)) < Vehicle->WalkableFloorZ)
		MotorTorque = 0.f;

	FLOAT TotalSpinVel = 0.0f;
	INT NumWheelsOnGround = 0;
	if (LSDFactor > 0.0f)
	{
		for(INT i=0; i<Vehicle->Wheels.Num(); i++)
		{
			USVehicleWheel* vw = Vehicle->Wheels(i);

			// Accumulate wheel spin speeds to use for LSD
			TotalSpinVel += vw->SpinVel;

			if (vw->bWheelOnGround)
				NumWheelsOnGround++;
		}
	}

	// Do model for each wheel.
	FLOAT TotalTorque = MotorTorque * Vehicle->NumPoweredWheels;
	FLOAT TotalBrakeTorque = BrakeTorque * Vehicle->NumPoweredWheels;

	// Percentage of torque applied to each wheel
	FLOAT EvenSplit;
	if (NumWheelsOnGround >= 1)
		EvenSplit = 1.0f / (FLOAT)NumWheelsOnGround;
	else
		EvenSplit = 1.0f / (FLOAT)Vehicle->NumPoweredWheels;

	// Hacked due to the fact that putting all the torque on one wheel is more powerful than splitting it evenly.
	// Apply no more than 3x normal power to a single wheel
	FLOAT MaxSplit = (1.0f / (FLOAT)Vehicle->NumPoweredWheels) * 3.0f; 

	for(INT i=0; i<Vehicle->Wheels.Num(); i++)
	{
		USVehicleWheel* vw = Vehicle->Wheels(i);

		FLOAT ExtraGripScale = 1.f;

		if (vw->bPoweredWheel)
		{
			/////////// LIMITED SLIP DIFFERENTIAL ///////////

			// Heuristic to divide torque up so that the wheels that are spinning slower get more of it.
			// Sum of LSDFactor across all wheels should be 1.
			FLOAT LSDSplit, UseSplit;

			if (LSDFactor > 0.0f)
			{	
				// If no wheels are spinning, just do an even split.
				if (TotalSpinVel > 0.1f)
				{
					if (vw->bWheelOnGround)
						LSDSplit = EvenSplit; //(TotalSpinVel - vw->SpinVel) / (((FLOAT)NumWheelsOnGround - 1.0f) * TotalSpinVel);
					else
						LSDSplit = 0.0f;
				}
				else
					LSDSplit = EvenSplit;

				UseSplit = Min(LSDSplit, MaxSplit); //((1 - LSDFactor) * EvenSplit) + (LSDFactor * LSDSplit);
			}
			else
				UseSplit = EvenSplit;

			
			vw->BrakeTorque = UseSplit * TotalBrakeTorque;
			vw->MotorTorque = UseSplit * TotalTorque;

			// Calculate torque applied back to chassis if wheel is on the ground
			if (vw->bWheelOnGround)
				vw->ChassisTorque = -1.0f * vw->MotorTorque * ChassisTorqueScale;
			else
				vw->ChassisTorque = 0.0f;

			// Additional wheel spin!
			FLOAT ExtraSpinVel = GearV->WheelExtraGraphicalSpin * ActualThrottle;
			vw->CurrentRotation += (ExtraSpinVel * DeltaTime * (180.f/(FLOAT)PI));

			if((ExtraSpinVel > vw->SpinVel) && (GearV->OutputGas > 0.9f))
			{
				ExtraGripScale = GearV->PeelOutFrictionScale;
			}
		}

#if WITH_NOVODEX // Update WheelShape in case it changed due to handbrake
		NxWheelShape* WheelShape = vw->GetNxWheelShape();
		check(WheelShape);	
		
		SetNxWheelShapeParams(WheelShape, vw, GearV->WheelLongGripScale * ExtraGripScale, GearV->WheelLatGripScale * ExtraGripScale);
#endif // WITH_NOVODEX


		/////////// STEERING  ///////////

		// Pass on steering to wheels that want it.
		vw->Steer = ActualSteering * vw->SteerFactor;
	}

	//////////////////// IN AIR LEVELLING /////////////////////////

	// If off ground, apply torque to keep vehicle level.
	if(Vehicle->bDriving)// && !Vehicle->bVehicleOnGround && !Vehicle->bChassisTouchingGround)
	{
		FVector LocalUp = Vehicle->Mesh->LocalToWorld.GetAxis(2);

		// Cross to get vector
		const FVector Cross = FVector(0,0,1) ^ LocalUp;
		const FLOAT CrossMag = Cross.Size();

		// If non-parallel..
		if(CrossMag > KINDA_SMALL_NUMBER)
		{
			// Find angle between vectors
			FLOAT LevelAngle = appAsin(CrossMag);

			const FLOAT Dot = FVector(0,0,1) | LocalUp;
			if(Dot < 0.0f)
			{
				LevelAngle = PI - LevelAngle;
			}
			// Normalize axis
			const FVector LevelAxis = Cross / CrossMag;

			FLOAT UprightTorqueMag = ::Clamp(InAirUprightTorqueFactor * LevelAngle, -InAirUprightMaxTorque, InAirUprightMaxTorque);
			Vehicle->AddTorque(UprightTorqueMag * LevelAxis);
		}

		// Apply angular velocity damping as well
		Vehicle->AddTorque(Vehicle->AngularVelocity * -InAirAngVelDamping);
	}
}

/** Returns a float representative of the vehicle's engine output. */
float UGearVehicleSimCar::GetEngineOutput(ASVehicle* Vehicle)
{
	return Abs(Vehicle->ForwardVel);
}

/** Grab the view direction as well, for LookToSteer support. */
void UGearVehicleSimCar::ProcessCarInput(ASVehicle* Vehicle)
{
	Super::ProcessCarInput(Vehicle);

	// Need this to stop braking while tank-turning
	if(Vehicle->OutputBrake > 0.f && Abs(Vehicle->OutputSteering) > KINDA_SMALL_NUMBER)
	{
		Vehicle->OutputBrake = 0.f;
	}

	Vehicle->OutputRise = Vehicle->Rise;

	if ( Vehicle->IsHumanControlled() )
	{			
		Vehicle->DriverViewPitch = Vehicle->Controller->Rotation.Pitch;
		Vehicle->DriverViewYaw = Vehicle->Controller->Rotation.Yaw;
	}
	else
	{
		Vehicle->DriverViewPitch = Vehicle->Rotation.Pitch;
		Vehicle->DriverViewYaw = Vehicle->Rotation.Yaw;
	}

	// Turn off steering when dead
	if(Vehicle->Driver == NULL)
	{
		Vehicle->OutputSteering = 0.f;
	}
}

void UGearVehicleSimCar::UpdateHandbrake(ASVehicle* Vehicle)
{
	Vehicle->bOutputHandbrake = FALSE;
}

/************************************************************************************
 * UGearVehicleSimChopper
 ***********************************************************************************/

void UGearVehicleSimChopper::ProcessCarInput(ASVehicle* Vehicle)
{
	if( !Vehicle->HasRelevantDriver() && Vehicle->Controller == NULL)
	{
		Vehicle->OutputBrake = 1.0f;
		Vehicle->OutputGas = 0.0f;
		Vehicle->bOutputHandbrake = FALSE;
		Vehicle->OutputSteering = 0.0;
	}
	else
	{
		if( Vehicle->Driver != NULL || Vehicle->Controller != NULL )
		{
			Vehicle->OutputGas = Vehicle->Throttle;
			Vehicle->OutputSteering = Vehicle->Steering;
		}
		else
		{
			Vehicle->OutputGas = 0.f;
			Vehicle->OutputSteering = 0.f;
		}

		Vehicle->OutputRise = Vehicle->Rise;

		// Keep awake physics of any driven vehicle.
		check(Vehicle->CollisionComponent);
		Vehicle->CollisionComponent->WakeRigidBody();
	}

	if( Vehicle->Controller)
	{
		if( Vehicle->IsHumanControlled() )
		{			
			Vehicle->DriverViewPitch = Vehicle->Controller->Rotation.Pitch;
			Vehicle->DriverViewYaw = Vehicle->Controller->Rotation.Yaw;
		}
		else
		{
			FRotator ViewRot = (Vehicle->Controller->GetFocalPoint() - Vehicle->Location).Rotation();
			Vehicle->DriverViewPitch = ViewRot.Pitch;
			Vehicle->DriverViewYaw = ViewRot.Yaw;
		}
	}
}

FLOAT UGearVehicleSimChopper::GetEngineOutput(ASVehicle* Vehicle)
{
	return ((Vehicle->Velocity.Size())/(Vehicle->MaxSpeed))*5000;
}

/**
 * Returns "forward" rotation used as frame of reference for applying vehicle forces
 */
void UGearVehicleSimChopper::GetRotationAxes(ASVehicle* Vehicle, FVector &DirX, FVector &DirY, FVector &DirZ)
{
	FRotationMatrix R(Vehicle->Rotation);
	DirX = R.GetAxis(0);
	DirY = R.GetAxis(1);
	DirZ = R.GetAxis(2);
}

void UGearVehicleSimChopper::UpdateVehicle(ASVehicle* Vehicle, FLOAT DeltaTime)
{
	if( Vehicle->bDriving )
	{
		// OutputSteering is actually a strafe value
		FLOAT OutputStrafe = Vehicle->OutputSteering;
		FLOAT OutputThrust = Vehicle->OutputGas;
		FLOAT OutputRise = Vehicle->OutputRise;

		// Zero force/torque accumulation.
		FVector Up(0.0f, 0.0f, 1.0f);
		FVector Force(0.0f, 0.0f, 0.0f);
		FVector Torque(0.0f, 0.0f, 0.0f);

		// Calc up (z), right(y) and forward (x) vectors
		FVector DirX, DirY, DirZ;
		GetRotationAxes(Vehicle, DirX, DirY, DirZ);

		// 'World plane' forward & right vectors ie. no z component.
		FVector Forward = DirX;
		if (!bAllowZThrust)
			Forward.Z = 0.0f;
		Forward.Normalize();

		FVector Right = DirY;
		if (!bAllowZThrust)
			Right.Z = 0.0f;
		Right.Normalize();

		// Get body angular velocity
		FRigidBodyState rbState(0);
		Vehicle->GetCurrentRBState(rbState);
		FVector AngVel(rbState.AngVel.X, rbState.AngVel.Y, rbState.AngVel.Z);
		FLOAT TurnAngVel = AngVel | Up;
		FLOAT RollAngVel = AngVel | DirX;
		FLOAT PitchAngVel = AngVel | DirY;

		FLOAT ForwardVelMag = Vehicle->Velocity | Forward;
		FLOAT RightVelMag = Vehicle->Velocity | Right;
		FLOAT UpVelMag = Vehicle->Velocity | Up;

		if ( bStabilizeStops )
		{
			Force += StabilizationForce(Vehicle, DeltaTime, ((OutputThrust == 0.f) && (OutputStrafe == 0.f)) );
		}

		// Thrust
		if ( bFullThrustOnDirectionChange && ((Vehicle->Velocity | (OutputThrust*Forward)) < 0.f) )
			Force += OutputThrust * ::Max(DirectionChangeForce, MaxThrustForce) * Forward;
		else if ( OutputThrust < 0.f )
			Force += OutputThrust * MaxReverseForce * Forward;
		else
			Force += OutputThrust * MaxThrustForce * Forward;

		// Strafe
		if ( bFullThrustOnDirectionChange && ((Vehicle->Velocity | OutputStrafe*Right) > 0.f) )
			Force -= OutputStrafe * ::Max(DirectionChangeForce, MaxThrustForce) * Right;
		else
			Force -= OutputStrafe * MaxStrafeForce * Right;

		Force -= (1.0f - Abs(OutputThrust)) * LongDamping * ForwardVelMag * Forward;
		Force -= (1.0f - Abs(OutputStrafe)) * LatDamping * RightVelMag * Right;

		// Rise
		AccumulatedTime += DeltaTime;
		UBOOL bAddRandForce = FALSE;
		if (AccumulatedTime > RandForceInterval)
		{
			AccumulatedTime = 0.0f;
			bAddRandForce = TRUE;
		}
		if( Vehicle->Location.Z > Vehicle->WorldInfo->StallZ )
		{
			Force += Up * ::Min(-1.f*UpDamping*UpVelMag, OutputRise*MaxRiseForce);
		}
		else if ( OutputRise == 0.f ) // If not pushing up or down, apply vertical damping and small perturbation force.
		{
			Force -= (UpDamping * UpVelMag * Up);

			if ( bAddRandForce && (Vehicle->Role == ROLE_Authority) )
			{
				RandForce.X = 2 * (appFrand() - 0.5) * MaxRandForce;
				RandForce.Y = 2 * (appFrand() - 0.5) * MaxRandForce;
				RandForce.Z = 2 * (appFrand() - 0.5) * MaxRandForce;
				RandTorque.X = 2 * (appFrand() - 0.5) * MaxRandForce;
				RandTorque.Y = 2 * (appFrand() - 0.5) * MaxRandForce;
				RandTorque.Z = 2 * (appFrand() - 0.5) * MaxRandForce;

				Force += RandForce;
				Torque += RandTorque;
			}
		}
		else
		{
			Force += OutputRise * MaxRiseForce * Up;
		}

		FRotator LookRot = FRotator(Vehicle->DriverViewPitch, Vehicle->DriverViewYaw, 0);
		FVector LookDir = LookRot.Vector();

		// Try to turn the helicopter to match the way the camera is facing.

		//// YAW ////
		FLOAT CurrentHeading = HeadingAngle(Forward);
		FLOAT DesiredHeading = HeadingAngle(LookDir);

		if ( !bHeadingInitialized )
		{
			TargetHeading = CurrentHeading;
			bHeadingInitialized = TRUE;
		}		

		// Move 'target heading' towards 'desired heading' as fast as MaxYawRate allows.
		FLOAT DeltaTargetHeading = FindDeltaAngle(TargetHeading, DesiredHeading);
		FLOAT MaxDeltaHeading = DeltaTime * MaxYawRate;
		DeltaTargetHeading = Clamp<FLOAT>(DeltaTargetHeading, -MaxDeltaHeading, MaxDeltaHeading);
		TargetHeading = UnwindHeading(TargetHeading + DeltaTargetHeading);
		
		// Then put a 'spring' on the copter to target heading.
		FLOAT DeltaHeading = FindDeltaAngle(CurrentHeading, TargetHeading);
		FLOAT TurnTorqueMag = (DeltaHeading / PI) * TurnTorqueFactor;
		//debugf(TEXT("TurnTorqueMag: %.2f"), TurnTorqueMag);
		TurnTorqueMag = Clamp<FLOAT>( TurnTorqueMag, -TurnTorqueMax, TurnTorqueMax );
		Torque += TurnTorqueMag * Up;

		//// ROLL ////
		// Add roll torque about local X vector as helicopter turns.
		FLOAT RollTorqueMag = ( (-DeltaHeading / PI) * RollTorqueTurnFactor ) + ( OutputStrafe * RollTorqueStrafeFactor );
		RollTorqueMag = Clamp<FLOAT>( RollTorqueMag, -RollTorqueMax, RollTorqueMax );
		Torque += ( RollTorqueMag * DirX );

		//// PITCH ////
		FLOAT PitchTorqueMag = OutputThrust * PitchTorqueFactor;
		PitchTorqueMag = Clamp<FLOAT>( PitchTorqueMag, -PitchTorqueMax, PitchTorqueMax );
		Torque += PitchTorqueMag * DirY ;
		Torque += (Vehicle->Rotation.Vector().Z - LookDir.Z)*PitchViewCorrelation*DirY;
		
		FLOAT ActualTurnDamping;
		if (bStrafeAffectsTurnDamping && OutputStrafe != 0.0)
		{
			ActualTurnDamping = StrafeTurnDamping;
		}
		else
		{
			ActualTurnDamping = TurnDamping;
		}

		// Steer (yaw) damping
		Torque -= TurnAngVel * ActualTurnDamping * Up;

		// Roll damping
		Torque -= RollAngVel * RollDamping * DirX;

		// Pitch damping
		Torque -= PitchAngVel * PitchDamping * DirY;

		// velocity damping to limit airspeed
		Force -= Vehicle->GetDampingForce(Force);

		// Apply a hard limit to the max velocity if desired.
		if(HardLimitAirSpeedScale > 0.f)
		{
			FLOAT VMag = Vehicle->Velocity.Size();
			if(VMag > (Vehicle->AirSpeed * HardLimitAirSpeedScale))
			{
				FVector NewVel = (Vehicle->Velocity/VMag) * (Vehicle->AirSpeed * HardLimitAirSpeedScale);
				Vehicle->Mesh->SetRBLinearVelocity(NewVel, FALSE);
			}
		}

		// Apply force/torque to body.
		Vehicle->AddForce( Force );
		Vehicle->AddTorque( Torque );

		for(INT i=0; i<Vehicle->Wheels.Num(); i++)
		{
			USVehicleWheel* vw = Vehicle->Wheels(i);
			vw->BrakeTorque = 0.f;
		}
	}
	else if ( bStabilizeStops )
	{
		for(INT i=0; i<Vehicle->Wheels.Num(); i++)
		{
			USVehicleWheel* vw = Vehicle->Wheels(i);
			vw->BrakeTorque = StoppedBrakeTorque;
		}

		// Apply stabilization force to body.
		FVector StabForce = StabilizationForce(Vehicle, DeltaTime, TRUE);
		if(StabForce.SizeSquared() > 0.5f * 0.5f)
		{
			Vehicle->AddForce(StabForce);
		}

		// when no driver, also damp rotation
		FVector StabTorque = StabilizationTorque(Vehicle, DeltaTime, TRUE);
		if(StabTorque.SizeSquared() > 0.5 * 0.5f)
		{
			Vehicle->AddTorque(StabTorque);
		}
	}
}

FVector UGearVehicleSimChopper::StabilizationForce(ASVehicle* Vehicle, FLOAT DeltaTime, UBOOL bShouldStabilize)
{
	FVector VehicleVelocity = Vehicle->Velocity;

	if ( !bAllowZThrust )
		VehicleVelocity.Z = 0.f;

	if(bShouldStabilize)
	{
		return VehicleVelocity * -StabilizationForceMultiplier;
	}
	else
	{
		return FVector(0,0,0);
	}
}

FVector UGearVehicleSimChopper::StabilizationTorque(ASVehicle* Vehicle, FLOAT DeltaTime, UBOOL bShouldStabilize)
{
		// Calc up (z) vector
		FRotationMatrix R(Vehicle->Rotation);
		FVector DirZ = R.GetAxis(2);

		// Steer (yaw) damping
		// Get body angular velocity
		FRigidBodyState rbState(0);
		Vehicle->GetCurrentRBState(rbState);
		FVector AngVel(rbState.AngVel.X, rbState.AngVel.Y, rbState.AngVel.Z);
		FLOAT TurnAngVel = AngVel | DirZ;
		return -1.f * TurnAngVel * TurnDamping * DirZ;
}


/************************************************************************************
 * UGearVehicleSimHover
 ***********************************************************************************/

FLOAT UGearVehicleSimHover::GetEngineOutput(ASVehicle* Vehicle)
{
	return ((Vehicle->Velocity.Size())/(Vehicle->MaxSpeed))*5000;
}

/**
  * if bCanClimbSlopes and on ground, "forward" depends on the slope
  */
void UGearVehicleSimHover::GetRotationAxes(ASVehicle* Vehicle, FVector &DirX, FVector &DirY, FVector &DirZ)
{
	FRotationMatrix R(Vehicle->Rotation);
	DirX = R.GetAxis(0);
	DirY = R.GetAxis(1);
	DirZ = R.GetAxis(2);
	if ( !bCanClimbSlopes || !Vehicle->bVehicleOnGround )
	{
		return;
	}

	FVector NormalSum(0.f, 0.f, 0.f);
	for(INT i=0; i<Vehicle->Wheels.Num(); i++)
	{
#if WITH_NOVODEX
		USVehicleWheel* vw = Vehicle->Wheels(i);
		check(vw);
		NxWheelShape* WheelShape = vw->GetNxWheelShape();
		check(WheelShape);

		if ( vw->bWheelOnGround )
		{
			NormalSum += vw->ContactNormal;
		}
#endif // WITH_NOVODEX

	}

	if ( NormalSum.IsZero() )
	{
		return;
	}
	// Calc up (z), right(y) and forward (x) vectors
	NormalSum.Normalize();

	DirX = DirX - (DirX | NormalSum) * NormalSum;
	DirY = DirY - (DirY | NormalSum) * NormalSum;
	DirZ = DirZ - (DirZ | NormalSum) * NormalSum;
}

void UGearVehicleSimHover::UpdateVehicle(ASVehicle* Vehicle, FLOAT DeltaTime)
{
	// handle outputrise differently from chopper
	Vehicle->OutputRise = 0.f;

	if ( bDisableWheelsWhenOff )
	{
		// set wheel collision based on whether have a driver
		if ( Vehicle->bDriving && !bUnPoweredDriving )
		{
			if ( !bRepulsorCollisionEnabled && (Vehicle->Wheels.Num() > 0) )
			{
				for ( INT i=0; i<Vehicle->Wheels.Num(); i++ )
				{
					Vehicle->SetWheelCollision( i, TRUE );
					Vehicle->Wheels(i)->BrakeTorque = StoppedBrakeTorque;
				}

				bRepulsorCollisionEnabled = TRUE;
			}
		}
		else if ( bRepulsorCollisionEnabled )
		{
			bRepulsorCollisionEnabled = FALSE;
			for ( INT i=0; i<Vehicle->Wheels.Num(); i++ )
			{
				Vehicle->SetWheelCollision(i,FALSE);
				Vehicle->Wheels(i)->BrakeTorque = 0.f;
			}
		}
	}	

#if WITH_NOVODEX // Update WheelShape in case it changed due to handbrake or parking
	for(INT i=0; i<Vehicle->Wheels.Num(); i++)
	{
		USVehicleWheel* vw = Vehicle->Wheels(i);

		if ( Vehicle->bUpdateWheelShapes )
		{
			NxWheelShape* WheelShape = vw->GetNxWheelShape();
			check(WheelShape);	
			SetNxWheelShapeParams(WheelShape, vw);
		}

		// Stop wheels when not driven (lets vehicle go to sleep).
		if(Vehicle->bDriving)
		{
			vw->BrakeTorque = 0.f;
		}
		else
		{
			vw->BrakeTorque = StoppedBrakeTorque;
		}
	}
	Vehicle->bUpdateWheelShapes = FALSE;

#endif // WITH_NOVODEX

	Super::UpdateVehicle(Vehicle, DeltaTime);
}


UBOOL AGearVehicle::JumpOutCheck(AActor *GoalActor, FLOAT Distance, FLOAT ZDiff)
{
	if ( GoalActor && (ZDiff > -500.f) && (WorldInfo->TimeSeconds - LastJumpOutCheck > 1.f) )
	{
		FLOAT GoalRadius, GoalHeight;
		GoalActor->GetBoundingCylinder(GoalRadius, GoalHeight);
		if ( Distance < ::Min(2.f*GoalRadius,ObjectiveGetOutDist) )
		{
			LastJumpOutCheck = WorldInfo->TimeSeconds;
			eventJumpOutCheck();
			return (Controller == NULL);
		}
	}
	return FALSE;
}

UBOOL AGearVehicle::ReachThresholdTest(const FVector& TestPosition, const FVector& Dest, AActor* GoalActor, FLOAT UpThresholdAdjust, FLOAT DownThresholdAdjust, FLOAT ThresholdAdjust)
{
	// give rigid body vehicle more leeway if going to navigation node we don't need to touch (allow anywhere inside current path radius)
	if (Physics == PHYS_RigidBody && Controller != NULL && Controller->CurrentPath != NULL)
	{
		ANavigationPoint* Nav = Cast<ANavigationPoint>(GoalActor);
		if (Nav != NULL && (!Nav->bMustTouchToReach || !Nav->bCollideActors || (!bCanPickupInventory && Nav->GetAPickupFactory() != NULL)))
		{
			FLOAT AdditionalThreshold = FLOAT(Controller->CurrentPath->CollisionRadius);
			if (Controller->NextRoutePath != NULL)
			{
				AdditionalThreshold = Min<FLOAT>(AdditionalThreshold, FLOAT(Controller->NextRoutePath->CollisionRadius));
			}
			if (AdditionalThreshold > CylinderComponent->CollisionRadius)
			{
				ThresholdAdjust += AdditionalThreshold - CylinderComponent->CollisionRadius;
			}
		}
	}

	DownThresholdAdjust += ExtraReachDownThreshold;

	return Super::ReachThresholdTest(TestPosition, Dest, GoalActor, UpThresholdAdjust, DownThresholdAdjust, ThresholdAdjust);
}

FLOAT AGearVehicle::GetMaxRiseForce()
{
	UGearVehicleSimChopper* SimChopper = Cast<UGearVehicleSimChopper>(SimObj);
	if ( SimChopper )
		return SimChopper->MaxRiseForce;
	return 100.f;
}

void AGearVehicle::GetBarrelLocationAndRotation(INT SeatIndex,FVector& SocketLocation,FRotator* SocketRotation)
{
	if (SeatIndex >= 0 && SeatIndex < Seats.Num() && Seats(SeatIndex).GunSocket.Num()>0)
	{
		Mesh->GetSocketWorldLocationAndRotation(Seats(SeatIndex).GunSocket(GetBarrelIndex(SeatIndex)), SocketLocation, SocketRotation);
	}
	else
	{
		SocketLocation = Location;
		if (SocketRotation != NULL)
		{
			*SocketRotation = Rotation;
		}
	}
}

FVector AGearVehicle::GetEffectLocation(INT SeatIndex)
{
	FVector SocketLocation;

	if ( SeatIndex < 0 || SeatIndex >= Seats.Num() || Seats(SeatIndex).GunSocket.Num() == 0 )
		return Location;

	GetBarrelLocationAndRotation(SeatIndex,SocketLocation);
	return SocketLocation;
}

FVector AGearVehicle::GetPhysicalFireStartLoc(AGearWeapon* ForWeapon)
{
	AGearVehicleWeapon* VWeap;

	VWeap = Cast<AGearVehicleWeapon>(ForWeapon);
	if ( VWeap != NULL )
	{
		return GetEffectLocation(VWeap->SeatIndex);
	}
	else
		return Location;
}

FVector AGearWeaponPawn::GetTargetLocation(AActor* RequestedBy, UBOOL bRequestAlternateLoc) const
{
	return (MyVehicle != NULL) ? (MyVehicle->GetTargetLocation(RequestedBy) + TargetLocationAdjustment) : Super::GetTargetLocation(RequestedBy);
}

AVehicle* AGearWeaponPawn::GetVehicleBase()
{
	return MyVehicle;
}

void AGearWeaponPawn::TickSpecial( FLOAT DeltaSeconds )
{
	if (Controller != NULL && MyVehicle != NULL)
	{
		AGearPC* const WPC = Cast<AGearPC>(Controller);
		FRotator Rot = WPC ? WPC->Rotation : MyVehicle->Rotation;
		MyVehicle->ApplyWeaponRotation(MySeatIndex, Rot);
	}
}


INT* AGearVehicleWeapon::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	Ptr = Super::GetOptimizedRepList(Recent,Retire,Ptr,Map,Channel);

	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);	
	if (bNetInitial && bNetOwner)
	{
		DOREP(GearVehicleWeapon,SeatIndex);
		DOREP(GearVehicleWeapon,MyVehicle);
	}

	return Ptr;
}

FVector AGearVehicleWeapon::GetPhysicalFireStartLoc(FVector AimDir)
{
	if ( MyVehicle )
		return MyVehicle->eventGetPhysicalFireStartLoc(this);
	else
		return Location;
}

FLOAT AGearVehicleWeapon::GetMaxFinalAimAdjustment()
{
	return MaxFinalAimAdjustment;
}

INT* AGearWeaponPawn::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	Ptr = Super::GetOptimizedRepList(Recent,Retire,Ptr,Map,Channel);

	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);	
	if (bNetDirty)
	{
		DOREP(GearWeaponPawn,MySeatIndex);
		DOREP(GearWeaponPawn,MyVehicle);
		DOREP(GearWeaponPawn,MyVehicleWeapon);
	}

	return Ptr;
}

INT* AGearVehicle::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	Ptr = Super::GetOptimizedRepList(Recent,Retire,Ptr,Map,Channel);

	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);	
	if (bNetDirty)
	{
		if ( !bNetOwner )
		{
			DOREP(GearVehicle,WeaponRotation);
		}
		DOREP(GearVehicle,bDeadVehicle);
		DOREP(GearVehicle,SeatMask);
		DOREP(GearVehicle,PassengerPRI);
	}
	return Ptr;
}

UBOOL AGearVehicle::ShouldClamp()
{
	return TRUE;
}

FRotator AGearVehicle::GetClampedViewRotation()
{
	FRotator ViewRotation, ControlRotation, MaxDelta;
	AGearVehicleWeapon* VWeap;

	VWeap = Cast<AGearVehicleWeapon>(Weapon);
	if (VWeap && ShouldClamp() && Controller)
	{
		// clamp view yaw so that it doesn't exceed how far the vehicle can aim on console
		ViewRotation.Yaw = Controller->Rotation.Yaw;
		MaxDelta.Pitch = 0;
		MaxDelta.Roll = 0;
		MaxDelta.Yaw = appTrunc(appAcos(VWeap->GetMaxFinalAimAdjustment()) * 180.0f / PI * 182.0444f);
		if (!ClampRotation(ViewRotation, Rotation, MaxDelta, MaxDelta))
		{
			// prevent the controller's rotation from diverging too much from the actual view rotation
			ControlRotation.Yaw = Controller->Rotation.Yaw;
			if (!ClampRotation(ControlRotation, ViewRotation, FRotator(0,16384,0), FRotator(0,16384,0)))
			{
				ControlRotation.Pitch = Controller->Rotation.Pitch;
				ControlRotation.Roll = Controller->Rotation.Roll;
				Controller->SetRotation(ControlRotation);
			}
		}

		ViewRotation.Pitch = Controller->Rotation.Pitch;
		ViewRotation.Roll = Controller->Rotation.Roll;
		return ViewRotation;
	}
	else
	{
		return Super::GetViewRotation();
	}
}
FRotator AGearVehicle::GetViewRotation()
{
	if(Cast<APlayerController>(Controller))
	{
		if (bIsConsoleTurning && !bSeparateTurretFocus)
		{
			LastClampedViewRot = GetClampedViewRotation();
		}
		else
		{
			LastClampedViewRot = Super::GetViewRotation();
		}
	}

	return LastClampedViewRot;
}

void AVehicle_Centaur_Base::OnRigidBodyCollision(const FRigidBodyCollisionInfo& Info0, const FRigidBodyCollisionInfo& Info1, const FCollisionImpactData& RigidCollisionData)
{
	Super::OnRigidBodyCollision(Info0, Info1, RigidCollisionData);

	AActor* OtherActor = (Info0.Actor != this) ? Info0.Actor : Info1.Actor;
	AGearVehicle* OtherGearV = Cast<AGearVehicle>(OtherActor);
	if(OtherGearV && (OtherGearV != this))
	{
		eventHitOtherGearVehicle(OtherGearV);
	}
}


void AVehicle_Centaur_Base::ApplyWeaponRotation(INT SeatIndex, FRotator NewRotation)
{
	check(Seats.Num() == 2);

	// Need to do different things on client and server, as client does not have both PCs
	UBOOL bApplyRot = FALSE;
	if(Role == ROLE_Authority)
	{
		// On server - fire if we are gunner, or the gunner is not there, or gunner is AI
		bApplyRot = ( SeatIndex == 1 || !Seats(1).SeatPawn || !Seats(1).SeatPawn->Controller || !Seats(1).SeatPawn->Controller->IsA(APlayerController::StaticClass()));
	}
	else
	{
		// On client - fire if we are gunner, or no-one is on second seat.
		// If we are client- we can only be Dom
		bApplyRot = ( SeatIndex == 1 || ((SeatMask & 2) == 0) );
	}

	if (bApplyRot)
	{
		Super::ApplyWeaponRotation(SeatIndex,NewRotation);
	}
}

static inline FLOAT GetMappedRangeValue(FVector2D const& InputRange, FVector2D const& OutputRange, FLOAT Value)
{
	FLOAT const ClampedPct = Clamp<FLOAT>(GetRangePct(InputRange.X, InputRange.Y, Value), 0.f, 1.f);
	return Lerp(OutputRange.X,OutputRange.Y,ClampedPct);
}


void AVehicle_Centaur_Base::TickSpecial(FLOAT DeltaSeconds)
{
	Super::TickSpecial(DeltaSeconds);

	// Look for nearby crowd members and push/kill them
	UBOOL bSquishCrowd = Velocity.SizeSquared() > (KillCrowdVel*KillCrowdVel);
	UBOOL bSquishedSomething = FALSE;

	FMemMark Mark(GMainThreadMemStack);
	FCheckResult* Link = GWorld->Hash->ActorOverlapCheck(GMainThreadMemStack, this, Location, Max(PushCrowdRadius,KillCrowdRadius));
	for( FCheckResult* result=Link; result; result=result->GetNext())
	{
		// Look for nearby agents
		ACrowdAgent* FlockActor = Cast<ACrowdAgent>(result->Actor);
		if(FlockActor)
		{
			// Find direction and dist from car to agent
			FVector ToAgent = FlockActor->Location - Location;
			FLOAT AgentDistSqr = ToAgent.SizeSquared();

			// If we are moving and close enough - kill
			if(bSquishCrowd && (AgentDistSqr < (KillCrowdRadius*KillCrowdRadius)))
			{
				bSquishedSomething = TRUE;
				FlockActor->PlayDeath();
				FlockActor->SetCollision(FALSE, FALSE, FALSE);
			}
			// If not, see if close enough to push
			else if(AgentDistSqr < (PushCrowdRadius*PushCrowdRadius))
			{
				FVector PushDir = ToAgent.SafeNormal();
				PushDir.Z = 0.f;
				FlockActor->ExternalForce = PushDir * PushCrowdForce;
				//GWorld->LineBatcher->DrawLine(FlockActor->Location, FlockActor->Location+10*FlockActor->ExternalForce, FColor(0,0,255), SDPG_World);
			}
		}
	}
	Mark.Pop();

	if (bSquishedSomething)
	{
		eventSquishedSomething();
	}

	// Don't get moved by explosions on ice
	Mesh->bIgnoreRadialImpulse = (CentaurSurface == ECOS_Ice);

	UGearVehicleSimCar* SimCar = CastChecked<UGearVehicleSimCar>(SimObj);

	// See if we are currently boosting
	UBOOL bNewBoosting = (SimCar->ActualBoost > KINDA_SMALL_NUMBER);

	// See if we have just started boosting
	if(!bIsBoosting && bNewBoosting)
	{
		eventStartBoost();
	}
	// See if we have just stopped boosting
	else if(bIsBoosting && !bNewBoosting)
	{
		eventEndBoost();
	}
	bIsBoosting = bNewBoosting;

	// Update fuel status (on server)
	if(Role == ROLE_Authority)
	{
		// If boosting, reduce fuel
		if(bIsBoosting)
		{
			BoostFuel = Max(BoostFuel - (DeltaSeconds * BoostUseRate), 0.f);
		}
		// If not boosting and not holding boost button, recharge
		else if(OutputRise < KINDA_SMALL_NUMBER)
		{
			BoostFuel = Min(BoostFuel + (DeltaSeconds * BoostRechargeRate), MaxBoostFuel);
		}
	}

	// Calc extra wheel vel we are applying
	FLOAT ExtraSpinVel = 0.f;
	UGearVehicleSimCar* GearCarObj = Cast<UGearVehicleSimCar>(SimObj);
	if(GearCarObj)
	{
		ExtraSpinVel = WheelExtraGraphicalSpin * GearCarObj->ActualThrottle;
	}

	check(Wheels.Num() == 4);

	// If its more than doubling the current wheel vel, we are 'peeling out'!
	USVehicleWheel* RearLeftVW = Wheels(1);
	USVehicleWheel* RearRightVW = Wheels(0);

	// If wheels are not moving that fast, but the extra graphics spin is making them go quickly - 'peel out'
	UBOOL bPeelingOut = ((ExtraSpinVel > RearLeftVW->SpinVel) || (ExtraSpinVel > RearRightVW->SpinVel)) && (Abs(RearLeftVW->SpinVel) < PeelOutMaxWheelVel && Abs(RearRightVW->SpinVel) < PeelOutMaxWheelVel) && (OutputGas > 0.9f);

	// RR, RL, RF, LF (same as Wheels array)
	FVector WheelTranslation[4];

	// If we are going to want it, calculate wheel effect translation (actor space)
	if(bPeelingOut || bIsBoosting)
	{
		// Get scale and transform for vehicle
		FVector TotalScale = Mesh->Scale * DrawScale * Mesh->Scale3D * DrawScale3D;

		USkeletalMesh* SkelMesh = Mesh->SkeletalMesh;
		FMatrix ActorToWorld = LocalToWorld();
		ActorToWorld.RemoveScaling();

		FMatrix CompToWorld = Mesh->LocalToWorld;
		CompToWorld.RemoveScaling();

		FMatrix CompToActor = CompToWorld * ActorToWorld.Inverse();

		for(INT i=0; i<4; i++)
		{
			WheelTranslation[i] = CompToActor.TransformFVector(Wheels(i)->WheelPosition) - FVector(0.f, 0.f, Wheels(i)->WheelRadius);
			WheelTranslation[i] /= TotalScale;
		}
	}
	else
	{
		for(INT i=0; i<4; i++)
		{
			WheelTranslation[i] = FVector(0,0,0);
		}
	}

	// Enable boost wheel effects if desired
	if(bIsBoosting)
	{
		for(INT i=0; i<2; i++)
		{
			if(!PSC_BoostWheelEffect[i]->bIsActive)
			{
				PSC_BoostWheelEffect[i]->SetActive(TRUE);
			}
		}

		// Update translation and params - back left and back right
		for(INT i=0; i<2; i++)
		{
			PSC_BoostWheelEffect[i]->Translation = WheelTranslation[i];
		}

		if (MIC_VehicleSkin)
		{
			//Boost heat!
			FLOAT TimeSinceBoost = Clamp(WorldInfo->TimeSeconds - LastBoostTime, 0.0f, 1.0f);
			MIC_VehicleSkin->SetScalarParameterValue(BoostHeatEffectName, TimeSinceBoost);
		}
	}
	else
	{
		// Disable effects
		for(INT i=0; i<2; i++)
		{
			if(PSC_BoostWheelEffect[i]->bIsActive)
			{
				PSC_BoostWheelEffect[i]->SetActive(FALSE);
			}
		}
			
		if (MIC_VehicleSkin)
		{
			//Boost cooldown
			FLOAT TimeLeftToNextBoost = Clamp(NextBoostTime - WorldInfo->TimeSeconds, 0.0f, 1.0f);
			MIC_VehicleSkin->SetScalarParameterValue(BoostHeatEffectName, TimeLeftToNextBoost);
		}

	}
	
	// Turn on particle effects when 'peeling out'
	if(bPeelingOut)
	{
		// Enable effects
		if(!bPeelOutEffectsActive)
		{
			bPeelOutEffectsActive = TRUE;

			// Call to script when beginning peel out (not through boost)
			eventOnPeelOutBegin();
		}

		for(INT i=0; i<4; i++)
		{
			if(!PSC_PeelOutEffect[i]->bIsActive)
			{
				PSC_PeelOutEffect[i]->SetActive(TRUE);
			}
		}

		// Update translation and params
		for(INT i=0; i<4; i++)
		{
			PSC_PeelOutEffect[i]->Translation = WheelTranslation[i];
			PSC_PeelOutEffect[i]->SetFloatParameter(RearLeftVW->SlipParticleParamName, 1.f);
		}
	}
	else
	{
		bPeelOutEffectsActive = FALSE;

		// Disable effects
		for(INT i=0; i<4; i++)
		{
			if(PSC_PeelOutEffect[i]->bIsActive)
			{
				PSC_PeelOutEffect[i]->SetActive(FALSE);
			}
		}
	}

	//
	// update engine audio
	//

	// ambient loop fades volume up/down with velocity, but no pitch bend
	if (EnginePlayerAmbientLoopAC)
	{
		// for debugging, editactor kills this playing for somereason
		if (!EnginePlayerAmbientLoopAC->IsPlaying())
		{
			EnginePlayerAmbientLoopAC->Play();
		}
		FLOAT const Vel = Velocity.Size2D();
		EnginePlayerAmbientLoopAC->VolumeMultiplier = GetMappedRangeValue(EnginePlayerAmbientVelocityRange, EnginePlayerAmbientVolumeRange, Vel);
	}

	static INT TESTTTTT=3;
	FLOAT CurrentRPM = SimObj->GetEngineOutput(this);

	// puts torque and vel-based rpms on similar scale
	
	// converts velocity to torque-similar units.  40 is the peak torque we see
	// in practice, and 1100 is the top speed.  precision isn't important here, 
	// being close enough to sound good is
	static const FLOAT VELFUDGE = 40.f / 1100.f;
	static const FLOAT TORQUEWEIGHT = 0.5f;

	// using a combination of torque and velocity as "rpm" in this case
	// pick max torque, since spinning in place, one side will have neg
	CurrentRPM = Max<FLOAT>(Wheels(0)->MotorTorque, Wheels(1)->MotorTorque) * TORQUEWEIGHT;
	CurrentRPM += ( SimObj->GetEngineOutput(this) * VELFUDGE * (1.f - TORQUEWEIGHT) );

	DebugLastRPM = CurrentRPM;

	static FLOAT RPMInterpSpeed = 10.f;
	if (CurrentEngineLoopAC)
	{
		// for debugging, editactor kills this playing for some reason
		if (!CurrentEngineLoopAC->IsPlaying())
		{
			CurrentEngineLoopAC->Play();
		}
		CurrentEngineLoopAC->PitchMultiplier = FInterpTo(CurrentEngineLoopAC->PitchMultiplier, GetMappedRangeValue(CentaurGear.PitchRPMRange, CentaurGear.PitchRange, CurrentRPM), DeltaSeconds, RPMInterpSpeed);
		CurrentEngineLoopAC->VolumeMultiplier = FInterpTo(CurrentEngineLoopAC->VolumeMultiplier, GetMappedRangeValue(CentaurGear.VolumeRPMRange, CentaurGear.VolumeRange, CurrentRPM), DeltaSeconds, RPMInterpSpeed);
	}

	if ( !bVehicleOnGround && (TimeOffGround > 1.f) )
	{
		bPlayLandSoundUponLanding = TRUE;
	}
	if (bPlayLandSoundUponLanding && bVehicleOnGround)
	{
		// just landed
		eventCentaurPlayLocalSound(CentaurLandSound);
		bPlayLandSoundUponLanding = FALSE;
	}

	// turret rotation sound
	if (CentaurTurretRotationAC)
	{
		INT MaxDeltaYaw = 0;
		for (INT i=0;i < Seats.Num(); i++)
		{
			FRotator CurrentRotation = SeatWeaponRotation(i, FRotator(0,0,0), TRUE); 
			
			FRotator DeltaRotation = (CurrentRotation - Seats(i).LastWeaponRotation).GetNormalized();
			MaxDeltaYaw = Max<INT>(MaxDeltaYaw, Abs<INT>(DeltaRotation.Yaw));

			Seats(i).LastWeaponRotation = CurrentRotation;
		}

		FLOAT RotRate = FLOAT(MaxDeltaYaw) / DeltaSeconds;

		
		static FLOAT TurretAudioInterpSpeed = 10.f;
		FLOAT NewVol = GetMappedRangeValue(CentaurTurretRotationVolumeVelRange, CentaurTurretRotationVolumeRange, RotRate);
		FLOAT NewPitch = GetMappedRangeValue(CentaurTurretRotationPitchVelRange, CentaurTurretRotationPitchRange, RotRate);
		CentaurTurretRotationAC->VolumeMultiplier = FInterpTo(CentaurTurretRotationAC->VolumeMultiplier, NewVol, DeltaSeconds, TurretAudioInterpSpeed);
		CentaurTurretRotationAC->PitchMultiplier = FInterpTo(CentaurTurretRotationAC->PitchMultiplier, NewPitch, DeltaSeconds, TurretAudioInterpSpeed);

		if ( (CentaurTurretRotationAC->VolumeMultiplier > 0.f) && !CentaurTurretRotationAC->IsPlaying() )
		{
			CentaurTurretRotationAC->Play();
		}

	}

	// tire loop audio
	if (TireAudioLoopAC)
	{
		TireAudioLoopAC->VolumeMultiplier = GetMappedRangeValue(TireAudioLoopVolumeVelocityRange, TireAudioLoopVolumeRange, Velocity.Size2D());

		if ( (TireAudioLoopAC->VolumeMultiplier > 0.f) && !TireAudioLoopAC->IsPlaying() )
		{
			TireAudioLoopAC->Play();
		}
	}
}

/** Set any params on wheel particle effect */
void AVehicle_Centaur_Base::SetWheelEffectParams(USVehicleWheel* VW, FLOAT SlipVel)
{
	FLOAT SpinParam = Abs(VW->SpinVel) * 0.1f;
	//debugf(TEXT("SPIN: %f"), SpinParam);

	// Named Instance Parameter that can be used by artists to control spawn rate etc.
	VW->WheelParticleComp->SetFloatParameter(VW->SlipParticleParamName, SpinParam);
}

FLOAT AVehicle_Centaur_Base::GetGravityZ()
{
	// Ignore influence of physics volumes
	return WorldInfo->RBPhysicsGravityScaling * GWorld->GetGravityZ() * VehicleGravityZScale;
}


UBOOL AVehicle_Centaur_Base::IgnoreBlockingBy( const AActor *Other ) const
{
	if(Other->IsA(AGearPawn_LocustBrumakBase::StaticClass()))
	{
		return TRUE;
	}

	return Super::IgnoreBlockingBy(Other);
}

void ACinematicCentaur_Base::TickSpecial(FLOAT DeltaSeconds)
{
	Super::TickSpecial(DeltaSeconds);

	// for characters we are not trying to animate we don't need to BlendInPhysics!
	const UBOOL bRecentlyRendered = (LastRenderTime > WorldInfo->TimeSeconds - 0.5f);

	FVector LocalUp = LocalToWorld().GetAxis(2);

	if (TRUE)//bRecentlyRendered && !bHidden && (SkeletalMeshComponent->MaxDistanceFactor >= MinDistFactorForUpdate))
	{
		// Calc forward velocity
		FLOAT ForwardVel = Velocity | SkeletalMeshComponent->LocalToWorld.GetAxis(0);
		// Calc wheel movement
		FLOAT DeltaAng = (ForwardVel * DeltaSeconds) / WheelRadius;
		// Calc steering angle from angular velocity
		FLOAT SteerAng = AngularVelocity.Z * AngVelSteerFactor;
		SteerAng = ::Clamp<FLOAT>(SteerAng, -MaxSteerAngle, MaxSteerAngle);

		// Update wheel controls
		for(INT i=0; i<4; i++)
		{
			check(WheelControls[i]);
			WheelControls[i]->WheelRoll += DeltaAng * (180.f/(FLOAT)PI);

			if(i == 2 || i ==3)
			{
				WheelControls[i]->WheelSteering = SteerAng;
			}

			// Do line check for suspension
			FVector WorldWheelDownPos = SkeletalMeshComponent->LocalToWorld.TransformFVector(WheelPosition[i]);
			//GWorld->LineBatcher->DrawLine(Location, WorldWheelDownPos, FColor(255,255,255), SDPG_World);
			FVector WorldWheelUpPos = WorldWheelDownPos + (SuspensionTravel*LocalUp);
			//GWorld->LineBatcher->DrawLine(Location, WorldWheelUpPos, FColor(128,128,255), SDPG_World);

			FLOAT ScaledRadius = DrawScale * WheelRadius;
			FVector LineStart = WorldWheelUpPos - (ScaledRadius * LocalUp);
			FVector LineEnd = WorldWheelDownPos - (ScaledRadius * LocalUp);
			//GWorld->LineBatcher->DrawLine(LineStart, LineEnd, FColor(255,0,0), SDPG_World);

			FVector TraceStart = LineStart + (100.f * LocalUp);
			FVector TraceEnd = LineEnd  - (100.f * LocalUp);

			FCheckResult Result;
			UBOOL bHit = !GWorld->SingleLineCheck(Result, this, TraceEnd, TraceStart, TRACE_World | TRACE_Blocking | TRACE_Volumes | TRACE_ComplexCollision, FVector(0,0,0));

			FVector HitLocation = LineEnd;
			if(bHit)
			{
				// If hit is before line, just use start of line
				if( (Result.Location - TraceStart).SizeSquared() < (LineStart - TraceStart).SizeSquared() )
				{
					HitLocation = LineStart;
				}
				// If past end of line, use end of line
				else if( (Result.Location - TraceStart).SizeSquared() > (LineEnd - TraceStart).SizeSquared() )
				{
					HitLocation = LineEnd;
				}
				else
				{
					HitLocation = Result.Location;
				}
			}
			//GWorld->LineBatcher->DrawLine(Location, HitLocation, FColor(255,255,0), SDPG_World);

			WheelControls[i]->WheelDisplacement = (HitLocation - LineEnd).Size();
		}

		// Find gun direction
		FRotator TurretRot = Rotation;
		if(AimAtActor != NULL)
		{
			TurretRot = (AimAtActor->Location - Location).Rotation();
		}

		// Update gun controls
		for(INT i=0; i<2; i++)
		{
			TurretControls[i]->DesiredBoneRotation = TurretRot;
		}
	}
}

UBOOL AVehicle_Centaur_Base::ShouldClamp()
{
	return (Seats.Num() < 2 || Seats(1).SeatPawn == NULL || Cast<APlayerController>(Seats(1).SeatPawn->Controller) == NULL);
}


void AVehicle_Crane_Base::physInterpolating(FLOAT DeltaTime)
{
	// If not being driven - zero out inputs, and stop all movement right away
	if(!bDriving)
	{
		Steering = 0.f;
		Throttle = 0.f;

		YawAngVel = 0.f;

		if(!bForceAbove)
		{
			ForceRaise = 0;
			RaiseAngVel = 0.f;
		}
	}

	// YAW //

	// Calc new torque
	FLOAT ApplyYawTorque = (YawTorque * -Steering * DeltaTime) + (YawDamping * (1.f-Abs(Steering)) * -YawAngVel);

	// Update ang velocity
	YawAngVel += ApplyYawTorque * DeltaTime;
	YawAngVel = Clamp(YawAngVel, -MaxYawAngVel, MaxYawAngVel);

	// Integrate to get new desired yaw
	CurrentYawAng += YawAngVel * DeltaTime;

	FRotator NewRot = Rotation;
	NewRot.Yaw = appRound(CurrentYawAng);

	// Clamp to range and kill vel when outside
	if(NewRot.Yaw < MinYaw)
	{
		NewRot.Yaw = MinYaw;
		YawAngVel = Max(0.f, YawAngVel);
	}
	else if(NewRot.Yaw > MaxYaw)
	{
		NewRot.Yaw = MaxYaw;
		YawAngVel = Min(0.f, YawAngVel);
	}

	// Update rotation
	SetRotation(NewRot);

	// RAISE //
	
	if(ArmRaiseControl)
	{
		// Allow override
		if ( (ForceRaise != 0) && (Role == ROLE_Authority) )
		{
			Throttle = ForceRaise;
		}

		// Calc new torque
		FLOAT ApplyRaiseTorque = (RaiseTorque * -Throttle * DeltaTime) + (RaiseDamping * (1.f-Abs(Throttle)) * -RaiseAngVel);

		// Update ang velocity
		RaiseAngVel += ApplyRaiseTorque * DeltaTime;
		RaiseAngVel = Clamp(RaiseAngVel, -MaxRaiseAngVel, MaxRaiseAngVel);

		// Integrate to get new desired yaw
		CurrentRaiseAng += RaiseAngVel * DeltaTime;

		FRotator NewArmRot = ArmRaiseControl->BoneRotation;
		NewArmRot.Pitch = appRound(CurrentRaiseAng);

		// Clamp to range and kill vel when outside
		INT UseMin, UseMax;
		BYTE StopSwingLimit = 0;
		UBOOL bSoftLimit = GetRaiseLimits(UseMax, UseMin, StopSwingLimit);

		// Because positive actually makes it go down, flip limits to make the bit more intuitive
		UseMin *= -1;
		UseMax *= -1;

		ForceRaise = 0;

		// Check it we are forcing the pitch up
		if(bForceAbove && NewArmRot.Pitch > -ForceAbovePitch)
		{
			ForceRaise = 1;
		}

		if(NewArmRot.Pitch < UseMin)
		{
			if(bSoftLimit)
			{
				ForceRaise = -1;
			}
			else
			{
				NewArmRot.Pitch = UseMin;
			}
			RaiseAngVel = Max(0.f, RaiseAngVel);
		}
		else if(NewArmRot.Pitch > UseMax)
		{
			if(bSoftLimit)
			{
				ForceRaise = 1;
			}
			else
			{
				NewArmRot.Pitch = UseMax;
			}

			RaiseAngVel = Min(0.f, RaiseAngVel);
		}

		// Turn off swing control if near limit
		check(SpringControl);
		UBOOL bStopSwing = ((StopSwingLimit != 0) && (UseMax - NewArmRot.Pitch < DisableSwingDist));
		//debugf(TEXT("bStopSwing %d %d  %d %d"), StopSwingLimit, bStopSwing, UseMax - NewArmRot.Pitch, UseMax);
		if(bStopSwing && SpringControl->StrengthTarget > 0.5f)
		{
			SpringControl->SetSkelControlStrength(0.f, SwingFadeOutTime);
		}
		else if(!bStopSwing && SpringControl->StrengthTarget < 0.5f)
		{
			SpringControl->SetSkelControlStrength(1.f, SwingFadeOutTime);
			SpringControl->bHadValidStrength = FALSE; // Force a reset
		}

		ArmRaiseControl->BoneRotation = NewArmRot;

		// Update control to spin drum
		DrumControl->BoneRotation.Yaw = appRound(NewArmRot.Pitch * DrumSpinFactor);
	}

}

/** Function that looks up raise limits for the current yaw. */
UBOOL AVehicle_Crane_Base::GetRaiseLimits(INT& OutMinRaise,INT& OutMaxRaise, BYTE& bStopSwingLimit)
{
	// By default, use overall limits
	OutMinRaise = MinRaise;
	OutMaxRaise = MaxRaise;

	// Then look to see if we are in a sub-section 
	for(INT i=0; i<CraneLimitSections.Num(); i++)
	{
		FCraneRaiseLimit& Limit = CraneLimitSections(i);
		if( Rotation.Yaw >= Limit.LimitYawMin &&
			Rotation.Yaw <= Limit.LimitYawMax )
		{
			OutMinRaise = Limit.MinRaise;
			OutMaxRaise = Limit.MaxRaise;
			bStopSwingLimit = (Limit.bDisableSwingAtBottom) ? 1 : 0;
			return !Limit.bHardLimit;
		}
	}

	return FALSE;
}


USeqAct_Interp* AVehicle_Crane_Base::FindCraneCameraMatinee() const
{
	TArray<USequenceObject*> Objs;

	// Look for the camera path
	for (INT LevelIdx = 0; LevelIdx < GWorld->Levels.Num(); LevelIdx++)
	{
		ULevel* const Level = GWorld->Levels(LevelIdx);

		for (INT SeqIdx=0; SeqIdx<Level->GameSequences.Num(); ++SeqIdx)
		{
			USequence* const RootSeq = Level->GameSequences(SeqIdx);

			RootSeq->FindSeqObjectsByName(CraneCameraMatineeName.ToString(), TRUE, Objs, TRUE);
			
			for (INT ObjIdx=0; ObjIdx<Objs.Num(); ++ObjIdx)
			{
				USeqAct_Interp* Matinee = Cast<USeqAct_Interp>(Objs(ObjIdx));
				if (Matinee)
				{
					// return first one found
					return Matinee;
				}
			}

			Objs.Empty();
		}
	}

	return NULL;
}

/** Use TIckSpecial to apply extra grav to falling pieces- we are not in PHYS_RigidBody */
void AHydra_Base::TickSpecial(FLOAT DeltaSeconds)
{
	Super::TickSpecial(DeltaSeconds);
	
	AddRBGravAndDamping();
}

FLOAT AHydra_Base::GetGravityZ()
{
	return Super::GetGravityZ() * HydraGravZScale;
}

/** Native function that adds to the angular velocity */
void AInterpActor_LocustBargeBase::ProcessAddRocking(USeqAct_BargeAddRocking* Action)
{
	if(Role == ROLE_Authority)
	{
		RollAngVel += Action->RollVel.GetValue(0.f, NULL);
		PitchAngVel += Action->PitchVel.GetValue(0.f, NULL);
	}
}

void AInterpActor_LocustBargeBase::TickSpecial(FLOAT DeltaSeconds)
{
	Super::TickSpecial(DeltaSeconds);

	// (on server) If its time to change random torque, do so
	if((Role == ROLE_Authority) && (GWorld->GetTimeSeconds() > NextRandomTorqueChange))
	{
		CurrentRandomRollTorque = (appFrand()*2.f - 1.f) * RandomTorqueStrength;
		CurrentRandomPitchTorque = (appFrand()*2.f - 1.f) * RandomTorqueStrength;

		// Then update when to change it again
		NextRandomTorqueChange = GWorld->GetTimeSeconds() + RandomTorqueChangeInterval;
	}

	///////////////// ROLL

	// integrator
	FLOAT RollTorque = (-RollPos * RollStiffness) + (-RollAngVel * RollDamping) + CurrentRandomRollTorque;
	RollAngVel += RollTorque * DeltaSeconds;
	RollPos += RollAngVel * DeltaSeconds;

	// Limit angle
	if(RollPos > RollMaxAngle)
	{
		RollPos = RollMaxAngle;
		RollAngVel = Min(0.f, RollAngVel); // Ensure vel is negative
	}
	else if(RollPos < -RollMaxAngle)
	{
		RollPos = -RollMaxAngle;
		RollAngVel = Max(0.f, RollAngVel); // Ensure vel is positive
	}

	///////////////// PITCH

	// integrator
	FLOAT PitchTorque = (-PitchPos * PitchStiffness) + (-PitchAngVel * PitchDamping) + CurrentRandomPitchTorque;
	PitchAngVel += PitchTorque * DeltaSeconds;
	PitchPos += PitchAngVel * DeltaSeconds;

	// Limit angle
	if(PitchPos > PitchMaxAngle)
	{
		PitchPos = PitchMaxAngle;
		PitchAngVel = Min(0.f, PitchAngVel); // Ensure vel is negative
	}
	else if(PitchPos < -PitchMaxAngle)
	{
		PitchPos = -PitchMaxAngle;
		PitchAngVel = Max(0.f, PitchAngVel); // Ensure vel is positive
	}
}

void AInterpActor_LocustBargeBase::AdjustInterpTrackMove(FVector& Pos, FRotator& Rot, FLOAT DeltaTime)
{
	Rot.Roll += appFloor(RollPos);
	Rot.Pitch += appFloor(PitchPos);
}


