//--------------------------------------------------------------------------------------
// File: Quad.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "SDKmisc.h"
#include "quad.h"


IDirect3DDevice9* CQuad::pDevice;
IDirect3DVertexBuffer9* CQuad::pVertexBuffer;
IDirect3DVertexDeclaration9* CQuad::pVertexDeclaration;


//--------------------------------------------------------------------------------------
CQuad::CQuad()
{
    pDevice = NULL;
}


//--------------------------------------------------------------------------------------
HRESULT CQuad::Create( IDirect3DDevice9* pd3dDevice, const WCHAR* effectFilename, DWORD shaderFlags )
{
    HRESULT hr;

    // Only call the static resource creation functions if a device doesn't already exist
    if( pDevice == NULL )
    {
        // Store it
        pDevice = pd3dDevice;
        pDevice->AddRef();

        V_RETURN( CreateVertexDeclaration( pd3dDevice ) );
        V_RETURN( CreateVertexBuffer( pd3dDevice ) );
    }

    // Determine the matrix transforms
    D3DXMatrixIdentity( &World );
    InitializeViewAndProjection( D3DX_PI/4 );
    
    WCHAR fullPath[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( fullPath, MAX_PATH, effectFilename ) );
    V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, fullPath, NULL, NULL, shaderFlags, NULL, &pEffect, NULL ) );

    D3DXHANDLE hTechnique = NULL;
    V_RETURN( pEffect->FindNextValidTechnique(NULL, &hTechnique) );
    V_RETURN( pEffect->SetTechnique( hTechnique ) );

    return S_OK;
}


//--------------------------------------------------------------------------------------
void CQuad::Destroy()
{
    SAFE_RELEASE( pEffect );
    SAFE_RELEASE( pVertexBuffer );
    SAFE_RELEASE( pVertexDeclaration );
    SAFE_RELEASE( pDevice );
}


//--------------------------------------------------------------------------------------
void CQuad::OnLostDevice()
{
    if( pEffect )
        pEffect->OnLostDevice();
}


