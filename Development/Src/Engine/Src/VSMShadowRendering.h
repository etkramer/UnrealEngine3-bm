/*=============================================================================
	VSMShadowRendering.h: VSM (Variance Shadow Map) rendering definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#if SUPPORTS_VSM

/**
* A pixel shader for projecting a shadow variance buffer onto the scene.
*/
class FVSMProjectionPixelShader : public FShader
{
	DECLARE_SHADER_TYPE(FVSMProjectionPixelShader,Global);
public:

	/** 
	* Constructor
	*/
	FVSMProjectionPixelShader() {}

	/** 
	* Constructor - binds all shader params
	* @param Initializer - init data from shader compiler
	*/
	FVSMProjectionPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer);

	/**
	* Sets the current pixel shader params
	* @param View - current view
	* @param ShadowInfo - projected shadow info for a single light
	*/
	void SetParameters(const FSceneView& View,const FProjectedShadowInfo* ShadowInfo);

	// FShader interface.	

	/**
	* @param Platform - hardware platform
	* @return TRUE if this shader should be cached
	*/
	static UBOOL ShouldCache(EShaderPlatform Platform);	
	
	/**
	* Serialize shader parameters for this shader
	* @param Ar - archive to serialize with
	*/
	virtual UBOOL Serialize(FArchive& Ar);

	/** VSM tweak epsilon */
	static FLOAT VSMEpsilon;
	/** VSM tweak exponent */
	static FLOAT VSMExponent;

private:
	/** params to sample scene color/depth */
	FSceneTextureShaderParameters SceneTextureParams;
	/** screen space to shadow map transform param */
	FShaderParameter ScreenToShadowMatrixParameter;
	/** shadow variance moments (M,M*M) param */
	FShaderResourceParameter ShadowVarianceTextureParameter;
	/** shadow depth texture param */
	FShaderResourceParameter ShadowDepthTextureParam;
	/** VSM tweak epsilon param */
	FShaderParameter VSMEpsilonParam;
	/** VSM tweak exponent param */
	FShaderParameter VSMExponentParam;
};

/**
* A pixel shader for projecting a shadow variance buffer onto the scene.
* Attenuates shadow based on distance and modulates its color.
* For use with modulated shadows.
*/
class FVSMModProjectionPixelShader : public FVSMProjectionPixelShader
{
	DECLARE_SHADER_TYPE(FVSMModProjectionPixelShader,Global);
public:

	/**
	* Constructor
	*/
	FVSMModProjectionPixelShader() {}

	/**
	* Constructor - binds all shader params
	* @param Initializer - init data from shader compiler
	*/
	FVSMModProjectionPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer);

	/**
	* Sets the current pixel shader
	* @param View - current view
	* @param ShadowInfo - projected shadow info for a single light
	*/
	virtual void SetParameters( const FSceneView& View, const FProjectedShadowInfo* ShadowInfo );

	// FShader interface.

	/**
	* Serialize the parameters for this shader
	* @param Ar - archive to serialize to
	*/
	virtual UBOOL Serialize(FArchive& Ar);

private:
	/** color to modulate shadowed areas on screen */
	FShaderParameter ShadowModulateColorParam;	
	/** needed to get world positions from deferred scene depth values */
	FShaderParameter ScreenToWorldParam;	
};

/**
* Attenuation is based on light type so the modulated shadow projection 
* is coupled with a LightTypePolicy type. Use with FModShadowProjectionVertexShader
*/
template<class LightTypePolicy>
class TVSMModProjectionPixelShader : public FVSMModProjectionPixelShader, public LightTypePolicy::ModShadowPixelParamsType
{
	DECLARE_SHADER_TYPE(TVSMModProjectionPixelShader,Global);
public:
	typedef typename LightTypePolicy::SceneInfoType LightSceneInfoType;

	/**
	* Constructor
	*/
	TVSMModProjectionPixelShader() {}

	/**
	* Constructor - binds all shader params
	* @param Initializer - init data from shader compiler
	*/
	TVSMModProjectionPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FVSMModProjectionPixelShader(Initializer)
	{
		LightTypePolicy::ModShadowPixelParamsType::Bind(Initializer.ParameterMap);
	}

	/**
	* Add any defines required by the shader or light policy
	* @param OutEnvironment - shader environment to modify
	*/
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		FVSMModProjectionPixelShader::ModifyCompilationEnvironment(Platform, OutEnvironment);
		LightTypePolicy::ModShadowPixelParamsType::ModifyCompilationEnvironment(Platform, OutEnvironment);		
	}

	/**
	* Sets the current pixel shader params
	* @param View - current view
	* @param ShadowInfo - projected shadow info for a single light
	*/
	void SetParameters(const FSceneView& View, const FProjectedShadowInfo* ShadowInfo)
	{
		FVSMModProjectionPixelShader::SetParameters(View,ShadowInfo);
		LightTypePolicy::ModShadowPixelParamsType::SetModShadowLight( this, (const LightSceneInfoType*) ShadowInfo->LightSceneInfo, &View);
	}

	/**
	* Serialize the parameters for this shader
	* @param Ar - archive to serialize to
	*/
	UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FVSMModProjectionPixelShader::Serialize(Ar);
		LightTypePolicy::ModShadowPixelParamsType::Serialize(Ar);
		return bShaderHasOutdatedParameters;
	}
};

/**
* Renders the shadow subject variance by sampling the existing shadow depth texture
* @param SceneRenderer - current scene rendering info
* @param Shadows - array of projected shadow infos
* @param DPGIndex - DPG level rendering the shadow projection pass
*/
void RenderShadowVariance(const class FSceneRenderer* SceneRenderer, const TArray<FProjectedShadowInfo*>& Shadows, UINT DPGIndex );

#endif //#if SUPPORTS_VSM


