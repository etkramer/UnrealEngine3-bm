/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorial_Reload extends GearTutorial_Base
	config(Game);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Percent of ammo left for us to start tutorial */
var config float PercentAmmoForStarting;

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Called when the tutorial gets added to the tutorial manager's list */
function OnTutorialAdded( GearTutorialManager TutMgr, SeqAct_ManageTutorials Action, bool IsKismetDriven  )
{
	Super.OnTutorialAdded( TutMgr, Action, IsKismetDriven );

	GearPC(Outer).AddGearEventDelegate( eGED_Reload, OnReload );
}

/** Called when the tutorial gets removed from the tutorial manager's list */
function OnTutorialRemoved()
{
	Super.OnTutorialRemoved();

	GearPC(Outer).ClearGearEventDelegate( eGED_Reload, OnReload );
}

/** Called every tick so that the tutorial has a chance to update itself */
event Update()
{
	local bool bIsLowOnAmmo;

	bIsLowOnAmmo = HasLowAmmo();

	// If this tutorial is not active see if we should set the flag for it to be able to go active
	if ( !bTutorialIsActive )
	{
		bTutorialIsReadyForActivation = bIsLowOnAmmo;
	}
	else
	{
		if ( !bIsLowOnAmmo )
		{
			TutorialMgr.DeactivateTutorial();
		}
	}

	Super.Update();
}

/** Checks for low ammo and starts the tutorial when that happens */
final function bool HasLowAmmo()
{
	local GearPC MyGearPC;
	local GearPawn MyGearPawn;
	local GearWeapon CurrWeapon;
	local float MagazineSize, RemainingAmmo;

	// Grab the current weapon
	MyGearPC = GearPC(Outer);
	if ( MyGearPC != None )
	{
		MyGearPawn = GearPawn(MyGearPC.Pawn);
		if ( MyGearPawn != None )
		{
			CurrWeapon = MyGearPawn.MyGearWeapon;
		}
	}

	// See if the ammo is low
	if ( CurrWeapon != None && CurrWeapon.bCanDisplayReloadTutorial )
	{
		MagazineSize = CurrWeapon.GetMagazineSize();
		RemainingAmmo = MagazineSize - CurrWeapon.AmmoUsedCount;

		// If the ammo is low start the tutorial
		if ( (MagazineSize > 0) && ((RemainingAmmo / MagazineSize) <= PercentAmmoForStarting) )
		{
			return true;
		}
	}

	return false;
}

/** Delegate fired when the player reloads */
function OnReload()
{
	// Make sure this isn't fired when the tutorial has already completed
	if ( !bTutorialIsComplete )
	{
		OnTutorialCompleted();
	}
}

defaultproperties
{
	TutorialType=GEARTUT_Reload
	TutorialPriority=TUTORIAL_PRIORITY_AUTOINIT
}
