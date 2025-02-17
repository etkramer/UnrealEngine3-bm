
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Rockworm_EatThatFruit extends GearSpecialMove;

/** Head Chomp animation */
var()	array<GearPawn.BodyStance>	BS_Animations;
var		int							BSIdx;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	BSIdx=0;
	// Play body stance animation.
	PawnOwner.BS_Play(BS_Animations[BSIdx], SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE);

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_Animations[BSIdx], TRUE);
}

function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{

	BSIdx++;
	
	if(BSIdx < BS_Animations.length)
	{
		// Play body stance animation.
		PawnOwner.BS_Play(BS_Animations[BSIdx], SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE);
		PawnOwner.BS_SetAnimEndNotify(BS_Animations[BSIdx], TRUE);
	}
	else
	{
		// By default end this special move.
		`LogSMExt(PawnOwner, GetFuncName() @ Self @ "calling EndSpecialMove()");
		PawnOwner.EndSpecialMove();
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);
	// Disable end of animation notification. 
	PawnOwner.BS_SetAnimEndNotify(BS_Animations[0], FALSE);
}

defaultproperties
{
	DefaultAICommand=class'AICmd_Base_PushedBySpecialMove'
	BS_Animations(0)=(AnimName[BS_FullBody]="Rockworm_Munch_Start")
	BS_Animations(1)=(AnimName[BS_FullBody]="Rockworm_Munch_Idle")
	BS_Animations(2)=(AnimName[BS_FullBody]="Rockworm_Munch_End")
	bBreakFromCover=TRUE
	bDisableCollision=FALSE
	bDisableMovement=TRUE
	bDisablePhysics=TRUE
}