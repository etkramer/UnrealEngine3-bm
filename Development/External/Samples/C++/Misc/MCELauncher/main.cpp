//----------------------------------------------------------------------------
// File: main.cpp
//
// See the "Introduction to the 10-Foot Experience for Windows Game Developers"
// for more details about how to use this.
//
// Copyright (c) Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#include "main.h"

//-----------------------------------------------------------------------------
struct SETTINGS
{
    bool bFileNameSupplied;
    bool bArgsSupplied;
    WCHAR strFile[1024];
    WCHAR strArgs[1024];
};

//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
HRESULT CreateD3DDevice( HWND hWnd );
bool ParseCommandLine( SETTINGS* pSettings );
bool GetCmdParam( WCHAR*& strsettings, WCHAR* strFlag, int nFlagLen );
bool IsNextArg( WCHAR*& strsettings, WCHAR* strArg );
bool GetValidFullscreenMode( IDirect3D9* pD3D, UINT adapterOrdinal, D3DDISPLAYMODE* pMode );
void DisplayUsage();
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );


//-----------------------------------------------------------------------------
// Name: main()
// Desc: Entry point for the application.  We use just the console window
//-----------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE hInst, HINSTANCE, LPWSTR, INT )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    int nRet = 0;
    SETTINGS settings;
    ZeroMemory( &settings, sizeof(SETTINGS) );
    ParseCommandLine( &settings );

    if( !settings.bFileNameSupplied )
    {
        DisplayUsage();
        return 0;
    }

    // Register the window class
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, 
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      L"MCE Launcher", NULL };
    RegisterClassEx( &wc );

    // Create the application's window
    HWND hWnd = CreateWindow( L"MCE Launcher", L"MCE Launcher", 
                              WS_POPUP|WS_SYSMENU, 100, 100, 300, 300,
                              NULL, NULL, wc.hInstance, NULL );


    bool bGotMsg;
    MSG  msg;
    msg.message = WM_NULL;
    PeekMessage( &msg, NULL, 0U, 0U, PM_NOREMOVE );

    while( WM_QUIT != msg.message  )
    {
        // Use PeekMessage() so we can use idle time to render the scene. 
        bGotMsg = ( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) != 0 );

        if( bGotMsg )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            Sleep(50);

            HRESULT hr = CreateD3DDevice( hWnd );
            if( SUCCEEDED(hr) || hr == D3DERR_NOTAVAILABLE )
            {
                // Launch file now.  Create device either succeeded or 
                // D3D HW acceleration isn't availble
                ShellExecute( NULL, NULL, settings.strFile,     
                              settings.bArgsSupplied ? settings.strArgs : NULL,
                              NULL, SW_SHOWNORMAL );
                break;
            }
        }
    }

    UnregisterClass( L"MCE Launcher", wc.hInstance );

    return nRet;
}


