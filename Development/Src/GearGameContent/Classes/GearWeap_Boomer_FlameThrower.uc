/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_Boomer_FlameThrower extends GearWeap_FlameThrower;

var bool bIsLeftGun;

function bool IsActiveReloadActive(GearPawn GP)
{
	// flame boomers always get active reload bonus
	return TRUE;
}

defaultproperties
{
	DroppedWeaponClass=class'GearWeap_FlameThrower'
}
