/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUISceneEOR_Horde extends GearUISceneMP_Base
	ClassRedirect(GearUIScene_EORHorde)
	Config(UI);


/************************************************************************/
/* Constants, structures, enums, etc.									*/
/************************************************************************/

/** Structure containing all widgets and data for a player */
struct GearEORPlayerData
{
	/** Reference to the parent panel for this player */
	var transient UIPanel ParentPanel;

	/** Reference to the label displaying the player's name */
	var transient UILabel NameLabel;

	/** Reference to the label displaying the player's rank */
	var transient UILabel RankLabel;

	/** Reference to the image displayed to show whether a player is dead or not */
	var transient UIImage StatusImage;

	/** Reference to the label displayed to show the score for the player */
	var transient UILabel ScoreLabel;

	/** Reference to the label displaying the number of kills (slashes) the player earned */
	var transient UILabel KillLabel;

	/** Reference to the image of the scratchie that is displayed to show a player is dead */
	var transient UIImage ScratchImage;

	/** Reference to the image for displaying the chat icon for the player */
	var transient UIImage ChatImage;

	/** The PRI for the player this UI represents */
	var transient GearPRI PRI;

	/** Previous number of kill slashes we showed */
	var transient int LastNumKillSlashes;
};

/** Structure containing all the widgets and data needed for each score tally */
struct GearEORScoreTallyData
{
	/** Reference to the label that describes the score tally */
	var transient UILabel DescLabel;

	/** Reference to the label of the actual score value */
	var transient UILabel ScoreLabel;

	/** Reference to the background image of the score tally */
	var transient UIImage ScoreBGImage;
};

/** Enum of the different states the scene will be in */
enum EGearEORState
{
	eGEAREORSTATE_Initialize,
	eGEAREORSTATE_PlayerScore,
	eGEAREORSTATE_TeamScore,
	eGEAREORSTATE_Survival,
	eGEAREORSTATE_Difficulty,
	eGEAREORSTATE_WaveScore,
	eGEAREORSTATE_TeamTotal,
	eGEAREORSTATE_AllDone,
};

/** The opacity of the tally scores when the beginning */
var const float DefaultScoreOpacity;

/** The opacity of the tally score when at the end */
var const float EndingScoreOpacity;

/** The time it takes to fade the tally score in */
var const float TallyFadeInTime;

/** The time it takes to fade the tally score out */
var const float TallyFadeOutTime;

/** Amount of time the player score counting occurs per slash */
var const float TotalPlayerSlashTime;

/** Amount of time each score tally will last */
var const float TotalTallyTime;

/** Amount of time the total team score will take to reach the total */
var const float TotalTeamScoreTime;

/** The time it takes to fade the total team score in */
var const float TotalTeamScoreFadeInTime;

/** The time it takes to fade the total team score out */
var const float TotalTeamScoreFadeOutTime;

/** Amount of time we remain on the scene after all totals are finished animating */
var const float TotalAllDoneWaitTime;


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Label displaying the wave number that just ended */
var transient UIImage BGImage;

/** Label displaying the wave number that just ended */
var transient UILabel WaveLabel;

/** Label displaying how much time the last wave took */
var transient UILabel TimeLabel;

/** Label to display the result of the game */
var transient UILabel ResultLabel;

/** List of player data to display in the scene */
var transient array<GearEORPlayerData> PlayerData;

/** Data for the team score before adjustments */
var transient GearEORScoreTallyData TeamScore;

/** Data for the survival bonus */
var transient GearEORScoreTallyData SurvivalBonus;

/** Data for the difficulty multiplier */
var transient GearEORScoreTallyData DifficultyMultiplier;

/** Data for the total score for this wave */
var transient GearEORScoreTallyData TeamScoreAdjusted;

/** Data for the total score across all waves */
var transient GearEORScoreTallyData TotalTeamScore;

/** Reference to the blood splat image displayed when everyone fails */
var transient UIImage SplatImage;

/** Reference to the waiting for host message that shows up when you fail */
var transient UILabel WaitForHostLabel;

/** Whether the scene has been initialized or not */
var transient bool bEndOfRoundInitialized;

/** Local reference to the GRI */
var transient GearGRI GRI;

/** The current state the scene is in */
var transient EGearEORState EORState;

/** The time the current EORState began */
var transient float StateStartTime;

/** Whether the opacities have hit their peak */
var transient bool bOpacitiesHitPeak;

/** Total number of player in the game */
var transient int NumPlayerInGame;

/** The number of players left alive */
var transient int NumPlayersAlive;

/** Slashes sound cue */
var SoundCue KillSlashSound;

/** Bonus sound cue */
var SoundCue BonusSound;

