/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/** Class that manages the recent players/match results */
class GearRecentPlayersList extends OnlineRecentPlayersList
	dependson(GearTypes);

/** The last matches' stats object holding everyone's stats */
var GearLeaderboardBase LastMatchResults;

/** The alltime stats for the participants in the last match */
var GearLeaderboardBase AllTimeMatchResults;

/** The last stats write class that we used */
var class<OnlineStatsWrite> LastStatsWriteClass;

/** The last MP map that was played */
var string LastMpMapName;

/** The type of match that was played */
var EGearMPTypes LastMpGameType;

/** Holds the additional information that the post game scoreboard needs */
struct GearRecentPlayer extends CurrentPlayerMet
{
	/** The name of the player that was met */
	var string Name;
	/** The pawn class that they chose */
	var string PawnClassName;
	/** The team name that they had */
	var string TeamName;
	/** The score for the team the player was on */
	var int TeamScore;
	/** Whether the player completed the match or not (dropped) */
	var bool bDidComplete;
};

/** Holds the list of recent players and the extra info the UI needs in the party */
var array<GearRecentPlayer> LastMatchPlayers;

/** Holds the list of bots from the last match and the extra info the UI needs in the party */
var array<GearRecentPlayer> LastMatchBots;

/**
 * Creates the last & all time leaderboard objects based off of the specified game type
 *
 * @param OnlineStatsWriteClass the game type to create the leaderboard objects for
 * @param MapName the name of the map that was just played
 * @param bUsesArbitration whether to use the public or private stats
 */
function InitRecentMatchResults(class<OnlineStatsWrite> OnlineStatsWriteClass,string MapName,bool bUsesArbitration)
{
	local class<GearLeaderboardBase> LeaderboardClass;

	LastMatchPlayers.Length = 0;
	LastMatchBots.Length = 0;

	LastMpMapName = MapName;

	LastStatsWriteClass = OnlineStatsWriteClass;
	switch (OnlineStatsWriteClass)
	{
		case class'GearLeaderboardWriteExecution':
			LastMpGameType = eGEARMP_Execution;
			LeaderboardClass = bUsesArbitration ? class'GearLeaderboardExecutionAllTime' : class'GearLeaderboardExecutionAllTimePri';
			break;
		case class'GearLeaderboardWriteGuardian':
			LastMpGameType = eGEARMP_KTL;
			LeaderboardClass = bUsesArbitration ? class'GearLeaderboardGuardianAllTime' : class'GearLeaderboardGuardianAllTimePri';
			break;
		case class'GearLeaderboardHordeWrite':
			LastMpGameType = eGEARMP_CombatTrials;
			LeaderboardClass = class'GearLeaderboardHorde';
			break;
		case class'GearLeaderboardWriteAnnex':
			LastMpGameType = eGEARMP_Annex;
			LeaderboardClass = bUsesArbitration ? class'GearLeaderboardAnnexAllTime' : class'GearLeaderboardAnnexAllTimePri';
			break;
		case class'GearLeaderboardWriteWingman':
			LastMpGameType = eGEARMP_Wingman;
			LeaderboardClass = bUsesArbitration ? class'GearLeaderboardWingmanAllTime' : class'GearLeaderboardWingmanAllTimePri';
			break;
		case class'GearLeaderboardWriteKoth':
			LastMpGameType = eGEARMP_KOTH;
			LeaderboardClass = bUsesArbitration ? class'GearLeaderboardKothAllTime' : class'GearLeaderboardKothAllTimePri';
			break;
		case class'GearLeaderboardWriteMeatflag':
			LastMpGameType = eGEARMP_CTM;
			LeaderboardClass = bUsesArbitration ? class'GearLeaderboardMeatflagAllTime' : class'GearLeaderboardMeatflagAllTimePri';
			break;
		case class'GearLeaderboardWarzoneWrite':
		default:
			LastMpGameType = eGEARMP_Warzone;
			LeaderboardClass = bUsesArbitration ? class'GearLeaderboardWarzoneAllTime' : class'GearLeaderboardWarzoneAllTimePri';
			break;
	}
	LastMatchResults = new LeaderboardClass;
	AllTimeMatchResults = new LeaderboardClass;
}

