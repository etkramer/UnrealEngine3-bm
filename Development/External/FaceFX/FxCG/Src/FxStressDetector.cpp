//------------------------------------------------------------------------------
// A class for detecting stressed syllables in spoken speech using power,
// fundamental frequency, and the phonetic transcript.
//
// The fundamental frequency detection algorithm is adapted from 
// "YIN, a fundamental frequency estimator for speech and music" 
// by Alain de Cheveigne.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxStressDetector.h"
#include "FxPhonemeEnum.h"
#include "FxSimplePhonemeList.h"
#include "FxMath.h"

namespace OC3Ent
{

namespace Face
{

FxStressDetector::FxStressDetector( const FxDReal* pSamples, FxInt32 sampleCount, FxInt32 sampleRate, FxReal windowLength, FxReal windowStep )
	: _pSamples(NULL)
	, _sampleCount(0)
	, _sampleRate(0)
	, _pPhonList(0)
	, _cmndThreshold(0.10f)
	, _certaintyThreshold(0.85f)
{
	Initialize(pSamples, sampleCount, sampleRate, windowLength, windowStep);
}


FxStressDetector::~FxStressDetector()
{
	if( _pSamples )
	{
		delete[] _pSamples;
		_pSamples = NULL;
	}
}

void FxStressDetector::Initialize( const FxDReal* pSamples, FxInt32 sampleCount, FxInt32 sampleRate, FxReal windowLength, FxReal windowStep )
{
	if( _pSamples )
	{
		delete[] _pSamples;
	}
	if( pSamples && sampleCount )
	{
		_pSamples = new FxDReal[sampleCount];
		FxMemcpy(_pSamples, pSamples, sampleCount * sizeof(FxDReal));
		_sampleCount = sampleCount;
		_sampleRate = sampleRate;
		_windowSampleCount = static_cast<FxSize>(_sampleRate * windowLength);
		_windowStep = static_cast<FxSize>(_sampleRate * windowStep);
	}
}

void FxStressDetector::Process()
{
	if( _pSamples )
	{
#ifdef FX_DETECT_FREQUENCY
		FxReal* differenceFunc = new FxReal[_windowSampleCount];
		FxReal* cmndFunc = new FxReal[_windowSampleCount];

		_fundamentalFrequency.Clear();
		_fundamentalFrequency.Reserve(_sampleCount/_windowStep);
		_certainty.Clear();
		_certainty.Reserve(_sampleCount/_windowStep);
#endif
		_power.Clear();
		_power.Reserve(_sampleCount/_windowStep);
		_minPower = FX_REAL_MAX;
		_maxPower = FX_REAL_MIN;

		// Create the window.
		FxArray<FxReal> window;
		CreateHammingWindow(window);

		// For each window, extract the fundamental frequency and its power.
		for( FxSize i = 0; i < _sampleCount - (2*_windowSampleCount); i+= _windowStep )
		{
#ifndef FX_DETECT_FREQUENCY
			ComputePower(i, window);
		}
#else
			GetDifferenceFunctionAndComputePower(i, differenceFunc, window);
			GetCumulativeMeanNormalizedDifferenceFunction(differenceFunc, cmndFunc);
			ExtractFundamentalFrequency(cmndFunc);
		}

		// Compute the average pitch for the utterance.
		_frequencyAverage = 0.0f;
		FxInt32 count = 0;
		for( FxSize i = 0; i < _fundamentalFrequency.Length(); ++i )
		{
			if( IsVoiced(static_cast<FxInt32>(i*_windowStep)) &&
				_fundamentalFrequency[i] >= 40.0f &&
				_fundamentalFrequency[i] <= 750.0f ) // Make sure the frequency is at least reasonable.
			{
				_frequencyAverage += _fundamentalFrequency[i];
				count++;
			}
		}
		_frequencyAverage /= static_cast<FxReal>(count);

		delete[] differenceFunc;
		delete[] cmndFunc;
#endif
	}
}

void FxStressDetector::Synch( FxPhonemeList* pPhonList, const FxGestureConfig& config )
{
	_pPhonList = pPhonList;
	if( _pPhonList && _pSamples )
	{
		FxReal windowDuration = static_cast<FxReal>(_windowStep) / _sampleRate;

#ifdef FX_DETECT_FREQUENCY
		// Compute the average pitch for the utterance.
		_frequencyAverage = 0.0f;
		FxInt32 count = 0;
		for( FxSize i = 0; i < _fundamentalFrequency.Length(); ++i )
		{
			if( IsVoiced(static_cast<FxInt32>(i*_windowStep)) &&
				// Make sure the frequency is reasonable.
				_fundamentalFrequency[i] >= config.pfeAllowedFrequency.min &&
				_fundamentalFrequency[i] <= config.pfeAllowedFrequency.max )
			{
				_frequencyAverage += _fundamentalFrequency[i];
				count++;
			}
		}
		_frequencyAverage /= static_cast<FxReal>(count);

		// Magic number alert: the 12th root of 2
		// If you multiply a given frequency by this number, you'll get
		// the next note up the scale. I.e. from concert A = 440Hz
		// * ~1.059 = ~466.16Hz = ~A#.  This provides me an easy way of
		// qualitatively determining thresholds for pitch contours.
		const FxReal noteIncrement = 1.059463094359f;
		// Thresholds for relative pitch classification.
		FxReal avgPitchUpper = GetAvgFrequency();
		for( FxInt32 i = 0; i < config.pfeAvgPitchUpperNumSteps; ++i )
		{
			avgPitchUpper *= noteIncrement;
		}
		FxReal avgPitchLower = GetAvgFrequency();
		for( FxInt32 i = 0; i < config.pfeAvgPitchLowerNumSteps; ++i )
		{
			avgPitchLower /= noteIncrement;
		}

		// Thresholds for pitch contour classification.
		const FxReal minAcceptablePitch = GetAvgFrequency() * 0.5f; // One octave down.
		const FxReal maxAcceptablePitch = GetAvgFrequency() * 2.0f; // One octave up.
#endif

		// Compute and store the per-vowel properties.
		_stressInfos.Clear();
		FxReal vowelDuration = 0.0f;
		FxSize vowelCount = 0;
		for( FxSize i = 0; i < _pPhonList->GetNumPhonemes(); ++i )
		{
			if( _pPhonList->GetPhonemeEnum(i) >= PHON_IY )
			{
				FxStressInformation stressInfo;
				FxMemset(&stressInfo, 0, sizeof(FxStressInformation));
				stressInfo.phonemeIndex = i;

				stressInfo.startTime = _pPhonList->GetPhonemeStartTime(i);
				stressInfo.endTime   = _pPhonList->GetPhonemeEndTime(i);
				stressInfo.duration  = stressInfo.endTime - stressInfo.startTime;
				vowelDuration += stressInfo.duration;
				vowelCount += 1;

				// Compute the power information over the phoneme.
				FxReal power = 0.0f;
				FxReal minPower = FX_REAL_MAX;
				FxReal maxPower = FX_REAL_MIN;
				FxReal windowCount = 0.0f;
				FxReal oneThird = (stressInfo.endTime - stressInfo.startTime) / 3;
				for( FxReal t = stressInfo.startTime + oneThird; t <= stressInfo.endTime - oneThird; t += windowDuration )
				{
					FxReal tempPower = GetPower(t);
					power += tempPower;
					minPower = FxMin(minPower, tempPower);
					maxPower = FxMax(maxPower, tempPower);
					windowCount += 1.0f;
				}
				power /= windowCount;
				stressInfo.averagePower = power;
				stressInfo.minPower = minPower;
				stressInfo.maxPower = maxPower;

#ifdef FX_DETECT_FREQUENCY
				// Compute the pitch information over the phoneme.
				// I've added a little extra room to the pitches array since it
				// seems that due to some floating point inaccuracies, we occasionally
				// sample an extra pitch or two from the time range.  Harmless 
				// algorithmically, but I don't want to trash any memory.
				FxInt32 numPitches = stressInfo.duration/windowDuration + 10;
				FxReal* pitches = new FxReal[numPitches];
				FxInt32 count = 0;
				// Throw out pitches outside of a given range.
				for( FxReal t = stressInfo.startTime; t < stressInfo.endTime; t += windowDuration )
				{
					FxReal pitch = GetFundamentalFrequency(t);
					if( pitch < minAcceptablePitch || pitch > maxAcceptablePitch )
					{
						pitch = FxInvalidValue;
					}
					pitches[count++] = pitch;
				}
				// Find the first valid pitch
				for( FxInt32 i = 0; i < count; ++i )
				{
					if( pitches[i] != FxInvalidValue )
					{
						stressInfo.startPitch = pitches[i];
						break;
					}
				}
				// Find the last valid pitch
				for( FxInt32 i = count-1; i >= 0; --i )
				{
					if( pitches[i] != FxInvalidValue )
					{
						stressInfo.endPitch = pitches[i];
						break;
					}
				}
				// Find the midpoint pitch
				FxInt32 halfLength = count/2;
				for( FxInt32 offset = 0; offset < halfLength; ++offset )
				{
					if( pitches[halfLength+offset] != FxInvalidValue )
					{
						stressInfo.midPitch = pitches[halfLength+offset];
						break;
					}
					if( pitches[halfLength-offset] != FxInvalidValue )
					{
						stressInfo.midPitch = pitches[halfLength-offset];
						break;
					}
				}

				// Calculate the average pitch
				FxInt32 numSamples = 0;
				stressInfo.averagePitch = 0.0f;
				for( FxInt32 i = 0; i < count; ++i )
				{
					if( pitches[i] != FxInvalidValue )
					{
						stressInfo.averagePitch += pitches[i];
						numSamples++;
					}
				}
				stressInfo.averagePitch /= static_cast<FxReal>(numSamples);
				delete[] pitches;

				// Classify the pitch contour.

				// Determine the relative pitch of the phoneme.
				if( stressInfo.averagePitch >= avgPitchUpper )
				{
					stressInfo.relativePitch = IRP_High;
				}
				else if( stressInfo.averagePitch <= avgPitchLower )
				{
					stressInfo.relativePitch = IRP_Low;
				}
				else
				{
					stressInfo.relativePitch = IRP_Neutral;
				}


				// For our purposes, we'll use a whole step in either direction
				// for determining rising or falling contours.
				FxReal risingTargetPitch = stressInfo.startPitch;
				FxReal fallingTargetPitch = stressInfo.startPitch;
				const FxSize numSteps = 2; // Two half steps, or one whole step.
				for( FxSize i = 0; i < numSteps; ++i )
				{
					risingTargetPitch *= noteIncrement;
					fallingTargetPitch /= noteIncrement;
				}

				if( stressInfo.endPitch >= risingTargetPitch )
				{
					stressInfo.contour = IC_Rising;
				}
				else if( stressInfo.endPitch <= fallingTargetPitch )
				{
					stressInfo.contour = IC_Falling;
				}
				else if( stressInfo.midPitch >= risingTargetPitch )
				{
					stressInfo.contour = IC_Hump;
				}
				else if( stressInfo.midPitch <= fallingTargetPitch )
				{
					stressInfo.contour = IC_Dip;
				}
				else
				{
					stressInfo.contour = IC_Flat;
				}
#endif

				_stressInfos.PushBack(stressInfo);
			}
			else if( _pPhonList->GetPhonemeEnum(i) == PHON_SIL &&
					 _pPhonList->GetPhonemeDuration(i) > config.sdShortSilence )
			{
				FxStressInformation stressInfo;
				FxMemset(&stressInfo, 0, sizeof(FxStressInformation));
				stressInfo.phonemeIndex = i;

				stressInfo.startTime = _pPhonList->GetPhonemeStartTime(i);
				stressInfo.endTime   = _pPhonList->GetPhonemeEndTime(i);
				stressInfo.duration  = stressInfo.endTime - stressInfo.startTime;
				
				stressInfo.maxPower     = FxInvalidValue;
				stressInfo.averagePower = FxInvalidValue;
				stressInfo.minPower		= FxInvalidValue;

				_stressInfos.PushBack(stressInfo);
			}
		}

		FxReal avgVowelDuration = vowelDuration / static_cast<FxReal>(vowelCount);

		// Normalize the stress calculations
		const FxInt32 numSamples = config.pfePowerNumAdjacentPhones; // Number of adjacent phonemes to consider.
		// TODO: time difference should be dependant on the rate of speech?
		const FxReal validWindowLength = config.pfePowerMaxWindowLength; // Max length of time difference in either direction to consider.
		FxInt32 numStressInfos = static_cast<FxInt32>(_stressInfos.Length());
		for( FxInt32 i = 0; i < numStressInfos; ++i )
		{
			if( !FxRealEquality(_stressInfos[i].averagePower, FxInvalidValue) )
			{
				FxReal sum = 0.0f;
				FxInt32 count = 0;
				FxBool goodLeft = FxTrue;
				FxBool goodRight = FxTrue;
				for( FxInt32 offset = 0; offset < numSamples; ++offset )
				{
					FxInt32 rightIndex = i + offset;
					if( rightIndex < numStressInfos )
					{
						if( !FxRealEquality(_stressInfos[rightIndex].averagePower, FxInvalidValue ) &&
							_stressInfos[rightIndex].startTime - _stressInfos[i].endTime <= validWindowLength && goodRight )
						{
							// Add in the vowel's contribution to the average.
							sum += _stressInfos[rightIndex].maxPower * config.pfeMaxPowerContribution + _stressInfos[rightIndex].averagePower * config.pfeAvgPowerContribution;
							count++;
						}
						else if( FxRealEquality(_stressInfos[rightIndex].averagePower, FxInvalidValue) &&
								_stressInfos[rightIndex].duration > config.sdShortSilence )
						{
							// Stop looking right if we encounter a significant silence.
							goodRight = FxFalse;
						}
					}

					FxInt32 leftIndex = i - offset;
					if( leftIndex >= 0 )
					{
						if( !FxRealEquality(_stressInfos[leftIndex].averagePower, FxInvalidValue ) &&
							_stressInfos[i].startTime - _stressInfos[leftIndex].endTime <= validWindowLength && goodLeft )
						{
							// Add in the vowel's contribution to the average.
							sum += _stressInfos[leftIndex].maxPower * config.pfeMaxPowerContribution + 
								   _stressInfos[leftIndex].averagePower * config.pfeAvgPowerContribution;
							count++;
						}
						else if( FxRealEquality(_stressInfos[leftIndex].averagePower, FxInvalidValue) &&
								_stressInfos[leftIndex].duration > config.sdShortSilence )
						{
							// Stop looking left if we encounter a significant silence.
							goodLeft = FxFalse;
						}
					}

				}

				// Compute the normalized average power.
				if( count > 0 )
				{
					sum /= static_cast<FxReal>(count);	
					_stressInfos[i].normalizedAvgPower = ((_stressInfos[i].maxPower * config.pfeMaxPowerContribution + 
														   _stressInfos[i].averagePower * config.pfeAvgPowerContribution) / sum) - 1.0f;
				}
				else
				{
					_stressInfos[i].normalizedAvgPower = config.sdStressThreshold;
				}
			
#ifdef FX_DETECT_FREQUENCY
				// Give a bonus to higher pitched segments.
				if( _stressInfos[i].relativePitch == IRP_High )
				{
					_stressInfos[i].normalizedAvgPower += config.pfeHighPitchBonus; 
				}
#endif

				// Give a bonus to longer duration segments.
				if( _stressInfos[i].duration > avgVowelDuration )
				{
					_stressInfos[i].normalizedAvgPower += config.pfeLongDurationBonus;
				}
			}
		}

		// Determine rate of speech.
		FxInt32 phonWindowLength = config.pfeRateOfSpeechNumAdjacentPhones;
		for( FxInt32 i = 0; i < numStressInfos; ++i )
		{
			FxInt32 phonIndex = static_cast<FxInt32>(_stressInfos[i].phonemeIndex);
			FxReal duration = 0.0f;
			FxInt32 count = 0;
			for( FxInt32 j = 0; j < phonWindowLength; ++j )
			{
				FxInt32 index = phonIndex - (phonWindowLength/2) + j;
				if( index >= 0 && index < static_cast<FxInt32>(_pPhonList->GetNumPhonemes()) )
				{
					if( _pPhonList->GetPhonemeEnum(index) != PHON_SIL )
					{
						duration += _pPhonList->GetPhonemeDuration(index);
						count++;
					}
				}
			}

			_stressInfos[i].localRateOfSpeech = static_cast<FxReal>(count) / duration; // phonemes/sec
			FxReal phonLength = FxClamp(config.rosAvgPhonemeDuration.rangeMin, 
										1.0f / _stressInfos[i].localRateOfSpeech,
										config.rosAvgPhonemeDuration.rangeMax);
			_stressInfos[i].localTimeScale = ((phonLength - config.rosAvgPhonemeDuration.rangeMin) / 
				(config.rosAvgPhonemeDuration.Length())) * 
				config.rosTimeScale.Length() + config.rosTimeScale.rangeMin;
		}

		// Classify the stresses.
		FxSize prevIndex = FxInvalidIndex;
		FxSize currIndex = FxInvalidIndex;
		FxSize nextIndex = FxInvalidIndex;
		// Set the indices
		FxBool setCurr = FxFalse;
		for( FxSize i = 0; i < static_cast<FxSize>(numStressInfos); ++i )
		{
			if( !setCurr && _stressInfos[i].normalizedAvgPower >= config.sdStressThreshold )
			{
				currIndex = i;
				setCurr = FxTrue;
			}
			else if( setCurr && _stressInfos[i].normalizedAvgPower >= config.sdStressThreshold )
			{
				nextIndex = i;
				break;
			}
		}
		if( currIndex == FxInvalidIndex )
		{
			//@TODO no stress in the phoneme stream?
		}
		if( nextIndex == FxInvalidIndex )
		{
			//@TODO only one stress in phoneme stream?
		}
		FxBool needsInitial = FxTrue;
		while( currIndex != FxInvalidIndex )
		{
			// Check time differences between stressed segments.
			FxBool quick = FxFalse;
			FxBool isolated = FxTrue;
			if( prevIndex != FxInvalidIndex )
			{
				FxReal timeDifference = _stressInfos[currIndex].startTime - _stressInfos[prevIndex].endTime;
				if( timeDifference < config.sdQuickStressLimit )
				{
					quick = FxTrue;
				}
				else if( timeDifference < config.sdIsolatedStressLimit )
				{
					isolated = FxFalse;
				}
			}
			if( nextIndex != FxInvalidIndex )
			{
				FxReal timeDifference = _stressInfos[nextIndex].startTime - _stressInfos[currIndex].endTime;
				if( timeDifference < config.sdQuickStressLimit )
				{
					quick = FxTrue;
				}
				else if( timeDifference < config.sdIsolatedStressLimit )
				{
					isolated = FxFalse;
				}
			}

			// Check if there's an utterance delimiting silence between curr and next
			FxBool needsFinal = FxFalse;
			if( nextIndex != FxInvalidIndex )
			{
				for( FxSize i = currIndex; i < nextIndex; ++i )
				{
					if( _stressInfos[i].averagePower == FxInvalidValue &&
						_stressInfos[i].duration >= config.sdLongSilence )
					{
						needsFinal = FxTrue;
					}
				}
			}
			else
			{
				needsFinal = FxTrue;
			}

			// Classify the current segment.
			if( needsInitial )
			{
				_stressInfos[currIndex].stressCategory = SC_Initial;
				needsInitial = FxFalse;
			}
			else if( needsFinal )
			{
				_stressInfos[currIndex].stressCategory = SC_Final;
				needsInitial = FxTrue;
			} 
			else if( quick )
			{
				_stressInfos[currIndex].stressCategory = SC_Quick;
			}
			else if( isolated )
			{
				_stressInfos[currIndex].stressCategory = SC_Isolated;
			}
			else
			{
				_stressInfos[currIndex].stressCategory = SC_Normal;
			}
			
			// Advance the counters.
			prevIndex = currIndex;
			currIndex = nextIndex;
			nextIndex = FxInvalidIndex;
			for( FxSize i = currIndex+1; i < static_cast<FxSize>(numStressInfos); ++i ) //@todo is this kosher?
			{
				if( _stressInfos[i].normalizedAvgPower >= config.sdStressThreshold )
				{
					nextIndex = i;
					break;
				}
			}
		}
	}
}

FxReal FxStressDetector::GetFundamentalFrequency( FxReal time )
{
	if( _pSamples )
	{
		return GetFundamentalFrequency(static_cast<FxInt32>(time * _sampleRate));
	}
	return 0.0f;
}

FxReal FxStressDetector::GetFundamentalFrequency( FxInt32 sample )
{
	// dumb hack
	sample = 0;
#ifdef FX_DETECT_FREQUENCY
	if( _pSamples && sample >= 0 && sample < _sampleCount - static_cast<FxInt32>(2*_windowSampleCount) )
	{
		return _fundamentalFrequency[sample / _windowStep];
	}
#endif
	return 0.0f;
}

FxBool FxStressDetector::IsVoiced( FxReal time )
{
	if( _pPhonList && _pSamples )
	{
		return IsVowel(time);
	}
	if( _pSamples )
	{
		return IsVoiced(static_cast<FxInt32>(time * _sampleRate));
	}
	return FxFalse;
}

FxBool FxStressDetector::IsVoiced( FxInt32 sample )
{
	if( _pPhonList && _pSamples )
	{
		return IsVowel(sample);
	}
#ifdef FX_DETECT_FREQUENCY
	if( _pSamples && sample >= 0 && sample < _sampleCount - static_cast<FxInt32>(2*_windowSampleCount) )
	{
		FxSize frameIndex = sample/_windowStep;
		FxReal frequency = _fundamentalFrequency[frameIndex];
		FxReal certainty = _certainty[frameIndex];
		FxReal power = _power[frameIndex];
		FxReal voicingProbability = 1.0 - ((1.0-certainty) * (1.0-(power-_minPower)/(_maxPower-_minPower)));
		return (50.0f <= frequency && frequency <= 750.0f && voicingProbability > _certaintyThreshold);
	}
#endif
	return FxFalse;
}

FxBool FxStressDetector::IsVowel( FxReal time )
{
	if( _pPhonList )
	{
		FxSize phonIndex = FxInvalidIndex;
		for( FxSize i = 0; i < _pPhonList->GetNumPhonemes(); ++i )
		{
			if( _pPhonList->GetPhonemeStartTime(i) <= time && time < _pPhonList->GetPhonemeEndTime(i) )
			{
				phonIndex = i;
			}
		}
		if( phonIndex != FxInvalidIndex )
		{
			return _pPhonList->GetPhonemeEnum(phonIndex) >= PHON_IY;
		}
	}
	return FxFalse;
}

FxBool FxStressDetector::IsVowel( FxInt32 sample )
{
	if( _pSamples )
	{
		return IsVowel(static_cast<FxReal>(static_cast<FxReal>(sample) / _sampleRate));
	}
	return FxFalse;
}

FxReal FxStressDetector::GetPower( FxReal time )
{
	if( _pSamples )
	{
		return GetPower( static_cast<FxInt32>(time * _sampleRate));
	}
	return 0.0f;
}

FxReal FxStressDetector::GetPower( FxInt32 sample )
{
	if( _pSamples && sample >= 0 && sample < _sampleCount - static_cast<FxInt32>(2*_windowSampleCount) )
	{
		return _power[sample/_windowStep];
	}
	return 0.0f;
}

FxReal FxStressDetector::GetMinPower()
{
	if( _minPower != FX_REAL_MAX )
	{
		return _minPower;
	}
	return 0.0f;
}

FxReal FxStressDetector::GetMaxPower()
{
	if( _maxPower != FX_REAL_MIN )
	{
		return _maxPower;
	}
	return 0.0f;
}

const FxStressInformation& FxStressDetector::GetStressInfo( FxSize vowelIndex ) const
{
	return _stressInfos[vowelIndex];
}

FxSize FxStressDetector::GetNumStressInfos() const
{
	return _stressInfos.Length();
}

#ifdef FX_DETECT_FREQUENCY

void FxStressDetector::GetDifferenceFunctionAndComputePower( FxSize startSample, FxReal* differenceFunc, const FxArray<FxReal>& windowingFunc )
{
	// d_t(rho) = sum(j=1->winLen)(sample_j-sample_(j+rho))^2
	FxReal power = 0.0;
	FxInt32 count = 0;
	FxInt32 halfSampleCount = _windowSampleCount/2;

	for( FxSize rho = 0; rho < _windowSampleCount; ++rho )
	{
		// Get the difference function
		FxReal differenceSum = 0.0f;
		for( FxSize j = 1; j <= _windowSampleCount; ++j )
		{
			FxReal difference = _pSamples[startSample+j] - _pSamples[startSample+rho+j];
			difference *= difference;
			differenceSum += difference;
		}
		differenceFunc[rho] = differenceSum;

		// Compute the power
		FxInt32 sampleIndex = startSample - halfSampleCount + rho;
		if( sampleIndex > 0 && sampleIndex < _sampleCount )
		{
			FxDReal windowedSample = windowingFunc[rho] * _pSamples[sampleIndex];
			windowedSample *= windowedSample;
			power += windowedSample;
			count += 1;
		}
	}

	power *= (1.0f/static_cast<FxReal>(count));
	_minPower = FxMin(_minPower, power);
	_maxPower = FxMax(_maxPower, power);
	_power.PushBack(power);

}

void FxStressDetector::GetCumulativeMeanNormalizedDifferenceFunction( const FxReal* differenceFunc,
																	  FxReal* cmndFunc )
{
	// d_t'(rho) = 1, if rho = 0
	//         or
	//         d_t(rho)/[(1/rho)*sum(j=1->rho)(d_t(j))]
	FxReal runningSum = differenceFunc[0];
	FxReal mean = 0.0f;
	cmndFunc[0] = 1.0f;
	for( FxSize rho = 1; rho < _windowSampleCount; ++rho )
	{
		runningSum += differenceFunc[rho];
		mean = runningSum / static_cast<FxReal>(rho);
		if( mean != 0.0f )
		{
			cmndFunc[rho] = differenceFunc[rho] / mean;
		}
		else
		{
			cmndFunc[rho] = 1.0f;
		}
	}
}

void FxStressDetector::ExtractFundamentalFrequency( const FxReal* cmndFunc )
{
	FxReal globalMin = FX_REAL_MAX;
	FxSize period = 0;
	for( FxSize i = 0; i < _windowSampleCount; ++i )
	{
		// Easy out: if we hit our threshold, we've found the period.
		if( cmndFunc[i] < _cmndThreshold )
		{
			globalMin = cmndFunc[i];
			period = i;
			break;
		}
		// General case: update the guess at the period if the difference is less
		// than the lowest difference we've seen yet.
		if( cmndFunc[i] < globalMin )
		{
			globalMin = cmndFunc[i];
			period = i;
		}
	}
	if( period == 0 )
	{
		_fundamentalFrequency.PushBack( 0.0f );
	}
	else
	{
		_fundamentalFrequency.PushBack( (1.0f / static_cast<FxReal>(period)) * _pAudio->GetSampleRate() );
	}
	_certainty.PushBack( 1.0f - globalMin );
}

#else

void FxStressDetector::ComputePower( FxSize startSample, const FxArray<FxReal>& windowingFunc )
{
	// d_t(rho) = sum(j=1->winLen)(sample_j-sample_(j+rho))^2
	FxReal power = 0.0;
	FxInt32 count = 0;
	FxInt32 halfSampleCount = _windowSampleCount/2;

	for( FxSize rho = 0; rho < _windowSampleCount; ++rho )
	{
		// Compute the power
		FxInt32 sampleIndex = startSample - halfSampleCount + rho;
		if( sampleIndex > 0 && sampleIndex < _sampleCount )
		{
			FxDReal windowedSample = windowingFunc[rho] * _pSamples[sampleIndex];
			windowedSample *= windowedSample;
			power += static_cast<FxReal>(windowedSample);
			count += 1;
		}
	}

	power *= (1.0f/static_cast<FxReal>(count));
	_minPower = FxMin(_minPower, power);
	_maxPower = FxMax(_maxPower, power);
	_power.PushBack(power);
}

#endif

void FxStressDetector::CreateHammingWindow( FxArray<FxReal>& window )
{
	window.Reserve(_windowSampleCount);
	FxDReal k = 2.0 * 3.141592653589793 / (_windowSampleCount - 1);
	for( FxSize i = 0; i < _windowSampleCount; ++i )
	{
		window.PushBack(static_cast<FxReal>(0.54f - 0.46f * FxCos(k * i)));
	}
}

} // namespace Face

} // namespace OC3Ent
