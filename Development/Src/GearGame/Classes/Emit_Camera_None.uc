/**
 * This is to allow a "none" effect to occur.  This is needed as we have a hierarchy and the code will always go
 * up up and find a camera effect.  So sometimes we want a physical material to not have any effect and we use this.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Emit_Camera_None extends Emit_CameraLensEffectBase
	config(Weapon);


defaultproperties
{
	PS_CameraEffect=none
	PS_CameraEffectNonExtremeContent=none
}



