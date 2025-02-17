/*=============================================================================
	D3DRenderTarget.cpp: D3D render target implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "XeD3DDrvPrivate.h"
#include "BatchedElements.h"
#include "ScreenRendering.h"

#if USE_XeD3D_RHI

extern void OffsetRenderTargetBase(UINT, UINT, UINT);

// bias all 7e3 target writes in order to get better precision
const INT SCENE_COLOR_BIAS_FACTOR_EXP = 3;

/** 
* @return D3D render target format for unreal Format 
*/
D3DFORMAT XeGetRenderTargetFormat(BYTE Format)
{
	D3DFORMAT D3DFormat = (D3DFORMAT)GPixelFormats[Format].PlatformFormat;
    switch( Format )
	{
	case PF_FloatRGB:
		D3DFormat = D3DFMT_A2B10G10R10F_EDRAM;
		break;
	case PF_FloatRGBA:
		D3DFormat = D3DFMT_A16B16G16R16F;
		break;
	case PF_G16R16:
		D3DFormat = D3DFMT_G16R16_EDRAM;
		break;
	case PF_G16R16F:
	case PF_G16R16F_FILTER:
		D3DFormat = D3DFMT_G16R16F;
		break;
	case PF_A16B16G16R16:
		D3DFormat = D3DFMT_A16B16G16R16_EDRAM;
		break;
	case PF_R32F:
		D3DFormat = D3DFMT_R32F;
		break;
	}
	return D3DFormat;
}

/** 
* @return TRUE if D3DFormat is a depth format 
*/
UBOOL XeIsDepthFormat(D3DFORMAT D3DFormat)
{
	switch( D3DFormat )
	{
	case D3DFMT_D24S8:
	case D3DFMT_LIN_D24S8:
	case D3DFMT_D24X8:
	case D3DFMT_LIN_D24X8:
	case D3DFMT_D24FS8:
	case D3DFMT_LIN_D24FS8:
	case D3DFMT_D32:
	case D3DFMT_LIN_D32:
		return TRUE;
	};
	return FALSE;
}

/**
* Certain render targets assume that we will use a bias factor when rendering to them 
* and when resolving them.
* @param D3DFormat - render target format to specify bias
* @return bias to apply when resolving the given render target format
*/
INT XeGetRenderTargetColorExpBias(D3DFORMAT SurfaceD3DFormat, D3DFORMAT TextureD3DFormat)
{
	if (SurfaceD3DFormat == D3DFMT_A2B10G10R10F_EDRAM && TextureD3DFormat == XeGetTextureFormat(PF_FloatRGB, 0))
	{
		return SCENE_COLOR_BIAS_FACTOR_EXP;
	}
	else if (SurfaceD3DFormat == D3DFMT_A2B10G10R10F_EDRAM && TextureD3DFormat == D3DFMT_A2B10G10R10)
	{
		// Resolving from D3DFMT_A2B10G10R10F_EDRAM to D3DFMT_A2B10G10R10 requires an exp bias of -5 to convert [0, 32] to [0, 1] (the range of D3DFMT_A2B10G10R10)
		return 5;
	}

	// 2^0 = 1 = (no bias)
	return 0;
}

INT XeGetTextureExpBias(D3DFORMAT SurfaceD3DFormat, D3DFORMAT TextureD3DFormat)
{
	if (SurfaceD3DFormat == D3DFMT_A2B10G10R10F_EDRAM && TextureD3DFormat == D3DFMT_A2B10G10R10)
	{
		// When sampling from D3DFMT_A2B10G10R10 we need to multiply [0, 1] by 4 to get the original range of [0, 4]
		return 2;
	}

	// 2^0 = 1 = (no bias)
	return 0;
}

