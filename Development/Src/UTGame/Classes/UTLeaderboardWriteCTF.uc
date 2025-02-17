
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/** The class that writes the CTF general stats */

class UTLeaderboardWriteCTF extends UTLeaderboardWriteTDM;

`include(UTStats.uci)

//Copies all relevant PRI game stats into the Properties struct of the OnlineStatsWrite
//There can be many more stats in the PRI than what is in the Properties table (on Xbox for example)
//If the Properties table does not contain the entry, the data is not written
function CopyAllStats(UTPlayerReplicationInfo PRI)
{
	Super.CopyAllStats(PRI);
}

defaultproperties
{
	WeaponsStatsClass=class'UTLeaderboardWriteWeaponsDM'
	VehicleStatsClass=class'UTLeaderboardWriteVehiclesDM'
	VehicleWeaponsStatsClass=class'UTLeaderboardWriteVehicleWeaponsDM'

	// Sort the leaderboard by this property
RatingId=PROPERTY_LEADERBOARDRATING

	// Views being written to depending on type of match (ranked or player)
//ViewIds=(STATS_VIEW_CTF_PLAYER_ALLTIME)
//ArbitratedViewIds=(STATS_VIEW_CTF_RANKED_ALLTIME)

Properties.Add((PropertyId=`PROPERTY_EVENT_HATTRICK,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_EVENT_KILLEDFLAGCARRIER,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_EVENT_RETURNEDFLAG,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_EVENT_SCOREDFLAG,Data=(Type=SDT_Int32,Value1=0)))

StatNameToStatIdMapping.Add((StatName=EVENT_HATTRICK,Id=`PROPERTY_EVENT_HATTRICK))
StatNameToStatIdMapping.Add((StatName=EVENT_KILLEDFLAGCARRIER,Id=`PROPERTY_EVENT_KILLEDFLAGCARRIER))
StatNameToStatIdMapping.Add((StatName=EVENT_RETURNEDFLAG,Id=`PROPERTY_EVENT_RETURNEDFLAG))
StatNameToStatIdMapping.Add((StatName=EVENT_SCOREDFLAG,Id=`PROPERTY_EVENT_SCOREDFLAG))

}
