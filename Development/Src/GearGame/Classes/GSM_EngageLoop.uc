
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_EngageLoop extends GSM_Engage;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);
	LoopEngage();
}

simulated function LoopEngage()
{
	local array<SequenceEvent> EngageEvents;
	local SeqEvt_Engage EngageEvent;
	local BodyStance LoopStance;

	if( PawnOwner.EngageTrigger != None )
	{
		if( PawnOwner.EngageTrigger.FindEventsOfClass(class'SeqEvt_Engage', EngageEvents) )
		{
			EngageEvent = SeqEvt_Engage(EngageEvents[0]);
			if( EngageEvent != None && EngageEvent.bEnabled )
			{
				// don't check fov since the camera could be anywhere
				EngageEvent.bCheckInteractFOV = FALSE;
				// fire an event
				PawnOwner.EngageTrigger.TriggerEventClass( class'SeqEvt_Engage', PawnOwner, eENGAGEOUT_Interacted );
				// reset the fov check
				EngageEvent.bCheckInteractFOV = TRUE;
				// Set the last timer timer
				PawnOwner.EngageTrigger.LastTurnTime = PawnOwner.WorldInfo.TimeSeconds;
				// Decrement the loop counter
				PawnOwner.EngageTrigger.LoopCounter--;
			}
		}

		if( PawnOwner.EngageTrigger.ENGAGE_AnimName_Loop != '' &&
			(PawnOwner.EngageTrigger.ENGAGE_bPlayAnimationForHeavyWeapons || PawnOwner.MyGearWeapon.WeaponType != WT_Heavy) )
		{
			// Create the loop stance
			LoopStance.AnimName[BS_FullBody] = PawnOwner.EngageTrigger.ENGAGE_AnimName_Loop;

			// Play body stance animation.
			PawnOwner.BS_Play(LoopStance, SpeedModifier, 0.1f/SpeedModifier, -1.f, FALSE, TRUE);

			// Enable end of animation notification. This is going to call SpecialMoveEnded()
			PawnOwner.BS_SetAnimEndNotify(LoopStance, TRUE);
		}
		else
		{
			PawnOwner.SetTimer(0.1f, FALSE, 'EndInteraction', Self);
		}
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	local BodyStance LoopStance;

	if( PawnOwner.EngageTrigger != None )
	{
		if( PawnOwner.EngageTrigger.ENGAGE_AnimName_Loop != '' &&
			(PawnOwner.EngageTrigger.ENGAGE_bPlayAnimationForHeavyWeapons || PawnOwner.MyGearWeapon.WeaponType != WT_Heavy) )
		{
			LoopStance.AnimName[BS_FullBody] = PawnOwner.EngageTrigger.ENGAGE_AnimName_Loop;

			// Stop animation
			PawnOwner.BS_SetAnimEndNotify(LoopStance, FALSE);
			PawnOwner.BS_Stop(LoopStance, 0.2f);
		}
	}

	Super.SpecialMoveEnded(PrevMove, NextMove);

	// See if we're finished, if not play the idle
	CheckForFinishEngage(TRUE);
}
