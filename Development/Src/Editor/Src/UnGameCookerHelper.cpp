/*=============================================================================
	UnGameCookerHelper.cpp: Game specific cooking helper class implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "UnGameCookerHelper.h"

//-----------------------------------------------------------------------------
//	GenerateGameContentSeekfree
//-----------------------------------------------------------------------------
#define GAME_CONTENT_PKG_PREFIX TEXT("MPContent")

static DOUBLE		GenerateGameContentTime;
static DOUBLE		GenerateGameContentInitTime;
static DOUBLE		GenerateGameContentListGenerationTime;
static DOUBLE		GenerateGameContentPackageGenerationTime;

/**
* Helper for adding game content standalone packages for cooking
*/
class FGenerateGameContentSeekfree
{
public:

	/**
	* Constructor
	*/
	FGenerateGameContentSeekfree()
		: bForceRecookSeekfreeGameTypes(FALSE)
	{
		Init();
	}

	/**
	* Initialize options from command line options
	*
	*	@param	Tokens			Command line tokens parsed from app
	*	@param	Switches		Command line switches parsed from app
	*/
	void InitOptions(const TArray<FString>& Tokens, const TArray<FString>& Switches)
	{
		SCOPE_SECONDS_COUNTER(GenerateGameContentTime);
		// Check for flag to recook seekfree game type content
		bForceRecookSeekfreeGameTypes = Switches.ContainsItem(TEXT("RECOOKSEEKFREEGAMETYPES"));
	}

