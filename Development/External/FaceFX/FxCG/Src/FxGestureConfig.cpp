//------------------------------------------------------------------------------
// The parameters for the gesture generation.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxGestureConfig.h"
#include "FxArchiveStoreFileFast.h"
#include "FxArchiveStoreMemory.h"
#include "FxArchiveStoreNull.h"
#include "FxArchiveStoreMemoryNoCopy.h"

namespace OC3Ent
{

namespace Face
{

FxSize FxCGGetNumSpeechGestureTracks()
{
	return GT_NumTracks;
}

FxName FxCGGetSpeechGestureTrackName(FxSize trackIndex)
{
	FxGestureTrack track = static_cast<FxGestureTrack>(trackIndex);
	switch( track )
	{
	case GT_OrientationHeadPitch:
		return "Orientation Head Pitch";
	case GT_OrientationHeadRoll:
		return "Orientation Head Roll";
	case GT_OrientationHeadYaw:
		return "Orientation Head Yaw";
	case GT_GazeEyePitch:
		return "Gaze Eye Pitch";
	case GT_GazeEyeYaw:
		return "Gaze Eye Yaw";
	case GT_EmphasisHeadPitch:
		return "Emphasis Head Pitch";
	case GT_EmphasisHeadRoll:
		return "Emphasis Head Roll";
	case GT_EmphasisHeadYaw:
		return "Emphasis Head Yaw";
	case GT_EyebrowRaise:
		return "Eyebrow Raise";
	case GT_Blink:
		return "Blink";
	default:
		return "Unknown Track Index";
	};
}

// Drift property constructors
FxDriftProperty::FxDriftProperty()
	: track(GT_Invalid)
	, probability(0.0f)
	, valueRange(FxCentredRange(0.0f, 0.0f))
{
}

FxDriftProperty::FxDriftProperty( FxGestureTrack iTrack, FxReal iProb, FxCentredRange iRange)
	: track(iTrack)
	, probability(iProb)
	, valueRange(iRange)
{
}

#define kCurrentFxGestureConfigVersion 2

FX_IMPLEMENT_CLASS(FxGestureConfig,kCurrentFxGestureConfigVersion,FxNamedObject);

// Gesture config constructor
FxGestureConfig::FxGestureConfig()
	: FxNamedObject("Default")
	, pfeAllowedFrequency(FxRange(40.0f, 750.0f))
	, pfeAvgPitchUpperNumSteps(2)
	, pfeAvgPitchLowerNumSteps(2)
	, pfePowerNumAdjacentPhones(15)
	, pfePowerMaxWindowLength(1.0f)
	, pfeAvgPowerContribution(0.25f)
	, pfeMaxPowerContribution(0.75f)
	, pfeHighPitchBonus(0.0f)
	, pfeLongDurationBonus(0.10f)
	, pfeRateOfSpeechNumAdjacentPhones(18)
	, sdStressThreshold(0.30f)
	, sdLongSilence(1.5f)
	, sdShortSilence(0.3f)
	, sdQuickStressLimit(0.30f)
	, sdIsolatedStressLimit(0.85f)
	, sdMinStressSeparation(0.35f)
	, rosAvgPhonemeDuration(FxRange(0.065f, 0.130f))
	, rosTimeScale(FxRange(0.75f, 3.0f))
	, odHeadDuration(FxRange(0.9f, 6.0))
	, odHeadTransition(FxRange(1.1f, 2.25))
	, odEyeDuration(FxRange(1.5f, 3.0f))
	, odEyeTransition(FxRange(0.040f, 0.120f))
	, eaCentreShift(0.0f)
	, blinkSeparation(FxRange(0.5f, 6.0f))
	, blinkLead(FxCentredRange(-0.125f, 0.015f))
	, blinkTail(FxCentredRange(0.083f, 0.015f))
	, gcSpeed(2.15f)
	, gcMagnitude(0.8f)
	, description("The default speech gesture configuration")
{
	// Set up the orientation drift properties.
	odHeadProperties.PushBack( FxDriftProperty(GT_OrientationHeadPitch, 0.50f, FxCentredRange(2.5f, 1.0f)) );
	odHeadProperties.PushBack( FxDriftProperty(GT_OrientationHeadRoll,  0.66f, FxCentredRange(1.5f, 1.0f)) );
	odHeadProperties.PushBack( FxDriftProperty(GT_OrientationHeadYaw,   0.66f, FxCentredRange(3.0f, 1.0f)) );

	odEyeProperties.PushBack(  FxDriftProperty(GT_GazeEyePitch, 0.13f, FxCentredRange(1.0f, 0.0f)) );
	odEyeProperties.PushBack(  FxDriftProperty(GT_GazeEyeYaw,   0.29f, FxCentredRange(0.6f, 0.0f)) );
	odEyeProperties.PushBack(  FxDriftProperty(GT_GazeEyeYaw,   0.29f, FxCentredRange(-0.6f, 0.0f)) );
	odEyeProperties.PushBack(  FxDriftProperty(GT_Invalid,		0.29f, FxCentredRange(0.0f, 1.0f)) );

	// Set up the emphasis actions.
	// Calculate a scaling factor so we can input the data exactly as Steve gave
	// it to us - as number of frames at 30fps.
	FxReal fps = 1.0f / 30.0f;

	FxEmphasisActionAnim strongHeadNod(EA_StrongHeadNod, FxRange(0.75f, 1.0f));
	strongHeadNod.tracks.PushBack(FxEmphasisActionTrack(GT_EmphasisHeadPitch, FxCentredRange(-5 * fps, 1 * fps), FxCentredRange(0 * fps, 0 * fps), FxCentredRange(11 * fps, 1 * fps), 5.0f, 1.0f));
	strongHeadNod.tracks.PushBack(FxEmphasisActionTrack(GT_EyebrowRaise,	  FxCentredRange(-4 * fps, 1 * fps), FxCentredRange(0 * fps, 0 * fps), FxCentredRange(7 * fps, 1 * fps),  0.7f, 0.8f));
	strongHeadNod.tracks.PushBack(FxEmphasisActionTrack(GT_Blink,			  FxCentredRange(-3 * fps, 1 * fps), FxCentredRange(0 * fps, 0 * fps), FxCentredRange(6 * fps, 1 * fps),  1.0f, 0.1f));
	eaProperties.PushBack(strongHeadNod);
	FxEmphasisActionAnim invertedHeadNod(EA_InvertedHeadNod, FxRange(0.75f, 1.0f));
	invertedHeadNod.tracks.PushBack(FxEmphasisActionTrack(GT_EmphasisHeadPitch,	FxCentredRange(-5 * fps, 1 * fps), FxCentredRange(0 * fps,   0 * fps), FxCentredRange(12 * fps, 1 * fps), -3.0f, 1.0f));
	invertedHeadNod.tracks.PushBack(FxEmphasisActionTrack(GT_EyebrowRaise,		FxCentredRange(-4 * fps, 1 * fps), FxCentredRange(-1 * fps, 0 * fps), FxCentredRange(6 * fps, 1 * fps),  0.7f, 1.0f));
	invertedHeadNod.tracks.PushBack(FxEmphasisActionTrack(GT_Blink,				FxCentredRange(-5 * fps, 1 * fps), FxCentredRange(-2 * fps, 0 * fps), FxCentredRange(3 * fps, 1 * fps),  1.0f, 0.1f));
	eaProperties.PushBack(invertedHeadNod);
	FxEmphasisActionAnim quickHeadNod(EA_QuickHeadNod, FxRange(0.75f, 1.0f));
	quickHeadNod.tracks.PushBack(FxEmphasisActionTrack(GT_EmphasisHeadPitch, FxCentredRange(-5 * fps, 1 * fps), FxCentredRange(0 * fps, 0 * fps), FxCentredRange(8 * fps, 1 * fps), 2.0f, 1.0f));
	quickHeadNod.tracks.PushBack(FxEmphasisActionTrack(GT_EyebrowRaise,		 FxCentredRange(-2 * fps, 1 * fps), FxCentredRange(0 * fps, 0 * fps), FxCentredRange(6 * fps, 1 * fps), 0.2f, 0.333f));
	quickHeadNod.tracks.PushBack(FxEmphasisActionTrack(GT_Blink,			 FxCentredRange(-3 * fps, 1 * fps), FxCentredRange(0 * fps, 0 * fps), FxCentredRange(6 * fps, 1 * fps),  1.0f, 0.1f));
	eaProperties.PushBack(quickHeadNod);
	FxEmphasisActionAnim normalHeadNod(EA_NormalHeadNod, FxRange(0.75f, 1.0f));
	normalHeadNod.tracks.PushBack(FxEmphasisActionTrack(GT_EmphasisHeadPitch, FxCentredRange(-5 * fps, 1 * fps), FxCentredRange(0 * fps, 0 * fps), FxCentredRange(11 * fps, 1 * fps), 3.0f, 1.0f));
	normalHeadNod.tracks.PushBack(FxEmphasisActionTrack(GT_EyebrowRaise,	  FxCentredRange(-3 * fps, 1 * fps), FxCentredRange(0 * fps, 0 * fps), FxCentredRange(7 * fps, 1 * fps),  0.3f, 1.0f));
	normalHeadNod.tracks.PushBack(FxEmphasisActionTrack(GT_Blink,			  FxCentredRange(-3 * fps, 1 * fps), FxCentredRange(0 * fps, 0 * fps), FxCentredRange(6 * fps, 1 * fps),  1.0f, 0.1f));
	eaProperties.PushBack(normalHeadNod);
	FxEmphasisActionAnim eyebrowRaise(EA_EyebrowRaise, FxRange(0.75f, 1.0f));
	eyebrowRaise.tracks.PushBack(FxEmphasisActionTrack(GT_EyebrowRaise, FxCentredRange(-3 * fps, 1 * fps), FxCentredRange(0 * fps, 0 * fps), FxCentredRange(7 * fps, 1 * fps), 0.5f, 1.0f));
	eyebrowRaise.tracks.PushBack(FxEmphasisActionTrack(GT_Blink,		FxCentredRange(-3 * fps, 1 * fps), FxCentredRange(0 * fps, 0 * fps), FxCentredRange(6 * fps, 1 * fps),  1.0f, 0.1f));
	eaProperties.PushBack(eyebrowRaise);
	FxEmphasisActionAnim headTilt(EA_HeadTilt, FxRange(0.75f, 1.0f));
	headTilt.tracks.PushBack(FxEmphasisActionTrack(GT_EmphasisHeadRoll, FxCentredRange(-4 * fps, 1 * fps), FxCentredRange(0 * fps, 0 * fps), FxCentredRange(9 * fps, 1 * fps), 2.0f, 1.0f));
	headTilt.tracks.PushBack(FxEmphasisActionTrack(GT_EyebrowRaise,		FxCentredRange(-3 * fps, 1 * fps), FxCentredRange(0 * fps, 0 * fps), FxCentredRange(7 * fps, 1 * fps), 0.2f, 1.0f));
	headTilt.tracks.PushBack(FxEmphasisActionTrack(GT_Blink,			FxCentredRange(-3 * fps, 1 * fps), FxCentredRange(0 * fps, 0 * fps), FxCentredRange(6 * fps, 1 * fps),  1.0f, 0.1f));
	eaProperties.PushBack(headTilt);
	FxEmphasisActionAnim headTurn(EA_HeadTurn, FxRange(0.75f, 1.0f));
	headTurn.tracks.PushBack(FxEmphasisActionTrack(GT_EmphasisHeadYaw,  FxCentredRange(-4 * fps, 1 * fps), FxCentredRange(0 * fps, 0 * fps), FxCentredRange(9 * fps, 1 * fps), 2.0f, 1.0f));
	headTurn.tracks.PushBack(FxEmphasisActionTrack(GT_EyebrowRaise,		FxCentredRange(-3 * fps, 1 * fps), FxCentredRange(0 * fps, 0 * fps), FxCentredRange(7 * fps, 1 * fps), 0.2f, 1.0f));
	headTurn.tracks.PushBack(FxEmphasisActionTrack(GT_Blink,			FxCentredRange(-3 * fps, 1 * fps), FxCentredRange(0 * fps, 0 * fps), FxCentredRange(6 * fps, 1 * fps),  1.0f, 0.1f));
	eaProperties.PushBack(headTurn);
	FxEmphasisActionAnim noAction(EA_NoAction, FxRange(0.75f, 1.0f));
	eaProperties.PushBack(noAction);

	// Set up the chances that a given stress will trigger a given action.
	//						Initial	Quick	Normal	Isolated	Final	
	//	Strong Head Nod     0.33	0		0.04	0.23		0.33	
	//	Inverted Head Nod   0.17	0.01    0.14  	0.2			0.1		
	//	Quick Head Nod      0.03	0.3		0.1		0.1			0.07    
	//	Normal Head Nod     0.14	0.09	0.36	0.17		0.24    
	//	Eyebrow raise       0.1		0.2		0.1		0.1			0.15    
	//	Head Tilt			0.1		0.2		0.13	0.1			0.03    
	//	Head Turn			0.12	0.2		0.13	0.1			0.07	
	//	No Action			0		0		0		0			0		
	FxArray<FxReal> initial;
	FxArray<FxReal> quick;
	FxArray<FxReal> normal;
	FxArray<FxReal> isolated;
	FxArray<FxReal> final;

	initial.PushBack(0.33f); quick.PushBack(0.00f); normal.PushBack(0.04f); isolated.PushBack(0.23f); final.PushBack(0.33f); // Strong Head Nod
	initial.PushBack(0.17f); quick.PushBack(0.01f); normal.PushBack(0.14f); isolated.PushBack(0.20f); final.PushBack(0.10f); // Inverted Head Nod
	initial.PushBack(0.03f); quick.PushBack(0.30f); normal.PushBack(0.10f); isolated.PushBack(0.10f); final.PushBack(0.07f); // Quick Head Nod
	initial.PushBack(0.13f); quick.PushBack(0.09f); normal.PushBack(0.36f); isolated.PushBack(0.17f); final.PushBack(0.24f); // Normal Head Nod
	initial.PushBack(0.10f); quick.PushBack(0.20f); normal.PushBack(0.10f); isolated.PushBack(0.10f); final.PushBack(0.15f); // Eyebrow raise
	initial.PushBack(0.10f); quick.PushBack(0.20f); normal.PushBack(0.13f); isolated.PushBack(0.10f); final.PushBack(0.03f); // Head Tilt
	initial.PushBack(0.12f); quick.PushBack(0.20f); normal.PushBack(0.13f); isolated.PushBack(0.10f); final.PushBack(0.07f); // Head Turn
	initial.PushBack(0.00f); quick.PushBack(0.00f); normal.PushBack(0.00f); isolated.PushBack(0.00f); final.PushBack(0.00f); // No Action

	eaWeights.PushBack(initial);
	eaWeights.PushBack(quick);
	eaWeights.PushBack(normal);
	eaWeights.PushBack(isolated);
	eaWeights.PushBack(final);
}

// Serialize an FxDriftProperty
FxArchive& operator<<( FxArchive& arc, FxDriftProperty& obj )
{
	FxInt32 track = static_cast<FxInt32>(obj.track);
	arc << track << obj.probability << obj.valueRange;
	obj.track = static_cast<FxGestureTrack>(track);
	return arc;
}

// Serialize an FxEmphasisActionTrack
FxArchive& operator<<( FxArchive& arc, FxEmphasisActionTrack& obj )
{
	FxInt32 track = static_cast<FxInt32>(obj.track);
	arc << track << obj.lead << obj.centre << obj.tail << obj.value << obj.probability;
	obj.track = static_cast<FxGestureTrack>(track);
	return arc;
}

// Serialize an FxEmphasisActionAnim
FxArchive& operator<<( FxArchive& arc, FxEmphasisActionAnim& obj )
{
	FxInt32 action = static_cast<FxInt32>(obj.action);
	arc << action << obj.tracks << obj.valueScale;
	obj.action = static_cast<FxEmphasisAction>(action);
	return arc;
}

// Serialize the configuration
void FxGestureConfig::Serialize( FxArchive& arc )
{
	Super::Serialize(arc);

	FxUInt16 version = arc.SerializeClassVersion("FxGestureConfig");
	
	arc
		// Power and frequency extraction.
		<< pfeAllowedFrequency
		<< pfeAvgPitchUpperNumSteps
		<< pfeAvgPitchLowerNumSteps
		<< pfePowerNumAdjacentPhones
		<< pfePowerMaxWindowLength
		<< pfeAvgPowerContribution
		<< pfeMaxPowerContribution
		<< pfeHighPitchBonus
		<< pfeLongDurationBonus
		<< pfeRateOfSpeechNumAdjacentPhones

		// Stress detection.
		<< sdStressThreshold
		<< sdLongSilence
		<< sdShortSilence
		<< sdQuickStressLimit
		<< sdIsolatedStressLimit

		// Rate of speech.
		<< rosAvgPhonemeDuration
		<< rosTimeScale

		// Orientation drift.
		<< odHeadDuration
		<< odHeadTransition
		<< odEyeDuration
		<< odEyeTransition
		<< odHeadProperties
		<< odEyeProperties

		// Emphasis actions.
		<< eaProperties
		<< eaWeights
		<< eaCentreShift

		// Blinks
		<< blinkSeparation
		<< blinkLead
		<< blinkTail

		// Broad gesture controls
		<< gcSpeed
		<< gcMagnitude;

	if( (arc.IsLoading() && version >= 2) || arc.IsSaving() )
	{
		arc << description;
	}
	else
	{
		description = FxString("");
	}
}

FxBool FX_CALL FxLoadGestureConfigFromFile( FxGestureConfig& config, const FxChar* filename, const FxBool bUseFastMethod, void(FX_CALL *callbackFunction)(FxReal) /*= NULL*/, FxReal updateFrequency /*= 0.01f */ )
{
	FxArchiveStore* pStore = bUseFastMethod ? FxArchiveStoreFileFast::Create(filename)
		: FxArchiveStoreFile::Create(filename);
	FxArchive configArchive(pStore, FxArchive::AM_LinearLoad);
	configArchive.Open();
	if( configArchive.IsValid() )
	{
		configArchive.RegisterProgressCallback(callbackFunction, updateFrequency);
		configArchive << config;
		return FxTrue;
	}
	return FxFalse;
}

FxBool FX_CALL FxSaveGestureConfigToFile( FxGestureConfig& config, const FxChar* filename, FxArchive::FxArchiveByteOrder byteOrder /*= FxArchive::ABO_LittleEndian*/, void(FX_CALL *callbackFunction)(FxReal) /*= NULL*/, FxReal updateFrequency /*= 0.01f */ )
{
#ifndef NO_SAVE_VERSION
	FxArchive directoryCreater(FxArchiveStoreNull::Create(), FxArchive::AM_CreateDirectory);
	directoryCreater.Open();
	directoryCreater << config;
	FxArchive configArchive(FxArchiveStoreFile::Create(filename), FxArchive::AM_Save, byteOrder);
	configArchive.Open();
	if( configArchive.IsValid() )
	{
		configArchive.SetInternalDataState(directoryCreater.GetInternalData());
		configArchive.RegisterProgressCallback(callbackFunction, updateFrequency);
		configArchive << config;
		return FxTrue;
	}
#else
	config; filename; byteOrder; callbackFunction; updateFrequency;
#endif
	return FxFalse;
}

FxBool FX_CALL FxLoadGestureConfigFromMemory( FxGestureConfig& config, const FxByte* pMemory, const FxSize numBytes, void(FX_CALL *callbackFunction)(FxReal) /*= NULL*/, FxReal updateFrequency /*= 0.01f */ )
{
	if( pMemory )
	{
		FxArchive configArchive(FxArchiveStoreMemoryNoCopy::Create(pMemory, numBytes), FxArchive::AM_LinearLoad);
		configArchive.Open();
		if( configArchive.IsValid() )
		{
			configArchive.RegisterProgressCallback(callbackFunction, updateFrequency);
			configArchive << config;
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FX_CALL FxSaveGestureConfigToMemory( FxGestureConfig& config, FxByte*& pMemory, FxSize& numBytes, FxArchive::FxArchiveByteOrder byteOrder /*= FxArchive::ABO_LittleEndian*/, void(FX_CALL *callbackFunction)(FxReal) /*= NULL*/, FxReal updateFrequency /*= 0.01f */ )
{
#ifndef NO_SAVE_VERSION
	FxAssert(pMemory == NULL);
	if( !pMemory )
	{
		FxArchive directoryCreater(FxArchiveStoreNull::Create(), FxArchive::AM_CreateDirectory);
		directoryCreater.Open();
		directoryCreater << config;
		FxArchiveStoreMemory* pStore = FxArchiveStoreMemory::Create(NULL, 0);
		FxArchive configArchive(pStore, FxArchive::AM_Save, byteOrder);
		configArchive.Open();
		if( configArchive.IsValid() )
		{
			configArchive.SetInternalDataState(directoryCreater.GetInternalData());
			configArchive.RegisterProgressCallback(callbackFunction, updateFrequency);
			configArchive << config;
			numBytes = pStore->GetSize();
			pMemory = static_cast<FxByte*>(FxAlloc(numBytes, "FxSaveGestureConfigToMemory"));
			FxMemcpy(pMemory, pStore->GetMemory(), numBytes);
			return FxTrue;
		}
	}
#else
	config; pMemory; numBytes; byteOrder; callbackFunction; updateFrequency;
#endif
	return FxFalse;
}


} // namespace Face

} // namespace OC3Ent
