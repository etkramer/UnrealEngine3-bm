/*=============================================================================
	D3DViewport.cpp: D3D viewport RHI implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "XeD3DDrvPrivate.h"

#if USE_XeD3D_RHI
#include "ChartCreation.h"

/*-----------------------------------------------------------------------------
	Defines.
-----------------------------------------------------------------------------*/

#define LOG_CPU_BLOCKING_ON_GPU 0

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

/** The global D3D device. */
IDirect3DDevice9* GDirect3DDevice;

/** The global D3D device's back buffer. */
IDirect3DSurface9* GD3DBackBuffer;

/** The global D3D device's 4X resolve source back buffer (overlapped with GD3DBackBuffer). */
IDirect3DSurface9* GD3DBackBufferResolveSource;

/** The global D3D device's front buffer texture */
IDirect3DTexture9* GD3DFrontBuffer;

/** The viewport RHI which is currently fullscreen. */
FD3DViewport* GD3DFullscreenViewport = NULL;

/** True if the currently set depth surface invertes the z-range to go from (1,0) */
UBOOL GInvertZ = FALSE;

/** The width of the D3D device's back buffer. */
INT GScreenWidth = 1280;

/** The height of the D3D device's back buffer. */
INT GScreenHeight= 720;

/** True if the D3D device is in fullscreen mode. */
UBOOL GD3DIsFullscreenDevice = FALSE;

/** True if the application has lost D3D device access. */
UBOOL GD3DDeviceLost = FALSE;

/** Whether we're allowed to swap/ resolve */
UBOOL GAllowBufferSwap = TRUE;

/** Value between 0-100 that determines the percentage of the vertical scan that is allowed to pass while still allowing us to swap when VSYNC'ed.
    This is used to get the same behavior as the old *_OR_IMMEDIATE present modes. */
DWORD GPresentImmediateThreshold = 100;

/** Value between 0-100 that determines the percentage of the vertical scan that is allowed to pass.  This used when we are locking fully to the VSYNC. */
DWORD GPresentLockThreshold = 10;

/** Amount of time the CPU is stalled waiting on the GPU */
FLOAT GCPUWaitingOnGPUTime = 0.0f;
/** Amount of time the CPU was stalled waiting on the GPU the previous frame. */
FLOAT GCPUWaitingOnGPUTimeLastFrame = 0.0f;

/** When TRUE, a gamma ramp will be set that compensates for the Xenon's default gamma ramp making darks too dark. */
UBOOL GUseCorrectiveGammaRamp = FALSE;

/**
 *	RingBuffer Information
 *
 *	These are the default values, unless they are found in the Engine.ini file.
 *	NOTE: If the application calls IDirect3DDevice9::SetRingBufferParameters
 *		  anywhere in the application, these need to be updated to the values
 *		  set.
 *
 /
/**	Primary ring buffer size. D3D Default value is 32kB.				*/
INT GRingBufferPrimarySize = 32 * 1024;
/** Secondary ring buffer size. D3D Default value is 2MB.				*/
INT GRingBufferSecondarySize = 3 * 1024 * 1024;
/**
 *	The number of segments the secondary ring buffer is partitioned
 *	into. When enough commands are added to the ring buffer to fill
 *	a segment, D3D kicks the segment to the device for processing.
 *	D3D Default value is 32.
 */
INT GRingBufferSegmentCount = 48;

/**
 * Callback function called whenever D3D blocks CPU till GPU is ready.
 */
void XeBlockCallback( DWORD Flags, D3DBLOCKTYPE BlockType, FLOAT ClockTime, DWORD ThreadTime )
{
	GCPUWaitingOnGPUTime += ClockTime;
#if LOOKING_FOR_PERF_ISSUES || LOG_CPU_BLOCKING_ON_GPU
	if( BlockType != D3DBLOCKTYPE_SWAP_THROTTLE )
	{
		debugf(NAME_PerfWarning,TEXT("CPU waiting for GPU: Blocked=%5.2f ms, Spun=%i ms, Event=%i"), ClockTime, ThreadTime, (INT)BlockType );
	}
#endif
}

