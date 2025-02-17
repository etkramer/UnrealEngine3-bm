
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_BloodMount_Melee extends GearSpecialMove;

/** animation */
var()	GearPawn.BodyStance	BS_Animation;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local GearPawn	Rider;
	
	Super.SpecialMoveStarted(bForced,PrevMove);

	// Play body stance animation.
	PawnOwner.BS_Play(BS_Animation, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE);
	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, TRUE);

	Rider = GearPawn_LocustBloodmount(PawnOwner).Driver;
	if( Rider != None )
	{
		Rider.BS_Play(BS_Animation, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE);
	}
}


function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Disable end of animation notification. 
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, FALSE);
}

defaultproperties
{
	BS_Animation=(AnimName[BS_FullBody]="Melee")

	bLockPawnRotation=TRUE
	bDisableMovement=TRUE
	bDisablePhysics=TRUE
}