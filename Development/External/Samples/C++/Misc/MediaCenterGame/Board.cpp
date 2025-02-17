//--------------------------------------------------------------------------------------
// File: Board.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "SDKmisc.h"
#include "math.h"
#include "board.h"

//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 

//--------------------------------------------------------------------------------------
// Moves the world-space XY plane into screen-space for pixel-perfect perspective
//--------------------------------------------------------------------------------------
HRESULT CalculateViewAndProjection( D3DXMATRIX* pViewOut, D3DXMATRIX* pProjectionOut, float fovy, float screenDepth=10000, float nearClip=0.1f )
{
    if( pViewOut == NULL ||
        pProjectionOut == NULL )
        return E_INVALIDARG;

    // Get back buffer description and determine aspect ratio
    const D3DSURFACE_DESC* pBackBufferSurfaceDesc = DXUTGetD3D9BackBufferSurfaceDesc();
    float Width = (float)pBackBufferSurfaceDesc->Width;
    float Height = (float)pBackBufferSurfaceDesc->Height;
    float fAspectRatio = Width/Height;

    // Determine the correct Z depth to completely fill the frustum
    float yScale = 1 / tanf( fovy/2 );
    float z = yScale * Height / 2;
    
    // Calculate perspective projection
    D3DXMatrixPerspectiveFovLH( pProjectionOut, fovy, fAspectRatio, nearClip, z + screenDepth );

    // Initialize the view matrix as a rotation and translation from "screen-coordinates"
    // in world space (the XY plane from the perspective of Z+) to a plane that's centered
    // along Z+
    D3DXMatrixIdentity( pViewOut );
    pViewOut->_22 = -1;
    pViewOut->_33 = -1;
    pViewOut->_41 = -(Width/2);
    pViewOut->_42 = (Height/2);
    pViewOut->_43 = z;

    
    return S_OK;
}


//--------------------------------------------------------------------------------------
CBoard::CBoard( D3DXVECTOR3 position, float size )
{
    Initialize( position, size );
}


//--------------------------------------------------------------------------------------
CBoard::CBoard( D3DXVECTOR3 position )
{
    Initialize( position );
}


//--------------------------------------------------------------------------------------
CBoard::CBoard()
{
    Initialize();
}


//--------------------------------------------------------------------------------------
void CBoard::Initialize( D3DXVECTOR3 position, float size )
{
    Position = position;
    
    pVertexBuffer = NULL;
    pDiscMesh = NULL;

    ZeroMemory( &CellAnimations, sizeof(CellAnimations) );
    ZeroMemory( &BoardAnimation, sizeof(BoardAnimation) );

    SetSize(size);
    Reset();
}


//--------------------------------------------------------------------------------------
void CBoard::Reset()
{
    ZeroMemory( &Discs, sizeof(Discs) );
    
    // Initial position
    Discs[3][3] = White;
    Discs[3][4] = Black;
    Discs[4][3] = Black;
    Discs[4][4] = White;

    InitializeCellRotations();

    ActiveColor = White;
    m_IsFinished = false;
 
    AnimateBoard();   
}


//--------------------------------------------------------------------------------------
void CBoard::AnimateBoard()
{
    BoardAnimation.StartTime = (float)DXUTGetTime();
    BoardAnimation.EndTime = BoardAnimation.StartTime + 0.7f;
    BoardAnimation.StartRotation = BoardAnimation.CurrentRotation;

    switch( ActiveColor )
    {
        case None:
            BoardAnimation.EndRotation = 0;
            break;

        case White:
            BoardAnimation.EndRotation = -D3DX_PI/8;
            break;

        case Black:
            BoardAnimation.EndRotation = D3DX_PI/8;
            break;
    }    
}

//--------------------------------------------------------------------------------------
void CBoard::InitializeCellRotations()
{
    for( int row=0; row < NUM_ROWS; row++ )
    {
        for( int column=0; column < NUM_ROWS; column++ )
        {
            switch( Discs[row][column] )
            {
            case White:
                CellAnimations[row][column].EndRotation = D3DX_PI/2;
                break;

            case Black:
                CellAnimations[row][column].EndRotation = 3 * D3DX_PI/2;
                break;

            case None:
                CellAnimations[row][column].EndRotation = 0;
                break;

            default:
                assert(false);
            }
        }
    }
}


