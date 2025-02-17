
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Wretch_Scream extends GearSpecialMove;

/** Mantle over animation */
var()	GearPawn.BodyStance	BS_Animation;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	// Play body stance animation.
	PawnOwner.BS_Play(BS_Animation, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE);

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, TRUE);

	GearPawn_LocustWretchBase(PawnOwner).PlayScreamSound();
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Disable end of animation notification. 
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, FALSE);

	PawnOwner.ClearTimer( 'LoopScreamEffect' );
}

defaultproperties
{
	BS_Animation=(AnimName[BS_FullBody]="Scream")

	bLockPawnRotation=TRUE
	bBreakFromCover=TRUE
	bDisableCollision=FALSE
	bDisableMovement=TRUE
	bDisablePhysics=TRUE
}