/**
 * Adds the specified player to the recent match data
 *
 * @param PRI the player replication info to add
 */
function AddPlayer(GearPRI PRI)
{
	local int AddIndex;

	AddIndex = LastMatchPlayers.Length;
	LastMatchPlayers.Length = AddIndex + 1;
	// Copy the information needed by the UI
	LastMatchPlayers[AddIndex].Name = PRI.PlayerName;
	LastMatchPlayers[AddIndex].PawnClassName = string(PRI.PawnClass.Name);
	LastMatchPlayers[AddIndex].TeamName = PRI.PawnClass.default.CharacterName;
	LastMatchPlayers[AddIndex].NetId = PRI.UniqueId;
	LastMatchPlayers[AddIndex].Skill = PRI.PlayerSkill;
	if (PRI.Team != None)
	{
		LastMatchPlayers[AddIndex].TeamNum = PRI.GetTeamNum();
		LastMatchPlayers[AddIndex].TeamScore = PRI.Team.Score;
	}
	else
	{
		// Should only ever be the meatflag
		LastMatchPlayers[AddIndex].TeamNum = 2;
	}
}

/**
 * Updates the specified player with the final recent match data
 *
 * @param PRI the player replication info to update
 */
function UpdatePlayer(GearPRI PRI)
{
	local int UpdateIndex;

	LastMatchResults.UpdateFromPRI(PRI);

	UpdateIndex = LastMatchPlayers.Find('NetId',PRI.UniqueId);
	if (UpdateIndex != INDEX_NONE)
	{
		// Mark them as completing and then set their scores
		LastMatchPlayers[UpdateIndex].bDidComplete = true;
		if (PRI.Team != None)
		{
			LastMatchPlayers[UpdateIndex].TeamNum = PRI.GetTeamNum();
			LastMatchPlayers[UpdateIndex].TeamScore = PRI.Team.Score;
		}
		else
		{
			// Should only ever be the meatflag
			LastMatchPlayers[UpdateIndex].TeamNum = 2;
		}
	}
}

/**
 * Adds the specified bot to the recent match data
 *
 * @param PRI the player replication info to add
 */
function AddBot(GearPRI PRI)
{
	local int AddIndex;

	AddIndex = LastMatchBots.Length;
	LastMatchBots.Length = AddIndex + 1;
	// Copy the information needed by the UI
	LastMatchBots[AddIndex].Name = PRI.PlayerName;
	LastMatchBots[AddIndex].PawnClassName = string(PRI.PawnClass.Name);
	LastMatchBots[AddIndex].TeamName = PRI.PawnClass.default.CharacterName;
	if (PRI.Team != None)
	{
		LastMatchBots[AddIndex].TeamNum = PRI.GetTeamNum();
		LastMatchBots[AddIndex].TeamScore = PRI.Team.Score;
	}
	else
	{
		// Should only ever be the meatflag
		LastMatchBots[AddIndex].TeamNum = 2;
	}
}

/**
 * Builds a single list of players from the last match array
 *
 * @param Players the array getting the data copied into it
 */
function GetPlayersForStatsRead(out array<UniqueNetId> Players)
{
	local int PlayerIndex;

	Players.Length = 0;
	// Look at each registered party and add them for showing
	for (PlayerIndex = 0; PlayerIndex < LastMatchPlayers.Length; PlayerIndex++)
	{
		Players.AddItem(LastMatchPlayers[PlayerIndex].NetId);
	}
}

/**
 * Finds the player indicated and returns their skill rating
 *
 * @param Player the player to search for
 *
 * @return the skill for the specified player
 */
function int GetSkillForPlayer(UniqueNetId Player)
{
	local int PlayerIndex;

	// Search for the specified player and return their skill
	for (PlayerIndex = 0; PlayerIndex < LastMatchPlayers.Length; PlayerIndex++)
	{
		if (LastMatchPlayers[PlayerIndex].NetId == Player)
		{
			return LastMatchPlayers[PlayerIndex].Skill;
		}
	}
	return 0;
}

