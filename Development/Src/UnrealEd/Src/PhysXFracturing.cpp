/*=============================================================================
	PhysXFracturing.cpp: Destructible Vertical Component.
	Copyright 2007-2008 AGEIA Technologies.
=============================================================================*/

#include "UnrealEd.h"
#include "EngineMaterialClasses.h"
#include "UnFracturedStaticMesh.h"
#include "EngineMeshClasses.h"
#include "EnginePhysicsClasses.h"
#include "ConvexDecompTool.h"
#include "GeomTools.h"

#define USE_NEW_TMAPPING	0

/* From Dan Wright:
	LODModels(0).RawTriangles is the unoptimized raw data for static meshes that is used for rebuilding the actual rendering data.
	All of James' fracturing tools operate on the raw triangles.  In the process of creating a new fractured mesh we call
	StaticMesh->Build() which does the vertex merging, sorting based on material index, cache optimizing indices and so on.
	The result is stored in a LODModels(0).PositionVertexBuffer and LODModels(0).VertexBuffer.  I think for your purposes you want
	to get these triangles and not the raw ones.  If you know the element index and fragment index then you can find the index range
	for a fragment in LODModels(0).Elements(ElemIndex).Fragments(FragmentIndex).  That is the range of indices in
	LODModels(0).IndexBuffer that belong to the fragment.  And the vertices are in LODModels(0).PositionVertexBuffer and
	LODModels(0).VertexBuffer.  On PC you should be able to read from all of those on the game thread without any special handling,
	but on consoles the CPU readable copy of some of those buffers are thrown away.
*/

// From StaticMeshEditor.cpp:
extern void ClipMeshWithHull(	const TArray<FClipSMTriangle>& StaticMeshTriangles,
								const TArray<FPlane>& HullPlanes,
								const TArray<INT>& InNeighbours,
								INT InteriorMatIndex,
								TArray<FStaticMeshTriangle>& OutExteriorTris, 
								TArray<FStaticMeshTriangle>& OutInteriorTris,
								TArray<INT>& OutNeighbours,
								TArray<FLOAT>& OutNeighbourDims);

// Returns a point uniformly distributed on the "polar cap" in +AxisN direction, of azimuthal size Range (in radians)
inline FVector RandomNormal( UINT AxisN, FLOAT Range )
{
	FVector Result;
	Result[AxisN%3] = 1.0f - (1.0f-appCos( Range ))*appFrand();
	const FLOAT SinTheta = appSqrt( 1.0f - Result[AxisN]*Result[AxisN] );
	const FLOAT Phi = 2.0f*PI*appFrand();
	Result[(AxisN+1)%3] = appCos( Phi )*SinTheta;
	Result[(AxisN+2)%3] = appSin( Phi )*SinTheta;
	return Result;
}

// Find the most dominant bone for each vertex
static INT GetDominantBoneIndex(FSoftSkinVertex* SoftVert)
{
	BYTE MaxWeightBone = 0;
	BYTE MaxWeightWeight = 0;

	for(INT i=0; i<4; i++)
	{
		if(SoftVert->InfluenceWeights[i] > MaxWeightWeight)
		{
			MaxWeightWeight = SoftVert->InfluenceWeights[i];
			MaxWeightBone = SoftVert->InfluenceBones[i];
		}
	}

	return MaxWeightBone;
}

