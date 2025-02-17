/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
#include "GearGame.h"

IMPLEMENT_CLASS(AGearPawn_CarryCrate_Base);

void AGearPawn_CarryCrate_Base::CalcVelocity(FVector &AccelDir, FLOAT DeltaTime, FLOAT MaxSpeed, FLOAT Friction, INT bFluid, INT bBrake, INT bBuoyant)
{
	APawn::CalcVelocity(AccelDir, DeltaTime, MaxSpeed, CrateFriction, bFluid, bBrake, bBuoyant);

	RepVelocityX = Velocity.X;
	RepVelocityY = Velocity.Y;
	RepVelocityZ = Velocity.Z;
}

void AGearPawn_CarryCrate_Base::physicsRotation(FLOAT DeltaTime, FVector OldVelocity)
{
	// Only update YawVelocity on the server - replicate to client
	if(Role == ROLE_Authority)
	{
		// Braking case
		if(Abs(YawAcceleration) < KINDA_SMALL_NUMBER)
		{
			FLOAT DeltaVel = 0.f;

			if(YawVelocity > 0.f)
			{
				DeltaVel = Max(-(CrateYawFriction * DeltaTime), -YawVelocity); // Clamp makes sure we don't go negative
			}
			else
			{
				DeltaVel = Min((CrateYawFriction * DeltaTime), -YawVelocity);
			}

			YawVelocity += DeltaVel;
		}
		// Moving case
		else
		{
			YawVelocity += (DeltaTime * YawAcceleration);
		}

		// Clamp max yaw vel
		YawVelocity = Clamp(YawVelocity, -CrateMaxYawVel, CrateMaxYawVel);
	}

	// Integrate to get new rotation.
	FRotator NewRotation = Rotation;
	NewRotation.Yaw += appFloor(YawVelocity * DeltaTime);

	// Update crate actor
	if( NewRotation != Rotation.GetDenormalized() )
	{
		FCheckResult Hit(1.f);
		GWorld->MoveActor( this, FVector(0,0,0), NewRotation, 0, Hit );
	}
}

void AGearPawn_CarryCrate_Base::TickSimulated( FLOAT DeltaSeconds )
{
	bSimGravityDisabled = TRUE;

	Super::TickSimulated(DeltaSeconds);

	physicsRotation(DeltaSeconds, FVector(0.f));
}

UBOOL AGearPawn_CarryCrate_Base::IgnoreBlockingBy(const AActor* Other) const
{
	return (Other == MarcusPawn || Other == DomPawn || Super::IgnoreBlockingBy(Other));
}

static FVector SavedLocation;

void AGearPawn_CarryCrate_Base::PostNetReceive()
{
	// this is a little weird, but it is correct :)
	// the replication code is going to put the new location into Location
	// and Super::PostNetReceive() is going to then exchange it with SavedLocation
	// so when we get to PostNetReceiveLocation(), SavedLocation will be the new location
	// and Location will be the old location
	SavedLocation = Location;

	Super::PostNetReceive();
}

void AGearPawn_CarryCrate_Base::PostNetReceiveLocation()
{
	if((Location - SavedLocation).SizeSquared() > 16.f)
	{
		SavedLocation.Z += 1.f; // make sure we don't get embedded in ground due to rounding error
		AActor::PostNetReceiveLocation();
	}
}