void RHIInit()
{
	if( !GIsRHIInitialized )
	{
		// Set RHI capabilities.
		GSupportsEmulatedVertexInstancing = TRUE;
		GSupportsDepthTextures = TRUE;
		GUsesInvertedZ = TRUE;
		GUseTilingCode = TRUE;
		GDrawUPVertexCheckCount = 400 * 1024;
		GDrawUPIndexCheckCount = 400 * 1024;
		GRHIShaderPlatform = SP_XBOXD3D;
		/*
		* Shadow buffer size 
		* 2 x 1280x720 x 4byte z-buffer and lighting targets = 7200 KB EDRAM
		* allows for max 864x864 = 2916 KB EDRAM shadow buffer (since we only store depths)
		* But since we are overlapping the shadow depth texture with light attenuation,
		* We must limit the size to 800 since light attenuation is at a minimum: 960 * 720 * 4
		*/
		GMaxShadowDepthBufferSize = 800;
		/** On 360 always use the unused alpha bit for storing emissive mask */
		GModShadowsWithAlphaEmissiveBit = TRUE;
	}
}

/**
 * Initializes the D3D device
 */
void XeInitD3DDevice()
{
	// Ensure the capabilities are set.
	RHIInit();

	// Read the selected video mode and adjust the backbuffer size/aspect ratio
	// accordingly (TCRs 018 & 019)
	XVIDEO_MODE VideoMode;
	appMemzero(&VideoMode,sizeof(XVIDEO_MODE));
	XGetVideoMode(&VideoMode);

	FLOAT AspectRatio = VideoMode.fIsWideScreen || VideoMode.dwDisplayHeight > 720 ? 16.f / 9.f :
		((FLOAT)VideoMode.dwDisplayWidth / (FLOAT)VideoMode.dwDisplayHeight);
	GScreenHeight = 720;
	GScreenWidth = Clamp(
		// Width is height times aspect ratio (900 = 5x4, 960 = 4x3, 1280 = 16x9)
		appTrunc(720.f * AspectRatio),
		// Min is 5x4 aspect ration
		900,
		// Max is 16x9 resolution
		1280);

	// Do not use tiling when the backbuffer isn't 1280
	if (GScreenWidth != 1280)
	{
		GUseTilingCode = FALSE;
	}

	// Always assume vsync is enabled on console and ignore config option loaded by GSystemSettings
	// Disable VSYNC if -novsync is on the command line.  
	GSystemSettings.bUseVSync = !ParseParam(appCmdLine(), TEXT("novsync"));

	D3DPRESENT_PARAMETERS PresentParameters		= { 0 };
	PresentParameters.BackBufferWidth			= GScreenWidth;
	PresentParameters.BackBufferHeight			= GScreenHeight;
	PresentParameters.SwapEffect				= D3DSWAPEFFECT_DISCARD; 
	PresentParameters.EnableAutoDepthStencil	= FALSE;
	PresentParameters.DisableAutoBackBuffer		= TRUE;
	PresentParameters.DisableAutoFrontBuffer	= TRUE;	
	PresentParameters.PresentationInterval		= GSystemSettings.bUseVSync ? D3DPRESENT_INTERVAL_TWO : D3DPRESENT_INTERVAL_IMMEDIATE;
	PresentParameters.hDeviceWindow				= NULL;
	
	// Ring buffer
	PresentParameters.RingBufferParameters.Flags = 0;
	PresentParameters.RingBufferParameters.PrimarySize = GRingBufferPrimarySize;
	PresentParameters.RingBufferParameters.SecondarySize = GRingBufferSecondarySize;
	PresentParameters.RingBufferParameters.SegmentCount = GRingBufferSegmentCount;

	// Create D3D device.
	VERIFYD3DRESULT( Direct3DCreate9(D3D_SDK_VERSION)->CreateDevice( 
		D3DADAPTER_DEFAULT, 
		D3DDEVTYPE_HAL, 
		NULL, 
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE | D3DCREATE_BUFFER_2_FRAMES, 
		&PresentParameters, 
		&GDirect3DDevice ));

	// Create backbuffer.
	D3DSURFACE_PARAMETERS BackBufferParameters	= { 0 };
	D3DFORMAT BackBufferFormat = D3DFMT_A8R8G8B8;  
	BackBufferParameters.Base = XeEDRAMOffset( TEXT("DefaultBB"), XGSurfaceSize(GScreenWidth, GScreenHeight, BackBufferFormat, D3DMULTISAMPLE_NONE) );
	VERIFYD3DRESULT( GDirect3DDevice->CreateRenderTarget( GScreenWidth, GScreenHeight, BackBufferFormat, D3DMULTISAMPLE_NONE, 0, 0, &GD3DBackBuffer, &BackBufferParameters ) );

	// Create overlaid resolve source texture
	D3DSURFACE_PARAMETERS ResolveBufferParameters = BackBufferParameters;
	ResolveBufferParameters.Base = XeEDRAMOffset( TEXT("DefaultColor"), XGSurfaceSize(GScreenWidth, GScreenHeight, D3DFMT_A2B10G10R10, D3DMULTISAMPLE_NONE) );
	ResolveBufferParameters.ColorExpBias = 0;
	VERIFYD3DRESULT( GDirect3DDevice->CreateRenderTarget( GScreenWidth/2, GScreenHeight/2, D3DFMT_A2B10G10R10F_EDRAM, D3DMULTISAMPLE_4_SAMPLES, 0, 0, &GD3DBackBufferResolveSource, &ResolveBufferParameters ) );

	// Create front buffer texture.
	D3DFORMAT FrontBufferFormat = D3DFMT_LE_X8R8G8B8;
	VERIFYD3DRESULT( GDirect3DDevice->CreateTexture( GScreenWidth, GScreenHeight, 1, 0, FrontBufferFormat, 0, &GD3DFrontBuffer, NULL ) );  

	// Set back and depth buffer.
	GDirect3DDevice->SetRenderTarget( 0, GD3DBackBuffer );
	GDirect3DDevice->SetDepthStencilSurface( NULL );

	// Default D3D half pixel offset
	GDirect3DDevice->SetRenderState( D3DRS_HALFPIXELOFFSET, FALSE );

	// Initialize the platform pixel format map.
	GPixelFormats[ PF_Unknown		].PlatformFormat	= D3DFMT_UNKNOWN;
	GPixelFormats[ PF_A32B32G32R32F	].PlatformFormat	= D3DFMT_A32B32G32R32F;
	GPixelFormats[ PF_A8R8G8B8		].PlatformFormat	= D3DFMT_A8R8G8B8;
	GPixelFormats[ PF_G8			].PlatformFormat	= D3DFMT_L8;
	GPixelFormats[ PF_G16			].PlatformFormat	= D3DFMT_UNKNOWN;	// Not supported for rendering.
	GPixelFormats[ PF_DXT1			].PlatformFormat	= D3DFMT_DXT1;
	GPixelFormats[ PF_DXT3			].PlatformFormat	= D3DFMT_DXT3;
	GPixelFormats[ PF_DXT5			].PlatformFormat	= D3DFMT_DXT5;
	GPixelFormats[ PF_UYVY			].PlatformFormat	= D3DFMT_UYVY;	
	GPixelFormats[ PF_FloatRGB		].PlatformFormat	= D3DFMT_A16B16G16R16F;
	GPixelFormats[ PF_FloatRGB		].Supported			= 1;
	GPixelFormats[ PF_FloatRGB		].BlockBytes		= 8;
	GPixelFormats[ PF_FloatRGBA		].PlatformFormat	= D3DFMT_A16B16G16R16F_EXPAND;
	GPixelFormats[ PF_FloatRGBA		].Supported			= 1;
	GPixelFormats[ PF_FloatRGBA		].BlockBytes		= 8;
	GPixelFormats[ PF_DepthStencil	].PlatformFormat	= GUsesInvertedZ ? D3DFMT_D24FS8 : D3DFMT_D24S8;
	GPixelFormats[ PF_DepthStencil	].BlockBytes		= 4;
	GPixelFormats[ PF_ShadowDepth	].PlatformFormat	= D3DFMT_D24X8;
	GPixelFormats[ PF_ShadowDepth	].BlockBytes		= 4;
	GPixelFormats[ PF_R32F			].PlatformFormat	= D3DFMT_R32F;
	GPixelFormats[ PF_G16R16		].PlatformFormat	= D3DFMT_G16R16;
	GPixelFormats[ PF_G16R16F		].PlatformFormat	= D3DFMT_G16R16F;
	GPixelFormats[ PF_G16R16F_FILTER].PlatformFormat	= D3DFMT_G16R16F_EXPAND;
	GPixelFormats[ PF_G32R32F		].PlatformFormat	= D3DFMT_G32R32F;
	GPixelFormats[ PF_A2B10G10R10   ].PlatformFormat    = D3DFMT_A2B10G10R10;
	GPixelFormats[ PF_A16B16G16R16  ].PlatformFormat	= D3DFMT_A16B16G16R16;
	GPixelFormats[ PF_R16F			].PlatformFormat	= D3DFMT_R16F;
	GPixelFormats[ PF_R16F_FILTER	].PlatformFormat	= D3DFMT_R16F_EXPAND;

	// Notify all initialized FRenderResources that there's a valid RHI device to create their RHI resources for now.
	for(TLinkedList<FRenderResource*>::TIterator ResourceIt(FRenderResource::GetResourceList());ResourceIt;ResourceIt.Next())
	{
		ResourceIt->InitDynamicRHI();
	}
	for(TLinkedList<FRenderResource*>::TIterator ResourceIt(FRenderResource::GetResourceList());ResourceIt;ResourceIt.Next())
	{
		ResourceIt->InitRHI();
	}

	// Register callback function to look into CPU blocking operations.
	GDirect3DDevice->SetBlockCallback( 0, XeBlockCallback );

	// Ensure that the buffer is clear
	GDirect3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0);
	XePerformSwap(TRUE);

	// RHI device is now valid
	GIsRHIInitialized = TRUE;
}

