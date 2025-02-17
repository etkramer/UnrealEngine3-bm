/*=============================================================================
	Core.cpp: Unreal core.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"

/*-----------------------------------------------------------------------------
	Temporary startup objects.
-----------------------------------------------------------------------------*/

// Error file manager.
class FFileManagerError : public FFileManager
{
public:
	FArchive* CreateFileReader( const TCHAR* Filename, DWORD Flags, FOutputDevice* Error )
		{appErrorf(TEXT("Called FFileManagerError::CreateFileReader")); return 0;}
	FArchive* CreateFileWriter( const TCHAR* Filename, DWORD Flags, FOutputDevice* Error, INT MaxFileSize )
		{appErrorf(TEXT("Called FFileManagerError::CreateFileWriter")); return 0;}
	INT UncompressedFileSize( const TCHAR* Filename )
		{return -1;}
	INT GetFileStartSector( const TCHAR* Filename )
		{return -1;}
	UBOOL Delete( const TCHAR* Filename, UBOOL RequireExists=0, UBOOL EvenReadOnly=0 )
		{return FALSE;}
	UBOOL IsReadOnly( const TCHAR* Filename )
		{return FALSE;}
	DWORD Copy( const TCHAR* Dest, const TCHAR* Src, UBOOL Replace=1, UBOOL EvenIfReadOnly=0, UBOOL Attributes=0, FCopyProgress* Progress=NULL )
		{return COPY_MiscFail;}
	UBOOL Move( const TCHAR* Dest, const TCHAR* Src, UBOOL Replace=1, UBOOL EvenIfReadOnly=0, UBOOL Attributes=0 )
		{return FALSE;}
	INT FindAvailableFilename( const TCHAR* Base, const TCHAR* Extension, FString& OutFilename, INT StartVal=-1 )
		{return -1;}
	UBOOL MakeDirectory( const TCHAR* Path, UBOOL Tree=0 )
		{return FALSE;}
	UBOOL DeleteDirectory( const TCHAR* Path, UBOOL RequireExists=0, UBOOL Tree=0 )
		{return FALSE;}
	void FindFiles( TArray<FString>& Result, const TCHAR* Filename, UBOOL Files, UBOOL Directories )
		{}
	DOUBLE GetFileAgeSeconds( const TCHAR* Filename )
		{return -1.0;}
	DOUBLE GetFileTimestamp( const TCHAR* Filename )
		{return -1.0;}
	UBOOL SetDefaultDirectory()
		{return FALSE;}
	FString GetCurrentDirectory()
		{return TEXT("");}
	UBOOL GetTimestamp( const TCHAR* Filename, timestamp& Timestamp )
		{ return FALSE; }
	UBOOL TouchFile(const TCHAR* Filename)
		{ return FALSE; }
	FFileHandle FileOpen( const TCHAR* Filename, DWORD Flags )
		{ return FFileHandle(); }
	void FileClose( FFileHandle FileHandle )
		{ }
	INT FileSeek( FFileHandle FileHandle, INT Offset, EFileSeekFlags Base=IO_SEEK_BEGIN )
		{ return INDEX_NONE; }
	INT GetFilePosition( FFileHandle FileHandle )
		{ return INDEX_NONE; }
	INT FileRead( FFileHandle FileHandle, void* Buffer, INT Size )
		{ return INDEX_NONE; }
	INT FileWrite( FFileHandle FileHandle, const void* Buffer, INT Size )
		{ return INDEX_NONE; }
	INT FileTruncate( const TCHAR* Filename, INT FileSize )
		{ return INDEX_NONE; }
	INT FileSize( const TCHAR* Filename )
		{ return INDEX_NONE; }
	UBOOL SetFileTimestamp( const TCHAR* Filename, DOUBLE TimeStamp )
		{ return FALSE; }
	void FileFlush( FFileHandle FileHandle )
		{ }

protected:
	virtual INT InternalFileSize(const TCHAR* Filename)
		{ return -1; }
} FileError;

