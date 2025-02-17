
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_PushOutOfCover extends GearSpecialMove;

/** Body Stances to play */
var()	GearPawn.BodyStance	BS_Cov_Mid_CoverBreak;
var()	GearPawn.BodyStance	BS_Cov_Mid_CoverBreakMirrored;
var()	GearPawn.BodyStance	BS_Cov_Std_CoverBreak;
var()	GearPawn.BodyStance	BS_Cov_Std_CoverBreakMirrored;

function bool CanChainMove(ESpecialMove NextMove)
{
	// all special moves can chain from a pushout (except mantles! :P)
	return (NextMove != SM_MidLvlJumpOver);
}

protected function bool InternalCanDoSpecialMove()
{
	if( PawnOwner.IsDoingASpecialMove() && !PawnOwner.IsDoingMove2IdleTransition() )
	{
		return FALSE;
	}

	// don't bother if doing a melee attack
	if( PawnOwner.bDoingMeleeAttack )
	{
		return FALSE;
	}

	return TRUE;
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	// For Heavy Weapons playing a mirror transition, we need the IK bones to be turned off...
	if( PawnOwner.bWantsToBeMirrored && PawnOwner.MirrorNode != None && PawnOwner.MyGearWeapon != None && PawnOwner.MyGearWeapon.WeaponType == WT_Heavy )
	{
		PawnOwner.MirrorNode.ForceDrivenNodesOff();
	}

	Super.SpecialMoveStarted(bForced,PrevMove);
}

/** 
 * See if we could play a mirror transition right now.
 * Make sure it won't collide with one playing currently and produce something ugly.
 */
