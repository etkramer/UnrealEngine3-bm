//--------------------------------------------------------------------------------------
// File: ProgressiveMesh.cpp
//
// Starting point for new Direct3D applications
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTsettingsdlg.h"
#include "SDKmisc.h"
#include "resource.h"

//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 

#define MESHFILENAME L"dwarf\\dwarf.x"


//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
ID3DXFont*              g_pFont = NULL;         // Font for drawing text
ID3DXSprite*            g_pTextSprite = NULL;   // Sprite for batching draw text calls
ID3DXEffect*            g_pEffect = NULL;       // D3DX effect interface
CModelViewerCamera      g_Camera;               // A model viewing camera
bool                    g_bShowHelp = true;     // If true, it renders the UI control text
CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg         g_SettingsDlg;          // Device settings dialog
CDXUTDialog             g_HUD;                  // dialog for standard controls
CDXUTDialog             g_SampleUI;             // dialog for sample specific controls
ID3DXPMesh**            g_ppPMeshes = NULL;
ID3DXPMesh*             g_pPMeshFull = NULL;
DWORD                   g_cPMeshes = 0;
DWORD                   g_iPMeshCur;
D3DMATERIAL9*           g_mtrlMeshMaterials = NULL;
LPDIRECT3DTEXTURE9*     g_ppMeshTextures = NULL;// Array of textures, entries are NULL if no texture specified
DWORD                   g_dwNumMaterials = 0;   // Number of materials
D3DXMATRIXA16           g_mWorldCenter;
D3DXVECTOR3             g_vObjectCenter;        // Center of bounding sphere of object
FLOAT                   g_fObjectRadius;        // Radius of bounding sphere of object
BOOL                    g_bShowOptimized = true;


//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           3
#define IDC_CHANGEDEVICE        4
#define IDC_DETAIL              5
#define IDC_DETAILLABEL         6
#define IDC_USEOPTIMIZED        7



//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool    CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
bool    CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void    CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
void    CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext );
void    CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void    CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void    CALLBACK OnLostDevice( void* pUserContext );
void    CALLBACK OnDestroyDevice( void* pUserContext );

void    InitApp();
void    RenderText();
void    SetNumVertices( DWORD dwNumVertices );


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // Set the callback functions. These functions allow DXUT to notify
    // the application about device changes, user input, and windows messages.  The 
    // callbacks are optional so you need only set callbacks for events you're interested 
    // in. However, if you don't handle the device reset/lost callbacks then the sample 
    // framework won't be able to reset your device since the application must first 
    // release all device resources before resetting.  Likewise, if you don't handle the 
    // device created/destroyed callbacks then DXUT won't be able to 
    // recreate your device resources.
    DXUTSetCallbackD3D9DeviceAcceptable( IsDeviceAcceptable );
    DXUTSetCallbackD3D9DeviceCreated( OnCreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnResetDevice );
    DXUTSetCallbackD3D9FrameRender( OnFrameRender );
    DXUTSetCallbackD3D9DeviceLost( OnLostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnDestroyDevice );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( KeyboardProc );
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

    // Show the cursor and clip it when in full screen
    DXUTSetCursorSettings( true, true );

    InitApp();

    // Initialize DXUT and create the desired Win32 window and Direct3D 
    // device for the application. Calling each of these functions is optional, but they
    // allow you to set several options which control the behavior of the framework.
    DXUTInit( true, true ); // Parse the command line and show msgboxes
    DXUTSetHotkeyHandling( true, true, true );  // handle the defaul hotkeys
    DXUTCreateWindow( L"ProgressiveMesh: Using Progressive Meshes in Direct3D" );
    DXUTCreateDevice( true, 640, 480 );

    // Pass control to DXUT for handling the message pump and 
    // dispatching render calls. DXUT will call your FrameMove 
    // and FrameRender callback when there is idle time between handling window messages.
    DXUTMainLoop();

    // Perform any application-level cleanup here. Direct3D device resources are released within the
    // appropriate callback functions and therefore don't require any cleanup code here.

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
    // Initialize dialogs
    g_SettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );

    g_HUD.SetCallback( OnGUIEvent ); int iY = 10; 
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, iY, 125, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 35, iY += 24, 125, 22 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, iY += 24, 125, 22, VK_F2 );

    g_SampleUI.EnableKeyboardInput( true );
    g_SampleUI.SetCallback( OnGUIEvent ); iY = 10;
    g_SampleUI.AddStatic( IDC_DETAILLABEL, L"Level of Detail:", 0, iY, 200, 16 );
    g_SampleUI.AddCheckBox( IDC_USEOPTIMIZED, L"Use optimized mesh", 50, iY, 200, 20, true );
    g_SampleUI.AddSlider( IDC_DETAIL, 10, iY += 16, 200, 16, 4, 4, 4 );

    g_Camera.SetButtonMasks( MOUSE_LEFT_BUTTON, MOUSE_WHEEL, 0 );
}


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    // Skip backbuffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3D9Object(); 
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
                    D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    // Must support pixel shader 2.0
    if( pCaps->PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
        return false;

    return true;
}


