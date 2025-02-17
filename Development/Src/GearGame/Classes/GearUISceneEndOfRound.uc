/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearUISceneEndOfRound extends GearUISceneMP_Base
	Config(UI);

/************************************************************************/
/* Constants, enums, structs, etc.										*/
/************************************************************************/

/** Struct to store all of the data needed for a player */
struct native GearEndRoundPlayerData
{
	//------ Widget references ---------
	/** Label for displaying the player's name */
	var UILabel NameLabel;
	/** Label for displaying what the player did */
	var UILabel TitleLabel;
	/** Label for displaying the player's score */
	var UILabel ScoreLabel;
	/** Image to show whether the player died in the round or not */
	var UIImage DeadImage;
};

/** Struct to store all of the data for a panel of player information */
struct native GearEndRoundPanelData
{
	/** Array of data for each player contained in the panel */
	var array<GearEndRoundPlayerData> PlayerDataList;

	//------ Widget references ---------
	/** The parent panel of all widgets */
	var UIPanel ParentPanel;
};

/** Struct to store all of the data needed for this game */
struct native GearEndRoundData
{
	/** The GameReplicationData for this game */
	var GearGRI MyGRI;

	/** The type of game this is */
	var EGearMPTypes GameType;

	/**
	 * The first panel on the screen and contains different data depending on game type (2 varieties)
	 *     1) 2 players with titles for each but no scores
	 *     2) 5 players with scores for each but no titles
	 */
	var GearEndRoundPanelData MainPanelData;

	/**
	 * The second panel on the screen and contains only 1 player with a title and no score
	 * This is an alternate to MainPanelData incase we only want to display 1 player's/team's information
	 * This panel does not exist on the 5 player version of this screen
	 */
	var GearEndRoundPanelData AlternatePanelData;

	//------ Widget references ---------
	/** Label for displaying the title */
	var UILabel TitleLabel;
	/** Label for displaying the number of rounds/waves won */
	var UILabel TeamScoreLabel;
	/** Image to display the team logo who won */
	var UIImage TeamLogoImage;
	/** Image to display the background */
	var UIImage BackgroundImage;
	/** Label for displaying the amount of time the round took */
	var UILabel TimeLabel;
};

/** Struct to store game-type specific delegates, variable, etc. */
struct native GearEndOfRoundGameTypeSpecificData
{
	/** Delegate for updating the game-specific strings/images for the scene */
	var const delegate<UpdateWidgetValues> UpdateWidgetValuesDelegate;
};

/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** The data for the game this EndOfRound scene is displaying */
var transient GearEndRoundData EndRoundData;

/** The game-type specific data for this scene */
var array<GearEndOfRoundGameTypeSpecificData> GameSpecificData;

/** The gamestatus of the game - used so we can determine if we JUST replicated the gamestatus from the GRI */
var transient EGameStatus PreviousGameStatus;

/** Localized strings needed for this scene */
var localized string TeamRoundWin;
var localized string TeamMatchWin;
var localized string TeamDraw;
var localized string TeamSurvived;
var localized string CTWaveWin;
var localized string CTWaveFail;
var localized string CTWaveWinAll;
var localized string FinalKill;
var localized string FinalTakedown;
var localized string FinalSuicide;
var localized string FinalFriendly;
var localized string FinalDisconnectString;
var localized string FinalCapture;
var localized string DrawMsg;
var localized string ScoreMsg;
var localized string TeamCOG;
var localized string TeamLOC;
var localized string FinalExecute;
var localized string LeaderSuicide;
var localized string LeaderDisconnect;
var localized string FinalEnvironmentDeath;


/** Delegate called to set the strings and images for the scene */
delegate UpdateWidgetValues();

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/**
 * Called after this screen object's children have been initialized
 * Overloaded to initialized the references to this scene's widgets
 */
event PostInitialize()
{
	local GearPC GearPlayer;

	GearPlayer = GetGearPlayerOwner();
	if ( GearPlayer != None && GearPlayer.Role < ROLE_Authority )
	{
		// Set invisible (needed so that the scene is not seen until the GRI is ready)
		SetVisibility( FALSE );
	}

	InitializeEndRound();

	Super.PostInitialize();
}

/** Initialize the EndOfRound scene - widget references etc. */
final function InitializeEndRound()
{
	if ( GetGRI() != None )
	{
		EndRoundData.MyGRI = GetGRI();

		// Determine the type of game this is (this will have impact on how many widget references will be initialized)
		EndRoundData.GameType = GetMPGameType();

		// Initialized the widget references so we can update them later
		InitializeWidgetReferences();

		// Sets all of the proper strings, images, etc. for the widgets
		UpdateWidgetData(0.0f);
	}
}

/** Whether the game we are playing is public */
final function bool IsPublicMatch()
{
	local OnlineSubsystem OnlineSub;
	local OnlineGameSettings OnlineSettings;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None &&
		OnlineSub.GameInterface != None)
	{
		OnlineSettings = OnlineSub.GameInterface.GetGameSettings('Game');
		if (OnlineSettings != None &&
			OnlineSettings.bUsesArbitration)
		{
			return true;
		}
	}
	return false;
}

