
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Wretch_LeapToCeiling extends GSM_BaseVariableFall;

/** Notification called when body stance animation finished playing */
simulated function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	// If Wretch has already reached ceiling, don't overwrite physics.
	if( MoveType != EMT_Land && PawnOwner.Physics == PHYS_Spider )
	{
		Landed(0.f, 0.f);
	}
	else
	{
		Super.BS_AnimEndNotify(SeqNode, PlayedTime, ExcessTime);
	}
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	PawnOwner.Mesh.RootMotionMode = RMM_Ignore;
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// reset animation mirroring
	PawnOwner.SetMirroredSide( FALSE );
	PawnOwner.LastEvadeTime = PawnOwner.WorldInfo.TimeSeconds;
	PawnOwner.Mesh.RootMotionMode = PawnOwner.default.Mesh.RootMotionMode; 
}

simulated function PlayFall()
{
	local GearAI_Wretch AI;

	super.PlayFall();

	AI = GearAI_Wretch(PawnOwner.Controller);
	if( AI != None )
	{
		AI.NotifyCeilingTransitionFall();
	}
}


defaultproperties
{
	BS_Jump=(AnimName[BS_FullBody]="Ceiling_Jump_Start")
	BS_Fall=(AnimName[BS_FullBody]="Ceiling_Jump_Fall")
	BS_Land=(AnimName[BS_FullBody]="Ceiling_Jump_Land")

	PreImpactTime=0.35f

	JumpingPhysics=PHYS_None
}
