/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Material extends MaterialInterface
	native
	hidecategories(object)
	collapsecategories;

enum EBlendMode
{
	BLEND_Opaque,
	BLEND_Masked,
	BLEND_Translucent,
	BLEND_Additive,
	BLEND_Modulate
};

enum EMaterialLightingModel
{
	MLM_Phong,
	MLM_NonDirectional,
	MLM_Unlit,
	MLM_SHPRT,
	MLM_Custom
};

// Material input structs.

struct MaterialInput
{
	var MaterialExpression	Expression;
	var int					Mask,
							MaskR,
							MaskG,
							MaskB,
							MaskA;
	var int					GCC64_Padding; // @todo 64: if the C++ didn't mismirror this structure (with ExpressionInput), we might not need this
};

struct ColorMaterialInput extends MaterialInput
{
	var bool	UseConstant;
	var color	Constant;
};

struct ScalarMaterialInput extends MaterialInput
{
	var bool	UseConstant;
	var float	Constant;
};

struct VectorMaterialInput extends MaterialInput
{
	var bool	UseConstant;
	var vector	Constant;
};

struct Vector2MaterialInput extends MaterialInput
{
	var bool	UseConstant;
	var float	ConstantX,
				ConstantY;
};

// Physics.

/** Physical material to use for this graphics material. Used for sounds, effects etc.*/
var() PhysicalMaterial		PhysMaterial;

/** For backwards compatibility only. */
var class<PhysicalMaterial>	PhysicalMaterial;

// Reflection.

//NOTE: If any additional inputs are added/removed WxMaterialEditor::GetVisibleMaterialParameters() must be updated
var ColorMaterialInput		DiffuseColor;
var ScalarMaterialInput		DiffusePower;
var ColorMaterialInput		SpecularColor;
var ScalarMaterialInput		SpecularPower;
var VectorMaterialInput		Normal;

// Emission.

var ColorMaterialInput		EmissiveColor;

// Transmission.

var ScalarMaterialInput		Opacity;
var ScalarMaterialInput		OpacityMask;

/** If BlendMode is BLEND_Masked, the surface is not rendered where OpacityMask < OpacityMaskClipValue. */
var() float OpacityMaskClipValue;

/** Allows the material to distort background color by offsetting each background pixel by the amount of the distortion input for that pixel. */
var Vector2MaterialInput	Distortion;

/** Determines how the material's color is blended with background colors. */
var() EBlendMode BlendMode;

/** Determines how inputs are combined to create the material's final color. */
var() EMaterialLightingModel LightingModel;

var ColorMaterialInput		CustomLighting;

/** Lerps between lighting color (diffuse * attenuation * Lambertian) and lighting without the Lambertian term color (diffuse * attenuation * TwoSidedLightingColor). */
var ScalarMaterialInput		TwoSidedLightingMask;

/** Modulates the lighting without the Lambertian term in two sided lighting. */
var ColorMaterialInput		TwoSidedLightingColor;

/** Indicates that the material should be rendered without backface culling and the normal should be flipped for backfaces. */
var() bool TwoSided;

/**
 * Allows the material to disable depth tests, which is only meaningful with translucent blend modes.
 * Disabling depth tests will make rendering significantly slower since no occluded pixels can get zculled.
 */
var(Translucency) bool bDisableDepthTest;

/** Whether the material should allow fog or be unaffected by fog.  This only has meaning for materials with translucent blend modes. */
var(Translucency) bool bAllowFog;

/**
 * Whether the material should use one-layer distortion, which can be cheaper than normal distortion for some primitive types (mainly fluid surfaces).
 * One layer distortion won't handle overlapping one layer distortion primitives correctly.
 * This causes an extra scene color resolve for the first primitive that uses one layer distortion and so should only be used in very specific circumstances.
 */
var(Translucency) bool bUseOneLayerDistortion;

