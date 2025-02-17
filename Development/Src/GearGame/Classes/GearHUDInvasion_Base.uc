/**
 * GearHUD_Base
 * Gear Heads Up Display
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearHUDInvasion_Base extends GearHUDMP_Base
	abstract
	config(Game);

/** Time when ai factory informed us of last killed spawn */
var float TimeOfLastKilledSpawn;

var float LastScore;

/** Localized strings for displaying the beginning of wave text */
var localized string BeginWaveString;
var deprecated localized string BeginFinalWaveString;
var localized string HUDWaveString;
var localized string ScoreString;
var localized string EnemiesString;

/** Stinger to play when a new round starts */
var SoundCue BeginRoundStinger;
/** Stinger to play when a round is beat */
var array<SoundCue> EndRoundSuccessStingers;
var SoundCue EndRoundStingerFailed;

/** HUD score BG */
var CanvasIcon ScoreBG;
/** HUD progress bar BG */
var CanvasIcon ProgressBarBG;
/** HUD progress bar */
var CanvasIcon ProgressBar;
/** Top padding between the BG and the elements within it */
var const int BGPaddingTop;
/** Left padding between the BG and the elements within it */
var const int BGPaddingLeft;
/** Right padding between the BG and the elements within it */
var const int BGPaddingRight;
/** Vertical pixel location of the Score in relation to the BG */
var const int ScorePositionV;
/** Vertical pixel location of the enemy count in relation to the BG */
var const int EnemyPositionV;

/** Draw Progress Bar Size that tries to approach the actual; provides smooth animation of progress bar */
var int DrawProgressBarSize;


/** Hard reference to the HordeOptions scene */
var GearUIScene_HordeOptions HordeOptionsScene;

/** Hard reference to the Locust get stronger screen */
var GearUIScene_HordeMod HordeModScene;
/** Instance of the Locust get stronger screen */
var GearUIScene_HordeMod HordeModSceneInstance;
/** Played when the horde mod scene is opened **/
var SoundCue HordeStrongerSound;

// Called by the super class so we can draw to the canvas.
function DrawHUD()
{
	Super.DrawHUD();

	// Set the GRI
	GetGRI();

	// Draw the scoreboard
	if (!DrawScoreboard() &&
		!HideHUDFromScenes() &&
		bShowHUD )
	{
		// If we didn't draw the scoreboard then draw the enemy count
		DrawEnemyCount();
	}
}

/** Returns the score of all players on the team combined */
final function float GetTeamScoreValue()
{
	local float CurrentScore, DrawScore;
	local GearPRI PRI;

	DrawScore = 0;
	PRI = GearPRI(PlayerOwner.PlayerReplicationInfo);

	if ( PRI != None && PRI.Team != None )
	{
		CurrentScore = PRI.Team.Score;
		if (Abs(LastScore - CurrentScore) > 2.f)
		{
			// interpolate the draw score
			DrawScore = FMin(LastScore + (CurrentScore - LastScore) * RenderDelta * 2.f, CurrentScore);
		}
		else
		{
			// otherwise just equalize
			DrawScore = CurrentScore;
		}
	}

	return DrawScore;
}

/** Sets the color and alpha values for drawing the number of enemies remaining */
final function SetNumEnemiesLeftColorAndAlpha(byte OverallAlpha)
{
	local float FadeInTime, FadeOutTime, Percent, SustainTime;
	local byte Alpha, WhiteColorValue, NormalAlpha, NormalWhiteColorValue;

	// Color and Alpha values
	NormalAlpha = 180;
	NormalWhiteColorValue = 200;

	// Timing values
	FadeInTime = 0.25f;
	FadeOutTime = 0.5f;
	SustainTime = 0.25f;

	// Make count brighten
	if ( TimeOfLastKilledSpawn+FadeInTime >= WorldInfo.TimeSeconds )
	{
		Percent = (TimeOfLastKilledSpawn+FadeInTime-WorldInfo.TimeSeconds) / FadeInTime;
		WhiteColorValue = NormalWhiteColorValue + (1.f-Percent)*(255-NormalWhiteColorValue);
		Alpha = NormalAlpha + (1.f-Percent)*(255-NormalAlpha);
	}
	// Make count sustain brightness
	else if ( TimeOfLastKilledSpawn+FadeInTime+SustainTime >= WorldInfo.TimeSeconds )
	{
		WhiteColorValue = 255;
		Alpha = 255;
	}
	// Make count dull back down to normal
	else if ( TimeOfLastKilledSpawn+FadeInTime+FadeOutTime+SustainTime > WorldInfo.TimeSeconds )
	{
		Percent = (TimeOfLastKilledSpawn+FadeInTime+FadeOutTime+SustainTime-WorldInfo.TimeSeconds) / FadeOutTime;
		WhiteColorValue = 255 - (1.f-Percent)*(255-NormalWhiteColorValue);
		Alpha = 255 - (1.f-Percent)*(255-NormalAlpha);
	}
	// Count is normal brightness
	else
	{
		WhiteColorValue = NormalWhiteColorValue;
		Alpha = NormalAlpha;
	}

	Canvas.SetDrawColor( WhiteColorValue, WhiteColorValue, WhiteColorValue, Alpha*(float(OverallAlpha)/255.0f) );
}

