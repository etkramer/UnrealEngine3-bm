//--------------------------------------------------------------------------------------
// File: EnhancedMesh.cpp
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
IDirect3DTexture9*      g_pDefaultTex = NULL;   // Default texture for texture-less material
bool                    g_bShowHelp = true;     // If true, it renders the UI control text
CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg         g_SettingsDlg;          // Device settings dialog
CDXUTDialog             g_HUD;                  // dialog for standard controls
CDXUTDialog             g_SampleUI;             // dialog for sample specific controls
ID3DXMesh*              g_pMeshSysMem = NULL;   // system memory version of mesh, lives through resize's
ID3DXMesh*              g_pMeshEnhanced = NULL; // vid mem version of mesh that is enhanced
UINT                    g_dwNumSegs = 2;        // number of segments per edge (tesselation level)
D3DXMATERIAL*           g_pMaterials = NULL;    // pointer to material info in m_pbufMaterials
LPDIRECT3DTEXTURE9*     g_ppTextures = NULL;    // array of textures, entries are NULL if no texture specified
DWORD                   g_dwNumMaterials = NULL;// number of materials
D3DXVECTOR3             g_vObjectCenter;        // Center of bounding sphere of object
FLOAT                   g_fObjectRadius;        // Radius of bounding sphere of object
D3DXMATRIXA16           g_mCenterWorld;         // World matrix to center the mesh
ID3DXBuffer*            g_pbufMaterials = NULL; // contains both the materials data and the filename strings
ID3DXBuffer*            g_pbufAdjacency = NULL; // Contains the adjacency info loaded with the mesh
bool                    g_bUseHWNPatches = true;
bool                    g_bWireframe = false;


//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           3
#define IDC_CHANGEDEVICE        4
#define IDC_FILLMODE            5
#define IDC_SEGMENTLABEL        6
#define IDC_SEGMENT             7
#define IDC_HWNPATCHES          8


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
HRESULT LoadMesh( IDirect3DDevice9* pd3dDevice, WCHAR* strFileName, ID3DXMesh** ppMesh );
void    RenderText();
HRESULT GenerateEnhancedMesh( IDirect3DDevice9 *pd3dDevice, UINT cNewNumSegs );


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
    DXUTSetHotkeyHandling( true, true, true );  // handle the default hotkeys
    DXUTCreateWindow( L"Enhanced Mesh - N-Patches" );
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

    g_SampleUI.SetCallback( OnGUIEvent ); iY = 10; 
    g_SampleUI.AddComboBox( IDC_FILLMODE, 10, iY, 150, 24, L'F' );
    g_SampleUI.GetComboBox( IDC_FILLMODE )->AddItem( L"(F)illmode: Solid", (void*)0 );
    g_SampleUI.GetComboBox( IDC_FILLMODE )->AddItem( L"(F)illmode: Wireframe", (void*)1 );
    g_SampleUI.AddStatic( IDC_SEGMENTLABEL, L"Number of segments: 2", 10, iY += 30, 150, 16 );
    g_SampleUI.AddSlider( IDC_SEGMENT, 10, iY += 14, 150, 24, 1, 10, 2 );
    g_SampleUI.AddCheckBox( IDC_HWNPATCHES, L"Use hardware N-patches", 10, iY += 26, 150, 20, true, L'H' );
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

    HRESULT hr;
    IDirect3D9* pD3D = DXUTGetD3D9Object();
    D3DCAPS9 caps;

    V( pD3D->GetDeviceCaps( pDeviceSettings->d3d9.AdapterOrdinal,
                            pDeviceSettings->d3d9.DeviceType,
                            &caps ) );

    // Turn vsync off
    pDeviceSettings->d3d9.pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    g_SettingsDlg.GetDialogControl()->GetComboBox( DXUTSETTINGSDLG_PRESENT_INTERVAL )->SetEnabled( false );

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


