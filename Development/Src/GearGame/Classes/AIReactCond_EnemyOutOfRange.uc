/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class AIReactCond_EnemyOutOfRange extends AIReactCond_Conduit_Base;

var() float Range;

event bool ShouldActivate( Actor EventInstigator, AIReactChannel OrigChan )
{
	if (!Super.ShouldActivate(EventInstigator,OrigChan))
	{
		return false;
	}

	if (EventInstigator == Enemy && VSizeSq(EventInstigator.Location - pawn.Location) <  Range * Range)
	{
		return false;
	}
	return true;
}

DefaultProperties
{
	AutoSubscribeChannels(0)=Sight
	AutoSubscribeChannels(1)=Hearing
	AutoSubscribeChannels(2)=Force
}
