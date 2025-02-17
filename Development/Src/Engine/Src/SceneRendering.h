/*=============================================================================
	SceneRendering.h: Scene rendering definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/** An association between a hit proxy and a mesh. */
class FHitProxyMeshPair : public FMeshElement
{
public:
	FHitProxyId HitProxyId;

	/** Initialization constructor. */
	FHitProxyMeshPair(const FMeshElement& InMesh,FHitProxyId InHitProxyId):
		FMeshElement(InMesh),
		HitProxyId(InHitProxyId)
	{}
};

/** Information about a visible light which is specific to the view it's visible in. */
class FVisibleLightViewInfo
{
public:

	/** Information about a visible light in a specific DPG */
	class FDPGInfo
	{
	public:
		FDPGInfo()
			:	bHasVisibleLitPrimitives(FALSE)
		{}

		/** The dynamic primitives which are both visible and affected by this light. */
		TArray<FPrimitiveSceneInfo*,SceneRenderingAllocator> VisibleDynamicLitPrimitives;

		/** The primitives which are visible, affected by this light and receiving lit decals. */
		TArray<FPrimitiveSceneInfo*,SceneRenderingAllocator> VisibleLitDecalPrimitives;

		/** Whether the light has any visible lit primitives (static or dynamic) in the DPG */
		UBOOL bHasVisibleLitPrimitives;
	};

	/** true if this light in the view frustum (dir/sky lights always are). */
	UBOOL bInViewFrustum;

	/** Information about the light in each DPG. */
	FDPGInfo DPGInfo[SDPG_MAX_SceneRender];
	
	/** Whether each shadow in the corresponding FVisibleLightInfo::ProjectedShadows array is visible. */
	TBitArray<SceneRenderingBitArrayAllocator> ProjectedShadowVisibilityMap;

	/** The view relevance of each shadow in the corresponding FVisibleLightInfo::ProjectedShadows array. */
	TArray<FPrimitiveViewRelevance,SceneRenderingAllocator> ProjectedShadowViewRelevanceMap;

	/** Initialization constructor. */
	FVisibleLightViewInfo()
	:	bInViewFrustum(FALSE)
	{}

	/** Check whether the Light has visible lit primitives in any DPG */
	UBOOL HasVisibleLitPrimitives() const 
	{ 
		for (UINT DPGIndex=0;DPGIndex<SDPG_MAX_SceneRender;++DPGIndex)
		{
			if (DPGInfo[DPGIndex].bHasVisibleLitPrimitives) 
			{
				return TRUE;
			}
		}
		return FALSE;
	}
};

/** Information about a visible light which isn't view-specific. */
class FVisibleLightInfo
{
public:

	/** The visible projected shadows. */
	TArray<FProjectedShadowInfo*,SceneRenderingAllocator> ProjectedShadows;

	/** The primitives which this light casts a shadow volume from. */
	TArray<FPrimitiveSceneInfo*,SceneRenderingAllocator> ShadowVolumePrimitives;
};

/** 
* Set of sorted translucent scene prims  
*/
class FTranslucentPrimSet
{
public:

	/** 
	* Iterate over the sorted list of prims and draw them
	* @param View - current view used to draw items
	* @param DPGIndex - current DPG used to draw items
	* @param bSceneColorPass - TRUE if only the translucent prims that read from scene color should be drawn
	* @return TRUE if anything was drawn
	*/
	UBOOL Draw(const class FViewInfo* View,UINT DPGIndex,UBOOL bSceneColorPass);

	/**
	* Add a new primitive to the list of sorted prims
	* @param PrimitiveSceneInfo - primitive info to add. Origin of bounds is used for sort.
	* @param ViewInfo - used to transform bounds to view space
	* @param bUsesSceneColor - primitive samples from scene color
	*/
	void AddScenePrimitive(FPrimitiveSceneInfo* PrimitivieSceneInfo,const FViewInfo& ViewInfo, UBOOL bUsesSceneColor=FALSE);

	/**
	* Sort any primitives that were added to the set back-to-front
	*/
	void SortPrimitives();

	/** 
	* @return number of prims to render
	*/
	INT NumPrims() const
	{
		return SortedPrims.Num() + SortedSceneColorPrims.Num();
	}
	
