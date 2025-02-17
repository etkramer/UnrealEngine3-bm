//--------------------------------------------------------------------------------------
// File: Blobs.cpp
//
// A pixel shader effect to mimic metaball physics in image space.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsdlg.h"
#include "SDKmisc.h"
#include "resource.h"

//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 


//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
#define NUM_BLOBS           5
#define FIELD_OF_VIEW       ( (70.0f/90.0f)*(D3DX_PI/2.0f) )

#define GAUSSIAN_TEXSIZE    64
#define GAUSSIAN_HEIGHT     1
#define GAUSSIAN_DEVIATION  0.125f


//--------------------------------------------------------------------------------------
// Custom types
//--------------------------------------------------------------------------------------

// Vertex format for blob billboards
struct POINTVERTEX  
{
    D3DXVECTOR3 pos;
    float       size;
    D3DXVECTOR3 color;

    static const DWORD FVF;
};
const DWORD POINTVERTEX::FVF = D3DFVF_XYZ | D3DFVF_PSIZE | D3DFVF_DIFFUSE; 

// Vertex format for screen space work
struct SCREENVERTEX     
{
    D3DXVECTOR4 pos;
    D3DXVECTOR2 tCurr;
    D3DXVECTOR2 tBack;
    FLOAT       fSize;
    D3DXVECTOR3 vColor;
    static const DWORD FVF;
};
const DWORD SCREENVERTEX::FVF = D3DFVF_XYZRHW | D3DFVF_TEX4
                                | D3DFVF_TEXCOORDSIZE2(0)
                                | D3DFVF_TEXCOORDSIZE2(1)
                                | D3DFVF_TEXCOORDSIZE1(2)
                                | D3DFVF_TEXCOORDSIZE3(3);


struct CRenderTargetSet
{
    IDirect3DSurface9* apCopyRT[2];
    IDirect3DSurface9* apBlendRT[2];
};


//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
ID3DXFont*              g_pFont = NULL;           // Font for drawing text
ID3DXSprite*            g_pTextSprite = NULL;     // Sprite for batching draw text calls
ID3DXEffect*            g_pEffect = NULL;         // D3DX effect interface
CModelViewerCamera      g_Camera;                 // A model viewing camera
bool                    g_bShowHelp = true;       // If true, it renders the UI control text
CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg         g_SettingsDlg;            // Device settings dialog
CDXUTDialog             g_HUD;                    // dialog for standard controls
LPDIRECT3DVERTEXBUFFER9 g_pBlobVB = NULL;         // Vertex buffer for blob billboards
POINTVERTEX             g_BlobPoints[NUM_BLOBS];  // Position, size, and color states
LPDIRECT3DTEXTURE9      g_pTexGBuffer[4] = {0};   // Buffer textures for blending effect   
LPDIRECT3DTEXTURE9      g_pTexScratch = NULL;     // Scratch texture
LPDIRECT3DTEXTURE9      g_pTexBlob = NULL;        // Blob texture
D3DFORMAT               g_BlobTexFormat;          // Texture format for blob texture
LPDIRECT3DCUBETEXTURE9  g_pEnvMap = NULL;         // Environment map   
int                     g_nPasses = 0;            // Number of rendering passes required
int                     g_nRtUsed = 0;            // Number of render targets used for blending
CRenderTargetSet        g_aRTSet[2];              // Render targets for each pass
D3DXHANDLE              g_hBlendTech = NULL;      // Technique to use for blending


//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           3
#define IDC_CHANGEDEVICE        4


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

