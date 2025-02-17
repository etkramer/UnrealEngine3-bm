//--------------------------------------------------------------------------------------
// File: DisplacementMapping10.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "dxut.h"
#include "DXUTCamera.h"
#include "DXUTGui.h"
#include "DXUTSettingsDlg.h"
#include "SDKmisc.h"
#include "SDKMesh.h"
#include "resource.h"

#define SHADOW_SIZE 512

struct QUAD_VERTEX
{
    D3DXVECTOR3 pos;
    D3DXVECTOR2 tex;
};

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
CModelViewerCamera      g_Camera;               // A model viewing camera
CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg         g_D3DSettingsDlg;       // Device settings dialog
CDXUTDialog             g_HUD;                  // manages the 3D UI
CDXUTDialog             g_SampleUI;             // dialog for sample specific controls

ID3DX10Font*            g_pFont10 = NULL;       
ID3DX10Sprite*          g_pSprite10 = NULL;
CDXUTTextHelper*        g_pTxtHelper = NULL;
ID3D10Effect*           g_pEffect10 = NULL;
ID3D10InputLayout*      g_pNormalVertexLayout = NULL;
ID3D10InputLayout*      g_pDispVertexLayout = NULL;
ID3D10InputLayout*      g_pSceneVertexLayout = NULL;
ID3D10InputLayout*      g_pQuadVertexLayout = NULL;

CDXUTSDKMesh            g_LizardMesh;
CDXUTSDKMesh            g_SceneMesh;
D3DXMATRIX              g_MeshWorld;

ID3D10Buffer*             g_pQuadVB = NULL;
ID3D10Texture2D*          g_pShadowTexture1 = NULL;
ID3D10ShaderResourceView* g_pShadowSRV1 = NULL;
ID3D10RenderTargetView*   g_pShadowRTV1 = NULL;
ID3D10Texture2D*          g_pShadowTexture2 = NULL;
ID3D10ShaderResourceView* g_pShadowSRV2 = NULL;
ID3D10RenderTargetView*   g_pShadowRTV2 = NULL;

ID3D10EffectTechnique*  g_pRenderNormal = NULL;
ID3D10EffectTechnique*  g_pRenderDisplaced = NULL;
ID3D10EffectTechnique*  g_pRenderScene = NULL;
ID3D10EffectTechnique*  g_pRenderBlack = NULL;
ID3D10EffectTechnique*  g_pBlurHorz = NULL;
ID3D10EffectTechnique*  g_pBlurVert = NULL;

ID3D10EffectMatrixVariable* g_pmWorldViewProj = NULL;
ID3D10EffectMatrixVariable* g_pmWorld = NULL;
ID3D10EffectMatrixVariable* g_pmWorldView = NULL;
ID3D10EffectMatrixVariable* g_pmView = NULL;
ID3D10EffectMatrixVariable* g_pmViewProj = NULL;
ID3D10EffectMatrixVariable* g_pmProj = NULL;
ID3D10EffectMatrixVariable* g_pmLightViewProj = NULL;
ID3D10EffectMatrixVariable* g_pmBoneWorld = NULL;
ID3D10EffectVectorVariable* g_pvLightPos = NULL;
ID3D10EffectVectorVariable* g_pvEyePt = NULL;
ID3D10EffectShaderResourceVariable* g_ptxDiffuse = NULL;
ID3D10EffectShaderResourceVariable* g_ptxNormal = NULL;
ID3D10EffectShaderResourceVariable* g_ptxDisplace = NULL;
ID3D10EffectShaderResourceVariable* g_ptxShadow = NULL;

D3DXVECTOR3 g_vLightPos = D3DXVECTOR3( 159.47f, 74.23f, -103.60f );
D3DXVECTOR3 g_vLightLook = D3DXVECTOR3( 0,0,0 );
D3DXVECTOR3 g_vShadowUp = D3DXVECTOR3( 0,1,0 );
bool g_bRenderNormalMapped = false;


//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           3
#define IDC_CHANGEDEVICE        4
#define IDC_RENDERNORMALMAPPED  5

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool    CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );
void    CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext );
void    CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void    CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );

