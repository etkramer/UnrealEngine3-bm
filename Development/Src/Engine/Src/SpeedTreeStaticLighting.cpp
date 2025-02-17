/*=============================================================================
	SpeedTreeStaticLighting.cpp: SpeedTreeComponent static lighting implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "SpeedTreeStaticLighting.h"

#if WITH_SPEEDTREE

void FStaticLightingSpeedTreeRayTracer::Init()
{
	// Build a kDOP tree for the tree's triangles.
	TArray<FkDOPBuildCollisionTriangle<WORD> > kDopTriangles;
	for(UINT TriangleIndex = 0;TriangleIndex < MeshElement.NumPrimitives;TriangleIndex++)
	{
		// Read the triangle from the mesh.
		const INT I0 = Indices(MeshElement.FirstIndex + TriangleIndex * 3 + 0);
		const INT I1 = Indices(MeshElement.FirstIndex + TriangleIndex * 3 + 1);
		const INT I2 = Indices(MeshElement.FirstIndex + TriangleIndex * 3 + 2);
		const FStaticLightingVertex& V0 = Vertices(I0);
		const FStaticLightingVertex& V1 = Vertices(I1);
		const FStaticLightingVertex& V2 = Vertices(I2);

		// Compute the triangle's normal.
		const FVector TriangleNormal = (V2.WorldPosition - V0.WorldPosition) ^ (V1.WorldPosition - V0.WorldPosition);

		// Compute the triangle area.
		const FLOAT TriangleArea = TriangleNormal.Size() * 0.5f;

		// Add the triangle to the list of kDOP triangles.
		new(kDopTriangles) FkDOPBuildCollisionTriangle<WORD>(I0,I1,I2,0,V0.WorldPosition,V1.WorldPosition,V2.WorldPosition);
	}
	kDopTree.Build(kDopTriangles);
}

FLightRayIntersection FStaticLightingSpeedTreeRayTracer::IntersectLightRay(const FVector& Start,const FVector& End) const
{
	// Check the line against the triangles.
	FVector CurrentStart = Start;
	while(TRUE)
	{
		FCheckResult CheckResult(1.0f);
		TkDOPLineCollisionCheck<const FStaticLightingSpeedTreeRayTracer,WORD> kDOPCheck(
			CurrentStart,
			End,
			0,
			*this,
			&CheckResult
			);
		if(kDopTree.LineCheck(kDOPCheck))
		{
			// Treat the opacity as a probability that the surface will block any particular ray.
			if(appFrand() < Opacity)
			{
				// Setup a vertex to represent the intersection.
				FStaticLightingVertex IntersectionVertex;
				IntersectionVertex.WorldPosition = CurrentStart + (End - CurrentStart) * CheckResult.Time;
				IntersectionVertex.WorldTangentZ = kDOPCheck.GetHitNormal();
				return FLightRayIntersection(TRUE,IntersectionVertex);
			}
			else
			{
				// If the ray fails the opacity probability check on this triangle, start the ray again behind the surface.
				const FVector Direction = (End - CurrentStart);
				const FLOAT NextSegmentStartTime = (CheckResult.Time + 16.0f / Direction.Size());
				if(NextSegmentStartTime < 1.0f)
				{
					CurrentStart = CurrentStart + Direction * NextSegmentStartTime;
				}
				else
				{
					return FLightRayIntersection::None();
				}
			}
		}
		else
		{
			// If the ray didn't intersect any triangles, return no intersection.
			return FLightRayIntersection::None();
		}
	}
}

FSpeedTreeStaticLightingMesh::FSpeedTreeStaticLightingMesh(
	FSpeedTreeComponentStaticLighting* InComponentStaticLighting,
	INT InLODIndex,
	const FMeshElement& InMeshElement,
	ESpeedTreeMeshType InMeshType,
	FLOAT InShadowOpacity,
	UBOOL bInTwoSidedMaterial,
	const TArray<ULightComponent*>& InRelevantLights,
	const FLightingBuildOptions& Options
	):
	FStaticLightingMesh(
		InMeshElement.NumPrimitives,
		InMeshElement.MaxVertexIndex - InMeshElement.MinVertexIndex + 1,
		InShadowOpacity > 0.0f,
		InComponentStaticLighting->GetComponent()->bSelfShadowOnly,
		bInTwoSidedMaterial,
		InRelevantLights,
		InComponentStaticLighting->GetComponent()->Bounds.GetBox()
		),
	ComponentStaticLighting(InComponentStaticLighting),
	LODIndex(InLODIndex),
	MeshElement(InMeshElement),
	MeshType(InMeshType),
	RayTracer(InComponentStaticLighting->GetVertices(InMeshType),InComponentStaticLighting->GetIndices(),InMeshElement,InShadowOpacity)
{
	// If the mesh casts shadows, build the kDOP tree.
	if(InShadowOpacity > 0.0f)
	{
		RayTracer.Init();
	}
}

void FSpeedTreeStaticLightingMesh::GetTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2) const
{
	// Read the triangle's vertex indices.
	const INT I0 = ComponentStaticLighting->GetIndices()(MeshElement.FirstIndex + TriangleIndex * 3 + 0);
	const INT I1 = ComponentStaticLighting->GetIndices()(MeshElement.FirstIndex + TriangleIndex * 3 + 1);
	const INT I2 = ComponentStaticLighting->GetIndices()(MeshElement.FirstIndex + TriangleIndex * 3 + 2);

	// Read the triangle's vertices.
	OutV0 = ComponentStaticLighting->GetVertices(MeshType)(I0);
	OutV1 = ComponentStaticLighting->GetVertices(MeshType)(I1);
	OutV2 = ComponentStaticLighting->GetVertices(MeshType)(I2);
}

void FSpeedTreeStaticLightingMesh::GetTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const
{
	// Read the triangle's vertex indices.
	OutI0 = ComponentStaticLighting->GetIndices()(MeshElement.FirstIndex + TriangleIndex * 3 + 0) - MeshElement.MinVertexIndex;
	OutI1 = ComponentStaticLighting->GetIndices()(MeshElement.FirstIndex + TriangleIndex * 3 + 1) - MeshElement.MinVertexIndex;
	OutI2 = ComponentStaticLighting->GetIndices()(MeshElement.FirstIndex + TriangleIndex * 3 + 2) - MeshElement.MinVertexIndex;
}

UBOOL FSpeedTreeStaticLightingMesh::ShouldCastShadow(ULightComponent* Light,const FStaticLightingMapping* Receiver) const
{
	// Check if the receiver is in the same SpeedTreeComponent and LOD as this mesh.
	UBOOL bReceiverIsSameLOD = FALSE;
	UBOOL bReceiverIsSameComponent = FALSE;
	if(ComponentStaticLighting->IsMappingFromThisComponent(Receiver))
	{
		FSpeedTreeStaticLightingMapping* ReceiverSpeedTreeMapping = (FSpeedTreeStaticLightingMapping*)Receiver;
		bReceiverIsSameComponent = TRUE;
		bReceiverIsSameLOD = (ReceiverSpeedTreeMapping->GetLODIndex() == LODIndex);
	}

	const UBOOL bThisIsHighestLOD = (LODIndex == 0);
	if(bThisIsHighestLOD)
	{
		// The highest LOD doesn't shadow the other LODs.
		if(bReceiverIsSameComponent && !bReceiverIsSameLOD)
		{
			return FALSE;
		}
	}
	else
	{
		// The lower LODs only shadow themselves.
		if(!(bReceiverIsSameComponent && bReceiverIsSameLOD))
		{
			return FALSE;
		}
	}

	return FStaticLightingMesh::ShouldCastShadow(Light,Receiver);
}

UBOOL FSpeedTreeStaticLightingMesh::IsUniformShadowCaster() const
{
	return FALSE;
}

FLightRayIntersection FSpeedTreeStaticLightingMesh::IntersectLightRay(const FVector& Start,const FVector& End,UBOOL bFindNearestIntersection) const
{
	return RayTracer.IntersectLightRay(Start,End);
}

FSpeedTreeStaticLightingMapping::FSpeedTreeStaticLightingMapping(FStaticLightingMesh* InMesh,FSpeedTreeComponentStaticLighting* InComponentStaticLighting,INT InLODIndex,const FMeshElement& InMeshElement,ESpeedTreeMeshType InMeshType):
	FStaticLightingVertexMapping(InMesh,InComponentStaticLighting->GetComponent(),InComponentStaticLighting->GetComponent()->bForceDirectLightMap,1.0f / Square(16.0f),FALSE),
	ComponentStaticLighting(InComponentStaticLighting),
	LODIndex(InLODIndex),
	MeshElement(InMeshElement),
	MeshType(InMeshType)
{}

void FSpeedTreeStaticLightingMapping::Apply(FLightMapData1D* LightMapData,const TMap<ULightComponent*,FShadowMapData1D*>& ShadowMapData)
{
	// Pass the static lighting data on to the component's static lighting manager.
	ComponentStaticLighting->ApplyCompletedMapping(this,LightMapData,ShadowMapData);
}

/**
 * Computes the local-space vertex position for a leaf mesh vertex.
 * @param PivotPoint - The leaf mesh vertex's pivot point.
 * @param VertexData - The leaf mesh vertex's data.
 * @return the local-space vertex position.
 */