//--------------------------------------------------------------------------------------
HRESULT CQuad::OnResetDevice()
{
    HRESULT hr = S_OK;

    if( pEffect )
        V_RETURN( pEffect->OnResetDevice() );

    InitializeViewAndProjection( D3DX_PI/4 );

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CQuad::CreateVertexDeclaration( IDirect3DDevice9* pd3dDevice )
{
    HRESULT hr;

    D3DVERTEXELEMENT9 elements[] =
    {
        { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
        { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        { 0, 32, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
        D3DDECL_END()
    };

    V_RETURN( pd3dDevice->CreateVertexDeclaration( elements, &pVertexDeclaration ) );

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CQuad::CreateVertexBuffer( IDirect3DDevice9* pd3dDevice )
{
    HRESULT hr;

    QuadVertex corners[] = 
    {
        QuadVertex( D3DXVECTOR3( -0.5f, -0.5f, 0 ), D3DXVECTOR3( 0, 0, 1 ), D3DXVECTOR2( 0.0f, 0.0f ), D3DCOLOR_ARGB( 255, 255, 255, 255 ) ),
        QuadVertex( D3DXVECTOR3(  0.5f, -0.5f, 0 ), D3DXVECTOR3( 0, 0, 1 ), D3DXVECTOR2( 1.0f, 0.0f ), D3DCOLOR_ARGB( 255, 255, 255, 255 ) ),
        QuadVertex( D3DXVECTOR3(  0.5f,  0.5f, 0 ), D3DXVECTOR3( 0, 0, 1 ), D3DXVECTOR2( 1.0f, 1.0f ), D3DCOLOR_ARGB( 255, 255, 255, 255 ) ),
        QuadVertex( D3DXVECTOR3( -0.5f,  0.5f, 0 ), D3DXVECTOR3( 0, 0, 1 ), D3DXVECTOR2( 0.0f, 1.0f ), D3DCOLOR_ARGB( 255, 255, 255, 255 ) )
    };

    // Create
    V_RETURN( pd3dDevice->CreateVertexBuffer( sizeof(QuadVertex) * 4,
                                              D3DUSAGE_WRITEONLY,
                                              0,
                                              D3DPOOL_MANAGED,
                                              &pVertexBuffer,
                                              NULL ) );

    // Fill
    QuadVertex* pVertex = NULL;
    V_RETURN( pVertexBuffer->Lock( 0, 0, (void**)&pVertex, 0 ) );
    CopyMemory( pVertex, corners, sizeof(corners) );
    pVertexBuffer->Unlock();

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CQuad::LoadTexture( D3DXHANDLE effectParameterHandle, const WCHAR* filename )
{
    HRESULT hr = S_OK;

    WCHAR fullPath[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( fullPath, MAX_PATH, filename ) );

    IDirect3DTexture9* pTexture = NULL;
    V_RETURN( D3DXCreateTextureFromFile( pDevice, fullPath, &pTexture ) );

    pEffect->SetTexture( effectParameterHandle, pTexture );
    
    SAFE_RELEASE( pTexture );

    return S_OK;
}


//--------------------------------------------------------------------------------------
void CQuad::Render()
{
    HRESULT hr;

    V( pEffect->SetMatrix( "World", &World ) );
    V( pEffect->SetMatrix( "View", &View ) );
    V( pEffect->SetMatrix( "Projection", &Projection ) );

    // Render the board
    UINT NumPasses;
    V( pEffect->Begin( &NumPasses, 0 ) );
    for( UINT iPass=0; iPass < NumPasses; iPass++ )
    {
        V( pEffect->BeginPass( iPass ) );
        DrawPrimitives();
        V( pEffect->EndPass() );
    }
    V( pEffect->End() );
}


//--------------------------------------------------------------------------------------
void CQuad::DrawPrimitives()
{
    HRESULT hr;

    V( pDevice->SetVertexDeclaration( pVertexDeclaration ) );
    V( pDevice->SetStreamSource( 0, pVertexBuffer, 0, sizeof(QuadVertex) ) );
    V( pDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 ) ); 
}


//--------------------------------------------------------------------------------------
// Moves the world-space XY plane into screen-space for pixel-perfect perspective
//--------------------------------------------------------------------------------------
void CQuad::InitializeViewAndProjection( float fovy, float screenDepth, float nearClip )
{
    // Get back buffer description and determine aspect ratio
    const D3DSURFACE_DESC* pBackBufferSurfaceDesc = DXUTGetD3D9BackBufferSurfaceDesc();
    float Width = (float)pBackBufferSurfaceDesc->Width;
    float Height = (float)pBackBufferSurfaceDesc->Height;
    float fAspectRatio = Width/Height;

    // Determine the correct Z depth to completely fill the frustum
    float yScale = 1 / tanf( fovy/2 );
    float z = yScale * Height / 2;
    
    // Calculate perspective projection
    D3DXMatrixPerspectiveFovLH( &Projection, fovy, fAspectRatio, nearClip, z + screenDepth );

    // Initialize the view matrix as a rotation and translation from "screen-coordinates"
    // in world space (the XY plane from the perspective of Z+) to a plane that's centered
    // along Z+
    D3DXMatrixIdentity( &View );
    View._22 = -1;
    View._33 = -1;
    View._41 = -(Width/2);
    View._42 = (Height/2);
    View._43 = z;
}


/*
//--------------------------------------------------------------------------------------
HRESULT CTexturedQuad::Create( IDirect3DDevice9* pd3dDevice, DWORD shaderFlags )
{
    // Base implementation
    CQuad::Create( pd3dDevice );

    HRESULT hr;

    WCHAR str[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"TexturedQuad.fx" ) );
    V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, shaderFlags, NULL, &pEffect, NULL ) );

    return S_OK;
}


//--------------------------------------------------------------------------------------
void CTexturedQuad::Destroy()
{
    // Base implementation
    CQuad::Destroy();

    SAFE_RELEASE( pEffect );
}


//--------------------------------------------------------------------------------------
void CTexturedQuad::Render()
{
    HRESULT hr;

    V( pEffect->SetMatrix( "World", &world ) );
    V( pEffect->SetMatrix( "View", &view ) );
    V( pEffect->SetMatrix( "Projection", &projection ) );

    // Render the board
    UINT NumPasses;
    pEffect->SetTechnique( "TexturedQuad" );
    V( pEffect->Begin( &NumPasses, 0 ) );
    for( UINT iPass=0; iPass < NumPasses; iPass++ )
    {
        V( pEffect->BeginPass( iPass ) );
        V( pDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 ) );
        V( pEffect->EndPass() );
    }
    V( pEffect->End() );
}
*/
                            