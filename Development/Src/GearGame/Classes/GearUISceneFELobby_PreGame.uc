/**
 * The pre-game lobby scene.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUISceneFELobby_PreGame extends GearUISceneFELobby_Base
	config(inherit);


/************************************************************************/
/* Constants, structures, enums, etc.									*/
/************************************************************************/

/** Structure containing all the widgets required for the voting of one gametype */
struct GearPGLGameVoteData
{
	/** Parent panel for this gametype being voted on */
	var transient UIPanel GameVotePanel;

	/** Title of the game getting voted on */
	var transient UILabel GameVoteTitleLabel;

	/** Image of the game getting voted on */
	var transient UIImage GameVoteImage;

	/** Current number of votes for the game getting voted on */
	var transient UILabel GameVoteTallyLabel;

	/** Background image if this game is the current winner */
	var transient UIImage GameVoteWinningBGImage;

	/** The voting icon image */
	var transient UILabel VoteIconImage;
};

/** Structure containing all the widgets and data required for the game-voting/game-descriptions in the pre-game lobby */
struct GearPGLGameData
{
	/** Parent panel of the gametype area */
	var transient UIPanel ParentPanel;

	/** --------INACTIVE MODE DATA--------- */

	/** Parent panel for the game section being inactive (prior to being voted on) */
	var transient UIPanel InactivePanel;

	/** --------DESCRIPTION MODE DATA--------- */

	/** Parent panel for the Description of the gametype */
	var transient UIPanel DescribePanel;

	/** Title of the game type */
	var transient UILabel DescribeTitleLabel;

	/** Description of the game type */
	var transient UILabel DescribeDescriptionLabel;

	/** Image of the game type */
	var transient UIImage DescribeImage;

	/** --------VOTING MODE DATA--------- */

	/** Parent panel for all the gametypes being voted on */
	var transient UIPanel VotingPanel;

	/** The image shown to designate that gametypes are being voted on */
	var transient UIImage VotingImage;

	/** List of data structures (one for each game being voted on) */
	var transient array<GearPGLGameVoteData> GameVoteData;

	/** Label displaying the time remaining in the gametype vote */
	var transient UILabel GameVoteTimerLabel;

	/** --------PRESS START DATA--------- */

	/** Parent panel for the area in the lobby that tells the host to press start */
	var transient UIPanel TooltipPanel;

	/** The text region for the area in the lobby that tells the host to press start */
	var transient UILabel TooltipLabel;

	/** The highlight image behind the press start panel */
	var transient UIImage TooltipBGImage;

	/** Indicates that the game is loading */
	var transient UIImage TooltipLoadingIndicator;
};

/** Structure containing all the widgets and data required for the voting of one map */
struct GearPGLMapVoteData
{
	/** Parent panel for this map being voted on */
	var transient UIPanel MapVotePanel;

	/** Label displaying the name of the map being voted on */
	var transient UILabel MapNameLabel;

	/** Current number of votes for this map */
	var transient UILabel MapVoteLabel;

	/** Background image if this map is the current winner */
	var transient UIImage MapVoteWinningBGImage;

	/** The voting icon image */
	var transient UILabel VoteIconImage;
};

/** Structure containing all the widgets and data required for the map-voting/map-descriptions in the pre-game lobby */
struct GearPGLMapData
{
	/** Parent panel of the map area */
	var transient UIPanel ParentPanel;

	/** --------INACTIVE MODE DATA--------- */

	/** Parent panel for the map section being inactive (prior to being voted on) */
	var transient UIPanel InactivePanel;

	/** --------DESCRIPTION MODE DATA--------- */

	/** Parent Panel for the Description of the map (after the vote) */
	var transient UIPanel DescribePanel;

	/** Title of the map */
	var transient UILabel DescribeTitleLabel;

	/** The image of the map being described */
	var transient UIImage DescribeMapImage;

	/** --------HOST SELECT MODE DATA--------- */

	/** Parent panel for host map selection */
	var transient UIPanel SelectPanel;

	/** Left trigger label during host map selection */
	var transient UILabel SelectLeftTrigger;

	/** Right trigger label during host map selection */
	var transient UILabel SelectRightTrigger;

	/** Title of the map being selected */
	var transient UILabel SelectTitleLabel;

	/** The image of the map being selected */
	var transient UIImage SelectMapImage;

	/** Label displaying the time remaining in the map selection */
	var transient UILabel MapSelectTimerLabel;

	/** Image to display when someone does not have the map */
	var transient UIImage MissingMapImage;

	/** --------VOTING MODE DATA--------- */

	/** Parent panel for all the maps being voted on */
	var transient UIPanel VotingPanel;

	/** The image shown to designate that maps are being voted on */
	var transient UIImage VotingImage;

	/** The image of the maps being voted on */
	var transient UIImage VotingMapImage;

	/** List of data structures (one of each map being voted on) */
	var transient array<GearPGLMapVoteData> MapVoteData;

	/** Label displaying the time remaining in the map vote */
	var transient UILabel MapVoteTimerLabel;

	/** Start value of the line that separates the 2 maps when voting */
	var transient float VoteImageValueStart;
	/** Destination value of the line that separates the 2 maps when voting */
	var transient float VoteImageValueDestination;
	/** Current value of the line that separates the 2 maps when voting */
	var transient float VoteImageValueCurrent;
	/** The time the vote image value destination changed */
	var transient int VoteImageDestinationChangeTime;
};

/** Structure containing all the widgets and data required for selecting/displaying a player's character */
struct GearPGLCharacterSelectData
{
	/** Parent panel for this character's widgets */
	var transient UIPanel CharacterPanel;

	/** The image shown to designate that the character is being selected */
	var transient UIImage SelectionImage;

	/** Label of the gamertag of this character */
	var transient UILabel GamerTagLabel;

	/** Current image of the character */
	var transient UIImage CharacterImage;

	/** Decrement/Increment labels for selecting the character */
	var transient UILabel CharacterDecrementLabel;
	var transient UILabel CharacterIncrementLabel;

	/** Current image of the weapon */
	var transient UIImage WeaponImage;

	/** Decrement/Increment labels for selecting the weapon */
	var transient UILabel WeaponDecrementLabel;
	var transient UILabel WeaponIncrementLabel;

	/** Label displaying the time remaining in the character selection */
	var transient UILabel SelectionTimerLabel;

	/** Current provider index of the selected character classes */
	var transient int CurrCOGCharacterSelected;
	var transient int CurrLocustCharacterSelected;

	/** Current provider index of the selected weapon class */
	var transient int CurrWeaponSelected;
};

/** Structure containing all the widgets and data required for the character/weapon selection in the pre-game lobby */
struct GearPGLCharacterData
{
	/** Parent panel of the character area */
	var transient UIPanel ParentPanel;

	/** --------INACTIVE MODE DATA--------- */

	/** Parent panel for the character section being inactive (prior to selection time) */
	var transient UIPanel InactivePanel;

	/** --------SELECTION/DISPLAY MODE DATA--------- */

	/** List of data structures (one for each gamertag selecting their character/weapon) */
	var transient array<GearPGLCharacterSelectData> SelectionData;
};

/** Structure containing all the widgets and data required for each player on a team */
struct GearPGLTeamMemberData
{
	/** Label button of the player */
	var transient UILabelButton ParentButton;

	/** Label that displays which controller this player uses */
	var transient UILabel ProfileLabel;

	/** Label the displays the skill level of the player */
	var transient UILabel RankLabel;

	/** Chat icon image of this player */
	var transient UIImage ChatIcon;

	/** The PRI of the player */
	var transient GearPreGameLobbyPRI PlayerPRI;

	/** Whether this spot will be filled by a bot or not */
	var transient bool bIsBot;
};

/** Structure containing all the widgets and data required for each team */
struct GearPGLTeamData
{
	/** Parent panel of the team */
	var transient UIPanel ParentPanel;

	/** List of the player data for this team */
	var transient array<GearPGLTeamMemberData> DataList;
};

/** Structure containing the team and player widgets & data for a specific kind of game */
struct GearPGLGameTypePlayerData
{
	/** Parent panel of the team data for this game */
	var transient UIPanel PlayersPanel;

	/** List of team data for this game */
	var transient array<GearPGLTeamData> PlayersTeamData;

};

/** Structure containing all the widgets and data required for handling the player lists */
struct GearPGLPlayerData
{
	/** List of game type player data */
	var transient array<GearPGLGameTypePlayerData> GameTeamData;

	/** Background of the player lists */
	var transient UIImage PlayerBackgroundImage;
};

/** Enum of the different playerlist formats for various games */
enum EGearPGLPlayerFormat
{
	ePGL_Normal,
	ePGL_Wingman,
};

/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Data for voting on gametypes and displaying the winner of the vote */
var transient GearPGLGameData PGLGameData;

/** Data for voting on maps and displaying the winner of the vote */
var transient GearPGLMapData PGLMapData;

/** Data for selecting the character and weapon for players */
var transient GearPGLCharacterData PGLCharacterData;

/** Data for maintaining and updating the player lists */
var transient GearPGLPlayerData PGLPlayerData;

/** Button bar at the bottom of the screen */
var	transient UICalloutButtonPanel btnbarPlayers;

/** Current state of the lobby */
var transient EGearPGLState CurrPGLobbyState;

/** Last time a pulse was set from the GRI */
var transient float LastPulseTime;

/** Whether the screen is ready for player data to be shown or not (copied from the GRI) */
var transient bool bPlayerDataReadyForShow;

/**
 * indicates that a new value for one of the timers has been replicated and the UI can play a beep if the counter is less than 5 seconds
 * remaining.
 */
var	transient	bool	bPlayCountdownBeep;

/**
 * Has the host triggered the pregame countdown; only applies to unofficial games
 */
var transient	bool	bHostStartedPregameCountdown;

/** How many times  we've successfully voted for a game type from this screen (could be 2 if we have a splitscreen player) */
var transient int NumLocalGameVotes;

/** How many times we've successfully voted for a map from this screen (could be 2 if we have a splitscreen player) */
var transient int NumLocalMapVotes;

/** Localized strings */
var localized string PlayerLoading;
var localized string PlayerEmpty;
var localized string MissingMap;
var localized string PressStartHost;
var localized string PressStartClient;
var localized string PressedStart;
var localized string TiedVote;
var localized string HostSelectingMap;
var localized string TooltipGametypeVoting;
var localized string TooltipMapVoting;
var localized string TooltipMapSelection;
var localized string TooltipMapSelectionClient;
var localized string TooltipCharacterSelection;
var localized string TooltipWingman;
var localized string TooltipReady;
var localized string TooltipWaitingForMatchStart;
var localized string BotSlotString;



/** Amount of time it takes the map vote image to interpolate */
var const float MapVoteInterpTime;


/** When TimerTimeLeft > 0.0 we decrement it by DeltaTime. Once the timer hits 0 we call OnTimerTriggered*/
var transient float TimerTimeLeft;

/** Time till next DLC check */
var transient float DLCCheckTime;

/** The cached enum value of the gametype that was voted for in public matches */
var transient EGearMPTypes PublicMatchType;

/** Whether the map selection initialization has completed or not (used to make sure the player doesn't spam the start button passed this stage) */
var transient bool bMapSelectInitCompleted;

/** Whether the character selection initialization has completed or not (used to make sure the player doesn't spam the start button passed this stage) */
var transient bool bCharacterSelectInitCompleted;

/************************************************************************/
/* Animations                                                           */
/************************************************************************/
var transient UIAnimationSeq GameTypeExpand;
var transient UIAnimationSeq MapSelectionExpand;
var transient UIAnimationSeq CharacterSelectExpand;
var transient UIAnimationSeq FadeIn;
var transient UIAnimationSeq FadeOut;
var transient UIAnimationSeq QuickFade;
var transient UIAnimationSeq MapSelectionSizeToDescription;
var transient UIAnimationSeq MoveToDescriptionsBottom;

/************************************************************************/
/* Sounds                                                               */
/************************************************************************/
var transient name LobbyCharacterScroll;
var transient name LobbyCharacterSelect;
var transient name LobbyScreenSlideBack;
var transient name LobbyScreenSlideForward;
var transient name LobbyWeaponScroll;
var transient name LobbyWeaponSelect;
var transient name VoteCompleteCue;
var transient name VoteCue;
var transient name VoteOpenCue;
var transient name VoteOpenPulseCue;

/** Called when an animation sequence has completed */
delegate OnTimerTriggered();

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/


/** 
 * Waits for the TimerValue to count down to 0.
 * When the counter reaches 0, OnTimerTriggered() will be called.
 * 
 * @param TimerValue The value to count down from.
 */
function WaitForTimer(float TimerValue)
{
	TimerTimeLeft = TimerValue;
}


/**
 * Called after this screen object's children have been initialized.  While the Initialized event is only called when
 * a widget is initialized for the first time, PostInitialize() will be called every time this widget receives a call
 * to Initialize(), even if the widget was already initialized.  Examples would be reparenting a widget.
 */
event PostInitialize()
{
	Super.PostInitialize();

	if ( !IsEditor() )
	{
		InitializeWidgetReferences();

		SetupCallbacks();
	}
}

