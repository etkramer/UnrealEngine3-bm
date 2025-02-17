//--------------------------------------------------------------------------------------
// File: CustomUI.fx
//
// The effect file for the CustomUI sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
float4x4 worldMatrix;					// World matrix for object
float4x4 worldViewProjection;	// World * View * Projection matrix
texture  sceneTexture;

sampler  g_samScene =
sampler_state
{
    Texture = <sceneTexture>;
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Point;
};


void VertScene( float4 Pos : POSITION,
                float2 Tex : TEXCOORD0,
                out float4 oPos : POSITION,
                out float2 oTex : TEXCOORD0 )
{
    oPos = mul( Pos, worldViewProjection );
    oTex = Tex;
}


float4 PixScene( float2 Tex : TEXCOORD0 ) : COLOR0
{
    return tex2D( g_samScene, Tex );
}


//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique RenderScene
{
    pass P0
    {
        VertexShader = compile vs_1_1 VertScene();
        PixelShader = compile ps_1_1 PixScene();
    }
}
