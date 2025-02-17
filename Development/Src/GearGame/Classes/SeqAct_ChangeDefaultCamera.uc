/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_ChangeDefaultCamera extends SequenceAction
	dependson(GearPlayerCamera)
	deprecated;

// @fixme, jf: need to reimplement?  give LDs a full set of offsets they can play with?
//var() GearPlayerCamera.EAltDefaultCameraModes		NewDefaultCamera;

var() float	PitchOffsetDegrees;

/**
 * Return the version number for this class.  Child classes should increment this method by calling Super then adding
 * a individual class version to the result.  When a class is first created, the number should be 0; each time one of the
 * link arrays is modified (VariableLinks, OutputLinks, InputLinks, etc.), the number that is added to the result of
 * Super.GetObjClassVersion() should be incremented by 1.
 *
 * @return	the version number for this specific class.
 */
static event int GetObjClassVersion()
{
	return Super.GetObjClassVersion() + 3;
}

defaultproperties
{
	PitchOffsetDegrees=0.f

	ObjName="Change Default Camera"
	ObjCategory="Camera"

	InputLinks(0)=(LinkDesc="Start")
	InputLInks(1)=(LinkDesc="Stop")
}
