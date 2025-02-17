
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_EvadeLt extends GSM_Evade;

// Animations to play
var	GearPawn.BodyStance	BS_EvadeStart_NonMirrored, BS_EvadeEnd_NonMirrored;
var	GearPawn.BodyStance	BS_EvadeStart_Mirrored, BS_EvadeEnd_Mirrored;

/** If TRUE, we are doing a mirroring transition. */
var bool				bDoMirroringTransition;

protected function bool InternalCanDoSpecialMove()
{
	return InternalCanDoSpecialMoveEvade_Worker(SM_EvadeLt);
}

simulated function PreProcessInput(GearPlayerInput Input)
{
	// call the super class's version
	super.PreProcessInput(Input);

	// force running to the left
	Input.aStrafe	= -default.AccelScale;

	// forward becomes turning
	//Input.aTurn		= Input.aBaseY * default.EvadeTurnPct;
	//Input.aBaseY	= 0.f;
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	// Play proper animations depending on mirroring or not
	if( PawnOwner.bIsMirrored )
	{
		bDoMirroringTransition = TRUE;
		BS_EvadeStart	= BS_EvadeStart_Mirrored;
		BS_EvadeEnd		= BS_EvadeEnd_Mirrored;
	}
	else
	{
		bDoMirroringTransition = FALSE;
		BS_EvadeStart	= BS_EvadeStart_NonMirrored;
		BS_EvadeEnd		= BS_EvadeEnd_NonMirrored;
	}

	Super.SpecialMoveStarted(bForced,PrevMove);

	if( bDoMirroringTransition )
	{
		// Play animation mirrored
		PawnOwner.BS_SetMirrorOptions(BS_EvadeStart, FALSE, TRUE, TRUE);

		// This is a super hack to not trigger the mirror transition blend out just now.
		// We want to play 2 animations in a row. ugh!
		PawnOwner.MirrorNode.bLockBlendOut = TRUE;
	}
}


/** Notification called when body stance animation finished playing */
simulated function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	if( bPlayStartAnimation )
	{
		// Turn off mirroring on this node.
		PawnOwner.BS_SetMirrorOptions(BS_EvadeStart, FALSE, FALSE, FALSE);
	}

	Super.BS_AnimEndNotify(SeqNode, PlayedTime, ExcessTime);
}


function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	if( bDoMirroringTransition )
	{
		// This is a super hack to not trigger the mirror transition blend out just now.
		// We want to play 2 animations in a row. ugh!
		PawnOwner.MirrorNode.bLockBlendOut = FALSE;
	}
}

defaultproperties
{
	BS_EvadeStart_NonMirrored=(AnimName[BS_FullBody]="AR_Evade_Lt_Start")
	BS_EvadeEnd_NonMirrored=(AnimName[BS_FullBody]="AR_Evade_Lt_End")

	BS_EvadeStart_Mirrored=(AnimName[BS_FullBody]="AR_Evade_Lt_Mirrored_Start")
	BS_EvadeEnd_Mirrored=(AnimName[BS_FullBody]="AR_Evade_Lt_Mirrored_End")

	bCoverExitMirrorTransition=TRUE
}
