/*=============================================================================
	ParticleInstancedMeshVertexFactory.cpp: Instanced Mesh Particles Vertex Factory.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ParticleInstancedMeshVertexFactory.h"

void FParticleInstancedMeshVertexFactoryShaderParameters::Bind(const FShaderParameterMap& ParameterMap)
{
	InvNumVerticesPerInstanceParameter.Bind(ParameterMap,TEXT("InvNumVerticesPerInstance"),TRUE);
	NumVerticesPerInstanceParameter.Bind(ParameterMap,TEXT("NumVerticesPerInstance"),TRUE);
}

void FParticleInstancedMeshVertexFactoryShaderParameters::Serialize(FArchive& Ar)
{
	Ar << InvNumVerticesPerInstanceParameter;
	Ar << NumVerticesPerInstanceParameter;
}

void FParticleInstancedMeshVertexFactoryShaderParameters::Set(FShader* VertexShader,const FVertexFactory* VertexFactory,const FSceneView& View) const
{
	SetVertexShaderValue(VertexShader->GetVertexShader(),InvNumVerticesPerInstanceParameter,1.0f / (FLOAT)VertexFactory->GetNumVerticesPerInstance());
	SetVertexShaderValue(VertexShader->GetVertexShader(),NumVerticesPerInstanceParameter,(FLOAT)VertexFactory->GetNumVerticesPerInstance());
}

void FParticleInstancedMeshVertexFactoryShaderParameters::SetMesh(FShader* VertexShader,const FMeshElement& Mesh,const FSceneView& View) const
{
}

UBOOL FParticleInstancedMeshVertexFactory::ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
{
	return Material->IsUsedWithInstancedMeshParticles() || Material->IsSpecialEngineMaterial();
}

void FParticleInstancedMeshVertexFactory::InitRHI()
{
	FVertexDeclarationElementList Elements;

	Elements.AddItem(AccessStreamComponent(Data.PositionComponent,VEU_Position));

	EVertexElementUsage TangentBasisUsages[2] = { VEU_Tangent, VEU_Normal };
	for(INT AxisIndex = 0;AxisIndex < 2;AxisIndex++)
	{
		if(Data.TangentBasisComponents[AxisIndex].VertexBuffer != NULL)
		{
			Elements.AddItem(AccessStreamComponent(Data.TangentBasisComponents[AxisIndex],TangentBasisUsages[AxisIndex]));
		}
	}

	if(Data.TextureCoordinateComponent.VertexBuffer)
	{
		Elements.AddItem(AccessStreamComponent(Data.TextureCoordinateComponent,VEU_TextureCoordinate));
	}

	if(Data.TextureCoordinateComponent.VertexBuffer)
	{
		Elements.AddItem(AccessStreamComponent(Data.TextureCoordinateComponent,VEU_Color));
	}

	Elements.AddItem(AccessStreamComponent(Data.InstanceOffsetComponent,VEU_TextureCoordinate,1));
	Elements.AddItem(AccessStreamComponent(Data.InstanceAxisComponents[0],VEU_TextureCoordinate,2));
	Elements.AddItem(AccessStreamComponent(Data.InstanceAxisComponents[1],VEU_TextureCoordinate,3));
	Elements.AddItem(AccessStreamComponent(Data.InstanceAxisComponents[2],VEU_TextureCoordinate,4));

	InitDeclaration(Elements,Data);
}

IMPLEMENT_VERTEX_FACTORY_TYPE(FParticleInstancedMeshVertexFactory,FParticleInstancedMeshVertexFactoryShaderParameters,"FoliageVertexFactory",TRUE,FALSE,FALSE, VER_SM2_BLENDING_SHADER_FIXES, 0);