/** Draw the enemy count if the game is being played */
final function DrawEnemyCount()
{
	local float BackGroundWidth, DrawX, DrawY, TextSizeX, TextSizeY, BackgroundX, BackgroundY;
	local String StringToDraw, TempScoreString;
	local byte OverallAlpha;
	local float DrawScore;
	local LinearColor LC;
	local int ActualProgressBarSize;

	// We must have replicated data for the HUD
	if (GRI != None && GRI.GameStatus == GS_RoundInProgress)
	{
		// Calculate the opacity
		OverallAlpha = 255 * (1.0f - TaccomFadeOpacity);
		// Set the font we will be using
		Canvas.Font = class'Engine'.static.GetAdditionalFont(FONT_Euro20);

		// Get the draw score
		DrawScore = GetTeamScoreValue();

		// Determine the width of the Horde HUD based on a potentially big score
		TempScoreString = ScoreString @ int(DrawScore);
		Canvas.TextSize(TempScoreString, TextSizeX, TextSizeY);
		BackGroundWidth = (TextSizeX > ScoreBG.UL+10) ? TextSizeX : ScoreBG.UL+10;

		// Draw the scoring background
		DrawX = SafeZoneLeft;
		DrawY = SafeZoneTop;
		LC = MakeLinearColor(0.94f, 0.94f, 0.94f, OverallAlpha/255);
		Canvas.SetPos(DrawX, DrawY);
		Canvas.DrawTileStretched(ScoreBG.Texture, BackGroundWidth, ScoreBG.VL, ScoreBG.U, ScoreBG.V, ScoreBG.UL, ScoreBG.VL, LC );
		BackgroundX = DrawX;
		BackgroundY = DrawY - 4;

		// Set the HUD color white
		SetHUDDrawColor(eWARHUDCOLOR_WHITE, OverallAlpha);

		// Draw the Score string
		DrawX = BackgroundX + BGPaddingLeft;
		DrawY = BackgroundY + ScorePositionV;
		Canvas.SetPos(DrawX, DrawY);
		Canvas.DrawText(ScoreString);

		// Draw the progress background
		DrawX = BackgroundX + BackGroundWidth - BGPaddingRight - ProgressBarBG.UL;
		DrawY = BackgroundY + EnemyPositionV + 4;
		Canvas.DrawIcon(ProgressBarBG, DrawX, DrawY);

		// Draw the wave number string
		StringToDraw = HUDWaveString @ ((GRI.ExtendedRestartCount * class'GearGameHorde_Base'.default.MaxWaves) + GRI.InvasionCurrentWaveIndex);
		Canvas.TextSize(StringToDraw, TextSizeX, TextSizeY);
		DrawX = BackgroundX + BackGroundWidth - TextSizeX - BGPaddingRight;
		DrawY = BackgroundY + BGPaddingTop;
		Canvas.SetPos(DrawX, DrawY);
		Canvas.DrawText(StringToDraw);

		// Draw team score
		StringToDraw = String(int(DrawScore));
		Canvas.TextSize(StringToDraw, TextSizeX, TextSizeY);
		DrawX = BackgroundX + BackGroundWidth - TextSizeX - BGPaddingRight;
		DrawY = BackgroundY + ScorePositionV;
		Canvas.SetPos(DrawX, DrawY);
		Canvas.DrawText(StringToDraw);

		SetNumEnemiesLeftColorAndAlpha(OverallAlpha);

		// Draw the Enemy count string
		DrawX = BackgroundX + BGPaddingLeft;
		DrawY = BackgroundY + EnemyPositionV;
		Canvas.SetPos(DrawX, DrawY);
		Canvas.DrawText(EnemiesString);

		// Draw the number of enemies left since we are close to the end
		if ( GRI.EnemiesLeftThisRound < 255 )
		{
			SetHUDDrawColor(eWARHUDCOLOR_RED, OverallAlpha);
			StringToDraw = String(GRI.EnemiesLeftThisRound);
			Canvas.TextSize(StringToDraw, TextSizeX, TextSizeY);
			DrawX = BackgroundX + BackGroundWidth - BGPaddingRight - TextSizeX;
			DrawY = BackgroundY + EnemyPositionV;
			Canvas.SetPos(DrawX, DrawY);
			Canvas.DrawText(StringToDraw);
		}
		// Draw the progress bar
		else
		{
			ActualProgressBarSize = ProgressBar.UL * ByteToFloat(GRI.WavePointsAlivePct);

			// If the DrawProgressBarSize is invalid
			if (DrawProgressBarSize < 0)
			{
				// then set it to the actual size immediately
				DrawProgressBarSize = ActualProgressBarSize;
			}
			// If it's larger than desired size
			else if (DrawProgressBarSize > ActualProgressBarSize)
			{
				// keep decrementing it until it's the correct size for a smooth animation effect
				DrawProgressBarSize -= 1;
			}


			SetHUDDrawColor(eWARHUDCOLOR_WHITE, OverallAlpha);
			DrawX = BackgroundX + BackGroundWidth - BGPaddingRight - DrawProgressBarSize;
			DrawY = BackgroundY + EnemyPositionV + 4;
			Canvas.SetPos(DrawX, DrawY);
			Canvas.DrawTile(ProgressBar.Texture, DrawProgressBarSize, ProgressBar.VL, ProgressBar.U + (ProgressBar.UL - DrawProgressBarSize), ProgressBar.V, DrawProgressBarSize, ProgressBar.VL);
		}

		// Store the last score drawn
		LastScore = DrawScore;
	}
}

