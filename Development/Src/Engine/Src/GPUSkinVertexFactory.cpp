/*=============================================================================
	GPUVertexFactory.cpp: GPU skin vertex factory implementation
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "GPUSkinVertexFactory.h"

/*-----------------------------------------------------------------------------
FGPUSkinVertexFactory
-----------------------------------------------------------------------------*/

UBOOL FGPUSkinVertexFactory::ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
{
	return Material->IsUsedWithSkeletalMesh() || Material->IsUsedWithDecals() || Material->IsUsedWithFracturedMeshes() || Material->IsSpecialEngineMaterial();
}

/**
* Add the decl elements for the streams
* @param InData - type with stream components
* @param OutElements - vertex decl list to modify
*/
void FGPUSkinVertexFactory::AddVertexElements(DataType& InData, FVertexDeclarationElementList& OutElements)
{
	// position decls
	OutElements.AddItem(AccessStreamComponent(InData.PositionComponent,VEU_Position));

	// tangent basis vector decls
	OutElements.AddItem(AccessStreamComponent(InData.TangentBasisComponents[0],VEU_Tangent));
	OutElements.AddItem(AccessStreamComponent(InData.TangentBasisComponents[1],VEU_Normal));

	// texture coordinate decls
	if(InData.TextureCoordinates.Num())
	{
		for(UINT CoordinateIndex = 0;CoordinateIndex < InData.TextureCoordinates.Num();CoordinateIndex++)
		{
			OutElements.AddItem(AccessStreamComponent(
				InData.TextureCoordinates(CoordinateIndex),
				VEU_TextureCoordinate,
				CoordinateIndex
				));
		}

		for(UINT CoordinateIndex = InData.TextureCoordinates.Num();CoordinateIndex < MAX_TEXCOORDS;CoordinateIndex++)
		{
			OutElements.AddItem(AccessStreamComponent(
				InData.TextureCoordinates(InData.TextureCoordinates.Num() - 1),
				VEU_TextureCoordinate,
				CoordinateIndex
				));
		}
	}

	// bone indices decls
	OutElements.AddItem(AccessStreamComponent(InData.BoneIndices,VEU_BlendIndices));

	// bone weights decls
	OutElements.AddItem(AccessStreamComponent(InData.BoneWeights,VEU_BlendWeight));
}

/**
* Creates declarations for each of the vertex stream components and
* initializes the device resource
*/
void FGPUSkinVertexFactory::InitRHI()
{
	// list of declaration items
	FVertexDeclarationElementList Elements;
	AddVertexElements(Data,Elements);	

	// create the actual device decls
	InitDeclaration(Elements,FVertexFactory::DataType(),FALSE,FALSE);
}

/*-----------------------------------------------------------------------------
FGPUSkinVertexFactoryShaderParameters
-----------------------------------------------------------------------------*/

/** Shader parameters for use with FGPUSkinVertexFactory */
class FGPUSkinVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
public:
	/**
	* Bind shader constants by name
	* @param	ParameterMap - mapping of named shader constants to indices
	*/
	virtual void Bind(const FShaderParameterMap& ParameterMap)
	{
		LocalToWorldParameter.Bind(ParameterMap,TEXT("LocalToWorld"));
		WorldToLocalParameter.Bind(ParameterMap,TEXT("WorldToLocal"),TRUE);
		BoneMatricesParameter.Bind(ParameterMap,TEXT("BoneMatrices"));
		MaxBoneInfluencesParameter.Bind(ParameterMap,TEXT("MaxBoneInfluences"),TRUE);
	}
	/**
	* Serialize shader params to an archive
	* @param	Ar - archive to serialize to
	*/
	virtual void Serialize(FArchive& Ar)
	{
		Ar << LocalToWorldParameter;
		Ar << WorldToLocalParameter;
		Ar << BoneMatricesParameter;
		Ar << MaxBoneInfluencesParameter;
	}
	/**
	* Set any shader data specific to this vertex factory
	*/
	virtual void Set(FShader* VertexShader,const FVertexFactory* VertexFactory,const FSceneView& View) const
	{
		const FGPUSkinVertexFactory::ShaderDataType& ShaderData = ((const FGPUSkinVertexFactory*)VertexFactory)->GetShaderData();

		//SCOPED_DRAW_EVENT(GPUSkinVF)(DEC_SCENE_ITEMS,TEXT("GPUSkinVF=%d"),ShaderData.MaxBoneInfluences);
		SetVertexShaderValues<FSkinMatrix3x4>(
			VertexShader->GetVertexShader(),
			BoneMatricesParameter,
			ShaderData.BoneMatrices.GetTypedData(),
			ShaderData.BoneMatrices.Num()
			);
		SetVertexShaderValue(
			VertexShader->GetVertexShader(),
			MaxBoneInfluencesParameter,
			(FLOAT)ShaderData.MaxBoneInfluences
			);
	}
	/**
	* Set the l2w transform shader
	*/
	virtual void SetMesh(FShader* VertexShader,const FMeshElement& Mesh,const FSceneView& View) const
	{
		SetVertexShaderValue(
			VertexShader->GetVertexShader(),
			LocalToWorldParameter,
			Mesh.LocalToWorld.ConcatTranslation(View.PreViewTranslation)
			);
		SetVertexShaderValue(VertexShader->GetVertexShader(),WorldToLocalParameter,Mesh.WorldToLocal);
	}
