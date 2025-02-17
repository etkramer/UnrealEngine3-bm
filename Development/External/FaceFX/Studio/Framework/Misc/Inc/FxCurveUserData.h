//------------------------------------------------------------------------------
// User data structures for FxCurves.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCurveUserData_H__
#define FxCurveUserData_H__

#include "FxPlatform.h"

namespace OC3Ent
{

namespace Face
{

#define KeyIsSelected  1
#define KeyIsPivot     2
#define TangentsLocked 4
#define KeyIsTemporary 8

class FxKeyUserData 
{
public:
	// Constructor.
	FxKeyUserData( FxInt32 iflags = TangentsLocked );
	// Destructor.
	~FxKeyUserData();

	FxInt32 flags;
	wxPoint incomingTanHandle;
	wxPoint outgoingTanHandle;
};

class FxCurveUserData
{
public:
	// Constructor.
	FxCurveUserData();
	// Destructor.
	~FxCurveUserData();

	FxBool  isVisible;
};

} // namespace Face

} // namespace OC3Ent

#endif
