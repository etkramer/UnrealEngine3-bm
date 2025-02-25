/*=============================================================================
	DiffPackagesCommandlet.cpp: Commandlet used for comparing two packages.

	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"

// the maximum number of packages that can be compared
#define MAX_PACKAGECOUNT 3

// whether to serialize object recursively when looking for changes (for debugging)
#define USE_DEEP_RECURSION 0

// whether to skip levels when building the initial diff sets (for debugging)
#define OPTIMIZE_LEVEL_DIFFS 1


static const TCHAR* GetDiffTypeText( EObjectDiff DiffType, const INT NumPackages )
{
	if ( NumPackages == 2 )
	{
		switch( DiffType )
		{
		case OD_None:
			return TEXT("None");

		case OD_AOnly:
			return TEXT("A only");

		case OD_BOnly:
			return TEXT("B only");

		case OD_ABSame:
			return TEXT("Identical");

		case OD_ABConflict:
			return TEXT("Changed");

		case OD_Invalid:
			return TEXT("Invalid");

		default:
			return TEXT("Unknown");
		}
	}
	else
	{
		switch( DiffType )
		{
		case OD_None:
			return TEXT("None");

		case OD_AOnly:
			return TEXT("A Only");

		case OD_BOnly:
			return TEXT("B Only");

		case OD_ABSame:
			return TEXT("Both (resolved)");

		case OD_ABConflict:
			return TEXT("Both (conflict)");

		case OD_Invalid:
			return TEXT("Invalid");

		default:
			return TEXT("Unknown");
		}
	}
}


/**
 * Contains the results for a comparison between two values of a single property.
 */
struct FPropertyComparison
{
	/** Constructor */
	FPropertyComparison()
	: Prop(NULL), DiffType(OD_None)
	{}

	/** the property that was compared */
	UProperty* Prop;

	/**
	 * The comparison result type for this property comparison.
	 */
	EObjectDiff DiffType;

	/** The name of the property that was compared; only used when comparing native property data (which will have no corresponding UProperty) */
	FString PropText;

	/**
	 * Contains the result of the comparison.
	 */
	FString DiffText;
};

/**
 * Contains information about a comparison of the property values for two object graphs.  One FObjectComparison is created for each
 * top-level object in a package (i.e. each object that has the package's LinkerRoot as its Outer), which contains comparison data for
 * the top-level object as well as its subobjects.
 */
struct FObjectComparison
{
	/** Constructor */
	FObjectComparison()
	: OverallDiffType(OD_None)
	{
		appMemzero(ObjectSets, sizeof(ObjectSets));
	}

	/**
	 * The path name for the top-level object in this FObjectComparison, minus the package portion of the path name.
	 */
	FString RootObjectPath;

	/**
	 * The graph of objects represented by this comparison from each package.  The graphs contain the top-level object along with
	 * all of its subobjects.
	 */
	FObjectGraph* ObjectSets[MAX_PACKAGECOUNT];

	/**
	 * The list of comparison results for all property values which not identical in all packages.
	 */
	TArray<FPropertyComparison> PropDiffs;

	/**
	 * The cumulative comparison result type for the entire object graph comparison.
	 */
	EObjectDiff OverallDiffType;
};

/**
 * Constructor
 *
 * Populates the PropertyData and PropertyText members if associated with a valid UObject
 */
FNativePropertyData::FNativePropertyData( UObject* InObject )
: Object(InObject)
{
	SetObject(InObject);
}

/**
 * Changes the UObject associated with this native property data container and re-initializes the
 * PropertyData and PropertyText members
 */
void FNativePropertyData::SetObject( UObject* NewObject )
{
	Object = NewObject;
	PropertyData.Empty();
	PropertyText.Empty();

	if ( Object != NULL )
	{
		UDiffPackagesCommandlet::LoadNativePropertyData(Object, PropertyData);
		Object->GetNativePropertyValues(PropertyText, PPF_SimpleObjectText);
	}
}

/**
 * Constructor
 *
 * Populates the Objects array with RootObject and its subobjects.
 *
 * @param	RootObject			the top-level object for this object graph
 * @param	PackageIndex		the index [into the Packages array] for the package that this object graph belongs to
 * @param	ObjectsToIgnore		optional list of objects to not include in this object graph, even if they are contained within RootObject
 */
FObjectGraph::FObjectGraph( UObject* RootObject, INT PackageIndex, TArray<FObjectComparison>* pObjectsToIgnore/*=NULL*/ )
{
	new(Objects) FObjectReference(RootObject);

	// start with just looking in the root object, but collect references on everything
	// that is put in to objects, etc
	for (INT ObjIndex = 0; ObjIndex < Objects.Num(); ObjIndex++)
	{
		FObjectReference& ObjSet = Objects(ObjIndex);

		// find all objects inside this object that are referenced by properties in the object
		TArray<UObject*> Subobjects;

		// if we want to ignore certain objects, pre-fill the Subobjects array with the list
		if ( pObjectsToIgnore != NULL )
		{
			TArray<FObjectComparison>& ObjectsToIgnore = *pObjectsToIgnore;
			for ( INT IgnoreIndex = 0; IgnoreIndex < ObjectsToIgnore.Num(); IgnoreIndex++ )
			{
				FObjectGraph* IgnoreGraph = ObjectsToIgnore(IgnoreIndex).ObjectSets[PackageIndex];
				if ( IgnoreGraph != NULL )
				{
					Subobjects.AddUniqueItem(IgnoreGraph->GetRootObject());
					/*
					adding the root object *should* be sufficient (as theoretically, the only object that should have a reference to its subobjects
					is that object....but if that doesn't work correctly, we'll add all of the objects in the graph
					for ( INT GraphIndex = 1; GraphIndex < IgnoreGraph->Objects.Num(); GraphIndex++ )
					{
						Subobjects.AddUniqueItem(IgnoreGraph->Objects(GraphIndex).Object);
					}
					*/
				}
			}
		}

		const INT StartIndex = Subobjects.Num();
		FArchiveObjectReferenceCollector ObjectReferenceCollector(&Subobjects, ObjSet.Object, TRUE, FALSE, USE_DEEP_RECURSION);
		ObjSet.Object->Serialize(ObjectReferenceCollector);

		// add all the newly serialized objects to the object set
		for (INT Index = StartIndex; Index < Subobjects.Num(); Index++)
		{
			new(Objects) FObjectReference(Subobjects(Index));
		}
	}
}

/**
 * Generates a simulated path name for the specified object, replacing the name of this object's actual outer-most with the specified package name.
 *
 * @param	Object				the object to generate a simulated path name for
 * @param	OtherPackageName	the package name to use in the simulated path name
 *
 * @todo ronp - optimize this by passing in the actual package instead of the package's name, then use Object->GetPathName(Object->GetOutermost())
 * to generate a path name which doesn't include the outermost package's name.
 */
FString MakeMatchingPathName(UObject* Object, const TCHAR* OtherPackageName)
{
	// turn SourceObject's pathname into the pathname for the other object
	FString ObjPath = Object->GetPathName();
	return FString::Printf(TEXT("%s%s"), OtherPackageName, *ObjPath.Right(ObjPath.Len() - ObjPath.InStr(TEXT("."))));
}

/**
 * Finds the counterpart object from the specified object set.  ObjectSet should be a list of objects from a different package than SourceObject is contained in.
 *
 * @param	SourceObject	the object to find the counterpart for
 * @param	PackageName		the name of the package which contains the objects in ObjectSet
 * @param	ObjectSet		the set of objects to search for SourceObject's counterpart; should be an object set from a different package than SourceObject.
 *
 * @return	a pointer to the counterpart object from another package (specified by the ObjectSet) for SourceObject, or NULL if the ObjectSet doesn't contain
 *			a counterpart to SourceObject.
 *
 * @todo ronp - optimize this by passing in the actual package instead of the package's name
 */
