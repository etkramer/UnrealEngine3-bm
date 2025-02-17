//------------------------------------------------------------------------------
// The manager of all the subcomponents for generating speech gestures.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxSpeechGestures.h"
#include "FxPhonemeEnum.h"
#include "FxSimplePhonemeList.h"
#include "FxList.h"

namespace OC3Ent
{

namespace Face
{

FxSpeechGestures::FxSpeechGestures()
{
	_blinks.SetName(FxCGGetSpeechGestureTrackName(GT_Blink));
}

FxSpeechGestures::~FxSpeechGestures()
{
}

// Initializes the speech gesture system.
void FxSpeechGestures::Initialize( const FxDReal* pSamples, FxInt32 sampleCount, FxInt32 sampleRate )
{
	if( pSamples )
	{
		_stressDetector.Synch(NULL, _config);
		_stressDetector.Initialize(pSamples, sampleCount, sampleRate);
		_stressDetector.Process();
	}
}

// Aligns the speech gestures to the phoneme list.
void FxSpeechGestures::Align( FxPhonemeList* pPhonList, FxInt32 seed )
{
	if( pPhonList )
	{
		// Clear the blinks.
		_blinks.RemoveAllKeys();

		// Synch the stress information with the phoneme list.
		_stressDetector.Synch(pPhonList, _config);

		// Pull out a list of the stressed syllables.
		FxArray<FxStressInformation> stressInfos;
		for( FxSize i = 0; i < _stressDetector.GetNumStressInfos(); ++i )
		{
			if( _stressDetector.GetStressInfo(i).normalizedAvgPower >= _config.sdStressThreshold )
			{
				stressInfos.PushBack(_stressDetector.GetStressInfo(i));
			}
		}

		if( !seed )
		{
			_emphasisActionGenerator.Initialize();
		}
		else
		{
			_emphasisActionGenerator.Initialize(seed);
		}

		if( stressInfos.Length() )
		{
//			// Remove hyper-repetitive stresses.
//			for( FxInt32 i = 0; i < static_cast<FxInt32>(stressInfos.Length() - 1); ++i )
//			{
//				if( stressInfos[i+1].startTime - stressInfos[i].endTime < _config.sdMinStressSeparation )
//				{
//					if( stressInfos[i+1].normalizedAvgPower < stressInfos[i].normalizedAvgPower )
//					{
//						stressInfos.Remove(i+1);
//					}
//					else
//					{
//						stressInfos.Remove(i);
//					}
//					i = 0;
//				}
//			}

			// Generate the emphasis-based head positions, eyebrow raises and blink times.
			_emphasisActionGenerator.Process(stressInfos, _config);
		}

		// Generate the random head orientations and gaze positions.
		if( !seed )
		{
			_gazeGenerator.Initialize(pPhonList);
		}
		else
		{
			_gazeGenerator.Initialize(pPhonList, seed);
		}
		_gazeGenerator.Process(_config);

		MakeBlinkTrack(_emphasisActionGenerator.GetBlinkTimes(), pPhonList);
	}
}

// Returns a generated animation track. (testing only?)
const FxAnimCurve& FxSpeechGestures::GetTrack( FxGestureTrack track )
{
	if( (GT_OrientationFirst <= track && track <= GT_OrientationLast) ||
		(GT_GazeFirst <= track && track <= GT_GazeLast) )
	{
		return _gazeGenerator.GetTrack(track);
	}
	else if( GT_EmphasisFirst <= track && track <= GT_EmphasisLast )
	{
		if( track != GT_Blink )
		{
			return _emphasisActionGenerator.GetTrack(track);
		}
		else
		{
			return _blinks;
		}
	}
	else
	{
		FxAssert(!"Invalid track in FxSpeechGestures::GetTrack()");
		// Pass back the blinks track in this case just so every control path
		// will return a value.  Obviously, this shouldn't be used, as it 
		// probably wasn't what the user was expecting.
		return _blinks;
	}
}

// Makes the blink track
void FxSpeechGestures::MakeBlinkTrack( FxArray<FxReal>& blinkTimes, FxPhonemeList* pPhonList )
{
	FxRandomGenerator random(static_cast<FxInt32>(time(0)));
	FxReal maxLeft  = _config.blinkLead.centre - _config.blinkLead.range;
	FxReal maxRight = _config.blinkTail.centre + _config.blinkTail.range;
	const FxReal blinkBuffer = 1.2f;

	FxList<FxReal> blinkList;
	for( FxSize i = 0; i < blinkTimes.Length(); ++i )
	{
		blinkList.PushBack(blinkTimes[i]);
	}

	// Make sure there's a blink within 750ms of the "end".
	FxReal endTime = pPhonList->GetPhonemeEndTime(pPhonList->GetNumPhonemes()-1);
	if( blinkList.Back() < endTime - 0.75f )
	{
		// Add a random blink somewhere in the last half second of the utterance.
		blinkList.PushBack( random.RealInRange(endTime - 0.75f + maxRight, endTime - 0.25f - maxRight) );
	}

	// Enforce a maximum separation between blinks
	FxBool needMoreBlinks = FxFalse;
	do 
	{
		FxReal prevBlinkTime = 0.0f;
		FxList<FxReal>::Iterator i = blinkList.Begin();
		for( ; i != blinkList.End(); ++i )
		{
			if( *i - prevBlinkTime > _config.blinkSeparation.rangeMax )
			{
				i = blinkList.Insert(random.RealInRange(prevBlinkTime + maxLeft + blinkBuffer, *i - maxRight - blinkBuffer), i);
			}
			prevBlinkTime = *i;
		}
		prevBlinkTime = 0.0f;
		needMoreBlinks = FxFalse;
		for( i = blinkList.Begin(); i != blinkList.End(); ++i )
		{
			if( *i - prevBlinkTime > _config.blinkSeparation.rangeMax )
			{
				needMoreBlinks = FxTrue;
			}
			prevBlinkTime = *i;
		}
	} while( needMoreBlinks );

	// Enforce a minimum separation between blinks.
	FxList<FxReal>::Iterator curr = ++blinkList.Begin();
	FxReal prevBlinkTime = *(blinkList.Begin());
	for( ; curr != blinkList.End(); ++curr )
	{
		if( prevBlinkTime + maxRight > *curr + maxLeft - _config.blinkSeparation.rangeMin )
		{
			FxList<FxReal>::Iterator temp = curr;
			--curr;
			blinkList.Remove(temp);
		}
		prevBlinkTime = *curr;
	}

	// Populate the blink track.
	for( curr = blinkList.Begin(); curr != blinkList.End(); ++curr )
	{
		FxReal lead   = random.RealInCentredRange(_config.blinkLead.centre, _config.blinkLead.range) + *curr;
		FxReal centre = *curr;
		FxReal tail   = random.RealInCentredRange(_config.blinkTail.centre, _config.blinkTail.range) + *curr;
		_blinks.InsertKey(FxAnimKey(lead, 0.0f));
		_blinks.InsertKey(FxAnimKey(centre, 1.0f));
		_blinks.InsertKey(FxAnimKey(tail, 0.0f));
	}
}

} // namespace Face

} // namespace OC3Ent