//--------------------------------------------------------------------------------------
HRESULT CBoard::OnCreateDevice( IDirect3DDevice9* pd3dDevice, DWORD shaderFlags )
{
    HRESULT hr;

    V_RETURN( CreateVertexDeclaration( pd3dDevice ) );
    V_RETURN( CreateVertexBuffer( pd3dDevice ) );

    WCHAR str[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"Board.fx" ) );
    V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, shaderFlags, NULL, &pEffect, NULL ) );

    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"Disc.x" ) );
    V_RETURN( D3DXLoadMeshFromX( str, D3DXMESH_MANAGED, pd3dDevice, NULL, NULL, NULL, NULL, &pDiscMesh ) );

    IDirect3DCubeTexture9* pCubeTexture = NULL;
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"Lobby\\LobbyCube.dds" ) );
    V_RETURN( D3DXCreateCubeTextureFromFile( pd3dDevice, str, &pCubeTexture ) );
    pEffect->SetTexture( "EnvironmentMap", pCubeTexture );
    SAFE_RELEASE( pCubeTexture );

    IDirect3DTexture9* pTexture = NULL;
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"Board.tga" ) );
    V_RETURN( D3DXCreateTextureFromFile( pd3dDevice, str, &pTexture ) );
    pEffect->SetTexture( "BoardTexture", pTexture );
    SAFE_RELEASE( pTexture );

    RippleQuad.Create( pd3dDevice, L"TexturedQuad.fx" );
    RippleQuad.LoadTexture( "QuadTexture", L"Ripple.png" );

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CBoard::OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc )
{
    HRESULT hr;

    V_RETURN( pEffect->OnResetDevice() );
    V_RETURN( RippleQuad.OnResetDevice() );

    return S_OK;
}


//--------------------------------------------------------------------------------------
void CBoard::OnLostDevice()
{
    HRESULT hr;

    RippleQuad.OnLostDevice();
    V( pEffect->OnLostDevice() );
}


//--------------------------------------------------------------------------------------
void CBoard::OnDestroyDevice()
{
    RippleQuad.Destroy();

    SAFE_RELEASE( pDiscMesh );
    SAFE_RELEASE( pEffect );
    SAFE_RELEASE( pVertexBuffer );
    SAFE_RELEASE( pVertexDeclaration );
}


//--------------------------------------------------------------------------------------
HRESULT CBoard::CreateVertexDeclaration( IDirect3DDevice9* pd3dDevice )
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
HRESULT CBoard::CreateVertexBuffer( IDirect3DDevice9* pd3dDevice )
{
    HRESULT hr;

    SAFE_RELEASE( pVertexBuffer );

    // Create
    V_RETURN( pd3dDevice->CreateVertexBuffer( sizeof(BoardVertex) * 4,
                                              D3DUSAGE_WRITEONLY,
                                              0,
                                              D3DPOOL_MANAGED,
                                              &pVertexBuffer,
                                              NULL ) );

    // Fill
    BoardVertex* pVertex = NULL;
    V_RETURN( pVertexBuffer->Lock( 0, 0, (void**)&pVertex, 0 ) );

    /*
    for( int row=0; row <= NUM_ROWS; row++ )
    {
        // Rows
        pVertex->Position = D3DXVECTOR3( -0.5f, ((float)row/NUM_ROWS)-0.5f, 0 );
        pVertex->Normal = D3DXVECTOR3( 0, 0, 1 );
        pVertex->Color = D3DCOLOR_ARGB( 255, 255, 255, 255 );
        pVertex++;

        pVertex->Position = D3DXVECTOR3( 0.5f, ((float)row/NUM_ROWS)-0.5f, 0 );
        pVertex->Normal = D3DXVECTOR3( 0, 0, 1 );
        pVertex->Color = D3DCOLOR_ARGB( 255, 255, 255, 255 );
        pVertex++;

        // Columns
        pVertex->Position = D3DXVECTOR3( ((float)row/NUM_ROWS)-0.5f, -0.5f, 0 );
        pVertex->Normal = D3DXVECTOR3( 0, 0, 1 );
        pVertex->Color = D3DCOLOR_ARGB( 255, 255, 255, 255 );
        pVertex++;

        pVertex->Position = D3DXVECTOR3( ((float)row/NUM_ROWS)-0.5f, 0.5f, 0 );
        pVertex->Normal = D3DXVECTOR3( 0, 0, 1 );
        pVertex->Color = D3DCOLOR_ARGB( 255, 255, 255, 255 );
        pVertex++;
    }
    */
    
    pVertex->Position = D3DXVECTOR3( -0.5f, -0.5f, 0 );
    pVertex->Normal = D3DXVECTOR3( 0, 0, 1 );
    pVertex->TexCoord = D3DXVECTOR2( 0.0f, 0.0f );
    pVertex->Color = D3DCOLOR_ARGB( 255, 255, 255, 255 );
    pVertex++;

    pVertex->Position = D3DXVECTOR3( 0.5f, -0.5f, 0  );
    pVertex->Normal = D3DXVECTOR3( 0, 0, 1 );
    pVertex->TexCoord = D3DXVECTOR2( 1.0f, 0.0f );
    pVertex->Color = D3DCOLOR_ARGB( 255, 255, 255, 255 );
    pVertex++;

    pVertex->Position = D3DXVECTOR3( 0.5f, 0.5f, 0  );
    pVertex->Normal = D3DXVECTOR3( 0, 0, 1 );
    pVertex->TexCoord = D3DXVECTOR2( 1.0f, 1.0f );
    pVertex->Color = D3DCOLOR_ARGB( 255, 255, 255, 255 );
    pVertex++;

    pVertex->Position = D3DXVECTOR3( -0.5f, 0.5f, 0  );
    pVertex->Normal = D3DXVECTOR3( 0, 0, 1 );
    pVertex->TexCoord = D3DXVECTOR2( 0.0f, 1.0f );
    pVertex->Color = D3DCOLOR_ARGB( 255, 255, 255, 255 );
    pVertex++;

    pVertexBuffer->Unlock();

    return S_OK;
}


