/*=============================================================================
	SourceControlIntegration.cpp: Interface for talking to source control clients
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "SourceControlIntegration.h"

#if HAVE_SCC

/** Global pointer to the one and only SCC pointer */
FSourceControlIntegration* FSourceControlIntegration::GSourceControlIntegration = NULL;

/** 
 * Return a valid HWND for use with SCC. If we aren't running in UnrealEd, then
 * GApp is NULL.
 *
 * @return An HWND to act as the parent to the SCC windows.
 */
static inline HWND GetParentWindow()
{
	return GApp ? (HWND)GApp->EditorFrame->GetHandle() : GetConsoleWindow() ? GetConsoleWindow() : GetDesktopWindow();
}

/**
 * A pointer to this function is passed into SCC_OpenProject.  If the
 * SCC provider has anything to say, it pipes text messages through
 * here.
 */

long SCCTextOutput( LPCSTR InStr, DWORD InWord )
{
	FString wk = ANSI_TO_TCHAR( InStr );

	switch( InWord )
	{
		case SCC_MSG_DOCANCEL:
			return 0;

		case SCC_MSG_INFO:
		case SCC_MSG_STATUS:
			debugf( NAME_SourceControl, TEXT("%s"), *wk );
			break;

		case SCC_MSG_WARNING:
			debugf( NAME_SourceControl, TEXT("Warning: %s"), *wk );
			//appMsgf( AMT_OK, *wk );
			break;

		case SCC_MSG_ERROR:
			debugf( NAME_SourceControl, TEXT("ERROR: %s"), *wk );
			appMsgf( AMT_OK, *wk );
			break;
	}

	return SCC_MSG_RTN_OK;
}

FSourceControlIntegration::FSourceControlIntegration()
{
	// set the pointer to our singleton
	check(GSourceControlIntegration == NULL);
	GSourceControlIntegration = this;

	Init();
}

FSourceControlIntegration::~FSourceControlIntegration()
{
	if (bOpen == TRUE)
	{
		Close();
	}

	// we no longer have a singleton
	GSourceControlIntegration = NULL;

	// remove our Control-C handler
	SetConsoleCtrlHandler(ConsoleInterruptHandler, FALSE);

	if( LibraryHandle )
	{
		debugf( NAME_SourceControl, TEXT("Shutting down.") );
		SCC_Uninitialize( Context );
		appFreeDllHandle( LibraryHandle );
		LibraryHandle = NULL;
	}
}

FSourceControlIntegration* FSourceControlIntegration::GetGlobalSourceControlIntegration()
{
	return GSourceControlIntegration;
}

/** 
	*An interrupt function to delete the GSourceControlIntegration 
	* singleton if we get a control-c when in a console program 
	*/
INT WINAPI FSourceControlIntegration::ConsoleInterruptHandler(DWORD /*ControlType*/)
{
	// if we get a control-c, we need to delete the singleton before killing the application so that 
	// it doesn't crash trying to use the global free, since it is constructed with our malloc
	delete FSourceControlIntegration::GetGlobalSourceControlIntegration();
	return FALSE;
}

/**
 * Gets a string out of the registry.
 *
 * @param	InMainKey	The primary key
 * @param	InKey		The secondary key
 *
 * @return	A char* buffer containing the string.  The caller is reponsible for freeing this!
 */
char* FSourceControlIntegration::GetRegistryString( char* InMainKey, char* InKey )
{
	HKEY Key = 0;
	char *Buffer = NULL;

	if( RegOpenKeyExA( HKEY_LOCAL_MACHINE, InMainKey, 0, KEY_READ, &Key ) == ERROR_SUCCESS )
	{
		DWORD Size = 0;

		// First, we'll call RegQueryValueEx to find out how large of a buffer we need

		RegQueryValueExA( Key, InKey, NULL, NULL, NULL, &Size );

		if( Size )
		{
			// Allocate a buffer to hold the value and call the function again to get the data

			Buffer = new char[Size];

			RegQueryValueExA( Key, InKey, NULL, NULL, (LPBYTE)Buffer, &Size );
		}

		RegCloseKey( Key );
	}

	return Buffer;
}

/**
 * Attempts to find the users SCC system, load the library associated with it and bind the functions.
 */
