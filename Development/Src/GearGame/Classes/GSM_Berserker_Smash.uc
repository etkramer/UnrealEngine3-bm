
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Berserker_Smash extends GearSpecialMove
	DependsOn(AnimSequence)
	native(SpecialMoves);

// C++ functions
cpptext
{
	virtual void TickSpecialMove(FLOAT DeltaTime);
}

var()	GearPawn.BodyStance	BS_Animation;

/** Old location of hands for traces during melee attack */
var()	Vector RHand_OldLocation, LHand_OldLocation;
/** Damage of attack is active */
var()	bool	bDamageActive;

var(Debug) bool bDebugLines;


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

	RHand_OldLocation = vect(0,0,0);
	LHand_OldLocation = vect(0,0,0);
}

event MeleeDamageTo( Actor Other, Vector HitLocation )
{
	local GearAI_Berserker AI;

	AI = GearAI_Berserker(PawnOwner.Controller);
	if( AI != None )
	{
		AI.NotifyBerserkerSmashCollision( Other, HitLocation );
	}
}

defaultproperties
{
	bDamageActive=TRUE

	BS_Animation=(AnimName[BS_FullBody]="Swinging_Attack")
}
