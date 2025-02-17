/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class MaterialInstance extends MaterialInterface
	abstract
	native(Material);


/** Physical material to use for this graphics material. Used for sounds, effects etc.*/
var() PhysicalMaterial PhysMaterial;

var() const MaterialInterface Parent;

/** indicates whether the instance has static permutation resources (which are required when static parameters are present) */
var bool bHasStaticPermutationResource;

/** indicates whether the static permutation resource needs to be updated on PostEditChange() */
var native transient bool bStaticPermutationDirty;

/**
* The set of static parameters that this instance will be compiled with, one for sm3 and one for sm2.
* This is indexed by EMaterialShaderPlatform.
*/
var const native duplicatetransient pointer StaticParameters[2]{FStaticParameterSet};

/**
* The material resources for this instance, one for sm3 and one for sm2.
* This is indexed by EMaterialShaderPlatform.
*/
var const native duplicatetransient pointer StaticPermutationResources[2]{FMaterialResource};

var const native duplicatetransient pointer Resources[2]{class FMaterialInstanceResource};


var private const native bool ReentrantFlag;

/**
 * Array of textures referenced, updated in PostLoad.  These are needed to keep the textures used by material resources
 * from getting destroyed by realtime GC.
 */
var private const array<texture> ReferencedTextures;

