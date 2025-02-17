/*=============================================================================
	UnContentCookers.h: Content cooker helper objects.

	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*-----------------------------------------------------------------------------
	Cooking.
-----------------------------------------------------------------------------*/

/**
 * Helper structure containing information needed for separating bulk data from it's archive
 */
struct FCookedBulkDataInfo
{
	/** From last saving or StoreInSeparateFile call: Serialized flags for bulk data									*/
	DWORD	SavedBulkDataFlags;
	/** From last saving or StoreInSeparateFile call: Number of elements in bulk data array								*/
	INT		SavedElementCount;
	/** From last saving or StoreInSeparateFile call: Offset of bulk data into file or INDEX_NONE if no association		*/
	INT		SavedBulkDataOffsetInFile;
	/** From last saving or StoreInSeparateFile call: Size of bulk data on disk or INDEX_NONE if no association			*/
	INT		SavedBulkDataSizeOnDisk;
	/** Name of texture file cache if saved in one.																		*/
	FName	TextureFileCacheName;

	/**
	 * Serialize operator
	 *
	 * @param	Ar		Archive to serialize with
	 * @param	Info	Structure to serialize
	 * @return	Ar after having been used for serialization
	 */
	friend FArchive& operator<<( FArchive& Ar, FCookedBulkDataInfo& Info )
    {
        Ar << Info.SavedBulkDataFlags;
		Ar << Info.SavedElementCount;
		Ar << Info.SavedBulkDataOffsetInFile;
		Ar << Info.SavedBulkDataSizeOnDisk;
		Ar << Info.TextureFileCacheName;
		return Ar;
    }
};

/**
* Helper structure containing information needed to track updated texture file cache entries 
*/
struct FCookedTextureFileCacheInfo
{
	/** 
	* Constructor 
	*/
	FCookedTextureFileCacheInfo() 
		:	TextureFileCacheGuid(0,0,0,0)
		,	TextureFileCacheName(NAME_None)
		,	LastSaved(0)
	{
	}

	/**
	* Serialize operator
	*
	* @param	Ar		Archive to serialize with
	* @param	Info	Structure to serialize
	* @return	Ar after having been used for serialization
	*/
	friend FArchive& operator<<( FArchive& Ar, FCookedTextureFileCacheInfo& Info )
	{
		Ar << Info.TextureFileCacheGuid;
		Ar << Info.TextureFileCacheName;
		Ar << Info.LastSaved;
		return Ar;
	}

	/** Unique texture ID to determine if texture file cache bulk data needs an update */
	FGuid TextureFileCacheGuid;
	/** Name of texture file cache archive used */
	FName TextureFileCacheName;
	/** Time the texture entry was last saved into the TFC */
	DOUBLE LastSaved;
};

/**
 *	Structure for tracking TFC entries for post-cook verification
 */
struct FTextureFileCacheEntry
{
	/** TRUE if it is in the CHAR texture file cache... */
	UBOOL bCharTextureFileCache;
	/** The path name of the texture... */
	FString TextureName;
	/** The mip of the texture... */
	INT MipIndex;
	/** The GUID of the texture, the file cache name and the last time it was saved. */
	//FCookedTextureFileCacheInfo TFCInfo;
	/** The offset of the entry */
	INT OffsetInCache;
	/** The size of the entry */
	INT SizeInCache;
	/** The flags of the entry */
	INT FlagsInCache;
	/** The flags of the entry */
	INT ElementCountInCache;

	FTextureFileCacheEntry() :
		  bCharTextureFileCache(FALSE)
		, TextureName(TEXT(""))
		, MipIndex(-1)
		, OffsetInCache(-1)
		, SizeInCache(-1)
		, FlagsInCache(-1)
		, ElementCountInCache(-1)
	{
	}

	FTextureFileCacheEntry(const FTextureFileCacheEntry& Src) :
		  bCharTextureFileCache(Src.bCharTextureFileCache)
		, TextureName(Src.TextureName)
		, MipIndex(Src.MipIndex)
		, OffsetInCache(Src.OffsetInCache)
		, SizeInCache(Src.SizeInCache)
		, FlagsInCache(Src.FlagsInCache)
		, ElementCountInCache(Src.ElementCountInCache)
	{
	}