/** Called when an AI factory spawn has been killed */
final function EnemyCountChanged()
{
	TimeOfLastKilledSpawn = WorldInfo.TimeSeconds;
}

/** Start the begin round screen */
function SignalStartofRoundOrMatch(GearGRI CurGRI)
{
	local int i;
	local class<GearGameHorde_Base> HordeGame;
	local string EnemyPathName;
	local class<Pawn> EnemyClass;

	// Don't show if there's a network error
	if (WorldInfo.Game != None && WorldInfo.Game.bHasNetworkError)
	{
		return;
	}

	DrawProgressBarSize = -1;

	DrawStartRound();

	// prestream the textures for the enemies in the next wave
	HordeGame = class<GearGameHorde_Base>(GRI.GameClass);
	if (HordeGame != None)
	{
		for (i = 0; i < HordeGame.default.EnemyList.Length; i++)
		{
			if ( CurGRI.InvasionCurrentWaveIndex >= HordeGame.default.EnemyList[i].MinWave &&
				CurGRI.InvasionCurrentWaveIndex <= HordeGame.default.EnemyList[i].MaxWave )
			{
				EnemyPathName = "GearGame." $ class'SeqAct_AIFactory'.default.SpawnInfo[HordeGame.default.EnemyList[i].EnemyType].PawnClassName;
				EnemyClass = class<Pawn>(FindObject(EnemyPathName, class'Class'));
				if (EnemyClass == None)
				{
					EnemyPathName = "GearGameContent." $ class'SeqAct_AIFactory'.default.SpawnInfo[HordeGame.default.EnemyList[i].EnemyType].PawnClassName;
					EnemyClass = class<Pawn>(FindObject(EnemyPathName, class'Class'));
				}
				if (EnemyClass != None && EnemyClass.default.Mesh != None)
				{
					EnemyClass.default.Mesh.PrestreamTextures(120.0, true);
				}
			}
		}
	}
}