var(Usage) const bool bUsedAsLightFunction;
var(Usage) const bool bUsedWithFogVolumes;
var(Usage) const bool bUsedAsSpecialEngineMaterial;
var(Usage) const bool bUsedWithSkeletalMesh;
var(Usage) const bool bUsedWithFracturedMeshes;
var		   const bool bUsedWithParticleSystem;
var(Usage) const bool bUsedWithParticleSprites;
var(Usage) const bool bUsedWithBeamTrails;
var(Usage) const bool bUsedWithParticleSubUV;
var(Usage) const bool bUsedWithFoliage;
var(Usage) const bool bUsedWithSpeedTree;
var(Usage) const bool bUsedWithStaticLighting;
var(Usage) const bool bUsedWithLensFlare;
/** Adds an extra pow instruction to the shader using the current render target's gamma value */
var(Usage) const bool bUsedWithGammaCorrection;
var(Usage) const bool bUsedWithInstancedMeshParticles;
var(Usage) const bool bUsedWithFluidSurfaces;
var(Usage) const bool bUsedWithDecals;
var(Usage) const bool bUsedWithMaterialEffect;

// BM1
var(Usage) const bool bUsedWithMorphTargets;
var(Usage) const bool bUsedWithOpacityShadows;

var() bool Wireframe;

/** Indicates that the material will be used as a fallback on sm2 platforms */
var bool bIsFallbackMaterial;

/** TRUE if Material uses distortion */
var private bool						bUsesDistortion;

/** TRUE if Material is masked and uses custom opacity */
var private bool						bIsMasked;

/** TRUE if Material can support an SH light in the base pass */
var private bool						bSupportsSinglePassSHLight;

/** TRUE if Material is the preview material used in the material editor. */
var transient duplicatetransient private bool bIsPreviewMaterial;

/** The fallback material, which will be used on sm2 platforms */
var Material FallbackMaterial;

// Two resources for sm3 and sm2, indexed by EMaterialShaderPlatform
var const native duplicatetransient pointer MaterialResources[2]{FMaterialResource};

var const native duplicatetransient pointer DefaultMaterialInstances[2]{class FDefaultMaterialInstance};

var int		EditorX,
			EditorY;

/** Array of material expressions, excluding Comments and Compounds.  Used by the material editor. */
var array<MaterialExpression>			Expressions;

/** Array of comments associated with this material; viewed in the material editor. */
var editoronly array<MaterialExpressionComment>	EditorComments;

/** Array of material expression compounds associated with this material; viewed in the material editor. */
var editoronly array<MaterialExpressionCompound> EditorCompounds;

var native map{FName, TArray<UMaterialExpression*>} EditorParameters;

/**
 * Array of textures referenced, updated in PostLoad.  These are needed to keep the textures used by material resources
 * from getting destroyed by realtime GC.
 */
var private const array<texture> ReferencedTextures;

/** Content tags for quick browsing in the editor */
var editoronly array<Name> ContentTags;