private:
	FShaderParameter LocalToWorldParameter;
	FShaderParameter WorldToLocalParameter;
	FShaderParameter BoneMatricesParameter;
	FShaderParameter MaxBoneInfluencesParameter;
};

/** bind gpu skin vertex factory to its shader file and its shader parameters */
IMPLEMENT_VERTEX_FACTORY_TYPE(FGPUSkinVertexFactory, FGPUSkinVertexFactoryShaderParameters, "GpuSkinVertexFactory", TRUE, FALSE, FALSE, VER_SKELETAL_MESH_REMOVE_BINORMAL_TANGENT_VECTOR, 0);

/*-----------------------------------------------------------------------------
FGPUSkinMorphVertexFactory
-----------------------------------------------------------------------------*/

/**
* Modify compile environment to enable the morph blend codepath
* @param OutEnvironment - shader compile environment to modify
*/
void FGPUSkinMorphVertexFactory::ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
{
	OutEnvironment.Definitions.Set(TEXT("GPUSKIN_MORPH_BLEND"),TEXT("1"));
}

/**
* Should we cache the material's shader type on this platform with this vertex factory? 
*/
UBOOL FGPUSkinMorphVertexFactory::ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
{
	return FGPUSkinVertexFactory::ShouldCache(Platform, Material, ShaderType);
}

/**
* Add the decl elements for the streams
* @param InData - type with stream components
* @param OutElements - vertex decl list to modify
*/
void FGPUSkinMorphVertexFactory::AddVertexElements(DataType& InData, FVertexDeclarationElementList& OutElements)
{
	// add the base gpu skin elements
	FGPUSkinVertexFactory::AddVertexElements(InData,OutElements);
	// add the morph delta elements
	// NOTE: TEXCOORD6,TEXCOORD7 used instead of POSITION1,NORMAL1 since those semantics are not supported by Cg 
	OutElements.AddItem(AccessStreamComponent(InData.DeltaPositionComponent,VEU_TextureCoordinate,6));
	OutElements.AddItem(AccessStreamComponent(InData.DeltaTangentZComponent,VEU_TextureCoordinate,7));
}

/**
* Creates declarations for each of the vertex stream components and
* initializes the device resource
*/
void FGPUSkinMorphVertexFactory::InitRHI()
{
	// list of declaration items
	FVertexDeclarationElementList Elements;	
	AddVertexElements(MorphData,Elements);

	// create the actual device decls
	InitDeclaration(Elements,FVertexFactory::DataType(),FALSE,FALSE);
}

/** bind morph target gpu skin vertex factory to its shader file and its shader parameters */
IMPLEMENT_VERTEX_FACTORY_TYPE(FGPUSkinMorphVertexFactory, FGPUSkinVertexFactoryShaderParameters, "GpuSkinVertexFactory", TRUE, FALSE, FALSE, VER_SKELETAL_MESH_REMOVE_BINORMAL_TANGENT_VECTOR, 0);

/*-----------------------------------------------------------------------------
FGPUSkinDecalVertexFactory
-----------------------------------------------------------------------------*/

/**
 * Shader parameters for use with FGPUSkinDecalVertexFactory.
 */
class FGPUSkinDecalVertexFactoryShaderParameters : public FGPUSkinVertexFactoryShaderParameters
{
public:
	typedef FGPUSkinVertexFactoryShaderParameters Super;

	/**
	 * Bind shader constants by name
	 * @param	ParameterMap - mapping of named shader constants to indices
	 */
	virtual void Bind(const FShaderParameterMap& ParameterMap)
	{
		Super::Bind( ParameterMap );
		BoneToDecalRow0Parameter.Bind( ParameterMap, TEXT("BoneToDecalRow0"), TRUE );
		BoneToDecalRow1Parameter.Bind( ParameterMap, TEXT("BoneToDecalRow1"), TRUE );
		DecalLocationParameter.Bind( ParameterMap, TEXT("DecalLocation"), TRUE );
	}