	static UBOOL EntriesOverlap(FTextureFileCacheEntry& EntryA, FTextureFileCacheEntry& EntryB)
	{
		if (EntryA.bCharTextureFileCache != EntryB.bCharTextureFileCache)
		{
			return FALSE;
		}

		if (EntryA.TextureName == EntryB.TextureName)
		{
			if (EntryA.OffsetInCache == EntryB.OffsetInCache)
			{
				if (EntryA.SizeInCache == EntryB.SizeInCache)
				{
					return FALSE;
				}
			}
		}

		if (((EntryA.OffsetInCache >= EntryB.OffsetInCache) && 
			(EntryA.OffsetInCache < (EntryB.OffsetInCache + EntryB.SizeInCache))) ||
			((EntryB.OffsetInCache >= EntryA.OffsetInCache) &&
			(EntryB.OffsetInCache < (EntryA.OffsetInCache + EntryA.SizeInCache))))
		{
			return TRUE;
		}

		return FALSE;
	}
};

/**
 * Serialized container class used for mapping a unique name to bulk data info.
 */
class UPersistentCookerData : public UObject
{
	DECLARE_CLASS(UPersistentCookerData,UObject,CLASS_Intrinsic,Editor);

	/**
	 * Create an instance of this class given a filename. First try to load from disk and if not found
	 * will construct object and store the filename for later use during saving.
	 *
	 * @param	Filename					Filename to use for serialization
	 * @param	bCreateIfNotFoundOnDisk		If FALSE, don't create if couldn't be found; return NULL.
	 * @return								instance of the container associated with the filename
	 */
	static UPersistentCookerData* CreateInstance( const TCHAR* Filename, UBOOL bCreateIfNotFoundOnDisk );
	
	/**
	 * Saves the data to disk.
	 */
	virtual void SaveToDisk();

	/**
	 * Serialize function.
	 *
	 * @param	Ar	Archive to serialize with.
	 */
	virtual void Serialize(FArchive& Ar);

	/**
	 * Stores the bulk data info after creating a unique name out of the object and name of bulk data
	 *
	 * @param	Object					Object used to create unique name
	 * @param	BulkDataName			Unique name of bulk data within object, e.g. MipLevel_3
	 * @param	CookedBulkDataInfo		Data to store
	 */
	void SetBulkDataInfo( UObject* Object, const TCHAR* BulkDataName, const FCookedBulkDataInfo& CookedBulkDataInfo );
	
	/**
	 * Retrieves previously stored bulk data info data for object/ bulk data array name combination.
	 *
	 * @param	Object					Object used to create unique name
	 * @param	BulkDataName			Unique name of bulk data within object, e.g. MipLevel_3
	 * @return	cooked bulk data info if found, NULL otherwise
	 */
	FCookedBulkDataInfo* GetBulkDataInfo( UObject* Object, const TCHAR* BulkDataName );

	/**
	 * Gathers bulk data infos of objects in passed in Outer.
	 *
	 * @param	Outer	Outer to use for object traversal when looking for bulk data
	 */
	void GatherCookedBulkDataInfos( UObject* Outer );

	/**
	 * Dumps all bulk info data to the log.
	 */
	void DumpBulkDataInfos();

	/**
	* Stores texture file cache entry info for the given object
	*
	* @param	Object					Object used to create unique name
	* @param	CookedBulkDataInfo		Data to store
	*/
	void SetTextureFileCacheEntryInfo( UObject* Object, const FCookedTextureFileCacheInfo& CookedTextureFileCacheInfo );

	/**
	* Retrieves texture file cache entry info data for given object
	*
	* @param	Object					Object used to create unique name
	* @return	texture file cache info entry if found, NULL otherwise
	*/
	FCookedTextureFileCacheInfo* GetTextureFileCacheEntryInfo( UObject* Object );