/**
* Creates a RHI surface that can be bound as a render target.
* Note that a surface cannot be created which is both resolvable AND readable.
* @param SizeX - The width of the surface to create.
* @param SizeY - The height of the surface to create.
* @param Format - The surface format to create.
* @param ResolveTargetTexture - The texture which the surface will be resolved to.  It must have been allocated with bResolveTargetable=TRUE
* @param Flags - Surface creation flags
* @param UsageStr - Text describing usage for this surface
* @return The surface that was created.
*/
FSurfaceRHIRef RHICreateTargetableSurface(
	UINT SizeX,
	UINT SizeY,
	BYTE Format,
	FTexture2DRHIParamRef ResolveTargetTexture,
	DWORD Flags,
	const TCHAR* UsageStr
	)
{
	// resulting resource ref
	TRefCountPtr<IDirect3DSurface9> SurfaceRef;

	// device format
	D3DFORMAT SurfaceD3DFormat = XeGetRenderTargetFormat(Format);
	D3DSURFACE_DESC SurfaceDesc;
	D3DFORMAT TextureD3DFormat = XeGetTextureFormat(Format, 0);
	if (ResolveTargetTexture != NULL)
	{
		((IDirect3DTexture9*)ResolveTargetTexture->Resource)->GetLevelDesc(0, &SurfaceDesc);
		TextureD3DFormat = SurfaceDesc.Format;
	}

	// surface params for render target creation
	D3DSURFACE_PARAMETERS SurfaceParameters = { 0 };
	DWORD SurfaceSize=0;
	
	if( XeIsDepthFormat(SurfaceD3DFormat) )
	{
		// size of surface in EDRAM tiles
		SurfaceSize = XGSurfaceSize( SizeX, SizeY, SurfaceD3DFormat, D3DMULTISAMPLE_NONE);
		// offset to start of available edram region		
		SurfaceParameters.Base = XeEDRAMOffset(UsageStr,SurfaceSize);
		// reserve HiZ memory for depth buffers
		SurfaceParameters.HierarchicalZBase = XeHiZOffset(UsageStr, XGHierarchicalZSize(SizeX,SizeY,D3DMULTISAMPLE_NONE));

		VERIFYD3DRESULT( GDirect3DDevice->CreateDepthStencilSurface( 
			SizeX, 
			SizeY, 
			SurfaceD3DFormat, 
			D3DMULTISAMPLE_NONE, 
			0, 
			FALSE, 
			SurfaceRef.GetInitReference(), 
			&SurfaceParameters ));
	}
	else
	{
		// size of surface in EDRAM tiles
		SurfaceSize = XGSurfaceSize( SizeX, SizeY, SurfaceD3DFormat, D3DMULTISAMPLE_NONE);
		// offset to start of available edram region		
		SurfaceParameters.Base = XeEDRAMOffset(UsageStr,SurfaceSize);
		// create render target edram surface
		VERIFYD3DRESULT( GDirect3DDevice->CreateRenderTarget( 
			SizeX, 
			SizeY, 
			SurfaceD3DFormat, 
			D3DMULTISAMPLE_NONE, 
			0, 
			0, 
			SurfaceRef.GetInitReference(), 
			&SurfaceParameters ));
	}

	return FSurfaceRHIRef(ResolveTargetTexture,SurfaceRef,FXeSurfaceInfo(SurfaceParameters.Base,SurfaceSize,XeGetRenderTargetColorExpBias(SurfaceD3DFormat, TextureD3DFormat)));
}

/**
* Creates a RHI surface that can be bound as a render target and can resolve w/ a cube texture
* Note that a surface cannot be created which is both resolvable AND readable.
* @param SizeX - The width of the surface to create.
* @param Format - The surface format to create.
* @param ResolveTargetTexture - The cube texture which the surface will be resolved to.  It must have been allocated with bResolveTargetable=TRUE
* @param CubeFace - face from resolve texture to use as surface
* @param Flags - Surface creation flags
* @param UsageStr - Text describing usage for this surface
* @return The surface that was created.
*/
FSurfaceRHIRef RHICreateTargetableCubeSurface(
	UINT SizeX,
	BYTE Format,
	FTextureCubeRHIParamRef ResolveTargetTexture,
	ECubeFace CubeFace,
	DWORD Flags,
	const TCHAR* UsageStr
	)
{
	// resulting resource ref
	TRefCountPtr<IDirect3DSurface9> SurfaceRef;

	// device format
	D3DFORMAT SurfaceD3DFormat = XeGetRenderTargetFormat(Format);
	D3DSURFACE_DESC SurfaceDesc;
	D3DFORMAT TextureD3DFormat = XeGetTextureFormat(Format, 0);
	if (ResolveTargetTexture != NULL)
	{
		((IDirect3DTexture9*)ResolveTargetTexture->Resource)->GetLevelDesc(0, &SurfaceDesc);
		TextureD3DFormat = SurfaceDesc.Format;
	}

	// surface params for render target creation
	D3DSURFACE_PARAMETERS SurfaceParameters = { 0 };
	DWORD SurfaceSize=0;

	if( XeIsDepthFormat(SurfaceD3DFormat) )
	{
		// size of surface in EDRAM tiles
		SurfaceSize = XGSurfaceSize( SizeX, SizeX, SurfaceD3DFormat, D3DMULTISAMPLE_NONE);
		// offset to start of available edram region
		SurfaceParameters.Base = XeEDRAMOffset(UsageStr,SurfaceSize);
		// reserve HiZ memory for depth buffers
		SurfaceParameters.HierarchicalZBase = XeHiZOffset(UsageStr, XGHierarchicalZSize(SizeX,SizeX,D3DMULTISAMPLE_NONE));;

		VERIFYD3DRESULT( GDirect3DDevice->CreateDepthStencilSurface( 
			SizeX, 
			SizeX, 
			SurfaceD3DFormat, 
			D3DMULTISAMPLE_NONE, 
			0, 
			FALSE, 
			SurfaceRef.GetInitReference(), 
			&SurfaceParameters ));
	}
	else
	{
		// size of surface in EDRAM tiles
		SurfaceSize = XGSurfaceSize( SizeX, SizeX, SurfaceD3DFormat, D3DMULTISAMPLE_NONE);
		// offset to start of available edram region		
		SurfaceParameters.Base = XeEDRAMOffset(UsageStr,SurfaceSize);
		// create render target edram surface
		VERIFYD3DRESULT( GDirect3DDevice->CreateRenderTarget( 
			SizeX, 
			SizeX, 
			SurfaceD3DFormat, 
			D3DMULTISAMPLE_NONE, 
			0, 
			0, 
			SurfaceRef.GetInitReference(), 
			&SurfaceParameters ));
	}

	return FSurfaceRHIRef(ResolveTargetTexture,SurfaceRef,FXeSurfaceInfo(SurfaceParameters.Base,SurfaceSize,XeGetRenderTargetColorExpBias(SurfaceD3DFormat, TextureD3DFormat)));
}