	/** 
	* @return number of prims that read from scene color
	*/
	INT NumSceneColorPrims() const
	{
		return SortedSceneColorPrims.Num();
	}

	/** 
	* @return the interface to a primitive which requires scene color
	*/
	const FPrimitiveSceneInfo* GetSceneColorPrim(INT i)const
	{
		check(i>=0 && i<NumSceneColorPrims());
		return SortedSceneColorPrims(i).PrimitiveSceneInfo;
	}

private:
	/** contains a scene prim and its sort key */
	struct FSortedPrim
	{
		/** Default constructor. */
		FSortedPrim() {}

		FSortedPrim(FPrimitiveSceneInfo* InPrimitiveSceneInfo,FLOAT InSortKey,INT InSortPriority)
			:	PrimitiveSceneInfo(InPrimitiveSceneInfo)
			,	SortPriority(InSortPriority)
			,	SortKey(InSortKey)
		{

		}

		FPrimitiveSceneInfo* PrimitiveSceneInfo;
		INT SortPriority;
		FLOAT SortKey;
	};
	/** list of sorted translucent primitives */
	TArray<FSortedPrim,SceneRenderingAllocator> SortedPrims;
	/** list of sorted translucent primitives that use the scene color. These are drawn after all other translucent prims */
	TArray<FSortedPrim,SceneRenderingAllocator> SortedSceneColorPrims;
	/** sortkey compare class */
	IMPLEMENT_COMPARE_CONSTREF( FSortedPrim,TranslucentRender,
	{ 
		if (A.SortPriority == B.SortPriority)
		{
			// sort normally from back to front
			return A.SortKey <= B.SortKey ? 1 : -1;
		}

		// else lower sort priorities should render first
		return A.SortPriority > B.SortPriority ? 1 : -1; 
	} )
};

/** MotionBlur parameters */
struct FMotionBlurParameters
{
	FMotionBlurParameters()
		:	VelocityScale( 1.0f )
		,	MaxVelocity( 1.0f )
		,	bFullMotionBlur( TRUE )
		,	RotationThreshold( 45.0f )
		,	TranslationThreshold( 10000.0f )
	{
	}
	FLOAT VelocityScale;
	FLOAT MaxVelocity;
	UBOOL bFullMotionBlur;
	FLOAT RotationThreshold;
	FLOAT TranslationThreshold;
};

/** A batched occlusion primitive. */
struct FOcclusionPrimitive
{
	FVector Origin;
	FVector Extent;
};

/**
 * Combines consecutive primitives which use the same occlusion query into a single DrawIndexedPrimitive call.
 */
class FOcclusionQueryBatcher
{
public:

	/** The maximum number of consecutive previously occluded primitives which will be combined into a single occlusion query. */
	enum { OccludedPrimitiveQueryBatchSize = 8 };

	/** Initialization constructor. */
	FOcclusionQueryBatcher(class FSceneViewState* ViewState,UINT InMaxBatchedPrimitives);

	/** Destructor. */
	~FOcclusionQueryBatcher();

	/** Renders the current batch and resets the batch state. */
	void Flush();

	/**
	 * Batches a primitive's occlusion query for rendering.
	 * @param Bounds - The primitive's bounds.
	 */
	FOcclusionQueryRHIParamRef BatchPrimitive(const FBoxSphereBounds& Bounds);

private:

	/** The pending batches. */
	TArray<FOcclusionQueryRHIRef,SceneRenderingAllocator> BatchOcclusionQueries;

	/** The pending primitives. */
	TArray<FOcclusionPrimitive,SceneRenderingAllocator> Primitives;

	/** The batch new primitives are being added to. */
	FOcclusionQueryRHIParamRef CurrentBatchOcclusionQuery;

	/** The maximum number of primitives in a batch. */
	const UINT MaxBatchedPrimitives;

	/** The number of primitives in the current batch. */
	UINT NumBatchedPrimitives;

	/** The pool to allocate occlusion queries from. */
	class FOcclusionQueryPool* OcclusionQueryPool;
};

/** The actor visibility set that is passed back to the game thread when the scene rendering is done. */
class FActorVisibilitySet : public FActorVisibilityHistoryInterface
{
public:

	/**
	 * Adds an actor to the visibility set.  Ensures that duplicates are not added.
	 * @param VisibleActor - The actor which is visible.
	 */
	void AddActor(const AActor* VisibleActor)
	{
		VisibleActors.Add(VisibleActor);
	}

	// FActorVisibilityHistoryInterface
	virtual UBOOL GetActorVisibility(const AActor* TestActor) const
	{
		return VisibleActors.Find(TestActor) != NULL;
	}

	//@todo debug -
	UBOOL DebugVerifyHash(const AActor* VisibleActor);

private:

	// The set of visible actors.
	TSet<const AActor*,DefaultKeyFuncs<const AActor*>,TSetAllocator<TSparseArrayAllocator<>,TInlineAllocator<4096,FDefaultAllocator> > > VisibleActors;
};

/** A FSceneView with additional state used by the scene renderer. */
class FViewInfo : public FSceneView
{
public:

	/** A map from primitive ID to a boolean visibility value. */
	TBitArray<SceneRenderingBitArrayAllocator> PrimitiveVisibilityMap;

	/** A map from primitive ID to the primitive's view relevance. */
	TArray<FPrimitiveViewRelevance,SceneRenderingAllocator> PrimitiveViewRelevanceMap;

	/** A map from static mesh ID to a boolean visibility value. */
	TBitArray<SceneRenderingBitArrayAllocator> StaticMeshVisibilityMap;

	/** A map from static mesh ID to a boolean occluder value. */
	TBitArray<SceneRenderingBitArrayAllocator> StaticMeshOccluderMap;

	/** A map from static mesh ID to a boolean velocity visibility value. */
	TBitArray<SceneRenderingBitArrayAllocator> StaticMeshVelocityMap;

	/** A map from decal static mesh ID to a boolean visibility value. */
	TBitArray<SceneRenderingBitArrayAllocator> DecalStaticMeshVisibilityMap;

	/** The dynamic primitives visible in this view. */
	TArray<const FPrimitiveSceneInfo*,SceneRenderingAllocator> VisibleDynamicPrimitives;

	/** The dynamic opaque decal primitives visible in this view. */
	TArray<const FPrimitiveSceneInfo*,SceneRenderingAllocator> VisibleOpaqueDynamicDecalPrimitives[SDPG_MAX_SceneRender];
	/** The dynamic translucent decal primitives visible in this view. */
	TArray<const FPrimitiveSceneInfo*,SceneRenderingAllocator> VisibleTranslucentDynamicDecalPrimitives[SDPG_MAX_SceneRender];

	/** Set of translucent prims for this view - one for each DPG */
	FTranslucentPrimSet TranslucentPrimSet[SDPG_MAX_SceneRender];

	/** Set of distortion prims for this view - one for each DPG */
	FDistortionPrimSet DistortionPrimSet[SDPG_MAX_SceneRender];
	
	/** A map from light ID to a boolean visibility value. */
	TArray<FVisibleLightViewInfo,SceneRenderingAllocator> VisibleLightInfos;

	/** The view's batched elements, sorted by DPG. */
	FBatchedElements BatchedViewElements[SDPG_MAX_SceneRender];

	/** The view's mesh elements, sorted by DPG. */
	TIndirectArray<FHitProxyMeshPair> ViewMeshElements[SDPG_MAX_SceneRender];

	/** TRUE if the DPG has at least one mesh in ViewMeshElements[DPGIndex] with a translucent material. */
	BITFIELD bHasTranslucentViewMeshElements : SDPG_MAX_SceneRender;

	/** TRUE if the DPG has at least one mesh in ViewMeshElements[DPGIndex] with a distortion material. */
	BITFIELD bHasDistortionViewMeshElements : SDPG_MAX_SceneRender;

	/** The dynamic resources used by the view elements. */
	TArray<FDynamicPrimitiveResource*> DynamicResources;

	/** fog params for 4 layers of height fog */
	FLOAT FogMinHeight[4];
	FLOAT FogMaxHeight[4];
	FLOAT FogDistanceScale[4];
	FLOAT FogExtinctionDistance[4];
	FLinearColor FogInScattering[4];
	FLOAT FogStartDistance[4];

	/** Whether FSceneRenderer needs to output velocities during pre-pass. */
	BITFIELD bRequiresVelocities : 1;