static void CreatePhysicsAsset(USkeletalMesh* SkeletalMesh, UPhysicsAsset* PhysicsAsset, INT Depth, INT MaxVerts, FLOAT CollapseThresh)
{	
	PhysicsAsset->DefaultSkelMesh = SkeletalMesh;

	// Create an empty default PhysicsInstance
	PhysicsAsset->DefaultInstance = ConstructObject<UPhysicsAssetInstance>( UPhysicsAssetInstance::StaticClass(), PhysicsAsset, NAME_None, RF_Transactional );
	
	PhysicsAsset->DefaultInstance->bInitBodies = FALSE;

	TArray<FBoneVertInfo> Infos;
	SkeletalMesh->CalcBoneVertInfos(Infos, TRUE);

	TArray< TArray<INT>* > BoneVertices;
	BoneVertices.Empty();
	BoneVertices.Add( SkeletalMesh->RefSkeleton.Num());

	for (INT BoneIndex = 0; BoneIndex<SkeletalMesh->RefSkeleton.Num(); BoneIndex++)
		BoneVertices( BoneIndex ) = new TArray<INT>();
	
	if( SkeletalMesh->LODModels.Num() == 0)
		return;

	FStaticLODModel* LODModel = &(SkeletalMesh->LODModels(0));

	for(INT ChunkIndex = 0;ChunkIndex < LODModel->Chunks.Num();ChunkIndex++)
	{
		INT VertIndex = 0;
		FSkelMeshChunk& Chunk = LODModel->Chunks(ChunkIndex);
		
		for(INT i=0; i<Chunk.RigidVertices.Num(); i++)
		{
			FRigidSkinVertex* RigidVert = &Chunk.RigidVertices(i);
			INT BoneIndex = Chunk.BoneMap(RigidVert->Bone);
			BoneVertices(BoneIndex)->AddItem(Chunk.BaseVertexIndex+VertIndex);
			VertIndex++;
		}

		for(INT i=0; i<Chunk.SoftVertices.Num(); i++)
		{
			FSoftSkinVertex* SoftVert = &Chunk.SoftVertices(i);
			INT BoneIndex = Chunk.BoneMap(GetDominantBoneIndex(SoftVert));
			BoneVertices(BoneIndex)->AddItem(Chunk.BaseVertexIndex+VertIndex);
			VertIndex++;
		}
	}
		
	for(INT BoneIndex = 0; BoneIndex < SkeletalMesh->RefSkeleton.Num(); BoneIndex++)
	{
		FName BoneName = SkeletalMesh->RefSkeleton( BoneIndex ).Name;

		INT NewBodyIndex = PhysicsAsset->CreateNewBody( BoneName );
		URB_BodySetup* bs = PhysicsAsset->BodySetup( NewBodyIndex );

		// Get the BodySetup we are going to put the collision into
		if(bs)
		{
			bs->AggGeom.EmptyElements();
			bs->ClearShapeCache();
		}

		// Make vertex buffer
		INT NumVerts = Infos(BoneIndex).Positions.Num();
		TArray<FVector> Verts;
		for(INT i=0; i<NumVerts; i++)
		{
			FVector Vert = Infos(BoneIndex).Positions(i);
			Verts.AddItem(Vert);
		}

		//Make index buffer
		TArray<INT> Indices;
		for(INT i=0; i<LODModel->IndexBuffer.Indices.Num(); i+=3)
		{
			for (INT j=0; j<3; j++)
			{
				WORD Index = SkeletalMesh->LODModels(0).IndexBuffer.Indices(i+j);
				INT NewIndex = BoneVertices(BoneIndex)->FindItemIndex(Index);
				if (NewIndex != INDEX_NONE) Indices.AddItem(NewIndex);
			}
		}

		delete BoneVertices(BoneIndex);

		// Run actual util to do the work
		DecomposeMeshToHulls(&(bs->AggGeom), Verts, Indices, Depth, 0.1f, CollapseThresh, MaxVerts);

		// Add a default cooking scale
		bs->PreCachedPhysScale.AddItem(FVector(1,1,1));
	}

	// Mark mesh as dirty
	SkeletalMesh->MarkPackageDirty();
}

inline UBOOL UVsEqual( FStaticMeshTriangle & T1, FStaticMeshTriangle & T2, INT VertexIndex, FLOAT Tolerance = 0.0f )
{
	if( T1.NumUVs != T2.NumUVs )
	{
		return 0;
	}

	for( INT i = 0; i < T1.NumUVs; ++i )
	{
		FVector2D & UV1 = T1.UVs[VertexIndex][i];
		FVector2D & UV2 = T2.UVs[VertexIndex][i];
		if( Abs(UV1.X-UV2.X) > Tolerance || Abs(UV1.Y-UV2.Y) > Tolerance )
		{
			return 0;
		}
	}

	return 1;
}

struct FStaticMeshRefTriangle
{
	INT			VertexIndices[3];
	FVector2D	UVs[3][8];
	FColor		Colors[3];
	INT			MaterialIndex;
	INT			FragmentIndex;
	DWORD		SmoothingMask;
	INT			NumUVs;
};

struct FStaticMeshInfo
{
	TArray<FStaticMeshRefTriangle>	Triangles;
	TArray<FVector>					Vertices;

	INT	Build( TArray<FStaticMeshTriangle> & SMTriangles );
};

INT
FStaticMeshInfo::Build( TArray<FStaticMeshTriangle> & SMTriangles )
{
	Triangles.Empty();
	Vertices.Empty();
	TArray<INT> TriangleIndices;

	Triangles.Add( SMTriangles.Num() );

	for( INT TriangleIndex = 0; TriangleIndex < SMTriangles.Num(); ++TriangleIndex )
	{
		FStaticMeshRefTriangle & Triangle = Triangles( TriangleIndex );
		FStaticMeshTriangle & SMTriangle = SMTriangles( TriangleIndex );
		for( INT TriangleVertexIndex = 0; TriangleVertexIndex < 3; ++TriangleVertexIndex )
		{
			for( INT UVIndex = 0; UVIndex < 8; ++UVIndex )
			{
				Triangle.UVs[TriangleVertexIndex][UVIndex] = SMTriangle.UVs[TriangleVertexIndex][UVIndex];
			}
			Triangle.Colors[TriangleVertexIndex] = SMTriangle.Colors[TriangleVertexIndex];
			INT FoundVertexIndex = INDEX_NONE;
			FVector & Vertex = SMTriangle.Vertices[TriangleVertexIndex];
			for( INT VertexIndex = 0; VertexIndex < Vertices.Num(); ++VertexIndex )
			{
				FStaticMeshTriangle & TestTriangle = SMTriangles( TriangleIndices( VertexIndex ) );
				if( Vertices(VertexIndex).Equals( Vertex, THRESH_POINTS_ARE_SAME ) &&
					UVsEqual( TestTriangle, SMTriangle, TriangleVertexIndex, 0.5f ) &&
					TestTriangle.MaterialIndex == SMTriangle.MaterialIndex &&
					(TestTriangle.SmoothingMask & SMTriangle.SmoothingMask ) != 0 )
				{
					FoundVertexIndex = VertexIndex;
					break;
				}
			}
			if( FoundVertexIndex == INDEX_NONE )
			{
				FoundVertexIndex = Vertices.Num();
				Vertices.AddItem( Vertex );
				TriangleIndices.AddItem( TriangleIndex );
			}
			Triangle.VertexIndices[TriangleVertexIndex] = FoundVertexIndex;
		}
		Triangle.MaterialIndex = SMTriangle.MaterialIndex;
		Triangle.FragmentIndex = SMTriangle.FragmentIndex;
		Triangle.SmoothingMask = SMTriangle.SmoothingMask;
		Triangle.NumUVs = SMTriangle.NumUVs;
	}

	return Vertices.Num();
};

