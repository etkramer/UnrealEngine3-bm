/*=============================================================================
	ShaderManager.h: Shader manager definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __SHADERMANAGER_H__
#define __SHADERMANAGER_H__

// Forward declarations.
class FShaderType;

extern EShaderPlatform GRHIShaderPlatform;

/** The minimum package version which stores valid compiled shaders.  This can be used to force a recompile of all shaders. */
#define VER_MIN_SHADER	VER_D3D10_SHADER_PARAMETER_CHANGES

/** Same as VER_MIN_SHADER, but for the licensee package version. */
#define LICENSEE_VER_MIN_SHADER	0

/** Controls discarding of shaders whose source .usf file has changed since the shader was compiled. */
extern UBOOL GAutoReloadChangedShaders;

/** A shader parameter's register binding. */
class FShaderParameter
{
public:
	FShaderParameter(): NumBytes(0) {}
	void Bind(const FShaderParameterMap& ParameterMap,const TCHAR* ParameterName,UBOOL bIsOptional = FALSE);
	friend FArchive& operator<<(FArchive& Ar,FShaderParameter& P);
	UBOOL IsBound() const { return NumBytes > 0; }
	UINT GetBufferIndex() const { return BufferIndex; }
	UINT GetBaseIndex() const { return BaseIndex; }
	UINT GetNumBytes() const { return NumBytes; }
private:
	WORD BufferIndex;
	WORD BaseIndex;
	WORD NumBytes;
};

/** A shader resource binding (just textures for now). */
class FShaderResourceParameter
{
public:
	FShaderResourceParameter(): NumResources(0) {}
	void Bind(const FShaderParameterMap& ParameterMap,const TCHAR* ParameterName,UBOOL bIsOptional = FALSE);
	friend FArchive& operator<<(FArchive& Ar,FShaderResourceParameter& P);
	UBOOL IsBound() const { return NumResources > 0; }
	UINT GetBaseIndex() const { return BaseIndex; }
	UINT GetNumResources() const { return NumResources; }
	UINT GetSamplerIndex() const { return SamplerIndex; }
private:
	WORD BaseIndex;
	WORD NumResources;
	WORD SamplerIndex;
};

/**
 * Pads a shader parameter value to the an integer number of shader registers.
 * @param Value - A pointer to the shader parameter value.
 * @param NumBytes - The number of bytes in the shader parameter value.
 * @return A pointer to the padded shader parameter value.
 */
FORCEINLINE const void* GetPaddedShaderParameterValue(const void* Value,UINT NumBytes)
{
	// Compute the number of bytes of padding to add, assuming the shader array element alignment is a power of two.
	const UINT NumPaddedBytes = Align(NumBytes,ShaderArrayElementAlignBytes);
	if(NumPaddedBytes != NumBytes)
	{
		// If padding is needed, use a lazily sized global buffer to hold the padded shader parameter value.
		static TArray<BYTE> PaddedShaderParameterValueBuffer;
		if((UINT)PaddedShaderParameterValueBuffer.Num() < NumPaddedBytes)
		{
			PaddedShaderParameterValueBuffer.Empty(NumPaddedBytes);
			PaddedShaderParameterValueBuffer.Add(NumPaddedBytes);
		}

		// Copy the shader parameter value into the padded shader buffer.
		appMemcpy(&PaddedShaderParameterValueBuffer(0),Value,NumBytes);

		// Write zeros to the rest of the padded buffer.
		appMemzero(&PaddedShaderParameterValueBuffer(NumBytes),NumPaddedBytes - NumBytes);

		return &PaddedShaderParameterValueBuffer(0);
	}
	else
	{
		// If padding isn't needed, simply return a pointer to the input value.
		return Value;
	}
}

/**
 * Sets the value of a vertex shader parameter.
 * A template parameter specified the type of the parameter value.
 */
