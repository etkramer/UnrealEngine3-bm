/*=============================================================================
	MeshMaterialShader.cpp: Mesh material shader implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

/**
* Finds a FMeshMaterialShaderType by name.
*/
FMeshMaterialShaderType* FMeshMaterialShaderType::GetTypeByName(const FString& TypeName)
{
	for(TLinkedList<FShaderType*>::TIterator It(FShaderType::GetTypeList()); It; It.Next())
	{
		FString CurrentTypeName = FString(It->GetName());
		FMeshMaterialShaderType* CurrentType = It->GetMeshMaterialShaderType();
		if (CurrentType && CurrentTypeName == TypeName)
		{
			return CurrentType;
		}
	}
	return NULL;
}

/**
 * Enqueues a compilation for a new shader of this type.
 * @param Platform - The platform to compile for.
 * @param Material - The material to link the shader with.
 * @param MaterialShaderCode - The shader code for the material.
 * @param VertexFactoryType - The vertex factory to compile with.
 */
void FMeshMaterialShaderType::BeginCompileShader(
	EShaderPlatform Platform,
	const FMaterial* Material,
	const TCHAR* MaterialShaderCode,
	FVertexFactoryType* VertexFactoryType
	)
{
	// Construct the shader environment.
	FShaderCompilerEnvironment Environment;

	// apply the vertex factory changes to the compile environment
	VertexFactoryType->ModifyCompilationEnvironment(Platform, Environment);

	FString VertexFactoryFile = LoadShaderSourceFile(VertexFactoryType->GetShaderFilename());
	
	Environment.IncludeFiles.Set(TEXT("Material.usf"),MaterialShaderCode);
	Environment.IncludeFiles.Set(TEXT("VertexFactory.usf"),*VertexFactoryFile);

	switch(Material->GetBlendMode())
	{
	case BLEND_Opaque: Environment.Definitions.Set(TEXT("MATERIALBLENDING_SOLID"),TEXT("1")); break;
	case BLEND_Masked: Environment.Definitions.Set(TEXT("MATERIALBLENDING_MASKED"),TEXT("1")); break;
	case BLEND_Translucent: Environment.Definitions.Set(TEXT("MATERIALBLENDING_TRANSLUCENT"),TEXT("1")); break;
	case BLEND_Additive: Environment.Definitions.Set(TEXT("MATERIALBLENDING_ADDITIVE"),TEXT("1")); break;
	case BLEND_Modulate: Environment.Definitions.Set(TEXT("MATERIALBLENDING_MODULATE"),TEXT("1")); break;
	default: appErrorf(TEXT("Unknown material blend mode: %u"),(INT)Material->GetBlendMode());
	}

	Environment.Definitions.Set(TEXT("MATERIAL_TWOSIDED"),Material->IsTwoSided() ? TEXT("1") : TEXT("0"));

	switch(Material->GetLightingModel())
	{
	case MLM_SHPRT: // For backward compatibility, treat the deprecated SHPRT lighting model as Phong.
	case MLM_Phong: Environment.Definitions.Set(TEXT("MATERIAL_LIGHTINGMODEL_PHONG"),TEXT("1")); break;
	case MLM_NonDirectional: Environment.Definitions.Set(TEXT("MATERIAL_LIGHTINGMODEL_NONDIRECTIONAL"),TEXT("1")); break;
	case MLM_Unlit: Environment.Definitions.Set(TEXT("MATERIAL_LIGHTINGMODEL_UNLIT"),TEXT("1")); break;
	case MLM_Custom: Environment.Definitions.Set(TEXT("MATERIAL_LIGHTINGMODEL_CUSTOM"),TEXT("1")); break;
	default: appErrorf(TEXT("Unknown material lighting model: %u"),(INT)Material->GetLightingModel());
	};

	if( Material->GetTransformsUsed() & UsedCoord_World 
		|| Material->GetTransformsUsed() & UsedCoord_View
		|| Material->GetTransformsUsed() & UsedCoord_Local )
	{
		// only use WORLD_COORDS code if a Transform expression was used by the material
		Environment.Definitions.Set(TEXT("WORLD_COORDS"),TEXT("1"));
	}

	if( Material->GetTransformsUsed() & UsedCoord_WorldPos )
	{
		Environment.Definitions.Set(TEXT("WORLD_POS"),TEXT("1"));
	}

	// decals always need WORLD_COORD usage in order to pass 2x2 matrix for normal transform
	// using the color interpolators used by WORLD_COORDS
	if( Material->IsUsedWithDecals() || Material->IsSpecialEngineMaterial() )
	{
		Environment.Definitions.Set(TEXT("WORLD_COORDS"),TEXT("1"));
	}
	if( Material->IsUsedWithDecals() )
	{
		Environment.Definitions.Set(TEXT("MATERIAL_DECAL"),TEXT("1"));
	}

	if( Material->UsesOneLayerDistortion() )
	{
		Environment.Definitions.Set(TEXT("MATERIAL_ONELAYERDISTORTION"),TEXT("1"));
	}

	if( Material->IsUsedWithGammaCorrection() )
	{
		// only use USE_GAMMA_CORRECTION code when enabled
		Environment.Definitions.Set(TEXT("MATERIAL_USE_GAMMA_CORRECTION"),TEXT("1"));
	}	

	//calculate the CRC for this type and store it in the shader cache.
	//this will be used to determine when shader files have changed.
	DWORD CurrentCRC = GetSourceCRC();
	UShaderCache::SetShaderTypeCRC(this, CurrentCRC, Platform);

	//update material shader stats
	UpdateMaterialShaderCompilingStats(Material);

	warnf(NAME_DevShadersDetailed, TEXT("			%s"), GetName());

	// Enqueue the compile
	FShaderType::BeginCompileShader(VertexFactoryType, Platform, Environment);
}

