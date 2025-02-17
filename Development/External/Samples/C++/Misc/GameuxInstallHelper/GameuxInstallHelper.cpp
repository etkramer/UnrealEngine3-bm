//--------------------------------------------------------------------------------------
// File: GameuxInstallHelper.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#define _WIN32_DCOM
#define _CRT_SECURE_NO_DEPRECATE

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif    
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif    
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif

#include <windows.h>
#include <shlobj.h>
#include <rpcsal.h>
#include <gameux.h>
#include <crtdbg.h>
#include <msi.h>
#include <msiquery.h>
#include <strsafe.h>
#include <assert.h>
#include <wbemidl.h>
#include <objbase.h>
#include <shellapi.h>
#include <shlobj.h>
#include <WtsApi32.h>

#define NO_SHLWAPI_STRFCNS
#include <shlwapi.h>

#include "GameuxInstallHelper.h"
#include "GDFParse.h"

// Uncomment to get a debug messagebox
//#define SHOW_S1_DEBUG_MSGBOXES 
//#define SHOW_S2_DEBUG_MSGBOXES // more detail


//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
LPWSTR GetPropertyFromMSI( MSIHANDLE hMSI, LPCWSTR szPropName );
HRESULT ConvertStringToGUID( const WCHAR* strSrc, GUID* pGuidDest );
HRESULT ConvertGUIDToStringCch( const GUID* pGuidSrc, WCHAR* strDest, int cchDestChar );
HRESULT CreateShortcut( WCHAR* strLaunchPath, WCHAR* strCommandLineArgs, WCHAR* strShortcutFilePath );
HRESULT GetAccountName( WCHAR* strUser, DWORD cchUser, WCHAR* strDomain, DWORD cchDomain );


//--------------------------------------------------------------------------------------
// This stores the install location, generates a instance GUID one hasn't been set, and 
// sets up the CustomActionData properties for the deferred custom actions
//--------------------------------------------------------------------------------------
UINT __stdcall SetMSIGameExplorerProperties( MSIHANDLE hModule ) 
{
    HRESULT hr;
    GUID guidGameInstance;
    bool bFoundExistingGUID = false;
    WCHAR szCustomActionData[1024] = {0};
    WCHAR strAbsPathToGDF[1024] = {0};
    WCHAR strGameInstanceGUID[128] = {0};
    WCHAR* szInstallDir = NULL;
    WCHAR* szALLUSERS = GetPropertyFromMSI(hModule, L"ALLUSERS");
    WCHAR* szRelativePathToGDF = GetPropertyFromMSI(hModule, L"RelativePathToGDF");
    WCHAR* szProductCode = GetPropertyFromMSI(hModule, L"ProductCode");

    // See if the install location property is set.  If it is, use that.  
    // Otherwise, get the property from TARGETDIR
    bool bGotInstallDir = false;
    if( szProductCode )
    {
        DWORD dwBufferSize = 1024;
        szInstallDir = new WCHAR[dwBufferSize];
        if( ERROR_SUCCESS == MsiGetProductInfo( szProductCode, INSTALLPROPERTY_INSTALLLOCATION, szInstallDir, &dwBufferSize ) )
            bGotInstallDir = true;
    }
    if( !bGotInstallDir )
        szInstallDir = GetPropertyFromMSI(hModule, L"TARGETDIR");

    // Set the ARPINSTALLLOCATION property to the install dir so that 
    // the uninstall custom action can have it when getting the INSTALLPROPERTY_INSTALLLOCATION
    MsiSetPropertyW( hModule, L"ARPINSTALLLOCATION", szInstallDir ); 

    // Get game instance GUID if it exists
    StringCchCopy( strAbsPathToGDF, 1024, szInstallDir );
    StringCchCat( strAbsPathToGDF, 1024, szRelativePathToGDF );
    hr = RetrieveGUIDForApplicationW( strAbsPathToGDF, &guidGameInstance );
    if( FAILED(hr) || memcmp(&guidGameInstance, &GUID_NULL, sizeof(GUID)) == 0 )
    {
        // If GUID is not there, then generate GUID
        CoCreateGuid( &guidGameInstance );
    }
    else
    {
        bFoundExistingGUID = true;
    }
    StringFromGUID2( guidGameInstance, strGameInstanceGUID, 128 );

    // Set the CustomActionData property for the "AddToGameExplorerUsingMSI" deferred custom action.
    StringCchCopy( szCustomActionData, 1024, szInstallDir );
    StringCchCat( szCustomActionData, 1024, szRelativePathToGDF );
    StringCchCat( szCustomActionData, 1024, L"|" );
    StringCchCat( szCustomActionData, 1024, szInstallDir );
    StringCchCat( szCustomActionData, 1024, L"|" );
    if( szALLUSERS && (szALLUSERS[0] == '1' || szALLUSERS[0] == '2') )  
        StringCchCat( szCustomActionData, 1024, L"3" );
    else
        StringCchCat( szCustomActionData, 1024, L"2" );
    StringCchCat( szCustomActionData, 1024, L"|" );
    StringCchCat( szCustomActionData, 1024, strGameInstanceGUID );

    // Set the CustomActionData property for the deferred custom actions
    MsiSetProperty( hModule, L"GameUXAddAsAdmin", szCustomActionData );
    MsiSetProperty( hModule, L"GameUXAddAsCurUser", szCustomActionData );
    MsiSetProperty( hModule, L"GameUXRollBackRemoveAsAdmin", szCustomActionData );
    MsiSetProperty( hModule, L"GameUXRollBackRemoveAsCurUser", szCustomActionData );

    // Set the CustomActionData property for the deferred custom actions
    MsiSetProperty( hModule, L"GameUXRemoveAsAdmin", strGameInstanceGUID );
    MsiSetProperty( hModule, L"GameUXRemoveAsCurUser", strGameInstanceGUID );
    MsiSetProperty( hModule, L"GameUXRollBackAddAsAdmin", strGameInstanceGUID );
    MsiSetProperty( hModule, L"GameUXRollBackAddAsCurUser", strGameInstanceGUID );

#ifdef SHOW_S2_DEBUG_MSGBOXES
    WCHAR sz[1024];
    StringCchPrintf( sz, 1024, L"szInstallDir='%s'\nszRelativePathToGDF='%s'\nstrAbsPathToGDF='%s'\nALLUSERS='%s'\nbFoundExistingGUID=%d\nstrGameInstanceGUID='%s'", 
            szInstallDir, szRelativePathToGDF, strAbsPathToGDF, szALLUSERS, bFoundExistingGUID, strGameInstanceGUID );
    MessageBox( NULL, sz, L"SetMSIGameExplorerProperties", MB_OK );
#endif

    if( szRelativePathToGDF ) delete [] szRelativePathToGDF;
    if( szALLUSERS ) delete [] szALLUSERS;
    if( szInstallDir ) delete [] szInstallDir;
    if( szProductCode ) delete [] szProductCode;

    return ERROR_SUCCESS;
}


//--------------------------------------------------------------------------------------
// The CustomActionData property must be formated like so:
//      "<path to GDF binary>|<game install path>|<install scope>|<generated GUID>"
// for example:
//      "C:\MyGame\GameGDF.dll|C:\MyGame|2|{1DE6CE3D-EA69-4671-941F-26F789F39C5B}"
//--------------------------------------------------------------------------------------
UINT __stdcall AddToGameExplorerUsingMSI( MSIHANDLE hModule ) 
{
    HRESULT hr = E_FAIL;
    WCHAR* szCustomActionData = GetPropertyFromMSI(hModule, L"CustomActionData");

    if( szCustomActionData )
    {
        WCHAR szGDFBinPath[MAX_PATH] = {0};
        WCHAR szGameInstallPath[MAX_PATH] = {0};
        GAME_INSTALL_SCOPE InstallScope = GIS_ALL_USERS;
        GUID InstanceGUID;

        WCHAR* pFirstDelim = wcschr( szCustomActionData, '|' );
        WCHAR* pSecondDelim = wcschr( pFirstDelim + 1, '|' );
        WCHAR* pThirdDelim = wcschr( pSecondDelim + 1, '|' );

        if( pFirstDelim )
        {
            *pFirstDelim = 0;            
            if( pSecondDelim )
            {
                *pSecondDelim = 0;
                if( pThirdDelim )
                {
                    *pThirdDelim = 0;
                    ConvertStringToGUID( pThirdDelim + 1, &InstanceGUID );
                }
                InstallScope = (GAME_INSTALL_SCOPE)_wtoi( pSecondDelim + 1 );
            }
            StringCchCopy( szGameInstallPath, MAX_PATH, pFirstDelim + 1 );
        }
        StringCchCopy( szGDFBinPath, MAX_PATH, szCustomActionData );

        hr = AddToGameExplorerW( szGDFBinPath, szGameInstallPath, InstallScope, &InstanceGUID );

#ifdef SHOW_S1_DEBUG_MSGBOXES
        WCHAR sz[1024];
        WCHAR szGUID[64] = {0};
        if( pThirdDelim )
            StringCchCopy( szGUID, 64, pThirdDelim + 1 );
        StringCchPrintf( sz, 1024, L"szGDFBinPath='%s'\nszGameInstallPath='%s'\nInstallScope='%s'\nszGUID='%s'\nhr=0x%0.8x\n", 
            szGDFBinPath, szGameInstallPath, (InstallScope == GIS_ALL_USERS) ? L"GIS_ALL_USERS" : L"GIS_CURRENT_USER", szGUID, hr);
        MessageBox( NULL, sz, L"AddToGameExplorerUsingMSI", MB_OK );
#endif

        delete [] szCustomActionData;
    }
    else
    {
#ifdef SHOW_S1_DEBUG_MSGBOXES
        WCHAR sz[1024];
        StringCchPrintf( sz, 1024, L"CustomActionData property not found\n" );
        MessageBox( NULL, sz, L"AddToGameExplorerUsingMSI", MB_OK );
#endif
    }

    // Ignore success/failure and continue on with install
    return ERROR_SUCCESS;
}


