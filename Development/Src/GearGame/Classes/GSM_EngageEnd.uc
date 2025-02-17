
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_EngageEnd extends GSM_Engage;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local array<SequenceEvent>	EngageEvents;
	local SeqEvt_Engage	EngageEvent;
	local BodyStance EndStance;

	Super.SpecialMoveStarted(bForced,PrevMove);

	if( PawnOwner.EngageTrigger != None )
	{
		if( PawnOwner.EngageTrigger.ENGAGE_AnimName_End != '' &&
			(PawnOwner.EngageTrigger.ENGAGE_bPlayAnimationForHeavyWeapons || PawnOwner.MyGearWeapon.WeaponType != WT_Heavy) )
		{
			// Create the STOP stance
			EndStance.AnimName[BS_FullBody] = PawnOwner.EngageTrigger.ENGAGE_AnimName_End;

			// Play body stance animation.
			PawnOwner.BS_Play(EndStance, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE);

			// Turn off Root motion on animation node
			PawnOwner.BS_SetRootBoneAxisOptions(EndStance, RBA_Discard, RBA_Discard, RBA_Discard);

			// Enable end of animation notification. This is going to call SpecialMoveEnded()
			PawnOwner.BS_SetAnimEndNotify(EndStance, TRUE);
		}
		else
		{
			PawnOwner.SetTimer(0.1f, FALSE, 'EndInteraction', Self);
		}

		PawnOwner.EngageTrigger.EngagedPawn = None;

		// fire proper kismet event
		if( PawnOwner.EngageTrigger.FindEventsOfClass(class'SeqEvt_Engage', EngageEvents) )
		{
			EngageEvent = SeqEvt_Engage(EngageEvents[0]);
			if( (EngageEvent != None) && EngageEvent.bEnabled )
			{
				// don't check fov since the camera could be anywhere
				EngageEvent.bCheckInteractFOV = false;
				// fire the event
				PawnOwner.EngageTrigger.TriggerEventClass( class'SeqEvt_Engage', PawnOwner, eENGAGEOUT_Stopped );
			}
		}
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	local BodyStance EndStance;

	if( PawnOwner.EngageTrigger != None )
	{
		if( PawnOwner.EngageTrigger.ENGAGE_AnimName_End != '' &&
			(PawnOwner.EngageTrigger.ENGAGE_bPlayAnimationForHeavyWeapons || PawnOwner.MyGearWeapon.WeaponType != WT_Heavy) )
		{
			// Create the STOP stance
			EndStance.AnimName[BS_FullBody] = PawnOwner.EngageTrigger.ENGAGE_AnimName_End;
			PawnOwner.BS_SetAnimEndNotify(EndStance, FALSE);
		}

		// unholster the weapon
		if( PawnOwner.EngageTrigger.ENGAGE_bShouldHolsterWeaponFirst )
		{
			PawnOwner.MyGearWeapon.HolsterWeaponTemporarily(FALSE);
		}
	}

	Super.SpecialMoveEnded(PrevMove, NextMove);
}
