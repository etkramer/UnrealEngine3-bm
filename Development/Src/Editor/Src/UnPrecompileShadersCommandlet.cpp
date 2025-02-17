/*=============================================================================
	UnPrecompileShaderCommandlet.cpp: Shader precompiler (both local/reference cache).
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "EngineMaterialClasses.h"
#include "UnTerrain.h"
#include "GPUSkinVertexFactory.h"
#include "UnConsoleSupportContainer.h"

/*-----------------------------------------------------------------------------
	UPrecompileShadersCommandlet implementation.
-----------------------------------------------------------------------------*/

static FShaderType* FindShaderTypeByKeyword(const TCHAR* Keyword, UBOOL bVertexShader)
{
	for(TLinkedList<FShaderType*>::TIterator ShaderTypeIt(FShaderType::GetTypeList());ShaderTypeIt;ShaderTypeIt.Next())
	{
		if (appStristr(ShaderTypeIt->GetName(), Keyword) && appStristr(ShaderTypeIt->GetName(), bVertexShader ? TEXT("Vertex") : TEXT("Pixel")))
		{
			return *ShaderTypeIt;
		}
	}
	return NULL;
}

FLOAT MaterialProcessingTime = 0;

void UPrecompileShadersCommandlet::ProcessMaterial(UMaterial* Material)
{
	DOUBLE MaterialProcessingStart = appSeconds();
	if(Material->IsTemplate())
	{
		// Material templates don't have shaders.
		return;
	}

	SET_WARN_COLOR(COLOR_WHITE);
	warnf(TEXT("Processing %s..."), *Material->GetPathName());
	CLEAR_WARN_COLOR();

	UBOOL bForceCompile = FALSE;
	// if the name contains our force string, then flush the shader from the cache
	if (ForceName.Len() && Material->GetPathName().ToUpper().InStr(ForceName.ToUpper()) != -1)
	{
		SET_WARN_COLOR(COLOR_DARK_YELLOW);
		warnf(TEXT("Flushing shaders for %s"), *Material->GetPathName());
		CLEAR_WARN_COLOR();

		bForceCompile = TRUE;
	}

	// compile for the platform!
	for(INT PlatformIndex = 0;PlatformIndex < ShaderPlatforms.Num();PlatformIndex++)
	{
		const EShaderPlatform ShaderPlatform = ShaderPlatforms(PlatformIndex);
	    TRefCountPtr<FMaterialShaderMap> MapRef;
	    const EMaterialShaderPlatform MaterialPlatform = GetMaterialPlatform(ShaderPlatform);
	    FMaterialResource * MaterialResource = Material->GetMaterialResource(MaterialPlatform);
	    if (MaterialResource == NULL)
	    {
		    SET_WARN_COLOR(COLOR_YELLOW);
		    warnf(NAME_Warning, TEXT("%s has a NULL MaterialResource"), *Material->GetFullName());
		    CLEAR_WARN_COLOR();
		    return;
	    }
 
	    FStaticParameterSet EmptySet(MaterialResource->GetId());
	    if (!MaterialResource->Compile(&EmptySet, ShaderPlatform, MapRef, bForceCompile))
	    {
		    // handle errors
		    warnf(NAME_Warning, TEXT("Material failed to be compiled:"));
		    const TArray<FString>& CompileErrors = MaterialResource->GetCompileErrors();
		    for(INT ErrorIndex = 0; ErrorIndex < CompileErrors.Num(); ErrorIndex++)
		    {
			    warnf(NAME_Warning, TEXT("%s"), *CompileErrors(ErrorIndex));
		    }
	    }
	}
	MaterialProcessingTime += appSeconds() - MaterialProcessingStart;
}

FLOAT MaterialInstanceProcessingTime = 0;

void UPrecompileShadersCommandlet::ProcessMaterialInstance(UMaterialInstance* MaterialInstance)
{
	DOUBLE MIProcessingStart = appSeconds();

	// only compile this material instance if it has a static permutation resource
	if(MaterialInstance->bHasStaticPermutationResource)
	{
		SET_WARN_COLOR(COLOR_WHITE);
		warnf(TEXT("Processing %s..."), *MaterialInstance->GetPathName());
		CLEAR_WARN_COLOR();

		UBOOL bForceCompile = FALSE;
		// if the name contains our force string, then flush the shader from the cache
		if (ForceName.Len() && MaterialInstance->GetPathName().ToUpper().InStr(ForceName.ToUpper()) != -1)
		{
			SET_WARN_COLOR(COLOR_DARK_YELLOW);
			warnf(TEXT("Flushing shaders for %s"), *MaterialInstance->GetPathName());
			CLEAR_WARN_COLOR();

			bForceCompile = TRUE;
		}

		for(INT PlatformIndex = 0;PlatformIndex < ShaderPlatforms.Num();PlatformIndex++)
		{
			const EShaderPlatform ShaderPlatform = ShaderPlatforms(PlatformIndex);
		    const EMaterialShaderPlatform RequestedMaterialPlatform = GetMaterialPlatform(ShaderPlatform);
    
		    // compile the material instance's shaders for the platform
		    MaterialInstance->CacheResourceShaders(ShaderPlatform, bForceCompile, FALSE);
    
		    const EMaterialShaderPlatform MaterialPlatform = GetMaterialPlatform(ShaderPlatform);
		    FMaterialResource* MaterialResource = MaterialInstance->GetMaterialResource(MaterialPlatform);
			TRefCountPtr<FMaterialShaderMap> MaterialShaderMapRef = MaterialResource ? MaterialResource->GetShaderMap() : NULL;
		    if (!MaterialShaderMapRef)
		    {
			    // handle errors
			    warnf(NAME_Warning, TEXT("Failed to compile Material Instance:"));
			    const TArray<FString>& CompileErrors = MaterialResource->GetCompileErrors();
			    for(INT ErrorIndex = 0; ErrorIndex < CompileErrors.Num(); ErrorIndex++)
			    {
				    warnf(NAME_Warning, TEXT("%s"), *CompileErrors(ErrorIndex));
			    }
		    }
		}
	}
	MaterialInstanceProcessingTime += appSeconds() - MIProcessingStart;
}

