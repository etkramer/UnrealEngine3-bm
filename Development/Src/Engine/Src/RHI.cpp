/*=============================================================================
	RHI.cpp: Render Hardware Interface implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

//
// RHI globals.
//

UBOOL GIsRHIInitialized = FALSE;
INT GMaxTextureMipCount = MAX_TEXTURE_MIP_COUNT;
#if BATMAN
// BM1: Changed to 8 to match BM1
INT GMinTextureResidentMipCount = 8;
#else
INT GMinTextureResidentMipCount = 7;
#endif
UBOOL GSupportsDepthTextures = FALSE;
UBOOL GSupportsHardwarePCF = FALSE;
UBOOL GSupportsFetch4 = FALSE;
UBOOL GSupportsFPFiltering = TRUE;
UBOOL GSupportsQuads = FALSE;
UBOOL GUsesInvertedZ = FALSE;
FLOAT GPixelCenterOffset = 0.5f;
INT GMaxShadowDepthBufferSize = 2048;
INT GCurrentColorExpBias = 0;
UBOOL GUseTilingCode = FALSE;
UBOOL GUseMSAASplitScreen = FALSE;
INT GDrawUPVertexCheckCount = MAXINT;
INT GDrawUPIndexCheckCount = MAXINT;
UBOOL GSupportsVertexInstancing = FALSE;
UBOOL GSupportsEmulatedVertexInstancing = FALSE;
UBOOL GVertexElementsCanShareStreamOffset = TRUE;
UBOOL GTexturesCanShareMipMemory = FALSE;
UBOOL GModShadowsWithAlphaEmissiveBit = FALSE;

/* A global depth bias to use when user clip planes are enabled, to avoid z-fighting. */
FLOAT GDepthBiasOffset = 0.0f;

FVertexElementTypeSupportInfo GVertexElementTypeSupport;
