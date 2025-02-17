/*=============================================================================
	UnBuild.h: Unreal build settings.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _HEADER_UNBUILD_H_
#define _HEADER_UNBUILD_H_

// Include defining EPIC_INTERNAL. We use P4 permissions to mask out our version which has it set to 1.
#include "BaseInclude.h"

#if FINAL_RELEASE_DEBUGCONSOLE
    #define ALLOW_TRACEDUMP		XBOX
    #define DO_CHECK			0
    #define STATS				0
    #define ALLOW_NON_APPROVED_FOR_SHIPPING_LIB 1  // this is to allow us to link against libs that we can no ship with (e.g. TCR unapproved libs)
#elif FINAL_RELEASE
    #define ALLOW_TRACEDUMP		0
	#define DO_CHECK			0
	#define STATS				0
    #define ALLOW_NON_APPROVED_FOR_SHIPPING_LIB 0  // this is to allow us to link against libs that we can no ship with (e.g. TCR unapproved libs)
#else
	#define ALLOW_TRACEDUMP		XBOX
	#define DO_CHECK			1
	#define STATS				1
    #define ALLOW_NON_APPROVED_FOR_SHIPPING_LIB 1  // this is to allow us to link against libs that we can no ship with (e.g. TCR unapproved libs)
#endif

/**
 * Whether we are in "ship" mode on the PC and therefore want to disable certain debug functionality.
 */
#ifndef SHIPPING_PC_GAME
#define SHIPPING_PC_GAME 0
#endif

/**
 * Whether the game is a demo version and therefore e.g. want to disable auto download and such.
 */
#ifndef DEMOVERSION
#define DEMOVERSION	0
#endif

/**
 * Controls whether a native UClass declaration is generated for native interface classes.  if false, only the non-UObject derived interface class will be exported to header
 */
#ifndef EXPORT_NATIVEINTERFACE_UCLASS
#define EXPORT_NATIVEINTERFACE_UCLASS 1
#endif

/**
 * DO_GUARD_SLOW is enabled for debug builds which enables checkSlow, appErrorfSlow, ...
 **/
#ifdef _DEBUG
#define DO_GUARD_SLOW	1
#else
#define DO_GUARD_SLOW	0
#endif

/**
 * DO_GUARD_SLOWISH is enabled when you want to do minimal checks on certain platforms
 * - currently disabled in release/final on consoles, and enabled on PC except for FINAL_RELEASE
 **/
#if (!CONSOLE && !FINAL_RELEASE) || defined(_DEBUG)
#define DO_GUARD_SLOWISH	1
#else
#define DO_GUARD_SLOWISH	0
#endif

/**
 * Whether to enable debugfSlow
 */
#ifndef DO_LOG_SLOW
#define DO_LOG_SLOW		0
#endif

/**
 * Checks to see if pure virtual has actually been implemented
 *
 * @see Core.h
 * @see UnObjBas.h
 **/
#ifndef CHECK_PUREVIRTUALS
#define CHECK_PUREVIRTUALS 0
#endif


/**
 * Checks for native class script/ C++ mismatch of class size and member variable
 * offset.
 * 
 * @see CheckNativeClassSizes()
 **/
#ifndef CHECK_NATIVE_CLASS_SIZES
#define CHECK_NATIVE_CLASS_SIZES 0
#endif


/**
 * Warn if native function doesn't actually exist.
 *
 * @see CompileFunctionDeclaration
 **/
#ifndef CHECK_NATIVE_MATCH
#define CHECK_NATIVE_MATCH 0
#endif


/**
 * Whether to use the PhysX physics engine.
 * NB: Unreal simplified collision relies on PhysX to pre-process convex hulls to generate face/edge data.
 **/
#ifndef WITH_NOVODEX
#define WITH_NOVODEX 1
#endif


/**
 * Whether to tie Ageia's Performance Monitor into Unreal's timers.
 * AGPerfmon is only supported on Win32 platforms.
 **/
#ifndef WIN32
#ifdef USE_AG_PERFMON
#undef USE_AG_PERFMON
#endif
#endif

#ifndef USE_AG_PERFMON
#define USE_AG_PERFMON 0
#endif


// to turn off novodex we just set WITH_NOVODEX to 0
#if !WITH_NOVODEX && !_XBOX  // this is turned off by novodex elsewhere on xbox
#define NX_DISABLE_FLUIDS 1
#endif


/**
 * Whether to use the FaceFX Facial animation. engine.
 *
 * to compile without FaceFX by doing the following:
 *    #define WITH_FACEFX 0
 *    remove/rename the FaceFX directory ( Development\External\FaceFX )
 *
 **/
