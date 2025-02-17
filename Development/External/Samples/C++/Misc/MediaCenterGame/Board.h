//--------------------------------------------------------------------------------------
// File: Board.h
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#ifndef _BOARD_H_
#define _BOARD_H_

#include "quad.h"

#define NUM_ROWS 8

struct Cell
{
    int Row;
    int Column;

    Cell( int row, int column ) { Row = row; Column = column; }
};

enum Color
{
    None,
    White,
    Black
};

struct BoardVertex
{
    D3DXVECTOR3 Position;
    D3DXVECTOR3 Normal;
    D3DXVECTOR2 TexCoord;
    D3DCOLOR Color;
};

struct AnimationEvent
{
    float CurrentRotation;

    float StartTime;
    float StartRotation;

    float EndTime;
    float EndRotation;
};



//--------------------------------------------------------------------------------------
class CBoard
{
public:
    CBoard();
    CBoard( D3DXVECTOR3 position );
    CBoard( D3DXVECTOR3 position, float size );
    void Reset();

    HRESULT OnCreateDevice( IDirect3DDevice9* pd3dDevice, DWORD shaderFlags=0 );
    HRESULT OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc );
    void OnLostDevice();
    void OnDestroyDevice();

    void Render( IDirect3DDevice9* pd3dDevice );
    void RenderBoard( IDirect3DDevice9* pd3dDevice );

    void SetPosition( D3DXVECTOR3 position ) { Position = position; }
    float GetPositionX() { return Position.x; }
    float GetPositionY() { return Position.y; }
    float GetPositionZ() { return Position.z; }

    void SetSize( float size ) { Size = size; }
    float GetSize() { return Size; }

    bool LegalMoveExists( Color color );
    bool IsLegalMove( Color disc, int row, int column );
    bool IsValidCellAddress( int row, int column );
    int NumberCaptured( Color disc, int row, int column );
    int NumberCapturedInDirection( Color disc, int row, int column, int incRow, int incColumn );

    Cell GetCellAtPoint( IDirect3DDevice9* pd3dDevice, int x, int y );
    bool PerformMove( Color disc, int row, int column );
    Color GetActiveColor() { return ActiveColor; }
    void SetActiveColor( Color color ) { ActiveColor = color; }

    int GetScore( Color color );
    bool IsFinished() { return m_IsFinished; }

private:
    HRESULT CreateVertexDeclaration( IDirect3DDevice9* pd3dDevice );
    HRESULT CreateVertexBuffer( IDirect3DDevice9* pd3dDevice );
    
    void Initialize( D3DXVECTOR3 position=D3DXVECTOR3(100, 100, 0), float size=100 );
    void InitializeCellRotations();
    void CalculateBoardWorldMatrix( D3DXMATRIX* world );
    void CalculateDiscWorldMatrix( D3DXMATRIX* world, int row, int column );
    void IncrementBoardState();
    void AnimateBoard();

    // Variables
    bool m_IsFinished;
    D3DXVECTOR3 Position;
    float Size;
    
    AnimationEvent BoardAnimation;
    AnimationEvent CellAnimations[NUM_ROWS][NUM_ROWS];

    ID3DXMesh* pDiscMesh;
    Color Discs[NUM_ROWS][NUM_ROWS];
    Color ActiveColor;

    //debug
    CQuad RippleQuad;
    float RippleScale;
    float RippleAlpha;
    int RippleRow;
    int RippleColumn;

    ID3DXEffect* pEffect;
    IDirect3DVertexBuffer9* pVertexBuffer;
    IDirect3DVertexDeclaration9* pVertexDeclaration;
};

#endif //_BOARD_H_