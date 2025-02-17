/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
/** Triggers when the enemy's location is very different from where we last thought he was **/
class AIReactCond_SurpriseEnemyLoc extends AIReactCond_Conduit_Base
	native(AI);

cpptext
{
	virtual UBOOL ShouldActivateNative(AActor* Instigator, UAIReactChannel* OriginatingChannel);
}

defaultproperties
{
	AutoSubscribeChannels(0)=Sight
	AutoSubscribeChannels(1)=Hearing
	AutoSubscribeChannels(2)=Damage
	OutputChannelname=SurpriseEnemyLoc
}