HRESULT GenerateGaussianTexture();
HRESULT FillBlobVB( D3DXMATRIXA16* pmatWorldView );
HRESULT RenderFullScreenQuad( float fDepth );
void    InitApp();
void    RenderText();


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

    // DXUT will create and use the best device (either D3D9 or D3D10) 
    // that is available on the system depending on which D3D callbacks are set below

    // Set DXUT callbacks
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

    DXUTSetCursorSettings( true, true );
    InitApp();
    DXUTInit( true, true ); // Parse the command line and show msgboxes
    DXUTSetHotkeyHandling( true, true, true );
    DXUTCreateWindow( L"Blobs" );
    DXUTCreateDevice( true, 640, 480 );
    DXUTMainLoop();

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

    g_HUD.SetCallback( OnGUIEvent ); int iY = 10; 
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, iY, 125, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 35, iY += 24, 125, 22 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, iY += 24, 125, 22, VK_F2 );

    // Set initial blob states
    for( int i=0; i < NUM_BLOBS; i++ )
    {
        g_BlobPoints[i].pos = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
        g_BlobPoints[i].size = 1.0f;
    }

    g_BlobPoints[0].color = D3DXVECTOR3(0.3f, 0.0f, 0.0f);
    g_BlobPoints[1].color = D3DXVECTOR3(0.0f, 0.3f, 0.0f);
    g_BlobPoints[2].color = D3DXVECTOR3(0.0f, 0.0f, 0.3f);
    g_BlobPoints[3].color = D3DXVECTOR3(0.3f, 0.3f, 0.0f);
    g_BlobPoints[4].color = D3DXVECTOR3(0.0f, 0.3f, 0.3f);
}


//--------------------------------------------------------------------------------------
// Rejects any D3D9 devices that aren't acceptable to the app by returning false
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

    // No fallback, so need ps2.0
    if( pCaps->PixelShaderVersion < D3DPS_VERSION(2,0) )
        return false;

    // No fallback, so need to support render target
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                AdapterFormat, D3DUSAGE_RENDERTARGET | D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
                D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
    {
        return false;
    }
    
    // Check support for pixel formats that are going to be used
    // D3DFMT_A16B16G16R16F render target
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_RENDERTARGET, 
                    D3DRTYPE_TEXTURE, D3DFMT_A16B16G16R16F ) ) )
    {
        return false;
    }

    // D3DFMT_R32F render target
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_RENDERTARGET, 
                    D3DRTYPE_TEXTURE, D3DFMT_R32F) ) )
    {
        return false;
    }

    return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D10 device, allowing the app to modify the device settings as needed
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
// Create any D3D9 resources that will live through a device reset (D3DPOOL_MANAGED)
// and aren't tied to the back buffer size
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );
    V_RETURN( g_SettingsDlg.OnD3D9CreateDevice( pd3dDevice ) );

    // Query multiple RT setting and set the num of passes required
    D3DCAPS9 Caps;
    pd3dDevice->GetDeviceCaps( &Caps );
    if( Caps.NumSimultaneousRTs < 2 )
    {
        g_nPasses = 2;
        g_nRtUsed = 1;
    }
    else
    {
        g_nPasses = 1;
        g_nRtUsed = 2;
    }

    // Determine which of D3DFMT_R16F or D3DFMT_R32F to use for blob texture
    IDirect3D9* pD3D;
    pd3dDevice->GetDirect3D( &pD3D );
    D3DDISPLAYMODE DisplayMode;
    pd3dDevice->GetDisplayMode( 0, &DisplayMode );

    if( FAILED( pD3D->CheckDeviceFormat( Caps.AdapterOrdinal, Caps.DeviceType,
                    DisplayMode.Format, D3DUSAGE_RENDERTARGET, 
                    D3DRTYPE_TEXTURE, D3DFMT_R16F) ) )
        g_BlobTexFormat = D3DFMT_R32F;
    else
        g_BlobTexFormat = D3DFMT_R16F;

    SAFE_RELEASE( pD3D );

    // Initialize the font
    V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                         OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                         L"Arial", &g_pFont ) );

    DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;

    #if defined( DEBUG ) || defined( _DEBUG )
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
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"Blobs.fx" ) );
    V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags, 
                                        NULL, &g_pEffect, NULL ) );

    // Initialize the technique for blending
    if( 1 == g_nPasses )
    {
        // Multiple RT available
        g_hBlendTech = g_pEffect->GetTechniqueByName( "BlobBlend" );
    } 
    else
    {
        // Single RT. Multiple passes.
        g_hBlendTech = g_pEffect->GetTechniqueByName( "BlobBlendTwoPasses" );
    }

    // Setup the camera's view parameters
    D3DXVECTOR3 vecEye(0.0f, 0.0f, -5.0f);
    D3DXVECTOR3 vecAt (0.0f, 0.0f, -0.0f);
    g_Camera.SetViewParams( &vecEye, &vecAt );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that won't live through a device reset (D3DPOOL_DEFAULT) 