//--------------------------------------------------------------------------------------
void CBoard::CalculateBoardWorldMatrix( D3DXMATRIX* world )
{
    D3DXMATRIXA16 scaling;
    D3DXMatrixScaling( &scaling, Size, Size, Size );

    D3DXMATRIXA16 translation;
    D3DXMatrixTranslation( &translation, Position.x, Position.y, Position.z );

    D3DXMATRIXA16 rotation;
    D3DXMatrixRotationY( &rotation, BoardAnimation.CurrentRotation );

    D3DXMatrixMultiply( world, &scaling, &rotation );
    D3DXMatrixMultiply( world, world, &translation );
}


//--------------------------------------------------------------------------------------
void CBoard::CalculateDiscWorldMatrix( D3DXMATRIX* world, int row, int column )
{
    float cellSize = Size / NUM_ROWS;

    D3DXMATRIXA16 scaling;
    float cellScale = 0.9f * cellSize;
    D3DXMatrixScaling( &scaling, cellScale, cellScale, cellScale );

    float x = cellSize * column - ((NUM_ROWS/2 - 0.5f) * cellSize);
    float y = cellSize * row - ((NUM_ROWS/2 - 0.5f) * cellSize);

    D3DXMATRIXA16 translation;
    D3DXMatrixTranslation( &translation, x, y, Position.z );

    D3DXMATRIXA16 rotation;
    D3DXMatrixRotationY( &rotation, BoardAnimation.CurrentRotation );

    D3DXMatrixMultiply( world, &scaling, &translation );
    D3DXMatrixMultiply( world, world, &rotation );

    D3DXMatrixTranslation( &translation, Position.x, Position.y, Position.z );
    D3DXMatrixMultiply( world, world, &translation );
}


