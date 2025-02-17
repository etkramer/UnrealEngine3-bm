/*=============================================================================
	UnXenon.h: Unreal definitions for Visual Studio .NET 2003 targetting Xe.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*----------------------------------------------------------------------------
	Platform compiler definitions.
----------------------------------------------------------------------------*/

#ifndef NOMINMAX
	#define NOMINMAX
#endif

// The following headers make use of pragma pack() without an argument which resets 
// to the default packing defined in the project which makes it impossible to correctly 
// use a packing other than /Zp8. 
//
// C:\Program Files\Microsoft Xenon SDK\include\xbox\d3d9caps.h(525):#pragma pack()
// C:\Program Files\Microsoft Xenon SDK\include\xbox\d3d9types.h(2830):#pragma pack()
// C:\Program Files\Microsoft Xenon SDK\include\xbox\poppack.h(31):#pragma pack()
// C:\Program Files\Microsoft Xenon SDK\include\xbox\poppack.h(34):#pragma pack()
// C:\Program Files\Microsoft Xenon SDK\include\xbox\stdlib.h(97):#pragma pack()
// C:\Program Files\Microsoft Xenon SDK\include\xbox\stdlib.h(138):#pragma pack()
//
// The problem is:
//
// #pragma pack(push,8)
// #include <xtl.h>
// #pragma pack(pop)
//
// so e.g. d3dcaps.h's
//
// #pragma pack()
//
// will reset the packing to whatever is defined via /Zp instead of 8 like the code expects.
//
// A temporary partial workaround is to reset the packing after each header. It does not catch
// all cases though.

#pragma pack(push,8)
#include <stdlib.h>
#pragma pack(8)
#include <stdio.h>
#pragma pack(8)
#include <math.h>
#pragma pack(8)
#include <float.h>
#pragma pack(8)
//@todo xenon: remove once XDK handles NOMINMAX
#define max(a, b)  (((a) > (b)) ? (a) : (b)) 
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#include <xtl.h>
#pragma pack(8)
#include <xbox.h>
#pragma pack(8)
#include <xonline.h>
#pragma pack(8)
#if ALLOW_NON_APPROVED_FOR_SHIPPING_LIB
#include <xbdm.h>
#endif
#pragma pack(8)
#include <xgraphics.h>
#undef max
#undef min
#pragma pack(8)
#include <xaudio.h>
#pragma pack(8)
#include <x3daudio.h>
#pragma pack(8)
#include <winsockx.h>
#pragma pack(8)
#include <tchar.h>
#pragma pack(8)
#include <ppcintrinsics.h>
#pragma pack(8)
#if ALLOW_TRACEDUMP
#include <tracerecording.h>
#endif
#pragma pack(pop)

#ifdef PF_MAX
#undef PF_MAX
#endif

/*----------------------------------------------------------------------------
	Platform specifics types and defines.
----------------------------------------------------------------------------*/

// PowerPC
#ifndef _PPC_
#define _PPC_
#endif

// Undo any Windows defines.
#undef BYTE
#undef WORD
#undef DWORD
#undef INT
#undef FLOAT
#undef MAXBYTE
#undef MAXWORD
#undef MAXDWORD
#undef MAXINT
#undef CDECL

// Make sure HANDLE is defined.
#define HANDLE void*

// Sizes.
enum {DEFAULT_ALIGNMENT = 8 }; // Default boundary to align memory allocations on.

#define RENDER_DATA_ALIGNMENT DEFAULT_ALIGNMENT // the value to align some renderer bulk data to


// Optimization macros (preceded by #pragma).
#define DISABLE_OPTIMIZATION optimize("",off)
#ifdef _DEBUG
	#define ENABLE_OPTIMIZATION  optimize("",off)
#else
	#define ENABLE_OPTIMIZATION  optimize("",on)
#endif

//@todo xenon: October XDK also defines FORCEINLINE - use our version for now
#undef FORCEINLINE

// Function type macros.
#define VARARGS     __cdecl					/* Functions with variable arguments */
#define CDECL	    __cdecl					/* Standard C function */
#define STDCALL		__stdcall				/* Standard calling convention */
#define FORCEINLINE __forceinline			/* Force code to be inline */
#define FORCENOINLINE __declspec(noinline)	/* Force code to NOT be inline */
#define ZEROARRAY                           /* Zero-length arrays in structs */