void FSourceControlIntegration::Init()
{
	bEnabled = 0;
	LibraryHandle = NULL;
	bOpen = 0;

	LoadFromINI();

	if (bDisabled)
		return;

	// Try to cache the firewall client if possible
	appGetDllHandle(TEXT("mswsock.dll"));
	appGetDllHandle(TEXT("dnsapi.dll"));
	appGetDllHandle(TEXT("winrnr.dll"));
	appGetDllHandle(TEXT("hnetcfg.dll"));
	appGetDllHandle(TEXT("wshtcpip.dll"));
	// Look up the path to the firewall stuff
	char* Buffer1 = GetRegistryString( "SOFTWARE\\Microsoft\\Firewall Client 2004", "InstallRoot" );
	if (Buffer1 != NULL)
	{
		FString Path(Buffer1);
		delete [] Buffer1;
		FString Path2(Path);
		Path += TEXT("FwcWsp.dll");
		Path2 += TEXT("FwcRes.dll");
		appGetDllHandle(*Path);
		appGetDllHandle(*Path2);
	}

	/**
	 * - Get the default SCC provider location from the reg key: HKEY_LOCAL_MACHINE\Software\SourceCodeControlProvider.
	 *
	 * - That value (hereafter called "SCCProviderLocation") is the location in the registry of the value we really want.  A new key
	 *   is then pieced together like this: HKEY_LOCAL_MACHINE\"SCCProviderLocation"\ProviderRegKey.  The value at that location
	 *   is the full path to the SCC providers DLL.
	 *
	 * - That DLL is loaded and the function pointers are bound to it.
	 */

	char* Buffer = GetRegistryString( STR_SCC_PROVIDER_REG_LOCATION, STR_PROVIDERREGKEY );

	if( Buffer )
	{
		char* DLLName = GetRegistryString( Buffer, STR_SCCPROVIDERPATH );

		if( DLLName )
		{
			LibraryHandle = appGetDllHandle( ANSI_TO_TCHAR( DLLName ) );
			if( !LibraryHandle )
			{
				debugf( NAME_Init, TEXT("Source Control : Couldn't locate %s - giving up.  Integration will be disabled."), ANSI_TO_TCHAR( DLLName ) );
				return;
			}

			// Bind function pointers

			SCC_GetVersion = (FuncSccGetVersion)appGetDllExport( LibraryHandle, TEXT("SccGetVersion") );
			SCC_Initialize = (FuncSccInitialize)appGetDllExport( LibraryHandle, TEXT("SccInitialize") );
			SCC_Uninitialize = (FuncSccUninitialize)appGetDllExport( LibraryHandle, TEXT("SccUninitialize") );
			SCC_GetProjPath = (FuncSccGetProjPath)appGetDllExport( LibraryHandle, TEXT("SccGetProjPath") );
			SCC_OpenProject = (FuncSccOpenProject)appGetDllExport( LibraryHandle, TEXT("SccOpenProject") );
			SCC_CloseProject = (FuncSccCloseProject)appGetDllExport( LibraryHandle, TEXT("SccCloseProject") );
			SCC_QueryInfo = (FuncSccQueryInfo)appGetDllExport( LibraryHandle, TEXT("SccQueryInfo") );
			SCC_History = (FuncSccHistory)appGetDllExport( LibraryHandle, TEXT("SccHistory") );
			SCC_CheckOut = (FuncSccCheckOut)appGetDllExport( LibraryHandle, TEXT("SccCheckout") );
			SCC_UncheckOut = (FuncSccUncheckOut)appGetDllExport( LibraryHandle, TEXT("SccUncheckout") );
			SCC_CheckIn = (FuncSccCheckIn)appGetDllExport( LibraryHandle, TEXT("SccCheckin") );
			SCC_Add = (FuncSccAdd)appGetDllExport( LibraryHandle, TEXT("SccAdd") );
			SCC_Remove = (FuncSccRemove)appGetDllExport( LibraryHandle, TEXT("SccRemove") );
			SCC_Rename = (FuncSccRename)appGetDllExport( LibraryHandle, TEXT("SccRename") );

			// If we have all the bindings we need, source control integration is available.

			if( SCC_GetVersion
					&& SCC_Initialize
					&& SCC_Uninitialize
					&& SCC_GetProjPath
					&& SCC_OpenProject
					&& SCC_CloseProject
					&& SCC_QueryInfo
					&& SCC_History
					&& SCC_CheckOut
					&& SCC_UncheckOut
					&& SCC_CheckIn
					&& SCC_Add
					&& SCC_Remove
					&& SCC_Rename
					)
			{
				// Initialize

				if( SCC_Initialize( &Context, GetParentWindow(), "UnrealEd", ProviderName, &Caps, Label, &CheckOutCommentLen, &CommentLen ) == SCC_OK )
				{
					bSupportsRename = (Caps & SCC_CAP_RENAME) != 0;
					LONG ver = SCC_GetVersion();
					INT Lo = LOWORD(ver), Hi = HIWORD(ver);

					char* ProviderName = GetRegistryString( Buffer, STR_SCCPROVIDERNAME );

					debugf( NAME_SourceControl, TEXT("Using provider - %s (API version: %d.%d)"), ANSI_TO_TCHAR(ProviderName), Lo, Hi );
					delete [] ProviderName;

					// Source control is ready for use!

					bEnabled = 1;
				}
				else
				{
					debugf( NAME_SourceControl, TEXT("Initialization failed!  Integration will be disabled.") );
				}
			}
			else
			{
				debugf( NAME_SourceControl, TEXT("Function bindings failed!  Integration will be disabled.") );
			}

			delete [] DLLName;
		}

		delete [] Buffer;
	}

	// make sure any Control-C handler is restored
	appSetConsoleInterruptHandler();
	// install our own Control-C handler to delete the SCC object
	// NOTE: This HAS to occur after the call to appSetConsoleInterruptHandler()
	SetConsoleCtrlHandler(ConsoleInterruptHandler, TRUE);
}

