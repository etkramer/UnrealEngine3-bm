/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTVoice_Akasha extends UTVoice
	abstract;

/**

*/

defaultproperties
{
	LocationSpeechOffset=1

	TauntSounds(0)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_AcceptYourDefeat'
	TauntSounds(1)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_ColdAsIce'
	TauntSounds(2)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_DarknessAwaits'
	TauntSounds(3)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_DeathGivesYouPurpose'
	TauntSounds(4)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_Denied'
	TauntSounds(5)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_EvenYouCannotStopMe'
	TauntSounds(6)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_YourSoulIsMine'
	TauntSounds(7)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_Gimblets'
	TauntSounds(8)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_GiveInToThePain'
	TauntSounds(9)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_HeHasForetoldOurVictory'
	TauntSounds(10)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_IAmForever'
	TauntSounds(11)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_IShallBurnTheFleshFromYourSoul'
	TauntSounds(12)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_IWillDrainYourLifeAway'
	TauntSounds(13)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_IWillPunishYou'
	TauntSounds(14)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_LetTheDarknessEmbraceYou'
	TauntSounds(15)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_MortalityIsYourWeakness'
	TauntSounds(16)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_MyVictoryIsInevitable'
	TauntSounds(17)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_NanoblackSustainMe'
	TauntSounds(18)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_NowYouKnowWhatColdFeelsLike'
	TauntSounds(19)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_OurVictoryIsNear'
	TauntSounds(20)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_PainPurifies'
	TauntSounds(21)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_WeShallCleanseThisPlanet'
	TauntSounds(22)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_YouCannotDefeatUs'

	TauntAnimSoundMap(0)=(EmoteTag=TauntA,TauntSoundIndex=(1,5,6,14,17))//Bring It On
	TauntAnimSoundMap(1)=(EmoteTag=TauntB,TauntSoundIndex=(7,10,16,22))//Hoolahoop
	TauntAnimSoundMap(2)=(EmoteTag=TauntC,TauntSoundIndex=(13,23,19,))//Hip Thrust
	TauntAnimSoundMap(3)=(EmoteTag=TauntD,TauntSoundIndex=(2,12,18,21))//Bullet To Head
	TauntAnimSoundMap(4)=(EmoteTag=TauntE,TauntSoundIndex=(0,8,9,11,15,23))//Come Here
	TauntAnimSoundMap(5)=(EmoteTag=TauntF,TauntSoundIndex=(3,4,13,20))//Throat Slit

	WeaponTauntSounds[2]=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_TheAssasinsChoice'
	WeaponTauntSounds[8]=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_Perforation'
	WeaponTauntSounds[9]=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_ShardsOfDestiny'

	EncouragementSounds(0)=SoundNodeWave'A_Character_NecrisFemale.Taunts.A_Taunt_NecrisFemale_Nice'

	ManDownSounds(0)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_SoCold'

	FlagKillSounds(0)=SoundNodeWave'A_Character_NecrisFemale.BotStatus.A_BotStatus_NecrisFemale_EnemyFlagCarrierDown'

	OrbKillSounds(0)=SoundNodeWave'A_Character_NecrisFemale.BotStatus.A_BotStatus_NecrisFemale_EnemyOrbCarrierDown'

	AckSounds(0)=SoundNodeWave'A_Character_NecrisFemale.BotStatus.A_BotStatus_NecrisFemale_Acknowledged_alt'

	FriendlyFireSounds(0)=SoundNodeWave'A_Character_NecrisFemale.BotStatus.A_BotStatus_NecrisFemale_SameTeam'

	NeedOurFlagSounds(0)=SoundNodeWave'A_Character_NecrisFemale.BotStatus.A_BotStatus_NecrisFemale_SomebodyGetOurFlagBack'

	GotYourBackSounds(0)=SoundNodeWave'A_Character_NecrisFemale.BotStatus.A_BotStatus_NecrisFemale_IveGotYourBack'

	SniperSounds(0)=SoundNodeWave'A_Character_NecrisFemale.BotStatus.A_BotStatus_NecrisFemale_Sniper'

	InPositionSounds(0)=SoundNodeWave'A_Character_NecrisFemale.BotStatus.A_BotStatus_NecrisFemale_InPosition'

	IncomingSound=SoundNodeWave'A_Character_NecrisFemale.BotStatus.A_BotStatus_NecrisFemale_Incoming'
	EnemyOrbCarrierSound=SoundNodeWave'A_Character_NecrisFemale.BotStatus.A_BotStatus_NecrisFemale_EnemyOrbCarrier'
	EnemyFlagCarrierSound=SoundNodeWave'A_Character_NecrisFemale.BotStatus.A_BotStatus_NecrisFemale_EnemyFlagCarrier'
	MidFieldSound=SoundNodeWave'A_Character_NecrisFemale.BotStatus.A_BotStatus_NecrisFemale_Midfield'

	EnemyFlagCarrierHereSound=SoundNodeWave'A_Character_NecrisFemale.BotStatus.A_BotStatus_NecrisFemale_EnemyFlagCarrier'
	EnemyFlagCarrierHighSound=SoundNodeWave'A_Character_NecrisFemale.BotStatus.A_BotStatus_NecrisFemale_EnemyFlagCarrierGoingHigh'
	EnemyFlagCarrierLowSound=SoundNodeWave'A_Character_NecrisFemale.BotStatus.A_BotStatus_NecrisFemale_EnemyFlagCarrierGoingLow'

	HaveOrbSounds(0)=SoundNodeWave'A_Character_NecrisFemale.BotStatus.A_BotStatus_NecrisFemale_IHaveTheOrb'

	HaveFlagSounds(0)=SoundNodeWave'A_Character_NecrisFemale.BotStatus.A_BotStatus_NecrisFemale_IHaveTheflag'

	UnderAttackSounds(0)=SoundNodeWave'A_Character_NecrisFemale.TauntsAkasha.A_Taunt_Akasha_NanoblackSustainMe'

	AreaSecureSounds(0)=SoundNodeWave'A_Character_NecrisFemale.BotStatus.A_BotStatus_NecrisFemale_AllClear'
	AreaSecureSounds(1)=SoundNodeWave'A_Character_NecrisFemale.BotStatus.A_BotStatus_NecrisFemale_AreaSecure'

	GotOurFlagSound=SoundNodeWave'A_Character_NecrisFemale.BotStatus.A_BotStatus_NecrisFemale_IveGotOurFlag'
}





