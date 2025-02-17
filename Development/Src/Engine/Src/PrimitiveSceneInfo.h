/*=============================================================================
	PrimitiveSceneInfo.h: Primitive scene info definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __PRIMITIVESCENEINFO_H__
#define __PRIMITIVESCENEINFO_H__

#include "GenericOctree.h"

/** The information needed to determine whether a primitive is visible. */
class FPrimitiveSceneInfoCompact
{
public:

	FPrimitiveSceneInfo* PrimitiveSceneInfo;
	FPrimitiveSceneProxy* Proxy;
	UPrimitiveComponent* Component;
	ULightEnvironmentComponent* LightEnvironment;
	FBoxSphereBounds Bounds;
	FLOAT MinDrawDistance;
	FLOAT MaxDrawDistance;
	FLightingChannelContainer LightingChannels;

	BITFIELD bAllowApproximateOcclusion : 1;
	BITFIELD bFirstFrameOcclusion : 1;
	BITFIELD bAcceptsLights : 1;
	BITFIELD bHasViewDependentDPG : 1;
	BITFIELD bShouldCullModulatedShadows : 1;
	BITFIELD bCastDynamicShadow : 1;
	BITFIELD bLightEnvironmentForceNonCompositeDynamicLights : 1;
	BITFIELD bIgnoreNearPlaneIntersection : 1;
	BITFIELD bAlwaysVisible : 1;

	BITFIELD StaticDepthPriorityGroup : UCONST_SDPG_NumBits;

	/** Initializes the compact scene info from the primitive's full scene info. */
	void Init(FPrimitiveSceneInfo* InPrimitiveSceneInfo);

	/** Default constructor. */
	FPrimitiveSceneInfoCompact():
		PrimitiveSceneInfo(NULL),
		Proxy(NULL),
		Component(NULL),
		LightEnvironment(NULL)
	{}

	/** Initialization constructor. */
	FPrimitiveSceneInfoCompact(FPrimitiveSceneInfo* InPrimitiveSceneInfo)
	{
		Init(InPrimitiveSceneInfo);
	}
};

/** The type of the octree used by FScene to find primitives. */
typedef TOctree<FPrimitiveSceneInfoCompact,struct FPrimitiveOctreeSemantics> FScenePrimitiveOctree;

/**
 * The information used to a render a primitive.  This is the rendering thread's mirror of the game thread's UPrimitiveComponent.
 */
class FPrimitiveSceneInfo : public FDeferredCleanupInterface
{
public:

	/** The render proxy for the primitive. */
	FPrimitiveSceneProxy* Proxy;

	/** The UPrimitiveComponent this scene info is for. */
	UPrimitiveComponent* Component;

	/** The actor which owns the PrimitiveComponent. */
	AActor* Owner;

	/** The primitive's static meshes. */
	TIndirectArray<class FStaticMesh> StaticMeshes;

	/** The index of the primitive in Scene->Primitives. */
	INT Id;

	/** The identifier for the primitive in Scene->PrimitiveOctree. */
	FOctreeElementId OctreeId;

	/** The translucency sort priority */
	INT TranslucencySortPriority;

	/** True if the primitive will cache static shadowing. */
	BITFIELD bStaticShadowing : 1;

	/** True if the primitive casts dynamic shadows. */
	BITFIELD bCastDynamicShadow : 1;
	
	/** True if the primitive only self shadows and does not cast shadows on other primitives. */
	BITFIELD bSelfShadowOnly : 1;

	/** True if the primitive casts static shadows. */
	BITFIELD bCastStaticShadow : 1;

	/** True if the primitive casts shadows even when hidden. */
	BITFIELD bCastHiddenShadow : 1;

	/** True if the primitive receives lighting. */
	BITFIELD bAcceptsLights : 1;

	/** True if the primitive should be affected by dynamic lights. */
	BITFIELD bAcceptsDynamicLights : 1;

	/** True if the primitive should only be affected by lights in the same level. */
	BITFIELD bSelfContainedLighting : 1;

	/** If this is True, this primitive will be used to occlusion cull other primitives. */
	BITFIELD bUseAsOccluder:1;

	/** If this is True, this primitive doesn't need exact occlusion info. */
	BITFIELD bAllowApproximateOcclusion : 1;

	/** If this is True, the component will return 'occluded' for the first frame. */
	BITFIELD bFirstFrameOcclusion:1;

	/** Whether to ignore the near plane intersection test before occlusion querying. */
	BITFIELD bIgnoreNearPlaneIntersection : 1;

	/** Forces the primitive to always pass renderer visibility tests. */
	BITFIELD bAlwaysVisible : 1;

	/** If this is True, this primitive can be selected in the editor. */
	BITFIELD bSelectable : 1;

	/** If this is TRUE, this primitive's static meshes need to be updated before it can be rendered. */
	BITFIELD bNeedsStaticMeshUpdate : 1;

	/** 
	* If TRUE, the primitive backfaces won't allow for modulated shadows to be cast on them. 
	* If FALSE, could help performance since the mesh doesn't have to be drawn again to cull the backface shadows 
	*/
	BITFIELD	bCullModulatedShadowOnBackfaces:1;
	/** 
	* If TRUE, the emissive areas of the primitive won't allow for modulated shadows to be cast on them. 
	* If FALSE, could help performance since the mesh doesn't have to be drawn again to cull the emissive areas in shadow
	*/
	BITFIELD	bCullModulatedShadowOnEmissive:1;

