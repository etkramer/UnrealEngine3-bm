/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustLancerGuard extends GearPawn_LocustDrone
	config(Pawn);


DefaultProperties
{
	HelmetType=class'Item_Helmet_LocustLancerGuard'
	ShoulderPadLeftType=class'Item_ShoulderPad_LocustOverLappingScales'

	DefaultInventory.Empty()
	DefaultInventory(0)=class'GearGame.GearWeap_AssaultRifle'
}