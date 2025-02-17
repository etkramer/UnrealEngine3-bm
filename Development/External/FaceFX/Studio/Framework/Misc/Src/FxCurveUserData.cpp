//------------------------------------------------------------------------------
// User data structures for FxCurves.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxCurveUserData.h"

namespace OC3Ent
{

namespace Face
{

FxKeyUserData::FxKeyUserData( FxInt32 iflags )
	: flags( iflags )
	, incomingTanHandle( wxPoint(0,0) )
	, outgoingTanHandle( wxPoint(0,0) )
{
}

FxKeyUserData::~FxKeyUserData()
{
}

FxCurveUserData::FxCurveUserData()
	: isVisible( FxFalse )
{
}

FxCurveUserData::~FxCurveUserData()
{
}

} // namespace Face

} // namespace OC3Ent
