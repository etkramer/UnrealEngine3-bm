/**
 * GearUIScene_Scoreboard
 * Class that handles the initialization and updating of labels for all scoreboards in the game.
 * All widget names are generically used so that this code can be used on all scoreboards, regardless of how many
 * teams and players there are.
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearUIScene_Scoreboard extends GearUISceneMP_Base;

/************************************************************************/
/* Constants, enums, structs, etc.										*/
/************************************************************************/

/** Struct to store all of the data needed for a player */
struct GearScoreboardPlayerData
{
	/** The PlayerReplication data for this player */
	var transient GearPRI PlayerPRI;

	//------ Widget references ---------
	/** The parent panel of all widgets for a player */
	var transient UIPanel ParentPanel;
	/** Image for drawing a strike through the player if he's dead */
	var transient UIImage StrikeImage;
	/** Label for the player's name */
	var transient UILabel NameLabel;
	/** Label for the rank of this player */
	var transient UILabel RankLabel;
	/** Image for the status of this player */
	var transient UIIMage StatusImage;
	/** Label for the total score for this player */
	var transient UILabel ScoreLabel;
	/** Image for the chat icons */
	var transient UIImage ChatIcon;
	/** Labels for the stats (from left to right) */
	var transient array<UILabel> StatLabels;
};

/** Struct to store all of the data needed for a team */
struct GearScoreboardTeamData
{
	/** Array of data for each player on the team */
	var transient array<GearScoreboardPlayerData> PlayerDataList;
	/** Whether this is the local player's team or not */
	var transient bool bIsLocalPlayerTeam;
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

	/** Images for describing the stat columns 0-3 (left to right) */
	var transient array<UIImage> StatImages;
};

/** Struct to store all of the data needed for this game */
struct GearScoreboardGameData
{
	/** The GameReplication data for this game */
	var transient GearGRI MyGRI;

	/** The type of scoreboard this is */
	var transient EGearMPTypes GameType;

	/** Array of data for each team in the game */
	var transient array<GearScoreboardTeamData> TeamDataList;

	//------ Widget references ---------
	/** Label for the match time */
	var transient UILabel MatchTimeLabel;
	/** Label for game specific rule */
	var transient UILabel GameRuleLabel;
	/** Image for game specific rule */
	var transient UIImage GameRuleImage;
	/** Label for the goal score */
	var transient UILabel GoalScoreLabel;
	/** Label for the game/mapname */
	var transient UILabel GameAndMapLabel;
	/** Panel at the bottom of the screen */
	var transient UIPanel FooterPanel;
	/** Label for showing a game message in teh footer */
	var transient UILabel FooterMessageLabel;
};

/** Struct to store game-type specific variables, delegates, etc. */
struct GearScoreboardGameTypeSpecificData
{
	/** Variable name in the localization file for the name of this game-type */
	var transient const string GameNameVariableString;

	/** Delegate for updating the game-specific stats for a player */
	var transient const delegate<UpdatePlayerStats> UpdatePlayerStatsDelegate;

	/** Stat order */
	var transient const array<GearScoreboardStatType> StatOrder;
};


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** The data for the game this scoreboard is displaying */
var transient GearScoreboardGameData ScoreGameData;

/** Whether the scoreboard was initialized or not */
var transient bool bScoreboardIsInitialized;

/** The last value of the GearGRI's RoundTime */
var transient int LastCountDownTime;

var	transient	float	TimeSinceLastUpdate;

/** Strings to show in the footer */
var localized string WaitingForPlayers;
var localized string WaitingForHost;
var localized string PressStart;

/**
 * List of data that allows for game-specific customization of the scoreboard
 * This list must match the EGearMPTypes in GearTypes.uc
 */
var transient array<GearScoreboardGameTypeSpecificData> GameSpecificData;

/** Styles for the header of each stat in the game */
var const array<String> StatHeaderStyles;


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Delegate called when a player's stats need updating */
delegate UpdatePlayerStats( int PlayerIdx, int TeamIdx, bool bPlayerExists, bool bPlayerEnabled, Name FontStyleName );

/**
 * Called after this screen object's children have been initialized
 * Overloaded to initialized the references to this scene's widgets
 */
event PostInitialize()
{
	// Set invisible (needed so that the scene is not seen until the GRI is ready)
	SetVisibility( false );

	InitializeScoreboard();

	Super.PostInitialize();
}

/** Initialize the scoreboard - widget references etc. */
final function bool InitializeScoreboard()
{
	local GearGRI GRI;

	GRI = GearGRI(GetWorldInfo().GRI);

	// Have to make sure this is the CORRECT GRI since seamless travel can carry GRIs across levels
	if (GRI != None &&
		GearPreGameGRI(GRI) == none)
	{
		ScoreGameData.MyGRI = GRI;
		bScoreboardIsInitialized = true;

		// Determine the type of game this is (this will have impact on how many widget references will be initialized)
		ScoreGameData.GameType = GetMPGameType();

		// Initialized the widget references so we can update them later
		InitializeWidgetReferences();

		return true;
	}

	return false;
}

