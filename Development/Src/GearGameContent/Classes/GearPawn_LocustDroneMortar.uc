/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustDroneMortar extends GearPawn_LocustDrone;

defaultproperties
{
	HelmetType=class'Item_Helmet_None'
	ShoulderPadLeftType=class'Item_ShoulderPad_LocustThreeCylinders'
	ShoulderPadRightType=class'Item_ShoulderPad_LocustThreeCylinders'

	DefaultInventory.Empty()
	DefaultInventory(0)=class'GearGameContent.GearWeap_HeavyMortar'
	DefaultInventory(1)=class'GearGame.GearWeap_LocustPistol'
}