UObject* FindMatchingObjectInObjectSet(UObject* SourceObject, const TCHAR* PackageName, FObjectGraph* ObjectSet)
{
	UObject* Result = NULL;

	// can't look in a NULL objectset!
	if (ObjectSet != NULL)
	{
		// generate the path name that SourceObject would have if it was in the package represented by ObjectSet.
		FString ObjPath = MakeMatchingPathName(SourceObject, PackageName);

		// the first object in an object set is always the package root, so we skip that object.
		for (INT Index = 1; Index < ObjectSet->Objects.Num(); Index++)
		{
			// does this object match?
			if (ObjPath == ObjectSet->Objects(Index).ObjectPathName)
			{
				// found it!
				Result = ObjectSet->Objects(Index).Object;
				break;
			}
		}
	}

	return Result;
}

/**
 * Searches for an object contained within the package which has the specified name, which is the counterpart to SourceObject.
 *
 * @param	SourceObject	the object to search for a counterpart for
 * @param	PackageName		the name of the package to search for SourceObject's counterpart in
 *
 * @return	a pointer to the object from the specified package which is the counterpart for SourceObject, or NULL if the there is no
 *			counterpart for SourceObject in the specified package.
 */
UObject* FindMatchingObject(UObject* SourceObject, const TCHAR* PackageName)
{
	FString ObjPath = MakeMatchingPathName(SourceObject, PackageName);
	UObject* OtherObject = UObject::StaticFindObject(SourceObject->GetClass(), NULL, *ObjPath, TRUE);

	return OtherObject;
}

/**
 * Copies an object into the given package. The new object will have the same
 * group hierarchy as the original object.
 *
 * @param Package	The package to copy the object in to
 * @param Object	The obhject to copy over.
 *
 * @return The newly copied object
 */
UObject* CopyObjectToPackage(UPackage* Package, UObject* Object)
{
	// if there was no outer, this is the top level package, which we don't weant to copy
	if (Object->GetOuter() == NULL)
	{
		return NULL;
	}

	UWorld* World = FindObject<UWorld>(Package, TEXT("TheWorld"));

	// get the pathname of our outer object
	FString OrigPathName = Object->GetOuter()->GetPathName();
	UObject* NewOuter = NULL;

	// we are going to replace the outermost package name of the object with the destination package name
	INT Dot = OrigPathName.InStr(TEXT("."));

	// if there was no dot, then our outer was the apckage, in which case our new will just be the package name
	if (Dot == -1)
	{
		NewOuter = Package;
	}
	else if (Object->GetOuter()->IsA(UPackage::StaticClass()))
	{
		// otherwise, we need to possibly create the whole package group hierarchy
		
		// cretate the new pathname from package name and everything after the original package name
		FString NewPathName = FString(*Package->GetName()) + OrigPathName.Right(OrigPathName.Len() - Dot);
		NewOuter = UObject::CreatePackage(NULL, *NewPathName);
	}
	else
	{
		// the last case is when we are in another object that's not a package. find the
		// corresponding outer in the new package
		// @todo: this is order-dependent, the outer must be copied over first!
		NewOuter = FindMatchingObject(Object->GetOuter(), *Package->GetName());

		if (!NewOuter)
		{
			appErrorf(TEXT("'%s's outer hasn't been copied yet!"), *Object->GetFullName());
		}
	}

	//@todo ronp - replace this with StaticDuplicateObject, which does the same thing...better
	// serialize out the original object
	TArray<BYTE> Bytes;
	FObjectWriter(Object, Bytes);

	// make a new object
	UObject* NewObject = UObject::StaticConstructObject(Object->GetClass(), NewOuter, Object->GetFName(), Object->GetFlags(), Object->GetArchetype(), GError, Object);

	// serialize old objects on top of the new object
	FObjectReader Reader(NewObject, Bytes);

	return NewObject;
}


/**
 * Any properties in any object in Package that point to a key in the ObjectReplacementMap
 * will be replaced with the key in the map.
 * This is so that when we copy objects into the merged package, the refs in the 
 * merged package will be fixed up to point to objects inside the merged package.
 */
void FixupObjectReferences(UPackage* Package, TMap<UObject*, UObject*>& ObjectReplacementMap)
{
	for (FObjectIterator It; It; ++It)
	{
		if (It->IsIn(Package))
		{
			FArchiveReplaceObjectRef<UObject> ReplaceAr(*It, ObjectReplacementMap, TRUE, FALSE, FALSE);
		}
	}
}

/**
 * Generates object graphs for the specified object and its corresponding objects in all packages being diffed.
 *
 * @param	RootObject			the object to generate the object comparison for
 * @param	out_Comparison		the object graphs for the specified object for each package being diffed
 * @param	ObjectsToIgnore		if specified, this list will be passed to the FObjectGraphs created for this comparison; this will prevent those object graphs from containing
 *								these objects.  Useful when generating an object comparison for package-root type objects, such as levels, worlds, etc. to prevent their comparison's
 *								object graphs from containing all objects in the level/world
 *
 * @return	TRUE if RootObject was found in any of the packages being compared.
 */
UBOOL UDiffPackagesCommandlet::GenerateObjectComparison( UObject* RootObject, FObjectComparison& out_Comparison, TArray<FObjectComparison>* ObjectsToIgnore/*=NULL*/ )
{
	check(RootObject);

	UBOOL bFound = FALSE;

	// mark that it's been put into a diff
	RootObject->SetFlags(RF_Marked);

	// the packages that we need to find a matching object in
	UBOOL bNeedsObjectMatch[MAX_PACKAGECOUNT] = {TRUE, TRUE, TRUE};

	// put the object and all its subobjects into the proper list
	for (INT PackageIndex = 0; PackageIndex < NumPackages; PackageIndex++)
	{
		if ( RootObject->IsIn(Packages[PackageIndex]))
		{
			bNeedsObjectMatch[PackageIndex] = FALSE;
			bFound = TRUE;

			FObjectGraph* NewObjectSet = new FObjectGraph(RootObject, PackageIndex, ObjectsToIgnore);
			out_Comparison.ObjectSets[PackageIndex] = NewObjectSet;

			// get the name of the object without the package name
			out_Comparison.RootObjectPath = RootObject->GetPathName(Packages[PackageIndex]);
			break;
		}
	}

	// if this object isn't in any of the packages, then skip it
	if ( bFound )
	{
		// find a matching object set in the other 2
		for (INT PackageIndex = 0; PackageIndex < NumPackages; PackageIndex++)
		{
			if ( bNeedsObjectMatch[PackageIndex] )
			{
				// look for the root object in this package
				UObject* MatchingObject = FindMatchingObject(RootObject, *Packages[PackageIndex]->GetName());
				if ( MatchingObject )
				{
					// mark that it's been put into a diff
					MatchingObject->SetFlags(RF_Marked);

					// make the object set for this object
					FObjectGraph* NewObjectSet = new FObjectGraph(MatchingObject, PackageIndex, ObjectsToIgnore);
					out_Comparison.ObjectSets[PackageIndex] = NewObjectSet;
				}
			}
		}
	}

	return bFound;
}

IMPLEMENT_COMPARE_CONSTREF( FObjectComparison, DiffPackagesCommandlet, { return appStricmp(*A.RootObjectPath,*B.RootObjectPath); } )

/**
 * Parses the command-line and loads the packages being compared.
 *
 * @param	Parms	the full command-line used to invoke this commandlet
 *
 * @return	TRUE if all parameters were parsed successfully; FALSE if any of the specified packages couldn't be loaded
 *			or the parameters were otherwise invalid.
 */
