/*=============================================================================
	UnSkeletalTools.cpp: Skeletal mesh helper classes.

	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#include "EnginePrivate.h"
#include "UnNovodexSupport.h"
#include "UnMeshBuild.h"

//
//	SkeletalMesh_UVsEqual
//

static inline UBOOL SkeletalMesh_UVsEqual(const FMeshWedge& V1, const FMeshWedge& V2)
{
	if(Abs(V1.U - V2.U) > (1.0f / 1024.0f))
		return 0;

	if(Abs(V1.V - V2.V) > (1.0f / 1024.0f))
		return 0;

	return 1;
}

/** soft skin vertex with extra data about its source origins */
struct FSkinVertexMeta
{
	FSoftSkinVertex Vertex;
	/** optional set of influences */
	FVertexInfluence ExtraInfluence;
	WORD PointWedgeIdx;
};

//
//	AddSkinVertex
//

static INT AddSkinVertex(TArray<FSkinVertexMeta>& Vertices,FSkinVertexMeta& VertexMeta )
{
	FSoftSkinVertex& Vertex = VertexMeta.Vertex;
	FVertexInfluence& ExtraInfluence = VertexMeta.ExtraInfluence;

	for(UINT VertexIndex = 0;VertexIndex < (UINT)Vertices.Num();VertexIndex++)
	{
		FSkinVertexMeta&	OtherVertexMeta = Vertices(VertexIndex);
		FSoftSkinVertex&	OtherVertex = OtherVertexMeta.Vertex;		

		if(!PointsEqual(OtherVertex.Position,Vertex.Position))
			continue;

		if(Abs(Vertex.U - OtherVertex.U) > (1.0f / 1024.0f))
			continue;

		if(Abs(Vertex.V - OtherVertex.V) > (1.0f / 1024.0f))
			continue;

		if(!NormalsEqual( OtherVertex.TangentX, Vertex.TangentX))
			continue;

		if(!NormalsEqual(OtherVertex.TangentY, Vertex.TangentY))
			continue;

		if(!NormalsEqual(OtherVertex.TangentZ, Vertex.TangentZ))
			continue;

		UBOOL	InfluencesMatch = 1;
		for(UINT InfluenceIndex = 0;InfluenceIndex < MAX_INFLUENCES;InfluenceIndex++)
		{
			if( Vertex.InfluenceBones[InfluenceIndex] != OtherVertex.InfluenceBones[InfluenceIndex] || 
				Vertex.InfluenceWeights[InfluenceIndex] != OtherVertex.InfluenceWeights[InfluenceIndex])
			{
				InfluencesMatch = 0;
				break;
			}
		}
		if(!InfluencesMatch)
			continue;

		// also check the extra influences since we don't want to weld vertices which may later be swapped to
		// use an alternate set of weights
		const FVertexInfluence&	OtherVertexExtraInfluence = OtherVertexMeta.ExtraInfluence;
		for(UINT InfluenceIndex = 0;InfluenceIndex < MAX_INFLUENCES;InfluenceIndex++)
		{
			if( ExtraInfluence.Bones.InfluenceBones[InfluenceIndex] != OtherVertexExtraInfluence.Bones.InfluenceBones[InfluenceIndex] || 
				ExtraInfluence.Weights.InfluenceWeights[InfluenceIndex] != OtherVertexExtraInfluence.Weights.InfluenceWeights[InfluenceIndex])
			{
				InfluencesMatch = 0;
				break;
			}
		}
		if(!InfluencesMatch)
			continue;

		return VertexIndex;
	}

	return Vertices.AddItem(VertexMeta);
}

//
//	FindEdgeIndex
//

static INT FindEdgeIndex(TArray<FMeshEdge>& Edges,const TArray<FSoftSkinVertex>& Vertices,FMeshEdge& Edge)
{
	for(INT EdgeIndex = 0;EdgeIndex < Edges.Num();EdgeIndex++)
	{
		FMeshEdge&	OtherEdge = Edges(EdgeIndex);

		if(Vertices(OtherEdge.Vertices[0]).Position != Vertices(Edge.Vertices[1]).Position)
			continue;

		if(Vertices(OtherEdge.Vertices[1]).Position != Vertices(Edge.Vertices[0]).Position)
			continue;

		UBOOL	InfluencesMatch = 1;
		for(UINT InfluenceIndex = 0;InfluenceIndex < MAX_INFLUENCES;InfluenceIndex++)
		{
			if(Vertices(OtherEdge.Vertices[0]).InfluenceBones[InfluenceIndex] != Vertices(Edge.Vertices[1]).InfluenceBones[InfluenceIndex] ||
				Vertices(OtherEdge.Vertices[0]).InfluenceWeights[InfluenceIndex] != Vertices(Edge.Vertices[1]).InfluenceWeights[InfluenceIndex] ||
				Vertices(OtherEdge.Vertices[1]).InfluenceBones[InfluenceIndex] != Vertices(Edge.Vertices[0]).InfluenceBones[InfluenceIndex] ||
				Vertices(OtherEdge.Vertices[1]).InfluenceWeights[InfluenceIndex] != Vertices(Edge.Vertices[0]).InfluenceWeights[InfluenceIndex])
			{
				InfluencesMatch = 0;
				break;
			}
		}

		if(!InfluencesMatch)
			break;

		if(OtherEdge.Faces[1] != INDEX_NONE)
			continue;

		OtherEdge.Faces[1] = Edge.Faces[0];
		return EdgeIndex;
	}

	new(Edges) FMeshEdge(Edge);

	return Edges.Num() - 1;
}


// this is used for a sub-quadratic routine to find "equal" verts
struct FSkeletalMeshVertIndexAndZ
{
	INT Index;
	FLOAT Z;
};

// Sorting function for vertex Z/index pairs
IMPLEMENT_COMPARE_CONSTREF( FSkeletalMeshVertIndexAndZ, CreateSkinningStreamsFast, { if (A.Z < B.Z) return -1; if (A.Z > B.Z) return 1; return 0; } )