/**
 * Either creates a new instance of this type or returns an equivalent existing shader.
 * @param Material - The material to link the shader with.
 * @param CurrentJob - Compile job that was enqueued by BeginCompileShader.
 */
FShader* FMeshMaterialShaderType::FinishCompileShader(
	const FMaterial* Material,
	const FShaderCompileJob& CurrentJob)
{
	check(CurrentJob.bSucceeded);
	// Check for shaders with identical compiled code.
	FShader* Shader = FindShaderByOutput(CurrentJob.Output);
	if(!Shader)
	{
		// Create the shader.
		Shader = (*ConstructCompiledRef)(CompiledShaderInitializerType(this,CurrentJob.Output,Material,CurrentJob.VFType));
		CurrentJob.Output.ParameterMap.VerifyBindingsAreComplete(GetName(), (EShaderFrequency)CurrentJob.Output.Target.Frequency);
	}
	return Shader;
}

/**
* Calculates a CRC based on this shader type's source code and includes
*/
DWORD FMeshMaterialShaderType::GetSourceCRC()
{
	DWORD SourceCRC = FShaderType::GetSourceCRC();
	return SourceCRC;
}

/**
 * Enqueues compilation for all shaders for a material and vertex factory type.
 * @param Material - The material to compile shaders for.
 * @param MaterialShaderCode - The shader code for Material.
 * @param VertexFactoryType - The vertex factory type to compile shaders for.
 * @param Platform - The platform to compile for.
 */
void FMeshMaterialShaderMap::BeginCompile(
	const FMaterial* Material,
	const TCHAR* MaterialShaderCode,
	FVertexFactoryType* InVertexFactoryType,
	EShaderPlatform Platform
	)
{
	UINT NumShadersPerVF = 0;
	VertexFactoryType = InVertexFactoryType;

	// Iterate over all mesh material shader types.
	for(TLinkedList<FShaderType*>::TIterator ShaderTypeIt(FShaderType::GetTypeList());ShaderTypeIt;ShaderTypeIt.Next())
	{
		FMeshMaterialShaderType* ShaderType = ShaderTypeIt->GetMeshMaterialShaderType();
		if (ShaderType && 
			VertexFactoryType && 
			ShaderType->ShouldCache(Platform, Material, VertexFactoryType) && 
			Material->ShouldCache(Platform, ShaderType, VertexFactoryType) &&
			VertexFactoryType->ShouldCache(Platform, Material, ShaderType)
			)
		{
			// only compile the shader if we don't already have it
			if (!HasShader(ShaderType))
			{
				NumShadersPerVF++;

			    // Compile this mesh material shader for this material and vertex factory type.
				ShaderType->BeginCompileShader(
					Platform,
					Material,
					MaterialShaderCode,
					VertexFactoryType				
					);
			}
		}
	}

	if (NumShadersPerVF > 0)
	{
		warnf(NAME_DevShadersDetailed, TEXT("		%s - %u shaders"), VertexFactoryType->GetName(), NumShadersPerVF);
	}

	//calculate the CRC for this vertex factory type and store it in the shader cache.
	//this will be used to determine when vertex factory shader files have changed.
	DWORD VertexFactoryCRC = VertexFactoryType->GetSourceCRC();
	UShaderCache::SetVertexFactoryTypeCRC(VertexFactoryType, VertexFactoryCRC, Platform);
}

