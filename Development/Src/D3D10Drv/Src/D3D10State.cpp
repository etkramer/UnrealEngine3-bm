/*=============================================================================
	D3D10State.cpp: D3D state implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3D10DrvPrivate.h"

static D3D10_TEXTURE_ADDRESS_MODE TranslateAddressMode(ESamplerAddressMode AddressMode)
{
	switch(AddressMode)
	{
	case AM_Clamp: return D3D10_TEXTURE_ADDRESS_CLAMP;
	case AM_Mirror: return D3D10_TEXTURE_ADDRESS_MIRROR;
	default: return D3D10_TEXTURE_ADDRESS_WRAP;
	};
}

static DWORD TranslateMipBias(ESamplerMipMapLODBias MipBias)
{
	return 0;
}

static D3D10_CULL_MODE TranslateCullMode(ERasterizerCullMode CullMode)
{
	switch(CullMode)
	{
	case CM_CW: return D3D10_CULL_BACK;
	case CM_CCW: return D3D10_CULL_FRONT;
	default: return D3D10_CULL_NONE;
	};
}

static D3D10_FILL_MODE TranslateFillMode(ERasterizerFillMode FillMode)
{
	switch(FillMode)
	{
	case FM_Wireframe: return D3D10_FILL_WIREFRAME;
	default: return D3D10_FILL_SOLID;
	};
}

static D3D10_COMPARISON_FUNC TranslateCompareFunction(ECompareFunction CompareFunction)
{
	switch(CompareFunction)
	{
	case CF_Less: return D3D10_COMPARISON_LESS;
	case CF_LessEqual: return D3D10_COMPARISON_LESS_EQUAL;
	case CF_Greater: return D3D10_COMPARISON_GREATER;
	case CF_GreaterEqual: return D3D10_COMPARISON_GREATER_EQUAL;
	case CF_Equal: return D3D10_COMPARISON_EQUAL;
	case CF_NotEqual: return D3D10_COMPARISON_NOT_EQUAL;
	case CF_Never: return D3D10_COMPARISON_NEVER;
	default: return D3D10_COMPARISON_ALWAYS;
	};
}

static D3D10_STENCIL_OP TranslateStencilOp(EStencilOp StencilOp)
{
	switch(StencilOp)
	{
	case SO_Zero: return D3D10_STENCIL_OP_ZERO;
	case SO_Replace: return D3D10_STENCIL_OP_REPLACE;
	case SO_SaturatedIncrement: return D3D10_STENCIL_OP_INCR_SAT;
	case SO_SaturatedDecrement: return D3D10_STENCIL_OP_DECR_SAT;
	case SO_Invert: return D3D10_STENCIL_OP_INVERT;
	case SO_Increment: return D3D10_STENCIL_OP_INCR;
	case SO_Decrement: return D3D10_STENCIL_OP_DECR;
	default: return D3D10_STENCIL_OP_KEEP;
	};
}

static D3D10_BLEND_OP TranslateBlendOp(EBlendOperation BlendOp)
{
	switch(BlendOp)
	{
	case BO_Subtract: return D3D10_BLEND_OP_SUBTRACT;
	case BO_Min: return D3D10_BLEND_OP_MIN;
	case BO_Max: return D3D10_BLEND_OP_MAX;
	default: return D3D10_BLEND_OP_ADD;
	};
}

static D3D10_BLEND TranslateBlendFactor(EBlendFactor BlendFactor)
{
	switch(BlendFactor)
	{
	case BF_One: return D3D10_BLEND_ONE;
	case BF_SourceColor: return D3D10_BLEND_SRC_COLOR;
	case BF_InverseSourceColor: return D3D10_BLEND_INV_SRC_COLOR;
	case BF_SourceAlpha: return D3D10_BLEND_SRC_ALPHA;
	case BF_InverseSourceAlpha: return D3D10_BLEND_INV_SRC_ALPHA;
	case BF_DestAlpha: return D3D10_BLEND_DEST_ALPHA;
	case BF_InverseDestAlpha: return D3D10_BLEND_INV_DEST_ALPHA;
	case BF_DestColor: return D3D10_BLEND_DEST_COLOR;
	case BF_InverseDestColor: return D3D10_BLEND_INV_DEST_COLOR;
	default: return D3D10_BLEND_ZERO;
	};
}

FSamplerStateRHIRef FD3D10DynamicRHI::CreateSamplerState(const FSamplerStateInitializerRHI& Initializer)
{
	FD3D10SamplerState* SamplerState = new FD3D10SamplerState;
	appMemzero(&SamplerState->SamplerDesc,sizeof(D3D10_SAMPLER_DESC));

	SamplerState->SamplerDesc.AddressU = TranslateAddressMode(Initializer.AddressU);
	SamplerState->SamplerDesc.AddressV = TranslateAddressMode(Initializer.AddressV);
	SamplerState->SamplerDesc.AddressW = TranslateAddressMode(Initializer.AddressW);
	SamplerState->SamplerDesc.MipLODBias = TranslateMipBias(Initializer.MipBias);
	SamplerState->SamplerDesc.MaxAnisotropy = 4;
	SamplerState->SamplerDesc.MaxLOD = D3D10_FLOAT32_MAX;
	switch(Initializer.Filter)
	{
	case SF_AnisotropicLinear:
	case SF_AnisotropicPoint:
		// D3D10 doesn't allow using point filtering for mip filter when using anisotropic filtering
		SamplerState->SamplerDesc.Filter = D3D10_FILTER_ANISOTROPIC;
		break;
	case SF_Trilinear:
        SamplerState->SamplerDesc.Filter = D3D10_FILTER_MIN_MAG_MIP_LINEAR;
		break;
	case SF_Bilinear:
		SamplerState->SamplerDesc.Filter = D3D10_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	case SF_Point:
		SamplerState->SamplerDesc.Filter = D3D10_FILTER_MIN_MAG_MIP_POINT;
		break;
	}
	return SamplerState;
}

FRasterizerStateRHIRef FD3D10DynamicRHI::CreateRasterizerState(const FRasterizerStateInitializerRHI& Initializer)
{
    FD3D10RasterizerState* RasterizerState = new FD3D10RasterizerState;
	appMemzero(&RasterizerState->RasterizerDesc,sizeof(D3D10_RASTERIZER_DESC));

	RasterizerState->RasterizerDesc.CullMode = TranslateCullMode(Initializer.CullMode);
	RasterizerState->RasterizerDesc.FillMode = TranslateFillMode(Initializer.FillMode);
	RasterizerState->RasterizerDesc.SlopeScaledDepthBias = Initializer.SlopeScaleDepthBias;
	RasterizerState->RasterizerDesc.FrontCounterClockwise = TRUE;
	RasterizerState->RasterizerDesc.DepthBias = appFloor(Initializer.DepthBias * (FLOAT)(1 << 24));

	// MultiSampleEnable and ScissorEnable are set based on context when the rasterizer state is bound.

	return RasterizerState;
}

/**
* Sets a rasterizer state using direct values instead of a cached object.  
* This is preferable to calling RHISetRasterizerState(Context, RHICreateRasterizerState(Initializer)) 
* since at least some hardware platforms can optimize this path.
*/
void FD3D10DynamicRHI::SetRasterizerStateImmediate(const FRasterizerStateInitializerRHI &ImmediateState)
{
	// TODO: determine the frequency of this call
	FRasterizerStateRHIRef RasterizerState = CreateRasterizerState(ImmediateState);
	SetRasterizerState(RasterizerState);
}

