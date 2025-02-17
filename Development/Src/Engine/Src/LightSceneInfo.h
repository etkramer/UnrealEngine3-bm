/*=============================================================================
	LightSceneInfo.h: Light scene info definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __LIGHTSCENEINFO_H__
#define __LIGHTSCENEINFO_H__

/** An interface to the information about a light's effect on a scene's DPG. */
class FLightSceneDPGInfoInterface
{
public:

	enum ELightPassDrawListType
	{
		ELightPass_Default=0,
		ELightPass_Decals,
		ELightPass_MAX
	};

	virtual UBOOL DrawStaticMeshesVisible(
		const FViewInfo& View,
		const TBitArray<SceneRenderingBitArrayAllocator>& StaticMeshVisibilityMap,
		ELightPassDrawListType DrawType
		) const = 0;

	virtual UBOOL DrawStaticMeshesAll(
		const FViewInfo& View,
		ELightPassDrawListType DrawType
		) const = 0;

	virtual ELightInteractionType AttachStaticMesh(const FLightSceneInfo* LightSceneInfo,FStaticMesh* Mesh) = 0;

	virtual void DetachStaticMeshes() = 0;

	virtual UBOOL DrawDynamicMesh(
		const FSceneView& View,
		const FLightSceneInfo* LightSceneInfo,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		UBOOL bPreFog,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		FHitProxyId HitProxyId
		) const = 0;
};

/**
 * The information needed to cull a light-primitive interaction.
 */
class FLightSceneInfoCompact
{
public:

	FLightSceneInfo* LightSceneInfo;
	const ULightEnvironmentComponent* LightEnvironment;
	FLightingChannelContainer LightingChannels;
	VectorRegister BoundingSphereVector;
	FLinearColor Color;
	BITFIELD bStaticShadowing : 1;
	BITFIELD bCastDynamicShadow : 1;
	BITFIELD bCastStaticShadow : 1;
	BITFIELD bProjectedShadows : 1;
	BITFIELD bModulateBetterShadows : 1;
	BITFIELD bStaticLighting : 1;
	BITFIELD bCastCompositeShadow : 1;


	/** Initializes the compact scene info from the light's full scene info. */
	void Init(FLightSceneInfo* InLightSceneInfo);

	/** Default constructor. */
	FLightSceneInfoCompact():
		LightSceneInfo(NULL),
		LightEnvironment(NULL)
	{}

	/** Initialization constructor. */
	FLightSceneInfoCompact(FLightSceneInfo* InLightSceneInfo)
	{
		Init(InLightSceneInfo);
	}

	/**
	 * Tests whether this light affects the given primitive.  This checks both the primitive and light settings for light relevance
	 * and also calls AffectsBounds.
	 *
	 * @param CompactPrimitiveSceneInfo - The primitive to test.
	 * @return True if the light affects the primitive.
	 */
	UBOOL AffectsPrimitive(const FPrimitiveSceneInfoCompact& CompactPrimitiveSceneInfo) const;

	/**
	* Tests whether this light's modulated shadow affects the given primitive by doing a bounds check.
	*
	* @param CompactPrimitiveSceneInfo - The primitive to test.
	* @return True if the modulated shadow affects the primitive.
	*/
	UBOOL AffectsModShadowPrimitive(const FPrimitiveSceneInfoCompact& CompactPrimitiveSceneInfo) const;
};

/** The type of the octree used by FScene to find lights. */
typedef TOctree<FLightSceneInfoCompact,struct FLightOctreeSemantics> FSceneLightOctree;

/**
 * The information used to render a light.  This is the rendering thread's mirror of the game thread's ULightComponent.
 */
class FLightSceneInfo
{
public:
    /** The light component. */
    const ULightComponent* LightComponent;

	/** The light's persistent shadowing GUID. */
	FGuid LightGuid;

	/** The light's persistent lighting GUID. */
	FGuid LightmapGuid;

	/** A transform from world space into light space. */
	FMatrix WorldToLight;

	/** A transform from light space into world space. */
	FMatrix LightToWorld;

	/** The homogenous position of the light. */
	FVector4 Position;

	/** The light color. */
	FLinearColor Color;

	/** The light channels which this light affects. */
	const FLightingChannelContainer LightingChannels;

	/** The list of static primitives affected by the light. */
	FLightPrimitiveInteraction* StaticPrimitiveList;

	/** The list of dynamic primitives affected by the light. */
	FLightPrimitiveInteraction* DynamicPrimitiveList;

	/** The index of the primitive in Scene->Lights. */
	INT Id;

	/** The identifier for the primitive in Scene->PrimitiveOctree. */
	FOctreeElementId OctreeId;

	/** Light function parameters. */
	FVector	LightFunctionScale;
	const FMaterialRenderProxy* LightFunction;
	/** Bound shader state for this light's light function. This is mutable because it is cached on first use, possibly when const */
	mutable FBoundShaderStateRHIRef LightFunctionBoundShaderState; 

