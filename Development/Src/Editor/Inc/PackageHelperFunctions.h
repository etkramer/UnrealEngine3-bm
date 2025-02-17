/**
 * File to hold common package helper functions.
 *
 * 	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#ifndef _PACKAGE_HELPER_FUNCTIONS_H_
#define _PACKAGE_HELPER_FUNCTIONS_H_


/**
 * Flags which modify the way that NormalizePackageNames works.
 */
enum EPackageNormalizationFlags
{
	/** reset the linker for any packages currently in memory that are part of the output list */
	NORMALIZE_ResetExistingLoaders		= 0x01,
	/** do not include map packages in the result array; only relevant if the input array is empty */
	NORMALIZE_ExcludeMapPackages		= 0x02,
	/** do not include content packages in the result array; only relevant if the input array is empty */
	NORMALIZE_ExcludeContentPackages	= 0x04,
	/** include cooked packages in the result array; only relevant if the input array is empty */
	NORMALIZE_IncludeCookedPackages 	= 0x10,
	
	/** Combo flags */
	NORMALIZE_DefaultFlags				= NORMALIZE_ResetExistingLoaders,
};

FORCEINLINE void SearchDirectoryRecursive( const FFilename& SearchPathMask, TArray<FString>& out_PackageNames, TArray<FFilename>& out_PackageFilenames )
{
	const FFilename SearchPath = SearchPathMask.GetPath();
	TArray<FString> PackageNames;
	GFileManager->FindFiles( PackageNames, *SearchPathMask, TRUE, FALSE );
	if ( PackageNames.Num() > 0 )
	{
		for ( INT PkgIndex = 0; PkgIndex < PackageNames.Num(); PkgIndex++ )
		{
			new(out_PackageFilenames) FFilename( SearchPath * PackageNames(PkgIndex) );
		}

		out_PackageNames += PackageNames;
	}

	// now search all subdirectories
	TArray<FString> Subdirectories;
	GFileManager->FindFiles( Subdirectories, *(SearchPath * TEXT("*")), FALSE, TRUE );
	for ( INT DirIndex = 0; DirIndex < Subdirectories.Num(); DirIndex++ )
	{
		SearchDirectoryRecursive( SearchPath * Subdirectories(DirIndex) * SearchPathMask.GetCleanFilename(), out_PackageNames, out_PackageFilenames);
	}
}

/**
 * Takes an array of package names (in any format) and converts them into relative pathnames for each package.
 *
 * @param	PackageNames		the array of package names to normalize.  If this array is empty, the complete package list will be used.
 * @param	PackagePathNames	will be filled with the complete relative path name for each package name in the input array
 * @param	PackageWildcard		if specified, allows the caller to specify a wildcard to use for finding package files
 * @param	PackageFilter		allows the caller to limit the types of packages returned.
 *
 * @return	TRUE if packages were found successfully, FALSE otherwise.
 */
