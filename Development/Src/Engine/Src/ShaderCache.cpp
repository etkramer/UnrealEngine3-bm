/*=============================================================================
	ShaderCache.cpp: Shader cache implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ShaderCache.h"

IMPLEMENT_CLASS(UShaderCache);

// local and reference shader cache type IDs.
enum EShaderCacheType
{
	SC_Local				= 0,
	SC_Reference			= 1,

	SC_NumShaderCacheTypes	= 2,
};

/**
 * The global caches for non-global shaders.
 * Not handled by GC and code associating objects is responsible for adding them to the root set.
 */
UShaderCache* GShaderCaches[SC_NumShaderCacheTypes][SP_NumPlatforms];

/** The global shader caches for global shaders. */
FShaderCache* GGlobalShaderCaches[SP_NumPlatforms];

UBOOL GSerializingLocalShaderCache = FALSE;

/** Controls discarding of shaders whose source .usf file has changed since the shader was compiled. */
static UBOOL ShouldReloadChangedShaders()
{
	static UBOOL bReloadChangedShaders = FALSE;
	static UBOOL bInitialized = FALSE;
	if(!bInitialized)
	{
		bInitialized = TRUE;

#if CONSOLE
		// Don't allow automatically recompiling changed shaders when using seek-free loading.
		if(!GUseSeekFreeLoading)
#endif
		{
			// get the option to skip shaders whose source files have changed since they were compiled
			GConfig->GetBool( TEXT("DevOptions.Shaders"), TEXT("AutoReloadChangedShaders"), bReloadChangedShaders, GEngineIni );
		}
	}
	return bReloadChangedShaders;
}

const DWORD* FShaderCache::GetShaderTypeCRC(FShaderType* LookupShaderType, EShaderPlatform Platform)
{
	if(LookupShaderType->GetGlobalShaderType())
	{
		// If the shader is global, add it to the global shader cache's CRC map.
		FShaderCache* ShaderCache = GetGlobalShaderCache(Platform);
		check(ShaderCache);
		return ShaderCache->ShaderTypeCRCMap.Find(LookupShaderType);
	}
	else
	{

		UShaderCache* LocalShaderCache = GetLocalShaderCache(Platform);
		check(LocalShaderCache);
		const DWORD* LocalShaderTypeCRC = LocalShaderCache->ShaderTypeCRCMap.Find(LookupShaderType);
		if (LocalShaderTypeCRC)
		{
			// Return the first one found, so that local shader cache entries will override ref shader cache entries
			return LocalShaderTypeCRC;
		}
		else
		{
			UShaderCache* RefShaderCache = GetReferenceShaderCache(Platform);
			if (RefShaderCache)
			{
				const DWORD* RefShaderTypeCRC = RefShaderCache->ShaderTypeCRCMap.Find(LookupShaderType);
				if (RefShaderTypeCRC)
				{
					return RefShaderTypeCRC;
				}
			}
		}
	}
	return NULL;
}

void FShaderCache::SetShaderTypeCRC(FShaderType* InShaderType, DWORD InCRC, EShaderPlatform Platform)
{
	if(InShaderType->GetGlobalShaderType())
	{
		// If the shader is global, add it to the global shader cache's CRC map.
		FShaderCache* ShaderCache = GetGlobalShaderCache(Platform);
		check(ShaderCache);
		ShaderCache->ShaderTypeCRCMap.Set(InShaderType,InCRC);
	}
	else
	{
		// If the shader isn't global, add it to the local shader cache's CRC map.
		UShaderCache* ShaderCache = GetLocalShaderCache(Platform);
		check(ShaderCache);
		ShaderCache->ShaderTypeCRCMap.Set(InShaderType, InCRC);
	}
}

void FShaderCache::RemoveShaderTypeCRC(FShaderType* InShaderType, EShaderPlatform Platform)
{
	if(InShaderType->GetGlobalShaderType())
	{
		// If the shader is global, add it to the global shader cache's CRC map.
		FShaderCache* ShaderCache = GetGlobalShaderCache(Platform);
		check(ShaderCache);
		ShaderCache->ShaderTypeCRCMap.Remove(InShaderType);
	}
	else
	{
		UShaderCache* LocalShaderCache = GetLocalShaderCache(Platform);
		check(LocalShaderCache);
		LocalShaderCache->ShaderTypeCRCMap.Remove(InShaderType);
		UShaderCache* RefShaderCache = GetReferenceShaderCache(Platform);
		if (RefShaderCache)
		{
			RefShaderCache->ShaderTypeCRCMap.Remove(InShaderType);
		}
	}
}

