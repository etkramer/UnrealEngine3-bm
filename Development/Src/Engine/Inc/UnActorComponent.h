/*=============================================================================
	UnActorComponent.h: Actor component definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

//
//	UActorComponent
//

class UActorComponent : public UComponent
{
	DECLARE_ABSTRACT_CLASS(UActorComponent,UComponent,CLASS_NoExport,Engine);
protected:

	friend class FComponentReattachContext;

	FSceneInterface* Scene;
	class AActor* Owner;
	BITFIELD bAttached:1 GCC_BITFIELD_MAGIC;

	/**
	 * Sets the ParentToWorld transform the component is attached to.
	 * This needs to be followed by a call to Attach or UpdateTransform.
	 * @param ParentToWorld - The ParentToWorld transform the component is attached to.
	 */
	virtual void SetParentToWorld(const FMatrix& ParentToWorld) {}

	/**
	 * Attaches the component to a ParentToWorld transform, owner and scene.
	 * Requires IsValidComponent() == true.
	 */
	virtual void Attach();

	/**
	 * Updates state dependent on the ParentToWorld transform.
	 * Requires bAttached == true
	 */
	virtual void UpdateTransform();

	/**
	 * Detaches the component from the scene it is in.
	 * Requires bAttached == true
	 *
	 * @param bWillReattach TRUE is passed if Attach will be called immediately afterwards.  This can be used to
	 *                      preserve state between reattachments.
	 */
	virtual void Detach( UBOOL bWillReattach = FALSE );

	/**
	 * Starts gameplay for this component.
	 * Requires bAttached == true.
	 */
	virtual void BeginPlay();

	/**
	 * Updates time dependent state for this component.
	 * Requires bAttached == true.
	 * @param DeltaTime - The time since the last tick.
	 */
	virtual void Tick(FLOAT DeltaTime);

	/**
	 * Checks the components that are indirectly attached via this component for pending lazy updates, and applies them.
	 */
	virtual void UpdateChildComponents() {}

public:
	BITFIELD		bTickInEditor:1;
	BITFIELD		bNeedsReattach:1;
	BITFIELD		bNeedsUpdateTransform:1;

public:

	/** The ticking group this component belongs to */
	BYTE TickGroup GCC_BITFIELD_MAGIC;

	/**
	 * Conditionally calls Attach if IsValidComponent() == true.
	 * @param InScene - The scene to attach the component to.
	 * @param InOwner - The actor which the component is directly or indirectly attached to.
	 * @param ParentToWorld - The ParentToWorld transform the component is attached to.
	 */
	void ConditionalAttach(FSceneInterface* InScene,AActor* InOwner,const FMatrix& ParentToWorld);

	/**
	 * Conditionally calls UpdateTransform if bAttached == true.
	 * @param ParentToWorld - The ParentToWorld transform the component is attached to.
	 */
	void ConditionalUpdateTransform(const FMatrix& ParentToWorld);

	/**
	 * Conditionally calls UpdateTransform if bAttached == true.
	 */
	void ConditionalUpdateTransform();

	/**
	 * Conditionally calls Detach if bAttached == true.
	 *
	 * @param bWillReattach TRUE is passed if Attach will be called immediately afterwards.  This can be used to
	 *                      preserve state between reattachments.
	 */
	void ConditionalDetach( UBOOL bWillReattach = FALSE );

	/**
	 * Conditionally calls BeginPlay if bAttached == true.
	 */
	void ConditionalBeginPlay();

	/**
	 * Conditionally calls Tick if bAttached == true.
	 * @param DeltaTime - The time since the last tick.
	 */
	void ConditionalTick(FLOAT DeltaTime);

	/**
	 * Returns whether the component's owner is selected.
	 */
	UBOOL IsOwnerSelected() const;

	/**
	 * Returns whether the component is valid to be attached.
	 * This should be implemented in subclasses to validate parameters.
	 * If this function returns false, then Attach won't be called.
	 */
	virtual UBOOL IsValidComponent() const;

	/**
	 * Called when this actor component has moved, allowing it to discard statically cached lighting information.
	 */
	virtual void InvalidateLightingCache() {}

	/**
	 * Marks the appropriate world as lighting requiring a rebuild.
	 */
	void MarkLightingRequiringRebuild();

	/**
	 * For initialising any rigid-body physics related data in this component.
	 */
	virtual void InitComponentRBPhys(UBOOL bFixed) {}

	/**
	 * For changing physics information between dynamic and simulating and 'fixed' (ie infinitely massive).
	 */
	virtual void SetComponentRBFixed(UBOOL bFixed) {}

	/**
	 * For terminating any rigid-body physics related data in this component.
	 */
	virtual void TermComponentRBPhys(class FRBPhysScene* InScene) {}

	/**
	 * Called during map error checking .  Derived class implementations should call up to their parents.
	 */
	virtual void CheckForErrors();

	/** if the component is attached, finds out what it's attached to and detaches it from that thing
	 * slower, so use only when you don't have an easy way to find out where it's attached
	 */
	void DetachFromAny();

	/**
	 * If the component needs attachment, or has a lazy update pending, perform it.  Also calls UpdateChildComponents to update indirectly attached components.
	 * @param InScene - The scene which the component should be attached to.
	 * @param InOwner - The actor which the component should be attached to.
	 * @param InActorToWorld - The transform which the component should be attached to.
	 */
	void UpdateComponent(FSceneInterface* InScene,AActor* InOwner,const FMatrix& InLocalToWorld,UBOOL bCollisionUpdate=FALSE);

	/**
	 * Begins a deferred component reattach.
	 * If the game is running, this will defer reattaching the component until the end of the game tick.
	 * If the editor is running, this will immediately reattach the component.
	 */
	void BeginDeferredReattach();

	/**
	 * Begins a deferred component reattach.
	 * If the game is running, this will defer reattaching the component until the end of the game tick.
	 * If the editor is running, this will immediately reattach the component.
	 */
	void BeginDeferredUpdateTransform();

	/**
	 * Dirties transform flag to ensure that UpdateComponent updates transform.
	 */
	void DirtyTransform() { bNeedsUpdateTransform = TRUE; }

	/**
	 * Returns whether component should be marked as pending kill inside AActor::MarkComponentsAsPendingKill if it has
	 * a say in it. This is e.g. used by audio components to not be removed by the garbage collector while they are
	 * playing.
	 *
	 * @return TRUE
	 */
	virtual UBOOL AllowBeingMarkedPendingKill()
	{
		return TRUE;
	}

	/** @name Accessors. */
	FSceneInterface* GetScene() const { return Scene; }
	class AActor* GetOwner() const { return Owner; }
	void SetOwner(class AActor* NewOwner) { Owner = NewOwner; }
	UBOOL IsAttached() const { return bAttached; }
	UBOOL NeedsReattach() const { return bNeedsReattach; }
	UBOOL NeedsUpdateTransform() const { return bNeedsUpdateTransform; }

	/** UObject interface. */
	virtual void BeginDestroy();
	virtual UBOOL NeedsLoadForClient() const;
	virtual UBOOL NeedsLoadForServer() const;
	virtual void PreEditChange(UProperty* PropertyThatWillChange);
	virtual void PostEditChange(UProperty* PropertyThatChanged);

	/**
	 * This pass-through function is necessary to work around a bug in the VS 2005 compiler
	 * where classes that inherit from UActorComponent are unable to call the inherited PEC from UObject
	 */
	virtual void PostEditChange(class FEditPropertyChain& PropertyThatChanged)
	{
		Super::PostEditChange(PropertyThatChanged);
	}

	DECLARE_FUNCTION(execSetComponentRBFixed)
	{
		P_GET_UBOOL(bFixed);
		P_FINISH;
		SetComponentRBFixed(bFixed);
	}

    DECLARE_FUNCTION(execSetTickGroup)
    {
        P_GET_BYTE(NewTickGroup);
        P_FINISH;
        SetTickGroup(NewTickGroup);
    }
	/**
	 * Sets the ticking group for this component
	 *
	 * @param NewTickGroup the new group to assign
	 */
	void SetTickGroup(BYTE NewTickGroup);

	DECLARE_FUNCTION(execForceUpdate);

	DECLARE_FUNCTION(execDetachFromAny)
	{
		P_FINISH;
		DetachFromAny();
	}
};