/**
* Create all render specific (but serializable) data like e.g. the 'compiled' rendering stream,
* mesh sections and index buffer.
*
* @todo: currently only handles LOD level 0.
*/
void USkeletalMesh::CreateSkinningStreams( 
	const TArray<FVertInfluence>& Influences, 
	const TArray<FMeshWedge>& Wedges, 
	const TArray<FMeshFace>& Faces, 
	const TArray<FVector>& Points,
	const FSkelMeshExtraInfluenceImportData* ExtraInfluenceData
	)
{
#if !CONSOLE
	UBOOL bTooManyVerts = FALSE;
	check( LODModels.Num() );
	FStaticLODModel& LODModel = LODModels(0);

	if( ExtraInfluenceData != NULL )
	{
		if( Wedges.Num() != ExtraInfluenceData->Wedges.Num() ||
			Faces.Num() != ExtraInfluenceData->Faces.Num() )
		{
			warnf(TEXT("Importing weights: Mismatched mesh data. Can not import mesh weights"));
			ExtraInfluenceData = NULL;
		}
	}

	// Allow multiple calls to CreateSkinningStreams for same model/LOD.

	LODModel.Sections.Empty();
	LODModel.Chunks.Empty();
	LODModel.ShadowIndices.Empty();
	LODModel.IndexBuffer.Indices.Empty();
	LODModel.Edges.Empty();
	LODModel.NumVertices = 0;
	LODModel.VertexInfluences.Empty();
	
	// Calculate face tangent vectors.

	TArray<FVector>	FaceTangentX(Faces.Num());
	TArray<FVector>	FaceTangentY(Faces.Num());

	for(INT FaceIndex = 0;FaceIndex < Faces.Num();FaceIndex++)
	{
		FVector	P1 = Points(Wedges(Faces(FaceIndex).iWedge[0]).iVertex),
				P2 = Points(Wedges(Faces(FaceIndex).iWedge[1]).iVertex),
				P3 = Points(Wedges(Faces(FaceIndex).iWedge[2]).iVertex);
		FVector	TriangleNormal = FPlane(P3,P2,P1);
		FMatrix	ParameterToLocal(
			FPlane(	P2.X - P1.X,	P2.Y - P1.Y,	P2.Z - P1.Z,	0	),
			FPlane(	P3.X - P1.X,	P3.Y - P1.Y,	P3.Z - P1.Z,	0	),
			FPlane(	P1.X,			P1.Y,			P1.Z,			0	),
			FPlane(	0,				0,				0,				1	)
			);

		FLOAT	U1 = Wedges(Faces(FaceIndex).iWedge[0]).U,
				U2 = Wedges(Faces(FaceIndex).iWedge[1]).U,
				U3 = Wedges(Faces(FaceIndex).iWedge[2]).U,
				V1 = Wedges(Faces(FaceIndex).iWedge[0]).V,
				V2 = Wedges(Faces(FaceIndex).iWedge[1]).V,
				V3 = Wedges(Faces(FaceIndex).iWedge[2]).V;

		FMatrix	ParameterToTexture(
			FPlane(	U2 - U1,	V2 - V1,	0,	0	),
			FPlane(	U3 - U1,	V3 - V1,	0,	0	),
			FPlane(	U1,			V1,			1,	0	),
			FPlane(	0,			0,			0,	1	)
			);

		FMatrix	TextureToLocal = ParameterToTexture.Inverse() * ParameterToLocal;
		FVector	TangentX = TextureToLocal.TransformNormal(FVector(1,0,0)).SafeNormal(),
				TangentY = TextureToLocal.TransformNormal(FVector(0,1,0)).SafeNormal(),
				TangentZ;

		TangentX = TangentX - TriangleNormal * (TangentX | TriangleNormal);
		TangentY = TangentY - TriangleNormal * (TangentY | TriangleNormal);

		FaceTangentX(FaceIndex) = TangentX.SafeNormal();
		FaceTangentY(FaceIndex) = TangentY.SafeNormal();
	}

	// Find wedge influences.

	TArray<INT>	WedgeInfluenceIndices;
	for(INT WedgeIndex = 0;WedgeIndex < Wedges.Num();WedgeIndex++)
	{
		for(UINT LookIdx = 0;LookIdx < (UINT)Influences.Num();LookIdx++)
		{
			if(Influences(LookIdx).VertIndex == Wedges(WedgeIndex).iVertex)
			{
				WedgeInfluenceIndices.AddItem(LookIdx);
				break;
			}
		}
	}
	check(Wedges.Num() == WedgeInfluenceIndices.Num());

	// update wedge influences for optional data
	// assumes that Wedges/Faces map one-to-one to ExtraInfluenceData->Wedges/Faces
	TArray<INT> ExtraWedgeInfluenceIndices;	
	if( ExtraInfluenceData != NULL )
	{
		for(INT WedgeIndex = 0;WedgeIndex < Wedges.Num();WedgeIndex++)
		{
			for(UINT LookIdx = 0;LookIdx < (UINT)ExtraInfluenceData->Influences.Num();LookIdx++)
			{
				if(ExtraInfluenceData->Influences(LookIdx).VertIndex == Wedges(WedgeIndex).iVertex)
				{
					ExtraWedgeInfluenceIndices.AddItem(LookIdx);
					break;
				}
			}
		}
		check(Wedges.Num() == ExtraWedgeInfluenceIndices.Num());		
	}

	// Calculate smooth wedge tangent vectors.

	GWarn->BeginSlowTask( *LocalizeUnrealEd("ProcessingSkeletalTriangles"), TRUE );


	// To accelerate generation of adjacency, we'll create a table that maps each vertex index
	// to its overlapping vertices, and a table that maps a vertex to the its influenced faces
	TMultiMap<INT,INT> Vert2Duplicates;
	TMultiMap<INT,INT> Vert2Faces;
	{
		// Create a list of vertex Z/index pairs
		TArray<FSkeletalMeshVertIndexAndZ> VertIndexAndZ;
		VertIndexAndZ.Empty(Points.Num());
		for (INT i = 0; i < Points.Num(); i++)
		{
			FSkeletalMeshVertIndexAndZ iandz;
			iandz.Index = i;
			iandz.Z = Points(i).Z;
			VertIndexAndZ.AddItem(iandz);
		}

		// Sort the vertices by z value
		Sort<USE_COMPARE_CONSTREF(FSkeletalMeshVertIndexAndZ, CreateSkinningStreamsFast)>( VertIndexAndZ.GetTypedData(), VertIndexAndZ.Num() );

		// Search for duplicates, quickly!
		for (INT i = 0; i < VertIndexAndZ.Num(); i++)
		{
			// only need to search forward, since we add pairs both ways
			for (INT j = i + 1; j < VertIndexAndZ.Num(); j++)
			{
				if (Abs(VertIndexAndZ(j).Z - VertIndexAndZ(i).Z) > THRESH_POINTS_ARE_SAME * 4.01f)
				{
					// our list is sorted, so there can't be any more dupes
					break;
				}

				// check to see if the points are really overlapping
				if(PointsEqual(
					Points(VertIndexAndZ(i).Index),
					Points(VertIndexAndZ(j).Index) ))					
				{
					Vert2Duplicates.Add(VertIndexAndZ(i).Index,VertIndexAndZ(j).Index);
					Vert2Duplicates.Add(VertIndexAndZ(j).Index,VertIndexAndZ(i).Index);
				}
			}
		}

		// we are done with this
		VertIndexAndZ.Empty();

		// now create a map from vert indices to faces
		for(INT FaceIndex = 0;FaceIndex < Faces.Num();FaceIndex++)
		{
			const FMeshFace&	Face = Faces(FaceIndex);
			for(INT VertexIndex = 0;VertexIndex < 3;VertexIndex++)
			{
				Vert2Faces.AddUnique(Wedges(Face.iWedge[VertexIndex]).iVertex,FaceIndex);
			}
		}
	}


	TArray<FRawIndexBuffer>	SectionIndexBufferArray;
	TArray< TArray<FSkinVertexMeta> > ChunkVerticesArray;
	TArray<INT> AdjacentFaces;
	TArray<INT> DupVerts;
	TArray<INT> DupFaces;

	for(INT FaceIndex = 0;FaceIndex < Faces.Num();FaceIndex++)
	{
		GWarn->StatusUpdatef( FaceIndex, Faces.Num(), *LocalizeUnrealEd("ProcessingSkeletalTriangles") );

		const FMeshFace&	Face = Faces(FaceIndex);

		FVector	VertexTangentX[3],
				VertexTangentY[3],
				VertexTangentZ[3];

        for(INT VertexIndex = 0;VertexIndex < 3;VertexIndex++)
		{
			VertexTangentX[VertexIndex] = FVector(0,0,0);
			VertexTangentY[VertexIndex] = FVector(0,0,0);
			VertexTangentZ[VertexIndex] = FVector(0,0,0);
		}

		FVector	TriangleNormal = FPlane(
			Points(Wedges(Face.iWedge[2]).iVertex),
			Points(Wedges(Face.iWedge[1]).iVertex),
			Points(Wedges(Face.iWedge[0]).iVertex)
			);
		FLOAT	Determinant = FTriple(FaceTangentX(FaceIndex),FaceTangentY(FaceIndex),TriangleNormal);

		// Start building a list of faces adjacent to this triangle
		AdjacentFaces.Reset();
		for(INT VertexIndex = 0;VertexIndex < 3;VertexIndex++)
		{
			INT vert = Wedges(Face.iWedge[VertexIndex]).iVertex;
			DupVerts.Reset();
			Vert2Duplicates.MultiFind(vert,DupVerts);
			DupVerts.AddItem(vert); // I am a "dupe" of myself
			for (INT k = 0; k < DupVerts.Num(); k++)
			{
				DupFaces.Reset();
				Vert2Faces.MultiFind(DupVerts(k),DupFaces);
				for (INT l = 0; l < DupFaces.Num(); l++)
				{
					AdjacentFaces.AddUniqueItem(DupFaces(l));
				}
			}
		}

		// Process adjacent faces
		for(INT AdjacentFaceIndex = 0;AdjacentFaceIndex < AdjacentFaces.Num();AdjacentFaceIndex++)
		{
			INT OtherFaceIndex = AdjacentFaces(AdjacentFaceIndex);
			const FMeshFace&	OtherFace = Faces(OtherFaceIndex);
			FVector		OtherTriangleNormal = FPlane(
							Points(Wedges(OtherFace.iWedge[2]).iVertex),
							Points(Wedges(OtherFace.iWedge[1]).iVertex),
							Points(Wedges(OtherFace.iWedge[0]).iVertex)
							);
			FLOAT		OtherFaceDeterminant = FTriple(FaceTangentX(OtherFaceIndex),FaceTangentY(OtherFaceIndex),OtherTriangleNormal);

			for(INT VertexIndex = 0;VertexIndex < 3;VertexIndex++)
			{
				for(INT OtherVertexIndex = 0;OtherVertexIndex < 3;OtherVertexIndex++)
				{
					if(PointsEqual(
						Points(Wedges(OtherFace.iWedge[OtherVertexIndex]).iVertex),
						Points(Wedges(Face.iWedge[VertexIndex]).iVertex)
						))					
					{
						if(Determinant * OtherFaceDeterminant > 0.0f && SkeletalMesh_UVsEqual(Wedges(OtherFace.iWedge[OtherVertexIndex]),Wedges(Face.iWedge[VertexIndex])))
						{
							VertexTangentX[VertexIndex] += FaceTangentX(OtherFaceIndex);
							VertexTangentY[VertexIndex] += FaceTangentY(OtherFaceIndex);
						}

						// Only contribute 'normal' if the vertices are truly one and the same to obey hard "smoothing" edges baked into 
						// the mesh by vertex duplication
						if( Wedges(OtherFace.iWedge[OtherVertexIndex]).iVertex == Wedges(Face.iWedge[VertexIndex]).iVertex ) 
						{
							VertexTangentZ[VertexIndex] += OtherTriangleNormal;
						}
					}
				}
			}
		}

		// Find a section which matches this triangle.
		FSkelMeshSection* Section = NULL;
		FSkelMeshChunk* Chunk = NULL;
		FRawIndexBuffer* SectionIndexBuffer = NULL;
		TArray<FSkinVertexMeta>* ChunkVertices = NULL;

		for(INT SectionIndex = 0;SectionIndex < LODModel.Sections.Num();SectionIndex++)
		{
			FSkelMeshSection& ExistingSection = LODModel.Sections(SectionIndex);
			FSkelMeshChunk& ExistingChunk = LODModel.Chunks(ExistingSection.ChunkIndex);
			if(ExistingSection.MaterialIndex == Face.MeshMaterialIndex)
			{
				// Count the number of bones this triangles uses which aren't yet used by the section.
				TArray<WORD> UniqueBones;
				for(INT VertexIndex = 0;VertexIndex < 3;VertexIndex++)
				{
					for(INT InfluenceIndex = WedgeInfluenceIndices(Face.iWedge[VertexIndex]);InfluenceIndex < Influences.Num();InfluenceIndex++)
					{
						const FVertInfluence& Influence = Influences(InfluenceIndex);
						if(Influence.VertIndex != Wedges(Face.iWedge[VertexIndex]).iVertex)
						{
							break;
						}
						if(ExistingChunk.BoneMap.FindItemIndex(Influence.BoneIndex) == INDEX_NONE)
						{
							UniqueBones.AddUniqueItem(Influence.BoneIndex);
						}
					}

					// update unique bones using wedge influences for optional data
					// assumes that Wedges/Faces map one-to-one to ExtraInfluenceData->Wedges/Faces
					if( ExtraInfluenceData != NULL &&
						ExtraWedgeInfluenceIndices.IsValidIndex(Face.iWedge[VertexIndex]) )
					{
						for( INT InfluenceIndex = ExtraWedgeInfluenceIndices(Face.iWedge[VertexIndex]);
							InfluenceIndex < ExtraInfluenceData->Influences.Num();
							InfluenceIndex++ )
						{
							const FVertInfluence& Influence = ExtraInfluenceData->Influences(InfluenceIndex);
							if(Influence.VertIndex != Wedges(Face.iWedge[VertexIndex]).iVertex)
							{
								break;
							}
							if(ExistingChunk.BoneMap.FindItemIndex(Influence.BoneIndex) == INDEX_NONE)
							{
								UniqueBones.AddUniqueItem(Influence.BoneIndex);
							}
						}
					}
				}

				if(ExistingChunk.BoneMap.Num() + UniqueBones.Num() <= MAX_GPUSKIN_BONES)
				{
					// This section has enough room in its bone table to fit the bones used by this triangle.
					Section = &ExistingSection;
					Chunk = &ExistingChunk;
					SectionIndexBuffer = &SectionIndexBufferArray(SectionIndex);
					ChunkVertices = &ChunkVerticesArray(ExistingSection.ChunkIndex);
					break;
				}
			}
		}

		if(!Section)
		{
			// Create a new skeletal mesh section.
			Section = new(LODModel.Sections) FSkelMeshSection;
			Section->MaterialIndex = Face.MeshMaterialIndex;	

			SectionIndexBuffer = new(SectionIndexBufferArray) FRawIndexBuffer;

			// Create a chunk for the section.
			Chunk = new(LODModel.Chunks) FSkelMeshChunk;
			Section->ChunkIndex = LODModel.Chunks.Num() - 1;
			ChunkVertices = new(ChunkVerticesArray) TArray<FSkinVertexMeta>();
		}

		WORD TriangleIndices[3];

		for(INT VertexIndex = 0;VertexIndex < 3;VertexIndex++)
		{
			FSoftSkinVertex	Vertex;
			FVector			TangentX = VertexTangentX[VertexIndex].SafeNormal(),
							TangentY = VertexTangentY[VertexIndex].SafeNormal(),
							TangentZ = VertexTangentZ[VertexIndex].SafeNormal();

			TangentY -= TangentX * (TangentX | TangentY);
			TangentY.Normalize();

			TangentX -= TangentZ * (TangentZ | TangentX);
			TangentY -= TangentZ * (TangentZ | TangentY);

			TangentX.Normalize();
			TangentY.Normalize();

			Vertex.Position = Points(Wedges(Face.iWedge[VertexIndex]).iVertex);
			Vertex.TangentX = TangentX;
			Vertex.TangentY = TangentY;
			Vertex.TangentZ = TangentZ;
			Vertex.U = Wedges(Face.iWedge[VertexIndex]).U;
			Vertex.V = Wedges(Face.iWedge[VertexIndex]).V;

			{
				// Count the influences.

				INT InfIdx = WedgeInfluenceIndices(Face.iWedge[VertexIndex]);
				INT LookIdx = InfIdx;

				UINT InfluenceCount = 0;
				while( Influences.IsValidIndex(LookIdx) && (Influences(LookIdx).VertIndex == Wedges(Face.iWedge[VertexIndex]).iVertex) )
				{			
					InfluenceCount++;
					LookIdx++;
				}
				InfluenceCount = Min<UINT>(InfluenceCount,MAX_INFLUENCES);

				// Setup the vertex influences.

				Vertex.InfluenceBones[0] = 0;
				Vertex.InfluenceWeights[0] = 255;
				for(UINT i = 1;i < MAX_INFLUENCES;i++)
				{
					Vertex.InfluenceBones[i] = 0;
					Vertex.InfluenceWeights[i] = 0;
				}

				UINT	TotalInfluenceWeight = 0;
				for(UINT i = 0;i < InfluenceCount;i++)
				{
					BYTE BoneIndex = (BYTE)Influences(InfIdx+i).BoneIndex;
					if( BoneIndex >= RefSkeleton.Num() )
						continue;

					LODModel.ActiveBoneIndices.AddUniqueItem(BoneIndex);
					Vertex.InfluenceBones[i] = Chunk->BoneMap.AddUniqueItem(BoneIndex);
					Vertex.InfluenceWeights[i] = (BYTE)(Influences(InfIdx+i).Weight * 255.0f);
					TotalInfluenceWeight += Vertex.InfluenceWeights[i];
				}
				Vertex.InfluenceWeights[0] += 255 - TotalInfluenceWeight;
			}

			// do the same thing for any extra vertex influences
			// add any extra bone indices which are needed to the active bones 
			FVertexInfluence ExtraInfluence;
			appMemzero(&ExtraInfluence,sizeof(ExtraInfluence));
			if( ExtraInfluenceData != NULL &&
				ExtraWedgeInfluenceIndices.IsValidIndex(Face.iWedge[VertexIndex]) )
			{
				// Count the influences.

				INT InfIdx = ExtraWedgeInfluenceIndices(Face.iWedge[VertexIndex]);
				INT LookIdx = InfIdx;

				UINT InfluenceCount = 0;
				while( ExtraInfluenceData->Influences.IsValidIndex(LookIdx) && 
					(ExtraInfluenceData->Influences(LookIdx).VertIndex == Wedges(Face.iWedge[VertexIndex]).iVertex) )
				{			
					InfluenceCount++;
					LookIdx++;
				}
				InfluenceCount = Min<UINT>(InfluenceCount,MAX_INFLUENCES);

				// Setup the vertex influences.

				ExtraInfluence.Bones.InfluenceBones[0] = 0;
				ExtraInfluence.Weights.InfluenceWeights[0] = 255;
				for(UINT i = 1;i < MAX_INFLUENCES;i++)
				{
					ExtraInfluence.Bones.InfluenceBones[i] = 0;
					ExtraInfluence.Weights.InfluenceWeights[i] = 0;
				}

				UINT	TotalInfluenceWeight = 0;
				for(UINT i = 0;i < InfluenceCount;i++)
				{
					BYTE BoneIndex = (BYTE)ExtraInfluenceData->Influences(InfIdx+i).BoneIndex;
					if( BoneIndex >= RefSkeleton.Num() )
						continue;

					LODModel.ActiveBoneIndices.AddUniqueItem(BoneIndex);
					ExtraInfluence.Bones.InfluenceBones[i] = Chunk->BoneMap.AddUniqueItem(BoneIndex);
					ExtraInfluence.Weights.InfluenceWeights[i] = (BYTE)(ExtraInfluenceData->Influences(InfIdx+i).Weight * 255.0f);
					TotalInfluenceWeight += ExtraInfluence.Weights.InfluenceWeights[i];
				}
				ExtraInfluence.Weights.InfluenceWeights[0] += 255 - TotalInfluenceWeight;
			}

			// Add the vertex as well as its original index in the points array
			FSkinVertexMeta VertexMeta = { Vertex, ExtraInfluence, Wedges(Face.iWedge[VertexIndex]).iVertex };

			INT	V = AddSkinVertex(*ChunkVertices,VertexMeta);

			// set the index entry for the newly added vertex
			// check(V >= 0 && V <= MAXWORD);
			if (V > MAXWORD)
			{
				bTooManyVerts = TRUE;
			}
			TriangleIndices[VertexIndex] = (WORD)V;
		}

		if(TriangleIndices[0] != TriangleIndices[1] && TriangleIndices[0] != TriangleIndices[2] && TriangleIndices[1] != TriangleIndices[2])
		{
			for(UINT VertexIndex = 0;VertexIndex < 3;VertexIndex++)
			{
				SectionIndexBuffer->Indices.AddItem(TriangleIndices[VertexIndex]);
			}
		}
	}

	// add to LODModel.VertexInfluences for extra influence data
	FSkeletalMeshVertexInfluences* ExtraVertInfluences = NULL;
	if( ExtraInfluenceData != NULL )
	{
		ExtraVertInfluences = new(LODModel.VertexInfluences) FSkeletalMeshVertexInfluences;
	}

	// Pack the chunk vertices into a single vertex buffer.
	TArray<FSoftSkinVertex> ShadowVertices;
	TArray< TArray<WORD> > VertexIndexRemap;
	TArray<WORD> RawPointIndices;
	LODModel.NumVertices = 0;
	for(INT ChunkIndex = 0;ChunkIndex < LODModel.Chunks.Num();ChunkIndex++)
	{
		FSkelMeshChunk& Chunk = LODModel.Chunks(ChunkIndex);
		TArray<FSkinVertexMeta>& ChunkVertices = ChunkVerticesArray(ChunkIndex);

		// Calculate the offset to this chunk's vertices in the vertex buffer.
		Chunk.BaseVertexIndex = LODModel.NumVertices;

		// Update the size of the vertex buffer.  Allocate space in the vertex buffer for two versions of each vertex: unextruded and extruded for shadowing.
		LODModel.NumVertices += ChunkVertices.Num();

		// Separate the section's vertices into rigid and soft vertices.
		TArray<WORD>* ChunkVertexIndexRemap = new(VertexIndexRemap) TArray<WORD>(ChunkVertices.Num());
		for(INT VertexIndex = 0;VertexIndex < ChunkVertices.Num();VertexIndex++)
		{
			const FSkinVertexMeta& VertexMeta = ChunkVertices(VertexIndex);
			const FSoftSkinVertex& SoftVertex = VertexMeta.Vertex;
			if(SoftVertex.InfluenceWeights[1] == 0)
			{
				FRigidSkinVertex RigidVertex;
				RigidVertex.Position = SoftVertex.Position;
				RigidVertex.TangentX = SoftVertex.TangentX;
				RigidVertex.TangentY = SoftVertex.TangentY;
				RigidVertex.TangentZ = SoftVertex.TangentZ;
				RigidVertex.U = SoftVertex.U;
				RigidVertex.V = SoftVertex.V;
				RigidVertex.Bone = SoftVertex.InfluenceBones[0];
				(*ChunkVertexIndexRemap)(VertexIndex) = (WORD)(Chunk.BaseVertexIndex + Chunk.RigidVertices.AddItem(RigidVertex));
				// add the index to the original wedge point source of this vertex
				RawPointIndices.AddItem( VertexMeta.PointWedgeIdx );

				FSoftSkinVertex ShadowVertex = SoftVertex;
				ShadowVertex.InfluenceBones[0] = Chunk.BoneMap(ShadowVertex.InfluenceBones[0]);
				ShadowVertices.AddItem(ShadowVertex);

				// add the optional vertex influence if it exists
				if( ExtraVertInfluences != NULL )
				{
					ExtraVertInfluences->Influences.AddItem(VertexMeta.ExtraInfluence);
				}
			}
		}
		for(INT VertexIndex = 0;VertexIndex < ChunkVertices.Num();VertexIndex++)
		{
			const FSkinVertexMeta& VertexMeta = ChunkVertices(VertexIndex);
			const FSoftSkinVertex& SoftVertex = VertexMeta.Vertex;
			if(SoftVertex.InfluenceWeights[1] > 0)
			{
				(*ChunkVertexIndexRemap)(VertexIndex) = (WORD)(Chunk.BaseVertexIndex + Chunk.RigidVertices.Num() + Chunk.SoftVertices.AddItem(SoftVertex));
				// add the index to the original wedge point source of this vertex
				RawPointIndices.AddItem( VertexMeta.PointWedgeIdx );

				FSoftSkinVertex ShadowVertex = SoftVertex;
				for(INT InfluenceIndex = 0;InfluenceIndex < MAX_INFLUENCES;InfluenceIndex++)
				{
					ShadowVertex.InfluenceBones[InfluenceIndex] = Chunk.BoneMap(ShadowVertex.InfluenceBones[InfluenceIndex]);
				}
				ShadowVertices.AddItem(ShadowVertex);

				// add the optional vertex influence if it exists
				if( ExtraVertInfluences != NULL )
				{
					ExtraVertInfluences->Influences.AddItem(VertexMeta.ExtraInfluence);
				}
			}
		}

		// update total num of verts added
		Chunk.NumRigidVertices = Chunk.RigidVertices.Num();
		Chunk.NumSoftVertices = Chunk.SoftVertices.Num();

		// update max bone influences
		Chunk.CalcMaxBoneInfluences();

		// Log info about the chunk.
		debugf(TEXT("Chunk %u: %u rigid vertices, %u soft vertices, %u active bones"),
			ChunkIndex,
			Chunk.RigidVertices.Num(),
			Chunk.SoftVertices.Num(),
			Chunk.BoneMap.Num()
			);
	}

	// Copy raw point indices to LOD model.
	LODModel.RawPointIndices.RemoveBulkData();
	if( RawPointIndices.Num() )
	{
		LODModel.RawPointIndices.Lock(LOCK_READ_WRITE);
		void* Dest = LODModel.RawPointIndices.Realloc( RawPointIndices.Num() );
		appMemcpy( Dest, RawPointIndices.GetData(), LODModel.RawPointIndices.GetBulkDataSize() );
		LODModel.RawPointIndices.Unlock();
	}

	// Finish building the sections.
	for(INT SectionIndex = 0;SectionIndex < LODModel.Sections.Num();SectionIndex++)
	{
		FSkelMeshSection& Section = LODModel.Sections(SectionIndex);
		FRawIndexBuffer& SectionIndexBuffer = SectionIndexBufferArray(SectionIndex);
		
		// Reorder the section index buffer for better vertex cache efficiency.
		SectionIndexBuffer.CacheOptimize();

		// Calculate the number of triangles in the section.  Note that CacheOptimize may change the number of triangles in the index buffer!
		Section.NumTriangles = SectionIndexBuffer.Indices.Num() / 3;

		// Calculate the offset to this section's indices in the index buffer.
		Section.BaseIndex = LODModel.IndexBuffer.Indices.Num();

		// Remap the section index buffer.
		for(INT Index = 0;Index < SectionIndexBuffer.Indices.Num();Index++)
		{
			WORD VertexIndex = VertexIndexRemap(Section.ChunkIndex)(SectionIndexBuffer.Indices(Index));
			LODModel.IndexBuffer.Indices.AddItem(VertexIndex);
			LODModel.ShadowIndices.AddItem(VertexIndex);
		}

		// Log info about the section.
		debugf(TEXT("Section %u: Material=%u, Chunk=%u, %u triangles"),
			SectionIndex,
			Section.MaterialIndex,
			Section.ChunkIndex,
			SectionIndexBuffer.Indices.Num() / 3
			);
	}

	// Build the edge list from the shadow indices
	FSkeletalMeshEdgeBuilder(LODModel.ShadowIndices,ShadowVertices,LODModel.Edges).FindEdges();

	GWarn->EndSlowTask();

	// Find mesh sections with open edges.

	TSet<INT>	SeparateTriangles;

	while(1)
	{
		INT	InitialSeparate = SeparateTriangles.Num();
		for(INT EdgeIndex = 0;EdgeIndex < LODModel.Edges.Num();EdgeIndex++)
		{
			FMeshEdge&	Edge = LODModel.Edges(EdgeIndex);
			if(Edge.Faces[1] == INDEX_NONE || SeparateTriangles.Contains(Edge.Faces[1]))
			{
				SeparateTriangles.Add(Edge.Faces[0]);
			}
			else if(SeparateTriangles.Contains(Edge.Faces[0]))
			{
				SeparateTriangles.Add(Edge.Faces[1]);
			}
		}
		if(SeparateTriangles.Num() == InitialSeparate)
			break;
	};

	LODModel.ShadowTriangleDoubleSided.Empty(LODModel.ShadowIndices.Num() / 3);
	LODModel.ShadowTriangleDoubleSided.AddZeroed(LODModel.ShadowIndices.Num() / 3);

	for(TSet<INT>::TIterator TriangleIt = TSet<INT>::TIterator(SeparateTriangles);TriangleIt;++TriangleIt)
	{
		LODModel.ShadowTriangleDoubleSided(*TriangleIt) = 1;
	}

	UINT NumOpenEdges = 0;
	for(INT EdgeIndex = 0;EdgeIndex < LODModel.Edges.Num();EdgeIndex++)
	{
		if(LODModel.Edges(EdgeIndex).Faces[1] == INDEX_NONE)
		{
			NumOpenEdges++;
		}
	}

	debugf(TEXT("%u double-sided, %u edges(%u open)"),SeparateTriangles.Num(),LODModel.Edges.Num(),NumOpenEdges);
	if (bTooManyVerts)
	{
		debugf(TEXT("Input mesh has too many vertices.  The generated mesh will be corrupt!"));
		appMsgf( AMT_OK,TEXT("Input mesh has too many vertices.  The generated mesh will be corrupt!  Consider adding extra materials to split up the source mesh into smaller chunks."));
	}
#else
	appErrorf(TEXT("Cannot call USkeletalMesh::CreateSkinningStreams on a console!"));
#endif
}