FORCEINLINE UBOOL NormalizePackageNames( TArray<FString> PackageNames, TArray<FFilename>& PackagePathNames, const FString& PackageWildcard=FString(TEXT("*.*")), BYTE PackageFilter=NORMALIZE_DefaultFlags )
{
	if ( PackageNames.Num() == 0 )
	{
		GFileManager->FindFiles( PackageNames, *PackageWildcard, TRUE, FALSE );
	}

	if( PackageNames.Num() == 0 )
	{
		// if no files were found, it might be an unqualified path; try prepending the .u output path
		// if one were going to make it so that you could use unqualified paths for package types other
		// than ".u", here is where you would do it
		SearchDirectoryRecursive( appScriptOutputDir() * PackageWildcard, PackageNames, PackagePathNames );
		if ( PackageNames.Num() == 0 )
		{
			TArray<FString> Paths;
			if ( GConfig->GetArray( TEXT("Core.System"), TEXT("Paths"), Paths, GEngineIni ) > 0 )
			{
				for ( INT i = 0; i < Paths.Num(); i++ )
				{
					FFilename SearchWildcard = Paths(i) * PackageWildcard;
					SearchDirectoryRecursive( SearchWildcard, PackageNames, PackagePathNames );
				}
			}
		}
		else
		{
			PackagePathNames.Empty(PackageNames.Num());

			// re-add the path information so that GetPackageLinker finds the correct version of the file.
			FFilename WildcardPath = appScriptOutputDir() * PackageWildcard;
			for ( INT FileIndex = 0; FileIndex < PackageNames.Num(); FileIndex++ )
			{
				PackagePathNames.AddItem( WildcardPath.GetPath() * PackageNames(FileIndex) );
			}
		}

		// Try finding package in package file cache.
		if ( PackageNames.Num() == 0 )
		{
			FString Filename;
			if( GPackageFileCache->FindPackageFile( *PackageWildcard, NULL, Filename ) )
			{
				new(PackagePathNames) FString(Filename);
			}
		}
	}
	else
	{
		// re-add the path information so that GetPackageLinker finds the correct version of the file.
		const FString WildcardPath = FFilename(*PackageWildcard).GetPath();
		for ( INT FileIndex = 0; FileIndex < PackageNames.Num(); FileIndex++ )
		{
			PackagePathNames.AddItem(WildcardPath * PackageNames(FileIndex));
		}
	}

	if ( PackagePathNames.Num() == 0 )
	{
		warnf(TEXT("No packages found using '%s'!"), *PackageWildcard);
		return FALSE;
	}

	// now apply any filters to the list of packages
	for ( INT PackageIndex = PackagePathNames.Num() - 1; PackageIndex >= 0; PackageIndex-- )
	{
		FString PackageExtension = PackagePathNames(PackageIndex).GetExtension();
		if ( !GSys->Extensions.ContainsItem(PackageExtension) )
		{
			// not a valid package file - remove it
			PackagePathNames.Remove(PackageIndex);
		}
		else
		{
			if ( (PackageFilter&NORMALIZE_ExcludeMapPackages) != 0 )
			{
				if ( PackageExtension == FURL::DefaultMapExt )
				{
					PackagePathNames.Remove(PackageIndex);
					continue;
				}
			}
			if ( (PackageFilter&NORMALIZE_ExcludeContentPackages) != 0 )
			{
				if ( PackageExtension != FURL::DefaultMapExt )
				{
					PackagePathNames.Remove(PackageIndex);
					continue;
				}
			}
			if ( (PackageFilter&NORMALIZE_IncludeCookedPackages) == 0 )
			{
				if ( PackageExtension == TEXT("xxx") )
				{
					PackagePathNames.Remove(PackageIndex);
					continue;
				}
			}
		}
	}

	if ( (PackageFilter&NORMALIZE_ResetExistingLoaders) != 0 )
	{
		// reset the loaders for the packages we want to load so that we don't find the wrong version of the file
		for ( INT PackageIndex = 0; PackageIndex < PackageNames.Num(); PackageIndex++ )
		{
			// (otherwise, attempting to run a commandlet on e.g. Engine.xxx will always return results for Engine.u instead)
			const FString& PackageName = FPackageFileCache::PackageFromPath(*PackageNames(PackageIndex));
			UPackage* ExistingPackage = FindObject<UPackage>(NULL, *PackageName, TRUE);
			if ( ExistingPackage != NULL )
			{
				UObject::ResetLoaders(ExistingPackage);
			}
		}
	}

	return TRUE;
}


/** 
 * Helper function to save a package that may or may not be a map package
 *
 * @param	Package		The package to save
 * @param	Filename	The location to save the package to
 * @param	ErrorDevice	the output device to use for warning and error messages
 * @param	LinkerToConformAgainst
 * @param				optional linker to use as a base when saving Package; if specified, all common names, imports and exports
 *						in Package will be sorted in the same order as the corresponding entries in the LinkerToConformAgainst
 *
 * @return TRUE if successful
 */
FORCEINLINE UBOOL SavePackageHelper(UPackage* Package, FString Filename, FOutputDevice* ErrorDevice=GWarn, ULinkerLoad* LinkerToConformAgainst=NULL)
{
	// look for a world object in the package (if there is one, there's a map)
	UWorld* World = FindObject<UWorld>(Package, TEXT("TheWorld"));
	UBOOL bSavedCorrectly;
	if (World)
	{	
		bSavedCorrectly = UObject::SavePackage(Package, World, 0, *Filename, ErrorDevice, LinkerToConformAgainst);
	}
	else
	{
		bSavedCorrectly = UObject::SavePackage(Package, NULL, RF_Standalone, *Filename, ErrorDevice, LinkerToConformAgainst);
	}

	// return success
	return bSavedCorrectly;
}