template<class ParameterType>
void SetVertexShaderValue(
	FVertexShaderRHIParamRef VertexShader,
	const FShaderParameter& Parameter,
	const ParameterType& Value,
	UINT ElementIndex = 0
	)
{
	const UINT AlignedTypeSize = Align(sizeof(ParameterType),ShaderArrayElementAlignBytes);
	const INT NumBytesToSet = Min<INT>(sizeof(ParameterType),Parameter.GetNumBytes() - ElementIndex * AlignedTypeSize);
	if(NumBytesToSet > 0)
	{
		RHISetVertexShaderParameter(
			VertexShader,
			Parameter.GetBufferIndex(),
			Parameter.GetBaseIndex() + ElementIndex * AlignedTypeSize,
			(UINT)NumBytesToSet,
			&Value
			);
	}
}

/**
 * Sets the value of a pixel shader parameter.
 * A template parameter specified the type of the parameter value.
 */
template<class ParameterType>
void SetPixelShaderValue(
	FPixelShaderRHIParamRef PixelShader,
	const FShaderParameter& Parameter,
	const ParameterType& Value,
	UINT ElementIndex = 0
	)
{
	const UINT AlignedTypeSize = Align(sizeof(ParameterType),ShaderArrayElementAlignBytes);
	const INT NumBytesToSet = Min<INT>(sizeof(ParameterType),Parameter.GetNumBytes() - ElementIndex * AlignedTypeSize);
	if(NumBytesToSet > 0)
	{
		RHISetPixelShaderParameter(
			PixelShader,
			Parameter.GetBufferIndex(),
			Parameter.GetBaseIndex() + ElementIndex * AlignedTypeSize,
			(UINT)NumBytesToSet,
			&Value
			);
	}
}

/**
 * Sets the value of a pixel shader bool parameter.
 */
extern void SetPixelShaderBool(
	FPixelShaderRHIParamRef PixelShader,
	const FShaderParameter& Parameter,
	UBOOL Value
	);

/**
 * Sets the value of a vertex shader parameter array.
 * A template parameter specified the type of the parameter value.
 */
template<class ParameterType>
void SetVertexShaderValues(
	FVertexShaderRHIParamRef VertexShader,
	const FShaderParameter& Parameter,
	const ParameterType* Values,
	UINT NumElements,
	UINT BaseElementIndex = 0
	)
{
	const UINT AlignedTypeSize = Align(sizeof(ParameterType),ShaderArrayElementAlignBytes);
	const INT NumBytesToSet = Min<INT>(NumElements * AlignedTypeSize,Parameter.GetNumBytes() - BaseElementIndex * AlignedTypeSize);
	if(NumBytesToSet > 0)
	{
		RHISetVertexShaderParameter(
			VertexShader,
			Parameter.GetBufferIndex(),
			Parameter.GetBaseIndex() + BaseElementIndex * AlignedTypeSize,
			(UINT)NumBytesToSet,
			Values
			);
	}
}

/**
 * Sets the value of a pixel shader parameter array.
 * A template parameter specified the type of the parameter value.
 */
template<class ParameterType>
void SetPixelShaderValues(
	FPixelShaderRHIParamRef PixelShader,
	const FShaderParameter& Parameter,
	const ParameterType* Values,
	UINT NumElements,
	UINT BaseElementIndex = 0
	)
{
	const UINT AlignedTypeSize = Align(sizeof(ParameterType),ShaderArrayElementAlignBytes);
	const INT NumBytesToSet = Min<INT>(NumElements * AlignedTypeSize,Parameter.GetNumBytes() - BaseElementIndex * AlignedTypeSize);
	if(NumBytesToSet > 0)
	{
		RHISetPixelShaderParameter(
			PixelShader,
			Parameter.GetBufferIndex(),
			Parameter.GetBaseIndex() + BaseElementIndex * AlignedTypeSize,
			(UINT)NumBytesToSet,
			Values
			);
	}
}

/**
 * Sets the value of a shader texture parameter.
 */
FORCEINLINE void SetTextureParameter(
	FPixelShaderRHIParamRef PixelShader,
	const FShaderResourceParameter& Parameter,
	const FTexture* Texture,
	UINT ElementIndex = 0,
	FLOAT MipBias = 0.0f
	)
{
	if(Parameter.IsBound())
	{
		check(ElementIndex < Parameter.GetNumResources());
		Texture->LastRenderTime = GCurrentTime;
		RHISetSamplerState(
			PixelShader,
			Parameter.GetBaseIndex() + ElementIndex,
			Parameter.GetSamplerIndex() + ElementIndex,
			Texture->SamplerStateRHI,
			Texture->TextureRHI,
			MipBias
			);
	}
}

