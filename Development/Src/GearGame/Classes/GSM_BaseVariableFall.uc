
/**
 * Base Special Move for variable distance falls.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_BaseVariableFall extends GearSpecialMove
	native(SpecialMoves)
	abstract;

/** Animations to play */
var	const GearPawn.BodyStance	BS_Jump, BS_Fall, BS_Land;

/** Physics to set when Jumping. */
var	EPhysics	JumpingPhysics;
/** Physics to set when Falling. */
var	EPhysics	FallingPhysics;

// Type of movement. Jump, Fall, Land.
enum EMoveType
{
	EMT_Jump,
	EMT_Fall,
	EMT_Land,
};

var EMoveType	MoveType;

/**
 * Distance from impact point, from where to start the landing animation.
 * This tries to anticipate the impact to play a "Pre Land" animation.
 */
var float		PreImpactTime;

// C++ functions
cpptext
{
	virtual void TickSpecialMove(FLOAT DeltaTime);
}


function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	//`log(PawnOwner.WorldInfo.TimeSeconds @ GetFuncName() @ PawnOwner);

	// Steps smoothing code interferes with falling,
	// so we disable it for this special move.
	PawnOwner.bCanDoStepsSmoothing = FALSE;
}

simulated function GearPawn.BodyStance GetJumpBS()
{
	return BS_Jump;
}
simulated function GearPawn.BodyStance GetFallBS()
{
	return BS_Fall;
}
simulated function GearPawn.BodyStance GetLandBS()
{
	return BS_Land;
}

/** Notification called when we're mirror transition safe. */
function OnMirrorTransitionSafeNotify()
{
	// Start with jump animation
	PlayJump();
}

/** Play Jump. */
simulated function PlayJump()
{
	//`log(PawnOwner.WorldInfo.TimeSeconds @ GetFuncName() @ PawnOwner);

	MoveType = EMT_Jump;

	// Play Jump animation.
	PawnOwner.BS_Play(GetJumpBS(), SpeedModifier, 0.2f/SpeedModifier, 0.f, FALSE, TRUE);

	// Set flag to get notification when animation is done playing
	PawnOwner.BS_SetAnimEndNotify(GetJumpBS(), TRUE);

	// Set Jumping physics
	PawnOwner.SetPhysics(JumpingPhysics);
	PawnOwner.SetBase( PawnOwner.ClampedBase );
}


/** Stop Jump animation and perform any clean up to restore Pawn is original state. */
simulated function StopJump()
{
	// Stop Jump animation.
	PawnOwner.BS_Stop(GetJumpBS(), 0.2f);

	// Turn off animend notification flag
	PawnOwner.BS_SetAnimEndNotify(GetJumpBS(), FALSE);
}


/** Play Fall */
simulated function PlayFall()
{
	//`log(PawnOwner.WorldInfo.TimeSeconds @ GetFuncName() @ PawnOwner);

	// Stop current move
	StopCurrentMoveType();

	// Now we're falling.
	MoveType = EMT_Fall;

	// Set falling physics
	PawnOwner.SetPhysics(FallingPhysics);
	PawnOwner.SetBase( PawnOwner.ClampedBase );

	// Play middle, variable animation.
	PawnOwner.BS_Play(GetFallBS(), SpeedModifier, 0.f, -1.f, IsFallLooping(), TRUE);
}

simulated function bool IsFallLooping()
{
	return FALSE;
}

/** Stop Jump animation and perform any clean up to restore Pawn is original state. */
simulated function StopFall()
{
	// Stop Fall animation.
	PawnOwner.BS_Stop(GetFallBS(), 0.2f);
}


/** Play Land */
simulated function PlayLand()
{
	//`log(PawnOwner.WorldInfo.TimeSeconds @ GetFuncName() @ PawnOwner);

	// Stop current move
	StopCurrentMoveType();

	// now we've landed
	MoveType = EMT_Land;

	// Play middle, variable animation.
	PawnOwner.BS_Play(GetLandBS(), SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE);

	// Set flag to get notification when animation is done playing
	PawnOwner.BS_SetAnimEndNotify(GetLandBS(), TRUE);
}


/** Stop Land animation and perform any clean up to restore Pawn is original state. */
simulated function StopLand()
{
	// Stop Fall animation.
	PawnOwner.BS_Stop(GetLandBS(), 0.2f);

	// Turn off animend notification flag
	PawnOwner.BS_SetAnimEndNotify(GetLandBS(), FALSE);
}


/** Notification called when body stance animation finished playing */
simulated function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	//`log(PawnOwner.WorldInfo.TimeSeconds @ GetFuncName() @ PawnOwner);

	if( MoveType == EMT_Jump )
	{
		PlayFall();
	}
	// If we're done playing the land animation, we can end this special move.
	else if( MoveType == EMT_Land )
	{
		PawnOwner.EndSpecialMove();
	}
}


/**
 * Event called when Impact with world has happened or has been anticipated.
 * (This is when PawnOwner is within PreImpactDistance units from impact point)
 */
event Landed(float DistanceToImpact, float TimeToImpact)
{
	//`log(PawnOwner.WorldInfo.TimeSeconds @ GetFuncName() @ PawnOwner);

	PlayLand();
}


/**
 * Stop current move type, to restore Pawn to its original state.
 * This is called during move type transitions, and when the special move is stopped.
 */
simulated function StopCurrentMoveType()
{
	switch( MoveType )
	{
		case EMT_Jump : StopJump(); break;
		case EMT_Fall : StopFall(); break;
		case EMT_Land : StopLand(); break;
	}
}


/** Special move has ended. */
function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Stop current move type.
	StopCurrentMoveType();

	// failsafe
	if ( PawnOwner.Physics == PHYS_Flying )
	{
		PawnOwner.SetPhysics(PHYS_Falling);
		PawnOwner.SetBase( PawnOwner.ClampedBase );
	}
	
	// Restore steps smoothing if Pawn had it by default
	PawnOwner.bCanDoStepsSmoothing = PawnOwner.default.bCanDoStepsSmoothing;

	//`log(PawnOwner.WorldInfo.TimeSeconds @ GetFuncName() @ PawnOwner);
}


defaultproperties
{
	BS_Jump=(AnimName[BS_FullBody]="Jump")
	BS_Fall=(AnimName[BS_FullBody]="Fall")
	BS_Land=(AnimName[BS_FullBody]="Land")

	JumpingPhysics=PHYS_Falling
	FallingPhysics=PHYS_Falling

	PreImpactTime=0.f
	bMirrorTransitionSafeNotify=TRUE
}