/**
 * This is our Functional "Do an Action to all Packages" Template.  Basically it handles all
 * of the boilerplate code which normally gets copy pasted around.  So now we just pass in
 * the OBJECTYPE  (e.g. Texture2D ) and then the Functor which will do the actual work.
 *
 * @see UFindMissingPhysicalMaterialsCommandlet
 * @see UFindTexturesWhichLackLODBiasOfTwoCommandlet
 **/
template< typename OBJECTYPE, typename FUNCTOR >
void DoActionToAllPackages( const FString& Params )
{
	// Parse command line.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	const TCHAR* Parms = *Params;
	UCommandlet::ParseCommandLine(Parms, Tokens, Switches);

	const UBOOL bVerbose = Switches.ContainsItem(TEXT("VERBOSE"));
	const UBOOL bLoadMaps = Switches.ContainsItem(TEXT("LOADMAPS"));
	const UBOOL bOverrideLoadMaps = Switches.ContainsItem(TEXT("OVERRIDELOADMAPS"));
	const UBOOL bOnlyLoadMaps = Switches.ContainsItem(TEXT("ONLYLOADMAPS"));
	const UBOOL bSkipReadOnly = Switches.ContainsItem(TEXT("SKIPREADONLY"));
	const UBOOL bOverrideSkipOnly = Switches.ContainsItem(TEXT("OVERRIDEREADONLY"));
	const UBOOL bGCEveryPackage = Switches.ContainsItem(TEXT("GCEVERYPACKAGE"));

	TArray<FString> FilesInPath;
	FilesInPath = GPackageFileCache->GetPackageFileList();

	INT GCIndex = 0;
	for( INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++ )
	{
		const FFilename& Filename = FilesInPath(FileIndex);

		const UBOOL	bIsShaderCacheFile		= FString(*Filename).ToUpper().InStr( TEXT("SHADERCACHE") ) != INDEX_NONE;
		const UBOOL	bIsAutoSave				= FString(*Filename).ToUpper().InStr( TEXT("AUTOSAVES") ) != INDEX_NONE;
		const UBOOL bIsReadOnly             = GFileManager->IsReadOnly( *Filename );

		// we don't care about trying to wrangle the various shader caches so just skipz0r them
		if(	Filename.GetBaseFilename().InStr( TEXT("LocalShaderCache") )	!= INDEX_NONE
			||	Filename.GetBaseFilename().InStr( TEXT("RefShaderCache") )		!= INDEX_NONE
			|| ( bIsAutoSave == TRUE )
			)
		{
			continue;
		}

		// See if we should skip read only packages
		if( bSkipReadOnly && !bOverrideSkipOnly )
		{
			const UBOOL bIsReadOnly = GFileManager->IsReadOnly( *Filename );
			if( bIsReadOnly )
			{
				warnf(TEXT("Skipping %s (read-only)"), *Filename);			
				continue;
			}
		}

		// if we don't want to load maps for this
		if( ((!bLoadMaps && !bOnlyLoadMaps) || bOverrideLoadMaps) && ( Filename.GetExtension() == FURL::DefaultMapExt ) )
		{
			continue;
		}

		// if we only want to load maps for this
		if( ( bOnlyLoadMaps == TRUE ) && ( Filename.GetExtension() != FURL::DefaultMapExt ) )
		{
			continue;
		}

		if( bVerbose == TRUE )
		{
			warnf( NAME_Log, TEXT("Loading %s"), *Filename );
		}

		// don't die out when we have a few bad packages, just keep on going so we get most of the data
		try
		{
			UPackage* Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
			if( Package != NULL )
			{
				FUNCTOR TheFunctor;
				TheFunctor.DoIt<OBJECTYPE>( Package, Tokens, Switches );
			}
			else
			{
				warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
			}
		}
		catch ( ... )
		{
			warnf( NAME_Log, TEXT("Exception %s"), *Filename.GetBaseFilename() );
		}



		if( ( (++GCIndex % 10) == 0 ) || ( bGCEveryPackage == TRUE ) )
		{
			UObject::CollectGarbage(RF_Native);
		}
	}
}




#endif // _PACKAGE_HELPER_FUNCTIONS_H_