/**
 * Sets the value of a shader texture parameter.
 */
FORCEINLINE void SetTextureParameter(
	FPixelShaderRHIParamRef PixelShader,
	const FShaderResourceParameter& Parameter,
	FSamplerStateRHIParamRef SamplerStateRHI,
	FTextureRHIParamRef TextureRHI,
	UINT ElementIndex = 0
	)
{
	if(Parameter.IsBound())
	{
		check(ElementIndex < Parameter.GetNumResources());
		RHISetSamplerState(
			PixelShader,
			Parameter.GetBaseIndex() + ElementIndex,
			Parameter.GetSamplerIndex() + ElementIndex,
			SamplerStateRHI,
			TextureRHI,
			0.0f
			);
	}
}

/**
 *	Shader recompile groups are used to minimize shader iteration time. 
 *  Shaders return their group from their GetRecompileGroup() member function.
 *  Currently only global shaders make use of these groups.
 */
enum EShaderRecompileGroup
{
	SRG_GLOBAL_MISC_SHADOW,
	SRG_GLOBAL_BPCF_SHADOW_LOW,
	SRG_GLOBAL_BPCF_SHADOW_MEDIUM_HIGH,
	SRG_GLOBAL_MISC,
	SRG_MISC
};

class FShaderKey
{
public:

	FShaderKey() : 
		ParameterMapCRC(0)
	{}

	FShaderKey(const TArray<BYTE>& InCode, const FShaderParameterMap& InParameterMap) : 
		Code(InCode)
	{
		ParameterMapCRC = InParameterMap.GetCRC();
	}

	TArray<BYTE> Code;
	DWORD ParameterMapCRC;
};

/**
 * A compiled shader and its parameter bindings.
 */
class FShader : public FRenderResource, public FDeferredCleanupInterface
{
	friend class FShaderType;
public:

	struct CompiledShaderInitializerType
	{
		FShaderType* Type;
		FShaderTarget Target;
		const TArray<BYTE>& Code;
		const FShaderParameterMap& ParameterMap;
		UINT NumInstructions;
		CompiledShaderInitializerType(
			FShaderType* InType,
			const FShaderCompilerOutput& CompilerOutput
			):
			Type(InType),
			Target(CompilerOutput.Target),
			Code(CompilerOutput.Code),
			ParameterMap(CompilerOutput.ParameterMap),
			NumInstructions(CompilerOutput.NumInstructions)
		{}
	};

	FShader() : 
		Type(NULL), 
		NumRefs(0),
		NumResourceInitRefs(0)
	{}

	FShader(const CompiledShaderInitializerType& Initializer);
	virtual ~FShader();

	/**
	 * Can be overridden by FShader subclasses to modify their compile environment just before compilation occurs.
	 */
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

	// Serializer.
	virtual UBOOL Serialize(FArchive& Ar);

	// FRenderResource interface.
	virtual void InitRHI();
	virtual void ReleaseRHI();

	// Reference counting.
	void AddRef();
	void Release();
	virtual void FinishCleanup();

	// Resource handling from the game thread
	void BeginInit();
	void BeginRelease();

	/**
	* @return the shader's vertex shader
	*/
	const FVertexShaderRHIRef& GetVertexShader();
	/**
	* @return the shader's vertex shader
	*/
	const FPixelShaderRHIRef& GetPixelShader();

	/**
	* @return the shader's recompile group
	*/
	virtual EShaderRecompileGroup GetRecompileGroup() const
	{
		return SRG_MISC;
	}

	// Accessors.
	const FGuid& GetId() const { return Id; }
	FShaderType* GetType() const { return Type; }
	const TArray<BYTE>& GetCode() const { return Key.Code; }
	const FShaderKey& GetKey() const { return Key; }
	UINT GetNumInstructions() const { return NumInstructions; }

