/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Clip markers for brush clipping mode.  2 or 3 of these are placed in a level to define a clipping plane.
 * These should NOT be manually added to the level -- the editor automatically adds and deletes them as necessary.
 */
class ClipMarker extends Keypoint
	placeable
	native;

defaultproperties
{
	Begin Object NAME=Sprite
		Sprite=Texture2D'EditorResources.S_ClipMarker'
	End Object
	
	bEdShouldSnap=true
	bStatic=true
}
