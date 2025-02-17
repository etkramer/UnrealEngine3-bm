/*=============================================================================
	ScenePrivate.h: Private scene manager definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __SCENEPRIVATE_H__
#define __SCENEPRIVATE_H__

// Shortcuts for the allocators used by scene rendering.
class SceneRenderingAllocator
	: public TMemStackAllocator<GRenderingThreadMemStack>
{
};


class SceneRenderingBitArrayAllocator
	: public TInlineAllocator<4,SceneRenderingAllocator>
{
};

class SceneRenderingSparseArrayAllocator
	: public TSparseArrayAllocator<SceneRenderingAllocator,SceneRenderingBitArrayAllocator>
{
};

class SceneRenderingSetAllocator
	: public TSetAllocator<SceneRenderingSparseArrayAllocator,TInlineAllocator<1,SceneRenderingAllocator> >
{
};

// Forward declarations.
class FScene;

/** max DPG for scene rendering */
enum { SDPG_MAX_SceneRender = SDPG_PostProcess };

// Dependencies.
#include "StaticBoundShaderState.h"
#include "BatchedElements.h"
#include "SceneRenderTargets.h"
#include "GenericOctree.h"
#include "SceneCore.h"
#include "PrimitiveSceneInfo.h"
#include "LightSceneInfo.h"
#include "DrawingPolicy.h"
#include "StaticMeshDrawList.h"
#include "DepthRendering.h"
#include "SceneHitProxyRendering.h"
#include "ShaderComplexityRendering.h"
#include "FogRendering.h"
#include "FogVolumeRendering.h"
#include "BasePassRendering.h"
#include "ShadowRendering.h"
#include "ShadowVolumeRendering.h"
#include "VSMShadowRendering.h"
#include "BranchingPCFShadowRendering.h"
#include "DistortionRendering.h"
#include "SceneRendering.h"
#include "DynamicPrimitiveDrawing.h"
#include "TranslucentRendering.h"
#include "VelocityRendering.h"
#include "TextureDensityRendering.h"
#include "UnDecalRenderData.h"

/** Holds information about a single primitive's occlusion. */
class FPrimitiveOcclusionHistory
{
public:
	/** The primitive the occlusion information is about. */
	const UPrimitiveComponent* Primitive;

	/** The occlusion query which contains the primitive's pending occlusion results. */
	FOcclusionQueryRHIRef PendingOcclusionQuery;

	/** The last time the primitive was visible. */
	FLOAT LastVisibleTime;

	/** The last time the primitive was in the view frustum. */
	FLOAT LastConsideredTime;

	/** 
	 *	The pixels that were rendered the last time the primitive was drawn.
	 *	It is the ratio of pixels unoccluded to the resolution of the scene.
	 */
	FLOAT LastPixelsPercentage;

	/** whether or not this primitive was grouped the last time it was queried */
	UBOOL bGroupedQuery;

	/** Initialization constructor. */
	FPrimitiveOcclusionHistory(const UPrimitiveComponent* InPrimitive = NULL):
		Primitive(InPrimitive),
		LastVisibleTime(0.0f),
		LastConsideredTime(0.0f),
		LastPixelsPercentage(0.0f),
		bGroupedQuery(FALSE)
	{}

	/** Destructor. Note that the query should have been released already. */
	~FPrimitiveOcclusionHistory()
	{
//		check( !IsValidRef(PendingOcclusionQuery) );
	}
};

/** Defines how the hash set indexes the FPrimitiveOcclusionHistory objects. */
struct FPrimitiveOcclusionHistoryKeyFuncs : BaseKeyFuncs<FPrimitiveOcclusionHistory,const UPrimitiveComponent*>
{
	static KeyInitType GetSetKey(const FPrimitiveOcclusionHistory& Element)
	{
		return Element.Primitive;
	}

	static UBOOL Matches(KeyInitType A,KeyInitType B)
	{
		return A == B;
	}

	static DWORD GetKeyHash(KeyInitType Key)
	{
		return PointerHash(Key);
	}
};


/**
 * A pool of occlusion queries which are allocated individually, and returned to the pool as a group.
 */
class FOcclusionQueryPool
{
public:
	FOcclusionQueryPool()	{ }
	virtual ~FOcclusionQueryPool();

	/** Releases all the occlusion queries in the pool. */
	void Release();

	/** Allocates an occlusion query from the pool. */
	FOcclusionQueryRHIRef AllocateQuery();

