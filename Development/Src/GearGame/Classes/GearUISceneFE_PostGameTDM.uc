/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUISceneFE_PostGameTDM extends GearUISceneFrontEnd_Base
	config(inherit);

`include(GearOnlineConstants.uci)

/************************************************************************/
/* Constants, enums, structs, etc.										*/
/************************************************************************/

//////////////////////// SCOREBOARD/STATS //////////////////////////////////

/** Struct to store all of the data needed for a player in the game stats */
struct GearPlayerStatData
{
	/** The unique network id of this player (can be used as the key to get stats data) */
	var transient UniqueNetId PlayerNetId;
	/** If this is a bot, this is the index in the LastMatchBots array of the RecentPlayersList for this bot */
	var transient int BotListIndex;
	/** Whether this is a human player who quit the game or not */
	var transient bool bIsQuitter;

	//------ Widget references ---------
	/** The parent button of all widgets for a player */
	var transient UILabelButton ParentButton;
	/** Label for the rank of this player */
	var transient UILabel RankLabel;
	/** Label for the total score for this player */
	var transient UILabel ScoreLabel;
	/** Labels for the stats (from left to right) */
	var transient array<UILabel> StatLabels;
};

/** Struct to store all of the data needed for a team in the game stats */
struct GearTeamStatData
{
	/** Array of data for each player on the team */
	var transient array<GearPlayerStatData> PlayerDataList;
	/** The team index of this team data */
	var transient int TeamIndex;
	/** The parent panel of all widgets for a team */
	var transient UIPanel ParentPanel;
	/** Image for the race icon */
	var transient UIImage RaceImage;
	/** Label for the team-name (only used on game-types with more than 2 teams) */
	var transient UILabel TeamNameLabel;
	/** Labels for this team's score */
	var transient UILabel ScoreLabel;
	var transient UILabel ScoreLabel2;
	/** The race of this team */
	var transient EGearRaceTypes RaceType;
};

/** Struct to store all of the data needed for this game in the game stats */
struct GearGameStatData
{
	/** Parent panel of the entire section */
	var transient UIPanel ParentPanel;
	/** Array of data for each team in the game */
	var transient array<GearTeamStatData> TeamDataList;

	//------ Widget references ---------
	/** Images for describing the stat columns 0-3 (left to right) */
	var transient array<UIImage> StatImages;
};

//////////////////////// LEADERBOARDS ////////////////////////////////////////

/** Struct to store all of the data needed for a player in the leaderboards */
struct GearPlayerLeaderData
{
	/** The unique network id of this player (can be used as the key to get stats data) */
	var transient UniqueNetId PlayerNetId;
	/** Whether this is a human player who quit the game or not */
	var transient bool bIsQuitter;

	//------ Widget references ---------
	/** The parent button of all widgets for a player */
	var transient UILabelButton ParentButton;
	/** Label for the rank of this player */
	var transient UILabel RankLabel;
	/** Label for the total score for this player */
	var transient UILabel ScoreLabel;
	/** The team index of this player */
	var transient int TeamIndex;
	/** The race of team this player was on */
	var transient EGearRaceTypes RaceType;
};

/** Struct to store all of the data needed for this game in the leaderboards */
struct GearGameLeaderData
{
	//------ Widget references ---------
	/** Parent panel of the entire section */
	var transient UIPanel ParentPanel;
	/** Array of data for each team in the game */
	var transient array<GearPlayerLeaderData> PlayerDataList;
};

//////////////////////// GENERAL ////////////////////////////////////////

/** The Display Modes this screen can be in */
enum EGearPostGameMode
{
	eGPGM_GameStats,
	eGPGM_Leaderboard,
};

/** Structure to store the misc data needed no matter what gametype or mode we are in */
struct GearGenericData
{
	/** The display mode we are currently in */
	var transient EGearPostGameMode PostGameMode;
	/** The game type most recently played */
	var transient EGearMPTypes GameType;
	/** The mapname of the game that was just played */
	var transient String MapName;

	//------ Widget references ---------
	/** Label for the game/mapname */
	var transient UILabel GameAndMapLabel;
	/** Label for post game mode we are currently in */
	var transient UILabel PostGameModeLabel;
	/** The screen's buttonbar */
	var transient UICalloutButtonPanel ButtonBar;
};

/** Struct to store game-type specific variables, delegates, etc. */
struct GearGameTypeSpecificStatData
{
	/** Stat order */
	var transient const array<GearScoreboardStatType> StatOrder;
};

/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** The non-game-specific and non-mode-specific data for the scene */
var transient GearGenericData GenericData;

/** The data needed when we are showing the leaderboards */
var transient GearGameLeaderData LeaderData;

/** The data needed when we are showing team/game stats */
var transient GearGameStatData StatData;

/**
 * List of data that allows for game-specific customization of the scoreboard
 * This list must match the EGearMPTypes in GearTypes.uc
 */
var transient array<GearGameTypeSpecificStatData> GameSpecificData;

/** Styles for the header of each stat in the game */
var const array<String> StatHeaderStyles;
/** Constants that map a stat to the data in the leaderboard/recentplayerlist */
var const array<int> StatConstants;

/** Localized strings needed for this scene */
var localized string StatModeString;
var localized string LeaderModeString;

/** The object that holds the last matches' results */
var transient GearRecentPlayersList PlayersList;

/** Access to the online subsystem for grabbing stats data */
var transient OnlineSubsystem OnlineSub;

/** The id of the player we are getting detailed information for */
var transient UniqueNetId DetailStatsPlayerId;

/** The first and last buttons in the scoreboard list */
var transient UILabelButton FirstScoreButton;
var transient UILabelButton LastScoreButton;
var transient UILabelButton FirstLeaderButton;
var transient UILabelButton LastLeaderButton;

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/**
 * Called after this screen object's children have been initialized
 * Overloaded to initialized the references to this scene's widgets
 */
event PostInitialize()
{
	local array<UniqueNetId> Players;

	if (!IsEditor())
	{
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if (OnlineSub != None)
		{
			// Get the recent player data for the last match stats
			PlayersList = GearRecentPlayersList(OnlineSub.GetNamedInterface('RecentPlayersList'));
			if (PlayersList != None)
			{
				// Build the list of players that we want to do the read for and kick that off
				PlayersList.GetPlayersForStatsRead(Players);
				// Now start the read of the all time data
				OnlineSub.StatsInterface.AddReadOnlineStatsCompleteDelegate(OnAllTimeReadComplete);
				OnlineSub.StatsInterface.ReadOnlineStats(Players,PlayersList.AllTimeMatchResults);
				// open COGSpin.com
				OpenUpdatingStatsScene();
			}
		}
		InitializePostGameData();
	}

	Super.PostInitialize();
}

/**
 * Called once the stats read has completed
 *
 * @param bWasSuccessful whether the call completed ok or not
 */
function OnAllTimeReadComplete(bool bWasSuccessful)
{
	OnlineSub.StatsInterface.ClearReadOnlineStatsCompleteDelegate(OnAllTimeReadComplete);

	SetPostGameData();
	SetPostGameMode(eGPGM_GameStats);
	// close COGSpin.com
	CloseUpdatingStatScene();
}

/** Opens or restarts the updating scene (used for blocking input during async tasks) */
function OpenUpdatingStatsScene()
{
	local GearUISceneFE_Updating SceneInstance;

	SceneInstance = class'GearUIScene_Base'.static.OpenUpdatingScene();
	if (SceneInstance != None)
	{
		SceneInstance.InitializeUpdatingScene("DLStatsTitle", "DLStatsDesc", 0.5f);
	}
}

/** Begins the process of closing the updating scene (there is a min time the scene must be open) */
function CloseUpdatingStatScene()
{
	class'GearUIScene_Base'.static.CloseUpdatingScene();
}

/** Initialize the scoreboard - widget references etc. */
final function InitializePostGameData()
{
	local int NumPlayersPerTeam;
	local int Index;
	local string WidgetName;
	local UIPanel ParentPanel;
	local int TeamIdx;
	local int PlayerIdx;
	local UIPanel TeamPanel;
	local bool bIsMeatflag;
	local UILabelButton PlayerButton;
	local int StatIdx;
	local UIPanel OtherGamePanel;
	local String StyleString;
	local String LabelString;

	//////////////////////// GENERAL ////////////////////////////////////////

	// Default the screen to the scoreboard when entering it
	GenericData.PostGameMode = eGPGM_GameStats;
	// Determine the type of game this is (this will have impact on how many widget references will be initialized)
	GenericData.GameType = GetMPGameType();
	GenericData.MapName = GetMPMapName();
	// Map and Game label
	GenericData.GameAndMapLabel = UILabel(FindChild('lblMatchTitle', true));
	LabelString = GetGameModeString(GenericData.GameType) @ GameToMapConjunction @ GetMapNameString(GenericData.MapName);
	GenericData.GameAndMapLabel.SetDataStoreBinding(LabelString);
	// Mode title
	GenericData.PostGameModeLabel = UILabel(FindChild('lblTitle', true));
	GenericData.PostGameModeLabel.SetDataStoreBinding(StatModeString);
	// Button bar
	GenericData.ButtonBar = UICalloutButtonPanel(FindChild('pnlButtonBar', true));
	GenericData.ButtonBar.SetButtonCallback('Stats', OnPlayerStatPress);
	GenericData.ButtonBar.SetButtonCallback('ShowGamercard', OnGamercardPress);
	GenericData.ButtonBar.SetButtonCallback('GenericBack', OnBackPress);
	GenericData.ButtonBar.SetButtonCallback('PartyUp', OnPartyUpPress);

	//////////////////////// SCOREBOARD/STATS //////////////////////////////////

	// Get the necessary info needed for initializing the game specific data and make all other panels invisible
	switch (GenericData.GameType)
	{
		case eGEARMP_Wingman:
			// Make the other panels disappear
			OtherGamePanel = UIPanel(FindChild('pnlStatsTDM', true));
			OtherGamePanel.GetParent().RemoveChild(OtherGamePanel);
			OtherGamePanel = UIPanel(FindChild('pnlStatsHorde', true));
			OtherGamePanel.GetParent().RemoveChild(OtherGamePanel);
			// Set game specific data for this game
			StatData.ParentPanel = UIPanel(FindChild('pnlStatsWM', true));
			StatData.TeamDataList.length = 5;
			NumPlayersPerTeam = 2;
			break;
		case eGEARMP_CTM:
			// Make the other panels disappear
			OtherGamePanel = UIPanel(FindChild('pnlStatsHorde', true));
			OtherGamePanel.GetParent().RemoveChild(OtherGamePanel);
			OtherGamePanel = UIPanel(FindChild('pnlStatsWM', true));
			OtherGamePanel.GetParent().RemoveChild(OtherGamePanel);
			// Set game specific data for this game
			StatData.ParentPanel = UIPanel(FindChild('pnlStatsTDM', true));
			StatData.TeamDataList.length = 3;
			NumPlayersPerTeam = 5;
			break;
		case eGEARMP_CombatTrials:
			// Make the other panels disappear
			OtherGamePanel = UIPanel(FindChild('pnlStatsTDM', true));
			OtherGamePanel.GetParent().RemoveChild(OtherGamePanel);
			OtherGamePanel = UIPanel(FindChild('pnlStatsWM', true));
			OtherGamePanel.GetParent().RemoveChild(OtherGamePanel);
			// Set game specific data for this game
			StatData.ParentPanel = UIPanel(FindChild('pnlStatsHorde', true));
			StatData.TeamDataList.length = 1;
			NumPlayersPerTeam = 5;
			break;
		default:
			// Make the other panels disappear
			OtherGamePanel = UIPanel(FindChild('pnlStatsHorde', true));
			OtherGamePanel.GetParent().RemoveChild(OtherGamePanel);
			OtherGamePanel = UIPanel(FindChild('pnlStatsWM', true));
			OtherGamePanel.GetParent().RemoveChild(OtherGamePanel);
			// Set game specific data for this game
			StatData.ParentPanel = UIPanel(FindChild('pnlStatsTDM', true));
			StatData.TeamDataList.length = 3;
			NumPlayersPerTeam = 5;
			break;
	}
	ParentPanel = StatData.ParentPanel;
	ParentPanel.SetVisibility(true);

	// Stat images
	StatData.StatImages.length = 4;
	for (Index = 0; Index < StatData.StatImages.length; Index++)
	{
		WidgetName = "imgStat" $ Index;
		StatData.StatImages[Index] = UIImage(ParentPanel.FindChild(Name(WidgetName), true));
		StyleString = StatHeaderStyles[GameSpecificData[GenericData.GameType].StatOrder[Index]];
		SetImageStyle(StatData.StatImages[Index], Name(StyleString) );
	}

	// Initialize the widget references for teams
	for (TeamIdx = 0; TeamIdx < StatData.TeamDataList.length; TeamIdx++)
	{
		// Team panel
		WidgetName = "pnlTeam" $ TeamIdx;
		StatData.TeamDataList[TeamIdx].ParentPanel = UIPanel(ParentPanel.FindChild(Name(WidgetName), true));
		TeamPanel = StatData.TeamDataList[TeamIdx].ParentPanel;

		// If this is not a Wingman or Meatflag match we must hide all teams after the second
		if (TeamIdx > 1 && GenericData.GameType != eGEARMP_CTM && GenericData.GameType != eGEARMP_Wingman)
		{
			TeamPanel.SetVisibility(false);
			continue;
		}
		else
		{
			TeamPanel.SetVisibility(true);
		}

		// See if this is the meatflag team (he's treated 'special')
		bIsMeatflag = (GenericData.GameType == eGEARMP_CTM && TeamIdx == 2);

		// Team Meatflag has no race image or team score widgets
		if (!bIsMeatflag)
		{
			// Race image
			StatData.TeamDataList[TeamIdx].RaceImage = UIImage(TeamPanel.FindChild('imgTeamIcon', true));
			// Score labels
			StatData.TeamDataList[TeamIdx].ScoreLabel = UILabel(TeamPanel.FindChild('lblTeamScore', true));
			StatData.TeamDataList[TeamIdx].ScoreLabel2 = UILabel(TeamPanel.FindChild('lblTeamScore2', true));
			StatData.TeamDataList[TeamIdx].ScoreLabel.SetVisibility(false);
			StatData.TeamDataList[TeamIdx].ScoreLabel2.SetVisibility(false);

			if (GenericData.GameType == eGEARMP_Wingman)
			{
				// Team name label (only used in wingman)
				StatData.TeamDataList[TeamIdx].TeamNameLabel = UILabel(TeamPanel.FindChild('lblTeamName', true));
				StatData.TeamDataList[TeamIdx].TeamNameLabel.SetVisibility(false);
				// Hide the Race Image until we know which team this is going to represent
				StatData.TeamDataList[TeamIdx].RaceImage.SetVisibility(false);
			}
			else if (GenericData.GameType == eGEARMP_CombatTrials)
			{
				// Hide the race label since it's not needed in Horde
				StatData.TeamDataList[TeamIdx].RaceImage.SetVisibility(false);
			}
			else
			{
				StatData.TeamDataList[TeamIdx].RaceImage.SetVisibility(true);
			}
		}

		// Set the size of the player list
		StatData.TeamDataList[TeamIdx].PlayerDataList.length = bIsMeatflag ? 1 : NumPlayersPerTeam;

		// Initialize the widget references for players
		for (PlayerIdx = 0; PlayerIdx < StatData.TeamDataList[TeamIdx].PlayerDataList.length; PlayerIdx++)
		{
			// Player main button (parent)
			WidgetName = "btnPlayer" $ PlayerIdx;
			StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].ParentButton = UILabelButton(TeamPanel.FindChild(Name(WidgetName), true));
			PlayerButton = StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].ParentButton;
			PlayerButton.SetDataStoreBinding(" ");
			// Rank (skill) label
			StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].RankLabel = UILabel(PlayerButton.FindChild('lblRank', true));
			StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].RankLabel.SetVisibility(false);
			// Score label
			StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].ScoreLabel = UILabel(PlayerButton.FindChild('lblTotal', true));
			StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].ScoreLabel.SetVisibility(false);

			// Set the size of the stat list (meatflag only has 2 stats) and grab the labels
			StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].StatLabels.length = bIsMeatflag ? 2 : GameSpecificData[GenericData.GameType].StatOrder.length;
			for (StatIdx = 0; StatIdx < StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].StatLabels.length; StatIdx++)
			{
				// The meatflag only has 2 stats
				if (bIsMeatflag && StatIdx > 1)
				{
					break;
				}
				WidgetName = "lblStat" $ StatIdx;
				StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].StatLabels[StatIdx] = UILabel(PlayerButton.FindChild(Name(WidgetName), true));
				StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].StatLabels[StatIdx].SetVisibility(false);
			}
		}
	}

	//////////////////////// LEADERBOARDS ////////////////////////////////////////
	
	// Parent panel of the leaderboards
	LeaderData.ParentPanel = UIPanel(FindChild('pnlLeaderboard', true));
	ParentPanel = LeaderData.ParentPanel;
	ParentPanel.SetVisibility(false);

	// Initialize the players
	LeaderData.PlayerDataList.length = 10;
	for (PlayerIdx = 0; PlayerIdx < LeaderData.PlayerDataList.length; PlayerIdx++)
	{
		// Player main button (parent)
		WidgetName = "btnPlayer" $ PlayerIdx;
		LeaderData.PlayerDataList[PlayerIdx].ParentButton = UILabelButton(ParentPanel.FindChild(Name(WidgetName), true));
		PlayerButton = LeaderData.PlayerDataList[PlayerIdx].ParentButton;
		PlayerButton.SetDataStoreBinding(" ");
		SetLabelButtonBGStyle(PlayerButton, 'img_Scoreboard_NeutralRow');
		// Rank (skill) label
		LeaderData.PlayerDataList[PlayerIdx].RankLabel = UILabel(PlayerButton.FindChild('lblRank', true));
		LeaderData.PlayerDataList[PlayerIdx].RankLabel.SetVisibility(false);
		// Score label
		LeaderData.PlayerDataList[PlayerIdx].ScoreLabel = UILabel(PlayerButton.FindChild('lblTotal', true));
		LeaderData.PlayerDataList[PlayerIdx].ScoreLabel.SetVisibility(false);
	}
}

/** Refresh the button bar to reflect what functionality they have access to */
final function UpdateButtonBar()
{
	local int PlayerIndex;
	local bool bHasValidNetId;
	local UniqueNetId PlayerNetId;
	local UniqueNetId ZeroId;

	PlayerIndex = GetBestPlayerIndex();
	PlayerNetId = GetNetIdOfFocusedPlayer();
	bHasValidNetId = (PlayerNetId != ZeroId);
	GenericData.ButtonBar.EnableButton('Stats', PlayerIndex, bHasValidNetId, false);
	GenericData.ButtonBar.EnableButton('ShowGamercard', PlayerIndex, bHasValidNetId, false);
}

/** Sets the post game mode and shows/hides the correct widgets */
final function SetPostGameMode(EGearPostGameMode PGMode)
{
	if (PGMode == eGPGM_GameStats)
	{
		StatData.ParentPanel.SetVisibility(true);
		LeaderData.ParentPanel.SetVisibility(false);
		GenericData.PostGameModeLabel.SetDataStoreBinding(StatModeString);
	}
	else
	{
		StatData.ParentPanel.SetVisibility(false);
		LeaderData.ParentPanel.SetVisibility(true);
		GenericData.PostGameModeLabel.SetDataStoreBinding(LeaderModeString);
	}

	GenericData.PostGameMode = PGMode;
	SetPostGameFocus();
	UpdateButtonBar();
}

/** Sets the focus after a the post game mode was changed */
final function SetPostGameFocus()
{
	if (GenericData.PostGameMode == eGPGM_GameStats)
	{
		FirstScoreButton.SetFocus(none);
		//StatData.TeamDataList[0].PlayerDataList[0].ParentButton.SetFocus(None);
	}
	else
	{
		FirstLeaderButton.SetFocus(none);
		//LeaderData.PlayerDataList[0].ParentButton.SetFocus(None);
	}
}

/** Sets the widgets to the data they should show */
final function SetPostGameData()
{
	// First put the player in their slots
	SetPlayersToSlots();

	// Set the data in the game stats widgets
	SetGameStatData();

	// Set the data in the leaderboard widgets
	SetLeaderboardData();
}

/** Sets the data in the game stats widgets */
final function SetGameStatData()
{
	local int TeamIdx;
	local int PlayerIdx;
	local String LabelString;
	local bool bIsMeatflag;
	local bool bIsValidTeam;
	local Name StyleName;
	local Name PlayerButtonStyleName;
	local UILabelButton PlayerButton;
	local UniqueNetId PlayerNetId;
	local UniqueNetId ZeroId;
	local int StatId;
	local int StatIdx;
	local UILabel HordeScore;
	local int BotIndex;

	// Set the data in the widgets for teams
	for (TeamIdx = 0; TeamIdx < StatData.TeamDataList.length; TeamIdx++)
	{
		// See if this is the meatflag team (he's treated 'special')
		bIsMeatflag = (GenericData.GameType == eGEARMP_CTM && TeamIdx == 2);
		// See if this is a valid team (has a valid player)
		bIsValidTeam = StatTeamHasValidPlayer(TeamIdx);

		// Only set data for valid teams (not sure if meatflag will have a valid netId)
		if (bIsValidTeam || bIsMeatflag)
		{
			// Team Meatflag has no race image or team score widgets, and Horde does not display them
			if (!bIsMeatflag && (GenericData.GameType != eGEARMP_CombatTrials))
			{
				// Score labels
				LabelString = GetTeamScoreString(TeamIdx);
				StatData.TeamDataList[TeamIdx].ScoreLabel.SetDataStoreBinding(LabelString);
				StatData.TeamDataList[TeamIdx].ScoreLabel2.SetDataStoreBinding(LabelString);
				StatData.TeamDataList[TeamIdx].ScoreLabel.SetVisibility(true);
				StatData.TeamDataList[TeamIdx].ScoreLabel2.SetVisibility(true);

				if (GenericData.GameType == eGEARMP_Wingman)
				{
					// Race image (only needs set and made visible in Wingman; it's already visible for the other games)
					StyleName = (StatData.TeamDataList[TeamIdx].RaceType == eGEARRACE_COG) ? 'img_Scoreboard_CogSM' : 'img_Scoreboard_LocustSM';
					SetImageStyle(StatData.TeamDataList[TeamIdx].RaceImage, StyleName);
					StatData.TeamDataList[TeamIdx].RaceImage.SetVisibility(true);

					// Team name label (only used in wingman)
					LabelString = GetWingmanTeamName(TeamIdx);
					if (LabelString != "")
					{
						StatData.TeamDataList[TeamIdx].TeamNameLabel.SetVisibility(true);
						StatData.TeamDataList[TeamIdx].TeamNameLabel.SetDataStoreBinding(LabelString);
					}
				}
			}

			// Horde has an extra label that we must right the team score to
			if (GenericData.GameType == eGEARMP_CombatTrials)
			{
				HordeScore = UILabel(StatData.ParentPanel.FindChild('lblTeamScore', true));
				HordeScore.SetVisibility(true);
				LabelString = GetHordeTeamScore();
				HordeScore.SetDataStoreBinding(LabelString);
			}

			// Set the data in the widgets for players
			for (PlayerIdx = 0; PlayerIdx < StatData.TeamDataList[TeamIdx].PlayerDataList.length; PlayerIdx++)
			{
				// Grab a reference to the player's netId
				PlayerNetId = StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].PlayerNetId;
				// Grab  the player's bot index
				BotIndex = StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].BotListIndex;

				// If this is a quitting player, change the background style
				if (PlayerNetId != ZeroId && StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].bIsQuitter)
				{
					PlayerButtonStyleName = 'img_Scoreboard_ErrorRow';
				}
				// Determine the style name for the background of the player buttons
				else if (StatData.TeamDataList[TeamIdx].RaceType == eGEARRACE_COG)
				{
					PlayerButtonStyleName = 'img_Scoreboard_CogRow';
				}
				else if (StatData.TeamDataList[TeamIdx].RaceType == eGEARRACE_LOCUST)
				{
					PlayerButtonStyleName = 'img_Scoreboard_LocRow';
				}
				else
				{
					PlayerButtonStyleName = 'img_Scoreboard_NeutralRow';
				}

				// Set widgets for valid netIds
				if (PlayerNetId != ZeroId)
				{
					// Player main button (parent)
					PlayerButton = StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].ParentButton;
					SetLabelButtonBGStyle(PlayerButton, PlayerButtonStyleName);
					LabelString = GetPlayerNameFromNetId(PlayerNetId);
					PlayerButton.SetDataStoreBinding(LabelString);

					// Rank (skill) label
					LabelString = GetPlayerRankFromNetId(PlayerNetId);
					StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].RankLabel.SetDataStoreBinding(LabelString);
					StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].RankLabel.SetVisibility(true);

					// Score label
					LabelString = GetPlayerScoreFromNetId(PlayerNetId);
					StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].ScoreLabel.SetDataStoreBinding(LabelString);
					StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].ScoreLabel.SetVisibility(true);

					// Stats
					for (StatIdx = 0; StatIdx < StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].StatLabels.length; StatIdx++)
					{
						// Meatflag has different stats than normal players
						if (bIsMeatflag)
						{
							StatId = (StatIdx == 0) ? StatConstants[eGSStat_Kills] : StatConstants[eGSStat_Downs];
						}
						else
						{
							StatId = StatConstants[GameSpecificData[GenericData.GameType].StatOrder[StatIdx]];
						}
						LabelString = GetPlayerStatFromNetId(PlayerNetId, StatId);
						StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].StatLabels[StatIdx].SetDataStoreBinding(LabelString);
						StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].StatLabels[StatIdx].SetVisibility(true);
					}
				}
				// Set widgets for bots
				else if (BotIndex != INDEX_NONE &&
						 PlayersList != none &&
						 PlayersList.LastMatchBots.Length > BotIndex)
				{
					// Set the bot's name
					PlayerButton = StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].ParentButton;
					SetLabelButtonBGStyle(PlayerButton, PlayerButtonStyleName);
					LabelString = PlayersList.LastMatchBots[BotIndex].Name;
					PlayerButton.SetDataStoreBinding(LabelString);
				}
			}
		}
	}
}

/** Sets the data in the leaderboard widgets */
final function SetLeaderboardData()
{
	local UILabelButton PlayerButton;
	local Name PlayerButtonStyleName;
	local String LabelString;
	local UniqueNetId PlayerNetId;
	local UniqueNetId ZeroId;
	local int PlayerIdx;

	// Set the data for the players
	for (PlayerIdx = 0; PlayerIdx < LeaderData.PlayerDataList.length; PlayerIdx++)
	{
		// Grab a reference to the player's netId
		PlayerNetId = LeaderData.PlayerDataList[PlayerIdx].PlayerNetId;

		// Only set widgets for valid netIds
		if (PlayerNetId != ZeroId)
		{
			// Player main button (parent)
			PlayerButton = LeaderData.PlayerDataList[PlayerIdx].ParentButton;
			// If this is a quitting player, change the background style
			if (PlayerNetId != ZeroId && LeaderData.PlayerDataList[PlayerIdx].bIsQuitter)
			{
				PlayerButtonStyleName = 'img_Scoreboard_ErrorRow';
			}
			else if (LeaderData.PlayerDataList[PlayerIdx].RaceType == eGEARRACE_COG)
			{
				PlayerButtonStyleName = 'img_Scoreboard_CogRow';
			}
			else if (LeaderData.PlayerDataList[PlayerIdx].RaceType == eGEARRACE_LOCUST)
			{
				PlayerButtonStyleName = 'img_Scoreboard_LocRow';
			}
			else
			{
				PlayerButtonStyleName = 'img_Scoreboard_NeutralRow';
			}
			SetLabelButtonBGStyle(PlayerButton, PlayerButtonStyleName);
			LabelString = GetPlayerNameFromNetId(PlayerNetId);
			PlayerButton.SetDataStoreBinding(LabelString);

			// Rank (skill) label
			LabelString = GetPlayerRankFromNetId(PlayerNetId);
			LeaderData.PlayerDataList[PlayerIdx].RankLabel.SetDataStoreBinding(LabelString);
			LeaderData.PlayerDataList[PlayerIdx].RankLabel.SetVisibility(true);

			// Score label
			LabelString = GetPlayerLifetimeScoreFromNetId(PlayerNetId);
			LeaderData.PlayerDataList[PlayerIdx].ScoreLabel.SetDataStoreBinding(LabelString);
			LeaderData.PlayerDataList[PlayerIdx].ScoreLabel.SetVisibility(true);
		}
	}
}

/** Whether the team has any valid players on it or not */
final function bool StatTeamHasValidPlayer(int TeamIdx)
{
	local UniqueNetId ZeroId;
	if (StatData.TeamDataList[TeamIdx].PlayerDataList.length > 0 &&
		(StatData.TeamDataList[TeamIdx].PlayerDataList[0].PlayerNetId != ZeroId ||
		 StatData.TeamDataList[TeamIdx].PlayerDataList[0].BotListIndex != INDEX_NONE))
	{
		return true;
	}
	return false;
}

/** Sets the players into UI slots on both the game stats and leaderboard sections */
final function SetPlayersToSlots()
{
	local int TeamDataIndex;
	local int PlayerIndex;
	local array<UniqueNetId> Players;
	local string PawnClassName;
	local UniqueNetId ZeroId;
	local array<int> Bots;
	local int BotIndex;

	if (PlayersList != None)
	{
		// Sort the players by their rank on the leaderboard
		PlayersList.SortPlayersByRank();
		// Update the scoreboard data
		for (TeamDataIndex = 0; TeamDataIndex < StatData.TeamDataList.Length; TeamDataIndex++)
		{
			StatData.TeamDataList[TeamDataIndex].TeamIndex = TeamDataIndex;
			Players.Length = 0;
			// Get the set of players for this team
			PlayersList.GetPlayersForTeam(TeamDataIndex,Players);
			// And add them to the UI elements
			for (PlayerIndex = 0; PlayerIndex < Players.Length; PlayerIndex++)
			{
				StatData.TeamDataList[TeamDataIndex].PlayerDataList[PlayerIndex].PlayerNetId = Players[PlayerIndex];
				StatData.TeamDataList[TeamDataIndex].PlayerDataList[PlayerIndex].bIsQuitter = !PlayersList.PlayerFinishedGame(Players[PlayerIndex]);

				PawnClassName = PlayersList.GetPawnClassNameForPlayer(Players[PlayerIndex]);
				if (InStr(PawnClassName, "COG") != -1)
				{
					StatData.TeamDataList[TeamDataIndex].RaceType = eGEARRACE_COG;
				}
				else if (InStr(PawnClassName, "Locust") != -1)
				{
					StatData.TeamDataList[TeamDataIndex].RaceType = eGEARRACE_LOCUST;
				}
				else
				{
					StatData.TeamDataList[TeamDataIndex].RaceType = eGEARRACE_MAX;
				}
			}
			// First mark all players as NOT being bots
			for (BotIndex = 0; BotIndex < StatData.TeamDataList[TeamDataIndex].PlayerDataList.length; BotIndex++)
			{
				StatData.TeamDataList[TeamDataIndex].PlayerDataList[BotIndex].BotListIndex = INDEX_NONE;
			}
			// Get the set of bots for this team
			PlayersList.GetBotsForTeam(TeamDataIndex,Bots);
			// Now set the bots on teams
			for (BotIndex = 0; BotIndex < Bots.length; BotIndex++)
			{
				StatData.TeamDataList[TeamDataIndex].PlayerDataList[PlayerIndex].BotListIndex = Bots[BotIndex];
				PlayerIndex++;

				PawnClassName = PlayersList.LastMatchBots[Bots[BotIndex]].PawnClassName;
				if (InStr(PawnClassName, "COG") != -1)
				{
					StatData.TeamDataList[TeamDataIndex].RaceType = eGEARRACE_COG;
				}
				else if (InStr(PawnClassName, "Locust") != -1)
				{
					StatData.TeamDataList[TeamDataIndex].RaceType = eGEARRACE_LOCUST;
				}
				else
				{
					StatData.TeamDataList[TeamDataIndex].RaceType = eGEARRACE_MAX;
				}
			}
		}
		// Update the leaderboard data
		for (PlayerIndex = 0; PlayerIndex < PlayersList.LastMatchPlayers.Length; PlayerIndex++)
		{
			LeaderData.PlayerDataList[PlayerIndex].TeamIndex = PlayersList.LastMatchPlayers[PlayerIndex].TeamNum;
			LeaderData.PlayerDataList[PlayerIndex].PlayerNetId = PlayersList.LastMatchPlayers[PlayerIndex].NetId;
			LeaderData.PlayerDataList[PlayerIndex].bIsQuitter = !PlayersList.PlayerFinishedGame(PlayersList.LastMatchPlayers[PlayerIndex].NetId);

			// Set the background color
			PawnClassName = PlayersList.GetPawnClassNameForPlayer(LeaderData.PlayerDataList[PlayerIndex].PlayerNetId);

			if (InStr(PawnClassName, "COG") != -1)
			{
				LeaderData.PlayerDataList[PlayerIndex].RaceType = eGEARRACE_COG;
			}
			else if (InStr(PawnClassName, "Locust") != -1)
			{
				LeaderData.PlayerDataList[PlayerIndex].RaceType = eGEARRACE_LOCUST;
			}
			else
			{
				LeaderData.PlayerDataList[PlayerIndex].RaceType = eGEARRACE_MAX;
			}
		}

		// Disable unused buttons
		FirstScoreButton = none;
		LastScoreButton = none;
		for (TeamDataIndex = 0; TeamDataIndex < StatData.TeamDataList.Length; TeamDataIndex++)
		{
			for (PlayerIndex = 0; PlayerIndex < StatData.TeamDataList[TeamDataIndex].PlayerDataList.length; PlayerIndex++)
			{
				if (StatData.TeamDataList[TeamDataIndex].PlayerDataList[PlayerIndex].PlayerNetId == ZeroId)
				{
					if ( StatData.TeamDataList[TeamDataIndex].PlayerDataList[PlayerIndex].ParentButton.IsFocused(GetPlayerOwnerIndex()) )
					{
						StatData.TeamDataList[TeamDataIndex].PlayerDataList[PlayerIndex].ParentButton.KillFocus(None, GetPlayerOwnerIndex());
					}
					StatData.TeamDataList[TeamDataIndex].PlayerDataList[PlayerIndex].ParentButton.SetEnabled(false);
				}
				else
				{
					LastScoreButton = StatData.TeamDataList[TeamDataIndex].PlayerDataList[PlayerIndex].ParentButton;
					LastScoreButton.SetEnabled(true);
					if (FirstScoreButton == none)
					{
						FirstScoreButton = LastScoreButton;
					}
				}
			}
		}

		// Set the docking
		if (FirstScoreButton != LastScoreButton)
		{
			FirstScoreButton.SetForcedNavigationTarget(UIFACE_Top, LastScoreButton);
			LastScoreButton.SetForcedNavigationTarget(UIFACE_Bottom, FirstScoreButton);
		}

		// Disable unused buttons
		FirstLeaderButton = none;
		LastLeaderButton = none;
		for (PlayerIndex = 0; PlayerIndex < LeaderData.PlayerDataList.length; PlayerIndex++)
		{
			if (LeaderData.PlayerDataList[PlayerIndex].PlayerNetId == ZeroId)
			{
				LeaderData.PlayerDataList[PlayerIndex].ParentButton.SetEnabled(false);
			}
			else
			{
				LastLeaderButton = LeaderData.PlayerDataList[PlayerIndex].ParentButton;
				LastLeaderButton.SetEnabled(true);
				if (FirstLeaderButton == none)
				{
					FirstLeaderButton = LastLeaderButton;
				}
			}
		}

		// Set the docking
		if (FirstLeaderButton != LastLeaderButton)
		{
			FirstLeaderButton.SetForcedNavigationTarget(UIFACE_Top, LastLeaderButton);
			LastLeaderButton.SetForcedNavigationTarget(UIFACE_Bottom, FirstLeaderButton);
		}
	}
}

/** Returns the MP game type being played - overloaded from base class since we have to grab it in a different way */
function EGearMPTypes GetMPGameType()
{
	if (PlayersList != None)
	{
		return PlayersList.LastMpGameType;
	}
	return eGEARMP_Warzone;
}

/** Returns the MP map name that was played */
function String GetMPMapName()
{
	if (PlayersList != None)
	{
		return PlayersList.LastMpMapName;
	}
	return "mp_gridlock";
}

/** Returns the team name for a wingman match based on the team index */
final function String GetWingmanTeamName(int TeamIdx)
{
	local string TeamName;
	// Grab the first player on this team and return his character name
	if (PlayersList != None && PlayersList.LastMatchResults != None)
	{
		TeamName = PlayersList.GetTeamName(TeamIdx);
		return Caps(TeamName);
	}
	return "";
}

/** Returns the team score (going to use STATS_COLUMN_ROUNDSWON from the first player on the team */
final function String GetTeamScoreString(int TeamIdx)
{
	if (PlayersList != None && PlayersList.LastMatchResults != None)
	{
		return string(PlayersList.GetScoreForTeam(TeamIdx));
	}
	return "0";
}

/** Returns the team score for Horde */
final function String GetHordeTeamScore()
{
	if (PlayersList != None && PlayersList.LastMatchResults != None)
	{
		return string(PlayersList.GetScoreForTeam(0));
	}
	return "0";
}

/** Returns the string of the name of the player using the netId as the key */
final function String GetPlayerNameFromNetId(UniqueNetId PlayerNetId)
{
	if (PlayersList != None && PlayersList.LastMatchResults != None)
	{
		return PlayersList.GetNameForPlayer(PlayerNetId);
	}
	return "RobRooster";
}

/** Returns the string of the rank of the player using the netId as the key */
final function String GetPlayerRankFromNetId(UniqueNetId PlayerNetId)
{
	if (PlayersList != None && PlayersList.LastMatchResults != None)
	{
		return GetPlayerSkillString(PlayersList.GetSkillForPlayer(PlayerNetId));
	}
	return "D";
}

/** Returns the string of the score of the player using the netId as the key */
final function String GetPlayerScoreFromNetId(UniqueNetId PlayerNetId)
{
	local int Value;

	if (PlayersList != None && PlayersList.LastMatchResults != None)
	{
		// Get the STATS_COLUMN_POINTS from the stats code
		if (PlayersList.LastMatchResults.GetIntStatValueForPlayer(PlayerNetId,STATS_COLUMN_POINTS,Value))
		{
			return string(Value);
		}
	}
	return "0";
}

/** Returns the string of the lifetime score of the player using the netId as the key */
final function String GetPlayerLifetimeScoreFromNetId(UniqueNetId PlayerNetId)
{
	local int Value;

	if (PlayersList != None && PlayersList.AllTimeMatchResults != None)
	{
		// Get the STATS_COLUMN_POINTS from the stats code
		if (PlayersList.AllTimeMatchResults.GetIntStatValueForPlayer(PlayerNetId,STATS_COLUMN_POINTS,Value))
		{
			return string(Value);
		}
	}
	return "0";
}

/**
 * Returns the string of the stat, using the netId as a key
 *
 * @param PlayerNetId - UniqueNetId of the player we're getting the stat for
 * @param StatId - const value from GearOnlineConstants for this stat "STAT_COLUMN_"
 */
final function String GetPlayerStatFromNetId(UniqueNetId PlayerNetId, int StatId)
{
	local int Value;

	if (PlayersList != None && PlayersList.LastMatchResults != None)
	{
		// Get the stat column from the stats code
		if (PlayersList.LastMatchResults.GetIntStatValueForPlayer(PlayerNetId,StatId,Value))
		{
			return string(Value);
		}
	}
	return "0";
}

/** Returns the network Id of the player who's currently focused */
final function UniqueNetId GetNetIdOfFocusedPlayer()
{
	local UniqueNetId NetworkId;
	local int TeamIdx;
	local int PlayerIdx;
	local class<UIState> StateClass;

	if (GenericData.PostGameMode == eGPGM_GameStats)
	{
		for (TeamIdx = 0; TeamIdx < StatData.TeamDataList.length; TeamIdx++)
		{
			for (PlayerIdx = 0; PlayerIdx < StatData.TeamDataList[TeamIdx].PlayerDataList.length; PlayerIdx++)
			{
				StateClass = StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].ParentButton.GetCurrentState(GetBestPlayerIndex()).Class;
				if (StateClass == class'UIState_Focused' || StateClass == class'UIState_Pressed')
				{
					return StatData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].PlayerNetId;
				}
			}
		}
	}
	else
	{
		for (PlayerIdx = 0; PlayerIdx < LeaderData.PlayerDataList.length; PlayerIdx++)
		{
			StateClass = LeaderData.PlayerDataList[PlayerIdx].ParentButton.GetCurrentState(GetBestPlayerIndex()).Class;
			if (StateClass == class'UIState_Focused' || StateClass == class'UIState_Pressed')
			{
				return LeaderData.PlayerDataList[PlayerIdx].PlayerNetId;
			}
		}
	}
	return NetworkId;
}

/**
 * Opens the player stat screen for the currently focused player
 *
 * The difference between this delegate and the OnPressRelease delegate is that OnClick will only be called on the
 * widget that received the matching key press. OnPressRelease will be called on whichever widget was under the cursor
 * when the key was released, which might not necessarily be the widget that received the key press.
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool OnPlayerStatPress(UIScreenObject EventObject, int PlayerIndex)
{
	local UIScene FoundStatScene;
	local GameUISceneClient GameSceneClient;
	local UniqueNetId ZeroId;

	// Make sure they have a valid NetId
	DetailStatsPlayerId = GetNetIdOfFocusedPlayer();
	if (DetailStatsPlayerId != ZeroId)
	{
		// Open the player stats screen
		GameSceneClient = GetSceneClient();
		if (GameSceneClient != None)
		{
			FoundStatScene = UIScene(FindObject("UI_Scenes_PostGame.PostPlayerStats", class'UIScene'));
			if (FoundStatScene != None)
			{
				OpenScene(FoundStatScene, GetPlayerOwner(PlayerIndex),,, StatSceneActivationComplete);
			}
		}
	}
	return true;
}

/** Called when the player stats scene pops up */
function StatSceneActivationComplete( UIScene ActivatedScene, bool bInitialActivation )
{
	GearUISceneFE_PostGamePlayer(ActivatedScene).PlayerNetworkId = DetailStatsPlayerId;
	GearUISceneFE_PostGamePlayer(ActivatedScene).GameType = GenericData.GameType;
	GearUISceneFE_PostGamePlayer(ActivatedScene).SceneActivationComplete(ActivatedScene, bInitialActivation);
}

/**
 * Opens the gamecard screen for the currently focused player
 *
 * The difference between this delegate and the OnPressRelease delegate is that OnClick will only be called on the
 * widget that received the matching key press. OnPressRelease will be called on whichever widget was under the cursor
 * when the key was released, which might not necessarily be the widget that received the key press.
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool OnGamercardPress(UIScreenObject EventObject, int PlayerIndex)
{
	local UniqueNetId NetworkId;
	local UniqueNetId ZeroId;
	local OnlinePlayerInterfaceEx PlayerIntEx;
	local int ControllerId;

	NetworkId = GetNetIdOfFocusedPlayer();
	if (NetworkId != ZeroId)
	{
		if (OnlineSub != None)
		{
			PlayerIntEx = OnlineSub.PlayerInterfaceEx;
			if (PlayerIntEx != None)
			{
				ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
				PlayerIntEx.ShowGamerCardUI(ControllerId, NetworkId);
			}
		}
	}
	return true;
}

/**
 * Called when the widget is no longer being pressed.  Not implemented by all widget types.
 *
 * The difference between this delegate and the OnPressRelease delegate is that OnClick will only be called on the
 * widget that received the matching key press. OnPressRelease will be called on whichever widget was under the cursor
 * when the key was released, which might not necessarily be the widget that received the key press.
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool OnBackPress(UIScreenObject EventObject, int PlayerIndex)
{
	CloseScene(self);
	return true;
}

/**
 * Sends an invite to all players in the opposing party so that you can have a rematch
 *
 * @param EventObject Object that issued the event.
 * @param PlayerIndex Player that performed the action that issued the event.
 *
 * @return TRUE to prevent the kismet OnClick event from firing
 */
function bool OnPartyUpPress(UIScreenObject EventObject, int PlayerIndex)
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	ButtonAliases.AddItem('GenericCancel');
	ButtonAliases.AddItem('PartyUpYes');
	GameSceneClient = GetSceneClient();
	GameSceneClient.ShowUIMessage(
		'ConfirmNetworkedPlayersKicked',
		"<Strings:GearGameUI.MessageBoxStrings.PartyUp_Title>",
		"<Strings:GearGameUI.MessageBoxStrings.PartyUp_Message>",
		"<Strings:GearGameUI.MessageBoxStrings.PartyUp_Question>",
		ButtonAliases,
		OnPartyUp_Confirm,
		GetPlayerOwner(GetBestPlayerIndex()) );

	return true;
}

/** Callback from asking whether they really want to invite the whole match to their party */
function bool OnPartyUp_Confirm( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	local OnlinePlayerInterface PlayerInt;
	local int ControllerId;
	local array<UniqueNetId> NetIdList;
	local GearPC PC;
	local int PriIndex;
	local string AlertText;

	if (SelectedInputAlias == 'PartyUpYes')
	{
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if (OnlineSub != None)
		{
			PlayerInt = OnlineSub.PlayerInterface;
			if (PlayerInt != None)
			{
				PC = GetGearPlayerOwner();
				// Grab all the players
				PlayersList.GetPlayersForStatsRead(NetIdList);
				// Remove any players already in the session
				for (PriIndex = 0; PriIndex < PC.WorldInfo.GRI.PRIArray.Length; PriIndex++)
				{
					NetIdList.RemoveItem(PC.WorldInfo.GRI.PRIArray[PriIndex].UniqueId);
				}
				// Only send if there were people left
				if (NetIdList.Length > 0)
				{
					ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
					PlayerInt.SendGameInviteToFriends(ControllerId,NetIdList);

					// Pop up an alert regarding the invite.
					AlertText = Localize("AlertStrings", "PartyReInvited", "GearGameUI");
					AlertText = Repl(AlertText, "\`NUMFRIENDS\`", string(NetIdList.length));
					PC.AlertManager.Alert(eALERT_FriendAlert, Localize("AlertStrings", "InviteSentTitle", "GearGameUI"), AlertText);
				}
			}
		}
	}

	return true;
}

/**
 * Called when a new UIState becomes the widget's currently active state, after all activation logic has occurred.
 *
 * @param	Sender					the widget that changed states.
 * @param	PlayerIndex				the index [into the GamePlayers array] for the player that activated this state.
 * @param	NewlyActiveState		the state that is now active
 * @param	PreviouslyActiveState	the state that used the be the widget's currently active state.
 */
function OnStateChanged( UIScreenObject Sender, int PlayerIndex, UIState NewlyActiveState, optional UIState PreviouslyActiveState )
{
	// If a new label button has become focused we need to refresh the button bar to reflect what functionality they have access to
	if (UIState_Focused(NewlyActiveState) != None &&
		UILabelButton(Sender) != None)
	{
		UpdateButtonBar();
	}
}

/**
 * Left or right trigger was pressed so we must move to the next part of the post game scene
 * @param bNavigateRight - whether we are moving right or left (used for a possible movement animation in the future)
 */
final function NavigatePostGame(bool bNavigateRight)
{
	if (GenericData.PostGameMode == eGPGM_GameStats)
	{
		SetPostGameMode(eGPGM_Leaderboard);
	}
	else
	{
		SetPostGameMode(eGPGM_GameStats);
	}

	PlayUISound(bNavigateRight ? 'NavigateRight' : 'NavigateLeft');
}

/**
 * Callback function when the scene gets input
 * @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
 */
function bool ProcessInput( out InputEventParameters EventParms )
{
	if (EventParms.InputKeyName == 'XboxTypeS_LeftTrigger' ||
		EventParms.InputKeyName == 'XboxTypeS_RightTrigger')
	{
		if (EventParms.EventType == IE_Pressed)
		{
			NavigatePostGame(EventParms.InputKeyName == 'XboxTypeS_RightTrigger');
		}
		return true;
	}
	return false;
}


defaultproperties
{
	OnRawInputKey=ProcessInput

	GameSpecificData(eGEARMP_Warzone)=(StatOrder=(eGSStat_Kills,eGSStat_Downs,eGSStat_Revives,eGSStat_Deaths))
	GameSpecificData(eGEARMP_Execution)=(StatOrder=(eGSStat_Kills,eGSStat_Downs,eGSStat_Revives,eGSStat_Deaths))
	GameSpecificData(eGEARMP_KTL)=(StatOrder=(eGSStat_Kills,eGSStat_Downs,eGSStat_Revives,eGSStat_Deaths))
	GameSpecificData(eGEARMP_CombatTrials)=(StatOrder=(eGSStat_Kills,eGSStat_Downs,eGSStat_Revives,eGSStat_Deaths))
	GameSpecificData(eGEARMP_Annex)=(StatOrder=(eGSStat_AnnexCap,eGSStat_AnnexBreak,eGSStat_Kills,eGSStat_Downs))
	GameSpecificData(eGEARMP_Wingman)=(StatOrder=(eGSStat_Kills,eGSStat_Downs,eGSStat_Revives,eGSStat_Deaths))
	GameSpecificData(eGEARMP_KOTH)=(StatOrder=(eGSStat_AnnexCap,eGSStat_AnnexBreak,eGSStat_Kills,eGSStat_Downs))
	GameSpecificData(eGEARMP_CTM)=(StatOrder=(eGSStat_MeatflagCap,eGSStat_Kills,eGSStat_Downs,eGSStat_Revives))

	StatHeaderStyles(eGSStat_Deaths)="img_ScoreboardIcons_Deaths"
	StatHeaderStyles(eGSStat_Downs)="img_ScoreboardIcons_Downs"
	StatHeaderStyles(eGSStat_Kills)="img_ScoreboardIcons_Kills"
	StatHeaderStyles(eGSStat_Revives)="img_ScoreboardIcons_Revives"
	StatHeaderStyles(eGSStat_AnnexCap)="Scoreboard_AnnexCog"
	StatHeaderStyles(eGSStat_AnnexBreak)="Scoreboard_AnnexCogBreak"
	StatHeaderStyles(eGSStat_MeatflagCap)="Scoreboard_MeatFlagCap"

	StatConstants(eGSStat_Deaths)=STATS_COLUMN_DEATHS
	StatConstants(eGSStat_Downs)=STATS_COLUMN_TAKEDOWNS
	StatConstants(eGSStat_Kills)=STATS_COLUMN_KILLS
	StatConstants(eGSStat_Revives)=STATS_COLUMN_REVIVES
	StatConstants(eGSStat_AnnexCap)=STATS_COLUMN_ANNEX_CAPTURES
	StatConstants(eGSStat_AnnexBreak)=STATS_COLUMN_ANNEX_BREAKS
	StatConstants(eGSStat_MeatflagCap)=STATS_COLUMN_MEATFLAG_MEATFLAGCAPTURES
}

