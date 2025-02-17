
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPickupFactory_Shield extends GearPickupFactory;


function GiveTo( Pawn P )
{
	local GearShield	NewShield;
	local GearPawn		GP;

	NewShield = GearShield(P.InvManager.CreateInventory(InventoryType));

	// play an effort
	GP = GearPawn(P);
	if (GP != None)
	{
		GP.SoundGroup.PlayEffort(GP, GearEffort_LiftHeavyWeaponEffort);
	}

	// @fixme, make shield GUDS event?
	GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_PickedUpNewWeapon, P, None);

	if (NewShield.PickupSound != None)
	{
		NewShield.PlaySound(NewShield.PickupSound);
	}

	PickedUpBy(P);

	// note: intentionally not calling super here
}

/** obsoleted by our implementation of GiveTo */
function SpawnCopyFor( Pawn Recipient );

function PickedUpBy(Pawn P)
{
	local GearPawn GP;

	super.PickedUpBy(P);

	GP = GearPawn(P);
	if (GP != None)
	{
		GP.EquipShield(GearInventoryManager(GP.InvManager).Shield);
	}
}

simulated function bool GetPickupAction(out ActionInfo out_PickupAction, Pawn P)
{
	local Name Interaction;

	if (!bPickupHidden)
	{
		Interaction = FindInteractionWith(P, TRUE);
		if (Interaction == 'PickUp')
		{
			out_PickupAction = PickupAction;
			out_PickupAction.ActionName = Name;

			out_PickupAction.ActionIconDatas[2].ActionIcons[0] = class'GearShield'.default.PickupIcon;

			// if it is something that can be interacted with but can't be picked up, apply the NO symbol
			if (!CanBePickedUpBy(P))
			{
				out_PickupAction.ActionIconDatas[0].ActionIcons[0] = NoSymbolIcon;
			}
			return TRUE;
		}
	}

	return FALSE;
}

function SetRespawn()
{
	// the shield that was spawned by picking this up lasts forever, so no need for the factory to reactivate
	GotoState('Disabled');
	bPickupHidden = true;
}

auto state Pickup
{
	simulated function bool CanBePickedUpBy(Pawn P)
	{
		local GearPawn MyGearPawn;

		MyGearPawn = GearPawn(P);
		return (MyGearPawn != None && MyGearPawn.EquippedShield == None && Super.CanBePickedUpBy(P));
	}
}

defaultproperties
{
	InventoryType=class'BoomerShield'

	Begin Object NAME=CollisionCylinder
		CollisionRadius=+000100.000000
		CollisionHeight=+00030.000000
	End Object
}
