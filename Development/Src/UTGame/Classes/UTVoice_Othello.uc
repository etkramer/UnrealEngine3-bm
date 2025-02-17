/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTVoice_Othello extends UTVoice
	abstract;

/** 
Aahh

*/

defaultproperties
{
	LocationSpeechOffset=2

	TauntSounds(0)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_AndStayDown'
	TauntSounds(1)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_YeahThisIsGettingFunNow'
	TauntSounds(2)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_NowThatsWhatImTalkingBout'
	TauntSounds(3)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_ItsTheArmyOfMeBaby'
	TauntSounds(4)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_ImOnFire'
	TauntSounds(5)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_OhSmack'
	TauntSounds(6)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_ManICouldDoThisAllDay'
	TauntSounds(7)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_Respect'
	TauntSounds(8)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_DeathWarrantEnforced'
	TauntSounds(9)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_FeelImpact'
	TauntSounds(10)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_YeahBitch'
	TauntSounds(11)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_YouWantMoreBeatDown'
	TauntSounds(12)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_Impossible'
	TauntSounds(13)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_YouWantSomeOfMe'
	TauntSounds(14)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_MadeMyPoint'
	TauntSounds(15)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_ThatJustAintRight'
	TauntSounds(16)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_OhYeah'
	TauntSounds(17)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_StandUpAndFight'
	TauntSounds(18)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_TakeThatFools'
	TauntSounds(19)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_WhasupWitDatFool'
	TauntSounds(20)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_Mang'
	TauntSounds(21)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_Ouch'
	TauntSounds(22)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_Bam'
	TauntSounds(23)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_BeatDown'
	TauntSounds(24)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_ComeOnIAintJustDickinAroundHere'
	TauntSounds(25)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_DamnStraight'
	TauntSounds(26)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_DontYouLikeMyGrill'
	TauntSounds(27)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_DoTheDance'
	TauntSounds(28)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_FeelImpact'
	TauntSounds(29)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_FightThroughThePain'
	TauntSounds(30)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_FoShizzle'
	TauntSounds(31)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_HearIComeBabyClenchThemCheeks'
	TauntSounds(32)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_ImaBeatYoAss'
	TauntSounds(33)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_ImaGetYoAss'

	TauntAnimSoundMap(0)=(EmoteTag=TauntA,TauntSoundIndex=(1,4,6,15,25))//Bring It On
	TauntAnimSoundMap(1)=(EmoteTag=TauntB,TauntSoundIndex=(3,7,14,27))//Hoolahoop
	TauntAnimSoundMap(2)=(EmoteTag=TauntC,TauntSoundIndex=(2,5,16,29))//Hip Thrust
	TauntAnimSoundMap(3)=(EmoteTag=TauntD,TauntSoundIndex=(8,9,22,28))//Bullet To Head
	TauntAnimSoundMap(4)=(EmoteTag=TauntE,TauntSoundIndex=(10,11,13,17,32,33))//Come Here
	TauntAnimSoundMap(5)=(EmoteTag=TauntF,TauntSoundIndex=(0,12,18,29))//Throat Slit

	WeaponTauntSounds[0]=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_Biohazard'
	WeaponTauntSounds[1]=None
	WeaponTauntSounds[2]=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_OneShotOneKill'
	WeaponTauntSounds[3]=None
	WeaponTauntSounds[4]=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_Shocking'
	WeaponTauntSounds[5]=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_ILovesItMessy'
	WeaponTauntSounds[6]=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_FlakAttack'

	EncouragementSounds(0)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_Nice'
	EncouragementSounds(1)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_HolyShit'
	EncouragementSounds(2)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_NoWay'
	EncouragementSounds(3)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_KeepItUp'
	EncouragementSounds(4)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_OhYeah'
	EncouragementSounds(5)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_AwwYeah'
	EncouragementSounds(6)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_HappyDamn'
	EncouragementSounds(7)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_Booya'

	ManDownSounds(0)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_ICantFeelMyLegs'
	ManDownSounds(1)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_ManIJustGotHosed'
	ManDownSounds(2)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_ImHit'
	ManDownSounds(3)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_NowImMad'
	ManDownSounds(4)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_OhIFeelThatNow'
	ManDownSounds(5)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_ISwearIHadThatOne'
	ManDownSounds(6)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_UpsetDamn'

	FlagKillSounds(0)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_NailedTheFlagCarrier'
	FlagKillSounds(1)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_EnemyFlagCarrierDown'

	OrbKillSounds(0)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_EnemyOrbCarrierDown'

	AckSounds(0)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_Acknowledged'
	AckSounds(1)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_Affirmative'
	AckSounds(2)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_GotIt'
	AckSounds(3)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_OnIt'
	AckSounds(4)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_Roger'
	AckSounds(5)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_RogerThat'
	AckSounds(6)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_ImOnIt'

	FriendlyFireSounds(0)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_YourTeam'
	FriendlyFireSounds(1)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_SameTeam'
	FriendlyFireSounds(2)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_ImOnYourTeam'
	FriendlyFireSounds(3)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_ImOnYourTeamIdiot'
	FriendlyFireSounds(4)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_FriendlyFire'

	NeedOurFlagSounds(0)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_SomebodyGetOurFlagBack'

	GotYourBackSounds(0)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_IveGotYourBack'
	GotYourBackSounds(1)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_GotYourBack'
	GotYourBackSounds(2)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_CoveringYou'

	SniperSounds(0)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_Sniper'
	SniperSounds(1)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_SuppressTheSniper'

	InPositionSounds(0)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_ImInPosition'
	InPositionSounds(1)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_InPosition'

	IncomingSound=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_Incoming'
	EnemyOrbCarrierSound=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_EnemyOrbCarrier'
	EnemyFlagCarrierSound=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_EnemyFlagCarrier'
	MidFieldSound=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_Midfield'

	EnemyFlagCarrierHereSound=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_EnemyFlagCarrierHere'
	EnemyFlagCarrierHighSound=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_EnemyFlagCarrierGoingHigh'
	EnemyFlagCarrierLowSound=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_EnemyFlagCarrierGoingLow'

	HaveOrbSounds(0)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_IHaveTheOrb'
	HaveOrbSounds(1)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_IveGotTheOrb'

	HaveFlagSounds(0)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_IHaveTheflag'
	HaveFlagSounds(1)=SoundNodeWave'A_Character_Othello.Taunts.A_Taunt_Othello_WeHaveTheFlagLetsBringItHome'

	UnderAttackSounds(0)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_UnderHeavyAttack'
	UnderAttackSounds(1)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_ImBeingOverrun'
	UnderAttackSounds(2)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_ImTakingHeavyFire'
	UnderAttackSounds(3)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_INeedBackup'
	UnderAttackSounds(4)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_INeedSomeBackup'

	AreaSecureSounds(0)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_AllClear'
	AreaSecureSounds(1)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_AreaSecure'

	GotOurFlagSound=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_IveGotOurFlag'
}