struct FSkeletalMeshInfo
{
	TArray<UMaterialInterface*> Materials; 
	TArray<FMeshBone>			Bones;
	TArray<FVector>				Points;
	TArray<FMeshWedge>			Wedges;
	TArray<FMeshFace>			Faces;
	TArray<FVertInfluence>		Influences;
	INT							RootBoneIndex;

	FSkeletalMeshInfo() : RootBoneIndex( 0 ) {}
};

static INT AppendBone( FSkeletalMeshInfo* SkeletalMeshInfo, 
					   const FStaticMeshInfo * StaticMeshInfo = NULL,
					   FVector Location = FVector(0,0,0) )
{
	//add a new bone for this chunk
	SkeletalMeshInfo->Bones.Add(1);		
	INT BoneIndex = SkeletalMeshInfo->Bones.Num() - 1;

	FMeshBone& Bone = SkeletalMeshInfo->Bones(BoneIndex);

	if ( SkeletalMeshInfo->RootBoneIndex != INDEX_NONE && SkeletalMeshInfo->RootBoneIndex != BoneIndex )
		Location -= SkeletalMeshInfo->Bones(SkeletalMeshInfo->RootBoneIndex).BonePos.Position;

	Bone.Name = NAME_None;
	Bone.ParentIndex = SkeletalMeshInfo->RootBoneIndex;
	Bone.Depth = 0;
	Bone.NumChildren = 0;
	Bone.Flags = 0;
	Bone.BonePos.Position = Location;
	Bone.BonePos.Orientation = FQuat( 0, 0, 0, 1 );
	Bone.BonePos.Length = 1.0f;
	Bone.BonePos.XSize = 1.0f;
	Bone.BonePos.YSize = 1.0f;
	Bone.BonePos.ZSize = 1.0f;

	if ( StaticMeshInfo != NULL && StaticMeshInfo->Triangles.Num() > 0 )
	{
		INT VertexOffset = SkeletalMeshInfo->Points.Num();
		INT VertexCount = StaticMeshInfo->Vertices.Num();

		SkeletalMeshInfo->Points.Add(VertexCount);
		SkeletalMeshInfo->Wedges.Add(VertexCount);
		SkeletalMeshInfo->Influences.Add(VertexCount);

		for(INT VertexIndex = 0; VertexIndex < StaticMeshInfo->Vertices.Num(); VertexIndex++)
		{
			INT OffsetVertexIndex = VertexOffset+VertexIndex;
			SkeletalMeshInfo->Points(OffsetVertexIndex) = StaticMeshInfo->Vertices(VertexIndex) + Location;
			SkeletalMeshInfo->Wedges(OffsetVertexIndex).iVertex = OffsetVertexIndex;
			SkeletalMeshInfo->Influences(OffsetVertexIndex).Weight = 1.0f;
			SkeletalMeshInfo->Influences(OffsetVertexIndex).VertIndex = OffsetVertexIndex;
			SkeletalMeshInfo->Influences(OffsetVertexIndex).BoneIndex = BoneIndex;
		}
		
		for(INT TriangleIndex = 0; TriangleIndex < StaticMeshInfo->Triangles.Num(); TriangleIndex++)
		{
			const FStaticMeshRefTriangle& Triangle = StaticMeshInfo->Triangles(TriangleIndex);			
			for(INT TriangleVertexIndex = 0; TriangleVertexIndex < 3; TriangleVertexIndex++)
			{
				INT VertexIndex = Triangle.VertexIndices[TriangleVertexIndex];
				INT OffsetVertexIndex = VertexOffset+VertexIndex;
				SkeletalMeshInfo->Wedges(OffsetVertexIndex).U = Triangle.UVs[TriangleVertexIndex][0].X;
				SkeletalMeshInfo->Wedges(OffsetVertexIndex).V = Triangle.UVs[TriangleVertexIndex][0].Y;
			}
		}

		INT TriangleOffset = SkeletalMeshInfo->Faces.Num();
		SkeletalMeshInfo->Faces.Add(StaticMeshInfo->Triangles.Num());
		for(INT TriangleIndex = 0; TriangleIndex < StaticMeshInfo->Triangles.Num(); TriangleIndex++)
		{
			const FStaticMeshRefTriangle& Triangle = StaticMeshInfo->Triangles(TriangleIndex);			
			SkeletalMeshInfo->Faces(TriangleOffset + TriangleIndex).MeshMaterialIndex = Triangle.MaterialIndex;
			for(INT TriangleVertexIndex = 0; TriangleVertexIndex < 3; TriangleVertexIndex++)
			{
				SkeletalMeshInfo->Faces(TriangleOffset + TriangleIndex).iWedge[TriangleVertexIndex] = VertexOffset + Triangle.VertexIndices[TriangleVertexIndex];
			}
		}
	}

	return  BoneIndex;
}