/**
 * Called when the user wants to connect to a different project.  Will pop
 * up the connection dialog and let them enter new info.
 */
void FSourceControlIntegration::GetProjPath()
{
	if( !IsEnabled() )
	{
		return;
	}

	// Get saved values from INI

	LoadFromINI();
	
	// Attempt to get info/log in.  If this fails, turn off integration.

	INT bNew = 0;
	if( SCC_GetProjPath( Context, GetParentWindow(), UserName, ProjectPath, LocalPath, AuxProjectPath, 0, &bNew ) != SCC_OK )
	{
		debugf( NAME_SourceControl, TEXT("GetProjPath() failed - Integration will be disabled.") );
		bEnabled = 0;
	}
	else
	{
		// Write values back out to the INI

		SaveToINI();
	}
}

/**
 * Opens a project connection with the SCC.  Also handles logging the user in if they haven't been already.
 */
void FSourceControlIntegration::Open()
{
	if( !IsEnabled() || bOpen == TRUE )
	{
		return;
	}
	
	// Get saved values from INI
	LoadFromINI();
	
	// Attempt to get info/log in.  If this fails, turn off integration.
	SCCRTN ret = SCC_OpenProject( Context, GetParentWindow(), UserName, ProjectPath, LocalPath, AuxProjectPath, "", SCCTextOutput, SCC_OP_SILENTOPEN );
	if( ret != SCC_OK )
	{
		debugf( NAME_SourceControl, TEXT("Open() failed - Integration will be disabled.") );
		bEnabled = FALSE;
	}
	else
	{
		// Write values back out to the INI
		SaveToINI();
		bOpen = TRUE;
	}
}

/**
 * Closes the current project connection.
 */
void FSourceControlIntegration::Close()
{
	if( !IsEnabled() )
	{
		return;
	}

	// Project has to be open before calling this function.
	check( bOpen );

	SCC_CloseProject( Context );

	bOpen = 0;
}

/**
 * Loads user/SCC information from the INI file.
 */
