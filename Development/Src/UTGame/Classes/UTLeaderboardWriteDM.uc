
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/** The class that writes the DM general stats */

class UTLeaderboardWriteDM extends UTLeaderboardWriteBase;

`include(UTStats.uci)

/** Grouping of stats for the category MULTIKILL */
var array<name> MultiKill;

/** Grouping of stats for the category SPREE */
var array<name> Spree;

/** Class that holds the stats mapping/info for weapons */
var class<UTLeaderboardWriteBase> WeaponsStatsClass;

/** Class that holds the stats mapping/info for vehicles */
var class<UTLeaderboardWriteBase> VehicleStatsClass;

/** Class that holds the stats mapping/info for vehicle weapons */
var class<UTLeaderboardWriteBase> VehicleWeaponsStatsClass;

//Copies all relevant PRI game stats into the Properties struct of the OnlineStatsWrite
//There can be many more stats in the PRI than what is in the Properties table (on Xbox for example)
//If the Properties table does not contain the entry, the data is not written
function CopyAllStats(UTPlayerReplicationInfo PRI)
{
	local IntStat tempIntStat;
	local TimeStat tempTimeStat;
	local int NumKills;

	NumKills = PRI.Kills;
	SetIntStat(`PROPERTY_EVENT_KILLS,NumKills);
	// Make sure to create a non-zero rating value
	if (NumKills == 0)
	{
		NumKills = -1;
	}
	SetIntStat(PROPERTY_LEADERBOARDRATING,NumKills);
	SetIntStat(`PROPERTY_EVENT_DEATHS,PRI.Deaths);

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

	//Event Stats such as Multikill, Spree, Reward
	foreach PRI.EventStats(tempIntStat)
	{
		SetIntStatFromMapping(tempIntStat.StatName, tempIntStat.StatValue);
	}

	//Number of node events accomplished (capture/destroy/etc)
	foreach PRI.NodeStats(tempIntStat)
	{
		SetIntStatFromMapping(tempIntStat.StatName, tempIntStat.StatValue);
	}

	//Number of things picked up
	foreach PRI.PickupStats(tempIntStat)
	{
		SetIntStatFromMapping(tempIntStat.StatName, tempIntStat.StatValue);
	}

	//Time under a powerup
	foreach PRI.PowerupTimeStats(tempTimeStat)
	{
		SetIntStatFromMapping(tempTimeStat.StatName, int(tempTimeStat.TotalTime));
	}

	Super.CopyAllStats(PRI);
}

//Calls CopyAllStats() on relevant stat tables and then writes the data out to the online interface
function CopyAndWriteAllStats(UniqueNetId UniqId, UTPlayerReplicationInfo PRI, bool bIsPureServer, OnlineStatsInterface StatsInterface)
{
	local UTLeaderboardWriteWeaponsDM WeaponStats;
	local UTLeaderboardWriteVehiclesDM VehicleStats;
	local UTLeaderboardWriteVehicleWeaponsDM VehicleWeaponsStats;
	if (StatsInterface != None)
	{
		//Copy all stats out of the PRI
		//and copy them into the online subsystem where they will be
		//sent via the network in EndOnlineGame()

		SetPureServerMode(bIsPureServer);
		CopyAllStats(PRI);
		StatsInterface.WriteOnlineStats('Game',UniqId, self);

		if (WeaponsStatsClass != none)
		{
			WeaponStats = UTLeaderboardWriteWeaponsDM(new WeaponsStatsClass);
			WeaponStats.SetPureServerMode(bIsPureServer);
			WeaponStats.CopyAllStats(PRI);
			StatsInterface.WriteOnlineStats('Game',UniqId, WeaponStats);
		}

		if (VehicleStatsClass != none)
		{
			VehicleStats = UTLeaderboardWriteVehiclesDM(new VehicleStatsClass);
			VehicleStats.SetPureServerMode(bIsPureServer);
			VehicleStats.CopyAllStats(PRI);
			StatsInterface.WriteOnlineStats('Game',UniqId, VehicleStats);
		}

		if (VehicleWeaponsStatsClass != none)
		{
			VehicleWeaponsStats = UTLeaderboardWriteVehicleWeaponsDM(new VehicleWeaponsStatsClass);
			VehicleWeaponsStats.SetPureServerMode(bIsPureServer);
			VehicleWeaponsStats.CopyAllStats(PRI);
			StatsInterface.WriteOnlineStats('Game',UniqId, VehicleWeaponsStats);
		}
	}
}

defaultproperties
{
	WeaponsStatsClass=class'UTLeaderboardWriteWeaponsDM'
	VehicleStatsClass=class'UTLeaderboardWriteVehiclesDM'
	VehicleWeaponsStatsClass=class'UTLeaderboardWriteVehicleWeaponsDM'

	// Sort the leaderboard by this property
RatingId=PROPERTY_LEADERBOARDRATING

	// Views being written to depending on type of match (ranked or player)
ViewIds=(STATS_VIEW_DM_PLAYER_ALLTIME)
PureViewIds=(STATS_VIEW_DM_RANKED_ALLTIME)
ArbitratedViewIds=(STATS_VIEW_DM_RANKED_ALLTIME)

	//All properties for the given table and their types
Properties.Add((PropertyId=`PROPERTY_EVENT_DEATHS,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_EVENT_KILLS,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_EVENT_BULLSEYE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_EVENT_DENIEDREDEEMER,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_EVENT_EAGLEEYE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_EVENT_ENDSPREE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_EVENT_FIRSTBLOOD,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_EVENT_HIJACKED,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_EVENT_RANOVERDEATHS,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_EVENT_RANOVERKILLS,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_EVENT_TOPGUN,Data=(Type=SDT_Int32,Value1=0)))

