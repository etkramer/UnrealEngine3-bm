/*=============================================================================
	GlobalShader.cpp: Global shader implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "GlobalShader.h"
#include "StaticBoundShaderState.h"

/** The global shader map. */
TShaderMap<FGlobalShaderType>* GGlobalShaderMap[SP_NumPlatforms];

IMPLEMENT_SHADER_TYPE(,FNULLPixelShader,TEXT("NULLPixelShader"),TEXT("Main"),SF_Pixel,0,0);
IMPLEMENT_SHADER_TYPE(,FOneColorVertexShader,TEXT("OneColorShader"),TEXT("MainVertexShader"),SF_Vertex,0,0);
IMPLEMENT_SHADER_TYPE(,FOneColorPixelShader,TEXT("OneColorShader"),TEXT("MainPixelShader"),SF_Pixel,0,0);

FShader* FGlobalShaderType::CompileShader(EShaderPlatform Platform,TArray<FString>& OutErrors,UBOOL bDebugDump)
{
	// Construct the shader environment.
	FShaderCompilerEnvironment Environment;

	// Compile the shader code.
	FShaderCompilerOutput Output;

	//calculate the CRC for this global shader type and store it in the shader cache.
	//this will be used to determine when global shader files have changed.
	DWORD CurrentCRC = GetSourceCRC();
	UShaderCache::SetShaderTypeCRC(this, CurrentCRC, Platform);

	warnf(NAME_DevShaders, TEXT("	%s"), GetName());

	TArray<FShaderCompileJob> CompilationResults;
	STAT(DOUBLE GlobalShaderCompilingTime = 0);
	{
		SCOPE_SECONDS_COUNTER(GlobalShaderCompilingTime);
		// Enqueue the shader to be compiled
		FShaderType::BeginCompileShader(NULL, Platform, Environment);
		// Flush compiling
		GShaderCompilingThreadManager->FinishCompiling(CompilationResults, TEXT("Global"), FALSE, bDebugDump);
		check(CompilationResults.Num() == 1 && CompilationResults(0).ShaderType->GetGlobalShaderType() == this);
	}
	INC_FLOAT_STAT_BY(STAT_ShaderCompiling_GlobalShaders,(FLOAT)GlobalShaderCompilingTime);
	FShaderCompileJob& CurrentJob = CompilationResults(0);
	if (CurrentJob.bSucceeded)
	{
		FShader* Shader = FindShaderByOutput(CurrentJob.Output);
		if(!Shader)
		{
			// Create the shader.
			Shader = (*ConstructCompiledRef)(CompiledShaderInitializerType(this,CurrentJob.Output));
			CurrentJob.Output.ParameterMap.VerifyBindingsAreComplete(GetName(), (EShaderFrequency)CurrentJob.Output.Target.Frequency);
		}
		return Shader;
	}
	else
	{
		OutErrors = Output.Errors;
		return NULL;
	}
}

/**
* Calculates a CRC based on this shader type's source code and includes
*/
DWORD FGlobalShaderType::GetSourceCRC()
{
	return FShaderType::GetSourceCRC();
}

void SerializeGlobalShaders(EShaderPlatform Platform,FArchive& Ar)
{
	check(IsInGameThread());

	/** An archive wrapper that is used to serialize the global shader map into a raw binary file. */
	class FGlobalShaderArchive : public FArchiveProxy
	{
	public:

		/** Initialization constructor. */
		FGlobalShaderArchive(FArchive& InInnerArchive)
		:	FArchiveProxy(InInnerArchive)
		{}

		// FArchive interface.
		virtual FArchive& operator<<( class FName& N )
		{
			// Serialize the name as a FString.
			if(InnerArchive.IsSaving())
			{
				FString NameString = N.ToString();
				InnerArchive << NameString;
			}
			else
			{
				FString NameString;
				InnerArchive << NameString;
				N = FName(*NameString);
			}
			return *this;
		}
		virtual FArchive& operator<<( class UObject*& Res )
		{
			appErrorf(TEXT("Global shader cache doesn't support object references"));
			return *this;
		}
	};

	TShaderMap<FGlobalShaderType>& GlobalShaderMap = *GetGlobalShaderMap(Platform);
	FShaderCache* GlobalShaderCache = GetGlobalShaderCache(Platform);
	check(GlobalShaderCache);

	// Serialize the global shader map binary file tag.
	static const DWORD ReferenceTag = 0x47534D42;
	DWORD Tag = ReferenceTag;
	Ar << Tag;
	checkf(Tag == ReferenceTag,TEXT("Global shader map binary file is missing GSMB tag."));

	// Serialize the version data.
	INT Version = GPackageFileVersion;
	INT LicenseeVersion = GPackageFileLicenseeVersion;
	Ar << Version;
	Ar << LicenseeVersion;
	if(Ar.IsLoading())
	{
		Ar.SetVer(Version);
		Ar.SetLicenseeVer(LicenseeVersion);
	}

	// Wrap the provided archive with our global shader proxy archive.
	FGlobalShaderArchive GlobalShaderArchive(Ar);

	// Serialize the global shaders.
	if(Ar.IsSaving())
	{
		TMap<FGuid,FShader*> GlobalShaders;
		GlobalShaderMap.GetShaderList(GlobalShaders);
		GlobalShaderCache->Save(GlobalShaderArchive,GlobalShaders);
	}
	else
	{
		GlobalShaderCache->Load(GlobalShaderArchive);
	}

	// Serialize the global shader map.
	GlobalShaderMap.Serialize(GlobalShaderArchive);
}