//--------------------------------------------------------------------------------------
// The CustomActionData property must be formated like so:
//      "<Game Instance GUID>"
// for example:
//      "{1DE6CE3D-EA69-4671-941F-26F789F39C5B}"
//--------------------------------------------------------------------------------------
UINT __stdcall RemoveFromGameExplorerUsingMSI( MSIHANDLE hModule ) 
{
    GUID InstanceGUID;
    HRESULT hr = E_FAIL;
    WCHAR* szCustomActionData = GetPropertyFromMSI(hModule, L"CustomActionData");

    if( szCustomActionData )
    {
        WCHAR strInstanceGUID[64] = {0};
        StringCchCopy( strInstanceGUID, 64, szCustomActionData );
        ConvertStringToGUID( strInstanceGUID, &InstanceGUID );

        hr = RemoveFromGameExplorer( &InstanceGUID );
        delete [] szCustomActionData;

#ifdef SHOW_S2_DEBUG_MSGBOXES
        WCHAR sz[1024];
        StringCchPrintf( sz, 1024, L"strInstanceGUID='%s'\nhr=0x%0.8x", strInstanceGUID, hr );
        MessageBox( NULL, sz, L"RemoveFromGameExplorerUsingMSI", MB_OK );
#endif 
    }

    // Ignore success/failure and continue on with uninstall
    return ERROR_SUCCESS;
}

