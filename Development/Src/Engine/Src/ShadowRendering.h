/*=============================================================================
	ShadowRendering.h: Shadow rendering definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Forward declarations.
class FProjectedShadowInfo;
// Globals
extern const UINT SHADOW_BORDER;

/**
 * Outputs no color, but can be used to write the mesh's depth values to the depth buffer.
 */
class FShadowDepthDrawingPolicy : public FMeshDrawingPolicy
{
public:

	FShadowDepthDrawingPolicy(
		const FVertexFactory* InVertexFactory,
		const FMaterialRenderProxy* InMaterialRenderProxy,
		const FProjectedShadowInfo* InShadowInfo
		);

	// FMeshDrawingPolicy interface.
	UBOOL Matches(const FShadowDepthDrawingPolicy& Other) const
	{
		return FMeshDrawingPolicy::Matches(Other) && VertexShader == Other.VertexShader && PixelShader == Other.PixelShader;
	}
	void DrawShared(const FSceneView* View,FBoundShaderStateRHIParamRef BoundShaderState) const;

	/** 
	 * Create bound shader state using the vertex decl from the mesh draw policy
	 * as well as the shaders needed to draw the mesh
	 * @param DynamicStride - optional stride for dynamic vertex data
	 * @return new bound shader state object
	 */
	FBoundShaderStateRHIRef CreateBoundShaderState(DWORD DynamicStride = 0);

	void SetMeshRenderState(
		const FSceneView& View,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		const ElementDataType& ElementData
		) const;
	friend INT Compare(const FShadowDepthDrawingPolicy& A,const FShadowDepthDrawingPolicy& B);

private:
	class FShadowDepthVertexShader* VertexShader;
	class FShadowDepthPixelShader* PixelShader;
	const class FProjectedShadowInfo* ShadowInfo;
};

/**
 * A drawing policy factory for the emissive drawing policy.
 */
class FShadowDepthDrawingPolicyFactory
{
public:

	enum { bAllowSimpleElements = FALSE };
	typedef const FProjectedShadowInfo* ContextType;

	static UBOOL DrawDynamicMesh(
		const FSceneView& View,
		const FProjectedShadowInfo* ShadowInfo,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		UBOOL bPreFog,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		FHitProxyId HitProxyId
		);
	static UBOOL IsMaterialIgnored(const FMaterialRenderProxy* MaterialRenderProxy)
	{
		return FALSE;
	}
};

/**
 * A projected shadow transform.
 */
class FProjectedShadowInitializer
{
public:

	/** A translation that is applied to world-space before transforming by one of the shadow matrices. */
	FVector PreShadowTranslation;

	FMatrix PreSubjectMatrix;	// Z range from MinLightW to MaxSubjectW.
	FMatrix SubjectMatrix;		// Z range from MinSubjectW to MaxSubjectW.
	FMatrix PostSubjectMatrix;	// Z range from MinSubjectW to MaxShadowW.

	FLOAT MaxSubjectDepth;

	FLOAT AspectRatio;

	BITFIELD bDirectionalLight : 1;

	/**
	 * Calculates the shadow transforms for the given parameters.
	 */
	UBOOL CalcTransforms(
		const FVector& InPreShadowTranslation,
		const FMatrix& WorldToLight,
		const FVector& FaceDirection,
		const FBoxSphereBounds& SubjectBounds,
		const FVector4& WAxis,
		FLOAT MinLightW,
		FLOAT MaxLightW,
		UBOOL bInDirectionalLight
		);
};

/**
 * Information about a projected shadow.
 */
class FProjectedShadowInfo
{
public:

	friend class FShadowDepthVertexShader;
	friend class FShadowDepthPixelShader;
	friend class FShadowProjectionVertexShader;
	friend class FShadowProjectionPixelShader;
	friend class FShadowDepthDrawingPolicyFactory;

	typedef TArray<const FPrimitiveSceneInfo*,SceneRenderingAllocator> PrimitiveArrayType;

	const FLightSceneInfo* const LightSceneInfo;
	const FLightSceneInfoCompact LightSceneInfoCompact;

	const FPrimitiveSceneInfo* const ParentSceneInfo;
	const FLightPrimitiveInteraction* const ParentInteraction;

	INT ShadowId;

	/** A translation that is applied to world-space before transforming by one of the shadow matrices. */
	FVector PreShadowTranslation;

	FMatrix SubjectMatrix;
	FMatrix SubjectAndReceiverMatrix;
	FMatrix ReceiverMatrix;

	FMatrix InvReceiverMatrix;

	FLOAT MaxSubjectDepth;

	FConvexVolume SubjectFrustum;
	FConvexVolume ReceiverFrustum;

	UINT X;
	UINT Y;
	UINT ResolutionX;
	UINT ResolutionY;

	FLOAT FadeAlpha;

