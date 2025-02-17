
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Berserker_Slide extends GearSpecialMove;

var()	GearPawn.BodyStance	BS_Animation;

/** Get Animation Play Rate */
simulated function float GetAnimPlayRate()
{
	local GearPawn_LocustBerserkerBase	Berserker;

	Berserker = GearPawn_LocustBerserkerBase(PawnOwner);

	// When heated up, animations play faster
	if( Berserker != None && Berserker.bVulnerableToDamage )
	{
		return Berserker.HeatedUpSpeedScale * SpeedModifier;
	}

	return SpeedModifier;
}


function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local float AnimRate;

	Super.SpecialMoveStarted(bForced,PrevMove);

	AnimRate = GetAnimPlayRate();

	// Play Slide animation
	PawnOwner.BS_Play(BS_Animation, AnimRate, 0.2f/AnimRate, 0.2f/AnimRate, FALSE, TRUE);

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, TRUE);

	// Have animation forward root motion.
	PawnOwner.BS_SetRootBoneAxisOptions(BS_Animation, RBA_Translate, RBA_Translate);

	// Use root rotation to rotate the actor.
	PawnOwner.BodyStanceNodes[BS_FullBody].GetCustomAnimNodeSeq().RootRotationOption[2] = RRO_Extract;
	PawnOwner.Mesh.RootMotionRotationMode = RMRM_RotateActor;

	// Turn on Root motion on mesh. Use RMM_Accel, to drive full movement.
	PawnOwner.Mesh.RootMotionMode = RMM_Accel;
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Reset root rotation
	PawnOwner.BodyStanceNodes[BS_FullBody].GetCustomAnimNodeSeq().RootRotationOption[2] = RRO_Default;
	PawnOwner.Mesh.RootMotionRotationMode = RMRM_Ignore;

	// Disable end of animation notification.
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, FALSE);

	// Restore default root motion mode
	PawnOwner.Mesh.RootMotionMode = PawnOwner.Mesh.default.RootMotionMode;

	// Turn off Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_Animation, RBA_Discard, RBA_Discard, RBA_Discard);

	// Stop animation if it's still playing
	PawnOwner.BS_Stop(BS_Animation, 0.2f);
}

defaultproperties
{
	BS_Animation=(AnimName[BS_FullBody]="Charge_Miss")
}