/*=============================================================================
	D3D10Commands.cpp: D3D RHI commands implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3D10DrvPrivate.h"
#include "EngineParticleClasses.h"
#include "StaticBoundShaderState.h"

// Globals
EXTERN_C const GUID GUID_INPUT_LAYOUT_DESC = { 0x77276856, 0x4767, 0x4f7b, { 0xa7, 0xf, 0x69, 0xfc, 0xac, 0x86, 0x70, 0xcb } };
EXTERN_C const GUID GUID_VERTEX_SHADER_CODE = { 0xec4cb6f, 0x23c4, 0x41db, { 0x89, 0xdd, 0x7b, 0xe9, 0x83, 0x92, 0x55, 0x21 } };

/** Vertex declaration for just one FVector4 position. */
class FVector4VertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;
	virtual void InitRHI()
	{
		FVertexDeclarationElementList Elements;
		Elements.AddItem(FVertexElement(0,0,VET_Float4,VEU_Position,0));
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}
	virtual void ReleaseRHI()
	{
		VertexDeclarationRHI.SafeRelease();
	}
};
FGlobalBoundShaderState GClearBoundShaderState;
TGlobalResource<FVector4VertexDeclaration> GVector4VertexDeclaration;

// Vertex state.
void FD3D10DynamicRHI::SetStreamSource(UINT StreamIndex,FVertexBufferRHIParamRef VertexBufferRHI,UINT Stride,UBOOL bUseInstanceIndex,UINT NumVerticesPerInstance,UINT NumInstances)
{
	DYNAMIC_CAST_D3D10RESOURCE(VertexBuffer,VertexBuffer);

	const UINT Offsets = 0;
	ID3D10Buffer* D3DBuffer = VertexBuffer;
	Direct3DDevice->IASetVertexBuffers(StreamIndex,1,&D3DBuffer,&Stride,&Offsets);

	PendingNumInstances = NumInstances;
}

// Rasterizer state.
void FD3D10DynamicRHI::SetRasterizerState(FRasterizerStateRHIParamRef NewStateRHI)
{
	DYNAMIC_CAST_D3D10RESOURCE(RasterizerState,NewState);

	CurrentRasterizerState = NewState->RasterizerDesc;
	ID3D10RasterizerState* CachedState = GetCachedRasterizerState(CurrentRasterizerState,CurrentScissorEnable,bCurrentRenderTargetIsMultisample);
	Direct3DDevice->RSSetState( CachedState );
}

void FD3D10DynamicRHI::SetViewport(UINT MinX,UINT MinY,FLOAT MinZ,UINT MaxX,UINT MaxY,FLOAT MaxZ)
{
	D3D10_VIEWPORT Viewport = { MinX, MinY, MaxX - MinX, MaxY - MinY, MinZ, MaxZ };
	//avoid setting a 0 extent viewport, which the debug runtime doesn't like
	if (Viewport.Width > 0 && Viewport.Height > 0)
	{
		Direct3DDevice->RSSetViewports(1,&Viewport);
	}
}

void FD3D10DynamicRHI::SetScissorRect(UBOOL bEnable,UINT MinX,UINT MinY,UINT MaxX,UINT MaxY)
{
	// Defined in UnPlayer.cpp. Used here to disable scissors when doing highres screenshots.
	extern UBOOL GIsTiledScreenshot;
	bEnable = GIsTiledScreenshot ? FALSE : bEnable;

	if(bEnable)
	{
		D3D10_RECT ScissorRect;
		ScissorRect.left = MinX;
		ScissorRect.right = MaxX;
		ScissorRect.top = MinY;
		ScissorRect.bottom = MaxY;
		Direct3DDevice->RSSetScissorRects(1,&ScissorRect);
	}

	CurrentScissorEnable = bEnable;
	ID3D10RasterizerState* CachedState = GetCachedRasterizerState(CurrentRasterizerState,CurrentScissorEnable,bCurrentRenderTargetIsMultisample);
	Direct3DDevice->RSSetState(CachedState);
}

/**
 * Set depth bounds test state.
 * When enabled, incoming fragments are killed if the value already in the depth buffer is outside of [MinZ, MaxZ]
 */
void FD3D10DynamicRHI::SetDepthBoundsTest( UBOOL bEnable, const FVector4 &ClipSpaceNearPos, const FVector4 &ClipSpaceFarPos)
{
	// not supported
}

/**
* Set bound shader state. This will set the vertex decl/shader, and pixel shader
* @param BoundShaderState - state resource
*/
void FD3D10DynamicRHI::SetBoundShaderState( FBoundShaderStateRHIParamRef BoundShaderStateRHI)
{
	DYNAMIC_CAST_D3D10RESOURCE(BoundShaderState,BoundShaderState);

	Direct3DDevice->IASetInputLayout(BoundShaderState->InputLayout);
	Direct3DDevice->VSSetShader(BoundShaderState->VertexShader);
	Direct3DDevice->PSSetShader(BoundShaderState->PixelShader);

	// Prevent the bound shader state's resources from being deleted while they're bound to the D3D pipeline.
	PipelineResources.AddItem((IUnknown*)BoundShaderState->InputLayout);

	// Prevent transient bound shader states from being recreated for each use by keeping a history of the most recently used bound shader states.
	// The history keeps them alive, and the bound shader state cache allows them to be reused if needed.
	BoundShaderStateHistory.Add(BoundShaderState);
}

