//------------------------------------------------------------------------------
// An animation curve.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxAnimCurve_H__
#define FxAnimCurve_H__

#include "FxNamedObject.h"
#include "FxList.h"
#include "FxKey.h"
#include "FxMath.h"

namespace OC3Ent
{

namespace Face
{

/// An animation curve.
/// An animation curve uses FxAnimKey keys as its key type, which contain
/// time, value, incoming slope and outgoing slope.  It also supports
/// two interpolation algorithms: linear and Hermite.  The default is Hermite, since
/// it looks more natural for most actions in facial animation.
///
/// The animation curve is responsible for ensuring that all the keys in the curve
/// remain sorted by time.
/// \ingroup animation
class FxAnimCurve : public FxNamedObject, public FxDataContainer
{
	/// Declare the class.
	FX_DECLARE_CLASS(FxAnimCurve, FxNamedObject)
public:
	/// Constructor.
	FxAnimCurve();
	/// Copy constructor.
	FxAnimCurve( const FxAnimCurve& other );
	/// Assignment operator.
	FxAnimCurve& operator=( const FxAnimCurve& other );
	/// Destructor.
	~FxAnimCurve();

	/// Tests for equality.
	FxBool operator==( const FxAnimCurve& other ) const;
	/// Test for inequality.
	FxBool operator!=( const FxAnimCurve& other ) const;

	/// Returns the interpolation algorithm for the curve.
	FX_INLINE FxInterpolationType GetInterpolator( void ) const { return _interp; }
	/// Sets the interpolation algorithm for the curve.
	void SetInterpolator( const FxInterpolationType& interp );

	/// Returns the number of keys in the curve.
	FX_INLINE FxSize GetNumKeys( void ) const { return _keys.Length(); }
	/// Inserts a key.  Keeps the internal list of keys sorted.
	/// \param key The key to insert.
	/// \param autocomputeSlope \p FxTrue if the method should calculate the slope
	/// such that inserting the key at the proper value would not affect the
	/// curve's shape, \p FxFalse if the slopes in \a key should not be modified.
	/// \return The index of the \a key in the list.
	FxSize InsertKey( const FxAnimKey& key, FxBool autocomputeSlope = FxFalse );
	/// Sets all keys at once from a native array of FxReals.
	/// \param pKeys Array of key times, values, incoming key slopes, and outgoing key slopes.
	/// \param numKeys the number of keys (must be > 0).  pKeys must be numKeys in length
	/// and must be sorted by increasing time.
	/// \note Internal Use Only
	void SetKeys( const FxReal* pKeys, FxSize numKeys );
	/// Removes the key at index.
	void RemoveKey( FxSize index );
	/// Removes all the keys from the curve.
	void RemoveAllKeys( void );
	/// Returns a const reference to the key at index.
	FX_INLINE const FxAnimKey& GetKey( FxSize index ) const { return _keys[index]; }
	/// Returns a mutable reference to the key at index.
	FX_INLINE FxAnimKey& GetKeyM( FxSize index ) { return _keys[index]; }
	
	/// Returns the index of the key with the given time, or \p FxInvalidIndex if
	/// the key does not exist.
	FxSize FindKey( FxReal time ) const;

	/// Sets the key at index equal to the given key, and ensures the list has
	/// remained sorted.
	/// \param index The index of the key to modify.
	/// \param key The new key.
	/// \return The new index of the key.
	/// \note Equivalent to a RemoveKey(), InsertKey() pair.
	FxSize ModifyKey( FxSize index, const FxAnimKey& key );

	/// Returns the value of the curve at the specified time (in seconds). 
	FxReal EvaluateAt( const FxReal time ) const;

	/// Finds the minimum and maximum key times in the curve and sets minTime
	/// and maxTime accordingly.
	void FindTimeExtents( FxReal& minTime, FxReal& maxTime ) const;
	/// Finds the minimum and maximum values in the curve and sets minValue
	/// and maxValue accordingly.  Note that this does not simply compute the
	/// minimum and maximum key values because it must take the tangents into
	/// account by discretely evaluating the curve.  
	/// \note This is a relatively expensive function to execute but is only
	/// executed inside of FaceFX Studio.
	void FindValueExtents( FxReal& minValue, FxReal& maxValue ) const;

	/// Serializes the curve to an archive.
	virtual void Serialize( FxArchive& arc );

private:
	/// Fills in copies of the two keys which bound time.
	void _findBoundingKeys( FxReal time, FxAnimKey& first, FxAnimKey& second ) const;

	/// The keys in the curve.
	FxArray<FxAnimKey> _keys;
	/// The cached position in the key array.
	mutable FxSize _cachedPos;
	/// The interpolator to use.
	FxInterpolationType _interp;
};

FX_INLINE FxBool FxAnimCurve::operator==( const FxAnimCurve& other ) const
{
	return _name == other._name && _keys == other._keys && _interp == other._interp;
}

FX_INLINE FxBool FxAnimCurve::operator!=( const FxAnimCurve& other ) const
{
	return !(operator==(other));
}

FX_INLINE void 
FxAnimCurve::_findBoundingKeys( FxReal time, FxAnimKey& first, FxAnimKey& second ) const
{
	FxSize numKeysM1 = _keys.Length() - 1;
	FxSize pos = 0;

	if( _cachedPos != FxInvalidIndex && _keys[_cachedPos].GetTime() <= time )
	{
		pos = _cachedPos;
	}

	for( ; pos < numKeysM1; ++pos )
	{
		if( _keys[pos].GetTime() <= time && time < _keys[pos+1].GetTime() ) break;
	}

	if( pos != numKeysM1 )
	{
		first  = _keys[pos];
		second = _keys[pos+1];
	}
	else
	{
		first  = _keys[pos-1];
		second = _keys[pos];
	}

	_cachedPos = pos;
}

} // namespace Face

} // namespace OC3Ent

#endif