	/**
	* Adds one standalone seekfree entry to the package list for each game type specified from ini.  
	*
	* @return TRUE if succeeded
	*/
	UBOOL GeneratePackageList( 
	class UCookPackagesCommandlet* Commandlet, 
		UE3::EPlatformType Platform,
		EShaderPlatform ShaderPlatform,
		TArray<FPackageCookerInfo>& NotRequiredFilenamePairs,
		TArray<FPackageCookerInfo>& RegularFilenamePairs,
		TArray<FPackageCookerInfo>& MapFilenamePairs,
		TArray<FPackageCookerInfo>& ScriptFilenamePairs,
		TArray<FPackageCookerInfo>& StartupFilenamePairs,
		TArray<FPackageCookerInfo>& StandaloneSeekfreeFilenamePairs)
	{
		SCOPE_SECONDS_COUNTER(GenerateGameContentTime);
		SCOPE_SECONDS_COUNTER(GenerateGameContentListGenerationTime);
		if (Commandlet->bIsInUserMode)
		{
			return TRUE;
		}

		UBOOL bSuccess=TRUE;

		FString OutDir = appGameDir() + PATH_SEPARATOR + FString(GAME_CONTENT_PKG_PREFIX) + PATH_SEPARATOR;
		TMap<FString,UBOOL> bIsSeekfreeFileDependencyNewerMap;

		// update the package cooker info for each content entry
		for( TMap<FString,FGameContentEntry>::TIterator It(GameContentMap); It; ++It )
		{
			const FString& GameName = It.Key();
			FGameContentEntry& ContentEntry = It.Value();

			// Source filename of temp package to be cooked
			ContentEntry.PackageCookerInfo.SrcFilename = OutDir + GameName;
			// Destination filename for cooked seekfree package
			ContentEntry.PackageCookerInfo.DstFilename = Commandlet->GetCookedPackageFilename(ContentEntry.PackageCookerInfo.SrcFilename);

			// setup flags to cook as seekfree standalone package
			ContentEntry.PackageCookerInfo.bShouldBeSeekFree = TRUE;
			ContentEntry.PackageCookerInfo.bIsNativeScriptFile = FALSE;
			ContentEntry.PackageCookerInfo.bIsCombinedStartupPackage = FALSE;
			ContentEntry.PackageCookerInfo.bIsStandaloneSeekFreePackage = TRUE;
			ContentEntry.PackageCookerInfo.bShouldOnlyLoad = FALSE;

			// check to see if this seekfree package's dependencies require it to be recooked
			UBOOL bIsSeekfreeFileDependencyNewer = FALSE;
			if( bForceRecookSeekfreeGameTypes )
			{
				// force recook
				bIsSeekfreeFileDependencyNewer = TRUE;
			}
			else
			{
				FString ActualDstName = ContentEntry.PackageCookerInfo.DstFilename.GetBaseFilename(FALSE) + STANDALONE_SEEKFREE_SUFFIX + FString(TEXT(".")) + ContentEntry.PackageCookerInfo.DstFilename.GetExtension();
				// get dest cooked file timestamp
				DOUBLE Time = GFileManager->GetFileTimestamp(*ActualDstName);
				// iterate over source content that needs to be cooked for this game type
				for( INT ContentIdx=0; ContentIdx < ContentEntry.ContentList.Num(); ContentIdx++ )
				{
					const FString& ContentStr = ContentEntry.ContentList(ContentIdx);

					FString SrcContentPackageFileBase = ContentStr;

					// strip off anything after the base package name
					INT FoundIdx = ContentStr.InStr(TEXT("."));
					if( FoundIdx != INDEX_NONE )
					{
						SrcContentPackageFileBase = ContentStr.LeftChop(ContentStr.Len() - FoundIdx);
					}

					FString SrcContentPackageFilePath;
					// find existing content package
					if( GPackageFileCache->FindPackageFile(*SrcContentPackageFileBase, NULL, SrcContentPackageFilePath) )
					{
						// check if we've already done the (slow) dependency check for this package
						UBOOL* bExistingEntryPtr = bIsSeekfreeFileDependencyNewerMap.Find(SrcContentPackageFileBase);					
						if( bExistingEntryPtr )
						{
							// use existing entry to trigger update
							bIsSeekfreeFileDependencyNewer |= *bExistingEntryPtr;
						}
						else
						{
							// do dependency check for packages coming from exports of the current content package
							UBOOL bHasNewer = Commandlet->AreSeekfreeDependenciesNewer(NULL, *SrcContentPackageFilePath, Time);
							bIsSeekfreeFileDependencyNewer |= bHasNewer;
							// keep track of this content package entry to check for duplicates
							bIsSeekfreeFileDependencyNewerMap.Set(SrcContentPackageFileBase,bHasNewer);
						}
					}				
				}
			}

			//@todo. Check if the language version being cooked for is present as well...
			if( !bIsSeekfreeFileDependencyNewer )
			{
			}

			if( !bIsSeekfreeFileDependencyNewer )
			{
				debugf(NAME_Log, TEXT("GamePreloadContent: standalone seekfree package for %s is UpToDate, skipping"), *GameName);
			}
			else
			{
				// add the entry to the standalone seekfree list
				StandaloneSeekfreeFilenamePairs.AddItem(ContentEntry.PackageCookerInfo);

				debugf(NAME_Log, TEXT("GamePreloadContent: Adding standalone seekfree package for %s"), *GameName);
			}
		}

		return bSuccess;
	}