UBOOL UDiffPackagesCommandlet::Initialize( const TCHAR* Parms )
{
	UBOOL bResult = FALSE;

	// disable instancing of new components and subobjects so that we don't get lots of false positives
	GUglyHackFlags |= (HACK_DisableComponentCreation|HACK_DisableSubobjectInstancing);

	// parse the command line into tokens and switches
	TArray<FString> Tokens, Switches;
	ParseCommandLine(Parms, Tokens, Switches);

	// if a merge package is specified, the pathname for the destination package
 	UPackage* MergePackage = NULL;

	// find the package files that should be diffed - doesn't need to be a valid package path (i.e. can be a package located in a tmp directory or something)
	for ( INT TokenIndex = 0; TokenIndex < Tokens.Num(); TokenIndex++ )
	{
		TArray<FFilename> FilesInPath;

		UBOOL bMergePackage = FALSE, bAncestorPackage = FALSE, bFirstPackage=FALSE, bSecondPackage=FALSE;
		FString	PackageWildcard = Tokens(TokenIndex);
		if ( PackageWildcard.InStr(TEXT("=")) != INDEX_NONE )
		{
			FString ParsedFilename;
			if ( Parse(*PackageWildcard, TEXT("MERGE="), ParsedFilename) )
			{
				bMergePackage = TRUE;
			}
			// look for a common ancestor setting
			else if ( Parse(*PackageWildcard, TEXT("ANCESTOR="), ParsedFilename))
			{
				bAncestorPackage = TRUE;
			}
			PackageWildcard = ParsedFilename;
		}
		else
		{
			if ( Packages[0] == NULL )
			{
				bFirstPackage = TRUE;
			}
			else if ( Packages[1] == NULL )
			{
				bSecondPackage = TRUE;
			}
			else
			{
				SET_WARN_COLOR(COLOR_RED);
				warnf(NAME_Error, TEXT("Too many packages specified (only two allowed)!  Use '%sGame help DiffPackagesCommandlet' to view correct usage syntax for this commandlet."), appGetGameName());
				CLEAR_WARN_COLOR();
				bResult = FALSE;
				break;
			}
		}

		if ( PackageWildcard.Len() == 0 )
		{
			SET_WARN_COLOR(COLOR_RED);
			warnf(NAME_Error, TEXT("No package specified for parameter %i: %s.  Use '%sGame help DiffPackagesCommandlet' to view correct usage syntax for this commandlet."),
				TokenIndex, *Tokens(TokenIndex), appGetGameName());

			CLEAR_WARN_COLOR();
			bResult = FALSE;
			break;
		}

		GFileManager->FindFiles( (TArray<FString>&)FilesInPath, *PackageWildcard, TRUE, FALSE );
		if( FilesInPath.Num() == 0 )
		{
			// if no files were found, it might be an unqualified path; try prepending the .u output path
			// if one were going to make it so that you could use unqualified paths for package types other
			// than ".u", here is where you would do it
			GFileManager->FindFiles( (TArray<FString>&)FilesInPath, *(appScriptOutputDir() * PackageWildcard), 1, 0 );
			if ( FilesInPath.Num() == 0 )
			{
				// if no files were found in the script directory, search all valid package paths
				TArray<FString> Paths;
				if ( GConfig->GetArray( TEXT("Core.System"), TEXT("Paths"), Paths, GEngineIni ) > 0 )
				{
					for ( INT i = 0; i < Paths.Num(); i++ )
					{
						GFileManager->FindFiles( (TArray<FString>&)FilesInPath, *(Paths(i) * PackageWildcard), 1, 0 );
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

		if ( bMergePackage )
		{
			// @todo ronp - enable merging
			//@{
			SET_WARN_COLOR(COLOR_RED);
			warnf(NAME_Error, TEXT("Merging is not currently supported"));
			bResult = FALSE;
			break;
			//@}

			if ( FilesInPath.Num() > 0 )
			{
				// this means that the location/package name that the user specified is an existing file...
				// eventually we'll probably allow this but for simplicity's sake, at this point it's an error
				SET_WARN_COLOR(COLOR_RED);
				warnf(NAME_Error, TEXT("Merge target file already exists: %s.  Overwriting an existing file via merging is now allowed!"), *FilesInPath(0));
				bResult = FALSE;
				break;
			}

			MergePackage = UObject::CreatePackage(NULL, TEXT("MergePackage"));
			// diff all properties if we are merging ?
			// todo: ??
			//		bDiffAllProps = TRUE;
			//		bDiffNonEditProps = TRUE;
			SET_WARN_COLOR_AND_BACKGROUND(COLOR_RED, COLOR_WHITE);
			warnf(TEXT("\n"));
			warnf(TEXT("=============================================================================="));
			warnf(TEXT("WARNING: Merge functionality is not finished! (It only copies from Package 1!)"));
			warnf(TEXT("=============================================================================="));
			warnf(TEXT("\n"));
			CLEAR_WARN_COLOR();
			//@todo - validation
		}
		else
		{
			// because of the nature of this commandlet, each parameter should correspond to exactly one package - 
			if ( FilesInPath.Num() == 1 )
			{
				const FFilename& Filename = FilesInPath(0);
				SET_WARN_COLOR(COLOR_DARK_RED);
				warnf( TEXT("Loading %s"), *Filename );
				CLEAR_WARN_COLOR();

				UPackage* Package = NULL;
				if ( bFirstPackage )
				{
					check(Packages[0] == NULL);

					// to avoid conflicts when loading packages from different locations that have the same name, create a dummy package to contain
					// the file we're about the load - this will prevent the second/third versions of the file from replacing the first version when loaded.
					Package = CreatePackage(NULL, TEXT("Package_A"));
					Packages[0] = LoadPackage(Package, *Filename, LOAD_None);
					NumPackages++;
				}
				else if ( bSecondPackage )
				{
					check(Packages[1] == NULL);

					// to avoid conflicts when loading packages from different locations that have the same name, create a dummy package to contain
					// the file we're about the load - this will prevent the second/third versions of the file from replacing the first version when loaded.
					Package = CreatePackage(NULL, TEXT("Package_B"));
					Packages[1] = LoadPackage(Package, *Filename, LOAD_None);
					NumPackages++;
				}
				else if ( bAncestorPackage )
				{
					check(Packages[2] == NULL);

					// to avoid conflicts when loading packages from different locations that have the same name, create a dummy package to contain
					// the file we're about the load - this will prevent the second/third versions of the file from replacing the first version when loaded.
					Package = CreatePackage(NULL, TEXT("Package_C"));
					Packages[2] = LoadPackage(Package, *Filename, LOAD_None);
					NumPackages++;
				}

				if( Package == NULL )
				{
					SET_WARN_COLOR(COLOR_RED);
					warnf( NAME_Error, TEXT("Error loading package %s!"), *Filename );
					CLEAR_WARN_COLOR();
					bResult = FALSE;
					break;
				}

				bResult = TRUE;
			}
			else
			{
				SET_WARN_COLOR(COLOR_RED);
				if ( FilesInPath.Num() > 0 )
				{
					if ( PackageWildcard.InStr(TEXT("*")) != INDEX_NONE || PackageWildcard.InStr(TEXT("?")) != INDEX_NONE )
					{
						warnf(NAME_Error, TEXT("Wildcards are not allowed when specifying the name of a package to compare: %s"), *Tokens(TokenIndex));
					}
					else
					{
						warnf(TEXT("Multiple source files found for parameter %i: '%s'.  Please use the fully qualified path name for the package to avoid ambiguity."),
							TokenIndex, *Tokens(TokenIndex));
					}
				}
				else
				{
					warnf(NAME_Error, TEXT("No files found for parameter %i: '%s'."), TokenIndex, *Tokens(TokenIndex));
				}

				CLEAR_WARN_COLOR();
				bResult = FALSE;
				break;
			}
		}
	}

	// now process the switches
	if ( Switches.ContainsItem(TEXT("FULL")) )
	{
		bDiffAllProps = TRUE;
		bDiffNonEditProps = TRUE;
	}
	else if ( Switches.ContainsItem(TEXT("MOST")) )
	{
		bDiffNonEditProps = TRUE;
	}


	// verify that we got at least two packages
	if ( bResult && (Packages[0] == NULL || Packages[1] == NULL) )
	{
		SET_WARN_COLOR(COLOR_RED);
		warnf(NAME_Error, TEXT("You must specify at least two packages (not counting the ancestor package) to use this commandlet.  Use '%sGame help DiffPackagesCommandlet' to view correct usage syntax for this commandlet."),
			appGetGameName());

		CLEAR_WARN_COLOR();
		bResult = FALSE;
	}

	return bResult;
}

INT UDiffPackagesCommandlet::Main(const FString& Params)
{
	if ( !Initialize(*Params) )
	{
		// Initialize fails if the command-line parameters were invalid.
		return 1;
	}
	
	TArray<UObject*> IdenticalObjects;
	TArray<FObjectComparison> ObjectDiffs;

	warnf(TEXT("%sBuilding list of objects to diff...."), LINE_TERMINATOR);

	TIndirectArray<FObjectGraph> AllObjectSets[MAX_PACKAGECOUNT];

#if OPTIMIZE_LEVEL_DIFFS
	// since ULevel objects reference most everything else in the package, we specially handle them so that the diff results don't all appear
	// under the level object
	TArray<ULevel*> Levels;
#endif

	// loop through all objects in A
	for (FObjectIterator It; It; ++It)
	{
		UObject* Obj = *It;

		// skip over package and world objects
		if ( It->IsA(UPackage::StaticClass()) || It->IsA(UWorld::StaticClass()) )
		{
			continue;
		}

		// we only care about high level objects, like objects not inside other objects (actors are inside a level, not a package)
		if ( !It->GetOuter()->IsA(UPackage::StaticClass()) && !It->GetOuter()->IsA(ULevel::StaticClass()) && !It->GetOuter()->IsA(UWorld::StaticClass()) )
		{
			continue;
		}

		// if we're already created an object comparison for this object, skip it
		if (It->HasAnyFlags(RF_Marked))
		{
			continue;
		}

#if OPTIMIZE_LEVEL_DIFFS
		if ( It->IsA(ULevel::StaticClass()) )
		{
			Levels.AddUniqueItem(Cast<ULevel>(*It));
			continue;
		}
#endif

		// if this object isn't in any of the packages, then skip it
		FObjectComparison Comparison;
		if ( GenerateObjectComparison(*It, Comparison) )
		{
			for (INT PackageIndex = 0; PackageIndex < NumPackages; PackageIndex++)
			{
				if ( Comparison.ObjectSets[PackageIndex] != NULL )
				{
					AllObjectSets[PackageIndex].AddRawItem(Comparison.ObjectSets[PackageIndex]);
				}
			}

			// add this diff to our global list of diffs
			ObjectDiffs.AddItem(Comparison);
		}
	}

#if OPTIMIZE_LEVEL_DIFFS
	// now process the levels
	for ( INT LevelIndex = 0; LevelIndex < Levels.Num(); LevelIndex++ )
	{
		ULevel* Level = Levels(LevelIndex);
		if ( !Level->HasAnyFlags(RF_Marked) )
		{
			FObjectComparison Comparison;
			if ( GenerateObjectComparison(Level, Comparison, &ObjectDiffs) )
			{
				for (INT PackageIndex = 0; PackageIndex < NumPackages; PackageIndex++)
				{
					if ( Comparison.ObjectSets[PackageIndex] != NULL )
					{
						AllObjectSets[PackageIndex].AddRawItem(Comparison.ObjectSets[PackageIndex]);
					}
				}

				ObjectDiffs.AddItem(Comparison);
			}
		}
	}
#endif


	Sort<USE_COMPARE_CONSTREF(FObjectComparison,DiffPackagesCommandlet)>( &ObjectDiffs(0), ObjectDiffs.Num() );

	SET_WARN_COLOR(COLOR_DARK_RED);
	warnf(TEXT("%sComparing %d objects"), LINE_TERMINATOR, ObjectDiffs.Num());
	for ( INT DiffIndex = 0; DiffIndex < ObjectDiffs.Num(); DiffIndex++ )
	{
		// diff all the combination of objects
		warnf(TEXT("Performing comparison for object %d: %s"), DiffIndex, *ObjectDiffs(DiffIndex).RootObjectPath);
		ProcessDiff(ObjectDiffs(DiffIndex));
	}
	CLEAR_WARN_COLOR();

	SET_WARN_COLOR(COLOR_WHITE);
	warnf(TEXT("\nDifferences Found:"));
	CLEAR_WARN_COLOR();

	for (INT DiffIndex = 0; DiffIndex < ObjectDiffs.Num(); DiffIndex++)
	{
		FObjectComparison& Diff = ObjectDiffs(DiffIndex);

		if (Diff.PropDiffs.Num())
		{
			warnf(TEXT("------------------------------"));
			warnf(TEXT("%s [Overall result: %s]:"), *Diff.RootObjectPath, GetDiffTypeText(Diff.OverallDiffType,NumPackages));

			for (INT PropDiffIndex = 0; PropDiffIndex < Diff.PropDiffs.Num(); PropDiffIndex++)
			{
				FPropertyComparison& PropDiff = Diff.PropDiffs(PropDiffIndex);

				SET_WARN_COLOR((PropDiff.DiffType == OD_ABConflict || PropDiff.DiffType == OD_Invalid) ? COLOR_RED : COLOR_YELLOW);
				warnf(TEXT("%s"), *PropDiff.DiffText);
				CLEAR_WARN_COLOR();
			}
		}
	}

#if 0
	// disable merging for now....

	// do we want to merge objects?
	if (MergePackage)
	{
		// map any old object refs to the merged refs
		TMap<UObject*, UObject*> ReplacementMap;

		// automatically merge over all identical objects
		for (INT Index = 0; Index < ObjectDiffs.Num(); Index++)
		{
			FObjectComparison& Diff = ObjectDiffs(Index);

			// if we had no diffs, then just copy the first object
			INT ObjectToCopy = 0;
			if (Diff.OverallDiffType == OD_BOnly)
			{
				ObjectToCopy = 1;
			}
			// @todo: handle conflict and invalid diffs

			// if the object we want to copy doesn't exist (ie, a valid deletion), skip it
			if (Diff.ObjectSets[ObjectToCopy] == NULL)
			{
				continue;
			}

warnf(TEXT("Copying %s from package %d"), *Diff.ObjectSets[ObjectToCopy]->GetRootObject()->GetPathName(), ObjectToCopy);
			//!!!!!!!!!!!!!!! @todo iterate over non-root objects for all 3 properly, etc etc
			for (INT SetIndex = 0; SetIndex < Diff.ObjectSets[ObjectToCopy]->Objects.Num(); SetIndex++)
			{
				UObject* NewObject = CopyObjectToPackage(MergePackage, Diff.ObjectSets[ObjectToCopy]->Objects(SetIndex).Object);
				if (NewObject)
				{
					// make sure any references to these private objects don't get nuled
					ReplacementMap.Set(NewObject, NewObject);

					// map any refs to the original packages to the new object
					for (INT Package = 0; Package < NumPackages; Package++)
					{
						if (Diff.ObjectSets[Package])
						{
							ReplacementMap.Set(Diff.ObjectSets[Package]->Objects(SetIndex).Object, NewObject);
						}
					}
				}
			}
		}

		// fixup our references so we aren't linking to external private objects
		FixupObjectReferences(MergePackage, ReplacementMap);

		UWorld* World = FindObject<UWorld>(MergePackage, TEXT("TheWorld"));
		if(World)
		{	
			UObject::SavePackage(MergePackage, World, 0, *MergePackageFilename, GWarn);
		}
		else
		{
			UObject::SavePackage(MergePackage, NULL, RF_Standalone, *MergePackageFilename, GWarn);
		}
	}
#endif

	return 0;
}

/**
 * Wrapper for appending a comparison result to an comparison result buffer.
 */
void AppendComparisonResultText(FString& ExistingResultText, const FString& NewResultText)
{
	ExistingResultText += NewResultText + LINE_TERMINATOR;
}

UBOOL UDiffPackagesCommandlet::ProcessDiff(FObjectComparison& Diff)
{
	// always diff the root objects against each other
	Diff.OverallDiffType = DiffObjects(
							Diff.ObjectSets[0] ? Diff.ObjectSets[0]->GetRootObject() : NULL,
							Diff.ObjectSets[1] ? Diff.ObjectSets[1]->GetRootObject() : NULL,
							Diff.ObjectSets[2] ? Diff.ObjectSets[2]->GetRootObject() : NULL,
							Diff);

	for (INT PackageIndex = 0; PackageIndex < NumPackages; PackageIndex++)
	{
		// its possible we have a NULL object set if the root object isn't in the package
		if (!Diff.ObjectSets[PackageIndex])
		{
			continue;
		}
		// now go through the non-root object sets looking for different objects
		for (INT ObjectIndex = 1; ObjectIndex < Diff.ObjectSets[PackageIndex]->Objects.Num(); ObjectIndex++)
		{
			UObject* Objects[MAX_PACKAGECOUNT] = {NULL, NULL, NULL};
			
			// get the object in the object set
			Objects[PackageIndex] = Diff.ObjectSets[PackageIndex]->Objects(ObjectIndex).Object;

			// if the object is marked, it's already been diffed against another objectset, no need to do it again
			if (Objects[PackageIndex]->HasAnyFlags(RF_Marked))
			{
				continue;
			}

			// find matching objects in the other packages
			for (INT OtherPackageIndex = PackageIndex + 1; OtherPackageIndex < NumPackages; OtherPackageIndex++)
			{
				Objects[OtherPackageIndex] = FindMatchingObjectInObjectSet(Objects[PackageIndex], *Packages[OtherPackageIndex]->GetName(), Diff.ObjectSets[OtherPackageIndex]);
			}

			// mark that these subobjects have been diffed (this is used for finding unmatched subobjects later)
			for (INT ObjIndex = 0; ObjIndex < NumPackages; ObjIndex++)
			{
				if ( Objects[ObjIndex] )
				{
					Objects[ObjIndex]->SetFlags(RF_Marked);
				}
			}

			// diff the 2-3 objects
			EObjectDiff DiffType = DiffObjects(Objects[0], Objects[1], Objects[2], Diff);
			if (DiffType != OD_None)
			{
				if (Diff.OverallDiffType == OD_None || Diff.OverallDiffType == DiffType)
				{
					Diff.OverallDiffType = DiffType;
				}
				else
				{
					Diff.OverallDiffType = OD_ABConflict;
				}
			}
		}
	}

	return Diff.OverallDiffType != OD_None;
}

EObjectDiff UDiffPackagesCommandlet::DiffObjects(UObject* ObjA, UObject* ObjB, UObject* ObjAncestor, FObjectComparison& Diff)
{
	// if all objects are NULL, there's no difference :)
	if (!ObjA && !ObjB && !ObjAncestor)
	{
		return OD_None;
	}

	UClass* ComparisonClass = (ObjA ? ObjA->GetClass() : (ObjB ? ObjB->GetClass() : ObjAncestor->GetClass()));

	// complex logic for what kind of differnce this is, if at all

	// if one of the objects is a different class, just abort this whole thing
	if ((ObjA && ObjA->GetClass() != ComparisonClass) ||
		(ObjB && ObjB->GetClass() != ComparisonClass) || 
		(ObjAncestor && ObjAncestor->GetClass() != ComparisonClass))
	{
		FPropertyComparison InvalidClassDiff;
		AppendComparisonResultText(InvalidClassDiff.DiffText, FString::Printf(TEXT("Incompatible classes ('%s' '%s' '%s'"), 
			*ObjA->GetFullName(), *ObjB->GetFullName(), *ObjAncestor->GetFullName() ));

		InvalidClassDiff.DiffType = OD_Invalid;

		// add this diff to the list of diffs
		Diff.PropDiffs.AddItem(InvalidClassDiff);
		return OD_Invalid;
	}

	// @todo: don't make this ongoing, make this one for each different property
	EObjectDiff OverallDiffType = OD_None;
	for ( UProperty* Prop = ComparisonClass->PropertyLink; Prop; Prop = Prop->PropertyLinkNext )
	{
		// if this is not an editable property and -most or -full was not specified, then skip this property
		if ( !bDiffNonEditProps && (Prop->PropertyFlags & CPF_Edit) == 0 )
		{
			continue;
		}

		// if this is UObject property and -full was not specified, then skip this property
		if (!bDiffAllProps && (Prop->Offset < sizeof(UObject)))
		{
			continue;
		}

		for (INT Index = 0; Index < Prop->ArrayDim; Index++)
		{
			// friendly property name
			FString PropName = (Prop->ArrayDim > 1) ? FString::Printf(TEXT("%s[%d]"), *Prop->GetName(), Index) : *Prop->GetName();

			// get the string values for the property
			FString PropTextA, PropTextB, PropTextAncestor;
			if (ObjA)
			{
				Prop->ExportText(Index, PropTextA, (BYTE*)ObjA, (BYTE*)ObjA, ObjA, PPF_SimpleObjectText);
			}
			if (ObjB)
			{
				Prop->ExportText(Index, PropTextB, (BYTE*)ObjB, (BYTE*)ObjB, ObjB, PPF_SimpleObjectText);
			}
			if (ObjAncestor)
			{
				Prop->ExportText(Index, PropTextAncestor, (BYTE*)ObjAncestor, (BYTE*)ObjAncestor, ObjAncestor, PPF_SimpleObjectText);
			}

			// if the original text is different and the text where we replaced package A with B
			// is different, then the properties are really different
			// we can't just check for the second case for this:
			//       PropTextA: I love A.
			//       PropTextB: I love A.
			//       PropTextPkgReplaedB: I Love B.

// @todo: handle the I Love case above when removing text and not replacing it!!
// @todo: handle empty strings!

			FPropertyComparison PropDiff;
			PropDiff.Prop = Prop;
			PropDiff.DiffType = OD_None;

			// check for a change from ancestor, but to the same result
			if ( PropTextA.Len() && PropTextA == PropTextB )
			{
				// if we had an ancestor, and it was different, then we have a diff, but same result
				if (ObjAncestor && PropTextA != PropTextAncestor)
				{
					check(ObjA);
					FString FullPath = ObjA->GetFullName(Packages[0]);

					PropDiff.DiffType = OD_ABSame;

					AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("(%s) %s::%s"), GetDiffTypeText(PropDiff.DiffType,NumPackages), *FullPath, *PropName));
					AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("     Was: %s"), *PropTextAncestor));
					AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("     Now: %s"), *PropTextA));

					// accumulate diff types
					if (OverallDiffType == OD_None)
					{
						OverallDiffType = OD_ABSame;
					}
				}
				// otherwise, if no ancestor, or ancestor and a is the same as ancestor, then b there is no diff at all!
				// this is hopefully the common case :)
			}
			// okay, if A and B are different, need to compare against ancestor if we have one
			else
			{
				// if we have an ancestor, compare a and b against ancestor
				if (ObjAncestor)
				{
					FString FullPath = ObjAncestor->GetFullName(Packages[2]);

					// if A == ancestor, then only B changed
					if (PropTextA.Len() && PropTextA == PropTextAncestor)
					{
						PropDiff.DiffType = OD_BOnly;
						AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("(%s) %s::%s"), GetDiffTypeText(PropDiff.DiffType,NumPackages), *FullPath, *PropName));
						AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("     Was: %s"), *PropTextAncestor));
						AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("     Now: %s"), *PropTextB));

						// accumulate diff types
						if (OverallDiffType == OD_None || OverallDiffType == OD_BOnly)
						{
							OverallDiffType = OD_BOnly;
						}
						else
						{
							OverallDiffType = OD_ABConflict;
						}
					}
					// otherwise, if B == ancestor, then only A changed
					else if (PropTextB.Len() && PropTextB == PropTextAncestor)
					{
						PropDiff.DiffType = OD_AOnly;
						AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("(%s) %s::%s"), GetDiffTypeText(PropDiff.DiffType,NumPackages), *FullPath, *PropName));
						AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("     Was: %s"), *PropTextAncestor));
						AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("     Now: %s"), *PropTextA));

						// accumulate diff types
						if (OverallDiffType == OD_None || OverallDiffType == OD_AOnly)
						{
							OverallDiffType = OD_AOnly;
						}
						else
						{
							OverallDiffType = OD_ABConflict;
						}
					}
					// otherwise neither A or B equal ancestor, so we have a conflict!
					else if (PropTextA.Len() && PropTextB.Len())
					{
						PropDiff.DiffType = OD_ABConflict;
						AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("(%s) %s::%s"), GetDiffTypeText(PropDiff.DiffType,NumPackages), *FullPath, *PropName));

						AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("     Was: %s"), *PropTextAncestor));
						AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("     %s: %s"), *Packages[0]->GetName(), *PropTextA));
						AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("     %s: %s"), *Packages[1]->GetName(), *PropTextB));
					}

					// accumulate diff types
					OverallDiffType = OD_ABConflict;
				}
				// if we have no ancestor, and they are different, there's no way to know which one is right, so we 
				// mark is as a conflict
				else if (PropTextA.Len() && PropTextB.Len())
				{
					check(ObjB);
					PropDiff.DiffType = OD_ABConflict;
					FString FullPath = ObjA ? ObjA->GetFullName(Packages[0]) : ObjB->GetFullName(Packages[1]);

					AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("(%s) %s::%s"), GetDiffTypeText(PropDiff.DiffType,NumPackages), *FullPath, *PropName));
					AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("     %s: %s"), *Packages[0]->GetName(), *PropTextA));
					AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("     %s: %s"), *Packages[1]->GetName(), *PropTextB));

					// accumulate diff types
					OverallDiffType = OD_ABConflict;
				}
			}

			// if we actually had a diff, add it to the list
			if (PropDiff.DiffType != OD_None)
			{
				Diff.PropDiffs.AddItem(PropDiff);
			}
		}
	}

	INT NumObjects = 0;
	if ( ObjA )			NumObjects++;
	if ( ObjB )			NumObjects++;
	if ( ObjAncestor )	NumObjects++;

	// if this is a native class and we have at least two objects, include the property values for any natively serialized properties in the property comparison.
	if ( ComparisonClass->HasAnyClassFlags(CLASS_Native) && NumObjects > 1 )
	{
		EObjectDiff NativePropertyDiffType = CompareNativePropertyValues(ObjA, ObjB, ObjAncestor, Diff);
		if ( NativePropertyDiffType != OD_None && OverallDiffType == OD_None )
		{
			//@todo ronp - smarter integration with the OverallDiffType?
			OverallDiffType = NativePropertyDiffType;
		}
	}


	// now that we have done the per-property diffs, we can do whole-object diffs:

	// this diff isn't actually a property diff, its for missing objects, etc
	FPropertyComparison MissingObjectPropDiff;

	// if we are missing both a and b, we know that we had an ancestor (otherwise, we early out of this function)
	if (!ObjA && !ObjB)
	{
		FString FullPath = ObjAncestor->GetPathName(Packages[2]) + TEXT(" [") + ObjAncestor->GetClass()->GetName() + TEXT("]");

		MissingObjectPropDiff.DiffType = OD_ABSame;
		MissingObjectPropDiff.DiffText += FString::Printf(TEXT("(%s) Removed %s"), GetDiffTypeText(MissingObjectPropDiff.DiffType,NumPackages), *FullPath);
	}
	// if we are missing object a (we know we have ObjB)
	else if (!ObjA)
	{
		FString FullPath = ObjB->GetPathName(Packages[1]) + TEXT(" [") + ObjB->GetClass()->GetName() + TEXT("]");

		// if we were have an ancestor package, compare to ancestor
		if (Packages[2])
		{
			// if we have an ancestor
			if (ObjAncestor)
			{
				// if B wasn't different from the ancestor, then we were just deleted from A
				if (OverallDiffType == OD_None)
				{
					MissingObjectPropDiff.DiffType = OD_AOnly;
					MissingObjectPropDiff.DiffText += FString::Printf(TEXT("(%s) Removed %s"), GetDiffTypeText(MissingObjectPropDiff.DiffType,NumPackages), *FullPath);
				}
				// if B was different from Ancestor, then we were deleted from A and changed in B, conflict!
				else
				{
					MissingObjectPropDiff.DiffType = OD_ABConflict;
					MissingObjectPropDiff.DiffText += FString::Printf(TEXT("(%s) Removed/Modified %s"), GetDiffTypeText(MissingObjectPropDiff.DiffType,NumPackages), *FullPath);
				}
			}
			// otherwise, if we have an ancestor package, but no ancestor, then is _added_ to B
			else
			{
				MissingObjectPropDiff.DiffType = OD_BOnly;
				MissingObjectPropDiff.DiffText += FString::Printf(TEXT("(%s) Added %s"), GetDiffTypeText(MissingObjectPropDiff.DiffType,NumPackages), *FullPath);
			}
		}
		// if no ancestor package, then we don't know if the object was added or deleted, so mark it as a conflict
		else
		{
			MissingObjectPropDiff.DiffType = OD_BOnly;
			MissingObjectPropDiff.DiffText += FString::Printf(TEXT("(%s) Added %s"), GetDiffTypeText(MissingObjectPropDiff.DiffType,NumPackages), *FullPath);
		}
	}
	// if we are missing object B (we know we have ObjA)
	else if ( !ObjB )
	{
		FString FullPath = ObjA->GetPathName(Packages[0]) + TEXT(" [") + ObjA->GetClass()->GetName() + TEXT("]");

		// if we were have an ancestor package, compare to ancestor
		if (Packages[2])
		{
			// if we have an ancestor
			if (ObjAncestor)
			{
				// if A wasn't different from the ancestor, then we were just deleted from B
				if (OverallDiffType == OD_None)
				{
					MissingObjectPropDiff.DiffType = OD_BOnly;
					MissingObjectPropDiff.DiffText += FString::Printf(TEXT("(%s) Removed %s"), GetDiffTypeText(MissingObjectPropDiff.DiffType,NumPackages), *FullPath);
				}
				// if A was different from Ancestor, then we were deleted from B and changed in A, conflict!
				else
				{
					MissingObjectPropDiff.DiffType = OD_ABConflict;
					MissingObjectPropDiff.DiffText += FString::Printf(TEXT("(%s) Modified/Removed %s"), GetDiffTypeText(MissingObjectPropDiff.DiffType,NumPackages), *FullPath);
				}
			}
			// otherwise, if we have an ancestor package, but no ancestor, then is _added_ to A
			else
			{
				MissingObjectPropDiff.DiffType = OD_AOnly;
				MissingObjectPropDiff.DiffText += FString::Printf(TEXT("(%s) Added %s"), GetDiffTypeText(MissingObjectPropDiff.DiffType,NumPackages), *FullPath);
			}
		}
		// if no ancestor package, then we don't know if the object was added or deleted, so mark it as a conflict
		else
		{
			MissingObjectPropDiff.DiffType = OD_AOnly;
			MissingObjectPropDiff.DiffText += FString::Printf(TEXT("(%s) Removed %s"), GetDiffTypeText(MissingObjectPropDiff.DiffType,NumPackages), *FullPath);
		}
	}

	// look for objects added to both, but only if an ancestor package was specified
	// ObjAncestor is NULL if no ancestor package was specified or the object didn't exist in the ancestor package, so check for NumPackages == 3
	else if (ObjA && ObjB && !ObjAncestor && NumPackages == 3 )
	{
		FString FullPath = ObjA->GetPathName(Packages[0]) + TEXT(" [") + ObjA->GetClass()->GetName() + TEXT("]");
		if (OverallDiffType == OD_None)
		{
			MissingObjectPropDiff.DiffType = OD_ABSame;
			MissingObjectPropDiff.DiffText += FString::Printf(TEXT("(%s) Added %s"), GetDiffTypeText(MissingObjectPropDiff.DiffType,NumPackages), *FullPath);
		}
		else
		{
			MissingObjectPropDiff.DiffType = OD_ABConflict;
			MissingObjectPropDiff.DiffText += FString::Printf(TEXT("(%s) Added %s"), GetDiffTypeText(MissingObjectPropDiff.DiffType,NumPackages), *FullPath);
		}
	}

	if (MissingObjectPropDiff.DiffType != OD_None)
	{
		// add this diff to the list of diffs
		Diff.PropDiffs.AddItem(MissingObjectPropDiff);

		if (OverallDiffType == OD_None)
		{
			OverallDiffType = MissingObjectPropDiff.DiffType;
		}
		// @todo: accumulate better into overalldifftype?
	}

	return OverallDiffType;
}