	/** True if the light will cast projected shadows from dynamic primitives. */
	const BITFIELD bProjectedShadows : 1;

	/** True if primitives will cache static lighting for the light. */
	const BITFIELD bStaticLighting : 1;

	/** True if primitives will cache static shadowing for the light. */
	const BITFIELD bStaticShadowing : 1;

	/** True if the light casts dynamic shadows. */
	const BITFIELD bCastDynamicShadow : 1;

	/** True if the light should cast shadow from primitives which use a composite light environment. */
	const BITFIELD bCastCompositeShadow : 1;

	/** True if the light casts static shadows. */
	const BITFIELD bCastStaticShadow : 1;

	/** Whether to only affect primitives that are in the same level/ share the same  GetOutermost() or are in the set of additionally specified ones. */
	const BITFIELD bOnlyAffectSameAndSpecifiedLevels : 1;

	/** True if the light's exclusion and inclusion volumes should be used to determine primitive relevance. */
	const BITFIELD bUseVolumes : 1;

	/** The light environment which the light is in. */
	const ULightEnvironmentComponent* LightEnvironment;

	/** The light type (ELightComponentType) */
	const BYTE LightType;

	/** Type of shadowing to apply for the light (ELightShadowMode) */
	BYTE LightShadowMode;

	/** Type of shadow projection to use for this light */
	const BYTE ShadowProjectionTechnique;

	/** Quality of shadow buffer filtering to use for this light's shadows */
	const BYTE ShadowFilterQuality;

	/**
	 * Override for min dimensions (in texels) allowed for rendering shadow subject depths.
	 * This also controls shadow fading, once the shadow resolution reaches MinShadowResolution it will be faded out completely.
	 * A value of 0 defaults to MinShadowResolution in SystemSettings.
	 */
	const INT MinShadowResolution;

	/**
	 * Override for max square dimensions (in texels) allowed for rendering shadow subject depths.
	 * A value of 0 defaults to MaxShadowResolution in SystemSettings.
	 */
	const INT MaxShadowResolution;

	/** 
	 * Resolution in texels below which shadows begin to be faded out. 
	 * Once the shadow resolution reaches MinShadowResolution it will be faded out completely.
	 * A value of 0 defaults to ShadowFadeResolution in SystemSettings.
	 */
	const INT ShadowFadeResolution;

	/** The name of the level the light is in. */
	FName LevelName;

	/** Array of other levels to affect if bOnlyAffectSameAndSpecifiedLevels is TRUE, own level always implicitly part of array. */
	TArray<FName> OtherLevelsToAffect;

	/** The light's exclusion volumes. */
	TArray<FConvexVolume> ExclusionConvexVolumes;

	/** The light's inclusion volumes. */
	TArray<FConvexVolume> InclusionConvexVolumes;

	/** Shadow color for modulating entire scene */
	const FLinearColor ModShadowColor;

	/** Time since the subject was last visible at which the mod shadow will fade out completely.  */
	FLOAT ModShadowFadeoutTime;

	/** Exponent that controls mod shadow fadeout curve. */
	FLOAT ModShadowFadeoutExponent;

	FName LightComponentName;
	FName GetLightName() const { return LightComponentName; }

	/** The scene the light is in. */
	FScene* Scene;

	// Accessors.
	FVector GetDirection() const { return FVector(WorldToLight.M[0][2],WorldToLight.M[1][2],WorldToLight.M[2][2]); }
	FVector GetOrigin() const { return LightToWorld.GetOrigin(); }
	FVector4 GetPosition() const { return Position; }
	FORCEINLINE FBoxCenterAndExtent GetBoundingBox() const
	{
		const FLOAT Extent = GetRadius();
		return FBoxCenterAndExtent(
			GetOrigin(),
			FVector(Extent,Extent,Extent)
			);
	}

	/** @return radius of the light */
	virtual FLOAT GetRadius() const { return FLT_MAX; }
	
	/** Initialization constructor. */
	FLightSceneInfo(const ULightComponent* InLight);
	virtual ~FLightSceneInfo() {}

	/** Adds the light to the scene. */
	void AddToScene();

	/** Removes the light from the scene. */
	void RemoveFromScene();

	/** Detaches the light from the primitives it affects. */
	void Detach();

	/**
	 * Tests whether the light affects the given bounding volume.
	 * @param Bounds - The bounding volume to test.
	 * @return True if the light affects the bounding volume
	 */
	virtual UBOOL AffectsBounds(const FBoxSphereBounds& Bounds) const
	{
		return TRUE;
	}

	virtual void DetachPrimitive(const FLightPrimitiveInteraction& Interaction) {}
	virtual void AttachPrimitive(const FLightPrimitiveInteraction& Interaction) {}

