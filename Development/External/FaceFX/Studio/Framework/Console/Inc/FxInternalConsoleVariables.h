//------------------------------------------------------------------------------
// All built-in FaceFX console variables should be included here.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxInternalConsoleVariables_H__
#define FxInternalConsoleVariables_H__

#include "FxPlatform.h"

namespace OC3Ent
{

namespace Face
{

// Registers and unregisters all of the built-in FaceFX console variables.
class FxInternalConsoleVariables
{
public:
	// Register all internal console variables.
	static void RegisterAll( void );

	// Unregister all internal console variables.
	static void UnregisterAll( void );
};

} // namespace Face

} // namespace OC3Ent

#endif
