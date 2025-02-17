/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
/** class which stores information about a reaction channel, (could be custom parameters for example...) **/
class AIReactChannel extends Object
	native
	dependsOn(AIReactCondition_Base);

cpptext
{
	void Init(FName NewChannelname);
	virtual void Poll(FLOAT DeltaTime){};
}
/** mostly for debugging.. easier to figure out what channel this is **/
var name ChannelName; 

/** whether this channel is expecting to be polled or not **/
var const bool bNeedsPoll;
/** List of reactions listening to this channel **/
var array<AIReactCondition_Base> Reactions;
/** if this bool is TRUE NudgeChannels will be ignored **/
var bool bChannelSuppressed;

/** Call these to add/remove react conditions from this channel **/
native function Subscribe(AIReactCondition_Base Condition);
native function bool UnSubscribe(AIReactCondition_Base Condition);

/** searches for a reaction of the given type, and suppresses it if found, returning a pointer to the found reaction **/
native function AIReactCondition_Base SuppressReactionByType(class<AIReactCondition_Base> Type);
/** searches for a reaction of the given type, and unsuppresses it if found, returning a pointer to the found reaction **/
native function AIReactCondition_Base UnSuppressReactionByType(class<AIReactCondition_Base> Type);
/** searches for a reaction of the given type **/
final native function AIReactCondition_Base FindReactionByType(class<AIReactCondition_Base> Type);
/** Called to update anyone listening to this channel **/
native function NudgeChannel(Actor Instigator);

/** disables reactions on this channel but does not remove them **/
event Suppress()
{
	bChannelSuppressed=true;
}

/** re-enables reactions on this channel after being suspended **/
event UnSuppress()
{
	bChannelSuppressed=false;	
}

defaultproperties
{
	bNeedsPoll=false
}