	/**
	* Updates texture file cache entry infos by saving TFC file timestamps for each entry
	*
	* @param	TextureCacheNameToFilenameMap	Map from texture file cache name to its archive filename
	*/
	void UpdateCookedTextureFileCacheEntryInfos( const TMap<FName,FString> TextureCacheNameToFilenameMap );

	/**
	 * Retrieves the file time if the file exists, otherwise looks at the cached data.
	 * This is used to retrieve file time of files we don't save out like empty destination files.
	 * 
	 * @param	Filename	Filename to look file age up
	 * @return	Timestamp of file in seconds
	 */
	DOUBLE GetFileTime( const TCHAR* Filename );

	/**
	 * Sets the time of the passed in file.
	 *
	 * @param	Filename		Filename to set file age
	 * @param	FileTime	Time to set
	 */
	void SetFileTime( const TCHAR* Filename, DOUBLE FileTime );

	/**
	 * Retrieves the cooked version of a previously cooked file.
	 * This is used to retrieve the version so we don't have to open the package to get it
	 * 
	 * @param	Filename	Filename to look version up
	 * @return	Cooked version of the file, or 0 if it doesn't exist
	 */
	INT GetFileCookedVersion( const TCHAR* Filename );

	/**
	 * Sets the version of the passed in file.
	 *
	 * @param	Filename		Filename to set version
	 * @param	CookedVersion	Version to set
	 */
	void SetFileCookedVersion( const TCHAR* Filename, INT CookedVersion );

	/**
	 * Sets the texture file cache waste in bytes.
	 *
	 * @param	NewWaste	New waste to set.
	 */
	void SetTextureFileCacheWaste( QWORD NewWaste )
	{
		TextureFileCacheWaste = NewWaste;
	}

	/**
	 * Returns current waste of texture file cache.
	 *
	 * @return waste of texture file cache in bytes.
	 */
	QWORD GetTextureFileCacheWaste()
	{
		return TextureFileCacheWaste;
	}

	/** Sets the path this CookerData object will be saved to. */
	void SetFilename(const TCHAR* InFilename)
	{
		Filename = InFilename;
	}

	/**
	 * Sets the last time that some non-seekfree packages were cooked. Used so that
	 * a map will cooked at next run, but without any non-seekfree packages cooked,
	 * will still be cooked.
	 */
	void SetLastNonSeekfreeCookTime();

	/**
	 * Gets the last time that some non-seekfree packages were cooked. Used so that
	 * a map will cooked at next run, but without any non-seekfree packages cooked,
	 * will still be cooked.
	 */
	DOUBLE GetLastNonSeekfreeCookTime()
	{
		return LastNonSeekfreeCookTime;
	}

	/**
	 * Merge another persistent cooker data object into this one
	 *
	 * @param Commendlet Commandlet object that is used to get archive for the base texture file cache
	 * @param OtherDirectory Directory used to find any TextureFileCache's used by the Other cooker data
	 */
	virtual void Merge(UPersistentCookerData* Other, class UCookPackagesCommandlet* Commandlet, const TCHAR* OtherDirectory);

protected:
	/** Map from unique name to bulk data info data */
	TMap<FString,FCookedBulkDataInfo> CookedBulkDataInfoMap;

	/** Map from unique texture name to TFC entry info */
	TMap<FString,FCookedTextureFileCacheInfo> CookedTextureFileCacheInfoMap;

	/** Map from filename to file time. */
	TMap<FString,DOUBLE> FilenameToTimeMap;

	/** Map from filename to cooked package version. */
	TMap<FString,INT> FilenameToCookedVersion;

	/** 
	 * Amount of waste in bytes due to using texture file cache. Can accumulate over time due to the way textures 
	 * are updated in the file cache as we can only replace ones with the same size. An easy fix is to perform a full
	 * recook.
	 */
	QWORD TextureFileCacheWaste;

	/** Filename to use for serialization. */
	FString	Filename;

	/** Timestamp when seekfree cooking was done */
	DOUBLE LastNonSeekfreeCookTime;
};

