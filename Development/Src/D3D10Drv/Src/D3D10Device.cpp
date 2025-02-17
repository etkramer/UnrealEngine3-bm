/*=============================================================================
	D3D10Device.cpp: D3D device RHI implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3D10DrvPrivate.h"
#include <delayimp.h>

/** This function is used as a SEH filter to catch only delay load exceptions. */
static UBOOL IsDelayLoadException(PEXCEPTION_POINTERS ExceptionPointers)
{
	switch(ExceptionPointers->ExceptionRecord->ExceptionCode)
	{
	case VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND):
	case VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND):
		return EXCEPTION_EXECUTE_HANDLER;
	default:
		return EXCEPTION_CONTINUE_SEARCH;
	}
}

/**
 * Since CreateDXGIFactory is a delay loaded import from the D3D10 DLL, if the user
 * doesn't have Vista/DX10, calling CreateDXGIFactory will throw an exception.
 * We use SEH to detect that case and fail gracefully.
 */
static void SafeCreateDXGIFactory(IDXGIFactory** DXGIFactory)
{
	__try
	{
		CreateDXGIFactory(__uuidof(IDXGIFactory),(void**)DXGIFactory);
	}
	__except(IsDelayLoadException(GetExceptionInformation()))
	{
	}
}

/** @return TRUE if Direct3D 10 is supported by the host. */
UBOOL IsDirect3D10Supported()
{
	// Try to create the DXGIFactory.  This will fail if we're not running Vista.
	TRefCountPtr<IDXGIFactory> DXGIFactory;
	SafeCreateDXGIFactory(DXGIFactory.GetInitReference());
	if(!DXGIFactory)
	{
		return FALSE;
	}

	// Enumerate the DXGIFactory's adapters.
	UINT AdapterIndex = 0;
	TRefCountPtr<IDXGIAdapter> TempAdapter;
	UBOOL bHasD3D10Adapter = FALSE;
	while(DXGIFactory->EnumAdapters(AdapterIndex,TempAdapter.GetInitReference()) != DXGI_ERROR_NOT_FOUND)
	{
		// Check that if adapter supports D3D10.
		if(TempAdapter && TempAdapter->CheckInterfaceSupport(__uuidof(ID3D10Device),NULL) == S_OK)
		{
			// Apparently checking for support for the D3D10Device interface doesn't tell us whether the device supports D3D10.
			// The only way to tell seems to be to try creating the device.
			TRefCountPtr<ID3D10Device> D3DDevice;
			if(SUCCEEDED(D3D10CreateDevice(TempAdapter,D3D10_DRIVER_TYPE_HARDWARE,NULL,0,D3D10_SDK_VERSION,D3DDevice.GetInitReference())))
			{
				bHasD3D10Adapter = TRUE;

				// Log some information about the available D3D10 adapters.
				DXGI_ADAPTER_DESC AdapterDesc;
				VERIFYD3D10RESULT(TempAdapter->GetDesc(&AdapterDesc));

				debugf(TEXT("Found D3D10 adapter %u: %s"),AdapterIndex,AdapterDesc.Description);
				debugf(
					TEXT("Adapter has %uMB of dedicated video memory, %uMB of dedicated system memory, and %uMB of shared system memory"),
					AdapterDesc.DedicatedVideoMemory / (1024*1024),
					AdapterDesc.DedicatedSystemMemory / (1024*1024),
					AdapterDesc.SharedSystemMemory / (1024*1024)
					);
			}
		}

		AdapterIndex++;
	};

	return bHasD3D10Adapter;
}

/**
 * Called at startup to initialize the D3D10 RHI.  This assumes that the caller has already checked that IsDirect3D10Supported is TRUE.
 * @return The D3D10 RHI
 */
FDynamicRHI* D3D10CreateRHI()
{
	TRefCountPtr<IDXGIFactory> DXGIFactory;
	SafeCreateDXGIFactory(DXGIFactory.GetInitReference());
	check(DXGIFactory);
	return new FD3D10DynamicRHI(DXGIFactory);
}

