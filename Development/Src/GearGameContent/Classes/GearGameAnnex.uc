/**
 * WarGameAnnex
 * Warfare Game Info for Annex Gametype
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearGameAnnex extends GearGameAnnex_Base;

/** Set the annex ring's color */
function SetAnnexRingColor( Color RingColor )
{
	Emit_AnnexArc(CommandPointEffect).SetAnnexColor( RingColor );
}

/** Set the ring progression for capturing */
function SetAnnexRingProgression( float Progression )
{
	Emit_AnnexArc(CommandPointEffect).SetAnnexProgression( Progression );
}


defaultproperties
{
	HUDType=class'GearGameContent.GearHUDAnnex'
	CommandPointEffectClass=class'Emit_AnnexArc'

	AnnexSound_CommandMeterBroken(0)=SoundCue'DLC_Annex.Annex.CommandPointBrokenCogCue'
	AnnexSound_CommandMeterBroken(1)=SoundCue'DLC_Annex.Annex.CommandPointBrokenLocCue'
	AnnexSound_CommandPointChanged=SoundCue'DLC_Annex.Annex.CommandPointMovesCogCue'
	AnnexSound_CommandTeamChanged(0)=SoundCue'DLC_Annex.Annex.TeamControlChangedCogCue'
	AnnexSound_CommandTeamChanged(1)=SoundCue'DLC_Annex.Annex.TeamControlChangedLocCue'
	AnnexSound_CommandMeterMaxed(0)=SoundCue'DLC_Annex.Annex.CommandPointMeterMaxedCogCue'
	AnnexSound_CommandMeterMaxed(1)=SoundCue'DLC_Annex.Annex.CommandPointMeterMaxedLocCue'

	COGClasses.Add((PawnClass=class'GearGameContent.GearPawn_COGMarcusMP'))
	COGClasses.Add((PawnClass=class'GearGameContent.GearPawn_COGMinhMP'))
	COGClasses.Add((PawnClass=class'GearGameContent.GearPawn_COGGusMP'))
	COGClasses.Add((PawnClass=class'GearGameContent.GearPawn_COGBairdMP'))
	COGClasses.Add((PawnClass=class'GearGameContent.GearPawn_COGDomMP'))
	COGClasses.Add((PawnClass=class'GearGameContent.GearPawn_COGCarmineMP'))
	COGClasses.Add((PawnClass=class'GearGameContent.GearPawn_COGBenjaminCarmineMP'))
	COGClasses.Add((PawnClass=class'GearGameContent.GearPawn_COGTaiMP'))
	COGClasses.Add((PawnClass=class'GearGameContent.GearPawn_COGDizzyMP'))
	COGClasses.Add((PawnClass=class'GearGameContent.GearPawn_COGHoffmanMP')) // must be at end of list

	LocustClasses.Add((PawnClass=class'GearGameContent.GearPawn_LocustMP'))
	LocustClasses.Add((PawnClass=class'GearGameContent.GearPawn_LocustFlameDroneMP'))
	LocustClasses.Add((PawnClass=class'GearGameContent.GearPawn_LocustSniperMP'))
	LocustClasses.Add((PawnClass=class'GearGameContent.GearPawn_LocustHunterArmorMP'))
	LocustClasses.Add((PawnClass=class'GearGameContent.GearPawn_LocustHunterNoArmorMP'))
	LocustClasses.Add((PawnClass=class'GearGameContent.GearPawn_LocustTheronMP'))
	LocustClasses.Add((PawnClass=class'GearGameContent.GearPawn_LocustTheronWithHelmetMP'))
	LocustClasses.Add((PawnClass=class'GearGameContent.GearPawn_LocustChopperMP'))
	LocustClasses.Add((PawnClass=class'GearGameContent.GearPawn_LocustBeastLordMP'))
	LocustClasses.Add((PawnClass=class'GearGameContent.GearPawn_LocustKantusMP'))
	LocustClasses.Add((PawnClass=class'GearGameContent.GearPawn_LocustSkorgeMP'))
	LocustClasses.Add((PawnClass=class'GearGameContent.GearPawn_LocustGeneralRaamMP')) // must be at end of list

	WeaponLookupTable(0)=(ProfileId=WeapSwap_FragGrenade,WeapClass=class'GearWeap_FragGrenade',WeapSwapType=eGEARWEAP_FragGrenade)
	WeaponLookupTable(1)=(ProfileId=WeapSwap_InkGrenade,WeapClass=class'GearWeap_InkGrenade',WeapSwapType=eGEARWEAP_InkGrenade)
	WeaponLookupTable(2)=(ProfileId=-1,WeapClass=class'GearWeap_SmokeGrenade',WeapSwapType=eGEARWEAP_SmokeGrenade)
	WeaponLookupTable(3)=(ProfileId=WeapSwap_Boomshot,WeapClass=class'GearWeap_Boomshot',WeapSwapType=eGEARWEAP_Boomshot)
	WeaponLookupTable(4)=(ProfileId=WeapSwap_Flame,WeapClass=class'GearWeap_FlameThrower',WeapSwapType=eGEARWEAP_Flame)
	WeaponLookupTable(5)=(ProfileId=WeapSwap_Sniper,WeapClass=class'GearWeap_SniperRifle',WeapSwapType=eGEARWEAP_Sniper)
	WeaponLookupTable(6)=(ProfileId=WeapSwap_Bow,WeapClass=class'GearWeap_Bow',WeapSwapType=eGEARWEAP_Bow)
	WeaponLookupTable(7)=(ProfileId=WeapSwap_Mulcher,WeapClass=class'GearWeap_HeavyMiniGun',WeapSwapType=eGEARWEAP_Mulcher)
	WeaponLookupTable(8)=(ProfileId=WeapSwap_Mortar,WeapClass=class'GearWeap_HeavyMortar',WeapSwapType=eGEARWEAP_Mortar)
	WeaponLookupTable(9)=(ProfileId=WeapSwap_HOD,WeapClass=class'GearWeap_HOD',WeapSwapType=eGEARWEAP_HOD)
	WeaponLookupTable(10)=(ProfileId=WeapSwap_Gorgon,WeapClass=class'GearWeap_LocustBurstPistol',WeapSwapType=eGEARWEAP_Gorgon)
	WeaponLookupTable(11)=(ProfileId=WeapSwap_Boltok,WeapClass=class'GearWeap_LocustPistol',WeapSwapType=eGEARWEAP_Boltok)
	WeaponLookupTable(12)=(ProfileId=-1,WeapClass=class'GearWeap_COGPistol',WeapSwapType=eGEARWEAP_Pistol)
	WeaponLookupTable(13)=(ProfileId=-1,WeapClass=class'GearWeap_AssaultRifle',WeapSwapType=eGEARWEAP_Lancer)
	WeaponLookupTable(14)=(ProfileId=-1,WeapClass=class'GearWeap_LocustAssaultRifle',WeapSwapType=eGEARWEAP_Hammerburst)
	WeaponLookupTable(15)=(ProfileId=-1,WeapClass=class'GearWeap_Shotgun',WeapSwapType=eGEARWEAP_Shotgun)
	WeaponLookupTable(16)=(ProfileId=WeapSwap_Shield,WeapClass=class'BoomerShield',WeapSwapType=eGEARWEAP_Shield)
	WeaponLookupTable(17)=(ProfileId=-1,WeapClass=class'GearWeap_AssaultRifle_Golden',WeapSwapType=eGEARWEAP_GoldLancer)
	WeaponLookupTable(18)=(ProfileId=-1,WeapClass=class'GearWeap_LocustAssaultRifle_Golden',WeapSwapType=eGEARWEAP_GoldHammer)

	EncouragementData(ET_WonMatchByShutout)={(
		CogSounds=(SoundCue'Human_Prescott_Dialog_Cue.WinsBattleWithLocust.ChairmanChatter_OnlyThingMattersIsYouWon01Cue',SoundCue'Human_Prescott_Dialog_Cue.WinsBattleWithLocust.ChairmanChatter_ProudToBeYourChairman04Cue',SoundCue'Human_Prescott_Dialog_Cue.WinsBattleWithLocust.ChairmanChatter_BloodyVictoryOverAnyLoss01Cue',SoundCue'Human_Prescott_Dialog_Cue.WinsBattleWithLocust.ChairmanChatter_ViolentWrecklessGoodWork01Cue',SoundCue'Human_Prescott_Dialog_Cue.WinsBattleWithLocust.ChairmanChatter_TextbookDeployment01Cue'),
		LocustSounds=(SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_PERFECT_MATCH01',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_PERFECT_MATCH02',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_PERFECT_MATCH03',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_PERFECT_MATCH04',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_PERFECT_MATCH05') )}

	EncouragementData(ET_WonMatch)={(
		CogSounds=(SoundCue'Human_Prescott_Dialog_Cue.WinsBattleWithLocust.ChairmanChatter_OnlyThingMattersIsYouWon01Cue',SoundCue'Human_Prescott_Dialog_Cue.WinsBattleWithLocust.ChairmanChatter_ProudToBeYourChairman04Cue',SoundCue'Human_Prescott_Dialog_Cue.WinsBattleWithLocust.ChairmanChatter_BloodyVictoryOverAnyLoss01Cue',SoundCue'Human_Prescott_Dialog_Cue.WinsBattleWithLocust.ChairmanChatter_ViolentWrecklessGoodWork01Cue',SoundCue'Human_Prescott_Dialog_Cue.WinsBattleWithLocust.ChairmanChatter_TextbookDeployment01Cue'),
		LocustSounds=(SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_WIN_MATCH01',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_WIN_MATCH02',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_WIN_MATCH03',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_WIN_MATCH04',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_WIN_MATCH05') )}

	EncouragementData(ET_LostMatch)={(
		CogSounds=(SoundCue'Human_Prescott_Dialog_Cue.LosesBattleWithLocust.ChairmanChatter_IndigentFathersInduldgentMothers02Cue',SoundCue'Human_Prescott_Dialog_Cue.LosesBattleWithLocust.ChairmanChatter_LillyliveredPanywasteBullshit04Cue',SoundCue'Human_Prescott_Dialog_Cue.LosesBattleWithLocust.ChairmanChatter_LocustJustHandYouAsses02Cue',SoundCue'Human_Prescott_Dialog_Cue.LosesBattleWithLocust.ChairmanChatter_IShouldHaveYouCourtMarshalled03Cue',SoundCue'Human_Prescott_Dialog_Cue.LosesBattleWithLocust.ChairmanChatter_OnlyThingMattersIsYouLost01Cue'),
		LocustSounds=(SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_LOSE_MATCH01',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_LOSE_MATCH02',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_LOSE_MATCH03',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_LOSE_MATCH04',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_LOSE_MATCH05') )}

	EncouragementData(ET_LostMatchByShutout)={(
		CogSounds=(SoundCue'Human_Prescott_Dialog_Cue.LosesBattleWithLocust.ChairmanChatter_IndigentFathersInduldgentMothers02Cue',SoundCue'Human_Prescott_Dialog_Cue.LosesBattleWithLocust.ChairmanChatter_LillyliveredPanywasteBullshit04Cue',SoundCue'Human_Prescott_Dialog_Cue.LosesBattleWithLocust.ChairmanChatter_LocustJustHandYouAsses02Cue',SoundCue'Human_Prescott_Dialog_Cue.LosesBattleWithLocust.ChairmanChatter_IShouldHaveYouCourtMarshalled03Cue',SoundCue'Human_Prescott_Dialog_Cue.LosesBattleWithLocust.ChairmanChatter_OnlyThingMattersIsYouLost01Cue'),
		LocustSounds=(SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_SHUTOUT_MATCH01',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_SHUTOUT_MATCH02',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_SHUTOUT_MATCH03',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_SHUTOUT_MATCH04',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_SHUTOUT_MATCH05') )}

	EncouragementData(ET_MatchOrRoundDraw)={(
		CogSounds=(SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw01_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw02_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw03_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw04_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw05_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw06_Cue'),
		LocustSounds=(SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw01_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw02_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw03_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw04_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw05_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw06_Cue') )}

	EncouragementData(ET_WonRound)={(
		CogSounds=(SoundCue'Human_Prescott_Dialog_Cue.WinsBattleWithLocust.ChairmanChatter_MissionIsNotOver01Cue',SoundCue'Human_Prescott_Dialog_Cue.WinsBattleWithLocust.ChairmanChatter_ShowNoMercy01Cue',SoundCue'Human_Prescott_Dialog_Cue.WinsBattleWithLocust.ChairmanChatter_ThatsWhatILikeContinue02Cue',SoundCue'Human_Prescott_Dialog_Cue.WinsBattleWithLocust.ChairmanChatter_YouMakeCogLookGood01Cue',SoundCue'Human_Prescott_Dialog_Cue.WinsBattleWithLocust.ChairmanChatter_ShiningExample02Cue'),
		LocustSounds=(SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_WIN_ROUND01',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_WIN_ROUND02',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_WIN_ROUND03',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_WIN_ROUND04',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_WIN_ROUND05',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_WIN_ROUND06',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_WIN_ROUND07',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_WIN_ROUND08',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_WIN_ROUND09',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_WIN_ROUND10') )}

	EncouragementData(ET_LostRound)={(
		CogSounds=(SoundCue'Human_Prescott_Dialog_Cue.LosesBattleWithLocust.ChairmanChatter_ReloadRefocusRespawn01Cue',SoundCue'Human_Prescott_Dialog_Cue.LosesBattleWithLocust.ChairmanChatter_DoINeedToExplainIfLocustWin01Cue',SoundCue'Human_Prescott_Dialog_Cue.LosesBattleWithLocust.ChairmanChatter_HardenedSoldiersOrSpoiledBrats02Cue',SoundCue'Human_Prescott_Dialog_Cue.LosesBattleWithLocust.ChairmanChatter_DisappointedPerformanceUnacceptable02Cue',SoundCue'Human_Prescott_Dialog_Cue.LosesBattleWithLocust.ChairmanChatter_YoullLearnToSpeakLocust01Cue'),
		LocustSounds=(SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_LOSE_ROUND01',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_LOSE_ROUND02',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_LOSE_ROUND03',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_LOSE_ROUND04',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_LOSE_ROUND05',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_LOSE_ROUND06',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_LOSE_ROUND07',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_LOSE_ROUND08',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_LOSE_ROUND09',SoundCue'Human_Myrrah_Chatter_Cue.MP.LOCUST_LOSE_ROUND10') )}
}
