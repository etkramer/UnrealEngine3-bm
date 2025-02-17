
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AIReactCond_BloodMountHeadDamage extends AIReactCondition_Base
	within GearAI_BloodMount
	config(AI);

/** GoW global macros */

var float LastReactTime;
var float MinTimeBetweenReactions;
var() config int HeadDamageThreshold;

event bool ShouldActivate( Actor EventInstigator, AIReactChannel OrigChan )
{
	local AIReactChan_Damage DmgChan;

	DmgChan = AIReactChan_Damage(OrigChan);

	//MessagePlayer(GetFuncName()@outer@EventInstigator@OrigChan@OrigChan.ChannelName@HeadDamageThreshold);
	if(DmgChan != none)
	{
		HeadDamageThreshold -= DmgChan.DamageAmt;
	}

	if(HeadDamageThreshold <= 0 && TimeSince(LastReactTime) >= MinTimeBetweenReactions && !MyGearPawn.IsDoingASpecialMove() && MyGearPawn.Health > 0)
	{
		return true;
	}
	return false;
}

event Activate( Actor EventInstigator, AIReactChannel OriginatingChannel )
{
	Super.Activate(EventInstigator,OriginatingChannel);
	LastReactTime = WorldInfo.TimeSeconds;
	
	// play SM for mount
	class'AICmd_React_BloodMountHeadDamage'.static.React( outer );

	// play SM for driver
	class'AICmd_React_BloodMountDriverHeadDamage'.static.React( GearAI(MyBloodMountPawn.Driver.Controller), MyBloodMountPawn );
}

defaultproperties
{
	MinTimeBetweenReactions=3.0f
	AutoSubscribeChannels(0)=HeadDamage
}