// Exception thrower.
class FThrowOut : public FOutputDevice
{
public:
	void Serialize( const TCHAR* V, EName Event )
	{
#if EXCEPTIONS_DISABLED
		appDebugBreak();
#else
		throw( V );
#endif
	}
} ThrowOut;

// Null output device.
class FNullOut : public FOutputDevice
{
public:
	void Serialize( const TCHAR* V, enum EName Event )
	{}
} NullOut;

// Dummy saver.
class FArchiveDummySave : public FArchive
{
public:
	FArchiveDummySave() { ArIsSaving = 1; }
} GArchiveDummySave;

/** Global output device redirector, can be static as FOutputDeviceRedirector explicitly empties TArray on TearDown */
static FOutputDeviceRedirector LogRedirector;

FString FFileManager::ConvertToAbsolutePath( const TCHAR* Filename )
{
	appErrorf(TEXT("Currently not implemented for this platform."));
	return FString();
}

FString FFileManager::ConvertAbsolutePathToUserPath(const TCHAR* AbsolutePath)
{
	appErrorf(TEXT("Currently not implemented for this platform."));
	return FString();
}

/**
 *	Threadsafely converts the platform-independent Unreal filename into platform-specific full path.
 *
 *	@param	Platform-independent Unreal filename
 *	@return	Platform-dependent full filepath
 **/
FString FFileManager::GetPlatformFilepath( const TCHAR* Filename )
{
	return FString(Filename);
}

DWORD FObjectPropagator::Paused = 0;
FObjectPropagator		NullPropagator; // a Null propagator to suck up unwanted propagations
void FObjectPropagator::SetPropagator(FObjectPropagator* InPropagator)
{
	// if we aren't passing one in for real, just clear it out
	if (!InPropagator)
	{
		ClearPropagator();
		return;
	}

	// disconnect the existing propagator
	GObjectPropagator->Disconnect();

	// let it connect if it needs
	if (!InPropagator->Connect())
	{
		// if it failed to connect, we have no propagator
		GObjectPropagator = &NullPropagator;
	}
	else
	{
		// set the global propagator to the new propagator
		GObjectPropagator = InPropagator;
	}

}
void FObjectPropagator::ClearPropagator()
{
	// disconnect the existing propagator
	GObjectPropagator->Disconnect();

	// set the propagator to the empty propagator that does nothing
	GObjectPropagator = &NullPropagator;
}

void FObjectPropagator::Pause()
{
	// we use a Puased stack for nested Pause/UnPause calls
	Paused++;
}

void FObjectPropagator::Unpause()
{
	// don't let us Unpause too many times
	if (Paused)
	{
		Paused--;
	}
}

void FNotifyHook::NotifyPreChange( void* Src, FEditPropertyChain* PropertyAboutToChange )
{
	NotifyPreChange( Src, PropertyAboutToChange != NULL && PropertyAboutToChange->Num() > 0 ? PropertyAboutToChange->GetActiveNode()->GetValue() : NULL );
}

void FNotifyHook::NotifyPostChange( void* Src, FEditPropertyChain* PropertyThatChanged )
{
	NotifyPostChange( Src, PropertyThatChanged != NULL && PropertyThatChanged->Num() > 0 ? PropertyThatChanged->GetActiveNode()->GetValue() : NULL );
}


/**
* FFileManger::timestamp implementation
*/
INT FFileManager::timestamp::GetJulian( void ) const
{
	return  Day - 32075L +
		1461L*(Year  + 4800L +  (Month  - 14L)/12L)/4L      +
		367L*(Month - 2L     - ((Month - 14L)/12L)*12L)/12L -
		3L*((Year + 4900L    +  (Month  - 14L)/12L)/100L)/4L;
}


/*
* @return Seconds of the day so far
*/
INT FFileManager::timestamp::GetSecondOfDay( void ) const
{
	return Second + Minute*60 + Hour*60*60;
}


