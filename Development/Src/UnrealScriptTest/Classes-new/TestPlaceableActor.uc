/**
 * Used for testing various engine features during development.
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class TestPlaceableActor extends Actor
	HideCategories(Advanced,Attachment,Collision,Debug,Physics)
	placeable;

var()	editconst	InheritanceTestDerived		ComponentVar;
var()	InheritanceTestDerived					RuntimeComponent;

defaultproperties
{
	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'EditorResources.S_Actor'
		HiddenGame=True
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(Sprite)

	Begin Object Class=InheritanceTestDerived Name=TestComp
		TestInt=5
	End Object
	Components.Add(TestComp)
	ComponentVar=TestComp
}