/**
 * Light Classification Enum
 *
 * @warning: this structure is manually mirrored in LightComponent.uc
 */
enum ELightAffectsClassification
{
	LAC_USER_SELECTED                = 0,
	LAC_DYNAMIC_AFFECTING            = 1,
	LAC_STATIC_AFFECTING             = 2,
	LAC_DYNAMIC_AND_STATIC_AFFECTING = 3,
	LAC_MAX                          = 4
};

/**
* Type of shadowing to apply for the light
* @warning: this structure is manually mirrored in LightComponent.uc
*/
enum ELightShadowMode
{
	/** Shadows rendered due to absence of light when doing dynamic lighting. Default */
	LightShadow_Normal=0,
	/** Shadows rendered as a fullscreen pass by modulating entire scene by a shadow factor. */
	LightShadow_Modulate,
	/** 
	* Shadows rendered as a fullscreen pass by modulating entire scene by a shadow factor, but also 
	* renders receiver geometry in order to prevent shadows on backfaces and emissive areas
	*/
	LightShadow_ModulateBetter
};

/**
* Type of shadow projection to apply for the light
* @warning: this structure is manually mirrored in LightComponent.uc
*/
enum EShadowProjectionTechnique
{
	/** Shadow projection is rendered using either PCF/VSM based on global settings  */
	ShadowProjTech_Default=0,
	/** Shadow projection is rendered using the PCF (Percentage Closer Filtering) technique. May have heavy banding artifacts */
	ShadowProjTech_PCF,
	/** Shadow projection is rendered using the VSM (Variance Shadow Map) technique. May have shadow offset and light bleed artifacts */
	ShadowProjTech_VSM,
	/** Shadow projection is rendered using the Low quality Branching PCF technique. May have banding and penumbra detection artifacts */
	ShadowProjTech_BPCF_Low,
	/** Shadow projection is rendered using the Medium quality Branching PCF technique. May have banding and penumbra detection artifacts */
	ShadowProjTech_BPCF_Medium,
	/** Shadow projection is rendered using the High quality Branching PCF technique. May have banding and penumbra detection artifacts */
	ShadowProjTech_BPCF_High,
};

/**
* Quality settings for projected shadow buffer filtering
* @warning: this structure is manually mirrored in LightComponent.uc
*/
enum EShadowFilterQuality
{
	SFQ_Low=0,
	SFQ_Medium,
	SFQ_High
};

/**
 * Lighting channel container.
 *
 * @warning: this structure is manually mirrored in LightComponent.uc
 * @warning: this structure cannot exceed 32 bits
 */
struct FLightingChannelContainer
{
	union
	{
		struct
		{
			/** Whether the lighting channel has been initialized. Used to determine whether UPrimitveComponent::Attach should set defaults. */
			BITFIELD bInitialized:1;

			// User settable channels that are auto set and also default to true for lights.
			BITFIELD BSP:1;
			BITFIELD Static:1;
			BITFIELD Dynamic:1;
			// User set channels.
			BITFIELD CompositeDynamic:1;
			BITFIELD Skybox:1;
			BITFIELD Unnamed_1:1;
			BITFIELD Unnamed_2:1;
			BITFIELD Unnamed_3:1;
			BITFIELD Unnamed_4:1;
			BITFIELD Unnamed_5:1;
			BITFIELD Unnamed_6:1;
			BITFIELD Cinematic_1:1;
			BITFIELD Cinematic_2:1;
			BITFIELD Cinematic_3:1;
			BITFIELD Cinematic_4:1;
			BITFIELD Cinematic_5:1;
			BITFIELD Cinematic_6:1;
			BITFIELD Cinematic_7:1;
			BITFIELD Cinematic_8:1;
			BITFIELD Cinematic_9:1;
			BITFIELD Cinematic_10:1;
			BITFIELD Gameplay_1:1;
			BITFIELD Gameplay_2:1;
			BITFIELD Gameplay_3:1;
			BITFIELD Gameplay_4:1;
			BITFIELD Crowd:1;
		};
		DWORD Bitfield;
	};

	/**
	 * Returns whether the passed in lighting channel container shares any set channels with the current one.
	 *
	 * @param	Other	other lighting channel to check against
	 * @return	TRUE if there's overlap, FALSE otherwise
	 */
	UBOOL OverlapsWith( const FLightingChannelContainer& Other ) const
	{
		// We need to mask out bInitialized when determining overlap.
		FLightingChannelContainer Mask;
		Mask.Bitfield		= 0;
		Mask.bInitialized	= TRUE;
		DWORD BitfieldMask	= ~Mask.Bitfield;
		return Bitfield & Other.Bitfield & BitfieldMask ? TRUE : FALSE;
	}