/**
 * Finds the player indicated and returns their name
 *
 * @param Player the player to search for
 *
 * @return the name for the player
 */
function string GetNameForPlayer(UniqueNetId Player)
{
	local int PlayerIndex;

	for (PlayerIndex = 0; PlayerIndex < LastMatchPlayers.Length; PlayerIndex++)
	{
		if (LastMatchPlayers[PlayerIndex].NetId == Player)
		{
			return LastMatchPlayers[PlayerIndex].Name;
		}
	}
	return "";
}

/**
 * Finds the player indicated and returns whether they finished the game or not
 *
 * @param Player the player to search for
 *
 * @return whether the player finished the game or not
 */
function bool PlayerFinishedGame(UniqueNetId Player)
{
	local int PlayerIndex;

	for (PlayerIndex = 0; PlayerIndex < LastMatchPlayers.Length; PlayerIndex++)
	{
		if (LastMatchPlayers[PlayerIndex].NetId == Player)
		{
			return LastMatchPlayers[PlayerIndex].bDidComplete;
		}
	}
	return false;
}

/**
 * Finds the player indicated and returns their pawn class name
 *
 * @param Player the player to search for
 *
 * @return the name for the player's pawn class
 */
function string GetPawnClassNameForPlayer(UniqueNetId Player)
{
	local int PlayerIndex;

	for (PlayerIndex = 0; PlayerIndex < LastMatchPlayers.Length; PlayerIndex++)
	{
		if (LastMatchPlayers[PlayerIndex].NetId == Player)
		{
			return LastMatchPlayers[PlayerIndex].PawnClassName;
		}
	}
	return "";
}

/**
 * Finds the player indicated and returns their pawn class name
 *
 * @param Player the player to search for
 *
 * @return the name for the player's pawn class
 */
function string GetTeamNameForPlayer(UniqueNetId Player)
{
	local int PlayerIndex;

	for (PlayerIndex = 0; PlayerIndex < LastMatchPlayers.Length; PlayerIndex++)
	{
		if (LastMatchPlayers[PlayerIndex].NetId == Player)
		{
			return LastMatchPlayers[PlayerIndex].TeamName;
		}
	}
	return "";
}

/**
 * Finds the team and returns their score
 *
 * @param TeamNum the team to search for
 *
 * @return the score for the team
 */
function int GetScoreForTeam(int TeamNum)
{
	local int PlayerIndex;

	// First try players
	for (PlayerIndex = 0; PlayerIndex < LastMatchPlayers.Length; PlayerIndex++)
	{
		if (LastMatchPlayers[PlayerIndex].TeamNum == TeamNum)
		{
			return LastMatchPlayers[PlayerIndex].TeamScore;
		}
	}
	// Now try bots
	for (PlayerIndex = 0; PlayerIndex < LastMatchBots.Length; PlayerIndex++)
	{
		if (LastMatchBots[PlayerIndex].TeamNum == TeamNum)
		{
			return LastMatchBots[PlayerIndex].TeamScore;
		}
	}
	return 0;
}

/**
 * Finds the team and returns their localized team name
 *
 * @param TeamNum the team to search for
 *
 * @return localized team name
 */
function string GetTeamName(int TeamNum)
{
	local int PlayerIndex;

	// First try players
	for (PlayerIndex = 0; PlayerIndex < LastMatchPlayers.Length; PlayerIndex++)
	{
		if (LastMatchPlayers[PlayerIndex].TeamNum == TeamNum)
		{
			return LastMatchPlayers[PlayerIndex].TeamName;
		}
	}
	// Now try bots
	for (PlayerIndex = 0; PlayerIndex < LastMatchBots.Length; PlayerIndex++)
	{
		if (LastMatchBots[PlayerIndex].TeamNum == TeamNum)
		{
			return LastMatchBots[PlayerIndex].TeamName;
		}
	}
	return "";
}

/**
 * Builds a single list of players from the last match array for a specific team
 *
 * @param TeamNum the team that we are reading the players for
 * @param Players the array getting the data copied into it
 */