	/** Whether the view should store the previous frame's transforms.  This is always true if bRequiresVelocities is true. */
	BITFIELD bRequiresPrevTransforms : 1;

	/** Indicates whether previous frame transforms were reset this frame for any reason. */
	BITFIELD bPrevTransformsReset : 1;

	/** Whether we should ignore queries from last frame (useful to ignoring occlusions on the first frame after a large camera movement). */
	BITFIELD bIgnoreExistingQueries : 1;

	/** Whether we should submit new queries this frame. (used to disable occlusion queries completely. */
	BITFIELD bDisableQuerySubmissions : 1;

	/** Whether one layer height fog has already been rendered with ambient occlusion. */
	BITFIELD bOneLayerHeightFogRenderedInAO : 1;

	/** Last frame's view and projection matrices, only tracked if bRequiresPrevTransforms is TRUE. */
	FMatrix					PrevViewProjMatrix;

	/** Last frame's view rotation and projection matrices, only tracked if bRequiresPrevTransforms is TRUE. */
	FMatrix					PrevViewRotationProjMatrix;

	/** Last frame's view origin, only tracked if bRequiresPrevTransforms is TRUE. */
	FVector					PrevViewOrigin;
	FMotionBlurParameters	MotionBlurParameters;

    /** Post process render proxies */
    TIndirectArray<FPostProcessSceneProxy> PostProcessSceneProxies;

	/** An intermediate number of visible static meshes.  Doesn't account for occlusion until after FinishOcclusionQueries is called. */
	INT NumVisibleStaticMeshElements;

	/** An intermediate number of visible dynamic primitives.  Doesn't account for occlusion until after FinishOcclusionQueries is called. */
	INT NumVisibleDynamicPrimitives;

	FOcclusionQueryBatcher IndividualOcclusionQueries;
	FOcclusionQueryBatcher GroupedOcclusionQueries;

	/** The actor visibility set for this view. */
	FActorVisibilitySet* ActorVisibilitySet;

	/** 
	 * Initialization constructor. Passes all parameters to FSceneView constructor
	 */
	FViewInfo(
		const FSceneViewFamily* InFamily,
		FSceneViewStateInterface* InState,
		INT InParentViewIndex,
		const FSceneViewFamily* InParentViewFamily,
		FSynchronizedActorVisibilityHistory* InHistory,
		const AActor* InViewActor,
		const UPostProcessChain* InPostProcessChain,
		const FPostProcessSettings* InPostProcessSettings,
		const FPostProcessMaskBase* InPostProcessMask,
		FViewElementDrawer* InDrawer,
		FLOAT InX,
		FLOAT InY,
		FLOAT InSizeX,
		FLOAT InSizeY,
		const FMatrix& InViewMatrix,
		const FMatrix& InProjectionMatrix,
		const FLinearColor& InBackgroundColor,
		const FLinearColor& InOverlayColor,
		const FLinearColor& InColorScale,
		const TSet<UPrimitiveComponent*>& InHiddenPrimitives,
		FLOAT InLODDistanceFactor = 1.0f
		);

	/** 
	* Initialization constructor. 
	* @param InView - copy to init with
	*/
	explicit FViewInfo(const FSceneView* InView);

	/** 
	* Destructor. 
	*/
	~FViewInfo();

	/** 
	* Initializes the dynamic resources used by this view's elements. 
	*/
	void InitDynamicResources();
};


/**
 * Used to hold combined stats for a shadow. In the case of shadow volumes the ShadowResolution remains
 * at INDEX_NONE and the Subjects array has a single entry. In the case of projected shadows the shadows
 * for the preshadow and subject are combined in this stat and so are primitives with a shadow parent.
 */
struct FCombinedShadowStats
{
	/** Array of shadow subjects. The first one is the shadow parent in the case of multiple entries.	*/
	FProjectedShadowInfo::PrimitiveArrayType	SubjectPrimitives;
	/** Array of preshadow primitives in the case of projected shadows.									*/
	FProjectedShadowInfo::PrimitiveArrayType	PreShadowPrimitives;
	/** Shadow resolution in the case of projected shadows, INDEX_NONE for shadow volumes.				*/
	INT									ShadowResolution;
	/** Shadow pass number in the case of projected shadows, INDEX_NONE for shadow volumes.				*/
	INT									ShadowPassNumber;