	/**
	 * Sets all channels.
	 */
	void SetAllChannels()
	{
		FLightingChannelContainer Mask;
		Mask.Bitfield		= 0;
		Mask.bInitialized	= TRUE;
		DWORD BitfieldMask	= ~Mask.Bitfield;
		Bitfield = Bitfield | BitfieldMask;
	}

	/**
	 * Clears all channels.
	 */
	void ClearAllChannels()
	{
		FLightingChannelContainer Mask;
		Mask.Bitfield		= 0;
		Mask.bInitialized	= TRUE;
		DWORD BitfieldMask	= Mask.Bitfield;
		Bitfield = Bitfield & BitfieldMask;
	}

	/** Lighting channel hash function. */
	friend DWORD GetTypeHash(const FLightingChannelContainer& LightingChannels)
	{
		return LightingChannels.Bitfield >> 1;
	}

	/** Comparison operator. */
	friend UBOOL operator==(const FLightingChannelContainer& A,const FLightingChannelContainer& B)
	{
		return A.Bitfield == B.Bitfield;
	}
};

/** different light component types */
enum ELightComponentType
{
	LightType_Sky,
	LightType_SphericalHarmonic,
	LightType_Directional,
	LightType_Point,
	LightType_Spot,
	LightType_MAX
};

//
//	ULightComponent
//

class ULightComponent : public UActorComponent
{
	DECLARE_ABSTRACT_CLASS(ULightComponent,UActorComponent,CLASS_NoExport,Engine)
public:

	/** The light's scene info. */
	FLightSceneInfo* SceneInfo;

	FMatrix WorldToLight;
	FMatrix LightToWorld;

	/**
	 * GUID used to associate a light component with precomputed shadowing information across levels.
	 * The GUID changes whenever the light position changes.
	 */
	FGuid					LightGuid;
	/**
	 * GUID used to associate a light component with precomputed shadowing information across levels.
	 * The GUID changes whenever any of the lighting relevant properties changes.
	 */
	FGuid					LightmapGuid;

	FLOAT					Brightness;
	FColor					LightColor;
	class ULightFunction*	Function;
		
	/** The intensity of bounced lighting from this light in DynamicLightEnvironments. */
	FLOAT LightEnv_BouncedLightBrightness;

	/** The color of bounced lighting from this light in DynamicLightEnvironments. */
	FColor LightEnv_BouncedModulationColor;

	BITFIELD bEnabled : 1;

	/** True if the light can be blocked by shadow casting primitives. */
	BITFIELD CastShadows : 1;

	/** True if the light can be blocked by static shadow casting primitives. */
	BITFIELD CastStaticShadows : 1;

	/** True if the light can be blocked by dynamic shadow casting primitives. */
	BITFIELD CastDynamicShadows : 1;

	/** True if the light should cast shadow from primitives which use a composite light environment. */
	BITFIELD bCastCompositeShadow : 1;

	/** If bCastCompositeShadow=TRUE, whether the light should affect the composite shadow direction. */
	BITFIELD bAffectCompositeShadowDirection : 1;

	/** This variable has been replaced with bForceDynamicLight. */
	BITFIELD RequireDynamicShadows : 1;

	/** True if this light should use dynamic shadows for all primitives. */
	BITFIELD bForceDynamicLight : 1;

	/** Set to True to store the direct flux of this light in a light-map. */
	BITFIELD UseDirectLightMap : 1;

	/** Whether light has ever been built into a lightmap */
	BITFIELD bHasLightEverBeenBuiltIntoLightMap : 1;

	/** Whether to only affect primitives that are in the same level/ share the same  GetOutermost() or are in the set of additionally specified ones. */
	BITFIELD bOnlyAffectSameAndSpecifiedLevels : 1;

	/** Whether the light can affect dynamic primitives even though the light is not affecting the dynamic channel. */
	BITFIELD bCanAffectDynamicPrimitivesOutsideDynamicChannel : 1;

	/** Whether to use the inclusion/ exclusion volumes. */
	BITFIELD bUseVolumes : 1;

	/**
	 * The light environment which the light affects.
	 * NULL represents an implicit default light environment including all primitives and lights with LightEnvironment=NULL.
	 */
	class ULightEnvironmentComponent* LightEnvironment;

	/** Array of other levels to affect if bOnlyAffectSameAndSpecifiedLevels is TRUE, own level always implicitly part of array. */
	TArrayNoInit<FName> OtherLevelsToAffect;

	/** Lighting channels controlling light/ primitive interaction. Only allows interaction if at least one channel is shared */
	FLightingChannelContainer LightingChannels;

	/**
	 * Array of volumes used by AffectsBounds if bUseVolumes is set. Light will only affect primitives if they are touching or are contained
	 * by at least one volume. Exclusion overrides inclusion.
	 */
	TArrayNoInit<class ABrush*> InclusionVolumes;

	/**
	 * Array of volumes used by AffectsBounds if bUseVolumes is set. Light will only affect primitives if they neither touching nor are
	 * contained by at least one volume. Exclusion overrides inclusion.
	 */
	TArrayNoInit<class ABrush*> ExclusionVolumes;

	/** Array of convex inclusion volumes, populated from InclusionVolumes by PostEditChange. */
	TArrayNoInit<FConvexVolume>	InclusionConvexVolumes;

	/** Array of convex exclusion volumes, populated from ExclusionVolumes by PostEditChange. */
	TArrayNoInit<FConvexVolume> ExclusionConvexVolumes;

	/**
	 * This is the classification of this light.  This is used for placing a light for an explicit
     * purpose.  Basically you can now have "type" information with lights and understand the
	 * intent of why a light was placed.  This is very useful for content people getting maps
	 * from others and understanding why there is a dynamic affect light in the middle of the world
	 * with a radius of 32k!  And also useful for being able to do searches such as the following:
	 * show me all lights which affect dynamic objects.  Now show me the set of lights which are
	 * not explicitly set as Dynamic Affecting lights.
	 *
	 **/
	BYTE LightAffectsClassification;

	/** Type of shadowing to apply for the light */
	BYTE LightShadowMode;

