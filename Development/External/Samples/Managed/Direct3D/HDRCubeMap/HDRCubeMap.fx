//-----------------------------------------------------------------------------
// File: HDRCubeMap.fx
//
// Desc: Effect file for high dynamic range cube mapping sample.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef MAX_NUM_LIGHTS
#define MAX_NUM_LIGHTS 4
#endif


float4x4 worldViewMatrix;
float4x4 projMatrix;
texture  cubeMapTexture;
texture  cubeMapTexture2;
texture  sceneTexture;
float4   lightIntensityVector = { 15.0f, 15.0f, 15.0f, 1.0f };
float4   lightPositionView[MAX_NUM_LIGHTS];  // Light positions in view space
float    reflectivity;                  // Reflectivity value


//-----------------------------------------------------------------------------
// Sampler: samCubeMap
// Desc: Process vertex for HDR environment mapping
//-----------------------------------------------------------------------------
samplerCUBE g_samCubeMap = 
sampler_state
{
    Texture = <cubeMapTexture>;
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
};


samplerCUBE g_samCubeMap2 = 
sampler_state
{
    Texture = <cubeMapTexture2>;
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
};


sampler2D g_samScene =
sampler_state
{
    Texture = <sceneTexture>;
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
};


//-----------------------------------------------------------------------------
// Vertex Shader: HDRVertEnvMap
// Desc: Process vertex for HDR environment mapping
//-----------------------------------------------------------------------------
void HDRVertEnvMap( float4 Pos : POSITION,
                    float3 Normal : NORMAL,
                    out float4 oPos : POSITION,
                    out float3 EnvTex : TEXCOORD0 )
{
    oPos = mul( Pos, worldViewMatrix );

    //
    // Compute normal in camera space
    //
    float3 vN = mul( Normal, worldViewMatrix );
    vN = normalize( vN );

    //
    // Obtain the reverse eye vector
    //
    float3 vEyeR = -normalize( oPos );

    //
    // Compute the reflection vector
    //
    float3 vRef = 2 * dot( vEyeR, vN ) * vN - vEyeR;

    //
    // Store the reflection vector in texcoord1
    //
    EnvTex = vRef;

    //
    // Apply the projection
    //
    oPos = mul( oPos, projMatrix );
}


//-----------------------------------------------------------------------------
// Pixel Shader: HDRPixEnvMap
// Desc: Process pixel for HDR environment mapped object
//-----------------------------------------------------------------------------
float4 HDRPixEnvMap( float3 Tex : TEXCOORD0 ) : COLOR
{
    return reflectivity * texCUBE( g_samCubeMap, Tex );
}


float4 HDRPixEnvMap2Tex( float3 Tex : TEXCOORD0 ) : COLOR
{
    return reflectivity * float4( texCUBE( g_samCubeMap, Tex ).xy, texCUBE( g_samCubeMap2, Tex ).xy );
}


//-----------------------------------------------------------------------------
// Vertex Shader: HDRVertScene
// Desc: Process vertex for HDR-enabled scene
//-----------------------------------------------------------------------------
void HDRVertScene( float4 iPos : POSITION,
                   float3 iNormal : NORMAL,
                   float2 iTex : TEXCOORD0,
                   out float4 oPos : POSITION,
                   out float2 Tex : TEXCOORD0,
                   out float3 Pos : TEXCOORD1,
                   out float3 Normal : TEXCOORD2 )
{
    //
    // Transform position to view space
    //
    oPos = mul( iPos, worldViewMatrix );

    //
    // Also write view position to texcoord1 to do per-pixel lighting
    //
    Pos = oPos;

    //
    // Transform to screen coord
    //
    oPos = mul( oPos, projMatrix );

    //
    // Transform normal and write to texcoord2 for per-pixel lighting
    //
    Normal = normalize( mul( iNormal, (float3x3)worldViewMatrix ) );
    
    //
    // Propagate texture coord
    //
    Tex = iTex;
}


//-----------------------------------------------------------------------------
// Pixel Shader: HDRPixScene
// Desc: Process pixel (do per-pixel lighting) for HDR-enabled scene
//-----------------------------------------------------------------------------
float4 HDRPixScene( float2 Tex : TEXCOORD0,
                    float3 Pos : TEXCOORD1,
                    float3 Normal : TEXCOORD2 ) : COLOR
{
    float3 N = normalize( Normal );

    // Variable to save lit value by each light
    float4 vPixValue = (float4)0;

    //
    // Iterate through each light and apply the light on the pixel
    //
    for( int LightIndex = 0; LightIndex < MAX_NUM_LIGHTS; ++LightIndex )
    {
        //
        // Compute light vector (pixel to light)
        //
        float3 vRLightVec = (float3)(lightPositionView[LightIndex] - Pos);

        //
        // Find out the light intensity at the vertex based on
        // N dot L and distance from the light.
        //
        float fDiffuse = saturate( dot( normalize( vRLightVec ), N ) );

        //
        // Attenuation is 1 / D^2. Clamp at 1 to avoid infinity.
        //
        float fAttenuation = saturate( 1.0f / dot( vRLightVec, vRLightVec ) );

        //
        // Compute and add pixel color to final value
        //
        vPixValue += fDiffuse * fAttenuation;
    }

    return tex2D( g_samScene, Tex ) * vPixValue * lightIntensityVector;
}