	BITFIELD bAllocated : 1;
	BITFIELD bRendered : 1;
	BITFIELD bDirectionalLight : 1;
	BITFIELD bPreShadow : 1;
	BITFIELD bCullShadowOnBackfacesAndEmissive : 1;

	/** Indicates whether the shadow has subjects in the foreground DPG but should cast shadows onto receivers in the world DPG */
	BITFIELD bForegroundCastingOnWorld : 1;

	/** True if any of the subjects only self shadow and do not cast shadows on other primitives. */
	BITFIELD bSelfShadowOnly : 1;

	/** Initialization constructor for an individual primitive shadow. */
	FProjectedShadowInfo(
		FLightSceneInfo* InLightSceneInfo,
		const FPrimitiveSceneInfo* InParentSceneInfo,
		const FLightPrimitiveInteraction* const InParentInteraction,
		const FProjectedShadowInitializer& Initializer,
		UBOOL bInPreShadow,
		UINT InResolutionX,
		UINT InResolutionY,
		FLOAT InFadeAlpha
		);

	/** Initialization constructor for a whole-scene shadow. */
	FProjectedShadowInfo(
		FLightSceneInfo* InLightSceneInfo,
		const FProjectedShadowInitializer& Initializer,
		UINT InResolutionX,
		UINT InResolutionY,
		FLOAT InFadeAlpha
		);

	/**
	 * Renders the shadow subject depth.
	 */
	void RenderDepth(const class FSceneRenderer* SceneRenderer, BYTE DepthPriorityGroup);

	/**
	 * Projects the shadow onto the scene for a particular view.
	 */
	void RenderProjection(const class FViewInfo* View, BYTE DepthPriorityGroup) const;

	/**
	 * Renders the projected shadow's frustum wireframe with the given FPrimitiveDrawInterface.
	 */
	void RenderFrustumWireframe(FPrimitiveDrawInterface* PDI) const;

	/**
	 * Adds a primitive to the shadow's subject list.
	 */
	void AddSubjectPrimitive(FPrimitiveSceneInfo* PrimitiveSceneInfo);

	/**
	 * Adds a primitive to the shadow's receiver list.
	 */
	void AddReceiverPrimitive(FPrimitiveSceneInfo* PrimitiveSceneInfo);

	/**
	* @return TRUE if this shadow info has any casting subject prims to render
	*/
	UBOOL HasSubjectPrims() const;

	/** 
	 * @param View view to check visibility in
	 * @return TRUE if this shadow info has any subject prims visible in the view
	 */
	UBOOL SubjectsVisible(const FViewInfo& View) const;

	/**
	 * Adds current subject primitives to out array.
	 *
	 * @param OutSubjectPrimitives [out]	Array to add current subject primitives to.
	 */
	void GetSubjectPrimitives( PrimitiveArrayType& OutSubjectPrimitives );

	/**
	* Adds a primitive to the modulated shadow's receiver list. 
	* These are rendered to mask out emissive and backface areas which shouldn't have shadows
	* 
	* @param PrimitiveSceneInfo - the primitive to add to the list
	*/
	void AddModShadowReceiverPrimitive(const FPrimitiveSceneInfo* PrimitiveSceneInfo);
	
	/**
	* Render modulated shadow receiver primitives using the FMeshModShadowDrawingPolicyFactory
	*
	* @param View - current view to render with
	* @param DepthPriorityGroup - current DPG to cull mesh elements based on relevancy
	* @param ModShadowPrims - list of primitives to render
	* @return TRUE if anything was rendered
	*/
	UBOOL RenderModShadowPrimitives(
		const class FViewInfo* View, BYTE DepthPriorityGroup, const PrimitiveArrayType& ModShadowPrims ) const;

	/** Hash function. */
	friend DWORD GetTypeHash(const FProjectedShadowInfo* ProjectedShadowInfo)
	{
		return PointerHash(ProjectedShadowInfo);
	}

private:

	/** dynamic shadow casting elements */
	PrimitiveArrayType SubjectPrimitives;
	/** For preshadows, this contains the receiver primitives to mask the projection to. */
	PrimitiveArrayType ReceiverPrimitives;
	/** static shadow casting elements */
	TStaticMeshDrawList<FShadowDepthDrawingPolicy> SubjectMeshElements;
	/** 
	* Primitives affected by modulated shadow which are within the shadow frustum and are not occluded.
	* These are rendered to mask out emissive and backface areas which shouldn't have shadows 
	*/
	PrimitiveArrayType ModShadowReceiverPrimitives;
	/** bound shader state for stencil masking the shadow projection */
	static FGlobalBoundShaderState MaskBoundShaderState;
	/** bound shader state for shadow projection pass */
	static FGlobalBoundShaderState ShadowProjectionBoundShaderState;
	/** bound shader state for VSM shadow projection pass */
	static FGlobalBoundShaderState ShadowProjectionVSMBoundShaderState;
	/** bound shader state for Branching PCF shadow projection pass */
	static FGlobalBoundShaderState BranchingPCFLowQualityBoundShaderState;
	static FGlobalBoundShaderState BranchingPCFMediumQualityBoundShaderState;
	static FGlobalBoundShaderState BranchingPCFHighQualityBoundShaderState;
};