FLOAT TerrainProcessingTime = 0;

void UPrecompileShadersCommandlet::ProcessTerrain(ATerrain* Terrain)
{
	DOUBLE TerrainProcessingStart = appSeconds();

	// Make sure materials are compiled for the target platform and add them to the shader cache embedded into seekfree packages.
	for(INT PlatformIndex = 0;PlatformIndex < ShaderPlatforms.Num();PlatformIndex++)
	{
		const EShaderPlatform ShaderPlatform = ShaderPlatforms(PlatformIndex);
	    const EMaterialShaderPlatform MaterialPlatform = GetMaterialPlatform(ShaderPlatform);
	    TArrayNoInit<FTerrainMaterialResource*>& CachedMaterials = Terrain->GetCachedTerrainMaterials(MaterialPlatform);
	    for (INT CachedMatIndex = 0; CachedMatIndex < CachedMaterials.Num(); CachedMatIndex++)
	    {
		    FTerrainMaterialResource* TMatRes = CachedMaterials(CachedMatIndex);
		    if (TMatRes)
		    {
			    SET_WARN_COLOR(COLOR_WHITE);
			    warnf(TEXT("Processing %s[%d]..."), *Terrain->GetPathName(), CachedMatIndex);
			    CLEAR_WARN_COLOR();
    
			    // Compile the material...
			    FStaticParameterSet EmptySet(TMatRes->GetId());
			    TRefCountPtr<FMaterialShaderMap> MaterialShaderMapRef;
			    if (!TMatRes->Compile(&EmptySet, ShaderPlatform, MaterialShaderMapRef, FALSE))
			    {
				    // handle errors
				    warnf(NAME_Warning, TEXT("Terrain material failed to be compiled:"));
				    const TArray<FString>& CompileErrors = TMatRes->GetCompileErrors();
				    for(INT ErrorIndex = 0; ErrorIndex < CompileErrors.Num(); ErrorIndex++)
				    {
					    warnf(NAME_Warning, TEXT("%s"), *CompileErrors(ErrorIndex));
				    }
			    }
		    }
	    }
	}
	TerrainProcessingTime += appSeconds() - TerrainProcessingStart;
}

