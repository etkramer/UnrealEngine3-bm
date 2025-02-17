//------------------------------------------------------------------------------
// Win32 platform-specific file system utility functions.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxFileSystemUtilitiesWin32.h"
#include "FxConsole.h"

#ifdef WIN32

#include <windows.h>

namespace OC3Ent
{

namespace Face
{

// Recursively searches startDirectory for files matching fileExtFilter, and 
// places the resulting files (fully qualified paths) into fileList.
void 
FxRecursiveBuildFileList( const FxString& startDirectory, 
						  const FxString& fileExtFilter,
						  FxArray<FxString>& fileList, 
						  FxBool recurseSubdirectories )
{
	// Switch to the startDirectory.
	wxString StartDirectory(startDirectory.GetData(), wxConvLibc);
	if( !SetCurrentDirectory(StartDirectory.c_str()) )
	{
		FxString errorMsg = "Could not switch to directory: ";
		errorMsg = FxString::Concat(errorMsg, startDirectory);
		FxWarn(errorMsg.GetData());
		return;
	}

	WIN32_FIND_DATA FindData;
	// Use * as the filter so we will be returned directories as well.  The 
	// actual fileExtFilter is checked when we hit a file.
	HANDLE FindHandle = FindFirstFile(_T("*"), &FindData);

	// Check to see if the file handle is invalid.
	if( INVALID_HANDLE_VALUE != FindHandle )
	{
		do 
		{
			if( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				// Search hit a directory, so make sure it's not the "." or 
				// ".." directory.
				if( wxString(FindData.cFileName) != wxString(wxT(".")) &&
					wxString(FindData.cFileName) != wxString(wxT("..")) )
				{
					// The directory is not "." or "..", so construct the full 
					// path.
					wxString NewDirectory = StartDirectory;
					// Append a path separator if it isn't there.
					if( NewDirectory[NewDirectory.Length()-1] != '\\' )
					{
						NewDirectory += wxT("\\");
					}
					NewDirectory += FindData.cFileName;
					
					// Recurse into the new directory, if recurseSubdirectories 
					// is true.
					if( recurseSubdirectories )
					{
						FxRecursiveBuildFileList(FxString(NewDirectory.mb_str(wxConvLibc)), 
												 fileExtFilter, 
												 fileList, 
												 recurseSubdirectories);
					}
				}
			}
			else
			{
				// Search hit a file, so construct the full path and file name.
				wxString NewFile = StartDirectory;
				// Append a path separator if it isn't there.
				if( NewFile[NewFile.Length()-1] != '\\' )
				{
					NewFile += wxT("\\");
				}
				NewFile += _wcslwr(FindData.cFileName);
			
				// Check the extension of the found file against the filter 
				// we're searching for.
				if( NewFile.AfterLast('.') == wxString(fileExtFilter.GetData(), wxConvLibc) )
				{
					fileList.PushBack(FxString(NewFile.mb_str(wxConvLibc)));
				}
			}
		  // While there are more files in the current directory.
		} while( FindNextFile(FindHandle, &FindData) );

		// Close the find handle.
		FindClose(FindHandle);
	}
}

// Recursively searches startDirectory for files matching fileExtFilter, and 
// places the resulting files (fully qualified paths) into fileList.
void 
FxBuildFileList( const FxString& startDirectory, 
				 const FxString& fileExtFilter, 
				 FxArray<FxString>& fileList, 
				 FxBool recurseSubdirectories )
{
	// Get the current directory.
	TCHAR CurrentDirectory[_MAX_PATH] = {0};
	GetCurrentDirectory(_MAX_PATH, CurrentDirectory);

	// Recursively build a file list based on the fileExtFilter.
	FxRecursiveBuildFileList(startDirectory, 
						     fileExtFilter, 
						     fileList, 
						     recurseSubdirectories);

	// FxRecursiveBuildFileList could have changed the current directory, so 
	// switch back to what it was.
	SetCurrentDirectory(CurrentDirectory);
}

} // namespace Face

} // namespace OC3Ent

#endif

