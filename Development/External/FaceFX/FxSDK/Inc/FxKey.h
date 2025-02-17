//------------------------------------------------------------------------------
// This class defines an animation key.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxKey_H__
#define FxKey_H__

#include "FxPlatform.h"
#include "FxArchive.h"
#include "FxObject.h"
#include "FxMath.h"

namespace OC3Ent
{

namespace Face
{

/// An animation key.  An animation key has a time, a value, an incoming
/// slope and an outgoing slope.
/// \ingroup animation
class FxAnimKey
{
public:
	/// Constructor.
	FxAnimKey();
	/// Constructor.
	FxAnimKey( FxReal iTime, FxReal iValue, 
			   FxReal iInSlope = 0.0f, FxReal iOutSlope = 0.0f );
	/// Copy constructor.
	FxAnimKey( const FxAnimKey& other );
	/// Assignment.
	FxAnimKey& operator=(const FxAnimKey& other );
	/// Destructor.
	~FxAnimKey();

	/// Tests for equality.
	FxBool operator==( const FxAnimKey& other ) const;
	/// Test for inequality.
	FxBool operator!=( const FxAnimKey& other ) const;

	/// Returns the time.
	FX_INLINE FxReal GetTime( void ) const { return _time; }
	/// Returns the value.
	FX_INLINE FxReal GetValue( void ) const { return _value; }
	/// Returns the incoming slope.
	FX_INLINE FxReal GetSlopeIn( void ) const { return _slopeIn; }
	/// Returns the  outgoing slope.
	FX_INLINE FxReal GetSlopeOut( void ) const { return _slopeOut; }

	/// Set the time.
	void SetTime( FxReal iTime );
	/// Set the value.
	void SetValue( FxReal iValue );
	/// Set the incoming slope.
	void SetSlopeIn( FxReal iInSlope );
	/// Set the outgoing slope.
	void SetSlopeOut( FxReal iOutSlope );

	/// Returns the user data pointer.
	void* GetUserData( void );
	/// Sets the user data pointer.
	void  SetUserData( void* userData );

	/// Serializes the key to an archive.
	friend FxArchive& operator<<( FxArchive& arc, FxAnimKey& key );

private:
	/// The time of the key (in seconds).
	FxReal _time;
	/// The value of the key.
	FxReal _value;
	/// The incoming slope at the key.
	FxReal _slopeIn;
	/// The outgoing slope at the key.
	FxReal _slopeOut;
	/// The user data.
	void*  _userData;
};

FxArchive& operator<<( FxArchive& arc, FxAnimKey& key );

FX_INLINE FxBool FxAnimKey::operator==( const FxAnimKey& other ) const
{
	return FxRealEquality(_time, other._time) &&
		   FxRealEquality(_value, other._value) &&
		   FxRealEquality(_slopeIn, other._slopeIn) &&
		   FxRealEquality(_slopeOut, other._slopeOut);
}

FX_INLINE FxBool FxAnimKey::operator!=( const FxAnimKey& other ) const
{
	return !(operator==(other));
}

/// Hermite interpolation between two keys.
/// Does not perform range checking on \a currentTime.
FX_INLINE FxReal FX_CALL FxHermiteInterpolate( const FxAnimKey& first,
									           const FxAnimKey& second, 
									           FxReal currentTime )
{
	FxReal time1 = first.GetTime();
	FxReal time2 = second.GetTime();
	FxReal deltaTime = time2 - time1;
	FxReal parametricTime = (currentTime - time1) / deltaTime;

	FxReal p0 = first.GetValue();
	FxReal p1 = second.GetValue();
	FxReal m0 = first.GetSlopeOut() * deltaTime;
	FxReal m1 = second.GetSlopeIn() * deltaTime;

	return parametricTime * (parametricTime * (parametricTime * 
		(2.0f*p0 - 2.0f*p1 + m0 + m1) + (-3.0f*p0 + 3.0f*p1 - 2.0f*m0 - m1)) + m0) + p0;
}

/// Linear interpolation between two keys.
/// Does not perform range checking on \a currentTime.
FX_INLINE FxReal FX_CALL FxLinearInterpolate( const FxAnimKey& first, 
									          const FxAnimKey& second,
									          FxReal currentTime )
{
	FxReal time1 = first.GetTime();
	FxReal time2 = second.GetTime();
	FxReal deltaTime = time2 - time1;
	FxReal parametricTime = (currentTime - time1) / deltaTime;

	FxReal p0 = first.GetValue();
	FxReal p1 = second.GetValue();

	return (p1 - p0) * parametricTime + p0;
}

} // namespace Face

} // namespace OC3Ent

#endif
