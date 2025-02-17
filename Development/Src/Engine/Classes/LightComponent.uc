/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class LightComponent extends ActorComponent
	native
	noexport
	abstract
	collapsecategories;


//@warning: this structure is manually mirrored in UnActorComponent.h
struct LightingChannelContainer
{
	/** Whether the lighting channel has been initialized. Used to determine whether UPrimitveComponent::Attach should set defaults. */
	var		bool	bInitialized;
	// User settable channels that are auto set and true for lights
	var()	bool	BSP;
	var()	bool	Static;
	var()	bool	Dynamic;
	// User set channels.
	var()	bool	CompositeDynamic;
	var()	bool	Skybox;
	var()	bool	Unnamed_1;
	var()	bool	Unnamed_2;
	var()	bool	Unnamed_3;
	var()	bool	Unnamed_4;
	var()	bool	Unnamed_5;
	var()	bool	Unnamed_6;
	var()	bool	Cinematic_1;
	var()	bool	Cinematic_2;
	var()	bool	Cinematic_3;
	var()	bool	Cinematic_4;
	var()	bool	Cinematic_5;
	var()	bool	Cinematic_6;
	var()	bool	Cinematic_7;
	var()	bool	Cinematic_8;
	var()	bool	Cinematic_9;
	var()	bool	Cinematic_10;
	var()	bool	Gameplay_1;
	var()	bool	Gameplay_2;
	var()	bool	Gameplay_3;
	var()	bool	Gameplay_4;
	var()	bool	Crowd;
};

var native private	transient noimport const pointer	SceneInfo;

var native const	transient matrix			WorldToLight;
var native const	transient matrix			LightToWorld;

/**
 * GUID used to associate a light component with precomputed shadowing information across levels.
 * The GUID changes whenever the light position changes.
 */
var	const duplicatetransient guid	LightGuid;
/**
 * GUID used to associate a light component with precomputed shadowing information across levels.
 * The GUID changes whenever any of the lighting relevant properties changes.
 */
var const duplicatetransient guid	LightmapGuid;

var() const interp float Brightness;
var() const interp color LightColor;

var() const editinline export LightFunction Function;

/** The intensity of bounced lighting from this light in DynamicLightEnvironments. */
var() const interp float LightEnv_BouncedLightBrightness;

/** The color of bounced lighting from this light in DynamicLightEnvironments. */
var() const interp color LightEnv_BouncedModulationColor;

/** Is this light enabled? */
var() const bool bEnabled;

/**
 * True if the light can be blocked by shadow casting primitives.
 *
 * controls whether the light should cast shadows
 **/
var() const bool CastShadows;

/**
 * True if the light can be blocked by static shadow casting primitives.
 *
 * controls whether the light should cast shadows from objects that can receive static shadowing
 */
var() const bool CastStaticShadows;

/**
 * True if the light can be blocked by dynamic shadow casting primitives.
 *
 * controls whether the light should cast shadows from objects that cannot receive static shadowing
 **/
var() bool CastDynamicShadows;

/** True if the light should cast shadow from primitives which use a composite light environment. */
var() bool bCastCompositeShadow;

/** If bCastCompositeShadow=TRUE, whether the light should affect the composite shadow direction. */
var() bool bAffectCompositeShadowDirection;

/** deprecated:  This variable has been replaced with bForceDynamicLight. */
var deprecated const bool RequireDynamicShadows;

/**
 * True if this light should use dynamic shadows for all primitives.
 *
 * forces the use of shadow volumes/ stencil shadows which avoid potential issues with cubemaps and also avoids any memory used for static shadowing
 **/
var() const bool bForceDynamicLight;

/** Set to True to store the direct flux of this light in a light-map. */
var() const bool UseDirectLightMap;

/** Whether light has ever been built into a lightmap */
var const bool bHasLightEverBeenBuiltIntoLightMap;

/** Whether to only affect primitives that are in the same level/ share the same  GetOutermost() or are in the set of additionally specified ones. */
var() const bool bOnlyAffectSameAndSpecifiedLevels;

/** Whether the light can affect dynamic primitives even though the light is not affecting the dynamic channel. */
var() const bool bCanAffectDynamicPrimitivesOutsideDynamicChannel;