	/** Shadow color for modulating entire scene */
	FLinearColor ModShadowColor;

	/** Time since the caster was last visible at which the mod shadow will fade out completely.  */
	FLOAT ModShadowFadeoutTime;

	/** Exponent that controls mod shadow fadeout curve. */
	FLOAT ModShadowFadeoutExponent;

private:
	/**
	 * The munged index of this light in the light list
	 *
	 * > 0 == static light list
	 *   0 == not part of any light list
	 * < 0 == dynamic light list
	 */
	INT LightListIndex;

public:
	/** Type of shadow projection to use for this light */
	BYTE ShadowProjectionTechnique;

	/** Quality of shadow buffer filtering to use for this light's shadows */
	BYTE ShadowFilterQuality;

	/**
	 * Override for min dimensions (in texels) allowed for rendering shadow subject depths.
	 * This also controls shadow fading, once the shadow resolution reaches MinShadowResolution it will be faded out completely.
	 * A value of 0 defaults to MinShadowResolution in SystemSettings.
	 */
	INT MinShadowResolution;

	/**
	 * Override for max square dimensions (in texels) allowed for rendering shadow subject depths.
	 * A value of 0 defaults to MaxShadowResolution in SystemSettings.
	 */
	INT MaxShadowResolution;

	/** 
	 * Resolution in texels below which shadows begin to be faded out. 
	 * Once the shadow resolution reaches MinShadowResolution it will be faded out completely.
	 * A value of 0 defaults to ShadowFadeResolution in SystemSettings.
	 */
	INT ShadowFadeResolution;

	/**
	 * Creates a proxy to represent the light to the scene manager in the rendering thread.
	 * @return The proxy object.
	 */
	virtual FLightSceneInfo* CreateSceneInfo() const PURE_VIRTUAL(ULightComponent::CreateSceneInfo,return NULL;);

	/**
	 * Tests whether this light affects the given primitive.  This checks both the primitive and light settings for light relevance
	 * and also calls AffectsBounds and AffectsLevel.
	 * @param PrimitiveSceneInfo - The primitive to test.
	 * @return True if the light affects the primitive.
	 */
	virtual UBOOL AffectsPrimitive(const UPrimitiveComponent* Primitive) const;

	/**
	 * Tests whether the light affects the given bounding volume.
	 * @param Bounds - The bounding volume to test.
	 * @return True if the light affects the bounding volume
	 */
	virtual UBOOL AffectsBounds(const FBoxSphereBounds& Bounds) const;

	/**
	 * Tests whether the light affects the given level.
	 * @param LevelPackage - The package of the level to test.
	 * @return TRUE if the light affects the level.
	 */
	UBOOL AffectsLevel(const UPackage* LevelPackage) const;

	/**
	 * Returns the world-space bounding box of the light's influence.
	 */
	virtual FBox GetBoundingBox() const { return FBox(FVector(-HALF_WORLD_MAX,-HALF_WORLD_MAX,-HALF_WORLD_MAX),FVector(HALF_WORLD_MAX,HALF_WORLD_MAX,HALF_WORLD_MAX)); }

	// GetDirection

	FVector GetDirection() const { return FVector(WorldToLight.M[0][2],WorldToLight.M[1][2],WorldToLight.M[2][2]); }

	// GetOrigin

	FVector GetOrigin() const { return LightToWorld.GetOrigin(); }

	/**
	 * Returns the homogenous position of the light.
	 */
	virtual FVector4 GetPosition() const PURE_VIRTUAL(ULightComponent::GetPosition,return FVector4(););

	/**
	* @return ELightComponentType for the light component class
	*/
	virtual ELightComponentType GetLightType() const PURE_VIRTUAL(ULightComponent::GetLightType,return LightType_MAX;);

	/**
	 * Computes the intensity of the direct lighting from this light on a specific point.
	 */
	virtual FLinearColor GetDirectIntensity(const FVector& Point) const;

	/**
	 * Computes the intensity of the bounced lighting from this light on a specific point.
	 */
	virtual FLinearColor GetBouncedIntensity(const FVector& Point) const;

	/**
	 * Updates/ resets light GUIDs.
	 */
	virtual void UpdateLightGUIDs();

	/**
	 * Validates light GUIDs and resets as appropriate.
	 */
	void ValidateLightGUIDs();

	/**
	 * Checks whether a given primitive will cast shadows from this light.
	 * @param Primitive - The potential shadow caster.
	 * @return Returns True if a primitive blocks this light.
	 */
	UBOOL IsShadowCast(UPrimitiveComponent* Primitive) const;

	/**
	 * Returns True if a light cannot move or be destroyed during gameplay, and can thus cast static shadowing.
	 */
	UBOOL HasStaticShadowing() const;

	/**
	 * Returns True if dynamic primitives should cast projected shadows for this light.
	 */
	UBOOL HasProjectedShadowing() const;

	/**
	 * Returns True if a light's parameters as well as its position is static during gameplay, and can thus use static lighting.
	 */
	UBOOL HasStaticLighting() const;

	/**
	 * Returns whether static lighting, aka lightmaps, is being used for primitive/ light
	 * interaction.
	 *
	 * @param bForceDirectLightMap	Whether primitive is set to force lightmaps
	 * @return TRUE if lightmaps/ static lighting is being used, FALSE otherwise
	 */
	UBOOL UseStaticLighting( UBOOL bForceDirectLightMap ) const;

	/**
	 * Updates the inclusion/ exclusion volumes.
	 */
	void UpdateVolumes();

	/**
	 * Toggles the light on or off
	 *
	 * @param bSetEnabled TRUE to enable the light or FALSE to disable it
	 */
	void SetEnabled( UBOOL bSetEnabled );

	DECLARE_FUNCTION(execSetEnabled);
	DECLARE_FUNCTION(execSetLightProperties);
	DECLARE_FUNCTION(execGetOrigin);
	DECLARE_FUNCTION(execGetDirection);
	DECLARE_FUNCTION(execUpdateColorAndBrightness);

	// UObject interface
	virtual void Serialize(FArchive& Ar);
	virtual void PostLoad();
	virtual void PreEditUndo();
	virtual void PostEditUndo();

	// UActorComponent interface.
	virtual void PostEditChange(UProperty* PropertyThatChanged);

	/**
	 * Called after duplication & serialization and before PostLoad. Used to e.g. make sure GUIDs remains globally unique.
	 */
	virtual void PostDuplicate();