float4 HDRPixScene_FirstHalf( float2 Tex : TEXCOORD0,
                              float3 Pos : TEXCOORD1,
                              float3 Normal : TEXCOORD2 ) : COLOR
{
    return HDRPixScene( Tex, Pos, Normal ).xyzw;
}


float4 HDRPixScene_SecondHalf( float2 Tex : TEXCOORD0,
                               float3 Pos : TEXCOORD1,
                               float3 Normal : TEXCOORD2 ) : COLOR
{
    return HDRPixScene( Tex, Pos, Normal ).zwww;
}


//-----------------------------------------------------------------------------
// Vertex Shader: HDRVertLight
// Desc: Process vertex for light objects
//-----------------------------------------------------------------------------
void HDRVertLight( float4 iPos : POSITION,
                   out float4 oPos : POSITION,
                   out float4 Diffuse : TEXCOORD1 )
{
    //
    // Transform position to screen space
    //
    oPos = mul( iPos, worldViewMatrix );
    oPos = mul( oPos, projMatrix );

    //
    // Diffuse color is the light intensity value
    //
    Diffuse = lightIntensityVector;
}


//-----------------------------------------------------------------------------
// Pixel Shader: HDRPixLight
// Desc: Process pixel for HDR-enabled scene
//-----------------------------------------------------------------------------
float4 HDRPixLight( float4 Diffuse : TEXCOORD1 ) : COLOR
{
    //
    // Diffuse has the full intensity of the light.
    // Just output it.
    //
    return Diffuse;
}


float4 HDRPixLight_FirstHalf( float4 Diffuse : TEXCOORD1 ) : COLOR
{
    //
    // Diffuse has the full intensity of the light.
    // Just output it.
    //
    return Diffuse.xyww;
}


float4 HDRPixLight_SecondHalf( float4 Diffuse : TEXCOORD1 ) : COLOR
{
    //
    // Diffuse has the full intensity of the light.
    // Just output it.
    //
    return Diffuse.zwww;
}


//-----------------------------------------------------------------------------
// Technique: RenderScene
// Desc: Renders scene objects
//-----------------------------------------------------------------------------
technique RenderScene
{
    pass p0
    {
        VertexShader = compile vs_1_1 HDRVertScene();
        PixelShader = compile ps_2_0 HDRPixScene();
    }
}


technique RenderSceneFirstHalf
{
    pass p0
    {
        VertexShader = compile vs_1_1 HDRVertScene();
        PixelShader = compile ps_2_0 HDRPixScene_FirstHalf();
    }
}


technique RenderSceneSecondHalf
{
    pass p0
    {
        VertexShader = compile vs_1_1 HDRVertScene();
        PixelShader = compile ps_2_0 HDRPixScene_SecondHalf();
    }
}


//-----------------------------------------------------------------------------
// Technique: RenderLight
// Desc: Renders light objects
//-----------------------------------------------------------------------------
technique RenderLight
{
    pass p0
    {
        VertexShader = compile vs_1_1 HDRVertLight();
        PixelShader = compile ps_2_0 HDRPixLight();
    }
}


technique RenderLightFirstHalf
{
    pass p0
    {
        VertexShader = compile vs_1_1 HDRVertLight();
        PixelShader = compile ps_2_0 HDRPixLight_FirstHalf();
    }
}


technique RenderLightSecondHalf
{
    pass p0
    {
        VertexShader = compile vs_1_1 HDRVertLight();
        PixelShader = compile ps_2_0 HDRPixLight_SecondHalf();
    }
}


//-----------------------------------------------------------------------------
// Technique: RenderEnvMesh
// Desc: Renders the HDR environment-mapped mesh
//-----------------------------------------------------------------------------
technique RenderHDREnvMap
{
    pass p0
    {
        VertexShader = compile vs_1_1 HDRVertEnvMap();
        PixelShader = compile ps_2_0 HDRPixEnvMap();
    }
}


technique RenderHDREnvMap2Tex
{
    pass p0
    {
        VertexShader = compile vs_1_1 HDRVertEnvMap();
        PixelShader = compile ps_2_0 HDRPixEnvMap2Tex();
    }
}