/**
* Creates a skeletal mesh from the given skeletal mesh info.
*
* @param	SkeletalMesh		[out] The resulting skeletal mesh.
* @param	SkeletalMeshInfo	The source skeletal mesh info.
*/
static void CreateSkeletalMesh(USkeletalMesh* SkeletalMesh, const FSkeletalMeshInfo* SkeletalMeshInfo)
{
	const TArray<FMeshBone>&			Bones		= SkeletalMeshInfo->Bones;
	const TArray<FVector>&				Points		= SkeletalMeshInfo->Points;
	const TArray<FMeshWedge>&			Wedges		= SkeletalMeshInfo->Wedges;
	const TArray<FMeshFace>&			Faces		= SkeletalMeshInfo->Faces;
	const TArray<FVertInfluence>&		Influences	= SkeletalMeshInfo->Influences;
	const TArray<UMaterialInterface*>&	Materials	= SkeletalMeshInfo->Materials;

	// Copy Materials
	SkeletalMesh->Materials.Add(Materials.Num());
	for(INT MaterialIndex = 0; MaterialIndex < Materials.Num(); MaterialIndex++)
	{
		SkeletalMesh->Materials(MaterialIndex) = Materials(MaterialIndex);
	}

	// Skeleton (ripped from CSkeletalMesh::FinishBones)
	SkeletalMesh->RefSkeleton = Bones;

	//reset the bone name now, as all bone names must be unique or shit breaks
	if(SkeletalMesh->RefSkeleton.Num())
	{
		SkeletalMesh->RefSkeleton(0).Name = FName(TEXT("Bone"), 0);
	}

	// Traverse the engine bone hierarchy directly, to calculate the number of children and the depth.
	// Note that all child bones are always positioned after their parents: the parent is always fully initialized.
	SkeletalMesh->SkeletalDepth = 0;
	for(INT BoneIndex = 1; BoneIndex < Bones.Num(); ++BoneIndex)
	{
		FMeshBone& EngineBone = SkeletalMesh->RefSkeleton( BoneIndex );
		//reset the bone name now, as all bone names must be unique or shit breaks
		EngineBone.Name = FName(TEXT("Bone"), BoneIndex);
		FMeshBone& ParentBone = SkeletalMesh->RefSkeleton( EngineBone.ParentIndex );
		++(ParentBone.NumChildren);
		EngineBone.Depth = ParentBone.Depth + 1;
		if(SkeletalMesh->SkeletalDepth < EngineBone.Depth) SkeletalMesh->SkeletalDepth = EngineBone.Depth;
	}

	// Now RefSkeleton is complete, initialise bone name -> bone index map
	SkeletalMesh->InitNameIndexMap();

	// Geometry (ripped from CSkeletalMesh::FinishGeometry)

	// In the engine, create/retrieve the first LOD mesh using the vertex streams
	// The rest of this function is adapted from USkeletalMeshFactory::FactoryCreateBinary
	SkeletalMesh->LODModels.Empty();
	new( SkeletalMesh->LODModels ) FStaticLODModel();

	SkeletalMesh->LODInfo.Empty();
	new( SkeletalMesh->LODInfo ) FSkeletalMeshLODInfo();
	SkeletalMesh->LODInfo(0).LODHysteresis = 0.02f;

	// Create the initial bounding box based on expanded version of reference pose
	// for meshes without physics assets. Can be overridden by artist.
	// Tuck up the bottom as this rarely extends lower than a reference pose's
	// (e.g. having its feet on the floor).
	FBox BoundingBox( &Points(0), Points.Num() );
	FBox Temp = BoundingBox;
	FVector MidMesh = 0.5f * ( Temp.Min + Temp.Max );
	BoundingBox.Min = Temp.Min + 1.0f * ( Temp.Min - MidMesh );
	BoundingBox.Max = Temp.Max + 1.0f * ( Temp.Max - MidMesh );
	BoundingBox.Min.Z = Temp.Min.Z + 0.1f * ( Temp.Min.Z - MidMesh.Z );
	SkeletalMesh->Bounds = FBoxSphereBounds( BoundingBox );

	// Create actual rendering data.
	SkeletalMesh->CreateSkinningStreams( Influences, Wedges, Faces, Points );

	// RequiredBones for base model includes all bones.
	FStaticLODModel& LODModel = SkeletalMesh->LODModels( 0 );
	INT RequiredBoneCount = SkeletalMesh->RefSkeleton.Num();
	LODModel.RequiredBones.Add( RequiredBoneCount );
	for( INT i=0; i < RequiredBoneCount; ++i )
	{
		LODModel.RequiredBones(i) = i;
	}

	// Support bEnableShadowCasting
	SkeletalMesh->LODInfo(0).bEnableShadowCasting.Empty( SkeletalMesh->LODModels(0).Sections.Num() );
	for ( INT SectionIndex = 0 ; SectionIndex < SkeletalMesh->LODModels(0).Sections.Num() ; ++SectionIndex )
	{
		SkeletalMesh->LODInfo(0).bEnableShadowCasting.AddItem( TRUE );
	}

	// Make GUID.
	SkeletalMesh->SkelMeshGUID = appCreateGuid();

	// End the importing, mark package as dirty so it prompts to save on exit.
	SkeletalMesh->PreEditChange(NULL);
	SkeletalMesh->PostEditChange(NULL);
	SkeletalMesh->CalculateInvRefMatrices();

	SkeletalMesh->MarkPackageDirty();
}