	/**
	 * Sets up a projected shadow initializer for shadows from the entire scene.
	 * @return True if the whole-scene projected shadow should be used.
	 */
	virtual UBOOL GetWholeSceneProjectedShadowInitializer(class FProjectedShadowInitializer& OutInitializer) const
	{
		return FALSE;
	}

	/**
	 * Sets up a projected shadow initializer for the given subject.
	 * @param SubjectBounds - The bounding volume of the subject.
	 * @param OutInitializer - Upon successful return, contains the initialization parameters for the shadow.
	 * @return True if a projected shadow should be cast by this subject-light pair.
	 */
	virtual UBOOL GetProjectedShadowInitializer(const FBoxSphereBounds& SubjectBounds,class FProjectedShadowInitializer& OutInitializer) const
	{
		return FALSE;
	}

	virtual void SetDepthBounds(const FSceneView* View) const
	{
	}

	virtual void SetScissorRect(const FSceneView* View) const
	{
	}

	/**
	 * Returns a pointer to the light type's DPG info object for the given DPG.
	 * @param DPGIndex - The index of the DPG to get the info object for.
	 * @return The DPG info interface.
	 */
	virtual const FLightSceneDPGInfoInterface* GetDPGInfo(UINT DPGIndex) const = 0;
	virtual FLightSceneDPGInfoInterface* GetDPGInfo(UINT DPGIndex) = 0;

	/**
	* @return modulated shadow projection pixel shader for this light type
	*/
	virtual class FShadowProjectionPixelShaderInterface* GetModShadowProjPixelShader() const = 0;
	
	/**
	* @return Branching PCF modulated shadow projection pixel shader for this light type
	*/
	virtual class FBranchingPCFProjectionPixelShaderInterface* GetBranchingPCFModProjPixelShader() const = 0;

	/**
	* @return modulated shadow projection pixel shader for this light type
	*/
	virtual class FModShadowVolumePixelShader* GetModShadowVolumeShader() const = 0;

	/**
	* @return modulated shadow projection bound shader state for this light type
	*/
	virtual FGlobalBoundShaderState* GetModShadowProjBoundShaderState() const = 0;
	/** Bound shader state for this light's modulated shadow projection. This is mutable because it is cached on first use, possibly when const */
	mutable FGlobalBoundShaderState ModShadowProjBoundShaderState;

#if SUPPORTS_VSM
	/**
	* @return VSM modulated shadow projection pixel shader for this light type
	*/
	virtual class FVSMModProjectionPixelShader* GetVSMModProjPixelShader() const = 0;
	/**
	* @return VSM modulated shadow projection bound shader state for this light type
	*/
	virtual FBoundShaderStateRHIParamRef GetVSMModProjBoundShaderState() const = 0;
	/** Bound shader state for this light's VSM modulated shadow projection. This is mutable because it is cached on first use, possibly when const */
	mutable FBoundShaderStateRHIRef VSMModProjBoundShaderState;
#endif //#if SUPPORTS_VSM

	/**
	* @return PCF Branching modulated shadow projection bound shader state for this light type
	*/
	virtual FGlobalBoundShaderState* GetBranchingPCFModProjBoundShaderState() const = 0;
	/** Bound shader state for this light's PCF Branching modulated shadow projection. This is mutable because it is cached on first use, possibly when const */
	mutable FGlobalBoundShaderState ModBranchingPCFLowQualityBoundShaderState;
	mutable FGlobalBoundShaderState ModBranchingPCFMediumQualityBoundShaderState;
	mutable FGlobalBoundShaderState ModBranchingPCFHighQualityBoundShaderState;

	/**
	* @return modulated shadow projection bound shader state for this light type
	*/
	virtual FBoundShaderStateRHIParamRef GetModShadowVolumeBoundShaderState() const = 0;
	/** Bound shader state for this light's light modulated shadow volume. This is mutable because it is cached on first use, possibly when const */
	mutable FBoundShaderStateRHIRef ModShadowVolumeBoundShaderState;

	/** Hash function. */
	friend DWORD GetTypeHash(const FLightSceneInfo* LightSceneInfo)
	{
		return (DWORD)LightSceneInfo->Id;
	}
};

/** Defines how the light is stored in the scene's light octree. */
struct FLightOctreeSemantics
{
	enum { MaxElementsPerLeaf = 16 };
	enum { MinInclusiveElementsPerNode = 7 };
	enum { MaxNodeDepth = 12 };

	FORCEINLINE static FBoxCenterAndExtent GetBoundingBox(const FLightSceneInfoCompact& Element)
	{
		return Element.LightSceneInfo->GetBoundingBox();
	}

	FORCEINLINE static UBOOL AreElementsEqual(const FLightSceneInfoCompact& A,const FLightSceneInfoCompact& B)
	{
		return A.LightSceneInfo == B.LightSceneInfo;
	}
	
	FORCEINLINE static void SetElementId(const FLightSceneInfoCompact& Element,FOctreeElementId Id)
	{
		Element.LightSceneInfo->OctreeId = Id;
	}
};

#endif