	/**
	 * Called after importing property values for this object (paste, duplicate or .t3d import)
	 * Allow the object to perform any cleanup for properties which shouldn't be duplicated or
	 * are unsupported by the script serialization
	 */
	virtual void PostEditImport();

    /** This will set the light classification based on the current settings **/
	virtual void SetLightAffectsClassificationBasedOnSettings();

    /** determines if this LightComponent meets the constraints for the dynamic affecting classifcation **/
	virtual UBOOL IsLACDynamicAffecting();

    /** determines if this LightComponent meets the constraints for the static affecting classifcation **/
	virtual UBOOL IsLACStaticAffecting();

    /** determines if this LightComponent meets the constraints for the dynamic and static affecting classifcation **/
	virtual UBOOL IsLACDynamicAndStaticAffecting();

	/**
	 * Invalidates lightmap data of affected primitives if this light has ever been built
	 * into a lightmap.
	 */
	virtual void InvalidateLightmapData();

	/**
	 * Functions to access the LightListIndex correctly
	 *
	 * > 0 == static light list
	 *   0 == not part of any light list
	 * < 0 == dynamic light list
	 */

	/**
	 * Returns whether light is currently in a light list.
	 *
	 * @return TRUE if light is in a light list, false otherwise
	 */
	FORCEINLINE UBOOL IsInLightList()
	{
		return LightListIndex != 0;
	}
	/**
	 * Returns whether the light is part of the static light list
	 *
	 * @return TRUE if light is in static light list, FALSE otherwise
	 */
	FORCEINLINE UBOOL IsInStaticLightList()
	{
		return LightListIndex > 0;
	}
	/**
	 * Returns whether the light is part of the dynamic light list
	 *
	 * @return TRUE if light is in dynamic light list, FALSE otherwise
	 */
	FORCEINLINE UBOOL IsInDynamicLightList()
	{
		return LightListIndex < 0;
	}
	/**
	 * Returns the light list index. The calling code is responsible for determining
	 * which list the index is for by using the other accessor functions.
	 *
	 * @return Index in light list
	 */
	FORCEINLINE INT	GetLightListIndex()
	{
		if( LightListIndex > 0 )
		{
			return LightListIndex-1;
		}
		// Technically should be an if < 0 but we assume that this function is only called
		// on valid light list indices for performance reasons.
		else
		{
			return -LightListIndex-1;
		}
	}
	/**
	 * Sets the static light list index.
	 *
	 * @param Index		New index to set
	 */
	FORCEINLINE void SetStaticLightListIndex(INT InIndex)
	{
		LightListIndex = InIndex+1;
	}
	/**
	 * Sets the dynamic light list index.
	 *
	 * @param Index		New index to set
	 */
	FORCEINLINE void SetDynamicLightListIndex(INT InIndex)
	{
		LightListIndex = -InIndex-1;
	}
	/**
	 * Invalidates the light list index.
	 */
	FORCEINLINE void InvalidateLightListIndex()
	{
		LightListIndex = 0;
	}

	/**
	 * Adds this light to the appropriate light list.
	 */
	void AddToLightList();

protected:
	virtual void SetParentToWorld(const FMatrix& ParentToWorld);
	virtual void Attach();
	virtual void UpdateTransform();
	virtual void Detach( UBOOL bWillReattach = FALSE );
public:
	virtual void InvalidateLightingCache();
};

enum ERadialImpulseFalloff
{
	RIF_Constant = 0,
	RIF_Linear = 1
};

//
//	UAudioComponent
//

class USoundCue;
class USoundNode;
class USoundNodeWave;
class UAudioComponent;

enum ELoopingMode
{
	/** One shot sound */
	LOOP_Never,
	/** Call the user callback on each loop for dynamic control */
	LOOP_WithNotification,
	/** Loop the sound forever */
	LOOP_Forever
};

/**
 * Structure encapsulating all information required to play a USoundNodeWave on a channel/source. This is required
 * as a single USoundNodeWave object can be used in multiple active cues or multiple times in the same cue.
 */
struct FWaveInstance
{
	/** Static helper to create good unique type hashes */
	static DWORD TypeHashCounter;

	/** Wave data */
	USoundNodeWave*		WaveData;
	/** Sound node to notify when the current audio buffer finishes */
	USoundNode*			NotifyBufferFinishedHook;
	/** Audio component this wave instance belongs to */
	UAudioComponent*	AudioComponent;

	/** Current volume */
	FLOAT				Volume;
	/** Current volume multiplier - used to zero the volume without stopping the source */
	FLOAT				VolumeMultiplier;
	/** Current priority */
	FLOAT				PlayPriority;
	/** Voice center channel volume */
	FLOAT				VoiceCenterChannelVolume;
	/** Radio volume multiplier */
	FLOAT				VoiceRadioVolume;
	/** Looping mode */
	INT					LoopingMode;

	/** Whether wave instanced has been started */
	UBOOL				bIsStarted;
	/** Whether wave instanced is finished */
	UBOOL				bIsFinished;
	/** Whether the notify finished hook has been called since the last update/parsenodes */
	UBOOL				bAlreadyNotifiedHook;
	/** Whether to use spatialization */
	UBOOL				bUseSpatialization;
	/** If TRUE, wave instance is requesting a restart */
	UBOOL				bIsRequestingRestart;

	/** Whether to apply audio effects */
	UBOOL				bApplyEffects;
	/** Whether to artificially keep active even at zero volume */
	UBOOL				bAlwaysPlay;
	/** Whether or not this sound plays when the game is paused in the UI */
	UBOOL				bIsUISound;
	/** Whether or not this wave is music */
	UBOOL				bIsMusic;
	/** Whether or not this wave is excluded from reverb */
	UBOOL				bNoReverb;
	/** Whether or not stereo sounds bleed to the rear speakers */
	UBOOL				bBleedStereo;

	/** Low pass filter setting */
	FLOAT				HighFrequencyGain;
	/** Current pitch */
	FLOAT				Pitch;
	/** Current velocity */
	FVector				Velocity;
	/** Current location */
	FVector				Location;
	/** Cached type hash */
	DWORD				TypeHash;
	/** GUID for mapping of USoundNode reference to USoundNodeWave. */
	QWORD				ParentGUID;

	/**
	 * Constructor, initializing all member variables.
	 *
	 * @param InAudioComponent	Audio component this wave instance belongs to.
	 */
	FWaveInstance( UAudioComponent* InAudioComponent );

