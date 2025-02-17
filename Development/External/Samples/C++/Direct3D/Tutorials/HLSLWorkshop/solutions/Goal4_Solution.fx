// High-Level Shader Language (HLSL) Workshop
// Goal 4: Procedural Texture
//
// This is a modified version of the HLSL Workshop presented at the Game 
// Developers Conference 2003. This workshop is intended to be a hands-on 
// introduction to HLSL by providing the student (that's you) with a 
// near-complete effect file with small tasks to be completed rather than 
// a start-from-scratch tutorial. This is meant to give you an opportunity
// to play with HLSL before starting a more rigorous study of syntax. Follow
// along with the instructions located in the documentation accompanying the 
// SDK. If you get stuck, check the completed effect file in the "solutions"
// folder. Have fun! 
//
// Copyright (c) Microsoft Corporation. All rights reserved.



//  Goal 4 - Procedural texture
//    A) Do a volume texture lookup
//    B) Generate a procedural texture
//   Extra Credit:  
//      Use if statements to create a volume checker board texture



string XFile = "misc\\skullocc.x";                // model
string Environment = "lobby\\lobbycube.dds";       // environment map

// light directions (view space)
float3 DirFromLight < string UIDirectional = "Light Direction"; > = {0.577, -0.577, 0.577};

// direction of light from sky (view space)
float3 DirFromSky < string UIDirectional = "Direction from Sky"; > = { 0.0f, -1.0f, 0.0f };            

// light intensity
float4 LightAmbientIntensity    = { 0.8f, 0.8f, 0.8f, 1.0f };    // ambient
float4 GroundColor              = { 0.1f, 0.4f, 0.0f, 1.0f };    // ground
float4 SkyColor                 = { 0.5f, 0.5f, 0.9f, 1.0f };    // sky
float4 LightDiffuseColor        = { 1.0f, 0.9f, 0.8f, 1.0f };    // diffuse
float4 LightSpecularColor       = { 1.0f, 1.0f, 1.0f, 1.0f };    // specular

// material reflectivity
float4 MaterialAmbientIntensity = { 0.5f, 0.5f, 0.5f, 1.0f };    // ambient
float4 MaterialDiffuseColor     = { 0.4f, 0.4f, 0.4f, 1.0f };    // diffuse
float4 MaterialSpecularColor    = { 0.2f, 0.2f, 0.2f, 1.0f };    // specular
int    MaterialSpecularPower    = 32;                            // power

// transformations
float4x3 World  : WORLD;
float4x3 View   : VIEW;
float4x4 ViewProjection : VIEWPROJECTION;
float3 CameraPos : CAMERAPOSITION;    

// use the TIME semantic to inform EffectEdit to fill in the
//   current time in seconds
float Time : TIME;

texture EnvironmentMap 
< 
    string type = "CUBE"; 
    string name = "lobbycube.dds"; 
>;

samplerCUBE EnvironmentSampler = sampler_state
{ 
    Texture = (EnvironmentMap);
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};


texture VolumeMap 
< 
    string type = "VOLUME"; 
    string function = "GenerateVolumeMap"; 
    int width = 32;
    int height = 32;
    int depth = 32; 
>;

sampler3D VolumeTexture =
sampler_state
{
    texture = (VolumeMap);
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    
    AddressU = WRAP;
    AddressV = WRAP;
};




//-----------------------------------------------------------------------------
// Name: ProceduralLookup
// Type: Helper function
// Desc: Samples the procedurally generated volume texture
//-----------------------------------------------------------------------------
float4 ProceduralLookup(float3 VolTexcoord)
{
    // Goal 4A: Do a volume texture lookup
    //
    //   Suggestions:
    //     Use the provided VolTexcoord and the tex3D intrinsic
    //       to do the texture lookup
    //     Unlike the cubemap lookup, it is easy to scale down
    //       the values of the volume map in the function that
    //       generates the volume instead of here, so no scale
    //       should be required.
    
    return tex3D(VolumeTexture, VolTexcoord);
}