// Compiler name.
#ifdef _DEBUG
	#define COMPILER "Compiled with Visual C++ (Xenon) Debug"
#else
	#define COMPILER "Compiled with Visual C++ (Xenon)"
#endif

// PowerPC can't cope with misaligned data access.
#define REQUIRES_ALIGNED_ACCESS 1

// MS compiler does support __noop
#define COMPILER_SUPPORTS_NOOP 1

// Compiler supports the restrict keyword for pointer aliasing
#define RESTRICT __restrict

// some of these types are actually typedef'd in windows.h.
// we redeclare them anyway so we will know if/when the basic
// typedefs may change.

// Unsigned base types.
typedef unsigned __int8		BYTE;		// 8-bit  unsigned.
typedef unsigned __int16	WORD;		// 16-bit unsigned.
typedef unsigned __int32	UINT;		// 32-bit unsigned.
typedef unsigned long		DWORD;		// defined in windows.h
typedef unsigned __int64	QWORD;		// 64-bit unsigned.

// Signed base types.
typedef	signed __int8		SBYTE;		// 8-bit  signed.
typedef signed __int16		SWORD;		// 16-bit signed.
typedef signed __int32 		INT;		// 32-bit signed.
typedef long				LONG;		// defined in windows.h
typedef signed __int64		SQWORD;		// 64-bit signed.

// Character types.
typedef char				ANSICHAR;	// An ANSI character. normally a signed type.
typedef wchar_t				UNICHAR;	// A unicode character. normally a signed type.
typedef wchar_t				WCHAR;		// defined in windows.h

// Other base types.
typedef UINT				UBOOL;		// Boolean 0 (false) or 1 (true).
typedef float				FLOAT;		// 32-bit IEEE floating point.
typedef double				DOUBLE;		// 64-bit IEEE double.
// SIZE_T is defined in windows.h based on WIN64 or not.
// Plus, it uses ULONG instead of size_t, so we can't use size_t.
typedef INT					PTRINT;		// Integer large enough to hold a pointer.

// Bitfield type.
typedef unsigned long       BITFIELD;	// For bitfields.

#define DECLARE_UINT64(x)	x

/*----------------------------------------------------------------------------
Stack walking.
----------------------------------------------------------------------------*/

/** @name ObjectFlags
* Flags used to control the output from stack tracing
*/
typedef DWORD EVerbosityFlags;
#define VF_DISPLAY_BASIC		0x00000000
#define VF_DISPLAY_FILENAME		0x00000001
#define VF_DISPLAY_MODULE		0x00000002
#define VF_DISPLAY_ALL			0xffffffff

// Variable arguments.
/**
* Helper function to write formatted output using an argument list
*
* @param Dest - destination string buffer
* @param DestSize - size of destination buffer
* @param Count - number of characters to write (not including null terminating character)
* @param Fmt - string to print
* @param Args - argument list
* @return number of characters written or -1 if truncated
*/
INT appGetVarArgs( TCHAR* Dest, SIZE_T DestSize, INT Count, const TCHAR*& Fmt, va_list ArgPtr );

/**
* Helper function to write formatted output using an argument list
* ASCII version
*
* @param Dest - destination string buffer
* @param DestSize - size of destination buffer
* @param Count - number of characters to write (not including null terminating character)
* @param Fmt - string to print
* @param Args - argument list
* @return number of characters written or -1 if truncated
*/
INT appGetVarArgsAnsi( ANSICHAR* Dest, SIZE_T DestSize, INT Count, const ANSICHAR*& Fmt, va_list ArgPtr );

#define GET_VARARGS(msg,msgsize,len,lastarg,fmt) {va_list ap; va_start(ap,lastarg);appGetVarArgs(msg,msgsize,len,fmt,ap);}
#define GET_VARARGS_ANSI(msg,msgsize,len,lastarg,fmt) {va_list ap; va_start(ap,lastarg);appGetVarArgsAnsi(msg,msgsize,len,fmt,ap);}
#define GET_VARARGS_RESULT(msg,msgsize,len,lastarg,fmt,result) {va_list ap; va_start(ap, lastarg); result = appGetVarArgs(msg,msgsize,len,fmt,ap); }
#define GET_VARARGS_RESULT_ANSI(msg,msgsize,len,lastarg,fmt,result) {va_list ap; va_start(ap, lastarg); result = appGetVarArgsAnsi(msg,msgsize,len,fmt,ap); }