static FVector GetLeafMeshVertexPosition(const FVector& PivotPoint,const FSpeedTreeVertexDataLeafMesh& VertexData)
{
	const FMatrix LeafToLocal = FBasisVectorMatrix(
		VertexData.LeafData2,
		(FVector)VertexData.LeafData2 ^ (FVector)VertexData.LeafData3,
		VertexData.LeafData3,
		FVector(0,0,0)
		);
	const FVector4 PositionRelativeToPivotPoint = LeafToLocal.TransformFVector(VertexData.LeafData1);
	return PivotPoint + PositionRelativeToPivotPoint;
}

/**
 * Computes the local-space vertex position for a leaf card vertex.
 * @param PivotPoint - The leaf card vertex's pivot point.
 * @param VertexData - The leaf card vertex's data.
 * @return the local-space vertex position.
 */
static FVector GetLeafCardVertexPosition(const FVector& PivotPoint,const FSpeedTreeVertexDataLeafCard& VertexData)
{
	static const FVector LeafUnitSquare[4] =
	{
		FVector(0.5f, 0.5f, 0.0f),
		FVector(-0.5f, 0.5f, 0.0f),
		FVector(-0.5f, -0.5f, 0.0f),
		FVector(0.5f, -0.5f, 0.0f)
	};
	const FVector PositionRelativeToPivotPoint = 
		(	LeafUnitSquare[(INT)VertexData.LeafData2.W] +
			FVector(0.0f,VertexData.LeafData1.Z,VertexData.LeafData1.W)
			) *
		FVector(VertexData.LeafData1.X,VertexData.LeafData1.X,VertexData.LeafData1.Y);\

	return PivotPoint + PositionRelativeToPivotPoint;
}

