//-----------------------------------------------------------------------------
// File: GDFInstall.cpp
//
// Desc: Windows code that calls GameuxInstallHelper sample dll and displays the results.
//
// (C) Copyright Microsoft Corp.  All rights reserved.
//-----------------------------------------------------------------------------
#define _WIN32_DCOM
#define _CRT_SECURE_NO_DEPRECATE
#include <rpcsal.h>
#include <gameux.h>
#include "GameuxInstallHelper.h"
#include <strsafe.h>
#include <shlobj.h>
#include <wbemidl.h>
#include <objbase.h>
#define NO_SHLWAPI_STRFCNS
#include <shlwapi.h>

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif    
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif    
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif

#include "GameuxRichSavedGame.h"

typedef struct _EXAMPLE_SAVED_GAME_DATA
{
    DWORD       dwLevel;
    DWORD       dwHitPoints;
    WCHAR       szName[256];
    BYTE		levelData[1024];
} EXAMPLE_SAVED_GAME_DATA;

struct SETTINGS
{
    WCHAR strInstallPath[MAX_PATH];
    WCHAR strGDFBinPath[MAX_PATH];
    WCHAR strExePath[MAX_PATH];    
    WCHAR strRichSavedGameExt[MAX_PATH];        
    bool bUseMCLFile;
    bool bEnumMode;
    bool bUninstall;
    bool bAllUsers;
    bool bSkipMediaCenter;
    bool bSkipRichSavedGames;
    bool bSilent;
};

HRESULT WriteSavedGame( WCHAR* strSavedGameFileName, WCHAR* strExistingScreenshotPath );
HRESULT EnumAndRemoveGames();
bool    ParseCommandLine( SETTINGS* pSettings );
bool    IsNextArg( WCHAR*& strCmdLine, WCHAR* strArg );
void    DisplayUsage();