	/**
	* Match game content package filename to the current filename being cooked and 
	* create a temporary package for it instead of loading a package from file. This
	* will also load all the game content needed and add an entry to the object
	* referencer in the package.
	* 
	* @param Commandlet - commandlet that is calling back
	* @param Filename - current filename that needs to be loaded for cooking
	*
	* @return TRUE if succeeded
	*/
	UPackage* LoadPackageForCookingCallback(class UCookPackagesCommandlet* Commandlet, const TCHAR* Filename)
	{
		SCOPE_SECONDS_COUNTER(GenerateGameContentTime);
		SCOPE_SECONDS_COUNTER(GenerateGameContentPackageGenerationTime);
		UPackage* Result=NULL;

		// find package that needs to be generated by looking up the filename
		for( TMap<FString,FGameContentEntry>::TIterator It(GameContentMap); It; ++It )
		{
			const FString& GameName = It.Key();
			FGameContentEntry& ContentEntry = It.Value();

			if( ContentEntry.PackageCookerInfo.SrcFilename == FString(Filename).ToLower() )
			{
				// create a temporary package to import the content objects
				Result = UObject::CreatePackage(NULL, *ContentEntry.PackageCookerInfo.SrcFilename);
				if( !Result )
				{
					warnf(NAME_Warning, TEXT("GamePreloadContent: Couldn't generate package %s"), *ContentEntry.PackageCookerInfo.SrcFilename);
				}
				else
				{
					// create a referencer to keep track of the content objects added in the package
					UObjectReferencer* ObjRefeferencer = ConstructObject<UObjectReferencer>(UObjectReferencer::StaticClass(),Result);
					ObjRefeferencer->SetFlags( RF_Standalone );

					// load all content objects and add them to the package
					for( INT ContentIdx=0; ContentIdx < ContentEntry.ContentList.Num(); ContentIdx++ )
					{
						const FString& ContentStr = ContentEntry.ContentList(ContentIdx);

						// load the requested content object (typically a content class) 
						debugf(NAME_Log, TEXT("GamePreloadContent: Loading content %s for %s"), *ContentStr, *GameName);
						UObject* ContentObj = LoadObject<UObject>(NULL, *ContentStr, NULL, LOAD_NoWarn, NULL);
						if( !ContentObj )
						{
							ContentObj = LoadObject<UClass>(NULL, *ContentStr, NULL, LOAD_NoWarn, NULL);
						}
						if( !ContentObj )
						{
							warnf(NAME_Warning, TEXT("GamePreloadContent: Couldn't load content %s"), *ContentStr);
						}
						else
						{							
							ObjRefeferencer->ReferencedObjects.AddUniqueItem(ContentObj);					
						}
					}
				}

				// need to fully load before saving 
				Result->FullyLoad();
				// should only have one filename entry for the source package we create
				break;
			}
		}

		return Result;
	}

	/**
	* Not used
	*/
	UBOOL PostLoadPackageForCookingCallback(class UCookPackagesCommandlet* Commandlet, UPackage* InPackage)
	{
		return TRUE;
	}

	/**
	 *	Dump out stats specific to the game cooker helper.
	 */
	virtual void DumpStats()
	{
		warnf( NAME_Log, TEXT("") );
		warnf( NAME_Log, TEXT("  GameContent Stats:") );
		warnf( NAME_Log, TEXT("    Total time                             %7.2f seconds"), GenerateGameContentTime );
		warnf( NAME_Log, TEXT("    Init time                              %7.2f seconds"), GenerateGameContentInitTime );
		warnf( NAME_Log, TEXT("    List generation time                   %7.2f seconds"), GenerateGameContentListGenerationTime );
		warnf( NAME_Log, TEXT("    Package generation time                %7.2f seconds"), GenerateGameContentPackageGenerationTime );
	}

private:

	/**
	* Initializes the list of game content packages that need to generated. 
	* Game types along with the content to be loaded for each is loaded from ini.
	* Set in [Cooker.MPGameContentCookStandalone] section
	*/
	void Init()
	{
		SCOPE_SECONDS_COUNTER(GenerateGameContentTime);
		SCOPE_SECONDS_COUNTER(GenerateGameContentInitTime);

		bForceRecookSeekfreeGameTypes=FALSE;
		GameContentMap.Empty();

		// check to see if MP game content should be cooked into its own SF packages
		// enabling this flag will remove the hard game references from the map files
		UBOOL bCookSeparateSharedMPGameContent = FALSE;
		GConfig->GetBool( TEXT("Engine.Engine"), TEXT("bCookSeparateSharedMPGameContent"), bCookSeparateSharedMPGameContent, GEngineIni );
		if ( bCookSeparateSharedMPGameContent )
		{
			debugf( NAME_Log, TEXT("Saw bCookSeparateSharedMPGameContent flag.") );
		}
		// find the game strings and content to be loaded for each
		if( bCookSeparateSharedMPGameContent )
		{
			// Allow user to specify a single game type on command line.
			FString CommandLineGameType;
			Parse( appCmdLine(), TEXT("MPGAMETYPE="), CommandLineGameType );

			TMap<FString, TArray<FString> > MPGameContentStrings;
			GConfig->Parse1ToNSectionOfStrings(TEXT("Cooker.MPGameContentCookStandalone"), TEXT("GameType"), TEXT("Content"), MPGameContentStrings, GEditorIni);
			for( TMap<FString,TArray<FString> >::TConstIterator It(MPGameContentStrings); It; ++It )
			{
				const FString& GameTypeStr = It.Key();
				const TArray<FString>& GameContent = It.Value();
				for( INT Idx=0; Idx < GameContent.Num(); Idx++ )
				{
					const FString& GameContentStr = GameContent(Idx);
					// Only add game type if it matches command line or no command line override was specified.
					if( CommandLineGameType.Len() == 0 || CommandLineGameType == GameTypeStr )
					{
						AddGameContentEntry(*GameTypeStr,*GameContentStr);
					}
				}
			}
		}
	}

