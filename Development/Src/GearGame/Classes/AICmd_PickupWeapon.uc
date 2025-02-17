/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_PickupWeapon extends AICommand_SpecialMove
	within GearAI;


/** the pickup the AI wants (only one should be set) */
var GearWeaponPickupFactory TargetFactory;
var GearDroppedPickup TargetPickup;

/** constructors that also take the pickup to use */
static function bool PickupWeaponFromFactory(GearAI AI, GearWeaponPickupFactory InFactory)
{
	local AICmd_PickupWeapon Cmd;

	if (AI != None && InFactory != None && InFactory.WeaponPickupClass != None)
	{
		Cmd = new(AI) default.Class;
		Cmd.TargetFactory = InFactory;
		AI.PushCommand(Cmd);
		return true;
	}
	else
	{
		return false;
	}
}
static function bool PickupWeaponFromPickup(GearAI AI, GearDroppedPickup InPickup)
{
	local AICmd_PickupWeapon Cmd;

	if (AI != None && InPickup != None && InPickup.InventoryClass != None)
	{
		Cmd = new(AI) default.Class;
		Cmd.TargetPickup = InPickup;
		AI.PushCommand(Cmd);
		return true;
	}
	else
	{
		return false;
	}
}

function Pushed()
{
	Super.Pushed();

	if (TargetFactory != None)
	{
		if (!Pawn.ReachedDestination(TargetFactory))
		{
			`AILog("Need to move to pickup first");
			SetMoveGoal(TargetFactory);
		}
	}
	else if (TargetPickup != None && !Pawn.ReachedDestination(TargetPickup))
	{
		`AILog("Need to move to pickup first");
		SetMoveGoal(TargetPickup);
	}
}

function Resumed(name OldCommandName)
{
	Super.Resumed(OldCommandName);

	if (ChildStatus != 'Success')
	{
		// we didn't get there so fail the command
		Status = 'Failure';
		PopCommand(self);
	}
}

auto state StartPickup `DEBUGSTATE
{
	/** decides if we should switch to a different weapon to swap with the one on the ground */
	function CheckWeaponSwitch()
	{
		local GearInventoryManager InvManager;
		local GearWeapon LeftShoulderWeap;
		local class<GearWeapon> WeaponClass;

		// don't need to switch when we already have the weapon and are just taking the ammo
		if ( (TargetPickup != None && TargetPickup.FindInteractionWith(Pawn, true) != 'TakeAmmo') ||
			(TargetFactory != None && TargetFactory.FindInteractionWith(Pawn, true) != 'TakeAmmo') )
		{
			if (TargetPickup != None)
			{
				WeaponClass = class<GearWeapon>(TargetPickup.InventoryClass);
			}
			else
			{
				WeaponClass = TargetFactory.WeaponPickupClass;
			}
			if (WeaponClass != None && WeaponClass.default.WeaponType == WT_Normal)
			{
				InvManager = GearInventoryManager(Pawn.InvManager);
				if (InvManager != None)
				{
					// try to keep assault rifle
					LeftShoulderWeap = GearWeapon(InvManager.GetInventoryInSlot(EASlot_LeftShoulder));
					if ( LeftShoulderWeap == None || !LeftShoulderWeap.HasAnyAmmo()
						|| (!LeftShoulderWeap.IsA('GearWeap_LocustAssaultRifle') && !LeftShoulderWeap.IsA('GearWeap_AssaultRifle')) )
					{
						InvManager.SetWeaponFromSlot(EASlot_LeftShoulder);
					}
					else
					{
						InvManager.SetWeaponFromSlot(EASlot_RightShoulder);
					}
				}
			}
		}
	}

Begin:
	// have to verify pickup's still valid since it might have been picked up by somebody else
	if ( (TargetPickup != None && !TargetPickup.bDeleteMe) ||
		(TargetFactory != None && TargetFactory.ReadyToPickup(0.0)) )
	{
		// we need to switch to the right weapon before the final check as the interaction finding code
		// may rely on a certain weapon being out
		CheckWeaponSwitch();
		while (IsSwitchingWeapons())
		{
			Sleep(0.5);
		}
		if ( (TargetPickup != None && TargetPickup.CanBePickedUpBy(Pawn)) ||
			(TargetFactory != None && TargetFactory.CanBePickedUpBy(Pawn)) )
		{
			if (Pawn != None)
			{
				if (TargetPickup != None && TargetPickup.CanBePickedUpBy(Pawn))
				{
					GearPawn(Pawn).PickupWeapon(TargetPickup);
					GotoState('Command_SpecialMove');
				}
				else if (TargetFactory != None && TargetFactory.CanBePickedUpBy(Pawn))
				{
					GearPawn(Pawn).PickupWeapon(TargetFactory);
					GotoState('Command_SpecialMove');
				}
			}
		}
	}

	Status = 'Failure';
	PopCommand(self);
}

state Command_SpecialMove
{
	function ESpecialMove GetSpecialMove()
	{
		return SM_WeaponPickup;
	}
}
