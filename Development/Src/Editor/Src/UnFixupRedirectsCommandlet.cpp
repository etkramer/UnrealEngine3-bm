/*=============================================================================
	UnFixupRedirectsCommandlet.cpp: Object redirect cleaner commandlet
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "../../UnrealEd/Inc/SourceControlIntegration.h"

#if 1
 
/**
 * Container struct that holds info about a redirector that was followed. 
 * Cannot contain a pointer to the actual UObjectRedirector as the object
 * will be GC'd before it is used
 */
struct FRedirection
{
	FString PackageFilename;
	FString RedirectorName;
	FString RedirectorPackageFilename;
	FString DestinationObjectName;

	UBOOL operator==(const FRedirection& Other) const
	{
		return PackageFilename == Other.PackageFilename &&
			RedirectorName == Other.RedirectorName &&
			RedirectorPackageFilename == Other.RedirectorPackageFilename;
	}

	friend FArchive& operator<<(FArchive& Ar, FRedirection& Redir)
	{
		Ar << Redir.PackageFilename;
		Ar << Redir.RedirectorName;
		Ar << Redir.RedirectorPackageFilename;
		Ar << Redir.DestinationObjectName;
		return Ar;
	}
};

/**
 * Callback structure to respond to redirect-followed callbacks and store
 * the information
 */
class FRedirectCollector : public FCallbackEventDevice
{
public:
	/**
	 * Responds to CALLBACK_RedirectorFollowed. Records all followed redirections
	 * so they can be cleaned later.
	 *
	 * @param InType Callback type (should only be CALLBACK_RedirectorFollowed)
	 * @param InString Name of the package that pointed to the redirect
	 * @param InObject The UObjectRedirector that was followed
	 */
	virtual void Send( ECallbackEventType InType, const FString& InString, UObject* InObject)
	{
		check(InType == CALLBACK_RedirectorFollowed);

		// the object had better be a redir
		UObjectRedirector* Redirector = Cast<UObjectRedirector>(InObject);
		check(Redirector);

		// save the info if it matches the parameters we need to match on:
		// if we are loading code packages, and the extension is .u OR the string matches the package we were loading
		// AND
		// we aren't matching a particular package || the string matches the package
		if (!FileToFixup.Len() || FileToFixup == FFilename(Redirector->GetLinker()->Filename).GetBaseFilename())
		{
			FRedirection Redir;
			Redir.PackageFilename = InString;
			Redir.RedirectorName = Redirector->GetFullName();
			Redir.RedirectorPackageFilename = Redirector->GetLinker()->Filename;
			Redir.DestinationObjectName = Redirector->DestinationObject->GetFullName();
			// script redirections are stored specially as they cannot be cleaned up
			if (FFilename(InString).GetExtension() == TEXT("u"))
			{
				// we only want one of each redirection reported
				ScriptRedirections.AddUniqueItem(Redir);
			}
			else
			{
				// we only want one of each redirection reported
				Redirections.AddUniqueItem(Redir);
			}
		}
	}
	/** If not an empty string, only fixup redirects in this package */
	FString FileToFixup;

	/** A gathered list of all non-script referenced redirections */
	TArrayNoInit<FRedirection> Redirections;

	/** A gathered list of all redirections referenced directly by script*/
	TArrayNoInit<FRedirection> ScriptRedirections;
};

// global redirect collector callback structure
FRedirectCollector GRedirectCollector;


/**
 * Allows commandlets to override the default behavior and create a custom engine class for the commandlet. 
 * However, this implementation simply installs the RedirectorFollowed callback to trap redirectors
 * being followed during script loading.
 */
void UFixupRedirectsCommandlet::CreateCustomEngine()
{
	SET_WARN_COLOR(COLOR_WHITE);
	warnf(NAME_Log, TEXT(""));
	warnf(NAME_Log, TEXT("Loading script code:"));
	warnf(NAME_Log, TEXT("--------------------"));
	CLEAR_WARN_COLOR();

	// make sure we register the callback BEFORE calling InitEditor
	// so we can trap redirects followed by script code
	GCallbackEvent->Register(CALLBACK_RedirectorFollowed, &GRedirectCollector);

	// by making sure these are NULL, the caller will create the default engine object
	GEngine = GEditor = NULL;
}

