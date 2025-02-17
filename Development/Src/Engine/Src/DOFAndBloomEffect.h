/*=============================================================================
	DOFAndBloomEffect.cpp: Combined depth of field and bloom effect implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _INC_DOFANDBLOOMEFFECT
#define _INC_DOFANDBLOOMEFFECT

/** Encapsulates the DOF parameters which are used by multiple shader types. */
class FDOFShaderParameters
{
public:

	/** Default constructor. */
	FDOFShaderParameters() {}

	/** Initialization constructor. */
	FDOFShaderParameters(const FShaderParameterMap& ParameterMap);

	/** Set the dof shader parameter values. */
	void Set(FShader* PixelShader,FLOAT FocusDistance,FLOAT FocusInnerRadius,FLOAT FalloffExponent,FLOAT MaxNearBlurAmount,FLOAT MaxFarBlurAmount);

	/** Serializer. */
	friend FArchive& operator<<(FArchive& Ar,FDOFShaderParameters& P);

private:
	FShaderParameter PackedParameters;
	FShaderParameter MinMaxBlurClampParameter;
};

/*-----------------------------------------------------------------------------
TDOFAndBloomGatherPixelShader - Encapsulates the DOF and bloom gather pixel shader.
-----------------------------------------------------------------------------*/
template<UINT NumSamples> 
class TDOFAndBloomGatherPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(TDOFAndBloomGatherPixelShader,Global);

public:
	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		//don't compile for sm2, since these use too many ALU instructions
		return Platform != SP_PCD3D_SM2;
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("NUM_SAMPLES"),*FString::Printf(TEXT("%u"),NumSamples));

		//tell the compiler to unroll
		new(OutEnvironment.CompilerFlags) ECompilerFlags(CFLAG_AvoidFlowControl);
	}

	/** Default constructor. */
	TDOFAndBloomGatherPixelShader() {}

	/** Initialization constructor. */
	TDOFAndBloomGatherPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FGlobalShader(Initializer)
		,	DOFParameters(Initializer.ParameterMap)
	{
		SceneTextureParameters.Bind(Initializer.ParameterMap);
		BloomScaleParameter.Bind(Initializer.ParameterMap,TEXT("BloomScale"),TRUE);
		SceneMultiplierParameter.Bind(Initializer.ParameterMap,TEXT("SceneMultiplier"),TRUE);
		SmallSceneColorTextureParameter.Bind(Initializer.ParameterMap,TEXT("SmallSceneColorTexture"),TRUE);
	}

	// FShader interface.
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << DOFParameters << SceneTextureParameters << BloomScaleParameter << SceneMultiplierParameter << SmallSceneColorTextureParameter;
		return bShaderHasOutdatedParameters;
	}

	FDOFShaderParameters DOFParameters;
	FSceneTextureShaderParameters SceneTextureParameters;
	FShaderParameter BloomScaleParameter;
	FShaderParameter SceneMultiplierParameter;
	FShaderResourceParameter SmallSceneColorTextureParameter;
};


/*-----------------------------------------------------------------------------
FDOFAndBloomGatherFallbackPixelShader
-----------------------------------------------------------------------------*/
class FDOFAndBloomGatherFallbackPixelShader : public TDOFAndBloomGatherPixelShader<4>
{
	DECLARE_SHADER_TYPE(FDOFAndBloomGatherFallbackPixelShader,Global);

public:
	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		//only compile for sm2, the other platforms use the TDOFAndBloomGatherPixelShader
		return Platform == SP_PCD3D_SM2;
	}

	/** Default constructor. */
	FDOFAndBloomGatherFallbackPixelShader() {}

	/** Initialization constructor. */
	FDOFAndBloomGatherFallbackPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	TDOFAndBloomGatherPixelShader<4>(Initializer)
	{}
};