/** Called when the game status has replicated a new value */
final function GameStatusHasReplicated()
{
	local PlayerController PC;
	local GearPRI LocalPRI;
	local bool bWonRound;
	local GearPC GPC;
	local EGearMapsShipped MapType;

	if (EndRoundData.MyGRI.GameStatus == GS_RoundOver ||
		EndRoundData.MyGRI.GameStatus == GS_EndMatch)
	{
		//foreach EndRoundData.MyGRI.LocalPlayerControllers(class'PlayerController', PC)
		PC = GetGearPlayerOwner();
		if (PC != None)
		{
			LocalPRI = GearPRI(PC.PlayerReplicationInfo);
			if (LocalPRI != None)
			{
				GPC = GearPC(PC);
				if (GPC != None && GPC.ProfileSettings != None)
				{
					bWonRound = LocalPRI.GetTeamNum() == TeamInfo(EndRoundData.MyGRI.Winner).TeamIndex;

					// Update the eGA_AroundTheWorld achievement
					if (EndRoundData.MyGRI.GameStatus == GS_EndMatch &&
						EndRoundData.GameType != eGEARMP_CombatTrials &&
						bWonRound)
					{
						MapType = class'GearUIDataStore_GameResource'.static.GetMPMapsShippedType( EndRoundData.MyGRI.GetURLMap() );
						GPC.ProfileSettings.UpdateMPMatchProgression( EndRoundData.GameType, MapType, GPC );
					}

					// Check for eGA_Party1999
					if (EndRoundData.MyGRI.GameStatus == GS_RoundOver || EndRoundData.MyGRI.GameStatus == GS_EndMatch)
					{
						GPC.ProfileSettings.UpdateAchievementProgression(eGA_Party1999, GPC);
					}

					// Won the match in a public game...
					if (bWonRound &&
						IsPublicMatch())
					{
						switch (EndRoundData.GameType)
						{
							// Check for eGA_ItTakesTwo achievement
							case eGEARMP_Wingman:
								// Won the match
								if (EndRoundData.MyGRI.GameStatus == GS_EndMatch)
								{
									GPC.ProfileSettings.UpdateAchievementProgression(eGA_ItTakesTwo, GPC);
								}
								break;
							// Check for eGA_TheOldBallAndChain achievement
							case eGEARMP_CTM:
								// See if this local player captured a meatflag
								if (LocalPRI == EndRoundData.MyGRI.MeatflagKidnapper)
								{
									GPC.ProfileSettings.UpdateAchievementProgression(eGA_TheOldBallAndChain, GPC);
								}
								break;
							// Check for eGA_ItsGoodToBeTheKing achievement
							case eGEARMP_KTL:
								// See if this local player is the leader
								/*
								if (LocalPRI.bIsLeader)
								{
									GPC.ProfileSettings.UpdateAchievementProgression(eGA_ItsGoodToBeTheKing, GPC);
								}
								*/
								break;
							// Check for eGA_YouGoAheadIllBeFine achievement
							case eGEARMP_KOTH:
								// Won the match
								if (EndRoundData.MyGRI.GameStatus == GS_EndMatch)
								{
									GPC.ProfileSettings.UpdateAchievementProgression(eGA_YouGoAheadIllBeFine, GPC);
								}
								break;
						}
					}
				}
			}
		}
	}
}

/** Update the widget styles, strings, etc. */
function UpdateWidgetData(float DeltaTime)
{
	local delegate<UpdateWidgetValues> UpdateDelegate;

	if ( EndRoundData.MyGRI != None )
	{
		if ( PreviousGameStatus != EndRoundData.MyGRI.GameStatus )
		{
			GameStatusHasReplicated();
			PreviousGameStatus = EndRoundData.MyGRI.GameStatus;
		}

		if ( (EndRoundData.MyGRI.GameStatus == GS_RoundOver) || (EndRoundData.MyGRI.GameStatus == GS_EndMatch) )
		{
			// Set visible (needed so that the scene is not seen until the GRI is ready)
			SetVisibility( TRUE );

			UpdateDelegate = GameSpecificData[EndRoundData.GameType].UpdateWidgetValuesDelegate;
			UpdateDelegate();
		}
	}
}

/** Initializes the widget references for the UI scene */
final function InitializeWidgetReferences()
{
	//-------- Non-team and non-player specific Widgets -------------------------
	EndRoundData.TitleLabel =		UILabel(FindChild('lblTitle', true));
	EndRoundData.TeamScoreLabel =	UILabel(FindChild('lblScore', true));
	EndRoundData.TeamLogoImage =	UIImage(FindChild('imgTeamLogo', true));
	EndRoundData.BackgroundImage =	UIImage(FindChild('imgColorBackground', true));
	EndRoundData.TimeLabel =		UILabel(FindChild('lblTime', true));

	// Empty the player lists
	EndRoundData.MainPanelData.PlayerDataList.length = 0;
	EndRoundData.AlternatePanelData.PlayerDataList.length = 0;

	// Initialize the rest of the widgets (combat trials does things differently so we
	// branch the code here
	if ( EndRoundData.GameType == eGEARMP_CombatTrials )
	{
		InitializeWidgetReferences_CombatTrials();
	}
	else
	{
		InitializeWidgetReferences_Generic();
	}
}

