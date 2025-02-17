/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class InterpActor_LocustGondolaBase extends InterpActor_GearBasePlatform
	abstract
	notplaceable;

var protected StaticMeshComponent HighCollision;
var protected StaticMeshComponent LowCollision;

simulated function OnGondolaControl(SeqAct_GondolaControl Action)
{
	if (Action.InputLinks[0].bHasImpulse)
	{
		// 0 is Allow Mantling
		DetachComponent(HighCollision);
		AttachComponent(LowCollision);
	}
	else
	{
		// 1 is Disallow Mantling
		DetachComponent(LowCollision);
		AttachComponent(HighCollision);
	}
}

defaultproperties
{
	BlockRigidBody=TRUE
	bCollideActors=TRUE
	bBlockActors=TRUE

	// no clamp so we can do the gondola switch
	bClampedBaseEnabled=FALSE

	Begin Object Name=MyLightEnvironment
		bEnabled=TRUE
	End Object

	// this obj is for all collision except rbs
	// has high walls, used everywhere except during mantle sequence
	Begin Object Class=StaticMeshComponent Name=HighWallCollision0
	End Object
	// don't attach this one by default.  this way the mantle stuff will build properly.
	// LDs will bring up high wall collision on demand
	HighCollision=HighWallCollision0

	// this obj is for all collision except rbs
	// has low walls, used during mantle sequence
	Begin Object Class=StaticMeshComponent Name=LowWallCollision0
	End Object
	Components.Add(LowWallCollision0)
	LowCollision=LowWallCollision0

	// this obj is designed solely to provide collision for rigid bodies
	Begin Object Class=StaticMeshComponent Name=RBCollision0
	End Object
	Components.Add(RBCollision0)
}