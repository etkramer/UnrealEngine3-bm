/*=============================================================================
	UnMorphMesh.h: Unreal morph target mesh objects.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/**
* Morph mesh vertex data used for comparisons and importing
*/
struct FMorphMeshVertexRaw
{
	FVector Position;
	FVector TanX,TanY,TanZ;
};

/**
* Converts a mesh to raw vertex data used to generate a morph target mesh
*/
class FMorphMeshRawSource
{
public:
	/** vertex data used for comparisons */
    TArray<FMorphMeshVertexRaw> Vertices;
	/** index buffer used for comparison */
	TArray<WORD> Indices;
	/** indices to original imported wedge points */
	TArray<WORD> WedgePointIndices;
	/** mesh that provided the source data */
	UObject* SourceMesh;

	/** Constructor (default) */
	FMorphMeshRawSource() : 
	SourceMesh(NULL)
	{
	}

	FMorphMeshRawSource( USkeletalMesh* SrcMesh, INT LODIndex=0 );
	FMorphMeshRawSource( UStaticMesh* SrcMesh, INT LODIndex=0 );

	UBOOL IsValidTarget( const FMorphMeshRawSource& Target ) const;
};

/** 
* Morph mesh vertex data used for rendering
*/
struct FMorphTargetVertex
{
	/** change in position */
	FVector			PositionDelta;
	/** change in tangent basis normal */
	FPackedNormal	TangentZDelta;

	/** index of source vertex to apply deltas to */
	WORD			SourceIdx;

	/** pipe operator */
	friend FArchive& operator<<( FArchive& Ar, FMorphTargetVertex& V )
	{
		Ar << V.PositionDelta << V.TangentZDelta << V.SourceIdx;
		return Ar;
	}
};

/**
* Mesh data for a single LOD model of a morph target
*/
struct FMorphTargetLODModel
{
	/** vertex data for a single LOD morph mesh */
    TArray<FMorphTargetVertex> Vertices;
	/** number of original verts in the base mesh */
	INT NumBaseMeshVerts;

	/** pipe operator */
	friend FArchive& operator<<( FArchive& Ar, FMorphTargetLODModel& M )
	{
        Ar << M.Vertices << M.NumBaseMeshVerts;
		return Ar;
	}
};

/**
* Morph target mesh 
*/
class UMorphTarget : public UObject
{
	DECLARE_CLASS( UMorphTarget, UObject, CLASS_SafeReplace|CLASS_NoExport, Engine )

	/** array of morph model mesh data for each LOD */
	TArray<FMorphTargetLODModel> MorphLODModels;

	// Object interface.
	void Serialize( FArchive& Ar );
	virtual void PostLoad();
	
	// creation routines - not used at runtime
	void CreateMorphMeshStreams( const FMorphMeshRawSource& BaseSource, const FMorphMeshRawSource& TargetSource, INT LODIndex );

	/**
	 * Returns the size of the object/ resource for display to artists/ LDs in the Editor.
	 *
	 * @return size of resource as to be displayed to artists/ LDs in the Editor.
	 */
	INT GetResourceSize();
};
