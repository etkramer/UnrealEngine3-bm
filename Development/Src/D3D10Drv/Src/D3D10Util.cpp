/*=============================================================================
	D3D10Util.h: D3D RHI utility implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3D10DrvPrivate.h"

FString GetD3D10ErrorString(HRESULT ErrorCode)
{
	FString ErrorCodeText;
#define D3DERR(x) case x: ErrorCodeText = TEXT(#x); break;
	switch(ErrorCode)
	{
		D3DERR(S_OK);
		D3DERR(D3D10_ERROR_FILE_NOT_FOUND)
		D3DERR(D3D10_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS)
		D3DERR(D3DERR_INVALIDCALL)
		D3DERR(D3DERR_WASSTILLDRAWING)
		D3DERR(E_FAIL)
		D3DERR(E_INVALIDARG)
		D3DERR(E_OUTOFMEMORY)
		default: ErrorCodeText = FString::Printf(TEXT("%08X"),(INT)ErrorCode);
	}
#undef D3DERR
	return ErrorCodeText;
}

FString GetD3D10TextureFormatString(DXGI_FORMAT TextureFormat)
{
	FString TextureFormatText;
#define D3DFORMATCASE(x) case x: TextureFormatText = TEXT(#x); break;
	switch(TextureFormat)
	{
		D3DFORMATCASE(DXGI_FORMAT_R8G8B8A8_UNORM)
		D3DFORMATCASE(DXGI_FORMAT_B8G8R8X8_UNORM )
		D3DFORMATCASE(DXGI_FORMAT_BC1_UNORM)
		D3DFORMATCASE(DXGI_FORMAT_BC2_UNORM)
		D3DFORMATCASE(DXGI_FORMAT_BC3_UNORM)
		D3DFORMATCASE(DXGI_FORMAT_R16G16B16A16_FLOAT)
		D3DFORMATCASE(DXGI_FORMAT_R32G32B32A32_FLOAT)
		D3DFORMATCASE(DXGI_FORMAT_UNKNOWN)
		D3DFORMATCASE(DXGI_FORMAT_R8_UNORM)
		D3DFORMATCASE(DXGI_FORMAT_D24_UNORM_S8_UINT)
		D3DFORMATCASE(DXGI_FORMAT_R24_UNORM_X8_TYPELESS)
		D3DFORMATCASE(DXGI_FORMAT_R32_FLOAT)
		D3DFORMATCASE(DXGI_FORMAT_R16G16_UINT)
		D3DFORMATCASE(DXGI_FORMAT_R16G16_FLOAT)
		D3DFORMATCASE(DXGI_FORMAT_R32G32_FLOAT)
		D3DFORMATCASE(DXGI_FORMAT_R10G10B10A2_UNORM)
		D3DFORMATCASE(DXGI_FORMAT_R16G16B16A16_UINT)
		default: TextureFormatText = FString::Printf(TEXT("%08X"),(INT)TextureFormat);
	}
#undef D3DFORMATCASE
	return TextureFormatText;
}

FString GetD3D10TextureFlagString(DWORD TextureFlags)
{
	FString TextureFormatText = TEXT("");

	if (TextureFlags & D3D10_BIND_RENDER_TARGET)
	{
		TextureFormatText += TEXT("D3D10_BIND_RENDER_TARGET ");
	}

	if (TextureFlags & D3D10_BIND_DEPTH_STENCIL)
	{
		TextureFormatText += TEXT("D3D10_BIND_DEPTH_STENCIL ");
	}

	if (TextureFlags & D3D10_BIND_SHADER_RESOURCE)
	{
		TextureFormatText += TEXT("D3D10_BIND_SHADER_RESOURCE ");
	}

	return TextureFormatText;
}

void VerifyD3D10Result(HRESULT D3DResult,const ANSICHAR* Code,const ANSICHAR* Filename,UINT Line)
{
	if(FAILED(D3DResult))
	{
		const FString& ErrorString = GetD3D10ErrorString(D3DResult);
		appErrorf(TEXT("%s failed \n at %s:%u \n with error %s"),ANSI_TO_TCHAR(Code),ANSI_TO_TCHAR(Filename),Line,*ErrorString);
	}
}

void VerifyD3D10CreateTextureResult(HRESULT D3DResult,const ANSICHAR* Code,const ANSICHAR* Filename,UINT Line,UINT SizeX,UINT SizeY,BYTE Format,UINT NumMips,DWORD Flags)
{
	if(FAILED(D3DResult))
	{
		const FString& ErrorString = GetD3D10ErrorString(D3DResult);
		const FString& D3DFormatString = GetD3D10TextureFormatString((DXGI_FORMAT)GPixelFormats[Format].PlatformFormat);
		//const FString& D3DFormatString = GetD3DTextureFormatString((D3DFORMAT)GPixelFormats[Format].PlatformFormat);
		appErrorf(
			TEXT("%s failed \n at %s:%u \n with error %s, \n SizeX=%i, SizeY=%i, Format=%s=%s, NumMips=%i, Flags=%s, TexMemoryAvailable=%dMB"),
			ANSI_TO_TCHAR(Code),
			ANSI_TO_TCHAR(Filename),
			Line,
			*ErrorString, 
			SizeX, 
			SizeY, 
			GPixelFormats[Format].Name, 
			*D3DFormatString, NumMips, 
			*GetD3D10TextureFlagString(Flags),
			RHIGetAvailableTextureMemory());
	}
}

#if 0
/**
 * Adds a PIX event using the D3DPerf api
 *
 * @param Color The color to draw the event as
 * @param Text The text displayed with the event
 */
