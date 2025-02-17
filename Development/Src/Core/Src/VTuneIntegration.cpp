/*=============================================================================
	VTuneIntegration.cpp: Provides integrated VTune timing support.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"
#include "VTuneIntegration.h"


/** Static: True if we're currently initialized (or have attempted to initialize, since startup) */
UBOOL FVTuneWrapper::bHasInitialized = FALSE;

/** Static: DLL handle for VTuneAPI.dll (HMODULE) */
void* FVTuneWrapper::VTuneAPIDLLInterface = NULL;

/** Static: VTPause() function pointer */
FVTuneWrapper::VTPauseFunctionPtr FVTuneWrapper::VTPauseFunction = NULL;

/** Static: VTResume() function pointer */
FVTuneWrapper::VTResumeFunctionPtr FVTuneWrapper::VTResumeFunction = NULL;

/** Static: Number of timers currently running.  VTune timers are always 'global inclusive'. */
INT FVTuneWrapper::TimerCount = 0;

/** Static: Whether or not VTune is currently paused (as far as we know.) */
UBOOL FVTuneWrapper::bIsPaused = TRUE;



/**
 * Initializes FVTuneWrapper
 */
void FVTuneWrapper::InitializeOnDemand()
{
	if( !bHasInitialized )
	{
		check( VTuneAPIDLLInterface == NULL );

// VTune integration must be enabled in UnBuild.h
// Also, VTune functionality is currently only supported on the Windows platform
#if WITH_VTUNE

		// Try to load the VTune DLL
		VTuneAPIDLLInterface = appGetDllHandle( TEXT( "VtuneApi.dll" ) );
		if( VTuneAPIDLLInterface != NULL )
		{
			// Get API function pointers of interest
			{
				// "VTPause"
				VTPauseFunction = ( VTPauseFunctionPtr )appGetDllExport( VTuneAPIDLLInterface, TEXT( "VTPause" ) );
				if( VTPauseFunction == NULL )
				{
					// Try decorated version of this function
					VTPauseFunction = ( VTPauseFunctionPtr )appGetDllExport( VTuneAPIDLLInterface, TEXT( "_VTPause@0" ) );
				}

				// "VTResume"
				VTResumeFunction = ( VTResumeFunctionPtr )appGetDllExport( VTuneAPIDLLInterface, TEXT( "VTResume" ) );
				if( VTResumeFunction == NULL )
				{
					// Try decorated version of this function
					VTResumeFunction = ( VTResumeFunctionPtr )appGetDllExport( VTuneAPIDLLInterface, TEXT( "_VTResume@0" ) );
				}
			}

			if( VTPauseFunction == NULL || VTResumeFunction == NULL )
			{
				// Couldn't find the functions we need.  VTune support will not be active.
				appFreeDllHandle( VTuneAPIDLLInterface );
				VTuneAPIDLLInterface = NULL;
				VTPauseFunction = NULL;
				VTResumeFunction = NULL;
			}
		}

		// Don't silently fail but rather warn immediately as no data will be collected.
		if( VTPauseFunction == NULL || VTResumeFunction == NULL )
		{
			appMsgf( AMT_OK, TEXT("Failed to initialize VTuneApi.dll") );
		}

#endif // WITH_VTUNE && __WIN32__

		// We've initialized (or at least tried to!)
		bHasInitialized = TRUE;

		// At this point we don't know whether VTune is capturing or not (and there's no way to query this),
		// so we'll assume that we're paused.  The first time a 'scope' object is used it will always either
		// pause or unpause VTune, so we're guaranteed to be in sync afterwards.
		bIsPaused = TRUE;
	}
}
