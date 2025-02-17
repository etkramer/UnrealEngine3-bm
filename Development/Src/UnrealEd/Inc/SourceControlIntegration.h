/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#ifndef __SOURCECONTORLINTEGRATION_H__
#define __SOURCECONTORLINTEGRATION_H__

#if _WINDOWS && !SHIPPING_PC_GAME
	#define HAVE_SCC 1
#else
	#define HAVE_SCC 0
#endif

#if HAVE_SCC
#include "scc.h"

typedef LONG (*FuncSccGetVersion) ( void );
typedef SCCRTN (*FuncSccInitialize) ( LPVOID*, HWND, LPCSTR, LPSTR, LPLONG, LPSTR, LPDWORD, LPDWORD );
typedef SCCRTN (*FuncSccUninitialize) ( LPVOID );
typedef SCCRTN (*FuncSccGetProjPath) ( LPVOID, HWND, LPSTR, LPSTR, LPSTR, LPSTR, BOOL, LPBOOL );
typedef SCCRTN (*FuncSccOpenProject) ( LPVOID, HWND, LPSTR, LPSTR, LPCSTR, LPSTR, LPCSTR, LPTEXTOUTPROC, LONG );
typedef SCCRTN (*FuncSccCloseProject) ( LPVOID );
typedef SCCRTN (*FuncSccQueryInfo) ( LPVOID, LONG, LPCSTR*, LPLONG );
typedef SCCRTN (*FuncSccHistory) ( LPVOID, HWND, LONG, LPCSTR*, LONG, LPCMDOPTS );
typedef SCCRTN (*FuncSccCheckOut) ( LPVOID, HWND, LONG, LPCSTR*, LPCSTR, LONG, LPCMDOPTS );
typedef SCCRTN (*FuncSccUncheckOut) ( LPVOID, HWND, LONG, LPCSTR*, LONG, LPCMDOPTS );
typedef SCCRTN (*FuncSccCheckIn) ( LPVOID, HWND, LONG, LPCSTR*, LPCSTR, LONG, LPCMDOPTS );
typedef SCCRTN (*FuncSccAdd) ( LPVOID, HWND, LONG, LPCSTR*, LPCSTR, LONG*, LPCMDOPTS );
typedef SCCRTN (*FuncSccRemove) ( LPVOID, HWND, LONG, LPCSTR*, LPCSTR, LONG, LPCMDOPTS );
typedef SCCRTN (*FuncSccRename) ( LPVOID, HWND, LPSTR, LPSTR );

class FSourceControlIntegration
{
public:
	FSourceControlIntegration();
	~FSourceControlIntegration();

	/**
	 * Called when the user wants to connect to a different project.  Will pop
	 * up the connection dialog and let them enter new info.
	 */
	void GetProjPath();

	/**
	 * Opens a project connection with the SCC.  Also handles logging the user in if they haven't been already.
	 */
	void Open();

	/**
	 * Closes the current project connection.
	 */
	void Close();

	/**
	 * Get source control state of a file by filename
	 *
	 * @param PackageName THe name of the package to query
	 * 
	 * @return The current source control status of the file (will return SCC_DontCare for a bad filename)
	 */
	ESourceControlState GetSCCState(const TCHAR* PackageName);

	/**
	 * Updates the status flags for a set of packages.
	 *
	 * @param	Packages	The packages to update status flags for
	 */
	void Update(const TArray<UPackage*>& Packages);

	/**
	 * Displays the revision history for the specified set of packages.
	 */
	void ShowHistory(const TArray<UPackage*>& Packages);

	////////////////////
	// File based operations

	/**
	 * Checks a file out of the SCC depot.
	 */
	void CheckOut(const TCHAR* PackageName);

	/**
	 * Reverts a file checked out of the SCC depot.
	 */
	void UncheckOut(const TCHAR* PackageName);

	/**
	 * Checks a file into the SCC depot.
	 */
	void CheckIn(const TCHAR* PackageName);

	/**
	 * Adds a new file to the depot.
	 *
	 * A "gotcha" here is that the file must exist on the hard drive before this function will work (obviously),
	 * so new files must be saved out before this will work.
	 */
	void Add(const TCHAR* PackageName, UBOOL bIsBinary=TRUE);

	/**
	 * Removes a file from the depot.
	 */
	void Remove(const TCHAR* PackageName);

	/**
	 * Renames a file in the depot.
	 */
	void Rename(const TCHAR* SourceName, const TCHAR* DestinationFilename);

	////////////////////
	// Package based operations

	/**
	 * Checks a package file out of the SCC depot.
	 */
	void CheckOut( UObject* InPackage );

	/**
	 * Reverts a package file checked out of the SCC depot.
	 */
	void UncheckOut( UObject* InPackage );

	/**
	 * Checks a package file into the SCC depot.
	 */
	void CheckIn( UObject* InPackage );

	/**
	 * Adds a new file to the depot.
	 *
	 * A "gotcha" here is that the file must exist on the hard drive before this function will work (obviously),
	 * so new packages must be saved out before this will work.
	 */
	void Add( UObject* InPackage );

	/**
	 * Removes a package file from the depot.
	 */
	void Remove( UObject* InPackage );

	/**
	 * Moves the specified package file to the trashcan director.
	 */
	void MoveToTrash( UObject* InPackage );