	/**
	 * Stops the wave instance without notifying NotifyWaveInstanceFinishedHook. This will NOT stop wave instance
	 * if it is set up to loop indefinitely or set to remain active.
 	 */
	void StopWithoutNotification( void );

	/**
	 * Notifies the wave instance that the current playback buffer has finished.
	 */
	void NotifyFinished( void );

	/**
	 * Returns whether wave instance has finished playback.
	 *
	 * @return TRUE if finished, FALSE otherwise.
	 */
	UBOOL IsFinished( void );

	/**
	 * Friend archive function used for serialization.
	 */
	friend FArchive& operator<<( FArchive& Ar, FWaveInstance* WaveInstance );
};

inline DWORD GetTypeHash( FWaveInstance* A ) { return A->TypeHash; }

struct FAudioComponentSavedState
{
	USoundNode*								CurrentNotifyBufferFinishedHook;
	FVector									CurrentLocation;
	FLOAT									CurrentVolume;
	FLOAT									CurrentPitch;
	FLOAT									CurrentHighFrequencyGain;
	UBOOL									CurrentUseSpatialization;
	UBOOL									CurrentUseSeamlessLooping;

	void Set( UAudioComponent* AudioComponent );
	void Restore( UAudioComponent* AudioComponent );

	static void Reset( UAudioComponent* AudioComonent );
};

struct FAudioComponentParam
{
	FName	ParamName;
	FLOAT	FloatParam;
	USoundNodeWave* WaveParam;
};

struct AudioComponent_eventOnAudioFinished_Parms
{
    class UAudioComponent* AC;
    AudioComponent_eventOnAudioFinished_Parms(EEventParm)
    {
    }
};

struct AudioComponent_eventOcclusionChanged_Parms
{
    UBOOL bNowOccluded;
    AudioComponent_eventOcclusionChanged_Parms(EEventParm)
    {
    }
};

class UAudioComponent : public UActorComponent
{
	DECLARE_CLASS(UAudioComponent,UActorComponent,CLASS_NoExport,Engine);

	// Variables.
	class USoundCue*						SoundCue;
	USoundNode*								CueFirstNode;

	TArray<FAudioComponentParam>			InstanceParameters;

	/** Spatialise to the owner's coordinates */
	BITFIELD								bUseOwnerLocation:1;
	/** Auto start this component on creation */
	BITFIELD								bAutoPlay:1;
	/** Auto destroy this component on completion */
	BITFIELD								bAutoDestroy:1;
	/** Stop sound when owner is destroyed */
	BITFIELD								bStopWhenOwnerDestroyed:1;
	/** Whether the wave instances should remain active if they're dropped by the prioritization code. Useful for e.g. vehicle sounds that shouldn't cut out. */
	BITFIELD								bShouldRemainActiveIfDropped:1;
	/** whether we were occluded the last time we checked */
	BITFIELD								bWasOccluded:1;
	/** If true, subtitles in the sound data will be ignored. */
	BITFIELD								bSuppressSubtitles:1;
	/** Set to true when the component has resources that need cleanup */
	BITFIELD								bWasPlaying:1;
	/** Is this audio component allowed to be spatialized? */
	BITFIELD								bAllowSpatialization:1;
	/** Whether the current component has finished playing */
	BITFIELD								bFinished:1;
	/** Whether this audio component is previewing a sound */
	BITFIELD								bPreviewComponent:1;
	/** If TRUE, this sound will not be stopped when flushing the audio device. */
	BITFIELD								bIgnoreForFlushing:1;

	/**
	 * Properties of the audio component set by its owning sound class
	 */

	/** Whether audio effects are applied */
	BITFIELD								bApplyEffects:1;
	/** Whether to artificially prioritise the component to play */
	BITFIELD								bAlwaysPlay:1;
	/** Whether or not this sound plays when the game is paused in the UI */
	BITFIELD								bIsUISound:1;
	/** Whether or not this audio component is a music clip */
	BITFIELD								bIsMusic:1;
	/** Whether or not the audio component should be excluded from reverb EQ processing */
	BITFIELD								bNoReverb:1;
	/** Whether or not to bleed stereo files to the rear speakers */
	BITFIELD								bBleedStereo:1;

	TArray<FWaveInstance*>					WaveInstances;
	TArray<BYTE>							SoundNodeData;
	TMap<USoundNode*,UINT>					SoundNodeOffsetMap;
	TMultiMap<USoundNode*,FWaveInstance*>	SoundNodeResetWaveMap;
	const struct FListener*					Listener;

	FLOAT									PlaybackTime;
	class APortalVolume*					PortalVolume;
	FVector									Location;
	FVector									ComponentLocation;

	/** Remember the last owner so we can remove it from the actor's component array even if it's already been detached */
	class AActor*							LastOwner;

	/** Used by the subtitle manager to prioritize subtitles wave instances spawned by this component. */
	FLOAT									SubtitlePriority;

	FLOAT									FadeInStartTime;
	FLOAT									FadeInStopTime;
	/** This is the volume level we are fading to **/
	FLOAT									FadeInTargetVolume;

	FLOAT									FadeOutStartTime;
	FLOAT									FadeOutStopTime;
	/** This is the volume level we are fading to **/
	FLOAT									FadeOutTargetVolume;

	FLOAT									AdjustVolumeStartTime;
	FLOAT									AdjustVolumeStopTime;
	/** This is the volume level we are adjusting to **/
	FLOAT									AdjustVolumeTargetVolume;
	FLOAT									CurrAdjustVolumeTargetVolume;

	// Temporary variables for node traversal.
	USoundNode*								CurrentNotifyBufferFinishedHook;
	FVector									CurrentLocation;
	FLOAT									CurrentVolume;
	FLOAT									CurrentPitch;
	FLOAT									CurrentHighFrequencyGain;
	UBOOL									CurrentUseSpatialization;
	UBOOL									CurrentUseSeamlessLooping;

	// Multipliers used before propagation to WaveInstance
	FLOAT									CurrentVolumeMultiplier;
	FLOAT									CurrentPitchMultiplier;

	// Accumulators used before propagation to WaveInstance (not simply multiplicative)
	FLOAT									CurrentVoiceCenterChannelVolume;
	FLOAT									CurrentVoiceRadioVolume;