/** Whether to use the inclusion/ exclusion volumes. */
var() bool bUseVolumes;

/**
 * The light environment which the light affects.
 * NULL represents an implicit default light environment including all primitives and lights with LightEnvironment=NULL.
 */
var() const LightEnvironmentComponent LightEnvironment;

/** Array of other levels to affect if bOnlyAffectSameAndSpecifiedLevels is TRUE, own level always implicitly part of array. */
var() const array<name>	OtherLevelsToAffect;

/** Lighting channels controlling light/ primitive interaction. Only allows interaction if at least one channel is shared */
var() const LightingChannelContainer LightingChannels;

/**
 * Array of volumes used by AffectsBounds if bUseVolumes is set. Light will only affect primitives if they are touching or are contained
 * by at least one volume. Exclusion overrides inclusion.
 */
var() editoronly const array<brush>		InclusionVolumes;

/**
 * Array of volumes used by AffectsBounds if bUseVolumes is set. Light will only affect primitives if they neither touching nor are
 * contained by at least one volume. Exclusion overrides inclusion.
 */
var() editoronly const array<brush>		ExclusionVolumes;

/** Array of convex inclusion volumes, populated from InclusionVolumes by PostEditChange. */
var native const array<pointer> InclusionConvexVolumes;

/** Array of convex exclusion volumes, populated from ExclusionVolumes by PostEditChange. */
var native const array<pointer> ExclusionConvexVolumes;

//@warning: this structure is manually mirrored in UnActorComponent.h
enum ELightAffectsClassification
{
	LAC_USER_SELECTED,
	LAC_DYNAMIC_AFFECTING,
	LAC_STATIC_AFFECTING,
	LAC_DYNAMIC_AND_STATIC_AFFECTING
};

/**
 * This is the classification of this light.  This is used for placing a light for an explicit
 * purpose.  Basically you can now have "type" information with lights and understand the
 * intent of why a light was placed.  This is very useful for content people getting maps
 * from others and understanding why there is a dynamic affect light in the middle of the world
 * with a radius of 32k!  And also useful for being able to do searches such as the following:
 * show me all lights which effect dynamic objects.  Now show me the set of lights which are
 * not explicitly set as Dynamic Affecting lights.
 *
 **/
var() const editconst ELightAffectsClassification LightAffectsClassification;

enum ELightShadowMode
{
	/** Shadows rendered due to absence of light when doing dynamic lighting. Default, high overhead per-light. */
	LightShadow_Normal,
	/** Shadows rendered as a fullscreen pass by modulating entire scene by a shadow factor.  Least expensive. */
	LightShadow_Modulate,
	/** 
	* Shadows rendered as a fullscreen pass by modulating entire scene by a shadow factor, but also 
	* renders receiver geometry in order to prevent shadows on backfaces and emissive areas.  More expensive than LightShadow_Modulate.
	* This can be used to stop shadows from casting through walls in MP games at a cost of 0.1-0.2 ms on gpu per ModulateBetter caster
	*/
	LightShadow_ModulateBetter
};

/** Type of shadowing to apply for the light */
var() ELightShadowMode LightShadowMode;

/** Shadow color for modulating entire scene */
var() LinearColor ModShadowColor;

/** Time since the caster was last visible at which the mod shadow will fade out completely.  */
var() float ModShadowFadeoutTime;

/** Exponent that controls mod shadow fadeout curve. */
var() float ModShadowFadeoutExponent;

/** 
 * The munged index of this light in the light list 
 * 
 * > 0 == static light list
 *   0 == not part of any light list
 * < 0 == dynamic light list
 */
var const native duplicatetransient int LightListIndex;

