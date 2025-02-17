//------------------------------------------------------------------------------
// This class defines an animation key.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxKey.h"

namespace OC3Ent
{

namespace Face
{

//------------------------------------------------------------------------------
// FxAnimKey
//------------------------------------------------------------------------------

FxAnimKey::FxAnimKey()
	: _time(0.0f)
	, _value(0.0f)
	, _slopeIn(0.0f)
	, _slopeOut(0.0f)
	, _userData(NULL)
{
}

FxAnimKey::FxAnimKey( FxReal iTime, FxReal iValue, 
					  FxReal iInSlope, FxReal iOutSlope )
	: _time(iTime)
	, _value(iValue)
	, _slopeIn(iInSlope)
	, _slopeOut(iOutSlope)
	, _userData(NULL)
{
}

FxAnimKey::FxAnimKey( const FxAnimKey& other )
	: _time(other._time)
	, _value(other._value)
	, _slopeIn(other._slopeIn)
	, _slopeOut(other._slopeOut)
	, _userData(other._userData)
{
}

FxAnimKey& FxAnimKey::operator=( const FxAnimKey& other )
{
	if( this == &other ) return *this;
	_time = other._time;
	_value = other._value;
	_slopeIn = other._slopeIn;
	_slopeOut = other._slopeOut;
	_userData = other._userData;
	return *this;
}

FxAnimKey::~FxAnimKey()
{
}

void FxAnimKey::SetTime( FxReal iTime )
{
	_time = iTime;
}

void FxAnimKey::SetValue( FxReal iValue )
{
	_value = iValue;
}

void FxAnimKey::SetSlopeIn( FxReal iInSlope )
{
	_slopeIn = iInSlope;
}

void FxAnimKey::SetSlopeOut( FxReal iOutSlope )
{
	_slopeOut = iOutSlope;
}

void* FxAnimKey::GetUserData( void )
{
	return _userData;
}

void FxAnimKey::SetUserData( void* userData )
{
	_userData = userData;
}

//@todo This is all legacy code meant only to load up old content, so it has not
//      been updated with new version serialization.
#define kCurrentFxAnimKeyVersion 0
FxArchive& operator<<( FxArchive& arc, FxAnimKey& key )
{
	FxUInt16 version = kCurrentFxAnimKeyVersion;
	arc << version;
    arc << key._time << key._value
		<< key._slopeIn << key._slopeOut;
	return arc;
}

} // namespace Face

} // namespace OC3Ent
