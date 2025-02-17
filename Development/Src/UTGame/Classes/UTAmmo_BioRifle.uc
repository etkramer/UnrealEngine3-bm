/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

//@FIXME: class should be removed after maps have switched to Content version
class UTAmmo_BioRifle extends UTAmmoPickupFactory
	native;

cpptext
{
	virtual void CheckForErrors()
	{
		Super::CheckForErrors();

		GWarn->MapCheck_Add(MCTYPE_ERROR, this, TEXT("This should be replaced with UTAmmo_BioRifle_Content"));
	}
}

defaultproperties
{
	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'EditorResources.S_Inventory'
		HiddenGame=True
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(Sprite)
}
