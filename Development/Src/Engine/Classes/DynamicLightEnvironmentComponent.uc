/**
 * This is used to light components / actors during the game.  Doing something like:
 * LightEnvironment=FooLightEnvironment
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class DynamicLightEnvironmentComponent extends LightEnvironmentComponent
	native;

/** The current state of the light environment. */
var private native transient const pointer State{class FDynamicLightEnvironmentState};

/** The number of seconds between light environment updates for actors which aren't visible. */
var() float InvisibleUpdateTime;

/** Minimum amount of time that needs to pass between full environment updates. */
var() float MinTimeBetweenFullUpdates;

/** The number of visibility samples to use within the primitive's bounding volume. */
var() int NumVolumeVisibilitySamples;

/** The color of the ambient shadow. */
var() LinearColor AmbientShadowColor;

/** The direction of the ambient shadow source. */
var() vector AmbientShadowSourceDirection;

/** Ambient color added in addition to the level's lighting. */
var() LinearColor AmbientGlow;

/** Desaturation percentage of level lighting, which can be used to help team colored characters stand out better under colored lighting. */
var() float LightDesaturation;

/** The distance to create the light from the owner's origin, in radius units. */
var() float LightDistance;

/** The distance for the shadow to project beyond the owner's origin, in radius units. */
var() float ShadowDistance;

/** Whether the light environment should cast shadows */
var() bool bCastShadows;

/** Whether the light environment's shadow includes the effect of dynamic lights. */
var() bool bCompositeShadowsFromDynamicLights;

/** Time since the caster was last visible at which the mod shadow will fade out completely.  */
var() float ModShadowFadeoutTime;

/** Exponent that controls mod shadow fadeout curve. */
var() float ModShadowFadeoutExponent;

/**
 * Override for min dimensions (in texels) allowed for rendering shadow subject depths.
 * This also controls shadow fading, once the shadow resolution reaches MinShadowResolution it will be faded out completely.
 * A value of 0 defaults to MinShadowResolution in SystemSettings.
 */
var() int MinShadowResolution;

/**
 * Override for max square dimensions (in texels) allowed for rendering shadow subject depths.
 * A value of 0 defaults to MaxShadowResolution in SystemSettings.
 */
var() int MaxShadowResolution;

/** 
 * Resolution in texels below which shadows begin to be faded out. 
 * Once the shadow resolution reaches MinShadowResolution it will be faded out completely.
 * A value of 0 defaults to ShadowFadeResolution in SystemSettings.
 */
var() int ShadowFadeResolution;

/** Quality of shadow buffer filtering to use on the light environment */
var() EShadowFilterQuality ShadowFilterQuality;

/** Whether the light environment should be dynamically updated. */
var() bool bDynamic;

/** Whether a directional light should be used to synthesize the dominant lighting in the environment. */
var() bool bSynthesizeDirectionalLight;

/**
 * Whether a SH light should be used to synthesize all light not accounted for by the synthesized directional light.
 * If not, a sky light is used instead.
 */
var() bool bSynthesizeSHLight;

/** 
 * This is to allow individual DLEs to force override and get and SH light.  We need this for levels which have their
 * worldinfo's bAllowLightEnvSphericalHarmonicLights set to FALSE but then have cinematic levels added which were lit needing SH lights
 * to look good.
 **/
var() bool bForceAllowLightEnvSphericalHarmonicLights;


/** The type of shadowing to use for the environment's shadow. */
var() ELightShadowMode LightShadowMode;

/** The intensity of the simulated bounced light, as a fraction of the LightComponent's bounced lighting settings. */
var() float BouncedLightingFactor;

/**
 * The minimum angle to allow between the shadow direction and horizontal.  An angle > 0 constrains the shadow to never be cast from a light
 * below horizontal.
 */
var() float MinShadowAngle;

/** Whether this is an actor that can't tolerate latency in lighting updates; a full lighting update is done every frame. */
var() bool bRequiresNonLatentUpdates;

/* Whether to do visibility traces from the closest point on the bounds to the light, or just from the center of the bounds. */
var bool bTraceFromClosestBoundsPoint;

/* Whether to override the bounds calculated off of the owner's components with OverriddenBounds. */
var bool bOverrideOwnerBounds;

/* The bounds to use for visibility calculations if bOverrideOwnerBounds is enabled. */
var BoxSphereBounds OverriddenBounds;

/* Whether to override the lighting channels of the owner with OverriddenLightingChannels. */
var bool bOverrideOwnerLightingChannels;

/* The lighting channels to use if bOverrideOwnerLightingChannels is enabled. */
var LightingChannelContainer OverriddenLightingChannels;

cpptext
{
	// UObject interface.
	virtual void FinishDestroy();
	virtual void AddReferencedObjects( TArray<UObject*>& ObjectArray );
	virtual void Serialize(FArchive& Ar);

	// UActorComponent interface.
	virtual void Tick(FLOAT DeltaTime);
	virtual void Attach();
	virtual void UpdateTransform();
	virtual void Detach( UBOOL bWillReattach = FALSE );
	
	// ULightEnvironmentComponent interface.
	virtual void UpdateLight(const ULightComponent* Light);

	/* Forces a full update the of the dynamic and static environments on the next Tick. */
	void ResetEnvironment();

	friend class FDynamicLightEnvironmentState;
}

defaultproperties
{
	InvisibleUpdateTime=4.0
	MinTimeBetweenFullUpdates=0.7
	NumVolumeVisibilitySamples=1
	AmbientShadowColor=(R=0.001,G=0.001,B=0.001)
	AmbientShadowSourceDirection=(X=0,Y=0,Z=1)
	LightDistance=10.0
	ShadowDistance=1.0
	TickGroup=TG_PostUpdateWork
	bCastShadows=TRUE
	bCompositeShadowsFromDynamicLights=TRUE
	ModShadowFadeoutExponent=3.0
	MinShadowResolution=0
	MaxShadowResolution=0
	ShadowFadeResolution=0
	ShadowFilterQuality=SFQ_Low
	bDynamic=TRUE
	LightShadowMode=LightShadow_Modulate
	bSynthesizeDirectionalLight=TRUE
	bSynthesizeSHLight=FALSE
	BouncedLightingFactor=1.0
	MinShadowAngle=35.0
}
