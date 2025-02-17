/*=============================================================================
	PrimitiveComponent.h: Primitive component definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Forward declarations.
class FDecalInteraction;
class FDecalRenderData;
class FPrimitiveSceneInfo;
class UDecalComponent;

/**
 * Encapsulates the data which is mirrored to render a primitive parallel to the game thread.
 */
class FPrimitiveSceneProxy
{
public:

	/** optional flags used for DrawDynamicElements */
	enum EDrawDynamicElementFlags
	{		
		/** mesh elements with dynamic data can be drawn */
		DontAllowDynamicMeshElementData = 1<<0,
		/** mesh elements without dynamic data can be drawn */
		DontAllowStaticMeshElementData = 1<<1,
	};

	/** Initialization constructor. */
	FPrimitiveSceneProxy(const UPrimitiveComponent* InComponent, FName ResourceName = NAME_None);

	/** Virtual destructor. */
	virtual ~FPrimitiveSceneProxy();

	/**
	 * Creates the hit proxies are used when DrawDynamicElements is called.
	 * Called in the game thread.
	 * @param OutHitProxies - Hit proxes which are created should be added to this array.
	 * @return The hit proxy to use by default for elements drawn by DrawDynamicElements.
	 */
	virtual HHitProxy* CreateHitProxies(const UPrimitiveComponent* Component,TArray<TRefCountPtr<HHitProxy> >& OutHitProxies);

	/**
	 * Draws the primitive's static elements.  This is called from the game thread once when the scene proxy is created.
	 * The static elements will only be rendered if GetViewRelevance declares static relevance.
	 * Called in the game thread.
	 * @param PDI - The interface which receives the primitive elements.
	 */
	virtual void DrawStaticElements(FStaticPrimitiveDrawInterface* PDI) {}

	/**
	 * Draws the primitive's dynamic elements.  This is called from the rendering thread for each frame of each view.
	 * The dynamic elements will only be rendered if GetViewRelevance declares dynamic relevance.
	 * Called in the rendering thread.
	 * @param PDI - The interface which receives the primitive elements.
	 * @param View - The view which is being rendered.
	 * @param InDepthPriorityGroup - The DPG which is being rendered.
	 * @param Flags - optional set of flags from EDrawDynamicElementFlags
	 */
	virtual void DrawDynamicElements(
		FPrimitiveDrawInterface* PDI,
		const FSceneView* View,
		UINT InDepthPriorityGroup,
		DWORD Flags=0
		) {}

	virtual void InitLitDecalFlags(UINT InDepthPriorityGroup) {}

	/**
	 * Adds a decal interaction to the primitive.  This is called in the rendering thread by AddDecalInteraction_GameThread.
	 */
	virtual void AddDecalInteraction_RenderingThread(const FDecalInteraction& DecalInteraction);

	/**
	 * Adds a decal interaction to the primitive.  This simply sends a message to the rendering thread to call AddDecalInteraction_RenderingThread.
	 * This is called in the game thread as new decal interactions are created.
	 */
	void AddDecalInteraction_GameThread(const FDecalInteraction& DecalInteraction);

	/**
	 * Removes a decal interaction from the primitive.  This is called in the rendering thread by RemoveDecalInteraction_GameThread.
	 */
	virtual void RemoveDecalInteraction_RenderingThread(UDecalComponent* DecalComponent);

	/**
	* Removes a decal interaction from the primitive.  This simply sends a message to the rendering thread to call RemoveDecalInteraction_RenderingThread.
	* This is called in the game thread when a decal is detached from a primitive which has been added to the scene.
	*/
	void RemoveDecalInteraction_GameThread(UDecalComponent* DecalComponent);

	/** 
	* Rebuilds the static mesh elements for decals that have a missing FStaticMesh* entry for their interactions
	* only called on the game thread
	*/
	void BuildMissingDecalStaticMeshElements_GameThread();

	/**
	* Rebuilds the static mesh elements for decals that have a missing FStaticMesh* entry for their interactions
	* enqued by BuildMissingDecalStaticMeshElements_GameThread on the render thread
	*/
	virtual void BuildMissingDecalStaticMeshElements_RenderThread();

	/**
	* Draws the primitive's static decal elements.  This is called from the game thread whenever this primitive is attached
	* as a receiver for a decal.
	*
	* The static elements will only be rendered if GetViewRelevance declares both static and decal relevance.
	* Called in the game thread.
	*
	* @param PDI - The interface which receives the primitive elements.
	*/
	virtual void DrawStaticDecalElements(FStaticPrimitiveDrawInterface* PDI,const FDecalInteraction& DecalInteraction) {}

	/**
	 * Draws the primitive's dynamic decal elements.  This is called from the rendering thread for each frame of each view.
	 * The dynamic elements will only be rendered if GetViewRelevance declares dynamic relevance.
	 * Called in the rendering thread.
	 *
	 * @param	PDI						The interface which receives the primitive elements.
	 * @param	View					The view which is being rendered.
	 * @param	InDepthPriorityGroup	The DPG which is being rendered.
	 * @param	bDynamicLightingPass	TRUE if drawing dynamic lights, FALSE if drawing static lights.
	 * @param	bTranslucentReceiverPass	TRUE during the decal pass for translucent receivers, FALSE for opaque receivers.
	 */
	virtual void DrawDynamicDecalElements(
		FPrimitiveDrawInterface* PDI,
		const FSceneView* View,
		UINT InDepthPriorityGroup,
		UBOOL bDynamicLightingPass,
		UBOOL bTranslucentReceiverPass
		) {}

	/**
	 * Draws the primitive's shadow volumes.  This is called from the rendering thread,
	 * in the FSceneRenderer::RenderLights phase.
	 * @param SVDI - The interface which performs the actual rendering of a shadow volume.
	 * @param View - The view which is being rendered.
	 * @param Light - The light for which shadows should be drawn.
	 * @param DPGIndex - The depth priority group the light is being drawn for.
	 */
	virtual void DrawShadowVolumes(FShadowVolumeDrawInterface* SVDI,const FSceneView* View,const FLightSceneInfo* Light,UINT DPGIndex) {}

	/**
	 * Removes potentially cached shadow volume data for the passed in light.
	 *
	 * @param Light		The light for which cached shadow volume data will be removed.
	 */
	virtual void RemoveCachedShadowVolumeData( const FLightSceneInfo* Light ) {}

