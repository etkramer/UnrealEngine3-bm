/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTVoice_DefaultMale extends UTVoice
	abstract;

/* Unused Orders

RedeemerIncoming
------------------
Attacking
Defending
Freelance

Othello
Reaper
Jester
Bishop

Negative
PoundTheCore
Sorry

TakeTheirFlag
Turtle
Status
Jump
Attack
AttackTheCore
AttackThisNode
SearchAndDestroy
DefendTheBase
DefendTheFlag
DefendThisNode
Dominate
HoldThisPosition
FullOffense
CoverMe
Defend

SeekerIncoming

HeyThatWasMyKill
KillStealer

ISeeTheOrb
OrbSpotted
OrbCarrierDown

FlagCarrierDown
FlagCarrierSpotted

*/

/* Unused Taunts

Aahh (falling scream)

Tight

LuckyShot
WeveTakenTheLead
WhereAreYou
WhereTheHellAreYou
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
	else if ( R == 2 )
	{
		R = 6;
	}
	return ENCOURAGEMENTINDEXSTART + R;
}

defaultproperties
{
	LocationSpeechOffset=0

	TauntSounds(0)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_AndStayDown'
	TauntSounds(1)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_BadNews'
	TauntSounds(2)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_DamnImGood'
	TauntSounds(3)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_FYeah'
	TauntSounds(4)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_ImOnFire'
	TauntSounds(5)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_OhSmack'
	TauntSounds(6)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_ThatWasNasty'
	TauntSounds(7)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_WhoWantsSome'
	TauntSounds(8)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_DeathWarrantEnforced'
	TauntSounds(9)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_FeelImpact'
	TauntSounds(10)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_HoldStillDammit'
	TauntSounds(11)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_WelcomeToBasicTraining'
	TauntSounds(12)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_Impossible'
	TauntSounds(13)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_KIA'
	TauntSounds(14)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_MadeMyPoint'
	TauntSounds(15)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_Next'
	TauntSounds(16)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_OhYeah'
	TauntSounds(17)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_Ownage'
	TauntSounds(18)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_Sick'
	TauntSounds(19)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_StepAside'
	TauntSounds(20)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_TakeIt'
	TauntSounds(21)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_ThatHadToHurt'
	TauntSounds(22)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_TryThatAgain'

	TauntAnimSoundMap(0)=(EmoteTag=TauntA,TauntSoundIndex=(1,12,16,22))//Bring It On
	TauntAnimSoundMap(1)=(EmoteTag=TauntB,TauntSoundIndex=(4,5,17))//Hoolahoop
	TauntAnimSoundMap(2)=(EmoteTag=TauntC,TauntSoundIndex=(3,9,16,20))//Hip Thrust
	TauntAnimSoundMap(3)=(EmoteTag=TauntD,TauntSoundIndex=(13,8,11,18))//Bullet To Head
	TauntAnimSoundMap(4)=(EmoteTag=TauntE,TauntSoundIndex=(7,10,15,19))//Come Here
	TauntAnimSoundMap(5)=(EmoteTag=TauntF,TauntSoundIndex=(2,6,14,21))//Throat Slit

	WeaponTauntSounds[0]=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_Biohazard'
	WeaponTauntSounds[1]=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_GreenIsYourColor'
	WeaponTauntSounds[2]=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_OneShotOneKill'
	WeaponTauntSounds[3]=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_RightBetweenTheEyes'
	WeaponTauntSounds[4]=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_Shocking'
	WeaponTauntSounds[5]=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_ThatWasMessy'
	WeaponTauntSounds[6]=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_FlakAttack'

	EncouragementSounds(0)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_GoodOne'
	EncouragementSounds(1)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_GoodShot'
	EncouragementSounds(2)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_NiceShot'
	EncouragementSounds(3)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_KeepItUp'
	EncouragementSounds(4)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_OhYeah'
	EncouragementSounds(5)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_ThatHadToHurt'
	EncouragementSounds(6)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_Nice'
	EncouragementSounds(7)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_HolyShit'

	ManDownSounds(0)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_ICantFeelMyLegs'
	ManDownSounds(1)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_ImGoingDown'
	ManDownSounds(2)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_ImHit'
	ManDownSounds(3)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_ManDown'
	ManDownSounds(4)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_Medic'
	ManDownSounds(5)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_Sonofabitch'

	FlagKillSounds(0)=SoundNodeWave'A_Character_IGMale.Taunts.A_Taunt_IGMale_NailedTheFlagCarrier'
	FlagKillSounds(1)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_EnemyFlagCarrierDown'

	OrbKillSounds(0)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_EnemyOrbCarrierDown'

	AckSounds(0)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_Acknowledged'
	AckSounds(1)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_Affirmative'
	AckSounds(2)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_GotIt'
	AckSounds(3)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_OnIt'
	AckSounds(4)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_Roger'
	AckSounds(5)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_RogerThat'
	AckSounds(6)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_ImOnIt'

	FriendlyFireSounds(0)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_YourTeam'
	FriendlyFireSounds(1)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_SameTeam'
	FriendlyFireSounds(2)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_ImOnYourTeam'
	FriendlyFireSounds(3)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_ImOnYourTeamIdiot'
	FriendlyFireSounds(4)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_FriendlyFire'

	NeedOurFlagSounds(0)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_SomebodyGetOurFlagBack'

	GotYourBackSounds(0)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_IveGotYourBack'
	GotYourBackSounds(1)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_GotYourBack'
	GotYourBackSounds(2)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_CoveringYou'

	SniperSounds(0)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_Sniper'
	SniperSounds(1)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_SuppressTheSniper'

	InPositionSounds(0)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_ImInPosition'
	InPositionSounds(1)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_InPosition'

	IncomingSound=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_Incoming'
	EnemyOrbCarrierSound=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_EnemyOrbCarrier'
	EnemyFlagCarrierSound=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_EnemyFlagCarrier'
	MidFieldSound=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_Midfield'

	EnemyFlagCarrierHereSound=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_EnemyFlagCarrierHere'
	EnemyFlagCarrierHighSound=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_EnemyFlagCarrierGoingHigh'
	EnemyFlagCarrierLowSound=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_EnemyFlagCarrierGoingLow'

	HaveOrbSounds(0)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_IHaveTheOrb'
	HaveOrbSounds(1)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_IveGotTheOrb'

	HaveFlagSounds(0)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_IHaveTheflag'
	HaveFlagSounds(1)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_IveGotTheFlag'

	UnderAttackSounds(0)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_UnderHeavyAttack'
	UnderAttackSounds(1)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_ImBeingOverrun'
	UnderAttackSounds(2)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_ImTakingHeavyFire'
	UnderAttackSounds(3)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_INeedBackup'
	UnderAttackSounds(4)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_INeedSomeBackup'

	AreaSecureSounds(0)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_AllClear'
	AreaSecureSounds(1)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_AreaSecure'

	GotOurFlagSound=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_IveGotOurFlag'
}