void FD3D10DynamicRHI::SetSamplerState(FPixelShaderRHIParamRef PixelShaderRHI,UINT TextureIndex,UINT SamplerIndex,FSamplerStateRHIParamRef NewStateRHI,FTextureRHIParamRef NewTextureRHI,FLOAT MipBias)
{
	//@TODO: Support variable MipBias

	DYNAMIC_CAST_D3D10RESOURCE(PixelShader,PixelShader);
	DYNAMIC_CAST_D3D10RESOURCE(SamplerState,NewState);
	DYNAMIC_CAST_D3D10RESOURCE(Texture,NewTexture);

	ID3D10ShaderResourceView* SRV = NewTexture->View;
	Direct3DDevice->PSSetShaderResources(TextureIndex,1,&SRV);

	ID3D10SamplerState* CachedState = GetCachedSamplerState(NewState);
	Direct3DDevice->PSSetSamplers(SamplerIndex,1,&CachedState);
}

void FD3D10DynamicRHI::SetVertexShaderParameter(FVertexShaderRHIParamRef VertexShaderRHI,UINT BufferIndex,UINT BaseIndex,UINT NumBytes,const void* NewValue)
{
	VSConstantBuffers(BufferIndex)->UpdateConstant((const BYTE*)NewValue,BaseIndex,NumBytes);
}


void FD3D10DynamicRHI::SetPixelShaderParameter(FPixelShaderRHIParamRef PixelShaderRHI,UINT BufferIndex,UINT BaseIndex,UINT NumBytes,const void* NewValue)
{
	PSConstantBuffers(BufferIndex)->UpdateConstant((const BYTE*)NewValue,BaseIndex,NumBytes);
}

void FD3D10DynamicRHI::SetPixelShaderBoolParameter(FPixelShaderRHIParamRef PixelShader,UINT BaseIndex,UBOOL NewValue)
{
	//@todo - implement
}

// These must match the cbuffer definitions in Common.usf
struct FVertexShaderOffsetConstantBufferContents
{
	FMatrix ViewProjectionMatrix;
	FVector4 ViewOrigin;
};
struct FPixelShaderOffsetConstantBufferContents
{
	FVector4 ScreenPositionScaleBias;
	FVector4 MinZ_MaxZRatio;
};

/**
 * Set engine shader parameters for the view.
 * @param View					The current view
 * @param ViewProjectionMatrix	Matrix that transforms from world space to projection space for the view
 * @param ViewOrigin			World space position of the view's origin
 */
void FD3D10DynamicRHI::SetViewParameters(  const FSceneView* View, const FMatrix& ViewProjectionMatrix, const FVector4& ViewOrigin )
{
	const FVector4 TranslatedViewOrigin = ViewOrigin + FVector4(View->PreViewTranslation,0);

	FVertexShaderOffsetConstantBufferContents VSCBContents;
	VSCBContents.ViewProjectionMatrix = ViewProjectionMatrix;
	VSCBContents.ViewOrigin = TranslatedViewOrigin;

	FPixelShaderOffsetConstantBufferContents PSCBContents;
	PSCBContents.ScreenPositionScaleBias = View->ScreenPositionScaleBias;
	PSCBContents.MinZ_MaxZRatio = View->InvDeviceZToWorldZTransform;

	// TODO: we assume that VSView constants are in CB1
	VSConstantBuffers(VS_OFFSET_CONSTANT_BUFFER)->UpdateConstant((BYTE*)&VSCBContents,0,sizeof(VSCBContents));

	// TODO: we assume that PSView constants are in CB2
	PSConstantBuffers(PS_OFFSET_CONSTANT_BUFFER)->UpdateConstant((BYTE*)&PSCBContents,0,sizeof(PSCBContents));
}

/**
 * Not used on PC
 */
void FD3D10DynamicRHI::SetViewPixelParameters(const FSceneView* View,FPixelShaderRHIParamRef PixelShader,const class FShaderParameter* SceneDepthCalcParameter,const class FShaderParameter* ScreenPositionScaleBiasParameter)
{
}
void FD3D10DynamicRHI::SetRenderTargetBias(  FLOAT ColorBias )
{
}
void FD3D10DynamicRHI::SetShaderRegisterAllocation(UINT NumVertexShaderRegisters, UINT NumPixelShaderRegisters)
{
}
void FD3D10DynamicRHI::ReduceTextureCachePenalty( FPixelShaderRHIParamRef PixelShader )
{
}

void FD3D10DynamicRHI::SetDepthState(FDepthStateRHIParamRef NewStateRHI)
{
	DYNAMIC_CAST_D3D10RESOURCE(DepthState,NewState);

	CurrentDepthState = NewState->DepthStencilDesc;
	ID3D10DepthStencilState* CachedState = GetCachedDepthStencilState(CurrentDepthState,CurrentStencilState);
	Direct3DDevice->OMSetDepthStencilState(CachedState,CurrentStencilRef);
}

void FD3D10DynamicRHI::SetStencilState(FStencilStateRHIParamRef NewStateRHI)
{
	DYNAMIC_CAST_D3D10RESOURCE(StencilState,NewState);

	CurrentStencilState = NewState->DepthStencilDesc;
	CurrentStencilRef = NewState->StencilRef;
	ID3D10DepthStencilState* CachedState = GetCachedDepthStencilState(CurrentDepthState,CurrentStencilState);
	Direct3DDevice->OMSetDepthStencilState(CachedState,CurrentStencilRef);
}

void FD3D10DynamicRHI::SetBlendState(FBlendStateRHIParamRef NewStateRHI)
{
	DYNAMIC_CAST_D3D10RESOURCE(BlendState,NewState);

	CurrentBlendState = NewState->BlendDesc;
	CurrentBlendFactor = NewState->BlendFactor;
	ID3D10BlendState* CachedState = GetCachedBlendState(CurrentBlendState,CurrentColorWriteEnable);
	Direct3DDevice->OMSetBlendState(CachedState,(FLOAT*)&CurrentBlendFactor,0xFFFFFFFF);
}

