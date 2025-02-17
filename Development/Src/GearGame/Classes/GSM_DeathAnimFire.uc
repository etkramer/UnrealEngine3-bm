
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_DeathAnimFire extends GSM_DeathAnimBase;

var BodyStance	BS_Stand2Kneel, BS_Kneel2Lying, BS_LyingLoop;

/** Restrictions for doing Death Animation. */
protected function bool InternalCanDoSpecialMove()
{
	// We need a mesh to do any of this.
	if( PawnOwner.Mesh == None || PawnOwner.SpecialMoveWhenDead == SM_MidLvlJumpOver )
	{
		return FALSE;
	}
	
	// Pawn has to be burning to do this type of death
	if( PawnOwner.CurrentSkinHeat < 0.1f )
	{
		return FALSE;
	}

	return TRUE;
}

/** This special move can override pretty much any other special moves. */
function bool CanOverrideSpecialMove(ESpecialMove InMove)
{
	return TRUE;
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced, PrevMove);

	// Those look better when blending to the kneeling animation straight.
	if( PawnOwner.SpecialMoveWhenDead == SM_DBNO )
	{
		PawnOwner.BS_Play(BS_Kneel2Lying, 1.f, 0.5f, 0.33f);
		PawnOwner.BS_SetAnimEndNotify(BS_Kneel2Lying, TRUE);
		DeathAnimBlendToMotors_Writhing();
	}
	// Otherwise do stand 2 kneel animation.
	else
	{
		PawnOwner.BS_Play(BS_Stand2Kneel, 1.f, 0.33f, 0.33f);
		PawnOwner.BS_SetAnimEndNotify(BS_Stand2Kneel, TRUE);
	}
}

function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	if( PawnOwner.BS_SeqNodeBelongsTo(SeqNode, BS_Stand2Kneel) )
	{
		PawnOwner.BS_Play(BS_Kneel2Lying, 1.f, 0.33f, 0.33f);
		PawnOwner.BS_SetPosition(BS_Kneel2Lying, ExcessTime * PawnOwner.BS_GetPlayRate(BS_Kneel2Lying));
		PawnOwner.BS_SetAnimEndNotify(BS_Kneel2Lying, TRUE);
		DeathAnimBlendToMotors_Writhing();
	}
	else if( PawnOwner.BS_SeqNodeBelongsTo(SeqNode, BS_Kneel2Lying) )
	{
		PawnOwner.BS_Play(BS_LyingLoop, 1.f, 0.33f, -1.f);
		PawnOwner.BS_SetPosition(BS_LyingLoop, ExcessTime * PawnOwner.BS_GetPlayRate(BS_Kneel2Lying));
		PawnOwner.BS_SetAnimEndNotify(BS_LyingLoop, TRUE);

		// Turn off bone springs
		SetSpringForBone(PawnOwner.PelvisBoneName, FALSE);
		SetSpringForBone(PawnOwner.RightHandBoneName, FALSE);
		SetSpringForBone(PawnOwner.LeftHandBoneName, FALSE);
		SetSpringForBone(PawnOwner.LeftFootBoneName, FALSE);
		SetSpringForBone(PawnOwner.RightFootBoneName, FALSE);
	}
	else
	{
		DeathAnimRagDoll();
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	PawnOwner.BS_SetAnimEndNotify(BS_Stand2Kneel, FALSE);
	PawnOwner.BS_SetAnimEndNotify(BS_Kneel2Lying, FALSE);
	PawnOwner.BS_SetAnimEndNotify(BS_LyingLoop, FALSE);

	Super.SpecialMoveEnded(PrevMove, NextMove);
}

function DeathAnimBlendToMotors_Writhing()
{
	// Setup a bunch of springs to have physics match animation (as long as we are standing on non-moving thing)
	// Don't do it if we have broken constraints. We could figure out which ones are broken... For now, just do nothing.
	if( PawnOwner.Base == None && !PawnOwner.Base.bMovable && !PawnOwner.bHasBrokenConstraints )
	{
		SetSpringForBone(PawnOwner.PelvisBoneName, TRUE);
		SetSpringForBone(PawnOwner.RightHandBoneName, TRUE);
		SetSpringForBone(PawnOwner.LeftHandBoneName, TRUE);
// 		SetSpringForBone(PawnOwner.LeftFootBoneName, TRUE);
// 		SetSpringForBone(PawnOwner.RightFootBoneName, TRUE);
	}

	Super.DeathAnimBlendToMotors();
}

function DeathAnimRagDoll()
{
	// Turn off bone springs
	SetSpringForBone(PawnOwner.PelvisBoneName, FALSE);
	SetSpringForBone(PawnOwner.RightHandBoneName, FALSE);
	SetSpringForBone(PawnOwner.LeftHandBoneName, FALSE);
	SetSpringForBone(PawnOwner.LeftFootBoneName, FALSE);
	SetSpringForBone(PawnOwner.RightFootBoneName, FALSE);

	Super.DeathAnimRagDoll();

	PawnOwner.EndSpecialMove();
}



/** Notification forwarded from RB_BodyInstance, when a spring is over extended and disabled. */
function OnRigidBodySpringOverextension(RB_BodyInstance BodyInstance)
{
	local Name				PelvisBoneName;
	local RB_BodyInstance	PelvisBodyInstance;

	// Make sure we have correct bone name if this is a socket.
	PelvisBoneName = PawnOwner.Mesh.GetSocketBoneName(PawnOwner.PelvisBoneName);
	PelvisBodyInstance = PawnOwner.Mesh.FindBodyInstanceNamed(PelvisBoneName);

	// If Pelvis spring was broken, we can't keep the other limbs (hands & legs) under spring influence, it would look too bad.
	// So we just turn them off and force the Pawn into rag doll.
	if( PelvisBodyInstance == BodyInstance )
	{
		DeathAnimRagDoll();
	}
}

defaultproperties
{
	BS_Stand2Kneel=(AnimName[BS_FullBody]="Death_Fire_Stand2Kneel")
	BS_Kneel2Lying=(AnimName[BS_FullBody]="Death_Fire_Kneel2Lying")
	BS_LyingLoop=(AnimName[BS_FullBody]="Death_Fire_LyingLoop")
}