#ifndef WITH_FACEFX
#define WITH_FACEFX 1
#endif

/** 
 * Whether to include the Fonix speech recognition library
 */
#ifndef WITH_SPEECH_RECOGNITION
#define WITH_SPEECH_RECOGNITION 0
#endif

/** 
 * Whether to include support for Fonix's TTS
 */
#ifndef WITH_TTS
#define WITH_TTS 0
#endif

/** 
 * Whether to include support for IME
 */
#ifndef WITH_IME
#define WITH_IME 1
#endif

/**
 * Whether to use Collada.
 **/
#ifndef WITH_COLLADA
#define WITH_COLLADA 1
#endif

/** Whether to use the null RHI. */
#ifndef USE_NULL_RHI
#define USE_NULL_RHI 0
#endif

/**
 * Whether to use the Bink codec
 **/
#ifndef USE_BINK_CODEC
#define USE_BINK_CODEC EPIC_INTERNAL
#endif
#if PS3 && USE_NULL_RHI
	#undef USE_BINK_CODEC
	#define USE_BINK_CODEC 0
#endif

/**
 * Whether to use SpeedTree
 */
#ifndef WITH_SPEEDTREE
	#define WITH_SPEEDTREE EPIC_INTERNAL
	#define WITH_SPEEDTREE_EXPORT EPIC_INTERNAL
	#define WITH_SPEEDTREE_MANGLE EPIC_INTERNAL
	// If WITH_SPEEDTREE_MANGLE is 1, you must define a project-specific SPEEDTREE_MANGLE_KEY in BaseInclude.h:
	// #define SPEEDTREE_MANGLE_KEY TEXT("")
#endif

/**
 * Whether to compile in support for database connectivity and SQL execution.
 */
#ifndef WITH_DATABASE_SUPPORT
	#define WITH_DATABASE_SUPPORT (EPIC_INTERNAL && (!FINAL_RELEASE || FINAL_RELEASE_DEBUGCONSOLE))
#endif

/** One stop and shop for turning on/off the mgs libs (tnt, tickettracker, vince etc.) **/
#ifndef WITH_MGS_EXTERNAL_LIBS
#define WITH_MGS_EXTERNAL_LIBS 0
#endif

/**
 * This is a global setting which will turn on logging / checks for things which are
 * considered especially bad for consoles.  Some of the checks are probably useful for PCs also.
 *
 * Throughout the code base there are specific things which dramatically affect performance and/or
 * are good indicators that something is wrong with the content.  These have PERF_ISSUE_FINDER in the
 * comment near the define to turn the individual checks on. 
 *
 * e.g. #if defined(LOG_DYNAMIC_LOAD_OBJECT) || LOOKING_FOR_PERF_ISSUES
 *
 * If one only cares about DLO, then one can enable the LOG_DYNAMIC_LOAD_OBJECT define.  Or one can
 * globally turn on all PERF_ISSUE_FINDERS :-)
 *
 **/
#ifndef LOOKING_FOR_PERF_ISSUES
#define LOOKING_FOR_PERF_ISSUES (0 && !FINAL_RELEASE)
#endif

/** Whether debugfSuppressed should be compiled out or not */
#ifndef SUPPORT_SUPPRESSED_LOGGING
#define SUPPORT_SUPPRESSED_LOGGING (LOOKING_FOR_PERF_ISSUES || _DEBUG)
#endif

/**
 * Enables/disables per-struct serialization performance tracking.
 */
#ifndef TRACK_SERIALIZATION_PERFORMANCE
#define TRACK_SERIALIZATION_PERFORMANCE 0
#endif

/**
 * Enables/disables detailed tracking of FAsyncPackage::Tick time.
 */
#ifndef TRACK_DETAILED_ASYNC_STATS
#define TRACK_DETAILED_ASYNC_STATS (0 || LOOKING_FOR_PERF_ISSUES)
#endif

/**
 * Enables general file IO stats tracking
 */
#ifndef TRACK_FILEIO_STATS
#define TRACK_FILEIO_STATS (0 || LOOKING_FOR_PERF_ISSUES)
#endif

/**
 * Enables/disables UE3 networking support
 */
#ifndef WITH_UE3_NETWORKING
    #define WITH_UE3_NETWORKING	1
#endif	// #ifndef WITH_UE3_NETWORKING

/**
 * Enables/disables GameSpy support. Enabled for Epic by default.
 * NOTE: Licensees must get the SDK from GameSpy.
 */