void FD3D10DynamicRHI::SetRenderTarget( FSurfaceRHIParamRef NewRenderTargetRHI, FSurfaceRHIParamRef NewDepthStencilTargetRHI)
{
	DYNAMIC_CAST_D3D10RESOURCE(Surface,NewRenderTarget);
	DYNAMIC_CAST_D3D10RESOURCE(Surface,NewDepthStencilTarget);

	// Reset all texture references, to ensure a reference to this render target doesn't remain set.
	for(UINT TextureIndex = 0;TextureIndex < 16;TextureIndex++)
	{
		ID3D10ShaderResourceView* NullView = NULL;
		Direct3DDevice->PSSetShaderResources(TextureIndex,1,&NullView);
	}

	ID3D10RenderTargetView* RTV = NULL;
	ID3D10DepthStencilView* DSV = NULL;
	if(NewRenderTarget)
	{
		RTV = NewRenderTarget->RenderTargetView;
	}
	if(NewDepthStencilTarget)
	{
		DSV = NewDepthStencilTarget->DepthStencilView;
	}

	Direct3DDevice->OMSetRenderTargets(1,&RTV,DSV);

	// Detect when the back buffer is being set, and set the correct viewport.
	if( DrawingViewport && (!NewRenderTarget || NewRenderTarget == DrawingViewport->GetBackBuffer()) )
	{
		RHISetViewport(0,0,0.0f,DrawingViewport->GetSizeX(),DrawingViewport->GetSizeY(),1.0f);
	}
	else if(RTV)
	{
		// Set the viewport to the full size of the surface
		D3D10_TEXTURE2D_DESC Desc;
		ID3D10Texture2D* BaseResource = NULL;
		RTV->GetResource((ID3D10Resource**)&BaseResource);
		BaseResource->GetDesc(&Desc);

		RHISetViewport(0,0,0.0f,Desc.Width,Desc.Height,1.0f);

		BaseResource->Release();
	}

	// Determine whether the new render target is multisample.
	if(RTV)
	{
		D3D10_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc;
		RTV->GetDesc(&RenderTargetViewDesc);
		bCurrentRenderTargetIsMultisample =
			(RenderTargetViewDesc.ViewDimension == D3D10_RTV_DIMENSION_TEXTURE2DMS) ||
			(RenderTargetViewDesc.ViewDimension == D3D10_RTV_DIMENSION_TEXTURE2DMSARRAY);

		// Update the rasterizer state to take into account whether new render target is multi-sample.
		ID3D10RasterizerState* CachedState = GetCachedRasterizerState(CurrentRasterizerState,CurrentScissorEnable,bCurrentRenderTargetIsMultisample);
		Direct3DDevice->RSSetState(CachedState);
	}
}
void FD3D10DynamicRHI::SetMRTRenderTarget( FSurfaceRHIParamRef NewRenderTargetRHI, UINT TargetIndex)
{
	//@todo: MRT support for D3D10
	check(0);
}
void FD3D10DynamicRHI::SetColorWriteEnable(UBOOL bEnable)
{
	UINT8 EnabledStateValue = bEnable ? D3D10_COLOR_WRITE_ENABLE_ALL : 0;
	CurrentColorWriteEnable = EnabledStateValue;
	ID3D10BlendState* CachedState = GetCachedBlendState(CurrentBlendState,CurrentColorWriteEnable);
	Direct3DDevice->OMSetBlendState(CachedState,(FLOAT*)&CurrentBlendFactor,0xFFFFFFFF);
}
void FD3D10DynamicRHI::SetColorWriteMask( UINT ColorWriteMask)
{
	UINT8 EnabledStateValue;
	EnabledStateValue  = (ColorWriteMask & CW_RED) ? D3D10_COLOR_WRITE_ENABLE_RED : 0;
	EnabledStateValue |= (ColorWriteMask & CW_GREEN) ? D3D10_COLOR_WRITE_ENABLE_GREEN : 0;
	EnabledStateValue |= (ColorWriteMask & CW_BLUE) ? D3D10_COLOR_WRITE_ENABLE_BLUE : 0;
	EnabledStateValue |= (ColorWriteMask & CW_ALPHA) ? D3D10_COLOR_WRITE_ENABLE_ALPHA : 0;

	CurrentColorWriteEnable = EnabledStateValue;
	ID3D10BlendState* CachedState = GetCachedBlendState(CurrentBlendState,CurrentColorWriteEnable);
	Direct3DDevice->OMSetBlendState(CachedState,(FLOAT*)&CurrentBlendFactor,0xFFFFFFFF);
}

// Not supported
void FD3D10DynamicRHI::BeginHiStencilRecord(UBOOL bCompareFunctionEqual, UINT RefValue) { }
void FD3D10DynamicRHI::BeginHiStencilPlayback(UBOOL bFlush) { }
void FD3D10DynamicRHI::EndHiStencil() { }

// Occlusion queries.
void FD3D10DynamicRHI::BeginOcclusionQuery(FOcclusionQueryRHIParamRef OcclusionQueryRHI)
{
	DYNAMIC_CAST_D3D10RESOURCE(OcclusionQuery,OcclusionQuery);
	OcclusionQuery->Resource->Begin();
}
void FD3D10DynamicRHI::EndOcclusionQuery(FOcclusionQueryRHIParamRef OcclusionQueryRHI)
{
	DYNAMIC_CAST_D3D10RESOURCE(OcclusionQuery,OcclusionQuery);
	OcclusionQuery->Resource->End();
}

// Primitive drawing.

