/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
#ifndef UMEMORY_DEFINES_H
#define UMEMORY_DEFINES_H

/**
 *	IMPORTANT:	The malloc proxy flags are mutually exclusive.
 *				You can have either none of them set or only one of them.
 */
/** TRACK_MEM_USING_TAGS			- Define this to use the FMallocProxySimpleTag allocator.	*/
/** USE_MALLOC_PROFILER				- Define this to use the FMallocProfiler allocator.			*/
/** ENABLE_MEM_TAGGING              - This needs to be "on" to use the FMallocProfiler          */
/** TRACK_MEM_USING_STAT_SECTIONS	- Define this to use the FMallocProxySimpleTrack allocator.	*/
/** EXPORT_MEM_STATS_TO_PIX         - Enable this to export per-frame memory statistics to PIX  */


#if PS3
    #ifndef TRACK_MEM_USING_TAGS
    #define TRACK_MEM_USING_TAGS			0
    #endif
    #ifndef USE_MALLOC_PROFILER
    #define USE_MALLOC_PROFILER				0
    #endif
    #ifndef ENABLE_MEM_TAGGING
    #define ENABLE_MEM_TAGGING (USE_MALLOC_PROFILER && !FINAL_RELEASE)
    #endif
    #ifndef TRACK_MEM_USING_STAT_SECTIONS
    #define TRACK_MEM_USING_STAT_SECTIONS	0
    #endif
    #ifndef EXPORT_MEM_STATS_TO_PIX
    #define EXPORT_MEM_STATS_TO_PIX			0
    #endif
#elif _XBOX
    #ifndef TRACK_MEM_USING_TAGS
    #define TRACK_MEM_USING_TAGS			0
    #endif
    #ifndef USE_MALLOC_PROFILER
    #define USE_MALLOC_PROFILER				0
    #endif
    #ifndef ENABLE_MEM_TAGGING
    #define ENABLE_MEM_TAGGING (USE_MALLOC_PROFILER && !FINAL_RELEASE)
    #endif
    #ifndef TRACK_MEM_USING_STAT_SECTIONS
    #define TRACK_MEM_USING_STAT_SECTIONS	0
    #endif
    #ifndef EXPORT_MEM_STATS_TO_PIX
    #define EXPORT_MEM_STATS_TO_PIX			0
    #endif
#elif _WINDOWS
    #ifndef TRACK_MEM_USING_TAGS
    #define TRACK_MEM_USING_TAGS			0
    #endif
    #ifndef USE_MALLOC_PROFILER
    #define USE_MALLOC_PROFILER				0
    #endif
    #ifndef ENABLE_MEM_TAGGING
    #define ENABLE_MEM_TAGGING (USE_MALLOC_PROFILER && !FINAL_RELEASE)
    #endif
    #ifndef TRACK_MEM_USING_STAT_SECTIONS
    #define TRACK_MEM_USING_STAT_SECTIONS	0
    #endif
    #ifndef EXPORT_MEM_STATS_TO_PIX
    #define EXPORT_MEM_STATS_TO_PIX			0
    #endif
#else
    #ifndef TRACK_MEM_USING_TAGS
    #define TRACK_MEM_USING_TAGS			0
    #endif
    #ifndef USE_MALLOC_PROFILER
    #define USE_MALLOC_PROFILER				0
    #endif
    #ifndef ENABLE_MEM_TAGGING
    #define ENABLE_MEM_TAGGING (USE_MALLOC_PROFILER && !FINAL_RELEASE)
    #endif
    #ifndef TRACK_MEM_USING_STAT_SECTIONS
    #define TRACK_MEM_USING_STAT_SECTIONS	0
    #endif
    #ifndef EXPORT_MEM_STATS_TO_PIX
    #define EXPORT_MEM_STATS_TO_PIX			0
    #endif
#endif

/**
 * Whether to load symbols at startup. This might result in better results for walking the stack but will
 * also severely slow down the stack walking itself.
 * NOTE: in general you do not want this on as it will make the Malloc basically unusable.   When DumpAllocsToFile is called
 *       then the symbols will be loaded.  (there was a regression in SP2 and vista that causes the slowness)
 */
#if !CONSOLE
	#define LOAD_SYMBOLS_FOR_STACK_WALKING 0
#endif

#endif	//#ifndef UMEMORY_DEFINES_H

