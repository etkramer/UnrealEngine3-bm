//------------------------------------------------------------------------------
// Responsible for correctly defining platform-specific information.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxPlatform_H__
#define FxPlatform_H__

// Tell the core to use operator new to allocate memory by default.
#define FX_DEFAULT_TO_OPERATOR_NEW
// Tell the core to track basic allocation statistics, as well as look for
// possible memory corruption around allocated blocks.
// Uncomment the following line to enable allocation statistics gathering
// and memory corruption detection.
//#define FX_TRACK_MEMORY_STATS

// Tell the name table system not to allow spaces in FxNames.  When creating
// or loading an FxName, all spaces will be converted to underscores.
#ifdef __UNREAL__
	// Explicitly disallow spaces in names for Unreal.
	#define FX_DISALLOW_SPACES_IN_NAMES
#else
	// Uncomment the following line to disable spaces in FxNames.
	//#define FX_DISALLOW_SPACES_IN_NAMES
#endif

#define fx_std(func) std::##func

#if (defined(_MSC_VER) && _MSC_VER <= 1200) || defined(__GNUG__) || defined(PS3)
	#undef fx_std
	#define fx_std(func) func
#endif

#ifdef WIN32
	#undef FX_BIG_ENDIAN
	#define FX_LITTLE_ENDIAN
	#include "FxPlatformWin32.h"
#elif _XBOX
	// _XBOX is the define for both the original XBox and XBox360.  FaceFX
	// assumes that _XBOX means XBox360.  Define FX_XBOX1 in your project 
	// settings to use the original XBox build settings, or simply modify 
	// this code locally such that _XBOX always indicates an original XBox 
	// build.
	#ifdef FX_XBOX1
		// Original XBox build.
		#undef FX_XBOX_360
		#define FX_ORIGINAL_XBOX
		#undef FX_BIG_ENDIAN
		#define FX_LITTLE_ENDIAN
		#include "FxPlatformXBox.h"
	#else
		// XBox360 build.
		#undef FX_ORIGINAL_XBOX
		#define FX_XBOX_360
		#undef FX_LITTLE_ENDIAN
		#define FX_BIG_ENDIAN
		#include "FxPlatformXBox.h"
	#endif
#elif PS3
	// For now simply bring in the GCC platform header.
	#undef FX_LITTLE_ENDIAN
	#define FX_BIG_ENDIAN
	#include "FxPlatformGCC.h"
#elif __GNUG__
	#ifdef __POWERPC__
		#undef FX_LITTLE_ENDIAN
		#define FX_BIG_ENDIAN
	#else
		#undef FX_BIG_ENDIAN
		#define FX_LITTLE_ENDIAN
	#endif
	#include "FxPlatformGCC.h"
#else
	#error "Unknown platform in FxPlatform.h!  Please provide an appropriate platform header."
#endif

/// The OC3Ent namespace contains all of OC3 Entertainment's code.
namespace OC3Ent
{

/// The Face namespace contains all of the FaceFX SDK code and classes.
namespace Face
{

#ifdef FX_USE_BUILT_IN_BOOL_TYPE
	#define FxTrue true
	#define FxFalse false
#else
	static const FxBool FxTrue = 1;
	static const FxBool FxFalse = 0;
#endif

/// The character to delimit paths in the FaceFX SDK.
static const FxChar FxPathSeparator = '/';

#ifndef FX_CALL
	#define FX_CALL
#endif

#ifndef FX_INLINE
	#define FX_INLINE inline
#endif

#ifndef FX_RESTRICT
	#define FX_RESTRICT
#endif

#ifndef NULL
	#define NULL 0
#endif

#ifndef FxAssert
	#ifndef assert
		#define FxAssert
	#else
		#define FxAssert assert
	#endif
#endif

#ifndef FX_REAL_EPSILON
	#error "FX_REAL_EPSILON not defined for target platform!"
#endif

#ifndef FX_REAL_MAX
	#error "FX_REAL_MAX not defined for target platform!"
#endif

#ifndef FX_REAL_MIN
	#error "FX_REAL_MIN not defined for target platform!"
#endif

#ifndef FX_INT32_MAX
	#error "FX_INT32_MAX not defined for target platform!"
#endif

#ifndef FX_INT32_MIN
	#error "FX_INT32_MIN not defined for target platform!"
#endif

#ifndef FX_BYTE_MAX
	#error "FX_BYTE_MAX not defined for target platform!"
#endif

#ifndef FX_CHAR_BIT
	#error "FX_CHAR_BIT not defined for target platform!"
#endif

/// \typedef OC3Ent::Face::FxInt8
/// \brief A signed 8-bit integer.

/// \typedef OC3Ent::Face::FxUInt8
/// \brief An unsigned 8-bit integer.

/// \typedef OC3Ent::Face::FxInt16
/// \brief A signed 16-bit integer.

/// \typedef OC3Ent::Face::FxUInt16
/// \brief An unsigned 16-bit integer.

/// \typedef OC3Ent::Face::FxInt32
/// \brief A signed 32-bit integer.

/// \typedef OC3Ent::Face::FxUInt32
/// \brief An unsigned 32-bit integer.

/// \typedef OC3Ent::Face::FxReal
/// \brief A floating-point number.

/// \typedef OC3Ent::Face::FxDReal
/// \brief A double precision floating-point number.

/// \typedef OC3Ent::Face::FxBool
/// \brief If the compiler supports a built-in bool type, a bool, otherwise a signed, 32-bit integer.

/// \typedef OC3Ent::Face::FxChar
/// \brief A signed 8-bit character.

/// \typedef OC3Ent::Face::FxUChar
/// \brief An unsigned 8-bit character.

/// \typedef OC3Ent::Face::FxByte
/// \brief An unsigned 8-bit character.

/// \typedef OC3Ent::Face::FxSize
/// \brief An unsigned 32-bit integer. 

} // namespace Face

} // namespace OC3Ent

#endif
