/*=============================================================================
	LaunchEngineLoop.cpp: Main engine loop.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "LaunchPrivate.h"
#include "UnConsoleSupportContainer.h"
#include "Database.h"

#if _WINDOWS && !CONSOLE
#if GAMENAME == GEARGAME
#include "GearEditorCookerHelper.h"
#endif	//GAMENAME
#endif	//#if _WINDOWS && !CONSOLE

#if XBOX
	#include "XeViewport.h"
#endif

#if PS3
	#include "PS3Viewport.h"
#endif

/*-----------------------------------------------------------------------------
	Defines.
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
	Initializing registrants.
-----------------------------------------------------------------------------*/

// Static linking support forward declaration.
void InitializeRegistrantsAndRegisterNames();

// Please note that not all of these apply to every game/platform, but it's
// safe to declare them here and never define or reference them.

//	AutoInitializeRegistrants* declarations.
extern void AutoInitializeRegistrantsCore( INT& Lookup );
extern void AutoInitializeRegistrantsEngine( INT& Lookup );
extern void AutoInitializeRegistrantsGameFramework( INT& Lookup );
extern void AutoInitializeRegistrantsUnrealScriptTest( INT& Lookup );
extern void AutoInitializeRegistrantsIpDrv( INT& Lookup );
extern void AutoInitializeRegistrantsXeAudio( INT& Lookup );
extern void AutoInitializeRegistrantsXeDrv( INT& Lookup );
extern void AutoInitializeRegistrantsPS3Drv( INT& Lookup );
extern void AutoInitializeRegistrantsEditor( INT& Lookup );
extern void AutoInitializeRegistrantsUnrealEd( INT& Lookup );
extern void AutoInitializeRegistrantsALAudio( INT& Lookup );
extern void AutoInitializeRegistrantsWinDrv( INT& Lookup );
extern void AutoInitializeRegistrantsSDLDrv( INT& Lookup );
extern void AutoInitializeRegistrantsOnlineSubsystemLive( INT& Lookup );
extern void AutoInitializeRegistrantsOnlineSubsystemGameSpy( INT& Lookup );
extern void AutoInitializeRegistrantsGearGame( INT& Lookup );
extern void AutoInitializeRegistrantsGearEditor( INT& Lookup );
extern void AutoInitializeRegistrantsUTGame( INT& Lookup );
extern void AutoInitializeRegistrantsUTEditor( INT& Lookup );
extern void AutoInitializeRegistrantsExampleGame( INT& Lookup );
extern void AutoInitializeRegistrantsExampleEditor( INT& Lookup );
extern void AutoInitializeRegistrantsBmGame( INT& Lookup );
extern void AutoInitializeRegistrantsBmEditor( INT& Lookup );

//	AutoGenerateNames* declarations.
extern void AutoGenerateNamesCore();
extern void AutoGenerateNamesEngine();
extern void AutoGenerateNamesGameFramework();
extern void AutoGenerateNamesUnrealScriptTest();
extern void AutoGenerateNamesEditor();
extern void AutoGenerateNamesUnrealEd();
extern void AutoGenerateNamesIpDrv();
extern void AutoGenerateNamesOnlineSubsystemLive();
extern void AutoGenerateNamesOnlineSubsystemGameSpy();
extern void AutoGenerateNamesGearGame();
extern void AutoGenerateNamesGearEditor();
extern void AutoGenerateNamesUTGame();
extern void AutoGenerateNamesUTEditor();
extern void AutoGenerateNamesExampleGame();
extern void AutoGenerateNamesExampleEditor();
extern void AutoGenerateNamesBmGame();
extern void AutoGenerateNamesBmEditor();

// !!! FIXME: remove this.
#if PLATFORM_UNIX
#define appShowSplash(x) STUBBED("appShowSplash")
void appHideSplash() { STUBBED("appHideSplash"); }
#endif


/*-----------------------------------------------------------------------------
	Global variables.
-----------------------------------------------------------------------------*/

#define SPAWN_CHILD_PROCESS_TO_COMPILE 0

// General.
#if _WINDOWS && !CONSOLE
extern "C" {HINSTANCE hInstance;}
#endif

#if PS3
// we use a different GPackage here so that PS3 will use PS3Launch.log to not conflict with PC's
// Launch.log when using -writetohost
extern "C" {TCHAR GPackage[64]=TEXT("PS3Launch");}
#else
extern "C" {TCHAR GPackage[64]=TEXT("Launch");}
#endif

/** Critical section used by MallocThreadSafeProxy for synchronization										*/

#if XBOX
/** Remote debug command																					*/
static CHAR							RemoteDebugCommand[1024];		
/** Critical section for accessing remote debug command														*/
static FCriticalSection				RemoteDebugCriticalSection;	
#endif

/** Helper function called on first allocation to create and initialize GMalloc */
void GCreateMalloc()
{
#if PS3
	#include "FMallocPS3.h"

	// give malloc a buffer to use instead of calling malloc internally
	static char buf[0x8000];
	setvbuf(stdout, buf, _IOLBF, sizeof(buf));

#if USE_FMALLOC_BUILTIN
	FMallocPS3Builtin::StaticInit();
	static FMallocPS3Builtin MallocPS3;
#endif

#if USE_FMALLOC_POOLED
	FMallocPS3Pooled::StaticInit();
	static FMallocPS3Pooled MallocPS3;
#endif

#if USE_FMALLOC_DL
	FMallocPS3DL::StaticInit();
	static FMallocPS3DL MallocPS3;
#endif

	// point to the static object
	GMalloc = &MallocPS3;

#elif defined(XBOX)

#if USE_FMALLOC_POOLED && !USE_FMALLOC_DL
	GMalloc = new FMallocXenonPooled();
#elif USE_FMALLOC_DL
	GMalloc = new FMallocXenonDL();
#endif

#elif _DEBUG && !USE_MALLOC_PROFILER
	GMalloc = new FMallocDebug();
#elif PLATFORM_UNIX
	GMalloc = new FMallocAnsi();
#elif _WINDOWS
	GMalloc = new FMallocWindows();
#else
	#error Please define your platform.
#endif

// so now check to see if we are using a Mem Profiler which wraps the GMalloc
#if TRACK_MEM_USING_TAGS
#if USE_MALLOC_PROFILER || TRACK_MEM_USING_STAT_SECTIONS
	#error Only a single memory allocator should be used at once.  There are multiple defined
#endif	//#if USE_MALLOC_PROFILER || TRACK_MEM_USING_STAT_SECTIONS
	GMalloc = new FMallocProxySimpleTag( GMalloc );
#elif TRACK_MEM_USING_STAT_SECTIONS
#if USE_MALLOC_PROFILER
	warnf(TEXT("Running with multiple allocators defined!"));
#endif	//#if USE_MALLOC_PROFILER
	GMalloc = new FMallocProxySimpleTrack( GMalloc );
#elif USE_MALLOC_PROFILER
	#if (_WINDOWS && !_DEBUG && !LOAD_SYMBOLS_FOR_STACK_WALKING)
		#pragma message("**** MallocProfile not supported in this configuration!**** ")
	#endif
	GMalloc = new FMallocProfiler( GMalloc );
#endif

	// if the allocator is already thread safe, there is no need for the thread safe proxy
	if (!GMalloc->IsInternallyThreadSafe())
	{
		GMalloc = new FMallocThreadSafeProxy( GMalloc );
	}
}

#if defined(XBOX)
static FOutputDeviceFile					Log;
static FOutputDeviceAnsiError				Error;
static FFeedbackContextAnsi					GameWarn;
static FFileManagerXenon					FileManager;
#elif PS3
#include "PS3DownloadableContent.h"
static FSynchronizeFactoryPU				SynchronizeFactory;
static FThreadFactoryPU						ThreadFactory;
static FOutputDeviceFile					Log;
static FOutputDeviceAnsiError				Error;
static FFeedbackContextAnsi					GameWarn;
static FFileManagerPS3						FileManager;
static FQueuedThreadPoolPS3					ThreadPool;
#elif PLATFORM_UNIX
#include "ALAudio.h"
static FSynchronizeFactoryUnix				SynchronizeFactory;
static FThreadFactoryUnix					ThreadFactory;
static FOutputDeviceFile					Log;
static FOutputDeviceAnsiError				Error;
static FFeedbackContextAnsi					GameWarn;
static FFileManagerUnix						FileManager;
static FQueuedThreadPoolUnix				ThreadPool;
#else
#include "FFeedbackContextEditor.h"
#include "ALAudio.h"
static FOutputDeviceFile					Log;
static FOutputDeviceWindowsError			Error;
static FFeedbackContextWindows				GameWarn;
static FFeedbackContextEditor				UnrealEdWarn;
static FFileManagerWindows					FileManager;
static FCallbackEventDeviceEditor			UnrealEdEventCallback;
static FCallbackQueryDeviceEditor			UnrealEdQueryCallback;
static FOutputDeviceConsoleWindows			LogConsole;
static FOutputDeviceConsoleWindowsInherited InheritedLogConsole(LogConsole);
static FSynchronizeFactoryWin				SynchronizeFactory;
static FThreadFactoryWin					ThreadFactory;
static FQueuedThreadPoolWin					ThreadPool;
#endif

#include "DownloadableContent.h"
#if GAMENAME == UTGAME
#include "UTDownloadableContent.h"
#include "UTPatchHelper.h"
#endif



static FCallbackEventObserver				GameEventCallback;
static FCallbackQueryDevice					GameQueryCallback;

#if !FINAL_RELEASE
	#if WITH_UE3_NETWORKING
		// this will allow the game to receive object propagation commands from the network
		FListenPropagator					ListenPropagator;
	#endif	//#if WITH_UE3_NETWORKING
#endif

extern	TCHAR								GCmdLine[16384];
/** Whether we are using wxWindows or not */
extern	UBOOL								GUsewxWindows;

/** Whether to log all asynchronous IO requests out */
extern UBOOL GbLogAsyncLoading;

/** Thread used for async IO manager */
FRunnableThread*							AsyncIOThread;

/** 
* if TRUE then FDeferredUpdateResource::UpdateResources needs to be called 
* (should only be set on the rendering thread)
*/
UBOOL FDeferredUpdateResource::bNeedsUpdate = TRUE;

/** Whether force feedback should be enabled or disabled. */
UBOOL GEnableForceFeedback = TRUE;

#if _WINDOWS
#include "..\..\Launch\Resources\resource.h"
/** Resource ID of icon to use for Window */
#if   GAMENAME == GEARGAME
INT			GGameIcon	= IDICON_GoW;
INT			GEditorIcon	= IDICON_Editor;
#elif GAMENAME == UTGAME
INT			GGameIcon	= IDICON_UTGame;
INT			GEditorIcon	= IDICON_Editor;
#elif GAMENAME == EXAMPLEGAME
INT			GGameIcon	= IDICON_DemoGame;
INT			GEditorIcon	= IDICON_DemoEditor;
#elif GAMENAME == BMGAME
INT			GGameIcon	= IDICON_DemoGame;
INT			GEditorIcon	= IDICON_DemoEditor;
#else
	#error Hook up your game name here
#endif

#include "../Debugger/UnDebuggerCore.h"
#endif

#if USE_BINK_CODEC
#include "../Bink/Src/FullScreenMovieBink.h"
#endif

/** global for full screen movie player */
FFullScreenMovieSupport* GFullScreenMovie = NULL;

/** Globally shared viewport for fullscreen movie playback. */
FViewport* GFullScreenMovieViewport = NULL;

/** Initialize the global full screen movie player */
void appInitFullScreenMoviePlayer()
{
	check( GFullScreenMovie == NULL );

	UBOOL bUseSound = !(ParseParam(appCmdLine(),TEXT("nosound")) || GIsBenchmarking);

	// handle disabling of movies
	if( appStrfind(GCmdLine, TEXT("nomovie")) != NULL || GIsEditor || !GIsGame )
	{
		GFullScreenMovie = FFullScreenMovieFallback::StaticInitialize(bUseSound);
	}
	else
	{
#if USE_BINK_CODEC
		GFullScreenMovie = FFullScreenMovieBink::StaticInitialize(bUseSound);
	#if XBOX
		GFullScreenMovieViewport = new FXenonViewport(NULL,NULL,TEXT("Movie"),GScreenWidth,GScreenHeight,TRUE);	
	#elif PS3
		GFullScreenMovieViewport = new FPS3Viewport(NULL,NULL,GScreenWidth,GScreenHeight);
	#endif
#else
		GFullScreenMovie = FFullScreenMovieFallback::StaticInitialize(bUseSound);
#endif
	}
	check( GFullScreenMovie != NULL );
}

// From UMakeCommandlet.cpp - See if scripts need rebuilding at runtime
extern UBOOL AreScriptPackagesOutOfDate();

// Lets each platform perform any post appInit processing
extern void appPlatformPostInit();

#if WITH_PANORAMA || _XBOX
/**
 * Returns the title id for this game
 * Licensees need to add their own game(s) here!
 */
DWORD appGetTitleId(void)
{
#if   GAMENAME == EXAMPLEGAME || GAMENAME == BMGAME
	return 0;
#elif GAMENAME == UTGAME
	return 0x4D5707DB;
#elif GAMENAME == GEARGAME
	return 0x4D53082D;
#else
	#error Hook up your game's title id here
#endif
}
#endif

#if WITH_GAMESPY
/**
 * Returns the game name to use with GameSpy
 * Licensees need to add their own game(s) here!
 */
const TCHAR* appGetGameSpyGameName(void)
{
#if GAMENAME == UTGAME
#if PS3
	static TCHAR GSGameName[7];
	GSGameName[0] = TEXT('u');
	GSGameName[1] = TEXT('t');
	GSGameName[2] = TEXT('3');
	GSGameName[3] = TEXT('p');
	GSGameName[4] = TEXT('s');
	GSGameName[5] = TEXT('3');
	GSGameName[6] = 0;
	return GSGameName;
#else
	static TCHAR GSGameName[7];
	GSGameName[0] = TEXT('u');
	GSGameName[1] = TEXT('t');
	GSGameName[2] = TEXT('3');
	GSGameName[3] = TEXT('p');
	GSGameName[4] = TEXT('c');
	GSGameName[5] = 0;
	return GSGameName;
#endif
#elif GAMENAME == GEARGAME || GAMENAME == EXAMPLEGAME || GAMENAME == BMGAME
	return NULL;
#else
	#error Hook up your game's GameSpy game name here
#endif
}

