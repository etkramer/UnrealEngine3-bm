/*=============================================================================
	D3DViewport.cpp: D3D viewport RHI implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "XeD3DDrvPrivate.h"

#if USE_XeD3D_RHI

/*-----------------------------------------------------------------------------
Globals
-----------------------------------------------------------------------------*/

/** A list of all viewport RHIs that have been created. */
TArray<FD3DViewport*> GD3DViewports;

/** The viewport which is currently being drawn. */
FD3DViewport* GD3DDrawingViewport = NULL;

/** Whether the device should be suspended */
extern UBOOL GD3DRenderingShouldBeSuspended;

/*-----------------------------------------------------------------------------
RHI Viewport support
-----------------------------------------------------------------------------*/

FD3DViewport::FD3DViewport(UINT InSizeX,UINT InSizeY,UBOOL bInIsFullscreen):
	SizeX(InSizeX),
	SizeY(InSizeY),
	bIsFullscreen(bInIsFullscreen)
{
	GD3DViewports.AddItem(this);
}

FD3DViewport::~FD3DViewport()
{
	GD3DViewports.RemoveItem(this);	
}

void FD3DViewport::Resize(UINT InSizeX,UINT InSizeY,UBOOL bInIsFullscreen)
{
	SizeX = InSizeX;
	SizeY = InSizeY;
	bIsFullscreen = bInIsFullscreen;
}

FViewportRHIRef RHICreateViewport(void* WindowHandle,UINT SizeX,UINT SizeY,UBOOL bIsFullscreen)
{
	check( IsInGameThread() );
	return new FD3DViewport(SizeX,SizeY,bIsFullscreen);
}

void RHIResizeViewport(FViewportRHIParamRef Viewport,UINT SizeX,UINT SizeY,UBOOL bIsFullscreen)
{
	check( IsInGameThread() );
	Viewport->Resize(SizeX,SizeY,bIsFullscreen);
}

void RHITick( FLOAT DeltaTime )
{
	check( IsInGameThread() );
}

extern UBOOL GD3DRenderingIsSuspended;

void RHIBeginDrawingViewport(FViewportRHIParamRef Viewport)
{
	check(GD3DRenderingIsSuspended == FALSE);
	check(!GD3DDrawingViewport);
	GD3DDrawingViewport = Viewport;

	// Tell D3D we're going to start rendering.
	GDirect3DDevice->BeginScene();

	// update any resources that needed a deferred update
	FDeferredUpdateResource::UpdateResources();

	// Set the configured D3D render state.
	for(INT SamplerIndex = 0;SamplerIndex < 16;SamplerIndex++)
	{
		GDirect3DDevice->SetSamplerState(SamplerIndex,D3DSAMP_MAXANISOTROPY,Clamp(GSystemSettings.MaxAnisotropy,1,16));
	}

	// set the render target and the viewport
	RHISetRenderTarget(FSurfaceRHIRef(GD3DBackBuffer),NULL);	
}

void RHIEndDrawingViewport(FViewportRHIParamRef Viewport,UBOOL bPresent,UBOOL bLockToVsync)
{
	check(GD3DRenderingIsSuspended == FALSE);
	check(GD3DDrawingViewport == Viewport);
	GD3DDrawingViewport = NULL;

	// Clear references the device might have to resources.
  	GDirect3DDevice->SetRenderTarget(0,GD3DBackBuffer);
  	GDirect3DDevice->SetDepthStencilSurface(NULL);
  
  	for(UINT TextureIndex = 0;TextureIndex < 16;TextureIndex++)
  	{
  		GDirect3DDevice->SetTexture(TextureIndex,NULL);
  	}

  	for(UINT TextureIndex = D3DVERTEXTEXTURESAMPLER0; TextureIndex <= D3DVERTEXTEXTURESAMPLER3; TextureIndex++)
  	{
  		GDirect3DDevice->SetTexture(TextureIndex,NULL);
  	}

	GDirect3DDevice->SetVertexShader(NULL);

	for(UINT StreamIndex = 0;StreamIndex < 16;StreamIndex++)
	{
		GDirect3DDevice->SetStreamSource(StreamIndex,NULL,0,0);
	}

	GDirect3DDevice->SetIndices(NULL);
	GDirect3DDevice->SetPixelShader(NULL);

	// Tell D3D we're done rendering.
	GDirect3DDevice->EndScene();

	// Delete unused resources.
	DeleteUnusedXeResources();

	if(bPresent)
	{
		XePerformSwap(TRUE,bLockToVsync);

		// If the input latency timer has been triggered, block until the GPU is completely
		// finished displaying this frame and calculate the delta time.
		if ( GInputLatencyTimer.RenderThreadTrigger )
		{
			XeBlockUntilGPUIdle();
			DWORD EndTime = appCycles();
			GInputLatencyTimer.DeltaTime = EndTime - GInputLatencyTimer.StartTime;
			GInputLatencyTimer.RenderThreadTrigger = FALSE;
		}
	}
}


void RHIBeginScene()
{
	if ( GD3DRenderingIsSuspended )
	{
		GDirect3DDevice->Resume();
		GD3DRenderingIsSuspended = FALSE;
	}
	// Tell D3D we're going to start rendering.
	GDirect3DDevice->BeginScene();
}

void RHIEndScene()
{
	// Tell D3D we're done rendering.
	GDirect3DDevice->EndScene();

	if ( !GD3DRenderingIsSuspended && GD3DRenderingShouldBeSuspended )
	{
		GD3DRenderingIsSuspended = TRUE;
		GDirect3DDevice->Suspend();
	}
}

FSurfaceRHIRef RHIGetViewportBackBuffer(FViewportRHIParamRef Viewport)
{
	return GD3DBackBuffer;
}

void RHISetLargestExpectedViewportSize( UINT NewLargestExpectedViewportWidth, UINT NewLargestExpectedViewportHeight )
{
	// NOTE: Not supported or needed on Xbox platform
}

#endif
