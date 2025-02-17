/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
/** Will trigger a new stimulus nudge when we see/hear an enemy we haven't seen in a long time **/
class AIReactCond_Ticker_ShouldExplode extends AIReactCond_GenericPushCommand
	within GearAI_Ticker;


event bool ShouldActivate(Actor EventInstigator, AIReactChannel OrigChannel )
{
	//`log(self@GetFuncName()@EventInstigator@OrigChannel.ChannelName@Super.ShouldActivate(EventInstigator,OrigChannel));
	if ( !Super.ShouldActivate(EventInstigator,OrigChannel)  || OtherTickerAboutToExplodeInArea() || ShouldDelayExplode())
	{
		return false;
	}
	return true;
}


defaultproperties
{
	AutoSubscribeChannels(0)=EnemyCloseAndVisible
	AutoSubscribeChannels(1)=HealthThreshold
	CommandClass=class'AICmd_Attack_Ticker_Explode'
}