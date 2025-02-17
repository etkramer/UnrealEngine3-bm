//-----------------------------------------------------------------------------
// File: PrtPerVertex.fx
//
// Desc: The technique PrecomputedSHLighting renders the scene with per vertex Prt
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
float4x4 worldViewProjection;
texture AlbedoTexture;

#define NumberChannels	3
// The values for NumberClusters and NumberPcaVectors are
// defined by the app upon the D3DXCreateEffectFromFile() call.

float4 prtConstants[NumberClusters * ( 1 + NumberChannels * (NumberPcaVectors / 4))];

float4 MaterialDiffuseColor = { 1.0f, 1.0f, 1.0f, 1.0f };    


//-----------------------------------------------------------------------------
sampler AlbedoSampler = sampler_state
{ 
    Texture = (AlbedoTexture);
    MipFilter = LINEAR; 
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};


//-----------------------------------------------------------------------------
// Vertex shader output structure
//-----------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Position  : POSITION;    // position of the vertex
    float4 Diffuse   : COLOR0;      // diffuse color of the vertex
    float2 TexCoord  : TEXCOORD0;
};


//-----------------------------------------------------------------------------
float4 GetPrtDiffuse( int iClusterOffset, float4 vPcaWeights[NumberPcaVectors/4] )
{
    // With compressed Prt, a single diffuse channel is caluated by:
    //       R[p] = (M[k] dot L') + sum( w[p][j] * (B[k][j] dot L');
    // where the sum runs j between 0 and # of Pca vectors
    //       R[p] = exit radiance at point p
    //       M[k] = mean of cluster k 
    //       L' = source radiance coefficients
    //       w[p][j] = the j'th Pca weight for point p
    //       B[k][j] = the j'th Pca basis vector for cluster k
    //
    // Note: since both (M[k] dot L') and (B[k][j] dot L') can be computed on the CPU, 
    // these values are passed in as the array prtConstants.   
           
    float4 vAccumR = float4(0,0,0,0);
    float4 vAccumG = float4(0,0,0,0);
    float4 vAccumB = float4(0,0,0,0);
    
    // For each channel, multiply and sum all the vPcaWeights[j] by prtConstants[x] 
    // where: vPcaWeights[j] is w[p][j]
    //		  prtConstants[x] is the value of (B[k][j] dot L') that was
    //		  calculated on the CPU and passed in as a shader constant
    // Note this code is multipled and added 4 floats at a time since each 
    // register is a 4-D vector, and is the reason for using (NumberPcaVectors/4)
    for (int j=0; j < (NumberPcaVectors/4); j++) 
    {
        vAccumR += vPcaWeights[j] * prtConstants[iClusterOffset+1+(NumberPcaVectors/4)*0+j];
        vAccumG += vPcaWeights[j] * prtConstants[iClusterOffset+1+(NumberPcaVectors/4)*1+j];
        vAccumB += vPcaWeights[j] * prtConstants[iClusterOffset+1+(NumberPcaVectors/4)*2+j];
    }    

	// Now for each channel, sum the 4D vector and add prtConstants[x] 
	// where: prtConstants[x] which is the value of (M[k] dot L') and
	//		  was calculated on the CPU and passed in as a shader constant.
    float4 vDiffuse = prtConstants[iClusterOffset];
    vDiffuse.r += dot(vAccumR,1);
    vDiffuse.g += dot(vAccumG,1);
    vDiffuse.b += dot(vAccumB,1);
    
    return vDiffuse;
}


//-----------------------------------------------------------------------------
// Renders using per vertex Prt with compression with optional texture
//-----------------------------------------------------------------------------
VS_OUTPUT PrtDiffuseVS( float4 vPos : POSITION,
                        float2 TexCoord : TEXCOORD0,
                        int iClusterOffset : BLENDWEIGHT,
                        float4 vPcaWeights[NumberPcaVectors/4] : BLENDWEIGHT1,
                        uniform bool bUseTexture )
{
    VS_OUTPUT Output;
    
    // Output the vetrex position in projection space
    Output.Position = mul(vPos, worldViewProjection);
    if( bUseTexture )
        Output.TexCoord = TexCoord;
    else
        Output.TexCoord = 0;
    
    // For spectral simulations the material properity is baked into the transfer coefficients.
    // If using nonspectral, then you can modulate by the diffuse material properity here.
    Output.Diffuse = GetPrtDiffuse( iClusterOffset, vPcaWeights );
    
    Output.Diffuse *= MaterialDiffuseColor;
    
    return Output;
}


//-----------------------------------------------------------------------------
// Pixel shader output structure
//-----------------------------------------------------------------------------
struct PS_OUTPUT
{   
    float4 RGBColor : COLOR0;  // Pixel color    
};


//-----------------------------------------------------------------------------
// Name: StandardPS
// Type: Pixel shader
// Desc: Trival pixel shader
//-----------------------------------------------------------------------------
PS_OUTPUT StandardPS( VS_OUTPUT In, uniform bool bUseTexture ) 
{ 
    PS_OUTPUT Output;
    
    if( bUseTexture )
    {
        float4 Albedo = tex2D(AlbedoSampler, In.TexCoord);    
        Output.RGBColor = In.Diffuse * Albedo;
    }
    else
    {
        Output.RGBColor = In.Diffuse;
    }

    return Output;
}


//-----------------------------------------------------------------------------
// Renders with per vertex Prt 
//-----------------------------------------------------------------------------
technique RenderWithPrtColorLights
{
    pass P0
    {          
        VertexShader = compile vs_1_1 PrtDiffuseVS( true );
        PixelShader  = compile ps_1_1 StandardPS( true ); // trival pixel shader 
    }
}

//-----------------------------------------------------------------------------
// Renders with per vertex Prt w/o albedo texture
//-----------------------------------------------------------------------------
technique RenderWithPrtColorLightsNoAlbedo
{
    pass P0
    {          
        VertexShader = compile vs_1_1 PrtDiffuseVS( false );
        PixelShader  = compile ps_1_1 StandardPS( false ); // trival pixel shader 
    }
}
