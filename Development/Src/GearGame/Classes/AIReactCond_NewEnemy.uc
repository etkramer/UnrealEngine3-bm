/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
/** Will trigger a new stimulus nudge when we see/hear an enemy we haven't seen in a long time **/
class AIReactCond_NewEnemy extends AIReactCond_Conduit_Base
	native(AI);

cpptext
{
	virtual UBOOL ShouldActivateNative(AActor* Instigator, UAIReactChannel* OriginatingChannel);
}

/** if we haven't seen a guy for at least this long, trigger a channel nudge **/
var() float TimeSinceSeenThresholdSeconds;

/** Activates if we have not seen the incoming dude in a long time (or haven't ever seen) **/


defaultproperties
{
	AutoSubscribeChannels(0)=Sight
	AutoSubscribeChannels(1)=Hearing
	AutoSubscribeChannels(2)=Damage
	AutoSubscribeChannels(3)=Force
	OutputChannelName=NewEnemy
	TimeSinceSeenThresholdSeconds=10.0f
	MinTimeBetweenActivations=0.25f
}