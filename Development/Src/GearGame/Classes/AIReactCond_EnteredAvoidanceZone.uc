/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
/** This is a generic reaction which will push a command whenever one of its subscribed channels is nudged **/
class AIReactCond_EnteredAvoidanceZone  extends AIReactCond_GenericPushCommand;

event Activate( Actor EventInstigator, AIReactChannel OriginatingChannel )
{
	`AILog(self@Instigator@EventInstigator@"Pushing command of class "@CommandClass@", Originating channel was "@OriginatingChannel.CHannelName);
	if( class'AICmd_VacateAvoidanceVolume'.static.VacateVolume(Outer,EventInstigator,OriginatingChannel.ChannelName == 'EnteredEvadeAvoidanceZone',OriginatingChannel.ChannelName == 'EnteredRoadieRunAvoidanceZone') )
	{
		LastOutputTime = WorldInfo.TimeSeconds;
	}
}

defaultproperties
{
	AutoSubscribeChannels(0)=EnteredAvoidanceZone
	AutoSubscribeChannels(1)=EnteredEvadeAvoidanceZone
	AutoSubscribeChannels(2)=EnteredRoadieRunAvoidanceZone
	CommandClass=class'AICmd_VacateAvoidanceVolume'
	MinTimeBetweenOutputsSeconds=0.25f
}