void FShaderCache::Load(FArchive& Ar)
{
	if(Ar.Ver() >= VER_GLOBAL_SHADER_FILE)
	{
		Ar << Platform;
		Ar << ShaderTypeCRCMap;
	}

	INT NumShaders = 0;
	INT NumLegacyShaders = 0;
	INT NumRedundantShaders = 0;
	TArray<FString> OutdatedShaderTypes;

	Ar << NumShaders;

	// Load the shaders in the cache.
	for( INT ShaderIndex=0; ShaderIndex<NumShaders; ShaderIndex++ )
	{
		// Deserialize the shader type and shader ID.
		FShaderType* ShaderType = NULL;
		FGuid ShaderId;
		Ar << ShaderType << ShaderId;

		// Deserialize the offset of the next shader.
		INT SkipOffset = 0;
		Ar << SkipOffset;

		if(!ShaderType)
		{
			// If the shader type doesn't exist anymore, skip the shader.
			Ar.Seek(SkipOffset);
			NumLegacyShaders++;
		}
		else
		{
			DWORD SavedCRC = 0;
			DWORD CurrentCRC = 0;

			if (ShouldReloadChangedShaders())
			{
				//calculate the source file's CRC to see if it has changed since it was last compiled
				CurrentCRC = ShaderType->GetSourceCRC();
				const DWORD* ShaderTypeCRC = ShaderTypeCRCMap.Find(ShaderType);
				if (ShaderTypeCRC)
				{
					SavedCRC = *ShaderTypeCRC;
				}
			}

			FShader* Shader = ShaderType->FindShaderById(ShaderId);
			if(Shader)
			{
				// If a shader with the same type and ID is already resident, skip this shader.
				Ar.Seek(SkipOffset);
				NumRedundantShaders++;
			}
			else if(ShouldReloadChangedShaders() && SavedCRC != CurrentCRC)
			{
				// If the shader has changed since it was last compiled, skip it.
				Ar.Seek(SkipOffset);
				NumLegacyShaders++;
				if (SavedCRC != 0)
				{
					OutdatedShaderTypes.AddUniqueItem(FString(ShaderType->GetName()));
				}
			}
			else if(Ar.Ver() < ShaderType->GetMinPackageVersion() || Ar.LicenseeVer() < ShaderType->GetMinLicenseePackageVersion())
			{
				// If the shader type's serialization is compatible with the version the shader was saved in, skip it.
				Ar.Seek(SkipOffset);
				NumLegacyShaders++;
			}
			else
			{
				// Create a new instance of the shader type.
				Shader = ShaderType->ConstructForDeserialization();

				// Deserialize the shader into the new instance.
				UBOOL bShaderHasOutdatedParameters = Shader->Serialize(Ar);

				if (bShaderHasOutdatedParameters)
				{
					//remove all references to the shader and delete it since it has outdated parameters.
					ShaderType->DeregisterShader(Shader);
					delete Shader;
				}

				//If this check fails and VER_LATEST_ENGINE was changed locally and the local shader cache was saved with it, then the local shader cache should be deleted.
				//Otherwise, if the shader's serialization was changed (usually shader parameters or vertex factory parameters) 
				//and the shader file was not changed, so the CRC check passed, then the shader's MinPackageVersion needs to be incremented.
				checkf(Ar.Tell() == SkipOffset, 
					TEXT("Deserialized the wrong amount for shader %s!  \n")
					TEXT("	This can happen if VER_LATEST_ENGINE was changed locally, so the local shader cache needs to be deleted, \n")
					TEXT("	or if the shader's serialization was changed without bumping its appropriate version.  Expected archive position %i, got position %i\n"), 
					ShaderType->GetName(), 
					SkipOffset, 
					Ar.Tell()
					);
			}
		}
	}
	
	if (ShouldReloadChangedShaders() && GSerializingLocalShaderCache)
	{
		INT NumOutdatedShaderTypes = OutdatedShaderTypes.Num();
		if (NumOutdatedShaderTypes > 0)
		{
			warnf(NAME_DevShaders, TEXT("Skipped %i outdated FShaderTypes:"), NumOutdatedShaderTypes);
			for (INT TypeIndex = 0; TypeIndex < NumOutdatedShaderTypes; TypeIndex++)
			{	
				warnf(NAME_DevShaders, TEXT("	%s"), *OutdatedShaderTypes(TypeIndex));
			}
		}
	}

	// Log some cache stats.
	if( NumShaders )
	{
		debugf(
			TEXT("... Loaded %u shaders (%u legacy, %u redundant)"),
			NumShaders,
			NumLegacyShaders,
			NumRedundantShaders
			);
	}
}

void FShaderCache::Save(FArchive& Ar,const TMap<FGuid,FShader*>& Shaders)
{
	Ar << Platform;
	Ar << ShaderTypeCRCMap;

	INT NumShaders = Shaders.Num();
	Ar << NumShaders;
	for(TMap<FGuid,FShader*>::TConstIterator ShaderIt(Shaders);ShaderIt;++ShaderIt)
	{
		FShader* Shader = ShaderIt.Value();

		// Serialize the shader type and ID separately, so at load time we can see whether this is a redundant shader without fully
		// deserializing it.
		FShaderType* ShaderType = Shader->GetType();
		FGuid ShaderId = Shader->GetId();
		Ar << ShaderType << ShaderId;

		// Write a placeholder value for the skip offset.
		INT SkipOffset = Ar.Tell();
		Ar << SkipOffset;

		// Serialize the shader.
		Shader->Serialize(Ar);

		// Write the actual offset of the end of the shader data over the placeholder value written above.
		INT EndOffset = Ar.Tell();
		Ar.Seek(SkipOffset);
		Ar << EndOffset;
		Ar.Seek(EndOffset);
	}
}

