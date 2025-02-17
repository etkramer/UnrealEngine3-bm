/**
 * Base class for all actors used in testing and verifying the behavior of components.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ComponentTestActorBase extends Actor
	placeable
	abstract
	native;

DefaultProperties
{
	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'EditorResources.S_Actor'
		HiddenGame=False
		AlwaysLoadOnClient=True
		AlwaysLoadOnServer=True
	End Object
	Components.Add(Sprite)
}