/**
 * Returns the secret key used by this game
 * Licensees need to add their own game(s) here!
 */
const TCHAR* appGetGameSpySecretKey(void)
{
#if GAMENAME == UTGAME
	static TCHAR GSSecretKey[7];
	GSSecretKey[0] = TEXT('n');
	GSSecretKey[1] = TEXT('T');
	GSSecretKey[2] = TEXT('2');
	GSSecretKey[3] = TEXT('M');
	GSSecretKey[4] = TEXT('t');
	GSSecretKey[5] = TEXT('z');
	GSSecretKey[6] = 0;
	return GSSecretKey;
#elif GAMENAME == GEARGAME || GAMENAME == EXAMPLEGAME || GAMENAME == BMGAME
	return NULL;
#else
	#error Hook up your game's secret key here
#endif
}
#endif

/**
 * The single function that sets the gamename based on the #define GAMENAME
 * Licensees need to add their own game(s) here!
 */
void appSetGameName()
{
	// Initialize game name
#if   GAMENAME == EXAMPLEGAME
	appStrcpy(GGameName, TEXT("Example"));
#elif GAMENAME == GEARGAME
	appStrcpy(GGameName, TEXT("Gear"));
#elif GAMENAME == UTGAME
	appStrcpy(GGameName, TEXT("UT"));
#elif GAMENAME == BMGAME
	appStrcpy(GGameName, TEXT("Bm"));
#else
	#error Hook up your game name here
#endif
}

/**
 * A single function to get the list of the script packages that are used by 
 * the current game (as specified by the GAMENAME #define)
 *
 * @param PackageNames					The output array that will contain the package names for this game (with no extension)
 * @param bCanIncludeEditorOnlyPackages	If possible, should editor only packages be included in the list?
 */
void appGetGameScriptPackageNames(TArray<FString>& PackageNames, UBOOL bCanIncludeEditorOnlyPackages)
{
#if CONSOLE || PLATFORM_UNIX
	// consoles and unix NEVER want to load editor only packages
	bCanIncludeEditorOnlyPackages = FALSE;
#endif

#if   GAMENAME == EXAMPLEGAME
	PackageNames.AddItem(TEXT("ExampleGame"));
//	@todo: ExampleEditor is not in .u form yet
//	if (bCanIncludeEditorOnlyPackages)
//	{
//		PackageNames.AddItem(TEXT("ExampleEditor"));
//	}
#elif GAMENAME == GEARGAME
	PackageNames.AddItem(TEXT("GearGame"));
	PackageNames.AddItem(TEXT("GearGameContent"));
	PackageNames.AddItem(TEXT("GearGameContentWeapons"));
	if (bCanIncludeEditorOnlyPackages)
	{
		PackageNames.AddItem(TEXT("GearEditor"));
	}
#elif GAMENAME == UTGAME
	PackageNames.AddItem(TEXT("UTGame"));
	PackageNames.AddItem(TEXT("UTGameContent"));
	if (bCanIncludeEditorOnlyPackages)
	{
		PackageNames.AddItem(TEXT("UTEditor"));
	}
#elif GAMENAME == BMGAME
	PackageNames.AddItem(TEXT("BmGame"));
	if (bCanIncludeEditorOnlyPackages)
	{
		PackageNames.AddItem(TEXT("BmEditor"));
	}

#else
	#error Hook up your game name here
#endif
}

/**
 * A single function to get the list of the script packages containing native
 * classes that are used by the current game (as specified by the GAMENAME #define)
 * Licensees need to add their own game(s) to the definition of this function!
 *
 * @param PackageNames					The output array that will contain the package names for this game (with no extension)
 * @param bCanIncludeEditorOnlyPackages	If possible, should editor only packages be included in the list?
 */
void appGetGameNativeScriptPackageNames(TArray<FString>& PackageNames, UBOOL bCanIncludeEditorOnlyPackages)
{
#if CONSOLE || PLATFORM_UNIX
	// consoles and unix NEVER want to load editor only packages
	bCanIncludeEditorOnlyPackages = FALSE;
#endif

#if   GAMENAME == EXAMPLEGAME
	PackageNames.AddItem(TEXT("ExampleGame"));
#elif GAMENAME == GEARGAME
	PackageNames.AddItem(TEXT("GearGame"));
	if (bCanIncludeEditorOnlyPackages)
	{
		PackageNames.AddItem(TEXT("GearEditor"));
	}
#elif GAMENAME == UTGAME
	PackageNames.AddItem(TEXT("UTGame"));
	if (bCanIncludeEditorOnlyPackages)
	{
		PackageNames.AddItem(TEXT("UTEditor"));
	}
#elif GAMENAME == BMGAME
	PackageNames.AddItem(TEXT("BmGame"));
	if (bCanIncludeEditorOnlyPackages)
	{
		PackageNames.AddItem(TEXT("BmEditor"));
	}
#else
	#error Hook up your game name here
#endif

#if WITH_UE3_NETWORKING
	#if XBOX || WITH_PANORAMA
		// only include this if we are not checking native class sizes.  Currently, the native classes which reside on specific console
		// platforms will cause native class size checks to fail even tho class sizes are hopefully correct on the target platform as the PC 
		// doesn't have access to that native class.
		if( ParseParam(appCmdLine(),TEXT("CHECK_NATIVE_CLASS_SIZES")) == FALSE )
		{
			PackageNames.AddItem(TEXT("OnlineSubsystemLive"));
		}
	#elif PS3 || _WINDOWS
		#if WITH_GAMESPY
			// Only add GameSpy when we aren't cooking for 360
			if (!GIsCooking ||
				(GIsCooking && GCookingTarget != UE3::PLATFORM_Xenon))
			{
				PackageNames.AddItem(TEXT("OnlineSubsystemGameSpy"));
			}
		#endif
	#endif
	// Load the online layer for the target platform since the defines won't match
	if (GIsCooking)
	{
		if (GCookingTarget == UE3::PLATFORM_Xenon)
		{
			PackageNames.AddItem(TEXT("OnlineSubsystemLive"));
		}
	}
#endif
}

/**
 * A single function to get the list of the script packages that are used by the base engine.
 *
 * @param PackageNames					The output array that will contain the package names for this game (with no extension)
 * @param bCanIncludeEditorOnlyPackages	If possible, should editor only packages be included in the list?
 */
void appGetEngineScriptPackageNames(TArray<FString>& PackageNames, UBOOL bCanIncludeEditorOnlyPackages)
{
#if CONSOLE || PLATFORM_UNIX
	// consoles and unix NEVER want to load editor only packages
	bCanIncludeEditorOnlyPackages = FALSE;
#endif
	PackageNames.AddItem(TEXT("Core"));
	PackageNames.AddItem(TEXT("Engine"));
	PackageNames.AddItem(TEXT("GameFramework"));
	if( bCanIncludeEditorOnlyPackages )
	{
		PackageNames.AddItem(TEXT("Editor"));
		PackageNames.AddItem(TEXT("UnrealEd"));
	}	
	PackageNames.AddItem(TEXT("UnrealScriptTest"));
#if WITH_UE3_NETWORKING
	PackageNames.AddItem(TEXT("IpDrv"));
#endif	//#if WITH_UE3_NETWORKING
}

/**
 * Gets the list of all native script packages that the game knows about.
 * 
 * @param PackageNames The output list of package names
 * @param bExcludeGamePackages TRUE if the list should only contain base engine packages
 * @param bIncludeLocalizedSeekFreePackages TRUE if the list should include the _LOC_int loc files
 */
void GetNativeScriptPackageNames(TArray<FString>& PackageNames, UBOOL bExcludeGamePackages, UBOOL bIncludeLocalizedSeekFreePackages)
{
	// Assemble array of native script packages.
	appGetEngineScriptPackageNames(PackageNames, !GUseSeekFreeLoading);
	if( !bExcludeGamePackages )
	{
		appGetGameNativeScriptPackageNames(PackageNames, !GUseSeekFreeLoading);
	}

	// required for seek free loading
#if SUPPORTS_SCRIPTPATCH_CREATION
	//@script patcher
	if ( GUseSeekFreeLoading || GIsScriptPatcherActive )
#else
	if( GUseSeekFreeLoading )
#endif
	{
	bIncludeLocalizedSeekFreePackages = TRUE;
	}

	// insert any localization for Seek free packages if requested
	if (bIncludeLocalizedSeekFreePackages)
	{
		for( INT PackageIndex=0; PackageIndex<PackageNames.Num(); PackageIndex++ )
		{
			// only insert localized package if it exists
			FString LocalizedPackageName = PackageNames(PackageIndex) + LOCALIZED_SEEKFREE_SUFFIX;
			FString LocalizedFileName;
			if( !GUseSeekFreeLoading || GPackageFileCache->FindPackageFile( *LocalizedPackageName, NULL, LocalizedFileName ) )
			{
				PackageNames.InsertItem(*LocalizedPackageName, PackageIndex);
				// skip over the package that we just localized
				PackageIndex++;
			}
		}
	}
}

/**
 * Gets the list of packages that are precached at startup for seek free loading
 *
 * @param PackageNames The output list of package names
 * @param EngineConfigFilename Optional engine config filename to use to lookup the package settings
 */
void GetNonNativeStartupPackageNames(TArray<FString>& PackageNames, const TCHAR* EngineConfigFilename=NULL, UBOOL bIsCreatingHashes=FALSE)
{
	// if we aren't cooking, we actually just want to use the cooked startup package as the only startup package
	if (bIsCreatingHashes || (!GIsUCC && !GIsEditor))
	{
		// make sure the startup package exists
		PackageNames.AddItem(FString(TEXT("Startup")));
	}
	else
	{
		// look for any packages that we want to force preload at startup
		TMultiMap<FString,FString>* PackagesToPreload = GConfig->GetSectionPrivate( TEXT("Engine.StartupPackages"), 0, 1, 
			EngineConfigFilename ? EngineConfigFilename : GEngineIni );
		if (PackagesToPreload)
		{
			// go through list and add to the array
			for( TMultiMap<FString,FString>::TIterator It(*PackagesToPreload); It; ++It )
			{
				if (It.Key() == TEXT("Package"))
				{
					// add this package to the list to be fully loaded later
					PackageNames.AddItem(*(It.Value()));
				}
			}
		}
	}
}

/**
 * Kicks off a list of packages to be read in asynchronously in the background by the
 * async file manager. The package will be serialized from RAM later.
 * 
 * @param PackageNames The list of package names to async preload
 */
void AsyncPreloadPackageList(const TArray<FString>& PackageNames)
{
	// Iterate over all native script packages and preload them.
	for (INT PackageIndex=0; PackageIndex<PackageNames.Num(); PackageIndex++)
	{
		// let ULinkerLoad class manage preloading
		ULinkerLoad::AsyncPreloadPackage(*PackageNames(PackageIndex));
	}
}

/**
 * Fully loads a list of packages.
 * 
 * @param PackageNames The list of package names to load
 */
void LoadPackageList(const TArray<FString>& PackageNames)
{
	// Iterate over all native script packages and fully load them.
	for( INT PackageIndex=0; PackageIndex<PackageNames.Num(); PackageIndex++ )
	{
		UObject* Package = UObject::LoadPackage(NULL, *PackageNames(PackageIndex), LOAD_None);
	}
}

/**
 * This will load up all of the various "core" .u packages.
 * 
 * We do this as an optimization for load times.  We also do this such that we can be assured that 
 * all of the .u classes are loaded so we can then verify them.
 *
 * @param bExcludeGamePackages	Whether to exclude game packages
 */
void LoadAllNativeScriptPackages( UBOOL bExcludeGamePackages )
{
	TArray<FString> PackageNames;

	// call the shared function to get all native script package names
	GetNativeScriptPackageNames(PackageNames, bExcludeGamePackages, FALSE);

	// load them
	LoadPackageList(PackageNames);
}

/**
 * This function will look at the given command line and see if we have passed in a map to load.
 * If we have then use that.
 * If we have not then use the DefaultLocalMap which is stored in the Engine.ini
 * 
 * @see UGameEngine::Init() for this method of getting the correct start up map
 *
 * @param CommandLine Commandline to use to get startup map (NULL or "" will return default startup map)
 *
 * @return Name of the startup map without an extension (so can be used as a package name)
 */
FString GetStartupMap(const TCHAR* CommandLine)
{
	FURL DefaultURL;
	DefaultURL.LoadURLConfig( TEXT("DefaultPlayer"), GGameIni );

	// convert commandline to a URL
	FString Error;
	TCHAR Parm[4096]=TEXT("");
	const TCHAR* Tmp = CommandLine ? CommandLine : TEXT("");
	if (!ParseToken( Tmp, Parm, ARRAY_COUNT(Parm), 0 ) || Parm[0]=='-')
	{
		appStrcpy(Parm, *FURL::DefaultLocalMap);
	}
	FURL URL(&DefaultURL, Parm, TRAVEL_Partial);

	// strip off extension of the map if there is one
	return FFilename(URL.Map).GetBaseFilename();
}

/**
 * Load all startup packages. If desired preload async followed by serialization from memory.
 * Only native script packages are loaded from memory if we're not using the GUseSeekFreeLoading
 * codepath as we need to reset the loader on those packages and don't want to keep the bulk
 * data around. Native script packages don't have any bulk data so this doesn't matter.
 *
 * The list of additional packages can be found at Engine.StartupPackages and if seekfree loading
 * is enabled, the startup map is going to be preloaded as well.
 */
