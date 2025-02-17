/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class TargetPoint extends KeyPoint;

defaultproperties
{
	Begin Object Name=Sprite
		Sprite=Texture2D'EditorMaterials.TargetIcon'
		Scale=0.35
	End Object

	Begin Object Class=ArrowComponent Name=Arrow
		ArrowColor=(R=150,G=200,B=255)
		ArrowSize=0.5
		bTreatAsASprite=True
		HiddenGame=true
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(Arrow)

	bStatic=False
	bNoDelete=true
	bMovable=True
}
