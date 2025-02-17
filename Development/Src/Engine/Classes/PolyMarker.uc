//=============================================================================
// PolyMarker.
//
// These are markers for the polygon drawing mode.
//
// These should NOT be manually added to the level.  The editor adds and
// deletes them on it's own.
//
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class PolyMarker extends Keypoint
	placeable
	native;

defaultproperties
{
	Begin Object NAME=Sprite
		Sprite=Texture2D'EditorResources.S_PolyMarker'
	End Object

	bEdShouldSnap=true
	bStatic=true
}
