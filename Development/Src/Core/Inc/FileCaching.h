/*=============================================================================
    FileCaching.h: Harddisk file caching.
    Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __FILECACHING_H__
#define __FILECACHING_H__


/**
 *	Platform-independent manager for the HDD side-by-side caching system.
 */
class FHDDCacheManager
{
public:
	/** Supported cache mechanisms */
	enum EFileCacheType
	{
		FCT_NotCached,
		FCT_MappedCache,
		FCT_OneDirectionalCache,
	};

	/** Default constructor */
	FHDDCacheManager();

	/** Virtual destructor */
	virtual ~FHDDCacheManager();

	/** Initializer function, to be called right after construction. */
	virtual void Initialize();

	/**
	 *	Returns a HDD cache object for the specified file.
	 *	@param	Filename		Unreal filename for the source file
	 *	@param	OverrideMappedChunkSize	[Optional] Size of each cached chunk, in bytes (only used for Mapped caches)
	 *	@return					New cache object, representing both the source file and the cached version of file
	 */
	virtual class FSideBySideCache*	GetCache( const TCHAR* Filename, INT OverrideMappedChunkSize = DVD_ECC_BLOCK_SIZE );

	/**
	 *	Programmatically adds cache preferences for all files matching the specified pattern.
	 *	@param	FilenamePattern		Filename pattern for match files to the specified caching mechanism
	 *	@param	CacheType			Cache mechanism to use
	 **/
	void AddCachePreferences( const TCHAR* FilenamePattern, EFileCacheType CacheType );

	/**
	 *	Returns the filepath prefix to access the HDD cache (e.g. "cache:"), not containing file path separator.
	 *	@return						Filepath prefix to access the HDD cache
	 */
	const FString&	GetFileCacheRoot( ) const;

	/**
	 * Check if the HD file exists and is as new as the source file. If not, it will create the file time
	 * side file.
	 *
	 * @param Filename				Source file
	 * @param bIsHDDFileGood		[out] Is hard drive file pre-existant and usable
	 * @param bIsHDDFileUpToDate	[out] Is hard drive file up to date
	 * @param bIsTimestampFileGood	[out] Is time stamp file now-existant and usable
	 * @return						Total size of the file in bytes, or -1 on error
	 */
	virtual INT CheckFiles(const TCHAR* Filename, UBOOL& bIsHDDFileGood, UBOOL& bIsHDDFileUpToDate, UBOOL& bIsTimestampFileValid);

	/**
	 * Reserves HDD cache space for the specified file.
	 *
	 * @param FileHandle			File to reserve space for
	 * @param NumBytesRequired		Amount of cache space required for the file, in total
	 * @return						TRUE if the HDD cache has enough space
	 */
	virtual UBOOL ReserveCacheSpace( FFileHandle FileHandle, INT NumBytesRequired ) = 0;

	/**
	 * Returns the full path to the cached version of the file
	 *
	 * @param Filename				Source file
	 * @return						Full path of the cached file
	 */
	virtual FString GetCachedFilepath(const TCHAR* Filename);

protected:

	/**
	 *	Determines whether the file path refers to a partition or folder that is allowed to be cached.
	 *	@param FilePath	- Path to the file that we're considering for caching.
	 *	@return			- TRUE if the file is contained in a path that is allowed to be cached
	 */
	virtual UBOOL ShouldCachePath( const TCHAR* FilePath );

	/**
	 *	Determines what cache mechanism to use for the specified file.
	 *	@param	Filename		Unreal filename for the source file
	 *	@return					Cache mechanism to use for the file
	 **/
	virtual EFileCacheType GetCacheType(const TCHAR* Filename );

	/**
	 *	Creates a platform-specific mapped cache object.
	 *	@param	Filename		Unreal filename for the source file
	 *	@param	OverrideMappedChunkSize	[Optional] Size of each cached chunk, in bytes
	 *	@return					New mapped cache object, representing both the source file and the cached version of file
	 **/
	virtual class FMappedCache* CreateMappedCache( const TCHAR* Filename, INT OverrideMappedChunkSize = DVD_ECC_BLOCK_SIZE );

	/**
	 *	Creates a platform-specific one-directional cache object.
	 *	@param	InFilename		Unreal filename for the source file
	 *	@param	bShouldCache	Whether we should actually be caching or not
	 *	@return					New one-directional cache object, representing both the source file and the cached version of file
	 **/
	virtual class FOneDirectionCache* CreateOneDirectionalCache( const TCHAR* Filename, UBOOL bShouldCache );

	/**
	 *	Make sure that all cache files has matching files and delete invalid files.
	 *	@param	bClearCache		Whether to clear the whole HDD cache
	 */
	virtual void ValidateCache( UBOOL bClearCache );

	/**
	 *	Dump all cache info to the log.
	 */
	virtual void DumpCacheInfo( );

