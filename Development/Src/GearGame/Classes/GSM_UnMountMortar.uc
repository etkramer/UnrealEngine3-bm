
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_UnMountMortar extends GSM_BasePlaySingleAnim;

var() const protected	GearPawn.BodyStance		BS_Idl_UnMount, BS_Mid_UnMount, BS_MidLean_UnMount, BS_StdLean_UnMount;
var() const protected	SoundCue				MortarUnmountSound;

protected function bool InternalCanDoSpecialMove()
{
	// Need to carry a heavy weapon for the animations to exist.
	return (PawnOwner.MyGearWeapon != None) && (PawnOwner.MyGearWeapon.WeaponType == WT_Heavy) && (!PawnOwner.IsReloadingWeapon() || !PawnOwner.IsInCover());
}

function bool CanChainMove(ESpecialMove NextMove)
{
	if( NextMove == SM_TargetMortar )
	{
		return TRUE;
	}

	return FALSE;
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local GearWeap_HeavyBase	HeavyWeapon;

	// Heavy Weapon being carried.
	HeavyWeapon = GearWeap_HeavyBase(PawnOwner.Weapon);
	// Play weapon unmounting animation in cover.
	if( HeavyWeapon != None )
	{
		if( PawnOwner.HeavyMountedCoverType != CT_None )
		{
			HeavyWeapon.PlayWeaponAnim('AR_Mortar_Cov_Deployed_Exit', SpeedModifier, 0.2f/SpeedModifier, 0.f);
		}
		else
		{
			HeavyWeapon.PlayWeaponAnim('AR_Mortar_Deployed_Exit', SpeedModifier, 0.2f/SpeedModifier, 0.f);
		}
	}

	// abort any firing that's in progress
	if (HeavyWeapon.IsFiring())
	{
		HeavyWeapon.StopFire(0);
	}

	// heavy lifting effort
	if(pawnOwner.GetTeamNum() == TEAM_COG)
	{
		PawnOwner.SoundGroup.PlayEffort(PawnOwner, GearEffort_LiftHeavyWeaponEffort, true);
	}
//	PawnOwner.PlaySound(MortarUnmountSound);

	switch( PawnOwner.HeavyMountedCoverType )
	{
		case CT_MidLevel :	
			if( PawnOwner.HeavyMountedCoverAction == CA_LeanLeft || PawnOwner.HeavyMountedCoverAction == CA_LeanRight )
			{
				BS_Animation = BS_MidLean_UnMount;	
			}
			else
			{
				BS_Animation = BS_Mid_UnMount;	
			}
			break;
		case CT_Standing :	
			BS_Animation = BS_StdLean_UnMount;	
			break;
		default:			
			BS_Animation = BS_Idl_UnMount;	
			break;
	}

	Super.SpecialMoveStarted(bForced, PrevMove);
}

function Tick(float DeltaTime)
{
	local GearWeap_HeavyBase HeavyWeap;

	Super.Tick(DeltaTime);

	if( PawnOwner.IsLocallyControlled() )
	{
		// If we ever lose our weapon, then exit this special move.
		HeavyWeap = GearWeap_HeavyBase(PawnOwner.Weapon);
		if( HeavyWeap == None )
		{
			PawnOwner.LocalEndSpecialMove();
		}
	}
}


function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	local GearWeap_HeavyBase HeavyWeap;

	Super.SpecialMoveEnded(PrevMove, NextMove);

	// check to see if we interrupted an auto-reload
	if (NextMove == SM_None && PawnOwner.MyGearWeapon.ShouldAutoReload())
	{
		GearWeapon(PawnOwner.Weapon).ForceReload();
	}

	// check to see if we delayed an auto-switch
	HeavyWeap = GearWeap_HeavyBase(PawnOwner.Weapon);
	if (HeavyWeap != None)
	{
		HeavyWeap.MaybeAutoSwitchWeapon();
	}

	// If we're unmounting the gun, check if we should mount it back again.
	if( NextMove == SM_None && PawnOwner != None && PawnOwner.IsLocallyControlled() )
	{
		PawnOwner.CheckHeavyWeaponMounting(TRUE);
	}
}

function PreProcessInput(GearPlayerInput Input)
{
	// manually disable movement
	Input.RawJoyRight = 0.f;
	Input.RawJoyUp = 0.f;
}


defaultproperties
{
	BlendOutTime=0.25f
	BS_Idl_UnMount=(AnimName[BS_FullBody]="HW_Mortar_Deployed_Exit")
	BS_Mid_UnMount=(AnimName[BS_FullBody]="HW_Mortar_Cov_Mid2Deployed_Exit")
	BS_MidLean_UnMount=(AnimName[BS_FullBody]="HW_Mortar_Cov_Mid2Deployed_Lean_Exit")
	BS_StdLean_UnMount=(AnimName[BS_FullBody]="HW_Mortar_Cov_Std2Deployed_Lean_Exit")

	//bShouldAbortWeaponReload=TRUE
	bBreakFromCover=FALSE
	bCanFireWeapon=FALSE
	bLockPawnRotation=TRUE
	//bDisablePhysics=TRUE
	bCameraFocusOnPawn=FALSE
	bDisableLook=FALSE
	bShouldAbortWeaponReload=TRUE

	//@fixme, get somethjing here
	MortarUnmountSound=None	//SoundCue'Weapon_Mortar.Sounds.MortarPickupCue'
}