	/** if TRUE then dependency checking is disabled when cooking the game content packages */
	UBOOL bForceRecookSeekfreeGameTypes;

	/**
	* Game content package entry
	*/
	struct FGameContentEntry
	{
		FGameContentEntry()
			: PackageCookerInfo(NULL,NULL,FALSE,FALSE,FALSE,FALSE)
		{
		}
		/** list of content strings that are going to be loaded */
		TArray<FString> ContentList;
		/** package list entry generated for the game type */
		FPackageCookerInfo PackageCookerInfo;
	};
	/** game string mapping to content entries for generating packages */
	TMap<FString,FGameContentEntry> GameContentMap;

	/**
	* Adds a unique entry for the given game type
	* Adds the content string for each game type given
	*
	* @param InGameStr - game string used as base for package filename
	* @param InContentStr - content to be loaded for the game type package
	*/
	void AddGameContentEntry(const TCHAR* InGameStr,const TCHAR* InContentStr)
	{
		if( !InGameStr || !InContentStr )
			return;

		FGameContentEntry* GameContentEntryPtr = GameContentMap.Find(FString(InGameStr).ToLower());
		if( GameContentEntryPtr )
		{
			GameContentEntryPtr->ContentList.AddUniqueItem(FString(InContentStr).ToLower());
		}
		else
		{
			FGameContentEntry GameContentEntry;
			GameContentEntry.ContentList.AddItem(FString(InContentStr).ToLower());
			GameContentMap.Set(FString(InGameStr).ToLower(),GameContentEntry);
		}
	}
};

/**
* @return FGenerateGameContentSeekfree	Singleton instance for game content seekfree helper class
*/
class FGenerateGameContentSeekfree* FGameCookerHelper::GetGameContentSeekfreeHelper()
{
	static FGenerateGameContentSeekfree MapPreloadHelper;
	return &MapPreloadHelper;
}

//-----------------------------------------------------------------------------
//	GameCookerHelper
//-----------------------------------------------------------------------------

/**
* Initialize the cooker helpr and process any command line params
*
*	@param	Commandlet		The cookpackages commandlet being run
*	@param	Tokens			Command line tokens parsed from app
*	@param	Switches		Command line switches parsed from app
*/
void FGameCookerHelper::Init(
	class UCookPackagesCommandlet* Commandlet, 
	const TArray<FString>& Tokens, 
	const TArray<FString>& Switches )
{
	GetGameContentSeekfreeHelper()->InitOptions(Tokens,Switches);
}

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
UPersistentCookerData* FGameCookerHelper::CreateInstance( const TCHAR* Filename, UBOOL bCreateIfNotFoundOnDisk )
{
	return FGameCookerHelperBase::CreateInstance(Filename,bCreateIfNotFoundOnDisk);
}