static void AddBoneInfo( TArrayNoInit<FPhysXDestructibleAssetChunk> & ChunkTree,
						 TArray<FSkeletalMeshInfo> & SkeletalMeshInfo,
						 TArray<FStaticMeshTriangle> & Triangles,
						 INT ParentIndex,
						 INT Depth,
						 FVector Location = FVector(0,0,0),
						 UBOOL bForceNewMesh = FALSE )
{
	enum
	{
		kMaxBoneCount	= 250
	};

	if( bForceNewMesh || SkeletalMeshInfo.Num() == 0 || (SkeletalMeshInfo( SkeletalMeshInfo.Num()-1 ).Bones.Num() % kMaxBoneCount) == 0 )
	{
		SkeletalMeshInfo.Add(1);
		FSkeletalMeshInfo* Info = new( &SkeletalMeshInfo( SkeletalMeshInfo.Num()-1 )) FSkeletalMeshInfo;
		Info->RootBoneIndex = AppendBone( Info );
	}

	FStaticMeshInfo StaticMeshInfo;
	const INT VertexCount = StaticMeshInfo.Build( Triangles );
	if( VertexCount > 65535 )
	{
		appErrorf( TEXT("Fracture: Chunk contains too many vertices (> 65535).") );
		return;
	}

	if( VertexCount > 0 )
	{
		if( SkeletalMeshInfo( SkeletalMeshInfo.Num()-1 ).Points.Num() + VertexCount > 65535 )
		{
			SkeletalMeshInfo.Add(1);
			FSkeletalMeshInfo* Info = new( &SkeletalMeshInfo( SkeletalMeshInfo.Num()-1 )) FSkeletalMeshInfo;
			Info->RootBoneIndex = AppendBone( Info );
		}
		INT MeshIndex = SkeletalMeshInfo.Num()-1;
		check( MeshIndex >= 0 );
		INT BoneIndex = AppendBone( &SkeletalMeshInfo( MeshIndex ), &StaticMeshInfo, Location );
		FPhysXDestructibleAssetChunk Chunk;
		Chunk.Index = ChunkTree.Num();
		Chunk.FragmentIndex = INDEX_NONE;
		Chunk.Size = 1.0f;	// To be set later
		Chunk.Depth = Depth; 
		Chunk.ParentIndex = ParentIndex;
		Chunk.FirstChildIndex = INDEX_NONE;
		Chunk.NumChildren = 0;
		Chunk.MeshIndex = MeshIndex;
		Chunk.BoneIndex = BoneIndex;
		Chunk.BoneName = NAME_None;
		Chunk.BodyIndex = INDEX_NONE;
		ChunkTree.AddItem( Chunk );
		if( ParentIndex >= 0 )
		{
			FPhysXDestructibleAssetChunk & Parent = ChunkTree( ParentIndex );
			if( !Parent.NumChildren++ )
			{
				Parent.FirstChildIndex = Chunk.Index;
			}
		}
	}
}

static void GetTrianglesFromMeshFragment( TArray<FClipSMTriangle> & OutTriangles, UFracturedStaticMesh * FracturedStaticMesh, INT FragmentIndex )
{
	OutTriangles.Empty();

	// Extract all of the static mesh's triangles.
	TArray<FClipSMTriangle> AllTriangles;
	GetClippableStaticMeshTriangles(AllTriangles,FracturedStaticMesh);

	// Filter out the triangles that aren't in the right fragment.
	for(INT TriangleIndex = 0;TriangleIndex < AllTriangles.Num();TriangleIndex++)
	{
		if(AllTriangles(TriangleIndex).FragmentIndex == FragmentIndex)
		{
			OutTriangles.AddItem(AllTriangles(TriangleIndex));
		}
	}
}

