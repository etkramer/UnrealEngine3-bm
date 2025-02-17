/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorial_UnlimitedAmmoBase extends GearTutorial_PCAction_Base
	abstract
	config(Game);

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Called every tick so that the tutorial has a chance to update itself */
event Update()
{
	local GearPC MyGearPC;
	local GearPawn MyGearPawn;
	local GearWeapon CurrWeapon;

	// Let's make sure we have ammo and don't have to reload after this
	MyGearPC = GearPC(Outer);
	if ( MyGearPC != None )
	{
		MyGearPawn = GearPawn(MyGearPC.Pawn);
		if ( MyGearPawn != None )
		{
			CurrWeapon = MyGearPawn.MyGearWeapon;
			if ( CurrWeapon != None )
			{
				// Lets make sure there is alway an extra clip for the impending reload tutorial
				if ( CurrWeapon.SpareAmmoCount < CurrWeapon.WeaponMagSize )
				{
					CurrWeapon.SpareAmmoCount = CurrWeapon.WeaponMagSize;
				}

				// Don't let them reload until this tutorial is over
				if ( CurrWeapon.WeaponMagSize - CurrWeapon.AmmoUsedCount < 2 )
				{
					CurrWeapon.AmmoUsedCount = CurrWeapon.WeaponMagSize - 2;
				}
			}
		}
	}

	Super.Update();
}