//--------------------------------------------------------------------------------------
// Generate a mesh that can be tesselated.
HRESULT GenerateEnhancedMesh( IDirect3DDevice9 *pd3dDevice, UINT dwNewNumSegs )
{
    LPD3DXMESH pMeshEnhancedSysMem = NULL;
    LPD3DXMESH pMeshTemp;
    HRESULT    hr;

    if( g_pMeshSysMem == NULL )
        return S_OK;

    // if using hw, just copy the mesh
    if( g_bUseHWNPatches )
    {
        hr = g_pMeshSysMem->CloneMeshFVF( D3DXMESH_WRITEONLY | D3DXMESH_NPATCHES | 
            (g_pMeshSysMem->GetOptions() & D3DXMESH_32BIT), 
            g_pMeshSysMem->GetFVF(), pd3dDevice, &pMeshTemp );
        if( FAILED(hr) )
            return hr;
    }
    else  // tesselate the mesh in sw
    {
        // Create an enhanced version of the mesh, will be in sysmem since source is
        hr = D3DXTessellateNPatches( g_pMeshSysMem, (DWORD*)g_pbufAdjacency->GetBufferPointer(), 
                                     (float)dwNewNumSegs, FALSE, &pMeshEnhancedSysMem, NULL );
        if( FAILED(hr) )
        {
            // If the tessellate failed, there might have been more triangles or vertices 
            // than can fit into a 16bit mesh, so try cloning to 32bit before tessellation

            hr = g_pMeshSysMem->CloneMeshFVF( D3DXMESH_SYSTEMMEM | D3DXMESH_32BIT, 
                g_pMeshSysMem->GetFVF(), pd3dDevice, &pMeshTemp );
            if (FAILED(hr))
                return hr;

            hr = D3DXTessellateNPatches( pMeshTemp, (DWORD*)g_pbufAdjacency->GetBufferPointer(), 
                                         (float)dwNewNumSegs, FALSE, &pMeshEnhancedSysMem, NULL );
            if( FAILED(hr) )
            {
                pMeshTemp->Release();
                return hr;
            }

            pMeshTemp->Release();
        }

        // Make a vid mem version of the mesh  
        // Only set WRITEONLY if it doesn't use 32bit indices, because those 
        // often need to be emulated, which means that D3DX needs read-access.
        DWORD dwMeshEnhancedFlags = pMeshEnhancedSysMem->GetOptions() & D3DXMESH_32BIT;
        if( (dwMeshEnhancedFlags & D3DXMESH_32BIT) == 0)
            dwMeshEnhancedFlags |= D3DXMESH_WRITEONLY;
        hr = pMeshEnhancedSysMem->CloneMeshFVF( dwMeshEnhancedFlags, g_pMeshSysMem->GetFVF(),
                                                pd3dDevice, &pMeshTemp );
        if( FAILED(hr) )
        {
            SAFE_RELEASE( pMeshEnhancedSysMem );
            return hr;
        }

        // Latch in the enhanced mesh
        SAFE_RELEASE( pMeshEnhancedSysMem );
    }

    SAFE_RELEASE( g_pMeshEnhanced );
    g_pMeshEnhanced = pMeshTemp;
    g_dwNumSegs     = dwNewNumSegs;

    return S_OK;
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
    WCHAR wszMeshDir[MAX_PATH];
    WCHAR wszWorkingDir[MAX_PATH];
    IDirect3DVertexBuffer9* pVB = NULL;

    V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );
    V_RETURN( g_SettingsDlg.OnD3D9CreateDevice( pd3dDevice ) );

    D3DCAPS9 d3dCaps;
    pd3dDevice->GetDeviceCaps( &d3dCaps );
    if( !( d3dCaps.DevCaps & D3DDEVCAPS_NPATCHES ) )
    {
        // No hardware support. Disable the checkbox.
        g_bUseHWNPatches = false;
        g_SampleUI.GetCheckBox( IDC_HWNPATCHES )->SetChecked( false );
        g_SampleUI.GetCheckBox( IDC_HWNPATCHES )->SetEnabled( false );
    } else
        g_SampleUI.GetCheckBox( IDC_HWNPATCHES )->SetEnabled( true );

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
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"EnhancedMesh.fx" ) );

    // If this fails, there should be debug output as to
    // they the .fx file failed to compile
    V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags, 
                                        NULL, &g_pEffect, NULL ) );

    // Load the mesh
    V_RETURN( DXUTFindDXSDKMediaFileCch( wszMeshDir, MAX_PATH, MESHFILENAME ) );
    V_RETURN( D3DXLoadMeshFromX( wszMeshDir, D3DXMESH_SYSTEMMEM, pd3dDevice,
                                 &g_pbufAdjacency, &g_pbufMaterials, NULL, &g_dwNumMaterials,
                                 &g_pMeshSysMem ) );

    // Initialize the mesh directory string
    WCHAR *pwszLastBSlash = wcsrchr( wszMeshDir, L'\\' );
    if( pwszLastBSlash )
        *pwszLastBSlash = L'\0';
    else
        StringCchCopyW( wszMeshDir, MAX_PATH, L"." );

    // Lock the vertex buffer, to generate a simple bounding sphere
    hr = g_pMeshSysMem->GetVertexBuffer( &pVB );
    if( FAILED(hr) )
        return hr;

    void* pVertices = NULL;
    hr = pVB->Lock( 0, 0, &pVertices, 0 );
    if( FAILED(hr) )
    {
        SAFE_RELEASE( pVB );
        return hr;
    }

    hr = D3DXComputeBoundingSphere( (D3DXVECTOR3*)pVertices, g_pMeshSysMem->GetNumVertices(),
                                    D3DXGetFVFVertexSize(g_pMeshSysMem->GetFVF()), &g_vObjectCenter,
                                    &g_fObjectRadius );
    pVB->Unlock();
    SAFE_RELEASE( pVB );

    if( FAILED(hr) )
        return hr;

    if( 0 == g_dwNumMaterials )
        return E_INVALIDARG;

    D3DXMatrixTranslation( &g_mCenterWorld, -g_vObjectCenter.x, -g_vObjectCenter.y, -g_vObjectCenter.z );

    // Change the current directory to the .x's directory so
    // that the search can find the texture files.
    GetCurrentDirectory( MAX_PATH, wszWorkingDir );
    wszWorkingDir[MAX_PATH - 1] = L'\0';
    SetCurrentDirectory( wszMeshDir );

    // Get the array of materials out of the returned buffer, allocate a
    // texture array, and load the textures
    g_pMaterials = (D3DXMATERIAL*)g_pbufMaterials->GetBufferPointer();
    g_ppTextures = new LPDIRECT3DTEXTURE9[g_dwNumMaterials];

    for( UINT i = 0; i < g_dwNumMaterials; i++ )
    {
        WCHAR strTexturePath[512] = L"";
        WCHAR *wszName;
        WCHAR wszBuf[MAX_PATH];
        wszName = wszBuf;
        MultiByteToWideChar( CP_ACP, 0, g_pMaterials[i].pTextureFilename, -1, wszBuf, MAX_PATH );
        wszBuf[MAX_PATH - 1] = L'\0';
        DXUTFindDXSDKMediaFileCch( strTexturePath, 512, wszName );
        if( FAILED( D3DXCreateTextureFromFile( pd3dDevice, strTexturePath,
                                               &g_ppTextures[i] ) ) )
            g_ppTextures[i] = NULL;
    }
    SetCurrentDirectory( wszWorkingDir );

    // Make sure there are normals, which are required for the tesselation
    // enhancement.
    if( !( g_pMeshSysMem->GetFVF() & D3DFVF_NORMAL ) )
    {
        ID3DXMesh* pTempMesh;

        V_RETURN( g_pMeshSysMem->CloneMeshFVF( g_pMeshSysMem->GetOptions(),
                                               g_pMeshSysMem->GetFVF() | D3DFVF_NORMAL,
                                               pd3dDevice, &pTempMesh ) );

        D3DXComputeNormals( pTempMesh, NULL );

        SAFE_RELEASE( g_pMeshSysMem );
        g_pMeshSysMem = pTempMesh;
    }

    // Create the 1x1 white default texture
    V_RETURN( pd3dDevice->CreateTexture( 1, 1, 1, 0, D3DFMT_A8R8G8B8,
                                         D3DPOOL_MANAGED, &g_pDefaultTex, NULL ) );
    D3DLOCKED_RECT lr;
    V_RETURN( g_pDefaultTex->LockRect( 0, &lr, NULL, 0 ) );
    *(LPDWORD)lr.pBits = D3DCOLOR_RGBA( 255, 255, 255, 255 );
    V_RETURN( g_pDefaultTex->UnlockRect( 0 ) );

    // Setup the camera's view parameters
    D3DXVECTOR3 vecEye(0.0f, 0.0f, -5.0f);
    D3DXVECTOR3 vecAt (0.0f, 0.0f, -0.0f);
    g_Camera.SetViewParams( &vecEye, &vecAt );

    return S_OK;
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

    V_RETURN( GenerateEnhancedMesh( pd3dDevice, g_dwNumSegs ) );

    if( g_bWireframe )
        pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
    else
        pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI/4, fAspectRatio, 0.1f, 1000.0f );
    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width-170, 0 );
    g_HUD.SetSize( 170, 170 );
    g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width-170, pBackBufferSurfaceDesc->Height-350 );
    g_SampleUI.SetSize( 170, 300 );

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
    IDirect3DDevice9* pd3dDevice = DXUTGetD3D9Device();

    // Update the camera's position based on user input 
    g_Camera.FrameMove( fElapsedTime );

    pd3dDevice->SetTransform( D3DTS_WORLD, g_Camera.GetWorldMatrix() );
    pd3dDevice->SetTransform( D3DTS_VIEW, g_Camera.GetViewMatrix() );
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
        mWorld = *g_Camera.GetWorldMatrix();
        mProj = *g_Camera.GetProjMatrix();
        mView = *g_Camera.GetViewMatrix();

        mWorldViewProjection = g_mCenterWorld * mWorld * mView * mProj;

        // Update the effect's variables.  Instead of using strings, it would 
        // be more efficient to cache a handle to the parameter by calling 
        // ID3DXEffect::GetParameterByName
        V( g_pEffect->SetMatrix( "g_mWorldViewProjection", &mWorldViewProjection ) );
        V( g_pEffect->SetMatrix( "g_mWorld", &mWorld ) );
        V( g_pEffect->SetFloat( "g_fTime", (float)fTime ) );

        if( g_bUseHWNPatches )
        {
            float fNumSegs;

            fNumSegs = (float)g_dwNumSegs;
            pd3dDevice->SetNPatchMode( fNumSegs );
        }

        UINT cPasses;
        V( g_pEffect->Begin( &cPasses, 0 ) );
        for( UINT p = 0; p < cPasses; ++p )
        {
            V( g_pEffect->BeginPass( p ) );

            // set and draw each of the materials in the mesh
            for( UINT i = 0; i < g_dwNumMaterials; i++ )
            {
                V( g_pEffect->SetVector( "g_vDiffuse", (D3DXVECTOR4*)&g_pMaterials[i].MatD3D.Diffuse ) );
                if( g_ppTextures[i] )
                {
                    V( g_pEffect->SetTexture( "g_txScene", g_ppTextures[i] ) );
                }
                else
                {
                    V( g_pEffect->SetTexture( "g_txScene", g_pDefaultTex ) );
                }
                V( g_pEffect->CommitChanges() );

                g_pMeshEnhanced->DrawSubset( i );
            }

            V( g_pEffect->EndPass() );
        }
        V( g_pEffect->End() );

        if( g_bUseHWNPatches )
        {
            pd3dDevice->SetNPatchMode( 0 );
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
    CDXUTTextHelper txtHelper( g_pFont, g_pTextSprite, 15 );

    // Output statistics
    txtHelper.Begin();
    txtHelper.SetInsertionPos( 5, 5 );
    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    txtHelper.DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) ); 
    txtHelper.DrawTextLine( DXUTGetDeviceStats() );
    
    // Draw help
    if( g_bShowHelp )
    {
        const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetD3D9BackBufferSurfaceDesc();
        txtHelper.SetInsertionPos( 10, pd3dsdBackBuffer->Height-15*6 );
        txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 0.75f, 0.0f, 1.0f ) );
        txtHelper.DrawTextLine( L"Controls (F1 to hide):" );

        txtHelper.SetInsertionPos( 40, pd3dsdBackBuffer->Height-15*5 );
        txtHelper.DrawTextLine( L"Rotate mesh: Left click drag\n"
                                L"Rotate camera: right click drag\n"
                                L"Zoom: Mouse wheel\n"
                                L"Quit: ESC" );
    }
    else
    {
        txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
        txtHelper.DrawTextLine( L"Press F1 for help" );
    }

    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 0.75f, 0.0f, 1.0f ) );
    txtHelper.SetInsertionPos( 10, 65 );
    txtHelper.DrawFormattedTextLine( L"NumSegs: %d\n", g_dwNumSegs );
    txtHelper.DrawFormattedTextLine( L"NumFaces: %d\n", ( g_pMeshEnhanced == NULL) ? 0 : g_pMeshEnhanced->GetNumFaces() );
    txtHelper.DrawFormattedTextLine( L"NumVertices: %d\n", ( g_pMeshEnhanced == NULL) ? 0 : g_pMeshEnhanced->GetNumVertices() );

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
        case IDC_FILLMODE:
        {
            g_bWireframe = ((CDXUTComboBox*)pControl)->GetSelectedData() != 0;
            IDirect3DDevice9 *pd3dDevice = DXUTGetD3D9Device();
            pd3dDevice->SetRenderState( D3DRS_FILLMODE, g_bWireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID );
            break;
        }
        case IDC_SEGMENT:
            g_dwNumSegs = ((CDXUTSlider*)pControl)->GetValue();
            WCHAR wszBuf[256];
            StringCchPrintf( wszBuf, 256, L"Number of segments: %u", g_dwNumSegs );
            g_SampleUI.GetStatic( IDC_SEGMENTLABEL )->SetText( wszBuf );
            GenerateEnhancedMesh( DXUTGetD3D9Device(), g_dwNumSegs );
            break;
        case IDC_HWNPATCHES:
            g_bUseHWNPatches = ((CDXUTCheckBox*)pControl)->GetChecked();
            GenerateEnhancedMesh( DXUTGetD3D9Device(), g_dwNumSegs );
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
    SAFE_RELEASE( g_pTextSprite );
    SAFE_RELEASE( g_pMeshEnhanced );
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
        SAFE_RELEASE( g_ppTextures[i] );

    SAFE_RELEASE( g_pDefaultTex );
    SAFE_DELETE_ARRAY( g_ppTextures );
    SAFE_RELEASE( g_pMeshSysMem );
    SAFE_RELEASE( g_pbufMaterials );
    SAFE_RELEASE( g_pbufAdjacency );
    g_dwNumMaterials = 0L;
}



