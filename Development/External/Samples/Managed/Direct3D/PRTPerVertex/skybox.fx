//-----------------------------------------------------------------------------
// File: SkyBox.fx
//
// Desc: 
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
float4x4 invertedWorldViewProjection;
float alpha;
float scale;

texture environmentTexture;

sampler EnvironmentSampler = sampler_state
{ 
    Texture = (environmentTexture);
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = POINT;
};


//-----------------------------------------------------------------------------
// Skybox stuff
//-----------------------------------------------------------------------------
struct SkyboxVS_Input
{
    float4 Pos : POSITION;
};

struct SkyboxVS_Output
{
    float4 Pos : POSITION;
    float3 Tex : TEXCOORD0;
};

SkyboxVS_Output SkyboxVS( SkyboxVS_Input Input )
{
    SkyboxVS_Output Output;
    
    Output.Pos = Input.Pos;
    Output.Tex = normalize( mul(Input.Pos, invertedWorldViewProjection) );
    
    return Output;
}

float4 SkyboxPS( SkyboxVS_Output Input ) : COLOR
{
    float4 color = texCUBE( EnvironmentSampler, Input.Tex )*scale;
    color.a = alpha;
    return color;
}

technique Skybox
{
    pass p0
    {
        VertexShader = compile vs_2_0 SkyboxVS();
        PixelShader = compile ps_2_0 SkyboxPS();
    }
}