	/**
	 * Determines the relevance of this primitive's elements to the given view.
	 * Called in the rendering thread.
	 * @param View - The view to determine relevance for.
	 * @return The relevance of the primitive's elements to the view.
	 */
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View);

	/**
	 *	Called during FSceneRenderer::InitViews for view processing on scene proxies before rendering them
	 *  Only called for primitives that are visible and have bDynamicRelevance
 	 *
	 *	@param	ViewFamily		The ViewFamily to pre-render for
	 *	@param	VisibilityMap	A BitArray that indicates whether the primitive was visible in that view (index)
	 *	@param	FrameNumber		The frame number of this pre-render
	 */
	virtual void PreRenderView(const FSceneViewFamily* ViewFamily, const TBitArray<FDefaultBitArrayAllocator>& VisibilityMap, INT FrameNumber) {}

	/**
	 *	Determines the relevance of this primitive's elements to the given light.
	 *	@param	LightSceneInfo			The light to determine relevance for
	 *	@param	bDynamic (output)		The light is dynamic for this primitive
	 *	@param	bRelevant (output)		The light is relevant for this primitive
	 *	@param	bLightMapped (output)	The light is light mapped for this primitive
	 */
	virtual void GetLightRelevance(FLightSceneInfo* LightSceneInfo, UBOOL& bDynamic, UBOOL& bRelevant, UBOOL& bLightMapped)
	{
		// Determine the lights relevance to the primitive.
		bDynamic = TRUE;
		bRelevant = TRUE;
		bLightMapped = FALSE;
	}

	/**
	 * @return		TRUE if the primitive has decals with lit materials that should be rendered in the given view.
	 */
	UBOOL HasLitDecals(const FSceneView* View) const;

	/**
	 *	Called when the rendering thread adds the proxy to the scene.
	 *	This function allows for generating renderer-side resources.
	 *	Called in the rendering thread.
	 */
	virtual UBOOL CreateRenderThreadResources()
	{
		return TRUE;
	}

	/**
	 * Called by the rendering thread to notify the proxy when a light is no longer
	 * associated with the proxy, so that it can clean up any cached resources.
	 * @param Light - The light to be removed.
	 */
	virtual void OnDetachLight(const FLightSceneInfo* Light)
	{
	}

	/**
	 * Called to notify the proxy when its transform has been updated.
	 * Called in the thread that owns the proxy; game or rendering.
	 */
	virtual void OnTransformChanged()
	{
	}

	/**
	* @return TRUE if the proxy requires occlusion queries
	*/
	virtual UBOOL RequiresOcclusion(const FSceneView* View) const
	{
		return TRUE;
	}

	/**
	 * Updates the primitive proxy's cached transforms, and calls OnUpdateTransform to notify it of the change.
	 * Called in the thread that owns the proxy; game or rendering.
	 * @param InLocalToWorld - The new local to world transform of the primitive.
	 * @param InLocalToWorldDeterminant - The new local to world transform's determinant.
	 */
	void SetTransform(const FMatrix& InLocalToWorld,FLOAT InLocalToWorldDeterminant);

	/**
	 * Returns whether the owning actor is movable or not.
	 * @return TRUE if the owning actor is movable
	 */
	UBOOL IsMovable() const
	{
		return bMovable;
	}

	/**
	 * Checks if the primitive is owned by the given actor.
	 * @param Actor - The actor to check for ownership.
	 * @return TRUE if the primitive is owned by the given actor.
	 */
	UBOOL IsOwnedBy(const AActor* Actor) const
	{
		return Owners.FindItemIndex(Actor) != INDEX_NONE;
	}

	/** @return TRUE if the primitive is in different DPGs depending on view. */
	UBOOL HasViewDependentDPG() const
	{
		return bUseViewOwnerDepthPriorityGroup;
	}

	/**
	 * Determines the DPG to render the primitive in regardless of view.
	 * Should only be called if HasViewDependentDPG()==TRUE.
	 */
	BYTE GetStaticDepthPriorityGroup() const
	{
		check(!HasViewDependentDPG());
		return StaticDepthPriorityGroup;
	}

	/**
	 * Determines the DPG to render the primitive in for the given view.
	 * May be called regardless of the result of HasViewDependentDPG.
	 * @param View - The view to determine the primitive's DPG for.
	 * @return The DPG the primitive should be rendered in for the given view.
	 */
	BYTE GetDepthPriorityGroup(const FSceneView* View) const
	{
		return (bUseViewOwnerDepthPriorityGroup && IsOwnedBy(View->ViewActor)) ?
			ViewOwnerDepthPriorityGroup :
			StaticDepthPriorityGroup;
	}

	/** @return The local to world transform of the primitive. */
	const FMatrix& GetLocalToWorld() const
	{
		return LocalToWorld;
	}

	/** Every derived class should override these functions */
	virtual EMemoryStats GetMemoryStatType( void ) const = 0;
	virtual DWORD GetMemoryFootprint( void ) const = 0;
	DWORD GetAllocatedSize( void ) const { return( Owners.GetAllocatedSize() ); }

protected:

	/** Pointer back to the PrimitiveSceneInfo that owns this Proxy. */
	class FPrimitiveSceneInfo *	PrimitiveSceneInfo;
	friend class FPrimitiveSceneInfo;

	/** The primitive's local to world transform. */
	FMatrix LocalToWorld;

	/** The determinant of the local to world transform. */
	FLOAT LocalToWorldDeterminant;
public:
	/** The decals which interact with the primitive. */
	TArray<FDecalInteraction*> Decals;

	/** The name of the resource used by the component. */
	FName ResourceName;

protected:
	/** @return True if the primitive is visible in the given View. */
	UBOOL IsShown(const FSceneView* View) const;
	/** @return True if the primitive is casting a shadow. */
	UBOOL IsShadowCast(const FSceneView* View) const;

	/** @return True if the primitive has decals with static relevance which should be rendered in the given view. */
	UBOOL HasRelevantStaticDecals(const FSceneView* View) const;
	/** @return True if the primitive has decals with dynamic relevance which should be rendered in the given view. */
	UBOOL HasRelevantDynamicDecals(const FSceneView* View) const;

protected:

	BITFIELD bHiddenGame : 1;
	BITFIELD bHiddenEditor : 1;
	BITFIELD bIsNavigationPoint : 1;
	BITFIELD bOnlyOwnerSee : 1;
	BITFIELD bOwnerNoSee : 1;
	BITFIELD bMovable : 1;

	/** TRUE if ViewOwnerDepthPriorityGroup should be used. */
	BITFIELD bUseViewOwnerDepthPriorityGroup : 1;

	/** DPG this prim belongs to. */
	BITFIELD StaticDepthPriorityGroup : UCONST_SDPG_NumBits;

	/** DPG this primitive is rendered in when viewed by its owner. */
	BITFIELD ViewOwnerDepthPriorityGroup : UCONST_SDPG_NumBits;

	TArray<const AActor*> Owners;

	FLOAT MaxDrawDistance;
};

/** Information about a vertex of a primitive's triangle. */
struct FPrimitiveTriangleVertex
{
	FVector WorldPosition;
	FVector WorldTangentX;
	FVector WorldTangentY;
	FVector WorldTangentZ;
};

