/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
/** Base class for Reactions **/
class AIReactCondition_Base extends Object within GearAI
	native
	abstract;

/** list of channels to subscribe to on init **/
var() array<name> AutoSubscribeChannels;

/** list of channels currently subscribed **/
var array<name> SubscribedChannels;

/** bool indicating whether this reaction is disabled at the moment **/
var bool bSuppressed;

/** by default reactions will not trigger when IgnoreNotifies returns TRUE, but if this bool is true this reaction will not respect IgnroeNotifies **/
var() bool bAlwaysNotify;

/** whether or not to activate when our pawn's base is an interpactor Defaults to FALSE*/
var() bool bActivateWhenBasedOnInterpActor;

/** whether or not to activate when our pawn's base is another pawn Defaults to FALSE*/
var() bool bActivateWhenBasedOnPawn;

/** whether this reaction should activate only once */
var() bool bOneTimeOnly;

/** used only for events subscribed to the timer channel.. indicates the interval the timer channel should nudge this react condition */
var() float TimerInterval;
var	  float TimerLastActivationTime;

cpptext
{
	virtual UBOOL ShouldActivateNative( AActor* Instigator, UAIReactChannel* OriginatingChannel);

	void ConditionalCheckActivate( AActor* Instigator, UAIReactChannel* OriginatingChannel );
}

final function private native SubscribeToMyChannels();
final function native UnsubscribeAll();

event Initialize()
{
	SubscribeToMyChannels();
}

/** Override these if you want to do something custom when the event is suppressed/unsuppressed **/
event Suppress()
{
	bSuppressed=true;
}
event UnSuppress()
{
	bSuppressed=false;
}

event bool ShouldActivate( Actor EventInstigator, AIReactChannel OriginatingChannel )
{
	return true;
}

event Activate( Actor EventInstigator, AIReactChannel OriginatingChannel )
{
	// if this should only activate once, unsubscribe from everything so we don't activate any longer
	if(bOneTimeOnly)
	{
		`AILog("Unsubscribing "@self@"from all");
		UnSubscribeAll();
	}
}


DefaultProperties
{
	TimerLastActivationTime=-1.0f
	bActivateWhenBasedOnInterpActor=TRUE
}

