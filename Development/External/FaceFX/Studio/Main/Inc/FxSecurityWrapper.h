//------------------------------------------------------------------------------
// A wrapper for determining whether or not to turn on the security code.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxSecurityWrapper_H__
#define FxSecurityWrapper_H__

#ifdef FX_ENABLE_SECURITY
	#include "FxSecurity.h"
#else
	#include "FxSecurityDefinesOnly.h"
#endif

#endif
