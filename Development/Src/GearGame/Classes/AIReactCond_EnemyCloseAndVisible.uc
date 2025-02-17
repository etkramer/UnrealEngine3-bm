/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
/** will trigger when we notice an enemy who is within range, and visible **/
class AIReactCond_EnemyCloseAndVisible extends AIReactCond_Conduit_Base;

/** if we haven't seen a guy for at least this long, trigger a channel nudge **/
var() float DistanceThreshold;

/** Activates if we have not seen the incoming dude in a long time (or haven't ever seen) **/
event bool ShouldActivate( Actor EventInstigator, AIReactChannel OrigChan )
{
	return (Super.ShouldActivate(EventInstigator,OrigChan) && VSizeSq(EventInstigator.Location - Pawn.Location) < DistanceThreshold * DistanceThreshold);
}

defaultproperties
{
	AutoSubscribeChannels(0)=Sight
	OutputChannelName=EnemyCloseAndVisible
}