//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point to the program. Initializes everything, and pops
//       up a message box with the results of the GameuxInstallHelper calls
//-----------------------------------------------------------------------------
int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR strCmdLine, int nCmdShow )
{
    GUID guid = GUID_NULL;
    HRESULT hr;
    WCHAR szMsg[512];
    bool bFailure = false;

    SETTINGS settings;
    memset(&settings, 0, sizeof(SETTINGS));

    // Set defaults
    WCHAR strSysDir[MAX_PATH];
    GetSystemDirectory( strSysDir, MAX_PATH );
    GetCurrentDirectory( MAX_PATH, settings.strInstallPath );
    PathAddBackslash( settings.strInstallPath );
    StringCchPrintf( settings.strGDFBinPath, MAX_PATH, L"%sGDFExampleBinary.dll", settings.strInstallPath );
    StringCchPrintf( settings.strRichSavedGameExt, MAX_PATH, L".ExampleSaveGame" );   
    StringCchPrintf( settings.strExePath, MAX_PATH, L"%s\\notepad.exe", strSysDir );

    if( !ParseCommandLine( &settings ) )
    {
        return 0;
    }

    if( settings.bEnumMode )
    {
        EnumAndRemoveGames();
        return 0;
    }

    if( !IsUserAnAdmin() && settings.bAllUsers && !settings.bSilent )
    {
        MessageBox( NULL, L"Warning: GDFInstall.exe does not have administrator privileges.  Installing for all users will fail.\n\n"
                          L"To correct, right click on GDFInstall.exe and run it as an administrator.", L"GDFInstall", MB_OK );
    }

    if( !settings.bUninstall )
    {
        // Installing

        GAME_INSTALL_SCOPE installScope = ( settings.bAllUsers ) ? GIS_ALL_USERS : GIS_CURRENT_USER;

        if( !settings.bSkipMediaCenter )
        {
            RegisterWithMediaCenterW( settings.strGDFBinPath, settings.strInstallPath, installScope, 
                                      settings.strExePath, NULL, !settings.bUseMCLFile );
        }

        // Upon install do this
        // Change the paths in this call to be correct
        hr = AddToGameExplorer( settings.strGDFBinPath, settings.strInstallPath, installScope, &guid );
        if( FAILED(hr) )
        {
            StringCchPrintf( szMsg, 256, L"Adding game failed: 0x%0.8x\nNote: This will fail if the game has already been added.  Make sure the game is removed first.", hr );
            if( !settings.bSilent )
                MessageBox( NULL, szMsg, TEXT("AddToGameExplorer"), MB_OK | MB_ICONINFORMATION );
            bFailure = true;
        }

        // Create tasks as needed
        hr = CreateTaskW( installScope, &guid, false, 0, L"Play", settings.strExePath, NULL );
        hr |= CreateTaskW( installScope, &guid, false, 1, L"Multiplayer", settings.strExePath, L"-multiplayer" );
        hr |= CreateTaskW( installScope, &guid, true, 0, L"Home Page", L"http://www.microsoft.com", NULL );
        if( FAILED(hr) )
        {
            StringCchPrintf( szMsg, 256, L"Adding task failed: 0x%0.8x", hr );
            if( !settings.bSilent )
                MessageBox( NULL, szMsg, TEXT("CreateTaskW"), MB_OK | MB_ICONINFORMATION );
            bFailure = true;
        }

        if( !settings.bSkipRichSavedGames )
        {
            // Setup rich save game support 
            hr = SetupRichSavedGamesW( settings.strRichSavedGameExt, settings.strExePath, NULL );
            if( FAILED(hr) )
            {
                StringCchPrintf( szMsg, 256, L"Adding rich saved game support failed: 0x%0.8x", hr );
                if( !settings.bSilent )
                    MessageBox( NULL, szMsg, TEXT("SetupRichSavedGamesW"), MB_OK | MB_ICONINFORMATION );
                bFailure = true;
            }

            // Create an example rich saved game.  The game itself would do this normally when it saves
            WCHAR strSaveFileName[MAX_PATH];
            StringCchPrintf( strSaveFileName, MAX_PATH, L"Save1%s", settings.strRichSavedGameExt );       
            hr = WriteSavedGame( strSaveFileName, L"Thumbnail.jpg" );
            if( FAILED(hr) )
            {
                StringCchPrintf( szMsg, 256, L"Creating example saved game failed: 0x%0.8x", hr );
                if( !settings.bSilent )
                    MessageBox( NULL, szMsg, TEXT("WriteSavedGame"), MB_OK | MB_ICONINFORMATION );
                bFailure = true;
            }
        }

        if( !bFailure )
        {
            StringCchPrintf( szMsg, 512, L"GDF binary: %s\nPlay EXE: %s\nGDF Install path: %s\nAll users: %d\n\n", settings.strGDFBinPath, settings.strExePath, settings.strInstallPath, settings.bAllUsers );

            StringCchCat( szMsg, 512, L"Adding GDF binary succeeded\n" );
            if( !settings.bSkipMediaCenter )
                StringCchCat( szMsg, 512, L"Adding media center entry point succeeded\n" );
            if( !settings.bSkipRichSavedGames )
                StringCchCat( szMsg, 512, L"Adding rich saved game support succeeded\n" );
            StringCchCat( szMsg, 512, L"\nGDFInstall.exe /? for a list of options" );

            if( !settings.bSilent )
                MessageBox( NULL, szMsg, TEXT("AddToGameExplorer"), MB_OK | MB_ICONINFORMATION );
        }
        else
        {
            StringCchPrintf( szMsg, 512, L"GDF binary: %s\nPlay EXE: %s\nGDF Install path: %s\nAll users: %d\n\n", settings.strGDFBinPath, settings.strExePath, settings.strInstallPath, settings.bAllUsers );

            StringCchPrintf( szMsg, 512, L"Adding GDF binary failed\n" );
            StringCchCat( szMsg, 512, L"\nGDFInstall.exe /? for a list of options" );
            if( !settings.bSilent )
                MessageBox( NULL, szMsg, TEXT("AddToGameExplorer"), MB_OK | MB_ICONINFORMATION );
        }
    }
    else
    {
        // Uninstalling

        bFailure = false;

        hr = RetrieveGUIDForApplication( settings.strGDFBinPath, &guid );
        if( FAILED(hr) )
        {
            StringCchPrintf( szMsg, 256, L"Getting guid failed: 0x%0.8x", hr );
            if( !settings.bSilent )
                MessageBox( NULL, szMsg, TEXT("RetrieveGUIDForApplication"), MB_OK | MB_ICONINFORMATION );
            bFailure = true;
        }

        if( !settings.bSkipMediaCenter )
        {
            UnRegisterWithMediaCenterW( settings.strInstallPath, GIS_ALL_USERS, settings.strExePath, !settings.bUseMCLFile );
            UnRegisterWithMediaCenterW( settings.strInstallPath, GIS_CURRENT_USER, settings.strExePath, !settings.bUseMCLFile );
        }

        // Upon uninstall do this
        hr = RemoveFromGameExplorer( &guid );
        if( FAILED(hr) )
        {
            StringCchPrintf( szMsg, 256, L"Removing game failed: 0x%0.8x", hr );
            if( !settings.bSilent )
                MessageBox( NULL, szMsg, TEXT("RemoveFromGameExplorer"), MB_OK | MB_ICONINFORMATION );
            bFailure = true;
        }

        hr = RemoveTasks( &guid );
        if( FAILED(hr) )
        {
            StringCchPrintf( szMsg, 256, L"Removing tasks failed: 0x%0.8x", hr );
            if( !settings.bSilent )
                MessageBox( NULL, szMsg, TEXT("RemoveTasks"), MB_OK | MB_ICONINFORMATION );
            bFailure = true;
        }

        if( !settings.bSkipRichSavedGames )
        {
            hr = RemoveRichSavedGames( settings.strRichSavedGameExt );
            if( FAILED(hr) )
            {
                StringCchPrintf( szMsg, 256, L"Removing tasks failed: 0x%0.8x", hr );
                if( !settings.bSilent )
                    MessageBox( NULL, szMsg, TEXT("RemoveTasks"), MB_OK | MB_ICONINFORMATION );
                bFailure = true;
            }
        }

        if( !bFailure )
        {
            StringCchPrintf( szMsg, 256, L"Uninstall of '%s' succeeded\n", settings.strGDFBinPath );
            if( !settings.bSilent )
                MessageBox( NULL, szMsg, TEXT("RemoveFromGameExplorer"), MB_OK | MB_ICONINFORMATION );
        }
    }

    return 0;
}