/**
 * Creates shaders for all of the compile jobs and caches them in this shader map.
 * @param Material - The material to compile shaders for.
 * @param CompilationResults - The compile results that were enqueued by BeginCompile.
 */
void FMeshMaterialShaderMap::FinishCompile(const FMaterial* Material, const TArray<FShaderCompileJob>& CompilationResults)
{
	// Find the matching FMeshMaterialShaderType for each compile job
	for (INT JobIndex = 0; JobIndex < CompilationResults.Num(); JobIndex++)
	{
		const FShaderCompileJob& CurrentJob = CompilationResults(JobIndex);
		if (CurrentJob.VFType == VertexFactoryType)
		{
			for(TLinkedList<FShaderType*>::TIterator ShaderTypeIt(FShaderType::GetTypeList());ShaderTypeIt;ShaderTypeIt.Next())
			{
				FMeshMaterialShaderType* MeshMaterialShaderType = ShaderTypeIt->GetMeshMaterialShaderType();
				if (*ShaderTypeIt == CurrentJob.ShaderType && MeshMaterialShaderType != NULL)
				{
					FShader* Shader = MeshMaterialShaderType->FinishCompileShader(Material, CurrentJob);
					check(Shader);
					AddShader(MeshMaterialShaderType,Shader);
				}
			}
		}
	}
}

UBOOL FMeshMaterialShaderMap::IsComplete(
	const FMeshMaterialShaderMap* MeshShaderMap,
	EShaderPlatform Platform,
	const FMaterial* Material,
	FVertexFactoryType* InVertexFactoryType,
	UBOOL bSilent
	)
{
	UBOOL bIsComplete = TRUE;

	// Iterate over all mesh material shader types.
	for(TLinkedList<FShaderType*>::TIterator ShaderTypeIt(FShaderType::GetTypeList());ShaderTypeIt;ShaderTypeIt.Next())
	{
		FMeshMaterialShaderType* ShaderType = ShaderTypeIt->GetMeshMaterialShaderType();
		if (ShaderType && 
			ShaderType->ShouldCache(Platform, Material, InVertexFactoryType) && 
			Material->ShouldCache(Platform, ShaderType, InVertexFactoryType) &&
			InVertexFactoryType->ShouldCache(Platform, Material, ShaderType) &&
			(!MeshShaderMap || !MeshShaderMap->HasShader(ShaderType))
			)
		{
			if (!bSilent)
			{
				warnf(NAME_DevShaders, TEXT("IsComplete failed for %s, missing %s from %s."), *Material->GetFriendlyName(), ShaderType->GetName(), InVertexFactoryType->GetName());
			}
			bIsComplete = FALSE;
			break;
		}
	}

	return bIsComplete;
}

/**
 * Removes all entries in the cache with exceptions based on a shader type
 * @param ShaderType - The shader type to flush or keep (depending on second param)
 * @param bFlushAllButShaderType - TRUE if all shaders EXCEPT the given type should be flush. FALSE will flush ONLY the given shader type
 */
void FMeshMaterialShaderMap::FlushShadersByShaderType(FShaderType* ShaderType, UBOOL bFlushAllButShaderType)
{
	// flush if flushing all but the given type, go over other shader types and remove them
	if (bFlushAllButShaderType)
	{
		for(TLinkedList<FShaderType*>::TIterator ShaderTypeIt(FShaderType::GetTypeList());ShaderTypeIt;ShaderTypeIt.Next())
		{
			if (ShaderType != *ShaderTypeIt && ShaderTypeIt->GetMeshMaterialShaderType())
			{
				RemoveShaderType(ShaderTypeIt->GetMeshMaterialShaderType());
			}
		}
	}
	// otherwise just remove this type
	else if (ShaderType->GetMeshMaterialShaderType())
	{
		RemoveShaderType(ShaderType->GetMeshMaterialShaderType());
	}
}

FArchive& operator<<(FArchive& Ar,FMeshMaterialShaderMap& S)
{
	S.Serialize(Ar);
	Ar << S.VertexFactoryType;
	if (Ar.IsLoading())
	{
		// Check the required version for the vertex factory type
		// If the package version is less, toss the shaders.
		FVertexFactoryType* VFType = S.GetVertexFactoryType();
		if (VFType && (Ar.Ver() < VFType->GetMinPackageVersion() || Ar.LicenseeVer() < VFType->GetMinLicenseePackageVersion()))
		{
			S.Empty();
		}
	}
	return Ar;
}
