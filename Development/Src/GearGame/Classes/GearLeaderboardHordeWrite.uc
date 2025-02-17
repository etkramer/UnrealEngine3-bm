/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Base class for all versus types. Each specific game type will override
 */
class GearLeaderboardHordeWrite extends GearLeaderboardWriteBase
	dependson(GearTeamInfo);

/**
 * Copies the values from the PRI
 *
 * @param PRI the replication info to process
 * @param Ignored1
 * @param Ignored2
 * @param Ignored3
 */
function UpdateFromPRI(GearPRI PRI,int Ignored1,int Ignored2,int Ignored3)
{
	local GearGRI GRI;
	local GearTeamInfo GearTeam;
	local int Score;

	GRI = GearGRI(PRI.WorldInfo.GRI);
	if (GRI != None)
	{
		// The round count indicates what wave we've reached
		SetIntStat(PROPERTY_WAVE,GRI.RoundCount);
	}
	GearTeam = GearTeamInfo(PRI.Team);
	if (GearTeam != None)
	{
		Score = GearTeam.TotalScore;
	}
	SetIntStat(PROPERTY_POINTS,Score);
	// Make sure to create a non-zero rating value
	if (Score == 0)
	{
		Score = -1;
	}
	SetIntStat(PROPERTY_POINTSRATED,Score);

	// Set the difficulty they played on
	SetIntStat(PROPERTY_DIFFICULTY,PRI.Difficulty.default.DifficultyLevel);

	// Set the leaderboard based off of map name
	UpdateViewIds(PRI.WorldInfo.GetMapName(true));
}

/**
 * Updates the view ids being written to based off of the map that the match was in
 *
 * @param WI the world info to determine the map from
 */
function UpdateViewIds(string MapName)
{
	local int ViewId;

	ViewId = class'GearLeaderboardSettings'.static.GetViewIdFromMapName(name(MapName));
	`Log("Using Horde view id of "$ViewId);
	
	ViewIds.Length = 0;
	ViewIds.AddItem(ViewId);
	
	ArbitratedViewIds = ViewIds;
}

defaultproperties
{
	// These are the stats we are collecting
	Properties=((PropertyId=PROPERTY_WAVE,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_POINTS,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_POINTSRATED,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_DIFFICULTY,Data=(Type=SDT_Int32,Value1=0)))
	RatingId=PROPERTY_POINTSRATED
}
