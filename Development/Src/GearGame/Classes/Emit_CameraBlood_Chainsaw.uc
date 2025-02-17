/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Emit_CameraBlood_Chainsaw extends Emit_CameraLensEffectBase
	config(Weapon);


defaultproperties
{
	PS_CameraEffect=ParticleSystem'CameraBlood.FX.P_CameraBlood'
	PS_CameraEffectNonExtremeContent=none
	bAllowMultipleInstances=TRUE
}