//--------------------------------------------------------------------------------------
// This callback function is called immediately before a device is created to allow the 
// application to modify the device settings. The supplied pDeviceSettings parameter 
// contains the settings that the framework has selected for the new device, and the 
// application can make any desired changes directly to this structure.  Note however that 
// DXUT will not correct invalid device settings so care must be taken 
// to return valid device settings, otherwise IDirect3D9::CreateDevice() will fail.  
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    assert( DXUT_D3D9_DEVICE == pDeviceSettings->ver );

    // Turn vsync off
    pDeviceSettings->d3d9.pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    g_SettingsDlg.GetDialogControl()->GetComboBox( DXUTSETTINGSDLG_PRESENT_INTERVAL )->SetEnabled( false );

    HRESULT hr;
    IDirect3D9* pD3D = DXUTGetD3D9Object();
    D3DCAPS9 caps;

    V( pD3D->GetDeviceCaps( pDeviceSettings->d3d9.AdapterOrdinal,
                            pDeviceSettings->d3d9.DeviceType,
                            &caps ) );

    // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
    // then switch to SWVP.
    if( ( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
         caps.VertexShaderVersion < D3DVS_VERSION(1,1) )
    {
        pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }

    // Debugging vertex shaders requires either REF or software vertex processing 
    // and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
    if( pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF )
    {
        pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
        pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;                            
        pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
#endif
#ifdef DEBUG_PS
    pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
#endif

    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF )
            DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
    }

    return true;
}