/** Total team Score sound */
var SoundCue TotalSound;

/** Total amount of time the slashes take to animate across */
var transient float TotalSlashTime;

/** Whether we've already handled the AllDone handling */
var transient bool bHandledAllDone;

/** Whether we've already told the scene to close or not */
var transient bool bClosingScene;

/** The gamestatus of the game - used so we can determine if we JUST replicated the gamestatus from the GRI */
var transient EGameStatus PreviousGameStatus;

/** Localized strings */
var localized string WaveHeaderString;
var localized string SurvivalBonusString;
var localized string Multiply;
var localized string WaveComplete;
var localized string WaveIncomplete;
var localized string HordeDefeated;

var transient float EORTime;

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Sets the state of the EOR scene */
function SetEORState( EGearEORState NewState )
{
	StateStartTime = EORTime;
	EORState = NewState;
	bOpacitiesHitPeak = false;
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
		GRI = GetGRI();
		SetEORState(eGEAREORSTATE_Initialize);
		InitializeWidgetReferences();

		EORTime = GRI.WorldInfo.TimeSeconds;
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

	// Turn chat delegates off
	TrackChat( false );
}

/** Goes through all the chat icons and updates them */
final function UpdateChatIcons()
{
	local int Idx;
	local GearPRI CurrPRI;
	local UIImage ChatIcon;

	for ( Idx = 0; Idx < PlayerData.length; Idx++ )
	{
		CurrPRI = PlayerData[Idx].PRI;
		ChatIcon = PlayerData[Idx].ChatImage;
		ChatIcon.Opacity = 0.0f;
		if ( CurrPRI != None )
		{
			if ( CurrPRI.ChatFadeValue > 0 )
			{
				CurrPRI.ChatFadeValue = 1.0f - (EORTime - CurrPRI.TaccomChatFadeStart) / CurrPRI.ChatFadeTime;
				ChatIcon.Opacity = CurrPRI.ChatFadeValue;
			}
		}
	}
}

/** Assigns all member variables to appropriate child widget from this scene */
function InitializeWidgetReferences()
{
	local int Idx;
	local string WidgetName;
	local UIPanel ParentPanel;

	// Player data widgets
	PlayerData.length = 5;
	for (Idx = 0; Idx < PlayerData.length; Idx++)
	{
		WidgetName = "pnlPlayer" $ Idx;
		PlayerData[Idx].ParentPanel	= UIPanel(FindChild(Name(WidgetName), true));
		ParentPanel = PlayerData[Idx].ParentPanel;

		PlayerData[Idx].NameLabel = UILabel(ParentPanel.FindChild('lblName', true));
		PlayerData[Idx].NameLabel.SetVisibility(false);

		PlayerData[Idx].RankLabel = UILabel(ParentPanel.FindChild('lblRank', true));
		PlayerData[Idx].RankLabel.SetVisibility(false);

		PlayerData[Idx].StatusImage = UIImage(ParentPanel.FindChild('imgStatus', true));
		PlayerData[Idx].StatusImage.SetVisibility(false);

		PlayerData[Idx].ScoreLabel = UILabel(ParentPanel.FindChild('lblTotal', true));
		PlayerData[Idx].ScoreLabel.SetVisibility(false);

		PlayerData[Idx].KillLabel = UILabel(ParentPanel.FindChild('lblKillCount', true));
		PlayerData[Idx].KillLabel.SetVisibility(false);

		PlayerData[Idx].ScratchImage = UIImage(ParentPanel.FindChild('imgStrike', true));
		PlayerData[Idx].ScratchImage.SetVisibility(false);

		WidgetName = "imgChat" $ Idx;
		PlayerData[Idx].ChatImage = UIImage(FindChild(Name(WidgetName), true));
		PlayerData[Idx].ChatImage.SetVisibility(true);
		PlayerData[Idx].ChatImage.Opacity = 0.0f;
	}

	// Team score widgets
	TeamScore.DescLabel		= UILabel(FindChild('lblTeamScore', true));
	TeamScore.ScoreLabel	= UILabel(TeamScore.DescLabel.FindChild('lblTeamScoreValue', true));
	TeamScore.ScoreLabel.SetDataStoreBinding(" ");
	TeamScore.ScoreBGImage	= UIImage(FindChild('imgTeamScoreBG', true));
	TeamScore.ScoreBGImage.SetVisibility(true);
	TeamScore.ScoreBGImage.Opacity = 0.0f;

	// Survival bonus widgets
	SurvivalBonus.DescLabel		= UILabel(FindChild('lblSurvival', true));
	SurvivalBonus.ScoreLabel	= UILabel(SurvivalBonus.DescLabel.FindChild('lblSurvivalValue', true));
	SurvivalBonus.ScoreLabel.SetDataStoreBinding(" ");
	SurvivalBonus.ScoreBGImage	= UIImage(FindChild('imgSurvivalBG', true));
	SurvivalBonus.ScoreBGImage.SetVisibility(true);
	SurvivalBonus.ScoreBGImage.Opacity = 0.0f;

	// Difficulty multiplier widgets
	DifficultyMultiplier.DescLabel		= UILabel(FindChild('lblDifficulty', true));
	DifficultyMultiplier.ScoreLabel		= UILabel(DifficultyMultiplier.DescLabel.FindChild('lblDifficultyValue', true));
	DifficultyMultiplier.ScoreLabel.SetDataStoreBinding(" ");
	DifficultyMultiplier.ScoreBGImage	= UIImage(FindChild('imgDifficultyBG', true));
	DifficultyMultiplier.ScoreBGImage.SetVisibility(true);
	DifficultyMultiplier.ScoreBGImage.Opacity = 0.0f;

	// Team wave score after adjustments widgets
	TeamScoreAdjusted.DescLabel		= UILabel(FindChild('lblWaveScore', true));
	TeamScoreAdjusted.ScoreLabel	= UILabel(TeamScoreAdjusted.DescLabel.FindChild('lbWaveScoreValue', true));
	TeamScoreAdjusted.ScoreLabel.SetDataStoreBinding(" ");
	TeamScoreAdjusted.ScoreBGImage	= UIImage(FindChild('imgWaveScoreBG', true));
	TeamScoreAdjusted.ScoreBGImage.SetVisibility(true);
	TeamScoreAdjusted.ScoreBGImage.Opacity = 0.0f;

	// Total team score across all waves widgets
	TotalTeamScore.ScoreLabel	= UILabel(FindChild('lblTotalTeam', true));
	TotalTeamScore.ScoreLabel.SetDataStoreBinding(" ");
	TotalTeamScore.ScoreBGImage	= UIImage(FindChild('imgTotalTeamBG', true));
	TotalTeamScore.ScoreBGImage.SetVisibility(true);
	TotalTeamScore.ScoreBGImage.Opacity = 0.0f;

	// No surviver blood splat
	SplatImage = UIImage(FindChild('imgSplat', true));
	SplatImage.SetVisibility(false);
	// No surviver message
	WaitForHostLabel = UILabel(FindChild('lblWaiting', true));
	WaitForHostLabel.SetVisibility(false);
}

