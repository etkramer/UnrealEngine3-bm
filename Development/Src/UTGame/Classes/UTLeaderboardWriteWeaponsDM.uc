
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/** The class that writes the DM stats */

class UTLeaderboardWriteWeaponsDM extends UTLeaderboardWriteBase;

`include(UTStats.uci)

//Copies all relevant PRI game stats into the Properties struct of the OnlineStatsWrite
//There can be many more stats in the PRI than what is in the Properties table (on Xbox for example)
//If the Properties table does not contain the entry, the data is not written
function CopyAllStats(UTPlayerReplicationInfo PRI)
{
	local IntStat tempIntStat;

	//Kill stats
	foreach PRI.KillStats(tempIntStat)
	{
		SetIntStatFromMapping(tempIntStat.StatName, tempIntStat.StatValue);
	}

	//Death stats
	foreach PRI.DeathStats(tempIntStat)
	{
		SetIntStatFromMapping(tempIntStat.StatName, tempIntStat.StatValue);
	}

	//Suicide stats
	foreach PRI.SuicideStats(tempIntStat)
	{
		SetIntStatFromMapping(tempIntStat.StatName, tempIntStat.StatValue);
	}

	Super.CopyAllStats(PRI);
}

defaultproperties
{
	// Sort the leaderboard by this property
RatingId=PROPERTY_LEADERBOARDRATING

	// Views being written to depending on type of match (ranked or player)
ViewIds=(STATS_VIEW_DM_WEAPONS_ALLTIME)
PureViewIds=(STATS_VIEW_DM_WEAPONS_RANKED_ALLTIME)
ArbitratedViewIds=(STATS_VIEW_DM_WEAPONS_RANKED_ALLTIME)

//All properties for the given table and their types

Properties.Add((PropertyId=`PROPERTY_DEATHS_BIORIFLE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_DEATHS_ENFORCER,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_DEATHS_ENVIRONMENT,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_DEATHS_FLAKCANNON,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_DEATHS_HEADSHOT,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_DEATHS_IMPACTHAMMER,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_DEATHS_INSTAGIB,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_DEATHS_LINKGUN,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_DEATHS_REDEEMER,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_DEATHS_ROCKETLAUNCHER,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_DEATHS_SHAPEDCHARGE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_DEATHS_SHOCKCOMBO,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_DEATHS_SHOCKRIFLE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_DEATHS_SNIPERRIFLE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_DEATHS_SPIDERMINE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_DEATHS_STINGER,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_DEATHS_TRANSLOCATOR,Data=(Type=SDT_Int32,Value1=0)))

Properties.Add((PropertyId=`PROPERTY_KILLS_BIORIFLE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_KILLS_ENFORCER,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_KILLS_ENVIRONMENT,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_KILLS_FLAKCANNON,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_KILLS_HEADSHOT,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_KILLS_IMPACTHAMMER,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_KILLS_INSTAGIB,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_KILLS_LINKGUN,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_KILLS_REDEEMER,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_KILLS_ROCKETLAUNCHER,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_KILLS_SHAPEDCHARGE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_KILLS_SHOCKCOMBO,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_KILLS_SHOCKRIFLE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_KILLS_SNIPERRIFLE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_KILLS_SPIDERMINE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_KILLS_STINGER,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_KILLS_TRANSLOCATOR,Data=(Type=SDT_Int32,Value1=0)))

Properties.Add((PropertyId=`PROPERTY_SUICIDES_BIORIFLE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_SUICIDES_ENFORCER,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_SUICIDES_ENVIRONMENT,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_SUICIDES_FLAKCANNON,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_SUICIDES_IMPACTHAMMER,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_SUICIDES_INSTAGIB,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_SUICIDES_LINKGUN,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_SUICIDES_REDEEMER,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_SUICIDES_ROCKETLAUNCHER,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_SUICIDES_SHAPEDCHARGE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_SUICIDES_SHOCKCOMBO,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_SUICIDES_SHOCKRIFLE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_SUICIDES_SNIPERRIFLE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_SUICIDES_SPIDERMINE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_SUICIDES_STINGER,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_SUICIDES_TRANSLOCATOR,Data=(Type=SDT_Int32,Value1=0)))
	
	//The mappings for every PRI in game stat to every Online stat regardless 
	//of whether or not we're writing it out this session
