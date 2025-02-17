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

float3 LightDirection = {0, -1, -1};
float3 SpecularLightDirection = {0, 1, 0.5};
//float3 LightDirection = {0, 0, -1};

//float3 BoardColor = {95/255.0, 173/255.0, 247/255.0};
float3 BoardColor = {255/255.0, 255/255.0, 255/255.0};
float BoardAlpha = 1.0f;

texture BoardTexture;
textureCUBE EnvironmentMap;

sampler BoardTextureSampler = sampler_state
{ 
    Texture = (BoardTexture);
    MinFilter = Linear;
    MagFilter = Linear;
};

sampler EnvironmentMapSampler = sampler_state
{ 
    Texture = (EnvironmentMap);
    MinFilter = Linear;
    MagFilter = Linear;
};


//-----------------------------------------------------------------------------------------
// VertexShader I/O
//-----------------------------------------------------------------------------------------
struct VSInput
{
    float4 Position : POSITION; 
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR;
};

struct VSOutput
{
    float4 TransformedPosition : POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : TEXCOORD1;
    float4 Color : COLOR;
};


//-----------------------------------------------------------------------------------------
// Vertex Shader
//-----------------------------------------------------------------------------------------
VSOutput VS( VSInput input )
{
    VSOutput output;
   
    // Transform to clip space by multiplying by the basic transform matrices.
    output.TransformedPosition = mul(input.Position, mul(World, mul(View, Projection)));
    
    // Move the incoming normal into world space for pixel shader lighting calculations
    output.Normal = mul(input.Normal, (float3x3)World);
    output.Color = input.Color;
    output.TexCoord = input.TexCoord;
        
    return output;    
}

//-----------------------------------------------------------------------------------------
float3 ClampToNTSC( float3 color )
{
    color = saturate(color);
    //return (219.0f/255.0f * color) + 16.0f/255.0f;
    return max( 16/255.0f, min( 235/255.0f, color ) );
}


//-----------------------------------------------------------------------------------------
// Pixel Shader
//-----------------------------------------------------------------------------------------
float4 PSBoard( VSOutput input ) : COLOR
{ 
    float4 color = tex2D(BoardTextureSampler, input.TexCoord);
	
	color.rgb *= BoardColor;
	color.a *= BoardAlpha;
	
    color.rgb = ClampToNTSC( color.rgb );
    return color;
}


//-----------------------------------------------------------------------------------------
// Pixel Shader
//-----------------------------------------------------------------------------------------
float4 PSDisc( VSOutput input ) : COLOR
{ 
    float3 color = input.Color;
    
    //debug
    LightDirection = normalize(LightDirection);
    
	// Normalize the normal
    float3 normal = normalize(input.Normal);
   
    // Start with N dot L lighting
    float light = saturate( dot(normal, -LightDirection) );
    
    // Add environment mapping
    float3 reflection = reflect( -View[3], normal );
    float3 environment = 0.1f * texCUBE(EnvironmentMapSampler, reflection);
    
    // Specular highlight
    float specular = saturate(dot(reflection, -SpecularLightDirection));
    specular = 0.1f * pow(specular, 16);
    
    //return float4( specular, specular, specular, 1.0f );
    
    color *= light;
    color += environment;
    color += specular;
    
    color = ClampToNTSC( color );
    return float4( color, 1.0f );
}


//-----------------------------------------------------------------------------------------
// Techniques
//-----------------------------------------------------------------------------------------
technique Board
{
    pass P0
    {   
        AlphaBlendEnable = true;
        SrcBlend = SrcAlpha;
        DestBlend = InvSrcAlpha;
        ZWriteEnable = false;
        
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PSBoard(); 
    }
}

technique Disc
{
    pass P0
    {   
        //AlphaBlendEnable = true;
        //SrcBlend = SrcAlpha;
        //DestBlend = InvSrcAlpha;
        
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PSDisc(); 
    }
}


