/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearVehicleSimCar extends SVehicleSimCar
	native(Vehicle);

/** Torque vs Speed curve: This curve approximates a transmission as opposed to actually simulating one as in SVehicleSimTransmission
    In general you want to have a higher torque at low speed and a lower torque at high speed */
var()	InterpCurveFloat		TorqueVSpeedCurve;

/** Limited slip differential: 0.60 would mean 60% of the power is routed through the LSD and 40% is divided evenly */
var()   float					LSDFactor;

/** ThrottleSpeed is the speed at which the vehicle will increase its ActualThrottle to get to the desired Throttle */
var()   float					ThrottleSpeed;

/** How quickly boost effect comes on/off. */
var()	float					BoostSpeed;

/** Torque applied when in the air to try and keep vehicle horizontal. */
var()	float					InAirUprightTorqueFactor;

/** Max torque that can be applied to try and */
var()	float					InAirUprightMaxTorque;

/** Damping applied to angular velocity while in the air. */
var()	float					InAirAngVelDamping;

/** How much to scale AirSpeed when boosting. */
var()	float					BoostAirSpeedScale;

/** How much to scale wheel torque when boosting */
var()	float					BoostTorqueScale;

/** Slow down steering speed when boosting */
var()	float					BoostSteerSpeed;

/** Reduce max steer angle when boosting */
var()	float					BoostMaxSteerAngleScale;

/* Tank steering */
/** Enables tank steering */
var (TankSteering) bool			bDoTankSteering;
/** Tank steer only kicks in if throttle is below this amount */
var (TankSteering) float		TankSteerThrottleThreshold;
/** Max engine torque applied while tank steering */
var (TankSteering) float		MaxEngineTorque;
/** Percentage of engine torque applied as a function of steering */
var (TankSteering) float		TurnInPlaceThrottle;
/** Percentage modifier applied to 'inside' track while turning */
var (TankSteering) float		InsideTrackTorqueFactor;

// Internal

/** Current amount of boost being applied - the speed it changes is affected by BoostSpeed */
var		float					ActualBoost;

var		bool					bForceStop;
var     float					MinRPM;
var     float					MaxRPM;
var     float					ActualThrottle;
var		float					TotalTorque;

cpptext
{
	// SVehicleSimBase interface.
	virtual void UpdateVehicle(ASVehicle* Vehicle, FLOAT DeltaTime);
	virtual FLOAT GetEngineOutput(ASVehicle* Vehicle);
	virtual void ProcessCarInput(ASVehicle* Vehicle);
	virtual void UpdateHandbrake(ASVehicle* Vehicle);

	// Approximate tank steering
	void ApplyTankSteering(ASVehicle* Vehicle, FLOAT DeltaTime);
}

defaultproperties
{
	ThrottleSpeed=2.5

	BoostMaxSteerAngleScale=1.0
	BoostSpeed=1.0
	BoostAirSpeedScale=1.0
	BoostTorqueScale=1.0
	BoostSteerSpeed=60

	bDoTankSteering=False
	MaxEngineTorque=440.0
	TurnInPlaceThrottle=0.25
	InsideTrackTorqueFactor=0.25
	TankSteerThrottleThreshold=0.15
}
