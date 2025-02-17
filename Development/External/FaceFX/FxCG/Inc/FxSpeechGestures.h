//------------------------------------------------------------------------------
// The manager of all the subcomponents for generating speech gestures.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxSpeechGestures_H__
#define FxSpeechGestures_H__

#include "FxPlatform.h"
#include "FxGestureConfig.h"
#include "FxGazeGenerator.h"
#include "FxStressDetector.h"
#include "FxEmphasisActionGenerator.h"
#include "FxAnimCurve.h"

namespace OC3Ent
{

namespace Face
{

class FxSpeechGestures
{
public:
	FxSpeechGestures();
	~FxSpeechGestures();

	// Initializes the speech gesture system.
	void Initialize( const FxDReal* pSamples, FxInt32 sampleCount, FxInt32 sampleRate );
	// Aligns the speech gestures to the phoneme list.
	void Align( FxPhonemeList* pPhonList, FxInt32 randomSeed = 0 );
	
	// Returns a generated animation track.
	const FxAnimCurve& GetTrack( FxGestureTrack track );

	// Returns the gesture generation configuration.
	FxGestureConfig GetConfig() { return _config; }
	// Sets the gesture generation configuration.
	void SetConfig( const FxGestureConfig& config ) { _config = config; }

	static FxSize	GetNumSpeechGestureTracks();
	static FxString	GetSpeechGestureTrackName(FxSize trackIndex);

protected:
	void MakeBlinkTrack( FxArray<FxReal>& blinkTimes, FxPhonemeList* pPhonList );

	FxGestureConfig				_config;

	FxStressDetector			_stressDetector;
	FxGazeGenerator				_gazeGenerator;
	FxEmphasisActionGenerator	_emphasisActionGenerator;

	FxAnimCurve					_blinks;
};

} // namespace Face

} // namespace OC3Ent

#endif
