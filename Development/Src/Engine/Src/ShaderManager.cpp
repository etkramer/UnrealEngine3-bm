/*=============================================================================
	ShaderManager.cpp: Shader manager implementation
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "DiagnosticTable.h"

EShaderPlatform GRHIShaderPlatform = SP_PCD3D_SM3;
EMaterialShaderPlatform GCurrentMaterialPlatform = MSP_SM3;

/** 
* When enabled, shaders will be initialized individually when needed for rendering, 
* instead of all the shaders in a material shader map getting initialized when the material is loaded.
* This is useful for working around Nvidia drivers running out of paged pool memory, 
* but as a result there will be a hitch the first time a new shader is needed for rendering.
*/
static UBOOL ShouldInitShadersOnDemand()
{
	static UBOOL bInitShadersOnDemand = FALSE;
	static UBOOL bInitialized = FALSE;
	#if !CONSOLE
		if(!bInitialized)
		{
			bInitialized = TRUE;
			// get the option to initialize shaders on demand from the engine ini
			GConfig->GetBool( TEXT("Engine.ISVHacks"), TEXT("bInitializeShadersOnDemand"), bInitShadersOnDemand, GEngineIni );
		}
	#endif
	return bInitShadersOnDemand;
}

/** The shader file cache, used to minimize shader file reads */
TMap<FFilename, FString> GShaderFileCache;

/** Protects GShaderFileCache from simultaneous access by multiple threads. */
FCriticalSection FileCacheCriticalSection;

/** The shader file CRC cache, used to minimize loading and CRC'ing shader files */
TMap<FString, DWORD> GShaderCRCCache;