bool    CALLBACK IsD3D10DeviceAcceptable( UINT Adapter, UINT Output, D3D10_DRIVER_TYPE DeviceType, DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnD3D10CreateDevice( ID3D10Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
HRESULT CALLBACK OnD3D10SwapChainResized( ID3D10Device* pd3dDevice, IDXGISwapChain *pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void    CALLBACK OnD3D10FrameRender( ID3D10Device* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
void    CALLBACK OnD3D10ReleasingSwapChain( void* pUserContext );
void    CALLBACK OnD3D10DestroyDevice( void* pUserContext );

void    InitApp();
void    RenderText();

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // DXUT will create and use the best device (either D3D9 or D3D10) 
    // that is available on the system depending on which D3D callbacks are set below

    // Set DXUT callbacks
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( KeyboardProc );
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackD3D10DeviceAcceptable( IsD3D10DeviceAcceptable );
    DXUTSetCallbackD3D10DeviceCreated( OnD3D10CreateDevice );
    DXUTSetCallbackD3D10SwapChainResized( OnD3D10SwapChainResized );
    DXUTSetCallbackD3D10SwapChainReleasing( OnD3D10ReleasingSwapChain );
    DXUTSetCallbackD3D10DeviceDestroyed( OnD3D10DestroyDevice );
    DXUTSetCallbackD3D10FrameRender( OnD3D10FrameRender );

    InitApp();
    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"DisplacementMapping10" );
    DXUTCreateDevice( true, 800, 600 );
    DXUTMainLoop(); // Enter into the DXUT render loop

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
    g_D3DSettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );

    g_HUD.SetCallback( OnGUIEvent ); int iY = 10; 
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, iY, 125, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 35, iY += 24, 125, 22, VK_F3 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, iY += 24, 125, 22, VK_F2 );
    g_SampleUI.SetCallback( OnGUIEvent ); iY = 10; 

    g_SampleUI.AddCheckBox( IDC_RENDERNORMALMAPPED, L"Render Normal Mapped", 0, 200, 140, 22, g_bRenderNormalMapped );
}



//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D10 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( (DXUT_D3D9_DEVICE == pDeviceSettings->ver && pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF) ||
            (DXUT_D3D10_DEVICE == pDeviceSettings->ver && pDeviceSettings->d3d10.DriverType == D3D10_DRIVER_TYPE_REFERENCE) )
            DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
    }

    return true;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    // Update the camera's position based on user input 
    g_Camera.FrameMove( fElapsedTime );

    D3DXMATRIX mIdentity;
    D3DXMatrixIdentity( &mIdentity );
    g_LizardMesh.TransformMesh( &mIdentity, fTime );
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
    // Pass messages to dialog resource manager calls so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass messages to settings dialog if its active
    if( g_D3DSettingsDlg.IsActive() )
    {
        g_D3DSettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
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
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
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
        case IDC_CHANGEDEVICE:     g_D3DSettingsDlg.SetActive( !g_D3DSettingsDlg.IsActive() ); break;

        case IDC_RENDERNORMALMAPPED: g_bRenderNormalMapped = !g_bRenderNormalMapped; break;
    }    
}


