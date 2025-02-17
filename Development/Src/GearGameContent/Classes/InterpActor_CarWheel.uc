/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 *	Car wheel that rotates around its X axis based on the +Y velocity of its Base.
 */
class InterpActor_CarWheel extends DynamicSMActor
	placeable;

/** Radius of wheel to use for calculations of its rotation. */
var()	float	WheelRadius;

/** Change the way the wheel rotates. */
var()	bool	bInvertRotation;

/** Make the Base the Owner, so line checks ignore it */
simulated function PostBeginPlay()
{
	if(Base != None)
	{
		SetOwner(Base);
	}
}

simulated function Tick(float DeltaTime)
{
	local float YVelMag, DeltaAng;
	local quat DeltaQuat, NewRelQuat;
	local vector HitLocation, HitNormal, BaseX, BaseY, BaseZ;
	local Actor HitActor;

	// If we have a base and it is moving
	if(Base != None && VSizeSq(Base.Velocity) > 0.0)
	{
		GetAxes(Base.Rotation, BaseX, BaseY, BaseZ);

		// Trace down to see if wheel is on the ground

		HitActor = Trace(HitLocation, HitNormal, Location-(WheelRadius*1.2*vect(0,0,1)), Location);

		// If we are - rotate it with the movement of the vehicle.
		if(HitActor != None)
		{
			// Find magnitude of velocity along base's Y
			YVelMag = Base.Velocity dot BaseY;

			// Find change in angle over this timestep.
			DeltaAng = (YVelMag * DeltaTime)/WheelRadius;

			// Make quat representing this rotation.
			if(bInvertRotation)
			{
				DeltaQuat = QuatFromAxisAndAngle( vect(1,0,0), -DeltaAng );
			}
			else
			{
				DeltaQuat = QuatFromAxisAndAngle( vect(1,0,0), DeltaAng );
			}

			// Convert current relative rotation to quat, and apply this rotation
			NewRelQuat = QuatProduct( QuatFromRotator(RelativeRotation) , DeltaQuat);

			// Then update our rotation relative to our base.
			SetRelativeRotation( QuatToRotator(NewRelQuat) );
		}
	}
}

defaultproperties
{
	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'COG_City_Ephyra4.COG_BrokenCar_Pushwheel_JJ'
		BlockRigidBody=false
	End Object

	DrawScale=1.4
	WheelRadius=32
	bHardAttach=true
	bCollideActors=false
	bBlockActors=false
	bNoEncroachCheck=true
	RemoteRole=ROLE_SimulatedProxy
	bUpdateSimulatedPosition=false
	bReplicateMovement=false
	bSkipActorPropertyReplication=true
	NetUpdateFrequency=0.1
	bAlwaysRelevant=true
	bNoDelete=true
}