//--------------------------------------------------------------------------------------
// Parses the command line for parameters.  See DXUTInit() for list 
//--------------------------------------------------------------------------------------
bool ParseCommandLine( SETTINGS* pSettings )
{
    WCHAR* strCmdLine;
    int nNumArgs;
    WCHAR** pstrArgList = CommandLineToArgvW( GetCommandLine(), &nNumArgs );
    for( int iArg=1; iArg<nNumArgs; iArg++ )
    {
        strCmdLine = pstrArgList[iArg];

        if( !pSettings->bFileNameSupplied )
        {
            // First arg must be the filename
            StringCchCopy( pSettings->strFile, 1024, strCmdLine );
            pSettings->bFileNameSupplied = true;
        }
        else
        {
            // Next args are the command line.  Cat them together into a single string
            if( !pSettings->bArgsSupplied  )
            {
                StringCchCopy( pSettings->strArgs, 1024, strCmdLine );
                pSettings->bArgsSupplied = true;
            }
            else
            {
                StringCchCat( pSettings->strArgs, 1024, L" " );
                StringCchCat( pSettings->strArgs, 1024, strCmdLine );
            }
        }
        continue;
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


//-----------------------------------------------------------------------------
void DisplayUsage()
{
    MessageBox( NULL, L"Ripple launcher for fullscreen games installing on the Windows XP Media Center Edition.\nFor more detail, see the \"Introduction to the 10-Foot Experience for Windows Game Developers\" article in the DirectX SDK.\n\nUsage: MCELauncher.exe \"<full exe path>\" <exe args>\n\nNote: use quotes to preseve spaces in filename", L"MCELauncher", MB_OK ); 
}


//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}


//--------------------------------------------------------------------------------------
HRESULT CreateD3DDevice( HWND hWnd )
{
    HRESULT hr;
    IDirect3D9* pD3D = Direct3DCreate9( D3D_SDK_VERSION );
    if( NULL == pD3D )
        return E_FAIL;

    D3DDISPLAYMODE Mode;
    if( !GetValidFullscreenMode( pD3D, D3DADAPTER_DEFAULT, &Mode ) )
        return E_FAIL;

    D3DPRESENT_PARAMETERS pp;
    ZeroMemory( &pp, sizeof(D3DPRESENT_PARAMETERS) ); 
    pp.BackBufferWidth             = Mode.Width;
    pp.BackBufferHeight            = Mode.Height;
    pp.BackBufferFormat            = Mode.Format;
    pp.BackBufferCount             = 1;
    pp.MultiSampleType             = D3DMULTISAMPLE_NONE;  
    pp.MultiSampleQuality          = 0;
    pp.SwapEffect                  = D3DSWAPEFFECT_DISCARD;
    pp.hDeviceWindow               = hWnd;
    pp.Windowed                    = FALSE;
    pp.EnableAutoDepthStencil      = FALSE;  
    pp.Flags                       = 0;                   
    pp.FullScreen_RefreshRateInHz  = 0;
    pp.PresentationInterval        = D3DPRESENT_INTERVAL_DEFAULT;

    IDirect3DDevice9* pd3dDevice;
    hr = pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, 
                             D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, &pd3dDevice );
    SAFE_RELEASE( pD3D );
    if( FAILED(hr) )
        return hr;

    SAFE_RELEASE( pd3dDevice );
    return S_OK;
}


//--------------------------------------------------------------------------------------
bool GetValidFullscreenMode( IDirect3D9* pD3D, UINT adapterOrdinal, D3DDISPLAYMODE* pMode )
{
    D3DDISPLAYMODE desktopMode;
    pD3D->GetAdapterDisplayMode(0, &desktopMode);

    const D3DFORMAT allowedAdapterFormatArray[] = 
    {   
        D3DFMT_X8R8G8B8, 
        D3DFMT_X1R5G5B5, 
        D3DFMT_R5G6B5, 
    };
    const UINT allowedAdapterFormatArrayCount  = sizeof(allowedAdapterFormatArray) / sizeof(allowedAdapterFormatArray[0]);

    bool bFound = false;
    D3DDISPLAYMODE displayMode = {0};
    for( UINT iFormatList = 0; iFormatList < allowedAdapterFormatArrayCount; iFormatList++ )
    {
        D3DFORMAT allowedAdapterFormat = allowedAdapterFormatArray[iFormatList];
        UINT numAdapterModes = pD3D->GetAdapterModeCount( adapterOrdinal, allowedAdapterFormat );
        for (UINT mode = 0; mode < numAdapterModes; mode++)
        {
            pD3D->EnumAdapterModes( adapterOrdinal, allowedAdapterFormat, mode, &displayMode );
            bFound = true;

            if( displayMode.Width == desktopMode.Width && 
                displayMode.Height == desktopMode.Height && 
                displayMode.Format == desktopMode.Format )
            {
                *pMode = desktopMode;
                return true;
            }
        }
    }

    if( !bFound )
        return false;

    *pMode = displayMode;
    return true;
}