// Unwanted VC++ level 4 warnings to disable.
#pragma warning(disable : 4244) /* conversion to float, possible loss of data							*/
#pragma warning(disable : 4699) /* creating precompiled header											*/
#pragma warning(disable : 4200) /* Zero-length array item at end of structure, a VC-specific extension	*/
#pragma warning(disable : 4100) /* unreferenced formal parameter										*/
#pragma warning(disable : 4514) /* unreferenced inline function has been removed						*/
#pragma warning(disable : 4201) /* nonstandard extension used : nameless struct/union					*/
#pragma warning(disable : 4710) /* inline function not expanded											*/
#pragma warning(disable : 4714) /* __forceinline function not expanded									*/  
#pragma warning(disable : 4702) /* unreachable code in inline expanded function							*/
#pragma warning(disable : 4711) /* function selected for automatic inlining								*/
#pragma warning(disable : 4127) /* Conditional expression is constant									*/
#pragma warning(disable : 4512) /* assignment operator could not be generated                           */
#pragma warning(disable : 4245) /* conversion from 'enum ' to 'unsigned long', signed/unsigned mismatch */
#pragma warning(disable : 4389) /* signed/unsigned mismatch                                             */
#pragma warning(disable : 4251) /* needs to have dll-interface to be used by clients of class 'ULinker' */
#pragma warning(disable : 4275) /* non dll-interface class used as base for dll-interface class         */
#pragma warning(disable : 4511) /* copy constructor could not be generated                              */
#pragma warning(disable : 4355) /* this used in base initializer list                                   */
#pragma warning(disable : 4291) /* typedef-name '' used as synonym for class-name ''                    */
#pragma warning(disable : 4324) /* structure was padded due to __declspec(align())						*/

// It'd be nice to turn these on, but at the moment they can't be used in DEBUG due to the varargs stuff.
#pragma warning(disable : 4189) /* local variable is initialized but not referenced */
#pragma warning(disable : 4505) /* unreferenced local function has been removed							*/

// If C++ exception handling is disabled, force guarding to be off.
#if !(defined _CPPUNWIND) && !EXCEPTIONS_DISABLED
	#error "Bad VCC option: C++ exception handling must be enabled"
#endif

// Make sure characters are unsigned.
#ifdef _CHAR_UNSIGNED
	#error "Bad VC++ option: Characters must be signed"
#endif

// No Intel assembly.
#define ASM_X86 0

// DLL file extension.
#define DLLEXT TEXT(".dll")

// Pathnames.
#define PATH(s) s

// NULL.
#define NULL 0

// Helper.
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

// Platform support options.
#define FORCE_ANSI_LOG           1

#define CP_OEMCP 1
// String conversion classes
#include "UnStringConv.h"

/**
 * NOTE: The objects these macros declare have very short lifetimes. They are
 * meant to be used as parameters to functions. You cannot assign a variable
 * to the contents of the converted string as the object will go out of
 * scope and the string released.
 *
 * NOTE: The parameter you pass in MUST be a proper string, as the parameter
 * is typecast to a pointer. If you pass in a char, not char* it will crash.
 *
 * Usage:
 *
 *		SomeApi(TCHAR_TO_ANSI(SomeUnicodeString));
 *
 *		const char* SomePointer = TCHAR_TO_ANSI(SomeUnicodeString); <--- Bad!!!
 */
#define TCHAR_TO_ANSI(str) (ANSICHAR*)FTCHARToANSI((const TCHAR*)str)
// Xe doesn't support to OEM conversions so just use ANSI
#define TCHAR_TO_OEM(str) (ANSICHAR*)FTCHARToANSI((const TCHAR*)str)
#define ANSI_TO_TCHAR(str) (TCHAR*)FANSIToTCHAR((const ANSICHAR*)str)
#undef CP_OEMCP

