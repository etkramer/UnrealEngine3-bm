//--------------------------------------------------------------------------------------
// File: EmptyProject.fx
//
// The effect file for the EmptyProject sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
float appTime;  //App's time in seconds
float4x4 worldMatrix;  // World matrix for object
float4x4 worldViewProjection;	// World * View * Projection matrix


//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique RenderScene
{
    pass P0
    {          
    }
}
