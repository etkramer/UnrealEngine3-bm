#include <Windows.h>
#include <shlobj.h>
#include <richedit.h>

#include "resource.h"

// Full version {INT, FRA, ESN, DEU, ITA}
static const wchar_t* INSTALLATION_KEY_x64 = L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\InstallShield_{BFA90209-7AFF-4DB6-8E4B-E57305751AD7}";
static const wchar_t* INSTALLATION_KEY_x86 = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\InstallShield_{BFA90209-7AFF-4DB6-8E4B-E57305751AD7}";

// Dedicated server {INT}
static const wchar_t* INSTALLATION_KEY_x64_DEDICATED = L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\InstallShield_{69082C6E-1944-4EAD-B119-06DCBF492C3F}";
static const wchar_t* INSTALLATION_KEY_x86_DEDICATED = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\InstallShield_{69082C6E-1944-4EAD-B119-06DCBF492C3F}";

// Low gore version {DEU}
static const wchar_t* INSTALLATION_KEY_x64_LG = L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\InstallShield_{FDBBAF14-5ED8-49B7-A5BE-1C35668B074D}";
static const wchar_t* INSTALLATION_KEY_x86_LG = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\InstallShield_{FDBBAF14-5ED8-49B7-A5BE-1C35668B074D}";

// Russian version {RUS}
static const wchar_t* INSTALLATION_KEY_x64_RUS = L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\InstallShield_{A007C579-B78D-4FDE-A85A-16987A251E53}";
static const wchar_t* INSTALLATION_KEY_x86_RUS = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\InstallShield_{A007C579-B78D-4FDE-A85A-16987A251E53}";

// Eastern European verions {POL, HUN, CZE}
static const wchar_t* INSTALLATION_KEY_x64_EE = L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\InstallShield_{C019D439-E7F8-49EB-85FA-6D0C8CCBDA23}";
static const wchar_t* INSTALLATION_KEY_x86_EE = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\InstallShield_{C019D439-E7F8-49EB-85FA-6D0C8CCBDA23}";

// define when building installer for bonus pack instead of patch (changes text strings)
#define BONUS_PACK 0
#if BONUS_PACK
	static const wchar_t* PROGRESS_TITLE = L"Copying bonus pack files ...";
	static const wchar_t* APPLY_MSG = L"Would you like to install the bonus pack to\n'%s'?";
	static const wchar_t* MESSAGEBOX_TITLE = L"UT3 Bonus Pack";
	static const wchar_t* APPLY_FAILED_MSG = L"Failed to install bonus pack!";
	static const wchar_t* NOT_APPLIED_MSG = L"The bonus pack was not installed!";
	static const wchar_t* SUCCESS_MSG = L"The UT3 bonus pack was successfully installed!";
#else
	static const wchar_t* PROGRESS_TITLE = L"Copying patch files ...";
	static const wchar_t* APPLY_MSG = L"Would you like to apply the patch to\n'%s'?";
	static const wchar_t* MESSAGEBOX_TITLE = L"Patch UT3";
	static const wchar_t* APPLY_FAILED_MSG = L"Failed to apply patch!";
	static const wchar_t* NOT_APPLIED_MSG = L"The patch was not applied!";
	static const wchar_t* SUCCESS_MSG = L"UT3 was successfully patched!";
#endif

static const wchar_t* RegKeys[] =
{
	INSTALLATION_KEY_x64,
	INSTALLATION_KEY_x86,
	INSTALLATION_KEY_x64_DEDICATED,
	INSTALLATION_KEY_x86_DEDICATED,
	INSTALLATION_KEY_x64_LG,
	INSTALLATION_KEY_x86_LG,
	INSTALLATION_KEY_x64_RUS,
	INSTALLATION_KEY_x86_RUS,
	INSTALLATION_KEY_x64_EE,
	INSTALLATION_KEY_x86_EE,
};