	/** De-reference an occlusion query, returning it to the pool instead of deleting it when the refcount reaches 0. */
	void ReleaseQuery( FOcclusionQueryRHIRef &Query );

private:
	/** Container for available occlusion queries. */
	TArray<FOcclusionQueryRHIRef> OcclusionQueries;
};

/**
 * The scene manager's private implementation of persistent view state.
 * This class is associated with a particular camera across multiple frames by the game thread.
 * The game thread calls AllocateViewState to create an instance of this private implementation.
 */
class FSceneViewState : public FSceneViewStateInterface, public FDeferredCleanupInterface, public FRenderResource
{
public:

    class FProjectedShadowKey
    {
	public:

        FORCEINLINE UBOOL operator == (const FProjectedShadowKey &Other) const
        {
            return (Primitive == Other.Primitive && Light == Other.Light);
        }

        FProjectedShadowKey(const UPrimitiveComponent* InPrimitive,const ULightComponent* InLight):
            Primitive(InPrimitive),
            Light(InLight)
        {
		}

		friend FORCEINLINE DWORD GetTypeHash(const FSceneViewState::FProjectedShadowKey& Key)
		{
			return PointerHash(Key.Light,PointerHash(Key.Primitive));
		}

	private:
		const UPrimitiveComponent* Primitive;
        const ULightComponent* Light;
    };

    class FPrimitiveComponentKey
    {
	public:

        FORCEINLINE UBOOL operator == (const FPrimitiveComponentKey &Other)
        {
            return (Primitive == Other.Primitive);
        }

        FPrimitiveComponentKey(const UPrimitiveComponent* InPrimitive):
            Primitive(InPrimitive)
        {
        }

		friend FORCEINLINE DWORD GetTypeHash(const FSceneViewState::FPrimitiveComponentKey& Key)
		{
			return PointerHash(Key.Primitive);
		}

	private:
		
        const UPrimitiveComponent* Primitive;
    };

    TMap<FProjectedShadowKey,FOcclusionQueryRHIRef> ShadowOcclusionQueryMap;

	/** The view's occlusion query pool. */
	FOcclusionQueryPool OcclusionQueryPool;

	/** Parameter to keep track of previous frame. Managed by the rendering thread. */
	FMatrix		PendingPrevProjMatrix;
	FMatrix		PrevProjMatrix;
	FMatrix		PendingPrevViewMatrix;
	FMatrix		PrevViewMatrix;
	FVector		PendingPrevViewOrigin;
	FVector		PrevViewOrigin;
	FLOAT		LastRenderTime;
	FLOAT		MotionBlurTimeScale;
	FMatrix		PrevViewMatrixForOcclusionQuery;
	FVector		PrevViewOriginForOcclusionQuery;

	/** Used by states that have IsViewParent() == TRUE to store primitives for child states. */
	TSet<const UPrimitiveComponent*> ParentPrimitives;

#if !FINAL_RELEASE
	/** Are we currently in the state of freezing rendering? (1 frame where we gather what was rendered) */
	UBOOL bIsFreezing;

	/** Is rendering currently frozen? */
	UBOOL bIsFrozen;

	/** The set of primitives that were rendered the frame that we froze rendering */
	TSet<const UPrimitiveComponent*> FrozenPrimitives;
#endif

	/** Default constructor. */
    FSceneViewState()
    {
		LastRenderTime = 0.0f;
		MotionBlurTimeScale = 1.0f;
		PendingPrevProjMatrix.SetIdentity();
		PrevProjMatrix.SetIdentity();
		PendingPrevViewMatrix.SetIdentity();
		PrevViewMatrix.SetIdentity();
		PendingPrevViewOrigin = FVector(0,0,0);
		PrevViewOrigin = FVector(0,0,0);
		PrevViewMatrixForOcclusionQuery.SetIdentity();
		PrevViewOriginForOcclusionQuery = FVector(0,0,0);
#if !FINAL_RELEASE
		bIsFreezing = FALSE;
		bIsFrozen = FALSE;
#endif

		// Register this object as a resource, so it will receive device reset notifications.
		if ( IsInGameThread() )
		{
			BeginInitResource(this);
		}
		else
		{
			InitResource();
		}
    }

