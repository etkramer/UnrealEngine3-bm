/*=============================================================================
	DistortionRendering.h: Distortion rendering definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/**
* A vertex shader for rendering the full screen distortion pass
*/
class FDistortionApplyScreenVertexShader : public FShader
{
	DECLARE_SHADER_TYPE(FDistortionApplyScreenVertexShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	FDistortionApplyScreenVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FShader(Initializer)
	{
		TransformParameter.Bind(Initializer.ParameterMap,TEXT("Transform"));
	}
	FDistortionApplyScreenVertexShader() {}

	void SetParameters(const FMatrix& Transform)
	{
		SetVertexShaderValue(GetVertexShader(),TransformParameter,Transform);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << TransformParameter;
		return bShaderHasOutdatedParameters;
	}

private:	
	FShaderParameter TransformParameter;
};

/**
* A pixel shader for rendering the full screen distortion pass
*/
class FDistortionApplyScreenPixelShader : public FShader
{
	DECLARE_SHADER_TYPE(FDistortionApplyScreenPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	FDistortionApplyScreenPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FShader(Initializer)
	{
		AccumulatedDistortionTextureParam.Bind(Initializer.ParameterMap,TEXT("AccumulatedDistortionTexture"));
		SceneColorTextureParam.Bind(Initializer.ParameterMap,TEXT("SceneColorTexture"));
		SceneColorRectParameter.Bind(Initializer.ParameterMap,TEXT("SceneColorRect"));
	}
	FDistortionApplyScreenPixelShader() {}

	void SetParameters(FTextureRHIParamRef AccumulatedDistortionTexture,FTextureRHIParamRef SceneColorTexture, const FVector4& SceneColorRect )
	{
		SetTextureParameter(
			GetPixelShader(),
			AccumulatedDistortionTextureParam,
			TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
			AccumulatedDistortionTexture
			);
		SetTextureParameter(
			GetPixelShader(),
			SceneColorTextureParam,
			TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
			SceneColorTexture
			);
		SetPixelShaderValue(
			GetPixelShader(),
			SceneColorRectParameter,
			SceneColorRect
			);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
#if PS3
		//@hack - compiler bug? optimized version crashes during FShader::Serialize call
		static INT RemoveMe=0;	RemoveMe=1;
#endif
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << AccumulatedDistortionTextureParam;
		Ar << SceneColorTextureParam;
		Ar << SceneColorRectParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderResourceParameter AccumulatedDistortionTextureParam;
	FShaderResourceParameter SceneColorTextureParam;
	FShaderParameter SceneColorRectParameter;
};

/**
* Base draw policy for distort meshes
*/
class FDistortMeshPolicy
{
public:
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
 		return Material &&
 			IsTranslucentBlendMode(Material->GetBlendMode()) &&
 			Material->IsDistorted();
	}
};

/**
* Policy for drawing distortion mesh accumulated offsets
*/
class FDistortMeshAccumulatePolicy : public FDistortMeshPolicy
{	
};

/**
* A vertex shader to render distortion meshes
*/
template<class DistortMeshPolicy>
class TDistortionMeshVertexShader : public FShader
{
	DECLARE_SHADER_TYPE(TDistortionMeshVertexShader,MeshMaterial);

public:
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return DistortMeshPolicy::ShouldCache(Platform,Material,VertexFactoryType);
	}

	TDistortionMeshVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FShader(Initializer)
		,	VertexFactoryParameters(Initializer.VertexFactoryType,Initializer.ParameterMap)
	{
	}

	TDistortionMeshVertexShader()
	{
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		bShaderHasOutdatedParameters |= Ar << VertexFactoryParameters;
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

private:
	FVertexFactoryParameterRef VertexFactoryParameters;
};

/**
* A pixel shader to render distortion meshes
*/
template<class DistortMeshPolicy>
class TDistortionMeshPixelShader : public FShader
{
	DECLARE_SHADER_TYPE(TDistortionMeshPixelShader,MeshMaterial);

public:
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return DistortMeshPolicy::ShouldCache(Platform,Material,VertexFactoryType);
	}

	TDistortionMeshPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FShader(Initializer)
	{
		MaterialParameters.Bind(Initializer.Material,Initializer.ParameterMap);
	}

	TDistortionMeshPixelShader()
	{
	}

	void SetParameters(const FVertexFactory* VertexFactory,const FMaterialRenderProxy* MaterialRenderProxy,const FSceneView* View)
	{
		FMaterialRenderContext MaterialRenderContext(MaterialRenderProxy, View->Family->CurrentWorldTime, View->Family->CurrentRealTime, View);
		MaterialParameters.Set(this,MaterialRenderContext);
	}

