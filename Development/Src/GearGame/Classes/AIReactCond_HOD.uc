/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
/** HOD reaction, exempt from IgnoreNotifies **/
class AIReactCond_HOD extends AIReactCondition_Base;

/** GoW global macros */


event bool ShouldActivate( Actor EventInstigator, AIReactChannel OrigChan)
{
	local AIReactChan_Damage DmgChan;

	// super pwns
	if(!Super.ShouldActivate(EventInstigator,OrigChan))
	{
		return false;
	}

	DmgChan = AIReactChan_Damage(OrigChan);
	if(DmgChan != none)
	{
		if( DmgChan.DamageType == class'GDT_HOD' &&
			!MyGearPawn.IsDoingASpecialMove() )
		{
			return true;
		}
	}

	return false;
}

event Activate( Actor EventInstigator, AIReactChannel OriginatingChannel)
{
	Super.Activate(EventInstigator,OriginatingChannel);

	MyGearPawn.Cringe();
}



defaultproperties
{
	bAlwaysNotify=true // this supercedes ignore notifies!
	AutoSubscribeChannels(0)=Damage
}
