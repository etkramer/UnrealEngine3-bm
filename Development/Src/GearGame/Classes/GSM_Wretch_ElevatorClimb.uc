/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class GSM_Wretch_ElevatorClimb extends GearSpecialMove;

/** Mantle over animation */
var()	GearPawn.BodyStance	BS_Animation;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	// Play body stance animation.
	PawnOwner.BS_Play(BS_Animation, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE);

	// Turn on root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_Animation, RBA_Translate, RBA_Translate);

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, TRUE);

	// Turn on Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = RMM_Accel;
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Turn off Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions( BS_Animation, RBA_Discard, RBA_Discard, RBA_Discard );

	PawnOwner.BS_SetAnimEndNotify(BS_Animation, FALSE);

	// Turn off Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = PawnOwner.default.Mesh.RootMotionMode;
}

defaultproperties
{
	BS_Animation=(AnimName[BS_FullBody]="Pole_Climb")

	bShouldAbortWeaponReload=TRUE
	bCanFireWeapon=FALSE

	bLockPawnRotation=TRUE
	bBreakFromCover=TRUE
	bDisableCollision=FALSE
	bDisableMovement=TRUE
}