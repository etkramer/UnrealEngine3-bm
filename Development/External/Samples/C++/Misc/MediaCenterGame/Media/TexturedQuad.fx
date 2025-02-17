//-----------------------------------------------------------------------------------------
// TexturedQuad.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------------------
float4x4 World;         
float4x4 View;
float4x4 Projection;

float ColorMultiplier = 1.0f;
float AlphaMultiplier = 1.0f;

texture QuadTexture;
sampler QuadTextureSampler = sampler_state
{ 
    Texture = (QuadTexture);
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
    return max( 16/255.0f, min( 235/255.0f, saturate(color) ) );
}


//-----------------------------------------------------------------------------------------
// Pixel Shader
//-----------------------------------------------------------------------------------------
float4 PS( VSOutput input ) : COLOR
{ 
    float4 color = tex2D(QuadTextureSampler, input.TexCoord);
   
	color.rgb *= ColorMultiplier;
    color.a *= AlphaMultiplier;
    
    color.rgb = ClampToNTSC( color.rgb );
    return color;
}



//-----------------------------------------------------------------------------------------
// Techniques
//-----------------------------------------------------------------------------------------
technique TexturedQuad
{
    pass P0
    {   
		AlphaBlendEnable = true;
        SrcBlend = SrcAlpha;
        DestBlend = InvSrcAlpha;
        
        ZWriteEnable = false;
        
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS(); 
    }
}
