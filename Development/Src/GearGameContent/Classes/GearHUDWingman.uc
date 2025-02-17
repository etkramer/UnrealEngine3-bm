/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearHUDWingman extends GearHUDWingman_Base;

defaultproperties
{
	ScoreBoardUISceneReference=GearUIScene_Scoreboard'UI_Scenes_MP.UI_Scoreboard_Wingman'
	EndRoundUISceneReference=GearUISceneMP_Base'UI_Scenes_MP.UI_EndOfRound'
	SpectatorUISceneReference=GearUIScene_Base'UI_Scenes_MP.UI_Game_Spectator'
	PauseMPUISceneReference=GearUIScenePause_MP'UI_Scenes_MP.UI_Pause_MP'

	BuddyLocatorHalo=Material'Gear_Annex_Effects.HUD.M_HUD_Annex_RadialGlow'
	BuddyIndicatorMaterialBG=Material'Gear_Annex_Effects.HUD.M_HUD_Annex_RadialBG'
	BuddyIndicatorMaterialCOG=Material'Gear_Annex_Effects.HUD.M_HUD_Annex_RadialCOG'
	BuddyIndicatorMaterialLOC=Material'Gear_Annex_Effects.HUD.M_HUD_Annex_RadialLOC'

	MyBuddyDiedSound=SoundCue'Music_Stingers.stinger_kryll02Cue'
}
