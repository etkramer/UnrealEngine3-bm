/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class FlockTest_Spawner extends Actor
	abstract
	deprecated;

defaultproperties
{
	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'GearsIcons.CrowdControl'
		Scale=0.25  // we are using 128x128 textures so we need to scale them down
		HiddenGame=true
		HiddenEditor=false
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(Sprite)
}