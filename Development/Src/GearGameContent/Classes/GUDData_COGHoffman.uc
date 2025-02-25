/**
 * Class: GUDData_COGHoffman
 * Gears Unscripted Dialog data for a single character.
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GUDData_COGHoffman extends GUDBank
	notplaceable
	abstract;

/** **** This file autogenerated by GUDS.xls.  Do not edit by hand. **** **/
defaultproperties
{
	GUDLines(0)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.OrdersResponse.HoffmanChatter_LostEm_Loud01Cue',bAlwaysLoad=1)
	GUDLines(1)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.OrdersResponse.HoffmanChatter_CantGetATarget_Loud03Cue')
	GUDActions(GUDAction_LostTarget_I)=(LineIndices=(0,1))

	GUDLines(2)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_ThatsOne_Loud01Cue')
	GUDLines(3)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_Next_Loud01Cue')
	GUDLines(4)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_GotOne_Loud01Cue')
	GUDLines(5)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_OneDeadGrub_Medium01Cue')
	GUDLines(6)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_ScratchOneGrub_Medium01Cue')
	GUDLines(7)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_BackInYourHole_Loud01Cue',Addressee=GUDRole_Recipient)
	GUDLines(8)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyKilled.HoffmanChatter_LooksDeadToMe_Medium02Cue',Addressee=GUDRole_Recipient)
	GUDLines(9)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyKilled.HoffmanChatter_Nice_Medium01Cue')
	GUDLines(10)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyKilled.HoffmanChatter_OhYeah_Medium04Cue',bAlwaysLoad=1)
	GUDLines(11)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyKilled.HoffmanChatter_OohYeah_Loud02Cue')
	GUDLines(12)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyKilled.HoffmanChatter_WhosNext_Medium01Cue')
	GUDLines(13)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyKilled.HoffmanChatter_TakeThat_Loud01Cue')
	GUDActions(GUDAction_KilledEnemyGeneric_I)=(CombatOnlyLineIndices=(2,3,4,5,6,7,8,9,10,11,12,13))

	GUDLines(14)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.WoundedEnemyCrawlsAway.HoffmanChatter_OoohThatsALottaOfBlood_Loud01Cue',bAlwaysLoad=1,Addressee=GUDRole_Instigator)
	GUDLines(15)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.WoundedEnemyCrawlsAway.HoffmanChatter_IDontThinkWeCanFixThatUp_Medium01Cue',Addressee=GUDRole_Instigator)
	GUDLines(16)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_ChainsawDeathAdLib_Loud05Cue',Addressee=GUDRole_Instigator)
	GUDLines(17)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_ManThatWasNasty_Loud02Cue',Addressee=GUDRole_Instigator)
	GUDActions(GUDAction_KilledEnemyChainsaw_TW)=(LineIndices=(14,15,16,17))

	GUDLines(18)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_GoodNight_Loud01Cue',Addressee=GUDRole_Recipient)
	GUDLines(19)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.WoundedEnemyCrawlsAway.HoffmanChatter_OoohThatsGottaHurt_Medium04Cue',Addressee=GUDRole_Recipient)
	GUDLines(20)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_LightsOut_Loud02Cue',Addressee=GUDRole_Recipient)
	GUDLines(21)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_CurbstompAdLib_Loud03Cue',Addressee=GUDRole_Recipient)
	GUDLines(22)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyKilled.HoffmanChatter_OhLookAtThatSuckerGo_Medium01Cue',Addressee=GUDRole_Recipient)
	GUDLines(23)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyKilled.HoffmanChatter_OhTheSpatterFactor_Medium01Cue',bAlwaysLoad=1,Addressee=GUDRole_Recipient)
	GUDLines(24)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_EatThat_Loud02Cue',Addressee=GUDRole_Recipient)
	GUDLines(25)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.ExecutingMeatShield.HoffmanChatter_NightyNight_Medium02Cue',Addressee=GUDRole_Recipient)
	GUDLines(26)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyKilled.HoffmanChatter_TakeThat_Loud03Cue',Addressee=GUDRole_Recipient)
	GUDLines(27)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.ExecutingMeatShield.HoffmanChatter_WereDoneHere_Medium01Cue',Addressee=GUDRole_Recipient)
	GUDLines(28)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyKilled.HoffmanChatter_LooksDeadEnoughToMe_Medium01Cue',Addressee=GUDRole_Recipient)
	GUDLines(29)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyKilled.HoffmanChatter_LooksDeadToMe_Medium02Cue',Addressee=GUDRole_Recipient)
	GUDActions(GUDAction_KilledEnemyHeadShot_I)=(LineIndices=(18,19,20,21,22,23,24,25,26,27,28,29))

	GUDLines(30)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyKilled.HoffmanChatter_Nice_Loud01Cue',bAlwaysLoad=1,Addressee=GUDRole_Instigator)
	GUDLines(31)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_LuckyShot_Loud02Cue',Addressee=GUDRole_Instigator)
	GUDLines(32)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_NiceWork_Medium02Cue',Addressee=GUDRole_Instigator)
	GUDActions(GUDAction_KilledEnemyHeadShot_TW)=(LineIndices=(30,31,32))

	GUDLines(33)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_ThatsGottaHurt_Loud01Cue',bAlwaysLoad=1,Addressee=GUDRole_Recipient)
	GUDLines(34)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_ThatSavedSomeAmmo_Loud01Cue',Addressee=GUDRole_Recipient)
	GUDLines(35)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_EatShitAndDie_Loud02Cue',Addressee=GUDRole_Recipient)
	GUDLines(36)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.ExecutingMeatShield.HoffmanChatter_AlrightStayInTouch_Medium01Cue',Addressee=GUDRole_Recipient)
	GUDLines(37)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_ShitTheyreTough_Loud01Cue',Addressee=GUDRole_Recipient)
	GUDLines(38)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyKilled.HoffmanChatter_WhosNext_Medium02Cue',Addressee=GUDRole_Recipient)
	GUDLines(39)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_AreYouKiddingMe_Loud01Cue',Addressee=GUDRole_Recipient)
	GUDLines(40)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.SomeoneAboutToFightYou.HoffmanChatter_TooClose_Medium01Cue',Addressee=GUDRole_Recipient)
	GUDActions(GUDAction_KilledEnemyMelee_I)=(LineIndices=(33,34,35,36,37,38,39,40))

	GUDLines(41)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_HowMuchCanTheyTake_Loud01Cue',bAlwaysLoad=1,Addressee=GUDRole_Instigator)
	GUDActions(GUDAction_KilledEnemyMelee_TW)=(LineIndices=(41))

	GUDLines(42)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_EatThat_Loud03Cue',bAlwaysLoad=1)
	GUDLines(43)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.WoundedEnemyCrawlsAway.HoffmanChatter_OoohThatsGottaHurt_Medium04Cue')
	GUDLines(44)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.WoundedEnemyCrawlsAway.HoffmanChatter_OoohThatsALottaOfBlood_Loud01Cue')
	GUDLines(45)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyKilled.HoffmanChatter_TakeThat_Loud02Cue')
	GUDActions(GUDAction_DamagedEnemyHeavy_I)=(LineIndices=(42,43,44,45))

	GUDLines(46)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.BattleStatus.HoffmanChatter_ImHit_Loud01Cue',bAlwaysLoad=1)
	GUDLines(47)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.TellingBattleStatus.HoffmanChatter_TakingFire_Loud01Cue')
	GUDLines(48)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.BattleStatus.HoffmanChatter_UnderFire_Loud05Cue')
	GUDActions(GUDAction_DamagedEnemyHeavy_R)=(LineIndices=(46,47,48))

	GUDLines(49)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_DownInFront_Medium01Cue',bAlwaysLoad=1,Addressee=GUDRole_Recipient)
	GUDLines(50)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_DownInFront_Loud01Cue',Addressee=GUDRole_Recipient)
	GUDLines(51)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_OutOfTheWay_Loud01Cue',Addressee=GUDRole_Recipient)
	GUDLines(52)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_YoureBlockingMyShots_Loud01Cue',Addressee=GUDRole_Recipient)
	GUDLines(53)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.GaveDamageToTeammate.HoffmanChatter_MyBad_Loud02Cue',Addressee=GUDRole_Recipient)
	GUDLines(54)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.GaveDamageToTeammate.HoffmanChatter_Sorry_Loud02Cue',Addressee=GUDRole_Recipient)
	GUDLines(55)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.TakingDamageFromTeammate.HoffmanChatter_FriendlyFire_Loud01Cue',Addressee=GUDRole_Recipient)
	GUDActions(GUDAction_DamageTeammate_I)=(LineIndices=(49,50,51,52,53,54,55))

	GUDLines(56)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_Hey_Loud01Cue',Addressee=GUDRole_Instigator)
	GUDLines(57)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_WatchIt_Medium01Cue',Addressee=GUDRole_Instigator)
	GUDLines(58)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_CheckYourAim_Loud01Cue',Addressee=GUDRole_Instigator)
	GUDLines(59)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_DontMakeMeShootYou_Medium02Cue',Addressee=GUDRole_Instigator)
	GUDLines(60)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_DoILookLocust_Loud01Cue',Addressee=GUDRole_Instigator)
	GUDLines(61)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.TakingDamageFromTeammate.HoffmanChatter_BroadSideOfABarn01Cue',Addressee=GUDRole_Instigator)
	GUDLines(62)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.TakingDamageFromTeammate.HoffmanChatter_CeaseAndDesist01Cue',Addressee=GUDRole_Instigator)
	GUDLines(63)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.TakingDamageFromTeammate.HoffmanChatter_CeaseFire_Loud01Cue',Addressee=GUDRole_Instigator)
	GUDLines(64)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.TakingDamageFromTeammate.HoffmanChatter_SameTeam_Medium02Cue',Addressee=GUDRole_Instigator)
	GUDLines(65)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.TakingDamageFromTeammate.HoffmanChatter_WhatAreYouShootinAt01Cue',bAlwaysLoad=1,Addressee=GUDRole_Instigator)
	GUDLines(66)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.TakingDamageFromTeammate.HoffmanChatter_WhatsTheMatterWithYou01Cue',Addressee=GUDRole_Instigator)
	GUDLines(67)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.TakingDamageFromTeammate.HoffmanChatter_WhatTheHellYouThinkin01Cue',Addressee=GUDRole_Instigator)
	GUDLines(68)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_TryAimingMrBlindfire_01Cue',Addressee=GUDRole_Instigator)
	GUDActions(GUDAction_DamageTeammate_R)=(LineIndices=(56,57,58,59,60,61,62,63,64,65,66,67,68))

	GUDLines(69)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.SomeoneAboutToFightYou.HoffmanChatter_BringIt_Medium01Cue',bAlwaysLoad=1)
	GUDLines(70)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.SomeoneAboutToFightYou.HoffmanChatter_YouWantSome_Medium04Cue')
	GUDLines(71)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.SomeoneAboutToFightYou.HoffmanChatter_ComeAndGetIt_Medium01Cue')
	GUDActions(GUDAction_TransToMeleeAI_EW)=(LineIndices=(69,70,71))

	GUDLines(72)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_FireInTheHole_Loud02Cue',bAlwaysLoad=1)
	GUDLines(73)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_FragOut_Loud01Cue')
	GUDLines(74)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_ThrowingFrag_Loud01Cue')
	GUDActions(GUDAction_ThrowingFragGrenade_I)=(LineIndices=(72,73,74))

	GUDLines(75)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_IncomingFrag_Loud01Cue',bAlwaysLoad=1)
	GUDActions(GUDAction_ThrowingFragGrenade_EW)=(LineIndices=(75))

	GUDLines(76)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_TheJobsNotOver_01Cue',bAlwaysLoad=1)
	GUDLines(77)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_WeCantStopNow_01Cue')
	GUDActions(GUDAction_PlayerHasntMoved_TW)=(LineIndices=(76,77))

	GUDLines(78)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_Medic_Loud02Cue')
	GUDLines(79)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_Medic_Loud01Cue')
	GUDLines(80)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.BattleStatus.HoffmanChatter_ImHit_Loud01Cue')
	GUDLines(81)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.BattleStatus.HoffmanChatter_ImHit_Medium01Cue')
	GUDLines(82)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.BattleStatus.HoffmanChatter_ImHit_Medium02Cue')
	GUDLines(83)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.KnockedDownButNotOut.HoffmanChatter_ReviveMe_Loud01Cue',bAlwaysLoad=1)
	GUDLines(84)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.KnockedDownButNotOut.HoffmanChatter_ReviveMe_Loud02Cue')
	GUDLines(85)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.KnockedDownButNotOut.HoffmanChatter_ReviveMe_Loud03Cue')
	GUDLines(86)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.KnockedDownButNotOut.HoffmanChatter_ReviveMe_Loud04Cue')
	GUDLines(87)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.KnockedDownButNotOut.HoffmanChatter_ReviveMe_Scream01Cue')
	GUDActions(GUDAction_HoffmanWentDown_I)=(LineIndices=(78,79,80,81,82,83,84,85,86,87))

	GUDLines(88)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_ComeGetMe_Loud02Cue',bAlwaysLoad=1)
	GUDLines(89)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_CouldUseSomeHelp_Loud02Cue')
	GUDLines(90)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_CouldUseAHand_Loud01Cue')
	GUDLines(91)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_SomebodyHelp_Loud02Cue')
	GUDLines(92)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_AnyoneOutThere_Loud01Cue')
	GUDLines(93)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_Medic_Loud02Cue')
	GUDLines(94)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_Medic_Loud01Cue')
	GUDLines(95)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.KnockedDownButNotOut.HoffmanChatter_ReviveMe_Loud01Cue')
	GUDLines(96)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.KnockedDownButNotOut.HoffmanChatter_ReviveMe_Loud02Cue')
	GUDLines(97)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.KnockedDownButNotOut.HoffmanChatter_ReviveMe_Loud03Cue')
	GUDLines(98)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.KnockedDownButNotOut.HoffmanChatter_ReviveMe_Loud04Cue')
	GUDLines(99)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.KnockedDownButNotOut.HoffmanChatter_ReviveMe_Scream01Cue')
	GUDActions(GUDAction_HoffmanNeedsRevived_I)=(LineIndices=(88,89,90,91,92,93,94,95,96,97,98,99))

	GUDLines(100)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_Go_Loud01Cue')
	GUDLines(101)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_KeepUpTheGoodWork_03Cue')
	GUDLines(102)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_WalkItOff_Medium02Cue')
	GUDLines(103)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_GetUp_Medium01Cue')
	GUDLines(104)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_GetUp_Loud01Cue')
	GUDLines(105)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_GoodToGo_Medium01Cue')
	GUDLines(106)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.AttachGrenadeToWall.HoffmanChatter_GoodToGo_Loud01Cue')
	GUDLines(107)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_WhatJustHappened_01Cue',bAlwaysLoad=1)
	GUDActions(GUDAction_Revived_I)=(LineIndices=(101,102,103,105,107),CombatOnlyLineIndices=(100,104,106))

	GUDLines(108)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_Thanks_Loud01Cue',bAlwaysLoad=1,Addressee=GUDRole_Instigator)
	GUDLines(109)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_NowImPissed_Loud01Cue')
	GUDLines(110)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_AlmostGotUgly_01Cue',Addressee=GUDRole_Instigator)
	GUDLines(111)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_DamnGrubsAreTough_Loud01Cue')
	GUDLines(112)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_LuckyShot_Loud01Cue')
	GUDLines(113)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_LockAndLoad_Loud01Cue')
	GUDLines(114)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.HoffmanSingle.HoffmanChatter_LetsDoThis_Loud01Cue')
	GUDActions(GUDAction_Revived_R)=(LineIndices=(108,109,110,111,112),CombatOnlyLineIndices=(113,114))

	GUDLines(115)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemiesSpotted.HoffmanChatter_Contact_Loud02Cue',bAlwaysLoad=1)
	GUDLines(116)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemiesSpotted.HoffmanChatter_EnemySpotted_Loud01Cue')
	GUDLines(117)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemiesSpotted.HoffmanChatter_Hostiles_Loud01Cue')
	GUDLines(118)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemiesSpotted.HoffmanChatter_Incoming_Loud01Cue')
	GUDLines(119)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemiesSpotted.HoffmanChatter_Incoming_Medium01Cue')
	GUDLines(120)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemiesSpotted.HoffmanChatter_Locust_Loud02Cue')
	GUDLines(121)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemiesSpotted.HoffmanChatter_Locust_Medium01Cue')
	GUDActions(GUDAction_NoticedEnemyGeneric_TW)=(LineIndices=(115,116,117,118,119,120,121))

	GUDLines(122)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.MoreLocustArriveToFight.HoffmanChatter_EnemyReinforcements_Loud01Cue',bAlwaysLoad=1)
	GUDLines(123)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.MoreLocustArriveToFight.HoffmanChatter_EnemyReinforcements_Loud02Cue')
	GUDLines(124)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.MoreLocustArriveToFight.HoffmanChatter_HereComesSomeMore_Loud03Cue')
	GUDLines(125)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.MoreLocustArriveToFight.HoffmanChatter_HereComesSomeMore_Loud04Cue')
	GUDLines(126)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.MoreLocustArriveToFight.HoffmanChatter_LocustReinforcements_Loud02Cue')
	GUDLines(127)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.MoreLocustArriveToFight.HoffmanChatter_WeveGotSomeMore_Loud01Cue')
	GUDLines(128)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.MoreLocustArriveToFight.HoffmanChatter_WeveGotSomeMore_Loud02Cue')
	GUDActions(GUDAction_NoticedReinforcements_TW)=(LineIndices=(122,123,124,125,126,127,128))

	GUDLines(129)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustUpHigh_Loud01Cue',bAlwaysLoad=1,ResponseEventIDs=(GUDEvent_TeammateAnnouncedEnemyDir),ReferringTo=GUDRole_ReferencedPawn)
	GUDLines(130)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustUpHigh_Loud02Cue',ResponseEventIDs=(GUDEvent_TeammateAnnouncedEnemyDir),ReferringTo=GUDRole_ReferencedPawn)
	GUDActions(GUDAction_EnemyLocationCallout_Above)=(LineIndices=(129,130))

	GUDLines(131)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustDownLow_Loud01Cue',bAlwaysLoad=1,ResponseEventIDs=(GUDEvent_TeammateAnnouncedEnemyDir),ReferringTo=GUDRole_ReferencedPawn)
	GUDLines(132)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustDownLow_Scream01Cue',ResponseEventIDs=(GUDEvent_TeammateAnnouncedEnemyDir),ReferringTo=GUDRole_ReferencedPawn)
	GUDActions(GUDAction_EnemyLocationCallout_Below)=(LineIndices=(131,132))

	GUDLines(133)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustToTheRight_Loud01Cue',bAlwaysLoad=1,ResponseEventIDs=(GUDEvent_TeammateAnnouncedEnemyDir),ReferringTo=GUDRole_ReferencedPawn)
	GUDLines(134)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustToTheRight_Loud02Cue',ResponseEventIDs=(GUDEvent_TeammateAnnouncedEnemyDir),ReferringTo=GUDRole_ReferencedPawn)
	GUDActions(GUDAction_EnemyLocationCallout_Right)=(LineIndices=(133,134))

	GUDLines(135)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustToTheLeft_Loud01Cue',bAlwaysLoad=1,ResponseEventIDs=(GUDEvent_TeammateAnnouncedEnemyDir),ReferringTo=GUDRole_ReferencedPawn)
	GUDLines(136)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustToTheLeft_Scream01Cue',ResponseEventIDs=(GUDEvent_TeammateAnnouncedEnemyDir),ReferringTo=GUDRole_ReferencedPawn)
	GUDActions(GUDAction_EnemyLocationCallout_Left)=(LineIndices=(135,136))

	GUDLines(137)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustDeadAhead_Loud01Cue',bAlwaysLoad=1,ResponseEventIDs=(GUDEvent_TeammateAnnouncedEnemyDir),ReferringTo=GUDRole_ReferencedPawn)
	GUDLines(138)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustDeadAhead_Loud02Cue',ResponseEventIDs=(GUDEvent_TeammateAnnouncedEnemyDir),ReferringTo=GUDRole_ReferencedPawn)
	GUDActions(GUDAction_EnemyLocationCallout_Ahead)=(LineIndices=(137,138))

	GUDLines(139)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustBehindUs_Loud01Cue',bAlwaysLoad=1,ResponseEventIDs=(GUDEvent_TeammateAnnouncedEnemyDir),ReferringTo=GUDRole_ReferencedPawn)
	GUDLines(140)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustBehindUs_Loud02Cue',ResponseEventIDs=(GUDEvent_TeammateAnnouncedEnemyDir),ReferringTo=GUDRole_ReferencedPawn)
	GUDActions(GUDAction_EnemyLocationCallout_Behind)=(LineIndices=(139,140))

	GUDLines(141)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustAtTheWindow_Loud01Cue',bAlwaysLoad=1,ReferringTo=GUDRole_ReferencedPawn)
	GUDLines(142)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustInTheWindow_Loud01Cue',ReferringTo=GUDRole_ReferencedPawn)
	GUDActions(GUDAction_EnemyLocationCallout_InWindow)=(LineIndices=(141,142))

	GUDLines(143)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustAtTheDoor_Loud01Cue',bAlwaysLoad=1,ReferringTo=GUDRole_ReferencedPawn)
	GUDLines(144)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustAtTheDoor_Scream01Cue',ReferringTo=GUDRole_ReferencedPawn)
	GUDActions(GUDAction_EnemyLocationCallout_InDoorway)=(LineIndices=(143,144))

	GUDLines(145)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustAtTheCar_Loud01Cue',bAlwaysLoad=1,ReferringTo=GUDRole_ReferencedPawn)
	GUDLines(146)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustBehindTheCar_Loud01Cue',ReferringTo=GUDRole_ReferencedPawn)
	GUDActions(GUDAction_EnemyLocationCallout_BehindCar)=(LineIndices=(145,146))

	GUDLines(147)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustAtTheTruck_Loud01Cue',bAlwaysLoad=1,ReferringTo=GUDRole_ReferencedPawn)
	GUDLines(148)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustBehindTheTruck_Loud01Cue',ReferringTo=GUDRole_ReferencedPawn)
	GUDActions(GUDAction_EnemyLocationCallout_BehindTruck)=(LineIndices=(147,148))

	GUDLines(149)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustOnTheTruck_Loud01Cue',bAlwaysLoad=1,ReferringTo=GUDRole_ReferencedPawn)
	GUDLines(150)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustOnTheTruck_Scream01Cue',ReferringTo=GUDRole_ReferencedPawn)
	GUDActions(GUDAction_EnemyLocationCallout_OnTruck)=(LineIndices=(149,150))

	GUDLines(151)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustAtTheBarriers_Loud01Cue',bAlwaysLoad=1,ReferringTo=GUDRole_ReferencedPawn)
	GUDLines(152)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustBehindTheBarriers_Loud01Cue',ReferringTo=GUDRole_ReferencedPawn)
	GUDActions(GUDAction_EnemyLocationCallout_BehindBarrier)=(LineIndices=(151,152))

	GUDLines(153)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustBehindTheColumns_Loud01Cue',bAlwaysLoad=1,ReferringTo=GUDRole_ReferencedPawn)
	GUDLines(154)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustBehindTheColumns_Scream01Cue',ReferringTo=GUDRole_ReferencedPawn)
	GUDActions(GUDAction_EnemyLocationCallout_BehindColumn)=(LineIndices=(153,154))

	GUDLines(155)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustBehindTheBox_Loud01Cue',bAlwaysLoad=1,ReferringTo=GUDRole_ReferencedPawn)
	GUDLines(156)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustBehindTheBoxes_Loud01Cue',ReferringTo=GUDRole_ReferencedPawn)
	GUDLines(157)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustBehindTheCrate_Loud01Cue',ReferringTo=GUDRole_ReferencedPawn)
	GUDLines(158)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustBehindTheCrates_Loud01Cue',ReferringTo=GUDRole_ReferencedPawn)
	GUDActions(GUDAction_EnemyLocationCallout_BehindCrate)=(LineIndices=(155,156,157,158))

	GUDLines(159)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustBehindTheWall_Loud01Cue',bAlwaysLoad=1,ReferringTo=GUDRole_ReferencedPawn)
	GUDLines(160)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustBehindTheWall_Scream01Cue',ReferringTo=GUDRole_ReferencedPawn)
	GUDActions(GUDAction_EnemyLocationCallout_BehindWall)=(LineIndices=(159,160))

	GUDLines(161)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustAtTheStatue_Loud01Cue',bAlwaysLoad=1,ReferringTo=GUDRole_ReferencedPawn)
	GUDLines(162)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustBehindTheStatue_Loud01Cue',ReferringTo=GUDRole_ReferencedPawn)
	GUDActions(GUDAction_EnemyLocationCallout_BehindStatue)=(LineIndices=(161,162))

	GUDLines(163)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustAtTheSandbags_Loud01Cue',bAlwaysLoad=1,ReferringTo=GUDRole_ReferencedPawn)
	GUDLines(164)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemyCallouts.HoffmanChatter_LocustBehindTheSandbags_Loud01Cue',ReferringTo=GUDRole_ReferencedPawn)
	GUDActions(GUDAction_EnemyLocationCallout_BehindSandbags)=(LineIndices=(163,164))

	GUDLines(165)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.FiringAMortar.HoffmanChatter_Incoming_Loud01Cue',bAlwaysLoad=1)
	GUDLines(166)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.FiringAMortar.HoffmanChatter_Incoming_Loud02Cue')
	GUDLines(167)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemiesSpotted.HoffmanChatter_Incoming_Scream01Cue')
	GUDLines(168)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.EnemiesSpotted.HoffmanChatter_Mortar_Scream01Cue')
	GUDActions(GUDAction_MortarLaunched_EW)=(LineIndices=(165,166,167,168))

	GUDLines(169)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.ThrowingGrenades.HoffmanChatter_IncomingInk_Loud01Cue',bAlwaysLoad=1)
	GUDLines(170)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.ThrowingGrenades.HoffmanChatter_IncomingInk_Loud02Cue')
	GUDActions(GUDAction_ThrowingInkGrenade_EW)=(LineIndices=(169,170))

	GUDLines(171)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.ShortCommentBeforeBlowingUp.HoffmanChatter_Crap_Medium01Cue')
	GUDLines(172)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.ShortCommentBeforeBlowingUp.HoffmanChatter_Damn_Loud01Cue')
	GUDLines(173)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.ShortCommentBeforeBlowingUp.HoffmanChatter_IAmFuckedNow_Medium01Cue')
	GUDLines(174)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.ShortCommentBeforeBlowingUp.HoffmanChatter_NotGood_Medium04Cue')
	GUDLines(175)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.ShortCommentBeforeBlowingUp.HoffmanChatter_OhBoy_Medium01Cue')
	GUDLines(176)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.ShortCommentBeforeBlowingUp.HoffmanChatter_OhShit_Loud01Cue')
	GUDLines(177)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.ShortCommentBeforeBlowingUp.HoffmanChatter_OhThisIsGoingToHurt_Medium01Cue')
	GUDLines(178)=(Audio=SoundCue'Human_Hoffman_Chatter_Cue.ShortCommentBeforeBlowingUp.HoffmanChatter_UhOh_Medium04Cue',bAlwaysLoad=1)
	GUDActions(GUDAction_StuckExplosiveToEnemy_R)=(LineIndices=(171,172,173,174,175,176,177,178))

}
