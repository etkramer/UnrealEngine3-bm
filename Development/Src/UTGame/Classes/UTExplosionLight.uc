/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class UTExplosionLight extends PointLightComponent
	native;

/** set false after frame rate dependent properties have been tweaked. */
var bool bCheckFrameRate;
/** used to initialize light properties from TimeShift on spawn so you don't have to update initial values in two places */
var bool bInitialized;

/** HighDetailFrameTime - if frame rate is above this, force super high detail. */
var float HighDetailFrameTime;

/** Lifetime - how long this explosion has been going */
var float Lifetime;

/** Index into TimeShift array */
var int TimeShiftIndex;

struct native LightValues
{
	var float StartTime;
	var float Radius;
	var float Brightness;
	var color LightColor;
};

var() array<LightValues> TimeShift;

final native function ResetLight();

/** called when the light has burnt out */
delegate OnLightFinished(UTExplosionLight Light);

cpptext
{
	virtual void Attach();
	virtual void Tick(FLOAT DeltaTime);
}

defaultproperties
{
	HighDetailFrameTime=+0.015
	bCheckFrameRate=true
	Brightness=8
	Radius=256
	CastShadows=false
	LightColor=(R=255,G=255,B=255,A=255)
}