event bool IsMirrorTransitionSafe()
{
	`LogSMExt(PawnOwner, "IsIKReady:" @ IsIKReady());

	if( IsIKReady() )
	{
		return Super.IsMirrorTransitionSafe();
	}

	return FALSE;
}


/** Called when ready to break from cover. Might be delayed because doing a mirror transition. */
function OnMirrorTransitionSafeNotify()
{
	StartPushOut();
}

/** 
 * Make sure that IKs are turned off. Only matters for Heavy Weapons. 
 * Work around for Heavy Weapons having a different IK chain setup, causing issues when blending with mirrored animations. 
 */
function bool IsIKReady()
{
	`LogSMExt(PawnOwner, "bWantsToBeMirrored:" @ PawnOwner.bWantsToBeMirrored );
	if( PawnOwner.bWantsToBeMirrored && PawnOwner.MirrorNode != None && PawnOwner.MyGearWeapon != None && PawnOwner.MyGearWeapon.WeaponType == WT_Heavy )
	{
		`LogSMExt(PawnOwner, "AreDrivenNodesTurnedOff:" @ PawnOwner.MirrorNode.AreDrivenNodesTurnedOff() );
		return PawnOwner.MirrorNode.AreDrivenNodesTurnedOff();
	}

	return TRUE;
}

/** Trigger animations, physics, etc. */
function StartPushOut()
{
	// Unlock Pawn Rotation so he interpolate back to camera rot.
	SetLockPawnRotation(FALSE);
	// Override
	// Rot time has to match blend in
	// Need special interp. Not always want shortpath to match anim blend.
	PawnOwner.InterpolatePawnRotation(0.15f/SpeedModifier, TRUE);

	// check to see if we are going into standing or midlevel cover
	if( PawnOwner.CoverType == CT_MidLevel )
	{
		PawnOwner.SoundGroup.PlayFoleySound( PawnOwner, GearFoley_ExitCoverLowFX );
	}
	else if( PawnOwner.CoverType == CT_Standing )
	{
		PawnOwner.SoundGroup.PlayFoleySound( PawnOwner, GearFoley_ExitCoverHighFX );
	}

	// Push Pawns only on locally owned clients or server.
	// This equals to the Pawn having a controller set
	if( PawnOwner.Controller != None )
	{
		// Give a velocity push away from cover
		PawnOwner.Velocity		= -PawnOwner.GroundSpeed * Vector(PawnOwner.Rotation) * SpeedModifier;
		PawnOwner.Acceleration	= Normal(PawnOwner.Velocity) * PawnOwner.AccelRate * SpeedModifier;

		// Force Pawn to keep moving
		PawnOwner.bForceMaxAccel = TRUE;

		`LogSMExt(PawnOwner,"Push out:"@PawnOwner.Velocity@PawnOwner.Acceleration);
	}

	// Play transition animation.
	if( PawnOwner.bIsMirrored )
	{
		// If we're popping up, then play standing exit animation.
		if( PawnOwner.CoverType == CT_MidLevel && PawnOwner.CoverAction == CA_PopUp )
		{
			// Play body stance animation.
			PawnOwner.BS_Play(BS_Cov_Mid_CoverBreakMirrored, SpeedModifier, 0.15f/SpeedModifier, 0.25f/SpeedModifier, FALSE, TRUE);
		}
		else
		{
			// Play body stance animation.
			PawnOwner.BS_Play(BS_Cov_Std_CoverBreakMirrored, SpeedModifier, 0.15f/SpeedModifier, 0.25f/SpeedModifier, FALSE, TRUE);
		}

		// Play animation mirrored
		PawnOwner.BS_SetMirrorOptions(BS_Cov_Std_CoverBreakMirrored, FALSE, TRUE, TRUE);

		// Don't trigger the notification early, as we want the IK to be switched back on after the transition animation has disapeared.
		// Heavy Weapons use a different IK setup, and we can't have them on during the transition animation.
		if( PawnOwner.MyGearWeapon.WeaponType == WT_Heavy )
		{
			PawnOwner.BS_SetEarlyAnimEndNotify(BS_Cov_Std_CoverBreakMirrored, FALSE);
		}
	}
	else
	{
		if( PawnOwner.CoverType == CT_MidLevel )
		{
			// Play body stance animation.
			PawnOwner.BS_Play(BS_Cov_Mid_CoverBreak, SpeedModifier, 0.15f/SpeedModifier, 0.25f/SpeedModifier, FALSE, TRUE);
		}
		else
		{
			// Play body stance animation.
			PawnOwner.BS_Play(BS_Cov_Std_CoverBreak, SpeedModifier, 0.15f/SpeedModifier, 0.25f/SpeedModifier, FALSE, TRUE);
		}
	}

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_Cov_Std_CoverBreak, TRUE);
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Disable forced movement
	PawnOwner.bForceMaxAccel = FALSE;

	// Turn off mirroring
	PawnOwner.BS_SetMirrorOptions(BS_Cov_Std_CoverBreak, FALSE, FALSE, FALSE);

	// Turn off AnimEnd notification
	PawnOwner.BS_SetAnimEndNotify(BS_Cov_Std_CoverBreak, FALSE);
}


defaultproperties
{
	bCoverExitMirrorTransition=TRUE
	bMirrorTransitionSafeNotify=TRUE
	bLockPawnRotation=TRUE
	bBreakFromCover=TRUE
	bDisableMovement=TRUE

	DefaultAICommand=class'AICmd_Base_PushedBySpecialMove'
	bDisableAI=TRUE

	BS_Cov_Mid_CoverBreak=(AnimName[BS_FullBody]="AR_Cov_Mid_Idle2Idle_Ready_Aim")
	BS_Cov_Mid_CoverBreakMirrored=(AnimName[BS_FullBody]="AR_Cov_Mid_Idle_Mirrored2Idle_Ready_Aim")
	BS_Cov_Std_CoverBreak=(AnimName[BS_FullBody]="AR_Cov_Std_Idle2Idle_Ready_Aim")
	BS_Cov_Std_CoverBreakMirrored=(AnimName[BS_FullBody]="AR_Cov_Std_Idle_Mirrored2Idle_Ready_Aim")
}