enum EShadowProjectionTechnique
{
	/** Shadow projection is rendered using either PCF/VSM based on global settings  */
	ShadowProjTech_Default,
	/** Shadow projection is rendered using the PCF (Percentage Closer Filtering) technique. May have heavy banding artifacts */
	ShadowProjTech_PCF,
	/** Shadow projection is rendered using the VSM (Variance Shadow Map) technique. May have shadow offset and light bleed artifacts */
	ShadowProjTech_VSM,
	/** Shadow projection is rendered using the Low quality Branching PCF technique. May have banding and penumbra detection artifacts */
	ShadowProjTech_BPCF_Low,
	/** Shadow projection is rendered using the Medium quality Branching PCF technique. May have banding and penumbra detection artifacts */
	ShadowProjTech_BPCF_Medium,
	/** Shadow projection is rendered using the High quality Branching PCF technique. May have banding and penumbra detection artifacts */
	ShadowProjTech_BPCF_High
};
/** Type of shadow projection to use for this light */
var() EShadowProjectionTechnique ShadowProjectionTechnique;

enum EShadowFilterQuality
{
	SFQ_Low,
	SFQ_Medium,
	SFQ_High
};
/** Quality of shadow buffer filtering to use on this light */
var() EShadowFilterQuality ShadowFilterQuality;

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

/**
 * Toggles the light on or off
 *
 * @param bSetEnabled TRUE to enable the light or FALSE to disable it
 */
native final function SetEnabled(bool bSetEnabled);

/** sets Brightness, LightColor, and/or LightFunction */
native final function SetLightProperties(optional float NewBrightness = Brightness, optional color NewLightColor = LightColor, optional LightFunction NewLightFunction = Function);

/** Script interface to retrieve light location. */
native final function vector GetOrigin();

/** Script interface to retrieve light direction. */
native final function vector GetDirection();

/** Script interface to update the color and brightness on the render thread. */
native final function UpdateColorAndBrightness();


defaultproperties
{
	LightAffectsClassification=LAC_USER_SELECTED

	Brightness=1.0
	LightColor=(R=255,G=255,B=255)
	LightEnv_BouncedModulationColor=(R=255,G=255,B=255)
	bEnabled=TRUE

	// for now we are leaving this as people may be depending on it in script and we just
    // set the specific default settings in each light as they are all pretty different
	CastShadows=TRUE
	CastStaticShadows=TRUE
	CastDynamicShadows=TRUE
	bCastCompositeShadow=TRUE
	bAffectCompositeShadowDirection=TRUE
	bForceDynamicLight=FALSE
	UseDirectLightMap=FALSE

	LightingChannels=(BSP=TRUE,Static=TRUE,Dynamic=TRUE,CompositeDynamic=TRUE,bInitialized=TRUE)

	// default UE3 lighting mode
	LightShadowMode=LightShadow_Normal
	ModShadowFadeoutExponent=3.0
	// default to PCF shadow projection
	ShadowProjectionTechnique=ShadowProjTech_Default
	ShadowFilterQuality=SFQ_Low
}


/*

 Notes on all of the various Affecting Classifications


USER SELECTED:
   settings that god knows what they do


DYNAMIC AFFECTING:  // pawns, characters, kactors
	CastShadows=TRUE
	CastStaticShadows=FALSE
	CastDynamicShadows=TRUE
	bForceDynamicLight=TRUE
	UseDirectLightMap=FALSE

    LightingChannels:  Dynamic


STATIC AFFECTING:
	CastShadows=TRUE
	CastStaticShadows=TRUE
	CastDynamicShadows=FALSE
	bForceDynamicLight=FALSE
	UseDirectLightMap=TRUE   // For Toggleables this is UseDirectLightMap=FALSE

    LightingChannels:  BSP, Static


DYNAMIC AND STATIC AFFECTING:
	CastShadows=TRUE
	CastStaticShadows=TRUE
	CastDynamicShadows=TRUE
	bForceDynamicLight=FALSE
	UseDirectLightMap=FALSE

    LightingChannels:  BSP, Dynamic, Static


how to light the skybox?

  -> make a user selected affecting light with the skybox channel checked.
     - if we need to have a special classification for this then we will make it at a later time

SKYLIGHT:
	CastShadows=FALSE
	CastStaticShadows=FALSE
	CastDynamicShadows=FALSE
	bForceDynamicLight=FALSE
	UseDirectLightMap=TRUE

    LightingChannels:  SkyLight


how to only light character then?

  -> Character Lighting Channel  not at this time as people will mis use it
  -> for cinematics (where character only lighting could be used) we just use the unamed_#
	    lighting channels!


*/