	// Serialized multipliers used to e.g. override volume for ambient sound actors.
	FLOAT									VolumeMultiplier;
	FLOAT									PitchMultiplier;

	/** while playing, this component will check for occlusion from its closest listener every this many seconds
	 * and call OcclusionChanged() if the status changes
	 */
	FLOAT									OcclusionCheckInterval;
	/** last time we checked for occlusion */
	FLOAT									LastOcclusionCheckTime;

	class UDrawSoundRadiusComponent*		PreviewSoundRadius;

	FScriptDelegate __OnAudioFinished__Delegate;

	// UObject interface.
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void Serialize(FArchive& Ar);
	virtual void FinishDestroy();

	/**
	 * This will return detail info about this specific object. (e.g. AudioComponent will return the name of the cue,
	 * ParticleSystemComponent will return the name of the ParticleSystem)  The idea here is that in many places
	 * you have a component of interest but what you really want is some characteristic that you can use to track
	 * down where it came from.  
	 *
	 */
	virtual FString GetDetailedInfoInternal() const;

	/** @name ActorComponent interface. */
	//@{
	virtual void Attach();
	virtual void Detach( UBOOL bWillReattach = FALSE );
	virtual void SetParentToWorld(const FMatrix& ParentToWorld);
	/**
	 * Returns whether component should be marked as pending kill inside AActor::MarkComponentsAsPendingKill if it has
	 * a say in it. Components that should continue playback after the deletion of their owner return FALSE in this case.
	 *
	 * @return TRUE if we allow being marked as pending kill, FALSE otherwise
	 */
	virtual UBOOL AllowBeingMarkedPendingKill();
	//@}

	// UAudioComponent interface.
	void SetSoundCue( USoundCue* NewSoundCue );
	void Play( void );
	void Stop( void );
	void UpdateWaveInstances( UAudioDevice* AudioDevice, TArray<FWaveInstance*> &WaveInstances, const TArray<struct FListener>& InListeners, FLOAT DeltaTime );

	/** Returns TRUE if this component is currently playing a SoundCue. */
	UBOOL IsPlaying();

	/**
 	 * This is called in place of "play".  So you will say AudioComponent->FadeIn().
	 * This is useful for fading in music or some constant playing sound.
	 *
	 * If FadeTime is 0.0, this is the same as calling Play() but just modifying the volume by
	 * FadeVolumeLevel. (e.g. you will play instantly but the FadeVolumeLevel will affect the AudioComponent
	 *
	 * If FadeTime is > 0.0, this will call Play(), and then increase the volume level of this
	 * AudioCompoenent to the passed in FadeVolumeLevel over FadeInTime seconds.
	 *
	 * The VolumeLevel is MODIFYING the AudioComponent's "base" volume.  (e.g.  if you have an
	 * AudioComponent that is volume 1000 and you pass in .5 as your VolumeLevel then you will fade to 500 )
	 *
	 * @param FadeInDuration how long it should take to reach the FadeVolumeLevel
	 * @param FadeVolumeLevel the percentage of the AudioComponents's calculated volume in which to fade to
	 **/
	void FadeIn( FLOAT FadeInDuration, FLOAT FadeVolumeLevel );

	/**
	 * This is called in place of "stop".  So you will say AudioComponent->FadeOut().
	 * This is useful for fading out music or some constant playing sound.
	 *
	 * If FadeTime is 0.0, this is the same as calling Stop().
	 *
	 * If FadeTime is > 0.0, this will decrease the volume level of this
	 * AudioCompoenent to the passed in FadeVolumeLevel over FadeInTime seconds.
	 *
	 * The VolumeLevel is MODIFYING the AudioComponent's "base" volume.  (e.g.  if you have an
	 * AudioComponent that is volume 1000 and you pass in .5 as your VolumeLevel then you will fade to 500 )
	 *
	 * @param FadeOutDuration how long it should take to reach the FadeVolumeLevel
	 * @param FadeVolumeLevel the percentage of the AudioComponents's calculated volume in which to fade to
	 **/
	void FadeOut( FLOAT FadeOutDuration, FLOAT FadeVolumeLevel );

	/**
 	 * This will allow one to adjust the volume of an AudioComponent on the fly
	 **/
	void AdjustVolume( FLOAT AdjustVolumeDuration, FLOAT AdjustVolumeLevel );

	/** if OcclusionCheckInterval > 0.0, checks if the sound has become (un)occluded during playback
	 * and calls eventOcclusionChanged() if so
	 * primarily used for gameplay-relevant ambient sounds
	 * CurrentLocation is the location of this component that will be used for playback
	 * @param ListenerLocation location of the closest listener to the sound
	 */
	void CheckOcclusion(const FVector& ListenerLocation);

	void SetFloatParameter(FName InName, FLOAT InFloat);
	void SetWaveParameter(FName InName, USoundNodeWave* InWave);
	UBOOL GetFloatParameter(FName InName, FLOAT& OutFloat);
	UBOOL GetWaveParameter(FName InName, USoundNodeWave*& OutWave);

	/**
	 * Dissociates component from audio device and deletes wave instances.
	 */
	void Cleanup( void );

	/** stops the audio (if playing), detaches the component, and resets the component's properties to the values of its template */
	void ResetToDefaults();

	DECLARE_FUNCTION(execPlay);
    DECLARE_FUNCTION(execPause);
	DECLARE_FUNCTION(execStop);
	DECLARE_FUNCTION(execIsPlaying);
	DECLARE_FUNCTION(execSetFloatParameter);
	DECLARE_FUNCTION(execSetWaveParameter);
	DECLARE_FUNCTION(execFadeIn);
	DECLARE_FUNCTION(execFadeOut);
	DECLARE_FUNCTION(execAdjustVolume);
	DECLARE_FUNCTION(execResetToDefaults)
	{
		P_FINISH;
		ResetToDefaults();
	}

	void delegateOnAudioFinished(UAudioComponent* AC)
	{
		AudioComponent_eventOnAudioFinished_Parms Parms(EC_EventParm);
		Parms.AC = AC;
		ProcessDelegate(FName(TEXT("OnAudioFinished")), &__OnAudioFinished__Delegate, &Parms);
	}