IMPLEMENT_COMPARE_POINTER(FProjectedShadowInfo,ShadowRendering,{ return B->ResolutionX*B->ResolutionY - A->ResolutionX*A->ResolutionY; });

/*-----------------------------------------------------------------------------
FShadowFrustumVertexDeclaration
-----------------------------------------------------------------------------*/

/** The shadow frustum vertex declaration resource type. */
class FShadowFrustumVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	virtual ~FShadowFrustumVertexDeclaration() {}

	virtual void InitRHI()
	{
		FVertexDeclarationElementList Elements;
		Elements.AddItem(FVertexElement(0,0,VET_Float3,VEU_Position,0));
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI()
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

/**
* A vertex shader for projecting a shadow depth buffer onto the scene.
*/
class FShadowProjectionVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FShadowProjectionVertexShader,Global);
public:

	FShadowProjectionVertexShader() {}
	FShadowProjectionVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer);

	static UBOOL ShouldCache(EShaderPlatform Platform);

	void SetParameters(const FSceneView& View,const FProjectedShadowInfo* ShadowInfo);
	virtual UBOOL Serialize(FArchive& Ar);

	virtual EShaderRecompileGroup GetRecompileGroup() const
	{
		return SRG_GLOBAL_MISC_SHADOW;
	}
};


/**
* A vertex shader for projecting a shadow depth buffer onto the scene.
* For use with modulated shadows
*/
class FModShadowProjectionVertexShader : public FShadowProjectionVertexShader
{
	DECLARE_SHADER_TYPE(FModShadowProjectionVertexShader,Global);
public:

	/**
	* Constructor
	*/
	FModShadowProjectionVertexShader() {}

	/**
	* Constructor - binds all shader params
	* @param Initializer - init data from shader compiler
	*/
	FModShadowProjectionVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer);

	/**
	* Sets the current vertex shader
	* @param View - current view
	* @param ShadowInfo - projected shadow info for a single light
	*/
	void SetParameters(const FSceneView& View,const FProjectedShadowInfo* ShadowInfo);

	/**
	* Serialize the parameters for this shader
	* @param Ar - archive to serialize to
	*/
	virtual UBOOL Serialize(FArchive& Ar);

	virtual EShaderRecompileGroup GetRecompileGroup() const
	{
		return SRG_GLOBAL_MISC_SHADOW;
	}
};

/**
* 
*/
class F4SampleHwPCF
{
public:
	static const UINT NumSamples = 4;
	static const UBOOL bUseHardwarePCF = TRUE;
	static const UBOOL bUseFetch4 = FALSE;

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return Platform != SP_XBOXD3D;
	}
};

/**
* 
*/
class F4SampleManualPCF
{
public:
	static const UINT NumSamples = 4;
	static const UBOOL bUseHardwarePCF = FALSE;
	static const UBOOL bUseFetch4 = FALSE;

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}
};

/**
 * Policy to use on SM3 hardware that supports Hardware PCF
 */
class F16SampleHwPCF
{
public:
	static const UINT NumSamples = 16;
	static const UBOOL bUseHardwarePCF = TRUE;
	static const UBOOL bUseFetch4 = FALSE;

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return Platform == SP_PCD3D_SM3 || Platform == SP_PS3;
	}
};

/**
 * Policy to use on SM3 hardware that supports Fetch4
 */
class F16SampleFetch4PCF
{
public:
	static const UINT NumSamples = 16;
	static const UBOOL bUseHardwarePCF = FALSE;
	static const UBOOL bUseFetch4 = TRUE;

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return Platform == SP_PCD3D_SM3;
	}
};

/**
 * Policy to use on SM3 hardware with no support for Hardware PCF
 */
class F16SampleManualPCF
{
public:
	static const UINT NumSamples = 16;
	static const UBOOL bUseHardwarePCF = FALSE;
	static const UBOOL bUseFetch4 = FALSE;

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return Platform != SP_PCD3D_SM2;
	}
};

/**
 * Samples used with the SM3 version
 */
static const FVector2D SixteenSampleOffsets[] =
{
	FVector2D(-1.5f,-1.5f), FVector2D(-0.5f,-1.5f), FVector2D(+0.5f,-1.5f), FVector2D(+1.5f,-1.5f),
	FVector2D(-1.5f,-0.5f), FVector2D(-0.5f,-0.5f), FVector2D(+0.5f,-0.5f), FVector2D(+1.5f,-0.5f),
	FVector2D(-1.5f,+0.5f), FVector2D(-0.5f,+0.5f), FVector2D(+0.5f,+0.5f), FVector2D(+1.5f,+0.5f),
	FVector2D(-1.5f,+1.5f), FVector2D(-0.5f,+1.5f), FVector2D(+0.5f,+1.5f), FVector2D(+1.5f,+1.5f)
};