INT UFixupRedirectsCommandlet::Main( const FString& Params )
{
	// Retrieve list of all packages in .ini paths.
	TArray<FString> PackageList = GPackageFileCache->GetPackageFileList();
	if( !PackageList.Num() )
		return 0;

	// process the commandline
	FString Token;
	const TCHAR* CommandLine	= appCmdLine();
	UBOOL bIsQuietMode			= FALSE;
	UBOOL bIsTestOnly			= FALSE;
	UBOOL bShouldRestoreProgress= FALSE;

	while (ParseToken(CommandLine, Token, 1))
	{
		if (Token == TEXT("-nowarn"))
		{
			bIsQuietMode = TRUE;
		}
		else if (Token == TEXT("-testonly"))
		{
			bIsTestOnly = TRUE;
		}
		else if (Token == TEXT("-restore"))
		{
			bShouldRestoreProgress = TRUE;
		}
		else
		{
			FString PackageName;
			if (GPackageFileCache->FindPackageFile(*Token, NULL, PackageName))
			{
				GRedirectCollector.FileToFixup = FFilename(Token).GetBaseFilename();
			}
		}
	}

	UBOOL bShouldSkipErrors		= FALSE;

	/////////////////////////////////////////////////////////////////////
	// Display any script code referenced redirects
	/////////////////////////////////////////////////////////////////////

	TArray<FString> UpdatePackages;
	TArray<FString> ReferencedTrashPackages;
	TArray<FString> RedirectorsThatCantBeCleaned;

	if (!bShouldRestoreProgress)
	{
		// load any remaining .u files
		TArray<FString>				AllScriptPackageNames;
		appGetEngineScriptPackageNames( AllScriptPackageNames, TRUE );
		appGetGameNativeScriptPackageNames( AllScriptPackageNames, TRUE );
		appGetGameScriptPackageNames( AllScriptPackageNames, TRUE );

		for (INT PackageIndex = 0; PackageIndex < AllScriptPackageNames.Num(); PackageIndex++)
		{
			UObject::LoadPackage(NULL, *AllScriptPackageNames(PackageIndex), 0);
		}


		// a list of the redirectors that can't be cleaned because a read-only package references them, or script code references them
		TArray<FString> RedirectorsThatCantBeCleaned;
		if (GRedirectCollector.ScriptRedirections.Num())
		{
			SET_WARN_COLOR(COLOR_DARK_RED);
			warnf(NAME_Log, TEXT("A programmer must fix the following script code references to redirects."));

			SET_WARN_COLOR(COLOR_DARK_GREEN);
			// any redirectors referenced by script code can't be cleaned up
			for (INT RedirIndex = 0; RedirIndex < GRedirectCollector.ScriptRedirections.Num(); RedirIndex++)
			{
				FRedirection& ScriptRedir = GRedirectCollector.ScriptRedirections(RedirIndex);
				warnf(NAME_Log, TEXT("%s reference to %s [-> %s]"), *FFilename(ScriptRedir.PackageFilename).GetBaseFilename(),
					*ScriptRedir.RedirectorName, *ScriptRedir.DestinationObjectName);
				RedirectorsThatCantBeCleaned.AddUniqueItem(ScriptRedir.RedirectorName);
			}
			CLEAR_WARN_COLOR();
		}

		/////////////////////////////////////////////////////////////////////
		// Build a list of packages that need to be updated
		/////////////////////////////////////////////////////////////////////

		SET_WARN_COLOR(COLOR_WHITE);
		warnf(NAME_Log, TEXT(""));
		warnf(NAME_Log, TEXT("Generating list of tasks:"));
		warnf(NAME_Log, TEXT("-------------------------"));
		CLEAR_WARN_COLOR();

		INT GCIndex = 0;

		// @todo: allow a wildcard to fixup certain packages

		// process script code first pass, then everything else
		for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
		{
			const FFilename& Filename = PackageList(PackageIndex);

			// already loaded code, skip them, and skip autosave packages
			if (Filename.GetExtension() == TEXT("u") || Filename.Left(GEditor->AutoSaveDir.Len()) == GEditor->AutoSaveDir)
			{
				continue;
			}

			// we don't care about trying to load the various shader caches so just skipz0r them
			if(	Filename.InStr( TEXT("LocalShaderCache") ) != INDEX_NONE
				|| Filename.InStr( TEXT("RefShaderCache") ) != INDEX_NONE )
			{
				continue;
			}


			warnf(NAME_Log, TEXT("Looking for redirects in %s..."), *Filename);

			// Assert if package couldn't be opened so we have no chance of messing up saving later packages.
			UPackage* Package = Cast<UPackage>(UObject::LoadPackage(NULL, *Filename, 0));
			if (!Package)
			{
				SET_WARN_COLOR(COLOR_RED);
				warnf(NAME_Log, TEXT("   ... failed to open %s"), *Filename);
				CLEAR_WARN_COLOR();
				// if we are not in quiet mode, and the user wants to continue, go on
				if (bShouldSkipErrors || (!bIsQuietMode && GWarn->YesNof(LocalizeSecure(LocalizeQuery(TEXT("PackageFailedToOpen"), TEXT("Core")), *Filename))))
				{
					bShouldSkipErrors = TRUE;
					continue;
				}
				else
				{
					return 1;
				}
			}

			// all notices here about redirects or trash get this color
			SET_WARN_COLOR(COLOR_DARK_GREEN);

			// look for any redirects that were followed that are referenced by this package
			// (may have been loaded earlier if this package was loaded by script code)
			// any redirectors referenced by script code can't be cleaned up
			for (INT RedirIndex = 0; RedirIndex < GRedirectCollector.Redirections.Num(); RedirIndex++)
			{
				FRedirection& Redir = GRedirectCollector.Redirections(RedirIndex);

				if (Redir.PackageFilename == Filename)
				{
					warnf(NAME_Log, TEXT("   ... references %s [-> %s]"), *Redir.RedirectorName, *Redir.DestinationObjectName);

					// put the source and dest packages in the list to be updated
					UpdatePackages.AddUniqueItem(Redir.PackageFilename);
					// if we are pointing to a redirector IN a .u file (it's happened) then we don't want
					// to clean the .u
					if (FFilename(Redir.RedirectorPackageFilename).GetExtension() != TEXT("u"))
					{
						UpdatePackages.AddUniqueItem(Redir.RedirectorPackageFilename);
					}
				}
			}

			// clear the flag for needing to collect garbage
			UBOOL bNeedToCollectGarbage = FALSE;

			// if this package has any redirectors, make sure we update it
			if (!GRedirectCollector.FileToFixup.Len() || GRedirectCollector.FileToFixup == Filename.GetBaseFilename())
			{
				for (TObjectIterator<UObjectRedirector> It; It; ++It)
				{
					if (It->IsIn(Package))
					{
						// make sure this package is in the list of packages to update
						UpdatePackages.AddUniqueItem(Filename);

						warnf(NAME_Log, TEXT("   ... has redirect %s [-> %s]"), *It->GetPathName(), *It->DestinationObject->GetFullName());

						// make sure we GC if we found a redirector
						bNeedToCollectGarbage = TRUE;
					}
				}
			}

			// if this package isn't trash, check to see if loading it has caused any trash packages
			// to load... these packages can't be deleted for good as they are still in use
			if (!(Cast<UPackage>(Package)->PackageFlags & PKG_Trash))
			{
				for (TObjectIterator<UPackage> It; It; ++It)
				{
					if (It->PackageFlags & PKG_Trash)
					{
						if (ReferencedTrashPackages.FindItemIndex(FString(*It->GetName())) == INDEX_NONE)
						{
							warnf(NAME_Log, TEXT("   Found a referenced trash package: %s"), *It->GetName());
							new(ReferencedTrashPackages) FString(*It->GetName());
						}
					}
				}
			}

			CLEAR_WARN_COLOR();

			// collect garbage every N packages, or if there was any redirectors in the package 
			// (to make sure that redirectors get reloaded properly and followed by the callback)
			// also, GC after loading a Trash package, to make sure it's unloaded so that we can track
			// other packages loading a Trash package as a dependency
			if (((++GCIndex) % 5) == 0 || bNeedToCollectGarbage || (Package->PackageFlags & PKG_Trash))
			{
				// collect garbage to close the package
				UObject::CollectGarbage(RF_Native);
				// reset our counter
				GCIndex = 0;
			}
		}

		// save off restore point
		FArchive* Ar = GFileManager->CreateFileWriter(*(appGameDir() + TEXT("Fixup.bin")));
		*Ar << GRedirectCollector.Redirections;
		*Ar << UpdatePackages;
		*Ar << ReferencedTrashPackages;
		*Ar << RedirectorsThatCantBeCleaned;
		delete Ar;
	}
	else
	{
		// load restore point
		FArchive* Ar = GFileManager->CreateFileReader(*(appGameDir() + TEXT("Fixup.bin")));
		if( Ar != NULL )
		{
			*Ar << GRedirectCollector.Redirections;
			*Ar << UpdatePackages;
			*Ar << ReferencedTrashPackages;
			*Ar << RedirectorsThatCantBeCleaned;
			delete Ar;
		}
	}

	// unregister the callback so we stop getting redirections added
	GCallbackEvent->Unregister(CALLBACK_RedirectorFollowed, &GRedirectCollector);

	/////////////////////////////////////////////////////////////////////
	// Explain to user what is happening
	/////////////////////////////////////////////////////////////////////
	SET_WARN_COLOR(COLOR_YELLOW);
	warnf(NAME_Log, TEXT(""));
	warnf(NAME_Log, TEXT("Files that need to fixed up:"));
	SET_WARN_COLOR(COLOR_DARK_YELLOW);

	// print out the working set of packages
	for (INT PackageIndex = 0; PackageIndex < UpdatePackages.Num(); PackageIndex++)
	{
		warnf(NAME_Log, TEXT("   %s"), *UpdatePackages(PackageIndex));
	}

	warnf(NAME_Log, TEXT(""));
	CLEAR_WARN_COLOR();

	// if we are only testing, just just quiet before actually doing anything
	if (bIsTestOnly)
	{
		return 0;
	}

	/////////////////////////////////////////////////////////////////////
	// Find redirectors that are referenced by read-only packages
	/////////////////////////////////////////////////////////////////////

	// source control object
#if HAVE_SCC
	FSourceControlIntegration* SCC = NULL;
#endif

	// Make sure all packages are writeable!
	for( INT PackageIndex = 0; PackageIndex < UpdatePackages.Num(); PackageIndex++ )
	{
		const FFilename& Filename = UpdatePackages(PackageIndex);

		if (GFileManager->IsReadOnly(*Filename))
		{
			// initialize source control if it hasn't been initialized yet
#if HAVE_SCC
			if (!SCC)
			{
				SCC = new FSourceControlIntegration;
			}

			FString PackageName(Filename.GetBaseFilename());
			if (SCC->GetSCCState(*PackageName) == SCC_ReadOnly)
			{
				SCC->CheckOut(*PackageName);
			}
#endif

			// if the checkout failed for any reason, this will still be readonly, so we can't clean it up
			if (GFileManager->IsReadOnly(*Filename))
			{
				SET_WARN_COLOR(COLOR_RED);
				warnf(NAME_Log, TEXT("Couldn't check out %s..."), *Filename);
				CLEAR_WARN_COLOR();

				// any redirectors that are pointed to by this read-only package can't be fixed up
				for (INT RedirIndex = 0; RedirIndex < GRedirectCollector.Redirections.Num(); RedirIndex++)
				{
					FRedirection& Redir = GRedirectCollector. Redirections(RedirIndex);

					// any redirectors pointed to by this file we don't clean
					if (Redir.PackageFilename == Filename)
					{
						RedirectorsThatCantBeCleaned.AddUniqueItem(Redir.RedirectorName);
						warnf(NAME_Log, TEXT("   ... can't fixup references to %s"), *Redir.RedirectorName);
					}
					// any redirectors in this file can't be deleted
					else if (Redir.RedirectorPackageFilename == Filename)
					{
						RedirectorsThatCantBeCleaned.AddUniqueItem(Redir.RedirectorName);
						warnf(NAME_Log, TEXT("   ... can't delete %s"), *Redir.RedirectorName);
					}
				}
			} 
			else
			{
				SET_WARN_COLOR(COLOR_DARK_GREEN);
				warnf(NAME_Log, TEXT("Checked out %s..."), *Filename);
				CLEAR_WARN_COLOR();
			}
		}
	}

	warnf(NAME_Log, TEXT(""));

	/////////////////////////////////////////////////////////////////////
	// Load and save packages to save out the proper fixed up references
	/////////////////////////////////////////////////////////////////////

	SET_WARN_COLOR(COLOR_WHITE);
	warnf(NAME_Log, TEXT("Fixing references to point to proper objects:"));
	warnf(NAME_Log, TEXT("---------------------------------------------"));
	CLEAR_WARN_COLOR();

	// keep a list of all packages that have ObjectRedirects in them 
	TArray<UBOOL> PackagesWithRedirects;
	PackagesWithRedirects.AddZeroed(UpdatePackages.Num());

	// Iterate over all packages, loading and saving to remove all references to ObjectRedirectors (can't delete the Redirects yet)
	for( INT PackageIndex = 0; PackageIndex < UpdatePackages.Num(); PackageIndex++ )
	{
		const FFilename& Filename = UpdatePackages(PackageIndex);
		
		// we can't do anything with packages that are read-only (we don't want to fixup the redirects, 
		if (GFileManager->IsReadOnly( *Filename))
		{
			continue;
		}

		warnf(NAME_Log, TEXT("Cleaning %s"), *Filename);

		// Assert if package couldn't be opened so we have no chance of messing up saving later packages.
		UPackage* Package = UObject::LoadPackage( NULL, *Filename, 0 );

		// if it failed to open, we have already dealt with quitting if we are going to, so just skip it
		if (!Package)
		{
			continue;
		}

		for (TObjectIterator<UObjectRedirector> It; It; ++It)
		{
			if (It->IsIn(Package))
			{
				PackagesWithRedirects(PackageIndex) = 1;
				break;
			}
		}

		// save out the package
		UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
		if( World )
		{	
			UObject::SavePackage( Package, World, 0, *Filename, GWarn );
		}
		else
		{
			UObject::SavePackage( Package, NULL, RF_Standalone, *Filename, GWarn );
		}

		// collect garbage to close the package
		UObject::CollectGarbage(RF_Native);
	}

	warnf(NAME_Log, TEXT(""));

	/////////////////////////////////////////////////////////////////////
	// Delete all redirects that are no longer referenced
	/////////////////////////////////////////////////////////////////////

	SET_WARN_COLOR(COLOR_WHITE);
	warnf(NAME_Log, TEXT("Deleting redirector objects:"));
	warnf(NAME_Log, TEXT("----------------------------"));
	CLEAR_WARN_COLOR();

	// Again, iterate over all packages, loading and saving to and this time deleting all 
	for( INT PackageIndex = 0; PackageIndex < UpdatePackages.Num(); PackageIndex++ )
	{
		const FFilename& Filename = UpdatePackages(PackageIndex);
		
		if (PackagesWithRedirects(PackageIndex) == 0)
			continue;

		warnf(NAME_Log, TEXT("Wiping %s..."), *Filename);

		UPackage* Package = UObject::LoadPackage( NULL, *Filename, 0 );
		// if it failed to open, we have already dealt with quitting if we are going to, so just skip it
		if (!Package)
		{
			continue;
		}

		// assume that all redirs in this file are still-referenced
		UBOOL bIsDirty = FALSE;

		// delete all ObjectRedirects now
		TArray<UObjectRedirector*> Redirs;
		// find all redirectors, put into an array so delete doesn't mess up the iterator
		for (TObjectIterator<UObjectRedirector> It; It; ++It)
		{
			if (It->IsIn(Package))
			{
				INT Index;
				// if the redirector was marked as uncleanable, skip it
				if (RedirectorsThatCantBeCleaned.FindItem(It->GetFullName(), Index))
				{
					SET_WARN_COLOR(COLOR_DARK_RED);
					warnf(NAME_Log, TEXT("   ... skipping still-referenced %s"), *It->GetFullName());
					CLEAR_WARN_COLOR();
					continue;
				}

				bIsDirty = TRUE;
				SET_WARN_COLOR(COLOR_DARK_GREEN);
				warnf(NAME_Log, TEXT("   ... deleting %s"), *It->GetFullName());
				CLEAR_WARN_COLOR();
				Redirs.AddItem(*It);
			}
		}

		// mark them for deletion.
		for (INT RedirIndex = 0; RedirIndex < Redirs.Num(); RedirIndex++)
		{
			UObjectRedirector* Redirector = Redirs(RedirIndex);
			// Remove standalone flag and mark as pending kill.
			Redirector->ClearFlags( RF_Standalone | RF_Public );
			Redirector->MarkPendingKill();
		}
		Redirs.Empty();

		// save the package
		UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
		if( World )
		{	
			UObject::SavePackage( Package, World, 0, *Filename, GWarn );
		}
		else
		{
			UObject::SavePackage( Package, NULL, RF_Standalone, *Filename, GWarn );
		}

		// collect garbage
		UObject::CollectGarbage(RF_Native);
	}

	warnf(NAME_Log, TEXT(""));

	/////////////////////////////////////////////////////////////////////
	// Clean up any unused trash
	/////////////////////////////////////////////////////////////////////

	SET_WARN_COLOR(COLOR_WHITE);
	warnf(NAME_Log, TEXT("Deleting unused packages from Trashcan:"));
	warnf(NAME_Log, TEXT("---------------------------------------"));
	CLEAR_WARN_COLOR();

	// Iterate over packages, attempting to delete trash packages
	for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
	{
		const FFilename& Filename = PackageList(PackageIndex);

		if (Filename.InStr(TRASHCAN_DIRECTORY_NAME) != -1)
		{
			FString PackageName(Filename.GetBaseFilename());
			// if this package wasn't referenced, then we can delete it!
			if (ReferencedTrashPackages.FindItemIndex(PackageName) == INDEX_NONE)
			{
				// initialize source control if it hasn't been initialized yet
#if HAVE_SCC
				if (!SCC)
				{
					SCC = new FSourceControlIntegration;
				}

				// get file SCC status
				ESourceControlState SCCState = SCC->GetSCCState(*PackageName);

				// remove it now that we've closed it
				switch (SCCState)
				{
					// if we are checked out, then undo the checkout and then fall through to the normal case					
					case SCC_CheckedOut:
						SCC->UncheckOut(*PackageName);
						// FALLING THROUGH TO NEXT CASE!

					// delete from source control
					case SCC_ReadOnly:
					case SCC_NotCurrent:
						SET_WARN_COLOR(COLOR_GREEN);
						warnf(NAME_Log, TEXT("Deleting '%s' from source control..."), *Filename);
						CLEAR_WARN_COLOR();
						SCC->Remove(*PackageName);
						break;

					// someone else has it checked out, so we can't do anything about it
					case SCC_CheckedOutOther:
						SET_WARN_COLOR(COLOR_RED);
						warnf(NAME_Log, TEXT("Couldn't delete '%s' from source control, someone has it checked out, skipping..."), *Filename);
						CLEAR_WARN_COLOR();
						break;
					

					// this file was never added to source control, so just delete it
					case SCC_NotInDepot:
						SET_WARN_COLOR(COLOR_GREEN);
						warnf(NAME_Log, TEXT("'%s' is not in source control, attempting to delete from disk..."), *Filename);
						if (!GFileManager->Delete(*Filename, FALSE, TRUE))
						{
							SET_WARN_COLOR(COLOR_RED);
							warnf(NAME_Log, TEXT("  ... failed to delete from disk."), *Filename);
						}
						CLEAR_WARN_COLOR();
						break;

					// not sure, so try to delete from disk
					case SCC_DontCare:
					case SCC_Ignore:
						SET_WARN_COLOR(COLOR_GREEN);
						warnf(NAME_Log, TEXT("'%s' is in an unknown source control state, attempting to delete from disk..."), *Filename);
						if (!GFileManager->Delete(*Filename, FALSE, TRUE))
						{
							SET_WARN_COLOR(COLOR_RED);
							warnf(NAME_Log, TEXT("  ... failed to delete from disk."), *Filename);
						}
						CLEAR_WARN_COLOR();
						break;
				}
#else  // No SCC support in this build.
				SET_WARN_COLOR(COLOR_GREEN);
				warnf(NAME_Log, TEXT("'%s' is in an unknown source control state, attempting to delete from disk..."), *Filename);
				if (!GFileManager->Delete(*Filename, FALSE, TRUE))
				{
					SET_WARN_COLOR(COLOR_RED);
					warnf(NAME_Log, TEXT("  ... failed to delete from disk."), *Filename);
				}
				CLEAR_WARN_COLOR();
#endif
			}
			else
			{
				SET_WARN_COLOR(COLOR_RED);
				warnf(NAME_Log, TEXT("Unable to delete %s from the trash, it's still referenced."), *Filename);
				CLEAR_WARN_COLOR();
			}
		}
	}

	// toss the SCC manager
#if HAVE_SCC
	delete SCC;
#endif

	return 0;
}
IMPLEMENT_CLASS(UFixupRedirectsCommandlet)





