/*=============================================================================
	LocalVertexFactoryShaderParms.cpp: Local vertex factory shader parameters
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "LocalVertexFactoryShaderParms.h"

void FLocalVertexFactoryShaderParameters::Bind(const FShaderParameterMap& ParameterMap)
{
	LocalToWorldParameter.Bind(ParameterMap,TEXT("LocalToWorld"));
	WorldToLocalParameter.Bind(ParameterMap,TEXT("WorldToLocal"),TRUE);
}

void FLocalVertexFactoryShaderParameters::Serialize(FArchive& Ar)
{
	Ar << LocalToWorldParameter;
	Ar << WorldToLocalParameter;
}

void FLocalVertexFactoryShaderParameters::Set(FShader* VertexShader,const FVertexFactory* VertexFactory,const FSceneView& View) const
{	
}

void FLocalVertexFactoryShaderParameters::SetMesh(FShader* VertexShader,const FMeshElement& Mesh,const FSceneView& View) const
{
	SetVertexShaderValue(
		VertexShader->GetVertexShader(),
		LocalToWorldParameter,
		Mesh.LocalToWorld.ConcatTranslation(View.PreViewTranslation)
		);

	// We don't bother removing the view translation from the world-to-local transform because the shader doesn't use it to transform points.
	SetVertexShaderValue(VertexShader->GetVertexShader(),WorldToLocalParameter,Mesh.WorldToLocal);
}
