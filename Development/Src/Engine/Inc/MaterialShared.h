/*=============================================================================
	MaterialShared.h: Shared material definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

extern EShaderPlatform GRHIShaderPlatform;

#define ME_CAPTION_HEIGHT		18
#define ME_STD_VPADDING			16
#define ME_STD_HPADDING			32
#define ME_STD_BORDER			8
#define ME_STD_THUMBNAIL_SZ		96
#define ME_PREV_THUMBNAIL_SZ	256
#define ME_STD_LABEL_PAD		16
#define ME_STD_TAB_HEIGHT		21

/**
 * The maximum number of texture samplers in the Material Editor by an artist.
 * If an artist uses more samples, the Material Editor will display a warning.
 *
 * The value of this constant assumes three samplers are currently used for lightmaps
 * in the base pass shader; any changes to texture sampler usage must be reflected here!
 */
#define MAX_ME_PIXELSHADER_SAMPLERS 13

/**
 * Number of samplers a material can use while supporting an SH light in the base pass.
 * This is used to avoid breaking materials which use the maximum number of samplers.
 */
#define MAX_SHLIGHT_BASEPASS_SAMPLERS 14

/**
 * The minimum package version which stores valid material compilation output.  Increasing this version will cause all materials saved in
 * older versions to generate their code on load.  Additionally, material shaders cached before the version will be discarded.
 * Warning: Because this version will invalidate old materials, bumping it requires a content resave! (otherwise shaders will be rebuilt on every run)
 */
#define VER_MIN_COMPILEDMATERIAL VER_DIFFUSEPOWER_DEFAULT
/** same as VER_MIN_COMPILEDMATERIAL but for UDecalMaterial */
#define VER_MIN_COMPILEDMATERIAL_DECAL VER_DECAL_MATERIAL_IDENDITY_NORMAL_XFORM

/** Same as VER_MIN_COMPILEDMATERIAL, but compared against the licensee package version. */
#define LICENSEE_VER_MIN_COMPILEDMATERIAL	0
/** same as LICENSEE_VER_MIN_COMPILEDMATERIAL but for UDecalMaterial */
#define LICENSEE_VER_MIN_COMPILEDMATERIAL_DECAL	0

/**
 * The types which can be used by materials.
 */
enum EMaterialValueType
{
	MCT_Float1		= 1,
	MCT_Float2		= 2,
	MCT_Float3		= 4,
	MCT_Float4		= 8,
	MCT_Float		= 8|4|2|1,
	MCT_Texture2D	= 16,
	MCT_TextureCube	= 32,
	MCT_Texture		= 16|32,
	MCT_Unknown		= 64
};

/**
 * The context of a material being rendered.
 */
struct FMaterialRenderContext
{
	/** material instance used for the material shader */
	const FMaterialRenderProxy* MaterialRenderProxy;	
	/** current scene time */
	FLOAT CurrentTime;
	/** The current real-time */
	FLOAT CurrentRealTime;
	/** view matrix used for transform expression */
	const FSceneView* View;

	/** 
	* Constructor
	*/
	FMaterialRenderContext(const FMaterialRenderProxy* InMaterialRenderProxy,FLOAT InCurrentTime,FLOAT InCurrentRealTime,const FSceneView* InView)
		:	MaterialRenderProxy(InMaterialRenderProxy)
		,	CurrentTime(InCurrentTime)
		,	CurrentRealTime(InCurrentRealTime)
		,	View(InView)
	{
	}
};

/**
 * Represents a subclass of FMaterialUniformExpression.
 */
class FMaterialUniformExpressionType
{
public:

	typedef class FMaterialUniformExpression* (*SerializationConstructorType)();

	/**
	 * @return The global uniform expression type list.  The list is used to temporarily store the types until
	 *			the name subsystem has been initialized.
	 */
	static TLinkedList<FMaterialUniformExpressionType*>*& GetTypeList();

	/**
	 * Should not be called until the name subsystem has been initialized.
	 * @return The global uniform expression type map.
	 */
	static TMap<FName,FMaterialUniformExpressionType*>& GetTypeMap();

	/**
	 * Minimal initialization constructor.
	 */
	FMaterialUniformExpressionType(const TCHAR* InName,SerializationConstructorType InSerializationConstructor);

	/**
	 * Serializer for references to uniform expressions.
	 */
	friend FArchive& operator<<(FArchive& Ar,class FMaterialUniformExpression*& Ref);

private:

	const TCHAR* Name;
	SerializationConstructorType SerializationConstructor;
};

#define DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(Name) \
	public: \
	static FMaterialUniformExpressionType StaticType; \
	static FMaterialUniformExpression* SerializationConstructor() { return new Name(); } \
	virtual FMaterialUniformExpressionType* GetType() const { return &StaticType; }