/**
 * Samples used with the SM2 version
 */
static const FVector2D FourSampleOffsets[] = 
{
	FVector2D(-0.5f,-0.5f), FVector2D(+0.5f,-0.5f), FVector2D(-0.5f,+0.5f), FVector2D(+0.5f,+0.5f)
};

/**
 * FShadowProjectionPixelShaderInterface - used to handle templated versions
 */

class FShadowProjectionPixelShaderInterface : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FShadowProjectionPixelShaderInterface,Global);
public:

	FShadowProjectionPixelShaderInterface() 
		:	FGlobalShader()
	{}

	/**
	 * Constructor - binds all shader params and initializes the sample offsets
	 * @param Initializer - init data from shader compiler
	 */
	FShadowProjectionPixelShaderInterface(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FGlobalShader(Initializer)
	{ }

	/**
	 * Sets the current pixel shader params
	 * @param View - current view
	 * @param ShadowInfo - projected shadow info for a single light
	 */
	virtual void SetParameters(
		const FSceneView& View,
		const FProjectedShadowInfo* ShadowInfo
		)
	{ }

};

/**
 * TShadowProjectionPixelShader
 * A pixel shader for projecting a shadow depth buffer onto the scene.  Used with any light type casting normal shadows.
 */
template<class UniformPCFPolicy> 
class TShadowProjectionPixelShader : public FShadowProjectionPixelShaderInterface
{
	DECLARE_SHADER_TYPE(TShadowProjectionPixelShader,Global);
public:

	TShadowProjectionPixelShader()
		: FShadowProjectionPixelShaderInterface()
	{ 
		SetSampleOffsets(); 
	}

	/**
	 * Constructor - binds all shader params and initializes the sample offsets
	 * @param Initializer - init data from shader compiler
	 */
	TShadowProjectionPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
	FShadowProjectionPixelShaderInterface(Initializer)
	{
		SceneTextureParams.Bind(Initializer.ParameterMap);
		ScreenToShadowMatrixParameter.Bind(Initializer.ParameterMap,TEXT("ScreenToShadowMatrix"));
		ShadowDepthTextureParameter.Bind(Initializer.ParameterMap,TEXT("ShadowDepthTexture"));
		SampleOffsetsParameter.Bind(Initializer.ParameterMap,TEXT("SampleOffsets"),TRUE);
		ShadowBufferSizeParameter.Bind(Initializer.ParameterMap,TEXT("ShadowBufferSize"),TRUE);

		SetSampleOffsets();
	}

	/**
	 *  Initializes the sample offsets
	 */
	void SetSampleOffsets()
	{
		check(UniformPCFPolicy::NumSamples == 4 || UniformPCFPolicy::NumSamples == 16);

		if (UniformPCFPolicy::NumSamples == 4)
		{
			appMemcpy(SampleOffsets, FourSampleOffsets, 4 * sizeof(FVector2D));
		}
		else if (UniformPCFPolicy::NumSamples == 16)
		{
			appMemcpy(SampleOffsets, SixteenSampleOffsets, 16 * sizeof(FVector2D));
		}
	}