FSpeedTreeComponentStaticLighting::FSpeedTreeComponentStaticLighting(USpeedTreeComponent* InComponent,const TArray<ULightComponent*>& InRelevantLights):
	Component(InComponent),
	RelevantLights(InRelevantLights),
	Indices(InComponent->SpeedTree->SRH->IndexBuffer.Indices),
	BranchAndFrondLightMapData(InComponent->SpeedTree->SRH->BranchFrondDataBuffer.Vertices.Num()),
	LeafMeshLightMapData(InComponent->SpeedTree->SRH->LeafMeshDataBuffer.Vertices.Num()),
	LeafCardLightMapData(InComponent->SpeedTree->SRH->LeafCardDataBuffer.Vertices.Num()),
	BillboardLightMapData(InComponent->SpeedTree->SRH->BillboardDataBuffer.Vertices.Num()),
	bHasBranchAndFrondStaticLighting(FALSE),
	bHasLeafMeshStaticLighting(FALSE),
	bHasLeafCardStaticLighting(FALSE),
	bHasBillboardStaticLighting(FALSE),
	NumCompleteMappings(0)
{
	// Compute the combined local to world transform, including rotation.
	const FMatrix LocalToWorld = Component->RotationOnlyMatrix.Inverse() * Component->LocalToWorld;

	// Compute the inverse transpose of the local to world transform for transforming normals.
	const FMatrix LocalToWorldInverseTranspose = LocalToWorld.Inverse().Transpose();

	// Set up the lighting vertices.
	for(INT MeshType = STMT_Min;MeshType < STMT_Max;MeshType++)
	{
		// Find the vertex arrays for this current mesh type.
		const TArray<FSpeedTreeVertexPosition>& SourceVertexPositionArray =
			ChooseByMeshType< TArray<FSpeedTreeVertexPosition> >(
				MeshType,
				Component->SpeedTree->SRH->BranchFrondPositionBuffer.Vertices,
				Component->SpeedTree->SRH->BranchFrondPositionBuffer.Vertices,
				Component->SpeedTree->SRH->LeafMeshPositionBuffer.Vertices,
				Component->SpeedTree->SRH->LeafCardPositionBuffer.Vertices,
				Component->SpeedTree->SRH->BillboardPositionBuffer.Vertices
				);
		const INT NumVertices =
			ChooseByMeshType<INT>(
					MeshType,
					Component->SpeedTree->SRH->BranchFrondDataBuffer.Vertices.Num(),
					Component->SpeedTree->SRH->BranchFrondDataBuffer.Vertices.Num(),
					Component->SpeedTree->SRH->LeafMeshDataBuffer.Vertices.Num(),
					Component->SpeedTree->SRH->LeafCardDataBuffer.Vertices.Num(),
					Component->SpeedTree->SRH->BillboardDataBuffer.Vertices.Num()
					);
		TArray<FStaticLightingVertex>& DestVertices =
			*ChooseByMeshType< TArray<FStaticLightingVertex>* >(
				MeshType,
				&BranchAndFrondVertices,
				&BranchAndFrondVertices,
				&LeafMeshVertices,
				&LeafCardVertices,
				&BillboardVertices
				);

		// Copy and convert the mesh's vertices to the static lighting vertex arrays.
		DestVertices.Empty(NumVertices);
		for(INT VertexIndex = 0;VertexIndex < NumVertices;VertexIndex++)
		{
			FVector VertexPosition = SourceVertexPositionArray(VertexIndex).Position;
			switch(MeshType)
			{
			case STMT_LeafMeshes:
				VertexPosition = GetLeafMeshVertexPosition(
					VertexPosition,
					Component->SpeedTree->SRH->LeafMeshDataBuffer.Vertices(VertexIndex)
					);
				break;
			case STMT_LeafCards:
				VertexPosition = GetLeafCardVertexPosition(
					VertexPosition,
					Component->SpeedTree->SRH->LeafCardDataBuffer.Vertices(VertexIndex)
					);
				break;
			}

			const FSpeedTreeVertexData* SourceVertexData = GetSpeedTreeVertexData(Component->SpeedTree->SRH,MeshType,VertexIndex);

			FStaticLightingVertex* DestVertex = new(DestVertices) FStaticLightingVertex;
			DestVertex->WorldPosition = LocalToWorld.TransformFVector(VertexPosition);
			DestVertex->WorldTangentX = LocalToWorld.TransformNormal(SourceVertexData->TangentX).SafeNormal();
			DestVertex->WorldTangentY = LocalToWorld.TransformNormal(SourceVertexData->TangentY).SafeNormal();
			DestVertex->WorldTangentZ = LocalToWorldInverseTranspose.TransformNormal(SourceVertexData->TangentZ).SafeNormal();
		}
	}
}

