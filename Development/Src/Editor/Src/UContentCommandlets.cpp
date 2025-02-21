/*=============================================================================
	UCContentCommandlets.cpp: Various commmandlets.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "EngineUIPrivateClasses.h"

#include "EngineAnimClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineAudioDeviceClasses.h"
#include "EngineSoundClasses.h"
#include "UnOctree.h"
#include "../../UnrealEd/Inc/SourceControlIntegration.h"

#include "PackageHelperFunctions.h"
#include "EnginePrefabClasses.h"


/*
Below is a template commandlet than can be used when you want to perform an operation on all packages.

INT UPerformAnOperationOnEveryPackage::Main(const FString& Params)
{
	// Parse command line args.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	const TCHAR* Parms = *Params;
	ParseCommandLine(Parms, Tokens, Switches);

	// Build package file list.
	const TArray<FString> FilesInPath( GPackageFileCache->GetPackageFileList() );
	if( FilesInPath.Num() == 0 )
	{
		warnf( NAME_Warning, TEXT("No packages found") );
		return 1;
	}

	// Iterate over all files doing stuff.
	for( INT FileIndex = 0 ; FileIndex < FilesInPath.Num() ; ++FileIndex )
	{
		const FFilename& Filename = FilesInPath(FileIndex);
		warnf( NAME_Log, TEXT("Loading %s"), *Filename );

		UPackage* Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
		if( Package == NULL )
		{
			warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
		}

		/////////////////
		//
		// Do your thing here
		//
		/////////////////

		TObjectIterator<UStaticMesh> It;...

		UStaticMesh* StaticMesh = *It;
		if( StaticMesh->IsIn( Package )




		UObject::CollectGarbage( RF_Native );
	}

	return 0;
}
*/
/*-----------------------------------------------------------------------------
	UCutDownContentCommandlet commandlet.
-----------------------------------------------------------------------------*/
/**
 * Allows commandlets to override the default behavior and create a custom engine class for the commandlet. If
 * the commandlet implements this function, it should fully initialize the UEngine object as well.  Commandlets
 * should indicate that they have implemented this function by assigning the custom UEngine to GEngine.
 */
void UCutDownContentCommandlet::CreateCustomEngine()
{
	UClass* EditorEngineClass	= UObject::StaticLoadClass( UEditorEngine::StaticClass(), NULL, TEXT("engine-ini:Engine.Engine.EditorEngine"), NULL, LOAD_None | LOAD_DisallowFiles, NULL );

	// must do this here so that the engine object that we create on the next line receives the correct property values
	UObject* DefaultEngine		= EditorEngineClass->GetDefaultObject(TRUE);

	// ConditionalLink normally doesn't do anything for non-intrinsic classes (since Link should be called from UClass::Serialize in those cases),
	// but since we loaded the editor class using the LOAD_DisallowFiles file, Serialize won't be called.  Temporarily allow these non-intrinsic classes
	// to be linked so that the commandlet can operate properly
	GUglyHackFlags |= HACK_ClassLoadingDisabled;;
	EditorEngineClass->ConditionalLink();
	GUglyHackFlags &= ~HACK_ClassLoadingDisabled;

	GEngine = GEditor			= ConstructObject<UEditorEngine>( EditorEngineClass );
	GEditor->InitEditor();
}

INT UCutDownContentCommandlet::Main( const FString& Params )
{
	const TCHAR* Parms = *Params;

	// Retrieve list of all packages in .ini paths.
	TArray<FString> PackageList = GPackageFileCache->GetPackageFileList();
	if( !PackageList.Num() )
		return 0;

	// Create directories for cutdown files.
	GFileManager->MakeDirectory( *FString::Printf(TEXT("..%sCutdownPackages%sPackages"	),PATH_SEPARATOR,PATH_SEPARATOR), 1 );
	GFileManager->MakeDirectory( *FString::Printf(TEXT("..%sCutdownPackages%sMaps"		),PATH_SEPARATOR,PATH_SEPARATOR), 1 );

	// Create a list of packages to base cutdown on.
	FString				PackageName;
	TArray<FString>		BasePackages;
	while( ParseToken( Parms, PackageName, 0 ) )
	{
		new(BasePackages)FString(FFilename(PackageName).GetBaseFilename());
	}

#if 1
	// Load all .u files
	for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
	{
		const FString &Filename = PackageList(PackageIndex);
		UObject* Package;

		if( appStricmp( *FFilename(Filename).GetExtension(), TEXT("u")) == 0 )
		{
			Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
			if( !Package )
			{
				GWarn->Logf(NAME_Log, TEXT("Error loading %s!"), *Filename);
				continue;
			}
		}
		else
			continue;
	}

	UObject::ResetLoaders( NULL );
#endif

	// Load all specified packages/ maps.
	for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
	{
		FFilename		Filename			= FFilename(PackageList(PackageIndex));
		INT				BasePackageIndex	= 0;			
		UPackage*		Package;

		// Find package.
		for( BasePackageIndex=0; BasePackageIndex<BasePackages.Num(); BasePackageIndex++ )
		{
			if( appStricmp( *Filename.GetBaseFilename(), *BasePackages(BasePackageIndex)) == 0 )
			{
				break;
			}
		}

		if( BasePackageIndex == BasePackages.Num() && BasePackageIndex )
		{
			continue;
		}

		UObject::CollectGarbage( RF_Native | RF_Standalone );

		Package = UObject::LoadPackage( NULL, *Filename, 0 );

		if( !Package )
		{
			GWarn->Logf(NAME_Log, TEXT("Error loading %s!"), *Filename);
			continue;
		}
		INT Total = PackageList.Num();
		GWarn->Logf(NAME_Log, TEXT("Loaded %s (%d/%d)"), *Filename,PackageIndex,Total);

		// Saving worlds.
		UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
		if( World )
		{
			FString NewMapName = FString::Printf(TEXT("..%sCutdownPackages%sMaps%s%s.%s"),PATH_SEPARATOR,PATH_SEPARATOR,PATH_SEPARATOR,*Filename.GetBaseFilename(),*Filename.GetExtension());
			UObject::SavePackage( Package, World, 0, *NewMapName, GWarn );

		}
		else
		{	
			warnf(NAME_Log, TEXT("Error loading %s! (no UWorld)"), *Filename);
		}
	}

	GWarn->Logf(NAME_Log, TEXT("Done loading. Now Saving."));

	// Save cutdown packages.
	for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
	{
		FFilename		Filename	= FFilename(PackageList(PackageIndex));
		UPackage*		Package		= FindObject<UPackage>( NULL, *Filename.GetBaseFilename() );

		// strip out editor meta data (source textures, etc) if desired
		if (ParseParam(*Params, TEXT("strip")))
		{
			for (TObjectIterator<UObject> It; It; ++It)
			{
				if (It->IsIn(Package))
				{
					It->StripData(UE3::PLATFORM_Unknown);
				}
			}
		}

		// Don't save EnginePackages.
		if( Filename.ToUpper().InStr( TEXT("ENGINEPACKAGES") ) != INDEX_NONE )
			continue;

		if( Filename.ToUpper().InStr( TEXT("ENGINE\\CONTENT") ) != INDEX_NONE )
			continue;
/*
		if( Filename.ToUpper().InStr( TEXT("UTGAME\\CONTENT") ) != INDEX_NONE )
			continue;		
*/
		// Only save .upk's that are currently loaded.
		if( !Package || appStricmp( *Filename.GetExtension(), TEXT("upk")) != 0 )
			continue;

		GWarn->Logf(NAME_Log, TEXT("Saving %s"),*Filename);

		FString NewPackageName = FString::Printf(TEXT("..%sCutdownPackages%sPackages%s%s.%s"),PATH_SEPARATOR,PATH_SEPARATOR,PATH_SEPARATOR,*Filename.GetBaseFilename(),*Filename.GetExtension());
		SavePackage( Package, NULL, RF_Standalone, *NewPackageName, NULL );
	}

	return 0;
}

IMPLEMENT_CLASS(UCutDownContentCommandlet);

/**-----------------------------------------------------------------------------
 *	UResavePackages commandlet.
 *
 * This commandlet is meant to resave packages as a default.  We are able to pass in
 * flags to determine which conditions we do NOT want to resave packages. (e.g. not dirty
 * or not older than some version)
 *
 *
----------------------------------------------------------------------------**/

#define CURRENT_PACKAGE_VERSION 0
#define IGNORE_PACKAGE_VERSION INDEX_NONE

/**
 * Evalutes the command-line to determine which maps to check.  By default all maps are checked (except PIE and trash-can maps)
 * Provides child classes with a chance to initialize any variables, parse the command line, etc.
 *
 * @param	Tokens			the list of tokens that were passed to the commandlet
 * @param	Switches		the list of switches that were passed on the commandline
 * @param	PackageNames	receives the list of path names for the packa that will be checked.
 *
 * @return	0 to indicate that the commandlet should continue; otherwise, the error code that should be returned by Main()
 */
INT UResavePackagesCommandlet::InitializeResaveParameters( const TArray<FString>& Tokens, const TArray<FString>& Switches, TArray<FFilename>& PackageNames )
{
	TArray<FString> Unused, PackageExtensions;
	UBOOL bExplicitPackages = FALSE;
	GConfig->GetArray( TEXT("Core.System"), TEXT("Extensions"), PackageExtensions, GEngineIni );

	// Check to see if we have an explicit list of packages
	for( INT SwitchIdx = 0; SwitchIdx < Switches.Num(); SwitchIdx++ )
	{
		FString Package;
		const FString& CurrentSwitch = Switches( SwitchIdx );
		if( Parse( *CurrentSwitch, TEXT( "PACKAGE="), Package ) )
		{
			FFilename PackageFile;
			GPackageFileCache->FindPackageFile( *Package, NULL, PackageFile, NULL );
			PackageNames.AddItem( *PackageFile );
			bExplicitPackages = TRUE;
		}
	}

	// ... if not, load in all packages
	if( !bExplicitPackages )
	{
		BYTE PackageFilter = NORMALIZE_DefaultFlags;
		if ( Switches.ContainsItem(TEXT("SKIPMAPS")) )
		{
			PackageFilter |= NORMALIZE_ExcludeMapPackages;
		}
		else if ( Switches.ContainsItem(TEXT("MAPSONLY")) )
		{
			PackageFilter |= NORMALIZE_ExcludeContentPackages;
		}

		for ( INT ExtensionIndex = 0; ExtensionIndex < PackageExtensions.Num(); ExtensionIndex++ )
		{
			if ( PackageExtensions(ExtensionIndex) != TEXT("U") )
			{
				if ( !NormalizePackageNames(Unused, PackageNames, FString(TEXT("*.")) + PackageExtensions(ExtensionIndex), PackageFilter) )
				{
					return 1;
				}
			}
		}
	}

	MinResaveVersion = MaxResaveVersion = IGNORE_PACKAGE_VERSION;
	if ( Switches.ContainsItem(TEXT("CHECKVER")) )
	{
		// only resave packages with old versions
		MaxResaveVersion = GPackageFileVersion - 1;
	}
	else
	{

		// determine if the resave operation should be constrained to certain package versions
		for ( INT SwitchIdx = 0; SwitchIdx < Switches.Num(); SwitchIdx++ )
		{
			const FString& CurrentSwitch = Switches(SwitchIdx);
			if ( MinResaveVersion == IGNORE_PACKAGE_VERSION && Parse(*CurrentSwitch,TEXT("-MINVER="),MinResaveVersion) )
			{
				if ( MinResaveVersion == CURRENT_PACKAGE_VERSION )
				{
					MinResaveVersion = GPackageFileVersion;
				}
			}

			if ( MaxResaveVersion == IGNORE_PACKAGE_VERSION && Parse(*CurrentSwitch,TEXT("-MAXVER="),MaxResaveVersion) )
			{
				if ( MaxResaveVersion == CURRENT_PACKAGE_VERSION )
				{
					MaxResaveVersion = GPackageFileVersion;
				}
			}
		}
	}

	FString ClassList;
	for ( INT SwitchIdx = 0; SwitchIdx < Switches.Num(); SwitchIdx++ )
	{
		const FString& CurrentSwitch = Switches(SwitchIdx);
		if ( Parse(*CurrentSwitch, TEXT("RESAVECLASS="), ClassList) )
		{
			TArray<FString> ClassNames;
			ClassList.ParseIntoArray(&ClassNames, TEXT(","), TRUE);
			for ( INT Idx = 0; Idx < ClassNames.Num(); Idx++ )
			{
				ResaveClasses.AddItem(*ClassNames(Idx));
			}

			break;
		}
	}

	return 0;
}

INT UResavePackagesCommandlet::Main( const FString& Params )
{
	const TCHAR* Parms = *Params;
	TArray<FString> Tokens, Switches;
	ParseCommandLine(Parms, Tokens, Switches);

#if HAVE_SCC
	TScopedPointer<FSourceControlIntegration> SCC(new FSourceControlIntegration);
#endif

	// skip the assert when a package can not be opened
	const UBOOL bCanIgnoreFails				= Switches.ContainsItem(TEXT("SKIPFAILS"));
	/** load all packages, and display warnings for those packages which would have been resaved but were read-only */
	const UBOOL bVerifyContent				= Switches.ContainsItem(TEXT("VERIFY"));
	/** if we should only save dirty packages **/
	const UBOOL bOnlySaveDirtyPackages		= Switches.ContainsItem(TEXT("OnlySaveDirtyPackages"));
	/** if we should auto checkout packages that need to be saved**/
	const UBOOL bAutoCheckOut				= Switches.ContainsItem(TEXT("AutoCheckOutPackages"));

	TArray<FFilename> PackageNames;
	INT ResultCode = InitializeResaveParameters(Tokens, Switches, PackageNames);
	if ( ResultCode != 0 )
	{
		return ResultCode;
	}

	// Retrieve list of all packages in .ini paths.
	if( !PackageNames.Num() )
	{
		return 0;
	}

	INT GCIndex = 0;

	// Iterate over all packages.
	for( INT PackageIndex = 0; PackageIndex < PackageNames.Num(); PackageIndex++ )
	{
		// Make sure we don't rebuild SMs that we're not going to save.
		extern FName GStaticMeshPackageNameToRebuild;
		GStaticMeshPackageNameToRebuild = NAME_None;

		const FFilename& Filename = PackageNames(PackageIndex);

		// we don't care about trying to resave the various shader caches so just skipz0r them
		if(	Filename.InStr( TEXT("LocalShaderCache") ) != INDEX_NONE
		|| Filename.InStr( TEXT("RefShaderCache") ) != INDEX_NONE )
		{
			continue;
		}

		UBOOL bIsReadOnly = GFileManager->IsReadOnly( *Filename);
		if( Filename.GetExtension() == TEXT("U") )
		{
			warnf(NAME_Log, TEXT("Skipping script file %s"), *Filename);
		}
		else if ( bIsReadOnly && !bVerifyContent && !bAutoCheckOut )
		{
			warnf(NAME_Log, TEXT("Skipping read-only file %s"), *Filename);
		}
		else
		{
			warnf(NAME_Log, TEXT("Loading %s"), *Filename);

			INT NumErrorsFromLoading = GWarn->Errors.Num();
			UBOOL bSavePackage = TRUE;

			// Get the package linker.
			UObject::BeginLoad();
			ULinkerLoad* Linker = UObject::GetPackageLinker(NULL,*Filename,LOAD_NoVerify,NULL,NULL);
			UObject::EndLoad();

			PerformPreloadOperations(Linker, bSavePackage);

			if(bSavePackage)
			{
				// Only rebuild static meshes on load for the to be saved package.
				GStaticMeshPackageNameToRebuild = FName(*Filename.GetBaseFilename());

				// Assert if package couldn't be opened so we have no chance of messing up saving later packages.
				UPackage* Package = UObject::LoadPackage( NULL, *Filename, 0 );
				if (Package == NULL)
				{
					if( bCanIgnoreFails == TRUE )
					{
						continue;
					}
					else
					{
						check(Package);
					}
				}

				// if we are only saving dirty packages and the package is not dirty, then we do not want to save the package (remember the default behavior is to ALWAYS save the package)
				if( ( bOnlySaveDirtyPackages == TRUE ) && ( Package->IsDirty() == FALSE ) )
				{
					bSavePackage = FALSE;
				}

				// here we want to check and see if we have any loading warnings
				// if we do then we want to resave this package
				if( !bSavePackage && ParseParam(appCmdLine(),TEXT("SavePackagesThatHaveFailedLoads")) == TRUE )
				{
					//warnf( TEXT( "NumErrorsFromLoading: %d GWarn->Errors num: %d" ), NumErrorsFromLoading, GWarn->Errors.Num() );

					if( NumErrorsFromLoading != GWarn->Errors.Num() )
					{
						bSavePackage = TRUE;
					}
				}


				// hook to allow performing additional checks without lumping everything into this one function
				PerformAdditionalOperations(Package,bSavePackage);

				// Check for any special per object operations
				for( FObjectIterator ObjectIt; ObjectIt; ++ObjectIt )
				{
					if( ObjectIt->IsIn( Package ) )
					{
						PerformAdditionalOperations( *ObjectIt, bSavePackage );
					}
				}


				// Now based on the computation above we will see if we should actually attempt
				// to save this package
				if( bSavePackage == TRUE )
				{
					if( bIsReadOnly == TRUE && bVerifyContent == TRUE && bAutoCheckOut == FALSE )
					{
						warnf(NAME_Log,TEXT("Package [%s] is read-only but needs to be resaved (Package's Version: %i  Current Version: %i)"), *Filename, Linker->Summary.GetFileVersion(), VER_LATEST_ENGINE );
					}
					else
					{
						// check to see if we need to check this package out
#if HAVE_SCC
						if( bIsReadOnly == TRUE && bAutoCheckOut == TRUE )
						{
							SCC->CheckOut(Package);
						}
#endif

						// so now we need to see if we actually were able to check this file out
						// if the file is still read only then we failed and need to emit an error and go to the next package
						if( GFileManager->IsReadOnly( *Filename ) == TRUE )
						{
							warnf( NAME_Error, TEXT("Unable to check out the Package: %s"), *Filename );
							continue;
						}


						warnf(NAME_Log,TEXT("Resaving package [%s] (Package's Version: %i  Saved Version: %i)"), *Filename, Linker->Summary.GetFileVersion(), VER_LATEST_ENGINE );
						if( SavePackageHelper(Package, Filename) )
						{
							warnf( NAME_Log, TEXT("Correctly saved:  [%s]."), *Filename );
						}
					}
				}
			}

			UObject::CollectGarbage(RF_Native);
		}

		// Potentially save the local shader cache to include this packages' shaders.
		SaveLocalShaderCaches();
	}

	return 0;
}

/**
 * Allow the commandlet to perform any operations on the export/import table of the package before all objects in the package are loaded.
 *
 * @param	PackageLinker	the linker for the package about to be loaded
 * @param	bSavePackage	[in]	indicates whether the package is currently going to be saved
 *							[out]	set to TRUE to resave the package
 */
UBOOL UResavePackagesCommandlet::PerformPreloadOperations( ULinkerLoad* PackageLinker, UBOOL& bSavePackage )
{
	UBOOL bResult = FALSE;

	const INT PackageVersion = PackageLinker->Summary.GetFileVersion();

	// validate that this package meets the minimum requirement
	if ( MinResaveVersion != IGNORE_PACKAGE_VERSION && PackageVersion < MinResaveVersion )
	{
		bSavePackage = FALSE;
		bResult = TRUE;
	}

	// make sure this package meets the maximum requirement
	if ( MaxResaveVersion != IGNORE_PACKAGE_VERSION && PackageVersion > MaxResaveVersion )
	{
		bSavePackage = FALSE;
		bResult = TRUE;
	}

	// Check if the package contains any instances of the class that needs to be resaved.
	if ( bSavePackage && ResaveClasses.Num() > 0 )
	{
		bSavePackage = FALSE;
		for (INT ExportIndex = 0; ExportIndex < PackageLinker->ExportMap.Num(); ExportIndex++)
		{
			if ( ResaveClasses.HasKey(PackageLinker->GetExportClassName(ExportIndex)) )
			{
				bSavePackage = TRUE;
				break;
			}
		}

		bResult = TRUE;
	}

	return bResult;
}

/**
 * Allows the commandlet to perform any additional operations on the object before it is resaved.
 *
 * @param	Object			the object in the current package that is currently being processed
 * @param	bSavePackage	[in]	indicates whether the package is currently going to be saved
 *							[out]	set to TRUE to resave the package
 */
void UResavePackagesCommandlet::PerformAdditionalOperations( class UObject* Object, UBOOL& bSavePackage )
{
	check( Object );
	UBOOL bShouldSavePackage = FALSE;

	if( Object->IsA( USoundNodeWave::StaticClass() ) )
	{
		bShouldSavePackage = CookSoundNodeWave( ( USoundNodeWave* )Object );
	}

	// add additional operations here

	bSavePackage = bSavePackage || bShouldSavePackage;
}

/**
 * Allows the commandlet to perform any additional operations on the package before it is resaved.
 *
 * @param	Package			the package that is currently being processed
 * @param	bSavePackage	[in]	indicates whether the package is currently going to be saved
 *							[out]	set to TRUE to resave the package
 */
void UResavePackagesCommandlet::PerformAdditionalOperations( UPackage* Package, UBOOL& bSavePackage )
{
	check(Package);
	UBOOL bShouldSavePackage = FALSE;
	
	if( ( ParseParam(appCmdLine(), TEXT("CLEANCLASSES")) == TRUE ) && ( CleanClassesFromContentPackages(Package) == TRUE ) )
	{
		bShouldSavePackage = TRUE;
	}
	else if( ( ParseParam(appCmdLine(), TEXT("INSTANCEMISSINGSUBOBJECTS")) == TRUE ) && ( InstanceMissingSubObjects(Package) == TRUE ) )
	{
		bShouldSavePackage = TRUE;
	}

	// add additional operations here

	bSavePackage = bSavePackage || bShouldSavePackage;
}

/**
 * Removes any UClass exports from packages which aren't script packages.
 *
 * @param	Package			the package that is currently being processed
 *
 * @return	TRUE to resave the package
 */
