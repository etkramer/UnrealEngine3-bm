/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class AIReactCond_CoverExposed extends AIReactCond_Conduit_Base
	within GearAI_Cover;


event bool ShouldActivate( Actor EventInstigator, AIReactChannel OrigChan )
{
	if (!Super.ShouldActivate(EventInstigator,OrigChan))
	{
		return false;
	}

	if(HasValidCover() && IsCoverExposedToAnEnemy(Cover,Pawn(EventInstigator)))
	{
		return TRUE;
	}

	return FALSE;
}

defaultproperties
{
	AutoSubscribeChannels(0)=Sight
	AutoSubscribeChannels(1)=Hearing
	AutoSubscribeChannels(2)=Damage
	AutoSubscribeChannels(3)=Force
	MinTimeBetweenActivations=1.0f
	OutputChannelname=CoverExposed
}