// or that are tied to the back buffer size 
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

    // Create the Gaussian distribution texture
    V_RETURN( GenerateGaussianTexture() );
  
    // Create the blob vertex buffer
    V_RETURN( pd3dDevice->CreateVertexBuffer( NUM_BLOBS * 6 * sizeof(SCREENVERTEX), 
                                                D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
                                                SCREENVERTEX::FVF, D3DPOOL_DEFAULT,
                                                &g_pBlobVB, NULL ) );
    
    // Create the blank texture
    V_RETURN( pd3dDevice->CreateTexture( 1, 1, 1,
                                           D3DUSAGE_RENDERTARGET,
                                           D3DFMT_A16B16G16R16F,
                                           D3DPOOL_DEFAULT,
                                           &g_pTexScratch, NULL ) );


    // Create buffer textures 
    for( int i=0; i < 4; ++i )
    {
        V_RETURN( pd3dDevice->CreateTexture( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1,
                                               D3DUSAGE_RENDERTARGET,
                                               D3DFMT_A16B16G16R16F,
                                               D3DPOOL_DEFAULT, &g_pTexGBuffer[i], NULL ) );
    }

    // Initialize the render targets
    if( 1 == g_nPasses )
    {
        // Multiple RT
        IDirect3DSurface9* pSurf;
        g_pTexGBuffer[2]->GetSurfaceLevel( 0, &pSurf );
        g_aRTSet[0].apCopyRT[0] = pSurf;
        g_pTexGBuffer[3]->GetSurfaceLevel( 0, &pSurf );
        g_aRTSet[0].apCopyRT[1] = pSurf;
        g_pTexGBuffer[0]->GetSurfaceLevel( 0, &pSurf );
        g_aRTSet[0].apBlendRT[0] = pSurf;
        g_pTexGBuffer[1]->GetSurfaceLevel( 0, &pSurf );
        g_aRTSet[0].apBlendRT[1] = pSurf;

        // 2nd pass is not needed. Therefore all RTs are NULL for this pass.
        g_aRTSet[1].apCopyRT[0] = NULL;
        g_aRTSet[1].apCopyRT[1] = NULL;
        g_aRTSet[1].apBlendRT[0] = NULL;
        g_aRTSet[1].apBlendRT[1] = NULL;
    } 
    else
    {
        // Single RT, multiple passes
        IDirect3DSurface9* pSurf;
        g_pTexGBuffer[2]->GetSurfaceLevel( 0, &pSurf );
        g_aRTSet[0].apCopyRT[0] = pSurf;
        g_pTexGBuffer[3]->GetSurfaceLevel( 0, &pSurf );
        g_aRTSet[1].apCopyRT[0] = pSurf;
        g_pTexGBuffer[0]->GetSurfaceLevel( 0, &pSurf );
        g_aRTSet[0].apBlendRT[0] = pSurf;
        g_pTexGBuffer[1]->GetSurfaceLevel( 0, &pSurf );
        g_aRTSet[1].apBlendRT[0] = pSurf;

        // RT 1 is not available. Therefore all RTs are NULL for this index.
        g_aRTSet[0].apCopyRT[1] = NULL;
        g_aRTSet[1].apCopyRT[1] = NULL;
        g_aRTSet[0].apBlendRT[1] = NULL;
        g_aRTSet[1].apBlendRT[1] = NULL;
    }

    // Set the camera parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
    D3DXVECTOR3 vEyePt    = D3DXVECTOR3( 0.0f, -2.0f, -5.0f );
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    g_Camera.SetViewParams( &vEyePt, &vLookatPt );
    g_Camera.SetProjParams( FIELD_OF_VIEW, fAspectRatio, 1.0f, 100.0f );
    g_Camera.SetRadius( 5.0f, 2.0f, 20.0f  );
    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
    g_Camera.SetInvertPitch( true );

    // Position the on-screen dialog
    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width-170, 0 );
    g_HUD.SetSize( 170, 170 );
    
    return S_OK;
}