	/**
	 * Cleans out old entries from the primitive occlusion history, and resets unused pending occlusion queries.
	 * @param MinHistoryTime - The occlusion history for any primitives which have been visible and unoccluded since
	 *							this time will be kept.  The occlusion history for any primitives which haven't been
	 *							visible and unoccluded since this time will be discarded.
	 * @param MinQueryTime - The pending occlusion queries older than this will be discarded.
	 */
	void TrimOcclusionHistory(FLOAT MinHistoryTime,FLOAT MinQueryTime);

	/**
	 * Checks whether a primitive is occluded this frame.  Also updates the occlusion history for the primitive.
	 * @param CompactPrimitiveSceneInfo - The compact scene info for the primitive to check the occlusion state for.
	 * @param View - The frame's view corresponding to this view state.
	 * @param CurrentRealTime - The current frame's real time.
	 * @param bOutPrimitiveIsDefinitelyUnoccluded - Upon return contains true if the primitive was definitely un-occluded, and not merely
	 *			estimated to be un-occluded.
	 */
	UBOOL UpdatePrimitiveOcclusion(
		const FPrimitiveSceneInfoCompact& CompactPrimitiveSceneInfo,
		FViewInfo& View,
		FLOAT CurrentRealTime,
		UBOOL& bOutPrimitiveIsDefinitelyUnoccluded
		);

	/**
	 * Checks whether a shadow is occluded this frame.
	 * @param Primitive - The shadow subject.
	 * @param Light - The shadow source.
	 */
	UBOOL IsShadowOccluded(const UPrimitiveComponent* Primitive,const ULightComponent* Light) const;

	/**
	 *	Retrieves the percentage of the views render target the primitive touched last time it was rendered.
	 *
	 *	@param	Primitive				The primitive of interest.
	 *	@param	OutCoveragePercentage	REFERENCE: The screen coverate percentage. (OUTPUT)
	 *	@return	UBOOL					TRUE if the primitive was found and the results are valid, FALSE is not
	 */
	UBOOL GetPrimitiveCoveragePercentage(const UPrimitiveComponent* Primitive, FLOAT& OutCoveragePercentage);

    // FRenderResource interface.
    virtual void ReleaseDynamicRHI()
    {
        ShadowOcclusionQueryMap.Reset();
		PrimitiveOcclusionHistorySet.Empty();
		OcclusionQueryPool.Release();
    }

	// FSceneViewStateInterface
	virtual void Destroy()
	{
		if ( IsInGameThread() )
		{
			// Release the occlusion query data.
			BeginReleaseResource(this);

			// Defer deletion of the view state until the rendering thread is done with it.
			BeginCleanup(this);
		}
		else
		{
			ReleaseResource();
			FinishCleanup();
		}
	}
	
	// FDeferredCleanupInterface
	virtual void FinishCleanup()
	{
		delete this;
	}

private:

	/** Information about visibility/occlusion states in past frames for individual primitives. */
	TSet<FPrimitiveOcclusionHistory,FPrimitiveOcclusionHistoryKeyFuncs> PrimitiveOcclusionHistorySet;
};

class FDepthPriorityGroup
{
public:

	enum EBasePassDrawListType
	{
		EBasePass_Default=0,
		EBasePass_Masked,
		EBasePass_Decals,
		EBasePass_Decals_Translucent,
		EBasePass_MAX
	};

	// various static draw lists for this DPG

	/** position-only opaque depth draw list */
	TStaticMeshDrawList<FPositionOnlyDepthDrawingPolicy> PositionOnlyDepthDrawList;
	/** opaque depth draw list */
	TStaticMeshDrawList<FDepthDrawingPolicy> DepthDrawList;
	/** Base pass draw list - no light map */
	TStaticMeshDrawList<TBasePassDrawingPolicy<FNoLightMapPolicy,FNoDensityPolicy> > BasePassNoLightMapDrawList[EBasePass_MAX];
	/** Base pass draw list - directional vertex light maps */
	TStaticMeshDrawList<TBasePassDrawingPolicy<FDirectionalVertexLightMapPolicy,FNoDensityPolicy> > BasePassDirectionalVertexLightMapDrawList[EBasePass_MAX];
	/** Base pass draw list - simple vertex light maps */
	TStaticMeshDrawList<TBasePassDrawingPolicy<FSimpleVertexLightMapPolicy,FNoDensityPolicy> > BasePassSimpleVertexLightMapDrawList[EBasePass_MAX];
	/** Base pass draw list - directional texture light maps */
	TStaticMeshDrawList<TBasePassDrawingPolicy<FDirectionalLightMapTexturePolicy,FNoDensityPolicy> > BasePassDirectionalLightMapTextureDrawList[EBasePass_MAX];
	/** Base pass draw list - simple texture light maps */
	TStaticMeshDrawList<TBasePassDrawingPolicy<FSimpleLightMapTexturePolicy,FNoDensityPolicy> > BasePassSimpleLightMapTextureDrawList[EBasePass_MAX];
	/** Base pass draw list - directional light only */
	TStaticMeshDrawList<TBasePassDrawingPolicy<FDirectionalLightLightMapPolicy,FNoDensityPolicy> > BasePassDirectionalLightDrawList[EBasePass_MAX];
	/** Base pass draw list - SH light + directional light */
	TStaticMeshDrawList<TBasePassDrawingPolicy<FSHLightLightMapPolicy,FNoDensityPolicy> > BasePassSHLightDrawList[EBasePass_MAX];
	/** hit proxy draw list */
	TStaticMeshDrawList<FHitProxyDrawingPolicy> HitProxyDrawList;
	/** draw list for motion blur velocities */
	TStaticMeshDrawList<FVelocityDrawingPolicy> VelocityDrawList;

