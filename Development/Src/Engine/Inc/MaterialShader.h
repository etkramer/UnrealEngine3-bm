/*=============================================================================
	MaterialShader.h: Material shader definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/** The minimum package version to load material pixel shaders with. */
#define VER_MIN_MATERIAL_PIXELSHADER	VER_CONTENT_RESAVE_AUGUST_2007_QA_BUILD

/** Same as VER_MIN_MATERIAL_PIXELSHADER, but for the licensee package version. */
#define LICENSEE_VER_MIN_MATERIAL_PIXELSHADER	0

/** The minimum package version to load material vertex shaders with. */
#define VER_MIN_MATERIAL_VERTEXSHADER	VER_CONTENT_RESAVE_AUGUST_2007_QA_BUILD

/** Same as VER_MIN_MATERIAL_VERTEXSHADER, but for the licensee package version. */
#define LICENSEE_VER_MIN_MATERIAL_VERTEXSHADER	0

/** A macro to implement material shaders which checks the package version for VER_MIN_MATERIAL_*SHADER and LICENSEE_VER_MIN_MATERIAL_*SHADER. */
#define IMPLEMENT_MATERIAL_SHADER_TYPE(TemplatePrefix,ShaderClass,SourceFilename,FunctionName,Frequency,MinPackageVersion,MinLicenseePackageVersion) \
	IMPLEMENT_SHADER_TYPE( \
		TemplatePrefix, \
		ShaderClass, \
		SourceFilename, \
		FunctionName, \
		Frequency, \
		Max(MinPackageVersion,Frequency == SF_Pixel ? Max(VER_MIN_COMPILEDMATERIAL, VER_MIN_MATERIAL_PIXELSHADER) : Max(VER_MIN_COMPILEDMATERIAL, VER_MIN_MATERIAL_VERTEXSHADER)), \
		Max(MinLicenseePackageVersion,Frequency == SF_Pixel ? Max(LICENSEE_VER_MIN_COMPILEDMATERIAL, LICENSEE_VER_MIN_MATERIAL_PIXELSHADER) : Max(LICENSEE_VER_MIN_COMPILEDMATERIAL, LICENSEE_VER_MIN_MATERIAL_VERTEXSHADER)) \
		);

/** Converts an EMaterialLightingModel to a string description. */
extern FString GetLightingModelString(EMaterialLightingModel LightingModel);

/** Converts an EBlendMode to a string description. */
extern FString GetBlendModeString(EBlendMode BlendMode);

/** Called for every material shader to update the appropriate stats. */
extern void UpdateMaterialShaderCompilingStats(const FMaterial* Material);

/**
 * An encapsulation of the material parameters for a shader.
 */
class FMaterialPixelShaderParameters
{
public:

	void Bind(const FMaterial* Material,const FShaderParameterMap& ParameterMap);
	void Set(FShader* PixelShader,const FMaterialRenderContext& MaterialRenderContext) const;
	
	/**
	* Set the material shader parameters which depend on the mesh element being rendered.
	* @param Context - command context
	* @param PixelShader - The pixel shader to set the parameters for.
	* @param View - The view that is being rendered.
	* @param Mesh - The mesh that is being rendered.
	* @param bBackFace - True if the backfaces of a two-sided material are being rendered.
	*/
	void SetMesh(
		FShader* PixelShader,
		const FMeshElement& Mesh,
		const FSceneView& View,
		UBOOL bBackFace
		) const;

	friend FArchive& operator<<(FArchive& Ar,FMaterialPixelShaderParameters& Parameters);

private:
	template<typename ParameterType> struct TUniformParameter
	{
		BYTE Type;
		INT Index;
		ParameterType ShaderParameter;
		friend FArchive& operator<<(FArchive& Ar,TUniformParameter<ParameterType>& P)
		{
			return Ar << P.Type << P.Index << P.ShaderParameter;
		}
	};
	TArray<TUniformParameter<FShaderParameter> > UniformParameters;
	TArray<TUniformParameter<FShaderResourceParameter> > UniformResourceParameters;
	/** matrix parameter for materials with a world transform */
	FShaderParameter LocalToWorldParameter;
	/** matrix parameter for materials with a local transform */
	FShaderParameter WorldToLocalParameter;
	/** matrix parameter for materials with a view transform */
	FShaderParameter WorldToViewParameter;
	/** matrix parameter for materials with a world position transform */
	FShaderParameter InvViewProjectionParameter;
	/** matrix parameter for materials with a world position node */
	FShaderParameter ViewProjectionParameter;
	/** world-space camera position */
	FShaderParameter CameraWorldPositionParameter;
	/** The scene texture parameters. */
	FSceneTextureShaderParameters SceneTextureParameters;
	/** Parameter indicating whether the front-side or the back-side of a two-sided material is being rendered. */
	FShaderParameter TwoSidedSignParameter;
	/** Inverse gamma parameter. Only used when USE_GAMMA_CORRECTION 1 */
	FShaderParameter InvGammaParameter;
	/** Parameter for distance to far plane for the decal (local or world space) */
	FShaderParameter DecalFarPlaneDistanceParameter;
};

