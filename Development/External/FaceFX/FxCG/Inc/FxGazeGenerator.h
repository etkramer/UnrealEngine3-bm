//------------------------------------------------------------------------------
// The generator for random head orientations and eye gazes.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxGazeGenerator_H__
#define FxGazeGenerator_H__

#include "FxPlatform.h"
#include "FxRandomGenerator.h"
#include "FxGestureConfig.h"
#include "FxAnimCurve.h"

#include <time.h>

namespace OC3Ent
{

namespace Face
{

// Forward declare the phoneme list class.
class FxPhonemeList;

// Forward declare the head position struct.
struct FxGazePosition;

class FxGazeGenerator
{
public:
	// Constructor.
	FxGazeGenerator();
	// Destructor.
	~FxGazeGenerator();

	// Initializes the generator.
	void Initialize( FxPhonemeList* pPhonList, FxInt32 randomSeed = time(0) );
	// Clears the generated gazes.
	void Clear();
	// Runs the random head orientation and eye gaze generation.
	void Process( const FxGestureConfig& config );

	// Returns a generated animation track. (testing only?)
	const FxAnimCurve& GetTrack( FxGestureTrack track );

protected:
	void GenerateRandomHeadPositions( const FxGestureConfig& config, FxArray<FxGazePosition>& headPositions );
	void GenerateHeadPositionTracks( const FxGestureConfig& config, const FxArray<FxGazePosition>& headPositions );
	void GenerateRandomGazePositions( const FxGestureConfig& config, FxArray<FxGazePosition>& gazePositions );
	void GenerateEyePositionTracks( const FxGestureConfig& config, const FxArray<FxGazePosition>& gazePositions );

	FxRandomGenerator _random;
	FxReal			  _startTime;
	FxReal			  _endTime;

	FxAnimCurve		  _headRoll;
	FxAnimCurve		  _headPitch;
	FxAnimCurve		  _headYaw;

	FxAnimCurve		  _gazeEyePitch;
	FxAnimCurve		  _gazeEyeYaw;
};

// Internal use only: the gaze position.
struct FxGazePosition
{
	FxGazePosition() 
		: transition(FxRange(FxInvalidValue, FxInvalidValue))
		, duration(FxRange(FxInvalidValue, FxInvalidValue))
		, roll(0.0f)
		, pitch(0.0f)
		, yaw(0.0f)
	{
	}
	FxRange transition;
	FxRange duration;
	FxReal  roll;
	FxReal  pitch;
	FxReal  yaw;
};

} // namespace Face

} // namespace OC3Ent

#endif