void FSpeedTreeComponentStaticLighting::CreateMapping(
	FStaticLightingPrimitiveInfo& OutPrimitiveInfo,
	INT LODIndex,
	const FMeshElement& MeshElement,
	ESpeedTreeMeshType MeshType,
	FLOAT ShadowOpacity,
	UBOOL bInTwoSidedMaterial,
	const FLightingBuildOptions& Options
	)
{
	if(MeshElement.NumPrimitives > 0)
	{
		// Create the static lighting mesh.
		FSpeedTreeStaticLightingMesh* const Mesh = new FSpeedTreeStaticLightingMesh(
			this,
			LODIndex,
			MeshElement,
			MeshType,
			ShadowOpacity,
			bInTwoSidedMaterial,
			RelevantLights,
			Options
			);

		// Create the static lighting mapping.
		FSpeedTreeStaticLightingMapping* const Mapping = new FSpeedTreeStaticLightingMapping(Mesh,this,LODIndex,MeshElement,MeshType);

		// Add the mapping to the mappings list.
		Mappings.AddItem(Mapping);

		// Add the mapping and mesh to the static lighting system's list of meshes and mappings for the primitive.
		OutPrimitiveInfo.Meshes.AddItem(Mesh);
		OutPrimitiveInfo.Mappings.AddItem(Mapping);

		// Flag the component as having static lighting for this mesh type.
		UBOOL& bComponentHasStaticLightingForThisMeshType =
			*ChooseByMeshType<UBOOL*>(MeshType,&bHasBranchAndFrondStaticLighting,&bHasBranchAndFrondStaticLighting,&bHasLeafMeshStaticLighting,&bHasLeafCardStaticLighting,&bHasBillboardStaticLighting);
		bComponentHasStaticLightingForThisMeshType = TRUE;
	}
}