/**
* Reads the contents of a surface to an output buffer.
*/
void RHIReadSurfaceData(FSurfaceRHIParamRef InSurface,UINT MinX,UINT MinY,UINT MaxX,UINT MaxY,TArray<BYTE>& OutData,ECubeFace CubeFace)
{
	FSurfaceRHIRef Surface = InSurface;

	UINT SizeX = MaxX - MinX + 1;
	UINT SizeY = MaxY - MinY + 1;

	check(InSurface != NULL);
	check(!IsValidRef(Surface.ResolveTargetTextureCube));

	// surface must have a texture to resolve to
	UBOOL bNeedsResolveTexture = !IsValidRef(Surface.ResolveTargetTexture2D);
	if( bNeedsResolveTexture )
	{
        D3DSURFACE_DESC SrcSurfaceDesc;
		Surface->GetDesc(&SrcSurfaceDesc);
        Surface.ResolveTargetTexture2D = RHICreateTexture2D(SrcSurfaceDesc.Width,SrcSurfaceDesc.Height,PF_A8R8G8B8,1,TexCreate_ResolveTargetable,NULL);
	}
	IDirect3DTexture9* Texture2D(Surface.ResolveTargetTexture2D);

	// resolve surface to it's texture
	RHICopyToResolveTarget(Surface,TRUE,FResolveParams());

	// read the surface from the texture's first level
	TRefCountPtr<IDirect3DSurface9> SourceSurface;
	VERIFYD3DRESULT(Texture2D->GetSurfaceLevel(0,SourceSurface.GetInitReference()));

	// Check the format of the surface. Only handle RGBA at the moment
	D3DSURFACE_DESC SurfaceDesc;
	VERIFYD3DRESULT(SourceSurface->GetDesc(&SurfaceDesc));
	check(SurfaceDesc.Format == D3DFMT_A8R8G8B8);

	// Allocate the output buffer.
	OutData.Empty((MaxX - MinX + 1) * (MaxY - MinY + 1) * sizeof(FColor));

	// Read back the surface data from (MinX,MinY) to (MaxX,MaxY)
	D3DLOCKED_RECT	LockedRect;
	RECT			Rect;
	Rect.left	= MinX;
	Rect.top	= MinY;
	Rect.right	= MaxX + 1;
	Rect.bottom	= MaxY + 1;
	VERIFYD3DRESULT(SourceSurface->LockRect(&LockedRect,&Rect,0/*D3DLOCK_READONLY*/));

	// Untile if necessary.
	BYTE*	UntiledImage = NULL;
	if( XGIsTiledFormat(SurfaceDesc.Format) )
	{
		XGTEXTURE_DESC	TextureDesc;
		XGGetTextureDesc( Texture2D, 0, &TextureDesc );

		RECT SrcRect;
		SrcRect.top		= 0;
		SrcRect.bottom	= TextureDesc.Height;
		SrcRect.left	= 0;
		SrcRect.right	= TextureDesc.Width;
		
		UntiledImage = new BYTE[ TextureDesc.Height * TextureDesc.RowPitch ];

		XGUntileSurface( 
			UntiledImage, 
			TextureDesc.RowPitch, 
			NULL, 
			LockedRect.pBits, 
			TextureDesc.Width,
			TextureDesc.Height, 
			&SrcRect, 
			TextureDesc.BitsPerPixel / 8 
			);

		LockedRect.pBits = UntiledImage;
	}

	for(UINT Y = MinY;Y <= MaxY;Y++)
	{
		BYTE* SrcPtr = (BYTE*)LockedRect.pBits + (Y - MinY) * LockedRect.Pitch;
		BYTE* DestPtr = (BYTE*)&OutData(OutData.Add(SizeX * sizeof(FColor)));
		appMemcpy(DestPtr,SrcPtr,SizeX * sizeof(FColor));
	}

	delete [] UntiledImage;
	VERIFYD3DRESULT(SourceSurface->UnlockRect());

	if( bNeedsResolveTexture )
	{
		Surface.ResolveTargetTexture2D.SafeRelease();
		Surface.ResolveTargetTexture2D = FTexture2DRHIRef();
	}
}

