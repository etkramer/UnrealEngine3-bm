/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class FoliageComponent extends PrimitiveComponent
	native(Foliage);

/** Information about an instance of the component's foliage mesh that is common to all foliage instance types. */
struct native FoliageInstanceBase
{
	/** The instance's world-space location. */
	var Vector Location;
	
	/** The instance's X Axis. */
	var Vector XAxis;
	
	/** The instance's Y Axis. */
	var Vector YAxis;
	
	/** The instance's Z Axis. */
	var Vector ZAxis;

	/** The instance's distance factor, squared. */
	var float DistanceFactorSquared;
};

/** The information for each instance that is gathered during lighting and saved. */
struct native GatheredFoliageInstance extends FoliageInstanceBase
{
	/** 
	* The static lighting received by the instance. The number of coefficients corresponds to NUM_GATHERED_LIGHTMAP_COEF in native.
	*/
	var Color StaticLighting[4];
};

/** The component's foliage instances. */
var const array<GatheredFoliageInstance> Instances;

/** The lights included in the foliage's static lighting. */
var const array<guid> StaticallyRelevantLights;

/** The statically irrelevant lights for all the component's foliage instances. */
var const array<guid> StaticallyIrrelevantLights;

/** The scale factors applied to the directional static lighting. */
var const float DirectionalStaticLightingScale[3];

/** The scale factors applied to the simple static lighting. */
var const float SimpleStaticLightingScale[3];

/** The mesh which is drawn for each foliage instance. */
var const StaticMesh InstanceStaticMesh;

/** The material applied to the foliage instance mesh. */
var const MaterialInterface Material;

/** The maximum distance to draw foliage instances at. */
var float MaxDrawRadius;

/** The minimum distance to start scaling foliage instances away at. */
var float MinTransitionRadius;

/** The minimum distance to start thinning foliage instances at. */
var float MinThinningRadius;

/** The minimum scale to draw foliage instances at. */
var vector MinScale;

/** The minimum scale to draw foliage instances at. */
var vector MaxScale;

/** A scale for the effect of wind on the foliage mesh. */
var float SwayScale;

cpptext
{
	// UPrimitiveComponent interface.
	virtual void UpdateBounds();
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
	virtual void GetStaticLightingInfo(FStaticLightingPrimitiveInfo& OutPrimitiveInfo,const TArray<ULightComponent*>& InRelevantLights,const FLightingBuildOptions& Options);
	virtual void InvalidateLightingCache();
}

defaultproperties
{
	bAcceptsLights=TRUE
	bUsePrecomputedShadows=TRUE
	bForceDirectLightMap=TRUE
}
