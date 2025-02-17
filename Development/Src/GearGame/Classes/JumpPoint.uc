/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class JumpPoint extends PathNode
	native;

/** List of destinations this node should try to create a jump spec to */
var() array<NavigationPoint> JumpDest;

defaultproperties
{
	bSpecialMove=TRUE
	bBuildLongPaths=FALSE
	
	Begin Object NAME=Sprite
		Sprite=Texture2D'EditorResources.S_NavP'
	End Object
}