/** Initialize the values to be drawn in the scene */
function InitializeSceneValues()
{
	local int PRIIdx, SortedIdx;
	local array<GearPRI> SortedPRIs;
	local GearPRI CurrPRI;

	NumPlayersAlive = 0;

	// Find all of the players and sort them from highest to lowest scores
	for (PRIIdx = 0; PRIIdx < GRI.PRIArray.length; PRIIdx++)
	{
		if (GRI.PRIArray[PRIIdx] != None && !GRI.PRIArray[PRIIdx].bIsInactive)
		{
			CurrPRI = GearPRI(GRI.PRIArray[PRIIdx]);
			if (CurrPRI.GetTeamNum() == 0)
			{
				for (SortedIdx = 0; SortedIdx < SortedPRIs.length; SortedIdx++)
				{
					if (CurrPRI.GetGameScore2() > SortedPRIs[SortedIdx].GetGameScore2())
					{
						SortedPRIs.InsertItem(SortedIdx, CurrPRI);
						break;
					}
				}

				// Didn't find a place to insert so append
				if (SortedIdx >= SortedPRIs.length)
				{
					SortedPRIs.AddItem(CurrPRI);
				}

				// Keep track of how many are alive
				if (!CurrPRI.bIsDead && !CurrPRI.bIsSpectator)
				{
					NumPlayersAlive++;
				}
				NumPlayerInGame++;
			}
		}
	}

	// Put the sorted players in their UI spots
	for (PRIIdx = 0; PRIIdx < SortedPRIs.length; PRIIdx++)
	{
		PlayerData[PRIIdx].PRI = SortedPRIs[PRIIdx];
	}

	// Initialize the player widgets that won't change
	InitializePlayerUI();

	// Initialize the rest of UI
	InitializeOtherUI();

	bEndOfRoundInitialized = true;
}

final function name GetPlayerNameStyle(PlayerReplicationInfo PRI)
{
	local GearPRI Player1PRI, Player2PRI;

	GetLocalPRIs(Player1PRI, Player2PRI);
	if (PRI == Player1PRI)
	{
		return 'cmb_Scoreboard_P1';
	}
	else if (PRI == Player2PRI)
	{
		return 'cmb_Scoreboard_P2';
	}
	else
	{
		return 'cmb_CogSM';
	}
}