	/**
	 * Serializes a shader reference by GUID.
	 */
	friend FArchive& operator<<(FArchive& Ar,FShader*& Ref);

private:

	FShaderKey Key;

	FShaderTarget Target;
	FVertexShaderRHIRef VertexShader;
	FPixelShaderRHIRef PixelShader;

	/** The shader type. */
	FShaderType* Type;

	/** A unique identifier for the shader. */
	FGuid Id;

	/** The number of references to this shader. */
	mutable UINT NumRefs;

	/** The shader's element id in the shader code map. */
	FSetElementId CodeMapId;

	/** The number of instructions the shader takes to execute. */
	UINT NumInstructions;

	/** 
	 * The number of times this shader has been requested to be initialized on the game thread. 
	 * This is used to know when to release the shader's resources even when it is not being deleted.
	 */
	mutable INT NumResourceInitRefs;
};

/**
 * An object which is used to serialize/deserialize, compile, and cache a particular shader class.
 */
class FShaderType
{
public:

	typedef class FShader* (*ConstructSerializedType)();
	typedef void (*ModifyCompilationEnvironmentType)(EShaderPlatform,FShaderCompilerEnvironment&);

	/**
	* @return The global shader factory list.
	*/
	static TLinkedList<FShaderType*>*& GetTypeList();

	/**
	* @return The global shader name to type map
	*/
	static TMap<FName, FShaderType*>& GetNameToTypeMap();

	/**
	* Gets a list of FShaderTypes whose source file no longer matches what that type was compiled with
	*/
	static void GetOutdatedTypes(TArray<FShaderType*>& OutdatedShaderTypes);

	/**
	 * Minimal initialization constructor.
	 */
	FShaderType(
		const TCHAR* InName,
		const TCHAR* InSourceFilename,
		const TCHAR* InFunctionName,
		DWORD InFrequency,
		INT InMinPackageVersion,
		INT InMinLicenseePackageVersion,
		ConstructSerializedType InConstructSerializedRef,
		ModifyCompilationEnvironmentType InModifyCompilationEnvironmentRef
		):
		Name(InName),
		SourceFilename(InSourceFilename),
		FunctionName(InFunctionName),
		Frequency(InFrequency),
		MinPackageVersion(InMinPackageVersion),
		MinLicenseePackageVersion(InMinLicenseePackageVersion),
		ConstructSerializedRef(InConstructSerializedRef),
		ModifyCompilationEnvironmentRef(InModifyCompilationEnvironmentRef)
	{
		//make sure the name is shorter than the maximum serializable length
		check(appStrlen(InName) < NAME_SIZE);

		// register this shader type
		(new TLinkedList<FShaderType*>(this))->Link(GetTypeList());
		GetNameToTypeMap().Set(FName(InName), this);

		// Assign the vertex factory type the next unassigned hash index.
		static DWORD NextHashIndex = 0;
		HashIndex = NextHashIndex++;
	}

	/**
	 * Registers a shader for lookup by ID or code.
	 */
	void RegisterShader(FShader* Shader);

	/**
	 * Removes a shader from the ID and code lookup maps.
	 */
	void DeregisterShader(FShader* Shader);

	/**
	 * Finds a shader of this type with the specified output.
	 * @return NULL if no shader with the specified code was found.
	 */
	FShader* FindShaderByOutput(const FShaderCompilerOutput& Output) const;

	/**
	 * Finds a shader of this type by ID.
	 * @return NULL if no shader with the specified ID was found.
	 */
	FShader* FindShaderById(const FGuid& Id) const;

	/**
	 * Constructs a new instance of the shader type for deserialization.
	 */
	FShader* ConstructForDeserialization() const;

	// Accessors.
	DWORD GetFrequency() const 
	{ 
		return Frequency; 
	}
	INT GetMinPackageVersion() const 
	{ 
		return MinPackageVersion; 
	}
	INT GetMinLicenseePackageVersion() const
	{
		return MinLicenseePackageVersion;
	}
	const TCHAR* GetName() const 
	{ 
		return Name; 
	}
	const TCHAR* GetShaderFilename() const 
	{ 
		return SourceFilename; 
	}
	/** 
	 * Returns the number of shaders of this type.
	 *
	 * @return number of shaders in shader code map
	 */
	INT GetNumShaders() const
	{
		return ShaderCodeMap.Num();
	}