void FSourceControlIntegration::LoadFromINI()
{
	// Init the values to their defaults

	ZeroMemory( UserName, SCC_USER_LEN );
	appStrcpyANSI( LocalPath, TCHAR_TO_ANSI(appBaseDir()) );
	ZeroMemory( ProjectPath, SCC_PRJPATH_LEN );
	ZeroMemory( AuxProjectPath, SCC_PRJPATH_LEN );

	// Attempt to read values from the INI file, only replacing the defaults if the value exists in the INI.

	FString Wk;

	if( GConfig->GetString( TEXT("SourceControl"), TEXT("UserName"), Wk, GEditorUserSettingsIni ) )
	{
		appStrcpyANSI( UserName, TCHAR_TO_ANSI( *Wk ) );
	}

	if( GConfig->GetString( TEXT("SourceControl"), TEXT("LocalPath"), Wk, GEditorUserSettingsIni ) )
	{
		appStrcpyANSI( LocalPath, TCHAR_TO_ANSI( *Wk ) );
	}

	if( GConfig->GetString( TEXT("SourceControl"), TEXT("ProjectPath"), Wk, GEditorUserSettingsIni ) )
	{
		appStrcpyANSI( ProjectPath, TCHAR_TO_ANSI( *Wk ) );
	}

	if( GConfig->GetString( TEXT("SourceControl"), TEXT("AuxProjectPath"), Wk, GEditorUserSettingsIni ) )
	{
		appStrcpyANSI( AuxProjectPath, TCHAR_TO_ANSI( *Wk ) );
	}

	bDisabled = false;
	GConfig->GetBool(TEXT("SourceControl"), TEXT("Disabled"), bDisabled, GEditorUserSettingsIni );
}

/**
 * Stores user/SCC information in the INI file.  This saves the user from having to log in every
 * time they start up the editor.
 */
void FSourceControlIntegration::SaveToINI()
{
	GConfig->SetString( TEXT("SourceControl"), TEXT("UserName"), ANSI_TO_TCHAR( UserName ), GEditorUserSettingsIni );
	GConfig->SetString( TEXT("SourceControl"), TEXT("LocalPath"), ANSI_TO_TCHAR( LocalPath ), GEditorUserSettingsIni );
	GConfig->SetString( TEXT("SourceControl"), TEXT("ProjectPath"), ANSI_TO_TCHAR( ProjectPath ), GEditorUserSettingsIni );
	GConfig->SetString( TEXT("SourceControl"), TEXT("AuxProjectPath"), ANSI_TO_TCHAR( AuxProjectPath ), GEditorUserSettingsIni );
	GConfig->SetBool(	TEXT("SourceControl"), TEXT("Disabled"), bDisabled, GEditorUserSettingsIni );
}

/**
 * Creates a fully qualified filename, including the path all the way back to the drive letter,
 * for the specific package.
 *
 * @param	PackageName	The name of the package (like "EngineMaterials")
 *
 * @return	The fully qualified filename
 */
FString FSourceControlIntegration::GetFullFilename(const TCHAR* PackageName)
{
	// Get the package's filename on the disk
	FFilename Filename;
	if (IsEnabled() && GPackageFileCache->FindPackageFile(PackageName, NULL, Filename))
	{
		return appConvertRelativePathToFull(Filename);
	}
	else
	{
		return TEXT("");
	}
}

/**
 * Creates a fully qualified filename, including the path all the way back to the drive letter,
 * for the specific package.
 *
 * @param	InPackage	The package we need the filename for
 *
 * @return	The fully qualified filename
 */
FString FSourceControlIntegration::GetFullFilename(UObject* InPackage)
{
	return GetFullFilename(*InPackage->GetOutermost()->GetName());
}

/**
 * Convert a SCC status code to a ESourceControlState
 *
 * @param StatusCode Status code received from SCC operation
 * 
 * @param The Unreal source control state associated with the status code
 */
ESourceControlState FSourceControlIntegration::GetSCCStateFromStatusCode(LONG StatusCode)
{
	// default to unknown
	ESourceControlState State = SCC_DontCare;

	// is it already checked out?
	if (StatusCode & SCC_STATUS_OUTBYUSER)
	{
		State = SCC_CheckedOut;
	}
	else
	{
		// if not, default to checked-outable
		State = SCC_ReadOnly;

		// old revision?
		if (StatusCode & SCC_STATUS_OUTOFDATE)
		{
            State =  SCC_NotCurrent;
		}

		// no status code?
		if (!StatusCode)
		{
			State = SCC_NotInDepot;
		}

		// does someone else have it out?
		if (StatusCode & SCC_STATUS_OUTOTHER)
		{
			State = SCC_CheckedOutOther;
		}
	}

	return State;
}

