
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_StumbleFromMelee extends GearSpecialMove;

/** Stumble Animation */
var const	GearPawn.BodyStance	BS_Animation;

protected function bool InternalCanDoSpecialMove()
{
	// Can't stumble if doing another special move
	if( ( PawnOwner.SpecialMove != SM_None 
			&& PawnOwner.SpecialMove != SM_ChainsawHold 
			&& !PawnOwner.IsDoingMove2IdleTransition() 
		)
		|| (PawnOwner.Base != None && ClassIsChildOf( PawnOwner.Base.Class, class'InterpActor_GearBasePlatform' )
		|| PawnOwner.CarriedCrate != none)
	  )
	{
		return FALSE;
	}



	return TRUE;
}

/** 
 * Can this special move override InMove if it is currently playing?
 */
function bool CanOverrideSpecialMove(ESpecialMove InMove)
{
	return (InMove == SM_ChainsawHold);
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	PawnOwner.StopFiring();

	// Play body stance animation.
	PawnOwner.BS_Play(BS_Animation, SpeedModifier, 0.2f/SpeedModifier, 0.3f/SpeedModifier, FALSE, TRUE);

	// Turn on root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_Animation, RBA_Translate, RBA_Translate, RBA_Translate);

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, TRUE);

	// Turn on Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = RMM_Accel;
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Turn off Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_Animation, RBA_Discard, RBA_Discard, RBA_Discard);

	PawnOwner.BS_SetAnimEndNotify(BS_Animation, FALSE);

	// Turn off Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = PawnOwner.default.Mesh.RootMotionMode;
}


defaultproperties
{
	BS_Animation=(AnimName[BS_FullBody]="AR_Injured_React1")

	bShouldAbortWeaponReload=FALSE
	bCanFireWeapon=FALSE
	bCameraFocusOnPawn=TRUE
	bLockPawnRotation=TRUE
	bBreakFromCover=TRUE
	bDisableMovement=TRUE
	bRestoreMovementAfterMove=TRUE
}