//-----------------------------------------------------------------------------
// Converts a string to a GUID
//-----------------------------------------------------------------------------
BOOL ConvertStringToGUID( const WCHAR* strIn, GUID* pGuidOut )
{
    UINT aiTmp[10];

    if( swscanf( strIn, L"{%8X-%4X-%4X-%2X%2X-%2X%2X%2X%2X%2X%2X}",
                    &pGuidOut->Data1, 
                    &aiTmp[0], &aiTmp[1], 
                    &aiTmp[2], &aiTmp[3],
                    &aiTmp[4], &aiTmp[5],
                    &aiTmp[6], &aiTmp[7],
                    &aiTmp[8], &aiTmp[9] ) != 11 )
    {
        ZeroMemory( pGuidOut, sizeof(GUID) );
        return FALSE;
    }
    else
    {
        pGuidOut->Data2       = (USHORT) aiTmp[0];
        pGuidOut->Data3       = (USHORT) aiTmp[1];
        pGuidOut->Data4[0]    = (BYTE) aiTmp[2];
        pGuidOut->Data4[1]    = (BYTE) aiTmp[3];
        pGuidOut->Data4[2]    = (BYTE) aiTmp[4];
        pGuidOut->Data4[3]    = (BYTE) aiTmp[5];
        pGuidOut->Data4[4]    = (BYTE) aiTmp[6];
        pGuidOut->Data4[5]    = (BYTE) aiTmp[7];
        pGuidOut->Data4[6]    = (BYTE) aiTmp[8];
        pGuidOut->Data4[7]    = (BYTE) aiTmp[9];
        return TRUE;
    }
}


//-----------------------------------------------------------------------------
// Returns the path of the saved games
//-----------------------------------------------------------------------------
HRESULT GetSavedGamesPath( WCHAR* strDest, int cchDest )
{
    HRESULT hr;
    WCHAR strMyDocsPath[MAX_PATH];

    hr = SHGetFolderPath( NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, strMyDocsPath );
    if( FAILED(hr) ) 
        return hr;

    hr = StringCchPrintf( strDest, cchDest, L"%s\\My Games\\ExampleGame", strMyDocsPath );
    if( FAILED(hr) ) 
        return hr;	
    
    int nSuccess = SHCreateDirectoryEx( NULL, strDest, NULL );
    if( nSuccess == ERROR_SUCCESS || nSuccess == ERROR_ALREADY_EXISTS )
        return S_OK;

    return E_FAIL;
}