UBOOL FShaderParameterMap::FindParameterAllocation(const TCHAR* ParameterName,WORD& OutBufferIndex,WORD& OutBaseIndex,WORD& OutSize,WORD& OutSamplerIndex) const
{
	const FParameterAllocation* Allocation = ParameterMap.Find(ParameterName);
	if(Allocation)
	{
		OutBufferIndex = Allocation->BufferIndex;
		OutBaseIndex = Allocation->BaseIndex;
		OutSize = Allocation->Size;
		OutSamplerIndex = Allocation->SamplerIndex;
		Allocation->bBound = TRUE;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void FShaderParameterMap::AddParameterAllocation(const TCHAR* ParameterName,WORD BufferIndex,WORD BaseIndex,WORD Size,WORD SamplerIndex)
{
	FParameterAllocation Allocation;
	Allocation.BufferIndex = BufferIndex;
	Allocation.BaseIndex = BaseIndex;
	Allocation.Size = Size;
	Allocation.SamplerIndex = SamplerIndex;
	ParameterMap.Set(ParameterName,Allocation);
}

/** Returns TRUE if the specified parameter is bound by RHISetViewParameters instead of by the shader */
static UBOOL IsParameterBoundByView(const FString& ParameterName, EShaderFrequency Frequency)
{
	UBOOL bParameterBoundByView = FALSE;
	if (Frequency == SF_Vertex)
	{
		bParameterBoundByView = ParameterName == TEXT("ViewProjectionMatrix") || ParameterName == TEXT("CameraPosition");
	}
	else if (Frequency == SF_Pixel)
	{
		bParameterBoundByView = ParameterName == TEXT("ScreenPositionScaleBias") 
			|| ParameterName == TEXT("MinZ_MaxZRatio") 
			|| ParameterName == TEXT("SCENE_COLOR_BIAS_FACTOR");
	}
	return bParameterBoundByView;
}

/** Returns TRUE if the specified parameter being unbound is a known issue */
static UBOOL IsKnownUnboundParameter(const FString& ParameterName, EShaderFrequency Frequency)
{
	UBOOL bMakeException = FALSE;
	if (Frequency == SF_Pixel)
	{
		bMakeException = 
			// TTP 84767 is open to fix these
			ParameterName == TEXT("DecalLocalBinormal") 
			|| ParameterName == TEXT("DecalLocalTangent");
	}
	return bMakeException;
}

/** Checks that all parameters are bound and asserts if any aren't in a debug build */
void FShaderParameterMap::VerifyBindingsAreComplete(const TCHAR* ShaderTypeName, EShaderFrequency Frequency) const
{
#if _DEBUG
	UBOOL bBindingsComplete = TRUE;
	FString UnBoundParameters = TEXT("");
	for (TMap<FString,FParameterAllocation>::TConstIterator ParameterIt(ParameterMap);ParameterIt;++ParameterIt)
	{
		const FString& ParamName = ParameterIt.Key();
		const FParameterAllocation& ParamValue = ParameterIt.Value();
		if (!ParamValue.bBound && !IsParameterBoundByView(ParamName, Frequency) && !IsKnownUnboundParameter(ParamName, Frequency))
		{
			// Only valid parameters should be in the shader map
			checkSlow(ParamValue.Size > 0);
			bBindingsComplete = bBindingsComplete && ParamValue.bBound;
			UnBoundParameters += FString(TEXT("		Parameter ")) + ParamName + TEXT(" not bound!\n");
		}
	}
	
	if (!bBindingsComplete)
	{
		FString ErrorMessage = FString(TEXT("Found unbound parameters being used in shadertype ")) + ShaderTypeName + TEXT("\n") + UnBoundParameters;
		// An unbound parameter means the engine is not going to set its value (because it was never bound) 
		// but it will be used in rendering, which will most likely cause artifacts
		appErrorf(*ErrorMessage);
	}
#endif
}

void FShaderParameter::Bind(const FShaderParameterMap& ParameterMap,const TCHAR* ParameterName,UBOOL bIsOptional)
{
	WORD UnusedSamplerIndex = 0;
	if(!ParameterMap.FindParameterAllocation(ParameterName,BufferIndex,BaseIndex,NumBytes,UnusedSamplerIndex) && !bIsOptional)
	{
		appErrorf(TEXT("Failure to bind non-optional shader parameter %s"),ParameterName);
	}
}

FArchive& operator<<(FArchive& Ar,FShaderParameter& P)
{
	return Ar << P.BaseIndex << P.NumBytes << P.BufferIndex;
}

void FShaderResourceParameter::Bind(const FShaderParameterMap& ParameterMap,const TCHAR* ParameterName,UBOOL bIsOptional)
{
	WORD UnusedBufferIndex = 0;
	if(!ParameterMap.FindParameterAllocation(ParameterName,UnusedBufferIndex,BaseIndex,NumResources,SamplerIndex) && !bIsOptional)
	{
		appErrorf(TEXT("Failure to bind non-optional shader resource parameter %s"),ParameterName);
	}
}

FArchive& operator<<(FArchive& Ar,FShaderResourceParameter& P)
{
	return Ar << P.BaseIndex << P.NumResources << P.SamplerIndex;
}

/**
 * Sets the value of a pixel shader bool parameter.
 */
void SetPixelShaderBool(
	FPixelShaderRHIParamRef PixelShader,
	const FShaderParameter& Parameter,
	UBOOL Value
	)
{
	if (Parameter.GetNumBytes() > 0)
	{
		RHISetPixelShaderBoolParameter(
			PixelShader,
			Parameter.GetBaseIndex(),
			Value
			);
	}
}

TLinkedList<FShaderType*>*& FShaderType::GetTypeList()
{
	static TLinkedList<FShaderType*>* TypeList = NULL;
	return TypeList;
}

/**
* @return The global shader name to type map
*/
TMap<FName, FShaderType*>& FShaderType::GetNameToTypeMap()
{
	static TMap<FName, FShaderType*> NameToTypeMap;
	return NameToTypeMap;
}

/**
* Gets a list of FShaderTypes whose source file no longer matches what that type was compiled with
*/
void FShaderType::GetOutdatedTypes(TArray<FShaderType*>& OutdatedShaderTypes)
{
	for(TLinkedList<FShaderType*>::TIterator It(GetTypeList()); It; It.Next())
	{
		//find the CRC that the shader type was compiled with
		const DWORD* ShaderTypeCRC = UShaderCache::GetShaderTypeCRC(*It, GRHIShaderPlatform);
		//calculate the current CRC
		DWORD CurrentCRC = It->GetSourceCRC();

		//if they are not equal, the current type is outdated
		if (ShaderTypeCRC != NULL && CurrentCRC != *ShaderTypeCRC)
		{
			warnf(TEXT("ShaderType %s is outdated"), It->GetName());
			OutdatedShaderTypes.Push(*It);
			//remove the type's crc from the cache, since we know it is outdated now
			//this is necessary since the shader may not be compiled before the next CRC check and it will just be reported as outdated again
			UShaderCache::RemoveShaderTypeCRC(*It, GRHIShaderPlatform);
		}
	}
}

FArchive& operator<<(FArchive& Ar,FShaderType*& Ref)
{
	if(Ar.IsSaving())
	{
		FName FactoryName = Ref ? FName(Ref->Name) : NAME_None;
		Ar << FactoryName;
	}
	else if(Ar.IsLoading())
	{
		FName FactoryName = NAME_None;
		Ar << FactoryName;
		
		Ref = NULL;

		if(FactoryName != NAME_None)
		{
			// look for the shader type in the global name to type map
			FShaderType** ShaderType = FShaderType::GetNameToTypeMap().Find(FactoryName);
			if (ShaderType)
			{
				// if we found it, use it
				Ref = *ShaderType;
			}
		}
	}
	return Ar;
}

void FShaderType::RegisterShader(FShader* Shader)
{
	ShaderIdMap.Set(Shader->GetId(),Shader);
#if !CONSOLE
	Shader->CodeMapId = ShaderCodeMap.Add(Shader);
#endif
}

void FShaderType::DeregisterShader(FShader* Shader)
{
	ShaderIdMap.Remove(Shader->GetId());
#if !CONSOLE
	ShaderCodeMap.Remove(Shader->CodeMapId);
#endif
}

FShader* FShaderType::FindShaderByOutput(const FShaderCompilerOutput& Output) const
{
#if !CONSOLE
	FSetElementId CodeMapId = ShaderCodeMap.FindId(FShaderKey(Output.Code, Output.ParameterMap));
	return CodeMapId.IsValidId() ? ShaderCodeMap(CodeMapId) : NULL;
#else
	return NULL;
#endif
}

FShader* FShaderType::FindShaderById(const FGuid& Id) const
{
	return ShaderIdMap.FindRef(Id);
}

FShader* FShaderType::ConstructForDeserialization() const
{
	return (*ConstructSerializedRef)();
}

/**
* Calculates a CRC based on this shader type's source code and includes
*/
DWORD FShaderType::GetSourceCRC()
{
	return GetShaderFileCRC(GetShaderFilename());
}

/**
 * Enqueues a shader to be compiled with the shader type's compilation parameters, using the provided shader environment.
 * @param VFType - Optional vertex factory type that the shader belongs to.
 * @param Platform - Platform to compile for.
 * @param Environment - The environment to compile the shader in.
 */
void FShaderType::BeginCompileShader(FVertexFactoryType* VFType, EShaderPlatform Platform, const FShaderCompilerEnvironment& InEnvironment)
{
	// Allow the shader type to modify its compile environment.
	FShaderCompilerEnvironment Environment = InEnvironment;
	(*ModifyCompilationEnvironmentRef)(Platform, Environment);

	// Construct shader target for the shader type's frequency and the specified platform.
	FShaderTarget Target;
	Target.Platform = Platform;
	Target.Frequency = Frequency;

	// Compile the shader environment passed in with the shader type's source code.
	::BeginCompileShader(
		VFType,
		this,
		SourceFilename,
		FunctionName,
		Target,
		Environment
		);
}

FShader::FShader(const FGlobalShaderType::CompiledShaderInitializerType& Initializer):
	Key(Initializer.Code, Initializer.ParameterMap),
	Target(Initializer.Target),
	Type(Initializer.Type),
	NumRefs(0),
	NumInstructions(Initializer.NumInstructions),
	NumResourceInitRefs(0)
{
	Id = appCreateGuid();
	Type->RegisterShader(this);

	INC_DWORD_STAT_BY((Target.Frequency == SF_Vertex) ? STAT_VertexShaderMemory : STAT_PixelShaderMemory, Key.Code.Num());
}

FShader::~FShader()
{
	DEC_DWORD_STAT_BY((Target.Frequency == SF_Vertex) ? STAT_VertexShaderMemory : STAT_PixelShaderMemory, Key.Code.Num());
}

/**
* @return the shader's vertex shader
*/
const FVertexShaderRHIRef& FShader::GetVertexShader() 
{ 
	// If the shader resource hasn't been initialized yet, initialize it.
	// In game, shaders are initialized on load by default, but they will be initialized on demand if ShouldInitShadersOnDemand() is true.
	if(!CONSOLE && 
		(GIsEditor || ShouldInitShadersOnDemand()) && 
		!IsInitialized())
	{
		INC_DWORD_STAT_BY(STAT_ShaderCompiling_NumShadersInitialized, 1);
		InitResource();
	}

	checkSlow(IsInitialized());

	return VertexShader; 
}

/**
* @return the shader's pixel shader
*/
const FPixelShaderRHIRef& FShader::GetPixelShader() 
{ 
	// If the shader resource hasn't been initialized yet, initialize it.
	// In game, shaders are initialized on load by default, but they will be initialized on demand if ShouldInitShadersOnDemand() is true.
	if(!CONSOLE && 
		(GIsEditor || ShouldInitShadersOnDemand()) && 
		!IsInitialized())
	{
		INC_DWORD_STAT_BY(STAT_ShaderCompiling_NumShadersInitialized, 1);
		InitResource();
	}

	checkSlow(IsInitialized());

	return PixelShader; 
}

UBOOL FShader::Serialize(FArchive& Ar)
{
	BYTE TargetPlatform = Target.Platform;
	BYTE TargetFrequency = Target.Frequency;
	Ar << TargetPlatform << TargetFrequency;
	Target.Platform = TargetPlatform;
	Target.Frequency = TargetFrequency;

	Ar << Key.Code;
	Ar << Key.ParameterMapCRC;
	Ar << Id << Type;
	if(Ar.IsLoading())
	{
		Type->RegisterShader(this);
	}

	Ar << NumInstructions;
	
	INC_DWORD_STAT_BY((Target.Frequency == SF_Vertex) ? STAT_VertexShaderMemory : STAT_PixelShaderMemory, Key.Code.Num());
	return FALSE;
}

void FShader::InitRHI()
{
	// we can't have this called on the wrong platform's shaders
	if (Target.Platform != GRHIShaderPlatform)
	{
#if CONSOLE
		appErrorf( TEXT("FShader::Init got platform %s but expected %s"), ShaderPlatformToText((EShaderPlatform)Target.Platform), ShaderPlatformToText(GRHIShaderPlatform) );
#endif
		return;
	}
	check(Key.Code.Num());
	if(Target.Frequency == SF_Vertex)
	{
		VertexShader = RHICreateVertexShader(Key.Code);
	}
	else if(Target.Frequency == SF_Pixel)
	{
		PixelShader = RHICreatePixelShader(Key.Code);
	}

#if CONSOLE
	// Memory has been duplicated at this point.
	DEC_DWORD_STAT_BY((Target.Frequency == SF_Vertex) ? STAT_VertexShaderMemory : STAT_PixelShaderMemory, Key.Code.Num());
	Key.Code.Empty();
#endif
}

void FShader::ReleaseRHI()
{
	if (GIsEditor || ShouldInitShadersOnDemand())
	{
		DEC_DWORD_STAT_BY(STAT_ShaderCompiling_NumShadersInitialized, 1);
	}

	VertexShader.SafeRelease();
	PixelShader.SafeRelease();
}

void FShader::AddRef()
{
	++NumRefs;
}

void FShader::Release()
{
	if(--NumRefs == 0)
	{
		// Deregister the shader now to eliminate references to it by the type's ShaderIdMap and ShaderCodeMap.
		Type->DeregisterShader(this);

		// Send a release message to the rendering thread when the shader loses its last reference.
		BeginReleaseResource(this);

		BeginCleanup(this);
	}
}

void FShader::FinishCleanup()
{
	delete this;
}

void FShader::BeginInit()
{
	NumResourceInitRefs++;
	// Initialize the shader's resources the first time it is requested
	// unless we are initializing on demand only, in which case the shader will be initialized 
	// in GetPixelShader or GetVertexShader when it is requested for rendering.
	if (NumResourceInitRefs == 1 && !ShouldInitShadersOnDemand())
	{
		INC_DWORD_STAT_BY(STAT_ShaderCompiling_NumShadersInitialized, 1);
		BeginInitResource(this);
	}
}

void FShader::BeginRelease()
{
	// No need to ever release shader resources through this mechanism on console.  
	// Instead, shader resources will be released when the FShader is destroyed, which happens on map transition.
	// This allows us to throw away the game thread copy of the shader data when initializing the resource (Key.Code)
#if !CONSOLE
	// In the editor shader resources are initialized individually on demand, and currently not released
	check(!GIsEditor);
	NumResourceInitRefs--;
	check(NumResourceInitRefs >= 0);
	// Release the shader's resources the last time it is released
	if (NumResourceInitRefs == 0)
	{
		if (!ShouldInitShadersOnDemand())
		{
			DEC_DWORD_STAT_BY(STAT_ShaderCompiling_NumShadersInitialized, 1);
		}
		BeginReleaseResource(this);
	}
#endif
}

FArchive& operator<<(FArchive& Ar,FShader*& Ref)
{
	if(Ar.IsSaving())
	{
		if(Ref)
		{
			// Serialize the shader's ID and type.
			FGuid ShaderId = Ref->GetId();
			FShaderType* ShaderType = Ref->GetType();
			Ar << ShaderId << ShaderType;
		}
		else
		{
			FGuid ShaderId(0,0,0,0);
			FShaderType* ShaderType = NULL;
			Ar << ShaderId << ShaderType;
		}
	}
	else if(Ar.IsLoading())
	{
		// Deserialize the shader's ID and type.
		FGuid ShaderId;
		FShaderType* ShaderType = NULL;
		Ar << ShaderId << ShaderType;

		Ref = NULL;

		if(ShaderType)
		{
			// Find the shader using the ID and type.
			Ref = ShaderType->FindShaderById(ShaderId);
		}
	}

	return Ar;
}

/**
 * Recursively populates IncludeFilenames with the unique include filenames found in the shader file named Filename.
 */
void GetShaderIncludes(const TCHAR* Filename, TArray<FString> &IncludeFilenames, UINT DepthLimit)
{
	FString FileContents = LoadShaderSourceFile(Filename);
	//avoid an infinite loop with a 0 length string
	check(FileContents.Len() > 0);

	//find the first include directive
	TCHAR* IncludeBegin = appStrstr(*FileContents, TEXT("#include "));

	UINT SearchCount = 0;
	const UINT MaxSearchCount = 20;
	//keep searching for includes as long as we are finding new ones and haven't exceeded the fixed limit
	while (IncludeBegin != NULL && SearchCount < MaxSearchCount && DepthLimit > 0)
	{
		//find the first double quotation after the include directive
		TCHAR* IncludeFilenameBegin = appStrstr(IncludeBegin, TEXT("\""));
		//find the trailing double quotation
		TCHAR* IncludeFilenameEnd = appStrstr(IncludeFilenameBegin + 1, TEXT("\""));
		//construct a string between the double quotations
		FString ExtractedIncludeFilename((INT)(IncludeFilenameEnd - IncludeFilenameBegin - 1), IncludeFilenameBegin + 1);

		//CRC the template, not the filled out version so that this shader's CRC will be independent of which material references it.
		if (ExtractedIncludeFilename == TEXT("Material.usf"))
		{
			ExtractedIncludeFilename = TEXT("MaterialTemplate.usf");
		}

		//vertex factories need to be handled separately
		if (ExtractedIncludeFilename != TEXT("VertexFactory.usf"))
		{
			GetShaderIncludes(*ExtractedIncludeFilename, IncludeFilenames, DepthLimit - 1);
			IncludeFilenames.AddUniqueItem(ExtractedIncludeFilename);
		}
		
		//find the next include directive
		IncludeBegin = appStrstr(IncludeFilenameEnd + 1, TEXT("#include "));
		SearchCount++;
	}
}

/**
 * Calculates a CRC for the given filename if it does not already exist in the CRC cache.
 * @param Filename - shader file to CRC
 * @param CombineCRC - an existing CRC to combine the result with
 */
DWORD GetShaderFileCRC(const TCHAR* Filename, DWORD CombineCRC)
{
	// Make sure we are only accessing GShaderCRCCache from one thread
	check(IsInGameThread());
	STAT(DOUBLE CRCTime = 0);
	{
		SCOPE_SECONDS_COUNTER(CRCTime);

		DWORD* CachedCRC = GShaderCRCCache.Find(Filename);

		//if a CRC for this filename has been cached, use that
		if (CachedCRC)
		{
			return *CachedCRC;
		}

		TArray<FString> IncludeFilenames;

		//get the list of includes this file contains
		GetShaderIncludes(Filename, IncludeFilenames);

		for (INT IncludeIndex = 0; IncludeIndex < IncludeFilenames.Num(); IncludeIndex++)
		{
			//load the source file and CRC it
			FString IncludeFileContents = LoadShaderSourceFile(*IncludeFilenames(IncludeIndex));
			CombineCRC = appMemCrc(*IncludeFileContents, IncludeFileContents.Len() * sizeof(TCHAR), CombineCRC);
		}

		FString FileContents = LoadShaderSourceFile(Filename);
		CombineCRC = appMemCrc(*FileContents, FileContents.Len() * sizeof(TCHAR), CombineCRC);

		//update the CRC cache
		GShaderCRCCache.Set(*FString(Filename), CombineCRC);
	}
	INC_FLOAT_STAT_BY(STAT_ShaderCompiling_CRCingShaderFiles,(FLOAT)CRCTime);
	return CombineCRC;
}

/**
 * Flushes the shader file and CRC cache.
 */
void FlushShaderFileCache()
{
	GShaderCRCCache.Empty();
	GShaderFileCache.Empty();
}

/**
 * Loads the shader file with the given name.
 * @return The contents of the shader file.
 */
FString LoadShaderSourceFile(const TCHAR* Filename)
{
	// Protect GShaderFileCache from simultaneous access by multiple threads
	FScopeLock ScopeLock(&FileCacheCriticalSection);

	FString	FileContents;
	STAT(DOUBLE ShaderFileLoadingTime = 0);
	{
		SCOPE_SECONDS_COUNTER(ShaderFileLoadingTime);

		// Load the specified file from the System/Shaders directory.
		FFilename ShaderFilename = FString(appBaseDir()) * appShaderDir() * FFilename(Filename).GetCleanFilename();

		if (ShaderFilename.GetExtension() != TEXT("usf"))
		{
			ShaderFilename += TEXT(".usf");
		}

		FString* CachedFile = GShaderFileCache.Find(ShaderFilename);

		//if this file has already been loaded and cached, use that
		if (CachedFile)
		{
			FileContents = *CachedFile;
		}
		else
		{
			// verify SHA hash of shader files on load. missing entries trigger an error
			if( !appLoadFileToString(FileContents, *ShaderFilename, GFileManager, LoadFileHash_EnableVerify|LoadFileHash_ErrorMissingHash) )
			{
				appErrorf(TEXT("Couldn't load shader file \'%s\'"),Filename);
			}

			//update the shader file cache
			GShaderFileCache.Set(ShaderFilename, *FileContents);
		}
	}

	INC_FLOAT_STAT_BY(STAT_ShaderCompiling_LoadingShaderFiles,(FLOAT)ShaderFileLoadingTime);
	return FileContents;
}

/**
 * Dumps shader stats to the log.
 */
void DumpShaderStats()
{
#if ALLOW_DEBUG_FILES
	FDiagnosticTableViewer ShaderTypeViewer(*FDiagnosticTableViewer::GetUniqueTemporaryFilePath(TEXT("ShaderTypes")));

	// Iterate over all shader types and log stats.
	INT TotalShaderCount	= 0;
	INT TotalTypeCount		= 0;

	// Write a row of headings for the table's columns.
	ShaderTypeViewer.AddColumn(TEXT("Type"));
	ShaderTypeViewer.AddColumn(TEXT("Instances"));
	ShaderTypeViewer.AddColumn(TEXT("Average instructions"));
	ShaderTypeViewer.CycleRow();

	for( TLinkedList<FShaderType*>::TIterator It(FShaderType::GetTypeList()); It; It.Next() )
	{
		const FShaderType* Type = *It;

		// Calculate the average instruction count of instances of this shader type.
		FLOAT AverageNumInstructions = 0.0f;
		if(Type->GetNumShaders())
		{
			INT NumInitializedInstructions = 0;
			for(TMap<FGuid,FShader*>::TConstIterator ShaderIt(Type->ShaderIdMap);ShaderIt;++ShaderIt)
			{
				NumInitializedInstructions += ShaderIt.Value()->GetNumInstructions();
			}
			AverageNumInstructions = (FLOAT)NumInitializedInstructions / (FLOAT)Type->GetNumShaders();
		}

		// Write a row for the shader type.
		ShaderTypeViewer.AddColumn(Type->GetName());
		ShaderTypeViewer.AddColumn(TEXT("%u"),Type->GetNumShaders());
		ShaderTypeViewer.AddColumn(TEXT("%.1f"),AverageNumInstructions);
		ShaderTypeViewer.CycleRow();

		TotalShaderCount += Type->GetNumShaders();
		TotalTypeCount++;
	}

	// Write a total row.
	ShaderTypeViewer.AddColumn(TEXT("Total"));
	ShaderTypeViewer.AddColumn(TEXT("%u"),TotalShaderCount);
	ShaderTypeViewer.CycleRow();

	// Launch the viewer.
	ShaderTypeViewer.OpenViewer();
#endif
}

//
FShaderType* FindShaderTypeByName(const TCHAR* ShaderTypeName)
{
	for(TLinkedList<FShaderType*>::TIterator ShaderTypeIt(FShaderType::GetTypeList());ShaderTypeIt;ShaderTypeIt.Next())
	{
		if(!appStricmp(ShaderTypeIt->GetName(),ShaderTypeName))
		{
			return *ShaderTypeIt;
		}
	}
	return NULL;
}