	/**
	* Controls whether ambient occlusion should be allowed on or from this primitive, only has an effect on movable primitives.
	* Note that setting this flag to FALSE will negatively impact performance.
	*/
	BITFIELD	bAllowAmbientOcclusion:1;

	/** Whether to render SHLightSceneInfo in the BasePass instead of as a separate pass after modulated shadows. */
	BITFIELD	bRenderSHLightInBasePass : 1;

	/** Whether motionblur is enabled for this primitive or not. */
	BITFIELD	bEnableMotionBlur : 1;

	/** The value of the Component->LightEnvironment->bForceNonCompositeDynamicLights; or TRUE if it has no light environment. */
	BITFIELD	bLightEnvironmentForceNonCompositeDynamicLights : 1;

	/** The primitive's bounds. */
	FBoxSphereBounds Bounds;

	/** The primitive's cull distance. */
	FLOAT MaxDrawDistance;

	/** The primitive's minimum cull distance. */
	FLOAT MinDrawDistance;

	/** The hit proxies used by the primitive. */
	TArray<TRefCountPtr<HHitProxy> > HitProxies;

	/** The ID of the hit proxy which is used to represent the primitive's dynamic elements. */
	FHitProxyId DefaultDynamicHitProxyId;

	/** The light channels which this primitive is affected by. */
	const FLightingChannelContainer LightingChannels;

	/** The light environment which the primitive is in. */
	ULightEnvironmentComponent* LightEnvironment;

	/** The name of the level the primitive is in. */
	FName LevelName;

	/** The list of lights affecting this primitive. */
	class FLightPrimitiveInteraction* LightList;

	/** The aggregate light color of the upper sky light hemispheres affecting this primitive. */
	FLinearColor UpperSkyLightColor;

	/** The aggregate light color of the lower sky light hemispheres affecting this primitive. */
	FLinearColor LowerSkyLightColor;

	/** The directional light to be rendered in the base pass, if any. */
	const class FLightSceneInfo* DirectionalLightSceneInfo;

	/** The spherical harmonic light to be rendered in the base pass, if any. */
	const class FLightSceneInfo* SHLightSceneInfo;

	/** A primitive which this primitive is grouped with for projected shadows. */
	UPrimitiveComponent* ShadowParent;

	/** The fog volume this primitive is intersecting with. */
	class FFogVolumeDensitySceneInfo* FogVolumeSceneInfo;

	/** Last render time in seconds since level started play. */
	FLOAT LastRenderTime;

	/** Last time that the primitive became visible in seconds since level started play. */
	FLOAT LastVisibilityChangeTime;

	/** The scene the primitive is in. */
	FScene* Scene;

	/** Initialization constructor. */
	FPrimitiveSceneInfo(UPrimitiveComponent* InPrimitive,FPrimitiveSceneProxy* InProxy,FScene* InScene);

	/** Destructor. */
	~FPrimitiveSceneInfo();

	/** Adds the primitive to the scene. */
	void AddToScene();

	/** Removes the primitive from the scene. */
	void RemoveFromScene();

	/** Updates the primitive's static meshes in the scene. */
	void ConditionalUpdateStaticMeshes();

	/** Sets a flag to update the primitive's static meshes before it is next rendered. */
	void BeginDeferredUpdateStaticMeshes();

	/** Links the primitive to its shadow parent. */
	void LinkShadowParent();

	/** Unlinks the primitive from its shadow parent. */
	void UnlinkShadowParent();

	// FDeferredCleanupInterface
	virtual void FinishCleanup();

	/** Size this class uses in bytes */
	UINT GetMemoryFootprint();

	/** Determines whether the primitive has dynamic sky lighting. */
	UBOOL HasDynamicSkyLighting() const
	{
		return (!UpperSkyLightColor.Equals(FLinearColor::Black) || !LowerSkyLightColor.Equals(FLinearColor::Black));
	}
};

/** Defines how the primitive is stored in the scene's primitive octree. */
struct FPrimitiveOctreeSemantics
{
	enum { MaxElementsPerLeaf = 16 };
	enum { MinInclusiveElementsPerNode = 7 };
	enum { MaxNodeDepth = 12 };

	FORCEINLINE static FBoxCenterAndExtent GetBoundingBox(const FPrimitiveSceneInfoCompact& PrimitiveSceneInfoCompact)
	{
		return FBoxCenterAndExtent(PrimitiveSceneInfoCompact.Bounds);
	}

	FORCEINLINE static UBOOL AreElementsEqual(const FPrimitiveSceneInfoCompact& A,const FPrimitiveSceneInfoCompact& B)
	{
		return A.PrimitiveSceneInfo == B.PrimitiveSceneInfo;
	}

	FORCEINLINE static void SetElementId(const FPrimitiveSceneInfoCompact& Element,FOctreeElementId Id)
	{
		Element.PrimitiveSceneInfo->OctreeId = Id;
	}
};

#endif
