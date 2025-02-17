/**
 *	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class SoftBody_Placeable extends Gear_SoftBodySpawnable
	placeable;

defaultproperties
{
	bNoDelete=TRUE
	bAutoRemove=FALSE

	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'EditorResources.S_Actor'
		HiddenGame=False
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(Sprite)
}