//--------------------------------------------------------------------------------------
// Write a UTF16-LE BOM to the file
//--------------------------------------------------------------------------------------
HRESULT WriteUnicodeHeader( HANDLE hFile )
{
    WORD dwHeader = 0xFEFF;
    DWORD dwWritten = 0;
    DWORD dwNumberOfBytesToWrite = (DWORD)(sizeof(WORD));
    WriteFile( hFile, &dwHeader, dwNumberOfBytesToWrite, &dwWritten, NULL );
    if( dwWritten != dwNumberOfBytesToWrite )
        return E_FAIL;

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT WriteLine( HANDLE hFile, const WCHAR* strMsg, ... )
{
    WCHAR strBuffer[512];

    va_list args;
    va_start(args, strMsg);
    StringCchVPrintf( strBuffer, 512, strMsg, args );
    strBuffer[511] = L'\0';
    va_end(args);

    // Add line feed
    StringCchCat( strBuffer, 512, L"\r\n" );

    size_t dwBufferLen = 0;
    StringCchLength( strBuffer, 1024, &dwBufferLen );

    DWORD dwWritten = 0;
    DWORD dwNumberOfBytesToWrite = (DWORD)(dwBufferLen * sizeof(WCHAR));
    WriteFile( hFile, strBuffer, dwNumberOfBytesToWrite, &dwWritten, NULL );
    if( dwWritten != dwNumberOfBytesToWrite )
        return E_FAIL;

    return S_OK;
}


//--------------------------------------------------------------------------------------
bool IsWindowsVistaOrHigher()
{
    OSVERSIONINFO os;
    ZeroMemory( &os, sizeof(OSVERSIONINFO) );
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx( &os );

    if( os.dwMajorVersion > 5 )
        return true;
    else
        return false;
}


//--------------------------------------------------------------------------------------
void GetFolderAndFileName( WCHAR* strFullPath, WCHAR* strFolderName, WCHAR* strFileName )
{
    WCHAR* strFileNameTmp = NULL;
    GetFullPathName( strFullPath, MAX_PATH, strFolderName, &strFileNameTmp );
    if( strFolderName == NULL ) 
    {
        StringCchCopy( strFolderName, MAX_PATH, strFullPath );
        strFileNameTmp = strFolderName;
    }

    // Make strFileName just be the filename w/o the extension
    StringCchCopy( strFileName, MAX_PATH, strFileNameTmp );
    WCHAR* pLastDot = wcsrchr( strFileName, '.' );
    if( pLastDot ) *pLastDot = 0;

    // Make strFolderName just contain the path
    if( strFileNameTmp ) *strFileNameTmp = 0;
}


//--------------------------------------------------------------------------------------
// Register with Media Center using the data from a GDF binary
//--------------------------------------------------------------------------------------
STDAPI RegisterWithMediaCenterW( WCHAR* strGDFBinPath, WCHAR* strGameInstallPath, 
                                 GAME_INSTALL_SCOPE InstallScope, 
                                 WCHAR* strExePath, WCHAR* strCommandLineArgs, 
                                 bool bUseRegisterMCEApp )
{
    HRESULT hr;

    CGDFParse gdfParse;
    hr = gdfParse.ExtractXML( strGDFBinPath );
    if( FAILED(hr) )
        return hr;

    WCHAR strName[256];
    WCHAR strDeveloper[256];
    WCHAR strDescription[256];
    gdfParse.GetName( strName, 256 );
    gdfParse.GetDeveloper( strDeveloper, 256 );
    gdfParse.GetDescription( strDescription, 256 );

    WCHAR strGameInstallPathWithSlash[MAX_PATH];
    StringCchCopy( strGameInstallPathWithSlash, MAX_PATH, strGameInstallPath );
    PathAddBackslash( strGameInstallPathWithSlash );

    WCHAR strExeFolderName[MAX_PATH];
    WCHAR strExeFileName[MAX_PATH];
    GetFolderAndFileName( strExePath, strExeFolderName, strExeFileName );

    WCHAR strThumbnailPath[MAX_PATH];
    WCHAR strXMLPath[MAX_PATH];
    WCHAR strShortcutFilePath[MAX_PATH];
    StringCchPrintf( strThumbnailPath, MAX_PATH, L"%s%s-MCE.png", strGameInstallPathWithSlash, strExeFileName );
    StringCchPrintf( strShortcutFilePath, MAX_PATH, L"%s%s-MCE.lnk", strGameInstallPathWithSlash, strExeFileName );

    WCHAR strCmdLineArgs[MAX_PATH];
    if( strCommandLineArgs )
        StringCchPrintf( strCmdLineArgs, MAX_PATH, L"%s -mce", strCommandLineArgs );
    else
        StringCchCopy( strCmdLineArgs, MAX_PATH, L"-mce" );

    if( bUseRegisterMCEApp )
    {
        // Using RegisterMCEApp.exe to register with Media Center, so call RegisterMCEApp.exe [/allusers] <file.xml>

        // Verify RegisterMCEApp.exe exists
        WCHAR strRegisterApp[MAX_PATH];
        WCHAR strWindowsDir[MAX_PATH];
        GetWindowsDirectory( strWindowsDir, MAX_PATH );
        StringCchPrintf( strRegisterApp, MAX_PATH, L"%s\\ehome\\RegisterMCEApp.exe", strWindowsDir );
        if( GetFileAttributes( strRegisterApp ) == INVALID_FILE_ATTRIBUTES )
            return E_FAIL;

        StringCchPrintf( strXMLPath, MAX_PATH, L"%s%s-MCE.xml", strGameInstallPathWithSlash, strExeFileName );

        // Create the XML metadata file for Media Center
        HANDLE hFile = CreateFile( strXMLPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
        if( hFile != INVALID_HANDLE_VALUE )
        {
            WCHAR strAppID[128] = {0};
            WCHAR strEntryID[128] = {0};
            GUID appID;
            GUID entryID;
            CoCreateGuid( &appID );
            CoCreateGuid( &entryID );
            StringFromGUID2( appID, strAppID, 128 );
            StringFromGUID2( entryID, strEntryID, 128 );

            WriteUnicodeHeader( hFile );
            WriteLine( hFile, L"<?xml version=\"1.0\" encoding=\"utf-16\"?>" );
            WriteLine( hFile, L"" );
            WriteLine( hFile, L"<application" );
            WriteLine( hFile, L"	title = \"%s\"", strName );
            WriteLine( hFile, L"	id = \"%s\"", strAppID );
            WriteLine( hFile, L"	name = \"%s\"", strName );
            WriteLine( hFile, L"	companyName = \"%s\"", strDeveloper );
            WriteLine( hFile, L"	description = \"%s\">", strDescription );
            WriteLine( hFile, L"" );
            WriteLine( hFile, L"    <entrypoint" );
            WriteLine( hFile, L"	    id = \"%s\"", strEntryID );
            WriteLine( hFile, L"	    run = \"%s\"", strShortcutFilePath );
            WriteLine( hFile, L"	    title = \"%s\"", strName );
            WriteLine( hFile, L"	    description = \"%s\"", strDescription );
            WriteLine( hFile, L"	    thumbnailUrl = \"%s\">", strThumbnailPath );
            WriteLine( hFile, L"" );
            WriteLine( hFile, L"        <capabilitiesRequired" );
            WriteLine( hFile, L"            directX=\"True\"" );
            WriteLine( hFile, L"            audio=\"True\"" );
            WriteLine( hFile, L"            video=\"True\"" );
            WriteLine( hFile, L"            intensiveRendering=\"True\"" );
            WriteLine( hFile, L"            console=\"False\"/>" );
            WriteLine( hFile, L"" );
            WriteLine( hFile, L"        <category category=\"More Programs\"/>" );      
            WriteLine( hFile, L"" );
            WriteLine( hFile, L"    </entrypoint>" );
            WriteLine( hFile, L"" );
            WriteLine( hFile, L"</application>" );
            CloseHandle( hFile );
        }

        // Call RegisterMCEApp.exe on the XML file
        WCHAR strRegisterArgs[MAX_PATH];
        if( InstallScope == GIS_ALL_USERS )
            StringCchPrintf( strRegisterArgs, MAX_PATH, L"/allusers \"%s\"", strXMLPath );
        else
            StringCchPrintf( strRegisterArgs, MAX_PATH, L"\"%s\"", strXMLPath );
        ShellExecute( NULL, NULL, strRegisterApp, strRegisterArgs, NULL, SW_HIDE );
    }
    else
    {
        // Use deprecated MCL files to register with Media Center, so just create a MCL file in a known folder

        StringCchPrintf( strXMLPath, MAX_PATH, L"%s%s-MCE.mcl", strGameInstallPathWithSlash, strExeFileName );

        HANDLE hFile = CreateFile( strXMLPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
        if( hFile != INVALID_HANDLE_VALUE )
        {
            WriteUnicodeHeader( hFile );
            WriteLine( hFile, L"<?xml version=\"1.0\" encoding=\"utf-16\"?>" );
            WriteLine( hFile, L"<application" );
            WriteLine( hFile, L"	Title = \"%s\"", strName );
            WriteLine( hFile, L"	ID = \"\"" );
            WriteLine( hFile, L"	Run = \"%s\"", strShortcutFilePath );
            WriteLine( hFile, L"	Name = \"%s\"", strName );
            WriteLine( hFile, L"	CompanyName = \"%s\"", strDeveloper );
            WriteLine( hFile, L"	ThumbnailImage = \"%s\"", strThumbnailPath );
            WriteLine( hFile, L"	Description = \"%s\"", strDescription );
            WriteLine( hFile, L">" );
            WriteLine( hFile, L"    <capabilitiesRequired" );
            WriteLine( hFile, L"        directX=\"True\"" );
            WriteLine( hFile, L"        audio=\"True\"" );
            WriteLine( hFile, L"        video=\"True\"" );
            WriteLine( hFile, L"        intensiveRendering=\"True\"" );
            WriteLine( hFile, L"        console=\"False\"" );
            WriteLine( hFile, L"    />" );
            WriteLine( hFile, L"</application>" );
            CloseHandle( hFile );
        }

        // Build the path to MCL depending on the OS version
        WCHAR strBasePath[MAX_PATH];
        WCHAR strShortcutToMCLPath[MAX_PATH];        
        if( IsWindowsVistaOrHigher() )
        {
            if( InstallScope == GIS_ALL_USERS )
                SHGetFolderPath( NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, strBasePath );
            else
                SHGetFolderPath( NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, strBasePath );
            StringCchPrintf( strShortcutToMCLPath, MAX_PATH, L"%s\\Media Center Programs", strBasePath );
            SHCreateDirectoryEx( NULL, strShortcutToMCLPath, NULL );
            StringCchPrintf( strShortcutToMCLPath, MAX_PATH, L"%s\\Media Center Programs\\%s.lnk", strBasePath, strExeFileName );
        }
        else
        {
            SHGetFolderPath( NULL, CSIDL_COMMON_STARTMENU, NULL, SHGFP_TYPE_CURRENT, strBasePath );
            StringCchPrintf( strShortcutToMCLPath, MAX_PATH, L"%s\\Programs\\Accessories\\Media Center\\Media Center Programs", strBasePath );
            SHCreateDirectoryEx( NULL, strShortcutToMCLPath, NULL );
            StringCchPrintf( strShortcutToMCLPath, MAX_PATH, L"%s\\Programs\\Accessories\\Media Center\\Media Center Programs\\%s.lnk", strBasePath, strExeFileName );
        }

        CreateShortcut( strXMLPath, L"", strShortcutToMCLPath );
    }

    gdfParse.ExtractGDFThumbnail( strGDFBinPath, strThumbnailPath );
    CreateShortcut( strExePath, strCmdLineArgs, strShortcutFilePath );

#ifdef SHOW_S2_DEBUG_MSGBOXES
    WCHAR sz[1024];
    StringCchPrintf( sz, 1024, L"strXMLPath='%s'\nstrThumbnailPath='%s'\nstrShortcutFilePath='%s'\nstrExeFileName='%s'\nstrExePath='%s'\nstrCmdLineArgs='%s'\n", 
        strXMLPath, strThumbnailPath, strShortcutFilePath, strExeFileName, strExePath, strCmdLineArgs);
    MessageBox( NULL, sz, L"RegisterWithMediaCenterW", MB_OK );
#endif 


    return S_OK;
}


//--------------------------------------------------------------------------------------
STDAPI RegisterWithMediaCenterA( CHAR* strGDFBinPath, CHAR* strGameInstallPath, GAME_INSTALL_SCOPE InstallScope, CHAR* strExePath, CHAR* strCommandLineArgs, bool bUseRegisterMCEApp )
{
    WCHAR wstrGDFBinPath[MAX_PATH] = {0};
    WCHAR wstrExePath[MAX_PATH] = {0};
    WCHAR wstrCommandLineArgs[MAX_PATH] = {0};
    WCHAR wstrGameInstallPath[MAX_PATH] = {0};
    MultiByteToWideChar(CP_ACP, 0, strCommandLineArgs, MAX_PATH, wstrCommandLineArgs, MAX_PATH);
    MultiByteToWideChar(CP_ACP, 0, strGameInstallPath, MAX_PATH, wstrGameInstallPath, MAX_PATH);
    MultiByteToWideChar(CP_ACP, 0, strExePath, MAX_PATH, wstrExePath, MAX_PATH);
    MultiByteToWideChar(CP_ACP, 0, strGDFBinPath, MAX_PATH, wstrGDFBinPath, MAX_PATH);
    return RegisterWithMediaCenterW( wstrGDFBinPath, wstrGameInstallPath, InstallScope, wstrExePath, wstrCommandLineArgs, bUseRegisterMCEApp );
}


//--------------------------------------------------------------------------------------
// Un-registers with Media Center 
//--------------------------------------------------------------------------------------
STDAPI UnRegisterWithMediaCenterW( WCHAR* strGameInstallPath, GAME_INSTALL_SCOPE InstallScope, 
                                   WCHAR* strExePath, bool bUseRegisterMCEApp )
{
    WCHAR strExeFolderName[MAX_PATH];
    WCHAR strExeFileName[MAX_PATH];
    GetFolderAndFileName( strExePath, strExeFolderName, strExeFileName );

    WCHAR strGameInstallPathWithSlash[MAX_PATH];
    StringCchCopy( strGameInstallPathWithSlash, MAX_PATH, strGameInstallPath );
    PathAddBackslash( strGameInstallPathWithSlash );

    // Delete the exe shortcut and thumbnail
    WCHAR strThumbnailPath[MAX_PATH];
    WCHAR strXMLPath[MAX_PATH];
    WCHAR strShortcutFilePath[MAX_PATH];
    StringCchPrintf( strThumbnailPath, MAX_PATH, L"%s%s-MCE.png", strGameInstallPathWithSlash, strExeFileName );
    StringCchPrintf( strShortcutFilePath, MAX_PATH, L"%s%s-MCE.lnk", strGameInstallPathWithSlash, strExeFileName );
    DeleteFile( strThumbnailPath );
    DeleteFile( strShortcutFilePath );

    // Remove the link from Media Center
    if( bUseRegisterMCEApp )
    {
        // Using RegisterMCEApp.exe to register with Media Center, so call RegisterMCEApp.exe /u [/allusers] <file.xml>

        // Verify RegisterMCEApp.exe exists
        WCHAR strRegisterApp[MAX_PATH];
        WCHAR strWindowsDir[MAX_PATH];
        GetWindowsDirectory( strWindowsDir, MAX_PATH );
        StringCchPrintf( strRegisterApp, MAX_PATH, L"%s\\ehome\\RegisterMCEApp.exe", strWindowsDir );
        if( GetFileAttributes( strRegisterApp ) == INVALID_FILE_ATTRIBUTES )
            return E_FAIL;

        StringCchPrintf( strXMLPath, MAX_PATH, L"%s%s-MCE.xml", strGameInstallPathWithSlash, strExeFileName );

        // Call RegisterMCEApp.exe on the XML file
        WCHAR strRegisterArgs[MAX_PATH];
        if( InstallScope == GIS_ALL_USERS )
            StringCchPrintf( strRegisterArgs, MAX_PATH, L"/u /allusers \"%s\"", strXMLPath );
        else
            StringCchPrintf( strRegisterArgs, MAX_PATH, L"/u \"%s\"", strXMLPath );
        ShellExecute( NULL, NULL, strRegisterApp, strRegisterArgs, NULL, SW_HIDE );
    }
    else
    {
        // Use deprecated MCL files to register with Media Center, so just delete the MCL file
        StringCchPrintf( strXMLPath, MAX_PATH, L"%s%s-MCE.mcl", strGameInstallPathWithSlash, strExeFileName );

        // Build the path to MCL depending on the OS version
        WCHAR strBasePath[MAX_PATH];
        WCHAR strShortcutToMCLPath[MAX_PATH];        
        if( IsWindowsVistaOrHigher() )
        {
            if( InstallScope == GIS_ALL_USERS )
                SHGetFolderPath( NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, strBasePath );
            else
                SHGetFolderPath( NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, strBasePath );
            StringCchPrintf( strShortcutToMCLPath, MAX_PATH, L"%s\\Media Center Programs\\%s.lnk", strBasePath, strExeFileName );
        }
        else
        {
            SHGetFolderPath( NULL, CSIDL_COMMON_STARTMENU, NULL, SHGFP_TYPE_CURRENT, strBasePath );
            StringCchPrintf( strShortcutToMCLPath, MAX_PATH, L"%s\\Programs\\Accessories\\Media Center\\Media Center Programs\\%s.lnk", strBasePath, strExeFileName );
        }

        DeleteFile( strShortcutToMCLPath );
    }

#ifdef SHOW_S2_DEBUG_MSGBOXES
    WCHAR sz[1024];
    StringCchPrintf( sz, 1024, L"strGameInstallPath='%s'\nInstallScope='%d'\nstrExePath='%s'\nbUseRegisterMCEApp='%d'\n", 
                               strGameInstallPath, InstallScope, strExePath, bUseRegisterMCEApp );
    MessageBox( NULL, sz, L"UnRegisterWithMediaCenter", MB_OK );
#endif 

    return S_OK;
}


//--------------------------------------------------------------------------------------
STDAPI UnRegisterWithMediaCenterA( CHAR* strGameInstallPath, GAME_INSTALL_SCOPE InstallScope, 
                                   CHAR* strExePath, bool bUseRegisterMCEApp )
{
    WCHAR wstrExePath[MAX_PATH] = {0};
    WCHAR wstrGameInstallPath[MAX_PATH] = {0};
    MultiByteToWideChar(CP_ACP, 0, strGameInstallPath, MAX_PATH, wstrGameInstallPath, MAX_PATH);
    MultiByteToWideChar(CP_ACP, 0, strExePath, MAX_PATH, wstrExePath, MAX_PATH);
    return UnRegisterWithMediaCenterW( wstrGameInstallPath, InstallScope, wstrExePath, bUseRegisterMCEApp );
}


//--------------------------------------------------------------------------------------
// Adds a game to the Game Explorer
//--------------------------------------------------------------------------------------
STDAPI AddToGameExplorerW( WCHAR* strGDFBinPath, WCHAR* strGameInstallPath, 
                           GAME_INSTALL_SCOPE InstallScope, GUID* pInstanceGUID )
{
    HRESULT hr = E_FAIL;
    bool    bCleanupCOM = false;
    BOOL    bHasAccess = FALSE;
    BSTR    bstrGDFBinPath = NULL;
    BSTR    bstrGameInstallPath = NULL;
    IGameExplorer* pFwGameExplorer = NULL;

    if( strGDFBinPath == NULL || strGameInstallPath == NULL || pInstanceGUID == NULL )
    {
        assert( false );
        return E_INVALIDARG;
    }

    bstrGDFBinPath = SysAllocString( strGDFBinPath );
    bstrGameInstallPath = SysAllocString( strGameInstallPath );
    if( bstrGDFBinPath == NULL || bstrGameInstallPath == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto LCleanup;
    }

    hr = CoInitialize( 0 );
    bCleanupCOM = SUCCEEDED(hr); 

    // Create an instance of the Game Explorer Interface
    hr = CoCreateInstance( __uuidof(GameExplorer), NULL, CLSCTX_INPROC_SERVER, 
                           __uuidof(IGameExplorer), (void**) &pFwGameExplorer );
    if( FAILED(hr) || pFwGameExplorer == NULL )
    {
        // On Windows XP or eariler, write registry keys to known location 
        // so that if the machine is upgraded to Windows Vista or later, these games will 
        // be automatically found.
        // 
        // Depending on GAME_INSTALL_SCOPE, write to:
        //      HKLM\Software\Microsoft\Windows\CurrentVersion\GameUX\GamesToFindOnWindowsUpgrade\{GUID}\
        // or
        //      HKCU\Software\Classes\Software\Microsoft\Windows\CurrentVersion\GameUX\GamesToFindOnWindowsUpgrade\{GUID}\
        // and write there these 2 string values: GDFBinaryPath and GameInstallPath 
        //
        HKEY hKeyGamesToFind = NULL, hKeyGame = NULL;
        LONG lResult;
        DWORD dwDisposition;
        if( InstallScope == GIS_CURRENT_USER )
            lResult = RegCreateKeyEx( HKEY_CURRENT_USER, L"Software\\Classes\\Software\\Microsoft\\Windows\\CurrentVersion\\GameUX\\GamesToFindOnWindowsUpgrade", 
                                      0, NULL, 0, KEY_WRITE, NULL, &hKeyGamesToFind, &dwDisposition );
        else
            lResult = RegCreateKeyEx( HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\GameUX\\GamesToFindOnWindowsUpgrade", 
                                      0, NULL, 0, KEY_WRITE, NULL, &hKeyGamesToFind, &dwDisposition );

#ifdef SHOW_S1_DEBUG_MSGBOXES
    WCHAR sz[1024];
    StringCchPrintf( sz, 1024, L"RegCreateKeyEx lResult=%d", lResult );
    MessageBox( NULL, sz, L"AddToGameExplorerW - WinXP Path", MB_OK );
#endif

        if( ERROR_SUCCESS == lResult ) 
        {
            WCHAR strGameInstanceGUID[128] = {0};
            if( *pInstanceGUID == GUID_NULL )
                GenerateGUID( pInstanceGUID );
            hr = StringFromGUID2( *pInstanceGUID, strGameInstanceGUID, 128 );

            if( SUCCEEDED(hr) )
            {
                lResult = RegCreateKeyEx( hKeyGamesToFind, strGameInstanceGUID, 0, NULL, 0, KEY_WRITE, NULL, &hKeyGame, &dwDisposition );
                if( ERROR_SUCCESS == lResult ) 
                {
                    size_t nGDFBinPath = 0, nGameInstallPath = 0;
                    StringCchLength( strGDFBinPath, MAX_PATH, &nGDFBinPath );
                    StringCchLength( strGameInstallPath, MAX_PATH, &nGameInstallPath );
                    RegSetValueEx( hKeyGame, L"GDFBinaryPath", 0, REG_SZ, (BYTE*)strGDFBinPath, (DWORD)((nGDFBinPath + 1)*sizeof(WCHAR)) );
                    RegSetValueEx( hKeyGame, L"GameInstallPath", 0, REG_SZ, (BYTE*)strGameInstallPath, (DWORD)((nGameInstallPath + 1)*sizeof(WCHAR)) );
                }
                if( hKeyGame ) RegCloseKey( hKeyGame );
            }
        }
        if( hKeyGamesToFind ) RegCloseKey( hKeyGamesToFind );
    }
    else
    {
        hr = pFwGameExplorer->VerifyAccess( bstrGDFBinPath, &bHasAccess );
        if( FAILED(hr) || !bHasAccess )
            goto LCleanup;

        hr = pFwGameExplorer->AddGame( bstrGDFBinPath, bstrGameInstallPath, 
            InstallScope, pInstanceGUID );

        if( FAILED(hr) )
        {
            // If AddGame() failed, then the game may have been already added.
            // So try to retrieve existing game instance GUID
            RetrieveGUIDForApplicationW( bstrGDFBinPath, pInstanceGUID );
        }

#ifdef SHOW_S1_DEBUG_MSGBOXES
    WCHAR strUser[256];
    WCHAR strDomain[256];
    GetAccountName( strUser, 256, strDomain, 256 );
    BOOL bAdmin = IsUserAnAdmin();

    WCHAR sz[1024];
    WCHAR strGameInstanceGUID[128] = {0};
    StringFromGUID2( *pInstanceGUID, strGameInstanceGUID, 128 );
    StringCchPrintf( sz, 1024, L"szGDFBinPath='%s'\nszGameInstallPath='%s'\nInstallScope='%s'\nszGUID='%s'\nAccount=%s\\%s\nAdmin=%d\nhr=0x%0.8x", 
                     bstrGDFBinPath, bstrGameInstallPath, (InstallScope == GIS_ALL_USERS) ? L"GIS_ALL_USERS" : L"GIS_CURRENT_USER", strGameInstanceGUID, strDomain, strUser, bAdmin, hr );
    MessageBox( NULL, sz, L"AddToGameExplorerW - Vista Path", MB_OK );
#endif
    }

LCleanup:
    if( bstrGDFBinPath ) SysFreeString( bstrGDFBinPath );
    if( bstrGameInstallPath ) SysFreeString( bstrGameInstallPath );
    if( pFwGameExplorer ) pFwGameExplorer->Release();
    if( bCleanupCOM ) CoUninitialize();

    return hr;
}


//--------------------------------------------------------------------------------------
// Removes a game from the Game Explorer
//--------------------------------------------------------------------------------------
STDAPI RemoveFromGameExplorer( GUID *pInstanceGUID )
{   
    HRESULT hr = E_FAIL;
    IGameExplorer* pFwGameExplorer = NULL;

    hr = CoInitialize( 0 );
    bool bCleanupCOM = SUCCEEDED(hr); 
	bool bVistaPath = false;

    // Create an instance of the Game Explorer Interface
    hr = CoCreateInstance( __uuidof(GameExplorer), NULL, CLSCTX_INPROC_SERVER, 
                      __uuidof(IGameExplorer), (void**) &pFwGameExplorer );
    if( SUCCEEDED(hr) ) 
    {
		bVistaPath = true;
        // Remove the game from the Game Explorer
        hr = pFwGameExplorer->RemoveGame( *pInstanceGUID );
    }
    else 
    {
        // On Windows XP remove reg keys
        if( pInstanceGUID == NULL )
            goto LCleanup;

        WCHAR strGameInstanceGUID[128] = {0};
        hr = StringFromGUID2( *pInstanceGUID, strGameInstanceGUID, 128 );
        if( FAILED(hr) )
            goto LCleanup;

        WCHAR szKeyPath[1024];
        if( SUCCEEDED( StringCchPrintf( szKeyPath, 1024, L"Software\\Classes\\Software\\Microsoft\\Windows\\CurrentVersion\\GameUX\\GamesToFindOnWindowsUpgrade\\%s", strGameInstanceGUID ) ) )
            SHDeleteKey( HKEY_CURRENT_USER, szKeyPath );

        if( SUCCEEDED( StringCchPrintf( szKeyPath, 1024, L"Software\\Microsoft\\Windows\\CurrentVersion\\GameUX\\GamesToFindOnWindowsUpgrade\\%s", strGameInstanceGUID ) ) )
            SHDeleteKey( HKEY_LOCAL_MACHINE, szKeyPath );

        hr = S_OK;
        goto LCleanup;
    }

#ifdef SHOW_S1_DEBUG_MSGBOXES
    WCHAR sz[1024];
    WCHAR strGameInstanceGUID[128] = {0};
    StringFromGUID2( *pInstanceGUID, strGameInstanceGUID, 128 );
    StringCchPrintf( sz, 1024, L"bVistaPath=%d\npInstanceGUID='%s'\nhr=0x%0.8x", bVistaPath, strGameInstanceGUID, hr );
    MessageBox( NULL, sz, L"RemoveFromGameExplorer", MB_OK );
#endif

LCleanup:
    if( pFwGameExplorer ) pFwGameExplorer->Release();
    if( bCleanupCOM ) CoUninitialize();

    return hr;
}


//--------------------------------------------------------------------------------------
// Adds application from exception list
//--------------------------------------------------------------------------------------
STDAPI AddToGameExplorerA( CHAR* strGDFBinPath, CHAR* strGameInstallPath, 
                           GAME_INSTALL_SCOPE InstallScope, GUID* pInstanceGUID )
{
    WCHAR wstrBinPath[MAX_PATH] = {0};
    WCHAR wstrInstallPath[MAX_PATH] = {0};

    MultiByteToWideChar(CP_ACP, 0, strGDFBinPath, MAX_PATH, wstrBinPath, MAX_PATH);
    MultiByteToWideChar(CP_ACP, 0, strGameInstallPath, MAX_PATH, wstrInstallPath, MAX_PATH);

    return AddToGameExplorerW( wstrBinPath, wstrInstallPath, InstallScope, pInstanceGUID );
}


//--------------------------------------------------------------------------------------
// Generates a random GUID
//--------------------------------------------------------------------------------------
STDAPI GenerateGUID( GUID *pInstanceGUID ) 
{
    return CoCreateGuid( pInstanceGUID ); 
}


//--------------------------------------------------------------------------------------
// Gets a property from MSI.  Deferred custom action can only access the property called
// "CustomActionData"
//--------------------------------------------------------------------------------------
LPWSTR GetPropertyFromMSI( MSIHANDLE hMSI, LPCWSTR szPropName )
{
    DWORD	dwSize = 0, dwBufferLen = 0;
    LPWSTR szValue = NULL;

    UINT uErr = MsiGetProperty( hMSI, szPropName, L"", &dwSize );
    if( (ERROR_SUCCESS == uErr) || (ERROR_MORE_DATA == uErr) )
    {
        dwSize++; // Add NULL term
        dwBufferLen = dwSize;
        szValue = new WCHAR[ dwBufferLen ];
        if( szValue )
        {
            uErr = MsiGetProperty( hMSI, szPropName, szValue, &dwSize );
            if( (ERROR_SUCCESS != uErr) )
            {
                // Cleanup on failure
                delete[] szValue;
                szValue = NULL;
            } 
            else
            {
                // Make sure buffer is null-terminated
                szValue[ dwBufferLen - 1 ] = '\0';
            }
        } 
    }

    return szValue;
}


//-----------------------------------------------------------------------------
// Converts a string to a GUID
//-----------------------------------------------------------------------------
HRESULT ConvertStringToGUID( const WCHAR* strSrc, GUID* pGuidDest )
{
    UINT aiTmp[10];

    if( swscanf( strSrc, L"{%8X-%4X-%4X-%2X%2X-%2X%2X%2X%2X%2X%2X}",
                    &pGuidDest->Data1, 
                    &aiTmp[0], &aiTmp[1], 
                    &aiTmp[2], &aiTmp[3],
                    &aiTmp[4], &aiTmp[5],
                    &aiTmp[6], &aiTmp[7],
                    &aiTmp[8], &aiTmp[9] ) != 11 )
    {
        ZeroMemory( pGuidDest, sizeof(GUID) );
        return E_FAIL;
    }
    else
    {
        pGuidDest->Data2       = (USHORT) aiTmp[0];
        pGuidDest->Data3       = (USHORT) aiTmp[1];
        pGuidDest->Data4[0]    = (BYTE) aiTmp[2];
        pGuidDest->Data4[1]    = (BYTE) aiTmp[3];
        pGuidDest->Data4[2]    = (BYTE) aiTmp[4];
        pGuidDest->Data4[3]    = (BYTE) aiTmp[5];
        pGuidDest->Data4[4]    = (BYTE) aiTmp[6];
        pGuidDest->Data4[5]    = (BYTE) aiTmp[7];
        pGuidDest->Data4[6]    = (BYTE) aiTmp[8];
        pGuidDest->Data4[7]    = (BYTE) aiTmp[9];
        return S_OK;
    }
}


//-----------------------------------------------------------------------------
HRESULT ConvertGUIDToStringCch( const GUID* pGuidSrc, WCHAR* strDest, int cchDestChar )
{
    return StringCchPrintf( strDest, cchDestChar, L"{%0.8X-%0.4X-%0.4X-%0.2X%0.2X-%0.2X%0.2X%0.2X%0.2X%0.2X%0.2X}",
               pGuidSrc->Data1, pGuidSrc->Data2, pGuidSrc->Data3,
               pGuidSrc->Data4[0], pGuidSrc->Data4[1],
               pGuidSrc->Data4[2], pGuidSrc->Data4[3],
               pGuidSrc->Data4[4], pGuidSrc->Data4[5],
               pGuidSrc->Data4[6], pGuidSrc->Data4[7] );
}


//-----------------------------------------------------------------------------
STDAPI RetrieveGUIDForApplicationA( CHAR* szPathToGDFdll, GUID* pGUID )
{
    WCHAR wszPathToGDFdll[MAX_PATH] = {0};
    HRESULT hr;

    MultiByteToWideChar(CP_ACP, 0, szPathToGDFdll, MAX_PATH, wszPathToGDFdll, MAX_PATH);
    hr = RetrieveGUIDForApplicationW( wszPathToGDFdll, pGUID );

    return hr;
}


//-----------------------------------------------------------------------------
// Enums WinXP registry for GDF upgrade keys, and returns the GUID
// based on the GDF binary path
//-----------------------------------------------------------------------------
bool RetrieveGUIDForApplicationOnWinXP( HKEY hKeyRoot, WCHAR* szPathToGDFdll, GUID* pGUID )
{
    DWORD iKey = 0;
    WCHAR strRegKeyName[256];
    WCHAR strGDFBinPath[MAX_PATH];
    HKEY hKey = NULL;
    LONG lResult;
    DWORD dwDisposition, dwType, dwSize;
    bool bFound = false;

    for(;;)
    {
        lResult = RegEnumKey( hKeyRoot, iKey, strRegKeyName, 256 );
        if( lResult != ERROR_SUCCESS  )
            break;

        lResult = RegCreateKeyEx( hKeyRoot, strRegKeyName, 0, NULL, 0, KEY_READ, NULL, &hKey, &dwDisposition );
        if( lResult == ERROR_SUCCESS )
        {
            dwSize = MAX_PATH * sizeof(WCHAR);
            lResult = RegQueryValueEx( hKey, L"GDFBinaryPath", 0, &dwType, (BYTE*)strGDFBinPath, &dwSize );            
            if( lResult == ERROR_SUCCESS )
            {
                if( wcscmp( strGDFBinPath, szPathToGDFdll ) == 0 )
                {
                    bFound = true;
                    ConvertStringToGUID( strRegKeyName, pGUID );
                }
            }
            RegCloseKey( hKey );
        }

        if( bFound )
            break;

        iKey++;
    }

    return bFound;
}

//-----------------------------------------------------------------------------
STDAPI RetrieveGUIDForApplicationW( WCHAR* szPathToGDFdll, GUID* pGUID )
{
    HRESULT hr;
    IWbemLocator*  pIWbemLocator = NULL;
    IWbemServices* pIWbemServices = NULL;
    BSTR           pNamespace = NULL;
    IEnumWbemClassObject* pEnum = NULL;
    bool bFound = false;

    hr = CoInitialize( 0 );
    bool bCleanupCOM = SUCCEEDED(hr); 

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

            // Double up the '\' marks for the WQL query
            WCHAR szDoubleSlash[2048];
            int iDest = 0, iSource = 0;
            for( ;; )
            {
                if( szPathToGDFdll[iSource] == 0 || iDest > 2000 )
                    break;
                szDoubleSlash[iDest] = szPathToGDFdll[iSource];
                if( szPathToGDFdll[iSource] == L'\\' )
                {
                    iDest++; szDoubleSlash[iDest] = L'\\';
                }
                iDest++;
                iSource++;
            }
            szDoubleSlash[iDest] = 0;

            WCHAR szQuery[1024];
            StringCchPrintf( szQuery, 1024, L"SELECT * FROM GAME WHERE GDFBinaryPath = \"%s\"", szDoubleSlash );
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
                hr = pEnum->Next( 5000, 1, &pGameClass, &uReturned );
                if( SUCCEEDED(hr) && uReturned != 0 && pGameClass != NULL )
                {
                    VARIANT var;

                    // Get the InstanceID string
                    pPropName = SysAllocString( L"InstanceID" );
                    hr = pGameClass->Get( pPropName, 0L, &var, NULL, NULL );
                    if( SUCCEEDED(hr) && var.vt == VT_BSTR )
                    {
                        bFound = true;
                        if( pGUID ) ConvertStringToGUID( var.bstrVal, pGUID );
                    }

                    if( pPropName ) SysFreeString( pPropName );
                }

                SAFE_RELEASE( pGameClass );
            }

            SAFE_RELEASE( pEnum );
        }

        if( pNamespace ) SysFreeString( pNamespace );
        SAFE_RELEASE( pIWbemServices );
    }

    SAFE_RELEASE( pIWbemLocator );

#ifdef SHOW_S2_DEBUG_MSGBOXES
    WCHAR sz[1024];
    StringCchPrintf( sz, 1024, L"szPathToGDFdll=%s bFound=%d", szPathToGDFdll, bFound );
    MessageBox( NULL, sz, L"RetrieveGUIDForApplicationW", MB_OK );
#endif

    if( bCleanupCOM ) CoUninitialize();

    if( !bFound )
    {
        // Look in WinXP regkey paths
        HKEY hKeyRoot;
        LONG lResult;
        DWORD dwDisposition;
        lResult = RegCreateKeyEx( HKEY_CURRENT_USER, L"Software\\Classes\\Software\\Microsoft\\Windows\\CurrentVersion\\GameUX\\GamesToFindOnWindowsUpgrade", 
                                  0, NULL, 0, KEY_READ, NULL, &hKeyRoot, &dwDisposition );
        if( ERROR_SUCCESS == lResult ) 
        {
            bFound = RetrieveGUIDForApplicationOnWinXP( hKeyRoot, szPathToGDFdll, pGUID );            
            RegCloseKey( hKeyRoot );
        }

        if( !bFound )
        {
            lResult = RegCreateKeyEx( HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\GameUX\\GamesToFindOnWindowsUpgrade", 
                                      0, NULL, 0, KEY_READ, NULL, &hKeyRoot, &dwDisposition );
            if( ERROR_SUCCESS == lResult ) 
            {
                bFound = RetrieveGUIDForApplicationOnWinXP( hKeyRoot, szPathToGDFdll, pGUID );            
                RegCloseKey( hKeyRoot );
            }
        }
    }

    return (bFound) ? S_OK : E_FAIL;
}


