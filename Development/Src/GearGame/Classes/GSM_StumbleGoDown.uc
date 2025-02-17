
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_StumbleGoDown extends GearSpecialMove;

/** Animations to play. */
var()	GearPawn.BodyStance	BS_DropAnimation;
var()	GearPawn.BodyStance	BS_Idle;

protected function bool InternalCanDoSpecialMove()
{
	if( PawnOwner.SpecialMove != SM_None && !PawnOwner.IsDoingMove2IdleTransition() && !PawnOwner.IsDoingStumbleFromMelee() )
	{
		return FALSE;
	}

	return TRUE;
}

function bool CanOverrideMoveWith(ESpecialMove NewMove)
{
	if( NewMove == SM_StumbleGetUp )
	{
		return TRUE;
	}

	return FALSE;
}


function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	// Play body stance animation.
	PawnOwner.BS_Play(BS_DropAnimation, SpeedModifier, 0.2f/SpeedModifier, 0.f, FALSE, TRUE);

	// Turn on root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_DropAnimation, RBA_Translate, RBA_Translate, RBA_Translate);

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_DropAnimation, TRUE);

	// Turn on Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = RMM_Accel;
}

function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	// Stop drop animation
	StopDropAnimation();

	// transition to loop idle animation 
	PawnOwner.BS_Play(BS_Idle, SpeedModifier, 0.f, 0.f, TRUE, TRUE);
	PawnOwner.BS_SetPosition(BS_Idle, ExcessTime * PawnOwner.BS_GetPlayRate(BS_Idle));
}

function StopDropAnimation()
{
	// Stop Pawn
	PawnOwner.ZeroMovementVariables();

	// Turn off Root Motion
	PawnOwner.Mesh.RootMotionMode = PawnOwner.default.Mesh.RootMotionMode;
	PawnOwner.BS_SetRootBoneAxisOptions(BS_DropAnimation, RBA_Discard, RBA_Discard, RBA_Discard );
	
	// Stop animation
	PawnOwner.BS_SetAnimEndNotify(BS_DropAnimation, FALSE);
	PawnOwner.BS_Stop(BS_DropAnimation, 0.2f);
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	// Stop drop animation
	StopDropAnimation();

	// Stop looping animation
	PawnOwner.BS_Stop(BS_Idle, 0.2f);

	Super.SpecialMoveEnded(PrevMove, NextMove);
}


defaultproperties
{
	BS_DropAnimation=(AnimName[BS_FullBody]="AR_Injured_Drop")
	BS_Idle=(AnimName[BS_FullBody]="AR_Injured_Idle")

	bDisableAI=TRUE
	DefaultAICommand=class'AICmd_Base_PushedBySpecialMove'

	bShouldAbortWeaponReload=TRUE
	bCanFireWeapon=FALSE
	bCameraFocusOnPawn=TRUE
	bLockPawnRotation=TRUE
	bBreakFromCover=TRUE
	bDisableMovement=TRUE
}