Properties.Add((PropertyId=`PROPERTY_MULTIKILL_DOUBLEKILL,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_MULTIKILL_MEGAKILL,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_MULTIKILL_MONSTERKILL,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_MULTIKILL_MULTIKILL,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_MULTIKILL_ULTRAKILL,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_PICKUPS_ARMOR,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_PICKUPS_BERSERK,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_PICKUPS_HEALTH,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_PICKUPS_INVISIBILITY,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_PICKUPS_INVULNERABILITY,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_PICKUPS_JUMPBOOTS,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_PICKUPS_SHIELDBELT,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_PICKUPS_UDAMAGE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_POWERUPTIME_BERSERK,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_POWERUPTIME_INVISIBILITY,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_POWERUPTIME_INVULNERABILITY,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_POWERUPTIME_UDAMAGE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_REWARD_BIGGAMEHUNTER,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_REWARD_BIOHAZARD,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_REWARD_BLUESTREAK,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_REWARD_COMBOKING,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_REWARD_FLAKMASTER,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_REWARD_GUNSLINGER,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_REWARD_HEADHUNTER,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_REWARD_JACKHAMMER,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_REWARD_ROADRAMPAGE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_REWARD_ROCKETSCIENTIST,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_REWARD_SHAFTMASTER,Data=(Type=SDT_Int32,Value1=0)))

Properties.Add((PropertyId=`PROPERTY_SPREE_DOMINATING,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_SPREE_GODLIKE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_SPREE_KILLINGSPREE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_SPREE_MASSACRE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_SPREE_RAMPAGE,Data=(Type=SDT_Int32,Value1=0)))
Properties.Add((PropertyId=`PROPERTY_SPREE_UNSTOPPABLE,Data=(Type=SDT_Int32,Value1=0)))


	//The mappings for every PRI in game stat to every Online stat regardless
	//of whether or not we're writing it out this session
StatNameToStatIdMapping.Add((StatName=EVENT_KILLS,Id=`PROPERTY_EVENT_KILLS))
StatNameToStatIdMapping.Add((StatName=EVENT_DEATHS,Id=`PROPERTY_EVENT_DEATHS))
StatNameToStatIdMapping.Add((StatName=EVENT_BULLSEYE,Id=`PROPERTY_EVENT_BULLSEYE))
StatNameToStatIdMapping.Add((StatName=EVENT_DENIEDREDEEMER,Id=`PROPERTY_EVENT_DENIEDREDEEMER))
StatNameToStatIdMapping.Add((StatName=EVENT_EAGLEEYE,Id=`PROPERTY_EVENT_EAGLEEYE))
StatNameToStatIdMapping.Add((StatName=EVENT_ENDSPREE,Id=`PROPERTY_EVENT_ENDSPREE))
StatNameToStatIdMapping.Add((StatName=EVENT_FIRSTBLOOD,Id=`PROPERTY_EVENT_FIRSTBLOOD))
StatNameToStatIdMapping.Add((StatName=EVENT_HIJACKED,Id=`PROPERTY_EVENT_HIJACKED))
StatNameToStatIdMapping.Add((StatName=EVENT_RANOVERDEATHS,Id=`PROPERTY_EVENT_RANOVERDEATHS))
StatNameToStatIdMapping.Add((StatName=EVENT_RANOVERKILLS,Id=`PROPERTY_EVENT_RANOVERKILLS))
StatNameToStatIdMapping.Add((StatName=EVENT_TOPGUN,Id=`PROPERTY_EVENT_TOPGUN))

