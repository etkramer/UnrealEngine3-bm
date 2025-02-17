#ifndef NX_EXTENSIONS_HPP
#define NX_EXTENSIONS_HPP
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "NxPhysics.h"


#define NXEXTENSIONSDLL_API NXPHYSXLOADERDLL_API

enum NxExtensionType
{
	NX_EXT_SCREEN_SURFACE,
	NX_EXT_PERFORMANCE_IDENTIFIER,
	NX_EXT_QUICK_LOAD,
	NX_EXT_LAST
};

class NxExtension
{
public:
  virtual NxExtensionType getExtensionType(void)    const = 0;
  virtual unsigned int    getExtensionVersion(void) const = 0;
};

/**
* Create an NxExtensions instance.
*/
NXEXTENSIONSDLL_API NxExtension * NX_CALL_CONV NxCreateExtension(NxExtensionType type);
NXEXTENSIONSDLL_API void         NX_CALL_CONV  NxReleaseExtension(NxExtension &ext);

#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
