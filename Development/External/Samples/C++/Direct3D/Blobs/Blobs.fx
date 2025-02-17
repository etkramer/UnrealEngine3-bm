//-----------------------------------------------------------------------------
// File: Blobs.fx
//
// Desc: Effect file for the Blobs sample. 
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
static const float GAUSSIANTEXSIZE = 64;
static const float TEMPTEXSIZE = 1024;
static const float THRESHOLD = 0.08f;

float4x4 g_mWorldViewProjection;

// Textures
texture g_tSourceBlob;
texture g_tNormalBuffer;
texture g_tColorBuffer;
texture g_tEnvMap;


//-----------------------------------------------------------------------------
// Samplers
//-----------------------------------------------------------------------------
sampler SourceBlobSampler = 
sampler_state
{
    Texture = <g_tSourceBlob>;
    MinFilter = Point;
    MagFilter = Point;
    MipFilter = Point;

    AddressU = Clamp;
    AddressV = Clamp;
};

sampler NormalBufferSampler = 
sampler_state
{
    Texture = <g_tNormalBuffer>;
    MinFilter = Point;
    MagFilter = Point;
    MipFilter = Point;

    AddressU = Clamp;
    AddressV = Clamp;
};

sampler ColorBufferSampler = 
sampler_state
{
    Texture = <g_tColorBuffer>;
    MinFilter = Point;
    MagFilter = Point;
    MipFilter = Point;

    AddressU = Clamp;
    AddressV = Clamp;
};

sampler EnvMapSampler =
sampler_state
{
    Texture = <g_tEnvMap>;
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;

    AddressU = Clamp;
    AddressV = Clamp;
};

//-----------------------------------------------------------------------------
// Vertex/pixel shader output structures
//-----------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 vPosition : POSITION;
    float2 tCurr     : TEXCOORD0;
    float2 tBack     : TEXCOORD1;
    float  fSize     : TEXCOORD2;
    float3 vColor    : TEXCOORD3;
};


struct PS_OUTPUT
{
    float4 vColor[2] : COLOR;
};




//-----------------------------------------------------------------------------
// Name: DoLerp
// Type: Helper function                                      
// Desc: Peform a linear interpolation
//-----------------------------------------------------------------------------
void DoLerp( in float2 tCurr, out float4 outval )
{
    // Scale out into pixel space
    float2 pixelpos = GAUSSIANTEXSIZE * tCurr;

    // Determine the lerp amounts
    float2 lerps = frac( pixelpos );

    // Get the upper left position
    float3 lerppos = float3((pixelpos-(lerps/GAUSSIANTEXSIZE))/GAUSSIANTEXSIZE,0);

    float4 sourcevals[4];
    sourcevals[0] = tex2D( SourceBlobSampler, lerppos );  
    sourcevals[1] = tex2D( SourceBlobSampler, lerppos + float3(1.0/GAUSSIANTEXSIZE, 0,0) );
    sourcevals[2] = tex2D( SourceBlobSampler, lerppos + float3(0, 1.0/GAUSSIANTEXSIZE,0) );
    sourcevals[3] = tex2D( SourceBlobSampler, lerppos + float3(1.0/GAUSSIANTEXSIZE, 1.0/GAUSSIANTEXSIZE,0) );

    // Bilinear filtering
    outval = lerp( lerp( sourcevals[0], sourcevals[1], lerps.x ),
                   lerp( sourcevals[2], sourcevals[3], lerps.x ),
                   lerps.y );
}


//-----------------------------------------------------------------------------
// Name: BlobBlenderPS
// Type: Pixel shader
// Desc: 
//-----------------------------------------------------------------------------
PS_OUTPUT BlobBlenderPS( VS_OUTPUT Input )
{ 
    PS_OUTPUT output;
    float4 weight;
   
    // Get the new blob weight
    DoLerp( Input.tCurr, weight );

    // Get the old data
    float4 oldNormalData = tex2D( NormalBufferSampler, Input.tBack );
    float4 oldColorData  = tex2D( ColorBufferSampler, Input.tBack );
    
    // Generate new surface data
    float4 newNormalData = float4((Input.tCurr.x-0.5) * Input.fSize,
                                (Input.tCurr.y-0.5) * Input.fSize,
                                0,
                                1);
    newNormalData *= weight.r;
    
    //generate new material properties
    float4 newColorData = float4(Input.vColor.r,
                               Input.vColor.g,
                               Input.vColor.b,
                               0);
    newColorData *= weight.r;
    
    // Additive blending
    output.vColor[0] = newNormalData + oldNormalData; 
    output.vColor[1] = newColorData + oldColorData;
    
    return output;
}


float4 BlobBlenderPSNormal( VS_OUTPUT Input ) : COLOR0
{ 
    float4 weight;

    // Get the new blob weight
    DoLerp( Input.tCurr, weight );

    // Get the old data
    float4 oldNormalData = tex2D( NormalBufferSampler, Input.tBack );

    // Generate new surface data
    float4 newNormalData = float4((Input.tCurr.x-0.5) * Input.fSize,
                                (Input.tCurr.y-0.5) * Input.fSize,
                                0,
                                1);
    newNormalData *= weight.r;

    // Additive blending
    return newNormalData + oldNormalData;
}


float4 BlobBlenderPSColor( VS_OUTPUT Input ) : COLOR0
{
    PS_OUTPUT output;
    float4 weight;

    // Get the new blob weight
    DoLerp( Input.tCurr, weight );

    // Get the old data
    float4 oldColorData  = tex2D( ColorBufferSampler, Input.tBack );

    //generate new material properties
    float4 newColorData = float4(Input.vColor.r,
                               Input.vColor.g,
                               Input.vColor.b,
                               0);
    newColorData *= weight.r;

    // Additive blending
    return newColorData + oldColorData;
}


//-----------------------------------------------------------------------------
// Name: BlobLightPS
// Type: Pixel shader
// Desc: 
//-----------------------------------------------------------------------------
float4 BlobLightPS( VS_OUTPUT Input ) : COLOR
{
    static const float aaval = THRESHOLD * 0.07f;

    float4 blobdata = tex2D( SourceBlobSampler, Input.tCurr);
    float4 color = tex2D( ColorBufferSampler, Input.tCurr);
    
    color /= blobdata.w;
    
    float3 surfacept = float3(blobdata.x/blobdata.w,
                              blobdata.y/blobdata.w, 
                              blobdata.w-THRESHOLD); 
    float3 thenorm = normalize(-surfacept);
    thenorm.z = -thenorm.z;

    float4 Output;  
    Output.rgb = color.rgb + texCUBE( EnvMapSampler, thenorm );
    Output.rgb *= saturate ((blobdata.a - THRESHOLD)/aaval);
    Output.a=1;
    
    return Output;
}




//-----------------------------------------------------------------------------
// Name: BlobBlend
// Type: Technique                                     
// Desc: 
//-----------------------------------------------------------------------------
technique BlobBlend
{
    pass P0
    {   
        PixelShader = compile ps_2_0 BlobBlenderPS();
    }
}


technique BlobBlendTwoPasses
{
    pass P0
    {
        PixelShader = compile ps_2_0 BlobBlenderPSNormal();
    }
    pass P1
    {
        PixelShader = compile ps_2_0 BlobBlenderPSColor();
    }
}


//-----------------------------------------------------------------------------
// Name: BlobLight
// Type: Technique                                     
// Desc: 
//-----------------------------------------------------------------------------
technique BlobLight
{
    pass P0
    {   
		PixelShader  = compile ps_2_0 BlobLightPS();
    }
}