/*
* @return Whether this time stamp is older than Other
*/
UBOOL FFileManager::timestamp::operator< ( FFileManager::timestamp& Other ) const
{
	const INT J = GetJulian();
	const INT OJ = Other.GetJulian();

	if (J<OJ)
	{
		return TRUE;
	}
	else if (J>OJ)
	{
		return FALSE;
	}
	else
	{
		// Have to compare times
		if (GetSecondOfDay() < Other.GetSecondOfDay())
		{
			return TRUE;
		}
	}

	return FALSE;
}

/*
* @return Whether this time stamp is newer than Other
*/
UBOOL FFileManager::timestamp::operator> ( FFileManager::timestamp& Other ) const
{
	const INT J = GetJulian();
	const INT OJ = Other.GetJulian();

	if (J>OJ)
	{
		return TRUE;
	}
	else if (J<OJ)
	{
		return FALSE;
	}
	else
	{
		// Have to compare times
		if (GetSecondOfDay() > Other.GetSecondOfDay())
		{
			return TRUE;
		}
	}

	return FALSE;
}

/*
* @return Whether this time stamp is equal to Other
*/
UBOOL FFileManager::timestamp::operator==( FFileManager::timestamp& Other ) const
{
	return ((Year  ==Other.Year  ) &&
		(Day   ==Other.Day   ) &&
		(Month ==Other.Month ) &&
		(Hour  ==Other.Hour  ) &&
		(Minute==Other.Minute) &&
		(Second==Other.Second));
}

/*
* @return Whether this time stamp is above or equal to Other
*/
UBOOL FFileManager::timestamp::operator>=( FFileManager::timestamp& Other ) const
{
	if (operator ==(Other))
	{
		return TRUE;
	}

	return operator >(Other);
}

/*
* @return Whether this time stamp is below or equal to Other
*/
UBOOL FFileManager::timestamp::operator<=( FFileManager::timestamp& Other ) const
{
	if (operator ==(Other))
	{
		return TRUE;
	}

	return operator <(Other);
}

/*-----------------------------------------------------------------------------
	FSelfRegisteringExec implementation.
-----------------------------------------------------------------------------*/

/** Constructor, registering this instance. */
FSelfRegisteringExec::FSelfRegisteringExec()
{
	RegisteredExecs.AddItem( this );
}

/** Destructor, unregistering this instance. */
FSelfRegisteringExec::~FSelfRegisteringExec()
{
	verify( RegisteredExecs.RemoveItem( this ) == 1 );
}

TArray<FSelfRegisteringExec*> FSelfRegisteringExec::RegisteredExecs;

/*-----------------------------------------------------------------------------
	Global variables.
-----------------------------------------------------------------------------*/

