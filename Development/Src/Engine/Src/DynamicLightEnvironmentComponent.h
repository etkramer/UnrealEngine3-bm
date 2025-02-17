/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * The character lighting stats.
 */
enum ECharacterLightingStats
{
	STAT_LightVisibilityTime = STAT_CharacterLightingFirstStat,
	STAT_UpdateStaticEnvironmentTime,
	STAT_UpdateDynamicEnvironmentTime,
	STAT_UpdateEnvironmentInterpolationTime,
	STAT_CreateLightsTime,
	STAT_NumEnvironments,
	STAT_StaticEnvironmentUpdates,
	STAT_DynamicEnvironmentUpdates,
	STAT_DynamicLightEnvironmentComponentTickTime
};

DECLARE_STATS_GROUP(TEXT("CharacterLighting"),STATGROUP_CharacterLighting);
DECLARE_CYCLE_STAT(TEXT("Light visibility time"),STAT_LightVisibilityTime,STATGROUP_CharacterLighting);
DECLARE_CYCLE_STAT(TEXT("UpdateStaticEnvironment time"),STAT_UpdateStaticEnvironmentTime,STATGROUP_CharacterLighting);
DECLARE_CYCLE_STAT(TEXT("UpdateDynamicEnvironment time"),STAT_UpdateDynamicEnvironmentTime,STATGROUP_CharacterLighting);
DECLARE_CYCLE_STAT(TEXT("UpdateEnvironmentInterpolation time"),STAT_UpdateEnvironmentInterpolationTime,STATGROUP_CharacterLighting);
DECLARE_CYCLE_STAT(TEXT("CreateLights time"),STAT_CreateLightsTime,STATGROUP_CharacterLighting);
DECLARE_DWORD_COUNTER_STAT(TEXT("Light Environments"),STAT_NumEnvironments,STATGROUP_CharacterLighting);
DECLARE_DWORD_COUNTER_STAT(TEXT("Environment Updates"),STAT_StaticEnvironmentUpdates,STATGROUP_CharacterLighting);
DECLARE_DWORD_COUNTER_STAT(TEXT("Environment Updates"),STAT_DynamicEnvironmentUpdates,STATGROUP_CharacterLighting);
DECLARE_CYCLE_STAT(TEXT("DynamicLightEnvComp Tick"),STAT_DynamicLightEnvironmentComponentTickTime,STATGROUP_CharacterLighting);

/** The private light environment state. */
class FDynamicLightEnvironmentState
{
public:

	/** Initialization constructor. */
	FDynamicLightEnvironmentState(UDynamicLightEnvironmentComponent* InComponent);

	/** Computes the bounds and various lighting relevance attributes of the owner and its primitives. */
	void UpdateOwner();

	/** Updates the contribution of static lights to the light environment. */
	void UpdateStaticEnvironment();
	
	/** Updates the contribution of dynamic lights to the light environment. */
	void UpdateDynamicEnvironment();

	/** Interpolates toward the target light environment state. */
	void UpdateEnvironmentInterpolation(FLOAT DeltaTime,FLOAT TimeBetweenUpdates);

	/** Performs a full update of the light environment. */
	void Update();

	/** Updates the light environment's state. */
	void Tick(FLOAT DeltaTime);

	/** Creates a light to represent the light environment's composite lighting. */
	ULightComponent* CreateRepresentativeLight(const FVector& Direction,const FLinearColor& Intensity);

	/** Creates a light to represent the light environment's composite shadowing. */
	UPointLightComponent* CreateRepresentativeShadowLight(const FVector& Direction,const FLinearColor& Intensity);

	/** Detaches the light environment's representative lights. */
	void DetachRepresentativeLights();

	/** Creates the lights to represent the character's light environment. */
	void CreateEnvironmentLightList(UBOOL bForceUpdate = FALSE);

	/** Builds a list of objects referenced by the state. */
	void AddReferencedObjects(TArray<UObject*>& ObjectArray);

	/** Forces a full update on the next Tick. */
	void ResetEnvironment();

private:

	/** The component which this is state for. */
	UDynamicLightEnvironmentComponent* Component;

	/** The bounds of the owner. */
	FBoxSphereBounds OwnerBounds;

