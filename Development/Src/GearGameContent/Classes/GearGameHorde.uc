/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearGameHorde extends GearGameHorde_Base;

// creates a hard reference to all the various Pawns we might spawn for cooking
var array< class<Pawn> > PawnClasses;

defaultproperties
{
	HUDType=class'GearGameContent.GearHUDInvasion'

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

	PawnClasses.Add(class'GearPawn_LocustDrone')
	PawnClasses.Add(class'GearPawn_LocustBoomer')
	PawnClasses.Add(class'GearPawn_LocustBoomerFlail')
	PawnClasses.Add(class'GearPawn_LocustBoomerFlame')
	PawnClasses.Add(class'GearPawn_LocustBoomerGatling')
	PawnClasses.Add(class'GearPawn_LocustBoomerButcher')
	PawnClasses.Add(class'GearPawn_LocustWretch')
	PawnClasses.Add(class'GearPawn_LocustTheron')
	PawnClasses.Add(class'GearPawn_LocustTicker')
	PawnClasses.Add(class'GearPawn_LocustHunterArmorNoGrenades')
	PawnClasses.Add(class'GearPawn_LocustHunterNoArmorNoGrenades')
	PawnClasses.Add(class'GearPawn_LocustSire')
	PawnClasses.Add(class'GearPawn_LocustKantus')
	PawnClasses.Add(class'GearPawn_LocustBloodMountWithDrone')
	PawnClasses.Add(class'GearPawn_LocustFlameDrone')
	PawnClasses.Add(class'GearPawn_LocustSpeedy')
	PawnClasses.Add(class'GearPawn_LocustLancerGuard')
	PawnClasses.Add(class'GearPawn_LocustSniper')

	FlameThrowerClass=class'GearWeap_FlameThrower'
}