/** Initialized the widgets for a typical MP match */
final function InitializeWidgetReferences_Generic()
{
	local int PlayerIdx, NumPlayers;
	local string LabelName;

	//-------- Initialize the main panel widgets ---------------------

	EndRoundData.MainPanelData.ParentPanel = UIPanel(FindChild('pnlHighlight0', true));

	// There are 2 player/team areas to this panel
	NumPlayers = 2;
	EndRoundData.MainPanelData.PlayerDataList.length = NumPlayers;
	for ( PlayerIdx = 0; PlayerIdx < NumPlayers; PlayerIdx++ )
	{
		LabelName = "lblPlayer" $ PlayerIdx;
		EndRoundData.MainPanelData.PlayerDataList[PlayerIdx].NameLabel = UILabel(EndRoundData.MainPanelData.ParentPanel.FindChild(Name(LabelName), true));
		LabelName = "lblCallOut" $ PlayerIdx;
		EndRoundData.MainPanelData.PlayerDataList[PlayerIdx].TitleLabel = UILabel(EndRoundData.MainPanelData.ParentPanel.FindChild(Name(LabelName), true));
	}

	//-------- Initialize the alternate panel widgets ---------------------------

	EndRoundData.AlternatePanelData.ParentPanel = UIPanel(FindChild('pnlHighlight1', true));

	// There is only 1 player/team areas to this panel (but we will treat it as many incase there is a change
	NumPlayers = 1;
	EndRoundData.AlternatePanelData.PlayerDataList.length = NumPlayers;
	for ( PlayerIdx = 0; PlayerIdx < NumPlayers; PlayerIdx++ )
	{
		LabelName = "lblPlayer" $ PlayerIdx;
		EndRoundData.AlternatePanelData.PlayerDataList[PlayerIdx].NameLabel = UILabel(EndRoundData.AlternatePanelData.ParentPanel.FindChild(Name(LabelName), true));
		LabelName = "lblCallOut" $ PlayerIdx;
		EndRoundData.AlternatePanelData.PlayerDataList[PlayerIdx].TitleLabel = UILabel(EndRoundData.AlternatePanelData.ParentPanel.FindChild(Name(LabelName), true));
	}
}

/** Initialized the widgets for a CombatTrials match */
final function InitializeWidgetReferences_CombatTrials()
{
	local int PlayerIdx, NumPlayers;
	local string LabelName;

	//-------- Initialize the main panel widgets ---------------------

	EndRoundData.MainPanelData.ParentPanel = UIPanel(FindChild('pnlHighlight0', true));

	// There are 5 player/team areas to this panel
	NumPlayers = 5;
	EndRoundData.MainPanelData.PlayerDataList.length = NumPlayers;
	for ( PlayerIdx = 0; PlayerIdx < NumPlayers; PlayerIdx++ )
	{
		LabelName = "lblPlayer" $ PlayerIdx;
		EndRoundData.MainPanelData.PlayerDataList[PlayerIdx].NameLabel = UILabel(EndRoundData.MainPanelData.ParentPanel.FindChild(Name(LabelName), true));
		LabelName = "lblScore" $ PlayerIdx;
		EndRoundData.MainPanelData.PlayerDataList[PlayerIdx].ScoreLabel = UILabel(EndRoundData.MainPanelData.ParentPanel.FindChild(Name(LabelName), true));
		LabelName = "imgDead" $ PlayerIdx;
		EndRoundData.MainPanelData.PlayerDataList[PlayerIdx].DeadImage = UIImage(EndRoundData.MainPanelData.ParentPanel.FindChild(Name(LabelName), true));
	}
}

/** Set the widgets to display a draw */
final function SetWidgetsForStalemate()
{
	// Hide the player/team panels
	EndRoundData.MainPanelData.ParentPanel.SetVisibility( false );
	EndRoundData.AlternatePanelData.ParentPanel.SetVisibility( false );

	// Title
	EndRoundData.TitleLabel.SetDataStoreBinding( TeamDraw );
	EndRoundData.TitleLabel.SetWidgetStyleByName( 'String Style', 'cmb_Draw' );

	// Team score (hide)
	EndRoundData.TeamScoreLabel.SetVisibility( false );

	// Background
	EndRoundData.BackgroundImage.SetWidgetStyleByName( 'Image Style', 'img_EndofRound_Draw' );

	// Logo
	EndRoundData.TeamLogoImage.SetWidgetStyleByName( 'Image Style', 'img_Logo_Draw' );

	// Time round took
	EndRoundData.TimeLabel.SetDataStoreBinding( FormatTime(EndRoundData.MyGRI.RoundEndTime - EndRoundData.MyGRI.NumSecondsUntilNextRound) );
}

/** Get the name of the style to use for displaying a PRI string */
final function Name GetPRIStringStyleName( GearPRI PRIToTest, GearPRI LocalPRI1, GearPRI LocalPRI2, optional TeamInfo TeamOfPRI )
{
	local GearTeamInfo TeamInfoToTest;

	if ( (LocalPRI1 != None) && (PRIToTest == LocalPRI1) )
	{
		return 'cmb_Scoreboard_P1';
	}
	else if ( (LocalPRI2 != None) && (PRIToTest == LocalPRI2) )
	{
		return 'cmb_Scoreboard_P2';
	}
	else
	{
		TeamInfoToTest = (TeamOfPRI == None) ? None : GearTeamInfo(TeamOfPRI);
		return ((GetPRIRace(PRIToTest, TeamInfoToTest) == eGEARRACE_COG) ? 'cmb_CogSM' : 'cmb_LocustSM');
	}
}

