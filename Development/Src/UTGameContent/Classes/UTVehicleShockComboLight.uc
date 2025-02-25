/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTVehicleShockComboLight extends UTExplosionLight;

defaultproperties
{
	HighDetailFrameTime=+0.02
	Brightness=40
	Radius=384
	LightColor=(R=186,G=145,B=204,A=255)
		//FalloffExponent=4

	TimeShift=((StartTime=0.0,Radius=384,Brightness=80,LightColor=(R=176,G=165,B=239,A=255)),(StartTime=0.3,Radius=128,Brightness=80,LightColor=(R=176,G=64,B=239,A=255)),(StartTime=0.4,Radius=128,Brightness=0,LightColor=(R=255,G=64,B=255,A=255)))
}