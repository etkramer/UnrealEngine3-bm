/*=============================================================================
	XeD3DDrv.h: Public XeD3D RHI definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _INC_XeD3DDRV
#define _INC_XeD3DDRV

/* Define this to 1 and add xapilib.lib to the IgnoreDefaultLib to enable PIX debugging with PIX Events. */
/* MUST put this before the #if check so even NULL_RHI builds can include this file to get at this #define */
#if !FINAL_RELEASE
#define LINK_AGAINST_PROFILING_D3D_LIBRARIES	1
#else
#define LINK_AGAINST_PROFILING_D3D_LIBRARIES	0
#endif

#if USE_XeD3D_RHI

/*-----------------------------------------------------------------------------
	RHI defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
	Xenon D3D RHI public headers.
-----------------------------------------------------------------------------*/

// utility headers
#include "XeD3DUtil.h"

// resource headers
#include "XeD3DResources.h"
#include "XeD3DVertexDeclaration.h"
#include "XeD3DShaders.h"
#include "XeD3DVertexBuffer.h"
#include "XeD3DIndexBuffer.h"
#include "XeD3DTextureAllocator.h"
#include "XeD3DTexture.h"
#include "XeD3DState.h"
#include "XeD3DRenderTarget.h"
#include "XeD3DOcclusionQuery.h"
#include "XeD3DCommandList.h"

// device headers
#include "XeD3DViewport.h"


/*-----------------------------------------------------------------------------
	RHI functions that are unused on Xenon.
-----------------------------------------------------------------------------*/

inline void RHISetViewPixelParameters( const class FSceneView* View, FPixelShaderRHIParamRef PixelShader, const class FShaderParameter* SceneDepthCalcParameter, const class FShaderParameter* ScreenPositionScaleBiasParameter )
{
}
inline void RHIReduceTextureCachePenalty( FPixelShaderRHIParamRef PixelShader )
{
}


#endif

#endif
