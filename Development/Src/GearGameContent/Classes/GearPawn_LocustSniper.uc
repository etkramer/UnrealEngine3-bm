/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 */
class GearPawn_LocustSniper extends GearPawn_LocustDroneBase
	config(Pawn);


defaultproperties
{
	DefaultInventory.Empty()
	DefaultInventory(0)=class'GearGame.GearWeap_SniperRifle'
	DefaultInventory(1)=class'GearGame.GearWeap_LocustPistol'

	HelmetType=class'Item_Helmet_LocustSniperRandom'
	ShoulderPadLeftType=class'Item_ShoulderPad_LocustSpade'
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=343,V=0,UL=48,VL=63)
}