/**
 * A shader meta type for material-linked shaders.
 */
class FMaterialShaderType : public FShaderType
{
public:

	/**
	 * Finds a FMaterialShaderType by name.
	 */
	static FMaterialShaderType* GetTypeByName(const FString& TypeName);

	struct CompiledShaderInitializerType : FGlobalShaderType::CompiledShaderInitializerType
	{
		const FMaterial* Material;
		CompiledShaderInitializerType(
			FShaderType* InType,
			const FShaderCompilerOutput& CompilerOutput,
			const FMaterial* InMaterial
			):
			FGlobalShaderType::CompiledShaderInitializerType(InType,CompilerOutput),
			Material(InMaterial)
		{}
	};
	typedef FShader* (*ConstructCompiledType)(const CompiledShaderInitializerType&);
	typedef UBOOL (*ShouldCacheType)(EShaderPlatform,const FMaterial*);

	FMaterialShaderType(
		const TCHAR* InName,
		const TCHAR* InSourceFilename,
		const TCHAR* InFunctionName,
		DWORD InFrequency,
		INT InMinPackageVersion,
		INT InMinLicenseePackageVersion,
		ConstructSerializedType InConstructSerializedRef,
		ConstructCompiledType InConstructCompiledRef,
		ModifyCompilationEnvironmentType InModifyCompilationEnvironmentRef,
		ShouldCacheType InShouldCacheRef
		):
		FShaderType(InName,InSourceFilename,InFunctionName,InFrequency,InMinPackageVersion,InMinLicenseePackageVersion,InConstructSerializedRef,InModifyCompilationEnvironmentRef),
		ConstructCompiledRef(InConstructCompiledRef),
		ShouldCacheRef(InShouldCacheRef)
	{}

	/**
	 * Enqueues a compilation for a new shader of this type.
	 * @param Material - The material to link the shader with.
	 * @param MaterialShaderCode - The shader code for the material.
	 */
	void BeginCompileShader(
		const FMaterial* Material,
		const TCHAR* MaterialShaderCode,
		EShaderPlatform Platform
		);

	/**
	 * Either creates a new instance of this type or returns an equivalent existing shader.
	 * @param Material - The material to link the shader with.
	 * @param CurrentJob - Compile job that was enqueued by BeginCompileShader.
	 */
	FShader* FinishCompileShader(
		const FMaterial* Material,
		const FShaderCompileJob& CurrentJob
		);

	/**
	 * Checks if the shader type should be cached for a particular platform and material.
	 * @param Platform - The platform to check.
	 * @param Material - The material to check.
	 * @return True if this shader type should be cached.
	 */
	UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material) const
	{
		return (*ShouldCacheRef)(Platform,Material);
	}

	virtual DWORD GetSourceCRC();

	// Dynamic casting.
	virtual FMaterialShaderType* GetMaterialShaderType() { return this; }

private:
	ConstructCompiledType ConstructCompiledRef;
	ShouldCacheType ShouldCacheRef;
};

/**
 * The set of material shaders for a single material.
 */
class FMaterialShaderMap : public TShaderMap<FMaterialShaderType>, public FRefCountedObject
{
public:

	/**
	 * Finds the shader map for a material.
	 * @param StaticParameterSet - The static parameter set identifying the shader map
	 * @param Platform - The platform to lookup for
	 * @return NULL if no cached shader map was found.
	 */
	static FMaterialShaderMap* FindId(const FStaticParameterSet& StaticParameterSet, EShaderPlatform Platform);