static D3D10_PRIMITIVE_TOPOLOGY GetD3D10PrimitiveType(UINT PrimitiveType)
{
	switch(PrimitiveType)
	{
	case PT_TriangleList: return D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case PT_TriangleStrip: return D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	case PT_LineList: return D3D10_PRIMITIVE_TOPOLOGY_LINELIST;
	default: appErrorf(TEXT("Unknown primitive type: %u"),PrimitiveType);
	};
	return D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

static UINT GetVertexCountForPrimitiveCount(UINT NumPrimitives, UINT PrimitiveType)
{
	UINT VertexCount = 0;
	switch(PrimitiveType)
	{
	case PT_TriangleList: VertexCount = NumPrimitives*3; break;
	case PT_TriangleStrip: VertexCount = NumPrimitives+2; break;
	case PT_LineList: VertexCount = NumPrimitives*2; break;
	default: appErrorf(TEXT("Unknown primitive type: %u"),PrimitiveType);
	};

	return VertexCount;
}

void FD3D10DynamicRHI::CommitVertexAndPixelShaderConstants()
{
	// Commit and bind vertex shader constants
	for(UINT i=0;i<MAX_CONSTANT_BUFFER_SLOTS; i++)
	{
		FD3D10ConstantBuffer* ConstantBuffer = VSConstantBuffers(i);
		if(ConstantBuffer->CommitConstantsToDevice())
		{
			ID3D10Buffer* DeviceBuffer = ConstantBuffer->GetConstantBuffer();
			Direct3DDevice->VSSetConstantBuffers(i,1,&DeviceBuffer);
		}
	}

	// Commit and bind pixel shader constants
	for(UINT i=0;i<MAX_CONSTANT_BUFFER_SLOTS; i++)
	{
		FD3D10ConstantBuffer* ConstantBuffer = PSConstantBuffers(i);
		if(ConstantBuffer->CommitConstantsToDevice())
		{
			ID3D10Buffer* DeviceBuffer = ConstantBuffer->GetConstantBuffer();
			Direct3DDevice->PSSetConstantBuffers(i,1,&DeviceBuffer);
		}
	}
}

void FD3D10DynamicRHI::DrawPrimitive(UINT PrimitiveType,UINT BaseVertexIndex,UINT NumPrimitives)
{
	INC_DWORD_STAT(STAT_D3D10DrawPrimitiveCalls);
	INC_DWORD_STAT_BY(STAT_D3D10Triangles,(DWORD)(PrimitiveType != PT_LineList ? NumPrimitives : 0));
	INC_DWORD_STAT_BY(STAT_D3D10Lines,(DWORD)(PrimitiveType == PT_LineList ? NumPrimitives : 0));

	CommitVertexAndPixelShaderConstants();
	UINT VertexCount = GetVertexCountForPrimitiveCount(NumPrimitives,PrimitiveType);
	Direct3DDevice->IASetPrimitiveTopology(GetD3D10PrimitiveType(PrimitiveType));

	if(PendingNumInstances > 1)
	{
		Direct3DDevice->DrawInstanced(VertexCount,PendingNumInstances,BaseVertexIndex,0);
	}
	else
	{
		Direct3DDevice->Draw(VertexCount,BaseVertexIndex);
	}
}

void FD3D10DynamicRHI::DrawIndexedPrimitive(FIndexBufferRHIParamRef IndexBufferRHI,UINT PrimitiveType,INT BaseVertexIndex,UINT MinIndex,UINT NumVertices,UINT StartIndex,UINT NumPrimitives)
{
	DYNAMIC_CAST_D3D10RESOURCE(IndexBuffer,IndexBuffer);

	INC_DWORD_STAT(STAT_D3D10DrawPrimitiveCalls);
	INC_DWORD_STAT_BY(STAT_D3D10Triangles,(DWORD)(PrimitiveType != PT_LineList ? NumPrimitives : 0));
	INC_DWORD_STAT_BY(STAT_D3D10Lines,(DWORD)(PrimitiveType == PT_LineList ? NumPrimitives : 0));

	CommitVertexAndPixelShaderConstants();
	// determine 16bit vs 32bit indices
	UINT SizeFormat = sizeof(DXGI_FORMAT);
	DXGI_FORMAT Format = DXGI_FORMAT_R32_UINT;
	IndexBuffer->GetPrivateData(GUID_INDEX_FORMAT,&SizeFormat,&Format);

	UINT IndexCount = GetVertexCountForPrimitiveCount(NumPrimitives,PrimitiveType);
	Direct3DDevice->IASetIndexBuffer(IndexBuffer,Format,0);
	Direct3DDevice->IASetPrimitiveTopology(GetD3D10PrimitiveType(PrimitiveType));

	if(PendingNumInstances > 1)
	{
		Direct3DDevice->DrawIndexedInstanced(IndexCount,PendingNumInstances,StartIndex,BaseVertexIndex,0);
	}
	else
	{
		Direct3DDevice->DrawIndexed(IndexCount,StartIndex,BaseVertexIndex);
	}

	// Prevent the index buffer resource from being deleted while it's bound to the D3D pipeline.
	PipelineResources.AddItem((IUnknown*)IndexBuffer);
}

void FD3D10DynamicRHI::DrawIndexedPrimitive_PreVertexShaderCulling(FIndexBufferRHIParamRef IndexBuffer,UINT PrimitiveType,INT BaseVertexIndex,UINT MinIndex,UINT NumVertices,UINT StartIndex,UINT NumPrimitives,const FMatrix& LocalToWorld)
{
	// On PC, don't use pre-vertex-shader culling.
	DrawIndexedPrimitive(IndexBuffer,PrimitiveType,BaseVertexIndex,MinIndex,NumVertices,StartIndex,NumPrimitives);
}

void* FD3D10DynamicRHI::CreateVertexDataBuffer(UINT Size)
{
	if( Size > StaticDataSize )
	{
		if( StaticData )
		{
			appFree(StaticData);
		}

		StaticDataSize = Size;
		StaticData = appMalloc( Max<UINT>(StaticDataSize,UserDataBufferSize) );
	}
	return StaticData;
}

void FD3D10DynamicRHI::ReleaseDynamicVBandIBBuffers()
{
	for(UINT i = 0;i < NumUserDataBuffers;i++)
	{
		if(DynamicVertexBufferArray[i])
			DynamicVertexBufferArray[i] = NULL;
		if(DynamicIndexBufferArray[i])
			DynamicIndexBufferArray[i] = NULL;
	}

	if(StaticData)
		appFree(StaticData);
}

/**
 * Returns an appropriate D3D10 buffer from an array of buffers.  It also makes sure the buffer is of the proper size.
 * @param Count The number of objects in the buffer
 * @param Stride The stride of each object
 * @param Binding Which type of binding (VB or IB) this buffer will need
 */
ID3D10Buffer* FD3D10DynamicRHI::EnsureBufferSize(UINT Count, UINT Stride, D3D10_BIND_FLAG Binding)
{
	TRefCountPtr<ID3D10Buffer>* BufferArray = (Binding & D3D10_BIND_VERTEX_BUFFER) ? DynamicVertexBufferArray : DynamicIndexBufferArray;
	UINT* Current = (Binding & D3D10_BIND_VERTEX_BUFFER) ? &CurrentDynamicVB : &CurrentDynamicIB;
	UBOOL CreateBuffer = FALSE;
	UINT SizeNeeded = Count*Stride;

	if(BufferArray[*Current])
	{
		D3D10_BUFFER_DESC Desc;
		BufferArray[*Current]->GetDesc(&Desc);
		if(Desc.ByteWidth < SizeNeeded)
		{
			CreateBuffer = TRUE;
			// This will internally release the buffer
			BufferArray[*Current] = NULL;
		}
	}
	else
	{
		CreateBuffer = TRUE;
	}

	if(CreateBuffer)
	{
		D3D10_BUFFER_DESC Desc;
		Desc.ByteWidth = Max<UINT>(SizeNeeded,UserDataBufferSize);
		Desc.Usage = D3D10_USAGE_DYNAMIC;
		Desc.BindFlags = Binding;
		Desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		Desc.MiscFlags = 0;
		appOutputDebugString(*FString::Printf( TEXT("%d Byte buffer created\n"),SizeNeeded));

		VERIFYD3D10RESULT(Direct3DDevice->CreateBuffer(&Desc,NULL,BufferArray[*Current].GetInitReference()));
	}

	ID3D10Buffer* retBuffer = BufferArray[*Current];
	(*Current) ++;
	if(*Current >= NumUserDataBuffers)
	{
		*Current = 0;
	}
	return retBuffer;
}

/**
 * Fills a D3D10 buffer with the input data.  Returns the D3D10 buffer with the data.
 * @param Count The number of objects in the buffer
 * @param Stride The stride of each object
 * @param Data The actual buffer data
 * @param Binding Which type of binding (VB or IB) this buffer will need
 */
ID3D10Buffer* FD3D10DynamicRHI::FillD3D10Buffer(UINT Count, UINT Stride, const void* Data, D3D10_BIND_FLAG Binding, UINT& OutOffset)
{
	UINT* CurrentOffset = (Binding & D3D10_BIND_VERTEX_BUFFER) ? &CurrentVBOffset : &CurrentIBOffset;

	UINT NewOffset = *CurrentOffset + Count*Stride;
	if( NewOffset >= UserDataBufferSize )
	{
		*CurrentOffset = 0;
		NewOffset = *CurrentOffset + Count*Stride;
	}
	OutOffset = *CurrentOffset;
	*CurrentOffset = NewOffset;

	ID3D10Buffer* Buffer = EnsureBufferSize(Count,Stride,Binding);
	BYTE* MappedData = NULL;
	Buffer->Map(D3D10_MAP_WRITE_NO_OVERWRITE,0,(void**)&MappedData);
	appMemcpy(MappedData+OutOffset,Data,Count*Stride);
	Buffer->Unmap();

	return Buffer;
}

/**
 * Preallocate memory or get a direct command stream pointer to fill up for immediate rendering . This avoids memcpys below in DrawPrimitiveUP
 * @param PrimitiveType The type (triangles, lineloop, etc) of primitive to draw
 * @param NumPrimitives The number of primitives in the VertexData buffer
 * @param NumVertices The number of vertices to be written
 * @param VertexDataStride Size of each vertex 
 * @param OutVertexData Reference to the allocated vertex memory
 */
void FD3D10DynamicRHI::BeginDrawPrimitiveUP( UINT PrimitiveType, UINT NumPrimitives, UINT NumVertices, UINT VertexDataStride, void*& OutVertexData)
{
	check(NULL == PendingDrawPrimitiveUPVertexData);
	PendingDrawPrimitiveUPVertexData = CreateVertexDataBuffer(NumVertices * VertexDataStride);
	OutVertexData = PendingDrawPrimitiveUPVertexData;

	PendingPrimitiveType = PrimitiveType;
	PendingNumPrimitives = NumPrimitives;
	PendingNumVertices = NumVertices;
	PendingVertexDataStride = VertexDataStride;
}

/**
 * Draw a primitive using the vertex data populated since RHIBeginDrawPrimitiveUP and clean up any memory as needed
 */
void FD3D10DynamicRHI::EndDrawPrimitiveUP()
{
	check(NULL != PendingDrawPrimitiveUPVertexData);

	CommitVertexAndPixelShaderConstants();

	// for now (while RHIDrawPrimitiveUP still exists), just call it because it does the same work we need here
	RHIDrawPrimitiveUP(PendingPrimitiveType, PendingNumPrimitives, PendingDrawPrimitiveUPVertexData, PendingVertexDataStride);

	// free used mem
	PendingDrawPrimitiveUPVertexData = NULL;
}
/**
 * Draw a primitive using the vertices passed in
 * VertexData is NOT created by BeginDrawPrimitveUP
 * @param PrimitiveType The type (triangles, lineloop, etc) of primitive to draw
 * @param NumPrimitives The number of primitives in the VertexData buffer
 * @param VertexData A reference to memory preallocate in RHIBeginDrawPrimitiveUP
 * @param VertexDataStride The size of one vertex
 */
void FD3D10DynamicRHI::DrawPrimitiveUP( UINT PrimitiveType, UINT NumPrimitives, const void* VertexData,UINT VertexDataStride)
{
	INC_DWORD_STAT(STAT_D3D10DrawPrimitiveCalls);
	INC_DWORD_STAT_BY(STAT_D3D10Triangles,(DWORD)(PrimitiveType != PT_LineList ? NumPrimitives : 0));
	INC_DWORD_STAT_BY(STAT_D3D10Lines,(DWORD)(PrimitiveType == PT_LineList ? NumPrimitives : 0));

	UINT VertexCount = GetVertexCountForPrimitiveCount(NumPrimitives,PrimitiveType);
	UINT DrawOffset = 0;
	ID3D10Buffer* Buffer = FillD3D10Buffer(VertexCount,VertexDataStride,VertexData,D3D10_BIND_VERTEX_BUFFER,DrawOffset);
	UINT Offset = DrawOffset;

	CommitVertexAndPixelShaderConstants();
	Direct3DDevice->IASetVertexBuffers(0,1,&Buffer,&VertexDataStride,&Offset);
	Direct3DDevice->IASetPrimitiveTopology(GetD3D10PrimitiveType(PrimitiveType));
	Direct3DDevice->Draw(VertexCount,0);
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
void FD3D10DynamicRHI::BeginDrawIndexedPrimitiveUP( UINT PrimitiveType, UINT NumPrimitives, UINT NumVertices, UINT VertexDataStride, void*& OutVertexData, UINT MinVertexIndex, UINT NumIndices, UINT IndexDataStride, void*& OutIndexData)
{
	check(NULL == PendingDrawPrimitiveUPVertexData);
	check(NULL == PendingDrawPrimitiveUPIndexData);

	PendingDrawPrimitiveUPVertexData = CreateVertexDataBuffer(NumVertices * VertexDataStride + NumIndices * IndexDataStride);
	OutVertexData = PendingDrawPrimitiveUPVertexData;

	PendingDrawPrimitiveUPIndexData = (BYTE*)PendingDrawPrimitiveUPVertexData + (NumVertices * VertexDataStride);
	OutIndexData = PendingDrawPrimitiveUPIndexData;

	check((sizeof(WORD) == IndexDataStride) || (sizeof(DWORD) == IndexDataStride));

	PendingPrimitiveType = PrimitiveType;
	PendingNumPrimitives = NumPrimitives;
	PendingMinVertexIndex = MinVertexIndex;
	PendingIndexDataStride = IndexDataStride;

	PendingNumVertices = NumVertices;
	PendingVertexDataStride = VertexDataStride;
}

/**
 * Draw a primitive using the vertex and index data populated since RHIBeginDrawIndexedPrimitiveUP and clean up any memory as needed
 */
void FD3D10DynamicRHI::EndDrawIndexedPrimitiveUP()
{
	check(NULL != PendingDrawPrimitiveUPVertexData);
	check(NULL != PendingDrawPrimitiveUPIndexData);

	CommitVertexAndPixelShaderConstants();

	// for now (while RHIDrawPrimitiveUP still exists), just call it because it does the same work we need here
	RHIDrawIndexedPrimitiveUP( PendingPrimitiveType, PendingMinVertexIndex, PendingNumVertices, PendingNumPrimitives, PendingDrawPrimitiveUPIndexData, PendingIndexDataStride, PendingDrawPrimitiveUPVertexData, PendingVertexDataStride);
	
	// free used mem
	PendingDrawPrimitiveUPIndexData = NULL;
	PendingDrawPrimitiveUPVertexData = NULL;
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
void FD3D10DynamicRHI::DrawIndexedPrimitiveUP( UINT PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT NumPrimitives, const void* IndexData, UINT IndexDataStride, const void* VertexData, UINT VertexDataStride)
{
	INC_DWORD_STAT(STAT_D3D10DrawPrimitiveCalls);
	INC_DWORD_STAT_BY(STAT_D3D10Triangles,(DWORD)(PrimitiveType != PT_LineList ? NumPrimitives : 0));
	INC_DWORD_STAT_BY(STAT_D3D10Lines,(DWORD)(PrimitiveType == PT_LineList ? NumPrimitives : 0));

	// create a buffer on the fly
	UINT VertexCount = NumVertices;
	UINT IndexCount = GetVertexCountForPrimitiveCount(NumPrimitives,PrimitiveType);
	UINT DrawOffsetVB = 0;
	UINT DrawOffsetIB = 0;
	ID3D10Buffer* VertexBuffer = FillD3D10Buffer(VertexCount,VertexDataStride,VertexData,D3D10_BIND_VERTEX_BUFFER,DrawOffsetVB);
	ID3D10Buffer* IndexBuffer = FillD3D10Buffer(IndexCount,IndexDataStride,IndexData,D3D10_BIND_INDEX_BUFFER,DrawOffsetIB);
	UINT Offset = DrawOffsetVB;

	CommitVertexAndPixelShaderConstants();
	Direct3DDevice->IASetVertexBuffers(0,1,&VertexBuffer,&VertexDataStride,&Offset);
	Direct3DDevice->IASetIndexBuffer(IndexBuffer,
			IndexDataStride == sizeof(WORD) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT,
			DrawOffsetIB);
	Direct3DDevice->IASetPrimitiveTopology(GetD3D10PrimitiveType(PrimitiveType));
	Direct3DDevice->DrawIndexed(IndexCount,0,MinVertexIndex);
}

//
/**
 * Draw a sprite particle emitter.
 *
 * @param Mesh The mesh element containing the data for rendering the sprite particles
 */
void FD3D10DynamicRHI::DrawSpriteParticles( const FMeshElement& Mesh)
{
	check(Mesh.DynamicVertexData);
	FDynamicSpriteEmitterData* SpriteData = (FDynamicSpriteEmitterData*)(Mesh.DynamicVertexData);

	// Sort the particles if required
	INT ParticleCount = SpriteData->GetSource().ActiveParticleCount;

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
	RHIBeginDrawIndexedPrimitiveUP( PT_TriangleList, 
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
void FD3D10DynamicRHI::DrawSubUVParticles( const FMeshElement& Mesh)
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
	RHIBeginDrawIndexedPrimitiveUP( PT_TriangleList, 
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
void FD3D10DynamicRHI::Clear(UBOOL bClearColor,const FLinearColor& Color,UBOOL bClearDepth,FLOAT Depth,UBOOL bClearStencil,DWORD Stencil)
{
	ID3D10RenderTargetView* pRTV = NULL;
	ID3D10DepthStencilView* pDSV = NULL;
	Direct3DDevice->OMGetRenderTargets(1,&pRTV,&pDSV);

	// Determine if we're trying to clear a subrect of the screen
	UBOOL UseDrawClear = FALSE;
	UINT NumViews = 1;
	D3D10_VIEWPORT Viewport;
	Direct3DDevice->RSGetViewports(&NumViews,&Viewport);
	if(Viewport.TopLeftX > 0 || Viewport.TopLeftY > 0)
		UseDrawClear = TRUE;
	if(CurrentScissorEnable)
	{
		UINT NumRects = 0;
		Direct3DDevice->RSGetScissorRects(&NumRects,NULL);
		if(NumRects > 0)
			UseDrawClear = TRUE;
	}

	if(!UseDrawClear)
	{
		UINT Width = 0;
		UINT Height = 0;
		if(pRTV)
		{
			ID3D10Texture2D* BaseTexture = NULL;
			pRTV->GetResource((ID3D10Resource**)&BaseTexture);
			D3D10_TEXTURE2D_DESC Desc;
			BaseTexture->GetDesc(&Desc);
			Width = Desc.Width;
			Height = Desc.Height;
			BaseTexture->Release();
		}
		else if(pDSV)
		{
			ID3D10Texture2D* BaseTexture = NULL;
			pDSV->GetResource((ID3D10Resource**)&BaseTexture);
			D3D10_TEXTURE2D_DESC Desc;
			BaseTexture->GetDesc(&Desc);
			Width = Desc.Width;
			Height = Desc.Height;
			BaseTexture->Release();
		}

		if((Viewport.Width < Width || Viewport.Height < Height) 
			&& (Viewport.Width > 1 && Viewport.Height > 1))
			UseDrawClear = TRUE;
	}

	if(UseDrawClear)
	{
		// Store the old states here
		ID3D10DepthStencilState* OldDepthStencilState = NULL;
		ID3D10RasterizerState* OldRasterizerState = NULL;
		ID3D10BlendState* OldBlendState = NULL;
		UINT StencilRef = 0;
		FLOAT BlendFactor[4];
		UINT SampleMask = 0;
		Direct3DDevice->OMGetDepthStencilState(&OldDepthStencilState,&StencilRef);
		Direct3DDevice->OMGetBlendState(&OldBlendState,BlendFactor,&SampleMask);
		Direct3DDevice->RSGetState(&OldRasterizerState);

		// Set new states
		FBlendStateRHIParamRef BlendStateRHI = TStaticBlendState<>::GetRHI();
		FRasterizerStateRHIParamRef RasterizerStateRHI = TStaticRasterizerState<FM_Solid,CM_None>::GetRHI();
		FLOAT BF[4] = {0,0,0,0};
		UINT8 ColorWriteMask = 0;
		FDepthStateRHIParamRef DepthStateRHI;
		FStencilStateRHIParamRef StencilStateRHI;

		if(bClearColor && pRTV)
		{
			ColorWriteMask = CurrentColorWriteEnable;//D3D10_COLOR_WRITE_ENABLE_ALL;
		}
		else
		{
			ColorWriteMask = 0;
		}

		if(bClearDepth)
		{
			DepthStateRHI = TStaticDepthState<TRUE, CF_Always>::GetRHI();
		}
		else
		{
			DepthStateRHI = TStaticDepthState<FALSE, CF_Always>::GetRHI();
		}

		if(bClearStencil)
		{
			StencilStateRHI = TStaticStencilState<
				TRUE,CF_Always,SO_Replace,SO_Replace,SO_Replace,
				FALSE,CF_Always,SO_Replace,SO_Replace,SO_Replace,
				0xff,0xff,1
				>::GetRHI();

		}
		else
		{
			StencilStateRHI = TStaticStencilState<>::GetRHI();
		}

		DYNAMIC_CAST_D3D10RESOURCE(BlendState,BlendState);
		DYNAMIC_CAST_D3D10RESOURCE(RasterizerState,RasterizerState);
		DYNAMIC_CAST_D3D10RESOURCE(DepthState,DepthState);
		DYNAMIC_CAST_D3D10RESOURCE(StencilState,StencilState);

		// Set the cached state objects
		ID3D10BlendState* CachedBlendState = GetCachedBlendState(BlendState->BlendDesc,ColorWriteMask);
		ID3D10DepthStencilState* CachedDepthStencilState = GetCachedDepthStencilState(DepthState->DepthStencilDesc,StencilState->DepthStencilDesc);
		ID3D10RasterizerState* CachedRasterizerState = GetCachedRasterizerState(RasterizerState->RasterizerDesc,CurrentScissorEnable,bCurrentRenderTargetIsMultisample);
		Direct3DDevice->OMSetBlendState(CachedBlendState,BF,0xFFFFFFFF);
		Direct3DDevice->OMSetDepthStencilState(CachedDepthStencilState,Stencil);
		Direct3DDevice->RSSetState(CachedRasterizerState);

		// Store the old shaders
		ID3D10VertexShader* VSOld = NULL;
		ID3D10PixelShader* PSOld = NULL;
		Direct3DDevice->VSGetShader(&VSOld);
		Direct3DDevice->PSGetShader(&PSOld);

		// Set the new shaders
		TShaderMapRef<FOneColorVertexShader> VertexShader(GetGlobalShaderMap());
		TShaderMapRef<FOneColorPixelShader> PixelShader(GetGlobalShaderMap());
		SetGlobalBoundShaderState(GClearBoundShaderState, GVector4VertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, sizeof(FVector4));
		SetPixelShaderValue(PixelShader->GetPixelShader(),PixelShader->ColorParameter,Color);

		// Draw a fullscreen quad
		FVector4 Vertices[4];
		Vertices[0].Set( -1.0f,  1.0f, Depth, 1.0f );
		Vertices[1].Set(  1.0f,  1.0f, Depth, 1.0f );
		Vertices[2].Set( -1.0f, -1.0f, Depth, 1.0f );
		Vertices[3].Set(  1.0f, -1.0f, Depth, 1.0f );
		RHIDrawPrimitiveUP(PT_TriangleStrip, 2, Vertices, sizeof(Vertices[0]) );

		// Restore the old shaders
		Direct3DDevice->VSSetShader(VSOld);
		Direct3DDevice->PSSetShader(PSOld);
		if(VSOld)
			VSOld->Release();
		if(PSOld)
			PSOld->Release();

		// Restore the old states
		Direct3DDevice->OMSetDepthStencilState(OldDepthStencilState,StencilRef);
		Direct3DDevice->OMSetBlendState(OldBlendState,BlendFactor,SampleMask);
		Direct3DDevice->RSSetState(OldRasterizerState);
		if(OldDepthStencilState)
			OldDepthStencilState->Release();
		if(OldBlendState)
			OldBlendState->Release();
		if(OldRasterizerState)
			OldRasterizerState->Release();

	}
	else
	{
		if(bClearColor && pRTV)
		{
			Direct3DDevice->ClearRenderTargetView(pRTV,(FLOAT*)&Color);
		}
		if((bClearDepth || bClearStencil) && pDSV)
		{
			UINT ClearFlags = 0;
			if(bClearDepth)
				ClearFlags |= D3D10_CLEAR_DEPTH;
			if(bClearStencil)
				ClearFlags |= D3D10_CLEAR_STENCIL;
			Direct3DDevice->ClearDepthStencilView(pDSV,ClearFlags,Depth,Stencil);
		}
	}

	if(pRTV)
		pRTV->Release();
	if(pDSV)
		pDSV->Release();
}

// Functions to yield and regain rendering control from D3D

void FD3D10DynamicRHI::SuspendRendering()
{
	// Not supported
}

void FD3D10DynamicRHI::ResumeRendering()
{
	// Not supported
}

UBOOL FD3D10DynamicRHI::IsRenderingSuspended()
{
	// Not supported
	return FALSE;
}

// Kick the rendering commands that are currently queued up in the GPU command buffer.
void FD3D10DynamicRHI::KickCommandBuffer()
{
	// Not really supported
}

// Blocks the CPU until the GPU catches up and goes idle.
void FD3D10DynamicRHI::BlockUntilGPUIdle()
{
	// Not really supported
}

/*
 * Returns the total GPU time taken to render the last frame. Same metric as appCycles().
 */
DWORD FD3D10DynamicRHI::GetGPUFrameCycles()
{
	//@TODO
	return 0;
}

/*
 * Returns an approximation of the available memory that textures can use, which is video + AGP where applicable, rounded to the nearest MB, in MB.
 */
DWORD FD3D10DynamicRHI::GetAvailableTextureMemory()
{
	//apparently GetAvailableTextureMem() returns available bytes (the docs don't say) rounded to the nearest MB.
	//TODO: Get this through DXGI or WMI channels
	return 512;
	//return Direct3DDevice->GetAvailableTextureMem() / 1048576;
}

// not used on PC

void FD3D10DynamicRHI::MSAAInitPrepass()
{
}
void FD3D10DynamicRHI::MSAAFixViewport()
{
}
void FD3D10DynamicRHI::MSAABeginRendering(UBOOL bRequiresClear)
{
}
void FD3D10DynamicRHI::MSAAEndRendering(FTexture2DRHIParamRef DepthTexture,FTexture2DRHIParamRef ColorTexture,UINT MSAAEndRendering)
{
}
void FD3D10DynamicRHI::RestoreColorDepth(FTexture2DRHIParamRef ColorTexture, FTexture2DRHIParamRef DepthTexture)
{
}
void FD3D10DynamicRHI::GetMSAAOffsets(UINT *HalfScreenY, UINT *ResolveOffset)
{
	check(0);
}
void FD3D10DynamicRHI::SetTessellationMode( ETessellationMode TessellationMode, FLOAT MinTessellation, FLOAT MaxTessellation )
{
}
