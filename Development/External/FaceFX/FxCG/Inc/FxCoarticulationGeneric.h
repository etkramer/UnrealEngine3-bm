//------------------------------------------------------------------------------
// Provides generic phoneme-to-target style coarticulation.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCoarticulationGeneric_H__
#define FxCoarticulationGeneric_H__

#include "FxPlatform.h"
#include "FxPhonemeEnum.h"
#include "FxAnimCurve.h"
#include "FxCoarticulationConfig.h"
#include "FxFaceGraph.h"
#include "FxCompiledFaceGraph.h"

namespace OC3Ent
{

namespace Face
{

class FxCoarticulationGeneric
{
public:
	// Constructor.
	FxCoarticulationGeneric();
	// Destructor.
	~FxCoarticulationGeneric();

	// Set the coarticulation configuration.
	void SetConfig( const FxCoarticulationConfig& config );

	// Begin the phoneme list.
	void BeginPhonemeList(FxSize numPhones);
	// Append a phoneme.  Must be called in order of increasing start times.
	void AppendPhoneme(FxPhoneme phonEnum, FxReal phonStart, FxReal phonEnd);
	// End the phoneme list.
	void EndPhonemeList();

	// Begin phoneme to track mapping input.
	void BeginMapping();
	// Add a mapping entry to the phoneme to track mapping.
	FxBool AddMappingEntry(FxPhoneme phonEnum, FxName track, FxReal amount);
	// End phoneme to track mapping input.
	void EndMapping();

	// Clear the phoneme list.
	void ClearPhonemeList();
	// Clear the phoneme to track mapping.
	void ClearMapping();

	// Does the coarticulation calculations and phoneme to track mapping.
	void Process();

	// Gets the number of curves that were generated.
	FxSize GetNumCurves();
	// Returns a curve that was generated.
	const FxAnimCurve& GetCurve( FxSize index );

protected:
	// Returns the name of a phoneme node.
	FxName GetPhonemeNodeName(FxPhoneme phon);

	// Do the coarticulation.
	void DoCoarticulation();
	// Do the phoneme to track mapping.
	void DoMapping();
	// Calculates the suppression of a phoneme pair.
	FxReal CalculateSuppression( FxPhoneme currPhoneme, FxPhoneme otherPhoneme, FxReal phonemeDuration, FxReal cachedSuppression );
	// Calculates the duration of a phoneme.
	FX_INLINE FxReal GetPhonemeDuration(FxSize i);
	// Calculates the end time of a phoneme.
	FX_INLINE FxReal GetPhonemeEndTime(FxSize i);
	// Performs Hermite evaluation on three keys
	FX_INLINE FxReal EvaluateCurve( const FxAnimKey& leadIn, const FxAnimKey& peak, 
		const FxAnimKey& leadOut, FxReal time );

private:
	struct FxCGPhonemeData
	{
		FxCGPhonemeData( FxPhoneme phon = PHON_SIL, FxReal start = 0.0f )
			: phoneme(phon)
			, startTime(start)
			, suppression(FxInvalidValue)
			, leadIn(FxAnimKey(0.0f, 0.0f))
			, peak(FxAnimKey(0.0f, 1.0f))
			, leadOut(FxAnimKey(0.0f, 0.0f)) {}
		
		FxPhoneme phoneme;
		FxReal startTime;
		FxReal suppression;
		FxAnimKey leadIn;
		FxAnimKey peak;
		FxAnimKey leadOut;
	};

	// The coarticulation config.
	FxCoarticulationConfig _config;
	// The phoneme data
	FxArray<FxCGPhonemeData> _phonData;
	// The end time of the last phoneme.
	FxReal _lastPhonEndTime;

	//@todo compiled
	FxCompiledFaceGraph _cg;
	FxArray<FxRegister> _registers;
	FxBool _hasBeenTicked;

	// The face graph used to process the phoneme to track mapping.
	FxFaceGraph _mappingGraph;
	// The phoneme nodes.
	FxArray<FxSize> _phonNodeMap;
	// The delta nodes.
	FxArray<FxSize> _deltaNodeMap;
	// The target nodes.
	FxArray<FxSize> _targetNodeMap;
	// The track names.
	FxArray<FxName> _trackNames;

	// The last derivatives of the targets
	FxArray<FxReal> _lastDerivatives;
	// The last values of the targets
	FxArray<FxReal> _lastValues;

	// The start time of the animation.
	FxReal _startTime;
	// The end time of the animation.
	FxReal _endTime;

	// The curves that were generated.
	FxArray<FxAnimCurve> _curves;
};

} // namespace Face

} // namespace OC3Ent

#endif