FD3D10DynamicRHI::FD3D10DynamicRHI(IDXGIFactory* InDXGIFactory):
	DXGIFactory(InDXGIFactory),
	bDeviceRemoved(FALSE),
	DeviceSizeX(0),
	DeviceSizeY(0),
	DeviceWindow(NULL),
	bIsFullscreenDevice(FALSE),
	bMSAAIsSupported(FALSE),
	bCurrentRenderTargetIsMultisample(FALSE),
	FrameSyncEvent(this),
	PendingNumInstances(0),
	CurrentDepthState(),
	CurrentStencilState(),
	CurrentRasterizerState(),
	CurrentBlendState(),
	CurrentStencilRef(0),
	CurrentScissorEnable(FALSE),
	CurrentColorWriteEnable(D3D10_COLOR_WRITE_ENABLE_ALL),
	CurrentBlendFactor(0,0,0,0),
	PendingDrawPrimitiveUPVertexData(NULL),
	PendingNumVertices(0),
	PendingVertexDataStride(0),
	StaticData(NULL),
	StaticDataSize(0),
	PendingDrawPrimitiveUPIndexData(NULL),
	PendingPrimitiveType(0),
	PendingNumPrimitives(0),
	PendingMinVertexIndex(0),
	PendingIndexDataStride(0),
	CurrentDynamicVB(0),
	CurrentDynamicIB(0),
	CurrentVBOffset(0),
	CurrentIBOffset(0)
{
	// This should be called once at the start 
	check( IsInGameThread() );
	check( !GIsThreadedRendering );

	// ensure we are running against the proper D3DX runtime
	if (FAILED(D3DX10CheckVersion(D3D10_SDK_VERSION, D3DX10_SDK_VERSION)))
	{
		appErrorf(
			NAME_FriendlyError,
			TEXT("The D3DX10 runtime version does not match what the application was built with (%d). Cannot continue."),
			D3DX10_SDK_VERSION
			);
	}

	// Initialize the RHI capabilities.
	GRHIShaderPlatform = SP_PCD3D_SM4;
	GPixelCenterOffset = 0.0f;	// Note that in D3D10, there is no more half-texel offset
	GUsesInvertedZ = TRUE;
	GSupportsVertexInstancing = TRUE;
	GSupportsDepthTextures = FALSE;
	GSupportsHardwarePCF = FALSE;
	GSupportsFetch4 = FALSE;
	GSupportsFPFiltering = TRUE;

	// Initialize the platform pixel format map.
	GPixelFormats[ PF_Unknown		].PlatformFormat	= DXGI_FORMAT_UNKNOWN;
	GPixelFormats[ PF_A32B32G32R32F	].PlatformFormat	= DXGI_FORMAT_R32G32B32A32_FLOAT;
	GPixelFormats[ PF_A8R8G8B8		].PlatformFormat	= DXGI_FORMAT_R8G8B8A8_UNORM;
	GPixelFormats[ PF_G8			].PlatformFormat	= DXGI_FORMAT_R8_UNORM;
	GPixelFormats[ PF_G16			].PlatformFormat	= DXGI_FORMAT_UNKNOWN;	// Not supported for rendering.
	GPixelFormats[ PF_DXT1			].PlatformFormat	= DXGI_FORMAT_BC1_UNORM;
	GPixelFormats[ PF_DXT3			].PlatformFormat	= DXGI_FORMAT_BC2_UNORM;
	GPixelFormats[ PF_DXT5			].PlatformFormat	= DXGI_FORMAT_BC3_UNORM;
	GPixelFormats[ PF_UYVY			].PlatformFormat	= DXGI_FORMAT_UNKNOWN;		// TODO: Not supported in D3D10
	GPixelFormats[ PF_DepthStencil	].PlatformFormat	= DXGI_FORMAT_D24_UNORM_S8_UINT;
	GPixelFormats[ PF_ShadowDepth	].PlatformFormat	= DXGI_FORMAT_D24_UNORM_S8_UINT;//DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	GPixelFormats[ PF_FilteredShadowDepth ].PlatformFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;//DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	GPixelFormats[ PF_R32F			].PlatformFormat	= DXGI_FORMAT_R32_FLOAT;
	GPixelFormats[ PF_G16R16		].PlatformFormat	= DXGI_FORMAT_R16G16_UNORM;
	GPixelFormats[ PF_G16R16F		].PlatformFormat	= DXGI_FORMAT_R16G16_FLOAT;
	GPixelFormats[ PF_G16R16F_FILTER].PlatformFormat	= DXGI_FORMAT_R16G16_FLOAT;
	GPixelFormats[ PF_G32R32F		].PlatformFormat	= DXGI_FORMAT_R32G32_FLOAT;
	GPixelFormats[ PF_A2B10G10R10   ].PlatformFormat    = DXGI_FORMAT_R10G10B10A2_UNORM;
	GPixelFormats[ PF_A16B16G16R16  ].PlatformFormat    = DXGI_FORMAT_R16G16B16A16_UNORM;
	GPixelFormats[ PF_D24 ].PlatformFormat				= DXGI_FORMAT_UNKNOWN;//(D3DFORMAT)(MAKEFOURCC('D','F','2','4'));
	GPixelFormats[ PF_R16F			].PlatformFormat	= DXGI_FORMAT_R16_FLOAT;
	GPixelFormats[ PF_R16F_FILTER	].PlatformFormat	= DXGI_FORMAT_R16_FLOAT;

	GPixelFormats[ PF_FloatRGB	].PlatformFormat		= DXGI_FORMAT_R16G16B16A16_FLOAT;
	GPixelFormats[ PF_FloatRGB	].BlockBytes			= 8;
	GPixelFormats[ PF_FloatRGBA	].PlatformFormat		= DXGI_FORMAT_R16G16B16A16_FLOAT;
	GPixelFormats[ PF_FloatRGBA	].BlockBytes			= 8;

	const INT MaxTextureDims = 8128;
	GMaxTextureMipCount = appCeilLogTwo( MaxTextureDims ) + 1;
	GMaxTextureMipCount = Min<INT>( MAX_TEXTURE_MIP_COUNT, GMaxTextureMipCount );

	// Initialize the frame query event.
	FrameSyncEvent.InitResource();

	// Initialize the constant buffers.
	InitConstantBuffers();
}