// Strings.
#define LINE_TERMINATOR TEXT("\r\n")
#define PATH_SEPARATOR TEXT("\\")
#define appIsPathSeparator( Ch )	((Ch) == PATH_SEPARATOR[0])

// Bitfield alignment.
#define GCC_PACK(n)
#define GCC_ALIGN(n)
#define GCC_BITFIELD_MAGIC
#define MS_ALIGN(n) __declspec(align(n))

/*----------------------------------------------------------------------------
	Math Functions.
----------------------------------------------------------------------------*/

const FLOAT	SRandTemp = 1.f;
extern INT	GSRandSeed;

inline FLOAT	appExp( FLOAT Value )			{ return expf(Value); }
inline FLOAT	appLoge( FLOAT Value )			{	return logf(Value); }
inline FLOAT	appFmod( FLOAT Y, FLOAT X )		{ return fmodf(Y,X); }
inline FLOAT	appSin( FLOAT Value )			{ return sinf(Value); }
inline FLOAT 	appAsin( FLOAT Value ) 			{ return asinf( (Value<-1.f) ? -1.f : ((Value<1.f) ? Value : 1.f) ); }
inline FLOAT 	appCos( FLOAT Value ) 			{ return cosf(Value); }
inline FLOAT 	appAcos( FLOAT Value ) 			{ return acosf( (Value<-1.f) ? -1.f : ((Value<1.f) ? Value : 1.f) ); }
inline FLOAT	appTan( FLOAT Value )			{ return tanf(Value); }
inline FLOAT	appAtan( FLOAT Value )			{ return atanf(Value); }
inline FLOAT	appAtan2( FLOAT Y, FLOAT X )	{ return atan2f(Y,X); }
inline FLOAT	appSqrt( FLOAT F )				{ return __fsqrts(F); }
inline FLOAT	appInvSqrt( FLOAT F )			{ return 1.f / __fsqrt(F); }
inline FLOAT	appPow( FLOAT A, FLOAT B )		
{
	// ~1000 cycles faster than powf and very accurate. Might not handle invalid input like powf though.
	DOUBLE Ln = log( A );
	Ln *= B;
	const DOUBLE Result = exp(Ln);
	return Result;
}
inline UBOOL	appIsNaN( FLOAT A )				{ return _isnan(A) != 0; }
inline UBOOL	appIsFinite( FLOAT A )			{ return _finite(A) != 0; }
inline INT		appFloor( FLOAT F )				{ return floorf( F ); }
inline INT		appCeil( FLOAT Value )			{ return (INT)ceilf(Value); }
inline INT		appRound( FLOAT F )				{ return (INT) floorf(F + 0.5f); }
inline INT		appTrunc( FLOAT F )				{ return (INT) F; }
inline FLOAT	appTruncFloat( FLOAT F )		{ return __fcfid(__fctidz(F)); }
inline FLOAT	appFractional( FLOAT Value )	{ return Value - appTruncFloat( Value ); }
inline FLOAT	appCopySign( FLOAT A, FLOAT B ) { return _copysign(A,B); }
inline INT		appRand()						{ return rand(); }
inline void		appRandInit(INT Seed)			{ srand( Seed ); }
inline FLOAT	appFrand()						{ return rand() / (FLOAT)RAND_MAX; }
inline void		appSRandInit( INT Seed )		{ GSRandSeed = Seed; }
inline FLOAT	appSRand() 
{ 
	GSRandSeed = (GSRandSeed * 196314165) + 907633515; 
	//@todo xenon: fix type punning
	FLOAT Result;
	*(INT*)&Result = (*(INT*)&SRandTemp & 0xff800000) | (GSRandSeed & 0x007fffff);
	return appFractional(Result); 
}

/**
 * Counts the number of leading zeros in the bit representation of the value
 *
 * @param Value the value to determine the number of leading zeros for
 *
 * @return the number of zeros before the first "on" bit
 */
FORCEINLINE DWORD appCountLeadingZeros(DWORD Value)
{
	// Use the PPC intrinsic
	return _CountLeadingZeros(Value);
}

/**
 * Computes the base 2 logarithm for an integer value that is greater than 0.
 * The result is rounded down to the nearest integer.
 *
 * @param Value the value to compute the log of
 */