/**
* Copies the contents of the given surface to its resolve target texture.
* @param SourceSurface - surface with a resolve texture to copy to
* @param bKeepOriginalSurface - TRUE if the original surface will still be used after this function so must remain valid
* @param ResolveParams - optional resolve params
*/
void RHICopyToResolveTarget(FSurfaceRHIParamRef SourceSurface, UBOOL bKeepOriginalSurface, const FResolveParams& ResolveParams)
{
	FTextureRHIParamRef ResolveTarget2D = ResolveParams.ResolveTarget ? ResolveParams.ResolveTarget : SourceSurface.ResolveTargetTexture2D;

	if( ResolveTarget2D || IsValidRef(SourceSurface.ResolveTargetTextureCube) )
	{
		// save the current viewport so that it can be restored
		D3DVIEWPORT9 SavedViewport;
		GDirect3DDevice->GetViewport(&SavedViewport);

		// get surface info
		D3DSURFACE_DESC SurfDesc;
		VERIFYD3DRESULT(SourceSurface->GetDesc( &SurfDesc ));

		// handle optional resolve rect
		D3DRECT SrcRect;
		D3DPOINT DestPoint;
		const D3DRECT* SrcRectPtr = NULL;
		const D3DPOINT* DestPointPtr = NULL;
		if( ResolveParams.X1 >= 0 && ResolveParams.X2 >= 0 && ResolveParams.Y1 >= 0 && ResolveParams.Y2 >= 0 )
		{
			// each coordinate of the source rect must be aligned to D3DRESOLVEALIGNMENT
			SrcRect.x1 = Max(Align(ResolveParams.X1-D3DRESOLVEALIGNMENT,D3DRESOLVEALIGNMENT),0);
			SrcRect.y1 = Max(Align(ResolveParams.Y1-D3DRESOLVEALIGNMENT,D3DRESOLVEALIGNMENT),0);
            // rect dimensions must be aligned to 32
			INT AlignedSizeX = Align((ResolveParams.X2 - SrcRect.x1),32);
			INT AlignedSizeY = Align((ResolveParams.Y2 - SrcRect.y1),32);
			SrcRect.x2 = Min<INT>(SrcRect.x1 + AlignedSizeX,SurfDesc.Width);
			SrcRect.y2 = Min<INT>(SrcRect.y1 + AlignedSizeY,SurfDesc.Height);
			// destination point always matching source rect origin
			DestPoint.x = SrcRect.x1;
			DestPoint.y = SrcRect.y1;
			// only set rect ptr if valid
			SrcRectPtr = &SrcRect;
			DestPointPtr = &DestPoint;
		}		
		if(GUseMSAASplitScreen)
		{
			// remove render target offset if present
			OffsetRenderTargetBase(0, 0, 0);
		}
		if( XeIsDepthFormat(SurfDesc.Format) )
		{
			// save current render target so it can be restored
			TRefCountPtr<IDirect3DSurface9> SavedDepthTarget;
			GDirect3DDevice->GetDepthStencilSurface(SavedDepthTarget.GetInitReference());
			// set to surface that needs resolving
			GDirect3DDevice->SetDepthStencilSurface(SourceSurface);
			// 2d resolve texture
			if( IsValidRef(SourceSurface.ResolveTargetTexture2D) )
			{
				// resolve EDRAM surface to texture in memory
				VERIFYD3DRESULT( GDirect3DDevice->Resolve( 
					D3DRESOLVE_DEPTHSTENCIL, 
					SrcRectPtr, 
					SourceSurface.ResolveTargetTexture2D, 
					DestPointPtr, 
					0, 
					0, 
					NULL, 
					0, 
					0, 
					NULL ));
			}
			// cube resolve texture
			else
			{
				// resolve EDRAM surface to texture in memory
				VERIFYD3DRESULT( GDirect3DDevice->Resolve( 
					D3DRESOLVE_DEPTHSTENCIL, 
					SrcRectPtr, 
					SourceSurface.ResolveTargetTextureCube, 
					DestPointPtr, 
					0, 
					GetD3DCubeFace(ResolveParams.CubeFace), 
					NULL, 
					0, 
					0, 
					NULL ));
			}
			// reset saved target
			GDirect3DDevice->SetDepthStencilSurface(SavedDepthTarget);
		}
		else
		{
			// save current render target so it can be restored
			TRefCountPtr<IDirect3DSurface9> SavedRenderTarget;
			GDirect3DDevice->GetRenderTarget(0, SavedRenderTarget.GetInitReference());
			// set to surface that needs resolving
			GDirect3DDevice->SetRenderTarget(0, SourceSurface);
			// 2d resolve texture
//			if( IsValidRef(SourceSurface.ResolveTargetTexture2D) )
			if( ResolveTarget2D )
			{
				IDirect3DTexture9* D3DTexture2D = (IDirect3DTexture9*)ResolveTarget2D->Resource;
				DWORD Flags = D3DRESOLVE_RENDERTARGET0 | D3DRESOLVE_EXPONENTBIAS(-SourceSurface.XeSurfaceInfo.GetColorExpBias());
				// resolve EDRAM surface to texture in memory
				VERIFYD3DRESULT( GDirect3DDevice->Resolve( 
					Flags, 
					SrcRectPtr, 
					D3DTexture2D, 
					DestPointPtr, 
					0, 
					0, 
					NULL, 
					0, 
					0, 
					NULL ));

				D3DSURFACE_DESC TextureDesc;
				D3DTexture2D->GetLevelDesc(0, &TextureDesc);
				D3DFORMAT TextureD3DFormat = TextureDesc.Format;

				D3DTexture2D->Format.ExpAdjust = XeGetTextureExpBias(SurfDesc.Format, TextureD3DFormat);
			}
			// cube resolve texture
			else
			{
				IDirect3DCubeTexture9* D3DTextureCube = (IDirect3DCubeTexture9*)SourceSurface.ResolveTargetTextureCube;
				DWORD Flags = D3DRESOLVE_RENDERTARGET0 | D3DRESOLVE_EXPONENTBIAS(-SourceSurface.XeSurfaceInfo.GetColorExpBias());
				// resolve EDRAM surface to texture in memory
				VERIFYD3DRESULT( GDirect3DDevice->Resolve( 
					Flags, 
					SrcRectPtr, 
					D3DTextureCube, 
					DestPointPtr, 
					0, 
					GetD3DCubeFace(ResolveParams.CubeFace), 
					NULL, 
					0, 
					0, 
					NULL ));
			}

			// reset saved target
			GDirect3DDevice->SetRenderTarget(0, SavedRenderTarget);
		}
		// reset saved viewport
		GDirect3DDevice->SetViewport(&SavedViewport);
	}		
}

