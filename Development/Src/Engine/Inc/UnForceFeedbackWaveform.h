/*=============================================================================
	UnForceFeedbackWaveform.h: Definition of platform independent forcefeedback
	classes and structs.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/** The type of function that generates the waveform sample */
enum EWaveformFunction
{
	WF_Constant,
	WF_LinearIncreasing,
	WF_LinearDecreasing,
	WF_Sin0to90,
	WF_Sin90to180,
	WF_Sin0to180,
	WF_Noise
};

/** Holds a single sample's information */
struct FWaveformSample
{
    BYTE LeftAmplitude;
    BYTE RightAmplitude;
    BYTE LeftFunction;
    BYTE RightFunction;
    FLOAT Duration;

	friend FArchive& operator<<(FArchive& Ar,FWaveformSample& A)
	{	
		return	Ar << A.LeftAmplitude << A.RightAmplitude << A.LeftFunction
			<< A.RightFunction << A.Duration;
	}
};

/**
 * This class manages the waveform data for a forcefeedback device,
 * specifically for the xbox gamepads.
 */
class UForceFeedbackWaveform : public UObject
{
    DECLARE_CLASS(UForceFeedbackWaveform,UObject,CLASS_SafeReplace|CLASS_NoExport,Engine)

	UBOOL bIsLooping;
    TArray<FWaveformSample> Samples;

	UForceFeedbackWaveform() {}

	void Serialize( FArchive& Ar )
	{
		Super::Serialize(Ar);
		Ar << bIsLooping;
		Ar << Samples;
	}
};

