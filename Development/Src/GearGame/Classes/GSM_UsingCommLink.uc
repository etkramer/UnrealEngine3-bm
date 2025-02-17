
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_UsingCommLink extends GearSpecialMove;

/** Animation to play */
var()	GearPawn.BodyStance	BS_Animation;


/** See if Player can get in CommLink mode. */
protected function bool InternalCanDoSpecialMove()
{
	// Simple pre-conditions to meet
	if( PawnOwner.bPlayedDeath || PawnOwner.bDeleteMe || PawnOwner.Health <= 0 ||
		PawnOwner.Weapon == None || PawnOwner.bSwitchingWeapons ||
		PawnOwner.bDoingMirrorTransition )
	{
		return FALSE;
	}

	// Special Moves that can be aborted
	if( PawnOwner.SpecialMove != SM_None &&
		PawnOwner.SpecialMove != SM_RoadieRun &&
		PawnOwner.SpecialMove != SM_CoverRun &&
		!PawnOwner.IsDoingMove2IdleTransition() &&
		!PawnOwner.IsDoingMeleeHoldSpecialMove() )
	{
		return FALSE;
	}

	return TRUE;
}

function bool CanChainMove(ESpecialMove NextMove)
{
	return TRUE;
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	// Pawn is using CommLink
	PawnOwner.bUsingCommLink = TRUE;

	if (PawnOwner.IsCarryingAHeavyWeapon())
	{
		// short-circuit holstering stuff, don't bother with finger-to-ear
		NotifyInCommLinkStance();
	}
	else
	{
		PawnOwner.MyGearWeapon.HolsterWeaponTemporarily(TRUE);
	}
}


/** Notification called when weapon has been temporarily holstered. */
simulated function WeaponTemporarilyHolstered()
{
	// Stop weapon holster animation.
	PawnOwner.MyGearWeapon.StopHolsterAnims();

	// Weapon is holstered, now go to commlink pose
	PawnOwner.BS_Play(BS_Animation, 1.f, 0.2f,, TRUE);

	PawnOwner.SetTimer( 0.2f, FALSE, nameof(self.NotifyInCommLinkStance), self );
}

/** Called once the pawn is fully in commlink stance */
simulated function NotifyInCommLinkStance()
{
	PawnOwner.bInCommLinkStance = TRUE;
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	local GearAI AI;

	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Stop CommLink animation
	PawnOwner.BS_Stop(BS_Animation, 0.2f);

	// UnHolster weapon
	if( PawnOwner.MyGearWeapon != None )
	{
		PawnOwner.MyGearWeapon.HolsterWeaponTemporarily(FALSE);
	}

	// Stop using CommLink
	PawnOwner.bUsingCommLink	= FALSE;
	PawnOwner.bInCommLinkStance = FALSE;

	AI = GearAI(PawnOwner.Controller);
	if (AI != None)
	{
		PawnOwner.bWantToUseCommLink = FALSE;
		PawnOwner.bWantToConverse = FALSE;
	}
}

defaultproperties
{
	bBreakFromCover=TRUE
	bDisableLeftHandIK=TRUE
	BS_Animation=(AnimName[BS_Std_Upper_NoAim]="AR_Idle_Headset")
}