#if STATS
#define PERFBUFFERSIZE	2
static D3DPerfCounters *GPerfCounterBeginFrame[PERFBUFFERSIZE];
static D3DPerfCounters *GPerfCounterEndFrame[PERFBUFFERSIZE];
static UINT GSwapFrameCounter = 0;
#endif

/**
 * Helper function performing tasks involved in advancing the presented frame by swapping buffers.
 *
 * @param bSyncToPresentationInterval	whether we should sync to the presentation interval defined when creating the device
 */
void XePerformSwap( UBOOL bSyncToPresentationInterval, UBOOL bLockToVsync )
{
#if LINK_AGAINST_PROFILING_D3D_LIBRARIES && STATS && !LOG_CPU_BLOCKING_ON_GPU
	if ( GSwapFrameCounter == 0 )
	{
		D3DPERFCOUNTER_EVENTS GPerfCounterEvents = { { GPUPE_CP_COUNT } };
		GPerfCounterEvents.CP[0] = GPUPE_CSF_RB_I1_I2_FETCHING;
		GDirect3DDevice->EnablePerfCounters( TRUE );
		GDirect3DDevice->SetPerfCounterEvents( &GPerfCounterEvents, 0 );
		for ( INT PerfCounterIndex=0; PerfCounterIndex < PERFBUFFERSIZE; ++PerfCounterIndex )
		{
			GDirect3DDevice->CreatePerfCounters( &GPerfCounterBeginFrame[PerfCounterIndex], 1 );
			GDirect3DDevice->CreatePerfCounters( &GPerfCounterEndFrame[PerfCounterIndex], 1 );
		}
	}

	INT PerfIndex = GSwapFrameCounter % PERFBUFFERSIZE;
	GDirect3DDevice->QueryPerfCounters( GPerfCounterEndFrame[PerfIndex], 0 );

	PerfIndex = ++GSwapFrameCounter % PERFBUFFERSIZE;
	if ( GSwapFrameCounter > PERFBUFFERSIZE )
	{
		D3DPERFCOUNTER_VALUES PerfValuesBeginFrame, PerfValuesEndFrame;
		GPerfCounterBeginFrame[PerfIndex]->GetValues( &PerfValuesBeginFrame, 0, NULL );
		GPerfCounterEndFrame[PerfIndex]->GetValues( &PerfValuesEndFrame, 0, NULL );

 		// Calculate time using RBBM; GPU utilization using Ring data availability
 		DWORD TotalGPUFrameTime = DWORD( (PerfValuesEndFrame.RBBM[0].QuadPart - PerfValuesBeginFrame.RBBM[0].QuadPart)*GCyclesPerSecond/500000000ull );
 		GGPUFrameTime = DWORD( (PerfValuesEndFrame.CP[0].QuadPart - PerfValuesBeginFrame.CP[0].QuadPart)*GCyclesPerSecond/500000000ull );
 
 		SET_FLOAT_STAT(STAT_GPUWaitingOnCPU, ((float)(TotalGPUFrameTime - GGPUFrameTime)) / GCyclesPerSecond * 1000.0f);
 		SET_FLOAT_STAT(STAT_CPUWaitingOnGPU, GCPUWaitingOnGPUTime); 
 		GCPUWaitingOnGPUTimeLastFrame = GCPUWaitingOnGPUTime;
		GCPUWaitingOnGPUTime = 0.0f;
	}
#else
	GCPUWaitingOnGPUTimeLastFrame = GCPUWaitingOnGPUTime;
	GCPUWaitingOnGPUTime = 0.0f;
#endif

	extern UBOOL GD3DRenderingIsSuspended;
	check(GD3DRenderingIsSuspended == FALSE);

	// Stop CPU trace if, and only if, one was kicked off below.
	static FName NameRender( TEXT("Render"), FNAME_Add );
	appStopCPUTrace( NameRender );

	if( GAllowBufferSwap )
	{
		if( bSyncToPresentationInterval )
		{
			// Sync to vertical blank.
			GDirect3DDevice->SynchronizeToPresentationInterval();
		}

		GDirect3DDevice->Resolve( 0, NULL, GD3DFrontBuffer, NULL, 0, 0, 0, 0, 0, NULL );
		GDirect3DDevice->Swap( GD3DFrontBuffer, NULL );
	}

	// allows for on-the-fly changing of VSYNC setting
	GDirect3DDevice->SetRenderState(D3DRS_PRESENTIMMEDIATETHRESHOLD, (bLockToVsync && GSystemSettings.bUseVSync) ? GPresentLockThreshold : GPresentImmediateThreshold);

	// Start tracing render thread if requested.
	appStartCPUTrace( NameRender, TRUE, FALSE, 40, NULL );

#if LINK_AGAINST_PROFILING_D3D_LIBRARIES && !FINAL_RELEASE && !_DEBUG
	extern UBOOL GShowMaterialDrawEvents;
	GShowMaterialDrawEvents = (PIXGetCaptureState() & PIX_CAPTURE_GPU) != 0;
	GEmitDrawEvents			= (PIXGetCaptureState() & PIX_CAPTURE_GPU) != 0 || (PIXGetCaptureState() & PIX_CAPTURE_TIMING) != 0;
#endif

#if LINK_AGAINST_PROFILING_D3D_LIBRARIES && STATS && !LOG_CPU_BLOCKING_ON_GPU
	GDirect3DDevice->QueryPerfCounters( GPerfCounterBeginFrame[PerfIndex], 0 );
#endif
}

