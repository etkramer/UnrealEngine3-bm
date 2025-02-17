
/**
 * Pawn To Pawn Interaction Leader base class
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_BasePawnToPawnInteractionLeader extends GearSpecialMove
	native(SpecialMoves);

/** Follower controlled by leader. */
var GearPawn				Follower;
/** Special Move Followers are forced into when interacting with the leader. */
var GearPawn.ESpecialMove	FollowerSpecialMove;

/** Notification called when Special Move starts */
function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	// Call Parent version first.
	Super.SpecialMoveStarted(bForced,PrevMove);

	// We need an Interaction Pawn to do something.
	// We are the Leader, and he is the follower.
	Follower = PawnOwner.InteractionPawn;

	if( Follower != None )
	{
		// Let Follower know who's the leader.
		Follower.InteractionPawn = PawnOwner;

		// Force our follower into his own SpecialMove so we can control him.
		if( FollowerSpecialMove != SM_None )
		{
			if ( PawnOwner.Role == ROLE_Authority )
			{
				// If forcing a human player, make sure we replicate this special move locally.
				if( Follower.Controller != None && Follower.Controller.IsA('GearPC') )
				{
					GearPC(Follower.Controller).ServerDictateSpecialMove(FollowerSpecialMove, PawnOwner);
				}
				else
				{
					GearAI(Follower.Controller).DictateSpecialMove( FollowerSpecialMove, PawnOwner );
				}
			}

			// We're ready to start the interaction!
			BeginInteraction();
		}
		else
		{
			`log( class @ GetFuncName() @ "Follower couldn't be forced to a Special Move");
		}
	}
	else
	{
		`log( class @ GetFuncName() @ "Follower == None!!! Cannot do anything here...");
	}
}

/** Event called when Interaction can be started */
function BeginInteraction();


/** 
 * Detaches a based Pawn from the Leader.
 */
function DetachPawn(GearPawn APawn)
{
	if( Follower.Role != ROLE_Authority )
	{
		return;
	}

	APawn.SetCollision(TRUE, TRUE);
	APawn.bCollideWorld = TRUE;
	APawn.SetHardAttach(FALSE);
	APawn.SetBase(None);
	APawn.SetPhysics(PHYS_Falling);
}

defaultproperties
{
	FollowerSpecialMove=SM_None
}