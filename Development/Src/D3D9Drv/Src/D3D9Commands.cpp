/*=============================================================================
	D3D9Commands.cpp: D3D RHI commands implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3D9DrvPrivate.h"

#include "EngineParticleClasses.h"
#include "ChartCreation.h"

// Globals

/**
 *	Used for checking if the device is lost or not.
 *	Using the global variable is not enough, since it's updated to rarely.
 */
extern UBOOL GParanoidDeviceLostChecking /*= TRUE*/;
#define IsDeviceLost()	(GParanoidDeviceLostChecking && Direct3DDevice->TestCooperativeLevel() != D3D_OK)

// Vertex state.
void FD3D9DynamicRHI::SetStreamSource(UINT StreamIndex,FVertexBufferRHIParamRef VertexBufferRHI,UINT Stride,UBOOL bUseInstanceIndex,UINT NumVerticesPerInstance,UINT NumInstances)
{
	check(StreamIndex < NumVertexStreams);

	DYNAMIC_CAST_D3D9RESOURCE(VertexBuffer,VertexBuffer);

	Direct3DDevice->SetStreamSource(StreamIndex,VertexBuffer,0,Stride);

	UBOOL bUseStride = TRUE;
	DWORD Frequency = 1;
	// Set the vertex stream frequency.
	if(GSupportsVertexInstancing)
	{
		PendingNumInstances = 1;
		if (bUseInstanceIndex || NumInstances > 1)
		{
			Frequency = bUseInstanceIndex ? 
				(D3DSTREAMSOURCE_INSTANCEDATA | 1) :
			(D3DSTREAMSOURCE_INDEXEDDATA | NumInstances);
		}
	}
	else
	{
		PendingNumInstances = NumInstances;
		if (NumInstances > 1 && bUseInstanceIndex)
		{
			PendingStreams[StreamIndex].VertexBuffer = VertexBuffer;
			PendingStreams[StreamIndex].Stride = Stride;
			PendingStreams[StreamIndex].NumVerticesPerInstance = NumVerticesPerInstance;
			bUseStride = FALSE; // We don't want this to advance per instance, but rather be advanced manually for fake instancing
			UpdateStreamForInstancingMask |= 1<<StreamIndex;
		}
	}
	Direct3DDevice->SetStreamSource(StreamIndex,VertexBuffer,0,bUseStride ? Stride : 0);
	Direct3DDevice->SetStreamSourceFreq(StreamIndex,Frequency);
}

// Rasterizer state.
void FD3D9DynamicRHI::SetRasterizerState(FRasterizerStateRHIParamRef NewStateRHI)
{
	DYNAMIC_CAST_D3D9RESOURCE(RasterizerState,NewState);

	Direct3DDevice->SetRenderState(D3DRS_FILLMODE,NewState->FillMode);
	Direct3DDevice->SetRenderState(D3DRS_CULLMODE,NewState->CullMode);
	// Add the global depth bias
	extern FLOAT GDepthBiasOffset;
	Direct3DDevice->SetRenderState(D3DRS_DEPTHBIAS,FLOAT_TO_DWORD(NewState->DepthBias + GDepthBiasOffset));
	Direct3DDevice->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS,FLOAT_TO_DWORD(NewState->SlopeScaleDepthBias));
}
void FD3D9DynamicRHI::SetViewport(UINT MinX,UINT MinY,FLOAT MinZ,UINT MaxX,UINT MaxY,FLOAT MaxZ)
{
	D3DVIEWPORT9 Viewport = { MinX, MinY, MaxX - MinX, MaxY - MinY, MinZ, MaxZ };
	//avoid setting a 0 extent viewport, which the debug runtime doesn't like
	if (Viewport.Width > 0 && Viewport.Height > 0)
	{
		Direct3DDevice->SetViewport(&Viewport);
	}
}

void FD3D9DynamicRHI::SetScissorRect(UBOOL bEnable,UINT MinX,UINT MinY,UINT MaxX,UINT MaxY)
{
	// Defined in UnPlayer.cpp. Used here to disable scissors when doing highres screenshots.
	extern UBOOL GIsTiledScreenshot;
	bEnable = GIsTiledScreenshot ? FALSE : bEnable;

	if(bEnable)
	{
		RECT ScissorRect;
		ScissorRect.left = MinX;
		ScissorRect.right = MaxX;
		ScissorRect.top = MinY;
		ScissorRect.bottom = MaxY;
		Direct3DDevice->SetScissorRect(&ScissorRect);
	}
	Direct3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE,bEnable);
}

/**
 * Set depth bounds test state.  
 * When enabled, incoming fragments are killed if the value already in the depth buffer is outside [ClipSpaceNearPos, ClipSpaceFarPos]
 *
 * @param bEnable - whether to enable or disable the depth bounds test
 * @param ClipSpaceNearPos - near point in clip space
 * @param ClipSpaceFarPos - far point in clip space
 */