//--------------------------------------------------------------------------------------
void CBoard::Render( IDirect3DDevice9* pd3dDevice )
{
    HRESULT hr;
    UINT NumPasses;
    D3DXVECTOR4 Color;

    // Render the board
    RenderBoard( pd3dDevice );

    //debug
    if( RippleQuad.pEffect == NULL )
        return;

    float elapsedTime = DXUTGetElapsedTime();
    RippleAlpha *= 0.98f;
    RippleQuad.pEffect->SetFloat( "AlphaMultiplier", RippleAlpha );

    RippleScale += 20*elapsedTime;
    D3DXMATRIXA16 mScaling, mWorld;
    D3DXMatrixScaling( &mScaling, RippleScale, RippleScale, RippleScale );
    CalculateDiscWorldMatrix( &mWorld, RippleRow, RippleColumn );
    D3DXMatrixMultiply( &RippleQuad.World, &mScaling, &mWorld );
    RippleQuad.Render();

    D3DXMATRIXA16 world, view, projection;
    CalculateViewAndProjection( &view, &projection, D3DX_PI/4 );

    // Set effect variables
    V( pEffect->SetMatrix( "View", &view ) );
    V( pEffect->SetMatrix( "Projection", &projection ) );

    // Render the discs    
    pEffect->SetTechnique( "Disc" );
    V( pEffect->Begin( &NumPasses, 0 ) );
    for( UINT iPass=0; iPass < NumPasses; iPass++ )
    {
        V( pEffect->BeginPass( iPass ) );

        for( int row=0; row < NUM_ROWS; row++ )
        {
            for( int column=0; column < NUM_ROWS; column++ )
            {
                if( Discs[row][column] == None )
                    continue;

                AnimationEvent& cellAnimation = CellAnimations[row][column];
                
                // Handle any active animations
                if( cellAnimation.StartTime < DXUTGetTime() )
                {
                    if( cellAnimation.EndTime > DXUTGetTime() )
                    {
                        float delta = (cellAnimation.EndRotation - cellAnimation.StartRotation);

                        float ratio = ((float)DXUTGetTime()-cellAnimation.StartTime) / (cellAnimation.EndTime-cellAnimation.StartTime);
                        ratio = powf( ratio, 0.2f );
                        cellAnimation.CurrentRotation = cellAnimation.StartRotation + (ratio * delta);
                    }
                    else
                    {
                        cellAnimation.CurrentRotation = cellAnimation.EndRotation;
                    }
                }

                D3DXMATRIXA16 rotation;
                D3DXMatrixRotationX( &rotation, cellAnimation.CurrentRotation );
   
                CalculateDiscWorldMatrix( &world, row, column );
                D3DXMatrixMultiply( &world, &rotation, &world );

                V( pEffect->SetMatrix( "World", &world ) );
                
                // Set the disc color
                Color = D3DXVECTOR4( 0, 0, 0, 0 );

                if( Discs[row][column] == White )
                    Color = D3DXVECTOR4( 1, 1, 1, 1 );
                else if( Discs[row][column] == Black )
                    Color = D3DXVECTOR4( 0, 0, 0, 1 );

                if( IsLegalMove( ActiveColor, row, column ) )
                    Color = D3DXVECTOR4( 0, 0, 1, 0.5f );

                V( pEffect->CommitChanges() );
    
                //if( !(Discs[row][column] == Black && cellAnimation.EndTime < DXUTGetTime() ) )
                V( pDiscMesh->DrawSubset(0) );

                //if( !(Discs[row][column] == White && cellAnimation.EndTime < DXUTGetTime() ) )
                V( pDiscMesh->DrawSubset(2) );
            }
        }

        V( pEffect->EndPass() );
    }
    V( pEffect->End() );
}


