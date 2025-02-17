/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTInventory extends Inventory
	native
	abstract;

var bool bDropOnDisrupt;

/** adds weapon overlay material this item uses (if any) to the GRI in the correct spot
 *  @see UTPawn.WeaponOverlayFlags, UTWeapon::SetWeaponOverlayFlags
 */
simulated static function AddWeaponOverlay(UTGameReplicationInfo GRI);

/** called on the owning client just before the pickup is dropped or destroyed */
reliable client function ClientLostItem()
{
	if (Role < ROLE_Authority)
	{
		// owner change might not get replicated to client so force it here
		SetOwner(None);
	}
}

simulated event Destroyed()
{
	local Pawn P;

	P = Pawn(Owner);
	if (P != None && (P.IsLocallyControlled() || (P.DrivenVehicle != None && P.DrivenVehicle.IsLocallyControlled())))
	{
		ClientLostItem();
	}

	Super.Destroyed();
}

function DropFrom(vector StartLocation, vector StartVelocity)
{
	ClientLostItem();

	Super.DropFrom(StartLocation, StartVelocity);
}

defaultproperties
{
	MessageClass=class'UTPickupMessage'

	DroppedPickupClass=class'UTRotatingDroppedPickup'
	bDropOnDisrupt=true
}
