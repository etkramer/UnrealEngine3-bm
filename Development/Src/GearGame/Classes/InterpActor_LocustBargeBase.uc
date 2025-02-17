/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class InterpActor_LocustBargeBase extends InterpActor_GearBasePlatform
	native
	abstract
	notplaceable;

var		float	RollPos;
var		float	RollAngVel;

var		float	PitchPos;
var		float	PitchAngVel;

var()	float	RollStiffness;
var()	float	RollDamping;
var()	float	RollMaxAngle;

var()	float	PitchStiffness;
var()	float	PitchDamping;
var()	float	PitchMaxAngle;

var()	float	RandomTorqueStrength;
var()	float	RandomTorqueChangeInterval;
var		float	CurrentRandomRollTorque;
var		float	CurrentRandomPitchTorque;
var		float	NextRandomTorqueChange;

replication
{
	if (Role == ROLE_Authority)
		RollPos, RollAngVel, PitchPos, PitchAngVel, CurrentRandomRollTorque, CurrentRandomPitchTorque;
}

cpptext
{
	virtual void TickSpecial(FLOAT DeltaSeconds);
	virtual void AdjustInterpTrackMove(FVector& Pos, FRotator& Rot, FLOAT DeltaTime);
}

/** Native function that adds to the angular velocity */
final native function ProcessAddRocking(SeqAct_BargeAddRocking Action);

/** Adds some rocking to the boat */
function OnBargeAddRocking(SeqAct_BargeAddRocking Action)
{
	ProcessAddRocking(Action);
}

defaultproperties
{
	RollStiffness=1.0
	RollDamping=0.2
	RollMaxAngle=1000.0

	PitchStiffness=1.0
	PitchDamping=0.2
	PitchMaxAngle=1000.0

	RandomTorqueChangeInterval=1.0
	RandomTorqueStrength=100.0

	BlockRigidBody=TRUE
	bCollideActors=TRUE
	bBlockActors=TRUE

	Begin Object Name=MyLightEnvironment
		bEnabled=TRUE
	End Object

	// this obj is designed solely to provide collision for rigid bodies
	Begin Object Class=StaticMeshComponent Name=RBCollision0
	End Object
	Components.Add(RBCollision0)

	bAlwaysConfineToClampedBase=TRUE
}