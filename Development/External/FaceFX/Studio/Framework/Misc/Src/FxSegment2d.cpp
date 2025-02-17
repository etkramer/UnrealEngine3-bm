//------------------------------------------------------------------------------
// Represents a line segment in two dimensions, providing a few useful functions
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.  All rights reserved.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxSegment2d.h"

namespace OC3Ent
{

namespace Face
{

FxSegment2d::FxSegment2d()
{
}

FxSegment2d::FxSegment2d( const FxVec2& start, const FxVec2& end )
	: _start( start )
	, _end( end )
{
}

FxSegment2d::FxSegment2d( const wxPoint& start, const wxPoint& end )
{
	_start.x = start.x;
	_start.y = start.y;
	_end.x   = end.x;
	_end.y   = end.y;
}

FxVec2 FxSegment2d::GetStart() const
{ 
	return _start;
}

void FxSegment2d::SetStart( const wxPoint& start )
{
	_start.x = start.x;
	_start.y = start.y;
}

FxVec2 FxSegment2d::GetEnd() const
{
	return _end;
}

void FxSegment2d::SetEnd( const wxPoint& end )
{
	_end.x = end.x;
	_end.y = end.y;
}


FxReal FxSegment2d::Distance( const FxVec2& point ) const
{
	if( _start == _end ) return FxInvalidValue;

	FxReal invSegmentLengthSquared = 1 / ( _end - _start ).LengthSquared();

	FxReal u = ( ( ( point.x - _start.x ) * ( _end.x - _start.x ) ) +
		( ( point.y - _start.y ) * ( _end.y - _start.y ) ) ) * invSegmentLengthSquared;

	if( u < 0.0f || u > 1.0f ) return FxInvalidValue;

	FxVec2 intersection( _start.x + u * ( _end.x - _start.x ),
						  _start.y + u * ( _end.y - _start.y ) );

	return ( point - intersection ).Length();
}

FxReal FxSegment2d::Length() const
{
	return ( _end - _start).Length();
}

FxReal FxSegment2d::DistanceToStartSquared( const FxVec2& point ) const
{
	return ( point - _start).LengthSquared(); 
}

FxReal FxSegment2d::DistanceToEndSquared( const FxVec2& point ) const
{
	return ( point - _end).LengthSquared();
}


} // namespace Face

} // namespace OC3Ent