	/**
	 * Serialize shader params to an archive
	 * @param	Ar - archive to serialize to
	 */
	virtual void Serialize(FArchive& Ar)
	{
		Super::Serialize( Ar );
		Ar << BoneToDecalRow0Parameter;
		Ar << BoneToDecalRow1Parameter;
		Ar << DecalLocationParameter;
	}

	/**
	 * Set any shader data specific to this vertex factory
	 */
	virtual void Set(FShader* VertexShader, const FVertexFactory* VertexFactory, const FSceneView& View) const
	{
		Super::Set( VertexShader, VertexFactory, View );

		FGPUSkinDecalVertexFactory * DecalVertexFactory = (FGPUSkinDecalVertexFactory *)VertexFactory;
		const FMatrix& DecalMtx = DecalVertexFactory->GetDecalMatrix();
		if ( BoneToDecalRow0Parameter.IsBound() )
		{
			const FVector4 Row0( DecalMtx.M[0][0], DecalMtx.M[1][0], DecalMtx.M[2][0], DecalMtx.M[3][0]);
			SetVertexShaderValue( VertexShader->GetVertexShader(), BoneToDecalRow0Parameter, Row0 );
		}
		if ( BoneToDecalRow1Parameter.IsBound() )
		{
			const FVector4 Row1( DecalMtx.M[0][1], DecalMtx.M[1][1], DecalMtx.M[2][1], DecalMtx.M[3][1]);
			SetVertexShaderValue( VertexShader->GetVertexShader(), BoneToDecalRow1Parameter, Row1 );
		}
		if ( DecalLocationParameter.IsBound() )
		{
			SetVertexShaderValue( VertexShader->GetVertexShader(), DecalLocationParameter, DecalVertexFactory->GetDecalLocation() );
		}
	}

private:
	FShaderParameter BoneToDecalRow0Parameter;
	FShaderParameter BoneToDecalRow1Parameter;
	FShaderParameter DecalLocationParameter;
};

/**
 * Should we cache the material's shader type on this platform with this vertex factory? 
 */
UBOOL FGPUSkinDecalVertexFactory::ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
{
	return (Material->IsUsedWithDecals() || Material->IsDecalMaterial() || Material->IsSpecialEngineMaterial());
}

/**
* Modify compile environment to enable the decal codepath
* @param OutEnvironment - shader compile environment to modify
*/
void FGPUSkinDecalVertexFactory::ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
{
	Super::ModifyCompilationEnvironment(Platform, OutEnvironment);
	OutEnvironment.Definitions.Set(TEXT("GPUSKIN_DECAL"),TEXT("1"));
}

/** bind gpu skin decal vertex factory to its shader file and its shader parameters */
IMPLEMENT_VERTEX_FACTORY_TYPE( FGPUSkinDecalVertexFactory, FGPUSkinDecalVertexFactoryShaderParameters, "GpuSkinVertexFactory", TRUE, FALSE, FALSE, VER_SKELETAL_MESH_REMOVE_BINORMAL_TANGENT_VECTOR, 0 );

/*-----------------------------------------------------------------------------
FGPUSkinMorphDecalVertexFactory
-----------------------------------------------------------------------------*/

/**
* Should we cache the material's shader type on this platform with this vertex factory? 
*/
UBOOL FGPUSkinMorphDecalVertexFactory::ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
{
	return (Material->IsUsedWithDecals() || Material->IsDecalMaterial() || Material->IsSpecialEngineMaterial());
}

/**
* Modify compile environment to enable the decal codepath
* @param OutEnvironment - shader compile environment to modify
*/
void FGPUSkinMorphDecalVertexFactory::ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
{
	Super::ModifyCompilationEnvironment(Platform, OutEnvironment);
	OutEnvironment.Definitions.Set(TEXT("GPUSKIN_DECAL"),TEXT("1"));
}

/** bind gpu skin decal vertex factory to its shader file and its shader parameters */
IMPLEMENT_VERTEX_FACTORY_TYPE( FGPUSkinMorphDecalVertexFactory, FGPUSkinDecalVertexFactoryShaderParameters, "GpuSkinVertexFactory", TRUE, FALSE, FALSE, VER_SKELETAL_MESH_REMOVE_BINORMAL_TANGENT_VECTOR, 0 );
