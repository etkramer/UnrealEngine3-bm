/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPC_Wingman extends GearPC;

/**
 * Tells the clients to write the stats using the specified stats object
 *
 * @param OnlineStatsWriteClass the stats class to write with
 */
reliable client function ClientWriteLeaderboardStats(class<OnlineStatsWrite> OnlineStatsWriteClass)
{
 	local GearLeaderboardWriteBase Stats;
 	local GearPRI PRI;
 	local GearGRI GRI;
 	local int Index;
	local UniqueNetId ZeroId;
	local OnlineGameSettings GameSettings;
	local bool bDidWin;
	local int RoundsWon;
	local int RoundsLost;
	local int TeamIndex;

	// Only calc this if the subsystem can write stats
 	if (OnlineSub != None &&
 		OnlineSub.StatsInterface != None &&
 		OnlineSub.GameInterface != None &&
 		OnlineStatsWriteClass != None)
 	{
		// Get the game setting so we can determine whether to write stats for none, one or all players
		GameSettings = OnlineSub.GameInterface.GetGameSettings(PlayerReplicationInfo.SessionName);
		if (GameSettings != None)
		{
			// Store the score data for review in the party lobby
			StoreLastMatchData(OnlineStatsWriteClass,GameSettings.bUsesArbitration);
			// Create the write objec that we are going to submit to live
 			Stats = GearLeaderboardWriteBase(new OnlineStatsWriteClass);
			// Get the GRI so we can iterate players
			GRI = GearGRI(WorldInfo.GRI);
			// Arbitration requires us to report for everyone, whereas non-arbitrated is just for ourselves
			if (GameSettings.bUsesArbitration)
			{
 				// Iterate through the active PRI list updating the stats
 				for (Index = 0; Index < GRI.PRIArray.Length; Index++)
 				{
 					PRI = GearPRI(GRI.PRIArray[Index]);
 					if (!PRI.bIsInactive &&
						PRI.UniqueId != ZeroId)
 					{
						bDidWin = true;
						// Search through all teams and see if we had the highest score or not
						for (TeamIndex = 0; TeamIndex < GRI.Teams.Length; TeamIndex++)
						{
							if (PRI.Team.TeamIndex != GRI.Teams[TeamIndex].TeamIndex)
							{
`Log("PRI.Team.Score = "$PRI.Team.Score$" GRI.Teams["$TeamIndex$"].Score = "$GRI.Teams[TeamIndex].Score);
								// See if we lost this match
								if (PRI.Team.Score < GRI.Teams[TeamIndex].Score)
								{
									bDidWin = false;
									break;
								}
							}
						}
`Log("bDidWin = "$bDidWin);
						if (bDidWin)
						{
							RoundsWon = 1;
							RoundsLost = 0;
						}
						else
						{
							RoundsWon = 0;
							RoundsLost = 1;
						}
`Log("RoundsWon = "$RoundsWon);
`Log("RoundsLost = "$RoundsLost);
						// Copy the stats from the PRI to the object
						Stats.UpdateFromPRI(PRI,1,RoundsWon,RoundsLost);
 						// This will copy them into the online subsystem where they will be
 						// sent via the network in EndOnlineGame()
 						OnlineSub.StatsInterface.WriteOnlineStats(PRI.SessionName,PRI.UniqueId,Stats);
 					}
 				}
			}
			else
			{
				PRI = GearPRI(PlayerReplicationInfo);
				bDidWin = true;
				for (TeamIndex = 0; TeamIndex < GRI.Teams.Length; TeamIndex++)
				{
					if (PRI.Team.TeamIndex != GRI.Teams[TeamIndex].TeamIndex)
					{
`Log("PRI.Team.Score = "$PRI.Team.Score$" GRI.Teams["$TeamIndex$"].Score = "$GRI.Teams[TeamIndex].Score);
						// See if we lost this match
						if (PRI.Team.Score < GRI.Teams[TeamIndex].Score)
						{
							bDidWin = false;
							break;
						}
					}
				}
`Log("bDidWin = "$bDidWin);
				if (bDidWin)
				{
					RoundsWon = 1;
					RoundsLost = 0;
				}
				else
				{
					RoundsWon = 0;
					RoundsLost = 1;
				}
`Log("RoundsWon = "$RoundsWon);
`Log("RoundsLost = "$RoundsLost);
				// Copy the stats from the PRI to the object
				Stats.UpdateFromPRI(PRI,1,RoundsWon,RoundsLost);
				// This will copy them into the online subsystem where they will be
				// sent via the network in EndOnlineGame()
				OnlineSub.StatsInterface.WriteOnlineStats(PRI.SessionName,PRI.UniqueId,Stats);
			}
		}
 	}
}
