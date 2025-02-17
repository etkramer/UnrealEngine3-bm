/**
 * Displays stats for all players from the previous match.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUISceneFE_PostGamePlayer extends GearUISceneFE_PlayerStatBase;

/** The object that holds the last matches' results */
var transient GearRecentPlayersList PlayersList;


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/
/**
 * Allows others to be notified when this scene becomes the active scene.  Called after other activation methods have been called
 * and after focus has been set on the scene
 *
 * @param	ActivatedScene			the scene that was activated
 * @param	bInitialActivation		TRUE if this is the first time this scene is being activated; FALSE if this scene has become active
 *									as a result of closing another scene or manually moving this scene in the stack.
 */
function SceneActivationComplete( UIScene ActivatedScene, bool bInitialActivation )
{
	if ( ActivatedScene == Self && bInitialActivation )
	{
		if (OnlineSub != None)
		{
			// Get the recent player data for the last match stats
			PlayersList = GearRecentPlayersList(OnlineSub.GetNamedInterface('RecentPlayersList'));
		}

		InitializePlayerData();
	}
}

/** Whether this is the post game stats screen or not */
function bool IsPostGameStats()
{
	return true;
}

/** Returns the background style name for the style to use for this player */
function name GetPlayerBackgroundStyleName()
{
	local name TeamBGName;
	local string PawnClassName;

	PawnClassName = PlayersList.GetPawnClassNameForPlayer(PlayerNetworkId);
	if (PlayersList != none &&
		!PlayersList.PlayerFinishedGame(PlayerNetworkId))
	{
		TeamBGName = 'img_PlayerStats_Error';
	}
	else if (InStr(PawnClassName, "COG") != -1)
	{
		TeamBGName = 'img_PlayerStats_Cog';
	}
	else if (InStr(PawnClassName, "Locust") != -1)
	{
		TeamBGName = 'img_PlayerStats_Locust';
	}
	else
	{
		TeamBGName = 'img_PlayerStats_Neutral';
	}

	return TeamBGName;
}

/** Sets the image component in the UIImage using the netId to find the portrait to draw */
function SetCharacterImage(UIImage ImageWidget)
{
	local string PawnClassName;
	local GearGameCharacterSummary CharacterData;
	local Surface Portrait;

	PawnClassName = PlayersList.GetPawnClassNameForPlayer(PlayerNetworkId);
	if (InStr(PawnClassName, "COG") != -1)
	{
		CharacterData = class'GearUIDataStore_GameResource'.static.GetCharacterProviderUsingClassName("GearGameContent."$PawnClassName, 'COGs');
	}
	else if (InStr(PawnClassName, "Locust") != -1)
	{
		CharacterData = class'GearUIDataStore_GameResource'.static.GetCharacterProviderUsingClassName("GearGameContent."$PawnClassName, 'Locusts');
	}
	else
	{
		CharacterData = class'GearUIDataStore_GameResource'.static.GetCharacterProviderUsingClassName("GearGameContent."$PawnClassName, 'Meatflags');
	}

	if (CharacterData != none)
	{
		Portrait = Surface(FindObject(CharacterData.PortraitIcon.ImagePathName, class'Surface'));
		ImageWidget.ImageComponent.SetImage(Portrait);
		ImageWidget.ImageComponent.SetCoordinates(CharacterData.PortraitIcon.Coordinates);
		ImageWidget.SetWidgetStyleByName('Image Style', 'Default Image Style');
	}
	else
	{
		ImageWidget.SetWidgetStyleByName( 'Image Style', 'imgBlank' );
	}
}

/** Returns the string of the name of the player using the netId as the key */
function String GetPlayerName()
{
	if (PlayersList != None && PlayersList.LastMatchResults != None)
	{
		return PlayersList.GetNameForPlayer(PlayerNetworkId);
	}
	return Super.GetPlayerName();
}

/** Returns the string of the rank of the player using the netId as the key */
function String GetPlayerRank()
{
	if (PlayersList != None && PlayersList.LastMatchResults != None)
	{
		return GetPlayerSkillString(PlayersList.GetSkillForPlayer(PlayerNetworkId));
	}
	return Super.GetPlayerRank();
}

/** Returns the string of the leaderboard rank of the player, using the netId as the key */
function String GetPlayerLeaderRank()
{
	local int Rank;

	if (PlayersList != None && PlayersList.AllTimeMatchResults != None)
	{
		Rank = PlayersList.AllTimeMatchResults.GetRankForPlayer(PlayerNetworkId);
		return Rank == 0 ? " " : string(Rank);
	}
	return Super.GetPlayerLeaderRank();
}

/** Returns the string of the stat for the player using the netId as the key */
function String GetPlayerStatValue(bool bIsLifetime,int StatId)
{
	local int Value;
	local float FloatValue;
	local GearLeaderboardBase Leaderboard;

	if (StatId > -1)
	{
		if (PlayersList != None)
		{
			Leaderboard = bIsLifetime ? PlayersList.AllTimeMatchResults : PlayersList.LastMatchResults;
			if (Leaderboard != None)
			{
				switch (StatId)
				{
					case STATS_COLUMN_KILLDEATH_RATIO:
					case STATS_COLUMN_POINTSPERMATCH_RATIO:
					case STATS_COLUMN_POINTSPERROUND_RATIO:
						if (bIsLifetime)
						{
							// Get the specified column from the stats code (last match)
							if (Leaderboard.GetFloatStatValueForPlayer(PlayerNetworkId,StatId,FloatValue))
							{
								if (FloatValue != 0)
								{
									// Snip off everything except one digit past the decimel
									return TruncateFloatString(string(FloatValue));
								}
								return "--";
							}
						}
						return "";
						break;
					default:
						// Get the specified column from the stats code (last match)
						if (Leaderboard.GetIntStatValueForPlayer(PlayerNetworkId,StatId,Value))
						{
							return string(Value);
						}
						break;
				}
			}
		}

		return Super.GetPlayerStatValue(bIsLifetime, StatId);
	}
	return "";
}

defaultproperties
{
	OnSceneActivated=SceneActivationComplete

	// the maximum number of stat groups we need to display (num generic stats + max num game-specific stats)
	NumStatGroups=22
}


