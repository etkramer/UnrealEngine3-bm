//------------------------------------------------------------------------------
// The FaceFX coarticulation and gestures library.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCG_H__
#define FxCG_H__

#include "FxPlatform.h"
#include "FxString.h"

namespace OC3Ent
{

namespace Face
{

/// Starts up and initializes the FaceFX coarticulation and gestures library.
/// This must be called exactly once in your application.
void FxCGStartup( void );
/// Shuts down the FaceFX coarticulation and gestures library.
/// This must be called exactly once in your application.
void FxCGShutdown( void );

/// Returns the current FaceFX coarticulation and gestures library version 
/// number.  This version number has been multiplied by 1000, so for example 
/// version 1.0 returns 1000, version 1.1 returns 1100, and so on.
FxSize FxCGGetVersion( void );

} // namespace Face

} // namespace OC3Ent

#endif
