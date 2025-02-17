/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearSpectatorPoint extends CameraActor
	native;

/** This should be a tagname to the localized string. */
var() string DisplayText;

/** so the LDs can arrange the order we'll move from one point to another */
var() int OrderIndex;

/** Range he player can move the camera.  This is a delta from center.  */
var() const rotator UserRotationRange;

/** How fast the camera can rotate, in rotator units per second. */
var() const float MaxRotationRate;


/**
 * The text in DisplayText is actually a tagname to the localized string,
 * so we have to go retrieve it from the localization system.
 */
native simulated function String RetrieveDisplayString( String TagName );


defaultproperties
{
	OrderIndex=0
	MaxRotationRate=32768.f
}