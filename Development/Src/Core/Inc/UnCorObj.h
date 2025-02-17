/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

enum ESourceControlState
{
	/** Don't know or don't care. */
	SCC_DontCare		= 0,

	/** File is checked out to current user. */
	SCC_CheckedOut		= 1,

	/** File is not checked out (but IS controlled by the source control system). */
	SCC_ReadOnly		= 2,

	/** File is not at the head revision - must sync the file before editing. */
	SCC_NotCurrent		= 3,

	/** File is new and not in the depot - needs to be added. */
	SCC_NotInDepot		= 4,

	/** File is checked out by another user and cannot be checked out locally. */
	SCC_CheckedOutOther	= 5,

	/** Certain packages are best ignored by the SCC system (MyLevel, Transient, etc). */
	SCC_Ignore			= 6
};

/** interface for objects that want to listen in for the addition/removal of net serializable objects and pacakges */
class FNetObjectNotify
{
public:
	/** destructor */
	virtual ~FNetObjectNotify();
	/** notification when a package is added to the NetPackages list */
	virtual void NotifyNetPackageAdded(UPackage* Package) = 0;
	/** notification when a package is removed from the NetPackages list */
	virtual void NotifyNetPackageRemoved(UPackage* Package) = 0;
	/** notification when an object is removed from a net package's NetObjects list */
	virtual void NotifyNetObjectRemoved(UObject* Object) = 0;
};

/**
 * A package.
 */
class UPackage : public UObject
{
private:
	DECLARE_CLASS(UPackage,UObject,CLASS_Intrinsic,Core)

	/** Used by the editor to determine if a package has been changed.																							*/
	UBOOL	bDirty;
	/** Used by the editor to determine if a package has changed since the last time the user Played-In-Editor */
	UBOOL bDirtyForPIE;
	/** Whether this package has been fully loaded (aka had all it's exports created) at some point.															*/
	UBOOL	bHasBeenFullyLoaded;
	/** Returns whether exports should be found in memory first before trying to load from disk from within CreateExport.										*/
	UBOOL	bShouldFindExportsInMemoryFirst;
	/** Whether the package is bound, set via SetBound.																											*/
	UBOOL	bIsBound;
	/** Indicates which folder to display this package under in the Generic Browser's list of packages. If not specified, package is added to the root level.	*/
	FName	FolderName;
	/** The state of this package in the source control system.																									*/
	ESourceControlState SCCState;
	/** Time in seconds it took to fully load this package. 0 if package is either in process of being loaded or has never been fully loaded.					*/
	FLOAT LoadTime;

	/** GUID of package if it was loaded from disk; used by netcode to make sure packages match between client and server */
	FGuid Guid;
	/** size of the file for this package; if the package was not loaded from a file or was a forced export in another package, this will be zero */
	INT FileSize;
	/** array of net serializable objects currently loaded into this package (top level packages only) */
	TArray<UObject*> NetObjects;
	/** number of objects in the Objects list that currently exist (are loaded) */
	INT CurrentNumNetObjects;
	/** number of objects in the list for each generation of the package (for conforming) */
	TArray<INT> GenerationNetObjectCount;
	/** for packages that were a forced export in another package (seekfree loading), the name of that base package, otherwise NAME_None */
	FName ForcedExportBasePackageName;
	
	/** array of UPackages that currently have NetObjects that are relevant to netplay */
	static TArray<UPackage*> NetPackages;
	/** objects that should be informed of NetPackage/NetObject changes */
	static TArray<FNetObjectNotify*> NetObjectNotifies;

public:
	/** Package flags, serialized.																																*/
	DWORD	PackageFlags;


	/** returns a const reference to NetPackages */
	static FORCEINLINE const TArray<UPackage*>& GetNetPackages()
	{
		return NetPackages;
	}

	// Constructors.
	UPackage();

	/**
	 * Static constructor called once per class during static initialization via IMPLEMENT_CLASS
	 * macro. Used to e.g. emit object reference tokens for realtime garbage collection or expose
	 * properties for native- only classes.
	 */
	void StaticConstructor();

	/** Serializer */
	virtual void Serialize( FArchive& Ar );

	// UPackage interface.
	void* GetExport( const TCHAR* ExportName, UBOOL Checked ) const;

	/**
	 * @return		TRUE if the packge is bound, FALSE otherwise.
	 */
	UBOOL IsBound() const
	{
		return bIsBound;
	}

	/**
	 * Marks/unmarks the package as being bound.
	 */
	void SetBound( UBOOL bInBound )
	{
		bIsBound = bInBound;
	}