//-----------------------------------------------------------------------------
STDAPI CreateTaskA( GAME_INSTALL_SCOPE InstallScope, GUID* pGameInstanceGUID, BOOL bSupportTask,                
                    int nTaskID, CHAR* strTaskName, CHAR* strLaunchPath, CHAR* strCommandLineArgs )         
{
    WCHAR wstrTaskName[MAX_PATH] = {0};
    WCHAR wstrLaunchPath[MAX_PATH] = {0};
    WCHAR wstrCommandLineArgs[MAX_PATH] = {0};

    MultiByteToWideChar(CP_ACP, 0, strTaskName, MAX_PATH, wstrTaskName, MAX_PATH);
    MultiByteToWideChar(CP_ACP, 0, strLaunchPath, MAX_PATH, wstrLaunchPath, MAX_PATH);
    MultiByteToWideChar(CP_ACP, 0, strCommandLineArgs, MAX_PATH, wstrCommandLineArgs, MAX_PATH);

    return CreateTaskW( InstallScope, pGameInstanceGUID, bSupportTask, nTaskID, 
                        wstrTaskName, wstrLaunchPath, wstrCommandLineArgs );
}


//-----------------------------------------------------------------------------
HRESULT CreateShortcut( WCHAR* strLaunchPath, WCHAR* strCommandLineArgs, WCHAR* strShortcutFilePath )
{
    HRESULT hr;

    hr = CoInitialize( 0 );
    bool bCleanupCOM = SUCCEEDED(hr); 
    
    IShellLink* psl; 
    hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, 
                          IID_IShellLink, (LPVOID*)&psl); 
    if (SUCCEEDED(hr)) 
    { 
        // Setup shortcut
        psl->SetPath( strLaunchPath ); 
        if( strCommandLineArgs ) psl->SetArguments( strCommandLineArgs ); 

        // These shortcut settings aren't needed for tasks
        // if( strIconPath ) psl->SetIconLocation( strIconPath, nIcon );
        // if( wHotkey ) psl->SetHotkey( wHotkey );
        // if( nShowCmd ) psl->SetShowCmd( nShowCmd );
        // if( strDescription ) psl->SetDescription( strDescription );

        // Set working dir to path of launch exe
        WCHAR strFullPath[512];
        WCHAR* strExePart; 
        GetFullPathName( strLaunchPath, 512, strFullPath, &strExePart );
        if( strExePart ) *strExePart = 0;
        psl->SetWorkingDirectory( strFullPath );

        // Save shortcut to file
        IPersistFile* ppf; 
        hr = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf); 
        if (SUCCEEDED(hr)) 
        { 
            hr = ppf->Save( strShortcutFilePath, TRUE ); 
            ppf->Release(); 
        } 
        psl->Release(); 
    } 

    if( bCleanupCOM ) CoUninitialize();

    return hr;
}


