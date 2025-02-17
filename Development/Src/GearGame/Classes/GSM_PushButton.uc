
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_PushButton extends GearSpecialMove
	native(SpecialMoves);

/** Body Stances to play */
var()	GearPawn.BodyStance	BS_ButtonPush;

/** Time that 'reaching out' IK Controller takes to blend in. */
var()	float				ReachTime;

/** Distance to pull back the button location towards the player, to avoid hand clipping into the wall. */
var()	float				ReachPullBack;

/** Distance away from trigger that we will walk to to push it. */
var()	float				StandPullBack;

var bool bPlayAnimation;

// C++ functions
cpptext
{
	virtual void TickSpecialMove(FLOAT DeltaTime);
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local vector PawnToTarget;

	Super.SpecialMoveStarted(bForced,PrevMove);

	if( PawnOwner.MyGearWeapon != None && PawnOwner.MyGearWeapon.WeaponType == WT_Heavy )
	{
		bPlayAnimation = FALSE;
		PawnOwner.SetTimer(0.25.f, FALSE, 'EndSpecialMove', Self);
	}
	else
	{
		bPlayAnimation = TRUE;

		// Play body stance animation.
		PawnOwner.BS_Play(BS_ButtonPush, SpeedModifier, 0.15f/SpeedModifier, 0.15f/SpeedModifier, FALSE, TRUE);

		// Enable end of animation notification. This is going to call SpecialMoveEnded()
		PawnOwner.BS_SetAnimEndNotify(BS_ButtonPush, TRUE);

		// Set weapon up 
		PawnOwner.SetWeaponAlert(PawnOwner.AimTimeAfterFiring*2.f);
	}

	// If we found the button - use IK controller to reach for it.
	if( PawnOwner.SpecialMoveLocation != vect(0,0,0) )
	{
		// Get the head to look at the button
		PawnOwner.HeadControl.DesiredTargetLocation = PawnOwner.SpecialMoveLocation;
		PawnOwner.HeadControl.TargetLocation		= PawnOwner.SpecialMoveLocation;
		PawnOwner.HeadControl.SetSkelControlActive(TRUE);

		// Find direction vector from Pawn to button.
		PawnToTarget = PawnOwner.SpecialMoveLocation - PawnOwner.Location;
		PawnToTarget.Z = 0; // Just pull back along X any Y
		PawnToTarget = Normal(PawnToTarget);

		// Pull back IK reach target by a bit
		if( bPlayAnimation )
		{
			PawnOwner.IKCtrl_LeftHand.BlendInTime = ReachTime;
			PawnOwner.IKCtrl_LeftHand.EffectorLocation = PawnOwner.SpecialMoveLocation - (ReachPullBack * PawnToTarget);
			PawnOwner.IKCtrl_LeftHand.SetSkelControlActive(TRUE);
		}

		// Face button
		SetReachPreciseDestination( PawnOwner.SpecialMoveLocation - (StandPullBack * PawnToTarget) );
		SetFacePreciseRotation(Rotator(PawnToTarget), ReachTime/2.f);
	}
}

function EndSpecialMove()
{
	PawnOwner.EndSpecialMove();
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	PawnOwner.BS_SetAnimEndNotify(BS_ButtonPush, FALSE);

	// Turn off head look
	PawnOwner.HeadControl.SetSkelControlActive(FALSE);

	// Make sure this turns off - in case we cancel action early.
	if( bPlayAnimation )
	{
		PawnOwner.IKCtrl_LeftHand.SetSkelControlActive(FALSE);
	}

	// Reset location info
	PawnOwner.SpecialMoveLocation = vect(0,0,0);
}

defaultproperties
{
	BS_ButtonPush=(AnimName[BS_Std_Up]="button_push",AnimName[BS_Std_Idle_Lower]="button_push")
	ReachTime=0.5
	ReachPullBack=20
	StandPullBack=90
	bDisableMovement=TRUE
	bLockPawnRotation=TRUE
}