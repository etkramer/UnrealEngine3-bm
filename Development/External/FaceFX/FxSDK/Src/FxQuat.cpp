//------------------------------------------------------------------------------
// This class implements a quaternion.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxQuat.h"

namespace OC3Ent
{

namespace Face
{

FxArchive& operator<<( FxArchive& arc, FxQuat& quat )
{
	return arc << quat.w << quat.x << quat.y << quat.z;
}

} // namespace Face

} // namespace OC3Ent