void FD3D9DynamicRHI::SetDepthBoundsTest(UBOOL bEnable, const FVector4 &ClipSpaceNearPos, const FVector4 &ClipSpaceFarPos)
{
	if (bEnable)
	{
		if (bDepthBoundsHackSupported)
		{
			// convert to normalized device coordinates, which are the units used by Nvidia's D3D depth bounds test driver hack.
			// clamp to valid ranges
			FLOAT MinZ = Clamp(Max(ClipSpaceNearPos.Z, 0.0f) / ClipSpaceNearPos.W, 0.0f, 1.0f);
			FLOAT MaxZ = Clamp(ClipSpaceFarPos.Z / ClipSpaceFarPos.W, 0.0f, 1.0f);

			// enable the depth bounds test
			Direct3DDevice->SetRenderState(D3DRS_ADAPTIVETESS_X,MAKEFOURCC('N','V','D','B'));

			// only set depth bounds if ranges are valid
			if (MinZ <= MaxZ)
			{
				// set the overridden render states which define near and far depth bounds in normalized device coordinates
				// Note: Depth bounds test operates on the value already in the depth buffer, not the incoming fragment!
				Direct3DDevice->SetRenderState(D3DRS_ADAPTIVETESS_Z, FLOAT_TO_DWORD(MinZ));
				Direct3DDevice->SetRenderState(D3DRS_ADAPTIVETESS_W, FLOAT_TO_DWORD(MaxZ));
			}
		}
		else
		{
			// construct a near plane in clip space that will reject any pixels whose z is closer than it
			FPlane NearPlane = FPlane(0.0f, 0.0f, 1.0f, -Max(ClipSpaceNearPos.Z, 0.0f) / ClipSpaceNearPos.W);
			// construct a far plane in clip space that will reject any pixels whose z is further than it
			FPlane FarPlane = FPlane(0.0f, 0.0f, -1.0f, Max(ClipSpaceFarPos.Z, 0.0f) / ClipSpaceFarPos.W);

			// turn on the first two planes which are specified through bit 0 and 1
			// @todo: if user clip planes are used for anything else in D3D, use an allocation scheme instead of just always using planes 0 and 1
			// Note: Using clip planes is different from depth bounds test since it operates on the position of the incoming pixel, 
			// so the incoming pixel and the corresponding depth buffer's depths must be the same for dependable results.
			Direct3DDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 0x3);
			Direct3DDevice->SetClipPlane(0, (FLOAT*)&NearPlane);
			Direct3DDevice->SetClipPlane(1, (FLOAT*)&FarPlane);

			// on cards that implement clip planes by splitting triangles there will be significant z-fighting, so a depth bias is necessary
			extern FLOAT GDepthBiasOffset;
			GDepthBiasOffset = -0.00001;
		}
	}
	else
	{
		if (bDepthBoundsHackSupported)
{
			// disable depth bounds test
			Direct3DDevice->SetRenderState(D3DRS_ADAPTIVETESS_X,0);
		}
		else
		{
			// turn all of the clip planes off
			Direct3DDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 0x0);

			// restore the depth bias to 0
			extern FLOAT GDepthBiasOffset;
			GDepthBiasOffset = 0.0f;
		}
	}
}

/**
* Set bound shader state. This will set the vertex decl/shader, and pixel shader
* @param BoundShaderState - state resource
*/
void FD3D9DynamicRHI::SetBoundShaderState(FBoundShaderStateRHIParamRef BoundShaderStateRHI)
{
	DYNAMIC_CAST_D3D9RESOURCE(BoundShaderState,BoundShaderState);

	// Clear the vertex streams that were used by the old bound shader state.
	ResetVertexStreams();

	check(BoundShaderState->VertexDeclaration);
	Direct3DDevice->SetVertexDeclaration(BoundShaderState->VertexDeclaration);
	Direct3DDevice->SetVertexShader(BoundShaderState->VertexShader);
	if ( BoundShaderState->PixelShader )
	{
		Direct3DDevice->SetPixelShader(BoundShaderState->PixelShader);
	}
	else
	{
		// use special null pixel shader when PixelSahder was set to NULL
		FPixelShaderRHIParamRef NullPixelShaderRHI = TShaderMapRef<FNULLPixelShader>(GetGlobalShaderMap())->GetPixelShader();
		DYNAMIC_CAST_D3D9RESOURCE(PixelShader,NullPixelShader);
		Direct3DDevice->SetPixelShader(NullPixelShader);
	}

	// Prevent transient bound shader states from being recreated for each use by keeping a history of the most recently used bound shader states.
	// The history keeps them alive, and the bound shader state cache allows them to be reused if needed.
	BoundShaderStateHistory.Add(BoundShaderState);
}

void FD3D9DynamicRHI::SetSamplerState(FPixelShaderRHIParamRef PixelShaderRHI,UINT TextureIndex,UINT SamplerIndex,FSamplerStateRHIParamRef NewStateRHI,FTextureRHIParamRef NewTextureRHI,FLOAT MipBias)
{
	DYNAMIC_CAST_D3D9RESOURCE(PixelShader,PixelShader);
	DYNAMIC_CAST_D3D9RESOURCE(SamplerState,NewState);
	DYNAMIC_CAST_D3D9RESOURCE(Texture,NewTexture);

	// Force linear mip-filter if MipBias has a fractional part.
	DWORD MipFilter;
	if (NewState->MipMapLODBias || appIsNearlyEqual(appTruncFloat(MipBias), MipBias))
	{
		MipFilter = NewState->MipFilter;
	}
	else
	{
		MipFilter = D3DTEXF_LINEAR;
	}

	Direct3DDevice->SetTexture(TextureIndex,*NewTexture);
	Direct3DDevice->SetSamplerState(TextureIndex,D3DSAMP_SRGBTEXTURE,NewTexture->IsSRGB());
	Direct3DDevice->SetSamplerState(TextureIndex,D3DSAMP_MAGFILTER,NewState->MagFilter);
	Direct3DDevice->SetSamplerState(TextureIndex,D3DSAMP_MINFILTER,NewState->MinFilter);
	Direct3DDevice->SetSamplerState(TextureIndex,D3DSAMP_MIPFILTER,NewState->MipFilter);
	Direct3DDevice->SetSamplerState(TextureIndex,D3DSAMP_ADDRESSU,NewState->AddressU);
	Direct3DDevice->SetSamplerState(TextureIndex,D3DSAMP_ADDRESSV,NewState->AddressV);
	Direct3DDevice->SetSamplerState(TextureIndex,D3DSAMP_ADDRESSW,NewState->AddressW);
	Direct3DDevice->SetSamplerState(TextureIndex,D3DSAMP_MIPMAPLODBIAS,NewState->MipMapLODBias ? NewState->MipMapLODBias : FLOAT_TO_DWORD(MipBias) );
}