UBOOL UResavePackagesCommandlet::CleanClassesFromContentPackages( UPackage* Package )
{
	check(Package);
	UBOOL bResult = FALSE;

	for ( TObjectIterator<UClass> It; It; ++It )
	{
		if ( It->IsIn(Package) )
		{
			warnf(NAME_Log, TEXT("Removing class '%s' from package [%s]"), *It->GetPathName(), *Package->GetName());

			// mark the class as transient so that it won't be saved into the package
			It->SetFlags(RF_Transient);

			// clear the standalone flag just to be sure :)
			It->ClearFlags(RF_Standalone);
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Instances subobjects for any existing objects with subobject properties pointing to the default object.
 * This is currently the case when a class has an object property and subobject definition added to it --
 * existing instances of such a class will see the new object property refer to the template object.
 *
 * @param	Package			The package that is currently being processed.
 *
 * @return					TRUE to resave the package.
 */
UBOOL UResavePackagesCommandlet::InstanceMissingSubObjects( UPackage* Package )
{
	check( Package );
	UBOOL bResult = FALSE;

	for ( TObjectIterator<UObject> It; It; ++It )
	{
		if ( It->IsIn( Package ) )
		{
			if ( !It->IsTemplate() )
			{
				for ( TFieldIterator<UProperty> ItP(It->GetClass()) ; ItP ; ++ItP )
				{
					UProperty* Property = *ItP;
					if ( Property->PropertyFlags & CPF_NeedCtorLink )
					{
						UBOOL bAlreadyLogged = FALSE;
						for ( INT i = 0 ; i < Property->ArrayDim ; ++i )
						{
							FString	Value;
							if ( Property->ExportText( i, Value, (BYTE*)*It, (BYTE*)*It, *It, PPF_Localized ) )
							{
								UObject* SubObject = UObject::StaticFindObject( UObject::StaticClass(), ANY_PACKAGE, *Value );
								if ( SubObject )
								{
									if ( SubObject->IsTemplate() )
									{
										// The heap memory allocated at this address is owned the source object, so
										// zero out the existing data so that the UProperty code doesn't attempt to
										// deallocate it before allocating the memory for the new value.
										appMemzero( (BYTE*)*It + Property->Offset, Property->GetSize() );
										Property->CopyCompleteValue( (BYTE*)*It + Property->Offset, &SubObject, *It/*NULL*/, *It );
										bResult = TRUE;
										if ( !bAlreadyLogged )
										{
											bAlreadyLogged = TRUE;
											if ( Property->ArrayDim == 1 )
											{
												warnf( NAME_Log, TEXT("Instancing %s::%s"), *It->GetPathName(), *Property->GetName() );
											}
											else
											{
												warnf( NAME_Log, TEXT("Instancing %s::%s[%i]"), *It->GetPathName(), *Property->GetName(), i );
											}
											
										}
									}
								}
							}
						} // Property->ArrayDim
					} // If property NeedCtorLink
				} // Field iterator
			} // if It->IsTemplate
		} // If in package
	} // Object iterator

	return bResult;
}

IMPLEMENT_CLASS(UResavePackagesCommandlet);


/**
 * Allow the commandlet to perform any operations on the export/import table of the package before all objects in the package are loaded.
 *
 * @param	PackageLinker	the linker for the package about to be loaded
 * @param	bSavePackage	[in]	indicates whether the package is currently going to be saved
 *							[out]	set to TRUE to resave the package
 */
UBOOL UChangePrefabSequenceClassCommandlet::PerformPreloadOperations( ULinkerLoad* PackageLinker, UBOOL& bSavePackage )
{
	UBOOL bResult = Super::PerformPreloadOperations(PackageLinker, bSavePackage);
	if ( bResult && bSavePackage )
	{
		bSavePackage = FALSE;

		PACKAGE_INDEX EnginePackageIndex = 0;
		PACKAGE_INDEX SequenceClassIndex = 0;
		PACKAGE_INDEX PrefabSeqContainerClassImportIndex=0;
		PACKAGE_INDEX PrefabSeqClassImportIndex=0;
		PACKAGE_INDEX PrefabClassIndex = 0;

		UClass* PrefabSeqContainerClass = UPrefabSequenceContainer::StaticClass();
		UClass* PrefabSeqClass = UPrefabSequence::StaticClass();

		static const FName SequenceClassName = USequence::StaticClass()->GetFName();
		static const FName PrefabSeqContainerClassName = PrefabSeqContainerClass->GetFName();
		static const FName PrefabSeqClassName = PrefabSeqClass->GetFName();
		static const FName PrefabSequenceName = PREFAB_SEQCONTAINER_NAME;
		static const FName PrefabClassName = UPrefab::StaticClass()->GetFName();

		// if we're here, we know that this package contains at least one prefab instance
		for ( INT ImportIndex = 0; ImportIndex < PackageLinker->ImportMap.Num(); ImportIndex++ )
		{
			FObjectImport& Import = PackageLinker->ImportMap(ImportIndex);
			if ( Import.ClassName == NAME_Class )
			{
				if ( Import.ObjectName == PrefabSeqContainerClassName )
				{
					PrefabSeqContainerClassImportIndex = -ImportIndex-1;
				}
				else if ( Import.ObjectName == PrefabSeqClassName )
				{
					PrefabSeqClassImportIndex = -ImportIndex-1;
				}
				else if ( Import.ObjectName == SequenceClassName )
				{
					SequenceClassIndex = -ImportIndex-1;
				}
				else if ( Import.ObjectName == PrefabClassName )
				{
					PrefabClassIndex = -ImportIndex-1;
				}
			}
			else if ( Import.ClassName == NAME_Package && Import.ObjectName == NAME_Engine )
			{
				EnginePackageIndex = -ImportIndex-1;
			}
		}

		if ( PrefabSeqContainerClassImportIndex == 0 )
		{
			// add it
			check(EnginePackageIndex != 0);
			PrefabSeqContainerClassImportIndex = -PackageLinker->ImportMap.Num()-1;
			FObjectImport* ContainerClassImport = new(PackageLinker->ImportMap) FObjectImport(NULL);
			ContainerClassImport->ObjectName = PrefabSeqContainerClassName;
			ContainerClassImport->OuterIndex = EnginePackageIndex;
			ContainerClassImport->ClassName = NAME_Class;
			ContainerClassImport->ClassPackage = NAME_Core;

			FObjectImport* ContainerCDO = new(PackageLinker->ImportMap) FObjectImport(NULL);
			ContainerCDO->ObjectName = PrefabSeqContainerClass->GetDefaultObject()->GetFName();
			ContainerCDO->OuterIndex = EnginePackageIndex;
			ContainerCDO->ClassName = PrefabSeqContainerClassName;
			ContainerCDO->ClassPackage = NAME_Engine;
			bResult = TRUE;
		}

		if ( PrefabSeqClassImportIndex == 0 )
		{
			check(EnginePackageIndex!=0);
			check(PrefabSeqContainerClassImportIndex!=0);

			PrefabSeqClassImportIndex = -PackageLinker->ImportMap.Num() - 1;
			FObjectImport* SeqClassImport = new(PackageLinker->ImportMap) FObjectImport(NULL);
			SeqClassImport->ObjectName = PrefabSeqClassName;
			SeqClassImport->OuterIndex = EnginePackageIndex;
			SeqClassImport->ClassName = NAME_Class;
			SeqClassImport->ClassPackage = NAME_Core;

			FObjectImport* PrefSeqCDO = new(PackageLinker->ImportMap) FObjectImport(NULL);
			PrefSeqCDO->ObjectName = PrefabSeqClass->GetDefaultObject()->GetFName();
			PrefSeqCDO->OuterIndex = EnginePackageIndex;
			PrefSeqCDO->ClassName = PrefabSeqClassName;
			PrefSeqCDO->ClassPackage = NAME_Engine;
			bResult = TRUE;
		}

		// it's possible that none of the PrefabInstance or Prefabs actually have a sequence, in which case the sequence class might
		// not necessarily be in the import map
		if ( SequenceClassIndex != 0 )
		{
			warnf(TEXT("Performing sequence class fixup for %s"), *PackageLinker->Filename);
			for ( INT ExportIndex = 0; ExportIndex < PackageLinker->ExportMap.Num(); ExportIndex++ )
			{
				FObjectExport& Export = PackageLinker->ExportMap(ExportIndex);
				if ( Export.ClassIndex == SequenceClassIndex )
				{
					if ( Export.ObjectName == PrefabSequenceName )
					{
						warnf(TEXT("\tFound export for Prefabs sequence (%s).  Changing class to PrefabSequenceContainer (%i)."), 
							*PackageLinker->GetExportPathName(ExportIndex), -PrefabSeqContainerClassImportIndex-1);

						Export.ClassIndex = PrefabSeqContainerClassImportIndex;
						bResult = bSavePackage = TRUE;
					}
					else
					{
						FObjectExport& OuterExport = PackageLinker->ExportMap(Export.OuterIndex - 1);
						if ( OuterExport.ObjectName == PrefabSequenceName || OuterExport.ClassIndex == PrefabClassIndex )
						{
							warnf(TEXT("\tFound export for %s sequence (%s).  Changing class to PrefabSequence (%i)."), 
								OuterExport.ClassIndex == PrefabClassIndex ? TEXT("Prefab") : TEXT("PrefabInstance"),
								*PackageLinker->GetExportPathName(ExportIndex), -PrefabSeqClassImportIndex-1);

							Export.ClassIndex = PrefabSeqClassImportIndex;
							bResult = bSavePackage = TRUE;
						}
					}
				}
			}
		}
	}

	return bResult;
}

/**
 * Allows the commandlet to perform any additional operations on the object before it is resaved.
 *
 * @param	Object			the object in the current package that is currently being processed
 * @param	bSavePackage	[in]	indicates whether the package is currently going to be saved
 *							[out]	set to TRUE to resave the package
 */
void UChangePrefabSequenceClassCommandlet::PerformAdditionalOperations( UObject* Object, UBOOL& bSavePackage )
{
	if ( Object->GetClass() == APrefabInstance::StaticClass() )
	{
		APrefabInstance* PrefInst = Cast<APrefabInstance>(Object);
		if ( PrefInst->SequenceInstance != NULL && PrefInst->SequenceInstance->GetOwnerPrefab() == NULL )
		{
			PrefInst->SequenceInstance->SetOwnerPrefab(PrefInst);
			bSavePackage = TRUE;
		}
	}
}

IMPLEMENT_CLASS(UChangePrefabSequenceClassCommandlet);

/*-----------------------------------------------------------------------------
	UScaleAudioVolumeCommandlet commandlet.
-----------------------------------------------------------------------------*/

INT UScaleAudioVolumeCommandlet::Main( const FString& Params )
{
	const TCHAR* Parms = *Params;

	// Retrieve list of all packages in .ini paths.
	TArray<FString> PackageList = GPackageFileCache->GetPackageFileList();
	if( !PackageList.Num() )
		return 0;

	// Read the scale amount from the commandline
	FLOAT VolumeScale = 1.f;
	if ( !Parse(Parms, TEXT("VolumeScale="), VolumeScale) )
	{
		warnf(NAME_Log, TEXT("Failed to parse volume scale"));
		return 0;
	}

	TArray<FString> SoundPackages;
#if HAVE_SCC
	FSourceControlIntegration* SCC = new FSourceControlIntegration;
#endif

	// Iterate over all packages.
	for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
	{
		const FFilename& Filename = PackageList(PackageIndex);

		if( Filename.GetExtension() == TEXT("U") )
		{
			warnf(NAME_Log, TEXT("Skipping script file %s"), *Filename);
		}
		else
		{
			warnf(NAME_Log, TEXT("Loading %s"), *Filename);

			// Assert if package couldn't be opened so we have no chance of messing up saving later packages.
			UPackage* Package = UObject::LoadPackage( NULL, *Filename, 0 );
			if (Package != NULL)
			{
				// Iterate over all SoundNodeWave objects
				UBOOL bHasSounds = FALSE;
				warnf(NAME_Log, TEXT("Looking for wave nodes..."));
				for (TObjectIterator<USoundNodeWave> NodeIt; NodeIt; ++NodeIt)
				{
					if (NodeIt->IsIn(Package))
					{
						bHasSounds = TRUE;
						// scale the volume
						NodeIt->Volume = NodeIt->Volume * VolumeScale;
					}
				}
				if ( bHasSounds )
				{
					if (GFileManager->IsReadOnly(*Filename))
					{
#if HAVE_SCC
						SCC->CheckOut(Package);
#endif

						if (GFileManager->IsReadOnly(*Filename))
						{
							SoundPackages.AddItem(Package->GetFullName());
							continue;
						}
					}

					// resave the package
					SavePackageHelper(Package, Filename);
				}
			}
		}

		UObject::CollectGarbage(RF_Native);
	}

	for (INT Idx = 0; Idx < SoundPackages.Num(); Idx++)
	{
		warnf(TEXT("Failed to save sound package %s"),*SoundPackages(Idx));
	}

#if HAVE_SCC
	delete SCC; // clean up our allocated SCC
#endif

	return 0;
}
IMPLEMENT_CLASS(UScaleAudioVolumeCommandlet)

/*-----------------------------------------------------------------------------
	UConformCommandlet commandlet.
-----------------------------------------------------------------------------*/

INT UConformCommandlet::Main( const FString& Params )
{
	GIsRequestingExit=TRUE;

	const TCHAR* Parms = *Params;

	FString NameOfPackageToBeConformed, NameOfPackageToConformAgainst;
	if( !ParseToken(Parms,NameOfPackageToBeConformed,0) )
	{
		warnf(NAME_Error, TEXT("Source file not specified"));
		return 1;
	}
	if( !ParseToken(Parms,NameOfPackageToConformAgainst,0) )
	{
		warnf(NAME_Error, (TEXT("Original file not specified")));
		return 1;
	}

	// get the full pathname of the package we're conforming
	FString SrcFilename;
	GPackageFileCache->FindPackageFile(*NameOfPackageToBeConformed, NULL, SrcFilename);

	// if it's read only, don't bother
	if (GFileManager->IsReadOnly(*SrcFilename))
	{
		warnf(NAME_Warning, TEXT("Package '%s' is read only"), *SrcFilename);
		return 0;
	}

	GWarn->Log( TEXT("Loading...") );
	BeginLoad();
	FFilename OldFilename = FFilename(NameOfPackageToConformAgainst);
	UPackage* OldPackage = CreatePackage( NULL, *(OldFilename.GetBaseFilename() + TEXT("_OLD")) );
	ULinkerLoad* OldLinker = UObject::GetPackageLinker( OldPackage, *NameOfPackageToConformAgainst, LOAD_NoWarn|LOAD_NoVerify, NULL, NULL );
	EndLoad();

	UPackage* NewPackage = LoadPackage( NULL, *NameOfPackageToBeConformed, LOAD_None );
	
	if( !OldLinker )
	{
		warnf(NAME_Error, TEXT("Failed to load original file '%s'"), *NameOfPackageToConformAgainst);
		return 1;
	}
	if( !NewPackage )
	{
		warnf(NAME_Error, TEXT("Failed to load file being conformed '%s'"), *NameOfPackageToBeConformed);
		return 1;
	}

	GWarn->Log( TEXT("Saving...") );
	if ( SavePackageHelper(NewPackage, SrcFilename, GError, OldLinker) )
	{
		warnf( TEXT("File %s successfully conformed to %s..."), *NameOfPackageToBeConformed, *NameOfPackageToConformAgainst );
	}
	else
	{
		warnf(TEXT("File %s could not be conformed to %s.  Please see the log for details."), *NameOfPackageToBeConformed, *NameOfPackageToConformAgainst);
	}
	
	return 0;
}
IMPLEMENT_CLASS(UConformCommandlet)

/*-----------------------------------------------------------------------------
	UDumpEmittersCommandlet commandlet.
-----------------------------------------------------------------------------*/
#include "EngineParticleClasses.h"

const TCHAR* GetParticleModuleAcronym(UParticleModule* Module)
{
	if (!Module)
		return TEXT("");

	if (Module->IsA(UParticleModuleAcceleration::StaticClass()))				return TEXT("IA");
	if (Module->IsA(UParticleModuleAccelerationOverLifetime::StaticClass()))	return TEXT("AOL");
	if (Module->IsA(UParticleModuleAttractorLine::StaticClass()))				return TEXT("AL");
	if (Module->IsA(UParticleModuleAttractorParticle::StaticClass()))			return TEXT("AP");
	if (Module->IsA(UParticleModuleAttractorPoint::StaticClass()))				return TEXT("APT");
	if (Module->IsA(UParticleModuleBeamNoise::StaticClass()))					return TEXT("BN");
	if (Module->IsA(UParticleModuleBeamSource::StaticClass()))					return TEXT("BS");
	if (Module->IsA(UParticleModuleBeamTarget::StaticClass()))					return TEXT("BT");
	if (Module->IsA(UParticleModuleCollision::StaticClass()))					return TEXT("CLS");
	if (Module->IsA(UParticleModuleColor::StaticClass()))						return TEXT("IC");
	if (Module->IsA(UParticleModuleColorByParameter::StaticClass()))			return TEXT("CBP");
	if (Module->IsA(UParticleModuleColorOverLife::StaticClass()))				return TEXT("COL");
	if (Module->IsA(UParticleModuleColorScaleOverLife::StaticClass()))			return TEXT("CSL");
	if (Module->IsA(UParticleModuleLifetime::StaticClass()))					return TEXT("LT");
	if (Module->IsA(UParticleModuleLocation::StaticClass()))					return TEXT("IL");
	if (Module->IsA(UParticleModuleLocationDirect::StaticClass()))				return TEXT("LD");
	if (Module->IsA(UParticleModuleLocationEmitter::StaticClass()))				return TEXT("LE");
	if (Module->IsA(UParticleModuleLocationPrimitiveCylinder::StaticClass()))	return TEXT("LPC");
	if (Module->IsA(UParticleModuleLocationPrimitiveSphere::StaticClass()))		return TEXT("LPS");
	if (Module->IsA(UParticleModuleMeshRotation::StaticClass()))				return TEXT("IMR");
	if (Module->IsA(UParticleModuleMeshRotationRate::StaticClass()))			return TEXT("IMRR");
	if (Module->IsA(UParticleModuleOrientationAxisLock::StaticClass()))			return TEXT("OAL");
	if (Module->IsA(UParticleModuleRotation::StaticClass()))					return TEXT("IR");
	if (Module->IsA(UParticleModuleRotationOverLifetime::StaticClass()))		return TEXT("ROL");
	if (Module->IsA(UParticleModuleRotationRate::StaticClass()))				return TEXT("IRR");
	if (Module->IsA(UParticleModuleRotationRateMultiplyLife::StaticClass()))	return TEXT("RRML");
	if (Module->IsA(UParticleModuleSize::StaticClass()))						return TEXT("IS");
	if (Module->IsA(UParticleModuleSizeMultiplyLife::StaticClass()))			return TEXT("SML");
	if (Module->IsA(UParticleModuleSizeMultiplyVelocity::StaticClass()))		return TEXT("SMV");
	if (Module->IsA(UParticleModuleSizeScale::StaticClass()))					return TEXT("SS");
	if (Module->IsA(UParticleModuleSubUV::StaticClass()))						return TEXT("SUV");
	if (Module->IsA(UParticleModuleSubUVDirect::StaticClass()))					return TEXT("SUVD");
	if (Module->IsA(UParticleModuleSubUVSelect::StaticClass()))					return TEXT("SUVS");
	if (Module->IsA(UParticleModuleTrailSource::StaticClass()))					return TEXT("TS");
	if (Module->IsA(UParticleModuleTrailSpawn::StaticClass()))					return TEXT("TSP");
	if (Module->IsA(UParticleModuleTrailTaper::StaticClass()))					return TEXT("TT");
	if (Module->IsA(UParticleModuleTypeDataBeam::StaticClass()))				return TEXT("TDB");
	if (Module->IsA(UParticleModuleTypeDataBeam2::StaticClass()))				return TEXT("TDB2");
	if (Module->IsA(UParticleModuleTypeDataMesh::StaticClass()))				return TEXT("TDM");
	if (Module->IsA(UParticleModuleTypeDataTrail::StaticClass()))				return TEXT("TDT");
	if (Module->IsA(UParticleModuleVelocity::StaticClass()))					return TEXT("IV");
	if (Module->IsA(UParticleModuleVelocityInheritParent::StaticClass()))		return TEXT("VIP");
	if (Module->IsA(UParticleModuleVelocityOverLifetime::StaticClass()))		return TEXT("VOL");

	return TEXT("???");
}

INT UDumpEmittersCommandlet::Main(const FString& Params)
{
	const TCHAR* Parms = *Params;

	FString						PackageWildcard;
	TArray<UParticleSystem*>	ParticleSystems;
	TArray<UPackage*>			ParticlePackages;
	TArray<FString>				UberModuleList;

	const TCHAR*	CmdLine		= appCmdLine();
	UBOOL			bAllRefs	= FALSE;

	if (ParseParam(CmdLine, TEXT("ALLREFS")))
	{
		bAllRefs	= TRUE;
	}

	while (ParseToken(Parms, PackageWildcard, 0))
	{
		TArray<FString> FilesInPath;

		if (PackageWildcard.Left(1) == TEXT("-"))
		{
			// this is a command-line parameter - skip it
			continue;
		}

		GFileManager->FindFiles(FilesInPath, *PackageWildcard, 1, 0);
		if( FilesInPath.Num() == 0 )
		{
			// if no files were found, it might be an unqualified path; try prepending the .u output path
			// if one were going to make it so that you could use unqualified paths for package types other
			// than ".u", here is where you would do it
			GFileManager->FindFiles(FilesInPath, *(appScriptOutputDir() * PackageWildcard), 1, 0);
		}

		if (FilesInPath.Num() == 0)
		{
			warnf(TEXT("No packages found using '%s'!"), *PackageWildcard);
			continue;
		}

		for (INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++)
		{
			const FString &Filename = FilesInPath(FileIndex);

			BeginLoad();
			// Assert if package couldn't be opened so we have no chance of messing up saving later packages.
			UPackage* Package = CastChecked<UPackage>(UObject::LoadPackage(NULL, *Filename, 0));
			check(Package);
			EndLoad();

			// Check for particle systems
			if (Package)
			{
				debugf(TEXT("Examining Package %s"), *Package->GetName());

				// Dump the particle systems
				for (TObjectIterator<UParticleSystem> It; It; ++It)
				{
					UParticleSystem* PartSys = Cast<UParticleSystem>(*It);
					check(PartSys->GetOuter());

					// Determine the package it is in...
					UObject*	Outer			= PartSys->GetOuter();
					UPackage*	ParticlePackage = Cast<UPackage>(Outer);
					while (ParticlePackage == NULL)
					{
						Outer			= Outer->GetOuter();
						ParticlePackage = Cast<UPackage>(Outer);
						if (Outer == NULL)
						{
							warnf(TEXT("No package??? %s"), *PartSys->GetName());
							break;
						}
					}

					if (bAllRefs)
					{
						// Find the package the particle is in...
						ParticleSystems.AddUniqueItem(PartSys);
						ParticlePackages.AddUniqueItem(ParticlePackage);
					}
					else
					{
						if (It->IsIn(Package))
						{
							ParticleSystems.AddUniqueItem(PartSys);
							ParticlePackages.AddUniqueItem(Package);
						}
					}
				}
			}

			// Now, dump out the particle systems and the package list.
			debugf(TEXT("**********************************************************************************************\n"));
			debugf(TEXT("                                     PARTICLE SYSTEM LIST                                     \n"));
			debugf(TEXT("**********************************************************************************************\n"));
			debugf(TEXT("ParticleSystem Count %d"), ParticleSystems.Num());
			for (INT PSysIndex = 0; PSysIndex < ParticleSystems.Num(); PSysIndex++)
			{
				UParticleSystem* PSys = ParticleSystems(PSysIndex);
				if (PSys)
				{
					debugf(TEXT("\tParticleSystem %s"), *PSys->GetFullName());

					for (INT EmitterIndex = 0; EmitterIndex < PSys->Emitters.Num(); EmitterIndex++)
					{
						UParticleEmitter* Emitter = PSys->Emitters(EmitterIndex);
						if (Emitter == NULL)
							continue;

						debugf(TEXT("\t\tParticleEmitter %s"), *Emitter->GetName());

						if (Emitter->LODLevels.Num() > 0)
						{
							UParticleLODLevel* LODLevel = Emitter->LODLevels(0);
							if (LODLevel)
							{
								FString	AcronymCollection;

								UParticleModule* Module;
								for (INT ModuleIndex = 0; ModuleIndex < LODLevel->Modules.Num(); ModuleIndex++)
								{
									Module = LODLevel->Modules(ModuleIndex);
									if (Module)
									{
										debugf(TEXT("\t\t\tModule %2d - %48s - %s"), ModuleIndex, *Module->GetName(), GetParticleModuleAcronym(Module));
										AcronymCollection += GetParticleModuleAcronym(Module);
									}
								}

								UberModuleList.AddUniqueItem(AcronymCollection);
							}
						}
					}
				}
			}

			debugf(TEXT("**********************************************************************************************\n"));
			debugf(TEXT("                                         PACKAGE LIST                                         \n"));
			debugf(TEXT("**********************************************************************************************\n"));
			debugf(TEXT("Package Count %d"), ParticlePackages.Num());
			for (INT PackageIndex = 0; PackageIndex < ParticlePackages.Num(); PackageIndex++)
			{
				UPackage* OutPackage	= ParticlePackages(PackageIndex);
				debugf(TEXT("\t%s"), *OutPackage->GetName());
			}

			UObject::CollectGarbage(RF_Native);

			debugf(TEXT("**********************************************************************************************\n"));
			debugf(TEXT("                                       UBER-MODULE LIST                                       \n"));
			debugf(TEXT("**********************************************************************************************\n"));
			debugf(TEXT("UberModule Count %d"), UberModuleList.Num());
			for (INT UberIndex = 0; UberIndex < UberModuleList.Num(); UberIndex++)
			{
				debugf(TEXT("\t%3d - %s"), UberIndex, *UberModuleList(UberIndex));
			}
		}
	}

	return 0;
}

IMPLEMENT_CLASS(UDumpEmittersCommandlet)


/*-----------------------------------------------------------------------------
UMergePackagesCommandlet
-----------------------------------------------------------------------------*/

/**
 * This function will return all objects that were created after a given object index.
 * The idea is that you load some stuff, remember how many objects are in the engine, 
 * and then load a map where you want to track what was loaded. Any objects you load 
 * after that point will come after the original number of objects.
 *
 * @param NewObjects	The array that receives the new objects loaded beginning with index Start
 * @param Start			The first index of object to copy into NewObjects
 */
void GetAllObjectsAfter(TArray<UObject*>& NewObjects, DWORD Start = 0)
{
	// i wish there was a better way
	DWORD NumObjects = 0;
	for (TObjectIterator<UObject> It; It; ++It)
	{
		NumObjects++;
	}

	DWORD ObjIndex = 0;
	for (TObjectIterator<UObject> It; It; ObjIndex++, ++It)
	{
		UObject* Obj = *It;
		if (ObjIndex >= Start)
		{
			NewObjects.AddItem(Obj);
		}
	}
}

INT UMergePackagesCommandlet::Main( const FString& Params )
{
	const TCHAR* Parms = *Params;

	FString	PackageWildcard;

	FString SrcPackageName, DestPackageName;
	ParseToken(Parms,SrcPackageName,0);
	ParseToken(Parms,DestPackageName,1);

	
	// Only moving objects from source package, so find offset of current objects
	INT FirstObjectToMove = 0;
	for (TObjectIterator<UObject> It; It; ++It)
	{
		FirstObjectToMove++;
	}

	UPackage* SrcPackage  = Cast<UPackage>(UObject::LoadPackage( NULL, *SrcPackageName, 0 ));

	// the objects to be moved are the ones in the object array
	TArray<UObject*> DiffObjects;
	GetAllObjectsAfter(DiffObjects, FirstObjectToMove);

	UPackage* DestPackage = Cast<UPackage>(UObject::LoadPackage( NULL, *DestPackageName, 0 ));

	// go through all the new objects and rename all packages that were loaded to their new package name
	for (INT ObjIndex = 0; ObjIndex < DiffObjects.Num(); ObjIndex++)
	{
		UPackage* DiffPackage = Cast<UPackage>(DiffObjects(ObjIndex));

		if (DiffPackage && DiffPackage != UObject::GetTransientPackage() && DiffPackage != DestPackage)
		{
			BeginLoad();
			ULinkerLoad* Linker = GetPackageLinker(DiffPackage, NULL, LOAD_NoWarn, NULL, NULL);
			EndLoad();
			if (Linker && !Linker->ContainsCode() && !Linker->ContainsMap())
			{
				warnf(TEXT("Moving object %s to %s"), *DiffPackage->GetFullName(),*DestPackageName);
				// Only move if won't conflict
				UBOOL Exists = StaticFindObject(DiffPackage->GetClass(), DestPackage, *DiffPackage->GetName(), TRUE)!=0;
				if(Exists)
				{
					warnf(TEXT("Object already exists in package, skipped"));
				}
				else
				{
					DiffPackage->Rename(*DiffPackage->GetName(), DestPackage, REN_ForceNoResetLoaders);
//					warnf(TEXT("Object added to package"));
				}
			}
			else
			{
//				warnf(TEXT("%s didn't match"), *DiffPackage->GetFullName());
			}
		}
	}

	SavePackageHelper(DestPackage, DestPackageName);

	UObject::CollectGarbage(RF_Native);
	return 0;
}
IMPLEMENT_CLASS(UMergePackagesCommandlet)

/* ==========================================================================================================
	UCreateDefaultStyleCommandlet
========================================================================================================== */
INT UCreateDefaultStyleCommandlet::Main( const FString& Params )
{
	const TCHAR* Parms = *Params;

	// create the UISkin package
	UPackage* SkinPackage = CreatePackage(NULL, TEXT("DefaultUISkin"));

	// load the classes we need, to ensure that their defaults are loaded
	LoadClass<UUISkin>(NULL, TEXT("Engine.UISkin"), NULL, LOAD_None, NULL);
	LoadClass<UUIStyle>(NULL, TEXT("Engine.UIStyle"), NULL, LOAD_None, NULL);
	LoadClass<UUIStyle_Data>(NULL, TEXT("Engine.UIStyle_Combo"), NULL, LOAD_None, NULL);

	// create the UISkin object
	DefaultSkin = ConstructObject<UUISkin>( UUISkin::StaticClass(), SkinPackage, TEXT("DefaultSkin"), RF_Public|RF_Standalone );
	DefaultSkin->Tag = TEXT("DefaultSkin");

	// create the default text style
	UUIStyle_Text* DefaultTextStyle = CreateTextStyle();

	// create the default image style
	UUIStyle_Image* DefaultImageStyle = CreateImageStyle();

	// create the default combo style
	UUIStyle_Combo* DefaultComboStyle = CreateComboStyle(DefaultTextStyle,DefaultImageStyle);

	DefaultSkin->AddStyle(DefaultTextStyle->GetOwnerStyle());
	DefaultSkin->AddStyle(DefaultImageStyle->GetOwnerStyle());
	DefaultSkin->AddStyle(DefaultComboStyle->GetOwnerStyle());

	CreateAdditionalStyles();

	CreateMouseCursors();

	// now save it!
	INT Result = SavePackage(SkinPackage, DefaultSkin, 0, TEXT("../Engine/Content/DefaultUISkin.upk"), GWarn) ? 0 : 1;

	return Result;
}

void UCreateDefaultStyleCommandlet::CreateAdditionalStyles() const
{
	CreateConsoleStyles();

	UUIStyle_Image* CursorImageStyle = CreateImageStyle(TEXT("CursorImageStyle"), FLinearColor(1.f,1.f,1.f,1.f));
	CursorImageStyle->DefaultImage = LoadObject<UTexture>(NULL, TEXT("EngineResources.Cursors.Arrow"), NULL, LOAD_None, NULL);
	DefaultSkin->AddStyle(CursorImageStyle->GetOwnerStyle());

	UUIStyle_Image* CaretImageStyle = CreateImageStyle(TEXT("DefaultCaretStyle"), FLinearColor(1.f,1.f,1.f,1.f));
	CaretImageStyle->DefaultImage = LoadObject<UTexture>(NULL, TEXT("EngineResources.White"), NULL, LOAD_None, NULL);
	DefaultSkin->AddStyle(CaretImageStyle->GetOwnerStyle());
}

void UCreateDefaultStyleCommandlet::CreateMouseCursors() const
{
	FUIMouseCursor ArrowCursor, SplitVCursor, SplitHCursor;
	ArrowCursor.CursorStyle = SplitVCursor.CursorStyle = SplitHCursor.CursorStyle = TEXT("CursorImageStyle");

	UUITexture* ArrowTexture = ConstructObject<UUITexture>(UUITexture::StaticClass(), DefaultSkin);
	UUITexture* SplitVTexture = ConstructObject<UUITexture>(UUITexture::StaticClass(), DefaultSkin);
	UUITexture* SplitHTexture = ConstructObject<UUITexture>(UUITexture::StaticClass(), DefaultSkin);

	ArrowTexture->ImageTexture = LoadObject<UTexture>(NULL, TEXT("EngineResources.Cursors.Arrow"), NULL, LOAD_None, NULL);
	SplitVTexture->ImageTexture = LoadObject<UTexture>(NULL, TEXT("EngineResources.Cursors.SplitterVert"), NULL, LOAD_None, NULL);
	SplitHTexture->ImageTexture = LoadObject<UTexture>(NULL, TEXT("EngineResources.Cursors.SplitterHorz"), NULL, LOAD_None, NULL);

	ArrowCursor.Cursor = ArrowTexture;
	SplitHCursor.Cursor = SplitHTexture;
	SplitVCursor.Cursor = SplitVTexture;

	DefaultSkin->AddCursorResource(TEXT("Arrow"), ArrowCursor);
	DefaultSkin->AddCursorResource(TEXT("SplitterHorz"), SplitHCursor);
	DefaultSkin->AddCursorResource(TEXT("SplitterVert"), SplitVCursor);
}

void UCreateDefaultStyleCommandlet::CreateConsoleStyles() const
{
	// create the style for rendering the text that is being typed into the console
	UUIStyle_Text* ConsoleTextStyle = CreateTextStyle(TEXT("ConsoleTextStyle"), FLinearColor(0.f,1.f,0.f,1.f));

	// create the style for the borders around the console typing region
	UUIStyle_Image* ConsoleImageStyle = CreateImageStyle(TEXT("ConsoleImageStyle"),FLinearColor(0.f,1.f,0.f,1.f));
	ConsoleImageStyle->DefaultImage = LoadObject<UTexture2D>(NULL, TEXT("EngineResources.White"),NULL,0,NULL);

	// create a combo style that will be applied to the console's typing region
	UUIStyle_Combo* ConsoleStyle = CreateComboStyle(ConsoleTextStyle, ConsoleImageStyle, TEXT("ConsoleStyle"));

	// create the style for rendering the console buffer text
	UUIStyle_Text* ConsoleBufferTextStyle = CreateTextStyle(TEXT("ConsoleBufferTextStyle"));

	// create the style for the buffer background
	UUIStyle_Image* ConsoleBufferImageStyle = CreateImageStyle(TEXT("ConsoleBufferImageStyle"),FLinearColor(0.f,0.f,0.f,1.f));
	ConsoleBufferImageStyle->DefaultImage = LoadObject<UTexture2D>(NULL, TEXT("EngineResources.Black"),NULL,0,NULL);

	UUIStyle_Combo* ConsoleBufferStyle = CreateComboStyle(ConsoleBufferTextStyle, ConsoleBufferImageStyle, TEXT("ConsoleBufferStyle"));

	DefaultSkin->AddStyle(ConsoleTextStyle->GetOwnerStyle());
	DefaultSkin->AddStyle(ConsoleImageStyle->GetOwnerStyle());
	DefaultSkin->AddStyle(ConsoleStyle->GetOwnerStyle());
	DefaultSkin->AddStyle(ConsoleBufferTextStyle->GetOwnerStyle());
	DefaultSkin->AddStyle(ConsoleBufferImageStyle->GetOwnerStyle());
	DefaultSkin->AddStyle(ConsoleBufferStyle->GetOwnerStyle());
}

UUIStyle_Text* UCreateDefaultStyleCommandlet::CreateTextStyle( const TCHAR* StyleName/*=TEXT("DefaultTextStyle")*/, FLinearColor StyleColor/*=FLinerColor(1.f,1.f,1.f,1.f)*/ ) const
{
	UUIStyle* Style = ConstructObject<UUIStyle>( UUIStyle::StaticClass(), DefaultSkin, StyleName, RF_Public|RF_Transactional|RF_PerObjectLocalized );
	Style->StyleDataClass = UUIStyle_Text::StaticClass();
	Style->StyleID = appCreateGuid();
	Style->StyleTag = Style->GetFName();

	UUIStyle_Text* Result = Cast<UUIStyle_Text>(Style->AddNewState(UUIState_Enabled::StaticClass()->GetDefaultObject<UUIState_Enabled>()));
	Result->StyleFont = GEngine->SmallFont;
	Result->StyleColor = StyleColor;

	return Result;
}

UUIStyle_Image* UCreateDefaultStyleCommandlet::CreateImageStyle( const TCHAR* StyleName/*=TEXT("DefaultImageStyle")*/, FLinearColor StyleColor/*=FLinerColor(1.f,1.f,1.f,1.f)*/ ) const
{
	UUIStyle* Style = ConstructObject<UUIStyle>( UUIStyle::StaticClass(), DefaultSkin, StyleName, RF_Public|RF_Transactional|RF_PerObjectLocalized );
	Style->StyleDataClass = UUIStyle_Image::StaticClass();
	Style->StyleID = appCreateGuid();
	Style->StyleTag = Style->GetFName();

	UUIStyle_Image* Result = Cast<UUIStyle_Image>(Style->AddNewState(UUIState_Enabled::StaticClass()->GetDefaultObject<UUIState_Enabled>()));
	Result->DefaultImage = LoadObject<UTexture2D>(NULL, TEXT("EngineResources.DefaultTexture"), NULL, 0, NULL);
	Result->StyleColor = StyleColor;

	return Result;
}

UUIStyle_Combo* UCreateDefaultStyleCommandlet::CreateComboStyle( UUIStyle_Text* TextStyle, UUIStyle_Image* ImageStyle, const TCHAR* StyleName/*=TEXT("DefaultComboStyle")*/ ) const
{
	UUIStyle* Style = ConstructObject<UUIStyle>( UUIStyle::StaticClass(), DefaultSkin, StyleName, RF_Public|RF_Transactional|RF_PerObjectLocalized );
	Style->StyleDataClass = UUIStyle_Combo::StaticClass();
	Style->StyleID = appCreateGuid();
	Style->StyleTag = Style->GetFName();

	UUIStyle_Combo* Result = Cast<UUIStyle_Combo>(Style->AddNewState(UUIState_Enabled::StaticClass()->GetDefaultObject<UUIState_Enabled>()));
	Result->StyleColor = FLinearColor(1.f, 1.f, 1.f, 1.f);
	Result->TextStyle = FStyleDataReference(TextStyle->GetOwnerStyle(),UUIState_Enabled::StaticClass()->GetDefaultObject<UUIState_Enabled>());
	Result->ImageStyle = FStyleDataReference(ImageStyle->GetOwnerStyle(), UUIState_Enabled::StaticClass()->GetDefaultObject<UUIState_Enabled>());

	return Result;
}

IMPLEMENT_CLASS(UCreateDefaultStyleCommandlet);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// UExamineOutersCommandlet
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT UExamineOutersCommandlet::Main(const FString& Params)
{
	// Parse command line args.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	const TCHAR* Parms = *Params;
	ParseCommandLine(Parms, Tokens, Switches);

	// Build package file list.
	const TArray<FString> FilesInPath( GPackageFileCache->GetPackageFileList() );
	if( FilesInPath.Num() == 0 )
	{
		warnf( NAME_Warning, TEXT("No packages found") );
		return 1;
	}

	const FString DefaultObjectPrefix( DEFAULT_OBJECT_PREFIX );

	warnf( NAME_Log, TEXT("%s%-7s%-7s%-7s%-7s%-7s%-7s%-7s%-7s%-7s"),
		*FString("Package").RightPad(72),TEXT("Match"), TEXT("Level"), TEXT("Pkg"), TEXT("Class"), TEXT("NULL"), TEXT("DefObj"), TEXT("PSReq"), TEXT("Other"), TEXT("Total") );

	// Iterate over all files doing stuff.
	for( INT FileIndex = 0 ; FileIndex < FilesInPath.Num() ; ++FileIndex )
	{
		const FFilename& Filename = FilesInPath(FileIndex);

		UObject* Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
		if( Package == NULL )
		{
			warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
		}

		// Iterate over all the editinline properties of an object and gather outer stats.
		INT NumMatchedOuters = 0;
		INT NumLevelOuters = 0;
		INT NumPackageOuters = 0;
		INT NumClassOuters = 0;
		INT NumNullOuters = 0;
		INT NumDefaultObjectOuters = 0;
		INT NumPSRequiredModule = 0;
		INT NumOther = 0;
		INT NumTotal = 0;

		for ( TObjectIterator<UObject> It ; It ; ++It )
		{
			UObject* Object = *It;
			UClass* ObjectClass = Object->GetClass();

			for ( TFieldIterator<UProperty,CASTCLASS_UProperty> PropIt( ObjectClass ) ; PropIt ; ++PropIt )
			{
				UObjectProperty* Property = Cast<UObjectProperty>( *PropIt );

				// Select out editinline object properties.
				if ( (PropIt->PropertyFlags & (CPF_EditInline | CPF_EditInlineUse)) && Property )
				{
					// Get the property value.
					UObject* EditInlineObject;
					Property->CopySingleValue( &EditInlineObject, ((BYTE*)Object) + Property->Offset );

					// If the property value was non-NULL . . .
					if ( EditInlineObject )
					{
						// Check its outer.
						UObject* EditInlineObjectOuter = EditInlineObject->GetOuter();

						if ( EditInlineObjectOuter == NULL )
						{
							++NumNullOuters;
						}
						else if ( EditInlineObjectOuter == Object )
						{
							++NumMatchedOuters;
						}
						else if ( EditInlineObjectOuter->IsA( ULevel::StaticClass() ) )
						{
							++NumLevelOuters;
						}
						else if ( EditInlineObjectOuter->IsA( UPackage::StaticClass() ) )
						{
							++NumPackageOuters;
						}
						else if ( EditInlineObjectOuter->IsA( UClass::StaticClass() ) )
						{
							++NumClassOuters;
						}
						else if ( FString( *EditInlineObjectOuter->GetName() ).StartsWith( DefaultObjectPrefix ) )
						{
							++NumDefaultObjectOuters;
						}
						else if ( appStricmp( *Property->GetName(), TEXT("RequiredModule") ) == 0 )
						{
							++NumPSRequiredModule;
						}
						else
						{
							++NumOther;
						}
						++NumTotal;
					}
				}
			} // property itor
		} // object itor

		warnf( NAME_Log, TEXT("%s%-7i%-7i%-7i%-7i%-7i%-7i%-7i%-7i%-7i"),
			*Filename.RightPad( 72 ), NumMatchedOuters, NumLevelOuters, NumPackageOuters, NumClassOuters, NumNullOuters, NumDefaultObjectOuters, NumPSRequiredModule, NumOther, NumTotal );

		// Clean up.
		UObject::CollectGarbage( RF_Native );
	}

	return 0;
}
IMPLEMENT_CLASS(UExamineOutersCommandlet);


/*-----------------------------------------------------------------------------
	URebuildMapCommandlet commandlet.
-----------------------------------------------------------------------------*/

INT URebuildMapCommandlet::Main(const FString& Params)
{
	// Parse command line args.
	TArray<FString> Tokens;
	TArray<FString> Switches;
	ParseCommandLine( *Params, Tokens, Switches);

	// Rebuilding a map is a very slow operation and there are bound to be bugs in the map transition code so we only support
	// a single map (including sublevels) at a time and require multiple maps to be handled at the calling level via e.g. a
	// batch file.

	if( Tokens.Num() == 0 )
	{
		warnf(NAME_Error, TEXT("Missing map name argument!"));
	}
	else if( Tokens.Num() > 1 )
	{
		warnf(NAME_Error, TEXT("Can only rebuild a single map at a time!"));
	}
	else
	{
		check(Tokens.Num()==1);

		// See whether filename was found in cache.		
		FFilename Filename;	
		if( GPackageFileCache->FindPackageFile( *Tokens(0), NULL, Filename ) )
		{
			// Load the map file.
			UPackage* Package = UObject::LoadPackage( NULL, *Filename, 0 );
			if( Package )
			{
				// Find the world object inside the map file.
				UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
				if( World )
				{
					// Set loaded world as global world object and add to root set so the lighting rebuild
					// code doesn't garbage collect it.
					GWorld = World;
					GWorld->AddToRoot();
					GWorld->Init();
				
					// Mark all sublevels as being visible in the Editor so they get rebuilt.
					for( INT LevelIndex=0; LevelIndex<GWorld->GetWorldInfo()->StreamingLevels.Num(); LevelIndex++ )
					{
						ULevelStreaming* LevelStreamingObject = GWorld->GetWorldInfo()->StreamingLevels(LevelIndex);
						if( LevelStreamingObject )
						{
							LevelStreamingObject->bShouldBeVisibleInEditor = TRUE;
						}
					}
			
					// Associate sublevels, loading and making them visible.
					GWorld->UpdateLevelStreaming();
					GWorld->FlushLevelStreaming();

					// Check for any PrefabInstances which are out of date.
					GEditor->UpdatePrefabs();

					// Update components for all levels.
					GWorld->UpdateComponents(FALSE);

					// Rebuild BSP.
					GEditor->Exec( TEXT("MAP REBUILD ALLVISIBLE") );
					GEditor->ClearComponents();
					GWorld->CleanupWorld();
					GEditor->UpdateComponents();

					// Rebuild lighting.
					DOUBLE LightingStartTime = appSeconds();
					FLightingBuildOptions Options;
					GEditor->BuildLighting( Options );
					warnf(TEXT("Building lighting took %5.2f minutes."), (appSeconds() - LightingStartTime) / 60 );


					// building paths with streaming levels doesn't currently work correctly
					// so only build paths if there are no streaming levels
					if( GWorld->GetWorldInfo()->StreamingLevels.Num() == 0 )
					{
						// Rebuild pathing.
						DOUBLE PathingStartTime = appSeconds();
						FPathBuilder::Exec( TEXT("PREDEFINEPATHS") );
						FPathBuilder::Exec( TEXT("DEFINEPATHS REVIEWPATHS=1 SHOWMAPCHECK=0") );
						FPathBuilder::Exec( TEXT("BUILDCOVER FROMDEFINEPATHS=0") );
						FPathBuilder::Exec( TEXT("BUILDCOMBATZONES FROMDEFINEPATHS=0") );
						FPathBuilder::Exec( TEXT("BUILDNETWORKIDS") );
						FPathBuilder::Exec( TEXT("POSTDEFINEPATHS") );
						FPathBuilder::Exec( TEXT("FINISHPATHBUILD") );
						warnf(TEXT("Building paths took    %5.2f minutes."), (appSeconds() - PathingStartTime) / 60 );
					}

					// Iterate over all levels in the world and save them.
					for( INT LevelIndex=0; LevelIndex<GWorld->Levels.Num(); LevelIndex++ )
					{
						ULevel*			Level			= GWorld->Levels(LevelIndex);
						check(Level);					
						UWorld*			World			= CastChecked<UWorld>(Level->GetOuter());
						ULinkerLoad*	Linker			= World->GetLinker();
						check(Linker);
						FString			LevelFilename	= Linker->Filename;

						// Save sublevel.
						UObject::SavePackage( World->GetOutermost(), World, 0, *LevelFilename, GWarn );
					}

					// Remove from root again.
					GWorld->RemoveFromRoot();
				}
				// Couldn't find the world object in the loaded package. Most likely because it's not a map.
				else
				{
					warnf(NAME_Log, TEXT("Cannot find world object in '%s'! Is it a map file?"), *Filename);
				}
			}
			// Couldn't load the package for some reason.
			else
			{
				warnf(NAME_Log, TEXT("Error loading '%s'!"), *Filename);
			}
		}
	}

	return 0;
}
IMPLEMENT_CLASS(URebuildMapCommandlet);

/*-----------------------------------------------------------------------------
	UTestCompressionCommandlet commandlet.
-----------------------------------------------------------------------------*/

/**
 * Run a compression/decompress test with the given package and compression options
 *
 * @param PackageName		The package to compress/decompress
 * @param Flags				The options for compression
 * @param UncompressedSize	The size of the uncompressed package file
 * @param CompressedSize	The options for compressed package file
 */
void UTestCompressionCommandlet::RunTest(const FFilename& PackageName, ECompressionFlags Flags, DWORD& UncompressedSize, DWORD& CompressedSize)
{
	SET_WARN_COLOR(COLOR_WHITE);
	warnf(TEXT(""));
	warnf(TEXT("Running Compression Test..."));

	SET_WARN_COLOR(COLOR_YELLOW);
	warnf(TEXT("Package:"));
	SET_WARN_COLOR(COLOR_GRAY);
	warnf(TEXT("  %s"), *PackageName.GetCleanFilename());

	SET_WARN_COLOR(COLOR_YELLOW);
	warnf(TEXT("Options:"));
	SET_WARN_COLOR(COLOR_GRAY);
	if (Flags & COMPRESS_LZO)
	{
		warnf(TEXT("  LZO"));
	}
	if (Flags & COMPRESS_LZX)
	{
		warnf(TEXT("  LZX"));
	}
	if (Flags & COMPRESS_ZLIB)
	{
		warnf(TEXT("  ZLIB"));
	}
	if (Flags & COMPRESS_BiasMemory)
	{
		warnf(TEXT("  Compressed for memory"));
	}
	if (Flags & COMPRESS_BiasSpeed)
	{
		warnf(TEXT("  Compressed for decompression speed"));
	}
#if _DEBUG
	warnf(TEXT("  Debug build"));
#elif FINAL_RELEASE
	warnf(TEXT("  Final release build"));
#else
	warnf(TEXT("  Release build"));
#endif

	// Generate new filename by swapping out extension.
	FFilename DstPackageFilename = FString::Printf(TEXT("%s%s%s.compressed_%x"), 
		*PackageName.GetPath(),
		PATH_SEPARATOR,
		*PackageName.GetBaseFilename(),
		(DWORD)Flags);

	// Create file reader and writers.
	FArchive* Reader = GFileManager->CreateFileReader( *PackageName );
	FArchive* Writer = GFileManager->CreateFileWriter( *DstPackageFilename );
	check( Reader );
	check( Writer );

	// Figure out filesize.
	INT ReaderTotalSize	= Reader->TotalSize();
	// Create buffers for serialization (Src) and comparison (Dst).
	BYTE* SrcBuffer			= (BYTE*) appMalloc( ReaderTotalSize );
	BYTE* DstBuffer			= (BYTE*) appMalloc( ReaderTotalSize );
	BYTE* DstBufferAsync	= (BYTE*) appMalloc( ReaderTotalSize );

	DOUBLE StartRead = appSeconds();
	Reader->Serialize( SrcBuffer, ReaderTotalSize );
	DOUBLE StopRead = appSeconds();

	DOUBLE StartCompress = appSeconds();
	Writer->SerializeCompressed( SrcBuffer, ReaderTotalSize, Flags );
	DOUBLE StopCompress = appSeconds();

	// Delete (and implicitly flush) reader and writers.
	delete Reader;
	delete Writer;

	// Figure out compressed size (and propagate uncompressed for completeness)
	CompressedSize		= GFileManager->FileSize( *DstPackageFilename );
	UncompressedSize	= ReaderTotalSize;

	// Load compressed via FArchive
	Reader = GFileManager->CreateFileReader( *DstPackageFilename );

	Reader->SerializeCompressed( DstBuffer, 0, Flags );
	delete Reader;
	
	// Load compressed via async IO system.
	FIOSystem* IO = GIOManager->GetIOSystem( IOSYSTEM_GenericAsync );
	check(GIOManager);
	FThreadSafeCounter Counter(1);
	IO->LoadCompressedData( DstPackageFilename, 0, CompressedSize, UncompressedSize, DstBufferAsync, Flags, &Counter, AIOP_Normal );
	// Wait till it completed.
	while( Counter.GetValue() > 0 )
	{
		appSleep(0.05f);
	}
	
	// Compare results and assert if they are different.
	for( DWORD Count=0; Count<UncompressedSize; Count++ )
	{
		BYTE Src		= SrcBuffer[Count];
		BYTE Dst		= DstBuffer[Count];
		BYTE DstAsync	= DstBufferAsync[Count];
		check( Src == Dst );
		check( Src == DstAsync );
	}

	appFree( SrcBuffer );
	appFree( DstBuffer );
	appFree( DstBufferAsync );


	SET_WARN_COLOR(COLOR_WHITE);
	warnf(TEXT(""));
	warnf(TEXT("Complete!"));

	SET_WARN_COLOR(COLOR_YELLOW);
	warnf(TEXT("Size information:"));
	SET_WARN_COLOR(COLOR_GRAY);
	warnf(TEXT("  Uncompressed Size:     %.3fMB"), (FLOAT)UncompressedSize / (1024.0f * 1024.0f));
	warnf(TEXT("  Compressed Size:       %.3fMB"), (FLOAT)CompressedSize / (1024.0f * 1024.0f));
	warnf(TEXT("  Ratio:                 %.3f%%"), ((FLOAT)CompressedSize / (FLOAT)UncompressedSize) * 100.0f);
	SET_WARN_COLOR(COLOR_YELLOW);
	warnf(TEXT("Time information:"));
	SET_WARN_COLOR(COLOR_GRAY);
	warnf(TEXT("  Read time:             %.3fs"), StopRead - StartRead);
	warnf(TEXT("  Compress time:         %.3fs"), StopCompress - StartCompress);
}

INT UTestCompressionCommandlet::Main(const FString& Params)
{
	// Parse command line args.
	TArray<FString> Tokens;
	TArray<FString> Switches;
	ParseCommandLine( *Params, Tokens, Switches);

	// a list of all the different tests to run (as defined by the flags)
	TArray<DWORD> CompressionTests;
	UBOOL bAllTests = ParseParam(*Params, TEXT("alltests"));

	if (bAllTests || ParseParam(*Params, TEXT("zlibtest")))
	{
		CompressionTests.AddItem(COMPRESS_ZLIB);
	}
	if (bAllTests || ParseParam(*Params, TEXT("lzxtest")))
	{
		CompressionTests.AddItem(COMPRESS_LZX);
	}
	if (bAllTests || ParseParam(*Params, TEXT("lzotest")))
	{
		CompressionTests.AddItem(COMPRESS_LZO);
	}
	if (bAllTests || ParseParam(*Params, TEXT("lzospeedtest")))
	{
		CompressionTests.AddItem(COMPRESS_LZO | COMPRESS_BiasSpeed);
	}
	if (bAllTests || ParseParam(*Params, TEXT("lzosizetest")))
	{
		CompressionTests.AddItem(COMPRESS_LZO | COMPRESS_BiasMemory);
	}

	// list of all files to compress
	TArray<FString> FilesToCompress;

	if (ParseParam(*Params, TEXT("allpackages")))
	{
		FilesToCompress = GPackageFileCache->GetPackageFileList();
	}
	else
	{
		// iterate over all passed in tokens looking for packages
		for (INT TokenIndex=0; TokenIndex<Tokens.Num(); TokenIndex++ )
		{
			FFilename PackageFilename;
			// See whether filename was found in cache.		
			if (GPackageFileCache->FindPackageFile(*Tokens(TokenIndex), NULL, PackageFilename))
			{
				FilesToCompress.AddItem(*PackageFilename);
			}
		}
	}

	// keep overall stats
	QWORD TotalUncompressedSize = 0;
	QWORD TotalCompressedSize = 0;

	// Iterate over all files doing stuff.
	for (INT FileIndex = 0; FileIndex < FilesToCompress.Num(); FileIndex++)
	{
		for (INT TestIndex = 0; TestIndex < CompressionTests.Num(); TestIndex++)
		{
			DWORD UncompressedSize = 0;
			DWORD CompressedSize = 0;

			// run compression test
			RunTest(FilesToCompress(FileIndex), (ECompressionFlags)CompressionTests(TestIndex), UncompressedSize, CompressedSize);

			// update stats
			TotalUncompressedSize += UncompressedSize;
			TotalCompressedSize += CompressedSize;
		}
	}

	SET_WARN_COLOR(COLOR_YELLOW);
	warnf(TEXT(""));
	warnf(TEXT(""));
	warnf(TEXT("OVERALL STATS"));
	CLEAR_WARN_COLOR();
	warnf(TEXT("  Total Uncompressed:    %.3fMB"), (FLOAT)TotalUncompressedSize / (1024.0f * 1024.0f));
	warnf(TEXT("  Total Compressed:      %.3fMB"), (FLOAT)TotalCompressedSize / (1024.0f * 1024.0f));
	warnf(TEXT("  Total Ratio:           %.3f%%"), ((FLOAT)TotalCompressedSize / (FLOAT)TotalUncompressedSize) * 100.0f);

	return 0;
}
IMPLEMENT_CLASS(UTestCompressionCommandlet);


/*-----------------------------------------------------------------------------
	UFindDuplicateKismetObjectsCommandlet.
-----------------------------------------------------------------------------*/
INT UFindDuplicateKismetObjectsCommandlet::Main( const FString& Params )
{
	const TCHAR* Parms = *Params;

	// skip the assert when a package can not be opened
	UBOOL bDeleteDuplicates		= ParseParam(Parms,TEXT("clean"));
	UBOOL bMapsOnly				= ParseParam(Parms,TEXT("mapsonly"));
	UBOOL bNonMapsOnly			= ParseParam(Parms,TEXT("nonmapsonly"));

	// Retrieve list of all packages in .ini paths.
	TArray<FString> PackageList = GPackageFileCache->GetPackageFileList();
	if( !PackageList.Num() )
		return 0;

	INT GCIndex = 0;

	// Iterate over all packages.
	for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
	{
		const FFilename& Filename = PackageList(PackageIndex);
		// if we only want maps, skip non-maps
		if (bMapsOnly && Filename.GetExtension() != FURL::DefaultMapExt)
		{
			continue;
		}
		if (bNonMapsOnly && Filename.GetExtension() == FURL::DefaultMapExt)
		{
			continue;
		}

		UObject::BeginLoad();
		ULinkerLoad* Linker = UObject::GetPackageLinker( NULL, *Filename, LOAD_Quiet|LOAD_NoWarn|LOAD_NoVerify, NULL, NULL );
		UObject::EndLoad();

		// see if we have any sequences
		UBOOL bHasSequences = FALSE;
		FName SequenceName(TEXT("Sequence"));
		for (INT ExportIndex = 0; ExportIndex < Linker->ExportMap.Num(); ExportIndex++)
		{
			if (Linker->GetExportClassName(ExportIndex) == SequenceName)
			{
				bHasSequences = TRUE;
				break;
			}
		}

		if (!bHasSequences)
		{
			SET_WARN_COLOR(COLOR_DARK_YELLOW);
			warnf(TEXT("Skipping %s (no sequences)"), *Filename);
			CLEAR_WARN_COLOR();
			continue;
		}

		warnf(TEXT("Processing %s"), *Filename);
		// open the package
		UPackage* Package = UObject::LoadPackage( NULL, *Filename, LOAD_NoWarn|LOAD_Quiet );
		if( Package == NULL )
		{
			warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
		}

		UBOOL bDirty = FALSE;
		// look for kismet sequences
		for (TObjectIterator<USequence> It; It; ++It)
		{
			if (!It->IsIn(Package))
			{
				continue;
			}
			TArray<USequenceObject*>& Objects = It->SequenceObjects;

			// n^2 operation looking for duplicate
			for (INT ObjIndex1 = 0; ObjIndex1 < Objects.Num(); ObjIndex1++)
			{
				USequenceObject* Obj1 = Objects(ObjIndex1);
				for (INT ObjIndex2 = ObjIndex1 + 1; ObjIndex2 < Objects.Num(); ObjIndex2++)
				{
					USequenceObject* Obj2 = Objects(ObjIndex2);
					if (Obj1->ObjPosX == Obj2->ObjPosX && Obj1->ObjPosY == Obj2->ObjPosY && 
						Obj1->DrawWidth == Obj2->DrawWidth && Obj1->DrawHeight == Obj2->DrawHeight &&
						Obj1->GetClass() == Obj2->GetClass())
					{
						SET_WARN_COLOR(COLOR_GREEN);
						warnf(TEXT("Two duplicate sequence objects: '%s' and '%s'"), *Obj1->GetFullName(), *Obj2->GetPathName());
						CLEAR_WARN_COLOR();
						if (bDeleteDuplicates)
						{
							Objects.RemoveItem(Obj2);
							bDirty = TRUE;
						}
					}
				}
			}
		}

		if (bDirty)
		{
			SavePackageHelper(Package, Filename);
		}

//		if (bDirty || (PackageIndex % 10) == 0)
		{
			UObject::CollectGarbage(RF_Native);
		}
	}
	return 0;
}
IMPLEMENT_CLASS(UFindDuplicateKismetObjectsCommandlet);

/*-----------------------------------------------------------------------------
	UFixupEmitters commandlet.
-----------------------------------------------------------------------------*/
INT UFixupEmittersCommandlet::Main( const FString& Params )
{
	// 
	FString			Token;
	const TCHAR*	CommandLine		= appCmdLine();
	UBOOL			bForceCheckAll	= FALSE;
	FString			FileToFixup(TEXT(""));

	while (ParseToken(CommandLine, Token, 1))
	{
		if (Token == TEXT("-FORCEALL"))
		{
			bForceCheckAll = TRUE;
		}
		else
		{
			FString PackageName;
			if (GPackageFileCache->FindPackageFile(*Token, NULL, PackageName))
			{
				FileToFixup = FFilename(Token).GetBaseFilename();
			}
			else
			{
				debugf(TEXT("Package %s Not Found."), *Token);
				return -1;
			}
		}
	}


	// Retrieve list of all packages in .ini paths.
	TArray<FString> PackageList = GPackageFileCache->GetPackageFileList();
	if (!PackageList.Num())
		return 0;

	// Determine the list of packages we have to touch...
	TArray<FString> RequiredPackageList;
	RequiredPackageList.Empty(PackageList.Num());

	if (FileToFixup != TEXT(""))
	{
		for (INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++)
		{
			const FFilename& Filename = PackageList(PackageIndex);
			if (appStricmp(*Filename.GetBaseFilename(), *FileToFixup) == 0)
			{
				new(RequiredPackageList)FString(PackageList(PackageIndex));
				break;
			}
		}
	}
	else
	{
		// Iterate over all packages.
		for (INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++)
		{
			const FFilename& Filename = PackageList(PackageIndex);
			if (Filename.GetExtension() == TEXT("U"))
			{
				warnf(NAME_Log, TEXT("Skipping script file %s"), *Filename);
				continue;
			}

			// Assert if package couldn't be opened so we have no chance of messing up saving later packages.
			UPackage* Package = CastChecked<UPackage>(UObject::LoadPackage(NULL, *Filename, 0));
			if (Package)
			{
				if (bForceCheckAll)
				{
					new(RequiredPackageList)FString(PackageList(PackageIndex));
				}
				else
				{
					TArray<UPackage*> Packages;
					Packages.AddItem(Package);

					UBOOL bIsDirty = FALSE;
					// Fix up ParticleSystems
					for (TObjectIterator<UParticleSystem> It; It && !bIsDirty; ++It)
					{
						if (It->IsIn(Package))
						{
							UParticleSystem* PartSys = Cast<UParticleSystem>(*It);
							check(PartSys->GetOuter());

							for (INT i = PartSys->Emitters.Num() - 1; (i >= 0) && !bIsDirty; i--)
							{
								UParticleEmitter* Emitter = PartSys->Emitters(i);
								if (Emitter == NULL)
									continue;

								UObject* EmitterOuter = Emitter->GetOuter();
								if (EmitterOuter != PartSys)
								{
									bIsDirty	= TRUE;
								}

								if (!bIsDirty)
								{
									for (INT LODIndex = 0; (LODIndex < Emitter->LODLevels.Num()) && !bIsDirty; LODIndex++)
									{
										UParticleLODLevel* LODLevel = Emitter->LODLevels(LODIndex);
										if (LODLevel == NULL)
										{
											continue;
										}

										UParticleModule* Module;
										Module = LODLevel->TypeDataModule;
										if (Module)
										{
											if (Module->GetOuter() != EmitterOuter)
											{
												bIsDirty	= TRUE;
											}
										}

										if (!bIsDirty)
										{
											for (INT i = 0; i < LODLevel->Modules.Num(); i++)
											{
												Module = LODLevel->Modules(i);
												if (Module)
												{
													if (Module->GetOuter() != EmitterOuter)
													{
														bIsDirty	= TRUE;
														break;
													}
												}
											}
										}
									}
								}
							}

							if (bIsDirty)
							{
								new(RequiredPackageList)FString(PackageList(PackageIndex));
							}
							else
							{
								warnf(NAME_Log, TEXT("ALREADY CONVERTED? Skipping package %32s"), *Filename);
							}
							break;
						}
					}

					// Fix up ParticleEmitters
					for (TObjectIterator<UParticleEmitter> It; It && !bIsDirty; ++It)
					{
						if (It->IsIn(Package))
						{
							UParticleEmitter* Emitter = *It;
							UParticleSystem* PartSys = Cast<UParticleSystem>(Emitter->GetOuter());
							if (PartSys == NULL)
							{
								bIsDirty	= TRUE;
							}

							if (!bIsDirty)
							{
								UObject* EmitterOuter = Emitter->GetOuter();
								if (EmitterOuter != PartSys)
								{
									bIsDirty	= TRUE;
								}

								if (!bIsDirty)
								{
									for (INT LODIndex = 0; (LODIndex < Emitter->LODLevels.Num()) && !bIsDirty; LODIndex++)
									{
										UParticleLODLevel* LODLevel = Emitter->LODLevels(LODIndex);
										if (LODLevel == NULL)
										{
											continue;
										}

										UParticleModule* Module;
										Module = LODLevel->TypeDataModule;
										if (Module)
										{
											if (Module->GetOuter() != EmitterOuter)
											{
												bIsDirty	= TRUE;
											}
										}

										if (!bIsDirty)
										{
											for (INT i = 0; i < LODLevel->Modules.Num(); i++)
											{
												Module = LODLevel->Modules(i);
												if (Module)
												{
													if (Module->GetOuter() != EmitterOuter)
													{
														bIsDirty	= TRUE;
														break;
													}
												}
											}
										}
									}
								}
							}
						}
						if (bIsDirty)
						{
							new(RequiredPackageList)FString(PackageList(PackageIndex));
						}
						else
						{
							warnf(NAME_Log, TEXT("ALREADY CONVERTED? Skipping package %32s"), *Filename);
						}
						break;
					}
				}
			}
			UObject::CollectGarbage(RF_Native);
		}
	}

	if (!RequiredPackageList.Num())
	{
		warnf(NAME_Log, TEXT("No emitter fixups required!"));
		return 0;
	}

	warnf(NAME_Log, TEXT("Emitter fixups required for %d packages..."), RequiredPackageList.Num());

	// source control object
#if HAVE_SCC
	FSourceControlIntegration* SCC = new FSourceControlIntegration;
#endif

	// Iterate over all packages.
	for (INT PackageIndex = 0; PackageIndex < RequiredPackageList.Num(); PackageIndex++)
	{
		const FFilename& Filename = RequiredPackageList(PackageIndex);

		// Assert if package couldn't be opened so we have no chance of messing up saving later packages.
		UPackage* Package = CastChecked<UPackage>(UObject::LoadPackage(NULL, *Filename, 0));
		check(Package);

		TArray<UPackage*> Packages;
		Packages.AddItem(Package);

		if (GFileManager->IsReadOnly( *Filename))
		{
#if HAVE_SCC
			// update scc status
			SCC->Update(Packages);

			// can we check it out?
			if (Package->SCCCanCheckOut())
			{
				// The package is still available, so do the check out.
				SCC->CheckOut(Package);
			}
#endif

			// if the checkout failed for any reason, this will still be readonly, so we can't clean it up
			const INT	FailCount	= 3;
			INT			iCount		= 0;
			while (GFileManager->IsReadOnly( *Filename) && (iCount < FailCount))
			{
				if (iCount == (FailCount - 1))
				{
					// No warning
				}
				else
				if ((iCount + 1) == (FailCount - 1))
				{
					appMsgf(AMT_OK, TEXT("LAST CHANCE - ABOUT TO SKIP!\nCouldn't check out %s, manually do it, sorry!"), *Filename);
				}
				else
				{
					appMsgf(AMT_OK, TEXT("Couldn't check out %s, manually do it, sorry!"), *Filename);
				}
				iCount++;
			}

			if (iCount >= FailCount)
			{
				appMsgf(AMT_OK, TEXT("Skipping %s"), *Filename);
				continue;
			}
		}
		
		warnf(NAME_Log, TEXT("Loading %s"), *Filename);

		if (Package)
		{
			UBOOL	bIsDirty = FALSE;

			// Fix up ParticleSystems
			for (TObjectIterator<UParticleSystem> It; It; ++It)
			{
				if (It->IsIn(Package))
				{
					UParticleSystem* PartSys = Cast<UParticleSystem>(*It);
					check(PartSys->GetOuter());
					
					UBOOL bResetTabs = FALSE;

					for (INT EmitterIndex = PartSys->Emitters.Num() - 1; EmitterIndex >= 0; EmitterIndex--)
					{
						UParticleEmitter* Emitter = PartSys->Emitters(EmitterIndex);
						if (Emitter == NULL)
							continue;

						UObject* EmitterOuter = Emitter->GetOuter();
						if (EmitterOuter != PartSys)
						{
							// Replace the outer...
							Emitter->Rename(NULL, PartSys, REN_ForceNoResetLoaders);
							debugf(TEXT("Renaming particle emitter %32s from Outer %32s to Outer %32s"),
								*(Emitter->GetName()), *(EmitterOuter->GetName()), *(GetName()));
							bResetTabs = TRUE;
							bIsDirty = TRUE;
						}

						UParticleModule* Module;
						UParticleLODLevel* LODLevel;

						for (INT LODIndex = 0; LODIndex < Emitter->LODLevels.Num(); LODIndex++)
						{
							LODLevel = Emitter->LODLevels(LODIndex);
							if (LODLevel == NULL)
							{
								continue;
							}

							Module = LODLevel->TypeDataModule;
							if (Module)
							{
								UObject* ModuleOuter = Module->GetOuter();
								if (ModuleOuter != EmitterOuter)
								{
									// Replace the outer...
									debugf(TEXT("  Renaming particle module %32s from Outer %32s to Outer %32s"),
										*(Module->GetName()), *(ModuleOuter->GetName()), *(EmitterOuter->GetName()));
									Module->Rename(NULL, EmitterOuter, REN_ForceNoResetLoaders);
									bResetTabs = TRUE;
									bIsDirty = TRUE;
								}
							}

							for (INT ModuleIndex = 0; ModuleIndex < LODLevel->Modules.Num(); ModuleIndex++)
							{
								Module = LODLevel->Modules(ModuleIndex);
								if (Module)
								{
									UObject* ModuleOuter = Module->GetOuter();
									if (ModuleOuter != EmitterOuter)
									{
										// Replace the outer...
										debugf(TEXT("  Renaming particle module %32s from Outer %32s to Outer %32s"),
											*(Module->GetName()), *(ModuleOuter->GetName()), 
											*(EmitterOuter->GetName()));
										Module->Rename(NULL, EmitterOuter, REN_ForceNoResetLoaders);
										bResetTabs = TRUE;
										bIsDirty = TRUE;
									}
								}
							}
						}
					}
					if (bResetTabs)
					{
						if (PartSys->CurveEdSetup)
						{
							PartSys->CurveEdSetup->ResetTabs();
						}
					}
				}
			}

			// Fix up ParticleEmitters
			for (TObjectIterator<UParticleEmitter> It; It; ++It)
			{
				if (It->IsIn(Package))
				{
					UParticleEmitter* Emitter = *It;
					UParticleSystem* PartSys = Cast<UParticleSystem>(Emitter->GetOuter());
					UObject* EmitterOuter = Emitter->GetOuter();

					UParticleModule* Module;
					UParticleLODLevel* LODLevel;
					for (INT LODIndex = 0; LODIndex < Emitter->LODLevels.Num(); LODIndex++)
					{
						LODLevel = Emitter->LODLevels(LODIndex);
						if (LODLevel == NULL)
						{
							continue;
						}

						Module = LODLevel->TypeDataModule;
						if (Module)
						{
							UObject* ModuleOuter = Module->GetOuter();
							if (ModuleOuter != EmitterOuter)
							{
								// Replace the outer...
								debugf(TEXT("  Renaming particle module %32s from Outer %32s to Outer %32s"),
									*(Module->GetName()), *(ModuleOuter->GetName()), *(EmitterOuter->GetName()));
								Module->Rename(NULL, EmitterOuter, REN_ForceNoResetLoaders);
								bIsDirty = TRUE;
							}
						}

						for (INT ModuleIndex = 0; ModuleIndex < LODLevel->Modules.Num(); ModuleIndex++)
						{
							Module = LODLevel->Modules(ModuleIndex);
							if (Module)
							{
								UObject* ModuleOuter = Module->GetOuter();
								if (ModuleOuter != EmitterOuter)
								{
									// Replace the outer...
									debugf(TEXT("  Renaming particle module %32s from Outer %32s to Outer %32s"),
										*(Module->GetName()), *(ModuleOuter->GetName()), 
										*(EmitterOuter->GetName()));
									Module->Rename(NULL, EmitterOuter, REN_ForceNoResetLoaders);
									bIsDirty = TRUE;
								}
							}
						}
					}
				}
			}

			if (bIsDirty)
			{
				UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
				if( World )
				{	
					UObject::SavePackage( Package, World, 0, *Filename, GWarn );
				}
				else
				{
					UObject::SavePackage( Package, NULL, RF_Standalone, *Filename, GWarn );
				}
			}
		}
		UObject::CollectGarbage(RF_Native);
	}

#if HAVE_SCC
	delete SCC;
#endif

	return 0;
}
IMPLEMENT_CLASS(UFixupEmittersCommandlet)

/*-----------------------------------------------------------------------------
	UFindEmitterMismatchedLODs commandlet.
	Used to list particle systems containing emitters with LOD level counts 
	that are different than other emitters in the same particle system.
-----------------------------------------------------------------------------*/
void UFindEmitterMismatchedLODsCommandlet::CheckPackageForMismatchedLODs( const FFilename& Filename, UBOOL bFixup )
{
	UPackage* Package = CastChecked<UPackage>(UObject::LoadPackage(NULL, *Filename, 0));
	if (Package)
	{
		UBOOL bSaveIt = FALSE;

		warnf( NAME_Log, TEXT( "Checking package for mismatched LODs: %s" ), *(Package->GetPathName()));
		// Check all ParticleSystems
		for (TObjectIterator<UParticleSystem> It; It; ++It)
		{
			if (It->IsIn(Package))
			{
				UBOOL bIsMismatched = FALSE;
			
				UParticleSystem* PartSys = Cast<UParticleSystem>(*It);

				INT LODLevelCount = -1;
				INT EmitterZeroLevelCount = (PartSys->Emitters.Num() > 0) ? PartSys->Emitters(0)->LODLevels.Num() : 0;
				for (INT i = PartSys->Emitters.Num() - 1; (i >= 0) && (!bIsMismatched || bFixup); i--)
				{
					UParticleEmitter* Emitter = PartSys->Emitters(i);
					if (Emitter == NULL)
					{
						continue;
					}

					INT CheckLODLevelCount = Emitter->LODLevels.Num();
					if (LODLevelCount == -1)
					{
						// First emitter in system...
						LODLevelCount = CheckLODLevelCount;
					}
					else
					{
						if (LODLevelCount != CheckLODLevelCount)
						{
							bIsMismatched = TRUE;
						}
					}
				}

				if (bIsMismatched == TRUE)
				{
					warnf( NAME_Log, TEXT("\t*** PSys with mismatched LODs - %s"), *(PartSys->GetPathName()));
					if ((bFixup == TRUE) && (EmitterZeroLevelCount > 0))
					{
						UBOOL bFixedIt = TRUE;
						for (INT EmitterIndex = 0; EmitterIndex < PartSys->Emitters.Num(); EmitterIndex++)
						{
							UParticleEmitter* Emitter = PartSys->Emitters(EmitterIndex);
							if (Emitter == NULL)
							{
								continue;
							}
							
							if (Emitter->LODLevels.Num() != EmitterZeroLevelCount)
							{
								// Need to fix this up...
								warnf( NAME_Log, TEXT("\t\tEmitter %2d: Has %2d levels, needs %2d"), 
									EmitterIndex, Emitter->LODLevels.Num(), EmitterZeroLevelCount);

								if (Emitter->LODLevels.Num() < EmitterZeroLevelCount)
								{
									for (INT LODInsertIndex = Emitter->LODLevels.Num(); LODInsertIndex < EmitterZeroLevelCount; LODInsertIndex++)
									{
										if (Emitter->CreateLODLevel(LODInsertIndex) != LODInsertIndex)
										{
											bFixedIt = FALSE;
										}
									}
								}
								else
								if (Emitter->LODLevels.Num() > EmitterZeroLevelCount)
								{
									for (INT LODRemoveIndex = Emitter->LODLevels.Num() - 1; LODRemoveIndex >= EmitterZeroLevelCount; LODRemoveIndex--)
									{
										Emitter->LODLevels.Remove(LODRemoveIndex);
									}
								}
							}
						}

						warnf( NAME_Log, TEXT("\t\tFix up - %s"), bFixedIt ? TEXT("COMPLETED") : TEXT("FAILED"));

						bSaveIt = TRUE;
					}
				}
			}
		}

		if (bSaveIt)
		{
			Package->FullyLoad();
			UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
			if( World )
			{	
				UObject::SavePackage( Package, World, 0, *Filename, GWarn );
			}
			else
			{
				UObject::SavePackage( Package, NULL, RF_Standalone, *Filename, GWarn );
			}
		}

		UObject::CollectGarbage(RF_Native);
	}
}

INT UFindEmitterMismatchedLODsCommandlet::Main( const FString& Params )
{
	// 
	FString			Token;
	const TCHAR*	CommandLine = appCmdLine();
	UBOOL			bForceCheckAll = FALSE;
	FString			FileToFixup(TEXT(""));
	UBOOL			bFixup = FALSE;
	UBOOL			bProcessMaps = FALSE;

	while (ParseToken(CommandLine, Token, 1))
	{
		if (Token == TEXT("-FIXUP"))
		{
			bFixup = TRUE;
		}
		else
		if (Token == TEXT("-MAPS"))
		{
			bProcessMaps = TRUE;
		}
		else
		if (Token == TEXT("-FORCEALL"))
		{
			bForceCheckAll = TRUE;
		}
		else
		{
			FString PackageName;
			if (GPackageFileCache->FindPackageFile(*Token, NULL, PackageName))
			{
				FileToFixup = FFilename(Token).GetBaseFilename();
			}
			else
			{
				debugf(TEXT("Package %s Not Found."), *Token);
				return -1;
			}
		}
	}


	// Retrieve list of all packages in .ini paths.
	TArray<FString> PackageList = GPackageFileCache->GetPackageFileList();
	if (!PackageList.Num())
		return 0;

	// Determine the list of packages we have to touch...
	TArray<FString> RequiredPackageList;
	RequiredPackageList.Empty(PackageList.Num());

	if (FileToFixup != TEXT(""))
	{
		for (INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++)
		{
			const FFilename& Filename = PackageList(PackageIndex);
			if (appStricmp(*Filename.GetBaseFilename(), *FileToFixup) == 0)
			{
				CheckPackageForMismatchedLODs(Filename, bFixup);
			}
		}
	}
	else
	{
		// Iterate over all packages.
		for (INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++)
		{
			const FFilename& Filename = PackageList(PackageIndex);
			if (Filename.GetExtension() == TEXT("U"))
			{
				warnf(NAME_Log, TEXT("Skipping script file %s"), *Filename);
				continue;
			}
			if (appStristr(*(Filename), TEXT("ShaderCache")) != NULL)
			{
				warnf(NAME_Log, TEXT("Skipping ShaderCache file %s"), *Filename);
				continue;
			}
			if (appStristr(*(Filename.GetPath()), TEXT("Autosave")) != NULL)
			{
				warnf(NAME_Log, TEXT("Skipping Autosave file %s"), *Filename);
				continue;
			}
			// if we don't want to load maps 
			if ((bProcessMaps == FALSE) && (Filename.GetExtension() == FURL::DefaultMapExt))
			{
				warnf(NAME_Log, TEXT("Skipping map file %s"), *Filename);
				continue;
			}

			CheckPackageForMismatchedLODs(Filename, bFixup);
		}
	}
	return 0;
}

IMPLEMENT_CLASS(UFindEmitterMismatchedLODsCommandlet)

/*-----------------------------------------------------------------------------
	UFindEmitterModifiedLODs commandlet.
	Used to list particle systems containing emitters with LOD levels that are
	marked as modified. (Indicating they should be examined to ensure they are
	'in-sync' with the high-lod level.)
-----------------------------------------------------------------------------*/
INT UFindEmitterModifiedLODsCommandlet::Main( const FString& Params )
{
	// 
	FString			Token;
	const TCHAR*	CommandLine		= appCmdLine();
	UBOOL			bForceCheckAll	= FALSE;
	FString			FileToFixup(TEXT(""));

	while (ParseToken(CommandLine, Token, 1))
	{
		if (Token == TEXT("-FORCEALL"))
		{
			bForceCheckAll = TRUE;
		}
		else
		{
			FString PackageName;
			if (GPackageFileCache->FindPackageFile(*Token, NULL, PackageName))
			{
				FileToFixup = FFilename(Token).GetBaseFilename();
			}
			else
			{
				debugf(TEXT("Package %s Not Found."), *Token);
				return -1;
			}
		}
	}


	// Retrieve list of all packages in .ini paths.
	TArray<FString> PackageList = GPackageFileCache->GetPackageFileList();
	if (!PackageList.Num())
		return 0;

	// Determine the list of packages we have to touch...
	TArray<FString> RequiredPackageList;
	RequiredPackageList.Empty(PackageList.Num());

	if (FileToFixup != TEXT(""))
	{
		for (INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++)
		{
			const FFilename& Filename = PackageList(PackageIndex);
			if (appStricmp(*Filename.GetBaseFilename(), *FileToFixup) == 0)
			{
				new(RequiredPackageList)FString(PackageList(PackageIndex));
				break;
			}
		}
	}
	else
	{
		// Iterate over all packages.
		for (INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++)
		{
			const FFilename& Filename = PackageList(PackageIndex);
			if (Filename.GetExtension() == TEXT("U"))
			{
				warnf(NAME_Log, TEXT("Skipping script file %s"), *Filename);
				continue;
			}
			if (appStristr(*(Filename), TEXT("ShaderCache")) != NULL)
			{
				warnf(NAME_Log, TEXT("Skipping ShaderCache file %s"), *Filename);
				continue;
			}

			// Assert if package couldn't be opened so we have no chance of messing up saving later packages.
			UPackage* Package = CastChecked<UPackage>(UObject::LoadPackage(NULL, *Filename, 0));
			if (Package)
			{
				// Check all ParticleSystems
				for (TObjectIterator<UParticleSystem> It; It; ++It)
				{
					if (It->IsIn(Package))
					{
						UBOOL bIsEditable = FALSE;
					
						UParticleSystem* PartSys = Cast<UParticleSystem>(*It);

						for (INT i = PartSys->Emitters.Num() - 1; (i >= 0) && !bIsEditable; i--)
						{
							UParticleEmitter* Emitter = PartSys->Emitters(i);
							if (Emitter == NULL)
							{
								continue;
							}

							if (Emitter->LODLevels.Num() > 2)
							{
								bIsEditable = TRUE;
							}

							for (INT LODIndex = 1; (LODIndex < Emitter->LODLevels.Num()) && !bIsEditable; LODIndex++)
							{
								UParticleLODLevel* LODLevel = Emitter->LODLevels(LODIndex);
								if (LODLevel == NULL)
								{
									continue;
								}

								UParticleModule* Module;
								Module = LODLevel->TypeDataModule;
								if (Module && (Module->bEditable == TRUE))
								{
									bIsEditable = TRUE;
									break;
								}

								for (INT i = 0; (i < LODLevel->Modules.Num()) && !bIsEditable; i++)
								{
									Module = LODLevel->Modules(i);
									if (Module && (Module->bEditable == TRUE))
									{
										bIsEditable = TRUE;
										break;
									}
								}
							}
						}
						
						if (bIsEditable == TRUE)
						{
							debugf(TEXT("\t*** PSys with modified LODs - %s"), *(PartSys->GetPathName()));
						}
					}
				}

				UObject::CollectGarbage(RF_Native);
			}
		}
	}
	return 0;
}
IMPLEMENT_CLASS(UFindEmitterModifiedLODsCommandlet)

/*-----------------------------------------------------------------------------
	UFindEmitterModuleLODErrors commandlet.
	Used to list particle systems containing emitters with modules that have
	incorrect LOD validity flags set.
-----------------------------------------------------------------------------*/
UBOOL UFindEmitterModuleLODErrorsCommandlet::CheckModulesForLODErrors( INT LODIndex, INT ModuleIndex, 
	const UParticleEmitter* Emitter, const UParticleModule* CurrModule )
{
	if (CurrModule->LODValidity == 0)
	{
		warnf(NAME_Log, TEXT("\t\tLODValidity Not Set? Module %2d in %2d in PSys %s"), 
			ModuleIndex, LODIndex, *(Emitter->GetOuter()->GetPathName()));
		return FALSE;
	}

	if (CurrModule->IsUsedInLODLevel(LODIndex) == FALSE)
	{
		warnf(NAME_Log, TEXT("\t\tNOT USED? Module %2d in %2d in PSys %s"), 
			ModuleIndex, LODIndex, *(Emitter->GetOuter()->GetPathName()));
		return FALSE;
	}

	UBOOL bResult = TRUE;

	if (LODIndex > 0)
	{
		INT CheckIndex = LODIndex - 1;
		while (CheckIndex >= 0)
		{
			if (CurrModule->IsUsedInLODLevel(CheckIndex))
			{
				// Ensure that it is the same as the one it THINKS it is shared with...
				UParticleLODLevel* CheckLODLevel = Emitter->LODLevels(CheckIndex);

				if (CurrModule->IsA(UParticleModuleSpawn::StaticClass()))
				{
					if (CheckLODLevel->SpawnModule != CurrModule)
					{
						warnf(NAME_Log, TEXT("\t\tMistagged SpawnModule at %2d in PSys %s"), 
							LODIndex, *(Emitter->GetOuter()->GetPathName()));
						bResult = FALSE;
					}
				}
				else
				if (CurrModule->IsA(UParticleModuleRequired::StaticClass()))
				{
					if (CheckLODLevel->RequiredModule != CurrModule)
					{
						warnf(NAME_Log, TEXT("\t\tMistagged RequiredModule at %2d in PSys %s"), 
							LODIndex, *(Emitter->GetOuter()->GetPathName()));
						bResult = FALSE;
					}
				}
				else
				if (CurrModule->IsA(UParticleModuleTypeDataBase::StaticClass()))
				{
					if (CheckLODLevel->TypeDataModule != CurrModule)
					{
						warnf(NAME_Log, TEXT("\t\tMistagged TypeDataModule at %2d in PSys %s"), 
							LODIndex, *(Emitter->GetOuter()->GetPathName()));
						bResult = FALSE;
					}
				}
				else
				{
					if (ModuleIndex >= CheckLODLevel->Modules.Num())
					{
						warnf(NAME_Log, TEXT("\t\tMismatched module count at %2d in PSys %s"), 
							LODIndex, *(Emitter->GetOuter()->GetPathName()));
						bResult = FALSE;
					}
					else
					{
						UParticleModule* CheckModule = CheckLODLevel->Modules(ModuleIndex);
						if (CheckModule != CurrModule)
						{
							warnf(NAME_Log, TEXT("\t\tMistagged module (%2d) at %2d in PSys %s"), 
								ModuleIndex, LODIndex, *(Emitter->GetOuter()->GetPathName()));
							bResult = FALSE;
						}
					}
				}
			}

			CheckIndex--;
		}
	}

	return bResult;
}

void UFindEmitterModuleLODErrorsCommandlet::CheckPackageForModuleLODErrors( const FFilename& Filename, UBOOL bFixup )
{
	UPackage* Package = CastChecked<UPackage>(UObject::LoadPackage(NULL, *Filename, 0));
	if (Package)
	{
		UBOOL bSaveIt = FALSE;

		warnf( NAME_Log, TEXT( "Checking package for module LOD errors: %s" ), *(Package->GetPathName()));
		// Check all ParticleSystems
		for (TObjectIterator<UParticleSystem> It; It; ++It)
		{
			if (It->IsIn(Package))
			{
				UBOOL bIsMismatched = FALSE;
			
				UParticleSystem* PartSys = Cast<UParticleSystem>(*It);

				warnf( NAME_Log, TEXT( "\tChecking particle system: %s" ), *(PartSys->GetPathName()));

				INT EmitterZeroLevelCount = (PartSys->Emitters.Num() > 0) ? PartSys->Emitters(0)->LODLevels.Num() : 0;
				for (INT i = PartSys->Emitters.Num() - 1; (i >= 0) && (!bIsMismatched || bFixup); i--)
				{
					UParticleEmitter* Emitter = PartSys->Emitters(i);
					if (Emitter == NULL)
					{
						continue;
					}

					// For each LOD level, check the validity flags on the modules and
					// verify 'shared' modules are really shared...
					UParticleLODLevel* PrevLOD = NULL;
					UParticleLODLevel* CurrLOD = NULL;

					for (INT LODIndex = 0; LODIndex < Emitter->LODLevels.Num(); LODIndex++)
					{
						CurrLOD = Emitter->LODLevels(LODIndex);
						if (CurrLOD == NULL)
						{
							warnf(NAME_Log, TEXT("\tEmitter %2d, Invalid LOD at %2d in PSys %s"), 
								i, LODIndex, *(PartSys->GetPathName()));
							continue;
						}

						// Check the spawn module
						check(CurrLOD->SpawnModule);
						if (CheckModulesForLODErrors(LODIndex, INDEX_SPAWNMODULE, Emitter, CurrLOD->SpawnModule) == FALSE)
						{
							warnf(NAME_Log, TEXT("\tEmitter %2d, Invalid SpawnModule in LOD %2d: PSys %s"), 
								i, LODIndex, *(PartSys->GetPathName()));
						}

						// Check the required module
						check(CurrLOD->RequiredModule);
						if (CheckModulesForLODErrors(LODIndex, INDEX_REQUIREDMODULE, Emitter, CurrLOD->RequiredModule) == FALSE)
						{
							warnf(NAME_Log, TEXT("\tEmitter %2d, Invalid RequiredModule in LOD %2d: PSys %s"), 
								i, LODIndex, *(PartSys->GetPathName()));
						}

						// Check the type data module (if present)
						if (CurrLOD->TypeDataModule)
						{
							if (PrevLOD)
							{
								if (PrevLOD->TypeDataModule == NULL)
								{
									warnf(NAME_Log, TEXT("\tEmitter %2d, Missing TDM in LOD %2d: PSys %s"), 
										i, PrevLOD->Level, *(PartSys->GetPathName()));
								}

								if (PrevLOD->TypeDataModule->IsA(CurrLOD->TypeDataModule->GetClass()) == FALSE)
								{
									warnf(NAME_Log, TEXT("\tEmitter %2d, Mismatched TDM in LOD %2d: PSys %s"), 
										i, PrevLOD->Level, *(PartSys->GetPathName()));
								}
							}

							if (CheckModulesForLODErrors(LODIndex, INDEX_TYPEDATAMODULE, Emitter, CurrLOD->TypeDataModule) == FALSE)
							{
								warnf(NAME_Log, TEXT("\tEmitter %2d, Invalid TDM in LOD %2d: PSys %s"), 
									i, LODIndex, *(PartSys->GetPathName()));
							}
						}
						else if (PrevLOD && PrevLOD->TypeDataModule)
						{
							warnf(NAME_Log, TEXT("\tEmitter %2d, Missing TDM in LOD %2d: PSys %s"), 
								i, LODIndex, *(PartSys->GetPathName()));
						}

						if (PrevLOD)
						{
							if (PrevLOD->Modules.Num() != CurrLOD->Modules.Num())
							{
								warnf(NAME_Log, TEXT("\tEmitter %2d, Mismatched modules counts in LOD %2d: PSys %s"), 
									i, LODIndex, *(PartSys->GetPathName()));
							}
						}

						// Check the remaining modules
						for (INT ModuleIndex = 0; ModuleIndex < CurrLOD->Modules.Num(); ModuleIndex++)
						{
							UParticleModule* CurrModule = CurrLOD->Modules(ModuleIndex);
							if (PrevLOD)
							{
								if (PrevLOD->Modules.Num() <= ModuleIndex)
								{
									warnf(NAME_Log, TEXT("\tEmitter %2d, Missing module %d in LOD %2d: PSys %s"), 
										i, ModuleIndex, PrevLOD->Level, *(PartSys->GetPathName()));
								}
							}
							if (CheckModulesForLODErrors(LODIndex, ModuleIndex, Emitter, CurrModule) == FALSE)
							{
								warnf(NAME_Log, TEXT("\tEmitter %2d, Invalid module in LOD %2d: PSys %s"), 
									i, LODIndex, *(PartSys->GetPathName()));
							}
						}

						PrevLOD = CurrLOD;
					}
				}
			}
		}

		UObject::CollectGarbage(RF_Native);
	}
}

INT UFindEmitterModuleLODErrorsCommandlet::Main( const FString& Params )
{
	// 
	FString			Token;
	const TCHAR*	CommandLine = appCmdLine();
	UBOOL			bForceCheckAll = FALSE;
	FString			FileToFixup(TEXT(""));
	UBOOL			bFixup = FALSE;
	UBOOL			bProcessMaps = FALSE;

	while (ParseToken(CommandLine, Token, 1))
	{
		if (Token == TEXT("-FIXUP"))
		{
			bFixup = TRUE;
		}
		else
		if (Token == TEXT("-MAPS"))
		{
			bProcessMaps = TRUE;
		}
		else
		if (Token == TEXT("-FORCEALL"))
		{
			bForceCheckAll = TRUE;
		}
		else
		{
			FString PackageName;
			if (GPackageFileCache->FindPackageFile(*Token, NULL, PackageName))
			{
				FileToFixup = FFilename(Token).GetBaseFilename();
			}
			else
			{
				debugf(TEXT("Package %s Not Found."), *Token);
				return -1;
			}
		}
	}


	// Retrieve list of all packages in .ini paths.
	TArray<FString> PackageList = GPackageFileCache->GetPackageFileList();
	if (!PackageList.Num())
		return 0;

	// Determine the list of packages we have to touch...
	TArray<FString> RequiredPackageList;
	RequiredPackageList.Empty(PackageList.Num());

	if (FileToFixup != TEXT(""))
	{
		for (INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++)
		{
			const FFilename& Filename = PackageList(PackageIndex);
			if (appStricmp(*Filename.GetBaseFilename(), *FileToFixup) == 0)
			{
				CheckPackageForModuleLODErrors(Filename, bFixup);
			}
		}
	}
	else
	{
		// Iterate over all packages.
		for (INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++)
		{
			const FFilename& Filename = PackageList(PackageIndex);
			if (Filename.GetExtension() == TEXT("U"))
			{
				warnf(NAME_Log, TEXT("Skipping script file %s"), *Filename);
				continue;
			}
			if (appStristr(*(Filename), TEXT("ShaderCache")) != NULL)
			{
				warnf(NAME_Log, TEXT("Skipping ShaderCache file %s"), *Filename);
				continue;
			}
			if (appStristr(*(Filename.GetPath()), TEXT("Autosave")) != NULL)
			{
				warnf(NAME_Log, TEXT("Skipping Autosave file %s"), *Filename);
				continue;
			}
			// if we don't want to load maps 
			if ((bProcessMaps == FALSE) && (Filename.GetExtension() == FURL::DefaultMapExt))
			{
				warnf(NAME_Log, TEXT("Skipping map file %s"), *Filename);
				continue;
			}

			CheckPackageForModuleLODErrors(Filename, bFixup);
		}
	}
	return 0;
}

IMPLEMENT_CLASS(UFindEmitterModuleLODErrorsCommandlet)

/**
 *	UFixupSourceUVsCommandlet.
 *
 *	Some source UVs in static meshes were corrupted on import - this commandlet attempts to fix them.
 *
 *  The general pattern is the UV coords appear as [0][0*n], [0][1*n] and [0][2*n] when they should be [0][0], [1][0] and [2][0]
 *  I also tried when the UV coords appear as [0][0], [1][1] and [2][0] but this is not fixable
 *
 *  I have tried to write it to not give false positives.
 */
INT UFixupSourceUVsCommandlet::Main( const FString & Params )
{
	// Parse command line args.
	FString			FullyQualified;
	TArray<FString> Tokens;
	TArray<FString> Switches;
	TArray<FString> FilesInPath;
	UBOOL			bResult, bPackageDirty;
	INT				Incorrect, Failed, FixType, i;

	const TCHAR * Parms = *Params;
	ParseCommandLine( Parms, Tokens, Switches );

	if( Tokens.Num() > 0 )
	{
		for( i = 0; i < Tokens.Num(); i++ )
		{
			GPackageFileCache->FindPackageFile( *Tokens( i ), NULL, FullyQualified );
			new( FilesInPath ) FString( FullyQualified );
		}
	}
	else
	{
		FilesInPath = GPackageFileCache->GetPackageFileList();
	}

	if( FilesInPath.Num() == 0 )
	{
		warnf( NAME_Warning, TEXT( "No packages found" ) );
		return 1;
	}

	Incorrect = 0;
	Failed = 0;

	// Iterate over all files checking for incorrect source UVs
	for( INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++ )
	{
		const FFilename & Filename = FilesInPath( FileIndex );
		warnf( NAME_Log, TEXT( "Loading %s" ), *Filename );
		bPackageDirty = FALSE;

		UPackage * Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
		if( Package == NULL )
		{
			warnf( NAME_Error, TEXT( "Error loading %s!" ), *Filename );
			continue;
		}

		for( TObjectIterator<UStaticMesh>It; It; ++It )
		{
			UStaticMesh * StaticMesh = *It;
			if( StaticMesh->IsIn( Package ) )
			{
				FStaticMeshRenderData * StaticMeshRenderData = &StaticMesh->LODModels( 0 );
				FStaticMeshTriangle * RawTriangleData = ( FStaticMeshTriangle * )StaticMeshRenderData->RawTriangles.Lock( LOCK_READ_ONLY );

				if( RawTriangleData->NumUVs < 0 || RawTriangleData->NumUVs > 8 )
				{
					warnf( NAME_Warning, TEXT( "Bad number of raw UV channels (%d) in %s" ), RawTriangleData->NumUVs, *StaticMesh->GetName() );
					StaticMeshRenderData->RawTriangles.Unlock();
					continue;	
				}

				bResult = CheckUVs( StaticMeshRenderData, RawTriangleData );
				StaticMeshRenderData->RawTriangles.Unlock();

				if( !bResult )
				{
					warnf( NAME_Log, TEXT( "UV source data incorrect in %s - fixing" ), *StaticMesh->GetName() );
					Incorrect++;

					FStaticMeshTriangle* RawTriangleData = ( FStaticMeshTriangle* )StaticMeshRenderData->RawTriangles.Lock( LOCK_READ_WRITE );
					FixType = CheckFixable( RawTriangleData, StaticMeshRenderData->RawTriangles.GetElementCount() );
					if( FixType < 0 )
					{
						warnf( NAME_Warning, TEXT( "UV source data degenerate or invalid in %s - please reimport" ), *StaticMesh->GetName() );
						Failed++;
					}
					else
					{
						FixupUVs( FixType, RawTriangleData, StaticMeshRenderData->RawTriangles.GetElementCount() );
						bPackageDirty = TRUE;
					}

					StaticMeshRenderData->RawTriangles.Unlock();
				}
			}
		}

		// Write out the package if necessary
		if( bPackageDirty )
		{
			bResult = SavePackageHelper(Package, Filename);

			if( !bResult )
			{
				warnf( NAME_Log, TEXT( "Failed to save: %s" ), *Filename );
			}
		}

		UObject::CollectGarbage( RF_Native );
	}

	warnf( NAME_Log, TEXT( "%d source static mesh(es) with incorrect UV data, %d failed to fix up" ), Incorrect, Failed );
	return( 0 );
}

/**
 *	FixupUVs
 *
 *  Actually remap the UV coords
 */
void UFixupSourceUVsCommandlet::FixupUVs( INT step, FStaticMeshTriangle * RawTriangleData, INT NumRawTriangles )
{
	INT			i, j, k;

	for( i = 0; i < NumRawTriangles; i++ )
	{
		check( RawTriangleData[i].NumUVs < 3 );

		// Copy in the valid UVs
		for( j = 0; j < RawTriangleData[i].NumUVs; j++ )
		{
			for( k = 0; k < 3; k++ )
			{
				RawTriangleData[i].UVs[k][j] = RawTriangleData[i].UVs[0][k * step];
			}
		}

		// Zero out the remainder
		for( j = RawTriangleData[i].NumUVs; j < 8; j++ )
		{
			for( k = 0; k < 3; k++ )
			{
				RawTriangleData[i].UVs[k][j].X = 0.0f;
				RawTriangleData[i].UVs[k][j].Y = 0.0f;
			}
		}
	}
}

/**
 *	ValidateUVChannels
 *
 *  Make sure there are a valid number of UV channels that can be fixed up
 */
UBOOL UFixupSourceUVsCommandlet::ValidateUVChannels( const FStaticMeshTriangle * RawTriangleData, INT NumRawTriangles )
{
	INT			i, UVChannelCount;
	FVector2D	UVsBad[8][3];

	UVChannelCount = RawTriangleData[0].NumUVs;
	check( UVChannelCount > 0 );
	if( UVChannelCount > 2 )
	{
		return( FALSE );
	}

	// Check to make sure all source tris have the same number of UV channels
	for( i = 0; i < NumRawTriangles; i++ )
	{
		if( RawTriangleData[i].NumUVs != UVChannelCount )
		{
			return( FALSE );
		}
	}

	return( TRUE );
}

/**
 *	CheckFixableType
 *
 *  Check to see if the UVs were serialized out incorrectly
 */
UBOOL UFixupSourceUVsCommandlet::CheckFixableType( INT step, const FStaticMeshTriangle * RawTriangleData, INT NumRawTriangles )
{
	INT			i, j, UVChannelCount;
	INT			NullCounts[6];

	UVChannelCount = RawTriangleData[0].NumUVs;
	for( j = 0; j < UVChannelCount * 3; j++ )
	{
		NullCounts[j] = 0;
	}

	for( i = 0; i < NumRawTriangles; i++ )
	{
		// Check to make sure the UVs are all valid
		for( j = 0; j < UVChannelCount * 3; j++ )
		{
			if( appIsNaN( RawTriangleData[i].UVs[0][j * step].X ) || appIsNaN( RawTriangleData[i].UVs[0][j * step].Y ) )
			{
				return( FALSE );
			}

			if( !appIsFinite( RawTriangleData[i].UVs[0][j * step].X ) && !appIsFinite( RawTriangleData[i].UVs[0][j * step].Y ) )
			{
				return( FALSE );
			}

			// Cull 0.0 and #DEN
			if( fabs( RawTriangleData[i].UVs[0][j * step].X ) > SMALL_NUMBER || fabs( RawTriangleData[i].UVs[0][j * step].Y ) > SMALL_NUMBER )
			{
				continue;
			}

			NullCounts[j]++;
		}
	}

	// If any coord is zeroed out more than half the time, it's likley incorrect
	for( j = 0; j < UVChannelCount * 3; j++ )
	{
		if( NullCounts[j] > NumRawTriangles / 2 )
		{
			return( FALSE );
		}
	}

	return( TRUE );
}

/**
 *	CheckFixable
 */
INT UFixupSourceUVsCommandlet::CheckFixable( const FStaticMeshTriangle * RawTriangleData, INT NumRawTriangles )
{
	INT		i;

	if( !ValidateUVChannels( RawTriangleData, NumRawTriangles ) )
	{
		return( -1 );
	}

	for( i = 1; i < 4; i++ )
	{
		if( CheckFixableType( i, RawTriangleData, NumRawTriangles ) )
		{
			return( i );
		}
	}

	return( -1 );
}

/**
 *	FindUV
 *
 *  Find if a UV coord exists anywhere in the source static mesh data
 */
UBOOL UFixupSourceUVsCommandlet::FindUV( const FStaticMeshTriangle * RawTriangleData, INT UVChannel, INT NumRawTriangles, const FVector2D & UV )
{
	INT		i;

	for( i = 0; i < NumRawTriangles; i++ )
	{
		if( RawTriangleData[i].UVs[0][UVChannel] == UV )
		{
			return( TRUE );
		}

		if( RawTriangleData[i].UVs[1][UVChannel] == UV )
		{
			return( TRUE );
		}

		if( RawTriangleData[i].UVs[2][UVChannel] == UV )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}

/**
 *	CheckUVs
 *
 *  Iterate over the final mesh data and check to see if every element of UV data exists somewhere in the source static mesh
 */
UBOOL UFixupSourceUVsCommandlet::CheckUVs( FStaticMeshRenderData * StaticMeshRenderData, const FStaticMeshTriangle * RawTriangleData )
{
	// As the conversion from raw triangles to indexed elements is non trivial and we are checking for a transposition error, just make sure
	// every UV pair in the final data exists somewhere in the raw data.
	INT NumRawTriangles = StaticMeshRenderData->RawTriangles.GetElementCount();

	for( UINT i = 0; i < StaticMeshRenderData->VertexBuffer.GetNumTexCoords(); i++ )
	{
		for( UINT j = 0; j < StaticMeshRenderData->NumVertices; j++ )
		{
			if( !FindUV( RawTriangleData, i, NumRawTriangles, StaticMeshRenderData->VertexBuffer.GetVertexUV(j,i) ) )
			{
				return( FALSE );
			}
		}
	}
	
	return( TRUE );
}

IMPLEMENT_CLASS( UFixupSourceUVsCommandlet );


/**
 *	UPIEToNormalCommandlet.
 *
 *	Take a map saved for PIE and make it openable in the editor
 */
INT UPIEToNormalCommandlet::Main( const FString & Params )
{
	// parse the command line
	TArray<FString> Tokens;
	TArray<FString> Switches;
	const TCHAR * Parms = *Params;
	ParseCommandLine( Parms, Tokens, Switches );

	// validate the input
	FString MapPath;
	FString OutputPath;
	if (Tokens.Num() > 0)
	{
		GPackageFileCache->FindPackageFile(*Tokens(0), NULL, MapPath);
	}
	else
	{
		warnf(TEXT("No map specified:\n\tPIEToNormal InputPIEMap [OptionalOutputPath]\n\tNote: If no output specified, the input will be saved over."));
		return 1;
	}

	// get where we want to write to
	if (Tokens.Num() > 1)
	{
		OutputPath = Tokens(1);
	}
	else
	{
		OutputPath = MapPath;
	}

	UPackage* Package = UObject::LoadPackage(NULL, *MapPath, 0);
	if (!Package)
	{
		warnf(TEXT("File %s not found"), *MapPath);
		return 1;
	}

	// make sure there's a world in it (ie a map)
	UWorld* World = FindObject<UWorld>(Package, TEXT("TheWorld"));
	if (World == NULL || (Package->PackageFlags & PKG_PlayInEditor) == 0)
	{
		warnf(TEXT("Package %s was not a map saved via Play In Editor"), *MapPath);
		return 1;
	}

	// first, strip PIE flag
	Package->PackageFlags &= ~PKG_PlayInEditor;

	// next, undo the PIE changes to streaming levels
	AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
	for (INT LevelIndex=0; LevelIndex<WorldInfo->StreamingLevels.Num(); LevelIndex++)
	{
		ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
		// is it a used streaming level?
		if (StreamingLevel && StreamingLevel->PackageName != NAME_None)
		{
			// cache the package name
			FString StreamingPackageName = StreamingLevel->PackageName.ToString();
			
			// it needs to start with the playworld prefix
			check(StreamingPackageName.StartsWith(PLAYWORLD_PACKAGE_PREFIX));

			// strip off the PIE prefix
			StreamingLevel->PackageName = FName(*StreamingPackageName.Right(StreamingPackageName.Len() - appStrlen(PLAYWORLD_PACKAGE_PREFIX)));
		}
	}


	// delete the play from here teleporter
	for (TObjectIterator<ATeleporter> It; It; ++It)
	{
		// is it the special teleporter?
		if (It->Tag == TEXT("PlayWorldStart"))
		{
			// if so, remove it from nav list and destroy it
			World->PersistentLevel->RemoveFromNavList(*It);
			World->DestroyActor(*It);
			break;
		}
	}

	// save the result
	if (!SavePackageHelper(Package, OutputPath))
	{
		warnf(TEXT("Failed to save to output pacakge %s"), *OutputPath);
	}
	

	return 0;
}
IMPLEMENT_CLASS(UPIEToNormalCommandlet);

/*-----------------------------------------------------------------------------
	UWrangleContent.
-----------------------------------------------------------------------------*/

/** 
 * Helper struct to store information about a unreferenced object
 */
struct FUnreferencedObject
{
	/** Name of package this object resides in */
	FString PackageName;
	/** Full name of object */
	FString ObjectName;
	/** Size on disk as recorded in FObjectExport */
	INT SerialSize;

	/**
	 * Constructor for easy creation in a TArray
	 */
	FUnreferencedObject(const FString& InPackageName, const FString& InObjectName, INT InSerialSize)
	: PackageName(InPackageName)
	, ObjectName(InObjectName)
	, SerialSize(InSerialSize)
	{
	}
};

/**
 * Helper struct to store information about referenced objects insde
 * a package. Stored in TMap<> by package name, so this doesn't need
 * to store the package name 
 */
struct FPackageObjects
{
	/** All objected referenced in this package, and their class */
	TMap<FString, UClass*> ReferencedObjects;

	/** Was this package a fully loaded package, and saved right after being loaded? */
	UBOOL bIsFullyLoadedPackage;

	FPackageObjects()
	: bIsFullyLoadedPackage(FALSE)
	{
	}

};
	
FArchive& operator<<(FArchive& Ar, FPackageObjects& PackageObjects)
{
	Ar << PackageObjects.bIsFullyLoadedPackage;

	if (Ar.IsLoading())
	{
		INT NumObjects;
		FString ObjectName;
		FString ClassName;

		Ar << NumObjects;
		for (INT ObjIndex = 0; ObjIndex < NumObjects; ObjIndex++)
		{
			Ar << ObjectName << ClassName;
			UClass* Class = UObject::StaticLoadClass(UObject::StaticClass(), NULL, *ClassName, NULL, LOAD_None, NULL);
			PackageObjects.ReferencedObjects.Set(*ObjectName, Class);
		}
	}
	else if (Ar.IsSaving())
	{
		INT NumObjects = PackageObjects.ReferencedObjects.Num();
		Ar << NumObjects;
		for (TMap<FString, UClass*>::TIterator It(PackageObjects.ReferencedObjects); It; ++It)
		{
			FString ObjectName, ClassName;
			ObjectName = It.Key();
			ClassName = It.Value()->GetPathName();

			Ar << ObjectName << ClassName;
		}
		
	}

	return Ar;
}

/**
 * Stores the fact that an object (given just a name) was referenced
 *
 * @param PackageName Name of the package the object lives in
 * @param ObjectName FullName of the object
 * @param ObjectClass Class of the object
 * @param ObjectRefs Map to store the object information in
 * @param bIsFullLoadedPackage TRUE if the packge this object is in was fully loaded
 */
void ReferenceObjectInner(const FString& PackageName, const FString& ObjectName, UClass* ObjectClass, TMap<FString, FPackageObjects>& ObjectRefs, UBOOL bIsFullyLoadedPackage)
{
	// look for an existing FPackageObjects
	FPackageObjects* PackageObjs = ObjectRefs.Find(*PackageName);
	// if it wasn't found make a new entry in the map
	if (PackageObjs == NULL)
	{
		PackageObjs = &ObjectRefs.Set(*PackageName, FPackageObjects());
	}

	// if either the package was already marked as fully loaded or it now is fully loaded, then
	// it will be fully loaded
	PackageObjs->bIsFullyLoadedPackage = PackageObjs->bIsFullyLoadedPackage || bIsFullyLoadedPackage;

	// add this referenced object to the map
	PackageObjs->ReferencedObjects.Set(*ObjectName, ObjectClass);

	// make sure the class is in the root set so it doesn't get GC'd, making the pointer we cached invalid
	ObjectClass->AddToRoot();
}

/**
 * Stores the fact that an object was referenced
 *
 * @param Object The object that was referenced
 * @param ObjectRefs Map to store the object information in
 * @param bIsFullLoadedPackage TRUE if the package this object is in was fully loaded
 */
void ReferenceObject(UObject* Object, TMap<FString, FPackageObjects>& ObjectRefs, UBOOL bIsFullyLoadedPackage)
{
	FString PackageName = Object->GetOutermost()->GetName();

	// find the outermost non-upackage object, as it will be loaded later with all its subobjects
	while (Object->GetOuter() && Object->GetOuter()->GetClass() != UPackage::StaticClass())
	{
		Object = Object->GetOuter();
	}

	// make sure this object is valid (it's not in a script or native-only package)
	UBOOL bIsValid = TRUE;
	// can't be in a script packge or be a field/template in a native package, or a top level pacakge, or in the transient package
	if ((Object->GetOutermost()->PackageFlags & PKG_ContainsScript) ||
		Object->IsA(UField::StaticClass()) ||
		Object->IsTemplate() ||
		Object->GetOuter() == NULL ||
		Object->IsIn(UObject::GetTransientPackage()))
	{
		bIsValid = FALSE;
	}

	if (bIsValid)
	{
		// save the reference
		ReferenceObjectInner(PackageName, Object->GetFullName(), Object->GetClass(), ObjectRefs, bIsFullyLoadedPackage);

		// add the other language versions
	    for (INT LangIndex = 0; GKnownLanguageExtensions[LangIndex]; LangIndex++)
		{
			// see if a localized package for this package exists
			FString LocPath;
			FString LocPackageName = PackageName + TEXT("_") + GKnownLanguageExtensions[LangIndex];
			if (GPackageFileCache->FindPackageFile(*LocPackageName, NULL, LocPath))
			{
				// make the localized object name (it doesn't even have to exist)
				FString LocObjectName = FString::Printf(TEXT("%s %s.%s"), *Object->GetClass()->GetName(), *LocPackageName, *Object->GetPathName(Object->GetOutermost()));

				// save a reference to the (possibly existing) localized object (these won't have been in a fully loaded package)
				ReferenceObjectInner(LocPackageName, LocObjectName, Object->GetClass(), ObjectRefs, FALSE);
			}
		}
	}
}

/**
 * Take a package pathname and return a path for where to save the cutdown
 * version of the package. Will create the directory if needed.
 *
 * @param Filename Path to a package file
 * @param CutdownDirectoryName Name of the directory to put this package into
 *
 * @return Location to save the cutdown package
 */
FFilename MakeCutdownFilename(const FFilename& Filename, const TCHAR* CutdownDirectoryName=TEXT("CutdownPackages"))
{
	// replace the .. with ..\GAMENAME\CutdownContent
	FFilename CutdownDirectory = Filename.GetPath();
	CutdownDirectory = CutdownDirectory.Replace(TEXT("..\\"), *FString::Printf(TEXT("..\\%sGame\\%s\\"), appGetGameName(), CutdownDirectoryName));

	// make sure it exists
	GFileManager->MakeDirectory(*CutdownDirectory, TRUE);

	// return the full pathname
	return CutdownDirectory * Filename.GetCleanFilename();
}

/**
 * Removes editor-only data for a package
 *
 * @param Package THe package to remove editor date from all objects
 * @param bStripTextures If TRUE, will strip texture objects of editor data
 * @param bStripSounds If TRUE, will strip sound objects of editor data
 * @param bStripEverything If TRUE, will strip all objects of editor data
 */
void StripEditorData(UPackage* Package, UBOOL bStripTextures, UBOOL bStripSounds, UBOOL bStripEverything)
{
	for( TObjectIterator<UObject> It; It; ++It )
	{
		if (It->IsIn(Package) && 
				(
					bStripEverything || 
					(bStripTextures && It->IsA(UTexture::StaticClass())) || 
					(bStripSounds && It->IsA(USoundNodeWave::StaticClass()))
				)
			)
		{
			It->StripData(UE3::PLATFORM_Unknown);
		}
	}
}

INT UWrangleContentCommandlet::Main( const FString& Params )
{
	// does the user want to save the stripped down content?
	UBOOL bShouldRestoreFromPreviousRun = ParseParam(*Params, TEXT("restore"));
	UBOOL bShouldSavePackages = !ParseParam(*Params, TEXT("nosave"));
	UBOOL bShouldSaveUnreferencedContent = !ParseParam(*Params, TEXT("nosaveunreferenced"));
	UBOOL bShouldDumpUnreferencedContent = ParseParam(*Params, TEXT("reportunreferenced"));
	UBOOL bShouldRemoveEditorOnlyData = ParseParam(*Params, TEXT("removeeditoronly"));
	UBOOL bShouldStripTextures = ParseParam(*Params, TEXT("striptextures"));
	UBOOL bShouldStripSounds = ParseParam(*Params, TEXT("stripsounds"));
	UBOOL bShouldUseDemoSection = ParseParam(*Params, TEXT("demosection"));
	UBOOL bShouldUseRuntimeSection = ParseParam(*Params, TEXT("runtimesection"));

	// store all referenced objects
	TMap<FString, FPackageObjects> AllReferencedPublicObjects;

	if (bShouldRestoreFromPreviousRun)
	{
		FArchive* Ar = GFileManager->CreateFileReader(*(appGameDir() + TEXT("Wrangle.bin")));
		if( Ar != NULL )
		{
			*Ar << AllReferencedPublicObjects;
			delete Ar;
		}
		else
		{
			warnf(TEXT("Could not read in Wrangle.bin so not restoring and doing a full wrangle") );

		}
	}
	else
	{
		// make name for our ini file to control loading
		FString WrangleContentIniName =	appGameConfigDir() + TEXT("WrangleContent.ini");

		// figure out which sectio to use to get the packages to fully load
		FString SectionToUse = TEXT("WrangleContent.PackagesToFullyLoad");
		if (bShouldUseDemoSection)
		{
			SectionToUse = TEXT("WrangleContent.DemoPackagesToFullyLoad");
		}
		else if (bShouldUseRuntimeSection)
		{
			SectionToUse = TEXT("WrangleContent.RuntimePackagesToFullyLoad");
		}

		// get a list of packages to load
		TMultiMap<FString,FString>* PackagesToFullyLoad = GConfig->GetSectionPrivate( *SectionToUse, 0, 1, *WrangleContentIniName );
		TMultiMap<FString,FString>* PackagesToAlwaysCook = GConfig->GetSectionPrivate( TEXT("Engine.PackagesToAlwaysCook"), 0, 1, GEngineIni );
		TMultiMap<FString,FString>* StartupPackages = GConfig->GetSectionPrivate( TEXT("Engine.StartupPackages"), 0, 1, GEngineIni );

		// this ini setting isn't required
		if (!PackagesToFullyLoad)
		{
			warnf(NAME_Error, TEXT("This commandlet needs a WrangleContent.ini in the Config directory with a [WrangleContent.PackagesToFullyLoad] section"));
			return 1;
		}

		// move any always cook packages to list of packages to load
		if (PackagesToAlwaysCook)
		{
			for (TMultiMap<FString,FString>::TIterator It(*PackagesToAlwaysCook); It; ++It)
			{
				if (It.Key() == TEXT("Package") || It.Key() == TEXT("SeekFreePackage"))
				{
					PackagesToFullyLoad->Add(*It.Key(), *It.Value());
				}
			}
		}

		// move any startup packages to list of packages to load
		if (StartupPackages)
		{
			for (TMultiMap<FString,FString>::TIterator It(*StartupPackages); It; ++It)
			{
				if (It.Key() == TEXT("Package"))
				{
					PackagesToFullyLoad->Add(*It.Key(), *It.Value());
				}
			}
		}


		// read in the per-map packages to cook
		TMap<FString, TArray<FString> > PerMapCookPackages;
		GConfig->Parse1ToNSectionOfStrings(TEXT("Engine.PackagesToForceCookPerMap"), TEXT("Map"), TEXT("Package"), PerMapCookPackages, GEngineIni);

		// gather any per map packages for cooking
		TArray<FString> PerMapPackagesToLoad;
		for (TMultiMap<FString,FString>::TIterator It(*PackagesToFullyLoad); It; ++It)
		{
			// add dependencies for the per-map packages for this map (if any)
			TArray<FString>* Packages = PerMapCookPackages.Find(It.Value());
			if (Packages != NULL)
			{
				for (INT PackageIndex = 0; PackageIndex < Packages->Num(); PackageIndex++)
				{
					PerMapPackagesToLoad.AddItem(*(*Packages)(PackageIndex));
				}
			}
		}

		// now add them to the list of all packges to load
		for (INT PackageIndex = 0; PackageIndex < PerMapPackagesToLoad.Num(); PackageIndex++)
		{
			PackagesToFullyLoad->Add(TEXT("Package"), *PerMapPackagesToLoad(PackageIndex));
		}

		// make sure all possible script packages are loaded
		TArray<FString> PackageNames;
		appGetGameScriptPackageNames(PackageNames, TRUE);
		appGetGameNativeScriptPackageNames(PackageNames, TRUE);
		appGetEngineScriptPackageNames(PackageNames, TRUE);
		for (INT PackageIndex = 0; PackageIndex < PackageNames.Num(); PackageIndex++)
		{
			warnf(TEXT("Loading script package %s..."), *PackageNames(PackageIndex));
			UObject::LoadPackage(NULL, *PackageNames(PackageIndex), LOAD_None);
		}
		
		// all currently loaded public objects were referenced by script code, so mark it as referenced
		for(FObjectIterator ObjectIt;ObjectIt;++ObjectIt)
		{
			UObject* Object = *ObjectIt;

			// record all public referenced objects
//			if (Object->HasAnyFlags(RF_Public))
			{
				ReferenceObject(Object, AllReferencedPublicObjects, FALSE);
			}
		}

		// go over all the packages that we want to fully load
		for (TMultiMap<FString,FString>::TIterator It(*PackagesToFullyLoad); It; ++It)
		{
			// there may be multiple sublevels to load if this package is a persistent level with sublevels
			TArray<FString> PackagesToLoad;
			// start off just loading this package (more may be added in the loop)
			PackagesToLoad.AddItem(*It.Value());

			for (INT PackageIndex = 0; PackageIndex < PackagesToLoad.Num(); PackageIndex++)
			{
				// save a copy of the packagename (not a reference in case the PackgesToLoad array gets realloced)
				FString PackageName = PackagesToLoad(PackageIndex);
				FFilename PackageFilename;

				if( GPackageFileCache->FindPackageFile( *PackageName, NULL, PackageFilename ) == TRUE )
				{
					warnf(TEXT("Fully loading %s..."), *PackageFilename);

	// @todo josh: track redirects in this package and then save the package instead of copy it if there were redirects
	// or make sure that the following redirects marks the package dirty (which maybe it shouldn't do in the editor?)

					// load the package fully
					UPackage* Package = UObject::LoadPackage(NULL, *PackageFilename, LOAD_None);

					UObject::BeginLoad();
					ULinkerLoad* Linker = UObject::GetPackageLinker( NULL, *PackageFilename, LOAD_Quiet|LOAD_NoWarn|LOAD_NoVerify, NULL, NULL );
					UObject::EndLoad();

					// look for special package types
					UBOOL bIsMap = Linker->ContainsMap();
					UBOOL bIsScriptPackage = Linker->ContainsCode();

					// collect all public objects loaded
					for(FObjectIterator ObjectIt; ObjectIt; ++ObjectIt)
					{
						UObject* Object = *ObjectIt;

						// record all public referenced objects (skipping over top level packages)
						if (/*Object->HasAnyFlags(RF_Public) &&*/ Object->GetOuter() != NULL)
						{
							// is this public object in a fully loaded package?
							UBOOL bIsObjectInFullyLoadedPackage = Object->IsIn(Package);

							if (bIsMap && bIsObjectInFullyLoadedPackage && Object->HasAnyFlags(RF_Public))
							{
								warnf(NAME_Warning, TEXT("Clearing public flag on map object %s"), *Object->GetFullName());
								Object->ClearFlags(RF_Public);
								// mark that we need to save the package since we modified it (instead of copying it)
								Object->MarkPackageDirty();
							}
							else
							{
								// record that this object was referenced
								ReferenceObject(Object, AllReferencedPublicObjects, bIsObjectInFullyLoadedPackage);
							}
						}
					}

					// remove editor-only data if desired
					if (bShouldRemoveEditorOnlyData || bShouldStripTextures || bShouldStripSounds)
					{
						StripEditorData(Package, bShouldStripTextures, bShouldStripSounds, bShouldRemoveEditorOnlyData);
						Package->SetDirtyFlag(TRUE);
					}

					// add any sublevels of this world to the list of levels to load
					for( TObjectIterator<UWorld> It; It; ++It )
					{
						UWorld*		World		= *It;
						AWorldInfo* WorldInfo	= World->GetWorldInfo();
						// iterate over streaming level objects loading the levels.
						for( INT LevelIndex=0; LevelIndex<WorldInfo->StreamingLevels.Num(); LevelIndex++ )
						{
							ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
							if( StreamingLevel )
							{
								// add this sublevel's package to the list of packages to laod
								PackagesToLoad.AddUniqueItem(*StreamingLevel->PackageName.ToString());
							}
						}
					}

					// save/copy the package if desired, and only if it's not a script package (script code is
					// not cutdown, so we always use original script code)
					if (bShouldSavePackages && !bIsScriptPackage)
					{
						// make the name of the location to put the package
						FString CutdownPackageName = MakeCutdownFilename(PackageFilename);
						
						// if the package was modified by loading it, then we should save the package
						if (Package->IsDirty())
						{
							// save the fully load packages
							warnf(TEXT("Saving fully loaded package %s..."), *CutdownPackageName);
							if (!SavePackageHelper(Package, CutdownPackageName))
							{
								warnf(NAME_Error, TEXT("Failed to save package %s..."), *CutdownPackageName);
							}
						}
						else
						{
							warnf(TEXT("Copying fully loaded package %s..."), *CutdownPackageName);
							// copy the unmodified file (faster than saving) (0 is success)
							if (GFileManager->Copy(*CutdownPackageName, *PackageFilename) != 0)
							{
								warnf(NAME_Error, TEXT("Failed to copy package to %s..."), *CutdownPackageName);
							}
						}
					}

					// close this package
					UObject::CollectGarbage(RF_Native);
				}
			}
		}

		// save out the referenced objects so we can restore
		FArchive* Ar = GFileManager->CreateFileWriter(*(appGameDir() + TEXT("Wrangle.bin")));
		*Ar << AllReferencedPublicObjects;
		delete Ar;
	}

	// list of all objects that aren't needed
	TArray<FUnreferencedObject> UnnecessaryPublicObjects;
	TMap<FFilename, FPackageObjects> UnnecessaryObjectsByPackage;
	TMap<FString, UBOOL> UnnecessaryObjects;
	TArray<FFilename> UnnecessaryPackages;

	// now go over all packages, quickly, looking for public objects NOT in the AllNeeded array
	const TArray<FString> AllPackages(GPackageFileCache->GetPackageFileList());

	if (bShouldDumpUnreferencedContent || bShouldSaveUnreferencedContent)
	{
		SET_WARN_COLOR(COLOR_WHITE);
		warnf(TEXT(""));
		warnf(TEXT("Looking for unreferenced objects:"));
		CLEAR_WARN_COLOR();

		// Iterate over all files doing stuff.
		for (INT PackageIndex = 0; PackageIndex < AllPackages.Num(); PackageIndex++)
		{
			FFilename PackageFilename(AllPackages(PackageIndex));
			FString PackageName = PackageFilename.GetBaseFilename();

			// we don't care about trying to wrangle the various shader caches so just skipz0r them
			if(	PackageFilename.GetBaseFilename().InStr( TEXT("LocalShaderCache") )	!= INDEX_NONE
				||	PackageFilename.GetBaseFilename().InStr( TEXT("RefShaderCache") )	!= INDEX_NONE )
			{
				continue;
			}


			// the list of objects in this package
			FPackageObjects* PackageObjs = NULL;

			// this will be set to true if every object in the package is unnecessary
			UBOOL bAreAllObjectsUnnecessary = FALSE;

			if (PackageFilename.GetExtension() == FURL::DefaultMapExt)
			{
				warnf(TEXT("Skipping map %s..."), *PackageFilename);
				continue;
			}
			else if (PackageFilename.GetExtension() == TEXT("u"))
			{
				warnf(TEXT("Skipping script package %s..."), *PackageFilename);
				continue;
			}
			else if (PackageFilename.ToUpper().InStr(TEXT("SHADERCACHE")) != -1)
			{
				warnf(TEXT("Skipping shader cache package %s..."), *PackageFilename);
				continue;
			}
			else
			{
				// get the objects referenced by this package
				PackageObjs = AllReferencedPublicObjects.Find(*PackageName);

				// if the were no objects referenced in this package, we can just skip it, 
				// and mark the whole package as unreferenced
				if (PackageObjs == NULL)
				{
					warnf(TEXT("No objects in %s were referenced..."), *PackageFilename);
					new(UnnecessaryPublicObjects) FUnreferencedObject(PackageName, 
						TEXT("ENTIRE PACKAGE"), GFileManager->FileSize(*PackageFilename));

					// all objects in this package are unnecasstry
					bAreAllObjectsUnnecessary = TRUE;
				}
				else if (PackageObjs->bIsFullyLoadedPackage)
				{
					warnf(TEXT("Skipping fully loaded package %s..."), *PackageFilename);
					continue;
				}
				else
				{
					warnf(TEXT("Scanning %s..."), *PackageFilename);
				}
			}

			UObject::BeginLoad();
			ULinkerLoad* Linker = UObject::GetPackageLinker( NULL, *PackageFilename, LOAD_Quiet|LOAD_NoWarn|LOAD_NoVerify, NULL, NULL );
			UObject::EndLoad();

			// go through the exports in the package, looking for public objects
			for (INT ExportIndex = 0; ExportIndex < Linker->ExportMap.Num(); ExportIndex++)
			{
				FObjectExport& Export = Linker->ExportMap(ExportIndex);
				FString ExportName = Linker->GetExportFullName(ExportIndex);

				// some packages may have brokenness in them so we want to just continue so we can wrangle
				if( Export.ObjectName == NAME_None )
				{
					warnf( TEXT( "    Export.ObjectName == NAME_None  for Package: %s " ), *PackageFilename );
					continue;
				}

				// make sure its outer is a package, and this isn't a package
				if (Linker->GetExportClassName(ExportIndex) == NAME_Package || 
					(Export.OuterIndex != 0 && Linker->GetExportClassName(Export.OuterIndex - 1) != NAME_Package))
				{
					continue;
				}

				// was it not already referenced?
				// NULL means it wasn't in the reffed public objects map for the package
				if (bAreAllObjectsUnnecessary || PackageObjs->ReferencedObjects.Find(ExportName) == NULL)
				{
					// is it public?
					if ((Export.ObjectFlags & RF_Public) != 0 && !bAreAllObjectsUnnecessary)
					{
						// if so, then add it to list of unused pcreateexportublic items
						new(UnnecessaryPublicObjects) FUnreferencedObject(PackageFilename.GetBaseFilename(), ExportName, Export.SerialSize);
					}

					// look for existing entry
					FPackageObjects* ObjectsInPackage = UnnecessaryObjectsByPackage.Find(*PackageFilename);
					// if not found, make a new one
					if (ObjectsInPackage == NULL)
					{
						ObjectsInPackage = &UnnecessaryObjectsByPackage.Set(*PackageFilename, FPackageObjects());
					}

					// get object's class
					check(IS_IMPORT_INDEX(Export.ClassIndex));
					FString ClassName = Linker->GetImportPathName(- Export.ClassIndex - 1);
					UClass* Class = StaticLoadClass(UObject::StaticClass(), NULL, *ClassName, NULL, LOAD_None, NULL);
					check(Class);

					// make sure it doesn't get GC'd
					Class->AddToRoot();
				
					// add this referenced object to the map
					ObjectsInPackage->ReferencedObjects.Set(*ExportName, Class);

					// add this to the map of all unnecessary objects
					UnnecessaryObjects.Set(*ExportName, TRUE);
				}
			}

			// collect garbage every 20 packages (we aren't fully loading, so it doesn't need to be often)
			if ((PackageIndex % 20) == 0)
			{
				UObject::CollectGarbage(RF_Native);
			}
		}
	}

	if (bShouldSavePackages)
	{
		INT NumPackages = AllReferencedPublicObjects.Num();

		// go through all packages, and save out referenced objects
		SET_WARN_COLOR(COLOR_WHITE);
		warnf(TEXT(""));
		warnf(TEXT("Saving referenced objects in %d Packages:"), NumPackages);
		CLEAR_WARN_COLOR();
		INT PackageIndex = 0;
		for (TMap<FString, FPackageObjects>::TIterator It(AllReferencedPublicObjects); It; ++It, PackageIndex++ )
		{
			// we don't care about trying to wrangle the various shader caches so just skipz0r them
			if(	It.Key().InStr( TEXT("LocalShaderCache") ) != INDEX_NONE
				|| It.Key().InStr( TEXT("RefShaderCache") ) != INDEX_NONE )
			{
				continue;
			}

			// if the package was a fully loaded package, than we already saved it
			if (It.Value().bIsFullyLoadedPackage)
			{
				continue;
			}

			// package for all loaded objects
			UPackage* Package = NULL;
			
			// fully load all the referenced objects in the package
			for (TMap<FString, UClass*>::TIterator It2(It.Value().ReferencedObjects); It2; ++It2)
			{
				// get the full object name
				FString ObjectPathName = It2.Key();

				// skip over the class portion (the It2.Value() has the class pointer already)
				INT Space = ObjectPathName.InStr(TEXT(" "));
				check(Space);

				// get everything after the space
				ObjectPathName = ObjectPathName.Right(ObjectPathName.Len() - (Space + 1));

				// load the referenced object

				UObject* Object = UObject::StaticLoadObject(It2.Value(), NULL, *ObjectPathName, NULL, LOAD_NoWarn, NULL);

				// the object may not exist, because of attempting to load localized content
				if (Object)
				{
					check(Object->GetPathName() == ObjectPathName);

					// set the package if needed
					if (Package == NULL)
					{
						Package = Object->GetOutermost();
					}
					else
					{
						// make sure all packages are the same
						check(Package == Object->GetOutermost());
					}
				}
			}

			// make sure we found some objects in here
			if (Package)
			{
				// mark this package as fully loaded so it can be saved, even though we didn't fully load it
				// (which is the point of this commandlet)
				Package->MarkAsFullyLoaded();

				// get original path of package
				FFilename OriginalPackageFilename;

				//warnf( TEXT( "*It.Key(): %s" ), *It.Key() );

				// we need to be able to find the original package
				//verify(GPackageFileCache->FindPackageFile(*It.Key(), NULL, OriginalPackageFilename) == TRUE);
				if( GPackageFileCache->FindPackageFile(*It.Key(), NULL, OriginalPackageFilename) == FALSE )
				{
					appErrorf( TEXT( "Could not find file in file cache: %s"), *It.Key() );
				}

				// any maps need to be fully referenced
				check(OriginalPackageFilename.GetExtension() != FURL::DefaultMapExt);

				// make the filename for the output package
				FString CutdownPackageName = MakeCutdownFilename(OriginalPackageFilename);

				warnf(TEXT("Saving %s... [%d/%d]"), *CutdownPackageName, PackageIndex + 1, NumPackages);

				// remove editor-only data if desired
				if (bShouldRemoveEditorOnlyData || bShouldStripTextures || bShouldStripSounds)
				{
					StripEditorData(Package, bShouldStripTextures, bShouldStripSounds, bShouldRemoveEditorOnlyData);
					Package->SetDirtyFlag(TRUE);
				}

				// save the package now that all needed objects in it are loaded
				SavePackageHelper(Package, *CutdownPackageName);

				// close up this package
				UObject::CollectGarbage(RF_Native);
			}
		}
	}

	if (bShouldDumpUnreferencedContent)
	{
		SET_WARN_COLOR(COLOR_WHITE);
		warnf(TEXT(""));
		warnf(TEXT("Unreferenced Public Objects:"));
		CLEAR_WARN_COLOR();

		// Create string with system time to create a unique filename.
		INT Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec;
		appSystemTime( Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec );
		FString	CurrentTime = FString::Printf(TEXT("%i.%02i.%02i-%02i.%02i.%02i"), Year, Month, Day, Hour, Min, Sec );

		// create a .csv
		FString CSVFilename = FString::Printf(TEXT("%sUnreferencedObjects-%s.csv"), *appGameLogDir(), *CurrentTime);
		FArchive* CSVFile = GFileManager->CreateFileWriter(*CSVFilename);
		if (!CSVFile)
		{
			warnf(NAME_Error, TEXT("Failed to open output file %s"), *CSVFilename);
		}

		for (INT ObjectIndex = 0; ObjectIndex < UnnecessaryPublicObjects.Num(); ObjectIndex++)
		{
			FUnreferencedObject& Object = UnnecessaryPublicObjects(ObjectIndex);
			warnf(*Object.ObjectName);

			// dump out a line to the .csv file
			// @todo: sort by size to Excel's 65536 limit gets the biggest objects
			FString CSVLine = FString::Printf(TEXT("%s,%s,%d%s"), *Object.PackageName, *Object.ObjectName, Object.SerialSize, LINE_TERMINATOR);
			CSVFile->Serialize(TCHAR_TO_ANSI(*CSVLine), CSVLine.Len());
		}
	}

	// load every unnecessary object by package, rename it and any unnecessary objects if uses, to the 
	// an unnecessary package, and save it
	if (bShouldSaveUnreferencedContent)
	{
		INT NumPackages = UnnecessaryObjectsByPackage.Num();
		SET_WARN_COLOR(COLOR_WHITE);
		warnf(TEXT(""));
		warnf(TEXT("Saving unreferenced objects [%d packages]:"), NumPackages);
		CLEAR_WARN_COLOR();

		// go through each package that has unnecessary objects in it
		INT PackageIndex = 0;
		for (TMap<FFilename, FPackageObjects>::TIterator PackageIt(UnnecessaryObjectsByPackage); PackageIt; ++PackageIt, PackageIndex++)
		{
			// we don't care about trying to wrangle the various shader caches so just skipz0r them
			if(	PackageIt.Key().InStr( TEXT("LocalShaderCache") ) != INDEX_NONE
				|| PackageIt.Key().InStr( TEXT("RefShaderCache") ) != INDEX_NONE )
			{
				continue;
			}


			//warnf(TEXT("Processing %s"), *PackageIt.Key());
			UPackage* FullyLoadedPackage = NULL;
			// fully load unnecessary packages with no objects, 
			if (PackageIt.Value().ReferencedObjects.Num() == 0)
			{
				// just load it, and don't need a reference to it
				FullyLoadedPackage = UObject::LoadPackage(NULL, *PackageIt.Key(), LOAD_None);
			}
			else
			{
				// load every unnecessary object in this package
				for (TMap<FString, UClass*>::TIterator ObjectIt(PackageIt.Value().ReferencedObjects); ObjectIt; ++ObjectIt)
				{
					// get the full object name
					FString ObjectPathName = ObjectIt.Key();

					// skip over the class portion (the It2.Value() has the class pointer already)
					INT Space = ObjectPathName.InStr(TEXT(" "));
					check(Space > 0);

					// get everything after the space
					ObjectPathName = ObjectPathName.Right(ObjectPathName.Len() - (Space + 1));

					// load the unnecessary object
					UObject* Object = UObject::StaticLoadObject(ObjectIt.Value(), NULL, *ObjectPathName, NULL, LOAD_NoWarn, NULL);
					
					// this object should exist since it was gotten from a linker
					if (!Object)
					{
						warnf(NAME_Error, TEXT("Failed to load object %s, it will be deleted permanently!"), *ObjectPathName);
					}
				}
			}

			// now find all loaded objects (in any package) that are in marked as unnecessary,
			// and rename them to their destination
			for (TObjectIterator<UObject> It; It; ++It)
			{
				// if was unnecessary...
				if (UnnecessaryObjects.Find(*It->GetFullName()))
				{
					// ... then rename it (its outer needs to be a package, everything else will have to be
					// moved by its outer getting moved)
					if (!It->IsA(UPackage::StaticClass()) &&
						It->GetOuter() &&
						It->GetOuter()->IsA(UPackage::StaticClass()) &&
						It->GetOutermost()->GetName().Left(4) != TEXT("NFS_"))
					{
						UPackage* NewPackage = UObject::CreatePackage(NULL, *(FString(TEXT("NFS_")) + It->GetOuter()->GetPathName()));
						//warnf(TEXT("Renaming object from %s to %s.%s"), *It->GetPathName(), *NewPackage->GetPathName(), *It->GetName());

						// move the object if we can. IF the rename fails, then the object was already renamed to this spot, but not GC'd.
						// that's okay.
						if (It->Rename(*It->GetName(), NewPackage, REN_Test))
						{
							It->Rename(*It->GetName(), NewPackage, REN_None);
						}
					}

				}
			}

			// find the one we moved this packages objects to
			FFilename PackagePath = PackageIt.Key();
			FString PackageName = PackagePath.GetBaseFilename();
			UPackage* MovedPackage = UObject::FindPackage(NULL, *(FString(TEXT("NFS_")) + PackageName));
			check(MovedPackage);

			// convert the new name to a a NFS directory directory
			FFilename MovedFilename = MakeCutdownFilename(FString::Printf(TEXT("%s\\NFS_%s"), *PackagePath.GetPath(), *PackagePath.GetCleanFilename()), TEXT("NFSContent"));
			warnf(TEXT("Saving package %s [%d/%d]"), *MovedFilename, PackageIndex, NumPackages);
			// finally save it out
			SavePackageHelper(MovedPackage, *MovedFilename);

			UObject::CollectGarbage(RF_Native);
		}
	}

	return 0;
}

IMPLEMENT_CLASS(UWrangleContentCommandlet);

/**
 *	UUT3MapStatsCommandlet.
 *
 *	Spits out information on shipping Unreal Tournament 3 maps in CSV format. This assumes that none of the maps are multi-level.
 *
 *	We only care about static mesh actor components to make it easier to gage what needs to be fixed up via major changes.
 */
INT UUT3MapStatsCommandlet::Main( const FString & Params )
{
	// Parse the command line.
	TArray<FString> Tokens;
	TArray<FString> Switches;
	const TCHAR * Parms = *Params;
	ParseCommandLine( Parms, Tokens, Switches );

	// Log at end to avoid other warnings messing up output.
	TArray<FString> GatheredData;
	new(GatheredData) FString( TEXT("Map,Primitive Count,Section Count,Instanced Triangle Count") );

	// Iterate over all packages, weeding out non shipping maps or non- map packages.
	TArray<FString> PackageList = GPackageFileCache->GetPackageFileList();
	for( INT PackageIndex=0; PackageIndex<PackageList.Num(); PackageIndex++ )
	{
		UBOOL bIsShippingMap	= FALSE;
		FFilename Filename		= PackageList(PackageIndex);
		
		if( Filename.GetExtension() == FURL::DefaultMapExt )
		{		
			if( Filename.GetPath().ToUpper().Right( ARRAY_COUNT(TEXT("MAPS"))-1 ) == TEXT("MAPS") )
			{
				bIsShippingMap = TRUE;
			}
			if( Filename.GetPath().ToUpper().Right( ARRAY_COUNT(TEXT("VISUALPROTOTYPES"))-1 ) == TEXT("VISUALPROTOTYPES") )
			{
				bIsShippingMap = TRUE;
			}
			if( Filename.ToUpper().InStr(TEXT("DEMOCONTENT")) != INDEX_NONE )
			{
				bIsShippingMap = FALSE;
			}
			if( Filename.ToUpper().InStr(TEXT("TESTMAPS")) != INDEX_NONE )
			{
				bIsShippingMap = FALSE;
			}
			if( Filename.ToUpper().InStr(TEXT("POC")) != INDEX_NONE )
			{
				bIsShippingMap = FALSE;
			}

			// Gather some information for later. We print it out all at once to not be interrupted by load warnings.
			// @warning: this code assumes that none of the maps are multi-level
			if( bIsShippingMap ) 
			{
				warnf(TEXT("Shipping map: %s"),*Filename);

				UPackage* Package = LoadPackage( NULL, *Filename, LOAD_None );
				if( Package )
				{
					INT SectionCount	= 0;
					INT PrimitiveCount	= 0;
					INT TriangleCount	= 0;

					// Iterate over all static mesh actors (we only care about ones in package).
					for( TObjectIterator<AStaticMeshActor> It; It; ++It )
					{
						// Figure out associate mesh if actor is in level package.
						AStaticMeshActor*	StaticMeshActor = *It;
						UStaticMesh*		StaticMesh		= NULL;
						if( StaticMeshActor 
						&&	StaticMeshActor->IsIn( Package ) 
						&&	StaticMeshActor->StaticMeshComponent )
						{
							StaticMesh = StaticMeshActor->StaticMeshComponent->StaticMesh;
						}
						// Gather stats for mesh.
						if( StaticMesh )
						{
							for( INT ElementIndex=0; ElementIndex<StaticMesh->LODModels(0).Elements.Num(); ElementIndex++ )
							{
								const FStaticMeshElement& StaticMeshElement = StaticMesh->LODModels(0).Elements(ElementIndex);
								TriangleCount += StaticMeshElement.NumTriangles;
								SectionCount++;
							}
							PrimitiveCount++;
						}
					}

					// Update gathered stats.
					new(GatheredData) FString( FString::Printf(TEXT("%s,%i,%i,%i"), *Filename.GetCleanFilename(), PrimitiveCount, SectionCount, TriangleCount) );
				}

				// Purge map again.
				UObject::CollectGarbage( RF_Native, TRUE );
			}
		}
	}

	// Log information in CSV format.
	for( INT LineIndex=0; LineIndex<GatheredData.Num(); LineIndex++ )
	{
		warnf(TEXT(",%s"),*GatheredData(LineIndex));
	}

	return 0;
}

IMPLEMENT_CLASS(UUT3MapStatsCommandlet);

INT UListLoopingEmittersCommandlet::Main(const FString& Params)
{
	// Load all game script files.
	TArray<FString> PackageNames;
	appGetGameScriptPackageNames( PackageNames, FALSE );
	extern void LoadPackageList(const TArray<FString>& PackageNames);
	LoadPackageList( PackageNames );

	// Find referenced looping emitters.
	for( TObjectIterator<UParticleSystem> It; It; ++It )
	{
		UParticleSystem* ParticleSystem = *It;

		UBOOL bIsInfinitelyLooping = FALSE;
		for( INT EmitterIndex=0; EmitterIndex<ParticleSystem->Emitters.Num(); EmitterIndex++ )
		{
			UParticleEmitter* ParticleEmitter = ParticleSystem->Emitters(EmitterIndex);
			if( ParticleEmitter )
			{
				for( INT LODIndex=0; LODIndex < ParticleEmitter->LODLevels.Num(); LODIndex++ )
				{
					UParticleLODLevel* ParticleLODLevel = ParticleEmitter->LODLevels(LODIndex);
					if( ParticleLODLevel && ParticleLODLevel->RequiredModule && ParticleLODLevel->RequiredModule->EmitterLoops == 0 )
					{
						bIsInfinitelyLooping = TRUE;
					}
				}					
			}
		}

		if( bIsInfinitelyLooping )
		{
			warnf(NAME_Warning,TEXT("%s"),*ParticleSystem->GetFullName());
		}
	}

	return 0;
}

IMPLEMENT_CLASS(UListLoopingEmittersCommandlet);

/** 
 * USoundGroupInfoCommandlet
 * 
 * Iterate over all sound cues and do a sanity check on the sound groups
 * Specifically -
 *  - List of Sound Groups assigned to all Sound Cues in Content\Sounds
 *  - List of Sound Cues not assigned to any Sound Group in Content\Sounds
 *  - List of Sound Cues assigned to Sound Group 'Ambient' that do not exist in packages AmbientLoops.upk and AmbientNonLoops
 *  - List of any Sound Wavs or Sound Cues that exist in packages other than Content\Sounds
 */
INT USoundGroupInfoCommandlet::Main( const FString& Params )
{
	INT Counter = 0;
	TArray<FName> UniqueGroups;
	TArray<INT> UniqueGroupCounts;
	TArray<FString> MisplacedSoundCues;
	TArray<FString> MisplacedSoundNodeWaves;
	TArray<FString> UngroupedSoundCues;
	TArray<FString> MisplacedAmbientSoundCues;

	// Build package file list.
	const TArray<FString> FilesInPath( GPackageFileCache->GetPackageFileList() );
	if( FilesInPath.Num() == 0 )
	{
		warnf( NAME_Warning, TEXT( "No packages found" ) );
		return( 1 );
	}

	// Iterate over all files doing stuff.
	for( INT FileIndex = 0; FileIndex < FilesInPath.Num(); ++FileIndex )
	{
		const FFilename& Filename = FilesInPath( FileIndex );
		if( Filename.InStr( TEXT( "Game\\Content\\" ) ) > 0 )
		{
			warnf( NAME_Log, TEXT( "Loading %s" ), *Filename );

			UPackage* Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
			if( Package == NULL )
			{
				warnf( NAME_Error, TEXT( "Error loading %s!" ), *Filename );
			}
			else
			{
				if( Filename.InStr( TEXT( "Game\\Content\\Sounds\\" ) ) > 0 )
				{
					for( TObjectIterator<USoundCue> It; It; ++It )
					{
						USoundCue* Cue = *It;
						if( Cue )
						{
							INT Index;

							if( UniqueGroups.FindItem(FName(TEXT("Ungrouped")), Index ) )
							{
								UniqueGroupCounts( Index )++;
							}
							else
							{
								UniqueGroups.AddItem( FName(TEXT("Ungrouped")) );
								UniqueGroupCounts.AddItem( 0 );
							}

							if( FName(TEXT("Ungrouped")) == FName( TEXT( "None" ) ) )
							{
								UngroupedSoundCues.AddUniqueItem( Cue->GetFullName() );
							}
						}
					}
				}
				else
				{
					for( TObjectIterator<USoundCue> It; It; ++It )
					{
						USoundCue* Cue = *It;
						if( Cue )
						{
							if( Cue->IsIn( Package ) )
							{
								FString FullName = Cue->GetFullName();
								if( FullName.InStr( TEXT( "AmbientSoundSimple_" ) ) < 0 
									&& FullName.InStr( TEXT( "AmbientSound_" ) ) < 0 
									&& FullName.InStr( TEXT( "AmbientSoundMovable_" ) ) < 0 
									&& FullName.InStr( TEXT( "AmbientSoundNonLoop_" ) ) < 0 
									&& FullName.InStr( TEXT( "AmbientSoundSimpleToggleable_" ) ) < 0 )
								{
									MisplacedSoundCues.AddUniqueItem( FullName );
									warnf( TEXT( "Misplaced sound cue: '%s'" ), *FullName );
								}
							}
						}
					}

					for( TObjectIterator<USoundNodeWave> It; It; ++It )
					{
						USoundNodeWave* Wave = *It;
						if( Wave )
						{
							if( Wave->IsIn( Package ) )
							{
								FString FullName = Wave->GetFullName();
								MisplacedSoundNodeWaves.AddUniqueItem( FullName );
								warnf( TEXT( "Misplaced sound node wave: '%s'" ), *FullName );
							}
						}
					}
				}
			}
		}

		if( ( Counter++ & 0xf ) == 0 )
		{
			UObject::CollectGarbage( RF_Native );
		}
	}

	// List all found sound groups
	warnf( NAME_Log, TEXT( "" ) );
	warnf( NAME_Log, TEXT( "%d sound groups found ..." ), UniqueGroups.Num() );
	for( INT i = 0; i < UniqueGroups.Num(); i++ )
	{
		warnf( NAME_Log, TEXT( " ... %4d : %s" ), UniqueGroupCounts( i ), *UniqueGroups( i ).GetNameString() );
	}

	// List all sound cues not under Content\\Sounds
	warnf( NAME_Log, TEXT( "" ) );
	warnf( NAME_Log, TEXT( "%d misplaced sound cues found ..." ), MisplacedSoundCues.Num() );
	for( TArray<FString>::TIterator It( MisplacedSoundCues ); It; ++It )
	{
		FString CueName = *It;
		warnf( NAME_Log, TEXT( " ... %s" ), *CueName );
	}

	// List all sound mode waves not under Content\\Sounds
	warnf( NAME_Log, TEXT( "" ) );
	warnf( NAME_Log, TEXT( "%d misplaced sound node waves found ..." ), MisplacedSoundNodeWaves.Num() );
	for( TArray<FString>::TIterator It( MisplacedSoundNodeWaves ); It; ++It )
	{
		FString SoundName = *It;
		warnf( NAME_Log, TEXT( " ... %s" ), *SoundName );
	}

	// List all sound cues in group ambient that aren't in the AmbientLoops or AmbientNonLoops packages
	warnf( NAME_Log, TEXT( "" ) );
	warnf( NAME_Log, TEXT( "%d misplaced ambient sound cues found (not in AmbientLoops or AmbientNonLoops) ..." ), MisplacedAmbientSoundCues.Num() );
	for( TArray<FString>::TIterator It( MisplacedAmbientSoundCues ); It; ++It )
	{
		FString CueName = *It;
		warnf( NAME_Log, TEXT( " ... %s" ), *CueName );
	}	

	// List all sound cues in group 'None' (ungrouped)
	warnf( NAME_Log, TEXT( "" ) );
	warnf( NAME_Log, TEXT( "%d ungrouped sound cues found ..." ), UngroupedSoundCues.Num() );
	for( TArray<FString>::TIterator It( UngroupedSoundCues ); It; ++It )
	{
		FString CueName = *It;
		warnf( NAME_Log, TEXT( " ... %s" ), *CueName );
	}

	return( 0 );
}

IMPLEMENT_CLASS( USoundGroupInfoCommandlet );

/** 
 * List all sound cues containing distance cross fade nodes as we can attenuate with a LPF to save memory
 */
INT UListDistanceCrossFadeNodesCommandlet::Main( const FString& Params )
{
	TArray<FString> SoundCues;

	// Build package file list.
	const TArray<FString> FilesInPath( GPackageFileCache->GetPackageFileList() );
	if( FilesInPath.Num() == 0 )
	{
		warnf( NAME_Warning, TEXT( "No packages found" ) );
		return( 1 );
	}

	// Iterate over all files doing stuff.
	for( INT FileIndex = 0; FileIndex < FilesInPath.Num(); ++FileIndex )
	{
		const FFilename& Filename = FilesInPath( FileIndex );
		if( Filename.InStr( TEXT( "Game\\Content\\Sounds" ) ) > 0 )
		{
			warnf( NAME_Log, TEXT( "Loading %s" ), *Filename );

			UPackage* Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
			if( Package == NULL )
			{
				warnf( NAME_Error, TEXT( "Error loading %s!" ), *Filename );
			}
			else
			{
				for( TObjectIterator<USoundNodeDistanceCrossFade> It; It; ++It )
				{
					USoundNodeDistanceCrossFade* Node = *It;
					if( Node )
					{
						FString CueName = Node->GetOuter()->GetFullName();
						SoundCues.AddUniqueItem( CueName );
					}
				}
			}

			UObject::CollectGarbage( RF_Native );
		}
	}

	// List all sound cues with a distance cross fade node
	warnf( NAME_Log, TEXT( "" ) );
	warnf( NAME_Log, TEXT( "%d sound cues found ..." ), SoundCues.Num() );
	for( TArray<FString>::TIterator It( SoundCues ); It; ++It )
	{
		FString CueName = *It;
		warnf( NAME_Log, TEXT( " ... %s" ), *CueName );
	}

	return( 0 );
}

IMPLEMENT_CLASS( UListDistanceCrossFadeNodesCommandlet );

/** 
 * ULocSoundInfoCommandlet
 * 
 * Sanity check the loc'd sound data
 */
INT ULocSoundInfoCommandlet::Main( const FString& Params )
{
	TArray<FFilename>	INTPackages;
	TArray<FString>		IntObjects;
	TArray<FString>		LocObjects;
	INT					Index;

	// Build package file list.
	const TArray<FString> FilesInPath( GPackageFileCache->GetPackageFileList() );
	if( FilesInPath.Num() == 0 )
	{
		warnf( NAME_Warning, TEXT( "No packages found" ) );
		return( 1 );
	}

	// Iterate over all files grabbing the objects in the INT packages
	for( INT FileIndex = 0; FileIndex < FilesInPath.Num(); ++FileIndex )
	{
		const FFilename& Filename = FilesInPath( FileIndex );
		if( Filename.StartsWith( TEXT( "..\\UTGame\\Content\\Sounds\\INT\\" ) ) )
		{
			INTPackages.AddItem( Filename );
		}
	}

	INT Count = 1;
	INT MissingLocalizedCount = 0;
	while( GKnownLanguageExtensions[Count] )
	{
		FString LangFolder = FString::Printf( TEXT( "\\%s\\" ), GKnownLanguageExtensions[Count] );
		FString Appendix = FString::Printf( TEXT( "_%s.upk" ), GKnownLanguageExtensions[Count] );

		INT MissingLocalizedCountPerLanguage = 0;
		for( INT PackageIndex = 0; PackageIndex < INTPackages.Num(); PackageIndex++ )
		{
			FFilename IntFilename = INTPackages( PackageIndex );
			FFilename LocFilename = IntFilename.Replace( TEXT( "\\INT\\" ), *LangFolder );
			LocFilename.ReplaceInline( TEXT( ".upk" ), *Appendix );
			if( !FilesInPath.FindItem( LocFilename, Index ) )
			{
				warnf( NAME_Log, TEXT( "Missing loc package: %s" ), *LocFilename );
			}
			else
			{
				// Load in both packages and compare them
				UPackage* IntPackage = UObject::LoadPackage( NULL, *IntFilename, LOAD_None );
				UPackage* LocPackage = UObject::LoadPackage( NULL, *LocFilename, LOAD_None );

				for( TObjectIterator<USoundNodeWave> It; It; ++It )
				{
					USoundNodeWave* Wave = *It;
					if( Wave->IsIn( IntPackage ) )
					{
						IntObjects.AddItem( Wave->GetName() );
					}
					if( Wave->IsIn( LocPackage ) )
					{
						LocObjects.AddItem( Wave->GetName() );
					}
				}

				// Remove common objects
				for( INT i = IntObjects.Num() - 1; i >= 0; i-- )
				{
					if( LocObjects.FindItem( IntObjects( i ), Index ) )
					{
						LocObjects.Remove( Index );
						IntObjects.Remove( i );
					}
				}

				warnf( NAME_Log, TEXT( "Objects only in package '%s' (%d) ... Missing in %s" ), *IntPackage->GetFullName(), IntObjects.Num(), GKnownLanguageExtensions[Count] );
				for( INT i = 0; i < IntObjects.Num(); i++ )
				{
					MissingLocalizedCountPerLanguage++;
					MissingLocalizedCount++;
					warnf( NAME_Log, TEXT( " ... %s" ), *IntObjects( i ) );
				}

				warnf( NAME_Log, TEXT( "Objects only in package '%s' (%d) ... Should be deleted?" ), *LocPackage->GetFullName(), LocObjects.Num() );
				for( INT i = 0; i < LocObjects.Num(); i++ )
				{
					warnf( NAME_Log, TEXT( " ...... %s" ), *LocObjects( i ) );
				}

				IntObjects.Empty();
				LocObjects.Empty();
			}
		}

		warnf( NAME_Log, TEXT( "Missing %d localized sounds for %s." ), MissingLocalizedCountPerLanguage, GKnownLanguageExtensions[Count] );
		UObject::CollectGarbage( RF_Native );
		Count++;
	}

	warnf( NAME_Log, TEXT( "Missing a total number of %d localized sounds." ), MissingLocalizedCount );
	return( 0 );
}

IMPLEMENT_CLASS( ULocSoundInfoCommandlet );

INT UBuildContentTagIndexCommandlet::Main( const FString& Params )
{
	FString IndexFilename = FString::Printf(TEXT("%sContent\\RefContentTagsIndex.upk"),*appGameDir());
	GFileManager->Delete(*IndexFilename);
	warnf(NAME_Log,TEXT("Building content tag index..."));
	UPackage *IndexPackage = CreatePackage(NULL,TEXT("RefContentTagsIndex"));
	UContentTagIndex *TagIndex = ConstructObject<UContentTagIndex>(UContentTagIndex::StaticClass(),IndexPackage,FName(TEXT("TagIndex")),RF_Public|RF_Standalone);
	TagIndex->AddToRoot();
	TagIndex->VersionInfo = GBuiltFromChangeList;
	TArray<FString> FilesToLoad( GPackageFileCache->GetPackageFileList() );
	/*
	TArray<FString> FilesToLoad;
	FilesToLoad.AddItem(FString(TEXT("Environments\\COG_EBA\\COG_EBA_InteriorDecos1")));
	FilesToLoad.AddItem(FString(TEXT("Effects\\War_Level_Effects")));
	*/
	INT Tmp = 0;
	for (TArray<FString>::TIterator It(FilesToLoad); It; ++It)
	{
		FString File = *It;
		// skip over shader packages
		if (File.InStr("ShaderCache",FALSE,TRUE) >= 0)
		{
			continue;
		}
		const FFilename Filename = *(*It);
		// skip over map packages
		if (Filename.GetExtension() == FURL::DefaultMapExt)
		{
			continue;
		}
		UPackage* Package = UObject::LoadPackage( NULL, *Filename, LOAD_NoWarn );
		if (Package != NULL)
		{
			warnf(NAME_Log,TEXT("...loaded package %s"),*Package->GetPathName());
			// static meshes first
			{
				// find or create the tag info
				FTagInfo& TagInfo = TagIndex->GetTagInfo(UStaticMesh::StaticClass());
				for (TObjectIterator<UStaticMesh> It; It; ++It)
				{
					UStaticMesh *Asset = *It;
					if (Asset->IsIn(Package))
					{
						TagIndex->AddContentReference(TagInfo,Asset->ContentTags,Asset->GetPathName());
					}
				}
			}
			// particles
			{
				// find or create the tag info
				FTagInfo& TagInfo = TagIndex->GetTagInfo(UParticleSystem::StaticClass());
				for (TObjectIterator<UParticleSystem> It; It; ++It)
				{
					UParticleSystem *Asset = *It;
					if (Asset->IsIn(Package))
					{
						TagIndex->AddContentReference(TagInfo,Asset->ContentTags,Asset->GetPathName());
					}
				}
			}
			// materials
			{
				// find or create the tag info
				FTagInfo& TagInfo = TagIndex->GetTagInfo(UMaterial::StaticClass());
				for (TObjectIterator<UMaterial> It; It; ++It)
				{
					UMaterial *Asset = *It;
					if (Asset->IsIn(Package))
					{
						TagIndex->AddContentReference(TagInfo,Asset->ContentTags,Asset->GetPathName());
					}
				}
			}
		}

		// only GC every 10 packages or so
		if (++Tmp > 10)
		{
			UObject::CollectGarbage(RF_Native);
			Tmp = 0;
		}
	}
	// save the results
	INT NumAssetTypes = TagIndex->Tags.Num();
	INT NumTotalTags = 0, NumTotalAssets = 0;
	for (INT Idx = 0; Idx < NumAssetTypes; Idx++)
	{
		NumTotalTags += TagIndex->Tags(Idx).Tags.Num();
		for (INT AssetIdx = 0; AssetIdx < TagIndex->Tags(Idx).Assets.Num(); AssetIdx++)
		{
			NumTotalAssets += TagIndex->Tags(Idx).Assets(AssetIdx).AssetReference.Num();
		}
	}
	if (ParseParam(appCmdLine(),TEXT("verbose")))
	{
		UContentTagIndex::AssetMapType AssetMap;
		TagIndex->BuildAssetTypeToAssetMap(AssetMap);
		for (UContentTagIndex::AssetMapType::TIterator Iter(AssetMap); Iter; ++Iter)
		{
			warnf(NAME_Log,TEXT("AssetType: %s"),*(Iter.Key())->GetPathName());
			for (TMap<FString,TArray<FName> >::TIterator AssetIter(Iter.Value()); AssetIter; ++AssetIter)
			{
				warnf(NAME_Log,TEXT("- Asset: %s"),*(AssetIter.Key()));
				for (TArray<FName>::TIterator TagIter(AssetIter.Value()); TagIter; ++TagIter)
				{
					warnf(NAME_Log,TEXT("  - Tag: %s"),*((*TagIter).GetNameString()));
				}
			}
		}
	}
	warnf(NAME_Log,TEXT("Saving content tag index... %d asset types, %d total tags, %d asset references"),NumAssetTypes,NumTotalTags,NumTotalAssets);
	return SavePackage(IndexPackage,TagIndex,0,*IndexFilename,GWarn) ? 0 : 1;		// have to flip the result as 0 == success for commandlets
}

IMPLEMENT_CLASS( UContentTagIndex );
IMPLEMENT_CLASS( UBuildContentTagIndexCommandlet );