/** Called to set the strings and images for Warzone */
final function UpdateWidgetValues_LastManStanding( optional bool bIsKTL )
{
	local TeamInfo TeamWinner, LastLoser;
	local string TeamString, LastAliveName, KillMessage;
	local GearPRI PRIWinner, MyGearPRI1, MyGearPRI2, PRIKilled, DBNOPRI, KillerPRI, LastToDiePRI;
	local EGearRaceTypes WinningRace;
	local int TeamIdx, HighestLossCount;

	// Grab the team
	TeamWinner = TeamInfo(EndRoundData.MyGRI.Winner);

	// If there was a stalemate, set the widgets and exit
	if ( TeamWinner == None )
	{
		SetWidgetsForStalemate();
		return;
	}

	// Grab the last PRI killer
	PRIWinner = GearPRI(EndRoundData.MyGRI.FinalKiller);

	// Determine the race of the winner
	WinningRace = GetPRIRace(FindPRIOnTeam(TeamWinner.TeamIndex, EndRoundData.MyGRI));
	// Get the team string for the title label
	if ( EndRoundData.GameType == eGEARMP_Wingman )
	{
		TeamString = EndRoundData.MyGRI.Caps( GetTeamName(None, EndRoundData.MyGRI, TeamWinner.TeamIndex) );
	}
	else
	{
		TeamString = (WinningRace == eGEARRACE_COG) ? TeamCOG : TeamLOC;
	}

	// Title string - (TEAM HAS WON THE MATCH!)
	if ( EndRoundData.MyGRI.GameStatus == GS_EndMatch )
	{
		EndRoundData.TitleLabel.SetDataStoreBinding( Repl(TeamMatchWin, "*t", TeamString) );
	}
	// Title string - (TEAM VICTORY!)
	else
	{
		EndRoundData.TitleLabel.SetDataStoreBinding( Repl((EndRoundData.GameType == eGEARMP_Wingman) ? TeamSurvived : TeamRoundWin, "*t", TeamString) );
	}

	// Score string
	EndRoundData.TeamScoreLabel.SetDataStoreBinding( string(int(TeamWinner.Score)) );
	// Time round took
	EndRoundData.TimeLabel.SetDataStoreBinding( FormatTime(EndRoundData.MyGRI.RoundEndTime - EndRoundData.MyGRI.NumSecondsUntilNextRound) );

	// Set the team styles
	if ( WinningRace == eGEARRACE_COG )
	{
		EndRoundData.TitleLabel.SetWidgetStyleByName( 'String Style', 'cmb_Cog' );
		EndRoundData.TeamLogoImage.SetWidgetStyleByName( 'Image Style', 'img_Logo_Cog' );
		EndRoundData.BackgroundImage.SetWidgetStyleByName( 'Image Style', 'img_EndofRound_Cog' );
		EndRoundData.TeamScoreLabel.SetWidgetStyleByName( 'String Style', 'cmb_Cog' );
	}
	else
	{
		EndRoundData.TitleLabel.SetWidgetStyleByName( 'Image Style', 'cmb_Locust' );
		EndRoundData.TeamLogoImage.SetWidgetStyleByName( 'Image Style', 'img_Logo_Locust' );
		EndRoundData.BackgroundImage.SetWidgetStyleByName( 'Image Style', 'img_EndofRound_Locust' );
		EndRoundData.TeamScoreLabel.SetWidgetStyleByName( 'String Style', 'cmb_Locust' );
	}

	// Grab a reference to the local player's PRI
	GetLocalPRIs( MyGearPRI1, MyGearPRI2 );

	// Set strings and styles for the players involved with the win
	if ( PRIWinner != None )
	{
		PRIKilled = GearPRI(PRIWinner.LastIKilledPRI);
		if ( PRIKilled != None )
		{
			KillerPRI = GearPRI(PRIKilled.LastToKillMePRI);
			DBNOPRI = (PRIKilled.LastToDBNOMePRI != None) ? GearPRI(PRIKilled.LastToDBNOMePRI) : KillerPRI;
		}
	}

	if ( (PRIWinner != None) && (EndRoundData.MyGRI.bGameIsExecutionRules || bIsKTL) && !EndRoundData.MyGRI.bFinalFriendlyKill )
	{
		// Check for suicide
		if ( (PRIKilled == KillerPRI) && (PRIKilled == DBNOPRI) )
		{
			EndRoundData.MainPanelData.ParentPanel.SetVisibility( false );
			EndRoundData.AlternatePanelData.ParentPanel.SetVisibility( true );

			// Suicide title/player
			EndRoundData.AlternatePanelData.PlayerDataList[0].TitleLabel.SetDataStoreBinding( bIsKTL ? LeaderSuicide : FinalSuicide );
			EndRoundData.AlternatePanelData.PlayerDataList[0].NameLabel.SetDataStoreBinding( (PRIKilled != None) ? PRIKilled.PlayerName : "" );
			EndRoundData.AlternatePanelData.PlayerDataList[0].NameLabel.SetWidgetStyleByName( 'String Style', GetPRIStringStyleName(PRIKilled, MyGearPRI1, MyGearPRI2) );
		}
		// Execution rules (and not a suicide) so hide the alternate panel since we need both player spots that the main panel provides
		else
		{
			EndRoundData.MainPanelData.ParentPanel.SetVisibility( true );
			EndRoundData.AlternatePanelData.ParentPanel.SetVisibility( false );

			// Takedown title/player
			EndRoundData.MainPanelData.PlayerDataList[0].TitleLabel.SetDataStoreBinding( FinalTakedown );
			EndRoundData.MainPanelData.PlayerDataList[0].NameLabel.SetDataStoreBinding( DBNOPRI.PlayerName );
			EndRoundData.MainPanelData.PlayerDataList[0].NameLabel.SetWidgetStyleByName( 'String Style', GetPRIStringStyleName(DBNOPRI, MyGearPRI1, MyGearPRI2) );

			// Killer title/player
			EndRoundData.MainPanelData.PlayerDataList[1].TitleLabel.SetDataStoreBinding( FinalExecute );
			EndRoundData.MainPanelData.PlayerDataList[1].NameLabel.SetDataStoreBinding( KillerPRI.PlayerName );
			EndRoundData.MainPanelData.PlayerDataList[1].NameLabel.SetWidgetStyleByName( 'String Style', GetPRIStringStyleName(KillerPRI, MyGearPRI1, MyGearPRI2) );
		}
	}
	else
	{
		// Not execution rules so we only need one player spot, so hide the main panel
		EndRoundData.MainPanelData.ParentPanel.SetVisibility( false );
		EndRoundData.AlternatePanelData.ParentPanel.SetVisibility( true );

		// Suicide/Disconnect title/player
		if ( PRIWinner == None )
		{
			// Wingman can end with a team winning the match on a round that was stalemated so the kill section will never be correct so hide it
			if ( EndRoundData.GameType == eGEARMP_Wingman )
			{
				EndRoundData.AlternatePanelData.ParentPanel.SetVisibility( false );
			}
			else
			{
				for ( TeamIdx = 0; TeamIdx < EndRoundData.MyGRI.Teams.length; TeamIdx++ )
				{
					// Figure out which team died last and record the name of the last player standing from that team
					if ( TeamIdx < ArrayCount(EndRoundData.MyGRI.TeamIndexLossOrder) &&
						HighestLossCount < EndRoundData.MyGRI.TeamIndexLossOrder[TeamIdx] )
					{
						LastLoser = EndRoundData.MyGRI.Teams[TeamIdx];
						LastAliveName = EndRoundData.MyGRI.LastStandingOnTeamName[TeamIdx];
						HighestLossCount = EndRoundData.MyGRI.TeamIndexLossOrder[TeamIdx];
					}
				}

				// See if this was an environment death
				if ( LastAliveName == "" || (LastAliveName != "" && EndRoundData.MyGRI.FinalVictim != None && GearPRI(EndRoundData.MyGRI.FinalVictim).PlayerName == LastAliveName) )
				{
					LastToDiePRI = GearPRI(EndRoundData.MyGRI.FinalVictim);
					LastAliveName = LastToDiePRI.PlayerName;
				}

				// Killer title/player
				EndRoundData.AlternatePanelData.PlayerDataList[0].TitleLabel.SetDataStoreBinding( IsEnvironmentDamageDeath(LastToDiePRI) ? FinalEnvironmentDeath : (bIsKTL ? LeaderDisconnect : FinalDisconnectString) );
				EndRoundData.AlternatePanelData.PlayerDataList[0].NameLabel.SetDataStoreBinding( LastAliveName );
				EndRoundData.AlternatePanelData.PlayerDataList[0].NameLabel.SetWidgetStyleByName( 'String Style', GetPRIStringStyleName(LastToDiePRI, MyGearPRI1, MyGearPRI2, LastLoser) );
			}
		}
		else
		{
			// Killer title/player
			if ( DBNOPRI == DBNOPRI.LastIKilledPRI )
			{
				KillMessage = FinalSuicide;
			}
			else if ( EndRoundData.MyGRI.bFinalFriendlyKill )
			{
				KillMessage = FinalFriendly;
			}
			else
			{
				KillMessage = FinalKill;
			}
			EndRoundData.AlternatePanelData.PlayerDataList[0].TitleLabel.SetDataStoreBinding( KillMessage );
			EndRoundData.AlternatePanelData.PlayerDataList[0].NameLabel.SetDataStoreBinding( DBNOPRI.PlayerName );
			EndRoundData.AlternatePanelData.PlayerDataList[0].NameLabel.SetWidgetStyleByName( 'String Style', GetPRIStringStyleName(DBNOPRI, MyGearPRI1, MyGearPRI2) );
		}
	}
}

