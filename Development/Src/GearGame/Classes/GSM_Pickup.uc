
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Pickup extends GSM_BasePlaySingleAnim;

// Animations
var BodyStance	BS_PickupWeapon, BS_KickUpWeapon, BS_PickupHeavyWeapon;
var CameraAnim	PickupCameraAnim, KickupCameraAnim;
// Actor to pickup.
var Actor		PickupActor;

enum EPickupType
{
	ePT_Pickup,
	ePT_KickUp,
	ePT_PickupHeavyWeapon,
};
var	EPickupType	PickupType;

protected function bool InternalCanDoSpecialMove()
{
	return !PawnOwner.bSwitchingWeapons && !PawnOwner.bDoingMeleeAttack;
}

static function INT PackSpecialMoveFlags(EPickupType InPickupType)
{
	local INT	Flags;

	Flags = INT(InPickupType);
	return Flags;
}

function UnPackSpecialMoveFlags()
{
	PickupType = EPickupType(PawnOwner.SpecialMoveFlags);
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	UnPackSpecialMoveFlags();

	Super.SpecialMoveStarted(bForced, PrevMove);

	// Trigger the pickup delegates in the GearPC
	if( PCOwner != None )
	{
		PCOwner.TriggerGearEventDelegates( eGED_Pickup );
	}

	if( PickupType != ePT_PickupHeavyWeapon || !PawnOwner.HolsterWeapon() )
	{
		PlayPickupAnimation();
	}
}

// Disable call from Special Move started
function PlayAnimation();

/** Play pickup animation when we're ready to do so. */
function PlayPickupAnimation()
{
// 	`log(PawnOwner.WorldInfo.TimeSeconds @ class @ GetFuncName() @ "Weapon:" @ PawnOwner.MyGearWeapon @ "PickupType:" @ PickupType);
	switch( PickupType )
	{
		case ePT_Pickup :
			BS_Animation = BS_PickupWeapon;
			PlayCameraAnim(PawnOwner, PickupCameraAnim,,, 0.25f, 0.25f);
			PawnOwner.SetTimer( 0.42f, FALSE, nameof(DoPickUp), Self );
			break;
		case ePT_KickUp :
			BS_Animation = BS_KickUpWeapon;
			PlayCameraAnim(PawnOwner, KickupCameraAnim,,, 0.25f, 0.25f);
			PawnOwner.IKBoneCtrl_LeftHand.SetSkelControlActive(FALSE);
			PawnOwner.IKBoneCtrl_RightHand.SetSkelControlActive(FALSE);
			PawnOwner.SetTimer( 0.11f, FALSE, nameof(DoPickUp), Self );
			break;

		case ePT_PickupHeavyWeapon :
			BS_Animation = BS_PickupHeavyWeapon;
			PlayCameraAnim(PawnOwner, PickupCameraAnim,,, 0.25f, 0.25f);
			PawnOwner.SetTimer( 0.52f, FALSE, nameof(DoPickUp), Self );
			break;
	}

	Super.PlayAnimation();
}

function WeaponTemporarilyHolstered()
{
// 	`log(PawnOwner.WorldInfo.TimeSeconds @ class @ GetFuncName() @ "Weapon:" @ PawnOwner.MyGearWeapon);
	PlayPickupAnimation();
}

function DoPickUp()
{
// 	`log(PawnOwner.WorldInfo.TimeSeconds @ class @ GetFuncName() @ "Weapon:" @ PawnOwner.MyGearWeapon @ "PickupType:" @ PickupType @ "PickupActor:" @ PickupActor);
	PawnOwner.GrabPickup(PickupActor);
}

function TryToPutDownNotify()
{
// 	`log(PawnOwner.WorldInfo.TimeSeconds @ class @ GetFuncName() @ "Weapon:" @ PawnOwner.MyGearWeapon @ "PickupType:" @ PickupType);
	if( PickupType == ePT_PickupHeavyWeapon )
	{
		HeavyWeaponCancelTemporaryHolster();
	}
}

function HeavyWeaponCancelTemporaryHolster()
{
	local GearWeapon PendingWeapon;
// 	`log(PawnOwner.WorldInfo.TimeSeconds @ class @ GetFuncName() @ "Weapon:" @ PawnOwner.MyGearWeapon @ "PendingWeapon:" @ PawnOwner.InvManager.PendingWeapon @ "PickupType:" @ PickupType);

	// Little hack to have the Pawn switch to his new weapon.
	// Make this temporaty holster a permanent one!
	if( PawnOwner.MyGearWeapon != None && PawnOwner.MyGearWeapon.bTemporaryHolster )
	{
		PendingWeapon = GearWeapon(PawnOwner.InvManager.PendingWeapon);
		if( PendingWeapon != None && PendingWeapon.WeaponType == WT_Heavy )
		{
			PawnOwner.MyGearWeapon.bTemporaryHolster = FALSE;
			if( PawnOwner.MyGearWeapon.IsInState('WeaponPuttingDown') )
			{
				PawnOwner.MyGearWeapon.WeaponIsDown();
			}
		}
		else
		{
			PawnOwner.UnHolsterWeapon();
		}
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	if( PawnOwner.IsTimerActive('DoPickUp', Self) )
	{
		PawnOwner.ClearTimer('DoPickUp', Self);
		DoPickUp();
	}

	// If we've been aborted, and we were holstering our weapon, unholster it.
	if( PickupType == ePT_PickupHeavyWeapon )
	{
		HeavyWeaponCancelTemporaryHolster();
	}

	PawnOwner.IKBoneCtrl_LeftHand.SetSkelControlActive(TRUE);
	PawnOwner.IKBoneCtrl_RightHand.SetSkelControlActive(TRUE);

	Super.SpecialMoveEnded(PrevMove, NextMove);

	// check for rr transition for clifford
	if( PCOwner != None && PCOwner.IsHoldingRoadieRunButton() &&
		PawnOwner != None && PawnOwner.PendingSpecialMoveStruct.SpecialMove == SM_None )
	{
		if( PCOwner.CanDoSpecialMove(SM_RoadieRun) )
		{
			PawnOwner.QueueRoadieRunMove(FALSE);
		}
	}
}

defaultproperties
{
	bShouldAbortWeaponReload=FALSE
	bCanFireWeapon=FALSE
	bDisableMovement=TRUE
	bLockPawnRotation=TRUE

	BS_PickupWeapon=(AnimName[BS_FullBody]="AR_Idle_Ready_Pickup")
	BS_KickUpWeapon=(AnimName[BS_FullBody]="AR_Idle_Ready_Kickup")
	BS_PickupHeavyWeapon=(AnimName[BS_FullBody]="AR_Idle_Ready_Pickup_HW")

	PickupCameraAnim=CameraAnim'COG_MarcusFenix.Camera_Anims.Pickup'
	KickupCameraAnim=CameraAnim'COG_MarcusFenix.Camera_Anims.Kickup'
}

