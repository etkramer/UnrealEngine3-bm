/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
/** This is a generic reaction which will push a command whenever one of its subscribed channels is nudged **/
class AIReactCond_GenericPushCommand extends AIReactCondition_Base;

/** class of the command to push **/
var class<AICommand> CommandClass;

/** Minimum time between output calls **/
var float MinTimeBetweenOutputsSeconds;

/** last time we called our output function **/
var float LastOutputTime;

event bool ShouldActivate(Actor EventInstigator, AIReactChannel OrigChannel )
{
	//`log(self@GetFuncName()@EventInstigator@OrigChannel.ChannelName@Super.ShouldActivate(EventInstigator,OrigChannel));
	if ( !Super.ShouldActivate(EventInstigator,OrigChannel) || (LastOutputTime != 0.0 && WorldInfo.TimeSeconds - LastOutputTime < MinTimeBetweenOutputsSeconds) )
	{
		return false;
	}

	return true;
}


event Activate( Actor EventInstigator, AIReactChannel OriginatingChannel )
{
	super.Activate(EventInstigator,OriginatingChannel);
	`AILog(self@"Pushing command of class "@CommandClass@", Originating channel was "@OriginatingChannel.CHannelName);
	if( CommandClass.static.InitCommandUserActor(Outer,EventInstigator) )
	{
		LastOutputTime = WorldInfo.TimeSeconds;
	}
}

defaultproperties
{
	MinTimeBetweenOutputsSeconds=3.0f
}
