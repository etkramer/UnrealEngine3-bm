/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearCTDefensePoint extends Trigger
	deprecated;

defaultproperties
{
	Begin Object NAME=Sprite
		Sprite=Texture2D'GearsIcons.WingmanIcon'
		Scale=0.25
	End Object

	Begin Object Name=CollisionCylinder
		CollisionRadius=256.f
		CollisionHeight=32.f
	End Object
}

