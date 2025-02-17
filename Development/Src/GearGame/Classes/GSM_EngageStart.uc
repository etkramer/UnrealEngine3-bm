
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_EngageStart extends GSM_Engage
	native(SpecialMoves);

// Whether the pawn is in place to start engaging
var bool bHasReachedDestination;

// C++ functions
cpptext
{
	virtual void TickSpecialMove(FLOAT DeltaTime);
}

/** See if we can start the move */
protected function bool InternalCanDoSpecialMove()
{
	local Pawn HitPawn;
	local vector HitL, HitN;
	local vector Start, End;

	// make sure we have a valid engage trigger and nothing is in the way
	if( PawnOwner.EngageTrigger != None && PawnOwner.EngageTrigger.ENGAGE_Actor != None )
	{
		Start = PawnOwner.EngageTrigger.ENGAGE_Actor.Location;
		End = Start + (PawnOwner.EngageTrigger.ENGAGE_OffsetFromActor >> PawnOwner.EngageTrigger.ENGAGE_Actor.Rotation);
		foreach PawnOwner.EngageTrigger.ENGAGE_Actor.TraceActors(class'Pawn',HitPawn,HitL,HitN,End,Start,vect(32,32,32))
		{
			return (HitPawn == PawnOwner);
		}
	}
	return TRUE;
}

/** Move started so clear the destination flag */
function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	// See if Heavy Weapon should be dropped first.
	if( PawnOwner.EngageTrigger.ENGAGE_bShouldAutoDropHeavyWeapon && PawnOwner.IsCarryingAHeavyWeapon() )
	{
		PawnOwner.ThrowActiveWeapon();
	}

	Super.SpecialMoveStarted(bForced,PrevMove);

	// init the flag
	bHasReachedDestination = FALSE;

	PawnOwner.EngageTrigger.EngagedPawn = PawnOwner;

	if( PawnOwner.Role == ROLE_Authority )
	{
		PawnOwner.SetTimer(3.0, FALSE, nameof(TeleportToGrab), self);
	}
}

/** teleports the player to the proper location if it's taking too long */
function TeleportToGrab()
{
	local vector TeleportLoc;

	TeleportLoc = GetTargetPosition();
	TeleportLoc.Z = PawnOwner.Location.Z;
	PawnOwner.SetLocation(TeleportLoc);

	PawnOwner.Velocity = vect(0,0,0);
	PawnOwner.Acceleration = vect(0,0,0);

	bHasReachedDestination = TRUE;
	StartGrabAnim();
}

/** Holster weapon and fire kismet event */
simulated event StartGrabAnim()
{
	local array<SequenceEvent>	EngageEvents;
	local SeqEvt_Engage			EngageEvent;

	PawnOwner.ClearTimer(nameof(TeleportToGrab), self);

	if( PawnOwner.EngageTrigger != None && PawnOwner.EngageTrigger.ENGAGE_bShouldHolsterWeaponFirst )
	{
		// holster the weapon
		PawnOwner.MyGearWeapon.HolsterWeaponTemporarily(TRUE);
	}

	// Set at a fixed time to offset matinee induced delay.
	// Proper fix would be to trigger eENGAGEOUT_Started when Pawn animation plays, but it's too risky to do this two weeks before ZBR.
	PawnOwner.SetTimer(0.5f, FALSE, nameof(ContinueStartAnim), Self);

	// fire proper kismet event
	if( PawnOwner.EngageTrigger != None && PawnOwner.EngageTrigger.FindEventsOfClass(class'SeqEvt_Engage', EngageEvents) )
	{
		EngageEvent = SeqEvt_Engage(EngageEvents[0]);
		if( (EngageEvent != None) && EngageEvent.bEnabled )
		{
			// don't check fov since the camera could be anywhere
			EngageEvent.bCheckInteractFOV = FALSE;
			// fire the event
			PawnOwner.EngageTrigger.TriggerEventClass( class'SeqEvt_Engage', PawnOwner, eENGAGEOUT_Started );
		}
	}
}

/** Play the START animation which is stored in the engage-trigger */
simulated function ContinueStartAnim()
{
	local BodyStance			StartStance;

	// Stop the holstering animations
	PawnOwner.MyGearWeapon.StopHolsterAnims();

	if( PawnOwner.EngageTrigger != None && PawnOwner.EngageTrigger.ENGAGE_AnimName_Start != '' &&
		(PawnOwner.EngageTrigger.ENGAGE_bPlayAnimationForHeavyWeapons || PawnOwner.MyGearWeapon.WeaponType != WT_Heavy) )
	{
		// Create the stance struct
		StartStance.AnimName[BS_FullBody] = PawnOwner.EngageTrigger.ENGAGE_AnimName_Start;

		// Play body stance animation.
		PawnOwner.BS_Play(StartStance, SpeedModifier, 0.2f/SpeedModifier, -1.f, FALSE, TRUE);

		// Turn off Root motion on animation node
		PawnOwner.BS_SetRootBoneAxisOptions(StartStance, RBA_Discard, RBA_Discard, RBA_Discard);

		// Enable end of animation notification. This is going to call SpecialMoveEnded()
		PawnOwner.BS_SetAnimEndNotify(StartStance, TRUE);
	}
	else
	{
		PawnOwner.SetTimer(0.1f, FALSE, 'EndInteraction', Self);
	}
}

/** Stop the START animation and call the IDLE special move if there is one, else call the END move */
function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	local BodyStance StartStance;

	PawnOwner.ClearTimer(nameof(TeleportToGrab), Self);
	if( PawnOwner.IsTimerActive(nameof(ContinueStartAnim), Self) )
	{
		ContinueStartAnim();
		PawnOwner.ClearTimer(nameof(ContinueStartAnim), Self);
	}

	Super.SpecialMoveEnded(PrevMove, NextMove);

	if ( PawnOwner.EngageTrigger != None )
	{
		// Create the stance struct
		StartStance.AnimName[BS_FullBody] = PawnOwner.EngageTrigger.ENGAGE_AnimName_Start;

		PawnOwner.BS_SetAnimEndNotify(StartStance, FALSE);

		// See if we're finished and play the idle if not
		CheckForFinishEngage( true );
	}
}
