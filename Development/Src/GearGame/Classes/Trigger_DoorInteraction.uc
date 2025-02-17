/**
* A trigger that streamlines the door interaction process.
* Touching and Untouching this trigger will allow code to handle the input
* and animation work needed to open the door.
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class Trigger_DoorInteraction extends Trigger
	placeable;

var() Actor AlignToObj;

/**	Handling Toggle event from Kismet. */
simulated function OnToggle(SeqAct_Toggle Action)
{
	// Turn ON
	if (Action.InputLinks[0].bHasImpulse)
	{
		if(!bCollideActors)
		{
			SetCollision(true, bBlockActors);
		}
		CollisionComponent.SetBlockRigidBody( TRUE );
	}
	// Turn OFF
	else if (Action.InputLinks[1].bHasImpulse)
	{
		if(bCollideActors)
		{
			SetCollision(false, bBlockActors);
		}
		CollisionComponent.SetBlockRigidBody( FALSE );
	}
	// Toggle
	else if (Action.InputLinks[2].bHasImpulse)
	{
		SetCollision(!bCollideActors, bBlockActors);
		CollisionComponent.SetBlockRigidBody( !CollisionComponent.BlockRigidBody );
	}
	`log(self@GetFuncName()@bCollideActors);
}

defaultproperties
{
	Begin Object Name=CollisionCylinder
		CollisionRadius=128.f
		CollisionHeight=128.f
	End Object

	SupportedEvents.Add(class'SeqEvt_DoorOpen')
	SupportedEvents.Remove(class'SeqEvent_Used')
}
