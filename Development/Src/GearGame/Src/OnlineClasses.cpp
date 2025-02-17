/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "GearGame.h"

#include "UnIpDrv.h"
#include "GearGameOnlineClasses.h"

IMPLEMENT_CLASS(UGearCoopGameSearch);
IMPLEMENT_CLASS(UGearPartyGameSearch);
IMPLEMENT_CLASS(UGearVersusGameSearch);
IMPLEMENT_CLASS(UGearVersusGameSettings);
IMPLEMENT_CLASS(UGearLeaderboardVersusSkill);
IMPLEMENT_CLASS(UGearLANPartySearch);
IMPLEMENT_CLASS(UGearCoopLANGameSearch);

/** Helper class that should be declared in the function using it for minimal scoping, but gcc doesn't handle this properly */
class PingSorter
{
public:
	static inline INT Compare(const FOnlineGameSearchResult& A,const FOnlineGameSearchResult& B)
	{
		// Sort ascending based off of ping
		return A.GameSettings->PingInMs - B.GameSettings->PingInMs;
	}
};

/**
 * Sorts the results by ping since skill doesn't matter here
 */
void UGearCoopGameSearch::SortSearchResults(void)
{
	// Now sort the results
	Sort<FOnlineGameSearchResult,PingSorter>(Results.GetTypedData(),Results.Num());
}

/**
 * Sorts the results by ping since skill doesn't matter here
 */
void UGearPartyGameSearch::SortSearchResults(void)
{
	// Now sort the results
	Sort<FOnlineGameSearchResult,PingSorter>(Results.GetTypedData(),Results.Num());
}

/** Helper class that should be declared in the function using it for minimal scoping, but gcc doesn't handle this properly */
class MatchSorter
{
public:
	static inline INT Compare(const FOnlineGameSearchResult& A,const FOnlineGameSearchResult& B)
	{
		const UGearVersusGameSettings* SettingsA = Cast<UGearVersusGameSettings>(A.GameSettings);
		const UGearVersusGameSettings* SettingsB = Cast<UGearVersusGameSettings>(B.GameSettings);
		// Sort ascending based off of the match quality determined previously
		const FLOAT Difference = SettingsA->OverallMatchQuality - SettingsB->OverallMatchQuality;
		INT SortValue = 0;
		if (Difference < -KINDA_SMALL_NUMBER)
		{
			SortValue = -1;
		}
		else if (Difference > KINDA_SMALL_NUMBER)
		{
			SortValue = 1;
		}
		return SortValue;
	}
};

/**
 * Sorts the results using our target match quality and target ping
 * Formula: (MaxMatchQuality - MatchQuality) * (MatchPing / TargetPing)
 * Then we sort the results from lowest to highest
 */
void UGearVersusGameSearch::SortSearchResults(void)
{
	TargetPing = Max(TargetPing,1);
	MaxMatchQuality = MaxMatchQuality == 0.f ? 1.f : MaxMatchQuality;
	// Determine the overall match quality for each match and then sort
	for (INT Index = 0; Index < Results.Num(); Index++)
	{
		UGearVersusGameSettings* GameSettings = Cast<UGearVersusGameSettings>(Results(Index).GameSettings);
		if (GameSettings)
		{
			// Set a min ping so skill still comes into play
			if (GameSettings->PingInMs == 0)
			{
				GameSettings->PingInMs = 10;
			}
			// Determine the overall match quality and store it
			FLOAT MatchQuality = MaxMatchQuality - GameSettings->MatchQuality;
			FLOAT Ping = (FLOAT)GameSettings->PingInMs / (FLOAT)TargetPing;
			GameSettings->OverallMatchQuality = MatchQuality * Ping;
		}
	}
	// Now sort the results
	Sort<FOnlineGameSearchResult,MatchSorter>(Results.GetTypedData(),Results.Num());
#if !FINAL_RELEASE
	debugf(NAME_DevOnline,TEXT("Sort order for search results is:"));
	for (INT Index = 0; Index < Results.Num(); Index++)
	{
		UGearVersusGameSettings* GameSettings = Cast<UGearVersusGameSettings>(Results(Index).GameSettings);
		if (GameSettings)
		{
			debugf(NAME_DevOnline,TEXT("%s overall quality %f"),*GameSettings->OwningPlayerName,GameSettings->OverallMatchQuality);
		}
	}
#endif
}

/**
 * Copies the results of the stats read to the search object
 * NOTE: This only exists due to a script native method bug for structs that have special export text (double, qword)
 *
 * @param Search the search object to populate with skill data
 */
void UGearLeaderboardVersusSkill::CopySkillDataToSearch(UOnlineGameSearch* Search)
{
	if (Search != NULL)
	{
		// Copy the skill leaderboard information over
		Search->ManualSkillOverride.LeaderboardId = ViewId;
		INT PlayerCount = Rows.Num();
		// Preallocate our space for exactly the number we need (no slack)
		Search->ManualSkillOverride.Players.Empty(PlayerCount);
		Search->ManualSkillOverride.Mus.Empty(PlayerCount);
		Search->ManualSkillOverride.Sigmas.Empty(PlayerCount);
		// Each row represents a player
		for (INT Index = 0; Index < PlayerCount; Index++)
		{
			// Default to middle of the range with 100% uncertainty
			DOUBLE Mu = 3.0;
			DOUBLE Sigma = 1.0;
			const FOnlineStatsRow& Row = Rows(Index);
			Search->ManualSkillOverride.Players.AddItem(Row.PlayerID);
			// Check each column to find the values we are looking for
			for (INT ColIndex = 0; ColIndex < Row.Columns.Num(); ColIndex++)
			{
				const FOnlineStatsColumn& Column = Row.Columns(ColIndex);
				switch (Column.ColumnNo)
				{
					case UCONST_STATS_COLUMN_MU:
					{
						Column.StatValue.GetData(Mu);
						break;
					}
					case UCONST_STATS_COLUMN_SIGMA:
					{
						Column.StatValue.GetData(Sigma);
						break;
					}
				}
			}
			// Make sure the player isn't new to the leaderboard
			if (Mu == 0.0 && Sigma == 0.0)
			{
				// Default to middle of the range with 100% uncertainty
				Mu = 3.0;
				Sigma = 1.0;
			}
			Search->ManualSkillOverride.Mus.AddItem(Mu);
			Search->ManualSkillOverride.Sigmas.AddItem(Sigma);
		}
	}
}