void FSpeedTreeComponentStaticLighting::ApplyCompletedMapping(
	FSpeedTreeStaticLightingMapping* Mapping,
	FLightMapData1D* MappingLightMapData,
	const TMap<ULightComponent*,FShadowMapData1D*>& MappingShadowMapData
	)
{
	const INT MinVertexIndex = Mapping->GetMeshElement().MinVertexIndex;

	if(MappingLightMapData)
	{
		// Copy the mapping's light-map data into the component's light-map data.
		FLightMapData1D& DestLightMapData = 
			*ChooseByMeshType<FLightMapData1D*>(
				Mapping->GetMeshType(),
				&BranchAndFrondLightMapData,
				&BranchAndFrondLightMapData,
				&LeafMeshLightMapData,
				&LeafCardLightMapData,
				&BillboardLightMapData
				);
		for(INT VertexIndex = 0;VertexIndex < MappingLightMapData->GetSize();VertexIndex++)
		{
			DestLightMapData(MinVertexIndex + VertexIndex) = (*MappingLightMapData)(VertexIndex);
		}

		// Merge the light-map's light list into the light list for all the component's light-maps.
		for(INT LightIndex = 0;LightIndex < MappingLightMapData->Lights.Num();LightIndex++)
		{
			ULightComponent* Light = MappingLightMapData->Lights(LightIndex);
			BranchAndFrondLightMapData.Lights.AddUniqueItem(Light);
			LeafMeshLightMapData.Lights.AddUniqueItem(Light);
			LeafCardLightMapData.Lights.AddUniqueItem(Light);
			BillboardLightMapData.Lights.AddUniqueItem(Light);
		}

		// Delete the mapping's temporary light-map data.
		delete MappingLightMapData;
		MappingLightMapData = NULL;
	}

	// Merge the shadow-map data into the component's shadow-map data.
	for(TMap<ULightComponent*,FShadowMapData1D*>::TConstIterator ShadowMapIt(MappingShadowMapData);ShadowMapIt;++ShadowMapIt)
	{
		// Find the existing shadow-maps for this light.
		TRefCountPtr<FLightShadowMaps>* LightShadowMaps = ComponentShadowMaps.Find(ShadowMapIt.Key());
		if(!LightShadowMaps)
		{
			// Create the shadow-maps for the light if they don't exist yet.
			LightShadowMaps = &ComponentShadowMaps.Set(
				ShadowMapIt.Key(),
				new FLightShadowMaps(
					Component->SpeedTree->SRH->BranchFrondDataBuffer.Vertices.Num(),
					Component->SpeedTree->SRH->LeafMeshDataBuffer.Vertices.Num(),
					Component->SpeedTree->SRH->LeafCardDataBuffer.Vertices.Num(),
					Component->SpeedTree->SRH->BillboardDataBuffer.Vertices.Num()
					)
				);
		}

		// Copy the mapping's shadow-map data into the component's shadow-map data.
		FShadowMapData1D& DestShadowMapData = 
			*ChooseByMeshType<FShadowMapData1D*>(
				Mapping->GetMeshType(),
				&(*LightShadowMaps)->BranchAndFrondShadowMapData,
				&(*LightShadowMaps)->BranchAndFrondShadowMapData,
				&(*LightShadowMaps)->LeafMeshShadowMapData,
				&(*LightShadowMaps)->LeafCardShadowMapData,
				&(*LightShadowMaps)->BillboardShadowMapData
				);
		for(INT VertexIndex = 0;VertexIndex < ShadowMapIt.Value()->GetSize();VertexIndex++)
		{
			DestShadowMapData(MinVertexIndex + VertexIndex) = (*ShadowMapIt.Value())(VertexIndex);
		}

		// Delete the mapping's temporary shadow-map data.
		delete ShadowMapIt.Value();
	}

	// Check if static lighting has been built for all the component's mappings.
	if(++NumCompleteMappings == Mappings.Num())
	{
		const INT NumLightmappedLights = BranchAndFrondLightMapData.Lights.Num();
		check(BranchAndFrondLightMapData.Lights.Num() == NumLightmappedLights);
		check(LeafMeshLightMapData.Lights.Num() == NumLightmappedLights);
		check(LeafCardLightMapData.Lights.Num() == NumLightmappedLights);
		check(BillboardLightMapData.Lights.Num() == NumLightmappedLights);

		// Create the component's branch and frond light-map.
		Component->BranchAndFrondLightMap = (bHasBranchAndFrondStaticLighting && NumLightmappedLights) ? new FLightMap1D(Component,BranchAndFrondLightMapData) : NULL;

		// Create the component's leaf mesh light-map.
		Component->LeafMeshLightMap = (bHasLeafMeshStaticLighting && NumLightmappedLights) ? new FLightMap1D(Component,LeafMeshLightMapData) : NULL;

		// Create the component's leaf card light-map.
		Component->LeafCardLightMap = (bHasLeafCardStaticLighting && NumLightmappedLights) ? new FLightMap1D(Component,LeafCardLightMapData) : NULL;
		
		// Create the component's billboard light-map.
		Component->BillboardLightMap = (bHasBillboardStaticLighting && NumLightmappedLights) ? new FLightMap1D(Component,BillboardLightMapData) : NULL;

		// Create the component's shadow-maps.
		Component->StaticLights.Empty();
		for(TMap<ULightComponent*,TRefCountPtr<FLightShadowMaps> >::TConstIterator ShadowMapIt(ComponentShadowMaps);ShadowMapIt;++ShadowMapIt)
		{
			const ULightComponent* Light = ShadowMapIt.Key();
			FSpeedTreeStaticLight* StaticLight = new(Component->StaticLights) FSpeedTreeStaticLight;
			StaticLight->Guid = Light->LightGuid;
			StaticLight->BranchAndFrondShadowMap = bHasBranchAndFrondStaticLighting ? new(Component) UShadowMap1D(Light->LightGuid,ShadowMapIt.Value()->BranchAndFrondShadowMapData) : NULL;
			StaticLight->LeafMeshShadowMap = bHasLeafMeshStaticLighting ? new(Component) UShadowMap1D(Light->LightGuid,ShadowMapIt.Value()->LeafMeshShadowMapData) : NULL;
			StaticLight->LeafCardShadowMap = bHasLeafCardStaticLighting ? new(Component) UShadowMap1D(Light->LightGuid,ShadowMapIt.Value()->LeafCardShadowMapData) : NULL;
			StaticLight->BillboardShadowMap = bHasBillboardStaticLighting ? new(Component) UShadowMap1D(Light->LightGuid,ShadowMapIt.Value()->BillboardShadowMapData) : NULL;
		}

		// Add any relevant lights which we don't have shadow-maps for to the static light list.
		for(INT LightIndex = 0;LightIndex < RelevantLights.Num();LightIndex++)
		{
			ULightComponent* Light = RelevantLights(LightIndex);
			if(!BranchAndFrondLightMapData.Lights.ContainsItem(Light) && !ComponentShadowMaps.Find(Light))
			{
				FSpeedTreeStaticLight* StaticLight = new(Component->StaticLights) FSpeedTreeStaticLight;
				StaticLight->Guid = Light->LightGuid;
				StaticLight->BranchAndFrondShadowMap = NULL;
				StaticLight->LeafMeshShadowMap = NULL;
				StaticLight->LeafCardShadowMap = NULL;
				StaticLight->BillboardShadowMap = NULL;
			}
		}
	}
}

