/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Kantus_ReviveScream extends AICommand_SpecialMove
	within GearAI_Kantus;

/** GoW global macros */

var AIReactCond_HealthThresh CancelReaction;
/** Simple constructor that takes one extra userdata param **/
static function bool InitCommandUserActor( GearAI AI, Actor UserActor )
{
	//`log(default.class@GetFuncName()@UserActor);
	if(AI.Pawn != none && UserActor != none && VSizeSq(AI.Pawn.Location-UserActor.Location) > class'GSM_Kantus_ReviveScream'.default.ReviveSearchRadius * class'GSM_Kantus_ReviveScream'.default.ReviveSearchRadius)
	{
		//`log(GetFuncName()@"BAILING!");
		return false;
	}

	return Super.InitCommandUserActor(AI,UserActor);
}


function Pushed()
{
	Super.Pushed();

	InvalidateCover();
	bWantsLedgeCheck = TRUE;


	// push a damage threshold reaction to cancel the revive SM if we take too much damage
	CancelReaction = new(outer) class'AIReactCond_HealthThresh';
	CancelReaction.HealthThreshold = Min(pawn.Health/2,750);
	CancelReaction.OutputFunction = CancelAnim;
	CancelReaction.Initialize();

	LockdownAI();
	DesiredRotation = Rotator(FireTarget.Location - pawn.Location);
	GotoState( 'Command_SpecialMove' );
}

function Popped()
{
	Super.Popped();
	UnLockAI();

	bWantsLedgeCheck = FALSE;
	// Allow normal rotation again
	CancelReaction.UnsubscribeAll();
	CancelReaction=none;
}

function CancelAnim(Actor EventInstigator, AIReactChannel OrigChannel)
{
	// notify the special move to stfu
	GSM_Kantus_ReviveScream(MyGearPawn.SpecialMoves[GSM_Kantus_ReviveScream]).ReviveCutShort();
}

state Command_SpecialMove
{
	function ESpecialMove GetSpecialMove()
	{
		return GSM_Kantus_ReviveScream;
	}

}

defaultproperties
{
	bShouldCheckSpecialMove=true
}