	FMaterialShaderMap() :
		bRegistered(FALSE)
	{}

	// Destructor.
	~FMaterialShaderMap();

	/**
	 * Compiles the shaders for a material and caches them in this shader map.
	 * @param Material - The material to compile shaders for.
	 * @param InStaticParameters - the set of static parameters to compile for
	 * @param MaterialShaderCode - The shader code for Material.
	 * @param Platform - The platform to compile to
	 * @param OutErrors - Upon compilation failure, OutErrors contains a list of the errors which occured.
	 * @param bDebugDump - Dump out the preprocessed and disassembled shader for debugging.
	 * @return True if the compilation succeeded.
	 */
	UBOOL Compile(const FMaterial* Material,const FStaticParameterSet* InStaticParameters, const TCHAR* MaterialShaderCode,EShaderPlatform Platform,TArray<FString>& OutErrors,UBOOL bSilent = FALSE,UBOOL bDebugDump = FALSE);

	/**
	 * Checks whether the material shader map is missing any shader types necessary for the given material.
	 * @param Material - The material which is checked.
	 * @return True if the shader map has all of the shader types necessary.
	 */
	UBOOL IsComplete(const FMaterial* Material, UBOOL bSilent) const;

	/**
	 * Builds a list of the shaders in a shader map.
	 */
	void GetShaderList(TMap<FGuid,FShader*>& OutShaders) const;

	/**
	 * Begins initializing the shaders used by the material shader map.
	 */
	void BeginInit();

	/**
	 * Begins releasing the shaders used by the material shader map.
	 */
	void BeginRelease();

	/**
	 * Registers a material shader map in the global map so it can be used by materials.
	 */
	void Register();

	/**
	 * Merges in OtherMaterialShaderMap's shaders and FMeshMaterialShaderMaps
	 */
	void Merge(const FMaterialShaderMap* OtherMaterialShaderMap);

	/**
	 * Removes all entries in the cache with exceptions based on a shader type
	 * @param ShaderType - The shader type to flush or keep (depending on second param)
	 * @param bFlushAllButShaderType - TRUE if all shaders EXCEPT the given type should be flush. FALSE will flush ONLY the given shader type
	 */
	void FlushShadersByShaderType(FShaderType* ShaderType, UBOOL bFlushAllButShaderType=FALSE);

	/**
	 * Removes all entries in the cache with exceptions based on a vertex factory type
	 * @param ShaderType - The shader type to flush or keep (depending on second param)
	 * @param bFlushAllButVertexFactoryType - TRUE if all shaders EXCEPT the given type should be flush. FALSE will flush ONLY the given vertex factory type
	 */
	void FlushShadersByVertexFactoryType(FVertexFactoryType* VertexFactoryType, UBOOL bFlushAllButVertexFactoryType=FALSE);

	// Serializer.
	void Serialize(FArchive& Ar);

	// Accessors.
	const class FMeshMaterialShaderMap* GetMeshShaderMap(FVertexFactoryType* VertexFactoryType) const;
	const FStaticParameterSet& GetMaterialId() const { return StaticParameters; }
	EShaderPlatform GetShaderPlatform() const { return Platform; }

private:

	/** A global map from a material's static parameter set to any shader map cached for that material. */
	static TMap<FStaticParameterSet,FMaterialShaderMap*> GIdToMaterialShaderMap[SP_NumPlatforms];

	/** The material's cached shaders for vertex factory type dependent shaders. */
	TIndirectArray<class FMeshMaterialShaderMap> MeshShaderMaps;

	/** The persistent GUID of this material shader map. */
	FGuid MaterialId;

	/** The material's user friendly name, typically the object name. */
	FString FriendlyName;

	/** The platform this shader map was compiled with */
	EShaderPlatform Platform;

	/** The static parameter set that this shader map was compiled with */
	FStaticParameterSet StaticParameters;

	/** A map from vertex factory type to the material's cached shaders for that vertex factory type. */
	TMap<FVertexFactoryType*,class FMeshMaterialShaderMap*> VertexFactoryMap;

	/** Indicates whether this shader map has been registered in GIdToMaterialShaderMap */
	BITFIELD bRegistered : 1;

	/**
	 * Initializes VertexFactoryMap from the contents of MeshShaderMaps.
	 */
	void InitVertexFactoryMap();
};