StatNameToStatIdMapping.Add((StatName=DEATHS_BIORIFLE,Id=`PROPERTY_DEATHS_BIORIFLE))
StatNameToStatIdMapping.Add((StatName=DEATHS_ENFORCER,Id=`PROPERTY_DEATHS_ENFORCER))
StatNameToStatIdMapping.Add((StatName=DEATHS_ENVIRONMENT,Id=`PROPERTY_DEATHS_ENVIRONMENT))
StatNameToStatIdMapping.Add((StatName=DEATHS_FLAKCANNON,Id=`PROPERTY_DEATHS_FLAKCANNON))
StatNameToStatIdMapping.Add((StatName=DEATHS_HEADSHOT,Id=`PROPERTY_DEATHS_HEADSHOT))
StatNameToStatIdMapping.Add((StatName=DEATHS_IMPACTHAMMER,Id=`PROPERTY_DEATHS_IMPACTHAMMER))
StatNameToStatIdMapping.Add((StatName=DEATHS_INSTAGIB,Id=`PROPERTY_DEATHS_INSTAGIB))
StatNameToStatIdMapping.Add((StatName=DEATHS_LINKGUN,Id=`PROPERTY_DEATHS_LINKGUN))
StatNameToStatIdMapping.Add((StatName=DEATHS_REDEEMER,Id=`PROPERTY_DEATHS_REDEEMER))
StatNameToStatIdMapping.Add((StatName=DEATHS_ROCKETLAUNCHER,Id=`PROPERTY_DEATHS_ROCKETLAUNCHER))
StatNameToStatIdMapping.Add((StatName=DEATHS_SHAPEDCHARGE,Id=`PROPERTY_DEATHS_SHAPEDCHARGE))
StatNameToStatIdMapping.Add((StatName=DEATHS_SHOCKCOMBO,Id=`PROPERTY_DEATHS_SHOCKCOMBO))
StatNameToStatIdMapping.Add((StatName=DEATHS_SHOCKRIFLE,Id=`PROPERTY_DEATHS_SHOCKRIFLE))
StatNameToStatIdMapping.Add((StatName=DEATHS_SNIPERRIFLE,Id=`PROPERTY_DEATHS_SNIPERRIFLE))
StatNameToStatIdMapping.Add((StatName=DEATHS_SPIDERMINE,Id=`PROPERTY_DEATHS_SPIDERMINE))
StatNameToStatIdMapping.Add((StatName=DEATHS_STINGER,Id=`PROPERTY_DEATHS_STINGER))
StatNameToStatIdMapping.Add((StatName=DEATHS_TRANSLOCATOR,Id=`PROPERTY_DEATHS_TRANSLOCATOR))

StatNameToStatIdMapping.Add((StatName=KILLS_BIORIFLE,Id=`PROPERTY_KILLS_BIORIFLE))
StatNameToStatIdMapping.Add((StatName=KILLS_ENFORCER,Id=`PROPERTY_KILLS_ENFORCER))
StatNameToStatIdMapping.Add((StatName=KILLS_ENVIRONMENT,Id=`PROPERTY_KILLS_ENVIRONMENT))
StatNameToStatIdMapping.Add((StatName=KILLS_FLAKCANNON,Id=`PROPERTY_KILLS_FLAKCANNON))
StatNameToStatIdMapping.Add((StatName=KILLS_HEADSHOT,Id=`PROPERTY_KILLS_HEADSHOT))
StatNameToStatIdMapping.Add((StatName=KILLS_IMPACTHAMMER,Id=`PROPERTY_KILLS_IMPACTHAMMER))
StatNameToStatIdMapping.Add((StatName=KILLS_INSTAGIB,Id=`PROPERTY_KILLS_INSTAGIB))
StatNameToStatIdMapping.Add((StatName=KILLS_LINKGUN,Id=`PROPERTY_KILLS_LINKGUN))
StatNameToStatIdMapping.Add((StatName=KILLS_REDEEMER,Id=`PROPERTY_KILLS_REDEEMER))
StatNameToStatIdMapping.Add((StatName=KILLS_ROCKETLAUNCHER,Id=`PROPERTY_KILLS_ROCKETLAUNCHER))
StatNameToStatIdMapping.Add((StatName=KILLS_SHAPEDCHARGE,Id=`PROPERTY_KILLS_SHAPEDCHARGE))
StatNameToStatIdMapping.Add((StatName=KILLS_SHOCKCOMBO,Id=`PROPERTY_KILLS_SHOCKCOMBO))
StatNameToStatIdMapping.Add((StatName=KILLS_SHOCKRIFLE,Id=`PROPERTY_KILLS_SHOCKRIFLE))
StatNameToStatIdMapping.Add((StatName=KILLS_SNIPERRIFLE,Id=`PROPERTY_KILLS_SNIPERRIFLE))
StatNameToStatIdMapping.Add((StatName=KILLS_SPIDERMINE,Id=`PROPERTY_KILLS_SPIDERMINE))
StatNameToStatIdMapping.Add((StatName=KILLS_STINGER,Id=`PROPERTY_KILLS_STINGER))
StatNameToStatIdMapping.Add((StatName=KILLS_TRANSLOCATOR,Id=`PROPERTY_KILLS_TRANSLOCATOR))