// Pre-calculate refpose-to-local transforms
void USkeletalMesh::CalculateInvRefMatrices()
{
	if( RefBasesInvMatrix.Num() != RefSkeleton.Num() )
	{
		RefBasesInvMatrix.Empty(RefSkeleton.Num());
		RefBasesInvMatrix.Add(RefSkeleton.Num());

		// Temporary storage for calculating mesh-space ref pose
		TArray<FMatrix> RefBases;
		RefBases.Add( RefSkeleton.Num() );

		// Precompute the Mesh.RefBasesInverse.
		for( INT b=0; b<RefSkeleton.Num(); b++)
		{
			// Make sure quaternions are normalized. It's a little scary that we can get ref skeletons not normalized!
			RefSkeleton(b).BonePos.Orientation.Normalize();
			// Render the default pose.
			RefBases(b) = GetRefPoseMatrix(b);

			// Construct mesh-space skeletal hierarchy.
			if( b>0 )
			{
				INT Parent = RefSkeleton(b).ParentIndex;
				RefBases(b) = RefBases(b) * RefBases(Parent);
			}

			// Precompute inverse so we can use from-refpose-skin vertices.
			RefBasesInvMatrix(b) = RefBases(b).Inverse(); 
		}
	}
}

// Find the most dominant bone for each vertex
static INT GetDominantBoneIndex(FSoftSkinVertex* SoftVert)
{
	BYTE MaxWeightBone = 0;
	BYTE MaxWeightWeight = 0;

	for(INT i=0; i<MAX_INFLUENCES; i++)
	{
		if(SoftVert->InfluenceWeights[i] > MaxWeightWeight)
		{
			MaxWeightWeight = SoftVert->InfluenceWeights[i];
			MaxWeightBone = SoftVert->InfluenceBones[i];
		}
	}

	return MaxWeightBone;
}

