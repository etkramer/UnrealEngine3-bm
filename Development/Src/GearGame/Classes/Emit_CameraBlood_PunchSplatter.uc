/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Emit_CameraBlood_PunchSplatter extends Emit_CameraLensEffectBase
	config(Weapon);



defaultproperties
{
	//Screen effect to be timed with each punch. Will spawn small splatters with each hit (has a small chance (25%) to spawn nothing, for randomness)
	PS_CameraEffect=ParticleSystem'CameraBlood.FX.P_Punch_Execution_Blood_Splatter_01'
	PS_CameraEffectNonExtremeContent=none
	bAllowMultipleInstances=TRUE
}