/** Initializes the widget references for the UI scene */
final function InitializeWidgetReferences()
{
	local int NumPlayers, TeamIdx, PlayerIdx, StatIdx, ChatCount;
	local string LabelName;
	local UIPanel TeamPanel, PlayerPanel;
	local bool bIsMeatflag;

	//-------- Game Description Widgets -----------------------------------------

	// Find the widget references we need for the game description widgets
	ScoreGameData.GameAndMapLabel =		UILabel(FindChild('lblGameMapName', true));
	ScoreGameData.MatchTimeLabel =		UILabel(FindChild('lblMatchTimer', true));
	ScoreGameData.GoalScoreLabel =		UILabel(FindChild('lblScoreLimit', true));
	ScoreGameData.GameRuleLabel =		UILabel(FindChild('lblPlayerCount', true));
	ScoreGameData.GameRuleImage =		UIImage(FindChild('imgPlayerCount', true));
	ScoreGameData.FooterPanel =			UIPanel(FindChild('pnlFooter', true));
	ScoreGameData.FooterMessageLabel =	UILabel(ScoreGameData.FooterPanel.FindChild('lblMessage', true));


	//-------- Team Widgets -----------------------------------------------------

	// Set the size of the TeamDataList for the GameData and get the number of players per team
	if ( ScoreGameData.GameType == eGEARMP_Wingman )
	{
		ScoreGameData.TeamDataList.length = 5;
		NumPlayers = 2;
	}
	else if ( ScoreGameData.GameType == eGEARMP_CTM )
	{
		ScoreGameData.TeamDataList.length = 3;
		// initialize the team indices here as they are always in the same position and always displayed for this gametype
		ScoreGameData.TeamDataList[0].TeamIndex = 0;
		ScoreGameData.TeamDataList[1].TeamIndex = 1;
		NumPlayers = 5;
	}
	else
	{
		ScoreGameData.TeamDataList.length = 2;
		// initialize the team indices here as they are always in the same position and always displayed for this gametype
		ScoreGameData.TeamDataList[0].TeamIndex = 0;
		ScoreGameData.TeamDataList[1].TeamIndex = 1;
		NumPlayers = 5;
	}

	// Keeps track of which chat icons we've grabbed references for
	ChatCount = 0;

	// Loop through all of the teams and initialize the widgets
	for ( TeamIdx = 0; TeamIdx < ScoreGameData.TeamDataList.length; TeamIdx++ )
	{
		// Construct the name of the team panel string and find the object in the scene
		LabelName = "pnlTeam" $ TeamIdx $ "Scores";
		ScoreGameData.TeamDataList[TeamIdx].ParentPanel = UIPanel(FindChild( Name(LabelName), true ));
		TeamPanel = ScoreGameData.TeamDataList[TeamIdx].ParentPanel;

		// If we could not find the team ParentPanel we can't look for anything else
		if ( TeamPanel != None )
		{
			// Find the widget references we need from the team ParentPanel
			ScoreGameData.TeamDataList[TeamIdx].RaceImage =		UIImage(TeamPanel.FindChild('imgTeamIcon', true));
			ScoreGameData.TeamDataList[TeamIdx].ScoreLabel =	UILabel(TeamPanel.FindChild('lblTeamScore', true));
			ScoreGameData.TeamDataList[TeamIdx].ScoreLabel2 =	UILabel(TeamPanel.FindChild('lblTeamScore2', true));
			if ( ScoreGameData.TeamDataList.length > 2 )
			{
				ScoreGameData.TeamDataList[TeamIdx].TeamNameLabel =	UILabel(TeamPanel.FindChild('lblTeamName', true));
			}

			// Find the widget references for the stats (if this is wingman or horde only the first team will have them)
			if ( (TeamIdx == 0) || (ScoreGameData.GameType != eGEARMP_Wingman && ScoreGameData.GameType != eGEARMP_CombatTrials) )
			{
				for ( StatIdx = 0; StatIdx < GameSpecificData[ScoreGameData.GameType].StatOrder.length; StatIdx++ )
				{
					LabelName = "imgStatHeader" $ StatIdx;
					ScoreGameData.TeamDataList[TeamIdx].StatImages[StatIdx] = UIImage(TeamPanel.FindChild( Name(LabelName), true ));
				}
			}

			// See if this is the meatflag section
			bIsMeatflag = (ScoreGameData.GameType == eGEARMP_CTM && TeamIdx == 2);

			// Set the size of the PlayerDataList for this team
			// If this is the meatflag we have to adjust the player count
			if ( bIsMeatflag )
			{
				ScoreGameData.TeamDataList[TeamIdx].PlayerDataList.length = 1;
			}
			else
			{
				ScoreGameData.TeamDataList[TeamIdx].PlayerDataList.length = NumPlayers;
			}

			//-------- Player Widgets -------------------------------------------------

			// Loop through all of the players on this team and initialize the widgets
			for ( PlayerIdx = 0; PlayerIdx < ScoreGameData.TeamDataList[TeamIdx].PlayerDataList.length; PlayerIdx++ )
			{
				// Construct the name of the player panel string and find the object in the team ParentPanel
				LabelName = "pnlPlayer" $ PlayerIdx;
				ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].ParentPanel = UIPanel(ScoreGameData.TeamDataList[TeamIdx].ParentPanel.FindChild( Name(LabelName), true ));
				PlayerPanel = ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].ParentPanel;

				// If we could not find the player ParentPanel we can't look for anything else
				if ( PlayerPanel != None )
				{
					// Find the widget references we need from the player ParentPanel
					ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].StrikeImage = UIImage(PlayerPanel.FindChild('imgStrike', true));
					ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].NameLabel =	UILabel(PlayerPanel.FindChild('lblName', true));
					ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].RankLabel =	UILabel(PlayerPanel.FindChild('lblRank', true));
					ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].StatusImage =	UIImage(PlayerPanel.FindChild('imgStatus', true));
					ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].ScoreLabel =	UILabel(PlayerPanel.FindChild('lblTotal', true));
					LabelName = "imgChat" $ ChatCount;
					ChatCount++;
					ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].ChatIcon =	UIImage(FindChild(Name(LabelName), true));
					ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].ChatIcon.Opacity = 0.0f;

					// The detailed stats will not be available on game-types with more than 2 teams, unless
					// it's the team of the player that opened the scoreboard
					if ( (TeamIdx == 0) || (ScoreGameData.GameType != eGEARMP_Wingman && ScoreGameData.GameType != eGEARMP_CombatTrials) )
					{
						if ( bIsMeatflag )
						{
							ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].StatLabels.length = 2;
						}
						else
						{
							ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].StatLabels.length = GameSpecificData[ScoreGameData.GameType].StatOrder.length;
						}

						for ( StatIdx = 0; StatIdx < ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].StatLabels.length; StatIdx++ )
						{
							LabelName = "lblStat" $ StatIdx;
							ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].StatLabels[StatIdx] = UILabel(PlayerPanel.FindChild( Name(LabelName), true ));
						}
					}
				}
			}
		}
	}
}