void LoadStartupPackages()
{
	// should script packages load from memory?
	UBOOL bSerializeStartupPackagesFromMemory = FALSE;
	GConfig->GetBool(TEXT("Engine.StartupPackages"), TEXT("bSerializeStartupPackagesFromMemory"), bSerializeStartupPackagesFromMemory, GEngineIni);

	// Get all native script package names.
	TArray<FString> NativeScriptPackages;
	GetNativeScriptPackageNames(NativeScriptPackages, FALSE, FALSE);

	// Get list of non-native startup packages.
	TArray<FString> NonNativeStartupPackages;
	GetNonNativeStartupPackageNames(NonNativeStartupPackages);

	if( bSerializeStartupPackagesFromMemory )
	{
		// start preloading them
		AsyncPreloadPackageList(NativeScriptPackages);

		if( GUseSeekFreeLoading )
		{
			// kick them off to be preloaded
			AsyncPreloadPackageList(NonNativeStartupPackages);
		}
	}

	// Load the native script packages.
	LoadPackageList(NativeScriptPackages);

	if( !GUseSeekFreeLoading && !GIsEditor )
	{
		// Reset loaders on native script packages as they are always fully loaded. This ensures the memory
		// for the async preloading process can be reclaimed.
		for( INT PackageIndex=0; PackageIndex<NativeScriptPackages.Num(); PackageIndex++ )
		{
			const FString& PackageName = NativeScriptPackages(PackageIndex);
			UPackage* Package = FindObjectChecked<UPackage>(NULL,*PackageName,TRUE);
			UObject::ResetLoaders( Package );
		}
	}
#if !CONSOLE
	// with PC seekfree, the shadercaches are loaded specially, they aren't cooked in, so at 
	// this point, their linkers are still in memory and should be reset
	else
	{
		for (TObjectIterator<UShaderCache> It; It; ++It)
		{
			UObject::ResetLoaders(It->GetOutermost());
		}
	}
#endif

	// Load the other startup packages.
	LoadPackageList(NonNativeStartupPackages);
}

/**
 * Get a list of all packages that may be needed at startup, and could be
 * loaded async in the background when doing seek free loading
 *
 * @param PackageNames The output list of package names
 * @param EngineConfigFilename Optional engine config filename to use to lookup the package settings
 */
void appGetAllPotentialStartupPackageNames(TArray<FString>& PackageNames, const TCHAR* EngineConfigFilename, UBOOL bIsCreatingHashes)
{
	// native script packages
	GetNativeScriptPackageNames(PackageNames, FALSE, TRUE);
	// startup packages from .ini
	GetNonNativeStartupPackageNames(PackageNames, EngineConfigFilename, bIsCreatingHashes);
	// add the startup map
	PackageNames.AddItem(*GetStartupMap(NULL));

	// go through and add localized versions of each package for all known languages
	// since they could be used at runtime depending on the language at runtime
	INT NumPackages = PackageNames.Num();
	for (INT PackageIndex = 0; PackageIndex < NumPackages; PackageIndex++)
	{
		// add the packagename with _XXX language extension
	    for (INT LangIndex = 0; GKnownLanguageExtensions[LangIndex]; LangIndex++)
		{
			PackageNames.AddItem(*(PackageNames(PackageIndex) + TEXT("_") + GKnownLanguageExtensions[LangIndex]));
		}
	}
}

/**
* Add a new entry to the list of shader source files
* Only unique entries which can be loaded are added as well as their #include files
*
* @param ShaderSourceFiles - [out] list of shader source files to add to
* @param ShaderFilename - shader file to add
*/
static void AddShaderSourceFileEntry( TArray<FString>& ShaderSourceFiles, const FString& ShaderFilename)
{
	FString ShaderFilenameBase( FFilename(ShaderFilename).GetBaseFilename() );

	// get the filename for the the vertex factory type
	if( !ShaderSourceFiles.ContainsItem(ShaderFilenameBase) )
	{
		ShaderSourceFiles.AddItem(ShaderFilenameBase);

		TArray<FString> ShaderIncludes;
		GetShaderIncludes(*ShaderFilenameBase,ShaderIncludes);
		for( INT IncludeIdx=0; IncludeIdx < ShaderIncludes.Num(); IncludeIdx++ )
		{
			ShaderSourceFiles.AddUniqueItem(ShaderIncludes(IncludeIdx));
		}
	}
}

/**
* Generate a list of shader source files that engine needs to load
*
* @param ShaderSourceFiles - [out] list of shader source files to add to
*/
void appGetAllShaderSourceFiles( TArray<FString>& ShaderSourceFiles )
{
	// add all shader source files for hashing
	for( TLinkedList<FVertexFactoryType*>::TIterator FactoryIt(FVertexFactoryType::GetTypeList()); FactoryIt; FactoryIt.Next() )
	{
		FVertexFactoryType* VertexFactoryType = *FactoryIt;
		if( VertexFactoryType )
		{
			FString ShaderFilename(VertexFactoryType->GetShaderFilename());
			AddShaderSourceFileEntry(ShaderSourceFiles,ShaderFilename);
		}
	}
	for( TLinkedList<FShaderType*>::TIterator ShaderIt(FShaderType::GetTypeList()); ShaderIt; ShaderIt.Next() )
	{
		FShaderType* ShaderType = *ShaderIt;
		if( ShaderType )
		{
			FString ShaderFilename(ShaderType->GetShaderFilename());
			AddShaderSourceFileEntry(ShaderSourceFiles,ShaderFilename);
		}
	}
	// also always add the MaterialTemplate.usf shader file
	AddShaderSourceFileEntry(ShaderSourceFiles,FString(TEXT("MaterialTemplate")));
	AddShaderSourceFileEntry(ShaderSourceFiles,FString(TEXT("Common")));
	AddShaderSourceFileEntry(ShaderSourceFiles,FString(TEXT("Definitions")));	
}

/**
* Kick off SHA verification for all shader source files
*/
void VerifyShaderSourceFiles()
{
	// get the list of shader files that can be used
	TArray<FString> ShaderSourceFiles;
	appGetAllShaderSourceFiles(ShaderSourceFiles);
	for( INT ShaderFileIdx=0; ShaderFileIdx < ShaderSourceFiles.Num(); ShaderFileIdx++ )
	{
		// load each shader source file. This will cache the shader source data after it has been verified
		LoadShaderSourceFile(*ShaderSourceFiles(ShaderFileIdx));
	}
}


/**
 * Checks for native class script/ C++ mismatch of class size and member variable
 * offset. Note that only the first and last member variable of each class in addition
 * to all member variables of noexport classes are verified to work around a compiler
 * limitation. The code is disabled by default as it has quite an effect on compile
 * time though is an invaluable tool for sanity checking and bringing up new
 * platforms.
 */
void CheckNativeClassSizes()
{
#if CHECK_NATIVE_CLASS_SIZES  // pass in via /DCHECK_NATIVE_CLASS_SIZES or set CL=/DCHECK_NATIVE_CLASS_SIZES and then this will be activated.  Good for setting up on your continuous integration machine
	debugf(TEXT("CheckNativeClassSizes..."));
	UBOOL Mismatch = FALSE;
	VERIFY_CLASS_SIZE_NODIE(AWeapon);
	VERIFY_CLASS_SIZE_NODIE(AKActor);
	#define NAMES_ONLY
	#define AUTOGENERATE_NAME(name)
	#define AUTOGENERATE_FUNCTION(cls,idx,name)
	#define VERIFY_CLASS_SIZES
	#include "CoreClasses.h"
	#include "EngineGameEngineClasses.h"
	#include "EngineClasses.h"
	#include "EngineAIClasses.h"
	#include "EngineMaterialClasses.h"
	#include "EngineTerrainClasses.h"
	#include "EnginePhysicsClasses.h"
	#include "EngineSequenceClasses.h"
	#include "EngineSoundClasses.h"
	#include "EngineInterpolationClasses.h"
	#include "EngineParticleClasses.h"
	#include "EngineAnimClasses.h"
	#include "EngineDecalClasses.h"
	#include "EngineFogVolumeClasses.h"
	#include "EngineMeshClasses.h"
	#include "EnginePrefabClasses.h"
	#include "EngineUserInterfaceClasses.h"
	#include "EngineUIPrivateClasses.h"
	#include "EngineUISequenceClasses.h"
	#include "EngineFoliageClasses.h"
	#include "EngineSpeedTreeClasses.h"
	#include "EngineLensFlareClasses.h"
	#include "GameFrameworkClasses.h"
	#include "GameFrameworkAnimClasses.h"
	#include "UnrealScriptTestClasses.h"
	#include "EngineFluidClasses.h"
#if !CONSOLE	
	#include "EditorClasses.h"
	#include "UnrealEdClasses.h"
	#include "UnrealEdCascadeClasses.h"
	#include "UnrealEdPrivateClasses.h"
#endif
#if WITH_UE3_NETWORKING
	#include "IpDrvClasses.h"
#endif	//#if WITH_UE3_NETWORKING
#if GAMENAME == GEARGAME
#include "GearGameClasses.h"
#include "GearGameAIClasses.h"
#include "GearGamePCClasses.h"
#include "GearGamePawnClasses.h"
#include "GearGameCameraClasses.h"
#include "GearGameSequenceClasses.h"
#include "GearGameSpecialMovesClasses.h"
#include "GearGameVehicleClasses.h"
#include "GearGameSoundClasses.h"
#include "GearGameWeaponClasses.h"
#include "GearGameUIClasses.h"
#include "GearGameUIPrivateClasses.h"
#include "GearGameAnimClasses.h"
#if !CONSOLE
	#include "GearEditorClasses.h"
#endif
#elif GAMENAME == UTGAME
#include "UTGameClasses.h"
#include "UTGameAnimationClasses.h"
#include "UTGameSequenceClasses.h"
#include "UTGameUIClasses.h"
#include "UTGameAIClasses.h"
#include "UTGameVehicleClasses.h"
#include "UTGameOnslaughtClasses.h"
#include "UTGameUIFrontEndClasses.h"
#if _WINDOWS
	#include "UTEditorClasses.h"
#endif

#elif GAMENAME == EXAMPLEGAME
	#include "ExampleGameClasses.h"
#if !CONSOLE
    #include "ExampleEditorClasses.h"
#endif
#elif GAMENAME == BMGAME
	#include "BmGameClasses.h"
#if !CONSOLE
    #include "BmEditorClasses.h"
#endif
#else
	#error Hook up your game name here
#endif
	#undef AUTOGENERATE_FUNCTION
	#undef AUTOGENERATE_NAME
	#undef NAMES_ONLY
	#undef VERIFY_CLASS_SIZES

	if( ( Mismatch == TRUE ) && ( GIsUnattended == TRUE ) )
	{
		appErrorf( NAME_Error, *LocalizeUnrealEd("Error_ScriptClassSizeMismatch") );
	}
	else if( Mismatch == TRUE )
	{
		appErrorf( NAME_FriendlyError, *LocalizeUnrealEd("Error_ScriptClassSizeMismatch") );
	}
	else
	{
		debugf(TEXT("CheckNativeClassSizes completed with no errors"));
	}
#endif  // #fi CHECK_NATIVE_CLASS_SIZES 
}


/**
 * Update GCurrentTime/ GDeltaTime while taking into account max tick rate.
 */
void appUpdateTimeAndHandleMaxTickRate()
{
	// start at now minus a millisecond so we don't get a zero delta.
	static DOUBLE LastTime = appSeconds() - 0.0001;
	static UBOOL bIsUsingFixedTimeStep = FALSE;

	// Figure out whether we want to use real or fixed time step.
	UBOOL bUseFixedTimeStep = GIsBenchmarking || GUseFixedTimeStep;

	// Calculate delta time and update time.
	if( bUseFixedTimeStep )
	{
		bIsUsingFixedTimeStep = TRUE;

		GDeltaTime		= GFixedDeltaTime;
		LastTime		= GCurrentTime;
		GCurrentTime	+= GDeltaTime;
	}
	else
	{
		GCurrentTime = appSeconds();

		// Did we just switch from a fixed time step to real-time?  If so, then we'll update our
		// cached 'last time' so our current interval isn't huge (or negative!)
		if( bIsUsingFixedTimeStep )
		{
			LastTime = GCurrentTime - GDeltaTime;
			bIsUsingFixedTimeStep = FALSE;
		}

		// Calculate delta time.
		FLOAT DeltaTime	= GCurrentTime - LastTime;

		// Negative delta time means something is wrong with the system. Error out so user can address issue.
		if( DeltaTime < 0 )
		{
			// AMD dual-core systems are a known issue that require AMD CPU drivers to be installed. Installer will take care of this for shipping.
			appErrorf(TEXT("Detected negative delta time - on AMD systems please install http://www.amd.com/us-en/Processors/TechnicalResources/0,,30_182_871_13118,00.html"));
		}

		// Get max tick rate based on network settings and current delta time.
		FLOAT MaxTickRate	= GEngine->GetMaxTickRate( DeltaTime );
		FLOAT WaitTime		= 0;
		// Convert from max FPS to wait time.
		if( MaxTickRate > 0 )
		{
			WaitTime = Max( 1.f / MaxTickRate - DeltaTime, 0.f );
		}

		// Enforce maximum framerate and smooth framerate by waiting.
		STAT( DOUBLE ActualWaitTime = 0.f ); 
		DWORD IdleStart = appCycles();
		if( WaitTime > 0 )
		{
			SCOPE_SECONDS_COUNTER(ActualWaitTime);
			SCOPE_CYCLE_COUNTER(STAT_GameTickWaitTime);
			SCOPE_CYCLE_COUNTER(STAT_GameIdleTime);

			// Sleep if we're waiting more than 5 ms. We set the scheduler granularity to 1 ms
			// at startup on PC. We reserve 2 ms of slack time which we will wait for by giving
			// up our timeslice.
			if( WaitTime > 5 / 1000.f )
			{
				appSleep( WaitTime - 3 / 1000.f );
			}

			// Give up timeslice for remainder of wait time.
			DOUBLE WaitStartTime = GCurrentTime;
			while( GCurrentTime - WaitStartTime < WaitTime )
			{
				GCurrentTime = appSeconds();
				appSleep( 0 );
			}
		}

		// Update game thread idle time even when stats are disabled so it works for LTCG-DebugConsole builds.
		extern DWORD GGameThreadIdle;
		GGameThreadIdle += appCycles() - IdleStart;

		SET_FLOAT_STAT(STAT_GameTickWantedWaitTime,WaitTime * 1000.f);
		SET_FLOAT_STAT(STAT_GameTickAdditionalWaitTime,Max<FLOAT>((ActualWaitTime-WaitTime)*1000.f,0.f));

		GDeltaTime		= GCurrentTime - LastTime;
		LastTime		= GCurrentTime;

		// Enforce a maximum delta time if wanted.
		FLOAT MaxDeltaTime = Cast<UGameEngine>(GEngine) ? (CastChecked<UGameEngine>(GEngine))->MaxDeltaTime : 0.f;
		if( MaxDeltaTime > 0.f )
		{
			// We don't want to modify delta time if we are dealing with network clients as either host or client.
			if( GWorld 
			// Not having a game info implies being a client.
			&&	GWorld->GetWorldInfo()->Game
			// NumPlayers and GamePlayer only match in standalone game types and handles the case of splitscreen.
			&&	GWorld->GetWorldInfo()->Game->NumPlayers == GEngine->GamePlayers.Num() )
			{
				// Happy clamping!
				GDeltaTime = Min<DOUBLE>( GDeltaTime, MaxDeltaTime );
			}
		}
	}
}