void FD3D9DynamicRHI::SetVertexShaderParameter(FVertexShaderRHIParamRef VertexShaderRHI,UINT BufferIndex,UINT BaseIndex,UINT NumBytes,const void* NewValue)
{
	Direct3DDevice->SetVertexShaderConstantF(
		BaseIndex / NumBytesPerShaderRegister,
		(FLOAT*)GetPaddedShaderParameterValue(NewValue,NumBytes),
		(NumBytes + NumBytesPerShaderRegister - 1) / NumBytesPerShaderRegister
		);
}

void FD3D9DynamicRHI::SetPixelShaderParameter(FPixelShaderRHIParamRef PixelShader,UINT BufferIndex,UINT BaseIndex,UINT NumBytes,const void* NewValue)
{
	Direct3DDevice->SetPixelShaderConstantF(
		BaseIndex / NumBytesPerShaderRegister,
		(FLOAT*)GetPaddedShaderParameterValue(NewValue,NumBytes),
		(NumBytes + NumBytesPerShaderRegister - 1) / NumBytesPerShaderRegister
		);
}

void FD3D9DynamicRHI::SetPixelShaderBoolParameter(FPixelShaderRHIParamRef PixelShader,UINT BaseIndex,UBOOL NewValue)
{
	//@todo - implement
}

/**
 * Set engine shader parameters for the view.
 * @param View					The current view
 * @param ViewProjectionMatrix	Matrix that transforms from world space to projection space for the view
 * @param ViewOrigin			World space position of the view's origin
 */
void FD3D9DynamicRHI::SetViewParameters( const FSceneView* View, const FMatrix& ViewProjectionMatrix, const FVector4& ViewOrigin )
{
	const FVector4 TranslatedViewOrigin = ViewOrigin + FVector4(View->PreViewTranslation,0);

	Direct3DDevice->SetVertexShaderConstantF( VSR_ViewProjMatrix, (const FLOAT*) &ViewProjectionMatrix, 4 );
	Direct3DDevice->SetVertexShaderConstantF( VSR_ViewOrigin, (const FLOAT*) &TranslatedViewOrigin, 1 );
	Direct3DDevice->SetPixelShaderConstantF( PSR_MinZ_MaxZ_Ratio, (const FLOAT*) &View->InvDeviceZToWorldZTransform, 1 );
	Direct3DDevice->SetPixelShaderConstantF( PSR_ScreenPositionScaleBias, (const FLOAT*) &View->ScreenPositionScaleBias, 1 );
}

/**
 * Not used on PC
 */
void FD3D9DynamicRHI::SetViewPixelParameters(const FSceneView* View,FPixelShaderRHIParamRef PixelShader,const class FShaderParameter* SceneDepthCalcParameter,const class FShaderParameter* ScreenPositionScaleBiasParameter)
{
}
void FD3D9DynamicRHI::SetRenderTargetBias( FLOAT ColorBias )
{
}
void FD3D9DynamicRHI::SetShaderRegisterAllocation(UINT NumVertexShaderRegisters, UINT NumPixelShaderRegisters)
{
}
void FD3D9DynamicRHI::ReduceTextureCachePenalty( FPixelShaderRHIParamRef PixelShader )
{
}

// Output state.
void FD3D9DynamicRHI::SetDepthState(FDepthStateRHIParamRef NewStateRHI)
{
	DYNAMIC_CAST_D3D9RESOURCE(DepthState,NewState);

	Direct3DDevice->SetRenderState(D3DRS_ZENABLE,NewState->bZEnable);
	Direct3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,NewState->bZWriteEnable);
	Direct3DDevice->SetRenderState(D3DRS_ZFUNC,NewState->ZFunc);
}
void FD3D9DynamicRHI::SetStencilState(FStencilStateRHIParamRef NewStateRHI)
{
	DYNAMIC_CAST_D3D9RESOURCE(StencilState,NewState);

	Direct3DDevice->SetRenderState(D3DRS_STENCILENABLE,NewState->bStencilEnable);
	Direct3DDevice->SetRenderState(D3DRS_STENCILFUNC,NewState->StencilFunc);
	Direct3DDevice->SetRenderState(D3DRS_STENCILFAIL,NewState->StencilFail);
	Direct3DDevice->SetRenderState(D3DRS_STENCILZFAIL,NewState->StencilZFail);
	Direct3DDevice->SetRenderState(D3DRS_STENCILPASS,NewState->StencilPass);
	Direct3DDevice->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE,NewState->bTwoSidedStencilMode);
	Direct3DDevice->SetRenderState(D3DRS_CCW_STENCILFUNC,NewState->CCWStencilFunc);
	Direct3DDevice->SetRenderState(D3DRS_CCW_STENCILFAIL,NewState->CCWStencilFail);
	Direct3DDevice->SetRenderState(D3DRS_CCW_STENCILZFAIL,NewState->CCWStencilZFail);
	Direct3DDevice->SetRenderState(D3DRS_CCW_STENCILPASS,NewState->CCWStencilPass);
	Direct3DDevice->SetRenderState(D3DRS_STENCILMASK,NewState->StencilReadMask);
	Direct3DDevice->SetRenderState(D3DRS_STENCILWRITEMASK,NewState->StencilWriteMask);
	Direct3DDevice->SetRenderState(D3DRS_STENCILREF,NewState->StencilRef);
}