/** Sets the styles for the stat headers */
final function SetGameSpecificStatHeaderStyles( int TeamIdx )
{
	local String StyleString;
	local int StatIdx;

	// If this is the meatflag do nothing
	if ( ScoreGameData.GameType == eGEARMP_CTM && TeamIdx == 2 )
	{
		return;
	}

	// If this is wingman or horde only the first team will have them
	if ( (TeamIdx == 0) || (ScoreGameData.GameType != eGEARMP_Wingman && ScoreGameData.GameType != eGEARMP_CombatTrials) )
	{
		for ( StatIdx = 0; StatIdx < GameSpecificData[ScoreGameData.GameType].StatOrder.length; StatIdx++ )
		{
			StyleString = StatHeaderStyles[GameSpecificData[ScoreGameData.GameType].StatOrder[StatIdx]];
			StyleString = Repl( StyleString, "<TEAM>", (GetPRIRace(FindPRIOnTeam(TeamIdx, ScoreGameData.MyGRI)) == eGEARRACE_Locust) ? "Locust" : "Cog" );
			SetImageStyle(ScoreGameData.TeamDataList[TeamIdx].StatImages[StatIdx], Name(StyleString) );
		}
	}
}

/** Returns the game specific stat for a given PRI */
final function string GetGameSpecificPlayerStat( GearPRI PlayerPRI, int StatIdx )
{
	if ( (PlayerPRI != None) && (StatIdx < GameSpecificData[ScoreGameData.GameType].StatOrder.length) )
	{
		switch ( GameSpecificData[ScoreGameData.GameType].StatOrder[StatIdx] )
		{
		case eGSStat_Deaths:			return string(PlayerPRI.GetDeaths());
		case eGSStat_Downs:				return string(PlayerPRI.GetTakeDowns());
		case eGSStat_Kills:				return string(PlayerPRI.GetKills());
		case eGSStat_Revives:			return string(PlayerPRI.GetRevives());
		case eGSStat_AnnexCap:			return string(PlayerPRI.GetGameScore1());
		case eGSStat_AnnexBreak:		return string(PlayerPRI.GetGameScore2());
		case eGSStat_MeatflagCap:		return string(PlayerPRI.GetGameScore1());
		}
	}

	return "";
}

/**
 * Sorts the teams and players on the scoreboard
 * @Return - whether we were able to sort the teams/players or not
 */
final function bool SortTeamsAndPlayers()
{
 	local GearPC MyGearPC;
 	local GearPRI MyGearPRI, CurrGearPRI;
	local int PRIArrayIdx, FirstTeamIdx, PlayerIdx, CurrTeamIdx;
	local array<GearPRI> PRIList;
	local bool bIsMeatflag;

	// Grab a reference to the local player's PRI
	if ( (PlayerOwner != None) && (PlayerOwner.Actor != None) )
	{
		MyGearPC = GearPC(PlayerOwner.Actor);
		if ( (MyGearPC != None) && (MyGearPC.PlayerReplicationInfo != None) )
		{
			MyGearPRI = GearPRI(MyGearPC.PlayerReplicationInfo);
		}
	}

	// Start with fresh PRI references
	ClearPlayerPRIs();

	// If the PRI is None then bail
	if ( MyGearPRI == None )
	{
		return false;
	}
	// Set the index of the first team in case this is a scoreboard with more than 2 teams.
	// This situation forces us to move the local player's team to the first position and to
	// shift all of the other teams up.
	else
	{
		if ( ScoreGameData.TeamDataList.length > 2 && ScoreGameData.GameType != eGEARMP_CTM )
		{
			if ( IsPRIOnValidTeam(MyGearPRI) )
			{
				FirstTeamIdx = MyGearPRI.Team.TeamIndex;
			}
			else
			{
				FirstTeamIdx = 0;
			}
		}
	}

	// Sort the players so that the one's on top have the highest scores
	SortPlayerPRIs( PRIList );

	// Loop through all of the PRIs
	for ( PRIArrayIdx = 0; PRIArrayIdx < PRIList.length; PRIArrayIdx++ )
	{
		// Grab a reference to the current PRI for code readability
		CurrGearPRI = PRIList[PRIArrayIdx];
		// skip if it's a spectator
		if (!CurrGearPRI.bOnlySpectator)
		{
			// See if this is the meatflag
			bIsMeatflag = IsMeatflagPRI( CurrGearPRI );

			// Force the meatflag on team 2
			if ( bIsMeatflag )
			{
				CurrTeamIdx = 2;
			}
			// This index in the TeamData array to put these players - takes into consideration the
			// fact that we force the local player team to team 0 on game-types with more than 2 teams
			else if ( FirstTeamIdx == 0 )
			{
				CurrTeamIdx = CurrGearPRI.Team.TeamIndex;
			}
			// There are more than 2 teams and the local player is not on team 0
			else
			{
				// Is on the local player's team
				if ( CurrGearPRI.Team.TeamIndex == FirstTeamIdx )
				{
					CurrTeamIdx = 0;
				}
				// Not on local player's team and is on a team with an index less than the local player's
				// so we need to move their index up one
				else if ( CurrGearPRI.Team.TeamIndex < FirstTeamIdx )
				{
					CurrTeamIdx = CurrGearPRI.Team.TeamIndex + 1;
				}
				// Not on local player's team and is on a team with an index greate than the local player's
				// so we just leave the index alone
				else
				{
					CurrTeamIdx = CurrGearPRI.Team.TeamIndex;
				}
			}

			// Find a team to place the PRI on and determine if it's the local player or not
			for ( PlayerIdx = 0; PlayerIdx < ScoreGameData.TeamDataList[CurrTeamIdx].PlayerDataList.length; PlayerIdx++ )
			{
				if ( ScoreGameData.TeamDataList[CurrTeamIdx].PlayerDataList[PlayerIdx].PlayerPRI == None )
				{
					// Set the PRI on the team
					ScoreGameData.TeamDataList[CurrTeamIdx].PlayerDataList[PlayerIdx].PlayerPRI = CurrGearPRI;
					ScoreGameData.TeamDataList[CurrTeamIdx].TeamIndex = CurrGearPRI.GetTeamNum();

					// See if this is the local player
					if ( CurrGearPRI == MyGearPRI )
					{
						// Set the local player team flag
						ScoreGameData.TeamDataList[CurrTeamIdx].bIsLocalPlayerTeam = true;
					}

					break;
				}
			}
		}
	}

	return true;
}