/**
 * Updates the status flags for a set of packages.
 *
 * @param	InPackages	The packages to update status flags for
 */
void FSourceControlIntegration::Update(const TArray<UPackage*>& InPackages)
{
	Open();

	if (!IsEnabled())
	{
		return;
	}

	INT NumFiles = 0;
	TArray<UPackage*> Packages;
	// get a list of packages and their filenames for a single SCC operation for many packages
	char** Filenames = GetFilenamesFromArray(InPackages, NumFiles, &Packages);

	LONG* StatusCodes = new LONG[NumFiles];
	if (SCC_QueryInfo(Context, NumFiles, (LPCSTR*)Filenames, StatusCodes ) == SCC_OK )
	{
		for (INT PackageIndex = 0; PackageIndex < NumFiles; PackageIndex++)
		{
			// set the status for each pacakge
			Packages(PackageIndex)->SetSCCState(GetSCCStateFromStatusCode(StatusCodes[PackageIndex]));
		}
	}

	// cleanup
	delete [] StatusCodes;
	FreeFilenameArray(Filenames, NumFiles);
}

/**
 * Get source control status of a file by filename
 *
 * @param PackageName THe name of the package to query
 * 
 * @return The current source control status of the file (will return SCC_DontCare for a bad filename)
 */
ESourceControlState FSourceControlIntegration::GetSCCState(const TCHAR* PackageName)
{
	Open();

	if (!IsEnabled())
	{
		return SCC_DontCare;
	}

	// default to unknown state
	ESourceControlState State = SCC_DontCare;

	// convert package name to a usable SCC filename
	FString FullName = GetFullFilename(PackageName);

	// make sure it succeeded
	if (FullName.Len())
	{
		FTCHARToANSI Convert(*FullName);
		char* Filenames[2] = { (ANSICHAR*)Convert, NULL };
		LONG StatusCode;

		if (SCC_QueryInfo(Context, 1, (LPCSTR*)Filenames, &StatusCode) == SCC_OK)
		{
			// convert status to a state value
			State = GetSCCStateFromStatusCode(StatusCode);
		}
	}

	return State;
}

/**
 * Cleaunup a list of packages to be just those packages that can be
 * used with source control operations
 *
 * @param InPackages List of packages to clean out
 * @param OutPackages Output list of packages. CANNOT be the same as InPacakges
 */
void FSourceControlIntegration::CleanupPackageArray(const TArray<UPackage*>& InPackages, TArray<UPackage*>& OutPackages)
{
	// duplicate the array so we don't overwrite input
	TArray<UPackage*> Packages = InPackages;

	// Prune the package list, removing any packages that we don't care about in terms of the SCC.
	for (INT PackageIndex = 0; PackageIndex < Packages.Num(); PackageIndex++)
	{
		UPackage* Package = Packages(PackageIndex);
		const FString Filename = GetFullFilename(Package);
		if (Filename.Len())
		{
			// - must be a top level package
			// - must not be named "Transient"
			// - filename on hard drive must not end with ".u"

			const FString PkgName = Package->GetName();
			const UBOOL bIsTopLevelPackage = (Package->GetOuter() == NULL);
			const UBOOL bShouldRemove = (!bIsTopLevelPackage
											|| !appStricmp(*PkgName, TEXT("Transient"))
											|| !appStricmp(*Filename.Right(2), TEXT(".u")));
			if ( bShouldRemove )
			{
				Packages.Remove(PackageIndex);
				// fixup the counter
				PackageIndex--;
			}
		}
		else
		{
			// If a filename can't be generated for this package, that means it has an invalid linker - which means
			// that it probably doesn't exist on the hard drive yet.
			Package->SetSCCState( SCC_NotInDepot );
			Packages.Remove(PackageIndex);
			// fixup the counter
			PackageIndex--;
		}
	}

	// copy the result back out
	OutPackages = Packages;
}