/**
 *	Check the config file for ring buffer parameters. If present, set them on the device.
 */
void XeSetRingBufferParametersFromConfig()
{
	UBOOL bIsSettingRingBufferParameters = FALSE;
	// Retrieve ring buffer settings from ini - if present.
	if (GConfig)
	{
		INT Temporary = 0;
		if (GConfig->GetInt(TEXT("XeD3D"), TEXT("RBPrimarySize"), Temporary, GEngineIni) == TRUE)
		{
			if ((Temporary & (Temporary - 1)) != 0)
			{
				warnf(TEXT("RingBufferPrimarySize must be power of two (%d from engine ini file). Using default of 32kB"), Temporary);
			}
			else
			{
				if (GRingBufferPrimarySize != Temporary)
				{
					GRingBufferPrimarySize = Temporary;
					bIsSettingRingBufferParameters = TRUE;
				}
			}
		}
		if (GConfig->GetInt(TEXT("XeD3D"), TEXT("RBSecondarySize"), Temporary, GEngineIni) == TRUE)
		{
			if ((Temporary > 0) && (Temporary < (500 * 1024)))
			{
				warnf(TEXT("RingBufferSecondarySize of %d from engine ini file. Possibly too small."), Temporary);
			}

			if (GRingBufferSecondarySize != Temporary)
			{
				GRingBufferSecondarySize = Temporary;
				bIsSettingRingBufferParameters = TRUE;
			}
		}
		if (GConfig->GetInt(TEXT("XeD3D"), TEXT("RBSegmentCount"), Temporary, GEngineIni) == TRUE)
		{
			if ((Temporary > 0) && (Temporary < (GRingBufferSegmentCount / 2)))
			{
				warnf(TEXT("RingBufferSegmentCount of %d from engine ini file. Possibly too small."), Temporary);
			}

			if (GRingBufferSegmentCount != Temporary)
			{
				GRingBufferSegmentCount = Temporary;
				bIsSettingRingBufferParameters = TRUE;
			}
		}
	}

	if (bIsSettingRingBufferParameters)
	{
		XeSetRingBufferParameters(GRingBufferPrimarySize, GRingBufferSecondarySize, GRingBufferSegmentCount, TRUE);
	}
}

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
 *									on the device. 
 */