/**
 * Populates the player reservation structure with the skill data contained herein
 *
 * @param PlayerId the player to read the skill data for
 * @param PlayerRes the reservation that is populated with the skill data
 */
void UGearLeaderboardVersusSkill::FillPlayerResevation(FUniqueNetId PlayerId,FPlayerReservation& PlayerRes)
{
	PlayerRes.NetId = PlayerId;
	// Find the player in the rows returned
	// Search the rows for the player and return the score for them
	for (INT RowIndex = 0; RowIndex < Rows.Num(); RowIndex++)
	{
		const FOnlineStatsRow& Row = Rows(RowIndex);
		if (Row.PlayerID == PlayerId)
		{
			DOUBLE Mu = 3.0;
			DOUBLE Sigma = 1.0;
			INT Skill = 0;
			// Check each column to find the values we are looking for
			for (INT ColIndex = 0; ColIndex < Row.Columns.Num(); ColIndex++)
			{
				const FOnlineStatsColumn& Column = Row.Columns(ColIndex);
				switch (Column.ColumnNo)
				{
					case UCONST_STATS_COLUMN_MU:
					{
						Column.StatValue.GetData(Mu);
						break;
					}
					case UCONST_STATS_COLUMN_SIGMA:
					{
						Column.StatValue.GetData(Sigma);
						break;
					}
					case UCONST_STATS_COLUMN_SKILL:
					{
						QWORD Value = 0;
						Column.StatValue.GetData(Value);
						Skill = (INT)(Value & 0xFFFFFFFF);
						break;
					}
				}
			}
			PlayerRes.Mu = Mu;
			PlayerRes.Sigma = Sigma;
			PlayerRes.Skill = Skill;
			return;
		}
	}
	// Wasn't found so use an unknown skill
	PlayerRes.Mu = 3.0;
	PlayerRes.Sigma = 1.0;
	PlayerRes.Skill = 0;
}

/**
 * Gets the skill value for the player specified
 *
 * @param PlayerId the unique id of the player to look up
 *
 * @return the skill value for that player
 */
INT UGearLeaderboardVersusSkill::GetSkillForPlayer(FUniqueNetId PlayerID)
{
	// Search the rows for the player and return the score for them
	for (INT RowIndex = 0; RowIndex < Rows.Num(); RowIndex++)
	{
		if (Rows(RowIndex).PlayerID == PlayerID)
		{
			// Search the columns for the high score
			for (INT ColumnIndex = 0; ColumnIndex < Rows(RowIndex).Columns.Num(); ColumnIndex++)
			{
				if (Rows(RowIndex).Columns(ColumnIndex).ColumnNo == UCONST_STATS_COLUMN_SKILL)
				{
					QWORD Value = 0;
					Rows(RowIndex).Columns(ColumnIndex).StatValue.GetData(Value);
					return (INT)(Value & 0xFFFFFFFF);
				}
			}
			return 0;
		}
	}
	return 0;
}

/**
 * Filters out any coop sessions
 */
void UGearLANPartySearch::SortSearchResults(void)
{
	// Search the results for any coop sessions and discard
	for (INT Index = 0; Index < Results.Num(); Index++)
	{
		FOnlineGameSearchResult& SearchResult = Results(Index);
		UOnlineGameSettings* GameSettings = Results(Index).GameSettings;
		INT GameMode = UCONST_CONTEXT_GAME_MODE_COOP;
		// Look at the game mode and remove any coop sessions
		if (GameSettings &&
			(GameSettings->GetStringSettingValue(UCONST_CONTEXT_GAME_MODE,GameMode) == FALSE ||
			GameMode == UCONST_CONTEXT_GAME_MODE_COOP))
		{
#if _XBOX
			// Free the data
			delete (XSESSION_INFO*)SearchResult.PlatformData;
#endif
			// And then remove from the list
			Results.Remove(Index);
			Index--;
		}
	}
}

/**
 * Sorts the results using our target match quality and target ping
 * Formula: (MaxMatchQuality - MatchQuality) * (MatchPing / TargetPing)
 * Then we sort the results from lowest to highest
 */
void UGearCoopLANGameSearch::SortSearchResults(void)
{
	// Search the results for any MP sessions and discard
	for (INT Index = 0; Index < Results.Num(); Index++)
	{
		FOnlineGameSearchResult& SearchResult = Results(Index);
		UOnlineGameSettings* GameSettings = Results(Index).GameSettings;
		INT GameMode = UCONST_CONTEXT_GAME_MODE_VERSUS;
		// Look at the game mode and remove any MP sessions
		if (GameSettings &&
			(GameSettings->GetStringSettingValue(UCONST_CONTEXT_GAME_MODE,GameMode) == FALSE ||
			GameMode == UCONST_CONTEXT_GAME_MODE_PARTY))
		{
#if _XBOX
			// Free the data
			delete (XSESSION_INFO*)SearchResult.PlatformData;
#endif
			// And then remove from the list
			Results.Remove(Index);
			Index--;
		}
	}
}