/*-----------------------------------------------------------------------------
	Frame end sync object implementation.
-----------------------------------------------------------------------------*/

/**
 * Special helper class for frame end sync. It respects a passed in option to allow one frame
 * of lag between the game and the render thread by using two events in round robin fashion.
 */
class FFrameEndSync
{
public:
	/** Events used to sync to. If frame lag is allowed we will sync to the previous event. */
	FEvent* Event[2];
	/** Current index into events array. */
	INT EventIndex;

	/**
	 * Constructor, initializing all member variables and creating events.
	 */
	FFrameEndSync()
	{
		check( GSynchronizeFactory );
		
		// Create sync events.
		Event[0] = GSynchronizeFactory->CreateSynchEvent();
		Event[1] = GSynchronizeFactory->CreateSynchEvent();

		// Trigger event 0 as we're waiting on its completion when we first start up in the
		// one frame lag case.
		Event[0]->Trigger();
		// Set event index to start at 1 as we've just triggered 0.
		EventIndex = 1;
	}

	/**
	 * Destructor, destroying the event objects.
	 */
	~FFrameEndSync()
	{
		check( GSynchronizeFactory );
		
		// Destroy events.
		GSynchronizeFactory->Destroy( Event[0] );
		GSynchronizeFactory->Destroy( Event[1] );
		
		// NULL out to aid debugging.
		Event[0] = NULL;
		Event[1] = NULL;
	}

	/**
	 * Syncs the game thread with the render thread. Depending on passed in bool this will be a total
	 * sync or a one frame lag.
	 */
	void Sync( UBOOL bAllowOneFrameThreadLag )
	{
		check(IsInGameThread());			

		// Reset the previously triggered event at this point. The render thread will set it.
		Event[EventIndex]->Reset();

		// Enqueue command to trigger event once the render thread executes it.
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			FenceCommand,
			FEvent*,EventToTrigger,Event[EventIndex],
			{
				// Trigger event to signal game thread.
				EventToTrigger->Trigger();
			});

		// Use two events if we allow a one frame lag.
		if( bAllowOneFrameThreadLag )
		{
			EventIndex = (EventIndex + 1) % 2;
		}

		SCOPE_CYCLE_COUNTER(STAT_GameIdleTime);
		DWORD IdleStart = appCycles();

		// Wait for completion of fence. Either the previous or the current one depending on whether
		// one frame lag is enabled or not.
		while(TRUE)
		{
			// Wait with a timeout for the event from the rendering thread; use a timeout to ensure that this doesn't deadlock
			// if the rendering thread crashes.
			if(Event[EventIndex]->Wait(100))
			{
				break;
			}

			// Break out of the loop with a fatal error if the rendering thread has crashed.
			CheckRenderingThreadHealth();
		}

		extern DWORD GGameThreadIdle;
		GGameThreadIdle += appCycles() - IdleStart;
	}
};
	

/*-----------------------------------------------------------------------------
	Debugging.
-----------------------------------------------------------------------------*/

#if _MSC_VER
/** Original C- Runtime pure virtual call handler that is being called in the (highly likely) case of a double fault */
_purecall_handler DefaultPureCallHandler;

/**
* Our own pure virtual function call handler, set by appPlatformPreInit. Falls back
* to using the default C- Runtime handler in case of double faulting.
*/
static void PureCallHandler()
{
	static UBOOL bHasAlreadyBeenCalled = FALSE;
	appDebugBreak();
	if( bHasAlreadyBeenCalled )
	{
		// Call system handler if we're double faulting.
		if( DefaultPureCallHandler )
		{
			DefaultPureCallHandler();
		}
	}
	else
	{
		bHasAlreadyBeenCalled = TRUE;
		if( GIsRunning )
		{
			appMsgf( AMT_OK, TEXT("Pure virtual function being called while application was running (GIsRunning == 1).") );
		}
		appErrorf(TEXT("Pure virtual function being called") );
	}
}
#endif

/*-----------------------------------------------------------------------------
	FEngineLoop implementation.
-----------------------------------------------------------------------------*/
#if !CONSOLE
FGameCookerHelper* GGameCookerHelper = NULL;
#endif	//#if !CONSOLE

