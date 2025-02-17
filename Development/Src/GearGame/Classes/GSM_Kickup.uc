
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_Kickup extends GSM_Pickup;

defaultproperties
{
	BS_Animation=(AnimName[BS_FullBody]="AR_Idle_Ready_Kickup")
	SpecialMoveCameraBoneAnims(0)=(AnimName="Camera_Kickup_A",CollisionTestVector=(X=-103.f,Y=136.f,Z=83.f))
	SpecialMoveCameraBoneAnims(1)=(AnimName="Camera_Kickup_B",CollisionTestVector=(X=-123.f,Y=54.f,Z=141.f))
}