void FD3D9DynamicRHI::SetBlendState(FBlendStateRHIParamRef NewStateRHI)
{
	DYNAMIC_CAST_D3D9RESOURCE(BlendState,NewState);

#if 0
	extern UBOOL GCurrentlyRenderingScene;
	if( CurrentRenderTargetFormat == D3DFMT_A16B16G16R16F && !CanBlendWithFPRenderTarget(GRHIShaderPlatform) && GCurrentlyRenderingScene )
	{
		Direct3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
		Direct3DDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE,FALSE);
	}
	else
#else
	{
		Direct3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,NewState->bAlphaBlendEnable);
		Direct3DDevice->SetRenderState(D3DRS_BLENDOP,NewState->ColorBlendOperation);
		Direct3DDevice->SetRenderState(D3DRS_SRCBLEND,NewState->ColorSourceBlendFactor);
		Direct3DDevice->SetRenderState(D3DRS_DESTBLEND,NewState->ColorDestBlendFactor);
		Direct3DDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE,NewState->bSeparateAlphaBlendEnable);
		Direct3DDevice->SetRenderState(D3DRS_BLENDOPALPHA,NewState->AlphaBlendOperation);
		Direct3DDevice->SetRenderState(D3DRS_SRCBLENDALPHA,NewState->AlphaSourceBlendFactor);
		Direct3DDevice->SetRenderState(D3DRS_DESTBLENDALPHA,NewState->AlphaDestBlendFactor);
	}
#endif
	
	Direct3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE,NewState->bAlphaTestEnable);
	Direct3DDevice->SetRenderState(D3DRS_ALPHAFUNC,NewState->AlphaFunc);
	Direct3DDevice->SetRenderState(D3DRS_ALPHAREF,NewState->AlphaRef);
}

void FD3D9DynamicRHI::SetRenderTarget(FSurfaceRHIParamRef NewRenderTargetRHI, FSurfaceRHIParamRef NewDepthStencilTargetRHI)
{
	DYNAMIC_CAST_D3D9RESOURCE(Surface,NewRenderTarget);
	DYNAMIC_CAST_D3D9RESOURCE(Surface,NewDepthStencilTarget);

	// Reset all texture references, to ensure a reference to this render target doesn't remain set.
	for(UINT TextureIndex = 0;TextureIndex < 16;TextureIndex++)
	{
		Direct3DDevice->SetTexture(TextureIndex,NULL);
	}

	if(!NewRenderTarget)
	{
		// 1. If we're setting a NULL color buffer, we must also set a NULL depth buffer.
		// 2. If we're setting a NULL color buffer, we're going to use the back buffer instead (D3D shortcoming).
		check( BackBuffer );
		Direct3DDevice->SetRenderTarget(0,*BackBuffer);
	}
	else
	{
		Direct3DDevice->SetRenderTarget(0,*NewRenderTarget);
	}

	Direct3DDevice->SetDepthStencilSurface((NewDepthStencilTarget ? (IDirect3DSurface9*)*NewDepthStencilTarget : NULL));

	// Detect when the back buffer is being set, and set the correct viewport.
	if( DrawingViewport && (!NewRenderTarget || NewRenderTarget == BackBuffer) )
	{
		D3DVIEWPORT9 D3DViewport = { 0, 0, DrawingViewport->GetSizeX(), DrawingViewport->GetSizeY(), 0.0f, 1.0f };
		Direct3DDevice->SetViewport(&D3DViewport);
	}

#if 0
	if( IsValidRef(NewRenderTarget) )
	{
		D3DSURFACE_DESC SurfaceDesc;
		NewRenderTarget->GetDesc(&SurfaceDesc);
		GD3DCurrentRenderTargetFormat = SurfaceDesc.Format;

		extern UBOOL GCurrentlyRenderingScene;
		if( GD3DCurrentRenderTargetFormat == D3DFMT_A16B16G16R16F && GCurrentlyRenderingScene &&!CanBlendWithFPRenderTarget(GRHIShaderPlatform) )
		{
			GDirect3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
			GDirect3DDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE,FALSE);
		}
	}
	else
	{
		GD3DCurrentRenderTargetFormat = D3DFMT_A8R8G8B8;
	}
#endif
}
void FD3D9DynamicRHI::SetMRTRenderTarget(FSurfaceRHIParamRef NewRenderTargetRHI, UINT TargetIndex)
{
	DYNAMIC_CAST_D3D9RESOURCE(Surface,NewRenderTarget);

	// Reset all texture references, to ensure a reference to this render target doesn't remain set.
	for(UINT TextureIndex = 0;TextureIndex < 16;TextureIndex++)
	{
		Direct3DDevice->SetTexture(TextureIndex,NULL);
	}

	Direct3DDevice->SetRenderTarget(TargetIndex,*NewRenderTarget);
}
void FD3D9DynamicRHI::SetColorWriteEnable(UBOOL bEnable)
{
	DWORD EnabledStateValue = D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED;
	Direct3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE,bEnable ? EnabledStateValue : 0);
}
void FD3D9DynamicRHI::SetColorWriteMask(UINT ColorWriteMask)
{
	DWORD EnabledStateValue;
	EnabledStateValue  = (ColorWriteMask & CW_RED) ? D3DCOLORWRITEENABLE_RED : 0;
	EnabledStateValue |= (ColorWriteMask & CW_GREEN) ? D3DCOLORWRITEENABLE_GREEN : 0;
	EnabledStateValue |= (ColorWriteMask & CW_BLUE) ? D3DCOLORWRITEENABLE_BLUE : 0;
	EnabledStateValue |= (ColorWriteMask & CW_ALPHA) ? D3DCOLORWRITEENABLE_ALPHA : 0;
	Direct3DDevice->SetRenderState( D3DRS_COLORWRITEENABLE, EnabledStateValue );
}

