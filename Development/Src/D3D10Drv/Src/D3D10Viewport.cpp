/*=============================================================================
	D3D10Viewport.cpp: D3D viewport RHI implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3D10DrvPrivate.h"

/**
 * Creates a FD3D10Surface to represent a swap chain's back buffer.
 */
static FD3D10Surface* GetSwapChainSurface(FD3D10DynamicRHI* D3DRHI,IDXGISwapChain* SwapChain)
{
	// Grab the back buffer
	TRefCountPtr<ID3D10Texture2D> BackBufferResource;
	VERIFYD3D10RESULT(SwapChain->GetBuffer(0,IID_ID3D10Texture2D,(void**)BackBufferResource.GetInitReference()));

	// create the render target view
	TRefCountPtr<ID3D10RenderTargetView> BackBufferRenderTargetView;
	D3D10_RENDER_TARGET_VIEW_DESC RTVDesc;
	RTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	RTVDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;
	VERIFYD3D10RESULT(D3DRHI->GetDevice()->CreateRenderTargetView(BackBufferResource,&RTVDesc,BackBufferRenderTargetView.GetInitReference()));

	return new FD3D10Surface(BackBufferRenderTargetView,NULL,NULL,BackBufferResource);
}

/**
 * Sets Panorama to draw to the viewport's swap chain.
 */
static void SetPanoramaSwapChain(FD3D10DynamicRHI* D3DRHI,IDXGISwapChain* SwapChain,HWND WindowHandle)
{
	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	SwapChain->GetDesc(&SwapChainDesc);

	#if WITH_PANORAMA
		extern void appPanoramaRenderHookInit(IUnknown*,void*,HWND);
		// Allow G4WLive to allocate any resources it needs
		appPanoramaRenderHookInit(D3DRHI->GetDevice(),&SwapChainDesc,WindowHandle);
	#endif
}

FD3D10Viewport::FD3D10Viewport(FD3D10DynamicRHI* InD3DRHI,HWND InWindowHandle,UINT InSizeX,UINT InSizeY,UBOOL bInIsFullscreen):
	D3DRHI(InD3DRHI),
	WindowHandle(InWindowHandle),
	SizeX(InSizeX),
	SizeY(InSizeY),
	bIsFullscreen(bInIsFullscreen),
	bIsValid(TRUE)
{
	D3DRHI->Viewports.AddItem(this);

	// Ensure that the D3D device has been created.
	D3DRHI->InitD3DDevice();

	// Create a backbuffer/swapchain for each viewport
	TRefCountPtr<IDXGIDevice> DXGIDevice;
	VERIFYD3D10RESULT(D3DRHI->GetDevice()->QueryInterface( IID_IDXGIDevice, (void**)DXGIDevice.GetInitReference() ));

	// Restore the window.
	// If the window is minimized when the swap chain is created, it may be created as an 8x8 buffer.
	::ShowWindow(WindowHandle,SW_RESTORE);

	// Create the swapchain.
	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	appMemzero( &SwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC) );
	SwapChainDesc.BufferDesc.Width = SizeX;
	SwapChainDesc.BufferDesc.Height = SizeY;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;	// illamas: use 0 to avoid a potential mismatch with hw
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 0;	// illamas: ditto
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	SwapChainDesc.SampleDesc.Count = 1;		// TODO: Are there MSAA settings in UE3?
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferCount = 1;
	SwapChainDesc.OutputWindow = WindowHandle;
	SwapChainDesc.Windowed = !bIsFullscreen;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	VERIFYD3D10RESULT(D3DRHI->GetFactory()->CreateSwapChain(DXGIDevice,&SwapChainDesc,SwapChain.GetInitReference()));

	// Set the DXGI message hook to not change the window behind our back.
	D3DRHI->GetFactory()->MakeWindowAssociation(WindowHandle,DXGI_MWA_NO_WINDOW_CHANGES);

	// Create a RHI surface to represent the viewport's back buffer.
	BackBuffer = GetSwapChainSurface(D3DRHI,SwapChain);

	// If this is the first viewport, set Panorama to render to it.
	if(D3DRHI->Viewports.FindItemIndex(this) == 0)
	{
		SetPanoramaSwapChain(D3DRHI,SwapChain,WindowHandle);
	}

	// Tell the window to redraw when they can.
	::PostMessage( WindowHandle, WM_PAINT, 0, 0 );
}

