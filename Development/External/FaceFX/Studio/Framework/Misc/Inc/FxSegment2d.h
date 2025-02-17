//------------------------------------------------------------------------------
// Represents a line segment in two dimensions, providing a few useful functions
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.  All rights reserved.
//------------------------------------------------------------------------------

#ifndef FxSegment2d_H__
#define FxSegment2d_H__

#include "FxPlatform.h"
#include "FxVec2.h"

namespace OC3Ent
{

namespace Face
{

class FxSegment2d
{
public:
	FxSegment2d();
	FxSegment2d( const FxVec2& start, const FxVec2& end );
	FxSegment2d( const wxPoint& start, const wxPoint& end );

	/// Returns the start point of the segment
	FxVec2 GetStart() const;
	void SetStart( const wxPoint& start );
	/// Returns the end point of the segment
	FxVec2 GetEnd() const;
	void SetEnd( const wxPoint& end );

	/// Returns the distance from point to the line segment
	FxReal Distance( const FxVec2& point ) const;

	/// Returns the length of the line segment
	FxReal Length() const;

	FxReal DistanceToStartSquared( const FxVec2& point ) const;
	FxReal DistanceToEndSquared( const FxVec2& point ) const;

private:
	FxVec2 _start;
	FxVec2 _end;
};

} // namespace Face

} // namespace OC3Ent

#endif