//-----------------------------------------------------------------------------
// Generate a texture to store gaussian distribution results
//-----------------------------------------------------------------------------
HRESULT GenerateGaussianTexture()
{
    HRESULT hr = S_OK;
    LPDIRECT3DSURFACE9 pBlobTemp = NULL;
    LPDIRECT3DSURFACE9 pBlobNew = NULL;
    
    IDirect3DDevice9* pd3dDevice = DXUTGetD3D9Device();
    
    // Create a temporary texture
    LPDIRECT3DTEXTURE9 texTemp;
    V_RETURN( pd3dDevice->CreateTexture( GAUSSIAN_TEXSIZE, GAUSSIAN_TEXSIZE, 1,
                                         D3DUSAGE_DYNAMIC, D3DFMT_R32F,
                                         D3DPOOL_DEFAULT, &texTemp, NULL ) );
    // Create the gaussian texture
    V_RETURN( pd3dDevice->CreateTexture( GAUSSIAN_TEXSIZE, GAUSSIAN_TEXSIZE, 1,
                                         D3DUSAGE_DYNAMIC, g_BlobTexFormat,
                                         D3DPOOL_DEFAULT, &g_pTexBlob, NULL ) );
   
    // Create the environment map
    TCHAR str[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"lobby\\lobbycube.dds" ) );
    V_RETURN( D3DXCreateCubeTextureFromFile( pd3dDevice, str, &g_pEnvMap ) );

    // Fill in the gaussian texture data
    D3DLOCKED_RECT Rect;
    V_RETURN( texTemp->LockRect(0, &Rect, 0, 0) );
    
    int u, v;
    float dx, dy, I;
    float* pBits;  
    
    for( v=0; v < GAUSSIAN_TEXSIZE; ++v )
    {
        pBits = (float*)((char*)(Rect.pBits)+v*Rect.Pitch);
        
        for( u=0; u < GAUSSIAN_TEXSIZE; ++u )
        {
            dx = 2.0f*u/(float)GAUSSIAN_TEXSIZE-1.0f;
            dy = 2.0f*v/(float)GAUSSIAN_TEXSIZE-1.0f;
            I = GAUSSIAN_HEIGHT * (float)exp(-(dx*dx+dy*dy)/GAUSSIAN_DEVIATION);

            *(pBits++) = I;  // intensity
        }
    }

    texTemp->UnlockRect(0);

    // Copy the temporary surface to the stored gaussian texture
    V_RETURN( texTemp->GetSurfaceLevel( 0, &pBlobTemp ) );
    V_RETURN( g_pTexBlob->GetSurfaceLevel( 0, &pBlobNew ) );
    V_RETURN( D3DXLoadSurfaceFromSurface( pBlobNew, 0, 0, pBlobTemp, 0, 0, D3DX_FILTER_NONE, 0 ) );
    
    SAFE_RELEASE(pBlobTemp);
    SAFE_RELEASE(pBlobNew);
    SAFE_RELEASE(texTemp);

    return S_OK;
}