/**
 *	Calculate the verts associated weighted to each bone of the skeleton.
 *	The vertices returned are in the local space of the bone.
 *
 *	@param	Infos	The output array of vertices associated with each bone.
 *	@param	bOnlyDominant	Controls whether a vertex is added to the info for a bone if it is most controlled by that bone, or if that bone has ANY influence on that vert.
 */
void USkeletalMesh::CalcBoneVertInfos( TArray<FBoneVertInfo>& Infos, UBOOL bOnlyDominant )
{
	if( LODModels.Num() == 0)
		return;

	CalculateInvRefMatrices();
	check( RefSkeleton.Num() == RefBasesInvMatrix.Num() );

	Infos.Empty();
	Infos.AddZeroed( RefSkeleton.Num() );

	FStaticLODModel* LODModel = &LODModels(0);
	for(INT ChunkIndex = 0;ChunkIndex < LODModel->Chunks.Num();ChunkIndex++)
	{
		FSkelMeshChunk& Chunk = LODModel->Chunks(ChunkIndex);
		for(INT i=0; i<Chunk.RigidVertices.Num(); i++)
		{
			FRigidSkinVertex* RigidVert = &Chunk.RigidVertices(i);
			INT BoneIndex = Chunk.BoneMap(RigidVert->Bone);

			FVector LocalPos = RefBasesInvMatrix(BoneIndex).TransformFVector(RigidVert->Position);
			Infos(BoneIndex).Positions.AddItem(LocalPos);

			FVector LocalNormal = RefBasesInvMatrix(BoneIndex).TransformNormal(RigidVert->TangentZ);
			Infos(BoneIndex).Normals.AddItem(LocalNormal);
		}

		for(INT i=0; i<Chunk.SoftVertices.Num(); i++)
		{
			FSoftSkinVertex* SoftVert = &Chunk.SoftVertices(i);

			if(bOnlyDominant)
			{
				INT BoneIndex = Chunk.BoneMap(GetDominantBoneIndex(SoftVert));

				FVector LocalPos = RefBasesInvMatrix(BoneIndex).TransformFVector(SoftVert->Position);
				Infos(BoneIndex).Positions.AddItem(LocalPos);

				FVector LocalNormal = RefBasesInvMatrix(BoneIndex).TransformNormal(SoftVert->TangentZ);
				Infos(BoneIndex).Normals.AddItem(LocalNormal);
			}
			else
			{
				for(INT j=0; j<MAX_INFLUENCES; j++)
				{
					if(SoftVert->InfluenceWeights[j] > 0.01f)
					{
						INT BoneIndex = Chunk.BoneMap(SoftVert->InfluenceBones[j]);

						FVector LocalPos = RefBasesInvMatrix(BoneIndex).TransformFVector(SoftVert->Position);
						Infos(BoneIndex).Positions.AddItem(LocalPos);

						FVector LocalNormal = RefBasesInvMatrix(BoneIndex).TransformNormal(SoftVert->TangentZ);
						Infos(BoneIndex).Normals.AddItem(LocalNormal);
					}
				}
			}
		}
	}
}