/** Whether this PRI is the meatflag or not */
final function bool IsMeatflagPRI( GearPRI PRI )
{
	return (PRI.bIsMeatflag);
}

/** Sort the PRIs in the GRI's PRIArray */
final function SortPlayerPRIs( out array<GearPRI> PRIList )
{
	local int PRIArrayIdx, PRIListIdx;
	local GearPRI CurrPRI;

	PRIList.Length = 0;
	if ( ScoreGameData.MyGRI != None )
	{
		for ( PRIArrayIdx = 0; PRIArrayIdx < ScoreGameData.MyGRI.PRIArray.length; PRIArrayIdx++ )
		{
			// Get the PRI
			if ( ScoreGameData.MyGRI.PRIArray[PRIArrayIdx] != None )
			{
				CurrPRI = GearPRI(ScoreGameData.MyGRI.PRIArray[PRIArrayIdx]);
			}

			// Make sure it's valid and on a team
			if ( IsValidPRI(CurrPRI) && (IsPRIOnValidTeam(CurrPRI) || IsMeatflagPRI(CurrPRI)) )
			{
				// Insert the PRI in the proper place in the list - lower index is higher PRI value
				for ( PRIListIdx = 0; PRIListIdx < PRIList.length; PRIListIdx++ )
				{
					if ( IsGreaterThanPRI(CurrPRI, PRIList[PRIListIdx]) )
					{
						break;
					}
				}
				PRIList.InsertItem(PRIListIdx, CurrPRI);
			}
		}
	}

	if ( GetGearPlayerOwner().Role == ROLE_Authority || PRIList.length > 1 )
	{
		// Set visible (needed so that the scene is not seen until the GRI is present)
		SetVisibility( true );
	}
}

/** Whether the first PRI should be higher in the sorting list than the second PRI */
final function bool IsGreaterThanPRI( GearPRI FirstPRI, GearPRI SecondPRI )
{
	if ( FirstPRI.GetScore() > SecondPRI.GetScore() )
	{
		return true;
	}

	return false;
}

/** Nulls out all of the PRIs in the scoreboard data */
final function ClearPlayerPRIs()
{
	local int TeamIdx, PlayerIdx;

	for ( TeamIdx = 0; TeamIdx < ScoreGameData.TeamDataList.length; TeamIdx++ )
	{
		// Clear the local player team flag
		ScoreGameData.TeamDataList[TeamIdx].bIsLocalPlayerTeam = false;

		for ( PlayerIdx = 0; PlayerIdx < ScoreGameData.TeamDataList[TeamIdx].PlayerDataList.length; PlayerIdx++ )
		{
			// Null the PRI
			ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].PlayerPRI = None;
		}
	}
}

/** Returns the goal string */
final function string GetGameGoalString()
{
	if ( ScoreGameData.GameType == eGEARMP_CombatTrials )
	{
		return "50";
	}
	else
	{
		return string(ScoreGameData.MyGRI.GoalScore);
	}
}

/** Called from the overloaded Tick() function so we can update the labels */
function UpdateScoreboardLabels(float DeltaTime)
{
	local bool bSkipLabelUpdate;

	// See if we still need to initialized the scoreboard (this might happen if the GRI hasn't replicated yet)
	if ( !bScoreboardIsInitialized && !InitializeScoreboard() )
	{
		return;
	}

	if ( TimeSinceLastUpdate < 0 )
	{
		TimeSinceLastUpdate = DeltaTime;
	}
	else if ( TimeSinceLastUpdate > 1 )
	{
		TimeSinceLastUpdate = 0;
	}
	else
	{
		TimeSinceLastUpdate += DeltaTime;
		bSkipLabelUpdate = true;
	}

	// Get the gametype every frame since we can't determine when the GRI will replicate and when options
	// such as KOTH have been set in the GRI
	ScoreGameData.GameType = GetMPGameType();

	// Update game description widgets
	if ( !bSkipLabelUpdate )
	{
		UpdateGameDescription();

		// Sort the teams and the players on those teams
		if ( SortTeamsAndPlayers() )
		{
			// Update team widgets
			UpdateTeams();
		}
	}

	// Update the chat icons
	UpdateChatIcons();

	// Play a sound for the countdown
	if ( (ScoreGameData.MyGRI != None) && (ScoreGameData.MyGRI.GameStatus == GS_PreMatch || ScoreGameData.MyGRI.GameStatus == GS_RoundOver) && (ScoreGameData.MyGRI.RoundTime != LastCountDownTime) )
	{
		if ( ScoreGameData.MyGRI.RoundTime <= 3 )
		{
			PlayUISound( 'Beep' );
		}
		LastCountDownTime = ScoreGameData.MyGRI.RoundTime;
	}
}