// Check to see if a reg key exists and extracts the install location if it does
bool GetInstallLocation( const wchar_t* RegKeyName, wchar_t* InstallFolder )
{
	wchar_t KeyValue[MAX_PATH] = { 0 };
	DWORD Size = sizeof( KeyValue );
	HKEY Key = 0;

	memset( InstallFolder, 0, MAX_PATH * sizeof( wchar_t ) );

	if( RegOpenKeyExW( HKEY_CURRENT_USER, RegKeyName, 0, KEY_READ, &Key ) == ERROR_SUCCESS )
	{
		RegQueryValueExW( Key, L"InstallLocation", NULL, NULL, ( LPBYTE )&KeyValue, &Size );
		RegCloseKey( Key );

		wcscpy_s( InstallFolder, MAX_PATH, KeyValue );
	}	

	return( wcslen( InstallFolder ) > 0 );
}

bool FolderExists( const wchar_t* Folder )
{
	DWORD dwAttr = GetFileAttributes( Folder );
	return( dwAttr != 0xffffffff && ( dwAttr & FILE_ATTRIBUTE_DIRECTORY ) );
}

// Makes sure the selected folder has Binaries, Engine and UTGame
bool ValidateFolder( const wchar_t* Folder )
{
	wchar_t TestFolder[MAX_PATH] = { 0 };

	wsprintf( TestFolder, L"%s\\Binaries", Folder );
	if( !FolderExists( TestFolder ) )
	{
		return( false );
	}

	wsprintf( TestFolder, L"%s\\Engine", Folder );
	if( !FolderExists( TestFolder ) )
	{
		return( false );
	}

	wsprintf( TestFolder, L"%s\\UTGame", Folder );
	if( !FolderExists( TestFolder ) )
	{
		return( false );
	}

	return( true );
}

// Browse for a folder
bool SelectFolder( wchar_t* Folder )
{
    LPITEMIDLIST pidlSelected = NULL;
    BROWSEINFO bi = { 0 };

    bi.hwndOwner = NULL;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = NULL;
    bi.lpszTitle = L"Please browse to where UT3 is installed";
    bi.ulFlags = BIF_EDITBOX | BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;
    bi.lpfn = NULL;
    bi.lParam = 0;

    pidlSelected = SHBrowseForFolder( &bi );

	return( !!SHGetPathFromIDList( pidlSelected, Folder ) );
}

void GetStartupPath( wchar_t* Path )
{
	GetModuleFileName( NULL, Path, MAX_PATH );
	for( size_t i = wcslen( Path ) - 1; i >= 0; i-- )
	{
		if( Path[i] == '\\' )
		{
			return;
		}
		Path[i] = 0;
	}
}

// Copy the files over to the patch
bool ApplyPatch( const wchar_t* Folder )
{
	wchar_t CurrentDir[MAX_PATH] = { 0 };
	wchar_t From[MAX_PATH] = { 0 };
	wchar_t To[MAX_PATH] = { 0 };
	SHFILEOPSTRUCT FileOp = { 0 };

	GetStartupPath( CurrentDir );

	wsprintf( From, L"%s\\Patch\\*.*", CurrentDir );
	wcscpy_s( To, MAX_PATH, Folder );

	FileOp.hwnd = NULL;
	FileOp.wFunc = FO_COPY;
	FileOp.pFrom = From;
	FileOp.pTo = To;
	FileOp.fFlags = FOF_NOCONFIRMMKDIR | FOF_NOCONFIRMATION;
	FileOp.lpszProgressTitle = PROGRESS_TITLE;

	return( SHFileOperation( &FileOp ) == 0 );
}

// Validate the destination folder and start the file copy
bool ValidateAndCopy( wchar_t* Folder )
{
	wchar_t Message[MAX_PATH] = { 0 };

	if( ValidateFolder( Folder ) )
	{
		wsprintf( Message, APPLY_MSG, Folder );
		if( MessageBox( NULL, Message, MESSAGEBOX_TITLE, MB_YESNO | MB_TOPMOST ) == IDYES )
		{
			return( ApplyPatch( Folder ) );
		}
	}
	else
	{
		wsprintf( Message, L"Unreal Tournament 3 does not seem to be installed at '%s'", Folder );
		MessageBox( NULL, Message, MESSAGEBOX_TITLE, MB_OK | MB_TOPMOST );
	}

	return( false );
}

