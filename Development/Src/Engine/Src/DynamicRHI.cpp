/*=============================================================================
	DynamicRHI.cpp: Dynamically bound Render Hardware Interface implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

#if USE_DYNAMIC_RHI

// External dynamic RHI factory functions.
extern FDynamicRHI* NullCreateRHI();

#if _WINDOWS
extern FDynamicRHI* D3D9CreateRHI();
extern FDynamicRHI* D3D10CreateRHI();
extern UBOOL IsDirect3D10Supported();
#endif

// Globals.
FDynamicRHI* GDynamicRHI = NULL;

void RHIInit()
{
	if(!GDynamicRHI)
	{	
		if(ParseParam(appCmdLine(),TEXT("nullrhi")) || USE_NULL_RHI || GIsUCC)
		{
			// Use the null RHI if it was specified during compile or on the command line, or if a commandlet is running.
			GDynamicRHI = NullCreateRHI();
		}
#if _WINDOWS
		else
		{
			// Try to create the D3D10 RHI if it's supported and either allowed by the system settings or specified on the command-line.
			// For the moment, only use D3D10 if it's explicitly requested.
			UBOOL bAllowD3D10 = GSystemSettings.bAllowD3D10 || ParseParam(appCmdLine(),TEXT("d3d10"));
			if(bAllowD3D10 && IsDirect3D10Supported())
			{
				GDynamicRHI = D3D10CreateRHI();
			}
			else
			{
				// If D3D10 wasn't desired and supported, try the D3D9 RHI.
				GDynamicRHI = D3D9CreateRHI();
			}
		}
#endif // _WINDOWS

		check(GDynamicRHI);
	}
}

void RHIExit()
{
	// Destruct the dynamic RHI.
	delete GDynamicRHI;
	GDynamicRHI = NULL;
}


#else

// Suppress linker warning "warning LNK4221: no public symbols found; archive member will be inaccessible"
INT DynamicRHILinkerHelper;

#endif // USE_DYNAMIC_RHI


#if !CONSOLE || USE_NULL_RHI

/**
 * Defragment the texture pool.
 */
void appDefragmentTexturePool()
{
}

/**
 * Log the current texture memory stats.
 *
 * @param Message	This text will be included in the log
 */
void appDumpTextureMemoryStats(const TCHAR* /*Message*/)
{
}

#endif	//#if !CONSOLE