	/** Maps a light-map type to the appropriate base pass draw list. */
	template<typename LightMapPolicyType>
	TStaticMeshDrawList<TBasePassDrawingPolicy<LightMapPolicyType,FNoDensityPolicy> >& GetBasePassDrawList(EBasePassDrawListType DrawType);
};

/** Information about the primitives and lights in a light environment. */
class FLightEnvironmentSceneInfo
{
public:

	/** The primitives in the light environment. */
	TArray<FPrimitiveSceneInfo*> Primitives;

	/** The lights in the light environment. */
	TArray<FLightSceneInfo*,TInlineAllocator<3> > Lights;
};

/** Information about the primitives in a shadow group. */
class FShadowGroupSceneInfo
{
public:

	/** The primitives in the shadow group. */
	TArray<FPrimitiveSceneInfo*> Primitives;
};

/** 
 *	Helper structure for setting up portal scene capture information
 */
struct FSceneCaptureViewInfoData
{
	FVector PlayerViewOrigin;
	FLOAT PlayerViewScreenSize;
	FLOAT PlayerViewFOVScreenSize;
};

class FScene : public FSceneInterface
{
public:

	/** An optional world associated with the level. */
	UWorld* World;

	/** The draw lists for the scene, sorted by DPG and translucency layer. */
	FDepthPriorityGroup DPGs[SDPG_MAX_SceneRender];

	/** The primitives in the scene. */
	TSparseArray<FPrimitiveSceneInfo*> Primitives;

	/** The scene capture probes for rendering the scene to texture targets */
	TSparseArray<FCaptureSceneInfo*> SceneCapturesRenderThread;

	/** Game thread copy of scene capture probes for rendering the scene to texture targets */
	TSparseArray<FCaptureSceneInfo*> SceneCapturesGameThread;

	/** Game thread container for storing all attached fluidsurfaces. */
	TArray<UFluidSurfaceComponent*> FluidSurfaces;

	/** The lights in the scene. */
	TSparseArray<FLightSceneInfoCompact> Lights;

	/** The static meshes in the scene. */
	TSparseArray<FStaticMesh*> StaticMeshes;

	/** The decal static meshes in the scene. Added by each FDecalInteraction during attachment */
	TSparseArray<FStaticMesh*> DecalStaticMeshes;

	/** The fog components in the scene. */
	TArray<FHeightFogSceneInfo> Fogs;

	/** The wind sources in the scene. */
	TArray<class FWindSourceSceneProxy*> WindSources;

	/** Maps a primitive component to the fog volume density information associated with it. */
	TMap<const UPrimitiveComponent*, FFogVolumeDensitySceneInfo*> FogVolumes;

	/** The light environments in the scene. */
	TMap<const ULightEnvironmentComponent*,FLightEnvironmentSceneInfo> LightEnvironments;

	/** The shadow groups in the scene.  The map key is the shadow group's parent primitive. */
	TMap<const UPrimitiveComponent*,FShadowGroupSceneInfo> ShadowGroups;

	/** An octree containing the lights in the scene. */
	FSceneLightOctree LightOctree;

	/** An octree containing the primitives in the scene. */
	FScenePrimitiveOctree PrimitiveOctree;

	/** Indicates this scene always allows audio playback. */
	UBOOL bAlwaysAllowAudioPlayback;

	/** Indicates whether this scene requires hit proxy rendering. */
	UBOOL bRequiresHitProxies;