INT FEngineLoop::PreInit( const TCHAR* CmdLine )
{
#if _MSC_VER
	// Use our own handler for pure virtuals being called.
	DefaultPureCallHandler = _set_purecall_handler( PureCallHandler );
#endif

	// remember thread id of the main thread
	GGameThreadId = appGetCurrentThreadId();
	GIsGameThreadIdInitialized = TRUE;

	// setup the streaming resource flush function pointer
	GFlushStreamingFunc = &FlushResourceStreaming;

	// Set the game name.
	appSetGameName();

	// Figure out whether we want to override the package map with the seekfree version. Needs to happen before the first call
	// to UClass::Link!
	GUseSeekFreePackageMap	= GUseSeekFreePackageMap || ParseParam( CmdLine, TEXT("SEEKFREEPACKAGEMAP") );
	// Figure out whether we are cooking for the demo or not.
	GIsCookingForDemo		= ParseParam( CmdLine, TEXT("COOKFORDEMO"));

	// look early for the editor token
	UBOOL bHasEditorToken = FALSE;
#if !CONSOLE
	// Figure out whether we're the editor, ucc or the game.
	const SIZE_T CommandLineSize = appStrlen(CmdLine)+1;
	TCHAR* CommandLineCopy			= new TCHAR[ CommandLineSize ];
	appStrcpy( CommandLineCopy, CommandLineSize, CmdLine );
	const TCHAR* ParsedCmdLine	= CommandLineCopy;
	FString Token				= ParseToken( ParsedCmdLine, 0);

	// trim any whitespace at edges of string - this can happen if the token was quoted with leaing or trailing whitespace
	// VC++ tends to do this in its "external tools" config
	Token = Token.Trim();

	bHasEditorToken = Token == TEXT("EDITOR");

	// set the seek free loading flag if it's given if we are running a commandlet or not
#if SHIPPING_PC_GAME
	// shipping PC game implies seekfreeloading for non-commandlets/editor
	GUseSeekFreeLoading = !bHasEditorToken;
#else
	GUseSeekFreeLoading = ParseParam(CmdLine, TEXT("SEEKFREELOADING")) && !bHasEditorToken;
#endif
#endif

	// Force a log flush after each line
	GForceLogFlush = ParseParam( CmdLine, TEXT("FORCELOGFLUSH") );

	// Force a complete recook of all sounds
	GForceSoundRecook = ParseParam( CmdLine, TEXT("FORCESOUNDRECOOK") );

	// Override compression settings wrt size.
	GAlwaysBiasCompressionForSize = ParseParam( CmdLine, TEXT("BIASCOMPRESSIONFORSIZE") );

	// Allows release builds to override not verifying GC assumptions. Useful for profiling as it's hitchy.
	extern UBOOL GShouldVerifyGCAssumptions;
	GShouldVerifyGCAssumptions = !ParseParam( CmdLine, TEXT("NOVERIFYGC") );

	// Please Trace FX
	extern UBOOL GShouldTraceFaceFX;
	GShouldTraceFaceFX = ParseParam ( CmdLine, TEXT("DEBUGFACEFX") );

#ifdef XBOX
	appInit( CmdLine, &Log, NULL       , &Error, &GameWarn, &FileManager, &GameEventCallback, &GameQueryCallback, FConfigCacheIni::Factory );
	// GearGame uses a movie so we don't need to load and display the splash screen.
	//appXenonShowSplash(TEXT("Xbox360\\Splash.bmp"));
#else  // XBOX
	// This is done in appXenonInit on Xenon.
	GSynchronizeFactory			= &SynchronizeFactory;
	GThreadFactory				= &ThreadFactory;
	GThreadPool					= &ThreadPool;
	INT NumThreadsInThreadPool	= 1;
#if _WINDOWS
	SYSTEM_INFO SI;
	GetSystemInfo(&SI);
	NumThreadsInThreadPool		= Max<INT>(1, SI.dwNumberOfProcessors - 1);
#endif
	verify(GThreadPool->Create(NumThreadsInThreadPool));

	FOutputDeviceConsole *LogConsolePtr = NULL;
#if _WINDOWS
	// see if we were launched from our .com command line launcher
	InheritedLogConsole.Connect();
	LogConsolePtr = &InheritedLogConsole;
#endif

	#if GAMENAME == UTGAME
		#if WITH_GAMESPY
		FString ModdedCmdLine(CmdLine);
		FString IgnoredString;
		// Append the default engine ini overload if not present
		if (Parse(CmdLine,TEXT("DEFENGINEINI="),IgnoredString) == FALSE)
		{
			ModdedCmdLine += TEXT(" -DEFENGINEINI=");
			ModdedCmdLine += appGameConfigDir();
			ModdedCmdLine += TEXT("DefaultEngineGameSpy.ini");
			CmdLine = *ModdedCmdLine;
		}
		#endif
	#endif

	FOutputDevice* LogToUse = &Log;
	if (ParseParam( CmdLine, TEXT("nowrite")))
	{
		LogToUse = GNull;
	}
	appInit( CmdLine, &Log, LogConsolePtr, &Error, &GameWarn, &FileManager, &GameEventCallback, &GameQueryCallback, FConfigCacheIni::Factory );
#endif	// XBOX

#if CHECK_PUREVIRTUALS
	appMsgf(AMT_OK, *LocalizeError(TEXT("Error_PureVirtualsEnabled"), TEXT("Launch")));
	appRequestExit(FALSE);
#endif

	// Create the object used to track task performance and report the data to a database. This relies on GConfig already being initialized, which
	// means that it needs to occur after appInit.
	GTaskPerfTracker = new FTaskPerfTracker();
	GTaskPerfMemDatabase = new FTaskPerfMemDatabase();

#if _WINDOWS
	const INT MinResolution[] = {640,480};
	if ( GetSystemMetrics(SM_CXSCREEN) < MinResolution[0] || GetSystemMetrics(SM_CYSCREEN) < MinResolution[1] )
	{
		appMsgf(AMT_OK, *LocalizeError(TEXT("Error_ResolutionTooLow"), TEXT("Launch")));
		appRequestExit( FALSE );
	}

	extern UBOOL HandleGameExplorerIntegration(const TCHAR* CmdLine);
	// allow for game explorer processing (including parental controls)
	if (HandleGameExplorerIntegration(appCmdLine()) == FALSE)
	{
		appRequestExit(FALSE);
	}

#if WITH_FIREWALL_SUPPORT
	extern void HandleFirewallIntegration(const TCHAR* CmdLine);
	if( ParseParam( appCmdLine(), TEXT( "installfw" ) ) || ParseParam( appCmdLine(), TEXT( "uninstallfw" ) ) )
	{
		HandleFirewallIntegration( appCmdLine() );
		appRequestExit( FALSE );
	}
#endif // WITH_FIREWALL_SUPPORT
#endif // _WINDOWS

	// Initialize system settings before anyone tries to use it...
	GSystemSettings.Initialize( bHasEditorToken );
	
	// Initialize global shadow volume setting
    GConfig->GetBool( TEXT("Engine.Engine"), TEXT("AllowShadowVolumes"), GAllowShadowVolumes, GEngineIni );

#if _WINDOWS
	// see if we are running a remote desktop session
	UBOOL bRunningRemoteDesktop = GetSystemMetrics(SM_REMOTESESSION);
#endif

#if _WINDOWS
	// get compatibility level
	const FCompatibilityLevelInfo MachineCompatLevel = appGetCompatibilityLevel();

	// set compatibility setting the first time the game is run
	// Don't set the CompatLevel if we're running remote desktop or in the editor.
	if ( !bRunningRemoteDesktop && !bHasEditorToken )
	{
		// deliberately write AppCompat info to EngineIni because if it is regenerated it won't have compat level settings copied into it.
		// By adding the CompatLevel results to the EngineIni file we ensure that always generate the Compat level settings when EngineIni is
		// regenerated.
		const TCHAR* AppCompatStr = TEXT("AppCompat");
		const TCHAR* AppCompatCompositeEntryStr = TEXT("CompatLevelComposite");
		const TCHAR* AppCompatCPUEntryStr = TEXT("CompatLevelCPU");
		const TCHAR* AppCompatGPUEntryStr = TEXT("CompatLevelGPU");
		FCompatibilityLevelInfo NewAppCompatLevel(0,0,0);
		FCompatibilityLevelInfo PreviouslySetCompatLevel(0,0,0);
		UBOOL bCompatLevelPreviouslySet = GConfig->GetInt( AppCompatStr, AppCompatCompositeEntryStr, (INT&)PreviouslySetCompatLevel.CompositeLevel, GEngineIni );
		bCompatLevelPreviouslySet &= GConfig->GetInt( AppCompatStr, AppCompatCPUEntryStr, (INT&)PreviouslySetCompatLevel.CPULevel, GEngineIni );
		bCompatLevelPreviouslySet &= GConfig->GetInt( AppCompatStr, AppCompatGPUEntryStr, (INT&)PreviouslySetCompatLevel.GPULevel, GEngineIni );
		debugf(NAME_Init, TEXT("Machine  detected compatibility level: Composite: %d. CPU: %d. GPU: %d."), MachineCompatLevel.CompositeLevel, MachineCompatLevel.CPULevel, MachineCompatLevel.GPULevel);
		debugf(NAME_Init, TEXT("Previous detected compatibility level: Composite: %d. CPU: %d. GPU: %d."), PreviouslySetCompatLevel.CompositeLevel, PreviouslySetCompatLevel.CPULevel, PreviouslySetCompatLevel.GPULevel);

		if (bCompatLevelPreviouslySet)
		{
			NewAppCompatLevel = PreviouslySetCompatLevel;
		}

		// Check if compatibility settings are given in the command line. They could override PCCompat tool settings.
		UBOOL bCompatScaleOverriden = Parse(appCmdLine(), TEXT("CompatScale="), (INT&)NewAppCompatLevel.CompositeLevel);
		if (bCompatScaleOverriden)
		{
			NewAppCompatLevel.CPULevel = NewAppCompatLevel.CompositeLevel;
			NewAppCompatLevel.GPULevel = NewAppCompatLevel.CompositeLevel;
			// if they specify < 0 override, use the detected compat level (gives a way to re-detect the defaults)
			if (NewAppCompatLevel.CompositeLevel < 0)
			{
				NewAppCompatLevel = MachineCompatLevel;
				debugf(NAME_Init, TEXT("Compatibility level overriden from the command line: forcing to detected level."));
			} else
			{
				debugf(NAME_Init, TEXT("Compatibility level overriden from the command line: %d"), NewAppCompatLevel.CompositeLevel);
			}
		}

		// set the detected compat level if we didn't override it and haven't set it before
		if (!bCompatScaleOverriden && !bCompatLevelPreviouslySet)
		{
			NewAppCompatLevel = MachineCompatLevel;
		}

		// only set the new compat level if:
		// * command line override
		// OR
		// * we haven't set it before.
		if (bCompatScaleOverriden || !bCompatLevelPreviouslySet )
		{
			// apply the new settings, but only write to the ini if we didn't explicitly override
			if (appSetCompatibilityLevel(NewAppCompatLevel, !bCompatScaleOverriden))
			{
				if (!bCompatScaleOverriden)
				{
					debugf(NAME_Init, TEXT("Writing new compatibility level to ini. Composite: %d. CPU: %d. GPU: %d."), NewAppCompatLevel.CompositeLevel, NewAppCompatLevel.CPULevel, NewAppCompatLevel.GPULevel);
					GConfig->SetInt( AppCompatStr, AppCompatCompositeEntryStr, NewAppCompatLevel.CompositeLevel, GEngineIni );
					GConfig->SetInt( AppCompatStr, AppCompatCPUEntryStr, NewAppCompatLevel.CPULevel, GEngineIni );
					GConfig->SetInt( AppCompatStr, AppCompatGPUEntryStr, NewAppCompatLevel.GPULevel, GEngineIni );
					GConfig->Flush( FALSE, GEngineIni );
				}
			}
		}
	}
#endif

	// Initialize the RHI.
	RHIInit();

#if !CONSOLE && !PLATFORM_UNIX   // !!! FIXME: get this into Unix port.
#if SPAWN_CHILD_PROCESS_TO_COMPILE
	// If the scripts need rebuilding, ask the user if they'd like to rebuild them
	//@script patcher
	if(Token != TEXT("MAKE") && Token != TEXT("PatchScript"))
	{	
		UBOOL bIsDone = FALSE;
		while (!bIsDone && AreScriptPackagesOutOfDate())
		{
			// figure out if we should compile scripts or not
			UBOOL bShouldCompileScripts = GIsUnattended;
			if( ( GIsUnattended == FALSE ) && ( GUseSeekFreeLoading == FALSE ) )
			{
				INT Result = appMsgf(AMT_YesNoCancel,TEXT("Scripts are outdated. Would you like to rebuild now?"));

				// check for Cancel
				if (Result == 2)
				{
					exit(1);
				}
				else
				{
					// 0 is Yes, 1 is No (unlike AMT_YesNo, where 1 is Yes)
					bShouldCompileScripts = Result == 0;
				}
			}

			if (bShouldCompileScripts)
			{
#ifdef _MSC_VER
				// get executable name
				TCHAR ExeName[MAX_PATH];
				GetModuleFileName(NULL, ExeName, ARRAY_COUNT(ExeName));

				// create the new process, with params as follows:
				//  - if we are running unattended, pass on the unattended flag
				//  - if not unattended, then tell the subprocess to pause when there is an error compiling
				//  - if we are running silently, pass on the flag
				void* ProcHandle = appCreateProc(ExeName, *FString::Printf(TEXT("make %s %s"), 
					GIsUnattended ? TEXT("-unattended") : TEXT("-nopauseonsuccess"), 
					GIsSilent ? TEXT("-silent") : TEXT("")));
				
				INT ReturnCode;
				// wait for it to finish and get return code
				while (!appGetProcReturnCode(ProcHandle, &ReturnCode))
				{
					appSleep(0);
				}

				// if we are running unattended, we can't run forever, so only allow one pass of compiling script code
				if (GIsUnattended)
				{
					bIsDone = TRUE;
				}
#else // _MSC_VER
				// if we can't spawn childprocess, just make within this process
				Token = TEXT("MAKE");
				bHasEditorToken = false;
				bIsDone = TRUE;
#endif
			}
			else
			{
				bIsDone = TRUE;
			}
		}

		// reload the package cache, as the child process may have created script packages!
		GPackageFileCache->CachePaths();
	}
#else // SPAWN_CHILD_PROCESS_TO_COMPILE

	// If the scripts need rebuilding, ask the user if they'd like to rebuild them
	//@script patcher
	if(Token != TEXT("MAKE") && Token != TEXT("PATCHSCRIPT") && AreScriptPackagesOutOfDate())
	{
		// If we are unattended, don't popup the dialog, but emit and error as scripts need to be updated!
		// Default response in unattended mode for appMsgf is cancel - which exits immediately. It should default to 'No' in this case
		if( ( GIsUnattended == FALSE ) && ( GUseSeekFreeLoading == FALSE ) )
		{
			switch ( appMsgf(AMT_YesNoCancel, TEXT("Scripts are outdated. Would you like to rebuild now?")) )
			{
				case 0:		// Yes - compile
					Token = TEXT("MAKE");
					bHasEditorToken = false;
					break;

				case 1:		// No - do nothing
					break;

				case 2:		// Cancel - exit
					appRequestExit( TRUE );
					break;

				default:
					warnf( TEXT( "Scripts are outdated. Failed to find valid case to switch on!" ) );
					break;
			}
		}
		else if( ( GIsUnattended == TRUE ) && ( GUseSeekFreeLoading == FALSE ) )
		{
			warnf( NAME_Error, TEXT( "Scripts are outdated. Please build them.") );
			appRequestExit( FALSE );
		}
	}

#endif

	if( Token == TEXT("MAKE") )
	{
		// allow ConditionalLink() to call Link() for non-intrinsic classes during make
		GUglyHackFlags |= HACK_ClassLoadingDisabled;

		// Rebuilding script requires some hacks in the engine so we flag that.
		GIsUCCMake = TRUE;
	}
#endif	// CONSOLE

	// Deal with static linking.
	InitializeRegistrantsAndRegisterNames();

	UBOOL bLoadStartupPackagesForCommandlet = FALSE;
#if !CONSOLE
	// if we are running a commandlet, disable GUseSeekfreeLoading at this point, so
	// look for special tokens

	// the server can handle seekfree loading, so don't disable it
	if (Token == TEXT("SERVER") || Token == TEXT("SERVERCOMMANDLET"))
	{
		// do nothing
	}
	// make wants seekfree loading off
	else if (Token == TEXT("MAKE") || Token == TEXT("MAKECOMMANDLET") ||
			 Token == TEXT("RUN"))
	{
		GUseSeekFreeLoading = FALSE;
		bLoadStartupPackagesForCommandlet = TRUE;
	}
	// otherwise, look if first token is a native commandlet
	else if( Token.Len() && appStrnicmp(*Token, TEXT("-"), 1) != 0)
	{
		// look for the commandlet by name
		UClass* Class = FindObject<UClass>(ANY_PACKAGE, *Token, FALSE);
		
		// tack on "Commandlet" and try again if not found
		if (!Class)
		{
			Class = FindObject<UClass>(ANY_PACKAGE, *(Token + TEXT("Commandlet")), FALSE);
		}

		// if a commandlet was specified, disable seekfree loading
		if (Class && Class->IsChildOf(UCommandlet::StaticClass()))
		{
			GUseSeekFreeLoading = FALSE;
			bLoadStartupPackagesForCommandlet = TRUE;
		}
	}
#endif

	// create global debug communications object (for talking to UnrealConsole)
//#if CONSOLE
#if !FINAL_RELEASE && !SHIPPING_PC_GAME && WITH_UE3_NETWORKING
	GDebugChannel = new FDebugServer();
	GDebugChannel->Init(DefaultDebugChannelReceivePort, DefaultDebugChannelSendPort);
#endif
//#endif

	// Create the streaming manager and add the default streamers.
	FStreamingManagerTexture* TextureStreamingManager = new FStreamingManagerTexture();
	TextureStreamingManager->AddTextureStreamingHandler( new FStreamingHandlerTextureStatic() );
	TextureStreamingManager->AddTextureStreamingHandler( new FStreamingHandlerTextureLevelForced() );
	GStreamingManager = new FStreamingManagerCollection();
	GStreamingManager->AddStreamingManager( TextureStreamingManager );
	
	GIOManager = new FIOManager();

	// Create the async IO manager.
#if XBOX
	FAsyncIOSystemXenon*	AsyncIOSystem = new FAsyncIOSystemXenon();
#elif PS3
	FAsyncIOSystemPS3*		AsyncIOSystem = new FAsyncIOSystemPS3();
#elif _MSC_VER
	FAsyncIOSystemWindows*	AsyncIOSystem = new FAsyncIOSystemWindows();
#elif PLATFORM_UNIX
	FAsyncIOSystemUnix*		AsyncIOSystem = new FAsyncIOSystemUnix();
#else
	#error implement async io manager for platform
#endif
	// This will auto- register itself with GIOMananger and be cleaned up when the manager is destroyed.
	AsyncIOThread = GThreadFactory->CreateThread( AsyncIOSystem, TEXT("AsyncIOSystem"), 0, 0, 16384, TPri_BelowNormal );
	check(AsyncIOThread);
#if XBOX
	// See UnXenon.h
	AsyncIOThread->SetProcessorAffinity(ASYNCIO_HWTHREAD);
#endif

	// Init physics engine before loading anything, in case we want to do things like cook during post-load.
	InitGameRBPhys();

#if WITH_FACEFX
	// Make sure FaceFX is initialized.
	UnInitFaceFX();
#endif // WITH_FACEFX

	// create the game specific DLC manager here
#if GAMENAME == UTGAME
	GDownloadableContent = new FUTDownloadableContent;
	GGamePatchHelper = new FUTPatchHelper;
#elif GAMENAME == GEARGAME
	GDownloadableContent = new FGearDownloadableContent;
	GGamePatchHelper = new FGamePatchHelper;
#else
	GDownloadableContent = new FDownloadableContent;
	GGamePatchHelper = new FGamePatchHelper;
#endif

	// create the platform specific DLC helper here
#if PS3
	GPlatformDownloadableContent = new FPS3DownloadableContent;
#elif XBOX
	GPlatformDownloadableContent = new FPlatformDownloadableContent;
#else
	GPlatformDownloadableContent = new FPCPlatformDownloadableContent;
#endif

	// Delete temporary files in cache.
	appCleanFileCache();

#if !CONSOLE
#if _WINDOWS  // !!! FIXME: get editor working on Unix.
	if( bHasEditorToken )
	{	
#if DEMOVERSION
		appErrorf(TEXT("Editor not supported in demo version."));
#endif

		// release our .com launcher -- this is to mimic previous behavior of detaching the console we launched from
		InheritedLogConsole.DisconnectInherited();

#if _WINDOWS
		if( !GIsCOMInitialized )
		{
			// here we start COM (used by some console support DLLs)
			CoInitialize(NULL);
			GIsCOMInitialized = TRUE;
		}

		// make sure we aren't running on Remote Desktop
		if (bRunningRemoteDesktop)
		{
			appMsgf(AMT_OK, *Localize(TEXT("Errors"), TEXT("Error_RemoteDesktop"), TEXT("Launch")));
			appRequestExit(FALSE);
			return 1;
		}
#endif

		// We're the editor.
		GIsClient	= 1;
		GIsServer	= 1;
		GIsEditor	= 1;
		GIsUCC		= 0;
		GGameIcon	= GEditorIcon;

		// UnrealEd requires a special callback handler and feedback context.
		GCallbackEvent	= &UnrealEdEventCallback;
		GCallbackQuery	= &UnrealEdQueryCallback;
		GWarn		= &UnrealEdWarn;

		// Remove "EDITOR" from command line.
		appStrcpy( GCmdLine, ParsedCmdLine );

		// Set UnrealEd as the current package (used for e.g. log and localization files).
		appStrcpy( GPackage, TEXT("UnrealEd") );
 	}
	else
#endif // _WINDOWS
	{
		// See whether the first token on the command line is a commandlet.

		//@hack: We need to set these before calling StaticLoadClass so all required data gets loaded for the commandlets.
		GIsClient	= 1;
		GIsServer	= 1;
		GIsEditor	= 1;
		GIsUCC		= 1;
		UBOOL bIsSeekFreeDedicatedServer = FALSE;

		UClass* Class = NULL;

#if SHIPPING_PC_GAME
		const UBOOL bIsRunningAsUser = TRUE;
#else
		const UBOOL bIsRunningAsUser = ParseParam(appCmdLine(), TEXT("user"));
#endif
		if ( bIsRunningAsUser )
		{
			const UBOOL bIsCooking = (appStristr(appCmdLine(), TEXT("CookPackages")) != NULL);
			const UBOOL bIsMaking = GIsUCCMake;
			if ( bIsCooking || bIsMaking || bLoadStartupPackagesForCommandlet )
			{
				// disable this flag to make package loading work as expected internally
				GIsUCC = FALSE;

				// load up the seekfree startup packages
				LoadStartupPackages();

				// restore the flag
				GIsUCC = TRUE;
			}
		}

		// We need to disregard the empty token as we try finding Token + "Commandlet" which would result in finding the
		// UCommandlet class if Token is empty.
		if( Token.Len() && appStrnicmp(*Token,TEXT("-"),1) != 0 )
		{
			DWORD CommandletLoadFlags = LOAD_NoWarn|LOAD_Quiet;
			UBOOL bNoFail = FALSE;
			UBOOL bIsUsingRunOption = Token == TEXT("RUN");

			if ( bIsUsingRunOption )
			{
				Token = ParseToken( ParsedCmdLine, 0);
				if ( Token.Len() == 0 )
				{
					warnf(TEXT("You must specify a commandlet to run, e.g. game.exe run test.samplecommandlet"));
					appRequestExit(FALSE);
					return 1;
				}
				bNoFail = TRUE;
				if ( Token == TEXT("MAKE") || Token == TEXT("MAKECOMMANDLET") )
				{
					// We can't bind to .u files if we want to build them via the make commandlet, hence the LOAD_DisallowFiles.
					CommandletLoadFlags |= LOAD_DisallowFiles;

					// allow the make commandlet to be invoked without requiring the package name
					Token = TEXT("Editor.MakeCommandlet");
				}

				if ( Token == TEXT("SERVER") || Token == TEXT("SERVERCOMMANDLET") )
				{
					// allow the server commandlet to be invoked without requiring the package name
					Token = TEXT("Engine.ServerCommandlet");
					bIsSeekFreeDedicatedServer = GUseSeekFreeLoading;
				}

				Class = LoadClass<UCommandlet>(NULL, *Token, NULL, CommandletLoadFlags, NULL );
				if ( Class == NULL && Token.InStr(TEXT("Commandlet")) == INDEX_NONE )
				{
					Class = LoadClass<UCommandlet>(NULL, *(Token+TEXT("Commandlet")), NULL, CommandletLoadFlags, NULL );
				}
			}
			else
			{
				// allow these commandlets to be invoked without requring the package name
				if ( Token == TEXT("MAKE") || Token == TEXT("MAKECOMMANDLET") )
				{
					// We can't bind to .u files if we want to build them via the make commandlet, hence the LOAD_DisallowFiles.
					CommandletLoadFlags |= LOAD_DisallowFiles;

					// allow the make commandlet to be invoked without requiring the package name
					Token = TEXT("Editor.MakeCommandlet");
				}

				else if ( Token == TEXT("SERVER") || Token == TEXT("SERVERCOMMANDLET") )
				{
					// allow the server commandlet to be invoked without requiring the package name
					Token = TEXT("Engine.ServerCommandlet");
					bIsSeekFreeDedicatedServer = GUseSeekFreeLoading;
				}
			}

			if (bIsSeekFreeDedicatedServer)
			{
				// when starting up a seekfree dedicated server, we act like we're the game starting up with the seekfree
				GIsEditor = FALSE;
				GIsUCC = FALSE;
				GIsGame = TRUE;

				// load up the seekfree startup packages
				LoadStartupPackages();
			}

			// See whether we're trying to run a commandlet. @warning: only works for native commandlets
			 
			//  Try various common name mangling approaches to find the commandlet class...
			if( !Class )
			{
				Class = FindObject<UClass>(ANY_PACKAGE,*Token,FALSE);
			}
			if( !Class )
			{
				Class = FindObject<UClass>(ANY_PACKAGE,*(Token+TEXT("Commandlet")),FALSE);
			}

			// Only handle '.' if we're using RUN commandlet option so we can specify maps including the extension.
			if( bIsUsingRunOption )
			{
				if( !Class && Token.InStr(TEXT(".")) != -1)
				{
					Class = LoadObject<UClass>(NULL,*Token,NULL,LOAD_NoWarn,NULL);
				}
				if( !Class && Token.InStr(TEXT(".")) != -1 )
				{
					Class = LoadObject<UClass>(NULL,*(Token+TEXT("Commandlet")),NULL,LOAD_NoWarn,NULL);
				}
			}

			// If the 'cookpackages' commandlet is being run, then create the game-specific helper...
#if !CONSOLE
			FString UpperCaseToken = Token.ToUpper();
			if (UpperCaseToken.InStr(TEXT("COOKPACKAGES")) != INDEX_NONE )
			{
#if GAMENAME == GEARGAME
				GGameCookerHelper = new FGearGameCookerHelper();
#else
				GGameCookerHelper = new FGameCookerHelper();
#endif
#endif	//#if !CONSOLE
			}

			// ... and if successful actually load it.
 			if( Class )
			{
				if ( Class->HasAnyClassFlags(CLASS_Intrinsic) )
				{
					// if this commandlet is native-only, we'll need to manually load its parent classes to ensure that it has
					// correct values in its PropertyLink array after it has been linked
					TArray<UClass*> ClassHierarchy;
					for ( UClass* ClassToLoad = Class->GetSuperClass(); ClassToLoad != NULL; ClassToLoad = ClassToLoad->GetSuperClass() )
					{
						ClassHierarchy.AddItem(ClassToLoad);
					}
					for ( INT i = ClassHierarchy.Num() - 1; i >= 0; i-- )
					{
						UClass* LoadedClass = UObject::StaticLoadClass( UObject::StaticClass(), NULL, *(ClassHierarchy(i)->GetPathName()), NULL, CommandletLoadFlags, NULL );
						check(LoadedClass);
					}
				}
				Class = UObject::StaticLoadClass( UCommandlet::StaticClass(), NULL, *Class->GetPathName(), NULL, CommandletLoadFlags, NULL );
			}
			
			if ( Class == NULL && bNoFail == TRUE )
			{
				appMsgf(AMT_OK, TEXT("Failed to find a commandlet named '%s'"), *Token);
				appRequestExit(FALSE);
				return 1;
			}
		}

		if( Class != NULL )
		{
#if DEMOVERSION
			if( !Class->IsChildOf(UServerCommandlet::StaticClass()) )
			{
				appErrorf(TEXT("Only supporting SERVER commandlet in demo version."));
			}
#endif
			//@todo - make this block a separate function

			// The first token was a commandlet so execute it.
	
			// Remove commandlet name from command line.
			appStrcpy( GCmdLine, ParsedCmdLine );

			// Set UCC as the current package (used for e.g. log and localization files).
			appStrcpy( GPackage, TEXT("UCC") );

			// Bring up console unless we're a silent build.
			if( GLogConsole && !GIsSilent )
			{
				GLogConsole->Show( TRUE );
			}

			// log out the engine meta data when running a commandlet
			debugf( NAME_Init, TEXT("Version: %i"), GEngineVersion );
			debugf( NAME_Init, TEXT("Epic Internal: %i"), GIsEpicInternal );
			debugf( NAME_Init, TEXT("Compiled: %s %s"), ANSI_TO_TCHAR(__DATE__), ANSI_TO_TCHAR(__TIME__) );
			debugf( NAME_Init, TEXT("Command line: %s"), appCmdLine() );
			debugf( NAME_Init, TEXT("Base directory: %s"), appBaseDir() );
			debugf( NAME_Init, TEXT("Character set: %s"), sizeof(TCHAR)==1 ? TEXT("ANSI") : TEXT("Unicode") );
			debugf( TEXT("Executing %s"), *Class->GetFullName() );

			// Allow commandlets to individually override those settings.
			UCommandlet* Default = CastChecked<UCommandlet>(Class->GetDefaultObject(TRUE));
			Class->ConditionalLink();

			if ( GIsRequestingExit )
			{
				// commandlet set GIsRequestingExit in StaticInitialize
				return 1;
			}

			GIsClient	= Default->IsClient;
			GIsServer	= Default->IsServer;
			GIsEditor	= Default->IsEditor;
			GIsGame		= !GIsEditor;
			GIsUCC		= TRUE;

			// Reset aux log if we don't want to log to the console window.
			if( !Default->LogToConsole )
			{
				GLog->RemoveOutputDevice( GLogConsole );
			}

#if _WINDOWS
			if( ParseParam(appCmdLine(),TEXT("AUTODEBUG")) )
			{
				debugf(TEXT("Attaching UnrealScript Debugger and breaking at first bytecode."));
				UDebuggerCore* Debugger = new UDebuggerCore();
				GDebugger 			= Debugger;

				// we want the UDebugger to break on the first bytecode it processes
				Debugger->SetBreakASAP(TRUE);

				// we need to allow the debugger to process debug opcodes prior to the first tick, so enable the debugger.
				Debugger->EnableDebuggerProcessing(TRUE);
			}
			else if ( ParseParam(appCmdLine(),TEXT("VADEBUG")) )
			{
				debugf(TEXT("Attaching UnrealScript Debugger"));

				// otherwise, if we are running the script debugger from within VS.NET, we need to create the UDebugger
				// so that we can receive log messages from the game and user input events (e.g. set breakpoint) from VS.NET
				UDebuggerCore* Debugger = new UDebuggerCore();
				GDebugger 			= Debugger;

				// we'll also attach the debugger so that it will break at any breakpoints which were set
				Debugger->AttachScriptDebugger(TRUE);

				// no ticking when running a commandlet and there is no script callstack at this point, so it's safe to enable the debugger.
				Debugger->EnableDebuggerProcessing(TRUE);
			}
#endif

			// Commandlets can't use disregard for GC optimizations as FEngineLoop::Init is not being called which does all 
			// the required fixup of creating defaults and marking dependencies as part of the root set.
			UObject::GObjFirstGCIndex = 0;

			GIsInitialLoad		= FALSE;

			//@hack: we don't want GIsRequestingExit=TRUE here for the server commandlet because it will break networking
			// as that code checks for this and doesn't add net packages to the package map when it is true
			// (probably should be checking something different, or maybe not checking it at all, or commandlets should be setting this flag themselves...
			// but this is the safer fix for now
			if (!Class->IsChildOf(UServerCommandlet::StaticClass()))
			{
				GIsRequestingExit	= TRUE;	// so CTRL-C will exit immediately
			}

			// allow the commandlet the opportunity to create a custom engine
			Class->GetDefaultObject<UCommandlet>()->CreateCustomEngine();
			if ( GEngine == NULL )
			{
#if !_WINDOWS  // !!! FIXME
				STUBBED("GIsEditor");
#else
				if ( GIsEditor )
				{
					UClass* EditorEngineClass	= UObject::StaticLoadClass( UEditorEngine::StaticClass(), NULL, TEXT("engine-ini:Engine.Engine.EditorEngine"), NULL, LOAD_None, NULL );

					// must do this here so that the engine object that we create on the next line receives the correct property values
					EditorEngineClass->GetDefaultObject(TRUE);
					EditorEngineClass->ConditionalLink();
					GEngine = GEditor			= ConstructObject<UEditorEngine>( EditorEngineClass );
					debugf(TEXT("Initializing Editor Engine..."));
					GEditor->InitEditor();
					debugf(TEXT("Initializing Editor Engine Completed"));
				}
				else
#endif
				{
					// pretend we're the game (with no client) again while loading/initializing the engine from seekfree 
					if (bIsSeekFreeDedicatedServer)
					{
						GIsEditor = FALSE;
						GIsGame = TRUE;
						GIsUCC = FALSE;
					}
					UClass* EngineClass = UObject::StaticLoadClass( UEngine::StaticClass(), NULL, TEXT("engine-ini:Engine.Engine.GameEngine"), NULL, LOAD_None, NULL );
					// must do this here so that the engine object that we create on the next line receives the correct property values
					EngineClass->GetDefaultObject(TRUE);
					EngineClass->ConditionalLink();
					GEngine = ConstructObject<UEngine>( EngineClass );
					debugf(TEXT("Initializing Game Engine..."));
					GEngine->Init();
					debugf(TEXT("Initializing Game Engine Completed"));

					// reset the flags if needed
					if (bIsSeekFreeDedicatedServer)
					{
						GIsEditor	= Default->IsEditor;
						GIsGame		= !GIsEditor;
						GIsUCC		= FALSE;
					}
				}
			}
	

#if !CONSOLE && _WINDOWS
			// Load in the console support dlls so commandlets can convert data
			FConsoleSupportContainer::GetConsoleSupportContainer()->LoadAllConsoleSupportModules();
#endif //!_CONSOLE && _WINDOWS

			UCommandlet* Commandlet = ConstructObject<UCommandlet>( Class );
			check(Commandlet);

			// Execute the commandlet.
			DOUBLE CommandletExecutionStartTime = appSeconds();

			Commandlet->InitExecution();
			Commandlet->ParseParms( appCmdLine() );
			INT ErrorLevel = Commandlet->Main( appCmdLine() );

			// Log warning/ error summary.
			if( Commandlet->ShowErrorCount )
			{
				if( GWarn->Errors.Num() || GWarn->Warnings.Num() )
				{
					SET_WARN_COLOR(COLOR_WHITE);
					warnf(TEXT(""));
					warnf(TEXT("Warning/Error Summary"));
					warnf(TEXT("---------------------"));

					SET_WARN_COLOR(COLOR_RED);
					for(INT I = 0; I < Min(50, GWarn->Errors.Num()); I++)
					{
						warnf((TCHAR*)GWarn->Errors(I).GetCharArray().GetData());
					}
					if (GWarn->Errors.Num() > 50)
					{
						SET_WARN_COLOR(COLOR_WHITE);
						warnf(TEXT("NOTE: Only first 50 warnings displayed."));
					}

					SET_WARN_COLOR(COLOR_YELLOW);
					for(INT I = 0; I < Min(50, GWarn->Warnings.Num()); I++)
					{
						warnf((TCHAR*)GWarn->Warnings(I).GetCharArray().GetData());
					}
					if (GWarn->Warnings.Num() > 50)
					{
						SET_WARN_COLOR(COLOR_WHITE);
						warnf(TEXT("NOTE: Only first 50 warnings displayed."));
					}
				}

				warnf(TEXT(""));

				if( ErrorLevel != 0 )
				{
					SET_WARN_COLOR(COLOR_RED);
					warnf( TEXT("Commandlet->Main return this error code: %d"), ErrorLevel );
					warnf( TEXT("With %d error(s), %d warning(s)"), GWarn->Errors.Num(), GWarn->Warnings.Num() );
				}
				else if( ( GWarn->Errors.Num() == 0 ) )
				{
					SET_WARN_COLOR(GWarn->Warnings.Num() ? COLOR_YELLOW : COLOR_GREEN);
					warnf( TEXT("Success - %d error(s), %d warning(s)"), GWarn->Errors.Num(), GWarn->Warnings.Num() );
				}
				else
				{
					SET_WARN_COLOR(COLOR_RED);
					warnf( TEXT("Failure - %d error(s), %d warning(s)"), GWarn->Errors.Num(), GWarn->Warnings.Num() );
					ErrorLevel = 1;
				}
				CLEAR_WARN_COLOR();
			}
			else
			{
				warnf( TEXT("Finished.") );
			}
		
			DOUBLE CommandletExecutionTime = appSeconds() - CommandletExecutionStartTime;
			warnf( TEXT("\nExecution of commandlet took:  %.2f seconds"), CommandletExecutionTime );
			GTaskPerfTracker->AddTask( *Class->GetName(), TEXT(""), CommandletExecutionTime );

			// We're ready to exit!
			return ErrorLevel;
		}
		else
#else
	{
#endif
		{
#if _WINDOWS
			// Load in the console support dlls so commandlets can convert data and
			// 360 texture stats work...
			FConsoleSupportContainer::GetConsoleSupportContainer()->LoadAllConsoleSupportModules();

			// make sure we aren't running on Remote Desktop
			if (bRunningRemoteDesktop)
			{
				appMsgf(AMT_OK, *LocalizeError(TEXT("Error_RemoteDesktop"), TEXT("Launch")));
				appRequestExit(FALSE);
				return 1;
			}
#endif

			// We're a regular client.
			GIsClient		= 1;
			GIsServer		= 0;
#if !CONSOLE
			GIsEditor		= 0;
			GIsUCC			= 0;
#endif

// handle launching dedicated server on console
#if CONSOLE
			// copy command line
			TCHAR* CommandLine	= new TCHAR[ appStrlen(appCmdLine())+1 ];
			appStrcpy( CommandLine, appStrlen(appCmdLine())+1, appCmdLine() );	
			// parse first token
			const TCHAR* ParsedCmdLine	= CommandLine;
			FString Token = ParseToken( ParsedCmdLine, 0);
			Token = Token.Trim();
			// dedicated server command line option
			if( Token == TEXT("SERVER") )
			{
				GIsClient = 0;
				GIsServer = 1;

				// Pretend RHI hasn't been initialized to mimick PC behavior.
				GIsRHIInitialized = FALSE;

				// Remove commandlet name from command line
				appStrcpy( GCmdLine, ParsedCmdLine );
#if XBOX
				// show server splash screen
				appXenonShowSplash(TEXT("Xbox360\\SplashServer.bmp"));
#endif
			}		
#endif

#if !FINAL_RELEASE
	#if WITH_UE3_NETWORKING
			// regular clients can listen for propagation requests
			FObjectPropagator::SetPropagator(&ListenPropagator);
	#endif	//#if WITH_UE3_NETWORKING
#endif
		}
	}

	// at this point, GIsGame is always not GIsEditor, because the only other way to set GIsGame is to be in a PIE world
	GIsGame = !GIsEditor;

#if !PS3
	// Exit if wanted.
	if( GIsRequestingExit )
	{
		if ( GEngine != NULL )
		{
			GEngine->PreExit();
		}
		appPreExit();
		// appExit is called outside guarded block.
		return 1;
	}
#endif

	// Movie recording.
	GIsDumpingTileShotMovie = Parse( appCmdLine(), TEXT("DUMPMOVIE_TILEDSHOT="), GScreenshotResolutionMultiplier );
	GIsTiledScreenshot = GIsDumpingTileShotMovie;

	// GIsDumpingMovie is mutually exclusive with GIsDumpingtileShotMovie, we only want one or the other
	GIsDumpingMovie	= !GIsDumpingTileShotMovie && ParseParam(appCmdLine(),TEXT("DUMPMOVIE"));

	// Disable force feedback
	GEnableForceFeedback = GEnableForceFeedback && !ParseParam(appCmdLine(),TEXT("NOFORCEFEEDBACK"));

	// Benchmarking.
	GIsBenchmarking	= ParseParam(appCmdLine(),TEXT("BENCHMARK"));

	// create the global full screen movie player. 
	// This needs to happen before the rendering thread starts since it adds a rendering thread tickable object
	appInitFullScreenMoviePlayer();
	
#if _WINDOWS
	if(!GIsBenchmarking)
	{
		// release our .com launcher -- this is to mimic previous behavior of detaching the console we launched from
		InheritedLogConsole.DisconnectInherited();
	}

#endif

	// Don't update ini files if benchmarking or -noini
	if( GIsBenchmarking || ParseParam(appCmdLine(),TEXT("NOINI")))
	{
		GConfig->Detach( GEngineIni );
		GConfig->Detach( GInputIni );
		GConfig->Detach( GGameIni );
		GConfig->Detach( GEditorIni );
		GConfig->Detach( GUIIni );
	}

	// do any post appInit processing, before the renderthread is started.
	appPlatformPostInit();

	UBOOL bOneThread = ParseParam(appCmdLine(),TEXT("ONETHREAD")); 

	// Make fluids simulate on the current thread if -onethread is supplied on the command line.
	if ( bOneThread )
	{
		extern UBOOL GThreadedFluidSimulation;
		GThreadedFluidSimulation = FALSE;
	}

	// Read texture fade settings
	{
		extern FLOAT GMipLevelFadingInRate;
		extern FLOAT GMipLevelFadingOutRate;
		GConfig->GetFloat(TEXT("Engine.Engine"), TEXT("MipLevelFadingInRate"), GMipLevelFadingInRate, GEngineIni);
		GConfig->GetFloat(TEXT("Engine.Engine"), TEXT("MipLevelFadingOutRate"), GMipLevelFadingOutRate, GEngineIni);
	}

	// -onethread will disable renderer thread
	if (GIsClient && !bOneThread)
	{
		// Create the rendering thread.  Note that commandlets don't make it to this point.
		if(GNumHardwareThreads > 1)
		{
			GUseThreadedRendering = TRUE;
			StartRenderingThread();
		}
	}

	return 0;
}

INT FEngineLoop::Init()
{
	if ( ParseParam(appCmdLine(), TEXT("logasync")))
	{
		GbLogAsyncLoading = TRUE;
		warnf(NAME_Warning, TEXT("*** ASYNC LOGGING IS ENABLED"));
	}

#if !CONSOLE
	// verify that all shader source files are intact
	VerifyShaderSourceFiles();
#endif

	// Load the global shaders.
	GetGlobalShaderMap();

#if XBOX || PS3	// The movie playing can only create the special startup movie viewport on XBOX and PS3.
	if (GFullScreenMovie && !GIsEditor)
	{
		// play a looping movie from RAM while booting up
		GFullScreenMovie->GameThreadInitiateStartupSequence();
	}
#endif

	// Load all startup packages, always including all native script packages. This is a superset of 
	// LoadAllNativeScriptPackages, which is done by the cooker, etc.
	LoadStartupPackages();

	if( !GUseSeekFreeLoading )
	{
		// Load the shader cache for the current shader platform if it hasn't already been loaded.
		GetLocalShaderCache( GRHIShaderPlatform );
	}

	// Iterate over all class objects and force the default objects to be created. Additionally also
	// assembles the token reference stream at this point. This is required for class objects that are
	// not taken into account for garbage collection but have instances that are.
	for( TObjectIterator<UClass> It; It; ++It )
	{
		UClass*	Class = *It;
		// Force the default object to be created.
		Class->GetDefaultObject( TRUE );
		// Assemble reference token stream for garbage collection/ RTGC.
		Class->AssembleReferenceTokenStream();
	}

	// Iterate over all objects and mark them to be part of root set.
	INT NumAlwaysLoadedObjects = 0;
	for( FObjectIterator It; It; ++It )
	{
		UObject*		Object		= *It;
		ULinkerLoad*	LinkerLoad	= Cast<ULinkerLoad>(Object);
		// Exclude linkers from root set if we're using seekfree loading.
		if( !GUseSeekFreeLoading || (LinkerLoad == NULL || LinkerLoad->HasAnyFlags(RF_ClassDefaultObject)) )
		{
			Object->AddToRoot();
			NumAlwaysLoadedObjects++;
		}
	}
	debugf(TEXT("%i objects as part of root set at end of initial load."), NumAlwaysLoadedObjects);
	debugf(TEXT("%i out of %i bytes used by permanent object pool."), UObject::GPermanentObjectPoolTail - UObject::GPermanentObjectPool, UObject::GPermanentObjectPoolSize );

	GIsInitialLoad = FALSE;

	// Figure out which UEngine variant to use.
	UClass* EngineClass = NULL;
	if( !GIsEditor )
	{
		// We're the game.
		EngineClass = UObject::StaticLoadClass( UGameEngine::StaticClass(), NULL, TEXT("engine-ini:Engine.Engine.GameEngine"), NULL, LOAD_None, NULL );
		GEngine = ConstructObject<UEngine>( EngineClass );
	}
	else
	{
#if !CONSOLE && !PLATFORM_UNIX
		// We're UnrealEd.
		EngineClass = UObject::StaticLoadClass( UUnrealEdEngine::StaticClass(), NULL, TEXT("engine-ini:Engine.Engine.UnrealEdEngine"), NULL, LOAD_None, NULL );
		GEngine = GEditor = GUnrealEd = ConstructObject<UUnrealEdEngine>( EngineClass );

		// we don't want any of the Live Update/Play On * functionality in a shipping pc editor
#if !SHIPPING_PC_GAME
		// load any Console support modules that exist
		FConsoleSupportContainer::GetConsoleSupportContainer()->LoadAllConsoleSupportModules();

		debugf(TEXT("Supported Consoles:"));
		for (FConsoleSupportIterator It; It; ++It)
		{
			debugf(TEXT("  %s"), It->GetConsoleName());
		}
#endif

#else
		check(0);
#endif
	}

	// if we should use all available cores then we want to compress with all
	if( ParseParam(appCmdLine(), TEXT("USEALLAVAILABLECORES")) == TRUE )
	{
		GNumUnusedThreads_SerializeCompressed = 0;
	}

	// If the -nosound or -benchmark parameters are used, disable sound.
	if(ParseParam(appCmdLine(),TEXT("nosound")) || GIsBenchmarking)
	{
		GEngine->bUseSound = FALSE;
	}

	// Disable texture streaming if that was requested
	if( ParseParam( appCmdLine(), TEXT( "NoTextureStreaming" ) ) )
	{
		GEngine->bUseTextureStreaming = FALSE;
	}

	// Setup up particle count clamping values...
	GEngine->MaxParticleSpriteCount = GEngine->MaxParticleVertexMemory / (4 * sizeof(FParticleSpriteVertex));
	GEngine->MaxParticleSubUVCount = GEngine->MaxParticleVertexMemory / (4 * sizeof(FParticleSpriteSubUVVertex));

	check( GEngine );
	debugf(TEXT("Initializing Engine..."));
	GEngine->Init();
	debugf(TEXT("Initializing Engine Completed"));

	// Verify native class sizes and member offsets.
	CheckNativeClassSizes();

	// Init variables used for benchmarking and ticking.
	GCurrentTime				= appSeconds();
	MaxFrameCounter				= 0;
	MaxTickTime					= 0;
	TotalTickTime				= 0;
	LastFrameCycles				= appCycles();

	FLOAT FloatMaxTickTime		= 0;
	Parse(appCmdLine(),TEXT("SECONDS="),FloatMaxTickTime);
	MaxTickTime					= FloatMaxTickTime;
	MaxFrameCounter				= appTrunc( MaxTickTime * 30 );

	// Use -FPS=X to override fixed tick rate if e.g. -BENCHMARK is used.
	FLOAT FixedFPS = 0;
	Parse(appCmdLine(),TEXT("FPS="),FixedFPS);
	if( FixedFPS > 0 )
	{
		GFixedDeltaTime = 1 / FixedFPS;
	}

	// Optionally Exec an exec file
	FString Temp;
	if( Parse(appCmdLine(), TEXT("EXEC="), Temp) )
	{
		Temp = FString(TEXT("exec ")) + Temp;
		UGameEngine* Engine = Cast<UGameEngine>(GEngine);
		if ( Engine != NULL && Engine->GamePlayers.Num() && Engine->GamePlayers(0) )
		{
			Engine->GamePlayers(0)->Exec( *Temp, *GLog );
		}
	}

	GIsRunning		= TRUE;

	// let the propagator do it's thing now that we are done initializing
	GObjectPropagator->Unpause();

#if !CONSOLE || XBOX
	// play a looping movie from RAM while booting up
	GFullScreenMovie->GameThreadInitiateStartupSequence();
#endif

	// stop the initial startup movies now. 
	// movies won't actually stop until startup sequence has finished
	GFullScreenMovie->GameThreadStopMovie();

	warnf(TEXT(">>>>>>>>>>>>>> Initial startup: %.2fs <<<<<<<<<<<<<<<"), appSeconds() - GStartTime);

	// handle test movie
	if( appStrfind(GCmdLine, TEXT("movietest")) != NULL )
	{
		// make sure language is correct for localized center channel
		UObject::SetLanguage(*appGetLanguageExt());
		// get the optional moviename from the command line (-movietest=Test.sfd)
		FString TestMovieName;
		Parse(GCmdLine, TEXT("movietest="), TestMovieName);
		// use default if not specified
		if( TestMovieName.Len() > 0 )
		{
			// play movie and wait for it to be done
			GFullScreenMovie->GameThreadPlayMovie(MM_PlayOnceFromStream, *TestMovieName);
			GFullScreenMovie->GameThreadWaitForMovie();			
		}
	}

	return 0;
}

void FEngineLoop::Exit()
{
	GIsRunning	= 0;
	GLogConsole	= NULL;

#if TRACK_FILEIO_STATS
	if( ParseParam( appCmdLine(), TEXT("DUMPFILEIOSTATS") ) )
	{
		GetFileIOStats()->DumpStats();
	}
#endif

#if !CONSOLE
	// Save the local shader cache if it has changed. This avoids loss of shader compilation work due to crashing.
    // GEMINI_TODO: Revisit whether this is worth thet slowdown in material editing once the engine has stabilized.
	SaveLocalShaderCaches();
#endif

	// Output benchmarking data.
	if( GIsBenchmarking )
	{
		FLOAT	MinFrameTime = 1000.f,
				MaxFrameTime = 0.f,
				AvgFrameTime = 0;

		// Determine min/ max framerate (discarding first 10 frames).
		for( INT i=10; i<FrameTimes.Num(); i++ )
		{		
			MinFrameTime = Min( MinFrameTime, FrameTimes(i) );
			MaxFrameTime = Max( MaxFrameTime, FrameTimes(i) );
			AvgFrameTime += FrameTimes(i);
		}
		AvgFrameTime /= FrameTimes.Num() - 10;

		// Output results to Benchmark/benchmark.log
		FString OutputString = TEXT("");
		appLoadFileToString( OutputString, *(appGameDir() + TEXT("Logs\\benchmark.log")) );
		OutputString += FString::Printf(TEXT("min= %6.2f   avg= %6.2f   max= %6.2f%s"), 1000.f / MaxFrameTime, 1000.f / AvgFrameTime, 1000.f / MinFrameTime, LINE_TERMINATOR );
		appSaveStringToFile( OutputString, *(appGameDir() + TEXT("Logs\\benchmark.log")) );

		FrameTimes.Empty();
	}

#if PS3
	debugf(TEXT("================================================"));
	debugf(TEXT("PS3 is known to crash on shutdown. This is OK."));
	debugf(TEXT("================================================"));
#endif

	// Make sure we're not in the middle of loading something.
	UObject::FlushAsyncLoading();
	// Block till all outstanding resource streaming requests are fulfilled.
	GStreamingManager->BlockTillAllRequestsFinished();
	
	if ( GEngine != NULL )
	{
		GEngine->PreExit();
	}
	appPreExit();
	DestroyGameRBPhys();

#if WITH_FACEFX
	// Make sure FaceFX is shutdown.
	UnShutdownFaceFX();
#endif // WITH_FACEFX

	// Stop the rendering thread.
	StopRenderingThread();

	delete GStreamingManager;
	GStreamingManager	= NULL;

	delete GIOManager;
	GIOManager			= NULL;

	// shutdown debug communications
#if WITH_UE3_NETWORKING && !SHIPPING_PC_GAME
	delete GDebugChannel;
	GDebugChannel = NULL;
#endif	//#if WITH_UE3_NETWORKING

	GThreadFactory->Destroy( AsyncIOThread );

#if STATS
	// Shutdown stats
	GStatManager.Destroy();
#endif
#if WITH_UE3_NETWORKING
	// Shutdown sockets layer
	appSocketExit();
#endif	//#if WITH_UE3_NETWORKING

	// Delete the task perf tracking object used to upload stats to a DB.
	delete GTaskPerfTracker;
	GTaskPerfTracker = NULL;

	// Delete the task perfmem database object used to upload perfmem stats to a DB.
	delete GTaskPerfMemDatabase;
	GTaskPerfMemDatabase = NULL;
}


void FEngineLoop::Tick()
{
	// Suspend game thread while we're trace dumping to avoid running out of memory due to holding
	// an IRQ for too long.
	while( GShouldSuspendGameThread )
	{
		appSleep( 1 );
	}

	if (GHandleDirtyDiscError)
	{
		appSleepInfinite();
	}

	// Flush debug output which has been buffered by other threads.
	GLog->FlushThreadedLogs();

#if !CONSOLE
	// If the local shader cache has changed, save it.
	static UBOOL bIsFirstTick = TRUE;
	if ((!GIsEditor && !UObject::IsAsyncLoading()) || bIsFirstTick)
	{
		SaveLocalShaderCaches();
		bIsFirstTick = FALSE;
	}
#endif

	if( GDebugger )
	{
		GDebugger->NotifyBeginTick();
	}

	// Exit if frame limit is reached in benchmark mode.
	if( (GIsBenchmarking && MaxFrameCounter && (GFrameCounter > MaxFrameCounter))
	// or timelimt is reached if set.
	||	(MaxTickTime && (TotalTickTime > MaxTickTime)) )
	{
		GEngine->DumpFPSChart();
		appRequestExit(0);
	}

	// Set GCurrentTime, GDeltaTime and potentially wait to enforce max tick rate.
	appUpdateTimeAndHandleMaxTickRate();

	// handle some per-frame tasks on the rendering thread
	ENQUEUE_UNIQUE_RENDER_COMMAND(
		ResetDeferredUpdatesAndTickTickables,
	{
		FDeferredUpdateResource::ResetNeedsUpdate();

		// make sure that rendering thread tickables get a change to tick, even if the game thread
		// is keeping the rendering queue always full
		TickRenderingTickables();
	});

	// Update.
	GEngine->Tick( GDeltaTime );

	// Update RHI.
	{
		SCOPE_CYCLE_COUNTER(STAT_RHITickTime);
		RHITick( GDeltaTime );
	}

	// Increment global frame counter. Once for each engine tick.
	GFrameCounter++;

	// Disregard first few ticks for total tick time as it includes loading and such.
	if( GFrameCounter > 5 )
	{
		TotalTickTime+=GDeltaTime;
	}

	// Find the objects which need to be cleaned up the next frame.
	FPendingCleanupObjects* PreviousPendingCleanupObjects = PendingCleanupObjects;
	PendingCleanupObjects = GetPendingCleanupObjects();

	// Sync game and render thread. Either total sync or allowing one frame lag.
	static FFrameEndSync FrameEndSync;
	FrameEndSync.Sync( GSystemSettings.bAllowOneFrameThreadLag );

	// Delete the objects which were enqueued for deferred cleanup before the previous frame.
	delete PreviousPendingCleanupObjects;

#if !CONSOLE && !PLATFORM_UNIX
	// Handle all incoming messages if we're not using wxWindows in which case this is done by their
	// message loop.
	if( !GUsewxWindows )
	{
		appWinPumpMessages();
	}

	
	// check to see if the window in the foreground was made by this process (ie, does this app
	// have focus)
	DWORD ForegroundProcess;
	GetWindowThreadProcessId(GetForegroundWindow(), &ForegroundProcess);
	UBOOL HasFocus = ForegroundProcess == GetCurrentProcessId();

	// If editor thread doesn't have the focus, don't suck up too much CPU time.
	if( GIsEditor )
	{
		static UBOOL HadFocus=1;
		if( HadFocus && !HasFocus )
		{
			// Drop our priority to speed up whatever is in the foreground.
			SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL );
		}
		else if( HasFocus && !HadFocus )
		{
			// Boost our priority back to normal.
			SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_NORMAL );
		}
		if( !HasFocus )
		{
			// Sleep for a bit to not eat up all CPU time.
			appSleep(0.005f);
		}
		HadFocus = HasFocus;
	}

	// if its our window, allow sound, otherwise silence it
	GALGlobalVolumeMultiplier = HasFocus ? 1.0f : 0.0f;