	/**
	 * Sets the time it took to load this package.
	 */
	void SetLoadTime( FLOAT InLoadTime )
	{
		LoadTime = InLoadTime;
	}

	/**
	 * Returns the time it took the last time this package was fully loaded, 0 otherwise.
	 *
	 * @return Time it took to load.
	 */
	FLOAT GetLoadTime()
	{
		return LoadTime;
	}

	/**
	 * Get the package's folder name
	 * @return		Folder name
	 */
	FName GetFolderName() const
	{
		return FolderName;
	}

	/**
	 * Set the package's folder name
	 */
	void SetFolderName (FName name)
	{
		FolderName = name;
	}

	/**
	 * Marks/Unmarks the package's bDirty flag
	 */
	void SetDirtyFlag( UBOOL bIsDirty );

	/**
	 * Returns whether the package needs to be saved.
	 *
	 * @return		TRUE if the package is dirty and needs to be saved, false otherwise.
	 */
	UBOOL IsDirty() const
	{
		return bDirty;
	}

	/**
	 * Returns whether the package has been changed since the last time we Played-In-Editor with it
	 *
	 * @return		TRUE if the package is dirty and needs to be saved for PIE, false otherwise.
	 */
	UBOOL IsDirtyForPIE() const
	{
		return bDirtyForPIE;
	}

	/**
	 * Called when the package is saved in a PIE level; clears the package's dirty bit for Play-In-Editor
	 */
	void ClearDirtyForPIEFlag()
	{
		bDirtyForPIE = FALSE;
	}

	/**
	 * Marks this package as being fully loaded.
	 */
	void MarkAsFullyLoaded()
	{
		bHasBeenFullyLoaded = TRUE;
	}

	/**
	 * Returns whether the package is fully loaded.
	 *
	 * @return TRUE if fully loaded or no file associated on disk, FALSE otherwise
	 */
	UBOOL IsFullyLoaded();

	/**
	 * Fully loads this package. Safe to call multiple times and won't clobber already loaded assets.
	 */
	void FullyLoad();

	/**
	 * Sets whether exports should be found in memory first or  not.
	 *
	 * @param bInShouldFindExportsInMemoryFirst	Whether to find in memory first or not
	 */
	void FindExportsInMemoryFirst( UBOOL bInShouldFindExportsInMemoryFirst )
	{
		bShouldFindExportsInMemoryFirst = bInShouldFindExportsInMemoryFirst;
	}

	/**
	 * Returns whether exports should be found in memory first before trying to load from disk
	 * from within CreateExport.
	 *
	 * @return TRUE if exports should be found via FindObject first, FALSE otherwise.
	 */
	UBOOL ShouldFindExportsInMemoryFirst()
	{
		return bShouldFindExportsInMemoryFirst;
	}

	/**
	 * Called to indicate that this package contains a ULevel or UWorld object.
	 */
	void ThisContainsMap() 
	{
		PackageFlags |= PKG_ContainsMap;
	}

	/**
	 * Returns whether this package contains a ULevel or UWorld object.
	 *
	 * @return		TRUE if package contains ULevel/ UWorld object, FALSE otherwise.
	 */
	UBOOL ContainsMap() const
	{
		return (PackageFlags & PKG_ContainsMap) ? TRUE : FALSE;
	}

	/** initializes net info for this package from the ULinkerLoad associated with it
	 * @param InLinker the linker associated with this package to grab info from
	 * @param ExportIndex the index of the export in that linker's ExportMap where this package's export info can be found
	 *					INDEX_NONE indicates that this package is the LinkerRoot and we should look in the package file summary instead
	 */
	void InitNetInfo(ULinkerLoad* InLinker, INT ExportIndex);
	/** removes an object from the NetObjects list
	 * @param OutObject the object to remove
	 */
	void AddNetObject(UObject* InObject);
	/** removes an object from the NetObjects list
	 * @param OutObject the object to remove
	 */
	void RemoveNetObject(UObject* OutObject);
	/** clears NetIndex on all objects in our NetObjects list that are inside the passed in object
	 * @param OuterObject the Outer to check for
	 */
	void ClearAllNetObjectsInside(UObject* OuterObject);

	/** 
	 * This will add a number of NetObjects to the NetObjects list if the new count is 
	 * greater than the current size of the list.
	 *
	 * @param NewNumNetObjects new size the NetObjects list should be
	 *
	 * @script patcher
	 **/
	void PatchNetObjectList( INT NewNumNetObjects );


