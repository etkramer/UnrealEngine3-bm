/*=============================================================================
	DownloadableContent.h
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef DOWNLOADABLECONTENT_INC__
#define DOWNLOADABLECONTENT_INC__

/** A DLC directory and its contents. */
class FDLCBundle
{
public:
	FDLCBundle(const FString& InDLCRoot, const FString& InDirectory);

	static void FindDLCFiles(const FString& DLCRoot, TArray<FDLCBundle>& OutBundles);

	FString Directory;
	TArray<FString> PackageFiles;
	TArray<FString> NonPackageFiles;
};

/**
 * Helper class to handle general purpose downloadable content. Each game can 
 * sublcass this to handle the OnDownloadableContentChanged callback
 */
class FDownloadableContent
{
public:
	/**
	 * This will add files that were downloaded (or similar mechanism) to the engine. The
	 * files themselves will have been found in a platform-specific method.
	 *
	 * @param Name Friendly, displayable name of the DLC
	 * @param PackageFiles The list of files that are game content packages (.upk etc)
	 * @param NonPacakgeFiles The list of files in the DLC that are everything but packages (.ini, etc)
	 * @param UserIndex Optional user index to associate with the content (used for removing)
	 *
	 * @return TRUE if the DLC was properly installed
	 */
	virtual UBOOL InstallDownloadableContent(const TCHAR* Name, const TArray<FString>& PackageFiles, const TArray<FString>& NonPackageFiles, INT UserIndex=NO_USER_SPECIFIED);

	/**
	 * Allows a game to handle special ini sections in a DLC ini file
	 *
	 * @param NewConfigPath Pathname to the config that can be used with GConfig
	 * @param IniFilename Filename that can be used to read in the ini file manually
	 */
	virtual void HandleExtraIniSection(const FString& NewConfigPath, const FString& IniFilename)
	{
	}

	/**
	 * Removes downloadable content from the system. Can use UserIndex's or not. Will always remove
	 * content that did not have a user index specified
	 * 
	 * @param MaxUserIndex (Platform-dependent) max number of users to flush (this will iterate over all users from 0 to MaxNumUsers), as well as NO_USER 
	 */
	virtual void RemoveAllDownloadableContent(INT MaxUserIndex=0);

	/**
	 * Game-specific code to handle DLC being added or removed
	 * 
	 * @param bContentWasInstalled TRUE if DLC was installed, FALSE if it was removed
	 */
	virtual void OnDownloadableContentChanged(UBOOL bContentWasInstalled);

	/**
	 * @return the list of all installed DLC
	 */
	virtual TArray<FString> GetDownloadableContentList();

	/** Default implementation is to simply call InstallDownloadableContent against each bundle. */
	virtual void InstallDLCBundles(TArray<FDLCBundle>& Bundles);

	/**
	 * Checks if the game will allow this file to be merged into the GConfigCache
	 *
	 * @param FullFilename Full path to the file 
	 * @param BaseFilename Name of the actual ini file that would be merged with
	 * @param ExistingConfigFiles A list of the existing config files
	 *
	 * @return TRUE if the file is valid and can be added to GConfig
	 */
	virtual UBOOL IsIniOrLocFileValid(const FFilename& FullFilename, const FFilename& BaseFilename, const TArray<FFilename>& ExistingConfigFiles)
	{
		// by default 
		return TRUE;
	}

	virtual UBOOL GetDLCTextureCachePath(FName TextureCacheName, FString& Path);

protected:
	/** List of names of the installed DLCs (for UI to delete, etc) */
	TArray<FString> InstalledDLCs;

	/** TRUE if the current DLC is a "special" DLC in that is installed, but not added to list of known DLC. In UT, this is used to differentiate PS3 DLC from mods */
	UBOOL bIsInstallingSpecialDLC;

	TMap<FName,FString> DLCTextureCaches;
};

/**
 * Helper class for platform-specific DLC handling (searching for, deleting, etc)
 * By default, the base class will be used, which will do nothing
 */
class FPlatformDownloadableContent
{
public:
	/**
	 * Looks on disk for DLC and install it
	 */
	virtual void FindDownloadableContent();

	/**
	 * Deletes a single DLC from the system (physically removes it, not just uninstalls it)
	 *
	 * @param Name Name of the DLC to delete
	 */
	virtual void DeleteDownloadableContent(const TCHAR* Name);
};

/**
 * PC DLC implementation.
 */
class FPCPlatformDownloadableContent : public FPlatformDownloadableContent
{
public:
	/**
	 * Looks on disk for DLC and install it
	 */
	virtual void FindDownloadableContent();
};

/**
 * Gears specific DLC handler.
 */
class FGearDownloadableContent : public FDownloadableContent
{
public:
	virtual void HandleExtraIniSection(const FString& NewConfigPath, const FString& IniFilename);
};


// global objects
extern FDownloadableContent* GDownloadableContent;
extern FPlatformDownloadableContent* GPlatformDownloadableContent;

#endif
