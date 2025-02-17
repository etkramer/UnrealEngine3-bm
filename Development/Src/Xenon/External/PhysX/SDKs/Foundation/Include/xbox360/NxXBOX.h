#ifndef NXXBOX_H
#define NXXBOX_H
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#define DEFAULT_STACK_SIZE 262144
#define NOMINMAX
#ifndef _XBOX_MINMAX
	#define _XBOX_MINMAX
	// Xbox 360 XDK 3215 (June 06) requires that this version of min and max
	// be defined in order for xmconvert.inl to build
	inline unsigned int max(unsigned int a, unsigned int b) { (((a) > (b)) ? (a) : (b)); }
	inline unsigned int min(unsigned int a, unsigned int b) { (((a) < (b)) ? (a) : (b)); }
#endif
#include<xtl.h>


#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
