//--------------------------------------------------------------------------------------
// File: FragmentLinker.fx
//
// The effect file for the FragmentLinker sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
float4 g_vMaterialAmbient = float4( 0.3f, 0.3f, 0.3f, 1.0f );       // Material's ambient color
float4 g_vMaterialDiffuse = float4( 0.6f, 0.6f, 0.6f, 1.0f );       // Material's diffuse color

float4   g_vLightColor = float4( 1.0f, 1.0f, 1.0f, 1.0f );        // Light color
float3   g_vLightPosition = float3( 0.0f, 5.0f, -5.0f );          // Light position

texture  g_MeshTexture;             // Color texture for mesh

float	 g_fTime;					// App's time in seconds
float4x4 g_mWorld;                  // World matrix
float4x4 g_mWorldViewProjection;	// World * View * Projection matrix


//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------
sampler MeshTextureSampler:s0;

//--------------------------------------------------------------------------------------
// Name: Projection
// Type: Vertex Shader Fragment
// Desc: Projection transform
//--------------------------------------------------------------------------------------
void Projection( float4 vPosObject: POSITION,
                 float3 vNormalObject: NORMAL,
                 float2 vTexCoordIn: TEXCOORD0,
                 out float4 vPosWorld: r_PosWorld,
                 out float3 vNormalWorld: r_NormalWorld,
                 out float4 vPosProj: POSITION,
                 out float2 vTexCoordOut: TEXCOORD0,
                 uniform bool bAnimate
               )
{
    // Optional vertex animation
    if( bAnimate )
        vPosObject.x *= (1 + sin( g_fTime )/2);
        
    // Transform the position into world space for lighting, and projected space
    // for display
    vPosWorld = mul( vPosObject, g_mWorld );
    vPosProj = mul( vPosObject, g_mWorldViewProjection );
    
    // Transform the normal into world space for lighting
    vNormalWorld = mul( vNormalObject, (float3x3)g_mWorld );
    
    // Pass the texture coordinate
    vTexCoordOut = vTexCoordIn;
}
vertexfragment ProjectionFragment_Animated = compile_fragment vs_2_0 Projection( true );
vertexfragment ProjectionFragment_Static = compile_fragment vs_2_0 Projection( false );


//--------------------------------------------------------------------------------------
// Name: Ambient
// Type: Vertex Shader Fragment
// Desc: Ambient scene lighting
//--------------------------------------------------------------------------------------
void Ambient( out float4 vColor: COLOR0 )
{
    // Compute the ambient component of illumination
    vColor = g_vLightColor * g_vMaterialAmbient;
}
vertexfragment AmbientFragment = compile_fragment vs_2_0 Ambient();


//--------------------------------------------------------------------------------------
// Name: AmbientDiffuse
// Type: Vertex Shader Fragment
// Desc: Ambient scene lighting
//--------------------------------------------------------------------------------------
void AmbientDiffuse( float3 vPosWorld: r_PosWorld,
                     float3 vNormalWorld: r_NormalWorld,
                     out float4 vColor: COLOR0 )
{  
    // Compute the light vector
    float3 vLight = normalize( g_vLightPosition - vPosWorld );
    
    // Compute the ambient and diffuse components of illumination
    vColor = g_vLightColor * g_vMaterialAmbient;
    vColor += g_vLightColor * g_vMaterialDiffuse * saturate( dot( vLight, vNormalWorld ) );
}
vertexfragment AmbientDiffuseFragment = compile_fragment vs_2_0 AmbientDiffuse();


//--------------------------------------------------------------------------------------
// Name: ModulateTexture
// Type: Pixel Shader
// Desc: Multiply the interpolated vertex color by the texture
//--------------------------------------------------------------------------------------
void ModulateTexture( float4 vColorIn: COLOR0,
                      float2 vTexCoord: TEXCOORD0,
                      out float4 vColorOut: COLOR0 )
{  
    // Sample and modulate the texture
    vColorOut = vColorIn * tex2D( MeshTextureSampler, vTexCoord );
}
