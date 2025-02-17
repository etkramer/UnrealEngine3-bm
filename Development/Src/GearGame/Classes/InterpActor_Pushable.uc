/** dynamic static mesh actor intended to be pushed
*	replaces movers
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class InterpActor_Pushable extends InterpActor
	native
	placeable;

/** Force exerted per push */
var() float PushForce;
/** Direction to move this object. */
var vector PushDirection;
/** Sound on Push*/
var() SoundCue PushSound;
/** Speed will be limited to this. */
var()	float	MaxSpeed;

cpptext
{
	virtual void physRigidBody(FLOAT DeltaTime);
}

simulated function Pushed()
{
	if (Physics == PHYS_None)
	{
		SetPhysics(PHYS_RigidBody);
	}
	StaticMeshComponent.AddForce( PushDirection * PushForce );
	if (PushSound != None)
	{
		PlaySound(PushSound, true);
	}
}

defaultproperties
{
	Physics=PHYS_None
	bCollideActors=true
	bCollideWorld=true

	PushForce=50000.f
	MaxSpeed=300.f

	Begin Object Name=StaticMeshComponent0
		RBChannel=RBCC_GameplayPhysics
	End Object

	RemoteRole=ROLE_SimulatedProxy
	bUpdateSimulatedPosition=true
	NetUpdateFrequency=100.0
	bReplicateRigidBodyLocation=true
	bReplicateMovement=true
	bOnlyDirtyReplication=false
	PushSound=SoundCue'Ambient_NonLoop.AmbientNonLoop.MetalMovesCue'
}
