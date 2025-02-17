/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTVoice_Reaper extends UTVoice
	abstract;

defaultproperties
{
	LocationSpeechOffset=0

	TauntSounds(0)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_Damn'
	TauntSounds(1)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_Booya'
	TauntSounds(2)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_AndStayDown'
	TauntSounds(3)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_BringingItHome'
	TauntSounds(4)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_DoNotPushMe'
	TauntSounds(5)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_FeelImpact'
	TauntSounds(6)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_ForTheFallen'
	TauntSounds(7)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_GetOffMe'
	TauntSounds(8)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_GotEm'
	TauntSounds(9)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_KilledIt'
	TauntSounds(10)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_LineEmUpAndKnockThemDown'
	TauntSounds(11)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_NeverSawItComing'
	TauntSounds(12)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_OhSmack'
	TauntSounds(13)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_OutOfMyWay'
	TauntSounds(14)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_RookieMistake_alt'
	TauntSounds(15)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_ThatsGottaHurt'
	TauntSounds(16)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_TheresMoreWhereThatCameFrom'
	TauntSounds(17)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_YouveGotToBeKiddingMe'

	TauntAnimSoundMap(0)=(EmoteTag=TauntA,TauntSoundIndex=(0,10,14))//Bring It On
	TauntAnimSoundMap(1)=(EmoteTag=TauntB,TauntSoundIndex=(3,11,17))//Hoolahoop
	TauntAnimSoundMap(2)=(EmoteTag=TauntC,TauntSoundIndex=(4,5,12))//Hip Thrust
	TauntAnimSoundMap(3)=(EmoteTag=TauntD,TauntSoundIndex=(1,8,13))//Bullet To Head
	TauntAnimSoundMap(4)=(EmoteTag=TauntE,TauntSoundIndex=(6,16))//Come Here
	TauntAnimSoundMap(5)=(EmoteTag=TauntF,TauntSoundIndex=(2,7,9))//Throat Slit

	WeaponTauntSounds[0]=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_Biohazard'
	WeaponTauntSounds[1]=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_GreenIsYourColor'
	WeaponTauntSounds[2]=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_OneShotOneKill'
	WeaponTauntSounds[4]=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_Shocking'
	WeaponTauntSounds[5]=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_ThatWasMessy'
	WeaponTauntSounds[6]=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_FlakAttack'
	WeaponTauntSounds[9]=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_MadeMyPoint'
	WeaponTauntSounds[10]=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_DeathWarrant'
	WeaponTauntSounds[11]=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_PackageDelivered'

	EncouragementSounds(0)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_Damn'
	EncouragementSounds(1)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_KeepItUp'
	EncouragementSounds(2)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_Booya'

	ManDownSounds(0)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_OhThisSucks'
	ManDownSounds(1)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_WhatTheHell'
	ManDownSounds(2)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_NowImPissed'
	ManDownSounds(3)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_AnyoneOutThere'
	ManDownSounds(4)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_LosingBlood'

	FlagKillSounds(0)=SoundNodeWave'A_Character_Reaper.Taunts.A_Taunt_Reaper_NailedTheFlagCarrier'
/*	FlagKillSounds(1)=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_EnemyFlagCarrierDown'

	OrbKillSounds(0)=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_EnemyOrbCarrierDown'

	AckSounds(0)=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_Acknowledged'
	AckSounds(1)=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_Affirmative'
	AckSounds(2)=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_ImOnIt'

	FriendlyFireSounds(0)=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_ImOnYourTeam'
	FriendlyFireSounds(1)=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_SameTeam'

	NeedOurFlagSounds(0)=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_SomebodyGetOurFlagBack'

	GotYourBackSounds(0)=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_IveGotYourBack'

	SniperSounds(0)=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_Sniper'

	InPositionSounds(0)=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_InPosition'

	IncomingSound=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_Incoming'
	EnemyOrbCarrierSound=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_EnemyOrbCarrier'
	EnemyFlagCarrierSound=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_EnemyFlagCarrier'
	MidFieldSound=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_Midfield'

	EnemyFlagCarrierHereSound=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_EnemyFlagCarrierHere'
	EnemyFlagCarrierHighSound=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_EnemyFlagCarrierGoingHigh'
	EnemyFlagCarrierLowSound=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_EnemyFlagCarrierGoingLow'

	HaveOrbSounds(0)=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_IHaveTheOrb'

	HaveFlagSounds(0)=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_IHaveTheflag'

	UnderAttackSounds(0)=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_UnderHeavyAttack'
	UnderAttackSounds(1)=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_ImBeingOverrun'
	UnderAttackSounds(2)=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_ImTakingHeavyFire'

	AreaSecureSounds(0)=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_AllClear'
	AreaSecureSounds(1)=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_AreaSecure'

	GotOurFlagSound=SoundNodeWave'A_Character_Reaper.BotStatus.A_BotStatus_Reaper_IveGotOurFlag'
*/
}





