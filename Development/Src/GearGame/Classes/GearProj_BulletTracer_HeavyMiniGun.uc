/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_BulletTracer_HeavyMiniGun extends GearProj_BulletTracer;

defaultproperties
{
	AccelRate=12000
	Speed=7000
	MaxSpeed=25000
	LifeSpan=6.000

	Begin Object Name=TracerMeshComp
		StaticMesh=StaticMesh'COG_GatlingGun.Mesh.S_Gatling_Tracer_New'
	End Object

	TracerDrawScale3D=(X=2.0f,Y=1.0f,Z=1.0f)
}
