/*=============================================================================
	D3D10RenderTarget.cpp: D3D render target implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3D10DrvPrivate.h"
#include "BatchedElements.h"
#include "ScreenRendering.h"

/**
* Copies the contents of the given surface to its resolve target texture.
* @param SourceSurface - surface with a resolve texture to copy to
* @param bKeepOriginalSurface - TRUE if the original surface will still be used after this function so must remain valid
* @param ResolveParams - optional resolve params
*/
void FD3D10DynamicRHI::CopyToResolveTarget(FSurfaceRHIParamRef SourceSurfaceRHI, UBOOL bKeepOriginalSurface, const FResolveParams& ResolveParams)
{
	DYNAMIC_CAST_D3D10RESOURCE(Surface,SourceSurface);

	FD3D10Texture2D* ResolveTargetParameter = (FD3D10Texture2D*)ResolveParams.ResolveTarget;
	TRefCountPtr<ID3D10Texture2D> ResolveTarget = ResolveParams.ResolveTarget ? ResolveTargetParameter->Resource : SourceSurface->ResolveTargetResource;

	if(	SourceSurface->Resource &&
		SourceSurface->Resource != SourceSurface->ResolveTargetResource)
	{
		if(ResolveTarget)
		{
			D3D10_TEXTURE2D_DESC ResolveTargetDesc;
			ResolveTarget->GetDesc(&ResolveTargetDesc);

			// Determine whether the resolve target is a cube-map face.
			UINT Subresource = 0;
			if(ResolveTargetDesc.MiscFlags == D3D10_RESOURCE_MISC_TEXTURECUBE)
			{
				// Resolving a cube texture
				UINT D3DFace = GetD3D10CubeFace(ResolveParams.CubeFace);
				Subresource = D3D10CalcSubresource(0,D3DFace,1);
			}

			// Determine whether a MSAA resolve is needed, or just a copy.
			D3D10_TEXTURE2D_DESC SourceSurfaceDesc;
			SourceSurface->Resource->GetDesc(&SourceSurfaceDesc);
			if(SourceSurfaceDesc.SampleDesc.Count != 1)
			{
				Direct3DDevice->ResolveSubresource(ResolveTarget,Subresource,SourceSurface->Resource,Subresource,ResolveTargetDesc.Format);
			}
			else
			{
				Direct3DDevice->CopySubresourceRegion(ResolveTarget,Subresource,0,0,0,SourceSurface->Resource,Subresource,NULL);
			}
		}
	}
}

void FD3D10DynamicRHI::CopyFromResolveTargetFast(FSurfaceRHIParamRef DestSurface)
{
	// these need to be referenced in order for the FScreenVertexShader/FScreenPixelShader types to not be compiled out on PC
	TShaderMapRef<FScreenVertexShader> ScreenVertexShader(GetGlobalShaderMap());
	TShaderMapRef<FScreenPixelShader> ScreenPixelShader(GetGlobalShaderMap());
}

void FD3D10DynamicRHI::CopyFromResolveTargetRectFast(FSurfaceRHIParamRef DestSurface,FLOAT X1,FLOAT Y1,FLOAT X2,FLOAT Y2)
{
	// these need to be referenced in order for the FScreenVertexShader/FScreenPixelShader types to not be compiled out on PC
	TShaderMapRef<FScreenVertexShader> ScreenVertexShader(GetGlobalShaderMap());
	TShaderMapRef<FScreenPixelShader> ScreenPixelShader(GetGlobalShaderMap());
}

void FD3D10DynamicRHI::CopyFromResolveTarget(FSurfaceRHIParamRef DestSurface)
{
	// these need to be referenced in order for the FScreenVertexShader/FScreenPixelShader types to not be compiled out on PC
	TShaderMapRef<FScreenVertexShader> ScreenVertexShader(GetGlobalShaderMap());
	TShaderMapRef<FScreenPixelShader> ScreenPixelShader(GetGlobalShaderMap());
}