#else




/**
 * Allows commandlets to override the default behavior and create a custom engine class for the commandlet. 
 * However, this implementation simply installs the RedirectorFollowed callback to trap redirectors
 * being followed during script loading.
 */
void UFixupRedirectsCommandlet::CreateCustomEngine()
{
	// by making sure these are NULL, the caller will create the default engine object
	GEngine = GEditor = NULL;
}

struct FPackageExports
{
	/** TRUE if this package was in the __Trashcan directory */
	UBOOL bIsTrashPackage;

	/** A map to lookup exports by object name quickly */
	TMultiMap<FName, FObjectExport> Exports;
};

struct FRedirection
{
	FString PackageFilename;
	FString RedirectorName;
	FString RedirectorPackageFilename;

	UBOOL operator==(const FRedirection& Other) const
	{
		return PackageFilename == Other.PackageFilename &&
			RedirectorName == Other.RedirectorName &&
			RedirectorPackageFilename == Other.RedirectorPackageFilename;
	}

};
TArray<FRedirection> AllRedirections;

// @todo: This assumes unique class name across all script code (since we HIJACK the ClassPackage)
void FindExport(const FString& ImportPackageName, TArray<FObjectImport>& Imports, INT ImportIndex, TMap<FString, FPackageExports> AllPackageExports)
{
	FObjectImport& Import = Imports(ImportIndex);

	// do nothing if already found
	if (Import.SourceIndex != INDEX_NONE)
	{
		return;
	}

	// if we have no outer, then we will search for an outermost export
	INT OuterIndexToSearchFor = INDEX_NONE;

	// find import from name and outer's SourceIndex
	if (Import.OuterIndex != 0)
	{
		check(IS_IMPORT_INDEX(Import.OuterIndex));
	
		INT OuterIndex = -Import.OuterIndex - 1; 
		FObjectImport& OuterImport = Imports(OuterIndex);

		// make sure we have our outer
		FindExport(ImportPackageName, Imports, OuterIndex, AllPackageExports);
		
		// copy our outer's cached export's package name (HIJACK)
		Import.ClassPackage = OuterImport.ClassPackage;

		// this is the outer we're searching for
		OuterIndexToSearchFor = OuterImport.SourceIndex;

		// make sure it worked
		check(OuterImport.SourceLinker != INDEX_NONE);
	}
	else if (Import.SourceIndex == INDEX_NONE)
	{
		// HIJACK the ClassPackage to be the name of the export's package
		Import.ClassPackage = Import.ObjectName;
		return;
	}

	// get the HIJACKED cached export's package name 
	FPackageExports* PackageExports = AllPackageExports.Find(*Import.ClassPackage.ToString());
	check(PackageExports);

	// find all the exports with this objectname
	TArray<FObjectExport> Exports;
	PackageExports->Exports.MultiFind(Import.ObjectName, Exports);

	// now we can find the export from the world's exports plus an outer
	for (INT ExportIndex = 0; ExportIndex < Exports.Num(); ExportIndex++)
	{
		FObjectExport& Export = Exports(ExportIndex);
		// get the cached class name from the export
		FName ClassName = *((FName*)&Export.ClassIndex);
		// if the exports outer matches our outer, and it's the same name
		// @todo: make this faster with a hash or something
		if ((Export.OuterIndex - 1) == OuterIndexToSearchFor && 
			Export.ObjectName == Import.ObjectName && 
			(Import.ClassName == ClassName || ClassName == NAME_ObjectRedirector))
		{
			// return the class of the export
			Import.ClassName = ClassName;
			Import.SourceIndex = ExportIndex;

			if (ClassName == NAME_ObjectRedirector)
			{
				FString ExportPackageName = Import.ClassPackage.ToString();
				FRedirection Redir;
				Redir.PackageFilename = ImportPackageName;

				// make the full name of the redirector
				FString S = ClassName.ToString() + TEXT(" ");
				for (INT LinkerIndex = ExportIndex + 1; LinkerIndex != 0; LinkerIndex = Exports(LinkerIndex - 1).OuterIndex)
				{ 
					if (LinkerIndex != ExportIndex + 1)
						S = US + TEXT(".") + S;
					S = Exports(LinkerIndex - 1).ObjectName.ToString() + S;
				}
				Redir.RedirectorName = ExportPackageName + TEXT(".") + S;

				// copy the HIJACKED export's package name to the packagefilename
				Redir.RedirectorPackageFilename = ExportPackageName;
				AllRedirections.AddUniqueItem(Redir);
			}
			return;
		}
	}

	// this prolly shouldn't get here
	warnf(NAME_Error, TEXT("Couldn't find export for %s %s"), *Import.ClassName.ToString(), *Import.ObjectName.ToString());
}