// Not supported
void FD3D9DynamicRHI::BeginHiStencilRecord(UBOOL bCompareFunctionEqual, UINT RefValue) { }
void FD3D9DynamicRHI::BeginHiStencilPlayback(UBOOL bFlush) { }
void FD3D9DynamicRHI::EndHiStencil() { }

// Occlusion queries.
void FD3D9DynamicRHI::BeginOcclusionQuery(FOcclusionQueryRHIParamRef OcclusionQueryRHI)
{
	DYNAMIC_CAST_D3D9RESOURCE(OcclusionQuery,OcclusionQuery);
	OcclusionQuery->Resource->Issue(D3DISSUE_BEGIN);
}
void FD3D9DynamicRHI::EndOcclusionQuery(FOcclusionQueryRHIParamRef OcclusionQueryRHI)
{
	DYNAMIC_CAST_D3D9RESOURCE(OcclusionQuery,OcclusionQuery);
	OcclusionQuery->Resource->Issue(D3DISSUE_END);
}

// Primitive drawing.

static D3DPRIMITIVETYPE GetD3D9PrimitiveType(UINT PrimitiveType)
{
	switch(PrimitiveType)
	{
	case PT_TriangleList: return D3DPT_TRIANGLELIST;
	case PT_TriangleStrip: return D3DPT_TRIANGLESTRIP;
	case PT_LineList: return D3DPT_LINELIST;
	default: appErrorf(TEXT("Unknown primitive type: %u"),PrimitiveType);
	};
	return D3DPT_TRIANGLELIST;
}

void FD3D9DynamicRHI::DrawPrimitive(UINT PrimitiveType,UINT BaseVertexIndex,UINT NumPrimitives)
{
	INC_DWORD_STAT(STAT_D3D9DrawPrimitiveCalls);
	INC_DWORD_STAT_BY(STAT_D3D9Triangles,(DWORD)(PrimitiveType != PT_LineList ? NumPrimitives : 0));
	INC_DWORD_STAT_BY(STAT_D3D9Lines,(DWORD)(PrimitiveType == PT_LineList ? NumPrimitives : 0));
	if ( !IsDeviceLost() )
	{
		Direct3DDevice->DrawPrimitive(
			GetD3D9PrimitiveType(PrimitiveType),
			BaseVertexIndex,
			NumPrimitives
			);
		check(UpdateStreamForInstancingMask == 0);
		check(PendingNumInstances < 2);
	}
}

void FD3D9DynamicRHI::DrawIndexedPrimitive(FIndexBufferRHIParamRef IndexBufferRHI,UINT PrimitiveType,INT BaseVertexIndex,UINT MinIndex,UINT NumVertices,UINT StartIndex,UINT NumPrimitives)
{
	DYNAMIC_CAST_D3D9RESOURCE(IndexBuffer,IndexBuffer);

	INC_DWORD_STAT(STAT_D3D9DrawPrimitiveCalls);
	INC_DWORD_STAT_BY(STAT_D3D9Triangles,(DWORD)(PrimitiveType != PT_LineList ? NumPrimitives : 0));
	INC_DWORD_STAT_BY(STAT_D3D9Lines,(DWORD)(PrimitiveType == PT_LineList ? NumPrimitives : 0));
	if ( !IsDeviceLost() )
	{
		Direct3DDevice->SetIndices(IndexBuffer);
		Direct3DDevice->DrawIndexedPrimitive(
			GetD3D9PrimitiveType(PrimitiveType),
			BaseVertexIndex,
			MinIndex,
			NumVertices,
			StartIndex,
			NumPrimitives
			);

		if (PendingNumInstances > 1 && UpdateStreamForInstancingMask)
		{
			for (UINT Instance = 1; Instance < PendingNumInstances; Instance++)
			{
				// Set the instance-indexed vertex streams with a base address of the current instance.
				UINT InstancingMask = UpdateStreamForInstancingMask;
				for (UINT StreamIndex = 0; StreamIndex < NumVertexStreams && InstancingMask; StreamIndex++)
				{
					if (InstancingMask & 1)
					{
						FD3D9VertexBuffer* VertexBuffer = (FD3D9VertexBuffer*)PendingStreams[StreamIndex].VertexBuffer;
						Direct3DDevice->SetStreamSource(
							StreamIndex,
							VertexBuffer,
							PendingStreams[StreamIndex].Stride * Instance,
							0
							);
					}
					InstancingMask >>= 1;
				}

				// Draw this instance.
				Direct3DDevice->DrawIndexedPrimitive(
					GetD3D9PrimitiveType(PrimitiveType),
					BaseVertexIndex,
					MinIndex,
					NumVertices,
					StartIndex,
					NumPrimitives
					);
			}

			// Reset the instanced vertex state.
			UINT InstancingMask = UpdateStreamForInstancingMask;
			for (UINT StreamIndex = 0; StreamIndex < NumVertexStreams && InstancingMask; StreamIndex++)
			{
				if (InstancingMask & 1)
				{
					PendingStreams[StreamIndex].VertexBuffer = 0;
				}
				InstancingMask >>= 1;
			}
			UpdateStreamForInstancingMask = 0;
			PendingNumInstances = 1;
		}
	}
}

