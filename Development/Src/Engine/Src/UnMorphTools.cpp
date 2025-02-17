/*=============================================================================
	UnMorphTools.cpp: Morph target creation helper classes.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#include "EnginePrivate.h"
#include "UnMeshBuild.h"

/** compare based on base mesh source vertex indices */
static int CDECL CompareMorphVertex(const void *A, const void *B)
{
	return( ((FMorphTargetVertex*)A)->SourceIdx - ((FMorphTargetVertex*)B)->SourceIdx );
}

/**
* Generate the streams for this morph target mesh using
* a base mesh and a target mesh to find the positon differences
* and other vertex attributes.
*
* @param	BaseSource - source mesh for comparing position differences
* @param	TargetSource - final target vertex positions/attributes 
* @param	LODIndex - level of detail to use for the geometry
*/
void UMorphTarget::CreateMorphMeshStreams( const FMorphMeshRawSource& BaseSource, const FMorphMeshRawSource& TargetSource, INT LODIndex )
{
	check(BaseSource.IsValidTarget(TargetSource));

	const FLOAT CLOSE_TO_ZERO_DELTA = THRESH_POINTS_ARE_SAME * 4.f;

	// create the LOD entry if it doesn't already exist
	if( LODIndex == MorphLODModels.Num() )
	{
		new(MorphLODModels) FMorphTargetLODModel();
	}
	
	// morph mesh data to modify
	FMorphTargetLODModel& MorphModel = MorphLODModels(LODIndex);

	// set the original number of vertices
	MorphModel.NumBaseMeshVerts = BaseSource.Vertices.Num();

	// empty morph mesh vertices first
	MorphModel.Vertices.Empty();

	// array to mark processed base vertices
	TArray<UBOOL> WasProcessed;
	WasProcessed.Empty(BaseSource.Vertices.Num());
	WasProcessed.AddZeroed(BaseSource.Vertices.Num());

	// iterate over all the base mesh indices
    for( INT Idx=0; Idx < BaseSource.Indices.Num(); Idx++ )
	{
		WORD BaseVertIdx = BaseSource.Indices(Idx);

		// check for duplicate processing
		if( !WasProcessed(BaseVertIdx) )
		{
			// mark this base vertex as already processed
			WasProcessed(BaseVertIdx) = TRUE;

			// get base mesh vertex using its index buffer
			const FMorphMeshVertexRaw& VBase = BaseSource.Vertices(BaseVertIdx);
			// get the base mesh's original wedge point index
			WORD BasePointIdx = BaseSource.WedgePointIndices(BaseVertIdx);

			// find the matching target vertex by searching for one
			// that has the same wedge point index
			INT TargetVertIdx = TargetSource.WedgePointIndices.FindItemIndex( BasePointIdx );

			// only add the vertex if the source point was found
			if( TargetVertIdx != INDEX_NONE )
			{
				// get target mesh vertex using its index buffer
				const FMorphMeshVertexRaw& VTarget = TargetSource.Vertices(TargetVertIdx);

				// change in position from base to target
				FVector PositionDelta( VTarget.Position - VBase.Position );

				// check if position actually changed much
				if( PositionDelta.Size() > CLOSE_TO_ZERO_DELTA )
				{
					// create a new entry
					FMorphTargetVertex NewVertex;
					// position delta
					NewVertex.PositionDelta = PositionDelta;
					// normal delta
					NewVertex.TangentZDelta = VTarget.TanZ - VBase.TanZ;
					// index of base mesh vert this entry is to modify
					NewVertex.SourceIdx = BaseVertIdx;

					// add it to the list of changed verts
					MorphModel.Vertices.AddItem( NewVertex );				
				}
			}			
		}
	}

	// sort the array of vertices for this morph target based on the base mesh indices 
	// that each vertex is associated with. This allows us to sequentially traverse the list
	// when applying the morph blends to each vertex.
	appQsort( &MorphModel.Vertices(0), MorphModel.Vertices.Num(), sizeof(FMorphTargetVertex), CompareMorphVertex );

	// remove array slack
	MorphModel.Vertices.Shrink();    
}

