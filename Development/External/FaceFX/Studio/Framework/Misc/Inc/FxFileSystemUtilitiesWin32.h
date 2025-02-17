//------------------------------------------------------------------------------
// Win32 platform-specific file system utility functions.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxFileSystemUtilitiesWin32_H__
#define FxFileSystemUtilitiesWin32_H__

#ifdef WIN32

#include "FxPlatform.h"
#include "FxString.h"
#include "FxArray.h"

namespace OC3Ent
{

namespace Face
{

// Recursively searches startDirectory for files matching fileExtFilter, and 
// places the resulting files (fully qualified paths) into fileList.
void FxRecursiveBuildFileList( const FxString& startDirectory, 
							   const FxString& fileExtFilter,
							   FxArray<FxString>& fileList, 
							   FxBool recurseSubdirectories );

// Recursively searches startDirectory for files matching fileExtFilter, and 
// places the resulting files (fully qualified paths) into fileList.
void FxBuildFileList( const FxString& startDirectory, 
					  const FxString& fileExtFilter, 
					  FxArray<FxString>& fileList, 
					  FxBool recurseSubdirectories );

} // namespace Face

} // namespace OC3Ent

#endif

#endif
