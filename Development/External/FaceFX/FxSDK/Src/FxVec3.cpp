//------------------------------------------------------------------------------
// This class defines a vector in 3-space.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxVec3.h"

namespace OC3Ent
{

namespace Face
{

void FxVec3::SetLength( FxReal length )
{
	FxReal tempLength = Length();
	if( (tempLength != length) && (tempLength > 0.0f) )
	{
		FxReal r = length / tempLength;
		x *= r;
		y *= r;
		z *= r;
	}
}

FxArchive& operator<<( FxArchive& arc, FxVec3& vec )
{
	return arc << vec.x << vec.y << vec.z;
}

} // namespace Face

} // namespace OC3Ent