INT UFixupRedirectsCommandlet::Main( const FString& Params )
{
	// Retrieve list of all packages in .ini paths.
	TArray<FString> PackageList = GPackageFileCache->GetPackageFileList();
	if( !PackageList.Num() )
		return 0;

	// process the commandline
	FString Token;
	const TCHAR* CommandLine	= appCmdLine();
	UBOOL bIsQuietMode			= FALSE;
	UBOOL bIsTestOnly			= FALSE;

	while (ParseToken(CommandLine, Token, 1))
	{
		if (Token == TEXT("-nowarn"))
		{
			bIsQuietMode = TRUE;
		}
		else if (Token == TEXT("-testonly"))
		{
			bIsTestOnly = TRUE;
		}
		else
		{
// @TODO: Handle single package cleanup
#if 0
			FString PackageName;
			if (GPackageFileCache->FindPackageFile(*Token, NULL, PackageName))
			{
				GRedirectCollector.FileToFixup = FFilename(Token).GetBaseFilename();
			}
#endif
		}
	}

	UBOOL bShouldSkipErrors		= FALSE;

	/////////////////////////////////////////////////////////////////////
	// Build a list of packages that need to be updated
	/////////////////////////////////////////////////////////////////////

	TArray<FString> UpdatePackages;
	TArray<FString> ReferencedTrashPackages;
	TArray<FString> RedirectorsThatCantBeCleaned;

	SET_WARN_COLOR(COLOR_WHITE);
	warnf(NAME_Log, TEXT(""));
	warnf(NAME_Log, TEXT("Getting a list of all objects:"));
	warnf(NAME_Log, TEXT("------------------------------"));
	CLEAR_WARN_COLOR();

	INT GCIndex = 0;

	// @todo: allow a wildcard to fixup certain packages

	TMap<FString, FPackageExports> AllPackageExports;

	// process script code first pass, then everything else
	for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
	{
		const FFilename& Filename = PackageList(PackageIndex);
		FString PackageName(Filename.GetBaseFilename());

		UObject::BeginLoad();
		ULinkerLoad* Linker = UObject::GetPackageLinker( NULL, *Filename, LOAD_Quiet|LOAD_NoWarn|LOAD_NoVerify, NULL, NULL );
		UObject::EndLoad();

		if (!Linker)
		{
			SET_WARN_COLOR(COLOR_RED);
			warnf(NAME_Log, TEXT("   ... failed to open %s"), *Filename);
			CLEAR_WARN_COLOR();
			// if we are not in quiet mode, and the user wants to continue, go on
			if (bShouldSkipErrors || (!bIsQuietMode && GWarn->YesNof(LocalizeSecure(LocalizeQuery(TEXT("PackageFailedToOpen"), TEXT("Core")), *Filename))))
			{
				bShouldSkipErrors = TRUE;
				continue;
			}
			else
			{
				return 1;
			}
		}

		// create a newentry in the package->exports map
		FPackageExports& PackageExports = AllPackageExports.Set(*Filename.GetBaseFilename(), FPackageExports());

		// look for any redirs in this package
		for (INT ExportIndex = 0; ExportIndex < Linker->ExportMap.Num(); ExportIndex++)
		{
			FObjectExport& Export = Linker->ExportMap(ExportIndex);
			// get class of the export
			FName ClassName = Linker->GetExportClassName(ExportIndex);
			if (ClassName == NAME_ObjectRedirector)
			{
				// tell user, and add this package to list of ones to fixup later
				warnf(TEXT("Found redirector %s"), *Linker->GetExportFullName(ExportIndex));
				UpdatePackages.AddUniqueItem(Filename);

				FRedirection Redir;
				Redir.PackageFilename = PackageName;
				Redir.RedirectorName = Linker->GetExportFullName(ExportIndex);
				// has no destination package name!

				AllRedirections.AddItem(Redir);
			}

			// cache the class name in the export (overwriting ClassIndex and SuperIndex assuming FName is 
			// 8 bytes. It will overwrite archetype index also if its 12, which is fine)
			*((FName*)&Export.ClassIndex) = ClassName;

			// add this export to the quick lookup map
			PackageExports.Exports.Add(Export.ObjectName, Export);
		}

		if ((PackageIndex % 20) == 0)
		{
			UObject::CollectGarbage(RF_Native);
		}
	}


	SET_WARN_COLOR(COLOR_DARK_GREEN);

	// process script code first pass, then everything else
	for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
	{
		const FFilename& Filename = PackageList(PackageIndex);
		FString PackageName(Filename.GetBaseFilename());

		// script shouldn't be referencing any redirects, skip them
		if (Filename.GetExtension() == TEXT("u"))
		{
			continue;
		}

		// we don't care about trying to load the various shader caches so just skipz0r them
		if(	Filename.InStr( TEXT("LocalShaderCache") ) != INDEX_NONE
			|| Filename.InStr( TEXT("RefShaderCache") ) != INDEX_NONE )
		{
			continue;
		}

		warnf(NAME_Log, TEXT("Looking for references to redirects in %s..."), *Filename);

		UObject::BeginLoad();
		ULinkerLoad* Linker = UObject::GetPackageLinker( NULL, *Filename, LOAD_Quiet|LOAD_NoWarn|LOAD_NoVerify, NULL, NULL );
		UObject::EndLoad();

		// go through all imports looking for refs to redirects in other packages
		for (INT ImportIndex = 0; ImportIndex < Linker->ImportMap.Num(); ImportIndex++)
		{
			FObjectImport& Import = Linker->ImportMap(ImportIndex);

			// get the corresponding export for this import
			FindExport(PackageName, Linker->ImportMap, ImportIndex, AllPackageExports);
			
			// FindExport will have set the classname to redirector if it pointed to one
			if (Import.ClassName == NAME_ObjectRedirector)
			{
				warnf(TEXT("   ... has a reference to a redirect %s"), *Import.ObjectName.ToString());
				UpdatePackages.AddUniqueItem(Filename);
			}
		}

// HANDLE TRASH! FindExport should return if it was in a trash package, which needs to get saved into the AllPackageExports
#if 0
		// if this package isn't trash, check to see if loading it has caused any trash packages
		// to load... these packages can't be deleted for good as they are still in use
		if (!Linker->RootPackage)->PackageFlags & PKG_Trash))
		{
			for (TObjectIterator<UPackage> It; It; ++It)
			{
				if (It->PackageFlags & PKG_Trash)
				{
					if (ReferencedTrashPackages.FindItemIndex(FString(*It->GetName())) == INDEX_NONE)
					{
						warnf(NAME_Log, TEXT("   Found a referenced trash package: %s"), *It->GetName());
						new(ReferencedTrashPackages) FString(*It->GetName());
					}
				}
			}
		}
