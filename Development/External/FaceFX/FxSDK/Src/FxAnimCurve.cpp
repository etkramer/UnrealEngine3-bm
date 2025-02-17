//------------------------------------------------------------------------------
// An animation curve.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxAnimCurve.h"
#include "FxFaceGraph.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxAnimCurveVersion 1

FX_IMPLEMENT_CLASS(FxAnimCurve, kCurrentFxAnimCurveVersion, FxNamedObject)

FxAnimCurve::FxAnimCurve()
	: FxDataContainer()
	, _cachedPos(FxInvalidIndex)
	, _interp(IT_Hermite)
{
}

FxAnimCurve::FxAnimCurve( const FxAnimCurve& other )
	: Super(other)
	, FxDataContainer(other)
	, _keys(other._keys)
	, _cachedPos(FxInvalidIndex)
	, _interp(other._interp)
{
}

FxAnimCurve& FxAnimCurve::operator=( const FxAnimCurve& other )
{
	if( this == &other ) return *this;
	Super::operator=(other);
	FxDataContainer::operator=(other);
	_keys      = other._keys;
	_cachedPos = FxInvalidIndex;
	_interp	   = other._interp;
	_userData  = other._userData;
	return *this;
}

FxAnimCurve::~FxAnimCurve()
{
}

void FxAnimCurve::SetInterpolator( const FxInterpolationType& interp )
{
	_interp = interp;
}

FxSize FxAnimCurve::InsertKey( const FxAnimKey& key, FxBool autocomputeSlope )
{
	// Sorted insertion based on key time
	// Find the index of the key whose time is greater than the given key's time
	FxSize numKeys = _keys.Length();
	FxSize index = 0;
	for( ; index < numKeys; ++index )
	{
		if( _keys[index].GetTime() > key.GetTime() ) 
		{
			break;
		}
	}

	if( autocomputeSlope && index != 0 && index != numKeys )
	{
		FxAnimKey* prev = &_keys[index-1];
		FxAnimKey* next = &_keys[index];

		// Compute parametric time.
		FxReal deltaTime = (next->GetTime() - prev->GetTime());
		if( deltaTime > 0.0f )
		{
			FxReal t = (key.GetTime() - prev->GetTime()) / deltaTime;
			FxReal s0 = prev->GetSlopeOut() * deltaTime;
			FxReal s1 = next->GetSlopeIn() * deltaTime;
			FxReal slope = (6.0f * prev->GetValue() - 6.0f * next->GetValue() + 3.0f * s0 + 3.0f * s1) * (t * t) + 
				(-6.0f * prev->GetValue() + 6.0f * next->GetValue() - 4.0f * s0 - 2.0f * s1) * t + 
				s0;
			slope /= deltaTime;

			FxAnimKey toInsert(key);
			toInsert.SetSlopeIn(slope);
			toInsert.SetSlopeOut(slope);
			_keys.Insert(toInsert, index);
		}
	}
	else
	{
		_keys.Insert(key, index);
	}
	_cachedPos = FxInvalidIndex;

	return index;
}

void FxAnimCurve::SetKeys( const FxReal* pKeys, FxSize numKeys )
{
	FxAssert(pKeys);
	if( pKeys && numKeys > 0 )
	{
		FxArrayBase<FxAnimKey> arrayBase(numKeys);
		arrayBase._usedCount = numKeys;
		FxSize keyIndex = 0;
		for( FxSize i = 0; i < numKeys; ++i )
		{
			FxReal time     = pKeys[keyIndex++];
			FxReal value    = pKeys[keyIndex++];
			FxReal slopeIn  = pKeys[keyIndex++];
			FxReal slopeOut = pKeys[keyIndex++];
			FxConstructDirect(arrayBase._v + i, FxAnimKey(time, value, slopeIn, slopeOut));
		}
		_keys.SwapArrayBase(arrayBase);
		_cachedPos = FxInvalidIndex;
	}
}

void FxAnimCurve::RemoveKey( FxSize index )
{
	_keys.Remove(index);
	_cachedPos = FxInvalidIndex;
}

void FxAnimCurve::RemoveAllKeys( void )
{
	_keys.Clear();
	_cachedPos = FxInvalidIndex;
}

