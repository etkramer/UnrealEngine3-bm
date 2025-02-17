/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_GearCameraShake extends SequenceAction;

/** deprecated in version 3. */
var const editconst ScreenShakeStruct	CameraShake;

/** TRUE to do a code-driven camera shake, defined by CodeShakeParams. */
var() bool bDoCodeDrivenShake;
/** Parameters defining the code-driven camera shake. */
var() ScreenShakeStruct	CodeShake_Params;
var() float	CodeShake_GlobalScale;
var() float	CodeShake_GlobalAmplitudeScale;
var() float	CodeShake_GlobalFrequencyScale;
/** Whether or not to do do controller vibration.  Code shakes only for now. */
var() bool	bDoControllerVibration;


/** TRUE to do a CameraAnim-driven camera shake, defined by CameraAnimShakeParams. */
var() bool bDoAnimDrivenShake;
/** Parameters defining the animation-driven camera shake. */
var() ScreenShakeAnimStruct	AnimShake_Params;


/** Scale shake by distance from Location actor. */
var() bool		bRadialFalloff;
/** Radius inside which the shake is full magnitude.  For CST_Radial shakes only. */
var() float		RadialShake_InnerRadius;
/** Radius at which the shake reaches 0 magnitude.  For CST_Radial shakes only. */
var() float		RadialShake_OuterRadius;
/** Falloff exponent, defines falloff between inner and outer radii.  For CST_Radial shakes only. */
var() float		RadialShake_Falloff;


var Actor		LocationActor;


function VersionUpdated(int OldVersion, int NewVersion)
{
	if ( (OldVersion <= 2) && (NewVersion >= 3) )
	{
		CodeShake_Params = CameraShake;
	}

	super.VersionUpdated(OldVersion, NewVersion);
}

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
	return Super.GetObjClassVersion() + 2;
}

defaultproperties
{
	ObjName="Camera Shake"
	ObjCategory="Camera"

	// assumes code shake by default
	bDoCodeDrivenShake=TRUE
	CodeShake_GlobalScale=1.f
	CodeShake_GlobalAmplitudeScale=1.f
	CodeShake_GlobalFrequencyScale=1.f
	bDoControllerVibration=TRUE

	RadialShake_InnerRadius=128
	RadialShake_OuterRadius=512
	RadialShake_Falloff=2.f

	InputLinks(0)=(LinkDesc="Timed")
	InputLinks(1)=(LinkDesc="Continuous")
	InputLinks(2)=(LinkDesc="Stop")

	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Location",PropertyName=LocationActor)
}
