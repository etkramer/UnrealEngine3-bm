/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
* this reaction will activate after we've received so much damage
*/
class AIReactCond_DmgThreshold extends AIReactCond_Conduit_Base
	within GearAI;

var int CurrentDamage;
var() int DamageThreshold;

event bool ShouldActivate( Actor EventInstigator, AIReactChannel OrigChan )
{
	local AIReactChan_Damage DmgChan;

	//MessagePlayer(GetFuncName()@EventInstigator@self);
	//`log(GetFuncName()@EventInstigator@OrigChan.channelname@OrigChan);
	if(Super.ShouldActivate(EventInstigator,OrigChan) == false)
	{
		//`log(GetFuncName()@"Super forbade activation!");
		return false;
	}

	DmgChan = AIReactChan_Damage(OrigChan);
	if(DmgChan != none)
	{
		if(DmgChan.bDirectDamage)
		{

			CurrentDamage += DmgChan.DamageAmt;
			//MessagePlayer(GetFuncName()@CurrentDamage@DamageThreshold);
			if(CurrentDamage >= DamageThreshold)
			{
				return true;
			}
		}
	}

	return false;
}

event Initialize()
{
	//`log("---->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"@GetFuncName()@self);
	Super.Initialize();
}

event Activate(Actor EventInstigator, AIReactChannel OriginatingChannel )
{
	//MessagePlayer(GetFuncName()@self);
	Super.Activate(EventInstigator,OriginatingChannel);
	CurrentDamage = 0;
}


defaultproperties
{
	AutoSubscribeChannels(0)=Damage
	DamageThreshold=100

	bActivateWhenBasedOnPawn=TRUE
	bAlwaysNotify=TRUE
}