cpptext
{
	// Constructor.
	UMaterial();

	/** @return TRUE if the material uses distortion */
	UBOOL HasDistortion() const;
	/** @return TRUE if the material uses the scene color texture */
	UBOOL UsesSceneColor() const;

	/**
	 * Allocates a material resource off the heap to be stored in MaterialResource.
	 */
	virtual FMaterialResource* AllocateResource();

	/**
	 * Gathers the textures used to render the material instance.
	 * @param OutTextures	Upon return contains the textures used to render the material instance.
	 * @param bOnlyAddOnce	Whether to add textures that are sampled multiple times uniquely or not
	 */
	virtual void GetTextures(TArray<UTexture*>& OutTextures,UBOOL bOnlyAddOnce = TRUE);

private:

	/** Sets the value associated with the given usage flag. */
	void SetUsageByFlag(EMaterialUsage Usage, UBOOL NewValue);

public:

	/** Gets the name of the given usage flag. */
	FString GetUsageName(EMaterialUsage Usage);

	/** Gets the value associated with the given usage flag. */
	UBOOL GetUsageByFlag(EMaterialUsage Usage);

	/**
	 * Checks if the material can be used with the given usage flag.
	 * If the flag isn't set in the editor, it will be set and the material will be recompiled with it.
	 * @param Usage - The usage flag to check
	 * @return UBOOL - TRUE if the material can be used for rendering with the given type.
	 */
	virtual UBOOL CheckMaterialUsage(EMaterialUsage Usage);

	/**
	 * Sets the given usage flag.
	 * @param bNeedsRecompile - TRUE if the material was recompiled for the usage change
	 * @param Usage - The usage flag to set
	 * @return UBOOL - TRUE if the material can be used for rendering with the given type.
	 */
	UBOOL SetMaterialUsage(UBOOL &bNeedsRecompile, EMaterialUsage Usage);

	/**
	 * @param	OutParameterNames		Storage array for the parameter names we are returning.
	 * @param	OutParameterIds			Storage array for the parameter id's we are returning.
	 *
	 * @return	Returns a array of vector parameter names used in this material.
	 */
	virtual void GetAllVectorParameterNames(TArray<FName> &OutParameterNames, TArray<FGuid> &OutParameterIds);

	/**
	 * @param	OutParameterNames		Storage array for the parameter names we are returning.
	 * @param	OutParameterIds			Storage array for the parameter id's we are returning.
	 *
	 * @return	Returns a array of scalar parameter names used in this material.
	 */
	virtual void GetAllScalarParameterNames(TArray<FName> &OutParameterNames, TArray<FGuid> &OutParameterIds);

	/**
	 * @param	OutParameterNames		Storage array for the parameter names we are returning.
	 * @param	OutParameterIds			Storage array for the parameter id's we are returning.
	 *
	 * @return	Returns a array of texture parameter names used in this material.
	 */
	virtual void GetAllTextureParameterNames(TArray<FName> &OutParameterNames, TArray<FGuid> &OutParameterIds);

	/**
	 * @param	OutParameterNames		Storage array for the parameter names we are returning.
	 * @param	OutParameterIds			Storage array for the parameter id's we are returning.
	 *
	 * @return	Returns a array of font parameter names used in this material.
	 */
	virtual void GetAllFontParameterNames(TArray<FName> &OutParameterNames, TArray<FGuid> &OutParameterIds);

	/**
	 * @param	OutParameterNames		Storage array for the parameter names we are returning.
	 * @param	OutParameterIds			Storage array for the parameter id's we are returning.
	 *
	 * @return	Returns a array of static switch parameter names used in this material.
	 */
	virtual void GetAllStaticSwitchParameterNames(TArray<FName> &OutParameterNames, TArray<FGuid> &OutParameterIds);

	/**
	 * @param	OutParameterNames		Storage array for the parameter names we are returning.
	 * @param	OutParameterIds			Storage array for the parameter id's we are returning.
	 *
	 * @return	Returns a array of static component mask parameter names used in this material.
	 */
	virtual void GetAllStaticComponentMaskParameterNames(TArray<FName> &OutParameterNames, TArray<FGuid> &OutParameterIds);

	/**
	 * Attempts to find a expression by its GUID.
	 *
	 * @param InGUID GUID to search for.
	 *
	 * @return Returns a expression object pointer if one is found, otherwise NULL if nothing is found.
	 */
	template<typename ExpressionType>
	ExpressionType* FindExpressionByGUID(const FGuid &InGUID)
	{
		ExpressionType* Result = NULL;

		for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
		{
			ExpressionType* ExpressionPtr =
				Cast<ExpressionType>(Expressions(ExpressionIndex));

			if(ExpressionPtr && ExpressionPtr->ExpressionGUID.IsValid() && ExpressionPtr->ExpressionGUID==InGUID)
			{
				Result = ExpressionPtr;
				break;
			}
		}

		return Result;
	}

	/**
	 * Builds a string of parameters in the fallback material that do not exist in the base material.
	 * These parameters won't be set by material instances, which get their parameter list from the base material.
	 *
	 * @param ParameterMismatches - string of unmatches material names to populate
	 */
	virtual void GetFallbackParameterInconsistencies(FString &ParameterMismatches);

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
    virtual UBOOL GetVectorParameterValue(FName ParameterName,FLinearColor& OutValue);
    virtual UBOOL GetScalarParameterValue(FName ParameterName,FLOAT& OutValue);
    virtual UBOOL GetTextureParameterValue(FName ParameterName,class UTexture*& OutValue);
	virtual UBOOL GetFontParameterValue(FName ParameterName,class UFont*& OutFontValue,INT& OutFontPage);

	/**
	 * Gets the value of the given static switch parameter
	 *
	 * @param	ParameterName	The name of the static switch parameter
	 * @param	OutValue		Will contain the value of the parameter if successful
	 * @return					True if successful
	 */
	virtual UBOOL GetStaticSwitchParameterValue(FName ParameterName,UBOOL &OutValue,FGuid &OutExpressionGuid);

	/**
	 * Gets the value of the given static component mask parameter
	 *
	 * @param	ParameterName	The name of the parameter
	 * @param	R, G, B, A		Will contain the values of the parameter if successful
	 * @return					True if successful
	 */
	virtual UBOOL GetStaticComponentMaskParameterValue(FName ParameterName, UBOOL &R, UBOOL &G, UBOOL &B, UBOOL &A, FGuid &OutExpressionGuid);

	virtual FMaterialRenderProxy* GetRenderProxy(UBOOL Selected) const;
	virtual UPhysicalMaterial* GetPhysicalMaterial() const;

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
	UBOOL CompileStaticPermutation(
		FStaticParameterSet* StaticParameters,
		FMaterialResource* StaticPermutation,
		EShaderPlatform Platform,
		EMaterialShaderPlatform MaterialPlatform,
		UBOOL bFlushExistingShaderMaps,
		UBOOL bDebugDump);

	/**
	 * Compiles material resources for the current platform if the shader map for that resource didn't already exist.
	 *
	 * @param ShaderPlatform - platform to compile for
	 * @param bFlushExistingShaderMaps - forces a compile, removes existing shader maps from shader cache.
	 * @param bForceAllPlatforms - compile for all platforms, not just the current.
	 */
	void CacheResourceShaders(EShaderPlatform Platform, UBOOL bFlushExistingShaderMaps=FALSE, UBOOL bForceAllPlatforms=FALSE);

	/**
	 * Flushes existing resource shader maps and resets the material resource's Ids.
	 */
	virtual void FlushResourceShaderMaps();

	/**
	 * Gets the material resource based on the input platform
	 * @return - the appropriate FMaterialResource if one exists, otherwise NULL
	 */
	virtual FMaterialResource * GetMaterialResource(EMaterialShaderPlatform Platform = GCurrentMaterialPlatform);

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
	/**
	 * Called before serialization on save to propagate referenced textures. This is not done
	 * during content cooking as the material expressions used to retrieve this information will
	 * already have been dissociated via RemoveExpressions
	 */
	void PreSave();

	virtual void AddReferencedObjects(TArray<UObject*>& ObjectArray);
	virtual void Serialize(FArchive& Ar);
	virtual void PostDuplicate();
	virtual void PostLoad();
	virtual void PreEditChange(UProperty* PropertyAboutToChange);
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void BeginDestroy();
	virtual UBOOL IsReadyForFinishDestroy();
	virtual void FinishDestroy();

	/**
	 * @return		Sum of the size of textures referenced by this material.
	 */
	virtual INT GetResourceSize();

	/**
	 * Used by various commandlets to purge Editor only data from the object.
	 *
	 * @param TargetPlatform Platform the object will be saved for (ie PC vs console cooking, etc)
	 */
	virtual void StripData(UE3::EPlatformType TargetPlatform);

	/**
	* Null any material expression references for this material
	*/
	void RemoveExpressions();

	UBOOL IsFallbackMaterial() { return bIsFallbackMaterial; }

	/**
	 * Goes through every material, flushes the specified types and re-initializes the material's shader maps.
	 */
	static void UpdateMaterialShaders(TArray<FShaderType*>& ShaderTypesToFlush, TArray<FVertexFactoryType*>& VFTypesToFlush);

	/**
	 * Adds an expression node that represents a parameter to the list of material parameters.
	 *
	 * @param	Expression	Pointer to the node that is going to be inserted if it's a parameter type.
	 */
	virtual UBOOL AddExpressionParameter(UMaterialExpression* Expression);

	/**
	 * Removes an expression node that represents a parameter from the list of material parameters.
	 *
	 * @param	Expression	Pointer to the node that is going to be removed if it's a parameter type.
	 */
	virtual UBOOL RemoveExpressionParameter(UMaterialExpression* Expression);

	/**
	 * A parameter with duplicates has to update its peers so that they all have the same value. If this step isn't performed then
	 * the expression nodes will not accurately display the final compiled material.
	 *
	 * @param	Parameter	Pointer to the expression node whose state needs to be propagated.
	 */
	virtual void PropagateExpressionParameterChanges(UMaterialExpression* Parameter);

	/**
	 * This function removes the expression from the editor parameters list (if it exists) and then re-adds it.
	 *
	 * @param	Expression	The expression node that represents a parameter that needs updating.
	 */
	virtual void UpdateExpressionParameterName(UMaterialExpression* Expression);

	/**
	 * Iterates through all of the expression nodes in the material and finds any parameters to put in EditorParameters.
	 */
	virtual void BuildEditorParameterList();

	/**
	 * Returns TRUE if the provided expression parameter has duplicates.
	 *
	 * @param	Expression	The expression parameter to check for duplicates.
	 */
	virtual UBOOL HasDuplicateParameters(UMaterialExpression* Expression);

	/**
	 * Returns TRUE if the provided expression dynamic parameter has duplicates.
	 *
	 * @param	Expression	The expression dynamic parameter to check for duplicates.
	 */
	virtual UBOOL HasDuplicateDynamicParameters(UMaterialExpression* Expression);

	/**
	 * Iterates through all of the expression nodes and fixes up changed names on
	 * matching dynamic parameters when a name change occurs.
	 *
	 * @param	Expression	The expression dynamic parameter.
	 */
	virtual void UpdateExpressionDynamicParameterNames(UMaterialExpression* Expression);

	/**
	 * Gets the name of a parameter.
	 *
	 * @param	Expression	The expression to retrieve the name from.
	 * @param	OutName		The variable that will hold the parameter name.
	 * @return	TRUE if the expression is a parameter with a name.
	 */
	static UBOOL GetExpressionParameterName(UMaterialExpression* Expression, FName& OutName);

	/**
	 * Copies the values of an expression parameter to another expression parameter of the same class.
	 *
	 * @param	Source			The source parameter.
	 * @param	Destination		The destination parameter that will receive Source's values.
	 */
	static UBOOL CopyExpressionParameters(UMaterialExpression* Source, UMaterialExpression* Destination);

	/**
	 * Returns TRUE if the provided expression node is a parameter.
	 *
	 * @param	Expression	The expression node to inspect.
	 */
	static UBOOL IsParameter(UMaterialExpression* Expression);

	/**
	 * Returns TRUE if the provided expression node is a dynamic parameter.
	 *
	 * @param	Expression	The expression node to inspect.
	 */
	static UBOOL IsDynamicParameter(UMaterialExpression* Expression);

	/**
	 * Returns the number of parameter groups. NOTE: The number returned can be innaccurate if you have parameters of different types with the same name.
	 */
	inline INT GetNumEditorParameters() const
	{
		return EditorParameters.Num();
	}

	/**
	 * Empties the editor parameters for the material.
	 */
	inline void EmptyEditorParameters()
	{
		EditorParameters.Empty();
	}

private:
	/**
	 * Sets overrides in the material's static parameters
	 *
	 * @param	Permutation		The set of static parameters to override and their values
	 */
	void SetStaticParameterOverrides(const FStaticParameterSet* Permutation);

	/**
	 * Clears static parameter overrides so that static parameter expression defaults will be used
	 *	for subsequent compiles.
	 */
	void ClearStaticParameterOverrides();

public:
	/** Helper functions for text output of properties... */
	static const TCHAR* GetMaterialLightingModelString(EMaterialLightingModel InMaterialLightingModel);
	static EMaterialLightingModel GetMaterialLightingModelFromString(const TCHAR* InMaterialLightingModelStr);
	static const TCHAR* GetBlendModeString(EBlendMode InBlendMode);
	static EBlendMode GetBlendModeFromString(const TCHAR* InBlendModeStr);
};

/** returns the Referneced Textures so one may set flats on them  (e.g. bForceMiplevelsToBeResident ) **/
function array<texture> GetTextures()
{
	return ReferencedTextures;
}

defaultproperties
{
	BlendMode=BLEND_Opaque
	DiffuseColor=(Constant=(R=128,G=128,B=128))
	DiffusePower=(Constant=1.0)
	SpecularColor=(Constant=(R=128,G=128,B=128))
	SpecularPower=(Constant=15.0)
	Distortion=(ConstantX=0,ConstantY=0)
	Opacity=(Constant=1)
	OpacityMask=(Constant=1)
	OpacityMaskClipValue=0.3333
	TwoSidedLightingColor=(Constant=(R=255,G=255,B=255))
	bAllowFog=TRUE
	bIsFallbackMaterial=FALSE
	bUsedWithStaticLighting=FALSE
	bSupportsSinglePassSHLight=TRUE
}