inline DWORD appFloorLog2(DWORD Value)
{
	return 31 - appCountLeadingZeros(Value);
}

/**
 * Fast inverse square root using the estimate intrinsic with Newton-Raphson refinement
 * Accurate to at least 0.00000001 of appInvSqrt() and 2.45x faster
 *
 * @param F the float to estimate the inverse square root of
 *
 * @return the inverse square root
 */
inline FLOAT appInvSqrtEst(FLOAT F)
{
	// 0.5 * rsqrt * (3 - x * rsqrt(x) * rsqrt(x))
	const FLOAT RecipSqRtEst = __frsqrte(F);
	return 0.5f * RecipSqRtEst * (3.f - (F * (RecipSqRtEst * RecipSqRtEst)));
}

/*----------------------------------------------------------------------------
	Platform specific globals.
----------------------------------------------------------------------------*/

/** Screen width													*/
extern INT		GScreenWidth;
/** Screen height													*/
extern INT		GScreenHeight;
/** Whether we're allowed to swap/ resolve							*/
extern UBOOL	GAllowBufferSwap;

/*----------------------------------------------------------------------------
	Memory.
----------------------------------------------------------------------------*/

#define DEFINED_appMemcpy
//inline void appMemcpy( void* Dest, const void* Src, INT Count ) { XMemCpy( Dest, Src, Count ); } 
inline void appMemcpy( void* Dest, const void* Src, INT Count ) { memcpy( Dest, Src, Count ); }
#define DEFINED_appMemzero
//inline void appMemzero( void* Dest, INT Count ) { XMemSet(Dest,0,Count); }
inline void appMemzero( void* Dest, INT Count ) { memset(Dest,0,Count); }

extern "C" void* __cdecl _alloca(size_t);
#define appAlloca(size) ((size==0) ? 0 : _alloca((size+7)&~7))

/**
* Enforces strict memory load/store ordering across the memory barrier call.
*/
FORCEINLINE void appMemoryBarrier()
{
	// Lightweight Synchronize.
	__lwsync();
}

extern UINT appEDRAMOffset( const TCHAR* Usage, UINT Size );
extern UINT appHiZOffset( const TCHAR* Usage, UINT Size );

/**
 * Defragment the texture pool.
 */
extern void appDefragmentTexturePool();

/**
 * Log the current texture memory stats.
 *
 * @param Message	This text will be included in the log
 */
extern void appDumpTextureMemoryStats(const TCHAR* Message);


/*----------------------------------------------------------------------------
	Initialization.
----------------------------------------------------------------------------*/

extern void appXenonInit( const TCHAR* CommandLine );

/*-----------------------------------------------------------------------------
	Time.
-----------------------------------------------------------------------------*/

extern DOUBLE GSecondsPerCycle;
extern QWORD GCyclesPerSecond;
extern QWORD GNumTimingCodeCalls;

#define DEFINED_appSeconds 1
/**
 * Returns time in seconds. Origin is arbitrary.
 *
 * @return time in seconds (arbitrary origin)
 */
inline DOUBLE appSeconds()
{
#if !FINAL_RELEASE
	GNumTimingCodeCalls++;
#endif
	LARGE_INTEGER Cycles;
	QueryPerformanceCounter(&Cycles);
	return Cycles.QuadPart * GSecondsPerCycle;
}

#define DEFINED_appCycles 1
/**
 * Current high resolution cycle counter. Origin is arbitrary.
 *
 * @return current value of high resolution cycle counter - origin is aribtrary
 */
inline DWORD appCycles()
{
#if !FINAL_RELEASE
	GNumTimingCodeCalls++;
#endif
	LARGE_INTEGER Cycles;
	QueryPerformanceCounter(&Cycles);
	return Cycles.QuadPart;
}

/** 
 * Returns whether the line can be broken between these two characters
 */
UBOOL appCanBreakLineAt( TCHAR Previous, TCHAR Current );

/*-----------------------------------------------------------------------------
	Misc.
-----------------------------------------------------------------------------*/

/**
 * Retrieve a environment variable from the system
 *
 * @param VariableName The name of the variable (ie "Path")
 * @param Result The string to copy the value of the variable into
 * @param ResultLength The size of the Result string
 */
