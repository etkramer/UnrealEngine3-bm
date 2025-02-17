/*=============================================================================
	XeD3DDrvPrivate.h: Private D3D RHI definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __XED3DDRVPRIVATE_H__
#define __XED3DDRVPRIVATE_H__

/*-----------------------------------------------------------------------------
	Definitions
-----------------------------------------------------------------------------*/

#define DEBUG_SHADERS							0

/*-----------------------------------------------------------------------------
	Includes
-----------------------------------------------------------------------------*/
#include "Engine.h"
#include "XeD3DDrv.h"

#if USE_XeD3D_RHI

/*-----------------------------------------------------------------------------
	Globals
-----------------------------------------------------------------------------*/

/** The global D3D device. */
extern IDirect3DDevice9* GDirect3DDevice;

/** The global D3D device's back buffer. */
extern IDirect3DSurface9* GD3DBackBuffer;

/** The global D3D device's 4X resolve source back buffer (overlapped with GD3DBackBuffer). */
extern IDirect3DSurface9* GD3DBackBufferResolveSource;

/** The global D3D device's front buffer texture */
extern IDirect3DTexture9* GD3DFrontBuffer;

/** A list of all viewport RHIs that have been created. */
extern TArray<FD3DViewport*> GD3DViewports;

/** The viewport which is currently being drawn. */
extern FD3DViewport* GD3DDrawingViewport;

/** TRUE If the currently set depth surface invertes the z-range to go from (1,0) */
extern UBOOL GInvertZ;

/**	Primary ring buffer size. D3D Default value is 32kB.				*/
extern INT GRingBufferPrimarySize;
/** Secondary ring buffer size. D3D Default value is 2MB.				*/
extern INT GRingBufferSecondarySize;
/**
 *	The number of segments the secondary ring buffer is partitioned
 *	into. When enough commands are added to the ring buffer to fill
 *	a segment, D3D kicks the segment to the device for processing.
 *	D3D Default value is 32.
 */
extern INT GRingBufferSegmentCount;


/*-----------------------------------------------------------------------------
	Global declarations
-----------------------------------------------------------------------------*/
extern void XeInitD3DDevice();
extern void XePerformSwap( UBOOL bSyncToPresentationInterval, UBOOL bLockToVsync=FALSE );

extern UINT XeEDRAMOffset( const TCHAR* Usage, UINT Size );
extern UINT XeHiZOffset( const TCHAR* Usage, UINT Size );
extern D3DFORMAT XeGetTextureFormat(BYTE Format,DWORD Flags);
extern D3DFORMAT XeGetRenderTargetFormat(BYTE Format);

/**
 * The XeD3D RHI stats.
 */
enum EXeD3DRHIStats
{
	STAT_CommandBufferBytesRemaining = STAT_XeRHIFirstStat,
	STAT_CommandBufferBytesUsed,
	STAT_CommandBufferBytesMinRemaining,
	STAT_CommandBufferBytesMaxUsed,
};

/**
 *	Check the config file for ring buffer parameters. If present, set them on the device.
 */
extern void XeSetRingBufferParametersFromConfig();

/** When TRUE, a gamma ramp will be set that compensates for the Xenon's default gamma ramp making darks too dark. */
extern UBOOL GUseCorrectiveGammaRamp;

/**
 * Enables or disables a gamma ramp that compensates for Xbox's default gamma ramp, which makes dark too dark.
 */
extern void XeSetCorrectiveGammaRamp(UBOOL On);

/**
 *	Set the ring buffer parameters
 *
 *	This function must be used if the ring buffer parameters are altered (rather
 *	than the IDirect3DDevice9 interface directly).
 *
 *	@param		PrimarySize			The size of the primary buffer
 *	@param		SecondarySize		The size of the secondary buffer
 *	@param		SegmentCount		The number of segments to divide the secondary buffer into
 *	@param		bCallDeviceSet		If TRUE, will call the SetRingBufferParameters function
 *									on the device. Otherwise, it will just do the allocations
 *									and match up the globals (intended primarily for 
 *									initializing the device).
 */
extern void XeSetRingBufferParameters(INT PrimarySize, INT SecondarySize, INT SegmentCount, 
									  UBOOL bCallDeviceSet = TRUE);

#endif

#endif