/**
 * Find if one bone index is a child of another.
 * Note - will return FALSE if ChildBoneIndex is the same as ParentBoneIndex ie. must be strictly a child.
 */
UBOOL USkeletalMesh::BoneIsChildOf(INT ChildBoneIndex, INT ParentBoneIndex) const
{
	// Bones are in strictly increasing order.
	// So child must have an index greater than his parent.
	if( ChildBoneIndex <= ParentBoneIndex )
	{
		return FALSE;
	}

	INT BoneIndex = RefSkeleton(ChildBoneIndex).ParentIndex;
	while(1)
	{
		if( BoneIndex == ParentBoneIndex )
		{
			return TRUE;
		}

		if( BoneIndex == 0 )
		{
			return FALSE;
		}
		BoneIndex = RefSkeleton(BoneIndex).ParentIndex;
	}
}

/** Allocate and initialise bone mirroring table for this skeletal mesh. Default is source = destination for each bone. */
void USkeletalMesh::InitBoneMirrorInfo()
{
	SkelMirrorTable.Empty(RefSkeleton.Num());
	SkelMirrorTable.AddZeroed(RefSkeleton.Num());

	// By default, no bone mirroring, and source is ourself.
	for(INT i=0; i<SkelMirrorTable.Num(); i++)
	{
		SkelMirrorTable(i).SourceIndex = i;
	}
}

