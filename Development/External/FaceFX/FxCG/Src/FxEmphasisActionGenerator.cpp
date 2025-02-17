//------------------------------------------------------------------------------
// The generator for emphasis-based head movements, eyebrow raises and blinks.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxEmphasisActionGenerator.h"
#include "FxGestureConfig.h"
#include "FxMath.h"

namespace OC3Ent
{

namespace Face
{

FxEmphasisActionGenerator::FxEmphasisActionGenerator()
{
	_eaHeadPitch.SetName(FxCGGetSpeechGestureTrackName(GT_EmphasisHeadPitch));
	_eaHeadRoll.SetName(FxCGGetSpeechGestureTrackName(GT_EmphasisHeadRoll));
	_eaHeadYaw.SetName(FxCGGetSpeechGestureTrackName(GT_EmphasisHeadYaw));
	_eyebrowRaise.SetName(FxCGGetSpeechGestureTrackName(GT_EyebrowRaise));
}

FxEmphasisActionGenerator::~FxEmphasisActionGenerator()
{
}

// Initializes the emphasis action generator.
void FxEmphasisActionGenerator::Initialize( FxInt32 seed )
{
	Clear();
	_random.ResetSeed(seed);
}

// Clears the results from the emphasis action generator.
void FxEmphasisActionGenerator::Clear()
{
	_eaHeadPitch.RemoveAllKeys();
	_eaHeadRoll.RemoveAllKeys();
	_eaHeadYaw.RemoveAllKeys();
	_eyebrowRaise.RemoveAllKeys();
	
	_blinkTimes.Clear();

	_headRolls.Clear();
	_headPitches.Clear();
	_headYaws.Clear();
	_eyebrows.Clear();

	_minTime = FX_REAL_MAX;
	_maxTime = FX_REAL_MIN;
}

// Processes a list of stress information.
void FxEmphasisActionGenerator::Process( const FxArray<FxStressInformation>& stressInfos, const FxGestureConfig& config )
{
	FxAnimKey key1(0.07f, 0.0f);
	FxAnimKey key2(0.50f, 1.0f);
	FxSize numStressInfos = stressInfos.Length();
	for( FxSize i = 0; i < numStressInfos; ++i )
	{
		// Select a random emphasis action according to the type of stress.
		FxSize emphasisActionIndex = _random.Index(config.eaWeights[stressInfos[i].stressCategory]);

		// For each track in the emphasis action animation
		for( FxSize j = 0; j < config.eaProperties[emphasisActionIndex].tracks.Length(); ++j )
		{
			// Randomize whether or not to include this track.
			if( _random.Bool(config.eaProperties[emphasisActionIndex].tracks[j].probability) )
			{
				FxGestureTrack track = config.eaProperties[emphasisActionIndex].tracks[j].track;
				// Slightly randomize the times of the keys.
				FxReal centre = _random.RealInCentredRange(config.eaProperties[emphasisActionIndex].tracks[j].centre.centre, config.eaProperties[emphasisActionIndex].tracks[j].centre.range);
				FxReal lead   = _random.RealInCentredRange(config.eaProperties[emphasisActionIndex].tracks[j].lead.centre, config.eaProperties[emphasisActionIndex].tracks[j].lead.range);
				FxReal tail   = _random.RealInCentredRange(config.eaProperties[emphasisActionIndex].tracks[j].tail.centre, config.eaProperties[emphasisActionIndex].tracks[j].tail.range);
				FxReal value  = config.eaProperties[emphasisActionIndex].tracks[j].value;
				// Blinks are immune from time scaling and value randomization.
				if( track != GT_Blink )
				{
					// Scale the time based on the local rate of speech.
					lead   *= stressInfos[i].localTimeScale * config.gcSpeed;
					tail   *= stressInfos[i].localTimeScale * config.gcSpeed;
					// Scale the centre value by a per-action random value.
					value  *= _random.RealInRange(config.eaProperties[emphasisActionIndex].valueScale.rangeMin, config.eaProperties[emphasisActionIndex].valueScale.rangeMax) * config.gcMagnitude;
				}
				else if( (track == GT_EmphasisHeadRoll || track == GT_EmphasisHeadYaw) && _random.Bool() )
				{
					// Head roll and yaw can flip directions.  Pitch must not.
					value *= -1;
				}

				if( track == GT_EmphasisHeadRoll || track == GT_EmphasisHeadYaw || track == GT_EmphasisHeadPitch )
				{
					// Constrain the amount of movement possible.
					if( i > 0 )
					{
						value *= EvaluateCurve(key1, key2, stressInfos[i].startTime - stressInfos[i-1].startTime);
					}
				}

				// Shift the key times.
				lead   = centre + lead + stressInfos[i].startTime + config.eaCentreShift;
				tail   = centre + tail + stressInfos[i].startTime + config.eaCentreShift;
				centre = centre + stressInfos[i].startTime + config.eaCentreShift;

				// Place the keys in the track.
				if( track == GT_Blink )
				{
					_blinkTimes.PushBack(centre);
				}
				else
				{
					GetSegmentArray(config.eaProperties[emphasisActionIndex].tracks[j].track).PushBack(FxAnimSegment(lead, centre, tail, value));
				}

				// Cache these for key reduction.
				_minTime = FxMin(lead, _minTime);
				_maxTime = FxMax(tail, _maxTime);
			}
		}
	}

	ReduceKeys();
}

// Returns a track
FxAnimCurve& FxEmphasisActionGenerator::GetTrack( FxGestureTrack track )
{
	switch( track )
	{
	default:
		FxAssert(!"Unknown track in FxEmphasisActionGenerator::GetTrack!");
	case GT_EmphasisHeadRoll:
		return _eaHeadRoll;
	case GT_EmphasisHeadPitch:
		return _eaHeadPitch;
	case GT_EmphasisHeadYaw:
		return _eaHeadYaw;
	case GT_EyebrowRaise:
		return _eyebrowRaise;
	}
}

// Performs key reduction on all the FxAnimSegments.
void FxEmphasisActionGenerator::ReduceKeys()
{
	for( FxSize i = GT_EmphasisHeadPitch; i <= GT_EyebrowRaise; ++i )
	{
		FxGestureTrack currTrack = static_cast<FxGestureTrack>(i);
		FxArray<FxAnimSegment>& segments = GetSegmentArray(currTrack);
		FxAnimCurve& track = GetTrack(currTrack);

		const FxReal timeStep = 0.01666666667f; // Reduce keys at 60fps.
		// Pad start and end to not miss first and last keys.
		FxReal endTime = _maxTime + (2 * timeStep);
		FxReal currTime = _minTime - (2 * timeStep);
		FxReal lastValue = 0.0f;
		FxReal lastDv = 0.0f;
		while( currTime <= endTime )
		{
			FxReal value = 0.0f;
			for( FxSize i = 0; i < segments.Length(); ++i )
			{
				value += EvaluateSegment(segments[i], currTime);
			}
			FxReal dv = value - lastValue;

			// Insert a key if we had a zero derivative and now we don't
			if( (FxRealEquality(lastDv, 0.0f) && !FxRealEquality(dv, 0.0f)) ||
				// Or, insert a key if we had a non-zero derivative and now we have a zero derivative
				(!FxRealEquality(lastDv, 0.0f) && FxRealEquality(dv, 0.0f)) ||
				// Or, insert a key if we had a positive derivative and now we have negative
				(lastDv > 0.0f && dv < 0.0f) || 
				// Or, insert a key if we had a negative derivative and now we have positive
				(lastDv < 0.0f && dv > 0.0f) )
			{
				track.InsertKey(FxAnimKey(currTime, lastValue));
			}

			lastValue = value;
			lastDv = dv;

			currTime += timeStep;
		}
	}
}

// Returns a segment array
FxArray<FxAnimSegment>& FxEmphasisActionGenerator::GetSegmentArray( FxGestureTrack track )
{
	switch( track )
	{
	default:
		FxAssert(!"Unknown track in FxEmphasisActionGenerator::GetTrack!");
	case GT_EmphasisHeadRoll:
		return _headRolls;
	case GT_EmphasisHeadPitch:
		return _headPitches;
	case GT_EmphasisHeadYaw:
		return _headYaws;
	case GT_EyebrowRaise:
		return _eyebrows;
	}
}

FxReal FxEmphasisActionGenerator::EvaluateSegment( const FxAnimSegment& segment, 
												   FxReal time )
{
	FxReal value = 0.0f;

	// Check for out-of-range time and clamp to end points of curve.
	if( time < segment.lead.GetTime() )
	{
		value = segment.lead.GetValue();
	}
	else if( time > segment.tail.GetTime() )
	{
		value = segment.tail.GetValue();
	}
	else
	{
		if( time < segment.centre.GetTime() )
		{
			value = EvaluateCurve(segment.lead, segment.centre, time);
		}
		else
		{
			value = EvaluateCurve(segment.centre, segment.tail, time);
		}
	}

	return value;
}

// Performs Hermite evaluation on two keys
FxReal FxEmphasisActionGenerator::EvaluateCurve( const FxAnimKey& first, const FxAnimKey& second, FxReal time )
{
	if( time < first.GetTime() )
	{
		return first.GetValue();
	}
	else if( time > second.GetTime() )
	{
		return second.GetValue();
	}
	else
	{
		return FxHermiteInterpolate(first, second, time);
	}
}

} // namespace Face

} // namespace OC3Ent