	/**
	 * @param Platform - hardware platform
	 * @return TRUE if this shader should be cached
	 */
	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return UniformPCFPolicy::ShouldCache(Platform);
	}

	/**
	 * Add any defines required by the shader
	 * @param OutEnvironment - shader environment to modify
	 */
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("NUM_SAMPLE_CHUNKS"),*FString::Printf(TEXT("%u"),UniformPCFPolicy::NumSamples / 4));
	}

	/**
	 * Sets the pixel shader's parameters
	 * @param View - current view
	 * @param ShadowInfo - projected shadow info for a single light
	 */
	virtual void SetParameters(
		const FSceneView& View,
		const FProjectedShadowInfo* ShadowInfo
		)
	{
		SceneTextureParams.Set(&View,this);

		// Set the transform from screen coordinates to shadow depth texture coordinates.
		const FLOAT InvBufferResolution = 1.0f / (FLOAT)GSceneRenderTargets.GetShadowDepthTextureResolution();
		const FLOAT ShadowResolutionFractionX = 0.5f * (FLOAT)ShadowInfo->ResolutionX * InvBufferResolution;
		const FLOAT ShadowResolutionFractionY = 0.5f * (FLOAT)ShadowInfo->ResolutionY * InvBufferResolution;
		const FMatrix ScreenToShadow = FMatrix(
			FPlane(1,0,0,0),
			FPlane(0,1,0,0),
			FPlane(0,0,View.ProjectionMatrix.M[2][2],1),
			FPlane(0,0,View.ProjectionMatrix.M[3][2],0)
			) *
			View.InvTranslatedViewProjectionMatrix *
			FTranslationMatrix(ShadowInfo->PreShadowTranslation - View.PreViewTranslation) *
			ShadowInfo->SubjectAndReceiverMatrix *
			FMatrix(
			FPlane(ShadowResolutionFractionX,0,							0,									0),
			FPlane(0,						 -ShadowResolutionFractionY,0,									0),
			FPlane(0,						0,							1.0f / ShadowInfo->MaxSubjectDepth,	0),
			FPlane(
			(ShadowInfo->X + SHADOW_BORDER + GPixelCenterOffset) * InvBufferResolution + ShadowResolutionFractionX,
			(ShadowInfo->Y + SHADOW_BORDER + GPixelCenterOffset) * InvBufferResolution + ShadowResolutionFractionY,
			0,
			1
			)
			);
		SetPixelShaderValue(FShader::GetPixelShader(),ScreenToShadowMatrixParameter,ScreenToShadow);

		if (ShadowBufferSizeParameter.IsBound())
		{
			SetPixelShaderValue(GetPixelShader(),ShadowBufferSizeParameter, FVector2D((FLOAT)GSceneRenderTargets.GetShadowDepthTextureResolution(), (FLOAT)GSceneRenderTargets.GetShadowDepthTextureResolution()));
		}

		FTexture2DRHIRef ShadowDepthSampler;
		FSamplerStateRHIParamRef DepthSamplerState;

		if (UniformPCFPolicy::bUseHardwarePCF)
		{
			//take advantage of linear filtering on nvidia depth stencil textures
			DepthSamplerState = TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
			//sample the depth texture
			ShadowDepthSampler = GSceneRenderTargets.GetShadowDepthZTexture();
		}
		else if (GSupportsDepthTextures)
		{
			DepthSamplerState = TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
			//sample the depth texture
			ShadowDepthSampler = GSceneRenderTargets.GetShadowDepthZTexture();
		} 
		else if (UniformPCFPolicy::bUseFetch4)
		{
			//enable Fetch4 on this sampler
			DepthSamplerState = TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp,MIPBIAS_Get4>::GetRHI();
			//sample the depth texture
			ShadowDepthSampler = GSceneRenderTargets.GetShadowDepthZTexture();
		}
		else
		{
			DepthSamplerState = TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
			//sample the color depth texture
			ShadowDepthSampler = GSceneRenderTargets.GetShadowDepthColorTexture();
		}

		SetTextureParameter(
			FShader::GetPixelShader(),
			ShadowDepthTextureParameter,
			DepthSamplerState,
			ShadowDepthSampler
			);

		const FLOAT CosRotation = appCos(0.25f * (FLOAT)PI);
		const FLOAT SinRotation = appSin(0.25f * (FLOAT)PI);
		const FLOAT TexelRadius = GSystemSettings.ShadowFilterRadius / 2 * InvBufferResolution;
		//set the sample offsets
		for(INT SampleIndex = 0;SampleIndex < UniformPCFPolicy::NumSamples;SampleIndex += 2)
		{
			SetPixelShaderValue(
				FShader::GetPixelShader(),
				SampleOffsetsParameter,
				FVector4(
				(SampleOffsets[0].X * +CosRotation + SampleOffsets[SampleIndex + 0].Y * +SinRotation) * TexelRadius,
				(SampleOffsets[0].X * -SinRotation + SampleOffsets[SampleIndex + 0].Y * +CosRotation) * TexelRadius,
				(SampleOffsets[1].X * +CosRotation + SampleOffsets[SampleIndex + 1].Y * +SinRotation) * TexelRadius,
				(SampleOffsets[1].X * -SinRotation + SampleOffsets[SampleIndex + 1].Y * +CosRotation) * TexelRadius
				),
				SampleIndex / 2
				);
		}
	}

	/**
	 * Serialize the parameters for this shader
	 * @param Ar - archive to serialize to
	 */
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << SceneTextureParams;
		Ar << ScreenToShadowMatrixParameter;
		Ar << ShadowDepthTextureParameter;
		Ar << SampleOffsetsParameter;
		Ar << ShadowBufferSizeParameter;
		return bShaderHasOutdatedParameters;
	}

	virtual EShaderRecompileGroup GetRecompileGroup() const
	{
		return SRG_GLOBAL_MISC_SHADOW;
	}

private:
	FVector2D SampleOffsets[UniformPCFPolicy::NumSamples];
	FSceneTextureShaderParameters SceneTextureParams;
	FShaderParameter ScreenToShadowMatrixParameter;
	FShaderResourceParameter ShadowDepthTextureParameter;
	FShaderParameter SampleOffsetsParameter;	
	FShaderParameter ShadowBufferSizeParameter;
};