	/** The predicted center of the owner at the time of the next update. */
	FVector PredictedOwnerPosition;

	/** The lighting channels the owner's primitives are affected by. */
	FLightingChannelContainer OwnerLightingChannels;

	/** The owner's level. */
	UPackage* OwnerPackage;

	/** The time the light environment was last updated. */
	FLOAT LastUpdateTime;

	/** Time between updates for invisible objects. */
	FLOAT InvisibleUpdateTime;

	/** Min time between full environment updates. */
	FLOAT MinTimeBetweenFullUpdates;

	/** A pool of unused light components. */
	mutable TArray<ULightComponent*> RepresentativeLightPool;

	/** The character's current static light environment. */
	FSHVectorRGB StaticLightEnvironment;
	/** The current static non-shadowed light environment. */
	FSHVectorRGB StaticNonShadowedLightEnvironment;
	/** The current static shadow environment. */
	FSHVectorRGB StaticShadowEnvironment;

	/** The current dynamic light environment. */
	FSHVectorRGB DynamicLightEnvironment;
	/** The current dynamic non-shadowed light environment. */
	FSHVectorRGB DynamicNonShadowedLightEnvironment;
	/** The current dynamic shadow environment. */
	FSHVectorRGB DynamicShadowEnvironment;

	/** New static light environment to interpolate to. */
	FSHVectorRGB NewStaticLightEnvironment;
	/** New static non-shadowed light environment to interpolate to. */
	FSHVectorRGB NewStaticNonShadowedLightEnvironment;
	/** New static shadow environment to interpolate to. */
	FSHVectorRGB NewStaticShadowEnvironment;

	/** The lighting which was used to create the current representative lights. */
	FSHVectorRGB CurrentRepresentativeLightEnvironment;
	/** The non-shadowed lighting which was used to create the current representative lights. */
	FSHVectorRGB CurrentRepresentativeNonShadowedLightEnvironment;
	/** The lighting which was used to create the current representative shadow. */
	FSHVectorRGB CurrentRepresentativeShadowEnvironment;

	/** The current shadow light's direction. */
	FVector CurrentShadowDirection;

	/** The current representative shadow-casting light. */
	UPointLightComponent* CurrentRepresentativeShadowLight;

	/** Whether light environment has been fully updated at least once. */
	UBOOL bFirstFullUpdate;

	/** The positions relative to the owner's bounding box which are sampled for light visibility. */
	TArray<FVector> LightVisibilitySamplePoints;

	/**
	 * Determines whether a light is visible, using cached results if available.
	 * @param Light - The light to test visibility for.
	 * @param OwnerPosition - The position of the owner to compute the light's effect for.
	 * @param OutVisibilityFactor - Upon return, contains an appromiate percentage of light that reaches the owner's primitives.
	 * @return TRUE if the light reaches the owner's primitives.
	 */
	UBOOL IsLightVisible(const ULightComponent* Light,const FVector& OwnerPosition,FLOAT& OutVisibilityFactor);

	/** Allocates a light, attempting to reuse a light with matching type from the free light pool. */
	template<typename LightType>
	LightType* AllocateLight() const;

	/**
	 * Tests whether a light affects the owner.
	 * @param Light - The light to test.
	 * @param OwnerPosition - The position of the owner to compute the light's effect for.
	 * @return TRUE if the light affects the owner.
	 */
	UBOOL DoesLightAffectOwner(const ULightComponent* Light,const FVector& OwnerPosition);

	/**
	 * Adds the light's contribution to the light environment.
	 * @param	Light						light to add
	 * @param	LightEnvironment			light environment to add the light's contribution to
	 * @param	NonShadowedLightEnvironment	light environment to add the non-shadowed part of the light's contribution to
	 * @param	ShadowEnvironment			The shadow environment to add the light's shadowing to.
	 * @param	OwnerPosition				The position of the owner to compute the light's effect for.
	 * @param	bIsDynamic					Whether the light is dynamic.
	 */
	void AddLightToEnvironment(
		const ULightComponent* Light, 
		FSHVectorRGB& LightEnvironment,
		FSHVectorRGB& NonShadowedLightEnvironment,
		FSHVectorRGB& ShadowEnvironment,
		const FVector& OwnerPosition,
		UBOOL bIsDynamic
		);
};