/** Goes through all the chat icons and updates them */
final function UpdateChatIcons()
{
	local int TeamIdx, PlayerIdx, NumTeams;
	local GearPRI CurrPRI;
	local UIImage ChatIcon;
	local name StyleName;

	if ( ScoreGameData.GameType == eGEARMP_CTM )
	{
		NumTeams = 2;
	}
	else
	{
		NumTeams = ScoreGameData.TeamDataList.length;
	}

	for ( TeamIdx = 0; TeamIdx < NumTeams; TeamIdx++ )
	{
		for ( PlayerIdx = 0; PlayerIdx < ScoreGameData.TeamDataList[TeamIdx].PlayerDataList.length; PlayerIdx++ )
		{
			CurrPRI = ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].PlayerPRI;
			ChatIcon = ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].ChatIcon;

			ChatIcon.Opacity = 0.0f;
			if ( CurrPRI != None )
			{
				if ( CurrPRI.ChatFadeValue > 0 )
				{
					CurrPRI.ChatFadeValue = 1.0f - (GetWorldInfo().TimeSeconds - CurrPRI.TaccomChatFadeStart) / CurrPRI.ChatFadeTime;
					ChatIcon.Opacity = CurrPRI.ChatFadeValue;

					StyleName = (GetPRIRace(CurrPRI) == eGEARRACE_COG) ? 'CogChat' : 'LocustChat';
					SetImageStyle(ChatIcon, StyleName);
				}
			}
		}
	}
}

/** Sets the labels for displaying the game description widgets */
final function UpdateGameDescription()
{
	local string LabelString;

	// Update the gamemode/mapname label
	if ( ScoreGameData.MyGRI != None )
	{
		LabelString = GetGameModeString(ScoreGameData.GameType) @ GameToMapConjunction @ GetMapNameString(ScoreGameData.MyGRI.GetURLMap());
	}
	ScoreGameData.GameAndMapLabel.SetDataStoreBinding( LabelString );

	// Update the match timer label
	if ( ScoreGameData.MyGRI != None )
	{
		LabelString = (ScoreGameData.MyGRI.GameStatus == GS_WaitingForHost) ? "" : FormatTime(ScoreGameData.MyGRI.RoundTime);
	}
	ScoreGameData.MatchTimeLabel.SetDataStoreBinding( LabelString );

	// Update the goal score label
	if ( ScoreGameData.MyGRI != None )
	{
		LabelString = GetGameGoalString();
	}
	ScoreGameData.GoalScoreLabel.SetDataStoreBinding( LabelString );

	// Update the game specific rule label and image
	UpdateGameSpecificRuleWidgets();

	// Update the footer/message
	if ( ScoreGameData.MyGRI == None )
	{
		ScoreGameData.FooterPanel.SetVisibility( false );
	}
	else
	{
		ScoreGameData.FooterPanel.SetVisibility( false );
		// Uncomment this if we find that we need a game message
		/*
		if ( ScoreGameData.MyGRI.GameStatus != GS_WaitingForHost )
		{
			ScoreGameData.FooterPanel.SetVisibility( false );
		}
		else
		{
			ScoreGameData.FooterPanel.SetVisibility( true );
			ScoreGameData.FooterMessageLabel.SetDataStoreBinding( GetGameMessageString() );
		}
		*/
	}
}

// Update the game specific rule label and image
final function UpdateGameSpecificRuleWidgets()
{
	if ( (ScoreGameData.MyGRI != None) && ((ScoreGameData.GameType == eGEARMP_Annex) || (ScoreGameData.GameType == eGEARMP_KOTH)) )
	{
		ScoreGameData.GameRuleLabel.SetDataStoreBinding( string(ScoreGameData.MyGRI.AnnexResourceGoal) );
		ScoreGameData.GameRuleLabel.SetVisibility( TRUE );
		ScoreGameData.GameRuleImage.SetVisibility( TRUE );
	}
	else
	{
		ScoreGameData.GameRuleLabel.SetVisibility( FALSE );
		ScoreGameData.GameRuleImage.SetVisibility( FALSE );
	}
}

/** Return the game message for display in the footer of the scoreboard */
final function string GetGameMessageString()
{
	return GetGearPlayerOwner().Role < ROLE_Authority ? WaitingForHost : PressStart;

	// return only this when the online party system is in place
	//return WaitingForPlayers;
}