/** An interface to some consumer of the primitive's triangles. */
class FPrimitiveTriangleDefinitionInterface
{
public:

	/**
	 * Defines a triangle.
	 * @param Vertex0 - The triangle's first vertex.
	 * @param Vertex1 - The triangle's second vertex.
	 * @param Vertex2 - The triangle's third vertex.
	 * @param StaticLighting - The triangle's static lighting information.
	 */
	virtual void DefineTriangle(
		const FPrimitiveTriangleVertex& Vertex0,
		const FPrimitiveTriangleVertex& Vertex1,
		const FPrimitiveTriangleVertex& Vertex2
		) = 0;
};

//
// FForceApplicator - Callback interface to apply a force field to a component(eg Rigid Body, Cloth, Particle System etc)
// Used by AddForceField.
//

class FForceApplicator
{
public:
	
	/**
	Called to compute a number of forces resulting from a force field.

	@param Positions Array of input positions (in world space)
	@param PositionStride Number of bytes between consecutive position vectors
	@param Velocities Array of inpute velocities
	@param VelocityStride Number of bytes between consecutive velocity vectors
	@param OutForce Output array of force vectors, computed from position/velocity, forces are added to the existing value
	@param OutForceStride Number of bytes between consectutiive output force vectors
	@param Count Number of forces to compute
	@param Scale factor to apply to positions before use(ie positions may not be in unreal units)
	@param PositionBoundingBox Bounding box for the positions passed to the call

	@return TRUE if any force is non zero.
	*/

	virtual UBOOL ComputeForce(
		FVector* Positions, INT PositionStride, FLOAT PositionScale,
		FVector* Velocities, INT VelocityStride, FLOAT VelocityScale,
		FVector* OutForce, INT OutForceStride, FLOAT OutForceScale,
		FVector* OutTorque, INT OutTorqueStride, FLOAT OutTorqueScale,
		INT Count, const FBox& PositionBoundingBox) = 0;
};

/** Information about a streaming texture that a primitive uses for rendering. */
struct FStreamingTexturePrimitiveInfo
{
	UTexture* Texture;
	FSphere Bounds;
	FLOAT TexelFactor;
};

//
//	UPrimitiveComponent
//

class UPrimitiveComponent : public UActorComponent
{
	DECLARE_ABSTRACT_CLASS(UPrimitiveComponent,UActorComponent,CLASS_NoExport,Engine);
public:

	static INT CurrentTag;

	/** The primitive's scene info. */
	class FPrimitiveSceneInfo* SceneInfo;

	/** A fence to track when the primitive is detached from the scene in the rendering thread. */
	FRenderCommandFence DetachFence;

	FLOAT LocalToWorldDeterminant;
	FMatrix LocalToWorld;
	INT MotionBlurInfoIndex;

	/** Current list of active decals attached to the primitive */
	TArray<FDecalInteraction*> DecalList;
	/** Decals that are detached from the primitive and need to be reattached */
	TArray<UDecalComponent*> DecalsToReattach;

	INT Tag;

	UPrimitiveComponent* ShadowParent;

	/** Keeps track of which fog component this primitive is using. */
	class UFogVolumeDensityComponent* FogVolumeComponent;

	FBoxSphereBounds Bounds;

	/** The lighting environment to take the primitive's lighting from. */
	class ULightEnvironmentComponent* LightEnvironment;

private:
	/** Stores the previous light environment if SetLightEnvironment is called while the primitive is attached, so that Detach can notify the previous light environment correctly. */
	ULightEnvironmentComponent* PreviousLightEnvironment;

public:
	/**
	 * The minimum distance at which the primitive should be rendered, 
	 * measured in world space units from the center of the primitive's bounding sphere to the camera position.
	 */
	FLOAT MinDrawDistance;

	/** 
	 * Max draw distance exposed to LDs. The real max draw distance is the min (disregarding 0) of this and volumes affecting this object. 
	 * This is renamed to LDMaxDrawDistance in c++
	 */
	FLOAT LDMaxDrawDistance;

	/**
	 * The distance to cull this primitive at.  
	 * A CachedMaxDrawDistance of 0 indicates that the primitive should not be culled by distance.
	 */
	FLOAT CachedMaxDrawDistance;

	/** Legacy, renamed to LDMaxDrawDistance */
	FLOAT LDCullDistance;
	/** Legacy, renamed to CachedMaxDrawDistance */
	FLOAT CachedCullDistance;

	/** The scene depth priority group to draw the primitive in. */
	BYTE DepthPriorityGroup;

	/** The scene depth priority group to draw the primitive in, if it's being viewed by its owner. */
	BYTE ViewOwnerDepthPriorityGroup;

	/** If detail mode is >= system detail mode, primitive won't be rendered. */
	BYTE DetailMode;
	
	/** Scalar controlling the amount of motion blur to be applied when object moves */
	FLOAT	MotionBlurScale;

	/** True if the primitive should be rendered using ViewOwnerDepthPriorityGroup if viewed by its owner. */
	BITFIELD	bUseViewOwnerDepthPriorityGroup:1 GCC_BITFIELD_MAGIC;

	/** Whether to accept cull distance volumes to modify cached cull distance. */
	BITFIELD	bAllowCullDistanceVolume:1;

	BITFIELD	HiddenGame:1;
	BITFIELD	HiddenEditor:1;

	/** If this is True, this component won't be visible when the view actor is the component's owner, directly or indirectly. */
	BITFIELD	bOwnerNoSee:1;

	/** If this is True, this component will only be visible when the view actor is the component's owner, directly or indirectly. */
	BITFIELD	bOnlyOwnerSee:1;

	/** If true, bHidden on the Owner of this component will be ignored. */
	BITFIELD	bIgnoreOwnerHidden : 1;

	/** If this is True, this primitive will be used to occlusion cull other primitives. */
	BITFIELD	bUseAsOccluder:1;

	/** If this is True, this component doesn't need exact occlusion info. */
	BITFIELD	bAllowApproximateOcclusion:1;

	/** If this is True, the component will return 'occluded' for the first frame. */
	BITFIELD	bFirstFrameOcclusion:1;

	/** If True, this component will still be queried for occlusion even when it intersects the near plane. */
	BITFIELD	bIgnoreNearPlaneIntersection:1;

	/** Forces the primitive to always pass renderer visibility tests. */
	BITFIELD	bAlwaysVisible:1;

	/** If this is True, this component can be selected in the editor. */
	BITFIELD	bSelectable:1;

	/** If TRUE, forces mips for textures used by this component to be resident when this component's level is loaded. */
	BITFIELD     bForceMipStreaming:1;

