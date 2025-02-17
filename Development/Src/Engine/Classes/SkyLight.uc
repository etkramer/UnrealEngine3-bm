/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SkyLight extends Light
	native
	placeable;

defaultproperties
{
	Begin Object Name=Sprite
		Sprite=Texture2D'EditorResources.S_SkyLight'
	End Object

	Begin Object Class=SkyLightComponent Name=SkyLightComponent0
		UseDirectLightMap=TRUE
		bCanAffectDynamicPrimitivesOutsideDynamicChannel=TRUE
	End Object
	LightComponent=SkylightComponent0
	Components.Add(SkyLightComponent0)
}
