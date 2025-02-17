/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/** Used to allow level designers to issue achievements */
class UTSeqAct_UnlockAchievement extends SequenceAction
	dependson(UTPlayerController);

/** The achievement to unlock (should be single/coop players only)*/
var() EUTGameAchievements AchievementId;

event Activated()
{
	local WorldInfo WI;
	local UTPlayerController PC;

	WI = GetWorldInfo();

	if ( WI != none && WI.GRI != none && UTGameReplicationInfo(WI.GRI) != none )
	{
		//Currently unlock the achievement for every player
		foreach WI.AllControllers(class'UTPlayerController', PC)
		{
			PC.UpdateAchievement(AchievementId);
		}
	}
}


defaultproperties
{
	ObjName="Unlock Achievement"
	ObjCategory="Misc"
}