INT UPrecompileShadersCommandlet::Main(const FString& Params)
{
#if !SHIPPING_PC_GAME
	// in a shipping game, don't bother checking for another instance, won't find it

	if( ( !GIsFirstInstance ) && ( ParseParam(appCmdLine(),TEXT("ALLOW_PARALLEL_PRECOMPILESHADERS")) == FALSE ) )
	{
		SET_WARN_COLOR_AND_BACKGROUND(COLOR_RED, COLOR_WHITE);
		warnf(TEXT(""));
		warnf(TEXT("ANOTHER INSTANCE IS RUNNING, THIS WILL NOT BE ABLE TO SAVE SHADER CACHE... QUITTING!"));
		CLEAR_WARN_COLOR();
		return 1;
	}
#endif

	// get some parameter
	FString Platform;
	if (!Parse(*Params, TEXT("PLATFORM="), Platform))
	{
		SET_WARN_COLOR(COLOR_YELLOW);
		warnf(TEXT("Usage: PrecompileShaders platform=<platform> [package=<package> [-depends]]\n   Platform is ps3, xenon, allpc, pc_sm2, pc_sm3, pc_sm4\n   Specifying no package will look in all packages\n   -depends will also look in dependencies of the package"));
		CLEAR_WARN_COLOR();
		return 1;
	}

	UBOOL bProcessDependencies = FALSE;

	// are we making the ref shader cache?
	UBOOL bSaveReferenceShaderCache = ParseParam(*Params, TEXT("refcache"));

	// Whether we should skip maps or not.
	UBOOL bShouldSkipMaps = ParseParam(*Params, TEXT("skipmaps"));

	// Determine the platform DLL and shader platform specified by the command-line platform ID.
	const UBOOL bCompileForAllPlatforms = (Platform == TEXT("ALL"));
	if(bCompileForAllPlatforms || Platform == TEXT("PS3"))
	{
		ShaderPlatforms.AddItem(SP_PS3);
	}
	if(bCompileForAllPlatforms || Platform == TEXT("xenon") || Platform == TEXT("xbox360"))
	{	
		ShaderPlatforms.AddItem(SP_XBOXD3D);
	}
	if(bCompileForAllPlatforms || Platform == TEXT("allpc") || Platform == TEXT("pc") || Platform == TEXT("pc_sm3"))
	{
		ShaderPlatforms.AddItem(SP_PCD3D_SM3);
	}
	if (bCompileForAllPlatforms || Platform == TEXT("allpc") || Platform == TEXT("pc_sm2"))
	{
		ShaderPlatforms.AddItem(SP_PCD3D_SM2);
	}
	if (bCompileForAllPlatforms || Platform == TEXT("allpc") || Platform == TEXT("pc_sm4"))
	{
		ShaderPlatforms.AddItem(SP_PCD3D_SM4);
	}

	// Find the target platform PC-side support implementations.
	static const TCHAR* ShaderPlatformToPlatformNameMap[] =
	{
		TEXT("PC"),		// SP_PCD3D_SM3
		TEXT("PS3"),	// SP_PS3
		TEXT("Xenon"),	// SP_XBOXD3D
		TEXT("PC"),		// SP_PCD3D_SM2
		TEXT("PC"),		// SP_PCD3D_SM4
	};
	check(ARRAY_COUNT(ShaderPlatformToPlatformNameMap) == SP_NumPlatforms);
	for(INT PlatformIndex = 0;PlatformIndex < ShaderPlatforms.Num();PlatformIndex++)
	{
		const EShaderPlatform ShaderPlatform = ShaderPlatforms(PlatformIndex);
		const TCHAR* ConsolePlatform = ShaderPlatformToPlatformNameMap[ShaderPlatform];
		FConsoleSupport* ConsoleSupport = FConsoleSupportContainer::GetConsoleSupportContainer()->GetConsoleSupport(ConsolePlatform);
		if(!ConsoleSupport)
		{
			SET_WARN_COLOR(COLOR_RED);
			warnf(NAME_Error, TEXT("Can't load the platform DLL for %s - shaders won't be compiled for that platform!"), ConsolePlatform);
			CLEAR_WARN_COLOR();

			ShaderPlatforms.Remove(PlatformIndex--);
		}
		else
		{
			warnf(TEXT("Compiling shaders for %s"),ShaderPlatformToText(ShaderPlatform));

			// Create the shader precompiler for the target platform.
			check(!GConsoleShaderPrecompilers[ShaderPlatform]);
			if( ShaderPlatform != SP_PCD3D_SM4 &&
				ShaderPlatform != SP_PCD3D_SM3 &&
				ShaderPlatform != SP_PCD3D_SM2)
			{
				GConsoleShaderPrecompilers[ShaderPlatform] = ConsoleSupport->GetGlobalShaderPrecompiler();
			}
		}
	}

	if(!ShaderPlatforms.Num())
	{
		SET_WARN_COLOR(COLOR_RED);
		warnf(NAME_Error, TEXT("No valid platforms specified. Run with no parameters for list of known platforms."));
		CLEAR_WARN_COLOR();
		return 1;
	}

	DOUBLE StartProcessingTime = appSeconds();
	DOUBLE PackageLoopSetupStartTime = appSeconds();

	// figure out what packages to iterate over
	TArray<FString> PackageList;
	FString SinglePackage;
	if (Parse(*Params, TEXT("PACKAGE="), SinglePackage))
	{
		FString PackagePath;
		if (!GPackageFileCache->FindPackageFile(*SinglePackage, NULL, PackagePath))
		{
			SET_WARN_COLOR(COLOR_RED);
			warnf(NAME_Error, TEXT("Failed to find file %s"), *SinglePackage);
			CLEAR_WARN_COLOR();
			return 1;
		}
		else
		{
			PackageList.AddItem(PackagePath);

			bProcessDependencies = ParseParam(*Params, TEXT("depends"));
		}
	}
	else
	{
		PackageList = GPackageFileCache->GetPackageFileList();
	}

	FString ForceName;
	Parse(*Params, TEXT("FORCE="), ForceName);

	// make sure we have all global shaders for the platform
	for(INT PlatformIndex = 0;PlatformIndex < ShaderPlatforms.Num();PlatformIndex++)
	{
		const EShaderPlatform ShaderPlatform = ShaderPlatforms(PlatformIndex);
		VerifyGlobalShaders(ShaderPlatform);
	}

	FLOAT PackageLoopSetupTime = appSeconds() - PackageLoopSetupStartTime;

	FLOAT PackageLoadingTime = 0;
	FLOAT ObjectLoadingTime = 0;
	FLOAT TotalProcessingTime = 0;
	FLOAT GCTime = 0;
	FLOAT PackagePrologueTime = 0;
	FLOAT DependencyProcessingTime = 0;
	FLOAT ShaderCacheSavingTime = 0;
	UINT NumFastPackages = 0;
	UINT NumFullyLoadedPackages = 0;

	TMap<INT, DOUBLE> PackageTimes;
	DOUBLE PackageStartTime;
	DOUBLE PackageTotalTime = 0;

	for (INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++)
	{
		SET_WARN_COLOR(COLOR_DARK_GREEN);
		warnf(TEXT("Starting package %d of %d..."), PackageIndex + 1, PackageList.Num());
		CLEAR_WARN_COLOR();

		PackageStartTime = appSeconds();

		DOUBLE PackagePrologueTimeStart = appSeconds();

		// Skip maps with default map extension if -skipmaps command line option is used.
		if( bShouldSkipMaps && FFilename(PackageList(PackageIndex)).GetExtension() == FURL::DefaultMapExt )
		{
			SET_WARN_COLOR(COLOR_DARK_GREEN);
			warnf(TEXT("Skipping map %s..."), *PackageList(PackageIndex));
			CLEAR_WARN_COLOR();
			continue;
		}

		if (PackageList(PackageIndex). InStr(TEXT("ShaderCache")) != -1)
		{
			SET_WARN_COLOR(COLOR_DARK_GREEN);
			warnf(TEXT("Skipping shader cache %s..."), *PackageList(PackageIndex));
			CLEAR_WARN_COLOR();
			continue;
		}

		SET_WARN_COLOR(COLOR_GREEN);
		warnf(TEXT("Loading %s..."), *PackageList(PackageIndex));
		CLEAR_WARN_COLOR();

		ULinkerLoad* PackageLinker = NULL;
		UBOOL bFullyLoadPackage = FALSE;

		// Check if there are any terrain actors in the package.  
		// If there are, we need to load the whole package to workaround LoadObject<ATerrain>() failing.
		if (!bProcessDependencies)
		{
			// Load the package's linker.
			UObject::BeginLoad();
			PackageLinker = UObject::GetPackageLinker(NULL,*PackageList(PackageIndex),LOAD_None,NULL,NULL);
			UObject::EndLoad();

			if(PackageLinker)
			{
				for(INT ExportIndex = 0;ExportIndex < PackageLinker->ExportMap.Num();ExportIndex++)
				{
					if(PackageLinker->GetExportClassName(ExportIndex) == ATerrain::StaticClass()->GetFName())
					{
						// There is at least one terrain actor in the package, the whole package needs to be loaded.
						bFullyLoadPackage = TRUE;
						break;
					}
				}
			}
		}

		PackagePrologueTime += appSeconds() - PackagePrologueTimeStart;

		if (bProcessDependencies || bFullyLoadPackage)
		{
			NumFullyLoadedPackages++;
			// If we're processing dependencies, we need to load the package to determine its dependencies.

			DOUBLE StartLoadPackageTime = appSeconds();
			// Load the package.
			UPackage* Package = Cast<UPackage>(UObject::LoadPackage( NULL, *PackageList(PackageIndex), LOAD_None ));
			PackageLoadingTime += appSeconds() - StartLoadPackageTime;

			// go over all the materials
			for (TObjectIterator<UMaterial> It; It; ++It)
			{
				UMaterial* Material = *It;

				// if we are loading a single package, and we want to process its dependencies, then don't restrict by package
				if (bProcessDependencies || Material->IsIn(Package))
				{
					ProcessMaterial(Material);
				}
			}

			// go over all the material instances with static parameters
			for (TObjectIterator<UMaterialInstance> It; It; ++It)
			{
				UMaterialInstance* MaterialInstance = *It;
				// if we are loading a single package, and we want to process its dependencies, then don't restrict by package
				if (bProcessDependencies || MaterialInstance->IsIn(Package))
				{
					ProcessMaterialInstance(MaterialInstance);
				}
			}

			for (TObjectIterator<ATerrain> It; It; ++It)
			{
				ATerrain* Terrain = *It;
				if (Terrain && Terrain->IsIn(Package))
				{
					ProcessTerrain(Terrain);
				}
			}
		}
		else if (PackageLinker)
		{
			NumFastPackages++;
			// If we don't need to process dependencies, we can skip loading the entire package and only load the relevant objects.
			for(INT ExportIndex = 0;ExportIndex < PackageLinker->ExportMap.Num();ExportIndex++)
			{
				FName ClassName = PackageLinker->GetExportClassName(ExportIndex);
				FName ClassPackageName = PackageLinker->GetExportClassPackage(ExportIndex);

				DOUBLE StartLoadObjectTime = appSeconds();
				// Load the object's class so we can do a robust comparison, handling inheritance.
				UClass* ObjectClass = LoadObject<UClass>(NULL,*(ClassPackageName.ToString() + "." + ClassName.ToString()),NULL,LOAD_None,NULL);
				ObjectLoadingTime += appSeconds() - StartLoadObjectTime;

				if (ObjectClass)
				{
					if (ObjectClass->IsChildOf(UMaterial::StaticClass()))
					{
						StartLoadObjectTime = appSeconds();
						UMaterial* Material = LoadObject<UMaterial>(NULL,*PackageLinker->GetExportPathName(ExportIndex),*PackageList(PackageIndex),LOAD_None,NULL);
						ObjectLoadingTime += appSeconds() - StartLoadObjectTime;

						if(Material)
						{
							ProcessMaterial(Material);
						}
					}
					else if (ObjectClass->IsChildOf(UMaterialInstance::StaticClass()))
					{
						StartLoadObjectTime = appSeconds();
						UMaterialInstance* MaterialInstance = LoadObject<UMaterialInstance>(NULL,*PackageLinker->GetExportPathName(ExportIndex),*PackageList(PackageIndex),LOAD_None,NULL);
						ObjectLoadingTime += appSeconds() - StartLoadObjectTime;

						if(MaterialInstance)
						{
							ProcessMaterialInstance(MaterialInstance);
						}
					}
				}

				if(ClassName == ATerrain::StaticClass()->GetFName())
				{
					appErrorf(TEXT("Terrain actor found when trying to use the fast path!"));
				}
			}
		}

		// Only garbage collect every 10 packages to balance a reduction in
		// time spent doing it and memory cost of not doing it.
		if (((PackageIndex + 1) % 10) == 0)
		{
			DOUBLE StartGCTime = appSeconds();
			// close the package
			UObject::CollectGarbage(RF_Native);
			GCTime += appSeconds() - StartGCTime;
		}

		DOUBLE CurrentPackageTime = appSeconds() - PackageStartTime;
		PackageTimes.Set(PackageIndex, CurrentPackageTime);
		PackageTotalTime += CurrentPackageTime;
	}

	// Finish GC'ing for any packages not GC'd during the loop above
	DOUBLE StartGCTime = appSeconds();
	// close the package
	UObject::CollectGarbage(RF_Native);
	GCTime += appSeconds() - StartGCTime;

	DOUBLE StartCacheSavingTime = appSeconds();
	// save out the local shader caches
	SaveLocalShaderCaches();
	ShaderCacheSavingTime += appSeconds() - StartCacheSavingTime;

	TotalProcessingTime += appSeconds() - StartProcessingTime;
	extern DOUBLE GRHIShaderCompileTime_Total;
	extern DOUBLE GRHIShaderCompileTime_PS3;
	extern DOUBLE GRHIShaderCompileTime_XBOXD3D;
	extern DOUBLE GRHIShaderCompileTime_PCD3D_SM2;
	extern DOUBLE GRHIShaderCompileTime_PCD3D_SM3;
	extern DOUBLE GRHIShaderCompileTime_PCD3D_SM4;
	const FLOAT BreakdownTimeTotal = MaterialProcessingTime
		                           + MaterialInstanceProcessingTime
								   + TerrainProcessingTime
								   + PackageLoadingTime
								   + ObjectLoadingTime
								   + GCTime
								   + ShaderCacheSavingTime
								   + PackageLoopSetupTime
								   + PackagePrologueTime;

	warnf(TEXT("Package processing complete."));
	warnf(TEXT("	TotalProcessingTime      = %.2f min"), TotalProcessingTime / 60.0f);
	warnf(TEXT("	Material                 = %.2f min"), MaterialProcessingTime / 60.0f);
	warnf(TEXT("	Material Instance        = %.2f min"), MaterialInstanceProcessingTime / 60.0f);
	warnf(TEXT("	Terrain                  = %.2f min"), TerrainProcessingTime / 60.0f);
	warnf(TEXT("	PackageLoopSetupTime     = %.2f min"), PackageLoopSetupTime / 60.0f);
	warnf(TEXT("	PackagePrologueTime      = %.2f min"), PackagePrologueTime / 60.0f);
	warnf(TEXT("	PackageLoadingTime       = %.2f min"), PackageLoadingTime / 60.0f);
	warnf(TEXT("	  NumFullyLoadedPackages = %u"), NumFullyLoadedPackages);
	warnf(TEXT("	  NumFastPackages        = %u"), NumFastPackages);
	warnf(TEXT("	ObjectLoadingTime        = %.2f min"), ObjectLoadingTime / 60.0f);
	warnf(TEXT("	GCTime                   = %.2f min"), GCTime / 60.0f);
	warnf(TEXT("	ShaderCacheSavingTime    = %.2f min"), ShaderCacheSavingTime / 60.0f);
	warnf(TEXT("	RHIShaderCompileTime     = %.2f min"), GRHIShaderCompileTime_Total / 60.0f);
	warnf(TEXT("	  PS3                    = %.2f min"), GRHIShaderCompileTime_PS3 / 60.0f);
	warnf(TEXT("	  XBOXD3D                = %.2f min"), GRHIShaderCompileTime_XBOXD3D / 60.0f);
	warnf(TEXT("	  PCD3D_SM2              = %.2f min"), GRHIShaderCompileTime_PCD3D_SM2 / 60.0f);
	warnf(TEXT("	  PCD3D_SM3              = %.2f min"), GRHIShaderCompileTime_PCD3D_SM3 / 60.0f);
	warnf(TEXT("	  PCD3D_SM4              = %.2f min"), GRHIShaderCompileTime_PCD3D_SM4 / 60.0f);
	warnf(TEXT("	UnaccountedTime          = %.2f min"), (TotalProcessingTime - BreakdownTimeTotal) / 60.0f);

	// Compute the standard deviation for the package times
	DOUBLE AllPackagesMean = PackageTotalTime / PackageTimes.Num();
	DOUBLE AllPackagesDifferenceSquaredSum = 0;
	DOUBLE PackageMeanDifference;
	for (TMap<INT, DOUBLE>::TConstIterator PackageTimesIt(PackageTimes); PackageTimesIt; ++PackageTimesIt)
	{
		PackageMeanDifference = PackageTimesIt.Value() - AllPackagesMean;
		AllPackagesDifferenceSquaredSum += (PackageMeanDifference * PackageMeanDifference);
	}
	DOUBLE AllPackagesStandardDeviation = sqrt(AllPackagesDifferenceSquaredSum / PackageTimes.Num());

	// Warn about any packages with times greater than a couple standard deviations out
	SET_WARN_COLOR(COLOR_YELLOW);
	DOUBLE WarningTimeLimit = AllPackagesMean + (3 * AllPackagesStandardDeviation);
	warnf(TEXT("\n"));
	for (TMap<INT, DOUBLE>::TConstIterator PackageTimesIt(PackageTimes); PackageTimesIt; ++PackageTimesIt)
	{
		if (PackageTimesIt.Value() > WarningTimeLimit)
		{
			warnf(TEXT("Warning: Package time greater than 3 standard deviations!"));
			warnf(TEXT("    Name = %s"), *PackageList(PackageTimesIt.Key()));
			warnf(TEXT("    Time = %.2f min"), PackageTimesIt.Value() / 60.0f);
			warnf(TEXT("\n"));
		}
	}
	CLEAR_WARN_COLOR();

	// Optionally dump all of the individual package times
	debugf(NAME_DevShadersDetailed, TEXT("\nPackage Times:"));
	for (TMap<INT, DOUBLE>::TConstIterator PackageTimesIt(PackageTimes); PackageTimesIt; ++PackageTimesIt)
	{
		debugf(NAME_DevShadersDetailed, TEXT("	%s"), *PackageList(PackageTimesIt.Key()));
		debugf(NAME_DevShadersDetailed, TEXT("	                         = %.2f min"), PackageTimesIt.Value() / 60.0f);
	}

	// Reset the shader precompiler reference to NULL;
	for(INT PlatformIndex = 0;PlatformIndex < ShaderPlatforms.Num();PlatformIndex++)
	{
		const EShaderPlatform ShaderPlatform = ShaderPlatforms(PlatformIndex);

		// destroy the precompiler objects
		GConsoleShaderPrecompilers[ShaderPlatform] = NULL;
	}

	// save out the local shader caches
	SaveLocalShaderCaches();

	// if we're saving the ref shader cache, save it now
	if (bSaveReferenceShaderCache)
	{
		for(INT PlatformIndex = 0;PlatformIndex < ShaderPlatforms.Num();PlatformIndex++)
		{
			const EShaderPlatform ShaderPlatform = ShaderPlatforms(PlatformIndex);

			// mark it as dirty so it will save
			GetLocalShaderCache(ShaderPlatform)->MarkDirty();

			// save it as the reference package
			SaveLocalShaderCache(ShaderPlatform,*GetReferenceShaderCacheFilename(ShaderPlatform));
		}
	}

	return 0;
}
IMPLEMENT_CLASS(UPrecompileShadersCommandlet)

