/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
/** this is a base class for reaction conditions that are conduits for other channels.  That is, they don't actually trigger reactions themselves,
    they just trigger another channel nudge when they activate. (e.g. listens to 3 channels, and then triggers when something interesting happens on any of those channels)
	**/
class AIReactCond_Conduit_Base extends AIReactCondition_Base
	abstract
	native;


/** name of channel to nudge when we activate **/
var name OutputChannelName;

/** minimum time in seconds between activations **/
var float MinTimeBetweenActivations;

/** last time this conduit triggered an output **/
var float LastActivationTime;

/** delegate to call as output (defaults to null)*/
delegate OutputFunction(Actor EventInstigator, AIReactChannel OrigChannel);

cpptext
{
	virtual UBOOL ShouldActivateNative(AActor* Instigator, UAIReactChannel* OriginatingChannel);
}

event Activate( Actor EventInstigator, AIReactChannel OriginatingChannel )
{
	Super.Activate(EventInstigator,OriginatingChannel);
	LastActivationTime=WorldInfo.TimeSeconds;
	Output(EventInstigator,OriginatingChannel);
}

function Output(Actor InInstigator, AIReactChannel InOriginatingChannel )
{
	OutputFunction(InInstigator,InOriginatingChannel);
	ReactionManager.NudgeChannel(InInstigator,OutputChannelName);
}


defaultproperties
{
	MinTimeBetweenActivations=-1
}