/**
 * Constructor.
 *
 * @param	InPlatform	Platform this shader cache is for.
 */
UShaderCache::UShaderCache( EShaderPlatform InPlatform )
:	FShaderCache(InPlatform)
,	bDirty(FALSE)
{}

/**
 * Flushes the shader map for a material from the cache.
 * @param StaticParameters - The static parameter set identifying the material
 * @param Platform Platform to flush.
 */
void UShaderCache::FlushId(const FStaticParameterSet& StaticParameters, EShaderPlatform Platform)
{
	UShaderCache* ShaderCache = GShaderCaches[SC_Local][Platform];
	if (ShaderCache)
	{
		// Remove the shaders cached for this material ID from the shader cache.
		ShaderCache->MaterialShaderMap.Remove(StaticParameters);
		// make sure the reference in the map is removed
		ShaderCache->MaterialShaderMap.Shrink();
		ShaderCache->bDirty = TRUE;
	}
}

/**
* Flushes the shader map for a material from all caches.
* @param StaticParameters - The static parameter set identifying the material
*/
void UShaderCache::FlushId(const class FStaticParameterSet& StaticParameters)
{
	for( INT PlatformIndex=0; PlatformIndex<SP_NumPlatforms; PlatformIndex++ )
	{
		UShaderCache::FlushId( StaticParameters, (EShaderPlatform)PlatformIndex );
	}
}

/**
 * Combines OtherCache's shaders with this one.  
 * OtherCache has priority and will overwrite any existing shaders, otherwise they will just be added.
 *
 * @param OtherCache	Shader cache to merge
 */
void UShaderCache::Merge(UShaderCache* OtherCache)
{
	check(OtherCache && Platform == OtherCache->Platform);

	//copy over all the material shader maps, overwriting existing ones with incoming ones when necessary
	for( TMap<FStaticParameterSet,TRefCountPtr<FMaterialShaderMap> >::TIterator MaterialIt(OtherCache->MaterialShaderMap); MaterialIt; ++MaterialIt )
	{
		TRefCountPtr<FMaterialShaderMap>& CurrentMaterialShaderMap = MaterialIt.Value();
		check(CurrentMaterialShaderMap->GetMaterialId() == MaterialIt.Key());
		AddMaterialShaderMap(CurrentMaterialShaderMap);
	}

	for(TLinkedList<FShaderType*>::TIterator It(FShaderType::GetTypeList()); It; It.Next())
	{
		const DWORD* OtherShaderTypeCRC = OtherCache->ShaderTypeCRCMap.Find(*It);
		//if the shader type's CRC was found in the other cache copy it over and overwrite current CRC's
		if (OtherShaderTypeCRC)
		{
			ShaderTypeCRCMap.Set(*It, *OtherShaderTypeCRC);
		}
	}

	for(TLinkedList<FVertexFactoryType*>::TIterator It(FVertexFactoryType::GetTypeList()); It; It.Next())
	{
		const DWORD* OtherShaderTypeCRC = OtherCache->VertexFactoryTypeCRCMap.Find(*It);
		//if the vertex factory type's CRC was found in the other cache copy it over and overwrite current CRC's
		if (OtherShaderTypeCRC)
		{
			VertexFactoryTypeCRCMap.Set(*It, *OtherShaderTypeCRC);
		}
	}
}

/**
 * Adds a material shader map to the cache fragment.
 */
void UShaderCache::AddMaterialShaderMap(FMaterialShaderMap* InMaterialShaderMap)
{
	MaterialShaderMap.Set(InMaterialShaderMap->GetMaterialId(),InMaterialShaderMap);
	bDirty = TRUE;
}

void UShaderCache::FinishDestroy()
{
	for( INT TypeIndex=0; TypeIndex<SC_NumShaderCacheTypes; TypeIndex++ )
	{
		for( INT PlatformIndex=0; PlatformIndex<SP_NumPlatforms; PlatformIndex++ )
		{
			if( GShaderCaches[TypeIndex][PlatformIndex] == this )
			{
				// The shader cache is a root object, but it will still be purged on exit.  Make sure there isn't a dangling reference to it.
				GShaderCaches[TypeIndex][PlatformIndex] = NULL;
			}
		}
	}

	Super::FinishDestroy();
}

/**
 * Finds a CRC stored for the given vertexFactoryType on the given platform
 */
const DWORD* UShaderCache::GetVertexFactoryTypeCRC(FVertexFactoryType* LookupVFType, EShaderPlatform Platform)
{
	for( INT TypeIndex=0; TypeIndex<SC_NumShaderCacheTypes; TypeIndex++ )
	{
		UShaderCache* ShaderCache = GShaderCaches[TypeIndex][Platform];
		if (ShaderCache)
		{
			const DWORD* VFTypeCRC = ShaderCache->VertexFactoryTypeCRCMap.Find(LookupVFType);
			//return the first one found, so that local shader cache entries will override ref shader cache entries
			if (VFTypeCRC)
			{
				return VFTypeCRC;
			}
		}
	}
	return NULL;
}