FxSize FxAnimCurve::FindKey( FxReal time ) const
{
	FxSize retval  = FxInvalidIndex;
	FxSize numKeys = _keys.Length();
	for( FxSize i = 0; i < numKeys; ++i )
	{
		FxReal currTime = _keys[i].GetTime();
		// If the key was found, set the index and break early.
		if( FxRealEquality(currTime, time) )
		{
			retval = i;
			break;
		}
		// If the time where the key should be was passed, break early.
		if( currTime > time )
		{
			break;
		}
	}
	return retval;
}

FxSize FxAnimCurve::ModifyKey( FxSize index, const FxAnimKey& key )
{
	RemoveKey(index);
	return InsertKey(key);
}

FxReal FxAnimCurve::EvaluateAt( const FxReal time ) const
{
	FxReal value = 0.0f;

	if( _keys.Length() > 0 )
	{
		// Check for out-of-range time and clamp to end points of curve.
		if( time <= _keys.Front().GetTime() )
		{
			value = _keys.Front().GetValue();
		}
		else if( time >= _keys.Back().GetTime() )
		{
			value = _keys.Back().GetValue();
		}
		else
		{
			// The time is in range.
			if( 1 == _keys.Length() )
			{
				value = _keys.Front().GetValue();
			}
			else
			{
				FxAnimKey thisKey;
				FxAnimKey nextKey;
				_findBoundingKeys(time, thisKey, nextKey);

				if( IT_Hermite == _interp )
				{
					value = FxHermiteInterpolate(thisKey, nextKey, time);
				}
				else if( IT_Linear == _interp )
				{
					value = FxLinearInterpolate(thisKey, nextKey, time);
				}
			}
		}
	}

	return value;
}

void FxAnimCurve::FindTimeExtents( FxReal& minTime, FxReal& maxTime ) const
{
	// Start with a very large minTime and a very small maxTime.
	minTime = FX_REAL_MAX;
	maxTime = FX_REAL_MIN;

	FxSize numKeys = GetNumKeys();
	if( numKeys > 0 )
	{
		maxTime = GetKey(numKeys-1).GetTime();
		minTime = GetKey(0).GetTime();
	}

	// If no keys were found, clamp minTime and maxTime to reasonable values.
	if( FX_REAL_MAX == minTime && FX_REAL_MIN == maxTime )
	{
		minTime = 0.0f;
		maxTime = 0.0f;
	}
}

void FxAnimCurve::FindValueExtents( FxReal& minValue, FxReal& maxValue ) const
{
	// Start with a very large minValue and a very small maxValue.
	minValue = FX_REAL_MAX;
	maxValue = FX_REAL_MIN;

	FxSize numKeys = GetNumKeys();
	if( numKeys > 0 )
	{
        // Pad the start and end times so we don't miss anything important and 
		// so that the case of only one key is handled gracefully without the 
		// need for another code path.
		FxReal timeStep  = 0.0166666667f; // 60fps.
		FxReal startTime = GetKey(0).GetTime() - (timeStep * 3);
		FxReal endTime   = GetKey(numKeys-1).GetTime() + (timeStep * 3);
		for( FxReal time = startTime; time < endTime; time += timeStep )
		{
			FxReal value = EvaluateAt(time);
			minValue = value < minValue ? value : minValue;
			maxValue = value > maxValue ? value : maxValue;
		}
	}

	// If no values were found, clamp minValue and maxValue to reasonable 
	// values.
	if( FX_REAL_MAX == minValue && FX_REAL_MIN == maxValue )
	{
		minValue = 0.0f;
		maxValue = 0.0f;
	}
}

void FxAnimCurve::Serialize( FxArchive& arc )
{
	Super::Serialize(arc);

	FxUInt16 version = arc.SerializeClassVersion("FxAnimCurve");
	
	FxInt32 tempInterp = static_cast<FxInt32>(_interp);
	arc << tempInterp;
	_interp = static_cast<FxInterpolationType>(tempInterp);

	if( arc.IsLoading() )
	{
		if( version < 1 )
		{
			arc << _keys;
		}
	}
}

} // namespace Face

} // namespace OC3Ent
