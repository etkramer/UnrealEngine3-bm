/*=============================================================================
	UnXenonSplash.cpp: Splash screen implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Core includes.
#include "CorePrivate.h"
#include "XeD3DDrvPrivate.h"

//--------------------------------------------------------------------------------------
// Vertex and pixel shaders for rendering the help screen
//--------------------------------------------------------------------------------------
static const CHAR* SplashShaderString =
" struct VS_IN                                  "
" {                                             "
"     float2 Pos            : POSITION;         "
"     float2 Tex            : TEXCOORD0;        "
" };                                            "
"                                               "
" struct VS_OUT                                 "
" {                                             "
"     float4 Position       : POSITION;         "
"     float2 TexCoord0      : TEXCOORD0;        "
" };                                            "
"                                               "
" VS_OUT SplashVertexShader( VS_IN In )         "
" {                                             "
"     VS_OUT Out;                               "
"     Out.Position.x  = In.Pos.x/1.0 - 0.0;     "
"     Out.Position.y  = In.Pos.y/1.0 - 0.0;     "
"     Out.Position.z  = 0.0;                    "
"     Out.Position.w  = 1.0;                    "
"     Out.TexCoord0.x = In.Tex.x;               "
"     Out.TexCoord0.y = In.Tex.y;               "
"     return Out;                               "
" }                                             "
"                                               "
"sampler Texture : register(s0);                "
"												"
"float4 SplashPixelShader( VS_OUT In ) : COLOR0		"
"{													"
"    return pow( tex2D(Texture,In.TexCoord0),2.2 );	"
"}													";


static D3DVertexDeclaration* SplashVertexDecl   = NULL;
static D3DVertexShader*      SplashVertexShader = NULL;
static D3DPixelShader*       SplashPixelShader  = NULL;

void appXenonInitSplash()
{
#if USE_XeD3D_RHI
	// Create vertex declaration
	D3DVERTEXELEMENT9 Declaration[] = 
	{
		{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
	verify( SUCCEEDED( GDirect3DDevice->CreateVertexDeclaration( Declaration, &SplashVertexDecl ) ) );

	// Create vertex shader
	ID3DXBuffer* ShaderCode;
	verify( SUCCEEDED( D3DXCompileShader( SplashShaderString, strlen(SplashShaderString), NULL, NULL, "SplashVertexShader", "vs.2.0", 0, &ShaderCode, NULL, NULL ) ) );
	verify( SUCCEEDED( GDirect3DDevice->CreateVertexShader( (DWORD*)ShaderCode->GetBufferPointer(), &SplashVertexShader ) ) );
	ShaderCode->Release();
	
	// Create pixel shader.
	verify( SUCCEEDED( D3DXCompileShader( SplashShaderString, strlen(SplashShaderString), NULL, NULL, "SplashPixelShader", "ps.2.0", 0, &ShaderCode, NULL, NULL ) ) );
	verify( SUCCEEDED( GDirect3DDevice->CreatePixelShader( (DWORD*)ShaderCode->GetBufferPointer(), &SplashPixelShader ) ) );
	ShaderCode->Release();
#endif  // USE_XeD3D_RHI
}

/**
 * Shows splash screen and optionally persists display so we can reboot.
 *
 * @param	SplashName	path to file containing splash screen
 * @param	bPersist	whether to persist display
 */
void appXenonShowSplash( const TCHAR* SplashName, UBOOL bPersist )
{
	FString SplashPath;
	// make sure the splash was found
	if (appGetSplashPath(SplashName, SplashPath) == FALSE)
	{
		return;
	}

#if USE_XeD3D_RHI
	// Load a texture into memory and create a texture from there.
	TArray<BYTE>		Data;
	IDirect3DTexture9*	Texture;
	if( appLoadFileToArray( Data, *SplashPath, GFileManager ) )
	{
		verify( SUCCEEDED( D3DXCreateTextureFromFileInMemory( GDirect3DDevice, &Data(0), Data.Num(), &Texture) ) );

		// Setup the splash screen vertices
		struct VERTEX
		{ 
			FLOAT sx, sy;
			FLOAT tu, tv;
		};
		VERTEX SplashVertices[4] = 
		{
			{ -1.0f,  1.0f,   0.0f, 0.0f },
			{  1.0f,  1.0f,   1.0f, 0.0f },
			{  1.0f, -1.0f,   1.0f, 1.0f },
			{ -1.0f, -1.0f,   0.0f, 1.0f },
		};

		// Set texture coordinates
		XGTEXTURE_DESC TextureDesc;
		XGGetTextureDesc( Texture, 0, &TextureDesc );
		SplashVertices[0].tu = 0.0f;   SplashVertices[0].tv = 0.0f;
		SplashVertices[1].tu = 1.0f;   SplashVertices[1].tv = 0.0f;
		SplashVertices[2].tu = 1.0f;   SplashVertices[2].tv = 1.0f;
		SplashVertices[3].tu = 0.0f;   SplashVertices[3].tv = 1.0f;

		// Set up state for rendering the splash screen.
		GDirect3DDevice->SetVertexDeclaration( SplashVertexDecl );
		GDirect3DDevice->SetVertexShader( SplashVertexShader );
		GDirect3DDevice->SetTexture( 0, Texture );
		GDirect3DDevice->SetPixelShader( SplashPixelShader );
		GDirect3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
		GDirect3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
		GDirect3DDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
		GDirect3DDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		GDirect3DDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
		GDirect3DDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
		GDirect3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
		GDirect3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		GDirect3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		GDirect3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

		// Draw the splash image
		GDirect3DDevice->DrawPrimitiveUP( D3DPT_QUADLIST, 1, SplashVertices, sizeof(SplashVertices[0]) );

		// Render the system UI and perform buffer swap.
		XePerformSwap( TRUE );

		// Make sure texture gets freed.
		GDirect3DDevice->SetTexture( 0, NULL );
		Texture->Release();
		Texture = NULL;

		// Keep the screen up even through reboots
		if( bPersist )
		{
			GDirect3DDevice->PersistDisplay( GD3DFrontBuffer, NULL );
		}
	}
#endif  // USE_XeD3D_RHI
}