	/**
	 * Serializes a shader type reference by name.
	 */
	friend FArchive& operator<<(FArchive& Ar,FShaderType*& Ref);
	
	// Hash function.
	friend DWORD GetTypeHash(FShaderType* Ref)
	{
		return Ref ? Ref->HashIndex : 0;
	}

	/**
	* Calculates a CRC based on this shader type's source code and includes
	*/
	virtual DWORD GetSourceCRC();

	// Dynamic casts.
	virtual class FGlobalShaderType* GetGlobalShaderType() { return NULL; }
	virtual class FMaterialShaderType* GetMaterialShaderType() { return NULL; }
	virtual class FMeshMaterialShaderType* GetMeshMaterialShaderType() { return NULL; }

protected:

	/**
	 * Enqueues a shader to be compiled with the shader type's compilation parameters, using the provided shader environment.
	 * @param VFType - Optional vertex factory type that the shader belongs to.
	 * @param Platform - Platform to compile for.
	 * @param Environment - The environment to compile the shader in.
	 */
	void BeginCompileShader(FVertexFactoryType* VFType, EShaderPlatform Platform, const FShaderCompilerEnvironment& Environment);

private:
	DWORD HashIndex;
	const TCHAR* Name;
	const TCHAR* SourceFilename;
	const TCHAR* FunctionName;
	DWORD Frequency;
	INT MinPackageVersion;
	INT MinLicenseePackageVersion;

	ConstructSerializedType ConstructSerializedRef;
	ModifyCompilationEnvironmentType ModifyCompilationEnvironmentRef;

	/** A map from shader ID to shader.  A shader will be removed from it when deleted, so this doesn't need to use a TRefCountPtr. */
	TMap<FGuid,FShader*> ShaderIdMap;

	// DumpShaderStats needs to access ShaderIdMap.
	friend void DumpShaderStats();

	/**
	 * Functions to extract the shader code from a FShader* as a key for TSet.
	 */
	struct FShaderCodeKeyFuncs : BaseKeyFuncs<FShader*,FShaderKey,TRUE>
	{
		static const KeyType& GetSetKey(FShader* Shader)
		{
			return Shader->GetKey();
		}

		static UBOOL Matches(const KeyType& A, const KeyType& B)
		{
			return A.ParameterMapCRC == B.ParameterMapCRC && A.Code == B.Code;
		}

		static DWORD GetKeyHash(const KeyType& ShaderKey)
		{
			return appMemCrc(&ShaderKey.Code(0),ShaderKey.Code.Num()) ^ ShaderKey.ParameterMapCRC;
		}
	};

	/** A map from shader code to shader. */
	TSet<FShader*,FShaderCodeKeyFuncs> ShaderCodeMap;
};

/**
 * A macro to declare a new shader type.  This should be called in the class body of the new shader type.
 * @param ShaderClass - The name of the class representing an instance of the shader type.
 * @param ShaderMetaTypeShortcut - The shortcut for the shader meta type: simple, material, meshmaterial, etc.  The shader meta type
 *	controls 
 */
#define DECLARE_SHADER_TYPE(ShaderClass,ShaderMetaTypeShortcut) \
	public: \
	typedef F##ShaderMetaTypeShortcut##ShaderType ShaderMetaType; \
	static ShaderMetaType StaticType; \
	static FShader* ConstructSerializedInstance() { return new ShaderClass(); } \
	static FShader* ConstructCompiledInstance(const ShaderMetaType::CompiledShaderInitializerType& Initializer) \
	{ return new ShaderClass(Initializer); }

/**
 * A macro to implement a shader type.
 */