FDepthStateRHIRef FD3D10DynamicRHI::CreateDepthState(const FDepthStateInitializerRHI& Initializer)
{
	FD3D10DepthState* DepthState = new FD3D10DepthState;
	appMemzero(&DepthState->DepthStencilDesc,sizeof(D3D10_DEPTH_STENCIL_DESC));

	// depth part
	DepthState->DepthStencilDesc.DepthEnable = Initializer.DepthTest != CF_Always || Initializer.bEnableDepthWrite;
	DepthState->DepthStencilDesc.DepthWriteMask = Initializer.bEnableDepthWrite ? D3D10_DEPTH_WRITE_MASK_ALL : D3D10_DEPTH_WRITE_MASK_ZERO;
	DepthState->DepthStencilDesc.DepthFunc = TranslateCompareFunction(Initializer.DepthTest);
	
	return DepthState;
}

FStencilStateRHIRef FD3D10DynamicRHI::CreateStencilState(const FStencilStateInitializerRHI& Initializer)
{
	FD3D10StencilState* StencilState = new FD3D10StencilState;
	appMemzero(&StencilState->DepthStencilDesc,sizeof(D3D10_DEPTH_STENCIL_DESC));

	// stencil part
	StencilState->DepthStencilDesc.StencilEnable = Initializer.bEnableFrontFaceStencil || Initializer.bEnableBackFaceStencil;
	StencilState->DepthStencilDesc.StencilReadMask = Initializer.StencilReadMask;
	StencilState->DepthStencilDesc.StencilWriteMask = Initializer.StencilWriteMask;
	StencilState->DepthStencilDesc.FrontFace.StencilFunc = TranslateCompareFunction(Initializer.FrontFaceStencilTest);
	StencilState->DepthStencilDesc.FrontFace.StencilFailOp = TranslateStencilOp(Initializer.FrontFaceStencilFailStencilOp);
	StencilState->DepthStencilDesc.FrontFace.StencilDepthFailOp = TranslateStencilOp(Initializer.FrontFaceDepthFailStencilOp);
	StencilState->DepthStencilDesc.FrontFace.StencilPassOp = TranslateStencilOp(Initializer.FrontFacePassStencilOp);
	if( Initializer.bEnableBackFaceStencil )
	{
		StencilState->DepthStencilDesc.BackFace.StencilFunc = TranslateCompareFunction(Initializer.BackFaceStencilTest);
		StencilState->DepthStencilDesc.BackFace.StencilFailOp = TranslateStencilOp(Initializer.BackFaceStencilFailStencilOp);
		StencilState->DepthStencilDesc.BackFace.StencilDepthFailOp = TranslateStencilOp(Initializer.BackFaceDepthFailStencilOp);
		StencilState->DepthStencilDesc.BackFace.StencilPassOp = TranslateStencilOp(Initializer.BackFacePassStencilOp);
	}
	else
	{
		StencilState->DepthStencilDesc.BackFace = StencilState->DepthStencilDesc.FrontFace;
	}
	StencilState->StencilRef = Initializer.StencilRef;
	return StencilState;
}


