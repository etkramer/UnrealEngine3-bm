/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GSM_Nemaslug_Emerge extends GearSpecialMove
	config(Pawn);


/** animation */
var()	GearPawn.BodyStance	BS_Animation;


function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	// Play body stance animation.
	PawnOwner.BS_Play(BS_Animation, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE);

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, TRUE);


	PawnOwner.SetPhysics(Phys_None);
	TogglePawnCollision(PawnOwner, FALSE);
	PawnOwner.BS_SetRootBoneAxisOptions(BS_Animation, RBA_Translate, RBA_Translate, RBA_Translate);
	PawnOwner.Mesh.RootMotionMode = RMM_Translate;
}


function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	PawnOwner.SetPhysics(Phys_Falling);
	// Disable end of animation notification. 
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, FALSE);
	PawnOwner.Mesh.RootMotionMode = PawnOwner.default.Mesh.RootMotionMode;
	TogglePawnCollision(PawnOwner, TRUE);
}


defaultproperties
{
	BS_Animation=(AnimName[BS_FullBody]="NS_Emerge")
	
	bLockPawnRotation=TRUE
	bDisableMovement=TRUE
	bDisablePhysics=TRUE

	DefaultAICommand=class'AICmd_Base_PushedBySpecialMove'
}