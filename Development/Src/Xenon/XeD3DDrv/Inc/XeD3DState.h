/*=============================================================================
	XeD3DState.h: XeD3D state RHI definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#if USE_XeD3D_RHI

/** Texture sampler state */
class FD3DSamplerState : public FXeResource
{
public:
	D3DTEXTUREFILTERTYPE MagFilter;
	D3DTEXTUREFILTERTYPE MinFilter;
	D3DTEXTUREFILTERTYPE MipFilter;
	D3DTEXTUREADDRESS AddressU;
	D3DTEXTUREADDRESS AddressV;
	D3DTEXTUREADDRESS AddressW;
};

/** triangle rasterization state */
class FD3DRasterizerState : public FXeResource
{
public:
	D3DFILLMODE FillMode;
	D3DCULL CullMode;
	FLOAT DepthBias;
	FLOAT SlopeScaleDepthBias;
};

/** depth surface state */
class FD3DDepthState : public FXeResource
{
public:
	UBOOL bZEnable;
	UBOOL bZWriteEnable;
	D3DCMPFUNC ZFunc;
};

/** stencil surface state */
class FD3DStencilState : public FXeResource
{
public:
	UBOOL bStencilEnable;
	UBOOL bTwoSidedStencilMode;
	D3DCMPFUNC StencilFunc;
	D3DSTENCILOP StencilFail;
	D3DSTENCILOP StencilZFail;
	D3DSTENCILOP StencilPass;
	D3DCMPFUNC CCWStencilFunc;
	D3DSTENCILOP CCWStencilFail;
	D3DSTENCILOP CCWStencilZFail;
	D3DSTENCILOP CCWStencilPass;
	DWORD StencilReadMask;
	DWORD StencilWriteMask;
	DWORD StencilRef;
};

/** target surface blend state */
class FD3DBlendState : public FXeResource
{
public:
	UBOOL bAlphaBlendEnable;
	D3DBLENDOP ColorBlendOperation;
	D3DBLEND ColorSourceBlendFactor;
	D3DBLEND ColorDestBlendFactor;
	UBOOL bSeparateAlphaBlendEnable;
	D3DBLENDOP AlphaBlendOperation;
	D3DBLEND AlphaSourceBlendFactor;
	D3DBLEND AlphaDestBlendFactor;
	UBOOL bAlphaTestEnable;
	D3DCMPFUNC AlphaFunc;
	DWORD AlphaRef;
};

/*-----------------------------------------------------------------------------
RHI state types
-----------------------------------------------------------------------------*/
typedef TRefCountPtr<FD3DSamplerState> FSamplerStateRHIRef;
typedef FD3DSamplerState* FSamplerStateRHIParamRef;

typedef TRefCountPtr<FD3DRasterizerState> FRasterizerStateRHIRef;
typedef FD3DRasterizerState* FRasterizerStateRHIParamRef;

typedef TRefCountPtr<FD3DDepthState> FDepthStateRHIRef;
typedef FD3DDepthState* FDepthStateRHIParamRef;

typedef TRefCountPtr<FD3DStencilState> FStencilStateRHIRef;
typedef FD3DStencilState* FStencilStateRHIParamRef;

typedef TRefCountPtr<FD3DBlendState> FBlendStateRHIRef;
typedef FD3DBlendState* FBlendStateRHIParamRef;

#endif