	/** deprecated */
	BITFIELD	bAcceptsDecals:1;

	/** deprecated */
	BITFIELD	bAcceptsDecalsDuringGameplay:1;

	/** If TRUE, this primitive accepts static level placed decals in the editor. */
	BITFIELD	bAcceptsStaticDecals:1;

	/** If TRUE, this primitive accepts dynamic decals spawned during gameplay.  */
	BITFIELD	bAcceptsDynamicDecals:1;

	BITFIELD	bIsRefreshingDecals:1;

	BITFIELD	bAllowDecalAutomaticReAttach:1;

	/** If TRUE, this primitive accepts foliage. */
	BITFIELD	bAcceptsFoliage:1;

	/** 
	* Translucent objects with a lower sort priority draw before objects with a higher priority.
	* Translucent objects with the same priority are rendered from back-to-front based on their bounds origin.
	*
	* Ignored if the object is not translucent.
	* The default priority is zero. 
	**/
	INT TranslucencySortPriority;

	// Lighting flags

	BITFIELD	CastShadow:1;

	/** If true, forces all static lights to use light-maps for direct lighting on this primitive, regardless of the light's UseDirectLightMap property. */
	BITFIELD	bForceDirectLightMap : 1;
	
	/** If true, primitive casts dynamic shadows. */
	BITFIELD	bCastDynamicShadow : 1;
	/** If true, primitive only self shadows and does not cast shadows on other primitives. */
	BITFIELD	bSelfShadowOnly : 1;
	/** If TRUE, primitive will cast shadows even if bHidden is TRUE. */
	BITFIELD	bCastHiddenShadow:1;
	
	BITFIELD	bAcceptsLights:1;
	
	/** Whether this primitives accepts dynamic lights */
	BITFIELD	bAcceptsDynamicLights:1;

	/** Lighting channels controlling light/ primitive interaction. Only allows interaction if at least one channel is shared */
	FLightingChannelContainer	LightingChannels;

	/** Whether the primitive supports/ allows static shadowing */
	BITFIELD	bUsePrecomputedShadows:1;

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

	// Collision flags.

	BITFIELD	CollideActors:1;
	BITFIELD	AlwaysCheckCollision:1;
	BITFIELD	BlockActors:1;
	BITFIELD	BlockZeroExtent:1;
	BITFIELD	BlockNonZeroExtent:1;
	BITFIELD	CanBlockCamera:1;
	BITFIELD	BlockRigidBody:1;

	/** 
	 *	Disables collision between any rigid body physics in this Component and pawn bodies. 
	 *	DEPRECATED! Use RBChannel/RBCollideWithChannels instead now
	 */
	BITFIELD	RigidBodyIgnorePawns:1;

	/** Enum indicating what type of object this should be considered for rigid body collision. */
	BYTE		RBChannel GCC_BITFIELD_MAGIC;

	/** Types of objects that this physics objects will collide with. */
	FRBCollisionChannelContainer RBCollideWithChannels;

	/** Never create any physics engine representation for this body. */
	BITFIELD	bDisableAllRigidBody:1;

	/** When creating rigid body, will skip normal geometry creation step, and will rely on ModifyNxActorDesc to fill in geometry. */
	BITFIELD	bSkipRBGeomCreation:1;

	/** Flag that indicates if OnRigidBodyCollision function should be called for physics collisions involving this PrimitiveComponent. */
	BITFIELD	bNotifyRigidBodyCollision:1;

	// Novodex fluids
	BITFIELD	bFluidDrain:1;
	BITFIELD	bFluidTwoWay:1;

	BITFIELD	bIgnoreRadialImpulse:1;
	BITFIELD	bIgnoreRadialForce:1;

	/** Disables the influence from ALL types of force fields. */
	BITFIELD	bIgnoreForceField:1;

	
	/** Place into a NxCompartment that will run in parallel with the primary scene's physics with potentially different simulation parameters.
	 *  If double buffering is enabled in the WorldInfo then physics will run in parallel with the entire game for this component. */
	BITFIELD	bUseCompartment:1;	// hardware scene support

	BITFIELD	AlwaysLoadOnClient:1;
	BITFIELD	AlwaysLoadOnServer:1;

	BITFIELD	bIgnoreHiddenActorsMembership:1;

	BITFIELD							bWasSNFiltered:1;
	TArrayNoInit<class FOctreeNode*>	OctreeNodes;
	
	class UPhysicalMaterial*	PhysMaterialOverride;
	class URB_BodyInstance*		BodyInstance;

	/** 
	 *	Used for creating one-way physics interactions (via constraints or contacts) 
	 *	Groups with lower RBDominanceGroup push around higher values in a 'one way' fashion. Must be <32.
	 */
	BYTE		RBDominanceGroup;

	// Copied from TransformComponent
	FMatrix CachedParentToWorld;
	FVector		Translation;
	FRotator	Rotation;
	FLOAT		Scale;
	FVector		Scale3D;
	BITFIELD	AbsoluteTranslation:1;
	BITFIELD	AbsoluteRotation:1;
	BITFIELD	AbsoluteScale:1;

	/** Last time the component was submitted for rendering (called FScene::AddPrimitive). */
	FLOAT		LastSubmitTime;

	/** Last render time in seconds since level started play. Updated to WorldInfo->TimeSeconds so float is sufficient. */
	FLOAT		LastRenderTime;

	/** if > 0, the script RigidBodyCollision() event will be called on our Owner when a physics collision involving
	 * this PrimitiveComponent occurs and the relative velocity is greater than or equal to this
	 */
	FLOAT ScriptRigidBodyCollisionThreshold;

	/**
	* Check if this primitive needs to be rendered for masking modulated shadows
	* @return TRUE if modulated shadows should be culled based on emissive or backfaces
	*/
	FORCEINLINE UBOOL ShouldCullModulatedShadows()
	{
		return bCullModulatedShadowOnEmissive || bCullModulatedShadowOnBackfaces;
	}

	// Should this Component be in the Octree for collision
	UBOOL ShouldCollide() const;

	void AttachDecal(class UDecalComponent* Decal, class FDecalRenderData* RenderData, const class FDecalState* DecalState);
	void DetachDecal(class UDecalComponent* Decal);

	/** Creates the render data for a decal on this primitive. */
	virtual class FDecalRenderData* GenerateDecalRenderData(class FDecalState* Decal) const;

	/**
	 * Returns True if a primitive cannot move or be destroyed during gameplay, and can thus cast and receive static shadowing.
	 */
	UBOOL HasStaticShadowing() const;

	/**
	 * Returns whether this primitive only uses unlit materials.
	 *
	 * @return TRUE if only unlit materials are used for rendering, false otherwise.
	 */
	virtual UBOOL UsesOnlyUnlitMaterials() const;