/**
 * Makes sure all global shaders are loaded and/or compiled for the passed in platform.
 *
 * @param	Platform	Platform to verify global shaders for
 */
void VerifyGlobalShaders(EShaderPlatform Platform)
{
	check(IsInGameThread());

	warnf(NAME_DevShaders, TEXT("Verifying Global Shaders for %s"), ShaderPlatformToText(Platform));

	// Ensure that the global shader map contains all global shader types.
	TShaderMap<FGlobalShaderType>* GlobalShaderMap = GetGlobalShaderMap(Platform);
	UBOOL bGlobalShaderCacheNeedToBeSaved = FALSE;
	for(TLinkedList<FShaderType*>::TIterator ShaderTypeIt(FShaderType::GetTypeList());ShaderTypeIt;ShaderTypeIt.Next())
	{
		FGlobalShaderType* GlobalShaderType = ShaderTypeIt->GetGlobalShaderType();
		if(GlobalShaderType && GlobalShaderType->ShouldCache(Platform))
		{
			if(!GlobalShaderMap->HasShader(GlobalShaderType))
			{
#if CONSOLE
				appErrorf(TEXT("Missing global shader %s, Please make sure cooking was successful."), GlobalShaderType->GetName());
#endif
				warnf(NAME_DevShaders, TEXT("	%s"), GlobalShaderType->GetName());

				// Compile this global shader type.
				TArray<FString> ShaderErrors;
				FShader* Shader = GlobalShaderType->CompileShader(Platform,ShaderErrors);
				if(Shader)
				{
					// Add the new global shader instance to the global shader map.
					// This will cause FShader::AddRef to be called, which will cause BeginInitResource(Shader) to be called.
					GlobalShaderMap->AddShader(GlobalShaderType,Shader);
				}
				else
				{
					appErrorf(TEXT("Failed to compile global shader %s"), GlobalShaderType->GetName());
				}

				// Indicate that the global shader cache needs to be saved.
				bGlobalShaderCacheNeedToBeSaved = TRUE;
			}
		}
	}
	GGlobalShaderMap[Platform]->BeginInit();

#if !CONSOLE
	if(bGlobalShaderCacheNeedToBeSaved)
	{
		// Save the global shader cache corresponding to this platform.
		FArchive* GlobalShaderFile = GFileManager->CreateFileWriter(*GetGlobalShaderCacheFilename(Platform));
		SerializeGlobalShaders(Platform,*GlobalShaderFile);
		delete GlobalShaderFile;
	}
#endif
}

TShaderMap<FGlobalShaderType>* GetGlobalShaderMap(EShaderPlatform Platform)
{
	// If the global shader map hasn't been created yet, create it.
	if(!GGlobalShaderMap[Platform])
	{
		// GetGlobalShaderMap is called the first time during startup in the main thread.
		check(IsInGameThread());

		GGlobalShaderMap[Platform] = new TShaderMap<FGlobalShaderType>();

		// Try to load the global shader map for this platform.
		FArchive* GlobalShaderFile = GFileManager->CreateFileReader(*GetGlobalShaderCacheFilename(Platform));
		if(GlobalShaderFile)
		{
			SerializeGlobalShaders(Platform,*GlobalShaderFile);
			delete GlobalShaderFile;
		}
		else if (CONSOLE)
		{
			appErrorf(TEXT("Couldn't find Global Shader Cache '%s', please recook."), *GetGlobalShaderCacheFilename(Platform));
		}

		// If any shaders weren't loaded, compile them now.
		VerifyGlobalShaders(Platform);
	}
	return GGlobalShaderMap[Platform];
}