/*-----------------------------------------------------------------------------
FDOFAndBloomGatherVertexShader - Encapsulates the DOF and bloom gather vertex shader.
-----------------------------------------------------------------------------*/
template<UINT NumSamples> 
class TDOFAndBloomGatherVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(TDOFAndBloomGatherVertexShader,Global);

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
	TDOFAndBloomGatherVertexShader() {}

	/** Initialization constructor. */
	TDOFAndBloomGatherVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FGlobalShader(Initializer)
	{
		SampleOffsetsParameter.Bind(Initializer.ParameterMap,TEXT("SampleOffsets"),TRUE);
	}

	// FShader interface.
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << SampleOffsetsParameter;
		return bShaderHasOutdatedParameters;
	}

	FShaderParameter SampleOffsetsParameter;
};

/** Encapsulates the DOF and bloom blend pixel shader. */
class FDOFAndBloomBlendPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FDOFAndBloomBlendPixelShader,Global);

	static UBOOL ShouldCache(EShaderPlatform Platform);

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment);

	/** Default constructor. */
	FDOFAndBloomBlendPixelShader() {}

public:

	FDOFShaderParameters DOFParameters;
	FSceneTextureShaderParameters SceneTextureParameters;
	FShaderResourceParameter BlurredImageParameter;

	/** Initialization constructor. */
	FDOFAndBloomBlendPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer);

	// FShader interface.
	virtual UBOOL Serialize(FArchive& Ar);
};

/** Encapsulates the DOF and bloom blend pixel shader. */
class FDOFAndBloomBlendVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FDOFAndBloomBlendVertexShader,Global);

	static UBOOL ShouldCache(EShaderPlatform Platform);
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment);

	/** Default constructor. */
	FDOFAndBloomBlendVertexShader() {}

public:

	FShaderParameter SceneCoordinateScaleBiasParameter;

	/** Initialization constructor. */
	FDOFAndBloomBlendVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer);

	// FShader interface.
	virtual UBOOL Serialize(FArchive& Ar);
};

/*-----------------------------------------------------------------------------
FDOFAndBloomPostProcessSceneProxy
-----------------------------------------------------------------------------*/

class FDOFAndBloomPostProcessSceneProxy : public FPostProcessSceneProxy
{
public:
	/** 
	* Initialization constructor. 
	* @param InEffect - DOF post process effect to mirror in this proxy
	*/
	FDOFAndBloomPostProcessSceneProxy(const UDOFAndBloomEffect* InEffect,const FPostProcessSettings* WorldSettings);

	/**
	* Render the post process effect
	* Called by the rendering thread during scene rendering
	* @param InDepthPriorityGroup - scene DPG currently being rendered
	* @param View - current view
	* @param CanvasTransform - same canvas transform used to render the scene
	* @return TRUE if anything was rendered
	*/
	UBOOL Render(const FScene* Scene, UINT InDepthPriorityGroup,FViewInfo& View,const FMatrix& CanvasTransform);

protected:

	/**
	* Renders the gather pass which downsamples scene color and depth, then uses those to 
	* calculate bloom color and unfocused DOF color which are stored in the filter buffer.
	*/
	void RenderDOFAndBloomGatherPass(FViewInfo& View);

	/** mirrored properties. See DOFEffect.uc for descriptions */
	FLOAT FalloffExponent;
	FLOAT BlurKernelSize;
	FLOAT MaxNearBlurAmount;
	FLOAT MaxFarBlurAmount;
	FColor ModulateBlurColor;
	BYTE FocusType;
	FLOAT FocusInnerRadius;
	FLOAT FocusDistance;
	FVector FocusPosition;
	FLOAT BloomScale;
	
	/** bound shader state for the high quality gather pass version */
	static FGlobalBoundShaderState HighQualityGatherBoundShaderState;
	/** bound shader state for the fast gather pass version */
	static FGlobalBoundShaderState FastGatherBoundShaderState;
	/** bound shader state for the blend pass */
	static FGlobalBoundShaderState BlendBoundShaderState;
};

#endif
