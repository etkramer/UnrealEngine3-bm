
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Berserker_Collide extends GearSpecialMove;

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

	// Play body stance animation.
	PawnOwner.BS_Play(BS_Animation, AnimRate, 0.2f/AnimRate, 0.2f/AnimRate, FALSE, TRUE);

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, TRUE);

	// Have animation forward root motion.
	PawnOwner.BS_SetRootBoneAxisOptions(BS_Animation, RBA_Translate, RBA_Translate);

	// Turn on Root motion on mesh. Use RMM_Accel, to drive full movement.
	PawnOwner.Mesh.RootMotionMode = RMM_Accel;
}


function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Turn off Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_Animation, RBA_Discard, RBA_Discard, RBA_Discard);

	// Disable end of animation notification.
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, FALSE);

	// Stop animation if it didn't already
	PawnOwner.BS_Stop(BS_Animation, 0.2f);

	// Restore default root motion mode
	PawnOwner.Mesh.RootMotionMode = PawnOwner.Mesh.default.RootMotionMode;
}


defaultproperties
{
	BS_Animation=(AnimName[BS_FullBody]="Charge_Hit")
}