void FD3D9DynamicRHI::DrawIndexedPrimitive_PreVertexShaderCulling(FIndexBufferRHIParamRef IndexBuffer,UINT PrimitiveType,INT BaseVertexIndex,UINT MinIndex,UINT NumVertices,UINT StartIndex,UINT NumPrimitives,const FMatrix& LocalToWorld)
{
	// On PC, don't use pre-vertex-shader culling.
	DrawIndexedPrimitive(IndexBuffer,PrimitiveType,BaseVertexIndex,MinIndex,NumVertices,StartIndex,NumPrimitives);
}

/**
 * Preallocate memory or get a direct command stream pointer to fill up for immediate rendering . This avoids memcpys below in DrawPrimitiveUP
 * @param PrimitiveType The type (triangles, lineloop, etc) of primitive to draw
 * @param NumPrimitives The number of primitives in the VertexData buffer
 * @param NumVertices The number of vertices to be written
 * @param VertexDataStride Size of each vertex 
 * @param OutVertexData Reference to the allocated vertex memory
 */
void FD3D9DynamicRHI::BeginDrawPrimitiveUP(UINT PrimitiveType, UINT NumPrimitives, UINT NumVertices, UINT VertexDataStride, void*& OutVertexData)
{
	check(!PendingBegunDrawPrimitiveUP);

	if((UINT)PendingDrawPrimitiveUPVertexData.Num() < NumVertices * VertexDataStride)
	{
		PendingDrawPrimitiveUPVertexData.Empty(NumVertices * VertexDataStride);
		PendingDrawPrimitiveUPVertexData.Add(NumVertices * VertexDataStride - PendingDrawPrimitiveUPVertexData.Num());
	}
	OutVertexData = &PendingDrawPrimitiveUPVertexData(0);

	PendingPrimitiveType = PrimitiveType;
	PendingNumPrimitives = NumPrimitives;
	PendingNumVertices = NumVertices;
	PendingVertexDataStride = VertexDataStride;
	PendingBegunDrawPrimitiveUP = TRUE;
}

/**
 * Draw a primitive using the vertex data populated since RHIBeginDrawPrimitiveUP and clean up any memory as needed
 */
void FD3D9DynamicRHI::EndDrawPrimitiveUP()
{
	check(PendingBegunDrawPrimitiveUP);
	PendingBegunDrawPrimitiveUP = FALSE;

	// for now (while RHIDrawPrimitiveUP still exists), just call it because it does the same work we need here
	RHIDrawPrimitiveUP(PendingPrimitiveType, PendingNumPrimitives, &PendingDrawPrimitiveUPVertexData(0), PendingVertexDataStride);
}
/**
 * Draw a primitive using the vertices passed in
 * VertexData is NOT created by BeginDrawPrimitveUP
 * @param PrimitiveType The type (triangles, lineloop, etc) of primitive to draw
 * @param NumPrimitives The number of primitives in the VertexData buffer
 * @param VertexData A reference to memory preallocate in RHIBeginDrawPrimitiveUP
 * @param VertexDataStride The size of one vertex
 */
void FD3D9DynamicRHI::DrawPrimitiveUP(UINT PrimitiveType, UINT NumPrimitives, const void* VertexData,UINT VertexDataStride)
{
	INC_DWORD_STAT(STAT_D3D9DrawPrimitiveCalls);
	INC_DWORD_STAT_BY(STAT_D3D9Triangles,(DWORD)(PrimitiveType != PT_LineList ? NumPrimitives : 0));
	INC_DWORD_STAT_BY(STAT_D3D9Lines,(DWORD)(PrimitiveType == PT_LineList ? NumPrimitives : 0));
	if ( !IsDeviceLost() )
	{
		// Reset vertex stream 0's frequency.
		Direct3DDevice->SetStreamSourceFreq(0,1);

		Direct3DDevice->DrawPrimitiveUP(
			GetD3D9PrimitiveType(PrimitiveType),
			NumPrimitives,
			VertexData,
			VertexDataStride
			);
	}
}

/**
 * Preallocate memory or get a direct command stream pointer to fill up for immediate rendering . This avoids memcpys below in DrawIndexedPrimitiveUP
 * @param PrimitiveType The type (triangles, lineloop, etc) of primitive to draw
 * @param NumPrimitives The number of primitives in the VertexData buffer
 * @param NumVertices The number of vertices to be written
 * @param VertexDataStride Size of each vertex
 * @param OutVertexData Reference to the allocated vertex memory
 * @param MinVertexIndex The lowest vertex index used by the index buffer
 * @param NumIndices Number of indices to be written
 * @param IndexDataStride Size of each index (either 2 or 4 bytes)
 * @param OutIndexData Reference to the allocated index memory
 */