/*-----------------------------------------------------------------------------
	UDumpShadersCommandlet implementation.
-----------------------------------------------------------------------------*/

INT UDumpShadersCommandlet::Main(const FString& Params)
{
	// get some parameter
	FString Platform;
	if (!Parse(*Params, TEXT("PLATFORM="), Platform))
	{
		warnf(NAME_Warning, TEXT("Usage: DumpShaders platform=<platform> [globalshader=<shadertype>] [material=<materialname>]\n Platform is ps3, xenon, allpc, pc_sm2, pc_sm3, pc_sm4"));
		return 1;
	}

	// Determine the platform DLL and shader platform specified by the command-line platform ID.
	const UBOOL bCompileForAllPlatforms = (Platform == TEXT("ALL"));
	if(bCompileForAllPlatforms || Platform == TEXT("PS3"))
	{
		ShaderPlatforms.AddItem(SP_PS3);
	}
	if(bCompileForAllPlatforms || Platform == TEXT("xenon") || Platform == TEXT("xbox360"))
	{	
		ShaderPlatforms.AddItem(SP_XBOXD3D);
	}
	if(bCompileForAllPlatforms || Platform == TEXT("allpc") || Platform == TEXT("pc") || Platform == TEXT("pc_sm3"))
	{
		ShaderPlatforms.AddItem(SP_PCD3D_SM3);
	}
	if (bCompileForAllPlatforms || Platform == TEXT("allpc") || Platform == TEXT("pc_sm2"))
	{
		ShaderPlatforms.AddItem(SP_PCD3D_SM2);
	}
	if (bCompileForAllPlatforms || Platform == TEXT("allpc") || Platform == TEXT("pc_sm4"))
	{
		ShaderPlatforms.AddItem(SP_PCD3D_SM4);
	}

	// Find the target platform PC-side support implementations.
	static const TCHAR* ShaderPlatformToPlatformNameMap[] =
	{
		TEXT("PC"),		// SP_PCD3D_SM3
		TEXT("PS3"),	// SP_PS3
		TEXT("Xenon"),	// SP_XBOXD3D
		TEXT("PC"),		// SP_PCD3D_SM2
		TEXT("PC"),		// SP_PCD3D_SM4
	};
	check(ARRAY_COUNT(ShaderPlatformToPlatformNameMap) == SP_NumPlatforms);
	for(INT PlatformIndex = 0;PlatformIndex < ShaderPlatforms.Num();PlatformIndex++)
	{
		const EShaderPlatform ShaderPlatform = ShaderPlatforms(PlatformIndex);
		const TCHAR* ConsolePlatform = ShaderPlatformToPlatformNameMap[ShaderPlatform];
		FConsoleSupport* ConsoleSupport = FConsoleSupportContainer::GetConsoleSupportContainer()->GetConsoleSupport(ConsolePlatform);
		if(!ConsoleSupport)
		{
			warnf(NAME_Error, TEXT("Can't load the platform DLL for %s - shaders won't be compiled for that platform!"), ConsolePlatform);
			ShaderPlatforms.Remove(PlatformIndex--);
		}
		else
		{
			warnf(TEXT("Compiling shaders for %s"),ShaderPlatformToText(ShaderPlatform));

			// Create the shader precompiler for the target platform.
			check(!GConsoleShaderPrecompilers[ShaderPlatform]);
			if( ShaderPlatform != SP_PCD3D_SM4 &&
				ShaderPlatform != SP_PCD3D_SM3 &&
				ShaderPlatform != SP_PCD3D_SM2)
			{
				GConsoleShaderPrecompilers[ShaderPlatform] = ConsoleSupport->GetGlobalShaderPrecompiler();
			}
		}
	}

	if(!ShaderPlatforms.Num())
	{
		warnf(NAME_Error, TEXT("No valid platforms specified. Run with no parameters for list of known platforms."));
		return 1;
	}

	FString GlobalShaderTypeName;
	UBOOL bOperatingOnGlobalShader = FALSE;
	UBOOL bFoundGlobalShader = FALSE;
	if (Parse(*Params, TEXT("GLOBALSHADER="), GlobalShaderTypeName))
	{
		bOperatingOnGlobalShader = TRUE;
	}

	FString MaterialName;
	Parse(*Params, TEXT("MATERIAL="), MaterialName);

	if (!bOperatingOnGlobalShader && MaterialName.Len() == 0)
	{
		warnf(NAME_Error, TEXT("Missing global shader or material to operate on.  Run without any parameters to see usage."));
		return 1;
	}

	if (bOperatingOnGlobalShader)
	{
		for (INT PlatformIndex = 0;PlatformIndex < ShaderPlatforms.Num();PlatformIndex++)
		{
			const EShaderPlatform ShaderPlatform = ShaderPlatforms(PlatformIndex);

			// search for the specified global shader
			TShaderMap<FGlobalShaderType>* GlobalShaderMap = GetGlobalShaderMap(ShaderPlatform);
			for (TLinkedList<FShaderType*>::TIterator ShaderTypeIt(FShaderType::GetTypeList());ShaderTypeIt;ShaderTypeIt.Next())
			{
				FGlobalShaderType* GlobalShaderType = ShaderTypeIt->GetGlobalShaderType();
				if (GlobalShaderType != NULL && GlobalShaderType->ShouldCache(ShaderPlatform))
				{
					FString TypeName = FString(GlobalShaderType->GetName());
					if (TypeName.ToUpper().InStr(GlobalShaderTypeName.ToUpper()) != -1)
					{
						// preprocess this global shader type
						TArray<FString> ShaderErrors;
						FShader* Shader = GlobalShaderType->CompileShader(ShaderPlatform,ShaderErrors,TRUE);
						if (Shader == NULL)
						{
							for (INT ErrorIndex = 0; ErrorIndex < ShaderErrors.Num(); ErrorIndex++)
							{
								warnf(TEXT("Error: %s\n"), *ShaderErrors(ErrorIndex));
							}
							warnf(TEXT("Failed to compile global shader: %s"), *TypeName);
						}
						bFoundGlobalShader = TRUE;
					}
				}
			}
		}
	}

	if (bOperatingOnGlobalShader)
	{
		if (bFoundGlobalShader)
		{
			SET_WARN_COLOR(COLOR_GREEN);
			warnf(TEXT("Preprocessed all global shaders matching: %s.\nDone."), *GlobalShaderTypeName);
			CLEAR_WARN_COLOR();
			return 0;
		}
		else
		{
			warnf(NAME_Error, TEXT("Could not find a global shader matching: %s.\nDone."), *GlobalShaderTypeName);
			return 1;
		}
	}

	UBOOL bMaterialDone = FALSE;

	// figure out what packages to iterate over
	FString SinglePackage;
	TArray<FString> PackageList;
	INT PackageEndIndex = MaterialName.InStr(TEXT("."));
	if (PackageEndIndex != INDEX_NONE)
	{
		SinglePackage = MaterialName.Left(PackageEndIndex);
		FString PackagePath;
		if (!GPackageFileCache->FindPackageFile(*SinglePackage, NULL, PackagePath))
		{
			warnf(NAME_Error, TEXT("Failed to find specified package %s"), *SinglePackage);
			return 1;
		}
		else
		{
			check(MaterialName.Len() > PackageEndIndex);
			MaterialName = MaterialName.Right(MaterialName.Len() - (PackageEndIndex + 1));
			PackageList.AddItem(PackagePath);
		}
	}
	else
	{
		warnf(NAME_Warning, TEXT("No package specified, searching through all packages which may be slow."), *SinglePackage);
		PackageList = GPackageFileCache->GetPackageFileList();
	}

	for (INT PackageIndex = 0; PackageIndex < PackageList.Num() && !bMaterialDone; PackageIndex++)
	{
		if (PackageList(PackageIndex). InStr(TEXT("ShaderCache")) != -1)
		{
			SET_WARN_COLOR(COLOR_DARK_GREEN);
			warnf(TEXT("Skipping shader cache %s..."), *PackageList(PackageIndex));
			CLEAR_WARN_COLOR();
			continue;
		}

		SET_WARN_COLOR(COLOR_GREEN);
		warnf(TEXT("Loading %s..."), *PackageList(PackageIndex));
		CLEAR_WARN_COLOR();

		// Load the package's linker.
		UObject::BeginLoad();
		ULinkerLoad* PackageLinker = UObject::GetPackageLinker(NULL,*PackageList(PackageIndex),LOAD_None,NULL,NULL);
		UObject::EndLoad();

		if (PackageLinker)
		{
			// If we don't need to process dependencies, we can skip loading the entire package and only load the relevant objects.
			for(INT ExportIndex = 0;ExportIndex < PackageLinker->ExportMap.Num();ExportIndex++)
			{
				FName ClassName = PackageLinker->GetExportClassName(ExportIndex);
				FName ClassPackageName = PackageLinker->GetExportClassPackage(ExportIndex);

				DOUBLE StartLoadObjectTime = appSeconds();
				// Load the object's class so we can do a robust comparison, handling inheritance.
				UClass* ObjectClass = LoadObject<UClass>(NULL,*(ClassPackageName.ToString() + "." + ClassName.ToString()),NULL,LOAD_None,NULL);

				if (ObjectClass)
				{
					if (ObjectClass->IsChildOf(UMaterialInterface::StaticClass()))
					{
						const FString ExportPathName = PackageLinker->GetExportPathName(ExportIndex);
						// Check if this is the material we are looking for
						if (ExportPathName.ToUpper().InStr(MaterialName.ToUpper()) != INDEX_NONE)
						{
							UMaterialInterface* MaterialInterface = LoadObject<UMaterialInterface>(NULL,*ExportPathName,*PackageList(PackageIndex),LOAD_None,NULL);
							if (MaterialInterface)
							{
								if(MaterialInterface->IsTemplate())
								{
									// Material templates don't have shaders.
									continue;
								}

								SET_WARN_COLOR(COLOR_WHITE);
								warnf(TEXT("Processing %s..."), *MaterialInterface->GetPathName());
								CLEAR_WARN_COLOR();

								UMaterial* BaseMaterial = Cast<UMaterial>(MaterialInterface);
								UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(MaterialInterface);
								if (BaseMaterial)
								{
									// compile for the platform!
									for(INT PlatformIndex = 0;PlatformIndex < ShaderPlatforms.Num();PlatformIndex++)
									{
										const EShaderPlatform ShaderPlatform = ShaderPlatforms(PlatformIndex);
										TRefCountPtr<FMaterialShaderMap> MapRef;
										const EMaterialShaderPlatform MaterialPlatform = GetMaterialPlatform(ShaderPlatform);
										FMaterialResource * MaterialResource = BaseMaterial->GetMaterialResource(MaterialPlatform);
										if (MaterialResource == NULL)
										{
											SET_WARN_COLOR(COLOR_YELLOW);
											warnf(NAME_Warning, TEXT("%s has a NULL MaterialResource"), *BaseMaterial->GetFullName());
											CLEAR_WARN_COLOR();
											continue;
										}

										FStaticParameterSet EmptySet(MaterialResource->GetId());
										if (!MaterialResource->Compile(&EmptySet, ShaderPlatform, MapRef, TRUE, FALSE, TRUE))
										{
											// handle errors
											warnf(NAME_Warning, TEXT("Material failed to be compiled:"));
											const TArray<FString>& CompileErrors = MaterialResource->GetCompileErrors();
											for(INT ErrorIndex = 0; ErrorIndex < CompileErrors.Num(); ErrorIndex++)
											{
												warnf(NAME_Warning, TEXT("%s"), *CompileErrors(ErrorIndex));
											}
										}
									}
									bMaterialDone = TRUE;
									break;
								}
								else if (MaterialInstance)
								{
									for(INT PlatformIndex = 0;PlatformIndex < ShaderPlatforms.Num();PlatformIndex++)
									{
										const EShaderPlatform ShaderPlatform = ShaderPlatforms(PlatformIndex);
										const EMaterialShaderPlatform RequestedMaterialPlatform = GetMaterialPlatform(ShaderPlatform);

										// compile the material instance's shaders for the platform
										MaterialInstance->CacheResourceShaders(ShaderPlatform, FALSE, FALSE, TRUE);

										const EMaterialShaderPlatform MaterialPlatform = GetMaterialPlatform(ShaderPlatform);
										FMaterialResource* MaterialResource = MaterialInstance->GetMaterialResource(MaterialPlatform);
										TRefCountPtr<FMaterialShaderMap> MaterialShaderMapRef = MaterialResource ? MaterialResource->GetShaderMap() : NULL;
										if (!MaterialShaderMapRef)
										{
											// handle errors
											warnf(NAME_Warning, TEXT("Failed to compile Material Instance:"));
											const TArray<FString>& CompileErrors = MaterialResource->GetCompileErrors();
											for(INT ErrorIndex = 0; ErrorIndex < CompileErrors.Num(); ErrorIndex++)
											{
												warnf(NAME_Warning, TEXT("%s"), *CompileErrors(ErrorIndex));
											}
										}
									}
									bMaterialDone = TRUE;
									break;
								}
							}
						}
					}
				}
			}
		}

		// close the package
		UObject::CollectGarbage(RF_Native);
	}

	if (MaterialName.Len() && !bMaterialDone)
	{
		warnf(TEXT("Material named %s not found"), *MaterialName );
	}

	// Reset the shader precompiler reference to NULL;
	for(INT PlatformIndex = 0;PlatformIndex < ShaderPlatforms.Num();PlatformIndex++)
	{
		const EShaderPlatform ShaderPlatform = ShaderPlatforms(PlatformIndex);

		// destroy the precompiler objects
		GConsoleShaderPrecompilers[ShaderPlatform] = NULL;
	}

	return 0;
}
IMPLEMENT_CLASS(UDumpShadersCommandlet)
