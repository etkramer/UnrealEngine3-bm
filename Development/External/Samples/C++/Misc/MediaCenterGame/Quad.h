//--------------------------------------------------------------------------------------
// File: Quad.h
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#ifndef _QUAD_H_
#define _QUAD_H_

struct QuadVertex
{
    D3DXVECTOR3 Position;
    D3DXVECTOR3 Normal;
    D3DXVECTOR2 TexCoord;
    D3DCOLOR Color;

    QuadVertex()
    {
        ZeroMemory( this, sizeof(QuadVertex) );
    }

    QuadVertex( D3DXVECTOR3 position, D3DXVECTOR3 normal, D3DXVECTOR2 texCoord, D3DCOLOR color )
    {
        Position = position; Normal = normal; TexCoord = texCoord; Color = color;
    }
};


class CQuad
{
public:
    ID3DXEffect* pEffect;
    D3DXMATRIXA16 World;
    D3DXMATRIXA16 View;
    D3DXMATRIXA16 Projection;

    CQuad();
    
    HRESULT Create( IDirect3DDevice9* pd3dDevice, const WCHAR* effectFilename, DWORD shaderFlags=0 );

    void Destroy();

    void OnLostDevice();
    HRESULT OnResetDevice();
    
    HRESULT LoadTexture( D3DXHANDLE effectParameterHandle, const WCHAR* filename );

    void Render();
    void DrawPrimitives();

    //debug - is there a need here?
    /*
    void SetPositionCenter( float x, float y, float z=0 );
    void SetPositionCenter( D3DXVECTOR3 p ) { return SetPositionCenter( p.x, p.y, p.z ); }

    void SetPositionTopLeft( float x, float y, float z=0 );
    void SetPositionTopLeft( D3DXVECTOR3 p ) { return SetPositionTopLeft( p.x, p.y, p.z ); }

    void SetSize
    */

private:
    static HRESULT CreateVertexDeclaration( IDirect3DDevice9* pd3dDevice );
    static HRESULT CreateVertexBuffer( IDirect3DDevice9* pd3dDevice );

    void InitializeViewAndProjection( float fovy, float screenDepth=10000, float nearClip=0.1f );

    static IDirect3DDevice9* pDevice;
    static IDirect3DVertexBuffer9* pVertexBuffer;
    static IDirect3DVertexDeclaration9* pVertexDeclaration;
};

/*
class CTexturedQuad : public CQuad
{
public:
    HRESULT Create( IDirect3DDevice9* pd3dDevice, DWORD shaderFlags );
    void Destroy();
    
    void Render();

    void SetTexture( IDirect3DTexture9* pTexture ) { this->pTexture = pTexture; }

private:
    IDirect3DTexture9* pTexture;
    ID3DXEffect* pEffect;
};
*/

#endif //_QUAD_H_