static UBOOL CreatePhysXDestructibleAsset(
	UPhysXDestructibleAsset * PhysXDestructibleAsset,
	UFracturedStaticMesh* FracturedStaticMesh,
	INT FragmentIndex,
	UPhysXFractureOptions* FractureOptions,
	UObject* Outer = INVALID_OBJECT)
{
	PhysXDestructibleAsset->Meshes.Empty();
	PhysXDestructibleAsset->Assets.Empty();
	PhysXDestructibleAsset->ChunkTree.Empty();

	// Extract mesh fragment triangles
	TArray<FClipSMTriangle> FragmentTriangles;
	GetTrianglesFromMeshFragment( FragmentTriangles, FracturedStaticMesh, FragmentIndex );
	if( FragmentTriangles.Num() == 0 )
	{
		debugf( TEXT("Fragment %d of %s had no triangles."), FragmentIndex, *FracturedStaticMesh->GetName() );
		return TRUE;
	}

	FBox BBox = FracturedStaticMesh->GetFragmentBox( FragmentIndex );
	FVector BoxExtent, BoxCenter;
	BBox.GetCenterAndExtents( BoxCenter, BoxExtent );
	BoxExtent *= 1.001f;	// Expand a bit, to account for roundoff error
	BBox = FBox::BuildAABB( BoxCenter, BoxExtent );	

	// Access to static mesh elements
	const TArray<FStaticMeshElement> & Elements = FracturedStaticMesh->LODModels(0).Elements;

	// Get interior material index
	INT InteriorMaterialIndex = INDEX_NONE;
	if( FracturedStaticMesh->GetInteriorElementIndex() >= 0 )
	{
		InteriorMaterialIndex = Elements( FracturedStaticMesh->GetInteriorElementIndex() ).MaterialIndex;
	}

	// This is the descriptor struct for the skeletal mesh
	TArray<FSkeletalMeshInfo> SkeletalMeshInfo;

	// Add the original mesh as "level 0", the unfractured mesh
	TArray<FStaticMeshTriangle> RawStaticMeshTriangles;
	GetRawStaticMeshTrianglesFromClipped(RawStaticMeshTriangles,FragmentTriangles);
	AddBoneInfo( PhysXDestructibleAsset->ChunkTree, SkeletalMeshInfo, RawStaticMeshTriangles, INDEX_NONE, 0 );

	// Working arrays
	TArray<FKConvexElem> Pieces0;
	TArray<FKConvexElem> Pieces1;
	TArray<FKConvexElem> LevelPieces;

	TArray<FKConvexElem>* Source = &Pieces0;
	TArray<FKConvexElem>* Target = &Pieces1;

	// Initialize convex block out of AABB, as first element of Source
	Source->AddZeroed(1);
	FKConvexElem & InitChunk = (*Source)(0);
	InitChunk.VertexData.AddItem( BoxCenter + FVector(-BoxExtent.X,-BoxExtent.Y,-BoxExtent.Z ) );
	InitChunk.VertexData.AddItem( BoxCenter + FVector(-BoxExtent.X,-BoxExtent.Y, BoxExtent.Z ) );
	InitChunk.VertexData.AddItem( BoxCenter + FVector(-BoxExtent.X, BoxExtent.Y,-BoxExtent.Z ) );
	InitChunk.VertexData.AddItem( BoxCenter + FVector(-BoxExtent.X, BoxExtent.Y, BoxExtent.Z ) );
	InitChunk.VertexData.AddItem( BoxCenter + FVector( BoxExtent.X,-BoxExtent.Y,-BoxExtent.Z ) );
	InitChunk.VertexData.AddItem( BoxCenter + FVector( BoxExtent.X,-BoxExtent.Y, BoxExtent.Z ) );
	InitChunk.VertexData.AddItem( BoxCenter + FVector( BoxExtent.X, BoxExtent.Y,-BoxExtent.Z ) );
	InitChunk.VertexData.AddItem( BoxCenter + FVector( BoxExtent.X, BoxExtent.Y, BoxExtent.Z ) );
	InitChunk.GenerateHullData();

	// Slice volume rejection ratio, perhaps should be exposed
	const FLOAT VolumeRejectionRatio = 0.1f;

	// The slicing algorithm
	INT ParentIndex = 0;
	for( INT Level = 0; Level < FractureOptions->SlicingLevels.Num(); ++Level )
	{
		FPhysXSlicingParameters & Slicing = FractureOptions->SlicingLevels( Level );
		INT SliceCounts[3] = {Slicing.SlicesInX, Slicing.SlicesInY, Slicing.SlicesInZ };

		// Slice each piece in Source and put the resulting pieces in Target.
		for( INT SourceNum = 0; SourceNum < Source->Num(); ++SourceNum, ++ParentIndex )
		{
			FKConvexElem & Piece = (*Source)(SourceNum);
			FVector Center;
			FVector Extents;
			Piece.ElemBox.GetCenterAndExtents( Center, Extents );

			LevelPieces.AddZeroed(1);
			LevelPieces(0) = Piece;

			// Keep track of the expected volume, and reject slices that produce extremely small pieces
			FLOAT Area, MinimumVolume;
			Piece.CalcSurfaceAreaAndVolume( Area, MinimumVolume );
			MinimumVolume *= VolumeRejectionRatio;

			for( INT SliceDir = 0; SliceDir < 3; ++SliceDir )
			{
				if( SliceCounts[SliceDir] <= 0 )
				{
					continue;
				}
				MinimumVolume /= SliceCounts[SliceDir]+1;	// Resulting slices must have at least this volume
				const FLOAT LinearNoise = Clamp( Slicing.LinearNoise[SliceDir], 0.0f, 1.0f );
				const FLOAT AngularNoise = Clamp( Slicing.AngularNoise[SliceDir], 0.0f, 90.0f );
				const FLOAT SliceWidth = 2.0f*Extents[SliceDir]/(FLOAT)(SliceCounts[SliceDir]+1);
				FVector SlicePoint = Center;
				SlicePoint[SliceDir] -= Extents[SliceDir];
				for( INT SliceNum = 0; SliceNum < SliceCounts[SliceDir]; ++SliceNum )
				{
					// Orient the plane (+apply the angular noise)
					FPlane Plane( RandomNormal( SliceDir, AngularNoise*(PI/180.0f) ), 0.0f );
					// Compute the W parameter (+apply the linear noise)
					SlicePoint[SliceDir] += SliceWidth;
					Plane.W = Plane.PlaneDot(SlicePoint) + SliceWidth*LinearNoise*(appFrand()-0.5f);
					// Slice!
					INT LevelNum = LevelPieces.Num();
					for( INT OriginalIndex = 0; OriginalIndex < LevelNum; ++OriginalIndex )
					{
						if( LevelPieces(OriginalIndex).ElemBox.IsValid )
						{
							FLOAT Volume;
							FKConvexElem TempA = LevelPieces(OriginalIndex);
							TempA.SliceHull( Plane );
							TempA.CalcSurfaceAreaAndVolume( Area, Volume );
							if( Volume >= MinimumVolume )
							{
								FKConvexElem TempB = LevelPieces(OriginalIndex);
								TempB.SliceHull( Plane*(-1.0f) );
								TempB.CalcSurfaceAreaAndVolume( Area, Volume );
								if( Volume >= MinimumVolume )
								{	// Both pieces are big enough
									INT SliceIndex = LevelPieces.AddZeroed(1);
									LevelPieces( OriginalIndex ) = TempA;
									LevelPieces( SliceIndex ) = TempB;
								}
							}
						}
					}
				}
			}

			for( INT LevelPieceIndex = 0; LevelPieceIndex < LevelPieces.Num(); ++LevelPieceIndex )
			{
				if( LevelPieces(LevelPieceIndex).ElemBox.IsValid )
				{
					TArray<INT> InNeighbours;
					TArray<INT> OutNeighbours;
					TArray<FLOAT> OutNeighbourDims;

					// For now just dummy this up - would be nice to get neighbor info!
					InNeighbours.Empty();
					InNeighbours.AddZeroed( LevelPieces(LevelPieceIndex).FacePlaneData.Num() );

					TArray<FStaticMeshTriangle> ClippedExteriorTris;
					TArray<FStaticMeshTriangle> ClippedInteriorTris;
					ClipMeshWithHull( FragmentTriangles,
									  LevelPieces(LevelPieceIndex).FacePlaneData,
									  InNeighbours, InteriorMaterialIndex, 
									  ClippedExteriorTris, ClippedInteriorTris, OutNeighbours, OutNeighbourDims );

					TArray<FStaticMeshTriangle> Triangles;
					Triangles.Append( ClippedExteriorTris );
					Triangles.Append( ClippedInteriorTris );

					if( Triangles.Num() > 0 )
					{
						INT ChildIndex = Target->AddItem( LevelPieces(LevelPieceIndex) );
						if( ChildIndex > 65535 )
						{
							debugf( TEXT("Fragment %d of %s generated too many chunks (64k limit).  Aborting."), FragmentIndex, *FracturedStaticMesh->GetName() );
							return FALSE;
						}
						AddBoneInfo( PhysXDestructibleAsset->ChunkTree, SkeletalMeshInfo, Triangles, ParentIndex, Level+1 );
					}
				}
			}
			LevelPieces.Empty();
		}

		Source->Empty();
		Swap( Source, Target );
	}

	// Create skeletal meshes, assign materials, and create physics assets
	for( INT MeshIndex = 0; MeshIndex < SkeletalMeshInfo.Num(); ++MeshIndex )
	{
		if ( Elements.Num() )
		{
			SkeletalMeshInfo( MeshIndex ).Materials.Add( Elements.Num() );
			for( INT ElementIndex = 0; ElementIndex < Elements.Num(); ++ElementIndex )
			{
				SkeletalMeshInfo( MeshIndex ).Materials( ElementIndex ) = Elements( ElementIndex ).Material;
			}
		}
		USkeletalMesh* SkeletalMesh = ConstructObject<USkeletalMesh>( USkeletalMesh::StaticClass(), PhysXDestructibleAsset, NAME_None, RF_Public|RF_Standalone|RF_Transactional );
		CreateSkeletalMesh( SkeletalMesh, &SkeletalMeshInfo( MeshIndex ) );
		PhysXDestructibleAsset->Meshes.AddItem( SkeletalMesh );
		UPhysicsAsset* PhysicsAsset = ConstructObject<UPhysicsAsset>( UPhysicsAsset::StaticClass(), PhysXDestructibleAsset, NAME_None, RF_Public|RF_Standalone|RF_Transactional );
		CreatePhysicsAsset( SkeletalMesh, PhysicsAsset, 2, 16, 0.1);
		PhysXDestructibleAsset->Assets.AddItem( PhysicsAsset );
	}

	// Finalize ChunkTree
	for( INT ChunkIndex = 0; ChunkIndex < PhysXDestructibleAsset->ChunkTree.Num(); ++ChunkIndex )
	{
		FPhysXDestructibleAssetChunk & Chunk = PhysXDestructibleAsset->ChunkTree( ChunkIndex );
		USkeletalMesh * SkeletalMesh = PhysXDestructibleAsset->Meshes( Chunk.MeshIndex );
		UPhysicsAsset * PhysicsAsset = PhysXDestructibleAsset->Assets( Chunk.MeshIndex );
		Chunk.BoneName = SkeletalMesh->RefSkeleton( Chunk.BoneIndex ).Name;
		Chunk.BodyIndex = PhysicsAsset->FindBodyIndex( Chunk.BoneName );
		Chunk.FragmentIndex = FragmentIndex;
		FLOAT Area;
		PhysXDestructibleAsset->ComputeChunkSurfaceAreaAndVolume( ChunkIndex, Area, Chunk.Volume );
	}

	const INT NumChunks = PhysXDestructibleAsset->ChunkTree.Num();
	if( NumChunks > 0 )
	{
		FLOAT RecipRootVolume = PhysXDestructibleAsset->ChunkTree( 0 ).Volume;
		RecipRootVolume = RecipRootVolume > 0.0f ? 1.0f/RecipRootVolume : 0.0f;
		for( INT ChunkIndex = PhysXDestructibleAsset->ChunkTree.Num(); ChunkIndex-- > 0; )
		{
			FPhysXDestructibleAssetChunk & Chunk = PhysXDestructibleAsset->ChunkTree( ChunkIndex );
			Chunk.Size = Chunk.Volume*RecipRootVolume;
		}
		PhysXDestructibleAsset->MaxDepth = PhysXDestructibleAsset->ChunkTree( NumChunks-1 ).Depth;
	}

	return TRUE;
}

