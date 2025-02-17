/*=============================================================================
SpeedTreeVertexFactory.h: SpeedTree vertex factory definition.
Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#if WITH_SPEEDTREE

class FSpeedTreeVertexFactory : public FVertexFactory
{
public:

	struct DataType
	{
		FVertexStreamComponent								PositionComponent;
		FVertexStreamComponent								TangentBasisComponents[3];
		FVertexStreamComponent								WindInfo;
		TStaticArray<FVertexStreamComponent,MAX_TEXCOORDS>	TextureCoordinates;
		FVertexStreamComponent								ShadowMapCoordinateComponent;
	};

	/** When rendering a mesh element using a FSpeedTreeVertexFactory, its user data must point to an instance of MeshUserDataType. */
	struct MeshUserDataType
	{
		FVector BoundsOrigin;
		FMatrix RotationOnlyMatrix;
		FLOAT WindMatrixOffset;
		FLOAT LodMinDistance;
		FLOAT LodMaxDistance;
		FLOAT LodFadeRadius;
	};

	/** Initialization constructor. */
	FSpeedTreeVertexFactory(const USpeedTree* InSpeedTree):
		SpeedTree(InSpeedTree)
	{}

	static UBOOL ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType);
	virtual void InitRHI(void);

	void SetData(const DataType& InData)			
	{ 
		Data = InData; 
		UpdateRHI(); 
	}

	const USpeedTree* GetSpeedTree() const
	{
		return SpeedTree;
	}

private:
	DataType Data;
	const USpeedTree* SpeedTree;
};


class FSpeedTreeBranchVertexFactory : public FSpeedTreeVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FSpeedTreeBranchVertexFactory);
public:

	/** Initialization constructor. */
	FSpeedTreeBranchVertexFactory(const USpeedTree* InSpeedTree):
		FSpeedTreeVertexFactory(InSpeedTree)
	{}
};


class FSpeedTreeLeafCardVertexFactory : public FSpeedTreeBranchVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FSpeedTreeLeafCardVertexFactory);
public:

	/** Initialization constructor. */
	FSpeedTreeLeafCardVertexFactory(const USpeedTree* InSpeedTree):
		FSpeedTreeBranchVertexFactory(InSpeedTree)
	{}
};


class FSpeedTreeLeafMeshVertexFactory : public FSpeedTreeLeafCardVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FSpeedTreeLeafMeshVertexFactory);
public:

	/** Initialization constructor. */
	FSpeedTreeLeafMeshVertexFactory(const USpeedTree* InSpeedTree):
		FSpeedTreeLeafCardVertexFactory(InSpeedTree)
	{}
};


class FSpeedTreeBillboardVertexFactory : public FSpeedTreeVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FSpeedTreeBillboardVertexFactory);
public:

	/** Initialization constructor. */
	FSpeedTreeBillboardVertexFactory(const USpeedTree* InSpeedTree):
		FSpeedTreeVertexFactory(InSpeedTree)
	{}
};


#endif // WITH_SPEEDTREE

