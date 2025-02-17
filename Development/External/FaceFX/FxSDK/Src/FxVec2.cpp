//------------------------------------------------------------------------------
// This class defines a vector in 2-space.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxVec2.h"

namespace OC3Ent
{

namespace Face
{

void FxVec2::SetLength( FxReal length )
{
	FxReal tempLength = Length();
	if( (tempLength != length) && (tempLength > 0.0f) )
	{
		FxReal r = length / tempLength;
		x *= r;
		y *= r;
	}
}

FxArchive& operator<<( FxArchive& arc, FxVec2& vec )
{
	return arc << vec.x << vec.y;
}

} // namespace Face

} // namespace OC3Ent
