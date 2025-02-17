// High-Level Shader Language (HLSL) Workshop
// Goal 1: Hemispheric Lighting
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



//  Goal 1 - Hemisphere lighting:
//    A) Use the occlusion term stored per vertex to modulate the hemiphere term 
//    B) Calculate the hemisphere light value
//    C) Change the sky and ground color
//   Extra Credit:
//       Make the skull translucent where the occlusion term is low (set alpha blend states)




string XFile = "misc\\skullocc.x";                // model
string Environment = "lobby\\lobbycube.dds";       // environment map

// light directions (view space)
float3 DirFromLight < string UIDirectional = "Light Direction"; > = {0.577, -0.577, 0.577};

// direction of light from sky (view space)
float3 DirFromSky < string UIDirectional = "Direction from Sky"; > = { 0.0f, -1.0f, 0.0f };            

// light intensity
float4 LightAmbientIntensity    = { 0.8f, 0.8f, 0.8f, 1.0f };    // ambient
float4 GroundColor              = { 0.1f, 0.0f, 0.0f, 1.0f };    // ground
float4 SkyColor                 = { 0.9f, 0.9f, 1.0f, 1.0f };    // sky
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




//-----------------------------------------------------------------------------
// Name: CalcHemisphere
// Type: Helper function
// Desc: Calculates the hemispheric lighting term
//-----------------------------------------------------------------------------
float4 CalcHemisphere(float3 Normal, float3 DirToSky, float Occ)
{
    float4 Hemi = MaterialAmbientIntensity;
    
    // Goal 1A; Use the occlusion term stored per vertex to modulate the hemiphere term 
    //
    //  What is the occlusion term:
    //    The occlusion term is a weight that was calculated in a offline
    //      pass of the model that is the percentage of the hemisphere that 
    //      is blocked by other parts of the model.  In the term provided
    //      a value of 0 means that the vertex is not occluded by any
    //      geometry (all light will be received) versus a value of 1
    //      which means that the vertex is completely blocked from light
    //      
    //  Suggestions:
    //    Multiplying by 1 - Occ will darken the hemisphere light in relation
    //      to how much this vertex is affected by a hemisphere light
    
    Hemi *= 1; // occlusion factor
    
    // Goal 1B: calculate the hemisphere light value
    //
    //   What is a hemisphere light:
    //      A "hemisphere" light is an ambient term that is made up of
    //      two colors, sky and ground, that are interpolated between
    //      based on the normal.  i.e. areas with normals pointing towards
    //      the sky are "sky" colored, and areas pointed towards the 
    //      "ground" are more "ground" colored.
    
    //   Suggestions:
    //     An easy way to implement a hemisphere light is to lerp
    //        between the sky color and the ground color based on 
    //        the angle between the normal and sky direction.
    //        The angle can easily be calculated by doing a dot 
    //        product between the normal and sky direction.
    //        This results in a -1 to 1 value that when remapped
    //        to 0 to 1, is perfect for lerping between the two colors

    float LerpFactor = 0.5; // calc lerp factor here
    
    // lerp between the ground and sky color based on the LerpFactor which 
    //   is calculated by the angle between the normal and the sky direction
    Hemi *= lerp(GroundColor, SkyColor, LerpFactor);

    // Goal 1C; Change the sky and ground color
    //
    //  Experiment with a few values for sky/ground:
    //    The hemisphere light is used to simulate an environment that
    //    is dominated by one color at the north and south poles of
    //    a "sphere" surrounding the object.  One hemisphere is the
    //    sky color and the other is the ground color.  An example
    //    of this is blue for a sky and green for ground.  This will
    //    simulate basic radiosity of a scene for an object sitting
    //    on a green surface like grass with a blue sky overhead.
    //
    //  Suggestions:
    //    Blue for sky and Green for ground
    //    NOTE: That the default color values are stored at the top of the file.
    
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
    
    OutColor = In.Diff + In.Spec;

    return OutColor;
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