/** Whether the damage type is from the environment or not */
final function bool IsEnvironmentDamageDeath( GearPRI DeadPRI )
{
	if ( DeadPRI != None )
	{
		if ( DeadPRI.DamageTypeToKillMe.default.bEnvironmentalDamage )
		{
			return TRUE;
		}
	}

	return FALSE;
}

/** Called to set the strings and images for Warzone */
final function UpdateWidgetValues_Warzone()
{
	UpdateWidgetValues_LastManStanding();
}

/** Called to set the strings and images for KTL */
final function UpdateWidgetValues_KTL()
{
	UpdateWidgetValues_LastManStanding();
}

/** Called to set the strings and images for Combat Trials */
final function UpdateWidgetValues_CombatTrials()
{
	local EGearRaceTypes WinningRace;
	local int PlayerIdx, PRIIdx, SortedIdx, NumAlive;
	local array<GearPRI> SortedPRIs;
	local GearPRI CurrPRI, MyGearPRI1, MyGearPRI2;
	local string ScoreString;

	// Find all of the players and sort them from highest to lowest scores
	for ( PRIIdx = 0; PRIIdx < EndRoundData.MyGRI.PRIArray.length; PRIIdx++ )
	{
		if ( EndRoundData.MyGRI.PRIArray[PRIIdx] != None )
		{
			CurrPRI = GearPRI(EndRoundData.MyGRI.PRIArray[PRIIdx]);
			if ( CurrPRI.GetTeamNum() == 0 )
			{
				for ( SortedIdx = 0; SortedIdx < SortedPRIs.length; SortedIdx++ )
				{
					if ( CurrPRI.GetScore() > SortedPRIs[SortedIdx].GetScore() )
					{
						SortedPRIs.InsertItem( SortedIdx, CurrPRI );
						break;
					}
				}

				// Didn't find a place to insert so append
				if ( SortedIdx >= SortedPRIs.length )
				{
					SortedPRIs.AddItem( CurrPRI );
				}

				// Keep track of how many are alive
				if ( !CurrPRI.bIsDead )
				{
					NumAlive++;
				}
			}
		}
	}

	WinningRace = (NumAlive > 0) ? eGEARRACE_COG : eGEARRACE_Locust;

	// Time round took
	EndRoundData.TimeLabel.SetDataStoreBinding( FormatTime(EndRoundData.MyGRI.RoundEndTime - EndRoundData.MyGRI.NumSecondsUntilNextRound) );

	// FAILED
	if ( WinningRace == eGEARRACE_Locust )
	{
		// Title string
		EndRoundData.TitleLabel.SetDataStoreBinding( CTWaveFail );

		// Non player styles
		EndRoundData.TitleLabel.SetWidgetStyleByName( 'Image Style', 'cmb_Locust' );
		EndRoundData.TeamLogoImage.SetWidgetStyleByName( 'Image Style', 'img_Logo_Cog' );
		EndRoundData.BackgroundImage.SetWidgetStyleByName( 'Image Style', 'img_EndofRound_Locust' );
		EndRoundData.TeamScoreLabel.SetWidgetStyleByName( 'String Style', 'cmb_Locust' );
	}
	// SUCCESS
	else
	{
		// Title string - (ALL WAVES COMPLETED)
		if ( EndRoundData.MyGRI.InvasionCurrentWaveIndex >= EndRoundData.MyGRI.InvasionNumTotalWaves )
		{
			EndRoundData.TitleLabel.SetDataStoreBinding( CTWaveWinAll );
		}
		// Title string - (WAVE COMPLETED)
		else
		{
			EndRoundData.TitleLabel.SetDataStoreBinding( CTWaveWin );
		}

		// Non player styles
		EndRoundData.TitleLabel.SetWidgetStyleByName( 'String Style', 'cmb_Cog' );
		EndRoundData.TeamLogoImage.SetWidgetStyleByName( 'Image Style', 'img_Logo_Cog' );
		EndRoundData.BackgroundImage.SetWidgetStyleByName( 'Image Style', 'img_EndofRound_Cog' );
		EndRoundData.TeamScoreLabel.SetWidgetStyleByName( 'String Style', 'cmb_Cog' );
	}

	ScoreString = EndRoundData.MyGRI.InvasionCurrentWaveIndex $ "/" $ EndRoundData.MyGRI.InvasionNumTotalWaves;
	EndRoundData.TeamScoreLabel.SetDataStoreBinding( ScoreString );

	// Grab a reference to the local player's PRI
	GetLocalPRIs( MyGearPRI1, MyGearPRI2 );

	// Set the player values
	EndRoundData.MainPanelData.ParentPanel.SetVisibility( true );
	for ( PlayerIdx = 0; PlayerIdx < EndRoundData.MainPanelData.PlayerDataList.length; PlayerIdx++ )
	{
		if ( SortedPRIs.length > PlayerIdx )
		{
			EndRoundData.MainPanelData.PlayerDataList[PlayerIdx].NameLabel.SetDataStoreBinding( SortedPRIs[PlayerIdx].PlayerName );
			EndRoundData.MainPanelData.PlayerDataList[PlayerIdx].NameLabel.SetWidgetStyleByName( 'String Style', GetPRIStringStyleName(SortedPRIs[PlayerIdx], MyGearPRI1, MyGearPRI2) );
			EndRoundData.MainPanelData.PlayerDataList[PlayerIdx].ScoreLabel.SetDataStoreBinding( string(SortedPRIs[PlayerIdx].GetScore()) );
			EndRoundData.MainPanelData.PlayerDataList[PlayerIdx].ScoreLabel.SetWidgetStyleByName( 'String Style', GetPRIStringStyleName(SortedPRIs[PlayerIdx], MyGearPRI1, MyGearPRI2) );

			if ( SortedPRIs[PlayerIdx].bIsDead )
			{
				EndRoundData.MainPanelData.PlayerDataList[PlayerIdx].NameLabel.ActivateStateByClass( class'UIState_Disabled', 0 );
				EndRoundData.MainPanelData.PlayerDataList[PlayerIdx].ScoreLabel.ActivateStateByClass( class'UIState_Disabled', 0 );
				EndRoundData.MainPanelData.PlayerDataList[PlayerIdx].DeadImage.SetVisibility( true );
			}
			else
			{
				EndRoundData.MainPanelData.PlayerDataList[PlayerIdx].NameLabel.ActivateStateByClass( class'UIState_Enabled', 0 );
				EndRoundData.MainPanelData.PlayerDataList[PlayerIdx].ScoreLabel.ActivateStateByClass( class'UIState_Enabled', 0 );
				EndRoundData.MainPanelData.PlayerDataList[PlayerIdx].DeadImage.SetVisibility( false );
			}
		}
		else
		{
			EndRoundData.MainPanelData.PlayerDataList[PlayerIdx].NameLabel.SetVisibility( false );
			EndRoundData.MainPanelData.PlayerDataList[PlayerIdx].ScoreLabel.SetVisibility( false );
			EndRoundData.MainPanelData.PlayerDataList[PlayerIdx].DeadImage.SetVisibility( false );
		}
	}
}

