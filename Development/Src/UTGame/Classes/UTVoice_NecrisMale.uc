/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTVoice_NecrisMale extends UTVoice
	abstract;

/** 

*/

defaultproperties
{
	LocationSpeechOffset=0

	TauntSounds(0)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_LetMeShowYouHell'
	TauntSounds(1)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_LifeIsWeakness'
	TauntSounds(2)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_LostIsYourSoul'
	TauntSounds(3)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_Morbid'
	TauntSounds(4)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_MyBloodIsBlack'
	TauntSounds(5)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_NiceCorpse'
	TauntSounds(6)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_PainWillPurifyYou'
	TauntSounds(7)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_WelcomeToMyPain'
	TauntSounds(8)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_YouBoreMe'
	TauntSounds(9)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_AWasteOfGoodSuffering'
	TauntSounds(10)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_Blood'
	TauntSounds(11)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_ColdBlooded'
	TauntSounds(12)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_DeathSetsYouFree'
	TauntSounds(13)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_EnforcedAssimilation'
	TauntSounds(14)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_IAmTheDeathBringer'
	TauntSounds(15)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_IFeelNoPain'
	TauntSounds(16)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_IGiveYouSalvation'
	TauntSounds(17)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_IHunger'
	TauntSounds(18)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_IllBeBackAgainAndAgain'
	TauntSounds(19)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_IShallBurnTheFleshFromYourSoul'
	TauntSounds(20)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_IShedTinyBlackTearsForYou'
	TauntSounds(21)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_IsThatYourBest'

	TauntAnimSoundMap(0)=(EmoteTag=TauntA,TauntSoundIndex=(5,8,11,18))//Bring It On
	TauntAnimSoundMap(1)=(EmoteTag=TauntB,TauntSoundIndex=(7,12,21))//Hoolahoop
	TauntAnimSoundMap(2)=(EmoteTag=TauntC,TauntSoundIndex=(13,17))//Hip Thrust
	TauntAnimSoundMap(3)=(EmoteTag=TauntD,TauntSoundIndex=(6,10,14))//Bullet To Head
	TauntAnimSoundMap(4)=(EmoteTag=TauntE,TauntSoundIndex=(0,1,2,16,19))//Come Here
	TauntAnimSoundMap(5)=(EmoteTag=TauntF,TauntSoundIndex=(3,4,9,15,20))//Throat Slit

	WeaponTauntSounds[2]=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_theassassinschoice'
	WeaponTauntSounds[8]=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_perforation'
	WeaponTauntSounds[9]=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_shardsofdestiny'

	EncouragementSounds(0)=SoundNodeWave'A_Character_NecrisMale01.botstatus.A_BotStatus_NecrisMale_Nice'
	EncouragementSounds(1)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_Impossible'

	ManDownSounds(0)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_KillMeAllYouWant'
	ManDownSounds(1)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_ThisIsNotTheEnd'
	ManDownSounds(2)=SoundNodeWave'A_Character_NecrisMale02.taunts.A_Taunt_Necrismale2_TryThatAgain'

	FlagKillSounds(0)=SoundNodeWave'A_Character_NecrisMale01.botstatus.A_BotStatus_NecrisMale_EnemyFlagCarrierDown'

	OrbKillSounds(0)=SoundNodeWave'A_Character_NecrisMale01.botstatus.A_BotStatus_NecrisMale_EnemyOrbCarrierDown'

	AckSounds(0)=SoundNodeWave'A_Character_NecrisMale01.botstatus.A_BotStatus_NecrisMale_Acknowledged'
	AckSounds(1)=SoundNodeWave'A_Character_NecrisMale01.botstatus.A_BotStatus_NecrisMale_Affirmative'

	FriendlyFireSounds(0)=SoundNodeWave'A_Character_NecrisMale01.botstatus.A_BotStatus_NecrisMale_SameTeam'

	NeedOurFlagSounds(0)=SoundNodeWave'A_Character_NecrisMale01.botstatus.A_BotStatus_NecrisMale_SomebodyGetOurFlagBack'

	GotYourBackSounds(0)=SoundNodeWave'A_Character_NecrisMale01.botstatus.A_BotStatus_NecrisMale_IveGotYourBack'

	SniperSounds(0)=SoundNodeWave'A_Character_NecrisMale01.botstatus.A_BotStatus_NecrisMale_Sniper'

	InPositionSounds(0)=SoundNodeWave'A_Character_NecrisMale01.botstatus.A_BotStatus_NecrisMale_InPosition'

	IncomingSound=SoundNodeWave'A_Character_NecrisMale01.botstatus.A_BotStatus_NecrisMale_Incoming'
	EnemyOrbCarrierSound=SoundNodeWave'A_Character_NecrisMale01.botstatus.A_BotStatus_NecrisMale_EnemyOrbCarrier'
	EnemyFlagCarrierSound=SoundNodeWave'A_Character_NecrisMale01.botstatus.A_BotStatus_NecrisMale_EnemyFlagCarrier'
	MidFieldSound=SoundNodeWave'A_Character_NecrisMale01.botstatus.A_BotStatus_NecrisMale_Midfield'

	EnemyFlagCarrierHereSound=SoundNodeWave'A_Character_NecrisMale01.botstatus.A_BotStatus_NecrisMale_EnemyFlagCarrierHere'
	EnemyFlagCarrierHighSound=SoundNodeWave'A_Character_NecrisMale01.botstatus.A_BotStatus_NecrisMale_EnemyFlagCarrierGoingHigh'
	EnemyFlagCarrierLowSound=SoundNodeWave'A_Character_NecrisMale01.botstatus.A_BotStatus_NecrisMale_EnemyFlagCarrierGoingLow'

	HaveOrbSounds(0)=SoundNodeWave'A_Character_NecrisMale01.botstatus.A_BotStatus_NecrisMale_IHaveTheOrb'

	HaveFlagSounds(0)=SoundNodeWave'A_Character_NecrisMale01.botstatus.A_BotStatus_NecrisMale_IHaveTheFlag'

	UnderAttackSounds(0)=SoundNodeWave'A_Character_NecrisMale01.botstatus.A_BotStatus_NecrisMale_UnderHeavyAttack'

	AreaSecureSounds(0)=SoundNodeWave'A_Character_NecrisMale01.botstatus.A_BotStatus_NecrisMale_AreaSecure'

	GotOurFlagSound=SoundNodeWave'A_Character_NecrisMale01.botstatus.A_BotStatus_NecrisMale_FlagInPossession'
}