/** Initialize the player widgets that won't change */
function InitializePlayerUI()
{
	local int Idx;
	local GearPRI PRI;

	for (Idx = 0; Idx < PlayerData.length; Idx++)
	{
		PRI = PlayerData[Idx].PRI;
		if ( PRI != None )
		{
			PlayerData[Idx].ParentPanel.SetWidgetStyleByName('Panel Background Style', PRI.bIsDead ? 'LocustPlayersNormal' : 'CogPlayersNormal');
			PlayerData[Idx].ParentPanel.SetVisibility(true);

			PlayerData[Idx].NameLabel.SetDataStoreBinding(PRI.PlayerName);
			PlayerData[Idx].NameLabel.SetVisibility(true);
			PlayerData[Idx].NameLabel.SetWidgetStyleByName('String Style', GetPlayerNameStyle(PRI));

			PlayerData[Idx].RankLabel.SetDataStoreBinding(GetPlayerSkillString(PRI.PlayerSkill));
			PlayerData[Idx].RankLabel.SetVisibility(true);

			PlayerData[Idx].StatusImage.SetVisibility(PRI.bIsDead);

			PlayerData[Idx].ScratchImage.SetVisibility(PRI.bIsDead);

			PlayerData[Idx].ScoreLabel.SetDataStoreBinding("0");
			PlayerData[Idx].ScoreLabel.SetVisibility(true);

			PlayerData[Idx].KillLabel.SetDataStoreBinding(" ");
			PlayerData[Idx].KillLabel.SetVisibility(true);

			PlayerData[Idx].ChatImage.SetWidgetStyleByName('Image Style', PRI.bIsDead ? 'LocustChat' : 'CogChat');
		}
		else
		{
			PlayerData[Idx].ParentPanel.SetWidgetStyleByName('Panel Background Style', (NumPlayersAlive == 0) ? 'LocustPlayersNormal' : 'PartyPlayersNormal');
			PlayerData[Idx].ParentPanel.SetVisibility(true);
		}
	}
}

/** Initialize the rest of UI */
function InitializeOtherUI()
{
	local string StringToDraw;
	local int WaveNumber;
	local UIImage OtherBGImage;

	WaveNumber = (GRI.ExtendedRestartCount * class'GearGameHorde_Base'.default.MaxWaves) + GRI.InvasionCurrentWaveIndex;

	// Widgets in the header
	if (NumPlayersAlive > 0 &&
		WaveNumber >= 50)
	{
		BGImage = UIImage(FindChild('imgBGWin', true));
		OtherBGImage = UIImage(FindChild('imgBG', true));
	}
	else
	{
		BGImage = UIImage(FindChild('imgBG', true));
		OtherBGImage = UIImage(FindChild('imgBGWin', true));
	}

	BGImage.SetVisibility(true);
	OtherBGImage.SetVisibility(false);
	WaveLabel = UILabel(BGImage.FindChild('lblWave', true));
	TimeLabel = UILabel(BGImage.FindChild('lblTime', true));
	ResultLabel = UILabel(FindChild('lblResult', true));
	ResultLabel.SetVisibility(true);

	// Draw the wave number string
	StringToDraw = WaveHeaderString @ WaveNumber;
	WaveLabel.SetDataStoreBinding(StringToDraw);
	TimeLabel.SetDataStoreBinding(FormatTime(GRI.RoundEndTime - GRI.NumSecondsUntilNextRound));
	// Result label
	if (NumPlayersAlive > 0)
	{
		if (WaveNumber >= 50)
		{
			ResultLabel.SetDataStoreBinding(HordeDefeated);
		}
		else
		{
			ResultLabel.SetDataStoreBinding(WaveComplete);
		}
	}
	else
	{
		ResultLabel.SetDataStoreBinding(WaveIncomplete);
	}

	TeamScore.DescLabel.Opacity = DefaultScoreOpacity;
	SurvivalBonus.DescLabel.SetDataStoreBinding(SurvivalBonusString @ "[" $ NumPlayersAlive $ "]" );
	SurvivalBonus.DescLabel.Opacity = DefaultScoreOpacity;
	DifficultyMultiplier.DescLabel.Opacity = DefaultScoreOpacity;
	TeamScoreAdjusted.DescLabel.Opacity = DefaultScoreOpacity;
	TotalTeamScore.ScoreLabel.Opacity = 1.0f;
	SplatImage.SetVisibility(NumPlayersAlive == 0);

	TeamScore.ScoreBGImage.SetWidgetStyleByName('Image Style', (NumPlayersAlive == 0) ? 'EOR_HordeRed' : 'EOR_HordeBlue');
	SurvivalBonus.ScoreBGImage.SetWidgetStyleByName('Image Style', (NumPlayersAlive == 0) ? 'EOR_HordeRed' : 'EOR_HordeBlue');
	DifficultyMultiplier.ScoreBGImage.SetWidgetStyleByName('Image Style', (NumPlayersAlive == 0) ? 'EOR_HordeRed' : 'EOR_HordeBlue');
	TeamScoreAdjusted.ScoreBGImage.SetWidgetStyleByName('Image Style', (NumPlayersAlive == 0) ? 'EOR_HordeRed' : 'EOR_HordeBlue');
	TotalTeamScore.ScoreBGImage.SetWidgetStyleByName('Image Style', (NumPlayersAlive == 0) ? 'EOR_HordeRed' : 'EOR_HordeBlue');
	TotalTeamScore.ScoreLabel.SetDataStoreBinding(string(int(GearTeamInfo(GRI.Teams[0]).TotalScore - GearTeamInfo(GRI.Teams[0]).Score)));
}

