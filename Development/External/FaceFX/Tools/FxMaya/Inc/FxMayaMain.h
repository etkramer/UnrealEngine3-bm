//------------------------------------------------------------------------------
// The main module for the FaceFx Maya plug-in.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMayaMain_H__
#define FxMayaMain_H__

#include "FxToolLog.h"

// The current version of the plug-in.
#define kCurrentFxMayaVersion "1.7.2"

// Tell Maya to use the C++ standard <iostream> instead of <iostream.h>.
#define REQUIRE_IOSTREAM
// Tell Maya we have a built in bool type.
#define _BOOL

// The plug-in API on windows.
#ifdef WIN32
	#define FX_PLUGIN_API __declspec( dllexport )
#endif

#endif