/**
 * Copies the raw property values for the natively serialized properties of the specified object into the output var.
 *
 * @param	Object	the object to load values for
 * @param	out_NativePropertyData	receives the raw bytes corresponding to Object's natively serialized property values.
 */
void UDiffPackagesCommandlet::LoadNativePropertyData( UObject* Object, TArray<BYTE>& out_NativePropertyData )
{
	// first, validate our input parameters
	check(Object);

	ULinkerLoad* ObjectLinker = Object->GetLinker();
	check(ObjectLinker);

	INT ObjectLinkerIndex = Object->GetLinkerIndex();
	check(ObjectLinker->ExportMap.IsValidIndex(ObjectLinkerIndex));

	// now begin the process of loading the data for this object's natively serialized properties into the memory archive
	out_NativePropertyData.Empty();
	
	FObjectExport& ObjectExport = ObjectLinker->ExportMap(ObjectLinkerIndex);
	const INT ScriptStartPos = ObjectExport.ScriptSerializationStartOffset;
	const INT ScriptEndPos = ObjectExport.ScriptSerializationEndOffset;

	const INT NativeStartPos = ScriptEndPos;
	const INT NativeEndPos = ObjectExport.SerialOffset + ObjectExport.SerialSize;

	const INT NativePropertySerialSize = NativeEndPos - NativeStartPos;
	if ( NativePropertySerialSize > 0 )
	{
		checkSlow(NativeStartPos>=ObjectExport.SerialOffset);
		checkSlow(NativeStartPos<NativeEndPos);
#if 1
		// here's one way to do it


		//@todo ronp - this code assumes that all natively serialized data occurs after script-serialized data,
		// but this might not be the case - need to make sure we catch any native data that is serialized before the property data
		const INT SavedPos = ((FArchive*)ObjectLinker)->Tell();

		((FArchive*)ObjectLinker)->Seek(NativeStartPos);
		((FArchive*)ObjectLinker)->Precache(NativeStartPos, NativePropertySerialSize);

		// allocate enough space to contain the data we're about to read from disk
		out_NativePropertyData.AddZeroed(NativePropertySerialSize);
		((FArchive*)ObjectLinker)->Serialize(out_NativePropertyData.GetData(), NativePropertySerialSize);

		// return the linker to its previous position
		((FArchive*)ObjectLinker)->Seek(SavedPos);
#else
		// here's another possible way to do it.  this approach is less ideal because we're reading property data from memory
		// so the data that is being serialized might not match the data on disk (i.e. if the object's PostLoad function performs
		// operations on the serialized data which modifies it)

		// the advantage of this approach is that we could create a subclass of FBufferArchive which implements the MarkScriptSerialization* methods, which
		// would eliminate the need for the object to still have a valid linker (since the linker is only used to find out where the object's script serialized
		// property data starts and ends)

		// we serialize the entire object into a memory buffer archive
		FBufferArchive ObjectDataArc(TRUE);
		Object->Serialize(ObjectDataArc);

		// then we copy over the bytes which correspond to the natively serialized data
		out_NativePropertyData.Empty(NativePropertySerialSize);
		for ( INT i = NativeStartPos; i <= NativeEndPos; i++ )
		{
			out_NativePropertyData.AddItem(ObjectDataArc(i));
		}
#endif
	}
}