/** Sets the labels for displaying the team information */
final function UpdateTeams()
{
	local int TeamIdx, PlayerIdx, BackGroundIdx, Count;
	local string LabelString;
	local Name TeamIconName, TeamBackgroundName;
	local bool bIsMeatflag;

	// Loop through all of the teams and update the team widgets and the player data
	for ( TeamIdx = 0; TeamIdx < ScoreGameData.TeamDataList.length; TeamIdx++ )
	{
		// Set stat header image styles
		SetGameSpecificStatHeaderStyles( TeamIdx );

		// If this is a game-type with more than 2 teams and one of the non-local teams does not have any players OR
		// This is a CombatTrial game (Horde) and the team is the Locust
		if ( (((ScoreGameData.TeamDataList.length > 2) && (TeamIdx > 0) && !TeamHasPlayers(TeamIdx)) ||
			  ((ScoreGameData.GameType == eGEARMP_CombatTrials) && (TeamIdx == 1))) &&
			 ((ScoreGameData.GameType != eGEARMP_CTM) || (TeamIdx != 2)) )
		{
			ScoreGameData.TeamDataList[TeamIdx].ParentPanel.SetVisibility( false );
			continue;
		}
		else
		{
			ScoreGameData.TeamDataList[TeamIdx].ParentPanel.SetVisibility( true );
		}

		// If there is more than 2 teams in this gametype we need to set the race icon, team name, and background
		Count = ScoreGameData.TeamDataList[TeamIdx].PlayerDataList.Length;
		if ( ScoreGameData.TeamDataList.length > 2 && ScoreGameData.GameType != eGEARMP_CTM )
		{
			// Set the data using the first player on the team to determine what the race is
			if ( (Count > 0) && (ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[0].PlayerPRI != None) )
			{
				// First set the race icon
				switch ( GetPRIRace(ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[0].PlayerPRI) )
				{
					case eGEARRACE_COG:
						TeamIconName = 'img_Scoreboard_CogSM';
						TeamBackgroundName = 'img_Scoreboard_CogRow';
						break;
					case eGEARRACE_Locust:
						TeamIconName = 'img_Scoreboard_LocustSM';
						TeamBackgroundName = 'img_Scoreboard_LocRow';
						break;
				}

				// Set the icon
				SetImageStyle(ScoreGameData.TeamDataList[TeamIdx].RaceImage, TeamIconName);

				// Set the player backgrounds
				for ( BackGroundIdx = 0; BackGroundIdx < Count; BackGroundIdx++ )
				{
					ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[BackGroundIdx].ParentPanel.SetWidgetStyleByName( 'Panel Background Style', TeamBackgroundName );
				}

				// Then set the team name
				ScoreGameData.TeamDataList[TeamIdx].TeamNameLabel.SetDataStoreBinding( Caps(GetTeamName(ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[0].PlayerPRI)) );
			}
		}

		bIsMeatflag = (ScoreGameData.GameType == eGEARMP_CTM && TeamIdx == 2);

		// Update the team score labels
		if ( !bIsMeatflag )
		{
			if ( ScoreGameData.GameType == eGEARMP_CombatTrials )
			{
				LabelString = string(ScoreGameData.MyGRI.InvasionCurrentWaveIndex);
			}
			else
			{
				LabelString = (ScoreGameData.MyGRI != None && ScoreGameData.MyGRI.Teams.length > TeamIdx && ScoreGameData.MyGRI.Teams[ScoreGameData.TeamDataList[TeamIdx].TeamIndex] != None)
					? string(int(ScoreGameData.MyGRI.Teams[ScoreGameData.TeamDataList[TeamIdx].TeamIndex].Score))
					: "";
			}
			ScoreGameData.TeamDataList[TeamIdx].ScoreLabel.SetDataStoreBinding( LabelString );
			ScoreGameData.TeamDataList[TeamIdx].ScoreLabel2.SetDataStoreBinding( LabelString );
		}

		// Loop through all of the players on the team and update the player widgets
		for ( PlayerIdx = 0; PlayerIdx < Count; PlayerIdx++ )
		{
			UpdatePlayer( TeamIdx, PlayerIdx );
		}
	}
}

/** Whether the team has players on it or not */
final function bool TeamHasPlayers( int TeamIdx )
{
	local int PlayerIdx;

	for ( PlayerIdx = 0; PlayerIdx < ScoreGameData.TeamDataList[TeamIdx].PlayerDataList.length; PlayerIdx++ )
	{
		if ( ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].PlayerPRI != None )
		{
			return true;
		}
	}

	return false;
}

/** Figures out the status of the player and returns the string for displaying that status */
final function Name GetPlayerStatus( GearPRI PlayerPRI )
{
	local bool bIsAssassinationLeader;

	if ( PlayerPRI != None )
	{
		bIsAssassinationLeader = ((ScoreGameData.GameType == eGEARMP_KTL) && PlayerPRI.bIsLeader);

		if ( PlayerIsDBNO(ScoreGameData.MyGRI, PlayerPRI) )
		{
			if ( bIsAssassinationLeader )
			{
				return  (GetRaceUsingPRIOnly(PlayerPRI) == eGEARRACE_COG) ? 'Scoreboard_LeaderDownCog' : 'Scoreboard_LeaderDownLocust';
			}
			else
			{
				return 'img_ScoreboardIcons_Downs';
			}
		}
		else if ( PlayerIsDead(ScoreGameData.MyGRI, PlayerPRI) )
		{
			if ( bIsAssassinationLeader )
			{
				return  (GetRaceUsingPRIOnly(PlayerPRI) == eGEARRACE_COG) ? 'Scoreboard_LeaderDeadCog' : 'Scoreboard_LeaderDeadLocust';
			}
			else
			{
				return 'img_ScoreboardIcons_Deaths';
			}
		}
		else if ( bIsAssassinationLeader )
		{
			return (GetRaceUsingPRIOnly(PlayerPRI) == eGEARRACE_COG) ? 'ScoreboardLeaderCog' : 'ScoreboardLeaderLocust';
		}
	}

	return '';
}

/** Figures out the rank of the player and returns the string for displaying that rank */
final function string GetPlayerRank( GearPRI PlayerPRI )
{
	local string RankString;

	RankString = " ";

	if ( PlayerPRI != None )
	{
		RankString = GetPlayerSkillString( PlayerPRI.PlayerSkill );
	}

	return RankString;
}

/** @return the style to use for this player's scoreboard line */
final function name GetPlayerTextStyle(PlayerReplicationInfo PRI)
{
	local GearPRI Player1PRI, Player2PRI;

	GetLocalPRIs(Player1PRI, Player2PRI);
	if (PRI == Player1PRI && Player1PRI != None && PlayerController(PRI.Owner).IsLocalPlayerController())
	{
		return 'cmb_Scoreboard_P1';
	}
	else if (PRI == Player2PRI && Player2PRI != None && PlayerController(PRI.Owner).IsLocalPlayerController())
	{
		return 'cmb_Scoreboard_P2';
	}
	else
	{
		return 'cmb_ScoreboardDefault';
	}
}

