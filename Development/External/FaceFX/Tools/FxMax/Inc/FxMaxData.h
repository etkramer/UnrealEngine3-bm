//------------------------------------------------------------------------------
// The static global data maintained by the FaceFx Max plug-in.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMaxData_H__
#define FxMaxData_H__

#include "FxToolMax.h"

namespace OC3Ent
{

namespace Face
{
	
// Static global data for the Max plug-in.
class FxMaxData
{
public:
	// The Max plug-in interface.
	static FxToolMax maxInterface;
};

} // namespace Face

} // namespace OC3Ent

#endif
