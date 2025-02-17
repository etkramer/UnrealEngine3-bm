/**
 * Actor used for testing changes to object instancing and verifying functionality.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SubobjectTestActor extends Actor
	placeable;

var() NestingTest_FirstLevelComponent Component;
var() instanced NestingTest_FirstLevelSubobject Subobject;

DefaultProperties
{
	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'EditorResources.S_Actor'
		HiddenGame=True
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(Sprite)

	Begin Object Class=NestingTest_FirstLevelComponent Name=ComponentTemplate
	End Object
	Component=ComponentTemplate

	Begin Object Class=NestingTest_FirstLevelSubobject Name=SubobjectTemplate
	End Object
	Subobject=SubobjectTemplate
}
