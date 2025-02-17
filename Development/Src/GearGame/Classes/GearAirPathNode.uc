/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAirPathNode extends PathNode
	placeable
	native;

cpptext
{
//	virtual void AddForcedSpecs( AScout *Scout );
	virtual void FindBase() {}
	virtual UBOOL CanConnectTo(ANavigationPoint* Dest, UBOOL bCheckDistance);
}

defaultproperties
{
	Begin Object NAME=Sprite
		Sprite=Texture2D'EditorResources.VolumePath'
	End Object

	bSpecialMove=TRUE
	bBuildLongPaths=FALSE
	bNotBased=true
}