	/**
	 * Returns the lightmap resolution used for this primivite instnace in the case of it supporting texture light/ shadow maps.
	 * 0 if not supported or no static shadowing.
	 *
	 * @param Width		[out]	Width of light/shadow map
	 * @param Height	[out]	Height of light/shadow map
	 */
	virtual void GetLightMapResolution( INT& Width, INT& Height ) const;

	/**
	 * Returns the light and shadow map memory for this primite in its out variables.
	 *
	 * Shadow map memory usage is per light whereof lightmap data is independent of number of lights, assuming at least one.
	 *
	 * @param [out] LightMapMemoryUsage		Memory usage in bytes for light map (either texel or vertex) data
	 * @param [out]	ShadowMapMemoryUsage	Memory usage in bytes for shadow map (either texel or vertex) data
	 */
	virtual void GetLightAndShadowMapMemoryUsage( INT& LightMapMemoryUsage, INT& ShadowMapMemoryUsage ) const;

	/**
	 * Validates the lighting channels and makes adjustments as appropriate.
	 */
	void ValidateLightingChannels();

	/**
	 * Function that gets called from within Map_Check to allow this actor to check itself
	 * for any potential errors and register them with map check dialog.
	 */
	virtual void CheckForErrors();

	/**
	 * Sets Bounds.  Default behavior is a bounding box/sphere the size of the world.
	 */
	virtual void UpdateBounds();

	/**
	 * Requests the information about the component that the static lighting system needs.
	 * @param OutPrimitiveInfo - Upon return, contains the component's static lighting information.
	 * @param InRelevantLights - The lights relevant to the primitive.
	 * @param InOptions - The options for the static lighting build.
	 */
	virtual void GetStaticLightingInfo(FStaticLightingPrimitiveInfo& OutPrimitiveInfo,const TArray<ULightComponent*>& InRelevantLights,const FLightingBuildOptions& Options) {}

	/**
	 * Gets the primitive's static triangles.
	 * @param PTDI - An implementation of the triangle definition interface.
	 */
	virtual void GetStaticTriangles(FPrimitiveTriangleDefinitionInterface* PTDI) const {}

	/**
	 * Enumerates the streaming textures used by the primitive.
	 * @param OutStreamingTextures - Upon return, contains a list of the streaming textures used by the primitive.
	 */
	virtual void GetStreamingTextureInfo(TArray<FStreamingTexturePrimitiveInfo>& OutStreamingTextures) const
	{}

	/**
	 * Determines the DPG the primitive's primary elements are drawn in.
	 * Even if the primitive's elements are drawn in multiple DPGs, a primary DPG is needed for occlusion culling and shadow projection.
	 * @return The DPG the primitive's primary elements will be drawn in.
	 */
	virtual BYTE GetStaticDepthPriorityGroup() const
	{
		return DepthPriorityGroup;
	}

	// Collision tests.

	virtual UBOOL PointCheck(FCheckResult& Result,const FVector& Location,const FVector& Extent,DWORD TraceFlags) { return 1; }
	virtual UBOOL LineCheck(FCheckResult& Result,const FVector& End,const FVector& Start,const FVector& Extent,DWORD TraceFlags) { return 1; }

	FVector GetOrigin() const
	{
		return LocalToWorld.GetOrigin();
	}

	/** Removes any scaling from the LocalToWorld matrix and returns it, along with the overall scaling. */
	void GetTransformAndScale(FMatrix& OutTransform, FVector& OutScale);

	// Rigid-Body Physics

	virtual void InitComponentRBPhys(UBOOL bFixed);
	virtual void SetComponentRBFixed(UBOOL bFixed);
	virtual void TermComponentRBPhys(FRBPhysScene* InScene);

	/** Return the BodySetup to use for this PrimitiveComponent (single body case) */
	virtual class URB_BodySetup* GetRBBodySetup() { return NULL; }

	/** Returns any pre-cooked convex mesh data associated with this PrimitiveComponent. Used by InitBody. */
	virtual FKCachedConvexData* GetCachedPhysConvexData(const FVector& InScale3D) { return NULL; }

	/** 
	 * Returns any pre-cooked convex mesh data associated with this PrimitiveComponent.
	 * UPrimitiveComponent calls GetCachedPhysConvexData(InScale3D).
	 * USkeletalMeshComponent returns data associated with the specified bone.
	 * Used by InitBody. 
	 */
	virtual FKCachedConvexData* GetBoneCachedPhysConvexData(const FVector& InScale3D, const FName& BoneName) 
	{
		// Even if (BoneName != NAME_None) we call GetCachedPhysConvexData(InScale3D) 
		// to be fully compatible with existing behavior.
		return GetCachedPhysConvexData(InScale3D);
	}

	class URB_BodySetup* FindRBBodySetup();
	virtual void AddImpulse(FVector Impulse, FVector Position = FVector(0,0,0), FName BoneName = NAME_None, UBOOL bVelChange=false);
	virtual void AddRadialImpulse(const FVector& Origin, FLOAT Radius, FLOAT Strength, BYTE Falloff, UBOOL bVelChange=false);
	void AddForce(FVector Force, FVector Position = FVector(0,0,0), FName BoneName = NAME_None);
	virtual void AddRadialForce(const FVector& Origin, FLOAT Radius, FLOAT Strength, BYTE Falloff);
	void AddTorque(FVector Torque, FName BoneName = NAME_None);

	/** Applies the affect of a force field to this primitive component. */
	virtual void AddForceField(FForceApplicator* Applicator, const FBox& FieldBoundingBox, UBOOL bApplyToCloth, UBOOL bApplyToRigidBody);

	virtual void SetRBLinearVelocity(const FVector& NewVel, UBOOL bAddToCurrent=false);
	virtual void SetRBAngularVelocity(const FVector& NewAngVel, UBOOL bAddToCurrent=false);
	virtual void RetardRBLinearVelocity(const FVector& RetardDir, FLOAT VelScale);
	virtual void SetRBPosition(const FVector& NewPos, FName BoneName = NAME_None);
	virtual void SetRBRotation(const FRotator& NewRot, FName BoneName = NAME_None);
	virtual void WakeRigidBody( FName BoneName = NAME_None );
	virtual void PutRigidBodyToSleep(FName BoneName = NAME_None);
	virtual UBOOL RigidBodyIsAwake( FName BoneName = NAME_None );
	virtual void SetBlockRigidBody(UBOOL bNewBlockRigidBody);
	void SetRBCollidesWithChannel(ERBCollisionChannel Channel, UBOOL bNewCollides);
	void SetRBChannel(ERBCollisionChannel Channel);
	virtual void SetNotifyRigidBodyCollision(UBOOL bNewNotifyRigidBodyCollision);
	
