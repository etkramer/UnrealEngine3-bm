/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class DecalActorMovable extends DecalActorBase
	native(Decal)
	placeable;

defaultproperties
{
	Begin Object Class=DecalComponent Name=NewDecalComponent
		bMovableDecal=TRUE
		bStaticDecal=TRUE
	End Object
	Decal=NewDecalComponent
	Components.Add(NewDecalComponent)

	bStatic=FALSE
	bMovable=TRUE
	bHardAttach=TRUE
}
