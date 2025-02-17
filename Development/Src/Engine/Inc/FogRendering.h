/*=============================================================================
	FogRendering.h: 
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/


/** Encapsulates parameters needed to calculate height fog in a vertex shader. */
class FHeightFogVertexShaderParameters
{
public:

	/** Binds the parameters. */
	void Bind(const FShaderParameterMap& ParameterMap);

	/** 
	* Sets the parameter values, this must be called before rendering the primitive with the shader applied. 
	* @param VertexShader - the vertex shader to set the parameters on
	*/
	void Set(const FMaterialRenderProxy* MaterialRenderProxy,const FSceneView* View,FShader* VertexShader) const;

	/** Serializer. */
	friend FArchive& operator<<(FArchive& Ar,FHeightFogVertexShaderParameters& P);

private:
	FShaderParameter	FogDistanceScaleParameter;
	FShaderParameter	FogExtinctionDistanceParameter;
	FShaderParameter	FogMinHeightParameter;
	FShaderParameter	FogMaxHeightParameter;
	FShaderParameter	FogInScatteringParameter;
	FShaderParameter	FogStartDistanceParameter;
};