/**
 * Sets a CRC for the given vertexFactoryType on the given platform
 */
void UShaderCache::SetVertexFactoryTypeCRC(FVertexFactoryType* InVertexFactoryType, DWORD InCRC, EShaderPlatform Platform)
{
	UShaderCache* ShaderCache = GShaderCaches[SC_Local][Platform];
	if (ShaderCache)
	{
		ShaderCache->VertexFactoryTypeCRCMap.Set(InVertexFactoryType, InCRC);
	}
}

/**
 * Removes a FVertexFactoryType from the CRC map of all shader caches of the given platform.
 */
void UShaderCache::RemoveVertexFactoryTypeCRC(FVertexFactoryType* InVertexFactoryType, EShaderPlatform Platform)
{
	for( INT TypeIndex=0; TypeIndex<SC_NumShaderCacheTypes; TypeIndex++ )
	{
		UShaderCache* ShaderCache = GShaderCaches[TypeIndex][Platform];
		if (ShaderCache)
		{
			ShaderCache->VertexFactoryTypeCRCMap.Remove(InVertexFactoryType);
		}
	}
}


/**
 * Copies over CRC entries from the ref cache if necessary,
 * only called on the local shader cache before saving.
 */
void UShaderCache::CompleteCRCMaps()
{
	if (GShaderCaches[SC_Reference][Platform])
	{
		//copy over the shaderType to CRC entries from the ref to the local shader cache
		//this is needed because CRC entries are only generated for the cache that the shader was compiled for
		//but shaders can be put into caches that they were not compiled for, and each shaderType will no longer have a CRC entry in that cache
		for(TLinkedList<FShaderType*>::TIterator It(FShaderType::GetTypeList()); It; It.Next())
		{
			const DWORD* RefShaderTypeCRC = GShaderCaches[SC_Reference][Platform]->ShaderTypeCRCMap.Find(*It);
			const DWORD* LocalShaderTypeCRC = ShaderTypeCRCMap.Find(*It);
			//if the shader type's CRC was found in the ref cache but not in the local, copy it over
			if (RefShaderTypeCRC && !LocalShaderTypeCRC)
			{
				ShaderTypeCRCMap.Set(*It, *RefShaderTypeCRC);
			}
		}

		for(TLinkedList<FVertexFactoryType*>::TIterator It(FVertexFactoryType::GetTypeList()); It; It.Next())
		{
			const DWORD* RefVFTypeCRC = GShaderCaches[SC_Reference][Platform]->VertexFactoryTypeCRCMap.Find(*It);
			const DWORD* LocalVFTypeCRC = VertexFactoryTypeCRCMap.Find(*It);
			//if the vertex factory type's CRC was found in the ref cache but not in the local, copy it over
			if (RefVFTypeCRC && !LocalVFTypeCRC)
			{
				VertexFactoryTypeCRCMap.Set(*It, *RefVFTypeCRC);
			}
		}
	}
}

IMPLEMENT_COMPARE_CONSTREF(FStaticParameterSet,SortMaterialsByStaticParamSet,
{
	for ( INT i = 0; i < 4; i++ )
	{
		if ( A.BaseMaterialId[i] > B.BaseMaterialId[i] )
		{
			return 1;
		}
		else if ( A.BaseMaterialId[i] < B.BaseMaterialId[i] )
		{
			return -1;
		}
	}

	if (A.StaticSwitchParameters.Num() > B.StaticSwitchParameters.Num())
	{
		return 1;
	}
	else if (A.StaticSwitchParameters.Num() < B.StaticSwitchParameters.Num())
	{
		return -1;
	}

	for (INT SwitchIndex = 0; SwitchIndex < A.StaticSwitchParameters.Num(); SwitchIndex++)
	{
		const FStaticSwitchParameter &SwitchA = A.StaticSwitchParameters(SwitchIndex);
		const FStaticSwitchParameter &SwitchB = B.StaticSwitchParameters(SwitchIndex);

		if (SwitchA.ParameterName.ToString() != SwitchB.ParameterName.ToString())
		{
			return (SwitchA.ParameterName.ToString() > SwitchB.ParameterName.ToString()) * 2 - 1;
		} 

		if (SwitchA.Value != SwitchB.Value) { return (SwitchA.Value > SwitchB.Value) * 2 - 1; } 
	}

	if (A.StaticComponentMaskParameters.Num() > B.StaticComponentMaskParameters.Num())
	{
		return 1;
	}
	else if (A.StaticComponentMaskParameters.Num() < B.StaticComponentMaskParameters.Num())
	{
		return -1;
	}

	for (INT MaskIndex = 0; MaskIndex < A.StaticComponentMaskParameters.Num(); MaskIndex++)
	{
		const FStaticComponentMaskParameter &MaskA = A.StaticComponentMaskParameters(MaskIndex);
		const FStaticComponentMaskParameter &MaskB = B.StaticComponentMaskParameters(MaskIndex);

		if (MaskA.ParameterName.ToString() != MaskB.ParameterName.ToString())
		{
			return (MaskA.ParameterName.ToString() > MaskB.ParameterName.ToString()) * 2 - 1;
		} 

		if (MaskA.R != MaskB.R) { return (MaskA.R > MaskB.R) * 2 - 1; } 
		if (MaskA.G != MaskB.G) { return (MaskA.G > MaskB.G) * 2 - 1; } 
		if (MaskA.B != MaskB.B) { return (MaskA.B > MaskB.B) * 2 - 1; } 
		if (MaskA.A != MaskB.A) { return (MaskA.A > MaskB.A) * 2 - 1; } 
	}

	return 0;
})