/** Called to set the strings and images for Annex/KOTH */
final function UpdateWidgetValues_Annex()
{
	local TeamInfo TeamWinner;
	local string TeamString;
	local EGearRaceTypes WinningRace;

	// Grab the team
	TeamWinner = TeamInfo(EndRoundData.MyGRI.Winner);

	// Determine the race of the winner
	WinningRace = GetPRIRace(FindPRIOnTeam(TeamWinner.TeamIndex, EndRoundData.MyGRI));
	// Get the team string for the title label
	TeamString = (WinningRace == eGEARRACE_COG) ? TeamCOG : TeamLOC;

	// Use the alternate panel
	EndRoundData.MainPanelData.ParentPanel.SetVisibility( false );
	EndRoundData.AlternatePanelData.ParentPanel.SetVisibility( true );

	// Title string - (TEAM HAS WON THE MATCH!)
	if ( EndRoundData.MyGRI.GameStatus == GS_EndMatch )
	{
		EndRoundData.TitleLabel.SetDataStoreBinding( Repl(TeamMatchWin, "*t", TeamString) );
	}
	// Title string - (TEAM VICTORY!)
	else
	{
		EndRoundData.TitleLabel.SetDataStoreBinding( Repl(TeamRoundWin, "*t", TeamString) );
	}

	// Score string
	EndRoundData.TeamScoreLabel.SetDataStoreBinding( string(int(TeamWinner.Score)) );

	// Winner title
	EndRoundData.AlternatePanelData.PlayerDataList[0].TitleLabel.SetDataStoreBinding( ScoreMsg );

	// Time round took
	EndRoundData.TimeLabel.SetDataStoreBinding( FormatTime(EndRoundData.MyGRI.RoundEndTime - EndRoundData.MyGRI.NumSecondsUntilNextRound) );

	// Set the team styles
	if ( WinningRace == eGEARRACE_COG )
	{
		EndRoundData.TitleLabel.SetWidgetStyleByName( 'String Style', 'cmb_Cog' );
		EndRoundData.TeamLogoImage.SetWidgetStyleByName( 'Image Style', 'img_Logo_Cog' );
		EndRoundData.BackgroundImage.SetWidgetStyleByName( 'Image Style', 'img_EndofRound_Cog' );
		EndRoundData.TeamScoreLabel.SetWidgetStyleByName( 'String Style', 'cmb_Cog' );
		EndRoundData.AlternatePanelData.PlayerDataList[0].NameLabel.SetDataStoreBinding( string(int(EndRoundData.MyGRI.GameScore[0] - EndRoundData.MyGRI.GameScore[1])) );
		EndRoundData.AlternatePanelData.PlayerDataList[0].NameLabel.SetWidgetStyleByName( 'String Style', 'cmb_CogSM' );
	}
	else
	{
		EndRoundData.TitleLabel.SetWidgetStyleByName( 'Image Style', 'cmb_Locust' );
		EndRoundData.TeamLogoImage.SetWidgetStyleByName( 'Image Style', 'img_Logo_Locust' );
		EndRoundData.BackgroundImage.SetWidgetStyleByName( 'Image Style', 'img_EndofRound_Locust' );
		EndRoundData.TeamScoreLabel.SetWidgetStyleByName( 'String Style', 'cmb_Locust' );
		EndRoundData.AlternatePanelData.PlayerDataList[0].NameLabel.SetDataStoreBinding( string(int(EndRoundData.MyGRI.GameScore[1] - EndRoundData.MyGRI.GameScore[0])) );
		EndRoundData.AlternatePanelData.PlayerDataList[0].NameLabel.SetWidgetStyleByName( 'String Style', 'cmb_LocustSM' );
	}
}