/** Called every frame so we can update the scene */
function UpdateWidgetData(float DeltaTime)
{
	// pausing the game pauses this scene's animation
	if (GRI != None && GRI.WorldInfo.Pauser != None)
	{
		return;
	}

	if (GRI != None &&
		bEndOfRoundInitialized &&
		PreviousGameStatus != GRI.GameStatus)
	{
		GameStatusHasReplicated();
		PreviousGameStatus = GRI.GameStatus;
	}

	if ( !bEndOfRoundInitialized )
	{
		if (GRI.GameStatus == GS_RoundOver || GRI.GameStatus == GS_EndMatch)
		{
			InitializeSceneValues();
			SetVisibility(true);
		}
		else
		{
			SetVisibility(false);
		}
	}
	else
	{
		RefreshEndOfRoundScene();
		UpdateChatIcons();
	}
	EORTime += DeltaTime;
}

/** Called every frame once we're initialized to we can update the widgets */
function RefreshEndOfRoundScene()
{
	local float CurrTime;

	CurrTime = EORTime;

	switch(EORState)
	{
		case eGEAREORSTATE_Initialize:
			if (bEndOfRoundInitialized)
			{
				TotalSlashTime = KillSlashTotalTime();
				SetEORState(eGEAREORSTATE_PlayerScore);
			}
			break;
		case eGEAREORSTATE_PlayerScore:
			RefreshPlayerScores();
			if (StateStartTime + TotalSlashTime < CurrTime)
			{
				SetEORState(eGEAREORSTATE_TeamScore);
			}
			break;
		case eGEAREORSTATE_TeamScore:
			RefreshTeamScore();
			if (StateStartTime + TotalTallyTime < CurrTime)
			{
				SetEORState(eGEAREORSTATE_Survival);
			}
			break;
		case eGEAREORSTATE_Survival:
			RefreshSurvivalBonus();
			if (StateStartTime + TotalTallyTime < CurrTime)
			{
				SetEORState(eGEAREORSTATE_Difficulty);
			}
			break;
		case eGEAREORSTATE_Difficulty:
			RefreshDifficultyMultiplier();
			if (StateStartTime + TotalTallyTime < CurrTime)
			{
				SetEORState(eGEAREORSTATE_WaveScore);
			}
			break;
		case eGEAREORSTATE_WaveScore:
			RefreshWaveScore();
			if (StateStartTime + TotalTallyTime < CurrTime)
			{
				SetEORState(eGEAREORSTATE_TeamTotal);
			}
			break;
		case eGEAREORSTATE_TeamTotal:
			RefreshTotalTeamScore();
			if (StateStartTime + TotalTeamScoreTime < CurrTime)
			{
				SetEORState(eGEAREORSTATE_AllDone);
			}
			break;
		case eGEAREORSTATE_AllDone:
			if (StateStartTime + TotalAllDoneWaitTime < CurrTime)
			{
				AllDoneWaitTimeComplete();

				// Close the scene if the round is beginning
				if ( !bClosingScene &&
					 GRI != None &&
					 GRI.GameStatus == GS_RoundInProgress )
				{
					CloseScene();
					bClosingScene = true;
				}
			}
			break;
	}
}

/** Calculate the total time it will take to draw all the slashes */
function float KillSlashTotalTime()
{
	local int Idx, HighestKillCount;
	local GearPRI PRI;

	for (Idx = 0; Idx < PlayerData.length; Idx++)
	{
		PRI = PlayerData[Idx].PRI;
		if (PRI != None)
		{
			if (HighestKillCount < PRI.GetGameScore1())
			{
				HighestKillCount = PRI.GetGameScore1();
			}
		}
	}

	return HighestKillCount * TotalPlayerSlashTime;
}