void XeSetRingBufferParameters(INT PrimarySize, INT SecondarySize, INT SegmentCount, UBOOL bCallDeviceSet)
{
	D3DRING_BUFFER_PARAMETERS RBParams;

	appMemset((void*)&RBParams, 0, sizeof(RBParams));

	if ((PrimarySize > 0) && (PrimarySize & (PrimarySize - 1)))
	{
		warnf(TEXT("XeSetRingBufferParameters: Primary must be power of two (%d)!"), PrimarySize);
		return;
	}

	RBParams.PrimarySize = Align(PrimarySize, GPU_COMMAND_BUFFER_ALIGNMENT);
	if (PrimarySize != RBParams.PrimarySize)
	{
		warnf(TEXT("XeSetRingBufferParameters: Primary size adjusted for alignment. From %d to %d"),
			PrimarySize, RBParams.PrimarySize);
	}

	RBParams.SecondarySize = Align(SecondarySize, GPU_COMMAND_BUFFER_ALIGNMENT);
	if (SecondarySize != RBParams.SecondarySize)
	{
		warnf(TEXT("XeSetRingBufferParameters: Secondary size adjusted for alignment. From %d to %d"),
			SecondarySize, RBParams.SecondarySize);
	}
	RBParams.SegmentCount = SegmentCount;

	if (bCallDeviceSet)
	{
		VERIFYD3DRESULT(GDirect3DDevice->SetRingBufferParameters(&RBParams));
	}

	GRingBufferPrimarySize = RBParams.PrimarySize;
	GRingBufferSecondarySize = RBParams.SecondarySize;
	GRingBufferSegmentCount = RBParams.SegmentCount;
}

