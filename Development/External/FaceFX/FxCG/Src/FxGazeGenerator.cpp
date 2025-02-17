//------------------------------------------------------------------------------
// The generator for random head orientations and eye gazes.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxGazeGenerator.h"
#include "FxPhonemeEnum.h"
#include "FxSimplePhonemeList.h"
#include "FxMath.h"

namespace OC3Ent
{

namespace Face
{

FxGazeGenerator::FxGazeGenerator()
{
	_headRoll.SetName(FxCGGetSpeechGestureTrackName(GT_OrientationHeadRoll));
	_headPitch.SetName(FxCGGetSpeechGestureTrackName(GT_OrientationHeadPitch));
	_headYaw.SetName(FxCGGetSpeechGestureTrackName(GT_OrientationHeadYaw));

	_gazeEyePitch.SetName(FxCGGetSpeechGestureTrackName(GT_GazeEyePitch));
	_gazeEyeYaw.SetName(FxCGGetSpeechGestureTrackName(GT_GazeEyeYaw));
	Clear();
}

FxGazeGenerator::~FxGazeGenerator()
{
}

// Initializes the generator.
void FxGazeGenerator::Initialize( FxPhonemeList* pPhonList, FxInt32 randomSeed )
{
	Clear();
	_random.ResetSeed(randomSeed);
	if( pPhonList && pPhonList->GetNumPhonemes() > 0 )
	{
		// Find the first non-silence phoneme.
		if( pPhonList->GetPhonemeEnum(0) == PHON_SIL )
		{
			_startTime = pPhonList->GetPhonemeEndTime(0);
		}
		else
		{
			_startTime = pPhonList->GetPhonemeStartTime(0);
		}
		// Find the last non-silence phoneme.
		FxSize lastPhonIndex = pPhonList->GetNumPhonemes() - 1;
		if( pPhonList->GetPhonemeEnum(lastPhonIndex) == PHON_SIL )
		{
			_endTime = pPhonList->GetPhonemeStartTime(lastPhonIndex);
		}
		else
		{
			_endTime = pPhonList->GetPhonemeEndTime(lastPhonIndex);
		}
	}
}

// Clears the generated gazes.
void FxGazeGenerator::Clear()
{
	_startTime = FxInvalidValue;
	_endTime = FxInvalidValue;
	_headRoll.RemoveAllKeys();
	_headPitch.RemoveAllKeys();
	_headYaw.RemoveAllKeys();
	_gazeEyePitch.RemoveAllKeys();
	_gazeEyeYaw.RemoveAllKeys();
}

// Runs the random head orientation and eye gaze generation.
void FxGazeGenerator::Process( const FxGestureConfig& config )
{
	// Clear only the tracks.  Keep the start and end times.
	_headRoll.RemoveAllKeys();
	_headPitch.RemoveAllKeys();
	_headYaw.RemoveAllKeys();
	_gazeEyePitch.RemoveAllKeys();
	_gazeEyeYaw.RemoveAllKeys();

	if( !FxRealEquality(_startTime, FxInvalidValue) && !FxRealEquality(_endTime, FxInvalidValue) )
	{
		FxArray<FxGazePosition> headPositions;
		// Create the random head positions.
		GenerateRandomHeadPositions(config, headPositions);
		// Generate the tracks corresponding to those head positions.
		GenerateHeadPositionTracks(config, headPositions);

		FxArray<FxGazePosition> eyePositions;
		// Create the random gaze positions.
		GenerateRandomGazePositions(config, eyePositions);
		// Generate the tracks corresponding to those gaze positions.
		GenerateEyePositionTracks(config, eyePositions);
	}
}

// Returns a generated animation track. (testing only?)
const FxAnimCurve& FxGazeGenerator::GetTrack( FxGestureTrack track )
{
	switch( track )
	{
	default:
		FxAssert(!"Invalid gesture track in FxGazeGenerator!");
	case GT_OrientationHeadRoll:
		return _headRoll;
	case GT_OrientationHeadPitch:
		return _headPitch;
	case GT_OrientationHeadYaw:
		return _headYaw;
	case GT_GazeEyePitch:
		return _gazeEyePitch;
	case GT_GazeEyeYaw:
		return _gazeEyeYaw;
	};
}

void FxGazeGenerator::GenerateRandomHeadPositions( const FxGestureConfig& config, FxArray<FxGazePosition>& headPositions )
{
	FxReal currentTime = _startTime; 
	headPositions.Clear();
	// While we still have room for another transition.
	while( currentTime + config.odHeadTransition.rangeMax * 2 < _endTime )
	{
		FxGazePosition position;
		position.transition.rangeMin = currentTime;
		position.transition.rangeMax = position.transition.rangeMin + _random.RealInRange(config.odHeadTransition.rangeMin, config.odHeadTransition.rangeMax);
		position.duration.rangeMin = position.transition.rangeMax;
		if( _endTime - position.transition.rangeMax - config.odHeadTransition.rangeMax > config.odHeadDuration.rangeMax )
		{
			// If there is enough time to randomize a orientation duration, do so.
			position.duration.rangeMax = position.duration.rangeMin + _random.RealInRange(config.odHeadDuration.rangeMin, config.odHeadDuration.rangeMax);
		}
		else
		{
			// Otherwise, fill the rest of the available time with a orientation
			// before returning to neutral.
			position.duration.rangeMax = _endTime - config.odHeadTransition.rangeMax;
		}
		// Fill each head drift track.
		for( FxSize i = 0; i < config.odHeadProperties.Length(); ++i )
		{
			// If the current track should be affected.
			if( _random.Bool(config.odHeadProperties[i].probability) )
			{
				FxReal value = _random.RealInCentredRange(config.odHeadProperties[i].valueRange.centre, config.odHeadProperties[i].valueRange.range);
				switch( config.odHeadProperties[i].track )
				{
				case GT_OrientationHeadPitch:
					// Pitch cannot be flipped.
					position.pitch = value;
					break;
				case GT_OrientationHeadRoll:
					// Roll can be flipped
					if( _random.Bool() )
					{
						value = -value;
					}
					position.roll = value;
					break;
				case GT_OrientationHeadYaw:
					// Yaw can be flipped.
					if( _random.Bool() )
					{
						value = -value;
					}
					position.yaw = value;
					break;
				default:
					FxAssert(!"Invalid gesture track in FxGazeGenerator!");
				}
			}
		}

		currentTime += position.transition.Length() + position.duration.Length();
		headPositions.PushBack(position);
	}

	// We don't have any more room.  Transition back to neutral.
	FxGazePosition finalPosition;
	finalPosition.transition.rangeMin = currentTime;
	finalPosition.transition.rangeMax = finalPosition.transition.rangeMin + _random.RealInRange(config.odHeadTransition.rangeMin, config.odHeadTransition.rangeMax);
	headPositions.PushBack(finalPosition);
}

void FxGazeGenerator::GenerateHeadPositionTracks( const FxGestureConfig& FxUnused(config), const FxArray<FxGazePosition>& headPositions )
{
	// Only generate head position tracks if there is something more than the 
	// transition back to the neutral position.
	if( headPositions.Length() > 1 )
	{
		FxGazePosition lastPosition;
		for( FxSize i = 0; i < headPositions.Length(); ++i )
		{
			if( headPositions[i].roll != lastPosition.roll )
			{
				_headRoll.InsertKey(FxAnimKey(headPositions[i].transition.rangeMin, lastPosition.roll));
				_headRoll.InsertKey(FxAnimKey(headPositions[i].transition.rangeMax, headPositions[i].roll));
			}
			if( headPositions[i].pitch != lastPosition.pitch )
			{
				_headPitch.InsertKey(FxAnimKey(headPositions[i].transition.rangeMin, lastPosition.pitch));
				_headPitch.InsertKey(FxAnimKey(headPositions[i].transition.rangeMax, headPositions[i].pitch));
			}
			if( headPositions[i].yaw != lastPosition.yaw )
			{
				_headYaw.InsertKey(FxAnimKey(headPositions[i].transition.rangeMin, lastPosition.yaw));
				_headYaw.InsertKey(FxAnimKey(headPositions[i].transition.rangeMax, headPositions[i].yaw));
			}

			lastPosition = headPositions[i];
			
			// Make the holds "moving holds", rather than staying absolutely fixed.
			if( !FxRealEquality(lastPosition.pitch, 0.0f) )
			{
				if( _random.Bool() )
				{
					// Increase
					FxRange range(lastPosition.pitch + (lastPosition.pitch * 0.04f), lastPosition.pitch + (lastPosition.pitch * 0.07f));
					lastPosition.pitch = _random.RealInRange(range.rangeMin, range.rangeMax);
				}
				else
				{
					// Decrease
					FxRange range(lastPosition.pitch - (lastPosition.pitch * 0.04f), lastPosition.pitch - (lastPosition.pitch * 0.07f));
					lastPosition.pitch = _random.RealInRange(range.rangeMin, range.rangeMax);
				}
			}
			if( !FxRealEquality(lastPosition.roll, 0.0f) )
			{
				if( _random.Bool() )
				{
					// Increase
					FxRange range(lastPosition.roll + (lastPosition.roll * 0.04f), lastPosition.roll + (lastPosition.roll * 0.07f));
					lastPosition.roll = _random.RealInRange(range.rangeMin, range.rangeMax);
				}
				else
				{
					// Decrease
					FxRange range(lastPosition.roll - (lastPosition.roll * 0.04f), lastPosition.roll - (lastPosition.roll * 0.07f));
					lastPosition.roll = _random.RealInRange(range.rangeMin, range.rangeMax);
				}
			}
			if( !FxRealEquality(lastPosition.yaw, 0.0f) )
			{
				if( _random.Bool() )
				{
					// Increase
					FxRange range(lastPosition.yaw + (lastPosition.yaw * 0.04f), lastPosition.yaw + (lastPosition.yaw * 0.07f));
					lastPosition.yaw = _random.RealInRange(range.rangeMin, range.rangeMax);
				}
				else
				{
					// Decrease
					FxRange range(lastPosition.yaw - (lastPosition.yaw * 0.04f), lastPosition.yaw - (lastPosition.yaw * 0.07f));
					lastPosition.yaw = _random.RealInRange(range.rangeMin, range.rangeMax);
				}
			}

		}
	}
}

void FxGazeGenerator::GenerateRandomGazePositions( const FxGestureConfig& config, FxArray<FxGazePosition>& gazePositions )
{
	gazePositions.Clear();

	// Set up the weight array.
	FxArray<FxReal> weights;
	for( FxSize i = 0; i < config.odEyeProperties.Length(); ++i )
	{
		weights.PushBack(config.odEyeProperties[i].probability);
	}

	FxReal currentTime = _startTime;
	while( currentTime + config.odEyeDuration.rangeMax + config.odEyeTransition.rangeMax < _endTime )
	{
		FxGazePosition position;
		position.transition.rangeMin = currentTime;
		position.transition.rangeMax = position.transition.rangeMin + _random.RealInRange(config.odEyeTransition.rangeMin, config.odEyeTransition.rangeMax);
		position.duration.rangeMin = position.transition.rangeMax;
		position.duration.rangeMax = position.duration.rangeMin + _random.RealInRange(config.odEyeDuration.rangeMin, config.odEyeDuration.rangeMax);

		FxSize index = _random.Index(weights);
		if( config.odEyeProperties[index].track == GT_GazeEyeYaw )
		{
			position.yaw = _random.RealInCentredRange(config.odEyeProperties[index].valueRange.centre, config.odEyeProperties[index].valueRange.range);
		}
		else if( config.odEyeProperties[index].track == GT_GazeEyePitch )
		{
			position.pitch = _random.RealInCentredRange(config.odEyeProperties[index].valueRange.centre, config.odEyeProperties[index].valueRange.range);
		}
		else // track == GT_Invalid - select a random pitch, yaw position.
		{
			position.yaw   = _random.RealInCentredRange(config.odEyeProperties[index].valueRange.centre, config.odEyeProperties[index].valueRange.range);
			position.pitch = _random.RealInCentredRange(config.odEyeProperties[index].valueRange.centre, config.odEyeProperties[index].valueRange.range);
		}

		currentTime += position.transition.Length() + position.duration.Length();
		gazePositions.PushBack(position);
	}

	// We don't have any more room.  Transition back to neutral.
	FxGazePosition finalPosition;
	finalPosition.transition.rangeMin = currentTime;
	finalPosition.transition.rangeMax = finalPosition.transition.rangeMin + _random.RealInRange(config.odEyeTransition.rangeMin, config.odEyeTransition.rangeMax);
	gazePositions.PushBack(finalPosition);
}

void FxGazeGenerator::GenerateEyePositionTracks( const FxGestureConfig& FxUnused(config), const FxArray<FxGazePosition>& gazePositions )
{
	// Only generate eye position tracks if there is something more than the 
	// transition back to the neutral position.
	if( gazePositions.Length() > 1 )
	{
		FxGazePosition lastPosition;
		for( FxSize i = 0; i < gazePositions.Length(); ++i )
		{
			if( gazePositions[i].pitch != lastPosition.pitch )
			{
				_gazeEyePitch.InsertKey(FxAnimKey(gazePositions[i].transition.rangeMin, lastPosition.pitch));
				_gazeEyePitch.InsertKey(FxAnimKey(gazePositions[i].transition.rangeMax, gazePositions[i].pitch));
			}
			if( gazePositions[i].yaw != lastPosition.yaw )
			{
				_gazeEyeYaw.InsertKey(FxAnimKey(gazePositions[i].transition.rangeMin, lastPosition.yaw));
				_gazeEyeYaw.InsertKey(FxAnimKey(gazePositions[i].transition.rangeMax, gazePositions[i].yaw));
			}
			lastPosition = gazePositions[i];
		}
	}
}

} // namespace Face

} // namespace OC3Ent
