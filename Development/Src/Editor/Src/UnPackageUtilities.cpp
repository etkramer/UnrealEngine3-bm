/*=============================================================================
UnPackageUtilities.cpp: Commandlets for viewing information about package files
Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "EngineMaterialClasses.h"
#include "EngineSequenceClasses.h"
#include "UnPropertyTag.h"
#include "EngineUIPrivateClasses.h"
#include "EnginePhysicsClasses.h"
#include "EngineParticleClasses.h"
#include "LensFlare.h"
#include "EngineAnimClasses.h"
#include "UnTerrain.h"
#include "EngineFoliageClasses.h"
#include "SpeedTree.h"
#include "UnTerrain.h"
#include "EnginePrefabClasses.h"
#include "Database.h"
#include "EngineSoundClasses.h"

#include "../../UnrealEd/Inc/SourceControlIntegration.h"

#include "PackageHelperFunctions.h"
#include "PackageUtilityWorkers.h"

#include "PerfMem.h"
#include "AnimationEncodingFormat.h"
#include "AnimationUtils.h"

/*-----------------------------------------------------------------------------
ULoadPackageCommandlet
-----------------------------------------------------------------------------*/

/**
* If you pass in -ALL this will recursively load all of the packages from the
* directories listed in the .ini path entries
**/

INT ULoadPackageCommandlet::Main( const FString& Params )
{
	TArray<FString> Tokens, Switches;
	ParseCommandLine(*Params, Tokens, Switches);

	const UBOOL bLoadAllPackages = Switches.ContainsItem(TEXT("ALL"));

	TArray<FFilename> FilesInPath;
	if ( bLoadAllPackages )
	{
		TArray<FString> PackageExtensions;
		GConfig->GetArray( TEXT("Core.System"), TEXT("Extensions"), PackageExtensions, GEngineIni );

		Tokens.Empty(PackageExtensions.Num());
		for ( INT ExtensionIndex = 0; ExtensionIndex < PackageExtensions.Num(); ExtensionIndex++ )
		{
			Tokens.AddItem(FString(TEXT("*.")) + PackageExtensions(ExtensionIndex));
		}
	}

	if ( Tokens.Num() == 0 )
	{
		warnf(TEXT("You must specify a package name (multiple files can be delimited by spaces) or wild-card, or specify -all to include all registered packages"));
		return 1;
	}

	BYTE PackageFilter = NORMALIZE_DefaultFlags;
	if ( Switches.ContainsItem(TEXT("MAPSONLY")) )
	{
		PackageFilter |= NORMALIZE_ExcludeContentPackages;
	}

	// assume the first token is the map wildcard/pathname
	TArray<FString> Unused;
	for ( INT TokenIndex = 0; TokenIndex < Tokens.Num(); TokenIndex++ )
	{
		TArray<FFilename> TokenFiles;
		if ( !NormalizePackageNames( Unused, TokenFiles, Tokens(TokenIndex), PackageFilter) )
		{
			debugf(TEXT("No packages found for parameter %i: '%s'"), TokenIndex, *Tokens(TokenIndex));
			continue;
		}

		FilesInPath += TokenFiles;
	}

	if ( FilesInPath.Num() == 0 )
	{
		warnf(TEXT("No files found."));
		return 1;
	}

	GIsClient = !Switches.ContainsItem(TEXT("NOCLIENT"));
	GIsServer = !Switches.ContainsItem(TEXT("NOSERVER"));
	GIsEditor = !Switches.ContainsItem(TEXT("NOEDITOR"));

	for( INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++ )
	{
		const FFilename& Filename = FilesInPath(FileIndex);

		// we don't care about trying to load the various shader caches so just skipz0r them
		if(	Filename.InStr( TEXT("LocalShaderCache") ) != INDEX_NONE
		|| Filename.InStr( TEXT("RefShaderCache") ) != INDEX_NONE )
		{
			continue;
		}

		warnf( NAME_Log, TEXT("Loading %s"), *Filename );

		const FString& PackageName = FPackageFileCache::PackageFromPath(*Filename);
		UPackage* Package = FindObject<UPackage>(NULL, *PackageName, TRUE);
		if ( Package != NULL && !bLoadAllPackages )
		{
			ResetLoaders(Package);
		}

		Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
		if( Package == NULL )
		{
			warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
		}

		if ( GIsEditor )
		{
			SaveLocalShaderCaches();
		}

		UObject::CollectGarbage( RF_Native );
	}
	GIsEditor = GIsServer = GIsClient = TRUE;

	return 0;
}
IMPLEMENT_CLASS(ULoadPackageCommandlet)


/*-----------------------------------------------------------------------------
UShowObjectCountCommandlet.
-----------------------------------------------------------------------------*/

void UShowObjectCountCommandlet::StaticInitialize()
{
}


IMPLEMENT_COMPARE_CONSTREF( FPackageObjectCount, UnPackageUtilities, { INT result = appStricmp(*A.ClassName, *B.ClassName); if ( result == 0 ) { result = B.Count - A.Count; } return result; } )


FObjectCountExecutionParms::FObjectCountExecutionParms( const TArray<UClass*>& InClasses, EObjectFlags InMask/*=RF_LoadForClient|RF_LoadForServer|RF_LoadForEdit*/ )
: SearchClasses(InClasses), ObjectMask(InMask), bShowObjectNames(FALSE), bIncludeCookedPackages(FALSE)
, bCookedPackagesOnly(FALSE), bIgnoreChildren(FALSE), bIgnoreCheckedOutPackages(FALSE)
, bIgnoreScriptPackages(FALSE), bIgnoreMapPackages(FALSE), bIgnoreContentPackages(FALSE)
{
}


/**
 * Searches all packages for the objects which meet the criteria specified.
 *
 * @param	Parms	specifies the parameters to use for the search
 * @param	Results	receives the results of the search
 * @param	bUnsortedResults	by default, the list of results will be sorted according to the number of objects in each package;
 *								specify TRUE to override this behavior.
 */
void UShowObjectCountCommandlet::ProcessPackages( const FObjectCountExecutionParms& Parms, TArray<FPackageObjectCount>& Results, UBOOL bUnsortedResults/*=FALSE*/ )
{
	TArray<FString> PackageFiles;

	if ( !Parms.bCookedPackagesOnly )
	{
		PackageFiles = GPackageFileCache->GetPackageFileList();
	}

	if ( Parms.bCookedPackagesOnly || Parms.bIncludeCookedPackages )
	{
		const INT StartIndex = PackageFiles.Num();
		const FString CookedPackageDirectory = appGameDir() + TEXT("CookedXenon");
		const FString CookedPackageSearchString = CookedPackageDirectory * TEXT("*.xxx");
		GFileManager->FindFiles(PackageFiles, *CookedPackageSearchString, TRUE, FALSE);

		// re-add the path information so that GetPackageLinker finds the correct version of the file.
		for ( INT FileIndex = StartIndex; FileIndex < PackageFiles.Num(); FileIndex++ )
		{
			PackageFiles(FileIndex) = CookedPackageDirectory * PackageFiles(FileIndex);
		}
	}

	INT GCIndex = 0;
	for( INT FileIndex=0; FileIndex<PackageFiles.Num(); FileIndex++ )
	{
		const FString &Filename = PackageFiles(FileIndex);

		if ( Parms.bIgnoreCheckedOutPackages && !GFileManager->IsReadOnly(*Filename) )
		{
			warnf(NAME_Progress, TEXT("Skipping '%s'..."), *Filename);
			continue;
		}

		UObject::BeginLoad();
		ULinkerLoad* Linker = UObject::GetPackageLinker( NULL, *Filename, LOAD_Quiet|LOAD_NoWarn|LOAD_NoVerify, NULL, NULL );
		UObject::EndLoad();

		if(	Linker != NULL )
		{
			const UBOOL bScriptPackage	= Linker->LinkerRoot != NULL && (Linker->LinkerRoot->PackageFlags&PKG_ContainsScript) == 0;
			const UBOOL bMapPackage		= Linker->LinkerRoot != NULL && !Linker->LinkerRoot->ContainsMap();
			const UBOOL bContentPackage	= !bScriptPackage && !bMapPackage;

			if ((!Parms.bIgnoreScriptPackages	|| bScriptPackage)
			&&	(!Parms.bIgnoreMapPackages		|| bMapPackage)
			&&	(!Parms.bIgnoreContentPackages	|| bContentPackage) )
			{
				warnf(NAME_Progress, TEXT("Checking '%s'..."), *Filename);

				TArray<INT> ObjectCounts;
				ObjectCounts.AddZeroed(Parms.SearchClasses.Num());

				TArray< TArray<FString> > PackageObjectNames;
				if ( Parms.bShowObjectNames )
				{
					PackageObjectNames.AddZeroed(Parms.SearchClasses.Num());
				}

				UBOOL bContainsObjects=FALSE;
				for ( INT i = 0; i < Linker->ExportMap.Num(); i++ )
				{
					FObjectExport& Export = Linker->ExportMap(i);
					if ( (Export.ObjectFlags&Parms.ObjectMask) != 0 )
					{
						continue;
					}

					FString ClassPathName;


					FName ClassFName = NAME_Class;
					PACKAGE_INDEX ClassPackageIndex = 0;

					// get the path name for this Export's class
					if ( IS_IMPORT_INDEX(Export.ClassIndex) )
					{
						FObjectImport& ClassImport = Linker->ImportMap(-Export.ClassIndex -1);
						ClassFName = ClassImport.ObjectName;
						ClassPackageIndex = ClassImport.OuterIndex;
					}
					else if ( Export.ClassIndex != UCLASS_INDEX )
					{
						FObjectExport& ClassExport = Linker->ExportMap(Export.ClassIndex-1);
						ClassFName = ClassExport.ObjectName;
						ClassPackageIndex = ClassExport.OuterIndex;
					}

					FName OuterName = NAME_Core;
					if ( ClassPackageIndex > 0 )
					{
						FObjectExport& OuterExport = Linker->ExportMap(ClassPackageIndex-1);
						OuterName = OuterExport.ObjectName;
					}
					else if ( ClassPackageIndex < 0 )
					{
						FObjectImport& OuterImport = Linker->ImportMap(-ClassPackageIndex-1);
						OuterName = OuterImport.ObjectName;
					}
					else if ( Export.ClassIndex != UCLASS_INDEX )
					{
						OuterName = Linker->LinkerRoot->GetFName();
					}

					ClassPathName = FString::Printf(TEXT("%s.%s"), *OuterName.ToString(), *ClassFName.ToString());
					UClass* ExportClass = FindObject<UClass>(ANY_PACKAGE, *ClassPathName);
					if ( ExportClass == NULL )
					{
						ExportClass = StaticLoadClass(UObject::StaticClass(), NULL, *ClassPathName, NULL, LOAD_NoVerify|LOAD_NoWarn|LOAD_Quiet, NULL);
					}

					if ( ExportClass == NULL )
					{
						continue;
					}

					FString ObjectName;
					for ( INT ClassIndex = 0; ClassIndex < Parms.SearchClasses.Num(); ClassIndex++ )
					{
						UClass* SearchClass = Parms.SearchClasses(ClassIndex);
						if ( Parms.bIgnoreChildren ? ExportClass == SearchClass : ExportClass->IsChildOf(SearchClass) )
						{
							bContainsObjects = TRUE;
							INT& CurrentObjectCount = ObjectCounts(ClassIndex);
							CurrentObjectCount++;

							if ( Parms.bShowObjectNames )
							{
								TArray<FString>& ClassObjectPaths = PackageObjectNames(ClassIndex);

								if ( ObjectName.Len() == 0 )
								{
									ObjectName = Linker->GetExportFullName(i);
								}
								ClassObjectPaths.AddItem(ObjectName);
							}
						}
					}
				}

				if ( bContainsObjects )
				{
					for ( INT ClassIndex = 0; ClassIndex < ObjectCounts.Num(); ClassIndex++ )
					{
						INT ClassObjectCount = ObjectCounts(ClassIndex);
						if ( ClassObjectCount > 0 )
						{
							FPackageObjectCount* ObjCount = new(Results) FPackageObjectCount(Filename, Parms.SearchClasses(ClassIndex)->GetName(), ClassObjectCount);
							if ( Parms.bShowObjectNames )
							{
								ObjCount->ObjectPathNames = PackageObjectNames(ClassIndex);
							}
						}
					}
				}
			}
			else
			{
				warnf(NAME_Progress, TEXT("Skipping '%s'..."), *Filename);
			}
		}

		// only GC every 10 packages (A LOT faster this way, and is safe, since we are not 
		// acting on objects that would need to go away or anything)
		if ((++GCIndex % 10) == 0)
		{
			UObject::CollectGarbage(RF_Native);
		}
	}

	if ( !bUnsortedResults && Results.Num() > 0 )
	{
		Sort<USE_COMPARE_CONSTREF(FPackageObjectCount,UnPackageUtilities)>( &Results(0), Results.Num() );
	}
}

INT UShowObjectCountCommandlet::Main( const FString& Params )
{
	const TCHAR* Parms = *Params;

	GIsRequestingExit			= 1;	// so CTRL-C will exit immediately
	TArray<FString> Tokens, Switches;
	ParseCommandLine(Parms, Tokens, Switches);

	if ( Tokens.Num() == 0 )
	{
		warnf(TEXT("No class specified!"));
		return 1;
	}

	const UBOOL bIncludeChildren = !Switches.ContainsItem(TEXT("ExactClass"));
	const UBOOL bIncludeCookedPackages = Switches.ContainsItem(TEXT("IncludeCooked"));
	const UBOOL bCookedPackagesOnly = Switches.ContainsItem(TEXT("CookedOnly"));

	// this flag is useful for skipping over old test packages which can cause the commandlet to crash
	const UBOOL bIgnoreCheckedOutPackages = Switches.ContainsItem(TEXT("IgnoreWriteable"));

	const UBOOL bIgnoreScriptPackages = !Switches.ContainsItem(TEXT("IncludeScript"));
	// this flag is useful when you know that the objects you are looking for are not placeable in map packages
	const UBOOL bIgnoreMaps = Switches.ContainsItem(TEXT("IgnoreMaps"));
	const UBOOL bIgnoreContentPackages = Switches.ContainsItem(TEXT("IgnoreContent"));

	const UBOOL bShowObjectNames = Switches.ContainsItem(TEXT("ObjectNames"));
		
	EObjectFlags ObjectMask = 0;
	if ( Switches.ContainsItem(TEXT("SkipClient")) )
	{
		ObjectMask |= RF_LoadForClient;
	}
	if ( Switches.ContainsItem(TEXT("SkipServer")) )
	{
		ObjectMask |= RF_LoadForServer;
	}
	if ( Switches.ContainsItem(TEXT("SkipEditor")) )
	{
		ObjectMask |= RF_LoadForEdit;
	}

	TArray<UClass*> SearchClasses;
	for ( INT TokenIndex = 0; TokenIndex < Tokens.Num(); TokenIndex++ )
	{
		FString& SearchClassName = Tokens(TokenIndex);
		UClass* SearchClass = LoadClass<UObject>(NULL, *SearchClassName, NULL, 0, NULL);
		if ( SearchClass == NULL )
		{
			warnf(TEXT("Failed to load class specified '%s'"), *SearchClassName);
			return 1;
		}

		SearchClasses.AddUniqueItem(SearchClass);

		// make sure it doesn't get garbage collected.
		SearchClass->AddToRoot();
	}

	FObjectCountExecutionParms ProcessParameters( SearchClasses, ObjectMask );
	ProcessParameters.bShowObjectNames			= bShowObjectNames;
	ProcessParameters.bIncludeCookedPackages	= bIncludeCookedPackages;
	ProcessParameters.bCookedPackagesOnly		= bCookedPackagesOnly;
	ProcessParameters.bIgnoreChildren			= !bIncludeChildren;
	ProcessParameters.bIgnoreCheckedOutPackages = bIgnoreCheckedOutPackages;
	ProcessParameters.bIgnoreScriptPackages		= bIgnoreScriptPackages;
	ProcessParameters.bIgnoreMapPackages		= bIgnoreMaps;
	ProcessParameters.bIgnoreContentPackages	= bIgnoreContentPackages;
	TArray<FPackageObjectCount> ClassObjectCounts;

	ProcessPackages(ProcessParameters, ClassObjectCounts);
	if( ClassObjectCounts.Num() )
	{
		INT TotalObjectCount=0;
		INT PerClassObjectCount=0;

		FString LastReportedClass;
		INT IndexPadding=0;
		for ( INT i = 0; i < ClassObjectCounts.Num(); i++ )
		{
			FPackageObjectCount& PackageObjectCount = ClassObjectCounts(i);
			if ( PackageObjectCount.ClassName != LastReportedClass )
			{
				if ( LastReportedClass.Len() > 0 )
				{
					warnf(TEXT("    Total: %i"), PerClassObjectCount);
				}

				PerClassObjectCount = 0;
				LastReportedClass = PackageObjectCount.ClassName;
				warnf(TEXT("\r\nPackages containing objects of class '%s':"), *LastReportedClass);
				IndexPadding = appItoa(PackageObjectCount.Count).Len();
			}

			warnf(TEXT("%s    Count: %*i    Package: %s"), (i > 0 && bShowObjectNames ? LINE_TERMINATOR : TEXT("")), IndexPadding, PackageObjectCount.Count, *PackageObjectCount.PackageName);
			PerClassObjectCount += PackageObjectCount.Count;
			TotalObjectCount += PackageObjectCount.Count;

			if ( bShowObjectNames )
			{
				warnf(TEXT("        Details:"));
				for ( INT NameIndex = 0; NameIndex < PackageObjectCount.ObjectPathNames.Num(); NameIndex++ )
				{
					warnf(TEXT("        %*i) %s"), IndexPadding, NameIndex, *PackageObjectCount.ObjectPathNames(NameIndex));
				}
			}
		}

		warnf(TEXT("    Total: %i"), PerClassObjectCount);
		warnf(TEXT("\r\nTotal number of object instances: %i"), TotalObjectCount);
	}
	return 0;
}

IMPLEMENT_CLASS(UShowObjectCountCommandlet);

/*-----------------------------------------------------------------------------
UShowTaggedPropsCommandlet.
-----------------------------------------------------------------------------*/

INT UShowTaggedPropsCommandlet::Main(const FString& Params)
{
	const TCHAR* CmdLine = appCmdLine();

	TArray<FString> Tokens, Switches;
	ParseCommandLine(CmdLine, Tokens, Switches);

	FString	ClassName, PackageParm, PropertyFilter, IgnorePropertyFilter;

	for ( INT Idx = 0; Idx < Tokens.Num(); Idx++ )
	{
		FString& Token = Tokens(Idx);
		if ( Parse(*Token, TEXT("IGNOREPROPS="), IgnorePropertyFilter) )
		{
			Tokens.Remove(Idx);
			break;
		}
	}

	if ( Tokens.Num() >  0 )
	{
		PackageParm = Tokens(0);
		if ( Tokens.Num() > 1 )
		{
			ClassName = Tokens(1);
			if ( Tokens.Num() > 2 )
			{
				PropertyFilter = Tokens(2);
			}
		}
	}
	else
	{
		PackageParm = TEXT("*.upk");
	}

	DWORD FilterFlags = 0;
	if ( Switches.ContainsItem(TEXT("COOKED")) )
	{
		FilterFlags |= NORMALIZE_IncludeCookedPackages;
	}

	TArray<FString> PackageNames;
	TArray<FFilename> PackageFilenames;
	if ( !NormalizePackageNames(PackageNames, PackageFilenames, PackageParm, FilterFlags) )
	{
		return 0;
	}

	for ( INT FileIndex = 0; FileIndex < PackageFilenames.Num(); FileIndex++ )
	{
		// reset the loaders for the packages we want to load so that we don't find the wrong version of the file
		// (otherwise, attempting to run pkginfo on e.g. Engine.xxx will always return results for Engine.u instead)
		const FString& PackageName = FPackageFileCache::PackageFromPath(*PackageFilenames(FileIndex));
		UPackage* ExistingPackage = FindObject<UPackage>(NULL, *PackageName, TRUE);
		if ( ExistingPackage != NULL )
		{
			ResetLoaders(ExistingPackage);
		}

		warnf(TEXT("Loading '%s'..."), *PackageName);
		UObject* Pkg = LoadPackage(NULL, *PackageName, LOAD_None);
		UClass* SearchClass = StaticLoadClass(UObject::StaticClass(), NULL, *ClassName, NULL, LOAD_None, NULL);
		if ( SearchClass == NULL && ClassName.Len() > 0 )
		{
			warnf(NAME_Error, TEXT("Failed to load class '%s'"), *ClassName);
			return 1;
		}

		if ( PropertyFilter.Len() > 0 )
		{
			TArray<FString> PropertyNames;
			PropertyFilter.ParseIntoArray(&PropertyNames, TEXT(","), TRUE);

			for ( INT PropertyIndex = 0; PropertyIndex < PropertyNames.Num(); PropertyIndex++ )
			{
				UProperty* Property = FindFieldWithFlag<UProperty,CASTCLASS_UProperty>(SearchClass, FName(*PropertyNames(PropertyIndex)));
				if ( Property != NULL )
				{
					SearchProperties.AddItem(Property);
				}
			}
		}

		if ( IgnorePropertyFilter.Len() > 0 )
		{
			TArray<FString> PropertyNames;
			IgnorePropertyFilter.ParseIntoArray(&PropertyNames, TEXT(","), TRUE);

			for ( INT PropertyIndex = 0; PropertyIndex < PropertyNames.Num(); PropertyIndex++ )
			{
				UProperty* Property = FindFieldWithFlag<UProperty,CASTCLASS_UProperty>(SearchClass, FName(*PropertyNames(PropertyIndex)));
				if ( Property != NULL )
				{
					IgnoreProperties.AddItem(Property);
				}
			}
		}

		// this is needed in case we end up serializing a script reference which results in VerifyImport being called
		BeginLoad();
		for ( FObjectIterator It; It; ++It )
		{
			UObject* Obj = *It;
			if ( Obj->IsA(SearchClass) && Obj->IsIn(Pkg) )
			{
				ShowSavedProperties(Obj);
			}
		}
		EndLoad();
	}

	return 0;
}

void UShowTaggedPropsCommandlet::ShowSavedProperties( UObject* Object ) const
{
	check(Object);

	warnf(TEXT("Showing property data for %s"), *Object->GetFullName());
	ULinkerLoad& Ar = *Object->GetLinker();
	INT LinkerIndex = Object->GetLinkerIndex();
	checkf(LinkerIndex != INDEX_NONE,TEXT("Invalid linker index for '%s'"), *Object->GetFullName());

	const UBOOL bIsArchetypeObject = Object->IsTemplate();
	if ( bIsArchetypeObject == TRUE )
	{
		Ar.StartSerializingDefaults();
	}

	FName PropertyName(NAME_None);
	FObjectExport& Export = Ar.ExportMap(LinkerIndex);
	Ar.Loader->Seek(Export.SerialOffset);
	Ar.Loader->Precache(Export.SerialOffset,Export.SerialSize);

	if( Object->HasAnyFlags(RF_HasStack) )
	{
		FStateFrame* DummyStateFrame = new FStateFrame(Object);

		Ar << DummyStateFrame->Node << DummyStateFrame->StateNode;
		Ar << DummyStateFrame->ProbeMask;
		Ar << DummyStateFrame->LatentAction;
		Ar << DummyStateFrame->StateStack;
		if( DummyStateFrame->Node )
		{
			Ar.Preload( DummyStateFrame->Node );
			INT Offset = DummyStateFrame->Code ? DummyStateFrame->Code - &DummyStateFrame->Node->Script(0) : INDEX_NONE;
			Ar << Offset;
			if( Offset!=INDEX_NONE )
			{
				if( Offset<0 || Offset>=DummyStateFrame->Node->Script.Num() )
				{
					appErrorf( TEXT("%s: Offset mismatch: %i %i"), *GetFullName(), Offset, DummyStateFrame->Node->Script.Num() );
				}
			}
			DummyStateFrame->Code = Offset!=INDEX_NONE ? &DummyStateFrame->Node->Script(Offset) : NULL;
		}
		else 
		{
			DummyStateFrame->Code = NULL;
		}

		delete DummyStateFrame;
	}

	if ( Object->IsA(UComponent::StaticClass()) && !Object->HasAnyFlags(RF_ClassDefaultObject) )
	{
		((UComponent*)Object)->PreSerialize(Ar);
	}

	Object->SerializeNetIndex(Ar);

	INT BufferSize = 256 * 256;
	BYTE* Data = (BYTE*)appMalloc(BufferSize);

	// we need to keep a pointer to the original location of the memory we allocated in case one of the functions we call moves the Data pointer
	BYTE* StartingData = Data;

	// Load tagged properties.
	UClass* ObjClass = Object->GetClass();

	// This code assumes that properties are loaded in the same order they are saved in. This removes a n^2 search 
	// and makes it an O(n) when properties are saved in the same order as they are loaded (default case). In the 
	// case that a property was reordered the code falls back to a slower search.
	UProperty*	Property			= ObjClass->PropertyLink;
	UBOOL		AdvanceProperty		= 0;
	INT			RemainingArrayDim	= Property ? Property->ArrayDim : 0;

	UBOOL bDisplayedObjectName = FALSE;

	// Load all stored properties, potentially skipping unknown ones.
	while( 1 )
	{
		FPropertyTag Tag;
		Ar << Tag;
		if( Tag.Name == NAME_None )
			break;
		PropertyName = Tag.Name;

		// Move to the next property to be serialized
		if( AdvanceProperty && --RemainingArrayDim <= 0 )
		{
			check(Property);
			Property = Property->PropertyLinkNext;
			// Skip over properties that don't need to be serialized.
			while( Property && !Property->ShouldSerializeValue( Ar ) )
			{
				Property = Property->PropertyLinkNext;
			}
			AdvanceProperty		= 0;
			RemainingArrayDim	= Property ? Property->ArrayDim : 0;
		}

		// If this property is not the one we expect (e.g. skipped as it matches the default value), do the brute force search.
		if( Property == NULL || Property->GetFName() != Tag.Name )
		{
			UProperty* CurrentProperty = Property;
			// Search forward...
			for ( ; Property; Property=Property->PropertyLinkNext )
			{
				if( Property->GetFName() == Tag.Name )
				{
					break;
				}
			}
			// ... and then search from the beginning till we reach the current property if it's not found.
			if( Property == NULL )
			{
				for( Property = ObjClass->PropertyLink; Property && Property != CurrentProperty; Property = Property->PropertyLinkNext )
				{
					if( Property->GetFName() == Tag.Name )
					{
						break;
					}
				}

				if( Property == CurrentProperty )
				{
					// Property wasn't found.
					Property = NULL;
				}
			}

			RemainingArrayDim = Property ? Property->ArrayDim : 0;
		}

		const UBOOL bSkipPropertyValue = Property != NULL && IgnoreProperties.HasKey(Property);
		const UBOOL bShowPropertyValue = !bSkipPropertyValue
			&&	(SearchProperties.Num() == 0
			||	(Property != NULL && SearchProperties.HasKey(Property)));

		if ( bShowPropertyValue && !bDisplayedObjectName )
		{
			bDisplayedObjectName = TRUE;
			warnf(TEXT("%s:"), *Object->GetFullName());
		}

		if ( Tag.Size >= BufferSize )
		{
			BufferSize = Tag.Size + 1;
			StartingData = (BYTE*)appRealloc(Data, BufferSize);
		}

		// zero out the data so that we don't accidentally call a copy ctor with garbage data in the dest address
		Data = StartingData;
		appMemzero(Data, BufferSize);

		if( !Property )
		{
			//@{
			//@compatibility
			if ( Tag.Name == NAME_InitChild2StartBone )
			{
				UProperty* NewProperty = FindField<UProperty>(ObjClass, TEXT("BranchStartBoneName"));
				if (NewProperty != NULL && NewProperty->IsA(UArrayProperty::StaticClass()) && ((UArrayProperty*)NewProperty)->Inner->IsA(UNameProperty::StaticClass()))
				{
					INT OldNameIndex, OldNameInstance;
					Ar << OldNameIndex << OldNameInstance;
					AdvanceProperty = FALSE;
					continue;
				}
			}
			//@}
			debugf( NAME_Warning, TEXT("Property %s of %s not found for package:  %s"), *Tag.Name.ToString(), *ObjClass->GetFullName(), *Ar.GetArchiveName() );
		}
		else if( Tag.ArrayIndex>=Property->ArrayDim || Tag.ArrayIndex < 0 )
		{
			debugf( NAME_Warning, TEXT("Array bounds in %s of %s: %i/%i for package:  %s"), *Tag.Name.ToString(), *GetName(), Tag.ArrayIndex, Property->ArrayDim, *Ar.GetArchiveName() );
		}
		else if( Tag.Type==NAME_StrProperty && Property->GetID()==NAME_NameProperty )  
		{
			FString str;  
			Ar << str;
			AdvanceProperty = TRUE;

			if ( bShowPropertyValue )
			{
				FString PropertyNameText = Property->GetName();
				if ( Property->ArrayDim != 1 )
				{
					PropertyNameText += FString::Printf(TEXT("[%i]"), Tag.ArrayIndex);
				}

				warnf(TEXT("\t%s%s"), *PropertyNameText.RightPad(32), *str);
			}
			continue; 
		}
		else if ( Tag.Type == NAME_ByteProperty && Property->GetID() == NAME_IntProperty )
		{
			// this property's data was saved as a BYTE, but the property has been changed to an INT.  Since there is no loss of data
			// possible, we can auto-convert to the right type.
			BYTE PreviousValue;

			// de-serialize the previous value
			Ar << PreviousValue;

			FString PropertyNameText = *Property->GetName();
			if ( Property->ArrayDim != 1 )
			{
				PropertyNameText += FString::Printf(TEXT("[%i]"), Tag.ArrayIndex);
			}
			warnf(TEXT("\t%s%i"), *PropertyNameText.RightPad(32), PreviousValue);
			AdvanceProperty = TRUE;
			continue;
		}
		else if( Tag.Type!=Property->GetID() )
		{
			debugf( NAME_Warning, TEXT("Type mismatch in %s of %s - Previous (%s) Current(%s) for package:  %s"), *Tag.Name.ToString(), *ObjClass->GetName(), *Tag.Type.ToString(), *Property->GetID().ToString(), *Ar.GetArchiveName() );
		}
		else if( Tag.Type==NAME_StructProperty && Tag.StructName!=CastChecked<UStructProperty>(Property)->Struct->GetFName() )
		{
			debugf( NAME_Warning, TEXT("Property %s of %s struct type mismatch %s/%s for package:  %s"), *Tag.Name.ToString(), *ObjClass->GetName(), *Tag.StructName.ToString(), *CastChecked<UStructProperty>(Property)->Struct->GetName(), *Ar.GetArchiveName() );
		}
		else if( !Property->ShouldSerializeValue(Ar) )
		{
			if ( bShowPropertyValue )
			{
				debugf( NAME_Warning, TEXT("Property %s of %s is not serializable for package:  %s"), *Tag.Name.ToString(), *ObjClass->GetName(), *Ar.GetArchiveName() );
			}
		}
		else if ( bShowPropertyValue )
		{
			// This property is ok.
			Tag.SerializeTaggedProperty( Ar, Property, Data, 0, NULL );

			FString PropertyValue;

			// if this is an array property, export each element individually so that it's easier to read
			UArrayProperty* ArrayProp = Cast<UArrayProperty>(Property);
			if ( ArrayProp != NULL )
			{
				FScriptArray* Array = (FScriptArray*)Data;
				if ( Array != NULL )
				{
					UProperty* InnerProp = ArrayProp->Inner;
					const INT ElementSize = InnerProp->ElementSize;
					for( INT i=0; i<Array->Num(); i++ )
					{
						const FString PropertyNameText = FString::Printf(TEXT("%s(%i)"), *Property->GetName(), i);
						PropertyValue.Empty();

						BYTE* PropData = (BYTE*)Array->GetData() + i * ElementSize;
						InnerProp->ExportTextItem( PropertyValue, PropData, NULL, NULL, PPF_Localized );
						warnf(TEXT("\t%s%s"), *PropertyNameText.RightPad(32), *PropertyValue);
					}
				}
			}
			else
			{
				Property->ExportTextItem(PropertyValue, Data, NULL, NULL, PPF_Localized);

				FString PropertyNameText = *Property->GetName();
				if ( Property->ArrayDim != 1 )
				{
					PropertyNameText += FString::Printf(TEXT("[%i]"), Tag.ArrayIndex);
				}
				warnf(TEXT("\t%s%s"), *PropertyNameText.RightPad(32), *PropertyValue);
			}

			if ( (Property->PropertyFlags&CPF_NeedCtorLink) != 0 )
			{
				// clean up the memory
				Property->DestroyValue( StartingData );
			}

			AdvanceProperty = TRUE;
			continue;
		}

		// Skip unknown or bad property.
		if ( bShowPropertyValue )
		{
			debugf( NAME_Warning, TEXT("Skipping %i bytes of type %s for package:  %s"), Tag.Size, *Tag.Type.ToString(), *Ar.GetArchiveName() );
			AdvanceProperty = FALSE;
		}
		else 
		{
			// if we're not supposed to show the value for this property, just skip it without logging a warning
			AdvanceProperty = TRUE;
		}

		Ar.Loader->Seek(Ar.Loader->Tell() + Tag.Size);
	}

	appFree(StartingData);

	// now the native properties
	TLookupMap<FString> SearchPropertyNames;
	for ( INT PropIdx = 0; PropIdx < SearchProperties.Num(); PropIdx++ )
	{
		SearchPropertyNames.AddItem(*SearchProperties(PropIdx)->GetName());
	}
	TMap<FString,FString> NativePropertyValues;
	if ( Object->GetNativePropertyValues(NativePropertyValues, 0) )
	{
		for ( TMap<FString,FString>::TIterator It(NativePropertyValues); It; ++It )
		{
			const FString& PropertyName = It.Key();
			const FString& PropertyValue = It.Value();

			const UBOOL bShowPropertyValue = SearchPropertyNames.Num() == 0
				|| (PropertyName.Len() > 0 && SearchPropertyNames.HasKey(PropertyName));

			if ( bShowPropertyValue && !bDisplayedObjectName )
			{
				bDisplayedObjectName = TRUE;
				warnf(TEXT("%s:"), *Object->GetFullName());
			}
			if ( bShowPropertyValue )
			{
				warnf(TEXT("\t%s%s"), *PropertyName.RightPad(32), *PropertyValue);
			}
		}
	}

	if ( bDisplayedObjectName )
		warnf(TEXT(""));

	if ( bIsArchetypeObject == TRUE )
	{
		Ar.StopSerializingDefaults();
	}
}

IMPLEMENT_CLASS(UShowTaggedPropsCommandlet)

/*-----------------------------------------------------------------------------
UListPackagesReferencing commandlet.
-----------------------------------------------------------------------------*/

/**
* Contains the linker name and filename for a package which is referencing another package.
*/
struct FReferencingPackageName
{
	/** the name of the linker root (package name) */
	FName LinkerFName;

	/** the complete filename for the package */
	FString Filename;

	/** Constructor */
	FReferencingPackageName( FName InLinkerFName, const FString& InFilename )
		: LinkerFName(InLinkerFName), Filename(InFilename)
	{
	}

	/** Comparison operator */
	inline UBOOL operator==( const FReferencingPackageName& Other ) const
	{
		return LinkerFName == Other.LinkerFName;
	}
};

inline DWORD GetTypeHash( const FReferencingPackageName& ReferencingPackageStruct )
{
	return GetTypeHash(ReferencingPackageStruct.LinkerFName);
}

IMPLEMENT_COMPARE_CONSTREF(FReferencingPackageName,UnPackageUtilities,{ return appStricmp(*A.LinkerFName.ToString(),*B.LinkerFName.ToString()); });

INT UListPackagesReferencingCommandlet::Main( const FString& Params )
{
	const TCHAR* Parms = *Params;

	TSet<FReferencingPackageName>	ReferencingPackages;
	TArray<FString> PackageFiles = GPackageFileCache->GetPackageFileList();


	//@todo ronp - add support for searching for references to multiple packages/resources at once.

	FString SearchName;
	if( ParseToken(Parms, SearchName, 0) )
	{

		// determine whether we're searching references to a package or a specific resource
		INT delimPos = SearchName.InStr(TEXT("."), TRUE);

		// if there's no dots in the search name, or the last part of the name is one of the registered package extensions, we're searching for a package
		const UBOOL bIsPackage = delimPos == INDEX_NONE || GSys->Extensions.FindItemIndex(SearchName.Mid(delimPos+1)) != INDEX_NONE;

		FName SearchPackageFName=NAME_None;
		if ( bIsPackage == TRUE )
		{
			// remove any extensions on the package name
			SearchPackageFName = FName(*FFilename(SearchName).GetBaseFilename());
		}
		else
		{
			// validate that this resource exists
			UObject* SearchObject = StaticLoadObject(UObject::StaticClass(), NULL, *SearchName, NULL, LOAD_NoWarn, NULL);
			if ( SearchObject == NULL )
			{
				warnf(TEXT("Unable to load specified resource: %s"), *SearchName);
				return 1;
			}

			// searching for a particular resource - pull off the package name
			SearchPackageFName = SearchObject->GetOutermost()->GetFName();

			// then change the SearchName to the object's actual path name, in case the name passed on the command-line wasn't a complete path name
			SearchName = SearchObject->GetPathName();

			// make sure it doesn't get GC'd
			SearchObject->AddToRoot();
		}

		INT GCIndex = 0;
		for( INT FileIndex=0; FileIndex<PackageFiles.Num(); FileIndex++ )
		{
			const FString &Filename = PackageFiles(FileIndex);

			warnf(NAME_Progress, TEXT("Loading '%s'..."), *Filename);

			UObject::BeginLoad();
			ULinkerLoad* Linker = UObject::GetPackageLinker( NULL, *Filename, LOAD_Quiet|LOAD_NoWarn, NULL, NULL );
			UObject::EndLoad();

			if( Linker )
			{
				FName LinkerFName = Linker->LinkerRoot->GetFName();

				// ignore the package if it's the one we're processing
				if( LinkerFName != SearchPackageFName )
				{
					// look for the search package in this package's ImportMap.
					for( INT ImportIndex=0; ImportIndex<Linker->ImportMap.Num(); ImportIndex++ )
					{
						FObjectImport& Import = Linker->ImportMap( ImportIndex );
						UBOOL bImportReferencesSearchPackage = FALSE;

						if ( bIsPackage == TRUE )
						{
							if ( Import.ClassPackage == SearchPackageFName )
							{
								// this import's class is contained in the package we're searching for references to
								bImportReferencesSearchPackage = TRUE;
							}
							else if ( Import.ObjectName == SearchPackageFName && Import.ClassName == NAME_Package && Import.ClassPackage == NAME_Core )
							{
								// this import is the package we're searching for references to
								bImportReferencesSearchPackage = TRUE;
							}
							else if ( Import.OuterIndex != ROOTPACKAGE_INDEX )
							{
								// otherwise, determine if this import's source package is the package we're searching for references to
								// Import.SourceLinker is cleared in UObject::EndLoad, so we can't use that
								PACKAGE_INDEX OutermostLinkerIndex = Import.OuterIndex;
								for ( PACKAGE_INDEX LinkerIndex = Import.OuterIndex; LinkerIndex != ROOTPACKAGE_INDEX; )
								{
									OutermostLinkerIndex = LinkerIndex;

									// this import's outer might be in the export table if the package was saved for seek-free loading
									if ( IS_IMPORT_INDEX(LinkerIndex) )
									{
										LinkerIndex = Linker->ImportMap( -LinkerIndex - 1 ).OuterIndex;
									}
									else
									{
										LinkerIndex = Linker->ExportMap( LinkerIndex - 1 ).OuterIndex;
									}
								}

								// if the OutermostLinkerIndex is ROOTPACKAGE_INDEX, this import corresponds to the root package for this linker
								if ( IS_IMPORT_INDEX(OutermostLinkerIndex) )
								{
									FObjectImport& PackageImport = Linker->ImportMap( -OutermostLinkerIndex - 1 );
									bImportReferencesSearchPackage =	PackageImport.ObjectName	== SearchPackageFName &&
										PackageImport.ClassName		== NAME_Package &&
										PackageImport.ClassPackage	== NAME_Core;
								}
								else
								{
									check(OutermostLinkerIndex != ROOTPACKAGE_INDEX);

									FObjectExport& PackageExport = Linker->ExportMap( OutermostLinkerIndex - 1 );
									bImportReferencesSearchPackage =	PackageExport.ObjectName == SearchPackageFName;
								}
							}
						}
						else
						{
							FString ImportPathName = Linker->GetImportPathName(ImportIndex);
							if ( SearchName == ImportPathName )
							{
								// this is the object we're search for
								bImportReferencesSearchPackage = TRUE;
							}
							else
							{
								// see if this import's class is the resource we're searching for
								FString ImportClassPathName = Import.ClassPackage.ToString() + TEXT(".") + Import.ClassName.ToString();
								if ( ImportClassPathName == SearchName )
								{
									bImportReferencesSearchPackage = TRUE;
								}
								else if ( Import.OuterIndex > ROOTPACKAGE_INDEX )
								{
									// and OuterIndex > 0 indicates that the import's Outer is in the package's export map, which would happen
									// if the package was saved for seek-free loading;
									// we need to check the Outer in this case since we are only iterating through the ImportMap
									FString OuterPathName = Linker->GetExportPathName(Import.OuterIndex - 1);
									if ( SearchName == OuterPathName )
									{
										bImportReferencesSearchPackage = TRUE;
									}
								}
							}
						}

						if ( bImportReferencesSearchPackage )
						{
							ReferencingPackages.Add( FReferencingPackageName(LinkerFName, Filename) );
							break;
						}
					}
				}
			}

			// only GC every 10 packages (A LOT faster this way, and is safe, since we are not 
			// acting on objects that would need to go away or anything)
			if ((++GCIndex % 10) == 0)
			{
				UObject::CollectGarbage(RF_Native);
			}
		}

		warnf( TEXT("%i packages reference %s:"), ReferencingPackages.Num(), *SearchName );

		// calculate the amount of padding to use when listing the referencing packages
		INT Padding=appStrlen(TEXT("Package Name"));
		for(TSet<FReferencingPackageName>::TConstIterator It(ReferencingPackages);It;++It)
		{
			Padding = Max(Padding, It->LinkerFName.ToString().Len());
		}

		warnf( TEXT("  %*s  Filename"), Padding, TEXT("Package Name"));

		// KeySort shouldn't be used with TLookupMap because then the Value for each pair in the Pairs array (which is the index into the Pairs array for that pair)
		// is no longer correct.  That doesn't matter to use because we don't use the value for anything, so sort away!
		ReferencingPackages.Sort<COMPARE_CONSTREF_CLASS(FReferencingPackageName,UnPackageUtilities)>();

		// output the list of referencers
		for(TSet<FReferencingPackageName>::TConstIterator It(ReferencingPackages);It;++It)
		{
			warnf( TEXT("  %*s  %s"), Padding, *It->LinkerFName.ToString(), *It->Filename );
		}
	}

	return 0;
}
IMPLEMENT_CLASS(UListPackagesReferencingCommandlet)

/*-----------------------------------------------------------------------------
	UPkgInfo commandlet.
-----------------------------------------------------------------------------*/

struct FExportInfo
{
	FObjectExport Export;
	INT ExportIndex;
	FString PathName;
	FString OuterPathName;
	FString ArchetypePathName;

	FExportInfo( ULinkerLoad* Linker, INT InIndex )
	: Export(Linker->ExportMap(InIndex)), ExportIndex(InIndex)
	, OuterPathName(TEXT("NULL")), ArchetypePathName(TEXT("NULL"))
	{
		PathName = Linker->GetExportPathName(ExportIndex);
		SetOuterPathName(Linker);
		SetArchetypePathName(Linker);
	}

	void SetOuterPathName( ULinkerLoad* Linker )
	{
		if ( Export.OuterIndex > 0 )
		{
			OuterPathName = Linker->GetExportPathName(Export.OuterIndex - 1);
		}
		else if ( IS_IMPORT_INDEX(Export.OuterIndex) )
		{
			OuterPathName = Linker->GetImportPathName(-Export.OuterIndex-1);
		}
	}

	void SetArchetypePathName( ULinkerLoad* Linker )
	{
		if ( Export.ArchetypeIndex > 0 )
		{
			ArchetypePathName = Linker->GetExportPathName(Export.ArchetypeIndex-1);
		}
		else if ( IS_IMPORT_INDEX(Export.ArchetypeIndex) )
		{
			ArchetypePathName = Linker->GetImportPathName(-Export.ArchetypeIndex-1);
		}
	}
};

namespace
{
	enum EExportSortType
	{
		EXPORTSORT_ExportSize,
		EXPORTSORT_ExportIndex,
		EXPORTSORT_ObjectPathname,
		EXPORTSORT_OuterPathname,
		EXPORTSORT_ArchetypePathname,
		EXPORTSORT_MAX
	};

	class FObjectExport_Sorter
	{
	public:
		static EExportSortType SortPriority[EXPORTSORT_MAX];

		// Comparison method
		static inline INT Compare( const FExportInfo& A, const FExportInfo& B )
		{
			INT Result = 0;

			for ( INT PriorityType = 0; PriorityType < EXPORTSORT_MAX; PriorityType++ )
			{
				switch ( SortPriority[PriorityType] )
				{
					case EXPORTSORT_ExportSize:
						Result = B.Export.SerialSize - A.Export.SerialSize;
						break;

					case EXPORTSORT_ExportIndex:
						Result = A.ExportIndex - B.ExportIndex;
						break;

					case EXPORTSORT_ObjectPathname:
						Result = A.PathName.Len() - B.PathName.Len();
						if ( Result == 0 )
						{
							Result = appStricmp(*A.PathName, *B.PathName);
						}
						break;

					case EXPORTSORT_OuterPathname:
						Result = A.OuterPathName.Len() - B.OuterPathName.Len();
						if ( Result == 0 )
						{
							Result = appStricmp(*A.OuterPathName, *B.OuterPathName);
						}
						break;

					case EXPORTSORT_ArchetypePathname:
						Result = A.ArchetypePathName.Len() - B.ArchetypePathName.Len();
						if ( Result == 0 )
						{
							Result = appStricmp(*A.ArchetypePathName, *B.ArchetypePathName);
						}
						break;

					case EXPORTSORT_MAX:
						return Result;
				}

				if ( Result != 0 )
				{
					break;
				}
			}
			return Result;
		}
	};

	EExportSortType FObjectExport_Sorter::SortPriority[EXPORTSORT_MAX] =
	{ EXPORTSORT_ExportIndex, EXPORTSORT_ExportSize, EXPORTSORT_ArchetypePathname, EXPORTSORT_OuterPathname, EXPORTSORT_ObjectPathname };
}

/**
 * Writes information about the linker to the log.
 *
 * @param	InLinker	if specified, changes this reporter's Linker before generating the report.
 */
void FPkgInfoReporter_Log::GeneratePackageReport( ULinkerLoad* InLinker/*=NULL*/ )
{
	check(InLinker);

	if ( InLinker != NULL )
	{
		SetLinker(InLinker);
	}

	if ( PackageCount++ > 0 )
	{
		warnf(TEXT(""));
	}

	// Display information about the package.
	FName LinkerName = Linker->LinkerRoot->GetFName();

	// Display summary info.
	GWarn->Log( TEXT("********************************************") );
	GWarn->Logf( TEXT("Package '%s' Summary"), *LinkerName.ToString() );
	GWarn->Log( TEXT("--------------------------------------------") );

	GWarn->Logf( TEXT("\t         Filename: %s"), *Linker->Filename);
	GWarn->Logf( TEXT("\t     File Version: %i"), Linker->Ver() );
	GWarn->Logf( TEXT("\t   Engine Version: %d"), Linker->Summary.EngineVersion);
	GWarn->Logf( TEXT("\t   Cooker Version: %d"), Linker->Summary.CookedContentVersion);
	GWarn->Logf( TEXT("\t     PackageFlags: %X"), Linker->Summary.PackageFlags );
	GWarn->Logf( TEXT("\t        NameCount: %d"), Linker->Summary.NameCount );
	GWarn->Logf( TEXT("\t       NameOffset: %d"), Linker->Summary.NameOffset );
	GWarn->Logf( TEXT("\t      ImportCount: %d"), Linker->Summary.ImportCount );
	GWarn->Logf( TEXT("\t     ImportOffset: %d"), Linker->Summary.ImportOffset );
	GWarn->Logf( TEXT("\t      ExportCount: %d"), Linker->Summary.ExportCount );
	GWarn->Logf( TEXT("\t     ExportOffset: %d"), Linker->Summary.ExportOffset );
	GWarn->Logf( TEXT("\tCompression Flags: %X"), Linker->Summary.CompressionFlags);

	FString szGUID = Linker->Summary.Guid.String();
	GWarn->Logf( TEXT("\t             Guid: %s"), *szGUID );
	GWarn->Log ( TEXT("\t      Generations:"));
	for( INT i = 0; i < Linker->Summary.Generations.Num(); ++i )
	{
		const FGenerationInfo& generationInfo = Linker->Summary.Generations( i );
		GWarn->Logf(TEXT("\t\t\t%d) ExportCount=%d, NameCount=%d, NetObjectCount=%d"), i, generationInfo.ExportCount, generationInfo.NameCount, generationInfo.NetObjectCount);
	}


	if( (InfoFlags&PKGINFO_Chunks) != 0 )
	{
		GWarn->Log( TEXT("--------------------------------------------") );
		GWarn->Log ( TEXT("Compression Chunks"));
		GWarn->Log ( TEXT("=========="));

		for ( INT ChunkIndex = 0; ChunkIndex < Linker->Summary.CompressedChunks.Num(); ChunkIndex++ )
		{
			FCompressedChunk& Chunk = Linker->Summary.CompressedChunks(ChunkIndex);
			GWarn->Log ( TEXT("\t*************************"));
			GWarn->Logf( TEXT("\tChunk %d:"), ChunkIndex );
			GWarn->Logf( TEXT("\t\tUncompressedOffset: %d"), Chunk.UncompressedOffset);
			GWarn->Logf( TEXT("\t\t  UncompressedSize: %d"), Chunk.UncompressedSize);
			GWarn->Logf( TEXT("\t\t  CompressedOffset: %d"), Chunk.CompressedOffset);
			GWarn->Logf( TEXT("\t\t    CompressedSize: %d"), Chunk.CompressedSize);
		}
	}

	if( (InfoFlags&PKGINFO_Names) != 0 )
	{
		GWarn->Log( TEXT("--------------------------------------------") );
		GWarn->Log ( TEXT("Name Map"));
		GWarn->Log ( TEXT("========"));
		for( INT i = 0; i < Linker->NameMap.Num(); ++i )
		{
			FName& name = Linker->NameMap( i );
			GWarn->Logf( TEXT("\t%d: Name '%s' Index %d [Internal: %s, %d]"), i, *name.ToString(), name.GetIndex(), *name.GetNameString(), name.GetNumber() );
		}
	}

	// if we _only_ want name info, skip this part completely
	if ( InfoFlags != PKGINFO_Names )
	{
		if( (InfoFlags&PKGINFO_Imports) != 0 )
		{
			GWarn->Log( TEXT("--------------------------------------------") );
			GWarn->Log ( TEXT("Import Map"));
			GWarn->Log ( TEXT("=========="));
		}

		TArray<FName> DependentPackages;
		for( INT i = 0; i < Linker->ImportMap.Num(); ++i )
		{
			FObjectImport& import = Linker->ImportMap( i );

			FName PackageName = NAME_None;
			FName OuterName = NAME_None;
			if ( import.OuterIndex != ROOTPACKAGE_INDEX )
			{
				if ( IS_IMPORT_INDEX(import.OuterIndex) )
				{
					if ( (InfoFlags&PKGINFO_Paths) != 0 )
					{
						OuterName = *Linker->GetImportPathName(-import.OuterIndex - 1);
					}
					else
					{
						FObjectImport& OuterImport = Linker->ImportMap(-import.OuterIndex-1);
						OuterName = OuterImport.ObjectName;
					}
				}
				else if ( import.OuterIndex > 0 )
				{
					if ( (InfoFlags&PKGINFO_Paths) != 0 )
					{
						OuterName = *Linker->GetExportPathName(import.OuterIndex-1);
					}
					else
					{
						FObjectExport& OuterExport = Linker->ExportMap(import.OuterIndex-1);
						OuterName = OuterExport.ObjectName;
					}
				}

				// Find the package which contains this import.  import.SourceLinker is cleared in UObject::EndLoad, so we'll need to do this manually now.
				PACKAGE_INDEX OutermostLinkerIndex = import.OuterIndex;
				for ( PACKAGE_INDEX LinkerIndex = import.OuterIndex; LinkerIndex != ROOTPACKAGE_INDEX; )
				{
					OutermostLinkerIndex = LinkerIndex;

					// this import's outer might be in the export table if the package was saved for seek-free loading
					if ( IS_IMPORT_INDEX(LinkerIndex) )
					{
						LinkerIndex = Linker->ImportMap( -LinkerIndex - 1 ).OuterIndex;
					}
					else
					{
						LinkerIndex = Linker->ExportMap( LinkerIndex - 1 ).OuterIndex;
					}
				}

				// if the OutermostLinkerIndex is ROOTPACKAGE_INDEX, this import corresponds to the root package for this linker
				if ( IS_IMPORT_INDEX(OutermostLinkerIndex) )
				{
					FObjectImport& PackageImport = Linker->ImportMap( -OutermostLinkerIndex - 1 );
					PackageName = PackageImport.ObjectName;
				}
				else
				{
					check(OutermostLinkerIndex != ROOTPACKAGE_INDEX);
					FObjectExport& PackageExport = Linker->ExportMap( OutermostLinkerIndex - 1 );
					PackageName = PackageExport.ObjectName;
				}
			}

			if ( (InfoFlags&PKGINFO_Imports) != 0 )
			{
				GWarn->Log ( TEXT("\t*************************"));
				GWarn->Logf( TEXT("\tImport %d: '%s'"), i, *import.ObjectName.ToString() );
				GWarn->Logf( TEXT("\t\t       Outer: '%s' (%d)"), *OuterName.ToString(), import.OuterIndex);
				GWarn->Logf( TEXT("\t\t     Package: '%s'"), *PackageName.ToString());
				GWarn->Logf( TEXT("\t\t       Class: '%s'"), *import.ClassName.ToString() );
				GWarn->Logf( TEXT("\t\tClassPackage: '%s'"), *import.ClassPackage.ToString() );
				GWarn->Logf( TEXT("\t\t     XObject: %s"), import.XObject ? TEXT("VALID") : TEXT("NULL"));
				GWarn->Logf( TEXT("\t\t SourceIndex: %d"), import.SourceIndex );

				// dump depends info
				if (InfoFlags & PKGINFO_Depends)
				{
					GWarn->Log(TEXT("\t\t  All Depends:"));
					if (Linker->Summary.PackageFlags & PKG_Cooked)
					{
						GWarn->Logf(TEXT("\t\t\t  Skipping (Cooked package)"));
					}
					else
					{
						TSet<FDependencyRef> AllDepends;
						Linker->GatherImportDependencies(i, AllDepends);
						INT DependsIndex = 0;
						for(TSet<FDependencyRef>::TConstIterator It(AllDepends);It;++It)
						{
							const FDependencyRef& Ref = *It;
							GWarn->Logf(TEXT("\t\t\t%i) %s"), DependsIndex++, *Ref.Linker->GetExportFullName(Ref.ExportIndex));
						}
					}
				}
			}

			if ( PackageName == NAME_None && import.ClassPackage == NAME_Core && import.ClassName == NAME_Package )
			{
				PackageName = import.ObjectName;
			}

			if ( PackageName != NAME_None && PackageName != LinkerName )
			{
				DependentPackages.AddUniqueItem(PackageName);
			}

			if ( import.ClassPackage != NAME_None && import.ClassPackage != LinkerName )
			{
				DependentPackages.AddUniqueItem(import.ClassPackage);
			}
		}

		if ( DependentPackages.Num() )
		{
			GWarn->Log( TEXT("--------------------------------------------") );
			warnf(TEXT("\tPackages referenced by %s:"), *LinkerName.ToString());
			for ( INT i = 0; i < DependentPackages.Num(); i++ )
			{
				warnf(TEXT("\t\t%i) %s"), i, *DependentPackages(i).ToString());
			}
		}
	}

	if( (InfoFlags&PKGINFO_Exports) != 0 )
	{
		GWarn->Log( TEXT("--------------------------------------------") );
		GWarn->Log ( TEXT("Export Map"));
		GWarn->Log ( TEXT("=========="));

		if ( (InfoFlags&PKGINFO_Compact) == 0 )
		{
			TArray<FExportInfo> SortedExportMap;
			SortedExportMap.Empty(Linker->ExportMap.Num());
			for( INT i = 0; i < Linker->ExportMap.Num(); ++i )
			{
				new(SortedExportMap) FExportInfo(Linker, i);
			}

			FString SortingParms;
			if ( Parse(appCmdLine(), TEXT("SORT="), SortingParms) )
			{
				TArray<FString> SortValues;
				SortingParms.ParseIntoArray(&SortValues, TEXT(","), TRUE);

				for ( INT i = 0; i < EXPORTSORT_MAX; i++ )
				{
					if ( i < SortValues.Num() )
					{
						const FString Value = SortValues(i);
						if ( Value == TEXT("index") )
						{
							FObjectExport_Sorter::SortPriority[i] = EXPORTSORT_ExportIndex;
						}
						else if ( Value == TEXT("size") )
						{
							FObjectExport_Sorter::SortPriority[i] = EXPORTSORT_ExportSize;
						}
						else if ( Value == TEXT("name") )
						{
							FObjectExport_Sorter::SortPriority[i] = EXPORTSORT_ObjectPathname;
						}
						else if ( Value == TEXT("outer") )
						{
							FObjectExport_Sorter::SortPriority[i] = EXPORTSORT_OuterPathname;
						}
						else if ( Value == TEXT("archetype") )
						{
							FObjectExport_Sorter::SortPriority[i] = EXPORTSORT_ArchetypePathname;
						}
					}
					else
					{
						FObjectExport_Sorter::SortPriority[i] = EXPORTSORT_MAX;
					}
				}
			}

			Sort<FExportInfo, FObjectExport_Sorter>( &SortedExportMap(0), SortedExportMap.Num() );

			for( INT SortedIndex = 0; SortedIndex < SortedExportMap.Num(); ++SortedIndex )
			{
				GWarn->Log ( TEXT("\t*************************"));
				FExportInfo& ExportInfo = SortedExportMap(SortedIndex);

				FObjectExport& Export = ExportInfo.Export;

				UBOOL bIsForcedExportPackage=FALSE;

				// determine if this export is a forced export in a cooked package
				FString ForcedExportString;
				if ( (Linker->Summary.PackageFlags&PKG_Cooked) != 0 && Export.HasAnyFlags(EF_ForcedExport) )
				{
					// find the package object this forced export was originally contained within
					INT PackageExportIndex = ROOTPACKAGE_INDEX;
					for ( INT OuterIndex = Export.OuterIndex; OuterIndex != ROOTPACKAGE_INDEX; OuterIndex = Linker->ExportMap(OuterIndex-1).OuterIndex )
					{
						PackageExportIndex = OuterIndex - 1;
					}

					if ( PackageExportIndex == ROOTPACKAGE_INDEX )
					{
						// this export corresponds to a top-level UPackage
						bIsForcedExportPackage = TRUE;
						ForcedExportString = TEXT(" [** FORCED **]");
					}
					else
					{
						// this export was a forced export that is not a top-level UPackage
						FObjectExport& OuterExport = Linker->ExportMap(PackageExportIndex);
						checkf(OuterExport.HasAnyFlags(EF_ForcedExport), TEXT("Export %i (%s) is a forced export but its outermost export %i (%s) is not!"),
							ExportInfo.ExportIndex, *ExportInfo.PathName, PackageExportIndex, *Linker->GetExportPathName(PackageExportIndex));

						ForcedExportString = FString::Printf(TEXT(" [** FORCED: '%s' (%i)]"), *OuterExport.ObjectName.ToString(), PackageExportIndex);
					}
				}
				GWarn->Logf( TEXT("\tExport %d: '%s'%s"), ExportInfo.ExportIndex, *Export.ObjectName.ToString(), *ForcedExportString );

				// find the name of this object's class
				INT ClassIndex = Export.ClassIndex;
				FName ClassName = ClassIndex > 0 
					? Linker->ExportMap(ClassIndex-1).ObjectName
					: IS_IMPORT_INDEX(ClassIndex)
						? Linker->ImportMap(-ClassIndex-1).ObjectName
						: FName(NAME_Class);

				// find the name of this object's parent...for UClasses, this will be the parent class
				// for UFunctions, this will be the SuperFunction, if it exists, etc.
				FString ParentName;
				if ( Export.SuperIndex > 0 )
				{
					if ( (InfoFlags&PKGINFO_Paths) != 0 )
					{
						ParentName = *Linker->GetExportPathName(Export.SuperIndex-1);
					}
					else
					{
						FObjectExport& ParentExport = Linker->ExportMap(Export.SuperIndex-1);
						ParentName = ParentExport.ObjectName.ToString();
					}
				}
				else if ( IS_IMPORT_INDEX(Export.SuperIndex) )
				{
					if ( (InfoFlags&PKGINFO_Paths) != 0 )
					{
						ParentName = *Linker->GetImportPathName(-Export.SuperIndex-1);
					}
					else
					{
						FObjectImport& ParentImport = Linker->ImportMap(-Export.SuperIndex-1);
						ParentName = ParentImport.ObjectName.ToString();
					}
				}

				// find the name of this object's Outer.  For UClasses, this will generally be the
				// top-level package itself.  For properties, a UClass, etc.
				FString OuterName;
				if ( Export.OuterIndex > 0 )
				{
					if ( (InfoFlags&PKGINFO_Paths) != 0 )
					{
						OuterName = *Linker->GetExportPathName(Export.OuterIndex - 1);
					}
					else
					{
						FObjectExport& OuterExport = Linker->ExportMap(Export.OuterIndex-1);
						OuterName = OuterExport.ObjectName.ToString();
					}
				}
				else if ( IS_IMPORT_INDEX(Export.OuterIndex) )
				{
					if ( (InfoFlags&PKGINFO_Paths) != 0 )
					{
						OuterName = *Linker->GetImportPathName(-Export.OuterIndex-1);
					}
					else
					{
						FObjectImport& OuterImport = Linker->ImportMap(-Export.OuterIndex-1);
						OuterName = OuterImport.ObjectName.ToString();
					}
				}

				FString TemplateName;
				if ( Export.ArchetypeIndex > 0 )
				{
					if ( (InfoFlags&PKGINFO_Paths) != 0 )
					{
						TemplateName = *Linker->GetExportPathName(Export.ArchetypeIndex-1);
					}
					else
					{
						FObjectExport& TemplateExport = Linker->ExportMap(Export.ArchetypeIndex-1);
						TemplateName = TemplateExport.ObjectName.ToString();
					}
				}
				else if ( IS_IMPORT_INDEX(Export.ArchetypeIndex) )
				{
					if ( (InfoFlags&PKGINFO_Paths) != 0 )
					{
						TemplateName = *Linker->GetImportPathName(-Export.ArchetypeIndex-1);
					}
					else
					{
						FObjectImport& TemplateImport = Linker->ImportMap(-Export.ArchetypeIndex-1);
						TemplateName = TemplateImport.ObjectName.ToString();
					}
				}

				GWarn->Logf( TEXT("\t\t         Class: '%s' (%i)"), *ClassName.ToString(), ClassIndex );
				GWarn->Logf( TEXT("\t\t        Parent: '%s' (%d)"), *ParentName, Export.SuperIndex );
				GWarn->Logf( TEXT("\t\t         Outer: '%s' (%d)"), *OuterName, Export.OuterIndex );
				GWarn->Logf( TEXT("\t\t     Archetype: '%s' (%d)"), *TemplateName, Export.ArchetypeIndex);
				GWarn->Logf( TEXT("\t\t      Pkg Guid: %s"), *Export.PackageGuid.String());
				GWarn->Logf( TEXT("\t\t   ObjectFlags: 0x%016I64X"), Export.ObjectFlags );
				GWarn->Logf( TEXT("\t\t          Size: %d"), Export.SerialSize );
				if ( !bHideOffsets )
				{
					GWarn->Logf( TEXT("\t\t      Offset: %d"), Export.SerialOffset );
				}
				GWarn->Logf( TEXT("\t\t       _Object: %s"), Export._Object ? TEXT("VALID") : TEXT("NULL"));
				if ( !bHideOffsets )
				{
					GWarn->Logf( TEXT("\t\t    _iHashNext: %d"), Export._iHashNext );
				}
				GWarn->Logf( TEXT("\t\t   ExportFlags: %X"), Export.ExportFlags );

				if ( bIsForcedExportPackage && Export.GenerationNetObjectCount.Num() > 0 )
				{
					warnf(TEXT("\t\tNetObjectCounts: %d generations"), Export.GenerationNetObjectCount.Num());
					for ( INT GenerationIndex = 0; GenerationIndex < Export.GenerationNetObjectCount.Num(); GenerationIndex++ )
					{
						warnf(TEXT("\t\t\t%d) %d"), GenerationIndex, Export.GenerationNetObjectCount(GenerationIndex));
					}
				}

				// dump depends info
				if (InfoFlags & PKGINFO_Depends)
				{
					if (ExportInfo.ExportIndex < Linker->DependsMap.Num())
					{
						TArray<INT>& Depends = Linker->DependsMap(ExportInfo.ExportIndex);
						GWarn->Log(TEXT("\t\t  DependsMap:"));
						if (Linker->Summary.PackageFlags & PKG_Cooked)
						{
							GWarn->Logf(TEXT("\t\t\t  Skipping (Cooked package)"));
						}
						else
						{
							for (INT DependsIndex = 0; DependsIndex < Depends.Num(); DependsIndex++)
							{
								GWarn->Logf(TEXT("\t\t\t%i) %s (%i)"), DependsIndex, 
									IS_IMPORT_INDEX(Depends(DependsIndex)) ? 
									*Linker->GetImportFullName(-Depends(DependsIndex) - 1) :
								*Linker->GetExportFullName(Depends(DependsIndex) - 1),
									Depends(DependsIndex));
							}

							TSet<FDependencyRef> AllDepends;
							Linker->GatherExportDependencies(ExportInfo.ExportIndex, AllDepends);
							GWarn->Log(TEXT("\t\t  All Depends:"));
							INT DependsIndex = 0;
							for(TSet<FDependencyRef>::TConstIterator It(AllDepends);It;++It)
							{
								const FDependencyRef& Ref = *It;
								GWarn->Logf(TEXT("\t\t\t%i) %s"), DependsIndex++, *Ref.Linker->GetExportFullName(Ref.ExportIndex));
							}
						}
					}
				}
			}
		}
		else
		{
			for( INT ExportIndex=0; ExportIndex<Linker->ExportMap.Num(); ExportIndex++ )
			{
				const FObjectExport& Export = Linker->ExportMap(ExportIndex);
				warnf(TEXT("  %8i %10i %s"), ExportIndex, Export.SerialSize, (InfoFlags&PKGINFO_Paths) != 0 ? *Linker->GetExportPathName(ExportIndex) : *Export.ObjectName.ToString());
			}
		}
	}
}

INT UPkgInfoCommandlet::Main( const FString& Params )
{
	const TCHAR* Parms = *Params;

	TArray<FString> Tokens, Switches;
	ParseCommandLine(Parms, Tokens, Switches);

	// find out which type of info we're looking for
	DWORD InfoFlags = PKGINFO_None;
	if ( Switches.ContainsItem(TEXT("names")) )
	{
		InfoFlags |= PKGINFO_Names;
	}
	if ( Switches.ContainsItem(TEXT("imports")) )
	{
		InfoFlags |= PKGINFO_Imports;
	}
	if ( Switches.ContainsItem(TEXT("exports")) )
	{
		InfoFlags |= PKGINFO_Exports;
	}
	if ( Switches.ContainsItem(TEXT("simple")) )
	{
		InfoFlags |= PKGINFO_Compact;
	}
	if ( Switches.ContainsItem(TEXT("chunks")) )
	{
		InfoFlags |= PKGINFO_Chunks;
	}
	if ( Switches.ContainsItem(TEXT("depends")) )
	{
		InfoFlags |= PKGINFO_Depends;
	}
	if ( Switches.ContainsItem(TEXT("paths")) )
	{
		InfoFlags |= PKGINFO_Paths;
	}
	if ( Switches.ContainsItem(TEXT("all")) )
	{
		InfoFlags |= PKGINFO_All;
	}

	const UBOOL bHideOffsets = Switches.ContainsItem(TEXT("HideOffsets"));

	/** What platform are we cooking for?	*/
	UE3::EPlatformType Platform = UE3::PLATFORM_Windows;
	FString PlatformStr;
	if (Parse(*Params, TEXT("PLATFORM="), PlatformStr))
	{
		if (PlatformStr == TEXT("PS3"))
		{
			Platform = UE3::PLATFORM_PS3;
		}
		else if (PlatformStr == TEXT("xenon") || PlatformStr == TEXT("xbox360"))
		{	
			Platform = UE3::PLATFORM_Xenon;
		}
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

	// determine how the output should be formatted
	FPkgInfoReporter* Reporter = NULL;
	if ( Switches.ContainsItem(TEXT("csv")) )
	{
		FFilename OutputFilename = appGameLogDir() * TEXT("PackageInfoReport.csv");
		for ( INT TokenIndex = Tokens.Num() - 1; TokenIndex >= 0; TokenIndex-- )
		{
			if ( Parse(*Tokens(TokenIndex), TEXT("OutputFilename="), (FString&)OutputFilename) )
			{
				Tokens.Remove(TokenIndex);
				break;
			}
		}

		if ( OutputFilename.GetExtension() != TEXT("csv") )
		{
			OutputFilename = OutputFilename.GetBaseFilename(FALSE) + TEXT(".csv");
		}

		//@todo ronp - create a reporter that will output in csv format
		Reporter = new FPkgInfoReporter_Log(InfoFlags, bHideOffsets);
// 		Reporter = new FPkgInfoReporter_CSV(InfoFlags, bHideOffsets, OutputFilename);
	}
	else
	{
		Reporter = new FPkgInfoReporter_Log(InfoFlags, bHideOffsets);
	}

	for ( INT TokenIndex = 0; TokenIndex < Tokens.Num(); TokenIndex++ )
	{
		FString& PackageWildcard = Tokens(TokenIndex);

		TArray<FString> FilesInPath;
		GFileManager->FindFiles( FilesInPath, *PackageWildcard, TRUE, FALSE );
		if( FilesInPath.Num() == 0 )
		{
			// if no files were found, it might be an unqualified path; try prepending the .u output path
			// if one were going to make it so that you could use unqualified paths for package types other
			// than ".u", here is where you would do it
			GFileManager->FindFiles( FilesInPath, *(appScriptOutputDir() * PackageWildcard), 1, 0 );

			if ( FilesInPath.Num() == 0 )
			{
				TArray<FString> Paths;
				if ( GConfig->GetArray( TEXT("Core.System"), TEXT("Paths"), Paths, GEngineIni ) > 0 )
				{
					for ( INT i = 0; i < Paths.Num(); i++ )
					{
						GFileManager->FindFiles( FilesInPath, *(Paths(i) * PackageWildcard), 1, 0 );
					}
				}
			}
			else
			{
				// re-add the path information so that GetPackageLinker finds the correct version of the file.
				FFilename WildcardPath = appScriptOutputDir() * PackageWildcard;
				for ( INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++ )
				{
					FilesInPath(FileIndex) = WildcardPath.GetPath() * FilesInPath(FileIndex);
				}
			}

			// Try finding package in package file cache.
			if ( FilesInPath.Num() == 0 )
			{
				FString Filename;
				if( GPackageFileCache->FindPackageFile( *PackageWildcard, NULL, Filename ) )
				{
					new(FilesInPath)FString(Filename);
				}
			}
		}
		else
		{
			// re-add the path information so that GetPackageLinker finds the correct version of the file.
			FFilename WildcardPath = PackageWildcard;
			for ( INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++ )
			{
				FilesInPath(FileIndex) = WildcardPath.GetPath() * FilesInPath(FileIndex);
			}
		}

		if ( FilesInPath.Num() == 0 )
		{
			warnf(TEXT("No packages found using '%s'!"), *PackageWildcard);
			continue;
		}

		for( INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++ )
		{
			const FString &Filename = FilesInPath(FileIndex);

			{
				// reset the loaders for the packages we want to load so that we don't find the wrong version of the file
				// (otherwise, attempting to run pkginfo on e.g. Engine.xxx will always return results for Engine.u instead)
				const FString& PackageName = FPackageFileCache::PackageFromPath(*Filename);
				UPackage* ExistingPackage = FindObject<UPackage>(NULL, *PackageName, TRUE);
				if ( ExistingPackage != NULL )
				{
					ResetLoaders(ExistingPackage);
				}
			}

			UObject::BeginLoad();
			ULinkerLoad* Linker = UObject::GetPackageLinker( NULL, *Filename, LOAD_NoVerify, NULL, NULL );
			UObject::EndLoad();

			if( Linker )
			{
				Reporter->GeneratePackageReport(Linker);
			}

			UObject::CollectGarbage(RF_Native);
		}
	}

	delete Reporter;
	Reporter = NULL;
	return 0;
}
IMPLEMENT_CLASS(UPkgInfoCommandlet)


/*-----------------------------------------------------------------------------
	UListCorruptedComponentsCommandlet
-----------------------------------------------------------------------------*/

/**
 * This commandlet is designed to find (and in the future, possibly fix) content that is affected by the components bug described in
 * TTPRO #15535 and UComponentProperty::InstanceComponents()
 */
INT UListCorruptedComponentsCommandlet::Main(const FString& Params)
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

	const UBOOL bCheckVersion = Switches.ContainsItem(TEXT("CHECKVER"));

	// Iterate over all files doing stuff.
	for( INT FileIndex = 0 ; FileIndex < FilesInPath.Num() ; ++FileIndex )
	{
		const FFilename& Filename = FilesInPath(FileIndex);
		warnf( NAME_Log, TEXT("Loading %s"), *Filename );

		UObject* Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
		if( Package == NULL )
		{
			warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
		}
		else if( bCheckVersion && Package->GetLinkerVersion() != GPackageFileVersion )
		{
			warnf( NAME_Log, TEXT("Version mismatch. Package [%s] should be resaved."), *Filename );
		}

		UBOOL bInsertNewLine = FALSE;
		for ( TObjectIterator<UComponent> It; It; ++It )
		{
			if ( It->IsIn(Package) && !It->IsTemplate(RF_ClassDefaultObject) )
			{
				UComponent* Component = *It;
				UComponent* ComponentTemplate = Cast<UComponent>(Component->GetArchetype());
				UObject* Owner = Component->GetOuter();
				UObject* TemplateOwner = ComponentTemplate->GetOuter();
				if ( !ComponentTemplate->HasAnyFlags(RF_ClassDefaultObject) )
				{
					if ( TemplateOwner != Owner->GetArchetype() )
					{
						bInsertNewLine = TRUE;

						FString RealArchetypeName;
						if ( Component->TemplateName != NAME_None )
						{
							UComponent* RealArchetype = Owner->GetArchetype()->FindComponent(Component->TemplateName);
							if ( RealArchetype != NULL )
							{
								RealArchetypeName = RealArchetype->GetFullName();
							}
							else
							{
								RealArchetypeName = FString::Printf(TEXT("NULL: no matching components found in Owner Archetype %s"), *Owner->GetArchetype()->GetFullName());
							}
						}
						else
						{
							RealArchetypeName = TEXT("NULL");
						}

						warnf(TEXT("\tPossible corrupted component: '%s'	Archetype: '%s'	TemplateName: '%s'	ResolvedArchetype: '%s'"),
							*Component->GetFullName(), 
							*ComponentTemplate->GetPathName(),
							*Component->TemplateName.ToString(),
							*RealArchetypeName);
					}

					if ( Component->GetClass()->HasAnyClassFlags(CLASS_UniqueComponent) )
					{
						bInsertNewLine = TRUE;
						warnf(TEXT("\tComponent is using unique component class: %s"), *Component->GetFullName());
					}

					if ( ComponentTemplate->GetClass()->HasAnyClassFlags(CLASS_UniqueComponent) )
					{
						bInsertNewLine = TRUE;
						warnf(TEXT("\tComponent archetype has unique component class: '%s'	Archetype: '%s'"), *Component->GetFullName(), *ComponentTemplate->GetPathName());
					}
				}
			}
		}

		if ( bInsertNewLine )
		{
			warnf(TEXT(""));
		}
		UObject::CollectGarbage( RF_Native );
	}

	return 0;
}

IMPLEMENT_CLASS(UListCorruptedComponentsCommandlet);

/*-----------------------------------------------------------------------------
	UAnalyzeCookedPackages commandlet.
-----------------------------------------------------------------------------*/

INT UAnalyzeCookedPackagesCommandlet::Main( const FString& Params )
{
	// Parse command line.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	const TCHAR* Parms = *Params;
	ParseCommandLine(Parms, Tokens, Switches);

	// Tokens on the command line are package wildcards.
	for( INT TokenIndex=0; TokenIndex<Tokens.Num(); TokenIndex++ )
	{
		// Find all files matching current wildcard.
		FFilename		Wildcard = Tokens(TokenIndex);
		TArray<FString> Filenames;
		GFileManager->FindFiles( Filenames, *Wildcard, TRUE, FALSE );

		// Iterate over all found files.
		for( INT FileIndex = 0; FileIndex < Filenames.Num(); FileIndex++ )
		{
			const FString& Filename = Wildcard.GetPath() + PATH_SEPARATOR + Filenames(FileIndex);

			UObject::BeginLoad();
			ULinkerLoad* Linker = UObject::GetPackageLinker( NULL, *Filename, LOAD_None, NULL, NULL );
			UObject::EndLoad();

			if( Linker )
			{
				check(Linker->LinkerRoot);
				check(Linker->Summary.PackageFlags & PKG_Cooked);

				// Display information about the package.
				FName LinkerName = Linker->LinkerRoot->GetFName();

				// Display summary info.
				GWarn->Logf( TEXT("********************************************") );
				GWarn->Logf( TEXT("Package '%s' Summary"), *LinkerName.ToString() );
				GWarn->Logf( TEXT("--------------------------------------------") );

				GWarn->Logf( TEXT("\t     Version: %i"), Linker->Ver() );
				GWarn->Logf( TEXT("\tPackageFlags: %x"), Linker->Summary.PackageFlags );
				GWarn->Logf( TEXT("\t   NameCount: %d"), Linker->Summary.NameCount );
				GWarn->Logf( TEXT("\t  NameOffset: %d"), Linker->Summary.NameOffset );
				GWarn->Logf( TEXT("\t ImportCount: %d"), Linker->Summary.ImportCount );
				GWarn->Logf( TEXT("\tImportOffset: %d"), Linker->Summary.ImportOffset );
				GWarn->Logf( TEXT("\t ExportCount: %d"), Linker->Summary.ExportCount );
				GWarn->Logf( TEXT("\tExportOffset: %d"), Linker->Summary.ExportOffset );

				FString szGUID = Linker->Summary.Guid.String();
				GWarn->Logf( TEXT("\t        Guid: %s"), *szGUID );
				GWarn->Logf( TEXT("\t Generations:"));
				for( INT i=0; i<Linker->Summary.Generations.Num(); i++ )
				{
					const FGenerationInfo& GenerationInfo = Linker->Summary.Generations( i );
					GWarn->Logf( TEXT("\t\t%d) ExportCount=%d, NameCount=%d"), i, GenerationInfo.ExportCount, GenerationInfo.NameCount );
				}

				GWarn->Logf( TEXT("") );
				GWarn->Logf( TEXT("Exports:") );
				GWarn->Logf( TEXT("Class Outer Name Size Offset ExportFlags ObjectFlags") );

				for( INT i = 0; i < Linker->ExportMap.Num(); ++i )
				{
					FObjectExport& Export = Linker->ExportMap( i );

					// Find the name of this object's class.
					INT ClassIndex	= Export.ClassIndex;
					FName ClassName = NAME_Class;
					if( ClassIndex > 0 )
					{
						ClassName = Linker->ExportMap(ClassIndex-1).ObjectName;
					}
					else if( ClassIndex < 0 )
					{
						Linker->ImportMap(-ClassIndex-1).ObjectName;
					}

					// Find the name of this object's Outer.  For UClasses, this will generally be the
					// top-level package itself.  For properties, a UClass, etc.
					FName OuterName = NAME_None;
					if ( Export.OuterIndex > 0 )
					{
						FObjectExport& OuterExport = Linker->ExportMap(Export.OuterIndex-1);
						OuterName = OuterExport.ObjectName;
					}
					else if ( Export.OuterIndex < 0 )
					{
						FObjectImport& OuterImport = Linker->ImportMap(-Export.OuterIndex-1);
						OuterName = OuterImport.ObjectName;
					}

					//GWarn->Logf( TEXT("Class Outer Name Size Offset ExportFlags ObjectFlags") );
					GWarn->Logf( TEXT("%s %s %s %i %i %x 0x%016I64X"), 
						*ClassName.ToString(), 
						*OuterName.ToString(), 
						*Export.ObjectName.ToString(), 
						Export.SerialSize, 
						Export.SerialOffset, 
						Export.ExportFlags, 
						Export.ObjectFlags );
				}
			}

			UObject::CollectGarbage(RF_Native);
		}
	}
	return 0;
}
IMPLEMENT_CLASS(UAnalyzeCookedPackagesCommandlet)

/*-----------------------------------------------------------------------------
	UMineCookedPackages commandlet.
-----------------------------------------------------------------------------*/

INT UMineCookedPackagesCommandlet::Main( const FString& Params )
{
	// Parse command line.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	const TCHAR* Parms = *Params;
	ParseCommandLine(Parms, Tokens, Switches);

	// Create the connection object; needs to be deleted via "delete".
	FDataBaseConnection* Connection = FDataBaseConnection::CreateObject();
	
	// Name of database to use.
	FString DataSource;
	if( !Parse( Parms, TEXT("-DATABASE="), DataSource ) )
	{
		warnf( TEXT("Use -DATABASE= to specify database.") );
		return -1;
	}

	// Name of catalog to use.
	FString Catalog;
	if( !Parse( Parms, TEXT("CATALOG="), Catalog ) )
	{
		warnf( TEXT("Use -CATALOG= to specify catalog.") );
		return -2;
	}

	// Create the connection string with Windows Authentication as the way to handle permissions/ login/ security.
	FString ConnectionString	= FString::Printf(TEXT("Provider=sqloledb;Data Source=%s;Initial Catalog=%s;Trusted_Connection=Yes;"),*DataSource,*Catalog);

	// Try to open connection to DB - this is a synchronous operation.
	if( Connection->Open( *ConnectionString, NULL, NULL ) )
	{
		warnf(NAME_DevDataBase,TEXT("Connection to %s.%s succeeded"),*DataSource,*Catalog);
	}
	// Connection failed :(
	else
	{
		warnf(NAME_DevDataBase,TEXT("Connection to %s.%s failed"),*DataSource,*Catalog);
		// Only delete object - no need to close as connection failed.
		delete Connection;
		Connection = NULL;
		// Early out on error.
		return -1;
	}

	// Make sure all classes are loaded.
	TArray<FString> ScriptPackageNames;
	appGetEngineScriptPackageNames( ScriptPackageNames, TRUE );
	appGetGameScriptPackageNames( ScriptPackageNames, TRUE );
	extern void LoadPackageList(const TArray<FString>& PackageNames);
	LoadPackageList( ScriptPackageNames );

	// Iterate over all classes, adding them to the DB.
	for( TObjectIterator<UClass> It; It; ++It )
	{
		UClass* Class = *It;
		UClass* Super = Class->GetSuperClass();
		TArray<UClass*> InheritanceTree;

		// Add all super classes, down to NULL class terminating chain.
		InheritanceTree.AddItem(Super);
		while( Super )
		{
			Super = Super->GetSuperClass();
			InheritanceTree.AddItem(Super);
		};

		// Add classes in reverse order so we can rely on the fact that super classes are added to the DB first. The ADDCLASS stored procedure
		// has special case handling for the None class.
		for( INT i=InheritanceTree.Num()-1; i>=0; i-- )
		{
			Super = InheritanceTree(i);

			// Add class, super, depth. It'll be multiple rows per class and we can do a select based on depth == 0 to get unique ones.
			FString ClassString = FString::Printf(TEXT("EXEC ADDCLASS @CLASSNAME='%s', @SUPERNAME='%s', @SUPERDEPTH=%i"),*Class->GetName(),*Super->GetName(),i);
			verify( Connection->Execute( *ClassString ) );
		}
	}

	// Collect garbage to unload previously loaded packages before we reset loaders.
	UObject::CollectGarbage(RF_Native);

	// Reset loaders to avoid finding packages that are already loaded. This will load bulk data associated with those objects, which is why
	// we trim down the set of loaded objects via GC before.
	UObject::ResetLoaders( NULL );
	
	// We need to add dependencies after adding all packages as the stored procedure relies on the packages to already
	// have been added at the time of execution. We do this by keeping track of all SQL calls to execute at the end. 
	TArray<FString> PackageDependencies;

	// Tokens on the command line are package wildcards.
	for( INT TokenIndex=0; TokenIndex<Tokens.Num(); TokenIndex++ )
	{
		// Find all files matching current wildcard.
		FFilename		Wildcard = Tokens(TokenIndex);
		TArray<FString> Filenames;
		GFileManager->FindFiles( Filenames, *Wildcard, TRUE, FALSE );

		// Iterate over all found files.
		for( INT FileIndex = 0; FileIndex < Filenames.Num(); FileIndex++ )
		{
			const FFilename& Filename = Wildcard.GetPath() + PATH_SEPARATOR + Filenames(FileIndex);
			debugf(TEXT("Handling %s"),*Filename);

			// Code currently doesn't handle fully compressed files.
			if( GFileManager->UncompressedFileSize( *Filename ) != -1 )
			{
				warnf(TEXT("Skipping fully compressed file %s"),*Filename);
				continue;
			}

			// Pick the right platform compression format for fully compressed files.
			if( Filename.ToUpper().InStr(TEXT("COOKEDXENON")) != INDEX_NONE )
			{
				GBaseCompressionMethod = COMPRESS_DefaultXbox360;
			}
			else if( Filename.ToUpper().InStr(TEXT("COOKEDPS3")) != INDEX_NONE )
			{
				GBaseCompressionMethod = COMPRESS_DefaultPS3;
			}
			else
			{
				GBaseCompressionMethod = COMPRESS_Default;
			}

			// Get the linker. ResetLoaders above guarantees that it has to be loded from disk. 
			// This won't load the package but rather just the package file summary and header.
			UObject::BeginLoad();
			ULinkerLoad* Linker = UObject::GetPackageLinker( NULL, *Filename, LOAD_None, NULL, NULL );
			UObject::EndLoad();

			if( Linker && (Linker->Summary.PackageFlags & PKG_Cooked) )
			{
				warnf(TEXT("Mining %s"), *Filename);

				check(Linker->LinkerRoot);

				// Add package information to DB.
				FString PackageName = Linker->LinkerRoot->GetName();
				// Chunk 0 contains total size information for file.
				check( Linker->Summary.CompressedChunks.Num() );
				INT CompressedSize		= Linker->Summary.CompressedChunks(0).CompressedSize;
				INT UncompressedSize	= Linker->Summary.CompressedChunks(0).UncompressedSize;
				INT NameTableSize		= Linker->Summary.ImportOffset - Linker->Summary.NameOffset;
				INT	ImportTableSize		= Linker->Summary.ExportOffset - Linker->Summary.ImportOffset;
				INT	ExportTableSize		= Linker->Summary.DependsOffset - Linker->Summary.ExportOffset;
				UBOOL bIsMapPackage		= (Linker->Summary.PackageFlags & PKG_ContainsMap);

				FString PackageString = FString::Printf(TEXT("EXEC ADDPACKAGE @PACKAGENAME='%s', @BISMAPPACKAGE='%s', @COMPRESSEDSIZE=%i, @UNCOMPRESSEDSIZE=%i, @TOTALHEADERSIZE=%i, "),
																*PackageName,
																bIsMapPackage ? TEXT("TRUE") : TEXT("FALSE"),
																CompressedSize,
																UncompressedSize,
																Linker->Summary.TotalHeaderSize );
				PackageString += FString::Printf(TEXT("@NAMETABLESIZE=%i, @IMPORTTABLESIZE=%i, @EXPORTTABLESIZE=%i, @NAMECOUNT=%i, @IMPORTCOUNT=%i, @EXPORTCOUNT=%i"),
																NameTableSize,
																ImportTableSize,
																ExportTableSize,
																Linker->Summary.NameCount,
																Linker->Summary.ImportCount,
																Linker->Summary.ExportCount );
				verify( Connection->Execute( *PackageString ) );

				// Keep track of package dependencies to execute at the end
				for( INT DependencyIndex=0; DependencyIndex < Linker->Summary.AdditionalPackagesToCook.Num(); DependencyIndex++ )
				{
					const FString& DependencyName = Linker->Summary.AdditionalPackagesToCook(DependencyIndex);
					new(PackageDependencies) FString(*FString::Printf(TEXT("ADDPACKAGEDEPENDENCY @PACKAGENAME='%s', @DEPENDENCYNAME='%s'"),*PackageName,*DependencyName));
				}


				// Iterate over all exports, including forced ones, and add their information to the DB.
				for( INT ExportIndex = 0; ExportIndex < Linker->ExportMap.Num(); ++ExportIndex )
				{
					FObjectExport& Export = Linker->ExportMap( ExportIndex );

					// Find the name of this object's class.
					FName ClassName = NAME_Class;
					if( Export.ClassIndex > 0 )
					{
						ClassName = Linker->ExportMap(Export.ClassIndex-1).ObjectName;
					}
					else if( Export.ClassIndex < 0 )
					{
						ClassName = Linker->ImportMap(-Export.ClassIndex-1).ObjectName;
					}

					// Look up the name of this export. Correctly handles forced exports.
					FString ObjectName = Linker->GetExportPathName(ExportIndex - 1,NULL,TRUE);
					
					// Add export.
					FString ExportString = FString::Printf(TEXT("EXEC ADDEXPORT @PACKAGENAME='%s', @CLASSNAME='%s', @OBJECTNAME='%s', @SIZE=%i"),
																		*PackageName, 
																		*ClassName.ToString(), 
																		*ObjectName, 
																		Export.SerialSize );
					verify( Connection->Execute( *ExportString ) );
				}
			}
			else
			{
				warnf(TEXT("Error opening %s. Skipping."),*Filename);
			}

			UObject::CollectGarbage(RF_Native);
		}
	}

	// Add dependency information now that all packages have been added.
	for( INT DependencyIndex=0; DependencyIndex < PackageDependencies.Num(); DependencyIndex++ )
	{
		verify( Connection->Execute( *PackageDependencies(DependencyIndex) ) );
	}

	// Close & delete connection;
	Connection->Close();
	delete Connection;
	Connection = NULL;

	return 0;
}
IMPLEMENT_CLASS(UMineCookedPackagesCommandlet)


/**
 * This will look for Textures which:
 *
 *   0) are probably specular (based off their name) and see if they have an LODBias of two
 *   1) have a negative LODBias
 *   2) have neverstream set
 *   3) a texture which looks to be a normalmap but doesn't have the correct texture compression set 
 *
 * All of the above are things which can be considered suspicious and probably should be changed
 * for best memory usage.  
 *
 * Specifically:  Specular with an LODBias of 2 was not noticeably different at the texture resolutions we used for gears; 512+
 *
 **/
struct TextureCheckerFunctor
{
	template< typename OBJECTYPE >
	void DoIt( UPackage* Package, TArray<FString>& Tokens, TArray<FString>& Switches )
	{
		for( TObjectIterator<OBJECTYPE> It; It; ++It )
		{
			OBJECTYPE* Texture2D = *It;

			if( Texture2D->IsIn( Package ) == FALSE )
			{
				continue;
			}

			const FString&  TextureName = Texture2D->GetPathName();
			const INT		LODBias     = Texture2D->LODBias;

			FString OrigDescription = TEXT( "" );
			TArray<FString> TextureGroupNames = FTextureLODSettings::GetTextureGroupNames();
			if( Texture2D->LODGroup < TextureGroupNames.Num() )
			{
				OrigDescription = TextureGroupNames(Texture2D->LODGroup);
			}

			//warnf( TEXT( " checking %s" ), *TextureName );

			// if we are a TEXTUREGROUP_Cinematic then we don't care as that will be some crazy size / settings to look awesome :-)
			// TEXTUREGROUP_Cinematic:  implies being baked out
			if( Texture2D->LODGroup == TEXTUREGROUP_Cinematic )
			{
				continue;
			}

			// if this has been named as a specular texture
			if( ( ( TextureName.ToUpper().InStr( TEXT("SPEC" )) != INDEX_NONE )  // gears
				   || ( TextureName.ToUpper().InStr( TEXT("_S0" )) != INDEX_NONE )  // ut
				   || ( TextureName.ToUpper().InStr( TEXT("_S_" )) != INDEX_NONE )  // ut
				   || ( (TextureName.ToUpper().Right(2)).InStr( TEXT("_S" )) != INDEX_NONE )  // ut
				   )
				&& ( !( ( LODBias == 2 ) 
       				      || ( Texture2D->LODGroup == TEXTUREGROUP_WorldSpecular )
						  || ( Texture2D->LODGroup == TEXTUREGROUP_CharacterSpecular )
						  || ( Texture2D->LODGroup == TEXTUREGROUP_WeaponSpecular )
						  || ( Texture2D->LODGroup == TEXTUREGROUP_VehicleSpecular )
					   )
				    )
				)		 
			{
				warnf( TEXT("%s:  Specular LODBias of 2 not correct.  ( Currently has %d )  OR not set to a SpecularTextureGroup (Currently has: %d (%s))"), *TextureName, LODBias, Texture2D->LODGroup, *OrigDescription );
			}

			if( (
				   ( TextureName.ToUpper().InStr( TEXT("_N0" )) != INDEX_NONE )  // ut
				|| ( TextureName.ToUpper().InStr( TEXT("_N_" )) != INDEX_NONE )  // ut
				|| ( (TextureName.ToUpper().Right(2)).InStr( TEXT("_N" )) != INDEX_NONE )  // ut
				|| ( Texture2D->LODGroup == TEXTUREGROUP_WorldNormalMap )
				|| ( Texture2D->LODGroup == TEXTUREGROUP_CharacterNormalMap )
				|| ( Texture2D->LODGroup == TEXTUREGROUP_WeaponNormalMap )
				|| ( Texture2D->LODGroup == TEXTUREGROUP_VehicleNormalMap )
				 ) )
			{
				// this first check should probably run for TC_NormalMap only.
				// NOTE: Technically, the check could simply require DXT1 since there doesn't
				//  appear to be a way to have the engine use anything else.
				if( ( Texture2D->CompressionSettings == TC_Normalmap)
					&& (( Texture2D->Format != PF_DXT1 )
					&& ( Texture2D->CompressionNoAlpha == FALSE ) )
					)
				{
					warnf( TEXT( "%s:  Normalmap CompressionNoAlpha should be TRUE or Format should be DXT1" ), *TextureName );
				}

				// checks for TC_NormalMapAlpha
				if( ( Texture2D->CompressionSettings == TC_NormalmapAlpha)
					&& (( Texture2D->Format == PF_DXT1 )
					|| ( Texture2D->CompressionNoAlpha != FALSE ) )
					)
				{
					warnf( TEXT( "%s: NormalMapAlpha CompressionNoAlpha should be FALSE and Format should not be DXT1" ), *TextureName );
				}

				// SRGB should be false
				if( Texture2D->SRGB )
				{
					warnf( TEXT( "%s: Normalmap should have SRGB = FALSE"), *TextureName);
				}
			}



			// if this has a negative LOD Bias that might be suspicious :-) (artists will often bump up the LODBias on their pet textures to make certain they are full full res)
			if( ( LODBias < 0 )
				)
			{
				warnf( TEXT("%s:  LODBias is negative ( Currently has %d )"), *TextureName, LODBias );
			}

			if( ( Texture2D->NeverStream == TRUE )
				&& ( TextureName.InStr( TEXT("TerrainWeightMapTexture" )) == INDEX_NONE )  // TerrainWeightMapTextures are neverstream so we don't care about them
				
				)
			{
				warnf( TEXT("%s:  NeverStream is set to true" ), *TextureName );
			}

			// if this texture is a G8 it usually can be converted to a dxt without loss of quality.  Remember a 1024x1024 G* is always 1024!
			if( Texture2D->CompressionSettings == TC_Grayscale )
			{
				warnf( TEXT("%s:  G8 texture"), *TextureName );
			}

		}
	}
};


/**
* This will find materials which are missing Physical Materials
**/
struct MaterialMissingPhysMaterialFunctor
{
	template< typename OBJECTYPE >
	void DoIt( UPackage* Package, TArray<FString>& Tokens, TArray<FString>& Switches )
	{
		for( TObjectIterator<OBJECTYPE> It; It; ++It )
		{
			OBJECTYPE* Material = *It;

			if( Material->IsIn( Package ) == FALSE )
			{
				continue;
			}

			UBOOL bHasPhysicalMaterial = FALSE;

			if( Material->PhysMaterial != NULL )
			{
				// if we want to do some other logic such as looking at the textures
				// used and seeing if they are all of one type (e.g. all effects textures probably
				// imply that the material is used in an effect and probably doesn't need a physical material)
				//TArray<UTexture*> OutTextures;
				//Material->GetTextures( OutTextures, TRUE );
				//const FTextureLODGroup& LODGroup = TextureLODGroups[Texture->LODGroup];

				bHasPhysicalMaterial = TRUE;
			}

			if( bHasPhysicalMaterial == FALSE )
			{
				const FString& MaterialName = Material->GetPathName();
				warnf( TEXT("%s:  Lacking PhysicalMaterial"), *MaterialName  );
			}
		}
	}
};


/** Will find all of the PhysicsAssets bodies which do not have a phys material assigned to them **/
struct SkeletalMeshesMissingPhysMaterialFunctor
{
	template< typename OBJECTYPE >
	void DoIt( UPackage* Package, TArray<FString>& Tokens, TArray<FString>& Switches )
	{
		for( TObjectIterator<OBJECTYPE> It; It; ++It )
		{
			OBJECTYPE* PhysicsAsset = *It;

			if( PhysicsAsset->IsIn( Package ) == FALSE )
			{
				continue;
			}

			for( INT i = 0; i < PhysicsAsset->BodySetup.Num(); ++i )
			{
				const UPhysicalMaterial* PhysMat = PhysicsAsset->BodySetup(i)->PhysMaterial;
				if( PhysMat == NULL )
				{
					warnf( TEXT( "Missing PhysMaterial on PhysAsset: %s Bone: %s" ), *PhysicsAsset->GetPathName(), *PhysicsAsset->BodySetup(i)->BoneName.ToString() );
				}
			}
		}
	}
};



struct AddAllSkeletalMeshesToListFunctor
{
	template< typename OBJECTYPE >
	void DoIt( UPackage* Package, TArray<FString>& Tokens, TArray<FString>& Switches )
	{
		for( TObjectIterator<OBJECTYPE> It; It; ++It )
		{
			OBJECTYPE* SkelMesh = *It;
			SkelMesh->AddToRoot();
		}
	}
};

/**
 * This will find materials which are missing Physical Materials
 */
struct CompressAnimationsFunctor
{
	template< typename OBJECTYPE >
	void DoIt( UPackage* Package, TArray<FString>& Tokens, TArray<FString>& Switches )
	{
		UBOOL bDirtyPackage = FALSE;
		const FName& PackageName = Package->GetFName(); 
		FString PackageFileName;
		GPackageFileCache->FindPackageFile( *PackageName.ToString(), NULL, PackageFileName );

#if HAVE_SCC
		TScopedPointer<FSourceControlIntegration> SCC(new FSourceControlIntegration);
#endif

		/** If we're analyzing, we're not actually going to recompress, so we can skip some significant work. */
		UBOOL bAnalyze = Switches.ContainsItem(TEXT("ANALYZE"));

		for( TObjectIterator<OBJECTYPE> It; It; ++It )
		{
			OBJECTYPE* AnimSeq = *It;

			if( !AnimSeq->IsIn(Package) )
			{
				continue;
			}

			UAnimSet* AnimSet = AnimSeq->GetAnimSet();
			check(AnimSet != NULL);

			if( bAnalyze )
			{
				if( AnimSeq->NumFrames > 2 && AnimSeq->KeyEncodingFormat != AKF_VariableKeyLerp )
				{
					++AnalyzeCompressionCandidates;
					warnf(TEXT("[%i] Animation could be recompressed: %s (%s), frames: %i, length: %f, size: %i bytes, compression scheme: %s"), 
						AnalyzeCompressionCandidates, *AnimSeq->SequenceName.ToString(), *AnimSet->GetFullName(),  AnimSeq->NumFrames, AnimSeq->SequenceLength, AnimSeq->GetResourceSize(), AnimSeq->CompressionScheme ? *AnimSeq->CompressionScheme->GetClass()->GetName() : TEXT("NULL"));
				}

				continue;
			}

			FLOAT HighestRatio = 0.f;
			USkeletalMesh*	BestSkeletalMeshMatch = NULL;

			// Test preview skeletal mesh
			USkeletalMesh* DefaultSkeletalMesh = LoadObject<USkeletalMesh>(NULL, *AnimSet->PreviewSkelMeshName.ToString(), NULL, LOAD_None, NULL);
			FLOAT DefaultMatchRatio = 0.f;
			if( DefaultSkeletalMesh )
			{
				DefaultMatchRatio = AnimSet->GetSkeletalMeshMatchRatio(DefaultSkeletalMesh);
			}

			// If our default mesh doesn't have a full match ratio, then see if we can find a better fit.
			if( DefaultMatchRatio < 1.f )
			{
				// Find the most suitable SkeletalMesh for this AnimSet
				for( TObjectIterator<USkeletalMesh> ItMesh; ItMesh; ++ItMesh )
				{
					USkeletalMesh* SkelMeshCandidate = *ItMesh;
					if( SkelMeshCandidate != DefaultSkeletalMesh )
					{
						FLOAT MatchRatio = AnimSet->GetSkeletalMeshMatchRatio(SkelMeshCandidate);
						if( MatchRatio > HighestRatio )
						{
							BestSkeletalMeshMatch = SkelMeshCandidate;
							HighestRatio = MatchRatio;

							// If we have found a perfect match, we can abort.
							if( Abs(1.f - MatchRatio) <= KINDA_SMALL_NUMBER )
							{
								break;
							}
						}
					}
				}

				// If we have found a best match
				if( BestSkeletalMeshMatch )
				{
					// if it is different than our preview mesh and his match ratio is higher
					// then replace preview mesh with this one, as it's a better match.
					if( BestSkeletalMeshMatch != DefaultSkeletalMesh && HighestRatio > DefaultMatchRatio )
					{
						warnf(TEXT("Found more suitable preview mesh for %s (%s): %s (%f) instead of %s (%f)."), 
							*AnimSeq->SequenceName.ToString(), *AnimSet->GetFullName(), *BestSkeletalMeshMatch->GetFName().ToString(), HighestRatio, *AnimSet->PreviewSkelMeshName.ToString(), DefaultMatchRatio);

						// We'll now use this one from now on as it's a better fit.
						AnimSet->PreviewSkelMeshName = FName( *BestSkeletalMeshMatch->GetPathName() );
						AnimSet->MarkPackageDirty();

						DefaultSkeletalMesh = BestSkeletalMeshMatch;
						bDirtyPackage = TRUE;
					}
				}
				else
				{
					warnf(TEXT("Could not find suitable mesh for %s (%s) !!! Default was %s"), 
							*AnimSeq->SequenceName.ToString(), *AnimSet->GetFullName(), *AnimSet->PreviewSkelMeshName.ToString());
				}
			}

			INT OldSize = AnimSeq->GetResourceSize();
			FAnimationUtils::CompressAnimSequence(AnimSeq, DefaultSkeletalMesh, TRUE, FALSE);
			INT NewSize = AnimSeq->GetResourceSize();

			// Only save package if size has changed.
			bDirtyPackage = (bDirtyPackage || (OldSize != NewSize));
		}

		// If we need to save package, do so.
		if( bDirtyPackage && !bAnalyze )
		{
			/** if we should auto checkout packages that need to be saved**/
			const UBOOL bAutoCheckOut = Switches.ContainsItem(TEXT("AUTOCHECKOUTPACKAGES"));
			// see if we should skip read only packages.
			UBOOL bIsReadOnly = GFileManager->IsReadOnly( *PackageFileName);

			// check to see if we need to check this package out
			if( bIsReadOnly == TRUE && bAutoCheckOut == TRUE )
			{
#if HAVE_SCC
				SCC->CheckOut(Package);
#endif // HAVE_SCC
			}

			bIsReadOnly = GFileManager->IsReadOnly( *PackageFileName);
			if( bIsReadOnly == FALSE )
			{
				try
				{
					if( SavePackageHelper( Package, PackageFileName ) == TRUE )
					{
						warnf( NAME_Log, TEXT("Correctly saved:  [%s]."), *PackageFileName );
					}
				}
				catch( ... )
				{
					warnf( NAME_Log, TEXT("Lame Exception %s"), *PackageFileName );
				}
			}

		}

	}
};

struct SetTextureLODGroupFunctor
{
	template< typename OBJECTYPE >
	void DoIt( UPackage* Package, TArray<FString>& Tokens, TArray<FString>& Switches )
	{
		UBOOL bDirtyPackage = FALSE;

		const FName& PackageName = Package->GetFName(); 
		FString PackageFileName;
		GPackageFileCache->FindPackageFile( *PackageName.ToString(), NULL, PackageFileName );
		//warnf( NAME_Log, TEXT("  Loading2 %s"), *PackageFileName );

		for( TObjectIterator<OBJECTYPE> It; It; ++It )
		{
			OBJECTYPE* Texture2D = *It;

			if( Texture2D->IsIn( Package ) == FALSE )
			{
				continue;
			}

			if( Package->GetLinker() != NULL
				&& Package->GetLinker()->Summary.EngineVersion == 2904 )
			{
				warnf( NAME_Log, TEXT( "Already 2904" ) );
				continue;
			}

			UBOOL bDirty = FALSE;
			UBOOL bIsSpec = FALSE;
			UBOOL bIsNormal = FALSE;

			const FString& TextureName = Texture2D->GetPathName();

			if( ParseParam(appCmdLine(),TEXT("VERBOSE")) == TRUE )
			{
				warnf( NAME_Log, TEXT( "TextureName: %s" ), *TextureName );
			}

			// do not set the LOD on in editor ThumbnailTextures
			if( (TextureName.Right(16)).InStr( TEXT("ThumbnailTexture" )) != INDEX_NONE )
			{
				continue;
			}


			const BYTE OrigGroup = Texture2D->LODGroup;
			const INT OrigLODBias = Texture2D->LODBias;

			// due to enum fiasco 2007 now we need to find which "type" it is based off the package name
			enum EPackageType
			{
				PACKAGE_CHARACTER,
				PACKAGE_EFFECTS,
				PACKAGE_SKYBOX,
				PACKAGE_UI,
				PACKAGE_VEHICLE,
				PACKAGE_WEAPON,
				PACKAGE_WORLD,
			};


			EPackageType ThePackageType = PACKAGE_WORLD;

			// DETERMINE which package type this package is

			// if not in the special effects non filtered group already
			if( ( PackageFileName.ToUpper().InStr( TEXT("EFFECTS") ) != INDEX_NONE ) )
			{
				ThePackageType = PACKAGE_EFFECTS;
			}
			else if( ( PackageName.ToString().Left(3).InStr( TEXT("CH_") ) != INDEX_NONE )
				|| ( PackageFileName.ToUpper().InStr( TEXT("CHARACTER" )) != INDEX_NONE ) 
				|| ( PackageFileName.ToUpper().InStr( TEXT("SOLDIERS" )) != INDEX_NONE ) 
				|| ( PackageFileName.ToUpper().InStr( TEXT("NPC" )) != INDEX_NONE ) 
				)
			{
				ThePackageType = PACKAGE_CHARACTER;
			}
			else if( ( PackageName.ToString().Left(3).InStr( TEXT("VH_") ) != INDEX_NONE )
				|| ( PackageFileName.ToUpper().InStr( TEXT("VEHICLE" )) != INDEX_NONE ) 
				)
			{
				ThePackageType = PACKAGE_VEHICLE;
			}
			else if( ( PackageName.ToString().Left(3).InStr( TEXT("WP_") ) != INDEX_NONE )
				|| ( PackageFileName.ToUpper().InStr( TEXT("WEAPON" )) != INDEX_NONE ) 
				)
			{
				ThePackageType = PACKAGE_WEAPON;
			}

			else if( ( PackageName.ToString().Left(3).InStr( TEXT("UI_") ) != INDEX_NONE )
				|| ( PackageFileName.ToUpper().InStr( TEXT("INTERFACE" )) != INDEX_NONE ) 
				)
			{
				ThePackageType = PACKAGE_UI;
			}
			else if( PackageFileName.InStr( TEXT("Sky") ) != INDEX_NONE )
			{
				ThePackageType = PACKAGE_SKYBOX;
			}
			else
			{
				ThePackageType = PACKAGE_WORLD;
			}


			// if this is a specular texture
			if( ( ( TextureName.ToUpper().InStr( TEXT("SPEC" )) != INDEX_NONE )  // gears
				|| ( TextureName.ToUpper().InStr( TEXT("_S0" )) != INDEX_NONE )  // ut
				|| ( TextureName.ToUpper().InStr( TEXT("_S_" )) != INDEX_NONE )  // ut
				|| ( (TextureName.ToUpper().Right(2)).InStr( TEXT("_S" )) != INDEX_NONE )  // ut
				|| ( (TextureName.ToUpper().Right(4)).InStr( TEXT("_SPM" )) != INDEX_NONE ) // specular mask
				|| ( (TextureName.ToUpper().Right(4)).InStr( TEXT("-SPM" )) != INDEX_NONE ) // specular mask (this is incorrect naming convention and will be expunged post haste)
				   )
				)
			{
				bIsSpec = TRUE;

				// all spec LOD should be 0 as we are going to be using a group now for setting it
				if( Texture2D->LODBias != 0 )
				{
					Texture2D->LODBias = 0; 
					bDirty = TRUE; 
				}

				if( ( ThePackageType == PACKAGE_WORLD ) && ( Texture2D->LODGroup != TEXTUREGROUP_WorldSpecular ) )
				{
					Texture2D->LODGroup = TEXTUREGROUP_WorldSpecular; 
					bDirty = TRUE; 
				}
				else if( ( ThePackageType == PACKAGE_CHARACTER ) && ( Texture2D->LODGroup != TEXTUREGROUP_CharacterSpecular ) )
				{
					Texture2D->LODGroup = TEXTUREGROUP_CharacterSpecular; 
					bDirty = TRUE; 
				}
				else if( ( ThePackageType == PACKAGE_WEAPON ) && ( Texture2D->LODGroup != TEXTUREGROUP_WeaponSpecular ) )
				{
					Texture2D->LODGroup = TEXTUREGROUP_WeaponSpecular; 
					bDirty = TRUE; 
				}
				else if( ( ThePackageType == PACKAGE_VEHICLE ) && ( Texture2D->LODGroup != TEXTUREGROUP_VehicleSpecular ) )
				{
					Texture2D->LODGroup = TEXTUREGROUP_VehicleSpecular; 
					bDirty = TRUE; 
				}
			}
		
			if( 
				( TextureName.ToUpper().InStr( TEXT("_N0" )) != INDEX_NONE )  // ut
				|| ( TextureName.ToUpper().InStr( TEXT("_N_" )) != INDEX_NONE )  // ut
				|| ( (TextureName.ToUpper().Right(2)).InStr( TEXT("_N" )) != INDEX_NONE )  // ut
				|| ( TextureName.ToUpper().InStr( TEXT("_NORMAL" )) != INDEX_NONE )
				|| ( Texture2D->CompressionSettings == TC_Normalmap )
				|| ( Texture2D->CompressionSettings == TC_NormalmapAlpha ) 
				)
			{
				bIsNormal = TRUE;

				if( ( ThePackageType == PACKAGE_WORLD ) && ( Texture2D->LODGroup != TEXTUREGROUP_WorldNormalMap ) )
				{
					Texture2D->LODGroup = TEXTUREGROUP_WorldNormalMap; 
					bDirty = TRUE; 
				}
				else if( ( ThePackageType == PACKAGE_CHARACTER ) && ( Texture2D->LODGroup != TEXTUREGROUP_CharacterNormalMap ))
				{
					Texture2D->LODGroup = TEXTUREGROUP_CharacterNormalMap; 
					bDirty = TRUE; 
				}
				else if( ( ThePackageType == PACKAGE_WEAPON ) && ( Texture2D->LODGroup != TEXTUREGROUP_WeaponNormalMap ))
				{
					Texture2D->LODGroup = TEXTUREGROUP_WeaponNormalMap; 
					bDirty = TRUE; 
				}
				else if( ( ThePackageType == PACKAGE_VEHICLE ) && ( Texture2D->LODGroup != TEXTUREGROUP_VehicleNormalMap ) )
				{
					Texture2D->LODGroup = TEXTUREGROUP_VehicleNormalMap; 
					bDirty = TRUE; 
				}

				// here we set Compression setting for textures which have lost their settings
				if( !( ( Texture2D->CompressionSettings == TC_Normalmap ) || ( Texture2D->CompressionSettings == TC_NormalmapAlpha ) )
					)
				{
					warnf( TEXT( "%s:  Incorrect Normalmap CompressionSetting. Now setting: TC_Normalmap (was %d)" ), *TextureName, Texture2D->CompressionSettings );
					Texture2D->CompressionSettings = TC_Normalmap;
					Texture2D->Compress();
					bDirty = TRUE;
				}
			}


			if( ( ThePackageType == PACKAGE_UI ) && ( Texture2D->LODGroup != TEXTUREGROUP_UI ) )
			{
				Texture2D->LODGroup = TEXTUREGROUP_UI; 
				bDirty = TRUE; 
			}

			if( ( ( PackageFileName.InStr( TEXT("EngineFonts") ) != INDEX_NONE )
				|| ( PackageFileName.InStr( TEXT("UI_Fonts") ) != INDEX_NONE )
				) 
				&& ( Texture2D->LODGroup != TEXTUREGROUP_UI )
				)
			{
				Texture2D->LODGroup = TEXTUREGROUP_UI;
				bDirty = TRUE;
			}


			if( ( ThePackageType == PACKAGE_SKYBOX ) && ( Texture2D->LODGroup != TEXTUREGROUP_Skybox ) )
			{
				Texture2D->LODGroup = TEXTUREGROUP_Skybox; 
				bDirty = TRUE; 
			}

			//// check for subgroups (e.g. effects, decals, etc.)

			// we need to look for the Effects string in the Texture name as there are many cases where we do <package>.effects.<texture>  in a non Effects packages (e.g. VH_)  (subgroup is <package>.<subgroups>.<texture>
			UBOOL bIsInASubGroup = FALSE;
			if( ( ThePackageType == PACKAGE_EFFECTS )
				|| ( TextureName.InStr( TEXT("Effects") ) != INDEX_NONE ) // check to see if it is in a  <package>.effects.<texture>
				)
			{
				// and if the texture has not been specifically set to not be filtered (e.g. need high ansio filtering on it)
				if( ( Texture2D->LODGroup != TEXTUREGROUP_Effects ) && ( Texture2D->LODGroup != TEXTUREGROUP_EffectsNotFiltered ) )
				{
					Texture2D->LODGroup = TEXTUREGROUP_Effects;
					bDirty = TRUE; 
				}

				// we are in a named subgroup (i.e. effects)
				bIsInASubGroup = TRUE;
			}

			// SO now if we have not already modified the texture above then we need to
			// do one more final check to see if the texture in the package is actually
			// classified correctly
			if( ( bDirty == FALSE ) && ( bIsSpec == FALSE ) && ( bIsNormal == FALSE ) && ( bIsInASubGroup == FALSE ) )
			{
				if( ( ThePackageType == PACKAGE_CHARACTER ) && ( Texture2D->LODGroup != TEXTUREGROUP_Character ) )
				{
					Texture2D->LODGroup = TEXTUREGROUP_Character; 
					bDirty = TRUE; 
				}
				else if( ( ThePackageType == PACKAGE_WEAPON ) && ( Texture2D->LODGroup != TEXTUREGROUP_Weapon ) )
				{
					Texture2D->LODGroup = TEXTUREGROUP_Weapon; 
					bDirty = TRUE; 
				}
				else if( ( ThePackageType == PACKAGE_VEHICLE ) && ( Texture2D->LODGroup != TEXTUREGROUP_Vehicle ) )
				{
					Texture2D->LODGroup = TEXTUREGROUP_Vehicle; 
					bDirty = TRUE; 
				}
			}


			if( bDirty == TRUE )
			{
				bDirtyPackage = TRUE;
				FString OrigDescription = TEXT( "" );
				FString NewDescription = TEXT( "" );

				TArray<FString> TextureGroupNames = FTextureLODSettings::GetTextureGroupNames();
				if( OrigGroup < TextureGroupNames.Num() )
				{
					OrigDescription = TextureGroupNames(OrigGroup);
				}

				if( Texture2D->LODGroup < TextureGroupNames.Num() )
				{
					NewDescription = TextureGroupNames(Texture2D->LODGroup);
				}
				
				if( OrigLODBias != Texture2D->LODBias )
				{
					warnf( TEXT("%s:  Changing LODBias from:  %d to %d "), *TextureName, OrigLODBias, Texture2D->LODBias );
				}

				if( OrigGroup != Texture2D->LODGroup )
				{
					warnf( TEXT("%s:  Changing LODGroup from:  %d (%s) to %d (%s)"), *TextureName, OrigGroup, *OrigDescription, Texture2D->LODGroup, *NewDescription );
				}

			}

			bDirty = FALSE;
			bIsNormal = FALSE;
			bIsSpec = FALSE;
		}

#if HAVE_SCC
		if( bDirtyPackage == TRUE )
		{
			FSourceControlIntegration* SCC = new FSourceControlIntegration;

			/** if we should auto checkout packages that need to be saved**/
			const UBOOL bAutoCheckOut = ParseParam(appCmdLine(),TEXT("AutoCheckOutPackages"));

			// kk now we want to possible save the package
			UBOOL bIsReadOnly = GFileManager->IsReadOnly( *PackageFileName);

			// check to see if we need to check this package out
			if( bIsReadOnly == TRUE && bAutoCheckOut == TRUE )
			{
				SCC->CheckOut(Package);
			}

			bIsReadOnly = GFileManager->IsReadOnly( *PackageFileName);
			if( bIsReadOnly == FALSE )
			{
				try
				{
					if( SavePackageHelper( Package, PackageFileName ) == TRUE )
					{
						warnf( NAME_Log, TEXT("Correctly saved:  [%s]."), *PackageFileName );
					}
				}
				catch( ... )
				{
					warnf( NAME_Log, TEXT("Lame Exception %s"), *PackageFileName );
				}
			}

			delete SCC; // clean up our allocated SCC
			SCC = NULL;
		}
#endif // HAVE_SCC
	}
};



/*-----------------------------------------------------------------------------
FindMissingPhysicalMaterials commandlet.
-----------------------------------------------------------------------------*/

INT UFindTexturesWithMissingPhysicalMaterialsCommandlet::Main( const FString& Params )
{
	DoActionToAllPackages<UMaterial, MaterialMissingPhysMaterialFunctor>(Params);
	DoActionToAllPackages<UPhysicsAsset, SkeletalMeshesMissingPhysMaterialFunctor>(Params);

	return 0;
}
IMPLEMENT_CLASS(UFindTexturesWithMissingPhysicalMaterialsCommandlet)


/*-----------------------------------------------------------------------------
FindQuestionableTextures Commandlet
-----------------------------------------------------------------------------*/

INT UFindQuestionableTexturesCommandlet::Main( const FString& Params )
{
	DoActionToAllPackages<UTexture2D, TextureCheckerFunctor>(Params);

	return 0;
}
IMPLEMENT_CLASS(UFindQuestionableTexturesCommandlet)


/*-----------------------------------------------------------------------------
SetTextureLODGroup Commandlet
-----------------------------------------------------------------------------*/
INT USetTextureLODGroupCommandlet::Main( const FString& Params )
{
	DoActionToAllPackages<UTexture2D, SetTextureLODGroupFunctor>(Params);

	return 0;
}
IMPLEMENT_CLASS(USetTextureLODGroupCommandlet)


static INT AnalyzeCompressionCandidates = 0;

/*-----------------------------------------------------------------------------
CompressAnimations Commandlet
-----------------------------------------------------------------------------*/
INT UCompressAnimationsCommandlet::Main( const FString& Params )
{
	// Parse command line.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	// want everything in upper case, it's a mess otherwise
	const FString ParamsUpperCase = Params.ToUpper();
	const TCHAR* Parms = *ParamsUpperCase;
	UCommandlet::ParseCommandLine(Parms, Tokens, Switches);

	/** If we're analyzing, we're not actually going to recompress, so we can skip some significant work. */
	UBOOL bAnalyze = Switches.ContainsItem(TEXT("ANALYZE"));

	if( bAnalyze )
	{
		warnf(TEXT("Analyzing content for uncompressed animations..."));
		DoActionToAllPackages<UAnimSequence, CompressAnimationsFunctor>(ParamsUpperCase);

		warnf(TEXT("Done analyzing. Potential canditates: %i"), AnalyzeCompressionCandidates);
	}
	else
	{
	// First scan all Skeletal Meshes
	warnf(TEXT("Scanning for all SkeletalMeshes..."));

		// If we have SKIPREADONLY, then override this, as we need to scan all packages for skeletal meshes.
		FString SearchAllMeshesParams = ParamsUpperCase;
		SearchAllMeshesParams += FString(TEXT(" -OVERRIDEREADONLY"));
		SearchAllMeshesParams += FString(TEXT(" -OVERRIDELOADMAPS"));
		DoActionToAllPackages<USkeletalMesh, AddAllSkeletalMeshesToListFunctor>(SearchAllMeshesParams);

	INT Count = 0;
	for( TObjectIterator<USkeletalMesh> It; It; ++It )
	{
		USkeletalMesh* SkelMesh = *It;
		warnf(TEXT("[%i] %s"), Count, *SkelMesh->GetFName().ToString());
		Count++;
	}
	warnf(TEXT("%i SkeletalMeshes found!"), Count);

	// Then do the animation recompression
	warnf(TEXT("Recompressing all animations..."));
		DoActionToAllPackages<UAnimSequence, CompressAnimationsFunctor>(ParamsUpperCase);
	}
	

	return 0;
}
IMPLEMENT_CLASS(UCompressAnimationsCommandlet)

/** Useful for finding PS with bad FixedRelativeBoundingBox **/
struct FindParticleSystemsWithBaseFixedRelativeBoundingBox
{
	template< typename OBJECTYPE >
	void DoIt( UPackage* Package )
	{
		UBOOL bDirtyPackage = FALSE;

		const FName& PackageName = Package->GetFName(); 
		FString PackageFileName;
		GPackageFileCache->FindPackageFile( *PackageName.ToString(), NULL, PackageFileName );
		//warnf( NAME_Log, TEXT("  Loading2 %s"), *PackageFileName );

		for( TObjectIterator<OBJECTYPE> It; It; ++It )
		{
			OBJECTYPE* PS = *It;

			if( PS->IsIn( Package ) == FALSE )
			{
				continue;
			}

			if( 
				PS->bUseFixedRelativeBoundingBox == TRUE 
				&&	PS->FixedRelativeBoundingBox.Min.X == -1
				&& PS->FixedRelativeBoundingBox.Min.Y == -1
				&& PS->FixedRelativeBoundingBox.Min.Z == -1
				&& PS->FixedRelativeBoundingBox.Max.X ==  1
				&& PS->FixedRelativeBoundingBox.Max.Y ==  1
				&& PS->FixedRelativeBoundingBox.Max.Z ==  1
			)
			{
				warnf( TEXT("PS is has bad FixedRelativeBoundingBox: %s"), *PS->GetFullName() );
			}
		}
	}
};



/*-----------------------------------------------------------------------------
UListScriptReferencedContentCommandlet.
-----------------------------------------------------------------------------*/
/**
* Processes a value found by ListReferencedContent(), possibly recursing for inline objects
*
* @param	Value			the object to be processed
* @param	Property		the property where Value was found (for a dynamic array, this is the Inner property)
* @param	PropertyDesc	string printed as the property Value was assigned to (usually *Property->GetName(), except for dynamic arrays, where it's the array name and index)
* @param	Tab				string with a number of tabs for the current tab level of the output
*/
void UListScriptReferencedContentCommandlet::ProcessObjectValue(UObject* Value, UProperty* Property, const FString& PropertyDesc, const FString& Tab)
{
	if (Value != NULL)
	{
		// if it's an inline object, recurse over its properties
		if ((Property->PropertyFlags & CPF_NeedCtorLink) || (Property->PropertyFlags & CPF_Component))
		{
			ListReferencedContent(Value->GetClass(), (BYTE*)Value, FString(*Value->GetName()), Tab + TEXT("\t"));
		}
		else
		{
			// otherwise, print it as content that's being referenced
			warnf(TEXT("%s\t%s=%s'%s'"), *Tab, *PropertyDesc, *Value->GetClass()->GetName(), *Value->GetPathName());
		}
	}
}

/**
* Lists content referenced by the given data
*
* @param	Struct		the type of the Default data
* @param	Default		the data to look for referenced objects in
* @param	HeaderName	string printed before any content references found (only if the data might contain content references)
* @param	Tab			string with a number of tabs for the current tab level of the output
*/
void UListScriptReferencedContentCommandlet::ListReferencedContent(UStruct* Struct, BYTE* Default, const FString& HeaderName, const FString& Tab)
{
	UBOOL bPrintedHeader = FALSE;

	// iterate over all its properties
	for (UProperty* Property = Struct->PropertyLink; Property != NULL; Property = Property->PropertyLinkNext)
	{
		if ( !bPrintedHeader &&
			(Property->IsA(UObjectProperty::StaticClass()) || Property->IsA(UStructProperty::StaticClass()) || Property->IsA(UArrayProperty::StaticClass())) &&
			Property->ContainsObjectReference() )
		{
			// this class may contain content references, so print header with class/struct name
			warnf(TEXT("%s%s"), *Tab, *HeaderName);
			bPrintedHeader = TRUE;
		}
		// skip class properties and object properties of class Object
		UObjectProperty* ObjectProp = Cast<UObjectProperty>(Property);
		if (ObjectProp != NULL && ObjectProp->PropertyClass != UObject::StaticClass() && ObjectProp->PropertyClass != UClass::StaticClass())
		{
			if (ObjectProp->ArrayDim > 1)
			{
				for (INT i = 0; i < ObjectProp->ArrayDim; i++)
				{
					ProcessObjectValue(*(UObject**)(Default + Property->Offset + i * Property->ElementSize), Property, FString::Printf(TEXT("%s[%d]"), *Property->GetName(), i), Tab);
				}
			}
			else
			{
				ProcessObjectValue(*(UObject**)(Default + Property->Offset), Property, FString(*Property->GetName()), Tab);
			}
		}
		else if (Property->IsA(UStructProperty::StaticClass()))
		{
			if (Property->ArrayDim > 1)
			{
				for (INT i = 0; i < Property->ArrayDim; i++)
				{
					ListReferencedContent(((UStructProperty*)Property)->Struct, (Default + Property->Offset + i * Property->ElementSize), FString::Printf(TEXT("%s[%d]"), *Property->GetName(), i), Tab + TEXT("\t"));
				}
			}
			else
			{
				ListReferencedContent(((UStructProperty*)Property)->Struct, (Default + Property->Offset), FString(*Property->GetName()), Tab + TEXT("\t"));
			}
		}
		else if (Property->IsA(UArrayProperty::StaticClass()))
		{
			UArrayProperty* ArrayProp = (UArrayProperty*)Property;
			FScriptArray* Array = (FScriptArray*)(Default + Property->Offset);
			UObjectProperty* ObjectProp = Cast<UObjectProperty>(ArrayProp->Inner);
			if (ObjectProp != NULL && ObjectProp->PropertyClass != UObject::StaticClass() && ObjectProp->PropertyClass != UClass::StaticClass())
			{
				for (INT i = 0; i < Array->Num(); i++)
				{
					ProcessObjectValue(*(UObject**)((BYTE*)Array->GetData() + (i * ArrayProp->Inner->ElementSize)), ObjectProp, FString::Printf(TEXT("%s[%d]"), *ArrayProp->GetName(), i), Tab);
				}
			}
			else if (ArrayProp->Inner->IsA(UStructProperty::StaticClass()))
			{
				UStruct* InnerStruct = ((UStructProperty*)ArrayProp->Inner)->Struct;
				INT PropertiesSize = InnerStruct->GetPropertiesSize();
				for (INT i = 0; i < Array->Num(); i++)
				{
					ListReferencedContent(InnerStruct, (BYTE*)Array->GetData() + (i * ArrayProp->Inner->ElementSize), FString(*Property->GetName()), Tab + TEXT("\t"));
				}
			}
		}
	}
}

/** lists all content referenced in the default properties of script classes */
INT UListScriptReferencedContentCommandlet::Main(const FString& Params)
{
	warnf(TEXT("Loading EditPackages..."));
	// load all the packages in the EditPackages list
	BeginLoad();
	for (INT i = 0; i < GEditor->EditPackages.Num(); i++)
	{
		LoadPackage(NULL, *GEditor->EditPackages(i), 0);
	}
	EndLoad();

	// iterate over all classes
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (!(It->ClassFlags & CLASS_Intrinsic))
		{
			ListReferencedContent(*It, (BYTE*)It->GetDefaultObject(), FString::Printf(TEXT("%s %s"), *It->GetFullName(), It->HasAnyFlags(RF_Native) ? TEXT("(native)") : TEXT("")));
		}
	}

	return 0;
}
IMPLEMENT_CLASS(UListScriptReferencedContentCommandlet);


/*-----------------------------------------------------------------------------
UAnalyzeContent commandlet.
-----------------------------------------------------------------------------*/

/**
* Helper structure to hold level/ usage count information.
*/
struct FLevelResourceStat
{
	/** Level package.						*/
	UObject*	LevelPackage;
	/** Usage count in above level package.	*/
	INT			Count;

	/**
	* Constructor, initializing Count to 1 and setting package.
	*
	* @param InLevelPackage	Level package to use
	*/
	FLevelResourceStat( UObject* InLevelPackage )
		: LevelPackage( InLevelPackage )
		, Count( 1 )
	{}
};
/**
* Helper structure containing usage information for a single resource and multiple levels.
*/
struct FResourceStat
{
	/** Resource object.											*/
	UObject*					Resource;
	/** Total number this resource is used across all levels.		*/
	INT							TotalCount;
	/** Array of detailed per level resource usage breakdown.		*/
	TArray<FLevelResourceStat>	LevelResourceStats;

	/**
	* Constructor
	*
	* @param	InResource		Resource to use
	* @param	LevelPackage	Level package to use
	*/
	FResourceStat( UObject* InResource, UObject* LevelPackage )
		:	Resource( InResource )
		,	TotalCount( 1 )
	{
		// Create initial stat entry.
		LevelResourceStats.AddItem( FLevelResourceStat( LevelPackage ) );
	}

	/**
	* Increment usage count by one
	*
	* @param	LevelPackage	Level package using the resource.
	*/
	void IncrementUsage( UObject* LevelPackage )
	{
		// Iterate over all level resource stats to find existing entry.
		UBOOL bFoundExisting = FALSE;
		for( INT LevelIndex=0; LevelIndex<LevelResourceStats.Num(); LevelIndex++ )
		{
			FLevelResourceStat& LevelResourceStat = LevelResourceStats(LevelIndex);
			// We found a match.
			if( LevelResourceStat.LevelPackage == LevelPackage )
			{
				// Increase its count and exit loop.
				LevelResourceStat.Count++;
				bFoundExisting = TRUE;
				break;
			}
		}
		// No existing entry has been found, add new one.
		if( !bFoundExisting )
		{
			LevelResourceStats.AddItem( FLevelResourceStat( LevelPackage ) );
		}
		// Increase total count.
		TotalCount++;
	}
};
/** Compare function used by sort. Sorts in descending order. */
IMPLEMENT_COMPARE_CONSTREF( FResourceStat, UnPackageUtilities, { return B.TotalCount - A.TotalCount; } );

/**
* Class encapsulating stats functionality.
*/
class FResourceStatContainer
{
public:
	/**
	* Constructor
	*
	* @param	InDescription	Description used for dumping stats.
	*/
	FResourceStatContainer( const TCHAR* InDescription )
		:	Description( InDescription )
	{}

	/** 
	* Function called when a resource is encountered in a level to increment count.
	*
	* @param	Resource		Encountered resource.
	* @param	LevelPackage	Level package resource was encountered in.
	*/
	void EncounteredResource( UObject* Resource, UObject* LevelPackage )
	{
		FResourceStat* ResourceStatPtr = ResourceToStatMap.Find( Resource );
		// Resource has existing stat associated.
		if( ResourceStatPtr != NULL )
		{
			ResourceStatPtr->IncrementUsage( LevelPackage );
		}
		// Associate resource with new stat.
		else
		{
			FResourceStat ResourceStat( Resource, LevelPackage );
			ResourceToStatMap.Set( Resource, ResourceStat );
		}
	}

	/**
	* Dumps all the stats information sorted to the log.
	*/
	void DumpStats()
	{
		// Copy TMap data into TArray so it can be sorted.
		TArray<FResourceStat> SortedList;
		for( TMap<UObject*,FResourceStat>::TIterator It(ResourceToStatMap); It; ++It )
		{
			SortedList.AddItem( It.Value() );
		}
		// Sort the list in descending order by total count.
		Sort<USE_COMPARE_CONSTREF(FResourceStat,UnPackageUtilities)>( SortedList.GetTypedData(), SortedList.Num() );

		warnf( NAME_Log, TEXT("") ); 
		warnf( NAME_Log, TEXT("") ); 
		warnf( NAME_Log, TEXT("Stats for %s."), *Description ); 
		warnf( NAME_Log, TEXT("") ); 

		// Iterate over all entries and dump info.
		for( INT i=0; i<SortedList.Num(); i++ )
		{
			const FResourceStat& ResourceStat = SortedList(i);
			warnf( NAME_Log, TEXT("%4i use%s%4i level%s for%s   %s"), 
				ResourceStat.TotalCount,
				ResourceStat.TotalCount > 1 ? TEXT("s in") : TEXT(" in "), 
				ResourceStat.LevelResourceStats.Num(), 
				ResourceStat.LevelResourceStats.Num() > 1 ? TEXT("s") : TEXT(""),
				ResourceStat.LevelResourceStats.Num() > 1 ? TEXT("") : TEXT(" "),
				*ResourceStat.Resource->GetFullName() );

			for( INT LevelIndex=0; LevelIndex<ResourceStat.LevelResourceStats.Num(); LevelIndex++ )
			{
				const FLevelResourceStat& LevelResourceStat = ResourceStat.LevelResourceStats(LevelIndex);
				warnf( NAME_Log, TEXT("    %4i use%s: %s"), 
					LevelResourceStat.Count, 
					LevelResourceStat.Count > 1 ? TEXT("s in") : TEXT("  in"), 
					*LevelResourceStat.LevelPackage->GetName() );
			}
		}
	}
private:
	/** Map from resource to stat helper structure. */
	TMap<UObject*,FResourceStat>	ResourceToStatMap;
	/** Description used for dumping stats.			*/
	FString							Description;
};

void UAnalyzeContentCommandlet::StaticInitialize()
{
	ShowErrorCount = FALSE;
}

INT UAnalyzeContentCommandlet::Main( const FString& Params )
{
	// Parse command line.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	const TCHAR* Parms = *Params;
	ParseCommandLine(Parms, Tokens, Switches);

	// Retrieve all package file names and iterate over them, comparing them to tokens.
	TArray<FString> PackageFileList = GPackageFileCache->GetPackageFileList();		
	for( INT PackageIndex=0; PackageIndex<PackageFileList.Num(); PackageIndex++ )
	{
		// Tokens on the command line are package names.
		for( INT TokenIndex=0; TokenIndex<Tokens.Num(); TokenIndex++ )
		{
			// Compare the two and see whether we want to include this package in the analysis.
			FFilename PackageName = PackageFileList( PackageIndex );
			if( Tokens(TokenIndex) == PackageName.GetBaseFilename() )
			{
				UPackage* Package = UObject::LoadPackage( NULL, *PackageName, LOAD_None );
				if( Package != NULL )
				{
					warnf( NAME_Log, TEXT("Loading %s"), *PackageName );
					// Find the world and load all referenced levels.
					UWorld* World = FindObjectChecked<UWorld>( Package, TEXT("TheWorld") );
					if( World )
					{
						AWorldInfo* WorldInfo	= World->GetWorldInfo();
						// Iterate over streaming level objects loading the levels.
						for( INT LevelIndex=0; LevelIndex<WorldInfo->StreamingLevels.Num(); LevelIndex++ )
						{
							ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
							if( StreamingLevel )
							{
								// Load package if found.
								FString Filename;
								if( GPackageFileCache->FindPackageFile( *StreamingLevel->PackageName.ToString(), NULL, Filename ) )
								{
									warnf(NAME_Log, TEXT("Loading sub-level %s"), *Filename);
									LoadPackage( NULL, *Filename, LOAD_None );
								}
							}
						}
					}
				}
				else
				{
					warnf( NAME_Error, TEXT("Error loading %s!"), *PackageName );
				}
			}
		}
	}

	// By now all objects are in memory.

	FResourceStatContainer StaticMeshMaterialStats( TEXT("materials applied to static meshes") );
	FResourceStatContainer StaticMeshStats( TEXT("static meshes placed in levels") );
	FResourceStatContainer BSPMaterialStats( TEXT("materials applied to BSP surfaces") );

	// Iterate over all static mesh components and add their materials and static meshes.
	for( TObjectIterator<UStaticMeshComponent> It; It; ++It )
	{
		UStaticMeshComponent*	StaticMeshComponent = *It;
		UPackage*				LevelPackage		= StaticMeshComponent->GetOutermost();

		// Only add if the outer is a map package.
		if( LevelPackage->ContainsMap() )
		{
			if( StaticMeshComponent->StaticMesh && StaticMeshComponent->StaticMesh->LODModels.Num() )
			{
				// Populate materials array, avoiding duplicate entries.
				TArray<UMaterial*> Materials;
				INT MaterialCount = StaticMeshComponent->StaticMesh->LODModels(0).Elements.Num();
				for( INT MaterialIndex=0; MaterialIndex<MaterialCount; MaterialIndex++ )
				{
					UMaterialInterface* MaterialInterface = StaticMeshComponent->GetMaterial( MaterialIndex );
					if( MaterialInterface && MaterialInterface->GetMaterial() )
					{
						Materials.AddUniqueItem( MaterialInterface->GetMaterial(MSP_SM3) );
					}
				}

				// Iterate over materials and create/ update associated stats.
				for( INT MaterialIndex=0; MaterialIndex<Materials.Num(); MaterialIndex++ )
				{
					UMaterial* Material = Materials(MaterialIndex);
					// Track materials applied to static meshes.			
					StaticMeshMaterialStats.EncounteredResource( Material, LevelPackage );
				}
			}

			// Track static meshes used by static mesh components.
			if( StaticMeshComponent->StaticMesh )
			{
				StaticMeshStats.EncounteredResource( StaticMeshComponent->StaticMesh, LevelPackage );
			}
		}
	}

	for( TObjectIterator<ABrush> It; It; ++It )
	{
		ABrush*		BrushActor		= *It;
		UPackage*	LevelPackage	= BrushActor->GetOutermost();

		// Only add if the outer is a map package.
		if( LevelPackage->ContainsMap() )
		{
			if( BrushActor->Brush && BrushActor->Brush->Polys )
			{
				UPolys* Polys = BrushActor->Brush->Polys;

				// Populate materials array, avoiding duplicate entries.
				TArray<UMaterial*> Materials;
				for( INT ElementIndex=0; ElementIndex<Polys->Element.Num(); ElementIndex++ )
				{
					const FPoly& Poly = Polys->Element(ElementIndex);
					if( Poly.Material && Poly.Material->GetMaterial() )
					{
						Materials.AddUniqueItem( Poly.Material->GetMaterial(MSP_SM3) );
					}
				}

				// Iterate over materials and create/ update associated stats.
				for( INT MaterialIndex=0; MaterialIndex<Materials.Num(); MaterialIndex++ )
				{
					UMaterial* Material = Materials(MaterialIndex);
					// Track materials applied to BSP.
					BSPMaterialStats.EncounteredResource( Material, LevelPackage );
				}
			}
		}
	}

	// Dump stat summaries.
	StaticMeshMaterialStats.DumpStats();
	StaticMeshStats.DumpStats();
	BSPMaterialStats.DumpStats();

	return 0;
}
IMPLEMENT_CLASS(UAnalyzeContentCommandlet)


/*-----------------------------------------------------------------------------
UAnalyzeReferencedContentCommandlet
-----------------------------------------------------------------------------*/

/** Constructor, initializing all members. */
UAnalyzeReferencedContentCommandlet::FStaticMeshStats::FStaticMeshStats( UStaticMesh* StaticMesh )
:	ResourceType(StaticMesh->GetClass()->GetName())
,	ResourceName(StaticMesh->GetPathName())
,	NumInstances(0)
,	NumTriangles(0)
,	NumSections(0)
,   NumConvexPrimitives(0)
,   bUsesSimpleRigidBodyCollision(StaticMesh->UseSimpleRigidBodyCollision)
,   NumElementsWithCollision(0)
,	bIsReferencedByScript(FALSE)
,   bIsReferencedByParticles(FALSE)
,	ResourceSize(StaticMesh->GetResourceSize())
,   bIsMeshNonUniformlyScaled(FALSE)
,   bShouldConvertBoxColl(FALSE)
{
	// Update triangle and section counts.
	for( INT ElementIndex=0; ElementIndex<StaticMesh->LODModels(0).Elements.Num(); ElementIndex++ )
	{
		const FStaticMeshElement& StaticMeshElement = StaticMesh->LODModels(0).Elements(ElementIndex);
        NumElementsWithCollision += StaticMeshElement.EnableCollision ? 1 : 0;
		NumTriangles += StaticMeshElement.NumTriangles;
		NumSections++;
	}

	if(StaticMesh->BodySetup)
	{
		NumConvexPrimitives = StaticMesh->BodySetup->AggGeom.ConvexElems.Num();
	}
}



/**
* Returns a header row for CSV
*
* @return comma separated header row
*/
FString UAnalyzeReferencedContentCommandlet::FStaticMeshStats::GetCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,NumInstances,NumTriangles,NumSections,NumSectionsWithCollision,UsesSimpleRigidBodyCollision,ConvexCollisionPrims,NumMapsUsedIn,ResourceSize,ScalesUsed,NonUniformScale,ShouldConvertBoxColl,ParticleMesh,Instanced Triangles") LINE_TERMINATOR;

	// we would like to have this in the "Instanced Triangles" column but the commas make it auto parsed into the next column for the CSV
	//=Table1[[#This Row],[NumInstances]]*Table1[[#This Row],[NumTriangles]]
}


/**
* Stringifies gathered stats in CSV format.
*
* @return comma separated list of stats
*/
FString UAnalyzeReferencedContentCommandlet::FStaticMeshStats::ToCSV() const
{

	return FString::Printf(TEXT("%s,%s,%i,%i,%i,%i,%i,%i,%i,%i,%d,%i,%i,%i%s"),
		*ResourceType,
		*ResourceName,
		NumInstances,
		NumTriangles,
		NumSections,
        NumElementsWithCollision,
		bUsesSimpleRigidBodyCollision,
		NumConvexPrimitives,
		MapsUsedIn.Num(),
		ResourceSize,
		UsedAtScales.Num(),
		bIsMeshNonUniformlyScaled,
		bShouldConvertBoxColl,
        bIsReferencedByParticles,
		LINE_TERMINATOR);
}

/** This takes a LevelName and then looks for the number of Instances of this StatMesh used within that level **/
FString UAnalyzeReferencedContentCommandlet::FStaticMeshStats::ToCSV( const FString& LevelName ) const
{
	UINT* NumInst = const_cast<UINT*>(LevelNameToInstanceCount.Find( LevelName ));
	if( NumInst == NULL )
	{
		*NumInst = 0;
	}

	return FString::Printf(TEXT("%s,%s,%i,%i,%i,%i,%i,%i,%i,%i,%d,%i,%i,%i%s"),
		*ResourceType,
		*ResourceName,
		*NumInst,
		NumTriangles,
		NumSections,
		NumElementsWithCollision,
		bUsesSimpleRigidBodyCollision,
		NumConvexPrimitives,
		MapsUsedIn.Num(),
		ResourceSize,
		UsedAtScales.Num(),
		bIsMeshNonUniformlyScaled,
		bShouldConvertBoxColl,
		bIsReferencedByParticles,
		LINE_TERMINATOR);
}



//Initialization of static member in FLightingOptimizationInfo
const INT UAnalyzeReferencedContentCommandlet::FLightingOptimizationStats::LightMapSizes[NumLightmapTextureSizes] = { 256, 128, 64, 32 };
static const INT BytesUsedThreshold = 5000;

/**
*   Calculate the memory required to light a mesh with given NumVertices using vertex lighting
*/
INT UAnalyzeReferencedContentCommandlet::FLightingOptimizationStats::CalculateVertexLightingBytesUsed(INT NumVertices)
{
	//3 color channels are  (3 colors * 4 bytes/color = 12 bytes)
	//1 color channel is    (1 color * 4 bytes/color = 4 bytes)
	const INT VERTEX_LIGHTING_DATA_SIZE = GSystemSettings.bAllowDirectionalLightMaps ? sizeof(FQuantizedDirectionalLightSample) : sizeof(FQuantizedSimpleLightSample);

	//BytesUsed = (Number of Vertices) * sizeof(VertexLightingData)
	INT BytesUsed = NumVertices * VERTEX_LIGHTING_DATA_SIZE;

	return BytesUsed;
}

/** Assuming DXT1 lightmaps...
*   4 bits/pixel * width * height = Highest MIP Level * 1.333 MIP Factor for approx usage for a full mip chain
*   Either 1 or 3 textures if we're doing simple or directional (3-axis) lightmap
*   Most lightmaps require a second UV channel which is probably an extra 4 bytes (2 floats compressed to SHORT) 
*/
INT UAnalyzeReferencedContentCommandlet::FLightingOptimizationStats::CalculateLightmapLightingBytesUsed(INT Width, INT Height, INT NumVertices, INT UVChannelIndex)
{
	if (Width <= 0 || Height <= 0 || NumVertices <= 0)
	{
		return 0;
	}

	const FLOAT MIP_FACTOR = 4.0f / 3.0f;

	FLOAT BYTES_PER_PIXEL = 0.0f;
	if (GSystemSettings.bAllowDirectionalLightMaps)
	{
		//DXT1 4bits/pixel * 4/3 mipfactor * 3 channels = 16 bits = 2 bytes / pixel
		BYTES_PER_PIXEL = 0.5f * MIP_FACTOR * NUM_DIRECTIONAL_LIGHTMAP_COEF;
	}
	else
	{      
		//DXT1 4bits/pixel * 4/3 mipfactor * 1 channel = 16/3 bits = 0.6666 bytes / pixel
		BYTES_PER_PIXEL = 0.5f * MIP_FACTOR * NUM_SIMPLE_LIGHTMAP_COEF;
	}

	//BytesUsed = (SizeOfTexture) + (SizeOfUVData)
	//SizeOfTexture = (Width * Height * BytesPerTexel) * MIP_FACTOR
	//SizeOfUVData = (Number of Vertices * SizeOfUVCoordData)
	INT SizeOfTexture = Width * Height * BYTES_PER_PIXEL;

	INT BytesUsed = 0;
	if ( TRUE )
	{
		BytesUsed = SizeOfTexture;
	}
	else
	{
		//I'm told by Dan that most static meshes will probably already have a 2nd uv channel and it wouldn't necessarily go away
		//with the addition/subtraction of a light map, otherwise we can reenable this
		const FLOAT UV_COORD_DATA_SIZE = UVChannelIndex * 2 * sizeof(SHORT); //Index * (Tuple / UVChannel) * (2 bytes / Tuple)
		INT SizeOfUVData = NumVertices * UV_COORD_DATA_SIZE;
		BytesUsed = SizeOfTexture + SizeOfUVData;
	}

	return BytesUsed;
}

/** 
*	For a given list of parameters, compute a full spread of potential savings values using vertex light, or 256, 128, 64, 32 pixel square light maps
*  @param LMType - Current type of lighting being used
*  @param NumVertices - Number of vertices in the given mesh
*  @param Width - Width of current lightmap
*  @param Height - Height of current lightmap
*  @param TexCoordIndex - channel index of the uvs currently used for lightmaps
*  @param LOI - A struct to be filled in by the function with the potential savings
*/
void UAnalyzeReferencedContentCommandlet::FLightingOptimizationStats::CalculateLightingOptimizationInfo(ELightMapInteractionType LMType, INT NumVertices, INT Width, INT Height, INT TexCoordIndex, FLightingOptimizationStats& LOStats)
{
	// Current Values
	LOStats.IsType = LMType;
	LOStats.TextureSize = Width;

	if (LMType == LMIT_Vertex)
	{
		LOStats.CurrentBytesUsed = CalculateVertexLightingBytesUsed(NumVertices);
	}
	else if (LMType == LMIT_Texture)
	{
		LOStats.CurrentBytesUsed = CalculateLightmapLightingBytesUsed(Width, Height, NumVertices, TexCoordIndex);
	}

	//Potential savings values
	INT VertexLitBytesUsed = CalculateVertexLightingBytesUsed(NumVertices);

	INT TextureMapBytesUsed[FLightingOptimizationStats::NumLightmapTextureSizes];
	for (INT i=0; i<FLightingOptimizationStats::NumLightmapTextureSizes; i++)
	{
		const INT TexCoordIndexAssumed = 1; //assume it will require 2 texcoord channels to do the lightmap
		TextureMapBytesUsed[i] = CalculateLightmapLightingBytesUsed(LightMapSizes[i], LightMapSizes[i], NumVertices, TexCoordIndexAssumed);
	}

	//Store this all away in a nice little struct
	for (INT i=0; i<NumLightmapTextureSizes; i++)
	{
		LOStats.BytesSaved[i] = LOStats.CurrentBytesUsed - TextureMapBytesUsed[i];
	}

	LOStats.BytesSaved[NumLightmapTextureSizes] = LOStats.CurrentBytesUsed - VertexLitBytesUsed;
}

/** 
* Calculate the potential savings for a given static mesh component by using an alternate static lighting method
*/
UAnalyzeReferencedContentCommandlet::FLightingOptimizationStats::FLightingOptimizationStats(AStaticMeshActor* StaticMeshActor)   :
    LevelName(StaticMeshActor->GetOutermost()->GetName())
,	ActorName(StaticMeshActor->GetName())
,	SMName(StaticMeshActor->StaticMeshComponent->StaticMesh->GetName())
,   IsType(LMIT_None)        
,   TextureSize(0)  
,	CurrentBytesUsed(0)
{
	UStaticMeshComponent* StaticMeshComponent = StaticMeshActor->StaticMeshComponent;

	appMemzero(BytesSaved, ARRAYSIZE(BytesSaved));
	if (StaticMeshComponent && StaticMeshComponent->StaticMesh && StaticMeshComponent->HasStaticShadowing())
	{
		UStaticMesh* StaticMesh = StaticMeshComponent->StaticMesh;

		INT NumLODModels = StaticMesh->LODModels.Num();
		for (INT LODModelIndex = 0; LODModelIndex < NumLODModels; LODModelIndex++)
		{
			const FStaticMeshRenderData& StaticMeshRenderData = StaticMesh->LODModels(LODModelIndex);

			//Mesh has to have LOD data in order to even consider this calculation?
			if (LODModelIndex < StaticMeshComponent->LODData.Num())
			{
				const FStaticMeshComponentLODInfo& StaticMeshComponentLODInfo = StaticMeshComponent->LODData(LODModelIndex);

				//Again without a lightmap, don't bother?
				if (StaticMeshComponentLODInfo.LightMap != NULL)
				{
					//What is the mesh currently using
					FLightMapInteraction LMInteraction = StaticMeshComponentLODInfo.LightMap->GetInteraction();
					IsType = LMInteraction.GetType();

					INT Width  = 0;
					INT Height = 0;
					//Returns the correct (including overrides) width/height for the lightmap
					StaticMeshComponent->GetLightMapResolution(Width, Height);

					//Get the number of vertices used by this static mesh
					INT NumVertices = StaticMeshRenderData.NumVertices;

					//Get the number of uv coordinates stored in the vertex buffer
					INT TexCoordIndex = StaticMesh->LightMapCoordinateIndex; 

					CalculateLightingOptimizationInfo(IsType, NumVertices, Width, Height, TexCoordIndex, *this);
				}
			}
		} //for each lodmodel
	}
}

/**
* Stringifies gathered stats in CSV format.
*
* @return comma separated list of stats
*/
FString UAnalyzeReferencedContentCommandlet::FLightingOptimizationStats::ToCSV() const
{
	FString CSVString;
	if (CurrentBytesUsed > BytesUsedThreshold)
	{
		FString CurrentType(TEXT("Unknown"));
		if (IsType == LMIT_Vertex)
		{
			CurrentType = TEXT("Vertex");
		}
		else
		{
			CurrentType = FString::Printf(TEXT("%d-Texture"), TextureSize);
		}

		FString BytesSavedString;
		UBOOL FoundSavings = FALSE;
		for (INT i=0; i<NumLightmapTextureSizes + 1; i++)
		{
			if (BytesSaved[i] > 0)
			{
				BytesSavedString += FString::Printf(TEXT(",%1.3f"), (FLOAT)BytesSaved[i] / 1024.0f);
				FoundSavings = TRUE;
			}
			else
			{
				BytesSavedString += TEXT(",");
			}
		}

		if (FoundSavings)
		{
			CSVString = FString::Printf(TEXT("%s,%s,%s,%s,%1.3f"), *LevelName, *ActorName, *SMName, *CurrentType, (FLOAT)CurrentBytesUsed/1024.0f);
			CSVString += BytesSavedString;
			CSVString += LINE_TERMINATOR;
		}
	}

	return CSVString;
}

/**
* Returns a header row for CSV
*
* @return comma separated header row
*/
FString UAnalyzeReferencedContentCommandlet::FLightingOptimizationStats::GetCSVHeaderRow()
{
	FString CSVHeaderString = FString::Printf(TEXT("LevelName,StaticMeshActor,StaticMesh,CurrentLightingType,CurrentUsage(kb)"));
	for (INT i=0; i<NumLightmapTextureSizes; i++)
	{
		CSVHeaderString += FString::Printf(TEXT(",%d-Texture"), LightMapSizes[i]);
	}

	CSVHeaderString += FString::Printf(TEXT(",Vertex"));
	CSVHeaderString += LINE_TERMINATOR;
	return CSVHeaderString;
}

/** Constructor, initializing all members */
UAnalyzeReferencedContentCommandlet::FShadowMap1DStats::FShadowMap1DStats(UShadowMap1D* ShadowMap1D)
:	ResourceType(ShadowMap1D->GetClass()->GetName())
,	ResourceName(ShadowMap1D->GetPathName())
,	ResourceSize(0)
,	NumSamples(0)
,	UsedByLight(TEXT("None"))
{
	NumSamples = ShadowMap1D->NumSamples();
	FArchiveCountMem CountAr(ShadowMap1D);
	ResourceSize = CountAr.GetMax();

	// find the light for this shadow map	
	for( TObjectIterator<ULightComponent> It; It; ++It )
	{
		const ULightComponent* Light = *It;
		if( Light->LightGuid == ShadowMap1D->GetLightGuid() )
		{
			if( Light->GetOwner() )
			{
				UsedByLight = Light->GetOwner()->GetName();
			}
			break;
		}
	}
}

/**
* Stringifies gathered stats in CSV format.
*
* @return comma separated list of stats
*/
FString UAnalyzeReferencedContentCommandlet::FShadowMap1DStats::ToCSV() const
{
	return FString::Printf(TEXT("%s,%s,%i,%i,%s%s"),
		*ResourceType,
		*ResourceName,						
		ResourceSize,
		NumSamples,		
		*UsedByLight,
		LINE_TERMINATOR);
}

/**
* Returns a header row for CSV
*
* @return comma separated header row
*/
FString UAnalyzeReferencedContentCommandlet::FShadowMap1DStats::GetCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,ResourceSize,NumSamples,UsedByLight") LINE_TERMINATOR;
}

/** Constructor, initializing all members */
UAnalyzeReferencedContentCommandlet::FShadowMap2DStats::FShadowMap2DStats(UShadowMap2D* ShadowMap2D)
:	ResourceType(ShadowMap2D->GetClass()->GetName())
,	ResourceName(ShadowMap2D->GetPathName())
,	ShadowMapTexture2D(TEXT("None"))
,	ShadowMapTexture2DSizeX(0)
,	ShadowMapTexture2DSizeY(0)
,	ShadowMapTexture2DFormat(TEXT("None"))
,	UsedByLight(TEXT("None"))
{	
	if( ShadowMap2D->IsValid() )
	{
		UShadowMapTexture2D* ShadowTex2D = ShadowMap2D->GetTexture();
		ShadowMapTexture2DSizeX = ShadowTex2D->SizeX;
		ShadowMapTexture2DSizeY = ShadowTex2D->SizeY;
		ShadowMapTexture2DFormat = FString(GPixelFormats[ShadowTex2D->Format].Name ? GPixelFormats[ShadowTex2D->Format].Name : TEXT("None"));
		ShadowMapTexture2D = ShadowTex2D->GetPathName();
	}

	// find the light for this shadow map	
	for( TObjectIterator<ULightComponent> It; It; ++It )
	{
		const ULightComponent* Light = *It;
		if( Light->LightGuid == ShadowMap2D->GetLightGuid() )
		{
			if( Light->GetOwner() )
			{
				UsedByLight = Light->GetOwner()->GetName();
			}
			break;
		}
	}
}

/**
* Stringifies gathered stats in CSV format.
*
* @return comma separated list of stats
*/
FString UAnalyzeReferencedContentCommandlet::FShadowMap2DStats::ToCSV() const
{
	return FString::Printf(TEXT("%s,%s,%s,%i,%i,%s,%s%s"),
		*ResourceType,
		*ResourceName,						
		*ShadowMapTexture2D,		
		ShadowMapTexture2DSizeX,
		ShadowMapTexture2DSizeY,
		*ShadowMapTexture2DFormat,
		*UsedByLight,
		LINE_TERMINATOR);
}

/**
* Returns a header row for CSV
*
* @return comma separated header row
*/
FString UAnalyzeReferencedContentCommandlet::FShadowMap2DStats::GetCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,ShadowMapTexture2D,SizeX,SizeY,Format,UsedByLight") LINE_TERMINATOR;
}

/** Constructor, initializing all members */
UAnalyzeReferencedContentCommandlet::FTextureStats::FTextureStats( UTexture* Texture )
:	ResourceType(Texture->GetClass()->GetName())
,	ResourceName(Texture->GetPathName())
,	bIsReferencedByScript(FALSE)
,	ResourceSize(Texture->GetResourceSize())
,	LODBias(Texture->LODBias)
,	LODGroup(Texture->LODGroup)
,	Format(TEXT("UNKOWN"))
{
	// Update format.
	UTexture2D* Texture2D = Cast<UTexture2D>(Texture);
	if( Texture2D )
	{
		Format = GPixelFormats[Texture2D->Format].Name;
	}
}

/**
* Stringifies gathered stats in CSV format.
*
* @return comma separated list of stats
*/
FString UAnalyzeReferencedContentCommandlet::FTextureStats::ToCSV() const
{
	return FString::Printf(TEXT("%s,%s,%i,%i,%i,%i,%i,%i,%s%s"),
		*ResourceType,
		*ResourceName,						
		MaterialsUsedBy.Num(),
		bIsReferencedByScript,
		MapsUsedIn.Num(),
		ResourceSize,
		LODBias,
		LODGroup,
		*Format,
		LINE_TERMINATOR);
}

/**
* Returns a header row for CSV
*
* @return comma separated header row
*/
FString UAnalyzeReferencedContentCommandlet::FTextureStats::GetCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,NumMaterialsUsedBy,ScriptReferenced,NumMapsUsedIn,ResourceSize,LODBias,LODGroup,Format") LINE_TERMINATOR;
}

/**
* Static helper to return instructions used by shader type.
*
* @param	MeshShaderMap	Shader map to use to find shader of passed in type
* @param	ShaderType		Type of shader to query instruction count for
* @return	Instruction count if found, 0 otherwise
*/
static INT GetNumInstructionsForShaderType( const FMeshMaterialShaderMap* MeshShaderMap, FShaderType* ShaderType )
{
	INT NumInstructions = 0;
	const FShader* Shader = MeshShaderMap->GetShader(ShaderType);
	if( Shader )
	{
		NumInstructions = Shader->GetNumInstructions();
	}
	return NumInstructions;
}

/** Constructor, initializing all members */
UAnalyzeReferencedContentCommandlet::FMaterialStats::FMaterialStats( UMaterial* Material, UAnalyzeReferencedContentCommandlet* Commandlet )
:	ResourceType(Material->GetClass()->GetName())
,	ResourceName(Material->GetPathName())
,	NumBrushesAppliedTo(0)
,	NumStaticMeshInstancesAppliedTo(0)
,	bIsReferencedByScript(FALSE)
,	ResourceSizeOfReferencedTextures(0)
{
	// Keep track of unique textures and texture sample count.
	TArray<UTexture*> UniqueTextures;
	TArray<UTexture*> SampledTextures;
	Material->GetTextures( UniqueTextures );
	Material->GetTextures( SampledTextures, FALSE );

	// Update texture samplers count.
	NumTextureSamples = SampledTextures.Num();

	// Update dependency chain stats.
	check( Material->MaterialResources[MSP_SM3]);
	MaxTextureDependencyLength = Material->MaterialResources[MSP_SM3]->GetMaxTextureDependencyLength();

	// Update instruction counts.
	const FMaterialShaderMap* MaterialShaderMap = Material->MaterialResources[MSP_SM3]->GetShaderMap();
	if(MaterialShaderMap)
	{
		// Use the local vertex factory shaders.
		const FMeshMaterialShaderMap* MeshShaderMap = MaterialShaderMap->GetMeshShaderMap(&FLocalVertexFactory::StaticType);
		check(MeshShaderMap);

		// Get intruction counts.
		NumInstructionsBasePassNoLightmap		= GetNumInstructionsForShaderType( MeshShaderMap, Commandlet->ShaderTypeBasePassNoLightmap	);
		NumInstructionsBasePassAndLightmap		= GetNumInstructionsForShaderType( MeshShaderMap, Commandlet->ShaderTypeBasePassAndLightmap );
		NumInstructionsPointLightWithShadowMap	= GetNumInstructionsForShaderType( MeshShaderMap, Commandlet->ShaderTypePointLightWithShadowMap );
	}

	// Iterate over unique texture refs and update resource size.
	for( INT TextureIndex=0; TextureIndex<UniqueTextures.Num(); TextureIndex++ )
	{
		UTexture* Texture = UniqueTextures(TextureIndex);
		if (Texture != NULL)
		{
			ResourceSizeOfReferencedTextures += Texture->GetResourceSize();
			TexturesUsed.AddItem( Texture->GetFullName() );
		}
		else
		{
			warnf(TEXT("Material %s has a NULL texture reference..."), *(Material->GetFullName()));
		}
	}
}

/**
* Stringifies gathered stats in CSV format.
*
* @return comma separated list of stats
*/
FString UAnalyzeReferencedContentCommandlet::FMaterialStats::ToCSV() const
{
	return FString::Printf(TEXT("%s,%s,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i%s"),
		*ResourceType,
		*ResourceName,
		NumBrushesAppliedTo,
		NumStaticMeshInstancesAppliedTo,
		StaticMeshesAppliedTo.Num(),
		bIsReferencedByScript,
		MapsUsedIn.Num(),
		TexturesUsed.Num(),
		NumTextureSamples,
		MaxTextureDependencyLength,
		NumInstructionsBasePassNoLightmap,
		NumInstructionsBasePassAndLightmap,
		NumInstructionsPointLightWithShadowMap,
		ResourceSizeOfReferencedTextures,
		LINE_TERMINATOR);
}

/**
* Returns a header row for CSV
*
* @return comma separated header row
*/
FString UAnalyzeReferencedContentCommandlet::FMaterialStats::GetCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,NumBrushesAppliedTo,NumStaticMeshInstancesAppliedTo,NumStaticMeshesAppliedTo,ScriptReferenced,NumMapsUsedIn,NumTextures,NumTextureSamples,MaxTextureDependencyLength,BasePass,BasePassLightmap,PointLightShadowMap,ResourceSizeOfReferencedTextures") LINE_TERMINATOR;
}

/** Constructor, initializing all members */
UAnalyzeReferencedContentCommandlet::FParticleStats::FParticleStats( UParticleSystem* ParticleSystem )
:	ResourceType(ParticleSystem->GetClass()->GetName())
,	ResourceName(ParticleSystem->GetPathName())
,	bIsReferencedByScript(FALSE)
,	NumEmitters(0)
,	NumModules(0)
,	NumPeakActiveParticles(0)
,   NumEmittersUsingCollision(0)
,   NumEmittersUsingPhysics(0)
,   MaxNumDrawnPerFrame(0)
,   PeakActiveToMaxDrawnRatio(0.0f)
,   NumBytesUsed(0)
,	bUsesDistortionMaterial(FALSE)
,	bUsesSceneTextureMaterial(FALSE)
,   bMeshEmitterHasDoCollisions(FALSE)
,   bMeshEmitterHasCastShadows(FALSE)
,   WarmUpTime( 0.0f )
,	bHasPhysXEmitters(FALSE)
{

	if( ParticleSystem->WarmupTime > 0.0f )
	{
		WarmUpTime = ParticleSystem->WarmupTime;
	}

	// Iterate over all sub- emitters and update stats.
	for( INT EmitterIndex=0; EmitterIndex<ParticleSystem->Emitters.Num(); EmitterIndex++ )
	{
		UParticleEmitter* ParticleEmitter = ParticleSystem->Emitters(EmitterIndex);
		if( ParticleEmitter )
		{
			if (ParticleEmitter->LODLevels.Num() > 0)
			{
				NumEmitters++;
				UParticleLODLevel* HighLODLevel = ParticleEmitter->LODLevels(0);
				check(HighLODLevel);
				NumModules += HighLODLevel->Modules.Num();
				
				for (INT LODIndex = 0; LODIndex < ParticleEmitter->LODLevels.Num(); LODIndex++)
				{
					UParticleLODLevel* LODLevel = ParticleEmitter->LODLevels(LODIndex);
					if (LODLevel)
					{
						if (LODLevel->bEnabled)
						{
							// Get peak active particles from LOD 0.
							if (LODIndex == 0)
							{
								INT PeakParticles = 0;
								if (ParticleEmitter->InitialAllocationCount > 0)
								{
									//If this value is non-zero it was overridden by user in the editor
									PeakParticles = ParticleEmitter->InitialAllocationCount;
								}
								else
								{
									//Peak number of particles simulated
									PeakParticles = LODLevel->PeakActiveParticles;
								}

								NumPeakActiveParticles += PeakParticles;                     
								
								if (LODLevel->RequiredModule && LODLevel->RequiredModule->bUseMaxDrawCount)
								{
									//Maximum number of particles allowed to draw per frame by this emitter
									MaxNumDrawnPerFrame += LODLevel->RequiredModule->MaxDrawCount;
								}
								else
								{
									//Make the "max drawn" equal to the number of particles simulated
									MaxNumDrawnPerFrame += PeakParticles;
								}
							}

							// flag distortion and scene color usage of materials
							if( LODLevel->RequiredModule && 
								LODLevel->RequiredModule->Material &&
								LODLevel->RequiredModule->Material->GetMaterial() )
							{
								if( LODLevel->RequiredModule->Material->GetMaterial()->HasDistortion() )
								{
									bUsesDistortionMaterial = TRUE;
									DistortMaterialNames.AddUniqueItem(LODLevel->RequiredModule->Material->GetPathName());
								}
								if( LODLevel->RequiredModule->Material->GetMaterial()->UsesSceneColor() )
								{
									bUsesSceneTextureMaterial = TRUE;
									SceneColorMaterialNames.AddUniqueItem(LODLevel->RequiredModule->Material->GetPathName());
								}
							}

							if( LODLevel->TypeDataModule)
							{
								UParticleModuleTypeDataMesh* MeshType = Cast<UParticleModuleTypeDataMesh>(LODLevel->TypeDataModule);
								UParticleModuleTypeDataPhysX* PhysXType = Cast<UParticleModuleTypeDataPhysX>(LODLevel->TypeDataModule);
								UParticleModuleTypeDataMeshPhysX* PhysXMeshType = Cast<UParticleModuleTypeDataMeshPhysX>(LODLevel->TypeDataModule);
								if (PhysXType || PhysXMeshType)
								{
									bHasPhysXEmitters = TRUE;
								}

								if (MeshType)
								{
									if( MeshType->DoCollisions == TRUE )
									{
										bMeshEmitterHasDoCollisions = TRUE;
									}

									if( MeshType->CastShadows == TRUE )
									{
										bMeshEmitterHasCastShadows = TRUE;
									}

									if( MeshType->Mesh )
									{
										for( INT MeshLODIdx=0; MeshLODIdx < MeshType->Mesh->LODInfo.Num(); MeshLODIdx++ )
										{
											const FStaticMeshLODInfo& MeshLOD = MeshType->Mesh->LODInfo(MeshLODIdx);
											for( INT ElementIdx=0; ElementIdx < MeshLOD.Elements.Num(); ElementIdx++ )
											{
												// flag distortion and scene color usage of materials
												const FStaticMeshLODElement& MeshElement = MeshLOD.Elements(ElementIdx);
												if( MeshElement.Material &&
													MeshElement.Material->GetMaterial() )
												{
													if( MeshElement.Material->GetMaterial()->HasDistortion() )
													{
														bUsesDistortionMaterial = TRUE;
														DistortMaterialNames.AddUniqueItem(MeshElement.Material->GetPathName());
													}
													if( MeshElement.Material->GetMaterial()->UsesSceneColor() )
													{
														bUsesSceneTextureMaterial = TRUE;
														SceneColorMaterialNames.AddUniqueItem(MeshElement.Material->GetPathName());
													}
												}
											}
										}
									}
								}
							}

							for (INT ModuleIndex = 0; ModuleIndex < LODLevel->Modules.Num(); ModuleIndex++)
							{
								UParticleModuleCollision* CollisionModule = Cast<UParticleModuleCollision>(LODLevel->Modules(ModuleIndex));                                      
								if (CollisionModule && CollisionModule->bEnabled)
								{
									NumEmittersUsingCollision++;
									if (CollisionModule->bApplyPhysics == TRUE)
									{
										NumEmittersUsingPhysics++;
									}
								}

								UParticleModuleMeshMaterial* MeshModule = Cast<UParticleModuleMeshMaterial>(LODLevel->Modules(ModuleIndex));
								if( MeshModule )
								{
									for( INT MatIdx=0; MatIdx < MeshModule->MeshMaterials.Num(); MatIdx++ )
									{
										// flag distortion and scene color usage of materials
										UMaterialInterface* Mat = MeshModule->MeshMaterials(MatIdx);									
										if( Mat && Mat->GetMaterial() )
										{
											if( Mat->GetMaterial()->HasDistortion() )
											{
												bUsesDistortionMaterial = TRUE;
												DistortMaterialNames.AddUniqueItem(Mat->GetPathName());
											}
											if( Mat->GetMaterial()->UsesSceneColor() )
											{
												bUsesSceneTextureMaterial = TRUE;
												SceneColorMaterialNames.AddUniqueItem(Mat->GetPathName());
											}
										}
									}
								}
								UParticleModuleMaterialByParameter* MaterialByModule = Cast<UParticleModuleMaterialByParameter>(LODLevel->Modules(ModuleIndex));
								if( MaterialByModule )
								{
									for( INT MatIdx=0; MatIdx < MaterialByModule->DefaultMaterials.Num(); MatIdx++ )
									{
										// flag distortion and scene color usage of materials
										UMaterialInterface* Mat = MaterialByModule->DefaultMaterials(MatIdx);
										if( Mat && Mat->GetMaterial() )
										{
											if( Mat->GetMaterial()->HasDistortion() )
											{
												bUsesDistortionMaterial = TRUE;
												DistortMaterialNames.AddUniqueItem(Mat->GetPathName());
											}
											if( Mat->GetMaterial()->UsesSceneColor() )
											{
												bUsesSceneTextureMaterial = TRUE; 
												SceneColorMaterialNames.AddUniqueItem(Mat->GetPathName());
											}
										}
									}
								}
							}
						}
						else
						{
							if (LODLevel->TypeDataModule)
							{
								UParticleModuleTypeDataPhysX* PhysXType = Cast<UParticleModuleTypeDataPhysX>(LODLevel->TypeDataModule);
								UParticleModuleTypeDataMeshPhysX* PhysXMeshType = Cast<UParticleModuleTypeDataMeshPhysX>(LODLevel->TypeDataModule);
								if (PhysXType || PhysXMeshType)
								{
									bHasPhysXEmitters = TRUE;
								}
							}
						}
					}
				} //for each lod
			}
		}
	} //for each emitter

    // @todo add this into the .xls
	//ParticleSystem->CalculateMaxActiveParticleCounts();

	//A number greater than 1 here indicates more particles simulated than drawn each frame
	if (MaxNumDrawnPerFrame > 0)
	{
		PeakActiveToMaxDrawnRatio = (FLOAT)NumPeakActiveParticles / (FLOAT)MaxNumDrawnPerFrame;
	}


	// determine the number of bytes this ParticleSystem uses
	FArchiveCountMem Count( ParticleSystem );
	NumBytesUsed = Count.GetNum();

	// Determine the number of bytes a PSysComp would use w/ this PSys as the template...
	if (ParticleSystem->IsTemplate() == FALSE)
	{
		UParticleSystemComponent* PSysComp = ConstructObject<UParticleSystemComponent>(UParticleSystemComponent::StaticClass());
		if (PSysComp)
		{
			PSysComp->SetTemplate(ParticleSystem);
			PSysComp->ActivateSystem(TRUE);

			FArchiveCountMem CountPSysComp(PSysComp);
			NumBytesUsed += CountPSysComp.GetNum();

			PSysComp->DeactivateSystem();
		}
	}
}

/** @return TRUE if this asset type should be logged */
UBOOL UAnalyzeReferencedContentCommandlet::FParticleStats::ShouldLogStat() const
{
	return TRUE;
	//return bUsesDistortionMaterial || bUsesSceneTextureMaterial;
}

/**
* Stringifies gathered stats in CSV format.
*
* @return comma separated list of stats
*/
FString UAnalyzeReferencedContentCommandlet::FParticleStats::ToCSV() const
{
	FString MatNames;
	for( INT MatIdx=0; MatIdx < SceneColorMaterialNames.Num(); MatIdx++ )
	{
		MatNames += FString(TEXT("(scenecolor)")) + SceneColorMaterialNames(MatIdx); 
		MatNames += FString(TEXT(","));
	}
	for( INT MatIdx=0; MatIdx < DistortMaterialNames.Num(); MatIdx++ )
	{
		MatNames += FString(TEXT("(distort)")) + DistortMaterialNames(MatIdx);
		MatNames += FString(TEXT(","));
	}

	return FString::Printf(TEXT("%s,%s,%i,%i,%i,%i,%i,%i,%1.2f,%i,%i,%i,%s,%s,%s,%s,%1.2f,%s,%s,%s"),
		*ResourceType,
		*ResourceName,	
 		bIsReferencedByScript,
 		MapsUsedIn.Num(),
 		NumEmitters,
 		NumModules,
 		NumPeakActiveParticles,
		MaxNumDrawnPerFrame,
        PeakActiveToMaxDrawnRatio,
		NumEmittersUsingCollision,
		NumEmittersUsingPhysics,
		NumBytesUsed,
 		bUsesDistortionMaterial ? TEXT("TRUE") : TEXT("FALSE"),
 		bUsesSceneTextureMaterial ? TEXT("TRUE") : TEXT("FALSE"),
 		bMeshEmitterHasDoCollisions ? TEXT("TRUE") : TEXT("FALSE"),
 		bMeshEmitterHasCastShadows ? TEXT("TRUE") : TEXT("FALSE"),
		WarmUpTime,
		bHasPhysXEmitters ? TEXT("TRUE") : TEXT("FALSE"),
 		*MatNames,
		LINE_TERMINATOR);
}

/**
 * Returns a header row for CSV
 *
 * @return comma separated header row
 */
FString UAnalyzeReferencedContentCommandlet::FParticleStats::GetCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,ScriptReferenced,NumMapsUsedIn,NumEmitters,NumModules,NumPeakActiveParticles,MaxParticlesDrawnPerFrame,PeakToMaxDrawnRatio,NumEmittersUsingCollision,NumEmittersUsingPhysics,NumBytesUsed,bUsesDistortion,bUsesSceneColor,bMeshEmitterHasDoCollisions,bMeshEmitterHasCastShadows,WarmUpTime,bHasPhysXEmitters") LINE_TERMINATOR;
}

//
//	FTextureToParticleSystemStats
//
UAnalyzeReferencedContentCommandlet::FTextureToParticleSystemStats::FTextureToParticleSystemStats(UTexture* InTexture) :
	  TextureName(InTexture->GetPathName())
{
	UTexture2D* Texture2D = Cast<UTexture2D>(InTexture);
	if (Texture2D)
	{
		TextureSize = FString::Printf(TEXT("%d x %d"), (INT)(Texture2D->GetSurfaceWidth()), (INT)(Texture2D->GetSurfaceHeight()));
		Format = GPixelFormats[Texture2D->Format].Name;
	}
	else
	{
		TextureSize = FString::Printf(TEXT("???"));
		Format = TextureSize;
	}
	appMemzero(&ParticleSystemsContainedIn, sizeof(TArray<UParticleSystem*>));
}
	
void UAnalyzeReferencedContentCommandlet::FTextureToParticleSystemStats::AddParticleSystem(UParticleSystem* InParticleSystem)
{
	ParticleSystemsContainedIn.AddUniqueItem(InParticleSystem->GetPathName());
}

/**
 * Stringifies gathered stats in CSV format.
 *
 * @return comma separated list of stats
 */
FString UAnalyzeReferencedContentCommandlet::FTextureToParticleSystemStats::ToCSV() const
{
	FString Row = FString::Printf(TEXT("%s,%s,%s,%i,%s"),
		*TextureName,
		*TextureSize, 
		*Format,
		ParticleSystemsContainedIn.Num(),
		LINE_TERMINATOR);

	// this will print out the specific particles systems that use this texture
	for (INT PSysIndex = 0; PSysIndex < GetParticleSystemsContainedInCount(); PSysIndex++)
	{
		const FString OutputText = FString::Printf(TEXT(",,,,%s,%s"), *(GetParticleSystemContainedIn(PSysIndex)), LINE_TERMINATOR);
		Row += OutputText;
	}

	return Row;
}

/**
 * Returns a header row for CSV
 *
 * @return comma separated header row
 */
FString UAnalyzeReferencedContentCommandlet::FTextureToParticleSystemStats::GetCSVHeaderRow()
{
	return TEXT("ResourceName,Size,Format,NumParticleSystemsUsedIn") LINE_TERMINATOR;
}

/** Constructor, initializing all members */
UAnalyzeReferencedContentCommandlet::FAnimSequenceStats::FAnimSequenceStats( UAnimSequence* Sequence )
:	ResourceType(Sequence->GetClass()->GetName())
,	ResourceName(Sequence->GetPathName())
,	bIsReferencedByScript(FALSE)
,	CompressionScheme(FString(TEXT("")))
,	TranslationFormat(ACF_None)
,	RotationFormat(ACF_None)
,	AnimationSize(0)
,	TotalTracks(0)
,	NumTransTracksWithOneKey(0)
,	NumRotTracksWithOneKey(0)
,	TrackTableSize(0)
,	TotalNumTransKeys(0)
,	TotalNumRotKeys(0)
,	TranslationKeySize(0)
,	RotationKeySize(0)
,	TotalFrames(0)
{
	// The sequence object name is not very useful - strip and add the friendly name.
	FString Left, Right;
	ResourceName.Split(TEXT("."), &Left, &Right, TRUE);
	ResourceName = Left + TEXT(".") + Sequence->SequenceName.ToString();

	if(Sequence->CompressionScheme)
	{
		CompressionScheme = Sequence->CompressionScheme->GetClass()->GetName();
	}

	TranslationFormat = static_cast<AnimationCompressionFormat>(Sequence->TranslationCompressionFormat);
	RotationFormat = static_cast<AnimationCompressionFormat>(Sequence->RotationCompressionFormat);
	AnimationSize = Sequence->GetResourceSize();

	INT NumRotTracks = 0;

	AnimationFormat_GetStats(	
		Sequence, 
		TotalTracks,
		NumRotTracks,
		TotalNumTransKeys,
		TotalNumRotKeys,
		TranslationKeySize,
		RotationKeySize,
		NumTransTracksWithOneKey,
		NumRotTracksWithOneKey);

	TrackTableSize = sizeof(INT)*Sequence->CompressedTrackOffsets.Num();
	TotalFrames = Sequence->NumFrames;
}


static FString GetCompressionFormatString(AnimationCompressionFormat InFormat)
{
	switch(InFormat)
	{
	case ACF_None:
		return FString(TEXT("ACF_None"));
	case ACF_Float96NoW:
		return FString(TEXT("ACF_Float96NoW"));
	case ACF_Fixed48NoW:
		return FString(TEXT("ACF_Fixed48NoW"));
	case ACF_IntervalFixed32NoW:
		return FString(TEXT("ACF_IntervalFixed32NoW"));
	case ACF_Fixed32NoW:
		return FString(TEXT("ACF_Fixed32NoW"));
	case ACF_Float32NoW:
		return FString(TEXT("ACF_Float32NoW"));
	default:
		warnf( TEXT("AnimationCompressionFormat was not found:  %i"), static_cast<INT>(InFormat) );
	}

	return FString(TEXT("Unknown"));
}

static FString GetAnimationKeyFormatString(AnimationKeyFormat InFormat)
{
	switch(InFormat)
	{
	case AKF_ConstantKeyLerp:
		return FString(TEXT("AKF_ConstantKeyLerp"));
	case AKF_VariableKeyLerp:
		return FString(TEXT("AKF_VariableKeyLerp"));
	default:
		warnf( TEXT("AnimationKeyFormat was not found:  %i"), static_cast<INT>(InFormat) );
	}

	return FString(TEXT("Unknown"));
}

/**
* Stringifies gathered stats in CSV format.
*
* @return comma separated list of stats
*/
FString UAnalyzeReferencedContentCommandlet::FAnimSequenceStats::ToCSV() const
{
	return FString::Printf(TEXT("%s,%s,%i,%i,%s,%s,%s,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i%s"),
		*ResourceType,
		*ResourceName,	
		bIsReferencedByScript,
		MapsUsedIn.Num(),
		*GetCompressionFormatString(TranslationFormat),
		*GetCompressionFormatString(RotationFormat),
		*CompressionScheme,
		AnimationSize,
		TrackTableSize,
		TotalTracks,
		NumTransTracksWithOneKey,
		NumRotTracksWithOneKey,
		TotalNumTransKeys,
		TotalNumRotKeys,
		TranslationKeySize,
		RotationKeySize,
		TotalFrames,
		LINE_TERMINATOR);
}

/**
* Returns a header row for CSV
*
* @return comma separated header row
*/
FString UAnalyzeReferencedContentCommandlet::FAnimSequenceStats::GetCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,ScriptReferenced,NumMapsUsedIn,TransFormat,RotFormat,CompressionScheme,AnimSize,TrackTableSize,Tracks,TracksWithoutTrans,TracksWithoutRot,TransKeys,RotKeys,TransKeySize,RotKeySize,TotalFrames") LINE_TERMINATOR;
}

/** Constructor, initializing all members. */
UAnalyzeReferencedContentCommandlet::FSoundCueStats::FSoundCueStats( USoundCue* SoundCue )
:	ResourceType(SoundCue->GetClass()->GetName())
,	ResourceName(SoundCue->GetPathName())
,	bIsReferencedByScript(FALSE)
,	FaceFXAnimName(SoundCue->FaceFXAnimName)
,	FaceFXGroupName(SoundCue->FaceFXGroupName)
,	ResourceSize(SoundCue->GetResourceSize())
{
}

/**
 * Stringifies gathered stats in CSV format.
 *
 * @return comma separated list of stats
 */
FString UAnalyzeReferencedContentCommandlet::FSoundCueStats::ToCSV() const
{
	return FString::Printf(TEXT("%s,%s,%i,%i,%i,%s,%s%s"),
		*ResourceType,
		*ResourceName,	
		bIsReferencedByScript,
		MapsUsedIn.Num(),
		ResourceSize,
		*FaceFXAnimName,
		*FaceFXGroupName,
		LINE_TERMINATOR);
}

/** This takes a LevelName and then looks for the number of Instances of this StatMesh used within that level **/
FString UAnalyzeReferencedContentCommandlet::FSoundCueStats::ToCSV( const FString& LevelName ) const
{
	return FString::Printf(TEXT("%s,%s,%i,%i,%i,%s,%s%s"),
		*ResourceType,
		*ResourceName,	
		bIsReferencedByScript,
		MapsUsedIn.Num(),
		ResourceSize,
		*FaceFXAnimName,
		*FaceFXGroupName,
		LINE_TERMINATOR);
}

/**
 * Returns a header row for CSV
 *
 * @return comma separated header row
 */
FString UAnalyzeReferencedContentCommandlet::FSoundCueStats::GetCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,ScriptReferenced,NumMapsUsedIn,NumBytesUsed,FaceFXAnimName,FaceFXGroupName") LINE_TERMINATOR;
}

/**
* Retrieves/ creates material stats associated with passed in material.
*
* @warning: returns pointer into TMap, only valid till next time Set is called
*
* @param	Material	Material to retrieve/ create material stats for
* @return	pointer to material stats associated with material
*/
UAnalyzeReferencedContentCommandlet::FMaterialStats* UAnalyzeReferencedContentCommandlet::GetMaterialStats( UMaterial* Material )
{
	UAnalyzeReferencedContentCommandlet::FMaterialStats* MaterialStats = ResourceNameToMaterialStats.Find( Material->GetFullName() );
	if( MaterialStats == NULL )
	{
		MaterialStats =	&ResourceNameToMaterialStats.Set( *Material->GetFullName(), UAnalyzeReferencedContentCommandlet::FMaterialStats( Material, this ) );
	}
	return MaterialStats;
}

/**
* Retrieves/ creates texture stats associated with passed in texture.
*
* @warning: returns pointer into TMap, only valid till next time Set is called
*
* @param	Texture		Texture to retrieve/ create texture stats for
* @return	pointer to texture stats associated with texture
*/
UAnalyzeReferencedContentCommandlet::FTextureStats* UAnalyzeReferencedContentCommandlet::GetTextureStats( UTexture* Texture )
{
	UAnalyzeReferencedContentCommandlet::FTextureStats* TextureStats = ResourceNameToTextureStats.Find( Texture->GetFullName() );
	if( TextureStats == NULL )
	{
		TextureStats = &ResourceNameToTextureStats.Set( *Texture->GetFullName(), UAnalyzeReferencedContentCommandlet::FTextureStats( Texture ) );
	}
	return TextureStats;
}

/**
* Retrieves/ creates static mesh stats associated with passed in static mesh.
*
* @warning: returns pointer into TMap, only valid till next time Set is called
*
* @param	StaticMesh	Static mesh to retrieve/ create static mesh stats for
* @return	pointer to static mesh stats associated with static mesh
*/
UAnalyzeReferencedContentCommandlet::FStaticMeshStats* UAnalyzeReferencedContentCommandlet::GetStaticMeshStats( UStaticMesh* StaticMesh, UPackage* LevelPackage )
{
	UAnalyzeReferencedContentCommandlet::FStaticMeshStats* StaticMeshStats = ResourceNameToStaticMeshStats.Find( StaticMesh->GetFullName() );

	if( StaticMeshStats == NULL )
	{
		StaticMeshStats = &ResourceNameToStaticMeshStats.Set( *StaticMesh->GetFullName(), UAnalyzeReferencedContentCommandlet::FStaticMeshStats( StaticMesh ) );
	}

	return StaticMeshStats;
}

/**
* Retrieves/ creates particle stats associated with passed in particle system.
*
* @warning: returns pointer into TMap, only valid till next time Set is called
*
* @param	ParticleSystem	Particle system to retrieve/ create static mesh stats for
* @return	pointer to particle system stats associated with static mesh
*/
UAnalyzeReferencedContentCommandlet::FParticleStats* UAnalyzeReferencedContentCommandlet::GetParticleStats( UParticleSystem* ParticleSystem )
{
	UAnalyzeReferencedContentCommandlet::FParticleStats* ParticleStats = ResourceNameToParticleStats.Find( ParticleSystem->GetFullName() );
	if( ParticleStats == NULL )
	{
		ParticleStats = &ResourceNameToParticleStats.Set( *ParticleSystem->GetFullName(), UAnalyzeReferencedContentCommandlet::FParticleStats( ParticleSystem ) );
	}
	return ParticleStats;
}

/**
 * Retrieves/creates texture in particle system stats associated with the passed in texture.
 *
 * @warning: returns pointer into TMap, only valid till next time Set is called
 *
 * @param	InTexture	The texture to retrieve/create stats for
 * @return	pointer to textureinparticlesystem stats
 */
UAnalyzeReferencedContentCommandlet::FTextureToParticleSystemStats* UAnalyzeReferencedContentCommandlet::GetTextureToParticleSystemStats(UTexture* InTexture)
{
	UAnalyzeReferencedContentCommandlet::FTextureToParticleSystemStats* TxtrToPSysStats = ResourceNameToTextureToParticleSystemStats.Find(InTexture->GetPathName());
	if (TxtrToPSysStats == NULL)
	{
		TxtrToPSysStats = &ResourceNameToTextureToParticleSystemStats.Set(*InTexture->GetPathName(), 
			UAnalyzeReferencedContentCommandlet::FTextureToParticleSystemStats(InTexture));
	}

	return TxtrToPSysStats;
}


/**
* Retrieves/ creates animation sequence stats associated with passed in animation sequence.
*
* @warning: returns pointer into TMap, only valid till next time Set is called
*
* @param	AnimSequence	Anim sequence to retrieve/ create anim sequence stats for
* @return	pointer to particle system stats associated with anim sequence
*/
UAnalyzeReferencedContentCommandlet::FAnimSequenceStats* UAnalyzeReferencedContentCommandlet::GetAnimSequenceStats( UAnimSequence* AnimSequence )
{
	UAnalyzeReferencedContentCommandlet::FAnimSequenceStats* AnimStats = ResourceNameToAnimStats.Find( AnimSequence->GetFullName() );
	if( AnimStats == NULL )
	{
		AnimStats = &ResourceNameToAnimStats.Set( *AnimSequence->GetFullName(), UAnalyzeReferencedContentCommandlet::FAnimSequenceStats( AnimSequence ) );
	}
	return AnimStats;
}

/**
* Retrieves/ creates lighting optimization stats associated with passed in static mesh actor.
*
* @warning: returns pointer into TMap, only valid till next time Set is called
*
* @param	ActorComponent	Actor component to calculate potential light map savings stats for
* @return	pointer to lighting optimization stats associated with this actor component
*/
UAnalyzeReferencedContentCommandlet::FLightingOptimizationStats* UAnalyzeReferencedContentCommandlet::GetLightingOptimizationStats( AStaticMeshActor* StaticMeshActor )
{
	UAnalyzeReferencedContentCommandlet::FLightingOptimizationStats* LightingStats = ResourceNameToLightingStats.Find( StaticMeshActor->GetFullName() );
	if( LightingStats == NULL )
	{
		LightingStats = &ResourceNameToLightingStats.Set( *StaticMeshActor->GetFullName(), UAnalyzeReferencedContentCommandlet::FLightingOptimizationStats( StaticMeshActor ) );
	}
	return LightingStats;
}

/**
 * Retrieves/ creates sound cue stats associated with passed in sound cue.
 *
 * @warning: returns pointer into TMap, only valid till next time Set is called
 *
 * @param	SoundCue	Sound cue  to retrieve/ create sound cue  stats for
 * @return				pointer to sound cue  stats associated with sound cue  
 */
UAnalyzeReferencedContentCommandlet::FSoundCueStats* UAnalyzeReferencedContentCommandlet::GetSoundCueStats( USoundCue* SoundCue, UPackage* LevelPackage )
{
	UAnalyzeReferencedContentCommandlet::FSoundCueStats* SoundCueStats = ResourceNameToSoundCueStats.Find( SoundCue->GetFullName() );

	if( SoundCueStats == NULL )
	{
		SoundCueStats = &ResourceNameToSoundCueStats.Set( *SoundCue->GetFullName(), UAnalyzeReferencedContentCommandlet::FSoundCueStats( SoundCue ) );
	}

	return SoundCueStats;
}

/**
* Retrieves/ creates shadowmap 1D stats associated with passed in shadowmap 1D object.
*
* @warning: returns pointer into TMap, only valid till next time Set is called
*
* @param	SoundCue	Sound cue  to retrieve/ create sound cue  stats for
* @return				pointer to sound cue  stats associated with sound cue  
*/
UAnalyzeReferencedContentCommandlet::FShadowMap1DStats* UAnalyzeReferencedContentCommandlet::GetShadowMap1DStats( UShadowMap1D* ShadowMap1D, UPackage* LevelPackage )
{
	UAnalyzeReferencedContentCommandlet::FShadowMap1DStats* ShadowMap1DStats = ResourceNameToShadowMap1DStats.Find( ShadowMap1D->GetFullName() );

	if( ShadowMap1DStats == NULL )
	{
		ShadowMap1DStats = &ResourceNameToShadowMap1DStats.Set( *ShadowMap1D->GetFullName(), UAnalyzeReferencedContentCommandlet::FShadowMap1DStats( ShadowMap1D ) );
	}

	return ShadowMap1DStats;
}

/**
* Retrieves/ creates shadowmap 2D stats associated with passed in shadowmap 2D object.
*
* @warning: returns pointer into TMap, only valid till next time Set is called
*
* @param	SoundCue	Sound cue  to retrieve/ create sound cue  stats for
* @return				pointer to sound cue  stats associated with sound cue  
*/
UAnalyzeReferencedContentCommandlet::FShadowMap2DStats* UAnalyzeReferencedContentCommandlet::GetShadowMap2DStats( UShadowMap2D* ShadowMap2D, UPackage* LevelPackage )
{
	UAnalyzeReferencedContentCommandlet::FShadowMap2DStats* ShadowMap2DStats = ResourceNameToShadowMap2DStats.Find( ShadowMap2D->GetFullName() );

	if( ShadowMap2DStats == NULL )
	{
		ShadowMap2DStats = &ResourceNameToShadowMap2DStats.Set( *ShadowMap2D->GetFullName(), UAnalyzeReferencedContentCommandlet::FShadowMap2DStats( ShadowMap2D ) );
	}

	return ShadowMap2DStats;
}

void UAnalyzeReferencedContentCommandlet::StaticInitialize()
{
	ShowErrorCount = FALSE;
}

/**
* Handles encountered object, routing to various sub handlers.
*
* @param	Object			Object to handle
* @param	LevelPackage	Currently loaded level package, can be NULL if not a level
* @param	bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleObject( UObject* Object, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	// Disregard marked objects as they won't go away with GC.
	if( !Object->HasAnyFlags( RF_Marked ) )
	{
		// Whether the object is the passed in level package if it is != NULL.
		const UBOOL bIsInALevelPackage = LevelPackage && Object->IsIn( LevelPackage );

		if( Object->IsA(UParticleSystemComponent::StaticClass()) && bIsInALevelPackage && ((IgnoreObjects & IGNORE_Particle) == 0))
		{
			HandleStaticMeshOnAParticleSystemComponent( (UParticleSystemComponent*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handle static mesh.
		else if( Object->IsA(UStaticMesh::StaticClass()) && ((IgnoreObjects & IGNORE_StaticMesh) == 0))
		{
			HandleStaticMesh( (UStaticMesh*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handles static mesh component if it's residing in the map package. LevelPackage == NULL for non map packages.
		else if( Object->IsA(UStaticMeshComponent::StaticClass()) && bIsInALevelPackage && ((IgnoreObjects & IGNORE_StaticMeshComponent) == 0))
		{
			HandleStaticMeshComponent( (UStaticMeshComponent*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handles static mesh component if it's residing in the map package. LevelPackage == NULL for non map packages.
		else if( Object->IsA(AStaticMeshActor::StaticClass()) && bIsInALevelPackage && ((IgnoreObjects & IGNORE_StaticMeshActor) == 0))
		{
			HandleStaticMeshActor( (AStaticMeshActor*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handle material.
		else if( Object->IsA(UMaterial::StaticClass()) && ((IgnoreObjects & IGNORE_Material) == 0))
		{
			HandleMaterial( (UMaterial*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handle texture.
		else if( Object->IsA(UTexture::StaticClass()) && ((IgnoreObjects & IGNORE_Texture) == 0))
		{
			HandleTexture( (UTexture*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handles brush actor if it's residing in the map package. LevelPackage == NULL for non map packages.
		else if( Object->IsA(ABrush::StaticClass()) && bIsInALevelPackage && ((IgnoreObjects & IGNORE_Brush) == 0))
		{
			HandleBrush( (ABrush*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handle particle system.
		else if( Object->IsA(UParticleSystem::StaticClass()) && ((IgnoreObjects & IGNORE_Particle) == 0))
		{
			HandleParticleSystem( (UParticleSystem*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handle anim sequence.
		else if( Object->IsA(UAnimSequence::StaticClass()) && ((IgnoreObjects & IGNORE_AnimSequence) == 0))
		{
			HandleAnimSequence( (UAnimSequence*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handle level
		else if( Object->IsA(ULevel::StaticClass()) && ((IgnoreObjects & IGNORE_Level) == 0))
		{
			HandleLevel( (ULevel*)Object, LevelPackage, bIsScriptReferenced );
		}
		// Handle sound cue
		else if (Object->IsA(USoundCue::StaticClass()) && ((IgnoreObjects & IGNORE_SoundCue) == 0))
		{
			HandleSoundCue((USoundCue*)Object, LevelPackage, bIsScriptReferenced);
		}
		// Handle 1D shadow maps
		else if (Object->IsA(UShadowMap1D::StaticClass()) && ((IgnoreObjects & IGNORE_ShadowMap) == 0))
		{
			HandleShadowMap1D((UShadowMap1D*)Object,LevelPackage,bIsScriptReferenced);			
		}
		// Handle 2D shadow maps
		else if (Object->IsA(UShadowMap2D::StaticClass()) && ((IgnoreObjects & IGNORE_ShadowMap) == 0))
		{
			HandleShadowMap2D((UShadowMap2D*)Object,LevelPackage,bIsScriptReferenced);
		}
	}
}

/**
* Handles gathering stats for passed in static mesh.
*
* @param StaticMesh	StaticMesh to gather stats for.
* @param LevelPackage	Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleStaticMesh( UStaticMesh* StaticMesh, UPackage* LevelPackage, UBOOL bIsScriptReferenced  )
{
	UAnalyzeReferencedContentCommandlet::FStaticMeshStats* StaticMeshStats = GetStaticMeshStats( StaticMesh, LevelPackage );

	if (appStristr(*StaticMesh->GetName(), TEXT("destruct")) != NULL)
	{
		debugf(TEXT("HandleStaticMesh MeshName:%s"), *StaticMesh->GetFullName());
	}

	if( LevelPackage )
 	{
		StaticMeshStats->MapsUsedIn.AddUniqueItem( LevelPackage->GetFullName() );
 	}

	if( bIsScriptReferenced )
	{
		StaticMeshStats->bIsReferencedByScript = TRUE;
	}

	// Populate materials array, avoiding duplicate entries.
	TArray<UMaterial*> Materials;
	// @todo need to do foreach over all LODModels
	INT MaterialCount = StaticMesh->LODModels(0).Elements.Num();
	for( INT MaterialIndex=0; MaterialIndex<MaterialCount; MaterialIndex++ )
	{
		UMaterialInterface* MaterialInterface = StaticMesh->LODModels(0).Elements(MaterialIndex).Material;
		if( MaterialInterface && MaterialInterface->GetMaterial(MSP_SM3) )
		{
			Materials.AddUniqueItem( MaterialInterface->GetMaterial(MSP_SM3) );
		}
	}

	// Iterate over materials and create/ update associated stats.
	for( INT MaterialIndex=0; MaterialIndex<Materials.Num(); MaterialIndex++ )
	{
		UMaterial* Material	= Materials(MaterialIndex);	
		UAnalyzeReferencedContentCommandlet::FMaterialStats* MaterialStats = GetMaterialStats( Material );
		MaterialStats->StaticMeshesAppliedTo.Set( *StaticMesh->GetFullName(), TRUE );
	}
}

/**
* Handles gathering stats for passed in static actor component.
*
* @param StaticMeshActor	StaticMeshActor to gather stats for
* @param LevelPackage		Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleStaticMeshActor( AStaticMeshActor* StaticMeshActor, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	UStaticMeshComponent* StaticMeshComponent = StaticMeshActor->StaticMeshComponent;
	if( StaticMeshComponent && StaticMeshComponent->StaticMesh && StaticMeshComponent->StaticMesh->LODModels.Num() && StaticMeshComponent->HasStaticShadowing() )
	{
		// Track lighting optimization values for a given static mesh actor.
		UAnalyzeReferencedContentCommandlet::FLightingOptimizationStats* LightingStats = GetLightingOptimizationStats( StaticMeshActor );

		UAnalyzeReferencedContentCommandlet::FStaticMeshStats* StaticMeshStats = GetStaticMeshStats( StaticMeshComponent->StaticMesh, LevelPackage );

		if( LevelPackage != NULL )
		{
			StaticMeshStats->MapsUsedIn.AddUniqueItem( LevelPackage->GetFullName() );

			UINT* NumInst = StaticMeshStats->LevelNameToInstanceCount.Find( LevelPackage->GetOutermost()->GetName() );
			//warnf( TEXT( "looking for! %s %s found: %d" ), *LevelPackage->GetOutermost()->GetName(), *StaticMeshComponent->StaticMesh->GetFullName(), NumInst ? *NumInst : -1 );
			if( NumInst != NULL )
			{
				//warnf( TEXT( "ahhh! %s" ), *LevelPackage->GetOutermost()->GetName());
				StaticMeshStats->LevelNameToInstanceCount.Set( LevelPackage->GetOutermost()->GetName(), ++(*NumInst) );
			}
			else
			{
				StaticMeshStats->LevelNameToInstanceCount.Set( LevelPackage->GetOutermost()->GetName(), 1 );
			}
		}
	}
}

/**
 * Handles special case for stats for passed in static mesh component who is part of a ParticleSystemComponent
 *
 * @param ParticleSystemComponent	ParticleSystemComponent to gather stats for
 * @param LevelPackage	Currently loaded level package, can be NULL if not a level
 * @param bIsScriptReferenced Whether object is handled because there is a script reference
 */
void UAnalyzeReferencedContentCommandlet::HandleStaticMeshOnAParticleSystemComponent( UParticleSystemComponent* ParticleSystemComponent, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	UParticleSystemComponent* PSC = ParticleSystemComponent;
	//warnf( TEXT("%d"), PSC->SMComponents.Num() );

	UStaticMeshComponent* StaticMeshComponent = NULL;
	TArray<UStaticMesh*> ReferencedStaticMeshes;
	for( INT i = 0; i < PSC->SMComponents.Num(); ++i )
	{
		StaticMeshComponent = PSC->SMComponents(i);
        if (StaticMeshComponent && StaticMeshComponent->StaticMesh)
        {
            ReferencedStaticMeshes.AddUniqueItem( StaticMeshComponent->StaticMesh );
        }
    }

	UStaticMesh* StaticMesh = NULL;
	for( INT i = 0; i < ReferencedStaticMeshes.Num(); ++i )
	{
		//warnf( TEXT("%s"), *ReferencedStaticMeshes(i)->GetFullName() );
        StaticMesh = ReferencedStaticMeshes(i);
		HandleStaticMesh( StaticMesh, LevelPackage, bIsScriptReferenced );

		UAnalyzeReferencedContentCommandlet::FStaticMeshStats* StaticMeshStats = GetStaticMeshStats( StaticMesh, LevelPackage );
		StaticMeshStats->NumInstances++;
        
        // Mark object as being referenced by particles.
        StaticMeshStats->bIsReferencedByParticles = TRUE;
	}
}


/**
* Handles gathering stats for passed in material.
*
* @param Material	Material to gather stats for
* @param LevelPackage	Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleMaterial( UMaterial* Material, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	UAnalyzeReferencedContentCommandlet::FMaterialStats* MaterialStats = GetMaterialStats( Material );	

	if( LevelPackage )
	{
		MaterialStats->MapsUsedIn.AddUniqueItem( LevelPackage->GetFullName() );
	}

	if( bIsScriptReferenced )
	{
		MaterialStats->bIsReferencedByScript = TRUE;
	}

	// Array of textures used by this material. No duplicates.
	TArray<UTexture*> TexturesUsed;
	Material->GetTextures(TexturesUsed);

	// Update textures used by this material.
	for( INT TextureIndex=0; TextureIndex<TexturesUsed.Num(); TextureIndex++ )
	{
		UTexture* Texture = TexturesUsed(TextureIndex);
		if (Texture != NULL)
		{
			UAnalyzeReferencedContentCommandlet::FTextureStats* TextureStats = GetTextureStats(Texture);
			TextureStats->MaterialsUsedBy.Set( *Material->GetFullName(), TRUE );
		}
	}
}

/**
* Handles gathering stats for passed in texture.
*
* @paramTexture	Texture to gather stats for
* @param LevelPackage	Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleTexture( UTexture* Texture, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	UAnalyzeReferencedContentCommandlet::FTextureStats* TextureStats = GetTextureStats( Texture );

	// Only handle further if we have a level package.
	if( LevelPackage )
	{
		TextureStats->MapsUsedIn.AddUniqueItem( LevelPackage->GetFullName() );
	}

	// Mark as being referenced by script.
	if( bIsScriptReferenced )
	{
		TextureStats->bIsReferencedByScript = TRUE;
	}
}

/**
* Handles gathering stats for passed in brush.
*
* @param BrushActor Brush actor to gather stats for
* @param LevelPackage	Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleBrush( ABrush* BrushActor, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	if( BrushActor->Brush && BrushActor->Brush->Polys )
	{
		UPolys* Polys = BrushActor->Brush->Polys;

		// Populate materials array, avoiding duplicate entries.
		TArray<UMaterial*> Materials;
		for( INT ElementIndex=0; ElementIndex<Polys->Element.Num(); ElementIndex++ )
		{
			const FPoly& Poly = Polys->Element(ElementIndex);
			if( Poly.Material && Poly.Material->GetMaterial(MSP_SM3) )
			{
				Materials.AddUniqueItem( Poly.Material->GetMaterial(MSP_SM3) );
			}
		}

		// Iterate over materials and create/ update associated stats.
		for( INT MaterialIndex=0; MaterialIndex<Materials.Num(); MaterialIndex++ )
		{
			UMaterial* Material = Materials(MaterialIndex);
			UAnalyzeReferencedContentCommandlet::FMaterialStats* MaterialStats = GetMaterialStats( Material );
			MaterialStats->NumBrushesAppliedTo++;
		}
	}
}

/**
* Handles gathering stats for passed in particle system.
*
* @param ParticleSystem	Particle system to gather stats for
* @param LevelPackage		Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleParticleSystem( UParticleSystem* ParticleSystem, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	UAnalyzeReferencedContentCommandlet::FParticleStats* ParticleStats = GetParticleStats( ParticleSystem );

	// Only handle further if we have a level package.
	if( LevelPackage )
	{
		ParticleStats->MapsUsedIn.AddUniqueItem( LevelPackage->GetFullName() );
	}

	// Mark object as being referenced by script.
	if( bIsScriptReferenced )
	{
		ParticleStats->bIsReferencedByScript = TRUE;
	}

	// Loop over the textures used in the particle system...
	for (INT EmitterIndex = 0; EmitterIndex < ParticleSystem->Emitters.Num(); EmitterIndex++)
	{
		UParticleSpriteEmitter* Emitter = Cast<UParticleSpriteEmitter>(ParticleSystem->Emitters(EmitterIndex));
		if (Emitter)
		{
			for (INT LODIndex = 0; LODIndex < Emitter->LODLevels.Num(); LODIndex++)
			{
				UParticleLODLevel* LODLevel = Emitter->LODLevels(LODIndex);
				check(LODLevel);
				check(LODLevel->RequiredModule);

				TArray<UTexture*> OutTextures;

				// First, check the sprite material
				UMaterialInterface* MatIntf = LODLevel->RequiredModule->Material;
				if (MatIntf)
				{
					OutTextures.Empty();
					MatIntf->GetTextures(OutTextures);
					for (INT TextureIndex = 0; TextureIndex < OutTextures.Num(); TextureIndex++)
					{
						UTexture* Texture = OutTextures(TextureIndex);
						Texture->ConditionalPostLoad();
						UAnalyzeReferencedContentCommandlet::FTextureToParticleSystemStats* TxtrToPSysStats = GetTextureToParticleSystemStats(Texture);
						if (TxtrToPSysStats)
						{
							TxtrToPSysStats->AddParticleSystem(ParticleSystem);
						}
					}
				}

				// Check if it is a mesh emitter...
				if (LODIndex == 0)
				{
					if (LODLevel->TypeDataModule)
					{
						UParticleModuleTypeDataMesh* MeshTD = Cast<UParticleModuleTypeDataMesh>(LODLevel->TypeDataModule);
						if (MeshTD)
						{
							if (MeshTD->bOverrideMaterial == FALSE)
							{
								// Grab the materials on the mesh...
								if (MeshTD->Mesh)
								{
									for (INT LODInfoIndex = 0; LODInfoIndex < MeshTD->Mesh->LODInfo.Num(); LODInfoIndex++)
									{
										FStaticMeshLODInfo& LODInfo = MeshTD->Mesh->LODInfo(LODInfoIndex);
										for (INT ElementIndex = 0; ElementIndex < LODInfo.Elements.Num(); ElementIndex++)
										{
											FStaticMeshLODElement& Element = LODInfo.Elements(ElementIndex);
											MatIntf = Element.Material;
											if (MatIntf)
											{
												OutTextures.Empty();
												MatIntf->GetTextures(OutTextures);
												for (INT TextureIndex = 0; TextureIndex < OutTextures.Num(); TextureIndex++)
												{
													UTexture* Texture = OutTextures(TextureIndex);

													UAnalyzeReferencedContentCommandlet::FTextureToParticleSystemStats* TxtrToPSysStats = 
														GetTextureToParticleSystemStats(Texture);
													if (TxtrToPSysStats)
													{
														TxtrToPSysStats->AddParticleSystem(ParticleSystem);
													}
												}

											}
										}
									}
								}
							}
						}
					}
				}

				// Check for a MeshMaterial override module...
			}
		}
	}
}

/**
* Handles gathering stats for passed in animation sequence.
*
* @param AnimSequence		AnimSequence to gather stats for
* @param LevelPackage		Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleAnimSequence( UAnimSequence* AnimSequence, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	UAnalyzeReferencedContentCommandlet::FAnimSequenceStats* AnimStats = GetAnimSequenceStats( AnimSequence );

	// Only handle further if we have a level package.
	if( LevelPackage )
	{
		AnimStats->MapsUsedIn.AddUniqueItem( LevelPackage->GetFullName() );
	}

	// Mark object as being referenced by script.
	if( bIsScriptReferenced )
	{
		AnimStats->bIsReferencedByScript = TRUE;
	}
}

void UAnalyzeReferencedContentCommandlet::HandleLevel( ULevel* Level, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	for ( TMultiMap<UStaticMesh*, FCachedPhysSMData>::TIterator MeshIt(Level->CachedPhysSMDataMap); MeshIt; ++MeshIt )
	{
		UStaticMesh* Mesh = MeshIt.Key();
		FVector Scale3D = MeshIt.Value().Scale3D;

		if(Mesh)
		{
			UAnalyzeReferencedContentCommandlet::FStaticMeshStats* StaticMeshStats = GetStaticMeshStats( Mesh, LevelPackage );

			if (appStristr(*Mesh->GetName(), TEXT("destruct")) != NULL)
			{
				debugf(TEXT("HandleLevel MeshName:%s Scale: [%f %f %f]"), *Mesh->GetFullName(), Scale3D.X, Scale3D.Y, Scale3D.Z);
			}
			UBOOL bHaveScale = FALSE;
			for (INT i=0; i < StaticMeshStats->UsedAtScales.Num(); i++)
			{
				// Found a shape with the right scale
				if ((StaticMeshStats->UsedAtScales(i) - Scale3D).IsNearlyZero())
				{
					bHaveScale = TRUE;
					break;
				}
			}

			if(!bHaveScale)
			{
				if (!Scale3D.IsUniform())
				{
					StaticMeshStats->bIsMeshNonUniformlyScaled = TRUE;
					//Any non uniform scaling of this mesh with box collision will result in no collision
					if (Mesh->BodySetup	&& Mesh->BodySetup->AggGeom.BoxElems.Num() > 0)
					{
						StaticMeshStats->bShouldConvertBoxColl = TRUE;
					}
				}

				StaticMeshStats->UsedAtScales.AddItem(Scale3D);
			}
		}
	}
}

/**
* Handles gathering stats for passed in static mesh component.
*
* @param StaticMeshComponent	StaticMeshComponent to gather stats for
* @param LevelPackage	Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleStaticMeshComponent( UStaticMeshComponent* StaticMeshComponent, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	if( StaticMeshComponent->StaticMesh && StaticMeshComponent->StaticMesh->LODModels.Num() )
	{
		// Populate materials array, avoiding duplicate entries.
		TArray<UMaterial*> Materials;
		INT MaterialCount = StaticMeshComponent->StaticMesh->LODModels(0).Elements.Num();
		for( INT MaterialIndex=0; MaterialIndex<MaterialCount; MaterialIndex++ )
		{
			UMaterialInterface* MaterialInterface = StaticMeshComponent->GetMaterial( MaterialIndex );
			if( MaterialInterface && MaterialInterface->GetMaterial(MSP_SM3) )
			{
				Materials.AddUniqueItem( MaterialInterface->GetMaterial(MSP_SM3) );
			}
		}

		// Iterate over materials and create/ update associated stats.
		for( INT MaterialIndex=0; MaterialIndex<Materials.Num(); MaterialIndex++ )
		{
			UMaterial* Material	= Materials(MaterialIndex);	
			UAnalyzeReferencedContentCommandlet::FMaterialStats* MaterialStats = GetMaterialStats( Material );
			MaterialStats->NumStaticMeshInstancesAppliedTo++;
		}

		// Track static meshes used by static mesh components.
		if( StaticMeshComponent->StaticMesh )
		{
			const UBOOL bBelongsToAParticleSystemComponent = StaticMeshComponent->GetOuter()->IsA(UParticleSystemComponent::StaticClass());

			if( bBelongsToAParticleSystemComponent == FALSE )
			{
				UAnalyzeReferencedContentCommandlet::FStaticMeshStats* StaticMeshStats = GetStaticMeshStats( StaticMeshComponent->StaticMesh, LevelPackage );
				StaticMeshStats->NumInstances++;

				//warnf( TEXT("HandleStaticMeshComponent SMC: %s   Outer: %s  %d"), *StaticMeshComponent->StaticMesh->GetFullName(), *StaticMeshComponent->GetOuter()->GetFullName(), StaticMeshStats->NumInstances );
			}	
		}
	}
}

/**
 * Handles gathering stats for passed in sound cue.
 *
 * @param SoundCue				SoundCue to gather stats for.
 * @param LevelPackage			Currently loaded level package, can be NULL if not a level
 * @param bIsScriptReferenced	Whether object is handled because there is a script reference
 */
void UAnalyzeReferencedContentCommandlet::HandleSoundCue( USoundCue* SoundCue, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	UAnalyzeReferencedContentCommandlet::FSoundCueStats* SoundCueStats = GetSoundCueStats( SoundCue, LevelPackage );
	// Only handle further if we have a level package.
	if( LevelPackage )
	{
		SoundCueStats->MapsUsedIn.AddUniqueItem( LevelPackage->GetFullName() );

		UINT* NumInst = SoundCueStats->LevelNameToInstanceCount.Find( LevelPackage->GetOutermost()->GetName() );
		if( NumInst != NULL )
		{
			//warnf( TEXT( "ahhh! %s" ), *LevelPackage->GetOutermost()->GetName());
			SoundCueStats->LevelNameToInstanceCount.Set( LevelPackage->GetOutermost()->GetName(), ++(*NumInst) );
		}
		else
		{
			SoundCueStats->LevelNameToInstanceCount.Set( LevelPackage->GetOutermost()->GetName(), 1 );
		}
	}

	// Mark as being referenced by script.
	if( bIsScriptReferenced )
	{
		SoundCueStats->bIsReferencedByScript = TRUE;
	}
}

/**
* Handles gathering stats for passed in shadow map 1D.
*
* @param SoundCue				SoundCue to gather stats for.
* @param LevelPackage			Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced	Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleShadowMap1D( UShadowMap1D* ShadowMap1D, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	UAnalyzeReferencedContentCommandlet::FShadowMap1DStats* ShadowMap1DStats = GetShadowMap1DStats( ShadowMap1D, LevelPackage );
	// Only handle further if we have a level package.
	if( LevelPackage )
	{
		ShadowMap1DStats->MapsUsedIn.AddUniqueItem( LevelPackage->GetFullName() );

		UINT* NumInst = ShadowMap1DStats->LevelNameToInstanceCount.Find( LevelPackage->GetOutermost()->GetName() );
		if( NumInst != NULL )
		{
			ShadowMap1DStats->LevelNameToInstanceCount.Set( LevelPackage->GetOutermost()->GetName(), ++(*NumInst) );
		}
		else
		{
			ShadowMap1DStats->LevelNameToInstanceCount.Set( LevelPackage->GetOutermost()->GetName(), 1 );
		}
	}
}

/**
* Handles gathering stats for passed in shadow map 2D.
*
* @param SoundCue				SoundCue to gather stats for.
* @param LevelPackage			Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced	Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleShadowMap2D( UShadowMap2D* ShadowMap2D, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	UAnalyzeReferencedContentCommandlet::FShadowMap2DStats* ShadowMap2DStats = GetShadowMap2DStats( ShadowMap2D, LevelPackage );
	// Only handle further if we have a level package.
	if( LevelPackage )
	{
		ShadowMap2DStats->MapsUsedIn.AddUniqueItem( LevelPackage->GetFullName() );

		UINT* NumInst = ShadowMap2DStats->LevelNameToInstanceCount.Find( LevelPackage->GetOutermost()->GetName() );
		if( NumInst != NULL )
		{
			ShadowMap2DStats->LevelNameToInstanceCount.Set( LevelPackage->GetOutermost()->GetName(), ++(*NumInst) );
		}
		else
		{
			ShadowMap2DStats->LevelNameToInstanceCount.Set( LevelPackage->GetOutermost()->GetName(), 1 );
		}
	}
}

/** This will write out the specified Stats to the AssetStatsCSVs dir **/
template< typename STAT_TYPE >
void WriteOutCSVs( TMap<FString,STAT_TYPE> StatsData, const FString& StatsName, const FString& CurrTime )
{
	// Re-used helper variables for writing to CSV file.
	const FString CSVDirectory = appGameLogDir() + TEXT("AssetStatsCSVs") + PATH_SEPARATOR + FString::Printf( TEXT("%s-%d-%s"), GGameName, GetChangeListNumberForPerfTesting(), *CurrTime ) + PATH_SEPARATOR;
	FString		CSVFilename		= TEXT("");
	FArchive*	CSVFile			= NULL;


	// Create CSV folder in case it doesn't exist yet.
	GFileManager->MakeDirectory( *CSVDirectory );


	// CSV: Human-readable spreadsheet format.
	CSVFilename	= FString::Printf(TEXT("%s%s-%s-%i.csv"), *CSVDirectory, *StatsName, GGameName, GetChangeListNumberForPerfTesting() );
	CSVFile	= GFileManager->CreateFileWriter( *CSVFilename );
	if( CSVFile != NULL )
	{	
		// Write out header row.
		const FString& HeaderRow = STAT_TYPE::GetCSVHeaderRow();
		CSVFile->Serialize( TCHAR_TO_ANSI( *HeaderRow ), HeaderRow.Len() );

		// Write out each individual stats row.
		for( TMap<FString,STAT_TYPE>::TIterator It(StatsData); It; ++ It )
		{
			const STAT_TYPE& StatsEntry = It.Value();
			const FString& Row = StatsEntry.ToCSV();
			if( Row.Len() > 0 &&
				StatsEntry.ShouldLogStat() )
			{
				CSVFile->Serialize( TCHAR_TO_ANSI( *Row ), Row.Len() );
			}
		}

		// Close and delete archive.
		CSVFile->Close();
		delete CSVFile;
	}
	else
	{
		debugf(NAME_Warning,TEXT("Could not create CSV file %s for writing."), *CSVFilename);
	}
}


INT UAnalyzeReferencedContentCommandlet::Main( const FString& Params )
{
	// Parse command line.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	const TCHAR* Parms = *Params;
	ParseCommandLine(Parms, Tokens, Switches);

	// Whether to only deal with map files.
	const UBOOL bShouldOnlyLoadMaps	= Switches.FindItemIndex(TEXT("MAPSONLY")) != INDEX_NONE;
	// Whether to exclude script references.
	const UBOOL bExcludeScript		= Switches.FindItemIndex(TEXT("EXCLUDESCRIPT")) != INDEX_NONE;
    // Whether to load non native script packages (e.g. useful for seeing what will always be loaded)
	const UBOOL bExcludeNonNativeScript = Switches.FindItemIndex(TEXT("EXCLUDENONNATIVESCRIPT")) != INDEX_NONE;
	// Whether to automatically load all the sublevels from the world
	const UBOOL bAutoLoadSublevels = Switches.FindItemIndex(TEXT("LOADSUBLEVELS")) != INDEX_NONE;

	IgnoreObjects = 0;
	if (Switches.FindItemIndex(TEXT("IGNORESTATICMESH")) != INDEX_NONE)			IgnoreObjects |= IGNORE_StaticMesh;
	if (Switches.FindItemIndex(TEXT("IGNORESMC")) != INDEX_NONE)				IgnoreObjects |= IGNORE_StaticMeshComponent;
	if (Switches.FindItemIndex(TEXT("IGNORESTATICMESHACTOR")) != INDEX_NONE)	IgnoreObjects |= IGNORE_StaticMeshActor;
	if (Switches.FindItemIndex(TEXT("IGNORETEXTURE")) != INDEX_NONE)			IgnoreObjects |= IGNORE_Texture;
	if (Switches.FindItemIndex(TEXT("IGNOREMATERIAL")) != INDEX_NONE)			IgnoreObjects |= IGNORE_Material;
	if (Switches.FindItemIndex(TEXT("IGNOREPARTICLE")) != INDEX_NONE)			IgnoreObjects |= IGNORE_Particle;
	if (Switches.FindItemIndex(TEXT("IGNOREANIMS")) != INDEX_NONE)				IgnoreObjects |= IGNORE_AnimSequence;
	if (Switches.FindItemIndex(TEXT("IGNORESOUNDCUE")) != INDEX_NONE)			IgnoreObjects |= IGNORE_SoundCue;
	if (Switches.FindItemIndex(TEXT("IGNOREBRUSH")) != INDEX_NONE)				IgnoreObjects |= IGNORE_Brush;
	if (Switches.FindItemIndex(TEXT("IGNORELEVEL")) != INDEX_NONE)				IgnoreObjects |= IGNORE_Level;
	if (Switches.FindItemIndex(TEXT("IGNORESHADOWMAP")) != INDEX_NONE)			IgnoreObjects |= IGNORE_ShadowMap;

	if( bExcludeNonNativeScript == FALSE )
	{
		// Load up all script files in EditPackages.
		const UEditorEngine* EditorEngine = CastChecked<UEditorEngine>(GEngine);
		for( INT i=0; i<EditorEngine->EditPackages.Num(); i++ )
		{
			LoadPackage( NULL, *EditorEngine->EditPackages(i), LOAD_NoWarn );
		}
	}

	// Mark loaded objects as they are part of the always loaded set and are not taken into account for stats.
	for( TObjectIterator<UObject> It; It; ++It )
	{
		UObject* Object = *It;
		// Script referenced asset.
		if( !bExcludeScript )
		{
			HandleObject( Object, NULL, TRUE );
		}
		// Mark object as always loaded so it doesn't get counted multiple times.
		Object->SetFlags( RF_Marked );
	}

	TArray<FString> FileList;
	
	// Build package file list from passed in command line if tokens are specified.
	if( Tokens.Num() )
	{
		for( INT TokenIndex=0; TokenIndex<Tokens.Num(); TokenIndex++ )
		{
			// Lookup token in file cache and add filename if found.
			FString OutFilename;
			if( GPackageFileCache->FindPackageFile( *Tokens(TokenIndex), NULL, OutFilename ) )
			{
				new(FileList)FString(OutFilename);
			}
		}
	}
	// Or use all files otherwise.
	else
	{
		FileList = GPackageFileCache->GetPackageFileList();
	}
	
	if( FileList.Num() == 0 )
	{
		warnf( NAME_Warning, TEXT("No packages found") );
		return 1;
	}

	// Find shader types.
	ShaderTypeBasePassNoLightmap		= FindShaderTypeByName(TEXT("TBasePassPixelShaderFNoLightMapPolicyNoSkyLight"));
	ShaderTypeBasePassAndLightmap		= FindShaderTypeByName(TEXT("TBasePassPixelShaderFDirectionalVertexLightMapPolicyNoSkyLight"));
	ShaderTypePointLightWithShadowMap	= FindShaderTypeByName(TEXT("TLightPixelShaderFPointLightPolicyFShadowTexturePolicy"));

	check( ShaderTypeBasePassNoLightmap	);
	check( ShaderTypeBasePassAndLightmap );
	check( ShaderTypePointLightWithShadowMap );

	// Iterate over all files, loading up ones that have the map extension..
	for( INT FileIndex=0; FileIndex<FileList.Num(); FileIndex++ )
	{
		const FFilename& Filename = FileList(FileIndex);		

		// Disregard filenames that don't have the map extension if we're in MAPSONLY mode.
		if( bShouldOnlyLoadMaps && (Filename.GetExtension() != FURL::DefaultMapExt) )
		{
			continue;
		}

		// Skip filenames with the script extension. @todo: don't hardcode .u as the script file extension
		if( (Filename.GetExtension() == TEXT("u")) )
		{
			continue;
		}

		warnf( NAME_Log, TEXT("Loading %s"), *Filename );
		UPackage* Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
		if( Package == NULL )
		{
			warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
		}
		else
		{
			// Find the world and load all referenced levels.
			UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
			if( bAutoLoadSublevels && World )
			{
				AWorldInfo* WorldInfo	= World->GetWorldInfo();
				// Iterate over streaming level objects loading the levels.
				for( INT LevelIndex=0; LevelIndex<WorldInfo->StreamingLevels.Num(); LevelIndex++ )
				{
					ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
					if( StreamingLevel )
					{
						// Load package if found.
						FString SubFilename;
						if( GPackageFileCache->FindPackageFile( *StreamingLevel->PackageName.ToString(), NULL, SubFilename ) )
						{
							warnf(NAME_Log, TEXT("Loading sub-level %s"), *SubFilename);
							UObject::LoadPackage( NULL, *SubFilename, LOAD_None );
						}
					}
				}
			}

			// Figure out whether package is a map or content package.
			UBOOL bIsAMapPackage = World ? TRUE : FALSE;

			// Handle currently loaded objects.
			for( TObjectIterator<UObject> It; It; ++It )
			{
				UObject* Object = *It;
				HandleObject( Object, bIsAMapPackage ? Package : NULL, FALSE );
			}

			for( TObjectIterator<UStaticMesh> It; It; ++It )
			{
				UStaticMesh* Mesh = *It;
				if (appStristr(*Mesh->GetName(), TEXT("destruct")) != NULL)
				{
					debugf(TEXT("ITERATOR MeshName:%s"), *Mesh->GetFullName());
				}
			}
		}

		// Collect garbage, going back to a clean slate.
		UObject::CollectGarbage( RF_Native );

		// Verify that everything we cared about got cleaned up correctly.
		UBOOL bEncounteredUnmarkedObject = FALSE;
		for( TObjectIterator<UObject> It; It; ++It )
		{
			UObject* Object = *It;
			if( !Object->HasAllFlags( RF_Marked ) && !Object->IsIn(UObject::GetTransientPackage()) )
			{
				bEncounteredUnmarkedObject = TRUE;
				debugf(TEXT("----------------------------------------------------------------------------------------------------"));
				debugf(TEXT("%s didn't get cleaned up!"),*Object->GetFullName());
				UObject::StaticExec(*FString::Printf(TEXT("OBJ REFS CLASS=%s NAME=%s"),*Object->GetClass()->GetName(),*Object->GetPathName()));
				TMap<UObject*,UProperty*>	Route		= FArchiveTraceRoute::FindShortestRootPath( Object, TRUE, RF_Native  );
				FString						ErrorString	= FArchiveTraceRoute::PrintRootPath( Route, Object );
				debugf(TEXT("%s"),*ErrorString);
			}
		}
		check(!bEncounteredUnmarkedObject);
	}


	// Get time as a string
	const FString CurrentTime = appSystemTimeString();

	// Re-used helper variables for writing to CSV file.
	const FString CSVDirectory = appGameLogDir() + TEXT("AssetStatsCSVs") + PATH_SEPARATOR + FString::Printf( TEXT("%s-%d-%s"), GGameName, GetChangeListNumberForPerfTesting(), *CurrentTime ) + PATH_SEPARATOR;
	FString		CSVFilename		= TEXT("");
	FArchive*	CSVFile			= NULL;

	// Create CSV folder in case it doesn't exist yet.
	GFileManager->MakeDirectory( *CSVDirectory );

	if ((IgnoreObjects & IGNORE_StaticMesh) == 0)
		WriteOutCSVs<FStaticMeshStats>( ResourceNameToStaticMeshStats, TEXT( "StaticMeshStats" ), CurrentTime );

	if ((IgnoreObjects & IGNORE_Texture) == 0)
		WriteOutCSVs<FTextureStats>( ResourceNameToTextureStats, TEXT( "TextureStats" ), CurrentTime );

	if ((IgnoreObjects & IGNORE_Material) == 0)
		WriteOutCSVs<FMaterialStats>( ResourceNameToMaterialStats, TEXT( "MaterialStats" ), CurrentTime );

	if ((IgnoreObjects & IGNORE_Particle) == 0)
		WriteOutCSVs<FParticleStats>( ResourceNameToParticleStats, TEXT( "ParticleStats" ), CurrentTime );

	if ((IgnoreObjects & IGNORE_AnimSequence) == 0)
		WriteOutCSVs<FAnimSequenceStats>( ResourceNameToAnimStats, TEXT( "AnimStats" ), CurrentTime );

	if ((IgnoreObjects & IGNORE_StaticMeshActor) == 0)
		WriteOutCSVs<FLightingOptimizationStats>( ResourceNameToLightingStats, TEXT( "LightMapStats" ), CurrentTime );

	if ((IgnoreObjects & IGNORE_Particle) == 0)
		WriteOutCSVs<FTextureToParticleSystemStats>( ResourceNameToTextureToParticleSystemStats, TEXT( "TextureToParticleStats" ), CurrentTime );

	if ((IgnoreObjects & IGNORE_SoundCue) == 0)
		WriteOutCSVs<FSoundCueStats>( ResourceNameToSoundCueStats, TEXT( "SoundCueStats" ), CurrentTime );

	if ((IgnoreObjects & IGNORE_ShadowMap) == 0)
	{
		WriteOutCSVs<FShadowMap1DStats>( ResourceNameToShadowMap1DStats, TEXT( "ShadowMap1DStats" ), CurrentTime );
		WriteOutCSVs<FShadowMap2DStats>( ResourceNameToShadowMap2DStats, TEXT( "ShadowMap2DStats" ), CurrentTime );
	}		

	if ((IgnoreObjects & IGNORE_StaticMesh) == 0)
	{
		// this will now re organize the data into Level based statistics ( at least for FStaticMeshStats );
		// (we can template this to do it for any stat)
		typedef TMap<FString,TArray<FStaticMeshStats>> LevelToDataMapType;
		LevelToDataMapType LevelToDataMap;

		for( TMap<FString,FStaticMeshStats>::TIterator It(ResourceNameToStaticMeshStats); It; ++ It )
		{
			const FStaticMeshStats& StatsEntry = It.Value();

			for( PerLevelDataMap::TConstIterator Itr(StatsEntry.LevelNameToInstanceCount); Itr; ++Itr )
			{
				// find the map name in our LevelToDataMap
				TArray<FStaticMeshStats>* LevelData = LevelToDataMap.Find( Itr.Key() );
				// if the data exists for this level then we need to add it
				if( LevelData == NULL )
				{
					TArray<FStaticMeshStats> NewArray;
					NewArray.AddItem( StatsEntry );
					LevelToDataMap.Set( Itr.Key(), NewArray  );
					//warnf( TEXT( "NewLevelEntry! Level: %s %s Inst: %d" ), *Itr.Key(), *StatsEntry.ResourceName, Itr.Value() );
				}
				else
				{
					LevelData->AddItem( StatsEntry );
					//warnf( TEXT( "NewStatsEntry! Level: %s %s Inst: %d" ), *Itr.Key(), *StatsEntry.ResourceName, Itr.Value() );
				}
				//warnf( TEXT( "Level: %s %s Inst: %d" ), *Itr.Key(), *StatsEntry.ResourceName, Itr.Value() );
			}
		}


		// so now we just need to print them all out per level as we have a list of FStatMeshStats and we can use our modified ToCSV which takes a levelname
		for( LevelToDataMapType::TIterator Itr(LevelToDataMap); Itr; ++Itr )
		{
			const FString LevelSubDir = CSVDirectory + FString::Printf( TEXT("%s"), PATH_SEPARATOR TEXT("Levels") PATH_SEPARATOR );
			// CSV: Human-readable spreadsheet format.
			CSVFilename	= FString::Printf(TEXT("%s%s-%s-%i.csv")
				, *LevelSubDir
				, *FString::Printf( TEXT( "%s-StaticMeshStats" ), *Itr.Key() )
				, GGameName
				, GetChangeListNumberForPerfTesting()
				);

			CSVFile = GFileManager->CreateFileWriter( *CSVFilename );
			if( CSVFile != NULL )
			{	
				// Write out header row.
				const FString& HeaderRow = FStaticMeshStats::GetCSVHeaderRow();
				CSVFile->Serialize( TCHAR_TO_ANSI( *HeaderRow ), HeaderRow.Len() );

				// Write out each individual stats row.
				for( TArray<FStaticMeshStats>::TIterator It(Itr.Value()); It; ++It )
				{
					const FStaticMeshStats& StatsEntry = *It;
					const FString& Row = StatsEntry.ToCSV( Itr.Key() );
					if( Row.Len() > 0 )
					{
						CSVFile->Serialize( TCHAR_TO_ANSI( *Row ), Row.Len() );
					}
				}

				// Close and delete archive.
				CSVFile->Close();
				delete CSVFile;
			}
			else
			{
				debugf(NAME_Warning,TEXT("Could not create CSV file %s for writing."), *CSVFilename);
			}
		}
	}

	if ((IgnoreObjects & IGNORE_SoundCue) == 0)
	{
		//@todo. Really, templatized this...
		// this will now re organize the data into Level based statistics ( at least for FStaticMeshStats );
		// (we can template this to do it for any stat)
		typedef TMap<FString,TArray<FSoundCueStats>> SoundCueLevelToDataMapType;
		SoundCueLevelToDataMapType SoundCueLevelToDataMap;

		for( TMap<FString,FSoundCueStats>::TIterator SCIt(ResourceNameToSoundCueStats); SCIt; ++ SCIt )
		{
			const FSoundCueStats& StatsEntry = SCIt.Value();

			for( PerLevelDataMap::TConstIterator Itr(StatsEntry.LevelNameToInstanceCount); Itr; ++Itr )
			{
				// find the map name in our LevelToDataMap
				TArray<FSoundCueStats>* LevelData = SoundCueLevelToDataMap.Find( Itr.Key() );
				// if the data exists for this level then we need to add it
				if( LevelData == NULL )
				{
					TArray<FSoundCueStats> NewArray;
					NewArray.AddItem( StatsEntry );
					SoundCueLevelToDataMap.Set( Itr.Key(), NewArray  );
					//warnf( TEXT( "NewLevelEntry! Level: %s %s Inst: %d" ), *Itr.Key(), *StatsEntry.ResourceName, Itr.Value() );
				}
				else
				{
					LevelData->AddItem( StatsEntry );
					//warnf( TEXT( "NewStatsEntry! Level: %s %s Inst: %d" ), *Itr.Key(), *StatsEntry.ResourceName, Itr.Value() );
				}
				//warnf( TEXT( "Level: %s %s Inst: %d" ), *Itr.Key(), *StatsEntry.ResourceName, Itr.Value() );
			}
		}


		// so now we just need to print them all out per level as we have a list of FStatMeshStats and we can use our modified ToCSV which takes a levelname
		for( SoundCueLevelToDataMapType::TIterator SCItr(SoundCueLevelToDataMap); SCItr; ++SCItr )
		{
			const FString LevelSubDir = CSVDirectory + FString::Printf( TEXT("%s"), PATH_SEPARATOR TEXT("Levels") PATH_SEPARATOR );
			// CSV: Human-readable spreadsheet format.
			CSVFilename	= FString::Printf(TEXT("%s%s-%s-%i.csv")
				, *LevelSubDir
				, *FString::Printf( TEXT( "%s-SoundCueStats" ), *SCItr.Key() )
				, GGameName
				, GetChangeListNumberForPerfTesting()
				);

			CSVFile = GFileManager->CreateFileWriter( *CSVFilename );
			if( CSVFile != NULL )
			{	
				// Write out header row.
				const FString& HeaderRow = FSoundCueStats::GetCSVHeaderRow();
				CSVFile->Serialize( TCHAR_TO_ANSI( *HeaderRow ), HeaderRow.Len() );

				// Write out each individual stats row.
				for( TArray<FSoundCueStats>::TIterator It(SCItr.Value()); It; ++It )
				{
					const FSoundCueStats& StatsEntry = *It;
					const FString& Row = StatsEntry.ToCSV( SCItr.Key() );
					if( Row.Len() > 0 )
					{
						CSVFile->Serialize( TCHAR_TO_ANSI( *Row ), Row.Len() );
					}
				}

				// Close and delete archive.
				CSVFile->Close();
				delete CSVFile;
			}
			else
			{
				debugf(NAME_Warning,TEXT("Could not create CSV file %s for writing."), *CSVFilename);
			}
		}
	}

#if 0
	debugf(TEXT("%s"),*FStaticMeshStats::GetCSVHeaderRow());
	for( TMap<FString,FStaticMeshStats>::TIterator It(ResourceNameToStaticMeshStats); It; ++ It )
	{
		const FStaticMeshStats& StatsEntry = It.Value();
		debugf(TEXT("%s"),*StatsEntry.ToCSV());
	}

	debugf(TEXT("%s"),*FTextureStats::GetCSVHeaderRow());
	for( TMap<FString,FTextureStats>::TIterator It(ResourceNameToTextureStats); It; ++ It )
	{
		const FTextureStats& StatsEntry	= It.Value();
		debugf(TEXT("%s"),*StatsEntry.ToCSV());
	}

	debugf(TEXT("%s"),*FMaterialStats::GetCSVHeaderRow());
	for( TMap<FString,FMaterialStats>::TIterator It(ResourceNameToMaterialStats); It; ++ It )
	{
		const FMaterialStats& StatsEntry = It.Value();
		debugf(TEXT("%s"),*StatsEntry.ToCSV());
	}
#endif

	return 0;
}
IMPLEMENT_CLASS(UAnalyzeReferencedContentCommandlet)


/** Constructor, initializing all members */
UAnalyzeFallbackMaterialsCommandlet::FMaterialStats::FMaterialStats( UMaterial* Material, UAnalyzeFallbackMaterialsCommandlet* Commandlet )
:	ResourceType(Material->GetClass()->GetName())
,	ResourceName(Material->GetPathName())
{
	// Update dependency chain stats.
	Material->CacheResourceShaders(SP_PCD3D_SM2, FALSE, FALSE);
	FMaterialResource* MaterialResource = Material->GetMaterialResource(MSP_SM2);
	check( MaterialResource);
	DroppedFallbackComponents = MaterialResource->GetDroppedFallbackComponents();
	bIsFallbackMaterial = Material->bIsFallbackMaterial;
	CompileErrors = MaterialResource->GetCompileErrors();
}

/**
* Stringifies gathered stats for generated fallbacks in CSV format.
*
* @return comma separated list of stats
*/
FString UAnalyzeFallbackMaterialsCommandlet::FMaterialStats::GeneratedFallbackToCSV() const
{
	FString FallbackFailed = TEXT("");
	if (DroppedFallbackComponents & DroppedFallback_Failed)
	{
		FallbackFailed = TEXT("YES");
	}
	FString EmissiveDropped = TEXT("");
	if (DroppedFallbackComponents & DroppedFallback_Emissive)
	{
		EmissiveDropped = TEXT("YES");
	}
	FString DiffuseDropped = TEXT("");
	if (DroppedFallbackComponents & DroppedFallback_Diffuse)
	{
		DiffuseDropped = TEXT("YES");
	}
	FString NormalDropped = TEXT("");
	if (DroppedFallbackComponents & DroppedFallback_Normal)
	{
		NormalDropped = TEXT("YES");
	}
	FString SpecularDropped = TEXT("");
	if (DroppedFallbackComponents & DroppedFallback_Specular)
	{
		SpecularDropped = TEXT("YES");
	}

	return FString::Printf(TEXT("%s,%s,%s,%s,%s,%s,%s%s"),
		*ResourceType,
		*ResourceName,
		*FallbackFailed,
		*EmissiveDropped,
		*DiffuseDropped,
		*NormalDropped,
		*SpecularDropped,
		LINE_TERMINATOR);
}

/**
* Stringifies gathered stats for manually specified fallbacks with errors in CSV format.
*
* @return comma separated list of stats
*/
FString UAnalyzeFallbackMaterialsCommandlet::FMaterialStats::FallbackErrorsToCSV() const
{
	FString IsFallbackMaterial = TEXT("");
	if (bIsFallbackMaterial)
	{
		IsFallbackMaterial = TEXT("YES");
	}
	FString CombinedCompileErrors = TEXT("");
	for (INT i = 0; i < CompileErrors.Num(); i++)
	{
		CombinedCompileErrors += CompileErrors(i);
	}

	return FString::Printf(TEXT("%s,%s,%s%s"),
		*ResourceType,
		*ResourceName,
		*CombinedCompileErrors,
		LINE_TERMINATOR);
}

/**
* Returns a Generated Fallback header row for CSV
*
* @return comma separated header row
*/
FString UAnalyzeFallbackMaterialsCommandlet::FMaterialStats::GetGeneratedFallbackCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,FAILED!,Emissive Dropped!,Diffuse Dropped!,Normal Dropped,Specular Dropped") LINE_TERMINATOR;
}

/**
* Returns a Fallback Error header row for CSV
*
* @return comma separated header row
*/
FString UAnalyzeFallbackMaterialsCommandlet::FMaterialStats::GetFallbackErrorsCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,CompileErrors") LINE_TERMINATOR;
}

/**
* Retrieves/ creates material stats associated with passed in material.
*
* @warning: returns pointer into TMap, only valid till next time Set is called
*
* @param	Material	Material to retrieve/ create material stats for
* @return	pointer to material stats associated with material
*/
UAnalyzeFallbackMaterialsCommandlet::FMaterialStats* UAnalyzeFallbackMaterialsCommandlet::GetMaterialStats( UMaterial* Material )
{
	UAnalyzeFallbackMaterialsCommandlet::FMaterialStats* MaterialStats = ResourceNameToMaterialStats.Find( *Material->GetFullName() );
	if( MaterialStats == NULL )
	{
		MaterialStats =	&ResourceNameToMaterialStats.Set( *Material->GetFullName(), UAnalyzeFallbackMaterialsCommandlet::FMaterialStats( Material, this ) );
	}
	return MaterialStats;
}


/**
* Handles encountered object, routing to various sub handlers.
*
* @param	Object			Object to handle
* @param	LevelPackage	Currently loaded level package, can be NULL if not a level
* @param	bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeFallbackMaterialsCommandlet::HandleObject( UObject* Object, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	// Disregard marked objects as they won't go away with GC.
	if( !Object->HasAnyFlags( RF_Marked ) )
	{
		// Handle material.
		if( Object->IsA(UMaterial::StaticClass()) )
		{
			HandleMaterial( (UMaterial*) Object, LevelPackage, bIsScriptReferenced );
		}
	}
}

/**
* Handles gathering stats for passed in material.
*
* @param Material	Material to gather stats for
* @param LevelPackage	Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeFallbackMaterialsCommandlet::HandleMaterial( UMaterial* Material, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	UAnalyzeFallbackMaterialsCommandlet::FMaterialStats* MaterialStats = GetMaterialStats( Material );	
}



INT UAnalyzeFallbackMaterialsCommandlet::Main( const FString& Params )
{
	// Parse command line.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	const TCHAR* Parms = *Params;
	ParseCommandLine(Parms, Tokens, Switches);

	// Whether to only deal with map files.
	const UBOOL bShouldOnlyLoadMaps	= Switches.FindItemIndex(TEXT("MAPSONLY")) != INDEX_NONE;
	// Whether to exclude script references.
	const UBOOL bExcludeScript		= Switches.FindItemIndex(TEXT("EXCLUDESCRIPT")) != INDEX_NONE;
	// Whether to dump out errors from existing manually specified fallback materials
	const UBOOL bTrackFallbackErrors = Switches.FindItemIndex(TEXT("FALLBACKERRORS")) != INDEX_NONE;

	// Load up all script files in EditPackages.
	UEditorEngine* EditorEngine = CastChecked<UEditorEngine>(GEngine);
	for( INT i=0; i<EditorEngine->EditPackages.Num(); i++ )
	{
		LoadPackage( NULL, *EditorEngine->EditPackages(i), LOAD_NoWarn );
	}

	// Mark loaded objects as they are part of the always loaded set and are not taken into account for stats.
	for( TObjectIterator<UObject> It; It; ++It )
	{
		UObject* Object = *It;
		// Script referenced asset.
		if( !bExcludeScript )
		{
			HandleObject( Object, NULL, TRUE );
		}
		// Mark object as always loaded so it doesn't get counted multiple times.
		Object->SetFlags( RF_Marked );
	}

	TArray<FString> FileList;

	// Build package file list from passed in command line if tokens are specified.
	if( Tokens.Num() )
	{
		for( INT TokenIndex=0; TokenIndex<Tokens.Num(); TokenIndex++ )
		{
			// Lookup token in file cache and add filename if found.
			FString OutFilename;
			if( GPackageFileCache->FindPackageFile( *Tokens(TokenIndex), NULL, OutFilename ) )
			{
				new(FileList)FString(OutFilename);
			}
		}
	}
	// Or use all files otherwise.
	else
	{
		FileList = GPackageFileCache->GetPackageFileList();
	}

	if( FileList.Num() == 0 )
	{
		warnf( NAME_Warning, TEXT("No packages found") );
		return 1;
	}

	// Iterate over all files, loading up ones that have the map extension..
	for( INT FileIndex=0; FileIndex<FileList.Num(); FileIndex++ )
	{
		const FFilename& Filename = FileList(FileIndex);		

		// Disregard filenames that don't have the map extension if we're in MAPSONLY mode.
		if( bShouldOnlyLoadMaps && (Filename.GetExtension() != FURL::DefaultMapExt) )
		{
			continue;
		}

		// Skip filenames with the script extension. @todo: don't hardcode .u as the script file extension
		if( (Filename.GetExtension() == TEXT("u")) )
		{
			continue;
		}

		warnf( NAME_Log, TEXT("Loading %s"), *Filename );
		UPackage* Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
		if( Package == NULL )
		{
			warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
		}
		else
		{
			// Figure out whether package is a map or content package.
			const UBOOL bIsAMapPackage = FindObject<UWorld>(Package, TEXT("TheWorld")) != NULL;

			// Handle currently loaded objects.
			for( TObjectIterator<UObject> It; It; ++It )
			{
				UObject* Object = *It;
				HandleObject( Object, bIsAMapPackage ? Package : NULL, FALSE );
			}
		}

		// Collect garbage, going back to a clean slate.
		UObject::CollectGarbage( RF_Native );

		// Verify that everything we cared about got cleaned up correctly.
		UBOOL bEncounteredUnmarkedObject = FALSE;
		for( TObjectIterator<UObject> It; It; ++It )
		{
			UObject* Object = *It;
			if( !Object->HasAllFlags( RF_Marked ) && !Object->IsIn(UObject::GetTransientPackage()) )
			{
				bEncounteredUnmarkedObject = TRUE;
				debugf(TEXT("----------------------------------------------------------------------------------------------------"));
				debugf(TEXT("%s didn't get cleaned up!"),*Object->GetFullName());
				UObject::StaticExec(*FString::Printf(TEXT("OBJ REFS CLASS=%s NAME=%s"),*Object->GetClass()->GetName(),*Object->GetPathName()));
				TMap<UObject*,UProperty*>	Route		= FArchiveTraceRoute::FindShortestRootPath( Object, TRUE, RF_Native  );
				const FString				ErrorString	= FArchiveTraceRoute::PrintRootPath( Route, Object );
				debugf(TEXT("%s"),*ErrorString);
			}
		}
		check(!bEncounteredUnmarkedObject);
	}


	// Create string with system time to create a unique filename.
	INT Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec;
	appSystemTime( Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec );
	const FString	CurrentTime = FString::Printf(TEXT("%i.%02i.%02i-%02i.%02i.%02i"), Year, Month, Day, Hour, Min, Sec );

	// Re-used helper variables for writing to CSV file.
	FString		CSVDirectory	= appGameLogDir() + TEXT("AssetStatsCSVs") PATH_SEPARATOR;
	FString		CSVFilename		= TEXT("");
	FArchive*	CSVFile			= NULL;

	// Create CSV folder in case it doesn't exist yet.
	GFileManager->MakeDirectory( *CSVDirectory );

	CSVFilename	= FString::Printf(TEXT("%sFallbackMaterialStats-%s-%i-%s.csv"), *CSVDirectory, GGameName, GetChangeListNumberForPerfTesting(), *CurrentTime);
	CSVFile		= GFileManager->CreateFileWriter( *CSVFilename );
	if( CSVFile )
	{	
		// Write out header row.
		if (bTrackFallbackErrors)
		{
			const FString& HeaderRow = FMaterialStats::GetFallbackErrorsCSVHeaderRow();
			CSVFile->Serialize( TCHAR_TO_ANSI( *HeaderRow ), HeaderRow.Len() );
		}
		else
		{
			const FString& HeaderRow = FMaterialStats::GetGeneratedFallbackCSVHeaderRow();
			CSVFile->Serialize( TCHAR_TO_ANSI( *HeaderRow ), HeaderRow.Len() );
		}

		// Write out each individual stats row.
		for( TMap<FString,FMaterialStats>::TIterator It(ResourceNameToMaterialStats); It; ++ It )
		{
			const FMaterialStats& StatsEntry = It.Value();
			//dump out information on existing fallback materials
			if (bTrackFallbackErrors)
			{
				//only print out the stats row if this is a fallback material that had compile errors
				if (StatsEntry.bIsFallbackMaterial && StatsEntry.CompileErrors.Num() > 0)
				{
					const FString& Row = StatsEntry.FallbackErrorsToCSV();
					CSVFile->Serialize( TCHAR_TO_ANSI( *Row ), Row.Len() );
				}
			}
			//dump out information on automatically generated fallbacks (which are only used on materials without specified fallbacks)
			else
			{
				//only print the stats row if any component was dropped
				if (StatsEntry.DroppedFallbackComponents != DroppedFallback_None)
				{
					const FString& Row = StatsEntry.GeneratedFallbackToCSV();
					CSVFile->Serialize( TCHAR_TO_ANSI( *Row ), Row.Len() );
				}
			}
		}

		// Close and delete archive.
		CSVFile->Close();
		delete CSVFile;
	}
	else
	{
		debugf(NAME_Warning,TEXT("Could not create CSV file %s for writing."), *CSVFilename);
	}

	return 0;
}
IMPLEMENT_CLASS(UAnalyzeFallbackMaterialsCommandlet);


/*-----------------------------------------------------------------------------
UShowStylesCommandlet
-----------------------------------------------------------------------------*/
IMPLEMENT_CLASS(UShowStylesCommandlet);

INT UShowStylesCommandlet::Main(const FString& Params)
{
	INT Result = 0;

	// Parse command line args.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	const TCHAR* Parms = *Params;
	ParseCommandLine(Parms, Tokens, Switches);

	for ( INT TokenIndex = 0; TokenIndex < Tokens.Num(); TokenIndex++ )
	{
		FString& PackageWildcard = Tokens(TokenIndex);

		TArray<FString> PackageFileNames;
		GFileManager->FindFiles( PackageFileNames, *PackageWildcard, 1, 0 );
		if( PackageFileNames.Num() == 0 )
		{
			// if no files were found, it might be an unqualified path; try prepending the .u output path
			// if one were going to make it so that you could use unqualified paths for package types other
			// than ".u", here is where you would do it
			GFileManager->FindFiles( PackageFileNames, *(appScriptOutputDir() * PackageWildcard), 1, 0 );

			if ( PackageFileNames.Num() == 0 )
			{
				TArray<FString> Paths;
				if ( GConfig->GetArray( TEXT("Core.System"), TEXT("Paths"), Paths, GEngineIni ) > 0 )
				{
					for ( INT i = 0; i < Paths.Num(); i++ )
					{
						GFileManager->FindFiles( PackageFileNames, *(Paths(i) * PackageWildcard), 1, 0 );
					}
				}
			}
			else
			{
				// re-add the path information so that GetPackageLinker finds the correct version of the file.
				FFilename WildcardPath = appScriptOutputDir() * PackageWildcard;
				for ( INT FileIndex = 0; FileIndex < PackageFileNames.Num(); FileIndex++ )
				{
					PackageFileNames(FileIndex) = WildcardPath.GetPath() * PackageFileNames(FileIndex);
				}
			}

			// Try finding package in package file cache.
			if ( PackageFileNames.Num() == 0 )
			{
				FString Filename;
				if( GPackageFileCache->FindPackageFile( *PackageWildcard, NULL, Filename ) )
				{
					new(PackageFileNames)FString(Filename);
				}
			}
		}
		else
		{
			// re-add the path information so that GetPackageLinker finds the correct version of the file.
			FFilename WildcardPath = PackageWildcard;
			for ( INT FileIndex = 0; FileIndex < PackageFileNames.Num(); FileIndex++ )
			{
				PackageFileNames(FileIndex) = WildcardPath.GetPath() * PackageFileNames(FileIndex);
			}
		}

		if ( PackageFileNames.Num() == 0 )
		{
			warnf(TEXT("No packages found using '%s'!"), *PackageWildcard);
			continue;
		}

		// reset the loaders for the packages we want to load so that we don't find the wrong version of the file
		// (otherwise, attempting to run pkginfo on e.g. Engine.xxx will always return results for Engine.u instead)
		for ( INT FileIndex = 0; FileIndex < PackageFileNames.Num(); FileIndex++ )
		{
			const FString& PackageName = FPackageFileCache::PackageFromPath(*PackageFileNames(FileIndex));
			UPackage* ExistingPackage = FindObject<UPackage>(NULL, *PackageName, TRUE);
			if ( ExistingPackage != NULL )
			{
				ResetLoaders(ExistingPackage);
			}
		}

		for( INT FileIndex = 0; FileIndex < PackageFileNames.Num(); FileIndex++ )
		{
			const FString &Filename = PackageFileNames(FileIndex);

			warnf( NAME_Log, TEXT("Loading %s"), *Filename );

			UObject* Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
			if( Package == NULL )
			{
				warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
			}
			else
			{
				for ( FObjectIterator It; It; ++It )
				{
					if ( It->IsIn(Package) && It->IsA(UUIStyle::StaticClass()) )
					{
						DisplayStyleInfo(Cast<UUIStyle>(*It));
					}
				}
			}
		}
	}

	return Result;
}

void UShowStylesCommandlet::DisplayStyleInfo( UUIStyle* Style )
{
	// display the info about this style
	GWarn->Log(TEXT("*************************"));
	GWarn->Logf(TEXT("%s"), *Style->GetPathName());
	GWarn->Logf(TEXT("\t Archetype: %s"), *Style->GetArchetype()->GetPathName());
	GWarn->Logf(TEXT("\t       Tag: %s"), *Style->GetStyleName());
	GWarn->Logf(TEXT("\tStyleClass: %s"), Style->StyleDataClass != NULL ? *Style->StyleDataClass->GetName() : TEXT("NULL"));
	GWarn->Log(TEXT("\tStyle Data:"));

	const INT Indent = GetStyleDataIndent(Style);
	for ( TMap<UUIState*,UUIStyle_Data*>::TIterator It(Style->StateDataMap); It; ++It )
	{
		UUIState* State = It.Key();
		UUIStyle_Data* StyleData = It.Value();

		GWarn->Logf(TEXT("\t%*s: %s"), Indent, *State->GetClass()->GetName(), *StyleData->GetPathName());
		UUIStyle_Combo* ComboStyleData = Cast<UUIStyle_Combo>(StyleData);
		if ( ComboStyleData != NULL )
		{
			UUIStyle_Data* CustomTextStyle = ComboStyleData->TextStyle.GetCustomStyleData();
			GWarn->Logf(TEXT("\t\t TextStyle: %s"), CustomTextStyle ? *CustomTextStyle->GetPathName() : TEXT("NULL"));

			UUIStyle_Data* CustomImageStyle = ComboStyleData->ImageStyle.GetCustomStyleData();
			GWarn->Logf(TEXT("\t\tImageStyle: %s"), CustomImageStyle ? *CustomImageStyle->GetPathName() : TEXT("NULL"));
		}
	}
}

INT UShowStylesCommandlet::GetStyleDataIndent( UUIStyle* Style )
{
	INT Result = 0;

	check(Style);
	for ( TMap<UUIState*,UUIStyle_Data*>::TIterator It(Style->StateDataMap); It; ++It )
	{
		FString StateClassName = It.Key()->GetClass()->GetName();
		Result = Max(StateClassName.Len(), Result);
	}

	return Result;
}


IMPLEMENT_CLASS(UTestWordWrapCommandlet);
INT UTestWordWrapCommandlet::Main(const FString& Params)
{
	INT Result = 0;

	// replace any \n strings with the real character code
	FString MyParams = Params.Replace(TEXT("\\n"), TEXT("\n"));
	const TCHAR* Parms = *MyParams;

	INT WrapWidth = 0;
	FString WrapWidthString;
	ParseToken(Parms, WrapWidthString, FALSE);
	WrapWidth = appAtoi(*WrapWidthString);

	// advance past the space between the width and the test string
	Parms++;
	warnf(TEXT("WrapWidth: %i  WrapText: '%s'"), WrapWidth, Parms);
	UFont* DrawFont = GEngine->GetTinyFont();

	FRenderParameters Parameters(0, 0, WrapWidth, 0, DrawFont);
	TArray<FWrappedStringElement> Lines;

	UUIString::WrapString(Parameters, 0, Parms, Lines, TEXT("\n"));

	warnf(TEXT("Result: %i lines"), Lines.Num());
	for ( INT LineIndex = 0; LineIndex < Lines.Num(); LineIndex++ )
	{
		FWrappedStringElement& Line = Lines(LineIndex);
		warnf(TEXT("Line %i): (X=%.2f,Y=%.2f) '%s'"), LineIndex, Line.LineExtent.X, Line.LineExtent.Y, *Line.Value);
	}

	return Result;
}

/** sets certain allowed package flags on the specified package(s) */
INT USetPackageFlagsCommandlet::Main(const FString& Params)
{
	TArray<FString> Tokens, Switches;

	ParseCommandLine(*Params, Tokens, Switches);

	if (Tokens.Num() < 2)
	{
		warnf(TEXT("Syntax: setpackageflags <package/wildcard> <flag1=value> <flag2=value>..."));
		warnf(TEXT("Supported flags: ServerSideOnly, ClientOptional, AllowDownload"));
		return 1;
	}

	// find all the files matching the specified filename/wildcard
	TArray<FString> FilesInPath;
	GFileManager->FindFiles(FilesInPath, *Tokens(0), 1, 0);
	if (FilesInPath.Num() == 0)
	{
		warnf(NAME_Error, TEXT("No packages found matching %s!"), *Tokens(0));
		return 2;
	}
	// get the directory part of the filename
	INT ChopPoint = Max(Tokens(0).InStr(TEXT("/"), 1) + 1, Tokens(0).InStr(TEXT("\\"), 1) + 1);
	if (ChopPoint < 0)
	{
		ChopPoint = Tokens(0).InStr( TEXT("*"), 1 );
	}
	FString PathPrefix = (ChopPoint < 0) ? TEXT("") : Tokens(0).Left(ChopPoint);

	// parse package flags
	DWORD PackageFlagsToAdd = 0, PackageFlagsToRemove = 0;
	for (INT i = 1; i < Tokens.Num(); i++)
	{
		DWORD NewFlag = 0;
		UBOOL bValue;
		if (ParseUBOOL(*Tokens(i), TEXT("ServerSideOnly="), bValue))
		{
			NewFlag = PKG_ServerSideOnly;
		}
		else if (ParseUBOOL(*Tokens(i), TEXT("ClientOptional="), bValue))
		{
			NewFlag = PKG_ClientOptional;
		}
		else if (ParseUBOOL(*Tokens(i), TEXT("AllowDownload="), bValue))
		{
			NewFlag = PKG_AllowDownload;
		}
		else
		{
			warnf(NAME_Warning, TEXT("Unknown package flag '%s' specified"), *Tokens(i));
		}
		if (NewFlag != 0)
		{
			if (bValue)
			{
				PackageFlagsToAdd |= NewFlag;
			}
			else
			{
				PackageFlagsToRemove |= NewFlag;
			}
		}
	}
	// process files
	for (INT i = 0; i < FilesInPath.Num(); i++)
	{
		const FString& PackageName = FilesInPath(i);
		const FString PackagePath = PathPrefix + PackageName;
		FString FileName;
		if( !GPackageFileCache->FindPackageFile( *PackagePath, NULL, FileName ) )
		{
			warnf(NAME_Error, TEXT("Couldn't find package '%s'"), *PackageName);
			continue;
		}

		// skip if read-only
		if (GFileManager->IsReadOnly(*FileName))
		{
			warnf(TEXT("Skipping %s (read-only)"), *FileName);
		}
		else
		{
			// load the package
			warnf(TEXT("Loading %s..."), *PackageName); 
			UPackage* Package = LoadPackage(NULL, *PackageName, LOAD_None);
			if (Package == NULL)
			{
				warnf(NAME_Error, TEXT("Failed to load package '%s'"), *PackageName);
			}
			else
			{
				// set flags
				Package->PackageFlags |= PackageFlagsToAdd;
				Package->PackageFlags &= ~PackageFlagsToRemove;
				// save the package
				warnf(TEXT("Saving %s..."), *PackageName);
				SavePackage(Package, NULL, RF_Standalone, *FileName, GWarn);
			}
			// GC the package
			warnf(TEXT("Cleaning up..."));
			CollectGarbage(RF_Native);
		}
	}
	return 0;
}
IMPLEMENT_CLASS(USetPackageFlagsCommandlet);


/* ==========================================================================================================
	UPerformMapCheckCommandlet
========================================================================================================== */
/**
 * Evalutes the command-line to determine which maps to check.  By default all maps are checked (except PIE and trash-can maps)
 * Provides child classes with a chance to initialize any variables, parse the command line, etc.
 *
 * @param	Tokens			the list of tokens that were passed to the commandlet
 * @param	Switches		the list of switches that were passed on the commandline
 * @param	MapPathNames	receives the list of path names for the maps that will be checked.
 *
 * @return	0 to indicate that the commandlet should continue; otherwise, the error code that should be returned by Main()
 */
INT UPerformMapCheckCommandlet::InitializeMapCheck( const TArray<FString>& Tokens, const TArray<FString>& Switches, TArray<FFilename>& MapPathNames )
{
	bTestOnly = Switches.ContainsItem(TEXT("TESTONLY"));
	MapIndex = TotalMapsChecked = 0;

	TArray<FString> Unused;
	// assume the first token is the map wildcard/pathname
	FString MapWildcard = Tokens.Num() > 0 ? Tokens(0) : FString(TEXT("*.")) + FURL::DefaultMapExt;
	if ( NormalizePackageNames( Unused, MapNames, MapWildcard, NORMALIZE_ExcludeContentPackages) )
	{
		return 0;
	}

	return 1;
}

INT UPerformMapCheckCommandlet::Main( const FString& Params )
{
	TArray<FString> Tokens, Switches;
	ParseCommandLine(*Params, Tokens, Switches);

	INT ReturnValue = InitializeMapCheck(Tokens, Switches, MapNames);
	if ( ReturnValue != 0 )
	{
		return ReturnValue;
	}

	warnf(TEXT("Found %i maps"), MapNames.Num());
	INT GCIndex=0;
	for ( MapIndex = 0; MapIndex < MapNames.Num(); MapIndex++ )
	{
		const FFilename& Filename = MapNames(MapIndex);
		warnf( TEXT("Loading  %s...  (%i / %i)"), *Filename, MapIndex, MapNames.Num() );

		UPackage* Package = UObject::LoadPackage( NULL, *Filename, LOAD_NoWarn );
		if ( Package == NULL )
		{
			warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
			continue;
		}

		// skip packages in the trashcan or PIE packages
		if ( (Package->PackageFlags&(PKG_Trash|PKG_PlayInEditor)) != 0 )
		{
			warnf(TEXT("Skipping %s (%s)"), *Filename, (Package->PackageFlags&PKG_Trash) != 0 ? TEXT("Trashcan Map") : TEXT("PIE Map"));
			UObject::CollectGarbage(RF_Native);
			continue;
		}

		TotalMapsChecked++;

		warnf(TEXT("Checking %s..."), *Filename);
		ReturnValue = CheckMapPackage( Package );
		if ( ReturnValue != 0 )
		{
			return ReturnValue;
		}

		// save shader caches incase we encountered some that weren't fully cached yet
		SaveLocalShaderCaches();

		// collecting garbage every 10 maps instead of every map makes the commandlet run much faster
		if( (++GCIndex % 10) == 0 )
		{
			UObject::CollectGarbage(RF_Native);
		}
	}

	ReturnValue = ProcessResults();
	return ReturnValue;
}

/**
 * The main worker method - performs the commandlets tests on the package.
 *
 * @param	MapPackage	the current package to be processed
 *
 * @return	0 to indicate that the commandlet should continue; otherwise, the error code that should be returned by Main()
 */
INT UPerformMapCheckCommandlet::CheckMapPackage( UPackage* MapPackage )
{
	for ( TObjectIterator<AActor> It; It; ++It )
	{
		AActor* Actor = *It;
		if ( Actor->IsIn(MapPackage) && !Actor->IsTemplate() )
		{
			Actor->CheckForErrors();
		}
	}

	return 0;
}
IMPLEMENT_CLASS(UPerformMapCheckCommandlet);

/* ==========================================================================================================
	UFindStaticActorsRefsCommandlet
========================================================================================================== */
INT UFindStaticActorsRefsCommandlet::Main(const FString& Params)
{
	return Super::Main(Params);
}
/**
 * Evalutes the command-line to determine which maps to check.  By default all maps are checked (except PIE and trash-can maps)
 * Provides child classes with a chance to initialize any variables, parse the command line, etc.
 *
 * @param	Tokens			the list of tokens that were passed to the commandlet
 * @param	Switches		the list of switches that were passed on the commandline
 * @param	MapPathNames	receives the list of path names for the maps that will be checked.
 *
 * @return	0 to indicate that the commandlet should continue; otherwise, the error code that should be returned by Main()
 */
INT UFindStaticActorsRefsCommandlet::InitializeMapCheck( const TArray<FString>& Tokens, const TArray<FString>& Switches, TArray<FFilename>& MapPathNames )
{
	bStaticKismetRefs = Switches.ContainsItem(TEXT("STATICREFS"));
	bShowObjectNames = Switches.ContainsItem(TEXT("OBJECTNAMES"));
	bLogObjectNames = Switches.ContainsItem(TEXT("LOGOBJECTNAMES"));
	bShowReferencers = Switches.ContainsItem(TEXT("SHOWREFERENCERS"));
	TotalStaticMeshActors = 0, TotalStaticLightActors = 0;
	TotalReferencedStaticMeshActors = 0, TotalReferencedStaticLightActors = 0;

	return Super::InitializeMapCheck(Tokens, Switches, MapPathNames);
}

/**
 * The main worker method - performs the commandlets tests on the package.
 *
 * @param	MapPackage	the current package to be processed
 *
 * @return	0 to indicate that the commandlet should continue; otherwise, the error code that should be returned by Main()
 */
INT UFindStaticActorsRefsCommandlet::CheckMapPackage( UPackage* MapPackage )
{
	INT Result = 0;

	// find all StaticMeshActors and static Light actors which are referenced by something in the map
	UWorld* World = FindObject<UWorld>( MapPackage, TEXT("TheWorld") );
	if ( World )
	{
		// make sure that the world's PersistentLevel is part of the levels array for the purpose of this test.
		World->Levels.AddUniqueItem(World->PersistentLevel);
		for ( INT LevelIndex = 0; LevelIndex < World->Levels.Num(); LevelIndex++ )
		{
			ULevel* Level = World->Levels(LevelIndex);

			// remove all StaticMeshActors from the level's Actor array so that we don't get false positives.
			TArray<AStaticMeshActor*> StaticMeshActors;
			if ( ContainsObjectOfClass<AActor>(Level->Actors, AStaticMeshActor::StaticClass(), FALSE, (TArray<AActor*>*)&StaticMeshActors) )
			{
				for ( INT i = 0; i < StaticMeshActors.Num(); i++ )
				{
					Level->Actors.RemoveItem(StaticMeshActors(i));
				}
			}

			// same for lights marked bStatic
			TArray<ALight*> Lights;
			if ( ContainsObjectOfClass<AActor>(Level->Actors, ALight::StaticClass(), FALSE, (TArray<AActor*>*)&Lights) )
			{
				for ( INT i = Lights.Num() - 1; i >= 0; i-- )
				{
					// only care about static lights - if the light is static, remove it from the level's Actors array
					// so that we don't get false positives; otherwise, remove it from the list of lights that we'll process
					if ( Lights(i)->bStatic )
					{
						Level->Actors.RemoveItem(Lights(i));
					}
					else
					{
						Lights.Remove(i);
					}
				}
			}

			// now use the object reference collector to find the static mesh actors that are still being referenced
			TArray<AStaticMeshActor*> ReferencedStaticMeshActors;
			TArchiveObjectReferenceCollector<AStaticMeshActor> SMACollector(&ReferencedStaticMeshActors, MapPackage, FALSE, TRUE, TRUE, TRUE);
			Level->Serialize( SMACollector );

			if ( ReferencedStaticMeshActors.Num() > 0 )
			{
				warnf(TEXT("\t%i of %i StaticMeshActors referenced"), ReferencedStaticMeshActors.Num(), StaticMeshActors.Num());
				if ( bShowReferencers )
				{
					TFindObjectReferencers<AStaticMeshActor> StaticMeshReferencers(ReferencedStaticMeshActors, MapPackage);

					for ( INT RefIndex = 0; RefIndex < ReferencedStaticMeshActors.Num(); RefIndex++ )
					{
						AStaticMeshActor* StaticMeshActor = ReferencedStaticMeshActors(RefIndex);
						debugf(TEXT("\t  %i) %s"), RefIndex, *StaticMeshActor->GetFullName());

						TArray<UObject*> Referencers;
						StaticMeshReferencers.MultiFind(StaticMeshActor, Referencers);

						INT Count=0;
						for ( INT ReferencerIndex = Referencers.Num() - 1; ReferencerIndex >= 0; ReferencerIndex-- )
						{
							if ( Referencers(ReferencerIndex) != StaticMeshActor->StaticMeshComponent )
							{
								debugf(TEXT("\t\t %i) %s"), Count, *Referencers(ReferencerIndex)->GetFullName());
								Count++;
							}
						}

						if ( Count == 0 )
						{
							debugf(TEXT("\t\t  (StaticMeshComponent referenced from external source)"));
						}

						debugf(TEXT(""));
					}

					debugf(TEXT("******"));
				}
				else if ( bShowObjectNames )
				{
					for ( INT RefIndex = 0; RefIndex < ReferencedStaticMeshActors.Num(); RefIndex++ )
					{
						warnf(TEXT("\t  %i) %s"), RefIndex, *ReferencedStaticMeshActors(RefIndex)->GetFullName());
					}

					warnf(TEXT(""));
				}
				else if ( bLogObjectNames )
				{
					for ( INT RefIndex = 0; RefIndex < ReferencedStaticMeshActors.Num(); RefIndex++ )
					{
						debugf(TEXT("\t  %i) %s"), RefIndex, *ReferencedStaticMeshActors(RefIndex)->GetFullName());
					}

					debugf(TEXT(""));
				}

				ReferencedStaticMeshActorMap.Set(MapIndex, ReferencedStaticMeshActors.Num());
				TotalReferencedStaticMeshActors += ReferencedStaticMeshActors.Num();
			}

			TArray<ALight*> ReferencedLights;
			TArchiveObjectReferenceCollector<ALight> LightCollector(&ReferencedLights, MapPackage, FALSE, TRUE, TRUE, TRUE);
			Level->Serialize( LightCollector );

			for ( INT RefIndex = ReferencedLights.Num() - 1; RefIndex >= 0; RefIndex-- )
			{
				if ( !ReferencedLights(RefIndex)->bStatic )
				{
					ReferencedLights.Remove(RefIndex);
				}
			}
			if ( ReferencedLights.Num() > 0 )
			{
				warnf(TEXT("\t%i of %i static Light actors referenced"), ReferencedLights.Num(), Lights.Num());
				if ( bShowReferencers )
				{
					TFindObjectReferencers<ALight> StaticLightReferencers(ReferencedLights, MapPackage);

					for ( INT RefIndex = 0; RefIndex < ReferencedLights.Num(); RefIndex++ )
					{
						ALight* StaticLightActor = ReferencedLights(RefIndex);
						debugf(TEXT("\t  %i) %s"), RefIndex, *StaticLightActor->GetFullName());

						TArray<UObject*> Referencers;
						StaticLightReferencers.MultiFind(StaticLightActor, Referencers);

						INT Count=0;
						UBOOL bShowSubobjects = FALSE;
LogLightReferencers:
						for ( INT ReferencerIndex = Referencers.Num() - 1; ReferencerIndex >= 0; ReferencerIndex-- )
						{
							if ( bShowSubobjects || !Referencers(ReferencerIndex)->IsIn(StaticLightActor) )
							{
								debugf(TEXT("\t\t %i) %s"), Count, *Referencers(ReferencerIndex)->GetFullName());
								Count++;
							}
						}

						if ( !bShowSubobjects && Count == 0 )
						{
							bShowSubobjects = TRUE;
							goto LogLightReferencers;
						}

						debugf(TEXT(""));
					}

					debugf(TEXT("******"));
				}
				else if ( bShowObjectNames )
				{
					for ( INT RefIndex = 0; RefIndex < ReferencedLights.Num(); RefIndex++ )
					{
						warnf(TEXT("\t  %i) %s"), RefIndex, *ReferencedLights(RefIndex)->GetFullName());
					}
					warnf(TEXT(""));
				}
				else if ( bLogObjectNames )
				{
					for ( INT RefIndex = 0; RefIndex < ReferencedLights.Num(); RefIndex++ )
					{
						debugf(TEXT("\t  %i) %s"), RefIndex, *ReferencedLights(RefIndex)->GetFullName());
					}

					debugf(TEXT(""));
				}

				ReferencedStaticLightActorMap.Set(MapIndex, ReferencedLights.Num());
				TotalReferencedStaticLightActors += ReferencedLights.Num();
			}

			if (!bShowObjectNames
				&&	(ReferencedStaticMeshActors.Num() > 0 || ReferencedLights.Num() > 0))
			{
				warnf(TEXT(""));
			}

			TotalStaticMeshActors += StaticMeshActors.Num();
			TotalStaticLightActors += Lights.Num();
		}
	}

	return Result;
}

IMPLEMENT_COMPARE_CONSTREF(INT,ReferencedStaticActorCount,
{
	return B - A;
})

/**
 * Called after all packages have been processed - provides commandlets with an opportunity to print out test results or
 * provide feedback.
 *
 * @return	0 to indicate that the commandlet should continue; otherwise, the error code that should be returned by Main()
 */
INT UFindStaticActorsRefsCommandlet::ProcessResults()
{
	INT Result = 0;

	// sort the list of maps by the number of referenceed static actors contained in them
	ReferencedStaticMeshActorMap.ValueSort<COMPARE_CONSTREF_CLASS(INT,ReferencedStaticActorCount)>();
	ReferencedStaticLightActorMap.ValueSort<COMPARE_CONSTREF_CLASS(INT,ReferencedStaticActorCount)>();

	warnf(LINE_TERMINATOR TEXT("Referenced StaticMeshActor Summary"));
	for ( TMap<INT,INT>::TIterator It(ReferencedStaticMeshActorMap); It; ++It )
	{
		warnf(TEXT("%-4i %s"), It.Value(), *MapNames(It.Key()));
	}

	warnf(LINE_TERMINATOR TEXT("Referenced Static Light Actor Summary"));
	for ( TMap<INT,INT>::TIterator It(ReferencedStaticLightActorMap); It; ++It )
	{
		warnf(TEXT("%-4i %s"), It.Value(), *MapNames(It.Key()));
	}

	warnf(LINE_TERMINATOR TEXT("Total static actors referenced across %i maps"), TotalMapsChecked);
	warnf(TEXT("StaticMeshActors: %i (of %i) across %i maps"), TotalReferencedStaticMeshActors, TotalStaticMeshActors, ReferencedStaticMeshActorMap.Num());
	warnf(TEXT("Static Light Actors: %i (of %i) across %i maps"), TotalReferencedStaticLightActors, TotalStaticLightActors, ReferencedStaticLightActorMap.Num());

	return Result;
}
IMPLEMENT_CLASS(UFindStaticActorsRefsCommandlet);

/* ==========================================================================================================
	UFindRenamedPrefabSequencesCommandlet
========================================================================================================== */
INT UFindRenamedPrefabSequencesCommandlet::Main(const FString& Params)
{
	return Super::Main(Params);
}
/**
 * Evalutes the command-line to determine which maps to check.  By default all maps are checked (except PIE and trash-can maps)
 * Provides child classes with a chance to initialize any variables, parse the command line, etc.
 *
 * @param	Tokens			the list of tokens that were passed to the commandlet
 * @param	Switches		the list of switches that were passed on the commandline
 * @param	MapPathNames	receives the list of path names for the maps that will be checked.
 *
 * @return	0 to indicate that the commandlet should continue; otherwise, the error code that should be returned by Main()
 */
INT UFindRenamedPrefabSequencesCommandlet::InitializeMapCheck( const TArray<FString>& Tokens, const TArray<FString>& Switches, TArray<FFilename>& MapPathNames )
{
	return Super::InitializeMapCheck(Tokens,Switches,MapPathNames);
}

/**
 * The main worker method - performs the commandlets tests on the package.
 *
 * @param	MapPackage	the current package to be processed
 *
 * @return	0 to indicate that the commandlet should continue; otherwise, the error code that should be returned by Main()
 */
INT UFindRenamedPrefabSequencesCommandlet::CheckMapPackage( UPackage* MapPackage )
{
	INT Result = 0;

	// find all StaticMeshActors and static Light actors which are referenced by something in the map
	UWorld* World = FindObject<UWorld>( MapPackage, TEXT("TheWorld") );
	if ( World )
	{
		// make sure that the world's PersistentLevel is part of the levels array for the purpose of this test.
		World->Levels.AddUniqueItem(World->PersistentLevel);
		for ( INT LevelIndex = 0; LevelIndex < World->Levels.Num(); LevelIndex++ )
		{
			ULevel* Level = World->Levels(LevelIndex);
			if ( Level != NULL )
			{
				USequence* LevelRootSequence = Level->GetGameSequence();
				if ( LevelRootSequence != NULL )
				{
					USequence* PrefabContainerSequence = LevelRootSequence->GetPrefabsSequence(FALSE);
					if ( PrefabContainerSequence != NULL )
					{
						const FString SeqPathName = PrefabContainerSequence->GetPathName();
						warnf(TEXT("Found prefab sequence container: %s"), *SeqPathName);

						if ( PrefabContainerSequence->ObjName != PREFAB_SEQCONTAINER_NAME )
						{
							RenamedPrefabSequenceContainers.AddItem(*SeqPathName);
						}
					}
				}
			}

			TArray<APrefabInstance*> PrefabInstances;
			if ( ContainsObjectOfClass<AActor>(Level->Actors, APrefabInstance::StaticClass(), FALSE, (TArray<AActor*>*)&PrefabInstances) )
			{
				for ( INT PrefIndex = 0; PrefIndex < PrefabInstances.Num(); PrefIndex++ )
				{
					APrefabInstance* Instance = PrefabInstances(PrefIndex);
					if ( Instance != NULL && Instance->SequenceInstance != NULL )
					{
						USequence* ParentSequence = Cast<USequence>(Instance->SequenceInstance->GetOuter());
						if ( ParentSequence != NULL && ParentSequence->ObjName != PREFAB_SEQCONTAINER_NAME )
						{
							const FString SeqPathName = ParentSequence->GetPathName();
							if ( !RenamedPrefabSequenceContainers.HasKey(*SeqPathName) )
							{
								warnf(TEXT("FOUND ORPHAN PREFAB SEQUENCE: %s"), *SeqPathName);
								RenamedPrefabSequenceContainers.AddItem(*SeqPathName);
							}
						}
					}
				}
			}
		}
	}

	return Result;
}

/**
 * Called after all packages have been processed - provides commandlets with an opportunity to print out test results or
 * provide feedback.
 *
 * @return	0 to indicate that the commandlet should continue; otherwise, the error code that should be returned by Main()
 */
INT UFindRenamedPrefabSequencesCommandlet::ProcessResults()
{
	INT Result = 0;

	if ( RenamedPrefabSequenceContainers.Num() > 0 )
	{
		warnf(TEXT("Found %i renamed prefab sequence containers."), RenamedPrefabSequenceContainers.Num());
		for ( INT PrefIndex = 0; PrefIndex < RenamedPrefabSequenceContainers.Num(); PrefIndex++ )
		{
			warnf(TEXT("    %i) %s"), PrefIndex, **RenamedPrefabSequenceContainers(PrefIndex));
		}
	}
	else
	{
		warnf(TEXT("No renamed prefab sequence containers found!"));
	}

	return Result;
}
IMPLEMENT_CLASS(UFindRenamedPrefabSequencesCommandlet);

/* ==========================================================================================================
	UDumpLightmapInfoCommandlet
========================================================================================================== */
INT UDumpLightmapInfoCommandlet::Main( const FString& Params )
{
	TArray<FString> Tokens, Switches, Unused;
	ParseCommandLine(*Params, Tokens, Switches);

	// assume the first token is the map wildcard/pathname
	FString MapWildcard = Tokens.Num() > 0 ? Tokens(0) : FString(TEXT("*.")) + FURL::DefaultMapExt;
	TArray<FFilename> MapNames;
	NormalizePackageNames( Unused, MapNames, MapWildcard, NORMALIZE_ExcludeContentPackages);

	INT GCIndex=0;
	INT TotalMapsChecked=0;

	// Get time as a string
	FString CurrentTime = appSystemTimeString();

	for ( INT MapIndex = 0; MapIndex < MapNames.Num(); MapIndex++ )
	{
		const FFilename& Filename = MapNames(MapIndex);
		warnf( TEXT("Loading  %s...  (%i / %i)"), *Filename, MapIndex, MapNames.Num() );

		// NOTE: This check for autosaves maps assumes that there will never be a map
		// or directory that holds a map with the 'autosaves' sub-string in it!
		if (Filename.GetPath().InStr(TEXT("Autosaves"), FALSE, TRUE) != -1)
		{
			warnf(NAME_Log, TEXT("Skipping autosave map %s"), *Filename);
			continue;
		}

		UPackage* Package = UObject::LoadPackage( NULL, *Filename, LOAD_NoWarn );
		if ( Package == NULL )
		{
			warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
			continue;
		}

		// skip packages in the trashcan or PIE packages
		if ( (Package->PackageFlags&(PKG_Trash|PKG_PlayInEditor)) != 0 )
		{
			warnf(TEXT("Skipping %s (%s)"), *Filename, (Package->PackageFlags&PKG_Trash) != 0 ? TEXT("Trashcan Map") : TEXT("PIE Map"));
			UObject::CollectGarbage(RF_Native);
			continue;
		}

		// Find the world and load all referenced levels.
		UWorld* World = FindObjectChecked<UWorld>( Package, TEXT("TheWorld") );
		if( World )
		{
			AWorldInfo* WorldInfo	= World->GetWorldInfo();
			// Iterate over streaming level objects loading the levels.
			for( INT LevelIndex=0; LevelIndex<WorldInfo->StreamingLevels.Num(); LevelIndex++ )
			{
				ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
				if( StreamingLevel )
				{
					// Load package if found.
					FString Filename;
					if( GPackageFileCache->FindPackageFile( *StreamingLevel->PackageName.ToString(), NULL, Filename ) )
					{
						warnf(NAME_Log, TEXT("\tLoading sub-level %s"), *Filename);
						LoadPackage( NULL, *Filename, LOAD_None );
					}
				}
			}
		}
		else
		{
			warnf( NAME_Error, TEXT("Can't find TheWorld in %s!"), *Filename );
			continue;
		}

		TotalMapsChecked++;

		warnf(TEXT("Checking %s..."), *Filename);

		FString		CSVDirectory	= appGameLogDir() + TEXT("MapLightMapData") PATH_SEPARATOR;
		FString		CSVFilename		= TEXT("");
		FArchive*	CSVFile			= NULL;

		// Create CSV folder in case it doesn't exist yet.
		GFileManager->MakeDirectory( *CSVDirectory );

		// CSV: Human-readable spreadsheet format.
		CSVFilename	= FString::Printf(TEXT("%s%s-%s-%i-%s.csv"), *CSVDirectory, *(Filename.GetBaseFilename()), GGameName, GetChangeListNumberForPerfTesting(), *CurrentTime);
		CSVFile		= GFileManager->CreateFileWriter( *CSVFilename );
		if (CSVFile == NULL)
		{
			warnf(NAME_Log, TEXT("\t\tFailed to create CSV data file... check log for results!"));
		}
		else
		{
			// Write out header row.
			FString HeaderRow = FString::Printf(TEXT("Component,Resource,Lightmap Width, Lightmap Height, Lightmap (kB)%s"),LINE_TERMINATOR);
			CSVFile->Serialize( TCHAR_TO_ANSI( *HeaderRow ), HeaderRow.Len() );
		}

		// Iterate over all primitive components.
		for( TObjectIterator<UPrimitiveComponent> It; It; ++It )
		{
			UPrimitiveComponent*	PrimitiveComponent		= *It;
			UStaticMeshComponent*	StaticMeshComponent		= Cast<UStaticMeshComponent>(*It);
			UModelComponent*		ModelComponent			= Cast<UModelComponent>(*It);
			USkeletalMeshComponent*	SkeletalMeshComponent	= Cast<USkeletalMeshComponent>(*It);
			UTerrainComponent*		TerrainComponent		= Cast<UTerrainComponent>(*It);
			USpeedTreeComponent*	SpeedTreeComponent		= Cast<USpeedTreeComponent>(*It);
			UObject*				Resource				= NULL;
			AActor*					ActorOuter				= Cast<AActor>(PrimitiveComponent->GetOuter());

			// The static mesh is a static mesh component's resource.
			if( StaticMeshComponent )
			{
				Resource = StaticMeshComponent->StaticMesh;
			}
			// A model component is its own resource.
			else if( ModelComponent )			
			{
				// Make sure model component is referenced by level.
				ULevel* Level = CastChecked<ULevel>(ModelComponent->GetOuter());
				if( Level->ModelComponents.FindItemIndex( ModelComponent ) != INDEX_NONE )
				{
					Resource = ModelComponent;
				}
			}
			// The skeletal mesh of a skeletal mesh component is its resource.
			else if( SkeletalMeshComponent )
			{
				Resource = SkeletalMeshComponent->SkeletalMesh;
			}
			// A terrain component's resource is the terrain actor.
			else if( TerrainComponent )
			{
				Resource = TerrainComponent->GetTerrain();
			}
			// The speed tree actor of a speed tree component is its resource.
			else if( SpeedTreeComponent )
			{
				Resource = SpeedTreeComponent->SpeedTree;
			}

			// Dont' care about components without a resource.
			if(	!Resource )
			{
				continue;
			}

			// Require actor association for selection and to disregard mesh emitter components. The exception being model components.
			if (!(ActorOuter || ModelComponent))
			{
				continue;
			}

			// Don't list pending kill components.
			if (PrimitiveComponent->IsPendingKill())
			{
				continue;
			}

			// Figure out memory used by light and shadow maps and light/ shadow map resolution.
			INT LightMapWidth	= 0;
			INT LightMapHeight	= 0;
			PrimitiveComponent->GetLightMapResolution( LightMapWidth, LightMapHeight );
			INT LMSMResolution	= appSqrt( LightMapHeight * LightMapWidth );
			INT LightMapData	= 0;
			INT ShadowMapData	= 0;
			PrimitiveComponent->GetLightAndShadowMapMemoryUsage( LightMapData, ShadowMapData );

			if ((LightMapWidth != 0) && (LightMapHeight != 0))
			{
				warnf(TEXT("%4dx%4d - %s (%s)"),
					LightMapWidth, LightMapHeight, *(PrimitiveComponent->GetName()), *(Resource->GetFullName()));
				if( CSVFile )
				{	
					FString HeaderRow = FString::Printf(TEXT("Component,Resource,Lightmap Width, Lightmap Height, Lightmap (kB)"));
					FString OutString = FString::Printf(TEXT("%s,%s,%d,%d,%d%s"),
						*(PrimitiveComponent->GetName()), *(Resource->GetFullName()),
						LightMapWidth, LightMapHeight, LightMapData, LINE_TERMINATOR);
					CSVFile->Serialize( TCHAR_TO_ANSI( *OutString ), OutString.Len() );
				}
			}
		}

		if (CSVFile)
		{
			// Close and delete archive.
			CSVFile->Close();
			delete CSVFile;
		}

		// collecting garbage every N maps instead of every map makes the commandlet run much faster
		if( (++GCIndex % 1) == 0 )
		{
			UObject::CollectGarbage(RF_Native);
		}
	}

	return 0;
}
IMPLEMENT_CLASS(UDumpLightmapInfoCommandlet);

/* ==========================================================================================================
	UPerformTerrainMaterialDumpCommandlet
========================================================================================================== */
INT UPerformTerrainMaterialDumpCommandlet::Main( const FString& Params )
{
	TArray<FString> Tokens, Switches, Unused;
	ParseCommandLine(*Params, Tokens, Switches);

	// assume the first token is the map wildcard/pathname
	FString MapWildcard = Tokens.Num() > 0 ? Tokens(0) : FString(TEXT("*.")) + FURL::DefaultMapExt;
	TArray<FFilename> MapNames;
	NormalizePackageNames( Unused, MapNames, MapWildcard, NORMALIZE_ExcludeContentPackages);

	INT GCIndex = 0;
	INT TotalMapsChecked = 0;
	for (INT MapIndex = 0; MapIndex < MapNames.Num(); MapIndex++)
	{
		const FFilename& Filename = MapNames(MapIndex);
		warnf( TEXT("Loading  %s...  (%i / %i)"), *Filename, MapIndex, MapNames.Num() );

		UPackage* Package = UObject::LoadPackage( NULL, *Filename, LOAD_NoWarn );
		if ( Package == NULL )
		{
			warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
			continue;
		}

		// skip packages in the trashcan or PIE packages
		if ( (Package->PackageFlags&(PKG_Trash|PKG_PlayInEditor)) != 0 )
		{
			warnf(TEXT("Skipping %s (%s)"), *Filename, (Package->PackageFlags&PKG_Trash) != 0 ? TEXT("Trashcan Map") : TEXT("PIE Map"));
			UObject::CollectGarbage(RF_Native);
			continue;
		}

		TotalMapsChecked++;

		warnf(TEXT("Checking %s..."), *Filename);
		for (TObjectIterator<ATerrain> It; It; ++It )
		{
			ATerrain* Terrain = *It;
			if (Terrain->IsIn(Package) && !Terrain->IsTemplate())
			{
				warnf(TEXT("\tTerrain %s"), *(Terrain->GetFullName()));

				INT ComponentIndex;

				TArray<FTerrainMaterialMask> BatchMaterials;
				TArray<FTerrainMaterialMask> FullMaterials;
				TArray<TArray<INT>> ComponentBatchMaterialPatchCounts;

				ComponentBatchMaterialPatchCounts.AddZeroed(Terrain->TerrainComponents.Num());
				for (ComponentIndex = 0; ComponentIndex < Terrain->TerrainComponents.Num(); ComponentIndex++)
				{
					UTerrainComponent* Comp = Terrain->TerrainComponents(ComponentIndex);
					if (Comp)
					{
						Comp->UpdatePatchBatches();

						TArray<INT>& BatchMaterialPatchCounts = ComponentBatchMaterialPatchCounts(ComponentIndex);

						BatchMaterialPatchCounts.Empty();
						BatchMaterialPatchCounts.AddZeroed(Comp->BatchMaterials.Num());

						FullMaterials.AddUniqueItem(Comp->BatchMaterials(Comp->FullBatch));
						for (INT PatchIndex = 0; PatchIndex < Comp->PatchBatches.Num(); PatchIndex++)
						{
							INT BMatIndex = Comp->PatchBatches(PatchIndex);
							BatchMaterials.AddUniqueItem(Comp->BatchMaterials(BMatIndex));
							BatchMaterialPatchCounts(BMatIndex)++;
						}
					}
				}

				warnf(TEXT("\t\tComponent Count = %d"), Terrain->TerrainComponents.Num());

				UBOOL bIsTerrainMaterialResourceInstance;
				FMaterialRenderProxy* MaterialRenderProxy;
				const FMaterial* Material;

				warnf(TEXT("\t\t\tFullBatches = %d"), FullMaterials.Num());
				for (INT FullIndex = 0; FullIndex < FullMaterials.Num(); FullIndex++)
				{
					MaterialRenderProxy = Terrain->GetCachedMaterial(FullMaterials(FullIndex), bIsTerrainMaterialResourceInstance);
					Material = MaterialRenderProxy ? MaterialRenderProxy->GetMaterial() : NULL;
					warnf(TEXT("\t\t\t\t%s"), Material ? *(Material->GetFriendlyName()) : TEXT("NO MATERIAL"));
				}

				warnf(TEXT("\t\t\tBatches = %d"), BatchMaterials.Num());
				for (INT BatchIndex = 0; BatchIndex < BatchMaterials.Num(); BatchIndex++)
				{
					MaterialRenderProxy = Terrain->GetCachedMaterial(BatchMaterials(BatchIndex), bIsTerrainMaterialResourceInstance);
					Material = MaterialRenderProxy ? MaterialRenderProxy->GetMaterial() : NULL;
					warnf(TEXT("\t\t\t\t%s"), Material ? *(Material->GetFriendlyName()) : TEXT("NO MATERIAL"));
				}

				warnf(TEXT("\t\t\tComponent Batch Usage"));
				for (ComponentIndex = 0; ComponentIndex < Terrain->TerrainComponents.Num(); ComponentIndex++)
				{
					UTerrainComponent* Comp = Terrain->TerrainComponents(ComponentIndex);
					if (Comp)
					{
						warnf(TEXT("\t\t\t\tComponent %d"), ComponentIndex);

						MaterialRenderProxy = Terrain->GetCachedMaterial(Comp->BatchMaterials(Comp->FullBatch), bIsTerrainMaterialResourceInstance);
						Material = MaterialRenderProxy ? MaterialRenderProxy->GetMaterial() : NULL;
						warnf(TEXT("\t\t\t\t\t%4d with FULL BATCH %s"), 
							Comp->GetMaxTriangleCount(), 
							Material ? *(Material->GetFriendlyName()) : TEXT("NO MATERIAL"));

						warnf(TEXT("\t\t\t\t\t*** BATCH SPECIFIC ***"));

						TArray<INT>& BatchMaterialPatchCounts = ComponentBatchMaterialPatchCounts(ComponentIndex);
						for (INT BMPIndex = 0; BMPIndex < BatchMaterialPatchCounts.Num(); BMPIndex++)
						{
							MaterialRenderProxy = Terrain->GetCachedMaterial(BatchMaterials(BMPIndex), bIsTerrainMaterialResourceInstance);
							Material = MaterialRenderProxy ? MaterialRenderProxy->GetMaterial() : NULL;
							warnf(TEXT("\t\t\t\t\t%4d triangles with %s"), 
								BatchMaterialPatchCounts(BMPIndex) * 2,
								Material ? *(Material->GetFriendlyName()) : TEXT("NO MATERIAL"));
						}
					}
				}
			}
		}

		// collecting garbage every 10 maps instead of every map makes the commandlet run much faster
		if( (++GCIndex % 10) == 0 )
		{
			UObject::CollectGarbage(RF_Native);
		}
	}

	return 0;
}

IMPLEMENT_CLASS(UPerformTerrainMaterialDumpCommandlet);

/* ==========================================================================================================
	UListPSysFixedBoundSettingCommandlet
========================================================================================================== */
INT UListPSysFixedBoundSettingCommandlet::Main( const FString& Params )
{
	// Parse command line.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	const TCHAR* Parms = *Params;
	ParseCommandLine(Parms, Tokens, Switches);

	const UBOOL bLoadAllPackages = Switches.FindItemIndex(TEXT("ALL")) != INDEX_NONE;
	TArray<FString> FilesInPath;
	if ( bLoadAllPackages )
	{
		FilesInPath = GPackageFileCache->GetPackageFileList();
	}
	else
	{
		for ( INT i = 0; i < Tokens.Num(); i++ )
		{
			FString	PackageWildcard = Tokens(i);	

			GFileManager->FindFiles( FilesInPath, *PackageWildcard, TRUE, FALSE );
			if( FilesInPath.Num() == 0 )
			{
				// if no files were found, it might be an unqualified path; try prepending the .u output path
				// if one were going to make it so that you could use unqualified paths for package types other
				// than ".u", here is where you would do it
				GFileManager->FindFiles( FilesInPath, *(appScriptOutputDir() * PackageWildcard), 1, 0 );

				if ( FilesInPath.Num() == 0 )
				{
					TArray<FString> Paths;
					if ( GConfig->GetArray( TEXT("Core.System"), TEXT("Paths"), Paths, GEngineIni ) > 0 )
					{
						for ( INT j = 0; j < Paths.Num(); j++ )
						{
							GFileManager->FindFiles( FilesInPath, *(Paths(j) * PackageWildcard), 1, 0 );
						}
					}
				}
				else
				{
					// re-add the path information so that GetPackageLinker finds the correct version of the file.
					FFilename WildcardPath = appScriptOutputDir() * PackageWildcard;
					for ( INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++ )
					{
						FilesInPath(FileIndex) = WildcardPath.GetPath() * FilesInPath(FileIndex);
					}
				}

				// Try finding package in package file cache.
				if ( FilesInPath.Num() == 0 )
				{
					FString Filename;
					if( GPackageFileCache->FindPackageFile( *PackageWildcard, NULL, Filename ) )
					{
						new(FilesInPath)FString(Filename);
					}
				}
			}
			else
			{
				// re-add the path information so that GetPackageLinker finds the correct version of the file.
				FFilename WildcardPath = PackageWildcard;
				for ( INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++ )
				{
					FilesInPath(FileIndex) = WildcardPath.GetPath() * FilesInPath(FileIndex);
				}
			}
		}
	}

	if( FilesInPath.Num() == 0 )
	{
		warnf(NAME_Warning,TEXT("No packages found matching '%s'"), Parms);
		return 1;
	}

	INT GCIndex = 0;
	INT TotalMapsChecked = 0;
	for (INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++)
	{
		const FFilename& Filename = FilesInPath(FileIndex);

		// we don't care about trying to load the various shader caches so just skipz0r them
		if ((Filename.InStr( TEXT("LocalShaderCache") ) != INDEX_NONE) || 
			(Filename.InStr( TEXT("RefShaderCache") ) != INDEX_NONE))
		{
			continue;
		}

		warnf( TEXT("Loading  %s...  (%i / %i)"), *Filename, FileIndex, FilesInPath.Num() );

		UPackage* Package = UObject::LoadPackage( NULL, *Filename, LOAD_NoWarn );
		if ( Package == NULL )
		{
			warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
			continue;
		}

		// skip packages in the trashcan or PIE packages
		if ( (Package->PackageFlags&(PKG_Trash|PKG_PlayInEditor)) != 0 )
		{
			warnf(TEXT("Skipping %s (%s)"), *Filename, (Package->PackageFlags&PKG_Trash) != 0 ? TEXT("Trashcan Map") : TEXT("PIE Map"));
			UObject::CollectGarbage(RF_Native);
			continue;
		}

		TotalMapsChecked++;

		warnf(TEXT("%s"), *Filename);
		for (TObjectIterator<UParticleSystem> It; It; ++It )
		{
			UBOOL bFirstOne = TRUE;
			UParticleSystem* PSys = *It;
			if (PSys->IsIn(Package) && !PSys->IsTemplate())
			{
				warnf(TEXT("\t%s,%s,%f,%f,%f,%f,%f,%f"), *(PSys->GetFullName()), 
					PSys->bUseFixedRelativeBoundingBox ? TEXT("ENABLED") : TEXT("disabled"),
					PSys->FixedRelativeBoundingBox.Max.X,
					PSys->FixedRelativeBoundingBox.Max.Y,
					PSys->FixedRelativeBoundingBox.Max.Z,
					PSys->FixedRelativeBoundingBox.Min.X,
					PSys->FixedRelativeBoundingBox.Min.Y,
					PSys->FixedRelativeBoundingBox.Min.Z);
			}
		}

		// collecting garbage every 10 maps instead of every map makes the commandlet run much faster
		if( (++GCIndex % 10) == 0 )
		{
			UObject::CollectGarbage(RF_Native);
		}
	}

	return 0;
}

IMPLEMENT_CLASS(UListPSysFixedBoundSettingCommandlet);


/* ==========================================================================================================
	UListEmittersUsingModuleCommandlet
========================================================================================================== */
INT UListEmittersUsingModuleCommandlet::Main( const FString& Params )
{
	// Parse command line.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	const TCHAR* Parms = *Params;
	ParseCommandLine(Parms, Tokens, Switches);

	const UBOOL bLoadAllPackages = Switches.FindItemIndex(TEXT("ALL")) != INDEX_NONE;
	TArray<FString> FilesInPath;
	if ( bLoadAllPackages )
	{
		FilesInPath = GPackageFileCache->GetPackageFileList();
	}
	else
	{
		for ( INT i = 0; i < Tokens.Num(); i++ )
		{
			FString	PackageWildcard = Tokens(i);	

			GFileManager->FindFiles( FilesInPath, *PackageWildcard, TRUE, FALSE );
			if( FilesInPath.Num() == 0 )
			{
				// if no files were found, it might be an unqualified path; try prepending the .u output path
				// if one were going to make it so that you could use unqualified paths for package types other
				// than ".u", here is where you would do it
				GFileManager->FindFiles( FilesInPath, *(appScriptOutputDir() * PackageWildcard), 1, 0 );

				if ( FilesInPath.Num() == 0 )
				{
					TArray<FString> Paths;
					if ( GConfig->GetArray( TEXT("Core.System"), TEXT("Paths"), Paths, GEngineIni ) > 0 )
					{
						for ( INT j = 0; j < Paths.Num(); j++ )
						{
							GFileManager->FindFiles( FilesInPath, *(Paths(j) * PackageWildcard), 1, 0 );
						}
					}
				}
				else
				{
					// re-add the path information so that GetPackageLinker finds the correct version of the file.
					FFilename WildcardPath = appScriptOutputDir() * PackageWildcard;
					for ( INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++ )
					{
						FilesInPath(FileIndex) = WildcardPath.GetPath() * FilesInPath(FileIndex);
					}
				}

				// Try finding package in package file cache.
				if ( FilesInPath.Num() == 0 )
				{
					FString Filename;
					if( GPackageFileCache->FindPackageFile( *PackageWildcard, NULL, Filename ) )
					{
						new(FilesInPath)FString(Filename);
					}
				}
			}
			else
			{
				// re-add the path information so that GetPackageLinker finds the correct version of the file.
				FFilename WildcardPath = PackageWildcard;
				for ( INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++ )
				{
					FilesInPath(FileIndex) = WildcardPath.GetPath() * FilesInPath(FileIndex);
				}
			}
		}
	}

	if( FilesInPath.Num() == 0 )
	{
		warnf(NAME_Warning,TEXT("No packages found matching '%s'"), Parms);
		return 1;
	}

	FString RequestedModuleName;
	RequestedModuleName.Empty();
	for (INT ModuleNameIndex = 0; ModuleNameIndex < Switches.Num(); ModuleNameIndex++)
	{
		INT ModuleNameOffset = Switches(ModuleNameIndex).InStr(TEXT("M="));
		if (ModuleNameOffset != -1)
		{
			RequestedModuleName = Switches(ModuleNameIndex);
			RequestedModuleName = RequestedModuleName.Right(RequestedModuleName.Len() - 2);
			break;
		}
	}

	if (RequestedModuleName.Len() == 0)
	{
		return -1;
	}

	FString ModuleName(TEXT("ParticleModule"));
	ModuleName += RequestedModuleName;

	warnf( TEXT("Looking for module %s..."), *ModuleName );

	INT GCIndex = 0;
	INT TotalMapsChecked = 0;
	for (INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++)
	{
		const FFilename& Filename = FilesInPath(FileIndex);

		// we don't care about trying to load the various shader caches so just skipz0r them
		if ((Filename.InStr( TEXT("LocalShaderCache") ) != INDEX_NONE) || 
			(Filename.InStr( TEXT("RefShaderCache") ) != INDEX_NONE))
		{
			continue;
		}

		warnf( TEXT("Loading  %s...  (%i / %i)"), *Filename, FileIndex, FilesInPath.Num() );

		UPackage* Package = UObject::LoadPackage( NULL, *Filename, LOAD_NoWarn );
		if ( Package == NULL )
		{
			warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
			continue;
		}

		// skip packages in the trashcan or PIE packages
		if ( (Package->PackageFlags&(PKG_Trash|PKG_PlayInEditor)) != 0 )
		{
			warnf(TEXT("Skipping %s (%s)"), *Filename, (Package->PackageFlags&PKG_Trash) != 0 ? TEXT("Trashcan Map") : TEXT("PIE Map"));
			UObject::CollectGarbage(RF_Native);
			continue;
		}

		TotalMapsChecked++;

		warnf(TEXT("%s"), *Filename);
		for (TObjectIterator<UParticleSystem> It; It; ++It )
		{
			UBOOL bFirstOne = TRUE;
			UParticleSystem* PSys = *It;
			if (PSys->IsIn(Package) && !PSys->IsTemplate())
			{
				for (INT EmitterIndex = 0; EmitterIndex < PSys->Emitters.Num(); EmitterIndex++)
				{
					UParticleSpriteEmitter* SpriteEmitter = Cast<UParticleSpriteEmitter>(PSys->Emitters(EmitterIndex));
					if (SpriteEmitter)
					{
						if (SpriteEmitter->LODLevels.Num() > 0)
						{
							UParticleLODLevel* LODLevel = SpriteEmitter->LODLevels(0);
							if (LODLevel)
							{
								for (INT ModuleIndex = 0; ModuleIndex < LODLevel->Modules.Num(); ModuleIndex++)
								{
									UParticleModule* Module = LODLevel->Modules(ModuleIndex);
									if (Module && (Module->GetClass()->GetName() == ModuleName))
									{
										warnf(TEXT("\t%s,%d,"), *(PSys->GetFullName()), EmitterIndex);
									}
								}
							}
						}
					}
				}
			}
		}

		// collecting garbage every 10 maps instead of every map makes the commandlet run much faster
		if( (++GCIndex % 10) == 0 )
		{
			UObject::CollectGarbage(RF_Native);
		}
	}

	return 0;
}

IMPLEMENT_CLASS(UListEmittersUsingModuleCommandlet);




//======================================================================
// Commandlet for replacing one kind of actor with another kind of actor, copying changed properties from the most-derived common superclass
IMPLEMENT_CLASS(UReplaceActorCommandlet)

INT UReplaceActorCommandlet::Main(const FString& Params)
{
	const TCHAR* Parms = *Params;

// 	// get the specified filename/wildcard
// 	FString PackageWildcard;
// 	if (!ParseToken(Parms, PackageWildcard, 0))
// 	{
// 		warnf(TEXT("Syntax: replaceactor <file/wildcard> <Package.Class to remove> <Package.Class to replace with>"));
// 		return 1;
// 	}

	// find all the files matching the specified filename/wildcard
// 	TArray<FString> FilesInPath;
// 	GFileManager->FindFiles(FilesInPath, *PackageWildcard, 1, 0);
// 	if (FilesInPath.Num() == 0)
// 	{
// 		warnf(NAME_Error, TEXT("No packages found matching %s!"), *PackageWildcard);
// 		return 2;
// 	}

	// Retrieve list of all packages in .ini paths.
	TArray<FString> PackageList;

	FString PackageWildcard;
	FString PackagePrefix;
// 	if(ParseToken(Parms,PackageWildcard,FALSE))
// 	{
// 		GFileManager->FindFiles(PackageList,*PackageWildcard,TRUE,FALSE);
// 		PackagePrefix = FFilename(PackageWildcard).GetPath() * TEXT("");
// 	}
// 	else
// 	{
		PackageList = GPackageFileCache->GetPackageFileList();
//	}
	if( !PackageList.Num() )
	{
		warnf( TEXT( "Found no packages to fun UReplaceActorCommandlet on!" ) );
		return 0;
	}


	// get the directory part of the filename
	INT ChopPoint = Max(PackageWildcard.InStr(TEXT("/"), 1) + 1, PackageWildcard.InStr(TEXT("\\"), 1) + 1);
	if (ChopPoint < 0)
	{
		ChopPoint = PackageWildcard.InStr( TEXT("*"), 1 );
	}

	FString PathPrefix = (ChopPoint < 0) ? TEXT("") : PackageWildcard.Left(ChopPoint);

	// get the class to remove and the class to replace it with
	FString ClassName;
	if (!ParseToken(Parms, ClassName, 0))
	{
		warnf(TEXT("Syntax: replaceactor <file/wildcard> <Package.Class to remove> <Package.Class to replace with>"));
		return 1;
	}

	UClass* ClassToReplace = (UClass*)StaticLoadObject(UClass::StaticClass(), NULL, *ClassName, NULL, LOAD_NoWarn | LOAD_Quiet, NULL);
	if (ClassToReplace == NULL)
	{
		warnf(NAME_Error, TEXT("Invalid class to remove: %s"), *ClassName);
		return 4;
	}
	else
	{
		ClassToReplace->AddToRoot();
	}

	if (!ParseToken(Parms, ClassName, 0))
	{
		warnf(TEXT("Syntax: replaceactor <file/wildcard> <Package.Class to remove> <Package.Class to replace with>"));
		return 1;
	}

	UClass* ReplaceWithClass = (UClass*)StaticLoadObject(UClass::StaticClass(), NULL, *ClassName, NULL, LOAD_NoWarn | LOAD_Quiet, NULL);
	if (ReplaceWithClass == NULL)
	{
		warnf(NAME_Error, TEXT("Invalid class to replace with: %s"), *ClassName);
		return 5;
	}
	else
	{
		ReplaceWithClass->AddToRoot();
	}

#if HAVE_SCC
	FSourceControlIntegration* SCC = new FSourceControlIntegration;
#endif // HAVE_SCC
	// find the most derived superclass common to both classes
	UClass* CommonSuperclass = NULL;
	for (UClass* BaseClass1 = ClassToReplace; BaseClass1 != NULL && CommonSuperclass == NULL; BaseClass1 = BaseClass1->GetSuperClass())
	{
		for (UClass* BaseClass2 = ReplaceWithClass; BaseClass2 != NULL && CommonSuperclass == NULL; BaseClass2 = BaseClass2->GetSuperClass())
		{
			if (BaseClass1 == BaseClass2)
			{
				CommonSuperclass = BaseClass1;
			}
		}
	}
	checkSlow(CommonSuperclass != NULL);

	for (INT i = 0; i < PackageList.Num(); i++)
	{

		const FString& PackageName = PackageList(i);
		// get the full path name to the file
		FFilename FileName = PathPrefix + PackageName;

		const UBOOL	bIsShaderCacheFile		= FString(*FileName).ToUpper().InStr( TEXT("SHADERCACHE") ) != INDEX_NONE;
		const UBOOL	bIsAutoSave				= FString(*FileName).ToUpper().InStr( TEXT("AUTOSAVES") ) != INDEX_NONE;

		// skip if read-only
// 		if( GFileManager->IsReadOnly(*FileName) )
// 		{
// 			warnf(TEXT("Skipping %s (read-only)"), *FileName);
// 			continue;
// 		}
// 		else 
			if( ( FileName.GetExtension() == TEXT( "u" ) )
				|| ( bIsShaderCacheFile == TRUE )
				|| ( bIsAutoSave == TRUE )
			//|| ( FileName.GetExtension() == TEXT( "upk" ) )
			)
		{
			warnf(TEXT("Skipping %s (non map)"), *FileName);
			continue;
		}
		else
		{
			// clean up any previous world
			if (GWorld != NULL)
			{
				GWorld->CleanupWorld();
				GWorld->RemoveFromRoot();
				GWorld = NULL;
			}

			// load the package
			warnf(TEXT("Loading %s..."), *FileName); 
			UPackage* Package = LoadPackage(NULL, *FileName, LOAD_None);

			// load the world we're interested in
			GWorld = FindObject<UWorld>(Package, TEXT("TheWorld"));

			// this is the case where .upk objects have class references (e.g. prefabs, animnodes, etc)
			if( GWorld == NULL )
			{
				warnf(TEXT("%s (not a map)"), *FileName);
				for( FObjectIterator It; It; ++It )
				{
					UObject* OldObject = *It;
					if( ( OldObject->GetOutermost() == Package )
						)
					{
						TMap<UClass*, UClass*> ReplaceMap;
						ReplaceMap.Set(ClassToReplace, ReplaceWithClass);
						FArchiveReplaceObjectRef<UClass> ReplaceAr(OldObject, ReplaceMap, FALSE, FALSE, FALSE);
						if( ReplaceAr.GetCount() > 0 )
						{
							warnf(TEXT("Replaced %i class references in an Object: %s"), ReplaceAr.GetCount(), *OldObject->GetName() );
							Package->MarkPackageDirty();
						}
					}
				}

				if( Package->IsDirty() == TRUE )
				{
					const UBOOL bAutoCheckOut = TRUE; //ParseParam(appCmdLine(),TEXT("AutoCheckOutPackages"));
					if( (GFileManager->IsReadOnly(*FileName)) && ( bAutoCheckOut == TRUE ) )
					{
#if HAVE_SCC
						SCC->CheckOut( Package );
#endif // HAVE_SCC
					}

					warnf(TEXT("Saving %s..."), *FileName);
					SavePackage( Package, NULL, RF_Standalone, *FileName, GWarn );
				}
			}
			else
			{
				// need to have a bool so we dont' save every single map
				UBOOL bIsDirty = FALSE;

				// add the world to the root set so that the garbage collection to delete replaced actors doesn't garbage collect the whole world
				GWorld->AddToRoot();
				// initialize the levels in the world
				GWorld->Init();
				GWorld->GetWorldInfo()->PostEditChange(NULL);

				// iterate through all the actors in the world, looking for matches with the class to replace (must have exact match, not subclass)
				for (FActorIterator It; It; ++It)
				{
					AActor* OldActor = *It;
					if (OldActor->GetClass() == ClassToReplace)
					{
						// replace an instance of the old actor
						warnf(TEXT("Replacing actor %s"), *OldActor->GetName());
						// make sure we spawn the new actor in the same level as the old
						//@warning: this relies on the outer of an actor being the level
						GWorld->CurrentLevel = OldActor->GetLevel();
						checkSlow(GWorld->CurrentLevel != NULL);
						// spawn the new actor
						AActor* NewActor = GWorld->SpawnActor(ReplaceWithClass, NAME_None, OldActor->Location, OldActor->Rotation, NULL, TRUE);
						// copy non-native non-transient properties common to both that were modified in the old actor to the new actor
						for (UProperty* Property = CommonSuperclass->PropertyLink; Property != NULL; Property = Property->PropertyLinkNext)
						{
							//@note: skipping properties containing components - don't have a reasonable way to deal with them and the instancing mess they create
							// to  hack around this for now you can do:
							// (!(Property->PropertyFlags & CPF_Component) || Property->GetFName() == FName(TEXT("Weapons"))) &&
							if ( !(Property->PropertyFlags & CPF_Native) && !(Property->PropertyFlags & CPF_Transient) &&
								!(Property->PropertyFlags & CPF_Component) &&
								!Property->Identical((BYTE*)OldActor + Property->Offset, (BYTE*)OldActor->GetClass()->GetDefaultObject() + Property->Offset) )
							{
								Property->CopyCompleteValue((BYTE*)NewActor + Property->Offset, (BYTE*)OldActor + Property->Offset);
								Package->MarkPackageDirty();
								bIsDirty = TRUE;
							}
						}

						// destroy the old actor
						//@warning: must do this before replacement so the new Actor doesn't get the old Actor's entry in the level's actor list (which would cause it to be in there twice)
						GWorld->DestroyActor(OldActor);
						check(OldActor->IsValid()); // make sure DestroyActor() doesn't immediately trigger GC since that'll break the reference replacement
						// check for any references to the old Actor and replace them with the new one
						TMap<AActor*, AActor*> ReplaceMap;
						ReplaceMap.Set(OldActor, NewActor);
						FArchiveReplaceObjectRef<AActor> ReplaceAr(GWorld, ReplaceMap, FALSE, FALSE, FALSE);
						if (ReplaceAr.GetCount() > 0)
						{
							warnf(TEXT("Replaced %i actor references in %s"), ReplaceAr.GetCount(), *It->GetName());
							Package->MarkPackageDirty();
							bIsDirty = TRUE;
						}
					}
					else
					{
						// check for any references to the old class and replace them with the new one
						TMap<UClass*, UClass*> ReplaceMap;
						ReplaceMap.Set(ClassToReplace, ReplaceWithClass);
						FArchiveReplaceObjectRef<UClass> ReplaceAr(*It, ReplaceMap, FALSE, FALSE, FALSE);
						if (ReplaceAr.GetCount() > 0)
						{
							warnf(TEXT("Replaced %i class references in actor %s"), ReplaceAr.GetCount(), *It->GetName());
							Package->MarkPackageDirty();
							bIsDirty = TRUE;
						}
					}
				}


				// replace Kismet references to the class
				USequence* Sequence = GWorld->GetGameSequence();
				if (Sequence != NULL)
				{
					TMap<UClass*, UClass*> ReplaceMap;
					ReplaceMap.Set(ClassToReplace, ReplaceWithClass);
					FArchiveReplaceObjectRef<UClass> ReplaceAr(Sequence, ReplaceMap, FALSE, FALSE, FALSE);
					if (ReplaceAr.GetCount() > 0)
					{
						warnf(TEXT("Replaced %i class references in Kismet"), ReplaceAr.GetCount());
						Package->MarkPackageDirty();
						bIsDirty = TRUE;
					}
				}

				// collect garbage to delete replaced actors and any objects only referenced by them (components, etc)
				GWorld->PerformGarbageCollection();

				// save the world
				if( ( Package->IsDirty() == TRUE ) && ( bIsDirty == TRUE ) )
				{
					const UBOOL bAutoCheckOut = TRUE; //ParseParam(appCmdLine(),TEXT("AutoCheckOutPackages"));
					if( (GFileManager->IsReadOnly(*FileName)) && ( bAutoCheckOut == TRUE ) )
					{
#if HAVE_SCC
						SCC->CheckOut( Package );
#endif // HAVE_SCC
					}

					warnf(TEXT("Saving %s..."), *FileName);
					SavePackage(Package, GWorld, 0, *FileName, GWarn);
				}

				// clear GWorld by removing it from the root set and replacing it with a new one
				GWorld->CleanupWorld();
				GWorld->RemoveFromRoot();
				GWorld = NULL;
			}
		}

		// get rid of the loaded world
		warnf(TEXT("GCing..."));
		CollectGarbage(RF_Native);
	}

	// UEditorEngine::FinishDestroy() expects GWorld to exist
	UWorld::CreateNew();
#if HAVE_SCC
	delete SCC; // clean up our allocated SCC
#endif // HAVE_SCC

	return 0;
}

/**
 *	ListSoundNodeWaves
 */
struct FListSoundNodeWaves_Entry
{
	FString Name;
	FString Path;
	INT Length;
};

IMPLEMENT_COMPARE_CONSTREF(FListSoundNodeWaves_Entry,UnPackageUtilities,{ return A.Length < B.Length ? 1 : -1; });

INT UListSoundNodeWavesCommandlet::Main( const FString& Params )
{
	TArray<FListSoundNodeWaves_Entry> NodeWaveList;
	INT TooLongCount = 0;

	TArray<FString> Tokens, Switches;
	ParseCommandLine(*Params, Tokens, Switches);

	const UBOOL bLoadAllPackages = Switches.FindItemIndex(TEXT("ALL")) != INDEX_NONE;
	TArray<FString> FilesInPath;
	if ( bLoadAllPackages )
	{
		FilesInPath = GPackageFileCache->GetPackageFileList();
	}
	else
	{
		for ( INT TokenIndex = 0; TokenIndex < Tokens.Num(); TokenIndex++ )
		{
			FString& PackageWildcard = Tokens(TokenIndex);

			GFileManager->FindFiles( FilesInPath, *PackageWildcard, TRUE, FALSE );
			if( FilesInPath.Num() == 0 )
			{
				// if no files were found, it might be an unqualified path; try prepending the .u output path
				// if one were going to make it so that you could use unqualified paths for package types other
				// than ".u", here is where you would do it
				GFileManager->FindFiles( FilesInPath, *(appScriptOutputDir() * PackageWildcard), 1, 0 );

				if ( FilesInPath.Num() == 0 )
				{
					TArray<FString> Paths;
					if ( GConfig->GetArray( TEXT("Core.System"), TEXT("Paths"), Paths, GEngineIni ) > 0 )
					{
						for ( INT i = 0; i < Paths.Num(); i++ )
						{
							GFileManager->FindFiles( FilesInPath, *(Paths(i) * PackageWildcard), 1, 0 );
						}
					}
				}
				else
				{
					// re-add the path information so that GetPackageLinker finds the correct version of the file.
					FFilename WildcardPath = appScriptOutputDir() * PackageWildcard;
					for ( INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++ )
					{
						FilesInPath(FileIndex) = WildcardPath.GetPath() * FilesInPath(FileIndex);
					}
				}

				// Try finding package in package file cache.
				if ( FilesInPath.Num() == 0 )
				{
					FString Filename;
					if( GPackageFileCache->FindPackageFile( *PackageWildcard, NULL, Filename ) )
					{
						new(FilesInPath)FString(Filename);
					}
				}
			}
			else
			{
				// re-add the path information so that GetPackageLinker finds the correct version of the file.
				FFilename WildcardPath = PackageWildcard;
				for ( INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++ )
				{
					FilesInPath(FileIndex) = WildcardPath.GetPath() * FilesInPath(FileIndex);
				}
			}

			if ( FilesInPath.Num() == 0 )
			{
				warnf(TEXT("No packages found using '%s'!"), *PackageWildcard);
				continue;
			}
		}
	}

	for( INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++ )
	{
		const FString &Filename = FilesInPath(FileIndex);

		{
			// reset the loaders for the packages we want to load so that we don't find the wrong version of the file
			// (otherwise, attempting to run pkginfo on e.g. Engine.xxx will always return results for Engine.u instead)
			const FString& PackageName = FPackageFileCache::PackageFromPath(*Filename);
			UPackage* ExistingPackage = FindObject<UPackage>(NULL, *PackageName, TRUE);
			if ( ExistingPackage != NULL )
			{
				ResetLoaders(ExistingPackage);
			}
		}

		UPackage* Package = UObject::LoadPackage( NULL, *Filename, LOAD_NoWarn );
		if ( Package == NULL )
		{
			warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
			continue;
		}

		// skip packages in the trashcan or PIE packages
		UBOOL bIsAMapPackage = FindObject<UWorld>(Package, TEXT("TheWorld")) != NULL;
		if ( bIsAMapPackage || ((Package->PackageFlags&(PKG_Trash|PKG_PlayInEditor)) != 0 ))
		{
			warnf(TEXT("Skipping %s (%s)"), *Filename, (Package->PackageFlags&PKG_Trash) != 0 ? TEXT("Trashcan Map") : TEXT("PIE Map"));
			UObject::CollectGarbage(RF_Native);
			continue;
		}

		warnf(TEXT("%s"), *Filename);
		for (TObjectIterator<USoundNodeWave> It; It; ++It )
		{
			UBOOL bFirstOne = TRUE;
			USoundNodeWave* SndNodeWave = *It;
			if (SndNodeWave->IsIn(Package) && !SndNodeWave->IsTemplate())
			{
				GWarn->Logf( TEXT("\t%3d - %s"), SndNodeWave->GetName().Len(), *(SndNodeWave->GetName()) );

				FListSoundNodeWaves_Entry* NewEntry = new(NodeWaveList)FListSoundNodeWaves_Entry;
				check(NewEntry);
				NewEntry->Length = SndNodeWave->GetName().Len();
				NewEntry->Name = SndNodeWave->GetName();
				NewEntry->Path = SndNodeWave->GetPathName();

				if (NewEntry->Length > 27)
				{
					TooLongCount++;
				}
			}
		}

		UObject::CollectGarbage(RF_Native);
	}

	if (NodeWaveList.Num() > 0)
	{
		debugf(TEXT("Dumping all SoundNodeWaves"));
		debugf(TEXT("\t%4d out of %4d are too long!"), TooLongCount, NodeWaveList.Num());

		Sort<USE_COMPARE_CONSTREF(FListSoundNodeWaves_Entry,UnPackageUtilities)>(&(NodeWaveList(0)),NodeWaveList.Num());
		for (INT Index = 0; Index < NodeWaveList.Num(); Index++)
		{
			debugf(TEXT("\t%3d,%s,%s"), 
				NodeWaveList(Index).Length,
				*(NodeWaveList(Index).Name),
				*(NodeWaveList(Index).Path));
		}
	}
	else
	{
		warnf(TEXT("No sound node waves found!"));
	}
	return 0;
}
IMPLEMENT_CLASS(UListSoundNodeWavesCommandlet);