void appBeginDrawEvent(const FColor& Color,const TCHAR* Text)
{
	//TODO: Find the analog for D3D10
	//D3DPERF_BeginEvent(Color.DWColor(),Text);
}

/**
 * Ends the current PIX event
 */
void appEndDrawEvent(void)
{
	//TODO: Find the analog for D3D10
	//D3DPERF_EndEvent();
}

/**
 * Platform specific function for setting the value of a counter that can be
 * viewed in PIX.
 */
void appSetCounterValue(const TCHAR* CounterName, FLOAT Value)
{
}

#endif

//
// Stat declarations.
//

DECLARE_STATS_GROUP(TEXT("D3D10RHI"),STATGROUP_D3D10RHI);
DECLARE_CYCLE_STAT(TEXT("Present time"),STAT_D3D10PresentTime,STATGROUP_D3D10RHI);
DECLARE_DWORD_COUNTER_STAT(TEXT("DrawPrimitive calls"),STAT_D3D10DrawPrimitiveCalls,STATGROUP_D3D10RHI);
DECLARE_DWORD_COUNTER_STAT(TEXT("Triangles drawn"),STAT_D3D10Triangles,STATGROUP_D3D10RHI);
DECLARE_DWORD_COUNTER_STAT(TEXT("Lines drawn"),STAT_D3D10Lines,STATGROUP_D3D10RHI);
DECLARE_CYCLE_STAT(TEXT("CreateTexture time"),STAT_D3D10CreateTextureTime,STATGROUP_D3D10RHI);
DECLARE_CYCLE_STAT(TEXT("LockTexture time"),STAT_D3D10LockTextureTime,STATGROUP_D3D10RHI);
DECLARE_CYCLE_STAT(TEXT("UnlockTexture time"),STAT_D3D10UnlockTextureTime,STATGROUP_D3D10RHI);
DECLARE_CYCLE_STAT(TEXT("CopyTexture time"),STAT_D3D10CopyTextureTime,STATGROUP_D3D10RHI);
DECLARE_CYCLE_STAT(TEXT("CopyMipToMipAsync time"),STAT_D3D10CopyMipToMipAsyncTime,STATGROUP_D3D10RHI);
DECLARE_CYCLE_STAT(TEXT("UploadTextureMip time"),STAT_D3D10UploadTextureMipTime,STATGROUP_D3D10RHI);
DECLARE_CYCLE_STAT(TEXT("CreateBoundShaderState time"),STAT_D3D10CreateBoundShaderStateTime,STATGROUP_D3D10RHI);
DECLARE_CYCLE_STAT(TEXT("Constant buffer update time"),STAT_D3D10ConstantBufferUpdateTime,STATGROUP_D3D10RHI);
