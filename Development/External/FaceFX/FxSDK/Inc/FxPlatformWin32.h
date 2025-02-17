//------------------------------------------------------------------------------
// The Win32 platform.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxPlatformWin32_H__
#define FxPlatformWin32_H__

#include <cwchar>  // For wchar_t and supporting functions.

#define FX_USE_BUILT_IN_BOOL_TYPE

namespace OC3Ent
{

namespace Face
{
	typedef char				FxInt8;
	typedef unsigned char		FxUInt8;
	typedef short				FxInt16;
	typedef unsigned short		FxUInt16;
	typedef int					FxInt32;
	typedef unsigned int		FxUInt32;

	typedef float				FxReal;
	typedef double				FxDReal;

#ifdef FX_USE_BUILT_IN_BOOL_TYPE
	typedef bool				FxBool;
#else
	typedef int					FxBool;
#endif

	typedef char				FxAChar;
	typedef unsigned char		FxUChar;
	typedef wchar_t				FxWChar;

	typedef char 				FxChar;

	typedef unsigned char		FxByte;

	typedef unsigned int		FxSize;

	static const FxChar FxPlatformPathSeparator = '\\';

#ifdef _MSC_VER // Microsoft compiler specific stuff.
	#ifdef _DEBUG
		#define FX_DEBUG 
	#endif
	// Shut up the warning level 4 conditional expression is constant warnings.
	#pragma warning(disable : 4127)
	// Shut up the function marked as __forceinline not inlined warnings.
	#pragma warning(disable : 4714)

	// Turn on some useful warnings that are turned off by default.

	// 'operator/operation' : unsafe conversion from 'type of expression' to 'type required'
	#pragma warning(default : 4191)
	// 'operator' : conversion from 'type1' to 'type2', possible loss of data
	#pragma warning(default : 4254)
	// 'function' : no function prototype given: converting '()' to '(void)'
	#pragma warning(default : 4255)
	// 'operator' : unsigned/negative constant mismatch
	#pragma warning(default : 4287)
	// nonstandard extension used : 'var' : loop control variable declared in the for-loop is used outside the for-loop scope
	#pragma warning(default : 4289)
	// 'type' : use of undefined type detected in CLR meta-data - use of this type may lead to a runtime exception
	#pragma warning(default : 4339)
	// 'type name' : type-name exceeds meta-data limit of 'limit' characters
	#pragma warning(default : 4536)
	// expression before comma evaluates to a function which is missing an argument list
	#pragma warning(default : 4545)
	// function call before comma missing argument list
	#pragma warning(default : 4546)
	// 'operator' : operator before comma has no effect; expected operator with side-effect
	#pragma warning(default : 4547)
	// 'operator' : operator before comma has no effect; did you intend 'operator'?
	#pragma warning(default : 4549)
	// '__assume' contains effect 'effect'
	#pragma warning(default : 4557)
	// digraphs not supported with -Ze. Character sequence 'digraph' not interpreted as alternate token for 'char'
	#pragma warning(default : 4628)
	// 'parameter' : no directional parameter attribute specified, defaulting to [in]
	#pragma warning(default : 4682)
	// 'user-defined type' : possible change in behavior, change in UDT return calling convention
	#pragma warning(default : 4686)
	// 'identifier' : identifier was truncated to 'number' characters in the debug information
	#pragma warning(default : 4786)
	// native code generated for function 'function': 'reason'
	#pragma warning(default : 4793)
	// wide string literal cast to 'LPSTR'
	#pragma warning(default : 4905)
	// string literal cast to 'LPWSTR'
	#pragma warning(default : 4906)
	// we are assuming the type library was built for number-bit pointers
	#pragma warning(default : 4931)
	// reinterpret_cast used between related classes: 'class1' and 'class2'
	#pragma warning(default : 4946)
	// illegal copy-initialization; more than one user-defined conversion has been implicitly applied
	#pragma warning(default : 4928)

	#define FX_CALL __cdecl
	#define FX_INLINE __forceinline
	#if _MSC_VER <= 1200
	    // Make vs6's for scoping behave.
		#define for if( 0 ) {} else for
	    // Shut up the conversion warnings.
		#pragma warning(disable : 4244)
		// Shut up the inline function has been removed
		// warnings.
		#pragma warning(disable : 4514)
		// Shut up the typedef-name used as synonym for class-name warnings.
		#pragma warning(disable : 4097)
		// Shut up the copy constructor could not be generated warnings.
		#pragma warning(disable : 4511)
		// Shut up the assignment operator could not be generated warnings.
		#pragma warning(disable : 4512)
	#endif
	#if _MSC_VER >= 1310
		#define FX_PLATFORM_SUPPORTS_PARTIAL_TEMPLATE_SPECIALIZATION
	#endif
#else
	#define FX_CALL
	#define FX_INLINE inline
#endif

} // namespace Face

} // namespace OC3Ent

// Bring in the ANSI platform.
#include "FxPlatformANSI.h"

#endif