void FD3D9DynamicRHI::BeginDrawIndexedPrimitiveUP(UINT PrimitiveType, UINT NumPrimitives, UINT NumVertices, UINT VertexDataStride, void*& OutVertexData, UINT MinVertexIndex, UINT NumIndices, UINT IndexDataStride, void*& OutIndexData)
{
	check(!PendingBegunDrawPrimitiveUP);

	if((UINT)PendingDrawPrimitiveUPVertexData.Num() < NumVertices * VertexDataStride)
	{
		PendingDrawPrimitiveUPVertexData.Empty(NumVertices * VertexDataStride);
		PendingDrawPrimitiveUPVertexData.Add(NumVertices * VertexDataStride - PendingDrawPrimitiveUPVertexData.Num());
	}
	OutVertexData = &PendingDrawPrimitiveUPVertexData(0);

	if((UINT)PendingDrawPrimitiveUPIndexData.Num() < NumIndices * IndexDataStride)
	{
		PendingDrawPrimitiveUPIndexData.Empty(NumIndices * IndexDataStride);
		PendingDrawPrimitiveUPIndexData.Add(NumIndices * IndexDataStride - PendingDrawPrimitiveUPIndexData.Num());
	}
	OutIndexData = &PendingDrawPrimitiveUPIndexData(0);

	check((sizeof(WORD) == IndexDataStride) || (sizeof(DWORD) == IndexDataStride));

	PendingPrimitiveType = PrimitiveType;
	PendingNumPrimitives = NumPrimitives;
	PendingMinVertexIndex = MinVertexIndex;
	PendingIndexDataStride = IndexDataStride;

	PendingNumVertices = NumVertices;
	PendingVertexDataStride = VertexDataStride;
	
	PendingBegunDrawPrimitiveUP = TRUE;
}

/**
 * Draw a primitive using the vertex and index data populated since RHIBeginDrawIndexedPrimitiveUP and clean up any memory as needed
 */
void FD3D9DynamicRHI::EndDrawIndexedPrimitiveUP()
{
	check(PendingBegunDrawPrimitiveUP);
	PendingBegunDrawPrimitiveUP = FALSE;

	// for now (while RHIDrawPrimitiveUP still exists), just call it because it does the same work we need here
	RHIDrawIndexedPrimitiveUP(PendingPrimitiveType, PendingMinVertexIndex, PendingNumVertices, PendingNumPrimitives, &PendingDrawPrimitiveUPIndexData(0), PendingIndexDataStride, &PendingDrawPrimitiveUPVertexData(0), PendingVertexDataStride);
}

/**
 * Draw a primitive using the vertices passed in as described the passed in indices. 
 * IndexData and VertexData are NOT created by BeginDrawIndexedPrimitveUP
 * @param PrimitiveType The type (triangles, lineloop, etc) of primitive to draw
 * @param MinVertexIndex The lowest vertex index used by the index buffer
 * @param NumVertices The number of vertices in the vertex buffer
 * @param NumPrimitives THe number of primitives described by the index buffer
 * @param IndexData The memory preallocated in RHIBeginDrawIndexedPrimitiveUP
 * @param IndexDataStride The size of one index
 * @param VertexData The memory preallocate in RHIBeginDrawIndexedPrimitiveUP
 * @param VertexDataStride The size of one vertex
 */
void FD3D9DynamicRHI::DrawIndexedPrimitiveUP(UINT PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT NumPrimitives, const void* IndexData, UINT IndexDataStride, const void* VertexData, UINT VertexDataStride)
{
	INC_DWORD_STAT(STAT_D3D9DrawPrimitiveCalls);
	INC_DWORD_STAT_BY(STAT_D3D9Triangles,(DWORD)(PrimitiveType != PT_LineList ? NumPrimitives : 0));
	INC_DWORD_STAT_BY(STAT_D3D9Lines,(DWORD)(PrimitiveType == PT_LineList ? NumPrimitives : 0));
	if ( !IsDeviceLost() )
	{
		// Reset vertex stream 0's frequency.
		Direct3DDevice->SetStreamSourceFreq(0,1);

		Direct3DDevice->DrawIndexedPrimitiveUP(
			GetD3D9PrimitiveType(PrimitiveType),
			MinVertexIndex,
			NumVertices,
			NumPrimitives,
			IndexData,
			IndexDataStride == sizeof(WORD) ? D3DFMT_INDEX16 : D3DFMT_INDEX32,
			VertexData,
			VertexDataStride
			);
	}
}

/**
 * Draw a sprite particle emitter.
 *
 * @param Mesh The mesh element containing the data for rendering the sprite particles
 */
void FD3D9DynamicRHI::DrawSpriteParticles(const FMeshElement& Mesh)
{
	check(Mesh.DynamicVertexData);
	FDynamicSpriteEmitterData* SpriteData = (FDynamicSpriteEmitterData*)(Mesh.DynamicVertexData);

	// Sort the particles if required
	INT ParticleCount = SpriteData->Source.ActiveParticleCount;

	// 'clamp' the number of particles actually drawn
	//@todo.SAS. If sorted, we really want to render the front 'N' particles...
	// right now it renders the back ones. (Same for SubUV draws)
	INT StartIndex = 0;
	INT EndIndex = ParticleCount;
	if ((SpriteData->Source.MaxDrawCount >= 0) && (ParticleCount > SpriteData->Source.MaxDrawCount))
	{
		ParticleCount = SpriteData->Source.MaxDrawCount;
	}

	TArray<FParticleOrder>* ParticleOrder = (TArray<FParticleOrder>*)(Mesh.DynamicIndexData);

	// Render the particles are indexed tri-lists
	void* OutVertexData = NULL;
	void* OutIndexData = NULL;

	// Get the memory from the device for copying the particle vertex/index data to
	RHIBeginDrawIndexedPrimitiveUP(PT_TriangleList, 
		ParticleCount * 2, ParticleCount * 4, Mesh.DynamicVertexStride, OutVertexData, 
		0, ParticleCount * 6, sizeof(WORD), OutIndexData);

	if (OutVertexData && OutIndexData)
	{
		// Pack the data
		FParticleSpriteVertex* Vertices = (FParticleSpriteVertex*)OutVertexData;
		SpriteData->GetVertexAndIndexData(Vertices, OutIndexData, ParticleOrder);
		// End the draw, which will submit the data for rendering
		RHIEndDrawIndexedPrimitiveUP();
	}
}

