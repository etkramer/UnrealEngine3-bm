/*=============================================================================
	D3D10State.h: D3D state definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

class FD3D10SamplerState : public FRefCountedObject, public TDynamicRHIResource<RRT_SamplerState>
{
public:
	D3D10_SAMPLER_DESC SamplerDesc;
};

class FD3D10RasterizerState : public FRefCountedObject, public TDynamicRHIResource<RRT_RasterizerState>
{
public:

	D3D10_RASTERIZER_DESC RasterizerDesc;
};

class FD3D10DepthState : public FRefCountedObject, public TDynamicRHIResource<RRT_DepthState>
{
public:

	D3D10_DEPTH_STENCIL_DESC DepthStencilDesc;
};

class FD3D10StencilState : public FRefCountedObject, public TDynamicRHIResource<RRT_StencilState>
{
public:

	D3D10_DEPTH_STENCIL_DESC DepthStencilDesc;
	UINT StencilRef;
};

class FD3D10BlendState : public FRefCountedObject, public TDynamicRHIResource<RRT_BlendState>
{
public:

	D3D10_BLEND_DESC BlendDesc;
	FLinearColor BlendFactor;

	FD3D10BlendState():
		FRefCountedObject(),
		BlendFactor(0,0,0,0)
	{
	}
	~FD3D10BlendState()
	{
	}
};

void ReleaseCachedD3D10States();