/** Refreshes the player scores based on how much time passed since we started showing the scores */
function RefreshPlayerScores()
{
	local float ElapsedTime, PercentToDraw;
	local int NumFivesToDraw, KillsToDraw, Idx, DrawIdx, ScoreToDraw, NumSlashesToDraw;
	local GearPRI PRI;
	local string StringToDraw;
	local bool bPlaySound;
	local AudioComponent AC;
	local GearPC OwnerPC;

	ElapsedTime = EORTime - StateStartTime;
	if ( TotalSlashTime > 0.0f )
	{
		PercentToDraw = FMin(ElapsedTime/TotalSlashTime, 1.0f);
	}
	else
	{
		PercentToDraw = 0.0f;
	}
	NumSlashesToDraw = ElapsedTime / TotalPlayerSlashTime;

	for (Idx = 0; Idx < PlayerData.length; Idx++)
	{
		PRI = PlayerData[Idx].PRI;
		if (PRI != None)
		{
			// Build the Kill string
			KillsToDraw = Min(PRI.GetGameScore1(), NumSlashesToDraw);
			NumFivesToDraw = KillsToDraw / 5;
			StringToDraw = "";
			for (DrawIdx = 0; DrawIdx < NumFivesToDraw; DrawIdx++)
			{
				StringToDraw $= "5";
			}
			if (KillsToDraw - (NumFivesToDraw * 5) > 0)
			{
				StringToDraw $= string(KillsToDraw - (NumFivesToDraw * 5));
			}
			PlayerData[Idx].KillLabel.SetDataStoreBinding(StringToDraw);

			// Build the score string
			ScoreToDraw = PRI.GetGameScore2() * PercentToDraw;
			PlayerData[Idx].ScoreLabel.SetDataStoreBinding(string(ScoreToDraw));

			// Let's play a sound
			if (KillsToDraw > PlayerData[Idx].LastNumKillSlashes)
			{
				bPlaySound = true;
			}

			PlayerData[Idx].LastNumKillSlashes = KillsToDraw;
		}
	}

	if (bPlaySound)
	{
		bOpacitiesHitPeak = true;
		OwnerPC = GearPC(GetPlayerOwner().Actor);
		AC = OwnerPC.CreateAudioComponent(KillSlashSound, false, true);
		if ( AC != None )
		{
			AC.bAllowSpatialization = false;
			AC.bAutoDestroy = true;
			AC.Play();
		}
	}
}

/** Returns the team score with the option to remove bonuses and multipliers */
function int GetTeamWaveScore( bool bRemoveDifficulty, bool bRemoveSurvival )
{
	local int ScoreResult;

	ScoreResult = GRI.Teams[0].Score;

	if (bRemoveDifficulty)
	{
		ScoreResult /= GetDifficultyMultiplier();
	}

	if (bRemoveSurvival)
	{
		ScoreResult -= GetSurvivalBonus();
	}

	return ScoreResult;
}

/** Returns the Survival Bonus earned this wave */
function int GetSurvivalBonus()
{
	return float(NumPlayersAlive) / NumPlayerInGame * GRI.InvasionCurrentWaveIndex *
		(class'GearGameHorde_Base'.default.PointsPerWave / class'GearGameHorde_Base'.default.ScoringBaselineWavePoints) *
		class'GearPRI'.default.PointsFor_DamagePct * class'GearGameHorde_Base'.default.SurvivalBonusPct;
}

/** Returns the Difficulty Multiplier earned this wave */
function int GetDifficultyMultiplier()
{
	 return GRI.EnemyDifficulty.default.HordeScoreMultiplier;
}

/** Returns the opacities for the strings and background for the scoring bonus portions of the scene */
function GetScoringOpacities(out float LabelOpacity, out float BGOpacity)
{
	local float ElapsedTime;
	local AudioComponent AC;
	local GearPC OwnerPC;

	ElapsedTime = EORTime - StateStartTime;

	// Ramp up the opacities
	if (ElapsedTime <= TallyFadeInTime)
	{
		LabelOpacity = DefaultScoreOpacity + ((1.0f - DefaultScoreOpacity) * (ElapsedTime / TallyFadeInTime));
		BGOpacity = ElapsedTime / TallyFadeInTime;
	}
	else
	{
		if (!bOpacitiesHitPeak)
		{
			bOpacitiesHitPeak = true;
			OwnerPC = GearPC(GetPlayerOwner().Actor);
			AC = OwnerPC.CreateAudioComponent(BonusSound, false, true);
			if ( AC != None )
			{
				AC.bAllowSpatialization = false;
				AC.bAutoDestroy = true;
				AC.Play();
			}
		}
		LabelOpacity = 1.0f - ((1.0f - EndingScoreOpacity) * ((ElapsedTime - TallyFadeInTime) / TallyFadeOutTime));
		BGOpacity = 1.0f - ((ElapsedTime - TallyFadeInTime) / TallyFadeOutTime);
	}
}

