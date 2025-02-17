
/**
 * Base Special Move to play a single animation, 
 * and end itself once it's done playing. 
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_BasePlaySingleAnim extends GearSpecialMove
	native(SpecialMoves)
	abstract;

/** Animation to play */
var()	GearPawn.BodyStance	BS_Animation;
var		FLOAT				BlendInTime;
var		FLOAT				BlendOutTime;

/** Name of camera animation to play when this special move starts. */
var protected array<CameraBoneAnimation> SpecialMoveCameraBoneAnims;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced, PrevMove);

	// Play animation
	PlayAnimation();
}

function PlayAnimation()
{
	// Play Animation
	PawnOwner.BS_Play(BS_Animation, SpeedModifier, BlendInTime/SpeedModifier, BlendOutTime/SpeedModifier, FALSE);

	// Enable end of animation notification. This is going to call Pawn.EndSpecialMove().
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, TRUE);
	
	if( PCOwner != None )
	{
		if( SpecialMoveCameraBoneAnims.Length > 0 )
		{
			PCOwner.PlayRandomCameraBoneAnim(SpecialMoveCameraBoneAnims, SpeedModifier, BlendInTime/SpeedModifier, BlendOutTime/SpeedModifier,,, TRUE, TRUE, TRUE);
		}
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Disable end of animation notification, so it doesn't interfere with future special moves
	// using that same animation channel.
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, FALSE);
}

defaultproperties
{
	BlendInTime=0.2f
	BlendOutTime=0.2f
}
