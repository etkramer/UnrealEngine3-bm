/*=============================================================================
	SceneFilterRendering.h: Filter rendering definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _INC_SCENEFILTERRENDERING
#define _INC_SCENEFILTERRENDERING

#define MAX_FILTER_SAMPLES	16

/** A pixel shader which filters a texture. */
template<UINT NumSamples>
class TFilterPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(TFilterPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("NUM_SAMPLES"),*FString::Printf(TEXT("%u"),NumSamples));
	}

	/** Default constructor. */
	TFilterPixelShader() {}

	/** Initialization constructor. */
	TFilterPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
	FGlobalShader(Initializer)
	{
		FilterTextureParameter.Bind(Initializer.ParameterMap,TEXT("FilterTexture"),TRUE);
		SampleWeightsParameter.Bind(Initializer.ParameterMap,TEXT("SampleWeights"),TRUE);
	}

	/** Serializer */
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << FilterTextureParameter;
		Ar << SampleWeightsParameter;
		return bShaderHasOutdatedParameters;
	}

	/** Sets shader parameter values */
	void SetParameters(FSamplerStateRHIParamRef SamplerStateRHI,FTextureRHIParamRef TextureRHI,const FLinearColor* SampleWeights)
	{
		SetTextureParameter(GetPixelShader(),FilterTextureParameter,SamplerStateRHI,TextureRHI);
		SetPixelShaderValues(GetPixelShader(),SampleWeightsParameter,SampleWeights,NumSamples);
	}

private:
	FShaderResourceParameter FilterTextureParameter;
	FShaderParameter SampleWeightsParameter;
};

/** A vertex shader which filters a texture. */
template<UINT NumSamples>
class TFilterVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(TFilterVertexShader,Global);
public:

	/** The number of 4D constant registers used to hold the packed 2D sample offsets. */
	enum { NumSampleChunks = (NumSamples + 1) / 2 };

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("NUM_SAMPLES"),*FString::Printf(TEXT("%u"),NumSamples));
	}

	/** Default constructor. */
	TFilterVertexShader() {}

	/** Initialization constructor. */
	TFilterVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
	FGlobalShader(Initializer)
	{
		SampleOffsetsParameter.Bind(Initializer.ParameterMap,TEXT("SampleOffsets"));
	}

	/** Serializer */
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << SampleOffsetsParameter;
		return bShaderHasOutdatedParameters;
	}

	/** Sets shader parameter values */
	void SetParameters(const FVector2D* SampleOffsets)
	{
		FVector4 PackedSampleOffsets[NumSampleChunks];
		for(INT SampleIndex = 0;SampleIndex < NumSamples;SampleIndex += 2)
		{
			PackedSampleOffsets[SampleIndex / 2].X = SampleOffsets[SampleIndex + 0].X;
			PackedSampleOffsets[SampleIndex / 2].Y = SampleOffsets[SampleIndex + 0].Y;
			if(SampleIndex + 1 < NumSamples)
			{
				PackedSampleOffsets[SampleIndex / 2].W = SampleOffsets[SampleIndex + 1].X;
				PackedSampleOffsets[SampleIndex / 2].Z = SampleOffsets[SampleIndex + 1].Y;
			}
		}
		SetVertexShaderValues(GetVertexShader(),SampleOffsetsParameter,PackedSampleOffsets,NumSampleChunks);
	}

private:
	FShaderParameter SampleOffsetsParameter;
};

/**
 * Sets the filter shaders with the provided filter samples.
 * @param SamplerStateRHI - The sampler state to use for the source texture.
 * @param TextureRHI - The source texture.
 * @param SampleOffsets - A pointer to an array of NumSamples UV offsets
 * @param SampleOffsets - A pointer to an array of NumSamples 4-vector weights
 * @param NumSamples - The number of samples used by the filter.
 */
extern void SetFilterShaders(
	FSamplerStateRHIParamRef SamplerStateRHI,
	FTextureRHIParamRef TextureRHI,
	FVector2D* SampleOffsets,
	FLinearColor* SampleWeights,
	UINT NumSamples
	);

/** The vertex data used to filter a texture. */
struct FFilterVertex
{
	FVector4 Position;
	FVector2D UV;
};

/** The filter vertex declaration resource type. */
class FFilterVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	/** Destructor. */
	virtual ~FFilterVertexDeclaration() {}

	virtual void InitRHI()
	{
		FVertexDeclarationElementList Elements;
		Elements.AddItem(FVertexElement(0,STRUCT_OFFSET(FFilterVertex,Position),VET_Float4,VEU_Position,0));
		Elements.AddItem(FVertexElement(0,STRUCT_OFFSET(FFilterVertex,UV),VET_Float2,VEU_TextureCoordinate,0));
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI()
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

/**
 * Draws a quad with the given vertex positions and UVs in denormalized pixel/texel coordinates.
 */
extern void DrawDenormalizedQuad(
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
	FLOAT ClipSpaceQuadZ = 0.0f
	);

/**
 * Downsamples the given view into the filter buffer.
 */
extern void DrawDownsampledTexture(const FViewInfo& View);

/**
 * Uses a gaussian blur to filter a subregion of the filter buffer.
 * @param SizeX - The width of the subregion to blur.
 * @param SizeY - The height of the subregion to blur.
 * @param KernelRadius - The radius of the gaussian kernel.
 */
extern void GaussianBlurFilterBuffer(UINT SizeX,UINT SizeY,FLOAT KernelRadius);

#endif
