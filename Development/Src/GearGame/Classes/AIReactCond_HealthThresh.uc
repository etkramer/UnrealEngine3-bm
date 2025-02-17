/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
/** A reaction conduit that will trigger when my pawn's health is below a certain threshold **/
class AIReactCond_HealthThresh extends AIReactCond_Conduit_Base;

/** If our pawn's health falls below this threshold, the conduit will fire **/
var() int HealthThreshold;
/** If TRUE, this bool indicates this reaction should automatically suppress itself after firing once **/
var() bool bAutoSuppressAfterInitialTrigger;

event bool ShouldActivate(Actor EventInstigator, AIReactChannel OrigChannel )
{
	if(!Super.ShouldActivate(EventInstigator,OrigChannel))
	{
		return false;
	}

	if(MyGearPawn.Health < HealthThreshold)
	{
		return true;
	}

	return false;
}

event Activate( Actor EventInstigator, AIReactChannel OriginatingChannel )
{
	if(bAutoSuppressAfterInitialTrigger)
	{
		Suppress();
	}

	Super.Activate(EventInstigator,OriginatingChannel);
}

defaultproperties
{
	AutoSubscribeChannels(0)=Damage
	OutputChannelName=HealthThreshold
	bAutoSuppressAfterInitialTrigger=true
}
