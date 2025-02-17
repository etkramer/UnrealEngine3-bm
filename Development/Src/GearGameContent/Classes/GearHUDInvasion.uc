/**
* GearHUD_Base
* Gear Heads Up Display
*
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GearHUDInvasion extends GearHUDInvasion_Base
	config(Game);

defaultproperties
{
	ScoreBoardUISceneReference=GearUIScene_Scoreboard'UI_Scenes_MP.UI_Scoreboard_Warzone'
	EndRoundUISceneReference=GearUISceneMP_Base'UI_Scenes_MP.UI_EOR_Horde'
	SpectatorUISceneReference=GearUIScene_Base'UI_Scenes_MP.UI_Game_Spectator'
	PauseMPUISceneReference=GearUIScenePause_MP'UI_Scenes_MP.UI_Pause_MP'
	HordeOptionsScene=GearUIScene_HordeOptions'UI_Scenes_MP.UI_Horde_Options'
	HordeModScene=GearUIScene_HordeMod'UI_Scenes_MP.UI_HordeMod'

	BeginRoundStinger=SoundCue'Music_Stingers.stinger_percussion04cue'
	EndRoundSuccessStingers[0]=SoundCue'Music_Stingers.Stingers.stinger_musiccog03cue'
	EndRoundSuccessStingers[1]=SoundCue'Music_Stingers.Stingers.stinger_musiccog03cue'
	EndRoundSuccessStingers[2]=SoundCue'Music_Stingers.Stingers.stinger_musiccog03cue'
	EndRoundSuccessStingers[3]=SoundCue'Music_Stingers.Stingers.stinger_musiccog02cue'
	EndRoundSuccessStingers[4]=SoundCue'Music_Stingers.Stingers.stinger_musiccog02cue'
	EndRoundSuccessStingers[5]=SoundCue'Music_Stingers.Stingers.stinger_musiccog02cue'
	EndRoundSuccessStingers[6]=SoundCue'Music_Stingers.Stingers.stinger_musiccog01cue'
	EndRoundStingerFailed=SoundCue'Music_Stingers.Stingers.stinger_thrill02Cue'
	HordeStrongerSound=SoundCue'Music_Stingers.stinger_kryll02Cue'
}