void SetNumVertices( DWORD dwNumVertices )
{
    g_pPMeshFull->SetNumVertices( dwNumVertices );

    // If current pm valid for desired value, then set the number of vertices directly
    if( ( dwNumVertices >= g_ppPMeshes[g_iPMeshCur]->GetMinVertices() ) &&
        ( dwNumVertices <= g_ppPMeshes[g_iPMeshCur]->GetMaxVertices() ) )
    {
        g_ppPMeshes[g_iPMeshCur]->SetNumVertices( dwNumVertices );
    }
    else  // Search for the right one
    {
        g_iPMeshCur = g_cPMeshes - 1;

        // Look for the correct "bin"
        while( g_iPMeshCur > 0 )
        {
            // If number of vertices is less than current max then we found one to fit
            if( dwNumVertices >= g_ppPMeshes[g_iPMeshCur]->GetMinVertices() )
                break;

            g_iPMeshCur -= 1;
        }

        // Set the vertices on the newly selected mesh
        g_ppPMeshes[g_iPMeshCur]->SetNumVertices( dwNumVertices );
    }
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// created, which will happen during application initialization and windowed/full screen 
// toggles. This is the best location to create D3DPOOL_MANAGED resources since these 
// resources need to be reloaded whenever the device is destroyed. Resources created  
// here should be released in the OnDestroyDevice callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;


    V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );
    V_RETURN( g_SettingsDlg.OnD3D9CreateDevice( pd3dDevice ) );
    // Initialize the font
    V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                         OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                         L"Arial", &g_pFont ) );

    // Define DEBUG_VS and/or DEBUG_PS to debug vertex and/or pixel shaders with the 
    // shader debugger. Debugging vertex shaders requires either REF or software vertex 
    // processing, and debugging pixel shaders requires REF.  The 
    // D3DXSHADER_FORCE_*_SOFTWARE_NOOPT flag improves the debug experience in the 
    // shader debugger.  It enables source level debugging, prevents instruction 
    // reordering, prevents dead code elimination, and forces the compiler to compile 
    // against the next higher available software target, which ensures that the 
    // unoptimized shaders do not exceed the shader model limitations.  Setting these 
    // flags will cause slower rendering since the shaders will be unoptimized and 
    // forced into software.  See the DirectX documentation for more information about 
    // using the shader debugger.
    DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;

    #if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DXSHADER_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DXSHADER_DEBUG;
    #endif

    #ifdef DEBUG_VS
        dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
    #endif
    #ifdef DEBUG_PS
        dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
    #endif

    // Read the D3DX effect file
    WCHAR str[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"ProgressiveMesh.fx" ) );

    // If this fails, there should be debug output as to 
    // they the .fx file failed to compile
    V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags, 
                                        NULL, &g_pEffect, NULL ) );

    V_RETURN( g_pEffect->SetTechnique( "RenderScene" ) );

    // Load the mesh
    LPD3DXBUFFER pAdjacencyBuffer = NULL;
    LPD3DXBUFFER pD3DXMtrlBuffer = NULL;
    D3DXMATERIAL* d3dxMaterials;
    DWORD        dw32BitFlag;
    LPD3DXMESH   pMesh = NULL;
    LPD3DXPMESH  pPMesh = NULL;

    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, MESHFILENAME ) );
    V_RETURN( D3DXLoadMeshFromX( str, D3DXMESH_MANAGED, pd3dDevice,
                                 &pAdjacencyBuffer, &pD3DXMtrlBuffer, NULL,
                                 &g_dwNumMaterials, &pMesh ) );

    // Change the current directory to the mesh's directory so we can
    // find the textures.
    WCHAR* pLastSlash = wcsrchr( str, L'\\' );
    if( pLastSlash )
        *(pLastSlash + 1) = 0;
    WCHAR strCWD[MAX_PATH];
    GetCurrentDirectory( MAX_PATH, strCWD );
    SetCurrentDirectory( str );

    dw32BitFlag = ( pMesh->GetOptions() & D3DXMESH_32BIT );

    // Perform simple cleansing operations on mesh
    LPD3DXMESH pTempMesh;
    if( FAILED( hr = D3DXCleanMesh( D3DXCLEAN_SIMPLIFICATION, pMesh, (DWORD*)pAdjacencyBuffer->GetBufferPointer(), &pTempMesh, 
                                    (DWORD*)pAdjacencyBuffer->GetBufferPointer(), NULL ) ) )
    {
        g_dwNumMaterials = 0;
        goto End;
    }
    SAFE_RELEASE( pMesh );
    pMesh = pTempMesh;

    // Perform a weld to try and remove excess vertices.
    // Weld the mesh using all epsilons of 0.0f.  A small epsilon like 1e-6 works well too
    D3DXWELDEPSILONS Epsilons;
    ZeroMemory( &Epsilons, sizeof(D3DXWELDEPSILONS) );
    if( FAILED( hr = D3DXWeldVertices( pMesh, 0, &Epsilons,
                                       (DWORD*)pAdjacencyBuffer->GetBufferPointer(),
                                       (DWORD*)pAdjacencyBuffer->GetBufferPointer(), NULL, NULL ) ) )
    {
        g_dwNumMaterials = 0;
        goto End;
    }

    // Verify validity of mesh for simplification
    if( FAILED( hr = D3DXValidMesh( pMesh, (DWORD*)pAdjacencyBuffer->GetBufferPointer(), NULL ) ) )
    {
        g_dwNumMaterials = 0;
        goto End;
    }

    // Allocate a material/texture arrays
    d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
    g_mtrlMeshMaterials = new D3DMATERIAL9[g_dwNumMaterials];
    g_ppMeshTextures    = new LPDIRECT3DTEXTURE9[g_dwNumMaterials];
    if( g_ppMeshTextures == NULL || g_mtrlMeshMaterials == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto End;
    }

    // Copy the materials and load the textures
    for( UINT i = 0; i < g_dwNumMaterials; i++ )
    {
        g_mtrlMeshMaterials[i] = d3dxMaterials[i].MatD3D;
        g_mtrlMeshMaterials[i].Ambient = g_mtrlMeshMaterials[i].Diffuse;

        // Find the path to the texture and create that texture
        MultiByteToWideChar( CP_ACP, 0, d3dxMaterials[i].pTextureFilename, -1, str, MAX_PATH );
        str[MAX_PATH - 1] = L'\0';
        if( FAILED( D3DXCreateTextureFromFile( pd3dDevice, str, &g_ppMeshTextures[i] ) ) )
            g_ppMeshTextures[i] = NULL;
    }
    SAFE_RELEASE( pD3DXMtrlBuffer );

    // Restore the current directory
    SetCurrentDirectory( strCWD );

    // Lock the vertex buffer, to generate a simple bounding sphere
    LPDIRECT3DVERTEXBUFFER9 pVertexBuffer = NULL;
    void* pVertices;
    hr = pMesh->GetVertexBuffer( &pVertexBuffer );
    if( FAILED(hr) )
        goto End;

    hr = pVertexBuffer->Lock( 0, 0, &pVertices, D3DLOCK_NOSYSLOCK );
    if( FAILED(hr) )
        goto End;

    hr = D3DXComputeBoundingSphere( (D3DXVECTOR3*)pVertices, pMesh->GetNumVertices(),
                                    D3DXGetFVFVertexSize(pMesh->GetFVF()),
                                    &g_vObjectCenter, &g_fObjectRadius );
    pVertexBuffer->Unlock();
    SAFE_RELEASE( pVertexBuffer );

    if( FAILED(hr) )
        goto End;

    {
        D3DXMatrixTranslation( &g_mWorldCenter, -g_vObjectCenter.x,
                                                -g_vObjectCenter.y,
                                                -g_vObjectCenter.z );
        D3DXMATRIXA16 m;
        D3DXMatrixScaling( &m, 2.0f / g_fObjectRadius,
                            2.0f / g_fObjectRadius,
                            2.0f / g_fObjectRadius );
        D3DXMatrixMultiply( &g_mWorldCenter, &g_mWorldCenter, &m );
    }

    // If the mesh is missing normals, generate them.
    if ( !( pMesh->GetFVF() & D3DFVF_NORMAL ) )
    {
        hr = pMesh->CloneMeshFVF( dw32BitFlag | D3DXMESH_MANAGED, pMesh->GetFVF() | D3DFVF_NORMAL,
                                  pd3dDevice, &pTempMesh );
        if( FAILED(hr) )
            goto End;

        D3DXComputeNormals( pTempMesh, NULL );

        pMesh->Release();
        pMesh = pTempMesh;
    }

    // Generate progressive meshes

    hr = D3DXGeneratePMesh( pMesh, (DWORD*)pAdjacencyBuffer->GetBufferPointer(),
                            NULL, NULL, 1, D3DXMESHSIMP_VERTEX, &pPMesh );
    if( FAILED(hr) )
        goto End;

    DWORD cVerticesMin = pPMesh->GetMinVertices();
    DWORD cVerticesMax = pPMesh->GetMaxVertices();

    DWORD cVerticesPerMesh = ( cVerticesMax - cVerticesMin + 10 ) / 10;

    g_cPMeshes = max( 1, (DWORD)ceil( (cVerticesMax - cVerticesMin + 1) / (float)cVerticesPerMesh ) );
    g_ppPMeshes = new LPD3DXPMESH[g_cPMeshes];
    if( g_ppPMeshes == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto End;
    }
    ZeroMemory( g_ppPMeshes, sizeof(LPD3DXPMESH) * g_cPMeshes );

    // Clone full size pmesh
    hr = pPMesh->ClonePMeshFVF( D3DXMESH_MANAGED | D3DXMESH_VB_SHARE, pPMesh->GetFVF(), pd3dDevice, &g_pPMeshFull );
    if( FAILED(hr) )
        goto End;

    // Clone all the separate pmeshes
    for( UINT iPMesh = 0; iPMesh < g_cPMeshes; iPMesh++ )
    {
        hr = pPMesh->ClonePMeshFVF( D3DXMESH_MANAGED | D3DXMESH_VB_SHARE, pPMesh->GetFVF(), pd3dDevice, &g_ppPMeshes[iPMesh] );
        if( FAILED(hr) )
            goto End;

        // Trim to appropriate space
        hr = g_ppPMeshes[iPMesh]->TrimByVertices( cVerticesMin + cVerticesPerMesh * iPMesh, cVerticesMin + cVerticesPerMesh * (iPMesh+1), NULL, NULL);
        if( FAILED(hr) )
            goto End;

        hr = g_ppPMeshes[iPMesh]->OptimizeBaseLOD( D3DXMESHOPT_VERTEXCACHE, NULL );
        if( FAILED(hr) )
            goto End;
    }

    // Set current to be maximum number of vertices
    g_iPMeshCur = g_cPMeshes - 1;
    hr = g_ppPMeshes[g_iPMeshCur]->SetNumVertices( cVerticesMax );
    if( FAILED(hr) )
        goto End;

    hr = g_pPMeshFull->SetNumVertices( cVerticesMax );
    if( FAILED(hr) )
        goto End;

    // Set up the slider to reflect the vertices range the mesh has
    g_SampleUI.GetSlider( IDC_DETAIL )->SetRange( g_ppPMeshes[0]->GetMinVertices(), g_ppPMeshes[g_cPMeshes-1]->GetMaxVertices() );
    g_SampleUI.GetSlider( IDC_DETAIL )->SetValue( g_ppPMeshes[g_iPMeshCur]->GetNumVertices() );

    // Setup the camera's view parameters
    {
        D3DXVECTOR3 vecEye(0.0f, 0.0f, -5.0f);
        D3DXVECTOR3 vecAt (0.0f, 0.0f, -0.0f);
        g_Camera.SetViewParams( &vecEye, &vecAt );
    }

