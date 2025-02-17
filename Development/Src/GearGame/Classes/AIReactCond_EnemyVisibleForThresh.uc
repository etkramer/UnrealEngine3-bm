/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
/** will trigger when we see an enemy that we've seen for at lest SeenThreshold seconds **/
class AIReactCond_EnemyVisibleForThresh extends AIReactCond_Conduit_Base;

/** amount of time we have to see an enemy before triggering **/
var() float SeenThresh;

/** Activates if we have not seen the incoming dude in a long time (or haven't ever seen) **/
event bool ShouldActivate( Actor InInstigator, AIReactChannel OrigChan )
{
	if( Super.ShouldActivate(InInstigator,OrigChan) && GetEnemyVisibleDuration(Pawn(InInstigator)) >= SeenThresh )
	{
		return true;
	}

	return false;
}

defaultproperties
{
	AutoSubscribeChannels(0)=Sight
}