FMemStack				GMainThreadMemStack(65536);									/* Global memory stack */
FOutputDeviceRedirectorBase* GLog						= &LogRedirector;			/* Regular logging */
FOutputDeviceError*		GError							= NULL;						/* Critical errors */
FOutputDevice*			GNull							= &NullOut;					/* Log to nowhere */
FOutputDevice*			GThrow							= &ThrowOut;				/* Exception thrower */
FFeedbackContext*		GWarn							= NULL;						/* User interaction and non critical warnings */
FConfigCacheIni*		GConfig							= NULL;						/* Configuration database cache */
FTransactionBase*		GUndo							= NULL;						/* Transaction tracker, non-NULL when a transaction is in progress */
FOutputDeviceConsole*	GLogConsole						= NULL;						/* Console log hook */
FMalloc*				GMalloc							= NULL;						/* Memory allocator */
FFileManager*			GFileManager					= NULL;						/* File manager */
FCallbackEventObserver*	GCallbackEvent					= NULL;						/* Used for making event callbacks into UnrealEd */
FCallbackQueryDevice*	GCallbackQuery					= NULL;						/* Used for making queries into UnrealEd */
USystem*				GSys							= NULL;						/* System control code */
UProperty*				GProperty						= NULL;						/* Property for UnrealScript interpretter */
/** Points to the UProperty currently being serialized														*/
UProperty*				GSerializedProperty				= NULL;
BYTE*					GPropAddr						= NULL;						/* Property address for UnrealScript interpreter */
UObject*				GPropObject						= NULL;						/* Object with Property for UnrealScript interpreter */
DWORD					GRuntimeUCFlags					= 0;						/* Property for storing flags between calls to bytecode functions */
class UPropertyWindowManager*	GPropertyWindowManager	= NULL;						/* Manages and tracks property editing windows */
TCHAR					GErrorHist[16384]				= TEXT("");					/* For building call stack text dump in guard/unguard mechanism */
TCHAR					GYes[64]						= TEXT("Yes");				/* Localized "yes" text */
TCHAR					GNo[64]							= TEXT("No");				/* Localized "no" text */
TCHAR					GTrue[64]						= TEXT("True");				/* Localized "true" text */
TCHAR					GFalse[64]						= TEXT("False");			/* Localized "false" text */
TCHAR					GNone[64]						= TEXT("None");				/* Localized "none" text */
DOUBLE					GSecondsPerCycle				= 0.0;						/* Seconds per CPU cycle for this PC */
DWORD					GUglyHackFlags					= 0;						/* Flags for passing around globally hacked stuff */
#if !CONSOLE
UBOOL					GIsEditor						= FALSE;					/* Whether engine was launched for editing */
UBOOL					GIsImportingT3D					= FALSE;					/* Whether editor is importing T3D */
UBOOL					GIsUCC							= FALSE;					/* Is UCC running? */
UBOOL					GIsUCCMake						= FALSE;					/* Are we rebuilding script via ucc make? */
#endif // CONSOLE
UBOOL					GEdSelectionLock				= FALSE;					/* Are selections locked? (you can't select/deselect additional actors) */
UBOOL					GIsClient						= FALSE;					/* Whether engine was launched as a client */
UBOOL					GIsServer						= FALSE;					/* Whether engine was launched as a server, true if GIsClient */
UBOOL					GIsCriticalError				= FALSE;					/* An appError() has occured */
UBOOL					GIsStarted						= FALSE;					/* Whether execution is happening from within main()/WinMain() */
UBOOL					GIsGuarded						= FALSE;					/* Whether execution is happening within main()/WinMain()'s try/catch handler */
UBOOL					GIsRunning						= FALSE;					/* Whether execution is happening within MainLoop() */
UBOOL					GIsGarbageCollecting			= FALSE;					/* Whether we are inside garbage collection */
UBOOL					GIsReplacingObject				= FALSE;					/* Whether we are currently in-place replacing an object */
/* On the xbox setting thread names messes up the XDK COM API that UnrealConsole uses. Have them off by default. */
#if XBOX
UBOOL					GSetThreadNames					= FALSE;
#endif
/** This determines if we should pop up any dialogs.  If Yes then no popping up dialogs.					*/
UBOOL					GIsUnattended					= FALSE;
/** This determines if we should output any log text.  If Yes then no log text should be emitted.			*/
UBOOL					GIsSilent						= FALSE;
/**
 * Used by non-UObject ctors/dtors of UObjects with multiple inheritance to
 * determine whether we're constructing/destructing the class default object
 */