/** Refreshes the team score based on how much time passed since we started showing the score */
function RefreshTeamScore()
{
	local float LabelOpacity, BGOpacity;

	TeamScore.ScoreLabel.SetDataStoreBinding( string(GetTeamWaveScore(true, true)) );
	GetScoringOpacities(LabelOpacity, BGOpacity);
	TeamScore.DescLabel.Opacity = LabelOpacity;
	TeamScore.ScoreLabel.Opacity = LabelOpacity;
	TeamScore.ScoreBGImage.Opacity = BGOpacity;
}

/** Refreshes the survival bonus based on how much time passed since we started showing the score */
function RefreshSurvivalBonus()
{
	local float LabelOpacity, BGOpacity;

	SurvivalBonus.ScoreLabel.SetDataStoreBinding( string(GetSurvivalBonus()) );
	GetScoringOpacities(LabelOpacity, BGOpacity);
	SurvivalBonus.DescLabel.Opacity = LabelOpacity;
	SurvivalBonus.ScoreLabel.Opacity = LabelOpacity;
	SurvivalBonus.ScoreBGImage.Opacity = BGOpacity;
}

/** Refreshes the difficulty multiplier based on how much time passed since we started showing the score */
function RefreshDifficultyMultiplier()
{
	local float LabelOpacity, BGOpacity;

	DifficultyMultiplier.ScoreLabel.SetDataStoreBinding( Multiply $ string(GetDifficultyMultiplier()) );
	GetScoringOpacities(LabelOpacity, BGOpacity);
	DifficultyMultiplier.DescLabel.Opacity = LabelOpacity;
	DifficultyMultiplier.ScoreLabel.Opacity = LabelOpacity;
	DifficultyMultiplier.ScoreBGImage.Opacity = BGOpacity;
}

/** Refreshes the total wave score earned after adjustments based on how much time passed since we started showing the score */
function RefreshWaveScore()
{
	local float LabelOpacity, BGOpacity;

	TeamScoreAdjusted.ScoreLabel.SetDataStoreBinding( string(GetTeamWaveScore(false, false)) );
	GetScoringOpacities(LabelOpacity, BGOpacity);
	TeamScoreAdjusted.DescLabel.Opacity = LabelOpacity;
	TeamScoreAdjusted.ScoreLabel.Opacity = LabelOpacity;
	TeamScoreAdjusted.ScoreBGImage.Opacity = BGOpacity;
}

/** Refreshes the total team score earned across all waves based on how much time passed since we started showing the score */
function RefreshTotalTeamScore()
{
	local float ElapsedTime, PercentToDraw, BGOpacity;
	local int ScoreToDraw;
	local GearTeamInfo GearTeam;
	local AudioComponent AC;
	local GearPC OwnerPC;

	ElapsedTime = EORTime - StateStartTime;
	PercentToDraw = FMin(ElapsedTime/TotalTeamScoreTime, 1.0f);
	GearTeam = GearTeamInfo(GRI.Teams[0]);
	ScoreToDraw = GearTeam.TotalScore - GearTeam.Score + (GearTeam.Score * PercentToDraw);

	// Ramp up the opacities
	if (ElapsedTime <= TotalTeamScoreFadeInTime)
	{
		BGOpacity = ElapsedTime / TotalTeamScoreFadeInTime;
	}
	else
	{
		if (!bOpacitiesHitPeak)
		{
			bOpacitiesHitPeak = true;
			OwnerPC = GearPC(GetPlayerOwner().Actor);
			AC = OwnerPC.CreateAudioComponent(TotalSound, false, true);
			if ( AC != None )
			{
				OwnerPC.AttachComponent(AC);
				AC.bAllowSpatialization = false;
				AC.bAutoDestroy = true;
				AC.Play();
			}
		}
		BGOpacity = 1.0f - (0.25f * ((ElapsedTime - TotalTeamScoreFadeInTime) / TotalTeamScoreFadeOutTime));
	}

	TotalTeamScore.ScoreLabel.SetDataStoreBinding(string(ScoreToDraw));
	TotalTeamScore.ScoreBGImage.Opacity = BGOpacity;
}

