/*=============================================================================
	UTDownloadableContent.h: Definitions of classes used for DLC in UT.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "DownloadableContent.h"

/**
 * UT specialization for DLC
 */
class FUTDownloadableContent : public FDownloadableContent
{
public:
	/**
	 * Game-specific code to handle DLC being added or removed
	 * 
	 * @param bContentWasInstalled TRUE if DLC was installed, FALSE if it was removed
	 */
	virtual void OnDownloadableContentChanged(UBOOL bContentWasInstalled);

	/**
	 * Checks if the game will allow this file to be merged into the GConfigCache
	 *
	 * @param FullFilename Full path to the file 
	 * @param BaseFilename Name of the actual ini file that would be merged with
	 * @param ExistingConfigFiles A list of the existing config files
	 *
	 * @return TRUE if the file is valid and can be added to GConfig
	 */
	virtual UBOOL IsIniOrLocFileValid(const FFilename& FullFilename, const FFilename& BaseFilename, const TArray<FFilename>& ExistingConfigFiles);

	virtual void InstallDLCBundles(TArray<FDLCBundle>& Bundles);

	/**
	 * Allows a game to handle special ini sections in a DLC ini file
	 *
	 * @param NewConfigPath Pathname to the config that can be used with GConfig
	 * @param IniFilename Filename that can be used to read in the ini file manually
	 */
	void HandleExtraIniSection(const FString& NewConfigPath, const FString& IniFilename);

protected:
	/** Generates any missing per-map .ini files for the specified DLC bundle. */
	void GenerateMissingCustomMapINIsForBundle(const FDLCBundle& Bundle);
};