UBOOL					GIsAffectingClassDefaultObject	= FALSE;
UBOOL					GIsSlowTask						= FALSE;					/* Whether there is a slow task in progress */
UBOOL					GIsRequestingExit				= FALSE;					/* Indicates that MainLoop() should be exited at the end of the current iteration */
FGlobalMath				GMath;														/* Math code */
FArchive*				GDummySave						= &GArchiveDummySave;		/* No-op save archive */
/** Archive for serializing arbitrary data to and from memory												*/
FReloadObjectArc*		GMemoryArchive					= NULL;
TArray<FEdLoadError>	GEdLoadErrors;												/* For keeping track of load errors in the editor */
UDebugger*				GDebugger						= NULL;						/* Unrealscript Debugger */
UBOOL					GIsBenchmarking					= 0;						/* Whether we are in benchmark mode or not */
UBOOL					GIsDumpingMovie					= 0;						/* Whether we are dumping screenshots */
UBOOL					GIsDumpingTileShotMovie			= 0;						/* Whether we are dumping tiledshot screenshots */
UBOOL					GForceLogFlush					= 0;						/* Whether to force output after each line written to the log */
UBOOL					GForceSoundRecook				= 0;						/* Whether to force a recook of all loaded sounds */
QWORD					GMakeCacheIDIndex				= 0;						/* Cache ID */
TCHAR					GConfigSubDirectory[1024]		= TEXT("");					/* optional subdirectory off default config dir for loading/saving .ini files */
TCHAR					GEngineIni[1024]				= TEXT("");					/* Engine ini file */
TCHAR					GEditorIni[1024]				= TEXT("");					/* Editor ini file */
TCHAR					GEditorUserSettingsIni[1024]	= TEXT("");					/* Editor User Settings ini file */
TCHAR					GCompatIni[1024]				= TEXT("");
TCHAR					GInputIni[1024]					= TEXT("");					/* Input ini file */
TCHAR					GGameIni[1024]					= TEXT("");					/* Game ini file */
TCHAR					GUIIni[1024]					= TEXT("");
TCHAR					GDefaultEngineIni[1024]			= TEXT("");					/* Default Engine ini file */
TCHAR					GDefaultEditorIni[1024]			= TEXT("");					/* Default Editor ini file */
TCHAR					GDefaultEditorUserSettingsIni[1024]	= TEXT("");				/* Default Editor User Settings ini file */
TCHAR					GDefaultCompatIni[1024]			= TEXT("");
TCHAR					GDefaultInputIni[1024]			= TEXT("");					/* Default Input ini file */
TCHAR					GDefaultGameIni[1024]			= TEXT("");					/* Default Game ini file */
TCHAR					GDefaultUIIni[1024]				= TEXT("");
FLOAT					NEAR_CLIPPING_PLANE				= 10.0f;					/* Near clipping plane */
/** Timestep if a fixed delta time is wanted.																*/
DOUBLE					GFixedDeltaTime					= 1 / 30.f;
/** Current delta time in seconds.																			*/
DOUBLE					GDeltaTime						= 1 / 30.f;
DOUBLE					GCurrentTime					= 0;						/* Current time */
INT						GSRandSeed						= 0;						/* Seed for appSRand */
UBOOL					GExitPurge						= FALSE;
/** Game name, used for base game directory and ini among other things										*/
TCHAR					GGameName[64]					= TEXT("Example");
/** The current SentinelRunID                                                                                  */
INT                     GSentinelRunID                  = -1;
//@{
//@script patcher
/** whether we're currently generating a script patch								*/
UBOOL					GIsScriptPatcherActive			= FALSE;
/** suffix to append to top-level package names when running the script patcher		*/
FString					GScriptPatchPackageSuffix		= TEXT("");
//@}
/** Exec handler for game debugging tool, allowing commands like "editactor", ...							*/
FExec*					GDebugToolExec;
/** Whether we are currently cooking.																		*/
UBOOL					GIsCooking						= FALSE;
/** Whether we are currently cooking for a demo build.														*/
UBOOL					GIsCookingForDemo				= FALSE;
/** Which platform we're cooking for, PLATFORM_Unknown (0) if not cooking.									*/
UE3::EPlatformType		GCookingTarget					= UE3::PLATFORM_Unknown;
/** Whether we're currently in the async loading codepath or not											*/
UBOOL					GIsAsyncLoading					= FALSE;
/** The global object property propagator,with a Null version so nothing crashes							*/
FObjectPropagator*		GObjectPropagator				= &NullPropagator;
/** Whether to allow execution of Epic internal code like e.g. TTP integration, ...							*/
UBOOL					GIsEpicInternal					= FALSE;
/** Whether GWorld points to the play in editor world														*/
UBOOL					GIsPlayInEditorWorld			= FALSE;
/** TRUE if a normal or PIE game is active (basically !GIsEditor || GIsPlayInEditorWorld)					*/
UBOOL					GIsGame							= FALSE;
/** TRUE if the runtime needs textures to be powers of two													*/
UBOOL					GPlatformNeedsPowerOfTwoTextures = FALSE;
/** TRUE if we're associating a level with the world, FALSE otherwise										*/
UBOOL					GIsAssociatingLevel				= FALSE;
/** Global IO manager																						*/
FIOManager*				GIOManager						= NULL;
/** Time at which appSeconds() was first initialized (very early on)										*/
DOUBLE					GStartTime;
/** Whether to use the seekfree package map over the regular linker based one.								*/
UBOOL					GUseSeekFreePackageMap			= FALSE;
/** Whether we are currently precaching or not.																*/
UBOOL					GIsPrecaching					= FALSE;
/** Whether we are still in the initial loading proces.														*/
UBOOL					GIsInitialLoad					= TRUE;
/** Whether we are currently purging an object in the GC purge pass.										*/
UBOOL					GIsPurgingObject				= FALSE;
/** TRUE when we are routing ConditionalPostLoad/PostLoad to objects										*/
UBOOL					GIsRoutingPostLoad				= FALSE;
/** Steadily increasing frame counter.																		*/
QWORD					GFrameCounter;
#if !SHIPPING_PC_GAME
// We cannot count on this variable to be accurate in a shipped game, so make sure no code tries to use it
/** Whether we are the first instance of the game running.													*/
UBOOL					GIsFirstInstance				= TRUE;
#endif
/** Whether to always use the compression resulting in the smallest size.									*/
UBOOL					GAlwaysBiasCompressionForSize	= FALSE;
/** The number of full speed hardware threads the system has. */
UINT					GNumHardwareThreads				= 1;
/** The number of unused threads to have for SerializeCompressed tasks										*/
UINT					GNumUnusedThreads_SerializeCompressed = 1;
/** This flag signals that the rendering code should throw up an appropriate error.							*/
UBOOL					GHandleDirtyDiscError			= FALSE;
/** Whether to forcefully enable capturing of scoped script stats (if > 0).									*/
INT						GForceEnableScopedCycleStats	= 0;
/** Size to break up data into when saving compressed data													*/
INT						GSavingCompressionChunkSize		= SAVING_COMPRESSION_CHUNK_SIZE;
/** Total amount of calls to appSeconds and appCycles.														*/
QWORD					GNumTimingCodeCalls				= 0;
#if !CONSOLE
/** Whether we are using the seekfree/ cooked loading codepath.												*/
UBOOL					GUseSeekFreeLoading				= FALSE;
#endif
/** Whether we are using full localization files or not.													*/
UBOOL					GUseFullLocalizationFiles		= FALSE;
/** Thread ID of the main/game thread																		*/
DWORD					GGameThreadId					= 0;
/** Has GGameThreadId been set yet?																			*/
UBOOL					GIsGameThreadIdInitialized		= FALSE;
#if ENABLE_SCRIPT_TRACING
/** Whether we are tracing script to a log file */
UBOOL					GIsUTracing						= FALSE;
#endif
/** Helper function to flush resource streaming.															*/
void					(*GFlushStreamingFunc)(void)	= NULL;
#if TRACK_SERIALIZATION_PERFORMANCE || LOOKING_FOR_PERF_ISSUES
/** Tracks time spent serializing UObject data, per object type												*/
FStructEventMap*		GObjectSerializationPerfTracker	= NULL;
/** Tracks the amount of time spent in UClass serialization, per class										*/
FStructEventMap*		GClassSerializationPerfTracker	= NULL;
#endif
/** Whether to emit begin/ end draw events.																	*/
UBOOL					GEmitDrawEvents					= FALSE;
/** Whether we are using software rendering or not.															*/
UBOOL					GUseSoftwareRendering			= FALSE;
/** Whether COM is already initialized or not.																*/
UBOOL					GIsCOMInitialized				= FALSE;
/** Whether we want the rendering thread to be suspended, used e.g. for tracing.							*/
UBOOL					GShouldSuspendRenderingThread	= FALSE;
/** Whether we want the game thread to be suspended, used e.g. for tracing.									*/
UBOOL					GShouldSuspendGameThread		= FALSE;
/** Whether we want to use a fixed time step or not.														*/
UBOOL					GUseFixedTimeStep				= FALSE;
/** Determines what kind of trace should occur, NAME_None for none.											*/
FName					GCurrentTraceName				= NAME_None;
/** whether to print time since GStartTime in log output													*/
UBOOL					GPrintLogTimes					= FALSE;		
/** This is the global screen shot index.  Which is a way to make it so we don't have overwriting ScreenShots  **/
INT                     GSceenShotBitmapIndex           = -1;