FD3D10Viewport::~FD3D10Viewport()
{
	// If this is the first viewport, reset Panorama's reference to it.
	if(D3DRHI->Viewports.FindItemIndex(this) == 0)
	{
		#if WITH_PANORAMA
			extern void appPanoramaHookDeviceDestroyed();
			appPanoramaHookDeviceDestroyed();
		#endif
	}

	// If the swap chain was in fullscreen mode, switch back to windowed before releasing the swap chain.
	// DXGI throws an error otherwise.
	VERIFYD3D10RESULT(SwapChain->SetFullscreenState(FALSE,NULL));

	D3DRHI->Viewports.RemoveItem(this);
}

void FD3D10Viewport::Resize(UINT InSizeX,UINT InSizeY,UBOOL bInIsFullscreen)
{
	// Release our backbuffer reference, as required by DXGI before calling ResizeBuffers.
	BackBuffer.SafeRelease();

	if(SizeX != InSizeX || SizeY != InSizeY)
	{
		SizeX = InSizeX;
		SizeY = InSizeY;

		// Resize the swap chain.
		VERIFYD3D10RESULT(SwapChain->ResizeBuffers(1,SizeX,SizeY,DXGI_FORMAT_R8G8B8A8_UNORM,DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
	}

	if(bIsFullscreen != bInIsFullscreen)
	{
		bIsFullscreen = bInIsFullscreen;
		bIsValid = FALSE;

		// Use ConditionalResetSwapChain to call SetFullscreenState, to handle the failure case.
		// Ignore the viewport's focus state; since Resize is called as the result of a user action we assume authority without waiting for Focus.
		ConditionalResetSwapChain(TRUE);
	}

	// Create a RHI surface to represent the viewport's back buffer.
	BackBuffer = GetSwapChainSurface(D3DRHI,SwapChain);

	// If this is the first viewport, set Panorama to render to it.
	if(D3DRHI->Viewports.FindItemIndex(this) == 0)
	{
		SetPanoramaSwapChain(D3DRHI,SwapChain,WindowHandle);
	}
}

void FD3D10Viewport::ConditionalResetSwapChain(UBOOL bIgnoreFocus)
{
	if(!bIsValid)
	{
		// Check if the viewport's window is focused before resetting the swap chain's fullscreen state.
		HWND FocusWindow = ::GetFocus();
		const UBOOL bIsFocused = FocusWindow == WindowHandle;
		const UBOOL bIsIconic = ::IsIconic( WindowHandle );
		if(bIgnoreFocus || (bIsFocused && !bIsIconic) )
		{
			FlushRenderingCommands();

			HRESULT Result = SwapChain->SetFullscreenState(bIsFullscreen,NULL);
			if(SUCCEEDED(Result))
			{
				bIsValid = TRUE;
			}
			else
			{
				// Even though the docs say SetFullscreenState always returns S_OK, that doesn't always seem to be the case.
				debugf(TEXT("IDXGISwapChain::SetFullscreenState returned %08x; waiting for the next frame to try again."),Result);
			}
		}
	}
}

void FD3D10Viewport::Present(UBOOL bLockToVsync)
{
	// We can't call Present if !bIsValid, as it waits a window message to be processed, but the main thread may not be pumping the message handler.
	if(bIsValid)
	{
		// Check if the viewport's swap chain has been invalidated by DXGI.
		BOOL bSwapChainFullscreenState;
		TRefCountPtr<IDXGIOutput> SwapChainOutput;
		VERIFYD3D10RESULT(SwapChain->GetFullscreenState(&bSwapChainFullscreenState,SwapChainOutput.GetInitReference()));
		if(bSwapChainFullscreenState != bIsFullscreen)
		{
			bIsValid = FALSE;
			
			// Minimize the window.
			::ShowWindow(WindowHandle,SW_FORCEMINIMIZE);
		}
	}

	// Present the back buffer to the viewport window.
	HRESULT Result = SwapChain->Present(bLockToVsync ? 1 : 0,0);

	// Detect a lost device.
	if(Result == DXGI_ERROR_DEVICE_REMOVED || Result == DXGI_ERROR_DEVICE_RESET || Result == DXGI_ERROR_DRIVER_INTERNAL_ERROR)
	{
		// This variable is checked periodically by the main thread.
		D3DRHI->bDeviceRemoved = TRUE;
	}
	else
	{
		VERIFYD3D10RESULT(Result);
	}
}

/*=============================================================================
 *	The following RHI functions must be called from the main thread.
 *=============================================================================*/
FViewportRHIRef FD3D10DynamicRHI::CreateViewport(void* WindowHandle,UINT SizeX,UINT SizeY,UBOOL bIsFullscreen)
{
	check( IsInGameThread() );
	return new FD3D10Viewport(this,(HWND)WindowHandle,SizeX,SizeY,bIsFullscreen);
}

void FD3D10DynamicRHI::ResizeViewport(FViewportRHIParamRef ViewportRHI,UINT SizeX,UINT SizeY,UBOOL bIsFullscreen)
{
	DYNAMIC_CAST_D3D10RESOURCE(Viewport,Viewport);

	check( IsInGameThread() );
	Viewport->Resize(SizeX,SizeY,bIsFullscreen);
}

void FD3D10DynamicRHI::Tick( FLOAT DeltaTime )
{
	check( IsInGameThread() );

	// Check to see if the device has been removed.
	if ( bDeviceRemoved )
	{
		InitD3DDevice();
	}

	// Check if any swap chains have been invalidated.
	for(INT ViewportIndex = 0;ViewportIndex < Viewports.Num();ViewportIndex++)
	{
		Viewports(ViewportIndex)->ConditionalResetSwapChain(FALSE);
	}
}

/*=============================================================================
 *	Viewport functions.
 *=============================================================================*/

void FD3D10DynamicRHI::BeginDrawingViewport(FViewportRHIParamRef ViewportRHI)
{
	DYNAMIC_CAST_D3D10RESOURCE(Viewport,Viewport);

	SCOPE_CYCLE_COUNTER(STAT_D3D10PresentTime);

	check(!DrawingViewport);
	DrawingViewport = Viewport;

	// update any resources that needed a deferred update
	FDeferredUpdateResource::UpdateResources();

	// Set the render target and viewport.
	RHISetRenderTarget(Viewport->GetBackBuffer(), FSurfaceRHIRef());
	RHISetViewport(0,0,0.0f,Viewport->GetSizeX(),Viewport->GetSizeY(),1.0f);
}

void FD3D10DynamicRHI::EndDrawingViewport(FViewportRHIParamRef ViewportRHI,UBOOL bPresent,UBOOL bLockToVsync)
{
	DYNAMIC_CAST_D3D10RESOURCE(Viewport,Viewport);

	SCOPE_CYCLE_COUNTER(STAT_D3D10PresentTime);

	check(DrawingViewport.GetReference() == Viewport);
	DrawingViewport = NULL;

	// Clear references to resources that were bound to the rendering pipeline while rendering this viewport.
	// This will allow them to be freed if the application no longer holds a reference to them.
	PipelineResources.Reset();

	// Clear references the device might have to resources.
	ID3D10RenderTargetView* RTV = Viewport->GetBackBuffer()->RenderTargetView;
	Direct3DDevice->OMSetRenderTargets(1,&RTV,NULL);
	bCurrentRenderTargetIsMultisample = FALSE;

	for(UINT TextureIndex = 0;TextureIndex < 16;TextureIndex++)
	{
		ID3D10ShaderResourceView* NullView = NULL;
		Direct3DDevice->PSSetShaderResources(TextureIndex,1,&NullView);
	}

	Direct3DDevice->VSSetShader(NULL);

	for(UINT StreamIndex = 0;StreamIndex < 16;StreamIndex++)
	{
		ID3D10Buffer* NullBuffer = NULL;
		UINT Strides = 0;
		UINT Offsets = 0;
		Direct3DDevice->IASetVertexBuffers(StreamIndex,1,&NullBuffer,&Strides,&Offsets);
	}

	Direct3DDevice->IASetIndexBuffer(NULL,DXGI_FORMAT_R16_UINT,0);
	Direct3DDevice->PSSetShader(NULL);

	#if WITH_PANORAMA
		extern void appPanoramaRenderHookRender(void);
		// Allow G4WLive to render the Live Guide as needed (or toasts)
		appPanoramaRenderHookRender();
	#endif

	if(bPresent)
	{
		Viewport->Present(bLockToVsync);
	}

	// Wait for the GPU to finish rendering the previous frame before finishing this frame.
	FrameSyncEvent.WaitForCompletion();
	FrameSyncEvent.IssueEvent();

	// If the input latency timer has been triggered, block until the GPU is completely
	// finished displaying this frame and calculate the delta time.
	if ( GInputLatencyTimer.RenderThreadTrigger )
	{
		FrameSyncEvent.WaitForCompletion();
		DWORD EndTime = appCycles();
		GInputLatencyTimer.DeltaTime = EndTime - GInputLatencyTimer.StartTime;
		GInputLatencyTimer.RenderThreadTrigger = FALSE;
	}
}

void FD3D10DynamicRHI::BeginScene()
{
}

void FD3D10DynamicRHI::EndScene()
{
}

FSurfaceRHIRef FD3D10DynamicRHI::GetViewportBackBuffer(FViewportRHIParamRef ViewportRHI)
{
	DYNAMIC_CAST_D3D10RESOURCE(Viewport,Viewport);

	return Viewport->GetBackBuffer();
}