End:
    SAFE_RELEASE( pAdjacencyBuffer );
    SAFE_RELEASE( pD3DXMtrlBuffer );
    SAFE_RELEASE( pMesh );
    SAFE_RELEASE( pPMesh );

    if( FAILED(hr) )
    {
        for( UINT iPMesh = 0; iPMesh < g_cPMeshes; iPMesh++ )
            SAFE_RELEASE( g_ppPMeshes[iPMesh] );

        delete[] g_ppPMeshes;
        g_cPMeshes = 0;
        g_ppPMeshes = NULL;
        SAFE_RELEASE( g_pPMeshFull )
    }

    return hr;
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// reset, which will happen after a lost device scenario. This is the best location to 
// create D3DPOOL_DEFAULT resources since these resources need to be reloaded whenever 
// the device is lost. Resources created here should be released in the OnLostDevice 
// callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, 
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );
    V_RETURN( g_SettingsDlg.OnD3D9ResetDevice() );

    if( g_pFont )
        V_RETURN( g_pFont->OnResetDevice() );
    if( g_pEffect )
        V_RETURN( g_pEffect->OnResetDevice() );

    // Create a sprite to help batch calls when drawing many lines of text
    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pTextSprite ) );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI/4, fAspectRatio, 0.1f, 1000.0f );
    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width-170, 0 );
    g_HUD.SetSize( 170, 170 );
    g_SampleUI.SetLocation( 0, pBackBufferSurfaceDesc->Height-50 );
    g_SampleUI.SetSize( pBackBufferSurfaceDesc->Width, 50 );
    g_SampleUI.GetControl( IDC_DETAILLABEL )->SetLocation( ( pBackBufferSurfaceDesc->Width - 200 ) / 2, 10 );
    g_SampleUI.GetControl( IDC_USEOPTIMIZED )->SetLocation( pBackBufferSurfaceDesc->Width - 130, 5 );
    g_SampleUI.GetControl( IDC_DETAIL )->SetSize( pBackBufferSurfaceDesc->Width - 20, 16 );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// OnFrameRender callback.  
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    // Update the camera's position based on user input 
    g_Camera.FrameMove( fElapsedTime );
}


