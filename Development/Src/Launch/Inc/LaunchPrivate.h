/*=============================================================================
	LaunchPrivate.h: Unreal launcher.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#if _WINDOWS

#define UTGAME		2
#define EXAMPLEGAME 3
#define GEARGAME	4
#define BMGAME	5

//@warning: this needs to be the very first include
#if _WINDOWS
#include "UnrealEd.h"
#endif

#include "Engine.h"
#include "UnIpDrv.h"

#if _WINDOWS
#include "WinDrv.h"
#endif

#if PLATFORM_UNIX
#include "SDLDrv.h"
#endif

// Includes for CIS.
#if CHECK_NATIVE_CLASS_SIZES
#include "EngineMaterialClasses.h"
#include "EnginePhysicsClasses.h"
#include "EngineSequenceClasses.h"
//#include "EngineUserInterfaceClasses.h" // included by Editor.h which is Included by UnrealEd.h above
#include "EngineUIPrivateClasses.h"
#include "EngineUISequenceClasses.h"
#include "EngineSoundClasses.h"
#include "EngineInterpolationClasses.h"
//#include "EngineParticleClasses.h" // included by UnrealEd.h above
#include "EngineAIClasses.h"
#include "EngineAnimClasses.h"
#include "EngineDecalClasses.h"
#include "EngineFogVolumeClasses.h"
#include "EngineMeshClasses.h"
#include "EnginePrefabClasses.h"
//#include "EngineTerrainClasses.h" // included by UnTerrain.h below
#include "EngineFoliageClasses.h"
#include "EngineFluidClasses.h"
#include "EngineSpeedTreeClasses.h"
#include "UnTerrain.h"
#include "UnCodecs.h"
#include "GameFrameworkClasses.h"
#include "GameFrameworkAnimClasses.h"
#include "UnrealScriptTestClasses.h"
#include "UnrealEdPrivateClasses.h"
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
#include "GearEditorClasses.h"
#include "GearGameWeaponClasses.h"
#include "GearGameUIClasses.h"
#include "GearGameUIPrivateClasses.h"
#include "GearGameAnimClasses.h"
#elif GAMENAME == UTGAME
#include "UTGameClasses.h"
#include "UTGameAnimationClasses.h"
#include "UTGameSequenceClasses.h"
#include "UTGameUIClasses.h"
#include "UTGameVehicleClasses.h"
#include "UTGameAIClasses.h"
#include "UTGameOnslaughtClasses.h"
#include "UTGameUIFrontEndClasses.h"
#include "UTEditorClasses.h"
#elif GAMENAME == EXAMPLEGAME
#include "ExampleGameClasses.h"
#include "ExampleEditorClasses.h"
#elif GAMENAME == BMGAME
#include "BmGameClasses.h"
#include "BmEditorClasses.h"
#else
#error Hook up your game name here
#endif
#include "EditorClasses.h"
#include "ALAudio.h"
#include "NullRHI.h"
#endif

#include "FMallocAnsi.h"
#include "FMallocDebug.h"
#include "FMallocProfiler.h"
#include "FMallocProxySimpleTrack.h"
#include "FMallocProxySimpleTag.h"
#include "FMallocThreadSafeProxy.h"
#include "FFeedbackContextAnsi.h"
#include "FCallbackDevice.h"
#include "FConfigCacheIni.h"
#include "LaunchEngineLoop.h"

#if _WINDOWS
#include "FMallocWindows.h"
#include "FFeedbackContextWindows.h"
#include "FFileManagerWindows.h"
#include "UnThreadingWindows.h"
#elif PLATFORM_UNIX
#include "FFileManagerUnix.h"
#include "UnThreadingUnix.h"
#else
#error Please define your platform.
#endif

#endif
