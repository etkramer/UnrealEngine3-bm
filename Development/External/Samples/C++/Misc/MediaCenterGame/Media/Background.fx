//-----------------------------------------------------------------------------------------
// Board.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------------------
float4x4 World;         
float4x4 View;
float4x4 Projection;

texture BackgroundTexture;
sampler BackgroundTextureSampler = sampler_state
{ 
    Texture = (BackgroundTexture);
    MinFilter = Linear;
    MagFilter = Linear;
};


//-----------------------------------------------------------------------------------------
// VertexShader I/O
//-----------------------------------------------------------------------------------------
struct VSInput
{
    float4 Position : POSITION; 
    float2 TexCoord : TEXCOORD0;
};

struct VSOutput
{
    float4 TransformedPosition : POSITION;
    float2 TexCoord : TEXCOORD;
};


//-----------------------------------------------------------------------------------------
// Vertex Shader
//-----------------------------------------------------------------------------------------
VSOutput VS( VSInput input )
{
    VSOutput output;
   
    output.TransformedPosition = mul(input.Position, mul(World, mul(View, Projection)));
    output.TexCoord = input.TexCoord;
        
    return output;    
}


//-----------------------------------------------------------------------------------------
float3 ClampToNTSC( float3 color )
{
    color = saturate(color);
    return (219.0f/255.0f * color) + 16.0f/255.0f;
}


//-----------------------------------------------------------------------------------------
// Pixel Shader
//-----------------------------------------------------------------------------------------
float4 PS( VSOutput input ) : COLOR
{ 
    float4 color = tex2D(BackgroundTextureSampler, input.TexCoord);
   
    color.rgb = ClampToNTSC( color.rgb );
    return color;
}



//-----------------------------------------------------------------------------------------
// Techniques
//-----------------------------------------------------------------------------------------
technique Background
{
    pass P0
    {   
        ZWriteEnable = false;
        
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS(); 
    }
}