/**
* Copies the contents of the given surface's resolve target texture back to the surface without doing
* anything to the pixels (no exponent correction, no gamma correction).
*
* If the surface isn't currently allocated, the copy may be deferred until the next time it is allocated.
*/
void RHICopyFromResolveTargetFast(FSurfaceRHIParamRef DestSurface)
{
	// only allowed to copy from the 2d resolve texture
	if( IsValidRef(DestSurface.ResolveTargetTexture2D) )
	{
		// Save the current viewport so that it can be restored
		D3DVIEWPORT9 SavedViewport;
		GDirect3DDevice->GetViewport(&SavedViewport);

		// Determine the destination surface size.
		D3DSURFACE_DESC DestSurfDesc;
		VERIFYD3DRESULT(DestSurface->GetDesc(&DestSurfDesc));

		// No alpha blending, no depth tests or writes, no stencil tests or writes, no backface culling.
		RHISetBlendState(TStaticBlendState<>::GetRHI());
		RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
		RHISetStencilState(TStaticStencilState<>::GetRHI());
		RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());

		// Save current render target so it can be restored
		TRefCountPtr<IDirect3DSurface9> SavedRenderTarget;
		GDirect3DDevice->GetRenderTarget(0, SavedRenderTarget.GetInitReference());

		// Set to surface that we will copy to
		GDirect3DDevice->SetRenderTarget(0, DestSurface);

		// Construct a temporary FTexture to represent the source surface.
		FTexture TempTexture;
		TempTexture.TextureRHI = DestSurface.ResolveTargetTexture2D;
		TempTexture.SamplerStateRHI = TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();

		// Generate the vertices used to copy from the source surface to the destination surface.
		FLOAT MinX = -1.f + (0.f - GPixelCenterOffset) / ((FLOAT)DestSurfDesc.Width * 0.5f);		
		FLOAT MinY = +1.f - (0.f - GPixelCenterOffset) / ((FLOAT)DestSurfDesc.Height * 0.5f);

		FLOAT MaxX = -1.f + ((FLOAT)DestSurfDesc.Width - GPixelCenterOffset) / ((FLOAT)DestSurfDesc.Width * 0.5f);		
		FLOAT MaxY = +1.f - ((FLOAT)DestSurfDesc.Height - GPixelCenterOffset) / ((FLOAT)DestSurfDesc.Height * 0.5f);

		DrawScreenQuad( MinX, MinY, 0.f, 0.f, MaxX, MaxY, 1.f, 1.f, &TempTexture );

		// Reset saved target
		GDirect3DDevice->SetRenderTarget(0, SavedRenderTarget);

		// Reset saved viewport
		GDirect3DDevice->SetViewport(&SavedViewport);
	}
}

