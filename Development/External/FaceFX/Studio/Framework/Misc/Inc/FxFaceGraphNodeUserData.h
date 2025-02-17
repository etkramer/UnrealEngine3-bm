//------------------------------------------------------------------------------
// The user data attached to an FxFaceGraphNode.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxFaceGraphNodeUserData_H__
#define FxFaceGraphNodeUserData_H__

#include "FxPlatform.h"
#include "FxPrimitives.h"
#include "FxArray.h"
#include "FxName.h"

namespace OC3Ent
{

namespace Face
{

struct FxLinkEnd
{
	FxLinkEnd( FxBool input = FxFalse, FxLinkEnd* d = NULL )
		: selected( FxFalse )
		, isInput( input )
		, dest( d )
	{
	}

	FxBool      selected;
	FxBool		isInput;
	FxLinkEnd*	dest;
	FxName		otherNode;
	FxRect		bounds;
};

struct FxNodeUserData
{
	FxNodeUserData()
		: selected( FxFalse )
	{
	}

	FxRect bounds;
	FxBool selected;
	FxArray<FxLinkEnd*> inputLinkEnds;
	FxArray<FxLinkEnd*> outputLinkEnds;
};

#define GetNodeUserData( node ) reinterpret_cast<OC3Ent::Face::FxNodeUserData*>( (*node).GetUserData() )

} // namespace Face

} // namespace OC3Ent

#endif
