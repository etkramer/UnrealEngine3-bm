/*=============================================================================
	SpeedTree.h: SpeedTree definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __SPEEDTREE_H__
#define __SPEEDTREE_H__

#include "EngineSpeedTreeClasses.h"			// SpeedTree object definitions

#if WITH_SPEEDTREE

//////////////////////////////////////////////////////
// includes

#include "SpeedTreeVertexFactory.h"			// SpeedTree vertex factory

#pragma pack(push,8)
#include "../../../External/SpeedTreeRT/include/SpeedTreeRT.h"
#pragma pack(pop)

class UMaterialInstanceConstant;
class USpeedTreeComponent;

/** Enumerates the types of meshes in a SpeedTreeComponent. */
enum ESpeedTreeMeshType
{
	STMT_Min,
	STMT_Branches = STMT_Min,
	STMT_Fronds,
	STMT_LeafMeshes,
	STMT_LeafCards,
	STMT_Billboards,
	STMT_Max
};

/** Chooses from the parameters given based on a given mesh type. */
template<typename T>
typename TCallTraits<T>::ParamType ChooseByMeshType(
	INT MeshType,
	typename TCallTraits<T>::ParamType BranchesChoice,
	typename TCallTraits<T>::ParamType FrondsChoice,
	typename TCallTraits<T>::ParamType LeafMeshesChoice,
	typename TCallTraits<T>::ParamType LeafCardsChoice,
	typename TCallTraits<T>::ParamType BillboardsChoice
	)
{
	switch(MeshType)
	{
	case STMT_Branches: return BranchesChoice;
	case STMT_Fronds: return FrondsChoice;
	case STMT_LeafMeshes: return LeafMeshesChoice;
	case STMT_LeafCards: return LeafCardsChoice;
	default:
	case STMT_Billboards: return BillboardsChoice;
	};
}

//	used for position
struct FSpeedTreeVertexPosition
{
	FVector			Position;
};

//	used for everything else
struct FSpeedTreeVertexData
{
	FPackedNormal	TangentX,
					TangentY,
					TangentZ;
	FVector2D		TexCoord;
	FVector4		WindInfo;
};

struct FSpeedTreeVertexDataBranchFrond : public FSpeedTreeVertexData
{
	// used for intermediate branch/frond LOD fading
	FLOAT			LODHint;
};

struct FSpeedTreeVertexDataLeafCard : public FSpeedTreeVertexData
{
	// for leaf cards:
	// width, height, pivot x, pivot y
	// angle x, angle y, angle index, corner

	FVector4		LeafData1;
	FVector4		LeafData2;
};

struct FSpeedTreeVertexDataLeafMesh : public FSpeedTreeVertexDataLeafCard
{
	// for leaf meshes:
	// offset x, y, z, angle x
	// orient up x, y, z, angle y
	// orient right x, y, z, angle index

	FVector4		LeafData3;
};

struct FSpeedTreeVertexDataBillboard : public FSpeedTreeVertexData
{
	FLOAT bIsVerticalBillboard;
	FLOAT BillboardIndex;
};


template<class VertexType>
class TSpeedTreeVertexBuffer : public FVertexBuffer
{
public:

	TArray<VertexType> Vertices;

	virtual void InitRHI()
	{
		const INT Size = Vertices.Num() * sizeof(VertexType);
		if(Vertices.Num())
		{
			// Recreate the vertex buffer to avoid stalls writing to a vertex buffer which is still being accessed by the GPU.
			VertexBufferRHI = RHICreateVertexBuffer(Size,NULL,RUF_Static);

			// Copy the vertex data into the vertex buffer.
			void* const Buffer = RHILockVertexBuffer(VertexBufferRHI,0,Size,FALSE);
			appMemcpy(Buffer,&Vertices(0),Size);
			RHIUnlockVertexBuffer(VertexBufferRHI);
		}
	}

	virtual void ReleaseRHI()
	{
		VertexBufferRHI.SafeRelease(); 
	}
	