/**
* Copies the contents of the given surface's resolve target texture back to the surface without doing
* anything to the pixels (no exponent correction, no gamma correction).
*
* If the surface isn't currently allocated, the copy may be deferred until the next time it is allocated.
*/
void RHICopyFromResolveTargetRectFast(FSurfaceRHIParamRef DestSurface,FLOAT X1,FLOAT Y1,FLOAT X2,FLOAT Y2)
{
	// only allowed to copy from the 2d resolve texture
	if( IsValidRef(DestSurface.ResolveTargetTexture2D) )
	{
		// Save the current viewport so that it can be restored
		D3DVIEWPORT9 SavedViewport;
		GDirect3DDevice->GetViewport(&SavedViewport);

		// Determine the destination surface size.
		D3DSURFACE_DESC DestSurfDesc;
		VERIFYD3DRESULT(DestSurface->GetDesc(&DestSurfDesc));

		// No alpha blending, no depth tests or writes, no stencil tests or writes, no backface culling.
		RHISetBlendState(TStaticBlendState<>::GetRHI());
		RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
		RHISetStencilState(TStaticStencilState<>::GetRHI());
		RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());

		// Save current render target so it can be restored
		TRefCountPtr<IDirect3DSurface9> SavedRenderTarget;
		GDirect3DDevice->GetRenderTarget(0, SavedRenderTarget.GetInitReference());

		// Set to surface that we will copy to
		GDirect3DDevice->SetRenderTarget(0, DestSurface);

		// Construct a temporary FTexture to represent the source surface.
		FTexture TempTexture;
		TempTexture.TextureRHI = DestSurface.ResolveTargetTexture2D;
		TempTexture.SamplerStateRHI = TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();

		// Generate the vertices used to copy from the source surface to the destination surface.
		FLOAT MinX = -1.f + (X1 - GPixelCenterOffset) / ((FLOAT)DestSurfDesc.Width * 0.5f);		
		FLOAT MinY = +1.f - (Y1 - GPixelCenterOffset) / ((FLOAT)DestSurfDesc.Height * 0.5f);

		FLOAT MaxX = -1.f + (X2 - GPixelCenterOffset) / ((FLOAT)DestSurfDesc.Width * 0.5f);		
		FLOAT MaxY = +1.f - (Y2 - GPixelCenterOffset) / ((FLOAT)DestSurfDesc.Height * 0.5f);

		DrawScreenQuad( MinX, MinY, 0.f, 0.f, MaxX, MaxY, 1.f, 1.f, &TempTexture );

		// Reset saved target
		GDirect3DDevice->SetRenderTarget(0, SavedRenderTarget);

		// Reset saved viewport
		GDirect3DDevice->SetViewport(&SavedViewport);
	}
}