	void SetMesh(const FMeshElement& Mesh,const FSceneView& View,UBOOL bBackFace)
	{
		MaterialParameters.SetMesh(this,Mesh,View,bBackFace);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{		
#if PS3
		//@hack - compiler bug? optimized version crashes during FShader::Serialize call
		static INT RemoveMe=0;	RemoveMe=1;
#endif
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << MaterialParameters;
		return bShaderHasOutdatedParameters;
	}

private:
	FMaterialPixelShaderParameters MaterialParameters;
};

/**
* Distortion mesh drawing policy
*/
template<class DistortMeshPolicy>
class TDistortionMeshDrawingPolicy : public FMeshDrawingPolicy
{
public:
	/** context type */
	typedef FMeshDrawingPolicy::ElementDataType ElementDataType;

	/**
	* Constructor
	* @param InIndexBuffer - index buffer for rendering
	* @param InVertexFactory - vertex factory for rendering
	* @param InMaterialRenderProxy - material instance for rendering
	* @param bInOverrideWithShaderComplexity - whether to override with shader complexity
	*/
	TDistortionMeshDrawingPolicy(
		const FVertexFactory* InVertexFactory,
		const FMaterialRenderProxy* InMaterialRenderProxy,
		UBOOL bInitializeOffsets,
		UBOOL bInOverrideWithShaderComplexity
		);

	// FMeshDrawingPolicy interface.

	/**
	* Match two draw policies
	* @param Other - draw policy to compare
	* @return TRUE if the draw policies are a match
	*/
	UBOOL Matches(const TDistortionMeshDrawingPolicy& Other) const;

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
	/** vertex shader based on policy type */
	TDistortionMeshVertexShader<DistortMeshPolicy>* VertexShader;
	/** whether we are initializing offsets or accumulating them */
	UBOOL bInitializeOffsets;
	/** pixel shader based on policy type */
	TDistortionMeshPixelShader<DistortMeshPolicy>* DistortPixelShader;
	/** pixel shader used to initialize offsets */
	FShaderComplexityAccumulatePixelShader* InitializePixelShader;
};

/**
* Distortion mesh draw policy factory. 
* Creates the policies needed for rendering a mesh based on its material
*/
template<class DistortMeshPolicy>
class TDistortionMeshDrawingPolicyFactory
{
public:
	enum { bAllowSimpleElements = FALSE };
	typedef UBOOL ContextType;

	/**
	* Render a dynamic mesh using a distortion mesh draw policy 
	* @return TRUE if the mesh rendered
	*/
	static UBOOL DrawDynamicMesh(
		const FSceneView& View,
		ContextType DrawingContext,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		UBOOL bPreFog,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		FHitProxyId HitProxyId
		);

	/**
	* Render a dynamic mesh using a distortion mesh draw policy 
	* @return TRUE if the mesh rendered
	*/
	static UBOOL DrawStaticMesh(
		const FSceneView* View,
		ContextType DrawingContext,
		const FStaticMesh& StaticMesh,
		UBOOL bBackFace,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		FHitProxyId HitProxyId
		);

	static UBOOL IsMaterialIgnored(const FMaterialRenderProxy* Material);
};

/** 
* Set of distortion scene prims  
*/
class FDistortionPrimSet
{
public:

	/** 
	* Iterate over the distortion prims and draw their accumulated offsets
	* @param ViewInfo - current view used to draw items
	* @param DPGIndex - current DPG used to draw items
	* @return TRUE if anything was drawn
	*/
	UBOOL DrawAccumulatedOffsets(const class FViewInfo* ViewInfo,UINT DPGIndex,UBOOL bInitializeOffsets);

	/** 
	* Apply distortion using the accumulated offsets as a fullscreen quad
	* @param ViewInfo - current view used to draw items
	* @param DPGIndex - current DPG used to draw items
	* @param CanvasTransform - default canvas transform used by scene rendering
	* @return TRUE if anything was drawn
	*/
	void DrawScreenDistort(const class FViewInfo* ViewInfo,UINT DPGIndex,const FMatrix& CanvasTransform, const FIntRect& QuadRect);

	/**
	* Add a new primitive to the list of distortion prims
	* @param PrimitiveSceneInfo - primitive info to add.
	* @param ViewInfo - used to transform bounds to view space
	*/
	void AddScenePrimitive(FPrimitiveSceneInfo* PrimitivieSceneInfo,const FViewInfo& ViewInfo);

	/** 
	* @return number of prims to render
	*/
	INT NumPrims() const
	{
		return Prims.Num();
	}

	/** 
	* @return a prim currently set to render
	*/
	const FPrimitiveSceneInfo* GetPrim(INT i)const
	{
		check(i>=0 && i<NumPrims());
		return Prims(i);
	}

private:
	/** list of distortion prims added from the scene */
	TArray<FPrimitiveSceneInfo*> Prims;

	/** bound shader state for applying fullscreen distort */
	static FGlobalBoundShaderState ApplyScreenBoundShaderState;

	/** bound shader state for transfering shader complexity from the accumulation target to scene color */
	static FGlobalBoundShaderState ShaderComplexityTransferBoundShaderState;
};