	/**
	 * Default constructor, initializing ShadowResolution for shadow volume case. 
	 */
	FCombinedShadowStats()
	:	ShadowResolution(INDEX_NONE)
	,	ShadowPassNumber(INDEX_NONE)
	{}
};

/**
* Global render state 
*/
class FGlobalSceneRenderState
{
public:
	FGlobalSceneRenderState() : FrameNumber(0) {}
	/** Incremented once per frame before the first scene is being rendered */
	UINT FrameNumber;
};

/**
 * Used as the scope for scene rendering functions.
 * It is initialized in the game thread by FSceneViewFamily::BeginRender, and then passed to the rendering thread.
 * The rendering thread calls Render(), and deletes the scene renderer when it returns.
 */
class FSceneRenderer
{
public:

	/** The scene being rendered. */
	FScene* Scene;

	/** The view family being rendered.  This references the Views array. */
	FSceneViewFamily ViewFamily;

	/** The views being rendered. */
	TArray<FViewInfo> Views;

	/** Information about the visible lights. */
	TArray<FVisibleLightInfo,SceneRenderingAllocator> VisibleLightInfos;
	
	/** The canvas transform used to render the scene. */
	FMatrix CanvasTransform;

	/** The width in screen pixels of the view family being rendered. */
	UINT FamilySizeX;

	/** The height in screen pixels of the view family being rendered. */
	UINT FamilySizeY;

	/** If a freeze request has been made */
	UBOOL bHasRequestedToggleFreeze;

	/** Whether to use a depth only pass before the base pass to maximize zcull efficiency. */
	UBOOL bUseDepthOnlyPass;

	/** Initialization constructor. */
	FSceneRenderer(const FSceneViewFamily* InViewFamily,FHitProxyConsumer* HitProxyConsumer,const FMatrix& InCanvasTransform);

	/** Destructor, stringifying stats if stats gathering was enabled. */
	~FSceneRenderer();

	/** Renders the view family. */
	void Render();

	/** Renders only the final post processing for the view */
	void RenderPostProcessOnly();

	/** Render the view family's hit proxies. */
	void RenderHitProxies();

	/** Renders the scene to capture target textures */
	void RenderSceneCaptures();

	/** 
	* Global state shared by all FSceneRender instances 
	* @return global state
	*/
	FGlobalSceneRenderState* GetGlobalSceneRenderState();

private:
	/** Visible shadow casting lights in any DPG. */
	TArray<const FLightSceneInfo*,SceneRenderingAllocator> VisibleShadowCastingLightInfos;

	/** Map from light primitive interaction to its combined shadow stats. Only used during stats gathering. */
	TMap<FLightPrimitiveInteraction*,FCombinedShadowStats,SceneRenderingSetAllocator> InteractionToDynamicShadowStatsMap;

	/** Whether we should be gathering dynamic shadow stats this frame. */
	UBOOL bShouldGatherDynamicShadowStats;

	/** The world time of the previous frame. */
	FLOAT PreviousFrameTime;

	/**
	 * Creates a projected shadow for light-primitive interaction.
	 * @param Interaction - The interaction to create a shadow for.
	 * @param OutPreShadows - If the primitive has a preshadow, CreateProjectedShadow adds it to OutPreShadows.
	 * @return new FProjectedShadowInfo if one was created
	 */
	FProjectedShadowInfo* CreateProjectedShadow(
		FLightPrimitiveInteraction* Interaction,
		TArray<FProjectedShadowInfo*,SceneRenderingAllocator>& OutPreShadows
		);

	/**
	 * Creates a projected shadow for all primitives affected by a light.  If the light doesn't support whole-scene shadows, it returns FALSE.
	 * @param LightSceneInfo - The light to create a shadow for.
	 * @return new FProjectedShadowInfo if one was created
	 */
	FProjectedShadowInfo* CreateWholeSceneProjectedShadow(FLightSceneInfo* LightSceneInfo);

	/** Gathers the list of primitives used to draw pre-shadows and modulate-better shadows. */
	void GatherShadowPrimitives(
		const TArray<FProjectedShadowInfo*,SceneRenderingAllocator>& ModulateBetterShadows,
		const TArray<FProjectedShadowInfo*,SceneRenderingAllocator>& PreShadows
		);