/** Called to set the strings and images for Wingman */
final function UpdateWidgetValues_Wingman()
{
	UpdateWidgetValues_LastManStanding();
}

/** Called to set the strings and images for Meatflag */
final function UpdateWidgetValues_CTM()
{
	local TeamInfo TeamWinner;
	local string TeamString;
	local EGearRaceTypes WinningRace;
	local GearPRI MyGearPRI1, MyGearPRI2;

	// Grab the team
	TeamWinner = TeamInfo(EndRoundData.MyGRI.Winner);

	// Determine the race of the winner
	WinningRace = GetPRIRace(FindPRIOnTeam(TeamWinner.TeamIndex, EndRoundData.MyGRI));
	// Get the team string for the title label
	TeamString = (WinningRace == eGEARRACE_COG) ? TeamCOG : TeamLOC;

	// Use the alternate panel
	EndRoundData.MainPanelData.ParentPanel.SetVisibility( false );
	EndRoundData.AlternatePanelData.ParentPanel.SetVisibility( true );

	// Title string - (TEAM HAS WON THE MATCH!)
	if ( EndRoundData.MyGRI.GameStatus == GS_EndMatch )
	{
		EndRoundData.TitleLabel.SetDataStoreBinding( Repl(TeamMatchWin, "*t", TeamString) );
	}
	// Title string - (TEAM VICTORY!)
	else
	{
		EndRoundData.TitleLabel.SetDataStoreBinding( Repl(TeamRoundWin, "*t", TeamString) );
	}

	// Score string
	EndRoundData.TeamScoreLabel.SetDataStoreBinding( string(int(TeamWinner.Score)) );

	// Winner title
	EndRoundData.AlternatePanelData.PlayerDataList[0].TitleLabel.SetDataStoreBinding( FinalCapture );

	// Time round took
	EndRoundData.TimeLabel.SetDataStoreBinding( FormatTime(EndRoundData.MyGRI.RoundEndTime - EndRoundData.MyGRI.NumSecondsUntilNextRound) );

	// Set the team styles
	if ( WinningRace == eGEARRACE_COG )
	{
		EndRoundData.TitleLabel.SetWidgetStyleByName( 'String Style', 'cmb_Cog' );
		EndRoundData.TeamLogoImage.SetWidgetStyleByName( 'Image Style', 'img_Logo_Cog' );
		EndRoundData.BackgroundImage.SetWidgetStyleByName( 'Image Style', 'img_EndofRound_Cog' );
		EndRoundData.TeamScoreLabel.SetWidgetStyleByName( 'String Style', 'cmb_Cog' );
	}
	else
	{
		EndRoundData.TitleLabel.SetWidgetStyleByName( 'Image Style', 'cmb_Locust' );
		EndRoundData.TeamLogoImage.SetWidgetStyleByName( 'Image Style', 'img_Logo_Locust' );
		EndRoundData.BackgroundImage.SetWidgetStyleByName( 'Image Style', 'img_EndofRound_Locust' );
		EndRoundData.TeamScoreLabel.SetWidgetStyleByName( 'String Style', 'cmb_Locust' );
	}

	// Grab a reference to the local player's PRI
	GetLocalPRIs( MyGearPRI1, MyGearPRI2 );

	EndRoundData.AlternatePanelData.PlayerDataList[0].NameLabel.SetDataStoreBinding( EndRoundData.MyGRI.MeatflagKidnapper.PlayerName );
	EndRoundData.AlternatePanelData.PlayerDataList[0].NameLabel.SetWidgetStyleByName( 'String Style', GetPRIStringStyleName(EndRoundData.MyGRI.MeatflagKidnapper, MyGearPRI1, MyGearPRI2) );
}