	/** 
	 *	Used for creating one-way physics interactions.
	 *	@see RBDominanceGroup
	 */
	virtual void SetRBDominanceGroup(BYTE InDomGroup);

	/** 
	 *	Changes the current PhysMaterialOverride for this component. 
	 *	Note that if physics is already running on this component, this will _not_ alter its mass/inertia etc, it will only change its 
	 *	surface properties like friction and the damping.
	 */
	virtual void SetPhysMaterialOverride(UPhysicalMaterial* NewPhysMaterial);

	/** returns the physics RB_BodyInstance for the root body of this component (if any) */
	virtual URB_BodyInstance* GetRootBodyInstance();

	virtual void UpdateRBKinematicData();

	// Copied from TransformComponent
	virtual void SetTransformedToWorld();

	/**
	 * Test overlap against another primitive component, uses a box for the smaller primcomp
	 * @param Other - the other primitive component to test against 
	 * @param Hit   - the hit result to add results to
	 * @param OverlapAdjust - offset to use for testing against a position this primitive is not currently at
	 * @param bCollideComplex - whether to use complex collision when pointchecking against this
	 * @param bOtherCollideComplex - whether to use complex collision when point checking against other
	 */
	FORCEINLINE UBOOL IsOverlapping(UPrimitiveComponent* Other, FCheckResult* Hit, const FVector& OverlapAdjust, DWORD MyTraceFlags, DWORD OtherTraceFlags)
	{
		FBox OtherBox = Other->Bounds.GetBox();
		FBox MyBox = Bounds.GetBox();

		// worth it to use the smaller?  maybe should just always use one or the other..
		if(OtherBox.GetVolume() > MyBox.GetVolume())
		{
			FVector OtherBoxCenter, OtherBoxExtent;
			// offset other's box along the inverse of our offset since we're not using a box for ourselves
			OtherBox.Min -= OverlapAdjust;
			OtherBox.Max -= OverlapAdjust;
			OtherBox.GetCenterAndExtents(OtherBoxCenter,OtherBoxExtent);
	
			if( PointCheck(*Hit, OtherBoxCenter, OtherBoxExtent, OtherTraceFlags) == 0 )
			{
				Hit->Component = Other;
				Hit->SourceComponent = this;
				return TRUE;
			}
		}
		else
		{
			FVector MyBoxCenter, MyBoxExtent;
			// adjust our box with the overlapadjustment
			MyBox.Min += OverlapAdjust;
			MyBox.Max += OverlapAdjust;
			MyBox.GetCenterAndExtents(MyBoxCenter,MyBoxExtent);
			
			if( Other->PointCheck(*Hit, MyBoxCenter, MyBoxExtent, MyTraceFlags) == 0 )
			{
				Hit->Component = Other;
				Hit->SourceComponent = this;
				return TRUE;
			}
		}

		return FALSE;
	}

	/** allows components with 'AlwaysCheckCollision' set to TRUE to override trace flags during collision testing */
	virtual void OverrideTraceFlagsForNonCollisionComponentChecks( DWORD& Flags ){/*default to do nothing*/}

	DECLARE_FUNCTION(execAddImpulse);
	DECLARE_FUNCTION(execAddRadialImpulse);
	DECLARE_FUNCTION(execAddForce);
	DECLARE_FUNCTION(execAddRadialForce);
	DECLARE_FUNCTION(execAddTorque);
	DECLARE_FUNCTION(execSetRBLinearVelocity);
	DECLARE_FUNCTION(execSetRBAngularVelocity);
	DECLARE_FUNCTION(execRetardRBLinearVelocity);
	DECLARE_FUNCTION(execSetRBPosition);
	DECLARE_FUNCTION(execSetRBRotation);
	DECLARE_FUNCTION(execWakeRigidBody);
	DECLARE_FUNCTION(execPutRigidBodyToSleep);
	DECLARE_FUNCTION(execRigidBodyIsAwake);
	DECLARE_FUNCTION(execSetBlockRigidBody);
	DECLARE_FUNCTION(execSetRBCollidesWithChannel);
	DECLARE_FUNCTION(execSetRBChannel);
	DECLARE_FUNCTION(execSetNotifyRigidBodyCollision);
	DECLARE_FUNCTION(execInitRBPhys);
	DECLARE_FUNCTION(execSetPhysMaterialOverride);
	DECLARE_FUNCTION(execSetRBDominanceGroup);

	DECLARE_FUNCTION(execSetHidden);
	DECLARE_FUNCTION(execSetOwnerNoSee);
	DECLARE_FUNCTION(execSetOnlyOwnerSee);
	DECLARE_FUNCTION(execSetIgnoreOwnerHidden);
	DECLARE_FUNCTION(execSetShadowParent);
	DECLARE_FUNCTION(execSetLightEnvironment);
	DECLARE_FUNCTION(execSetCullDistance);
	DECLARE_FUNCTION(execSetLightingChannels);
	DECLARE_FUNCTION(execSetDepthPriorityGroup);
	DECLARE_FUNCTION(execSetViewOwnerDepthPriorityGroup);
	DECLARE_FUNCTION(execSetTraceBlocking);
	DECLARE_FUNCTION(execSetActorCollision);
	DECLARE_FUNCTION(execGetRootBodyInstance);

	// Copied from TransformComponent
	DECLARE_FUNCTION(execSetTranslation);
	DECLARE_FUNCTION(execSetRotation);
	DECLARE_FUNCTION(execSetScale);
	DECLARE_FUNCTION(execSetScale3D);
	DECLARE_FUNCTION(execSetAbsolute);

	DECLARE_FUNCTION(execGetRotation);

#if WITH_NOVODEX
	virtual class NxActor* GetNxActor(FName BoneName = NAME_None);
	virtual class NxActor* GetIndexedNxActor(INT BodyIndex = INDEX_NONE);

	/** Utility for getting all physics bodies contained within this component. */
	virtual void GetAllNxActors(TArray<class NxActor*>& OutActors);
#endif // WITH_NOVODEX

	/**
	 * Creates a proxy to represent the primitive to the scene manager in the rendering thread.
	 * @return The proxy object.
	 */
	virtual FPrimitiveSceneProxy* CreateSceneProxy()
	{
		return NULL;
	}

	/**
	 * Determines whether the proxy for this primitive type needs to be recreated whenever the primitive moves.
	 * @return TRUE to recreate the proxy when UpdateTransform is called.
	 */
	virtual UBOOL ShouldRecreateProxyOnUpdateTransform() const
	{
		return FALSE;
	}

protected:
	/** @name UActorComponent interface. */
	//@{
	virtual void SetParentToWorld(const FMatrix& ParentToWorld);
	virtual void Attach();
	virtual void UpdateTransform();
	virtual void Detach( UBOOL bWillReattach = FALSE );
	//@}

