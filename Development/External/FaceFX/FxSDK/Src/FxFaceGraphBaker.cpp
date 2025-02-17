//------------------------------------------------------------------------------
// A special animation player that bakes out Face Graph nodes into animation
// curves.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxFaceGraphBaker.h"
#include "FxActorInstance.h"
#include "FxDeltaNode.h"

namespace OC3Ent
{

namespace Face
{

FxFaceGraphBaker::FxFaceGraphBaker( FxActor* pActor )
	: _pActor(pActor)
{
}

FxFaceGraphBaker::~FxFaceGraphBaker()
{
}

void FxFaceGraphBaker::SetActor( FxActor* pActor )
{
	_pActor = pActor;
}

FxActor* FxFaceGraphBaker::GetActor( void )
{
	return _pActor;
}

void FxFaceGraphBaker::SetAnim( const FxName& animGroupName, const FxName& animName )
{
	_animGroupName = animGroupName;
	_animName = animName;
}

const FxName& FxFaceGraphBaker::GetAnimGroupName( void ) const
{
	return _animGroupName;
}

const FxName& FxFaceGraphBaker::GetAnimName( void ) const
{
	return _animName;
}

static const FxReal FxBakerFPS          = 2048.0f;
static const FxReal FxBakerSpotCheckFPS = 256.0f;
static const FxReal FxDerivativeEpsilon = 0.000001f;
static const FxReal FxBakerAccuracy     = 200.0f;

FX_INLINE FxBool FX_CALL FxRealEqualityBaker( FxReal first, FxReal second, FxReal epsilon )
{
	FxReal difference = first - second;
	return (difference < epsilon && difference > -epsilon);
}

FxBool FxFaceGraphBaker::Bake( const FxArray<FxName>& nodesToBake )
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
				// Set up the first derivative and second derivative nodes as well.
				FxArray<FxFaceGraphNode*> firstDerivativeNodes;
				FxArray<FxFaceGraphNode*> secondDerivativeNodes;
				firstDerivativeNodes.Reserve(numNodesToBake);
				secondDerivativeNodes.Reserve(numNodesToBake);
				for( FxSize i = 0; i < numNodesToBake; ++i )
				{
					// FindNode() returns NULL if the node cannot be found.
					FxFaceGraphNode* nodeToBake = faceGraph.FindNode(nodesToBake[i]);
					if( nodeToBake )
					{
						faceGraphNodes.PushBack(nodeToBake);
						// Create and link the first derivative node.
						FxString firstDerivativeNodeNameStr = nodesToBake[i].GetAsString();
						firstDerivativeNodeNameStr += "_$Baker_Auto_First_Derivative$_";
						FxFaceGraphNode* firstDerivativeNode = faceGraph.FindNode(firstDerivativeNodeNameStr.GetData());
						FxAssert(NULL == firstDerivativeNode);
						firstDerivativeNode = new FxDeltaNode();
						firstDerivativeNode->SetName(firstDerivativeNodeNameStr.GetData());
						faceGraph.AddNode(firstDerivativeNode);
						faceGraph.Link(firstDerivativeNodeNameStr.GetData(), nodesToBake[i], "linear", FxLinkFnParameters());
						firstDerivativeNodes.PushBack(firstDerivativeNode);

						// Create and link the second derivative node.
						FxString secondDerivativeNodeNameStr = nodesToBake[i].GetAsString();
						secondDerivativeNodeNameStr += "_$Baker_Auto_Second_Derivative$_";
						FxFaceGraphNode* secondDerivativeNode = faceGraph.FindNode(secondDerivativeNodeNameStr.GetData());
						FxAssert(NULL == secondDerivativeNode);
						secondDerivativeNode = new FxDeltaNode();
						secondDerivativeNode->SetName(secondDerivativeNodeNameStr.GetData());
						faceGraph.AddNode(secondDerivativeNode);
						faceGraph.Link(secondDerivativeNodeNameStr.GetData(), firstDerivativeNode->GetName(), "linear", FxLinkFnParameters());
						secondDerivativeNodes.PushBack(secondDerivativeNode);
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
				FxArray<FxArray<FxAnimKey> > secondDerivativeKeys;
				FxArray<FxArray<FxAnimKey> > potentiallyDuplicateKeys;

				// Initialize the last values and last derivatives arrays and set up the array of resulting curves.
				FxArray<FxReal> values1FrameAgo;
				FxArray<FxReal> values2FramesAgo;
				FxArray<FxReal> values3FramesAgo;
				FxArray<FxReal> firstDerivatives1FrameAgo;
				FxArray<FxReal> firstDerivatives2FramesAgo;
				FxArray<FxReal> firstDerivatives3FramesAgo;
				FxArray<FxReal> firstDerivatives4FramesAgo;
				FxArray<FxReal> secondDerivatives1FrameAgo;
				FxArray<FxReal> secondDerivatives2FramesAgo;
				FxArray<FxReal> valueEpsilons;
				firstDerivativeKeys.Reserve(numNodes);
				secondDerivativeKeys.Reserve(numNodes);
				potentiallyDuplicateKeys.Reserve(numNodes);
				values1FrameAgo.Reserve(numNodes);
				values2FramesAgo.Reserve(numNodes);
				values3FramesAgo.Reserve(numNodes);
				firstDerivatives1FrameAgo.Reserve(numNodes);
				firstDerivatives2FramesAgo.Reserve(numNodes);
				firstDerivatives3FramesAgo.Reserve(numNodes);
				firstDerivatives4FramesAgo.Reserve(numNodes);
				secondDerivatives1FrameAgo.Reserve(numNodes);
				secondDerivatives2FramesAgo.Reserve(numNodes);
				valueEpsilons.Reserve(numNodes);
				_resultingAnimCurves.Reserve(numNodes);
				for( FxSize i = 0; i < numNodes; ++i )
				{
					firstDerivativeKeys.PushBack(FxArray<FxAnimKey>());
					secondDerivativeKeys.PushBack(FxArray<FxAnimKey>());
					potentiallyDuplicateKeys.PushBack(FxArray<FxAnimKey>());
					values1FrameAgo.PushBack(0.0f);
					values2FramesAgo.PushBack(0.0f);
					values3FramesAgo.PushBack(0.0f);
					firstDerivatives1FrameAgo.PushBack(0.0f);
					firstDerivatives2FramesAgo.PushBack(0.0f);
					firstDerivatives3FramesAgo.PushBack(0.0f);
					firstDerivatives4FramesAgo.PushBack(0.0f);
					secondDerivatives1FrameAgo.PushBack(0.0f);
					secondDerivatives2FramesAgo.PushBack(0.0f);

					FxAssert(faceGraphNodes[i]);
					valueEpsilons.PushBack((faceGraphNodes[i]->GetMax() - faceGraphNodes[i]->GetMin()) / FxBakerAccuracy);

					FxAnimCurve resultingAnimCurve;
					resultingAnimCurve.SetName(faceGraphNodes[i]->GetName());
					_resultingAnimCurves.PushBack(resultingAnimCurve);
				}

				// Compute the timeStep for stepping through the animation at the desired frame rate.
				FxReal timeStep = 1.0f / FxBakerFPS;

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
						FxReal secondDerivative = actorInstance.GetRegister(secondDerivativeNodes[i]->GetName());
						if( FxAbs(secondDerivative) < FxDerivativeEpsilon )
						{
							secondDerivative = 0.0f;
						}
						FxReal value = actorInstance.GetRegister(faceGraphNodes[i]->GetName());

						FxBool firstDerivative2FramesAgoEqualsZero  = FxRealEquality(firstDerivatives2FramesAgo[i],  0.0f);
						FxBool firstDerivative1FrameAgoEqualsZero   = FxRealEquality(firstDerivatives1FrameAgo[i],   0.0f);
						FxBool secondDerivative2FramesAgoEqualsZero = FxRealEquality(secondDerivatives2FramesAgo[i], 0.0f);
						FxBool secondDerivative1FrameAgoEqualsZero  = FxRealEquality(secondDerivatives1FrameAgo[i],  0.0f);
						// Insert a key if we had a zero first derivative and now we don't.  This catches local maxima
						// and local minima of the function.
						if( (firstDerivative2FramesAgoEqualsZero && !firstDerivative1FrameAgoEqualsZero) ||
							// Or, insert a key if we had a non-zero first derivative and now we have a zero derivative.
							(!firstDerivative2FramesAgoEqualsZero && firstDerivative1FrameAgoEqualsZero) ||
							// Or, insert a key if we had a positive first derivative and now we have negative.
							(firstDerivatives2FramesAgo[i] > 0.0f && firstDerivatives1FrameAgo[i] < 0.0f) || 
							// Or, insert a key if we had a negative first derivative and now we have positive.
							(firstDerivatives2FramesAgo[i] < 0.0f && firstDerivatives1FrameAgo[i] > 0.0f) )
						{
							// The first derivative nodes are one frame behind.  In addition we are intentionally 
							// one frame ahead in our evaluation so we can calculate a reliable slopeOut.  This is the
							// reason for the 2.0f multiplication in insertionTime and accessing derivatives from several 
							// frames ago.
							FxReal insertionTime = time - (timeStep * 2.0f);
							// We can only calculate the time value of the derivative change with an accuracy of +/-
							// one frame.  To calculate a reliable derivative, we therefore need to go outside of this
							// uncertainty range. 
							FxReal slopeIn  = firstDerivatives3FramesAgo[i] / timeStep;
							FxReal slopeOut = firstDerivative / timeStep;
							firstDerivativeKeys[i].PushBack(FxAnimKey(insertionTime, values2FramesAgo[i], slopeIn, slopeOut));
						}
						// Insert a key if we had a zero second derivative and now we don't.  This code is in an else if
						// block because these conditions are also true if the above conditions are true and we don't want
						// to insert duplicate keys.  Therefore this else if block should catch only "true" inflection points
						// in the function and not also the local maxima and local minima.
						else if( (secondDerivative2FramesAgoEqualsZero && !secondDerivative1FrameAgoEqualsZero) ||
								 // Or, insert a key if we had a non-zero second derivative and now we have a zero derivative.
								 (!secondDerivative2FramesAgoEqualsZero && secondDerivative1FrameAgoEqualsZero) ||
								 // Or, insert a key if we had a positive second derivative and now we have negative.
								 (secondDerivatives2FramesAgo[i] > 0.0f && secondDerivatives1FrameAgo[i] < 0.0f) || 
								 // Or, insert a key if we had a negative second derivative and now we have positive.
								 (secondDerivatives2FramesAgo[i] < 0.0f && secondDerivatives1FrameAgo[i] > 0.0f) )
						{
							// The second derivative nodes are two frames behind. In addition we are intentionally 
							// one frame ahead in our evaluation so we can calculate a reliable slopeOut.  This is the
							// reason for the 3.0f multiplication in insertionTime and accessing derivatives from several 
							// frames ago.
							FxReal insertionTime = time - (timeStep * 3.0f);
							// We can only calculate the time value of the derivative change with an accuracy of +/-
							// one frame.  To calculate a reliable derivative, we therefore need to go outside of this
							// uncertainty range. 
							FxReal slopeIn  = firstDerivatives4FramesAgo[i] / timeStep;
							FxReal slopeOut = firstDerivatives1FrameAgo[i] / timeStep;
							secondDerivativeKeys[i].PushBack(FxAnimKey(insertionTime, values3FramesAgo[i], slopeIn, slopeOut));
						}

						// Cache the last seen values.
						values3FramesAgo[i]            = values2FramesAgo[i];
						values2FramesAgo[i]            = values1FrameAgo[i];
						values1FrameAgo[i]             = value;
						firstDerivatives4FramesAgo[i]  = firstDerivatives3FramesAgo[i];
						firstDerivatives3FramesAgo[i]  = firstDerivatives2FramesAgo[i];
						firstDerivatives2FramesAgo[i]  = firstDerivatives1FrameAgo[i];
						firstDerivatives1FrameAgo[i]   = firstDerivative;
						secondDerivatives2FramesAgo[i] = secondDerivatives1FrameAgo[i];
						secondDerivatives1FrameAgo[i]  = secondDerivative;
					}
				}