/** Assigns all member variables to appropriate child widget from this scene */
function InitializeWidgetReferences()
{
	local int Idx, PlayerIdx, PlayerIndex;
	local string WidgetName;
	local bool bIsWingman;

	bIsWingman = IsWingmanGametype();

	// GAME SELECTION WIDGETS

	PGLGameData.ParentPanel =				UIPanel(FindChild('pnlPlaylist',true));
	PGLGameData.InactivePanel =				UIPanel(PGLGameData.ParentPanel.FindChild('pnlGametypeInactive',true));
	PGLGameData.DescribePanel =				UIPanel(PGLGameData.ParentPanel.FindChild('pnlGametypeDescribe',true));
	PGLGameData.DescribeTitleLabel =		UILabel(PGLGameData.DescribePanel.FindChild('lblTitle',true));
	PGLGameData.DescribeDescriptionLabel =	UILabel(PGLGameData.DescribePanel.FindChild('lblDescribe',true));
	PGLGameData.DescribeImage =				UIImage(PGLGameData.DescribePanel.FindChild('imgIcon',true));
	PGLGameData.VotingPanel =				UIPanel(PGLGameData.ParentPanel.FindChild('pnlPlaylistVote',true));
	PGLGameData.VotingImage =				UIImage(FindChild('imgVoteGametype',true));
	PGLGameData.GameVoteTimerLabel =		UILabel(PGLGameData.VotingPanel.FindChild('lblTime',true));
	PGLGameData.TooltipLabel =			UILabel(FindChild('lblLobbyText',true));
	PGLGameData.TooltipPanel =			UIPanel(FindChild('pnlStartGame',true));
	PGLGameData.TooltipPanel.SetVisibility( false );
	PGLGameData.TooltipBGImage =			UIImage(FindChild('imgStartGameBG',true));
	PGLGameData.TooltipBGImage.SetVisibility( false );
	PGLGameData.TooltipLoadingIndicator = UIImage(FindChild('imgSpinner', true));
	PGLGameData.TooltipLoadingIndicator.SetVisibility( false );

	PGLGameData.GameVoteData.length = 2;
	for ( Idx = 0; Idx < PGLGameData.GameVoteData.length; Idx++ )
	{
		WidgetName = "pnlGametype" $ Idx;
		PGLGameData.GameVoteData[Idx].GameVotePanel =			UIPanel(PGLGameData.VotingPanel.FindChild(Name(WidgetName),true));
		PGLGameData.GameVoteData[Idx].GameVoteTitleLabel =		UILabel(PGLGameData.GameVoteData[Idx].GameVotePanel.FindChild('lblGametype',true));
		PGLGameData.GameVoteData[Idx].GameVoteImage =			UIImage(PGLGameData.GameVoteData[Idx].GameVotePanel.FindChild('imgIcon',true));
		PGLGameData.GameVoteData[Idx].GameVoteTallyLabel =		UILabel(PGLGameData.GameVoteData[Idx].GameVotePanel.FindChild('lblGametypeVote',true));
		PGLGameData.GameVoteData[Idx].GameVoteWinningBGImage =	UIImage(PGLGameData.GameVoteData[Idx].GameVotePanel.FindChild('imgWinner',true));
		PGLGameData.GameVoteData[Idx].VoteIconImage =			UILabel(PGLGameData.GameVoteData[Idx].GameVotePanel.FindChild('lblLTrigger',true));
	}

	// MAP SELECTION WIDGETS

	PGLMapData.ParentPanel =			UIPanel(FindChild('pnlMapVote',true));
	PGLMapData.InactivePanel =			UIPanel(PGLMapData.ParentPanel.FindChild('pnlMapInactive',true));
	PGLMapData.DescribePanel =			UIPanel(PGLMapData.ParentPanel.FindChild('pnlMapDescribe',true));
	PGLMapData.DescribeTitleLabel =		UILabel(PGLMapData.DescribePanel.FindChild('lblTitle',true));
	PGLMapData.DescribeMapImage =		UIImage(PGLMapData.DescribePanel.FindChild('imgMap',true));
	PGLMapData.SelectPanel =			UIPanel(PGLMapData.ParentPanel.FindChild('pnlMapHostSelect',true));
	PGLMapData.SelectLeftTrigger=		UILabel(PGLMapData.SelectPanel.FindChild('lblLTrigger',true));
	PGLMapData.SelectRightTrigger=		UILabel(PGLMapData.SelectPanel.FindChild('lblRTrigger',true));
	PGLMapData.SelectTitleLabel =		UILabel(PGLMapData.SelectPanel.FindChild('lblMap',true));
	PGLMapData.SelectMapImage =			UIImage(PGLMapData.SelectPanel.FindChild('imgMap',true));
	PGLMapData.MapSelectTimerLabel =	UILabel(PGLMapData.SelectPanel.FindChild('lblTime',true));
	PGLMapData.MissingMapImage =		UIImage(PGLMapData.SelectPanel.FindChild('imgError',true));
	PGLMapData.VotingPanel =			UIPanel(PGLMapData.ParentPanel.FindChild('pnlMapVote',true));
	PGLMapData.VotingImage =			UIImage(FindChild('imgVoteMap',true));
	PGLMapData.VotingMapImage =			UIImage(PGLMapData.VotingPanel.FindChild('imgMap',true));
	PGLMapData.MapVoteTimerLabel =		UILabel(PGLMapData.VotingPanel.FindChild('lblTime',true));

	PGLMapData.MapVoteData.length = 2;
	for ( Idx = 0; Idx < PGLMapData.MapVoteData.length; Idx++ )
	{
		WidgetName = "pnlMap" $ Idx;
		PGLMapData.MapVoteData[Idx].MapVotePanel =			UIPanel(PGLMapData.VotingPanel.FindChild(Name(WidgetName),true));
		PGLMapData.MapVoteData[Idx].MapNameLabel =			UILabel(PGLMapData.MapVoteData[Idx].MapVotePanel.FindChild('lblMap',true));
		PGLMapData.MapVoteData[Idx].MapVoteLabel =			UILabel(PGLMapData.MapVoteData[Idx].MapVotePanel.FindChild('lblMapVote',true));
		PGLMapData.MapVoteData[Idx].MapVoteWinningBGImage =	UIImage(PGLMapData.MapVoteData[Idx].MapVotePanel.FindChild('imgWinner',true));
		PGLMapData.MapVoteData[Idx].VoteIconImage =			UILabel(PGLMapData.MapVoteData[Idx].MapVotePanel.FindChild( (Idx == 0) ? 'lblLTrigger' : 'lblRTrigger',true));
	}

	// CHARACTER/WEAPON SELECTION WIDGETS

	PGLCharacterData.ParentPanel =		UIPanel(FindChild('pnlCharacter',true));
	PGLCharacterData.InactivePanel =	UIPanel(PGLCharacterData.ParentPanel.FindChild('pnlCharInactive',true));

	PGLCharacterData.SelectionData.length = 2;
	for ( Idx = 0; Idx < PGLCharacterData.SelectionData.length; Idx++ )
	{
		WidgetName = "pnlPlayer" $ Idx $ "Setup";
		PGLCharacterData.SelectionData[Idx].CharacterPanel =			UIPanel(PGLCharacterData.ParentPanel.FindChild(Name(WidgetName),true));
		WidgetName = "imgCharSetup" $ Idx;
		PGLCharacterData.SelectionData[Idx].SelectionImage =			UIImage(FindChild(Name(WidgetName),true));
		PGLCharacterData.SelectionData[Idx].GamerTagLabel =				UILabel(PGLCharacterData.SelectionData[Idx].CharacterPanel.FindChild('lblTitle',true));
		PGLCharacterData.SelectionData[Idx].CharacterImage =			UIImage(PGLCharacterData.SelectionData[Idx].CharacterPanel.FindChild('imgCharacter',true));
		PGLCharacterData.SelectionData[Idx].CharacterDecrementLabel =	UILabel(PGLCharacterData.SelectionData[Idx].CharacterPanel.FindChild('lblLTrigger',true));
		PGLCharacterData.SelectionData[Idx].CharacterIncrementLabel =	UILabel(PGLCharacterData.SelectionData[Idx].CharacterPanel.FindChild('lblRTrigger',true));
		PGLCharacterData.SelectionData[Idx].WeaponImage =				UIImage(PGLCharacterData.SelectionData[Idx].CharacterPanel.FindChild('imgWeapon',true));
		PGLCharacterData.SelectionData[Idx].WeaponDecrementLabel =		UILabel(PGLCharacterData.SelectionData[Idx].CharacterPanel.FindChild('lblLBumper',true));
		PGLCharacterData.SelectionData[Idx].WeaponIncrementLabel =		UILabel(PGLCharacterData.SelectionData[Idx].CharacterPanel.FindChild('lblRBumper',true));
		PGLCharacterData.SelectionData[Idx].SelectionTimerLabel =		UILabel(PGLCharacterData.SelectionData[Idx].CharacterPanel.FindChild('lblTime',true));
	}

	// PLAYER WIDGETS
	PGLPlayerData.GameTeamData.length = ePGL_MAX;

	// Normal Player Lists
	PGLPlayerData.GameTeamData[ePGL_Normal].PlayersPanel = UIPanel(FindChild('pnlPlayerParty',true));
	PGLPlayerData.GameTeamData[ePGL_Normal].PlayersPanel.SetVisibility( !bIsWingman );
	PGLPlayerData.GameTeamData[ePGL_Normal].PlayersTeamData.length = 2;
	for ( Idx = 0; Idx < PGLPlayerData.GameTeamData[ePGL_Normal].PlayersTeamData.length; Idx++ )
	{
		WidgetName = "pnlTeam" $ Idx;
		PGLPlayerData.GameTeamData[ePGL_Normal].PlayersTeamData[Idx].ParentPanel = UIPanel(PGLPlayerData.GameTeamData[ePGL_Normal].PlayersPanel.FindChild(Name(WidgetName),true));
		PGLPlayerData.GameTeamData[ePGL_Normal].PlayersTeamData[Idx].ParentPanel.SetVisibility( false );

		PGLPlayerData.GameTeamData[ePGL_Normal].PlayersTeamData[Idx].DataList.length = 5;
		for ( PlayerIdx = 0; PlayerIdx < PGLPlayerData.GameTeamData[ePGL_Normal].PlayersTeamData[Idx].DataList.length; PlayerIdx++ )
		{
			WidgetName = "btnPlayer" $ PlayerIdx;
			PGLPlayerData.GameTeamData[ePGL_Normal].PlayersTeamData[Idx].DataList[PlayerIdx].ParentButton =		UILabelButton(PGLPlayerData.GameTeamData[ePGL_Normal].PlayersTeamData[Idx].ParentPanel.FindChild(Name(WidgetName),true));
			PGLPlayerData.GameTeamData[ePGL_Normal].PlayersTeamData[Idx].DataList[PlayerIdx].ParentButton.DisableWidget( GetPlayerOwnerIndex() );
			PGLPlayerData.GameTeamData[ePGL_Normal].PlayersTeamData[Idx].DataList[PlayerIdx].ProfileLabel =		UILabel(PGLPlayerData.GameTeamData[ePGL_Normal].PlayersTeamData[Idx].DataList[PlayerIdx].ParentButton.FindChild('lblProfile',true));
			PGLPlayerData.GameTeamData[ePGL_Normal].PlayersTeamData[Idx].DataList[PlayerIdx].RankLabel =		UILabel(PGLPlayerData.GameTeamData[ePGL_Normal].PlayersTeamData[Idx].DataList[PlayerIdx].ParentButton.FindChild('lblRank',true));
			WidgetName = (Idx == 0) ? "imgCogChat" : "imgLocustChat";
			WidgetName $= PlayerIdx;
			PGLPlayerData.GameTeamData[ePGL_Normal].PlayersTeamData[Idx].DataList[PlayerIdx].ChatIcon =			UIImage(PGLPlayerData.GameTeamData[ePGL_Normal].PlayersPanel.FindChild(Name(WidgetName),true));
			PGLPlayerData.GameTeamData[ePGL_Normal].PlayersTeamData[Idx].DataList[PlayerIdx].ChatIcon.SetVisibility( true );
			PGLPlayerData.GameTeamData[ePGL_Normal].PlayersTeamData[Idx].DataList[PlayerIdx].ChatIcon.Opacity = 0.0f;
		}
	}

	// Wingman Player Lists
	PGLPlayerData.GameTeamData[ePGL_Wingman].PlayersPanel = UIPanel(FindChild('pnlWingmanPlayerLists',true));
	PGLPlayerData.GameTeamData[ePGL_Wingman].PlayersPanel.SetVisibility( bIsWingman );
	PGLPlayerData.GameTeamData[ePGL_Wingman].PlayersTeamData.length = 5;
	for ( Idx = 0; Idx < PGLPlayerData.GameTeamData[ePGL_Wingman].PlayersTeamData.length; Idx++ )
	{
		WidgetName = "pnlwmTeam" $ Idx;
		PGLPlayerData.GameTeamData[ePGL_Wingman].PlayersTeamData[Idx].ParentPanel =	UIPanel(PGLPlayerData.GameTeamData[ePGL_Wingman].PlayersPanel.FindChild(Name(WidgetName),true));
		PGLPlayerData.GameTeamData[ePGL_Wingman].PlayersTeamData[Idx].ParentPanel.SetVisibility( false );

		PGLPlayerData.GameTeamData[ePGL_Wingman].PlayersTeamData[Idx].DataList.length = 2;
		for ( PlayerIdx = 0; PlayerIdx < PGLPlayerData.GameTeamData[ePGL_Wingman].PlayersTeamData[Idx].DataList.length; PlayerIdx++ )
		{
			WidgetName = "btnPlayer" $ PlayerIdx;
			PGLPlayerData.GameTeamData[ePGL_Wingman].PlayersTeamData[Idx].DataList[PlayerIdx].ParentButton =	UILabelButton(PGLPlayerData.GameTeamData[ePGL_Wingman].PlayersTeamData[Idx].ParentPanel.FindChild(Name(WidgetName),true));
			PGLPlayerData.GameTeamData[ePGL_Wingman].PlayersTeamData[Idx].DataList[PlayerIdx].ParentButton.DisableWidget( GetPlayerOwnerIndex() );
			PGLPlayerData.GameTeamData[ePGL_Wingman].PlayersTeamData[Idx].DataList[PlayerIdx].ProfileLabel =	UILabel(PGLPlayerData.GameTeamData[ePGL_Wingman].PlayersTeamData[Idx].DataList[PlayerIdx].ParentButton.FindChild('lblProfile',true));
			PGLPlayerData.GameTeamData[ePGL_Wingman].PlayersTeamData[Idx].DataList[PlayerIdx].RankLabel =		UILabel(PGLPlayerData.GameTeamData[ePGL_Wingman].PlayersTeamData[Idx].DataList[PlayerIdx].ParentButton.FindChild('lblRank',true));
			WidgetName = "imgwmChat" $ Idx $ "_" $ PlayerIdx;
			PGLPlayerData.GameTeamData[ePGL_Wingman].PlayersTeamData[Idx].DataList[PlayerIdx].ChatIcon =		UIImage(PGLPlayerData.GameTeamData[ePGL_Wingman].PlayersPanel.FindChild(Name(WidgetName),true));
		}
	}

	// Player BG Image
	PGLPlayerData.PlayerBackgroundImage = UIImage(FindChild('imgPartyMemberBG',true));

	// Button bar
	btnbarPlayers = UICalloutButtonPanel(FindChild('pnlPlayerButtonBar', true));
	if ( IsOfficialMatch() )
	{
		btnbarPlayers.SetVisibility( false );
	}
	else
	{
		PlayerIndex = GetPlayerOwnerIndex();
		btnbarPlayers.SetVisibility( true );
		btnbarPlayers.EnableButton('TeamChange', PlayerIndex, PlayersCanChangeTeams(), false);
		btnbarPlayers.EnableButton('DLC', PlayerIndex, ShouldEnableDLC(), false);
		// The host can't return to lobby until we can switch teams (fixes clients softlock)
		btnbarPlayers.EnableButton('GenericBack', PlayerIndex, CanLeaveGame(), false);
	}
}

/** Returns the number of teams we need to display */
function GetNumTeamsToDisplay( out int NumTeams, out int NumPlayerPerTeam )
{
	local OnlinePlaylistManager PlaylistMan;
	local OnlineSubsystem OnlineSub;
	local GearPreGameGRI PreGameGRI;

	if ( IsOfficialMatch() )
	{
		NumTeams = 2;
		NumPlayerPerTeam = 5;
		PreGameGRI = GearPreGameGRI(GetGRI());
		if ( PreGameGRI != None )
		{
			OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
			if ( OnlineSub != None )
			{
				PlaylistMan = OnlinePlaylistManager(OnlineSub.GetNamedInterface('PlaylistManager'));
				if ( PlaylistMan != None )
				{
					PlaylistMan.GetTeamInfoFromPlaylist( PreGameGRI.PlaylistId, NumPlayerPerTeam, NumTeams );
				}
			}
		}
	}
	else
	{
		if ( IsWingmanGametype() )
		{
			NumTeams = 5;
			NumPlayerPerTeam = 2;
		}
		else if ( IsHordeGametype() )
		{
			NumTeams = 1;
			NumPlayerPerTeam = 5;
		}
		else
		{
			NumTeams = 2;
			NumPlayerPerTeam = 5;
		}
	}
}

/**
 * Assigns delegates in important child widgets to functions in this scene class.
 */
function SetupCallbacks()
{
	local int GameIdx, Idx, PlayerIdx;

	for ( GameIdx = 0; GameIdx < PGLPlayerData.GameTeamData.length; GameIdx++ )
	{
		for ( Idx = 0; Idx < PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData.length; Idx++ )
		{
			for ( PlayerIdx = 0; PlayerIdx < PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[Idx].DataList.length; PlayerIdx++ )
			{
				PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[Idx].DataList[PlayerIdx].ParentButton.OnClicked = OnPlayerListPress;
			}
		}
	}

	if ( !IsOfficialMatch() )
	{
		btnbarPlayers.SetButtonCallback('TeamChange', OnChangeTeam);
		btnbarPlayers.SetButtonCallback('DLC', OnDownloadContent);
		btnbarPlayers.SetButtonCallback('GenericBack', OnLeaveGame);
	}
}

/** Called when a player requests to change teams */
function bool OnChangeTeam( UIScreenObject EventObject, int PlayerIndex )
{
	local GearPC PC;
	local GearPreGameGRI PreGameGRI;
	local int NumTeams, NumPlayers;

	PC = GetGearPlayerOwner( PlayerIndex );
	PreGameGRI = GearPreGameGRI(GetGRI());
	if ( PC != None && PreGameGRI != None )
	{
		GetNumTeamsToDisplay( NumTeams, NumPlayers );
		PC.ServerRequestChangeTeam( NumPlayers );
	}

	return true;
}

/** Called when a player wants to download content */
function bool OnDownloadContent( UIScreenObject EventObject, int PlayerIndex )
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterfaceEx PlayerIntEx;
	local int ControllerId;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if ( OnlineSub != None )
	{
		PlayerIntEx = OnlineSub.PlayerInterfaceEx;
		if ( PlayerIntEx != None )
		{
			ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
			PlayerIntEx.ShowContentMarketplaceUI( ControllerId );
		}
	}

	return true;
}

/** Called when a player wants to leave the game */
function bool OnLeaveGame( UIScreenObject EventObject, int PlayerIndex )
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;
	local bool bCanLeave;

	// See if they can actually leave
	if (IsOfficialMatch())
	{
		bCanLeave = IsMatchHost(PlayerIndex);
	}
	else
	{
		bCanLeave = CanLeaveGame();
	}

	// If they can leave prompt them first
	if (bCanLeave)
	{
		ButtonAliases.AddItem('CancelStay');
		ButtonAliases.AddItem('AcceptLeave');
		GameSceneClient = GetSceneClient();
		GameSceneClient.ShowUIMessage('ConfirmQuit',
			"<Strings:GearGameUI.MessageBoxStrings.LeavePregameLobby_Title>",
			"<Strings:GearGameUI.MessageBoxStrings.LeavePregameLobby_" $ (IsPartyLeader(PlayerIndex) ? "Leader" : "Member") $ "Message>",
			"",
			ButtonAliases, OnLeaveLobby_Confirmed, GetPlayerOwner(PlayerIndex));
	}

	return false;
}

/**
 * Handler for the confirmation dialog's OnSelection delegate.  Leaves the lobby if the user confirmed.
 *
 * @param	Sender				the message box that generated this call
 * @param	SelectedInputAlias	the alias of the button that the user selected.  Should match one of the aliases passed into
 *								this message box.
 * @param	PlayerIndex			the index of the player that selected the option.
 *
 * @return	TRUE to indicate that the message box should close itself.
 */
function bool OnLeaveLobby_Confirmed( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	local bool bCanLeave;

	if ( SelectedInputAlias == 'AcceptLeave' )
	{
		// Make sure it's still ok for them to leave (time has passed since they first clicked!!!!
		if (IsOfficialMatch())
		{
			bCanLeave = IsMatchHost(PlayerIndex);
		}
		else
		{
			bCanLeave = CanLeaveGame();
		}

		if (bCanLeave)
		{
			// this call will trigger the loading movie
			LeaveMatch();
		}
	}

	return true;
}

/**
 * Responsible for hooking up all countdown delegates in the GearPreGameGRI class.  If the GRI hasn't been replicated yet, sets a timer in
 * the owning player's PlayerController to check again next tick.
 */
function SetGRICallbacks()
{
	local GearPC OwnerPC;
	local GearPreGameGRI PreGameGRI;

	OwnerPC = GetGearPlayerOwner(0);
	if ( OwnerPC.PlayerReplicationInfo != None )
	{
		PreGameGRI = GetPreGameGRI();
		if ( PreGameGRI != None )
		{
			PreGameGRI.OnGameVoteSubmitted = ReceivedGameVote;
			PreGameGRI.OnHostSelectedMapChanged = SelectedMapChangedByHost;
			PreGameGRI.OnMapVoteSubmitted = ReceivedMapVote;
			PreGameGRI.OnLobbyStateChanged = LobbyStateChanged;
			PreGameGRI.OnSelectionCountdownTick = SelectionCountdownTick;
			PreGameGRI.OnGameTypeWinnerDetermined = CloseGameVoting;
			PreGameGRI.OnMapWinnerDetermined = CloseMapSelection;
			PreGameGRI.OnHostStartedPregameCountdown = BeginPregameCountdown;
			PreGameGRI.OnMapSelectionUpdate = UpdateMapSelectionImageAndTitle;
			return;
		}
	}

	OwnerPC.SetTimer( 0.1, false, nameof(self.SetGRICallbacks), self );
}

/** Starts an async task to read map only DLC packages */
function ReadAvailableContent()
{
	local OnlineSubsystem OnlineSub;
	local int ControllerId;

	ControllerId = class'UIInteraction'.static.GetPlayerControllerId(GetBestPlayerIndex());

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None && OnlineSub.ContentInterface != None)
	{
		OnlineSub.ContentInterface.AddQueryAvailableDownloadsComplete(ControllerId,OnReadAvailableContentComplete);
		// NOTE: 2 is map DLC mask
		OnlineSub.ContentInterface.QueryAvailableDownloads(ControllerId,2);
	}
}

/**
 * Called when the content read has completed so we can enable/disable the DLC button
 * based upon whether there is new content to download
 *
 * @param bWasSuccessful whether the read worked or not
 */
function OnReadAvailableContentComplete(bool bWasSuccessful)
{
	local OnlineSubsystem OnlineSub;
	local int ControllerId;
	local int NewDownloads;
	local int TotalDownloads;

	ControllerId = class'UIInteraction'.static.GetPlayerControllerId(GetBestPlayerIndex());

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None && OnlineSub.ContentInterface != None)
	{
		OnlineSub.ContentInterface.ClearQueryAvailableDownloadsComplete(ControllerId,OnReadAvailableContentComplete);
		if (bWasSuccessful)
		{
			// Get the number of new items and enable the button based upon that
			OnlineSub.ContentInterface.GetAvailableDownloadCounts(ControllerId,NewDownloads,TotalDownloads);
		}
	}
	btnbarPlayers.EnableButton('DLC', GetPlayerOwnerIndex(), NewDownloads > 0, false);
}

/**
 * Clears all delegates which have been assigned to methods in this class so that those objects
 * don't hold a reference to this scene.
 */
function ClearDelegates()
{
	local CurrentGameDataStore CurrentGameDS;
	local GearPreGameGRI PreGameGRI;

	CurrentGameDS = GetCurrentGameDS();
	if (CurrentGameDS != None)
	{
		CurrentGameDS.OnAddTeamProvider = None;
	}

	PreGameGRI = GetPreGameGRI();
	PreGameGRI.OnGameVoteSubmitted = None;
	PreGameGRI.OnHostSelectedMapChanged = None;
	PreGameGRI.OnMapVoteSubmitted = None;
	PreGameGRI.OnLobbyStateChanged = None;
	PreGameGRI.OnSelectionCountdownTick = None;
	PreGameGRI.OnGameTypeWinnerDetermined = None;
	PreGameGRI.OnMapWinnerDetermined = None;
}