/** Utility for copying and converting a mirroring table from another SkeletalMesh. */
void USkeletalMesh::CopyMirrorTableFrom(USkeletalMesh* SrcMesh)
{
	// Do nothing if no mirror table in source mesh
	if(SrcMesh->SkelMirrorTable.Num() == 0)
	{
		return;
	}

	// First, allocate and default mirroring table.
	InitBoneMirrorInfo();

	// Keep track of which entries in the source we have already copied
	TArray<UBOOL> EntryCopied;
	EntryCopied.AddZeroed( SrcMesh->SkelMirrorTable.Num() );

	// Mirror table must always be size of ref skeleton.
	check(SrcMesh->SkelMirrorTable.Num() == SrcMesh->RefSkeleton.Num());

	// Iterate over each entry in the source mesh mirror table.
	// We assume that the src table is correct, and don't check for errors here (ie two bones using the same one as source).
	for(INT i=0; i<SrcMesh->SkelMirrorTable.Num(); i++)
	{
		if(!EntryCopied(i))
		{
			// Get name of source and dest bone for this entry in the source table.
			FName DestBoneName = SrcMesh->RefSkeleton(i).Name;
			INT SrcBoneIndex = SrcMesh->SkelMirrorTable(i).SourceIndex;
			FName SrcBoneName = SrcMesh->RefSkeleton(SrcBoneIndex).Name;
			BYTE FlipAxis = SrcMesh->SkelMirrorTable(i).BoneFlipAxis;

			// Look up bone names in target mesh (this one)
			INT DestBoneIndexTarget = MatchRefBone(DestBoneName);
			INT SrcBoneIndexTarget = MatchRefBone(SrcBoneName);

			// If both bones found, copy data to this mesh's mirror table.
			if( DestBoneIndexTarget != INDEX_NONE && SrcBoneIndexTarget != INDEX_NONE )
			{
				SkelMirrorTable(DestBoneIndexTarget).SourceIndex = SrcBoneIndexTarget;
				SkelMirrorTable(DestBoneIndexTarget).BoneFlipAxis = FlipAxis;


				SkelMirrorTable(SrcBoneIndexTarget).SourceIndex = DestBoneIndexTarget;
				SkelMirrorTable(SrcBoneIndexTarget).BoneFlipAxis = FlipAxis;

				// Flag entries as copied, so we don't try and do it again.
				EntryCopied(i) = TRUE;
				EntryCopied(SrcBoneIndex) = TRUE;
			}
		}
	}
}

/** Utility for copying and converting a mirroring table from another SkeletalMesh. */
void USkeletalMesh::ExportMirrorTable(TArray<FBoneMirrorExport> &MirrorExportInfo)
{
	// Do nothing if no mirror table in source mesh
	if( SkelMirrorTable.Num() == 0 )
	{
		return;
	}
	
	// Mirror table must always be size of ref skeleton.
	check(SkelMirrorTable.Num() == RefSkeleton.Num());

	MirrorExportInfo.Empty(SkelMirrorTable.Num());
	MirrorExportInfo.AddZeroed(SkelMirrorTable.Num());

	// Iterate over each entry in the source mesh mirror table.
	// We assume that the src table is correct, and don't check for errors here (ie two bones using the same one as source).
	for(INT i=0; i<SkelMirrorTable.Num(); i++)
	{
		MirrorExportInfo(i).BoneName		= RefSkeleton(i).Name;
		MirrorExportInfo(i).SourceBoneName	= RefSkeleton(SkelMirrorTable(i).SourceIndex).Name;
		MirrorExportInfo(i).BoneFlipAxis	= SkelMirrorTable(i).BoneFlipAxis;
	}
}


/** Utility for copying and converting a mirroring table from another SkeletalMesh. */
void USkeletalMesh::ImportMirrorTable(TArray<FBoneMirrorExport> &MirrorExportInfo)
{
	// Do nothing if no mirror table in source mesh
	if( MirrorExportInfo.Num() == 0 )
	{
		return;
	}

	// First, allocate and default mirroring table.
	InitBoneMirrorInfo();

	// Keep track of which entries in the source we have already copied
	TArray<UBOOL> EntryCopied;
	EntryCopied.AddZeroed( RefSkeleton.Num() );

	// Mirror table must always be size of ref skeleton.
	check(SkelMirrorTable.Num() == RefSkeleton.Num());

	// Iterate over each entry in the source mesh mirror table.
	// We assume that the src table is correct, and don't check for errors here (ie two bones using the same one as source).
	for(INT i=0; i<MirrorExportInfo.Num(); i++)
	{
		FName DestBoneName	= MirrorExportInfo(i).BoneName;
		INT DestBoneIndex	= MatchRefBone(DestBoneName);

		if( DestBoneIndex != INDEX_NONE && !EntryCopied(DestBoneIndex) )
		{
			FName SrcBoneName	= MirrorExportInfo(i).SourceBoneName;
			INT SrcBoneIndex	= MatchRefBone(SrcBoneName);
			BYTE FlipAxis		= MirrorExportInfo(i).BoneFlipAxis;

			// If both bones found, copy data to this mesh's mirror table.
			if( SrcBoneIndex != INDEX_NONE )
			{
				SkelMirrorTable(DestBoneIndex).SourceIndex = SrcBoneIndex;
				SkelMirrorTable(DestBoneIndex).BoneFlipAxis = FlipAxis;

				SkelMirrorTable(SrcBoneIndex).SourceIndex = DestBoneIndex;
				SkelMirrorTable(SrcBoneIndex).BoneFlipAxis = FlipAxis;

				// Flag entries as copied, so we don't try and do it again.
				EntryCopied(DestBoneIndex)	= TRUE;
				EntryCopied(SrcBoneIndex)	= TRUE;
			}
		}
	}
}

/** 
 *	Utility for checking that the bone mirroring table of this mesh is good.
 *	Return TRUE if mirror table is OK, false if there are problems.
 *	@param	ProblemBones	Output string containing information on bones that are currently bad.
 */