//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the 
// rendering calls for the scene, and it will also be called if the window needs to be 
// repainted. After this function has returned, DXUT will call 
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    // If the settings dialog is being shown, then
    // render it instead of rendering the app's scene
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.OnRender( fElapsedTime );
        return;
    }

    HRESULT hr;
    D3DXMATRIXA16 mWorld;
    D3DXMATRIXA16 mView;
    D3DXMATRIXA16 mProj;
    D3DXMATRIXA16 mWorldViewProjection;
    
    // Clear the render target and the zbuffer 
    V( pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 66, 75, 121), 1.0f, 0) );

    // Render the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
        // Get the projection & view matrix from the camera class
        mWorld = g_mWorldCenter * *g_Camera.GetWorldMatrix();
        mProj = *g_Camera.GetProjMatrix();
        mView = *g_Camera.GetViewMatrix();

        mWorldViewProjection = mWorld * mView * mProj;

        if( g_ppPMeshes )
        {
            // Update the effect's variables.  Instead of using strings, it would 
            // be more efficient to cache a handle to the parameter by calling 
            // ID3DXEffect::GetParameterByName
            V( g_pEffect->SetMatrix( "g_mWorldViewProjection", &mWorldViewProjection ) );
            V( g_pEffect->SetMatrix( "g_mWorld", &mWorld ) );
            // Set and draw each of the materials in the mesh
            for( DWORD i = 0; i < g_dwNumMaterials; i++ )
            {
                V( g_pEffect->SetVector( "g_vDiffuse", (D3DXVECTOR4*)&g_mtrlMeshMaterials[i] ) );
                V( g_pEffect->SetTexture( "g_txScene", g_ppMeshTextures[i] ) );
                UINT cPasses;
                V( g_pEffect->Begin( &cPasses, 0 ) );
                for( UINT p = 0; p < cPasses; ++p )
                {
                    V( g_pEffect->BeginPass( p ) );
                    if( g_bShowOptimized )
                    {
                        V( g_ppPMeshes[g_iPMeshCur]->DrawSubset( i ) );
                    }
                    else
                    {
                        V( g_pPMeshFull->DrawSubset( i ) );
                    }
                    V( g_pEffect->EndPass() );
                }
                V( g_pEffect->End() );
            }
        }

        RenderText();
        V( g_HUD.OnRender( fElapsedTime ) );
        V( g_SampleUI.OnRender( fElapsedTime ) );

        V( pd3dDevice->EndScene() );
    }
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work however the 
    // pFont->DrawText() will not be batched together.  Batching calls will improves performance.
    const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetD3D9BackBufferSurfaceDesc();
    CDXUTTextHelper txtHelper( g_pFont, g_pTextSprite, 15 );

    // Output statistics
    txtHelper.Begin();
    txtHelper.SetInsertionPos( 5, 5 );
    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    txtHelper.DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) ); // Show FPS
    txtHelper.DrawTextLine( DXUTGetDeviceStats() );

    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
    if( g_bShowOptimized )
        txtHelper.DrawFormattedTextLine( L"Using optimized mesh %d of %d\n"
                                        L"Current mesh vertices range: %u / %u\n"
                                        L"Absolute vertices range: %u / %u\n"
                                        L"Current vertices: %d\n",
                                        g_iPMeshCur + 1, g_cPMeshes,
                                        g_ppPMeshes[g_iPMeshCur]->GetMinVertices(),
                                        g_ppPMeshes[g_iPMeshCur]->GetMaxVertices(),
                                        g_ppPMeshes[0]->GetMinVertices(),
                                        g_ppPMeshes[g_cPMeshes-1]->GetMaxVertices(),
                                        g_ppPMeshes[g_iPMeshCur]->GetNumVertices() );
    else
        txtHelper.DrawFormattedTextLine( L"Using unoptimized mesh\n"
                                        L"Mesh vertices range: %u / %u\n"
                                        L"Current vertices: %d\n",
                                        g_pPMeshFull->GetMinVertices(),
                                        g_pPMeshFull->GetMaxVertices(),
                                        g_pPMeshFull->GetNumVertices() );

    // Draw help
    if( g_bShowHelp )
    {
        txtHelper.SetInsertionPos( 10, pd3dsdBackBuffer->Height-15*7 );
        txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 0.75f, 0.0f, 1.0f ) );
        txtHelper.DrawTextLine( L"Controls (F1 to hide):" );

        txtHelper.SetInsertionPos( 40, pd3dsdBackBuffer->Height-15*6 );
        txtHelper.DrawTextLine( L"Rotate mesh: Left click drag\n"
                                L"Zoom: mouse wheel\n"
                                L"Quit: ESC" );
    }
    else
    {
        txtHelper.SetInsertionPos( 10, pd3dsdBackBuffer->Height-15*4 );
        txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 0.75f, 0.0f, 1.0f ) );
        txtHelper.DrawTextLine( L"Press F1 for help" );
    }
    txtHelper.End();
}


