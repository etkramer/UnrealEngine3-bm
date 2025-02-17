/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTCTFBlueFlagBase extends UTCTFBase_Content
	placeable;

defaultproperties
{
	FlagType=class'UTGameContent.UTCTFBlueFlag'
    DefenderTeamIndex=1

	Begin Object Class=ParticleSystemComponent Name=EmptyParticles
		Template=ParticleSystem'Pickups.flag.effects.P_Flagbase_Empty_Idle_Blue'
		bAutoActivate=false
		SecondsBeforeInactive=1.0f
	End Object
	FlagEmptyParticles=EmptyParticles
	Components.Add(EmptyParticles)

	FlagBaseMaterial=MaterialInstanceConstant'Pickups.Base_Flag.Materials.M_Pickups_Base_Flag_Blue'

	bHasLocationSpeech=true
	LocationSpeech(0)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_InTheBlueBase'
	LocationSpeech(1)=SoundNodeWave'A_Character_Jester.BotStatus.A_BotStatus_Jester_InTheBlueBase'
	LocationSpeech(2)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_InTheBlueBase'
	NearLocationSpeech(0)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_NearTheBlueBase'
	NearLocationSpeech(1)=SoundNodeWave'A_Character_Jester.BotStatus.A_BotStatus_Jester_NearTheBlueBase'
	NearLocationSpeech(2)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_NearTheBlueBase'
	MidfieldHighSpeech(0)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_MidfieldHigh'
	MidfieldHighSpeech(1)=SoundNodeWave'A_Character_Jester.BotStatus.A_BotStatus_Jester_MidfieldHigh'
	MidfieldHighSpeech(2)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_MidfieldHigh'
	MidfieldLowSpeech(0)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_MidfieldLow'
	MidfieldLowSpeech(1)=SoundNodeWave'A_Character_Jester.BotStatus.A_BotStatus_Jester_MidfieldLow'
	MidfieldLowSpeech(2)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_MidfieldLow'
}