	void eventOcclusionChanged(UBOOL bNowOccluded)
	{
		AudioComponent_eventOcclusionChanged_Parms Parms(EC_EventParm);
		Parms.bNowOccluded=bNowOccluded ? FIRST_BITFIELD : FALSE;
		ProcessEvent(FindFunctionChecked(FName(TEXT("OcclusionChanged"))), &Parms);
	}

private:
	FLOAT GetFadeInMultiplier() const;
	FLOAT GetFadeOutMultiplier() const;
	/** Helper function to do determine the fade volume value based on start, start, target volume levels **/
	FLOAT FadeMultiplierHelper( FLOAT FadeStartTime, FLOAT FadeStopTime, FLOAT FadeTargetValue ) const;

	FLOAT GetAdjustVolumeOnFlyMultiplier();
};

/**
 * Detaches a component for the lifetime of this class.
 *
 * Typically used by constructing the class on the stack:
 * {
 *		FComponentReattachContext ReattachContext(this);
 *		// The component is removed from the scene here as ReattachContext is constructed.
 *		...
 * }	// The component is returned to the scene here as ReattachContext is destructed.
 */
class FComponentReattachContext
{
private:
	UActorComponent* Component;
	FSceneInterface* Scene;
	AActor* Owner;

public:
	FComponentReattachContext(UActorComponent* InComponent)
		: Scene(NULL)
		, Owner(NULL)
	{
		check(InComponent);
		checkf(!InComponent->HasAnyFlags(RF_Unreachable), TEXT("%s"), *InComponent->GetFullName());
		if((InComponent->IsAttached() || !InComponent->IsValidComponent()) && InComponent->GetScene())
		{
			Component = InComponent;

			// Detach the component from the scene.
			if(Component->bAttached)
			{
				Component->Detach( TRUE );	// Will reattach?
			}

			// Save the component's owner.
			Owner = Component->GetOwner();
			Component->Owner = NULL;

			// Save the scene and set the component's scene to NULL to prevent a nested FComponentReattachContext from reattaching this component.
			Scene = Component->GetScene();
			Component->Scene = NULL;
		}
		else
		{
			Component = NULL;
		}
	}
	~FComponentReattachContext()
	{
		if(Component && Component->IsValidComponent())
		{
			// If the component has been reattached since the recreate context was constructed, detach it so Attach isn't called on an already
			// attached component.
			if(Component->IsAttached())
			{
				Component->Detach( TRUE );	// WIll reattach?
			}

			// Since the component was attached when this context was constructed, we can assume that ParentToWorld has been called at some point.
			Component->Scene = Scene;
			Component->Owner = Owner;
			Component->Attach();
		}
	}
};

/**
 * Removes a component from the scene for the lifetime of this class. This code does NOT handle nested calls.
 *
 * Typically used by constructing the class on the stack:
 * {
 *		FPrimitiveSceneAttachmentContext ReattachContext(this);
 *		// The component is removed from the scene here as ReattachContext is constructed.
 *		...
 * }	// The component is returned to the scene here as ReattachContext is destructed.
 */
class FPrimitiveSceneAttachmentContext
{
private:
	/** Cached primitive passed in to constructor, can be NULL if we don't need to re-add. */
	UPrimitiveComponent* Primitive;
	/** Scene of cached primitive */
	FSceneInterface* Scene;

public:
	/** Constructor, removing primitive from scene and caching it if we need to readd */
	FPrimitiveSceneAttachmentContext(UPrimitiveComponent* InPrimitive );
	/** Destructor, adding primitive to scene again if needed. */
	~FPrimitiveSceneAttachmentContext();
};

/** Causes all components to be re-attached to the scene after class goes out of scope. */
class FGlobalPrimitiveSceneAttachmentContext
{
public:
	/** Initialization constructor. */
	FGlobalPrimitiveSceneAttachmentContext()
	{
		// wait until resources are released
		FlushRenderingCommands();

		// Detach all primitive components from the scene.
		for(TObjectIterator<UPrimitiveComponent> ComponentIt;ComponentIt;++ComponentIt)
		{
			new(ComponentContexts) FPrimitiveSceneAttachmentContext(*ComponentIt);
		}
	}

private:
	/** The recreate contexts for the individual components. */
	TIndirectArray<FPrimitiveSceneAttachmentContext> ComponentContexts;
};

/** Removes all components from their scenes for the lifetime of the class. */
class FGlobalComponentReattachContext
{
public:
	/** 
	* Initialization constructor. 
	*/
	FGlobalComponentReattachContext();
	
	/** 
	* Initialization constructor. 
	*
	* @param ExcludeComponents - Component types to exclude when reattaching 
	*/
	FGlobalComponentReattachContext(const TArray<UClass*>& ExcludeComponents);

	/** Destructor */
	~FGlobalComponentReattachContext();

	/** Indicates that a FGlobalComponentReattachContext is currently active */
	static INT ActiveGlobalReattachContextCount;

private:
	/** The recreate contexts for the individual components. */
	TIndirectArray<FComponentReattachContext> ComponentContexts;
};

/** Removes all components of the templated type from their scenes for the lifetime of the class. */
template<class ComponentType>
class TComponentReattachContext
{
public:
	/** Initialization constructor. */
	TComponentReattachContext()
	{
		// wait until resources are released
		FlushRenderingCommands();

		// Detach all components of the templated type.
		for(TObjectIterator<ComponentType> ComponentIt;ComponentIt;++ComponentIt)
		{
			new(ComponentContexts) FComponentReattachContext(*ComponentIt);
		}
	}

private:
	/** The recreate contexts for the individual components. */
	TIndirectArray<FComponentReattachContext> ComponentContexts;
};

/** Represents a wind source component to the scene manager in the rendering thread. */
class FWindSourceSceneProxy
{
public:

	/** Initialization constructor. */
	FWindSourceSceneProxy(const FVector& InDirection,FLOAT InStrength,FLOAT InPhase,FLOAT InFrequency,FLOAT InSpeed):
		Direction(InDirection),
		Strength(InStrength),
		Phase(InPhase),
		Frequency(InFrequency),
		InvSpeed(1.0f / InSpeed)
	{}

	/**
	 * Calculates the wind skew vector at a specific location and time.
	 * @param Location - The world-space location.
	 * @param CurrentWorldTime - The current world time.
	 * @return A vector representing the direction and strength of the wind.
	 */
	FVector GetWindSkew(const FVector& Location,FLOAT CurrentWorldTime) const;

private:

	FVector	Direction;
	FLOAT Strength;
	FLOAT Phase;
	FLOAT Frequency;
	FLOAT InvSpeed;
};

