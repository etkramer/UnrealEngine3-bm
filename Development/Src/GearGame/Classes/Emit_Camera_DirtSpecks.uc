/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Emit_Camera_DirtSpecks extends Emit_CameraLensEffectBase
	config(Weapon);


defaultproperties
{
	PS_CameraEffect=ParticleSystem'CameraBlood.FX.P_Dirt_Specks'
	PS_CameraEffectNonExtremeContent=ParticleSystem'CameraBlood.FX.P_Dirt_Specks'
	bAllowMultipleInstances=TRUE
}



