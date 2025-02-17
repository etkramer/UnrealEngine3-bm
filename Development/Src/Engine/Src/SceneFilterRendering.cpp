/*=============================================================================
	SceneFilterRendering.cpp: Filter rendering implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "SceneFilterRendering.h"

/** Vertex declaration for the light function fullscreen 2D quad. */
TGlobalResource<FFilterVertexDeclaration> GFilterVertexDeclaration;

/** A macro to declaring a filter shader type for a specific number of samples. */
#define IMPLEMENT_FILTER_SHADER_TYPE(NumSamples) \
	IMPLEMENT_SHADER_TYPE(template<>,TFilterPixelShader<NumSamples>,TEXT("FilterPixelShader"),TEXT("Main"),SF_Pixel,0,0); \
	IMPLEMENT_SHADER_TYPE(template<>,TFilterVertexShader<NumSamples>,TEXT("FilterVertexShader"),TEXT("Main"),SF_Vertex,0,0);

/*
 * The filter shader types for 1-MAX_FILTER_SAMPLES samples.
 */
IMPLEMENT_FILTER_SHADER_TYPE(1);
IMPLEMENT_FILTER_SHADER_TYPE(2);
IMPLEMENT_FILTER_SHADER_TYPE(3);
IMPLEMENT_FILTER_SHADER_TYPE(4);
IMPLEMENT_FILTER_SHADER_TYPE(5);
IMPLEMENT_FILTER_SHADER_TYPE(6);
IMPLEMENT_FILTER_SHADER_TYPE(7);
IMPLEMENT_FILTER_SHADER_TYPE(8);
IMPLEMENT_FILTER_SHADER_TYPE(9);
IMPLEMENT_FILTER_SHADER_TYPE(10);
IMPLEMENT_FILTER_SHADER_TYPE(11);
IMPLEMENT_FILTER_SHADER_TYPE(12);
IMPLEMENT_FILTER_SHADER_TYPE(13);
IMPLEMENT_FILTER_SHADER_TYPE(14);
IMPLEMENT_FILTER_SHADER_TYPE(15);
IMPLEMENT_FILTER_SHADER_TYPE(16);

//
void SetFilterShaders(
	FSamplerStateRHIParamRef SamplerStateRHI,
	FTextureRHIParamRef TextureRHI,
	FVector2D* SampleOffsets,
	FLinearColor* SampleWeights,
	UINT NumSamples
	)
{

	// A macro to handle setting the filter shader for a specific number of samples.
#define SET_FILTER_SHADER_TYPE(NumSamples) \
	case NumSamples: \
	{ \
		TShaderMapRef<TFilterPixelShader<NumSamples> > PixelShader(GetGlobalShaderMap()); \
		TShaderMapRef<TFilterVertexShader<NumSamples> > VertexShader(GetGlobalShaderMap()); \
		VertexShader->SetParameters(SampleOffsets); \
		PixelShader->SetParameters(SamplerStateRHI,TextureRHI,SampleWeights); \
		{ \
			static FGlobalBoundShaderState BoundShaderState; \
			SetGlobalBoundShaderState( BoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, sizeof(FFilterVertex)); \
		} \
		break; \
	};

	// Set the appropriate filter shader for the given number of samples.
	switch(NumSamples)
	{
	SET_FILTER_SHADER_TYPE(1);
	SET_FILTER_SHADER_TYPE(2);
	SET_FILTER_SHADER_TYPE(3);
	SET_FILTER_SHADER_TYPE(4);
	SET_FILTER_SHADER_TYPE(5);
	SET_FILTER_SHADER_TYPE(6);
	SET_FILTER_SHADER_TYPE(7);
	SET_FILTER_SHADER_TYPE(8);
	SET_FILTER_SHADER_TYPE(9);
	SET_FILTER_SHADER_TYPE(10);
	SET_FILTER_SHADER_TYPE(11);
	SET_FILTER_SHADER_TYPE(12);
	SET_FILTER_SHADER_TYPE(13);
	SET_FILTER_SHADER_TYPE(14);
	SET_FILTER_SHADER_TYPE(15);
	SET_FILTER_SHADER_TYPE(16);
	default:
		appErrorf(TEXT("Invalid number of samples: %u"),NumSamples);
	}

#undef SET_FILTER_SHADER_TYPE
}