#ifndef WITH_GAMESPY
	#if _XBOX
		#define WITH_GAMESPY 0
	#else
		#define WITH_GAMESPY (EPIC_INTERNAL && WITH_UE3_NETWORKING)
	#endif
#endif

/**
 * Enables/disables integration with TestTrack Pro
 */
#ifndef WITH_TESTTRACK
	// TestTrack is only supported on Windows platform.
	#define WITH_TESTTRACK ( EPIC_INTERNAL && WIN32 && !CONSOLE && !FINAL_RELEASE )
#endif

/**
 * Enables/disables support for VTune integration
 */
#ifndef WITH_VTUNE
	// VTune is only supported on Windows platform.
	#define WITH_VTUNE ( _WINDOWS && !FINAL_RELEASE )
#endif


#ifndef WITH_XDK
	#if PS3
		#define WITH_XDK 0
	#elif XBOX
		#define WITH_XDK 1
	#else
		#define WITH_XDK (XDKINSTALLED && !SHIPPING_PC_GAME && WIN32)
	#endif
#endif

/**
 * Enables/disables Games for Windows Live support
 * NOTE: Licensees must get the SDK from Microsoft.
 */
#ifndef WITH_PANORAMA
	#define WITH_PANORAMA (0 && WITH_UE3_NETWORKING)
#endif

/** Whether to use the secure CRT variants. Enabled by default */
#ifndef USE_SECURE_CRT
	#define USE_SECURE_CRT 1
#endif

/**
 *	Enables/disables LZO compression support
 */
#ifndef WITH_LZO
	#if PS3 // ps3 uses zlib task
		#define WITH_LZO 	0
	#else
		#define WITH_LZO	1
	#endif
#endif	// #ifndef WITH_LZO

/** 
 * Whether support for integrating into the firewall is there
 */
#define WITH_FIREWALL_SUPPORT	0

/**
 * Whether this build supports generating a script patch
 */
//@script patcher
#ifndef SUPPORTS_SCRIPTPATCH_CREATION
	#if CONSOLE
		#define SUPPORTS_SCRIPTPATCH_CREATION 0
	#else
		// this should only be enabled when actually creating script patches, as it has a slight performance impact
		#define SUPPORTS_SCRIPTPATCH_CREATION 0
	#endif
#endif

/**
 * Enables SHA validation when loading signed files
 */
#ifndef USE_HASHES_SHA
	#if CONSOLE
		#define USE_HASHES_SHA 0
	#else
		#define USE_HASHES_SHA SHIPPING_PC_GAME
	#endif
#endif

//#if EPIC_INTERNAL && _XBOX
//	/**
//	 * Enables graphical surveys for user playtest sessions
//	 */
//	#ifndef WITH_SURVEYS
//		#define WITH_SURVEYS	1
//	#endif
//#endif
   
/**
 * Compile out FNameEntry::Flags on consoles in final release. This saves 64 bytes per name. The
 * name flags are only used for log suppresion and package saving so this doesn't remove any
 * used functionality.
 */
#ifndef SUPPORT_NAME_FLAGS
#define SUPPORT_NAME_FLAGS !(CONSOLE && FINAL_RELEASE)
#endif

/**
 * Allowing debug files means debugging files can be created, and CreateFileWriter does not need
 * a max file size it can grow to (needed for PS3 HD caching for now, may use for Xbox caching)
 */
#define ALLOW_DEBUG_FILES (!PS3 || !FINAL_RELEASE || FINAL_RELEASE_DEBUGCONSOLE)

/**
 * Set profiling define for non final release configurations. This is e.g. used by PIX.
 */
#if !FINAL_RELEASE && !FINAL_RELEASE_DEBUGCONSOLE
#define PROFILE 1
#endif

/**
 * Whether code should verify that no unreachable actors and components are referenced.
 */
#if FINAL_RELEASE || FINAL_RELEASE_DEBUGCONSOLE || SHIPPING_PC_GAME
#define VERIFY_NO_UNREACHABLE_OBJECTS_ARE_REFERENCED 0
#else
#define	VERIFY_NO_UNREACHABLE_OBJECTS_ARE_REFERENCED 1
#endif

#ifndef USE_ALLOCATORFIXEDSIZEFREELIST
#if PS3
#define USE_ALLOCATORFIXEDSIZEFREELIST 1
#else
#define USE_ALLOCATORFIXEDSIZEFREELIST 0
#endif
#endif

#endif	// #ifndef _HEADER_UNBUILD_H_

