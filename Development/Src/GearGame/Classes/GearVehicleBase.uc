/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearVehicleBase extends SVehicle
	abstract
	native(Vehicle);

cpptext
{
	virtual UBOOL ReachedDesiredRotation();
}

simulated function PostBeginPlay()
{
	local PlayerController PC;

	super.PostBeginPlay();

	ForEach LocalPlayerControllers(class'PlayerController', PC)
	{
		GearPC(PC).bCheckVehicles = true;
	}
}

/**
request change to adjacent vehicle seat
*/
simulated function AdjacentSeat(int Direction, Controller C)
{
	ServerAdjacentSeat(Direction, C);
}

/**
request change to adjacent vehicle seat
*/
reliable server function ServerAdjacentSeat(int Direction, Controller C);

/**
* Called when a client is requesting a seat change
*
* @network	Server-Side
*/
reliable server function ServerChangeSeat(int RequestedSeat);

/** Called when a client presses X while in the vehicle. */
reliable server function ServerPressedX();

/** server only */
function PassengerBecomeDriver(Controller C);

/** server only */
function PassengerSentTwoFloats(float A, float B);

/**
 * When this pawn suicides (via the controller) it needs to kill the driver
 */
function Suicide()
{
	if ( Driver != None )
		HandleDeadVehicleDriver();
	else
		KilledBy(self);
}

/**
* AI - Returns the best firing mode for this weapon
*/
function byte ChooseFireMode()
{
	return 0;
}

/**
* AI - An AI controller wants to fire
*
* @Param 	bFinished	unused
*/

function bool BotFire(bool bFinished)
{
	StartFire(ChooseFireMode());

	return true;
}

/**
* @Returns the scale factor to apply to damage affecting this vehicle
*/
function float GetDamageScaling()
{
	if (Driver != None)
	{
		return (Driver.GetDamageScaling() * Super.GetDamageScaling());
	}
	else
	{
		return Super.GetDamageScaling();
	}
}

/**
* @Returns true if the AI needs to turn towards a target
*/
function bool NeedToTurn(vector Targ)
{
	local GearVehicleWeapon VWeapon;

	// vehicles can have weapons that rotate independently of the vehicle, so check with the weapon instead
	VWeapon = GearVehicleWeapon(Weapon);
	if (VWeapon != None)
	{
		return !VWeapon.IsAimCorrect();
	}
	else
	{
		return Super.NeedToTurn(Targ);
	}
}

simulated function DrivingStatusChanged()
{
	Super.DrivingStatusChanged();

	if (!bDriving)
	{
		StopFiringWeapon();
	}
}

function bool DriverEnter(Pawn P)
{
	local AIController C;

	if (Super.DriverEnter(P))
	{
		// update AI enemy
		foreach WorldInfo.AllControllers(class'AIController', C)
		{
			if (C.Enemy == P)
			{
				C.Enemy = self;
			}
		}

		return true;
	}
	else
	{
		return false;
	}
}

/**
* Called on both the server an owning client when the player leaves the vehicle.  We want to make sure
* any active weapon is shut down
*/
function DriverLeft()
{
	local AIController C;

	// update AI enemy
	foreach WorldInfo.AllControllers(class'AIController', C)
	{
		if (C.Enemy == self)
		{
			C.Enemy = Driver;
		}
	}

	Super.DriverLeft();
}

/**
* Shut down the weapon
*/
simulated function StopFiringWeapon()
{
	if (Weapon != none)
	{
		Weapon.ForceEndFire();
	}
}

/** handles the driver pawn of the dead vehicle (decide whether to ragdoll it, etc) */
function HandleDeadVehicleDriver()
{
	local Pawn OldDriver;
	local GearVehicle VehicleBase;

	if (Driver != None)
	{
		VehicleBase = GearVehicle(self);
		if ( VehicleBase == None )
			VehicleBase = GearVehicle(GetVehicleBase());

		// if Driver wasn't visible in vehicle, destroy it
		if (VehicleBase != None && VehicleBase.bEjectKilledBodies && (WorldInfo.TimeSeconds - LastRenderTime < 1.0) && (bDriverIsVisible || ((WorldInfo.GetDetailMode() != DM_Low) && !WorldInfo.bDropDetail)) )
		{
			// otherwise spawn dead physics body
			if (!bDriverIsVisible && PlaceExitingDriver())
			{
				Driver.StopDriving(self);
				Driver.DrivenVehicle = self;
			}
			Driver.TearOffMomentum = Velocity * 0.25;
			Driver.SetOwner(None);
			Driver.Died(None, (VehicleBase != None) ? VehicleBase.RanOverDamageType : class'DamageType', Driver.Location);
		}
		else
		{
			OldDriver = Driver;
			Driver = None;
			OldDriver.DrivenVehicle = None;
			OldDriver.Destroy();
		}
	}
}

/**
 * Called when the controller stops controlling this pawn.  At this point, we want to force all Weapons to
 * stop firing
 */

function UnPossessed()
{
	super.UnPossessed();

	if (Weapon!=none)
	{
		Weapon.StopFire(0);
		Weapon.StopFire(1);
	}
}

/** GearVehicles still use the separate camera system */
simulated function bool CalcCamera(float DeltaTime, out vector out_CamLoc, out rotator out_CamRot, out float out_FOV)
{
	return false;
}

simulated function bool AllowTargetting()
{
	return true;
}

simulated function OnGearExitVehicle( SeqAct_GearExitVehicle inAction )
{
	DriverLeave(true);
}

defaultproperties
{
}