//-----------------------------------------------------------------------------
STDAPI CreateTaskW( GAME_INSTALL_SCOPE InstallScope,   // Either GIS_CURRENT_USER or GIS_ALL_USERS 
                    GUID* pGameInstanceGUID,           // valid GameInstance GUID that was passed to AddGame()
                    BOOL bSupportTask,                 // if TRUE, this is a support task otherwise it is a play task
                    int nTaskID,                       // ID of task
                    WCHAR* strTaskName,                // Name of task.  Ex "Play"
                    WCHAR* strLaunchPath,              // Path to exe.  Example: "C:\Program Files\Microsoft\MyGame.exe"
                    WCHAR* strCommandLineArgs )        // Can be NULL.  Example: "-windowed"
{
    HRESULT hr;
    WCHAR strPath[512];
    WCHAR strGUID[256];
    WCHAR strCommonFolder[MAX_PATH];
    WCHAR strShortcutFilePath[512];

    // Get base path based on install scope
    if( InstallScope == GIS_CURRENT_USER )
        SHGetFolderPath( NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, strCommonFolder );
    else
        SHGetFolderPath( NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, strCommonFolder );

    // Convert GUID to string
    hr = StringFromGUID2( *pGameInstanceGUID, strGUID, 256 );
    if( FAILED(hr) )
        return hr;

    // Create dir path for shortcut
    StringCchPrintf( strPath, 512, L"%s\\Microsoft\\Windows\\GameExplorer\\%s\\%s\\%d", 
                     strCommonFolder, strGUID, (bSupportTask) ? L"SupportTasks" : L"PlayTasks", nTaskID );

    // Create the directory and all intermediate directories
    SHCreateDirectoryEx( NULL, strPath, NULL ); 

    // Create full file path to shortcut 
    StringCchPrintf( strShortcutFilePath, 512, L"%s\\%s.lnk", strPath, strTaskName );

#ifdef SHOW_S2_DEBUG_MSGBOXES
    WCHAR sz[1024];
    StringCchPrintf( sz, 1024, L"strShortcutFilePath='%s' strTaskName='%s'", strShortcutFilePath, strTaskName );
    MessageBox( NULL, sz, L"CreateTaskW", MB_OK );
#endif

    // Create shortcut
    CreateShortcut( strLaunchPath, strCommandLineArgs, strShortcutFilePath );

    return S_OK;
}


