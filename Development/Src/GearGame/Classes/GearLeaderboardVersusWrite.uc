/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Base class for all versus types. Each specific game type will override
 */
class GearLeaderboardVersusWrite extends GearLeaderboardWriteBase;

/**
 * Copies the values from the PRI
 *
 * @param PRI the replication info to process
 * @param RoundsPlayed the total number of rounds for the match
 * @param RoundsWon how many rounds this PRI won
 * @param RoundsLost how many rounds this PRI lost
 */
function UpdateFromPRI(GearPRI PRI,int RoundsPlayed,int RoundsWon,int RoundsLost)
{
	local int PriIndex;
	local int BotCount;
	local EDifficultyLevel Difficulty;
	local GearPRI GPRI;

	SetPoints(PRI.Score);

	SetIntStat(PROPERTY_REVIVES,PRI.Score_Revives);
	SetIntStat(PROPERTY_TAKEDOWNS,PRI.Score_Takedowns);
	SetIntStat(PROPERTY_KILLS,PRI.Score_Kills);
	SetIntStat(PROPERTY_DEATHS,PRI.Deaths);

	// Calculate match win/lose
	if (RoundsWon > RoundsLost)
	{
		SetIntStat(PROPERTY_MATCHESWON,1);
		SetIntStat(PROPERTY_MATCHESLOST,0);
	}
	else
	{
		SetIntStat(PROPERTY_MATCHESWON,0);
		SetIntStat(PROPERTY_MATCHESLOST,1);
	}

	// Calculate round data
	SetIntStat(PROPERTY_ROUNDSPLAYED,RoundsPlayed);
	SetIntStat(PROPERTY_ROUNDSWON,RoundsWon);
	SetIntStat(PROPERTY_ROUNDSLOST,RoundsLost);
	SetIntStat(PROPERTY_ROUNDSDRAW,Max(0,RoundsPlayed - RoundsWon - RoundsLost));

	// Count how many bots were in the session
	for (PriIndex = 0; PriIndex < PRI.WorldInfo.GRI.PRIArray.Length; PriIndex++)
	{
		GPRI = GearPRI(PRI.WorldInfo.GRI.PRIArray[PriIndex]);
		if (GPRI != None && GPRI.bBot)
		{
			BotCount++;
			Difficulty = GPRI.Difficulty.default.DifficultyLevel;
		}
	}
	// Append any bot information if present
	if (BotCount > 0)
	{
		switch (Difficulty)
		{
			case DL_Casual:
				SetIntStat(PROPERTY_CASUALDIFFICULTY,1);
				break;
			case DL_Normal:
				SetIntStat(PROPERTY_NORMALDIFFICULTY,1);
				break;
			case DL_Hardcore:
				SetIntStat(PROPERTY_HARDCOREDIFFICULTY,1);
				break;
			case DL_Insane:
				SetIntStat(PROPERTY_INSANEDIFFICULTY,1);
				break;
		}
	}
}

defaultproperties
{
	// These are the stats we are collecting
	Properties=((PropertyId=PROPERTY_MATCHESPLAYED,Data=(Type=SDT_Int32,Value1=1)),(PropertyId=PROPERTY_MATCHESWON,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_MATCHESLOST,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_ROUNDSPLAYED,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_ROUNDSWON,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_ROUNDSLOST,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_ROUNDSDRAW,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_KILLS,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_DEATHS,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_REVIVES,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_TAKEDOWNS,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_POINTS,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_POINTSRATED,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_CASUALDIFFICULTY,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_NORMALDIFFICULTY,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_HARDCOREDIFFICULTY,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_INSANEDIFFICULTY,Data=(Type=SDT_Int32,Value1=0)))
}
