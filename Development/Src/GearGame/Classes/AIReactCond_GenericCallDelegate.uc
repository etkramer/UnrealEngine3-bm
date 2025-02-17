/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
/** This is a generic reaction which will simply call a delegate when one of its subscribed channels is nudged **/
class AIReactCond_GenericCallDelegate extends AIReactCondition_Base;

/** Minimum time between output calls **/
var float MinTimeBetweenOutputsSeconds;

/** last time we called our output function **/
var float LastOutputTime;

// delegate to call as output
delegate OutputFunction(Actor EventInstigator, AIReactChannel OrigChannel);


event bool ShouldActivate(Actor EventInstigator, AIReactChannel OrigChannel )
{
	if(!Super.ShouldActivate(EventInstigator,OrigChannel))
	{
		return false;
	}

	if(MinTimeBetweenOutputsSeconds > 0.f && LastOutputTime > 0.f && TimeSince(LastOutputTime) < MinTimeBetweenOutputsSeconds)
	{
		return false;
	}

	return true;
}

function BeginMeleeCommandWrapper( Actor Inst, AIReactChannel OriginatingChannel )
{
	`log("WARNING!"$GetFuncName()@"Called on "@self@" Outer:"@Outer);
	ScriptTrace();
}
event Activate( Actor EventInstigator, AIReactChannel OriginatingChannel )
{
	Super.Activate(EventInstigator,OriginatingChannel);
	LastOutputTime = WorldInfo.TimeSeconds;
	OutputFunction(EventInstigator, OriginatingChannel);
}

defaultproperties
{
	MinTimeBetweenOutputsSeconds=0.5f
}