/** Total number of calls to Malloc, if implemented by derived class. */
QWORD FMalloc::TotalMallocCalls;
/** Total number of calls to Free, if implemented by derived class. */
QWORD FMalloc::TotalFreeCalls;
/** Total number of calls to Realloc, if implemented by derived class. */
QWORD FMalloc::TotalReallocCalls;
/** Total number of calls to PhysicalAlloc, if implemented by derived class. */
QWORD FMalloc::TotalPhysicalAllocCalls;
/** Total number of calls to PhysicalFree, if implemented by derived class. */
QWORD FMalloc::TotalPhysicalFreeCalls;


/**
 * Object stats declarations
 */
DECLARE_STATS_GROUP(TEXT("Object"),STATGROUP_Object);

/** Memory stats objects 
 *
 * If you add new Stat Memory stats please update:  FMemoryChartEntry so the auotmated memory chart has the info
 */

DECLARE_STATS_GROUP(TEXT("Memory"),STATGROUP_Memory);
DECLARE_MEMORY_STAT(TEXT("Physical Memory Used"),STAT_PhysicalAllocSize,STATGROUP_Memory);
#if !PS3 // PS3 doesn't have a need for virtual memory stat
DECLARE_MEMORY_STAT(TEXT("Virtual Memory Used"),STAT_VirtualAllocSize,STATGROUP_Memory);
#endif
DECLARE_MEMORY_STAT(TEXT("Audio Memory Used"),STAT_AudioMemory,STATGROUP_Memory);
DECLARE_MEMORY_STAT(TEXT("Texture Memory Used"),STAT_TextureMemory,STATGROUP_Memory);
#if _WINDOWS // The TextureMemory stat will be what is used on Xbox...
DECLARE_MEMORY_STAT(TEXT("360 Texture Memory Used"),STAT_360TextureMemory,STATGROUP_Memory);
#endif
DECLARE_MEMORY_STAT(TEXT("Novodex Allocation Size"),STAT_MemoryNovodexTotalAllocationSize,STATGROUP_Memory);
DECLARE_MEMORY_STAT(TEXT("Animation Memory"),STAT_AnimationMemory,STATGROUP_Memory);
DECLARE_MEMORY_STAT(TEXT("Vertex Lighting Memory"),STAT_VertexLightingMemory,STATGROUP_Memory);
DECLARE_MEMORY_STAT(TEXT("StaticMesh Vertex Memory"),STAT_StaticMeshVertexMemory,STATGROUP_Memory);
DECLARE_MEMORY_STAT(TEXT("StaticMesh Index Memory"),STAT_StaticMeshIndexMemory,STATGROUP_Memory);
DECLARE_MEMORY_STAT(TEXT("FracturedMesh Index Memory"),STAT_FracturedMeshIndexMemory,STATGROUP_Memory);
DECLARE_MEMORY_STAT(TEXT("SkeletalMesh Vertex Memory"),STAT_SkeletalMeshVertexMemory,STATGROUP_Memory);
DECLARE_MEMORY_STAT(TEXT("SkeletalMesh Index Memory"),STAT_SkeletalMeshIndexMemory,STATGROUP_Memory);
DECLARE_MEMORY_STAT(TEXT("Decal Vertex Memory"),STAT_DecalVertexMemory,STATGROUP_Memory);
DECLARE_MEMORY_STAT(TEXT("Decal Index Memory"),STAT_DecalIndexMemory,STATGROUP_Memory);
DECLARE_MEMORY_STAT(TEXT("Decal Interaction Memory"),STAT_DecalInteractionMemory,STATGROUP_Memory);
DECLARE_MEMORY_STAT2(TEXT("VertexShader Memory"),STAT_VertexShaderMemory,STATGROUP_Memory, MCR_Physical, FALSE);
DECLARE_MEMORY_STAT2(TEXT("PixelShader Memory"),STAT_PixelShaderMemory,STATGROUP_Memory, MCR_Physical, FALSE);
DECLARE_CYCLE_STAT(TEXT("TrimMemory cycles"),STAT_TrimMemoryTime,STATGROUP_Memory);