//-----------------------------------------------------------------------------
// Fill the vertex buffer for the blob objects
//-----------------------------------------------------------------------------
HRESULT FillBlobVB( D3DXMATRIXA16* pmatWorldView )
{
    HRESULT hr = S_OK;
    UINT i=0; // Loop variable
    
    SCREENVERTEX* pBlobVertex;
    V_RETURN( g_pBlobVB->Lock( 0, 0, (void**)&pBlobVertex, D3DLOCK_DISCARD ) );
    
    POINTVERTEX blobPos[ NUM_BLOBS ];
    
    for( i=0; i < NUM_BLOBS; ++i )
    {
        // Transform point to camera space
        D3DXVECTOR4 blobPosCamera;
        D3DXVec3Transform( &blobPosCamera, &g_BlobPoints[i].pos, pmatWorldView );
        
        blobPos[i] = g_BlobPoints[i];
        blobPos[i].pos.x = blobPosCamera.x;
        blobPos[i].pos.y = blobPosCamera.y;
        blobPos[i].pos.z = blobPosCamera.z;
    }

    int posCount=0;
    for( i=0; i < NUM_BLOBS; ++i )
    {
        D3DXVECTOR4 blobScreenPos;

        // For calculating billboarding
        D3DXVECTOR4 billOfs(blobPos[i].size,blobPos[i].size,blobPos[i].pos.z,1);
        D3DXVECTOR4 billOfsScreen;

        // Transform to screenspace
        const D3DXMATRIX* pmatProjection = g_Camera.GetProjMatrix();
        D3DXVec3Transform(&blobScreenPos, &blobPos[i].pos, pmatProjection);
        D3DXVec4Transform(&billOfsScreen, &billOfs, pmatProjection);

        // Project
        D3DXVec4Scale(&blobScreenPos,&blobScreenPos,1.0f/blobScreenPos.w);
        D3DXVec4Scale(&billOfsScreen,&billOfsScreen,1.0f/billOfsScreen.w);

        D3DXVECTOR2 vTexCoords[] = 
        {
            D3DXVECTOR2(0.0f,0.0f),
            D3DXVECTOR2(1.0f,0.0f),
            D3DXVECTOR2(0.0f,1.0f),
            D3DXVECTOR2(0.0f,1.0f),
            D3DXVECTOR2(1.0f,0.0f),
            D3DXVECTOR2(1.0f,1.0f),
        };

        D3DXVECTOR4 vPosOffset[] =
        {
            D3DXVECTOR4(-billOfsScreen.x,-billOfsScreen.y,0.0f,0.0f),
            D3DXVECTOR4( billOfsScreen.x,-billOfsScreen.y,0.0f,0.0f),
            D3DXVECTOR4(-billOfsScreen.x, billOfsScreen.y,0.0f,0.0f),
            D3DXVECTOR4(-billOfsScreen.x, billOfsScreen.y,0.0f,0.0f),
            D3DXVECTOR4( billOfsScreen.x,-billOfsScreen.y,0.0f,0.0f),
            D3DXVECTOR4( billOfsScreen.x, billOfsScreen.y,0.0f,0.0f),
        };
        
        const D3DSURFACE_DESC* pBackBufferSurfaceDesc = DXUTGetD3D9BackBufferSurfaceDesc();

        // Set constants across quad
        for( int j=0; j < 6 ;++j )
        {
            // Scale to pixels
            D3DXVec4Add( &pBlobVertex[posCount].pos, &blobScreenPos, &vPosOffset[j] );  
            
            pBlobVertex[posCount].pos.x *= pBackBufferSurfaceDesc->Width;             
            pBlobVertex[posCount].pos.y *= pBackBufferSurfaceDesc->Height;
            pBlobVertex[posCount].pos.x += 0.5f * pBackBufferSurfaceDesc->Width; 
            pBlobVertex[posCount].pos.y += 0.5f * pBackBufferSurfaceDesc->Height;
            
            pBlobVertex[posCount].tCurr = vTexCoords[j];
            pBlobVertex[posCount].tBack = D3DXVECTOR2((0.5f+pBlobVertex[posCount].pos.x)*(1.0f/pBackBufferSurfaceDesc->Width),
                                                   (0.5f+pBlobVertex[posCount].pos.y)*(1.0f/pBackBufferSurfaceDesc->Height));
            pBlobVertex[posCount].fSize = blobPos[i].size;
            pBlobVertex[posCount].vColor = blobPos[i].color;

            posCount++;
        }
    }
    g_pBlobVB->Unlock();

    return hr;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    HRESULT hr;

    static bool bPaused = false;

    // Update the camera's position based on user input 
    g_Camera.FrameMove( fElapsedTime );

    // Pause animatation if the user is rotating around
    if( !IsIconic( DXUTGetHWND() ) )
    {
        if( g_Camera.IsBeingDragged() && !DXUTIsTimePaused() )
            DXUTPause( true, false );
        if( !g_Camera.IsBeingDragged() && DXUTIsTimePaused() )
            DXUTPause( false, false );
    }
    
    // Get the projection & view matrix from the camera class
    D3DXMATRIXA16 mWorldView;
    D3DXMATRIXA16 mWorldViewProjection;
    
    D3DXMatrixMultiply( &mWorldView, g_Camera.GetWorldMatrix(), g_Camera.GetViewMatrix() );
    D3DXMatrixMultiply( &mWorldViewProjection, &mWorldView, g_Camera.GetProjMatrix() );

    // Update the effect's variables.  Instead of using strings, it would 
    // be more efficient to cache a handle to the parameter by calling 
    // ID3DXEffect::GetParameterByName
    V( g_pEffect->SetMatrix( "g_mWorldViewProjection", &mWorldViewProjection ) );
        
    // Animate the blobs
    float pos = (float) ( 1.0f + cos( 2 * D3DX_PI * fTime/3.0f ) );
    g_BlobPoints[1].pos.x = pos;
    g_BlobPoints[2].pos.x = -pos;
    g_BlobPoints[3].pos.y = pos;
    g_BlobPoints[4].pos.y = -pos;
    
    FillBlobVB( &mWorldView );
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D9 device
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.OnRender( fElapsedTime );
        return;
    }

    HRESULT hr;
    
    LPDIRECT3DSURFACE9 apSurfOldRenderTarget[2] = { NULL, NULL };
    LPDIRECT3DSURFACE9 pSurfOldDepthStencil = NULL;
    LPDIRECT3DSURFACE9 pGBufSurf[4];

    // Clear the render target and the zbuffer 
    V( pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 45, 50, 170), 1.0f, 0) );

    // Render the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
        // Get the initial device surfaces
        V( pd3dDevice->GetRenderTarget( 0, &apSurfOldRenderTarget[0] ) );  // Only RT 0 should've been set.
        V( pd3dDevice->GetDepthStencilSurface( &pSurfOldDepthStencil ) );

        // Turn off Z
        V( pd3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE ) );
        V( pd3dDevice->SetDepthStencilSurface( NULL ) );

        // Clear the blank texture
        LPDIRECT3DSURFACE9 pSurfBlank;
        V( g_pTexScratch->GetSurfaceLevel( 0, &pSurfBlank ) );
        V( pd3dDevice->SetRenderTarget( 0, pSurfBlank ) );
        V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET , D3DCOLOR_ARGB(0,0,0,0), 1.0f, 0 ) );
        SAFE_RELEASE( pSurfBlank );

        // Clear temp textures
        int i;
        for( i=0; i < 2; ++i )
        {
            V( g_pTexGBuffer[i]->GetSurfaceLevel( 0, &pGBufSurf[i] ) );
            V( pd3dDevice->SetRenderTarget( 0, pGBufSurf[i] ) );
            V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET , D3DCOLOR_ARGB(0,0,0,0), 1.0f, 0 ) );
        }
        
        for( i = 2; i < 4; ++i )
            V( g_pTexGBuffer[i]->GetSurfaceLevel( 0, &pGBufSurf[i] ) );

        V( pd3dDevice->SetStreamSource( 0, g_pBlobVB, 0, sizeof(SCREENVERTEX) ) );
        V( pd3dDevice->SetFVF( SCREENVERTEX::FVF ) );

        // Render the blobs
        UINT iPass, nNumPasses;

        V( g_pEffect->SetTechnique( g_hBlendTech ) );
        for( i=0; i < NUM_BLOBS; ++i )
        {
            // Copy bits off of render target into scratch surface [for blending].
            V( g_pEffect->SetTexture( "g_tSourceBlob", g_pTexScratch ) );
            V( g_pEffect->SetTexture( "g_tNormalBuffer", g_pTexGBuffer[0] ) );
            V( g_pEffect->SetTexture( "g_tColorBuffer", g_pTexGBuffer[1] ) );

            if( SUCCEEDED( g_pEffect->Begin( &nNumPasses, 0 ) ) )
            {
                for( iPass=0; iPass < nNumPasses; iPass++ )
                {
                    for( int rt = 0; rt < g_nRtUsed; ++rt )
                        V( pd3dDevice->SetRenderTarget( rt, g_aRTSet[iPass].apCopyRT[rt] ) );

                    V( g_pEffect->BeginPass( iPass ) );

                    V( pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, i*6, 2 ) );
                    V( g_pEffect->EndPass() );
                }

                V( g_pEffect->End() );
            }

            // Render the blob
            V( g_pEffect->SetTexture( "g_tSourceBlob", g_pTexBlob ) );
            V( g_pEffect->SetTexture( "g_tNormalBuffer", g_pTexGBuffer[2] ) );
            V( g_pEffect->SetTexture( "g_tColorBuffer", g_pTexGBuffer[3] ) );

            if( SUCCEEDED( g_pEffect->Begin( &nNumPasses, 0 ) ) )
            {
                for( iPass=0; iPass < nNumPasses; iPass++ )
                {
                    for( int rt = 0; rt < g_nRtUsed; ++rt )
                        V( pd3dDevice->SetRenderTarget( rt, g_aRTSet[iPass].apBlendRT[rt] ) );

                    V( g_pEffect->BeginPass( iPass ) );
                    V( pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, i*6, 2 ) );
                    V( g_pEffect->EndPass() );
                }

                V( g_pEffect->End() );
            }
        }

        // Restore initial device surfaces
        V( pd3dDevice->SetDepthStencilSurface( pSurfOldDepthStencil ) );

        for( int rt = 0; rt < g_nRtUsed; ++rt )
            V( pd3dDevice->SetRenderTarget( rt, apSurfOldRenderTarget[rt] ) );

        // Light and composite blobs into backbuffer
        V( g_pEffect->SetTechnique( "BlobLight" ) );
        
        V( g_pEffect->Begin( &nNumPasses, 0 ) );

        for( iPass=0; iPass < nNumPasses; iPass++ )
        {
            V( g_pEffect->BeginPass( iPass ) );
            for( i=0; i < NUM_BLOBS; ++i )
            {
                V( g_pEffect->SetTexture( "g_tSourceBlob", g_pTexGBuffer[0] ) );
                V( g_pEffect->SetTexture( "g_tColorBuffer", g_pTexGBuffer[1] ) );
                V( g_pEffect->SetTexture( "g_tEnvMap", g_pEnvMap ) );
                V( g_pEffect->CommitChanges() );

                V( RenderFullScreenQuad( 1.0f ) );
            }
            V( g_pEffect->EndPass() );
        }

        V( g_pEffect->End() );

        RenderText();
        V( g_HUD.OnRender( fElapsedTime ) );
        
        V( pd3dDevice->EndScene() );
    }

    for( int rt = 0; rt < g_nRtUsed; ++rt )
        SAFE_RELEASE( apSurfOldRenderTarget[rt] );
    SAFE_RELEASE( pSurfOldDepthStencil );
  
    for( int i=0; i < 4; ++i )
    {
        SAFE_RELEASE( pGBufSurf[i] );
    }
}