/**
 * Compares the natively serialized property values for the specified objects by comparing the non-script serialized portion of each object's data as it
 * is on disk.  If a different is detected, gives each object the chance to generate a textual representation of its natively serialized property values
 * that will be displayed to the user in the final comparison report.
 *
 * @param	ObjA		the object from the first package being compared.  Can be NULL if both ObjB and ObjAncestor are valid, which indicates that this object
 *						doesn't exist in the first package.
 * @param	ObjB		the object from the second package being compared.  Can be NULL if both ObjA and ObjAncestor are valid, which indicates that this object
 *						doesn't exist in the second package.
 * @param	ObjAncestor	the object from the optional common base package.  Can only be NULL if both ObjA and ObjB are valid, which indicates that this is either
 *						a two-way comparison (if NumPackages == 2) or the object was added to both packages (if NumPackages == 3)
 * @param	PropertyValueComparisons	contains the results for all property values that were different so far; for any native property values which are determined
 *										to be different, new entries will be added to the ObjectComparison's list of PropDiffs.
 *
 * @return	The cumulative comparison result type for a comparison of all natively serialized property values.
 *
 * @todo ronp - add support for individual native property comparisons when the FPropertyComparison::PropertyText is filled in.
 */
EObjectDiff UDiffPackagesCommandlet::CompareNativePropertyValues( UObject* ObjA, UObject* ObjB, UObject* ObjAncestor, FObjectComparison& PropertyValueComparisons )
{
	FNativePropertyData PropertyDataA(ObjA), PropertyDataB(ObjB), PropertyDataAncestor(ObjAncestor);

	FPropertyComparison NativeDataComparison;
	NativeDataComparison.PropText = TEXT("Native Properties");
	NativeDataComparison.DiffType = OD_None;

	if ( ObjA == NULL )
	{
		check(ObjB);
		check(ObjAncestor);

		FString ObjectPathName = ObjB->GetPathName(Packages[1]);
		if ( PropertyDataAncestor )
		{
			// If the values in ObjB are identical to the values in the common ancestor, then the object was removed from the first package
			if ( PropertyDataB == PropertyDataAncestor )
			{
				NativeDataComparison.DiffType = OD_AOnly;
				NativeDataComparison.DiffText += FString::Printf(TEXT("(%s) Removed %s"), GetDiffTypeText(NativeDataComparison.DiffType,NumPackages), *ObjectPathName);
			}
			else
			{
				// if the values in ObjB are different from the values in the common ancestor, then the object was removed from the first package
				// but changed in the second package, which is a conflict
				NativeDataComparison.DiffType = OD_ABConflict;
				NativeDataComparison.DiffText += FString::Printf(TEXT("(%s) Removed/Modified %s"), GetDiffTypeText(NativeDataComparison.DiffType,NumPackages), *ObjectPathName);
			}
		}
		else
		{
			NativeDataComparison.DiffType = OD_BOnly;
			NativeDataComparison.DiffText += FString::Printf(TEXT("(%s) Added %s"), GetDiffTypeText(NativeDataComparison.DiffType,NumPackages), *ObjectPathName);
		}
	}
	else if ( ObjB == NULL )
	{
		check(ObjA);
		check(ObjAncestor);

		FString ObjectPathName = ObjA->GetPathName(Packages[0]);
		if ( PropertyDataAncestor )
		{
			// If the values in ObjA are identical to the values in the common ancestor, then the object was removed from the second package
			if ( PropertyDataB == PropertyDataAncestor )
			{
				NativeDataComparison.DiffType = OD_BOnly;
				NativeDataComparison.DiffText += FString::Printf(TEXT("(%s) Removed %s"), GetDiffTypeText(NativeDataComparison.DiffType,NumPackages), *ObjectPathName);
			}
			else
			{
				// if the values in ObjA are different from the values in the common ancestor, then the object was removed from the second package
				// but changed in the first package, which is a conflict
				NativeDataComparison.DiffType = OD_ABConflict;
				NativeDataComparison.DiffText += FString::Printf(TEXT("(%s) Removed/Modified %s"), GetDiffTypeText(NativeDataComparison.DiffType,NumPackages), *ObjectPathName);
			}
		}
		else
		{
			NativeDataComparison.DiffType = OD_AOnly;
			NativeDataComparison.DiffText += FString::Printf(TEXT("(%s) Added %s"), GetDiffTypeText(NativeDataComparison.DiffType,NumPackages), *ObjectPathName);
		}
	}
	// look for objects added to both, but only if an ancestor package was specified
	// ObjAncestor is NULL if no ancestor package was specified or the object didn't exist in the ancestor package, so check for NumPackages == 3
	else if ( ObjA && ObjB && !ObjAncestor && NumPackages == 3 )
	{
		FString ObjectPathName = ObjA->GetPathName(Packages[0]) + TEXT(" [") + ObjA->GetClass()->GetName() + TEXT("]");
		if ( PropertyDataA == PropertyDataB )
		{
			NativeDataComparison.DiffType = OD_ABSame;
			NativeDataComparison.DiffText += FString::Printf(TEXT("(%s) Added %s"), GetDiffTypeText(NativeDataComparison.DiffType,NumPackages), *ObjectPathName);
		}
		else
		{
			NativeDataComparison.DiffType = OD_ABConflict;
			NativeDataComparison.DiffText += FString::Printf(TEXT("(%s) Added %s"), GetDiffTypeText(NativeDataComparison.DiffType,NumPackages), *ObjectPathName);
		}
	}
	else
	{
		// first, check to see if both packages were changed to the same value
		check(ObjA && ObjB);
		if ( PropertyDataA && PropertyDataA == PropertyDataB )
		{
			// if we have an ancestor and its data is different than the data from ObjA, then both packages were changed to the same value
			if ( ObjAncestor && PropertyDataA != PropertyDataAncestor )
			{
				FString ObjectPathName = ObjA->GetFullName(Packages[0]);
// 				NativePropertyComparison.DiffType = OD_ABSame;
// 
// 				AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("(%s) %s::%s"), GetDiffTypeText(PropDiff.DiffType,NumPackages), *FullPath, *PropName));
// 				AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("     Was: %s"), *PropTextAncestor));
// 				AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("     Now: %s"), *PropTextA));
				if ( NativeDataComparison.DiffType == OD_None )
				{
					NativeDataComparison.DiffType = OD_ABSame;
					AppendComparisonResultText(NativeDataComparison.DiffText, FString::Printf(TEXT("(%s) %s"), GetDiffTypeText(NativeDataComparison.DiffType, NumPackages), *ObjectPathName));
					AppendComparisonResultText(NativeDataComparison.DiffText, TEXT("     Unknown native property data"));
				}
			}

			// otherwise, if there is no ancestor object or ObjA's value is identical to the ancestor's value, then all three packages
			// have the same values for this object
		}
		else
		{
			// if A and B are different, we need to compare against the common ancestor (if we have one)
			if ( ObjAncestor )
			{
				FString ObjectPathName = ObjAncestor->GetFullName(Packages[2]);

				// if the values from ObjA are identical to the values in the common base, then ObjB was changed
				if ( PropertyDataA && PropertyDataA == PropertyDataAncestor )
				{
// 					CurrentPropertyComparison.DiffType = OD_BOnly;
// 					AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("(%s) %s::%s"), GetDiffTypeText(PropDiff.DiffType,NumPackages), *FullPath, *PropName));
// 					AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("     Was: %s"), *PropTextAncestor));
// 					AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("     Now: %s"), *PropTextB));

					if ( NativeDataComparison.DiffType == OD_None )
					{
						NativeDataComparison.DiffType = OD_BOnly;
					}
					else if ( NativeDataComparison.DiffType != OD_BOnly )
					{
						NativeDataComparison.DiffType = OD_ABConflict;
					}
					AppendComparisonResultText(NativeDataComparison.DiffText, FString::Printf(TEXT("(%s) %s"), GetDiffTypeText(NativeDataComparison.DiffType, NumPackages), *ObjectPathName));
					AppendComparisonResultText(NativeDataComparison.DiffText, TEXT("     Unknown native property data"));
				}
				// Otherwise, if the values from ObjB are identical to the values in the common base, then only ObjA was changed
				else if ( PropertyDataB && PropertyDataB == PropertyDataAncestor )
				{
// 					CurrentPropertyComparison.DiffType = OD_AOnly;
// 					AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("(%s) %s::%s"), GetDiffTypeText(PropDiff.DiffType,NumPackages), *FullPath, *PropName));
// 					AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("     Was: %s"), *PropTextAncestor));
// 					AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("     Now: %s"), *PropTextA));

					if (NativeDataComparison.DiffType == OD_None )
					{
						NativeDataComparison.DiffType = OD_AOnly;
						AppendComparisonResultText(NativeDataComparison.DiffText, FString::Printf(TEXT("(%s) %s"),
							GetDiffTypeText(NativeDataComparison.DiffType, NumPackages), *ObjectPathName));
						AppendComparisonResultText(NativeDataComparison.DiffText, TEXT("     Unknown native property data"));
					}
					else if ( NativeDataComparison.DiffType != OD_AOnly )
					{
						NativeDataComparison.DiffType = OD_ABConflict;
						AppendComparisonResultText(NativeDataComparison.DiffText, FString::Printf(TEXT("(%s) %s"),
							GetDiffTypeText(NativeDataComparison.DiffType, NumPackages), *ObjectPathName));
						AppendComparisonResultText(NativeDataComparison.DiffText, TEXT("     Unknown native property data"));
					}
				}
				// Otherwise, the values from ObjA and ObjB are different from each other as well as from the values in the common base
				else if ( PropertyDataA && PropertyDataB )
				{
// 					CurrentPropertyComparison.DiffType = OD_ABConflict;
// 					AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("(%s) %s::%s"), GetDiffTypeText(PropDiff.DiffType,NumPackages), *FullPath, *PropName));
// 
// 					AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("     Was: %s"), *PropTextAncestor));
// 					AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("     %s: %s"), *Packages[0]->GetName(), *PropTextA));
// 					AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("     %s: %s"), *Packages[1]->GetName(), *PropTextB));

					NativeDataComparison.DiffType = OD_ABConflict;
					AppendComparisonResultText(NativeDataComparison.DiffText, FString::Printf(TEXT("(%s) %s"),
						GetDiffTypeText(NativeDataComparison.DiffType, NumPackages), *ObjectPathName));
					AppendComparisonResultText(NativeDataComparison.DiffText, TEXT("     Unknown native property data"));
				}
			}
			// If we have no common base and the values from ObjA & ObjB are different, mark it as a conflict
			else if ( PropertyDataA && PropertyDataB )
			{
				FString ObjectPathName = ObjA ? ObjA->GetFullName(Packages[0]) : ObjB->GetFullName(Packages[1]);
// 				CurrentPropertyComparison.DiffType = OD_ABConflict;
// 				AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("(%s) %s::%s"), GetDiffTypeText(PropDiff.DiffType,NumPackages), *ObjectPathName, *PropName));
// 				AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("     %s: %s"), *Packages[0]->GetName(), *PropTextA));
// 				AppendComparisonResultText(PropDiff.DiffText, FString::Printf(TEXT("     %s: %s"), *Packages[1]->GetName(), *PropTextB));

				// accumulate diff types
				NativeDataComparison.DiffType = OD_ABConflict;
				AppendComparisonResultText(NativeDataComparison.DiffText, FString::Printf(TEXT("(%s) %s"),
					GetDiffTypeText(NativeDataComparison.DiffType, NumPackages), *ObjectPathName));
				AppendComparisonResultText(NativeDataComparison.DiffText, TEXT("     Unknown native property data"));
			}
		}

// 		// if we actually had a diff, add it to the list
// 		if (PropDiff.DiffType != OD_None)
// 		{
// 			Diff.PropDiffs.AddItem(PropDiff);
// 		}
	}

	// If we had any type of difference between property values for this object, add an entry to the object comparison to indicate that
	// there were differences in the property data for this object's natively serialized properties
	if (NativeDataComparison.DiffType != OD_None)
	{
		PropertyValueComparisons.PropDiffs.AddItem(NativeDataComparison);

		// If PropertyValueComparisons.OverallDiffType is still OD_None, it means that the values for this object's script-serialized properties
		// were identical across all packages.  If we encountered differences in the native property data, update the object comparison's OverallDiffType so
		// that the differences are reported
		if ( PropertyValueComparisons.OverallDiffType == OD_None )
		{
			PropertyValueComparisons.OverallDiffType = NativeDataComparison.DiffType;
		}
	}

	return NativeDataComparison.DiffType;
}

IMPLEMENT_CLASS(UDiffPackagesCommandlet)