#endif

	if( GIsBenchmarking )
	{
#if STATS
		FrameTimes.AddItem( GFPSCounter.GetFrameTime() * 1000.f );
#endif
	}

#if XBOX
	// Handle remote debugging commands.
	{
		FScopeLock ScopeLock(&RemoteDebugCriticalSection);
		if( RemoteDebugCommand[0] != '\0' )
		{
			new(GEngine->DeferredCommands) FString(ANSI_TO_TCHAR(RemoteDebugCommand));
		}
		RemoteDebugCommand[0] = '\0';
	}
#endif

#if WITH_UE3_NETWORKING && !SHIPPING_PC_GAME
	if ( GDebugChannel )
	{
		GDebugChannel->Tick();
	}
#endif	//#if WITH_UE3_NETWORKING

	// Execute deferred commands.
	for( INT DeferredCommandsIndex=0; DeferredCommandsIndex<GEngine->DeferredCommands.Num(); DeferredCommandsIndex++ )
	{
		// Use LocalPlayer if available...
		if( GEngine->GamePlayers.Num() && GEngine->GamePlayers(0) )
		{
			ULocalPlayer* Player = GEngine->GamePlayers(0);
			{
				Player->Exec( *GEngine->DeferredCommands(DeferredCommandsIndex), *GLog );
			}
		}
		// and fall back to UEngine otherwise.
		else
		{
			GEngine->Exec( *GEngine->DeferredCommands(DeferredCommandsIndex), *GLog );
		}
		
	}
	GEngine->DeferredCommands.Empty();
}


