/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTVehicleSimHoverboard extends SVehicleSimBase
    native(Vehicle);

var()	float				MaxThrustForce;
/** Scaling of extra help given based on the steepness of the hill you are going up. */
var()	float				UphillHelpThrust;
/** Max value of extra uphill help force. */
var()	float				MaxUphillHelpThrust;
var()	float				MaxReverseForce;
var()	float				MaxReverseVelocity;
var()	float				LongDamping;

var()	float				MaxStrafeForce;
var()	float				LatDamping;

var()	float				MaxRiseForce;

var()	float				TurnTorqueFactor;
var()	float				SpinTurnTorqueScale;
var()	float				MaxTurnTorque;
var()	InterpCurveFloat	TurnDampingSpeedFunc;

/** Set to true when hoverboard is over deep water and is not receiving any thrust. */
var		bool				bIsOverDeepWater;

var()	float				StopThreshold;

var()	float				FlyingTowTurnDamping;
var()	float				FlyingTowRelVelDamping;
var()	float				TowRelVelDamping;

cpptext
{
    virtual void ProcessCarInput(ASVehicle* Vehicle);
	virtual void UpdateVehicle(ASVehicle* Vehicle, FLOAT DeltaTime);
	FLOAT GetEngineOutput(ASVehicle* Vehicle);
}
