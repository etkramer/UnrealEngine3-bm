/*=============================================================================
	GearEditorCookerHelper.cpp: Gear editor cokking helper class implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#include "GearEditor.h"
#include "GearEditorCookerHelper.h"
#include "EngineSoundClasses.h"

//-----------------------------------------------------------------------------
//	GearsPersistentCookerData
//-----------------------------------------------------------------------------
/**
 * Gears-specific persistent cooker data object...
 */
/**
 *	Checks to see if GUDS need to be updated during the cook
 *
 *	@param	Commandlet		The cookpackages commandlet being run
 *	@param	Tokens			Command line tokens parsed from app
 *	@param	Switches		Command line switches parsed from app
 *	@return	UBOOL			TRUE if they should, FALSE if not
 */
UBOOL FGUDPersistentCookerData::ShouldBeGenerated(class UCookPackagesCommandlet* Commandlet)
{
	// See if the PawnNameToInfoRequired contains any entries at all
	// If won't during a full recook as the PCD would have been deleted...
	if (PawnNameToInfoRequired.Num() == 0)
	{
		return TRUE;
	}

	// Catch the case where someone hasn't cooked w/ the filename pairs stored in the PCD...
	if (StandaloneSeekfreeFilenamePairs.Num() == 0)
	{
		return TRUE;
	}

	// Catch the special GUDS package list...
	if (StandaloneSeekfreeFilenamePairs.Num() != SpecialGUDSPackages.Num())
	{
		return TRUE;
	}

	// Check each Source file in the pairs array still exists...
	for (INT FileIndex = 0; FileIndex < StandaloneSeekfreeFilenamePairs.Num(); FileIndex++)
	{
		FPackageCookerInfo& PCInfo = StandaloneSeekfreeFilenamePairs(FileIndex);

		DOUBLE SrcTimeStamp = GFileManager->GetFileTimestamp(*(PCInfo.SrcFilename));
		if (SrcTimeStamp == -1.0)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/**
 *	Checks to see if GUDS need to be generated during the cook
 *
 *	@param	Commandlet		The cookpackages commandlet being run
 *	@param	bForceRecook	If TRUE, forcibly recook all guds
 *	@param	GUDSCookDir		The destination directory for cooked GUDS
 *	@return	UBOOL			TRUE if they should, FALSE if not
 */
UBOOL FGUDPersistentCookerData::ShouldBeCooked(
	class UCookPackagesCommandlet* Commandlet, 
	UBOOL bForceRecook, 
	FString& GUDSCookDir,
	TArray<FPackageCookerInfo>& OutGUDSToCook)
{
	// The GUDSPersistant cooker data contains the list of filename pairs that were generated...
	// Check the time stamps for each one, and also check the language!
	for (INT PairIndex = 0; PairIndex < StandaloneSeekfreeFilenamePairs.Num(); PairIndex++)
	{
		FPackageCookerInfo& PCInfo = StandaloneSeekfreeFilenamePairs(PairIndex);

		// GUDS are always seekfre...
		FString TrueDstFilenameStr = GUDSCookDir + PCInfo.DstFilename.GetBaseFilename() + STANDALONE_SEEKFREE_SUFFIX;
		TrueDstFilenameStr += (Commandlet->Platform == UE3::PLATFORM_Windows ? TEXT(".upk") : TEXT(".xxx"));
		FFilename TrueDstFilename = TrueDstFilenameStr;

		DOUBLE SrcTimeStamp = GFileManager->GetFileTimestamp(*(PCInfo.SrcFilename));
		DOUBLE DstTimeStamp = GFileManager->GetFileTimestamp(*TrueDstFilename);

		if (SrcTimeStamp == -1.0)
		{
			warnf(NAME_Warning, TEXT("Source GUDS file is not present??? %s"), *(PCInfo.SrcFilename));
			continue;
		}
		if (DstTimeStamp < SrcTimeStamp)
		{
			//warnf(NAME_Warning, TEXT("Destination GUDS file is too old: %s, %s"), *(PCInfo.SrcFilename), *TrueDstFilename);
			OutGUDSToCook.AddItem(PCInfo);
		}
		else if (bForceRecook)
		{
			//warnf(NAME_Warning, TEXT("Forced recook: %s, %s"), *(PCInfo.SrcFilename), *TrueDstFilename);
			OutGUDSToCook.AddItem(PCInfo);
		}
		else
		{
			// Find the localized version of the file...
			FString LocDstFilenameStr = GUDSCookDir + PCInfo.DstFilename.GetBaseFilename() + STANDALONE_SEEKFREE_SUFFIX + LOCALIZED_SEEKFREE_SUFFIX;
			FFilename LocDstFilename = LocDstFilenameStr + (Commandlet->Platform == UE3::PLATFORM_Windows ? TEXT(".upk") : TEXT(".xxx"));
			FString LocFilenameStr = LocDstFilename.GetLocalizedFilename(UObject::GetLanguage());
			DOUBLE DstLocTimeStamp = GFileManager->GetFileTimestamp(*LocFilenameStr);
			if (DstLocTimeStamp < SrcTimeStamp)
			{
				//warnf(NAME_Warning, TEXT("Localized GUDS file is too old: %s, %s"), *(PCInfo.SrcFilename), *(PCInfo.DstFilename));
				OutGUDSToCook.AddItem(PCInfo);
			}
		}
	}

	return (OutGUDSToCook.Num() > 0);
}

/**
 * Serialize function.
 *
 * @param	Ar	Archive to serialize with.
 */
void FGUDPersistentCookerData::Serialize(FArchive& Ar)
{
	Ar << SourceGUDBanks;
	Ar << PawnNameToInfoRequired;
	Ar << ReferencedFaceFXAnimations;
	Ar << ReferencedSoundCues;
	Ar << TableOfContents;
	Ar << SpecialGUDSPackages;
	Ar << StandaloneSeekfreeFilenamePairs;
}

// Serializer.
FArchive& operator<<(FArchive& Ar, FGUDPersistentCookerData& GUDPCD)
{
	return Ar 
		<< GUDPCD.SourceGUDBanks 
		<< GUDPCD.PawnNameToInfoRequired 
		<< GUDPCD.ReferencedFaceFXAnimations 
		<< GUDPCD.ReferencedSoundCues 
		<< GUDPCD.TableOfContents
		<< GUDPCD.SpecialGUDSPackages
		<< GUDPCD.StandaloneSeekfreeFilenamePairs;
}

/**
 * Create an instance of this class given a filename. First try to load from disk and if not found
 * will construct object and store the filename for later use during saving.
 *
 * @param	Filename					Filename to use for serialization
 * @param	bCreateIfNotFoundOnDisk		If FALSE, don't create if couldn't be found; return NULL.
 * @return								instance of the container associated with the filename
 */
UPersistentCookerData* UGearsPersistentCookerData::CreateInstance( const TCHAR* Filename, UBOOL bCreateIfNotFoundOnDisk )
{
	return Cast<UPersistentCookerData>(CreateGearsInstance(Filename, bCreateIfNotFoundOnDisk));
}

UGearsPersistentCookerData* UGearsPersistentCookerData::CreateGearsInstance( const TCHAR* Filename, UBOOL bCreateIfNotFoundOnDisk )
{
	UGearsPersistentCookerData* Instance = NULL;

	// Find it on disk first.
	if( !Instance )
	{
		// Try to load the package.
		static INT BulkDataIndex = 0;
		FString Temp = FString::Printf(TEXT("BulkData_%d"), BulkDataIndex++);
		UPackage* BulkDataPackage = CreatePackage(NULL, *Temp);
		UPackage* Package = LoadPackage( BulkDataPackage, Filename, LOAD_NoWarn );

		// Find in memory if package loaded successfully.
		if( Package )
		{
			Instance = FindObject<UGearsPersistentCookerData>( Package, TEXT("PersistentCookerData") );
		}
	}

	// If not found, create an instance.
	if ( !Instance && bCreateIfNotFoundOnDisk )
	{
		UPackage* Package = UObject::CreatePackage( NULL, NULL );
		Instance = ConstructObject<UGearsPersistentCookerData>( 
							UGearsPersistentCookerData::StaticClass(),
							Package,
							TEXT("PersistentCookerData")
							);
		// Mark package as cooked as it is going to be loaded at run-time and has xxx extension.
		Package->PackageFlags |= PKG_Cooked;
		check( Instance );
	}


	// Keep the filename around for serialization and add to root to prevent garbage collection.
	if ( Instance )
	{
		Instance->Filename = Filename;
		Instance->AddToRoot();
	}

	return Instance;
}

/**
 * Saves the data to disk.
 */
void UGearsPersistentCookerData::SaveToDisk()
{
	Super::SaveToDisk();
}

/**
 * Serialize function.
 *
 * @param	Ar	Archive to serialize with.
 */
void UGearsPersistentCookerData::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	Ar << GUDPersistentData;
}

/**
 *	Generate the table of contents object.\
 *
 *	@return	UBOOL		TRUE if successful, FALSE otherwise.
 */
UBOOL UGearsPersistentCookerData::CreateTableOfContents()
{
	if (GUDPersistentData.TableOfContents == NULL)
	{
		UPackage* OuterMost = Cast<UPackage>(GetOutermost());
		GUDPersistentData.TableOfContents = Cast<UGUDToC>(UObject::StaticConstructObject(UGUDToC::StaticClass(), OuterMost, UGUDToC::ToCObjectName));
		check(GUDPersistentData.TableOfContents);
		GUDPersistentData.TableOfContents->SetFlags(RF_Standalone);
	}

	return TRUE;
}

IMPLEMENT_CLASS(UGearsPersistentCookerData);

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
		TArray<FPackageCookerInfo>& MPMapFilenamePairs,
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
class FGenerateGameContentSeekfree* FGearGameCookerHelper::GetGameContentSeekfreeHelper()
{
	static FGenerateGameContentSeekfree MapPreloadHelper;
	return &MapPreloadHelper;
}

class FGearGameGUDSCookerHelper* FGearGameCookerHelper::GetGUDSCooker()
{
	static FGearGameGUDSCookerHelper GUDSCooker;
	return &GUDSCooker;
}

//-----------------------------------------------------------------------------
//	GUDSCookerHelper
//-----------------------------------------------------------------------------
#if WITH_FACEFX
using namespace OC3Ent;
using namespace Face;
#endif // WITH_FACEFX

/** Helper macros */
#define DEBUG_GUDSCOOK_MESSAGES		0
#if DEBUG_GUDSCOOK_MESSAGES
#define GUDSCookDebugf		debugf
#else
#define	GUDSCookDebugf		__noop
#endif

static DOUBLE	GUDS_TotalTime;
static DOUBLE	GUDS_InitTime;
static DOUBLE	GUDS_ListGenerationTime;
static DOUBLE	GUDS_PackageGenerationTime;
static DOUBLE	GUDS_CookObjectTime;
static DOUBLE	GUDS_PostLoadPackageTime;
static DOUBLE	GUDS_GenerateGUDSPackageTime;
static DOUBLE	GUDS_SaveGUDSPackageTime;

const FString GUDGenerationStartFileName = TEXT("_GUD_Start.txt");
const FString GUDGenerationEndFileName = TEXT("_GUD_End.txt");

/** 
 *	Helper class for cooking GUDS audio
 */
/**
 * Initialize the cooker helpr and process any command line params
 *
 *	@param	Commandlet		The cookpackages commandlet being run
 *	@param	Tokens			Command line tokens parsed from app
 *	@param	Switches		Command line switches parsed from app
 */
void FGearGameGUDSCookerHelper::Init(
	class UCookPackagesCommandlet* Commandlet, 
	const TArray<FString>& Tokens, 
	const TArray<FString>& Switches )
{
	SCOPE_SECONDS_COUNTER(GUDS_TotalTime);
	SCOPE_SECONDS_COUNTER(GUDS_InitTime);
	if (Commandlet->bIsInUserMode)
	{
		return;
	}

	GUDSOutDir = appGameDir() + TEXT("Content") + PATH_SEPARATOR + TEXT("GeneratedGUDS\\");

	bAbortGUDSCooking = FALSE;
	// Check for flag to recook seekfree GUDS
	bForceRecookSeekfreeGUDS = Switches.ContainsItem(TEXT("RECOOKSEEKFREEGUDS"));
	bForceRegenerateGUDS = Switches.ContainsItem(TEXT("REGENERATEGUDS"));

	// List non-GUDS references to GUDS sound cues...
	bListGUDSSoundCueReferences = Switches.ContainsItem(TEXT("LISTGUDSREFS"));

	// Grab the persistent data info
	UGearsPersistentCookerData* GearsPCD = Cast<UGearsPersistentCookerData>(Commandlet->PersistentCookerData);
	check(GearsPCD);

	// See if the GUDS should be generated...
	if (ShouldBeGenerated(Commandlet, GearsPCD) == TRUE)
	{
		// Set the flag to indicate that GUDS were regenerated...
		bForceRegenerateGUDS = TRUE;
		warnf(NAME_Log, TEXT("Generating GUDS packages..."));
		if (GenerateGUDSPackages(Commandlet, GearsPCD) == TRUE)
		{
			// If we are regenerating, then we must recook!
			bForceRecookSeekfreeGUDS = TRUE;
		}
	}

	if (bForceRecookSeekfreeGUDS == FALSE)
	{
		// Store off the TableOfContents pointer for cooking into the startup packages.
		GUDToC = GearsPCD->GUDPersistentData.TableOfContents;
		if (GUDToC)
		{
			GUDToC->SetFlags(RF_Standalone|RF_MarkedByCooker);
			GUDToC->AddToRoot();
		}
	}
}

/**
 *	Helper function that removes any GeneratedGUDS packages from the given pairs list.
 */
void GUDSCOOKEDHELPER_RemoveGUDSPackagesFromFilenamePairs(TArray<FPackageCookerInfo>& FilenamePairs)
{
	// Make sure we don't cook any temporary packages!
	for (INT FPIndex = FilenamePairs.Num() - 1; FPIndex >= 0; FPIndex--)
	{
		FPackageCookerInfo& PkgCookInfo = FilenamePairs(FPIndex);
		if (PkgCookInfo.SrcFilename.InStr(TEXT("GeneratedGUDS")) != -1)
		{
			FilenamePairs.Remove(FPIndex);
		}
	}
}

/** 
 *	Generate the package list for audio-related files.
 *
 *	@param	Commandlet							The cookpackages commandlet being run
 *	@param	Platform							The platform being cooked for
 *	@param	ShaderPlatform						The shader platform being cooked for
 *	@param	NotRequiredFilenamePairs			The package lists being filled in...
 *	@param	RegularFilenamePairs				""
 *	@param	MapFilenamePairs					""
 *	@param	MPMapFilenamePairs					""
 *	@param	ScriptFilenamePairs					""
 *	@param	StartupFilenamePairs				""
 *	@param	StandaloneSeekfreeFilenamePairs		""
 *	
 *	@return	UBOOL		TRUE if successfull, FALSE is something went wrong.
 */
UBOOL FGearGameGUDSCookerHelper::GeneratePackageList( 
	class UCookPackagesCommandlet* Commandlet, 
	UE3::EPlatformType Platform,
	EShaderPlatform ShaderPlatform,
	TArray<FPackageCookerInfo>& NotRequiredFilenamePairs,
	TArray<FPackageCookerInfo>& RegularFilenamePairs,
	TArray<FPackageCookerInfo>& MapFilenamePairs,
	TArray<FPackageCookerInfo>& MPMapFilenamePairs,
	TArray<FPackageCookerInfo>& ScriptFilenamePairs,
	TArray<FPackageCookerInfo>& StartupFilenamePairs,
	TArray<FPackageCookerInfo>& StandaloneSeekfreeFilenamePairs)
{
	SCOPE_SECONDS_COUNTER(GUDS_TotalTime);
	SCOPE_SECONDS_COUNTER(GUDS_ListGenerationTime);
	if (Commandlet->bIsInUserMode)
	{
		return TRUE;
	}

	// Remove GUDS files from the lists...
	GUDSCOOKEDHELPER_RemoveGUDSPackagesFromFilenamePairs(NotRequiredFilenamePairs);
	GUDSCOOKEDHELPER_RemoveGUDSPackagesFromFilenamePairs(RegularFilenamePairs);
	GUDSCOOKEDHELPER_RemoveGUDSPackagesFromFilenamePairs(MapFilenamePairs);
	GUDSCOOKEDHELPER_RemoveGUDSPackagesFromFilenamePairs(MPMapFilenamePairs);
	GUDSCOOKEDHELPER_RemoveGUDSPackagesFromFilenamePairs(ScriptFilenamePairs);
	GUDSCOOKEDHELPER_RemoveGUDSPackagesFromFilenamePairs(StartupFilenamePairs);
	GUDSCOOKEDHELPER_RemoveGUDSPackagesFromFilenamePairs(StandaloneSeekfreeFilenamePairs);

	// Create folder for generated data.
	FString ToCFilename = GUDSOutDir + UGUDToC::ToCPackageName + TEXT(".upk");

	// Always add the ToC package to the filename list
	ToCSrcFilename = FFilename(ToCFilename);
	FFilename ToCDstFilename = Commandlet->GetCookedPackageFilename(ToCSrcFilename);
	StartupFilenamePairs.AddItem(FPackageCookerInfo(*ToCSrcFilename, *ToCDstFilename, TRUE, TRUE, TRUE, FALSE));

	// Did initialization fail?
	if (bAbortGUDSCooking == TRUE)
	{
		warnf(TEXT("GUDS cooking is being aborted!"));
		return FALSE;
	}

	// Grab the persistent data info
	UGearsPersistentCookerData* GearsPCD = Cast<UGearsPersistentCookerData>(Commandlet->PersistentCookerData);
	check(GearsPCD);

	TArray<FPackageCookerInfo> GUDSToCook;
	FString OutDir = FString::Printf(TEXT("%s%s\\"), *appGameDir(), *(Commandlet->GetCookedDirectory()));
	if (ShouldBeCooked(Commandlet, bForceRecookSeekfreeGUDS, OutDir, GearsPCD, GUDSToCook) == TRUE)
	{
		warnf(NAME_Log, TEXT("Cooking %3d of %3d total GUDS packages..."),
			GUDSToCook.Num(), GearsPCD->GUDPersistentData.StandaloneSeekfreeFilenamePairs.Num());
		//@todo.SAS. Only add the filenames of those that need to be cooked...
		StandaloneSeekfreeFilenamePairs += GUDSToCook;
	}

	if (bLogGUDSGeneration)
	{
		debugf(NAME_Log, TEXT("Dumping GUDS TableOfContents for %s:"), UObject::GetLanguage());
		for (TMap<FString,FGUDCollection>::TIterator ToCIt(GUDToC->TableOfContents); ToCIt; ++ToCIt)
		{
			debugf(NAME_Log, TEXT("\tGUD Class: %s"), *(ToCIt.Key()));
			FGUDCollection& Collection = ToCIt.Value();
			debugf(NAME_Log, TEXT("\t\t%s"), *(Collection.RootGUDBank.BankName));
			for (INT DumpIndex = 0; DumpIndex < Collection.GUDBanks.Num(); DumpIndex++)
			{
				FGUDToCEntry& TOCEntry = Collection.GUDBanks(DumpIndex);
				debugf(NAME_Log, TEXT("\t\t%s"), *(TOCEntry.BankName));
			}
		}
	}

	return TRUE;
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
UBOOL FGearGameGUDSCookerHelper::CookObject(class UCookPackagesCommandlet* Commandlet, UPackage* Package, UObject* Object, UBOOL bIsSavedInSeekFreePackage)
{
	SCOPE_SECONDS_COUNTER(GUDS_TotalTime);
	SCOPE_SECONDS_COUNTER(GUDS_CookObjectTime);
	check(Object);

	USoundCue* SoundCue = Cast<USoundCue>(Object);
	if (SoundCue)
	{
		return CookSoundCue(Commandlet, Package, SoundCue, bIsSavedInSeekFreePackage);
	}

	USoundNodeWave* SoundNodeWave = Cast<USoundNodeWave>(Object);
	if (SoundNodeWave && bCookingSpecialGUDSPackage)
	{
		int dummy = 0;
	}
#if WITH_FACEFX
	if ((Object->IsA(UFaceFXAsset::StaticClass()))|| 
		(Object->IsA(UFaceFXAnimSet::StaticClass())))
	{
		return CookFaceFXObject(Commandlet, Package, Object, bIsSavedInSeekFreePackage);
	}
#endif // WITH_FACEFX

	return TRUE;
}

/**
 * Cooks the sound cue object.
 *
 *	@param	Commandlet					The cookpackages commandlet being run
 *	@param	Package						Package going to be saved
 *	@param	InSoundCue					Soundcue to cook
 *	@param	bIsSavedInSeekFreePackage	Whether object is going to be saved into a seekfree package
 *
 *	@return	UBOOL						TRUE if the object should continue the 'normal' cooking operations.
 *										FALSE if the object should not be processed any further.
 */
UBOOL FGearGameGUDSCookerHelper::CookSoundCue(class UCookPackagesCommandlet* Commandlet, UPackage* Package, USoundCue* InSoundCue, UBOOL bIsSavedInSeekFreePackage)
{
	check(InSoundCue);

	// Grab the persistent data info
	UGearsPersistentCookerData* GearsPCD = Cast<UGearsPersistentCookerData>(Commandlet->PersistentCookerData);
	check(GearsPCD);

	TMap<FString, INT>* SoundMap = GearsPCD->GUDPersistentData.ReferencedSoundCues.Find(InSoundCue->GetPathName());
	if (SoundMap != NULL)
	{
		INT* PkgIsInSoundMap = SoundMap->Find(Package->GetName());
		if ((PkgIsInSoundMap == NULL) || (bCookingSpecialGUDSPackage == FALSE))
		{
			if (PkgIsInSoundMap)
			{
				GUDSCookDebugf(TEXT("FOUND IN Package %s: %s"), *(Package->GetName()), *(InSoundCue->GetPathName()));
			}
			
			if (bCookingSpecialGUDSPackage == FALSE)
			{
				warnf(NAME_Warning, TEXT("SoundCue will not be included due to already being referenced by guds: %s %s"), *Package->GetName(), *(InSoundCue->GetPathName()));
				if (bListGUDSSoundCueReferences)
				{
					// Dump out the references to the soundcue...
					// NOTE: This is the exact code from "obj refs"
					FStringOutputDevice TempAr;
					InSoundCue->OutputReferencers(TempAr,TRUE);

					TArray<FString> Lines;
					TempAr.ParseIntoArray(&Lines, LINE_TERMINATOR, 0);
					for ( INT i = 0; i < Lines.Num(); i++ )
					{
						warnf(NAME_Warning, TEXT("\t%s"), *Lines(i));
					}
				}
			}

			InSoundCue->ClearFlags(RF_ForceTagExp);
			InSoundCue->SetFlags(RF_MarkedByCooker);
			TArray<USoundNodeAttenuation*> OutAttens;
			TArray<USoundNodeWave*> OutWaves;

			InSoundCue->RecursiveFindAttenuation(InSoundCue->FirstNode, OutAttens);
			InSoundCue->RecursiveFindWaves(InSoundCue->FirstNode, OutWaves);

			for (INT AttenIndex = 0; AttenIndex < OutAttens.Num(); AttenIndex++)
			{
				USoundNodeAttenuation* AttnNode = OutAttens(AttenIndex);
				if (AttnNode)
				{
					AttnNode->ClearFlags(RF_ForceTagExp);
					AttnNode->SetFlags(RF_MarkedByCooker);
				}
			}
			for (INT WaveIndex = 0; WaveIndex < OutWaves.Num(); WaveIndex++)
			{
				USoundNodeWave* WaveNode = OutWaves(WaveIndex);
				if (WaveNode)
				{
					warnf(NAME_Warning, TEXT("\tClearing SoundNodeWave %s"), *(WaveNode->GetName()));
					WaveNode->ClearFlags(RF_ForceTagExp);
					WaveNode->SetFlags(RF_MarkedByCooker);
				}
			}
		}
		else
		{
			GUDSCookDebugf(TEXT("FOUND IN Package %s: %s"), *(Package->GetName()), *(InSoundCue->GetPathName()));
			InSoundCue->ClearFlags(RF_MarkedByCooker);
			InSoundCue->SetFlags( RF_ForceTagExp );
			
			TArray<USoundNodeAttenuation*> OutAttens;
			TArray<USoundNodeWave*> OutWaves;

			InSoundCue->RecursiveFindAttenuation(InSoundCue->FirstNode, OutAttens);
			InSoundCue->RecursiveFindWaves(InSoundCue->FirstNode, OutWaves);

			for (INT AttenIndex = 0; AttenIndex < OutAttens.Num(); AttenIndex++)
			{
				USoundNodeAttenuation* AttnNode = OutAttens(AttenIndex);
				if (AttnNode)
				{
					AttnNode->ClearFlags(RF_MarkedByCooker);
					AttnNode->SetFlags( RF_ForceTagExp );
				}
			}
			for (INT WaveIndex = 0; WaveIndex < OutWaves.Num(); WaveIndex++)
			{
				USoundNodeWave* WaveNode = OutWaves(WaveIndex);
				if (WaveNode)
				{
					WaveNode->ClearFlags(RF_MarkedByCooker);
					WaveNode->SetFlags( RF_ForceTagExp );
				}
			}

			// Fix up the sound cue FaceFXGroupName
			if (InSoundCue->FaceFXAnimName != TEXT(""))
			{
				GUDSCookDebugf(TEXT("\tFixing up FFXGroupName: Was %s, Setting to %s"), *(InSoundCue->FaceFXGroupName), *PackageBeingCooked);
				InSoundCue->FaceFXGroupName = PackageBeingCooked;
			}
		}
	}
	else
	{
		GUDSCookDebugf(TEXT("SoundCue: NOT IN REFERENCEDSOUNDCUES %s"), *(InSoundCue->GetPathName()));
	}

	return TRUE;
}


#if WITH_FACEFX

/**
 * Cooks FaceFX* object if it hasn't been already.
 *
 *	@param	Commandlet					The cookpackages commandlet being run
 *	@param	Package						Package going to be saved
 *	@param	Object						Object to cook
 *	@param	bIsSavedInSeekFreePackage	Whether object is going to be saved into a seekfree package
 *
 *	@return	UBOOL						TRUE if the object should continue the 'normal' cooking operations.
 *										FALSE if the object should not be processed any further.
 */
UBOOL FGearGameGUDSCookerHelper::CookFaceFXObject(class UCookPackagesCommandlet* Commandlet, UPackage* Package, UObject* Object, UBOOL bIsSavedInSeekFreePackage)
{
	check(Object);

	if (bCookingSpecialGUDSPackage == TRUE)
	{
		return TRUE;
	}

	UFaceFXAsset* FaceFXAsset = Cast<UFaceFXAsset>(Object);
	UFaceFXAnimSet* FaceFXAnimSet = Cast<UFaceFXAnimSet>(Object);

	if (!FaceFXAsset && !FaceFXAnimSet)
	{
		warnf(TEXT("Object is not a FaceFX object: %s"), *(Object->GetPathName()));
		return TRUE;
	}

	UBOOL bIsAsset = FaceFXAsset ? TRUE : FALSE;

	GUDSCookDebugf(TEXT("GameCooker:CookFaceFXObject> %15s : %s"), *(Object->GetClass()->GetName()), *(Object->GetPathName()));
	TGUDSReferencedFaceFXAnimations* RefdAnims = GetReferencedAnimationsForObject(Commandlet, Object, FALSE);
	if (RefdAnims)
	{
		GUDSCookDebugf(TEXT("\tHad a referenced anim object!"));

		OC3Ent::Face::FxActor* FaceFXActor = NULL;
		OC3Ent::Face::FxAnimSet* FXAnimSet = NULL;

		// Grab the FxAnimGroup
		// We need to remove the animations from the data set...
		GUDSCookDebugf(TEXT("\tRemoving %3d animations from this object!"), RefdAnims->Num());

		if (bIsAsset)
		{
			FaceFXActor = FaceFXAsset->GetFxActor();
		}
		else
		{
			FXAnimSet = FaceFXAnimSet->GetFxAnimSet();
		}

		for (INT RefAnimIndex = 0; RefAnimIndex < RefdAnims->Num(); RefAnimIndex++)
		{
			FGUDSReferencedFaceFXAnimation& RefdAnim = (*RefdAnims)(RefAnimIndex);

			FxName AGName(TCHAR_TO_ANSI(*(RefdAnim.AnimGroup)));
			FxName AnimName(TCHAR_TO_ANSI(*(RefdAnim.AnimName)));
			
			FxAnimGroup* FFXAnimGroup = NULL;
			if (bIsAsset)
			{
				FxSize AGIndex = FaceFXActor->FindAnimGroup(AGName);
				if (AGIndex != FxInvalidIndex)
				{
					FxAnimGroup& TempFFXAnimGroup = FaceFXActor->GetAnimGroup(AGIndex);
					FFXAnimGroup = &TempFFXAnimGroup;
				}
			}
			else
			{
				const FxAnimGroup& TempFFXAnimGroup = FXAnimSet->GetAnimGroup();
				FFXAnimGroup = (FxAnimGroup*)&TempFFXAnimGroup;
			}

			if ( FFXAnimGroup != NULL )
			{
				FxSize AnimIndex = FFXAnimGroup->FindAnim(AnimName);
				if (AnimIndex != FxInvalidIndex)
				{
					const FxAnim& FXAnim = FFXAnimGroup->GetAnim(AnimIndex);
					if (FFXAnimGroup->RemoveAnim(AnimName) == FALSE)
					{
						warnf(TEXT("GUDSCooker: Failed to remove anim %s.%s from %s"),
							*(RefdAnim.AnimGroup),
							*(RefdAnim.AnimName),
							*(Object->GetPathName()));
					}
				}
				else
				{
					warnf(TEXT("GUDSCooker: Failed to find anim %s.%s in %s"),
						*(RefdAnim.AnimGroup),
						*(RefdAnim.AnimName),
						*(Object->GetPathName()));
				}
			}
			else
			{
				warnf(TEXT("GUDSCooker: Failed to find anim group %s.%s in %s"), *RefdAnim.AnimGroup, *RefdAnim.AnimName, *Object->GetPathName());
			}
		}

		if (bIsAsset)
		{
			FaceFXAsset->ReferencedSoundCues.Empty();
			FaceFXAsset->FixupReferencedSoundCues();
		}
		else
		{
			FaceFXAnimSet->ReferencedSoundCues.Empty();
			FaceFXAnimSet->FixupReferencedSoundCues();
		}
	}
	else
	{
		GUDSCookDebugf(TEXT("\tDid NOT have a referenced anim object!"));
	}

	return TRUE;
}

#endif // WITH_FACEFX

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
UPackage* FGearGameGUDSCookerHelper::LoadPackageForCookingCallback(class UCookPackagesCommandlet* Commandlet, const TCHAR* Filename)
{
	SCOPE_SECONDS_COUNTER(GUDS_TotalTime);
	SCOPE_SECONDS_COUNTER(GUDS_PackageGenerationTime);

	// Grab the persistent data info
	UGearsPersistentCookerData* GearsPCD = Cast<UGearsPersistentCookerData>(Commandlet->PersistentCookerData);
	check(GearsPCD);

	bCookingSpecialGUDSPackage = FALSE;
	FString* PackageName = GearsPCD->GUDPersistentData.SpecialGUDSPackages.Find(FString(Filename));
	if (PackageName != NULL)
	{
		GUDSCookDebugf(TEXT("Cooking special GUDS package: %s (%s)"), *(*PackageName), Filename);
		PackageBeingCooked = *PackageName;
		bCookingSpecialGUDSPackage = TRUE;
	}
	else
	if ((UGUDToC::ToCPackageName == Filename) || (ToCSrcFilename == Filename))
	{
		// Grab the persistent data info
		UGearsPersistentCookerData* GearsPCD = Cast<UGearsPersistentCookerData>(Commandlet->PersistentCookerData);
		check(GearsPCD);

		UPackage* ToCPackage = UObject::CreatePackage(NULL, *(UGUDToC::ToCPackageName));
		check(ToCPackage);
		UGUDToC* NewToC = Cast<UGUDToC>(UObject::StaticConstructObject(UGUDToC::StaticClass(), ToCPackage, UGUDToC::ToCObjectName));
		check(NewToC);
		NewToC->TableOfContents = GearsPCD->GUDPersistentData.TableOfContents->TableOfContents;
		NewToC->SetFlags(RF_Standalone);
		NewToC->ClearFlags(RF_MarkedByCooker);

		return ToCPackage;
	}

	return NULL;
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
UBOOL FGearGameGUDSCookerHelper::PostLoadPackageForCookingCallback(class UCookPackagesCommandlet* Commandlet, UPackage* InPackage)
{
	SCOPE_SECONDS_COUNTER(GUDS_TotalTime);
	SCOPE_SECONDS_COUNTER(GUDS_PostLoadPackageTime);
	GUDSCookDebugf(TEXT("PostLoadPackageForCookingCallback: %s"), InPackage ? *(InPackage->GetName()) : TEXT("?????"));

	if ((bCookingSpecialGUDSPackage == FALSE) &&
		((InPackage->GetName().InStr("GearGame") != -1) ||
		 (InPackage->GetName().InStr("GearGameContent") != -1) ||
		 (InPackage->GetName().InStr("Startup") != -1))
		 )
	{
		// Grab the persistent data info
		UGearsPersistentCookerData* GearsPCD = Cast<UGearsPersistentCookerData>(Commandlet->PersistentCookerData);
		check(GearsPCD);

		for (FObjectIterator It; It; ++It)
		{
			if (It->IsIn(InPackage))
			{
				UClass* ClassObj = Cast<UClass>(*It);
				UGUDBank* GUDBank = Cast<UGUDBank>(*It);
				INT* RemoveThem = NULL;

				if (ClassObj)
				{
					// GUDBank?
					if (ClassObj && ClassObj->IsChildOf(UGUDBank::StaticClass()))
					{
						RemoveThem = GearsPCD->GUDPersistentData.SourceGUDBanks.Find(ClassObj->GetName());
						GUDBank = Cast<UGUDBank>(ClassObj->GetDefaultObject());
					}
				}

				if (GUDBank)
				{
					if ((RemoveThem == NULL) && (ClassObj == NULL))
					{
						RemoveThem = GearsPCD->GUDPersistentData.SourceGUDBanks.Find(GUDBank->GetClass()->GetName());
					}

					if (RemoveThem != NULL)
					{
						// Clear out the references to actions and lines to prevent them being referenced.
						for (INT LineIndex = 0; LineIndex < GUDBank->GUDLines.Num(); LineIndex++)
						{
							if (GUDBank->GUDLines(LineIndex).Audio)
							{
								// Make sure these don't serialize...
								GUDBank->GUDLines(LineIndex).Audio->SetFlags(RF_MarkedByCooker);
							}
						}
						GUDBank->GUDActions.Empty();
						GUDBank->GUDLines.Empty();
					}
				}
			}
		}
	}

	if (bCookingSpecialGUDSPackage == TRUE)
	{
		// Grab the persistent data info
		UGearsPersistentCookerData* GearsPCD = Cast<UGearsPersistentCookerData>(Commandlet->PersistentCookerData);
		check(GearsPCD);

		// Iterate over all sound cues and if they aren't in the 'to cook' list, tag their child nodes as marked by cooker.
		for (TObjectIterator<USoundCue> SoundCueIt; SoundCueIt; ++SoundCueIt)
		{
			USoundCue* SoundCue = *SoundCueIt;

			UBOOL bDoNotCook = FALSE;
			TMap<FString, INT>* SoundMap = GearsPCD->GUDPersistentData.ReferencedSoundCues.Find(SoundCue->GetPathName());
			if (SoundMap != NULL)
			{
				if (InPackage->GetName().InStr(TEXT("GUD_")) != -1)
				{
					int usdsh = 0;
				}

				INT* PkgIsInSoundMap = SoundMap->Find(InPackage->GetName());
				if (PkgIsInSoundMap == NULL)
				{
					bDoNotCook = TRUE;
				}
			}
			else
			{
				bDoNotCook = TRUE;
			}

			if (bDoNotCook == TRUE)
			{
				TArray<USoundNodeAttenuation*> OutAttens;
				TArray<USoundNodeWave*> OutWaves;

				SoundCue->RecursiveFindAttenuation(SoundCue->FirstNode, OutAttens);
				SoundCue->RecursiveFindWaves(SoundCue->FirstNode, OutWaves);

				for (INT AttenIndex = 0; AttenIndex < OutAttens.Num(); AttenIndex++)
				{
					USoundNodeAttenuation* AttnNode = OutAttens(AttenIndex);
					if (AttnNode)
					{
						AttnNode->ClearFlags(RF_ForceTagExp);
						AttnNode->SetFlags(RF_MarkedByCooker);
					}
				}
				for (INT WaveIndex = 0; WaveIndex < OutWaves.Num(); WaveIndex++)
				{
					USoundNodeWave* WaveNode = OutWaves(WaveIndex);
					if (WaveNode)
					{
						WaveNode->ClearFlags(RF_ForceTagExp);
						WaveNode->SetFlags(RF_MarkedByCooker);
					}
				}
			}
		}
	}

	return TRUE;
}

/**
 *	Return TRUE if the sound cue should be ignored when generating persistent FaceFX list.
 *
 *	@param	Commandlet		The commandlet being run
 *	@param	InSoundCue		The sound cue of interest
 *
 *	@return	UBOOL			TRUE if the sound cue should be ignored, FALSE if not
 */
UBOOL FGearGameGUDSCookerHelper::ShouldSoundCueBeIgnoredForPersistentFaceFX(class UCookPackagesCommandlet* Commandlet, const USoundCue* InSoundCue)
{
	// Grab the persistent data info
	UGearsPersistentCookerData* GearsPCD = Cast<UGearsPersistentCookerData>(Commandlet->PersistentCookerData);
	check(GearsPCD);

	TMap<FString, INT>* SoundMap = GearsPCD->GUDPersistentData.ReferencedSoundCues.Find(InSoundCue->GetPathName());
	if (SoundMap != NULL)
	{
		return TRUE;
	}

	return FALSE;
}

/**
 *	Dump out stats specific to the game cooker helper.
 */
void FGearGameGUDSCookerHelper::DumpStats()
{
	warnf( NAME_Log, TEXT("") );
	warnf( NAME_Log, TEXT("  GUDS Generation Stats:") );
	warnf( NAME_Log, TEXT("    Total time                             %7.2f seconds"), GUDS_TotalTime );
	warnf( NAME_Log, TEXT("    Init time                              %7.2f seconds"), GUDS_InitTime );
	warnf( NAME_Log, TEXT("    List generation time                   %7.2f seconds"), GUDS_ListGenerationTime );
	warnf( NAME_Log, TEXT("    Package generation time                %7.2f seconds"), GUDS_PackageGenerationTime );
	warnf( NAME_Log, TEXT("    Cook object time                       %7.2f seconds"), GUDS_CookObjectTime );
	warnf( NAME_Log, TEXT("    PostLoad package time                  %7.2f seconds"), GUDS_PostLoadPackageTime );
	warnf( NAME_Log, TEXT("    Generate GUDS package time             %7.2f seconds"), GUDS_GenerateGUDSPackageTime );
	warnf( NAME_Log, TEXT("    Save GUDS package time                 %7.2f seconds"), GUDS_SaveGUDSPackageTime );
}

/**
 *	Generate the GUD bank packages...
 *
 *	@param	Commandlet		The cookpackages commandlet being run
 *	@param	GearsPCD		The cooker persistent data to fill in...
 *	
 *	@return	UBOOL			TRUE if successful.
 */
UBOOL FGearGameGUDSCookerHelper::GenerateGUDSPackages(
	class UCookPackagesCommandlet* Commandlet, 
	UGearsPersistentCookerData* GearsPCD)
{
	// Clear out the GUDS if we are regenerating them...
	ClearGeneratedGUDSFiles();

	// Check for flag to cook rootbank only
	bGenerateRootBankOnly = Commandlet->Switches.ContainsItem(TEXT("GUDSROOTONLY"));
	// CHeck if the generation should be logged.
	bLogGUDSGeneration = Commandlet->Switches.ContainsItem(TEXT("LOGGUDSGENERATION"));

	// Read in the settings
	const TCHAR* GUDGenIniSection = TEXT("Cooker.GUDGeneration");
	if (GConfig->GetInt(GUDGenIniSection, TEXT("VarietyLineCount"), VarietyLineCount, GEditorIni) == FALSE)
	{
		VarietyLineCount = 48;
	}

	// See if the directory exists...
	FString ToCFilename = GUDSOutDir + UGUDToC::ToCPackageName + TEXT(".upk");
	UBOOL bDirectoryDoesntExist = FALSE;
	if (GFileManager->GetFileTimestamp(*ToCFilename) == -1.0)
	{
		// Assume that it does not exist...
		bDirectoryDoesntExist = TRUE;
	}

	// Touch the start file to track GUDS generation...
	TouchGenerationTrackingFile(TRUE);

	// Fill in the PCD data - it is used in cooking non-GUDs packages!
	if (GeneratePersistentCookerData(Commandlet, GearsPCD) == FALSE)
	{
		appDebugMessagef(TEXT("Failed to generate persistent cooker data for GUDS..."));
		bAbortGUDSCooking = TRUE;
	}

	// Generate the actual files now...
	// Did initialization fail?
	if (bAbortGUDSCooking == TRUE)
	{
		warnf(TEXT("GUDS cooking is being aborted!"));
		return FALSE;
	}

	// For now, skip out if not forcing recook and the generated directory exists
	if ((bForceRecookSeekfreeGUDS == FALSE) && (bDirectoryDoesntExist == FALSE))
	{
		GUDSCookDebugf(TEXT("*** Skipping GUDS generation and cooking"));
		return TRUE;
	}

	if (bDirectoryDoesntExist == TRUE)
	{
		// Create it...
		if( !GFileManager->MakeDirectory( *GUDSOutDir, TRUE ) )
		{
			appDebugMessagef(TEXT("Couldn't create %s"), *GUDSOutDir);
			return FALSE;
		}
	}

	// Grab the persistent data info
	ProcessedGUDBanks.Empty();
	for (TMap<FString, FGUDCueAndFaceFX>::TIterator PawnInfoIt(GearsPCD->GUDPersistentData.PawnNameToInfoRequired); PawnInfoIt; ++PawnInfoIt)
	{
		FString& PawnName = PawnInfoIt.Key();
		FGUDCueAndFaceFX& GUDCAFF = PawnInfoIt.Value();

#if DEBUG_GUDSCOOK_MESSAGES
		GUDSCookDebugf(TEXT("Pawn %s"), *PawnName);
		GUDSCookDebugf(TEXT("\tGUD Banks:"));
		for (INT GBNIndex = 0; GBNIndex < GUDCAFF.GUDBankNames.Num(); GBNIndex++)
		{
			GUDSCookDebugf(TEXT("\t\t%s"), *(GUDCAFF.GUDBankNames(GBNIndex)));
		}
		GUDSCookDebugf(TEXT("\tFaceFXAsset:"));
		GUDSCookDebugf(TEXT("\t\t%s"), *(GUDCAFF.FaceFXAssetName));
		GUDSCookDebugf(TEXT("\tFaceFXAnimSets:"));
		for (INT FFASIndex = 0; FFASIndex < GUDCAFF.FaceFXAnimSets.Num(); FFASIndex++)
		{
			GUDSCookDebugf(TEXT("\t\t%s"), *(GUDCAFF.FaceFXAnimSets(FFASIndex)));
		}
#endif	//#if DEBUG_GUDSCOOK_MESSAGES

		// Generate the GUDs bank and associated FaceFX anim sets...
		FString Temp;
		if (GenerateGUDSPackage(Commandlet, PawnName, GUDCAFF, Temp, GearsPCD) == FALSE)
		{
			warnf(TEXT("Failed to generate GUDS packages for %s"), *(GUDCAFF.GUDBankNames(0)));
		}
	}

	// Touch the end file to track GUDS generation...
	TouchGenerationTrackingFile(FALSE);

	return TRUE;
}

/**
 *	Generate the information that is stored in the persistent cooker data.
 *	This is done to properly cook stuff out, even when not processing GUDS. 
 *
 *	@param	Commandlet		The cookpackages commandlet being run
 *	@param	Platform		The platform being cooked for
 *	@param	ShaderPlatform	The shader platform being cooked for
 *	@param	GearsPCD		The cooker persistent data to fill in...
 *	
 *	@return	UBOOL			TRUE if successful.
 */
UBOOL FGearGameGUDSCookerHelper::GeneratePersistentCookerData(
	class UCookPackagesCommandlet* Commandlet, 
	UGearsPersistentCookerData* GearsPCD)
{
	check(GearsPCD);

	// We need to clear out any existing data in the PCD...
	GearsPCD->GUDPersistentData.PawnNameToInfoRequired.Empty();
	GearsPCD->GUDPersistentData.ReferencedSoundCues.Empty();
	GearsPCD->GUDPersistentData.ReferencedFaceFXAnimations.Empty();
	GearsPCD->GUDPersistentData.SourceGUDBanks.Empty();
	GearsPCD->GUDPersistentData.StandaloneSeekfreeFilenamePairs.Empty();

	// Create the Table of contents
	if (GearsPCD->CreateTableOfContents() == FALSE)
	{
		warnf(TEXT("GUDSCook: GeneratePersistentData: Failed to create table of contents..."));
		return FALSE;
	}
	GUDToC = GearsPCD->GUDPersistentData.TableOfContents;
	GUDToC->SetFlags(RF_Standalone|RF_MarkedByCooker);
	GUDToC->AddToRoot();

	// Fill in the PawnNameToInfoRequired map...
	// This is required even if not cooking GUDS as the sound cues need to be cooked out.
	if (FillPawnNameToInfoRequiredMap(Commandlet, GearsPCD, &(GearsPCD->GUDPersistentData.PawnNameToInfoRequired)) == FALSE)
	{
		warnf(TEXT("GUDSCook: GeneratePersistentData: Failed to fill pawn name map..."));
		return FALSE;
	}

	ProcessedGUDBanks.Empty();
	UBOOL bResult = TRUE;
	for (TMap<FString, FGUDCueAndFaceFX>::TIterator PawnInfoIt(GearsPCD->GUDPersistentData.PawnNameToInfoRequired); PawnInfoIt; ++PawnInfoIt)
	{
		FString& PawnName = PawnInfoIt.Key();
		FGUDCueAndFaceFX& GUDCAFF = PawnInfoIt.Value();

#if DEBUG_GUDSCOOK_MESSAGES
		GUDSCookDebugf(TEXT("Pawn %s"), *PawnName);
		GUDSCookDebugf(TEXT("\tGUD Banks:"));
		for (INT GBNIndex = 0; GBNIndex < GUDCAFF.GUDBankNames.Num(); GBNIndex++)
		{
			GUDSCookDebugf(TEXT("\t\t%s"), *(GUDCAFF.GUDBankNames(GBNIndex)));
		}
		GUDSCookDebugf(TEXT("\tFaceFXAsset:"));
		GUDSCookDebugf(TEXT("\t\t%s"), *(GUDCAFF.FaceFXAssetName));
		GUDSCookDebugf(TEXT("\tFaceFXAnimSets:"));
		for (INT FFASIndex = 0; FFASIndex < GUDCAFF.FaceFXAnimSets.Num(); FFASIndex++)
		{
			GUDSCookDebugf(TEXT("\t\t%s"), *(GUDCAFF.FaceFXAnimSets(FFASIndex)));
		}
#endif	//#if DEBUG_GUDSCOOK_MESSAGES

		// Generate the GUDs bank and associated FaceFX anim sets...
		if (GenerateGUDSPackageInfo(Commandlet, PawnName, GUDCAFF, GearsPCD) == FALSE)
		{
			warnf(TEXT("Failed to generate GUDS packages for %s"), *(GUDCAFF.GUDBankNames(0)));
			bResult = FALSE;
		}
	}

	return bResult;
}

/**
 *	Fill in the PawnNameToInfoRequired map.
 *
 *	@param	Commandlet							The cookpackages commandlet being run
 *	@param	GearsPCD							The persistent cooker data
 *	@param	InOutPawnNameToInfoRequired			Where to fill in the info
 *	
 *	@return	UBOOL		TRUE if successful.
 */
UBOOL FGearGameGUDSCookerHelper::FillPawnNameToInfoRequiredMap(
	class UCookPackagesCommandlet* Commandlet, 
	UGearsPersistentCookerData* GearsPCD,
	TMap<FString, FGUDCueAndFaceFX>* InOutPawnNameToInfoRequired
	)
{
	// Check each pawn
	const TCHAR* PawnIniSection = TEXT("Cooker.GearPawns");
	TMultiMap<FString,FString>* IniPawnsAndRemotesList = GConfig->GetSectionPrivate(PawnIniSection, FALSE, TRUE, GEditorIni);
	if (IniPawnsAndRemotesList)
	{
		// Split up the remotes and the pawns...
		TArray<FString> RemotesList;
		TArray<FString> PawnsList;

		for (TMultiMap<FString,FString>::TIterator It(*IniPawnsAndRemotesList); It; ++It)
		{
			FString TypeString = It.Key();
			FString ValueString = It.Value();

			if (TypeString == TEXT("RemoteList"))
			{
				RemotesList.AddUniqueItem(ValueString);
			}
			else
			{
				PawnsList.AddUniqueItem(ValueString);
			}
		}

		// Now, handle each type (GearPawn vs RemoteSpeaker) grabbing their MasterGUDClassName(s)
		for (INT RemoteIndex = 0; RemoteIndex < RemotesList.Num(); RemoteIndex++)
		{
			GUDSCookDebugf(TEXT("Loading remote speaker for GUD check: %s"), *(RemotesList(RemoteIndex)));
			FGUDCueAndFaceFX* CheckGUD = InOutPawnNameToInfoRequired->Find(RemotesList(RemoteIndex));
			if (CheckGUD != NULL)
			{
				// We shouldn't find an existing one!!!
				GUDSCookDebugf(TEXT("*** ALREADY EXISTS????????"));
			}
			else
			{
				// Load the class...
				UClass* RemoteClass = LoadObject<UClass>(NULL, *(RemotesList(RemoteIndex)), NULL, LOAD_None, NULL);
				if (RemoteClass)
				{
					AGearRemoteSpeaker* RemoteSpeaker = Cast<AGearRemoteSpeaker>(RemoteClass->GetDefaultObject());
					if (RemoteSpeaker)
					{
						FGUDCueAndFaceFX NewGUDInfo;

						for (INT GUDIndex = 0; GUDIndex < RemoteSpeaker->MasterGUDBankClassNames.Num(); GUDIndex++)
						{
							NewGUDInfo.GUDBankNames.AddUniqueItem(RemoteSpeaker->MasterGUDBankClassNames(GUDIndex));
							GearsPCD->GUDPersistentData.SourceGUDBanks.Set(RemoteSpeaker->MasterGUDBankClassNames(GUDIndex), 1);
						}
						NewGUDInfo.FaceFXAssetName = TEXT("");
						InOutPawnNameToInfoRequired->Set(RemotesList(RemoteIndex), NewGUDInfo);
					}
				}
			}
		}

		// Now, handle each type (GearPawn vs PawnSpeaker) grabbing their MasterGUDClassName(s)
		for (INT PawnIndex = 0; PawnIndex < PawnsList.Num(); PawnIndex++)
		{
			GUDSCookDebugf(TEXT("Loading pawn           for GUD check: %s"), *(PawnsList(PawnIndex)));
			FGUDCueAndFaceFX* CheckGUD = InOutPawnNameToInfoRequired->Find(PawnsList(PawnIndex));
			if (CheckGUD != NULL)
			{
				// We shouldn't find an existing one!!!
				GUDSCookDebugf(TEXT("*** ALREADY EXISTS????????"));
			}
			else
			{
				// Load the class...
				UClass* PawnClass = LoadObject<UClass>(NULL, *(PawnsList(PawnIndex)), NULL, LOAD_None, NULL);
				if (PawnClass)
				{
					AGearPawn* Pawn = Cast<AGearPawn>(PawnClass->GetDefaultObject());
					if (Pawn)
					{
						FGUDCueAndFaceFX NewGUDInfo;

						for (INT GUDIndex = 0; GUDIndex < Pawn->MasterGUDBankClassNames.Num(); GUDIndex++)
						{
							NewGUDInfo.GUDBankNames.AddUniqueItem(Pawn->MasterGUDBankClassNames(GUDIndex));
							GearsPCD->GUDPersistentData.SourceGUDBanks.Set(Pawn->MasterGUDBankClassNames(GUDIndex), 1);
						}
						NewGUDInfo.FaceFXAssetName = TEXT("");
						if (Pawn->Mesh && Pawn->Mesh->SkeletalMesh && Pawn->Mesh->SkeletalMesh->FaceFXAsset)
						{
							NewGUDInfo.FaceFXAssetName = Pawn->Mesh->SkeletalMesh->FaceFXAsset->GetPathName();
						}

						// loop over all of the Animsets which guds can grab out lines from
						for( INT ChatterNameIdx = 0; ChatterNameIdx <  Pawn->FAS_ChatterNames.Num(); ++ChatterNameIdx )
						{
							if (Pawn->FAS_ChatterNames(ChatterNameIdx) != TEXT(""))
							{
								NewGUDInfo.FaceFXAnimSets.AddUniqueItem(Pawn->FAS_ChatterNames(ChatterNameIdx));
							}
						}

						InOutPawnNameToInfoRequired->Set(PawnsList(PawnIndex), NewGUDInfo);
					}
				}
			}
		}
	}
	return TRUE;
}

/** 
 *	Generate the package information for the GUDS.
 *
 *	@param	Commandlet							The cookpackages commandlet being run
 *	@param	InGUDInfo							The GUD info to generate the packages from
 *	@param	GearsPCD							The persistent cooker data
 *	
 *	@return	UBOOL		TRUE if successful.
 */
UBOOL FGearGameGUDSCookerHelper::GenerateGUDSPackageInfo( 
	class UCookPackagesCommandlet* Commandlet, 
	FString& PawnName, 
	FGUDCueAndFaceFX& InGUDInfo,
	UGearsPersistentCookerData* GearsPCD)
{
	UBOOL bResult = TRUE;

	TMap<FString, TMap<FString, INT>>& RefdSoundCues = GearsPCD->GUDPersistentData.ReferencedSoundCues;

	for (INT BankIndex = 0; BankIndex < InGUDInfo.GUDBankNames.Num(); BankIndex++)
	{
		FString GUDBankName = InGUDInfo.GUDBankNames(BankIndex);
		if (ProcessedGUDBanks2.Find(GUDBankName) != NULL)
		{
			// Already processed this GUD bank...
			GUDSCookDebugf(TEXT("GUDSCooker: Already processed bank %s"), *GUDBankName);
			GUDSCookDebugf(TEXT("\tFaceFx: %s"), *(InGUDInfo.FaceFXAssetName));
			continue;
		}

		UClass* GUDSClass = LoadObject<UClass>(NULL, *GUDBankName, NULL, LOAD_None, NULL);
		if (GUDSClass)
		{
			// Loaded object!!!!
			if (bLogGUDSGeneration)
			{
				debugf(TEXT("Loaded GUDS class data: %s"), *(GUDSClass->GetName()));
			}

			UGUDBank* DefaultBank = Cast<UGUDBank>(GUDSClass->GetDefaultObject());
			if (DefaultBank)
			{
				FString GUDPackageName;
				FString GUDFaceFXName;

				GenerateGUDNames(GUDSClass, GUDPackageName, GUDFaceFXName);

				if (bLogGUDSGeneration)
				{
					debugf(TEXT("\t%32s: GUDPackageName: %32s, GUDFaceFXName : %32s"), *(GUDSClass->GetName()), *GUDPackageName, *GUDFaceFXName);
				}

				// Load the facefx asset, if there is one...
				UFaceFXAsset* FaceFXAsset = NULL;
				TArray<UFaceFXAnimSet*> FaceFXAnimSets;
				UBOOL bProcessFaceFX = FALSE;
				if (InGUDInfo.FaceFXAssetName != TEXT(""))
				{
					FaceFXAsset = LoadObject<UFaceFXAsset>(NULL, *(InGUDInfo.FaceFXAssetName), NULL, LOAD_None, NULL);
					if (FaceFXAsset != NULL)
					{
						bProcessFaceFX = TRUE;
					}
					else
					{
						warnf(TEXT("Failed to load FaceFX Asset %s"), *(InGUDInfo.FaceFXAssetName));
					}
				}

				INT AnimSetIndex;
				for (AnimSetIndex = 0; AnimSetIndex < InGUDInfo.FaceFXAnimSets.Num(); AnimSetIndex++)
				{
					UFaceFXAnimSet* AnimSet = LoadObject<UFaceFXAnimSet>(NULL, *(InGUDInfo.FaceFXAnimSets(AnimSetIndex)), NULL, LOAD_None, NULL);
					if (AnimSet != NULL)
					{
						FaceFXAnimSets.AddItem(AnimSet);
						bProcessFaceFX = TRUE;
					}
					else
					{
						warnf(TEXT("Failed to load FaceFX AnimSet %s"), *(InGUDInfo.FaceFXAnimSets(AnimSetIndex)));
					}
				}

				UBOOL bDoneProcessingBank = FALSE;
				INT GenerationPass = 0;

				// Count the total number of non-root sounds
				INT TotalNonRootCount = 0;
				for (INT GLineIndex = 0; GLineIndex < DefaultBank->GUDLines.Num(); GLineIndex++)
				{
					FGUDLine& GUDLine = DefaultBank->GUDLines(GLineIndex);
					if ((GUDLine.Audio != NULL) && (GUDLine.bAlwaysLoad == FALSE))
					{
						TotalNonRootCount++;
					}
				}

				// Determine the number of variety banks to create
				INT VarietyBankCount = 0;
				if (VarietyLineCount > 0)
				{
					VarietyBankCount = TotalNonRootCount / VarietyLineCount;
					// Make sure we cover all sounds...
					if ((TotalNonRootCount % VarietyLineCount) > 0)
					{
						VarietyBankCount += 1;
					}
				}

				// Determine the spread across banks that is most even...
				TArray<INT> VarietyCounts;
				INT TempLineCount = TotalNonRootCount;
				INT TempBankCount = VarietyBankCount;
				for (INT CalcIndex = 0; CalcIndex < VarietyBankCount; CalcIndex++)
				{
					INT LocalLineCount = appRound((FLOAT)TempLineCount / (FLOAT)TempBankCount);
					VarietyCounts.AddItem(LocalLineCount);
					TempLineCount -= LocalLineCount;
					TempBankCount--;
				}
				if (bLogGUDSGeneration)
				{
					debugf(TEXT("GUDCooker: TotalNonRootCount = %7d (VarietyLineCount = %d) for %2d banks for GUD %s"),
						TotalNonRootCount, VarietyLineCount, VarietyBankCount, *GUDBankName);
					for (INT TempIndex = 0; TempIndex < VarietyBankCount; TempIndex++)
					{
						debugf(TEXT("\tVariety Bank %d has %2d lines"), TempIndex + 1, VarietyCounts(TempIndex));
					}
				}

				INT VarietyActionIndex = 0;

				while (!bDoneProcessingBank)
				{
					UBOOL bSkipAddingPackageToStandaloneList = FALSE;
					// Generate the package name.
					FString LocalPkgName = GUDPackageName;
					if (GenerationPass != 0)
					{
						LocalPkgName += FString::Printf(TEXT("%d"), GenerationPass);
					}

					if (bLogGUDSGeneration)
					{
						debugf(TEXT("\t%s"), *LocalPkgName);
					}

					// Generate the filenames for this package.
					FString GUDFilename = GUDSOutDir + LocalPkgName + TEXT(".upk");
					FFilename GUDSrcFilename = FFilename(GUDFilename);
					FFilename GUDDstFilename = Commandlet->GetCookedPackageFilename(GUDSrcFilename);

					FGUDSGeneratedPackageInfo* GenPkgInfo = GUDFileNameToGUDSGenerationInfoMap.Find(GUDSrcFilename);
					if (GenPkgInfo == NULL)
					{
						FGUDSGeneratedPackageInfo TempPkgInfo;
						GUDFileNameToGUDSGenerationInfoMap.Set(GUDSrcFilename, TempPkgInfo);
						GenPkgInfo = GUDFileNameToGUDSGenerationInfoMap.Find(GUDSrcFilename);
					}
					else
					{
						checkf(0, TEXT("GUDSCooker: Generated package already in map?"));
					}

					UGUDBank* CreatedGUDBank = DefaultBank;

					GenPkgInfo->PackageName = LocalPkgName;
					GenPkgInfo->SourceGUDBank = GUDBankName;
					GenPkgInfo->SourcePawn = PawnName;
					GenPkgInfo->FaceFXAssetName = FaceFXAsset ? FaceFXAsset->GetPathName() : TEXT("");
					for (INT FFASIndex = 0; FFASIndex < FaceFXAnimSets.Num(); FFASIndex++)
					{
						GenPkgInfo->FaceFXAnimSetNames.AddItem(FaceFXAnimSets(FFASIndex) ? FaceFXAnimSets(FFASIndex)->GetPathName() : TEXT(""));
					}

					if (GenerationPass == 0)
					{
						GenPkgInfo->bIsRootBank = TRUE;
						GenPkgInfo->RootLineCount = DefaultBank->GUDLines.Num();
					}
					else
					{
						GenPkgInfo->bIsRootBank = FALSE;
					}
#if WITH_FACEFX
					OC3Ent::Face::FxAnimSet* FFXAnimSet = NULL;
					OC3Ent::Face::FxAnimGroup* DestFFXAnimGroup = NULL;
#endif // WITH_FACEFX
					UBOOL bHadFaceFX = FALSE;

					INT MemorySize = 0;
					UBOOL bMemoryMaxed = FALSE;
					INT RunningTotalLineCount = 0;

					if (GenerationPass == 0)
					{
						// For the first pass (root bank), we simply pull in all sounds marked bAlwaysLoad...
						for (INT ActionIndex = 0; (ActionIndex < DefaultBank->GUDActions.Num()) && !bMemoryMaxed; ActionIndex++)
						{
							const FGUDAction& CheckAction = DefaultBank->GUDActions(ActionIndex);

							// Process the 'standard' lines
							for (INT LIndex = 0; LIndex < CheckAction.LineIndices.Num(); LIndex++)
							{
								INT LineIndex = CheckAction.LineIndices(LIndex);
								FGUDLine& Line = DefaultBank->GUDLines(LineIndex);

								// Only handle lines that have not already been processed.
								if (!Line.bCookerProcessed)
								{
									if ((Line.bAlwaysLoad == TRUE) || (VarietyBankCount == 0))
									{
										if (Line.Audio)
										{
											MemorySize += Line.Audio->GetResourceSize(Commandlet->Platform);
										}
										// Add it...
										GenPkgInfo->LinesIncluded.AddItem(LineIndex);

										FGUDSActionLineInfo* ActionLineInfo = GenPkgInfo->ActionMap.Find(ActionIndex);
										if (ActionLineInfo == NULL)
										{
											FGUDSActionLineInfo TempActionLineInfo;
											GenPkgInfo->ActionMap.Set(ActionIndex, TempActionLineInfo);
											ActionLineInfo = GenPkgInfo->ActionMap.Find(ActionIndex);
											check(ActionLineInfo);
										}
										ActionLineInfo->LineIndices.AddItem(LineIndex);
										// Mark the source line as processed...
										Line.bCookerProcessed = TRUE;

										if (bLogGUDSGeneration)
										{
											debugf(TEXT("\t\t%32s"), *(Line.Audio->GetPathName()));
										}
									}
								}
							}
							
							// Process the combat lines
							for (INT CLIndex = 0; CLIndex < CheckAction.CombatOnlyLineIndices.Num(); CLIndex++)
							{
								INT LineIndex = CheckAction.CombatOnlyLineIndices(CLIndex);
								FGUDLine& Line = DefaultBank->GUDLines(LineIndex);

								// Only handle lines that have not already been processed.
								if (!Line.bCookerProcessed)
								{
									if ((Line.bAlwaysLoad == TRUE) || (VarietyBankCount == 0))
									{
										if (Line.Audio)
										{
											MemorySize += Line.Audio->GetResourceSize(Commandlet->Platform);
										}
										// Add it...
										GenPkgInfo->LinesIncluded.AddItem(LineIndex);

										FGUDSActionLineInfo* ActionLineInfo = GenPkgInfo->ActionMap.Find(ActionIndex);
										if (ActionLineInfo == NULL)
										{
											FGUDSActionLineInfo TempActionLineInfo;
											GenPkgInfo->ActionMap.Set(ActionIndex, TempActionLineInfo);
											ActionLineInfo = GenPkgInfo->ActionMap.Find(ActionIndex);
											check(ActionLineInfo);
										}
										ActionLineInfo->CombatLineIndices.AddItem(LineIndex);
										// Mark the source line as processed...
										Line.bCookerProcessed = TRUE;

										if (bLogGUDSGeneration)
										{
											debugf(TEXT("\t\t%32s"), *(Line.Audio->GetPathName()));
										}
									}
								}
							}

							// Process the noncombat lines
							for (INT NCLIndex = 0; NCLIndex < CheckAction.NonCombatOnlyLineIndices.Num(); NCLIndex++)
							{
								INT LineIndex = CheckAction.NonCombatOnlyLineIndices(NCLIndex);
								FGUDLine& Line = DefaultBank->GUDLines(LineIndex);

								// Only handle lines that have not already been processed.
								if (!Line.bCookerProcessed)
								{
									if ((Line.bAlwaysLoad == TRUE) || (VarietyBankCount == 0))
									{
										if (Line.Audio)
										{
											MemorySize += Line.Audio->GetResourceSize(Commandlet->Platform);
										}
										// Add it...
										GenPkgInfo->LinesIncluded.AddItem(LineIndex);

										FGUDSActionLineInfo* ActionLineInfo = GenPkgInfo->ActionMap.Find(ActionIndex);
										if (ActionLineInfo == NULL)
										{
											FGUDSActionLineInfo TempActionLineInfo;
											GenPkgInfo->ActionMap.Set(ActionIndex, TempActionLineInfo);
											ActionLineInfo = GenPkgInfo->ActionMap.Find(ActionIndex);
											check(ActionLineInfo);
										}
										ActionLineInfo->NonCombatLineIndices.AddItem(LineIndex);
										// Mark the source line as processed...
										Line.bCookerProcessed = TRUE;

										if (bLogGUDSGeneration)
										{
											debugf(TEXT("\t\t%32s"), *(Line.Audio->GetPathName()));
										}
									}
								}
							}
						}
					}
					else
					{
						// When doing the variety banks, start at the last action index...
						INT PassVarietyCount = VarietyCounts(GenerationPass - 1);
						INT CurrentVarietyLineCount = 0;
						UBOOL bOutOfLines = FALSE;
						while ((CurrentVarietyLineCount < PassVarietyCount) && !bOutOfLines)
						{
							const FGUDAction& CheckAction = DefaultBank->GUDActions(VarietyActionIndex);

							const INT CombatLineOffset = 10000;
							const INT NonCombatLineOffset = 20000;
							TArray<INT> AvailableLines;
							// Gather all available standard lines
							for (INT LIndex = 0; LIndex < CheckAction.LineIndices.Num(); LIndex++)
							{
								INT LineIndex = CheckAction.LineIndices(LIndex);
								FGUDLine& Line = DefaultBank->GUDLines(LineIndex);

								// Only handle lines that have not already been processed.
								if (!Line.bCookerProcessed)
								{
									if (Line.bAlwaysLoad == FALSE)
									{
										AvailableLines.AddUniqueItem(LineIndex);
									}
								}
							}
							// Gather all available combat lines
							for (INT CLIndex = 0; CLIndex < CheckAction.CombatOnlyLineIndices.Num(); CLIndex++)
							{
								INT LineIndex = CheckAction.CombatOnlyLineIndices(CLIndex);
								FGUDLine& Line = DefaultBank->GUDLines(LineIndex);

								// Only handle lines that have not already been processed.
								if (!Line.bCookerProcessed)
								{
									if (Line.bAlwaysLoad == FALSE)
									{
										AvailableLines.AddUniqueItem(LineIndex + CombatLineOffset);
									}
								}
							}

							// Gather all available noncombat lines
							for (INT NCLIndex = 0; NCLIndex < CheckAction.NonCombatOnlyLineIndices.Num(); NCLIndex++)
							{
								INT LineIndex = CheckAction.NonCombatOnlyLineIndices(NCLIndex);
								FGUDLine& Line = DefaultBank->GUDLines(LineIndex);

								// Only handle lines that have not already been processed.
								if (!Line.bCookerProcessed)
								{
									if (Line.bAlwaysLoad == FALSE)
									{
										AvailableLines.AddUniqueItem(LineIndex + NonCombatLineOffset);
									}
								}
							}

							if (AvailableLines.Num() > 0)
							{
								// There are still lines in this action...
								INT RandIndex = MyRandom.GetFraction() * AvailableLines.Num();
								INT PickedIndex = AvailableLines(RandIndex);
								UBOOL bIsCombatLine = FALSE;
								UBOOL bIsNonCombatLine = FALSE;

								if (PickedIndex >= NonCombatLineOffset)
								{
									// NonCombat line
									PickedIndex -= NonCombatLineOffset;
									bIsNonCombatLine = TRUE;
								}
								else if (PickedIndex >= CombatLineOffset)
								{
									// Combat line
									PickedIndex -= CombatLineOffset;
									bIsCombatLine = TRUE;
								}

								FGUDLine& Line = DefaultBank->GUDLines(PickedIndex);
								if (Line.Audio)
								{
									MemorySize += Line.Audio->GetResourceSize(Commandlet->Platform);
								}

								GenPkgInfo->LinesIncluded.AddItem(PickedIndex);

								FGUDSActionLineInfo* ActionLineInfo = GenPkgInfo->ActionMap.Find(VarietyActionIndex);
								if (ActionLineInfo == NULL)
								{
									FGUDSActionLineInfo TempActionLineInfo;
									GenPkgInfo->ActionMap.Set(VarietyActionIndex, TempActionLineInfo);
									ActionLineInfo = GenPkgInfo->ActionMap.Find(VarietyActionIndex);
									check(ActionLineInfo);
								}
								if (bIsNonCombatLine)
								{
									ActionLineInfo->NonCombatLineIndices.AddItem(PickedIndex);
								}
								else if (bIsCombatLine)
								{
									ActionLineInfo->CombatLineIndices.AddItem(PickedIndex);
								}
								else
								{
									ActionLineInfo->LineIndices.AddItem(PickedIndex);
								}
								// Mark the source line as processed...
								Line.bCookerProcessed = TRUE;

								if (bLogGUDSGeneration)
								{
									debugf(TEXT("\t\t%32s"), *(Line.Audio->GetPathName()));
								}
								// Remove the line from the available array
								AvailableLines.Remove(RandIndex);

								CurrentVarietyLineCount++;
								RunningTotalLineCount++;
							}

							if (RunningTotalLineCount >= TotalNonRootCount)
							{
								bOutOfLines = TRUE;
							}

							// Move to the next action, wrapping back to 0 if at the end
							VarietyActionIndex++;
							if (VarietyActionIndex >= DefaultBank->GUDActions.Num())
							{
								VarietyActionIndex = 0;
							}
						}
					}
					GUDSCookDebugf(TEXT("GUDCooker: %6d bytes for package %s"), MemorySize, *LocalPkgName);

					// Add it to the ToC
					if (GUDToC)
					{
						FGUDCollection* ToCCollection = GUDToC->TableOfContents.Find(GUDBankName);
						if (GenerationPass != 0)
						{
							check(ToCCollection);
						}
						if (ToCCollection == NULL)
						{
							FGUDCollection NewCollection;
							appMemzero(&NewCollection, sizeof(FGUDCollection));
							GUDToC->TableOfContents.Set(GUDBankName, NewCollection);
							ToCCollection = GUDToC->TableOfContents.Find(GUDBankName);
						}
						check(ToCCollection);

						if (GenerationPass == 0)
						{
							ToCCollection->RootGUDBank.BankName = LocalPkgName;
							ToCCollection->RootGUDBank.ApproxBankSize = MemorySize;
						}
						else
						{
							if (bGenerateRootBankOnly == FALSE)
							{
								FGUDToCEntry TmpEntry;
								TmpEntry.ApproxBankSize = MemorySize;
								TmpEntry.BankName = LocalPkgName;
								// Don't put the Banks in the TOC if generating root only!
								ToCCollection->GUDBanks.AddUniqueItem(TmpEntry);
							}
							else
							{
								bSkipAddingPackageToStandaloneList = TRUE;
							}
						}
					}

					// Now process any facefx...
					for (INT LineIndex = 0; LineIndex < GenPkgInfo->LinesIncluded.Num(); LineIndex++)
					{
						UBOOL bFound = FALSE;
						FGUDLine& GUDLine = DefaultBank->GUDLines(GenPkgInfo->LinesIncluded(LineIndex));
						USoundCue* SoundCue = GUDLine.Audio;
						if (SoundCue)
						{
							// Add it to the ref'd cue list...
							// Add it to the ref'd cue list...
							TMap<FString, INT>* SoundMap = RefdSoundCues.Find(SoundCue->GetPathName());
							if (SoundMap == NULL)
							{
								TMap<FString, INT> TempSoundMap;
								RefdSoundCues.Set(SoundCue->GetPathName(), TempSoundMap);
								SoundMap = RefdSoundCues.Find(SoundCue->GetPathName());
							}
							check(SoundMap);

							INT* PkgIsInMap = SoundMap->Find(LocalPkgName);
							if (PkgIsInMap == NULL)
							{
								GUDSCookDebugf(TEXT("STUCK IN Package %s: %s"), *LocalPkgName, *(SoundCue->GetPathName()));
								SoundMap->Set(LocalPkgName, 1);
							}
							FString LocalPkgNameSF = LocalPkgName + TEXT("_SF");
							PkgIsInMap = SoundMap->Find(LocalPkgNameSF);
							if (PkgIsInMap == NULL)
							{
								SoundMap->Set(LocalPkgNameSF, 1);
							}


#if WITH_FACEFX
							// Handle FaceFX...
							if ((SoundCue->FaceFXAnimName != TEXT("")) && (bProcessFaceFX == TRUE))
							{
								bHadFaceFX = TRUE;
								// Load the animgroup
								// Give the name a numeric ending that matches the bank #
								//FString NewAGName = SoundCue->FaceFXGroupName + TEXT("0");
								FString NewAGName = LocalPkgName;
								FxName AGNewName(TCHAR_TO_ANSI(*(NewAGName)));
								FxName AGName(TCHAR_TO_ANSI(*(SoundCue->FaceFXGroupName)));

								FxName AnimName(TCHAR_TO_ANSI(*(SoundCue->FaceFXAnimName)));
								FxAnimGroup* FFXAnimGroup = NULL;
								FxSize AnimIndex = FxInvalidIndex;

								// Check the FaceFXAsset
								if (FaceFXAsset)
								{
									OC3Ent::Face::FxActor* FaceFXActor = FaceFXAsset->GetFxActor();
									if (FaceFXActor)
									{
										FxSize AGIndex = FaceFXActor->FindAnimGroup(AGName);
										if (AGIndex != FxInvalidIndex)
										{
											FxAnimGroup& TempFFXAnimGroup = FaceFXActor->GetAnimGroup(AGIndex);
											FFXAnimGroup = &TempFFXAnimGroup;

											AnimIndex = FFXAnimGroup->FindAnim(AnimName);
											if (AnimIndex != FxInvalidIndex)
											{
												bFound = TRUE;
											}
										}
									}
								}

								UFaceFXAnimSet* FaceFXAnimSet = NULL;
								// If it wasn't found in the asset, check in any available animsets...
								if ((bFound == FALSE) && (FaceFXAnimSets.Num() > 0))
								{
									// Clear this so the lookup map gets the correct key!
									FaceFXAsset = NULL;
									for (AnimSetIndex = 0; (AnimSetIndex < FaceFXAnimSets.Num()) && !bFound; AnimSetIndex++)
									{
										FaceFXAnimSet = FaceFXAnimSets(AnimSetIndex);
										if (FaceFXAnimSet)
										{
											FxAnimSet* FFXAnimSet = FaceFXAnimSet->GetFxAnimSet();
											if (FFXAnimSet)
											{
												const FxAnimGroup& TempFFXAnimGroup = FFXAnimSet->GetAnimGroup();
												FFXAnimGroup = (FxAnimGroup*)&TempFFXAnimGroup;

												AnimIndex = FFXAnimGroup->FindAnim(AnimName);
												if (AnimIndex != FxInvalidIndex)
												{
													bFound = TRUE;
												}
											}
										}
									}
								}

								if (bFound)
								{
									UObject* FaceFXObject = (FaceFXAsset != NULL) ? (UObject*)FaceFXAsset : (UObject*)FaceFXAnimSet;

									TArray<FGUDSReferencedFaceFXAnimation>* RefdAnims = GenPkgInfo->FaceFxAnimations.Find(FaceFXObject->GetName());
									if (RefdAnims == NULL)
									{
										TArray<FGUDSReferencedFaceFXAnimation> TempAnims;
										GenPkgInfo->FaceFxAnimations.Set(FaceFXObject->GetName(), TempAnims);
										RefdAnims = GenPkgInfo->FaceFxAnimations.Find(FaceFXObject->GetName());
									}
									if (RefdAnims)
									{
										FGUDSReferencedFaceFXAnimation NewAnim;
										NewAnim.AnimGroup = SoundCue->FaceFXGroupName;
										NewAnim.AnimName = SoundCue->FaceFXAnimName;
										RefdAnims->AddUniqueItem(NewAnim);
									}

									TGUDSReferencedFaceFXAnimations* PCDRefdAnims = GetReferencedAnimationsForObject(Commandlet, FaceFXObject);
									if (PCDRefdAnims)
									{
										FGUDSReferencedFaceFXAnimation NewAnim;
										NewAnim.AnimGroup = SoundCue->FaceFXGroupName;
										NewAnim.AnimName = SoundCue->FaceFXAnimName;
										PCDRefdAnims->AddUniqueItem(NewAnim);
									}
								}
							}
#endif // WITH_FACEFX
						}
					}

					// Add the package to the list for cooking.
					if (bSkipAddingPackageToStandaloneList == FALSE)
					{
						GearsPCD->GUDPersistentData.StandaloneSeekfreeFilenamePairs.AddItem(FPackageCookerInfo(*GUDSrcFilename, *GUDDstFilename, TRUE, FALSE, FALSE, TRUE));
						if (GearsPCD->GUDPersistentData.SpecialGUDSPackages.Find(GUDSrcFilename) == NULL)
						{
							GearsPCD->GUDPersistentData.SpecialGUDSPackages.Set(GUDSrcFilename, LocalPkgName);
						}
					}
					GenerationPass++;
					if (GenerationPass > VarietyBankCount)
					{
						bDoneProcessingBank = TRUE;
					}
				}

				//@todo.SAS. Re-enable this check for debugging when xls-exporter is fixed up!
#if 0
				for (INT CheckLineIndex = 0; CheckLineIndex < DefaultBank->GUDLines.Num(); CheckLineIndex++)
				{
					const FGUDLine& CheckLine = DefaultBank->GUDLines(CheckLineIndex);
					if (CheckLine.Audio)
					{
						if (!CheckLine.bCookerProcessed)
						{
							warnf(TEXT("GUDCooker: Unpack line from Bank %s, %s"),
								*GUDBankName, *(CheckLine.Audio->GetPathName()));
						}
					}
				}
#endif
				// Set this bank as processed to avoid doing it multiple times.
				ProcessedGUDBanks2.Set(InGUDInfo.GUDBankNames(BankIndex), 1);
			}
		}
		else
		{
			warnf(TEXT("Failed to load GUDBank class %s"), *GUDBankName);
			bResult = FALSE;
		}
	}

	return bResult;
}

/** 
 *	Generate the package(s) for the GUDS.
 *
 *	@param	Commandlet							The cookpackages commandlet being run
 *	@param	InGUDInfo							The GUD info to generate the packages from
 *	@param	GearsPCD							The persistent cooker data
 *	
 *	@return	UBOOL		TRUE if successful.
 */
UBOOL FGearGameGUDSCookerHelper::GenerateGUDSPackage( 
	class UCookPackagesCommandlet* Commandlet, 
	FString& PawnName, 
	FGUDCueAndFaceFX& InGUDInfo,
	FString& PackageName,
	UGearsPersistentCookerData* GearsPCD)
{
	SCOPE_SECONDS_COUNTER(GUDS_GenerateGUDSPackageTime);
	for (INT GBIndex = 0; GBIndex < InGUDInfo.GUDBankNames.Num(); GBIndex++)
	{
		FString GBName = InGUDInfo.GUDBankNames(GBIndex);
		for (TMap<FString, FGUDSGeneratedPackageInfo>::TIterator It(GUDFileNameToGUDSGenerationInfoMap); It; ++It)
		{
			FString GUDSrcFilename = It.Key();
			FGUDSGeneratedPackageInfo& GenPkgInfo = It.Value();

			if (GenPkgInfo.SourceGUDBank == GBName)
			{
				if (bGenerateRootBankOnly == TRUE)
				{
					if (GenPkgInfo.bIsRootBank == FALSE)
					{
						continue;
					}
				}
				UPackage* NewGUDPackage = GenerateGUDSPackage(Commandlet, InGUDInfo, GenPkgInfo, GearsPCD);
				if (NewGUDPackage)
				{
					FString GUDFilename = GUDSOutDir + GenPkgInfo.PackageName + TEXT(".upk");
					// Save the temp package
					{
						SCOPE_SECONDS_COUNTER(GUDS_SaveGUDSPackageTime);
						UObject::SavePackage(NewGUDPackage, NULL, RF_Standalone, *GUDFilename, GWarn);
					}
				}
			}
		}
	}

	return TRUE;
}

/** 
 *	Generate the package(s) for the GUDS.
 *
 *	@param	Commandlet							The cookpackages commandlet being run
 *	@param	InGUDInfo							The GUD info to generate the packages from
 *	@param	GearsPCD							The persistent cooker data
 *	
 *	@return	UBOOL		TRUE if successful.
 */
UPackage* FGearGameGUDSCookerHelper::GenerateGUDSPackage( 
	class UCookPackagesCommandlet* Commandlet, 
	FGUDCueAndFaceFX& InGUDInfo,
	FGUDSGeneratedPackageInfo& GenPkgInfo,
	UGearsPersistentCookerData* GearsPCD)
{
	UPackage* GUDPackage = NULL;

	FString GUDBankName = GenPkgInfo.SourceGUDBank;
	FString LocalPackageName = GenPkgInfo.PackageName;

	UClass* GUDSClass = LoadObject<UClass>(NULL, *GUDBankName, NULL, LOAD_None, NULL);
	if (GUDSClass)
	{
		// Loaded object!!!!
		GUDSCookDebugf(TEXT("Loaded GUDS class data: %s"), *(GUDSClass->GetName()));

		FString LocalGUDPackageName;
		FString LocalGUDFaceFXName;

		GenerateGUDNames(GUDSClass, LocalGUDPackageName, LocalGUDFaceFXName);
		LocalGUDFaceFXName = LocalPackageName + TEXT("_FX");

		GUDSCookDebugf(TEXT("Generate GUDPackageName: %s"), *LocalGUDPackageName);
		GUDSCookDebugf(TEXT("Generate GUDFaceFXName : %s"), *LocalGUDFaceFXName);

		UGUDBank* DefaultBank = Cast<UGUDBank>(GUDSClass->GetDefaultObject());
		if (DefaultBank)
		{
			// Now, generate an actual package from it!
			// Create the temp package here for now... 
			FString GUDFilename = GUDSOutDir + GenPkgInfo.PackageName + TEXT(".upk");

			GUDPackage = UObject::CreatePackage(NULL, *LocalPackageName);
			GUDPackage->PackageFlags |= PKG_ServerSideOnly;
			// Why do I have to 'fully load' a package I just created???
			GUDPackage->FullyLoad();
			// Create the GUD bank
			UGUDBank* CreatedGUDBank = Cast<UGUDBank>(UObject::StaticConstructObject(UGUDBank::StaticClass(), GUDPackage, FName(TEXT("GeneratedGUDBank"))));
			CreatedGUDBank->SetFlags( RF_Standalone );
			CreatedGUDBank->SourceGUDBankPath = GenPkgInfo.SourceGUDBank;

			// Put in all possible lines - but empty...
			CreatedGUDBank->GUDLines.AddZeroed(DefaultBank->GUDLines.Num());
			CreatedGUDBank->GUDActions.AddZeroed(DefaultBank->GUDActions.Num());

			UBOOL bSoundCuesHadFaceFXReferences = FALSE;
			// Now copy everything in...
			// Lines first...
			for (INT CopyIndex = 0; CopyIndex < GenPkgInfo.LinesIncluded.Num(); CopyIndex++)
			{
				INT LineIndex = GenPkgInfo.LinesIncluded(CopyIndex);
				CreatedGUDBank->GUDLines(LineIndex) = DefaultBank->GUDLines(LineIndex);
				if (CreatedGUDBank->GUDLines(LineIndex).Audio)
				{
					if (CreatedGUDBank->GUDLines(LineIndex).Audio->FaceFXGroupName.Len() > 0)
					{
						bSoundCuesHadFaceFXReferences = TRUE;
					}
				}
			}

			// Now actions...
			for (TMap<INT, FGUDSActionLineInfo>::TIterator ActionMapIter(GenPkgInfo.ActionMap); ActionMapIter; ++ActionMapIter)
			{
				INT ActionID = ActionMapIter.Key();
				FGUDSActionLineInfo& ALInfo = ActionMapIter.Value();

				FGUDAction& NewAction = CreatedGUDBank->GUDActions(ActionID);

				NewAction.LineIndices = ALInfo.LineIndices;
				NewAction.CombatOnlyLineIndices = ALInfo.CombatLineIndices;
				NewAction.NonCombatOnlyLineIndices = ALInfo.NonCombatLineIndices;
			}

#if WITH_FACEFX
			// Now FaceFX stuff...
			// Load the facefx asset, if there is one...
			INT AnimSetIndex;
			UFaceFXAsset* FaceFXAsset = NULL;
			TArray<UFaceFXAnimSet*> FaceFXAnimSets;
			UBOOL bProcessFaceFX = FALSE;
			if (bSoundCuesHadFaceFXReferences)
			{
				if (InGUDInfo.FaceFXAssetName != TEXT(""))
				{
					FaceFXAsset = LoadObject<UFaceFXAsset>(NULL, *(InGUDInfo.FaceFXAssetName), NULL, LOAD_None, NULL);
					if (FaceFXAsset != NULL)
					{
						bProcessFaceFX = TRUE;
					}
					else
					{
						warnf(TEXT("Failed to load FaceFX Asset %s"), *(InGUDInfo.FaceFXAssetName));
					}
				}

				for (AnimSetIndex = 0; AnimSetIndex < InGUDInfo.FaceFXAnimSets.Num(); AnimSetIndex++)
				{
					UFaceFXAnimSet* AnimSet = LoadObject<UFaceFXAnimSet>(NULL, *(InGUDInfo.FaceFXAnimSets(AnimSetIndex)), NULL, LOAD_None, NULL);
					if (AnimSet != NULL)
					{
						FaceFXAnimSets.AddItem(AnimSet);
						bProcessFaceFX = TRUE;
					}
					else
					{
						warnf(TEXT("Failed to load FaceFX AnimSet %s"), *(InGUDInfo.FaceFXAnimSets(AnimSetIndex)));
					}
				}
			}

			UFaceFXAnimSet* NewFaceFXAnimSet = NULL;
			if (bProcessFaceFX)
			{
				// Need to pull in all the associated FaceFX stuff
				NewFaceFXAnimSet = Cast<UFaceFXAnimSet>(UObject::StaticConstructObject(UFaceFXAnimSet::StaticClass(), GUDPackage, 
					FName(TEXT("GeneratedFaceFXAnimSet"))));
				NewFaceFXAnimSet->SetFlags( RF_Standalone );
			}

			OC3Ent::Face::FxAnimSet* FFXAnimSet = NULL;
			OC3Ent::Face::FxAnimGroup* DestFFXAnimGroup = NULL;

			UBOOL bHadFaceFX = FALSE;
			for (INT LineIndex = 0; LineIndex < CreatedGUDBank->GUDLines.Num(); LineIndex++)
			{
				UBOOL bFound = FALSE;
				FGUDLine& GUDLine = CreatedGUDBank->GUDLines(LineIndex);
				USoundCue* SoundCue = GUDLine.Audio;
				if (SoundCue)
				{
					// Handle FaceFX...
					if ((SoundCue->FaceFXAnimName != TEXT("")) && (bProcessFaceFX == TRUE))
					{
						bHadFaceFX = TRUE;
						// Load the animgroup
						// Give the name a numeric ending that matches the bank #
						//FString NewAGName = SoundCue->FaceFXGroupName + TEXT("0");
						FString NewAGName = LocalPackageName;
						FxName AGNewName(TCHAR_TO_ANSI(*(NewAGName)));
						FxName AGName(TCHAR_TO_ANSI(*(SoundCue->FaceFXGroupName)));

						// This assumes that ALL the chatter lines will be in the same anim group!
						if (DestFFXAnimGroup == NULL)
						{
							// Create the FxAnimSet...
							FxName FFXAnimSetName(TCHAR_TO_ANSI(*LocalGUDFaceFXName));
							OC3Ent::Face::FxAnimSet* FFXAnimSet = new FxAnimSet();
							check(FFXAnimSet);
							FFXAnimSet->SetName(FFXAnimSetName);

							// Create the FXAnimGroup...
							OC3Ent::Face::FxAnimGroup ContainedAnimGroup;
							ContainedAnimGroup.SetName(AGNewName);
							FFXAnimSet->SetAnimGroup(ContainedAnimGroup);

							DestFFXAnimGroup = (OC3Ent::Face::FxAnimGroup*)&(FFXAnimSet->GetAnimGroup());

							NewFaceFXAnimSet->InternalFaceFXAnimSet = FFXAnimSet;
						}

						FxName AnimName(TCHAR_TO_ANSI(*(SoundCue->FaceFXAnimName)));
						FxAnimGroup* FFXAnimGroup = NULL;
						FxSize AnimIndex = FxInvalidIndex;
						if (FaceFXAsset)
						{
							OC3Ent::Face::FxActor* FaceFXActor = FaceFXAsset->GetFxActor();
							if (FaceFXActor)
							{
								FxSize AGIndex = FaceFXActor->FindAnimGroup(AGName);
								if (AGIndex != FxInvalidIndex)
								{
									FxAnimGroup& TempFFXAnimGroup = FaceFXActor->GetAnimGroup(AGIndex);
									FFXAnimGroup = &TempFFXAnimGroup;

									AnimIndex = FFXAnimGroup->FindAnim(AnimName);
									if (AnimIndex != FxInvalidIndex)
									{
										bFound = TRUE;
									}
								}
							}
						}

						UFaceFXAnimSet* FaceFXAnimSet = NULL;
						// If it wasn't found in the asset, check in any available animsets...
						if ((bFound == FALSE) && (FaceFXAnimSets.Num() > 0))
						{
							// Clear this so the lookup map gets the correct key!
							FaceFXAsset = NULL;
							for (AnimSetIndex = 0; (AnimSetIndex < FaceFXAnimSets.Num()) && !bFound; AnimSetIndex++)
							{
								FaceFXAnimSet = FaceFXAnimSets(AnimSetIndex);
								if (FaceFXAnimSet)
								{
									FxAnimSet* FFXAnimSet = FaceFXAnimSet->GetFxAnimSet();
									if (FFXAnimSet)
									{
										const FxAnimGroup& TempFFXAnimGroup = FFXAnimSet->GetAnimGroup();
										FFXAnimGroup = (FxAnimGroup*)&TempFFXAnimGroup;

										AnimIndex = FFXAnimGroup->FindAnim(AnimName);
										if (AnimIndex != FxInvalidIndex)
										{
											bFound = TRUE;
										}
									}
								}
							}
						}

						if (bFound)
						{
							if (CopyAnimationToAnimGroup(SoundCue, AnimIndex, AnimName, FFXAnimGroup, DestFFXAnimGroup) == FALSE)
							{
								GUDSCookDebugf(TEXT("Failed to copy anim to group: %s, %s for %s"), 
									*(SoundCue->FaceFXAnimName), 
									*(SoundCue->FaceFXGroupName),
									*(SoundCue->GetPathName()));
							}
						}
					}
				}
			}

			if (bHadFaceFX == TRUE)
			{
				NewFaceFXAnimSet->ReferencedSoundCues.Empty();
				NewFaceFXAnimSet->FixupReferencedSoundCues();
			}
#endif // WITH_FACEFX
		}
	}

	return GUDPackage;
}

/**
 *	Get the various names that will be generated from the given GUD class.
 *
 *	@param	InGUDClass					The GUD class to generate names for.
 *	@param	OutGeneratedGUDName			The resulting generated GUD name.
 *	@param	OutGeneratedGUDFaceFXName	The resulting generated FaceFX name.
 *
 *	@return	UBOOL						TRUE if successful.
 */
UBOOL FGearGameGUDSCookerHelper::GenerateGUDNames(
	const UClass* InGUDClass,
	FString& OutGeneratedGUDName,
	FString& OutGeneratedGUDFaceFXName
	)
{
	// Create the package
	FString TempName = InGUDClass->GetName();
	FString TempName2;

	UBOOL bMultiplayer = FALSE;
	INT MPIndex = TempName.InStr(TEXT("MP"));
	if (MPIndex != -1)
	{
		INT CheckIndex = TempName.Len() - 2;
		if (MPIndex == CheckIndex)
		{
			bMultiplayer = TRUE;
			TempName = TempName.Left(CheckIndex);
		}
	}
	// Strip out the 'Data' part of the name...
	INT DataIndex = TempName.InStr(TEXT("_"));
	if (DataIndex != -1)
	{
		if (bMultiplayer)
		{
			TempName2 = FString(TEXT("GUDMP"));
		}
		else
		{
			TempName2 = FString(TEXT("GUD"));
		}
		TempName2 += TempName.Right(TempName.Len() - DataIndex);
		TempName = TempName2;
	}
	OutGeneratedGUDName = TempName2 + TEXT("G");
	OutGeneratedGUDFaceFXName = (*(OutGeneratedGUDName + TEXT("_FX")));

	return TRUE;
}

#if WITH_FACEFX
/** 
 *	Move the animation at the given index from the source group to the dest group.
 *
 *	@param	InSourceSoundCue	The sound cue that matches the animation.
 *	@param	InAnimIndex			The index of the animation.
 *	@param	InAnimName			The name of the animation.
 *	@param	SrcFFXAnimGroup		The AnimGroup the animation is in.
 *	@param	DestFFXAnimGroup	The AnimGroup to copy the animation to.
 *	
 *	@return	UBOOL		TRUE if successful.
 */
UBOOL FGearGameGUDSCookerHelper::CopyAnimationToAnimGroup(
	USoundCue* InSourceSoundCue, 
	OC3Ent::Face::FxSize InAnimIndex,
	OC3Ent::Face::FxName InAnimName,
	OC3Ent::Face::FxAnimGroup* SrcFFXAnimGroup,
	OC3Ent::Face::FxAnimGroup* DestFFXAnimGroup)
{
	const OC3Ent::Face::FxAnim& AnimRef = SrcFFXAnimGroup->GetAnim(InAnimIndex);
	if (DestFFXAnimGroup->AddAnim(AnimRef) == FALSE)
	{
		UBOOL bAlreadyExists = (DestFFXAnimGroup->FindAnim(InAnimName) != FxInvalidIndex) ? TRUE : FALSE;
		GUDSCookDebugf(TEXT("Failed to add anim %s to group %s - %s"),
			*(InSourceSoundCue->FaceFXAnimName),
			*(InSourceSoundCue->FaceFXGroupName),
			bAlreadyExists ? TEXT("ALREADY IN GROUP!") : TEXT("????"));
		if (bAlreadyExists == FALSE)
		{
			return FALSE;
		}
	}

	return TRUE;
}

#endif // WITH_FACEFX

/**
 *	Retrieve the referenced animation list for the given object.
 */
TGUDSReferencedFaceFXAnimations* FGearGameGUDSCookerHelper::GetReferencedAnimationsForObject(
	class UCookPackagesCommandlet* Commandlet, UObject* InFaceFXObject, UBOOL bCreateIfNotFound)
{
	if (InFaceFXObject == NULL)
	{
		return NULL;
	}

	// Grab the persistent data info
	UGearsPersistentCookerData* GearsPCD = Cast<UGearsPersistentCookerData>(Commandlet->PersistentCookerData);
	check(GearsPCD);

	TMap<FString, TArray<FGUDSReferencedFaceFXAnimation>>& RefdFaceFXAnims = GearsPCD->GUDPersistentData.ReferencedFaceFXAnimations;
	TGUDSReferencedFaceFXAnimations* RefdAnims = RefdFaceFXAnims.Find(InFaceFXObject->GetPathName());
	if (RefdAnims == NULL)
	{
		if (bCreateIfNotFound == TRUE)
		{
			TGUDSReferencedFaceFXAnimations EmptyList;
			RefdFaceFXAnims.Set(InFaceFXObject->GetPathName(), EmptyList);
			RefdAnims = RefdFaceFXAnims.Find(InFaceFXObject->GetPathName());
			GUDSCookDebugf(TEXT("GetRefdAnimations: Did not exist... creating - %s"), *(InFaceFXObject->GetPathName()));
		}
		else
		{
			GUDSCookDebugf(TEXT("GetRefdAnimations: Did not exist... no gen   - %s"), *(InFaceFXObject->GetPathName()));
		}
	}

	return RefdAnims;
}

/**
 *	Checks to see if GUDS need to be generated during the cook
 *
 *	@param	Commandlet		The cookpackages commandlet being run
 *	@param	GearsPCD		The UGearsPersistentCookerData instance
 *	@return	UBOOL			TRUE if they should, FALSE if not
 */
UBOOL FGearGameGUDSCookerHelper::ShouldBeGenerated(
	class UCookPackagesCommandlet* Commandlet, 
	UGearsPersistentCookerData* GearsPCD)
{
	// Check the PCD for out-of-dateness
	if (GearsPCD->GUDPersistentData.ShouldBeGenerated(Commandlet) == TRUE)
	{
		return TRUE;
	}

	// Check the Start and End file times...
	DOUBLE StartFileTime = GetGenerationTrackingFileTimestamp(TRUE);
	DOUBLE EndFileTime = GetGenerationTrackingFileTimestamp(FALSE);
	if ((EndFileTime  < StartFileTime) || (EndFileTime == -1.0) || (StartFileTime == -1.0))
	{
		return TRUE;
	}

	//@todo.SAS. Check for out-of-date sources vs. the GUDData_*.uc file that they were generated from

	// Made it this far, so we must not need to regenerate!
	return FALSE;
}

/**
 *	Checks to see if GUDS need to be cooked
 *
 *	@param	Commandlet		The cookpackages commandlet being run
 *	@param	bForceRecook	If TRUE, forcibly recook all guds
 *	@param	GUDSCookDir		The destination directory for cooked GUDS
 *	@param	GearsPCD		The UGearsPersistentCookerData instance
 *	@param	OutGUDSToCook	Array that gets filled in with the GUD files to cook
 *	@return	UBOOL			TRUE if they should, FALSE if not
 */
UBOOL FGearGameGUDSCookerHelper::ShouldBeCooked(
	class UCookPackagesCommandlet* Commandlet, 
	UBOOL bForceRecook, 
	FString& GUDSCookDir,
	UGearsPersistentCookerData* GearsPCD,
	TArray<FPackageCookerInfo>& OutGUDSToCook)
{
	if (GearsPCD->GUDPersistentData.ShouldBeCooked(Commandlet, bForceRecook, GUDSCookDir, OutGUDSToCook))
	{
		return TRUE;
	}

	return FALSE;
}

/**
 *	Get the generation tracking filename
 *
 *	@param	bStart		If TRUE, the start file is requested, otherwise the end one.
 */
FString FGearGameGUDSCookerHelper::GetGenerationTrackingFilename(UBOOL bStart)
{
	//@todo. THIS ASSUMES FULL ACCESS TO FILE SYSTEM ON WINDOWS!
	GFileManager->MakeDirectory(*GUDSOutDir, TRUE);
	if (bStart)
	{
		return GUDSOutDir + GUDGenerationStartFileName;
	}

	return GUDSOutDir + GUDGenerationEndFileName;
}

/**
 *	Touch the 'start generation' and 'end generation' files, creating them if necessary.
 *	These are used to determine if GUDS were successfully generated.
 *
 *	@param	bStart		If TRUE, the start file is to be touched, otherwise the end one.
 */
void FGearGameGUDSCookerHelper::TouchGenerationTrackingFile(UBOOL bStart)
{
	FString GenFilename = GetGenerationTrackingFilename(bStart);
	if (GFileManager->GetFileTimestamp(*GenFilename) == -1)
	{
		FArchive* FileAr = GFileManager->CreateFileWriter(*GenFilename, 0, GThrow);
		FileAr->Close();
		delete FileAr;
	}
	else
	{
		GFileManager->TouchFile(*GenFilename);
	}
}

/**
 *	Get the timestamp for the generation tracking file.
 *
 *	@param	bStart		If TRUE, check the start file, otherwise the end one.
 */
DOUBLE FGearGameGUDSCookerHelper::GetGenerationTrackingFileTimestamp(UBOOL bStart)
{
	return GFileManager->GetFileTimestamp(*(GetGenerationTrackingFilename(bStart)));
}

/**
 *	Check the timestamp of the given file vs. the generation tracking file requested.
 *
 *	@param	CheckFilename	The filename to check against.
 *	@param	bStart			If TRUE, check the start file, otherwise the end one.
 *
 *	@return	INT				-1 if the given file is older, 1 if it is newer or the same, 0 if it was not found
 */
INT FGearGameGUDSCookerHelper::CheckFileVersusGenerationTrackingFile(const TCHAR* CheckFilename, UBOOL bStart)
{
	DOUBLE CheckTime = GFileManager->GetFileTimestamp(CheckFilename);
	if (CheckTime == 0.0)
	{
		return 0;
	}
	DOUBLE GenTime = GetGenerationTrackingFileTimestamp(bStart);

	if (CheckTime < GenTime)
	{
		return -1;
	}

	return 1;
}

/**
 *	Clears out the generated GUDS folder...
 */
void FGearGameGUDSCookerHelper::ClearGeneratedGUDSFiles()
{
	GFileManager->DeleteDirectory(*GUDSOutDir, FALSE, TRUE);
}

//-----------------------------------------------------------------------------
//	GearGameCookerHelper
//-----------------------------------------------------------------------------
/**
* Initialize the cooker helpr and process any command line params
*
*	@param	Commandlet		The cookpackages commandlet being run
*	@param	Tokens			Command line tokens parsed from app
*	@param	Switches		Command line switches parsed from app
*/
void FGearGameCookerHelper::Init(
	class UCookPackagesCommandlet* Commandlet, 
	const TArray<FString>& Tokens, 
	const TArray<FString>& Switches )
{
	GetGameContentSeekfreeHelper()->InitOptions(Tokens,Switches);
	GetGUDSCooker()->Init(Commandlet, Tokens, Switches);
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
UPersistentCookerData* FGearGameCookerHelper::CreateInstance( const TCHAR* Filename, UBOOL bCreateIfNotFoundOnDisk )
{
	return UGearsPersistentCookerData::CreateInstance(Filename, bCreateIfNotFoundOnDisk);
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
 *	@param	MPMapFilenamePairs					""
 *	@param	ScriptFilenamePairs					""
 *	@param	StartupFilenamePairs				""
 *	@param	StandaloneSeekfreeFilenamePairs		""
 *	
 *	@return	UBOOL		TRUE if successful, FALSE is something went wrong.
 */
UBOOL FGearGameCookerHelper::GeneratePackageList( 
	class UCookPackagesCommandlet* Commandlet, 
	UE3::EPlatformType Platform,
	EShaderPlatform ShaderPlatform,
	TArray<FPackageCookerInfo>& NotRequiredFilenamePairs,
	TArray<FPackageCookerInfo>& RegularFilenamePairs,
	TArray<FPackageCookerInfo>& MapFilenamePairs,
	TArray<FPackageCookerInfo>& MPMapFilenamePairs,
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
		MPMapFilenamePairs,
		ScriptFilenamePairs,
		StartupFilenamePairs,
		StandaloneSeekfreeFilenamePairs
		) == FALSE)
	{
		bSuccess = FALSE;
	}

	if (GetGUDSCooker()->GeneratePackageList(Commandlet, Platform, ShaderPlatform,
		NotRequiredFilenamePairs, RegularFilenamePairs,
		MapFilenamePairs, MPMapFilenamePairs,
		ScriptFilenamePairs, StartupFilenamePairs,
		StandaloneSeekfreeFilenamePairs) == FALSE)
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
UBOOL FGearGameCookerHelper::CookObject(class UCookPackagesCommandlet* Commandlet, UPackage* Package, UObject* Object, UBOOL bIsSavedInSeekFreePackage)
{
	UBOOL bContinueProcessing = TRUE;

	if (GetGUDSCooker()->CookObject(Commandlet, Package, Object, bIsSavedInSeekFreePackage) == FALSE)
	{
		bContinueProcessing = FALSE;
	}

	return bContinueProcessing;
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
UPackage* FGearGameCookerHelper::LoadPackageForCookingCallback(class UCookPackagesCommandlet* Commandlet, const TCHAR* Filename)
{
	UPackage* Result=NULL;
	
	// load package for standalone seekfree game content for the given filename
	Result = GetGameContentSeekfreeHelper()->LoadPackageForCookingCallback(Commandlet,Filename);
	if (Result == NULL)
	{
		// See if the GUDSCooker wants it...
		Result = GetGUDSCooker()->LoadPackageForCookingCallback(Commandlet, Filename);
	}

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
UBOOL FGearGameCookerHelper::PostLoadPackageForCookingCallback(class UCookPackagesCommandlet* Commandlet, UPackage* InPackage)
{
	UBOOL bContinueProcessing = TRUE;

	// post process loaded package for standalone seekfree game content
	if (GetGameContentSeekfreeHelper()->PostLoadPackageForCookingCallback(Commandlet,InPackage) == FALSE)
	{
		bContinueProcessing = FALSE;
	}

	if (GetGUDSCooker()->PostLoadPackageForCookingCallback(Commandlet, InPackage) == FALSE)
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
void FGearGameCookerHelper::CleanupKismet(class UCookPackagesCommandlet* Commandlet, UPackage* InPackage)
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
UBOOL FGearGameCookerHelper::ShouldSoundCueBeIgnoredForPersistentFaceFX(class UCookPackagesCommandlet* Commandlet, const USoundCue* InSoundCue)
{
	return GetGUDSCooker()->ShouldSoundCueBeIgnoredForPersistentFaceFX(Commandlet, InSoundCue);
}

/**
 *	Dump out stats specific to the game cooker helper.
 */
void FGearGameCookerHelper::DumpStats()
{
	warnf( NAME_Log, TEXT("") );
	warnf( NAME_Log, TEXT("Game-specific Cooking Stats:") );
	GetGameContentSeekfreeHelper()->DumpStats();
	GetGUDSCooker()->DumpStats();
}