/**
* Copies the contents of the given surface's resolve target texture back to the surface.
* If the surface isn't currently allocated, the copy may be deferred until the next time it is allocated.
*/
void RHICopyFromResolveTarget(FSurfaceRHIParamRef DestSurface)
{
	// only allowed to copy from the 2d resolve texture
	if( IsValidRef(DestSurface.ResolveTargetTexture2D) )
	{
		// Save the current viewport so that it can be restored
		D3DVIEWPORT9 SavedViewport;
		GDirect3DDevice->GetViewport(&SavedViewport);

		// Determine the destination surface size.
		D3DSURFACE_DESC DestSurfDesc;
		VERIFYD3DRESULT(DestSurface->GetDesc(&DestSurfDesc));

		// Generate the vertices used to copy from the source surface to the destination surface.
		FLOAT MinX = -1.f + (0.f - GPixelCenterOffset) / ((FLOAT)DestSurfDesc.Width * 0.5f);		
		FLOAT MinY = +1.f - (0.f - GPixelCenterOffset) / ((FLOAT)DestSurfDesc.Height * 0.5f);

		FLOAT MaxX = -1.f + ((FLOAT)DestSurfDesc.Width - GPixelCenterOffset) / ((FLOAT)DestSurfDesc.Width * 0.5f);		
		FLOAT MaxY = +1.f - ((FLOAT)DestSurfDesc.Height - GPixelCenterOffset) / ((FLOAT)DestSurfDesc.Height * 0.5f);

		// Construct a temporary FTexture to represent the source surface.
		FTexture TempTexture;
		TempTexture.TextureRHI = DestSurface.ResolveTargetTexture2D;
		TempTexture.SamplerStateRHI = TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();

		FBatchedElements BatchedElements;
		INT V00 = BatchedElements.AddVertex(FVector4(MinX,MinY,0,1),FVector2D(0,0),FColor(255,255,255),FHitProxyId());
		INT V10 = BatchedElements.AddVertex(FVector4(MaxX,MinY,0,1),FVector2D(1,0),FColor(255,255,255),FHitProxyId());
		INT V01 = BatchedElements.AddVertex(FVector4(MinX,MaxY,0,1),FVector2D(0,1),FColor(255,255,255),FHitProxyId());
		INT V11 = BatchedElements.AddVertex(FVector4(MaxX,MaxY,0,1),FVector2D(1,1),FColor(255,255,255),FHitProxyId());

		// No alpha blending, no depth tests or writes, no backface culling.
		RHISetBlendState(TStaticBlendState<>::GetRHI());
		RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
		RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());

		// Save current render target so it can be restored
		TRefCountPtr<IDirect3DSurface9> SavedRenderTarget;
		GDirect3DDevice->GetRenderTarget(0, SavedRenderTarget.GetInitReference());
		// save color bias
		INT SavedColorExpBias = GCurrentColorExpBias;
		// use the dest render target color bias for batched element draw
		GCurrentColorExpBias = DestSurface.XeSurfaceInfo.GetColorExpBias();

		// Set to surface that we will copy to
		GDirect3DDevice->SetRenderTarget(0, DestSurface);

		// Draw a quad using the generated vertices.
		BatchedElements.AddTriangle(V00,V10,V11,&TempTexture,BLEND_Opaque);
		BatchedElements.AddTriangle(V00,V11,V01,&TempTexture,BLEND_Opaque);
		BatchedElements.Draw(FMatrix::Identity,DestSurfDesc.Width,DestSurfDesc.Height,FALSE);

		// restore color bias
		GCurrentColorExpBias = SavedColorExpBias;
		// Reset saved target
		GDirect3DDevice->SetRenderTarget(0, SavedRenderTarget);

		// Reset saved viewport
		GDirect3DDevice->SetViewport(&SavedViewport);
	}
}

/**
 * Given a usage name, calculate the offset in tiles into EDRAM the render
 * target or depth stencil surface should use.
 *
 * 10 MByte EDRAM, 5120 (160x32) bytes per tile => 2048 tiles
 *
 * @param	Usage	usage for size request
 * @param	Size	requested EDRAM size
 *
 * @return	EDRAM offset used to create render target at
 */