#define IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(Name) \
	FMaterialUniformExpressionType Name::StaticType(TEXT(#Name),&Name::SerializationConstructor);

/**
 * Represents an expression which only varies with uniform inputs.
 */
class FMaterialUniformExpression : public FRefCountedObject
{
public:

	virtual ~FMaterialUniformExpression() {}

	virtual FMaterialUniformExpressionType* GetType() const = 0;
	virtual void Serialize(FArchive& Ar) = 0;

	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const {}
	virtual void GetTextureValue(const FMaterialRenderContext& Context,const FTexture** OutValue) const {}
	virtual void GetGameThreadTextureValue(class UMaterialInterface* InMaterialInterface,UTexture** OutValue) const {}
	virtual UBOOL IsConstant() const { return FALSE; }
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const { return FALSE; }
};

//
//	FMaterialCompiler
//

struct FMaterialCompiler
{
	virtual INT Error(const TCHAR* Text) = 0;
	INT Errorf(const TCHAR* Format,...);

	virtual INT CallExpression(UMaterialExpression* MaterialExpression,FMaterialCompiler* InCompiler) = 0;

	virtual EMaterialValueType GetType(INT Code) = 0;
	virtual INT ForceCast(INT Code,EMaterialValueType DestType) = 0;

	virtual INT VectorParameter(FName ParameterName,const FLinearColor& DefaultValue) = 0;
	virtual INT ScalarParameter(FName ParameterName,FLOAT DefaultValue) = 0;
	virtual INT FlipBookOffset(UTexture* InFlipBook) = 0;

	virtual INT Constant(FLOAT X) = 0;
	virtual INT Constant2(FLOAT X,FLOAT Y) = 0;
	virtual INT Constant3(FLOAT X,FLOAT Y,FLOAT Z) = 0;
	virtual INT Constant4(FLOAT X,FLOAT Y,FLOAT Z,FLOAT W) = 0;

	virtual INT GameTime() = 0;
	virtual INT RealTime() = 0;
	virtual INT PeriodicHint(INT PeriodicCode) { return PeriodicCode; }

	virtual INT Sine(INT X) = 0;
	virtual INT Cosine(INT X) = 0;

	virtual INT Floor(INT X) = 0;
	virtual INT Ceil(INT X) = 0;
	virtual INT Frac(INT X) = 0;
	virtual INT Abs(INT X) = 0;

	virtual INT ReflectionVector() = 0;
	virtual INT CameraVector() = 0;
	virtual INT CameraWorldPosition() = 0;
	virtual INT LightVector() = 0;

	virtual INT ScreenPosition( UBOOL bScreenAlign ) = 0;
	virtual INT WorldPosition() = 0;

	virtual INT If(INT A,INT B,INT AGreaterThanB,INT AEqualsB,INT ALessThanB) = 0;

	virtual INT TextureCoordinate(UINT CoordinateIndex) = 0;
	virtual INT TextureSample(INT Texture,INT Coordinate) = 0;

	virtual INT Texture(UTexture* Texture) = 0;
	virtual INT TextureParameter(FName ParameterName,UTexture* DefaultTexture) = 0;

	virtual	INT SceneTextureSample( BYTE TexType, INT CoordinateIdx, UBOOL ScreenAlign )=0;
	virtual	INT SceneTextureDepth( UBOOL bNormalize, INT CoordinateIdx)=0;
	virtual	INT PixelDepth(UBOOL bNormalize)=0;
	virtual	INT DestColor()=0;
	virtual	INT DestDepth(UBOOL bNormalize)=0;
	virtual INT DepthBiasedAlpha( INT SrcAlphaIdx, INT BiasIdx, INT BiasScaleIdx )=0;
	virtual INT DepthBiasedBlend( INT SrcColorIdx, INT BiasIdx, INT BiasScaleIdx )=0;

	virtual INT VertexColor() = 0;

	virtual INT Add(INT A,INT B) = 0;
	virtual INT Sub(INT A,INT B) = 0;
	virtual INT Mul(INT A,INT B) = 0;
	virtual INT Div(INT A,INT B) = 0;
	virtual INT Dot(INT A,INT B) = 0;
	virtual INT Cross(INT A,INT B) = 0;

	virtual INT Power(INT Base,INT Exponent) = 0;
	virtual INT SquareRoot(INT X) = 0;

	virtual INT Lerp(INT X,INT Y,INT A) = 0;
	virtual INT Min(INT A,INT B) = 0;
	virtual INT Max(INT A,INT B) = 0;
	virtual INT Clamp(INT X,INT A,INT B) = 0;

	virtual INT ComponentMask(INT Vector,UBOOL R,UBOOL G,UBOOL B,UBOOL A) = 0;
	virtual INT AppendVector(INT A,INT B) = 0;
	virtual INT TransformVector(BYTE CoordType,INT A) = 0;
	virtual INT TransformPosition(BYTE CoordType,INT A) = 0;

	virtual INT LensFlareIntesity() = 0;
	virtual INT LensFlareOcclusion() = 0;
	virtual INT LensFlareRadialDistance() = 0;
	virtual INT LensFlareRayDistance() = 0;
	virtual INT LensFlareSourceDistance() = 0;

	virtual INT DynamicParameter() = 0;
};

//
//	FProxyMaterialCompiler - A proxy for the compiler interface which by default passes all function calls unmodified.
//

struct FProxyMaterialCompiler: FMaterialCompiler
{
	FMaterialCompiler*	Compiler;

	// Constructor.

	FProxyMaterialCompiler(FMaterialCompiler* InCompiler):
		Compiler(InCompiler)
	{}

	// Simple pass through all other material operations unmodified.

	virtual INT Error(const TCHAR* Text) { return Compiler->Error(Text); }

	virtual INT CallExpression(UMaterialExpression* MaterialExpression,FMaterialCompiler* InCompiler) { return Compiler->CallExpression(MaterialExpression,InCompiler); }

	virtual EMaterialValueType GetType(INT Code) { return Compiler->GetType(Code); }
	virtual INT ForceCast(INT Code,EMaterialValueType DestType) { return Compiler->ForceCast(Code,DestType); }

	virtual INT VectorParameter(FName ParameterName,const FLinearColor& DefaultValue) { return Compiler->VectorParameter(ParameterName,DefaultValue); }
	virtual INT ScalarParameter(FName ParameterName,FLOAT DefaultValue) { return Compiler->ScalarParameter(ParameterName,DefaultValue); }

	virtual INT Constant(FLOAT X) { return Compiler->Constant(X); }
	virtual INT Constant2(FLOAT X,FLOAT Y) { return Compiler->Constant2(X,Y); }
	virtual INT Constant3(FLOAT X,FLOAT Y,FLOAT Z) { return Compiler->Constant3(X,Y,Z); }
	virtual INT Constant4(FLOAT X,FLOAT Y,FLOAT Z,FLOAT W) { return Compiler->Constant4(X,Y,Z,W); }

	virtual INT GameTime() { return Compiler->GameTime(); }
	virtual INT RealTime() { return Compiler->RealTime(); }

	virtual INT PeriodicHint(INT PeriodicCode) { return Compiler->PeriodicHint(PeriodicCode); }

	virtual INT Sine(INT X) { return Compiler->Sine(X); }
	virtual INT Cosine(INT X) { return Compiler->Cosine(X); }

	virtual INT Floor(INT X) { return Compiler->Floor(X); }
	virtual INT Ceil(INT X) { return Compiler->Ceil(X); }
	virtual INT Frac(INT X) { return Compiler->Frac(X); }
	virtual INT Abs(INT X) { return Compiler->Abs(X); }

	virtual INT ReflectionVector() { return Compiler->ReflectionVector(); }
	virtual INT CameraVector() { return Compiler->CameraVector(); }
	virtual INT CameraWorldPosition() { return Compiler->CameraWorldPosition(); }
	virtual INT LightVector() { return Compiler->LightVector(); }

	virtual INT ScreenPosition( UBOOL bScreenAlign ) { return Compiler->ScreenPosition( bScreenAlign ); }
	virtual INT WorldPosition() { return Compiler->WorldPosition(); }

	virtual INT If(INT A,INT B,INT AGreaterThanB,INT AEqualsB,INT ALessThanB) { return Compiler->If(A,B,AGreaterThanB,AEqualsB,ALessThanB); }

	virtual INT TextureSample(INT InTexture,INT Coordinate) { return Compiler->TextureSample(InTexture,Coordinate); }
	virtual INT TextureCoordinate(UINT CoordinateIndex) { return Compiler->TextureCoordinate(CoordinateIndex); }

	virtual INT Texture(UTexture* InTexture) { return Compiler->Texture(InTexture); }
	virtual INT TextureParameter(FName ParameterName,UTexture* DefaultValue) { return Compiler->TextureParameter(ParameterName,DefaultValue); }

	virtual	INT SceneTextureSample(BYTE TexType,INT CoordinateIdx,UBOOL ScreenAlign) { return Compiler->SceneTextureSample(TexType,CoordinateIdx,ScreenAlign);	}
	virtual	INT SceneTextureDepth( UBOOL bNormalize, INT CoordinateIdx) { return Compiler->SceneTextureDepth(bNormalize,CoordinateIdx);	}
	virtual	INT PixelDepth(UBOOL bNormalize) { return Compiler->PixelDepth(bNormalize);	}
	virtual	INT DestColor() { return Compiler->DestColor(); }
	virtual	INT DestDepth(UBOOL bNormalize) { return Compiler->DestDepth(bNormalize); }
	virtual INT DepthBiasedAlpha( INT SrcAlphaIdx, INT BiasIdx, INT BiasScaleIdx ) { return Compiler->DepthBiasedAlpha(SrcAlphaIdx,BiasIdx,BiasScaleIdx); }
	virtual INT DepthBiasedBlend( INT SrcColorIdx, INT BiasIdx, INT BiasScaleIdx ) { return Compiler->DepthBiasedBlend(SrcColorIdx,BiasIdx,BiasScaleIdx); }

	virtual INT VertexColor() { return Compiler->VertexColor(); }

	virtual INT Add(INT A,INT B) { return Compiler->Add(A,B); }
	virtual INT Sub(INT A,INT B) { return Compiler->Sub(A,B); }
	virtual INT Mul(INT A,INT B) { return Compiler->Mul(A,B); }
	virtual INT Div(INT A,INT B) { return Compiler->Div(A,B); }
	virtual INT Dot(INT A,INT B) { return Compiler->Dot(A,B); }
	virtual INT Cross(INT A,INT B) { return Compiler->Cross(A,B); }

	virtual INT Power(INT Base,INT Exponent) { return Compiler->Power(Base,Exponent); }
	virtual INT SquareRoot(INT X) { return Compiler->SquareRoot(X); }

	virtual INT Lerp(INT X,INT Y,INT A) { return Compiler->Lerp(X,Y,A); }
	virtual INT Min(INT A,INT B) { return Compiler->Min(A,B); }
	virtual INT Max(INT A,INT B) { return Compiler->Max(A,B); }
	virtual INT Clamp(INT X,INT A,INT B) { return Compiler->Clamp(X,A,B); }

	virtual INT ComponentMask(INT Vector,UBOOL R,UBOOL G,UBOOL B,UBOOL A) { return Compiler->ComponentMask(Vector,R,G,B,A); }
	virtual INT AppendVector(INT A,INT B) { return Compiler->AppendVector(A,B); }
	virtual INT TransformVector(BYTE CoordType,INT A) { return Compiler->TransformVector(CoordType,A); }
	virtual INT TransformPosition(BYTE CoordType,INT A) { return Compiler->TransformPosition(CoordType,A); }

	virtual INT DynamicParameter() { return Compiler->DynamicParameter(); }
};

//
//	EMaterialProperty
//

enum EMaterialProperty
{
	MP_EmissiveColor = 0,
	MP_Opacity,
	MP_OpacityMask,
	MP_Distortion,
	MP_TwoSidedLightingMask,
	MP_DiffuseColor,
	MP_DiffusePower,
	MP_SpecularColor,
	MP_SpecularPower,
	MP_Normal,
	MP_CustomLighting,
	MP_MAX
};

/**
 * @return The type of value expected for the given material property.
 */
extern EMaterialValueType GetMaterialPropertyType(EMaterialProperty Property);

/** transform types usable by a material shader */
enum ECoordTransformUsage
{
	// no transforms used
	UsedCoord_None		=0,
	// local to world used
	UsedCoord_World		=1<<0,
	// local to view used
	UsedCoord_View		=1<<1,
	// local to local used
	UsedCoord_Local		=1<<2,
	// World Position used
	UsedCoord_WorldPos	=1<<3
};

/** Components that were dropped while compiling the FMaterial to get it to fit in shader model restraints */
enum EDroppedFallbackComponents
{
	DroppedFallback_None		=0,
	DroppedFallback_Specular	=1<<0,
	DroppedFallback_Normal		=1<<1,
	DroppedFallback_Diffuse		=1<<2,
	DroppedFallback_Emissive	=1<<3,
	DroppedFallback_Failed		=1<<31
};

/**
 * A material.
 */
class FMaterial
{
	friend class FMaterialPixelShaderParameters;
	friend class FHLSLMaterialTranslator;
public:

	/**
	 * Minimal initialization constructor.
	 */
	FMaterial():
		MaxTextureDependencyLength(0),
		ShaderMap(NULL),
		Id(0,0,0,0),
		UsingTransforms(UsedCoord_None),
		bUsesSceneColor(FALSE),
		bUsesSceneDepth(FALSE),
		bUsesDynamicParameter(FALSE),
		DroppedFallbackComponents(DroppedFallback_None),
		bValidCompilationOutput(FALSE)
	{}

	/**
	 * Destructor
	 */
	virtual ~FMaterial();

	/**
	* Compiles this material for Platform, storing the result in OutShaderMap
	*
	* @param StaticParameters - the set of static parameters to compile
	* @param Platform - the platform to compile for
	* @param OutShaderMap - the shader map to compile
	* @param bForceCompile - force discard previous results 
	* @param bCompileFallback - indicates that if Platform is a sm2 platform and the normal compile failed
	*							drop components until the material compiles.
	* @param bDebugDump - Dump out the preprocessed and disassembled shader for debugging.
	* @return - TRUE if compile succeeded or was not necessary (shader map for StaticParameters was found and was complete)
	*/
	virtual UBOOL Compile(
		class FStaticParameterSet* StaticParameters, 
		EShaderPlatform Platform, 
		TRefCountPtr<class FMaterialShaderMap>& OutShaderMap, 
		UBOOL bForceCompile=FALSE, 
		UBOOL bCompileFallback=FALSE,
		UBOOL bDebugDump=FALSE);

	/**
	* Compiles OutShaderMap using the shader code from MaterialShaderCode on Platform
	*
	* @param StaticParameters - the set of static parameters to compile
	* @param Platform - the platform to compile for
	* @param OutShaderMap - the shader map to compile
	* @param MaterialShaderCode - a filled out instance of MaterialTemplate.usf to compile
	* @param bForceCompile - force discard previous results 
	* @param bSilent - indicates that no error message should be outputted on shader compile failure
	* @param bDebugDump - Dump out the preprocessed and disassembled shader for debugging.
	* @return - TRUE if compile succeeded or was not necessary (shader map for StaticParameters was found and was complete)
	*/
	virtual UBOOL FallbackCompile( 
		class FStaticParameterSet* StaticParameters, 
		EShaderPlatform Platform, 
		TRefCountPtr<FMaterialShaderMap>& OutShaderMap, 
		FString MaterialShaderCode,
		UBOOL bForceCompile,
		UBOOL bSilent = FALSE,
		UBOOL bDebugDump = FALSE);

	/**
	* Caches the material shaders for this material with no static parameters on the given platform.
	*/
	UBOOL CacheShaders(EShaderPlatform Platform=GRHIShaderPlatform, UBOOL bCompileFallback=FALSE);

	/**
	* Caches the material shaders for the given static parameter set and platform
	*/
	UBOOL CacheShaders(class FStaticParameterSet* StaticParameters, EShaderPlatform Platform, UBOOL bCompileFallback, UBOOL bFlushExistingShaderMap, UBOOL bDebugDump = FALSE);

	/**
	 * Should the shader for this material with the given platform, shader type and vertex 
	 * factory type combination be compiled
	 *
	 * @param Platform		The platform currently being compiled for
	 * @param ShaderType	Which shader is being compiled
	 * @param VertexFactory	Which vertex factory is being compiled (can be NULL)
	 *
	 * @return TRUE if the shader should be compiled
	 */
	virtual UBOOL ShouldCache(EShaderPlatform Platform, const FShaderType* ShaderType, const FVertexFactoryType* VertexFactoryType) const;

	/**
	 * Called by the material compilation code with a map of the compilation errors.
	 * Note that it is called even if there were no errors, but it passes an empty error map in that case.
	 * @param Errors - A set of expression error pairs.
	 */
	virtual void HandleMaterialErrors(const TMultiMap<UMaterialExpression*,FString>& Errors) {}

	/** Adds objects referenced by the material resource to an array. */
	void AddReferencedObjects(TArray<UObject*>& ObjectArray);

	/** Serializes the material. */
	void Serialize(FArchive& Ar);

	/** Initializes the material's shader map. */
	UBOOL InitShaderMap(EShaderPlatform Platform=GRHIShaderPlatform, UBOOL bCompileFallback=FALSE);

	/** Initializes the material's shader map. */
	UBOOL InitShaderMap(FStaticParameterSet * StaticParameters, EShaderPlatform Platform, UBOOL bCompileFallback=FALSE);

	/** Flushes this material's shader map from the shader cache if it exists. */
	void FlushShaderMap();

	/**
	 * Null any material expression references for this material
	 */
	void RemoveExpressions();


	// Material properties.
	virtual INT CompileProperty(EMaterialShaderPlatform MatPlatform,EMaterialProperty Property,FMaterialCompiler* Compiler) const = 0;
	virtual UBOOL IsTwoSided() const = 0;
	virtual UBOOL NeedsDepthTestDisabled() const = 0;
	virtual UBOOL AllowsFog() const { return TRUE; }
	virtual UBOOL UsesOneLayerDistortion() const { return FALSE; }
	virtual UBOOL IsUsedWithFogVolumes() const = 0;
	virtual UBOOL IsLightFunction() const = 0;
	virtual UBOOL IsWireframe() const = 0;
	virtual UBOOL IsDistorted() const = 0;
	virtual UBOOL IsSpecialEngineMaterial() const = 0;
	virtual UBOOL IsTerrainMaterial() const = 0;
	virtual UBOOL IsDecalMaterial() const = 0;
	virtual UBOOL IsUsedWithSkeletalMesh() const { return FALSE; }
	virtual UBOOL IsUsedWithFracturedMeshes() const { return FALSE; }
	virtual UBOOL IsUsedWithSpeedTree() const { return FALSE; }
	virtual UBOOL IsUsedWithParticleSystem() const { return FALSE; }
	virtual UBOOL IsUsedWithParticleSprites() const { return FALSE; }
	virtual UBOOL IsUsedWithBeamTrails() const { return FALSE; }
	virtual UBOOL IsUsedWithParticleSubUV() const { return FALSE; }
	virtual UBOOL IsUsedWithFoliage() const { return FALSE; }
	virtual UBOOL IsUsedWithStaticLighting() const { return FALSE; }
	virtual UBOOL IsUsedWithLensFlare() const { return FALSE; }
	virtual UBOOL IsUsedWithGammaCorrection() const { return FALSE; }
	virtual UBOOL IsUsedWithInstancedMeshParticles() const { return FALSE; }
	virtual UBOOL IsUsedWithFluidSurfaces() const { return FALSE; }
	virtual UBOOL IsUsedWithMaterialEffect() const { return FALSE; }
	virtual UBOOL IsUsedWithDecals() const { return FALSE; }
	/** @return TRUE if the material should only be used with simple static lighting */
	virtual UBOOL RequiresSimpleStaticLighting(EMaterialShaderPlatform MatPlatform) const { return FALSE; }
	virtual UBOOL IsMasked() const = 0;
	virtual UBOOL SupportsSinglePassSHLight() const { return FALSE; }
	virtual enum EBlendMode GetBlendMode() const = 0;
	virtual enum EMaterialLightingModel GetLightingModel() const = 0;
	virtual FLOAT GetOpacityMaskClipValue() const = 0;
	virtual FString GetFriendlyName() const = 0;

	/**
	 * Should shaders compiled for this material be saved to disk?
	 */
	virtual UBOOL IsPersistent() const = 0;

	// Accessors.
	const TArray<FString>& GetCompileErrors() const { return CompileErrors; }
	void SetCompileErrors(const TArray<FString>& InCompileErrors) { CompileErrors = InCompileErrors; }
	void SetUsesSceneColor(UBOOL bInUsesSceneColor) { bUsesSceneColor = bInUsesSceneColor; }
	const TMap<UMaterialExpression*,INT>& GetTextureDependencyLengthMap() const { return TextureDependencyLengthMap; }
	INT GetMaxTextureDependencyLength() const { return MaxTextureDependencyLength; }
	const TArray<TRefCountPtr<FMaterialUniformExpression> >& GetUniform2DTextureExpressions() const { return Uniform2DTextureExpressions; }
	const TArray<TRefCountPtr<FMaterialUniformExpression> >& GetUniformCubeTextureExpressions() const { return UniformCubeTextureExpressions; }
	class FMaterialShaderMap* GetShaderMap() const { return ShaderMap; }
	const FGuid& GetId() const { return Id; }
	void SetId(const FGuid& NewId)
	{
		Id = NewId;
	}
	DWORD GetTransformsUsed() const { return UsingTransforms; }
	UINT GetUserTexCoordsUsed() const { return NumUserTexCoords; }
	/** Boolean indicators of using SceneColorTexture or SceneDepthTexture	*/
	UBOOL GetUsesSceneColor() const { return bUsesSceneColor; }
	UBOOL GetUsesSceneDepth() const { return bUsesSceneDepth; }
	/** Boolean indicating using DynamicParameter */
	UBOOL GetUsesDynamicParameter() const { return bUsesDynamicParameter; }
	DWORD GetDroppedFallbackComponents() const { return DroppedFallbackComponents; }
	void ClearDroppedFallbackComponents() { DroppedFallbackComponents = DroppedFallback_None; }

	/** Information about one texture lookup. */
	struct FTextureLookup
	{
		void	Serialize(FArchive& Ar);
		INT		TexCoordIndex;
		INT		TextureIndex;			// Index into Uniform2DTextureExpressions

		/** Horizontal multiplier that can be different from 1.0f if the artist uses tiling */
		FLOAT	UScale;

		/** Vertical multiplier */
		FLOAT VScale;
	};
	typedef TArray<FTextureLookup> FTextureLookupInfo;

	/** Returns information about all texture lookups. */
	const FTextureLookupInfo &	GetTextureLookupInfo() const	{ return TextureLookups; }

	/**
	 * Finds the shader matching the template type and the passed in vertex factory, asserts if not found.
	 * Note - Only implemented for FMeshMaterialShaderTypes
	 */
	template<typename ShaderType>
	ShaderType* GetShader(FVertexFactoryType* VertexFactoryType) const
	{
		return (ShaderType*)GetShader(&ShaderType::StaticType, VertexFactoryType);
	}

	/** Returns a string that describes the material's usage for debugging purposes. */
	FString GetMaterialUsageDescription() const;

	/** Indicates whether warnings should be logged on the given platform. */
	static UBOOL ShouldWarnOnBrokenMaterial(EShaderPlatform ShaderPlatform);

protected:
	void SetUsesDynamicParameter(UBOOL bInUsesDynamicParameter) { bUsesDynamicParameter = bInUsesDynamicParameter; }

	/** Rebuilds the information about all texture lookups. */
	void	RebuildTextureLookupInfo( UMaterial *Material );

	/** Returns the index to the Expression in the Expressions array, or -1 if not found. */
	INT		FindExpression( const TArray<TRefCountPtr<FMaterialUniformExpression> >&Expressions, const FMaterialUniformExpression &Expression );

private:

	TArray<FString> CompileErrors;

	/** The texture dependency lengths for the materials' expressions. */
	TMap<UMaterialExpression*,INT> TextureDependencyLengthMap;

	/** The maximum texture dependency length for the material. */
	INT MaxTextureDependencyLength;

	TRefCountPtr<FMaterialShaderMap> ShaderMap;

	FGuid Id;

	TArray<TRefCountPtr<FMaterialUniformExpression> > UniformVectorExpressions;
	TArray<TRefCountPtr<FMaterialUniformExpression> > UniformScalarExpressions;
	TArray<TRefCountPtr<FMaterialUniformExpression> > Uniform2DTextureExpressions;
	TArray<TRefCountPtr<FMaterialUniformExpression> > UniformCubeTextureExpressions;

	/** Information about each texture lookup in the pixel shader. */
	FTextureLookupInfo	TextureLookups;

	UINT NumUserTexCoords;

	/** combination of ECoordTransformUsage flags used by this shader */
	DWORD UsingTransforms;

	/** Boolean indicators of using SceneColorTexture or SceneDepthTexture	*/
	UBOOL bUsesSceneColor;
	UBOOL bUsesSceneDepth;

	/** Boolean indicating using DynamicParameter */
	UBOOL bUsesDynamicParameter;

	DWORD DroppedFallbackComponents;

	/**
	 * False if the material's persistent compilation output was loaded from an archive older than VER_MIN_COMPILEDMATERIAL.
	 * (VER_MIN_COMPILEDMATERIAL is defined in MaterialShared.cpp)
	 */
	UBOOL bValidCompilationOutput;

	friend class UDecalMaterial;

	/**
	 * Finds the shader matching the template type and the passed in vertex factory, asserts if not found.
	 */
	FShader* GetShader(class FMeshMaterialShaderType* ShaderType, FVertexFactoryType* VertexFactoryType) const;
};

/**
 * A material render proxy used by the renderer.
 */
class FMaterialRenderProxy
{
public:

	// These functions should only be called by the rendering thread.
	virtual const class FMaterial* GetMaterial() const = 0;
	virtual UBOOL GetVectorValue(const FName& ParameterName, FLinearColor* OutValue, const FMaterialRenderContext& Context) const = 0;
	virtual UBOOL GetScalarValue(const FName& ParameterName, FLOAT* OutValue, const FMaterialRenderContext& Context) const = 0;
	virtual UBOOL GetTextureValue(const FName& ParameterName,const FTexture** OutValue, const FMaterialRenderContext& Context) const = 0;
	virtual ~FMaterialRenderProxy() {}
};

/**
 * An material render proxy which overrides the material's Color vector parameter.
 */
class FColoredMaterialRenderProxy : public FMaterialRenderProxy
{
public:

	const FMaterialRenderProxy* const Parent;
	const FLinearColor Color;

	/** Initialization constructor. */
	FColoredMaterialRenderProxy(const FMaterialRenderProxy* InParent,const FLinearColor& InColor):
		Parent(InParent),
		Color(InColor)
	{}

	// FMaterialRenderProxy interface.
	virtual const class FMaterial* GetMaterial() const;
	virtual UBOOL GetVectorValue(const FName& ParameterName, FLinearColor* OutValue, const FMaterialRenderContext& Context) const;
	virtual UBOOL GetScalarValue(const FName& ParameterName, FLOAT* OutValue, const FMaterialRenderContext& Context) const;
	virtual UBOOL GetTextureValue(const FName& ParameterName,const FTexture** OutValue, const FMaterialRenderContext& Context) const;
};

/**
 * A material render proxy of GEngine->SimpleElementMaterial which specifies the texture parameter.
 */
class FTexturedMaterialRenderProxy : public FMaterialRenderProxy
{
public:

	const FMaterialRenderProxy* const Parent;
	const FTexture* const Texture;

	/** Initialization constructor. */
	FTexturedMaterialRenderProxy(const FMaterialRenderProxy* InParent,const FTexture* InTexture):
		Parent(InParent),
		Texture(InTexture)
	{}

	// FMaterialRenderProxy interface.
	virtual const class FMaterial* GetMaterial() const;
	virtual UBOOL GetVectorValue(const FName& ParameterName, FLinearColor* OutValue, const FMaterialRenderContext& Context) const;
	virtual UBOOL GetScalarValue(const FName& ParameterName, FLOAT* OutValue, const FMaterialRenderContext& Context) const;
	virtual UBOOL GetTextureValue(const FName& ParameterName,const FTexture** OutValue, const FMaterialRenderContext& Context) const;
};

/**
* A material render proxy for font rendering
*/
class FFontMaterialRenderProxy : public FMaterialRenderProxy
{
public:

	/** parent material instance for fallbacks */
	const FMaterialRenderProxy* const Parent;
	/** font which supplies the texture pages */
	const class UFont* Font;
	/** index to the font texture page to use by the intance */
	const INT FontPage;
	/** font parameter name for finding the matching parameter */
	const FName& FontParamName;

	/** Initialization constructor. */
	FFontMaterialRenderProxy(const FMaterialRenderProxy* InParent,const class UFont* InFont,const INT InFontPage, const FName& InFontParamName)
	:	Parent(InParent)
	,	Font(InFont)
	,	FontPage(InFontPage)
	,	FontParamName(InFontParamName)
	{
		check(Parent);
		check(Font);
	}

	// FMaterialRenderProxy interface.
	virtual const class FMaterial* GetMaterial() const;
	virtual UBOOL GetVectorValue(const FName& ParameterName, FLinearColor* OutValue, const FMaterialRenderContext& Context) const;
	virtual UBOOL GetScalarValue(const FName& ParameterName, FLOAT* OutValue, const FMaterialRenderContext& Context) const;
	virtual UBOOL GetTextureValue(const FName& ParameterName,const FTexture** OutValue, const FMaterialRenderContext& Context) const;
};

//
//	FExpressionInput
//

//@warning: FExpressionInput is mirrored in MaterialExpression.uc and manually "subclassed" in Material.uc (FMaterialInput)
struct FExpressionInput
{
	class UMaterialExpression*	Expression;
	UBOOL						Mask,
								MaskR,
								MaskG,
								MaskB,
								MaskA;
	DWORD						GCC64Padding; // @todo 64: if the C++ didn't mismirror this structure, we might not need this

	INT Compile(FMaterialCompiler* Compiler);
};

//
//	FMaterialInput
//

template<class InputType> struct FMaterialInput: FExpressionInput
{
	BITFIELD	UseConstant:1;
	InputType	Constant;
};

struct FColorMaterialInput: FMaterialInput<FColor>
{
	INT Compile(FMaterialCompiler* Compiler,const FColor& Default);
};
struct FScalarMaterialInput: FMaterialInput<FLOAT>
{
	INT Compile(FMaterialCompiler* Compiler,FLOAT Default);
};

struct FVectorMaterialInput: FMaterialInput<FVector>
{
	INT Compile(FMaterialCompiler* Compiler,const FVector& Default);
};

struct FVector2MaterialInput: FMaterialInput<FVector2D>
{
	INT Compile(FMaterialCompiler* Compiler,const FVector2D& Default);
};

//
//	FExpressionOutput
//

struct FExpressionOutput
{
	FString	Name;
	UBOOL	Mask,
			MaskR,
			MaskG,
			MaskB,
			MaskA;

	FExpressionOutput(UBOOL InMask,UBOOL InMaskR = 0,UBOOL InMaskG = 0,UBOOL InMaskB = 0,UBOOL InMaskA = 0):
		Mask(InMask),
		MaskR(InMaskR),
		MaskG(InMaskG),
		MaskB(InMaskB),
		MaskA(InMaskA)
	{}
};

/**
 * @return True if BlendMode is translucent, False if it is opaque.
 */
extern UBOOL IsTranslucentBlendMode(enum EBlendMode BlendMode);

/**
 * A resource which represents UMaterial to the renderer.
 */
class FMaterialResource : public FMaterial
{
public:

	FRenderCommandFence ReleaseFence;

	FMaterialResource(UMaterial* InMaterial);
	virtual ~FMaterialResource() {}

	void SetMaterial(UMaterial* InMaterial)
	{
		Material = InMaterial;
	}

	/** Returns the number of samplers used in this material. */
	INT GetSamplerUsage() const;

	// FMaterial interface.
	virtual INT CompileProperty(EMaterialShaderPlatform MatPlatform,EMaterialProperty Property,FMaterialCompiler* Compiler) const;
	virtual UBOOL IsTwoSided() const;
	virtual UBOOL NeedsDepthTestDisabled() const;
	virtual UBOOL AllowsFog() const;
	virtual UBOOL UsesOneLayerDistortion() const;
	virtual UBOOL IsUsedWithFogVolumes() const;
	virtual UBOOL IsLightFunction() const;
	virtual UBOOL IsWireframe() const;
	virtual UBOOL IsSpecialEngineMaterial() const;
	virtual UBOOL IsTerrainMaterial() const;
	virtual UBOOL IsDecalMaterial() const;
	virtual UBOOL IsUsedWithSkeletalMesh() const;
	virtual UBOOL IsUsedWithFracturedMeshes() const;
	virtual UBOOL IsUsedWithSpeedTree() const;
	virtual UBOOL IsUsedWithParticleSystem() const;
	virtual UBOOL IsUsedWithParticleSprites() const;
	virtual UBOOL IsUsedWithBeamTrails() const;
	virtual UBOOL IsUsedWithParticleSubUV() const;
	virtual UBOOL IsUsedWithFoliage() const;
	virtual UBOOL IsUsedWithStaticLighting() const;
	virtual UBOOL IsUsedWithLensFlare() const;
	virtual UBOOL IsUsedWithGammaCorrection() const;
	virtual UBOOL IsUsedWithInstancedMeshParticles() const;
	virtual UBOOL IsUsedWithFluidSurfaces() const;
	virtual UBOOL IsUsedWithMaterialEffect() const;
	virtual UBOOL IsUsedWithDecals() const;
	virtual enum EBlendMode GetBlendMode() const;
	virtual enum EMaterialLightingModel GetLightingModel() const;
	virtual FLOAT GetOpacityMaskClipValue() const;
	virtual UBOOL IsDistorted() const;
	virtual UBOOL IsMasked() const;
	virtual UBOOL SupportsSinglePassSHLight() const;
	virtual FString GetFriendlyName() const;
	/**
	 * Should shaders compiled for this material be saved to disk?
	 */
	virtual UBOOL IsPersistent() const;

	/** Allows the resource to do things upon compile. */
	virtual UBOOL Compile( class FStaticParameterSet* StaticParameters, EShaderPlatform Platform, TRefCountPtr<FMaterialShaderMap>& OutShaderMap, UBOOL bForceCompile=FALSE, UBOOL bCompileFallback=FALSE, UBOOL bDebugDump=FALSE);

	/**
	 * Gets instruction counts that best represent the likely usage of this material based on lighting model and other factors.
	 * @param Descriptions - an array of descriptions to be populated
	 * @param InstructionCounts - an array of instruction counts matching the Descriptions.  
	 *		The dimensions of these arrays are guaranteed to be identical and all values are valid.
	 */
	void GetRepresentativeInstructionCounts(TArray<FString> &Descriptions, TArray<INT> &InstructionCounts) const;

protected:
	UMaterial* Material;
};


/**
* Holds the information for a static switch parameter
*/
class FStaticSwitchParameter
{
public:
	FName ParameterName;
	UBOOL Value;
	UBOOL bOverride;
	FGuid ExpressionGUID;

	FStaticSwitchParameter() :
	ParameterName(TEXT("None")),
		Value(FALSE),
		bOverride(FALSE),
		ExpressionGUID(0,0,0,0)
	{ }

	FStaticSwitchParameter(FName InName, UBOOL InValue, UBOOL InOverride, FGuid InGuid) :
	ParameterName(InName),
		Value(InValue),
		bOverride(InOverride),
		ExpressionGUID(InGuid)
	{ }

	friend FArchive& operator<<(FArchive& Ar,FStaticSwitchParameter& P)
	{
		Ar << P.ParameterName << P.Value << P.bOverride;
		Ar << P.ExpressionGUID;
		return Ar;
	}
};

/**
* Holds the information for a static component mask parameter
*/
class FStaticComponentMaskParameter
{
public:
	FName ParameterName;
	UBOOL R, G, B, A;
	UBOOL bOverride;
	FGuid ExpressionGUID;

	FStaticComponentMaskParameter() :
	ParameterName(TEXT("None")),
		R(FALSE),
		G(FALSE),
		B(FALSE),
		A(FALSE),
		bOverride(FALSE),
		ExpressionGUID(0,0,0,0)
	{ }

	FStaticComponentMaskParameter(FName InName, UBOOL InR, UBOOL InG, UBOOL InB, UBOOL InA, UBOOL InOverride, FGuid InGuid) :
	ParameterName(InName),
		R(InR),
		G(InG),
		B(InB),
		A(InA),
		bOverride(InOverride),
		ExpressionGUID(InGuid)
	{ }

	friend FArchive& operator<<(FArchive& Ar,FStaticComponentMaskParameter& P)
	{
		Ar << P.ParameterName << P.R << P.G << P.B << P.A << P.bOverride;
		Ar << P.ExpressionGUID;
		return Ar;
	}
};


/**
* Contains all the information needed to identify a FMaterialShaderMap.  
*/
class FStaticParameterSet
{
public:
	/** 
	* The Id of the base material's FMaterialResource.  This, along with the static parameters,
	* are used to identify identical shader maps so that material instances with static permutations can share shader maps.
	*/
	FGuid BaseMaterialId;

	/** An array of static switch parameters in this set */
	TArray<FStaticSwitchParameter> StaticSwitchParameters;

	/** An array of static component mask parameters in this set */
	TArray<FStaticComponentMaskParameter> StaticComponentMaskParameters;

	FStaticParameterSet() :
		BaseMaterialId(0, 0, 0, 0)
	{
	}

	FStaticParameterSet(FGuid InBaseId) :
		BaseMaterialId(InBaseId)
	{
	}

	/** 
	* Checks if this set contains any parameters
	* 
	* @return	TRUE if this set has no parameters
	*/
	UBOOL IsEmpty()
	{
		return StaticSwitchParameters.Num() == 0 && StaticComponentMaskParameters.Num() == 0;
	}

	void Serialize(FArchive& Ar)
	{
		Ar << BaseMaterialId << StaticSwitchParameters << StaticComponentMaskParameters;
	}

	/** 
	* Indicates whether this set is equal to another, copying override settings.
	* 
	* @param ReferenceSet	The set to compare against
	* @return				TRUE if the sets are not equal
	*/
	UBOOL ShouldMarkDirty(const FStaticParameterSet * ReferenceSet);

	friend DWORD GetTypeHash(const FStaticParameterSet &Ref)
	{
		return Ref.BaseMaterialId.A;
	}

	/** 
	* Tests this set against another for equality, disregarding override settings.
	* 
	* @param ReferenceSet	The set to compare against
	* @return				TRUE if the sets are equal
	*/
	UBOOL operator==(const FStaticParameterSet &ReferenceSet) const;
};

