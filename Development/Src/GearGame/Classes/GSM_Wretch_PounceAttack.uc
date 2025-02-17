
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Wretch_PounceAttack extends GearSpecialMove;

/** Mantle over animation */
var()	GearPawn.BodyStance	BS_Animation;
var()	float				RootMotionScaleFactor;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local GearAI AI;

	Super.SpecialMoveStarted(bForced,PrevMove);

	GearPawn_LocustWretchBase(PawnOwner).PlayLeapSound();
	GearPawn_LocustWretchBase(PawnOwner).PlaySwipeAttackSound();

	// reset animation mirroring
	PawnOwner.SetMirroredSide(FALSE);

	// Play body stance animation.
	PawnOwner.BS_Play(BS_Animation, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE);

	// Turn on Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_Animation, RBA_Translate, RBA_Translate, RBA_Translate);

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, TRUE);

	// Turn on Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = RMM_Accel;
	// Scale root motion translation by ScaleFactor, except for Z component.
	PawnOwner.Mesh.RootMotionAccelScale.X = RootMotionScaleFactor;
	PawnOwner.Mesh.RootMotionAccelScale.Y = RootMotionScaleFactor;
	PawnOwner.Mesh.RootMotionAccelScale.Z = 1.f;
	

	AI = GearAI(PawnOwner.Controller);
	if( AI != None )
	{
		// Tell AI to wait for attack to finish
		AI.WaitForEvent( 'EndSwipeAttack' );
	}
}


function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	local GearAI AI;

	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Scale root motion translation by ScaleFactor, except for Z component.
	PawnOwner.Mesh.RootMotionAccelScale.X = RootMotionScaleFactor;
	PawnOwner.Mesh.RootMotionAccelScale.Y = RootMotionScaleFactor;
	PawnOwner.Mesh.RootMotionAccelScale.Z = 1.f;


	// Turn off Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_Animation, RBA_Discard, RBA_Discard, RBA_Discard);

	// Disable end of animation notification. 
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, FALSE);

	// Turn off Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = PawnOwner.default.Mesh.RootMotionMode;

	AI = GearAI(PawnOwner.Controller);
	if( AI != None )
	{
		// Tell AI attack is finished
		AI.ReceiveEvent('EndSwipeAttack');
	}

}

defaultproperties
{
	BS_Animation=(AnimName[BS_FullBody]="Jump_Swipe")

	bLockPawnRotation=TRUE
	bBreakFromCover=TRUE
	bDisableMovement=TRUE
	RootMotionScaleFactor=0.5f
}