cpptext
{
	// Constructor.
	UMaterialInstance();

	/**
	* Passes the allocation request up the MIC chain
	* @return	The allocated resource
	*/
	FMaterialResource* AllocateResource();

	/** Initializes the material instance's resources. */
	virtual void InitResources();

	/**
	 * Gets the static permutation resource if the instance has one
	 * @return - the appropriate FMaterialResource if one exists, otherwise NULL
	 */
	virtual FMaterialResource * GetMaterialResource(EMaterialShaderPlatform Platform = GCurrentMaterialPlatform);

	// UMaterialInterface interface.

	/**
	 * Get the material which this is an instance of.
	 * Warning - This is platform dependent!  Do not call GetMaterial(GCurrentMaterialPlatform) and save that reference,
	 * as it will be different depending on the current platform.  Instead call GetMaterial(MSP_BASE) to get the base material and save that.
	 * When getting the material for rendering/checking usage, GetMaterial(GCurrentMaterialPlatform) is fine.
	 *
	 * @param Platform - The platform to get material for.
	 */
	virtual UMaterial* GetMaterial(EMaterialShaderPlatform Platform = GCurrentMaterialPlatform);

	/**
	 * Gathers the textures used to render the material instance.
	 * @param OutTextures	Upon return contains the textures used to render the material instance.
	 * @param bOnlyAddOnce	Whether to add textures that are sampled multiple times uniquely or not
	 */
	virtual void GetTextures(TArray<UTexture*>& OutTextures,UBOOL bOnlyAddOnce = TRUE);

	/**
	 * Checks if the material can be used with the given usage flag.
	 * If the flag isn't set in the editor, it will be set and the material will be recompiled with it.
	 * @param Usage - The usage flag to check
	 * @return UBOOL - TRUE if the material can be used for rendering with the given type.
	 */
	virtual UBOOL CheckMaterialUsage(EMaterialUsage Usage);

	/**
	* Gets the value of the given static switch parameter.  If it is not found in this instance then
	*		the request is forwarded up the MIC chain.
	*
	* @param	ParameterName	The name of the static switch parameter
	* @param	OutValue		Will contain the value of the parameter if successful
	* @return					True if successful
	*/
	virtual UBOOL GetStaticSwitchParameterValue(FName ParameterName,UBOOL &OutValue,FGuid &OutExpressionGuid);

	/**
	* Gets the value of the given static component mask parameter. If it is not found in this instance then
	*		the request is forwarded up the MIC chain.
	*
	* @param	ParameterName	The name of the parameter
	* @param	R, G, B, A		Will contain the values of the parameter if successful
	* @return					True if successful
	*/
	virtual UBOOL GetStaticComponentMaskParameterValue(FName ParameterName, UBOOL &R, UBOOL &G, UBOOL &B, UBOOL &A,FGuid &OutExpressionGuid);

	virtual UBOOL IsDependent(UMaterialInterface* TestDependency);
	virtual FMaterialRenderProxy* GetRenderProxy(UBOOL Selected) const;
	virtual UPhysicalMaterial* GetPhysicalMaterial() const;

	/**
	* Makes a copy of all the instance's inherited and overriden static parameters
	*
	* @param StaticParameters - The set of static parameters to fill, must be empty
	*/
	void GetStaticParameterValues(FStaticParameterSet * StaticParameters);

	/**
	* Sets the instance's static parameters and marks it dirty if appropriate.
	*
	* @param	EditorParameters	The new static parameters.  If the set does not contain any static parameters,
	*								the static permutation resource will be released.
	* @return		TRUE if the static permutation resource has been marked dirty
	*/
	UBOOL SetStaticParameterValues(const FStaticParameterSet* EditorParameters);

	/**
	* Compiles the static permutation resource if the base material has changed and updates dirty states
	*/
	void UpdateStaticPermutation();

	/**
	* Updates static parameters and recompiles the static permutation resource if necessary
	*/
	void InitStaticPermutation();

	/**
	* Compiles material resources for the given platform if the shader map for that resource didn't already exist.
	*
	* @param ShaderPlatform - the platform to compile for.
	* @param bFlushExistingShaderMaps - forces a compile, removes existing shader maps from shader cache.
	* @param bForceAllPlatforms - compile for all platforms, not just the current.
	*/
	void CacheResourceShaders(EShaderPlatform ShaderPlatform, UBOOL bFlushExistingShaderMaps=FALSE, UBOOL bForceAllPlatforms=FALSE, UBOOL bDebugDump=FALSE);

	/**
	 * Passes the compile request up the MIC chain
	 *
	 * @param StaticParameters - The set of static parameters to compile for
	 * @param StaticPermutation - The resource to compile
	 * @param Platform - The platform to compile for
	 * @param MaterialPlatform - The material platform to compile for
	 * @param bFlushExistingShaderMaps - Indicates that existing shader maps should be discarded
	 * @return TRUE if compilation was successful or not necessary
	 */
	UBOOL CompileStaticPermutation(
		FStaticParameterSet* Permutation,
		FMaterialResource* StaticPermutation,
		EShaderPlatform Platform,
		EMaterialShaderPlatform MaterialPlatform,
		UBOOL bFlushExistingShaderMaps,
		UBOOL bDebugDump);

	/**
	* Allocates the static permutation resources for all platforms if they haven't been already.
	* Also updates the material resource's Material member as it may have changed.
	* (This can happen if a Fallback Material is assigned after the material resource is created)
	*/
	void AllocateStaticPermutations();

	/**
	* Releases the static permutation resource if it exists, in a thread safe way
	*/
	void ReleaseStaticPermutations();

	/** === USurface interface === */
	/**
	 * Method for retrieving the width of this surface.
	 *
	 * This implementation returns the maximum width of all textures applied to this material - not exactly accurate, but best approximation.
	 *
	 * @return	the width of this surface, in pixels.
	 */
	virtual FLOAT GetSurfaceWidth() const;
	/**
	 * Method for retrieving the height of this surface.
	 *
	 * This implementation returns the maximum height of all textures applied to this material - not exactly accurate, but best approximation.
	 *
	 * @return	the height of this surface, in pixels.
	 */
	virtual FLOAT GetSurfaceHeight() const;

	// UObject interface.
	void PreSave();
	virtual void Serialize(FArchive& Ar);
	virtual void PostLoad();
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void BeginDestroy();
	virtual UBOOL IsReadyForFinishDestroy();
	virtual void FinishDestroy();

	/**
	* Refreshes parameter names using the stored reference to the expression object for the parameter.
	*/
	virtual void UpdateParameterNames();

#if !FINAL_RELEASE
	/** Displays warning if this material instance is not safe to modify in the game (is in content package) */
	void CheckSafeToModifyInGame(TCHAR* FuncName) const;
#endif
};



// SetParent - Updates the parent.

native function SetParent(MaterialInterface NewParent);

// Set*ParameterValue - Updates the entry in ParameterValues for the named parameter, or adds a new entry.

native function SetVectorParameterValue(name ParameterName, LinearColor Value);
native function SetScalarParameterValue(name ParameterName, float Value);
native function SetScalarCurveParameterValue(name ParameterName, InterpCurveFloat Value);
native function SetTextureParameterValue(name ParameterName, Texture Value);

/**
* Sets the value of the given font parameter.
*
* @param	ParameterName	The name of the font parameter
* @param	OutFontValue	New font value to set for this MIC
* @param	OutFontPage		New font page value to set for this MIC
*/
native function SetFontParameterValue(name ParameterName, Font FontValue, int FontPage);

/** Removes all parameter values */
native function ClearParameterValues();

/**
 *	Returns if this MI is either in a map package or the transient package
 *	During gameplay, Set..Parameter should only be called on MIs where this is TRUE -
 *	otherwise you are modifying an MI within a content package, that will persist across level reload etc.
 */
native function bool IsInMapOrTransientPackage() const;

defaultproperties
{
	bHasStaticPermutationResource=False
}
