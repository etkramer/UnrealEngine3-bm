/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearCamMod_ScreenShake extends GearCameraModifier
	native  // this needs to be in the base classes.h
	config(Camera);


/** Active ScreenShakes array */
var		Array<ScreenShakeStruct>	Shakes;

/** Always active ScreenShake for testing purposes */
var()	ScreenShakeStruct			TestShake;

/** Alpha to use while targeting. */
var()	protected float				TargetingAlpha;


final function RemoveAllScreenShakes()
{
	Shakes.Length = 0;
}

final function RemoveScreenShake(Name ShakeName)
{
	local int Idx;
	Idx = Shakes.Find('ShakeName',ShakeName);
	if (Idx != INDEX_NONE)
	{
		Shakes.Remove(Idx,1);
	}
}

/** Add a new screen shake to the list */
final function AddScreenShake( ScreenShakeStruct NewShake )
{
	local int ShakeIdx, NumShakes;

	NumShakes = Shakes.Length;

	// search for existng shake of same name
	if (NewShake.ShakeName != '')
	{
		for (ShakeIdx=0; ShakeIdx<NumShakes; ++ShakeIdx)
		{
			if (Shakes[ShakeIdx].ShakeName == NewShake.ShakeName)
			{
				// found matching shake, reinit with new params!
				Shakes[ShakeIdx] = InitializeShake(NewShake);
				return;
			}
		}
	}

	// Initialize new screen shake and add it to the list of active shakes
	Shakes[NumShakes] = InitializeShake( NewShake );
}

/** Initialize screen shake structure */
final function ScreenShakeStruct InitializeShake( ScreenShakeStruct NewShake )
{
	NewShake.TimeToGo	= NewShake.TimeDuration;

	if( !IsZero( NewShake.RotAmplitude ) )
	{
		NewShake.RotSinOffset.X		= InitializeOffset( NewShake.RotParam.X );
		NewShake.RotSinOffset.Y		= InitializeOffset( NewShake.RotParam.Y );
		NewShake.RotSinOffset.Z		= InitializeOffset( NewShake.RotParam.Z );
	}

	if( !IsZero( NewShake.LocAmplitude ) )
	{
		NewShake.LocSinOffset.X		= InitializeOffset( NewShake.LocParam.X );
		NewShake.LocSinOffset.Y		= InitializeOffset( NewShake.LocParam.Y );
		NewShake.LocSinOffset.Z		= InitializeOffset( NewShake.LocParam.Z );
	}

	if( NewShake.FOVAmplitude != 0 )
	{
		NewShake.FOVSinOffset		= InitializeOffset( NewShake.FOVParam );
	}

	return NewShake;
}

/** Initialize sin wave start offset */
final static function float InitializeOffset( EShakeParam Param )
{
	Switch( Param )
	{
		case ESP_OffsetRandom	: return FRand() * 2 * Pi;	break;
		case ESP_OffsetZero		: return 0;					break;
	}

	return 0;
}

/**
 * ComposeNewShake
 * Take Screen Shake parameters and create a new ScreenShakeStruct variable
 *
 * @param	Duration			Duration in seconds of shake
 * @param	newRotAmplitude		view rotation amplitude (pitch,yaw,roll)
 * @param	newRotFrequency		frequency of rotation shake
 * @param	newLocAmplitude		relative view offset amplitude (x,y,z)
 * @param	newLocFrequency		frequency of view offset shake
 * @param	newFOVAmplitude		fov shake amplitude
 * @param	newFOVFrequency		fov shake frequency
 */
final function ScreenShakeStruct ComposeNewShake
(
	float	Duration,
	vector	newRotAmplitude,
	vector	newRotFrequency,
	vector	newLocAmplitude,
	vector	newLocFrequency,
	float	newFOVAmplitude,
	float	newFOVFrequency
)
{
	local ScreenShakeStruct	NewShake;

	NewShake.TimeDuration	= Duration;

	NewShake.RotAmplitude	= newRotAmplitude;
	NewShake.RotFrequency	= newRotFrequency;

	NewShake.LocAmplitude	= newLocAmplitude;
	NewShake.LocFrequency	= newLocFrequency;

	NewShake.FOVAmplitude	= newFOVAmplitude;
	NewShake.FOVFrequency	= newFOVFrequency;

	return NewShake;
}

/**
 * StartNewShake
 *
 * @param	Duration			Duration in seconds of shake
 * @param	newRotAmplitude		view rotation amplitude (pitch,yaw,roll)
 * @param	newRotFrequency		frequency of rotation shake
 * @param	newLocAmplitude		relative view offset amplitude (x,y,z)
 * @param	newLocFrequency		frequency of view offset shake
 * @param	newFOVAmplitude		fov shake amplitude
 * @param	newFOVFrequency		fov shake frequency
 */
function StartNewShake
(
	float	Duration,
	vector	newRotAmplitude,
	vector	newRotFrequency,
	vector	newLocAmplitude,
	vector	newLocFrequency,
	float	newFOVAmplitude,
	float	newFOVFrequency
)
{
	local ScreenShakeStruct	NewShake;

	// check for a shake abort
	if (Duration == -1.f)
	{
		Shakes.Length = 0;
	}
	else
	{
		NewShake = ComposeNewShake
		(
			Duration,
			newRotAmplitude,
			newRotFrequency,
			newLocAmplitude,
			newLocFrequency,
			newFOVAmplitude,
			newFOVFrequency
		);

		AddScreenShake( NewShake );
	}
}

/** Update a ScreenShake */
native function UpdateScreenShake(float DeltaTime, out ScreenShakeStruct Shake, out TPOV OutPOV);


native function float GetTargetAlpha( Camera Camera );

/** @see CameraModifer::ModifyCamera */
native function bool ModifyCamera
(
		Camera	Camera,
		float	DeltaTime,
	out TPOV	OutPOV
);

defaultproperties
{
	TargetingAlpha=0.5f
}
