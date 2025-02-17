//------------------------------------------------------------------------------
// This class implements a reference counted object.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxRefObject.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxRefObjectVersion 0

FX_IMPLEMENT_CLASS(FxRefObject, kCurrentFxRefObjectVersion, FxObject)

FxRefObject::FxRefObject()
	: _referenceCount(0)
{
}

FxRefObject::~FxRefObject()
{
}

void FxRefObject::Serialize( FxArchive& FxUnused(arc) )
{
	FxAssert(!"Attempt to serialize FxRefObject!");
}

} // namespace Face

} // namespace OC3Ent
