/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class DecalActor extends DecalActorBase
	native(Decal)
	placeable;

defaultproperties
{
	Begin Object Class=DecalComponent Name=NewDecalComponent
		bStaticDecal=true
	End Object
	Decal=NewDecalComponent
	Components.Add(NewDecalComponent)

	bStatic=true
	bMovable=false
}