/*-----------------------------------------------------------------------------
	Static linking support.
-----------------------------------------------------------------------------*/

/**
 * Initializes all registrants and names for static linking support.
 */
void InitializeRegistrantsAndRegisterNames()
{
	// Static linking.
	for( INT k=0; k<ARRAY_COUNT(GNativeLookupFuncs); k++ )
	{
		GNativeLookupFuncs[k] = NULL;
	}
	INT Lookup = 0;
	// Auto-generated lookups and statics
	AutoInitializeRegistrantsCore( Lookup );
	AutoInitializeRegistrantsEngine( Lookup );
	AutoInitializeRegistrantsGameFramework( Lookup );
	AutoInitializeRegistrantsUnrealScriptTest( Lookup );
#if WITH_UE3_NETWORKING
	AutoInitializeRegistrantsIpDrv( Lookup );
#endif	//#if WITH_UE3_NETWORKING
#if defined(XBOX)
	AutoInitializeRegistrantsXeAudio( Lookup );
	AutoInitializeRegistrantsXeDrv( Lookup );
#elif PS3
	AutoInitializeRegistrantsPS3Drv( Lookup );
#elif _WINDOWS  // !!! FIXME: some of these will be Unix, too!
	AutoInitializeRegistrantsEditor( Lookup );
	AutoInitializeRegistrantsUnrealEd( Lookup );
	AutoInitializeRegistrantsALAudio( Lookup );
	AutoInitializeRegistrantsWinDrv( Lookup );
#elif PLATFORM_UNIX && !USE_NULL_RHI
	AutoInitializeRegistrantsALAudio( Lookup );
	AutoInitializeRegistrantsSDLDrv( Lookup );
#endif

#if WITH_UE3_NETWORKING
	#if XBOX || WITH_PANORAMA
		AutoInitializeRegistrantsOnlineSubsystemLive( Lookup );
	#elif (PS3 || _WINDOWS) && WITH_GAMESPY
		AutoInitializeRegistrantsOnlineSubsystemGameSpy( Lookup );
	#endif
#endif

#if GAMENAME == GEARGAME
	AutoInitializeRegistrantsGearGame( Lookup );
#if _WINDOWS
	AutoInitializeRegistrantsGearEditor( Lookup );
#endif

#elif GAMENAME == UTGAME
	AutoInitializeRegistrantsUTGame( Lookup );
#if _WINDOWS
	AutoInitializeRegistrantsUTEditor( Lookup );
#endif

#elif GAMENAME == EXAMPLEGAME
	AutoInitializeRegistrantsExampleGame( Lookup );
#if _WINDOWS
	AutoInitializeRegistrantsExampleEditor( Lookup );
#endif

#elif GAMENAME == BMGAME
	AutoInitializeRegistrantsBmGame( Lookup );
#if _WINDOWS
	AutoInitializeRegistrantsBmEditor( Lookup );
#endif

#else
	#error Hook up your game name here
#endif
	// It is safe to increase the array size of GNativeLookupFuncs if this assert gets triggered.
	check( Lookup < ARRAY_COUNT(GNativeLookupFuncs) );

//	AutoGenerateNames* declarations.
	AutoGenerateNamesCore();
	AutoGenerateNamesEngine();
	AutoGenerateNamesGameFramework();
	AutoGenerateNamesUnrealScriptTest();
#if _WINDOWS
	AutoGenerateNamesEditor();
	AutoGenerateNamesUnrealEd();
#endif
#if WITH_UE3_NETWORKING
	AutoGenerateNamesIpDrv();
#endif	//#if WITH_UE3_NETWORKING

#if GAMENAME == GEARGAME
	AutoGenerateNamesGearGame();
	#if _WINDOWS
		AutoGenerateNamesGearEditor();
	#endif

#elif GAMENAME == UTGAME
	AutoGenerateNamesUTGame();
	#if _WINDOWS
		AutoGenerateNamesUTEditor();
	#endif

#elif GAMENAME == EXAMPLEGAME
	AutoGenerateNamesExampleGame();
	#if _WINDOWS
		AutoGenerateNamesExampleEditor();
	#endif

#elif GAMENAME == BMGAME
	AutoGenerateNamesBmGame();
	#if _WINDOWS
		AutoGenerateNamesBmEditor();
	#endif

#else
	#error Hook up your game name here
#endif

#if WITH_UE3_NETWORKING
	#if XBOX || WITH_PANORAMA
		AutoGenerateNamesOnlineSubsystemLive();
	#elif (PS3 || _WINDOWS) && WITH_GAMESPY
		AutoGenerateNamesOnlineSubsystemGameSpy();
	#endif
#endif

}