/** Sets the labels for displaying the player information */
final function UpdatePlayer( int TeamIdx, int PlayerIdx )
{
	local GearPRI PlayerPRI;
	local delegate<UpdatePlayerStats> StatsDelegate;
	local bool bPlayerExists, bPlayerIsDead;
	local Name FontStyleName, ImageStyleName;
	local int PlayerOwnerIndex;

	PlayerOwnerIndex = GetBestPlayerIndex();

	// Reference to the player PRI for code readability
	PlayerPRI = ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].PlayerPRI;

	// Set whether this player exists or not so we can blank out the strings if this slot is empty
	bPlayerExists = (PlayerPRI != None);

	// Set whether this player is dead or not
	bPlayerIsDead = PlayerIsDead( ScoreGameData.MyGRI, PlayerPRI );

	// Set the visibility of the death strike-through
	ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].StrikeImage.SetVisibility( bPlayerIsDead );

	// Figure out what style of text this player is supposed to use
	FontStyleName = GetPlayerTextStyle(PlayerPRI);

	// Update the player's name label
	ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].NameLabel.SetDataStoreBinding( bPlayerExists ? PlayerPRI.PlayerName : "" );
	SetWidgetEnabledState(ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].NameLabel, !bPlayerIsDead, PlayerOwnerIndex);
	SetLabelStyle(ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].NameLabel, FontStyleName );

	// Update the player's rank label
	ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].RankLabel.SetDataStoreBinding( bPlayerExists ? GetPlayerRank(PlayerPRI) : "" );

	// Update the player's status label
	ImageStyleName = GetPlayerStatus(PlayerPRI);
	if ( ImageStyleName == '' )
	{
		ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].StatusImage.SetVisibility( FALSE );
	}
	else
	{
		SetImageStyle(ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].StatusImage, ImageStyleName);
		ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].StatusImage.SetVisibility( TRUE );
	}

	// Update the stat labels of the player
	StatsDelegate = GameSpecificData[ScoreGameData.GameType].UpdatePlayerStatsDelegate;
	StatsDelegate( PlayerIdx, TeamIdx, bPlayerExists, !bPlayerIsDead, FontStyleName );
}

/** Set the state of the stat labels */
final function SetUIStateOnStats( int PlayerIdx, int TeamIdx, bool bPlayerEnabled, Name FontStyleName )
{
	local int StatIdx, PlayerOwnerIndex, Count;
	local UILabel Label;

	PlayerOwnerIndex = GetBestPlayerIndex();

	// Take care of the score first since it's not game specific
	Label = ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].ScoreLabel;
	SetWidgetEnabledState(Label, bPlayerEnabled, PlayerOwnerIndex );
	SetLabelStyle(Label, FontStyleName);

	// Now set the game specific data
	Count = ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].StatLabels.length;
	for ( StatIdx = 0; StatIdx < Count; StatIdx++ )
	{
		Label = ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].StatLabels[StatIdx];
		SetWidgetEnabledState(Label, bPlayerEnabled, PlayerOwnerIndex );
		SetLabelStyle(Label, FontStyleName);
	}
}

/** Set the stats of the player based on the ordering in the GameSpecificData structure */
final function SetGameSpecificPlayerStats( int PlayerIdx, int TeamIdx, bool bPlayerExists )
{
	local GearPRI PlayerPRI;
	local int StatIdx, Count;

	PlayerPRI = ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].PlayerPRI;

	// Every game uses the score and it has it's own special label so set that one first
	ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].ScoreLabel.SetDataStoreBinding( bPlayerExists ? string(PlayerPRI.GetScore()) : "" );

	// Now loop through the ordered stats unless this is not the local team for Wingman
	Count = GameSpecificData[ScoreGameData.GameType].StatOrder.length;
	for ( StatIdx = 0; StatIdx < Count; StatIdx++ )
	{
		ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].StatLabels[StatIdx].SetDataStoreBinding( bPlayerExists ? GetGameSpecificPlayerStat(PlayerPRI, StatIdx) : "" );
	}
}

/** Update the player's stats for a typical game where we will have a scoreboard for each team that will show the same stats for each player */
final function UpdatePlayerStats_Normal( int PlayerIdx, int TeamIdx, bool bPlayerExists, bool bPlayerEnabled, Name FontStyleName )
{
	SetGameSpecificPlayerStats( PlayerIdx, TeamIdx, bPlayerExists );
	SetUIStateOnStats( PlayerIdx, TeamIdx, bPlayerEnabled, FontStyleName );
}

/** Update the player's stats for a Combat Trials game (Special because we only show the COG team) */
final function UpdatePlayerStats_CombatTrials( int PlayerIdx, int TeamIdx, bool bPlayerExists, bool bPlayerEnabled, Name FontStyleName )
{
	local string TeamScoreStr;

	// don't show enemy team in CT
	if (TeamIdx > 0)
	{
		return;
	}

	TeamScoreStr = (ScoreGameData.MyGRI != None) ? string(ScoreGameData.MyGRI.InvasionCurrentWaveIndex) : "";
	ScoreGameData.TeamDataList[TeamIdx].ScoreLabel.SetDataStoreBinding(TeamScoreStr);
	ScoreGameData.TeamDataList[TeamIdx].ScoreLabel2.SetDataStoreBinding(TeamScoreStr);

	SetGameSpecificPlayerStats( PlayerIdx, TeamIdx, bPlayerExists );
	SetUIStateOnStats( PlayerIdx, TeamIdx, bPlayerEnabled, FontStyleName );
}

/** Update the player's stats for a Wingman game (Special because only the local team will show the detailed stats, other teams will show only score) */
final function UpdatePlayerStats_Wingman( int PlayerIdx, int TeamIdx, bool bPlayerExists, bool bPlayerEnabled, Name FontStyleName )
{
	local GearPRI PlayerPRI;

	// Only display the detailed stats if this is the local team
	if ( ScoreGameData.TeamDataList[TeamIdx].bIsLocalPlayerTeam )
	{
		SetGameSpecificPlayerStats( PlayerIdx, TeamIdx, bPlayerExists );
		SetUIStateOnStats( PlayerIdx, TeamIdx, bPlayerEnabled, FontStyleName );
	}
	else
	{
		PlayerPRI = ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].PlayerPRI;
		ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].ScoreLabel.SetDataStoreBinding( bPlayerExists ? string(PlayerPRI.GetScore()) : "" );
		SetWidgetEnabledState( ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].ScoreLabel, bPlayerEnabled, GetBestPlayerIndex() );
		SetLabelStyle(ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].ScoreLabel, FontStyleName);
	}
}