/** 
 *	Generate the package list that is specific for the game being cooked.
 *
 *	@param	Commandlet							The cookpackages commandlet being run
 *	@param	Platform							The platform being cooked for
 *	@param	ShaderPlatform						The shader platform being cooked for
 *	@param	NotRequiredFilenamePairs			The package lists being filled in...
 *	@param	RegularFilenamePairs				""
 *	@param	MapFilenamePairs					""
 *	@param	ScriptFilenamePairs					""
 *	@param	StartupFilenamePairs				""
 *	@param	StandaloneSeekfreeFilenamePairs		""
 *	
 *	@return	UBOOL		TRUE if successful, FALSE is something went wrong.
 */
UBOOL FGameCookerHelper::GeneratePackageList( 
	class UCookPackagesCommandlet* Commandlet, 
	UE3::EPlatformType Platform,
	EShaderPlatform ShaderPlatform,
	TArray<FPackageCookerInfo>& NotRequiredFilenamePairs,
	TArray<FPackageCookerInfo>& RegularFilenamePairs,
	TArray<FPackageCookerInfo>& MapFilenamePairs,
	TArray<FPackageCookerInfo>& ScriptFilenamePairs,
	TArray<FPackageCookerInfo>& StartupFilenamePairs,
	TArray<FPackageCookerInfo>& StandaloneSeekfreeFilenamePairs)
{
	UBOOL bSuccess=TRUE;

	// Add to list of seekfree packages needed for standalon game content
	if (GetGameContentSeekfreeHelper()->GeneratePackageList(
		Commandlet,
		Platform,
		ShaderPlatform,
		NotRequiredFilenamePairs,
		RegularFilenamePairs,
		MapFilenamePairs,
		ScriptFilenamePairs,
		StartupFilenamePairs,
		StandaloneSeekfreeFilenamePairs
		) == FALSE)
	{
		bSuccess = FALSE;
	}

	return bSuccess;
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
UBOOL FGameCookerHelper::CookObject(class UCookPackagesCommandlet* Commandlet, UPackage* Package, UObject* Object, UBOOL bIsSavedInSeekFreePackage)
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
UPackage* FGameCookerHelper::LoadPackageForCookingCallback(class UCookPackagesCommandlet* Commandlet, const TCHAR* Filename)
{
	UPackage* Result=NULL;
	
	// load package for standalone seekfree game content for the given filename
	Result = GetGameContentSeekfreeHelper()->LoadPackageForCookingCallback(Commandlet,Filename);

	return Result;
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
UBOOL FGameCookerHelper::PostLoadPackageForCookingCallback(class UCookPackagesCommandlet* Commandlet, UPackage* InPackage)
{
	UBOOL bContinueProcessing = TRUE;

	// post process loaded package for standalone seekfree game content
	if (GetGameContentSeekfreeHelper()->PostLoadPackageForCookingCallback(Commandlet,InPackage) == FALSE)
	{
		bContinueProcessing = FALSE;
	}

	return bContinueProcessing;
}

/**
 *	Clean up the kismet for the given level...
 *	Remove 'danglers' - sequences that don't actually hook up to anything, etc.
 *
 *	@param	Commandlet	The cookpackages commandlet being run
 *	@param	Package		The package being cooked.
 */
void FGameCookerHelper::CleanupKismet(class UCookPackagesCommandlet* Commandlet, UPackage* InPackage)
{
}

/**
 *	Return TRUE if the sound cue should be ignored when generating persistent FaceFX list.
 *
 *	@param	Commandlet		The commandlet being run
 *	@param	InSoundCue		The sound cue of interest
 *
 *	@return	UBOOL			TRUE if the sound cue should be ignored, FALSE if not
 */
UBOOL FGameCookerHelper::ShouldSoundCueBeIgnoredForPersistentFaceFX(class UCookPackagesCommandlet* Commandlet, const USoundCue* InSoundCue)
{
	return FALSE;
}

/**
 *	Dump out stats specific to the game cooker helper.
 */
void FGameCookerHelper::DumpStats()
{
	warnf( NAME_Log, TEXT("") );
	warnf( NAME_Log, TEXT("Game-specific Cooking Stats:") );
	GetGameContentSeekfreeHelper()->DumpStats();
}