//-----------------------------------------------------------------------------
// Name: GenerateVolumeMap
// Type: Helper function
// Desc: Procedurally generates a volume texture
//-----------------------------------------------------------------------------
float4 GenerateVolumeMap(float3 Pos : POSITION) : COLOR
{
    // Goal 4B: Generate a procedural texture
    //
    //   How is the function called:
    //     EffectEdit uses the D3DXFillTextureTX function to
    //       execute this function for every texel in the volume
    //       texture.  The height/width/depth and function name
    //       are all described in the texture definition.
    //     The input is the position of the texel in the 
    //       texture.  In this case it is a three dimentional
    //       coordinate that ranges from 0 to 1 on all axis
    //       x -> width, y -> height, z -> depth
    //     The return value is the color stored at the texel
    //       position specified by the input position.
    //   
    //   Suggestions:
    //     The noise intrinsic is a good start for a fun 
    //       procedural texture.  Even by itself it can give
    //       a nice grainy texture.  One thing to remember
    //       about the noise function is that it is a good
    //       idea to scale up the access to it, since the
    //       range of noise on inputs of 0 to 1 is not as 
    //       interesting as that of 0 to 500.  You get
    //       a more random looking set of values then.  
    
    return noise(Pos * 500) * 0.5f;
}




//-----------------------------------------------------------------------------
// Name: CubeMapLookup
// Type: Helper function
// Desc: Performs a cubemap texture sample
//-----------------------------------------------------------------------------
float4 CubeMapLookup(float3 CubeTexcoord)
{
    return texCUBE(EnvironmentSampler, CubeTexcoord);
}




//-----------------------------------------------------------------------------
// Name: CalcReflectionVector
// Type: Helper function
// Desc: Calculate the reflection vector based on view and normal vectors
//-----------------------------------------------------------------------------
float3 CalcReflectionVector(float3 ViewToPos, float3 Normal)
{
    return reflect(ViewToPos, Normal);
}




//-----------------------------------------------------------------------------
// Name: CalcVertexAnimation
// Type: Helper function
// Desc: Calculate a per-vertex transformation offset
//-----------------------------------------------------------------------------
float3 CalcVertexAnimation(float3 Offset)
{
    return sin(Time + Offset) * 0.1;      
}




//-----------------------------------------------------------------------------
// Name: CalcHemisphere
// Type: Helper function
// Desc: Calculates the hemispheric lighting term
//-----------------------------------------------------------------------------
float4 CalcHemisphere(float3 Normal, float3 DirToSky, float Occ)
{
    float4 Hemi = MaterialAmbientIntensity;
    
    // occlusion factor
    Hemi *= (1 - Occ); 
    
    // calc lerp factor here
    float LerpFactor = (dot(Normal, DirToSky) + 1) / 2; 
    
    // lerp between the ground and sky color based on the LerpFactor which 
    //   is calculated by the angle between the normal and the sky direction
    Hemi *= lerp(GroundColor, SkyColor, LerpFactor);

    return Hemi;
}




//-----------------------------------------------------------------------------
// Name: CalcAmbient
// Type: Helper function
// Desc: Calculates the ambient lighting term
//-----------------------------------------------------------------------------
float4 CalcAmbient()
{
    return LightAmbientIntensity * MaterialAmbientIntensity; 
}




//-----------------------------------------------------------------------------
// Name: CalcDiffuse
// Type: Helper function
// Desc: Calculates the diffuse lighting term
//-----------------------------------------------------------------------------
float4 CalcDiffuse(float3 Normal, float3 DirToLight)
{
    return MaterialDiffuseColor * LightDiffuseColor * max(0, dot(Normal, DirToLight));
}




//-----------------------------------------------------------------------------
// Name: CalcSpecular
// Type: Helper function
// Desc: Calculates the specular lighting term
//-----------------------------------------------------------------------------
float4 CalcSpecular(float3 Normal, float3 DirFromLight, float3 EyeToVertex)
{
    float3 R = normalize(reflect(DirFromLight, Normal));
    return MaterialSpecularColor * LightSpecularColor * pow(max(0, dot(R, -EyeToVertex)), MaterialSpecularPower/4);
}




// vertex shader output structure
struct VS_OUTPUT
{
    float4 Pos  : POSITION;
    float4 Diff : COLOR0;
    float4 Spec : COLOR1;
    float3 CubeTexcoord : TEXCOORD0;
    float3 VolTexcoord: TEXCOORD1;
};