/**
 * Draw a sprite subuv particle emitter.
 *
 * @param Mesh The mesh element containing the data for rendering the sprite subuv particles
 */
void FD3D9DynamicRHI::DrawSubUVParticles(const FMeshElement& Mesh)
{
	check(Mesh.DynamicVertexData);
	FDynamicSubUVEmitterData* SubUVData = (FDynamicSubUVEmitterData*)(Mesh.DynamicVertexData);

	// Sort the particles if required
	INT ParticleCount = SubUVData->Source.ActiveParticleCount;

	// 'clamp' the number of particles actually drawn
	//@todo.SAS. If sorted, we really want to render the front 'N' particles...
	// right now it renders the back ones. (Same for SubUV draws)
	INT StartIndex = 0;
	INT EndIndex = ParticleCount;
	if ((SubUVData->Source.MaxDrawCount >= 0) && (ParticleCount > SubUVData->Source.MaxDrawCount))
	{
		ParticleCount = SubUVData->Source.MaxDrawCount;
	}

	TArray<FParticleOrder>* ParticleOrder = (TArray<FParticleOrder>*)(Mesh.DynamicIndexData);

	// Render the particles are indexed tri-lists
	void* OutVertexData = NULL;
	void* OutIndexData = NULL;

	// Get the memory from the device for copying the particle vertex/index data to
	RHIBeginDrawIndexedPrimitiveUP(PT_TriangleList, 
		ParticleCount * 2, ParticleCount * 4, Mesh.DynamicVertexStride, OutVertexData, 
		0, ParticleCount * 6, sizeof(WORD), OutIndexData);

	if (OutVertexData && OutIndexData)
	{
		// Pack the data
		FParticleSpriteSubUVVertex* Vertices = (FParticleSpriteSubUVVertex*)OutVertexData;
		SubUVData->GetVertexAndIndexData(Vertices, OutIndexData, ParticleOrder);
		// End the draw, which will submit the data for rendering
		RHIEndDrawIndexedPrimitiveUP();
	}
}

// Raster operations.
void FD3D9DynamicRHI::Clear(UBOOL bClearColor,const FLinearColor& Color,UBOOL bClearDepth,FLOAT Depth,UBOOL bClearStencil,DWORD Stencil)
{
	// Determine the clear flags.
	DWORD Flags = 0;
	if(bClearColor)
	{
		Flags |= D3DCLEAR_TARGET;
	}
	if(bClearDepth)
	{
		Flags |= D3DCLEAR_ZBUFFER;
	}
	if(bClearStencil)
	{
		Flags |= D3DCLEAR_STENCIL;
	}

	// Clear the render target/depth-stencil surfaces based on the flags.
	FColor QuantizedColor(Color.Quantize());
	Direct3DDevice->Clear(0,NULL,Flags,D3DCOLOR_RGBA(QuantizedColor.R,QuantizedColor.G,QuantizedColor.B,QuantizedColor.A),Depth,Stencil);
}

// Functions to yield and regain rendering control from D3D

void FD3D9DynamicRHI::SuspendRendering()
{
	// Not supported
}

void FD3D9DynamicRHI::ResumeRendering()
{
	// Not supported
}

UBOOL FD3D9DynamicRHI::IsRenderingSuspended()
{
	// Not supported
	return FALSE;
}

// Kick the rendering commands that are currently queued up in the GPU command buffer.
void FD3D9DynamicRHI::KickCommandBuffer()
{
	// Not really supported
}

// Blocks the CPU until the GPU catches up and goes idle.
void FD3D9DynamicRHI::BlockUntilGPUIdle()
{
	// Not really supported
}

/*
 * Returns the total GPU time taken to render the last frame. Same metric as appCycles().
 */
DWORD FD3D9DynamicRHI::GetGPUFrameCycles()
{
	return GGPUFrameTime;
}

/*
 * Returns an approximation of the available memory that textures can use, which is video + AGP where applicable, rounded to the nearest MB, in MB.
 */
DWORD FD3D9DynamicRHI::GetAvailableTextureMemory()
{
	//apparently GetAvailableTextureMem() returns available bytes (the docs don't say) rounded to the nearest MB.
	return Direct3DDevice->GetAvailableTextureMem() / 1048576;
}

// not used on PC

void FD3D9DynamicRHI::MSAAInitPrepass()
{
}
void FD3D9DynamicRHI::MSAAFixViewport()
{
}
void FD3D9DynamicRHI::MSAABeginRendering(UBOOL bRequiresClear)
{
}
void FD3D9DynamicRHI::MSAAEndRendering(FTexture2DRHIParamRef DepthTexture, FTexture2DRHIParamRef ColorTexture, UINT ViewIndex)
{
}
void FD3D9DynamicRHI::RestoreColorDepth(FTexture2DRHIParamRef ColorTexture, FTexture2DRHIParamRef DepthTexture)
{
}
void FD3D9DynamicRHI::GetMSAAOffsets(UINT *HalfScreenY, UINT *ResolveOffset)
{
	check(0);
}
void FD3D9DynamicRHI::SetTessellationMode( ETessellationMode TessellationMode, FLOAT MinTessellation, FLOAT MaxTessellation )
{
}