//--------------------------------------------------------------------------------------
// Reject any D3D10 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D10DeviceAcceptable( UINT Adapter, UINT Output, D3D10_DRIVER_TYPE DeviceType, DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D10 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D10CreateDevice( ID3D10Device* pd3dDevice, const DXGI_SURFACE_DESC *pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D10CreateDevice( pd3dDevice ) );
    V_RETURN( g_D3DSettingsDlg.OnD3D10CreateDevice( pd3dDevice ) );
    V_RETURN( D3DX10CreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                                OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                                L"Arial", &g_pFont10 ) );
    V_RETURN( D3DX10CreateSprite( pd3dDevice, 512, &g_pSprite10 ) );
    g_pTxtHelper = new CDXUTTextHelper( NULL, NULL, g_pFont10, g_pSprite10, 15 );

    // Read the D3DX effect file
    WCHAR str[MAX_PATH];
   
    DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
    #if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3D10_SHADER_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3D10_SHADER_DEBUG;
    #endif

    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"DisplacementMapping10.fx" ) );
    V_RETURN( D3DX10CreateEffectFromFile( str, NULL, NULL, "fx_4_0", dwShaderFlags, 0, pd3dDevice, NULL, NULL, &g_pEffect10, NULL, NULL ) );

    // Obtain the technique handles
    g_pRenderNormal = g_pEffect10->GetTechniqueByName( "RenderNormal" );
    g_pRenderDisplaced = g_pEffect10->GetTechniqueByName( "RenderDisplaced" );
    g_pRenderScene = g_pEffect10->GetTechniqueByName( "RenderScene" );
    g_pRenderBlack = g_pEffect10->GetTechniqueByName( "RenderBlack" );
    g_pBlurHorz = g_pEffect10->GetTechniqueByName( "BlurHorz" );
    g_pBlurVert = g_pEffect10->GetTechniqueByName( "BlurVert" );

    // Obtain the parameter handles
    g_pmWorldViewProj = g_pEffect10->GetVariableByName( "g_mWorldViewProj" )->AsMatrix();
    g_pmWorld = g_pEffect10->GetVariableByName( "g_mWorld" )->AsMatrix();
    g_pmWorldView = g_pEffect10->GetVariableByName( "g_mWorldView" )->AsMatrix();
    g_pmView = g_pEffect10->GetVariableByName( "g_mView" )->AsMatrix();
    g_pmViewProj = g_pEffect10->GetVariableByName( "g_mViewProj" )->AsMatrix();
    g_pmProj = g_pEffect10->GetVariableByName( "g_mProj" )->AsMatrix();
    g_pmLightViewProj = g_pEffect10->GetVariableByName( "g_mLightViewProj" )->AsMatrix();
    g_pmBoneWorld = g_pEffect10->GetVariableByName( "g_mBoneWorld" )->AsMatrix();
    g_pvLightPos = g_pEffect10->GetVariableByName( "g_vLightPos" )->AsVector();
    g_pvEyePt = g_pEffect10->GetVariableByName( "g_vEyePt" )->AsVector();
    g_ptxDiffuse = g_pEffect10->GetVariableByName( "g_txDiffuse" )->AsShaderResource();
    g_ptxNormal = g_pEffect10->GetVariableByName( "g_txNormal" )->AsShaderResource();
    g_ptxDisplace = g_pEffect10->GetVariableByName( "g_txDisplace" )->AsShaderResource();
    g_ptxShadow = g_pEffect10->GetVariableByName( "g_txShadow" )->AsShaderResource();

    // Define our vertex data layout for skinned objects
    const D3D10_INPUT_ELEMENT_DESC skinnedlayout[] =
    {
        { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,			 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        { "WEIGHTS", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0,	    12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        { "BONES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0,			16, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,		20, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,		32, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,		40, D3D10_INPUT_PER_VERTEX_DATA, 0 },
    };
    int numElements = sizeof(skinnedlayout)/sizeof(skinnedlayout[0]);
    D3D10_PASS_DESC PassDesc;
    g_pRenderDisplaced->GetPassByIndex( 0 )->GetDesc( &PassDesc );
    V_RETURN( pd3dDevice->CreateInputLayout( skinnedlayout, numElements, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &g_pDispVertexLayout ) );
    g_pRenderNormal->GetPassByIndex( 0 )->GetDesc( &PassDesc );
    V_RETURN( pd3dDevice->CreateInputLayout( skinnedlayout, numElements, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &g_pNormalVertexLayout ) );

    // Define our scene vertex data layout
    const D3D10_INPUT_ELEMENT_DESC scenelayout[] =
    {
        { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	32, D3D10_INPUT_PER_VERTEX_DATA, 0 },
    };
    numElements = sizeof(scenelayout)/sizeof(scenelayout[0]);
    g_pRenderScene->GetPassByIndex( 0 )->GetDesc( &PassDesc );
    V_RETURN( pd3dDevice->CreateInputLayout( scenelayout, numElements, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &g_pSceneVertexLayout ) );

    // Create our quad vertex input layout
    const D3D10_INPUT_ELEMENT_DESC quadlayout[] =
    {
        { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_UINT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
    };
    g_pBlurHorz->GetPassByIndex( 0 )->GetDesc( &PassDesc );
    V_RETURN( pd3dDevice->CreateInputLayout( quadlayout, sizeof(quadlayout)/sizeof(quadlayout[0]), PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &g_pQuadVertexLayout ) );

    // Load the animated mesh
    V_RETURN( g_LizardMesh.Create( pd3dDevice, L"DisplacementMapping\\lizard.sdkmesh", true ) );
    V_RETURN( g_LizardMesh.LoadAnimation( L"DisplacementMapping\\lizard.sdkmesh_anim" ) );
    D3DXMATRIX mIdentity;
    D3DXMatrixIdentity( &mIdentity );
    g_LizardMesh.TransformBindPose( &mIdentity );
    
    V_RETURN( g_SceneMesh.Create( pd3dDevice, L"DisplacementMapping\\lizardrock.sdkmesh", true ) );

    //Create the shadow texture and views
    D3D10_TEXTURE2D_DESC desc;
    ZeroMemory( &desc, sizeof(D3D10_TEXTURE2D_DESC) );
    desc.Format = DXGI_FORMAT_R8_UNORM;
    desc.Width = SHADOW_SIZE;
    desc.Height = SHADOW_SIZE;
    desc.ArraySize = 1;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D10_USAGE_DEFAULT;
    desc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
    V_RETURN(pd3dDevice->CreateTexture2D( &desc, NULL, &g_pShadowTexture1 ));
    V_RETURN(pd3dDevice->CreateTexture2D( &desc, NULL, &g_pShadowTexture2 ));

    D3D10_RENDER_TARGET_VIEW_DESC DescRTV;
    DescRTV.Format = desc.Format;
    DescRTV.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
    DescRTV.Texture2D.MipSlice = 0;
    V_RETURN(pd3dDevice->CreateRenderTargetView( g_pShadowTexture1, &DescRTV, &g_pShadowRTV1 ));
    V_RETURN(pd3dDevice->CreateRenderTargetView( g_pShadowTexture2, &DescRTV, &g_pShadowRTV2 ));

    D3D10_SHADER_RESOURCE_VIEW_DESC DescSRV;
    DescSRV.Format = desc.Format;
    DescSRV.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
    DescSRV.Texture2D.MipLevels = 1;
    DescSRV.Texture2D.MostDetailedMip = 0;
    V_RETURN(pd3dDevice->CreateShaderResourceView( g_pShadowTexture1, &DescSRV, &g_pShadowSRV1 ));
    V_RETURN(pd3dDevice->CreateShaderResourceView( g_pShadowTexture2, &DescSRV, &g_pShadowSRV2 ));

    // Create a screen quad for render-to-texture
    UINT uiVertBufSize = 4*sizeof(QUAD_VERTEX);
    QUAD_VERTEX pVerts[4];
    pVerts[0].pos = D3DXVECTOR3( -1, -1, 0.5f );
    pVerts[0].tex.x = 0;
    pVerts[0].tex.y = 0;
    pVerts[1].pos = D3DXVECTOR3( -1, 1, 0.5f );
    pVerts[1].tex.x = 0;
    pVerts[1].tex.y = 1.0f;
    pVerts[2].pos = D3DXVECTOR3( 1, -1, 0.5f );
    pVerts[2].tex.x = 1.0f;
    pVerts[2].tex.y = 0;
    pVerts[3].pos = D3DXVECTOR3( 1, 1, 0.5f );
    pVerts[3].tex.x = 1.0f;
    pVerts[3].tex.y = 1.0f;

    D3D10_BUFFER_DESC vbdesc =
    {
        uiVertBufSize,
        D3D10_USAGE_IMMUTABLE,
        D3D10_BIND_VERTEX_BUFFER,
        0,
        0
    };

    D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = pVerts;
    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;
    hr = pd3dDevice->CreateBuffer( &vbdesc, &InitData, &g_pQuadVB );

    // Setup the camera's view parameters
    /*D3DXVECTOR3 vecEye(2.5f, 1.0f, -2.5f);
    vecEye *= 1.5;
    D3DXVECTOR3 vecAt(0.0f,0.1f,0.0f);*/

    D3DXVECTOR3 vecEye(10,5,4);
    D3DXVECTOR3 vecAt(0,0,-2);
    g_Camera.SetViewParams( &vecEye, &vecAt );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D10 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D10SwapChainResized( ID3D10Device* pd3dDevice, IDXGISwapChain *pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr = S_OK;

    V_RETURN( g_DialogResourceManager.OnD3D10ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );
    V_RETURN( g_D3DSettingsDlg.OnD3D10ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI/4, fAspectRatio, 0.1f, 1000.0f );
    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
    g_Camera.SetButtonMasks( MOUSE_LEFT_BUTTON, MOUSE_WHEEL, MOUSE_MIDDLE_BUTTON );

    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width-170, 0 );
    g_HUD.SetSize( 170, 170 );
    g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width-170, pBackBufferSurfaceDesc->Height-300 );
    g_SampleUI.SetSize( 170, 300 );

    return hr;
}

//--------------------------------------------------------------------------------------
// Render the shadow texture
//--------------------------------------------------------------------------------------
void RenderShadowTexture( ID3D10Device* pd3dDevice )
{
    // store off the old RTs
    ID3D10RenderTargetView* pOrigRTV = NULL;
    ID3D10DepthStencilView* pOrigDSV = NULL;
    pd3dDevice->OMGetRenderTargets( 1, &pOrigRTV, &pOrigDSV );

    // store off the old viewport
    UINT NumVP = 1;
    D3D10_VIEWPORT Viewports[1];
    pd3dDevice->RSGetViewports( &NumVP, Viewports );

    // set the new RT
    ID3D10ShaderResourceView* pNULLViews[4] = {NULL,NULL,NULL,NULL};
    pd3dDevice->PSSetShaderResources( 0, 4, pNULLViews );
    pd3dDevice->OMSetRenderTargets( 1, &g_pShadowRTV1, NULL );

    // set the new viewport
    D3D10_VIEWPORT NewViewports[1];
    NewViewports[0].Width = SHADOW_SIZE;
    NewViewports[0].Height = SHADOW_SIZE;
    NewViewports[0].TopLeftX = 0;
    NewViewports[0].TopLeftY = 0;
    NewViewports[0].MinDepth = 0.0f;
    NewViewports[0].MaxDepth = 1.0f;
    pd3dDevice->RSSetViewports( 1, NewViewports );

    // clear the new one to white
    float ClearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    pd3dDevice->ClearRenderTargetView( g_pShadowRTV1, ClearColor);

    //setup the matrices
    D3DXMATRIX mLightView;
    D3DXMATRIX mLightProj;
    D3DXMatrixLookAtLH( &mLightView, &g_vLightPos, &g_vLightLook, &g_vShadowUp );
    D3DXMatrixOrthoLH( &mLightProj, 15.0f, 15.0f, 10.0f, 300.0f );
    D3DXMATRIX mLightViewProj = mLightView*mLightProj;
    
    //set the effect variables
    g_pmViewProj->SetMatrix( (float*)&mLightViewProj );
  
    //IA setup
    pd3dDevice->IASetInputLayout( g_pNormalVertexLayout );

    g_LizardMesh.Render( pd3dDevice, g_pRenderBlack );

    pd3dDevice->OMSetRenderTargets( 1, &pOrigRTV, pOrigDSV );
    pd3dDevice->RSSetViewports( 1, Viewports );
    SAFE_RELEASE( pOrigRTV );
    SAFE_RELEASE( pOrigDSV );
}

//--------------------------------------------------------------------------------------
// Blur Shadow Texture
//--------------------------------------------------------------------------------------
void BlurShadowTex( ID3D10Device* pd3dDevice )
{
    // store off the old RTs
    ID3D10RenderTargetView* pOrigRTV = NULL;
    ID3D10DepthStencilView* pOrigDSV = NULL;
    pd3dDevice->OMGetRenderTargets( 1, &pOrigRTV, &pOrigDSV );

    // store off the old viewport
    UINT NumVP = 1;
    D3D10_VIEWPORT Viewports[1];
    pd3dDevice->RSGetViewports( &NumVP, Viewports );

    // set the new RT
    pd3dDevice->OMSetRenderTargets( 1, &g_pShadowRTV2, NULL );

    // set the new viewport
    D3D10_VIEWPORT NewViewports[1];
    NewViewports[0].Width = SHADOW_SIZE;
    NewViewports[0].Height = SHADOW_SIZE;
    NewViewports[0].TopLeftX = 0;
    NewViewports[0].TopLeftY = 0;
    NewViewports[0].MinDepth = 0.0f;
    NewViewports[0].MaxDepth = 1.0f;
    pd3dDevice->RSSetViewports( 1, NewViewports );

     // Set input params
    pd3dDevice->IASetInputLayout( g_pQuadVertexLayout );
    UINT offsets = 0;
    UINT uStrides[] = { sizeof(QUAD_VERTEX) };
    pd3dDevice->IASetVertexBuffers( 0, 1, &g_pQuadVB, uStrides, &offsets );
    pd3dDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

    ID3D10EffectTechnique* pTech = g_pBlurHorz;
    D3D10_TECHNIQUE_DESC techDesc;
    pTech->GetDesc( &techDesc );
    g_ptxDiffuse->SetResource( g_pShadowSRV1 );

    for( UINT p = 0; p < techDesc.Passes; ++p )
    {
        pTech->GetPassByIndex( p )->Apply(0);
        pd3dDevice->Draw( 4, 0 );
    }

    // unbind all shader resources so that OMSetRenderTargets can cleanly set the previous resource as a RT
    ID3D10ShaderResourceView* pNULLS[4] = { NULL, NULL, NULL, NULL };
    pd3dDevice->PSSetShaderResources( 0, 4, pNULLS );
    // set the new RT
    pd3dDevice->OMSetRenderTargets( 1, &g_pShadowRTV1, NULL );

    pTech = g_pBlurVert;
    pTech->GetDesc( &techDesc );
    g_ptxDiffuse->SetResource( g_pShadowSRV2 );

    for( UINT p = 0; p < techDesc.Passes; ++p )
    {
        pTech->GetPassByIndex( p )->Apply(0);
        pd3dDevice->Draw( 4, 0 );
    }

    pd3dDevice->OMSetRenderTargets( 1, &pOrigRTV, pOrigDSV );
    pd3dDevice->RSSetViewports( 1, Viewports );
    SAFE_RELEASE( pOrigRTV );
    SAFE_RELEASE( pOrigDSV );
}

//--------------------------------------------------------------------------------------
// Render non-displaced scene geometry
//--------------------------------------------------------------------------------------
void RenderSceneGeometry( ID3D10Device* pd3dDevice )
{
    //setup the matrices
    D3DXMATRIX mWorld;
    D3DXMatrixIdentity( &mWorld );
    D3DXMATRIX mProj = *g_Camera.GetProjMatrix();
    D3DXMATRIX mView = *g_Camera.GetViewMatrix();
    D3DXMATRIX mWorldViewProj = mWorld*mView*mProj;

    D3DXMATRIX mLightView;
    D3DXMATRIX mLightProj;
    D3DXMatrixLookAtLH( &mLightView, &g_vLightPos, &g_vLightLook, &g_vShadowUp );
    D3DXMatrixOrthoLH( &mLightProj, 15.0f, 15.0f, 0.1f, 200.0f );
    D3DXMATRIX mLightViewProj = mLightView*mLightProj;

    //effect params
    g_pmWorld->SetMatrix( (float*)&mWorld );
    g_pmWorldViewProj->SetMatrix( (float*)&mWorldViewProj );
    g_pmLightViewProj->SetMatrix( (float*)&mLightViewProj );
    g_ptxShadow->SetResource( g_pShadowSRV1 );

    //IA setup
    pd3dDevice->IASetInputLayout( g_pSceneVertexLayout );
    g_SceneMesh.Render( pd3dDevice, g_pRenderScene, g_ptxDiffuse, g_ptxNormal );
}

//--------------------------------------------------------------------------------------
// Render displaced geometry
//--------------------------------------------------------------------------------------
void RenderDisplacedGeometry( ID3D10Device* pd3dDevice )
{
    D3DXMATRIX mWorld;
    D3DXMATRIX mWorldView;
    D3DXMATRIX mView;
    D3DXMATRIX mViewProj;
    D3DXMATRIX mProj;
    D3DXMATRIX mWorldViewProj;
    mWorld = g_MeshWorld;
    mProj = *g_Camera.GetProjMatrix();
    mView = *g_Camera.GetViewMatrix();
    mWorldViewProj = mWorld*mView*mProj;
    mWorldView = mWorld*mView;
    mViewProj = mView*mProj;

    g_pmWorldViewProj->SetMatrix( (float*)&mWorldViewProj );
    g_pmWorld->SetMatrix( (float*)&mWorld );
    g_pmWorldView->SetMatrix( (float*)&mWorldView );
    g_pmView->SetMatrix( (float*)&mView );
    g_pmViewProj->SetMatrix( (float*)&mViewProj );
    g_pmProj->SetMatrix( (float*)&mProj );

    ID3D10EffectTechnique* pTech = g_pRenderDisplaced;
    if( g_bRenderNormalMapped )
        pTech = g_pRenderNormal;

    // Set up the Input Assembler for rendering
    if( g_bRenderNormalMapped )
        pd3dDevice->IASetInputLayout( g_pNormalVertexLayout );
    else
        pd3dDevice->IASetInputLayout( g_pDispVertexLayout );

    g_LizardMesh.Render( pd3dDevice, pTech, g_ptxDiffuse, g_ptxNormal, g_ptxDisplace );
}

//--------------------------------------------------------------------------------------
// Render the scene using the D3D10 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D10FrameRender( ID3D10Device* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    // Clear the render target
    ID3D10RenderTargetView* pRTV = DXUTGetD3D10RenderTargetView();

    // If the settings dialog is being shown, then render it instead of rendering the app's scene
    if( g_D3DSettingsDlg.IsActive() )
    {
        float ClearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
        pd3dDevice->ClearRenderTargetView( pRTV, ClearColor);
        g_D3DSettingsDlg.OnRender( fElapsedTime );
        return;
    }

    float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    pd3dDevice->ClearRenderTargetView( pRTV, ClearColor);
    ID3D10DepthStencilView* pDSV = DXUTGetD3D10DepthStencilView();
    pd3dDevice->ClearDepthStencilView( pDSV, D3D10_CLEAR_DEPTH, 1.0, 0);
    
    // Set general effect values like bone matrices and light position
    for( UINT i=0; i<g_LizardMesh.GetNumInfluences( 0 ); i++ )
    {
        D3DXMATRIX* pMat = g_LizardMesh.GetMeshInfluenceMatrix( 0, i );
        g_pmBoneWorld->SetMatrixArray( (float*)pMat, i, 1 );
    }

    g_pvLightPos->SetFloatVector( (float*)&g_vLightPos );
    D3DXVECTOR3 vEye = *g_Camera.GetEyePt();
    g_pvEyePt->SetFloatVector( (float*)&vEye );

    //Render the shadowmap
    RenderShadowTexture( pd3dDevice );
    //Blur the shadow texture
    BlurShadowTex( pd3dDevice );
    //Render the scene geometry
    RenderSceneGeometry( pd3dDevice );
    //Render the lizard
    RenderDisplacedGeometry( pd3dDevice );

    DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"HUD / Stats" );
    RenderText();
    g_HUD.OnRender( fElapsedTime );
    g_SampleUI.OnRender( fElapsedTime );
    DXUT_EndPerfEvent();
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text
//--------------------------------------------------------------------------------------
void RenderText()
{
    g_pTxtHelper->Begin();
    g_pTxtHelper->SetInsertionPos( 2, 0 );
    g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    g_pTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );  
    g_pTxtHelper->DrawTextLine( DXUTGetDeviceStats() );
    g_pTxtHelper->End();
}


