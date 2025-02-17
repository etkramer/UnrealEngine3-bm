/*=============================================================================
	UnFaceFXSupport.h: FaceFX support.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#if WITH_FACEFX

#include "../../../External/FaceFX/FxSDK/Inc/FxSDK.h"
#include "../../../External/FaceFX/FxSDK/Inc/FxMemory.h"
#include "../../../External/FaceFX/FxSDK/Inc/FxActor.h"
#include "../../../External/FaceFX/FxSDK/Inc/FxActorInstance.h"
#include "../../../External/FaceFX/FxSDK/Inc/FxArchiveStoreFile.h"
#include "../../../External/FaceFX/FxSDK/Inc/FxArchiveStoreMemory.h"

// Set this to 1 to log FaceFX performance timing data to the log.
#define LOG_FACEFX_PERF 0

// Initialize FaceFX.
void UnInitFaceFX( void );
// Shut down FaceFX.
void UnShutdownFaceFX( void );


#endif // WITH_FACEFX
