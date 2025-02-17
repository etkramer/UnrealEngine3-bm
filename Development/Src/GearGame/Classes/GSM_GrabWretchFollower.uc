
/**
 * Wretch Grabbing test
 * Follower class
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_GrabWretchFollower extends GearSpecialMove;

// Animations

/* Last Played Animation */
var					GearPawn.BodyStance	BS_LastPlayedAnimation;
/** Grabbing and Grabbed Idle animations */
var()				GearPawn.BodyStance	BS_GrabAnim, BS_GrabIdleAnim, BS_Thrown, BS_ThrownIdle;


/**
 * Play a custom body stance animation.
 * Supports many features, including blending in and out.
 *
 * @param	Stance			BodyStance animation to play.
 * @param	Rate			Rate animation should be played at.
 * @param	BlendInTime		Blend duration to play anim.
 * @param	BlendOutTime	Time before animation ends (in seconds) to blend out.
 *							-1.f means no blend out.
 *							0.f = instant switch, no blend.
 *							otherwise it's starting to blend out at AnimDuration - BlendOutTime seconds.
 * @param	bLooping		Should the anim loop? (and play forever until told to stop)
 * @param	bOverride		play same animation over again only if bOverride is set to true.
 *
 * @return	PlayBack length of animation.
 */
final simulated function float BS_Play
(
	BodyStance	Stance,
	float		Rate,
	optional	float		BlendInTime,
	optional	float		BlendOutTime,
	optional	bool		bLooping,
	optional	bool		bOverride = TRUE
 )
{
	// If we have a PlayerOwner, then we can play the body stance on him
	if( PawnOwner != None )
	{
		// Keep track of animation played
		BS_LastPlayedAnimation = Stance;

		// Play animation
		return PawnOwner.BS_Play(Stance, Rate, BlendInTime, BlendOutTime, bLooping, bOverride);
	}
	else
	{
		`Log( class @ GetFuncName() @ "PawnOwner == None");
		return 0.f;
	}
}


function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	// If we're an AI Controlled Character, force us to be in the right state
	if( PawnOwner.Controller != None && PawnOwner.Controller.IsA('GearAI') && !PawnOwner.Controller.IsInState('SubAction_PawnToPawnInteractionFollower') )
	{
		PawnOwner.Controller.PushState('SubAction_PawnToPawnInteractionFollower');
	}
}


simulated function bool MessageEvent(Name EventName, Object Sender)
{
	// If Leader tells us to start the animation, let's go!
	if( EventName == 'FollowerStartAnim' )
	{
		PlayFollowerPreAttachAnim();
		return TRUE;
	}
	else if( EventName == 'AttachedToLeader' )
	{
		PlayFollowerAttachedAnimation();
		return TRUE;
	}
	else if( EventName == 'FollowerThrown' )
	{
		PlayFollowerThrown();
		return TRUE;
	}

	return Super.MessageEvent(EventName, Sender);
}


simulated function PlayFollowerPreAttachAnim()
{
	// Have Pawn play grabbing animation on full body
	BS_Play(BS_GrabAnim, SpeedModifier, 0.2f/SpeedModifier, -1.f, FALSE, TRUE);
}

/** 
 * Wretch has been attached to the leader.
 */
function PlayFollowerAttachedAnimation()
{
	// Turn off full body pre attach animation
	PawnOwner.BS_Stop(BS_GrabAnim, 0.f);

	// Play attached idle animation on upper body
	// Instant transition, because root bone moves to attachment loc.
	BS_Play(BS_GrabIdleAnim, SpeedModifier, 0.f, 0.f, TRUE, TRUE);
}


/** 
 * Marcus is throwing wretch.
 * Play wretch animation accordingly.
 */
function PlayFollowerThrown()
{
	// Stop previously played animation.
	PawnOwner.BS_Stop(BS_LastPlayedAnimation, 0.2f);

	// Play Throw animation
	BS_Play(BS_Thrown, SpeedModifier, 0.2f, 0.f, FALSE, TRUE);

	// Set up notify to get call back when animation is done playing
	PawnOwner.BS_SetAnimEndNotify(BS_Thrown, TRUE);
}


/** 
 * Wretch is done playing is 'being thrown' animation
 * It's now time to detach him from Marcus.
 */
function EndOfThrow()
{
	// Turn off full body grab animation
	PawnOwner.BS_Stop(BS_Thrown, 0.f);

	// Set up notify to get call back when animation is done playing
	PawnOwner.BS_SetAnimEndNotify(BS_Thrown, FALSE);

	// Play Grabbed idle animation on upper body
	BS_Play(BS_ThrownIdle, SpeedModifier, 0.f, 0.f, TRUE, TRUE);

	// Let our leader know that we're now ready to be detached from him.
	if( !PawnOwner.InteractionPawn.SpecialMoveMessageEvent('FollowerReadyToBeDetached', Self) )
	{
		`log(class @ GetFuncName() @ "MessageEvent FollowerReadyToBeDetached not processed by InteractionPawn!!" @ PawnOwner.InteractionPawn);
	}
}



simulated function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	// If playing grabbing animation
	if( PawnOwner.BS_IsPlaying(BS_Thrown) )
	{
		EndOfThrow();
	}
	else
	{
		Super.BS_AnimEndNotify(SeqNode, PlayedTime, ExcessTime);
	}
}


function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// If we were playing an animation, turn it off
	if( PawnOwner.BS_IsPlaying(BS_LastPlayedAnimation) )
	{
		PawnOwner.BS_Stop(BS_LastPlayedAnimation, 0.f);
	}

	// Restore AI Controller to previous state
	/*
	if( PawnOwner.Controller != None && PawnOwner.Controller.IsA('GearAI') )
	{
		PawnOwner.Controller.PopState();
	}
	*/
}

simulated event ReachedPrecisePosition()
{
	// Let our leader know that we're now in position!
	if( !PawnOwner.InteractionPawn.SpecialMoveMessageEvent('FollowerInPosition', Self) )
	{
		`log(class @ GetFuncName() @ "MessageEvent FollowerInPosition not processed by InteractionPawn!!" @ PawnOwner.InteractionPawn);
	}
}


defaultproperties
{
	BS_GrabAnim=(AnimName[BS_FullBody]="PickedUp")
	BS_GrabIdleAnim=(AnimName[BS_FullBody]="PickedUp_idle")
	BS_Thrown=(AnimName[BS_FullBody]="Picked_Up_Thrown")
	BS_ThrownIdle=(AnimName[BS_FullBody]="Picked_Up_Thrown_Loop")

	bLockPawnRotation=TRUE
}