StatNameToStatIdMapping.Add((StatName=MULTIKILL_DOUBLEKILL,Id=`PROPERTY_MULTIKILL_DOUBLEKILL))
StatNameToStatIdMapping.Add((StatName=MULTIKILL_MEGAKILL,Id=`PROPERTY_MULTIKILL_MEGAKILL))
StatNameToStatIdMapping.Add((StatName=MULTIKILL_MONSTERKILL,Id=`PROPERTY_MULTIKILL_MONSTERKILL))
StatNameToStatIdMapping.Add((StatName=MULTIKILL_MULTIKILL,Id=`PROPERTY_MULTIKILL_MULTIKILL))
StatNameToStatIdMapping.Add((StatName=MULTIKILL_ULTRAKILL,Id=`PROPERTY_MULTIKILL_ULTRAKILL))

StatNameToStatIdMapping.Add((StatName=PICKUPS_ARMOR,Id=`PROPERTY_PICKUPS_ARMOR))
StatNameToStatIdMapping.Add((StatName=PICKUPS_BERSERK,Id=`PROPERTY_PICKUPS_BERSERK))
StatNameToStatIdMapping.Add((StatName=PICKUPS_HEALTH,Id=`PROPERTY_PICKUPS_HEALTH))
StatNameToStatIdMapping.Add((StatName=PICKUPS_INVISIBILITY,Id=`PROPERTY_PICKUPS_INVISIBILITY))
StatNameToStatIdMapping.Add((StatName=PICKUPS_INVULNERABILITY,Id=`PROPERTY_PICKUPS_INVULNERABILITY))
StatNameToStatIdMapping.Add((StatName=PICKUPS_JUMPBOOTS,Id=`PROPERTY_PICKUPS_JUMPBOOTS))
StatNameToStatIdMapping.Add((StatName=PICKUPS_SHIELDBELT,Id=`PROPERTY_PICKUPS_SHIELDBELT))
StatNameToStatIdMapping.Add((StatName=PICKUPS_UDAMAGE,Id=`PROPERTY_PICKUPS_UDAMAGE))
StatNameToStatIdMapping.Add((StatName=POWERUPTIME_BERSERK,Id=`PROPERTY_POWERUPTIME_BERSERK))
StatNameToStatIdMapping.Add((StatName=POWERUPTIME_INVISIBILITY,Id=`PROPERTY_POWERUPTIME_INVISIBILITY))
StatNameToStatIdMapping.Add((StatName=POWERUPTIME_INVULNERABILITY,Id=`PROPERTY_POWERUPTIME_INVULNERABILITY))
StatNameToStatIdMapping.Add((StatName=POWERUPTIME_UDAMAGE,Id=`PROPERTY_POWERUPTIME_UDAMAGE))

StatNameToStatIdMapping.Add((StatName=REWARD_BIGGAMEHUNTER,Id=`PROPERTY_REWARD_BIGGAMEHUNTER))
StatNameToStatIdMapping.Add((StatName=REWARD_BIOHAZARD,Id=`PROPERTY_REWARD_BIOHAZARD))
StatNameToStatIdMapping.Add((StatName=REWARD_BLUESTREAK,Id=`PROPERTY_REWARD_BLUESTREAK))
StatNameToStatIdMapping.Add((StatName=REWARD_COMBOKING,Id=`PROPERTY_REWARD_COMBOKING))
StatNameToStatIdMapping.Add((StatName=REWARD_FLAKMASTER,Id=`PROPERTY_REWARD_FLAKMASTER))
StatNameToStatIdMapping.Add((StatName=REWARD_GUNSLINGER,Id=`PROPERTY_REWARD_GUNSLINGER))
StatNameToStatIdMapping.Add((StatName=REWARD_HEADHUNTER,Id=`PROPERTY_REWARD_HEADHUNTER))
StatNameToStatIdMapping.Add((StatName=REWARD_JACKHAMMER,Id=`PROPERTY_REWARD_JACKHAMMER))
StatNameToStatIdMapping.Add((StatName=REWARD_ROADRAMPAGE,Id=`PROPERTY_REWARD_ROADRAMPAGE))
StatNameToStatIdMapping.Add((StatName=REWARD_ROCKETSCIENTIST,Id=`PROPERTY_REWARD_ROCKETSCIENTIST))
StatNameToStatIdMapping.Add((StatName=REWARD_SHAFTMASTER,Id=`PROPERTY_REWARD_SHAFTMASTER))

StatNameToStatIdMapping.Add((StatName=SPREE_DOMINATING,Id=`PROPERTY_SPREE_DOMINATING))
StatNameToStatIdMapping.Add((StatName=SPREE_GODLIKE,Id=`PROPERTY_SPREE_GODLIKE))
StatNameToStatIdMapping.Add((StatName=SPREE_KILLINGSPREE,Id=`PROPERTY_SPREE_KILLINGSPREE))
StatNameToStatIdMapping.Add((StatName=SPREE_MASSACRE,Id=`PROPERTY_SPREE_MASSACRE))
StatNameToStatIdMapping.Add((StatName=SPREE_RAMPAGE,Id=`PROPERTY_SPREE_RAMPAGE))
StatNameToStatIdMapping.Add((StatName=SPREE_UNSTOPPABLE,Id=`PROPERTY_SPREE_UNSTOPPABLE))

MultiKill.Add(MULTIKILL_DOUBLEKILL)
MultiKill.Add(MULTIKILL_MULTIKILL)
MultiKill.Add(MULTIKILL_MEGAKILL)
MultiKill.Add(MULTIKILL_ULTRAKILL)
MultiKill.Add(MULTIKILL_MONSTERKILL)

Spree.Add(SPREE_KILLINGSPREE)
Spree.Add(SPREE_RAMPAGE)
Spree.Add(SPREE_DOMINATING)
Spree.Add(SPREE_UNSTOPPABLE)
Spree.Add(SPREE_GODLIKE)
Spree.Add(SPREE_MASSACRE)
}