	/** Set by the rendering thread to signal to the game thread that the scene needs a static lighting build. */
	volatile mutable INT NumUncachedStaticLightingInteractions;

	/** Default constructor. */
	FScene()
	:	LightOctree(FVector(0,0,0),HALF_WORLD_MAX)
	,	PrimitiveOctree(FVector(0,0,0),HALF_WORLD_MAX)
	,	NumUncachedStaticLightingInteractions(0)
	{}

	// FSceneInterface interface.
	virtual void AddPrimitive(UPrimitiveComponent* Primitive);
	virtual void RemovePrimitive(UPrimitiveComponent* Primitive);
	virtual void UpdatePrimitiveTransform(UPrimitiveComponent* Primitive);
	virtual void AddLight(ULightComponent* Light);
	virtual void RemoveLight(ULightComponent* Light);
	virtual void UpdateLightTransform(ULightComponent* Light);
	virtual void UpdateLightColorAndBrightness(ULightComponent* Light);
	virtual void AddHeightFog(UHeightFogComponent* FogComponent);
	virtual void RemoveHeightFog(UHeightFogComponent* FogComponent);
	virtual void AddWindSource(UWindDirectionalSourceComponent* WindComponent);
	virtual void RemoveWindSource(UWindDirectionalSourceComponent* WindComponent);
	virtual const TArray<FWindSourceSceneProxy*>& GetWindSources_RenderThread() const;

	/**
	 * Adds a default FFogVolumeConstantDensitySceneInfo to UPrimitiveComponent pair to the Scene's FogVolumes map.
	 */
	virtual void AddFogVolume(const UPrimitiveComponent* VolumeComponent);

	/**
	 * Adds a FFogVolumeDensitySceneInfo to UPrimitiveComponent pair to the Scene's FogVolumes map.
	 */
	virtual void AddFogVolume(const UFogVolumeDensityComponent* FogVolumeComponent, const UPrimitiveComponent* MeshComponent);

	/**
	 * Removes an entry by UPrimitiveComponent from the Scene's FogVolumes map.
	 */
	virtual void RemoveFogVolume(const UPrimitiveComponent* VolumeComponent);

	/**
	 * Retrieves the lights interacting with the passed in primitive and adds them to the out array.
	 *
	 * @param	Primitive				Primitive to retrieve interacting lights for
	 * @param	RelevantLights	[out]	Array of lights interacting with primitive
	 */
	virtual void GetRelevantLights( UPrimitiveComponent* Primitive, TArray<const ULightComponent*>* RelevantLights ) const;

	/**
	 * Create the scene capture info for a capture component and add it to the scene
	 * @param CaptureComponent - component to add to the scene 
	 */
	virtual void AddSceneCapture(USceneCaptureComponent* CaptureComponent);

	/**
	 * Remove the scene capture info for a capture component from the scene
	 * @param CaptureComponent - component to remove from the scene 
	 */
	virtual void RemoveSceneCapture(USceneCaptureComponent* CaptureComponent);

	/**
	 * Adds a fluidsurface to the scene (gamethread)
	 * @param FluidComponent - component to add to the scene 
	 */
	virtual void AddFluidSurface(UFluidSurfaceComponent* FluidComponent);

	/**
	 * Removes a fluidsurface from the scene (gamethread)
	 * @param CaptureComponent - component to remove from the scene 
	 */
	virtual void RemoveFluidSurface(UFluidSurfaceComponent* FluidComponent);

	/**
	 * Retrieves a pointer to the fluidsurface container.
	 * @return TArray pointer, or NULL if the scene doesn't support fluidsurfaces.
	 **/
	virtual const TArray<UFluidSurfaceComponent*>* GetFluidSurfaces();

	virtual void Release();
	virtual UWorld* GetWorld() const { return World; }

	/** Indicates if sounds in this should be allowed to play. */
	virtual UBOOL AllowAudioPlayback();

	/**
	 * @return		TRUE if hit proxies should be rendered in this scene.
	 */
	virtual UBOOL RequiresHitProxies() const;

	/**
	 * Accesses the scene info for a light environment.
	 * @param	LightEnvironment - The light environment to access scene info for.
	 * @return	The scene info for the light environment.
	 */
	FLightEnvironmentSceneInfo& GetLightEnvironmentSceneInfo(const ULightEnvironmentComponent* LightEnvironment);