	/**
	 *	Returns the platform-specific maximum length of a filename with full path.
	 *	@return					Maximum length of a full file path
	 **/
	virtual INT GetMaxFilePathLength() = 0;

	/**
	 *	Initializes the preference settings from the .ini file.
	 **/
	void InitPreferences();


	/** Helper struct that represents a wildcard pattern for filename matching */
	struct FWildCardPattern
	{
		/**
		 *	Initializer constructor.
		 *	@param InPattern			Wildcard pattern for filename matching
		 *	@param InWildCardPosition	Index to the position of the wildcard character ('*')
		 */
		FWildCardPattern( const FString& InPattern, INT InWildCardPosition )
		:	Pattern(InPattern)
		,	WildCardPosition(InWildCardPosition)
		{
		}
		/** Comparison operator to be used by TMap */
		UBOOL operator==( const FWildCardPattern& Other ) const
		{
			return appStricmp(*Pattern, *Other.Pattern) == 0;
		}
		/** Filename pattern (e.g. "GUD_*.xxx") */
		FString	Pattern;
		/** Index to the position of the wildcard character ('*') */
		INT		WildCardPosition;
	};

	/** Critical section for hard drive caching access. Public for other file routines to be able to block caching. */
	FCriticalSection FileCacheLock;

	/** Whether the preference settings have been initialized or not. */
	UBOOL bPreferencesInitialized;

	/** Contains all wildcard patterns for filename matching and maps them to a certain cache mechanism */
	TMap<FWildCardPattern, EFileCacheType> WildcardPreferences;

	/** Contains all explicit filenames and maps them to a certain cache mechanism */
	TMap<FString, EFileCacheType> Preferences;

	/** Filepath prefix to access the HDD cache (e.g. "cache:"), not containing file path separator */
	FString		FileCacheRoot;

	/** Whether HDD caching is enabled at all. */
	UBOOL		bEnableCaching;

	friend DWORD GetTypeHash( const FHDDCacheManager::FWildCardPattern& Pattern );
};

/** Helper function for TMap. */
FORCEINLINE DWORD GetTypeHash( const FHDDCacheManager::FWildCardPattern& Pattern )
{
	return appStrihash(*Pattern.Pattern);
}


/**
 * Helper struct to manage caching a file to the HD while reading from it from DVD
 */
class FSideBySideCache
{
public:
	/**
	 * Constructor
	 *
	 * @param InFilename Unreal filename for the file this represents
	 */
	FSideBySideCache(const TCHAR* InFilename, UBOOL bShouldCache);

	/**
	 * Virtual destructor
	 */
	virtual ~FSideBySideCache();

	/**
	 * Initialize the cache.
	 * 
	 * @return TRUE if successful
	 */
	virtual UBOOL Initialize() = 0;

	/**
	 * Seek the source and/or cached file
	 *
	 * @param Pos New position the source file will be reading from
	 *
	 * @return TRUE if successful
	 */
	virtual UBOOL Seek(INT Pos) = 0;

	/** 
	 * Serialize from source and/or HD
	 *
	 * @param Buffer The buffer to read into
	 * @param Size Amount to read
	 *
	 * @return TRUE if successful
	 */
	virtual UBOOL Serialize(void* Buffer, INT Size) = 0;

	/**
	 * Fully caches a file to HD
	 */
	virtual UBOOL FullyCache() = 0;

	/**
	 * Reserves HDD cache space to fit the entire file
	 *
	 * @return TRUE if the HD had enough space
	 */
	virtual UBOOL ReserveCacheSpace() = 0;

	/**
	 * @return the Unreal filename for this cache 
	 */
	FORCEINLINE const FFilename& GetFilename()
	{
		return Filename;
	}

	/**
	 * @return Size of source file, or -1 for error
	 */
	FORCEINLINE INT GetSourceFileSize()
	{
		return SourceFileSize;
	}

	/**
	 * @return TRUE if the file is able to be cached to the HD, or is reading from HD
	 */
	FORCEINLINE INT UsesHardDrive()
	{
		return HardDriveFile.IsValid();
	}

	/**
	 * @return TRUE if the file was found in the source drive (BD or host)
	 */
	FORCEINLINE INT UsesSource()
	{
		return SourceFile.IsValid();
	}

	FORCEINLINE QWORD GetUniqueID()
	{
		return UniqueID;
	}

protected:

	/**
	 * Open the source and HD files for caching. If caching is disabled, only open the source file.
	 *
	 * @param CacheFilename		Optional filename for the cache file (NULL to use the same as the source).
	 * @return					TRUE if successful
	 */
	UBOOL OpenFilesForCaching( const TCHAR* CacheFilename=NULL );


	/** Cached Unreal filename */
	FFilename	Filename;

	/** Handle to the file for writing */
	FFileHandle	HardDriveFile;

	/** BSource archive that this file is caching - needed to read from to fill in gaps */
	FFileHandle	SourceFile;

	/** Size of the original file - used to know when we have finished caching */
	INT			SourceFileSize;