void DeletePatchFiles( void )
{
	wchar_t CurrentDir[MAX_PATH] = { 0 };
	wchar_t From[MAX_PATH] = { 0 };
	SHFILEOPSTRUCT FileOp = { 0 };

	GetStartupPath( CurrentDir );

	wsprintf( From, L"%s\\Patch\\*.*", CurrentDir );

	FileOp.hwnd = NULL;
	FileOp.wFunc = FO_DELETE;
	FileOp.pFrom = From;
	FileOp.pTo = NULL;
	FileOp.fFlags = FOF_NOCONFIRMMKDIR | FOF_NOCONFIRMATION;
	FileOp.lpszProgressTitle = L"Deleting temp files ...";

	SHFileOperation( &FileOp );
}

DWORD CALLBACK EditStreamCallback( DWORD_PTR dwCookie, LPBYTE lpBuff, LONG cb, PLONG pcb )
{
	HANDLE hFile = ( HANDLE )dwCookie;
	return( !ReadFile( hFile, lpBuff, cb, ( DWORD* )pcb, NULL ) );
}

INT_PTR CALLBACK EULADialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM )
{
	HANDLE hFile;
	HWND RTB = GetDlgItem( hwndDlg, IDC_EULA_TEXT );

	switch( uMsg )
	{
	case WM_INITDIALOG:
		// Enable word wrap
		SendMessage( RTB, EM_SETTARGETDEVICE, 0, 0 );

		// Yes our EULA does require this much space!
		SendMessage( RTB, ( UINT )EM_EXLIMITTEXT, 0, 512 * 1024 );  

		// Stream in the RTF file to the edit box
		hFile = CreateFile( L"EULA.rtf", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
		if( hFile != INVALID_HANDLE_VALUE ) 
		{
			EDITSTREAM es = { ( DWORD_PTR )hFile, 0, EditStreamCallback };
			SendMessage( RTB, EM_STREAMIN, SF_RTF, ( LPARAM )&es );
			CloseHandle( hFile );		
		}
		return( TRUE );

	case WM_COMMAND:
		if( wParam == IDOK || wParam == IDCANCEL )
		{
			EndDialog( hwndDlg, wParam );
		}
		return( TRUE );

	case WM_CLOSE:
		EndDialog( hwndDlg, IDCANCEL );
		return( TRUE );
	};

	return( FALSE );
}

bool AcceptEULA( void )
{
	LoadLibraryA( "RICHED20.DLL" );

	return( DialogBox( NULL, MAKEINTRESOURCE( IDD_EULA ), NULL, EULADialogProc ) == IDOK );
}

// Main program
int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
	wchar_t UT3Folder[MAX_PATH] = { 0 };
	bool PatchApplied = false;

	CoInitialize( NULL );

	if( AcceptEULA() )
	{
		for( int i = 0; i < sizeof( RegKeys ) / sizeof( const wchar_t* ); i++ )
		{
			if( GetInstallLocation( RegKeys[i], UT3Folder ) )
			{
				if( !ValidateAndCopy( UT3Folder ) )
				{
					MessageBox( NULL, APPLY_FAILED_MSG, MESSAGEBOX_TITLE, MB_OK | MB_TOPMOST );
				}
				else
				{
					PatchApplied = true;
				}
			}
		}

		// Backup should an instance not be found
		if( !PatchApplied )
		{
			if( SelectFolder( UT3Folder ) )
			{
				if( !ValidateAndCopy( UT3Folder ) )
				{
					MessageBox( NULL, APPLY_FAILED_MSG, MESSAGEBOX_TITLE, MB_OK | MB_TOPMOST );
				}
				else
				{
					PatchApplied = true;
				}
			}
		}
	}

	if( !PatchApplied )
	{
		MessageBox( NULL, NOT_APPLIED_MSG, MESSAGEBOX_TITLE, MB_OK | MB_TOPMOST );
	}
	else
	{
		MessageBox( NULL, SUCCESS_MSG, MESSAGEBOX_TITLE, MB_OK | MB_TOPMOST );
	}

	// Delete temp files
	DeletePatchFiles();

	CoUninitialize();

	return( 0 );
}
