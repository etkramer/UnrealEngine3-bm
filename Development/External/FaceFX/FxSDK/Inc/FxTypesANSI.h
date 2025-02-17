//------------------------------------------------------------------------------
// ANSI type definitions.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxTypesANSI_H__
#define FxTypesANSI_H__

#include <cwchar>  // For wchar_t and supporting functions.

#define FX_USE_BUILT_IN_BOOL_TYPE

#define FX_API

#define FX_INLINE inline

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

	static const FxChar FxPlatformPathSeparator = '/';

} // namespace Face

} // namespace OC3Ent

#endif