FBlendStateRHIRef FD3D10DynamicRHI::CreateBlendState(const FBlendStateInitializerRHI& Initializer)
{
	FD3D10BlendState* BlendState = new FD3D10BlendState;
	appMemzero(&BlendState->BlendDesc,sizeof(D3D10_BLEND_DESC));

	BlendState->BlendDesc.AlphaToCoverageEnable = FALSE;
	BlendState->BlendDesc.BlendEnable[0] = 
		Initializer.ColorBlendOperation != BO_Add || Initializer.ColorDestBlendFactor != BF_Zero || Initializer.ColorSourceBlendFactor != BF_One ||
		Initializer.AlphaBlendOperation != BO_Add || Initializer.AlphaDestBlendFactor != BF_Zero || Initializer.AlphaSourceBlendFactor != BF_One;
	BlendState->BlendDesc.BlendOp = TranslateBlendOp(Initializer.ColorBlendOperation);
	BlendState->BlendDesc.SrcBlend = TranslateBlendFactor(Initializer.ColorSourceBlendFactor);
	BlendState->BlendDesc.DestBlend = TranslateBlendFactor(Initializer.ColorDestBlendFactor);
	if( Initializer.AlphaDestBlendFactor != Initializer.ColorDestBlendFactor ||
		Initializer.AlphaSourceBlendFactor != Initializer.ColorSourceBlendFactor )
	{
		BlendState->BlendDesc.BlendOpAlpha = TranslateBlendOp(Initializer.AlphaBlendOperation);
		BlendState->BlendDesc.SrcBlendAlpha = TranslateBlendFactor(Initializer.AlphaSourceBlendFactor);
		BlendState->BlendDesc.DestBlendAlpha = TranslateBlendFactor(Initializer.AlphaDestBlendFactor);
	}
	else
	{
		BlendState->BlendDesc.BlendOpAlpha = D3D10_BLEND_OP_ADD;
		BlendState->BlendDesc.SrcBlendAlpha = D3D10_BLEND_ZERO;
		BlendState->BlendDesc.DestBlendAlpha = D3D10_BLEND_ONE;
	}
	// TODO: Handle alpha test!!!
	// Initializer.AlphaTest != CF_Always;
	// TranslateCompareFunction(Initializer.AlphaTest)
	// Initializer.AlphaRef
	return BlendState;
}
