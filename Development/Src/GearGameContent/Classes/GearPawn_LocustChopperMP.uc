
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustChopperMP extends GearPawn_LocustDroneMP
	config(Pawn);

simulated function vector GetWeaponAimIKPositionFix()
{
	return Super.GetWeaponAimIKPositionFix() + vect(0,1,-2);
}

DefaultProperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_LocustHeads',U=138,V=127,UL=62,VL=62)
	HelmetType=class'Item_Helmet_LocustLancerGuard'
}