defaultproperties
{
	OnGearUISceneTick=UpdateWidgetData

	GameSpecificData(eGEARMP_Warzone)=(UpdateWidgetValuesDelegate=GearUISceneEndOfRound.UpdateWidgetValues_Warzone)
	GameSpecificData(eGEARMP_Execution)=(UpdateWidgetValuesDelegate=GearUISceneEndOfRound.UpdateWidgetValues_Warzone)
	GameSpecificData(eGEARMP_KTL)=(UpdateWidgetValuesDelegate=GearUISceneEndOfRound.UpdateWidgetValues_KTL)
	GameSpecificData(eGEARMP_CombatTrials)=(UpdateWidgetValuesDelegate=GearUISceneEndOfRound.UpdateWidgetValues_CombatTrials)
	GameSpecificData(eGEARMP_Annex)=(UpdateWidgetValuesDelegate=GearUISceneEndOfRound.UpdateWidgetValues_Annex)
	GameSpecificData(eGEARMP_Wingman)=(UpdateWidgetValuesDelegate=GearUISceneEndOfRound.UpdateWidgetValues_Wingman)
	GameSpecificData(eGEARMP_KOTH)=(UpdateWidgetValuesDelegate=GearUISceneEndOfRound.UpdateWidgetValues_Annex)
	GameSpecificData(eGEARMP_CTM)=(UpdateWidgetValuesDelegate=GearUISceneEndOfRound.UpdateWidgetValues_CTM)
}