/**
 * Creates an array of ANSI filenames from an array of packages.
 *
 * @param	InSrcPackages	The packages to get the filenames for
 * @param	OutNumFiles		The number of filenames returned will be placed here
 * @param	InPackages		If this is non-NULL, this will be filled with the packages which correspond to each filename
 *
 * @return	A pointer to an array of ANSI filenames
 */
char** FSourceControlIntegration::GetFilenamesFromArray(const TArray<UPackage*>& InSrcPackages, INT& OutNumFiles, TArray<UPackage*>* InPackages )
{
	if( !IsEnabled() )
	{
		return NULL;
	}

	TArray<UPackage*> SrcPackages;
	// remove unusable entries
	CleanupPackageArray(InSrcPackages, SrcPackages);

	OutNumFiles = SrcPackages.Num();
	char** Filenames = NULL;

	if( SrcPackages.Num() )
	{
		// Build the array of filenames

		Filenames = new char*[SrcPackages.Num()];

		FString Wk;
		for( INT x = 0 ; x < SrcPackages.Num() ; ++x )
		{
			Wk = *GetFullFilename( SrcPackages(x) );
			Filenames[x] = new char[ Wk.Len()+1 ];
			appStrcpyANSI( Filenames[x], Wk.Len()+1, TCHAR_TO_ANSI( *Wk ) );

			if( InPackages )
			{
				InPackages->AddItem( SrcPackages(x) );
			}
		}
	}

	return Filenames;
}

/**
 * Frees memory previously allocated in GetFilenamesFromArray
 * 
 * @param FilenameArray Array of char*'s returned from GetFilenamesFromArray
 * @param NumFilenames Size of FilenameArray
 */
void FSourceControlIntegration::FreeFilenameArray(char** FilenameArray, INT NumFilenames)
{
	for (INT FilenameIndex = 0; FilenameIndex < NumFilenames; FilenameIndex++)
	{
		delete [] FilenameArray[FilenameIndex];
	}
	delete [] FilenameArray;
}

/**
 * Displays the revision history for the specified set of packages.
 */
void FSourceControlIntegration::ShowHistory(const TArray<UPackage*>& Packages)
{
	Open();

	if( !IsEnabled() )
	{
		return;
	}

	INT NumFiles = 0;
	char** Filenames = GetFilenamesFromArray(Packages, NumFiles, NULL);
	//char** Filenames = GetFilenames( NumFiles );

	if( Filenames )
	{
		SCCRTN ret = SCC_History( Context, GetParentWindow(), NumFiles, (LPCSTR*)Filenames, SCC_COMMAND_HISTORY, NULL );

		switch( ret )
		{
			case SCC_I_RELOADFILE:
				check(0);
				break;

			default:
				break;
		}

		// Clean up
		FreeFilenameArray(Filenames, NumFiles);
	}
}

/**
 * Checks a file out of the SCC depot.
 */
void FSourceControlIntegration::CheckOut(const TCHAR* PackageName)
{
	Open();

	if (!IsEnabled())
	{
		return;
	}

	FString FullName = GetFullFilename(PackageName);
	if (FullName.Len())
	{
		FTCHARToANSI Convert(*FullName);
		char* Filenames[2] = { (ANSICHAR*)Convert, NULL };

		SCC_CheckOut(Context, GetParentWindow(), 1, (LPCSTR*)Filenames, NULL, 0, NULL);
	}
}

/**
 * Checks a package file out of the SCC depot.
 */
void FSourceControlIntegration::CheckOut(UObject* InPackage)
{
	check(InPackage);
	CheckOut(*InPackage->GetOutermost()->GetName());
}

/**
 * Reverts a file checked out of the SCC depot.
 */
void FSourceControlIntegration::UncheckOut(const TCHAR* PackageName)
{
	Open();

	if( !IsEnabled() )
	{
		return;
	}

	FString FullName = GetFullFilename(PackageName);
	if (FullName.Len())
	{
		FTCHARToANSI Convert(*FullName);
		char* Filenames[2] = { (ANSICHAR*)Convert, NULL };

		SCC_UncheckOut(Context, GetParentWindow(), 1, (LPCSTR*)Filenames, 0, NULL);
	}
}

/**
 * Reverts a package file checked out of the SCC depot.
 */