function GetPlayersForTeam(int TeamNum,out array<UniqueNetId> Players)
{
	local int PlayerIndex;

	Players.Length = 0;
	// Look at each registered party and add them for showing
	for (PlayerIndex = 0; PlayerIndex < LastMatchPlayers.Length; PlayerIndex++)
	{
		if (LastMatchPlayers[PlayerIndex].TeamNum == TeamNum)
		{
			Players.AddItem(LastMatchPlayers[PlayerIndex].NetId);
		}
	}
}

/**
 * Builds a single list of bots from the last match array for a specific team
 *
 * @param TeamNum the team that we are reading the bots for
 * @param Bots the array holding the indexes in the LastMatchBots for the bots on this team
 */
function GetBotsForTeam(int TeamNum,out array<int> Bots)
{
	local int PlayerIndex;

	Bots.Length = 0;
	// Look at each registered party and add them for showing
	for (PlayerIndex = 0; PlayerIndex < LastMatchBots.Length; PlayerIndex++)
	{
		if (LastMatchBots[PlayerIndex].TeamNum == TeamNum)
		{
			Bots.AddItem(PlayerIndex);
		}
	}
}

/**
 * Builds a single list of players from the last match array for a specific team
 *
 * @param TeamNum the team that we are reading the players for
 * @param Players the array getting the data copied into it
 */
function GetPlayersNotOnTeam(int TeamNum,out array<UniqueNetId> Players)
{
	local int PlayerIndex;

	Players.Length = 0;
	// Look at each registered party and add them for showing
	for (PlayerIndex = 0; PlayerIndex < LastMatchPlayers.Length; PlayerIndex++)
	{
		if (LastMatchPlayers[PlayerIndex].TeamNum != TeamNum)
		{
			Players.AddItem(LastMatchPlayers[PlayerIndex].NetId);
		}
	}
}

/** Orders the players list by their corresponding overall rank */
function SortPlayersByRank()
{
	local array<GearRecentPlayer> SortedPlayers;
	local array<int> SortedRank;
	local int PlayerIndex;
	local int SearchIndex;
	local bool bWasInserted;
	local UniqueNetId NetId;
	local int Rank;

	if (AllTimeMatchResults != None)
	{
		SortedRank.Length = 0;
		// Look at each registered player and sort them into the new array
		for (PlayerIndex = 0; PlayerIndex < LastMatchPlayers.Length; PlayerIndex++)
		{
			bWasInserted = false;
			// Get the id for the next player
			NetId = LastMatchPlayers[PlayerIndex].NetId;
			// Search for the overall rank from the leaderboard
			Rank = AllTimeMatchResults.GetRankForPlayer(NetId);
			// Force unranked players to the bottom
			if (Rank == 0)
			{
				Rank = 1000000000;
			}
			// Search the set of ranks to find where to insert
			for (SearchIndex = 0; SearchIndex < SortedRank.Length; SearchIndex++)
			{
				// See if their rank is better
				if (Rank < SortedRank[SearchIndex])
				{
					SortedPlayers.InsertItem(SearchIndex,LastMatchPlayers[PlayerIndex]);
					SortedRank.InsertItem(SearchIndex,Rank);
					bWasInserted = true;
					break;
				}
			}
			if (!bWasInserted)
			{
				SortedPlayers.AddItem(LastMatchPlayers[PlayerIndex]);
				SortedRank.AddItem(Rank);
			}
		}
		LastMatchPlayers = SortedPlayers;
	}
}

/** Erases all of the data that was set on the object */
function ClearRecentMatchResults()
{
	LastMatchPlayers.Length = 0;
	LastMatchResults = None;
	AllTimeMatchResults = None;
}

/**
 * Determines whether the player completed the match or not
 *
 * @param Player the player in question
 *
 * @return true if they finished the match, false otherwise
 */
function bool DidPlayerCompleteMatch(UniqueNetId Player)
{
	local int FoundIndex;

	FoundIndex = LastMatchPlayers.Find('NetId',Player);
	if (FoundIndex != INDEX_NONE)
	{
		return LastMatchPlayers[FoundIndex].bDidComplete;
	}
	return false;
}