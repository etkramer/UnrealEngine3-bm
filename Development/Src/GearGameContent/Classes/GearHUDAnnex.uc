/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearHUDAnnex extends GearHUDAnnex_Base;

/** Defaults properties */
defaultproperties
{
	ScoreBoardUISceneReference=GearUIScene_Scoreboard'UI_Scenes_MP.UI_Scoreboard_Warzone'
	EndRoundUISceneReference=GearUISceneMP_Base'UI_Scenes_MP.UI_EndOfRound'
	SpectatorUISceneReference=GearUIScene_Base'UI_Scenes_MP.UI_Game_Spectator'
	PauseMPUISceneReference=GearUIScenePause_MP'UI_Scenes_MP.UI_Pause_MP'

	MainScoreMaterialCOG=Material'Gear_Annex_Effects.HUD.M_HUD_Annex_MainScore_COG'
	MainScoreMaterialLOC=Material'Gear_Annex_Effects.HUD.M_HUD_Annex_MainScore_LOC'
	ScoreInvertMaterial=Material'Gear_Annex_Effects.HUD.M_HUD_Annex_MainScore_Inverted'
	CPIndicatorMaterialBG=Material'Gear_Annex_Effects.HUD.M_HUD_Annex_RadialBG'
	CPIndicatorMaterialCOG=Material'Gear_Annex_Effects.HUD.M_HUD_Annex_RadialCOG'
	CPIndicatorMaterialLOC=Material'Gear_Annex_Effects.HUD.M_HUD_Annex_RadialLOC'
	CPIndicatorWeapons=Material'Warfare_HUD.HUD.M_Gears_AnnexIcons'
	CPIndicatorGlow=Material'Gear_Annex_Effects.HUD.M_HUD_Annex_RadialGlow'

	AnnexSound_TickFTW=SoundCue'Interface_Audio.Interface.TaccomOpen01Cue'
	AnnexSound_TeamAboutToWin(0)=SoundCue'DLC_Annex.Annex.TeamAbouttoWinCogCue'
	AnnexSound_TeamAboutToWin(1)=SoundCue'DLC_Annex.Annex.TeamAbouttoWinLocCue'

	GameSpecificIcon(0)=(Texture=Texture2D'Warfare_HUD.ScoreBoard.UI_MP_Icons',U=0,V=72,UL=25,VL=26)
	GameSpecificIcon(1)=(Texture=Texture2D'Warfare_HUD.ScoreBoard.UI_MP_Icons',U=27,V=72,UL=25,VL=26)
}