/**
 * Helper structure encapsulating mapping from src file to cooked data filename.
 */
struct FPackageCookerInfo
{
	/** Src filename.									*/
	FFilename SrcFilename;
	/** Cooked dst filename.							*/
	FFilename DstFilename;
	/** Whether this package should be made seek free.	*/
	UBOOL bShouldBeSeekFree;
	/** Whether this package is a native script file.	*/
	UBOOL bIsNativeScriptFile;
	/** Whether this packages is a startup package.		*/
	UBOOL bIsCombinedStartupPackage;
	/** Whether this packages is standalone seekfree.	*/
	UBOOL bIsStandaloneSeekFreePackage;
	/** Whether or not to just load the package during cooking, don't save it */
	UBOOL bShouldOnlyLoad;

	/** Empty constructor */
	FPackageCookerInfo()
	{
	}

	/** Constructor, initializing member variables with passed in ones */
	FPackageCookerInfo( const TCHAR* InSrcFilename, const TCHAR* InDstFilename, UBOOL InbShouldBeSeekFree, UBOOL InbIsNativeScriptFile, UBOOL InbIsCombinedStartupPackage, UBOOL InbIsStandaloneSeekFreePackage )
	:	SrcFilename( InSrcFilename )
	,	DstFilename( InDstFilename )
	,	bShouldBeSeekFree( InbShouldBeSeekFree )
	,	bIsNativeScriptFile( InbIsNativeScriptFile )
	,	bIsCombinedStartupPackage( InbIsCombinedStartupPackage )
	,	bIsStandaloneSeekFreePackage( InbIsStandaloneSeekFreePackage )
	,	bShouldOnlyLoad( FALSE )
	{}

	/**
	 * Serialize function.
	 *
	 * @param	Ar	Archive to serialize with.
	 */
	virtual void Serialize(FArchive& Ar)
	{
		Ar << SrcFilename;
		Ar << DstFilename;
		Ar << bShouldBeSeekFree;
		Ar << bIsNativeScriptFile;
		Ar << bIsCombinedStartupPackage;
		Ar << bIsStandaloneSeekFreePackage;
		Ar << bShouldOnlyLoad;
	}

	// Serializer.
	friend FArchive& operator<<(FArchive& Ar, FPackageCookerInfo& CookerInfo)
	{
		return Ar 
			<< CookerInfo.SrcFilename
			<< CookerInfo.DstFilename
			<< CookerInfo.bShouldBeSeekFree
			<< CookerInfo.bIsNativeScriptFile
			<< CookerInfo.bIsCombinedStartupPackage
			<< CookerInfo.bIsStandaloneSeekFreePackage
			<< CookerInfo.bShouldOnlyLoad;
	}
};

/**
 *	Helper class to handle game-specific cooking. Each game can 
 *	subclass this to handle special-case cooking needs.
 */
class FGameCookerHelper
{
public:
	/**
	* Initialize the cooker helpr and process any command line params
	*
	*	@param	Commandlet		The cookpackages commandlet being run
	*	@param	Tokens			Command line tokens parsed from app
	*	@param	Switches		Command line switches parsed from app
	*/
	virtual void Init(
		class UCookPackagesCommandlet* Commandlet, 
		const TArray<FString>& Tokens, 
		const TArray<FString>& Switches )
	{}

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
	virtual UPersistentCookerData* CreateInstance( const TCHAR* Filename, UBOOL bCreateIfNotFoundOnDisk );