	virtual FString GetFriendlyName(void) const	{ return TEXT("SpeedTree vertex buffer"); }
};

class FSpeedTreeResourceHelper
{
public:

	FSpeedTreeResourceHelper( class USpeedTree* InOwner );

	void Load( const BYTE* Buffer, INT NumBytes );

	void SetupBranchesOrFronds(UBOOL bBranches);
	void SetupLeaves();
	void SetupBillboards();
	void UpdateWind(const FVector& WindDirection, FLOAT WindStrength, FLOAT CurrentTime);
	void UpdateGeometry(const FVector& CurrentCameraOrigin,const FVector& CurrentCameraZ,UBOOL bApplyWind);

	void CleanUp();

	FBoxSphereBounds GetBounds();
	INT GetNumCollisionPrimitives();
	void GetCollisionPrimitive( INT Index, CSpeedTreeRT::ECollisionObjectType& Type, FVector& Position, FVector& Dimensions, FVector& EulerAngles );

	class USpeedTree*					Owner;

	/** TRUE if branches should be drawn. */
	UBOOL bHasBranches;

	/** TRUE if fronds should be drawn. */
	UBOOL bHasFronds;

	/** TRUE if leaves should be drawn. */
	UBOOL bHasLeaves;

	/** TRUE if any of the leaves are billboards. */
	UBOOL bHasBillboardLeaves;

	/** TRUE if the resources have been initialized. */
	UBOOL bIsInitialized;

	/** TRUE if the resources have been updated for the frame. */
	UBOOL bIsUpdated;

	CSpeedTreeRT*						SpeedTree;					// the speedtree class
	CSpeedWind							SpeedWind;					// SpeedWind object

	TSpeedTreeVertexBuffer<FSpeedTreeVertexPosition>	BranchFrondPositionBuffer;	// buffer for vertex position
	TSpeedTreeVertexBuffer<FSpeedTreeVertexDataBranchFrond>		BranchFrondDataBuffer;		// buffer for normals, texcoords, etc

	TSpeedTreeVertexBuffer<FSpeedTreeVertexPosition>	LeafCardPositionBuffer;			// buffer for leaf card position
	TSpeedTreeVertexBuffer<FSpeedTreeVertexDataLeafCard>	LeafCardDataBuffer;				// buffer for leaf card normals, texcoords, etc

	TSpeedTreeVertexBuffer<FSpeedTreeVertexPosition>	LeafMeshPositionBuffer;			// buffer for leaf mesh position
	TSpeedTreeVertexBuffer<FSpeedTreeVertexDataLeafMesh>	LeafMeshDataBuffer;				// buffer for leaf mesh normals, texcoords, etc

	TSpeedTreeVertexBuffer<FSpeedTreeVertexPosition>	BillboardPositionBuffer;	// buffer for leaves' and billboards' position
	TSpeedTreeVertexBuffer<FSpeedTreeVertexDataBillboard>	BillboardDataBuffer;		// buffer for leaves' and billboards' data 

	FSpeedTreeBranchVertexFactory		BranchFrondVertexFactory;	// vertex factory for branches and fronds
	FSpeedTreeLeafCardVertexFactory		LeafCardVertexFactory;		// vertex factory for the leaf cards
	FSpeedTreeLeafMeshVertexFactory		LeafMeshVertexFactory;		// vertex factory for the leaf meshes
	FSpeedTreeBillboardVertexFactory	BillboardVertexFactory;		// vertex factory for billboards

	FRawIndexBuffer						IndexBuffer;				// the index buffer (holds everything)

	TArray<FMeshElement>				BranchElements;				// sections to draw per branch LOD
	TArray<FMeshElement>				FrondElements;				// sections to draw per frond LOD
	TArray<FMeshElement>				LeafCardElements;			// sections to draw per leaf LOD
	TArray<FMeshElement>				LeafMeshElements;			// sections to draw per leaf LOD
	FMeshElement						BillboardElement;			// sections to draw per billboard (and horizontal billboard)

