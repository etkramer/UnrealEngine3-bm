/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class FlockAttractor extends CrowdAttractor
	placeable;

defaultproperties
{
	Begin Object Name=Sprite
		Sprite=Texture2D'GearsIcons.CrowdAttract'
		Scale=0.25  // we are using 128x128 textures so we need to scale them down
	End Object
}