/** Close the scene because we are all done */
function AllDoneWaitTimeComplete()
{
	local UIScene HordeOptionsScene;
	local GearGameHorde_Base Game;

	if ( !bHandledAllDone )
	{
		Game = GearGameHorde_Base(GRI.WorldInfo.Game);
		// If this is the end of the wave/match and everyone died the host will display an option menu while the clients wait
		if ( GRI.GameStatus >= GS_RoundOver &&
			 NumPlayersAlive == 0 )
		{
			if ( GRI.Role == ROLE_Authority )
			{
				if (Game != None && !Game.bUsingArbitration)
				{
					HordeOptionsScene = UIScene(DynamicLoadObject("UI_Scenes_MP.UI_Horde_Options", class'UIScene'));
					HordeOptionsScene.OpenScene(HordeOptionsScene);
				}
				else
				{
					Game.TellClientsToReturnToPartyHost();
				}
			}
			else
			{
				WaitForHostLabel.SetVisibility(true);
			}
		}
		else
		{
			// start the next wave immediately after the host's menu is done
			if (GRI.Role == ROLE_Authority)
			{
				CloseScene();
				bClosingScene = true;
				if (Game != None && Game.IsTimerActive(nameof(Game.StartNextWave)))
				{
					Game.ClearTimer(nameof(Game.StartNextWave));
					Game.StartNextWave();
				}
			}
			else
			{
				 if (NumPlayersAlive > 0)
				 {
					 CloseScene();
					 bClosingScene = true;
				 }
			}
		}

		bHandledAllDone = true;
	}
}

/** Called when the game status has replicated a new value */
final function GameStatusHasReplicated()
{
	local PlayerController PC;
	local GearPC GPC;
	local GearPRI LocalPRI;
	local int WaveNumber;

	// Wave is over
	if (GRI.GameStatus == GS_RoundOver ||
		GRI.GameStatus == GS_EndMatch)
	{
		// Go through all local players
		foreach GRI.LocalPlayerControllers(class'PlayerController', PC)
		{
			LocalPRI = GearPRI(PC.PlayerReplicationInfo);
			if (LocalPRI != None)
			{
				GPC = GearPC(PC);
				if (GPC != None && GPC.ProfileSettings != None)
				{
					if (NumPlayersAlive > 0)
					{
						// Calculate the wave index for the profile (it's 0 based so we subtract 1)
						WaveNumber = (GRI.InvasionCurrentWaveIndex - 1) + (GRI.ExtendedRestartCount * 10);
						GPC.ProfileSettings.MarkHordeWaveAsCompleted(WaveNumber, GPC);
					}
				}
			}
		}
	}
}

/** Hack to eat all input */
function bool ProcessInput( out InputEventParameters EventParms )
{
	return true;
}

/** Need to allow the game to access pause menu */
function bool ProcessInputKey( out InputEventParameters EventParms )
{
	local GearPC MyGearPC;

	// Must be able to pause the game
	MyGearPC = GetGearPlayerOwner(EventParms.PlayerIndex);
	if (EventParms.InputKeyName == 'XboxTypeS_Start')
	{
		if (MyGearPC != None &&
			EventParms.EventType == IE_Released)
		{
			MyGearPC.ConditionalPauseGame();
		}
	}

	return true;
}

/** Needed for nulling the HUD reference to this scene */
function OnSceneDeactivatedCallback( UIScene DeactivatedScene )
{
	local GearPC MyGearPC;

	MyGearPC = GetGearPlayerOwner(GetBestPlayerIndex());
	if (MyGearPC != none && MyGearPC.MyGearHud != none)
	{
		GearHUDMP_Base(MyGearPC.MyGearHud).EndRoundUISceneInstance = none;
	}
}

defaultproperties
{
	OnSceneDeactivated=OnSceneDeactivatedCallback
	OnGearUISceneTick=UpdateWidgetData
	OnRawInputAxis=ProcessInput
	OnRawInputKey=ProcessInputKey

	Begin Object Name=SceneEventComponent
		DisabledEventAliases.Add(CloseScene)
	End Object

	KillSlashSound=SoundCue'Interface_Audio.Interface.HordeEndOfRoundKillSlashCue'
	BonusSound=SoundCue'Interface_Audio.Interface.HordeEndOfRoundBonusCue'
	TotalSound=SoundCue'Interface_Audio.Interface.HordeEndOfRoundTotalCue'

	DefaultScoreOpacity=0.25f
	EndingScoreOpacity=0.75f
	TotalPlayerSlashTime=0.2f
	TotalAllDoneWaitTime=5.0f

	TallyFadeInTime=0.25f
	TallyFadeOutTime=1.0f
	TotalTallyTime=1.25f

	TotalTeamScoreFadeInTime=0.25f
	TotalTeamScoreFadeOutTime=1.0f
	TotalTeamScoreTime=1.25f
}