//--------------------------------------------------------------------------------------
void CBoard::RenderBoard( IDirect3DDevice9* pd3dDevice )
{
    HRESULT hr;

    if( pEffect == NULL )
        return;

    if( BoardAnimation.StartTime < DXUTGetTime() )
    {
        if( BoardAnimation.EndTime > DXUTGetTime() )
        {
            float delta = (BoardAnimation.EndRotation - BoardAnimation.StartRotation);
            float ratio = ((float)DXUTGetTime()-BoardAnimation.StartTime) / (BoardAnimation.EndTime-BoardAnimation.StartTime);
            ratio = sinf( ratio * D3DX_PI/2 );
            BoardAnimation.CurrentRotation = BoardAnimation.StartRotation + (ratio * delta);
        }
        else
        {
            BoardAnimation.CurrentRotation = BoardAnimation.EndRotation;
        }
    }

    // Configure the device
    V( pd3dDevice->SetVertexDeclaration( pVertexDeclaration ) );
    V( pd3dDevice->SetStreamSource( 0, pVertexBuffer, 0, sizeof(BoardVertex) ) );

    //debug
    float boardAlpha = 0.8f;
    for( int i=0; i < 8; i++ )
    {
        D3DXMATRIXA16 world, view, projection;
        CalculateBoardWorldMatrix( &world );
        CalculateViewAndProjection( &view, &projection, D3DX_PI/4 );

        //debug
        if( i == 1 )
        {
            boardAlpha = 0.15f;
            D3DXMATRIXA16 translation;
            D3DXMatrixTranslation( &translation, 0, 0, -0.1f );
            D3DXMatrixMultiply( &world, &translation, &world );
        }
        if( i == 2 )
        {
            boardAlpha = 0.1f;
            D3DXMATRIXA16 translation;
            D3DXMatrixTranslation( &translation, 0, 0, -0.2f );
            D3DXMatrixMultiply( &world, &translation, &world );
        }

        if( i == 3 )
        {
            boardAlpha = 0.05f;
            D3DXMATRIXA16 translation;
            D3DXMatrixTranslation( &translation, 0, 0, -0.3f );
            D3DXMatrixMultiply( &world, &translation, &world );
        }
        if( i == 4 )
        {
            boardAlpha = 0.15f;
            D3DXMATRIXA16 translation;
            D3DXMatrixTranslation( &translation, 0, 0, 0.1f );
            D3DXMatrixMultiply( &world, &translation, &world );
        }
        if( i == 5 )
        {
            boardAlpha = 0.1f;
            D3DXMATRIXA16 translation;
            D3DXMatrixTranslation( &translation, 0, 0, 0.2f );
            D3DXMatrixMultiply( &world, &translation, &world );
        }

        if( i == 6 )
        {
            boardAlpha = 0.05f;
            D3DXMATRIXA16 translation;
            D3DXMatrixTranslation( &translation, 0, 0, 0.3f );
            D3DXMatrixMultiply( &world, &translation, &world );
        }

        if( i == 7 )
        {
            boardAlpha = 0.02f;
            D3DXMATRIXA16 translation;
            D3DXMatrixTranslation( &translation, 0, 0, 0.4f );
            D3DXMatrixMultiply( &world, &translation, &world );
        }

        V( pEffect->SetFloat( "BoardAlpha", boardAlpha ) );
        V( pEffect->SetMatrix( "World", &world ) );
        V( pEffect->SetMatrix( "View", &view ) );
        V( pEffect->SetMatrix( "Projection", &projection ) );

        // Render the board
        UINT NumPasses;
        pEffect->SetTechnique( "Board" );
        V( pEffect->Begin( &NumPasses, 0 ) );
        for( UINT iPass=0; iPass < NumPasses; iPass++ )
        {
            V( pEffect->BeginPass( iPass ) );
            V( pd3dDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 ) );
            V( pEffect->EndPass() );
        }
        V( pEffect->End() );
    }
}


//--------------------------------------------------------------------------------------
Cell CBoard::GetCellAtPoint( IDirect3DDevice9* pd3dDevice, int x, int y )
{
    D3DXVECTOR3 point( (float)x, (float)y, 0.0f );
    D3DXVECTOR3 direction( 0.0f, 0.0f, 1.0f );

    D3DXMATRIXA16 world, view, projection;
    CalculateViewAndProjection( &view, &projection, D3DX_PI/4 );

    D3DVIEWPORT9 viewport;
    pd3dDevice->GetViewport( &viewport );

    for( int row=0; row < NUM_ROWS; row++ )
    {
        for( int column=0; column < NUM_ROWS; column++ )
        {
            D3DXVECTOR3 vertices[] = 
            {
                D3DXVECTOR3( -0.5f, -0.5f, 0 ),
                D3DXVECTOR3(  0.5f, -0.5f, 0 ),
                D3DXVECTOR3(  0.5f,  0.5f, 0 ),
                D3DXVECTOR3( -0.5f,  0.5f, 0 )
            };

            // See where this cell is on the screen
            CalculateDiscWorldMatrix( &world, row, column );
        
            for( int i=0; i < 4; i++ )
            {
                D3DXVec3Project( &vertices[i], &vertices[i], &viewport, &projection, &view, &world );
            }

            if( D3DXIntersectTri( &vertices[0], &vertices[1], &vertices[2], &point, &direction, NULL, NULL, NULL ) ||
                D3DXIntersectTri( &vertices[2], &vertices[3], &vertices[0], &point, &direction, NULL, NULL, NULL ) )
                return Cell( row, column );
        }
    }
   
    // Not found
    return Cell( -1, -1 );
}


//--------------------------------------------------------------------------------------
bool CBoard::IsValidCellAddress( int row, int column )
{
    return (row >= 0 && row < NUM_ROWS &&
            column >= 0 && column < NUM_ROWS );
}


//--------------------------------------------------------------------------------------
int CBoard::NumberCapturedInDirection( Color disc, int row, int column, int incRow, int incColumn )
{
    int capturedDiscs = 0;
    for(;;)
    {
        row += incRow;
        column += incColumn;

        // Reaching the edge of the board or an None cell before reaching a similar color indicates no captures
        if( !IsValidCellAddress( row, column ) )
            return 0;

        if( Discs[row][column] == None )
            return 0;

        // When reaching the same color as a captured disc, return the number captured
        if( Discs[row][column] == disc )
            return capturedDiscs;

        // Else it's a capture
        capturedDiscs++;
    }
}


