/**
 * LaunchPrivate.h:  Xenon version
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#define UTGAME		2
#define EXAMPLEGAME 3
#define GEARGAME	4

#include "Engine.h"
#include "UnIpDrv.h"

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
#else
#elif GAMENAME == BMGAME
#include "BmGameClasses.h"
#include "BmEditorClasses.h"
#else
#error Hook up your game name here
#endif
#include "XeD3DDrv.h"
#include "XeDrv.h"
#include "XeAudioDevice.h"
#endif

#include "FMallocXenon.h"
#include "FMallocProfiler.h"
#include "FMallocProxySimpleTrack.h"
#include "FMallocProxySimpleTag.h"
#include "FMallocThreadSafeProxy.h"
#include "FFeedbackContextAnsi.h"
#include "FFileManagerXenon.h"
#include "FCallbackDevice.h"
#include "FConfigCacheIni.h"
#include "../../Launch/Inc/LaunchEngineLoop.h"
#include "UnThreadingWindows.h"



