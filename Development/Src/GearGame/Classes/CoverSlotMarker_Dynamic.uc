/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class CoverSlotMarker_Dynamic extends CoverSlotMarker
	native;

native simulated function InitializeDynamicMantleSpec(class<MantleReachSpec> SpecClass = class'MantleReachSpec');

native simulated function SetMaxPathSize(float Radius, float Height);

native simulated protected function RefreshOctreePosition();

simulated event string GetDebugAbbrev()
{
	return "DynCSM";
}

defaultproperties
{
	bStatic=FALSE
	bMovable=TRUE
	bHardAttach=TRUE

	Begin Object Class=SpriteComponent Name=Sprite3
		Sprite=Texture2D'EditorResources.S_NavP'
		HiddenEditor=FALSE
		HiddenGame=TRUE
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(Sprite3)
}
