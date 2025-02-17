
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_CoverHead extends GearSpecialMove
	config(Pawn);

/** Mantle over animation */
var()	array<GearPawn.BodyStance>	BS_Start;
var()	array<GearPawn.BodyStance>	BS_Loop;
var()	array<GearPawn.BodyStance>	BS_End;

var()	int	VariationIndex;

/** This is how long a pawn will spend covering their head when doing this SpecialMove **/
var() config float TimeSpentCoveringHead;


protected function bool InternalCanDoSpecialMove()
{
	// don't cover head if in cover
	if( !Super.InternalCanDoSpecialMove() || PawnOwner.IsInCover() || PawnOwner.DrivenVehicle != None )
	{
		return FALSE;
	}

	return TRUE;
}

/** 
 * Basically an after-the-fact override for TimeSpendCoveringHead.
 * This will reset active timers, so it's best to call this right after starting the special move. 
 */
function SetCustomDuration(float Duration)
{
	local float StartDuration, LoopDuration;

	if (PawnOwner.IsTimerActive('StartAnimDone', Self))
	{
		// restart it
		StartDuration = PawnOwner.GetTimerRate('StartAnimDone', Self);
		PawnOwner.SetTimer( StartDuration, FALSE, nameof(self.StartAnimDone), self );
	}

	LoopDuration = Duration - StartDuration;

	if (PawnOwner.Role == ROLE_Authority)
	{
		PawnOwner.SetTimer( LoopDuration,  FALSE, nameof(self.LoopAnimDone), self );
	}
}

/** This will stop the special move while still playing the transition-out anim. */
function StopCoveringHead()
{
	if (PawnOwner.IsTimerActive('StartAnimDone', Self))
	{
		// haven't made it to the loop yet
		StartAnimDone();
		LoopAnimDone();

		PawnOwner.ClearTimer('StartAnimDone', Self);
		PawnOwner.ClearTimer('LoopAnimDone', Self);

	}
	else if (PawnOwner.IsTimerActive('LoopAnimDone', Self))
	{
		LoopAnimDone();
		PawnOwner.ClearTimer('LoopAnimDone', Self);
	}
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local float StartDuration, LoopDuration;

	Super.SpecialMoveStarted(bForced,PrevMove);

	// Pick up a random set of animations
	VariationIndex = Rand(BS_Start.Length);

	// Play intro animation
	StartDuration = PawnOwner.BS_Play(BS_Start[VariationIndex], SpeedModifier, 0.2f/SpeedModifier, -1.f, FALSE, TRUE);

	// Go to loop animation in x seconds and loop for a total of x seconds
	LoopDuration  = StartDuration + TimeSpentCoveringHead;		// should be determined by difficulty

	PawnOwner.SetTimer( StartDuration, FALSE, nameof(self.StartAnimDone), self );

	// server decides how long to play this.  clients will stop this special move
	// when they hear the special move is finished via replication on the pawn.
	if (PawnOwner.Role == ROLE_Authority)
	{
		PawnOwner.SetTimer( LoopDuration,  FALSE, nameof(self.LoopAnimDone), self );
	}
}

function StartAnimDone()
{
	PawnOwner.BS_Stop(BS_Start[VariationIndex], 0.2f/SpeedModifier);
	PawnOwner.BS_Play(BS_Loop[VariationIndex], SpeedModifier, 0.2f/SpeedModifier, 0.f, TRUE, TRUE);
}

function LoopAnimDone()
{
	PawnOwner.ServerEndSpecialMove();
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	PawnOwner.BS_SetAnimEndNotify(BS_End[VariationIndex], FALSE);

	// Stop cring anims, play the end cringe animation.
	PawnOwner.BS_Stop(BS_Start[VariationIndex], 0.2f/SpeedModifier);
	PawnOwner.BS_Stop(BS_Loop[VariationIndex], 0.2f/SpeedModifier);
	PawnOwner.BS_Play(BS_End[VariationIndex], SpeedModifier, 0.2f/SpeedModifier, 0.3f, FALSE, TRUE);

	PawnOwner.ClearTimer('StartAnimDone', Self);
	PawnOwner.ClearTimer('LoopAnimDone', Self);
}

defaultproperties
{
	BS_Start=((AnimName[BS_FullBody]="ar_injured_headcover1_start"),(AnimName[BS_FullBody]="ar_injured_headcover2_start"))
	BS_Loop=((AnimName[BS_FullBody]="ar_injured_headcover1_idle"),(AnimName[BS_FullBody]="ar_injured_headcover2_idle"))
	BS_End=((AnimName[BS_FullBody]="ar_injured_headcover1_end"),(AnimName[BS_FullBody]="ar_injured_headcover2_end"))

	bShouldAbortWeaponReload=TRUE
	bCanFireWeapon=FALSE

	bBreakFromCover=TRUE
	bDisableMovement=TRUE
	bLockPawnRotation=TRUE
	DefaultAICommand=class'AICmd_Base_PushedBySpecialMove'
}
