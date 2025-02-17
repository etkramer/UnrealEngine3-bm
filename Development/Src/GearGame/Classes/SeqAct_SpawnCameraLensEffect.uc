/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_SpawnCameraLensEffect extends SequenceAction;

/** This is the CameraLensEffect to spawn on the player(s) attached. **/
var() class<Emit_CameraLensEffectBase> LensEffectToSpawn;


defaultproperties
{
	ObjName="Spawn Camera Lens Effect"
	ObjCategory="Camera"
}
