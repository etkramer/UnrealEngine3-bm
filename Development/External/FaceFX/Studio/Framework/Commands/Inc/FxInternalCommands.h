//------------------------------------------------------------------------------
// All built-in FaceFX commands should be included here.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxInternalCommands_H__
#define FxInternalCommands_H__

#include "FxPlatform.h"

namespace OC3Ent
{

namespace Face
{

// Registers and unregisters all of the built-in FaceFX commands.
class FxInternalCommands
{
public:
	// Register all internal commands.
	static void RegisterAll( void );

	// Unregister all internal commands.
	static void UnregisterAll( void );
};

} // namespace Face

} // namespace OC3Ent

#endif