//-----------------------------------------------------------------------------
// Name: VS
// Type: Vertex shader
// Desc: Transforms the model from object to projection space and calculates
//       the per-vertex lighting components.
//-----------------------------------------------------------------------------
VS_OUTPUT VS(
    float3 InPos  : POSITION,   // Vertex position in model space
    float3 InNormal : NORMAL,   // Vertex normal in model space
    float  Occ  : TEXCOORD0,    // Occlusion factor
    uniform bool bAmbient,      // Ambient lighting toggle
    uniform bool bHemispheric,  // Hemispheric lighting toggle
    uniform bool bDiffuse,      // Diffuse lighting toggle
    uniform bool bSpecular)     // Specular lighting toggle
{
    VS_OUTPUT Out = (VS_OUTPUT)0;

    // transform the position and normal
    float3 Pos = mul(float4(InPos, 1), (float4x3)World);         // position (view space)
    
    // add to position based on time calculation for vertex animation
    Pos += CalcVertexAnimation(InPos);

    float3 Normal = normalize(mul(InNormal, (float3x3)World));   // normal (view space)
    float3 EyeToVertex = normalize(Pos - CameraPos);             // vector from vertex towards eye


    Out.Pos  = mul(float4(Pos, 1), ViewProjection);              // position (projected)
    
    // calculate the selected lighting terms
    if( bDiffuse )
        Out.Diff += CalcDiffuse(Normal, -DirFromLight); 
        
    if( bAmbient )
        Out.Diff += CalcAmbient();
        
    if( bHemispheric )
        Out.Diff += CalcHemisphere(Normal, -DirFromSky, Occ); 
        
    if( bSpecular )
        Out.Spec += CalcSpecular(Normal, DirFromLight, EyeToVertex);

    // generate a reflection vector to lookup in a reflection cube-map
    Out.CubeTexcoord = CalcReflectionVector(EyeToVertex, Normal);
    
    // output the position as the volume texture coordinate
    Out.VolTexcoord = InPos * 2;
    
    return Out;
}




//-----------------------------------------------------------------------------
// Name: PS
// Type: Pixel shader
// Desc: Calculates the pixel color based on interpolated vertex colors
//-----------------------------------------------------------------------------
float4 PS ( VS_OUTPUT In) : COLOR
{
    float4 OutColor;
    
    OutColor = In.Diff + ProceduralLookup(In.VolTexcoord);
    
    OutColor = lerp(OutColor, CubeMapLookup(In.CubeTexcoord), 0.25);

    OutColor += In.Spec;

    return OutColor;
}




//-----------------------------------------------------------------------------
// Name: HemisphereDiffuseSpecular
// Type: Technique
// Desc: Render with hemispheric, diffuse, and specular lighting terms
//-----------------------------------------------------------------------------
technique HemisphereDiffuseSpecular
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS(false, true, true, true);
        PixelShader  = compile ps_2_0 PS();
    }
}




//-----------------------------------------------------------------------------
// Name: HemisphereDiffuse
// Type: Technique
// Desc: Render with hemispheric and diffuse lighting terms
//-----------------------------------------------------------------------------
technique HemisphereDiffuse
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS(false, true, true, false);
        PixelShader  = compile ps_2_0 PS();
    }
}




//-----------------------------------------------------------------------------
// Name: Hemisphere
// Type: Technique
// Desc: Render with hemispheric lighting term only
//-----------------------------------------------------------------------------
technique Hemisphere
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS(false, true, false, false);
        PixelShader  = compile ps_2_0 PS();
    }
}




//-----------------------------------------------------------------------------
// Name: Specular
// Type: Technique
// Desc: Render with specular lighting term only
//-----------------------------------------------------------------------------
technique Specular
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS(false, false, false, true);
        PixelShader  = compile ps_2_0 PS();
    }
}




//-----------------------------------------------------------------------------
// Name: Ambient
// Type: Technique
// Desc: Render with ambient lighting term only
//-----------------------------------------------------------------------------
technique Ambient
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS(true, false, false, false);
        PixelShader  = compile ps_2_0 PS();
    }
}




//-----------------------------------------------------------------------------
// Name: AmbientDiffuse
// Type: Technique
// Desc: Render with ambient and diffuse lighting terms
//-----------------------------------------------------------------------------
technique AmbientDiffuse
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS(true, false, true, false);
        PixelShader  = compile ps_2_0 PS();
    }
}




//-----------------------------------------------------------------------------
// Name: AmbientDiffuseSpecular
// Type: Technique
// Desc: Render with ambient, diffuse, and specular lighting terms
//-----------------------------------------------------------------------------
technique AmbientDiffuseSpecular
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS(true, false, true, true);
        PixelShader  = compile ps_2_0 PS();
    }
}

