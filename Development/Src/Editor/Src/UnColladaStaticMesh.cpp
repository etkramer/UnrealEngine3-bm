/*=============================================================================
	COLLADA skeletal mesh helper.
	Based on top of the Feeling Software's Collada import classes [FCollada].
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.

	Class implementation inspired by the code of Richard Stenson, SCEA R&D
==============================================================================*/

#include "EditorPrivate.h"
#include "EngineAnimClasses.h"
#include "UnCollada.h"
#include "UnColladaExporter.h"
#include "UnColladaImporter.h"
#include "UnColladaSkeletalMesh.h"
#include "UnColladaStaticMesh.h"

namespace UnCollada {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CStaticMesh - COLLADA Static Mesh Importer
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CStaticMesh::CStaticMesh(CImporter* _BaseImporter)
	: BaseImporter( _BaseImporter ), BaseExporter(NULL)
{
}

CStaticMesh::CStaticMesh(CExporter* _BaseExporter)
	: BaseImporter(NULL), BaseExporter(_BaseExporter)
{
}

CStaticMesh::~CStaticMesh()
{
	BaseImporter = NULL;
	BaseExporter = NULL;
}

/*
 * Imports the polygonal elements of a given COLLADA mesh into the given UE3 rendering mesh
 */
void CStaticMesh::ImportStaticMeshPolygons(FStaticMeshRenderData* RenderMesh, const FCDGeometryInstance* ColladaInstance, const FCDGeometryMesh* ColladaMesh)
{
	// Create the smoothing groups from the COLLADA normals.
	UINTArray PolygonsSmoothingGroups;
	CreateSmoothingGroups(ColladaMesh, PolygonsSmoothingGroups);
	UINT PolygonsOffset = 0;

	// Initialize the element count and the basic elements for this static mesh. 
	uint32 MeshElementCount = ColladaMesh->GetPolygonsCount();
	for (uint32 ElementIndex = 0; ElementIndex < MeshElementCount; ++ElementIndex)
	{
		const FCDGeometryPolygons* ColladaPolygons = ColladaMesh->GetPolygons(ElementIndex);
		UMaterialInterface* EngineMaterial = BaseImporter->FindMaterialInstance(ColladaInstance, ColladaPolygons);
		const INT NewMaterialIndex = RenderMesh->Elements.Num();
		FStaticMeshElement* Element = new(RenderMesh->Elements) FStaticMeshElement(EngineMaterial,NewMaterialIndex);
		Element->Name = Element->Material->GetName(); // Necessary?

		// Generate the list of empty rendering triangles for these polygons.
		// Simple triangulation for now. If all polygons are relatively flat and convex, there should be no major issue.
		// If the user wants his original triangulation, he can use the correct export option in his exporter!
		const uint32* VertexCounts = ColladaPolygons->GetFaceVertexCounts();
		const INT PolygonCount = (INT) ColladaPolygons->GetFaceVertexCountCount();

		// Assemble triangle count.
		INT NewTriangleCount = 0;
		for (INT PolygonIndex = 0; PolygonIndex < PolygonCount; ++PolygonIndex)
		{
			const INT PolygonTriangleCount	= ((INT) VertexCounts[PolygonIndex]) - 2;
			NewTriangleCount				+= PolygonTriangleCount;
		}

		// Allocate new triangles.
		RenderMesh->RawTriangles.Lock( LOCK_READ_WRITE );
		const INT TriangleOffset		= RenderMesh->RawTriangles.GetElementCount();
		FStaticMeshTriangle* Base	= (FStaticMeshTriangle*) RenderMesh->RawTriangles.Realloc( TriangleOffset+NewTriangleCount );

		// Fill in new triangle data.
		INT NewTriangleIndex = TriangleOffset;
		for (INT PolygonIndex = 0; PolygonIndex < PolygonCount; ++PolygonIndex)
		{
			const INT PolygonTriangleCount	= ((INT) VertexCounts[PolygonIndex]) - 2;
			const INT NumTriangles			= RenderMesh->RawTriangles.GetElementCount();
			for (INT PolygonTriangleIndex = 0; PolygonTriangleIndex < PolygonTriangleCount; ++PolygonTriangleIndex)
			{
				//FStaticMeshTriangle* Triangle = new(RenderMesh->RawTriangles) FStaticMeshTriangle;
				FStaticMeshTriangle& Triangle = Base[NewTriangleIndex++];
				Triangle.SmoothingMask = PolygonsSmoothingGroups(PolygonsOffset + PolygonIndex);
				Triangle.MaterialIndex = ElementIndex;
				Triangle.FragmentIndex = 0;
				Triangle.NumUVs = 0;
				Triangle.bOverrideTangentBasis = FALSE;
				for( INT NormalIndex = 0; NormalIndex < 3; ++NormalIndex )
				{
					Triangle.TangentX[ NormalIndex ] = FVector( 0.0f, 0.0f, 0.0f );
					Triangle.TangentY[ NormalIndex ] = FVector( 0.0f, 0.0f, 0.0f );
					Triangle.TangentZ[ NormalIndex ] = FVector( 0.0f, 0.0f, 0.0f );
				}
			}
		}
		check( NewTriangleIndex == TriangleOffset+NewTriangleCount );
		const INT TriangleCount = RenderMesh->RawTriangles.GetElementCount();
		RenderMesh->RawTriangles.Unlock();
		PolygonsOffset += PolygonCount;

		INT TexCoordCount = 0;

		// Import into the triangles the pure tessellation data.
		// I'm therefore un-indexing everything here.
		// Triangulation is done by fanning the polygons, with the assumptions described above.
		for ( UINT CurInputIndex = 0; CurInputIndex < ColladaPolygons->GetInputCount(); ++CurInputIndex )
		{
			const FCDGeometryPolygonsInput* PolygonsInput = ColladaPolygons->GetInput( CurInputIndex );
			const uint32* InputIndices = PolygonsInput->GetIndices();
			if (InputIndices == NULL || PolygonsInput->GetSource() == NULL)
			{
				continue;
			}

			const FCDParameterListAnimatableFloat& SourceData = PolygonsInput->GetSource()->GetSourceData();
			UINT Stride = (UINT) PolygonsInput->GetSource()->GetStride();

			FStaticMeshTriangle* RawTriangleData = (FStaticMeshTriangle*) RenderMesh->RawTriangles.Lock(LOCK_READ_WRITE);
 /*&RenderMesh->RawTriangles(TriangleIndex++); \*/
#			define FOREACH_TRIANGLE { \
				INT TriangleIndex = TriangleOffset, Offset = 0; \
				for (INT PolygonIndex = 0; PolygonIndex < PolygonCount && TriangleIndex < TriangleCount; ++PolygonIndex) { \
					INT PolygonTriangleCount = ((INT) VertexCounts[PolygonIndex]) - 2; \
					for (INT PolygonTriangleIndex = 0; PolygonTriangleIndex < PolygonTriangleCount && TriangleIndex < TriangleCount; ++PolygonTriangleIndex) { \
						FStaticMeshTriangle* Triangle = &RawTriangleData[TriangleIndex++]; \
						UINT DataIndices[3] = { (UINT) InputIndices[Offset], \
							(UINT) InputIndices[Offset + PolygonTriangleIndex + 1], \
							(UINT) InputIndices[Offset + PolygonTriangleIndex + 2] };
#			define END_FOREACH } Offset += PolygonTriangleCount + 2; } }

			switch (PolygonsInput->GetSource()->GetType())
			{
			case FUDaeGeometryInput::POSITION:
				if (Stride >= 3)
				{
					if (!BaseImporter->InvertUpAxis())
					{
						FOREACH_TRIANGLE
						{
							for (UINT I = 0; I < 3; ++I)
							{
								Triangle->Vertices[I].X = SourceData.GetDataList()[Stride * DataIndices[I]];
								Triangle->Vertices[I].Y = -SourceData.GetDataList()[Stride * DataIndices[I] + 1];
								Triangle->Vertices[I].Z = SourceData.GetDataList()[Stride * DataIndices[I] + 2];
							}
						} END_FOREACH
					}
					else
					{
						FOREACH_TRIANGLE
						{
							for (UINT I = 0; I < 3; ++I)
							{
								Triangle->Vertices[I].X = SourceData[Stride * DataIndices[I]];
								Triangle->Vertices[I].Y = SourceData[Stride * DataIndices[I] + 2];
								Triangle->Vertices[I].Z = SourceData[Stride * DataIndices[I] + 1];
							}
						} END_FOREACH
					}
				} break;

			case FUDaeGeometryInput::TEXCOORD:
				if (Stride >= 2 && TexCoordCount < 8)
				{
					INT UVIndex = TexCoordCount++;
					FOREACH_TRIANGLE
					{
						for (UINT I = 0; I < 3; ++I)
						{
							Triangle->UVs[I][UVIndex].X = SourceData[Stride * DataIndices[I]];
							Triangle->UVs[I][UVIndex].Y = 1.0f - SourceData[Stride * DataIndices[I] + 1];
						}
						Triangle->NumUVs = TexCoordCount;

					} END_FOREACH
				} break;

			case FUDaeGeometryInput::COLOR:
				if (Stride == 3)
				{
					FLinearColor FloatColor(0.0f, 0.0f, 0.0f, 1.0f);
					FOREACH_TRIANGLE
					{
						for (UINT I = 0; I < 3; ++I)
						{
							FloatColor.R = SourceData[Stride * DataIndices[I]];
							FloatColor.G = SourceData[Stride * DataIndices[I] + 1];
							FloatColor.B = SourceData[Stride * DataIndices[I] + 2];
							Triangle->Colors[I] = FColor(FloatColor);
						}
					} END_FOREACH
				}
				else if (Stride >= 4)
				{
					FLinearColor FloatColor;
					FOREACH_TRIANGLE
					{
						for (UINT I = 0; I < 3; ++I)
						{
							FloatColor.R = SourceData[Stride * DataIndices[I]];
							FloatColor.G = SourceData[Stride * DataIndices[I] + 1];
							FloatColor.B = SourceData[Stride * DataIndices[I] + 2];
							FloatColor.A = SourceData[Stride * DataIndices[I] + 3];
							Triangle->Colors[I] = FColor(FloatColor);
						}
					} END_FOREACH
				} break;

			default:
				break;
			}

#			undef FOREACH_TRIANGLE
#			undef END_FOREACH
			RenderMesh->RawTriangles.Unlock();
		}
	}
}

struct CStaticMeshSmoothingGroup
{
	UInt32List Mergeable;
	UInt32List NotMergeable;

	void AddMergeable(uint32 Index) { if (NotMergeable.find(Index) == NotMergeable.end() && Mergeable.find(Index) == Mergeable.end()) Mergeable.push_back(Index); }
	void AddNotMergeable(uint32 Index) { if (NotMergeable.find(Index) == NotMergeable.end()) { UInt32List::iterator it = Mergeable.find(Index); if (it != Mergeable.end()) Mergeable.erase(it); NotMergeable.push_back(Index); } }
};
typedef fm::vector<CStaticMeshSmoothingGroup*> CStaticMeshSmoothingGroupList;

struct CStaticMeshFace
{
	UInt32List Vertices;
	UInt32List Normals;
	UInt32List SmoothingGroups;
};
typedef fm::vector<CStaticMeshFace*> CStaticMeshPolygonList;

void CStaticMesh::CreateSmoothingGroups(const FCDGeometryMesh* ColladaMesh, UINTArray& PolygonSmoothingGroups)
{
	// Retrieve the total number of vertices in the mesh.
	const FCDGeometrySource* ColladaPositionSource = ColladaMesh->FindSourceByType(FUDaeGeometryInput::POSITION);
	size_t VertexCount = ColladaPositionSource->GetSourceData().size() / ColladaPositionSource->GetStride();

	// Generate a vertex face map and an informative list of the polygon data.
	CStaticMeshPolygonList* VertexFaceMap = new CStaticMeshPolygonList[VertexCount];
	UINT FaceCount = (UINT) ColladaMesh->GetFaceCount() + ColladaMesh->GetHoleCount();
	CStaticMeshFace* Faces = new CStaticMeshFace[ FaceCount ];
	PolygonSmoothingGroups.AddZeroed( FaceCount );
	UINT FaceOffset = 0;
	size_t ColladaPolygonsCount = ColladaMesh->GetPolygonsCount();
	for (size_t PolygonIndex = 0; PolygonIndex < ColladaPolygonsCount; ++PolygonIndex)
	{
		// Retrieve the polygons set index data.
		const FCDGeometryPolygons* ColladaPolygons = ColladaMesh->GetPolygons( PolygonIndex );
		const FCDGeometryPolygonsInput* PositionInput = ColladaPolygons->FindInput( FUDaeGeometryInput::POSITION );
		const FCDGeometryPolygonsInput* NormalInput = ColladaPolygons->FindInput( FUDaeGeometryInput::NORMAL );
		if (PositionInput == NULL || NormalInput == NULL) continue;
		const uint32* PositionIndices = PositionInput->GetIndices();
		const uint32* NormalIndices = NormalInput->GetIndices();
		if (PositionIndices == NULL || NormalIndices == NULL) continue;

		size_t ColladaPolygonsFaceCount = ColladaPolygons->GetFaceVertexCountCount();
		size_t FaceVertexOffset = 0;
		for (size_t FaceIndex = 0; FaceIndex < ColladaPolygonsFaceCount; ++FaceIndex)
		{
			// Skip holes.
			if( const_cast< FCDGeometryPolygons* >( ColladaPolygons )->IsHoleFaceHole( FaceIndex ) )
			{
				continue;
			}

			// Retrieve all the relevant polygon information.
			CStaticMeshFace* Face = Faces + FaceOffset + FaceIndex;
			size_t FaceVertexCount = ColladaPolygons->GetFaceVertexCounts()[ FaceIndex ];
			for( int CurVertexIndex = 0; CurVertexIndex < ( int )FaceVertexCount; ++CurVertexIndex )
			{
				Face->Vertices.push_back( PositionIndices[ FaceVertexOffset + CurVertexIndex ] );
				Face->Normals.push_back( NormalIndices[ FaceVertexOffset + CurVertexIndex ] );
			}

			// Fill in the vertex-face map for faster adjacency look-ups.
			for (UInt32List::iterator It = Face->Vertices.begin(); It != Face->Vertices.end(); ++It)
			{
				check( (*It) < VertexCount );
				VertexFaceMap[ *It ].push_back( Face );
			}

			FaceVertexOffset += FaceVertexCount;
		}

		FaceOffset += ColladaPolygonsFaceCount;
	}

	// Assign all the triangles their initial smoothing group.
	CStaticMeshSmoothingGroupList SmoothingGroups;
	uint32 DetachedFaceGroupIndex = UINT_MAX;
	for (UINT FaceIndex = 0; FaceIndex < FaceCount; ++FaceIndex)
	{
		CStaticMeshFace* Face = Faces + FaceIndex;
		UInt32List NotMergeableGroups;

		// Look for adjacent polygons.
		size_t AdjacencyCount = Face->Vertices.size();
		size_t RealAdjacencyCount = 0;
		for (size_t AdjacencyIndex = 0; AdjacencyIndex < AdjacencyCount; ++AdjacencyIndex)
		{
			uint32 EdgeVertices[2] = { Face->Vertices[ AdjacencyIndex ], Face->Vertices[ (AdjacencyIndex + 1) % AdjacencyCount ] };
			CStaticMeshPolygonList* VertexPolygons[2] = { VertexFaceMap + EdgeVertices[0], VertexFaceMap + EdgeVertices[1] };

			// Look for another face that belongs to both lists.
			CStaticMeshFace* AdjacentFace = NULL;
			for (CStaticMeshPolygonList::iterator It0 = VertexPolygons[0]->begin(); It0 != VertexPolygons[0]->end() && AdjacentFace == NULL; ++It0)
			{
				if ((*It0) != Face)
				{
					for (CStaticMeshPolygonList::iterator It1 = VertexPolygons[1]->begin(); It1 != VertexPolygons[1]->end(); ++It1)
					{
						if ((*It1) == (*It0))
						{
							AdjacentFace = (*It0);
							break;
						}
					}
				}
			}

			if (AdjacentFace != NULL && !AdjacentFace->SmoothingGroups.empty())
			{
				// Figure out whether the shared edge is hard or smooth.
				uint32 EdgeNormals[2] = { Face->Normals[ AdjacencyIndex ], Face->Normals[ (AdjacencyIndex + 1) % AdjacencyCount ] };
				BOOL MatchedNormals[2] = { FALSE, FALSE };
				size_t AdjacentFaceVertexCount = AdjacentFace->Vertices.size();
				for (size_t VertexIndex = 0; VertexIndex < AdjacentFaceVertexCount; ++VertexIndex)
				{
					for (size_t I = 0; I < 2; ++I)
					{
						if (!MatchedNormals[I] && AdjacentFace->Vertices[ VertexIndex ] == EdgeVertices[I])
						{
							MatchedNormals[I] = AdjacentFace->Normals[ VertexIndex ] == EdgeNormals[I];
							break;
						}
					}
				}
				BOOL IsSmoothEdge = MatchedNormals[0] && MatchedNormals[1];

				if (IsSmoothEdge)
				{
					// Mark the two faces' smoothing groups as mergeable.
					for (UInt32List::iterator It0 = Face->SmoothingGroups.begin(); It0 != Face->SmoothingGroups.end(); ++It0)
					{
						for (UInt32List::iterator It1 = AdjacentFace->SmoothingGroups.begin(); It1 != AdjacentFace->SmoothingGroups.end(); ++It1)
						{
							if ( (*It0) < (*It1) ) SmoothingGroups[ *It0 ]->AddMergeable( *It1 );
							else if ( (*It0) > (*It1) ) SmoothingGroups[ *It1 ]->AddMergeable( *It0 );
						}
					}
					
					// Add the smoothing groups to the list.
					for (UInt32List::iterator It1 = AdjacentFace->SmoothingGroups.begin(); It1 != AdjacentFace->SmoothingGroups.end(); ++It1)
					{
						if (Face->SmoothingGroups.find( *It1 ) == Face->SmoothingGroups.end()
							&& NotMergeableGroups.find( *It1 ) == NotMergeableGroups.end())
						{
							Face->SmoothingGroups.push_back( *It1 );
						}
					}
				}
				else
				{
					// Remove the smoothing groups from the list.
					for (UInt32List::iterator It1 = AdjacentFace->SmoothingGroups.begin(); It1 != AdjacentFace->SmoothingGroups.end(); ++It1)
					{
						UInt32List::iterator It0 = Face->SmoothingGroups.find( *It1 );
						if (It0 != Face->SmoothingGroups.end())
						{
							Face->SmoothingGroups.erase( It0 );
						}
					}

					// Buffer the list of not-mergeable groups in case a new group is created for this face.
					NotMergeableGroups.insert( NotMergeableGroups.end(), AdjacentFace->SmoothingGroups.begin(), AdjacentFace->SmoothingGroups.end() );
				}
			}
			
			if (AdjacentFace != NULL) ++RealAdjacencyCount;
		}

		if (Face->SmoothingGroups.empty())
		{
			// Check for detached triangles
			if (RealAdjacencyCount == 0)
			{
				if (DetachedFaceGroupIndex == UINT_MAX) 
				{
					DetachedFaceGroupIndex = (uint32) SmoothingGroups.size();
					SmoothingGroups.push_back( new CStaticMeshSmoothingGroup() );
				}
				Face->SmoothingGroups.push_back(DetachedFaceGroupIndex);
			}
			else
			{
				// Create a new smoothing group.
				Face->SmoothingGroups.push_back( (uint32) SmoothingGroups.size() );
				SmoothingGroups.push_back( new CStaticMeshSmoothingGroup() );
			}
		}

		// Mark the faces' smoothing groups as not-mergeable according to buffered edge information.
		for (UInt32List::iterator It0 = Face->SmoothingGroups.begin(); It0 != Face->SmoothingGroups.end(); ++It0)
		{
			for (UInt32List::iterator It1 = NotMergeableGroups.begin(); It1 != NotMergeableGroups.end(); ++It1)
			{
				if ( (*It0) < (*It1) ) SmoothingGroups[ *It0 ]->AddNotMergeable( *It1 );
				else if ( (*It0) > (*It1) ) SmoothingGroups[ *It1 ]->AddNotMergeable( *It0 );
			}
		}
	}

	// Setup an indexable map for the results of the smoothing group merge.
	size_t SmoothingGroupCount = SmoothingGroups.size();
	UInt32List MergeMap; 
	MergeMap.resize( SmoothingGroups.size() );
	for (size_t GroupIndex = 0; GroupIndex < SmoothingGroupCount; ++GroupIndex) MergeMap[ GroupIndex ] = GroupIndex;

	// Merge the mergeable smoothing groups.
	for (size_t GroupIndex = 0; GroupIndex < SmoothingGroupCount; ++GroupIndex)
	{
		if (MergeMap[ GroupIndex ] == GroupIndex)
		{
			// Check for mergeability.
			uint32 MergeableIndex = UINT_MAX;
			CStaticMeshSmoothingGroup* Group = SmoothingGroups[ GroupIndex ];
			for (UInt32List::iterator It = Group->Mergeable.begin(); It != Group->Mergeable.end(); ++It)
			{
				// Check that the smoothing group is not already merged with another smoothing group.
				if (MergeMap[ *It ] == *It) { MergeableIndex = *It; break; }
			}
			if (MergeableIndex == UINT_MAX) continue;

			// Merge these two smoothing groups.
			CStaticMeshSmoothingGroup* MergingGroup = SmoothingGroups[ MergeableIndex ];
			MergeMap[ MergeableIndex ] = GroupIndex;
			for (UInt32List::iterator It = MergingGroup->Mergeable.begin(); It != MergingGroup->Mergeable.end(); ++It)
			{
				Group->AddMergeable( *It );
			}
			for (UInt32List::iterator It = MergingGroup->NotMergeable.begin(); It != MergingGroup->NotMergeable.end(); ++It)
			{
				Group->AddNotMergeable( *It );
			}
		}
	}

	// Merge groups that don't touch in order to save on indices.
	// Also use this last pass to generate the list of merged smoothing groups.
	fm::vector<UInt32List> MergedSmoothingGroups; MergedSmoothingGroups.resize(SmoothingGroupCount);
	for (size_t GroupIndex = 0; GroupIndex < SmoothingGroupCount; ++GroupIndex)
	{
		uint32 MapGroupIndex = MergeMap[ GroupIndex ];
		if (MapGroupIndex == GroupIndex)
		{
			CStaticMeshSmoothingGroup* Group = SmoothingGroups[ GroupIndex ];
			for (size_t OtherGroupIndex = GroupIndex + 1; OtherGroupIndex < SmoothingGroupCount; ++OtherGroupIndex)
			{
				if (MergeMap[ OtherGroupIndex ] == OtherGroupIndex)
				{
					// Two non-merged groups found. Verify whether they touch.
					if (Group->Mergeable.find(OtherGroupIndex) == Group->Mergeable.end()
						&& Group->NotMergeable.find(OtherGroupIndex) == Group->NotMergeable.end())
					{
						// Merge these two smoothing groups.
						CStaticMeshSmoothingGroup* MergingGroup = SmoothingGroups[ OtherGroupIndex ];
						MergeMap[ OtherGroupIndex ] = GroupIndex;
						for (UInt32List::iterator It = MergingGroup->Mergeable.begin(); It != MergingGroup->Mergeable.end(); ++It)
						{
							Group->AddMergeable( *It );
						}
						for (UInt32List::iterator It = MergingGroup->NotMergeable.begin(); It != MergingGroup->NotMergeable.end(); ++It)
						{
							Group->AddNotMergeable( *It );
						}
					}
				}
			}
		}
		else
		{
			// Remove any multi-level indexation.
			MergeMap[ GroupIndex ] = MergeMap[ MapGroupIndex ];
		}

		MergedSmoothingGroups[ MergeMap[ GroupIndex ] ].push_back( GroupIndex );
	}

	// Re-index the smoothing groups in order to have a continuous index list.
	// Also ensure that there is never more than 32 smoothing groups.
	uint32 RealIndex = 0;
	for (size_t GroupIndex = 0; GroupIndex < SmoothingGroupCount; ++GroupIndex)
	{
		if (!MergedSmoothingGroups[ GroupIndex ].empty())
		{
			// Assign these smoothing groups their new index.
			for (UInt32List::iterator It = MergedSmoothingGroups[ GroupIndex ].begin(); It != MergedSmoothingGroups[ GroupIndex ].end(); ++It)
			{
				MergeMap[ *It ] = RealIndex;
			}
			if (++RealIndex >= 32) RealIndex = 0;
		}
	}

	// Finally, generate the list of polygons smoothing groups.
	for (size_t FaceIndex = 0; FaceIndex < FaceCount; ++FaceIndex)
	{
		CStaticMeshFace* Face = Faces + FaceIndex;
		UINT& SmoothingMask = PolygonSmoothingGroups(FaceIndex) = 0;
		for (UInt32List::iterator It = Face->SmoothingGroups.begin(); It != Face->SmoothingGroups.end(); ++It)
		{
			uint32 GroupIndex = MergeMap[ *It ];
			SmoothingMask |= (0x1 << GroupIndex);
		}
	}

	// Release the temporary data structures.
	CLEAR_POINTER_VECTOR( SmoothingGroups );
	SAFE_DELETE_ARRAY( VertexFaceMap );
	SAFE_DELETE_ARRAY( Faces );
}

/*
 * Exports the given static rendering mesh into a COLLADA geometry.
 */
FCDGeometry* CStaticMesh::ExportStaticMesh(FStaticMeshRenderData& RenderMesh, const TCHAR* MeshName)
{
	if (BaseExporter == NULL) return NULL;

	// Verify the integrity of the static mesh.
	if (RenderMesh.VertexBuffer.GetNumVertices() == 0) return NULL;
	if (RenderMesh.Elements.Num() == 0) return NULL;

	// Retrieve the COLLADA document and look for an already exported version of this render mesh.
	FCDocument* ColladaDocument = BaseExporter->GetColladaDocument();
	INT GeometryCount = ColladaDocument->GetGeometryLibrary()->GetEntityCount();
	for (INT GeometryIndex = 0; GeometryIndex < GeometryCount; ++GeometryIndex)
	{
		FCDGeometry* Geometry = ColladaDocument->GetGeometryLibrary()->GetEntity(GeometryIndex);
		if (Geometry->GetUserHandle() == &RenderMesh)
		{
			return Geometry;
		}
	}

	// Create a new COLLADA geometry entity for this render mesh
	FCDGeometry* ColladaGeometry = ColladaDocument->GetGeometryLibrary()->AddEntity();
	ColladaGeometry->SetName(MeshName);
	ColladaGeometry->SetDaeId(TO_STRING(ColladaGeometry->GetName()) + "-obj");
	FCDGeometryMesh* ColladaMesh = ColladaGeometry->CreateMesh();

	// Create and fill in the vertex position data source.
	// The position vertices are duplicated, for some reason, retrieve only the first half vertices.
	FCDGeometrySource* PositionSource = ColladaMesh->AddVertexSource(FUDaeGeometryInput::POSITION);
	const INT VertexCount = RenderMesh.VertexBuffer.GetNumVertices();
	PositionSource->SetDaeId(ColladaGeometry->GetDaeId() + "-positions");
	PositionSource->SetStride(3);
	PositionSource->SetValueCount( VertexCount );
	for (INT PosIndex = 0; PosIndex < VertexCount; ++PosIndex)
	{
		const FVector& Position = RenderMesh.PositionVertexBuffer.VertexPosition(PosIndex);
		PositionSource->GetSourceData().set( PosIndex * 3 + 0, Position.X );
		PositionSource->GetSourceData().set( PosIndex * 3 + 1, -Position.Y );
		PositionSource->GetSourceData().set( PosIndex * 3 + 2, Position.Z );
	}

	// Create and fill in the per-face-vertex normal data source.
	// We extract the Z-tangent and drop the X/Y-tangents which are also stored in the render mesh.
	FCDGeometrySource* NormalSource = ColladaMesh->AddSource(FUDaeGeometryInput::NORMAL);
	NormalSource->SetDaeId(ColladaGeometry->GetDaeId() + "-normals");
	NormalSource->SetStride(3);
	NormalSource->SetValueCount( VertexCount );
	for (INT NormalIndex = 0; NormalIndex < VertexCount; ++NormalIndex)
	{
		FVector Normal = (FVector) (RenderMesh.VertexBuffer.VertexTangentZ(NormalIndex));
		NormalSource->GetSourceData().set( NormalIndex * 3 + 0, Normal.X );
		NormalSource->GetSourceData().set( NormalIndex * 3 + 1, -Normal.Y );
		NormalSource->GetSourceData().set( NormalIndex * 3 + 2, Normal.Z );
	}

	// Create and fill in the per-face-vertex texture coordinate data source(s).
	INT TexCoordSourceCount = RenderMesh.VertexBuffer.GetNumTexCoords();
	FCDGeometrySourceList TexCoordSources;
	TexCoordSources.resize(TexCoordSourceCount);
	for (INT TexCoordSourceIndex = 0; TexCoordSourceIndex < TexCoordSourceCount; ++TexCoordSourceIndex)
	{
		// Create the texture coordinate data source.
		FCDGeometrySource* TexCoordSource = TexCoordSources[TexCoordSourceIndex] = ColladaMesh->AddSource(FUDaeGeometryInput::TEXCOORD);
		TexCoordSource->SetDaeId(ColladaMesh->GetDaeId() + "-texcoord" + FUStringConversion::ToString(TexCoordSourceIndex));
		TexCoordSource->SetStride(2);
		TexCoordSource->SetValueCount( VertexCount );
		for (INT TexCoordIndex = 0; TexCoordIndex < VertexCount; ++TexCoordIndex)
		{
			const FVector2D& TexCoord = RenderMesh.VertexBuffer.GetVertexUV(TexCoordSourceIndex, TexCoordIndex);
			TexCoordSource->GetSourceData().set( TexCoordIndex * 2 + 0, TexCoord.X );
			TexCoordSource->GetSourceData().set( TexCoordIndex * 2 + 1, -TexCoord.Y );
		}
	}

	// Create the per-material polygons sets.
	INT PolygonsCount = RenderMesh.Elements.Num();
	for (INT PolygonsIndex = 0; PolygonsIndex < PolygonsCount; ++PolygonsIndex)
	{
		FStaticMeshElement& Polygons = RenderMesh.Elements(PolygonsIndex);

		// Create the COLLADA per-material polygons set and attach to it the correct inputs.
		// As with the brush models, the static mesh data has been prepared by the engine in order
		// to generate unique indices. Therefore, all the data sources use the same indices.
		FCDGeometryPolygons* ColladaPolygons = ColladaMesh->AddPolygons();
		ColladaPolygons->AddInput(NormalSource, 0);
		for (INT TexCoordSourceIndex = 0; TexCoordSourceIndex < TexCoordSourceCount; ++TexCoordSourceIndex)
		{
			FCDGeometryPolygonsInput* ColladaTexCoordInput = ColladaPolygons->AddInput(TexCoordSources[TexCoordSourceIndex], 0);
			ColladaTexCoordInput->SetSet( TexCoordSourceIndex );
		}

		// Static meshes contain one triangle list per element.
		// [GLAFORTE] Could it occasionally contain triangle strips? How do I know?
		INT TriangleCount = Polygons.NumTriangles;
		ColladaPolygons->SetFaceVertexCountCount( 0 );
		for( INT CurTriangleIndex = 0; CurTriangleIndex < TriangleCount; ++CurTriangleIndex )
		{
			ColladaPolygons->AddFaceVertexCount( 3 );
		}

		UInt32List ColladaIndices;
		ColladaIndices.reserve(TriangleCount * 3);

		// Copy over the index buffer into the COLLADA polygons set.
		for (INT IndexIndex = 0; IndexIndex < TriangleCount * 3; ++IndexIndex)
		{
			ColladaIndices.push_back(RenderMesh.IndexBuffer.Indices(Polygons.FirstIndex + IndexIndex));
		}

		ColladaPolygons->FindInput( PositionSource )->AddIndices( ColladaIndices );

		// Copy over the index buffer into the COLLADA polygons set. [GLAFORTE : version for triangle strips]
		//for (INT IndexIndex = 0; IndexIndex < TriangleCount; ++IndexIndex)
		//{
		//	WORD* Indices = &(RenderMesh.IndexBuffer.Indices(Polygons.FirstIndex + IndexIndex));

		//	// Remove degenerate triangles
		//	if (Indices[0] == Indices[1] || Indices[1] == Indices[2] || Indices[0] == Indices[2]) continue;

		//	ColladaIndices->push_back(Indices[0]);
		//	ColladaIndices->push_back(Indices[1]);
		//	ColladaIndices->push_back(Indices[2]);
		//}

		// Generate a material semantic for this polygons set.
		ColladaPolygons->SetMaterialSemantic(FS("mat") + FUStringConversion::ToFString(PolygonsIndex));
	}

	return ColladaGeometry;
}

} // namespace UnCollada