	/** returns the object at the specified index */
	FORCEINLINE UObject* GetNetObjectAtIndex(INT InIndex)
	{
		return (InIndex < NetObjects.Num() && NetObjects(InIndex) != NULL && !NetObjects(InIndex)->HasAnyFlags(RF_AsyncLoading)) ? NetObjects(InIndex) : NULL;
	}
	/** returns the number of net objects in the specified generation */
	FORCEINLINE INT GetNetObjectCount(INT Generation)
	{
		return (Generation < GenerationNetObjectCount.Num()) ? GenerationNetObjectCount(Generation) : 0;
	}
	/** returns a reference to GenerationNetObjectCount */
	FORCEINLINE const TArray<INT>& GetGenerationNetObjectCount()
	{
		return GenerationNetObjectCount;
	}
	/** returns our Guid */
	FORCEINLINE FGuid GetGuid()
	{
		return Guid;
	}
	/** makes our a new fresh Guid */
	FORCEINLINE void MakeNewGuid()
	{
		Guid = appCreateGuid();
	}
	/** creates empty (0 count) network object info
	 * for packages you want to be in the package map for downloading, etc, but shouldn't actually have any of its objects be relevant */
	FORCEINLINE void CreateEmptyNetInfo()
	{
		GenerationNetObjectCount.Empty();
		GenerationNetObjectCount.AddItem(0);
	}

	/** returns our FileSize */
	FORCEINLINE INT GetFileSize()
	{
		return FileSize;
	}
	/** returns ForcedExportBasePackageName */
	FORCEINLINE FName GetForcedExportBasePackageName()
	{
		return ForcedExportBasePackageName;
	}
	/** returns CurrentNumNetObjects */
	FORCEINLINE INT GetCurrentNumNetObjects()
	{
		return CurrentNumNetObjects;
	}

	///////////////////////////////////////////////////////////////////////////
	// SCC functions

	/**
	 * Accessor for the package's source control.
	 *
	 * @return		The ESourceControlState of this package.
	 */
	ESourceControlState GetSCCState() const
	{
		return SCCState;
	}

	/**
	 * Sets the package's source control state.
	 *
	 * @param		The target ESourceControlState for this package.
	 */
	void SetSCCState(ESourceControlState InSCCState)
	{
		SCCState = InSCCState;
	}

	/**
	 * Determines if this package can be checked out from the SCC depot based on it's status.
	 *
	 * @return		TRUE if the package can be checked out, FALSE otherwise.
	 */
	UBOOL SCCCanCheckOut() const
	{
		return ( GetSCCState() == SCC_ReadOnly );
	}

	/**
	 * Determines if this package can be edited.  Meaning, the current user has it checked out or it's a package we don't track in the SCC depot.
	 *
	 * @return		TRUE if the package can be edited, FALSE otherwise.
	 */
	UBOOL SCCCanEdit() const
	{
		return ( GetSCCState() == SCC_CheckedOut || GetSCCState() == SCC_Ignore );
	}


	////////////////////////////////////////////////////////
	// MetaData 

	/**
	 * Get (after possibly creating) a metadata object for this package
	 *
	 * @return A valid UMetaData pointer for all objects in this package
	 */
	class UMetaData* GetMetaData();

protected:
	// MetaData for the editor, or NULL in the game
	class UMetaData*	MetaData;
};

/*-----------------------------------------------------------------------------
	UTextBuffer.
-----------------------------------------------------------------------------*/

/**
 * An object that holds a bunch of text.  The text is contiguous and, if
 * of nonzero length, is terminated by a NULL at the very last position.
 */
class UTextBuffer : public UObject, public FOutputDevice
{
	DECLARE_CLASS(UTextBuffer,UObject,CLASS_Intrinsic,Core)

	// Variables.
	INT Pos, Top;
	FString Text;

	// Constructors.
	UTextBuffer( const TCHAR* Str=TEXT("") );

	// UObject interface.
	void Serialize( FArchive& Ar );

	// FOutputDevice interface.
	void Serialize( const TCHAR* Data, EName Event );
};

/*-----------------------------------------------------------------------------
	UMetaData.
-----------------------------------------------------------------------------*/

/**
 * An object that holds a map of key/value pairs. 
 */
class UMetaData : public UObject
{
	DECLARE_CLASS(UMetaData, UObject, CLASS_Intrinsic, Core)

#if 1
	// Variables.
	TMap<UObject*, TMap<FName, FString> > ObjectMetaDataMap;

	// UObject interface.
	void Serialize(FArchive& Ar);