				// Reset the timeStep to the baker's spot checking frame rate.
				timeStep = 1.0f / FxBakerSpotCheckFPS;

				// Actually insert the keys in the correct order.  The face graph is also cleaned up here and 
				// new start and end times of the "animation" (e.g. the collection of resulting animation curves) 
				// are computed.
				FxReal newStartTime = FX_REAL_MAX;
				FxReal newEndTime   = FX_REAL_MIN;
				for( FxSize i = 0; i < numNodes; ++i )
				{
					// First derivative keys are inserted first provided they don't have the same value as the prior key.
					FxReal priorKeyValue = 0.0f;
					FxSize numFirstDerivativeKeys = firstDerivativeKeys[i].Length();
					for( FxSize j = 0; j < numFirstDerivativeKeys; ++j )
					{
						const FxAnimKey& key = firstDerivativeKeys[i][j];
						if( !FxRealEqualityBaker(key.GetValue(), priorKeyValue, valueEpsilons[i]) )
						{
							// Insert the new first derivative key.
							_resultingAnimCurves[i].InsertKey(key);
						}
						else
						{
							// Remember this as a potentially duplicate key.
							potentiallyDuplicateKeys[i].PushBack(key);
						}
						priorKeyValue = key.GetValue();
					}

					// Insert any potentially duplicate keys that actually need to be in the curve to reliably reproduce
					// the function.
					FxSize numPotentiallyDuplicateKeys = potentiallyDuplicateKeys[i].Length();
					for( FxSize j = 0; j < numPotentiallyDuplicateKeys; ++j )
					{
						const FxAnimKey& key = potentiallyDuplicateKeys[i][j];
						// Check to see if our curve already evaluates to the key's value without the key being there.
						// If so, it's a duplicate key.
						if( !FxRealEqualityBaker(key.GetValue(), _resultingAnimCurves[i].EvaluateAt(key.GetTime()), valueEpsilons[i]) )
						{
							// Insert the key as it is not a duplicate and it is actually important.
							_resultingAnimCurves[i].InsertKey(key);
						}
					}

					// Second derivative keys are inserted last.
					FxSize numSecondDerivativeKeys = secondDerivativeKeys[i].Length();
					for( FxSize j = 0; j < numSecondDerivativeKeys; ++j )
					{
						const FxAnimKey& key = secondDerivativeKeys[i][j];
						if( !FxRealEqualityBaker(key.GetValue(), _resultingAnimCurves[i].EvaluateAt(key.GetTime()), valueEpsilons[i]) )
						{
							// Insert the new second derivative key.
							_resultingAnimCurves[i].InsertKey(key);
						}
					}

					// Finally, loop through all of key segments and if we ever get off by more than 
					// valueEpsilons[i], insert one additional key at the maximum error point with
					// autocomputeSlope set to FxTrue.  Once the "corrective" key has been inserted,
					// continue the operation further subdividing the problem segment until a maximum
					// error point can no longer be found.
					FxSize numKeys = _resultingAnimCurves[i].GetNumKeys();
					if( numKeys > 0 )
					{
						FxSize numKeysM1 = numKeys - 1;
						for( FxSize j = 0; j < numKeysM1; ++j )
						{
							// Assume we have an error to start.
							FxBool bFoundError = FxTrue;
							// While there are no more errors in the segment...
							while( bFoundError )
							{
								// Spot check the segment.
								FxReal startTime         = _resultingAnimCurves[i].GetKey(j).GetTime();
								FxReal endTime           = _resultingAnimCurves[i].GetKey(j+1).GetTime();
								FxReal maxErrorValue     = 0.0f;
								FxReal maxErrorTime      = 0.0f;
								FxReal maxErrorValueDiff = 0.0f;
								for( FxReal time = startTime; time < endTime; time += timeStep )
								{
									actorInstance.ForceTick(_animName, _animGroupName, time);
									FxReal nodeValue = actorInstance.GetRegister(faceGraphNodes[i]->GetName());
									FxReal diffValue = FxAbs(_resultingAnimCurves[i].EvaluateAt(time) - nodeValue);
									if( diffValue > maxErrorValueDiff )
									{
										maxErrorValueDiff = diffValue;
										maxErrorTime      = time;
										maxErrorValue     = nodeValue;
									}
								}
								if( maxErrorValueDiff > valueEpsilons[i] )
								{
									// Found a maximum error point so insert a key there with autocomputeSlope turned on.
									_resultingAnimCurves[i].InsertKey(FxAnimKey(maxErrorTime, maxErrorValue), FxTrue);
									numKeysM1++;
								}
								else
								{
									// A maximum error point could not be found.
									bFoundError = FxFalse;
								}
							}
						}
					}
					else
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
					newStartTime = curveMinTime < newStartTime ? curveMinTime : newStartTime;
					newEndTime   = curveMaxTime > newEndTime ? curveMaxTime : newEndTime;

					// Remove the second derivative nodes.
					FxAssert(secondDerivativeNodes[i]);
					faceGraph.RemoveNode(secondDerivativeNodes[i]->GetName());
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

FxSize FxFaceGraphBaker::GetNumResultAnimCurves( void ) const
{
	return _resultingAnimCurves.Length();
}

const FxAnimCurve& FxFaceGraphBaker::GetResultAnimCurve( FxSize index ) const
{
	return _resultingAnimCurves[index];
}

} // namespace Face

} // namespace OC3Ent
