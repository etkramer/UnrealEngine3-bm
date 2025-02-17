/*=============================================================================
	D3D10DrvPrivate.h: Private D3D RHI definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __D3D10DRVPRIVATE_H__
#define __D3D10DRVPRIVATE_H__

// Definitions.
#define DEBUG_SHADERS 0

// Dependencies
#include "Engine.h"
#include "D3D10Drv.h"

/**
 * The D3D RHI stats.
 */
enum ED3D10RHIStats
{
	STAT_D3D10PresentTime = STAT_D3D10RHIFirstStat,
	STAT_D3D10DrawPrimitiveCalls,
	STAT_D3D10Triangles,
	STAT_D3D10Lines,
	STAT_D3D10CreateTextureTime,
	STAT_D3D10LockTextureTime,
	STAT_D3D10UnlockTextureTime,
	STAT_D3D10CopyTextureTime,
	STAT_D3D10CopyMipToMipAsyncTime,
	STAT_D3D10UploadTextureMipTime,
	STAT_D3D10CreateBoundShaderStateTime,
	STAT_D3D10ConstantBufferUpdateTime,
};

#endif