FD3D10DynamicRHI::~FD3D10DynamicRHI()
{
	check(IsInGameThread() && IsInRenderingThread());

	// Cleanup the D3D device.
	CleanupD3DDevice();

	// Delink the frame sync event from the resource list.
	FrameSyncEvent.ReleaseResource();
}

void FD3D10DynamicRHI::InitD3DDevice()
{
	check( IsInGameThread() );

	// Wait for the rendering thread to go idle.
	SCOPED_SUSPEND_RENDERING_THREAD(TRUE);

	// If the device we were using has been removed, release it and the resources we created for it.
	if(bDeviceRemoved)
	{
		bDeviceRemoved = FALSE;

		// Cleanup the D3D device.
		CleanupD3DDevice();

		// We currently don't support removed devices because FTexture2DResource can't recreate its RHI resources from scratch.
		// We would also need to recreate the viewport swap chains from scratch.
		appErrorf(TEXT("The Direct3D 10 device that was being used has been removed.  Please restart the game."));
	}

	// If we don't have a device yet, either because this is the first viewport, or the old device was removed, create a device.
	if(!Direct3DDevice)
	{
		check(!GIsRHIInitialized);

		// Determine the adapter and device type to use.
		TRefCountPtr<IDXGIAdapter> Adapter;
		D3D10_DRIVER_TYPE DriverType = D3D10_DRIVER_TYPE_HARDWARE;
		UINT DeviceFlags = D3D10_CREATE_DEVICE_SINGLETHREADED;

		// Use a debug device if specified on the command line.
		if(ParseParam(appCmdLine(),TEXT("d3ddebug")))
		{
			DeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
		}

		// Allow selection of NVPerfHUD Adapter (if available)
		TRefCountPtr<IDXGIAdapter> EnumAdapter;
		UINT CurrentAdapter = 0;
		while (DXGIFactory->EnumAdapters(CurrentAdapter,EnumAdapter.GetInitReference()) != DXGI_ERROR_NOT_FOUND)
		{
			if (EnumAdapter && EnumAdapter->CheckInterfaceSupport(__uuidof(ID3D10Device),NULL) == S_OK)
			{
				DXGI_ADAPTER_DESC AdapterDesc;
				if (SUCCEEDED(EnumAdapter->GetDesc(&AdapterDesc)))
				{
					const UBOOL bIsPerfHUD = !appStricmp(AdapterDesc.Description,TEXT("NVIDIA PerfHUD"));

					// Select the first adapter in normal circumstances or the PerfHUD one if it exists.
					if(CurrentAdapter == 0 || bIsPerfHUD)
					{
						Adapter = EnumAdapter;
					}
					if(bIsPerfHUD)
					{
						DriverType = D3D10_DRIVER_TYPE_REFERENCE;
					}
				}
			}
			++CurrentAdapter;
		}

		// Creating the Direct3D device.
		VERIFYD3D10RESULT(D3D10CreateDevice(
			Adapter,
			DriverType,
			NULL,
			DeviceFlags,
			D3D10_SDK_VERSION,
			Direct3DDevice.GetInitReference()
			));

		// Determine whether the device supports MSAA on the formats we use for scene rendering.
		// (DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_FLOAT) 
		UINT LDRColorFormatSupport = 0;
		UINT HDRColorFormatSupport = 0;
		VERIFYD3D10RESULT(Direct3DDevice->CheckFormatSupport(DXGI_FORMAT_R8G8B8A8_UNORM,&LDRColorFormatSupport));
		VERIFYD3D10RESULT(Direct3DDevice->CheckFormatSupport(DXGI_FORMAT_R16G16B16A16_FLOAT,&HDRColorFormatSupport));

		// We need to be able to render to MSAA color surfaces, and resolve them to a non-MSAA texture.
		const UBOOL RequiredMSAASupport = D3D10_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET | D3D10_FORMAT_SUPPORT_MULTISAMPLE_RESOLVE;
		bMSAAIsSupported = (LDRColorFormatSupport & RequiredMSAASupport) && (HDRColorFormatSupport & RequiredMSAASupport);

		// Notify all initialized FRenderResources that there's a valid RHI device to create their RHI resources for now.
		for(TLinkedList<FRenderResource*>::TIterator ResourceIt(FRenderResource::GetResourceList());ResourceIt;ResourceIt.Next())
		{
			ResourceIt->InitDynamicRHI();
		}
		for(TLinkedList<FRenderResource*>::TIterator ResourceIt(FRenderResource::GetResourceList());ResourceIt;ResourceIt.Next())
		{
			ResourceIt->InitRHI();
		}

		// Set the RHI initialized flag.
		GIsRHIInitialized = TRUE;
	}
}