/**
 *	Retrieve available screen resolutions.
 *
 *	@param	Resolutions			TArray<FScreenResolutionRHI> parameter that will be filled in.
 *	@param	bIgnoreRefreshRate	If TRUE, ignore refresh rates.
 *
 *	@return	UBOOL				TRUE if successfully filled the array
 */
UBOOL RHIGetAvailableResolutions(FScreenResolutionArray& Resolutions, UBOOL bIgnoreRefreshRate)
{
	//@todo. Implement 360 version of this function
	return FALSE;
}

/**
 *	Blocks the CPU until the GPU has processed all pending commands and gone idle.
 */
void XeBlockUntilGPUIdle()
{
	RHIKickCommandBuffer();
	GDirect3DDevice->BlockUntilIdle();
}

/**
 * Enables or disables a gamma ramp that compensates for Xbox's default gamma ramp, which makes dark too dark.
 */
void XeSetCorrectiveGammaRamp(UBOOL On)
{
	// We measured the response of a monitor on PC, PS3 and Xbox 360, and found that on Xbox 360, 
	// The darks are much too dark compared to the other platforms.  
	// GameDS provided us with the gamma ramp that the 360 applies to output that causes this discrepancy.
	// This gamma ramp compensates for the 360's default gamma ramp.
	static const WORD XenonTVLumaRamp[256]=
	{
		0, 738, 1476, 2214, 2937, 3575, 4141, 4655, 5128, 5567, 5978, 6367, 6735, 7085, 7421, 7742,
		8051, 8350, 8638, 8917, 9187, 9434, 9694, 9955, 10215, 10475, 10734, 10993, 11251, 11509, 11767, 12025,
		12282, 12539, 12795, 13051, 13307, 13563, 13818, 14073, 14327, 14582, 14836, 15090, 15343, 15596, 15849,
		16102, 16354, 16607, 16858, 17110, 17362, 17613, 17864, 18114, 18365, 18615, 18865, 19115, 19365, 19614,
		19863, 20112, 20361, 20609, 20858, 21106, 21354, 21602, 21849, 22096, 22344, 22591, 22837, 23084, 23330,
		23577, 23823, 24069, 24314, 24560, 24805, 25050, 25295, 25540, 25785, 26030, 26274, 26518, 26762, 27006,
		27250, 27493, 27737, 27980, 28223, 28466, 28709, 28952, 29194, 29437, 29679, 29921, 30163, 30405, 30647,
		30888, 31130, 31371, 31612, 31853, 32094, 32335, 32576, 32816, 33056, 33297, 33537, 33777, 34017, 34257,
		34496, 34736, 34975, 35214, 35454, 35693, 35932, 36170, 36409, 36648, 36886, 37125, 37363, 37601, 37839,
		38077, 38315, 38552, 38790, 39028, 39265, 39502, 39739, 39977, 40213, 40450, 40687, 40924, 41160, 41397,
		41633, 41869, 42106, 42342, 42578, 42814, 43049, 43285, 43521, 43756, 43992, 44227, 44462, 44697, 44932,
		45167, 45402, 45637, 45872, 46106, 46341, 46575, 46809, 47044, 47278, 47512, 47746, 47980, 48214, 48447,
		48681, 48914, 49148, 49381, 49615, 49848, 50081, 50314, 50547, 50780, 51013, 51246, 51478, 51711, 51943,
		52176, 52408, 52640, 52873, 53105, 53337, 53569, 53801, 54032, 54264, 54496, 54727, 54959, 55190, 55422,
		55653, 55884, 56116, 56347, 56578, 56809, 57039, 57270, 57501, 57732, 57962, 58193, 58423, 58654, 58884,
		59114, 59344, 59575, 59805, 60035, 60265, 60494, 60724, 60954, 61184, 61413, 61643, 61872, 62102, 62331,
		62560, 62789, 63019, 63248, 63477, 63706, 63934, 64163, 64392, 64621, 64849, 65078, 65307, 65535, 
	};

	D3DGAMMARAMP GammaRamp;
	if (On)
	{
		// build a corrective ramp from the XenonTVLumaRamp table
		check(sizeof(GammaRamp.red) == sizeof(XenonTVLumaRamp));
		appMemcpy(GammaRamp.red, XenonTVLumaRamp, sizeof(XenonTVLumaRamp));
		appMemcpy(GammaRamp.green, XenonTVLumaRamp, sizeof(XenonTVLumaRamp));
		appMemcpy(GammaRamp.blue, XenonTVLumaRamp, sizeof(XenonTVLumaRamp));
	}
	else
	{
		// build a default linear ramp
		for (INT i=0; i<256; ++i)
		{
			INT value= (i<<8)+i;

			GammaRamp.red[i] = value;
			GammaRamp.green[i] = value;
			GammaRamp.blue[i] = value;
		}
	}

	GDirect3DDevice->SetGammaRamp(0,0,&GammaRamp);

	debugf(TEXT("Corrective Gamma Ramp is now %s"), On ? TEXT("enabled") : TEXT("disabled")); 
}
#endif
