/**
 * This is needed as we must have a base class which native classes may reference that have
 * no references to content.  As if a native class references a class then that class must be
 * always loaded.  By having this abstraction layer we can not have specific classes loaded all
 * the time.
 *
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearInterActorAttachableBase extends InterpActor
	abstract;


function bool HasAvailableSlots();

function bool ClaimSlot( Pawn NewClaim, out Rotator out_RelativeRotation );


defaultproperties
{
}