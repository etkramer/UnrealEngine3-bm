/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class StaticMeshComponent extends MeshComponent
	native
	noexport
	hidecategories(Object)
	editinlinenew;


/** If 0, auto-select LOD level. if >0, force to (ForcedLodModel-1). */
var() int		ForcedLodModel; 
var int			PreviousLODLevel; // Previous LOD level

var() const StaticMesh StaticMesh;
var() Color WireframeColor;

/**
 *	Ignore this instance of this static mesh when calculating streaming information.
 *	This can be useful when doing things like applying character textures to static geometry,
 *	to avoid them using distance-based streaming.
 */
var()	bool	bIgnoreInstanceForTextureStreaming;

/** Whether to override the lightmap resolution defined in the static mesh */
var() const bool bOverrideLightMapResolution;
/** Light map resolution used if bOverrideLightMapResolution is TRUE */
var() const int	 OverriddenLightMapResolution;


/** Subdivision step size for static vertex lighting.				*/
var(AdvancedLighting) const int	SubDivisionStepSize;
/** Minimum number of subdivisions, needs to be at least 2.			*/
var(AdvancedLighting) const int MinSubDivisions;
/** Maximum number of subdivisions.									*/
var(AdvancedLighting) const int MaxSubDivisions;
/** Whether to use subdivisions or just the triangle's vertices.	*/
var(AdvancedLighting) const bool bUseSubDivisions;
/** if True then decals will always use the fast path and will be treated as static wrt this mesh */
var const transient bool bForceStaticDecals;

var const array<Guid>	IrrelevantLights;

struct StaticMeshComponentLODInfo
{
	var private const array<ShadowMap2D> ShadowMaps;
	var private const array<Object> ShadowVertexBuffers;
	var native private const pointer LightMap{FLightMap};
};

var native private const array<StaticMeshComponentLODInfo> LODData;

/** Change the StaticMesh used by this instance. */
simulated native function bool SetStaticMesh( StaticMesh NewMesh, optional bool bForce );

/** Disables physics collision between a specific pair of meshes. */
simulated native function DisableRBCollisionWithSMC( StaticMeshComponent OtherSMC, bool bDisabled );

/**
 * Changes the value of bForceStaticDecals.
 * @param bInForceStaticDecals - The value to assign to bForceStaticDecals.
 */
native final function SetForceStaticDecals(bool bInForceStaticDecals);

defaultproperties
{
	// Various physics related items need to be ticked pre physics update
	TickGroup=TG_PreAsyncWork

	CollideActors=True
	BlockActors=True
	BlockZeroExtent=True
	BlockNonZeroExtent=True
	BlockRigidBody=True
	WireframeColor=(R=0,G=255,B=255,A=255)
	bAcceptsStaticDecals=TRUE
	bAcceptsDecals=TRUE
	bOverrideLightMapResolution=TRUE
	bUsePrecomputedShadows=FALSE
	SubDivisionStepSize=16
	MinSubDivisions=2
	MaxSubDivisions=8
	bUseSubDivisions=TRUE
	bForceStaticDecals=FALSE
}
