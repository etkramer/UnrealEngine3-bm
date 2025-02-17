/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class InterpActor_MineCart extends GearInterActorAttachableBase;

// Slot for each side of the cart, max 4 wretches attacking
// Each slot is has a relative rotation of rot(0,16384,0)*Idx
var		Pawn	Slots[4];

/** Checks to see if there are any slots open */
function bool HasAvailableSlots()
{
	local int Idx;

	for( Idx = 0; Idx < 4; Idx++ )
	{
		// Open if empty or dead
		if( Slots[Idx] == None ||
			Slots[Idx].Health <= 0 )
		{
			return TRUE;
		}
	}

	return FALSE;
}

/** Assigns a pawn to a slot */
function bool ClaimSlot( Pawn NewClaim, out Rotator out_RelativeRotation )
{
	local array<int> Open;
	local int		 Idx;
	local bool		 bResult;

	// Get a list of all open slots
	for( Idx = 0; Idx < 4; Idx++ )
	{
		if( Slots[Idx] == None ||
			Slots[Idx].Health <= 0 )
		{
			Open[Open.Length] = Idx;
		}
	}

	// If found at least one open
	if( Open.Length > 0 )
	{
		// Pick a random index
		Idx = Open[Rand(Open.Length)];
		// Claim the slot
		Slots[Idx] = NewClaim;
		// Set relative rotation for the slot
		out_RelativeRotation = rot(0,16384,0) * Idx;

		bResult = TRUE;
	}
	
	return bResult;
}

defaultproperties
{
	bBlockActors=TRUE
	bCollideActors=TRUE

	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'MineCartAssets.Cart.MineCart_New_DR'
		CollideActors=TRUE
		BlockActors=TRUE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=TRUE
		BlockRigidBody=TRUE
    End Object
	StaticMeshComponent=StaticMeshComponent0
	CollisionComponent=StaticMeshComponent0
	Components.Add(StaticMeshComponent0)
}