/** 
* Returns whether the line can be broken between these two characters
*/
UBOOL appCanBreakLineAt( TCHAR Previous, TCHAR Current )
{
#if 0 //GAMENAME == GEARGAME && !PS3
/*
WordWrap_CanBreakLineAt is a method from the Microsoft XDK sample "WordWrap" (Source\Samples\System\WordWrap).  If you are developing a game
for the Xbox360, and wish to use this code, copy the WordwrapUtil.h and WordwrapUtil.cpp file (from the XDK Samples) into the WarfareGame\Src
directory.  You must also contact Microsoft to get a license to use this code in your game.
*/
	// Use the microsoft word-wrapping code from the XDK sample source
	extern bool WordWrap_CanBreakLineAt( WCHAR prev, WCHAR curr );
	return WordWrap_CanBreakLineAt(Previous, Current);

#else

	return appIsPunct(Previous) && Previous != TEXT('\'') || appIsWhitespace(Current);

#endif
}

/*-----------------------------------------------------------------------------
	Remote debug channel support.
-----------------------------------------------------------------------------*/

#if (defined XBOX) && ALLOW_NON_APPROVED_FOR_SHIPPING_LIB

static int dbgstrlen( const CHAR* str )
{
    const CHAR* strEnd = str;
    while( *strEnd )
        strEnd++;
    return strEnd - str;
}
static inline CHAR dbgtolower( CHAR ch )
{
    if( ch >= 'A' && ch <= 'Z' )
        return ch - ( 'A' - 'a' );
    else
        return ch;
}
static INT dbgstrnicmp( const CHAR* str1, const CHAR* str2, int n )
{
    while( n > 0 )
    {
        if( dbgtolower( *str1 ) != dbgtolower( *str2 ) )
            return *str1 - *str2;
        --n;
        ++str1;
        ++str2;
    }
    return 0;
}
static void dbgstrcpy( CHAR* strDest, const CHAR* strSrc )
{
    while( ( *strDest++ = *strSrc++ ) != 0 );
}

HRESULT __stdcall DebugConsoleCmdProcessor( const CHAR* Command, CHAR* Response, DWORD ResponseLen, PDM_CMDCONT pdmcc )
{
	// Skip over the command prefix and the exclamation mark.
	Command += dbgstrlen("UNREAL") + 1;

	// Check if this is the initial connect signal
	if( dbgstrnicmp( Command, "__connect__", 11 ) == 0 )
	{
		// If so, respond that we're connected
		lstrcpynA( Response, "Connected.", ResponseLen );
		return XBDM_NOERR;
	}

	{
		FScopeLock ScopeLock(&RemoteDebugCriticalSection);
		if( RemoteDebugCommand[0] )
		{
			// This means the application has probably stopped polling for debug commands
			dbgstrcpy( Response, "Cannot execute - previous command still pending." );
		}
		else
		{
			dbgstrcpy( RemoteDebugCommand, Command );
		}
	}

	return XBDM_NOERR;
}

#endif


