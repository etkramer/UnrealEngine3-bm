//------------------------------------------------------------------------------
// The static global data maintained by the FaceFx Maya plug-in.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMayaData_H__
#define FxMayaData_H__

#include "FxToolMaya.h"

namespace OC3Ent
{

namespace Face
{

// Static global data for the Maya plug-in.
class FxMayaData
{
public:
	// The Maya plug-in interface.
	static FxToolMaya mayaInterface;
};

} // namespace Face

} // namespace OC3Ent

#endif