	/**
	 * Loads user/SCC information from the INI file.
	 */
	void LoadFromINI();

	/**
	 * Stores user/SCC information in the INI file.  This saves the user from having to log in every
	 * time they start up the editor.
	 */
	void SaveToINI();

	/**
	 * Creates a fully qualified filename, including the path all the way back to the drive letter,
	 * for the specific package.
	 *
	 * @param	PackageName	The name of the package (like "EngineMaterials")
	 *
	 * @return	The fully qualified filename
	 */
	FString GetFullFilename(const TCHAR* PackageName);

	/**
	 * Creates a fully qualified filename, including the path all the way back to the drive letter,
	 * for the specific package.
	 *
	 * @param	InPackage	The package we need the filename for
	 *
	 * @return	The fully qualified filename
	 */
	FString GetFullFilename(UObject* InPackage);

	UBOOL IsEnabled()
	{
		return bEnabled;
	}

	UBOOL IsOpen()
	{
		return bOpen;
	}

	/**
	 * Retrieve the singleton object
	 */
	static FSourceControlIntegration* GetGlobalSourceControlIntegration();

private:
	/**
	 * Attempts to find the users SCC system, load the library associated with it and bind the functions.
	 */
	void Init();

	/**
	 * Gets a string out of the registry.
	 *
	 * @param	InMainKey	The primary key
	 * @param	InKey		The secondary key
	 *
	 * @return	A char* buffer containing the string.  The caller is reponsible for freeing this!
	 */
	char* GetRegistryString( char* InMainKey, char* InKey );

	/**
	 * Creates an array of ANSI filenames from an array of packages.
	 *
	 * @param	InSrcPackages	The packages to get the filenames for
	 * @param	OutNumFiles		The number of filenames returned will be placed here
	 * @param	InPackages		If this is non-NULL, this will be filled with the packages which correspond to each filename
	 *
	 * @return	A pointer to an array of ANSI filenames
	 */
	char** GetFilenamesFromArray(const TArray<UPackage*>& InSrcPackages, INT& OutNumFiles, TArray<UPackage*>* InPackages = NULL);

	/**
	 * Frees memory previously allocated in GetFilenamesFromArray
	 * 
	 * @param FilenameArray Array of char*'s returned from GetFilenamesFromArray
	 * @param NumFilenames Size of FilenameArray
	 */
	void FreeFilenameArray(char** FilenameArray, INT NumFilenames);

	/**
	 * Cleaunup a list of packages to be just those packages that can be
	 * used with source control operations
	 *
	 * @param InPackages List of packages to clean out
	 * @param OutPackages Output list of packages
	 */
	void CleanupPackageArray(const TArray<UPackage*>& InPackages, TArray<UPackage*>& OutPackages);

	/**
	 * Convert a SCC status code to a ESourceControlState
	 *
	 * @param StatusCode Status code received from SCC operation
	 * 
	 * @param The Unreal source control state associated with the status code
	 */
	ESourceControlState GetSCCStateFromStatusCode(LONG StatusCode);

	/** 
	 * An interrupt function to delete the GSourceControlIntegration 
	 * singleton if we get a control-c when in a console program 
	 */
	static INT WINAPI ConsoleInterruptHandler(DWORD ControlType);

	/** Values returned from SCCInitialize */
	LONG Caps;
	char Label[SCC_AUXLABEL_LEN], ProviderName[SCC_NAME_LEN];
	DWORD CheckOutCommentLen;
	DWORD CommentLen;

	/** Indicates if source control integration is available or not. */
	UBOOL bEnabled;

	/** Indicates that the project is currently open. */
	UBOOL bOpen;

	/** Function bindings */
	FuncSccGetVersion SCC_GetVersion;
	FuncSccInitialize SCC_Initialize;
	FuncSccUninitialize SCC_Uninitialize;
	FuncSccGetProjPath SCC_GetProjPath;
	FuncSccOpenProject SCC_OpenProject;
	FuncSccCloseProject SCC_CloseProject;
	FuncSccQueryInfo SCC_QueryInfo;
	FuncSccHistory SCC_History;
	FuncSccCheckOut SCC_CheckOut;
	FuncSccUncheckOut SCC_UncheckOut;
	FuncSccCheckIn SCC_CheckIn;
	FuncSccAdd SCC_Add;
	FuncSccRemove SCC_Remove;
	FuncSccRename SCC_Rename;

	/** Flag to disable source control integration. **/
	UBOOL	bDisabled;

	/** Indicates whether or not the SCC provider support rename (p4, for example, does not, but VSS does) */
	UBOOL bSupportsRename;

	/** Handle to the loaded library. */
	void* LibraryHandle;

	/** Context pointer (used exclusively by the SCC provider) */
	void* Context;

	/** The user name we are using for log ins. */
	char UserName[SCC_USER_LEN];

	/** Not used for anything, but we need to pass it to the API for certain functions to work. */
	char LocalPath[SCC_PRJPATH_LEN];

	/** The project paths */
	char ProjectPath[SCC_PRJPATH_LEN];
	char AuxProjectPath[SCC_PRJPATH_LEN];

	/** Global pointer to the one and only SCC pointer */
	static FSourceControlIntegration* GSourceControlIntegration;
};

#endif // HAVE_SCC
#endif // #define __SOURCECONTORLINTEGRATION_H__