UINT XeEDRAMOffset( const TCHAR* Usage, UINT Size )
{
	DWORD	EDRamOffset=0xffffffff;

	DWORD DefaultDepthSize	= XGSurfaceSize( GScreenWidth, GScreenHeight, D3DFMT_D24S8, D3DMULTISAMPLE_NONE );
	DWORD DefaultColorSize	= XGSurfaceSize( GScreenWidth, GScreenHeight, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE );
	DWORD ShadowDepthZSize	= XGSurfaceSize( GMaxShadowDepthBufferSize, GMaxShadowDepthBufferSize, D3DFMT_D24X8, D3DMULTISAMPLE_NONE );
	DWORD SmallDepthSize	= XGSurfaceSize( GScreenWidth/2, GScreenHeight/2, D3DFMT_D24S8, D3DMULTISAMPLE_NONE );
	DWORD SmallColorSize	= XGSurfaceSize( GScreenWidth/2, GScreenHeight/2, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE );

	//	Normal EDRAM tile layout:
	//	0				720				1440	1624	2048
	//	+---------------+---------------+--------+------+
	//	|    Zbuffer	|   SceneColor	| SmallZ | Free	|	
	//	+---------------+---------------+--------+------+
	//
	//	SceneColor:		SceneColor, DefaultBB, AuxColor, LightAttenuation, DefaultColorRaw, ShadowVariance
	//	SmallZ + Free:	SmallDepth, ShadowDepthZ
	//	Free:			FilterColor, VelocityBuffer, AmbientOcclusion, AOHistory, Fog buffers

	if( appStricmp(Usage,TEXT("DefaultDepth"))==0 )
	{
		// Place in the beginning of EDRAM.
		EDRamOffset = 0;
	}
	else if( appStricmp(Usage,TEXT("SmallDepth"))==0 )
	{
		// Place the quarter-sized depth buffer after SceneDepth and SceneColor.
		EDRamOffset = DefaultDepthSize + DefaultColorSize;
	}
	else if( appStricmp(Usage,TEXT("ShadowDepthZ"))==0 ||
			 appStricmp(Usage,TEXT("ShadowVariance"))==0 ||
			 appStricmp(Usage,TEXT("ShadowDepthRT"))==0)
	{
		// Co-exist with SceneDepth and SceneColor. Overlap SmallZ + Free.
		EDRamOffset = DefaultDepthSize + DefaultColorSize;
	}
	else if( appStricmp(Usage,TEXT("FogBuffer"))==0 )
	{
		// Co-exist with SceneDepth, SceneColor, SmallDepth and AmbientOcclusion buffers.
		EDRamOffset = DefaultDepthSize + DefaultColorSize + SmallDepthSize + SmallColorSize;
	}
	else if( appStricmp(Usage,TEXT("FilterColor"))==0 ||
			 appStricmp(Usage,TEXT("FogFrontfacesIntegralAccumulationRT"))==0 ||
			 appStricmp(Usage,TEXT("FogBackfacesIntegralAccumulationRT"))==0 ||
			 appStricmp(Usage,TEXT("AmbientOcclusion"))==0 ||
			 appStricmp(Usage,TEXT("AOHistory"))==0 ||
			 appStricmp(Usage,TEXT("VelocityBuffer"))==0 )
	{
		// Make these co-exist with SceneDepth, SceneColor and SmallDepth by placing them in the 4th, "Free" region.
		EDRamOffset = DefaultDepthSize + DefaultColorSize + SmallDepthSize;
	}
	else
	{
		// By default, co-exist with SceneDepth only (overlap SceneColor).
		EDRamOffset = DefaultDepthSize;
	}

	// Sanity check the resultant address
	check( EDRamOffset + Size < GPU_EDRAM_TILES );

	return EDRamOffset;
}

/**
 * Given a usage name, calculate the offset in tiles into HiZ memory
 *
 * Max 3600 usable hierarchical Z tiles.  This is enough for 1280x720x2X:
 *
 * @param	Usage	usage for size request
 * @param	Size	requested HiZ size
 *
 * @return	HiZ offset used to create render target at
 */
UINT XeHiZOffset( const TCHAR* Usage, UINT Size )
{
	DWORD DepthBufferSize = XGHierarchicalZSize( GScreenWidth, GScreenHeight, D3DMULTISAMPLE_NONE );

	DWORD HiZOffset = 0xffffffff;

	if( appStricmp(Usage, TEXT("DefaultDepth")) == 0 )
	{
		HiZOffset = 0;
	}
	else if( appStricmp(Usage, TEXT("ShadowDepthZ")) == 0 ||
			 appStricmp(Usage, TEXT("SmallDepth")) == 0 )
	{
		// Co-exist with SceneDepth
		HiZOffset = DepthBufferSize;
	}
	else if( appStricmp(Usage, TEXT("AuxDepth")) == 0 )
	{
		HiZOffset = 0;
	}

	// Sanity check the resultant address
	check( HiZOffset + Size < GPU_HIERARCHICAL_Z_TILES );

    return HiZOffset;
}

#endif