	/** Unique ID for the cache object */
	QWORD		UniqueID;

	/** Whether caching should be enabled for this file */
	UBOOL		bEnableCaching;

	/** Next unique ID to assign */
	static QWORD GSideBySideNextID;

	/** Critical section for hard drive caching access so that two threads don't fight over the BD */
	static FCriticalSection SideBySideCacheSection;
};


/**
 * Cache type that caches only increasingly. This means that if the
 * file is seeked around, it will read intermediate data in a loop caching
 * whatever is skipped over
 */
class FOneDirectionCache : public FSideBySideCache
{
public:
	/**
	 * Constructor
	 *
	 * @param InFilename	Unreal filename for the file this represents
	 * @param bShouldCache	Whether we should actually be caching or not
	 */
	FOneDirectionCache(const TCHAR* InFilename, UBOOL bShouldCache);

	/**
	 * Destructor
	 */
	virtual ~FOneDirectionCache();

	/**
	 * Initialize the cache.
	 *
	 * @return TRUE if successful
	 */
	virtual UBOOL Initialize();

	/**
	 * Handle the source archive moving position
	 *
	 * @param Pos New position the source file will be reading from
	 *
	 * @return TRUE if successful
	 */
	virtual UBOOL Seek(INT Pos);

	/** 
	 * Serialize from source and/or HD
	 *
	 * @param Buffer The buffer to read into
	 * @param Size Amount to read
	 *
	 * @return TRUE if successful
	 */
	virtual UBOOL Serialize(void* Buffer, INT Size);

	/**
	 * Fully caches a file to HD
	 */
	virtual UBOOL FullyCache();

	/**
	 * Reserves HDD cache space to fit the entire file
	 *
	 * @return TRUE if the HD had enough space
	 */
	virtual UBOOL ReserveCacheSpace();

protected:

	/**
	 * Closes source file, syncs up dates, etc
	 *
	 * @return TRUE if successful
	 */
	UBOOL FinalizeCache();

	/** Amount written so far in this cached file (since this is unidirectional, this tells all) */
	INT AmountWritten;

	/** Position in the source file */
	INT FilePos;
};

/**
 * Cache that caches on a per chunk basis, and keeps a mapping of what
 * chunks have been cached to HD
 */
class FMappedCache : public FSideBySideCache
{
	friend class FHDDCacheManager;

public:
	/**
	 * Constructor
	 *
	 * @param InFilename		Unreal filename for the file this represents
	 * @param InChunkSize		Size of each mapped chunk
	 * @param bReserveSpace		Whether HDD cache space should be reserved for this file
	 */
	FMappedCache(const TCHAR* InFilename, INT InChunkSize, UBOOL bReserveSpace );

	/**
	 * Destructor
	 */
	virtual ~FMappedCache();

	/**
	 * Initialize the cache.
	 * 
	 * @return TRUE if successful
	 */
	virtual UBOOL Initialize();

	/**
	 * Handle the source archive moving position
	 *
	 * @param Pos New position the source file will be reading from
	 *
	 * @return TRUE if successful
	 */
	virtual UBOOL Seek(INT Pos);

	/** 
	 * Serialize from source and/or HD
	 *
	 * @param Buffer The buffer to read into
	 * @param Size Amount to read
	 *
	 * @return TRUE if successful
	 */
	virtual UBOOL Serialize(void* Buffer, INT Size);

	/**
	 * Fully caches a file to HD
	 */
	virtual UBOOL FullyCache();

	/**
	 * Reserves HDD cache space to fit the entire file
	 *
	 * @return TRUE if the HD had enough space
	 */
	virtual UBOOL ReserveCacheSpace();

protected:

	/**
	 * Read the metadata file from HD
	 *
	 * @return TRUE if successful
	 */
	UBOOL LoadMetadata();

	/**
	 * Read the metadata file from HD
	 *
	 * @return TRUE if successful
	 */
	UBOOL SaveMetadata();

	/** Size of each mapped chunk */
	INT ChunkSize;

	/** Number of chunks needed to map the source file */
	INT NumChunks;

	/** Number of chunks that have been cached to HD */
	INT NumCachedChunks;

	/** Tracks which chunks are mapped to the HD file */
	TArray<WORD>* MappedChunks;

	/** Metadata file handle */
	FFileHandle MetadataFile;

	/** Is the HD file fully mapped? (installed, etc) */
	UBOOL bIsFileFullyMapped;

	/** Curent "virtual" offset into the file (seek pos) */
	INT FilePos;

	/** Current actual position in HD file */
	INT HardDriveFilePos;

	/** Current actual position in HD file */
	INT SourceFilePos;

	/** Timestamp when the metadata was last saved */
	DOUBLE LastTimeMetadataWasSaved;

	/** Whether HDD cache space should be reserved for this file */
	UBOOL bReserveSpace;
};

/** Global HDD Cache Manager object */
extern FHDDCacheManager* GHDDCacheManager;


#endif