DECLARE_STATS_GROUP(TEXT("MemoryChurn"),STATGROUP_MemoryChurn);
DECLARE_DWORD_COUNTER_STAT(TEXT("Malloc calls"),STAT_MallocCalls,STATGROUP_MemoryChurn);
DECLARE_DWORD_COUNTER_STAT(TEXT("Realloc calls"),STAT_ReallocCalls,STATGROUP_MemoryChurn);
DECLARE_DWORD_COUNTER_STAT(TEXT("Free calls"),STAT_FreeCalls,STATGROUP_MemoryChurn);
DECLARE_DWORD_COUNTER_STAT(TEXT("PhysicalAlloc calls"),STAT_PhysicalAllocCalls,STATGROUP_MemoryChurn);
DECLARE_DWORD_COUNTER_STAT(TEXT("PhysicalFree calls"),STAT_PhysicalFreeCalls,STATGROUP_MemoryChurn);
DECLARE_DWORD_COUNTER_STAT(TEXT("Total Allocator calls"),STAT_TotalAllocatorCalls,STATGROUP_MemoryChurn);

/** Threading stats objects */
DECLARE_STATS_GROUP(TEXT("Threading"),STATGROUP_Threading);
DECLARE_CYCLE_STAT(TEXT("Rendering thread idle time"),STAT_RenderingIdleTime,STATGROUP_Threading);
DECLARE_CYCLE_STAT(TEXT("Rendering thread busy time"),STAT_RenderingBusyTime,STATGROUP_Threading);
DECLARE_CYCLE_STAT(TEXT("Game thread idle time"),STAT_GameIdleTime,STATGROUP_Threading);
DECLARE_CYCLE_STAT(TEXT("Game thread tick wait time"),STAT_GameTickWaitTime,STATGROUP_Threading);
DECLARE_FLOAT_COUNTER_STAT(TEXT("Game thread requested wait time"),STAT_GameTickWantedWaitTime,STATGROUP_Threading);
DECLARE_FLOAT_COUNTER_STAT(TEXT("Game thread additional wait time"),STAT_GameTickAdditionalWaitTime,STATGROUP_Threading);
#if XBOX
DECLARE_FLOAT_COUNTER_STAT(TEXT("GPU waiting on CPU time"),STAT_GPUWaitingOnCPU,STATGROUP_Threading);
DECLARE_FLOAT_COUNTER_STAT(TEXT("CPU waiting on GPU time"),STAT_CPUWaitingOnGPU,STATGROUP_Threading);
#endif

#if !PS3
/** Stats notify providers need to be here so the linker doesn't strip them */
#include "UnStatsNotifyProviders.h"

/** List of provider factories */
DECLARE_STAT_NOTIFY_PROVIDER_FACTORY(
	FStatNotifyProvider_BinaryFileFactory,
	FStatNotifyProvider_BinaryFile,
	BinaryFileProvider );
DECLARE_STAT_NOTIFY_PROVIDER_FACTORY(FStatNotifyProvider_XMLFactory,
	FStatNotifyProvider_XML,XmlProvider);
DECLARE_STAT_NOTIFY_PROVIDER_FACTORY(FStatNotifyProvider_CSVFactory,
	FStatNotifyProvider_CSV,CsvProvider);

#if XBOX
#include "UnStatsNotifyProvidersXe.h"
/** The PIX notifier factory. Declared here because the linker optimizes it out if it's in the Xe file */
DECLARE_STAT_NOTIFY_PROVIDER_FACTORY(FStatNotifyProvider_PIXFactory,
	FStatNotifyProvider_PIX,PixProvider);
#endif
#endif


