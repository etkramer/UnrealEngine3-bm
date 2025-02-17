/*=============================================================================
	ShaderCache.h: Shader cache definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __SHADERCACHE_H__
#define __SHADERCACHE_H__

#include "ShaderManager.h"

/** A collection of persistent shaders. */
class FShaderCache
{
public:

	/** Finds a CRC stored for the given shaderType on the given platform */
	static const DWORD* GetShaderTypeCRC(FShaderType* LookupShaderType, EShaderPlatform Platform);

	/** Sets a CRC for the given shaderType on the given platform */
	static void SetShaderTypeCRC(FShaderType* InShaderType, DWORD InCRC, EShaderPlatform Platform);

	/** Removes a FShaderType from the CRC map of all shader caches of the given platform. */
	static void RemoveShaderTypeCRC(FShaderType* InShaderType, EShaderPlatform Platform);

	/** Initialization constructor. */
	FShaderCache(EShaderPlatform InPlatform = SP_PCD3D_SM3)
	:	Platform(InPlatform)
	{}

	/** Loads the shader cache. */
	void Load(FArchive& Ar);
	
	/** Saves the shader cache. */
	void Save(FArchive& Ar,const TMap<FGuid,FShader*>& Shaders);

protected:

	/** Map from shader type to the CRC that the shader type was compiled with */
	TMap<FShaderType*, DWORD> ShaderTypeCRCMap;

	/** Platform this shader cache is for. */
	BYTE Platform;
};

/** A collection of persistent shaders and material shader maps. */
class UShaderCache : public UObject, public FShaderCache
{
	DECLARE_CLASS(UShaderCache,UObject,CLASS_Intrinsic,Engine);
	NO_DEFAULT_CONSTRUCTOR(UShaderCache);
public:

	/**
	 * Flushes the shader map for a material from the platform cache.
	 * @param StaticParameters - The static parameter set identifying the material
	 * @param Platform Platform to flush.
	 */
	static void FlushId(const class FStaticParameterSet& StaticParameters, EShaderPlatform Platform);

	/**
	 * Flushes the shader map for a material from all caches.
	 * @param StaticParameters - The static parameter set identifying the material
	 */
	static void FlushId(const class FStaticParameterSet& StaticParameters);

	/** Finds a CRC stored for the given vertexFactoryType on the given platform */
	static const DWORD* GetVertexFactoryTypeCRC(FVertexFactoryType* LookupVFType, EShaderPlatform Platform);

	/** Sets a CRC for the given vertexFactoryType on the given platform */
	static void SetVertexFactoryTypeCRC(FVertexFactoryType* InVertexFactoryType, DWORD InCRC, EShaderPlatform Platform);

	/** Removes a FVertexFactoryType from the CRC map of all shader caches of the given platform. */
	static void RemoveVertexFactoryTypeCRC(FVertexFactoryType* InVertexFactoryType, EShaderPlatform Platform);

	/**
	 * Combines OtherCache's shaders with this one.  
	 * OtherCache has priority and will overwrite any existing shaders, otherwise they will just be added.
	 *
	 * @param OtherCache	Shader cache to merge
	 */
	void Merge(UShaderCache* OtherCache);

	/**
	 * Adds a material shader map to the cache fragment.
	 *
	 * @param MaterialShaderIndex - The shader map for the material.
	 */
	void AddMaterialShaderMap(class FMaterialShaderMap* MaterialShaderMap);

	/**
	 * Constructor.
	 * @param	Platform	Platform this shader cache is for.
	 */
	UShaderCache( EShaderPlatform Platform );

	// UObject interface.
	virtual void FinishDestroy();
	virtual void PreSave();
	virtual void Serialize(FArchive& Ar);

	// Accessors.
	UBOOL IsDirty() const 
	{ 
		return bDirty; 
	}

	// set the package as clean
	void MarkClean() 
	{
		bDirty = FALSE; 
	}
	// set the package as dirty
	void MarkDirty() 
	{ 
		bDirty = TRUE; 
	}

private:

	/** Map from vertex factory type to the CRC that the shader type was compiled with */
	TMap<FVertexFactoryType*, DWORD> VertexFactoryTypeCRCMap;

	/** Map from material static parameter set to shader map.			*/
	TMap<FStaticParameterSet,TRefCountPtr<class FMaterialShaderMap> > MaterialShaderMap;
	/** Whether shader cache has been modified since the last save.	*/
	UBOOL bDirty;

	/**
	 * Copies over CRC entries from the ref cache if necessary,
	 * only called on the local shader cache before saving.
	 */
	void CompleteCRCMaps();
	
	/** Loads the shader cache. */
	void Load(FArchive& Ar);
	
	/** Saves the shader cache. */
	void Save(FArchive& Ar);
};

/**
 * Returns the reference shader cache for the passed in platform
 *
 * @param	Platform	Platform to return reference shader cache for.
 * @return	The reference shader cache for the passed in platform
 */
extern UShaderCache* GetReferenceShaderCache( EShaderPlatform Platform );

/**
 * Returns the local shader cache for the passed in platform
 *
 * @param	Platform	Platform to return local shader cache for.
 * @return	The local shader cache for the passed in platform
 */
extern UShaderCache* GetLocalShaderCache( EShaderPlatform Platform );

/**
 * Returns the global shader cache for the passed in platform
 *
 * @param	Platform	Platform to return global shader cache for.
 * @return	The global shader cache for the passed in platform
 */
extern FShaderCache* GetGlobalShaderCache( EShaderPlatform Platform );

/**
 * Saves the local shader cache for the passed in platform.
 *
 * @param	Platform	Platform to save shader cache for.
 * @param	OverrideCacheFilename If non-NULL, then the shader cache will be saved to the given path
 */
extern void SaveLocalShaderCache( EShaderPlatform Platform, const TCHAR* OverrideCacheFilename=NULL );

/** Saves all local shader caches. */
extern void SaveLocalShaderCaches();

/** @return The filename for the local shader cache file on the specified platform. */
extern FString GetLocalShaderCacheFilename( EShaderPlatform Platform );

/** @return The filename for the reference shader cache file on the specified platform. */
extern FString GetReferenceShaderCacheFilename( EShaderPlatform Platform );

/** @return The filename for the global shader cache on the specified platform. */
extern FString GetGlobalShaderCacheFilename(EShaderPlatform Platform);

#endif
