//------------------------------------------------------------------------------
// This class implements a reference counted string.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxRefString.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxRefStringVersion 0

FX_IMPLEMENT_CLASS(FxRefString, kCurrentFxRefStringVersion, FxRefObject)

FxRefString::FxRefString()
{
}

FxRefString::FxRefString( const FxChar* string )
	: _string(string)
{
}

FxRefString::~FxRefString()
{
}

void FxRefString::SetString( const FxChar* string )
{
	_string = string;
}

void FxRefString::Serialize( FxArchive& FxUnused(arc) )
{
	FxAssert(!"Attempt to serialize FxRefString!");
}

} // namespace Face

} // namespace OC3Ent
