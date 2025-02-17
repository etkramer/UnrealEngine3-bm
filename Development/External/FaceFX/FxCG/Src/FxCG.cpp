//------------------------------------------------------------------------------
// The FaceFX coarticulation and gestures library.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxCG.h"
#include "FxCoarticulationConfig.h"
#include "FxGestureConfig.h"

namespace OC3Ent
{

namespace Face
{

/// The version number of the FaceFX coarticulation and gestures library.
/// This is the version number multiplied by 1000.  For example, version 1.0
/// is 1000, version 1.1 is 1100, and so on.
static const FxSize FX_CG_VERSION = 1000;

void FxCGStartup( void )
{
	FxCoarticulationConfig::StaticClass();
	FxGestureConfig::StaticClass();
}

void FxCGShutdown( void )
{
	// Nothing happens here yet.
}

FxSize FxCGGetVersion( void )
{
	return FX_CG_VERSION;
}

} // namespace Face

} // namespace OC3Ent