//
void DrawDenormalizedQuad(
	FLOAT X,
	FLOAT Y,
	FLOAT SizeX,
	FLOAT SizeY,
	FLOAT U,
	FLOAT V,
	FLOAT SizeU,
	FLOAT SizeV,
	UINT TargetSizeX,
	UINT TargetSizeY,
	UINT TextureSizeX,
	UINT TextureSizeY,
	FLOAT ClipSpaceQuadZ
	)
{
	// Set up the vertices.
	FFilterVertex Vertices[4];

	Vertices[0].Position = FVector4(X,			Y,			ClipSpaceQuadZ,	1);
	Vertices[1].Position = FVector4(X + SizeX,	Y,			ClipSpaceQuadZ,	1);
	Vertices[2].Position = FVector4(X,			Y + SizeY,	ClipSpaceQuadZ,	1);
	Vertices[3].Position = FVector4(X + SizeX,	Y + SizeY,	ClipSpaceQuadZ,	1);

	Vertices[0].UV = FVector2D(U,			V);
	Vertices[1].UV = FVector2D(U + SizeU,	V);
	Vertices[2].UV = FVector2D(U,			V + SizeV);
	Vertices[3].UV = FVector2D(U + SizeU,	V + SizeV);

	for(INT VertexIndex = 0;VertexIndex < 4;VertexIndex++)
	{
		Vertices[VertexIndex].Position.X = -1.0f + 2.0f * (Vertices[VertexIndex].Position.X - GPixelCenterOffset) / (FLOAT)TargetSizeX;
		Vertices[VertexIndex].Position.Y = +1.0f - 2.0f * (Vertices[VertexIndex].Position.Y - GPixelCenterOffset) / (FLOAT)TargetSizeY;

		Vertices[VertexIndex].UV.X = Vertices[VertexIndex].UV.X / (FLOAT)TextureSizeX;
		Vertices[VertexIndex].UV.Y = Vertices[VertexIndex].UV.Y / (FLOAT)TextureSizeY;
	}

	static WORD Indices[] =
	{
		0, 1, 3,
		0, 3, 2
	};

	RHIDrawIndexedPrimitiveUP(PT_TriangleList,0,4,2,Indices,sizeof(Indices[0]),Vertices,sizeof(Vertices[0]));
}

//
void DrawDownsampledTexture(const FViewInfo& View)
{
	const UINT BufferSizeX = GSceneRenderTargets.GetBufferSizeX();
	const UINT BufferSizeY = GSceneRenderTargets.GetBufferSizeY();
	const UINT FilterBufferSizeX = GSceneRenderTargets.GetFilterBufferSizeX();
	const UINT FilterBufferSizeY = GSceneRenderTargets.GetFilterBufferSizeY();
	const UINT FilterDownsampleFactor = GSceneRenderTargets.GetFilterDownsampleFactor();
	const UINT DownsampledSizeX = View.RenderTargetSizeX / FilterDownsampleFactor;
	const UINT DownsampledSizeY = View.RenderTargetSizeY / FilterDownsampleFactor;

	// No depth tests, no backface culling.
	RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
	RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
	RHISetBlendState(TStaticBlendState<>::GetRHI());

	// Setup the downsamples.
	FVector2D DownsampleOffsets[MAX_FILTER_SAMPLES];
	FLinearColor DownsampleWeights[MAX_FILTER_SAMPLES];
	for(UINT SampleY = 0;SampleY < FilterDownsampleFactor;SampleY++)
	{
		for(UINT SampleX = 0;SampleX < FilterDownsampleFactor;SampleX++)
		{
			const INT SampleIndex = SampleY * FilterDownsampleFactor + SampleX;
			DownsampleOffsets[SampleIndex] = FVector2D(
				(FLOAT)SampleX / (FLOAT)GSceneRenderTargets.GetBufferSizeX(),
				(FLOAT)SampleY / (FLOAT)GSceneRenderTargets.GetBufferSizeY()
				);
			DownsampleWeights[SampleIndex] = FLinearColor::White / Square(FilterDownsampleFactor);
		}
	}

	// Downsample the scene color buffer.
	GSceneRenderTargets.BeginRenderingFilter();

//@TODO - We shouldn't have to clear since we fill the entire buffer anyway. :(
#if !PS3
	RHIClear(TRUE,FLinearColor(0,0,0,0),FALSE,0,FALSE,0);
#endif

	SetFilterShaders(
		TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
		GSceneRenderTargets.GetSceneColorTexture(),
		DownsampleOffsets,
		DownsampleWeights,
		FilterDownsampleFactor * FilterDownsampleFactor
		);
	DrawDenormalizedQuad(
		1,1,DownsampledSizeX,DownsampledSizeY,
		View.RenderTargetX,View.RenderTargetY,
		View.RenderTargetSizeX,View.RenderTargetSizeY,
		FilterBufferSizeX,FilterBufferSizeY,
		BufferSizeX,BufferSizeY
		);
	GSceneRenderTargets.FinishRenderingFilter();
}

