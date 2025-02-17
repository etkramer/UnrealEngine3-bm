//------------------------------------------------------------------------------
// The generator for emphasis-based head movements, eyebrow raises and blinks.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxEmphasisActionGenerator_H__
#define FxEmphasisActionGenerator_H__

#include "FxPlatform.h"
#include "FxRandomGenerator.h"
#include "FxGestureConfig.h"
#include "FxArray.h"
#include "FxAnimCurve.h"

#include <time.h>

namespace OC3Ent
{

namespace Face
{

struct FxAnimSegment;

class FxEmphasisActionGenerator
{
public:
	FxEmphasisActionGenerator();
	~FxEmphasisActionGenerator();

	// Initializes the emphasis action generator.
	void Initialize( FxInt32 seed = time(0) );
	// Clears the results from the emphasis action generator.
	void Clear();
	// Processes a list of stress information.
	void Process( const FxArray<FxStressInformation>& stressInfos, const FxGestureConfig& config );

	// Returns a track
	FxAnimCurve& GetTrack( FxGestureTrack track );
	// Returns the blink times.
	FxArray<FxReal>& GetBlinkTimes() { return _blinkTimes; }

protected:
	// Performs key reduction on all the FxAnimSegments.
	void ReduceKeys();

	// Returns a segment array
	FxArray<FxAnimSegment>& GetSegmentArray( FxGestureTrack track );
	// Evaluates an anim segment
	FxReal EvaluateSegment( const FxAnimSegment& segment, FxReal time );
	// Evaluates an anim curve
	FxReal EvaluateCurve( const FxAnimKey& first, const FxAnimKey& second, FxReal time );

	FxRandomGenerator _random;

	FxArray<FxAnimSegment> _headPitches;
	FxArray<FxAnimSegment> _headRolls;
	FxArray<FxAnimSegment> _headYaws;
	FxArray<FxAnimSegment> _eyebrows;

	FxAnimCurve _eaHeadPitch;
	FxAnimCurve _eaHeadRoll;
	FxAnimCurve _eaHeadYaw;
	FxAnimCurve _eyebrowRaise;
	
	FxArray<FxReal> _blinkTimes;

	FxReal		_minTime;
	FxReal		_maxTime;
};

// A segment of animation.
struct FxAnimSegment
{
	FxAnimSegment(FxReal l = 0.0f, FxReal c = 0.0f, FxReal t = 0.0f, FxReal v = 0.0f)
	{
		lead = FxAnimKey(l, 0.0f);
		centre = FxAnimKey(c, v);
		tail = FxAnimKey(t, 0.0f);
	}
	FxAnimKey lead;
	FxAnimKey centre;
	FxAnimKey tail;
};

} // namespace Face

} // namespace OC3Ent

#endif