/**
 * Presave function. Gets called once before an object gets serialized for saving.  Sorts the MaterialShaderMap
 * maps so that shader cache serialization is deterministic.
 */
void UShaderCache::PreSave()
{
	Super::PreSave();
	MaterialShaderMap.KeySort<COMPARE_CONSTREF_CLASS(FStaticParameterSet,SortMaterialsByStaticParamSet)>();
}

void UShaderCache::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if(Ar.IsSaving())
	{
		Save(Ar);

		// Mark the cache as not dirty.
		bDirty = FALSE;
	}
	else if(Ar.IsLoading())
	{
		Load(Ar);
	}

	if( Ar.IsCountingMemory() )
	{
		MaterialShaderMap.CountBytes( Ar );
		for( TMap<FStaticParameterSet,TRefCountPtr<class FMaterialShaderMap> >::TIterator It(MaterialShaderMap); It; ++It )
		{
			It.Value()->Serialize( Ar );
		}
	}
}

/** Keeps track of accumulative time spent in UShaderCache::Load. */
DOUBLE GShaderCacheLoadTime = 0;

void UShaderCache::Load(FArchive& Ar)
{
	SCOPE_SECONDS_COUNTER(GShaderCacheLoadTime);

	if(Ar.Ver() < VER_GLOBAL_SHADER_FILE)
	{
		Ar << Platform;
		Ar << ShaderTypeCRCMap;
		Ar << VertexFactoryTypeCRCMap;
	}

	// Serialize the shaders.
	FShaderCache::Load(Ar);

	if(Ar.Ver() >= VER_GLOBAL_SHADER_FILE)
	{
		Ar << VertexFactoryTypeCRCMap;
	}

	TArray<FString> OutdatedVFTypes;
	INT NumMaterialShaderMaps = 0;
	INT NumRedundantMaterialShaderMaps = 0;
	Ar << NumMaterialShaderMaps;

	// Load the material shader indices in the cache.
	for( INT MaterialIndex=0; MaterialIndex<NumMaterialShaderMaps; MaterialIndex++ )
	{
		FStaticParameterSet StaticParameters;
		StaticParameters.Serialize(Ar);

		// Deserialize the offset of the next material.
		INT SkipOffset = 0;
		Ar << SkipOffset;

		FMaterialShaderMap* ExistingMaterialShaderIndex = FMaterialShaderMap::FindId(StaticParameters,(EShaderPlatform)Platform);

		if (ExistingMaterialShaderIndex && CONSOLE)
		{
			++NumRedundantMaterialShaderMaps;

			// If a shader map with the same ID is already resident, skip this one.
			// On PC we want to still deserialize the shader map in case it is more complete than the existing one.
			Ar.Seek(SkipOffset);
		}
		else
		{
			// Deserialize the material shader map.
			FMaterialShaderMap* MaterialShaderIndex = new FMaterialShaderMap();
			MaterialShaderIndex->Serialize(Ar);

			if (ShouldReloadChangedShaders())
			{
				//go through each vertex factory type
				for(TLinkedList<FVertexFactoryType*>::TIterator It(FVertexFactoryType::GetTypeList()); It; It.Next())
				{
					const DWORD* VFTypeCRC = VertexFactoryTypeCRCMap.Find(*It);
					DWORD CurrentCRC = It->GetSourceCRC();

					//if the CRC that the type was compiled with doesn't match the current CRC, flush the type
					if (VFTypeCRC != NULL && CurrentCRC != *VFTypeCRC)
					{
						MaterialShaderIndex->FlushShadersByVertexFactoryType(*It);
						OutdatedVFTypes.AddUniqueItem(FString(It->GetName()));
					}
				}
			}

			if(ExistingMaterialShaderIndex)
			{
				// merge the new material shader map into the existing one
				ExistingMaterialShaderIndex->Merge(MaterialShaderIndex);
				delete MaterialShaderIndex;
			}
			else
			{
				// Register this shader map in the global map with the material's ID.
				MaterialShaderIndex->Register();
				ExistingMaterialShaderIndex = MaterialShaderIndex;
			}
		}

		// Add a reference to the shader map from the cache. This ensures that the shader map isn't deleted between 
		// the cache being deserialized and PostLoad being called on materials in the same package.
		MaterialShaderMap.Set(StaticParameters,ExistingMaterialShaderIndex);
	}

	if(Ar.Ver() < VER_GLOBAL_SHADER_FILE)
	{
		// Ignore legacy global shader maps saved in package shader caches.
		TShaderMap<FGlobalShaderType>* NewGlobalShaderMap = new TShaderMap<FGlobalShaderType>();
		NewGlobalShaderMap->Serialize(Ar);
	}

	// Log which types were skipped/flushed because their source files have changed
	// Note that these aren't always accurate, for example if the local shader cache is older 
	// than the ref then these are expected to be out of date.
	if (ShouldReloadChangedShaders() && GSerializingLocalShaderCache)
	{
		INT NumOutdatedVFTypes = OutdatedVFTypes.Num();
		if (NumOutdatedVFTypes > 0)
		{
			warnf(NAME_DevShaders, TEXT("Skipped %i outdated FVertexFactoryTypes:"), NumOutdatedVFTypes);
			for (INT TypeIndex = 0; TypeIndex < NumOutdatedVFTypes; TypeIndex++)
			{	
				warnf(NAME_DevShaders, TEXT("	%s"), *OutdatedVFTypes(TypeIndex));
			}
		}
	}
	
	// Log some cache stats.
	if( NumMaterialShaderMaps )
	{
		debugf(TEXT("Shader cache for %s contains %u materials (%u redundant)"),*GetOutermost()->GetName(),NumMaterialShaderMaps,NumRedundantMaterialShaderMaps);
	}
}