	/**
	* @return	TRUE if the base primitive component should handle detaching decals when the primitive is detached
	*/
	virtual UBOOL AllowDecalRemovalOnDetach() const
	{
		return TRUE;
	}

	/**
    * Only valid for cases when the primitive will be reattached
	* @return	TRUE if the base primitive component should handle reattaching decals when the primitive is attached
	*/
	virtual UBOOL AllowDecalAutomaticReAttach() const
	{
		return bAllowDecalAutomaticReAttach;
	}

	/** Internal function that updates physics objects to match the RBChannel/RBCollidesWithChannel info. */
	virtual void UpdatePhysicsToRBChannels();
public:

	/** 
	* @return TRUE if the primitive component can render decals
	*/
	virtual UBOOL SupportsDecalRendering() const
	{
		return TRUE;
	}

	// UObject interface.

	virtual void Serialize(FArchive& Ar);
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void PostLoad();
	virtual UBOOL IsReadyForFinishDestroy();
	virtual UBOOL NeedsLoadForClient() const;
	virtual UBOOL NeedsLoadForServer() const;

	virtual void SetOwnerNoSee(UBOOL bNewOwnerNoSee);
	virtual void SetOnlyOwnerSee(UBOOL bNewOnlyOwnerSee);
	virtual void SetIgnoreOwnerHidden(UBOOL bNewIgnoreOwnerHidden);

	virtual void SetHiddenGame(UBOOL NewHidden);

	/**
	 *	Sets the HiddenEditor flag and reattaches the component as necessary.
	 *
	 *	@param	NewHidden		New Value fo the HiddenEditor flag.
	 */
	virtual void SetHiddenEditor(UBOOL NewHidden);
	virtual void SetShadowParent(UPrimitiveComponent* NewShadowParent);
	virtual void SetLightEnvironment(ULightEnvironmentComponent* NewLightEnvironment);
	virtual void SetCullDistance(FLOAT NewCullDistance);
	virtual void SetLightingChannels(FLightingChannelContainer NewLightingChannels);
	virtual void SetDepthPriorityGroup(ESceneDepthPriorityGroup NewDepthPriorityGroup);
	virtual void SetViewOwnerDepthPriorityGroup(
		UBOOL bNewUseViewOwnerDepthPriorityGroup,
		ESceneDepthPriorityGroup NewViewOwnerDepthPriorityGroup
		);

	/**
	 * Default constructor, generates a GUID for the primitive.
	 */
	UPrimitiveComponent();
};

//
//	UMeshComponent
//

class UMeshComponent : public UPrimitiveComponent
{
	DECLARE_ABSTRACT_CLASS(UMeshComponent,UPrimitiveComponent,CLASS_NoExport,Engine);
public:

	TArrayNoInit<UMaterialInterface*>	Materials;

	/** @return The total number of elements in the mesh. */
	virtual INT GetNumElements() const PURE_VIRTUAL(UMeshComponent::GetNumElements,return 0;);

	/** Accesses the material applied to a specific material index. */
	virtual UMaterialInterface* GetMaterial(INT ElementIndex) const;

	/** Sets the material applied to a material index. */
	void SetMaterial(INT ElementIndex,UMaterialInterface* Material);

	/** Accesses the scene relevance information for the materials applied to the mesh. */
	FMaterialViewRelevance GetMaterialViewRelevance() const;

	/**
	 *	Tell the streaming system to start loading all textures with all mip-levels.
	 *	@param Seconds							Number of seconds to force all mip-levels to be resident
	 *	@param bPrioritizeCharacterTextures		Whether character textures should be prioritized for a while by the streaming system
	 */
	void PrestreamTextures( FLOAT Seconds, UBOOL bPrioritizeCharacterTextures );

	// UnrealScript interface.
	DECLARE_FUNCTION(execGetMaterial);
	DECLARE_FUNCTION(execSetMaterial);
	DECLARE_FUNCTION(execGetNumElements);
	DECLARE_FUNCTION(execPrestreamTextures);
};

//
//	USpriteComponent
//

class USpriteComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(USpriteComponent,UPrimitiveComponent,CLASS_NoExport,Engine);
public:
	
	UTexture2D*	Sprite;
	BITFIELD	bIsScreenSizeScaled:1;
	FLOAT		ScreenSize;

	// UPrimitiveComponent interface.
	/**
	 * Creates a proxy to represent the primitive to the scene manager in the rendering thread.
	 * @return The proxy object.
	 */
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
	virtual void UpdateBounds();
};

//
//	UBrushComponent
//

class UBrushComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(UBrushComponent,UPrimitiveComponent,CLASS_NoExport,Engine);

	UModel*						Brush;

	/** Simplified collision data for the mesh. */
	FKAggregateGeom				BrushAggGeom;

	/** Physics engine shapes created for this BrushComponent. */
	class NxActorDesc*			BrushPhysDesc;

	/** Cached brush convex-mesh data for use with the physics engine. */
	FKCachedConvexData			CachedPhysBrushData;

	/** 
	 *	Indicates version that CachedPhysBrushData was created at. 
	 *	Compared against CurrentCachedPhysDataVersion.
	 */
	INT							CachedPhysBrushDataVersion;

	/** 
	*	Normally a blocking volume is considered 'pure simplified collision', so when tracing for complex collision, never collide 
	*	This flag overrides that behaviour
	*/
	BITFIELD	bBlockComplexCollisionTrace:1;

	// UObject interface.
	virtual void Serialize( FArchive& Ar );
	virtual void PreSave();
	virtual void FinishDestroy();

	// UPrimitiveComponent interface.
	/**
	 * Creates a proxy to represent the primitive to the scene manager in the rendering thread.
	 * @return The proxy object.
	 */
	virtual FPrimitiveSceneProxy* CreateSceneProxy();

	virtual void InitComponentRBPhys(UBOOL bFixed);

	virtual UBOOL PointCheck(FCheckResult& Result,const FVector& Location,const FVector& Extent,DWORD TraceFlags);
	virtual UBOOL LineCheck(FCheckResult& Result,const FVector& End,const FVector& Start,const FVector& Extent,DWORD TraceFlags);

	virtual void UpdateBounds();

	virtual BYTE GetStaticDepthPriorityGroup() const;

	// UBrushComponent interface.
	virtual UBOOL IsValidComponent() const;

	// UBrushComponent interface

	/** Create the BrushAggGeom collection-of-convex-primitives from the Brush UModel data. */
	void BuildSimpleBrushCollision();

	/** Build cached convex data for physics engine. */
	void BuildPhysBrushData();
};

/**
 * Breaks a set of brushes down into a set of convex volumes.  The convex volumes are in world-space.
 * @param Brushes			The brushes to enumerate the convex volumes from.
 * @param OutConvexVolumes	Upon return, contains the convex volumes which compose the input brushes.
 */