//-----------------------------------------------------------------------------
STDAPI RemoveTasks( GUID* pGUID ) // valid GameInstance GUID that was passed to AddGame()
{
    HRESULT hr;
    WCHAR strPath[512] = {0};
    WCHAR strGUID[256];
    WCHAR strLocalAppData[MAX_PATH];
    WCHAR strCommonAppData[MAX_PATH];

    // Get base path based on install scope
    if( FAILED( hr = SHGetFolderPath( NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, strLocalAppData ) ) )
        return hr;

    if( FAILED( hr = SHGetFolderPath( NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, strCommonAppData ) ) )
        return hr;

    // Convert GUID to string
    if( FAILED( hr = StringFromGUID2( *pGUID, strGUID, 256 ) ) )
        return hr;

    if( FAILED( hr = StringCchPrintf( strPath, 512, L"%s\\Microsoft\\Windows\\GameExplorer\\%s", strLocalAppData, strGUID ) ) )
        return hr;

    SHFILEOPSTRUCT fileOp;
    ZeroMemory( &fileOp, sizeof(SHFILEOPSTRUCT) );
    fileOp.wFunc = FO_DELETE;
    fileOp.pFrom = strPath;
    fileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
    SHFileOperation( &fileOp );

    if( FAILED( hr = StringCchPrintf( strPath, 512, L"%s\\Microsoft\\Windows\\GameExplorer\\%s", strCommonAppData, strGUID ) ) )
        return hr;

    ZeroMemory( &fileOp, sizeof(SHFILEOPSTRUCT) );
    fileOp.wFunc = FO_DELETE;
    fileOp.pFrom = strPath;
    fileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
    SHFileOperation( &fileOp );

    return S_OK;
}


