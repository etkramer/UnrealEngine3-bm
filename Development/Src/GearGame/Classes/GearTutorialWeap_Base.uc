/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
/**
 * GearTutorialWeap_Base
 *
 * Base class for tutorials that will be triggered the first time a player equips a weapon
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorialWeap_Base extends GearTutorial_Base
	config(Game);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Name of the class of the weapon this tutorial cares about */
var Name TutorialWeaponClassName;


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Called when the tutorial gets added to the tutorial manager's list */
function OnTutorialAdded( GearTutorialManager TutMgr, SeqAct_ManageTutorials Action, bool IsKismetDriven )
{
	Super.OnTutorialAdded( TutMgr, Action, IsKismetDriven );

	GearPC(Outer).AddGearWeaponEquipDelegate( OnGearWeaponEquip );
}

/** Called when the tutorial gets removed from the tutorial manager's list */
function OnTutorialRemoved()
{
	Super.OnTutorialRemoved();

	GearPC(Outer).ClearGearWeaponEquipDelegate( OnGearWeaponEquip );
}

/** Delegate fired when the player equips a weapon */
function OnGearWeaponEquip( class<GearWeapon> WeaponClass )
{
	// Make sure this isn't fired when the tutorial has already completed
	if ( (TutorialWeaponClassName == WeaponClass.Name) && !bTutorialIsComplete && !bTutorialIsActive )
	{
		bTutorialIsReadyForActivation = TRUE;
	}
}

/** Called when the tutorial's scene is finished closing */
function OnTutorialSceneClosed( UIScene DeactivatedScene )
{
	Super.OnTutorialSceneClosed( DeactivatedScene );

	// Tutorial is done after the scene closes
	OnTutorialCompleted();
}


defaultproperties
{
	TutorialPriority=TUTORIAL_PRIORITY_AUTOINIT_WEAPON
}