//-----------------------------------------------------------------------------
HRESULT WriteSavedGame( WCHAR* strSavedGameFileName, WCHAR* strExistingScreenshotPath )
{
    WCHAR strPath[MAX_PATH] = {0};
    WCHAR strFileName[MAX_PATH] = {0};
    BYTE* pThumbnailBuffer = NULL;
    DWORD dwThumbnailFileSize = 0;

    // Get the folder to the saved games
    GetSavedGamesPath( strPath, MAX_PATH );

    // Create the saved game file
    StringCchPrintf( strFileName, MAX_PATH, L"%s\\%s", strPath, strSavedGameFileName );
    HANDLE hFile = CreateFile( strFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    if( hFile != INVALID_HANDLE_VALUE )
    {
        // Write out the rich saved game header
        DWORD dwWritten;
        RICH_GAME_MEDIA_HEADER savedHeader;
        ZeroMemory( &savedHeader, sizeof(RICH_GAME_MEDIA_HEADER) );
        savedHeader.dwMagicNumber = RM_MAGICNUMBER;
        savedHeader.dwHeaderVersion = 1;
        savedHeader.dwHeaderSize = sizeof(RICH_GAME_MEDIA_HEADER);

        // Save embedded thumbnail.  This reads a screenshot from a file on disk it could also come from a file in memory.  
        // It could also be an icon.
        DWORD dwRead = 0;
        HANDLE hThumbnailFile = CreateFile( strExistingScreenshotPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        if( hThumbnailFile ) 
        {
            dwThumbnailFileSize = GetFileSize( hThumbnailFile, NULL );
            if( INVALID_FILE_SIZE != dwThumbnailFileSize )
            {
                pThumbnailBuffer = new BYTE[dwThumbnailFileSize];
                ReadFile( hThumbnailFile, (BYTE*) pThumbnailBuffer, dwThumbnailFileSize, &dwRead, NULL );

                if( dwThumbnailFileSize != dwRead )
                {
                    SAFE_DELETE_ARRAY( pThumbnailBuffer );
                    dwThumbnailFileSize = 0;
                }
            }
            else
            {
                dwThumbnailFileSize = 0;
            }

            CloseHandle( hThumbnailFile );
        }

        // Point to the embedded thumbnail (optional)
        // The offset it relative to the end of the RICH_GAME_MEDIA_HEADER structure.  
        savedHeader.liThumbnailOffset.QuadPart = sizeof(EXAMPLE_SAVED_GAME_DATA); // put it after save game data
        savedHeader.dwThumbnailSize = dwThumbnailFileSize;

        // Change this string to the gameID GUID found in the game's GDF file
        ConvertStringToGUID( L"{8236D2E9-2528-4C5C-ABA3-E0B8B657A297}", &savedHeader.guidGameId );

        // These strings should be blank if they aren't needed
        StringCchCopy( savedHeader.szGameName, RM_MAXLENGTH, L"game name test" );  
        StringCchCopy( savedHeader.szSaveName, RM_MAXLENGTH, L"save name test" );  
        StringCchCopy( savedHeader.szLevelName, RM_MAXLENGTH, L"level name test" );
        StringCchCopy( savedHeader.szComments, RM_MAXLENGTH, L"comments test" ); 

        WriteFile( hFile, &savedHeader, sizeof(RICH_GAME_MEDIA_HEADER), &dwWritten, NULL );

        // Create some app-specific save game data and write it after the header
        EXAMPLE_SAVED_GAME_DATA appData;
        ZeroMemory( &appData, sizeof(EXAMPLE_SAVED_GAME_DATA) );
        appData.dwLevel = 1;
        appData.dwHitPoints = 100;
        StringCchCopy( appData.szName, 256, L"Fighter" );
        appData.levelData[0] = 0xFF; // etc
        WriteFile( hFile, &appData, sizeof(EXAMPLE_SAVED_GAME_DATA), &dwWritten, NULL );

        if( pThumbnailBuffer )
        {
            WriteFile( hFile, pThumbnailBuffer, dwThumbnailFileSize, &dwWritten, NULL );
            SAFE_DELETE_ARRAY( pThumbnailBuffer );
        }

        CloseHandle( hFile );
    }

    return S_OK;
}



//-----------------------------------------------------------------------------
HRESULT EnumAndRemoveGames()
{
    HRESULT hr;
    IWbemLocator*  pIWbemLocator = NULL;
    IWbemServices* pIWbemServices = NULL;
    BSTR           pNamespace = NULL;
    IEnumWbemClassObject* pEnum = NULL;
    GUID           guid;
    WCHAR          strGameName[256];
    WCHAR          strGameGUID[256];
    WCHAR          strGDFBinaryPath[256];
    
    CoInitialize(0);

    hr = CoCreateInstance( __uuidof(WbemLocator), NULL, CLSCTX_INPROC_SERVER, 
                           __uuidof(IWbemLocator), (LPVOID*) &pIWbemLocator );
    if( SUCCEEDED(hr) && pIWbemLocator )
    {
        // Using the locator, connect to WMI in the given namespace.
        pNamespace = SysAllocString( L"\\\\.\\root\\cimv2\\Applications\\Games" );

        hr = pIWbemLocator->ConnectServer( pNamespace, NULL, NULL, 0L,
                                        0L, NULL, NULL, &pIWbemServices );
        if( SUCCEEDED(hr) && pIWbemServices != NULL )
        {
            // Switch security level to IMPERSONATE. 
            CoSetProxyBlanket( pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, 
                               RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0 );

            BSTR bstrQueryType = SysAllocString( L"WQL" );

            WCHAR szQuery[1024];
            StringCchCopy( szQuery, 1024, L"SELECT * FROM GAME" );
            BSTR bstrQuery = SysAllocString( szQuery );

            hr = pIWbemServices->ExecQuery( bstrQueryType, bstrQuery, 
                                            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                            NULL, &pEnum );
            if( SUCCEEDED(hr) )
            {
                IWbemClassObject* pGameClass = NULL;
                DWORD uReturned = 0;
                BSTR pPropName = NULL;

                // Get the first one in the list
                for(;;)
                {
                    hr = pEnum->Next( 5000, 1, &pGameClass, &uReturned );
                    if( SUCCEEDED(hr) && uReturned != 0 && pGameClass != NULL )
                    {
                        VARIANT var;

                        // Get the InstanceID string
                        pPropName = SysAllocString( L"InstanceID" );
                        hr = pGameClass->Get( pPropName, 0L, &var, NULL, NULL );
                        if( SUCCEEDED(hr) && var.vt == VT_BSTR )
                        {
                            StringCchCopy( strGameGUID, 256, var.bstrVal );
                            ConvertStringToGUID( var.bstrVal, &guid );
                        }
                        if( pPropName ) SysFreeString( pPropName );

                        // Get the InstanceID string
                        pPropName = SysAllocString( L"Name" );
                        hr = pGameClass->Get( pPropName, 0L, &var, NULL, NULL );
                        if( SUCCEEDED(hr) && var.vt == VT_BSTR )
                        {
                            StringCchCopy( strGameName, 256, var.bstrVal );
                        }
                        if( pPropName ) SysFreeString( pPropName );

                        // Get the InstanceID string
                        pPropName = SysAllocString( L"GDFBinaryPath" );
                        hr = pGameClass->Get( pPropName, 0L, &var, NULL, NULL );
                        if( SUCCEEDED(hr) && var.vt == VT_BSTR )
                        {
                            StringCchCopy( strGDFBinaryPath, 256, var.bstrVal );
                        }
                        if( pPropName ) SysFreeString( pPropName );

                        WCHAR szMsg[256];
                        StringCchPrintf( szMsg, 256, L"Remove %s [%s] [%s]?", strGameName, strGDFBinaryPath, strGameGUID );
                        if( IDYES == MessageBox( NULL, szMsg, L"GDFInstall", MB_YESNO ) )
                        {
                            RemoveFromGameExplorer( &guid );
                        }

                        SAFE_RELEASE( pGameClass );
                    }
                    else
                    {
                        break;
                    }
                }
            }

            SAFE_RELEASE( pEnum );
        }

        if( pNamespace ) SysFreeString( pNamespace );
        SAFE_RELEASE( pIWbemServices );
    }

    SAFE_RELEASE( pIWbemLocator );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Parses the command line for parameters.  See DXUTInit() for list 
//--------------------------------------------------------------------------------------
bool ParseCommandLine( SETTINGS* pSettings )
{
    WCHAR* strCmdLine;
    WCHAR* strArg;

    int nNumArgs;
    WCHAR** pstrArgList = CommandLineToArgvW( GetCommandLine(), &nNumArgs );
    for( int iArg=1; iArg<nNumArgs; iArg++ )
    {
        strCmdLine = pstrArgList[iArg];

        // Handle flag args
        if( *strCmdLine == L'/' || *strCmdLine == L'-' )
        {
            strCmdLine++;

            if( IsNextArg( strCmdLine, L"enum" ) )
            {
                pSettings->bEnumMode = true;
                continue;
            }

            if( IsNextArg( strCmdLine, L"mcl" ) )
            {
                pSettings->bUseMCLFile = true;
                continue;
            }

            if( IsNextArg( strCmdLine, L"u" ) )
            {
                pSettings->bUninstall = true;
                continue;
            }

            if( IsNextArg( strCmdLine, L"allusers" ) )
            {
                pSettings->bAllUsers = true;
                continue;
            }

            if( IsNextArg( strCmdLine, L"nomediacenter" ) )
            {
                pSettings->bSkipMediaCenter = true;
                continue;
            }

            if( IsNextArg( strCmdLine, L"norichsavedgames" ) )
            {
                pSettings->bSkipRichSavedGames = true;
                continue;
            }

            if( IsNextArg( strCmdLine, L"installpath" ) )
            {
                if( iArg+1 < nNumArgs )
                {
                    strArg = pstrArgList[++iArg];
                    StringCchCopy( pSettings->strInstallPath, MAX_PATH, strArg );
                    PathAddBackslash( pSettings->strInstallPath );
                    continue;
                }

                if( !pSettings->bSilent )
                    MessageBox( NULL, L"Incorrect flag usage: /installpath\n", L"GDFInstall", MB_OK );
                continue;
            }
            
            if( IsNextArg( strCmdLine, L"silent" ) )
            {
                pSettings->bSilent = true;
                continue;
            }

            if( IsNextArg( strCmdLine, L"exe" ) )
            {
                if( iArg+1 < nNumArgs )
                {
                    strArg = pstrArgList[++iArg];
                    StringCchCopy( pSettings->strExePath, MAX_PATH, strArg );
                    continue;
                }

                if( !pSettings->bSilent )
                    MessageBox( NULL, L"Incorrect flag usage: /exe\n", L"GDFInstall", MB_OK );
                continue;
            }

            if( IsNextArg( strCmdLine, L"richext" ) )
            {
                if( iArg+1 < nNumArgs )
                {
                    strArg = pstrArgList[++iArg];
                    StringCchCopy( pSettings->strRichSavedGameExt, MAX_PATH, strArg );
                    continue;
                }

                if( !pSettings->bSilent )
                    MessageBox( NULL, L"Incorrect flag usage: /richext\n", L"GDFInstall", MB_OK );
                continue;
            }

            if( IsNextArg( strCmdLine, L"?" ) )
            {
                DisplayUsage();
                return false;
            }
        }
        else 
        {
            // Handle non-flag args as seperate input files
            StringCchPrintf( pSettings->strGDFBinPath, MAX_PATH, L"%s%s", pSettings->strInstallPath, strCmdLine );
            continue;
        }
    }

    return true;
}


//--------------------------------------------------------------------------------------
bool IsNextArg( WCHAR*& strCmdLine, WCHAR* strArg )
{
    int nArgLen = (int) wcslen(strArg);
    if( _wcsnicmp( strCmdLine, strArg, nArgLen ) == 0 && strCmdLine[nArgLen] == 0 )
        return true;

    return false;
}


//--------------------------------------------------------------------------------------
void DisplayUsage()
{
    MessageBox( NULL, 
        L"GDFInstall - a command line sample to show how to register with Game Explorer and Media Center\n"
        L"\n"
        L"Usage: GDFInstall.exe [options] <gdf binary>\n" 
        L"\n" 
        L"where:\n" 
        L"\n" 
        L"  [/silent]\t\tSilent mode.  No message boxes\n" 
        L"  [/enum]\t\tEnters enum mode where each installed GDF is enumerated\n" 
        L"  \t\tand the user is prompted to uninstalled. Other arguments are ignored.\n" 
        L"  [/u]\t\tUninstalls the game instead of installing\n"
        L"  [/allusers]\tInstalls the game for all users.  Defaults to current user\n"
        L"  \t\tNote: This requires the process have adminstrator privledges\n"
        L"  [/mcl]\t\tUses MCL file to register with Media Center.  Defaults to using RegisterMCEApp.exe.\n"
        L"  [/nomediacenter]\tDoes not register with Media Center\n"
        L"  [/norichsavedgames] Does not add Rich Saved Game support\n"
        L"  [/installpath x]\tSets the install path for the game. Defaults to the current working directory\n"
        L"  [/richext x]\tSets the rich saved game extension for the game. Defaults .ExampleSaveGame\n"
        L"  [/exe x]\t\tSets the path to game exe. Defaults notepad.exe\n"
        L"  <gdf binary>\tThe path to the GDF binary to install or remove.\n" 
        L"  \t\tDefaults to GDFExampleBinary.dll in current working directory.\n" 
        L"  \t\tGDFExampleBinary.dll is a sample GDF binary in the DXSDK.\n" 
        , L"GDFInstall", MB_OK );

}