#endif
		if ((PackageIndex % 20) == 0)
		{
			UObject::CollectGarbage(RF_Native);
		}
	}
	CLEAR_WARN_COLOR();

	/////////////////////////////////////////////////////////////////////
	// Explain to user what is happening
	/////////////////////////////////////////////////////////////////////
	SET_WARN_COLOR(COLOR_YELLOW);
	warnf(NAME_Log, TEXT(""));
	warnf(NAME_Log, TEXT("Files that need to fixed up:"));
	SET_WARN_COLOR(COLOR_DARK_YELLOW);

	// print out the working set of packages
	for (INT PackageIndex = 0; PackageIndex < UpdatePackages.Num(); PackageIndex++)
	{
		warnf(NAME_Log, TEXT("   %s"), *UpdatePackages(PackageIndex));
	}

	warnf(NAME_Log, TEXT(""));
	CLEAR_WARN_COLOR();

	// if we are only testing, just just quiet before actually doing anything
	if (bIsTestOnly)
	{
		return 0;
	}

	/////////////////////////////////////////////////////////////////////
	// Find redirectors that are referenced by read-only packages
	/////////////////////////////////////////////////////////////////////

	// source control object
#if HAVE_SCC
	FSourceControlIntegration* SCC = NULL;
#endif

	// Make sure all packages are writeable!
	for( INT PackageIndex = 0; PackageIndex < UpdatePackages.Num(); PackageIndex++ )
	{
		const FFilename& Filename = UpdatePackages(PackageIndex);

		if (GFileManager->IsReadOnly(*Filename))
		{
			// initialize source control if it hasn't been initialized yet
#if HAVE_SCC
			if (!SCC)
			{
				SCC = new FSourceControlIntegration;
			}
#endif

			FString PackageName(Filename.GetBaseFilename());
#if HAVE_SCC
			if (SCC->GetSCCState(*PackageName) == SCC_ReadOnly)
			{
				SCC->CheckOut(*PackageName);
			}
#endif

			// if the checkout failed for any reason, this will still be readonly, so we can't clean it up
			if (GFileManager->IsReadOnly(*Filename))
			{
				SET_WARN_COLOR(COLOR_RED);
				warnf(NAME_Log, TEXT("Couldn't check out %s..."), *Filename);
				CLEAR_WARN_COLOR();

				// any redirectors that are pointed to by this read-only package can't be fixed up
				for (INT RedirIndex = 0; RedirIndex < AllRedirections.Num(); RedirIndex++)
				{
					FRedirection& Redir = AllRedirections(RedirIndex);

					// any redirectors pointed to by this file we don't clean
					if (Redir.PackageFilename == Filename)
					{
						RedirectorsThatCantBeCleaned.AddUniqueItem(Redir.RedirectorName);
						warnf(NAME_Log, TEXT("   ... can't fixup references to %s"), *Redir.RedirectorName);
					}
					// any redirectors in this file can't be deleted
					else if (Redir.RedirectorPackageFilename == Filename)
					{
						RedirectorsThatCantBeCleaned.AddUniqueItem(Redir.RedirectorName);
						warnf(NAME_Log, TEXT("   ... can't delete %s"), *Redir.RedirectorName);
					}
				}
			} 
			else
			{
				SET_WARN_COLOR(COLOR_DARK_GREEN);
				warnf(NAME_Log, TEXT("Checked out %s..."), *Filename);
				CLEAR_WARN_COLOR();
			}
		}
	}

	warnf(NAME_Log, TEXT(""));

	/////////////////////////////////////////////////////////////////////
	// Load and save packages to save out the proper fixed up references
	/////////////////////////////////////////////////////////////////////

	SET_WARN_COLOR(COLOR_WHITE);
	warnf(NAME_Log, TEXT("Fixing references to point to proper objects:"));
	warnf(NAME_Log, TEXT("---------------------------------------------"));
	CLEAR_WARN_COLOR();

	// keep a list of all packages that have ObjectRedirects in them 
	TArray<UBOOL> PackagesWithRedirects;
	PackagesWithRedirects.AddZeroed(UpdatePackages.Num());

	// Iterate over all packages, loading and saving to remove all references to ObjectRedirectors (can't delete the Redirects yet)
	for( INT PackageIndex = 0; PackageIndex < UpdatePackages.Num(); PackageIndex++ )
	{
		const FFilename& Filename = UpdatePackages(PackageIndex);
		
		// we can't do anything with packages that are read-only (we don't want to fixup the redirects, 
		if (GFileManager->IsReadOnly( *Filename))
		{
			continue;
		}

		warnf(NAME_Log, TEXT("Cleaning %s"), *Filename);

		// Assert if package couldn't be opened so we have no chance of messing up saving later packages.
		UPackage* Package = UObject::LoadPackage( NULL, *Filename, 0 );

		// if it failed to open, we have already dealt with quitting if we are going to, so just skip it
		if (!Package)
		{
			continue;
		}

		for (TObjectIterator<UObjectRedirector> It; It; ++It)
		{
			if (It->IsIn(Package))
			{
				PackagesWithRedirects(PackageIndex) = 1;
				break;
			}
		}

		// save out the package
		UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
		if( World )
		{	
			UObject::SavePackage( Package, World, 0, *Filename, GWarn );
		}
		else
		{
			UObject::SavePackage( Package, NULL, RF_Standalone, *Filename, GWarn );
		}

		// collect garbage to close the package
		UObject::CollectGarbage(RF_Native);
	}

	warnf(NAME_Log, TEXT(""));

	/////////////////////////////////////////////////////////////////////
	// Delete all redirects that are no longer referenced
	/////////////////////////////////////////////////////////////////////

	SET_WARN_COLOR(COLOR_WHITE);
	warnf(NAME_Log, TEXT("Deleting redirector objects:"));
	warnf(NAME_Log, TEXT("----------------------------"));
	CLEAR_WARN_COLOR();

	// Again, iterate over all packages, loading and saving to and this time deleting all 
	for( INT PackageIndex = 0; PackageIndex < UpdatePackages.Num(); PackageIndex++ )
	{
		const FFilename& Filename = UpdatePackages(PackageIndex);
		
		if (PackagesWithRedirects(PackageIndex) == 0)
			continue;

		warnf(NAME_Log, TEXT("Wiping %s..."), *Filename);

		UPackage* Package = UObject::LoadPackage( NULL, *Filename, 0 );
		// if it failed to open, we have already dealt with quitting if we are going to, so just skip it
		if (!Package)
		{
			continue;
		}

		// assume that all redirs in this file are still-referenced
		UBOOL bIsDirty = FALSE;

		// delete all ObjectRedirects now
		TArray<UObjectRedirector*> Redirs;
		// find all redirectors, put into an array so delete doesn't mess up the iterator
		for (TObjectIterator<UObjectRedirector> It; It; ++It)
		{
			if (It->IsIn(Package))
			{
				INT Index;
				// if the redirector was marked as uncleanable, skip it
				if (RedirectorsThatCantBeCleaned.FindItem(It->GetFullName(), Index))
				{
					SET_WARN_COLOR(COLOR_DARK_RED);
					warnf(NAME_Log, TEXT("   ... skipping still-referenced %s"), *It->GetFullName());
					CLEAR_WARN_COLOR();
					continue;
				}

				bIsDirty = TRUE;
				SET_WARN_COLOR(COLOR_DARK_GREEN);
				warnf(NAME_Log, TEXT("   ... deleting %s"), *It->GetFullName());
				CLEAR_WARN_COLOR();
				Redirs.AddItem(*It);
			}
		}

		// mark them for deletion.
		for (INT RedirIndex = 0; RedirIndex < Redirs.Num(); RedirIndex++)
		{
			UObjectRedirector* Redirector = Redirs(RedirIndex);
			// Remove standalone flag and mark as pending kill.
			Redirector->ClearFlags( RF_Standalone | RF_Public );
			Redirector->MarkPendingKill();
		}
		Redirs.Empty();

		// save the package
		UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
		if( World )
		{	
			UObject::SavePackage( Package, World, 0, *Filename, GWarn );
		}
		else
		{
			UObject::SavePackage( Package, NULL, RF_Standalone, *Filename, GWarn );
		}

		// collect garbage
		UObject::CollectGarbage(RF_Native);
	}

	warnf(NAME_Log, TEXT(""));

	/////////////////////////////////////////////////////////////////////
	// Clean up any unused trash
	/////////////////////////////////////////////////////////////////////

	SET_WARN_COLOR(COLOR_WHITE);
	warnf(NAME_Log, TEXT("Deleting unused packages from Trashcan:"));
	warnf(NAME_Log, TEXT("---------------------------------------"));
	CLEAR_WARN_COLOR();

	// Iterate over packages, attempting to delete trash packages
	for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
	{
		const FFilename& Filename = PackageList(PackageIndex);

		if (Filename.InStr(TRASHCAN_DIRECTORY_NAME) != -1)
		{
			FString PackageName(Filename.GetBaseFilename());
			// if this package wasn't referenced, then we can delete it!
			if (ReferencedTrashPackages.FindItemIndex(PackageName) == INDEX_NONE)
			{
				// initialize source control if it hasn't been initialized yet
#if HAVE_SCC
				if (!SCC)
				{
					SCC = new FSourceControlIntegration;
				}

				// get file SCC status
				ESourceControlState SCCState = SCC->GetSCCState(*PackageName);

				// remove it now that we've closed it
				switch (SCCState)
				{
					// if we are checked out, then undo the checkout and then fall through to the normal case					
					case SCC_CheckedOut:
						SCC->UncheckOut(*PackageName);
						// FALLING THROUGH TO NEXT CASE!

					// delete from source control
					case SCC_ReadOnly:
					case SCC_NotCurrent:
						SET_WARN_COLOR(COLOR_GREEN);
						warnf(NAME_Log, TEXT("Deleting '%s' from source control..."), *Filename);
						CLEAR_WARN_COLOR();
						SCC->Remove(*PackageName);
						break;

					// someone else has it checked out, so we can't do anything about it
					case SCC_CheckedOutOther:
						SET_WARN_COLOR(COLOR_RED);
						warnf(NAME_Log, TEXT("Couldn't delete '%s' from source control, someone has it checked out, skipping..."), *Filename);
						CLEAR_WARN_COLOR();
						break;
					

					// this file was never added to source control, so just delete it
					case SCC_NotInDepot:
						SET_WARN_COLOR(COLOR_GREEN);
						warnf(NAME_Log, TEXT("'%s' is not in source control, attempting to delete from disk..."), *Filename);
						if (!GFileManager->Delete(*Filename, FALSE, TRUE))
						{
							SET_WARN_COLOR(COLOR_RED);
							warnf(NAME_Log, TEXT("  ... failed to delete from disk."), *Filename);
						}
						CLEAR_WARN_COLOR();
						break;

					// not sure, so try to delete from disk
					case SCC_DontCare:
					case SCC_Ignore:
						SET_WARN_COLOR(COLOR_GREEN);
						warnf(NAME_Log, TEXT("'%s' is in an unknown source control state, attempting to delete from disk..."), *Filename);
						if (!GFileManager->Delete(*Filename, FALSE, TRUE))
						{
							SET_WARN_COLOR(COLOR_RED);
							warnf(NAME_Log, TEXT("  ... failed to delete from disk."), *Filename);
						}
						CLEAR_WARN_COLOR();
						break;
				}
#else  // No SCC support in this build.
				SET_WARN_COLOR(COLOR_GREEN);
				warnf(NAME_Log, TEXT("'%s' is in an unknown source control state, attempting to delete from disk..."), *Filename);
				if (!GFileManager->Delete(*Filename, FALSE, TRUE))
				{
					SET_WARN_COLOR(COLOR_RED);
					warnf(NAME_Log, TEXT("  ... failed to delete from disk."), *Filename);
				}
				CLEAR_WARN_COLOR();
#endif
			}
			else
			{
				SET_WARN_COLOR(COLOR_RED);
				warnf(NAME_Log, TEXT("Unable to delete %s from the trash, it's still referenced."), *Filename);
				CLEAR_WARN_COLOR();
			}
		}
	}

#if HAVE_SCC
	// toss the SCC manager
	delete SCC;
#endif

	return 0;
}
IMPLEMENT_CLASS(UFixupRedirectsCommandlet)


#endif