void USpeedTreeComponent::GetStaticLightingInfo(FStaticLightingPrimitiveInfo& OutPrimitiveInfo,const TArray<ULightComponent*>& InRelevantLights,const FLightingBuildOptions& Options)
{
	if(IsValidComponent() && HasStaticShadowing())
	{
		// Count the number of LODs.
		INT NumLODs = Max(
			Max(
				SpeedTree->SRH->FrondElements.Num(),
				SpeedTree->SRH->BranchElements.Num()
				),
			Max(
				SpeedTree->SRH->LeafMeshElements.Num(),
				SpeedTree->SRH->LeafCardElements.Num()
				)
			);

		// Create the component static lighting object which aggregates all the static lighting mappings used by the component.
		FSpeedTreeComponentStaticLighting* ComponentStaticLighting = new FSpeedTreeComponentStaticLighting(this,InRelevantLights);

		// Create static lighting mappings and meshes for each LOD.
		for(INT LODIndex = 0;LODIndex < NumLODs;LODIndex++)
		{
			// Create the static lighting mesh and mapping for this LOD of the tree's branch triangles.
			if(bUseBranches && SpeedTree->SRH->bHasBranches && LODIndex < SpeedTree->SRH->BranchElements.Num())
			{
				ComponentStaticLighting->CreateMapping(OutPrimitiveInfo,LODIndex,SpeedTree->SRH->BranchElements(LODIndex),STMT_Branches,CastShadow ? 1.0f : 0.0f,FALSE,Options);
			}

			// Create the static lighting mesh and mapping for this LOD of the tree's frond triangles.
			if(bUseFronds && SpeedTree->SRH->bHasFronds && LODIndex < SpeedTree->SRH->FrondElements.Num())
			{
				ComponentStaticLighting->CreateMapping(OutPrimitiveInfo,LODIndex,SpeedTree->SRH->FrondElements(LODIndex),STMT_Fronds,0.0f,TRUE,Options);
			}

			// Create the static lighting mesh and mapping for this LOD of the tree's leaf triangles.
			if(bUseLeaves && SpeedTree->SRH->bHasLeaves && LODIndex < SpeedTree->SRH->LeafMeshElements.Num())
			{
				ComponentStaticLighting->CreateMapping(OutPrimitiveInfo,LODIndex,SpeedTree->SRH->LeafMeshElements(LODIndex),STMT_LeafMeshes,CastShadow ? SpeedTree->LeafStaticShadowOpacity : 0.0f,TRUE,Options);
			}
			if(bUseLeaves && SpeedTree->SRH->bHasLeaves && LODIndex < SpeedTree->SRH->LeafCardElements.Num())
			{
				ComponentStaticLighting->CreateMapping(OutPrimitiveInfo,LODIndex,SpeedTree->SRH->LeafCardElements(LODIndex),STMT_LeafCards,CastShadow ? SpeedTree->LeafStaticShadowOpacity : 0.0f,TRUE,Options);
			}
		}
		
		// Create the static lighting mesh and mapping for the tree's billboards.
		if(bUseBillboards)
		{
			ComponentStaticLighting->CreateMapping(OutPrimitiveInfo,0,SpeedTree->SRH->BillboardElement,STMT_Billboards,0.0f,TRUE,Options);
		}
	}
}

void USpeedTreeComponent::InvalidateLightingCache()
{
	// Save the static mesh state for transactions.
	Modify();

	// Mark lighting as requiring a rebuilt.
	MarkLightingRequiringRebuild();

	// Detach the component from the scene for the duration of this function.
	FComponentReattachContext ReattachContext(this);
	FlushRenderingCommands();

	// Discard all cached lighting.
	StaticLights.Empty( );
	BranchAndFrondLightMap = NULL;
	LeafCardLightMap = NULL;
	LeafMeshLightMap = NULL;
	BillboardLightMap = NULL;
}

#endif
