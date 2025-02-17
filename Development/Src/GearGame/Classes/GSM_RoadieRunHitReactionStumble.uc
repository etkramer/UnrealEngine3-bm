
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_RoadieRunHitReactionStumble extends GearSpecialMove;

/** Last Time Pawn Stumbled */
var transient	FLOAT	LastStumbleTime;
/** Time between stumbles */
var config		FLOAT	MinTimeBetweenStumbles;
/** Stumble animation */
var GearPawn.BodyStance	BS_Stumble;
/** Test if we can chain Roadie Run */
var config	bool		bCanChainRoadieRun;

protected function bool InternalCanDoSpecialMove()
{
	return (PawnOwner.WorldInfo.TimeSeconds - LastStumbleTime) > MinTimeBetweenStumbles;
}

/** 
 * Can this special move override InMove if it is currently playing?
 */
function bool CanOverrideSpecialMove(ESpecialMove InMove)
{
	return (InMove == SM_RoadieRun);
}

/**
 * On server only, possibly save chained move
 * @Return TRUE if move could be chained to this one
 */
function bool CanChainMove(ESpecialMove NextMove)
{
	return bCanChainRoadieRun && (NextMove == SM_RoadieRun || NextMove == SM_Run2MidCov || NextMove == SM_Run2StdCov);
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	// Force Pawn to keep moving
	PawnOwner.bForceMaxAccel = TRUE;

	PawnOwner.BS_Play(BS_Stumble, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier);
	PawnOwner.BS_SetAnimEndNotify(BS_Stumble, TRUE);
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	LastStumbleTime = PawnOwner.WorldInfo.TimeSeconds;

	// Stop Pawn from moving.
	PawnOwner.bForceMaxAccel = FALSE;

	PawnOwner.BS_Stop(BS_Stumble, 0.2f/SpeedModifier);
	PawnOwner.BS_SetAnimEndNotify(BS_Stumble, FALSE);

	Super.SpecialMoveEnded(PrevMove, NextMove);

	if( bCanChainRoadieRun )
	{
		// check for a transition to roadie run
		if( PCOwner!= None )
		{
			if( PCOwner.IsHoldingRoadieRunButton() )
			{
				if( (PCOwner.bUseAlternateControls || !PCOwner.TryToRunToCover(TRUE,0.75f)) && PCOwner.CanDoSpecialMove(SM_RoadieRun) )
				{
					// delay for a second to allow the slip to completely finish
					PawnOwner.SetTimer( 0.01f, FALSE, nameof(PawnOwner.QueueRoadieRunMove) );
				}
			}
		}
	}
}

defaultproperties
{
	BS_Stumble=(AnimName[BS_FullBody]="Hit_StoppingPower_Stumble")

	DefaultAICommand=class'AICmd_Base_PushedBySpecialMove'
	bDisableAI=TRUE

	bNoStoppingPower=FALSE
	bShouldAbortWeaponReload=TRUE
	bCanFireWeapon=FALSE
	bDisableMovement=TRUE
	bDisableLook=TRUE
	bLockPawnRotation=TRUE
	bCameraFocusOnPawn=TRUE
}