/**
 * Forces a recompile of the global shaders.
 */
void RecompileGlobalShaders()
{
	if( !GUseSeekFreeLoading )
	{
		// Flush pending accesses to the existing global shaders.
		FlushRenderingCommands();

		GetGlobalShaderMap(GRHIShaderPlatform)->Empty();

		//invalidate global bound shader states so they will be created with the new shaders the next time they are set (in SetGlobalBoundShaderState)
		for(TLinkedList<FGlobalBoundShaderStateResource*>::TIterator It(FGlobalBoundShaderStateResource::GetGlobalBoundShaderStateList());It;It.Next())
		{
			BeginUpdateResourceRHI(*It);
		}

		VerifyGlobalShaders(GRHIShaderPlatform);
	}
}

/**
 * Recompiles the specified global shader types, and flushes their bound shader states.
 */
void RecompileGlobalShaders(const TArray<FShaderType*>& OutdatedShaderTypes)
{
	if( !GUseSeekFreeLoading )
	{
		// Flush pending accesses to the existing global shaders.
		FlushRenderingCommands();

		TShaderMap<FGlobalShaderType>* GlobalShaderMap = GetGlobalShaderMap(GRHIShaderPlatform);

		for (INT TypeIndex = 0; TypeIndex < OutdatedShaderTypes.Num(); TypeIndex++)
		{
			FGlobalShaderType* CurrentGlobalShaderType = OutdatedShaderTypes(TypeIndex)->GetGlobalShaderType();
			if (CurrentGlobalShaderType)
			{
				debugf(TEXT("Flushing Global Shader %s"), CurrentGlobalShaderType->GetName());
				GlobalShaderMap->RemoveShaderType(CurrentGlobalShaderType);
				
				//invalidate global bound shader states so they will be created with the new shaders the next time they are set (in SetGlobalBoundShaderState)
				for(TLinkedList<FGlobalBoundShaderStateResource*>::TIterator It(FGlobalBoundShaderStateResource::GetGlobalBoundShaderStateList());It;It.Next())
				{
					BeginUpdateResourceRHI(*It);
				}
			}
		}

		VerifyGlobalShaders(GRHIShaderPlatform);
	}
}

/**
 * Forces a recompile of only the specified group of global shaders. 
 * Also invalidates global bound shader states so that the new shaders will be used.
 */
void RecompileGlobalShaderGroup(EShaderRecompileGroup FlushGroup)
{
	if( !GUseSeekFreeLoading )
	{
		// Flush pending accesses to the existing global shaders.
		FlushRenderingCommands();

		TShaderMap<FGlobalShaderType>* GlobalShaderMap = GetGlobalShaderMap(GRHIShaderPlatform);
		TMap<FGuid,FShader*> GlobalShaderList;
		GlobalShaderMap->GetShaderList(GlobalShaderList);
		for(TMap<FGuid,FShader*>::TIterator ShaderIt(GlobalShaderList);ShaderIt;++ShaderIt)
		{
			FShader* CurrentGlobalShader = ShaderIt.Value();
			if (CurrentGlobalShader->GetRecompileGroup() == FlushGroup)
			{
				FShaderType* CurrentShaderType = CurrentGlobalShader->GetType();
				FGlobalShaderType* CurrentGlobalShaderType = CurrentShaderType->GetGlobalShaderType();
				check(CurrentGlobalShaderType);
				debugf(TEXT("Flushing Global Shader %s"), CurrentGlobalShaderType->GetName());
				GlobalShaderMap->RemoveShaderType(CurrentGlobalShaderType);
			}
		}

		debugf(TEXT("Flushing Global Bound Shader States..."));

		//invalidate global bound shader states so they will be created with the new shaders the next time they are set (in SetGlobalBoundShaderState)
		for(TLinkedList<FGlobalBoundShaderStateResource*>::TIterator It(FGlobalBoundShaderStateResource::GetGlobalBoundShaderStateList());It;It.Next())
		{
			BeginUpdateResourceRHI(*It);
		}

		VerifyGlobalShaders(GRHIShaderPlatform);
	}
}