void FSourceControlIntegration::UncheckOut(UObject* InPackage)
{
	check(InPackage);
	UncheckOut(*InPackage->GetOutermost()->GetName());
}

/**
 * Checks a file into the SCC depot.
 */
void FSourceControlIntegration::CheckIn(const TCHAR* PackageName)
{
	Open();

	if( !IsEnabled() )
	{
		return;
	}

	FString FullName = GetFullFilename(PackageName);
	if (FullName.Len())
	{
		FTCHARToANSI Convert(*FullName);
		char* Filenames[2] = { (ANSICHAR*)Convert, NULL };

		SCC_CheckIn(Context, GetParentWindow(), 1, (LPCSTR*)Filenames, NULL, 0, NULL);
	}
}

/**
 * Checks a package file into the SCC depot.
 */
void FSourceControlIntegration::CheckIn(UObject* InPackage)
{
	check(InPackage);
	CheckIn(*InPackage->GetOutermost()->GetName());
}

/**
 * Adds a new file to the depot.
 *
 * A "gotcha" here is that the file must exist on the hard drive before this function will work (obviously),
 * so new files must be saved out before this will work.
 */
void FSourceControlIntegration::Add(const TCHAR* PackageName, UBOOL bIsBinary)
{
	Open();

	if( !IsEnabled() )
	{
		return;
	}

	FString FullName = GetFullFilename(PackageName);
	if (FullName.Len())
	{
		FTCHARToANSI Convert(*FullName);
		char* Filenames[2] = { (ANSICHAR*)Convert, NULL };
		LONG Flags[] = { bIsBinary ? SCC_FILETYPE_BINARY : SCC_FILETYPE_TEXT };

		SCC_Add(Context, GetParentWindow(), 1, (LPCSTR*)Filenames, "", Flags, NULL);
	}
}

/**
 * Adds a package new file to the depot.
 *
 * A "gotcha" here is that the file must exist on the hard drive before this function will work (obviously),
 * so new packages must be saved out before this will work.
 */
void FSourceControlIntegration::Add( UObject* InPackage )
{
	check( InPackage );
	Add(*InPackage->GetOutermost()->GetName());
}


/**
 * Removes a file from the depot.
 */
void FSourceControlIntegration::Remove(const TCHAR* PackageName)
{
	Open();

	if( !IsEnabled() )
	{
		return;
	}

	FString FullName = GetFullFilename(PackageName);
	if (FullName.Len())
	{
		FTCHARToANSI Convert(*FullName);
		char* Filenames[2] = { (ANSICHAR*)Convert, NULL };

		SCC_Remove(Context, GetParentWindow(), 1, (LPCSTR*)Filenames, "", 0, NULL);
	}
}

/**
 * Removes a package file from the depot.
 */
void FSourceControlIntegration::Remove(UObject* InPackage)
{
	check(InPackage);
	Remove(*InPackage->GetOutermost()->GetName());
}

/**
 * Renames a file in the depot.
 */
void FSourceControlIntegration::Rename(const TCHAR* SourceFilename, const TCHAR* DestinationFilename)
{
	Open();

	if( !IsEnabled() )
	{
		return;
	}

	if (bSupportsRename)
	{
		if (*SourceFilename && *DestinationFilename)
		{
			SCC_Rename(Context, GetParentWindow(), (LPSTR)TCHAR_TO_ANSI(DestinationFilename), (LPSTR)TCHAR_TO_ANSI(SourceFilename));
		}
	}
	else
	{
		// attempt to run p4 manually, since it doesn't support Rename over SCC
//		FString PerforceBranchCommand = TEXT("p4 rename 
//		FString PerforceDeleteCommand = TEXT("p4 rename 
	}
}

/**
 * Moves the specified package file to the trashcan director.
 */
void FSourceControlIntegration::MoveToTrash( UObject* InPackage )
{
	FFilename PackageFilename = GetFullFilename(InPackage);
	FFilename TrashFilename = appConvertRelativePathToFull(appGameDir() * TRASHCAN_DIRECTORY_NAME * PackageFilename.GetCleanFilename());

	// @todo: check for success, and mark the package as PKG_Trash!
	Rename(*PackageFilename, *TrashFilename);
}

#endif // HAVE_SCC