/**
 * Returns a supported screen resolution that most closely matches the input.
 * @param Width - Input: Desired resolution width in pixels. Output: A width that the platform supports.
 * @param Height - Input: Desired resolution height in pixels. Output: A height that the platform supports.
 */
void FD3D10DynamicRHI::GetSupportedResolution( UINT &Width, UINT &Height )
{
	UINT InitializedMode = FALSE;
	DXGI_MODE_DESC BestMode;
	BestMode.Width = 0;
	BestMode.Height = 0;

	// Enumerate all DXGI adapters
	// TODO: Cap at 1 for default adapter
	for(UINT i = 0;i < 1;i++)
    {
		HRESULT hr = S_OK;
        TRefCountPtr<IDXGIAdapter> Adapter;
        hr = DXGIFactory->EnumAdapters(i,Adapter.GetInitReference());
        if( DXGI_ERROR_NOT_FOUND == hr )
        {
            hr = S_OK;
            break;
        }
        if( FAILED(hr) )
        {
            return;
        }

        // get the description of the adapter
        DXGI_ADAPTER_DESC AdapterDesc;
        VERIFYD3D10RESULT(Adapter->GetDesc(&AdapterDesc));
      
        // Enumerate outputs for this adapter
		// TODO: Cap at 1 for default output
		for(UINT o = 0;o < 1; o++)
		{
			TRefCountPtr<IDXGIOutput> Output;
			hr = Adapter->EnumOutputs(o,Output.GetInitReference());
			if(DXGI_ERROR_NOT_FOUND == hr)
				break;
			if(FAILED(hr))
				return;

			// TODO: GetDisplayModeList is a terribly SLOW call.  It can take up to a second per invocation.
			//  We might want to work around some DXGI badness here.
			DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			UINT NumModes = 0;
			hr = Output->GetDisplayModeList(Format,0,&NumModes,NULL);
			if(hr == DXGI_ERROR_NOT_FOUND)
			{
				return;
			}
			else if(hr == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
			{
				appErrorf(
					NAME_FriendlyError,
					TEXT("This application cannot be run over a remote desktop configuration")
					);
				return;
			}
			DXGI_MODE_DESC* ModeList = new DXGI_MODE_DESC[ NumModes ];
			VERIFYD3D10RESULT(Output->GetDisplayModeList(Format,0,&NumModes,ModeList));

			for(UINT m = 0;m < NumModes;m++)
			{
				// Search for the best mode
				UBOOL IsEqualOrBetterWidth = Abs((INT)ModeList[m].Width - (INT)Width) <= Abs((INT)BestMode.Width - (INT)Width);
				UBOOL IsEqualOrBetterHeight = Abs((INT)ModeList[m].Height - (INT)Height) <= Abs((INT)BestMode.Height - (INT)Height);
				if(!InitializedMode || (IsEqualOrBetterWidth && IsEqualOrBetterHeight))
				{
					BestMode = ModeList[m];
					InitializedMode = TRUE;
				}
			}

			delete[] ModeList;
		}
    }

	check(InitializedMode);
	Width = BestMode.Width;
	Height = BestMode.Height;
}

/**
 *	Retrieve available screen resolutions.
 *
 *	@param	Resolutions			TArray<FScreenResolutionRHI> parameter that will be filled in.
 *	@param	bIgnoreRefreshRate	If TRUE, ignore refresh rates.
 *
 *	@return	UBOOL				TRUE if successfully filled the array
 */
UBOOL FD3D10DynamicRHI::GetAvailableResolutions(FScreenResolutionArray& Resolutions, UBOOL bIgnoreRefreshRate)
{
	INT MinAllowableResolutionX = 0;
	INT MinAllowableResolutionY = 0;
	INT MaxAllowableResolutionX = 10480;
	INT MaxAllowableResolutionY = 10480;
	INT MinAllowableRefreshRate = 0;
	INT MaxAllowableRefreshRate = 10480;

	GConfig->GetInt(TEXT("WinDrv.WindowsClient"), TEXT("MinAllowableResolutionX"), MinAllowableResolutionX, GEngineIni);
	GConfig->GetInt(TEXT("WinDrv.WindowsClient"), TEXT("MinAllowableResolutionY"), MinAllowableResolutionY, GEngineIni);
	GConfig->GetInt(TEXT("WinDrv.WindowsClient"), TEXT("MaxAllowableResolutionX"), MaxAllowableResolutionX, GEngineIni);
	GConfig->GetInt(TEXT("WinDrv.WindowsClient"), TEXT("MaxAllowableResolutionY"), MaxAllowableResolutionY, GEngineIni);
	GConfig->GetInt(TEXT("WinDrv.WindowsClient"), TEXT("MinAllowableRefreshRate"), MinAllowableRefreshRate, GEngineIni);
	GConfig->GetInt(TEXT("WinDrv.WindowsClient"), TEXT("MaxAllowableRefreshRate"), MaxAllowableRefreshRate, GEngineIni);

	if (MaxAllowableResolutionX == 0)
	{
		MaxAllowableResolutionX = 10480;
	}
	if (MaxAllowableResolutionY == 0)
	{
		MaxAllowableResolutionY = 10480;
	}
	if (MaxAllowableRefreshRate == 0)
	{
		MaxAllowableRefreshRate = 10480;
	}

	// Check the default adapter only.
	INT CurrentAdapter = 0;
	HRESULT hr = S_OK;
	TRefCountPtr<IDXGIAdapter> Adapter;
	hr = DXGIFactory->EnumAdapters(CurrentAdapter,Adapter.GetInitReference());

	if( DXGI_ERROR_NOT_FOUND == hr )
		return FALSE;
	if( FAILED(hr) )
		return FALSE;

	// get the description of the adapter
	DXGI_ADAPTER_DESC AdapterDesc;
	VERIFYD3D10RESULT(Adapter->GetDesc(&AdapterDesc));

	INT CurrentOutput = 0;
	do 
	{
		TRefCountPtr<IDXGIOutput> Output;
		hr = Adapter->EnumOutputs(CurrentOutput,Output.GetInitReference());
		if(DXGI_ERROR_NOT_FOUND == hr)
			break;
		if(FAILED(hr))
			return FALSE;

		// TODO: GetDisplayModeList is a terribly SLOW call.  It can take up to a second per invocation.
		//  We might want to work around some DXGI badness here.
		DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		UINT NumModes = 0;
		hr = Output->GetDisplayModeList(Format, 0, &NumModes, NULL);
		if(hr == DXGI_ERROR_NOT_FOUND)
		{
			continue;
		}
		else if(hr == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
		{
			appErrorf(
				NAME_FriendlyError,
				TEXT("This application cannot be run over a remote desktop configuration")
				);
			return FALSE;
		}

		DXGI_MODE_DESC* ModeList = new DXGI_MODE_DESC[ NumModes ];
		VERIFYD3D10RESULT(Output->GetDisplayModeList(Format, 0, &NumModes, ModeList));

		for(UINT m = 0;m < NumModes;m++)
		{
			if (((INT)ModeList[m].Width >= MinAllowableResolutionX) &&
				((INT)ModeList[m].Width <= MaxAllowableResolutionX) &&
				((INT)ModeList[m].Height >= MinAllowableResolutionY) &&
				((INT)ModeList[m].Height <= MaxAllowableResolutionY)
				)
			{
				UBOOL bAddIt = TRUE;
				if (bIgnoreRefreshRate == FALSE)
				{
					if (((INT)ModeList[m].RefreshRate.Numerator < MinAllowableRefreshRate * ModeList[m].RefreshRate.Denominator) ||
						((INT)ModeList[m].RefreshRate.Numerator > MaxAllowableRefreshRate * ModeList[m].RefreshRate.Denominator)
						)
					{
						continue;
					}
				}
				else
				{
					// See if it is in the list already
					for (INT CheckIndex = 0; CheckIndex < Resolutions.Num(); CheckIndex++)
					{
						FScreenResolutionRHI& CheckResolution = Resolutions(CheckIndex);
						if ((CheckResolution.Width == ModeList[m].Width) &&
							(CheckResolution.Height == ModeList[m].Height))
						{
							// Already in the list...
							bAddIt = FALSE;
							break;
						}
					}
				}

				if (bAddIt)
				{
					// Add the mode to the list
					INT Temp2Index = Resolutions.AddZeroed();
					FScreenResolutionRHI& ScreenResolution = Resolutions(Temp2Index);

					ScreenResolution.Width = ModeList[m].Width;
					ScreenResolution.Height = ModeList[m].Height;
					ScreenResolution.RefreshRate = ModeList[m].RefreshRate.Numerator / ModeList[m].RefreshRate.Denominator;
				}
			}
		}

		delete[] ModeList;

		++CurrentOutput;

	// TODO: Cap at 1 for default output
	} while(CurrentOutput < 1);

	return TRUE;
}

void FD3D10DynamicRHI::ReleaseCachedD3D10States()
{
	CachedSamplers.Empty();
	CachedDepthStencilStates.Empty();
	CachedRasterizerStates.Empty();
	CachedBlendStates.Empty();

	// Null out the temporaries that we were using to keep track of current state
	CurrentDepthState = D3D10_DEPTH_STENCIL_DESC();
	CurrentStencilState = D3D10_DEPTH_STENCIL_DESC();
	CurrentRasterizerState = D3D10_RASTERIZER_DESC();
	CurrentBlendState = D3D10_BLEND_DESC();
	CurrentBlendFactor = FLinearColor(0,0,0,0);
	CurrentStencilRef = 0;
}

/**
 * Gets
 */
ID3D10SamplerState* FD3D10DynamicRHI::GetCachedSamplerState( FSamplerStateRHIParamRef SamplerStateRHI )
{
	DYNAMIC_CAST_D3D10RESOURCE(SamplerState,SamplerState);

	FSamplerKey SamplerKey(SamplerState);
#if !LET_D3D10_CACHE_STATE
	TRefCountPtr<ID3D10SamplerState>* FoundSampler = CachedSamplers.Find(SamplerKey);
	if(FoundSampler)
	{
		return *FoundSampler;
	}
	else
#endif
	{
		D3D10_SAMPLER_DESC Desc;
		if(SamplerState)
			appMemcpy(&Desc,&SamplerState->SamplerDesc,sizeof(D3D10_SAMPLER_DESC));
		else
			appMemzero(&Desc,sizeof(D3D10_SAMPLER_DESC));

		ID3D10SamplerState* NewState = NULL;
		VERIFYD3D10RESULT(Direct3DDevice->CreateSamplerState(&Desc,&NewState));

		// Add the key to the list so we can find it next time
		CachedSamplers.Set(SamplerKey,NewState);
		return NewState;
	}
}

ID3D10DepthStencilState* FD3D10DynamicRHI::GetCachedDepthStencilState( const D3D10_DEPTH_STENCIL_DESC& DepthState, const D3D10_DEPTH_STENCIL_DESC& StencilState )
{
	FDepthStencilKey DepthStencilKey(DepthState,StencilState);
#if !LET_D3D10_CACHE_STATE
	TRefCountPtr<ID3D10DepthStencilState>* FoundDepthStencil = CachedDepthStencilStates.Find(DepthStencilKey);
	if(FoundDepthStencil)
	{
		return *FoundDepthStencil;
	}
	else
#endif
	{
		D3D10_DEPTH_STENCIL_DESC Desc;
		appMemcpy(&Desc,&StencilState,sizeof(D3D10_DEPTH_STENCIL_DESC));
		Desc.DepthEnable = DepthState.DepthEnable;
		Desc.DepthWriteMask = DepthState.DepthWriteMask;
		Desc.DepthFunc = DepthState.DepthFunc;

		ID3D10DepthStencilState* NewState = NULL;
		VERIFYD3D10RESULT(Direct3DDevice->CreateDepthStencilState(&Desc,&NewState));

		// Add the key to the list so we can find it next time
		CachedDepthStencilStates.Set(DepthStencilKey,NewState);
		return NewState;
	}
}

ID3D10RasterizerState* FD3D10DynamicRHI::GetCachedRasterizerState( const D3D10_RASTERIZER_DESC& InRasterizerState,UBOOL bScissorEnabled, UBOOL bMultisampleEnable )
{
	D3D10_RASTERIZER_DESC RasterizerState = InRasterizerState;

	// Verify that D3D10_RASTERIZER_DESC has valid settings.  The initial zeroed values in CurrentRasterizerDesc aren't valid.
	if(RasterizerState.CullMode == 0)
	{
		RasterizerState.CullMode = D3D10_CULL_NONE;
	}
	if(RasterizerState.FillMode == 0)
	{
		RasterizerState.FillMode = D3D10_FILL_SOLID;
	}

	// Convert our platform independent depth bias into a D3D10 depth bias.
	extern FLOAT GDepthBiasOffset;
	const INT D3DDepthBias = RasterizerState.DepthBias + appFloor(GDepthBiasOffset * (FLOAT)(1 << 24));

	FRasterizerKey RasterizerKey(RasterizerState,D3DDepthBias,bScissorEnabled,bMultisampleEnable);
#if !LET_D3D10_CACHE_STATE
	TRefCountPtr<ID3D10RasterizerState>* FoundRasterizer = CachedRasterizerStates.Find(RasterizerKey);
	if(FoundRasterizer)
	{
		return *FoundRasterizer;
	}
	else
#endif
	{
		RasterizerState.ScissorEnable = bScissorEnabled;
		RasterizerState.MultisampleEnable = bMultisampleEnable;
		RasterizerState.DepthBias = D3DDepthBias;

		ID3D10RasterizerState* NewState = NULL;
		VERIFYD3D10RESULT(Direct3DDevice->CreateRasterizerState(&RasterizerState,&NewState));

		// Add the key to the list so we can find it next time
		CachedRasterizerStates.Set(RasterizerKey,NewState);
		return NewState;
	}
}

ID3D10BlendState* FD3D10DynamicRHI::GetCachedBlendState( const D3D10_BLEND_DESC& BlendState, UINT8 EnabledStateValue )
{
	FBlendKey BlendKey(BlendState,EnabledStateValue);
#if !LET_D3D10_CACHE_STATE
	TRefCountPtr<ID3D10BlendState>* FoundBlend = CachedBlendStates.Find(BlendKey);
	if(FoundBlend)
	{
		return *FoundBlend;
	}
	else
#endif
	{
		D3D10_BLEND_DESC Desc;
		appMemcpy(&Desc,&BlendState,sizeof(D3D10_BLEND_DESC));
		Desc.RenderTargetWriteMask[0] = EnabledStateValue;

		ID3D10BlendState* NewState = NULL;
		VERIFYD3D10RESULT(Direct3DDevice->CreateBlendState(&Desc,&NewState));

		// Add the key to the list so we can find it next time
		CachedBlendStates.Set(BlendKey,NewState);
		return NewState;
	}
}

void FD3D10DynamicRHI::CleanupD3DDevice()
{
	if(GIsRHIInitialized)
	{
		check(Direct3DDevice);

		// Reset the RHI initialized flag.
		GIsRHIInitialized = FALSE;

		// Ask all initialized FRenderResources to release their RHI resources.
		for(TLinkedList<FRenderResource*>::TIterator ResourceIt(FRenderResource::GetResourceList());ResourceIt;ResourceIt.Next())
		{
			ResourceIt->ReleaseRHI();
		}
		for(TLinkedList<FRenderResource*>::TIterator ResourceIt(FRenderResource::GetResourceList());ResourceIt;ResourceIt.Next())
		{
			ResourceIt->ReleaseDynamicRHI();
		}

		// release our state cache
		ReleaseCachedD3D10States();
		// release our dynamic VB and IB buffers
		ReleaseDynamicVBandIBBuffers();

		// Release the device.
		Direct3DDevice = NULL;
	}
}


/**
 *	Sets the maximum viewport size the application is expecting to need for the time being, or zero for
 *	no preference.  This is used as a hint to the RHI to reduce redundant device resets when viewports
 *	are created or destroyed (typically in the editor.)
 *
 *	@param NewLargestExpectedViewportWidth Maximum width of all viewports (or zero if not known)
 *	@param NewLargestExpectedViewportHeight Maximum height of all viewports (or zero if not known)
 */
void FD3D10DynamicRHI::SetLargestExpectedViewportSize( UINT NewLargestExpectedViewportWidth,
													   UINT NewLargestExpectedViewportHeight )
{
	// @todo: Add support for this after we add robust support for editor viewports in D3D10 (similar to D3D9)
}
