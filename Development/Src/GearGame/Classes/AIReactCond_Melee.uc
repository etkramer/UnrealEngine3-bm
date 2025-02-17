/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
/** Melee reaction **/
class AIReactCond_Melee extends AIReactCondition_Base;

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
		// CHECK FOR REACTION TO MELEE ATTACK
		if( ClassIsChildOf(DmgChan.damageType, class'GDT_Melee') &&
			!MyGearPawn.IsDoingStumbleFromMelee() && !MyGearPawn.IsDoingSpecialMeleeAttack() && MyGearPawn.CanDoSpecialMove(SM_StumbleFromMelee) )
		{
			return true;
		}
	}

	return false;
}

event Activate( Actor EventInstigator, AIReactChannel OriginatingChannel)
{
	Super.Activate(EventInstigator,OriginatingChannel);
	class'AICmd_React_StumbleFromMelee'.static.InitCommand( Outer );
}



defaultproperties
{
	AutoSubscribeChannels(0)=Damage
}
