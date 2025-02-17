//------------------------------------------------------------------------------
// Provides generic phoneme-to-target style coarticulation.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxCoarticulationGeneric.h"
#include "FxDeltaNode.h"

namespace OC3Ent
{

namespace Face
{

// Constructor.
FxCoarticulationGeneric::FxCoarticulationGeneric()
	: _config(FxCoarticulationConfig()) // Default configuration.
	, _lastPhonEndTime(FxInvalidValue)
	, _hasBeenTicked(FxFalse)
	, _startTime(FX_REAL_MAX)
	, _endTime(FX_REAL_MIN)
{
}

// Destructor.
FxCoarticulationGeneric::~FxCoarticulationGeneric()
{
}

// Set the coarticulation configuration.
void FxCoarticulationGeneric::SetConfig( const FxCoarticulationConfig& config )
{
	_config = config;
}

// Begin the phoneme list.
void FxCoarticulationGeneric::BeginPhonemeList(FxSize numPhones)
{
	ClearPhonemeList();
	_phonData.Reserve(numPhones);
}

// Append a phoneme.
void FxCoarticulationGeneric::AppendPhoneme(FxPhoneme phonEnum, FxReal phonStart, FxReal phonEnd)
{
	// Check if we should split a diphthong.
	FxPhoneme firstPhoneme = phonEnum;
	FxPhoneme secondPhoneme = PHON_SIL;
	switch( phonEnum )
	{
	case PHON_EY:
		firstPhoneme = PHON_EH;
		secondPhoneme = PHON_IY;
		break;
	case PHON_AW:
		firstPhoneme = PHON_AE;
		secondPhoneme = PHON_UH;
		break;
	case PHON_AY:
		firstPhoneme = PHON_AE;
		secondPhoneme = PHON_IY;
		break;
	case PHON_OY:
		firstPhoneme = PHON_AO;
		secondPhoneme = PHON_IY;
		break;
	case PHON_OW:
		firstPhoneme = PHON_AO;
		secondPhoneme = PHON_UW; // UH?
		break;
	default:
		break;
	};

	// Undo the changes if the diphthong is too short.
	if( !_config.splitDiphthongs || (secondPhoneme != PHON_SIL && phonEnd - phonStart < _config.diphthongBoundary) )
	{
		firstPhoneme = phonEnum;
		secondPhoneme = PHON_SIL;
	}

	// Append the phoneme.
	_phonData.PushBack(FxCGPhonemeData(firstPhoneme, phonStart - _config.phoneInfo[firstPhoneme].anticipation ));	
	if( secondPhoneme == PHON_SIL )
	{
		// Set the end of the list.
		_lastPhonEndTime = phonEnd - _config.phoneInfo[firstPhoneme].anticipation;
	}
	else
	{
		// Append the second phoneme, if necessary
		_phonData.PushBack(FxCGPhonemeData(secondPhoneme, (phonEnd - phonStart) * 0.5f + phonStart - _config.phoneInfo[secondPhoneme].anticipation));
		// Set the end of the list.
		_lastPhonEndTime = phonEnd - _config.phoneInfo[secondPhoneme].anticipation;
	}

	// Make sure there are enough phonemes in the list before proceeding.
	if( (secondPhoneme == PHON_SIL && _phonData.Length() > 1) ||
		(secondPhoneme != PHON_SIL && _phonData.Length() > 2) )
	{
		FxSize i = _phonData.Length() - (secondPhoneme == PHON_SIL ? 1 : 2);
		FxSize iminus1 = _phonData.Length() - (secondPhoneme == PHON_SIL ? 2 : 3);
		// If we should convert to flaps, and
		if( _config.convertToFlaps && 
			// the previous phoneme is convertible to a flap, and
			(_phonData[iminus1].phoneme == PHON_T || _phonData[iminus1].phoneme == PHON_D) &&
			// there are enough phonemes in the list.
			((secondPhoneme == PHON_SIL && _phonData.Length() > 2) ||
			(secondPhoneme != PHON_SIL && _phonData.Length() > 3)) )
		{
			// Check if we should convert a T or D to a flap
			FxSize iminus2 = _phonData.Length() - (secondPhoneme == PHON_SIL ? 3 : 4);
			if( _phonData[iminus2].phoneme >= PHON_IY && _phonData[i].phoneme >= PHON_IY )
			{
				_phonData[iminus1].phoneme = PHON_FLAP;
			}
		}

		// Check if we should merge an HH into the succeeding phoneme.
		if( _phonData[iminus1].phoneme == PHON_HH )
		{
			_phonData[iminus1].phoneme = firstPhoneme;
			_phonData.Remove(i);
		}
		// Check if the phoneme is the same as the preceding phoneme, and merge.
		else if( _phonData[iminus1].phoneme == _phonData[i].phoneme )
		{
			_phonData.PopBack();
		}
	}
}

// End the phoneme list.
void FxCoarticulationGeneric::EndPhonemeList()
{
	// Intentionally blank... not a necessary function call, but it looks nicer
	// if the code flows like
	// 
	// BeginPhonemeList();
	// AppendPhoneme(); // num phoneme times
	// EndPhonemeList();
	// 
	// ... just so that you don't think I forget to write the function body.
}

// Begin phoneme to track mapping input.
void FxCoarticulationGeneric::BeginMapping()
{
	ClearMapping();

	// Add the phoneme nodes.
	for( FxInt32 phoneme = 0; phoneme < NUM_PHONS; ++phoneme )
	{
		FxCombinerNode* phonNode = new FxCombinerNode();
		phonNode->SetName(GetPhonemeNodeName(static_cast<FxPhoneme>(phoneme)));
		_mappingGraph.AddNode(phonNode);
	}
}

// Add a mapping entry to the phoneme to track mapping.
FxBool FxCoarticulationGeneric::AddMappingEntry(FxPhoneme phonEnum, FxName track, FxReal amount)
{
	// Ensure a node for the track exists, as well as its corresponding delta node.
	FxFaceGraphNode* trackNode = _mappingGraph.FindNode(track);
	if( !trackNode )
	{
		// Create the combiner node and add it to the graph.
		trackNode = new FxCombinerNode();
		trackNode->SetName(track);
		if( amount > 0.0f )
		{
			trackNode->SetMax(amount);
		}
		else
		{
			trackNode->SetMin(amount);
			trackNode->SetMax(0.0f);
		}
		_mappingGraph.AddNode(trackNode);

		// Create the corresponding delta node and add it to the graph.
		FxDeltaNode* trackDeltaNode = new FxDeltaNode();
		FxString trackDeltaNodeName(FxString::Concat(track.GetAsCstr(), "_Delta"));
		trackDeltaNode->SetName(trackDeltaNodeName.GetData());
		_mappingGraph.AddNode(trackDeltaNode);

		// Link the track combiner node to the track delta node.
		_mappingGraph.Link(trackDeltaNode->GetName(), trackNode->GetName(), "linear", FxLinkFnParameters());

		// Cache the track name.
		_trackNames.PushBack(track);
	}
	else
	{
		// Need to update bounds
		if( amount < trackNode->GetMin() )
		{
			trackNode->SetMin(amount);
		}
		else if( amount > trackNode->GetMax() )
		{
			trackNode->SetMax(amount);
		}
	}

	// Link the phoneme to the track with the correct weight.
	FxLinkFnParameters params;
	params.parameters.PushBack(amount);
	FxLinkErrorCode err = _mappingGraph.Link(trackNode->GetName(), GetPhonemeNodeName(phonEnum), "linear", params);
	return err == LEC_None || err == LEC_Duplicate; // Duplicate links are OK.
}

// End phoneme to track mapping input.
void FxCoarticulationGeneric::EndMapping()
{
	// Compile the graph.
	_cg.Compile(_mappingGraph);

	// Create the registers.
	FxSize numNodes = _cg.nodes.Length();
	_registers.Reserve(numNodes);
	for( FxSize i = 0; i < numNodes; ++i )
	{
		_registers.PushBack(FxRegister());
	}

	// Clear the face graph leaving only the compiled graph.
	_mappingGraph.Clear();

	// Create the phoneme node map.
	_phonNodeMap.Reserve(NUM_PHONS);
	for( FxInt32 phoneme = 0; phoneme < NUM_PHONS; ++phoneme )
	{
		FxSize phonNodeIndex = _cg.FindNodeIndex(GetPhonemeNodeName(static_cast<FxPhoneme>(phoneme)));
		FxAssert(FxInvalidIndex != phonNodeIndex);
		_phonNodeMap.PushBack(phonNodeIndex);
	}

	FxSize numTargets = _trackNames.Length();

	// Clear the arrays in preparation for generating new ones.
	_curves.Clear();
	_curves.Reserve(numTargets);

    _targetNodeMap.Reserve(numTargets);
	_deltaNodeMap.Reserve(numTargets);
	_lastDerivatives.Reserve(numTargets);
	_lastValues.Reserve(numTargets);
	for( FxSize i = 0; i < numTargets; ++i )
	{
		FxAnimCurve curve;
		curve.SetName(_trackNames[i]);
		_curves.PushBack(curve);

		FxSize targetNodeIndex = _cg.FindNodeIndex(_trackNames[i]);
		FxAssert(FxInvalidIndex != targetNodeIndex);
		_targetNodeMap.PushBack(targetNodeIndex);
		FxString trackDeltaNodeName(FxString::Concat(_trackNames[i].GetAsString(), "_Delta"));
		FxSize deltaNodeIndex = _cg.FindNodeIndex(trackDeltaNodeName.GetData());
		FxAssert(FxInvalidIndex != deltaNodeIndex);
		_deltaNodeMap.PushBack(deltaNodeIndex);

		_lastDerivatives.PushBack(0.0f);
		_lastValues.PushBack(0.0f);

	}

}

// Clear the phoneme list.
void FxCoarticulationGeneric::ClearPhonemeList()
{
	_phonData.Clear();
	_lastPhonEndTime = FxInvalidValue;
}

// Clear the phoneme to track mapping.
void FxCoarticulationGeneric::ClearMapping()
{
	_mappingGraph.Clear();
	_cg.Clear();
	_registers.Clear();
	_trackNames.Clear();
	_targetNodeMap.Clear();
	_deltaNodeMap.Clear();
	_phonNodeMap.Clear();
	_lastDerivatives.Clear();
	_lastValues.Clear();
	_hasBeenTicked = FxFalse;
}

// Does the coarticulation calculations and phoneme to track mapping.
void FxCoarticulationGeneric::Process()
{
	// Coarticulate the phoneme list, filling out the per-phoneme curves.
	DoCoarticulation();
	// Merge the per-phoneme curves into per-track curves based on the mapping.
	DoMapping();
}

// Gets the number of curves that were generated.
FxSize FxCoarticulationGeneric::GetNumCurves()
{
	return _curves.Length();
}

// Returns a curve that was generated.
const FxAnimCurve& FxCoarticulationGeneric::GetCurve( FxSize index )
{
	return _curves[index];
}

// Returns the name of a phoneme node.
FxName FxCoarticulationGeneric::GetPhonemeNodeName(FxPhoneme phon)
{
	FxChar tempBuffer[255];
	return FxItoa(phon, tempBuffer, 255);
}

// Do the coarticulation.
void FxCoarticulationGeneric::DoCoarticulation()
{
	// Clear the start time, end time and phoneme data.
	FxSize numPhones = _phonData.Length();
	_startTime = FX_REAL_MAX;
	_endTime = FX_REAL_MIN;

	// The amount that the last phoneme was suppressed.
	// This prevents a suppressed phoneme from over-suppressing it's neighbor.
	FxReal cachedSuppression = 0.0f;

	// The amount the previous phoneme is suppressing the current phoneme.
	FxReal prevSuppression = 0.0f;
	// The amount the next phoneme is suppressing the current phoneme.
	FxReal nextSuppression = 0.0f;

	// For each phoneme in the list, 
	// - Calculate the suppression of the phoneme
	// - Insert the default triplet
	for( FxSize i = 0; i < numPhones; ++i )
	{
		if( i != 0 )
		{
			// Calculate the suppression that the previous phoneme has on the current phoneme.
			prevSuppression = CalculateSuppression(_phonData[i].phoneme, _phonData[i-1].phoneme, GetPhonemeDuration(i-1), cachedSuppression);

			if( _phonData[i-1].phoneme != PHON_SIL )
			{
				// If we weren't preceded by a silence, use the preceding phoneme's start time.
				_phonData[i].leadIn.SetTime(_phonData[i-1].startTime);
			}
			else
			{
				if( GetPhonemeDuration(i-1) > _config.shortSilenceDuration || i < 2 )
				{
					// If we have a "longer" silence, ramp in.
					_phonData[i].leadIn.SetTime(_phonData[i].startTime - _config.leadInTime);
				}
				else
				{
					// If we have a "short" silence, blend over it completely.
					_phonData[i].leadIn.SetTime(_phonData[i-2].startTime);
				}
			}
		}
		else
		{
			// If we're the first phoneme in the animation, ramp in.
			_phonData[i].leadIn.SetTime(_phonData[0].startTime - _config.leadInTime);
		}

		if( i != numPhones - 1 )
		{
			// Calculate the suppression that the next phoneme has on the current phoneme.
			// We pass in 0.0f as the prevSuppression parameter since it doesn't apply
			// to calculations dealing with the next phoneme.
			nextSuppression = CalculateSuppression(_phonData[i].phoneme, _phonData[i+1].phoneme, GetPhonemeDuration(i), 0.0f);

			if( _phonData[i+1].phoneme != PHON_SIL )
			{
				// If we're not succeeded by a silence, use the phoneme's end time.
				_phonData[i].leadOut.SetTime(GetPhonemeEndTime(i));
			}
			else
			{
				if( GetPhonemeDuration(i+1) > _config.shortSilenceDuration || i > numPhones - 2)
				{
					// If we have a "longer" silence, ramp out.
					_phonData[i].leadOut.SetTime(_phonData[i].startTime + _config.leadOutTime);
				}
				else
				{
					// If we have a "short" silence, blend over it completely.
					_phonData[i].leadOut.SetTime(GetPhonemeEndTime(i+1));
				}
			}
		}
		else
		{
			// If we're the last phoneme in the animation, ramp out.
			_phonData[i].leadOut.SetTime(_lastPhonEndTime + _config.leadOutTime);

			nextSuppression = 0.0f;
		}

		// We want to use the greater of the two suppression values.  The lesser suppression 
		// doesn't impact the mouth shape at all because the target was already suppressed
		// beyond it's influence.
		_phonData[i].suppression = FxMax(prevSuppression,nextSuppression);
		cachedSuppression = _phonData[i].suppression;

		// Set the time of the peak key
		_phonData[i].peak.SetTime(_phonData[i].startTime);
		// Set the value of the peak key
		_phonData[i].peak.SetValue(1.0f - _phonData[i].suppression);

		// Update the animation start and end times
		if( _phonData[i].leadIn.GetTime() < _startTime )
		{
			_startTime = _phonData[i].leadIn.GetTime();
		}
		if( _phonData[i].leadOut.GetTime() > _endTime )
		{
			_endTime = _phonData[i].leadOut.GetTime();
		}
	}

	// For each phoneme in the list, fudge around the neighboring keys according
	// to how much the phoneme was being suppressed.
	for( FxSize i = 0; i < numPhones; ++i )
	{
		if( _phonData[i].phoneme != PHON_SIL )
		{
			FxSize prevIndex = i-1;
			FxSize nextIndex = i+1;

			if( _phonData[i].suppression != 0.0f )
			{
				if( i != 0 )
				{
					FxReal duration = GetPhonemeDuration(i);
					FxReal timeChange = _phonData[i].suppression * duration;

					_phonData[prevIndex].leadOut.SetTime(_phonData[prevIndex].leadOut.GetTime() + timeChange);
				}

				if( i != numPhones - 1 )
				{
					FxReal duration = GetPhonemeDuration(i);
					if( i != 0 )
					{
						if( _phonData[prevIndex].phoneme == PHON_SIL && 
							GetPhonemeDuration(prevIndex) > _config.shortSilenceDuration )

						{
							duration += _config.leadInTime;
						}
						else
						{
							duration += GetPhonemeDuration(prevIndex);
						}
					}
					FxReal timeChange = _phonData[i].suppression * duration;

					_phonData[nextIndex].leadIn.SetTime(_phonData[nextIndex].leadIn.GetTime() - timeChange);
				}

			}
		}
	}
}

// Do the phoneme to track mapping.
void FxCoarticulationGeneric::DoMapping()
{
	FxSize numTargets = _trackNames.Length();
	FxArray< FxArray<FxAnimKey> > firstDerivativeKeys;
	firstDerivativeKeys.Reserve(numTargets);
	// Initialize the arrays.
	for( FxSize i = 0; i < numTargets; ++i )
	{
		FxArray<FxAnimKey> keyArray;
		firstDerivativeKeys.PushBack(keyArray);
		_lastDerivatives[i] = 0.0f;
		_lastValues[i] = 0.0f;
		_curves[i].RemoveAllKeys();
	}
	// Pad the start and end times so we don't miss anything important.
	FxReal timeStep = 0.0166666667f; // 60fps
	FxReal halfStep = timeStep * .5f;
	FxReal startTime = _startTime - (timeStep * 3);
	FxReal endTime = _endTime + (timeStep * 3);
	FxSize startPhon = 0;
	FxSize numPhones = _phonData.Length();
	for( FxReal time = startTime; time < endTime; time += timeStep )
	{
		// Evaluate the phoneme curves at the time.
		for( FxSize i = startPhon; i < numPhones; ++i )
		{
 			if( i > 0 && time > _phonData[i-1].leadOut.GetTime() ) startPhon = i - 1;
 			if( i < numPhones - 1 && time < _phonData[i].leadIn.GetTime() && time < _phonData[i+1].leadIn.GetTime() ) break;
			FxReal value = EvaluateCurve(_phonData[i].leadIn, 
				                         _phonData[i].peak, 
										 _phonData[i].leadOut, 
										 time);
			if( !FxRealEquality(value, 0.0f) )
			{
				// Insert the value into the curves.
				FxSize nodeIndex = _phonNodeMap[_phonData[i].phoneme];
				_cg.nodes[nodeIndex].trackValue += value;
			}
		}

		// Tick the compiled graph.
		_cg.Tick(0.0f, _registers, _hasBeenTicked);

		// Go through each track and get its value and derivative.
		for( FxSize i = 0; i < numTargets; ++i )
		{
			FxReal derivative = _registers[_deltaNodeMap[i]].value;
			if( FxAbs(derivative) < FX_REAL_EPSILON )
			{
				derivative = 0.0f;
			}
			FxReal value = _registers[_targetNodeMap[i]].value;
			FxBool lastDerivEqualsZero = FxRealEquality(_lastDerivatives[i], 0.0f);
			FxBool thisDerivEqualsZero = FxRealEquality(derivative, 0.0f);
			// Insert a key if we had a zero derivative and now we don't
			if( lastDerivEqualsZero && !thisDerivEqualsZero)
			{
				// In this case, we can be confident that the actual key is somewhere in 
				// this frame.  1/2 way through is a good guess.  Use _lastValues[i] to 
				// make sure if we've started up from zero that the keys are actually on zero.
				FxAnimKey key(time-halfStep, _lastValues[i]);
				firstDerivativeKeys[i].PushBack(key);
			}
			// Or, insert a key if we had a non-zero derivative and now we have a zero derivative
			else if (!lastDerivEqualsZero && thisDerivEqualsZero)
			{
				// In this case, the key is not in this last frame, but somewhere in the
				// frame before.  1/2 way through is a good guess. 
				FxAnimKey key(time-timeStep-halfStep, _lastValues[i]);
				firstDerivativeKeys[i].PushBack(key);
			}
			else if (
				// Or, insert a key if we had a positive derivative and now we have negative
				(_lastDerivatives[i] > 0.0f && derivative < 0.0f) || 
				// Or, insert a key if we had a negative derivative and now we have positive
				(_lastDerivatives[i] < 0.0f && derivative > 0.0f) )
			{
				// We know the key is somewhere between time and time-timestep*2.  Calculate
				// an optimal time placement by comparing the changes.
				FxReal keyTime = time - .5f * timeStep - timeStep * (FxAbs(derivative)/(FxAbs(derivative) + FxAbs(_lastDerivatives[i])));
				firstDerivativeKeys[i].PushBack(FxAnimKey(keyTime, _lastValues[i]));
			}
			_lastValues[i] = value;
			_lastDerivatives[i] = derivative;
		}
	}

	// Accuracy constant (memory vs. accuracy).  The threshold is relative to node min/max.
	// In the second pass this number is used to calculate a threshold, but the slope
	// of the curve is also used since the baker can only be as accurate as the timeStep variable.
	FxReal fxMaximumErrorConstant = 0.05f;
	FxArray<FxReal> threshold;
	for( FxSize i = 0; i < numTargets; ++i )
	{
		FxSize nodeIndex = _cg.FindNodeIndex(_trackNames[i]);
		threshold.PushBack(FxAbs(fxMaximumErrorConstant * (_cg.nodes[nodeIndex].nodeMax - _cg.nodes[nodeIndex].nodeMin)));
	}

	// Go through the first derivative keys and insert them into the curve.
	// We do this in a separate pass so it is easy to remove duplicate first derivative
	// keys.  Use a *very* low threshold if you want to remove duplicates because
	// you can be off by this amount for extended periods of time.
	// Duplicate first derivative keys aren't much of a problem at 60fps, so we'll
	// just insert the keys directly.
	for( FxSize i = 0; i < numTargets; ++i )
	{
		FxSize numKeys = firstDerivativeKeys[i].Length();
		for( FxSize j = 0; j < numKeys; ++j )
		{	
			_curves[i].InsertKey(firstDerivativeKeys[i][j]);
		}
	}

	// Now we begin the second pass through the animation, adding keys where
	// the first-derivative-keys don't quite cut it and tweaking the first derivative keys.
	FxArray<FxSize> nextKeyIndexes;
	FxArray<FxReal> lastError;
	FxArray<FxReal> lastErrorDelta;

	nextKeyIndexes.Reserve(numTargets);
	lastError.Reserve(numTargets);
	lastErrorDelta.Reserve(numTargets);
	threshold.Reserve(numTargets);

	// Initialize the arrays.
	for( FxSize i = 0; i < numTargets; ++i )
	{
		if(_curves[i].GetNumKeys() > 1)
		{
			nextKeyIndexes.PushBack(1);
		}
		else
		{
			nextKeyIndexes.PushBack(FxInvalidIndex);
		}
		lastError.PushBack(0.0f);
		lastErrorDelta.PushBack(0.0f);
		_lastDerivatives.PushBack(0.0f);
	}
	startPhon = 0;
	numPhones = _phonData.Length();

	// Go through the animation a second time, inserting additional keys where 
	// needed in between first derivative keys at the point where the difference 
	// is at a local maximum.
	for( FxReal time = startTime; time < endTime; time += timeStep )
	{
		// Evaluate the phoneme curves at the time.
		for( FxSize i = startPhon; i < numPhones; ++i )
		{
			if( i > 0 && time > _phonData[i-1].leadOut.GetTime() ) startPhon = i - 1;
			if( i < numPhones - 1 && time < _phonData[i].leadIn.GetTime() && time < _phonData[i+1].leadIn.GetTime() ) break;
			FxReal value = EvaluateCurve(_phonData[i].leadIn, 
										 _phonData[i].peak, 
										 _phonData[i].leadOut, 
										 time);
			if( !FxRealEquality(value, 0.0f) )
			{
				// Insert the value into the curves.
				FxSize nodeIndex = _phonNodeMap[_phonData[i].phoneme];
				_cg.nodes[nodeIndex].trackValue += value;
			}
		}
		// Tick the compiled graph.
		_cg.Tick(0.0f, _registers, _hasBeenTicked);
		// Go through each track and get its value and derivative.
		for( FxSize i = 0; i < numTargets; ++i )
		{
			// If we are past the last key in the curve (or there was only one key)
			// there is nothing to do here.  Inside this loop, both nextKeyIndex[i] and 
			// nextKeyIndex[i] - 1 index into valid keys in the curve.  They are the next
			// and prior keys from the time (time - timestep).
			if(nextKeyIndexes[i] != FxInvalidIndex)
			{
				FxAssert(nextKeyIndexes[i] != 0);
				FxReal value = _registers[_targetNodeMap[i]].value;
				FxReal estimated = _curves[i].EvaluateAt(time);
				FxReal error = value - estimated;
				FxReal errorDelta = error - lastError[i];
				FxReal derivative = _registers[_deltaNodeMap[i]].value;
				if( FxAbs(derivative) < FX_REAL_EPSILON )
				{
					derivative = 0.0f;
				}
				// Here we insert additional keys at maximum "off points"
				// Only insert additional keys if the error is significant.
				if(	FxAbs(lastError[i]) > threshold[i])		
				{			
					// Check to see if our difference curve is at a local max or a min.
					if( (lastErrorDelta[i] > 0.0f && errorDelta <= 0.0f) || 
						(lastErrorDelta[i] < 0.0f && errorDelta >= 0.0f) )
					{
						FxAnimKey animKey(time-timeStep, _lastValues[i]);
						FxAnimKey priorKey = _curves[i].GetKeyM(nextKeyIndexes[i]-1);
						FxAnimKey nextKey = _curves[i].GetKeyM(nextKeyIndexes[i]);
						if(	animKey.GetTime() - FX_REAL_RELAXED_EPSILON > priorKey.GetTime()	&&
							animKey.GetTime() + FX_REAL_RELAXED_EPSILON < nextKey.GetTime()		)
						{
							// Set the slopes on the new key to point at the previous and next keys.
							// Using autocompute slope or other methods can result in significant differences,
							// especially when the new key is close in time to a neighboring key.
							FxReal newKeySlopeIn = (animKey.GetValue() - priorKey.GetValue()) / (animKey.GetTime() - priorKey.GetTime());
							FxReal newKeySlopeOut = (nextKey.GetValue() - animKey.GetValue()) / (nextKey.GetTime() - animKey.GetTime());

							// Regardless of the threshold for the node, the baker's accuracy is limited by the timeStep
							// variable.  When the slope of the curves are high, we can only expect that the baked curve 
							// has the right value somewhere within one timeStep of the current time.  This prevents unnecessary 
							// keys from being inserted on our standard target curves.
							FxReal keySlope = (nextKey.GetValue() - priorKey.GetValue())/(nextKey.GetTime() - priorKey.GetTime());
							FxReal adjustedThreshold = FxMax(FxAbs(keySlope*timeStep), threshold[i]);

							if (FxAbs(lastError[i]) > adjustedThreshold)
							{
								animKey.SetSlopeIn(newKeySlopeIn);
								animKey.SetSlopeOut(newKeySlopeOut);

								// Slopes on the initial set of keys remains at 0, but if we have 
								// two consecutive correction keys from this pass, we want the slopes 
								// pointing at the adjacent key.  A convenient way to test for how the key was 
								// inserted is to see if the slope is 0, which should never happen on this
								// second pass.
								if(nextKey.GetSlopeIn() != 0.0f)
								{
									_curves[i].GetKeyM(nextKeyIndexes[i]).SetSlopeIn(newKeySlopeOut);
								}
								if(priorKey.GetSlopeOut() != 0.0f)
								{
									_curves[i].GetKeyM(nextKeyIndexes[i]-1).SetSlopeOut(newKeySlopeIn);
								}
								// Insert the key.  I don't autocompute slope because keys that are inserted close to other keys 
								// yeilds an unpredictable autocompute slope.  The calcualted tangents are the best way to go.
								_curves[i].InsertKey(animKey);
								nextKeyIndexes[i] = nextKeyIndexes[i] + 1;
								// Set error and errorDelta to 0 since the curve is perfectly matched at this point.
								error = 0.0f;
								errorDelta = 0.0f;
							}
						}
					}
				}
				if(time - FX_REAL_RELAXED_EPSILON > _curves[i].GetKey(nextKeyIndexes[i]).GetTime())
				{
					nextKeyIndexes[i] = nextKeyIndexes[i] + 1;
					if(nextKeyIndexes[i] >= _curves[i].GetNumKeys())
					{
						nextKeyIndexes[i] = FxInvalidIndex;
					}
				}
				_lastValues[i] = value;
				lastError[i] = error;
				lastErrorDelta[i] = errorDelta;
				_lastDerivatives[i] = derivative;
			}
		}
	}
}

// Calculates the suppression of a phoneme pair.
FxReal FxCoarticulationGeneric::CalculateSuppression( FxPhoneme currPhoneme, FxPhoneme otherPhoneme, FxReal phonemeDuration, FxReal cachedSuppression )
{
	// Calculate the dominance ratio between the phonemes.  A value of 0.0f
	// indicates that currPhoneme is very dominant.
	FxReal dominanceRatio = (1.0f - _config.phoneInfo[currPhoneme].dominance) * _config.phoneInfo[otherPhoneme].dominance;

	// Any phoneme lengths greater than this value will not result in any suppression.
	FxReal zeroSuppressionCutoff = _config.suppressionWindowMax + dominanceRatio * _config.suppressionWindowMaxSlope;
	// The length at which the phoneme will be fully suppressed.
	FxReal fullSuppressionCutoff = _config.suppressionWindowMin + dominanceRatio * _config.suppressionWindowMinSlope;

	// Clamp the separation so it is between zeroSuppressionCutoff and fullSuppressionCutoff.
	FxReal separation = FxClamp(fullSuppressionCutoff, phonemeDuration, zeroSuppressionCutoff);

	// Calculate the suppression.
	FxReal suppression = (zeroSuppressionCutoff - separation) / (zeroSuppressionCutoff - fullSuppressionCutoff);
	// Take into account that the previous phoneme may have been suppressed and 
	// therefore does not suppress the current phoneme to the same extent.
	return suppression - (cachedSuppression * suppression);
}

// Calculates the duration of a phoneme
FxReal FxCoarticulationGeneric::GetPhonemeDuration(FxSize i)
{
	if( i < _phonData.Length() - 1 )
	{
		return _phonData[i+1].startTime - _phonData[i].startTime;
	}
	else
	{
		return _lastPhonEndTime - _phonData.Back().startTime;
	}
}

// Calculates the end time of a phoneme.
FxReal FxCoarticulationGeneric::GetPhonemeEndTime(FxSize i)
{
	if( i < _phonData.Length() - 1 )
	{
		return _phonData[i+1].startTime;
	}
	else
	{
		return _lastPhonEndTime;
	}
}

// Performs Hermite evaluation on three keys
FxReal FxCoarticulationGeneric::EvaluateCurve( const FxAnimKey& leadIn, 
											   const FxAnimKey& peak, 
											   const FxAnimKey& leadOut, 
											   FxReal time )
{
	if( time > leadIn.GetTime() && time < leadOut.GetTime() )
	{
		if( time < peak.GetTime() )
		{
			return FxHermiteInterpolate(leadIn, peak, time);
		}
		else
		{
			return FxHermiteInterpolate(peak, leadOut, time);
		}
	}

	return 0.0f;
}

} // namespace Face

} // namespace OC3Ent
