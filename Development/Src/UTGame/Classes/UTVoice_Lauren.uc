/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTVoice_Lauren extends UTVoice
	abstract;


defaultproperties
{
	LocationSpeechOffset=1

	TauntSounds(0)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_AndStayDown'
	TauntSounds(1)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_ANewLow'
	TauntSounds(2)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_AreYouEvenTrying'
	TauntSounds(3)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_AreYouKiddingMe'
	TauntSounds(4)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_BottomFeeder'
	TauntSounds(5)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_DuckFasterNextTime'
	TauntSounds(6)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_HellYeah'
	TauntSounds(7)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_HoldStillDamnit'
	TauntSounds(8)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_ImSorryDidIBreakYourConcentration'
	TauntSounds(9)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_IronGuard_a'
	TauntSounds(10)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_JustHoldStillAndIllMakeItQuick'
	TauntSounds(11)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_Next'
	TauntSounds(12)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_OhSmack'
	TauntSounds(13)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_OhSmack_alt'
	TauntSounds(14)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_OhYeah'
	TauntSounds(15)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_Sick'
	TauntSounds(16)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_StillNotAsGoodAsMe'
	TauntSounds(17)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_TargetEliminated'
	TauntSounds(18)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_TryThatAgain'
	TauntSounds(19)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_TryTurningTheSafetyOff'
	TauntSounds(20)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_WhatAMess'
	TauntSounds(21)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_YouBleedBetterThanYouShoot'
	TauntSounds(22)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_YouCantHandleARealWoman'
	TauntSounds(23)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_YouThinkYouHaveWhatItTakes'

	TauntAnimSoundMap(0)=(EmoteTag=TauntA,TauntSoundIndex=(2,6,9,16,22))//Bring It On
	TauntAnimSoundMap(1)=(EmoteTag=TauntB,TauntSoundIndex=(1,14,19))//Hoolahoop
	TauntAnimSoundMap(2)=(EmoteTag=TauntC,TauntSoundIndex=(4,12,13,23))//Hip Thrust
	TauntAnimSoundMap(3)=(EmoteTag=TauntD,TauntSoundIndex=(3,8,17))//Bullet To Head
	TauntAnimSoundMap(4)=(EmoteTag=TauntE,TauntSoundIndex=(7,10,11,18))//Come Here
	TauntAnimSoundMap(5)=(EmoteTag=TauntF,TauntSoundIndex=(0,5,15,21))//Throat Slit

	WeaponTauntSounds[0]=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_Biohazard'
	WeaponTauntSounds[1]=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_GreenIsDefinitelyYourColor'
	WeaponTauntSounds[2]=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_OneShotOneKill'
	WeaponTauntSounds[4]=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_Shocking'
	WeaponTauntSounds[5]=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_ThatWasMessy'
	WeaponTauntSounds[6]=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_FlakAttack'
	WeaponTauntSounds[9]=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_IThinkIveMadeMyPoint'
	WeaponTauntSounds[10]=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_DeathWarrantEnforced'

	EncouragementSounds(0)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_Impressive'
	EncouragementSounds(1)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_GoodJobKeepItUp'
	EncouragementSounds(2)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_HellYeah'
	EncouragementSounds(3)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_OhSmack_alt'
	EncouragementSounds(4)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_OhYeah'

	ManDownSounds(0)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_ICantFeelMyLegs'
	ManDownSounds(1)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_ImDown'
	ManDownSounds(2)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_IllRememberThat'
	ManDownSounds(3)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_ImHit'
	ManDownSounds(4)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_Medic'

	FlagKillSounds(0)=SoundNodeWave'A_Character_Lauren.Taunts.A_Taunt_Lauren_NailedTheFlagCarrier'
	FlagKillSounds(1)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_EnemyFlagCarrierDown'

	OrbKillSounds(0)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_EnemyOrbCarrierDown'

	AckSounds(0)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_Affirmative'
	AckSounds(1)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_GotIt'
	AckSounds(2)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_RogerThat'
	AckSounds(3)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_ImOnIt'

	FriendlyFireSounds(0)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_YourTeam'
	FriendlyFireSounds(1)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_SameTeam'
	FriendlyFireSounds(2)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_ImOnYourTeam'
	FriendlyFireSounds(3)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_ImOnYourTeamIdiot'

	NeedOurFlagSounds(0)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_SombodyGetOurFlagBack'

	GotYourBackSounds(0)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_IveGotYourBack'
	GotYourBackSounds(1)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_CoveringYou'

	SniperSounds(0)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_Sniper'

	InPositionSounds(0)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_ImInPosition'
	InPositionSounds(1)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_InPosition'

	IncomingSound=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_Incoming'
	EnemyOrbCarrierSound=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_EnemyOrbCarrier'
	EnemyFlagCarrierSound=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_EnemyFlagCarrier'
	MidFieldSound=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_Midfield'

	EnemyFlagCarrierHereSound=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_EnemyFlagCarrier'
	EnemyFlagCarrierHighSound=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_EnemyFlagCarrierGoingHigh'
	EnemyFlagCarrierLowSound=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_EnemyFlagCarrierGoingLow'

	HaveOrbSounds(0)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_IHaveTheOrb'
	HaveOrbSounds(1)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_IveGotTheOrb'

	HaveFlagSounds(0)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_IHaveTheflag'
	HaveFlagSounds(1)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_IveGotTheFlag'

	UnderAttackSounds(0)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_UnderHeavyAttack'
	UnderAttackSounds(1)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_ImBeingOverrun'
	UnderAttackSounds(2)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_ImTakingHeavyFire'
	UnderAttackSounds(3)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_INeedBackup'
	UnderAttackSounds(4)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_INeedSomeBackup'

	AreaSecureSounds(0)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_AllClear'
	AreaSecureSounds(1)=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_AreaSecure'

	GotOurFlagSound=SoundNodeWave'A_Character_Lauren.BotStatus.A_BotStatus_Lauren_IveGotOurFlag'
}