	/** Calculates projected shadow visibility. */
	void InitProjectedShadowVisibility();

	/** Finds the visible dynamic shadows for each view. */
	void InitDynamicShadows();

	/** Determines which primitives are visible for each view. */
	void InitViews(); 

	/** Initialized the fog constants for each view. */
	void InitFogConstants();

	/**
	 * Renders the scene's prepass and occlusion queries.
	 * If motion blur is enabled, it will also render velocities for dynamic primitives to the velocity buffer and
	 * flag those pixels in the stencil buffer.
	 * @param DPGIndex - current depth priority group index
	 * @param bIsOcclusionTesting - TRUE if testing occlusion
	 * @param ViewIndex - view to render; -1 for all views
	 * @return TRUE if anything was rendered
	 */
	UBOOL RenderPrePass(UINT DPGIndex,UBOOL bIsOcclusionTesting,UINT ViewIndex);

	/**
	 * Renders the scene's prepass and occlusion queries.
	 * Used by RenderPrePass
	 */
	UBOOL RenderPrePassInner(UINT DPGIndex,UBOOL bIsOcclusionTesting,UINT ViewIndex);

	/**
	 * Renders the prepass for the given DPG and View.
	 */
	UBOOL RenderDPGPrePass(UINT DPGIndex, FViewInfo& View);

	/** 
	* Renders the scene's base pass 
	*
	* @param DPGIndex - current depth priority group index
	* @param bIsTiledRendering - TRUE if currently within a Begin/End tiled rendering block
	* @return TRUE if anything was rendered
	*/
	UBOOL RenderBasePass(UINT DPGIndex, UBOOL bIsTiledRendering);

	/** 
	* Renders the scene's decals
	*
	* @param DPGIndex - current depth priority group index
	* @param bTranslucentPass - if TRUE render translucent decals on opqaue receivers
	*							if FALSE render opqaue decals on opaque/translucent receivers
	* @return TRUE if anything was rendered to scene color
	*/
	UBOOL RenderDecals(const FViewInfo& View, UINT DPGIndex, UBOOL bTranslucentPass);

	/** 
	* Issues occlusion tests if a depth pass was not rendered.
	*/
	void BeginOcclusionTests();

	/** Renders the scene's fogging. */
	UBOOL RenderFog(UINT DPGIndex);

	/** bound shader state for extracting LDR scene color from HDR in SM2 */
	static FGlobalBoundShaderState LDRExtractBoundShaderState;

	/** Renders the scene's lighting. */
	UBOOL RenderLights(UINT DPGIndex,UBOOL bAffectedByModulatedShadows, UBOOL bWasSceneColorDirty);

	/** Renders the scene's distortion */
	UBOOL RenderDistortion(UINT DPGIndex);
	
	/** 
	 * Renders the scene's translucency.
	 *
	 * @param	DPGIndex	Current DPG used to draw items.
	 * @return				TRUE if anything was drawn.
	 */
	UBOOL RenderTranslucency(UINT DPGIndex);

	/** 
	 * Extracts LDR scene color from HDR scene color
	 */
	void ExtractLDRSceneColor(UINT DPGIndex);

	/** Renders the velocities of movable objects for the motion blur effect. */
	void RenderVelocities(UINT DPGIndex);

	/** Renders world-space texture density instead of the normal color. */
	UBOOL RenderTextureDensities(UINT DPGIndex);

	/** bound shader state for occlusion test prims */
	static FGlobalBoundShaderState OcclusionTestBoundShaderState;

	/** Renders the post process effects for a view. */
	UBOOL RenderPostProcessEffects(UINT DPGIndex, UBOOL bAffectLightingOnly = FALSE);

	/**
	* Finish rendering a view, mapping accumulated shader complexity to a color.
	* @param View - The view to process.
	*/
	void RenderShaderComplexity(const FViewInfo* View);
	/** bound shader state for full-screen shader complexity apply pass */
	static FGlobalBoundShaderState ShaderComplexityBoundShaderState;

	/**
	 * Finish rendering a view, writing the contents to ViewFamily.RenderTarget.
	 * @param View - The view to process.
	*/
	void FinishRenderViewTarget(const FViewInfo* View);
	/** bound shader state for full-screen gamma correction pass */
	static FGlobalBoundShaderState PostProcessBoundShaderState;