//-----------------------------------------------------------------------------
// Render a quad at the specified tranformed depth 
//-----------------------------------------------------------------------------
HRESULT RenderFullScreenQuad( float fDepth )
{
    SCREENVERTEX aVertices[4];

    IDirect3DDevice9* pd3dDevice = DXUTGetD3D9Device();
    const D3DSURFACE_DESC* pBackBufferSurfaceDesc = DXUTGetD3D9BackBufferSurfaceDesc();

    aVertices[0].pos = D3DXVECTOR4( -0.5f, -0.5f, fDepth, fDepth );
    aVertices[1].pos = D3DXVECTOR4( pBackBufferSurfaceDesc->Width - 0.5f, -0.5f, fDepth, fDepth );
    aVertices[2].pos = D3DXVECTOR4( -0.5f, pBackBufferSurfaceDesc->Height - 0.5f, fDepth, fDepth );
    aVertices[3].pos = D3DXVECTOR4( pBackBufferSurfaceDesc->Width - 0.5f, pBackBufferSurfaceDesc->Height - 0.5f, fDepth, fDepth );
    
    aVertices[0].tCurr = D3DXVECTOR2( 0.0f, 0.0f );
    aVertices[1].tCurr = D3DXVECTOR2( 1.0f, 0.0f );
    aVertices[2].tCurr = D3DXVECTOR2( 0.0f, 1.0f );
    aVertices[3].tCurr = D3DXVECTOR2( 1.0f, 1.0f );

    for (int i=0;i<4;++i)
    {
        aVertices[i].tBack = aVertices[i].tCurr;
        aVertices[i].tBack.x += (1.0f/pBackBufferSurfaceDesc->Width);
        aVertices[i].tBack.y += (1.0f/pBackBufferSurfaceDesc->Height);
        aVertices[i].fSize = 0.0f;
    }

    pd3dDevice->SetFVF(SCREENVERTEX::FVF);
    pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,aVertices,sizeof(SCREENVERTEX));
    
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( g_pSprite, strMsg, -1, &rc, DT_NOCLIP, g_clr );
    // If NULL is passed in as the sprite object, then it will work however the 
    // pFont->DrawText() will not be batched together.  Batching calls will improves performance.
    CDXUTTextHelper txtHelper( g_pFont, g_pTextSprite, 15 );

    // Output statistics
    txtHelper.Begin();
    txtHelper.SetInsertionPos( 5, 5 );
    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    txtHelper.DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
    txtHelper.DrawTextLine( DXUTGetDeviceStats() );

    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
    
    // Draw help
    if( g_bShowHelp )
    {
        const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetD3D9BackBufferSurfaceDesc();
        txtHelper.SetInsertionPos( 10, pd3dsdBackBuffer->Height-15*6 );
        txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 0.75f, 0.0f, 1.0f ) );
        txtHelper.DrawTextLine( L"Controls (F1 to hide):" );

        txtHelper.SetInsertionPos( 40, pd3dsdBackBuffer->Height-15*5 );
        txtHelper.DrawTextLine( L"Rotate model: Left mouse button\n"
                                L"Rotate camera: Right mouse button\n"
                                L"Zoom camera: Mouse wheel scroll\n" );
    }
    else
    {
        txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
        txtHelper.DrawTextLine( L"Press F1 for help" );
    }
    txtHelper.End();
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
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

    // Pass all remaining windows messages to camera so it can respond to user input
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if( bKeyDown )
    {
        switch( nChar )
        {
            case VK_F1: g_bShowHelp = !g_bShowHelp; break;
            case VK_F2: if( DXUTIsTimePaused() ) DXUTPause(false, false);
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
    }
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9ResetDevice callback 
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
    SAFE_RELEASE( g_pBlobVB );
    SAFE_RELEASE( g_pTexScratch );
    SAFE_RELEASE( g_pTexBlob );
    SAFE_RELEASE( g_pEnvMap );

    for( int p = 0; p < 2; ++p )
        for( int i = 0; i < 2; ++i )
        {
            SAFE_RELEASE( g_aRTSet[p].apCopyRT[i] );
            SAFE_RELEASE( g_aRTSet[p].apBlendRT[i] );
        }

    for( UINT i=0; i < 4; i++ )
    {
        SAFE_RELEASE( g_pTexGBuffer[i] );
    }
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9CreateDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9DestroyDevice();
    g_SettingsDlg.OnD3D9DestroyDevice();
    SAFE_RELEASE(g_pEffect);
    SAFE_RELEASE(g_pFont);
}