/**
 * TModShadowProjectionPixelShader - pixel shader used with lights casting modulative shadows
 * Attenuation is based on light type so the modulated shadow projection is coupled with a LightTypePolicy type
 */
template<class LightTypePolicy, class UniformPCFPolicy>
class TModShadowProjectionPixelShader : public TShadowProjectionPixelShader<UniformPCFPolicy>, public LightTypePolicy::ModShadowPixelParamsType
{
	DECLARE_SHADER_TYPE(TModShadowProjectionPixelShader,Global);
public:
	typedef typename LightTypePolicy::SceneInfoType LightSceneInfoType;

	/**
	 * Constructor
	 */
	TModShadowProjectionPixelShader() {}

	/**
	* Constructor - binds all shader params
	* @param Initializer - init data from shader compiler
	*/
	TModShadowProjectionPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	TShadowProjectionPixelShader<UniformPCFPolicy>(Initializer)
	{
		ShadowModulateColorParam.Bind(Initializer.ParameterMap,TEXT("ShadowModulateColor"));
		ScreenToWorldParam.Bind(Initializer.ParameterMap,TEXT("ScreenToWorld"),TRUE);
		EmissiveAlphaMaskScale.Bind(Initializer.ParameterMap,TEXT("EmissiveAlphaMaskScale"),TRUE);
		UseEmissiveMaskParameter.Bind(Initializer.ParameterMap,TEXT("bApplyEmissiveToShadowCoverage"),TRUE);
		LightTypePolicy::ModShadowPixelParamsType::Bind(Initializer.ParameterMap);
	}

	/**
	 * @param Platform - hardware platform
	 * @return TRUE if this shader should be cached
	 */
	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TShadowProjectionPixelShader<UniformPCFPolicy>::ShouldCache(Platform);
	}

	/**
	 * Add any defines required by the shader or light policy
	 * @param OutEnvironment - shader environment to modify
	 */
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		TShadowProjectionPixelShader<UniformPCFPolicy>::ModifyCompilationEnvironment(Platform, OutEnvironment);
        LightTypePolicy::ModShadowPixelParamsType::ModifyCompilationEnvironment(Platform, OutEnvironment);	
	}

	/**
	 * Sets the pixel shader's parameters
	 * @param View - current view
	 * @param ShadowInfo - projected shadow info for a single light
	 */
	virtual void SetParameters(
		const FSceneView& View,
		const FProjectedShadowInfo* ShadowInfo
		)
	{
		TShadowProjectionPixelShader<UniformPCFPolicy>::SetParameters(View,ShadowInfo);		
		const FLightSceneInfo* LightSceneInfo = ShadowInfo->LightSceneInfo;

		// color to modulate shadowed areas on screen
		SetPixelShaderValue(
			FShader::GetPixelShader(),
			ShadowModulateColorParam,
			Lerp(FLinearColor::White,LightSceneInfo->ModShadowColor,ShadowInfo->FadeAlpha)
			);
		// screen space to world space transform
		FMatrix ScreenToWorld = FMatrix(
			FPlane(1,0,0,0),
			FPlane(0,1,0,0),
			FPlane(0,0,(1.0f - Z_PRECISION),1),
			FPlane(0,0,-View.NearClippingDistance * (1.0f - Z_PRECISION),0)
			) * 
			View.InvTranslatedViewProjectionMatrix;	
		SetPixelShaderValue( FShader::GetPixelShader(), ScreenToWorldParam, ScreenToWorld );

#if XBOX
		const FLOAT SceneColorScale = appPow(2.0f,GCurrentColorExpBias);
		// when the scene color texture is not Raw, we need to scale it by SCENE_COLOR_BIAS_FACTOR_EXP to re-normalize the range
		SetPixelShaderValue(GetPixelShader(),EmissiveAlphaMaskScale, GSceneRenderTargets.bSceneColorTextureIsRaw ? 1.0f : SceneColorScale);	
		// use the emissive mask for modulate-better shadows
		SetPixelShaderBool(GetPixelShader(),UseEmissiveMaskParameter,ShadowInfo->bCullShadowOnBackfacesAndEmissive);
#endif

		LightTypePolicy::ModShadowPixelParamsType::SetModShadowLight( this, (const LightSceneInfoType*) ShadowInfo->LightSceneInfo, &View );
	}

	/**
	 * Serialize the parameters for this shader
	 * @param Ar - archive to serialize to
	 */
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = TShadowProjectionPixelShader<UniformPCFPolicy>::Serialize(Ar);
		Ar << ShadowModulateColorParam;
		Ar << ScreenToWorldParam;
		Ar << EmissiveAlphaMaskScale;
		Ar << UseEmissiveMaskParameter;
		LightTypePolicy::ModShadowPixelParamsType::Serialize(Ar);
		return bShaderHasOutdatedParameters;
	}

	virtual EShaderRecompileGroup GetRecompileGroup() const
	{
		return SRG_GLOBAL_MISC_SHADOW;
	}