    FRenderCommandFence					ReleaseResourcesFence;

	FVector								CachedCameraOrigin;
	FVector								CachedCameraZ;
	FLOAT								LastWindTime;
	FLOAT								WindTimeOffset;

	/** The maximum ratio between local space coordinates and texture coordinates for the highest LOD of each mesh type the SpeedTree has.  Not valid unless bHasValidTexelFactors==TRUE. */
	FLOAT								TexelFactors[STMT_Max];

	/** TRUE if the cached texel factors above are valid. */
	UBOOL								bHasValidTexelFactors;

	FVector2D							LeafAngleScalars;

	FLOAT								BranchFadingDistance;
	FLOAT								FrondFadingDistance;
};

/** Reads a stream of triangle strip vertex indices and outputs the vertex indices for each triangle. */
template<typename IndexType>
class TTriangleStripReader
{
public:

	/** Initialization constructor. */
	TTriangleStripReader(const IndexType* FirstIndex,INT NumIndices):
		bHasUnreadTriangles(TRUE),
		bFlipWinding(FALSE),
		NextIndex(FirstIndex),
		LastIndex(FirstIndex + NumIndices - 1)
	{
		// Ensure the strip is at least a single triangle.
		check(NextIndex + 3 <= LastIndex);

		// Initialize the strip state with the first two vertices.
		SecondTriangleIndex = *NextIndex++;
		ThirdTriangleIndex = *NextIndex++;

		// Advance to the first triangle.
		Advance();
	}

	/** Reads the current triangle's indices. */
	void GetTriangleIndices(IndexType& OutI0,IndexType& OutI1,IndexType& OutI2) const
	{
		check(bHasUnreadTriangles);

		OutI0 = bFlipWinding ? SecondTriangleIndex : FirstTriangleIndex;
		OutI1 = bFlipWinding ? FirstTriangleIndex : SecondTriangleIndex;
		OutI2 = ThirdTriangleIndex;
	}

	/** @return TRUE if there are triangles remaining to read. */
	UBOOL HasUnreadTriangles() const
	{
		return bHasUnreadTriangles;
	}

	/** Advances to the next triangle in the strip. */
	void Advance()
	{
		// Read triangles until a non-degenerate triangle or the end of the indices is found.
		do 
		{
			// Check for the end of the indices.
			if(NextIndex <= LastIndex)
			{
				// Replace the last triangle's oldest index with the next index.
				FirstTriangleIndex = SecondTriangleIndex;
				SecondTriangleIndex = ThirdTriangleIndex;
				ThirdTriangleIndex = *NextIndex++;

				// Alternate the winding with every triangle.
				bFlipWinding = !bFlipWinding;
			}
			else
			{
				// Set the done flag when the end of the indices has been reached.
				bHasUnreadTriangles = FALSE;
			}
		}
		while(bHasUnreadTriangles &&
			// Check for degenerate triangles.
			(	FirstTriangleIndex == SecondTriangleIndex ||
				SecondTriangleIndex == ThirdTriangleIndex ||
				FirstTriangleIndex == ThirdTriangleIndex
				));
	}

private:

	IndexType FirstTriangleIndex;
	IndexType SecondTriangleIndex;
	IndexType ThirdTriangleIndex;
	BITFIELD bHasUnreadTriangles : 1;
	BITFIELD bFlipWinding : 1;

	const IndexType* NextIndex;
	const IndexType* const LastIndex;
};

/**
 * Reads the SpeedTree vertex data for a specific mesh type.
 * @param SRH - The SpeedTree resource data to access.
 * @param MeshType - The mesh type the vertex is in.
 * @param VertexIndex - The index of the vertex.
 */
const FSpeedTreeVertexData* GetSpeedTreeVertexData(const FSpeedTreeResourceHelper* SRH,INT MeshType,INT VertexIndex);

#endif // WITH_SPEEDTREE

#endif