//-----------------------------------------------------------------------------
// Creates the registry keys to enable rich saved games.  The game still need to use 
// the rich saved game header as defined in the documention and support loading a 
// saved game from the command line.
//
// strSavedGameExtension should begin with a period. ex: .ExampleSaveGame
// strLaunchPath should be enclosed in quotes.  ex: "%ProgramFiles%\ExampleGame\ExampleGame.exe"
// strCommandLineToLaunchSavedGame should be enclosed in quotes.  ex: "%1".  If NULL, it defaults to "%1"
//-----------------------------------------------------------------------------
STDAPI SetupRichSavedGamesW( WCHAR* strSavedGameExtension, WCHAR* strLaunchPath, 
                             WCHAR* strCommandLineToLaunchSavedGame ) 
{
    HKEY hKey = NULL;
    LONG lResult;
    DWORD dwDisposition;
    WCHAR strExt[256];
    WCHAR strType[256];
    WCHAR strCmdLine[256];
    WCHAR strTemp[512];
    size_t nStrLength = 0;

    // Validate args 
    if( strLaunchPath == NULL || strSavedGameExtension == NULL )
    {
        assert( false );
        return E_INVALIDARG;
    }
    
    // Setup saved game extension arg - make sure there's a period at the start
    if( strSavedGameExtension[0] == L'.' )
    {
        StringCchCopy( strExt, 256, strSavedGameExtension );
        StringCchPrintf( strType, 256, L"%sType", strSavedGameExtension+1 );
    }
    else
    {
        StringCchPrintf( strExt, 256, L".%s", strSavedGameExtension );
        StringCchPrintf( strType, 256, L"%sType", strSavedGameExtension );
    }

    // Create default command line arg if none supplied
    if( strCommandLineToLaunchSavedGame )
        StringCchCopy( strCmdLine, 256, strCommandLineToLaunchSavedGame );
    else
        StringCchCopy( strCmdLine, 256, L"\"%1\"" );

    // Create file association & metadata regkeys
    lResult = RegCreateKeyEx( HKEY_CLASSES_ROOT, strExt, 0, NULL, 0, KEY_WRITE, NULL, &hKey, &dwDisposition );
    if( ERROR_SUCCESS == lResult ) 
    {
        // Create the following regkeys:
        //
        // [HKEY_CLASSES_ROOT\.ExampleGameSave]
        // (Default)="ExampleGameSaveFileType"
        //
        StringCchLength( strType, 256, &nStrLength );
        RegSetValueEx( hKey, L"", 0, REG_SZ, (BYTE*)strType, (DWORD)((nStrLength + 1)*sizeof(WCHAR)) );

        // Create the following regkeys:
        //
        // [HKEY_CLASSES_ROOT\.ExampleGameSave\ShellEx\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}]
        // (Default)="{4E5BFBF8-F59A-4e87-9805-1F9B42CC254A}"
        //
        HKEY hSubKey = NULL;
        lResult = RegCreateKeyEx( hKey, L"ShellEx\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", 0, NULL, 0, KEY_WRITE, NULL, &hSubKey, &dwDisposition );
        if( ERROR_SUCCESS == lResult ) 
        {
            StringCchPrintf( strTemp, 512, L"{4E5BFBF8-F59A-4e87-9805-1F9B42CC254A}" );
            StringCchLength( strTemp, 256, &nStrLength );
            RegSetValueEx( hSubKey, L"", 0, REG_SZ, (BYTE*)strTemp, (DWORD)((nStrLength + 1)*sizeof(WCHAR)) );
        }
        if( hSubKey ) RegCloseKey( hSubKey );
    }
    if( hKey ) RegCloseKey( hKey );

    lResult = RegCreateKeyEx( HKEY_CLASSES_ROOT, strType, 0, NULL, 0, KEY_WRITE, NULL, &hKey, &dwDisposition );
    if( ERROR_SUCCESS == lResult ) 
    {
        // Create the following regkeys:
        //
        // [HKEY_CLASSES_ROOT\ExampleGameSaveFileType]
        // PreviewTitle="prop:System.Game.RichSaveName;System.Game.RichApplicationName"
        // PreviewDetails="prop:System.Game.RichLevel;System.DateChanged;System.Game.RichComment;System.DisplayName;System.DisplayType"
        //
        size_t nPreviewDetails = 0, nPreviewTitle = 0;
        WCHAR* strPreviewTitle = L"prop:System.Game.RichSaveName;System.Game.RichApplicationName";
        WCHAR* strPreviewDetails = L"prop:System.Game.RichLevel;System.DateChanged;System.Game.RichComment;System.DisplayName;System.DisplayType";
        StringCchLength( strPreviewTitle, 256, &nPreviewTitle );
        StringCchLength( strPreviewDetails, 256, &nPreviewDetails );
        RegSetValueEx( hKey, L"PreviewTitle", 0, REG_SZ, (BYTE*)strPreviewTitle, (DWORD)((nPreviewTitle + 1)*sizeof(WCHAR)) );
        RegSetValueEx( hKey, L"PreviewDetails", 0, REG_SZ, (BYTE*)strPreviewDetails, (DWORD)((nPreviewDetails + 1)*sizeof(WCHAR)) );

        // Create the following regkeys:
        //
        // [HKEY_CLASSES_ROOT\ExampleGameSaveFileType\Shell\Open\Command]
        // (Default)=""%ProgramFiles%\ExampleGame.exe" "%1""
        //
        HKEY hSubKey = NULL;
        lResult = RegCreateKeyEx( hKey, L"Shell\\Open\\Command", 0, NULL, 0, KEY_WRITE, NULL, &hSubKey, &dwDisposition );
        if( ERROR_SUCCESS == lResult ) 
        {
            StringCchPrintf( strTemp, 512, L"%s %s", strLaunchPath, strCmdLine );
            StringCchLength( strTemp, 256, &nStrLength );
            RegSetValueEx( hSubKey, L"", 0, REG_SZ, (BYTE*)strTemp, (DWORD)((nStrLength + 1)*sizeof(WCHAR)) );
        }
        if( hSubKey ) RegCloseKey( hSubKey );
    }
    if( hKey ) RegCloseKey( hKey );

    // Create the following regkeys:
    //
    // [HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers\.ExampleGameSave]
    // (Default)="{ECDD6472-2B9B-4b4b-AE36-F316DF3C8D60}"
    //
    StringCchPrintf( strTemp, 512, L"Software\\Microsoft\\Windows\\CurrentVersion\\PropertySystem\\PropertyHandlers\\%s", strExt );
    lResult = RegCreateKeyEx( HKEY_LOCAL_MACHINE, strTemp, 0, NULL, 0, KEY_WRITE, NULL, &hKey, &dwDisposition );
    if( ERROR_SUCCESS == lResult ) 
    {
        StringCchCopy( strTemp, 512, L"{ECDD6472-2B9B-4B4B-AE36-F316DF3C8D60}" );
        StringCchLength( strTemp, 256, &nStrLength );
        RegSetValueEx( hKey, L"", 0, REG_SZ, (BYTE*)strTemp, (DWORD)((nStrLength + 1)*sizeof(WCHAR)) );
    }
    if( hKey ) RegCloseKey( hKey );

    return S_OK;
}