//--------------------------------------------------------------------------------------
// Release D3D10 resources created in OnD3D10ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D10ReleasingSwapChain( void* pUserContext )
{
    g_DialogResourceManager.OnD3D10ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D10 resources created in OnD3D10CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D10DestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D10DestroyDevice();
    g_D3DSettingsDlg.OnD3D10DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();
    SAFE_RELEASE( g_pFont10 );
    SAFE_RELEASE( g_pSprite10 );
    SAFE_DELETE( g_pTxtHelper );
    SAFE_RELEASE( g_pEffect10 );
    SAFE_RELEASE( g_pNormalVertexLayout );
    SAFE_RELEASE( g_pDispVertexLayout );
    SAFE_RELEASE( g_pSceneVertexLayout );
    SAFE_RELEASE( g_pQuadVertexLayout );

    SAFE_RELEASE( g_pShadowTexture1 );
    SAFE_RELEASE( g_pShadowRTV1 );
    SAFE_RELEASE( g_pShadowSRV1 );

    SAFE_RELEASE( g_pShadowTexture2 );
    SAFE_RELEASE( g_pShadowRTV2 );
    SAFE_RELEASE( g_pShadowSRV2 );

    SAFE_RELEASE( g_pQuadVB );

    g_LizardMesh.Destroy();
    g_SceneMesh.Destroy();
}