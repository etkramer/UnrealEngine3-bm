//------------------------------------------------------------------------------
// A class for detecting stressed syllables in spoken speech using power,
// fundamental frequency, and the phonetic transcript.
//
// The fundamental frequency detection algorithm is adapted from 
// "YIN, a fundamental frequency estimator for speech and music" 
// by Alain de Cheveigné.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxStressDetector_H__
#define FxStressDetector_H__

#include "FxPlatform.h"
#include "FxArray.h"
#include "FxGestureConfig.h"

namespace OC3Ent
{

namespace Face
{

// Forward declare the phoneme/word list.
class FxPhonemeList;

class FxStressDetector
{
public:
	// Constructor.
	// windowLength is the length of the integration window in seconds. The 
	// default value is 25ms, corresponding to a minimum frequency resolution
	// of 40Hz.  Since the deepest male voice is around 50Hz and the algorithm
	// has no upper limit on the search space, this should be able to detect
	// the fundamental frequency of any human vocalized sound.  windowStep
	// is the length of time between window samplings, default set for 50% overlap.
	FxStressDetector( const FxDReal* pSamples = NULL, FxInt32 sampleCount = 0, FxInt32 sampleRate = 0, FxReal windowLength = 0.025f, FxReal windowStep = 0.010f );
	// Destructor.
	~FxStressDetector();

	// Sets the audio pointer, window length and window step.
	void Initialize( const FxDReal* pSamples, FxInt32 sampleCount, FxInt32 sampleRate, FxReal windowLength = 0.025f, FxReal windowStep = 0.010f );

	// Processes the audio.  Must be called before any following function.
	void Process();
	// Synchs the results to the phoneme list
	void Synch( FxPhonemeList* pPhonList, const FxGestureConfig& config );

	// Gets the fundamental frequency for a time.
	FxReal GetFundamentalFrequency( FxReal time );
	// Gets the fundamental frequency for a sample.
	FxReal GetFundamentalFrequency( FxInt32 sample );

	// Gets whether the time is voiced.
	FxBool IsVoiced( FxReal time );
	// Gets whether the sample is voiced.
	FxBool IsVoiced( FxInt32 sample );
	// Gets whether the time is a vowel.
	FxBool IsVowel( FxReal time );
	// Gets whether the sample is a vowel.
	FxBool IsVowel( FxInt32 sample );

	// Gets the power of a time.
	FxReal GetPower( FxReal time );
	// Gets the power of a sample.
	FxReal GetPower( FxInt32 sample );
	// Gets the minimum power.
	FxReal GetMinPower();
	// Gets the maximum power.
	FxReal GetMaxPower();

#ifdef FX_DETECT_FREQUENCY
	// Gets the average frequency for the utterance.
	FxReal GetAvgFrequency() { return _frequencyAverage; }
#else
	FxReal GetAvgFrequency() { return 0.0f; }
#endif

	// Gets the stress information for a vowel.
	const FxStressInformation& GetStressInfo( FxSize vowelIndex ) const;
	// Gets the number of stress informations.
	FxSize GetNumStressInfos() const;

protected:

#ifdef FX_DETECT_FREQUENCY
	// Processes a single window of audio for the difference function.
	void GetDifferenceFunctionAndComputePower( FxSize startSample, FxReal* differenceFunc, const FxArray<FxReal>& windowingFunc );
	// Processes a difference function into the cumulative mean normalized difference function.
	void GetCumulativeMeanNormalizedDifferenceFunction(const FxReal* differenceFunc, FxReal* cmndFunc );
	// Extracts the fundamental frequency from the cumulative mean normalized difference function.
	void ExtractFundamentalFrequency( const FxReal* cmndFunc );
#else
	void ComputePower( FxSize startSample, const FxArray<FxReal>& windowingFunc );
#endif
	// Creates a hamming window of as long as the window.
	void CreateHammingWindow( FxArray<FxReal>& window );

	// Input
	//FxAudio* _pAudio; // A pointer to the audio to process.
	FxDReal*		_pSamples;
	FxInt32			_sampleCount;
	FxInt32			_sampleRate;
	FxPhonemeList*  _pPhonList; // A pointer to the phoneme list.

	// Parameters
	FxSize			_windowSampleCount; // The number of samples in a window.
	FxSize			_windowStep; // The number of samples from one window to another.
	FxReal			_cmndThreshold; // The difference threshold for extracting the frequency.
	FxReal			_certaintyThreshold; // The certainty threshold for calling a segment "voiced".

	// Data
#ifdef FX_DETECT_FREQUENCY
	FxArray<FxReal> _fundamentalFrequency; // The fundamental frequency for each window.
	FxArray<FxReal> _certainty; // The certainty of the fundamental frequency decision.
	FxReal			_frequencyAverage; // The average pitch for the utterance.
#endif
	FxArray<FxReal> _power; // The power for each window.
	FxReal			_minPower; // The minimum power seen.
	FxReal			_maxPower; // The maximum power seen.

	// Gesture data
	FxArray<FxStressInformation> _stressInfos; // The stress information for each vowel.
};

} // namespace Face

} // namespace OC3Ent

#endif
