/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GSM_Engage extends GearSpecialMove
	native(SpecialMoves);

// C++ functions
cpptext
{
	virtual void TickSpecialMove(FLOAT DeltaTime);
}

/** get PawnOwner's target placement to engage the trigger correctly */
native final function vector GetTargetPosition();

/**
* Can the special move be chained after the current one finishes?
*/
function bool CanChainMove(ESpecialMove NextMove)
{
	return TRUE;
}

/** Checks to see if the engage is finished, and if so will stop the move and return true */
simulated function bool CheckForFinishEngage( optional bool bPlayIdleIfUnfinished )
{
	local GearPC PC;
	local array<SequenceEvent> EngageEvents;
	local SeqEvt_Engage	EngageEvent;
	local bool bIsFinished;

	PC = GearPC(PawnOwner.Controller);

	bIsFinished = TRUE;

	if ( (PawnOwner.Controller != None) && (PawnOwner.Controller.Role == ROLE_Authority) && (PawnOwner.EngageTrigger != None) )
	{
		if ( PawnOwner.EngageTrigger.FindEventsOfClass(class'SeqEvt_Engage', EngageEvents) )
		{
			EngageEvent = SeqEvt_Engage(EngageEvents[0]);

			if ( PC != None &&
				(EngageEvent != None) && EngageEvent.bEnabled &&
				 ((PawnOwner.EngageTrigger.ENGAGE_AnimName_Loop == '') || (PawnOwner.EngageTrigger.LoopCounter <= 0)) )
			{
				// don't check fov since the camera could be anywhere
				EngageEvent.bCheckInteractFOV = false;

				// fire the "finished" event and set the player back to walking
				PawnOwner.EngageTrigger.TriggerEventClass( class'SeqEvt_Engage', PawnOwner, eENGAGEOUT_Finished );

				// fire the "released" event too, since we're disabling it and it won't fire later
				PawnOwner.EngageTrigger.TriggerEventClass( class'SeqEvt_Engage', PawnOwner, eENGAGEOUT_Stopped );

				// all turns done, disable event
				EngageEvent.bEnabled = FALSE;

				// Reset the counter so we can do the engage again if reenabled
				PawnOwner.EngageTrigger.LoopCounter = PawnOwner.EngageTrigger.ENGAGE_TurnsToComplete;

				if(PC != none)
				{
					PC.ClientGotoState('PlayerWalking');
					PC.GotoState('PlayerWalking');
				}

				// reset the fov check
				EngageEvent.bCheckInteractFOV = TRUE;
			}
			else
			{
				bIsFinished = false;
			}
		}

		if ( !bIsFinished && bPlayIdleIfUnfinished )
		{
			PawnOwner.ServerDoSpecialMove(SM_Engage_Idle, false);
		}
	}

	return bIsFinished;
}

/** Called when there is no animation to play to prevent from being stuck */
function EndInteraction()
{
	PawnOwner.EndSpecialMove();
}

defaultproperties
{
	bShouldAbortWeaponReload=TRUE
	bDisableMovement=TRUE
	bLockPawnRotation=TRUE
}
