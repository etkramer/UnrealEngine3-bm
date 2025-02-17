/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTDeathmatch extends UTGame
	config(game);

function bool WantsPickups(UTBot B)
{
	return true;
}

/** return a value based on how much this pawn needs help */
function int GetHandicapNeed(Pawn Other)
{
	local float ScoreDiff;

	if ( Other.PlayerReplicationInfo == None )
	{
		return 0;
	}

	// base handicap on how far pawn is behind top scorer
	GameReplicationInfo.SortPRIArray();
	ScoreDiff = GameReplicationInfo.PriArray[0].Score - Other.PlayerReplicationInfo.Score;

	if ( ScoreDiff < 3 )
	{
		// ahead or close
		return 0;
	}
	return ScoreDiff/3;
}

/**
 * Writes out the stats for the game type
 */
function WriteOnlineStats()
{
	local UTLeaderboardWriteDM Stats;
	local UTPlayerController PC;
	local UTPlayerReplicationInfo PRI;
	local UniqueNetId ZeroUniqueId;
	local int NumInactives;
	local bool bIsPureGame;
	local int i;
	local float TimeInGame;

	if ((SinglePlayerMissionID > INDEX_None) || (WorldInfo.NetMode == NM_Standalone))
	{
		//We don't record single player stats
		return;
	}
	
	// Only calc this if the subsystem can write stats
	if (OnlineSub != None && OnlineSub.StatsInterface != None)
	{
		//Epic content + No bots => Pure
		bIsPureGame = IsPureGame() && !bPlayersVsBots;

		// Iterate through the playercontroller list updating the stats
		foreach WorldInfo.AllControllers(class'UTPlayerController',PC)
		{
			PRI = UTPlayerReplicationInfo(PC.PlayerReplicationInfo);

			// Don't record stats for bots (bots have a zero unique net id)
			// don't record for spectators either (@warning: assumes active players can't become spectators)
			if (PRI != None && !PRI.bOnlySpectator && (PRI.UniqueId != ZeroUniqueId))
			{
				TimeInGame = float(WorldInfo.GRI.ElapsedTime - PRI.StartTime);
				//Game has lasted more than 10 seconds, you've been in at least 30 secs of it or 90% of the elapsed time
				if (WorldInfo.GRI.ElapsedTime > 10 && (TimeInGame >= Min(30.0f, float(WorldInfo.GRI.ElapsedTime) * 0.9f)))
			{
					//`log("Writing out stats for player"@PRI.PlayerName);
				//Write out all relevant stats
					Stats = UTLeaderboardWriteDM(new OnlineStatsWriteClass);
				Stats.CopyAndWriteAllStats(PC.PlayerReplicationInfo.UniqueId, PRI, bIsPureGame, OnlineSub.StatsInterface);
			}
			}
            else
            {
                `log("Player"@PRI.PlayerName@"did not have a valid UniqueID, stats not written");
            } 
		}

		//Write out stats of players who left the game
		NumInactives = InactivePRIArray.length;
		for (i=0; i<NumInactives; i++)
		{
			PRI = UTPlayerReplicationInfo(InactivePRIArray[i]);
		    if (PRI != None && (PRI.UniqueId != ZeroUniqueId))
			{
				`log("Writing out stats for inactive player"@PRI.PlayerName);
				//Write out all relevant stats
				Stats = UTLeaderboardWriteDM(new OnlineStatsWriteClass);
				Stats.CopyAndWriteAllStats(PRI.UniqueId, PRI, bIsPureGame, OnlineSub.StatsInterface);
			}
            else
            {
                `log("Inactive player"@PRI.PlayerName@"did not have a valid UniqueID, stats not written");
            } 
		}
	}
}

defaultproperties
{
	Acronym="DM"
	MapPrefixes[0]="DM"

	// Class used to write stats to the leaderboard
	OnlineStatsWriteClass=class'UTGame.UTLeaderboardWriteDM'
	// Default set of options to publish to the online service
	OnlineGameSettingsClass=class'UTGame.UTGameSettingsDM'

	bScoreDeaths=true

	// Deathmatch games don't care about teams for voice chat
	bIgnoreTeamForVoiceChat=true
}
