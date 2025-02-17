/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTVoice_DefaultFemale extends UTVoice
	abstract;

/**
UNUSED TAUNTS

WeveTakenTheLeadHoldThemOff
WhatsTakingSoLong
WhereAreYou
WhereTheHellAreYou
ProfessionalQuality_a-01

*/


static function int GetEncouragementMessageIndex(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype)
{
	local int R;
	
	if ( default.EncouragementSounds.Length == 0)
	{
		return -1;
	}
	R = Rand(default.EncouragementSounds.Length);
	if ( ( R == 1) || (R == 5) )
	{ 
		R -= 1;
	} 
	return ENCOURAGEMENTINDEXSTART + R;
}

defaultproperties
{
	LocationSpeechOffset=1

	TauntSounds(0)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_AndAnotherThankYouVeryMuch'
	TauntSounds(1)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_AndILookGoodDoingItToo'
	TauntSounds(2)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_AndStayDown'
	TauntSounds(3)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_BeatenByAGirl'
	TauntSounds(4)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_BetterLuckNextTime'
	TauntSounds(5)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_BoomBaby'
	TauntSounds(6)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_BooYa'
	TauntSounds(7)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_DamnImGood'
	TauntSounds(8)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_DeathWarrantEnforced'
	TauntSounds(9)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_Eliminated'
	TauntSounds(10)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_HoldStillDamnit'
	TauntSounds(11)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_IAmazeMyself'
	TauntSounds(12)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_IDontWantThatShitInMyHair'
	TauntSounds(13)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_IHateItWhenThatHappens'
	TauntSounds(14)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_ThatsOneMoreForTheGirl'
	TauntSounds(15)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_NextTimeTryNotToSuck'
	TauntSounds(16)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_OhYeah'
	TauntSounds(17)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_ThatsJustWrong'
	TauntSounds(18)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_Seeya'
	TauntSounds(19)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_StepAside'
	TauntSounds(20)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_WhoWantsSome'
	TauntSounds(21)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_YoullFeelTheImpactOfThatOne'
	TauntSounds(22)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_TopThat'
	TauntSounds(23)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_FreezeFrameMoment'
	TauntSounds(24)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_IsThatYourBest'
	TauntSounds(25)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_ItDoesntGetAnyBetterThanThat'
	TauntSounds(26)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_ThatsOneMoreForTheGirl'
	TauntSounds(27)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_IThinkIveMadeMyPoint'
	TauntSounds(28)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_OhSmack'
	TauntSounds(29)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_ThatsOneForTheCameras'
	TauntSounds(30)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_ThatsSoEmbarassing'

	TauntAnimSoundMap(0)=(EmoteTag=TauntA,TauntSoundIndex=(3,5,16,14,20))//Bring It On
	TauntAnimSoundMap(1)=(EmoteTag=TauntB,TauntSoundIndex=(0,6,11,19))//Hoolahoop
	TauntAnimSoundMap(2)=(EmoteTag=TauntC,TauntSoundIndex=(1,7,14,28,29,26))//Hip Thrust
	TauntAnimSoundMap(3)=(EmoteTag=TauntD,TauntSoundIndex=(4,8,18,21))//Bullet To Head
	TauntAnimSoundMap(4)=(EmoteTag=TauntE,TauntSoundIndex=(10,24))//Come Here
	TauntAnimSoundMap(5)=(EmoteTag=TauntF,TauntSoundIndex=(2,9,15))//Throat Slit

	WeaponTauntSounds[0]=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_Biohazard'
	WeaponTauntSounds[1]=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_GreenIsDefinitelyYourColor'
	WeaponTauntSounds[2]=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_OneShotOneKill'
	WeaponTauntSounds[3]=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_IJustSlaughteredThatGuy'
	WeaponTauntSounds[4]=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_Shocking'
	WeaponTauntSounds[5]=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_ThatWasAMess'
	WeaponTauntSounds[6]=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_FlakAttack'

	EncouragementSounds(0)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_GoodOne'
	EncouragementSounds(1)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_HatingYou'
	EncouragementSounds(2)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_NiceMove'
	EncouragementSounds(3)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_KeepItUp'
	EncouragementSounds(4)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_OhYeah'
	EncouragementSounds(5)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_ThatHadToHurt'
	EncouragementSounds(6)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_NoWay'
	EncouragementSounds(7)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_Damn'

	ManDownSounds(0)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_AnyoneOutThere'
	ManDownSounds(1)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_Bullshit'
	ManDownSounds(2)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_IllRememberThat'
	ManDownSounds(3)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_NotOneOfMyBetterMoments'
	ManDownSounds(4)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_WellThatJustSucked'
	ManDownSounds(5)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_ThatPlanFailed'
	ManDownSounds(6)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_WhyAmIAlwaysFirstToGetShot'

	FlagKillSounds(0)=SoundNodeWave'A_Character_Jester.Taunts.A_Taunt_Jester_NailedTheFlagCarrier'
	FlagKillSounds(1)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_EnemyFlagCarrierDown'

	OrbKillSounds(0)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_EnemyOrbCarrierDown'

	AckSounds(0)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_Acknowledged'
	AckSounds(1)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_Affirmative'
	AckSounds(2)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_GotIt'
	AckSounds(3)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_OnIt'
	AckSounds(4)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_Roger'
	AckSounds(5)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_RogerThat'
	AckSounds(6)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_ImOnIt'

	FriendlyFireSounds(0)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_YourTeam'
	FriendlyFireSounds(1)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_SameTeam'
	FriendlyFireSounds(2)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_ImOnYourTeam'
	FriendlyFireSounds(3)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_ImOnYourTeamIdiot'
	FriendlyFireSounds(4)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_FriendlyFire'

	NeedOurFlagSounds(0)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_SomebodyGetOurFlagBack'

	GotYourBackSounds(0)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_IveGotYourBack'
	GotYourBackSounds(1)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_GotYourBack'
	GotYourBackSounds(2)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_CoveringYou'

	SniperSounds(0)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_Sniper'
	SniperSounds(1)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_SuppressTheSniper'

	InPositionSounds(0)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_ImInPosition'
	InPositionSounds(1)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_InPosition'

	IncomingSound=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_Incoming'
	EnemyOrbCarrierSound=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_EnemyOrbCarrier'
	EnemyFlagCarrierSound=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_EnemyFlagCarrier'
	MidFieldSound=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_Midfield'

	EnemyFlagCarrierHereSound=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_EnemyFlagCarrier'
	EnemyFlagCarrierHighSound=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_EnemyFlagCarrierGoingHigh'
	EnemyFlagCarrierLowSound=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_EnemyFlagCarrierGoingLow'

	HaveOrbSounds(0)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_IHaveTheOrb'
	HaveOrbSounds(1)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_IveGotTheOrb'

	HaveFlagSounds(0)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_IHaveTheflag'
	HaveFlagSounds(1)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_IveGotTheFlag'

	UnderAttackSounds(0)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_UnderHeavyAttack'
	UnderAttackSounds(1)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_ImBeingOverrun'
	UnderAttackSounds(2)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_ImTakingHeavyFire'
	UnderAttackSounds(3)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_INeedBackup'
	UnderAttackSounds(4)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_INeedSomeBackup'

	AreaSecureSounds(0)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_AllClear'
	AreaSecureSounds(1)=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_AreaSecure'

	GotOurFlagSound=SoundNodeWave'A_Character_jester.BotStatus.A_BotStatus_jester_IveGotOurFlag'
}