UBOOL USkeletalMesh::MirrorTableIsGood(FString& ProblemBones)
{
	TArray<INT>	BadBoneMirror;

	for(INT i=0; i<SkelMirrorTable.Num(); i++)
	{
		INT SrcIndex = SkelMirrorTable(i).SourceIndex;
		if( SkelMirrorTable(SrcIndex).SourceIndex != i)
		{
			BadBoneMirror.AddItem(i);
		}
	}

	if(BadBoneMirror.Num() > 0)
	{
		for(INT i=0; i<BadBoneMirror.Num(); i++)
		{
			INT BoneIndex = BadBoneMirror(i);
			FName BoneName = RefSkeleton(BoneIndex).Name;

			ProblemBones += FString::Printf( TEXT("%s (%d)\n"), *BoneName.ToString(), BoneIndex );
		}

		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


/** Uses the ClothBones array to analyze the graphics mesh and generate informaton needed to construct simulation mesh (ClothToGraphicsVertMap etc). */
void USkeletalMesh::BuildClothMapping()
{
	ClothToGraphicsVertMap.Empty();
	ClothIndexBuffer.Empty();

	FStaticLODModel* LODModel = &LODModels(0);

	// Make array of indices of bones whose verts are to be considered 'cloth'
	TArray<BYTE> ClothBoneIndices;
	for(INT i=0; i<ClothBones.Num(); i++)
	{
		INT BoneIndex = MatchRefBone(ClothBones(i));
		if(BoneIndex != INDEX_NONE)
		{
			check(BoneIndex < 255);
			ClothBoneIndices.AddItem( (BYTE)BoneIndex );
		}
	}
	
	// Add 'special' cloth bones. eg bones which are fixed to the physics asset
	for(INT i=0; i<ClothSpecialBones.Num(); i++)
	{
		INT BoneIndex = MatchRefBone(ClothSpecialBones(i).BoneName);
		if(BoneIndex != INDEX_NONE)
		{
			check(BoneIndex < 255);
			ClothBoneIndices.AddItem( (BYTE)BoneIndex );
		}
	}

	// Fail if no bones defined.
	if( ClothBoneIndices.Num() == 0)
	{
		return;
	}

	TArray<FVector> AllClothVertices;
	ClothWeldingMap.Empty();
	ClothWeldingDomain = 0;

	// Now we find all the verts that are part of the cloth (ie weighted at all to cloth bones)
	// and add them to the ClothToGraphicsVertMap.
	INT VertIndex = 0;
	for(INT ChunkIndex = 0;ChunkIndex < LODModel->Chunks.Num();ChunkIndex++)
	{
		FSkelMeshChunk& Chunk = LODModel->Chunks(ChunkIndex);

		for(INT i=0; i<Chunk.RigidVertices.Num(); i++)
		{
			FRigidSkinVertex& RV = Chunk.RigidVertices(i);
			if( ClothBoneIndices.ContainsItem(Chunk.BoneMap(RV.Bone)) )
			{
				ClothToGraphicsVertMap.AddItem(VertIndex);
				INT NewIndex = AllClothVertices.AddUniqueItem(RV.Position);
				ClothWeldingDomain = Max(ClothWeldingDomain, NewIndex+1);
				ClothWeldingMap.AddItem(NewIndex);
			}

			VertIndex++;
		}

		for(INT i=0; i<Chunk.SoftVertices.Num(); i++)
		{
			FSoftSkinVertex& SV = Chunk.SoftVertices(i);
			if( (SV.InfluenceWeights[0] > 0 && ClothBoneIndices.ContainsItem(Chunk.BoneMap(SV.InfluenceBones[0]))) ||
				(SV.InfluenceWeights[1] > 0 && ClothBoneIndices.ContainsItem(Chunk.BoneMap(SV.InfluenceBones[1]))) ||
				(SV.InfluenceWeights[2] > 0 && ClothBoneIndices.ContainsItem(Chunk.BoneMap(SV.InfluenceBones[2]))) ||
				(SV.InfluenceWeights[3] > 0 && ClothBoneIndices.ContainsItem(Chunk.BoneMap(SV.InfluenceBones[3]))) )
			{
				ClothToGraphicsVertMap.AddItem(VertIndex);
				INT NewIndex = AllClothVertices.AddUniqueItem(SV.Position);
				ClothWeldingDomain = Max(ClothWeldingDomain, NewIndex+1);
				ClothWeldingMap.AddItem(NewIndex);
			}

			VertIndex++;
		}
	}

	// This is the divider between the 'free' vertices, and those that will be fixed.
	NumFreeClothVerts = ClothToGraphicsVertMap.Num();

	// Bail out if no cloth verts found.
	if(NumFreeClothVerts == 0)
	{
		return;
	}

	// The vertex buffer will have all the cloth verts, and then all the fixed verts.

	// Iterate over triangles, finding connected non-cloth verts
	for(INT i=0; i<LODModel->IndexBuffer.Indices.Num(); i+=3)
	{
		WORD Index0 = LODModel->IndexBuffer.Indices(i+0);
		WORD Index1 = LODModel->IndexBuffer.Indices(i+1);
		WORD Index2 = LODModel->IndexBuffer.Indices(i+2);

		// Its a 'free' vert if its in the array, and before NumFreeClothVerts.
		INT Index0InClothVerts = ClothToGraphicsVertMap.FindItemIndex(Index0);
		UBOOL Index0IsCloth = (Index0InClothVerts != INDEX_NONE) && (Index0InClothVerts < NumFreeClothVerts);

		INT Index1InClothVerts = ClothToGraphicsVertMap.FindItemIndex(Index1);
		UBOOL Index1IsCloth = (Index1InClothVerts != INDEX_NONE) && (Index1InClothVerts < NumFreeClothVerts);

		INT Index2InClothVerts = ClothToGraphicsVertMap.FindItemIndex(Index2);
		UBOOL Index2IsCloth = (Index2InClothVerts != INDEX_NONE) && (Index2InClothVerts < NumFreeClothVerts);

		// If this is a triangle that should be part of the cloth mesh (at least one vert is 'free'), 
		// add it to the cloth index buffer.
		if( Index0IsCloth || Index1IsCloth || Index2IsCloth )
		{
			// If this vert is not free ...
			if(!Index0IsCloth)
			{
				// Add to ClothToGraphicsVertMap (if not already present), and then to index buffer.
				const INT VIndex = ClothToGraphicsVertMap.AddUniqueItem(Index0);		
				ClothIndexBuffer.AddItem(VIndex);

				if (ClothToGraphicsVertMap.Num() > ClothWeldingMap.Num())
				{
					ClothWeldingMap.AddItem(ClothWeldingDomain++);
				}
			}
			// If this vert is part of the cloth, we have its location in the overall vertex buffer
			else
			{
				ClothIndexBuffer.AddItem(Index0InClothVerts);
			}

			// We do vertex 2 now, to change the winding order (Novodex likes it this way).
			if(!Index2IsCloth)
			{
				INT VertIndex = ClothToGraphicsVertMap.AddUniqueItem(Index2);		
				ClothIndexBuffer.AddItem(VertIndex);

				if (ClothToGraphicsVertMap.Num() > ClothWeldingMap.Num())
				{
					ClothWeldingMap.AddItem(ClothWeldingDomain++);
				}

			}
			else
			{
				ClothIndexBuffer.AddItem(Index2InClothVerts);
			}

			// Repeat for vertex 1
			if(!Index1IsCloth)
			{
				INT VertIndex = ClothToGraphicsVertMap.AddUniqueItem(Index1);		
				ClothIndexBuffer.AddItem(VertIndex);

				if (ClothToGraphicsVertMap.Num() > ClothWeldingMap.Num())
				{
					ClothWeldingMap.AddItem(ClothWeldingDomain++);
				}
			}
			else
			{
				ClothIndexBuffer.AddItem(Index1InClothVerts);
			}
		}
	}

	// Check Welding map, if it's the identity then we don't need it at all
	// If it's not the identity then it means that the domain is bigger than range and thus the last value can't be the size of the domain
	if (bForceNoWelding || (ClothWeldingMap(ClothWeldingMap.Num()-1) == ClothWeldingMap.Num() - 1))
	{
		if (!bForceNoWelding)
		{
			check(ClothWeldingDomain == ClothWeldingMap.Num());
		}
		ClothWeldingMap.Empty();
		ClothWeldedIndices.Empty();
	}

	if (ClothWeldingMap.Num() > 0)
	{
		ClothWeldedIndices = ClothIndexBuffer;
		for (INT i = 0; i < ClothWeldedIndices.Num(); i++)
		{
			check(ClothWeldedIndices(i) < ClothWeldingMap.Num());
			ClothWeldedIndices(i) = ClothWeldingMap(ClothWeldedIndices(i));
		}
	}

	//Build tables of cloth vertices which are to be attached to bodies associated with the PhysicsAsset

	for(INT i=0; i<ClothSpecialBones.Num(); i++)
	{
		ClothSpecialBones(i).AttachedVertexIndices.Empty();

		INT BoneIndex = MatchRefBone(ClothSpecialBones(i).BoneName);
		if(BoneIndex != INDEX_NONE)
		{
			check(BoneIndex < 255);

			INT VertIndex = 0;
			for(INT ChunkIndex = 0;ChunkIndex < LODModel->Chunks.Num();ChunkIndex++)
			{
				FSkelMeshChunk& Chunk = LODModel->Chunks(ChunkIndex);

				for(INT j=0; j<Chunk.GetNumRigidVertices(); j++)
				{
					FRigidSkinVertex& RV = Chunk.RigidVertices(j);

					if(Chunk.BoneMap(RV.Bone) == BoneIndex)
					{
						//Map the graphics index to the unwelded cloth index.
						INT ClothVertexIndex = ClothToGraphicsVertMap.FindItemIndex(VertIndex);
						
						if(ClothVertexIndex != INDEX_NONE)
						{
							//We want the welded vertex index...
							if (ClothWeldingMap.Num() > 0)
							{
								ClothVertexIndex = ClothWeldingMap(ClothVertexIndex);
							}
							
							ClothSpecialBones(i).AttachedVertexIndices.AddItem(ClothVertexIndex);
						}
						//else the vertex is not in the cloth.
					}


					VertIndex++; 
				}

				//Only allow attachment to rigid skinned vertices.
				VertIndex += Chunk.GetNumSoftVertices();
			}
		}
	}
}

/* Build a mapping from a set of 3 indices(a triangle) to the location in the index buffer */
void USkeletalMesh::BuildClothTornTriMap()
{
	if( !bEnableClothTearing || (ClothTornTriMap.Num() != 0) || (ClothWeldingMap.Num() != 0) )
	{
		return;
	}

	if( NumFreeClothVerts == 0 )
	{
		return;
	}

	FStaticLODModel* LODModel = &LODModels(0);

	for(INT i=0; i<LODModel->IndexBuffer.Indices.Num(); i+=3)
	{
		WORD Index0 = LODModel->IndexBuffer.Indices(i+0);
		WORD Index1 = LODModel->IndexBuffer.Indices(i+1);
		WORD Index2 = LODModel->IndexBuffer.Indices(i+2);

		// Its a 'free' vert if its in the array, and before NumFreeClothVerts.
		INT Index0InClothVerts = ClothToGraphicsVertMap.FindItemIndex(Index0);
		UBOOL Index0IsCloth = (Index0InClothVerts != INDEX_NONE) && (Index0InClothVerts < NumFreeClothVerts);

		INT Index1InClothVerts = ClothToGraphicsVertMap.FindItemIndex(Index1);
		UBOOL Index1IsCloth = (Index1InClothVerts != INDEX_NONE) && (Index1InClothVerts < NumFreeClothVerts);

		INT Index2InClothVerts = ClothToGraphicsVertMap.FindItemIndex(Index2);
		UBOOL Index2IsCloth = (Index2InClothVerts != INDEX_NONE) && (Index2InClothVerts < NumFreeClothVerts);

		if(Index0IsCloth || Index1IsCloth || Index2IsCloth)
		{
			check(Index0InClothVerts < 0xffFF);
			check(Index1InClothVerts < 0xffFF);
			check(Index2InClothVerts < 0xffFF);

			QWORD i0 = Index0InClothVerts;
			QWORD i1 = Index1InClothVerts;
			QWORD i2 = Index2InClothVerts;

			QWORD PackedTri = i0 + (i1 << 16) + (i2 << 32);

			//check(!ClothTornTriMap.HasKey(PackedTri));
			ClothTornTriMap.Set(PackedTri, i);
		}
	}

}

UBOOL USkeletalMesh::IsOnlyClothMesh() const
{
	const FStaticLODModel* LODModel = &LODModels(0);
	if(LODModel == NULL)
		return FALSE;

	//Cache this?
	INT VertexCount = 0;
	for(INT ChunkIndex = 0;ChunkIndex < LODModel->Chunks.Num();ChunkIndex++)
	{
		const FSkelMeshChunk& Chunk = LODModel->Chunks(ChunkIndex);

		VertexCount += Chunk.GetNumRigidVertices();
		VertexCount += Chunk.GetNumSoftVertices();
	}

	if(VertexCount == NumFreeClothVerts)
		return TRUE;
	else
		return FALSE;
}

#if WITH_NOVODEX && !NX_DISABLE_CLOTH

/**
 * We must factor out the retrieval of the cloth positions so that we can use it to initialize the 
 * cloth buffers on cloth creation. Sadly we cant just use saveToDesc() on the cloth mesh because
 * it returns permuted cloth vertices.
*/
UBOOL USkeletalMesh::ComputeClothSectionVertices(TArray<FVector>& ClothSectionVerts, FLOAT InScale, UBOOL ForceNoWelding)
{
		// Build vertex buffer with _all_ verts in skeletal mesh.
	FStaticLODModel* LODModel = &LODModels(0);
	TArray<FVector>	ClothVerts;

	for(INT ChunkIndex = 0;ChunkIndex < LODModel->Chunks.Num();ChunkIndex++)
	{
		FSkelMeshChunk& Chunk = LODModel->Chunks(ChunkIndex);

		if(Chunk.GetNumRigidVertices() > 0)
		{
			for(INT VertIdx=0; VertIdx<Chunk.GetNumRigidVertices(); VertIdx++)
			{
				FGPUSkinVertexBase* SrcRigidVertex = LODModel->VertexBufferGPUSkin.GetVertexPtr(Chunk.GetRigidVertexBufferIndex()+VertIdx);
				ClothVerts.AddItem( SrcRigidVertex->Position * InScale * U2PScale );
			}
		}

		if(Chunk.GetNumSoftVertices() > 0)
		{
			for(INT VertIdx=0; VertIdx<Chunk.GetNumSoftVertices(); VertIdx++)
			{
				FGPUSkinVertexBase* SrcSoftVertex = LODModel->VertexBufferGPUSkin.GetVertexPtr(Chunk.GetSoftVertexBufferIndex()+VertIdx);
				ClothVerts.AddItem( SrcSoftVertex->Position * InScale * U2PScale );
			}
		}
	}

	if (ClothVerts.Num() == 0)
	{
		return FALSE;
	}

	// Build initial vertex buffer for cloth section - pulling in just the verts we want.

	if (ClothWeldingMap.Num() == 0 || ForceNoWelding)
	{
		if(ClothSectionVerts.Num() < ClothToGraphicsVertMap.Num())
		{
			ClothSectionVerts.Empty();
			ClothSectionVerts.AddZeroed(ClothToGraphicsVertMap.Num());
		}

		for(INT i=0; i<ClothToGraphicsVertMap.Num(); i++)
		{
			ClothSectionVerts(i) = ClothVerts(ClothToGraphicsVertMap(i));
		}
	}
	else
	{
		if(ClothSectionVerts.Num() < ClothWeldingMap.Num())
		{
			ClothSectionVerts.Empty();
			ClothSectionVerts.AddZeroed(ClothWeldingDomain);
		}

		for(INT i=0; i<ClothWeldingMap.Num(); i++)
		{
			INT Mapped = ClothWeldingMap(i);
			ClothSectionVerts(Mapped) = ClothVerts(ClothToGraphicsVertMap(i));
		}
	}

	return TRUE;
}

/** Get the cooked NxClothMesh for this mesh at the given scale. */
NxClothMesh* USkeletalMesh::GetClothMeshForScale(FLOAT InScale)
{
	check(ClothMesh.Num() == ClothMeshScale.Num());

	// Look to see if we already have this mesh at this scale.
	for(INT i=0; i<ClothMesh.Num(); i++)
	{
		if( Abs(InScale - ClothMeshScale(i)) < KINDA_SMALL_NUMBER )
		{
			return (NxClothMesh*)ClothMesh(i);
		}
	}

	// If we have no info about cloth mesh - we can't do anything.
	// This would be generated using BuildClothMapping.
	if(ClothToGraphicsVertMap.Num() == 0)
	{
		return NULL;
	}

	TArray<FVector> ClothSectionVerts;

	if(!ComputeClothSectionVertices(ClothSectionVerts, InScale))
	{
		return NULL;
	}

	// Fill in cloth description
	NxClothMeshDesc Desc;
	Desc.points = ClothSectionVerts.GetData();
	Desc.numVertices = ClothSectionVerts.Num();
	Desc.pointStrideBytes = sizeof(FVector);

	if (ClothWeldingMap.Num() == 0)
	{
		Desc.triangles = ClothIndexBuffer.GetData();
		Desc.numTriangles = ClothIndexBuffer.Num()/3;
		Desc.triangleStrideBytes = 3*sizeof(INT);
	}
	else
	{
		check(ClothWeldedIndices.Num() > 0);
		Desc.triangles = ClothWeldedIndices.GetData();
		Desc.numTriangles = ClothWeldedIndices.Num()/3;
		Desc.triangleStrideBytes = 3*sizeof(INT);
	}
	
	if(bEnableClothTearing && (ClothWeldingMap.Num() == 0))
	{
		Desc.flags |= NX_CLOTH_MESH_TEARABLE;

	}

	// Cook mesh
	TArray<BYTE> TempData;
	FNxMemoryBuffer Buffer(&TempData);
	GNovodexCooking->NxCookClothMesh(Desc, Buffer);
	NxClothMesh* NewClothMesh = GNovodexSDK->createClothMesh(Buffer);

	ClothMesh.AddItem( NewClothMesh );
	ClothMeshScale.AddItem( InScale );

	//Make sure we have built the data needed for tearing.
	//Could do this during editing if we had proper serialization for TMaps(ie in BuildClothMapping().

	BuildClothTornTriMap();

	return NewClothMesh;
}
#endif

/** Reset the store of cooked cloth meshes. Need to make sure you are not actually using any when you call this. */
void USkeletalMesh::ClearClothMeshCache()
{
#if WITH_NOVODEX && !NX_DISABLE_CLOTH
	for (INT i = 0; i < ClothMesh.Num(); i++)
	{
		NxClothMesh* CM = (NxClothMesh*)ClothMesh(i);
		check(CM);
		GNovodexPendingKillClothMesh.AddItem(CM);
	}
	ClothMesh.Empty();
	ClothMeshScale.Empty();

	ClothTornTriMap.Empty();

#endif // WITH_NOVODEX
}


////// SKELETAL MESH THUMBNAIL SUPPORT ////////

/** 
 * Returns a one line description of an object for viewing in the thumbnail view of the generic browser
 */
FString USkeletalMesh::GetDesc()
{
	check(LODModels.Num() > 0);
	return FString::Printf( TEXT("%d Triangles, %d Bones"), LODModels(0).GetTotalFaces(), RefSkeleton.Num() );
}

/** 
 * Returns detailed info to populate listview columns
 */
FString USkeletalMesh::GetDetailedDescription( INT InIndex )
{
	FString Description = TEXT( "" );
	switch( InIndex )
	{
	case 0:
		Description = FString::Printf( TEXT( "%d Triangles" ), LODModels(0).GetTotalFaces() );
		break;
	case 1: 
		Description = FString::Printf( TEXT( "%d Bones" ), RefSkeleton.Num() );
		break;
	}
	return( Description );
}