/**
* Creates a PhysXDestructible by slicing up each fragment of the given static mesh.
* 
* @param	StaticMesh		The source mesh.
* @param	FractureOptions	The fracture options.
* @param	Outer			The resulting UPhysXDestructible owner.
* @param	Name			The resulting UPhysXDestructible name.
* @param	SetFlags		The resulting UPhysXDestructible flags to set.
*/
void PhysXFractureMesh( UFracturedStaticMesh* FracturedStaticMesh, 
						UPhysXFractureOptions* FractureOptions,
						UObject* Outer = INVALID_OBJECT, 
						FName Name = NAME_None,
						EObjectFlags SetFlags = 0)
{
	FName UniqueName = UObject::MakeUniqueObjectName( Outer, UPhysXDestructible::StaticClass(), Name );
	UPhysXDestructible* PhysXDestructible = new( Outer, UniqueName, SetFlags ) UPhysXDestructible();

	UBOOL Success = TRUE;
	INT ChunkCount = 0;
	INT MaxDepth = 0;

	for( INT Index = 0; Index < FracturedStaticMesh->GetNumFragments(); ++Index )
	{
		// Create a PhysXDestructibleAsset
		UPhysXDestructibleAsset* PhysXDestructibleAsset =
			ConstructObject<UPhysXDestructibleAsset>( UPhysXDestructibleAsset::StaticClass(), Outer, NAME_None, RF_Public|RF_Standalone|RF_Transactional );
		if( PhysXDestructibleAsset == NULL )
		{
			debugf( TEXT("Failed to create UPhysXDestructibleAsset for %s, fragment index %d"), *FracturedStaticMesh->GetName(), Index );
		}
		UBOOL FragmentSuccess = CreatePhysXDestructibleAsset( PhysXDestructibleAsset, FracturedStaticMesh, Index, FractureOptions, Outer );
		if( !FragmentSuccess )
		{
			debugf( TEXT("Fragment %d of %s failed."), Index, *FracturedStaticMesh->GetName() );
			Success = FALSE;
			break;
		}
		ChunkCount += PhysXDestructibleAsset->ChunkTree.Num();
		if( ChunkCount > 65535 )
		{
			debugf( TEXT("Failed to build UPhysXDestructibleAsset for %s, too many chunks (64k limit)."), *FracturedStaticMesh->GetName() );
			Success = FALSE;
			break;
		}
		PhysXDestructible->DestructibleAssets.AddItem( PhysXDestructibleAsset );
		MaxDepth = Max( PhysXDestructibleAsset->MaxDepth, MaxDepth );
	}

	if( Success )
	{
		PhysXDestructible->FracturedStaticMesh = FracturedStaticMesh;
		// Fill depth-dependent parameter array, and set level(0) to the more expensive settings
		// We need MaxDepth+1 levels for each PhysXDestructiblePart [0,1,...,MaxDepth].  We add one level to
		// that to represent the PhysXDestructibleActor which contains that part.
		PhysXDestructible->DestructibleParameters.DepthParameters.AddZeroed( MaxDepth+2 );
		FPhysXDestructibleDepthParameters & Depth0Parameters = PhysXDestructible->DestructibleParameters.DepthParameters(0);
		Depth0Parameters.bTakeImpactDamage = TRUE;
		Depth0Parameters.bPlaySoundEffect = TRUE;
		Depth0Parameters.bPlayParticleEffect = TRUE;
		Depth0Parameters.bDoNotTimeOut = TRUE;
	}
	else
	{
		PhysXDestructible->DestructibleAssets.Empty();
	}

	GCallbackEvent->Send( CALLBACK_RefreshEditor_GenericBrowser );
}