	/** 
	 *	Generate the package list that is specific for the game being cooked.
	 *
	 *	@param	Commandlet							The cookpackages commandlet being run
	 *	@param	Platform							The platform being cooked for
	 *	@param	ShaderPlatform						The shader platform being cooked for
	 *	@param	NotRequiredFilenamePairs			The package lists being filled in...
	 *	@param	RegularFilenamePairs				""
	 *	@param	MapFilenamePairs					""
	 *	@param	MPMapFilenamePairs					""
	 *	@param	ScriptFilenamePairs					""
	 *	@param	StartupFilenamePairs				""
	 *	@param	StandaloneSeekfreeFilenamePairs		""
	 *	
	 *	@return	UBOOL		TRUE if successful, FALSE is something went wrong.
	 */
	virtual UBOOL GeneratePackageList( 
		class UCookPackagesCommandlet* Commandlet, 
		UE3::EPlatformType Platform,
		EShaderPlatform ShaderPlatform,
		TArray<FPackageCookerInfo>& NotRequiredFilenamePairs,
		TArray<FPackageCookerInfo>& RegularFilenamePairs,
		TArray<FPackageCookerInfo>& MapFilenamePairs,
		TArray<FPackageCookerInfo>& MPMapFilenamePairs,
		TArray<FPackageCookerInfo>& ScriptFilenamePairs,
		TArray<FPackageCookerInfo>& StartupFilenamePairs,
		TArray<FPackageCookerInfo>& StandaloneSeekfreeFilenamePairs)
	{
		return TRUE;
	}

	/**
	 * Cooks passed in object if it hasn't been already.
	 *
	 *	@param	Commandlet					The cookpackages commandlet being run
	 *	@param	Package						Package going to be saved
	 *	@param	Object						Object to cook
	 *	@param	bIsSavedInSeekFreePackage	Whether object is going to be saved into a seekfree package
	 *
	 *	@return	UBOOL						TRUE if the object should continue the 'normal' cooking operations.
	 *										FALSE if the object should not be processed any further.
	 */
	virtual UBOOL CookObject( class UCookPackagesCommandlet* Commandlet, UPackage* Package, 
		UObject* Object, UBOOL bIsSavedInSeekFreePackage )
	{
		return TRUE;
	}

	/** 
	 *	LoadPackageForCookingCallback
	 *	This function will be called in LoadPackageForCooking, allowing the cooker
	 *	helper to handle the package creation as they wish.
	 *
	 *	@param	Commandlet		The cookpackages commandlet being run
	 *	@param	Filename		The name of the package to load.
	 *
	 *	@return	UPackage*		The package generated/loaded
	 *							NULL if the commandlet should load the package normally.
	 */
	virtual UPackage* LoadPackageForCookingCallback(class UCookPackagesCommandlet* Commandlet, const TCHAR* Filename)
	{
		return NULL;
	}

	/** 
	 *	PostLoadPackageForCookingCallback
	 *	This function will be called in LoadPackageForCooking, prior to any
	 *	operations occurring on the contents...
	 *
	 *	@param	Commandlet	The cookpackages commandlet being run
	 *	@param	Package		The package just loaded.
	 *
	 *	@return	UBOOL		TRUE if the package should be processed further.
	 *						FALSE if the cook of this package should be aborted.
	 */
	virtual UBOOL PostLoadPackageForCookingCallback(class UCookPackagesCommandlet* Commandlet, UPackage* InPackage)
	{
		return TRUE;
	}

	/**
	 *	Clean up the kismet for the given level...
	 *	Remove 'danglers' - sequences that don't actually hook up to anything, etc.
	 *
	 *	@param	Commandlet	The cookpackages commandlet being run
	 *	@param	Package		The package being cooked.
	 */
	virtual void CleanupKismet(class UCookPackagesCommandlet* Commandlet, UPackage* InPackage)
	{
	}

	/**
	 *	Return TRUE if the sound cue should be ignored when generating persistent FaceFX list.
	 *
	 *	@param	InSoundCue		The sound cue of interest
	 *
	 *	@return	UBOOL			TRUE if the sound cue should be ignored, FALSE if not
	 */
	virtual UBOOL ShouldSoundCueBeIgnoredForPersistentFaceFX(class UCookPackagesCommandlet* Commandlet, const USoundCue* InSoundCue)
	{
		return FALSE;
	}

	/**
	 *	Dump out stats specific to the game cooker helper.
	 */
	virtual void DumpStats() {};
};

// global objects
extern FGameCookerHelper* GGameCookerHelper;
