/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustBeastLordMP extends GearPawn_LocustHunterNoArmorMP
	config(Pawn);


DefaultProperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_LocustHeads',U=0,V=192,UL=62,VL=62)
	DefaultInventory.Empty()
	DefaultInventory(0)=class'GearGame.GearWeap_LocustAssaultRifle'
	DefaultInventory(1)=class'GearGame.GearWeap_LocustPistol'

	HelmetType=class'Item_Helmet_LocustBeastRider'
}