/** Return whether someone is alive or not */
function bool HasGameEndedInSuccess()
{
	local int PRIIdx;
	local GearPRI CurrPRI;

	// Find all of the alive players and sort them from highest to lowest scores
	for ( PRIIdx = 0; PRIIdx < WorldInfo.GRI.PRIArray.length; PRIIdx++ )
	{
		if ( WorldInfo.GRI.PRIArray[PRIIdx] != None )
		{
			CurrPRI = GearPRI(WorldInfo.GRI.PRIArray[PRIIdx]);
			if ( CurrPRI.GetTeamNum() == 0 )
			{
				if ( !CurrPRI.bIsDead )
				{
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

/** Overload to play a sound */
function SignalEndofRoundOrMatch(GearGRI CurGRI)
{
	local SoundCue SuccessStinger;

	// Don't show the who won on the server if there was a network error
	if (WorldInfo.Game != None && WorldInfo.Game.bHasNetworkError)
	{
		return;
	}

	if (HasGameEndedInSuccess())
	{
		if (CurGRI.InvasionCurrentWaveIndex < EndRoundSuccessStingers.length)
		{
			SuccessStinger = EndRoundSuccessStingers[CurGRI.InvasionCurrentWaveIndex];
		}
		else if (EndRoundSuccessStingers.length > 0)
		{
			SuccessStinger = EndRoundSuccessStingers[EndRoundSuccessStingers.length - 1];
		}
		if (SuccessStinger != None)
		{
			PlayerOwner.PlaySound(SuccessStinger, true);
		}
	}
	else
	{
		PlayerOwner.PlaySound(EndRoundStingerFailed, true);
	}

	Super.SignalEndofRoundOrMatch(CurGRI);
}

/** Start wave screen */
final function DrawStartRound()
{
	local String Title, Message;

	// don't display if replication of the wave number hasn't happened in time (may happen for join in progress)
	if (GRI.InvasionCurrentWaveIndex > 0)
	{
		Title = Localize( "GearUIScene_Base", "CT", "GearGame" );
		Message = BeginWaveString @ ((GRI.ExtendedRestartCount * class'GearGameHorde_Base'.default.MaxWaves) + GRI.InvasionCurrentWaveIndex);
		StartDrawingDramaticText( Title, Message, 8.0f, 2.0f, 2.0f, GetCountdownYPosition() + GetCountdownHeight() + GearPC(PlayerOwner).NumPixelsToMoveDownForDramaText(), 2.0f, 2.0f );
		// Play a stinger
		PlayerOwner.PlaySound(BeginRoundStinger, true);
	}
}

/**
 * Do NOT close the EndOfRound scene and do NOT null the EndRoundUISceneInstance
 * The scene is now nulled by the scene's deactivated delegate
 */
function CloseEndOfRoundScene()
{
}

/** Uses the game status to determine if the scoreboard should be showing or not */
function bool ShouldShowScoreboardBasedOnGameStatus()
{
	if ( GearGRI(WorldInfo.GRI).GameStatus != GS_RoundInProgress &&
		 GearGRI(WorldInfo.GRI).GameStatus != GS_RoundOver &&
		 GearGRI(WorldInfo.GRI).GameStatus != GS_EndMatch &&
		 GearGRI(WorldInfo.GRI).GameStatus != GS_Loading )
	{
		return true;
	}

	return false;
}

/** See if it's time to show the "Locust get stronger" screen */
function AttemptHordeModOpen()
{
	OpenHordeModScene();
}

/** Opens the horde mod scene and holds a reference to the scene instance */
function OpenHordeModScene()
{
	local GearPC MyGearPC;

	if (HordeModSceneInstance == none)
	{
		MyGearPC = GearPC(PlayerOwner);
		HordeModSceneInstance = GearUIScene_HordeMod(MyGearPC.ClientOpenScene(HordeModScene));
		if (HordeModSceneInstance != none)
		{
			PlaySound(HordeStrongerSound, true);
			SetTimer(8.0f, false, 'CloseHordeModScene');
		}
	}
}

/** Called when the horde mod scene is deactivated */
function CloseHordeModScene()
{
	local GearPC MyGearPC;

	if (HordeModSceneInstance != none)
	{
		MyGearPC = GearPC(PlayerOwner);
		MyGearPC.ClientCloseScene(HordeModSceneInstance);
		HordeModSceneInstance = none;
	}
}


defaultproperties
{
	TotalScoreTime=3.f
	TotalEndRoundTime=10.f
	EndRoundSceneDelayTime=0.5 // give some time for final team score to replicate

	ScoreBG=(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=257,V=385,UL=182,VL=58)
	ProgressBarBG=(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=370,V=494,UL=111,VL=15)
	ProgressBar=(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=257,V=494,UL=111,VL=15)
	BGPaddingTop=2
	BGPaddingLeft=4
	BGPaddingRight=3
	ScorePositionV=20
	EnemyPositionV=40
}