/** Accessor for determining whether the currently selected gametype is wingman */
final function bool IsWingmanGametype()
{
	local int ValueId;
	local OnlineGameSettings GameSettings;
	local GearPreGameGRI GRI;

	if ( IsOfficialMatch() )
	{
		GRI = GearPreGameGRI(GetGRI());
		if ( GRI != None )
		{
			return class'GearUIDataStore_GameResource'.static.PlaylistContainsGameType( GRI.PlaylistId, eGEARMP_Wingman );
		}
	}
	else
	{
		GameSettings = GetCurrentGameSettings();
		if ( GameSettings != None &&
			GameSettings.GetStringSettingValue(class'GearVersusGameSettings'.const.CONTEXT_VERSUSMODES, ValueId) &&
			ValueId == eGEARMP_Wingman )
		{
			return true;
		}
	}

	return false;
}

/** Returns the number of bots that the server has selected for this game */
final function int GetNumBots()
{
	local OnlineGameSettings GameSettings;
	local int IntValue;

	if (!IsOfficialMatch())
	{
		GameSettings = GetCurrentGameSettings();
		GameSettings.GetStringSettingValue(class'GearVersusGameSettings'.const.CONTEXT_NUMBOTS, IntValue);
	}
	else
	{
		IntValue = 0;
	}
	return IntValue;
}

/** Accessor for determining whether the currently selected gametype is guardian */
final function bool IsGuardianGametype()
{
	local int ValueId;
	local OnlineGameSettings GameSettings;

	if ( IsOfficialMatch() )
	{
		return (PublicMatchType == eGEARMP_KTL);
	}
	else
	{
		GameSettings = GetCurrentGameSettings();
		if ( GameSettings != None &&
			GameSettings.GetStringSettingValue(class'GearVersusGameSettings'.const.CONTEXT_VERSUSMODES, ValueId) &&
			ValueId == eGEARMP_KTL )
		{
			return true;
		}
	}

	return false;
}

/** Accessor for determining whether the currently selected gametype is horde */
final function bool IsHordeGametype()
{
	local int ValueId;
	local OnlineGameSettings GameSettings;
	local GearPreGameGRI GRI;

	if ( IsOfficialMatch() )
	{
		GRI = GearPreGameGRI(GetGRI());
		if ( GRI != None )
		{
			return class'GearUIDataStore_GameResource'.static.PlaylistContainsGameType( GRI.PlaylistId, eGEARMP_CombatTrials );
		}
	}
	else
	{
		GameSettings = GetCurrentGameSettings();
		if ( GameSettings != None &&
			GameSettings.GetStringSettingValue(class'GearVersusGameSettings'.const.CONTEXT_VERSUSMODES, ValueId) &&
			ValueId == eGEARMP_CombatTrials )
		{
			return true;
		}
	}

	return false;
}

/**
 * Wrapper for determining whether the match is configured as an official public match
 */
function bool IsOfficialMatch()
{
	local OnlineGameSettings CurrentSettings;
	local int SelectedMatchType;

	CurrentSettings = GetCurrentGameSettings();
	if ( CurrentSettings != None &&
		 CurrentSettings.GetStringSettingValue(class'GearProfileSettings'.const.VERSUS_MATCH_MODE, SelectedMatchType) &&
		 SelectedMatchType == eGVMT_Official )
	{
		return true;
	}

	return false;
}

/**
 * Wrapper for determining whether the match is configured as a private live match
 */
function bool IsCustomMatch()
{
	local OnlineGameSettings CurrentSettings;
	local int SelectedMatchType;

	CurrentSettings = GetCurrentGameSettings();
	if (CurrentSettings != None &&
		CurrentSettings.GetStringSettingValue(class'GearProfileSettings'.const.VERSUS_MATCH_MODE, SelectedMatchType) &&
		SelectedMatchType == eGVMT_Custom)
	{
		return true;
	}

	return false;
}

/** Whether players can change teams or not */
function bool PlayersCanChangeTeams()
{
	local GearPreGameGRI PreGameGRI;

	PreGameGRI = GearPreGameGRI(GetGRI());

	if ( !IsOfficialMatch() &&
		 PreGameGRI != None &&
		 PreGameGRI.bAllPlayerArePresentForDisplay &&
		 PreGameGRI.PreGameLobbyState > eGPGLSTATE_Initialize &&
		 PreGameGRI.PreGameLobbyState < eGPGLSTATE_PostCharacterSelect &&
		 !PreGameGRI.bHostTriggeredGameStart &&
		 !IsHordeGametype() )
	{
		return true;
	}

	return false;
}

/** Whether players can leave the game or not */
function bool CanLeaveGame()
{
	local GearPreGameGRI PreGameGRI;

	PreGameGRI = GearPreGameGRI(GetGRI());

	if ( !IsOfficialMatch() &&
		 PreGameGRI != None &&
		 PreGameGRI.bAllPlayerArePresentForDisplay &&
		 PreGameGRI.PreGameLobbyState > eGPGLSTATE_Initialize &&
		 PreGameGRI.PreGameLobbyState < eGPGLSTATE_PostCharacterSelect &&
		 !PreGameGRI.bHostTriggeredGameStart)
	{
		return true;
	}

	return false;
}

/** Refreshes the state of the button bar */
function RefreshButtonBar()
{
	local int PlayerIndex;

	if ( !IsOfficialMatch() )
	{
		PlayerIndex = GetPlayerOwnerIndex();
		btnbarPlayers.SetVisibility( true );
		btnbarPlayers.EnableButton('TeamChange', PlayerIndex, PlayersCanChangeTeams(), false);
		// The host can't return to lobby until we can switch teams (fixes clients softlock)
		btnbarPlayers.EnableButton('GenericBack', PlayerIndex, CanLeaveGame(), false);
	}
}

/** Update the chat icon opacities */
final function UpdateChatIcons()
{
	local int Idx, TeamIdx, NumTeams, GameIdx;
	local GearPRI CurrPRI;
	local bool bIsWingman;

	bIsWingman = IsWingmanGametype();
	GameIdx = bIsWingman ? ePGL_Wingman : ePGL_Normal;
	NumTeams = PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData.length;
	for ( TeamIdx = 0; TeamIdx < NumTeams; TeamIdx++ )
	{
		for ( Idx = 0; Idx < PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList.length; Idx++ )
		{
			PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[Idx].ChatIcon.Opacity = 0.0f;
			CurrPRI = PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[Idx].PlayerPRI;
			if ( CurrPRI != None )
			{
				if( CurrPRI.ChatFadeValue > 0 )
				{
					CurrPRI.ChatFadeValue = 1.0f - (GetWorldInfo().TimeSeconds - CurrPRI.TaccomChatFadeStart) / CurrPRI.ChatFadeTime;
					PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[Idx].ChatIcon.Opacity = CurrPRI.ChatFadeValue;
				}
			}
		}
	}
}

/** Updates the playerlist */
function UpdatePlayerButtons()
{
	local int GameIdx;
	local bool bIsWingman;

	// If there is no GRI, hide the player panel and exit
	if ( GetGRI() == None )
	{
		for ( GameIdx = 0; GameIdx < PGLPlayerData.GameTeamData.length; GameIdx++ )
		{
			PGLPlayerData.GameTeamData[GameIdx].PlayersPanel.SetVisibility( false );
		}
		PGLPlayerData.PlayerBackgroundImage.SetVisibility( false );
		return;
	}
	else
	{
		bIsWingman = IsWingmanGametype();

		for ( GameIdx = 0; GameIdx < PGLPlayerData.GameTeamData.length; GameIdx++ )
		{
			if ( GameIdx == ePGL_Wingman )
			{
				PGLPlayerData.GameTeamData[GameIdx].PlayersPanel.SetVisibility( bIsWingman );
			}
			else
			{
				PGLPlayerData.GameTeamData[GameIdx].PlayersPanel.SetVisibility( !bIsWingman );
			}
		}
		PGLPlayerData.PlayerBackgroundImage.SetVisibility( true );
	}

	SetPlayerListPRIs();
	RefreshPlayerWidgets();
}

/** Whether a PRI is ready to show data or not */
function bool PRIIsReadyForShow( PlayerReplicationInfo CurrPRI, int GameIdx )
{
	if ( CurrPRI != None &&
		 !CurrPRI.bIsInactive &&
		 CurrPRI.GetTeamNum() >= 0 &&
		 CurrPRI.GetTeamNum() < PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData.length )
	{
		return true;
	}
	return false;
}

/**
 * Puts players in the correct slots based on their team ids
 * Places PRIs on the proper teams in the case of a public match
 * Places PRIs on teams in a balanced fashion if the PRI is not already on a team
 */
