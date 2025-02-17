
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_EvadeRt extends GSM_Evade;

/** If TRUE, we are doing a mirroring transition. */
var bool				bDoMirroringTransition;

protected function bool InternalCanDoSpecialMove()
{
	return InternalCanDoSpecialMoveEvade_Worker(SM_EvadeRt);
}

simulated function PreProcessInput(GearPlayerInput Input)
{
	// call the super class's version
	Super.PreProcessInput(Input);

	// force running to the right
	Input.aStrafe	= default.AccelScale;

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
	}
	else
	{
		bDoMirroringTransition = FALSE;
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
	BS_EvadeStart=(AnimName[BS_FullBody]="AR_Evade_Rt_Start")
	BS_EvadeEnd=(AnimName[BS_FullBody]="AR_Evade_Rt_End")

	bCoverExitMirrorTransition=TRUE
}