inline void appGetEnvironmentVariable(const TCHAR* VariableName, TCHAR* Result, INT ResultLength)
{
	// return an empty string
	*Result = 0;
}

#include "UMemoryDefines.h"

/**
* Converts the passed in program counter address to a human readable string and appends it to the passed in one.
* @warning: The code assumes that HumanReadableString is large enough to contain the information.
*
* @param	ProgramCounter			Address to look symbol information up for
* @param	HumanReadableString		String to concatenate information with
* @param	HumanReadableStringSize size of string in characters
* @param	VerbosityFlags			Bit field of requested data for output. -1 for all output.
*/ 
void appProgramCounterToHumanReadableString( QWORD ProgramCounter, ANSICHAR* HumanReadableString, SIZE_T HumanReadableStringSize, EVerbosityFlags VerbosityFlags = VF_DISPLAY_ALL );

/**
 * Capture a stack backtrace and optionally use the passed in exception pointers.
 *
 * @param	BackTrace			[out] Pointer to array to take backtrace
 * @param	MaxDepth			Entries in BackTrace array
 * @param	Context				Optional thread context information
 * @return	Number of function pointers captured
 */
DWORD appCaptureStackBackTrace( QWORD* BackTrace, DWORD MaxDepth, void* Context = NULL );

/**
 * Handles IO failure by ending gameplay.
 *
 * @param Filename	If not NULL, name of the file the I/O error occured with
 */
void appHandleIOFailure( const TCHAR* Filename );

/**
 * Push a marker for external profilers.
 *
 * @param MarkerName A descriptive name for the marker to display in the profiler
 */
inline void appPushMarker( const TCHAR* MarkerName )
{
	//@TODO
}

/**
 * Pop the previous marker for external profilers.
 */
inline void appPopMarker( )
{
	//@TODO
}

/*----------------------------------------------------------------------------
	Splash screen.
----------------------------------------------------------------------------*/

/**
 * Does one-time splash screen initialization
 *
 */
extern void appXenonInitSplash();

/**
 * Shows splash screen and optionally persists display so we can reboot.
 *
 * @param	SplashName	path to file containing splash screen
 * @param	bPersist	whether to persist display
 */
void appXenonShowSplash( const TCHAR* SplashName, UBOOL bPersist = FALSE );

/** struct for passing data when rebooting between images */
struct FXenonLaunchData
{
	enum { MAX_CMDLINE_LENGTH=1000 };
    ANSICHAR CmdLine[MAX_CMDLINE_LENGTH];
};

/*----------------------------------------------------------------------------
	UObject* <-> DWORD functions.
----------------------------------------------------------------------------*/
/** 
 * Support functions for overlaying an object/name pointer onto an index (like in script code
 */
FORCEINLINE DWORD appPointerToDWORD(void* Pointer)
{
	return (DWORD)Pointer;
}

FORCEINLINE void* appDWORDToPointer(DWORD Value)
{
	return (void*)Value;
}

/** Returns the title id of this game */
DWORD appGetTitleId(void);

/**
 * Locations for all of the threads to run
 *
 * 0 = Game thread
 * 1 = Physics simulation & Stats listener
 * 2 = Stats sender
 * 3 = Rendering thread
 * 4 = Thread pool & XHV & Audio
 * 5 = Thread pool & Async i/o
 */
#define ASYNCIO_HWTHREAD			5
#define XHV_HWTHREAD				4
#define PHYSICS_HWTHREAD			1
#define RENDERING_HWTHREAD			3
#define AUDIO_HWTHREAD				XAUDIOTHREADUSAGE_THREAD4
#define THREAD_POOL_COUNT			2
#define THREAD_POOL1				32
#define THREAD_POOL2				16
#define THREAD_POOL_HWTHREAD		(THREAD_POOL1 | THREAD_POOL2)
#define STATS_LISTENER_HWTHREAD		1
#define STATS_SENDER_HWTHREAD		2
#define ASYNC_INPUT_HWTHREAD		1

/**
 * Prints out detailed memory info (slow)
 *
 * @param Ar Device to send output to
 */
void appXenonDumpDetailedMemoryInformation( class FOutputDevice& Ar );