//-----------------------------------------------------------------------------
// Removes the registry keys to enable rich saved games.  
//
// strSavedGameExtension should begin with a period. ex: .ExampleSaveGame
//-----------------------------------------------------------------------------
STDAPI RemoveRichSavedGamesW( WCHAR* strSavedGameExtension ) 
{
    WCHAR strExt[256];
    WCHAR strType[256];
    WCHAR strTemp[512];

    // Validate args 
    if( strSavedGameExtension == NULL )
    {
        assert( false );
        return E_INVALIDARG;
    }
    
    // Setup saved game extension arg - make sure there's a period at the start
    if( strSavedGameExtension[0] == L'.' )
    {
        StringCchCopy( strExt, 256, strSavedGameExtension );
        StringCchPrintf( strType, 256, L"%sType", strSavedGameExtension+1 );
    }
    else
    {
        StringCchPrintf( strExt, 256, L".%s", strSavedGameExtension );
        StringCchPrintf( strType, 256, L"%sType", strSavedGameExtension );
    }

    // Delete the following regkeys:
    //
    // [HKEY_CLASSES_ROOT\.ExampleGameSave]
    // [HKEY_CLASSES_ROOT\ExampleGameSaveFileType]
    // [HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers\.ExampleGameSave]
    SHDeleteKey( HKEY_CLASSES_ROOT, strExt );
    SHDeleteKey( HKEY_CLASSES_ROOT, strType );
    StringCchPrintf( strTemp, 512, L"Software\\Microsoft\\Windows\\CurrentVersion\\PropertySystem\\PropertyHandlers\\%s", strExt );
    SHDeleteKey( HKEY_LOCAL_MACHINE, strTemp );

    return S_OK;
}


//-----------------------------------------------------------------------------
STDAPI SetupRichSavedGamesA( CHAR* strSavedGameExtension, CHAR* strLaunchPath, CHAR* strCommandLineToLaunchSavedGame )
{
    WCHAR wstrSavedGameExtension[MAX_PATH] = {0};
    WCHAR wstrLaunchPath[MAX_PATH] = {0};
    WCHAR wstrCommandLineToLaunchSavedGame[MAX_PATH] = {0};

    MultiByteToWideChar(CP_ACP, 0, strSavedGameExtension, MAX_PATH, wstrSavedGameExtension, MAX_PATH);
    MultiByteToWideChar(CP_ACP, 0, strLaunchPath, MAX_PATH, wstrLaunchPath, MAX_PATH);
    MultiByteToWideChar(CP_ACP, 0, strCommandLineToLaunchSavedGame, MAX_PATH, wstrCommandLineToLaunchSavedGame, MAX_PATH);

    return SetupRichSavedGamesW( wstrSavedGameExtension, wstrLaunchPath, wstrCommandLineToLaunchSavedGame );
}


//-----------------------------------------------------------------------------
STDAPI RemoveRichSavedGamesA( CHAR* strSavedGameExtension ) 
{
    WCHAR wstrSavedGameExtension[MAX_PATH] = {0};
    MultiByteToWideChar(CP_ACP, 0, strSavedGameExtension, MAX_PATH, wstrSavedGameExtension, MAX_PATH);
    return RemoveRichSavedGamesW( wstrSavedGameExtension );
}


//-----------------------------------------------------------------------------
// Debug function to return account name of calling process
//-----------------------------------------------------------------------------
HRESULT GetAccountName( WCHAR* strUser, DWORD cchUser, WCHAR* strDomain, DWORD cchDomain )
{
    HRESULT hr = E_FAIL;
    WCHAR strMachine[256];
    DWORD cchMachine = 256;
    GetComputerName( strMachine, &cchMachine );

    WTS_PROCESS_INFOW* pProcessInfo = NULL;
    HANDLE hServer = WTSOpenServer(strMachine); 
    DWORD dwCount = 0; 
    DWORD dwCurrentProcessId = GetCurrentProcessId();
    if( WTSEnumerateProcesses(hServer, 0, 1, &pProcessInfo, &dwCount) )
    {
        for(DWORD n = 0; n < dwCount; n++) 
        { 
            if( pProcessInfo[n].ProcessId == dwCurrentProcessId )
            {
                SID_NAME_USE eUse;
                BOOL bSuccess = LookupAccountSid( NULL, pProcessInfo[n].pUserSid, strUser, &cchUser, strDomain, &cchDomain, &eUse ); 
                if( bSuccess )
                    hr = S_OK;
                break;
            }
        } 
    }
    WTSFreeMemory( pProcessInfo );
    WTSCloseServer(hServer); 
    return hr;
}