/** Update the player's stats for a Combat Trials game (Special because we only show the COG team) */
final function UpdatePlayerStats_CTM( int PlayerIdx, int TeamIdx, bool bPlayerExists, bool bPlayerEnabled, Name FontStyleName )
{
	local GearPRI PlayerPRI;

	// See whether this is the meatflag or not
	if ( ScoreGameData.GameType == eGEARMP_CTM && TeamIdx == 2 )
	{
		PlayerPRI = ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].PlayerPRI;

		// Every game uses the score and it has it's own special label so set that one first
		ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[PlayerIdx].ScoreLabel.SetDataStoreBinding( bPlayerExists ? string(PlayerPRI.GetScore()) : "" );

		// Now set stats
		ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[0].StatLabels[0].SetDataStoreBinding( bPlayerExists ? GetGameSpecificPlayerStat(PlayerPRI, eGSStat_Downs) : "" );
		ScoreGameData.TeamDataList[TeamIdx].PlayerDataList[0].StatLabels[1].SetDataStoreBinding( bPlayerExists ? GetGameSpecificPlayerStat(PlayerPRI, eGSStat_Kills) : "" );
	}
	else
	{
		UpdatePlayerStats_Normal( PlayerIdx, TeamIdx, bPlayerExists, bPlayerEnabled, FontStyleName );
	}
}

final function SetWidgetEnabledState( UIObject Widget, bool bEnabled, int PlayerIndex )
{
	local UIState CurrentState;

	CurrentState = Widget.GetCurrentState();
	if ( bEnabled )
	{
		if ( CurrentState.Class != class'UIState_Enabled' )
		{
			Widget.SetEnabled(true, PlayerIndex);
		}
	}
	else if ( CurrentState.Class != class'UIState_Disabled' )
	{
		Widget.SetEnabled(false, PlayerIndex);
	}
}

/**
 * Callback function called when the scene is activated
 * Will see if it needs to make itself visible or not
 * Overloaded to turn chat on
 */
function OnSceneActivatedCallback( UIScene ActivatedScene, bool bInitialActivation )
{
	Super.OnSceneActivatedCallback( ActivatedScene, bInitialActivation );

	if ( !IsEditor() )
	{
		if ( bInitialActivation )
		{
			// Turn chat delegates on
			TrackChat( true );
		}
	}
}

/** Called just after this scene is removed from the active scenes array */
event SceneDeactivated()
{
	Super.SceneDeactivated();

	ClearPlayerPRIs();

	// Turn chat delegates off
	TrackChat( false );
}

defaultproperties
{
	OnGearUISceneTick=UpdateScoreboardLabels
	TimeSinceLastUpdate=-1
	bCaptureMatchedInput=false

	GameSpecificData(eGEARMP_Warzone)=(GameNameVariableString="Warzone",UpdatePlayerStatsDelegate=GearUIScene_Scoreboard.UpdatePlayerStats_Normal,StatOrder=(eGSStat_Kills,eGSStat_Downs,eGSStat_Revives,eGSStat_Deaths))
	GameSpecificData(eGEARMP_Execution)=(GameNameVariableString="Execution",UpdatePlayerStatsDelegate=GearUIScene_Scoreboard.UpdatePlayerStats_Normal,StatOrder=(eGSStat_Kills,eGSStat_Downs,eGSStat_Revives,eGSStat_Deaths))
	GameSpecificData(eGEARMP_KTL)=(GameNameVariableString="KTL",UpdatePlayerStatsDelegate=GearUIScene_Scoreboard.UpdatePlayerStats_Normal,StatOrder=(eGSStat_Kills,eGSStat_Downs,eGSStat_Revives,eGSStat_Deaths))
	GameSpecificData(eGEARMP_CombatTrials)=(GameNameVariableString="CT",UpdatePlayerStatsDelegate=GearUIScene_Scoreboard.UpdatePlayerStats_Normal,StatOrder=(eGSStat_Kills,eGSStat_Downs,eGSStat_Revives,eGSStat_Deaths))
	GameSpecificData(eGEARMP_Annex)=(GameNameVariableString="Annex",UpdatePlayerStatsDelegate=GearUIScene_Scoreboard.UpdatePlayerStats_Normal,StatOrder=(eGSStat_AnnexCap,eGSStat_AnnexBreak,eGSStat_Kills,eGSStat_Downs))
	GameSpecificData(eGEARMP_Wingman)=(GameNameVariableString="Wingman",UpdatePlayerStatsDelegate=GearUIScene_Scoreboard.UpdatePlayerStats_Wingman,StatOrder=(eGSStat_Kills,eGSStat_Downs,eGSStat_Revives,eGSStat_Deaths))
	GameSpecificData(eGEARMP_KOTH)=(GameNameVariableString="KOTH",UpdatePlayerStatsDelegate=GearUIScene_Scoreboard.UpdatePlayerStats_Normal,StatOrder=(eGSStat_AnnexCap,eGSStat_AnnexBreak,eGSStat_Kills,eGSStat_Downs))
	GameSpecificData(eGEARMP_CTM)=(GameNameVariableString="CTM",UpdatePlayerStatsDelegate=GearUIScene_Scoreboard.UpdatePlayerStats_CTM,StatOrder=(eGSStat_MeatflagCap,eGSStat_Kills,eGSStat_Downs,eGSStat_Revives))

	StatHeaderStyles(eGSStat_Deaths)="img_ScoreboardIcons_Deaths"
	StatHeaderStyles(eGSStat_Downs)="img_ScoreboardIcons_Downs"
	StatHeaderStyles(eGSStat_Kills)="img_ScoreboardIcons_Kills"
	StatHeaderStyles(eGSStat_Revives)="img_ScoreboardIcons_Revives"
	StatHeaderStyles(eGSStat_AnnexCap)="Scoreboard_Annex<TEAM>"
	StatHeaderStyles(eGSStat_AnnexBreak)="Scoreboard_Annex<TEAM>Break"
	StatHeaderStyles(eGSStat_MeatflagCap)="Scoreboard_MeatFlagCap"
}