private:
	/** color to modulate shadowed areas on screen */
	FShaderParameter ShadowModulateColorParam;	
	/** needed to get world positions from deferred scene depth values */
	FShaderParameter ScreenToWorldParam;
	FShaderParameter EmissiveAlphaMaskScale;
	FShaderParameter UseEmissiveMaskParameter;
};

/**
 * Get the version of TModShadowProjectionPixelShader that should be used based on the hardware's capablities
 * @return a pointer to the chosen shader
 */
template<class LightTypePolicy>
FShadowProjectionPixelShaderInterface* GetModProjPixelShaderRef(BYTE LightShadowQuality)
{
	//apply the system settings bias to the light's shadow quality
	BYTE EffectiveShadowFilterQuality = Max(LightShadowQuality + GSystemSettings.ShadowFilterQualityBias, 0);

	//force using 4 samples for shader model 2 cards
	if (EffectiveShadowFilterQuality == SFQ_Low || GRHIShaderPlatform == SP_PCD3D_SM2)
	{
		if (GSupportsHardwarePCF)
		{
			TShaderMapRef<TModShadowProjectionPixelShader<LightTypePolicy,F4SampleHwPCF> > ModShadowShader(GetGlobalShaderMap());
			return *ModShadowShader;
		}
		else
		{
			TShaderMapRef<TModShadowProjectionPixelShader<LightTypePolicy,F4SampleManualPCF> > ModShadowShader(GetGlobalShaderMap());
			return *ModShadowShader;
		}
	}
	else
	{
		if (GSupportsHardwarePCF)
		{
			TShaderMapRef<TModShadowProjectionPixelShader<LightTypePolicy,F16SampleHwPCF> > ModShadowShader(GetGlobalShaderMap());
			return *ModShadowShader;
		}
		else if (GSupportsFetch4)
		{
			TShaderMapRef<TModShadowProjectionPixelShader<LightTypePolicy,F16SampleFetch4PCF> > ModShadowShader(GetGlobalShaderMap());
			return *ModShadowShader;
		}
		else
		{
			TShaderMapRef<TModShadowProjectionPixelShader<LightTypePolicy,F16SampleManualPCF> > ModShadowShader(GetGlobalShaderMap());
			return *ModShadowShader;
		}
	}
	
}

/**
* Vertex shader to render modulated shadow receiver meshes
*/
class FModShadowMeshVertexShader : public FShader
{
	DECLARE_SHADER_TYPE(FModShadowMeshVertexShader,MeshMaterial);

public:
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return(	Material && 
				!IsTranslucentBlendMode(Material->GetBlendMode()) &&
				Material->GetLightingModel() != MLM_Unlit );
	}

	FModShadowMeshVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FShader(Initializer)
		,	VertexFactoryParameters(Initializer.VertexFactoryType,Initializer.ParameterMap)
	{
		LightPositionParameter.Bind(Initializer.ParameterMap,TEXT("LightPosition"));
	}

	FModShadowMeshVertexShader()
	{		
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);		
		bShaderHasOutdatedParameters |= Ar << VertexFactoryParameters;
		Ar << LightPositionParameter;
		return bShaderHasOutdatedParameters;
	}

	void SetParameters(const FVertexFactory* VertexFactory,const FMaterialRenderProxy* MaterialRenderProxy,const FSceneView* View)
	{
		VertexFactoryParameters.Set(this,VertexFactory,*View);
	}

	void SetMesh(const FMeshElement& Mesh,const FSceneView& View)
	{
		VertexFactoryParameters.SetMesh(this,Mesh,View);
	}
	
	void SetLightPosition(FShader* VertexShader, const FVector4& LightPosition) const
	{
		SetVertexShaderValue(VertexShader->GetVertexShader(),LightPositionParameter,LightPosition);
	}
	
private:
	FVertexFactoryParameterRef VertexFactoryParameters;
	FShaderParameter LightPositionParameter;
};

