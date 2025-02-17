//------------------------------------------------------------------------------
// A special animation player that bakes out Face Graph nodes into animation
// curves.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxFastFaceGraphBaker.h"
#include "FxActorInstance.h"
#include "FxDeltaNode.h"

namespace OC3Ent
{

namespace Face
{

FxFastFaceGraphBaker::FxFastFaceGraphBaker( FxActor* pActor )
	: _pActor(pActor)
{
}

FxFastFaceGraphBaker::~FxFastFaceGraphBaker()
{
}

void FxFastFaceGraphBaker::SetActor( FxActor* pActor )
{
	_pActor = pActor;
}

FxActor* FxFastFaceGraphBaker::GetActor( void )
{
	return _pActor;
}

void FxFastFaceGraphBaker::SetAnim( const FxName& animGroupName, const FxName& animName )
{
	_animGroupName = animGroupName;
	_animName = animName;
}

const FxName& FxFastFaceGraphBaker::GetAnimGroupName( void ) const
{
	return _animGroupName;
}

const FxName& FxFastFaceGraphBaker::GetAnimName( void ) const
{
	return _animName;
}


// Accuracy constant (memory vs. accuracy).  The threshold is relative to node min/max.
// In the second pass this number is used to calculate a threshold, but the slope
// of the curve is also used since the baker can only be as accurate as the timeStep variable.
static const FxReal FxMaximumErrorConstant = 0.05f;
static const FxReal FxFastBakerFPS = 60.0f;
static const FxReal FxDerivativeEpsilon = 0.000001f;

FX_INLINE FxBool FX_CALL FxRealEqualityBaker( FxReal first, FxReal second, FxReal epsilon )
{
	FxReal difference = first - second;
	return (difference < epsilon && difference > -epsilon);
}

FxBool FxFastFaceGraphBaker::Bake( const FxArray<FxName>& nodesToBake )
{
	// Always clear the results from a previous bake to keep the baker in
	// a consistent state.
	_resultingAnimCurves.Clear();
	if( _pActor && nodesToBake.Length() > 0 )
	{
		FxSize animGroupIndex = _pActor->FindAnimGroup(_animGroupName);
		if( FxInvalidIndex != animGroupIndex )
		{
			FxAnimGroup& animGroup = _pActor->GetAnimGroup(animGroupIndex);
			FxSize animIndex = animGroup.FindAnim(_animName);
			if( FxInvalidIndex != animIndex )
			{
				// The animation is valid, so grab a reference to it and the
				// Face Graph.
				const FxAnim& anim     = animGroup.GetAnim(animIndex);
				FxFaceGraph& faceGraph = _pActor->GetDecompiledFaceGraph();

				// Set up an array of Face Graph nodes based on nodesToBake.
				FxSize numNodesToBake = nodesToBake.Length();
				FxArray<FxFaceGraphNode*> faceGraphNodes;
				faceGraphNodes.Reserve(numNodesToBake);
				// Set up the first derivative nodes.
				FxArray<FxFaceGraphNode*> firstDerivativeNodes;
				firstDerivativeNodes.Reserve(numNodesToBake);
				for( FxSize i = 0; i < numNodesToBake; ++i )
				{
					// FindNode() returns NULL if the node cannot be found.
					FxFaceGraphNode* nodeToBake = faceGraph.FindNode(nodesToBake[i]);
					if( nodeToBake )
					{
						faceGraphNodes.PushBack(nodeToBake);
						// Create and link the first derivative node.
						FxString firstDerivativeNodeNameStr =  nodesToBake[i].GetAsString();
						firstDerivativeNodeNameStr += "_$Baker_Auto_First_Derivative$_";
						FxFaceGraphNode* firstDerivativeNode = faceGraph.FindNode(firstDerivativeNodeNameStr.GetData());
						FxAssert(NULL == firstDerivativeNode);
						firstDerivativeNode = new FxDeltaNode();
						firstDerivativeNode->SetName(firstDerivativeNodeNameStr.GetData());
						faceGraph.AddNode(firstDerivativeNode);
						faceGraph.Link(firstDerivativeNodeNameStr.GetData(), nodesToBake[i], "linear", FxLinkFnParameters());
						firstDerivativeNodes.PushBack(firstDerivativeNode);
					}
				}

				FxSize numNodes = faceGraphNodes.Length();

				// If the baker would not bake anything early out without doing any work.
				// No need to clean up here because if this condition is true no nodes were
				// ever added to the face graph in the above step.
				if( 0 == numNodes )
				{
					return FxFalse;
				}
				// Note that numNodes is guaranteed to be greater than zero for the remainder
				// of this function, so checking this state is redundant.  This holds true
				// for checking this state on _resultingAnimCurves as well.

				// Compile the face graph.
				_pActor->CompileFaceGraph();
				// Instance the actor.
				FxActorInstance actorInstance(_pActor);

				// Temporary containers for the keys.  As an optimization, the keys aren't inserted as they
				// are found because the correct insertion method would require two passes through the
				// animation.  However, we can compute all the keys in one pass and insert them correctly
				// later without having to pass through the animation for a second time.
				FxArray<FxArray<FxAnimKey> > firstDerivativeKeys;

				// Initialize the last values and last derivatives arrays and set up the array of resulting curves.
				FxArray<FxReal> values1FrameAgo;
				FxArray<FxReal> firstDerivatives1FrameAgo;
				
				firstDerivativeKeys.Reserve(numNodes);
				values1FrameAgo.Reserve(numNodes);
				firstDerivatives1FrameAgo.Reserve(numNodes);
				_resultingAnimCurves.Reserve(numNodes);

				for( FxSize i = 0; i < numNodes; ++i )
				{
					firstDerivativeKeys.PushBack(FxArray<FxAnimKey>());
					values1FrameAgo.PushBack(0.0f);
					firstDerivatives1FrameAgo.PushBack(0.0f);
					FxAssert(faceGraphNodes[i]);
					FxAnimCurve resultingAnimCurve;
					resultingAnimCurve.SetName(faceGraphNodes[i]->GetName());
					_resultingAnimCurves.PushBack(resultingAnimCurve);
				}

				// Compute the timeStep for stepping through the animation at the desired frame rate.
				FxReal timeStep = 1.0f / FxFastBakerFPS;
				FxReal halfStep = timeStep * .5f;

				// Pad the start and end times so we don't miss anything important.
				FxReal startTime = anim.GetStartTime() - (timeStep * 3.0f);
				FxReal endTime = anim.GetEndTime() + (timeStep * 3.0f);

				// Compute all first and second derivatives and cache keys representing the approximate
				// local maxima and local minima as well as important inflection points of the function.  The exact
				// tangents computed from the first derivative are used when caching the keys.
				for( FxReal time = startTime; time < endTime; time += timeStep )
				{
					actorInstance.ForceTick(_animName, _animGroupName, time);
					// Go through each track and get its value and first and second derivatives.
					for( FxSize i = 0; i < numNodes; ++i )
					{
						FxReal firstDerivative = actorInstance.GetRegister(firstDerivativeNodes[i]->GetName());
						if( FxAbs(firstDerivative) < FxDerivativeEpsilon )
						{
							firstDerivative = 0.0f;
						}
						FxReal value = actorInstance.GetRegister(faceGraphNodes[i]->GetName());

						FxBool lastDerivEqualsZero = FxRealEquality(firstDerivatives1FrameAgo[i], 0.0f);
						FxBool thisDerivEqualsZero = FxRealEquality(firstDerivative, 0.0f);
						// Insert a key if we had a zero derivative and now we don't
						if( lastDerivEqualsZero && !thisDerivEqualsZero)
						{
							// In this case, we can be confident that the actual key is somewhere in 
							// this frame.  1/2 way through is a good guess.  Use values1FrameAgo[i] to 
							// make sure if we've started up from zero that the keys are actually on zero.
							FxAnimKey key(time-halfStep, values1FrameAgo[i]);
							firstDerivativeKeys[i].PushBack(key);
						}
						// Or, insert a key if we had a non-zero derivative and now we have a zero derivative
						else if (!lastDerivEqualsZero && thisDerivEqualsZero)
						{
							// In this case, the key is not in this last frame, but somewhere in the
							// frame before.  1/2 way through is a good guess. 
							FxAnimKey key(time-timeStep-halfStep, values1FrameAgo[i]);
							firstDerivativeKeys[i].PushBack(key);
						}
						else if (
							// Or, insert a key if we had a positive derivative and now we have negative
							(firstDerivatives1FrameAgo[i] > 0.0f && firstDerivative < 0.0f) || 
							// Or, insert a key if we had a negative derivative and now we have positive
							(firstDerivatives1FrameAgo[i] < 0.0f && firstDerivative > 0.0f) )
						{
							// We know the key is somewhere between time and time-timestep*2.  Calculate
							// an optimal time placement by comparing the changes.
							FxReal keyTime = time - .5f * timeStep - timeStep * (FxAbs(firstDerivative)/(FxAbs(firstDerivative) + FxAbs(firstDerivatives1FrameAgo[i])));
							firstDerivativeKeys[i].PushBack(FxAnimKey(keyTime, values1FrameAgo[i]));
						}
						values1FrameAgo[i] = value;
						firstDerivatives1FrameAgo[i] = firstDerivative;
					}
				}
				// Go through the first derivative keys and insert them into the curve.
				// We do this in a separate pass so it is easy to remove duplicate first derivative
				// keys.  Use a *very* low threshold if you want to remove duplicates because
				// you can be off by this amount for extended periods of time.
				// Duplicate first derivative keys aren't much of a problem at 60fps, so we'll
				// just insert the keys directly.
				for( FxSize i = 0; i < numNodes; ++i )
				{
					FxSize numKeys = firstDerivativeKeys[i].Length();
					for( FxSize j = 0; j < numKeys; ++j )
					{	
						_resultingAnimCurves[i].InsertKey(firstDerivativeKeys[i][j]);
					}
				}

				// Now we begin the second pass through the animation, adding keys where
				// the first-derivative-keys don't quite cut it and tweaking the first derivative keys.
				FxArray<FxSize> nextKeyIndexes;
				FxArray<FxReal> lastError;
				FxArray<FxReal> lastErrorDelta;
				FxArray<FxReal> valueEpsilons;

				nextKeyIndexes.Reserve(numNodes);
				lastError.Reserve(numNodes);
				lastErrorDelta.Reserve(numNodes);
				valueEpsilons.Reserve(numNodes);

				// Initialize the arrays.
				for( FxSize i = 0; i < numNodes; ++i )
				{
					if(_resultingAnimCurves[i].GetNumKeys() > 1)
					{
						nextKeyIndexes.PushBack(1);
					}
					else
					{
						nextKeyIndexes.PushBack(FxInvalidIndex);
					}
					lastError.PushBack(0.0f);
					lastErrorDelta.PushBack(0.0f);
					valueEpsilons.PushBack(FxMaximumErrorConstant * (faceGraphNodes[i]->GetMax() - faceGraphNodes[i]->GetMin()));
					firstDerivatives1FrameAgo[i] = 0.0f;
					values1FrameAgo[i] = 0.0f;
				}

				// Go through the animation a second time, inserting additional keys where 
				// needed in between first derivative keys at the point where the difference 
				// is at a local maximum.
				for( FxReal time = startTime; time < endTime; time += timeStep )
				{
					actorInstance.ForceTick(_animName, _animGroupName, time);
					// Go through each track and get its value and first and second derivatives.
					for( FxSize i = 0; i < numNodes; ++i )
					{
						// If we are past the last key in the curve (or there was only one key)
						// there is nothing to do here.  Inside this loop, both nextKeyIndex[i] and 
						// nextKeyIndex[i] - 1 index into valid keys in the curve.  They are the next
						// and prior keys from the time (time - timestep).
						if(nextKeyIndexes[i] != FxInvalidIndex)
						{
							FxAssert(nextKeyIndexes[i] != 0);
							FxReal firstDerivative = actorInstance.GetRegister(firstDerivativeNodes[i]->GetName());
							if( FxAbs(firstDerivative) < FxDerivativeEpsilon )
							{
								firstDerivative = 0.0f;
							}
							FxReal value = actorInstance.GetRegister(faceGraphNodes[i]->GetName());
							FxReal estimated = _resultingAnimCurves[i].EvaluateAt(time);
							FxReal error = value - estimated;
							FxReal errorDelta = error - lastError[i];
							// Here we insert additional keys at maximum "off points"
							// Only insert additional keys if the error is significant.
							if(	FxAbs(lastError[i]) > valueEpsilons[i])		
							{			
								// Check to see if our difference curve is at a local max or a min.
								if( (lastErrorDelta[i] > 0.0f && errorDelta <= 0.0f) || 
									(lastErrorDelta[i] < 0.0f && errorDelta >= 0.0f) )
								{
									FxAnimKey animKey(time-timeStep, values1FrameAgo[i]);
									FxAnimKey priorKey = _resultingAnimCurves[i].GetKeyM(nextKeyIndexes[i]-1);
									FxAnimKey nextKey = _resultingAnimCurves[i].GetKeyM(nextKeyIndexes[i]);
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
										FxReal adjustedThreshold = FxMax(FxAbs(keySlope*timeStep), valueEpsilons[i]);

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
												_resultingAnimCurves[i].GetKeyM(nextKeyIndexes[i]).SetSlopeIn(newKeySlopeOut);
											}
											if(priorKey.GetSlopeOut() != 0.0f)
											{
												_resultingAnimCurves[i].GetKeyM(nextKeyIndexes[i]-1).SetSlopeOut(newKeySlopeIn);
											}
											// Insert the key.  I don't autocompute slope because keys that are inserted close to other keys 
											// yeilds an unpredictable autocompute slope.  The calcualted tangents are the best way to go.
											_resultingAnimCurves[i].InsertKey(animKey);
											nextKeyIndexes[i] = nextKeyIndexes[i] + 1;
											// Set error and errorDelta to 0 since the curve is perfectly matched at this point.
											error = 0.0f;
											errorDelta = 0.0f;
										}
									}
								}
							}
							if(time - FX_REAL_RELAXED_EPSILON > _resultingAnimCurves[i].GetKey(nextKeyIndexes[i]).GetTime())
							{
								nextKeyIndexes[i] = nextKeyIndexes[i] + 1;
								if(nextKeyIndexes[i] >= _resultingAnimCurves[i].GetNumKeys())
								{
									nextKeyIndexes[i] = FxInvalidIndex;
								}
							}
							values1FrameAgo[i] = value;
							lastError[i] = error;
							lastErrorDelta[i] = errorDelta;
							firstDerivatives1FrameAgo[i] = firstDerivative;
						}
					}
				}
				// Perform some clean up.
				FxReal newStartTime = FX_REAL_MAX;
				FxReal newEndTime   = FX_REAL_MIN;
				for( FxSize i = 0; i < numNodes; ++i )
				{
					FxSize numKeys = _resultingAnimCurves[i].GetNumKeys();
					if( numKeys == 0 )
					{
						// If there are no keys in the curve check for the edge case of no change in derivatives 
						// but non-zero value (e.g. a constant curve with non-zero value).
						if( !FxRealEquality(values1FrameAgo[i], 0.0f) )
						{
							// This inserts with zero derivative, which is appropriate for this type of curve.
							_resultingAnimCurves[i].InsertKey(FxAnimKey(0.0f, values1FrameAgo[i]));
						}
					}

					// Now check for an updated start or end time.
					FxReal curveMinTime = 0.0f;
					FxReal curveMaxTime = 0.0f;
					_resultingAnimCurves[i].FindTimeExtents(curveMinTime, curveMaxTime);
					// FindTimeExtents will return 0,0 in the event of no keys, so only update newStartTime and 
					// newEndTime if there are keys in the curve.
					if( _resultingAnimCurves[i].GetNumKeys() > 0 ) 
					{
						newStartTime = curveMinTime < newStartTime ? curveMinTime : newStartTime;
						newEndTime   = curveMaxTime > newEndTime ? curveMaxTime : newEndTime;
					}
					// Remove the first derivative nodes.
					FxAssert(firstDerivativeNodes[i]);
					faceGraph.RemoveNode(firstDerivativeNodes[i]->GetName());
				}

				// Preserve the original animation length.
				FxReal originalStartTime = anim.GetStartTime();
				FxReal originalEndTime   = anim.GetEndTime();
				if( !FxRealEquality(newStartTime, originalStartTime) )
				{
					// Insert a key at the original start time that preserves the current value of the curve.
					_resultingAnimCurves[0].InsertKey(FxAnimKey(originalStartTime, _resultingAnimCurves[0].EvaluateAt(originalStartTime)));
				}
				if( !FxRealEquality(newEndTime, originalEndTime) )
				{
					// Insert a key at the original end time that preserves the current value of the curve.
					_resultingAnimCurves[0].InsertKey(FxAnimKey(originalEndTime, _resultingAnimCurves[0].EvaluateAt(originalEndTime)));
				}

				// Compile the face graph.
				_pActor->CompileFaceGraph();
				return FxTrue;
			}
		}
	}
	return FxFalse;
}

FxSize FxFastFaceGraphBaker::GetNumResultAnimCurves( void ) const
{
	return _resultingAnimCurves.Length();
}

const FxAnimCurve& FxFastFaceGraphBaker::GetResultAnimCurve( FxSize index ) const
{
	return _resultingAnimCurves[index];
}

} // namespace Face

} // namespace OC3Ent
