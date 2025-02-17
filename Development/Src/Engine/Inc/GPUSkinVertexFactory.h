/*=============================================================================
	GPUSkinVertexFactory.h: GPU skinning vertex factory definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __GPUSKINVERTEXFACTORY_H__
#define __GPUSKINVERTEXFACTORY_H__

#include "GPUSkinPublicDefs.h"

/** for final bone matrices */
struct FSkinMatrix3x4
{
	FLOAT M[3][4];
	FORCEINLINE void SetMatrix(const FMatrix& Mat)
	{
		M[0][0] = Mat.M[0][0];
		M[0][1] = Mat.M[0][1];
		M[0][2] = Mat.M[0][2];
		M[0][3] = Mat.M[0][3];

		M[1][0] = Mat.M[1][0];
		M[1][1] = Mat.M[1][1];
		M[1][2] = Mat.M[1][2];
		M[1][3] = Mat.M[1][3];

		M[2][0] = Mat.M[2][0];
		M[2][1] = Mat.M[2][1];
		M[2][2] = Mat.M[2][2];
		M[2][3] = Mat.M[2][3];
	}

	FORCEINLINE void SetMatrixTranspose(const FMatrix& Mat)
	{
		M[0][0] = Mat.M[0][0];
		M[0][1] = Mat.M[1][0];
		M[0][2] = Mat.M[2][0];
		M[0][3] = Mat.M[3][0];

		M[1][0] = Mat.M[0][1];
		M[1][1] = Mat.M[1][1];
		M[1][2] = Mat.M[2][1];
		M[1][3] = Mat.M[3][1];

		M[2][0] = Mat.M[0][2];
		M[2][1] = Mat.M[1][2];
		M[2][2] = Mat.M[2][2];
		M[2][3] = Mat.M[3][2];
	}
};

/** Vertex factory with vertex stream components for GPU skinned vertices */
class FGPUSkinVertexFactory : public FVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FGPUSkinVertexFactory);

public:
	struct DataType
	{
		/** The stream to read the vertex position from. */
		FVertexStreamComponent PositionComponent;

		/** The streams to read the tangent basis from. */
		FVertexStreamComponent TangentBasisComponents[2];

		/** The streams to read the texture coordinates from. */
		TStaticArray<FVertexStreamComponent,MAX_TEXCOORDS> TextureCoordinates;

		/** The stream to read the bone indices from */
		FVertexStreamComponent BoneIndices;

		/** The stream to read the bone weights from */
		FVertexStreamComponent BoneWeights;
	};

	struct ShaderDataType
	{
		/**
		 * Constructor, presizing bones array.
		 *
		 * @param	NumBones	Number of bones to presize aray for.
		 */
		ShaderDataType( INT NumBones )
		{
            BoneMatrices.Empty(NumBones);
			BoneMatrices.Add(NumBones);
		}
		/** ref pose to local space matrices */
		TArray<FSkinMatrix3x4> BoneMatrices;
		/** max number of bone influences used by verts */
		INT MaxBoneInfluences;

		/** Set max bone influences clamped to MAX_INFLUENCES */
		void SetMaxBoneInfluences(INT InMaxBoneInfluences)
		{
			MaxBoneInfluences = Min<INT>(InMaxBoneInfluences,MAX_INFLUENCES);
		}
	};

	/**
	 * Constructor presizing bone matrices array to used amount.
	 *
	 * @param	NumBones	Number of bones associated with this vertex factory.
	 */
	FGPUSkinVertexFactory( INT NumBones )
	:	ShaderData( NumBones )
	{}

	/**
	 * Should we cache the material's shadertype on this platform with this vertex factory? 
	 */
	static UBOOL ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType);

	/**
	* An implementation of the interface used by TSynchronizedResource to 
	* update the resource with new data from the game thread.
	* @param	InData - new stream component data
	*/
	void SetData(const DataType& InData)
	{
		Data = InData;
		UpdateRHI();
	}

	/** accessor */
	FORCEINLINE ShaderDataType& GetShaderData()
	{
		return ShaderData;
	}

	FORCEINLINE const ShaderDataType& GetShaderData() const
	{
		return ShaderData;
	}

	virtual UBOOL IsGPUSkinned() const { return TRUE; }

	// FRenderResource interface.

	/**
	* Creates declarations for each of the vertex stream components and
	* initializes the device resource
	*/
	virtual void InitRHI();

protected:
	/**
	* Add the decl elements for the streams
	* @param InData - type with stream components
	* @param OutElements - vertex decl list to modify
	*/
	void AddVertexElements(DataType& InData, FVertexDeclarationElementList& OutElements);

