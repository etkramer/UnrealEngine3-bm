/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class MaterialInterface extends Surface
	abstract
	native;

enum EMaterialUsage
{
	MATUSAGE_SkeletalMesh,
	MATUSAGE_FracturedMeshes,
	MATUSAGE_ParticleSprites,
	MATUSAGE_BeamTrails,
	MATUSAGE_ParticleSubUV,
	MATUSAGE_Foliage,
	MATUSAGE_SpeedTree,
	MATUSAGE_StaticLighting,
	MATUSAGE_GammaCorrection,
	MATUSAGE_LensFlare,
	MATUSAGE_InstancedMeshParticles,
	MATUSAGE_FluidSurface,
	MATUSAGE_Decals,
	MATUSAGE_MaterialEffect
};

/** A fence to track when the primitive is no longer used as a parent */
var native const transient RenderCommandFence_Mirror ParentRefFence{FRenderCommandFence};

cpptext
{
	/**
	 * Get the material which this is an instance of.
	 * Warning - This is platform dependent!  Do not call GetMaterial(GCurrentMaterialPlatform) and save that reference,
	 * as it will be different depending on the current platform.  Instead call GetMaterial(MSP_BASE) to get the base material and save that.
	 * When getting the material for rendering/checking usage, GetMaterial(GCurrentMaterialPlatform) is fine.
	 *
	 * @param Platform - The platform to get material for.
	 */
	virtual class UMaterial* GetMaterial(EMaterialShaderPlatform Platform = GCurrentMaterialPlatform) PURE_VIRTUAL(UMaterialInterface::GetMaterial,return NULL;);

	/**
	* Tests this material instance for dependency on a given material instance.
	* @param	TestDependency - The material instance to test this instance for dependency upon.
	* @return	True if the material instance is dependent on TestDependency.
	*/
	virtual UBOOL IsDependent(UMaterialInterface* TestDependency) { return FALSE; }

	/**
	* Returns a pointer to the FMaterialRenderProxy used for rendering.
	*
	* @param	Selected	specify TRUE to return an alternate material used for rendering this material when part of a selection
	*						@note: only valid in the editor!
	*
	* @return	The resource to use for rendering this material instance.
	*/
	virtual FMaterialRenderProxy* GetRenderProxy(UBOOL Selected) const PURE_VIRTUAL(UMaterialInterface::GetRenderProxy,return NULL;);

	/**
	* Returns a pointer to the physical material used by this material instance.
	* @return The physical material.
	*/
	virtual UPhysicalMaterial* GetPhysicalMaterial() const PURE_VIRTUAL(UMaterialInterface::GetPhysicalMaterial,return NULL;);

	/**
	 * Gathers the textures used to render the material instance.
	 * @param OutTextures	Upon return contains the textures used to render the material instance.
	 * @param bOnlyAddOnce	Whether to add textures that are sampled multiple times uniquely or not
	 */
	virtual void GetTextures(TArray<UTexture*>& OutTextures,UBOOL bOnlyAddOnce = TRUE) PURE_VIRTUAL(UMaterial::GetTextures,return;);

	/**
	 * Checks if the material can be used with the given usage flag.  
	 * If the flag isn't set in the editor, it will be set and the material will be recompiled with it.
	 * @param Usage - The usage flag to check
	 * @return UBOOL - TRUE if the material can be used for rendering with the given type.
	 */
	virtual UBOOL CheckMaterialUsage(EMaterialUsage Usage) PURE_VIRTUAL(UMaterialInterface::CheckMaterialUsage,return FALSE;);

	/**
	* Allocates a new material resource
	* @return	The allocated resource
	*/
	virtual FMaterialResource* AllocateResource() PURE_VIRTUAL(UMaterialInterface::AllocateResource,return NULL;);

	/**
	 * Gets the static permutation resource if the instance has one
	 * @return - the appropriate FMaterialResource if one exists, otherwise NULL
	 */
	virtual FMaterialResource * GetMaterialResource(EMaterialShaderPlatform Platform = GCurrentMaterialPlatform) { return NULL; }

	/**
	 * Compiles a FMaterialResource on the given platform with the given static parameters
	 *
	 * @param StaticParameters - The set of static parameters to compile for
	 * @param StaticPermutation - The resource to compile
	 * @param Platform - The platform to compile for
	 * @param MaterialPlatform - The material platform to compile for
	 * @param bFlushExistingShaderMaps - Indicates that existing shader maps should be discarded
	 * @return TRUE if compilation was successful or not necessary
	 */
	virtual UBOOL CompileStaticPermutation(
		FStaticParameterSet* StaticParameters, 
		FMaterialResource* StaticPermutation, 
		EShaderPlatform Platform, 
		EMaterialShaderPlatform MaterialPlatform,
		UBOOL bFlushExistingShaderMaps,
		UBOOL bDebugDump)
		PURE_VIRTUAL(UMaterialInterface::CompileStaticPermutation,return FALSE;);

	/**
	* Gets the value of the given static switch parameter
	*
	* @param	ParameterName	The name of the static switch parameter
	* @param	OutValue		Will contain the value of the parameter if successful
	* @return					True if successful
	*/
	virtual UBOOL GetStaticSwitchParameterValue(FName ParameterName,UBOOL &OutValue,FGuid &OutExpressionGuid) 
		PURE_VIRTUAL(UMaterialInterface::GetStaticSwitchParameterValue,return FALSE;);

	/**
	* Gets the value of the given static component mask parameter
	*
	* @param	ParameterName	The name of the parameter
	* @param	R, G, B, A		Will contain the values of the parameter if successful
	* @return					True if successful
	*/
	virtual UBOOL GetStaticComponentMaskParameterValue(FName ParameterName, UBOOL &R, UBOOL &G, UBOOL &B, UBOOL &A, FGuid &OutExpressionGuid) 
		PURE_VIRTUAL(UMaterialInterface::GetStaticComponentMaskParameterValue,return FALSE;);

	virtual UBOOL IsFallbackMaterial() { return FALSE; }

	/** @return The material's view relevance. */
	FMaterialViewRelevance GetViewRelevance();

	INT GetWidth() const;
	INT GetHeight() const;

	// USurface interface
	virtual FLOAT GetSurfaceWidth() const { return GetWidth(); }
	virtual FLOAT GetSurfaceHeight() const { return GetHeight(); }

	// UObject interface
	virtual void BeginDestroy();
	virtual UBOOL IsReadyForFinishDestroy();
}