extern void GetConvexVolumesFromBrushes(const TArray<ABrush*>& Brushes,TArray<FConvexVolume>& OutConvexVolumes);

//
//	UCylinderComponent
//

class UCylinderComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(UCylinderComponent,UPrimitiveComponent,CLASS_NoExport,Engine);

	FLOAT	CollisionHeight;
	FLOAT	CollisionRadius;

	/** Color used to draw the cylinder. */
	FColor	CylinderColor;

	/**	Whether to draw the red bounding box for this cylinder. */
	BITFIELD	bDrawBoundingBox:1;

	/** If TRUE, this cylinder will always draw when SHOW_Collision is on, even if CollideActors is FALSE. */
	BITFIELD	bDrawNonColliding:1;
		
	// UPrimitiveComponent interface.

	virtual UBOOL PointCheck(FCheckResult& Result,const FVector& Location,const FVector& Extent,DWORD TraceFlags);
	virtual UBOOL LineCheck(FCheckResult& Result,const FVector& End,const FVector& Start,const FVector& Extent,DWORD TraceFlags);

	/**
	 * Creates a proxy to represent the primitive to the scene manager in the rendering thread.
	 * @return The proxy object.
	 */
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
	virtual void UpdateBounds();

	// UCylinderComponent interface.

	void SetCylinderSize(FLOAT NewRadius, FLOAT NewHeight);

	// Native script functions.

	DECLARE_FUNCTION(execSetCylinderSize);
};

//
//	UArrowComponent
//

class UArrowComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(UArrowComponent,UPrimitiveComponent,CLASS_NoExport,Engine);

	FColor	ArrowColor;
	FLOAT	ArrowSize;
	UBOOL	bTreatAsASprite;

	// UPrimitiveComponent interface.
	/**
	 * Creates a proxy to represent the primitive to the scene manager in the rendering thread.
	 * @return The proxy object.
	 */
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
	virtual void UpdateBounds();
};

//
//	UDrawSphereComponent
//

class UDrawSphereComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(UDrawSphereComponent,UPrimitiveComponent,CLASS_NoExport,Engine);

	FColor				SphereColor;
	UMaterialInterface*	SphereMaterial;
	FLOAT				SphereRadius;
	INT					SphereSides;
	BITFIELD			bDrawWireSphere:1;
	BITFIELD			bDrawLitSphere:1;

	// UPrimitiveComponent interface.
	/**
	 * Creates a proxy to represent the primitive to the scene manager in the rendering thread.
	 * @return The proxy object.
	 */
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
	virtual void UpdateBounds();
};

//
//	UDrawCylinderComponent
//

class UDrawCylinderComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(UDrawCylinderComponent,UPrimitiveComponent,CLASS_NoExport,Engine);

	FColor						CylinderColor;
	class UMaterialInstance*	CylinderMaterial;
	FLOAT						CylinderRadius;
	FLOAT						CylinderTopRadius;
	FLOAT						CylinderHeight;
	FLOAT						CylinderHeightOffset;
	INT							CylinderSides;
	BITFIELD					bDrawWireCylinder:1;
	BITFIELD					bDrawLitCylinder:1;

	// UPrimitiveComponent interface.
	/**
	 * Creates a proxy to represent the primitive to the scene manager in the rendering thread.
	 * @return The proxy object.
	 */
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
	virtual void UpdateBounds();
};

//
//	UDrawBoxComponent
//

class UDrawBoxComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(UDrawBoxComponent,UPrimitiveComponent,CLASS_NoExport,Engine);

	FColor				BoxColor;
	UMaterialInstance*	BoxMaterial;
	FVector				BoxExtent;
	BITFIELD			bDrawWireBox:1;
	BITFIELD			bDrawLitBox:1;

	// UPrimitiveComponent interface.
	/**
	 * Creates a proxy to represent the primitive to the scene manager in the rendering thread.
	 * @return The proxy object.
	 */
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
	virtual void UpdateBounds();
};

//
//	UDrawBoxComponent
//

class UDrawCapsuleComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(UDrawCapsuleComponent,UPrimitiveComponent,CLASS_NoExport,Engine);

	FColor				CapsuleColor;
	UMaterialInstance*	CapsuleMaterial;
	float				CapsuleHeight;
	float				CapsuleRadius;
	BITFIELD			bDrawWireCapsule:1;
	BITFIELD			bDrawLitCapsule:1;

	// UPrimitiveComponent interface.
	/**
	 * Creates a proxy to represent the primitive to the scene manager in the rendering thread.
	 * @return The proxy object.
	 */
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
	virtual void UpdateBounds();
};

//
//	UDrawFrustumComponent
//

class UDrawFrustumComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(UDrawFrustumComponent,UPrimitiveComponent,CLASS_NoExport,Engine);

	FColor			FrustumColor;
	FLOAT			FrustumAngle;
	FLOAT			FrustumAspectRatio;
	FLOAT			FrustumStartDist;
	FLOAT			FrustumEndDist;
	/** optional texture to show on the near plane */
	UTexture*		Texture;

	// UPrimitiveComponent interface.
	/**
	 * Creates a proxy to represent the primitive to the scene manager in the rendering thread.
	 * @return The proxy object.
	 */
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
	virtual void UpdateBounds();
};

class UDrawLightRadiusComponent : public UDrawSphereComponent
{
	DECLARE_CLASS(UDrawLightRadiusComponent,UDrawSphereComponent,CLASS_NoExport,Engine);

	// UPrimitiveComponent interface.
	/**
	 * Creates a proxy to represent the primitive to the scene manager in the rendering thread.
	 * @return The proxy object.
	 */
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
	virtual void UpdateBounds();
};

//
//	UCameraConeComponent
//

class UCameraConeComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(UCameraConeComponent,UPrimitiveComponent,CLASS_NoExport,Engine);

	// UPrimitiveComponent interface.
	/**
	 * Creates a proxy to represent the primitive to the scene manager in the rendering thread.
	 * @return The proxy object.
	 */
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
	virtual void UpdateBounds();
};


/**
 *	Utility component for drawing a textured quad face. 
 *  Origin is at the component location, frustum points down position X axis.
 */
class UDrawQuadComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(UDrawQuadComponent,UPrimitiveComponent,CLASS_NoExport,Engine);

	/** Texture source to draw on quad face */
	UTexture*	Texture;
	/** Width of quad face */
	FLOAT		Width;
	/** Height of quad face */
	FLOAT		Height;

	// UPrimitiveComponent interface.

	virtual void Render(const FSceneView* View,class FPrimitiveDrawInterface* PDI);
	virtual void UpdateBounds();
};