StatNameToStatIdMapping.Add((StatName=SUICIDES_BIORIFLE,Id=`PROPERTY_SUICIDES_BIORIFLE))
StatNameToStatIdMapping.Add((StatName=SUICIDES_ENFORCER,Id=`PROPERTY_SUICIDES_ENFORCER))
StatNameToStatIdMapping.Add((StatName=SUICIDES_ENVIRONMENT,Id=`PROPERTY_SUICIDES_ENVIRONMENT))
StatNameToStatIdMapping.Add((StatName=SUICIDES_FLAKCANNON,Id=`PROPERTY_SUICIDES_FLAKCANNON))
StatNameToStatIdMapping.Add((StatName=SUICIDES_IMPACTHAMMER,Id=`PROPERTY_SUICIDES_IMPACTHAMMER))
StatNameToStatIdMapping.Add((StatName=SUICIDES_INSTAGIB,Id=`PROPERTY_SUICIDES_INSTAGIB))
StatNameToStatIdMapping.Add((StatName=SUICIDES_LINKGUN,Id=`PROPERTY_SUICIDES_LINKGUN))
StatNameToStatIdMapping.Add((StatName=SUICIDES_REDEEMER,Id=`PROPERTY_SUICIDES_REDEEMER))
StatNameToStatIdMapping.Add((StatName=SUICIDES_ROCKETLAUNCHER,Id=`PROPERTY_SUICIDES_ROCKETLAUNCHER))
StatNameToStatIdMapping.Add((StatName=SUICIDES_SHAPEDCHARGE,Id=`PROPERTY_SUICIDES_SHAPEDCHARGE))
StatNameToStatIdMapping.Add((StatName=SUICIDES_SHOCKCOMBO,Id=`PROPERTY_SUICIDES_SHOCKCOMBO))
StatNameToStatIdMapping.Add((StatName=SUICIDES_SHOCKRIFLE,Id=`PROPERTY_SUICIDES_SHOCKRIFLE))
StatNameToStatIdMapping.Add((StatName=SUICIDES_SNIPERRIFLE,Id=`PROPERTY_SUICIDES_SNIPERRIFLE))
StatNameToStatIdMapping.Add((StatName=SUICIDES_SPIDERMINE,Id=`PROPERTY_SUICIDES_SPIDERMINE))
StatNameToStatIdMapping.Add((StatName=SUICIDES_STINGER,Id=`PROPERTY_SUICIDES_STINGER))
StatNameToStatIdMapping.Add((StatName=SUICIDES_TRANSLOCATOR,Id=`PROPERTY_SUICIDES_TRANSLOCATOR))


}