	/**
	  * Used by RenderLights to figure out if projected shadows need to be rendered to the attenuation buffer.
	  *
	  * @param LightSceneInfo Represents the current light
	  * @return TRUE if anything needs to be rendered
	  */
	UBOOL CheckForProjectedShadows( const FLightSceneInfo* LightSceneInfo, UINT DPGIndex );

	/**
	  * Used by RenderLights to render projected shadows to the attenuation buffer.
	  *
	  * @param LightSceneInfo Represents the current light
	  * @return TRUE if anything got rendered
	  */
	UBOOL RenderProjectedShadows( const FLightSceneInfo* LightSceneInfo, UINT DPGIndex );

	/**
	  * Used by RenderLights to figure out if shadow volumes need to be rendered to the stencil buffer.
	  *
	  * @param LightSceneInfo Represents the current light
	  * @return TRUE if anything needs to be rendered
	  */
	UBOOL CheckForShadowVolumes( const FLightSceneInfo* LightSceneInfo, UINT DPGIndex );

	/**
	  * Used by RenderLights to render shadow volumes to the attenuation buffer.
	  *
	  * @param LightSceneInfo Represents the current light
	  * @param LightIndex The light's index into FScene::Lights
	  * @return TRUE if anything got rendered
	  */
	UBOOL RenderShadowVolumes( const FLightSceneInfo* LightSceneInfo, UINT DPGIndex );

	/**
	* Used by RenderLights to figure out if shadow volumes need to be rendered to the attenuation buffer.
	*
	* @param LightSceneInfo Represents the current light
	* @return TRUE if anything needs to be rendered
	*/
	UBOOL CheckForShadowVolumeAttenuation( const FLightSceneInfo* LightSceneInfo );

	/**
	* Attenuate the shadowed area of a shadow volume. For use with modulated shadows
	* @param LightSceneInfo - Represents the current light
	* @return TRUE if anything got rendered
	*/
	UBOOL AttenuateShadowVolumes( const FLightSceneInfo* LightSceneInfo );

	/**
	  * Used by RenderLights to figure out if light functions need to be rendered to the attenuation buffer.
	  *
	  * @param LightSceneInfo Represents the current light
	  * @return TRUE if anything got rendered
	  */
	UBOOL CheckForLightFunction( const FLightSceneInfo* LightSceneInfo, UINT DPGIndex );

	/**
	  * Used by RenderLights to render a light function to the attenuation buffer.
	  *
	  * @param LightSceneInfo Represents the current light
	  * @param LightIndex The light's index into FScene::Lights
	  * @return TRUE if anything got rendered
	  */
	UBOOL RenderLightFunction( const FLightSceneInfo* LightSceneInfo, UINT DPGIndex );

	/**
	  * Used by RenderLights to render a light to the scene color buffer.
	  *
	  * @param LightSceneInfo Represents the current light
	  * @param LightIndex The light's index into FScene::Lights
	  * @return TRUE if anything got rendered
	  */
	UBOOL RenderLight( const FLightSceneInfo* LightSceneInfo, UINT DPGIndex );

	/**
	 * Renders all the modulated shadows to the scene color buffer.
	 * @param	DPGIndex					Current DPG used to draw items.
	 * @return TRUE if anything got rendered
	 */
	UBOOL RenderModulatedShadows(UINT DPGIndex);

	/**
	* Clears the scene color depth (stored in alpha channel) to max depth
	* This is needed for depth bias blend materials to show up correctly
	*/
	void ClearSceneColorDepth();

	/** Saves the actor and primitive visibility states for the game thread. */
	void SaveVisibilityState();

	/** Helper used to set device viewports for Render*Pass */
	void ViewSetViewport(UINT ViewIndex, UBOOL bTiledRenderingPass, UBOOL bReverseZ );

	/** Helper used to compute the minimual screen bounds of all translucent primitives which require scene color */
	UBOOL ComputeTranslucencyResolveRectangle(INT DPGIndex, FIntRect& PixelRect);

	/** Helper used to compute the minimual screen bounds of all distortion primitives which require scene color */
	UBOOL ComputeDistortionResolveRectangle(INT DPGIndex, FIntRect& PixelRect);
};
