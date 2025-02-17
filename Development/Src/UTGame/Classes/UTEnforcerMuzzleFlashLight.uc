/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class UTEnforcerMuzzleFlashLight extends UTExplosionLight;

defaultproperties
{
	HighDetailFrameTime=+0.02
	Brightness=4
	Radius=96
	LightColor=(R=255,G=255,B=255,A=255)
	TimeShift=((StartTime=0.0,Radius=96,Brightness=4,LightColor=(R=255,G=220,B=192,A=255)),(StartTime=0.2,Radius=64,Brightness=4,LightColor=(R=225,G=186,B=132,A=255)),(StartTime=0.25,Radius=64,Brightness=0,LightColor=(R=225,G=186,B=132,A=255)))
}