void UShaderCache::Save(FArchive& Ar)
{
// the reference shader should never be saved (when it's being created, it's actually the local shader cache for that run of the commandlet)
//		check(this != GShaderCaches[SC_Reference][Platform]);

	// Find the shaders used by materials in the cache.
	TMap<FGuid,FShader*> Shaders;

	for( TMap<FStaticParameterSet,TRefCountPtr<FMaterialShaderMap> >::TIterator MaterialIt(MaterialShaderMap); MaterialIt; ++MaterialIt )
	{
		const TRefCountPtr<FMaterialShaderMap>& ShaderMap = MaterialIt.Value();
		ShaderMap->GetShaderList(Shaders);
	}

	CompleteCRCMaps();

	// Serialize the shaders.
	FShaderCache::Save(Ar,Shaders);

	Ar << VertexFactoryTypeCRCMap;

	// Save the material shader indices in the cache.
	INT NumMaterialShaderMaps = MaterialShaderMap.Num();
	Ar << NumMaterialShaderMaps;
	for( TMap<FStaticParameterSet,TRefCountPtr<FMaterialShaderMap> >::TIterator MaterialIt(MaterialShaderMap); MaterialIt; ++MaterialIt )
	{
		// Serialize the material static parameter set separate, so at load time we can see whether this is a redundant material shader map without fully
		// deserializing it.
		FStaticParameterSet StaticParameters = MaterialIt.Key();
		StaticParameters.Serialize(Ar);

		// Write a placeholder value for the skip offset.
		INT SkipOffset = Ar.Tell();
		Ar << SkipOffset;

		// Serialize the material shader map.
		MaterialIt.Value()->Serialize(Ar);

		// Write the actual offset of the end of the material shader map data over the placeholder value written above.
		INT EndOffset = Ar.Tell();
		Ar.Seek(SkipOffset);
		Ar << EndOffset;
		Ar.Seek(EndOffset);
	}
}

/**
 * Loads the reference and local shader caches for the passed in platform
 *
 * @param	Platform	Platform to load shader caches for.
 */