function SetPlayerListPRIs()
{
	local int PRIIdx, TeamIdx, PlayerIdx, GameIdx, NumTeams, NumPlayers;
	local GearPreGameGRI MyGRI;
	local PlayerReplicationInfo CurrPRI;
	local bool bIsWingman;
	local int BotTeam;
	local array<int> NumPlayersOnTeam;
	local int NumBots;
	local int BotIdx;

	bIsWingman = IsWingmanGametype();
	GameIdx = bIsWingman ? ePGL_Wingman : ePGL_Normal;
	GetNumTeamsToDisplay( NumTeams, NumPlayers );

	// First loop through the lists and null the slots
	for ( TeamIdx = 0; TeamIdx < PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData.length; TeamIdx++ )
	{
		for ( PlayerIdx = 0; PlayerIdx < PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList.length; PlayerIdx++ )
		{
			PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[PlayerIdx].PlayerPRI = None;
			PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[PlayerIdx].bIsBot = false;
			PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[PlayerIdx].ParentButton.SetForcedNavigationTarget(UIFACE_Top, none);
			PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[PlayerIdx].ParentButton.SetForcedNavigationTarget(UIFACE_Bottom, none);
		}
	}

	MyGRI = GearPreGameGRI(GetGRI());
	if ( MyGRI != None )
	{
		for ( PRIIdx = 0; PRIIdx < MyGRI.PRIArray.length; PRIIdx++ )
		{
			CurrPRI = MyGRI.PRIArray[PRIIdx];
			if ( PRIIsReadyForShow(CurrPRI, GameIdx) )
			{
				for ( PlayerIdx = 0; PlayerIdx < PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[CurrPRI.GetTeamNum()].DataList.length; PlayerIdx++ )
				{
					if ( CurrPRI.GetTeamNum() >= NumTeams )
					{
						`log("ERROR: GearUISceneFELobby_PreGame:SetPlayerListPRIs - Illegal team id " $ CurrPRI.GetTeamNum() $ " for player " $ PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[CurrPRI.GetTeamNum()].DataList[PlayerIdx].PlayerPRI $ ":" $ " Max team size should be: " $ NumTeams );
					}
					if ( PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[CurrPRI.GetTeamNum()].DataList[PlayerIdx].PlayerPRI == None )
					{
						PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[CurrPRI.GetTeamNum()].DataList[PlayerIdx].PlayerPRI = GearPreGameLobbyPRI(CurrPRI);
						break;
					}
				}
			}
		}
	}

	// Place bots (attempts to mimic the AddBot code in GearGameMP_Base)
	if (!IsOfficialMatch() && !IsHordeGametype())
	{
		NumBots = GetNumBots();
		if (NumBots > 0)
		{
			NumPlayersOnTeam.length = NumTeams;
			// First grab all the team counts
			for (TeamIdx = 0; TeamIdx < NumTeams; TeamIdx++)
			{
				NumPlayersOnTeam[TeamIdx] = GetNumPlayersOnTeam(GameIdx, TeamIdx);
			}

			// Now place bots
			for (BotIdx = 0; BotIdx < NumBots; BotIdx++)
			{
				BotTeam = 0;
				// for wingman pick the first team with one player
				if (bIsWingman)
				{
					for (TeamIdx = 0; TeamIdx < NumTeams; TeamIdx++)
					{
						if (bIsWingman && NumPlayersOnTeam[TeamIdx] <= 1)
						{
							//`log(`showvar(BotIdx)@"moving to team"@`showvar(TeamIdx)@`showvar(bIsWingman));
							BotTeam = TeamIdx;
							break;
						}
					}
				}
				else
				{
					for (TeamIdx = 1; TeamIdx < NumTeams; TeamIdx++)
					{
						if (NumPlayersOnTeam[BotTeam] > NumPlayersOnTeam[TeamIdx])
						{
							BotTeam = TeamIdx;
						}
					}
				}
				// Find first spot on team for this bot
				if (NumPlayersOnTeam[BotTeam] < PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[BotTeam].DataList.length)
				{
					for (PlayerIdx = 0; PlayerIdx < PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[BotTeam].DataList.length; PlayerIdx++)
					{
						if (PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[BotTeam].DataList[PlayerIdx].PlayerPRI == none &&
							!PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[BotTeam].DataList[PlayerIdx].bIsBot)
						{
							PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[BotTeam].DataList[PlayerIdx].bIsBot = true;
							NumPlayersOnTeam[BotTeam]++;
							break;
						}
					}
				}
			}
		}
	}
}

/** Get the number of players on a team */
function int GetNumPlayersOnTeam(int GameIdx, int TeamIdx, optional bool bIncludeBots)
{
	local int Count;
	local int PlayerIdx;

	for (PlayerIdx = 0; PlayerIdx < PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList.length; PlayerIdx++)
	{
		if (PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[PlayerIdx].PlayerPRI != none)
		{
			Count++;
		}
		else if (bIncludeBots && PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[PlayerIdx].bIsBot)
		{
			Count++;
		}
	}
	return Count;
}

/** Returns whether there are at least 2 teams available for starting the match */
function bool AtLeastTwoTeamsAreAvailable()
{
	local int NumTeams;
	local int NumPlayers;
	local int TeamIdx;
	local int ValidTeamCount;
	local bool bIsWingman;
	local int GameIdx;

	bIsWingman = IsWingmanGametype();
	GameIdx = bIsWingman ? ePGL_Wingman : ePGL_Normal;

	GetNumTeamsToDisplay( NumTeams, NumPlayers );
	for (TeamIdx = 0; TeamIdx < NumTeams; TeamIdx++)
	{
		if (GetNumPlayersOnTeam(GameIdx, TeamIdx, true) > 0)
		{
			ValidTeamCount++;
			if (ValidTeamCount > 1)
			{
				return true;
			}
		}
	}
	return false;
}

/** Refreshes the player data widgets */
function RefreshPlayerWidgets()
{
	local int TeamIdx, PlayerIdx, GameIdx, NumTeams, NumPlayers;
	local GearPreGameGRI PreGameGRI;
	local UILabelButton LastVisibleButton;
	local bool bAPlayerButtonHasFocus, bIsWingman;
	local UILabelButton FirstEnabledButton;
	local UILabelButton LastEnabledButton;
	local bool bPlayerIsValid;
	local UILabelButton PrevEnabledButton;

	PrevEnabledButton = none;
	PreGameGRI = GearPreGameGRI(GetGRI());
	bIsWingman = IsWingmanGametype();
	GameIdx = bIsWingman ? ePGL_Wingman : ePGL_Normal;
	GetNumTeamsToDisplay( NumTeams, NumPlayers );

	// Loop through the teams hiding/showing the proper teams based on the gametype and playlist
	for ( TeamIdx = 0; TeamIdx < PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData.length; TeamIdx++ )
	{
		if ( TeamIdx < NumTeams )
		{
			PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].ParentPanel.SetVisibility( true );
		}
		else
		{
			PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].ParentPanel.SetVisibility( false );
		}
	}

	bAPlayerButtonHasFocus = false;
	for ( TeamIdx = 0; TeamIdx < PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData.length; TeamIdx++ )
	{
		// No need to refresh widgets when the panel is not visible due to the code above
		if ( TeamIdx < NumTeams )
		{
			for ( PlayerIdx = 0; PlayerIdx < PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList.length; PlayerIdx++ )
			{
				if ( PlayerIdx < NumPlayers )
				{
					// Grab the last button in the list
					LastVisibleButton = PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[PlayerIdx].ParentButton;
					LastVisibleButton.SetVisibility( true );
					bPlayerIsValid = RefreshPlayerName( GameIdx, TeamIdx, PlayerIdx );
					RefreshPlayerProfileIcon( GameIdx, TeamIdx, PlayerIdx );
					RefreshPlayerRankIcon( GameIdx, TeamIdx, PlayerIdx );

					if (bPlayerIsValid)
					{
						LastEnabledButton = LastVisibleButton;
						if (FirstEnabledButton == none)
						{
							FirstEnabledButton = LastEnabledButton;
						}

						if (PrevEnabledButton != none)
						{
							PrevEnabledButton.SetForcedNavigationTarget(UIFACE_Bottom, LastEnabledButton);
							LastEnabledButton.SetForcedNavigationTarget(UIFACE_Top, PrevEnabledButton);
						}
						PrevEnabledButton = LastEnabledButton;
					}

					// See if this player button has focus
					if ( PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[PlayerIdx].PlayerPRI != None &&
						 (LastVisibleButton.IsFocused(GetPlayerOwnerIndex())))
					{
						bAPlayerButtonHasFocus = true;
					}
				}
				else
				{
					PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[PlayerIdx].ParentButton.SetVisibility( false );
				}
			}
		}
	}

	// Set the navigation
	if (FirstEnabledButton != none && LastEnabledButton != none && FirstEnabledButton != LastEnabledButton)
	{
		FirstEnabledButton.SetForcedNavigationTarget(UIFACE_Top, LastEnabledButton);
		LastEnabledButton.SetForcedNavigationTarget(UIFACE_Bottom, FirstEnabledButton);
	}

	if (IsSceneActive(true))
	{
		// Previous frame the players were not ready, which means nothing had focus, so give focus to the first button in the list
		if ( (PreGameGRI.bAllPlayerArePresentForDisplay && !bPlayerDataReadyForShow) ||
			 (!bAPlayerButtonHasFocus && PreGameGRI.bAllPlayerArePresentForDisplay) )
		{
			// Find the first button that has a player to give focus to
			for ( TeamIdx = 0; TeamIdx < PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData.length; TeamIdx++ )
			{
				for ( PlayerIdx = 0; PlayerIdx < PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList.length; PlayerIdx++ )
				{
					if ( PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[PlayerIdx].PlayerPRI != None )
					{
						PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[PlayerIdx].ParentButton.SetFocus(None,GetPlayerOwnerIndex());
						bAPlayerButtonHasFocus = true;
						break;
					}
				}
				if ( bAPlayerButtonHasFocus )
				{
					break;
				}
			}
		}
		bPlayerDataReadyForShow = PreGameGRI.bAllPlayerArePresentForDisplay;
	}

	if ( LastVisibleButton != None )
	{
		// Dock the background image to the last button
		PGLPlayerData.PlayerBackgroundImage.SetDockTarget( UIFACE_Bottom, LastVisibleButton, UIFACE_Bottom );
	}
}

/** Whether a player should be showing the error background or not */
function bool PlayerShouldDisplayError( GearPreGameLobbyPRI PlayerPRI, string MapName)
{
	local GearPreGameGRI PreGameGRI;

	PreGameGRI = GearPreGameGRI(GetGRI());

	if ( PreGameGRI != None && !PlayerHasRequiredDLCForMap(PlayerPRI, MapName) )
	{
		return true;
	}

	return false;
}

/**
 * Whether the player has the required DLCs needed for the selected map or not
 *
 * @param PlayerPRI - the PRI of the player in question
 * @param MapName - the name of the map we are checking for content on
 * @param bUseStrictCheck - used when proper replication has not taken place yet
 *		(this is useful for when you MUST know if the player has DLC, for normal user feedback we won't care about
 *		 this but for a button click that will require a proper check, we must use it)
 */
function bool PlayerHasRequiredDLCForMap( GearPRI PlayerPRI, string MapName, optional bool bUseStrictCheck )
{
	local GearPreGameGRI GRI;
	local GearGameMapSummary MapData;

	GRI = GearPreGameGRI(GetGRI());

	if (GRI != None)
	{
		// Testing against both vote enabled flags lets us know that this is a custom match where the host selects the map
		if (!GRI.bMapVoteEnabled && !GRI.bGameVoteEnabled)
		{
			if (PlayerPRI != None && !PlayerPRI.bIsInactive)
			{
				MapData = class'GearUIDataStore_GameResource'.static.GetMapSummaryFromMapName( MapName );
				if (MapData != None)
				{
					return GRI.PlayerHasRequiredContentForMap(MapData, PlayerPRI, bUseStrictCheck);
				}
				else
				{
					return !PRIIsLocalPlayer(PlayerPRI);
				}
			}
		}
	}
	else
	{
		return !bUseStrictCheck;
	}

	return true;
}

/** Whether a PRI is a local player or not */
function bool PRIIsLocalPlayer( GearPRI PlayerPRI )
{
	local LocalPlayer LP;
	local GearPC MyGearPC;
	local PlayerReplicationInfo MyPRI;
	local int Idx;

	for ( Idx = 0; Idx < 2; Idx++ )
	{
		LP = GetPlayerOwner(Idx);
		if ( LP != None )
		{
			MyGearPC = GearPC(LP.Actor);
			MyPRI = MyGearPC.PlayerReplicationInfo;
			if ( MyPRI == PlayerPRI )
			{
				return true;
			}
		}
	}
	return false;
}

/** Refresh the string drawn in the player button - return whether there was a valid PRI for this player */
function bool RefreshPlayerName( int GameIdx, int TeamIdx, int PlayerIdx )
{
	local GearPreGameLobbyPRI CurrPRI;
	local GearPreGameGRI PreGameGRI;
	local Name NormalStyle, LoadStyle;
	local String DotsString;
	local int DotIdx, TotalDots;
	local bool bIsBot;
	local bool bResult;
	local UILabelButton CurrParentButton;
	local int PlayerIndex;

	PlayerIndex = GetPlayerOwnerIndex();
	PreGameGRI = GearPreGameGRI(GetGRI());
	CurrPRI = PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[PlayerIdx].PlayerPRI;
	bIsBot = PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[PlayerIdx].bIsBot;
	if ( (TeamIdx % 2) == 0 )
	{
		NormalStyle = 'CogPlayersNormal';
		LoadStyle = 'CogPlayerConnecting';
	}
	else
	{
		NormalStyle = 'LocustPlayersNormal';
		LoadStyle = 'LocustPlayerConnecting';
	}

	// The screen first wait for everyone to have joined so display "LOADING" until it's ready
	if ( !PreGameGRI.bAllPlayerArePresentForDisplay )
	{
		DotsString = "";
		TotalDots = PreGameGRI.WorldInfo.TimeSeconds % 4;
		for ( DotIdx = 0; DotIdx < TotalDots; DotIdx++ )
		{
			DotsString $= ".";
		}
		CurrParentButton = PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[PlayerIdx].ParentButton;
		CurrParentButton.SetDataStoreBinding(PlayerLoading $ DotsString);
		CurrParentButton.SetWidgetStyleByName('Background Image Style', LoadStyle);
		if (CurrParentButton.IsFocused(PlayerIndex))
		{
			CurrParentButton.KillFocus(none, PlayerIndex);
		}
		CurrParentButton.DisableWidget(PlayerIndex);
	}
	else
	{
		// We have a PRI so display the name
		if ( PRIIsReadyForShow(CurrPRI, GameIdx) )
		{
			CurrParentButton = PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[PlayerIdx].ParentButton;
			CurrParentButton.SetDataStoreBinding(CurrPRI.PlayerName);
			CurrParentButton.EnableWidget(PlayerIndex);

			// Testing against both vote enabled flags lets us know that this is a custom match where the host selects the map
			if ( !PreGameGRI.bGameVoteEnabled && !PreGameGRI.bMapVoteEnabled && PlayerShouldDisplayError(CurrPRI, PreGameGRI.SelectedMapName) )
			{
				CurrParentButton.SetWidgetStyleByName( 'Background Image Style', 'PartyPlayerError' );
			}
			// If a player is requesting a team change
			else if ( CurrPRI.bRequestedTeamChange )
			{
				CurrParentButton.SetWidgetStyleByName( 'Background Image Style', 'PartyTeamSwap' );
			}
			else
			{
				CurrParentButton.SetWidgetStyleByName( 'Background Image Style', NormalStyle );
			}
			bResult = true;
		}
		// Handle bots
		else if (bIsBot)
		{
			CurrParentButton = PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[PlayerIdx].ParentButton;
			CurrParentButton.SetDataStoreBinding(BotSlotString);
			if (CurrParentButton.IsFocused(PlayerIndex))
			{
				CurrParentButton.KillFocus(none,PlayerIndex);
			}
			CurrParentButton.DisableWidget(PlayerIndex);
			CurrParentButton.SetWidgetStyleByName('Background Image Style', NormalStyle);
		}
		// No PRI so display empty slot
		else
		{
			CurrParentButton = PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[PlayerIdx].ParentButton;
			CurrParentButton.SetDataStoreBinding(PlayerEmpty);
			if (CurrParentButton.IsFocused(PlayerIndex))
			{
				CurrParentButton.KillFocus(none,PlayerIndex);
			}
			CurrParentButton.DisableWidget(PlayerIndex);
			CurrParentButton.SetWidgetStyleByName('Background Image Style', NormalStyle);
		}
	}

	return bResult;
}

/** Refresh the lable on the far left that shows either the A button or the profile icon */
function RefreshPlayerProfileIcon( int GameIdx, int TeamIdx, int PlayerIdx )
{
	local PlayerController PC;
	local string ControllerString;
	local LocalPlayer LP;
	local GearPreGameLobbyPRI CurrPRI;
	local GearPreGameGRI PreGameGRI;

	ControllerString = " ";
	CurrPRI = PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[PlayerIdx].PlayerPRI;
	PreGameGRI = GearPreGameGRI(GetGRI());

	// If a player is requesting a team change
	if ( PRIIsReadyForShow(CurrPRI, GameIdx) && CurrPRI.bRequestedTeamChange )
	{
		ControllerString = "s";
	}
	else if ( PreGameGRI != None && PreGameGRI.bAllPlayerArePresentForDisplay && PRIIsReadyForShow(CurrPRI, GameIdx) )
	{
		PC = PlayerController(CurrPRI.Owner);
		if ( PC != None && PC.IsLocalPlayerController() )
		{
			LP = LocalPlayer(PC.Player);
			if ( LP != None )
			{
				ControllerString = GetControllerIconString( LP.ControllerId );
			}
		}
	}

	PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[PlayerIdx].ProfileLabel.SetDataStoreBinding( ControllerString );
}

/** Refresh the player rank icon */
function RefreshPlayerRankIcon( int GameIdx, int TeamIdx, int PlayerIdx )
{
	local string RankString;
	local GearPreGameLobbyPRI CurrPRI;
	local GearPreGameGRI PreGameGRI;

	RankString = " ";
	CurrPRI = PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[PlayerIdx].PlayerPRI;
	PreGameGRI = GearPreGameGRI(GetGRI());

	if ( PreGameGRI != None && PreGameGRI.bAllPlayerArePresentForDisplay && PRIIsReadyForShow(CurrPRI, GameIdx) )
	{
		RankString = GetPlayerSkillString( CurrPRI.PlayerSkill );
	}

	PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[PlayerIdx].RankLabel.SetDataStoreBinding( RankString );
}

/**
 * Handler for the scene's OnSceneActivated delegate.  Called after scene focus has been initialized, or when the scene becomes active as
 * the result of another scene closing.
 *
 * @param	ActivatedScene			the scene that was activated
 * @param	bInitialActivation		TRUE if this is the first time this scene is being activated; FALSE if this scene has become active
 *									as a result of closing another scene or manually moving this scene in the stack.
 */
function SceneActivationComplete( UIScene ActivatedScene, bool bInitialActivation )
{
	if ( !IsEditor() )
	{
		if ( ActivatedScene == self && bInitialActivation )
		{
			// Set the initial lobby state
			CurrPGLobbyState = eGPGLSTATE_Initialize;

			InitializeLobbyScene();

			SetGRICallbacks();

			// Have all players send their DLC values
			UpdateLocalPlayersDLCValues();

			// Turn chat delegates on
			TrackChat( true );
		}

		ReadAvailableContent();
	}
}

/** Called just after this scene is removed from the active scenes array */
event SceneDeactivated()
{
	Super.SceneDeactivated();

	ClearDelegates();

	// Turn chat delegates off
	TrackChat( false );
}

/**
 * Wrapper for checking whether the owning player is the match host.
 */
function bool IsMatchHost( int PlayerIndex=GetBestPlayerIndex() )
{
	if ( GetPreGameGRI().Role == ROLE_Authority &&
		 PlayerIndex == 0 )
	{
		return true;
	}

	return false;
}

/** Called when any change in the vote count for a game is changed */
function ReceivedGameVote()
{
	local int Idx;

	for ( Idx = 0; Idx < PGLGameData.GameVoteData.length; Idx++ )
	{
		PGLGameData.GameVoteData[Idx].GameVoteTallyLabel.RefreshSubscriberValue();
	}
}

/** Called when any change in the vote count for a map is changed */
function ReceivedMapVote()
{
	local int Idx;

	if ( GearPreGameGRI(GetGRI()).PreGameLobbyState == eGPGLSTATE_MapSelection )
	{
		for ( Idx = 0; Idx < PGLMapData.MapVoteData.length; Idx++ )
		{
			PGLMapData.MapVoteData[Idx].MapVoteLabel.RefreshSubscriberValue();
		}

		// Update the map image since the vote will effect the voting mask
		UpdateMapVoteImage();
	}
}

/** Called when an updated value is received for HostSelectedMapIndex */
function SelectedMapChangedByHost()
{
	UpdateMapSelectionImageAndTitle(GetPreGameGRI().bSuspendMapSelection);
}

/** Called when any of the 3 selection countdowns are changed */
function SelectionCountdownTick()
{
	UpdateCountdown();
}

/** Called when the pre-game lobby state changes */
function LobbyStateChanged( EGearPGLState NewLobbyState )
{
	switch ( NewLobbyState )
	{
		case eGPGLSTATE_GameVote:
			TransitionToBeginGameVote();
			break;
		case eGPGLSTATE_MapSelection:
			TransitionToMapSelection();
			break;
		case eGPGLSTATE_CharacterSelect:
			TransitionToCharacterSelection();
			break;
		case eGPGLSTATE_PostCharacterSelect:
			EndCharacterSelection();
			break;
		case eGPGLSTATE_StartingMatch:
			BeginMatch();
			break;
	}

	CurrPGLobbyState = NewLobbyState;
}


/** Play animation to transition to begin voting */
function TransitionToBeginGameVote()
{
	PGLGameData.InactivePanel.PlayUIAnimation( '', GameTypeExpand, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
	PlayUISound(VoteOpenCue);

	OnTimerTriggered = BeginGameVote;
	WaitForTimer(0.5);

}


/** Play animation that transitions to character selection */
function TransitionToCharacterSelection()
{
	PGLCharacterData.InactivePanel.PlayUIAnimation( '', CharacterSelectExpand, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
	PlayUISound(VoteOpenCue);

	OnTimerTriggered = BeginCharacterSelection;
	WaitForTimer(0.35);
}

/** Play an animation; after some time pop up the map selection */
function TransitionToMapSelection()
{
	PGLGameData.InactivePanel.PlayUIAnimation( '', GameTypeExpand, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
	PGLMapData.InactivePanel.PlayUIAnimation( '', MapSelectionExpand, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
	PlayUISound(VoteOpenCue);
	
	OnTimerTriggered = OnEndTransitionToMapSelection;
	WaitForTimer(0.35);
}

/** Called when man selection is about to begin */
function OnEndTransitionToMapSelection()
{
	PlayUISound(VoteOpenPulseCue);
	BeginMapSelection( GetPreGameGRI().bMapVoteEnabled );
}


/** Sets the scene up to show all of the selections collapsed */
function InitializeLobbyScene()
{
	// GAME SELECT SECTION
	PGLGameData.DescribePanel.SetVisibility(false);
	PGLGameData.VotingPanel.SetVisibility(false);
	PGLGameData.InactivePanel.SetVisibility(true);

	// MAP SELECT SECTION
	PGLMapData.DescribePanel.SetVisibility(false);
	PGLMapData.VotingPanel.SetVisibility(false);
	PGLMapData.SelectPanel.SetVisibility(false);
	PGLMapData.InactivePanel.SetVisibility(true);
	PGLMapData.InactivePanel.SetDockTarget( UIFACE_Top, PGLGameData.InactivePanel, UIFACE_Bottom );

	// CHARACTER SELECT SECTION
	PGLCharacterData.SelectionData[0].CharacterPanel.SetVisibility(false);
	PGLCharacterData.SelectionData[1].CharacterPanel.SetVisibility(false);
	PGLCharacterData.InactivePanel.SetVisibility(true);
	PGLCharacterData.InactivePanel.SetDockTarget( UIFACE_Top, PGLMapData.InactivePanel, UIFACE_Bottom );
}

/** Returns the current opacity for all pulsing widgets */
function float GetPulseOpacity()
{
	local float CurrOpacity;

	CurrOpacity = GetPreGameGRI().WorldInfo.TimeSeconds - LastPulseTime;
	CurrOpacity = (CurrOpacity > 1.0f) ? 1.0f : CurrOpacity;
	CurrOpacity = 0.7f + ((1.0f - CurrOpacity) * 0.3f);
	return CurrOpacity;
}

/** Build countdown string */
function string BuildCountdownString( int Countdown )
{
	local string CountdownString;
	CountdownString = "00:";
	CountdownString $= (Countdown < 10) ? "0" : "";
	CountdownString $= Countdown;
	return CountdownString;
}

/** Update the pulsing of the voting background and the countdown */
function UpdateGameTypeCountdownAndPulsing( int Countdown )
{
	local float CurrOpacity;
	local string CountdownString;
	local bool bShouldPlayBeep, bFinalBeep;

	CurrOpacity = GetPulseOpacity();
	CountdownString = BuildCountdownString(Countdown);
	bShouldPlayBeep = bPlayCountdownBeep && Countdown <= 5;
	bFinalBeep = bShouldPlayBeep && CountdownString == PGLGameData.GameVoteTimerLabel.GetDataStoreBinding();

	PGLGameData.GameVoteTimerLabel.SetDataStoreBinding( CountdownString );
	PGLGameData.GameVoteTimerLabel.Opacity = CurrOpacity;
	PGLGameData.GameVoteTimerLabel.SetVisibility(true);
	PGLGameData.VotingImage.Opacity = CurrOpacity;
	//PGLGameData.VotingImage.SetVisibility(true);

	if ( bShouldPlayBeep )
	{
		if ( bFinalBeep )
		{
			PlayUISound('VoteCountdownComplete');
			bPlayCountdownBeep = false;
		}
		else
		{
			PlayUISound('VoteCountdownWarning');
		}
	}
}

/**
 * Sets the widget data sources for a specific game-type
 *
 * @param VoteIdx - index of the game-type being voted on (0 or 1)
 * @param GameIdx - index of the game-type these widgets should set themselves to
 */
function SetGameTypeNameAndIcon( int VoteIdx, int GameIdx )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local string GameName;
	local GearGamePlaylistGameTypeProvider PlaylistGameProvider;
	local GearGameInfoSummary GameInfoProvider;

	GameName = "";

	GameResourceDS = GetGameResourceDataStore();
	if ( GameResourceDS != None )
	{
		// Get the playlist game mode provider
		PlaylistGameProvider = class'GearUIDataStore_GameResource'.static.GetPlaylistGameTypeProvider(GameIdx);
		if ( PlaylistGameProvider != None )
		{
			// Get the gameinfo provider
			GameInfoProvider = class'GearUIDataStore_GameResource'.static.GetGameTypeProvider(PlaylistGameProvider.MPGameMode);
			if ( GameInfoProvider != None )
			{
				// First try and get the string from the playlist gamemode provider
				GameName = PlaylistGameProvider.DisplayName;

				// Wasn't found so grab it from the game info it uses
				if ( GameName == "" )
				{
					GameName = GameInfoProvider.GameName;
				}

				// Set the name to the widget
				PGLGameData.GameVoteData[VoteIdx].GameVoteTitleLabel.SetDataStoreBinding( GameName );

				// Set the image to the widget
				PGLGameData.GameVoteData[VoteIdx].GameVoteImage.SetDataStoreBinding( GameInfoProvider.IconPathName );
			}
		}
	}
}

/** Updates the backgrounds of the game being voted on */
function UpdateGameTypeVoteBackgrounds()
{
	local float CurrOpacity;
	local GearPreGameGRI PreGameGRI;

	PreGameGRI = GetPreGameGRI();
	CurrOpacity = GetPulseOpacity();

	if ( PreGameGRI.FirstGameVoteCount > PreGameGRI.SecondGameVoteCount )
	{
		PGLGameData.GameVoteData[0].GameVoteWinningBGImage.Opacity = CurrOpacity;
		PGLGameData.GameVoteData[0].GameVoteWinningBGImage.SetVisibility(true);
		PGLGameData.GameVoteData[1].GameVoteWinningBGImage.SetVisibility(false);
	}
	else if ( PreGameGRI.FirstGameVoteCount < PreGameGRI.SecondGameVoteCount )
	{
		PGLGameData.GameVoteData[1].GameVoteWinningBGImage.Opacity = CurrOpacity;
		PGLGameData.GameVoteData[1].GameVoteWinningBGImage.SetVisibility(true);
		PGLGameData.GameVoteData[0].GameVoteWinningBGImage.SetVisibility(false);
	}
	else
	{
		PGLGameData.GameVoteData[0].GameVoteWinningBGImage.SetVisibility(false);
		PGLGameData.GameVoteData[1].GameVoteWinningBGImage.SetVisibility(false);
	}
}

/** Initialize the widgets for a game-type vote */
function InitializeGameTypeVote()
{
	local GearPreGameGRI PreGameGRI;
	local int Idx, GameIdx;
	local string DataMarkup;

	PreGameGRI = GetPreGameGRI();

	// setup the game-types being voted on
	for ( Idx = 0; Idx < PGLGameData.GameVoteData.length; Idx++ )
	{
		GameIdx = (Idx == 0) ? PreGameGRI.FirstGameIndex : PreGameGRI.SecondGameIndex;
		SetGameTypeNameAndIcon( Idx, GameIdx );

		DataMarkup = (Idx == 0) ? "[<CurrentGame:FirstGameVoteCount>]" : "[<CurrentGame:SecondGameVoteCount>]";
		PGLGameData.GameVoteData[Idx].GameVoteTallyLabel.SetDataStoreBinding( DataMarkup );
	}
	UpdateGameTypeVoteBackgrounds();

	// setup the generic game-type voting widgets
	UpdateGameTypeCountdownAndPulsing( PreGameGRI.RemainingGameVoteSeconds );
}

/** Initialize the widgets for the game-type description */
function InitializeGameTypeDescription()
{
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue GameResourceData;
	local string GameName, GameDescription, ImagePath, TieString;
	local GearGamePlaylistGameTypeProvider PlaylistGameProvider;
	local GearGameInfoSummary GameInfoProvider;
	local GearPreGameGRI PreGameGRI;

	GameName = "";
	GameDescription = "";
	ImagePath = "";

	GameResourceDS = GetGameResourceDataStore();
	PreGameGRI = GetPreGameGRI();

	if ( GameResourceDS != None )
	{
		// If this was a vote we will first try and get the name and description from the playlist gamemode provider
		if ( PreGameGRI.bGameVoteEnabled )
		{
			PlaylistGameProvider = class'GearUIDataStore_GameResource'.static.GetPlaylistGameTypeProvider( PreGameGRI.WinningGameIndex );
			if ( PlaylistGameProvider != None )
			{
				GameInfoProvider = class'GearUIDataStore_GameResource'.static.GetGameTypeProvider( PlaylistGameProvider.MPGameMode );
				if ( GameInfoProvider != None )
				{
					// Cache the game type
					PublicMatchType = GameInfoProvider.MPGameMode;

					// Attempt to get the name from the playlist gamemode provider
					GameName = PlaylistGameProvider.DisplayName;
					if ( GameName == "" )
					{
						// Didn't find it so grab it from the game info the playlist references
						GameName = GameInfoProvider.GameName;
					}

					// Attempt to get the description from the playlist gamemode provider
					GameDescription = PlaylistGameProvider.Description;
					if ( GameDescription == "" )
					{
						// Didn't find it so grab it from the game info the playlist references
						GameDescription = GameInfoProvider.Description;
					}

					// Get the icon path from the gameinfo the playlist references
					ImagePath = GameInfoProvider.IconPathName;
				}
			}
		}
		// Not a vote so just grab the data from the gametypes provider
		else
		{
			if ( GameResourceDS.GetProviderFieldValue('GameTypes', 'GameName', PreGameGRI.WinningGameIndex, GameResourceData) )
			{
				GameName = GameResourceData.StringValue;
			}

			if ( GameResourceDS.GetProviderFieldValue('GameTypes', 'IconPathName', PreGameGRI.WinningGameIndex, GameResourceData) )
			{
				ImagePath = GameResourceData.StringValue;
			}

			if ( GameResourceDS.GetProviderFieldValue('GameTypes', 'Description', PreGameGRI.WinningGameIndex, GameResourceData) )
			{
				GameDescription = GameResourceData.StringValue;
			}
		}
	}

	TieString = "";
	if ( PreGameGRI.bGameVoteEnabled &&
		 (PreGameGRI.FirstGameVoteCount == PreGameGRI.SecondGameVoteCount) &&
		 (PreGameGRI.FirstGameIndex != PreGameGRI.SecondGameIndex) )
	{
		TieString = " " $ TiedVote;
	}
	PGLGameData.DescribeTitleLabel.SetDataStoreBinding( GameName $ TieString );
	PGLGameData.DescribeImage.SetDataStoreBinding( ImagePath );
	PGLGameData.DescribeDescriptionLabel.SetDataStoreBinding( GameDescription );
}

/** Update the widgets for a game-type vote */
function UpdateGameTypeVote()
{
	UpdateGameTypeVoteBackgrounds();
	UpdateGameTypeCountdownAndPulsing( GetPreGameGRI().RemainingGameVoteSeconds );
}

/** Sets the scene up for voting on the gametype */
function BeginGameVote()
{
	// GAME SELECT SECTION
	PGLGameData.DescribePanel.SetVisibility(false);
	PGLGameData.VotingPanel.SetVisibility(true);
	PGLGameData.VotingPanel.PlayUIAnimation( '', FadeIn, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
	PGLGameData.VotingImage.SetVisibility(true);
	PGLGameData.InactivePanel.SetVisibility(false);
	//PGLGameData.InactivePanel.PlayUIAnimation( '', FadeOut, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
	InitializeGameTypeVote();

	// MAP SELECT SECTION
	PGLMapData.DescribePanel.SetVisibility(false);
	PGLMapData.VotingPanel.SetVisibility(false);
	PGLMapData.SelectPanel.SetVisibility(false);
	//PGLMapData.InactivePanel.SetVisibility(true);
	PGLMapData.InactivePanel.SetDockTarget( UIFACE_Top, PGLGameData.VotingPanel, UIFACE_Bottom );

	// CHARACTER SELECT SECTION
	PGLCharacterData.SelectionData[0].CharacterPanel.SetVisibility(false);
	PGLCharacterData.SelectionData[1].CharacterPanel.SetVisibility(false);
	PGLCharacterData.InactivePanel.SetVisibility(true);
	PGLCharacterData.InactivePanel.SetDockTarget( UIFACE_Top, PGLMapData.InactivePanel, UIFACE_Bottom );

	// Tooltip hack
	PGLGameData.TooltipBGImage.SetVisibility(true);
	PGLGameData.TooltipPanel.SetVisibility(true);
	PGLGameData.TooltipLabel.SetValue(TooltipGametypeVoting);
	PGLGameData.TooltipPanel.PlayUIAnimation( '', FadeIn, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );

	PlayUISound(VoteOpenPulseCue);
}

/** Closes voting for games (will eventually animate) */
function CloseGameVoting()
{
	// GAME SELECTION SECTION
	InitializeGameTypeDescription();
	PGLGameData.VotingPanel.SetVisibility(false);
	PGLGameData.DescribePanel.SetVisibility(true);
	PGLGameData.DescribePanel.PlayUIAnimation( '', FadeIn, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );

	PlayUISound(VoteCompleteCue);


	HideTooltip();

	OnTimerTriggered = CloseGameVotingComplete;
	WaitForTimer(0.3);
}

/** Changes the game vote section to represent the winning gametype */
function CloseGameVotingComplete()
{
	// GAME SELECT SECTION
	PGLGameData.InactivePanel.SetVisibility(false);
	PGLGameData.VotingImage.SetVisibility(false);
	
	// MAP SELECT SECTION
	PGLMapData.DescribePanel.SetVisibility(false);
	PGLMapData.VotingPanel.SetVisibility(false);
	PGLMapData.SelectPanel.SetVisibility(false);
	PGLMapData.InactivePanel.SetVisibility(true);
	PGLMapData.InactivePanel.SetDockTarget( UIFACE_Top, PGLGameData.DescribePanel, UIFACE_Bottom );

	// CHARACTER SELECT SECTION
	PGLCharacterData.SelectionData[0].CharacterPanel.SetVisibility(false);
	PGLCharacterData.SelectionData[1].CharacterPanel.SetVisibility(false);
	PGLCharacterData.InactivePanel.SetVisibility(true);
	PGLCharacterData.InactivePanel.SetDockTarget( UIFACE_Top, PGLMapData.InactivePanel, UIFACE_Bottom );

	PlayUISound(VoteOpenPulseCue);
}

/** Update the widgets for a map vote */
function UpdateMapVote()
{
	UpdateMapVoteBackgrounds();
	UpdateMapVoteCountdownAndPulsing( GetPreGameGRI().RemainingMapVoteSeconds );
}

/** Updates the backgrounds of the map being voted on */
function UpdateMapVoteBackgrounds()
{
	local float CurrOpacity;
	local GearPreGameGRI PreGameGRI;

	PreGameGRI = GetPreGameGRI();
	CurrOpacity = GetPulseOpacity();

	if ( PreGameGRI.FirstMapVoteCount > PreGameGRI.SecondMapVoteCount )
	{
		PGLMapData.MapVoteData[0].MapVoteWinningBGImage.Opacity = CurrOpacity;
		PGLMapData.MapVoteData[0].MapVoteWinningBGImage.SetVisibility(true);
		PGLMapData.MapVoteData[1].MapVoteWinningBGImage.SetVisibility(false);
	}
	else if ( PreGameGRI.FirstMapVoteCount < PreGameGRI.SecondMapVoteCount )
	{
		PGLMapData.MapVoteData[1].MapVoteWinningBGImage.Opacity = CurrOpacity;
		PGLMapData.MapVoteData[1].MapVoteWinningBGImage.SetVisibility(true);
		PGLMapData.MapVoteData[0].MapVoteWinningBGImage.SetVisibility(false);
	}
	else
	{
		PGLMapData.MapVoteData[0].MapVoteWinningBGImage.SetVisibility(false);
		PGLMapData.MapVoteData[1].MapVoteWinningBGImage.SetVisibility(false);
	}
}

/** Update the pulsing of the voting background and the countdown */
function UpdateMapVoteCountdownAndPulsing( int Countdown )
{
	local float CurrOpacity;
	local string CountdownString;
	local bool bShouldPlayBeep;

	CurrOpacity = GetPulseOpacity();
	CountdownString = BuildCountdownString(Countdown);
	bShouldPlayBeep =	bPlayCountdownBeep
					&&	Countdown <= 5
					&&	CountdownString != PGLMapData.MapVoteTimerLabel.GetDataStoreBinding();

	PGLMapData.MapVoteTimerLabel.SetDataStoreBinding( CountdownString );
	PGLMapData.MapVoteTimerLabel.Opacity = CurrOpacity;
	PGLMapData.MapVoteTimerLabel.SetVisibility(true);
	PGLMapData.VotingImage.Opacity = CurrOpacity;
	//PGLMapData.VotingImage.SetVisibility(true);

	if ( bShouldPlayBeep )
	{
		bPlayCountdownBeep = false;
		PlayUISound('Beep');
	}
}

/** Update the map texture for voting on a map */
function UpdateMapVoteImage()
{
	local MaterialInstanceConstant MapMaterial;
	local float FirstVote, SecondVote, NumVoters, SliceSize;
	local Texture2D MapATexture, MapBTexture;
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue GameResourceData;
	local GearPreGameGRI PreGameGRI;
	local string MapAPath, MapBPath;
	local int FirstMapIndex, SecondMapIndex;

	PreGameGRI = GetPreGameGRI();
	GameResourceDS = GetGameResourceDataStore();

	MapMaterial = MaterialInstanceConstant(PGLMapData.VotingMapImage.ImageComponent.GetImage());

	// Get the paths to the map textures
	MapAPath = "";
	MapBPath = "";
	if ( GameResourceDS != None )
	{
		FirstMapIndex = GameResourceDS.GetProviderIndexFromMapName( PreGameGRI.FirstMapName );
		if ( FirstMapIndex == INDEX_NONE )
		{
			MapAPath = "UI_Portraits.MP.UI_MP_Random";
		}
		else
		{
			if ( GameResourceDS.GetProviderFieldValue('Maps', 'ScreenshotPathName', FirstMapIndex, GameResourceData) )
			{
				MapAPath = GameResourceData.StringValue;
			}
		}

		SecondMapIndex = GameResourceDS.GetProviderIndexFromMapName( PreGameGRI.SecondMapName );
		if ( SecondMapIndex == INDEX_NONE )
		{
			MapBPath = "UI_Portraits.MP.UI_MP_Random";
		}
		else
		{
			if ( GameResourceDS.GetProviderFieldValue('Maps', 'ScreenshotPathName', SecondMapIndex, GameResourceData) )
			{
				MapBPath = GameResourceData.StringValue;
			}
		}
	}

	// Attempt to find and set the texture for the first map
	MapATexture = Texture2D(FindObject(MapAPath, class'Texture2D'));
	if ( MapATexture != None )
	{
		MapMaterial.SetTextureParameterValue( 'MapB', MapATexture );
	}

	// Attempt to find and set the texture for the second map
	MapBTexture = Texture2D(FindObject(MapBPath, class'Texture2D'));
	if ( MapBTexture != None )
	{
		MapMaterial.SetTextureParameterValue( 'MapA', MapBTexture );
	}

	// values will range from 0.6 for Map A and -3.4 for Map B
	FirstVote = PreGameGRI.FirstMapVoteCount;
	SecondVote = PreGameGRI.SecondMapVoteCount;
	NumVoters = PreGameGRI.GetNumPotentialVoters();
	SliceSize = 2.0f / NumVoters;
    PGLMapData.VoteImageValueDestination = 2.0f + (SecondVote * SliceSize) - (FirstVote * SliceSize);
	PGLMapData.VoteImageValueStart = PGLMapData.VoteImageValueCurrent;
	PGLMapData.VoteImageDestinationChangeTime = PreGameGRI.WorldInfo.TimeSeconds;

	// Set the texture
	PGLMapData.VotingMapImage.ImageComponent.SetImage( MapMaterial );
}

/** Refreshes the map voting material */
function RefreshMapVotingMaterial( bool bForceDestination )
{
	local MaterialInstanceConstant MapMaterial;
	local float InterpPercent, FinalValue;

	if ( bForceDestination || PGLMapData.VoteImageValueDestination != PGLMapData.VoteImageValueStart )
	{
		MapMaterial = MaterialInstanceConstant(PGLMapData.VotingMapImage.ImageComponent.GetImage());

		// Out of time, set it
		if ( GetPreGameGRI().WorldInfo.TimeSeconds >= PGLMapData.VoteImageDestinationChangeTime + MapVoteInterpTime || bForceDestination )
		{
			PGLMapData.VoteImageValueCurrent = PGLMapData.VoteImageValueDestination;
			PGLMapData.VoteImageValueStart = PGLMapData.VoteImageValueDestination;
		}
		else
		{
			InterpPercent = 1.0f - ((PGLMapData.VoteImageDestinationChangeTime + MapVoteInterpTime - GetPreGameGRI().WorldInfo.TimeSeconds) / MapVoteInterpTime);
			if ( PGLMapData.VoteImageValueStart > PGLMapData.VoteImageValueDestination )
			{
				PGLMapData.VoteImageValueCurrent = PGLMapData.VoteImageValueStart - ((PGLMapData.VoteImageValueStart - PGLMapData.VoteImageValueDestination) * InterpPercent);
			}
			else if ( PGLMapData.VoteImageValueDestination > PGLMapData.VoteImageValueStart )
			{
				PGLMapData.VoteImageValueCurrent = PGLMapData.VoteImageValueStart + ((PGLMapData.VoteImageValueDestination - PGLMapData.VoteImageValueStart) * InterpPercent);
			}
		}

		// Set the voting mask
		FinalValue = PGLMapData.VoteImageValueCurrent - 3.4;
		MapMaterial.SetVectorParameterValue( 'VotingMask', MakeLinearColor(FinalValue, 0.0f, 0.0f, 1.0f) );

		// Set the texture
		PGLMapData.VotingMapImage.ImageComponent.SetImage( MapMaterial );
	}
}

/** Update the widgets for a map selection */
function UpdateMapSelection()
{
	UpdateMapSelectionCountdownAndPulsing( GetPreGameGRI().RemainingMapVoteSeconds );
}

/** Update the pulsing of the background and the countdown */
function UpdateMapSelectionCountdownAndPulsing( int Countdown )
{
	local float CurrOpacity;

	CurrOpacity = GetPulseOpacity();

	if (GetWorldInfo().NetMode != NM_Client)
	{
		PGLMapData.MapSelectTimerLabel.SetValue(HostSelectingMap);
	}
	else
	{
		PGLMapData.MapSelectTimerLabel.SetValue("");
	}
	PGLMapData.MapSelectTimerLabel.Opacity = CurrOpacity;
	PGLMapData.SelectRightTrigger.Opacity = CurrOpacity;
	PGLMapData.SelectLeftTrigger.Opacity = CurrOpacity;
	PGLMapData.MapSelectTimerLabel.SetVisibility(true);
	PGLMapData.VotingImage.Opacity = CurrOpacity;
	//PGLMapData.VotingImage.SetVisibility(true);
}

/** Update the map texture for voting on a map */
function UpdateMapSelectionImageAndTitle(bool bMapSelectSuspended)
{
	local MaterialInstanceConstant MapMaterial;
	local Texture2D MapTexture;
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue GameResourceData;
	local GearPreGameGRI PreGameGRI;
	local string MapPath;
	local int SelectedMapProviderIndex;

	PreGameGRI = GetPreGameGRI();
	GameResourceDS = GetGameResourceDataStore();

	MapMaterial = MaterialInstanceConstant(PGLMapData.SelectMapImage.ImageComponent.GetImage());

	// Get the paths to the map textures
	MapPath = "";
	if ( GameResourceDS != None )
	{
		SelectedMapProviderIndex = GameResourceDS.GetProviderIndexFromMapName( PreGameGRI.SelectedMapName );
		// If the provider index is invalid then this is a map we do not have
		if ( SelectedMapProviderIndex == INDEX_NONE )
		{
			MapPath = "UI_Portraits.MP.UI_MP_Random";
			PGLMapData.MissingMapImage.SetVisibility( true );
		}
		else
		{
			if ( GameResourceDS.GetProviderFieldValue('Maps', 'ScreenshotPathName', SelectedMapProviderIndex, GameResourceData) )
			{
				MapPath = GameResourceData.StringValue;
			}

			// If someone doesn't have the map we must show the NO icon on the map
			if ( bMapSelectSuspended )
			{
				PGLMapData.MissingMapImage.SetVisibility( true );
			}
			else
			{
				PGLMapData.MissingMapImage.SetVisibility( false );
			}
		}
	}

	// Attempt to find and set the texture for the map
	MapTexture = Texture2D(FindObject(MapPath, class'Texture2D'));
	if ( MapTexture != None )
	{
		MapMaterial.SetTextureParameterValue( 'MapA', MapTexture );
		// Set the voting mask
		MapMaterial.SetVectorParameterValue( 'VotingMask', MakeLinearColor(0.6f, 0.0f, 0.0f, 1.0f) );
		// Set the texture
		PGLMapData.SelectMapImage.ImageComponent.SetImage( MapMaterial );
	}

	// Set the map name
	if ( GameResourceDS != None )
	{
		PGLMapData.SelectTitleLabel.SetDataStoreBinding( GetMapName(SelectedMapProviderIndex) );
	}
}

/** Sets the scene up for host selection on the map to play */
function InitializeMapSelection()
{
	UpdateMapSelectionImageAndTitle(GetPreGameGRI().bSuspendMapSelection);
}

/** Returns the name of the map at provider index MapIdx */
function string GetMapName( int MapIdx )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue GameResourceData;

	// if the value is INDEX_NONE then they don't have the map
	if ( MapIdx == INDEX_NONE )
	{
		return MissingMap;
	}
	else
	{
		GameResourceDS = GetGameResourceDataStore();
		if ( GameResourceDS != None )
		{
			if ( GameResourceDS.GetProviderFieldValue('Maps', 'DisplayName', MapIdx, GameResourceData) )
			{
				return GameResourceData.StringValue;
			}
		}
	}

	return "";
}

/** Sets the scene up for voting on the map to play */
function InitializeMapVote()
{
	local GearUIDataStore_GameResource GameResourceDS;
	local GearPreGameGRI PreGameGRI;
	local int Idx, MapIdx;
	local string DataMarkup;

	PreGameGRI = GetPreGameGRI();
	GameResourceDS = GetGameResourceDataStore();
	if ( GameResourceDS != None && PreGameGRI != None )
	{
		// setup the maps being voted on
		for ( Idx = 0; Idx < PGLMapData.MapVoteData.length; Idx++ )
		{
			MapIdx = (Idx == 0) ? GameResourceDS.GetProviderIndexFromMapName(PreGameGRI.FirstMapName) : GameResourceDS.GetProviderIndexFromMapName(PreGameGRI.SecondMapName);
			PGLMapData.MapVoteData[Idx].MapNameLabel.SetDataStoreBinding( GetMapName(MapIdx) );

			DataMarkup = (Idx == 0) ? "[<CurrentGame:FirstMapVoteCount>]" : "[<CurrentGame:SecondMapVoteCount>]";
			PGLMapData.MapVoteData[Idx].MapVoteLabel.SetDataStoreBinding( DataMarkup );
		}

		// setup the generic game-type voting widgets
		UpdateMapVoteCountdownAndPulsing( PreGameGRI.RemainingMapVoteSeconds );

		UpdateMapVoteImage();

		PGLMapData.VoteImageValueDestination = 2.0f;
		PGLMapData.VoteImageValueCurrent = 2.0f;
		PGLMapData.VoteImageValueStart = 2.0f;

		RefreshMapVotingMaterial( true );
	}
}

/**
 * Sets the scene up for voting on the map if bPrepareVoting is true,
 * otherwise sets the scene up for map selection by the host
 */
function BeginMapSelection( bool bPrepareVoting )
{
	// GAME SELECT SECTION
	InitializeGameTypeDescription();
	PGLGameData.InactivePanel.SetVisibility(false);
	PGLGameData.VotingPanel.SetVisibility(false);
	PGLGameData.DescribePanel.SetVisibility(true);
	PGLGameData.VotingImage.SetVisibility(false);

	// MAP SELECT SECTION
	PGLMapData.InactivePanel.PlayUIAnimation( '', FadeOut, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
	PGLMapData.VotingImage.SetVisibility(true);
	PGLMapData.DescribePanel.SetVisibility(false);
	if ( bPrepareVoting )
	{
		PGLMapData.VotingPanel.SetVisibility(true);
		PGLMapData.VotingPanel.PlayUIAnimation( '', FadeIn, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
		PGLMapData.SelectPanel.SetVisibility(false);
		InitializeMapVote();
	}
	else
	{
		PGLMapData.VotingPanel.SetVisibility(false);
		PGLMapData.SelectPanel.SetVisibility(true);
		PGLMapData.SelectPanel.PlayUIAnimation( '', FadeIn, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
		InitializeMapSelection();
	}


	// Only the "host" actually uses the left and right triggers.
	if ( GetWorldInfo().NetMode == NM_Client )
	{
		PGLMapData.SelectLeftTrigger.SetVisibility( false );
		PGLMapData.SelectRightTrigger.SetVisibility( false );
	}
	
	

	// CHARACTER SELECT SECTION
	PGLCharacterData.SelectionData[0].CharacterPanel.SetVisibility(false);
	PGLCharacterData.SelectionData[1].CharacterPanel.SetVisibility(false);
	PGLCharacterData.InactivePanel.SetVisibility(true);
	PGLCharacterData.InactivePanel.SetDockTarget( UIFACE_Top, bPrepareVoting ? PGLMapData.VotingPanel : PGLMapData.SelectPanel, UIFACE_Bottom);
	PGLCharacterData.InactivePanel.SetDockPadding( UIFACE_Top, -20 );


	// Tooltip hack
	PGLGameData.TooltipBGImage.SetVisibility(true);
	PGLGameData.TooltipPanel.SetVisibility(true);
	PGLGameData.TooltipPanel.PlayUIAnimation( '', FadeIn, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
	if ( bPrepareVoting )
	{
		PGLGameData.TooltipLabel.SetValue(TooltipMapVoting);
	}
	else 
	{
		if (GetWorldInfo().NetMode == NM_Client)
		{
			PGLGameData.TooltipLabel.SetValue(TooltipMapSelectionClient);
		}
		else
		{
			PGLGameData.TooltipLabel.SetValue(TooltipMapSelection);
		}
	}

	// Hack to make sure the player doesn't spam the start button passed map selection init time
	bMapSelectInitCompleted = true;
}

/** Closes map selection (step 1 of sequence; collapsing and sliding widgets) */
function CloseMapSelection()
{
	// Shrink map panels to the description pannel's size
	PGLMapData.VotingImage.PlayUIAnimation( '', MapSelectionSizeToDescription, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );

	PGLMapData.VotingPanel.PlayUIAnimation( '', MapSelectionSizeToDescription, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
	PGLMapData.SelectPanel.PlayUIAnimation( '', MapSelectionSizeToDescription, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );

	// Undock the character inactive panel
	PGLCharacterData.InactivePanel.SetDockTarget( UIFACE_Top, None, UIFACE_Bottom);
	PGLCharacterData.InactivePanel.SetDockPadding( UIFACE_Top, 0 );

	// Slide the Inactive panel up to the bottom of the description panel's bottom
	PGLCharacterData.InactivePanel.PlayUIAnimation( '', MoveToDescriptionsBottom, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );

	PlayUISound(VoteCompleteCue);

	HideTooltip();
	
	OnTimerTriggered = CloseMapSelectionStep2;
	WaitForTimer(0.25);

}

/** 
 * Closes map selection (step 2 of sequence; fading and flash).
 * Changes the map section to represent the winning map.
 */
function CloseMapSelectionStep2()
{
	// GAME SELECT SECTION
	InitializeGameTypeDescription();
	PGLGameData.InactivePanel.SetVisibility(false);
	PGLGameData.VotingPanel.SetVisibility(false);
	PGLGameData.DescribePanel.SetVisibility(true);

	// MAP SELECT SECTION
	InitializeMapDescription();
	PGLMapData.VotingPanel.SetVisibility(false);
	PGLMapData.SelectPanel.SetVisibility(false);
	PGLMapData.DescribePanel.SetVisibility(true);

	PGLMapData.DescribePanel.PlayUIAnimation( '', FadeIn, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );

	// CHARACTER SELECT SECTION
	PGLCharacterData.SelectionData[0].CharacterPanel.SetVisibility(false);
	PGLCharacterData.SelectionData[1].CharacterPanel.SetVisibility(false);
	PGLCharacterData.InactivePanel.SetVisibility(true);
	PGLCharacterData.InactivePanel.SetDockTarget( UIFACE_Top, None, UIFACE_Bottom);
	PGLCharacterData.InactivePanel.SetDockPadding( UIFACE_Top, 0 );

	// Turn off the press start highlight BG
	PGLGameData.TooltipBGImage.SetVisibility( false );
	PGLGameData.TooltipLabel.SetValue( PressedStart );



	// Wait for the fade and then hide the throbber background
	OnTimerTriggered = CloseMapSelectionComplete;
	WaitForTimer(0.3);
	PlayUISound(VoteOpenPulseCue);
}

/** Initialize the map texture describing the map */
function UpdateMapDescriptionImage()
{
	local MaterialInstanceConstant MapMaterial;
	local Texture2D MapTexture;
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue GameResourceData;
	local GearPreGameGRI PreGameGRI;
	local string MapPath;
	local int WinningMapIndex;

	PreGameGRI = GetPreGameGRI();
	GameResourceDS = GetGameResourceDataStore();

	MapMaterial = MaterialInstanceConstant(PGLMapData.DescribeMapImage.ImageComponent.GetImage());

	// Get the paths to the map textures
	MapPath = "";
	if ( GameResourceDS != None )
	{
		WinningMapIndex = GameResourceDS.GetProviderIndexFromMapName( PreGameGRI.WinningMapName );
		if ( WinningMapIndex == INDEX_NONE )
		{
			MapPath = "UI_Portraits.MP.UI_MP_Random";
		}
		else
		{
			if ( GameResourceDS.GetProviderFieldValue('Maps', 'ScreenshotPathName', WinningMapIndex, GameResourceData) )
			{
				MapPath = GameResourceData.StringValue;
			}
		}
	}

	// Attempt to find and set the texture for the map
	MapTexture = Texture2D(FindObject(MapPath, class'Texture2D'));
	if ( MapTexture != None )
	{
		MapMaterial.SetTextureParameterValue( 'MapA', MapTexture );
	}

	// Set the voting mask
	MapMaterial.SetVectorParameterValue( 'VotingMask', MakeLinearColor(0.6f, 0.0f, 0.0f, 1.0f) );

	// Set the texture
	PGLMapData.DescribeMapImage.ImageComponent.SetImage( MapMaterial );
}

/** Initialize the widgets for the map description */
function InitializeMapDescription()
{
	local GearPreGameGRI PreGameGRI;
	local GearUIDataStore_GameResource GameResourceDS;
	local int WinningMapIndex;
	local string TieString;

	PreGameGRI = GetPreGameGRI();
	GameResourceDS = GetGameResourceDataStore();
	if ( GameResourceDS != None && PreGameGRI != None )
	{
		WinningMapIndex = GameResourceDS.GetProviderIndexFromMapName( PreGameGRI.WinningMapName );

		TieString = "";
		if ( PreGameGRI.bMapVoteEnabled && (PreGameGRI.FirstMapVoteCount == PreGameGRI.SecondMapVoteCount) )
		{
			TieString = " " $ TiedVote;
		}
		PGLMapData.DescribeTitleLabel.SetDataStoreBinding( GetMapName(WinningMapIndex) $ TieString );
	}

	UpdateMapDescriptionImage();
}

/** Close map selection (step 3; hide the white tab) */
function CloseMapSelectionComplete()
{
	PGLMapData.VotingImage.SetVisibility(false);
}

/** initializes the character/weapon selections for all local players */
function InitializeCharacterConfigs()
{
	local int Idx;
	local bool bIsEnabled;

	for ( Idx = 0; Idx < PGLCharacterData.SelectionData.length; Idx++ )
	{
		bIsEnabled = (Idx == 0 || GetPreGameGRI().HasSplitscreenPlayer());
		InitializeCharacterWeaponSelection( Idx, bIsEnabled );
	}
}

/** Update the widgets for character selection */
function UpdateCharacterSelection()
{
	local int Idx;
	local bool bIsEnabled;
	local LocalPlayer LP;
	local GearPC MyGearPC;
	local GearPreGameLobbyPRI MyPRI;

	for ( Idx = 0; Idx < PGLCharacterData.SelectionData.length; Idx++ )
	{
		bIsEnabled = (Idx == 0 || GetPreGameGRI().HasSplitscreenPlayer());
		UpdateCharacterSelectionCountdownAndPulsing( GetPreGameGRI().RemainingSecondsForLoadoutSelection, Idx, bIsEnabled );

		// Get all the objects we need
		LP = GetPlayerOwner(Idx);
		if ( LP != None )
		{
			MyGearPC = GearPC(LP.Actor);
			MyPRI = GearPreGameLobbyPRI(MyGearPC.PlayerReplicationInfo);

			// If the team has changed we must update the character selection data
			if ( MyPRI != None && MyPRI.GetTeamNum() != MyPRI.LastCheckedTeamNum )
			{
				MyPRI.LastCheckedTeamNum = MyPRI.GetTeamNum();

				// If this is a private wingman match we must reinitialize the currently selected
				// character provider index since teams are determined previously in the GRI
				if ( !IsOfficialMatch() && IsWingmanGametype() )
				{
					InitializeCharacterProviderIds( true );
				}

				// Update the widgets
				UpdateCharacterSelectionData( Idx );
			}
		}
	}
}

/**
 * Update the pulsing of the background and the countdown
 *
 * @param Countdown - amount of time to display in the label
 * @param PlayerIndex - playerindex of the player to intialize the character selection for
 * @param bIsEnabled - Whether to show the character selection for this player or not
 */
function UpdateCharacterSelectionCountdownAndPulsing( int Countdown, int PlayerIndex, bool bIsEnabled )
{
	local float CurrOpacity;
	local string CountdownString;
	local bool bShouldPlayBeep;

	CurrOpacity = GetPulseOpacity();

	if ( !bIsEnabled )
	{
		PGLCharacterData.SelectionData[PlayerIndex].SelectionImage.SetVisibility(false);
	}
	else
	{
		//PGLCharacterData.SelectionData[PlayerIndex].SelectionImage.SetVisibility(true);
		PGLCharacterData.SelectionData[PlayerIndex].SelectionImage.Opacity = CurrOpacity;

		PGLCharacterData.SelectionData[PlayerIndex].WeaponIncrementLabel.Opacity = CurrOpacity;
		PGLCharacterData.SelectionData[PlayerIndex].WeaponDecrementLabel.Opacity = CurrOpacity;
		PGLCharacterData.SelectionData[PlayerIndex].CharacterIncrementLabel.Opacity = CurrOpacity;
		PGLCharacterData.SelectionData[PlayerIndex].CharacterDecrementLabel.Opacity = CurrOpacity;

		// Only show the character selection timer in public games and after the host pressed start in private games
		if ( IsOfficialMatch() || GetPreGameGRI().bHostTriggeredGameStart )
		{
			CountdownString = BuildCountdownString(Countdown);
			bShouldPlayBeep =	bPlayCountdownBeep
							&&	Countdown <= 5
							&&	CountdownString != PGLMapData.MapSelectTimerLabel.GetDataStoreBinding();

			PGLCharacterData.SelectionData[PlayerIndex].SelectionTimerLabel.SetDataStoreBinding( CountdownString );
			PGLCharacterData.SelectionData[PlayerIndex].SelectionTimerLabel.SetVisibility(true);
			PGLCharacterData.SelectionData[PlayerIndex].SelectionTimerLabel.Opacity = CurrOpacity;
			if ( bShouldPlayBeep )
			{
				bPlayCountdownBeep = false;
				PlayUISound('Beep');
			}
		}
		else
		{
			PGLCharacterData.SelectionData[PlayerIndex].SelectionTimerLabel.SetVisibility(false);
		}

		if ( !IsOfficialMatch() )
		{
			PGLGameData.TooltipBGImage.Opacity = CurrOpacity;
		}
	}
}

/**
 * Retrieves the player's configured preferred character.
 *
 * @param	PlayerIndex		the index for the player whose profile to pull from.
 * @param	TeamIndex		the index of the player's team; determines whether the preferred cog or preferred locust is returned.
 *
 * @return	the index for the data provider which has a ProfileId value matching the player's profile value.
 */
function int GetPreferredCharacterIndex( int PlayerIndex, int TeamIndex )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local GearProfileSettings Profile;
	local UIProviderScriptFieldValue CharacterValue;
	local int ClassId, Result;

	Profile = GetPlayerProfile(PlayerIndex);
	if ( Profile != None )
	{
		ClassId = Profile.GetPreferredClassIndex(TeamIndex);
		GameResourceDS = GetGameResourceDataStore();
		if ( GameResourceDS != None )
		{
			CharacterValue.PropertyTag = 'ProfileId';
			CharacterValue.PropertyType = DATATYPE_Property;
			CharacterValue.ArrayValue.AddItem(ClassId);

			CharacterValue.StringValue = string(GetEnum(GameResourceDS.GetTeamCharacterProviderIdType(TeamIndex), ClassId));

			Result = GameResourceDS.FindProviderIndexByFieldValue(class'GearUIDataStore_GameResource'.static.GetTeamCharacterProviderTag(TeamIndex), 'ProfileId', CharacterValue);
		}
	}

	return Result;
}

/**
 * initializes the character/weapon selections for a local player
 *
 * @param PlayerIndex - playerindex of the player to intialize the character selection for
 * @param bIsEnabled - Whether to show the character selection for this player or not
 */
function InitializeCharacterWeaponSelection( int PlayerIndex, bool bIsEnabled )
{
	local LocalPlayer LP;
	local GearPC MyGearPC;
	local GearPRI MyGearPRI;
	local bool bIsWingman;

	bIsWingman = IsWingmanGametype();

	// First update the background and timer since we will have to hide the background if there
	// is no splitscreen player
	UpdateCharacterSelectionCountdownAndPulsing( GetPreGameGRI().RemainingSecondsForLoadoutSelection, PlayerIndex, bIsEnabled );

	if ( !bIsEnabled )
	{
		PGLCharacterData.SelectionData[PlayerIndex].CharacterPanel.SetVisibility(false);
		return;
	}
	else
	{
		PGLCharacterData.SelectionData[PlayerIndex].CharacterPanel.SetVisibility(true);
	}

	// Turn all the input icons on/off
	PGLCharacterData.SelectionData[PlayerIndex].CharacterDecrementLabel.SetVisibility(!bIsWingman);
	PGLCharacterData.SelectionData[PlayerIndex].CharacterIncrementLabel.SetVisibility(!bIsWingman);
	PGLCharacterData.SelectionData[PlayerIndex].WeaponDecrementLabel.SetVisibility(true);
	PGLCharacterData.SelectionData[PlayerIndex].WeaponIncrementLabel.SetVisibility(true);

	// Get all the objects we need
	LP = GetPlayerOwner(PlayerIndex);
	MyGearPC = GearPC(LP.Actor);
	MyGearPRI = GearPRI(MyGearPC.PlayerReplicationInfo);

	// Set the gamer tag name
	if ( MyGearPRI != None )
	{
		PGLCharacterData.SelectionData[PlayerIndex].GamerTagLabel.SetDataStoreBinding( MyGearPRI.PlayerName );
	}

	// Update the character and weapon images and set the PRI
	UpdateCharacterSelectionData( PlayerIndex );
	UpdateWeaponSelectionData( PlayerIndex );
}

/** Update the character portrait and set the PRI */
function UpdateCharacterSelectionData( int PlayerIndex )
{
	local LocalPlayer LP;
	local GearPC MyGearPC;
	local GearUIDataStore_GameResource GameResourceDS;
	local Name ProviderTag;
	local GearGameCharacterSummary CharacterData;
	local Texture2D CharacterTexture;
	local int CurrSelection;

	// Get all the objects we need
	LP = GetPlayerOwner(PlayerIndex);
	MyGearPC = GearPC(LP.Actor);

	GameResourceDS = GetGameResourceDataStore();
	if ( GameResourceDS != None )
	{
		// Get the character data provider
		if ( MyGearPC.GetTeamNum() % 2 == 0 )
		{
			ProviderTag = 'COGs';
			CurrSelection = PGLCharacterData.SelectionData[PlayerIndex].CurrCOGCharacterSelected;
		}
		else
		{
			ProviderTag = 'Locusts';
			CurrSelection = PGLCharacterData.SelectionData[PlayerIndex].CurrLocustCharacterSelected;
		}
		CharacterData = class'GearUIDataStore_GameResource'.static.GetCharacterProvider( CurrSelection, ProviderTag );
		if ( CharacterData != None )
		{
			// Attempt to find and set the texture for the character
			CharacterTexture = Texture2D(FindObject(CharacterData.PortraitIcon.ImagePathName, class'Texture2D'));

			// Set the texture
			if ( CharacterTexture != None )
			{
				PGLCharacterData.SelectionData[PlayerIndex].CharacterImage.ImageComponent.SetImage( CharacterTexture );
				PGLCharacterData.SelectionData[PlayerIndex].CharacterImage.ImageComponent.SetCoordinates( CharacterData.PortraitIcon.Coordinates );
			}
		}

		// Update the PRI
		GearPRI(MyGearPC.PlayerReplicationInfo).ServerSetCharacterId( GetCharacterIdFromProviderIndex(CurrSelection) );
	}
}

/** Update the weapon portrait and set the PRI */
function UpdateWeaponSelectionData( int PlayerIndex )
{
	local LocalPlayer LP;
	local GearPC MyGearPC;
	local GearUIDataStore_GameResource GameResourceDS;
	local Surface WeaponTexture;
	local GearGameWeaponSummary WeaponData;

	// Get all the objects we need
	LP = GetPlayerOwner(PlayerIndex);
	MyGearPC = GearPC(LP.Actor);

	GameResourceDS = GetGameResourceDataStore();
	if ( GameResourceDS != None )
	{
		// Get the weapon data provider
		WeaponData = class'GearUIDataStore_GameResource'.static.GetWeaponProvider( PGLCharacterData.SelectionData[PlayerIndex].CurrWeaponSelected );
		if ( WeaponData != None )
		{
			// Attempt to find and set the texture for the weapon
			WeaponTexture = WeaponData.GetWeaponIcon();

			// Set the texture
			if ( WeaponTexture != None )
			{
				PGLCharacterData.SelectionData[PlayerIndex].WeaponImage.ImageComponent.SetImage( WeaponTexture );
				//PGLCharacterData.SelectionData[PlayerIndex].WeaponImage.ImageComponent.SetCoordinates( WeaponData.WeaponIcon.Coordinates );
			}
		}

		// Update the PRI
		GearPRI(MyGearPC.PlayerReplicationInfo).ServerSetWeaponId(GetWeaponIdFromWeaponProviderIndex(PGLCharacterData.SelectionData[PlayerIndex].CurrWeaponSelected));
	}
}

// initializes the provider ids for the character and weapon classes
function InitializeCharacterProviderIds( optional bool bOnlyInitCharacters )
{
	local LocalPlayer LP;
	local GearPC MyGearPC;
	local GearProfileSettings MyProfile;
	local int Idx, ClassId, TeamIndex, WeaponId;
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue CharacterValue;
	local UIProviderScriptFieldValue WeaponValue;
	local GearGameCharacterSummary CharacterData;

	for ( Idx = 0; Idx < PGLCharacterData.SelectionData.length; Idx++ )
	{
		if ( Idx == 0 || GetPreGameGRI().HasSplitscreenPlayer() )
		{
			// Get all the objects we need
			LP = GetPlayerOwner(Idx);
			MyGearPC = GearPC(LP.Actor);
			MyProfile = MyGearPC.ProfileSettings;

			if ( MyProfile != None )
			{
				GameResourceDS = GetGameResourceDataStore();
				if ( GameResourceDS != None )
				{
					TeamIndex = MyGearPC.GetTeamNum();
					GearPreGameLobbyPRI(MyGearPC.PlayerReplicationInfo).LastCheckedTeamNum = TeamIndex;

					// Find the provider id for the character
					CharacterValue.PropertyTag = 'ProfileId';
					CharacterValue.PropertyType = DATATYPE_Property;
					if ( IsWingmanGametype() )
					{
						ClassId = GetWingmanClassId( TeamIndex );
						CharacterValue.StringValue = string(GetEnum(GameResourceDS.GetTeamCharacterProviderIdType(TeamIndex), ClassId));
						CharacterValue.ArrayValue.AddItem(ClassId);
						if ( TeamIndex % 2 == 0 )
						{
							PGLCharacterData.SelectionData[Idx].CurrCOGCharacterSelected = GameResourceDS.FindProviderIndexByFieldValue(class'GearUIDataStore_GameResource'.static.GetTeamCharacterProviderTag(TeamIndex), 'ProfileId', CharacterValue);
						}
						else
						{
							PGLCharacterData.SelectionData[Idx].CurrLocustCharacterSelected = GameResourceDS.FindProviderIndexByFieldValue(class'GearUIDataStore_GameResource'.static.GetTeamCharacterProviderTag(TeamIndex), 'ProfileId', CharacterValue);
						}
					}
					else
					{
						ClassId = MyProfile.GetPreferredClassIndex( 0 );
						CharacterValue.StringValue = string(GetEnum(GameResourceDS.GetTeamCharacterProviderIdType(0), ClassId));
						CharacterValue.ArrayValue.AddItem(ClassId);
						PGLCharacterData.SelectionData[Idx].CurrCOGCharacterSelected = GameResourceDS.FindProviderIndexByFieldValue(class'GearUIDataStore_GameResource'.static.GetTeamCharacterProviderTag(0), 'ProfileId', CharacterValue);
						CharacterData = class'GearUIDataStore_GameResource'.static.GetCharacterProvider( PGLCharacterData.SelectionData[Idx].CurrCOGCharacterSelected, 'COGs' );
						if (!CharacterIsUsableInGameType(CharacterData))
						{
							PGLCharacterData.SelectionData[Idx].CurrCOGCharacterSelected = 0;
						}

						ClassId = MyProfile.GetPreferredClassIndex( 1 );
						CharacterValue.StringValue = string(GetEnum(GameResourceDS.GetTeamCharacterProviderIdType(1), ClassId));
						CharacterValue.ArrayValue.AddItem(ClassId);
						PGLCharacterData.SelectionData[Idx].CurrLocustCharacterSelected = GameResourceDS.FindProviderIndexByFieldValue(class'GearUIDataStore_GameResource'.static.GetTeamCharacterProviderTag(1), 'ProfileId', CharacterValue);
						CharacterData = class'GearUIDataStore_GameResource'.static.GetCharacterProvider( PGLCharacterData.SelectionData[Idx].CurrLocustCharacterSelected, 'Locusts' );
						if (!CharacterIsUsableInGameType(CharacterData))
						{
							PGLCharacterData.SelectionData[Idx].CurrLocustCharacterSelected = 0;
						}
					}

					if ( !bOnlyInitCharacters )
					{
						// Find the provider id for the weapon
						WeaponId = MyProfile.GetPreferredWeaponId();
						WeaponValue.PropertyTag = 'WeaponId';
						WeaponValue.PropertyType = DATATYPE_Property;
						WeaponValue.StringValue = string(WeaponId);
						PGLCharacterData.SelectionData[Idx].CurrWeaponSelected = GameResourceDS.FindProviderIndexByFieldValue('Weapons', 'WeaponId', WeaponValue);
					}
				}
			}
		}
	}
}

/** Returns the hard-coded character id for a wingman player based on TeamId */
function int GetWingmanClassId( int TeamIndex )
{
	local GearPreGameGRI PreGameGRI;

	PreGameGRI = GearPreGameGRI(GetGRI());
	if ( PreGameGRI != None )
	{
		return PreGameGRI.WingmanClassIndexes[TeamIndex];
	}

	return 0;
}

/** Sets the scene up for selecting a player's character and weapon */
function BeginCharacterSelection()
{
	local bool bHasSplitscreenPlayer;

	// GAME SELECT SECTION
	InitializeGameTypeDescription();
	PGLGameData.InactivePanel.SetVisibility(false);
	PGLGameData.VotingPanel.SetVisibility(false);
	PGLGameData.DescribePanel.SetVisibility(true);

	// Wrap up map selection.
	PGLMapData.VotingImage.SetVisibility(false);
	PGLMapData.InactivePanel.SetVisibility(false);
	PGLMapData.VotingPanel.SetVisibility(false);
	PGLMapData.SelectPanel.SetVisibility(false);

	// initializes the provider ids for the character and weapon classes
	InitializeCharacterProviderIds();

	// initializes the character/weapon selections
	InitializeCharacterConfigs();
	

	// CHARACTER SELECT SECTION
	bHasSplitscreenPlayer = GetPreGameGRI().HasSplitscreenPlayer();
	
	// Fade in character selection.
	PGLCharacterData.InactivePanel.PlayUIAnimation( '', FadeOut, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
	PGLCharacterData.SelectionData[0].CharacterPanel.PlayUIAnimation( '', FadeIn, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
	PGLCharacterData.SelectionData[0].SelectionImage.SetVisibility(true);
	if (bHasSplitscreenPlayer)
	{
		PGLCharacterData.SelectionData[1].CharacterPanel.PlayUIAnimation( '', FadeIn, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
		PGLCharacterData.SelectionData[1].SelectionImage.SetVisibility(true);
	}

	//PGLCharacterData.InactivePanel.SetVisibility(false);
	PGLCharacterData.SelectionData[0].CharacterPanel.SetVisibility(true);
	
	PGLCharacterData.SelectionData[1].CharacterPanel.SetVisibility(bHasSplitscreenPlayer);

	// Initialize the tooltip section if we're in a custom match
	if ( !IsOfficialMatch() )
	{
		PGLGameData.TooltipLabel.SetValue( (GetWorldInfo().NetMode != NM_Client) ? PressStartHost : PressStartClient );
	}
	else
	{
		PGLGameData.TooltipLabel.SetValue( (IsWingmanGametype()) ? (TooltipWingman) : (TooltipCharacterSelection) );
	}
	// Tooltip hack
	PGLGameData.TooltipBGImage.SetVisibility(true);
	PGLGameData.TooltipPanel.SetVisibility(true);
	PGLGameData.TooltipPanel.PlayUIAnimation( '', FadeIn, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );

	PlayUISound(VoteOpenPulseCue);

	// Hack to make sure the player doesn't spam the start button passed character selection init time
	bCharacterSelectInitCompleted = true;
}

/** Stops the scene for selecting a player's character and weapon */
function EndCharacterSelection()
{
	local int Idx;

	for ( Idx = 0; Idx < PGLCharacterData.SelectionData.length; Idx++ )
	{
		if ( Idx == 0 || GetPreGameGRI().HasSplitscreenPlayer() )
		{
			// Turn all the input icons off
			PGLCharacterData.SelectionData[Idx].CharacterDecrementLabel.SetVisibility(false);
			PGLCharacterData.SelectionData[Idx].CharacterIncrementLabel.SetVisibility(false);
			PGLCharacterData.SelectionData[Idx].WeaponDecrementLabel.SetVisibility(false);
			PGLCharacterData.SelectionData[Idx].WeaponIncrementLabel.SetVisibility(false);

			// Turn timer and background off
			PGLCharacterData.SelectionData[Idx].SelectionTimerLabel.SetVisibility(false);
			PGLCharacterData.SelectionData[Idx].SelectionImage.SetVisibility(false);
		}
	}
}

/** Starts the process of beginning the match */
function BeginMatch()
{

}

/** Updates any strings that are displaying a counter in the lobby */
function UpdateCountdown()
{
	local int Idx;

	// Set the pulse time
	LastPulseTime = GetPreGameGRI().WorldInfo.TimeSeconds;

	switch ( CurrPGLobbyState )
	{
		case eGPGLSTATE_MapSelection:
			//PGLMapData.MapSelectTimerLabel.RefreshSubscriberValue();
			PGLMapData.MapVoteTimerLabel.RefreshSubscriberValue();
			break;

		case eGPGLSTATE_CharacterSelect:
			for ( Idx = 0; Idx < PGLCharacterData.SelectionData.length; Idx++ )
			{
				PGLCharacterData.SelectionData[Idx].SelectionTimerLabel.RefreshSubscriberValue();
			}
			break;
	}

	bPlayCountdownBeep = LobbyIsInVoteState();
}

/** Update the match is beginning text */
function UpdateMatchBeginning()
{
	local String DotsString;
	local int DotIdx, TotalDots;
	local GearPreGameGRI PreGameGRI;

	PreGameGRI = GearPreGameGRI(GetGRI());

	PGLGameData.TooltipPanel.SetVisibility( true );
	PGLGameData.TooltipBGImage.SetVisibility( true );
	PGLGameData.TooltipLoadingIndicator.SetVisibility( true );
	PGLGameData.TooltipBGImage.Opacity = 1.0f;
	

	DotsString = "";
	TotalDots = PreGameGRI.WorldInfo.TimeSeconds % 4;
	for ( DotIdx = 0; DotIdx < TotalDots; DotIdx++ )
	{
		DotsString $= ".";
	}
	PGLGameData.TooltipLabel.SetValue( PressedStart $ DotsString );
	
	
}

/** Called by tick to update script code if subscribed to */
function OnSceneTick( float DeltaTime )
{
	local GearPreGameGRI PreGameGRI;

	PreGameGRI = GetPreGameGRI();

	UpdatePlayerButtons();
	UpdateChatIcons();
	RefreshButtonBar();

	if (TimerTimeLeft > 0.0)
	{
		TimerTimeLeft -= DeltaTime;
		if (TimerTimeLeft <= 0.0)
		{
			OnTimerTriggered();
		}
	}

	switch ( CurrPGLobbyState )
	{
		case eGPGLSTATE_GameVote:
			UpdateGameTypeVote();
			break;
		case eGPGLSTATE_MapSelection:
			if ( PreGameGRI.bMapVoteEnabled )
			{
				UpdateMapVote();
				RefreshMapVotingMaterial( false );
			}
			else
			{
				UpdateMapSelection();
			}
			break;

		case eGPGLSTATE_CharacterSelect:
			UpdateCharacterSelection();
			break;
		case eGPGLSTATE_PostCharacterSelect:
		case eGPGLSTATE_StartingMatch:
			UpdateMatchBeginning();
			break;
	}

	// Check for DLC every few seconds
	DLCCheckTime += DeltaTime;
	if (DLCCheckTime > 5.0f)
	{
		UpdateLocalPlayersDLCValues();
		DLCCheckTime = 0.0f;
	}
}

/** Whether the lobby is in s state of selecting game, map, character, or not */
function bool LobbyIsInVoteState()
{
	if ( CurrPGLobbyState == eGPGLSTATE_GameVote ||
		 CurrPGLobbyState == eGPGLSTATE_MapSelection ||
		 CurrPGLobbyState == eGPGLSTATE_CharacterSelect )
	{
		return true;
	}

	return false;
}

/** Sends our game vote */
function bool SubmitGameVote( int DesiredGameIndex, int PlayerIndex )
{
	local bool bResult;
	local GearPreGameLobbyPRI PRI;
	local bool bHasSplitscreen;

	bHasSplitscreen = GetPreGameGRI().HasSplitscreenPlayer();

	PRI = GearPreGameLobbyPRI(GetGearPlayerOwner(PlayerIndex).PlayerReplicationInfo);
	if ( PRI != None )
	{
		// only allow a vote if this player has not yet voted
		if ( ! PRI.bSubmittedGameVote )
		{
			PRI.SubmitGameVote( DesiredGameIndex );
			if ( PRI.bSubmittedGameVote )
			{
				// Fade for to expose the white background - flash of white notifying that we have voted.
				PGLGameData.VotingPanel.PlayUIAnimation( '', QuickFade, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
				PlayUISound(VoteCue);

				NumLocalGameVotes++;
				// Turn off the gametype voting buttons
				if ( (bHasSplitscreen && NumLocalGameVotes >= 2) || (!bHasSplitscreen && NumLocalGameVotes >= 1) )
				{
					PGLGameData.GameVoteData[0].VoteIconImage.SetVisibility( false );
					PGLGameData.GameVoteData[1].VoteIconImage.SetVisibility( false );
				}
			}
		}
		else
		{
			PlayUISound('Error');
		}
		bResult = true;
	}

	return bResult;
}

/** Sends our map vote */
function bool SubmitMapVote( int DesiredMapIndex, int PlayerIndex )
{
	local bool bResult;
	local GearPreGameLobbyPRI PRI;
	local bool bHasSplitscreen;

	bHasSplitscreen = GetPreGameGRI().HasSplitscreenPlayer();

	PRI = GearPreGameLobbyPRI(GetGearPlayerOwner(PlayerIndex).PlayerReplicationInfo);
	if ( PRI != None )
	{
		// only allow a vote if this player has not yet voted
		if ( ! PRI.bSubmittedMapVote )
		{
			PRI.SubmitMapVote( DesiredMapIndex );
			if ( PRI.bSubmittedMapVote )
			{
				// Fade for to expose the white background - flash of white notifying that we have voted.
				PGLMapData.VotingPanel.PlayUIAnimation( '', QuickFade, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
				PlayUISound(VoteCue);

				NumLocalMapVotes++;				
				// Turn off the map voting buttons
				if ( (bHasSplitscreen && NumLocalMapVotes >= 2) || (!bHasSplitscreen && NumLocalMapVotes >= 1) )
				{
					PGLMapData.MapVoteData[0].VoteIconImage.SetVisibility( false );
					PGLMapData.MapVoteData[1].VoteIconImage.SetVisibility( false );
				}
			}
			
		}
		else
		{
			PlayUISound('Error');
		}
		bResult = true;
	}

	return bResult;
}


/** Increment or decrement the selected player class */
function SubmitCharacterSelection( int DesiredDirection, int PlayerIndex )
{
	local LocalPlayer LP;
	local GearPC MyGearPC;
	local GearUIDataStore_GameResource GameResourceDS;
	local int CharacterCount, TeamId, CurrProviderId, OriginalProviderId;
	local Name ProviderTag;
	local GearGameCharacterSummary CharacterData;

	// Get all the objects we need
	LP = GetPlayerOwner(PlayerIndex);
	MyGearPC = GearPC(LP.Actor);

	GameResourceDS = GetGameResourceDataStore();
	if ( GameResourceDS != None && MyGearPC.ProfileSettings != None )
	{
		TeamId = MyGearPC.GetTeamNum();
		if ( TeamId % 2 == 0 )
		{
			ProviderTag = 'COGs';
			CurrProviderId = PGLCharacterData.SelectionData[PlayerIndex].CurrCOGCharacterSelected;
		}
		else
		{
			ProviderTag = 'Locusts';
			CurrProviderId = PGLCharacterData.SelectionData[PlayerIndex].CurrLocustCharacterSelected;
		}
		OriginalProviderId = CurrProviderId;
		CharacterCount = GameResourceDS.GetProviderCount(ProviderTag);
		if ( CharacterCount > 0 )
		{
			// Loop until we find an unlocked character or looped, which shouldn't happen
			do
			{
				CurrProviderId += DesiredDirection;
				if ( CurrProviderId < 0 )
				{
					CurrProviderId = CharacterCount - 1;
				}
				else
				{
					CurrProviderId = CurrProviderId % CharacterCount;
				}
				CharacterData = class'GearUIDataStore_GameResource'.static.GetCharacterProvider( CurrProviderId, ProviderTag );
			}
			until ( (CharacterData == None ||
					 CharacterData.UnlockableValue == eUNLOCK_Character_None ||
				     MyGearPC.ProfileSettings.HasUnlockableBeenUnlocked(CharacterData.UnlockableValue) ||
				     CurrProviderId == OriginalProviderId) &&
					(CharacterIsUsableInGameType(CharacterData)));

			if ( CurrProviderId != OriginalProviderId )
			{
				if ( TeamId % 2 == 0 )
				{
					PGLCharacterData.SelectionData[PlayerIndex].CurrCOGCharacterSelected = CurrProviderId;
				}
				else
				{
					PGLCharacterData.SelectionData[PlayerIndex].CurrLocustCharacterSelected = CurrProviderId;
				}
				UpdateCharacterSelectionData( PlayerIndex );
			}
		}
	}
}


/** Sends our ready status */
function bool SubmitReady( int PlayerIndex )
{
	local bool bResult;
	local GearPreGameLobbyPRI PRI;

	PRI = GearPreGameLobbyPRI(GetGearPlayerOwner(PlayerIndex).PlayerReplicationInfo);
	if ( PRI != None && !PRI.bReady )
	{
		PRI.SubmitReady();
		if ( PRI.bReady )
		{
			// Tooltip hack
			PGLGameData.TooltipBGImage.SetVisibility(true);
			PGLGameData.TooltipPanel.SetVisibility(true);
			PGLGameData.TooltipLabel.SetValue(TooltipReady);
			PGLGameData.TooltipPanel.PlayUIAnimation( '', FadeIn, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
		}
		bResult = true;
	}

	return bResult;
}


/** Whether this character can be used in the current gametype */
final function bool CharacterIsUsableInGameType(GearGameCharacterSummary CharacterData)
{
	return (!IsGuardianGametype() || !CharacterData.bIsLeader);
}

/**
 * Retrieves the ProfileId value for a GearGameCharacterSummary data provider.
 *
 * @param	CharacterProviderIndex	the index [into the GameResource data store's list of character data providers; the owning player's
 *									current team will determine which of the data store's lists of character providers will be used.
 *
 * @return	the value of the specified character data provider's ProfileId; will be one of the values of either the
 *			ECogMPCharacter or ELocustMPCharacter enums, depending on the player's team.
 */
function byte GetCharacterIdFromProviderIndex( int CharacterProviderIndex )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local byte Result;
	local GearGameCharacterSummary CharacterInfo;
	local name TeamProviderTag;

	Result = 0;
	GameResourceDS = GetGameResourceDataStore();

	if ( GameResourceDS != None )
	{
		TeamProviderTag = GetTeamProviderName();
		CharacterInfo = GameResourceDS.GetCharacterProvider( CharacterProviderIndex, GetTeamProviderName() );
		if ( CharacterInfo != None )
		{
			if ( TeamProviderTag == 'COGs' )
			{
				Result = COGCharacterSummary(CharacterInfo).ProfileId;
			}
			else
			{
				Result = LocustCharacterSummary(CharacterInfo).ProfileId;
			}
		}
	}

	return Result;
}

/**
 * Wrapper for retrieving the name used by the GameResource data store to identify the
 * character data providers for the owning player's team.
 *
 * @return	the name to use for retrieving information from a character data provider associated
 *			with the owning player's team.
 */
function name GetTeamProviderName()
{
	local name Result;

	Result = class'GearUIDataStore_GameResource'.static.GetTeamCharacterProviderTag(GetTeamIndex());
	return Result;
}

/**
 * Converts a character type enum value string into its byte value.
 *
 * @param	TeamIndex			determines which enum is used for converting the string value.
 * @param	EnumValueString		a string representing a value from either the ECogMPCharacter or the ELocustMPCharacter
 *								enums, depending on the specified TeamIndex.
 *
 * @return	the actual byte value of the specified enum value string
 */
static function byte ConvertCharacterValueStringToByte( int TeamIndex, string EnumValueString )
{
	local int i, MaxCharacters;
	local byte Result;

	MaxCharacters = (TeamIndex % 2) == 0 ? CMPC_MAX : LMPC_MAX;
	for ( i = 0; i < MaxCharacters; i++ )
	{
		if ( EnumValueString == string(GetEnum(class'GearUIDataStore_GameResource'.static.GetTeamCharacterProviderIdType(TeamIndex),i)) )
		{
			Result = i;
			break;
		}
	}

	return Result;
}

/**
 * Converts a string into its byte value.
 *
 * @param	EnumValueString		a string representing a value from the EGearWeaponType enum
 *
 * @return	the actual byte value of the specified enum value string
 */
static function byte ConvertWeaponValueStringToByte( string EnumValueString )
{
	local int i;
	local byte Result;

	for ( i = 0; i < eGEARWEAP_MAX; i++ )
	{
		if ( EnumValueString == string(GetEnum(enum'EGearWeaponType',i)) )
		{
			Result = i;
			break;
		}
	}

	return Result;
}

/** Increment or decrement the selected weapon class */
function SubmitWeaponSelection( int DesiredDirection, int PlayerIndex )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local int WeaponCount, CurrProviderId, OriginalProviderId;
	local GearGameWeaponSummary WeaponInfo;

	GameResourceDS = GetGameResourceDataStore();
	if ( GameResourceDS != None )
	{
		CurrProviderId = PGLCharacterData.SelectionData[PlayerIndex].CurrWeaponSelected;
		OriginalProviderId = CurrProviderId;
		WeaponCount = GameResourceDS.GetProviderCount('Weapons');
		if ( WeaponCount > 1 )
		{
			do
			{
				CurrProviderId += DesiredDirection;
				if ( CurrProviderId < 0 )
				{
					CurrProviderId = WeaponCount - 1;
				}
				else
				{
					CurrProviderId = CurrProviderId % WeaponCount;
				}
				WeaponInfo = GameResourceDS.GetWeaponProvider( CurrProviderId );
			}
			until ( WeaponInfo == None ||
				    CurrProviderId == OriginalProviderId ||
					WeaponInfo.bIsDefaultWeapon );

			PGLCharacterData.SelectionData[PlayerIndex].CurrWeaponSelected = CurrProviderId;
			UpdateWeaponSelectionData( PlayerIndex );
		}
	}
}

/** Fade out the tooltip */
function HideTooltip()
{
	PGLGameData.TooltipBGImage.SetVisibility(false);
	PGLGameData.TooltipPanel.PlayUIAnimation( '', FadeOut, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
}

/**
 * Retrieves the ProfileId for a GearGameWeaponSummary data provider.
 *
 * @param	WeaponProviderIndex		the index [into the GameResource data store's list of weapon data providers
 *
 * @return	the value of the specified weapon data provider's WeaponId
 */
function byte GetWeaponIdFromWeaponProviderIndex( int WeaponProviderIndex )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local GearGameWeaponSummary WeaponInfo;

	GameResourceDS = GetGameResourceDataStore();
	if ( GameResourceDS != None )
	{
		WeaponInfo = GameResourceDS.GetWeaponProvider( WeaponProviderIndex );
		if ( WeaponInfo != None )
		{
			return WeaponInfo.WeaponId;
		}
	}

	return 0;
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
function bool OnPlayerListPress( UIScreenObject EventObject, int PlayerIndex )
{
	local GearPRI SelectedPRI;
	local int TeamIdx, PlayerIdx, GameIdx;
	local bool bIsWingman;
	local array<EGearPlayerOptions> Options;

	bIsWingman = IsWingmanGametype();
	GameIdx = bIsWingman ? ePGL_Wingman : ePGL_Normal;
	GetPlayerDataIndexFromWidget( EventObject, GameIdx, TeamIdx, PlayerIdx );
	SelectedPRI = PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[TeamIdx].DataList[PlayerIdx].PlayerPRI;
	// If there is a player in this slot, go to their playercard
	if ( SelectedPRI != None )
	{
		Options.AddItem(eGPOPTION_Gamercard);
		Options.AddItem(eGPOPTION_Feedback);
		Options.AddItem(eGPOPTION_Cancel);
		OpenPlayerOptionsScene( PlayerIndex, SelectedPRI, Options, IsOfficialMatch() || IsCustomMatch() );
	}

	return true;
}

/** Returns the team and player indexs in the PlayerData array of which playerdata contains this widget */
function GetPlayerDataIndexFromWidget( UIScreenObject Widget, int GameIdx, out int OutTeamIdx, out int OutPlayerIdx )
{
	local int Idx, PlayerIdx;

	for ( Idx = 0; Idx < PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData.length; Idx++ )
	{
		for ( PlayerIdx = 0; PlayerIdx < PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[Idx].DataList.length; PlayerIdx++ )
		{
			if ( PGLPlayerData.GameTeamData[GameIdx].PlayersTeamData[Idx].DataList[PlayerIdx].ParentButton == Widget )
			{
				OutTeamIdx = Idx;
				OutPlayerIdx = PlayerIdx;
				return;
			}
		}
	}
}

/** Called when the error message closes for not enough teams */
function bool OnNotEnoughTeams_Confirmed( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	return true;
}

/**
 * Start the brief 5-second countdown in a private match
 */
function StartPregameCountdown()
{
	local GearPreGameGRI PreGameGRI;
	PreGameGRI = GetPreGameGRI();

	if ( !PreGameGRI.bHostTriggeredGameStart )
	{
		PreGameGRI.bHostTriggeredGameStart = TRUE;
		PreGameGRI.CountdownTime = 5;
		PreGameGRI.RemainingSecondsForLoadoutSelection = 5;

		BeginPregameCountdown();
	}
}

/**
 * Sets the tooltip to notify the player that they are now waiting for a pregame countdown.
 */
function BeginPregameCountdown()
{
	// Tooltip hack
	PGLGameData.TooltipBGImage.SetVisibility(true);
	PGLGameData.TooltipPanel.SetVisibility(true);
	PGLGameData.TooltipLabel.SetValue(TooltipWaitingForMatchStart);
	PGLGameData.TooltipPanel.PlayUIAnimation( '', FadeIn, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
}

/**
 * Callback function when the scene gets input
 * @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
 */
function bool ProcessInput( out InputEventParameters EventParms )
{
	local GearPreGameGRI PreGameGRI;
	local array<name> ButtonAliases;
	local GameUISceneClient GameSceneClient;

	PreGameGRI = GetPreGameGRI();

	// Handle game, map, character, and weapon selections
	if ( LobbyIsInVoteState() )
	{
		if ( EventParms.EventType == IE_Pressed )
		{
			// Handle TRIGGERS
			if ( EventParms.InputKeyName == 'XboxTypeS_LeftTrigger' ||
				 EventParms.InputKeyName == 'XboxTypeS_RightTrigger' )
			{
				switch ( CurrPGLobbyState )
				{
					case eGPGLSTATE_GameVote:
						SubmitGameVote( (EventParms.InputKeyName == 'XboxTypeS_LeftTrigger') ? 0 : 1, EventParms.PlayerIndex );
						break;
					case eGPGLSTATE_MapSelection:
						if (bMapSelectInitCompleted)
						{
							if ( PreGameGRI.bMapVoteEnabled )
							{
								SubmitMapVote( (EventParms.InputKeyName == 'XboxTypeS_LeftTrigger') ? 0 : 1, EventParms.PlayerIndex );
							}
							else if ( IsMatchHost(EventParms.PlayerIndex) )
							{
								PreGameGRI.SubmitMapSelection( (EventParms.InputKeyName == 'XboxTypeS_LeftTrigger') ? -1 : 1 );
								// Use the weapon scroll for the map
								PlayUISound(LobbyWeaponSelect);
							}
						}
						break;
					case eGPGLSTATE_CharacterSelect:
						if ( !IsWingmanGametype() )
						{
							SubmitCharacterSelection( (EventParms.InputKeyName == 'XboxTypeS_LeftTrigger') ? -1 : 1, EventParms.PlayerIndex );
							PlayUISound(LobbyCharacterSelect);
						}
						break;
				}

				return true;
			}
			else if ( CurrPGLobbyState == eGPGLSTATE_CharacterSelect &&
				      (EventParms.InputKeyName == 'XboxTypeS_LeftShoulder' ||
					   EventParms.InputKeyName == 'XboxTypeS_RightShoulder') )
			{
				SubmitWeaponSelection( (EventParms.InputKeyName == 'XboxTypeS_LeftShoulder') ? -1 : 1, EventParms.PlayerIndex );
				PlayUISound(LobbyWeaponSelect);
			}
			else if ( EventParms.InputKeyName == 'XboxTypeS_Start' )
			{
				// Handle START
				switch ( CurrPGLobbyState )
				{
					case eGPGLSTATE_CharacterSelect:
						if (bCharacterSelectInitCompleted)
						{
							if (!IsOfficialMatch())
							{
								// Only the match host (not the splitscreen player) should be able to start the match;
								if ( GetWorldInfo().NetMode != NM_Client && IsMatchHost(EventParms.PlayerIndex) )
								{
									if ( AtLeastTwoTeamsAreAvailable() || IsHordeGametype() )
									{
										StartPregameCountdown();
									}
									else
									{
										ButtonAliases.AddItem('GenericContinue');
										GameSceneClient = GetSceneClient();
										GameSceneClient.ShowUIMessage('NotEnoughPlayers',
											"<Strings:GearGameUI.MessageBoxErrorStrings.NotEnoughTeams_Title>",
											"<Strings:GearGameUI.MessageBoxErrorStrings.NotEnoughTeams_Message>",
											"",
											ButtonAliases,
											OnNotEnoughTeams_Confirmed);
										return true;
									}
								}
							}
							else
							{
								//SubmitReady(EventParms.PlayerIndex);
							}
						}
						break;

					case eGPGLSTATE_MapSelection:
						if (bMapSelectInitCompleted)
						{
							if (GetWorldInfo().NetMode != NM_Client &&
								!PreGameGRI.bMapVoteEnabled &&
								IsMatchHost(EventParms.PlayerIndex))
							{
								if (PreGameGRI.bSuspendMapSelection)
								{
									PlayUISound('Error');
								}
								else
								{
									PreGameGRI.SubmitHostFinishedMapSelection();
									PlayUISound(VoteCompleteCue);
								}
							}
						}
						break;
				}
			}
		}
	}

	return false;
}

/**
 * Allows others to be notified when this scene is closed.  Called after the SceneDeactivated event, after the scene has published
 * its data to the data stores bound by the widgets of this scene.
 *
 * @param	DeactivatedScene	the scene that was deactivated
 */
delegate SceneDeactivatedHandler( UIScene DeactivatedScene )
{
	local OnlineSubsystem OnlineSub;
	local int ControllerId;

	if (DeactivatedScene == Self)
	{
		ControllerId = class'UIInteraction'.static.GetPlayerControllerId(GetBestPlayerIndex());
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();

		if (OnlineSub != None && OnlineSub.ContentInterface != None)
		{
			OnlineSub.ContentInterface.ClearQueryAvailableDownloadsComplete(ControllerId, OnReadAvailableContentComplete);
		}
	}
}

defaultproperties
{
	/** Animations */
	Begin Object Class=UIAnimationSeq Name=GameTypeExpand_Template
		SeqName=GameTypeExpand
		Tracks(0)=(TrackType=EAT_Bottom,KeyFrames=((RemainingTime=0.125,Data=(DestAsFloat=245.0))))
	End Object
	GameTypeExpand = GameTypeExpand_Template

	Begin Object Class=UIAnimationSeq Name=MapSelectionExpand_Template
		SeqName=MapSelectionExpand
		Tracks(0)=(TrackType=EAT_Bottom,KeyFrames=((RemainingTime=0.125,Data=(DestAsFloat=485.0))))
	End Object
	MapSelectionExpand = MapSelectionExpand_Template

	Begin Object Class=UIAnimationSeq Name=MapSelectionSizeToDescription_Template
		SeqName=MapSelectionSizeToDescription
		Tracks(0)=(TrackType=EAT_Bottom,KeyFrames=((RemainingTime=0.125,Data=(DestAsFloat=440.0))))
	End Object
	MapSelectionSizeToDescription = MapSelectionSizeToDescription_Template

	Begin Object Class=UIAnimationSeq Name=CharacterSelectExpand_Template
		SeqName=CharacterSelectExpand
		Tracks(0)=(TrackType=EAT_Bottom,KeyFrames=((RemainingTime=0.125,Data=(DestAsFloat=670.0))))
		Tracks(1)=(TrackType=EAT_Top,KeyFrames=((RemainingTime=0.125,Data=(DestAsFloat=440.0))))
	End Object
	CharacterSelectExpand = CharacterSelectExpand_Template

	Begin Object Class=UIAnimationSeq Name=MoveToDescriptionsBottom_Template
		SeqName=MoveToDescriptionsBottom
		Tracks(0)=(TrackType=EAT_Top,KeyFrames=((RemainingTime=0.125,Data=(DestAsFloat=445.0))))
	End Object
	MoveToDescriptionsBottom = MoveToDescriptionsBottom_Template


	Begin Object Class=UIAnimationSeq Name=FadeIn_Template
		SeqName=FadeIn
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=0.0)),(RemainingTime=0.3,Data=(DestAsFloat=1.0))))
	End Object
	FadeIn = FadeIn_Template

	Begin Object Class=UIAnimationSeq Name=FadeOut_Template
		SeqName=FadeOut
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.3,Data=(DestAsFloat=0.0))))
	End Object
	FadeOut = FadeOut_Template

	Begin Object Class=UIAnimationSeq Name=QuickFade_Template
		SeqName=QuickFade
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=0.0)),(RemainingTime=0.125,Data=(DestAsFloat=1.0))))
	End Object
	QuickFade = QuickFade_Template



	bExemptFromAutoClose=true
	OnSceneActivated=SceneActivationComplete
	OnGearUISceneTick=OnSceneTick
	OnRawInputKey=ProcessInput

	Begin Object Name=SceneEventComponent
		DisabledEventAliases.Add(CloseScene)
	End Object

	MapVoteInterpTime=1.0f
	TimerTimeLeft=0.0
	NumLocalMapVotes=0
	NumLocalGameVotes=0

	OnSceneDeactivated=SceneDeactivatedHandler

	/** Sounds */
	LobbyCharacterScroll=G2UI_LobbyCharacterScroll01Cue
	LobbyCharacterSelect=G2UI_LobbyCharacterSelect01Cue
	LobbyScreenSlideBack=G2UI_LobbyScreenSlideBack01Cue
	LobbyScreenSlideForward=G2UI_LobbyScreenSlideForward01Cue
	LobbyWeaponScroll=G2UI_LobbyWeaponScroll01Cue
	LobbyWeaponSelect=G2UI_LobbyWeaponSelect01Cue
	VoteCompleteCue=G2UI_VoteCompleteCue
	VoteCue=G2UI_VoteCue
	VoteOpenCue=G2UI_VoteOpenCue
	VoteOpenPulseCue=G2UI_VoteOpenPulseCue






}