	// MetaData utility functions
	/**
	 * Return the value for the given key in the given property
	 * @param Object the object to lookup the metadata for
	 * @param Key The key to lookup
	 * @return The value if found, otherwise an empty string
	 */
	const FString& GetValue(const UObject* Object, const TCHAR* Key);
	
	/**
	 * Return the value for the given key in the given property
	 * @param Object the object to lookup the metadata for
	 * @param Key The key to lookup
	 * @return The value if found, otherwise an empty string
	 */
	const FString& GetValue(const UObject* Object, FName Key);

	/**
	 * Return whether or not the Key is in the meta data
	 * @param Object the object to lookup the metadata for
	 * @param Key The key to query for existence
	 * @return TRUE if found
	 */
	UBOOL HasValue(const UObject* Object, const TCHAR* Key);

	/**
	 * Return whether or not the Key is in the meta data
	 * @param Object the object to lookup the metadata for
	 * @param Key The key to query for existence
	 * @return TRUE if found
	 */
	UBOOL HasValue(const UObject* Object, FName Key);

	/**
	 * Is there any metadata for this property?
	 * @param Object the object to lookup the metadata for
	 * @return TrUE if the object has any metadata at all
	 */
	UBOOL HasObjectValues(const UObject* Object);

	/**
	 * Set the key/value pair in the Property's metadata
	 * @param Object the object to set the metadata for
	 * @Values The metadata key/value pairs
	 */
	void SetObjectValues(const UObject* Object, const TMap<FName, FString>& Values);

	/**
	 * Set the key/value pair in the Property's metadata
	 * @param Object the object to set the metadata for
	 * @param Key A key to set the data for
	 * @param Value The value to set for the key
	 * @Values The metadata key/value pairs
	 */
	void SetValue(const UObject* Object, const TCHAR* Key, const FString& Value);

	/**
	 * Set the key/value pair in the Property's metadata
	 * @param Object the object to set the metadata for
	 * @param Key A key to set the data for
	 * @param Value The value to set for the key
	 * @Values The metadata key/value pairs
	 */
	void SetValue(const UObject* Object, FName Key, const FString& Value);
#else
	// Variables.
	TMap<FString, TMap<FName, FString> > Values;

	// UObject interface.
	void Serialize(FArchive& Ar);

	// MetaData utility functions
	/**
	 * Return the value for the given key in the given property
	 * @param ObjectPath The path name (GetPathName()) to the object in this package to lookup in
	 * @param Key The key to lookup
	 * @return The value if found, otherwise an empty string
	 */
	const FString& GetValue(const FString& ObjectPath, FName Key);

	/**
	 * Return the value for the given key in the given property
	 * @param ObjectPath The path name (GetPathName()) to the object in this package to lookup in
	 * @param Key The key to lookup
	 * @return The value if found, otherwise an empty string
	 */
	const FString& GetValue(const FString& ObjectPath, const TCHAR* Key);
	
	/**
	 * Return whether or not the Key is in the meta data
	 * @param ObjectPath The path name (GetPathName()) to the object in this package to lookup in
	 * @param Key The key to query for existence
	 * @return TRUE if found
	 */
	UBOOL HasValue(const FString& ObjectPath, FName Key);

	/**
	 * Return whether or not the Key is in the meta data
	 * @param ObjectPath The path name (GetPathName()) to the object in this package to lookup in
	 * @param Key The key to query for existence
	 * @return TRUE if found
	 */
	UBOOL HasValue(const FString& ObjectPath, const TCHAR* Key);

	/**
	 * Is there any metadata for this property?
	 * @param ObjectPath The path name (GetPathName()) to the object in this package to lookup in
	 * @return TrUE if the object has any metadata at all
	 */
	UBOOL HasObjectValues(const FString& ObjectPath);

	/**
	 * Set the key/value pair in the Property's metadata
	 * @param ObjectPath The path name (GetPathName()) to the object in this package to lookup in
	 * @Values The metadata key/value pairs
	 */
	void SetObjectValues(const FString& ObjectPath, const TMap<FName, FString>& Values);

	/**
	 * Set the key/value pair in the Property's metadata
	 * @param ObjectPath The path name (GetPathName()) to the object in this package to lookup in
	 * @param Key A key to set the data for
	 * @param Value The value to set for the key
	 * @Values The metadata key/value pairs
	 */
	void SetValue(const FString& ObjectPath, FName Key, const FString& Value);

	/**
	 * Set the key/value pair in the Property's metadata
	 * @param ObjectPath The path name (GetPathName()) to the object in this package to lookup in
	 * @param Key A key to set the data for
	 * @param Value The value to set for the key
	 * @Values The metadata key/value pairs
	 */
	void SetValue(const FString& ObjectPath, const TCHAR* Key, const FString& Value);
#endif
};

