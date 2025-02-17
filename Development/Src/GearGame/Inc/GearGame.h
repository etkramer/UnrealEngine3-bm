//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
// Confidential.
//=============================================================================

#ifndef __GEARGAME_H__
#define __GEARGAME_H__

#include "Engine.h"
#include "EngineAnimClasses.h"
#include "EngineAIClasses.h"
#include "EngineInterpolationClasses.h"
#include "EngineSequenceClasses.h"
#include "EnginePhysicsClasses.h"
#include "EngineParticleClasses.h"
#include "EngineUserInterfaceClasses.h"
#include "EngineFogVolumeClasses.h"

#include "EngineDecalClasses.h"

#include "GameFrameworkClasses.h"
#include "GameFrameworkAnimClasses.h"

#include "GearGameClasses.h"
#include "GearGameAIClasses.h"
#include "GearGamePCClasses.h"
#include "GearGamePawnClasses.h"

#include "GearScreenshotIO.h"

#define AI_PollMoveAlongSpline			600
#define AI_PollAdjustToSlot				601
#define AI_PollPrepareWeaponToFire		602
#define AI_PollFireUntilDone			603

// used for AI initial visibility checks so that we can have special primitives that block only those
const DWORD TRACE_GearAIVisibility = 0x800000;

BEGIN_COMMANDLET(CheckpointEdit, GearGame)
END_COMMANDLET

// simple macro to toggle stats logging
#if 0
	#define DEBUGSTAT			debugf
#else
	#define DEBUGSTAT(...)
#endif

#if XBOX && (!FINAL_RELEASE || FINAL_RELEASE_DEBUGCONSOLE)

#define _MGSTEST_ 1

#ifndef _VINCE_
#define _VINCE_ 0
#endif

#define _TICKET_TRACKER_ 1
#define _XCR_ 1

#define _TNT_ 1
#define _XENON_UTILITY_ 1

#endif // XBOX && !FINAL_RELEASE  && WITH_MGS_EXTERNAL_LIBS

#endif
