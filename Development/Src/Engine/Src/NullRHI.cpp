/*=============================================================================
	NullRHI.cpp: Null RHI implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

#if USE_DYNAMIC_RHI || USE_NULL_RHI

#include "NullRHI.h"

/**
 * Called at startup to initialize the null RHI.
 * @return The null RHI implementation if supported, otherwise NULL.
 */
FDynamicRHI* NullCreateRHI()
{
	GIsRHIInitialized = TRUE;
	return new FNullDynamicRHI();
}

/**
 * Return a shared large static buffer that can be used to return from any 
 * function that needs to return a valid pointer (but can be garbage data)
 */
void* FNullDynamicRHI::GetStaticBuffer()
{
	static void* Buffer = NULL;
	if (!Buffer)
	{
		// allocate a 4 meg buffer, should be big enough for any texture/surface
		Buffer = appMalloc(4 * 1024 * 1024);
	}

	return Buffer;
}

// needed for xenon compiling.  These will not be used but just need to exist for the externing we have

const INT GNumTilingModes = -1;
static const INT GMaxTiles = -1;
UINT GTilingMode = 0;

/** Value between 0-100 that determines the percentage of the vertical scan that is allowed to pass while still allowing us to swap when VSYNC'ed.
This is used to get the same behavior as the old *_OR_IMMEDIATE present modes. */
DWORD GPresentImmediateThreshold = 100;

/** The width of the D3D device's back buffer. */
INT GScreenWidth = 1280;

/** The height of the D3D device's back buffer. */
INT GScreenHeight= 720;

#if XBOX || PS3 || PLATFORM_UNIX
void appBeginDrawEvent(const FColor& Color,const TCHAR* Text)
{
}

/**
 * Ends the current PIX event
 */
void appEndDrawEvent(void)
{
}

/**
 * Platform specific function for setting the value of a counter that can be
 * viewed in PIX.
 */
void appSetCounterValue(const TCHAR* CounterName, FLOAT Value)
{
}

#endif // XBOX

void XePerformSwap( UBOOL bSyncToPresentationInterval, UBOOL bLockToVsync )
{
}

void* GRSXBaseAddress = 0;


#else

// Suppress linker warning "warning LNK4221: no public symbols found; archive member will be inaccessible"
INT NullRHILinkerHelper;

#endif // USE_DYNAMIC_RHI || USE_NULL_RHI