/**
* Creates a RHI surface that can be bound as a render target.
* Note that a surface cannot be created which is both resolvable AND readable.
* @param SizeX - The width of the surface to create.
* @param SizeY - The height of the surface to create.
* @param Format - The surface format to create.
* @param ResolveTargetTexture - The 2d texture which the surface will be resolved to.  It must have been allocated with bResolveTargetable=TRUE
* @param Flags - Surface creation flags
* @param UsageStr - Text describing usage for this surface
* @return The surface that was created.
*/
FSurfaceRHIRef FD3D10DynamicRHI::CreateTargetableSurface(
	UINT SizeX,
	UINT SizeY,
	BYTE Format,
	FTexture2DRHIParamRef ResolveTargetTextureRHI,
	DWORD Flags,
	const TCHAR* UsageStr
	)
{
	DYNAMIC_CAST_D3D10RESOURCE(Texture2D,ResolveTargetTexture);

	const UBOOL bDepthFormat = (Format == PF_DepthStencil || Format == PF_ShadowDepth|| Format == PF_FilteredShadowDepth || Format == PF_D24);
	const DXGI_FORMAT PlatformFormat = (DXGI_FORMAT)GPixelFormats[Format].PlatformFormat;

	D3D10_DSV_DIMENSION DepthStencilViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	D3D10_RTV_DIMENSION RenderTargetViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
	D3D10_SRV_DIMENSION ShaderResourceViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;

	// Determine whether to use MSAA for this surface.
	const UINT MaxMultiSamples = GSystemSettings.RenderThreadSettings.MaxMultiSamples;
	UINT MultiSampleCount = 1;
	UINT MultiSampleQuality = 0;
	if(MaxMultiSamples > 0 && (Flags&TargetSurfCreate_Multisample) && bMSAAIsSupported)
	{
		UBOOL bUseMultiSample = FALSE;

		// Find an appropriate multi-sample format for the surface.
		for(UINT Index = 1;Index <= MaxMultiSamples && Index <= D3D10_MAX_MULTISAMPLE_SAMPLE_COUNT;++Index)
		{
			UINT NumMultiSampleQualities = 0;
			if(	SUCCEEDED(Direct3DDevice->CheckMultisampleQualityLevels(PlatformFormat,Index,&NumMultiSampleQualities)) &&
				NumMultiSampleQualities > 0)
			{
				bUseMultiSample = TRUE;
				MultiSampleCount = Index;
			}
		}

		if(bUseMultiSample)
		{
			DepthStencilViewDimension = D3D10_DSV_DIMENSION_TEXTURE2DMS;
			RenderTargetViewDimension = D3D10_RTV_DIMENSION_TEXTURE2DMS;
			ShaderResourceViewDimension = D3D10_SRV_DIMENSION_TEXTURE2DMS;

			// MSAA surfaces can't be shared with a texture.
			Flags |= TargetSurfCreate_Dedicated;
		}
	}

	// Determine the surface's resource.
	TRefCountPtr<ID3D10Texture2D> SurfaceResource;
	if(ResolveTargetTexture && !(Flags & TargetSurfCreate_Dedicated))
	{
		// For non-dedicated views, use the resolve target resource directly.
		SurfaceResource = ResolveTargetTexture->Resource;
	}
	else
	{
		// For dedicated views, or views without a resolve target, create a new resource for the surface.
		D3D10_TEXTURE2D_DESC Desc;
		Desc.Width = SizeX;
		Desc.Height = SizeY;
		Desc.MipLevels = 1;
		Desc.ArraySize = 1;
		Desc.Format = PlatformFormat;
		Desc.SampleDesc.Count = MultiSampleCount;
		Desc.SampleDesc.Quality = MultiSampleQuality;
		Desc.Usage = D3D10_USAGE_DEFAULT;
		Desc.BindFlags = bDepthFormat ? D3D10_BIND_DEPTH_STENCIL : D3D10_BIND_RENDER_TARGET;
		if(!bDepthFormat && !ResolveTargetTexture)
			Desc.BindFlags |= D3D10_BIND_SHADER_RESOURCE;
		Desc.CPUAccessFlags = 0;
		Desc.MiscFlags = 0;
		VERIFYD3D10RESULT(Direct3DDevice->CreateTexture2D(&Desc,NULL,SurfaceResource.GetInitReference()));
	}

	// Create either a render target view or depth stencil view for the surface.
	TRefCountPtr<ID3D10RenderTargetView> RenderTargetView;
	TRefCountPtr<ID3D10DepthStencilView> DepthStencilView;
	if(bDepthFormat)
	{
		D3D10_DEPTH_STENCIL_VIEW_DESC DSVDesc;
		DSVDesc.Format = PlatformFormat;
		DSVDesc.ViewDimension = DepthStencilViewDimension;
		DSVDesc.Texture2D.MipSlice = 0;
		VERIFYD3D10RESULT(Direct3DDevice->CreateDepthStencilView(SurfaceResource,&DSVDesc,DepthStencilView.GetInitReference()));
	}
	else
	{
		D3D10_RENDER_TARGET_VIEW_DESC RTVDesc;
		RTVDesc.Format = PlatformFormat;
		RTVDesc.ViewDimension = RenderTargetViewDimension;
		RTVDesc.Texture2D.MipSlice = 0;
		VERIFYD3D10RESULT(Direct3DDevice->CreateRenderTargetView(SurfaceResource,&RTVDesc,RenderTargetView.GetInitReference()));
	}

	if(ResolveTargetTexture)
	{
		return new FD3D10Surface(RenderTargetView,DepthStencilView,ResolveTargetTexture->Resource,SurfaceResource);
	}
	else
	{
		return new FD3D10Surface(RenderTargetView,DepthStencilView,NULL,SurfaceResource);
	}
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
FSurfaceRHIRef FD3D10DynamicRHI::CreateTargetableCubeSurface(
	UINT SizeX,
	BYTE Format,
	FTextureCubeRHIParamRef ResolveTargetTextureRHI,
	ECubeFace CubeFace,
	DWORD Flags,
	const TCHAR* UsageStr
	)
{
	DYNAMIC_CAST_D3D10RESOURCE(TextureCube,ResolveTargetTexture);

	check(Format != PF_DepthStencil);
	if(!ResolveTargetTexture)
	{
		checkMsg(FALSE,TEXT("No resolve target cube texture specified.  Just use RHICreateTargetableSurface instead."));
		return NULL;
	}
	else
	{
		const DXGI_FORMAT PlatformFormat = (DXGI_FORMAT)GPixelFormats[Format].PlatformFormat;
		const UINT D3DFace = GetD3D10CubeFace(CubeFace);

		// Determine the surface's resource.
		TRefCountPtr<ID3D10Texture2D> SurfaceResource;
		D3D10_RTV_DIMENSION DimensionRTV;
		D3D10_DSV_DIMENSION DimensionDSV;
		if(ResolveTargetTexture && !(Flags & TargetSurfCreate_Dedicated))
		{
			// For non-dedicated views, use the resolve target resource directly.
			SurfaceResource = ResolveTargetTexture->Resource;
			DimensionRTV = D3D10_RTV_DIMENSION_TEXTURE2DARRAY;
			DimensionDSV = D3D10_DSV_DIMENSION_TEXTURE2DARRAY;
		}
		else
		{
			// For dedicated views, or views without a resolve target, create a new resource for the surface.
			D3D10_TEXTURE2D_DESC Desc;
			Desc.Width = SizeX;
			Desc.Height = SizeX;
			Desc.MipLevels = 1;
			Desc.ArraySize = 1;
			Desc.Format = PlatformFormat;
			Desc.SampleDesc.Count = 1;
			Desc.SampleDesc.Quality = 0;
			Desc.Usage = D3D10_USAGE_DEFAULT;
			Desc.BindFlags = (Format == PF_ShadowDepth) ? D3D10_BIND_DEPTH_STENCIL : D3D10_BIND_RENDER_TARGET;
			Desc.CPUAccessFlags = 0;
			Desc.MiscFlags = 0;
			VERIFYD3D10RESULT(Direct3DDevice->CreateTexture2D(&Desc,NULL,SurfaceResource.GetInitReference()));
			DimensionRTV = D3D10_RTV_DIMENSION_TEXTURE2D;
			DimensionDSV = D3D10_DSV_DIMENSION_TEXTURE2D;
		}

		// Create either a render target view or depth stencil view for the surface.
		TRefCountPtr<ID3D10RenderTargetView> RenderTargetView;
		TRefCountPtr<ID3D10DepthStencilView> DepthStencilView;
		if(Format == PF_ShadowDepth)
		{
			D3D10_DEPTH_STENCIL_VIEW_DESC DSVDesc;
			DSVDesc.Format = PlatformFormat;
			DSVDesc.ViewDimension = DimensionDSV;
			DSVDesc.Texture2D.MipSlice = 0;
			DSVDesc.Texture2DArray.ArraySize = 1;
			DSVDesc.Texture2DArray.FirstArraySlice = D3DFace;
			DSVDesc.Texture2DArray.MipSlice = 0;
			VERIFYD3D10RESULT(Direct3DDevice->CreateDepthStencilView(SurfaceResource,&DSVDesc,DepthStencilView.GetInitReference()));
		}
		else
		{
			D3D10_RENDER_TARGET_VIEW_DESC RTVDesc;
			RTVDesc.Format = PlatformFormat;
			RTVDesc.ViewDimension = DimensionRTV;
			RTVDesc.Texture2D.MipSlice = 0;
			RTVDesc.Texture2DArray.ArraySize = 1;
			RTVDesc.Texture2DArray.FirstArraySlice = D3DFace;
			RTVDesc.Texture2DArray.MipSlice = 0;
			VERIFYD3D10RESULT(Direct3DDevice->CreateRenderTargetView(SurfaceResource,&RTVDesc,RenderTargetView.GetInitReference()));
		}

		return new FD3D10Surface(RenderTargetView,DepthStencilView,ResolveTargetTexture->Resource,SurfaceResource);
	}
}

void FD3D10DynamicRHI::ReadSurfaceData(FSurfaceRHIParamRef SurfaceRHI,UINT MinX,UINT MinY,UINT MaxX,UINT MaxY,TArray<BYTE>& OutData,ECubeFace CubeFace)
{
	DYNAMIC_CAST_D3D10RESOURCE(Surface,Surface);

	UINT SizeX = MaxX - MinX + 1;
	UINT SizeY = MaxY - MinY + 1;
	
	// Select which resource to copy from depending on whether there is a resolve target
	ID3D10Texture2D* SourceSurface = Surface->Resource;
	if( Surface->ResolveTargetResource )
	{
		SourceSurface = Surface->ResolveTargetResource;
	}
	check(SourceSurface);

	// Check the format of the surface
	D3D10_TEXTURE2D_DESC SurfaceDesc;
	SourceSurface->GetDesc(&SurfaceDesc);

	check(SurfaceDesc.Format == GPixelFormats[PF_A8R8G8B8].PlatformFormat);

	// Allocate the output buffer.
	OutData.Empty((MaxX - MinX + 1) * (MaxY - MinY + 1) * sizeof(FColor));

	// Read back the surface data from (MinX,MinY) to (MaxX,MaxY)
	D3D10_BOX	Rect;
	Rect.left	= MinX;
	Rect.top	= MinY;
	Rect.right	= MaxX + 1;
	Rect.bottom	= MaxY + 1;
	Rect.back = 1;
	Rect.front = 0;

	// create a temp 2d texture to copy render target to
	TRefCountPtr<ID3D10Texture2D> Texture2D;
	D3D10_TEXTURE2D_DESC Desc;
	Desc.Width = SizeX;
	Desc.Height = SizeY;
	Desc.MipLevels = 1;
	Desc.ArraySize = 1;
	Desc.Format = SurfaceDesc.Format;
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.Usage = D3D10_USAGE_STAGING;
	Desc.BindFlags = 0;
	Desc.CPUAccessFlags = D3D10_CPU_ACCESS_READ;
	Desc.MiscFlags = 0;
	VERIFYD3D10RESULT(Direct3DDevice->CreateTexture2D(&Desc,NULL,Texture2D.GetInitReference()));

	// Copy the data to a staging resource.
	UINT Subresource = 0;
	if( SurfaceDesc.MiscFlags == D3D10_RESOURCE_MISC_TEXTURECUBE )
	{
		UINT D3DFace = GetD3D10CubeFace(CubeFace);
		Subresource = D3D10CalcSubresource(0,D3DFace,1);
	}
	Direct3DDevice->CopySubresourceRegion(Texture2D,0,0,0,0,SourceSurface,Subresource,&Rect);

	// Lock the staging resource.
	D3D10_MAPPED_TEXTURE2D LockedRect;
	VERIFYD3D10RESULT(Texture2D->Map(0,D3D10_MAP_READ,0,&LockedRect));

	// Read the data out of the buffer, converting it from ABGR to ARGB.
	for(UINT Y = MinY;Y <= MaxY;Y++)
	{
		FColor* SrcPtr = (FColor*)((BYTE*)LockedRect.pData + (Y - MinY) * LockedRect.RowPitch);
		FColor* DestPtr = (FColor*)((BYTE*)&OutData(OutData.Add(SizeX * sizeof(FColor))));
		for(UINT X = MinX;X <= MaxX;X++)
		{
			*DestPtr = FColor(SrcPtr->B,SrcPtr->G,SrcPtr->R,SrcPtr->A);
			++SrcPtr;
			++DestPtr;
		}
	}

	Texture2D->Unmap(0);
}