#define IMPLEMENT_SHADER_TYPE(TemplatePrefix,ShaderClass,SourceFilename,FunctionName,Frequency,MinPackageVersion,MinLicenseePackageVersion) \
	TemplatePrefix \
	ShaderClass::ShaderMetaType ShaderClass::StaticType( \
		TEXT(#ShaderClass), \
		SourceFilename, \
		FunctionName, \
		Frequency, \
		Max(VER_MIN_SHADER,MinPackageVersion), \
		Max(LICENSEE_VER_MIN_SHADER,MinLicenseePackageVersion), \
		ShaderClass::ConstructSerializedInstance, \
		ShaderClass::ConstructCompiledInstance, \
		ShaderClass::ModifyCompilationEnvironment, \
		ShaderClass::ShouldCache \
		);

/**
 * A collection of shaders of different types, but the same meta type.
 */
template<typename ShaderMetaType>
class TShaderMap
{
public:

	/** Default constructor. */
	TShaderMap():
		ResourceInitCount(0)
	{}

	/** Finds the shader with the given type.  Asserts on failure. */
	template<typename ShaderType>
	ShaderType* GetShader() const
	{
		const TRefCountPtr<FShader>* ShaderRef = Shaders.Find(&ShaderType::StaticType);
		checkf(ShaderRef != NULL && *ShaderRef != NULL, TEXT("Failed to find shader type %s"), ShaderType::StaticType.GetName());
		return (ShaderType*)(FShader*)*ShaderRef;
	}

	/** Finds the shader with the given type.  May return NULL. */
	FShader* GetShader(FShaderType* ShaderType) const
	{
		const TRefCountPtr<FShader>* ShaderRef = Shaders.Find(ShaderType);
		return ShaderRef ? (FShader*)*ShaderRef : NULL;
	}

	/** Finds the shader with the given type. */
	UBOOL HasShader(ShaderMetaType* Type) const
	{
		const TRefCountPtr<FShader>* ShaderRef = Shaders.Find(Type);
		return ShaderRef != NULL && *ShaderRef != NULL;
	}

	void Serialize(FArchive& Ar)
	{
		// Serialize the shader references by factory and GUID.
		Ar << Shaders;
	}

	void AddShader(ShaderMetaType* Type,FShader* Shader)
	{
		Shaders.Set(Type,Shader);

		// Increment the shader initialization count by the number of times this map has been initialized.
		for(INT ResourceInitIteration = 0;ResourceInitIteration < ResourceInitCount;ResourceInitIteration++)
		{
			Shader->BeginInit();
		}
	}

	/**
	 * Merges OtherShaderMap's shaders
	 */
	void Merge(const TShaderMap<ShaderMetaType>* OtherShaderMap)
	{
		check(OtherShaderMap);
		TMap<FGuid,FShader*> OtherShaders;
		OtherShaderMap->GetShaderList(OtherShaders);
		for(TMap<FGuid,FShader*>::TIterator ShaderIt(OtherShaders);ShaderIt;++ShaderIt)
		{
			FShader* CurrentShader = ShaderIt.Value();
			check(CurrentShader);
			ShaderMetaType* CurrentShaderType = (ShaderMetaType*)CurrentShader->GetType();
			if (!HasShader(CurrentShaderType))
			{
				AddShader(CurrentShaderType,CurrentShader);
			}
		}
	}

	/**
	 * Removes the shader of the given type from the shader map
	 * @param Type Shader type to remove the entry for 
	 */
	void RemoveShaderType(ShaderMetaType* Type)
	{
		Shaders.Remove(Type);
	}

	/**
	 * Builds a list of the shaders in a shader map.
	 */
	void GetShaderList(TMap<FGuid,FShader*>& OutShaders) const
	{
		for(TMap<FShaderType*,TRefCountPtr<FShader> >::TConstIterator ShaderIt(Shaders);ShaderIt;++ShaderIt)
		{
			if(ShaderIt.Value())
			{
				OutShaders.Set(ShaderIt.Value()->GetId(),ShaderIt.Value());
			}
		}
	}
	
	/**
	 * Begins initializing the shaders used by the shader map.
	 */
	void BeginInit()
	{
		for(TMap<FShaderType*,TRefCountPtr<FShader> >::TConstIterator ShaderIt(Shaders);ShaderIt;++ShaderIt)
		{
			if(ShaderIt.Value())
			{
				ShaderIt.Value()->BeginInit();
			}
		}
		ResourceInitCount++;
	}

	/**
	 * Begins releasing the shaders used by the shader map.
	 */
	void BeginRelease()
	{
		for(TMap<FShaderType*,TRefCountPtr<FShader> >::TConstIterator ShaderIt(Shaders);ShaderIt;++ShaderIt)
		{
			if(ShaderIt.Value())
			{
				ShaderIt.Value()->BeginRelease();
			}
		}
		ResourceInitCount--;
		check(ResourceInitCount >= 0);
	}

	/**
	 *	IsEmpty - Returns TRUE if the map is empty
	 */
	UBOOL IsEmpty()
	{
		return ((Shaders.Num() == 0) ? TRUE : FALSE);
	}

	UINT GetNumShaders() const
	{
		return Shaders.Num();
	}

	/**
	 *	Empty - clears out all shaders held in the map
	 */
	void Empty()
	{
		Shaders.Empty();
	}

private:
	TMap<FShaderType*,TRefCountPtr<FShader> > Shaders;
	INT ResourceInitCount;
};

/**
 * A reference which is initialized with the requested shader type from a shader map.
 */
template<typename ShaderType>
class TShaderMapRef
{
public:
	TShaderMapRef(const TShaderMap<typename ShaderType::ShaderMetaType>* ShaderIndex):
	 Shader(ShaderIndex->template GetShader<ShaderType>()) // gcc3 needs the template quantifier so it knows the < is not a less-than
	{}
	ShaderType* operator->() const
	{
		return Shader;
	}
	ShaderType* operator*() const
	{
		return Shader;
	}
private:
	ShaderType* Shader;
};

/**
 * Recursively populates IncludeFilenames with the include filenames from Filename
 */
extern void GetShaderIncludes(const TCHAR* Filename, TArray<FString> &IncludeFilenames, UINT DepthLimit=7);

/**
 * Calculates a CRC for the given filename if it does not already exist in the CRC cache.
 * @param Filename - shader file to CRC
 * @param CombineCRC - an existing CRC to combine the result with
 */
extern DWORD GetShaderFileCRC(const TCHAR* Filename, DWORD CombineCRC=0);

/**
 * Flushes the shader file and CRC cache.
 */
extern void FlushShaderFileCache();

/**
 * Dumps shader stats to the log.
 */
extern void DumpShaderStats();

/**
 * Finds the shader type with a given name.
 *
 * @param ShaderTypeName - The name of the shader type to find.
 * @return The shader type, or NULL if none matched.
 */
extern FShaderType* FindShaderTypeByName(const TCHAR* ShaderTypeName);

/** Encapsulates scene texture shader parameter bindings. */
class FSceneTextureShaderParameters
{
public:

	/** Binds the parameters using a compiled shader's parameter map. */
	void Bind(const FShaderParameterMap& ParameterMap);

	/** Sets the scene texture parameters for the given view. */
	void Set(const FSceneView* View,FShader* PixelShader, ESamplerFilter ColorFilter=SF_Point) const;

	/** Sets the scene texture parameters for the given view with a user-specified scene color texture. */
	void Set(const FSceneView* View,FShader* PixelShader, ESamplerFilter ColorFilter, const FTexture2DRHIRef& DesiredSceneColorTexture) const;

	/** Serializer. */
	friend FArchive& operator<<(FArchive& Ar,FSceneTextureShaderParameters& P);

private:
	/** The SceneColorTexture parameter for materials that use SceneColor */
	FShaderResourceParameter SceneColorTextureParameter;
	/** The SceneDepthTexture parameter for materials that use SceneDepth */
	FShaderResourceParameter SceneDepthTextureParameter;
	/** Required parameter for using SceneDepthTexture on certain platforms. */
	FShaderParameter SceneDepthCalcParameter;
	/** Required parameter for using SceneColorTexture. */
	FShaderParameter ScreenPositionScaleBiasParameter;
};

#endif