/**
 * Evaluates a normal distribution PDF at given X.
 * @param X - The X to evaluate the PDF at.
 * @param Mean - The normal distribution's mean.
 * @param Variance - The normal distribution's variance.
 * @return The value of the normal distribution at X.
 */
static FLOAT NormalDistribution(FLOAT X,FLOAT Mean,FLOAT Variance)
{
	const FLOAT StandardDeviation = appSqrt(Variance);
	return appExp(-Square(X - Mean) / (2.0f * Variance)) / (StandardDeviation * appSqrt(2.0f * (FLOAT)PI));
}

/**
 * Blurs the subregion of the filter buffer along a single axis.
 * @param SizeX - The width of the subregion to blur.
 * @param SizeY - The height of the subregion to blur.
 */
static void ApplyGaussianBlurStep(UINT SizeX,UINT SizeY,FVector2D Direction,FLOAT KernelRadius)
{
	const UINT BufferSizeX = GSceneRenderTargets.GetFilterBufferSizeX();
	const UINT BufferSizeY = GSceneRenderTargets.GetFilterBufferSizeY();

	FLOAT ClampedKernelRadius = Clamp<FLOAT>(KernelRadius,DELTA,MAX_FILTER_SAMPLES - 1);
	INT IntegerKernelRadius = Min<INT>(appCeil(ClampedKernelRadius),MAX_FILTER_SAMPLES - 1);

	FVector2D BlurOffsets[MAX_FILTER_SAMPLES];
	FLinearColor BlurWeights[MAX_FILTER_SAMPLES];
	UINT NumSamples;

	NumSamples = 0;
	FLOAT WeightSum = 0.0f;
	for(INT SampleIndex = -IntegerKernelRadius;SampleIndex <= IntegerKernelRadius;SampleIndex += 2)
	{
		FLOAT Weight0 = NormalDistribution(SampleIndex,0,ClampedKernelRadius);
		FLOAT Weight1 = NormalDistribution(SampleIndex + 1,0,ClampedKernelRadius);
		FLOAT TotalWeight = Weight0 + Weight1;
		BlurOffsets[NumSamples] = Direction * (SampleIndex + Weight1 / TotalWeight);
		BlurWeights[NumSamples] = FLinearColor::White * TotalWeight;
		WeightSum += TotalWeight;
		NumSamples++;
	}

	// Normalize blur weights.
	for(UINT SampleIndex = 0;SampleIndex < NumSamples;SampleIndex++)
	{
		BlurWeights[SampleIndex] /= WeightSum;
	}

	GSceneRenderTargets.BeginRenderingFilter();
	SetFilterShaders(
		TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
		GSceneRenderTargets.GetFilterColorTexture(),
		BlurOffsets,
		BlurWeights,
		NumSamples
		);
	DrawDenormalizedQuad(
		1,1,
		SizeX,SizeY,
		1,1,
		SizeX,SizeY,
		BufferSizeX,BufferSizeY,
		BufferSizeX,BufferSizeY
		);
	GSceneRenderTargets.FinishRenderingFilter();
}

/**
* Uses a gaussian blur to filter a subregion of the filter buffer.
* @param SizeX - The width of the subregion to blur.
* @param SizeY - The height of the subregion to blur.
* @param KernelRadius - The radius of the gaussian kernel.
*/
void GaussianBlurFilterBuffer(UINT SizeX,UINT SizeY,FLOAT KernelRadius)
{
	const UINT BufferSizeX = GSceneRenderTargets.GetFilterBufferSizeX();
	const UINT BufferSizeY = GSceneRenderTargets.GetFilterBufferSizeY();
	const UINT FilterDownsampleFactor = GSceneRenderTargets.GetFilterDownsampleFactor();

	// Rescale the blur kernel to be resolution independent.
	// The effect blur scale is specified in pixel widths at a reference resolution of 1280x?.
	const FLOAT ResolutionFraction = SizeX * FilterDownsampleFactor / 1280.0f;
	const FLOAT KernelRadiusX = Min<FLOAT>(KernelRadius / FilterDownsampleFactor,MAX_FILTER_SAMPLES) * ResolutionFraction;
	const FLOAT KernelRadiusY = Min<FLOAT>(KernelRadius / FilterDownsampleFactor,MAX_FILTER_SAMPLES) * ResolutionFraction;

	ApplyGaussianBlurStep( SizeX,SizeY,FVector2D(1.0f / BufferSizeX,0),KernelRadiusX);
	ApplyGaussianBlurStep( SizeX,SizeY,FVector2D(0,1.0f / BufferSizeY),KernelRadiusY);
}

