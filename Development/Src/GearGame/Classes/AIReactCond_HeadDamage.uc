/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
/** head Damage reaction **/
class AIReactCond_HeadDamage extends AIReactCond_Conduit_Base;

/** GoW global macros */


event bool ShouldActivate( Actor EventInstigator, AIReactChannel OrigChan)
{
	local AIReactChan_Damage DmgChan;

	if(!Super.ShouldActivate(EventInstigator,OrigChan))
	{
		return false;
	}

	DmgChan = AIReactChan_Damage(OrigChan);
	if(DmgChan != none)
	{
		if( MyGearPawn.bLastHitWasHeadShot )
		{
			return true;
		}
	}

	return false;
}

function Output(Actor InInstigator, AIReactChannel InOriginatingChannel )
{
	local AIReactChan_Damage DmgChan;

	OutputFunction(InInstigator,InOriginatingChannel);

	DmgChan = AIReactChan_Damage(InOriginatingChannel);

	if(DmgChan != none)
	{
		ReactionManager.IncomingDamage(Pawn(InInstigator),DmgChan.bDirectDamage,DmgChan.damageType,DmgChan.HitInfo,DmgChan.LastInstigatorLoc,DmgChan.DamageAmt,OutputChannelName);
	}

}


defaultproperties
{
	AutoSubscribeChannels(0)=Damage
	OutputChannelname=HeadDamage
}
