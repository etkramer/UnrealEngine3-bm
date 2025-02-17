/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class AIReactCond_CanReturnToTroika extends AIReactCond_GenericPushCommand
	within GearAI_Cover
	config(AI);

/** World Time that we last checked to see if we could return to our troika */
var float LastCheckTime;

/** interval to check if we can return to our troika */
var() float CheckInterval;

event bool ShouldActivate( Actor EventInstigator, AIReactChannel OrigChan )
{
	if (!Super.ShouldActivate(EventInstigator,OrigChan))
	{
		return false;
	}

	if (WorldInfo.TimeSeconds - LastCheckTime > CheckInterval || LastCheckTime == 0)
	{
		LastCheckTime = WorldInfo.TimeSeconds;
		if(IsInCombat() && Enemy != none && LastTurret != none && CanTurretFireAt(Enemy,LastTurret) && LastTurret.ClaimTurret(outer))
		{
			`AILog("Huzzah! I can fire at my enemy from "$LastTurret$" so I'm going back to it to do so!");
			return true;
		}
	}

	return false;
}


event Activate( Actor EventInstigator, AIReactChannel OriginatingChannel )
{
	super(AIReactCondition_Base).Activate(EventInstigator,OriginatingChannel);
	`AILog(self@"Pushing command of class "@CommandClass@", Originating channel was "@OriginatingChannel.CHannelName);
	BeginCombatCommand(class<AICommand_Base_Combat>(CommandClass),"I can fire from my turret now",TRUE);
	LastOutputTime = WorldInfo.TimeSeconds;
}

defaultproperties
{
	AutoSubscribeChannels(0)=Damage
	AutoSubscribeChannels(1)=Sight
	AutoSubscribeChannels(2)=Hearing
	CommandClass=class'AICmd_Base_TroikaGunner'
	bOneTimeOnly=true
	CheckInterval=1.25f
}
