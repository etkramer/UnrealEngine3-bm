/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTVoice_Bishop extends UTVoice
	abstract;

defaultproperties
{
	LocationSpeechOffset=0

	TauntSounds(0)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_AbsolutionForTheSoulsOfTheWretched'
	TauntSounds(1)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_AMartyrForMyCause'
	TauntSounds(2)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_AndHeTakethAway'
	TauntSounds(3)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_YourDayOfJudgementIsAtHand'
	TauntSounds(4)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_ASacrificeForTheCause'
	TauntSounds(5)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_AtonementAndFireyWrath'
	TauntSounds(6)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_DepartAndEmbraceTheAfterlife'
	TauntSounds(7)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_HeGuidesMyHand'
	TauntSounds(8)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_DeathWarrantEnforced'
	TauntSounds(9)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_HisJusticeIsSwift'
	TauntSounds(10)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_IAmButAnInstrumentOfDeath'
	TauntSounds(11)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_ItWasHisWill'
	TauntSounds(12)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_LoThatIWalkInTheValleyOfShadows'
	TauntSounds(13)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_MadeInHisImageDestroyedInHisName'
	TauntSounds(14)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_MeetYourMaker'
	TauntSounds(15)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_SmoteDownInTheNameOfVengeance'
	TauntSounds(16)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_ThereIsNoMercyForYourKind'
	TauntSounds(17)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_VengeanceShallBeMine'

	TauntAnimSoundMap(0)=(EmoteTag=TauntA,TauntSoundIndex=(0,2,9,11,17))//Bring It On
	TauntAnimSoundMap(1)=(EmoteTag=TauntB,TauntSoundIndex=(5,13))//Hoolahoop
	TauntAnimSoundMap(2)=(EmoteTag=TauntC,TauntSoundIndex=(14))//Hip Thrust
	TauntAnimSoundMap(3)=(EmoteTag=TauntD,TauntSoundIndex=(4,7,12,15))//Bullet To Head
	TauntAnimSoundMap(4)=(EmoteTag=TauntE,TauntSoundIndex=(3,6,16))//Come Here
	TauntAnimSoundMap(5)=(EmoteTag=TauntF,TauntSoundIndex=(1,8,10))//Throat Slit

	WeaponTauntSounds[0]=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_Biohazard'
	WeaponTauntSounds[1]=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_GreenIsDefinitelyYourColor'
	WeaponTauntSounds[2]=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_OneShotOneKill'
	WeaponTauntSounds[3]=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_ThisIsMyRifleThisIsMyGun'
	WeaponTauntSounds[4]=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_Shocking'
	WeaponTauntSounds[5]=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_OohThatWasMessy'
	WeaponTauntSounds[7]=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_AndTheAngelOfDeathSpreadHisWingsUponTHeBlast'

	EncouragementSounds(0)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_Yes'
	EncouragementSounds(1)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_SweetRapture'
	EncouragementSounds(2)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_Indeed'
	EncouragementSounds(3)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_Nice'

	ManDownSounds(0)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_ToFallOnTheBattlefieldIsToRiseAbove'
	ManDownSounds(1)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_ABriefMomentOfSilence'
	ManDownSounds(2)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_INeverAskedForThis'
	ManDownSounds(3)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_ImHit'
	ManDownSounds(4)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_ManDown'
	ManDownSounds(5)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_Medic'

	FlagKillSounds(0)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_ISmoteTheeFlagCarrier'
	FlagKillSounds(1)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_EnemyFlagCarrierDown'

	OrbKillSounds(0)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_EnemyOrbCarrierDown'

	AckSounds(0)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_Acknowledged'
	AckSounds(1)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_Affirmative'
	AckSounds(2)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_AsYouRequest'
	AckSounds(3)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_YesSir'
	AckSounds(4)=SoundNodeWave'A_Character_Bishop.Taunts.A_Taunt_Bishop_ThyWillBeDone'
	AckSounds(5)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_ImOnIt'

	FriendlyFireSounds(0)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_YourTeam'
	FriendlyFireSounds(1)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_SameTeam'
	FriendlyFireSounds(2)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_ImOnYourTeam'
	FriendlyFireSounds(3)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_ImOnYourTeamIdiot'
	FriendlyFireSounds(4)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_FriendlyFire'

	NeedOurFlagSounds(0)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_SomebodyGetOurFlagBack'

	GotYourBackSounds(0)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_IveGotYourBack'
	GotYourBackSounds(1)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_GotYourBack'
	GotYourBackSounds(2)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_CoveringYou'

	SniperSounds(0)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_Sniper'
	SniperSounds(1)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_SuppressTheSniper'

	InPositionSounds(0)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_MovingIntoPosition'
	InPositionSounds(1)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_InPosition'

	IncomingSound=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_Incoming'
	EnemyOrbCarrierSound=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_EnemyOrbCarrier'
	EnemyFlagCarrierSound=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_EnemyFlagCarrier'
	MidFieldSound=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_Midfield'

	EnemyFlagCarrierHereSound=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_EnemyFlagCarrierHere'
	EnemyFlagCarrierHighSound=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_EnemyFlagCarrierGoingHigh'
	EnemyFlagCarrierLowSound=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_EnemyFlagCarrierGoingLow'

	HaveOrbSounds(0)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_IHaveTheOrb'
	HaveOrbSounds(1)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_IveGotTheOrb'

	HaveFlagSounds(0)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_IHaveTheflag'
	HaveFlagSounds(1)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_IveGotTheFlag'

	UnderAttackSounds(0)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_UnderHeavyAttack'
	UnderAttackSounds(1)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_ImBeingOverrun'
	UnderAttackSounds(2)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_ImTakingHeavyFire'
	UnderAttackSounds(3)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_INeedBackup'
	UnderAttackSounds(4)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_INeedSomeBackup'

	AreaSecureSounds(0)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_AllClear'
	AreaSecureSounds(1)=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_AreaSecure'

	GotOurFlagSound=SoundNodeWave'A_Character_Bishop.BotStatus.A_BotStatus_Bishop_IveGotOurFlag'
}