/*----------------------------------------------------------------------------
	USystem.
----------------------------------------------------------------------------*/

class USystem : public USubsystem
{
	DECLARE_CLASS(USystem,USubsystem,CLASS_Config|CLASS_Intrinsic,Core)

	// Variables.
	/** Files older than this will be deleted at startup down to MaxStaleCacheSize */
	INT StaleCacheDays;
	/** The size to clean the cache down to at startup of files older than StaleCacheDays. This will allow even old files to be cached, up to this set amount */
	INT MaxStaleCacheSize;
	/** The max size the cache can ever be */
	INT MaxOverallCacheSize;
	/** Soft limit for package size in MByte. There will be a warning when saving for larger packages. */
	INT PackageSizeSoftLimit;

	FLOAT AsyncIOBandwidthLimit;
	FString SavePath;
	FString CachePath;
	FString CacheExt;
	FString ScreenShotPath;
	TArray<FString> Paths;
	/** Paths if -seekfreeloading is used on PC. */
	TArray<FString> SeekFreePCPaths;
	/** List of directories containing script packages */
	TArray<FString> ScriptPaths;
	/** List of directories containing script packages compiled with the -FINAL_RELEASE switch */
	TArray<FString> FRScriptPaths;
	TArray<FString> CutdownPaths;
	TArray<FName> Suppress;
	TArray<FString> Extensions;
	/** Extensions if -seekfreeloading is used on PC. */
	TArray<FString> SeekFreePCExtensions;
	TArray<FString> LocalizationPaths;
	FString TextureFileCacheExtension;

	// Constructors.
	void StaticConstructor();
	USystem();

	// FExec interface.
	UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar=*GLog );

	/**
	 * Performs periodic cleaning of the cache
	 *
	 * @param bForceDeleteDownloadCache If TRUE, the entire autodownload cache will be cleared
	 */
	void PerformPeriodicCacheCleanup(UBOOL bForceDeleteDownloadCache=FALSE);

	/**
	 * Cleans out the cache as necessary to free the space needed for an incoming
	 * incoming downloaded package 
	 *
	 * @param SpaceNeeded Amount of space needed to download a package
	 */
	void CleanCacheForNeededSpace(INT SpaceNeeded);

private:

	/**
	 * Internal helper function for cleaning the cache
	 *
	 * @param MaxSize Max size the total of the cache can be (for files older than ExpirationSeconds)
	 * @param ExpirationSeconds Only delete files older than this down to MaxSize
	 */
	void CleanCache(INT MaxSize, DOUBLE ExpirationSeconds);
};

/*-----------------------------------------------------------------------------
	UCommandlet helper defines.
-----------------------------------------------------------------------------*/

#define BEGIN_COMMANDLET(ClassName,Package) \
class U##ClassName##Commandlet : public UCommandlet \
{ \
	DECLARE_CLASS(U##ClassName##Commandlet,UCommandlet,CLASS_Transient|CLASS_Intrinsic,Package); \
	U##ClassName##Commandlet() \
	{ \
		if ( !HasAnyFlags(RF_ClassDefaultObject) ) \
		{ \
			AddToRoot(); \
		} \
	}

#define END_COMMANDLET \
	void InitializeIntrinsicPropertyValues() \
	{ \
		LogToConsole	= 0;	\
		IsClient        = 1;	\
		IsEditor        = 1;	\
		IsServer        = 1;	\
		ShowErrorCount  = 1;	\
		ThisClass::StaticInitialize();	\
	} \
	INT Main(const FString& Params); \
};

#define BEGIN_CHILD_COMMANDLET(ClassName,BaseClass,Package) \
class U##ClassName##Commandlet : public U##BaseClass##Commandlet \
{ \
	DECLARE_CLASS(U##ClassName##Commandlet,U##BaseClass##Commandlet,CLASS_Transient|CLASS_Intrinsic,Package); \
	U##ClassName##Commandlet() \
	{ \
		if ( !HasAnyFlags(RF_ClassDefaultObject) ) \
		{ \
			AddToRoot(); \
		} \
	}

#define END_CHILD_COMMANDLET \
	void InitializeIntrinsicPropertyValues() \
	{ \
		LogToConsole	= 0;	\
		IsClient        = 1;	\
		IsEditor        = 1;	\
		IsServer        = 1;	\
		ShowErrorCount  = 1;	\
		ThisClass::StaticInitialize();	\
	} \
};

