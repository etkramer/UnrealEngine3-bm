/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_BulletTracer_Brumak_Player extends GearProj_BulletTracer;

defaultproperties
{
	Speed=7200
	DrawScale=2.5
	LifeSpan=6.000

	Begin Object Name=TracerMeshComp
    	StaticMesh=StaticMesh'Locust_Brumak.Effects.S_Brumak_Gun_Tracer'
    End Object

	TracerDrawScale3D=(X=2.0f,Y=1.0f,Z=1.0f)
}