//--------------------------------------------------------------------------------------
// Before handling window messages, DXUT passes incoming windows 
// messages to the application through this callback function. If the application sets 
// *pbNoFurtherProcessing to TRUE, then DXUT will not process this message.
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
    // Always allow dialog resource manager calls to handle global messages
    // so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass all remaining windows messages to camera so it can respond to user input
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}


//--------------------------------------------------------------------------------------
// As a convenience, DXUT inspects the incoming windows messages for
// keystroke messages and decodes the message parameters to pass relevant keyboard
// messages to the application.  The framework does not remove the underlying keystroke 
// messages, which are still passed to the application's MsgProc callback.
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if( bKeyDown )
    {
        switch( nChar )
        {
            case VK_F1: g_bShowHelp = !g_bShowHelp; break;
        }
    }
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN: DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:        DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:     g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() ); break;
        case IDC_DETAIL:
            SetNumVertices( ((CDXUTSlider*)pControl)->GetValue() );
            break;
        case IDC_USEOPTIMIZED:
            g_bShowOptimized = ((CDXUTCheckBox*)pControl)->GetChecked();
            break;
    }
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
// in the OnResetDevice callback should be released here, which generally includes all 
// D3DPOOL_DEFAULT resources. See the "Lost Devices" section of the documentation for 
// information about lost devices.
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9LostDevice();
    g_SettingsDlg.OnD3D9LostDevice();
    if( g_pFont )
        g_pFont->OnLostDevice();
    if( g_pEffect )
        g_pEffect->OnLostDevice();
    SAFE_RELEASE(g_pTextSprite);
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// been destroyed, which generally happens as a result of application termination or 
// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
// should be released here, which generally includes all D3DPOOL_MANAGED resources. 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9DestroyDevice();
    g_SettingsDlg.OnD3D9DestroyDevice();
    SAFE_RELEASE(g_pEffect);
    SAFE_RELEASE(g_pFont);

    for( UINT i = 0; i < g_dwNumMaterials; i++ )
        SAFE_RELEASE( g_ppMeshTextures[i] );
    SAFE_DELETE_ARRAY( g_ppMeshTextures );

    SAFE_RELEASE( g_pPMeshFull );
    for( UINT i = 0; i < g_cPMeshes; i++ )
        SAFE_RELEASE( g_ppPMeshes[i] );

    g_cPMeshes = 0;
    delete[] g_ppPMeshes;

    SAFE_DELETE_ARRAY( g_mtrlMeshMaterials );
    g_dwNumMaterials = 0;
}