	/**
	* Return the scene to be used for rendering
	*/
	virtual FSceneInterface* GetRenderScene()
	{
		return this;
	}

	/** 
	 *	Set the primitives motion blur info
	 * 
	 *	@param PrimitiveSceneInfo	The primitive to add
	 *	@param	bRemoving			TRUE if the primitive is being removed
	 */
	static void AddPrimitiveMotionBlur(FPrimitiveSceneInfo* PrimitiveSceneInfo, UBOOL bRemoving);

	/**
	 *	Clear out the motion blur info. Call this once per frame.
	 */
	static void ClearMotionBlurInfo();

	/** 
	 *	Get the primitives motion blur info
	 * 
	 *	@param	PrimitiveSceneInfo	The primitive to retrieve the motion blur info for
	 *	@param	MotionBlurInfo		The pointer to set to the motion blur info for the primitive (OUTPUT)
	 *
	 *	@return	UBOOL				TRUE if the primitive info was found and set
	 */
	static UBOOL GetPrimitiveMotionBlurInfo(const FPrimitiveSceneInfo* PrimitiveSceneInfo, const FMotionBlurInfo*& MBInfo);

	/**
	 *	Add the scene captures view info to the streaming manager
	 *
	 *	@param	StreamingManager			The streaming manager to add the view info to.
	 *	@param	View						The scene view information
	 */
	virtual void AddSceneCaptureViewInformation(FStreamingManagerCollection* StreamingManager, FSceneView* View);

	/**
	 * Dumps dynamic lighting and shadow interactions for scene to log.
	 *
	 * @param	bOnlyIncludeShadowCastingInteractions	Whether to only include shadow casting interactions
	 */
	virtual void DumpDynamicLightShadowInteractions( UBOOL bOnlyIncludeShadowCastingInteractions ) const;

private:

	/**
	 * Retrieves the lights interacting with the passed in primitive and adds them to the out array.
	 * Render thread version of function.
	 * @param	Primitive				Primitive to retrieve interacting lights for
	 * @param	RelevantLights	[out]	Array of lights interacting with primitive
	 */
	void GetRelevantLights_RenderThread( UPrimitiveComponent* Primitive, TArray<const ULightComponent*>* RelevantLights ) const;

	/**
	 * Adds a primitive to the scene.  Called in the rendering thread by AddPrimitive.
	 * @param PrimitiveSceneInfo - The primitive being added.
	 */
	void AddPrimitiveSceneInfo_RenderThread(FPrimitiveSceneInfo* PrimitiveSceneInfo);

	/**
	 * Removes a primitive from the scene.  Called in the rendering thread by RemovePrimitive.
	 * @param PrimitiveSceneInfo - The primitive being removed.
	 */
	void RemovePrimitiveSceneInfo_RenderThread(FPrimitiveSceneInfo* PrimitiveSceneInfo);

	/**
	 * Adds a light to the scene.  Called in the rendering thread by AddLight.
	 * @param LightSceneInfo - The light being added.
	 */
	void AddLightSceneInfo_RenderThread(FLightSceneInfo* LightSceneInfo);

	/**
	 * Removes a light from the scene.  Called in the rendering thread by RemoveLight.
	 * @param LightSceneInfo - The light being removed.
	 */
	void RemoveLightSceneInfo_RenderThread(FLightSceneInfo* LightSceneInfo);

	/**
	 * Dumps dynamic lighting and shadow interactions for scene to log.
	 *
	 * @param	bOnlyIncludeShadowCastingInteractions	Whether to only include shadow casting interactions
	 */
	void DumpDynamicLightShadowInteractions_RenderThread( UBOOL bOnlyIncludeShadowCastingInteractions ) const;

	/** The motion blur info entries for the frame. Accessed on Renderthread only! */
	static TArray<FMotionBlurInfo> MotionBlurInfoArray;
};

/** The scene update stats. */
enum
{
	STAT_AddScenePrimitiveRenderThreadTime = STAT_SceneUpdateFirstStat,
	STAT_AddScenePrimitiveGameThreadTime,
	STAT_AddSceneLightTime,
	STAT_RemoveScenePrimitiveTime,
	STAT_RemoveSceneLightTime,
	STAT_UpdatePrimitiveTransformGameThreadTime,
	STAT_UpdatePrimitiveTransformRenderThreadTime,
	STAT_SceneCaptureComponentTickTime,
	STAT_LineBatchComponentTickTime,
};

#endif // __SCENEPRIVATE_H__