static void LoadShaderCaches( EShaderPlatform Platform )
{
	// If we're building the reference shader cache, only load the reference shader cache.
	const UBOOL bLoadOnlyReferenceShaderCache = ParseParam(appCmdLine(), TEXT("refcache"));

	for( INT TypeIndex=0; TypeIndex<SC_NumShaderCacheTypes; TypeIndex++ )
	{
		UShaderCache*& ShaderCacheRef = GShaderCaches[TypeIndex][Platform];
		//check for reentry
		check(ShaderCacheRef == NULL);

		// If desired, skip loading the local shader cache.
		if(TypeIndex == SC_Local && bLoadOnlyReferenceShaderCache)
		{
			continue;
		}

		// mark that we are serializing the local shader cache, so we can load the global shaders
		GSerializingLocalShaderCache = (TypeIndex == SC_Local);
		
		// by default, we have no shadercache
		ShaderCacheRef = NULL;

		// only look for the shader cache object if the package exists (avoids a throw inside the code)
		FFilename PackageName = (TypeIndex == SC_Local ? GetLocalShaderCacheFilename(Platform) : GetReferenceShaderCacheFilename(Platform));
		FString Filename;
		if (GPackageFileCache->FindPackageFile(*PackageName, NULL, Filename))
		{
			// if another instance is writing the shader cache while we are reading, then opening will fail, and 
			// that's not good, so retry until we can open it
#if !CONSOLE
			// this "lock" will make sure that another process can't be writing to the package before we actually 
			// read from it with LoadPackage, etc below
			FArchive* ReaderLock = NULL;
			// try to open the shader cache for 
			DOUBLE StartTime = appSeconds();
			const DOUBLE ShaderRetryDelaySeconds = 15.0;

			// try until we can read the file, or until ShaderRetryDelaySeconds has passed
			while (ReaderLock == NULL && appSeconds() - StartTime < ShaderRetryDelaySeconds)
			{
				ReaderLock = GFileManager->CreateFileReader(*Filename);
				if(!ReaderLock)
				{
					// delay a bit
					appSleep(1.0f);
				}
			}
#endif

			UBOOL bSkipLoading = FALSE;
			if( TypeIndex == SC_Local && (CONSOLE || !GUseSeekFreeLoading))
			{
				// This function is being called during script compilation, which is why we need to use LOAD_FindIfFail.
				UObject::BeginLoad();
				ULinkerLoad* Linker = UObject::GetPackageLinker( NULL, *Filename, LOAD_NoWarn | LOAD_FindIfFail, NULL, NULL );
				UObject::EndLoad();
				// Skip loading the local shader cache if it was built with an old version.
				const INT MaxVersionDifference = 10;
				// Skip loading the local shader cache if the version # is less than a given threshold
				const INT MinShaderCacheVersion = VER_PARTICLE_LOD_DISTANCE_FIXUP;
				if( Linker && 
					((GEngineVersion - Linker->Summary.EngineVersion) > MaxVersionDifference ||
					Linker->Ver() < MinShaderCacheVersion) )
				{
					bSkipLoading = TRUE;
				}
			}

			// Skip loading the shader cache if wanted.
			if( !bSkipLoading )
			{
#if !CONSOLE
				// if we are seekfree loading and we are in here, we are a PC game which doesn't cook the shadercaches into each package, 
				// but we can't call LoadPackage normally because we are going to be inside loading Engine.u at this point, so the 
				// ResetLoaders at the end of the LoadPackage (in the seekfree case) will fail because the objects in the shader cache 
				// package won't get EndLoad called on them because we are already inside an EndLoad (for Engine.u).
				// Instead, we use LoadObject, and we will ResetLoaders on the package later in the startup process. We need to disable 
				// seekfree temporarily because LoadObject doesn't work in the seekfree case if the object isn't already loaded.
				if (GUseSeekFreeLoading)
				{
					// unset GUseSeekFreeLoading so that the LoadObject will work
					GUseSeekFreeLoading = FALSE;
					ShaderCacheRef = LoadObject<UShaderCache>(NULL, *(PackageName.GetBaseFilename() + TEXT(".CacheObject")), NULL, LOAD_NoWarn, NULL );
					GUseSeekFreeLoading = TRUE;
				}
				else
#endif
				{
					// This function is being called during script compilation, which is why we need to use LOAD_FindIfFail.
					UPackage* ShaderCachePackage = UObject::LoadPackage( NULL, *Filename, LOAD_NoWarn | LOAD_FindIfFail );
					if( ShaderCachePackage )
					{
						ShaderCacheRef = FindObject<UShaderCache>( ShaderCachePackage, TEXT("CacheObject") );
					}
				}
			}
#if !CONSOLE
			delete ReaderLock;
#endif
		}

		if(!ShaderCacheRef)
		{
			// if we didn't find the local shader cache, create it. if we don't find the refshadercache, that's okay, just leave it be
			if (TypeIndex == SC_Local || bLoadOnlyReferenceShaderCache)
			{
				// If the local shader cache couldn't be loaded, create an empty cache.
				FString LocalShaderCacheName = FString(TEXT("LocalShaderCache-")) + ShaderPlatformToText(Platform);
				ShaderCacheRef = new(UObject::CreatePackage(NULL,*LocalShaderCacheName),TEXT("CacheObject")) UShaderCache(Platform);
				ShaderCacheRef->MarkPackageDirty(FALSE);
			}
		}
		// if we found it, make sure it's loaded
		else
		{
			// if this function was inside a BeginLoad()/EndLoad(), then the LoadObject above didn't actually serialize it, this will
			ShaderCacheRef->GetLinker()->Preload(ShaderCacheRef);
		}

		if (ShaderCacheRef)
		{
			// make sure it's not GC'd
			ShaderCacheRef->AddToRoot();
		}

		GSerializingLocalShaderCache = FALSE;
	}

	// If we only loaded the reference shader cache, use it in place of the local shader cache.
	if(bLoadOnlyReferenceShaderCache)
	{
		GShaderCaches[SC_Local][Platform] = GShaderCaches[SC_Reference][Platform];
	}
}

/**
 * Returns the reference shader cache for the passed in platform
 *
 * @param	Platform	Platform to return reference shader cache for.
 * @return	The reference shader cache for the passed in platform
 */
UShaderCache* GetReferenceShaderCache( EShaderPlatform Platform )
{
	// make sure shader caches are loaded
	if( !GShaderCaches[SC_Local][Platform] )
	{
		LoadShaderCaches( Platform );
	}

	return GShaderCaches[SC_Reference][Platform];
}

/**
 * Returns the local shader cache for the passed in platform
 *
 * @param	Platform	Platform to return local shader cache for.
 * @return	The local shader cache for the passed in platform
 */
UShaderCache* GetLocalShaderCache( EShaderPlatform Platform )
{
	if( !GShaderCaches[SC_Local][Platform] )
	{
		LoadShaderCaches( Platform );
	}

	return GShaderCaches[SC_Local][Platform];
}