/**
* Constructor. 
* Converts a skeletal mesh to raw vertex data
* needed for creating a morph target mesh
*
* @param	SrcMesh - source skeletal mesh to convert
* @param	LODIndex - level of detail to use for the geometry
*/
FMorphMeshRawSource::FMorphMeshRawSource( USkeletalMesh* SrcMesh, INT LODIndex ) :
	SourceMesh(SrcMesh)
{
    check(SrcMesh);
	check(SrcMesh->LODModels.IsValidIndex(LODIndex));

	// get the mesh data for the given lod
	FStaticLODModel& LODModel = SrcMesh->LODModels(LODIndex);

	// vertices are packed in this order iot stay consistent
	// with the indexing used by the FStaticLODModel vertex buffer
	//
	//	Chunk0
	//		Rigid0
	//		Rigid1
	//		Soft0
	//		Soft1
	//	Chunk1
	//		Rigid0
	//		Rigid1
	//		Soft0
	//		Soft1
    
	// iterate over the chunks for the skeletal mesh
	for( INT ChunkIdx=0; ChunkIdx < LODModel.Chunks.Num(); ChunkIdx++ )
	{
		// each chunk has both rigid and smooth vertices
		const FSkelMeshChunk& Chunk = LODModel.Chunks(ChunkIdx);
		// rigid vertices should always be added first so that we are
		// consistent with the way vertices are packed in the FStaticLODModel vertex buffer
		for( INT VertexIdx=0; VertexIdx < Chunk.RigidVertices.Num(); VertexIdx++ )
		{
			const FRigidSkinVertex& SourceVertex = Chunk.RigidVertices(VertexIdx);
			FMorphMeshVertexRaw RawVertex = 
			{
                SourceVertex.Position,
				SourceVertex.TangentX,
				SourceVertex.TangentY,
				SourceVertex.TangentZ
			};
			Vertices.AddItem( RawVertex );			
		}
		// smooth vertices are added next. The resulting Vertices[] array should
		// match the FStaticLODModel vertex buffer when indexing vertices
		for( INT VertexIdx=0; VertexIdx < Chunk.SoftVertices.Num(); VertexIdx++ )
		{
			const FSoftSkinVertex& SourceVertex = Chunk.SoftVertices(VertexIdx);
			FMorphMeshVertexRaw RawVertex = 
			{
                SourceVertex.Position,
				SourceVertex.TangentX,
				SourceVertex.TangentY,
				SourceVertex.TangentZ
			};
			Vertices.AddItem( RawVertex );			
		}		
	}

    // Copy the indices manually, since the LODModel's index buffer may have a different alignment.
	Indices.Empty(LODModel.IndexBuffer.Indices.Num());
	for(INT Index = 0;Index < LODModel.IndexBuffer.Indices.Num();Index++)
	{
		Indices.AddItem(LODModel.IndexBuffer.Indices(Index));
	}
    
	// copy the wedge point indices
	if( LODModel.RawPointIndices.GetBulkDataSize() )
	{
		WedgePointIndices.Empty( LODModel.RawPointIndices.GetElementCount() );
		WedgePointIndices.Add( LODModel.RawPointIndices.GetElementCount() );
		appMemcpy( WedgePointIndices.GetData(), LODModel.RawPointIndices.Lock(LOCK_READ_ONLY), LODModel.RawPointIndices.GetBulkDataSize() );
		LODModel.RawPointIndices.Unlock();
	}
}

/**
* Constructor. 
* Converts a static mesh to raw vertex data
* needed for creating a morph target mesh
*
* @param	SrcMesh - source static mesh to convert
* @param	LODIndex - level of detail to use for the geometry
*/
FMorphMeshRawSource::FMorphMeshRawSource( UStaticMesh* SrcMesh, INT LODIndex ) :
	SourceMesh(SrcMesh)
{
	// @todo - not implemented
	// not sure if we will support static mesh morphing yet
}

/**
* Return true if current vertex data can be morphed to the target vertex data
* 
*/
UBOOL FMorphMeshRawSource::IsValidTarget( const FMorphMeshRawSource& Target ) const
{
	//@todo sz -
	// heuristic is to check for the same number of original points
	//return( WedgePointIndices.Num() == Target.WedgePointIndices.Num() );
	return TRUE;
}