/**
* Pixel shader to render modulated shadow receiver meshes
*/
class FModShadowMeshPixelShader : public FShader
{
	DECLARE_SHADER_TYPE(FModShadowMeshPixelShader,MeshMaterial);

public:
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return(	Material && 
				!IsTranslucentBlendMode(Material->GetBlendMode()) &&
				Material->GetLightingModel() != MLM_Unlit );
	}

	FModShadowMeshPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FShader(Initializer)
	{
		MaterialParameters.Bind(Initializer.Material,Initializer.ParameterMap);
		AttenAllowedParameter.Bind(Initializer.ParameterMap,TEXT("AttenAllowed"));
	}

	FModShadowMeshPixelShader()
	{
	}

	void SetParameters(const FVertexFactory* VertexFactory,const FMaterialRenderProxy* MaterialRenderProxy,const FSceneView* View)
	{
		FMaterialRenderContext MaterialRenderContext(MaterialRenderProxy, View->Family->CurrentWorldTime, View->Family->CurrentRealTime, View);
		MaterialParameters.Set(this,MaterialRenderContext);
	}

	void SetMesh(const FMeshElement& Mesh,const FSceneView& View,UBOOL bBackFace,const FPrimitiveSceneInfo& PrimSceneInfo)
	{
		MaterialParameters.SetMesh(this,Mesh,View,bBackFace);

		FLOAT AttenAllowed[2] = { 
			PrimSceneInfo.bCullModulatedShadowOnBackfaces ? 1.0f : 0.0f,
			PrimSceneInfo.bCullModulatedShadowOnEmissive ? 1.0f : 0.0f,
		};
		SetPixelShaderValue(GetPixelShader(),AttenAllowedParameter,AttenAllowed);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{		
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << MaterialParameters;		
		Ar << AttenAllowedParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FMaterialPixelShaderParameters MaterialParameters;
	FShaderParameter AttenAllowedParameter;
};

/**
* Modulated shadow mesh drawing policy. For attenuating modulated shadows based on N*L and emissive
*/
class FMeshModShadowDrawingPolicy : public FMeshDrawingPolicy
{
public:
	/** context type */
	typedef FMeshDrawingPolicy::ElementDataType ElementDataType;

	/**
	* Constructor
	* @param InVertexFactory - vertex factory for rendering
	* @param InMaterialRenderProxy - material instance for rendering
	* @param InLight - info about light casting a shadow on this mesh
	*/
	FMeshModShadowDrawingPolicy(
		const FVertexFactory* InVertexFactory,
		const FMaterialRenderProxy* InMaterialRenderProxy,
		const FLightSceneInfo* InLight);

	// FMeshDrawingPolicy interface.

	/**
	* Match two draw policies
	* @param Other - draw policy to compare
	* @return TRUE if the draw policies are a match
	*/
	UBOOL Matches(const FMeshModShadowDrawingPolicy& Other) const;

	/**
	* Executes the draw commands which can be shared between any meshes using this drawer.
	* @param CI - The command interface to execute the draw commands on.
	* @param View - The view of the scene being drawn.
	*/
	void DrawShared(const FSceneView* View,FBoundShaderStateRHIParamRef BoundShaderState) const;

	/** 
	* Create bound shader state using the vertex decl from the mesh draw policy
	* as well as the shaders needed to draw the mesh
	* @param DynamicStride - optional stride for dynamic vertex data
	* @return new bound shader state object
	*/
	FBoundShaderStateRHIRef CreateBoundShaderState(DWORD DynamicStride = 0);

	friend INT Compare(const FMeshModShadowDrawingPolicy& A,const FMeshModShadowDrawingPolicy& B);

	/**
	* Sets the render states for drawing a mesh.
	* @param PrimitiveSceneInfo - The primitive drawing the dynamic mesh.  If this is a view element, this will be NULL.
	* @param Mesh - mesh element with data needed for rendering
	* @param ElementData - context specific data for mesh rendering
	*/
	void SetMeshRenderState(
		const FSceneView& View,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		const ElementDataType& ElementData
		) const;

private:
	/** vertex shader for mod shadow atten */
	FModShadowMeshVertexShader* VertexShader;
	/** pixel shader for mod shadow atten */
	FModShadowMeshPixelShader* PixelShader;
	/** light casting a shadow on this mesh */
	const FLightSceneInfo* Light;
	
};

/**
* Modulated shadow mesh drawing policy factor. For attenuating modulated shadows based on N*L and emissive
*/
class FMeshModShadowDrawingPolicyFactory
{
public:

	enum { bAllowSimpleElements = FALSE };
	typedef const FLightSceneInfo* ContextType;

	/**
	* Render a dynamic mesh using the mod shadow mesh mesh draw policy 
	*
	* @return TRUE if the mesh rendered
	*/
	static UBOOL DrawDynamicMesh(
		const FSceneView& View,
		const FLightSceneInfo* Light,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		UBOOL bPreFog,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		FHitProxyId HitProxyId
		);

	/**
	* Render a static mesh using the mod shadow mesh mesh draw policy 
	*
	* @return TRUE if the mesh rendered
	*/
	static UBOOL DrawStaticMesh(
		const FSceneView* View,
		const FLightSceneInfo* Light,
		const FStaticMesh& StaticMesh,
		UBOOL bBackFace,
		UBOOL bPreFog,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		FHitProxyId HitProxyId
		);

	static UBOOL IsMaterialIgnored(const FMaterialRenderProxy* MaterialRenderProxy);
};