/**
 * Returns the global shader cache for the passed in platform
 *
 * @param	Platform	Platform to return global shader cache for.
 * @return	The global shader cache for the passed in platform
 */
FShaderCache* GetGlobalShaderCache( EShaderPlatform Platform )
{
	if(!GGlobalShaderCaches[Platform])
	{
		GGlobalShaderCaches[Platform] = new FShaderCache(Platform);
	}
	return GGlobalShaderCaches[Platform];
}

/**
 * Saves the local shader cache for the passed in platform.
 *
 * @param	Platform	Platform to save shader cache for.
 * @param	OverrideCacheFilename If non-NULL, then the shader cache will be saved to the given path
 */
void SaveLocalShaderCache(EShaderPlatform Platform, const TCHAR* OverrideCacheFilename)
{
#if !SHIPPING_PC_GAME
	// Only save the shader cache for the first instance running.
	if( GIsFirstInstance )
	{
#endif

		UShaderCache* ShaderCache = GShaderCaches[SC_Local][Platform];
		if( ShaderCache && ShaderCache->IsDirty())
		{
			// Reset the LinkerLoads for all shader caches, since we may be saving the local shader cache over the refshadercache file.
			for(INT TypeIndex = 0;TypeIndex < SC_NumShaderCacheTypes;TypeIndex++)
			{
				if(GShaderCaches[TypeIndex][Platform])
				{
					UObject::ResetLoaders(GShaderCaches[TypeIndex][Platform]);
				}
			}

			UPackage* ShaderCachePackage = ShaderCache->GetOutermost();
			// The shader cache isn't network serializable
			ShaderCachePackage->PackageFlags |= PKG_ServerSideOnly;

			if( OverrideCacheFilename )
			{
				UObject::SavePackage(ShaderCachePackage, ShaderCache, 0, OverrideCacheFilename, GWarn, NULL, FALSE, TRUE, SAVE_NoError);
			}
			else
			{
				UObject::SavePackage(ShaderCachePackage, ShaderCache, 0, *GetLocalShaderCacheFilename(Platform), GWarn, NULL, FALSE, TRUE, SAVE_NoError);
			}



			// mark it as clean, as its been saved!
			ShaderCache->MarkClean();

			// release memory held by cached shader files
			FlushShaderFileCache();
		}

#if !SHIPPING_PC_GAME
	}
	else
	{
		// Only warn once.
		static UBOOL bAlreadyWarned = FALSE;
		if( !bAlreadyWarned )
		{
			bAlreadyWarned = TRUE;
			debugf( NAME_Warning, TEXT("Skipping saving the shader cache as another instance of the game is running.") );
		}
	}
#endif
}

/**
 * Saves all local shader caches.
 */
extern void SaveLocalShaderCaches()
{
	for( INT PlatformIndex=0; PlatformIndex<SP_NumPlatforms; PlatformIndex++ )
	{
		SaveLocalShaderCache( (EShaderPlatform)PlatformIndex );
	}
}

FString GetShaderCacheFilename(const TCHAR* Name, const TCHAR* Extension, EShaderPlatform Platform)
{
	if(GUseSeekFreeLoading)
	{
		// When running with seek-free loading, the shader caches will be in the cooked content folder.
		return FString::Printf(
			TEXT("%s%s%s-%s.%s"), 
			*(appGameDir() * TEXT("Cooked") + appGetPlatformString()),
			PATH_SEPARATOR,
			Name,
			ShaderPlatformToText(Platform),
			Extension
			);
	}
	else
	{
		// find the first entry in the Paths array that contains the GameDir (so we don't find an ..\Engine path)
		for (INT PathIndex = 0; PathIndex < GSys->Paths.Num(); PathIndex++)
		{
			// Swap Windows directory separators with platform-dependent ones.
			const FString Path(GSys->Paths(PathIndex).Replace(TEXT("\\"), PATH_SEPARATOR));
			// does the path contain GameDir? (..\ExampleGame\)
			if (Path.InStr(appGameDir(), FALSE, TRUE) != -1)
			{
				// if so, use it
				return FString::Printf(TEXT("%s%s%s-%s.%s"), 
					*GSys->Paths(PathIndex),
					PATH_SEPARATOR,
					Name,
					ShaderPlatformToText(Platform),
					Extension
					);
			}
		}

		checkf(FALSE, TEXT("When making the ShaderCache filename, failed to find a GSys->Path containing %s"), *appGameDir());
		return TEXT("");
	}
}

FString GetLocalShaderCacheFilename( EShaderPlatform Platform )
{
	return GetShaderCacheFilename(TEXT("LocalShaderCache"), TEXT("upk"), Platform);
}

FString GetReferenceShaderCacheFilename( EShaderPlatform Platform )
{
	return GetShaderCacheFilename(TEXT("RefShaderCache"), TEXT("upk"), Platform);
}

FString GetGlobalShaderCacheFilename(EShaderPlatform Platform)
{
	return GetShaderCacheFilename(TEXT("GlobalShaderCache"),TEXT("bin"),Platform);
}