/** The mesh used by the material editor to preview the material.*/
var() editoronly string PreviewMesh;

native final noexport function Material GetMaterial();

/**
* Returns a pointer to the physical material used by this material instance.
* @return The physical material.
*/
native final noexport function PhysicalMaterial GetPhysicalMaterial() const;

// Get*ParameterValue - Gets the entry from the ParameterValues for the named parameter.
// Returns false is parameter is not found.


native function bool GetFontParameterValue(name ParameterName,out font OutFontValue, out int OutFontPage);
native function bool GetScalarParameterValue(name ParameterName, out float OutValue);
native function bool GetScalarCurveParameterValue(name ParameterName, out InterpCurveFloat OutValue);
native function bool GetTextureParameterValue(name ParameterName, out Texture OutValue);
native function bool GetVectorParameterValue(name ParameterName, out LinearColor OutValue);
native function bool GetVectorCurveParameterValue(name ParameterName, out InterpCurveVector OutValue);

/**
 * Forces the streaming system to disregard the normal logic for the specified duration and
 * instead always load all mip-levels for all textures used by this material.
 *
 * @param ForceDuration	- Number of seconds to keep all mip-levels in memory, disregarding the normal priority logic.
 */
native function SetForceMipLevelsToBeResident( float ForceDuration );

defaultproperties
{

}
