/*=============================================================================
	UnContentCookers.cpp: Various platform specific content cookers.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "EngineAIClasses.h"
#include "EngineMaterialClasses.h"
#include "EngineSoundClasses.h"
#include "UnTerrain.h"
#include "EngineParticleClasses.h"
#include "EngineAnimClasses.h"
#include "EngineInterpolationClasses.h"
#include "AnimationUtils.h"
#include "UnCodecs.h"
#include "UnConsoleSupportContainer.h"
#include "UnNet.h"

#include "DebuggingDefines.h"

#define MPSounds_PackageName		TEXT("MPSounds")
#define MPSounds_ReferencerName		MPSounds_PackageName TEXT("ObjRef")

static INT BrushPhysDataBytes = 0;

//@script patcher
static UBOOL bAbortCooking=FALSE;

UPersistentCookerData* GShippingCookerData = NULL;

/**
 * Detailed execution time stats
 */
static DOUBLE	TotalTime;

static DOUBLE		LoadSectionPackagesTime;
static DOUBLE		LoadNativePackagesTime;
static DOUBLE		LoadDependenciesTime;
static DOUBLE		LoadPackagesTime;
static DOUBLE		LoadPerMapPackagesTime;
static DOUBLE		LoadCombinedStartupPackagesTime;
static DOUBLE		CheckDependentPackagesTime;
extern DOUBLE		GShaderCacheLoadTime;
static DOUBLE		SaveShaderCacheTime;
static DOUBLE		CopyShaderCacheTime;
extern DOUBLE		GRHIShaderCompileTime_Total;
extern DOUBLE		GRHIShaderCompileTime_PS3;
extern DOUBLE		GRHIShaderCompileTime_XBOXD3D;
extern DOUBLE		GRHIShaderCompileTime_PCD3D_SM2;
extern DOUBLE		GRHIShaderCompileTime_PCD3D_SM3;
extern DOUBLE		GRHIShaderCompileTime_PCD3D_SM4;

static DOUBLE		CookTime;
static DOUBLE			CookPhysicsTime;
static DOUBLE			CookTextureTime;
static DOUBLE			CookSoundTime;
static DOUBLE			LocSoundTime;
static DOUBLE			CookMovieTime;
static DOUBLE			CookStripTime;
static DOUBLE			CookSkeletalMeshTime;
static DOUBLE			CookStaticMeshTime;

static DOUBLE		PackageSaveTime;
static DOUBLE		PackageLocTime;

static DOUBLE		PrepareForSavingTime;
static DOUBLE			PrepareForSavingTextureTime;
static DOUBLE			PrepareForSavingTerrainTime;
static DOUBLE			PrepareForSavingMaterialTime;
static DOUBLE			PrepareForSavingMaterialInstanceTime;

static DOUBLE		CollectGarbageAndVerifyTime;

static DOUBLE		PrePackageIterationTime;
static DOUBLE		PackageIterationTime;
static DOUBLE			PrepPackageTime;
static DOUBLE			InitializePackageTime;
static DOUBLE			FinalizePackageTime;
static DOUBLE		PostPackageIterationTime;

static DOUBLE		PersistentFaceFXTime;
static DOUBLE			PersistentFaceFXDeterminationTime;
static DOUBLE			PersistentFaceFXGenerationTime;

// Compression stats.
extern DOUBLE		GCompressorTime;
extern QWORD		GCompressorSrcBytes;
extern QWORD		GCompressorDstBytes;
extern DOUBLE		GArchiveSerializedCompressedSavingTime;

extern INT Localization_GetLanguageExtensionIndex(const TCHAR* Ext);

using namespace UE3;

/**
 * @return A string representing the platform
 */
FString UCookPackagesCommandlet::GetPlatformString()
{
	switch (Platform)
	{
		case PLATFORM_Windows:
			return TEXT("PC");

		case PLATFORM_Xenon:
			return TEXT("Xenon");

		case PLATFORM_PS3:
			return TEXT("PS3");

		case PLATFORM_Linux:
			return TEXT("Linux");

		case PLATFORM_MacOSX:
			return TEXT("MacOSX");

		default:
			return TEXT("");
	}	
}

/**
 * @return The prefix used in ini files, etc for the platform
 */
FString UCookPackagesCommandlet::GetPlatformPrefix()
{
	switch (Platform)
	{
		case PLATFORM_Xenon:
			return TEXT("Xe");

		default:
			return GetPlatformString();
	}	
}

/**
 * @return The name of the output cooked data directory
 */
FString UCookPackagesCommandlet::GetCookedDirectory()
{
	FString CookedDir;

	// user mode cooking requires special handling
	if (bIsInUserMode)
	{
		switch (Platform)
		{
			// cooking unpublished data goes to published
			case PLATFORM_Windows:
				CookedDir = FString(TEXT("Published\\CookedPC")); 
				break;

			// PS3 mods go into DLC directory
			case PLATFORM_PS3:
				CookedDir = FString(TEXT("Mods\\PS3\\")) + UserModName;
				break;

			// Xbox DLC 
			case PLATFORM_Xenon:
				CookedDir = FString::Printf(TEXT("DLC\\Xenon\\%s\\CookedXenon\\"),*UserModName);
				break;
		}
	}
	else
	{
		CookedDir = FString(TEXT("Cooked")) + GetPlatformString();
	}
	
	return CookedDir;
}

/**
 * @return The name of the directory where cooked ini files go
 */
FString UCookPackagesCommandlet::GetConfigOutputDirectory()
{
	return GetPlatformString() * TEXT("Cooked");
}

/**
 * @return The prefix to pass to appCreateIniNamesAndThenCheckForOutdatedness for non-platform specific inis
 */
FString UCookPackagesCommandlet::GetConfigOutputPrefix()
{
	return GetConfigOutputDirectory() + PATH_SEPARATOR;

}

/**
 * @return The prefix to pass to appCreateIniNamesAndThenCheckForOutdatedness for platform specific inis
 */
FString UCookPackagesCommandlet::GetPlatformConfigOutputPrefix()
{
	return GetConfigOutputPrefix() + GetPlatformPrefix() + TEXT("-");
}

/**
 * @return The default ini prefix to pass to appCreateIniNamesAndThenCheckForOutdatedness for 
 */
FString UCookPackagesCommandlet::GetPlatformDefaultIniPrefix()
{
	return GetPlatformString() * GetPlatformString();
}

/**
 * @return TRUE if the destination platform expects pre-byteswapped data (packages, coalesced ini files, etc)
 */
UBOOL UCookPackagesCommandlet::ShouldByteSwapData()
{
	return Platform == PLATFORM_Xenon || Platform == PLATFORM_PS3;
}

/**
 * @return The name of the bulk data container file to use
 */
FString UCookPackagesCommandlet::GetBulkDataContainerFilename()
{
	if (bIsInUserMode && Platform == PLATFORM_PS3)
	{
		return TEXT("GlobalPersistentCookerDataPS3.upk");
	}
	else
	{
		return TEXT("GlobalPersistentCookerData.upk");
	}
}

/**
 * @return The name of the guid cache file to use
 */
FString UCookPackagesCommandlet::GetGuidCacheFilename()
{
	FString Filename = TEXT("GuidCache");

	// append mod name if it exists so that it is differentiated fro the shipping guid cache
	if (bIsInUserMode && UserModName != TEXT(""))
	{
		Filename += TEXT("_");
		Filename += UserModName;
	}

	if (Platform == PLATFORM_Windows)
	{
		// tack on a PC usable extension
		Filename += TEXT(".");
		Filename += GSys->Extensions(0);
	}
	else
	{
		// use cooked extension
		Filename += TEXT(".xxx");
	}

	return Filename;
}

FString UCookPackagesCommandlet::GetTextureCacheName(UTexture* Texture)
{
	FString Filename;
#if 0	
	if(	Texture->LODGroup == TEXTUREGROUP_Character ||
		Texture->LODGroup == TEXTUREGROUP_CharacterNormalMap ||
		Texture->LODGroup == TEXTUREGROUP_CharacterSpecular)
	{
		// Character textures go in CharTextures.tfc
		Filename = TEXT("CharTextures");
	}
	else
	{
		// All other textures go in Textures.tfc
		Filename = TEXT("Textures");
	}
#else
	Filename = TEXT("Textures");
#endif

	// append mod name if cooking a new texture for a mod
	// @todo pc: How to handle this for PC cooking, which has no mod names??! Also GuidCache!

	UBOOL bIsShippedTexture = FALSE;
	if (GShippingCookerData)
	{
		// look for any mips for the texture in the shipped texture info
		for (INT Mip = 0; Mip < 12 && !bIsShippedTexture; Mip++)
		{
			FString					BulkDataName = *FString::Printf(TEXT("MipLevel_%i"), Mip);
			FCookedBulkDataInfo*	BulkDataInfo = GShippingCookerData->GetBulkDataInfo(Texture, *BulkDataName);

			// if the texture was found in the shipped info, then we mark it as shipped
			if (BulkDataInfo)
			{
				bIsShippedTexture = TRUE;
			}
		}
	}

	// shipped textures point to the original, not the mod version
	if (!bIsShippedTexture && bIsInUserMode && UserModName != TEXT(""))
	{
		Filename += TEXT("_");
		Filename += UserModName;
	}

	return Filename;
}

FArchive* UCookPackagesCommandlet::GetTextureCacheArchive(const FName& TextureCacheName)
{
	check( bUseTextureFileCache );

	// Check for an existing open archive for the specified texture file cache.
	FArchive* TextureCacheArchive = TextureCacheNameToArMap.FindRef(TextureCacheName);
	if(!TextureCacheArchive)
	{
		FString TextureCacheFilename;
		// need to handle distributed cooking here because the file isn't a package, so 
		// UObject::SetSavePackagePathRedirection won't help
		if (bIsDistributed)
		{
			TextureCacheFilename = appGameDir() + FString(TEXT("Jobs-")) + GetPlatformString() * JobName * GetCookedDirectory() * TextureCacheName.ToString() + TEXT(".") + GSys->TextureFileCacheExtension;
		}
		else
		{
			TextureCacheFilename = CookedDir + TextureCacheName.ToString() + TEXT(".") + GSys->TextureFileCacheExtension;
		}

		// Create texture cache file writer.
		TextureCacheArchive = GFileManager->CreateFileWriter( *TextureCacheFilename, FILEWRITE_Append );
		if (TextureCacheArchive == NULL)
		{
			appMsgf(AMT_OK, TEXT("Failed to open %s - make sure it's not in use by another program or console"),*TextureCacheFilename);
			return FALSE;
		}
		// Propagate byte swapping based on cooking platform.	
		TextureCacheArchive->SetByteSwapping( ShouldByteSwapData() );
		// Seek to the end of the file.
		INT CurrentFileSize = TextureCacheArchive->TotalSize();
		check( CurrentFileSize >= 0 );
		TextureCacheArchive->Seek( CurrentFileSize );

		// Add the newly opened texture cache file to our opened texture cache map.
		TextureCacheNameToArMap.Set(TextureCacheName,TextureCacheArchive);
		TextureCacheNameToFilenameMap.Set(TextureCacheName,TextureCacheFilename);
	}

	return TextureCacheArchive;
}

/**
 *	@return	UBOOL		TRUE if the texture cache entry was valid, FALSE if not.
 */
UBOOL UCookPackagesCommandlet::VerifyTextureFileCacheEntry()
{
	if (bUseTextureFileCache == FALSE)
	{
		// Don't fail, as technically the entry is valid (ie it's not used so who cares)
		return TRUE;
	}

	UBOOL bValid = TRUE;

	// Verify the character texture file cache...
	for (INT CharIndex = 0; CharIndex < CharTFCCheckData.Num(); CharIndex++)
	{
		FTextureFileCacheEntry& TFCEntry = CharTFCCheckData(CharIndex);

		// Open the file and verify the expected header...

		// Compare against all other entries in the cache...
		for (INT InnerIndex = CharIndex + 1; InnerIndex < CharTFCCheckData.Num(); InnerIndex++)
		{
			FTextureFileCacheEntry& InnerTFCEntry = CharTFCCheckData(InnerIndex);
			if (FTextureFileCacheEntry::EntriesOverlap(TFCEntry, InnerTFCEntry) == TRUE)
			{
				warnf(NAME_Warning, TEXT("**** Texture overlap found in Character TFC!"));
				warnf(NAME_Warning, TEXT("\tOffset = %10d, Size = %10d, Mip %d of %s"), 
					TFCEntry.OffsetInCache, TFCEntry.SizeInCache, TFCEntry.MipIndex, *(TFCEntry.TextureName));
				warnf(NAME_Warning, TEXT("\tOffset = %10d, Size = %10d, Mip %d of %s"), 
					InnerTFCEntry.OffsetInCache, InnerTFCEntry.SizeInCache, InnerTFCEntry.MipIndex, *(InnerTFCEntry.TextureName));

				bValid = FALSE;
			}
		}
	}

	for (INT Index = 0; Index < TFCCheckData.Num(); Index++)
	{
		FTextureFileCacheEntry& TFCEntry = TFCCheckData(Index);

		// Open the file and verify the expected header...

		// Compare against all other entries in the cache...
		for (INT InnerIndex = Index + 1; InnerIndex < TFCCheckData.Num(); InnerIndex++)
		{
			FTextureFileCacheEntry& InnerTFCEntry = TFCCheckData(InnerIndex);
			if (FTextureFileCacheEntry::EntriesOverlap(TFCEntry, InnerTFCEntry) == TRUE)
			{
				warnf(NAME_Warning, TEXT("**** Texture overlap found in TFC!"));
				warnf(NAME_Warning, TEXT("\tOffset = %10d, Size = %10d, Mip %d of %s"), 
					TFCEntry.OffsetInCache, TFCEntry.SizeInCache, TFCEntry.MipIndex, *(TFCEntry.TextureName));
				warnf(NAME_Warning, TEXT("\tOffset = %10d, Size = %10d, Mip %d of %s"), 
					InnerTFCEntry.OffsetInCache, InnerTFCEntry.SizeInCache, InnerTFCEntry.MipIndex, *(InnerTFCEntry.TextureName));

				bValid = FALSE;
			}
		}
	}

	return bValid;
}

/**
 *	Verifies that the data in the texture file cache for the given texture
 *	is a valid 'header' packet...
 *
 *	@param	Package		The package the texture was cooked into.
 *	@param	Texture2D	The texture that was cooked.
 *	@param	bIsSaved...	If TRUE, the texture was saved in a seekfree pacakge
 *
 *	@return	UBOOL		TRUE if the texture cache entry was valid, FALSE if not.
 */
UBOOL UCookPackagesCommandlet::AddVerificationTextureFileCacheEntry(UPackage* Package, UTexture2D* Texture2D, UBOOL bIsSavedInSeekFreePackage)
{
	if (bUseTextureFileCache == FALSE)
	{
		// Don't fail, as technically the entry is valid (ie it's not used so who cares)
		return TRUE;
	}
	if (Texture2D->TextureFileCacheName == NAME_None)
	{
		return TRUE;
	}

	// look for any mips for the texture in the shipped texture info
	//@SAS. Why 12? Shouldn't this be a define somewhere??
	for (INT Mip = 0; Mip < 12; Mip++)
	{
		FString					BulkDataName = *FString::Printf(TEXT("MipLevel_%i"), Mip);
		FCookedBulkDataInfo*	BulkDataInfo = PersistentCookerData->GetBulkDataInfo(Texture2D, *BulkDataName);

		// if the texture was found in the shipped info, then we mark it as shipped
		if (BulkDataInfo)
		{
			if ((BulkDataInfo->SavedBulkDataFlags & BULKDATA_StoreInSeparateFile) != 0)
			{
				FTextureFileCacheEntry* TFCEntry = TFCVerificationData.Find(BulkDataName);
				if (TFCEntry == NULL)
				{
					FTextureFileCacheEntry TempEntry;
					TFCVerificationData.Set(Texture2D->GetPathName(), TempEntry);
					TFCEntry = TFCVerificationData.Find(Texture2D->GetPathName());

					TFCEntry->TextureName = Texture2D->GetPathName();
					TFCEntry->MipIndex = Mip;
					TFCEntry->OffsetInCache = BulkDataInfo->SavedBulkDataOffsetInFile;
					TFCEntry->SizeInCache = BulkDataInfo->SavedBulkDataSizeOnDisk;
					TFCEntry->FlagsInCache = BulkDataInfo->SavedBulkDataFlags;
					TFCEntry->ElementCountInCache = BulkDataInfo->SavedElementCount;

					if (Texture2D->TextureFileCacheName == TEXT("CharTextures"))
					{
						TFCEntry->bCharTextureFileCache = TRUE;
					}
					else
					{
						TFCEntry->bCharTextureFileCache = FALSE;
					}

#if 0 // Define this to 1 to log out each texture added to the TFC when running in VerifyTFC...
					warnf(NAME_Warning, TEXT("Added to %8s TFC: Offset = %10d, Size = %10d, Mip = %2d, %s"),
						TFCEntry->bCharTextureFileCache == TRUE ? TEXT("CHAR") : TEXT("NORM"),
						TFCEntry->OffsetInCache, TFCEntry->SizeInCache,
						TFCEntry->MipIndex, *(TFCEntry->TextureName));
#endif

					FTextureFileCacheEntry* OverlapEntry = FindTFCEntryOverlap(*TFCEntry);
					if (OverlapEntry)
					{
						warnf(NAME_Warning, TEXT("AddVerificationTextureFileCacheEntry> Texture overlap found in TFC!"));
						warnf(NAME_Warning, TEXT("\tMip %d of %s"), TFCEntry->MipIndex, *(TFCEntry->TextureName));
						warnf(NAME_Warning, TEXT("\t\tOffset = %10d, Size = %d"), TFCEntry->OffsetInCache, TFCEntry->SizeInCache);
						warnf(NAME_Warning, TEXT("\tMip %d of %s"), OverlapEntry->MipIndex, *(OverlapEntry->TextureName));
						warnf(NAME_Warning, TEXT("\t\tOffset = %10d, Size = %d"), OverlapEntry->OffsetInCache, OverlapEntry->SizeInCache);
					}

					if (Texture2D->TextureFileCacheName == TEXT("CharTextures"))
					{
						FTextureFileCacheEntry* CheckData = new(CharTFCCheckData)FTextureFileCacheEntry(*TFCEntry);
					}
					else
					{
						FTextureFileCacheEntry* CheckData = new(TFCCheckData)FTextureFileCacheEntry(*TFCEntry);
					}
				}
				else
				{
					if ((TFCEntry->TextureName != Texture2D->GetPathName()) ||
						(TFCEntry->MipIndex != Mip) ||
						(TFCEntry->OffsetInCache != BulkDataInfo->SavedBulkDataOffsetInFile) ||
						(TFCEntry->SizeInCache != BulkDataInfo->SavedBulkDataSizeOnDisk) ||
						(TFCEntry->FlagsInCache != BulkDataInfo->SavedBulkDataFlags) ||
						(TFCEntry->ElementCountInCache != BulkDataInfo->SavedElementCount))
					{
						warnf(NAME_Warning, TEXT("DATA MISMATCH on texture already in file cache!"));
						warnf(NAME_Warning, TEXT("\tTextureName:	      EXPECTED %s"), *(Texture2D->GetPathName()));
						warnf(NAME_Warning, TEXT("\t                      FOUND    %s"), *(TFCEntry->TextureName));
						warnf(NAME_Warning, TEXT("\tMipIndex:             EXPECTED %10d, FOUND %10d"), Mip, TFCEntry->MipIndex);
						warnf(NAME_Warning, TEXT("\tOffsetInCache:        EXPECTED %10d, FOUND %10d"), TFCEntry->OffsetInCache, BulkDataInfo->SavedBulkDataOffsetInFile);
						warnf(NAME_Warning, TEXT("\tSizeInCache:          EXPECTED %10d, FOUND %10d"), BulkDataInfo->SavedBulkDataSizeOnDisk, TFCEntry->SizeInCache);
						warnf(NAME_Warning, TEXT("\tFlagsInCache:         EXPECTED 0x%08x, FOUND 0x%08x"), BulkDataInfo->SavedBulkDataFlags, TFCEntry->FlagsInCache);
						warnf(NAME_Warning, TEXT("\tElementCountInCache:  EXPECTED %10d, FOUND %10d"), BulkDataInfo->SavedElementCount, TFCEntry->ElementCountInCache);
					}
				}
			}
		}
	}

	return TRUE;
}

FTextureFileCacheEntry* UCookPackagesCommandlet::FindTFCEntryOverlap(FTextureFileCacheEntry& InEntry)
{
	if (InEntry.bCharTextureFileCache == TRUE)
	{
		for (INT CharCheckIndex = 0; CharCheckIndex < CharTFCCheckData.Num(); CharCheckIndex++)
		{
			FTextureFileCacheEntry& CheckEntry = CharTFCCheckData(CharCheckIndex);
			if (FTextureFileCacheEntry::EntriesOverlap(InEntry, CheckEntry))
			{
				return &CheckEntry;
			}
		}
	}
	else
	{
		for (INT CheckIndex = 0; CheckIndex < TFCCheckData.Num(); CheckIndex++)
		{
			FTextureFileCacheEntry& CheckEntry = TFCCheckData(CheckIndex);
			if (FTextureFileCacheEntry::EntriesOverlap(InEntry, CheckEntry))
			{
				return &CheckEntry;
			}
		}
	}

	return NULL;
}

/**
 *	Generate the mappings of sub-levels to PMaps...
 *
 *	@param	CommandLineMapList		The array of map names passed in on the command-line
 */
void UCookPackagesCommandlet::GeneratePersistentMapList(TArray<FString>& CommandLineMapList)
{
#if WITH_FACEFX
	SCOPE_SECONDS_COUNTER(PersistentFaceFXTime);
	SCOPE_SECONDS_COUNTER(PersistentFaceFXDeterminationTime);
	if (bGeneratePersistentMapAnimSet)
	{
		warnf(NAME_Log, TEXT("Generating level to persistent level list..."));
		for (INT MapIndex = 0; MapIndex < CommandLineMapList.Num(); MapIndex++)
		{
			FFilename SrcFilename = CommandLineMapList(MapIndex);
/***
			//@todo.SAS. Move this to an INI section!
			FString CheckName = SrcFilename.GetBaseFilename();
			if ((CheckName.InStr(TEXT("GearGame")) != INDEX_NONE) ||
				(CheckName.InStr(TEXT("GearStart")) != INDEX_NONE) ||
				(CheckName.InStr(TEXT("GearEntry")) != INDEX_NONE))
			{
				continue;
			}
***/
			FString PersistentLevelName = SrcFilename.GetBaseFilename();
			PersistentLevelName = PersistentLevelName.ToUpper();

			// See if it is a PersistentMap...
			// This code assumes there are no nested streaming levels.
			// Ie, a streamed level doesn't itself contain streaming levels.
			// If also assumes only Persistent levels are passed in!
			FFilename PackageFilename = SrcFilename;
			TArray<FFilename> MapsToProcess;
			MapsToProcess.AddUniqueItem(PackageFilename);
			// read the package file summary of the package to get a list of sublevels
			FArchive* PackageFile = GFileManager->CreateFileReader(*PackageFilename);
			if (PackageFile)
			{
				// read the package summary, which has list of sub levels
				FPackageFileSummary Summary;
				(*PackageFile) << Summary;

				// close the map
				delete PackageFile;

				if ((Summary.PackageFlags & PKG_ContainsMap) != 0)
				{
					// if it's an old map, then we have to load it to get the list of sublevels
					if (Summary.GetFileVersion() < VER_ADDITIONAL_COOK_PACKAGE_SUMMARY)
					{
						SCOPE_SECONDS_COUNTER(LoadPackagesTime);

						warnf(NAME_Log, TEXT("  Old package, so must open fully to look for sublevels"), *PackageFilename);
						UPackage* Package = UObject::LoadPackage(NULL, *PackageFilename, LOAD_None);

						// Iterate over all UWorld objects and look for streaming levels.
						for( TObjectIterator<UWorld> It; It; ++It )
						{
							UWorld*		World		= *It;
							if (World->IsIn(Package))
							{
								AWorldInfo* WorldInfo	= World->GetWorldInfo();
								// Iterate over streaming level objects loading the levels.
								for (INT StreamIndex = 0; StreamIndex < WorldInfo->StreamingLevels.Num(); StreamIndex++)
								{
									MapsToProcess.AddUniqueItem(WorldInfo->StreamingLevels(StreamIndex)->PackageName.ToString());
								}
							}
						}

						// close the package
						CollectGarbageAndVerify();
					}
					else
					{
						// Streaming levels present?
						for (INT AdditionalPackageIndex = 0; AdditionalPackageIndex < Summary.AdditionalPackagesToCook.Num(); AdditionalPackageIndex++)
						{
							if( GPackageFileCache->FindPackageFile(*Summary.AdditionalPackagesToCook(AdditionalPackageIndex), NULL, PackageFilename))
							{
								FArchive* AdditionalPackageFile = GFileManager->CreateFileReader(*PackageFilename);
								if (AdditionalPackageFile)
								{
									// read the package summary, which has list of sub levels
									FPackageFileSummary AdditionalSummary;
									(*AdditionalPackageFile) << AdditionalSummary;

									// close the map
									delete AdditionalPackageFile;

									if ((AdditionalSummary.PackageFlags & PKG_ContainsMap) != 0)
									{
										MapsToProcess.AddUniqueItem(PackageFilename);
									}
								}
							}
						}
					}
				}
			}

			if (bLogPersistentFaceFXGeneration)
			{
				warnf(NAME_Log, TEXT("GeneratePersistentMapList> Processing persistent map package %s"), *PackageFilename);
			}

			for (INT MapIndex = 0; MapIndex < MapsToProcess.Num(); MapIndex++)
			{
				FFilename MapPackageFile = MapsToProcess(MapIndex);
				FArchive* CheckMapPackageFile = GFileManager->CreateFileReader(*MapPackageFile);
				if (CheckMapPackageFile)
				{
					// read the package summary, which has list of sub levels
					FPackageFileSummary AdditionalSummary;
					(*CheckMapPackageFile) << AdditionalSummary;

					// close the map
					delete CheckMapPackageFile;

					if ((AdditionalSummary.PackageFlags & PKG_ContainsMap) != 0)
					{
						FString UpperMapName = MapPackageFile.GetBaseFilename().ToUpper();
						FString* ExistingEntry = LevelToPersistentLevelMap.Find(UpperMapName);
						if (ExistingEntry)
						{
							if (*ExistingEntry != PersistentLevelName)
							{
								warnf(NAME_Warning, TEXT("Map found in LevelToPersistentLevelMap already!"));
								warnf(NAME_Warning, TEXT("\t%s - was %s, trying to set %s"),
									*UpperMapName, *(*ExistingEntry), *PersistentLevelName);
							}
						}
						LevelToPersistentLevelMap.Set(UpperMapName, PersistentLevelName);
					}
				}
			}
		}

		warnf(NAME_Log, TEXT("\tThere are %d levels to process"), LevelToPersistentLevelMap.Num());
#if 0 // Make this one to output the Map --> PMap list
		warnf(NAME_Log, TEXT("\t..... Level to PMap list ....."));
		for (TMap<FString, FString>::TIterator L2PMapIt(LevelToPersistentLevelMap); L2PMapIt; ++L2PMapIt)
		{
			warnf(NAME_Log, TEXT("\t\t%32s........%s"), *(L2PMapIt.Key()), *(L2PMapIt.Value()));
		}
		warnf(NAME_Log, TEXT("\t..... End Level to PMap list ....."));
#endif	//#
	}
#endif	//#if WITH_FACEFX
}

#if WITH_FACEFX
/** Helper function to mangle the given FaceFX names for the persistent level anim set */
void UCookPackagesCommandlet_GenerateMangledFaceFXInformation(UCookPackagesCommandlet* Commandlet, 
	FString& PersistentLevelName, UCookPackagesCommandlet::FMangledFaceFXInfo& PFFXData)
{
	// This code assume the PFFXData.FaceFXAnimSet, PFFXData.OriginalFaceFXGroupName 
	// and PFFXData.OriginalFaceFXAnimName are set correctly!
	FString ConcatanatedGroupName = FString::Printf(TEXT("%s.%s"), *(PFFXData.FaceFXAnimSet), *(PFFXData.OriginalFaceFXGroupName));
	FString* MangledNamePrefix = Commandlet->GroupNameToMangledMap.Find(ConcatanatedGroupName);
	if (MangledNamePrefix == NULL)
	{
		FString NewMangledName = FString::Printf(TEXT("%d"), Commandlet->FaceFXMangledNameCount++);
		Commandlet->GroupNameToMangledMap.Set(ConcatanatedGroupName, NewMangledName);
		MangledNamePrefix = Commandlet->GroupNameToMangledMap.Find(ConcatanatedGroupName);
		check(MangledNamePrefix);
		//debugf(TEXT("\tAdded mangled group name %5s (%s)"), *NewMangledName, *ConcatanatedGroupName);
	}
	
	PFFXData.MangledFaceFXGroupName = PersistentLevelName;
	PFFXData.MangledFaceFXAnimName = *MangledNamePrefix + PFFXData.OriginalFaceFXAnimName;
}

INT UCookPackagesCommandlet_FindSourceAnimSet(UCookPackagesCommandlet* Commandlet, 
	TArrayNoInit<class UFaceFXAnimSet*>& FaceFXAnimSets, FString& GroupName, FString& AnimName)
{
	for (INT SetIndex = 0; SetIndex < FaceFXAnimSets.Num(); SetIndex++)
	{
		UFaceFXAnimSet* FFXAnimSet = FaceFXAnimSets(SetIndex);
		if (FFXAnimSet)
		{
			FString SeqNameToCheck = GroupName + TEXT(".") + AnimName;
			TArray<FString> SeqNames;
			FFXAnimSet->GetSequenceNames(SeqNames);
			for (INT SeqIndex = 0; SeqIndex < SeqNames.Num(); SeqIndex++)
			{
				if (SeqNameToCheck == SeqNames(SeqIndex))
				{
					return SetIndex;
				}
			}
		}
	}
	return -1;
}

UCookPackagesCommandlet::FMangledFaceFXInfo* UCookPackagesCommandlet_FindMangledFaceFXInfo(
	UCookPackagesCommandlet* Commandlet, UCookPackagesCommandlet::TGroupAnimToMangledFaceFXMap* GroupAnimToFFXMap,
	FString& AnimSetRefName, FString& GroupName, FString& AnimName)
{
	UCookPackagesCommandlet::FMangledFaceFXInfo* MangledFFXInfo = NULL;

	FString GroupAnimName = GroupName + TEXT(".") + AnimName;
	GroupAnimName = GroupAnimName.ToUpper();
	UCookPackagesCommandlet::TMangledFaceFXMap* MangledFaceFXMap = GroupAnimToFFXMap->Find(GroupAnimName);
	if (MangledFaceFXMap)
	{
		MangledFFXInfo = MangledFaceFXMap->Find(AnimSetRefName);
	}

	return MangledFFXInfo;
}

UCookPackagesCommandlet::TMangledFaceFXMap* UCookPackagesCommandlet_GetMangledFaceFXMap(
	UCookPackagesCommandlet* Commandlet, UCookPackagesCommandlet::TGroupAnimToMangledFaceFXMap* GroupAnimToFFXMap,
	FString& GroupName, FString& AnimName, UBOOL bCreateIfNotFound)
{
	FString GroupAnimName = GroupName + TEXT(".") + AnimName;
	GroupAnimName = GroupAnimName.ToUpper();
	UCookPackagesCommandlet::TMangledFaceFXMap* MangledFFXMap = GroupAnimToFFXMap->Find(GroupAnimName);
	if ((MangledFFXMap == NULL) && bCreateIfNotFound)
	{
		// Doesn't exist yet...
		UCookPackagesCommandlet::TMangledFaceFXMap TempMangled;
		GroupAnimToFFXMap->Set(GroupAnimName, TempMangled);
		MangledFFXMap = GroupAnimToFFXMap->Find(GroupAnimName);
	}
	check(bCreateIfNotFound ? (MangledFFXMap != NULL) : 1);
	return MangledFFXMap;
}

UCookPackagesCommandlet::FMangledFaceFXInfo* UCookPackagesCommandlet_GetMangledFaceFXMap(
	UCookPackagesCommandlet* Commandlet, UCookPackagesCommandlet::TMangledFaceFXMap* MangledFFXMap,
	FString& PersistentLevelName, FString& AnimSetRefName, FString& GroupName, FString& AnimName, UBOOL bCreateIfNotFound)
{
	UCookPackagesCommandlet::FMangledFaceFXInfo* MangledFFXInfo = MangledFFXMap->Find(AnimSetRefName);
	if (bCreateIfNotFound)
	{
		if (MangledFFXInfo == NULL)
		{
			UCookPackagesCommandlet::FMangledFaceFXInfo TempMangledFFX;
			MangledFFXMap->Set(AnimSetRefName, TempMangledFFX);
			MangledFFXInfo = MangledFFXMap->Find(AnimSetRefName);
			check(MangledFFXInfo);

			MangledFFXInfo->FaceFXAnimSet = AnimSetRefName;
			MangledFFXInfo->OriginalFaceFXGroupName = GroupName;
			MangledFFXInfo->OriginalFaceFXAnimName = AnimName;

			UCookPackagesCommandlet_GenerateMangledFaceFXInformation(Commandlet, PersistentLevelName, *MangledFFXInfo);
		}
		else
		{
			if ((MangledFFXInfo->FaceFXAnimSet != AnimSetRefName) ||
				(MangledFFXInfo->OriginalFaceFXGroupName != GroupName) ||
				(MangledFFXInfo->OriginalFaceFXAnimName != AnimName))
			{
				warnf(NAME_Log, TEXT("ERROR: Persistent FaceFX Data mismatch on %s: Stored %s %s, On Cue %s %s)"), 
					*MangledFFXInfo->OriginalFaceFXGroupName, *MangledFFXInfo->OriginalFaceFXAnimName,
					*GroupName, *AnimName);
			}
		}
	}
	check(bCreateIfNotFound ? (MangledFFXInfo != NULL) : 1);
	return MangledFFXInfo;
}
#endif	//#if WITH_FACEFX

/** 
 *	Check the given map package for persistent level.
 *	If it is, then generate the corresponding FaceFX animation information.
 *
 *	@param	InPackageFile	The name of the package being loaded
 */
void UCookPackagesCommandlet::SetupPersistentMapFaceFXAnimation(const FFilename& InPackageFile)
{
#if WITH_FACEFX
	SCOPE_SECONDS_COUNTER(PersistentFaceFXTime);
	SCOPE_SECONDS_COUNTER(PersistentFaceFXDeterminationTime);

	FString UpperMapName = InPackageFile.GetBaseFilename();
	FString* PMapStringPtr = LevelToPersistentLevelMap.Find(UpperMapName);
	if (PMapStringPtr == NULL)
	{
		// Map not found... nothing to do!
		return;
	}

	FString PersistentLevelName = *PMapStringPtr;
	PersistentLevelName = PersistentLevelName.ToUpper();

	UBOOL bItIsThePersistentMap = (UpperMapName == PersistentLevelName);

	// Find the mapping of info...
	TGroupAnimToMangledFaceFXMap* GroupAnimToFFXMap = PersistentMapToGroupAnimLookupMap.Find(PersistentLevelName);
	if (GroupAnimToFFXMap == NULL)
	{
		// It has not been generated yet - do it!
		TGroupAnimToMangledFaceFXMap TempPMGA;
		PersistentMapToGroupAnimLookupMap.Set(PersistentLevelName, TempPMGA);
		GroupAnimToFFXMap = PersistentMapToGroupAnimLookupMap.Find(PersistentLevelName);
		check(GroupAnimToFFXMap);

		TArray<FMangledFaceFXInfo>* PMapToFaceFXData = PersistentMapFaceFXArray.Find(PersistentLevelName);
		if (PMapToFaceFXData == NULL)
		{
			TArray<FMangledFaceFXInfo> TempData;
			PersistentMapFaceFXArray.Set(PersistentLevelName, TempData);
			PMapToFaceFXData = PersistentMapFaceFXArray.Find(PersistentLevelName);
		}
		check(PMapToFaceFXData);

		// Figure out all the sub-levels (and the pmap) that are pointed to by this map
		TArray<FFilename> MapsToProcess;
		for (TMap<FString, FString>::TIterator L2PMapIt(LevelToPersistentLevelMap); L2PMapIt; ++L2PMapIt)
		{
			FString Value = L2PMapIt.Value();

			if (Value == PersistentLevelName)
			{
				MapsToProcess.AddItem(L2PMapIt.Key());
			}
		}

		// Process each map...
		for (INT MapIndex = 0; MapIndex < MapsToProcess.Num(); MapIndex++)
		{
			FFilename MapPackageFile = MapsToProcess(MapIndex);
			if (bLogPersistentFaceFXGeneration)
			{
				warnf(NAME_Log, TEXT("SetupPersistentMapFaceFXAnimation> Processing map package %s"), *MapPackageFile);
			}
			UPackage* MapPackage = UObject::LoadPackage( NULL, *MapPackageFile, LOAD_None );
			if (MapPackage == NULL)
			{
				warnf(NAME_Log, TEXT("SetupPersistentMapFaceFXAnimation> Failed to load package %s"), *MapPackageFile);
				continue;
			}

			UWorld* World = FindObject<UWorld>(MapPackage, TEXT("TheWorld"));
			if (World == NULL)
			{
				//debugf(TEXT("SetupPersistentMapFaceFXAnimation> Not a map package? %s"), *MapPackageFile);
				continue;
			}
			AWorldInfo* WorldInfo = World->GetWorldInfo();
			if (WorldInfo == NULL)
			{
				//debugf(TEXT("SetupPersistentMapFaceFXAnimation> No WorldInfo? %s"), *MapPackageFile);
				continue;
			}

			FString UpperMapName = MapPackage->GetName().ToUpper();

			INT SoundCueCount = 0;
			INT MangledSoundCueCount = 0;
			INT SeqAct_PlayFaceFXAnmCount = 0;
			INT MangledSeqAct_PlayFaceFXAnmCount = 0;
			INT InterpFaceFXCount = 0;
			INT MangledInterpFaceFXCount = 0;

			if (bLogPersistentFaceFXGeneration)
			{
				debugf(TEXT("\t..... Checking objects ....."));
			}
			for (FObjectIterator It; It; ++It)
			{
				USoundCue* SoundCue = Cast<USoundCue>(*It);
				UInterpTrackFaceFX* ITFaceFX = Cast<UInterpTrackFaceFX>(*It);
				USeqAct_PlayFaceFXAnim* SAPlayFFX = Cast<USeqAct_PlayFaceFXAnim>(*It);

				// Note: It is assumed that ANY sound cue that is encountered is going to be cooked into the map...
				if (SoundCue && (SoundCue->HasAnyFlags(RF_MarkedByCooker|RF_Transient) == FALSE))
				{
					UBOOL bIgnoreSoundCue = FALSE;
					if (GGameCookerHelper)
					{
						bIgnoreSoundCue = GGameCookerHelper->ShouldSoundCueBeIgnoredForPersistentFaceFX(this, SoundCue);
					}

					if (bIgnoreSoundCue == FALSE)
					{
						SoundCueCount++;
						if (SoundCue->FaceFXAnimSetRef)
						{
							FString AnimSetRefName = SoundCue->FaceFXAnimSetRef->GetPathName();
							if (ScriptReferencedFaceFXAnimSets.Find(AnimSetRefName) == NULL)
							{
								if (SoundCue->FaceFXGroupName.Len())
								{
									if (SoundCue->FaceFXAnimName.Len())
									{
										// Find it in the mangled array
										FString GroupName = SoundCue->FaceFXGroupName;
										FString AnimName = SoundCue->FaceFXAnimName;

										TMangledFaceFXMap* MangledFFXMap = UCookPackagesCommandlet_GetMangledFaceFXMap(this, 
											GroupAnimToFFXMap, GroupName, AnimName, TRUE);
										check(MangledFFXMap);
										// Find the info for this sound
										FMangledFaceFXInfo* MangledFFXInfo = UCookPackagesCommandlet_GetMangledFaceFXMap(this, 
											MangledFFXMap, PersistentLevelName, AnimSetRefName, GroupName, AnimName, FALSE);
										if (MangledFFXInfo == NULL)
										{
											MangledFFXInfo = UCookPackagesCommandlet_GetMangledFaceFXMap(this, 
												MangledFFXMap, PersistentLevelName, AnimSetRefName, 
												GroupName, AnimName, TRUE);
											PMapToFaceFXData->AddUniqueItem(*MangledFFXInfo);
											if (bLogPersistentFaceFXGeneration)
											{
												warnf(NAME_Log, TEXT("SoundCue %s: %s-%s --> %s-%s"), 
													*(SoundCue->GetPathName()),
													*GroupName, *AnimName,
													*(MangledFFXInfo->MangledFaceFXGroupName), 
													*(MangledFFXInfo->MangledFaceFXAnimName));
											}
										}
										MangledSoundCueCount++;
									}
									else
									{
										if (bLogPersistentFaceFXGeneration)
										{
											warnf(TEXT("\tInvalid FaceFXAnimName  on %s"), *(SoundCue->GetName()));
										}
									}
								}
								else
								{
									if (bLogPersistentFaceFXGeneration)
									{
										warnf(TEXT("\tInvalid FaceFXAnimGroup on %s"), *(SoundCue->GetName()));
									}
								}
							}
							else
							{
								// Skipping as it is an untouched FaceFX animset.
								if (bLogPersistentFaceFXGeneration)
								{
									debugf(TEXT("Skipping Safe FaceFXAnimSet %s - SoundCue %s"), 
										*(SoundCue->FaceFXAnimSetRef->GetPathName()), *(SoundCue->GetName()));
								}
							}
						}
						else
						{
							if (SoundCue->FaceFXGroupName.Len())
							{
								if (SoundCue->FaceFXAnimName.Len())
								{
									if (bLogPersistentFaceFXGeneration)
									{
										warnf(TEXT("\tNo AnimSetRef on %s"), *(SoundCue->GetName()));
									}
								}
							}
						}
					}
					else
					{
						//debugf(TEXT("\tIGNORING SoundCue %s"), *(SoundCue->GetPathName()));
					}
				}

				// Note: It is assumed that ANY InterpTrackFaceFX that is encountered is going to be cooked into the map...
				if (ITFaceFX && (ITFaceFX->HasAnyFlags(RF_MarkedByCooker|RF_Transient) == FALSE))
				{
					// 
					for (INT FFXIndex = 0; FFXIndex < ITFaceFX->FaceFXSeqs.Num(); FFXIndex++)
					{
						InterpFaceFXCount++;

						FFaceFXTrackKey& FFXKey = ITFaceFX->FaceFXSeqs(FFXIndex);

						INT SetIndex = UCookPackagesCommandlet_FindSourceAnimSet(this, ITFaceFX->FaceFXAnimSets, FFXKey.FaceFXGroupName, FFXKey.FaceFXSeqName);
						if (SetIndex != -1)
						{
							// If it doesn't find the AnimSetRef, it is assumed to be bogus!
							UFaceFXAnimSet* TheFaceFXAnimSet = ITFaceFX->FaceFXAnimSets(SetIndex);
							if (ScriptReferencedFaceFXAnimSets.Find(TheFaceFXAnimSet->GetPathName()) == NULL)
							{
								// It's a valid one...
								// Find it in the mangled array
								FString AnimSetRefName = TheFaceFXAnimSet->GetPathName();
								FString GroupName = FFXKey.FaceFXGroupName;
								FString AnimName = FFXKey.FaceFXSeqName;

								TMangledFaceFXMap* MangledFFXMap = UCookPackagesCommandlet_GetMangledFaceFXMap(this, GroupAnimToFFXMap, GroupName, AnimName, TRUE);
								check(MangledFFXMap);
								// Find the info for this sound
								FMangledFaceFXInfo* MangledFFXInfo = UCookPackagesCommandlet_GetMangledFaceFXMap(this, MangledFFXMap, PersistentLevelName, 
									AnimSetRefName, GroupName, AnimName, FALSE);
								if (MangledFFXInfo == NULL)
								{
									MangledFFXInfo = UCookPackagesCommandlet_GetMangledFaceFXMap(this, 
										MangledFFXMap, PersistentLevelName, AnimSetRefName, 
										GroupName, AnimName, TRUE);
									PMapToFaceFXData->AddUniqueItem(*MangledFFXInfo);
									if (bLogPersistentFaceFXGeneration)
									{
										warnf(NAME_Log, TEXT("InterpTrack %s: %s-%s --> %s-%s"), 
											*(ITFaceFX->GetPathName()),
											*GroupName, *AnimName,
											*(MangledFFXInfo->MangledFaceFXGroupName), 
											*(MangledFFXInfo->MangledFaceFXAnimName));
									}
								}
								MangledInterpFaceFXCount++;
							}
						}
						else
						{
							if (bLogPersistentFaceFXGeneration)
							{
								warnf(NAME_Log, TEXT("Failed to find AnimSetRef for %s, %s-%s"), 
									*(ITFaceFX->GetPathName()), *FFXKey.FaceFXGroupName, *FFXKey.FaceFXSeqName);
							}
						}
					}
				}

				// Note: It is assumed that ANY InterpTrackFaceFX that is encountered is going to be cooked into the map...
				if (SAPlayFFX && (SAPlayFFX->HasAnyFlags(RF_MarkedByCooker|RF_Transient) == FALSE))
				{
					SeqAct_PlayFaceFXAnmCount++;
					if (SAPlayFFX->FaceFXAnimSetRef != NULL)
					{
						if (ScriptReferencedFaceFXAnimSets.Find(SAPlayFFX->FaceFXAnimSetRef->GetPathName()) == NULL)
						{
							// If valid to mangle...
							// See if it is already in the list.
							FMangledFaceFXInfo TempPFFXData;

							FString AnimSetRefName = SAPlayFFX->FaceFXAnimSetRef->GetPathName();
							FString GroupName = SAPlayFFX->FaceFXGroupName;
							FString AnimName = SAPlayFFX->FaceFXAnimName;

							TMangledFaceFXMap* MangledFFXMap = UCookPackagesCommandlet_GetMangledFaceFXMap(this, GroupAnimToFFXMap, GroupName, AnimName, TRUE);
							check(MangledFFXMap);
							// Find the info for this sound
							FMangledFaceFXInfo* MangledFFXInfo = UCookPackagesCommandlet_GetMangledFaceFXMap(this, MangledFFXMap, PersistentLevelName, 
								AnimSetRefName, GroupName, AnimName, FALSE);
							if (MangledFFXInfo == NULL)
							{
								MangledFFXInfo = UCookPackagesCommandlet_GetMangledFaceFXMap(this, 
									MangledFFXMap, PersistentLevelName, AnimSetRefName, 
									GroupName, AnimName, TRUE);
								PMapToFaceFXData->AddUniqueItem(*MangledFFXInfo);
								if (bLogPersistentFaceFXGeneration)
								{
									warnf(NAME_Log, TEXT("SeqActPlayFaceFX %s: %s-%s --> %s-%s"), 
										*(SAPlayFFX->GetPathName()),
										*GroupName, *AnimName,
										*(MangledFFXInfo->MangledFaceFXGroupName), 
										*(MangledFFXInfo->MangledFaceFXAnimName));
								}
							}
							MangledSeqAct_PlayFaceFXAnmCount++;
						}
					}
					else
					{
						if (bLogPersistentFaceFXGeneration)
						{
							warnf(NAME_Log, TEXT("\t\tNo FaceFXAnimSetRef on %s!"), *(SAPlayFFX->GetPathName()));
						}
					}
				}
			}
			if (bLogPersistentFaceFXGeneration)
			{
				warnf(NAME_Log, TEXT("\t..... Completed SoundCues - Mangled %5d out of %5d....."), MangledSoundCueCount, SoundCueCount);
				warnf(NAME_Log, TEXT("\t..... Completed InterpTrackFaceFX - Mangled %5d out of %5d....."), MangledInterpFaceFXCount, InterpFaceFXCount);
				warnf(NAME_Log, TEXT("\t..... Completed SeqAct_PlayFaceFXAnim - Mangled %5d out of %5d....."), MangledSeqAct_PlayFaceFXAnmCount, SeqAct_PlayFaceFXAnmCount);
			}

			CollectGarbageAndVerify();
		}
	}
#endif	//#if WITH_FACEFX
}

/** 
 *	Generate the persistent level FaceFX AnimSet.
 *
 *	@param	InPackageFile	The name of the package being loaded
 *	@return	UFaceFXAnimSet*	The generated anim set, or NULL if not the persistent map
 */
UFaceFXAnimSet* UCookPackagesCommandlet::GeneratePersistentMapFaceFXAnimSet(const FFilename& InPackageFile)
{
#if WITH_FACEFX 
	SCOPE_SECONDS_COUNTER(PersistentFaceFXTime);
	SCOPE_SECONDS_COUNTER(PersistentFaceFXGenerationTime);

	UFaceFXAnimSet* NewFaceFXAnimSet = NULL;
	FString PersistentMapName = InPackageFile.GetBaseFilename();
	if (CurrentPersistentMapFaceFXArray)
	{
		// Make sure there are some...
		if (CurrentPersistentMapFaceFXArray->Num() == 0)
		{
			// No animations to grab!
			return NewFaceFXAnimSet;
		}

		// Need to pull in all the associated FaceFX stuff
		warnf(NAME_Log, TEXT("Generating FaceFXAnimSet for PersistentMap %s"), *PersistentMapName);
		warnf(NAME_Log, TEXT("\tThere are %5d FaceFX Animations to add..."), CurrentPersistentMapFaceFXArray->Num());

		//@todo.SAS. Sort them by package to minimize overhead? Probably not necessary...

		// Create the new FaceFXAnimSet
		FString AnimSetName = PersistentMapName + TEXT("_FaceFXAnimSet");
		NewFaceFXAnimSet = Cast<UFaceFXAnimSet>(UObject::StaticConstructObject(UFaceFXAnimSet::StaticClass(), (UObject*)GetTransientPackage(), FName(*AnimSetName)));
		check(NewFaceFXAnimSet);
		NewFaceFXAnimSet->SetFlags( RF_Standalone );
		// Add to root to prevent GC!
		NewFaceFXAnimSet->AddToRoot();

		// Create the FxAnimSet...
		OC3Ent::Face::FxName FFXAnimSetName(TCHAR_TO_ANSI(*AnimSetName));
		FString TempAGNewName = PersistentMapName.ToUpper();
		OC3Ent::Face::FxName AnimGroupNewName(TCHAR_TO_ANSI(*TempAGNewName));

		OC3Ent::Face::FxAnimSet* DstFFXAnimSet = new OC3Ent::Face::FxAnimSet();
		check(DstFFXAnimSet);
		DstFFXAnimSet->SetName(FFXAnimSetName);

		// Create the FXAnimGroup...
		OC3Ent::Face::FxAnimGroup ContainedAnimGroup;
		ContainedAnimGroup.SetName(AnimGroupNewName);
		DstFFXAnimSet->SetAnimGroup(ContainedAnimGroup);
		NewFaceFXAnimSet->InternalFaceFXAnimSet = DstFFXAnimSet;

		OC3Ent::Face::FxAnimGroup* DestFFXAnimGroup = (OC3Ent::Face::FxAnimGroup*)&(DstFFXAnimSet->GetAnimGroup());

		// Copy all of the reference animations into the new anim set
		for (INT AnimIndex = 0; AnimIndex < CurrentPersistentMapFaceFXArray->Num(); AnimIndex++)
		{
			FMangledFaceFXInfo& FFXInfo = (*CurrentPersistentMapFaceFXArray)(AnimIndex);

			// loading a FaceFXAsset
			//UFaceFXAsset* SrcFaceFXAsset = LoadObject<UFaceFXAsset>(NULL, *(FFXInfo.FaceFXAnimSet), NULL, LOAD_None, NULL);
			// loading a FaceFXAnimSet
			UFaceFXAnimSet* SrcFaceFXAnimSet = LoadObject<UFaceFXAnimSet>(NULL, *(FFXInfo.FaceFXAnimSet), NULL, LOAD_None, NULL);
			if (SrcFaceFXAnimSet)
			{
				OC3Ent::Face::FxName SrcAnimGroupName(TCHAR_TO_ANSI(*(FFXInfo.OriginalFaceFXGroupName)));

				OC3Ent::Face::FxName SrcAnimName(TCHAR_TO_ANSI(*(FFXInfo.OriginalFaceFXAnimName)));
				OC3Ent::Face::FxSize AnimIndex = OC3Ent::Face::FxInvalidIndex;

				// If it wasn't found in the asset, check in any available animsets...
				const OC3Ent::Face::FxAnimSet* SrcFFXAnimSet = SrcFaceFXAnimSet->GetFxAnimSet();
				const OC3Ent::Face::FxAnimGroup& TempFFXAnimGroup = SrcFFXAnimSet->GetAnimGroup();
				OC3Ent::Face::FxAnimGroup* SrcFFXAnimGroup = (OC3Ent::Face::FxAnimGroup*)&TempFFXAnimGroup;

				AnimIndex = SrcFFXAnimGroup->FindAnim(SrcAnimName);
				if (AnimIndex != OC3Ent::Face::FxInvalidIndex)
				{
					const OC3Ent::Face::FxAnim& AnimRef = SrcFFXAnimGroup->GetAnim(AnimIndex);
					if (DestFFXAnimGroup->AddAnim(AnimRef) == FALSE)
					{
						UBOOL bAlreadyExists = (DestFFXAnimGroup->FindAnim(SrcAnimName) != OC3Ent::Face::FxInvalidIndex) ? TRUE : FALSE;
						warnf(NAME_Log, TEXT("\t%s: Failed to add animation %s from %s"),  
							bAlreadyExists ? TEXT("Already exists") : TEXT("ERROR"), 
							*(FFXInfo.OriginalFaceFXAnimName), *(FFXInfo.FaceFXAnimSet));
					}
					else
					{
						// Rename it within the new AnimSet
						OC3Ent::Face::FxSize NewAnimIndex = DestFFXAnimGroup->FindAnim(SrcAnimName);
						if (AnimIndex != OC3Ent::Face::FxInvalidIndex)
						{
							const OC3Ent::Face::FxAnim& NewAnimRef = DestFFXAnimGroup->GetAnim(NewAnimIndex);
							OC3Ent::Face::FxAnim* NewAnim = (OC3Ent::Face::FxAnim*)&NewAnimRef;
							OC3Ent::Face::FxName NewAnimName(TCHAR_TO_ANSI(*(FFXInfo.MangledFaceFXAnimName)));
							NewAnim->SetName(NewAnimName);
							if (bLogPersistentFaceFXGeneration)
							{
								warnf(NAME_Log, TEXT("\t%s,%s,%s,as,%s,%s"),  
									*(FFXInfo.FaceFXAnimSet), *(FFXInfo.OriginalFaceFXGroupName), *(FFXInfo.OriginalFaceFXAnimName),
									*(FFXInfo.MangledFaceFXGroupName), *(FFXInfo.MangledFaceFXAnimName)
									);
							}
						}
						else
						{
							warnf(NAME_Log, TEXT("\tFailed to find anim %s in dest group!"), ANSI_TO_TCHAR(SrcAnimName.GetAsCstr()));
						}
					}
				}
				else
				{
					warnf(NAME_Log, TEXT("\tFailed to load source animation %s in %s"),  *(FFXInfo.OriginalFaceFXAnimName), *(FFXInfo.FaceFXAnimSet));
				}
			}
			else
			{
				warnf(NAME_Warning, TEXT("\tFailed to load source FaceFXAnimSet %s"), *(FFXInfo.FaceFXAnimSet));
			}
		}

		// Collect garbage and verify it worked as expected.	
		CollectGarbageAndVerify();
	}
	else
	{
		if (bLogPersistentFaceFXGeneration)
		{
			debugf(TEXT("\tNo CurrentPersistentMapFaceFXArray set!"));
		}
	}
	return NewFaceFXAnimSet;
#else	//#if WITH_FACEFX
	return NULL;
#endif	//#if WITH_FACEFX
}

/** 
 *	Prep the given package w.r.t. localization.
 */
void UCookPackagesCommandlet::PrepPackageLocalization( UPackage* Package )
{
	SCOPE_SECONDS_COUNTER(PackageLocTime);

	TArray<USoundNodeWave*> SoundNodeWaves;

	// Iterate over all objects and cook them if they are going to be saved with the package.
	for( FObjectIterator It; It; ++It )
	{
		UObject* Object = *It;
		USoundNodeWave* SoundNodeWave = Cast<USoundNodeWave>(Object);
		if (SoundNodeWave && (Object->IsIn(Package) || Object->HasAnyFlags(RF_ForceTagExp)))
		{
			// Prep the object.
			SoundNodeWaves.AddUniqueItem(SoundNodeWave);
			SoundNodeWave->LocalizedSubtitles.Empty();
			SoundNodeWave->LocalizedSubtitles.AddZeroed(GKnownLanguageCount);
		}
	}

	if (SoundNodeWaves.Num() == 0)
	{
		return;
	}

	// For each language, hook them up...
	TCHAR SavedLangExt[64];
	appStrcpy(SavedLangExt, 64, UObject::GetLanguage());
	INT SavedIndex = Localization_GetLanguageExtensionIndex(SavedLangExt);

	if (bCookCurrentLanguageOnly == FALSE)
	{
		for (INT KnownLangIndex = 0; GKnownLanguageExtensions[KnownLangIndex]; KnownLangIndex++)
		{
			const TCHAR* Ext = GKnownLanguageExtensions[KnownLangIndex];
			// mark each coalesced loc file for sha
			INT LangIndex = Localization_GetLanguageExtensionIndex(Ext);
			if ((LangIndex != -1) && (SavedIndex != LangIndex))
			{
				checkf((LangIndex < GKnownLanguageCount), TEXT("GKnownLanguageCount is not set correctly!"));
				UObject::SetLanguage(Ext, FALSE);
				for (INT SNWIndex = 0; SNWIndex < SoundNodeWaves.Num(); SNWIndex++)
				{
					USoundNodeWave* SoundNodeWave = SoundNodeWaves(SNWIndex);
					if (SoundNodeWave->IsLocalizedResource())
					{
						// Fill in the items...
						FLocalizedSubtitle& LocSubtitle = SoundNodeWave->LocalizedSubtitles(LangIndex);
						SoundNodeWave->LoadLocalized(NULL, FALSE);
						// Copy it into the appropriate slot
						LocSubtitle.bManualWordWrap = SoundNodeWave->bManualWordWrap;
						LocSubtitle.bMature = SoundNodeWave->bMature;
						LocSubtitle.Subtitles = SoundNodeWave->Subtitles;
					}
				}
			}
		}
	}

	// Restore the expected language, and localize the SoundNodeWave for that as well.
	// This will result in the default being the language being cooked for.
	UObject::SetLanguage(SavedLangExt, FALSE);
	for (INT SNWIndex = 0; SNWIndex < SoundNodeWaves.Num(); SNWIndex++)
	{
		USoundNodeWave* SoundNodeWave = SoundNodeWaves(SNWIndex);
		if (SoundNodeWave->IsLocalizedResource())
		{
			// Fill in the items...
			FLocalizedSubtitle& LocSubtitle = SoundNodeWave->LocalizedSubtitles(SavedIndex);
			SoundNodeWave->LoadLocalized(NULL, FALSE);
			// Copy it into the appropriate slot
			LocSubtitle.bManualWordWrap = SoundNodeWave->bManualWordWrap;
			LocSubtitle.bMature = SoundNodeWave->bMature;
			LocSubtitle.Subtitles = SoundNodeWave->Subtitles;
		}
	}
}

void UCookPackagesCommandlet_CookFaceFXInterpTrack(UCookPackagesCommandlet* Commandlet, UInterpTrackFaceFX* FaceFXInterpTrack);
void UCookPackagesCommandlet_CookSeqActPlayFaceFXAnim(UCookPackagesCommandlet* Commandlet, USeqAct_PlayFaceFXAnim* SeqAct_PlayFaceFX);

/**
 * Cooks passed in object if it hasn't been already.
 *
 * @param	Package						Package going to be saved
 * @param	Object		Object to cook
 * @param	bIsSavedInSeekFreePackage	Whether object is going to be saved into a seekfree package
 */
void UCookPackagesCommandlet::CookObject( UPackage* Package, UObject* Object, UBOOL bIsSavedInSeekFreePackage )
{ 
	if( Object && !Object->HasAnyFlags( RF_Cooked ) )
	{
		if( !Object->HasAnyFlags( RF_ClassDefaultObject ) )
		{
			SCOPE_SECONDS_COUNTER(CookTime);

			UBOOL bProcessCook = TRUE;
			if (GGameCookerHelper)
			{
				bProcessCook = GGameCookerHelper->CookObject(this, Package, Object, bIsSavedInSeekFreePackage);
			}

			if (bProcessCook)
			{
				UTexture2D*			Texture2D			= Cast<UTexture2D>(Object);
				UTextureMovie*		TextureMovie		= Cast<UTextureMovie>(Object);
				USoundNodeWave*		SoundNodeWave		= Cast<USoundNodeWave>(Object);
				UTerrainComponent*	TerrainComponent	= Cast<UTerrainComponent>(Object);
				UBrushComponent*	BrushComponent		= Cast<UBrushComponent>(Object);
				UParticleSystem*	ParticleSystem		= Cast<UParticleSystem>(Object);

				USkeletalMesh*		SkeletalMesh		= Cast<USkeletalMesh>(Object);
				UStaticMesh*		StaticMesh			= Cast<UStaticMesh>(Object);
				UClass*				ClassObj			= Cast<UClass>(Object);
				
				AWorldInfo*			WorldInfoActor		= Cast<AWorldInfo>(Object);

				USoundCue*			SoundCue			= Cast<USoundCue>(Object);
				UInterpTrackFaceFX* FaceFXInterpTrack	= Cast<UInterpTrackFaceFX>(Object);
				USeqAct_PlayFaceFXAnim* SeqActPlayFaceFX= Cast<USeqAct_PlayFaceFXAnim>(Object);

				{
					SCOPE_SECONDS_COUNTER(CookPhysicsTime);


					// Rebuild Novodex brush data for BrushComponents that are part of Volumes (eg blocking volumes)
					if (BrushComponent && !BrushComponent->IsTemplate() && BrushComponent->GetOwner() && BrushComponent->GetOwner()->IsVolumeBrush())
					{
						BrushComponent->BuildSimpleBrushCollision();
						BrushComponent->BuildPhysBrushData();

						// Add to memory used total.
						for(INT HullIdx = 0; HullIdx < BrushComponent->CachedPhysBrushData.CachedConvexElements.Num(); HullIdx++)
						{
							FKCachedConvexDataElement& Hull = BrushComponent->CachedPhysBrushData.CachedConvexElements(HullIdx);
							BrushPhysDataBytes += Hull.ConvexElementData.Num();
						}
					}
				}

				// Cook texture, storing in platform format.
				if( Texture2D )
				{
					CookTexture( Package, Texture2D, bIsSavedInSeekFreePackage );
				}
				// Cook movie texture data
				else if( TextureMovie )
				{
					CookMovieTexture( TextureMovie );
				}
				// Cook sound, compressing to platform format
				else if( SoundNodeWave )
				{
					CookSoundNodeWave( SoundNodeWave );

					LocSoundNodeWave( SoundNodeWave );

					// Free up the data cached for the platforms we are not cooking for
					if( Platform == PLATFORM_Xenon )
					{
						SoundNodeWave->CompressedPCData.RemoveBulkData();
						SoundNodeWave->CompressedPS3Data.RemoveBulkData();
					}
					else if( Platform == PLATFORM_PS3 )
					{
						SoundNodeWave->CompressedPCData.RemoveBulkData();
						SoundNodeWave->CompressedXbox360Data.RemoveBulkData();
					}
					else if( Platform == PLATFORM_Windows )
					{
						SoundNodeWave->CompressedPS3Data.RemoveBulkData();
						SoundNodeWave->CompressedXbox360Data.RemoveBulkData();
					}
				}
				else if (SoundCue)
				{
					CookSoundCue(SoundCue);
				}
				else if (FaceFXInterpTrack)
				{
					UCookPackagesCommandlet_CookFaceFXInterpTrack(this, FaceFXInterpTrack);
				}
				else if (SeqActPlayFaceFX)
				{
					UCookPackagesCommandlet_CookSeqActPlayFaceFXAnim(this, SeqActPlayFaceFX);
				}

				// Cook particle systems
				else if (ParticleSystem)
				{
					CookParticleSystem(ParticleSystem);
				}

				else if (SkeletalMesh)
				{
					CookSkeletalMesh(SkeletalMesh);
				}

				else if (StaticMesh)
				{
					CookStaticMesh(StaticMesh);
				}
				// If WorldInfo, clear the ClipPadEntry array
				else if (WorldInfoActor)
				{
					WorldInfoActor->ClipPadEntries.Empty();
				}
				else if ( ClassObj && (Platform&PLATFORM_Console) != 0 )
				{
					// if cooking for console and this is the StaticMeshCollectionActor class, mark it placeable
					// so that it can be spawned at runtime
					if ( ClassObj == AStaticMeshCollectionActor::StaticClass() || ClassObj == AStaticLightCollectionActor::StaticClass() )
					{
						ClassObj->ClassFlags |= CLASS_Placeable;
					}
				}
			}
		}

		{
			SCOPE_SECONDS_COUNTER(CookStripTime);
			// Remove extra data by platform
			Object->StripData(Platform);
		}

		// Avoid re-cooking.
		Object->SetFlags( RF_Cooked );
	}
}

/** mirrored in XeTools.cpp */
enum TextureCookerCreateFlags
{
	// skip miptail packing
	TextureCreate_NoPackedMip = 1<<0,
	// convert to piecewise-linear approximated gamma curve
	TextureCreate_PWLGamma = 1<<1
};

// always use packed miptails
DWORD GCreationFlags = 0;

/**
 * Helper function used by CookObject - performs texture specific cooking.
 *
 * @param	Package						Package going to be saved
 * @param	Texture		Texture to cook
 * @param	bIsSavedInSeekFreePackage	Whether object is going to be saved into a seekfree package
 */
void UCookPackagesCommandlet::CookTexture( UPackage* Package, UTexture2D* Texture2D, UBOOL bIsSavedInSeekFreePackage )
{
	// Cook 2D textures.
	check(Texture2D);
	if( !Texture2D->IsTemplate( RF_ClassDefaultObject ) )
	{
		SCOPE_SECONDS_COUNTER(CookTextureTime);

		// Strip out miplevels for UI textures.
		if( Texture2D->LODGroup == TEXTUREGROUP_UI )
		{
			if( Texture2D->Mips.Num() > 1 )
			{
				Texture2D->Mips.Remove( 1, Texture2D->Mips.Num() - 1 );
			}
		}

		// Take texture LOD settings into account, avoiding cooking and keeping memory around for unused miplevels.
		INT	FirstMipIndex = Clamp<INT>( PlatformLODSettings.CalculateLODBias( Texture2D ), 0, Texture2D->Mips.Num()-1 );		

		if( Platform == PLATFORM_PS3 || Platform == PLATFORM_Xenon )
		{
			// Initialize texture cooker for given format and size.
			TextureCooker->Init( 
				Texture2D->Format, 
				Texture2D->SizeX, 
				Texture2D->SizeY, 
				Texture2D->Mips.Num(),
				(Texture2D->SRGB ? GCreationFlags : (GCreationFlags & ~TextureCreate_PWLGamma)) );

			// first level of the packed miptail
			Texture2D->MipTailBaseIdx = TextureCooker->GetMipTailBase();

			// make sure we load at least the first packed mip level
			FirstMipIndex = Min(FirstMipIndex, Texture2D->MipTailBaseIdx);

			// Only cook mips up to the first packed mip level
			INT MaxMipLevel = Texture2D->Mips.Num();
			if( !(GCreationFlags&TextureCreate_NoPackedMip) )
			{
				MaxMipLevel = Min(Texture2D->MipTailBaseIdx,MaxMipLevel);
			}		

			for( INT MipLevel=FirstMipIndex; MipLevel<MaxMipLevel; MipLevel++ )
			{
				FTexture2DMipMap& Mip = Texture2D->Mips(MipLevel);

				// Allocate enough memory for cooked miplevel.
				UINT	MipSize				= TextureCooker->GetMipSize( MipLevel );
				// a size of 0 means to use original data size as dest size
				if (MipSize == 0)
				{
					MipSize = Mip.Data.GetBulkDataSize();
				}

				void*	IntermediateData	= appMalloc( MipSize );
				appMemzero(IntermediateData, MipSize);

				UINT	SrcRowPitch			= Max<UINT>( 1,	(Texture2D->SizeX >> MipLevel) / GPixelFormats[Texture2D->Format].BlockSizeX ) 
											* GPixelFormats[Texture2D->Format].BlockBytes;
				// Resize upfront to new size to work around issue in Xbox 360 texture cooker reading memory out of bounds.
				// zero-out the newly allocated block of memory as we may not end up using it all
				Mip.Data.Lock(LOCK_READ_WRITE);

				// remember the size of the buffer before realloc
				const INT SizeBeforeRealloc = Mip.Data.GetBulkDataSize();
				void*	MipData				= Mip.Data.Realloc( MipSize );

				// get the size of the newly allocated region
				const INT SizeOfReallocRegion = Mip.Data.GetBulkDataSize() - SizeBeforeRealloc;
				if ( SizeOfReallocRegion > 0 )
				{
					appMemzero((BYTE*)MipData + SizeBeforeRealloc, SizeOfReallocRegion);
				}

				// Cook the miplevel into the intermediate memory.
				TextureCooker->CookMip( MipLevel, MipData, IntermediateData, SrcRowPitch );

				// And replace existing data.
				appMemcpy( MipData, IntermediateData, MipSize );
				appFree( IntermediateData );
				Mip.Data.Unlock();
			}

			// Cook the miptail. This will be the last mip level of the texture
			if( !(GCreationFlags&TextureCreate_NoPackedMip) &&
				Texture2D->MipTailBaseIdx < Texture2D->Mips.Num() )
			{
				// Should always be a multiple of the tile size for this texture's format			
				UINT MipTailSize = TextureCooker->GetMipSize(Texture2D->MipTailBaseIdx); 
				// Source mip data for base level
				FTexture2DMipMap& BaseLevelMip = Texture2D->Mips(Texture2D->MipTailBaseIdx);

				// Allocate space for the mip tail
				void* DstMipTail = new BYTE[MipTailSize];
				appMemzero(DstMipTail, MipTailSize);

				// Arrays to hold the data for the tail mip levels
				const INT TailMipLevelCount = Texture2D->Mips.Num() - Texture2D->MipTailBaseIdx;
				void** SrcMipTailData = new void*[TailMipLevelCount];
				UINT* SrcMipPitch = new UINT[TailMipLevelCount];
				appMemzero(SrcMipPitch, TailMipLevelCount * sizeof(UINT));

				// Build up arrays of data to send to the MipTail cooker
				for( INT MipLevel = Texture2D->MipTailBaseIdx; MipLevel < Texture2D->Mips.Num(); MipLevel++ )
				{
					// source mip data for this MipLevel
					FTexture2DMipMap& Mip = Texture2D->Mips(MipLevel);
					// surface pitch for this mip level
					SrcMipPitch[MipLevel - Texture2D->MipTailBaseIdx] = Max<UINT>( 1, (Texture2D->SizeX >> MipLevel) / GPixelFormats[Texture2D->Format].BlockSizeX ) 
						* GPixelFormats[Texture2D->Format].BlockBytes;

					// lock source data for use. realloc to MipTailSize to work around issue with Xbox 360 texture cooker.
					// zero-out the newly allocated block of memory as we may not end up using it all
					Mip.Data.Lock(LOCK_READ_WRITE);

					// remember the size of the buffer before realloc
					const INT SizeBeforeRealloc = Mip.Data.GetBulkDataSize();
					void* MipData = Mip.Data.Realloc(MipTailSize);

					// get the size of the newly allocated region
					const INT SizeOfReallocRegion = Mip.Data.GetBulkDataSize() - SizeBeforeRealloc;
					if ( SizeOfReallocRegion > 0 )
					{
						appMemzero((BYTE*)MipData + SizeBeforeRealloc, SizeOfReallocRegion);
					}

					SrcMipTailData[MipLevel - Texture2D->MipTailBaseIdx] = MipData;
				}

				// Cook the tail mips together
				TextureCooker->CookMipTail( SrcMipTailData, SrcMipPitch, DstMipTail );

				// And replace existing data. Base level is already locked for writing
				appMemcpy( BaseLevelMip.Data.Realloc( MipTailSize ), DstMipTail, MipTailSize );
				delete [] DstMipTail;
				
				// Unlock the src mip data
				for( INT MipLevel = Texture2D->MipTailBaseIdx+1; MipLevel < Texture2D->Mips.Num(); MipLevel++ )
				{
					FTexture2DMipMap& Mip = Texture2D->Mips(MipLevel);
					// Clear out unused tail mips.
					Mip.Data.Realloc(0);
					Mip.Data.Unlock();
				}

				BaseLevelMip.Data.Unlock();

				delete [] SrcMipTailData;
				delete [] SrcMipPitch;
			}
		}
		else
		{
			// Whether the texture can be streamed.
			UBOOL	bIsStreamingTexture	= !Texture2D->NeverStream;
			// Whether the texture is applied to the face of a cubemap, in which case we can't stream it.
			UBOOL	bIsCubemapFace		= Texture2D->GetOuter()->IsA(UTextureCube::StaticClass());

			// make sure we load at least the first packed mip level
			FirstMipIndex = Min(FirstMipIndex, Texture2D->MipTailBaseIdx);

			// Textures residing in seekfree packages cannot be streamed as the file offset is not known till after
			// they are saved.
			if( (bIsSavedInSeekFreePackage && Texture2D->IsIn( Package ))
			// Cubemap faces cannot be streamed.
			||	bIsCubemapFace )
			{
				bIsStreamingTexture		= FALSE;
				Texture2D->NeverStream	= TRUE;
			}

			// Make sure data is (and remains) loaded for (and after) saving.
			for( INT MipIndex=FirstMipIndex; MipIndex<Texture2D->Mips.Num(); MipIndex++ )
			{
				FTexture2DMipMap& Mip = Texture2D->Mips(MipIndex);
				
				UBOOL bIsStoredInSeparateFile	= FALSE;

				// We still require MinStreamedInMips to be present in the seek free package as the streaming code 
				// assumes those to be always loaded.
				if( bIsSavedInSeekFreePackage
				&&	bIsStreamingTexture
				// @todo streaming, @todo cooking: This assumes the same value of GMinTextureResidentMipCount across platforms.
				&&	(MipIndex < Texture2D->Mips.Num() - GMinTextureResidentMipCount)
				// Miptail must always be loaded.
				&&	MipIndex < Texture2D->MipTailBaseIdx )
				{
					bIsStoredInSeparateFile = TRUE;
				}

				if (!bIsStoredInSeparateFile)
				{
					Mip.Data.Lock(LOCK_READ_WRITE);
					Mip.Data.Unlock();
				}
			}
		}
	}
}

/**
* Check the first bytes of the movie stream for a valid signature
*
* @param Buffer - movie stream buffer including header
* @param BufferSize - total size of movie stream buffer
* @param Signature - signature to compare against
* @param SignatureSize - size of the signature buffer
* @return TRUE if success
*/
UBOOL IsMovieSignatureValid(void* Buffer,INT BufferSize,BYTE* Signature,INT SignatureSize)
{
	UBOOL Result = TRUE;
	// need at least enough room for the signature
	if ( BufferSize >= SignatureSize )
	{
		BYTE* BufferSignature = static_cast<BYTE*>(Buffer);
		// make sure there is a matching signature in the buffer
		for( INT i=0; i < SignatureSize; ++i )
		{
			if( Signature[i] != BufferSignature[i] )
			{
				Result = FALSE;
				break;
			}
		}
	}
	return Result;
}

/**
* Byte swap raw movie data byte stream 
*
* @param Buffer - movie stream buffer including header
* @param BufferSize - total size of movie stream buffer
*/
void EnsureMovieEndianess(void* Buffer, INT BufferSize)
{
	// endian swap the data
	DWORD* Data = static_cast<DWORD*>(Buffer);
	UINT   DataSize = BufferSize / 4;
	for (UINT DataIndex = 0; DataIndex < DataSize; ++DataIndex)
	{
		DWORD SourceData = Data[DataIndex];
		Data[DataIndex] = ((SourceData & 0x000000FF) << 24) |
			((SourceData & 0x0000FF00) <<  8) |
			((SourceData & 0x00FF0000) >>  8) |
			((SourceData & 0xFF000000) >> 24) ;
	}
}

/**
* Helper function used by CookObject - performs movie specific cooking.
*
* @param	TextureMovie	Movie texture to cook
*/
void UCookPackagesCommandlet::CookMovieTexture( UTextureMovie* TextureMovie )
{
	check(TextureMovie);
	if( !TextureMovie->IsTemplate(RF_ClassDefaultObject) )
	{
		if( ShouldByteSwapData() )
		{
			SCOPE_SECONDS_COUNTER(CookMovieTime);

			// load the movie stream data
			void* Buffer = TextureMovie->Data.Lock(LOCK_READ_WRITE);
			INT BufferSize = TextureMovie->Data.GetBulkDataSize();
			UBOOL Result = FALSE;
			if( TextureMovie->DecoderClass == UCodecMovieBink::StaticClass() )
			{
				// check for a correct signature in the movie stream
				BYTE Signature[]={'B','I','K'};
				if( IsMovieSignatureValid(Buffer,BufferSize,Signature,ARRAY_COUNT(Signature)) )
				{
					// byte swap the data
					EnsureMovieEndianess(Buffer,BufferSize);
					Result = TRUE;
				}
			}
			else
			{
				warnf(NAME_Error, TEXT("Codec type [%s] not implemented! Removing movie data."),
					TextureMovie->DecoderClass ? *TextureMovie->DecoderClass->GetName() : TEXT("None"));
			}
			TextureMovie->Data.Unlock();
			if( !Result )
			{
				// invalid movie type so remove its data
				TextureMovie->Data.RemoveBulkData();
			}			
		}
	}
}

/**
 * Helper function used by CookObject - performs sound cue specific cooking.
 */
void UCookPackagesCommandlet::CookSoundCue( USoundCue* SoundCue )
{
	if (SoundCue && SoundCue->FaceFXAnimSetRef)
	{
		if (ScriptReferencedFaceFXAnimSets.Find(SoundCue->FaceFXAnimSetRef->GetPathName()) == NULL)
		{
			// Do not allow the FaceFXAnimSetRef to cook into anything!
			SoundCue->FaceFXAnimSetRef->ClearFlags(RF_ForceTagExp);
			SoundCue->FaceFXAnimSetRef->SetFlags(RF_MarkedByCooker|RF_Transient);
			if (bLogPersistentFaceFXGeneration)
			{
				warnf(NAME_Log, TEXT("CookSoundCue> Cleared FaceFXAnimSet %s for sound cue %s"),
					*(SoundCue->FaceFXAnimSetRef->GetPathName()),
					*(SoundCue->GetPathName()));
			}
		}
		else
		{
			if (bLogPersistentFaceFXGeneration)
			{
				warnf(NAME_Log, TEXT("CookSoundCue> Allowing FaceFXAnimSet %s for sound cue %s"),
					*(SoundCue->FaceFXAnimSetRef->GetPathName()),
					*(SoundCue->GetPathName()));
			}
		}

		if (bGeneratePersistentMapAnimSet && CurrentPMapGroupAnimLookupMap)
		{
			FString AnimSetRefName = SoundCue->FaceFXAnimSetRef->GetPathName();
			FString GroupName = SoundCue->FaceFXGroupName;
			FString AnimName = SoundCue->FaceFXAnimName;
			FMangledFaceFXInfo* MangledFFXInfo = UCookPackagesCommandlet_FindMangledFaceFXInfo(this, 
				CurrentPMapGroupAnimLookupMap, AnimSetRefName, GroupName, AnimName);
			if (MangledFFXInfo)
			{
				// Fix up the names!
				if ((MangledFFXInfo->OriginalFaceFXAnimName == AnimName) &&
					(MangledFFXInfo->OriginalFaceFXGroupName == GroupName))
				{
					if (bLogPersistentFaceFXGeneration)
					{
						warnf(NAME_Log, TEXT("FaceFX mangling on SoundCue %s"), *(SoundCue->GetPathName()));
						warnf(NAME_Log, TEXT("\t%s-%s --> %s-%s"), 
							*GroupName, *AnimName,
							*(MangledFFXInfo->MangledFaceFXGroupName), *(MangledFFXInfo->MangledFaceFXAnimName)
							);
					}
					SoundCue->FaceFXAnimName = MangledFFXInfo->MangledFaceFXAnimName;
					SoundCue->FaceFXGroupName = MangledFFXInfo->MangledFaceFXGroupName;
				}
				else
				{
					warnf(NAME_Warning, TEXT("Mismatched FaceFX mangling on SoundCue %s"), *(SoundCue->GetPathName()));
					warnf(NAME_Warning, TEXT("\tExpected %s-%s, found %s-%s"), 
						*GroupName, *AnimName,
						*(MangledFFXInfo->OriginalFaceFXGroupName), *(MangledFFXInfo->OriginalFaceFXAnimName)
						);
				}
			}
			else
			{
				if (bLogPersistentFaceFXGeneration)
				{
					warnf(NAME_Log, TEXT("CookSoundCue> Did not find %s.%s in PMap list"), *GroupName, *AnimName);
				}
			}
		}
	}
}

/**
 * Helper function used by CookObject - performs InterpTrackFaceFX specific cooking.
 */
void UCookPackagesCommandlet_CookFaceFXInterpTrack(UCookPackagesCommandlet* Commandlet, UInterpTrackFaceFX* FaceFXInterpTrack)
{
	// Are we cooking persistent FaceFX?
	if (Commandlet->bGeneratePersistentMapAnimSet && Commandlet->CurrentPMapGroupAnimLookupMap)
	{
		//FaceFXInterpTrack->UpdateFaceFXSoundCueReferences()
		for (INT SeqIndex = 0; SeqIndex < FaceFXInterpTrack->FaceFXSeqs.Num(); SeqIndex++)
		{
			FFaceFXTrackKey& TrackKey = FaceFXInterpTrack->FaceFXSeqs(SeqIndex);
			FFaceFXSoundCueKey* SoundKey = (FaceFXInterpTrack->FaceFXSoundCueKeys.Num() < SeqIndex) ? &(FaceFXInterpTrack->FaceFXSoundCueKeys(SeqIndex)) : NULL;

			FString GroupName = TrackKey.FaceFXGroupName;
			FString AnimName = TrackKey.FaceFXSeqName;

			UBOOL bFound = FALSE;

			INT SetIndex = UCookPackagesCommandlet_FindSourceAnimSet(Commandlet, FaceFXInterpTrack->FaceFXAnimSets, GroupName, AnimName);
			if (SetIndex != -1)
			{
				UFaceFXAnimSet* TheAnimSet = FaceFXInterpTrack->FaceFXAnimSets(SetIndex);
				if (Commandlet->ScriptReferencedFaceFXAnimSets.Find(TheAnimSet->GetPathName()) == NULL)
				{
					TheAnimSet->ClearFlags(RF_ForceTagExp);
					TheAnimSet->SetFlags(RF_MarkedByCooker|RF_Transient);
					if (Commandlet->bLogPersistentFaceFXGeneration)
					{
						warnf(NAME_Log, TEXT("CookFaceFXInterpTrack> Cleared FaceFXAnimSet %s for %s"),
							*(TheAnimSet->GetPathName()),
							*(FaceFXInterpTrack->GetPathName()));
					}

					FString AnimSetRefName = TheAnimSet->GetPathName();
					UCookPackagesCommandlet::FMangledFaceFXInfo* MangledFFXInfo = UCookPackagesCommandlet_FindMangledFaceFXInfo(
						Commandlet, Commandlet->CurrentPMapGroupAnimLookupMap, AnimSetRefName, GroupName, AnimName);
					if (MangledFFXInfo)
					{
						// Fix up the names!
						if ((MangledFFXInfo->OriginalFaceFXAnimName == AnimName) &&
							(MangledFFXInfo->OriginalFaceFXGroupName == GroupName))
						{
							if (Commandlet->bLogPersistentFaceFXGeneration)
							{
								warnf(NAME_Log, TEXT("FaceFX mangling on InterTrack %d-%s"), SeqIndex, *(FaceFXInterpTrack->GetPathName()));
								warnf(NAME_Log, TEXT("\t%s-%s --> %s-%s"), 
									*(TrackKey.FaceFXGroupName), *(TrackKey.FaceFXSeqName),
									*(MangledFFXInfo->MangledFaceFXGroupName), *(MangledFFXInfo->MangledFaceFXAnimName)
									);
							}
							TrackKey.FaceFXGroupName = MangledFFXInfo->MangledFaceFXGroupName;
							TrackKey.FaceFXSeqName = MangledFFXInfo->MangledFaceFXAnimName;
						}
						else
						{
							warnf(NAME_Warning, TEXT("Mismatched FaceFX mangling on InterTrack %d-%s"), SeqIndex, *(FaceFXInterpTrack->GetPathName()));
							warnf(NAME_Warning, TEXT("\tExpected %s-%s, found %s-%s"), 
								*(TrackKey.FaceFXGroupName), *(TrackKey.FaceFXSeqName),
								*(MangledFFXInfo->OriginalFaceFXGroupName), *(MangledFFXInfo->OriginalFaceFXAnimName)
								);
						}
					}
					else
					{
						if (Commandlet->bLogPersistentFaceFXGeneration)
						{
							warnf(NAME_Log, TEXT("CookFaceFXInterpTrack> Did not find %s.%s in PMap list"), *GroupName, *AnimName);
						}
					}
				}
				else
				{
					if (Commandlet->bLogPersistentFaceFXGeneration)
					{
						warnf(NAME_Log, TEXT("CookInterpTrackFaceFX> Allowing FaceFXAnimSet %s for %s"),
							*(TheAnimSet->GetPathName()),
							*(FaceFXInterpTrack->GetPathName()));
					}
				}
			}
		}

		// Catch unused FaceFXAnimSets...
		for (INT AnimSetIndex = 0; AnimSetIndex < FaceFXInterpTrack->FaceFXAnimSets.Num(); AnimSetIndex++)
		{
			UFaceFXAnimSet* TheAnimSet = FaceFXInterpTrack->FaceFXAnimSets(AnimSetIndex);
			if (Commandlet->ScriptReferencedFaceFXAnimSets.Find(TheAnimSet->GetPathName()) == NULL)
			{
				TheAnimSet->ClearFlags(RF_ForceTagExp);
				TheAnimSet->SetFlags(RF_MarkedByCooker|RF_Transient);
				if (Commandlet->bLogPersistentFaceFXGeneration)
				{
					warnf(NAME_Log, TEXT("CookFaceFXInterpTrack> Cleared FaceFXAnimSet %s for %s"),
						*(TheAnimSet->GetPathName()),
						*(FaceFXInterpTrack->GetPathName()));
				}
			}
		}
	}
}

/**
 * Helper function used by CookObject - performs USeqAct_PlayFaceFXAnim specific cooking.
 */
void UCookPackagesCommandlet_CookSeqActPlayFaceFXAnim(UCookPackagesCommandlet* Commandlet, USeqAct_PlayFaceFXAnim* SeqAct_PlayFaceFX)
{
	if (Commandlet->bGeneratePersistentMapAnimSet && Commandlet->CurrentPMapGroupAnimLookupMap)
	{
		if (SeqAct_PlayFaceFX && SeqAct_PlayFaceFX->FaceFXAnimSetRef)
		{
			if (Commandlet->ScriptReferencedFaceFXAnimSets.Find(SeqAct_PlayFaceFX->FaceFXAnimSetRef->GetPathName()) == NULL)
			{
				// Do not allow the FaceFXAnimSetRef to cook into anything!
				SeqAct_PlayFaceFX->FaceFXAnimSetRef->ClearFlags(RF_ForceTagExp);
				SeqAct_PlayFaceFX->FaceFXAnimSetRef->SetFlags(RF_MarkedByCooker|RF_Transient);
				if (Commandlet->bLogPersistentFaceFXGeneration)
				{
					warnf(NAME_Log, TEXT("CookSeqActPlayFaceFXAnim> Cleared FaceFXAnimSet %s for %s"),
						*(SeqAct_PlayFaceFX->FaceFXAnimSetRef->GetPathName()),
						*(SeqAct_PlayFaceFX->GetPathName()));
				}
			}
			else
			{
				if (Commandlet->bLogPersistentFaceFXGeneration)
				{
					warnf(NAME_Log, TEXT("CookSeqActPlayFaceFXAnim> Allowing FaceFXAnimSet %s for %s"),
						*(SeqAct_PlayFaceFX->FaceFXAnimSetRef->GetPathName()),
						*(SeqAct_PlayFaceFX->GetPathName()));
				}
			}

			FString AnimSetRefName = SeqAct_PlayFaceFX->FaceFXAnimSetRef->GetPathName();
			FString GroupName = SeqAct_PlayFaceFX->FaceFXGroupName;
			FString AnimName = SeqAct_PlayFaceFX->FaceFXAnimName;
			UCookPackagesCommandlet::FMangledFaceFXInfo* MangledFFXInfo = UCookPackagesCommandlet_FindMangledFaceFXInfo(Commandlet, 
				Commandlet->CurrentPMapGroupAnimLookupMap, AnimSetRefName, GroupName, AnimName);
			if (MangledFFXInfo)
			{
				// Fix up the names!
				if ((MangledFFXInfo->OriginalFaceFXAnimName == AnimName) &&
					(MangledFFXInfo->OriginalFaceFXGroupName == GroupName))
				{
					if (Commandlet->bLogPersistentFaceFXGeneration)
					{
						warnf(NAME_Log, TEXT("FaceFX mangling on SeqActPlayFaceFXAnim %s"), *(SeqAct_PlayFaceFX->GetPathName()));
						warnf(NAME_Log, TEXT("\t%s-%s --> %s-%s"), 
							*GroupName, *AnimName,
							*(MangledFFXInfo->MangledFaceFXGroupName), *(MangledFFXInfo->MangledFaceFXAnimName)
							);
					}
					SeqAct_PlayFaceFX->FaceFXAnimName = MangledFFXInfo->MangledFaceFXAnimName;
					SeqAct_PlayFaceFX->FaceFXGroupName = MangledFFXInfo->MangledFaceFXGroupName;
				}
				else
				{
					warnf(NAME_Warning, TEXT("Mismatched FaceFX mangling on SeqActPlayFaceFXAnim %s"), *(SeqAct_PlayFaceFX->GetPathName()));
					warnf(NAME_Warning, TEXT("\tExpected %s-%s, found %s-%s"), 
						*GroupName, *AnimName,
						*(MangledFFXInfo->OriginalFaceFXGroupName), *(MangledFFXInfo->OriginalFaceFXAnimName)
						);
				}
			}
			else
			{
				if (Commandlet->bLogPersistentFaceFXGeneration)
				{
					warnf(NAME_Log, TEXT("CookSeqActPlayFaceFXAnim> Did not find %s.%s in PMap list"), *GroupName, *AnimName);
				}
			}
		}
	}
}

/**
 * Helper function used by CookSoundNodeWave - performs sound specific cooking.
 */
void UCookPackagesCommandlet::CookSoundNodeWave( USoundNodeWave* SoundNodeWave )
{
	SCOPE_SECONDS_COUNTER( CookSoundTime );

	// Ensure the current platforms are cooked
	::CookSoundNodeWave( SoundNodeWave, Platform );
}

/**
 * Helper function used by CookSoundNodeWave - localises sound
 */
void UCookPackagesCommandlet::LocSoundNodeWave( USoundNodeWave* SoundNodeWave )
{
	SCOPE_SECONDS_COUNTER( LocSoundTime );

	// Setup the localization if needed
	if (SoundNodeWave->IsLocalizedResource())
	{
		TCHAR SavedLangExt[64];
		appStrcpy(SavedLangExt, 64, UObject::GetLanguage());
		INT SavedIndex = Localization_GetLanguageExtensionIndex(SavedLangExt);

		SoundNodeWave->LocalizedSubtitles.Empty();
		SoundNodeWave->LocalizedSubtitles.AddZeroed(GKnownLanguageCount);

		if (bCookCurrentLanguageOnly == FALSE)
		{
			for (INT KnownLangIndex = 0; GKnownLanguageExtensions[KnownLangIndex]; KnownLangIndex++)
			{
				const TCHAR* Ext = GKnownLanguageExtensions[KnownLangIndex];

				// mark each coalesced loc file for sha
				INT LangIndex = Localization_GetLanguageExtensionIndex(Ext);
				if ((LangIndex != -1) && (SavedIndex != LangIndex))
				{
					checkf((LangIndex < GKnownLanguageCount), TEXT("GKnownLanguageCount is not set correctly!"));

					// Fill in the items...
					FLocalizedSubtitle& LocSubtitle = SoundNodeWave->LocalizedSubtitles(LangIndex);
					UObject::SetLanguage(Ext, FALSE);
					SoundNodeWave->LoadLocalized(NULL, FALSE);
					// Copy it into the appropriate slot
					LocSubtitle.bManualWordWrap = SoundNodeWave->bManualWordWrap;
					LocSubtitle.bMature = SoundNodeWave->bMature;
					LocSubtitle.Subtitles = SoundNodeWave->Subtitles;
				}
			}
		}

		// Restore the expected language, and localize the SoundNodeWave for that as well.
		// This will result in the default being the language being cooked for.
		UObject::SetLanguage(SavedLangExt, FALSE);
		// Fill in the items...
		FLocalizedSubtitle& LocSubtitle = SoundNodeWave->LocalizedSubtitles(SavedIndex);
		SoundNodeWave->LoadLocalized(NULL, FALSE);
		// Copy it into the appropriate slot
		LocSubtitle.bManualWordWrap = SoundNodeWave->bManualWordWrap;
		LocSubtitle.bMature = SoundNodeWave->bMature;
		LocSubtitle.Subtitles = SoundNodeWave->Subtitles;
	}
}

/**
 * Helper function used by CookObject - performs ParticleSystem specific cooking.
 *
 * @param	ParticleSystem	ParticleSystem to cook
 */
void UCookPackagesCommandlet::CookParticleSystem(UParticleSystem* ParticleSystem)
{
	check(ParticleSystem);

	// Cook out the thumbnail image - no reason to store it for gameplay.
	ParticleSystem->ThumbnailImageOutOfDate = TRUE;
	if ( ParticleSystem->ThumbnailImage != NULL )
	{
		ParticleSystem->ThumbnailImage->SetFlags(RF_NotForClient|RF_NotForServer);
		// clear ForceTagExp so that it won't get PrepareForSaving called on it
		ParticleSystem->ThumbnailImage->ClearFlags(RF_Standalone|RF_ForceTagExp);	
	}
	ParticleSystem->ThumbnailImage = NULL;

	// Examine each emitter, iterating in reverse as we are removing entries.
	for( INT EmitterIndex=ParticleSystem->Emitters.Num()-1; EmitterIndex>= 0; EmitterIndex-- )
	{
		UParticleEmitter* Emitter = ParticleSystem->Emitters(EmitterIndex);
		// Default to remove the entry, disabled if the emitter is enabled in an LOD level.
		UBOOL bShouldRemoveEntry = TRUE;
		
		// Iterate over LOD levels to see whether it is enabled in any.
		if( Emitter )
		{
			for( INT LODLevelIndex=0; LODLevelIndex<Emitter->LODLevels.Num(); LODLevelIndex++ )
			{
				// Check required module to see whether emitter is enabled in this LOD level.
				UParticleLODLevel* LODLevel = Emitter->LODLevels(LODLevelIndex);
				if( LODLevel )
				{
					check(LODLevel->RequiredModule);
					// We don't remove the entry if it's enabled in one, set flag and break out of loop.
					if( LODLevel->bEnabled )
					{
						bShouldRemoveEntry = FALSE;
						break;
					}
				}
			}
		}

		// Remove current entry as it's not used.
		if( bShouldRemoveEntry )
		{
			// We're iterating in reverse so we can remove current index without decrementing.
			ParticleSystem->Emitters.Remove(EmitterIndex);
		}
	}
}

/**
 * Helper function used by CookSkeletalMesh - performs SkeletalMesh specific cooking.
 */
void UCookPackagesCommandlet::CookSkeletalMesh( USkeletalMesh* SkeletalMesh )
{
	SCOPE_SECONDS_COUNTER(CookSkeletalMeshTime);
}

/**
 * Helper function used by CookStaticMesh - performs StaticMesh specific cooking.
 */
void UCookPackagesCommandlet::CookStaticMesh( UStaticMesh* StaticMesh )
{
	SCOPE_SECONDS_COUNTER(CookStaticMeshTime);

	// this is only needed on PS3, but it could be called for Xbox
	if (Platform == PLATFORM_PS3)
	{
		StaticMeshCooker->Init();

		// loop through each LOD model
		for (INT nLOD=0; nLOD<StaticMesh->LODModels.Num(); nLOD++)
		{
			// loop through each section of the particular LOD
			for (INT nElement=0; nElement<StaticMesh->LODModels(nLOD).Elements.Num(); nElement++)
			{
				FRawStaticIndexBuffer&	indexBuf = StaticMesh->LODModels(nLOD).IndexBuffer;
				INT						numTrisPerElement = StaticMesh->LODModels(nLOD).Elements(nElement).NumTriangles;
				DWORD					baseIndex = StaticMesh->LODModels(nLOD).Elements(nElement).FirstIndex;
				WORD*					outputTriangleList = (WORD*)appMalloc(sizeof(WORD)*numTrisPerElement*3);

				if (outputTriangleList)
				{
					FMeshElementCookInfo MeshElementCookInfo;
					MeshElementCookInfo.Indices = &indexBuf.Indices(baseIndex);
					MeshElementCookInfo.NumTriangles = numTrisPerElement;
					StaticMeshCooker->CookMeshElement(MeshElementCookInfo, outputTriangleList);

					// replace the current section of indices with the newly reordered indices
					// note that reordering the indices currently breaks shadow volumes as shadow volume data is not reordered as well
					for (INT i=0; i<numTrisPerElement*3; i++)
					{
						indexBuf.Indices(baseIndex + i) = *(outputTriangleList+i);
					}
					appFree(outputTriangleList);
				}
			}
		}
	}
}

/**
 * Creates an instance of a StaticMeshCollectorActor.  If a valid World is specified, uses SpawnActor;
 * otherwise, uses ConstructObject.
 *
 * @param	Package		the package to create the new actor in
 * @param	World		if Package corresponds to a map package, the reference to the UWorld object for the map.
 */
namespace
{
	template< class T >
	T* CreateComponentCollector( UPackage* Package, UWorld* World )
	{
		T* Result = NULL;
		
		if ( Package != NULL )
		{
			if ( World != NULL && World->PersistentLevel != NULL )
			{
				Result = Cast<T>(World->SpawnActor(T::StaticClass()));
			}
			else
			{
				Result = ConstructObject<T>(T::StaticClass(), Package);
			}

			if ( Result != NULL )
			{
				Result->SetFlags(RF_Cooked);
			}
		}

		return Result;
	}
};

/**
 * Cooks out all static mesh actors in the specified package by re-attaching their StaticMeshComponents to
 * a StaticMeshCollectionActor referenced by the world.
 *
 * @param	Package		the package being cooked
 */
void UCookPackagesCommandlet::CookStaticMeshActors( UPackage* Package )
{
	// 'Cook-out' material expressions on consoles
	// In this case, simply don't load them on the client
	// Keep 'parameter' types around so that we can get their defaults.
	if ( (GCookingTarget & PLATFORM_Console) != 0 )
	{
		// only cook-out StaticMeshActors when cooking for console
		check(Package);

		// find all StaticMeshActors and static Light actors which are referenced by something in the map
		UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
		if ( World != NULL )
		{
			TArray<ULevel*> LevelsToSearch = World->Levels;

			// make sure that the world's PersistentLevel is part of the levels array for the purpose of this test.
			if ( World->PersistentLevel != NULL )
			{
				LevelsToSearch.AddUniqueItem(World->PersistentLevel);
			}
			for ( INT LevelIndex = 0; LevelIndex < LevelsToSearch.Num(); LevelIndex++ )
			{
				ULevel* Level = LevelsToSearch(LevelIndex);

				// we need to remove all StaticMeshActors and static Light actors from the level's Actors array so that we don't
				// get false positives during our reference checking.
				// however, we'll need to restore any actors which don't get cooked out, so keep track of their indices
				TMap<AStaticMeshActor*,INT> StaticActorIndices;
				TLookupMap<AStaticMeshActor*> StaticMeshActors;

				// remove all StaticMeshActors from the level's Actor array so that we don't get false positives.
				for ( INT ActorIndex = 0; ActorIndex < Level->Actors.Num(); ActorIndex++ )
				{
					AActor* Actor = Level->Actors(ActorIndex);
					if ( Actor != NULL && Actor->IsA(AStaticMeshActor::StaticClass()) )
					{
						AStaticMeshActor* StaticMeshActor = static_cast<AStaticMeshActor*>(Actor);

						StaticMeshActors.AddItem(StaticMeshActor);
						StaticActorIndices.Set(StaticMeshActor, ActorIndex);
						Level->Actors(ActorIndex) = NULL;
					}
				}

				// now use the object reference collector to find the static mesh actors that are still being referenced
				TArray<AStaticMeshActor*> ReferencedStaticMeshActors;
				TArchiveObjectReferenceCollector<AStaticMeshActor> SMACollector(&ReferencedStaticMeshActors, Package, FALSE, TRUE, TRUE, TRUE);
				Level->Serialize( SMACollector );

				// remove any StaticMeshActors which aren't valid for cooking out
				TFindObjectReferencers<AStaticMeshActor> StaticMeshReferencers(ReferencedStaticMeshActors, Package);
				for ( INT ActorIndex = ReferencedStaticMeshActors.Num() - 1; ActorIndex >= 0; ActorIndex-- )
				{
					AStaticMeshActor* StaticMeshActor = ReferencedStaticMeshActors(ActorIndex);
					UStaticMeshComponent* Component = StaticMeshActor->StaticMeshComponent;

					// for now, we'll ignore StaticMeshActors that are archetypes or instances of archetypes.
					if ( Component == NULL || Component->IsTemplate(RF_ArchetypeObject) || Component->GetArchetype()->IsTemplate(RF_ArchetypeObject) )
					{
						ReferencedStaticMeshActors.Remove(ActorIndex);
					}
					else
					{
						TArray<UObject*> Referencers;
						StaticMeshReferencers.MultiFind(StaticMeshActor, Referencers);
						for ( INT ReferencerIndex = Referencers.Num() - 1; ReferencerIndex >= 0; ReferencerIndex-- )
						{
							UObject* Referencer = Referencers(ReferencerIndex);
							if ( Referencer == StaticMeshActor->GetLevel() )
							{
								// if this is the actor's level, ignore this reference
								Referencers.Remove(ReferencerIndex);
							}
							else if ( Referencer == StaticMeshActor->StaticMeshComponent )
							{
								// if the referencer is the StaticMeshActor's StaticMeshComponent, we can ignore this reference as this means that
								// something else in the level is referencing the StaticMeshComponent (which will still be around even if we cook
								// out the StaticMeshActor)
								Referencers.Remove(ReferencerIndex);
							}
							else if ( Referencer->IsA(ANavigationPoint::StaticClass())
							&&	(static_cast<ANavigationPoint*>(Referencer)->Base == StaticMeshActor) )
							{
								// If the actor that's based on me is an interp actor, then we need to preserve this
								// reference, since Matinee movement track data is stored relative to the object's
								// coordinate system.  If we change the coordinate system of the object by debasing it,
								// the Matinee data will be positioned incorrectly.  This is super important for
								// objects like Turrets!
								if( static_cast<AActor*>(Referencer)->Physics != PHYS_Interpolating )
								{
									// if this actor references the StaticMeshActor because it's based on the static mesh actor, ignore this reference
									Referencers.Remove(ReferencerIndex);
								}
							}
						}

						// if this StaticMeshActor is still referenced by something, do not cook it out
						if ( Referencers.Num() == 0 )
						{
							ReferencedStaticMeshActors.Remove(ActorIndex);
						}
					}
				}

				// remove the referenced static mesh actors from the list of actors to be cooked-out.
				for ( INT ActorIndex = 0; ActorIndex < ReferencedStaticMeshActors.Num(); ActorIndex++ )
				{
					StaticMeshActors.RemoveItem(ReferencedStaticMeshActors(ActorIndex));
				}

				AStaticMeshCollectionActor* MeshCollector = NULL;
				for ( INT ActorIndex = 0; ActorIndex < StaticMeshActors.Num(); ActorIndex++ )
				{
					AStaticMeshActor* StaticMeshActor = StaticMeshActors(ActorIndex);
					UStaticMeshComponent* Component = StaticMeshActor->StaticMeshComponent;

					// SMAs without a SMC should be removed from maps. There already is a map warning for this so we simply silently handle this case.
					if( !Component )
					{
						continue;
					}

					// Detect static mesh components that have a transform that can't be represented by a single scale/rotation/translation.
					const UBOOL bActorHasScale =
						StaticMeshActor->DrawScale != 1.0f ||
						StaticMeshActor->DrawScale3D != FVector(1.0f,1.0f,1.0f);
					const UBOOL bComponentHasRotation = !Component->Rotation.IsZero();
					const UBOOL bComponentHasTranslation = !Component->Translation.IsZero();
					if(bActorHasScale && (bComponentHasRotation || bComponentHasTranslation))
					{
						continue;
					}

					// if there is a limit to the number of components that can be added to a StaticMeshCollectorActor, create a new
					// one if we have reached the limit
					if (MeshCollector == NULL
					|| (MeshCollector->MaxStaticMeshComponents > 0
					&&  MeshCollector->StaticMeshComponents.Num() >= MeshCollector->MaxStaticMeshComponents) )
					{
						MeshCollector = CreateComponentCollector<AStaticMeshCollectionActor>(Package, World);
					}

					// UPrimitiveComponent::Detach() will clear the ShadowParent but it will never be restored, so save the reference and restore it later
					UPrimitiveComponent* ComponentShadowParent = Component->ShadowParent;

					// remove it from the StaticMeshActor.
					StaticMeshActor->DetachComponent(Component);

					// rather than duplicating the StaticMeshComponent into the mesh collector, rename the component into the collector
					// so that we don't lose any outside references to this component (@todo ronp - are external references to components even allowed?)
					const UBOOL bWasPublic = Component->HasAnyFlags(RF_Public);

					// clear the RF_Public flag so that we don't create a redirector
					Component->ClearFlags(RF_Public);

					// since we're renaming multiple components into the same Outer, it's likely that we'll end up with a name
					// conflict.  unfortunately, for the script patcher these components need to have deterministic names, so
					// create a mangled name using the actor and some tag
					const FString OriginalComponentName = Component->GetName();
					const FString NewComponentName = *(StaticMeshActor->GetName() + TEXT("_SMC"));
					if (Component->Rename(*NewComponentName, MeshCollector, REN_ForceNoResetLoaders | REN_KeepNetIndex | REN_Test))
					{
						Component->Rename(*NewComponentName, MeshCollector, REN_ForceNoResetLoaders | REN_KeepNetIndex);
					}
					else
					{
						Component->Rename(NULL, MeshCollector, REN_ForceNoResetLoaders | REN_KeepNetIndex);
					}

					if ( bWasPublic )
					{
						Component->SetFlags(RF_Public);
					}

					// now add it to the mesh collector's StaticMeshComponents array
					MeshCollector->StaticMeshComponents.AddItem(Component);

					// it must also exist in the AllComponents array so that the component's physics data can be cooked
					MeshCollector->AllComponents.AddItem(Component);

					// copy any properties which are usually pulled from the Owner at runtime
					Component->ShadowParent = ComponentShadowParent;
					Component->CollideActors = StaticMeshActor->bCollideActors && Component->CollideActors;
					Component->HiddenGame = Component->HiddenGame || (!Component->bCastHiddenShadow && StaticMeshActor->bHidden);
					Component->HiddenEditor = Component->HiddenEditor || StaticMeshActor->bHiddenEd;

					// Since UPrimitiveComponent::SetTransformedToWorld() generates a matrix which includes the
					// component's Scale/Scale3D then multiplies that by the actor's LocalToWorld to get the final
					// transform, the component's Scale/Scale3D must include the actor's scale and the LocalToWorld
					// matrix we serialize for the component must NOT include scaling.  This is necessary so that if
					// the component has a value for Scale/Scale3D it is preserved without applying the actor's scaling
					// twice for all cases.
					FMatrix ActorLocalToWorld = StaticMeshActor->LocalToWorld();

					FVector ActorScale3D = StaticMeshActor->DrawScale * StaticMeshActor->DrawScale3D;
					Component->Scale3D *= ActorScale3D;
					
					FVector RecipScale( 1.f/ActorScale3D.X, 1.f/ActorScale3D.Y, 1.f/ActorScale3D.Z );

					ActorLocalToWorld.M[0][0] *= RecipScale.X;
					ActorLocalToWorld.M[0][1] *= RecipScale.X;
					ActorLocalToWorld.M[0][2] *= RecipScale.X;

					ActorLocalToWorld.M[1][0] *= RecipScale.Y;
					ActorLocalToWorld.M[1][1] *= RecipScale.Y;
					ActorLocalToWorld.M[1][2] *= RecipScale.Y;

					ActorLocalToWorld.M[2][0] *= RecipScale.Z;
					ActorLocalToWorld.M[2][1] *= RecipScale.Z;
					ActorLocalToWorld.M[2][2] *= RecipScale.Z;

					Component->ConditionalUpdateTransform(ActorLocalToWorld);

					// now mark the StaticMeshActor with no-load flags so that it will disappear on save
					StaticMeshActor->SetFlags(RF_NotForClient|RF_NotForServer|RF_NotForEdit);
					for ( INT CompIndex = 0; CompIndex < StaticMeshActor->Components.Num(); CompIndex++ )
					{
						if ( StaticMeshActor->Components(CompIndex) != NULL )
						{
							StaticMeshActor->Components(CompIndex)->SetFlags(RF_NotForClient|RF_NotForServer|RF_NotForEdit);
						}
					}

					debugf(NAME_DevCooking, TEXT("Cooking out StaticMeshActor %s; re-attaching %s to %s as %s"),
						*StaticMeshActor->GetName(), *OriginalComponentName, *MeshCollector->GetName(), *Component->GetName());

					StaticActorIndices.Remove(StaticMeshActor);
				}

				// finally, restore the entries in the level's Actors array for the StaticMeshActors not being cooked out
				for ( TMap<AStaticMeshActor*,INT>::TIterator It(StaticActorIndices); It; ++It )
				{
					INT ActorIndex = It.Value();

					// make sure nothing filled in this entry in the array
					checkSlow(Level->Actors(ActorIndex)==NULL);
					Level->Actors(ActorIndex) = It.Key();
				}
#if _REMOVE_EMPTY_STATIC_ACTORS_
				INT TotalCount = Level->Actors.Num();
				INT RemoveCount = 0;
				for (INT RemoveIndex = Level->Actors.Num() - 1; RemoveIndex >= 0; RemoveIndex--)
				{
					if (Level->Actors(RemoveIndex) == NULL)
					{
						Level->Actors.Remove(RemoveIndex);
						RemoveCount++;
					}
				}
				warnf(TEXT("Purged %5d NULL actors (out of %6d) from Level->Actors in %s"), RemoveCount, TotalCount, *(Level->GetName()));
#endif	//#if _REMOVE_EMPTY_STATIC_ACTORS_
			}
		}
	}
}

/**
 * Cooks out all static Light actors in the specified package by re-attaching their LightComponents to a 
 * StaticLightCollectionActor referenced by the world.
 */
void UCookPackagesCommandlet::CookStaticLightActors( UPackage* Package )
{
	if ( (GCookingTarget & PLATFORM_Console) != 0 )
	{
		// only cook-out static Lights when cooking for console
		check(Package);

		// find all StaticMeshActors and static Light actors which are referenced by something in the map
		UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
		if ( World != NULL )
		{
			TArray<ULevel*> LevelsToSearch = World->Levels;

			// make sure that the world's PersistentLevel is part of the levels array for the purpose of this test.
			if ( World->PersistentLevel != NULL )
			{
				LevelsToSearch.AddUniqueItem(World->PersistentLevel);
			}
			for ( INT LevelIndex = 0; LevelIndex < LevelsToSearch.Num(); LevelIndex++ )
			{
				ULevel* Level = LevelsToSearch(LevelIndex);

				// we need to remove all static Light actors from the level's Actors array so that we don't
				// get false positives during our reference checking.
				// however, we'll need to restore any actors which don't get cooked out, so keep track of their indices
				TMap<ALight*,INT> StaticActorIndices;
				TLookupMap<ALight*> StaticLightActors;

				// remove all StaticMeshActors from the level's Actor array so that we don't get false positives.
				for ( INT ActorIndex = 0; ActorIndex < Level->Actors.Num(); ActorIndex++ )
				{
					AActor* Actor = Level->Actors(ActorIndex);
					if ( Actor != NULL && Actor->IsA(ALight::StaticClass()) && Actor->bStatic )
					{
						ALight* Light = static_cast<ALight*>(Actor);

						StaticLightActors.AddItem(Light);
						StaticActorIndices.Set(Light, ActorIndex);
						Level->Actors(ActorIndex) = NULL;
					}
				}

				// now use the object reference collector to find the static mesh actors that are still being referenced
				TArray<ALight*> ReferencedStaticLightActors;
				{
					TArchiveObjectReferenceCollector<ALight> LightCollector(&ReferencedStaticLightActors, Package, FALSE, TRUE, TRUE, TRUE);
					Level->Serialize( LightCollector );
				}

				// remove any static light actors which aren't valid for cooking out
				for ( INT ActorIndex = ReferencedStaticLightActors.Num() - 1; ActorIndex >= 0; ActorIndex-- )
				{
					ALight* Light = ReferencedStaticLightActors(ActorIndex);
					if ( Light->bStatic )
					{
						ULightComponent* Component = ReferencedStaticLightActors(ActorIndex)->LightComponent;

						// for now, we'll ignore static Lights that are archetypes or instances of archetypes.
						if ( Component != NULL
						&&	(Component->IsTemplate(RF_ArchetypeObject) || Component->GetArchetype()->IsTemplate(RF_ArchetypeObject)) )
						{
							ReferencedStaticLightActors.Remove(ActorIndex);
						}
					}
					else
					{
						ReferencedStaticLightActors.Remove(ActorIndex);
					}
				}

				TFindObjectReferencers<ALight> StaticLightReferencers(ReferencedStaticLightActors, Package);
				for ( INT ActorIndex = ReferencedStaticLightActors.Num() - 1; ActorIndex >= 0; ActorIndex-- )
				{
					ALight* LightActor = ReferencedStaticLightActors(ActorIndex);

					TArray<UObject*> Referencers;
					StaticLightReferencers.MultiFind(LightActor, Referencers);
					for ( INT ReferencerIndex = Referencers.Num() - 1; ReferencerIndex >= 0; ReferencerIndex-- )
					{
						UObject* Referencer = Referencers(ReferencerIndex);
						if ( Referencer == LightActor->GetLevel() )
						{
							// if this is the actor's level, ignore this reference
							Referencers.Remove(ReferencerIndex);
						}
						else if ( Referencer->IsIn(LightActor) && Referencer->IsA(UComponent::StaticClass()) )
						{
							// if the referencer is one of the LightActor's components, we can ignore this reference as this means that
							// something else in the level is referencing the component directly (which will still be around even if we cook
							// out the Light actor)
							Referencers.Remove(ReferencerIndex);
						}
					}

					// if this actor is still referenced by something, do not cook it out
					if ( Referencers.Num() == 0 )
					{
						ReferencedStaticLightActors.Remove(ActorIndex);
					}
				}

				for ( INT ActorIndex = 0; ActorIndex < ReferencedStaticLightActors.Num(); ActorIndex++ )
				{
					StaticLightActors.RemoveItem(ReferencedStaticLightActors(ActorIndex));
				}

				AStaticLightCollectionActor* LightCollector = NULL;
				for ( INT ActorIndex = 0; ActorIndex < StaticLightActors.Num(); ActorIndex++ )
				{
					ALight* LightActor = StaticLightActors(ActorIndex);
					ULightComponent* Component = LightActor->LightComponent;

					// Light actors without a light component should be removed from maps. There already is a map warning for this so we simply silently handle this case.
					if( !Component )
					{
						continue;
					}

					// if there is a limit to the number of components that can be added to a StaticLightCollectorActor, create a new
					// one if we have reached the limit
					if (LightCollector == NULL
					|| (LightCollector->MaxLightComponents > 0
					&&  LightCollector->LightComponents.Num() >= LightCollector->MaxLightComponents) )
					{
						LightCollector = CreateComponentCollector<AStaticLightCollectionActor>(Package, World);
					}

					// remove it from the Light actor.
					LightActor->DetachComponent(Component);

					// rather than duplicating the LightComponent into the light collector, rename the component into the collector
					// so that we don't lose any outside references to this component (@todo ronp - are external references to components even allowed?)
					const UBOOL bWasPublic = Component->HasAnyFlags(RF_Public);

					// clear the RF_Public flag so that we don't create a redirector
					Component->ClearFlags(RF_Public);

					// since we're renaming multiple components into the same Outer, it's likely that we'll end up with a name
					// conflict.  unfortunately, for the script patcher these components need to have deterministic names, so
					// create a mangled name using the actor and some tag
					const FString OriginalComponentName = Component->GetName();
					const FString NewComponentName = *(LightActor->GetName() + TEXT("_LC"));
					if (Component->Rename(*NewComponentName, LightCollector, REN_ForceNoResetLoaders | REN_KeepNetIndex | REN_Test))
					{
						Component->Rename(*NewComponentName, LightCollector, REN_ForceNoResetLoaders | REN_KeepNetIndex);
					}
					else
					{
						Component->Rename(NULL, LightCollector, REN_ForceNoResetLoaders | REN_KeepNetIndex);
					}

					if ( bWasPublic )
					{
						Component->SetFlags(RF_Public);
					}

					// now add it to the light collector's LightComponents array
					LightCollector->LightComponents.AddItem(Component);

					// it must also exist in the AllComponents array so that the component's physics data can be cooked
					LightCollector->AllComponents.AddItem(Component);

					// copy any properties which are usually pulled from the Owner at runtime
					// @todo


					// set the component's LightToWorld while we still have a reference to the original owning Light actor.  This matrix
					// will be serialized to disk by the LightCollector

					Component->LightToWorld = LightActor->LocalToWorld();

					// now mark the Light actor with no-load flags so that it will disappear on save
					LightActor->SetFlags(RF_NotForClient|RF_NotForServer|RF_NotForEdit);
					for ( INT CompIndex = 0; CompIndex < LightActor->Components.Num(); CompIndex++ )
					{
						if ( LightActor->Components(CompIndex) != NULL )
						{
							LightActor->Components(CompIndex)->SetFlags(RF_NotForClient|RF_NotForServer|RF_NotForEdit);
						}
					}

					debugf(NAME_DevCooking, TEXT("Cooking out %s %s; re-attaching %s to %s as %s"),
						*LightActor->GetClass()->GetName(), *LightActor->GetName(), *OriginalComponentName, *LightCollector->GetName(), *Component->GetName());

					StaticActorIndices.Remove(LightActor);
				}


				// finally, restore the entries in the level's Actors array for the static Lights not being cooked out
				for ( TMap<ALight*,INT>::TIterator It(StaticActorIndices); It; ++It )
				{
					INT ActorIndex = It.Value();

					// make sure nothing filled in this entry in the array
					checkSlow(Level->Actors(ActorIndex)==NULL);
					Level->Actors(ActorIndex) = It.Key();
				}
			}
		}
	}
}

/**
 *	Clean up the kismet for the given level...
 *	Remove 'danglers' - sequences that don't actually hook up to anything, etc.
 *
 *	@param	Package		The map being cooked
 */
void UCookPackagesCommandlet::CleanupKismet(UPackage* Package)
{
	check(Package);

	// Find the UWorld object (only valid if we're looking at a map).
	UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
	if (World == NULL)
	{
		debugf(TEXT("CleanupKismet called on non-map package %s"), *(Package->GetName()));
		return;
	}

	UnreferencedMatineeData.Empty();

	// Find dangling InterpData
	TArray<USequence*> FoundSequences;
	TArray<USequenceAction*> FoundActions;
	for (FObjectIterator It; It; ++It)
	{
		UObject* Object = *It;

		if (Object->IsIn(Package) || Object->HasAnyFlags(RF_ForceTagExp))
		{
			UInterpData* InterpData = Cast<UInterpData>(Object);
			USequenceAction* Action = Cast<USequenceAction>(Object);
		    USequence* Sequence = Cast<USequence>(Object);

			if (InterpData)
			{
				UnreferencedMatineeData.AddUniqueItem(InterpData);
			}
			if (Action)
			{
				FoundActions.AddUniqueItem(Action);
			}
			if (Sequence)
			{
				FoundSequences.AddUniqueItem(Sequence);
			}
		}
	}

	// Check all actions... this will catch any variables that are interp data!
	for (INT ActionIndex = 0; ActionIndex < FoundActions.Num(); ActionIndex++)
	{
		USequenceAction* Action = FoundActions(ActionIndex);
		if (Action)
		{
			USeqAct_Interp* Matinee = Cast<USeqAct_Interp>(Action);
			if (Matinee)
			{
				UInterpData* InterpData = Matinee->FindInterpDataFromVariable();
				if (InterpData)
				{
					debugf(TEXT("InterpData %s, ref'd by %s"), 
						*(InterpData->GetPathName()),
						*(Action->GetFullName()));
					UnreferencedMatineeData.RemoveItem(InterpData);
				}
			}

			TArray<UInterpData*> IDataArray;
			Action->GetInterpDataVars(IDataArray);

			for (INT InterpIndex = 0; InterpIndex < IDataArray.Num(); InterpIndex++)
			{
				UInterpData* InterpData = IDataArray(InterpIndex);
				if (InterpData)
				{
					debugf(TEXT("InterpData %s, ref'd by %s"), 
						*(InterpData->GetPathName()),
						*(Action->GetFullName()));
					UnreferencedMatineeData.RemoveItem(InterpData);
				}
			}
		}
	}

	// Allow the game-specific stuff a chance as well...
	if (GGameCookerHelper)
	{
		GGameCookerHelper->CleanupKismet(this, Package);
	}

	// Now, any InterpData remaining are assumed to be unreferenced...
	for (INT UnrefIndex = 0; UnrefIndex < UnreferencedMatineeData.Num(); UnrefIndex++)
	{
		UInterpData* InterpData = UnreferencedMatineeData(UnrefIndex);
		if (InterpData)
		{
			debugf(TEXT("Cooking out InterpData %s"), *(InterpData->GetPathName()));
			InterpData->ClearFlags(RF_ForceTagExp);
			InterpData->SetFlags(RF_MarkedByCooker|RF_Transient);

			if (0) // Make this 1 to list referencers to it...
			{
				// Dump out the references to the soundcue...
				// NOTE: This is the exact code from "obj refs"
				FStringOutputDevice TempAr;
				InterpData->OutputReferencers(TempAr,TRUE);
				TArray<FString> Lines;
				TempAr.ParseIntoArray(&Lines, LINE_TERMINATOR, 0);
				for ( INT i = 0; i < Lines.Num(); i++ )
				{
					warnf(NAME_Warning, TEXT("\t%s"), *Lines(i));
				}
			}
			else
			{
				debugf(TEXT("Cooking out orphaned InterpData: %s"), *(InterpData->GetPathName()));
			}

			for (INT SeqIndex = 0; SeqIndex < FoundSequences.Num(); SeqIndex++)
			{
				USequence* Sequence = FoundSequences(SeqIndex);
				if (Sequence)
				{
					for (INT SeqObjIndex = 0; SeqObjIndex < Sequence->SequenceObjects.Num(); SeqObjIndex++)
					{
						USequenceObject* SeqObj = Sequence->SequenceObjects(SeqObjIndex);
						if (SeqObj && SeqObj->IsA(UInterpData::StaticClass()))
						{
							if (SeqObj == InterpData)
							{
								debugf(TEXT("InterpData %s, remove from %s"), 
									*(InterpData->GetPathName()),
									*(Sequence->GetFullName()));
								Sequence->SequenceObjects(SeqObjIndex) = NULL;
							}
						}
					}
				}
			}
		}
	}
}

/**
 *	Bake and prune all matinee sequences that are tagged as such.
 */
void UCookPackagesCommandlet::BakeAndPruneMatinee( UPackage* Package )
{
	// Bake and prune matinees when cooking for console.
	if ((GCookingTarget & PLATFORM_Console) != 0)
	{
		check(Package);
		check(GEditor);
		// find all SeqAct_Interp which are referenced by something in the map
		UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
		if ( World != NULL )
		{
			TArray<ULevel*> LevelsToSearch = World->Levels;

			// make sure that the world's PersistentLevel is part of the levels array for the purpose of this test.
			if ( World->PersistentLevel != NULL )
			{
				LevelsToSearch.AddUniqueItem(World->PersistentLevel);
			}

			// Bake animsets...
			for ( INT LevelIndex = 0; LevelIndex < LevelsToSearch.Num(); LevelIndex++ )
			{
				ULevel* Level = LevelsToSearch(LevelIndex);
				GEditor->BakeAnimSetsInLevel(Level);
			}

			// Prune animsets...
			for ( INT LevelIndex = 0; LevelIndex < LevelsToSearch.Num(); LevelIndex++ )
			{
				ULevel* Level = LevelsToSearch(LevelIndex);
				GEditor->PruneAnimSetsInLevel(Level);
			}
		}
	}
}

/**
 * Make sure materials are compiled for the target platform and add them to the shader cache embedded 
 * into seekfree packages.
 * @param Material - Material to process
 */
void UCookPackagesCommandlet::CompileMaterialShaders( UMaterial* Material )
{	
	check(Material);
	if( !Material->HasAnyFlags( RF_ClassDefaultObject ) 
	&&	ShaderCache	
	&&	!AlreadyHandledMaterials.Find( Material->GetFullName() )
	)
	{
		// compile the material
		TRefCountPtr<FMaterialShaderMap> MaterialShaderMapRef;
		const EMaterialShaderPlatform MaterialPlatform = GetMaterialPlatform(ShaderPlatform);
		FMaterialResource * MaterialResource = Material->GetMaterialResource(MaterialPlatform);
		check(MaterialResource);

		// create an empty static parameter set with just the Id since this is the base material
		FStaticParameterSet EmptySet(MaterialResource->GetId());
		if(MaterialResource->Compile( &EmptySet, ShaderPlatform, MaterialShaderMapRef, FALSE ))
		{
			check(MaterialShaderMapRef);
			// add the material's shader map to the shader cache being saved into the seekfree package
			ShaderCache->AddMaterialShaderMap( MaterialShaderMapRef );
		}
		else if (Material->bUsedAsSpecialEngineMaterial)
		{
			appErrorf(TEXT("Failed to compile default material %s!"), *Material->GetName());
		}
		else
		{
			warnf(NAME_Warning, TEXT("Failed to compile shaders for Material %s, Default Material will be used in game."), *Material->GetPathName());
		}
	}
}

/**
* Make sure material instances are compiled and add them to the shader cache embedded into seekfree packages.
* @param MaterialInstance - MaterialInstance to process
*/

void UCookPackagesCommandlet::CompileMaterialInstanceShaders( UMaterialInstance* MaterialInstance )
{	
	check(MaterialInstance);
	//only process if the material instance has a static permutation 
	if( !MaterialInstance->HasAnyFlags( RF_ClassDefaultObject ) 
		&&	MaterialInstance->bHasStaticPermutationResource
		&&	ShaderCache	
		&&	!AlreadyHandledMaterialInstances.Find( MaterialInstance->GetFullName() )
		)
	{
		// compile the material instance's shaders for the target platform
		MaterialInstance->CacheResourceShaders(ShaderPlatform, FALSE, FALSE);
		const EMaterialShaderPlatform MaterialPlatform = GetMaterialPlatform(ShaderPlatform);
		FMaterialResource* MaterialResource = MaterialInstance->GetMaterialResource(MaterialPlatform);
		check(MaterialResource);
		TRefCountPtr<FMaterialShaderMap> MaterialShaderMapRef = MaterialResource->GetShaderMap();
		if (MaterialShaderMapRef)
		{
			// add it to the shader cache being saved into the seekfree package
			ShaderCache->AddMaterialShaderMap( MaterialShaderMapRef );
		}
	}
}

/**
 * Determine if source package of given object is newer than timestamp
 *
 * @param Object - object to check source package file timestamp
 * @param TimeToCheck - time stamp to check against
 * @return TRUE if object source package is newere than the timestamp
 **/
static UBOOL IsSourcePackageFileNewer(UObject* Object, DOUBLE TimeToCheck)
{
	UBOOL bResult = TRUE;
	// compare source package file timestamp
	ULinkerLoad* Linker = Object->GetLinker();
	if( Linker )
	{
		DOUBLE SrcFileTime = GFileManager->GetFileTimestamp(*Linker->Filename);
		if( SrcFileTime > 0 )
		{
			// src packge is newer
			bResult = SrcFileTime > TimeToCheck;
		}
	}
	return bResult;
}

/**
 * Prepares object for saving into package. Called once for each object being saved 
 * into a new package.
 *
 * @param	Package						Package going to be saved
 * @param	Object						Object to prepare
 * @param	bIsSavedInSeekFreePackage	Whether object is going to be saved into a seekfree package
 * @param	bIsTextureOnlyFile			Whether file is only going to contain texture mips
 */
void UCookPackagesCommandlet::PrepareForSaving( UPackage* Package, UObject* Object, UBOOL bIsSavedInSeekFreePackage, UBOOL bIsTextureOnlyFile )
{
	SCOPE_SECONDS_COUNTER(PrepareForSavingTime);

	// Don't serialize editor-only objects!
	if ((Object->IsIn(Package) || Object->HasAllFlags(RF_ForceTagExp)) &&
		Object->HasAnyFlags(RF_LoadForEdit) && 
		!Object->HasAnyFlags(RF_LoadForClient) &&
		!Object->HasAnyFlags(RF_LoadForServer))
	{
		Object->ClearFlags(RF_ForceTagExp);
		Object->ClearFlags(RF_TagExp);
		Object->SetFlags(RF_Transient);
		return;
	}

	// See if it's in the 'never cook' list...
	if (NeverCookObjects.Find(Object->GetPathName()) != NULL)
	{
		Object->ClearFlags(RF_ForceTagExp);
		Object->ClearFlags(RF_TagExp);
		Object->SetFlags(RF_Transient);
		return;
	}

	// Prepare texture for saving unless it's the class default object.
	UTexture2D* Texture2D = Cast<UTexture2D>(Object);
	if( Texture2D && !Texture2D->HasAnyFlags( RF_ClassDefaultObject ) )
	{
		SCOPE_SECONDS_COUNTER(PrepareForSavingTextureTime);

		// Number of mips in texture.
		INT		NumMips				= Texture2D->Mips.Num();								
		// Index of first mip, taking into account LOD bias and settings.
		INT		FirstMipIndex		= Clamp<INT>( PlatformLODSettings.CalculateLODBias( Texture2D ), 0, NumMips-1 );
		// Whether the texture can be streamed.
		UBOOL	bIsStreamingTexture	= !Texture2D->NeverStream;
		// Whether the texture is applied to the face of a cubemap, in which case we can't stream it.
		UBOOL	bIsCubemapFace		= Texture2D->GetOuter()->IsA(UTextureCube::StaticClass());

		// make sure we load at least the first packed mip level
		FirstMipIndex = Min(FirstMipIndex, Texture2D->MipTailBaseIdx);

		// Textures residing in seekfree packages cannot be streamed as the file offset is not known till after
		// they are saved.
		if( (bIsSavedInSeekFreePackage && Texture2D->IsIn( Package ))
		// Cubemap faces cannot be streamed.
		||	bIsCubemapFace )
		{
			bIsStreamingTexture		= FALSE;
			Texture2D->NeverStream	= TRUE;
		}

		// Streaming textures will use the texture file cache if it is enabled.
		if( bUseTextureFileCache && bIsStreamingTexture )
		{
			Texture2D->TextureFileCacheName = FName(*GetTextureCacheName(Texture2D));
		}
		// Disable use of texture file cache.
		else
		{
			Texture2D->TextureFileCacheName = NAME_None;
		}
		// TRUE if the texture mips can be stored in a separate file
		const UBOOL bAllowStoreInSeparateFile = bIsStreamingTexture && bIsSavedInSeekFreePackage;

		// see if we have an existing cooked entry for the texture in the TFC
		FCookedTextureFileCacheInfo* TextureFileCacheInfo = NULL;
		// if texture has a matching GUID and has been already been saved in the TFC
		UBOOL bHasMatchingGuidTFC = FALSE;
		// if texture's source package has a newer timestamp than the entry already saved in the TFC
		UBOOL bSrcIsNewerTimeStampTFC = TRUE;		
		if( bUseTextureFileCache && bAllowStoreInSeparateFile )
		{
			TextureFileCacheInfo = PersistentCookerData->GetTextureFileCacheEntryInfo(Texture2D);
			if( TextureFileCacheInfo )
			{
				// compare Guids
				bHasMatchingGuidTFC = (TextureFileCacheInfo->TextureFileCacheGuid == Texture2D->TextureFileCacheGuid);
				// texture Guid trumps file stamp comparison so no need to check it
				if( !bHasMatchingGuidTFC )
				{
					bSrcIsNewerTimeStampTFC = IsSourcePackageFileNewer(Texture2D, TextureFileCacheInfo->LastSaved);
				}
				// Ensure the texture is in the same cache it was...
				if (TextureFileCacheInfo->TextureFileCacheName != Texture2D->TextureFileCacheName)
				{
					bHasMatchingGuidTFC = FALSE;
				}
			}
		}		

		for( INT MipIndex=NumMips-1; MipIndex>=0; MipIndex-- )
		{	
			FTexture2DMipMap& Mip = Texture2D->Mips(MipIndex);
			
			UBOOL bIsStoredInSeparateFile	= FALSE;
			UBOOL bUnusedMipLevel			= FALSE;

			// We still require MinStreamedInMips to be present in the seek free package as the streaming code 
			// assumes those to be always loaded.
			if( bAllowStoreInSeparateFile
			// @todo streaming, @todo cooking: This assumes the same value of GMinTextureResidentMipCount across platforms.
			&&	(MipIndex < NumMips - GMinTextureResidentMipCount)
			// Miptail must always be loaded.
			&&	MipIndex < Texture2D->MipTailBaseIdx )
			{
				bIsStoredInSeparateFile = TRUE;
			}

			// Cut off miplevels for streaming textures that are always loaded and therefore don't need to be streamed if
			// we are using the texture file cache and saving a texture only file. We don't cut them off otherwise as the 
			// packages are openable in the Editor for e.g. the PC in their cooked state and therefore require the lower
			// miplevels.
			//
			//	Texture file cache is enabled.
			if( bUseTextureFileCache 
			//	Texture-only file, all other objects  will be stripped.
			&&	bIsTextureOnlyFile
			//	Non- streaming textures don't need to be stored in file cache, neither do miplevels that are duplicated
			//	into the seekfree packages or cubemap faces.
			&&	(!bIsStreamingTexture || (MipIndex >= NumMips - GMinTextureResidentMipCount)) )
			{
				bUnusedMipLevel = TRUE;
			}

			// Cut off miplevels that are never going to be used. This requires a FULL recook every time the texture
			// resolution is increased.
			if( MipIndex < FirstMipIndex )
			{
				bUnusedMipLevel	= TRUE;
			}

			// Cut off miplevels that are already packed in the textures miptail level 
			// since they're all stored in the base miptail level
			if( MipIndex > Texture2D->MipTailBaseIdx )
			{
				bUnusedMipLevel	= TRUE;
			}

			// skip mip levels that are not being used	
			if( bUnusedMipLevel )
			{
				// bulk data is not saved 
				Mip.Data.StoreInSeparateFile( 
					TRUE,
					BULKDATA_Unused,
					0,
					INDEX_NONE,
					INDEX_NONE );
			}
			// Handle miplevels that are being streamed and therefore stored in separate file.
			else if( bIsStoredInSeparateFile )
			{
				// Retrieve offset and size of payload in separate file.
				FString					BulkDataName = *FString::Printf(TEXT("MipLevel_%i"),MipIndex);
				FCookedBulkDataInfo*	BulkDataInfo = PersistentCookerData->GetBulkDataInfo( Texture2D, *BulkDataName );

				if( bUseTextureFileCache )
				{
					// If we have an existing entry and the texture GUIDs match then no TFC update is needed for the mip
					if( BulkDataInfo && 
						(bHasMatchingGuidTFC || !bSrcIsNewerTimeStampTFC) )
					{
						// use previous bulk data info for setting info
						Mip.Data.StoreInSeparateFile( 
							TRUE,
							BulkDataInfo->SavedBulkDataFlags,
							BulkDataInfo->SavedElementCount,
							BulkDataInfo->SavedBulkDataOffsetInFile,
							BulkDataInfo->SavedBulkDataSizeOnDisk );
					}
					else
					{
						// Update the mip entry in the TFC file
						// This will also update the mip bulk data info
						SaveMipToTextureFileCache(Package, Texture2D, MipIndex);
					}
				}
				// not using TFC so assumption is that texture has already been saved in
				// another SF package and just need to update the bulk data info for the mip in this package
				else 
				{
					if( BulkDataInfo )
					{
						// use previous bulk data info for setting info
						Mip.Data.StoreInSeparateFile( 
							TRUE,
							BulkDataInfo->SavedBulkDataFlags,
							BulkDataInfo->SavedElementCount,
							BulkDataInfo->SavedBulkDataOffsetInFile,
							BulkDataInfo->SavedBulkDataSizeOnDisk );
					}
					else
					{
						appErrorf( TEXT("Couldn't find seek free bulk data info: %s for %s\nRecook with -full"), *BulkDataName, *Texture2D->GetFullName() );
					}
				}
/*** TESTING FOR TFC GENERATION ERRORS...
				debugf(TEXT("Mip %2d: Flags = %10d, Offset = %10d, Size = %10d, %s"),
					MipIndex,
					Mip.Data.SavedBulkDataFlags,
					Mip.Data.SavedBulkDataOffsetInFile,
					Mip.Data.SavedBulkDataSizeOnDisk,
					*(Texture2D->GetPathName())
					);
				debugf(TEXT("  Bulk: Flags = %10d, Offset = %10d, Size = %10d"),
					BulkDataInfo ? BulkDataInfo->SavedBulkDataFlags : -1,
					BulkDataInfo ? BulkDataInfo->SavedBulkDataOffsetInFile : -1,
					BulkDataInfo ? BulkDataInfo->SavedBulkDataSizeOnDisk : -1
					);
				if (BulkDataInfo)
				{
					if ((BulkDataInfo->SavedBulkDataFlags != Mip.Data.SavedBulkDataFlags) ||
						(BulkDataInfo->SavedBulkDataOffsetInFile != Mip.Data.SavedBulkDataOffsetInFile) ||
						(BulkDataInfo->SavedBulkDataSizeOnDisk != Mip.Data.SavedBulkDataSizeOnDisk) ||
						(BulkDataInfo->SavedElementCount != Mip.Data.SavedElementCount)
						)
					{
						checkf(0, TEXT("Mip bulk data information does not match bulk info for Mip %d, %s"),
							MipIndex, *(Texture2D->GetPathName()));
					}
				}
***/
			}
			// We're not cutting off miplevels as we're either a regular package or the texture is not streamable.
			else
			{
				Mip.Data.StoreInSeparateFile( FALSE );
			}

			// Miplevels that are saved into the map/ seekfree package shouldn't be individually compressed as we're not going
			// to stream out of them and it just makes serialization slower. In the long run whole package compression is going
			// to take care of getting the file size down.
			if (bIsSavedInSeekFreePackage && !bIsStoredInSeparateFile)
			{
				Mip.Data.StoreCompressedOnDisk( COMPRESS_None );
			}
			// Store mips compressed in regular packages so we can stream compressed from disk.
			else
			{
				Mip.Data.StoreCompressedOnDisk( GBaseCompressionMethod );
			}
		}

		if( bUseTextureFileCache && bAllowStoreInSeparateFile )
		{
			// update GUIDs for this texture since it was saved in the TFC
			if( TextureFileCacheInfo )
			{
				TextureFileCacheInfo->TextureFileCacheGuid = Texture2D->TextureFileCacheGuid;
			}
			else
			{
				// add a new entry if dont have an existing one
				FCookedTextureFileCacheInfo NewTextureFileCacheInfo;
				NewTextureFileCacheInfo.TextureFileCacheGuid = Texture2D->TextureFileCacheGuid;				
				NewTextureFileCacheInfo.TextureFileCacheName = Texture2D->TextureFileCacheName;
				PersistentCookerData->SetTextureFileCacheEntryInfo(Texture2D,NewTextureFileCacheInfo);
			}			
		}

		if (bUseTextureFileCache && bVerifyTextureFileCache && (Texture2D->TextureFileCacheName != NAME_None))
		{
			// Add it to the tracking data...
			AddVerificationTextureFileCacheEntry(Package, Texture2D, bIsSavedInSeekFreePackage);
		}
	}

	ATerrain* Terrain = Cast<ATerrain>(Object);
	if (Terrain)
	{
		SCOPE_SECONDS_COUNTER(PrepareForSavingTerrainTime);
		const EMaterialShaderPlatform MaterialPlatform = GetMaterialPlatform(ShaderPlatform);
		TArrayNoInit<FTerrainMaterialResource*>& CachedMaterials = Terrain->GetCachedTerrainMaterials(MaterialPlatform);
		// Make sure materials are compiled for the platform and add them to the shader cache embedded into seekfree packages.
		for (INT CachedMatIndex = 0; CachedMatIndex < CachedMaterials.Num(); CachedMatIndex++)
		{
			FTerrainMaterialResource* TMatRes = CachedMaterials(CachedMatIndex);
			if( TMatRes && ShaderCache )
			{
				// Compile the material...
				TRefCountPtr<FMaterialShaderMap> MaterialShaderMapRef;
				FStaticParameterSet EmptySet(TMatRes->GetId());
				if (TMatRes->Compile(&EmptySet, ShaderPlatform, MaterialShaderMapRef, FALSE))
				{
					check(MaterialShaderMapRef);
					// ... and add it to the shader cache being saved into the seekfree package.
					ShaderCache->AddMaterialShaderMap( MaterialShaderMapRef );
				}
			}
		}
	}

	// Compile shaders for materials
	UMaterial* Material = Cast<UMaterial>(Object);
	if( Material ) 
	{
		SCOPE_SECONDS_COUNTER(PrepareForSavingMaterialTime);
		CompileMaterialShaders( Material );
	}

	// Compile shaders for material instances with static parameters
	UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(Object);
	if( MaterialInstance ) 
	{
		SCOPE_SECONDS_COUNTER(PrepareForSavingMaterialInstanceTime);
		CompileMaterialInstanceShaders( MaterialInstance );
	}

	// 'Cook-out' material expressions on consoles
	// In this case, simply don't load them on the client
	// Keep 'parameter' types around so that we can get their defaults.
	if (GCookingTarget & PLATFORM_Console)
	{
		UMaterialExpression* MatExp = Cast<UMaterialExpression>(Object);
		if (MatExp)
		{
			if( MatExp->IsTemplate() == FALSE && !MatExp->bIsParameterExpression )
			{
				debugfSuppressed(NAME_DevCooking, TEXT("Cooking out material expression %s (%s)"), 
					*(MatExp->GetName()), MatExp->GetOuter() ? *(MatExp->GetOuter()->GetName()) : TEXT("????"));
				MatExp->SetFlags(RF_NotForClient|RF_NotForServer|RF_NotForEdit);
			}
		}
	}
	
	// Cook out extra detail static meshes on consoles.
	if( GCookingTarget & PLATFORM_Console)
	{
		// NULL out static mesh reference for components below platform detail mode threshold.
		UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Object);
		if( StaticMeshComponent && StaticMeshComponent->DetailMode > PlatformDetailMode )
		{
			StaticMeshComponent->StaticMesh = NULL;
		}
	}
}

/**
 * Setup the commandlet's platform setting based on commandlet params
 * @param Params The commandline parameters to the commandlet - should include "platform=xxx"
 */
UBOOL UCookPackagesCommandlet::SetPlatform(const FString& Params)
{
	// default to success
	UBOOL Ret = TRUE;

	FString PlatformStr;
	if (Parse(*Params, TEXT("PLATFORM="), PlatformStr))
	{
		if (PlatformStr == TEXT("PS3"))
		{
			Platform = PLATFORM_PS3;
			ShaderPlatform = SP_PS3;
			GCreationFlags = TextureCreate_NoPackedMip;
		}
		else if (PlatformStr == TEXT("xenon") || PlatformStr == TEXT("xbox360"))
		{	
			Platform = PLATFORM_Xenon;
			ShaderPlatform = SP_XBOXD3D;
			GCreationFlags = 0;
		}
		else if (PlatformStr == TEXT("pc") || PlatformStr == TEXT("win32"))
		{
			Platform = PLATFORM_Windows;
			ShaderPlatform = SP_PCD3D_SM3;
			GCreationFlags = TextureCreate_NoPackedMip;
		}
		else
		{
			SET_WARN_COLOR(COLOR_RED);
			warnf(NAME_Error, TEXT("Unknown platform! No shader platform exists for this platform yet!"));
			CLEAR_WARN_COLOR();

			// this is a failure
			Ret = FALSE;
		}
	}
	else
	{
		Ret = FALSE;
	}

	return Ret;
}

/**
 * Tried to load the DLLs and bind entry points.
 *
 * @return	TRUE if successful, FALSE otherwise
 */
UBOOL UCookPackagesCommandlet::BindDLLs()
{
	if( Platform == PLATFORM_PS3 || Platform == PLATFORM_Xenon )
	{
		// Load in the console support containers
		FConsoleSupportContainer::GetConsoleSupportContainer()->LoadAllConsoleSupportModules();

		// Find the target platform PC-side support implementation.
		ConsoleSupport = FConsoleSupportContainer::GetConsoleSupportContainer()->GetConsoleSupport(*GetPlatformString());
		if(!ConsoleSupport)
		{
			appDebugMessagef(TEXT("Couldn't bind to support DLL for %s."), *GetPlatformString());
			return FALSE;
		}

		// Create the platform specific texture and sound cookers and shader compiler.
		TextureCooker		= ConsoleSupport->GetGlobalTextureCooker();
		SoundCooker			= ConsoleSupport->GetGlobalSoundCooker();

		SkeletalMeshCooker	= ConsoleSupport->GetGlobalSkeletalMeshCooker();
		StaticMeshCooker	= ConsoleSupport->GetGlobalStaticMeshCooker();

		check(!GConsoleShaderPrecompilers[ShaderPlatform]);
		GConsoleShaderPrecompilers[ShaderPlatform] = ConsoleSupport->GetGlobalShaderPrecompiler();
		
		// Clean up and abort on failure.
		if( !TextureCooker 
		||	!SoundCooker 
		||	!SkeletalMeshCooker
		||	!StaticMeshCooker
		||	!GConsoleShaderPrecompilers[ShaderPlatform] )
		{
			appDebugMessagef(TEXT("Couldn't create platform cooker singletons."));
			return FALSE;
		}
	}
	return TRUE;
}

/**
* Update all game .ini files from defaults
*
* @param IniPrefix	prefix for ini filename in case we want to write to cooked path
*/
void UCookPackagesCommandlet::UpdateGameIniFilesFromDefaults(const TCHAR* IniPrefix)
{
	// get all files in the config dir
	TArray<FString> IniFiles;
	appFindFilesInDirectory(IniFiles, *appGameConfigDir(), FALSE, TRUE);

	UINT YesNoToAll = ART_No;
	// go over ini files and generate non-default ini files from default inis
	for (INT FileIndex = 0; FileIndex < IniFiles.Num(); FileIndex++)
	{
		// the ini path/file name
		FFilename IniPath = IniFiles(FileIndex);
		FString IniFilename = IniPath.GetCleanFilename();

		// we want to process all the Default*.ini files
		if (IniFilename.Left(7) == PC_DEFAULT_INI_PREFIX )
		{
			// generate ini files
			TCHAR GeneratedIniName[MAX_SPRINTF] = TEXT("");
			TCHAR GeneratedDefaultIniName[MAX_SPRINTF] = TEXT("");

			appCreateIniNames(
				GeneratedIniName, 
				GeneratedDefaultIniName,
				NULL, NULL,
				*IniFilename.Right(IniFilename.Len() - 7),
				PC_DEFAULT_INI_PREFIX,
				IniPrefix);

			const UBOOL bTryToPreserveContents = FALSE;
			appCheckIniForOutdatedness(
				GeneratedIniName, 
				GeneratedDefaultIniName,
				bTryToPreserveContents,
				YesNoToAll);
		}
	}
}

/**
 * Precreate all the .ini files that the platform will use at runtime
 * @param bAddForHashing - TRUE if running with -sha and ini files should be added to list of hashed files
 */
void UCookPackagesCommandlet::CreateIniFiles(UBOOL bAddForHashing)
{
	// force unattended for ini generation so it doesn't prompt
	UBOOL bWasUnattended = GIsUnattended;
	GIsUnattended = TRUE;

	if( Platform == PLATFORM_PS3 || Platform == PLATFORM_Xenon )
	{
		// make sure the output config directory exists
		GFileManager->MakeDirectory(*(appGameConfigDir() + GetConfigOutputDirectory()));

		UINT YesNoToAll = ART_No;
		TCHAR PlatformIniName[MAX_SPRINTF] = TEXT("");
		TCHAR PlatformDefaultIniName[MAX_SPRINTF] = TEXT("");

		const UBOOL bTryToPreserveContents = FALSE;

		// assemble standard ini files
		appCreateIniNames(
			PlatformEngineConfigFilename, 
			PlatformDefaultIniName,
			NULL, NULL, 
			TEXT("Engine.ini"), 
			*GetPlatformDefaultIniPrefix(),
			*GetPlatformConfigOutputPrefix());
		appCheckIniForOutdatedness(
			PlatformEngineConfigFilename, 
			PlatformDefaultIniName,
			bTryToPreserveContents,
			YesNoToAll);

		appCreateIniNames(
			PlatformIniName, 
			PlatformDefaultIniName,
			NULL, NULL, 
			TEXT("Game.ini"), 
			*GetPlatformDefaultIniPrefix(),
			*GetPlatformConfigOutputPrefix());
		appCheckIniForOutdatedness(
			PlatformIniName, 
			PlatformDefaultIniName,
			bTryToPreserveContents,
			YesNoToAll);

		appCreateIniNames(
			PlatformIniName, 
			PlatformDefaultIniName,
			NULL, NULL, 
			TEXT("Input.ini"), 
			*GetPlatformDefaultIniPrefix(),
			*GetPlatformConfigOutputPrefix());
		appCheckIniForOutdatedness(
			PlatformIniName, 
			PlatformDefaultIniName,
			bTryToPreserveContents,
			YesNoToAll);

		appCreateIniNames(
			PlatformIniName, 
			PlatformDefaultIniName,
			NULL, NULL, 
			TEXT("UI.ini"), 
			*GetPlatformDefaultIniPrefix(),
			*GetPlatformConfigOutputPrefix());
		appCheckIniForOutdatedness(
			PlatformIniName, 
			PlatformDefaultIniName,
			bTryToPreserveContents,
			YesNoToAll);

		// update all game inis
		UpdateGameIniFilesFromDefaults( *GetConfigOutputPrefix() );

		// coalesce the ini and int files
		GConfig->CoalesceFilesFromDisk(*(appGameConfigDir() + GetConfigOutputDirectory()), ShouldByteSwapData(), PlatformEngineConfigFilename, *GetPlatformString(), bCookCurrentLanguageOnly);

		if( bAddForHashing )
		{
			if (bCookCurrentLanguageOnly)
			{
				const TCHAR* Ext = UObject::GetLanguage();
				// mark each coalesced loc file for sha
				FilesForSHA.AddItem(*FString::Printf(TEXT("%s%s%sCoalesced_%s.bin"), *appGameDir(), *GetCookedDirectory(), PATH_SEPARATOR, Ext));
			}
			else
			{
				for (INT LangIndex = 0; GKnownLanguageExtensions[LangIndex]; LangIndex++)
				{
					const TCHAR* Ext = GKnownLanguageExtensions[LangIndex];
					// mark each coalesced loc file for sha
					FilesForSHA.AddItem(*FString::Printf(TEXT("%s%s%sCoalesced_%s.bin"), *appGameDir(), *GetCookedDirectory(), PATH_SEPARATOR, Ext));
				}
			}
		}
	}
	else
	{
		// Use the current engine ini file on the PC.
		appStrcpy( PlatformEngineConfigFilename, GEngineIni );

		if( bAddForHashing )
		{
			// update all game inis in default PC directory
			UpdateGameIniFilesFromDefaults(PC_INI_PREFIX);

			// get list of files we don't want to sign
			TMultiMap<FString, FString>* NonSignedConfigFiles = GConfig->GetSectionPrivate(TEXT("NonSignedConfigFilter"), FALSE, TRUE, GEngineIni);

			// get all generated ini files in the game config dir
			TArray<FString> GameIniFiles;
			appFindFilesInDirectory(GameIniFiles, *appGameConfigDir(), FALSE, TRUE);
			for( INT FileIdx=0; FileIdx < GameIniFiles.Num(); FileIdx++ )
			{
				INT GameNameLen = appStrlen(GGameName);
				check(GameNameLen>0);

				// the ini path/file name
				FFilename IniPath = GameIniFiles(FileIdx);
				FString IniFilename = IniPath.GetCleanFilename();

				UBOOL bShouldHash =TRUE;
				// we want to process all the <GameName>*.ini files
				if( IniFilename.Left(GameNameLen) != GGameName )
				{
					bShouldHash = FALSE;
				}
				// skip over *.xml files
				else if( FFilename(IniFilename).GetExtension().ToLower() == FString(TEXT("xml")) )
				{
					bShouldHash = FALSE;
				}
				// skip any that are marked to not be signed
				else if( NonSignedConfigFiles )
				{
					for( TMultiMap<FString, FString>::TIterator It(*NonSignedConfigFiles); It; ++It )
					{
						if( FFilename(It.Value()).GetCleanFilename() == IniFilename )
						{
							bShouldHash = FALSE;
							break;
						}
					}
				}

				// add to list of files for hashing
				if( bShouldHash )
				{
					FilesForSHA.AddItem(IniPath);
				}
			}
		}
	}

	// restore unattendedness
	GIsUnattended = bWasUnattended;
}

/** 
 * Prepares shader files for the given platform to make sure they can be used for compiling
 */
UBOOL UCookPackagesCommandlet::PrepareShaderFiles()
{
	// for cooking as an enduser for PS3, the shader files must exist in the My Games directory
	// because we need to be able to write the Material.usf/VertexShader.usf to the same directory
	// that contains the shader files
	if (bIsInUserMode && Platform == PLATFORM_PS3)
	{
		// get all the shaders in the shaders directory
		TArray<FString> Results;
		FString ShaderDir = FString(appBaseDir()) * TEXT("..") PATH_SEPARATOR TEXT("Engine") PATH_SEPARATOR TEXT("Shaders");
		FString UserShaderDir = GFileManager->ConvertAbsolutePathToUserPath(*GFileManager->ConvertToAbsolutePath(*ShaderDir));
		appFindFilesInDirectory(Results, *ShaderDir, FALSE, TRUE);

		if (GFileManager->ConvertToAbsolutePath(*ShaderDir) != UserShaderDir)
		{
			// Make sure the target directory exists.
			if( !GFileManager->MakeDirectory( *UserShaderDir, TRUE ) )
			{
				return FALSE;
			}

			// copy each shader over
			for (INT ShaderIndex = 0; ShaderIndex < Results.Num(); ShaderIndex++)
			{
				FString DestFilename = GFileManager->ConvertAbsolutePathToUserPath(*GFileManager->ConvertToAbsolutePath(*Results(ShaderIndex)));
				if ( GFileManager->Copy(*DestFilename, *Results(ShaderIndex), TRUE, TRUE) != COPY_OK )
				{
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}

/** 
 * Cleans up shader files for the given platform 
 */
void UCookPackagesCommandlet::CleanupShaderFiles()
{
	// delete any files we copied over in PrepareShaderFiles
	if (bIsInUserMode && Platform == PLATFORM_PS3)
	{
		// this will clean up the shader files
		appCleanFileCache();
	}
}

/**
 * Warns the user if the map they are cooking has references to editor content (EditorMeshes, etc)
 *
 * @param Package Package that has been loaded by the cooker
 */
void UCookPackagesCommandlet::WarnAboutEditorContentReferences(UPackage* Package)
{
	if ( (GCookingTarget&PLATFORM_Console) != 0 && Package->ContainsMap() )
	{
		if( EditorOnlyContentPackageNames.Num() )
		{
			// get linker load for the package
			UObject::BeginLoad();
			ULinkerLoad* Linker = UObject::GetPackageLinker(Package, NULL, LOAD_None, NULL, NULL);
			UObject::EndLoad();
			
			// look for editor references
			for (INT ImportIndex = 0; ImportIndex < Linker->ImportMap.Num(); ImportIndex++)
			{
				// don't bother outputting package references, just the objects
				if (Linker->ImportMap(ImportIndex).ClassName != NAME_Package)
				{
					// get package name of the import
					FString ImportPackage = FFilename(Linker->GetImportPathName(ImportIndex)).GetBaseFilename();
					// Warn if part of content package list.
					if( EditorOnlyContentPackageNames.FindItemIndex(ImportPackage) != INDEX_NONE )
					{
						warnf(NAME_Warning, TEXT("Map (%s) references editor only content: '%s'! This will not load in-game!!"), *Package->GetName(), *Linker->GetImportFullName(ImportIndex));
					}
				}
			}
		}
	}
}

/**
 * Loads a package that will be used for cooking. This will cache the source file time
 * and add the package to the Guid Cache
 *
 * @param Filename Name of package to load
 *
 * @return Package that was loaded and cached
 */
UPackage* UCookPackagesCommandlet::LoadPackageForCooking(const TCHAR* Filename)
{
	UPackage* Package = NULL;

	// Clear the persistent map FaceFX tracker...
	CurrentPMapGroupAnimLookupMap = NULL;
	CurrentPersistentMapFaceFXArray = NULL;

	UFaceFXAnimSet* PersistentFaceFXAnimSet = NULL;
	if (bGeneratePersistentMapAnimSet)
	{
		// Setup the persistent FaceFX animation information, if required...
		FFilename CheckPackageFilename = Filename;
		SetupPersistentMapFaceFXAnimation(CheckPackageFilename);

		// If this is the persistent map, need to create it BEFORE actually loading the package
		// This is to properly clean up the package(s) that get(s) loaded generating it...
		FString LevelCheckName = CheckPackageFilename.GetBaseFilename().ToUpper();
		FString* PMapString = LevelToPersistentLevelMap.Find(LevelCheckName);
		if (PMapString != NULL)
		{
			if (bLogPersistentFaceFXGeneration)
			{
				debugf(TEXT("Found PMap %32s for level %s"), *(*PMapString), *LevelCheckName);
			}
			
			CurrentPMapGroupAnimLookupMap = PersistentMapToGroupAnimLookupMap.Find(*PMapString);
			CurrentPersistentMapFaceFXArray = PersistentMapFaceFXArray.Find(*PMapString);

			// Generate the FaceFX anim set for the level...
			if (*PMapString == LevelCheckName)
			{
				PersistentFaceFXAnimSet = GeneratePersistentMapFaceFXAnimSet(CheckPackageFilename);
			}
		}
	}

	if (GGameCookerHelper)
	{
		Package = GGameCookerHelper->LoadPackageForCookingCallback(this, Filename);
	}

	if (Package == NULL)
	{
		Package = UObject::LoadPackage(NULL, Filename, LOAD_None);
	}

	// if the package loaded, then add it to the package cache, and cache the filetime (unless it's the guid cache or persistent data)
	if (Package)
	{
		if (GGameCookerHelper)
		{
			if (GGameCookerHelper->PostLoadPackageForCookingCallback(this, Package) == FALSE)
			{
				Package = NULL;
				// Collect garbage and verify it worked as expected.	
				CollectGarbageAndVerify();
				return NULL;
			}
		}

		if ( (Package->PackageFlags&(PKG_ContainsDebugInfo|PKG_ContainsScript)) == (PKG_ContainsDebugInfo|PKG_ContainsScript) )
		{
			warnf(NAME_Error, TEXT("The script contained in package '%s' was compiled in debug mode.  Please recompile all script package in release before cooking."), Filename);
			return NULL;
		}

		if( bUseTextureFileCache )
		{
			// cache the guid for all currently loaded packages since we don't iterate over all packages when using the TFC
			for( TObjectIterator<UPackage> It; It; ++It )
			{
				UPackage* PackageObj = *It;
				GuidCache->SetPackageGuid(PackageObj->GetFName(), PackageObj->GetGuid());
			}
		}
		else
		{
			// cache guid for current cooked package
			GuidCache->SetPackageGuid(Package->GetFName(), Package->GetGuid());
		}
		
		// save it out
		GuidCache->SaveToDisk(ShouldByteSwapData());

		// warn about any references to editor-only content
		if( ParseParam(appCmdLine(),TEXT("WarnAboutEditorContentReferences")) )
		{
			WarnAboutEditorContentReferences(Package);
		}

		if (bGeneratePersistentMapAnimSet && PersistentFaceFXAnimSet)
		{
			PersistentFaceFXAnimSet->Rename(*(PersistentFaceFXAnimSet->GetName()), Package);
			UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
			check(World);
			PersistentFaceFXAnimSet->RemoveFromRoot();
			World->PersistentFaceFXAnimSet = PersistentFaceFXAnimSet;
		}

		CleanupKismet(Package);
	}

	return Package;
}

/**
 * Force load a package and emit some useful info
 * 
 * @param PackageName Name of package, could be different from filename due to localization
 * @param bRequireServerSideOnly If TRUE, the loaded packages are required to have the PKG_ServerSideOnly flag set for this function to succeed
 *
 * @return TRUE if successful
 */
UBOOL UCookPackagesCommandlet::ForceLoadPackage(const FString& PackageName, UBOOL bRequireServerSideOnly)
{
	warnf( NAME_Log, TEXT("Force loading:  %s"), *PackageName );
	UPackage* Package = LoadPackageForCooking(*PackageName);
	if (Package == NULL)
	{
		warnf(NAME_Error, TEXT("Failed to load package '%s'"), *PackageName);
		return FALSE;
	}
	else if (bRequireServerSideOnly && !(Package->PackageFlags & PKG_ServerSideOnly))
	{
		warnf(NAME_Error, TEXT("Standalone seekfree packages must have ServerSideOnly set. Use the 'SetPackageFlags' commandlet to do this."));
		return FALSE;
	}

	return TRUE;
}

/**
 * Load all packages in a specified ini section with the Package= key
 * @param SectionName Name of the .ini section ([Engine.PackagesToAlwaysCook])
 * @param PackageNames Paths of the loaded packages
 * @param KeyName Optional name for the key of the list to load (defaults to "Package")
 * @param bShouldSkipLoading If TRUE, this function will only fill out PackageNames, and not load the package
 * @param bRequireServerSideOnly If TRUE, the loaded packages are required to have the PKG_ServerSideOnly flag set for this function to succeed
 * @return if loading was required, whether we successfully loaded all the packages; otherwise, always TRUE
 */
UBOOL UCookPackagesCommandlet::LoadSectionPackages(const TCHAR* SectionName, TArray<FString>& PackageNames, const TCHAR* KeyName, UBOOL bShouldSkipLoading, UBOOL bRequireServerSideOnly)
{
	// here we need to look in the .ini to see which packages to load
	TMultiMap<FString,FString>* PackagesToLoad = GConfig->GetSectionPrivate( SectionName, 0, 1, PlatformEngineConfigFilename );
	if (PackagesToLoad)
	{
		SCOPE_SECONDS_COUNTER(LoadSectionPackagesTime);
		for( TMultiMap<FString,FString>::TIterator It(*PackagesToLoad); It; ++It )
		{
			if (It.Key() == KeyName)
			{
				FString& PackageName = It.Value();
				FString PackageFileName;
				if( GPackageFileCache->FindPackageFile( *PackageName, NULL, PackageFileName ) == TRUE )
				{
					PackageNames.AddItem( FString(*PackageName).ToUpper() );
					if (!bShouldSkipLoading)
					{
						if (!ForceLoadPackage(PackageName, bRequireServerSideOnly))
						{
							return FALSE;
						}
					}
				}
				else
				{
					warnf(NAME_Error, TEXT("Failed to find package '%s'"), *PackageName);
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}


/**
 * Helper struct to sort a list of packages by size
 * Not fast!
 * @todo: Cache file sizes for faster sorting
 */
struct FPackageSizeSorter
{
	static INT Compare(const FString& A, const FString& B)
	{
		// get the paths to the files
		FString PathA;
		if (GPackageFileCache->FindPackageFile(*A, NULL, PathA) == FALSE)
		{
			return 0;
		}

		FString PathB;
		if (GPackageFileCache->FindPackageFile(*B, NULL, PathB) == FALSE)
		{
			return 0;
		}

		// get the sizes of the files
		QWORD SizeA = GFileManager->FileSize(*PathA);
		QWORD SizeB = GFileManager->FileSize(*PathB);

		// we want biggest to smallest, so return < 0 if A is bigger
		return (INT)(SizeB - SizeA);
	}
};

TArray<FString> TokenMaps;
TArray<FString> TokenScriptPackages;
TArray<FString> TokenContentPackages;
/** The maps passed in on the command line */
TArray<FString> CommandLineMaps;

/**
 * Performs command line and engine specific initialization.
 *
 * @param	Params	command line
 * @param	bQuitAfterInit [out] If TRUE, the caller will quit the commandlet, even if the Init function returns TRUE
 * @return	TRUE if successful, FALSE otherwise
 */
UBOOL UCookPackagesCommandlet::Init( const TCHAR* Params, UBOOL& bQuitAfterInit )
{
	// Parse command line args.
	ParseCommandLine( Params, Tokens, Switches );

	// Set the "we're cooking" flags.
	GIsCooking				= TRUE;
	GIsCookingForDemo		= Switches.FindItemIndex(TEXT("COOKFORDEMO")) != INDEX_NONE;
	GCookingTarget 			= Platform;

	// Look for -SKIPMAPS command line switch.
	bSkipCookingMaps			= Switches.FindItemIndex(TEXT("SKIPMAPS")) != INDEX_NONE;
	// Look for -INISONLY command line switch
	bIniFilesOnly				= Switches.FindItemIndex(TEXT("INISONLY")) != INDEX_NONE;
#if SHIPPING_PC_GAME
	bGenerateSHAHashes			= FALSE;
#else
	// Check if we are only doing ini files
	bGenerateSHAHashes			= Switches.FindItemIndex(TEXT("SHA")) != INDEX_NONE;
#endif //SHIPPING_PC_GAME
	// Skip saving maps if SKIPSAVINGMAPS is specified, useful for LOC cooking.
	bSkipSavingMaps				= Switches.FindItemIndex(TEXT("SKIPSAVINGMAPS")) != INDEX_NONE;
	// Skip loading & saving packages not required for cooking process to speed up LOC cooking.
	bSkipNotRequiredPackages	= Switches.FindItemIndex(TEXT("SKIPNOTREQUIREDPACKAGES")) != INDEX_NONE;
	// Check for a flag to cook all maps
	bCookAllMaps				= Switches.FindItemIndex(TEXT("COOKALLMAPS")) != INDEX_NONE;
	// Check for flag to recook seekfree
	bForceRecookSeekfree		= Switches.FindItemIndex(TEXT("RECOOKSEEKFREE")) != INDEX_NONE;
	// is this a distributed job?
	bIsDistributed				= Switches.FindItemIndex(TEXT("DISTRIBUTED")) != INDEX_NONE;
	// should we merge job outputs?
	bMergeJobs					= Switches.FindItemIndex(TEXT("MERGEJOBS")) != INDEX_NONE;
	// should non map packages be cooked?
	bCookMapsOnly				= Switches.FindItemIndex(TEXT("MAPSONLY")) != INDEX_NONE;
	// should the shader cache be saved at end instead of after each package?
	bSaveShaderCacheAtEnd		= Switches.FindItemIndex(TEXT("SAVESHADERSATEND")) != INDEX_NONE;
	// Disallow map and package compression if option is set.
	bDisallowPackageCompression	= Switches.FindItemIndex(TEXT("NOPACKAGECOMPRESSION")) != INDEX_NONE;
	bCookCurrentLanguageOnly	= Switches.FindItemIndex(TEXT("NOLOCCOOKING")) != INDEX_NONE;
	bVerifyTextureFileCache		= Switches.FindItemIndex(TEXT("VERIFYTFC")) != INDEX_NONE;
	bLogPersistentFaceFXGeneration = Switches.FindItemIndex(TEXT("LOGPERSISTENTFACEFX")) != INDEX_NONE;
	
#if SHIPPING_PC_GAME
	bIsInUserMode				= TRUE;
#else
	// Check for user mode
	bIsInUserMode				= (Switches.FindItemIndex(TEXT("USER")) != INDEX_NONE) || (Switches.FindItemIndex(TEXT("INSTALLED")) != INDEX_NONE);
#endif

	// Check for user mode
	UBOOL bDumpBulkData			= Switches.FindItemIndex(TEXT("DUMPBULK")) != INDEX_NONE;
	
	// For PS3 mods . . .
	if (bIsInUserMode && (Platform == PLATFORM_PS3 || Platform == PLATFORM_Xenon))
	{
		// Look for a specified mod name and provide a default if none present.
		if (!Parse(Params, TEXT("UserModName="), UserModName))
		{
			UserModName = TEXT("UserMod");
		}
	}

	// Load the 'never cook' list.
	const TCHAR* NeverCookIniSection = TEXT("Cooker.ObjectsToNeverCook");
	TMultiMap<FString,FString>* IniNeverCookList = GConfig->GetSectionPrivate(NeverCookIniSection, FALSE, TRUE, GEditorIni);
	if (IniNeverCookList)
	{
		// Split up the remotes and the pawns...
		for (TMultiMap<FString,FString>::TIterator It(*IniNeverCookList); It; ++It)
		{
			FString TypeString = It.Key();
			FString ValueString = It.Value();

			INT* pCheckVal = NeverCookObjects.Find(ValueString);
			if (!pCheckVal)
			{
				NeverCookObjects.Set(ValueString, 1);
				debugf(TEXT("Adding object to NeverCook list: %s"), *ValueString);
			}
		}
	}

	// Create folder for cooked data.
	CookedDir = appGameDir() + GetCookedDirectory() + PATH_SEPARATOR;
	if( !GFileManager->MakeDirectory( *CookedDir, TRUE ) )
	{
		appDebugMessagef(TEXT("Couldn't create %s"), *CookedDir);
		return FALSE;
	}

	// get the job name, which is used for outputting to unique directories for each job
	if (bIsDistributed)
	{
		INT Bucket = 0;
		if (!bCookMapsOnly)
		{
			// if this is specified, then the list of tokens is a list of non seekfree packages to cook for this job
			if (Switches.FindItemIndex(TEXT("JOBPACKAGES")) != INDEX_NONE)
			{
				// loop over the tokens, look them up in the package cache, and use those as the packages to cook for the job
				for (INT TokenIndex = 0; TokenIndex < Tokens.Num(); TokenIndex++)
				{
					FString TokenPath;
					if (GPackageFileCache->FindPackageFile(*Tokens(TokenIndex), NULL, TokenPath))
					{
						warnf(NAME_Log, TEXT("Adding package %s to the job"), *TokenPath);
						JobPackagesToCook.Add(Tokens(TokenIndex));
					}
				}
			}
			else
			{
				INT NumBuckets = 10;
				Parse(Params, TEXT("NumJobs="), NumBuckets);

				// keep a running total of all package file sizes
				QWORD TotalSizeOfAllPackages = 0;

				// get a list of files for the bucket to pull out of
				TArray<FString> PackageList = GPackageFileCache->GetPackageFileList();
				TArray<FString> UnsortedPackages;
				// first get non-maps
				for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
				{
					FString Ext = FFilename(PackageList(PackageIndex)).GetExtension();
					// don't put maps or script into the job buckets
					if (Ext != FURL::DefaultMapExt && Ext != TEXT("u") && FFilename(PackageList(PackageIndex)).GetBaseFilename() != TEXT("Entry"))
					{
						// remember this valid package
						UnsortedPackages.AddItem(FFilename(PackageList(PackageIndex)).GetBaseFilename());

						// update the total size
						TotalSizeOfAllPackages += GFileManager->FileSize(*PackageList(PackageIndex));
					}
				}

				// calculate average file size
				INT AverageFileSize = TotalSizeOfAllPackages / UnsortedPackages.Num();

				// remember how many packages we have
				INT NumPackages = UnsortedPackages.Num();

				// maximum length of commandline
				const INT MaxCommandLineLen = 1023;

				// sort the packages by size
				Sort<FString, FPackageSizeSorter>(UnsortedPackages.GetTypedData(), UnsortedPackages.Num());

				TArray<TSet<FString> > ValidPackages;
				FLOAT PackagesPerBucket = (FLOAT)NumPackages / (FLOAT)NumBuckets;

				// reorder the list so that it goes big small big small, to average out the size of packages in each job
				INT PackageIndex = 0;
				for (INT JobIndex = 0; JobIndex < NumBuckets; JobIndex++)
				{
					// add an empty TSet
					ValidPackages.AddItem(TSet<FString>());

					// track the size of the job
					INT JobTotalFileSize = 0;
					INT CommandLineSize = 0;

					// calculate last package to put in this job
					INT MaxPackageThisJob = (INT)(PackagesPerBucket * (JobIndex + 1));

					// last bucket gets all remainder (truncating could lose one at end, etc)
					if (JobIndex == NumBuckets - 1)
					{
						MaxPackageThisJob = NumPackages;
					}

					// do the next set of packages (note that PackageIndex doesn't reset)
					for (INT JobPackageIndex = 0; PackageIndex < MaxPackageThisJob; PackageIndex++, JobPackageIndex++)
					{
						// get average so far
						INT AverageJobPackageSize = JobPackageIndex == 0 ? 0 : JobTotalFileSize / JobPackageIndex;

						// pull off from the front if our current job average filesize is less than average filesize
						INT ArrayIndex = (AverageJobPackageSize < AverageFileSize) ? 0 : (UnsortedPackages.Num() - 1);

						// Make sure the commandline doesn't get too long
						if( UnsortedPackages( ArrayIndex ).Len() + CommandLineSize < MaxCommandLineLen )
						{
							CommandLineSize += UnsortedPackages( ArrayIndex ).Len() + 1;

							// update the size of this job
							FString Path;
							GPackageFileCache->FindPackageFile(*UnsortedPackages(ArrayIndex), NULL, Path);
							JobTotalFileSize += GFileManager->FileSize(*Path);

							//warnf(NAME_Log, TEXT("Adding package %s to bucket %d"), *UnsortedPackages(ArrayIndex), JobIndex);

							// pull off and insert into the end result array
							ValidPackages(JobIndex).Add(UnsortedPackages(ArrayIndex));
							UnsortedPackages.Remove(ArrayIndex);
						}
						else
						{
							break;
						}
					}
					//warnf(NAME_Log, TEXT("JobSize = %d [locavg = %d], Avg = %d"), JobTotalFileSize, JobTotalFileSize / PackagesPerBucket, AverageFileSize);
				}
				//warnf(TEXT("%d [%d] Packages, NumPerBucket = %f"), ValidPackages.Num(), NumPackages, PackagesPerBucket);

				if( UnsortedPackages.Num() > 0 )
				{
					appErrorf( NAME_Error, TEXT( "Not enough buckets to hold all packages - please update the number of packages" ) );
				}

				// look for the switch to generate the jobs and quit out
				if (Switches.FindItemIndex(TEXT("GENERATEJOBS")) != INDEX_NONE)
				{
					FString JobConfigFile = appGameDir() + GetCookedDirectory() * TEXT("Jobs.xml");
					// delete any previous one
					GFileManager->Delete(*JobConfigFile);

					FArchive* JobFile = GFileManager->CreateFileWriter( *JobConfigFile );

					FString XmlHeader = TEXT( "<JobDescriptions>\r\n\t<Jobs>\r\n" );
					JobFile->Serialize( TCHAR_TO_ANSI( *XmlHeader ), XmlHeader.Len() );

					// for each bucket, get the list of packages for the job, and write out
					for (INT BucketIndex = 0; BucketIndex < NumBuckets; BucketIndex++)
					{
						// calculate the number of 
						TSet<FString>& JobPackages = ValidPackages(BucketIndex);

						// get the section array for list of packages
						FString Line = FString::Printf( TEXT( "\t\t<JobInfo Name=\"Job_%d\" Command=\"Jobs/CookJob\" Parameter=\"" ), BucketIndex );
						//INT TotalFileSize = 0;
						// dump out packages to the xml file
						for( TSet<FString>::TIterator It( JobPackages ); It; ++It )
						{
							// add the package name to the xml file
							Line += FFilename(*It).GetBaseFilename() + TEXT( " " );
							//FString Path;
							//GPackageFileCache->FindPackageFile(*(*It), NULL, Path);
							//TotalFileSize += GFileManager->FileSize(*Path);
						}
						//warnf(NAME_Log, TEXT("Job %d filesize is %d"), BucketIndex, TotalFileSize);
						Line += TEXT( "\" />\r\n" );
						JobFile->Serialize( TCHAR_TO_ANSI( *Line ), Line.Len() );
					}

					FString XmlFooter = TEXT( "\t</Jobs>\r\n</JobDescriptions>\r\n" );
					JobFile->Serialize( TCHAR_TO_ANSI( *XmlFooter ), XmlFooter.Len() );

					JobFile->Close();

					warnf(NAME_Log, TEXT("\nGenerated job lists to '%s'\n"), *JobConfigFile);
					
					bQuitAfterInit = TRUE;
					return TRUE;
				}
				else
				{
					Parse(Params, TEXT("JobBucket="), Bucket);

					// just use the Nth bucket from commandline
					JobPackagesToCook = ValidPackages(Bucket);

					INT TotalFileSize = 0;
					// dump out packages to the xml file
					for( TSet<FString>::TIterator It( JobPackagesToCook ); It; ++It )
					{
						FString Path;
						GPackageFileCache->FindPackageFile(*(*It), NULL, Path);
						TotalFileSize += GFileManager->FileSize(*Path);
					}
					warnf(NAME_Log, TEXT("Job %d filesize is %d"), Bucket, TotalFileSize);
				}
			}
		}

		// Look for a specified job name and provide a default if none present.
		if (!Parse(Params, TEXT("JobName="), JobName))
		{
			JobName = FString::Printf(TEXT("Job_%d"), Bucket);
		}

		// redirect package writes to the job directory
		FString CookedJobDir = appGameDir() + FString(TEXT("Jobs-")) + GetPlatformString() * JobName * GetCookedDirectory() + PATH_SEPARATOR;
		UObject::SetSavePackagePathRedirection(*CookedDir, *CookedJobDir);

		// consoles need to have their local shader caches redirect also
		if (Platform != PLATFORM_Windows)
		{
			// redirect normal package saving (for local shader caches)
			UObject::SetSavePackagePathRedirection(appGameDir() + TEXT("Content"), appGameDir() + FString(TEXT("Jobs-")) + GetPlatformString() * JobName * TEXT("Content"));
		}
	}

	// Check for -FULL command line switch. If we have passed in -FULL we want to cook all packages
	UBOOL bForceFullRecook = Switches.FindItemIndex(TEXT("FULL")) != INDEX_NONE;

	// Force a full recook if PersistentCookerData doesn't exist or is outdated.
	FString PersistentCookerDataFilename = CookedDir + GetBulkDataContainerFilename();
	if ( !bForceFullRecook )
	{
		UBOOL bFileOutOfDate = TRUE;
		UBOOL bFileExists = GFileManager->FileSize( *PersistentCookerDataFilename ) > 0;
		if ( bFileExists )
		{
			// Create a dummy package for the already existing package.
			UPackage* Package = UObject::CreatePackage( NULL, TEXT( "PersistentCookerDataPackage") );
			// GetPackageLinker will find already existing packages first and we cannot reset loaders at this point
			// so we revert to manually creating the linker load ourselves. Don't do this at home, kids!
			UObject::BeginLoad();
			ULinkerLoad* Linker	= ULinkerLoad::CreateLinker( Package, *PersistentCookerDataFilename, LOAD_NoWarn | LOAD_NoVerify );
			UObject::EndLoad();

			bFileOutOfDate = !Linker || (Linker->Summary.CookedContentVersion != GPackageFileCookedContentVersion);

			// Collect garbage to make sure the file gets closed.
			UObject::CollectGarbage(RF_Native);
		}
		if ( !bFileExists || bFileOutOfDate )
		{
			if ( bIsInUserMode && (Platform == PLATFORM_PS3 || Platform == PLATFORM_Xenon) )
			{
				// Try to load the Epic CookerData!
				const FString ShippingCookedDir = appGameDir() + FString(TEXT("Cooked")) + GetPlatformString() + PATH_SEPARATOR;
				const FString ShippingCookerDataFilename = ShippingCookedDir + GetBulkDataContainerFilename();

				if (GGameCookerHelper)
				{
					PersistentCookerData = GGameCookerHelper->CreateInstance(*ShippingCookerDataFilename, FALSE);
				}
				else
				{
					PersistentCookerData = UPersistentCookerData::CreateInstance( *ShippingCookerDataFilename, FALSE );
				}
				if ( !PersistentCookerData )
				{
					appDebugMessagef(TEXT("Can't load cooker data. (%s) (%s)"), *PersistentCookerDataFilename, *ShippingCookerDataFilename );
					return FALSE;
				}
				// We've loaded the shipping cooked data, so now update the Filename member of
				// PersistentCookerData so that it saves to the mod directory 
				PersistentCookerData->SetFilename( *PersistentCookerDataFilename );
			}
// we never want to delete files in a shipping build unless the user specifically said so
#if !SHIPPING_PC_GAME
			else
			{
				// Make sure we delete all cooked files so that they will be properly recooked
				// even if we abort cooking for whatever reason but PersistentCookerData has been updated on disk.
				warnf(NAME_Log, TEXT("%s is out of date. Forcing a full recook."), *PersistentCookerDataFilename );
			}
			bForceFullRecook = TRUE;
#endif
		}
	}

	// Are we doing a full recook?
	if( bForceFullRecook )
	{
		// We want to make certain there are no outdated packages in existence
		// so we delete all of the files in the CookedDir!
		TArray<FString> AllFiles; 
		appFindFilesInDirectory(AllFiles, *CookedDir, TRUE, TRUE);

		// delete em all
		for (INT FileIndex = 0; FileIndex < AllFiles.Num(); FileIndex++)
		{
			warnf(NAME_Log, TEXT("Deleting: %s"), *AllFiles(FileIndex));
			if (!GFileManager->Delete(*AllFiles(FileIndex)))
			{
				appDebugMessagef(TEXT("Couldn't delete %s"), *AllFiles(FileIndex));
				return FALSE;
			}
		}
	}

	// create the ini files for the target platform (we aren't signing inis on PC)
	CreateIniFiles(Platform & PLATFORM_PC ? FALSE : bGenerateSHAHashes);

	// prepare shaders for compiling
	if ( !PrepareShaderFiles() )
	{
		appDebugMessagef( TEXT("Couldn't prepare shader files") );
		return FALSE;
	}

	if (Platform == PLATFORM_Windows)
	{
		// on windows, we use the current settings since the values in the INI are set for what this machine will play
		// with, not necessarily what it should cook with. We force the current settings to the highest when running
		// the cooker, so just use those.
		PlatformLODSettings = GSystemSettings.TextureLODSettings;
		PlatformDetailMode = GSystemSettings.DetailMode;
	}
	else
	{
		// Initialize LOD settings from the patform's engine ini.
		PlatformLODSettings.Initialize(PlatformEngineConfigFilename, TEXT("SystemSettings"));

		// Initialize platform detail mode.
		GConfig->GetInt( TEXT("SystemSettings"), TEXT("DetailMode"), PlatformDetailMode, PlatformEngineConfigFilename );

	}
	// Initialize global shadow volume setting from Xenon ini
	GConfig->GetBool( TEXT("Engine.Engine"), TEXT("AllowShadowVolumes"), GAllowShadowVolumes, PlatformEngineConfigFilename );

	bSeparateSharedMPResources = FALSE;
	GConfig->GetBool( TEXT("Engine.Engine"), TEXT("bCookSeparateSharedMPSoundContent"), bSeparateSharedMPResources, GEngineIni );
	if ( bSeparateSharedMPResources )
	{
		warnf( NAME_Log, TEXT("Saw bCookSeparateSharedMPSoundContent flag.") );
	}

	// Add a blank line to break up initial logging from cook logging.
	warnf(TEXT(" "));

	if ( (GCookingTarget & PLATFORM_Console) != 0 )
	{
		// determine whether we should cook-out StaticMeshActors
		GConfig->GetBool(TEXT("Engine.StaticMeshCollectionActor"), TEXT("bCookOutStaticMeshActors"), bCookOutStaticMeshActors, PlatformEngineConfigFilename);
		bCookOutStaticMeshActors = (bCookOutStaticMeshActors || Switches.ContainsItem(TEXT("RemoveStaticMeshActors"))) && !Switches.ContainsItem(TEXT("KeepStaticMeshActors"));
		if ( bCookOutStaticMeshActors )
		{
			debugf(NAME_Log, TEXT("StaticMeshActors will be removed from cooked files."));
		}
		else
		{
			debugf(NAME_Log, TEXT("StaticMeshActors will NOT be removed from cooked files."));
		}

		// determine whether we should cook-out static Lights
		GConfig->GetBool(TEXT("Engine.StaticLightCollectionActor"), TEXT("bCookOutStaticLightActors"), bCookOutStaticLightActors, PlatformEngineConfigFilename);
		bCookOutStaticLightActors = (bCookOutStaticLightActors || Switches.ContainsItem(TEXT("RemoveStaticLights"))) && !Switches.ContainsItem(TEXT("KeepStaticLights"));
		if ( bCookOutStaticLightActors )
		{
			debugf(NAME_Log, TEXT("Static Light actors will be removed from cooked files."));
		}
		else
		{
			debugf(NAME_Log, TEXT("Static Light actors will NOT be removed from cooked files."));
		}

		GConfig->GetBool(TEXT("Cooker.MatineeOptions"), TEXT("bBakeAndPruneDuringCook"), bBakeAndPruneDuringCook, GEditorIni);
		bBakeAndPruneDuringCook = bBakeAndPruneDuringCook || Switches.ContainsItem(TEXT("BakeAndPrune"));
		if (bBakeAndPruneDuringCook)
		{
			debugf(NAME_Log, TEXT("Matinees will be baked and pruned when tagged as such."));
		}
		else
		{
			debugf(NAME_Log, TEXT("Matinees will be NOT baked and pruned when tagged as such."));
		}

		GConfig->GetBool(TEXT("Cooker.FaceFXOptions"), TEXT("bGeneratePersistentMapAnimSet"), bGeneratePersistentMapAnimSet, GEditorIni);
		bGeneratePersistentMapAnimSet = bGeneratePersistentMapAnimSet || Switches.ContainsItem(TEXT("PersistentAnimSet"));
		if (bGeneratePersistentMapAnimSet)
		{
			debugf(NAME_Log, TEXT("FaceFX animations will be pulled into the persistent map."));
		}
		else
		{
			debugf(NAME_Log, TEXT("FaceFX animation will be NOT be pulled into the persistent map."));
		}
		FaceFXMangledNameCount = 0;
		CurrentPersistentMapFaceFXArray = NULL;
		CurrentPMapGroupAnimLookupMap = NULL;
	}

	if (bIniFilesOnly)
	{
		warnf(NAME_Log, TEXT("Cooked .ini and localization files."));
		// return false so further processing is not done
		return TRUE;
	}

	if (Platform == PLATFORM_Xenon)
	{
		// get the texture gamma conversion setting
		GConfig->GetBool(TEXT("Engine.PWLGamma"), TEXT("bShouldConvertPWLGamma"), bShouldConvertPWLGamma, PlatformEngineConfigFilename);
		if( bShouldConvertPWLGamma )
		{
			GCreationFlags |= TextureCreate_PWLGamma;
		}
		// Change physics to cook content for Xenon target.
		SetPhysCookingXenon();
	}
	else if (Platform == PLATFORM_PS3)
	{
		// Change physics to cook content for PS3 target.
		SetPhysCookingPS3();

		// set the compression chunk size to 64k to fit in SPU local ram
		extern INT GSavingCompressionChunkSize;
		GSavingCompressionChunkSize = 64 * 1024;
	}

	// when cooking a mod for PS3, we need to get the shipped cooker data to know where a cooked texture came from
	// (we need a separate instance that cannot contain any mod texture info)
	if (bIsInUserMode && ((Platform == PLATFORM_PS3) || (Platform == PLATFORM_Xenon)))
	{
		const FString ShippingCookedDir = appGameDir() + FString(TEXT("Cooked")) + GetPlatformString() + PATH_SEPARATOR;

		// hardcode the filename because we know what we shipped with :)
		const FString ShippingCookerDataFilename = ShippingCookedDir + GetBulkDataContainerFilename();

		// create the instance for the shipped info
		if (GGameCookerHelper)
		{
			GShippingCookerData = GGameCookerHelper->CreateInstance(*ShippingCookerDataFilename, FALSE);
		}
		else
		{
			GShippingCookerData = UPersistentCookerData::CreateInstance( *ShippingCookerDataFilename, FALSE );
		}

		checkf(GShippingCookerData, TEXT("Failed to find shipping global cooker info file [%s]"), *ShippingCookerDataFilename);
	}

	// recompile the global shaders if we need to
	if( Switches.FindItemIndex(TEXT("RECOMPILEGLOBALSHADERS")) != INDEX_NONE )
	{
		extern TShaderMap<FGlobalShaderType>* GGlobalShaderMap[SP_NumPlatforms];	
		delete GGlobalShaderMap[ShaderPlatform];
		GGlobalShaderMap[ShaderPlatform] = NULL;
	}

	// Keep track of time spent loading packages.
	DOUBLE StartTime = appSeconds();

	{
		SCOPE_SECONDS_COUNTER(LoadNativePackagesTime);
		// Load up all native script files, not excluding game specific ones.
		LoadAllNativeScriptPackages( FALSE );
	}
		
	// Make sure that none of the loaded script packages get garbage collected.
	for( FObjectIterator It; It; ++It )
	{
		UObject* Object = *It;
		// Check for code packages and add them and their contents to root set.
		if( Object->GetOutermost()->PackageFlags & PKG_ContainsScript )
		{
			Object->AddToRoot();
		}
	}

	// get bool indicating whether standalone seekfree packages must be server side only
	UBOOL bStandaloneSFServerSideOnly = FALSE;
	GConfig->GetBool(TEXT("Engine.PackagesToAlwaysCook"), TEXT("bStandaloneSFServerSideOnly"), bStandaloneSFServerSideOnly, PlatformEngineConfigFilename);

	// read in the per-map packages to cook
	TMap<FName, TArray<FName> > PerMapCookPackages;
	GConfig->Parse1ToNSectionOfNames(TEXT("Engine.PackagesToForceCookPerMap"), TEXT("Map"), TEXT("Package"), PerMapCookPackages, PlatformEngineConfigFilename);

	// Retrieve list of editor-only package names.
	GConfig->GetArray(TEXT("Editor.EditorEngine"), TEXT("EditorOnlyContentPackages"), EditorOnlyContentPackageNames, GEngineIni);
	
	if( GCookingTarget & PLATFORM_Console )
	{
		// List of packages we removed.
		TArray<FString> RemovedPackageNames;

		// For each package in editor-only list, find all objects inside and remove from root and mark as pending kill.
		// This will allow the garbage collector to purge references to them.
		for( INT NameIndex=0; NameIndex<EditorOnlyContentPackageNames.Num(); NameIndex++ )
		{
			// Check whether package is loaded. This will also find subgroups of the same name so we verify that it is a toplevel package.
			UPackage* Package = FindObject<UPackage>(ANY_PACKAGE,*EditorOnlyContentPackageNames(NameIndex));
			if( Package && Package->GetOutermost() == Package )
			{
				// Keep track of what we're removing so we can verify.
				new(RemovedPackageNames) FString(*EditorOnlyContentPackageNames(NameIndex));

				// Iterate over all objects inside package and mark them as pending kill and remove from root.
				for( FObjectIterator It; It; ++It )
				{
					UObject* Object = *It;
					if( Object->IsIn( Package ) )
					{
						Object->RemoveFromRoot();
						Object->MarkPendingKill();
					}
				}
				// Last but not least, do the same for the package.
				Package->RemoveFromRoot();
				Package->MarkPendingKill();
			}
		}

		// This should purge all editor-only packages and content.
		UObject::CollectGarbage(RF_Native);

		// Verify removal of packages we tried to remove.
		for( INT PackageIndex=0; PackageIndex<RemovedPackageNames.Num(); PackageIndex++ )
		{
			UPackage* Package = FindObject<UPackage>(ANY_PACKAGE,*RemovedPackageNames(PackageIndex));
			if( Package && Package->GetOutermost() == Package )
			{
				warnf(TEXT("Unable to remove editor-only package %s"),*Package->GetName());
			}
		}
	}

	// Iterate over all objects and mark them to be excluded from being put into the seek free package.
	for( FObjectIterator It; It; ++It )
	{
		UObject*		Object				= *It;
		UPackage*		Package				= Cast<UPackage>(Object);
		ULinkerLoad*	LinkerLoad			= Object->GetLinker();
		
		// Toplevel packages don't have a linker so we try to find it unless we know it's not a package or it is
		// the transient package.
		if( !LinkerLoad && Package && Package != UObject::GetTransientPackage() && !Package->HasAnyFlags(RF_ClassDefaultObject) )
		{
			SCOPE_SECONDS_COUNTER(LoadPackagesTime);
			UObject::BeginLoad();
			// GetPackageLinker will throw an exception when it cant find a file for the associated package.
			// This is a problem for packages of intrinsic classes, which don't exist on disk.  So, we
			// simply intercept these exceptions and NULL out the linker.
			try
			{
				LinkerLoad = UObject::GetPackageLinker( Package, NULL, LOAD_NoWarn, NULL, NULL );
			}
			catch(...)
			{
				LinkerLoad = NULL;
			}
			UObject::EndLoad();
		}

		// Mark objects that reside in a code package, are native, a default object or transient to not be put 
		// into the seek free package.
		if( (LinkerLoad && LinkerLoad->ContainsCode()) || Object->HasAnyFlags(RF_Transient|RF_Native|RF_ClassDefaultObject))
		{
			Object->SetFlags( RF_MarkedByCooker );
		}
	}

	// packages needed just to start the cooker, which we don't want to cook when in user mode
	TMap<FString,INT> InitialDependencies;
	if (bIsInUserMode)
	{
		// mark that all packages currently loaded (ie before loading usermode commandline specified packages) shouldn't be cooked later
		for( FObjectIterator It; It; ++It )
		{
			UObject* Object = *It;
			if( Object->GetLinker() )
			{
				// We need to (potentially) cook this package.
				InitialDependencies.Set( *Object->GetLinker()->Filename, 0 );
			}
		}
	}


	// create or open Guid cache file (except when cooking mods for PC, they just look in the packages for GUIDs)
	if (bIsInUserMode && Platform == PLATFORM_Windows)
	{
		GuidCache = NULL;
	}
	else
	{
		const FString GuidCacheFilename( CookedDir + GetGuidCacheFilename() );
		GuidCache = UGuidCache::CreateInstance( *GuidCacheFilename );
		check( GuidCache );
	}

	if( !PersistentCookerData )
	{
		// Create container helper object for keeping track of where bulk data ends up inside cooked packages. This is needed so we can fix
		// them up in the case of forced exports that don't need their bulk data inside the package they were forced into.
		//		PersistentCookerData = UPersistentCookerData::CreateInstance( *PersistentCookerDataFilename, TRUE );
		if (GGameCookerHelper)
		{
			PersistentCookerData = GGameCookerHelper->CreateInstance(*PersistentCookerDataFilename, TRUE );
		}
		else
		{
			PersistentCookerData = UPersistentCookerData::CreateInstance( *PersistentCookerDataFilename, TRUE );
		}
		check( PersistentCookerData );
	}

	// Check whether we only want to cook passed in packages and their dependencies, unless
	// it was specified to cook all packages
//	bOnlyCookDependencies =  bIsInUserMode;
	bOnlyCookDependencies = FALSE;
	if( bOnlyCookDependencies )
	{
		SCOPE_SECONDS_COUNTER(LoadDependenciesTime);

		// Iterate over all passed in packages, loading them.
		for( INT TokenIndex=0; TokenIndex<Tokens.Num(); TokenIndex++ )
		{
			const FString& Token = Tokens(TokenIndex);

			// Load package if found.
			FString PackageFilename;
			UPackage* Result = NULL;
			if( GPackageFileCache->FindPackageFile( *Token, NULL, PackageFilename ) )
			{
				warnf(NAME_Log, TEXT("Loading base level %s"), *PackageFilename);
				Result = LoadPackageForCooking(*PackageFilename);
				if (Result == NULL)
				{
					warnf(NAME_Error, TEXT("Failed to load base level %s"), *Token);
					return FALSE;
				}

				// Classify packages specified on the command line.
				if ( Result->PackageFlags & PKG_ContainsMap )
				{
					TokenMaps.AddItem( PackageFilename );
					CommandLineMaps.AddItem(PackageFilename);
				}
				else if ( Result->PackageFlags & PKG_ContainsScript )
				{
					TokenScriptPackages.AddItem( PackageFilename );
				}
				else
				{
					TokenContentPackages.AddItem( PackageFilename );
				}

				// add dependencies for the per-map packages for this map (if any)
				TArray<FName>* Packages = PerMapCookPackages.Find(Result->GetFName());
				if (Packages != NULL)
				{
					for (INT PackageIndex = 0; PackageIndex < Packages->Num(); PackageIndex++)
					{
						FName PackageName = (*Packages)(PackageIndex);
						FString PackageFilename;
						if( GPackageFileCache->FindPackageFile( *PackageName.ToString(), NULL, PackageFilename ) )
						{
							if (!ForceLoadPackage(PackageFilename))
							{
								return FALSE;
							}
						}
					}
				}
			}
			if (Result == NULL)
			{
				warnf(NAME_Error, TEXT("Failed to load base level %s"), *Token);
				return FALSE;
			}
		}

		TArray<FString> SubLevelFilenames;

		// Iterate over all UWorld objects and load the referenced levels.
		for( TObjectIterator<UWorld> It; It; ++It )
		{
			UWorld*		World		= *It;
			AWorldInfo* WorldInfo	= World->GetWorldInfo();
			// Iterate over streaming level objects loading the levels.
			for( INT LevelIndex=0; LevelIndex<WorldInfo->StreamingLevels.Num(); LevelIndex++ )
			{
				ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
				if( StreamingLevel )
				{
					// Load package if found.
					FString PackageFilename;
					if( GPackageFileCache->FindPackageFile( *StreamingLevel->PackageName.ToString(), NULL, PackageFilename ) )
					{
						SubLevelFilenames.AddItem(*PackageFilename);
					}
				}
			}
		}

		for (INT SubLevelIndex = 0; SubLevelIndex < SubLevelFilenames.Num(); SubLevelIndex++)
		{
			warnf(NAME_Log, TEXT("Loading sub-level %s"), *SubLevelFilenames(SubLevelIndex));
			LoadPackageForCooking(*SubLevelFilenames(SubLevelIndex));
		}

		// Iterate over all objects and add the filename of their linker (if existing) to the dependencies map.
		for( FObjectIterator It; It; ++It )
		{
			UObject* Object = *It;
			if( Object->GetLinker() )
			{
				// by default, all loaded object's packages are valid dependencies
				UBOOL bIsValidDependency = TRUE;

				// handle special cases for user mode
				if (bIsInUserMode)
				{
					// don't cook packages during startup, or already cooked packages
					if (InitialDependencies.Find(Object->GetLinker()->Filename) != NULL ||
						(Object->GetOutermost()->PackageFlags & PKG_Cooked) || 
						(Object->GetLinker()->Summary.PackageFlags & PKG_Cooked))
					{
						bIsValidDependency = FALSE;
					}
				}

				// if allowed, put this objects package into the list of dependent packages
				if (bIsValidDependency)
				{
					// We need to (potentially) cook this package.
					PackageDependencies.Set( *Object->GetLinker()->Filename, 0 );
				}
			}
		}
	}
	else
	{
		// add the always cook list to the tokens to cook
		if (!bIsDistributed)
		{
			TArray<FString>				AlwaysCookPackages;
			LoadSectionPackages(TEXT("Engine.PackagesToAlwaysCook"), AlwaysCookPackages, TEXT("Package"), TRUE);
			for ( INT PackageIdx = 0; PackageIdx < AlwaysCookPackages.Num(); PackageIdx++ )
			{
				Tokens.AddUniqueItem(AlwaysCookPackages(PackageIdx));
			}
		}

		// Figure out the list of maps to check for need of cooking
		for( INT TokenIndex=0; TokenIndex<Tokens.Num(); TokenIndex++ )
		{
			FString PackageFilename;
			if( GPackageFileCache->FindPackageFile(*Tokens(TokenIndex), NULL, PackageFilename))
			{
				TArray<FString>* ListOfPackages;

				// add the token to the appropriate list along wiht any additional packages stored in the 
				// package file summary

				// read the package file summary of the package to get a list of sublevels
				FArchive* PackageFile = GFileManager->CreateFileReader(*PackageFilename);
				if (PackageFile)
				{
					// read the package summary, which has list of sub levels
					FPackageFileSummary Summary;
					(*PackageFile) << Summary;

					// close the map
					delete PackageFile;

					if ( (Summary.PackageFlags&PKG_ContainsMap) != 0 )
					{
						warnf(NAME_Log, TEXT("Adding level %s for cooking..."), *PackageFilename);
						ListOfPackages = &TokenMaps;
						CommandLineMaps.AddItem(PackageFilename);
					}
					else if ( (Summary.PackageFlags&PKG_ContainsScript) != 0 )
					{
						warnf(NAME_Log, TEXT("Adding script %s for cooking..."), *PackageFilename);
						ListOfPackages = &TokenScriptPackages;
					}
					else
					{
						warnf(NAME_Log, TEXT("Adding package %s for cooking..."), *PackageFilename);
						ListOfPackages = &TokenContentPackages;
					}

					ListOfPackages->AddUniqueItem(PackageFilename);

					// if it's an old map, then we have to load it to get the list of sublevels
					if (Summary.GetFileVersion() < VER_ADDITIONAL_COOK_PACKAGE_SUMMARY)
					{
						SCOPE_SECONDS_COUNTER(LoadPackagesTime);

						if (ListOfPackages == &TokenMaps)
						{
							warnf(NAME_Log, TEXT("  Old package, so must open fully to look for sublevels"), *PackageFilename);
							UPackage* Package = UObject::LoadPackage(NULL, *PackageFilename, LOAD_None);

							// Iterate over all UWorld objects and load the referenced levels.
							for( TObjectIterator<UWorld> It; It; ++It )
							{
								UWorld*		World		= *It;
								if (World->IsIn(Package))
								{
									AWorldInfo* WorldInfo	= World->GetWorldInfo();
									// Iterate over streaming level objects loading the levels.
									for (INT LevelIndex = 0; LevelIndex < WorldInfo->StreamingLevels.Num(); LevelIndex++)
									{
										ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
										if( StreamingLevel )
										{
											// Load package if found.
											FString SubPackageFilename;
											if (GPackageFileCache->FindPackageFile(*StreamingLevel->PackageName.ToString(), NULL, SubPackageFilename))
											{
												warnf(NAME_Log, TEXT("Adding sublevel %s for cooking..."), *SubPackageFilename);
												ListOfPackages->AddUniqueItem(SubPackageFilename);
											}
										}
									}
								}
							}

							// close the package
							CollectGarbageAndVerify();
						}
					}
					else
					{
						// tack the streaming levels onto the list of maps to cook
						for (INT AdditionalPackageIndex = 0; AdditionalPackageIndex < Summary.AdditionalPackagesToCook.Num(); AdditionalPackageIndex++)
						{
							if( GPackageFileCache->FindPackageFile(*Summary.AdditionalPackagesToCook(AdditionalPackageIndex), NULL, PackageFilename))
							{
								warnf(NAME_Log, TEXT("Adding additional package %s for cooking..."), *PackageFilename);
								ListOfPackages->AddUniqueItem(PackageFilename);
							}
						}
					}
				}
			}
		}
	}

	if (bDumpBulkData)
	{
		PersistentCookerData->DumpBulkDataInfos();
		return FALSE;
	}

	// Check whether we are using the texture file cache or not.
	GConfig->GetBool(TEXT("TextureStreaming"), TEXT("UseTextureFileCache"), bUseTextureFileCache, PlatformEngineConfigFilename);
	// Alignment for bulk data stored in the texture file cache. This will result in alignment overhead and will affect the size of the TFC files generated
	INT TempTextureFileCacheBulkDataAlignment = DVD_ECC_BLOCK_SIZE;
	GConfig->GetInt(TEXT("TextureStreaming"), TEXT("TextureFileCacheBulkDataAlignment"), TempTextureFileCacheBulkDataAlignment, PlatformEngineConfigFilename);
	TextureFileCacheBulkDataAlignment = Max<INT>(TempTextureFileCacheBulkDataAlignment,0);

	// currently the xbox doesn't handle dlc TFC files
	/*
	if (bIsInUserMode && Platform == PLATFORM_Xenon)
	{
		bUseTextureFileCache = FALSE;
	}
	*/

	// 'Game' cooker helper call to Init
	if (GGameCookerHelper)
	{
		GGameCookerHelper->Init(this, Tokens, Switches);
	}

	// Compile all global shaders.
	VerifyGlobalShaders( ShaderPlatform );

	// Make sure shader caches are loaded for the target platform before we start processing any materials
	// Without this, the first material (DefaultMaterial) will be recompiled every cook
	GetLocalShaderCache( ShaderPlatform );

	// Disabled for this check-in
#if WITH_FACEFX
	if (bGeneratePersistentMapAnimSet)
	{
		GeneratePersistentMapList(CommandLineMaps);
	}
#endif	//#if WITH_FACEFX

	return TRUE;
}

/**
* Get the destination filename for the given source package file based on platform
*
* @param SrcFilename - source package file path
* @return cooked filename destination path
*/
FFilename UCookPackagesCommandlet::GetCookedPackageFilename( const FFilename& SrcFilename )
{
	// Dest pathname depends on platform
	FFilename			DstFilename;
	if( GIsCookingForDemo )
	{
		DstFilename = CookedDir + SrcFilename.GetCleanFilename();
	}
	else if( Platform == PLATFORM_Windows )
	{
		// get top level directory under appGameDir of the source (..\ExampleGame\) or ..\Engine
		FFilename AfterRootDir;
		// look for packages in engine
		FString RootDir = TEXT("..\\Engine\\");
		if (SrcFilename.StartsWith(RootDir))
		{
			AfterRootDir = SrcFilename.Right(SrcFilename.Len() - RootDir.Len());
		}
		// if it's not in Engine, it needs to be in the appGameDir
		else
		{
			RootDir = appGameDir();
			check(SrcFilename.StartsWith(RootDir));
			AfterRootDir = SrcFilename.Right(SrcFilename.Len() - RootDir.Len());
		}
		// now skip over the first directory after which ever root directory it is (ie Content)
		FFilename AfterTopLevelDir = AfterRootDir.Right(AfterRootDir.Len() - AfterRootDir.InStr(PATH_SEPARATOR));
		DstFilename = CookedDir + AfterTopLevelDir;
	}
	else
	{
		DstFilename = CookedDir + SrcFilename.GetBaseFilename() + TEXT(".xxx");
	}
	return DstFilename;
}



/**
 * Check if any dependencies of a seekfree package are newer than the cooked seekfree package.
 * If they are, the package needs to be recooked.
 *
 * @param SrcLinker Optional source package linker to check dependencies for when no src file is available
 * @param SrcFilename Name of the source of the seekfree package
 * @param DstTimestamp Timestamp of the cooked version of this package
 *
 * @return TRUE if any dependencies are newer, meaning to recook the package
 */
UBOOL UCookPackagesCommandlet::AreSeekfreeDependenciesNewer(ULinkerLoad* SrcLinker, const FString& SrcFilename, DOUBLE DstTimestamp)
{
	SCOPE_SECONDS_COUNTER(CheckDependentPackagesTime);

	// use source package linker if available
	ULinkerLoad* Linker = SrcLinker;
	// open the linker of the source package (this is pretty fast)
	if( !Linker )
	{	
		UObject::BeginLoad();
		Linker = UObject::GetPackageLinker(NULL, *SrcFilename, LOAD_NoVerify, NULL, NULL);
		UObject::EndLoad();
	}

	// gather all the unique dependencies for all the objects in the package
	TSet<FDependencyRef> ObjectDependencies;
	for (INT ExportIndex = 0; ExportIndex < Linker->ExportMap.Num(); ExportIndex++)
	{
		Linker->GatherExportDependencies(ExportIndex, ObjectDependencies);
	}

	// now figure out just the packages from the list
	TSet<FName> DependentPackages;
	for( TSet<FDependencyRef>::TConstIterator It(ObjectDependencies); It; ++It )
	{
		const FDependencyRef& Ref = *It;
		DependentPackages.Add(Ref.Linker->LinkerRoot->GetFName());
	}

	UBOOL bHasNewerDependentPackage = FALSE;
	// look for any newer dependent packages
	for( TSet<FName>::TConstIterator It(DependentPackages); It; ++It )
	{
		const FName& PackageName = *It;
		FString Path;
		if (GPackageFileCache->FindPackageFile(*PackageName.ToString(), NULL, Path))
		{
			DOUBLE Timestamp = GFileManager->GetFileTimestamp(*Path);

			if (Timestamp > DstTimestamp)
			{
				bHasNewerDependentPackage = TRUE;
				break;
			}
		}
	}

	// return if there were any newer dependent packages
	return bHasNewerDependentPackage;
}

/**
 * Generates list of src/ dst filename mappings of packages that need to be cooked after taking the command
 * line options into account.
 *
 * @param [out] FirstStartupIndex		index of first startup package in returned array, untouched if there are none
 * @param [out]	FirstScriptIndex		index of first script package in returned array, untouched if there are none
 * @param [out] FirstGameScriptIndex	index of first game script package in returned array, untouched if there are none
 * @param [out] FirstMapIndex			index of first map package in returned array, untouched if there are none
 * @param [out] FirstMPMapIndex			index of first map package in returned array, untouched if there are none
 *
 * @return	array of src/ dst filename mappings for packages that need to be cooked
 */
TArray<FPackageCookerInfo> UCookPackagesCommandlet::GeneratePackageList( INT& FirstStartupIndex, INT& FirstScriptIndex, INT& FirstGameScriptIndex, INT& FirstMapIndex, INT& FirstMPMapIndex )
{
	// Split into two to allow easy sorting via array concatenation at the end.
	TArray<FPackageCookerInfo>	NotRequiredFilenamePairs;
	TArray<FPackageCookerInfo>	MapFilenamePairs;
	// Maintain a separate list of multiplayer maps.
	TArray<FPackageCookerInfo>	MPMapFilenamePairs;
	TArray<FPackageCookerInfo>	ScriptFilenamePairs;
	TArray<FPackageCookerInfo>	StartupFilenamePairs;
	TArray<FPackageCookerInfo>	StandaloneSeekfreeFilenamePairs;
	// unused (here for helper function)
	TArray<FPackageCookerInfo>	RegularFilenamePairs;


	// Get a list of all script package names split by type, excluding editor- only ones.
	TArray<FString>				EngineScriptPackageNames;
	TArray<FString>				GameNativeScriptPackageNames;
	TArray<FString>				GameScriptPackageNames;
	appGetEngineScriptPackageNames( EngineScriptPackageNames, Platform == PLATFORM_Windows );
	appGetGameNativeScriptPackageNames( GameNativeScriptPackageNames, Platform == PLATFORM_Windows );
	appGetGameScriptPackageNames( GameScriptPackageNames, Platform == PLATFORM_Windows );

	// Get combined list of all script package names, including editor- only ones.
	TArray<FString>				AllScriptPackageNames;
	appGetEngineScriptPackageNames( AllScriptPackageNames, TRUE );
	appGetGameNativeScriptPackageNames( AllScriptPackageNames, TRUE );
	appGetGameScriptPackageNames( AllScriptPackageNames, TRUE );

	TArray<FString>				AllStartupPackageNames;
	// get all the startup packages that the runtime will use (so we need to pass it the platform-specific engine config name)
	appGetAllPotentialStartupPackageNames(AllStartupPackageNames, PlatformEngineConfigFilename);

	// get a list of the seekfree, always cook maps, but don't reload them
	TArray<FString>				SeekFreeAlwaysCookMaps;
	LoadSectionPackages(TEXT("Engine.PackagesToAlwaysCook"), SeekFreeAlwaysCookMaps, TEXT("SeekFreePackage"), TRUE);
	
	TArray<FString> PackageList = GPackageFileCache->GetPackageFileList();

	// if we are cooking a language other than default, always cook the script and starutp packages, because 
	// dependency checking may not catch that it needs to regenerate the _LOC_LANG files since the base package
	// was already cooked
	UBOOL bForceCookNativeScript = bForceRecookSeekfree || (appStricmp(UObject::GetLanguage(), TEXT("INT")) != 0);
	UBOOL bForceCookMaps = bForceRecookSeekfree || (appStricmp(UObject::GetLanguage(), TEXT("INT")) != 0);
	UBOOL bForceCookStartupPackage = bForceRecookSeekfree || (appStricmp(UObject::GetLanguage(), TEXT("INT")) != 0);
	UBOOL bShouldLogSkippedPackages = ParseParam( appCmdLine(), TEXT("LOGSKIPPEDPACKAGES") );

	// split the package iteration in two passes, one for normal packages, one for native script
	// this will allow bForceCookNativeScript to be set if needed before deciding what to do with 
	// the native script packages
	for (INT Pass = 0; Pass < 2; Pass++)
	{
		for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
		{
			FFilename SrcFilename = PackageList(PackageIndex);
			FFilename DstFilename = GetCookedPackageFilename(SrcFilename);
			// We store cooked packages in a folder that is outside the Package search path and use an "unregistered" extension.
			FFilename BaseSrcFilename = SrcFilename.GetBaseFilename();	

			// if we are cooking a limited set of packages, make sure it's in the set
			if (JobPackagesToCook.Num() != 0 && !JobPackagesToCook.Contains(BaseSrcFilename))
			{
				continue;
			}

			// Check whether we are autosave, PIE, a manual dependency or a map file.
			UBOOL	bIsShaderCacheFile		= FString(*SrcFilename).ToUpper().InStr( TEXT("SHADERCACHE") ) != INDEX_NONE;
			UBOOL	bIsAutoSave				= FString(*SrcFilename).ToUpper().InStr( TEXT("AUTOSAVES") ) != INDEX_NONE;
			UBOOL	bIsPIE					= bIsAutoSave && FString(*SrcFilename).InStr( PLAYWORLD_CONSOLE_BASE_PACKAGE_PREFIX ) != INDEX_NONE;
			UBOOL	bIsADependency			= PackageDependencies.Find( SrcFilename ) != NULL;
			UBOOL	bIsEngineScriptFile		= EngineScriptPackageNames.FindItemIndex( BaseSrcFilename ) != INDEX_NONE;
			UBOOL	bIsGameNativeScriptFile	= GameNativeScriptPackageNames.FindItemIndex( BaseSrcFilename ) != INDEX_NONE;
			UBOOL	bIsNativeScriptFile		= bIsEngineScriptFile || bIsGameNativeScriptFile;
			UBOOL	bIsNonNativeScriptFile	= !bIsGameNativeScriptFile && GameScriptPackageNames.FindItemIndex( BaseSrcFilename ) != INDEX_NONE;
			UBOOL	bIsEditorScriptFile		= !(bIsNativeScriptFile || bIsNonNativeScriptFile) && AllScriptPackageNames.FindItemIndex( BaseSrcFilename ) != INDEX_NONE;
			UBOOL	bIsMapFile				= (SrcFilename.GetExtension() == FURL::DefaultMapExt) || (BaseSrcFilename == TEXT("entry"));
			UBOOL	bIsStartupPackage		= AllStartupPackageNames.FindItemIndex( BaseSrcFilename ) != INDEX_NONE;
			UBOOL   bIsScriptFile           = bIsNativeScriptFile || bIsNonNativeScriptFile ||  bIsEditorScriptFile;
			UBOOL	bIsCombinedStartupPackage = bIsStartupPackage && !bIsMapFile && !bIsScriptFile;
			UBOOL	bIsStandaloneSeekfree	= SeekFreeAlwaysCookMaps.FindItemIndex(BaseSrcFilename) != INDEX_NONE;
			UBOOL	bIsSeekfree				= bIsMapFile || bIsNativeScriptFile || bIsCombinedStartupPackage || bIsStandaloneSeekfree;

			// only do maps on second pass (and process startup packages/native script in both passes)
			if (Pass == (bIsMapFile ? 0 : 1) && !bIsCombinedStartupPackage && !bIsNativeScriptFile)
			{
				continue;
			}


			UBOOL	bIsTokenMap				= TokenMaps.ContainsItem(SrcFilename);
			UBOOL	bIsTokenScriptPackage	= TokenScriptPackages.ContainsItem(SrcFilename);
			UBOOL	bIsTokenContentPackage	= TokenContentPackages.ContainsItem(SrcFilename);
			UBOOL	bIsCommandLinePackage	= bIsTokenMap || bIsTokenScriptPackage || bIsTokenContentPackage;

			// Compare file times.
			DOUBLE	DstFileTime				= PersistentCookerData->GetFileTime( *DstFilename );
			DOUBLE	SrcFileTime				= GFileManager->GetFileTimestamp( *SrcFilename );
			UBOOL	DstFileExists			= GFileManager->FileSize( *DstFilename ) > 0;
			UBOOL	DstFileNewer			= (DstFileTime >= 0) && (DstFileTime >= SrcFileTime);// && !bIsScriptFile;

			// It would be nice if we could just rely on the map extension to be set but in theory we would have to load up the package
			// and see whether it contained a UWorld object to be sure. Checking the map extension with a FindObject should give us
			// very good coverage and should always work in the case of dependency cooking. In theory we could also use LoadObject albeit
			// at the cost of having to create linkers for all existing packages.
			if( FindObject<UWorld>( NULL, *FString::Printf(TEXT("%s.TheWorld"),*BaseSrcFilename) ) )
			{
				bIsMapFile = TRUE;
			}
			// Special case handling for Entry.upk.
			if( BaseSrcFilename == TEXT("Entry") )
			{
				bIsMapFile = TRUE;
				bIsTokenMap = bIsTokenMap || bIsTokenContentPackage;
			}

			// Skip over shader caches.
			if( bIsShaderCacheFile )
			{
				if( bShouldLogSkippedPackages )
				{
					warnf(NAME_Log, TEXT("Skipping shader cache %s"), *SrcFilename);
				}
				continue;
			}
			// Skip over packages we don't depend on if we're only cooking dependencies .
			else if( bOnlyCookDependencies && !bIsADependency )
			{
				if( bShouldLogSkippedPackages )
				{
					warnf(NAME_Log, TEXT("Skipping %s"), *SrcFilename);
				}
				continue;
			}
			// Skip cooking maps if wanted.
			else if( bSkipCookingMaps && bIsMapFile )
			{
				if( bShouldLogSkippedPackages )
				{
					warnf(NAME_Log, TEXT("Skipping %s"), *SrcFilename);
				}
				continue;
			}
			else if ( !bIsMapFile && bCookMapsOnly)
			{
				continue;
			}
			// Only cook maps on the commandline, or all maps
			// if we are doing a phase 1 distributed cook, don't cook ANY maps (Entry.upk, I'm looking at you, which confuses things from the extension)
			else if( !bCookAllMaps && bIsMapFile && (!bIsTokenMap || (bIsDistributed && !bCookMapsOnly)) )
			{
				if( bShouldLogSkippedPackages )
				{
					warnf(NAME_Log, TEXT("Skipping %s"), *SrcFilename);
				}
				continue;
			}
			// Skip over autosaves, unless it is a PlayInEditor file.
			else if( bIsAutoSave && !bIsPIE )
			{
				if( bShouldLogSkippedPackages )
				{
					warnf(NAME_Log, TEXT("Disregarding %s"), *SrcFilename);
				}
				continue;
			}
			// Skip over editor script packages.
			else if( bIsEditorScriptFile )
			{
				continue;
			}
			// Skip over packages not needed for users
			else if ( bIsInUserMode )
			{
				// skip over any startup (which prolly haven't been loaded, but just in case), and script files
				// @todo: need to handle a command-line specified script package for code-only mods!
				if ( bIsStartupPackage || bIsScriptFile)
				{
					continue;
				}
			}


			// look for a language extension, _XXX
			INT Underscore = BaseSrcFilename.InStr(TEXT("_"), TRUE);
			UBOOL bIsWrongLanguage = FALSE;
			// don't bother filtering on language if we are doing a distributed cook - we cook all packages then
			if (Underscore != INDEX_NONE && !bIsDistributed)
			{
				FString PostUnderscore = BaseSrcFilename.Right((BaseSrcFilename.Len() - Underscore) - 1);
				// if there is an underscore, is what follows a language extension?
				for (INT LangIndex = 0; GKnownLanguageExtensions[LangIndex]; LangIndex++)
				{
					// is the extension a language extension?
					if (PostUnderscore == GKnownLanguageExtensions[LangIndex])
					{
						// if it's not the language 
						if (PostUnderscore != UObject::GetLanguage())
						{
							bIsWrongLanguage = TRUE;
						}

						// found a language extension, we can stop looking
						break;
					}
				}
			}

			// don't cook if it's the wrong language
			if (bIsWrongLanguage)
			{
				if( bShouldLogSkippedPackages )
				{
					warnf(NAME_Log, TEXT("Skipping wrong language file %s"), *SrcFilename);
				}
				continue;
			}


			// check to see if the existing cooked file is an older version, which will need recooking
			UBOOL bCookedVersionIsOutDated = FALSE;
			if( DstFileExists == TRUE )
			{
				INT CookedVersion = PersistentCookerData->GetFileCookedVersion(*DstFilename);
				
				// if it's cooked with a different version, recook!
				if (CookedVersion != GPackageFileCookedContentVersion)
				{
					bCookedVersionIsOutDated = TRUE;
				}
			}



			// Skip over unchanged files (unless we specify ignoring it for e.g. maps or script).
			if(		( DstFileNewer == TRUE ) 
				&&	( bCookedVersionIsOutDated == FALSE ) 
				&&	( !bIsMapFile || !bForceCookMaps )
				&&	( !bIsNativeScriptFile || !bForceCookNativeScript ) 
				&&	( !bIsStandaloneSeekfree || !bForceRecookSeekfree )
				&&	( !bIsCombinedStartupPackage ) )
			{
				// check if any of the dependent packages of a seekfree packages are out to date (otherwise we need to recook the seekfree package)
				UBOOL bIsSeekfreeFileDependencyNewer = FALSE;
				if (bIsSeekfree && DstFileExists)
				{
					FFilename ActualDstName = DstFilename;
					// we need to check the _SF version for standalone seekfree packages
					if (bIsStandaloneSeekfree)
					{
						ActualDstName = DstFilename.GetBaseFilename(FALSE) + STANDALONE_SEEKFREE_SUFFIX + FString(TEXT(".")) + DstFilename.GetExtension();
					}

					// check dependencies against the cooked package
					DOUBLE Time = GFileManager->GetFileTimestamp(*ActualDstName);
					bIsSeekfreeFileDependencyNewer = AreSeekfreeDependenciesNewer(NULL, SrcFilename, Time);
				}

				// if there weren't any seekfree dependencies newer, then we are up to date
				if (!bIsSeekfreeFileDependencyNewer)
				{
					if( bShouldLogSkippedPackages )
					{
						warnf(NAME_Log, TEXT("UpToDate %s"), *SrcFilename);
					}
					continue;
				}
			}
						
			// Skip over any cooked files residing in cooked folder.
			if( SrcFilename.InStr( GetCookedDirectory() ) != INDEX_NONE )
			{
				if( bShouldLogSkippedPackages )
				{
					warnf(NAME_Log, TEXT("Skipping %s"), *SrcFilename);
				}
				continue;
			}		

			// Determine which container to add the item to so it can be cooked correctly

			// Package is definitely a map file.
			if( bIsMapFile || bIsTokenMap )
			{
				const UBOOL bIsMultiplayerMap = bSeparateSharedMPResources && SrcFilename.GetBaseFilename( TRUE ).StartsWith(TEXT("MP_"));
				if ( bIsMultiplayerMap )
				{
					// Maintain a separate list of multiplayer maps.
					warnf( NAME_Log, TEXT("GeneratePackageList: Treating %s as a multiplayer map"), *SrcFilename );
					MPMapFilenamePairs.AddItem( FPackageCookerInfo( *SrcFilename, *DstFilename, TRUE, FALSE, FALSE, FALSE ) );
				}
				else
				{
					MapFilenamePairs.AddItem( FPackageCookerInfo( *SrcFilename, *DstFilename, TRUE, FALSE, FALSE, FALSE ) );
				}
			}
			// Package is a script file.
			else if( bIsNativeScriptFile )
			{
				// in the first pass, if a native script was marked as needing to be cooked, cook all
				// native script packages, otherwise some subtle import/forcedexport issues can arise
				if (Pass == 0)
				{
					bForceCookNativeScript = TRUE;
				}
				else
				{
					ScriptFilenamePairs.AddItem( FPackageCookerInfo( *SrcFilename, *DstFilename, TRUE, TRUE, FALSE, FALSE ) );
				}
			}
			// Package is a combined startup package
			else if ( bIsCombinedStartupPackage )
			{
				// first pass, just check if any packages are out of date
				if (Pass == 0)
				{
					FString StartupPackageName = FString::Printf(TEXT("%s%s\\Startup_%s.%s"), *appGameDir(), *GetCookedDirectory(), UObject::GetLanguage(), (Platform == PLATFORM_Windows ? TEXT("upk") : TEXT("xxx")));
					DOUBLE	StartupFileTime				= GFileManager->GetFileTimestamp(*StartupPackageName);
					UBOOL	StartupFileExists			= GFileManager->FileSize(*StartupPackageName) > 0;
					UBOOL	StartupFileNewer			= (StartupFileTime >= 0) && (StartupFileTime >= SrcFileTime);

					// is the startup file out of date?
					if (!StartupFileExists || !StartupFileNewer)
					{
						bForceCookStartupPackage = TRUE;
					}

					// also add it to the 'not required' list (if it needs cooking) so that the texture mips are saved out (with no other cooking going on)
					if (!DstFileNewer || bCookedVersionIsOutDated)
					{
						NotRequiredFilenamePairs.AddItem( FPackageCookerInfo( *SrcFilename, *DstFilename, FALSE, FALSE, FALSE, FALSE ) );
					}
				}
				// on second pass, cook all startup files into the combined startup package if needed (as determined on first pass)
				else
				{
					// always add to the startup list, we'll decide below if we need it or not
					StartupFilenamePairs.AddItem( FPackageCookerInfo( *SrcFilename, *DstFilename, TRUE, TRUE, TRUE, FALSE ) );
				}
			}
			else if ( (!bIsInUserMode && bIsStandaloneSeekfree )
				// In user mode, also considered to be standalone-seekfree are
				//   1) mod script (non-native by definition); and
				//   2) non-map content packages.
				|| (bIsInUserMode && (bIsTokenScriptPackage || bIsTokenContentPackage)) )
			{
				StandaloneSeekfreeFilenamePairs.AddItem( FPackageCookerInfo( *SrcFilename, *DstFilename, TRUE, FALSE, FALSE, TRUE ) );

				// also add it to the 'not required' list (if it needs cooking) so that the texture mips are saved out (with no other cooking going on)
				if (!DstFileNewer || bCookedVersionIsOutDated)
				{
					NotRequiredFilenamePairs.AddItem( FPackageCookerInfo( *SrcFilename, *DstFilename, FALSE, FALSE, FALSE, FALSE ) );
				}
			}
			// Package is not required and can be stripped of everything but texture miplevels.
			else
			{
				NotRequiredFilenamePairs.AddItem( FPackageCookerInfo( *SrcFilename, *DstFilename, FALSE, FALSE, FALSE, FALSE ) );
			}
		}

		// if any of the non-seekfree packages needed cooking, then force cook the startup
		if (NotRequiredFilenamePairs.Num())
		{
//			bForceCookNativeScript = TRUE;
			bForceCookStartupPackage = TRUE;

			// set when the cooker last marked non seekfree packages for cooking, so next runs
			// will still recook maps that weren't cooked
			PersistentCookerData->SetLastNonSeekfreeCookTime();
		}
	}

	// 'Game' cooker helper call to GeneratePackageList
	if (GGameCookerHelper)
	{
		if (GGameCookerHelper->GeneratePackageList(this, Platform, ShaderPlatform, 
			NotRequiredFilenamePairs, RegularFilenamePairs, MapFilenamePairs, 
			MPMapFilenamePairs, ScriptFilenamePairs, StartupFilenamePairs, StandaloneSeekfreeFilenamePairs) == FALSE)
		{
			// Failed for some reason... handle it.

		}
	}

	// Sort regular files, script and finally maps to be last.
	TArray<FPackageCookerInfo> SortedFilenamePairs;
	
	// Always cooked regular (non script, non map) packages first.
	SortedFilenamePairs += NotRequiredFilenamePairs;

	// Followed by script packages
	FirstScriptIndex		= SortedFilenamePairs.Num();
	if( ScriptFilenamePairs.Num() )
	{
		FirstGameScriptIndex	= FirstScriptIndex + EngineScriptPackageNames.Num();
		// Add in order of appearance in EditPackages and not layout on file determined by filename.
		TArray<FString> ScriptPackageNames;
		ScriptPackageNames += EngineScriptPackageNames;
		ScriptPackageNames += GameNativeScriptPackageNames;
		for( INT NameIndex=0; NameIndex<ScriptPackageNames.Num(); NameIndex++ )
		{
			for( INT PairIndex=0; PairIndex<ScriptFilenamePairs.Num(); PairIndex++ )
			{
				if( ScriptPackageNames(NameIndex) == ScriptFilenamePairs(PairIndex).SrcFilename.GetBaseFilename() )
				{
					SortedFilenamePairs.AddItem( ScriptFilenamePairs(PairIndex) );
					break;
				}
			}
		}
	}

	// Then cook startup packages
	FirstStartupIndex = SortedFilenamePairs.Num();

	if (!bIsDistributed)
	{
		// check if anything will be getting cooked after the startup packages
		UBOOL bArePostStartupPackagesCooked = StandaloneSeekfreeFilenamePairs.Num() != 0 ||
			MapFilenamePairs.Num() != 0 || MPMapFilenamePairs.Num() != 0;

		// we only need to process the startup packages if there are packages that will be cooked after this or we 
		// the package needs cooking. if it doens't need cooking, we still need to load the packages if stuff
		// after it will be cooked (for proper MarkedByCooker flags)
		if (bForceCookStartupPackage || bArePostStartupPackagesCooked)
		{
			// if we didn't need to actually cook the startup package, mark the packages to be loaded and not cooked
			if (!bForceCookStartupPackage)
			{
				for (INT PackageIndex = 0; PackageIndex < StartupFilenamePairs.Num(); PackageIndex++)
				{
					StartupFilenamePairs(PackageIndex).bShouldOnlyLoad = TRUE;
				}
			}

			// add the startup packages to the list of packages
			SortedFilenamePairs += StartupFilenamePairs;

		}
		SortedFilenamePairs += StandaloneSeekfreeFilenamePairs;
	}

	// Now append maps.
	if( MapFilenamePairs.Num() )
	{
		FirstMapIndex = SortedFilenamePairs.Num();
		SortedFilenamePairs += MapFilenamePairs;
	}

	// Maintain a separate list of multiplayer maps.
	if( MPMapFilenamePairs.Num() )
	{
		FirstMPMapIndex = SortedFilenamePairs.Num();
		SortedFilenamePairs += MPMapFilenamePairs;
	}

	return SortedFilenamePairs;
}

/**
 * Cleans up DLL handles and destroys cookers
 */
void UCookPackagesCommandlet::Cleanup()
{
	// Reset the global shader precompiler pointer to NULL.
	if( GConsoleShaderPrecompilers[ShaderPlatform] )
	{
		check(GConsoleShaderPrecompilers[ShaderPlatform] == ConsoleSupport->GetGlobalShaderPrecompiler());
		GConsoleShaderPrecompilers[ShaderPlatform] = NULL;
	}
}

/**
 * Handles duplicating cubemap faces that are about to be saved with the passed in package.
 *
 * @param	Package	 Package for which cubemaps that are going to be saved with it need to be handled.
 */
void UCookPackagesCommandlet::HandleCubemaps( UPackage* Package )
{
#if 0 //@todo cooking: this breaks regular textures being used for cubemap faces
	// Create dummy, non serialized package old cubemap faces get renamed into.
	UPackage* OldCubemapFacesRenamedByCooker = CreatePackage( NULL, TEXT("OldCubemapFacesRenamedByCooker") );
#endif
	// Duplicate textures used by cubemaps.
	for( TObjectIterator<UTextureCube> It; It; ++It )
	{
		UTextureCube* Cubemap = *It;
		// Valid cubemap (square textures of same size and format assigned to all faces)
		if (Cubemap->bIsCubemapValid)
		{
			// Only duplicate cubemap faces saved into this package.
			if( Cubemap->IsIn( Package ) || Cubemap->HasAnyFlags( RF_ForceTagExp ) )
			{
				for( INT FaceIndex=0; FaceIndex<6; FaceIndex++ )
				{
					UTexture2D* OldFaceTexture	= Cubemap->GetFace(FaceIndex);
					// Only duplicate once.
					if( OldFaceTexture && OldFaceTexture->GetOuter() != Cubemap )
					{
						// Duplicate cubemap faces so every single texture only gets loaded once.
						Cubemap->SetFace(
							FaceIndex,
							CastChecked<UTexture2D>( UObject::StaticDuplicateObject( 
							OldFaceTexture, 
							OldFaceTexture, 
							Cubemap, 
							*FString::Printf(TEXT("CubemapFace%i"),FaceIndex)
									) )
								);
#if 0 //@todo cooking: this breaks regular textures being used for cubemap faces
						// Rename old cubemap faces into non- serialized package and make sure they are never put into
						// a seekfree package by clearing flags and marking them.
						OldFaceTexture->Rename( NULL, OldCubemapFacesRenamedByCooker, REN_ForceNoResetLoaders );
						OldFaceTexture->ClearFlags( RF_ForceTagExp );
						OldFaceTexture->SetFlags( RF_MarkedByCooker );
#endif
					}
				}
			}
		}
		// Cubemap is invalid!
		else
		{
			// No need to reference texture from invalid cubemaps.
			for( INT FaceIndex=0; FaceIndex<6; FaceIndex++ )
			{
				Cubemap->SetFace(FaceIndex,NULL);
			}
		}
	}
}

/**
 * Collects garbage and verifies all maps have been garbage collected.
 */
void UCookPackagesCommandlet::CollectGarbageAndVerify()
{
	SCOPE_SECONDS_COUNTER(CollectGarbageAndVerifyTime);

	// Clear all components as quick sanity check to ensure GC can remove everything.
	GWorld->ClearComponents();

	// Collect garbage up-front to ensure that only required objects will be put into the seekfree package.
	UObject::CollectGarbage(RF_Native);

	// At this point the only world still around should be GWorld, which is the one created in CreateNew.
	for( TObjectIterator<UWorld> It; It; ++It )
	{
		UWorld* World = *It;
		if( World != GWorld )
		{
			UObject::StaticExec(*FString::Printf(TEXT("OBJ REFS CLASS=WORLD NAME=%s.TheWorld"), *World->GetOutermost()->GetName()));

			TMap<UObject*,UProperty*>	Route		= FArchiveTraceRoute::FindShortestRootPath( World, TRUE, GARBAGE_COLLECTION_KEEPFLAGS );
			FString						ErrorString	= FArchiveTraceRoute::PrintRootPath( Route, World );
			debugf(TEXT("%s"),*ErrorString);		
		
			// We cannot safely recover from this.
			appErrorf( TEXT("%s not cleaned up by garbage collection!") LINE_TERMINATOR TEXT("%s"), *World->GetFullName(), *ErrorString );
		}
	}
}

/**
* Adds the mip data payload for the given texture and mip index to the texture file cache.
* If an entry exists it will try to replace it if the mip is <= the existing entry or
* the mip data will be appended to the end of the TFC file.
* Also updates the bulk data entry for the texture mip with the saved size/offset.
*
* @param Package - Package for texture that is going to be saved
* @param Texture - 2D texture with mips to be saved
* @param MipIndex - index of mip entry in the texture that needs to be saved
*/
void UCookPackagesCommandlet::SaveMipToTextureFileCache( UPackage* Package, UTexture2D* Texture, INT MipIndex )
{
	check(bUseTextureFileCache);	
	
	// Allocate temporary scratch buffer for alignment and zero it out.
	static void* Zeroes = NULL;
	if( !Zeroes )
	{	
		Zeroes = appMalloc( TextureFileCacheBulkDataAlignment );
		appMemzero( Zeroes, TextureFileCacheBulkDataAlignment );
	}

	// Access the texture cache archive for this texture.
	FArchive* TextureCacheArchive = GetTextureCacheArchive(Texture->TextureFileCacheName);

	check(Texture);
	check(Texture->Mips.IsValidIndex(MipIndex));

	// miplevel to save into texture cache file.
	FTexture2DMipMap& Mip = Texture->Mips(MipIndex);

	// Set flag to ensure that we're only saving the raw data to the archive. Required for size comparison.
	Mip.Data.SetBulkDataFlags( BULKDATA_StoreOnlyPayload );	
	// Allow actual payload to get serialized instead of just status info
	Mip.Data.ClearBulkDataFlags( BULKDATA_StoreInSeparateFile );
	// Enable compression on streaming mips
	Mip.Data.StoreCompressedOnDisk( GBaseCompressionMethod );

	// Create persistent memory writer that has byte swapping propagated and is using scratch memory.
	TArray<BYTE> ScratchMemory;
	FMemoryWriter MemoryWriter( ScratchMemory, TRUE );
	MemoryWriter.SetByteSwapping( ShouldByteSwapData() );

	// Serialize to memory writer, taking compression settings into account.
	Mip.Data.Serialize( MemoryWriter, Texture );
	check( ScratchMemory.Num() );

	// Retrieve bulk data info if its already cached.
	FString					BulkDataName = *FString::Printf(TEXT("MipLevel_%i"),MipIndex);
	FCookedBulkDataInfo*	BulkDataInfo = PersistentCookerData->GetBulkDataInfo( Texture, *BulkDataName );

	// Texture mip is already part of file, see whether we can replace existing data.
	if ( BulkDataInfo )
	{
		INT OldAlignedBulkDataSize = Align(BulkDataInfo->SavedBulkDataSizeOnDisk, TextureFileCacheBulkDataAlignment);

		if (BulkDataInfo->TextureFileCacheName != Texture->TextureFileCacheName)
		{
			BulkDataInfo = NULL;
			// Make sure we are at the end of the file...
			INT CacheFileSize = TextureCacheArchive->TotalSize();
			TextureCacheArchive->Seek( CacheFileSize );
			PersistentCookerData->SetTextureFileCacheWaste( PersistentCookerData->GetTextureFileCacheWaste() + OldAlignedBulkDataSize );
		}
		else
		{
			// Only look at aligned sizes as that's what we write out to file.
			INT NewAlignedBulkDataSize = Align(ScratchMemory.Num(),TextureFileCacheBulkDataAlignment);

			// We can replace existing data if the aligned data is less than or equal to existing data.
			if( OldAlignedBulkDataSize >= NewAlignedBulkDataSize )
			{
				// Seek to position in file.
				TextureCacheArchive->Seek( BulkDataInfo->SavedBulkDataOffsetInFile );
				// If smaller, track waste...
				if (NewAlignedBulkDataSize < OldAlignedBulkDataSize)
				{
					PersistentCookerData->SetTextureFileCacheWaste( PersistentCookerData->GetTextureFileCacheWaste() + (OldAlignedBulkDataSize - NewAlignedBulkDataSize) );
				}
			}
			// Can't replace so we need to append.
			else
			{
				// Make sure we are at the end of the file...
				INT CacheFileSize = TextureCacheArchive->TotalSize();
				TextureCacheArchive->Seek( CacheFileSize );
				// Keep track of wasted memory in file.
				PersistentCookerData->SetTextureFileCacheWaste( PersistentCookerData->GetTextureFileCacheWaste() + OldAlignedBulkDataSize );
			}
		}
	}

	check( (TextureCacheArchive->Tell() % TextureFileCacheBulkDataAlignment == 0) );

	// Tell mip data where we are going to serialize data to, simulating serializing it again.
	Mip.Data.StoreInSeparateFile( 
		TRUE, 
		Mip.Data.GetSavedBulkDataFlags() & ~BULKDATA_StoreOnlyPayload,
		Mip.Data.GetElementCount(),
		TextureCacheArchive->Tell(),
		ScratchMemory.Num() );		
	// Serialize scratch data to disk at the proper offset.
	TextureCacheArchive->Serialize( ScratchMemory.GetData(), ScratchMemory.Num() );

	// Figure out position in file and ECC align it by zero padding.
	INT EndPos				= TextureCacheArchive->Tell();
	INT NumRequiredZeroes	= Align( EndPos, TextureFileCacheBulkDataAlignment ) - EndPos;

	// Serialize padding to file.
	if( NumRequiredZeroes )
	{
		TextureCacheArchive->Serialize( Zeroes, NumRequiredZeroes );
	}	
	check( TextureCacheArchive->Tell() % TextureFileCacheBulkDataAlignment == 0 );

	// Seek back to the end of the file.
	INT CurrentFileSize = TextureCacheArchive->TotalSize();
	check( CurrentFileSize >= 0 );
	TextureCacheArchive->Seek( CurrentFileSize );

	// Remove temporary serialization flag again.
	Mip.Data.ClearBulkDataFlags( BULKDATA_StoreOnlyPayload );

	// Update the bulk data info for the saved mip entry
	if( !BulkDataInfo )
	{
		FCookedBulkDataInfo Info;
		PersistentCookerData->SetBulkDataInfo( Texture, *FString::Printf(TEXT("MipLevel_%i"),MipIndex), Info );
		BulkDataInfo = PersistentCookerData->GetBulkDataInfo(Texture, *FString::Printf(TEXT("MipLevel_%i"),MipIndex));
	}

	BulkDataInfo->SavedBulkDataFlags		= Mip.Data.GetSavedBulkDataFlags();
	BulkDataInfo->SavedElementCount			= Mip.Data.GetSavedElementCount();
	BulkDataInfo->SavedBulkDataOffsetInFile	= Mip.Data.GetSavedBulkDataOffsetInFile();
	BulkDataInfo->SavedBulkDataSizeOnDisk	= Mip.Data.GetSavedBulkDataSizeOnDisk();
	BulkDataInfo->TextureFileCacheName		= Texture->TextureFileCacheName;

	if (bVerifyTextureFileCache)
	{
		FTextureFileCacheEntry TempEntry;

		TempEntry.TextureName = Texture->GetPathName();
		TempEntry.MipIndex = MipIndex;
		TempEntry.OffsetInCache = BulkDataInfo->SavedBulkDataOffsetInFile;
		TempEntry.SizeInCache = BulkDataInfo->SavedBulkDataSizeOnDisk;
		TempEntry.FlagsInCache = BulkDataInfo->SavedBulkDataFlags;
		TempEntry.ElementCountInCache = BulkDataInfo->SavedElementCount;
		if (Texture->TextureFileCacheName == TEXT("CharTextures"))
		{
			TempEntry.bCharTextureFileCache = TRUE;
		}
		else
		{
			TempEntry.bCharTextureFileCache = FALSE;
		}

		if (Texture->TextureFileCacheName != NAME_None)
		{
			FTextureFileCacheEntry* OverlapEntry = FindTFCEntryOverlap(TempEntry);
			if (OverlapEntry)
			{
				warnf(NAME_Warning, TEXT("SaveMipToTextureFileCache> Texture overlap found in TFC!"));
				warnf(NAME_Warning, TEXT("\tMip %d of %s"), TempEntry.MipIndex, *(TempEntry.TextureName));
				warnf(NAME_Warning, TEXT("\t\tOffset = %10d, Size = %d"), TempEntry.OffsetInCache, TempEntry.SizeInCache);
				warnf(NAME_Warning, TEXT("\tMip %d of %s"), OverlapEntry->MipIndex, *(OverlapEntry->TextureName));
				warnf(NAME_Warning, TEXT("\t\tOffset = %10d, Size = %d"), OverlapEntry->OffsetInCache, OverlapEntry->SizeInCache);
			}
		}
	}
}

/**
 * Saves the passed in package, gathers and stores bulk data info and keeps track of time spent.
 *
 * @param	Package						Package to save
 * @param	Base						Base/ root object passed to SavePackage, can be NULL
 * @param	TopLevelFlags				Top level "keep"/ save flags, passed onto SavePackage
 * @param	DstFilename					Filename to save under
 * @param	bStripEverythingButTextures	Whether to strip everything but textures
 * @param	bRememberSavedObjects		TRUE if objects should be marked to not be saved again, as well as materials (if bRemeberSavedObjects is set)
 */
void UCookPackagesCommandlet::SaveCookedPackage( UPackage* Package, UObject* Base, EObjectFlags TopLevelFlags, const TCHAR* DstFilename, UBOOL bStripEverythingButTextures, UBOOL bCleanupAfterSave, UBOOL bRememberSavedObjects )
{
	SCOPE_SECONDS_COUNTER(PackageSaveTime);

	// Mark the chain of any object set for ForceTagExp as well...
	for (FObjectIterator TagObjIt; TagObjIt; ++TagObjIt)
	{
		UObject* TagObject = *TagObjIt;
		if (TagObject->HasAnyFlags(RF_ForceTagExp))
		{
			UObject* TagOuter = TagObject->GetOuter();
			while (TagOuter)
			{
				TagOuter->SetFlags(RF_ForceTagExp);
				TagOuter = TagOuter->GetOuter();
			}
		}
	}

	// for PC cooking, we leave all objects in their original packages, and in cooked packages
	if (Platform == PLATFORM_Windows)
	{
		bStripEverythingButTextures = FALSE;

		// make sure destination exists
		GFileManager->MakeDirectory(*FFilename(DstFilename).GetPath(), TRUE);

		for( TObjectIterator<UObject> It; It; ++It )
		{
			// never allow any shader objects to get cooked into another package
			if( It->IsA(UShaderCache::StaticClass()) )
			{
				It->ClearFlags( RF_ForceTagExp );
			}
		}
	}

	// Remove RF_Standalone from everything but UTexture2D objects if we're stripping out data.
	TMap<UObject*,UBOOL> ObjectsToRestoreStandaloneFlag;
	TMap<UObject*,UBOOL> ObjectsToRemoveStandaloneFlag;
	UBOOL bPackageContainsTextures = FALSE;
	if( bStripEverythingButTextures )
	{
		check(Base==NULL);
		// Iterate over objects, remove RF_Standalone fron non- UTexture2D objects and keep track of them for later.
		for( TObjectIterator<UObject> It; It; ++It )
		{
			UObject* Object = *It;
			// Non UTexture2D object that is marked RF_Standalone.
			if( Object->IsA(UTexture2D::StaticClass())
				// if cooking for a console, only keep textures that can actually be loaded on the console
				// if cooking for PC, keep them since we support loading cooked packages in the editor
				&&( ((GCookingTarget&UE3::PLATFORM_Console) == 0) || !Object->HasAllFlags(RF_NotForServer|RF_NotForClient)) )
			{
				if( Object->IsIn( Package ) )
				{
					bPackageContainsTextures = TRUE;

					// fonts aren't standalone (and in the future maybe other's won't be as well), so
					// temporarily mark them as standalone so they are saved out in the streaming texture package
					// yes, fonts shouldn't be streaming, but just in case, we don't want to cause crashes!
					if (Object->HasAllFlags(RF_Standalone) == FALSE)
					{
						// Keep track of object and set flag.
						ObjectsToRemoveStandaloneFlag.Set( Object, TRUE );
						Object->SetFlags( RF_Standalone );
					}
				}
			}
			else if( Object->HasAllFlags( RF_Standalone ) )
			{
				// Keep track of object and remove flag.
				ObjectsToRestoreStandaloneFlag.Set( Object, TRUE );
				Object->ClearFlags( RF_Standalone );
			}
		}
	}

	if( !bStripEverythingButTextures || bPackageContainsTextures)
	{
		// Save package. World can be NULL for regular packages. @warning: DO NOT conform here, as that will cause 
		// the number of generations in the summary of package files to mismatch with the number of generations in 
		// forced export copies of that package! SavePackage has code for the cooking case to match up GUIDs with 
		// the original package

		//@script patcher
		ULinkerLoad* ConformBase = NULL;
		if( !ParseParam(appCmdLine(),TEXT("NOCONFORM")) )
		{
			FFilename BasePackageName = DstFilename;
			FString ConformLocationPath;
			UBOOL bUserSpecifiedConformDirectory = Parse(appCmdLine(),TEXT("CONFORMDIR="),ConformLocationPath);
			if ( !bUserSpecifiedConformDirectory )
			{
				//@script patcher fixme - make the default conform directory an .ini setting or something?
				ConformLocationPath = appGameDir() * TEXT("Build\\ConformSourceBuild\\CookedXenon");
			}

			if ( ConformLocationPath.Len() > 0 )
			{
				// verify that the directory exists
				TArray<FString> DummyArray;
				GFileManager->FindFiles(DummyArray, *ConformLocationPath, FALSE, TRUE);
				if ( DummyArray.Num() == 0 )
				{
					// attempt to normalize the specified directory name
					ConformLocationPath = appConvertRelativePathToFull(*ConformLocationPath);
					GFileManager->FindFiles(DummyArray, *ConformLocationPath, FALSE, TRUE);
					if ( DummyArray.Num() == 0 && bUserSpecifiedConformDirectory )
					{
						if ( appMsgf(AMT_YesNo, TEXT("Conform path specified doesn't exist (%s).  Choose 'Yes' to stop cooking or 'No' to continue without cooking without conforming")) )
						{
							bAbortCooking = TRUE;
						}
					}
				}
			}
			
			if ( !bAbortCooking )
			{
				FFilename ConformBasePathName = ConformLocationPath * BasePackageName.GetCleanFilename();
				if ( GFileManager->FileSize(*ConformBasePathName) > 0 )
				{
					// check the default location for script packages to conform against, if a like-named package exists in the
					// auto-conform directory, use that as the conform package
					UObject::BeginLoad();

					UPackage* ConformBasePackage = UObject::CreatePackage(NULL, *(BasePackageName.GetBaseFilename() + TEXT("_OLD")));

					// loading this cooked package will cause the package file lookup to thereafter refer to the cooked file, so before loading the file, grab the current path
					// associated with this package name so we can restore it later
					FString PreviousPackagePath;
					GPackageFileCache->FindPackageFile(*BasePackageName.GetBaseFilename(), NULL, PreviousPackagePath);

					// now load the cooked version of the original package
					ConformBase = UObject::GetPackageLinker( ConformBasePackage, *ConformBasePathName, LOAD_Quiet|LOAD_NoWarn|LOAD_NoVerify, NULL, NULL );

					// hmmmm, the question is - do we restore the previous path now or after we call EndLoad()?
					if ( PreviousPackagePath.Len() )
					{
						GPackageFileCache->CachePackage(*PreviousPackagePath, TRUE, FALSE);
					}

					UObject::EndLoad();
					warnf( TEXT("Conforming: %s"), DstFilename );

					// If the passed in Base is an ObjectReferencer and it was created using a transient name, we'll need to rename it so that
					// the package can be conformed correctly
					if ( Base != NULL && Base->IsA(UObjectReferencer::StaticClass()) && Base->GetName().StartsWith(TEXT("ObjectReferencer_")) )
					{
						// find the one in the package we're conforming against
						for ( INT ExportIndex = 0; ExportIndex < ConformBase->ExportMap.Num(); ExportIndex++ )
						{
							FObjectExport& OriginalExport = ConformBase->ExportMap(ExportIndex);
							if ( OriginalExport._Object && OriginalExport._Object->GetClass() == UObjectReferencer::StaticClass() )
							{
								if ( OriginalExport._Object->GetName().StartsWith(TEXT("ObjectReferencer_")) &&
									Base->GetName() != OriginalExport._Object->GetName() )
								{
									// found it - now we rename our own ObjectReferencer to have the same name as the original one
									// no need to worry about a redirect being created since ObjectReferencers aren't marked RF_Public

									// first, sanity check that there isn't already another ObjectReferencer using this name
									UObjectReferencer* ExistingReferencer = FindObject<UObjectReferencer>(Base->GetOuter(),
										*OriginalExport._Object->GetName(), TRUE);
									check(ExistingReferencer==NULL);

									Base->Rename(*OriginalExport._Object->GetName(), NULL, REN_ForceNoResetLoaders);
								}
							}
						}
					}
				}
			}
		}

		if ( !bAbortCooking )
		{
			// cache the cooked version for this file
			PersistentCookerData->SetFileCookedVersion(DstFilename, GPackageFileCookedContentVersion);

			// Remove package compression flags if package compression is not wanted.
			if( bDisallowPackageCompression )
			{
				Package->PackageFlags &= ~(PKG_StoreCompressed | PKG_StoreFullyCompressed);
			}

			UObject::SavePackage( Package, Base, TopLevelFlags, DstFilename, GWarn, ConformBase, ShouldByteSwapData() );
		}
	}

	//@script patcher
	if ( !bAbortCooking )
	{
		// Warn if waste exceeds 100 MByte.
		INT TextureFileCacheWasteMByte = PersistentCookerData->GetTextureFileCacheWaste() / 1024 / 1024;
		if( TextureFileCacheWasteMByte > 100 )
		{
			warnf( NAME_Warning, TEXT("Texture file cache waste exceeds %i MByte. Please consider a full recook!"), TextureFileCacheWasteMByte );
		}

		// Flush texture file cache archives to disk.
		for(TMap<FName,FArchive*>::TIterator TextureCacheIt(TextureCacheNameToArMap);TextureCacheIt;++TextureCacheIt)
		{
			TextureCacheIt.Value()->Flush();
		}
		// update TFC entries with new file timestamps
		PersistentCookerData->UpdateCookedTextureFileCacheEntryInfos( TextureCacheNameToFilenameMap );

		// Gather bulk data information from just saved packages and save persistent cooker data to disk.
		PersistentCookerData->GatherCookedBulkDataInfos( Package );
		PersistentCookerData->SaveToDisk();
	}

	// Restore RF_Standalone flag on objects.
	if( bStripEverythingButTextures )
	{
		for( TMap<UObject*,UBOOL>::TIterator It(ObjectsToRestoreStandaloneFlag); It; ++It )
		{
			UObject* Object = It.Key();
			Object->SetFlags( RF_Standalone );
		}
		for( TMap<UObject*,UBOOL>::TIterator It(ObjectsToRemoveStandaloneFlag); It; ++It )
		{
			UObject* Object = It.Key();
			Object->ClearFlags( RF_Standalone );
		}
	}

	// Clean up objects for subsequent runs.
	if( bCleanupAfterSave )
	{
		// Remove RF_ForceTagExp again for subsequent calls to SavePackage and set RF_MarkedByCooker for objects
		// saved into script. Also ensures that always loaded materials are only put into a single seekfree shader 
		// cache.
		for( FObjectIterator It; It; ++It )
		{
			UObject*			Object				= *It;
			UMaterial*			Material			= Cast<UMaterial>(Object);
			UMaterialInstance*	MaterialInstance	= Cast<UMaterialInstance>(Object);

			if (bRememberSavedObjects)
			{
				// Avoid object from being put into subsequent script or map packages.
				if( Object->HasAnyFlags( RF_Saved ) )
				{
					Object->SetFlags( RF_MarkedByCooker );
				}

				// Don't put material into subsequent seekfree shader caches.
				if( ShaderCache && Material )
				{
					AlreadyHandledMaterials.Add(Material->GetFullName());
				}
				if( ShaderCache && MaterialInstance )
				{
					AlreadyHandledMaterialInstances.Add(MaterialInstance->GetFullName());
				}
			}

			Object->ClearFlags( RF_ForceTagExp | RF_Saved );
		}
	}
}

/**
 * Returns whether there are any localized resources that need to be handled.
 *
 * @param Package			Current package that is going to be saved
 * @param TopLevelFlags		TopLevelFlags that are going to be passed to SavePackage
 * 
 * @return TRUE if there are any localized resources pending save, FALSE otherwise
 */
UBOOL UCookPackagesCommandlet::AreThereLocalizedResourcesPendingSave( UPackage* Package, EObjectFlags TopLevelFlags )
{
	UBOOL bAreLocalizedResourcesPendingSave = FALSE;
	for( FObjectIterator It; It; ++It )
	{
		UObject* Object = *It;
		if( Object->IsLocalizedResource() )
		{
			if (Object->IsIn(Package) || Object->HasAnyFlags(TopLevelFlags | RF_ForceTagExp))
			{
				if (!Object->HasAnyFlags(RF_MarkedByCooker))
				{
					bAreLocalizedResourcesPendingSave = TRUE;
					break;
				}
				else
				{
					debugf(TEXT("AreThereLocalizedResourcesPendingSave> LocRes MarkedByCooker: %s for package %s"), 
						*(Object->GetPathName()), *(Package->GetName()));
				}
			}
		}
	}
	return bAreLocalizedResourcesPendingSave;
}

/**
 * @return		TRUE if there are localized resources using the current language that need to be handled.
 */
static UBOOL AreThereNonEnglishLocalizedResourcesPendingSave(UPackage* Package)
{
	// Shouldn't be called in English, as _INT doesn't appear as a package file prefix.
	check( appStricmp( TEXT("INT"), UObject::GetLanguage() ) != 0 );

	UBOOL bAreNonEnglishLocalizedResourcesPendingSave = FALSE;
	for( FObjectIterator It; It; ++It )
	{
		UObject* Object = *It;
		if( Object->IsLocalizedResource() )
		{
			if (Object->IsIn(Package) || Object->HasAnyFlags(RF_ForceTagExp))
			{
				if (!Object->HasAnyFlags(RF_MarkedByCooker))
				{
					UPackage* ObjPackage = Object->GetOutermost();
					const FString ObjPackageName( ObjPackage->GetName() );

					FString FoundPackageFileName;
					// Check if the found filename contains for a cached package of the same name.
					FFilename LocalizedPackageName = FPackageFileCache::PackageFromPath( *ObjPackageName );
					LocalizedPackageName = LocalizedPackageName.GetLocalizedFilename( UObject::GetLanguage() );

					if( GPackageFileCache->FindPackageFile( *LocalizedPackageName, NULL, FoundPackageFileName ) )
					{
						bAreNonEnglishLocalizedResourcesPendingSave = TRUE;
						break;
					}
					else
					{
						debugf(TEXT("AreThereNonEnglishLocalizedResourcesPendingSave> LocRes PackageNotFound: %s for package %s (%s), localizedname %s"), 
							*(Object->GetPathName()), *ObjPackageName, *FoundPackageFileName, *LocalizedPackageName);
					}
				}
				else
				{
					debugf(TEXT("AreThereNonEnglishLocalizedResourcesPendingSave> LocRes MarkedByCooker: %s for package %s"), 
						*(Object->GetPathName()), *(Package->GetName()));
				}
			}
		}
	}
	return bAreNonEnglishLocalizedResourcesPendingSave;
}

template < class T >
class TMPSoundObjectReferenceCollector : public FArchive
{
public:

	/**
	* Constructor
	*
	* @param	InObjectArray			Array to add object references to
	* @param	InOuters				value for LimitOuter
	* @param	bInRequireDirectOuter	value for bRequireDirectOuter
	* @param	bShouldIgnoreArchetype	whether to disable serialization of ObjectArchetype references
	* @param	bInSerializeRecursively	only applicable when LimitOuter != NULL && bRequireDirectOuter==TRUE;
	*									serializes each object encountered looking for subobjects of referenced
	*									objects that have LimitOuter for their Outer (i.e. nested subobjects/components)
	* @param	bShouldIgnoreTransient	TRUE to skip serialization of transient properties
	*/
	TMPSoundObjectReferenceCollector( TArray<T*>* InObjectArray, const TArray<FString>& InOuters, UBOOL bInRequireDirectOuter=TRUE, UBOOL bShouldIgnoreArchetype=FALSE, UBOOL bInSerializeRecursively=FALSE, UBOOL bShouldIgnoreTransient=FALSE )
		:	ObjectArray( InObjectArray )
		,	LimitOuters(InOuters)
		,	bRequireDirectOuter(bInRequireDirectOuter)
	{
		ArIsObjectReferenceCollector = TRUE;
		ArIsPersistent = bShouldIgnoreTransient;
		ArIgnoreArchetypeRef = bShouldIgnoreArchetype;
		bSerializeRecursively = bInSerializeRecursively && LimitOuters.Num() > 0;
	}
protected:

	/** Stored pointer to array of objects we add object references to */
	TArray<T*>*		ObjectArray;

	/** List of objects that have been recursively serialized */
	TArray<UObject*> SerializedObjects;

	/** only objects within these outers will be considered. */
	TArray<FString>	LimitOuters;

	/** determines whether nested objects contained within LimitOuter are considered */
	UBOOL			bRequireDirectOuter;

	/** determines whether we serialize objects that are encounterd by this archive */
	UBOOL			bSerializeRecursively;
};

/**
* Helper implementation of FArchive used to collect object references, avoiding duplicate entries.
*/
class FArchiveMPSoundObjectReferenceCollector : public TMPSoundObjectReferenceCollector<UObject>
{
public:
	/**
	* Constructor
	*
	* @param	InObjectArray			Array to add object references to
	* @param	InOuters				value for LimitOuter
	* @param	bInRequireDirectOuter	value for bRequireDirectOuter
	* @param	bShouldIgnoreArchetype	whether to disable serialization of ObjectArchetype references
	* @param	bInSerializeRecursively	only applicable when LimitOuter != NULL && bRequireDirectOuter==TRUE;
	*									serializes each object encountered looking for subobjects of referenced
	*									objects that have LimitOuter for their Outer (i.e. nested subobjects/components)
	* @param	bShouldIgnoreTransient	TRUE to skip serialization of transient properties
	*/
	FArchiveMPSoundObjectReferenceCollector( TArray<UObject*>* InObjectArray, const TArray<FString>& InOuters, UBOOL bInRequireDirectOuter=TRUE, UBOOL bShouldIgnoreArchetypes=FALSE, UBOOL bInSerializeRecursively=FALSE, UBOOL bShouldIgnoreTransient=FALSE )
		:	TMPSoundObjectReferenceCollector<UObject>( InObjectArray, InOuters, bInRequireDirectOuter, bShouldIgnoreArchetypes, bInSerializeRecursively, bShouldIgnoreTransient )
	{}

private:
	/** 
	* UObject serialize operator implementation
	*
	* @param Object	reference to Object reference
	* @return reference to instance of this class
	*/
	FArchive& operator<<( UObject*& Object )
	{
		// Avoid duplicate entries.
		if ( Object != NULL )
		{
			if ( LimitOuters.Num() == 0 || LimitOuters.ContainsItem(Object->GetOutermost()->GetName()) )
			{
				if ( !ObjectArray->ContainsItem(Object) )
				{
					ObjectArray->AddItem( Object );

					// check this object for any potential object references
					if ( bSerializeRecursively )
					{
						Object->Serialize(*this);
					}
				}
			}
		}
		return *this;
	}
};

/**
 * Generates a list of all objects in the object tree rooted at the specified sound cues.
 *
 * @param	MPCueNames				The names of the MP sound 'root set'.
 * @param	OutMPObjectNames		[out] The names of all objects referenced by the input set of objects, including those objects.
 */
static void GenerateMPObjectList(const TArray<FString>& MPCueNames, TArray<FString>& OutMPObjectNames)
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Load all MP-referenced cues.
	TArray<USoundCue*> MPCues;
	for ( INT CueIndex = 0 ; CueIndex < MPCueNames.Num() ; ++CueIndex )
	{
		const FString& CueName = MPCueNames(CueIndex);
		USoundCue* Cue = FindObject<USoundCue>( NULL, *CueName );
		if ( !Cue )
		{
			Cue = LoadObject<USoundCue>( NULL, *CueName, NULL, LOAD_None, NULL );
		}
		if ( Cue )
		{
			MPCues.AddItem( Cue );
		}
		else
		{
			warnf( NAME_Log, TEXT("MP Sound Cues: couldn't load %s"), *CueName );
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Load all MP-referenced sound cue packages.
	const TCHAR* MPSoundcuePackagesIniSection = TEXT("Cooker.MPSoundCuePackages");
	TMultiMap<FString,FString>* IniMPPackagesList = GConfig->GetSectionPrivate( MPSoundcuePackagesIniSection, FALSE, TRUE, GEditorIni );
	//TArray<UObject*> SoundPackages;
	TArray<FString> SoundPackages;
	if ( MPSoundcuePackagesIniSection && IniMPPackagesList )
	{
		for( TMultiMap<FString,FString>::TConstIterator It(*IniMPPackagesList) ; It ; ++It )
		{
			const FString& PackageName = It.Value();
			FString PackageFilename;
			UPackage* Result = NULL;
			if( GPackageFileCache->FindPackageFile( *PackageName, NULL, PackageFilename ) )
			{
				SoundPackages.AddUniqueItem( PackageName );
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Serialize all the cues into a reference collector.
	TArray<UObject*> References;
	FArchiveMPSoundObjectReferenceCollector ObjectReferenceCollector( &References, SoundPackages, FALSE, FALSE, TRUE, TRUE );
	for ( INT CueIndex = 0 ; CueIndex < MPCues.Num() ; ++ CueIndex )
	{
		USoundCue* Cue = MPCues(CueIndex);
		Cue->Serialize( ObjectReferenceCollector );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create a list of the names of all objects referenced by the MP cues.
	OutMPObjectNames.Empty();

	// Seed the list with the cues themselves.
	OutMPObjectNames = MPCueNames;

	// Add referenced objects.
	for ( INT Index = 0 ; Index < References.Num() ; ++Index )
	{
		const UObject* Object = References(Index);
		// Exclude transient objects.
		if( !Object->HasAnyFlags( RF_Transient ) &&	!Object->IsIn( UObject::GetTransientPackage() ) )
		{
			const FString ObjectPath = Object->GetPathName();
			debugfSuppressed( NAME_DevCooking, TEXT("MPObjects: %s (%s)"), *ObjectPath, *Object->GetClass()->GetName() );
			OutMPObjectNames.AddItem( ObjectPath );
		}
	}
}

/**
 * Cooks the specified cues into the shared MP sound package.
 *
 * @param	MPCueNames		The set of sound cue names to cook.
 */
void UCookPackagesCommandlet::CookMPSoundPackages(const TArray<FString>& MPCueNames)
{
	if ( bSeparateSharedMPResources && MPCueNames.Num() > 0 )
	{
		// Mark all objects (cues) that are already in memory (and thus already cooked),
		// so we don't cook into the MP sounds package.
		for( FObjectIterator It ; It ; ++It )
		{
			UObject*	Object		= *It;
			// Mark the object as unwanted.
			Object->SetFlags( RF_MarkedByCooker );
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// Find/Load all the requested cues.
		for ( INT CueIndex = 0 ; CueIndex < MPCueNames.Num() ; ++CueIndex )
		{
			const FString& CueName = MPCueNames(CueIndex);
			USoundCue* Cue = FindObject<USoundCue>( NULL, *CueName );
			if ( !Cue )
			{
				Cue = LoadObject<USoundCue>( NULL, *CueName, NULL, LOAD_None, NULL );
				if ( !Cue )
				{
					warnf( NAME_Log, TEXT("MP Sound Cues: couldn't load %s"), *CueName );
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// Iterate over all objects and mark them as RF_ForceTagExp where appropriate.
		for( FObjectIterator It ; It ; ++It )
		{
			UObject* Object = *It;
			// Exclude objects that are either marked "marked" or transient.
			if( !Object->HasAnyFlags( RF_MarkedByCooker|RF_Transient ) 
				// Exclude objects that reside in the transient package.
				&&	!Object->IsIn( UObject::GetTransientPackage() ) )
			{
				// Mark object and outer chain so they get forced into the export table.
				while( Object )
				{
					Object->SetFlags( RF_ForceTagExp );
					Object = Object->GetOuter();
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// Create a new package.

		FFilename DstFilename	= CookedDir + MPSounds_PackageName + (Platform == PLATFORM_Windows ? TEXT(".upk") : TEXT(".xxx"));
		UPackage* MPSoundsPackage = CreatePackage( NULL, MPSounds_PackageName );
		check( !(MPSoundsPackage->PackageFlags & PKG_Cooked) ); 
		MPSoundsPackage->PackageFlags |= PKG_Cooked;
		MPSoundsPackage->PackageFlags |= PKG_DisallowLazyLoading;

		// Treat the MP sounds package as seek free.
		MPSoundsPackage->PackageFlags |= PKG_RequireImportsAlreadyLoaded;
		MPSoundsPackage->PackageFlags |= PKG_StoreCompressed;
		MPSoundsPackage->PackageFlags |= PKG_ServerSideOnly;

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// Iterate over all objects and cook them if they are going to be saved with the package.
		for( FObjectIterator It ; It ; ++It )
		{
			UObject* Object = *It;
			if( Object->HasAnyFlags( RF_ForceTagExp ) )
			{
				// Cook object.
				CookObject( MPSoundsPackage, Object, TRUE );
				// Prepare it for serialization.
				PrepareForSaving( MPSoundsPackage, Object, TRUE, FALSE );
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// Cook localized objects.

		const FString MPsoundsObjectReferencerName( MPSounds_ReferencerName );

		// The MP sounds package is seekfree, and most likely localized.
		if ( AreThereLocalizedResourcesPendingSave( NULL, 0 ) )
		{
			const UBOOL bCookingEnglish = appStricmp( TEXT("INT"), UObject::GetLanguage() ) == 0;

			// The cooker should not save out localization packages when cooking for non-English if no non-English resources were found.
			if ( bCookingEnglish || AreThereNonEnglishLocalizedResourcesPendingSave( NULL ) )
			{
				// Figure out localized package name and file name.
				FFilename LocalizedPackageName		= FFilename(DstFilename.GetBaseFilename() + LOCALIZED_SEEKFREE_SUFFIX).GetLocalizedFilename();
				FFilename LocalizedPackageFilename	= DstFilename.GetPath() + PATH_SEPARATOR + LocalizedPackageName + (Platform == PLATFORM_Windows ? TEXT(".upk") : TEXT(".xxx"));

				// Create a localized package and disallow lazy loading as we're seek free.
				UPackage* LocalizedPackage = CreatePackage( NULL, *LocalizedPackageName );
				LocalizedPackage->PackageFlags |= PKG_Cooked;
				LocalizedPackage->PackageFlags |= PKG_DisallowLazyLoading;
				LocalizedPackage->PackageFlags |= PKG_RequireImportsAlreadyLoaded;
				LocalizedPackage->PackageFlags |= PKG_ServerSideOnly;
				LocalizedPackage->PackageFlags |= PKG_StoreCompressed;

				// Create object references within package.
				const FString LocalizedObjectReferencerName( MPsoundsObjectReferencerName + TEXT("_") + UObject::GetLanguage() );
				UObjectReferencer* ObjectReferencer = ConstructObject<UObjectReferencer>(
					UObjectReferencer::StaticClass(),
					LocalizedPackage,
					FName(*LocalizedObjectReferencerName),
					RF_Cooked );

				// Reference all localized audio with the appropriate flags by the helper object.
				for( FObjectIterator It; It; ++It )
				{
					UObject* Object = *It;
					if( Object->IsLocalizedResource() )
					{
						if( Object->HasAnyFlags(RF_ForceTagExp) )
						{
							ObjectReferencer->ReferencedObjects.AddItem( Object );
						}
					}
				}

				// Save localized assets referenced by object referencer.
				SaveCookedPackage( LocalizedPackage, ObjectReferencer, RF_Standalone, *LocalizedPackageFilename, FALSE);

				// Prevent objects from being saved into seekfree package.
				for( INT ResourceIndex=0; ResourceIndex<ObjectReferencer->ReferencedObjects.Num(); ResourceIndex++ )
				{
					UObject* Object = ObjectReferencer->ReferencedObjects(ResourceIndex);
					Object->ClearFlags( RF_ForceTagExp );
					Object->SetFlags( RF_MarkedByCooker );
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// Cook remaining (unlocalized) objects.

		// Create an object referencer for any remaining (unlocalized) objects.
		UObjectReferencer* ObjectReferencer = ConstructObject<UObjectReferencer>(
			UObjectReferencer::StaticClass(),
			MPSoundsPackage,
			FName(*MPsoundsObjectReferencerName),
			RF_Cooked );

		// Reference all localized audio with the appropriate flags by the helper object.
		for( FObjectIterator It; It; ++It )
		{
			UObject* Object = *It;
			if( Object->HasAnyFlags(RF_ForceTagExp) )
			{
				ObjectReferencer->ReferencedObjects.AddItem( Object );
			}
		}

		// Saved any remaining unlocalized objects.
		SaveCookedPackage( MPSoundsPackage, ObjectReferencer, RF_Standalone, *DstFilename, FALSE );

		for( FObjectIterator It ; It ; ++It )
		{
			UObject* Object = *It;
			Object->ClearFlags( RF_ForceTagExp );
			Object->SetFlags( RF_MarkedByCooker );
		}
	}
}


/**
 * We use the CreateCustomEngine call to set some flags which will allow SerializeTaggedProperties to have the correct settings
 * such that editoronly and notforconsole data can correctly NOT be loaded from startup packages (e.g. engine.u)
 *
 **/
void UCookPackagesCommandlet::CreateCustomEngine()
{
	// get the platform from the Params (MUST happen first thing)
	if ( !SetPlatform( appCmdLine() ) )
	{
		SET_WARN_COLOR(COLOR_RED);
		warnf(NAME_Error, TEXT("Platform not specified. You must use:\n   platform=[xenon|pc|ps3|linux|macosx]"));
		CLEAR_WARN_COLOR();
	}
    
	GIsCooking				= TRUE;
	GCookingTarget 			= Platform;

	// force cooking to use highest compat level
	appSetCompatibilityLevel(FCompatibilityLevelInfo(5,5,5), FALSE);
}


/**
 * Main entry point for commandlets.
 */
INT UCookPackagesCommandlet::Main( const FString& Params )
{
	TotalTime = appSeconds();
	STAT(PrePackageIterationTime -= appSeconds());

	// get the platform from the Params (MUST happen first thing)
	if ( !SetPlatform(Params) )
	{
		SET_WARN_COLOR(COLOR_RED);
		warnf(NAME_Error, TEXT("Platform not specified. You must use:\n   platform=[xenon|pc|ps3|linux|macosx]"));
		CLEAR_WARN_COLOR();

		return 1;
	}

	// Set compression method based on platform.
	if( Platform & UE3::PLATFORM_PS3 )
	{
		// Zlib uses SPU tasks on PS3.
		GBaseCompressionMethod = COMPRESS_ZLIB;
	}
	else if( Platform & UE3::PLATFORM_Xenon )
	{
		// LZX is best trade-off of perf/ compression size on Xbox 360
		GBaseCompressionMethod = COMPRESS_LZX;
	}
	// Other platforms default to what the PC uses by default.

	// Bind to cooking DLLs
 	if( !BindDLLs() )
	{
		return 1;
	}

	// Parse command line and perform any initialization that isn't per package.
	UBOOL bQuitEvenOnSuccess = FALSE;
	UBOOL bInitWasSuccessful = Init( *Params, bQuitEvenOnSuccess );
	if (!bInitWasSuccessful || bQuitEvenOnSuccess)
	{
		// make sure we delete any copied shader files
		CleanupShaderFiles();
		return bInitWasSuccessful ? 0 : 1;
	}

	// We don't cook combined startup packages in user build, so manually mark objects before we do any cooking.
	if ( bIsInUserMode )
	{
		for( FObjectIterator It; It; ++It )
		{
			It->SetFlags( RF_MarkedByCooker );
		}
	}

	// Make a list of script ref'd anim sets... (for efforts)
	const TCHAR* SafeFaceFXIniSection = TEXT("Cooker.SafeFaceFXAnimSets");
	TMultiMap<FString,FString>* IniSafeFaceFXList = GConfig->GetSectionPrivate(SafeFaceFXIniSection, FALSE, TRUE, GEditorIni);
	if (IniSafeFaceFXList)
	{
		for (TMultiMap<FString,FString>::TConstIterator It(*IniSafeFaceFXList) ; It ; ++It)
		{
			const FString& SafeName = It.Value();
			ScriptReferencedFaceFXAnimSets.Set(SafeName, 1);
		}
	}

	// if we only wanted to cook inis, then skip over package processing
	if (!bIniFilesOnly)
	{
		if (!bMergeJobs)
		{
			// Generate list of packages to cook.
			INT FirstStartupIndex		= INDEX_NONE;
			INT FirstScriptIndex		= INDEX_NONE;
			INT FirstMapIndex			= INDEX_NONE;
			INT FirstMPMapIndex			= INDEX_NONE;
			INT FirstGameScriptIndex	= INDEX_NONE;
			TArray<FPackageCookerInfo> PackageList = GeneratePackageList( FirstStartupIndex, FirstScriptIndex, FirstGameScriptIndex, FirstMapIndex, FirstMPMapIndex );

			TArray<FString> MPCueNames;
			TArray<FString> MPObjectNames;
			if ( bSeparateSharedMPResources )
			{
				////////////////////////////////////////////////////////////////////////////////////////////////////
				// Look to the .ini for the list of sound cues referenced by MP
				const TCHAR* MPSoundcueIniSection = TEXT("Cooker.MPSoundCues");
				TMultiMap<FString,FString>* IniMPCuesList = GConfig->GetSectionPrivate( MPSoundcueIniSection, FALSE, TRUE, GEditorIni );
				if ( IniMPCuesList )
				{
					for( TMultiMap<FString,FString>::TConstIterator It(*IniMPCuesList) ; It ; ++It )
					{
						const FString& CueName = It.Value();
						MPCueNames.AddUniqueItem( CueName );
					}
				}

				if ( MPCueNames.Num() == 0 )
				{
					warnf( NAME_Warning, TEXT("No MP sounds to cook -- was the %s section of %s empty?"), MPSoundcueIniSection, GEditorIni );
				}
				else
				{
					// Generate the full set of objects referenced by MP sound cues.
					GenerateMPObjectList( MPCueNames, MPObjectNames );
				}
			}

			// find out if the target platform wants to fully load packages from memory at startup (fully compressed)
			UBOOL bFullyCompressStartupPackages = FALSE;
			GConfig->GetBool(TEXT("Engine.StartupPackages"), TEXT("bFullyCompressStartupPackages"), bFullyCompressStartupPackages, PlatformEngineConfigFilename);

			// create the combined startup package
			FString StartupPackageName = FString(TEXT("Startup_")) + UObject::GetLanguage();

			UPackage* CombinedStartupPackage = UObject::CreatePackage(NULL, *StartupPackageName);
			// make sure this package isn't GC'd
			CombinedStartupPackage->AddToRoot();
			// give the package a guid
			CombinedStartupPackage->MakeNewGuid();

			// create an object to reference all the objects in the startup packages to be combined together
			UObjectReferencer* CombinedStartupReferencer = ConstructObject<UObjectReferencer>( UObjectReferencer::StaticClass(), CombinedStartupPackage, NAME_None, RF_Cooked );
			// make sure this referencer isn't GC'd
			CombinedStartupReferencer->AddToRoot();

			// read in the per-map packages to cook
			TMap<FName, TArray<FName> > PerMapCookPackages;
			GConfig->Parse1ToNSectionOfNames(TEXT("Engine.PackagesToForceCookPerMap"), TEXT("Map"), TEXT("Package"), PerMapCookPackages, PlatformEngineConfigFilename);

			const UBOOL bCookingPS3Mod = ( bIsInUserMode && Platform == PLATFORM_PS3 );

			// save out the startup shaders
			SaveLocalShaderCaches();

			STAT(PrePackageIterationTime += appSeconds());

			// Iterate over all packages.
			//@script patcher (bAbortCooking)
			for( INT PackageIndex=0; !bAbortCooking && PackageIndex<PackageList.Num(); PackageIndex++ )
			{
				SCOPE_SECONDS_COUNTER(PackageIterationTime);				

				// Skip maps not required for cooking process. This is skipping packages that are only going to contain
				// higher miplevels of textures.				
				if( bSkipNotRequiredPackages && (PackageIndex < FirstScriptIndex) )
				{
					continue;
				}

				const FFilename& SrcFilename			= PackageList(PackageIndex).SrcFilename;
				FFilename DstFilename					= PackageList(PackageIndex).DstFilename;
				UBOOL bShouldBeSeekFree					= PackageList(PackageIndex).bShouldBeSeekFree;
				UBOOL bIsNativeScriptFile				= PackageList(PackageIndex).bIsNativeScriptFile;
				UBOOL bIsCombinedStartupPackage			= PackageList(PackageIndex).bIsCombinedStartupPackage;
				UBOOL bIsStandaloneSeekFreePackage		= PackageList(PackageIndex).bIsStandaloneSeekFreePackage;
				UBOOL bShouldOnlyLoad					= PackageList(PackageIndex).bShouldOnlyLoad;
				UBOOL bShouldBeFullyCompressed			= bFullyCompressStartupPackages && (bIsNativeScriptFile || bIsCombinedStartupPackage);
				FString PackageName						= SrcFilename.GetBaseFilename( TRUE );
				const UBOOL bIsMultiplayerMap			= bSeparateSharedMPResources && PackageName.StartsWith(TEXT("MP_"));
				if ( bIsMultiplayerMap )
				{
					// Assert that MP maps are being cooked last.
					check( PackageIndex >= FirstMPMapIndex );
					warnf( NAME_Log, TEXT("Treating %s as a multiplayer map"), *PackageName );
				}

				// Skip non-required packages if cooking with TFC 
				// since only referenced textures need to be updated into the TFC file
				if( bUseTextureFileCache && (PackageIndex < FirstScriptIndex) )
				{
					continue;
				}

				STAT(PrepPackageTime -= appSeconds());

				// Collect garbage and verify it worked as expected.	
				CollectGarbageAndVerify();

				// Save the shader caches after each package has been verified.
				if (!bSaveShaderCacheAtEnd)
				{
					SCOPE_SECONDS_COUNTER(SaveShaderCacheTime);
					SaveLocalShaderCaches();
				}

				STAT(PrepPackageTime += appSeconds());
				STAT(InitializePackageTime -= appSeconds());

				// When cooking mods for console, we want to enforce constraints on what mods can and cannot include.
				// Classify the specified package to determine if it's part of the mod 'set' by having been
				// specified on the cooker command line.
				UBOOL bIsTokenMap = FALSE;
				UBOOL bIsTokenScriptPackage = FALSE;
				UBOOL bIsTokenContentPackage = FALSE;
				UBOOL bIsTokenPackage = FALSE;
				if ( bCookingPS3Mod )
				{
					bIsTokenMap = TokenMaps.ContainsItem(SrcFilename);
					bIsTokenScriptPackage = TokenScriptPackages.ContainsItem(SrcFilename);
					bIsTokenContentPackage = TokenContentPackages.ContainsItem(SrcFilename);
					bIsTokenPackage = bIsTokenMap | bIsTokenScriptPackage | bIsTokenContentPackage;
				}

				// for PC we don't strip out non-texture objects
				const UBOOL bStripEverythingButTextures	= (PackageIndex < FirstScriptIndex) && (Platform != PLATFORM_Windows);
		
				// MP maps are cooked last.  Before cooking any MP maps, though,
				// first cook shared MP sounds, and mark them so they're not cooked into MP maps.
				if ( bSeparateSharedMPResources && PackageIndex == FirstMPMapIndex )
				{
					CookMPSoundPackages( MPCueNames );
				}

				UPackage* Package;

				UObjectReferencer* StandaloneSeekFreeReferencer = NULL;

				// TRUE if after processing this package, we should mark all objects with RF_MarkedByCOoker, and mark
				// materials as already handled
				UBOOL bMarkObjectsAfterProcessing = FALSE;

				// handle combining several packages into one
				if (bIsCombinedStartupPackage)
				{
					SCOPE_SECONDS_COUNTER(LoadCombinedStartupPackagesTime);

					Package = CombinedStartupPackage;
					PackageName = StartupPackageName;
					DstFilename = CookedDir + StartupPackageName + (Platform == PLATFORM_Windows ? TEXT(".upk") : TEXT(".xxx"));

					if (bShouldOnlyLoad)
					{
						warnf( NAME_Log, TEXT("Loading startup packages:"), *PackageName); 
					}
					else
					{
						warnf( NAME_Log, TEXT("Cooking [Combined] %s with:"), *PackageName); 
					}

					// load all the combined startup packages (they'll be in a row)
					for (; PackageIndex < PackageList.Num() && PackageList(PackageIndex).bIsCombinedStartupPackage; PackageIndex++)
					{
						const FFilename& Filename = PackageList(PackageIndex).SrcFilename;
						warnf( NAME_Log, TEXT("   %s"), *Filename.GetBaseFilename()); 

						// load the package and all objects inside
						// get base package name, including stripping off the language
						FString BasePackageName = PackageList(PackageIndex).SrcFilename.GetBaseFilename();
						FString LangExt = FString(TEXT("_")) + UObject::GetLanguage();
						if (BasePackageName.Right(LangExt.Len()) == LangExt)
						{
							BasePackageName = BasePackageName.Left(BasePackageName.Len() - LangExt.Len());
						}
						UPackage* StartupPackage = LoadPackageForCooking(*BasePackageName);
						if ( StartupPackage == NULL )
						{
							return 1;
						}
						//warnf( NAME_Log, TEXT("      (package is %s)"), *StartupPackage->GetName()); 

						// mark anything inside as unmarked, for any objects already loaded (like EngineMaterials.WireframeMaterial, etc)
						for (FObjectIterator It; It; ++It)
						{
							if (It->IsIn(StartupPackage))
							{
								// reference it so SavePackage will save it
								CombinedStartupReferencer->ReferencedObjects.AddItem(*It);
								It->ClearFlags(RF_MarkedByCooker);
							}
						}
					}

					// decrement the PackageIndex as the outer for loop will jump to the next package as well
					PackageIndex--;

					// after saving, mark the objects
					bMarkObjectsAfterProcessing = TRUE;
				}
				else if (bIsStandaloneSeekFreePackage)
				{
					SCOPE_SECONDS_COUNTER(LoadPackagesTime);

					// open the source package
					UPackage* SourcePackage = LoadPackageForCooking(*SrcFilename);
					if ( SourcePackage == NULL )
					{
						return 1;
					}

					// when in user mode, just skip already cooked packages
					if (bIsInUserMode && (SourcePackage->PackageFlags & PKG_Cooked))
					{
						warnf(NAME_Log, TEXT("  Skipping, already cooked")); 
						continue;
					}

					Package = SourcePackage;

					if ( !bCookingPS3Mod )
					{
						// Don't append the seekfree suffix when cooking non-map, content-only mods.
						const FString CookedPackageName = (bIsInUserMode && !bIsTokenMap && !bIsTokenScriptPackage && bIsTokenContentPackage) ? PackageName : PackageName + STANDALONE_SEEKFREE_SUFFIX;
						warnf( NAME_Log, TEXT("Cooking [Seekfree Standalone] %s from %s"), *CookedPackageName, *PackageName);

						// create the new package to cook into
						Package = UObject::CreatePackage(NULL, *CookedPackageName);
						
						// this package needs a new guid, because it's cooked and so SavePackage won't create a new guid for it, expecting it to have a 
						// come from some source linker to get it from, that's not the case
						Package->MakeNewGuid();
						// copy networking package flags from original
						Package->PackageFlags |= SourcePackage->PackageFlags & (PKG_AllowDownload | PKG_ClientOptional | PKG_ServerSideOnly);
						if (!(Package->PackageFlags & PKG_ServerSideOnly))
						{
							Package->CreateEmptyNetInfo();
						}

						// fix up the destination filename
						DstFilename = DstFilename.Replace(*(PackageName + TEXT(".")), *(CookedPackageName + TEXT(".")));
						
						//create an object to reference all the objects in the original package
						StandaloneSeekFreeReferencer = ConstructObject<UObjectReferencer>( UObjectReferencer::StaticClass(), Package, NAME_None, RF_Cooked );

						for (FObjectIterator It; It; ++It)
						{
							if (It->IsIn(SourcePackage))
							{
								// reference it so SavePackage will save it
								StandaloneSeekFreeReferencer->ReferencedObjects.AddItem(*It);
							}
						}
					}
					else
					{
						warnf( NAME_Log, TEXT("Cooking [Seekfree Standalone] %s"), *PackageName);
						for (FObjectIterator It; It; ++It)
						{
							if (It->IsIn(SourcePackage))
							{
								// Mark the object as RF_Standalone so SavePackge will save it.
								It->SetFlags( RF_Standalone );
							}
						}
					}
				}
				else
				{
					SCOPE_SECONDS_COUNTER(LoadPackagesTime);

					warnf( NAME_Log, TEXT("Cooking%s%s %s"), 
						bShouldBeSeekFree ? TEXT(" [Seekfree]") : TEXT(""),
						bShouldBeFullyCompressed ? TEXT(" [Fully compressed]") : TEXT(""),
						*SrcFilename.GetBaseFilename() );

					Package = LoadPackageForCooking(*SrcFilename);
				}

				STAT(InitializePackageTime += appSeconds());
				STAT(FinalizePackageTime -= appSeconds());

				if (Package && !bShouldOnlyLoad)
				{
					// when in user mode, just skip already cooked packages
					if (bIsInUserMode && (Package->PackageFlags & PKG_Cooked))
					{
						warnf(NAME_Log, TEXT("  Skipping, already cooked")); 
						continue;
					}


					DWORD OriginalPackageFlags = Package->PackageFlags;

					// Don't try cooking already cooked data!
					checkf( !(Package->PackageFlags & PKG_Cooked), TEXT("%s is already cooked!"), *SrcFilename ); 
					Package->PackageFlags |= PKG_Cooked;

					// Find the UWorld object (only valid if we're looking at a map).
					UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );

					// Initialize components, so they have correct bounds, Owners and such
					if( World && World->PersistentLevel )
					{
						// Reset cooked physics data counters.
						BrushPhysDataBytes = 0;

						World->PersistentLevel->UpdateComponents();
					}

					if (bShouldBeSeekFree && World != NULL)
					{
						// look up per-map packages
						TArray<FName>* PerMapPackages = PerMapCookPackages.Find(Package->GetFName());

						if (PerMapPackages != NULL)
						{
							SCOPE_SECONDS_COUNTER(LoadPerMapPackagesTime);

							for (INT PerMapPackageIndex = 0; PerMapPackageIndex < PerMapPackages->Num(); PerMapPackageIndex++)
							{
								FName PackageName = (*PerMapPackages)(PerMapPackageIndex);

								UPackage* Pkg = LoadPackageForCooking(*PackageName.ToString());
								if (Pkg != NULL)
								{
									warnf( NAME_Log, TEXT("   Forcing %s into %s"), *PackageName.ToString(), *SrcFilename.GetBaseFilename());

									// reference all objects in this package with the referencer
									for (FObjectIterator It; It; ++It)
									{
										if (It->IsIn(Pkg))
										{
											// only add objects that are going to be cooked into this package (public, not already marked)
											if (!It->HasAnyFlags(RF_MarkedByCooker|RF_Transient) && It->HasAllFlags(RF_Public))
											{
												World->ExtraReferencedObjects.AddItem(*It);
											}
										}
									}
								}
								else
								{
									warnf( NAME_Log, TEXT("   Failed to force %s into %s"), *PackageName.ToString(), *SrcFilename.GetBaseFilename());
								}
							}
						}
					}

					// Disallow lazy loading for all packages
					Package->PackageFlags |= PKG_DisallowLazyLoading;

					// packages that should be fully compressed need to be marked as such
					if( bShouldBeFullyCompressed )
					{
						Package->PackageFlags |= PKG_StoreFullyCompressed;
					}

					// Handle creation of seek free packages/ levels.
					if( bShouldBeSeekFree )
					{
						Package->PackageFlags |= PKG_RequireImportsAlreadyLoaded;
						// fully compressed packages should not be marked with the PKG_StoreCompressed flag, as that will compress
						// on the export level, not the package level
						if( !bShouldBeFullyCompressed )
						{
							Package->PackageFlags |= PKG_StoreCompressed;
						}

						// Iterate over all objects and mark them as RF_ForceTagExp where appropriate.
						for( FObjectIterator It; It; ++It )
						{
							UObject* Object = *It;
							// Exclude objects that are either marked "marked" or transient.
							if( !Object->HasAnyFlags( RF_MarkedByCooker|RF_Transient ) 
							// Exclude objects that reside in the transient package.
							&&	!Object->IsIn( UObject::GetTransientPackage() ) 
							// Exclude objects that are in the package as they will be serialized anyways.
							&&	!Object->IsIn( Package ) )
							{
								UBOOL bShouldDuplicate = TRUE;
								// If we're cooking an MP map . . .
								if ( bIsMultiplayerMap && bSeparateSharedMPResources )
								{
									const FString ObjectPath = Object->GetPathName();
									if ( MPObjectNames.ContainsItem( ObjectPath ) )
									{
										bShouldDuplicate = FALSE;
									}
								}
								if ( bShouldDuplicate )
								{
									// Mark object so it gets forced into the export table.
									Object->SetFlags( RF_ForceTagExp );
								}
							}
						}
					}

					// make a local shader cache in each of the seekfree packages for consoles
					if (bShouldBeSeekFree && Platform != PLATFORM_Windows)
					{
						// We can't save the shader cache into Core as the UShaderCache class is loaded afterwards.
						if( Package->GetFName() != NAME_Core )
						{
							// Construct a shader cache in the package with RF_Standalone so it is being saved into the package even though it is not
							// referenced by any objects in the case of passing in a root, like done e.g. when saving worlds.
							ShaderCache = new(Package,TEXT("SeekFreeShaderCache"),RF_Standalone) UShaderCache(ShaderPlatform);
						}
					}

					// Handle duplicating cubemap faces. This needs to be done before the below object iterator as it creates new objects.
					HandleCubemaps( Package );

					// Replace static mesh actors with a StaticMeshCollectionActor
					if ( !bStripEverythingButTextures )
					{
						if ( bCookOutStaticMeshActors )
						{
							CookStaticMeshActors( Package );
						}

						if ( bCookOutStaticLightActors )
						{
							CookStaticLightActors(Package);
						}

						if (bBakeAndPruneDuringCook)
						{
							BakeAndPruneMatinee(Package);
						}
					}

					// Cook localization...
#if 0
					if ( !bStripEverythingButTextures )
					{
						PrepPackageLocalization(Package);
					}
#endif

					// Iterate over all objects and cook them if they are going to be saved with the package.
					for( FObjectIterator It; It; ++It )
					{
						UObject* Object = *It;
						if( Object->IsIn( Package ) || Object->HasAnyFlags( RF_ForceTagExp ) )
						{
							// Don't cook non texture objects for packages that will only contain texture mips.
							if( !bStripEverythingButTextures || Object->IsA(UTexture2D::StaticClass()) )
							{
								// Cook object.
								CookObject( Package, Object, bShouldBeSeekFree );
							}
						}
					}

					// Iterate over all objects and prepare for saving them if they are going to be saved with the package.
					// This is done in two passes now in case the CookObject above removes objects from being saved (see 
					// particle thumbnails in CookParticleSystem)
					for( FObjectIterator It; It; ++It )
					{
						UObject* Object = *It;
						if( Object->IsIn( Package ) || Object->HasAnyFlags( RF_ForceTagExp ) )
						{
							// Don't cook non texture objects for packages that will only contain texture mips.
							if( !bStripEverythingButTextures || Object->IsA(UTexture2D::StaticClass()) )
							{
								// Prepare it for serialization.
								PrepareForSaving( Package, Object, bShouldBeSeekFree, bStripEverythingButTextures );
							}
						}
					}

					// Clear components before saving and recook static mesh physics data cache.
					if( World && World->PersistentLevel )
					{
						// Re-build the cooked staticmesh physics data cache for the map, in target format.
						World->PersistentLevel->BuildPhysStaticMeshCache();

						// Clear components before we save.
						World->PersistentLevel->ClearComponents();

						// Print out physics cooked data sizes.
						debugfSuppressed( NAME_DevCooking, TEXT("COOKEDPHYSICS: Brush: %f KB"), ((FLOAT)BrushPhysDataBytes)/1024.f );
					}

					// Figure out whether there are any localized resource that would be saved into seek free package and create a special case package
					// that is loaded upfront instead.
					//
					// @warning: Engine does not support localized materials. Mostly because the localized packages don't have their own shader cache.
					UBOOL bIsLocalizedScriptPackage = bIsNativeScriptFile && (PackageIndex == FirstGameScriptIndex) && bShouldBeSeekFree;
					UBOOL bSupportsLocalization = bIsLocalizedScriptPackage || (!bIsNativeScriptFile && bShouldBeSeekFree);
					if( bSupportsLocalization && AreThereLocalizedResourcesPendingSave( Package, 0 ) )
					{
						const UBOOL bCookingEnglish = appStricmp( TEXT("INT"), UObject::GetLanguage() ) == 0;

						// The cooker should not save out localization packages when cooking for non-English if no non-English resources were found.
						if ( bCookingEnglish || AreThereNonEnglishLocalizedResourcesPendingSave( Package ) )
						{
							// Figure out localized package name and file name.
							FFilename LocalizedPackageName		= FFilename(DstFilename.GetBaseFilename() + LOCALIZED_SEEKFREE_SUFFIX).GetLocalizedFilename();
							FFilename LocalizedPackageFilename	= DstFilename.GetPath() + PATH_SEPARATOR + LocalizedPackageName + (Platform == PLATFORM_Windows ? TEXT(".upk") : TEXT(".xxx"));

							// Create a localized package and disallow lazy loading as we're seek free.
							UPackage* LocalizedPackage = CreatePackage( NULL, *LocalizedPackageName );
							LocalizedPackage->PackageFlags |= PKG_Cooked;
							LocalizedPackage->PackageFlags |= PKG_DisallowLazyLoading;
							LocalizedPackage->PackageFlags |= PKG_RequireImportsAlreadyLoaded;
							LocalizedPackage->PackageFlags |= PKG_ServerSideOnly;
							// fully compressed packages should not be marked with the PKG_StoreCompressed flag, as that will compress
							// on the export level, not the package level
							if (bShouldBeFullyCompressed)
							{
								LocalizedPackage->PackageFlags |= PKG_StoreFullyCompressed;
							}
							else
							{
								LocalizedPackage->PackageFlags |= PKG_StoreCompressed;
							}
							// Create object references within package.
							UObjectReferencer* ObjectReferencer = ConstructObject<UObjectReferencer>( UObjectReferencer::StaticClass(), LocalizedPackage, NAME_None, RF_Cooked );

							// Reference all localized audio with the appropriate flags by the helper object.
							for( FObjectIterator It; It; ++It )
							{
								UObject* Object = *It;
								if( Object->IsLocalizedResource() )
								{
									if( Object->HasAnyFlags(RF_ForceTagExp) )
									{
										ObjectReferencer->ReferencedObjects.AddItem( Object );
									}
									// We don't localize objects that are part of the map package as we can't easily avoid duplication.
									else if( Object->IsIn( Package ) )
									{
										warnf( NAME_Warning, TEXT("Localized resources cannot reside in map packages! Please move %s."), *Object->GetFullName() );
									}
								}
							}

							// Save localized assets referenced by object referencer.
							check( bStripEverythingButTextures == FALSE );
							SaveCookedPackage( LocalizedPackage, ObjectReferencer, RF_Standalone, *LocalizedPackageFilename, FALSE);

							// Prevent objects from being saved into seekfree package.
							for( INT ResourceIndex=0; ResourceIndex<ObjectReferencer->ReferencedObjects.Num(); ResourceIndex++ )
							{
								UObject* Object = ObjectReferencer->ReferencedObjects(ResourceIndex);
								Object->ClearFlags( RF_ForceTagExp );
								// Avoid object from being put into subsequent script or map packages.
								if( bIsLocalizedScriptPackage && Object->HasAnyFlags( RF_TagExp ) )
								{
									Object->SetFlags( RF_MarkedByCooker );
								}
							}

							// If an asset B has a loc asset A as its outer then A gets the RF_ForceTagExp flag
							// cleared while B does not.  This goes through and performs the same process on all B's
							// that has an outer in the ReferencedObjects list.
							for( FObjectIterator It; It; ++It )
							{
								UObject *Object = *It;
								for( INT ResourceIndex=0; ResourceIndex<ObjectReferencer->ReferencedObjects.Num(); ResourceIndex++ )
								{
									UObject* PossibleOuter = ObjectReferencer->ReferencedObjects(ResourceIndex);
									if( Object->IsIn(PossibleOuter) )
									{
										Object->ClearFlags( RF_ForceTagExp );
										// Avoid object from being put into subsequent script or map packages.
										if( bIsLocalizedScriptPackage && Object->HasAnyFlags( RF_TagExp ) )
										{
											Object->SetFlags( RF_MarkedByCooker );
										}
									}
								}
							}
						}
					}

					// Save the cooked package if wanted.
					if( !World || !bSkipSavingMaps )
					{
						UObject* Base = World;
						if (bIsCombinedStartupPackage)
						{
							check(CombinedStartupReferencer);
							Base = CombinedStartupReferencer;
						}
						else if (bIsStandaloneSeekFreePackage)
						{
							if ( bCookingPS3Mod )
							{
								Base = NULL;
							}
							else
							{
								check(StandaloneSeekFreeReferencer);
								Base = StandaloneSeekFreeReferencer;
							}
						}

						// Only script mod packages support auto downloading.
						if ( bIsTokenContentPackage || bIsTokenMap )
						{
							Package->PackageFlags &= ~PKG_AllowDownload;
						}

						// cache the file age of the source file now that we've cooked it
						PersistentCookerData->SetFileTime(*DstFilename, GFileManager->GetFileTimestamp(*SrcFilename));

						SaveCookedPackage( Package, 
							Base, 
							RF_Standalone, 
							*DstFilename, 
							bStripEverythingButTextures,
							bShouldBeSeekFree, // should we clean up objects for next packages
							bIsNativeScriptFile || bIsCombinedStartupPackage); // should we mark saved objects to not be saved again

					}

					// restore original package flags so that if future cooking loads this package again the code doesn't get confused and think it's loading a cooked package
					Package->PackageFlags = OriginalPackageFlags;

					// NULL out shader cache reference before garbage collection to avoid dangling pointer.
					ShaderCache = NULL;
				}
				else if (!Package)
				{
					warnf( NAME_Error, TEXT("Failed loading %s"), *SrcFilename );
				}

				if (bMarkObjectsAfterProcessing)
				{
					// Make sure that none of the currently existing objects get put into a map file. This is required for
					// the code to work correctly in combination with not always recooking script packages.
					// Iterate over all objects, setting RF_MarkedByCooker and marking materials as already handled.
					for( FObjectIterator It; It; ++It )
					{
						UObject*	Object		= *It;
						UMaterial*	Material	= Cast<UMaterial>(Object);
						UMaterialInstance*	MaterialInstance	= Cast<UMaterialInstance>(Object);
						// Don't put into any seekfree packages.
						Object->SetFlags( RF_MarkedByCooker );
						// Don't put material into seekfree shader caches. Also remove all expressions.
						if( Material )
						{
							AlreadyHandledMaterials.Add(Material->GetFullName());
						}

						if( MaterialInstance )
						{
							AlreadyHandledMaterialInstances.Add(MaterialInstance->GetFullName());
						}
					}
				}

				STAT(FinalizePackageTime += appSeconds());
			}
		}

		STAT(PostPackageIterationTime -= appSeconds());

		// always save final version of the shader cache
		{
			SCOPE_SECONDS_COUNTER(SaveShaderCacheTime);
			SaveLocalShaderCaches();
		}

		// merge data if desired
		if (bMergeJobs)
		{
			TArray<FString> Dirs;
			// get a list of the directories in the jobs dir
			GFileManager->FindFiles(Dirs, *(appGameDir() + FString(TEXT("Jobs-")) + GetPlatformString() * FString(TEXT("*.*"))), FALSE, TRUE);
			CombineCaches(Dirs);
		}

		if (Platform == PLATFORM_Windows)
		{
			SCOPE_SECONDS_COUNTER(CopyShaderCacheTime);

			// Cooked PC builds need the cooked reference shader caches for all PC shader platforms.
			CookReferenceShaderCache(SP_PCD3D_SM2);
			CookReferenceShaderCache(SP_PCD3D_SM3);
			CookReferenceShaderCache(SP_PCD3D_SM4);

			// Cooked PC builds also need the cooked global shader caches for all PC shader platforms.
			CookGlobalShaderCache(SP_PCD3D_SM2);
			CookGlobalShaderCache(SP_PCD3D_SM3);
			CookGlobalShaderCache(SP_PCD3D_SM4);
		}
		else
		{
			// Non-PC cooked builds only need the cooked global shader cache a single shader platform.
			CookGlobalShaderCache(ShaderPlatform);
		}

		// Print out detailed stats
		TotalTime = appSeconds() - TotalTime;
		STAT(PostPackageIterationTime += appSeconds());

		warnf( NAME_Log, TEXT("") );
		warnf( NAME_Log, TEXT("TotalTime                                %7.2f seconds"), TotalTime );
		warnf( NAME_Log, TEXT("  LoadSectionPackages                    %7.2f seconds"), LoadSectionPackagesTime );
		warnf( NAME_Log, TEXT("  LoadNativePackages                     %7.2f seconds"), LoadNativePackagesTime );
		warnf( NAME_Log, TEXT("  LoadDependencies                       %7.2f seconds"), LoadDependenciesTime );
		warnf( NAME_Log, TEXT("  LoadPackages                           %7.2f seconds"), LoadPackagesTime );
		warnf( NAME_Log, TEXT("  LoadPerMapPackages                     %7.2f seconds"), LoadPerMapPackagesTime );
		warnf( NAME_Log, TEXT("  LoadCombinedStartupPackages            %7.2f seconds"), LoadCombinedStartupPackagesTime );
		warnf( NAME_Log, TEXT("  CheckDependentPackages                 %7.2f seconds"), CheckDependentPackagesTime );
		warnf( NAME_Log, TEXT("  Load ShaderCache                       %7.2f seconds"), GShaderCacheLoadTime );
		warnf( NAME_Log, TEXT("  Save ShaderCache                       %7.2f seconds"), SaveShaderCacheTime );
		warnf( NAME_Log, TEXT("  Copy ShaderCache                       %7.2f seconds"), CopyShaderCacheTime );
		warnf( NAME_Log, TEXT("  RHI shader compile time                %7.2f seconds"), GRHIShaderCompileTime_Total );
		warnf( NAME_Log, TEXT("    PS3                                  %7.2f seconds"), GRHIShaderCompileTime_PS3 );
		warnf( NAME_Log, TEXT("    XBOXD3D                              %7.2f seconds"), GRHIShaderCompileTime_XBOXD3D );
		warnf( NAME_Log, TEXT("    PCD3D_SM2                            %7.2f seconds"), GRHIShaderCompileTime_PCD3D_SM2 );
		warnf( NAME_Log, TEXT("    PCD3D_SM3                            %7.2f seconds"), GRHIShaderCompileTime_PCD3D_SM3 );
		warnf( NAME_Log, TEXT("    PCD3D_SM4                            %7.2f seconds"), GRHIShaderCompileTime_PCD3D_SM4 );
		warnf( NAME_Log, TEXT("  CookTime                               %7.2f seconds"), CookTime );
		warnf( NAME_Log, TEXT("    CookPhysics                          %7.2f seconds"), CookPhysicsTime );
		warnf( NAME_Log, TEXT("    CookTexture                          %7.2f seconds"), CookTextureTime );
		warnf( NAME_Log, TEXT("    CookSound                            %7.2f seconds"), CookSoundTime );
		warnf( NAME_Log, TEXT("    LocSound                             %7.2f seconds"), LocSoundTime );
		warnf( NAME_Log, TEXT("    CookMovie                            %7.2f seconds"), CookMovieTime );
		warnf( NAME_Log, TEXT("    CookStrip                            %7.2f seconds"), CookStripTime );
		warnf( NAME_Log, TEXT("    CookSkeletalMesh                     %7.2f seconds"), CookSkeletalMeshTime );
		warnf( NAME_Log, TEXT("    CookStaticMesh                       %7.2f seconds"), CookStaticMeshTime );
		warnf( NAME_Log, TEXT("  PackageSave                            %7.2f seconds"), PackageSaveTime );
		warnf( NAME_Log, TEXT("  PrepareForSaving                       %7.2f seconds"), PrepareForSavingTime );
		warnf( NAME_Log, TEXT("    PrepareForSavingTexture              %7.2f seconds"), PrepareForSavingTextureTime );
		warnf( NAME_Log, TEXT("    PrepareForSavingTerrain              %7.2f seconds"), PrepareForSavingTerrainTime );
		warnf( NAME_Log, TEXT("    PrepareForSavingMaterial             %7.2f seconds"), PrepareForSavingMaterialTime );
		warnf( NAME_Log, TEXT("    PrepareForSavingMaterialInstance     %7.2f seconds"), PrepareForSavingMaterialInstanceTime );
		warnf( NAME_Log, TEXT("  PackageLocTime                         %7.2f seconds"), PackageLocTime );
		warnf( NAME_Log, TEXT("  CollectGarbageAndVerify                %7.2f seconds"), CollectGarbageAndVerifyTime );
		warnf( NAME_Log, TEXT("") );
		warnf( NAME_Log, TEXT("Regional Stats:") );
		warnf( NAME_Log, TEXT("  Before Package Iteration               %7.2f seconds"), PrePackageIterationTime );
		warnf( NAME_Log, TEXT("  Package Iteration                      %7.2f seconds"), PackageIterationTime );
		warnf( NAME_Log, TEXT("    Prep Package                         %7.2f seconds"), PrepPackageTime );
		warnf( NAME_Log, TEXT("    Initialize Package                   %7.2f seconds"), InitializePackageTime );
		warnf( NAME_Log, TEXT("    Finalize Package                     %7.2f seconds"), FinalizePackageTime );
		warnf( NAME_Log, TEXT("  After Package Iteration                %7.2f seconds"), PostPackageIterationTime );
		warnf( NAME_Log, TEXT("") );
		warnf( NAME_Log, TEXT("Compression Stats:") );
		warnf( NAME_Log, TEXT("  FArchive::SerializeCompressed time     %7.2f seconds"), GArchiveSerializedCompressedSavingTime );
		warnf( NAME_Log, TEXT("  Compressor thread time                 %7.2f seconds"), GCompressorTime );
		warnf( NAME_Log, TEXT("  Compressed src bytes                   %7i MByte"), GCompressorSrcBytes / 1024 / 1024 );
		warnf( NAME_Log, TEXT("  Compressor dst bytes                   %7i MByte"), GCompressorDstBytes / 1024 / 1024 );
		warnf( NAME_Log, TEXT("") );
		warnf( NAME_Log, TEXT("PersistentFaceFX Stats:") );
		warnf( NAME_Log, TEXT("  Total time	                            %7.2f seconds"), PersistentFaceFXTime );
		warnf( NAME_Log, TEXT("  Determination time                     %7.2f seconds"), PersistentFaceFXDeterminationTime );
		warnf( NAME_Log, TEXT("  Generation time                        %7.2f seconds"), PersistentFaceFXGenerationTime );

		if( GCompressorSrcBytes )
		{
			warnf( NAME_Log, TEXT("  Compression ratio                      %7.2f %%"), 100.f * GCompressorDstBytes / GCompressorSrcBytes );
		}

		if (GGameCookerHelper)
		{
			GGameCookerHelper->DumpStats();
		}

		// Frees DLL handles and deletes cooker objects.
		Cleanup();
	}

	if( bGenerateSHAHashes )
	{
		// generate SHA hashes if desired
		GenerateSHAHashes();
	}

	// Close texture file caches.
	for(TMap<FName,FArchive*>::TIterator TextureCacheIt(TextureCacheNameToArMap);TextureCacheIt;++TextureCacheIt)
	{
		delete TextureCacheIt.Value();
	}
	TextureCacheNameToArMap.Empty();

	// make sure we delete any copied shader files
	CleanupShaderFiles();

	if (bUseTextureFileCache && bVerifyTextureFileCache)
	{
		// Verify the supposed offsets w/ the contents of the TextureFileCache.
		VerifyTextureFileCacheEntry();
	}

	return 0;
}

/**
* If -sha is specified then iterate over all FilesForSHA and generate their hashes
* The results are written to Hashes.sha
*/
void UCookPackagesCommandlet::GenerateSHAHashes()
{
	// add the fully loaded packages to the list to verify (since we fully load the memory for
	// them, it is not crazy to verify the data at runtime) if we are going to preload anything
	UBOOL bSerializeStartupPackagesFromMemory = FALSE;
	GConfig->GetBool(TEXT("Engine.StartupPackages"), TEXT("bSerializeStartupPackagesFromMemory"), bSerializeStartupPackagesFromMemory, PlatformEngineConfigFilename);

	if (bSerializeStartupPackagesFromMemory)
	{
		if (Platform & PLATFORM_PC)
		{
			// Get combined list of all script package names, including editor- only ones.
			TArray<FString>	 AllScriptPackageNames;
			appGetEngineScriptPackageNames( AllScriptPackageNames, TRUE );
			appGetGameNativeScriptPackageNames( AllScriptPackageNames, TRUE );
			appGetGameScriptPackageNames( AllScriptPackageNames, TRUE );

			// turn them into file names that can be opened
			for (INT PackageIndex = 0; PackageIndex < AllScriptPackageNames.Num(); PackageIndex++)
			{
				// see if the file exists and get the filename + extension
				FString PackageFilename;
				if( GPackageFileCache->FindPackageFile(*AllScriptPackageNames(PackageIndex), NULL, PackageFilename) )
				{
					const FFilename SrcFilename(PackageFilename);
					UBOOL bIsScriptFile = AllScriptPackageNames.FindItemIndex(SrcFilename.GetBaseFilename()) != INDEX_NONE;
					if( bIsScriptFile )
					{
						// convert the filename to the cooked path
						FFilename DestFilename = GetCookedPackageFilename(SrcFilename);
						FilesForSHA.AddItem( DestFilename );
					}
				}
				else
				{
					warnf(TEXT("SHA: package not found [%s]"),*AllScriptPackageNames(PackageIndex));
				}
			}

			// get the startup packages
			TArray<FString> StartupPackages;
			appGetAllPotentialStartupPackageNames(StartupPackages, PlatformEngineConfigFilename);

			// add all startup packages except for script 
			for (INT PackageIndex = 0; PackageIndex < StartupPackages.Num(); PackageIndex++)
			{
				// see if the file exists and get the filename + extension
				FString PackageFilename;
				if( GPackageFileCache->FindPackageFile(*StartupPackages(PackageIndex), NULL, PackageFilename) )
				{
					const UBOOL bShouldSignStartupContentPackages = FALSE;
					const FFilename SrcFilename(PackageFilename);
					UBOOL bIsScriptFile = AllScriptPackageNames.FindItemIndex(SrcFilename.GetBaseFilename()) != INDEX_NONE;
					UBOOL bIsMapFile = (SrcFilename.GetExtension() == FURL::DefaultMapExt) || (SrcFilename.GetBaseFilename() == TEXT("entry"));
					UBOOL bIsCombinedStartupPackage = !(bIsScriptFile || bIsMapFile);

					if( !bIsScriptFile && (!bIsCombinedStartupPackage || !bShouldSignStartupContentPackages) )
					{
						// convert the filename to the cooked path
						FFilename DestFilename = GetCookedPackageFilename(SrcFilename);
						FilesForSHA.AddItem( DestFilename );
					}
				}
				else
				{
					warnf(TEXT("SHA: package not found [%s]"),*StartupPackages(PackageIndex));
				}

			}

			// also add the "Startup_int" package and all of its language versions
			for (INT LangIndex = 0; GKnownLanguageExtensions[LangIndex]; LangIndex++)
			{
				const TCHAR* Ext = GKnownLanguageExtensions[LangIndex];
				FilesForSHA.AddItem(*FString(CookedDir + TEXT("Startup_") + Ext + TEXT(".upk")));
			}

			// add all of the shader source files for hashing
			TArray<FString> ShaderSourceFiles;
			appGetAllShaderSourceFiles(ShaderSourceFiles);
			for( INT ShaderFileIdx=0; ShaderFileIdx < ShaderSourceFiles.Num(); ShaderFileIdx++ )
			{
				FString ShaderFilename = FString(appShaderDir()) * ShaderSourceFiles(ShaderFileIdx);
				ShaderFilename += TEXT(".usf");
				FilesForSHA.AddItem( ShaderFilename );
			}
		}
		else // Console
		{
			// get the startup packages
			TArray<FString> StartupPackages;
			appGetAllPotentialStartupPackageNames(StartupPackages, PlatformEngineConfigFilename, TRUE);

			// turn them into file names that can be opened
			for (INT PackageIndex = 0; PackageIndex < StartupPackages.Num(); PackageIndex++)
			{
				FilesForSHA.AddItem(*(CookedDir + (FFilename(StartupPackages(PackageIndex)).GetBaseFilename()) + TEXT(".xxx")));
			}
		}
	}
	else
	{
		warnf(TEXT("SHA: Skipping startup packages. bSerializeStartupPackagesFromMemory=True is required."));
	}


	warnf(TEXT("Generating Hashes.sha:"));
	// open the output file to store all the hashes
	FArchive* SHAWriter = GFileManager->CreateFileWriter(*(appGameDir() + TEXT("Build") PATH_SEPARATOR TEXT("Hashes.sha")));
	if( SHAWriter == NULL )
	{
		warnf( TEXT( "SHA: Unable to create hashes.sha; is it checked out?" ) );
		return;
	}

	for (INT SHAIndex = 0; SHAIndex < FilesForSHA.Num(); SHAIndex++)
	{
		FString& Filename = FilesForSHA(SHAIndex);
		TArray<BYTE> Contents;

		// look to see if the file was fully compressed
		FString UncompressedSizeStr;
		if (appLoadFileToString(UncompressedSizeStr, *(Filename + TEXT(".uncompressed_size"))))
		{
			// if it was, we need to generate the hash on the uncomprseed
			INT UncompressedSize = appAtoi(*UncompressedSizeStr);

			// read and uncompress the data
			FArchive* Reader = GFileManager->CreateFileReader(*Filename);
			if (Reader)
			{
				warnf(TEXT("  Decompressing %s"), *Filename);
				Contents.Add(UncompressedSize);
				Reader->SerializeCompressed(Contents.GetData(), 0, GBaseCompressionMethod);
				delete Reader;
			}
		}
		else
		{
			// otherwise try to load the file, checking first so the Error log inside
			// doesn't look like a failure, as it's okay to not exist
			if (GFileManager->FileSize(*Filename) >= 0)
			{
				appLoadFileToArray(Contents, *Filename);
			}
		}

		// is there any data to hash?
		if (Contents.Num() != 0)
		{
			warnf(TEXT("  Hashing %s"), *Filename);
			// if we loaded it, generate the hash
			BYTE Hash[20];
			FSHA1::HashBuffer(Contents.GetData(), Contents.Num(), Hash);

			// write out the filename (including trailing 0)
			// don't write the file path
			FString CleanFilename( FFilename(Filename).GetCleanFilename().ToLower() );
			SHAWriter->Serialize(TCHAR_TO_ANSI(*CleanFilename), CleanFilename.Len() + 1);

			// write out hash
			SHAWriter->Serialize(Hash, sizeof(Hash));
		}
		else
		{
			warnf(TEXT(" File not found %s"), *Filename);
		}
	}

	// close the file
	delete SHAWriter;
}

/**
 * Merges shader caches of the matching CookShaderPlatform and saves the reference and global caches into the cooked directory.
 */
void UCookPackagesCommandlet::CookReferenceShaderCache(EShaderPlatform CookShaderPlatform)
{
	UShaderCache* LocalShaderCache = GetLocalShaderCache(CookShaderPlatform);
	UShaderCache* RefShaderCache = GetReferenceShaderCache(CookShaderPlatform);

	// get the destination reference shader cache package name
	FFilename ReferenceShaderCachePath = GetReferenceShaderCacheFilename(CookShaderPlatform);
	FFilename AfterGameDir = ReferenceShaderCachePath.Right(ReferenceShaderCachePath.Len() - appGameDir().Len());
	FFilename AfterTopLevelDir = AfterGameDir.Right(AfterGameDir.Len() - (AfterGameDir.InStr(PATH_SEPARATOR) + 1));

	// check file times looking for updated files
	DOUBLE LocalSrcTime = GFileManager->GetFileTimestamp(*GetLocalShaderCacheFilename(CookShaderPlatform));
	DOUBLE RefSrcTime = GFileManager->GetFileTimestamp(*GetReferenceShaderCacheFilename(CookShaderPlatform));
	DOUBLE RefDstTime = GFileManager->GetFileTimestamp(*(CookedDir + AfterTopLevelDir));

	// check if either of the "uncooked" shaders are newer
	UBOOL bIsLocalNewer = LocalSrcTime != -1.0 && LocalSrcTime > RefDstTime;
	UBOOL bIsRefNewer = RefSrcTime != -1.0 && RefSrcTime > RefDstTime;

	// make sure there's something to do
	if ((LocalShaderCache || RefShaderCache) &&
		(bIsLocalNewer || bIsRefNewer || (LocalShaderCache && LocalShaderCache->IsDirty())))
	{
		// if only the local shader cache exists, save that as the reference
		if (bIsDistributed || (LocalShaderCache && !RefShaderCache))
		{
			RefShaderCache = LocalShaderCache;
		}
		// if they both exist, merge the local into the reference
		else if (LocalShaderCache && RefShaderCache)
		{
			RefShaderCache->Merge(LocalShaderCache);
		}

		warnf(NAME_Log, TEXT("Copying shader cache %s"), *AfterTopLevelDir);

		// save the reference shader cache into the cooked directory
		UPackage* ReferenceShaderCachePackage = RefShaderCache->GetOutermost();
		ReferenceShaderCachePackage->PackageFlags |= PKG_ServerSideOnly;
		UObject::SavePackage(ReferenceShaderCachePackage, RefShaderCache, 0, *(CookedDir + AfterTopLevelDir));
	}
}

void UCookPackagesCommandlet::CookGlobalShaderCache(EShaderPlatform CookShaderPlatform)
{
	// On all platforms, write the GlobalShaders file into the cooked folder with the target platform's native byte order.
	const FFilename GlobalShaderCacheFilename = GetGlobalShaderCacheFilename(CookShaderPlatform);
	const FFilename CookedGlobalShaderCacheFilename = appGameDir() * GetCookedDirectory() * GlobalShaderCacheFilename.GetCleanFilename();
	FArchive* GlobalShaderCacheFile = GFileManager->CreateFileWriter(*CookedGlobalShaderCacheFilename);
	if(!GlobalShaderCacheFile)
	{
		appErrorf(TEXT("Failed to save cooked global shader cache for %s."),ShaderPlatformToText(CookShaderPlatform));
	}
	GlobalShaderCacheFile->SetByteSwapping(ShouldByteSwapData());
	SerializeGlobalShaders(CookShaderPlatform,*GlobalShaderCacheFile);
	delete GlobalShaderCacheFile;
}

/**
 * Merge together the cache files resulting from distributed cooking jobs
 *
 * @param JobDirectories List of directories (inside appGameDir()) to merge together
 */
void UCookPackagesCommandlet::CombineCaches(const TArray<FString>& JobDirectories)
{
	for (INT JobIndex = 0; JobIndex < JobDirectories.Num(); JobIndex++)
	{
		// get a list of all files in the job directory
		TArray<FString> JobFiles;
		FString JobDir = appGameDir() + FString(TEXT("Jobs-")) + GetPlatformString() * JobDirectories(JobIndex);
		appFindFilesInDirectory(JobFiles, *JobDir, TRUE, TRUE);

		// go over each file, looking for special ones, and copying the others
		for (INT FileIndex = 0; FileIndex < JobFiles.Num(); FileIndex++)
		{
			FFilename FilePath = JobFiles(FileIndex);
			FFilename Filename = FilePath.GetCleanFilename();

			UBOOL bWasFileHandled = FALSE;

			// look for the persistent cooker data
			if (Filename == GetBulkDataContainerFilename())
			{
				// create a unique package to not stomp on another one
				UPackage* UniquePackage = UObject::CreatePackage(NULL, *FString::Printf(TEXT("PersistentCooker_%d_%d"), JobIndex, FileIndex));

				// load the persistent data
				UPackage* Package = UObject::LoadPackage(UniquePackage, *FilePath, LOAD_NoWarn );

				// merge this into the global one
				if (Package)
				{
					warnf(NAME_Log, TEXT("Merging %s into global persistent cooker data"), *FilePath);
					UPersistentCookerData* JobData = FindObject<UPersistentCookerData>(Package, TEXT("PersistentCookerData"));
					check(JobData);
					PersistentCookerData->Merge(JobData, this, *(JobDir * GetCookedDirectory()));
				}

				// mark that we handled this job file, and don't need to copy it
				bWasFileHandled = TRUE;
			}
			else if (Filename == GetGuidCacheFilename())
			{
				// create a unique package to not stomp on another one
				UPackage* UniquePackage = UObject::CreatePackage(NULL, *FString::Printf(TEXT("GuidCache_%d_%d"), JobIndex, FileIndex));

				// load the persistent data
				UPackage* Package = UObject::LoadPackage(UniquePackage, *FilePath, LOAD_NoWarn );

				// merge this into the global one
				if (Package)
				{
					warnf(NAME_Log, TEXT("Merging %s into global guid cache"), *FilePath);
					UGuidCache* JobGuidCache = FindObject<UGuidCache>(Package, TEXT("GuidCache"));
					check(JobGuidCache);
					GuidCache->Merge(JobGuidCache);
				}

				// mark that we handled this job file, and don't need to copy it
				bWasFileHandled = TRUE;
			}
			else if (FilePath.GetExtension() == GSys->TextureFileCacheExtension)
			{
				// nothing to do, handled in the persistentcookerdata merging
				bWasFileHandled = TRUE;
			}
			else
			{
				EShaderPlatform ShaderPlatformToMerge = ShaderPlatform;
				UBOOL bIsFileShaderCache = FALSE;
				
				// look for shader caches to merge
				for (INT ShaderPlatformIndex = 0; ShaderPlatformIndex < (Platform == PLATFORM_Windows ? 3 : 1); ShaderPlatformIndex++)
				{
					// on PC, we merge the result refshadercache from the cook step
					if (Platform == PLATFORM_Windows)
					{

						// get the platform we are cooking/merging for
						if (Platform == PLATFORM_Windows)
						{
							switch (ShaderPlatformIndex)
							{
								case 0: ShaderPlatformToMerge = SP_PCD3D_SM2; break;
								case 1: ShaderPlatformToMerge = SP_PCD3D_SM3; break;
								case 2: ShaderPlatformToMerge = SP_PCD3D_SM4; break;
							}
						}

						// if the file matches the name of a shadercache, then merge it as a shader
						if (Filename == FFilename(GetReferenceShaderCacheFilename(ShaderPlatformToMerge)).GetCleanFilename())
						{
							bIsFileShaderCache = TRUE;
							// break out so that the ShaderPlatformToMerge is correct
							break;
						}
					}
					else
					{
						// if the file matches the name of a shadercache, then merge it as a shader
						if (Filename == FFilename(GetLocalShaderCacheFilename(ShaderPlatformToMerge)).GetCleanFilename())
						{
							bIsFileShaderCache = TRUE;
							// break out so that the ShaderPlatformToMerge is correct
							break;
						}
					}
				}

				if (bIsFileShaderCache)
				{
					// create a unique package to not stomp on another one
					UPackage* UniquePackage = UObject::CreatePackage(NULL, *FString::Printf(TEXT("ShaderCache_%d_%d"), JobIndex, FileIndex));

					// get the local shader cache to merge into
					UShaderCache* LocalShaderCache = GetLocalShaderCache(ShaderPlatformToMerge);
					check(LocalShaderCache);

					// load the persistent data
					UPackage* Package = UObject::LoadPackage(UniquePackage, *FilePath, LOAD_NoWarn);

					// merge this into the global one
					if (Package)
					{
						warnf(NAME_Log, TEXT("Merging %s into shader cache %s"), *FilePath, *LocalShaderCache->GetFullName());
						UShaderCache* JobShaderCache = FindObject<UShaderCache>(Package, TEXT("CacheObject"));
						check(JobShaderCache);

						LocalShaderCache->Merge(JobShaderCache);
					}

					// mark that we handled this job file, and don't need to copy it
					bWasFileHandled = TRUE;
				}
			}
		
			// if the file wasn't previously handled, then copy it into the final Cooked directory
			if (!bWasFileHandled)
			{
				// calculate the destination location, keeping the path intact
				FFilename DestFilename = FilePath.Replace(*(appGameDir() + FString(TEXT("Jobs-")) + GetPlatformString() * JobDirectories(JobIndex) * GetCookedDirectory()), *(appGameDir() + GetCookedDirectory()));
				warnf(NAME_Log, TEXT("Copying %s to %s..."), *FilePath, *DestFilename);
				if (GFileManager->Move(*DestFilename, *FilePath, FALSE) == 0)
				{
					warnf(NAME_Warning, TEXT("   Failed!"), *FilePath, *DestFilename);
				}
			}
		}

		// close down packages
		UObject::CollectGarbage(RF_Native);
	}

	// save merged caches
	PersistentCookerData->SaveToDisk();
	GuidCache->SaveToDisk(ShouldByteSwapData());
	SaveLocalShaderCaches();
}




IMPLEMENT_CLASS(UCookPackagesCommandlet)

/*-----------------------------------------------------------------------------
	UPersistentCookerData implementation.
-----------------------------------------------------------------------------*/

/**
 * Create an instance of this class given a filename. First try to load from disk and if not found
 * will construct object and store the filename for later use during saving.
 *
 * @param	Filename	Filename to use for serialization
 * @param	bCreateIfNotFoundOnDisk		If FALSE, don't create if couldn't be found; return NULL.
 * @return	instance of the container associated with the filename
 */
UPersistentCookerData* UPersistentCookerData::CreateInstance( const TCHAR* Filename, UBOOL bCreateIfNotFoundOnDisk )
{
	UPersistentCookerData* Instance = NULL;

	// Find it on disk first.
	if( !Instance )
	{
		// Try to load the package.
		static INT BulkDataIndex = 0;
		FString Temp = FString::Printf(TEXT("BulkData_%d"), BulkDataIndex++);
		UPackage* BulkDataPackage = CreatePackage(NULL, *Temp);
		UPackage* Package = LoadPackage( BulkDataPackage, Filename, LOAD_NoWarn );

		// Find in memory if package loaded successfully.
		if( Package )
		{
			Instance = FindObject<UPersistentCookerData>( Package, TEXT("PersistentCookerData") );
		}
	}

	// If not found, create an instance.
	if ( !Instance && bCreateIfNotFoundOnDisk )
	{
		UPackage* Package = UObject::CreatePackage( NULL, NULL );
		Instance = ConstructObject<UPersistentCookerData>( 
							UPersistentCookerData::StaticClass(),
							Package,
							TEXT("PersistentCookerData")
							);
		// Mark package as cooked as it is going to be loaded at run-time and has xxx extension.
		Package->PackageFlags |= PKG_Cooked;
		check( Instance );
	}


	// Keep the filename around for serialization and add to root to prevent garbage collection.
	if ( Instance )
	{
		Instance->Filename = Filename;
		Instance->AddToRoot();
	}

	return Instance;
}

/**
 * Saves the data to disk.
 */
void UPersistentCookerData::SaveToDisk()
{
	// Save package to disk using filename that was passed to CreateInstance.
	UObject::SavePackage( GetOutermost(), this, 0, *Filename );
}

/**
 * Serialize function.
 *
 * @param	Ar	Archive to serialize with.
 */
void UPersistentCookerData::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );
	Ar << CookedBulkDataInfoMap;
	Ar << FilenameToTimeMap;
	Ar << TextureFileCacheWaste;
	Ar << LastNonSeekfreeCookTime;
	Ar << FilenameToCookedVersion;

	if( Ar.Ver() >= VER_ADDED_TEXTURE_FILECACHE_GUIDS )
	{
		Ar << CookedTextureFileCacheInfoMap;
	}
}

/**
 * Stores the bulk data info after creating a unique name out of the object and name of bulk data
 *
 * @param	Object					Object used to create unique name
 * @param	BulkDataName			Unique name of bulk data within object, e.g. MipLevel_3
 * @param	CookedBulkDataInfo		Data to store
 */
void UPersistentCookerData::SetBulkDataInfo( UObject* Object, const TCHAR* BulkDataName, const FCookedBulkDataInfo& CookedBulkDataInfo )
{
	FString UniqueName = Object->GetPathName() + TEXT(".") + BulkDataName;
	CookedBulkDataInfoMap.Set( *UniqueName, CookedBulkDataInfo );
}

/**
 * Retrieves previously stored bulk data info data for object/ bulk data name combination.
 *
 * @param	Object					Object used to create unique name
 * @param	BulkDataName			Unique name of bulk data within object, e.g. MipLevel_3
 * @return	cooked bulk data info if found, NULL otherwise
 */
FCookedBulkDataInfo* UPersistentCookerData::GetBulkDataInfo( UObject* Object, const TCHAR* BulkDataName )
{
	FString UniqueName = Object->GetPathName() + TEXT(".") + BulkDataName;
	FCookedBulkDataInfo* CookedBulkDataInfo = CookedBulkDataInfoMap.Find( *UniqueName );
	return CookedBulkDataInfo;
}

/**
 * Dumps all bulk data info data to the log.
 */
IMPLEMENT_COMPARE_CONSTREF( FString, UnContentCookersStringSort, { return appStricmp(*A,*B); } )

void UPersistentCookerData::DumpBulkDataInfos()
{
	warnf(TEXT("BulkDataInfo:"));
	TMap<FString,FCookedBulkDataInfo> SortedCookedBulkDataInfoMap = CookedBulkDataInfoMap;
	SortedCookedBulkDataInfoMap.KeySort<COMPARE_CONSTREF_CLASS(FString,UnContentCookersStringSort)>();
	for( TMap<FString,FCookedBulkDataInfo>::TIterator It(SortedCookedBulkDataInfoMap); It; ++It )
	{
		const FString&				UniqueName	= It.Key();
		const FCookedBulkDataInfo&	Info		= It.Value();

		warnf( TEXT("Offset in File: % 10i  Size on Disk: % 10i ElementCount: % 4i  Flags: 0x%08x  TFC: %16s - %s "), 
			Info.SavedBulkDataOffsetInFile,
			Info.SavedBulkDataSizeOnDisk,
			Info.SavedBulkDataFlags, 
			Info.SavedElementCount,
			*Info.TextureFileCacheName.GetNameString(),
			*UniqueName
			);
	}

	warnf(TEXT("\n\nFiletime Info:"));
	for( TMap<FString,DOUBLE>::TIterator It(FilenameToTimeMap); It; ++It )
	{
		warnf(TEXT("%s = %f"), *It.Key(), It.Value());
	}
}

/**
 * Gathers bulk data infos of objects in passed in Outer.
 *
 * @param	Outer	Outer to use for object traversal when looking for bulk data
 */
void UPersistentCookerData::GatherCookedBulkDataInfos( UObject* Outer )
{
	// Iterate over all objects in passed in outer and store the bulk data info for types supported.
	for( TObjectIterator<UObject> It; It; ++It )
	{
		UObject* Object = *It;
		// Make sure we're in the same package.
		if( Object->IsIn( Outer ) )
		{
			// Texture handling.
			UTexture2D* Texture2D = Cast<UTexture2D>( Object );		
			if( Texture2D )
			{
				check( Texture2D->HasAllFlags( RF_Cooked ) );
				for( INT MipLevel=0; MipLevel<Texture2D->Mips.Num(); MipLevel++ )
				{
					FTexture2DMipMap& Mip = Texture2D->Mips(MipLevel);
					// Only store if we've actually saved the bulk data in the archive.			
					if( !Mip.Data.IsStoredInSeparateFile() )
					{
						FCookedBulkDataInfo Info;
						Info.SavedBulkDataFlags			= Mip.Data.GetSavedBulkDataFlags();
						Info.SavedElementCount			= Mip.Data.GetSavedElementCount();
						Info.SavedBulkDataOffsetInFile	= Mip.Data.GetSavedBulkDataOffsetInFile();
						Info.SavedBulkDataSizeOnDisk	= Mip.Data.GetSavedBulkDataSizeOnDisk();
						Info.TextureFileCacheName		= NAME_None;
						SetBulkDataInfo( Texture2D, *FString::Printf(TEXT("MipLevel_%i"),MipLevel), Info );
					}
				}
			}
		}
	}
}

/**
* Stores texture file cache entry info for the given object
*
* @param	Object					Object used to create unique name
* @param	CookedBulkDataInfo		Data to store
*/
void UPersistentCookerData::SetTextureFileCacheEntryInfo( 
	UObject* Object, 
	const FCookedTextureFileCacheInfo& CookedTextureFileCacheInfo )
{
	CookedTextureFileCacheInfoMap.Set( Object->GetPathName(), CookedTextureFileCacheInfo );
}

/**
* Retrieves texture file cache entry info data for given object
*
* @param	Object					Object used to create unique name
* @return	texture file cache info entry if found, NULL otherwise
*/
FCookedTextureFileCacheInfo* UPersistentCookerData::GetTextureFileCacheEntryInfo( UObject* Object )
{
	return CookedTextureFileCacheInfoMap.Find( Object->GetPathName() );
}

/**
* Updates texture file cache entry infos by saving TFC file timestamps for each entry
*
* @param	TextureCacheNameToFilenameMap	Map from texture file cache name to its archive filename
*/
void UPersistentCookerData::UpdateCookedTextureFileCacheEntryInfos( const TMap<FName,FString> TextureCacheNameToFilenameMap )
{

	for( TMap<FString,FCookedTextureFileCacheInfo>::TIterator TextureCacheInfoIt(CookedTextureFileCacheInfoMap); TextureCacheInfoIt; ++TextureCacheInfoIt )
	{
		FCookedTextureFileCacheInfo& TextureCacheInfoEntry = TextureCacheInfoIt.Value();
		const FString* TextureCacheArchiveFilename = TextureCacheNameToFilenameMap.Find(TextureCacheInfoEntry.TextureFileCacheName);
		if( TextureCacheArchiveFilename )
		{
			TextureCacheInfoEntry.LastSaved = GFileManager->GetFileTimestamp( **TextureCacheArchiveFilename );
		}
	}
}

/**
 * Retrieves the file time if the file exists, otherwise looks at the cached data.
 * This is used to retrieve file time of files we don't save out like empty destination files.
 * 
 * @param	Filename	Filename to look file age up
 * @return	Timestamp of file in seconds
 */
DOUBLE UPersistentCookerData::GetFileTime( const TCHAR* InFilename )
{
	FFilename Filename = FFilename(InFilename).GetBaseFilename();
	// Look up filename in cache.
	DOUBLE* FileTimePtr = FilenameToTimeMap.Find( *Filename );
	// If found, return cached file age.
	if( FileTimePtr )
	{
		return *FileTimePtr;
	}
	// If it wasn't found, fall back to using filemanager.
	else
	{
		return GFileManager->GetFileTimestamp( *Filename );
	}
}

/**
 * Sets the time of the passed in file.
 *
 * @param	Filename		Filename to set file age
 * @param	FileTime		Time to set
 */
void UPersistentCookerData::SetFileTime( const TCHAR* InFilename, DOUBLE FileTime )
{
	FFilename Filename = FFilename(InFilename).GetBaseFilename();
	FilenameToTimeMap.Set( *Filename, FileTime );
}


/**
 * Retrieves the cooked version of a previously cooked file.
 * This is used to retrieve the version so we don't have to open the package to get it
 * 
 * @param	Filename	Filename to look version up
 * @return	Cooked version of the file, or 0 if it doesn't exist
 */
INT UPersistentCookerData::GetFileCookedVersion( const TCHAR* Filename )
{
	// look up the version
	INT* Version = FilenameToCookedVersion.Find(Filename);

	// return the version, or 0 if the file didn't exist
	if (Version == NULL)
	{
		return 0;
	}

	return *Version;
}

/**
 * Sets the version of the passed in file.
 *
 * @param	Filename		Filename to set version
 * @param	CookedVersion	Version to set
 */
void UPersistentCookerData::SetFileCookedVersion( const TCHAR* Filename, INT CookedVersion )
{
	FilenameToCookedVersion.Set( Filename, CookedVersion );
}

/**
 * Sets the last time that some non-seekfree packages were cooked. Used so that
 * a map will cooked at next run, but without any non-seekfree packages cooked,
 * will still be cooked.
 */
void UPersistentCookerData::SetLastNonSeekfreeCookTime()
{
	// write a dummy file
	const TCHAR* TimestampFile = TEXT("TimestampCheckFile.chk");
	FArchive* Ar = GFileManager->CreateFileWriter(TimestampFile);
	delete Ar;

	// get the timestamp (ie NOW)
	LastNonSeekfreeCookTime = GFileManager->GetFileTimestamp(TimestampFile);

	// delete it
	GFileManager->Delete(TimestampFile);
}

/**
 * Merge another persistent cooker data object into this one
 *
 * @param Other Other persistent cooker data object to merge in
 * @param Commandlet Commandlet object that is used to get archive for the base texture file cache
 * @param OtherDirectory Directory used to find any TextureFileCache's used by the Other cooker data
 */
void UPersistentCookerData::Merge(UPersistentCookerData* Other, UCookPackagesCommandlet* Commandlet, const TCHAR* OtherDirectory)
{
	// go over other bulk data and merge into this
	for (TMap<FString, FCookedBulkDataInfo>::TIterator It(Other->CookedBulkDataInfoMap); It; ++It)
	{
		// don't merge in existing ones
		// @todo: Check to make sure they are the same?!? What to do if they differ?!??!
		FCookedBulkDataInfo* Existing = CookedBulkDataInfoMap.Find(It.Key());

		if (!Existing)
		{
			// merge over from a texture file cache if one is used
			FCookedBulkDataInfo& New = It.Value();
			if (New.TextureFileCacheName != NAME_None)
			{
				FArchive* BaseTextureFileCache = Commandlet->GetTextureCacheArchive(New.TextureFileCacheName);
				FArchive* OtherTextureFileCache = GFileManager->CreateFileReader(*(FString(OtherDirectory) * New.TextureFileCacheName.ToString() + TEXT(".") + GSys->TextureFileCacheExtension));

				// we can get the offset into the other TFC from the info
				OtherTextureFileCache->Seek(New.SavedBulkDataOffsetInFile);

				// read it in, aligned to ECC_BLOCK_SIZE
				INT SizeToRead = Align(New.SavedBulkDataSizeOnDisk, Commandlet->TextureFileCacheBulkDataAlignment);
				TArray<BYTE> MipData;
				MipData.Add(SizeToRead);
				OtherTextureFileCache->Serialize(MipData.GetData(), SizeToRead);

				// update the info
				New.SavedBulkDataOffsetInFile = BaseTextureFileCache->Tell();
				check((New.SavedBulkDataOffsetInFile % Commandlet->TextureFileCacheBulkDataAlignment) == 0);

				// the base TFC is ready to be written to
				BaseTextureFileCache->Serialize(MipData.GetData(), SizeToRead);

				// @todo: CACHE THIS!!
				delete OtherTextureFileCache;
			}

			// set the info into the existing map
			CookedBulkDataInfoMap.Set(It.Key(), New);
		}
	}
	
	// go over other filetimes and merge into this
	for (TMap<FString, DOUBLE>::TIterator It(Other->FilenameToTimeMap); It; ++It)
	{
		// don't merge in existing ones
		// @todo: Check to make sure they are the same?!? What to do if they differ?!??!
		DOUBLE* Existing = FilenameToTimeMap.Find(It.Key());

		if (!Existing)
		{
			FilenameToTimeMap.Set(It.Key(), It.Value());
		}
		else if (*Existing != It.Value())
		{
			warnf(NAME_Warning, TEXT("Mismatched file time for %s, skipping"), *It.Key());
		}
	}

	// go over other filetimes and merge into this
	for (TMap<FString, INT>::TIterator It(Other->FilenameToCookedVersion); It; ++It)
	{
		// don't merge in existing ones
		INT* Existing = FilenameToCookedVersion.Find(It.Key());

		if (!Existing)
		{
			FilenameToCookedVersion.Set(It.Key(), It.Value());
		}
		else if (*Existing != It.Value())
		{
			warnf(NAME_Warning, TEXT("Mismatched cooked version for %s, skipping"), *It.Key());
		}
	}

	TextureFileCacheWaste += Other->TextureFileCacheWaste;

	// @todo: what do we do about the last non-seekfree cook time?
	LastNonSeekfreeCookTime = Max(LastNonSeekfreeCookTime, Other->LastNonSeekfreeCookTime);
}

IMPLEMENT_CLASS(UPersistentCookerData);

/**
 *	Create an instance of the persistent cooker data given a filename. 
 *	First try to load from disk and if not found will construct object and store the 
 *	filename for later use during saving.
 *
 *	The cooker will call this first, and if it returns NULL, it will use the standard
 *		UPersistentCookerData::CreateInstance function. 
 *	(They are static hence the need for this)
 *
 * @param	Filename					Filename to use for serialization
 * @param	bCreateIfNotFoundOnDisk		If FALSE, don't create if couldn't be found; return NULL.
 * @return								instance of the container associated with the filename
 */
UPersistentCookerData* FGameCookerHelper::CreateInstance( const TCHAR* Filename, UBOOL bCreateIfNotFoundOnDisk )
{
	return UPersistentCookerData::CreateInstance( Filename, bCreateIfNotFoundOnDisk );
}