//--------------------------------------------------------------------------------------
int CBoard::NumberCaptured( Color disc, int row, int column )
{
    if( disc != White && disc != Black )
        return 0;

    if( !IsValidCellAddress( row, column ) )
        return 0;

    int capturedDiscs = 0;
    for( int x=-1; x <= 1; x++ )
    {
        for( int y=-1; y <= 1; y++ )
        {
            if( x==0 && y==0 )
                continue;

            capturedDiscs += NumberCapturedInDirection( disc, row, column, y, x );
        }
    }

    return capturedDiscs;
}


//--------------------------------------------------------------------------------------
bool CBoard::IsLegalMove( Color disc, int row, int column )
{
    if( disc != White && disc != Black )
        return false;

    if( !IsValidCellAddress( row, column ) )
        return false;

    if( Discs[row][column] != None )
        return false;

    return ( 0 != NumberCaptured( disc, row, column ) );
}


//--------------------------------------------------------------------------------------
bool CBoard::PerformMove( Color disc, int row, int column )
{
    if( !IsLegalMove( disc, row, column ) )
        return false;

    //debug
    RippleRow = row;
    RippleColumn = column;
    RippleScale = 1.0f;
    RippleAlpha = 0.3f;

    if( disc == White )
    {
        float colorMultiplier[] = {1, 1, 1};
        RippleQuad.pEffect->SetFloatArray( "ColorMultiplier", colorMultiplier, 3 );
    }
    else
    {
        float colorMultiplier[] = {0, 0, 0};
        RippleQuad.pEffect->SetFloatArray( "ColorMultiplier", colorMultiplier, 3 );
    }
    

    Discs[row][column] = disc;
    CellAnimations[row][column].EndRotation = (disc == White) ? D3DX_PI/2 : 3*D3DX_PI/2;

    for( int x=-1; x <= 1; x++ )
    {
        for( int y=-1; y <= 1; y++ )
        {
            if( x==0 && y==0 )
                continue;

            // Verify there are cells to capture in this direction
            if( 0 == NumberCapturedInDirection( disc, row, column, y, x ) )
                continue;

            int captureX = column;
            int captureY = row;
            
            for(;;)
            {
                captureX += x;
                captureY += y;

                // Break when you hit your own color
                if( Discs[captureY][captureX] == disc )
                    break;

                Discs[captureY][captureX] = disc;

                AnimationEvent& cellAnimation = CellAnimations[captureY][captureX];

                float distance = sqrtf( powf( (float)(row - captureY), 2 ) + powf( (float)(column - captureX), 2 ) );
                float startDelay = 0.2f * distance;
                cellAnimation.StartTime = (float)DXUTGetTime() + startDelay;
                cellAnimation.EndTime = cellAnimation.StartTime + 1.5f;
                cellAnimation.StartRotation = cellAnimation.CurrentRotation;
                cellAnimation.EndRotation = (disc == White) ? D3DX_PI/2 : 3 * D3DX_PI/2;
            }
        }
    }

    IncrementBoardState();

    return true;
}


//--------------------------------------------------------------------------------------
void CBoard::IncrementBoardState()
{
    // See who moves next
    ActiveColor = (ActiveColor == White) ? Black : White;

    // Check to see if they have any legal moves. If not, see of the other player has
    // a legal move
    if( !LegalMoveExists(ActiveColor) )
    {
        // If this player doesn't have any legal moves either the game is over
        ActiveColor = (ActiveColor == White) ? Black : White;   
        if( !LegalMoveExists(ActiveColor) )
        {
            ActiveColor = None;
            m_IsFinished = true;
        }
    }

    AnimateBoard();
}


//--------------------------------------------------------------------------------------
bool CBoard::LegalMoveExists( Color color )
{
    for( int row=0; row < NUM_ROWS; row++ )
    {
        for( int column=0; column < NUM_ROWS; column++ )
        {
            if(IsLegalMove( color, row, column ))
                return true;
        }
    }

    return false;
}


//--------------------------------------------------------------------------------------
int CBoard::GetScore( Color color )
{
    int score = 0;
    for( int row=0; row < NUM_ROWS; row++ )
    {
        for( int column=0; column < NUM_ROWS; column++ )
        {
            if( Discs[row][column] == color )
                score++;
        }
    }

    return score;
}