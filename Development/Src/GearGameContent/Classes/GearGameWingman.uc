/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearGameWingman extends GearGameWingman_Base;

defaultproperties
{
	HUDType=class'GearGameContent.GearHUDWingman'

	COGClasses.Add((PawnClass=class'GearGameContent.GearPawn_COGMarcusMP'))
	COGClasses.Add((PawnClass=class'GearGameContent.GearPawn_LocustHunterNoArmorMP'))
	COGClasses.Add((PawnClass=class'GearGameContent.GearPawn_COGBenjaminCarmineMP'))
	COGClasses.Add((PawnClass=class'GearGameContent.GearPawn_LocustKantusMP'))
	COGClasses.Add((PawnClass=class'GearGameContent.GearPawn_COGDomMP'))
	COGClasses.Add((PawnClass=class'GearGameContent.GearPawn_COGGusMP'))
	COGClasses.Add((PawnClass=class'GearGameContent.GearPawn_COGBairdMP'))
	COGClasses.Add((PawnClass=class'GearGameContent.GearPawn_COGTaiMP'))
	COGClasses.Add((PawnClass=class'GearGameContent.GearPawn_COGDizzyMP'))
	COGClasses.Add((PawnClass=class'GearGameContent.GearPawn_LocustSniperMP'))
	COGClasses.Add((PawnClass=class'GearGameContent.GearPawn_LocustTheronMP'))
	COGClasses.Add((PawnClass=class'GearGameContent.GearPawn_LocustBeastLordMP'))

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

	// note: we want the music-only draw stingers every time, so we just duplicate the data in each slot
 	EncouragementData(ET_WonMatchByShutout)={(
 		CogSounds=(SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw01_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw02_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw03_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw04_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw05_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw06_Cue'),
 		LocustSounds=(SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw01_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw02_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw03_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw04_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw05_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw06_Cue') )}
 
 	EncouragementData(ET_WonMatch)={(
		CogSounds=(SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw01_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw02_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw03_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw04_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw05_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw06_Cue'),
		LocustSounds=(SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw01_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw02_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw03_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw04_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw05_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw06_Cue') )}
 
 	EncouragementData(ET_LostMatch)={(
		CogSounds=(SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw01_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw02_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw03_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw04_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw05_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw06_Cue'),
		LocustSounds=(SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw01_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw02_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw03_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw04_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw05_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw06_Cue') )}
 
 	EncouragementData(ET_LostMatchByShutout)={(
		CogSounds=(SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw01_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw02_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw03_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw04_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw05_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw06_Cue'),
		LocustSounds=(SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw01_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw02_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw03_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw04_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw05_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw06_Cue') )}
 
 	EncouragementData(ET_MatchOrRoundDraw)={(
 		CogSounds=(SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw01_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw02_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw03_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw04_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw05_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw06_Cue'),
 		LocustSounds=(SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw01_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw02_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw03_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw04_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw05_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw06_Cue') )}
 
 	EncouragementData(ET_WonRound)={(
		CogSounds=(SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw01_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw02_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw03_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw04_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw05_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw06_Cue'),
		LocustSounds=(SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw01_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw02_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw03_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw04_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw05_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw06_Cue') )}
 
 	EncouragementData(ET_LostRound)={(
		CogSounds=(SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw01_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw02_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw03_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw04_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw05_Cue',SoundCue'Human_Prescott_Dialog_Cue.MPDrawMatch.G2COGDraw06_Cue'),
		LocustSounds=(SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw01_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw02_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw03_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw04_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw05_Cue',SoundCue'Human_Myrrah_Chatter_Cue.Taunts.G2LocustDraw06_Cue') )}
}

