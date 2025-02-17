/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearHUDCTM extends GearHUDCTM_Base;

/** Whether the pawn is the meatflag */
function bool IsMeatflagPawn( GearPawn AGearPawn )
{
	if ( ClassIsChildOf(AGearPawn.Class, class'GearPawn_MeatflagBase') )
	{
		return TRUE;
	}

	return FALSE;
}

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
	CPIndicatorGlow=Material'Gear_Annex_Effects.HUD.M_HUD_Annex_RadialGlow'
	CPIndicatorWeapons=Material'Warfare_HUD.HUD.M_Gears_AnnexIcons'
	VictimLocatorHalo=Material'Gear_Annex_Effects.HUD.M_HUD_Annex_RadialGlow'
	MeatflagDownIcon=(Texture=Texture2D'Warfare_HUD.HUD_down',U=0,V=0,UL=32,VL=32)

	AnnexSound_TeamAboutToWin(0)=SoundCue'DLC_Annex.Annex.TeamAbouttoWinCogCue'
	AnnexSound_TeamAboutToWin(1)=SoundCue'DLC_Annex.Annex.TeamAbouttoWinLocCue'
}
