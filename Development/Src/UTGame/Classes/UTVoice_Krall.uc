/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTVoice_Krall extends UTVoice
	abstract;

/** 
HonorableDeath
*/

static function bool SendLocationUpdate(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype, UTGame G, Pawn StatusPawn, optional bool bDontSendMidfield)
{
	return false;
}

defaultproperties
{
	LocationSpeechOffset=3

	TauntSounds(0)=SoundNodeWave'A_Character_KrallScythe.taunts.A_Taunt_scythe_Bleed'
	TauntSounds(1)=SoundNodeWave'A_Character_KrallScythe.taunts.A_Taunt_scythe_AngryRoar_alt'
	TauntSounds(2)=SoundNodeWave'A_Character_KrallScythe.taunts.A_Taunt_scythe_DieEnemy'
	TauntSounds(3)=SoundNodeWave'A_Character_KrallScythe.taunts.A_Taunt_scythe_ForTribe'
	TauntSounds(4)=SoundNodeWave'A_Character_KrallScythe.taunts.A_Taunt_scythe_FreshMeat'
	TauntSounds(5)=SoundNodeWave'A_Character_KrallScythe.taunts.A_Taunt_scythe_Hatred'
	TauntSounds(6)=SoundNodeWave'A_Character_KrallScythe.taunts.A_Taunt_scythe_HunterBecomesPrey'
	TauntSounds(7)=SoundNodeWave'A_Character_KrallScythe.taunts.A_Taunt_scythe_ItFearsUs'
	TauntSounds(8)=SoundNodeWave'A_Character_KrallScythe.taunts.A_Taunt_scythe_ItHasNoEscape'
	TauntSounds(9)=SoundNodeWave'A_Character_KrallScythe.taunts.A_Taunt_scythe_PraiseOurSkill'
	TauntSounds(10)=SoundNodeWave'A_Character_KrallScythe.taunts.A_Taunt_scythe_Punished'
	TauntSounds(11)=SoundNodeWave'A_Character_KrallScythe.taunts.A_Taunt_scythe_Shame'
	TauntSounds(12)=SoundNodeWave'A_Character_KrallScythe.taunts.A_Taunt_scythe_SmellBurningMeatMakeHungry'
	TauntSounds(13)=SoundNodeWave'A_Character_KrallScythe.taunts.A_Taunt_scythe_Useless'

	TauntAnimSoundMap(0)=(EmoteTag=TauntA,TauntSoundIndex=(1,7,12))//Bring It On
	TauntAnimSoundMap(1)=(EmoteTag=TauntB,TauntSoundIndex=(6,9))//Hoolahoop
	TauntAnimSoundMap(2)=(EmoteTag=TauntC,TauntSoundIndex=(5,11))//Hip Thrust
	TauntAnimSoundMap(3)=(EmoteTag=TauntD,TauntSoundIndex=(2,13))//Bullet To Head
	TauntAnimSoundMap(4)=(EmoteTag=TauntE,TauntSoundIndex=(4,8))//Come Here
	TauntAnimSoundMap(5)=(EmoteTag=TauntF,TauntSoundIndex=(0,10))//Throat Slit

	WeaponTauntSounds[0]=SoundNodeWave'A_Character_KrallScythe.taunts.A_Taunt_scythe_Slime'
	WeaponTauntSounds[2]=SoundNodeWave'A_Character_KrallScythe.taunts.A_Taunt_scythe_OneShot'
	WeaponTauntSounds[5]=SoundNodeWave'A_Character_KrallScythe.taunts.A_Taunt_scythe_HammerMessy'
	WeaponTauntSounds[7]=SoundNodeWave'A_Character_KrallScythe.taunts.A_Taunt_scythe_RocketsGoBoom'

	EncouragementSounds(0)=SoundNodeWave'A_Character_KrallScythe.taunts.A_Taunt_scythe_AngryRoar_alt'
	EncouragementSounds(1)=SoundNodeWave'A_Character_KrallGrobar.BotStatus.A_BotStatus_krall_Nice'

	ManDownSounds(0)=SoundNodeWave'A_Character_KrallScythe.taunts.A_Taunt_scythe_AngryRoar_alt'

	FlagKillSounds(0)=SoundNodeWave'A_Character_KrallGrobar.BotStatus.A_BotStatus_krall_EnemyFlagCarrierDown'

	OrbKillSounds(0)=SoundNodeWave'A_Character_KrallGrobar.BotStatus.A_BotStatus_krall_EnemyOrbCarrierDown'

	AckSounds(0)=SoundNodeWave'A_Character_KrallGrobar.BotStatus.A_BotStatus_krall_GotIt'

	FriendlyFireSounds(0)=SoundNodeWave'A_Character_KrallGrobar.BotStatus.A_BotStatus_krall_SameTeam'

	NeedOurFlagSounds(0)=SoundNodeWave'A_Character_CorruptEnigma.Mean_Taunts.A_Taunt_Corrupt_BeginSearchRoutine'

	GotYourBackSounds(0)=SoundNodeWave'A_Character_KrallGrobar.BotStatus.A_BotStatus_krall_GotYourBack'

	SniperSounds(0)=SoundNodeWave'A_Character_KrallGrobar.BotStatus.A_BotStatus_krall_Sniper'

	InPositionSounds(0)=SoundNodeWave'A_Character_KrallGrobar.BotStatus.A_BotStatus_krall_InPosition'

	IncomingSound=SoundNodeWave'A_Character_KrallGrobar.BotStatus.A_BotStatus_krall_Incoming'
	EnemyOrbCarrierSound=SoundNodeWave'A_Character_KrallGrobar.BotStatus.A_BotStatus_krall_EnemyOrbCarrier'
	EnemyFlagCarrierSound=SoundNodeWave'A_Character_KrallGrobar.BotStatus.A_BotStatus_krall_EnemyFlagCarrier'
	MidFieldSound=SoundNodeWave'A_Character_KrallGrobar.BotStatus.A_BotStatus_krall_Midfield'

	EnemyFlagCarrierHereSound=SoundNodeWave'A_Character_KrallGrobar.BotStatus.A_BotStatus_krall_EnemyFlagCarrierHere'
	EnemyFlagCarrierHighSound=SoundNodeWave'A_Character_KrallGrobar.BotStatus.A_BotStatus_krall_EnemyFlagCarrierGoingHigh'
	EnemyFlagCarrierLowSound=SoundNodeWave'A_Character_KrallGrobar.BotStatus.A_BotStatus_krall_EnemyFlagCarrierGoingLow'

	HaveOrbSounds(0)=SoundNodeWave'A_Character_KrallGrobar.BotStatus.A_BotStatus_krall_WeCarryOrb'

	HaveFlagSounds(0)=SoundNodeWave'A_Character_KrallGrobar.BotStatus.A_BotStatus_krall_WeCarryFlag'

	UnderAttackSounds(0)=SoundNodeWave'A_Character_KrallScythe.taunts.A_Taunt_scythe_AngryRoar_alt'

	AreaSecureSounds(0)=SoundNodeWave'A_Character_KrallScythe.taunts.A_Taunt_scythe_AngryRoar_alt'

	GotOurFlagSound=SoundNodeWave'A_Character_KrallGrobar.BotStatus.A_BotStatus_krall_WeSeeFlag'
}





