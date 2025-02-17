
/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GSM_NemaSlug_Melee extends GearSpecialMove;

/** animation */
var() array<Name> AttackAnimations;
var()	GearPawn.BodyStance	BS_Animation;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	BS_Animation.AnimName[BS_FullBody] = AttackAnimations[rand(AttackAnimations.length)];
	PawnOwner.BS_Play(BS_Animation, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE);

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, TRUE);
}


function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Disable end of animation notification. 
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, FALSE);
}

defaultproperties
{
	AttackAnimations(0)="NS_AttackA"
	AttackAnimations(1)="NS_AttackB"

	bLockPawnRotation=TRUE
	bDisableCollision=FALSE
	bDisableMovement=TRUE
}