private:
	/** stream component data bound to this vertex factory */
	DataType Data;  
	/** dynamic data need for setting the shader */ 
	ShaderDataType ShaderData;
};

/** Vertex factory with vertex stream components for GPU-skinned and morph target streams */
class FGPUSkinMorphVertexFactory : public FGPUSkinVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FGPUSkinMorphVertexFactory);

public:

	struct DataType : FGPUSkinVertexFactory::DataType
	{
		/** stream which has the position deltas to add to the vertex position */
		FVertexStreamComponent DeltaPositionComponent;
		/** stream which has the TangentZ deltas to add to the vertex normals */
		FVertexStreamComponent DeltaTangentZComponent;
	};

	/**
	 * Constructor presizing bone matrices array to used amount.
	 *
	 * @param	NumBones	Number of bones associated with this vertex factory.
	 */
	FGPUSkinMorphVertexFactory( INT NumBones )
	:	FGPUSkinVertexFactory( NumBones )
	{}

	/**
	* Modify compile environment to enable the morph blend codepath
	* @param OutEnvironment - shader compile environment to modify
	*/
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment);

	/**
	* Should we cache the material's shader type on this platform with this vertex factory? 
	*/
	static UBOOL ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType);

	/**
	* An implementation of the interface used by TSynchronizedResource to 
	* update the resource with new data from the game thread.
	* @param	InData - new stream component data
	*/
	void SetData(const DataType& InData)
	{
		MorphData = InData;
		UpdateRHI();
	}

	// FRenderResource interface.

	/**
	* Creates declarations for each of the vertex stream components and
	* initializes the device resource
	*/
	virtual void InitRHI();

protected:
	/**
	* Add the decl elements for the streams
	* @param InData - type with stream components
	* @param OutElements - vertex decl list to modify
	*/
	void AddVertexElements(DataType& InData, FVertexDeclarationElementList& OutElements);

private:
	/** stream component data bound to this vertex factory */
	DataType MorphData; 

};

#include "SkelMeshDecalVertexFactory.h"
 
/** Decal vertex factory with vertex stream components for GPU-skinned vertices */
class FGPUSkinDecalVertexFactory : public FGPUSkinVertexFactory, public FSkelMeshDecalVertexFactoryBase
{
	DECLARE_VERTEX_FACTORY_TYPE(FGPUSkinDecalVertexFactory);

public:
	typedef FGPUSkinVertexFactory Super;

	/**
	 * Constructor presizing bone matrices array to used amount.
	 *
	 * @param	NumBones	Number of bones associated with this vertex factory.
	 */
	FGPUSkinDecalVertexFactory( INT NumBones )
	:	FGPUSkinVertexFactory( NumBones )
	{}

	virtual FVertexFactory* CastToFVertexFactory()
	{
		return static_cast<FVertexFactory*>( this );
	}

	virtual FSkelMeshDecalVertexFactoryBase* CastToFSkelMeshDecalVertexFactoryBase()
	{
		return static_cast<FSkelMeshDecalVertexFactoryBase*>( this );
	}

	/**
	 * Should we cache the material's shader type on this platform with this vertex factory? 
	 */
	static UBOOL ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType);

	/**
	* Modify compile environment to enable the decal codepath
	* @param OutEnvironment - shader compile environment to modify
	*/
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment);
};

/** Decal vertex factory with vertex stream components for GPU-skinned vertices */
class FGPUSkinMorphDecalVertexFactory : public FGPUSkinMorphVertexFactory, public FSkelMeshDecalVertexFactoryBase
{
	DECLARE_VERTEX_FACTORY_TYPE(FGPUSkinMorphDecalVertexFactory);

public:
	typedef FGPUSkinMorphVertexFactory Super;

	/**
	 * Constructor presizing bone matrices array to used amount.
	 *
	 * @param	NumBones	Number of bones associated with this vertex factory.
	 */
	FGPUSkinMorphDecalVertexFactory( INT NumBones )
	:	FGPUSkinMorphVertexFactory( NumBones )
	{}

	virtual FVertexFactory* CastToFVertexFactory()
	{
		return static_cast<FVertexFactory*>( this );
	}

	virtual FSkelMeshDecalVertexFactoryBase* CastToFSkelMeshDecalVertexFactoryBase()
	{
		return static_cast<FSkelMeshDecalVertexFactoryBase*>( this );
	}

	/**
	* Should we cache the material's shader type on this platform with this vertex factory? 
	*/
	static UBOOL ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType);

	/**
	* Modify compile environment to enable the decal codepath
	* @param OutEnvironment - shader compile environment to modify
	*/
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment);
};

#endif // __GPUSKINVERTEXFACTORY_H__
