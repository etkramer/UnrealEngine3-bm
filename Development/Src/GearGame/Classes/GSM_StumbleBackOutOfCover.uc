
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_StumbleBackOutOfCover extends GearSpecialMove;

/** Mantle over animation */
var()	GearPawn.BodyStance	BS_Animation;

protected function bool InternalCanDoSpecialMove()
{
	if( !PawnOwner.IsInCover() )
	{
		return FALSE;
	}

	return TRUE;
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local GearPC PC;
	local rotator SavedRot;

	Super.SpecialMoveStarted(bForced,PrevMove);

	SavedRot = PawnOwner.Rotation;

	PC = GearPC(PawnOwner.Controller);
	if (PC != None)
	{
		PC.LeaveCover();
	}
	PawnOwner.SetRotation(SavedRot);

	// Play body stance animation.
	PawnOwner.BS_Play(BS_Animation, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE);

	// Turn on root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_Animation, RBA_Translate, RBA_Translate );

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, TRUE);

	// Turn on Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = RMM_Accel;
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Turn off Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_Animation, RBA_Discard, RBA_Discard, RBA_Discard );

	PawnOwner.BS_SetAnimEndNotify(BS_Animation, FALSE);

	// Turn off Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = PawnOwner.default.Mesh.RootMotionMode;
}


defaultproperties
{
	BS_Animation=(AnimName[BS_FullBody]="AR_Injured_ReactToMantle")

	bShouldAbortWeaponReload=TRUE
	bCanFireWeapon=FALSE
	bCameraFocusOnPawn=TRUE
	bLockPawnRotation=TRUE
	bBreakFromCover=TRUE
	bDisableMovement=TRUE
}
