/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class UTBioExplosionLight extends UTExplosionLight;

defaultproperties
{
	HighDetailFrameTime=+0.02
	Brightness=200
	Radius=48
	LightColor=(R=128,G=255,B=192,A=255)

	TimeShift=((StartTime=0.0,Radius=48,Brightness=20,LightColor=(R=128,G=255,B=128,A=255)),(StartTime=0.11,Radius=48,Brightness=10,LightColor=(R=64,G=255,B=64,A=255)),(StartTime=0.15,Radius=32,Brightness=0,LightColor=(R=0,G=255,B=0,A=255)))
}
