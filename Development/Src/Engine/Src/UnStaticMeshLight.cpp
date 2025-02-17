/*=============================================================================
UnStaticMeshLight.cpp: Static mesh lighting code.
Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnRaster.h"

/**
 * Creates a static lighting vertex to represent the given static mesh vertex.
 * @param VertexBuffer - The static mesh's vertex buffer.
 * @param VertexIndex - The index of the static mesh vertex to access.
 * @param OutVertex - Upon return, contains a static lighting vertex representing the specified static mesh vertex.
 */
static void GetStaticLightingVertex(
	const FPositionVertexBuffer& PositionVertexBuffer,
	const FStaticMeshVertexBuffer& VertexBuffer,
	UINT VertexIndex,
	const FMatrix& LocalToWorld,
	const FMatrix& LocalToWorldInverseTranspose,
	FStaticLightingVertex& OutVertex
	)
{
	OutVertex.WorldPosition = LocalToWorld.TransformFVector(PositionVertexBuffer.VertexPosition(VertexIndex));
	OutVertex.WorldTangentX = LocalToWorld.TransformNormal(VertexBuffer.VertexTangentX(VertexIndex)).SafeNormal();
	OutVertex.WorldTangentY = LocalToWorld.TransformNormal(VertexBuffer.VertexTangentY(VertexIndex)).SafeNormal();
	OutVertex.WorldTangentZ = LocalToWorldInverseTranspose.TransformNormal(VertexBuffer.VertexTangentZ(VertexIndex)).SafeNormal();

	checkSlow(VertexBuffer.GetNumTexCoords() <= ARRAY_COUNT(OutVertex.TextureCoordinates));
	for(UINT TextureCoordinateIndex = 0;TextureCoordinateIndex < VertexBuffer.GetNumTexCoords();TextureCoordinateIndex++)
	{
		OutVertex.TextureCoordinates[TextureCoordinateIndex] = VertexBuffer.GetVertexUV(VertexIndex,TextureCoordinateIndex);
	}
}


/** Initialization constructor. */
FStaticMeshStaticLightingMesh::FStaticMeshStaticLightingMesh(const UStaticMeshComponent* InPrimitive,INT InLODIndex,const TArray<ULightComponent*>& InRelevantLights):
	FStaticLightingMesh(
		InPrimitive->StaticMesh->LODModels(InLODIndex).GetTriangleCount(),
		InPrimitive->StaticMesh->LODModels(InLODIndex).NumVertices,
		InPrimitive->CastShadow | InPrimitive->bCastHiddenShadow,
		InPrimitive->bSelfShadowOnly,
		FALSE,
		InRelevantLights,
		InPrimitive->Bounds.GetBox()
		),
	LODIndex(InLODIndex),
	StaticMesh(InPrimitive->StaticMesh),
	Primitive(InPrimitive),
	LocalToWorldInverseTranspose(InPrimitive->LocalToWorld.Inverse().Transpose()),
	bReverseWinding(InPrimitive->LocalToWorldDeterminant < 0.0f)
{
}

// FStaticLightingMesh interface.

void FStaticMeshStaticLightingMesh::GetTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2) const
{
	const FStaticMeshRenderData& LODRenderData = StaticMesh->LODModels(LODIndex);

	// Lookup the triangle's vertex indices.
	const WORD I0 = LODRenderData.IndexBuffer.Indices(TriangleIndex * 3 + 0);
	const WORD I1 = LODRenderData.IndexBuffer.Indices(TriangleIndex * 3 + (bReverseWinding ? 2 : 1));
	const WORD I2 = LODRenderData.IndexBuffer.Indices(TriangleIndex * 3 + (bReverseWinding ? 1 : 2));

	// Translate the triangle's static mesh vertices to static lighting vertices.
	GetStaticLightingVertex(LODRenderData.PositionVertexBuffer,LODRenderData.VertexBuffer,I0,Primitive->LocalToWorld,LocalToWorldInverseTranspose,OutV0);
	GetStaticLightingVertex(LODRenderData.PositionVertexBuffer,LODRenderData.VertexBuffer,I1,Primitive->LocalToWorld,LocalToWorldInverseTranspose,OutV1);
	GetStaticLightingVertex(LODRenderData.PositionVertexBuffer,LODRenderData.VertexBuffer,I2,Primitive->LocalToWorld,LocalToWorldInverseTranspose,OutV2);
}

void FStaticMeshStaticLightingMesh::GetTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const
{
	const FStaticMeshRenderData& LODRenderData = StaticMesh->LODModels(LODIndex);

	// Lookup the triangle's vertex indices.
	OutI0 = LODRenderData.IndexBuffer.Indices(TriangleIndex * 3 + 0);
	OutI1 = LODRenderData.IndexBuffer.Indices(TriangleIndex * 3 + (bReverseWinding ? 2 : 1));
	OutI2 = LODRenderData.IndexBuffer.Indices(TriangleIndex * 3 + (bReverseWinding ? 1 : 2));
}

UBOOL FStaticMeshStaticLightingMesh::ShouldCastShadow(ULightComponent* Light,const FStaticLightingMapping* Receiver) const
{
	// If the receiver is the same primitive but a different LOD, don't cast shadows on it.
	if(OtherLODs.ContainsItem(Receiver->Mesh))
	{
		return FALSE;
	}
	else
	{
		return FStaticLightingMesh::ShouldCastShadow(Light,Receiver);
	}
}

/** @return		TRUE if the specified triangle casts a shadow. */
UBOOL FStaticMeshStaticLightingMesh::IsTriangleCastingShadow(UINT TriangleIndex) const
{
	// Find the mesh element containing the specified triangle.
	const FStaticMeshRenderData& LODRenderData = StaticMesh->LODModels(LODIndex);
	for ( INT ElementIndex = 0 ; ElementIndex < LODRenderData.Elements.Num() ; ++ElementIndex )
	{
		const FStaticMeshElement& Element = LODRenderData.Elements( ElementIndex );
		if ( ( TriangleIndex >= Element.FirstIndex / 3 ) && ( TriangleIndex < Element.FirstIndex / 3 + Element.NumTriangles ) )
		{
			return Element.bEnableShadowCasting;
		}
	}

	return TRUE;
}

/** @return		TRUE if the mesh wants to control shadow casting per element rather than per mesh. */
UBOOL FStaticMeshStaticLightingMesh::IsControllingShadowPerElement() const
{
	const FStaticMeshRenderData& LODRenderData = StaticMesh->LODModels(LODIndex);
	for ( INT ElementIndex = 0 ; ElementIndex < LODRenderData.Elements.Num() ; ++ElementIndex )
	{
		if ( !LODRenderData.Elements( ElementIndex ).bEnableShadowCasting )
		{
			return TRUE;
		}
	}
	return FALSE;
}

UBOOL FStaticMeshStaticLightingMesh::IsUniformShadowCaster() const
{
	// If this mesh is one of multiple LODs, it won't uniformly shadow all of them.
	return OtherLODs.Num() == 0 && FStaticLightingMesh::IsUniformShadowCaster();
}

FLightRayIntersection FStaticMeshStaticLightingMesh::IntersectLightRay(const FVector& Start,const FVector& End,UBOOL bFindNearestIntersection) const
{
	// Create the object that knows how to extract information from the component/mesh
	FStaticMeshCollisionDataProvider Provider(Primitive);

	// Create the check structure with all the local space fun
	FCheckResult Result(1.0f);
	TkDOPLineCollisionCheck<FStaticMeshCollisionDataProvider,WORD> kDOPCheck(Start,End,!bFindNearestIntersection ? TRACE_StopAtAnyHit : 0,Provider,&Result);

	// Do the line check
	const UBOOL bIntersects = StaticMesh->kDOPTree.LineCheck(kDOPCheck);

	// Setup a vertex to represent the intersection.
	FStaticLightingVertex IntersectionVertex;
	if(bIntersects)
	{
		IntersectionVertex.WorldPosition = Start + (End - Start) * Result.Time;
		IntersectionVertex.WorldTangentZ = kDOPCheck.GetHitNormal();
	}
	else
	{
		IntersectionVertex.WorldPosition.Set(0,0,0);
		IntersectionVertex.WorldTangentZ.Set(0,0,1);
	}
	return FLightRayIntersection(bIntersects,IntersectionVertex);
}


/** Represents a static mesh primitive with texture mapped static lighting. */
class FStaticMeshStaticLightingTextureMapping : public FStaticLightingTextureMapping
{
public:

	/** Initialization constructor. */
	FStaticMeshStaticLightingTextureMapping(UStaticMeshComponent* InPrimitive,INT InLODIndex,FStaticLightingMesh* InMesh,INT InSizeX,INT InSizeY,INT InTextureCoordinateIndex,UBOOL bPerformFullQualityRebuild):
		FStaticLightingTextureMapping(
			InMesh,
			InPrimitive,
			bPerformFullQualityRebuild ? InSizeX : InSizeX / 2,
			bPerformFullQualityRebuild ? InSizeY : InSizeY / 2,
			InTextureCoordinateIndex,
			InPrimitive->bForceDirectLightMap
			),
		Primitive(InPrimitive),
		LODIndex(InLODIndex)
	{}

	// FStaticLightingTextureMapping interface
	virtual void Apply(FLightMapData2D* LightMapData,const TMap<ULightComponent*,FShadowMapData2D*>& ShadowMapData)
	{
		// Determine the material to use for grouping the light-maps and shadow-maps.
		UMaterialInterface* const Material = Primitive->GetNumElements() == 1 ? Primitive->GetMaterial(0) : NULL;

		// Discard the existing static lighting data for LODs that no longer exist.
		if(Primitive->LODData.Num() > Primitive->StaticMesh->LODModels.Num())
		{
			Primitive->LODData.Remove(
				Primitive->StaticMesh->LODModels.Num(),
				Primitive->LODData.Num() - Primitive->StaticMesh->LODModels.Num()
				);
		}

		// Ensure LODData has enough entries in it.
		if(LODIndex >= Primitive->LODData.Num())
		{
			Primitive->LODData.AddZeroed(LODIndex + 1 - Primitive->LODData.Num());
		}
		FStaticMeshComponentLODInfo& ComponentLODInfo = Primitive->LODData(LODIndex);

		// Create a light-map for the primitive.
		ComponentLODInfo.LightMap = FLightMap2D::AllocateLightMap(
			Primitive,
			LightMapData,
			Material,
			Primitive->Bounds
			);
		delete LightMapData;

		// Create the shadow-maps for the primitive.
		ComponentLODInfo.ShadowVertexBuffers.Empty();
		ComponentLODInfo.ShadowMaps.Empty(ShadowMapData.Num());
		for(TMap<ULightComponent*,FShadowMapData2D*>::TConstIterator ShadowMapDataIt(ShadowMapData);ShadowMapDataIt;++ShadowMapDataIt)
		{
			ComponentLODInfo.ShadowMaps.AddItem(
				new(Owner) UShadowMap2D(
					*ShadowMapDataIt.Value(),
					ShadowMapDataIt.Key()->LightGuid,
					Material,
					Primitive->Bounds
					)
				);
			delete ShadowMapDataIt.Value();
		}

		// Build the list of statically irrelevant lights.
		// TODO: This should be stored per LOD.
		Primitive->IrrelevantLights.Empty();
		for(INT LightIndex = 0;LightIndex < Mesh->RelevantLights.Num();LightIndex++)
		{
			const ULightComponent* Light = Mesh->RelevantLights(LightIndex);

			// Check if the light is stored in the light-map.
			const UBOOL bIsInLightMap = ComponentLODInfo.LightMap && ComponentLODInfo.LightMap->LightGuids.ContainsItem(Light->LightmapGuid);

			// Check if the light is stored in the shadow-map.
			UBOOL bIsInShadowMap = FALSE;
			for(INT ShadowMapIndex = 0;ShadowMapIndex < ComponentLODInfo.ShadowMaps.Num();ShadowMapIndex++)
			{
				if(ComponentLODInfo.ShadowMaps(ShadowMapIndex)->GetLightGuid() == Light->LightGuid)
				{
					bIsInShadowMap = TRUE;
					break;
				}
			}

			// Add the light to the statically irrelevant light list if it is in the potentially relevant light list, but didn't contribute to the light-map or a shadow-map.
			if(!bIsInLightMap && !bIsInShadowMap)
			{	
				Primitive->IrrelevantLights.AddUniqueItem(Light->LightGuid);
			}
		}

		// Mark the primitive's package as dirty.
		Primitive->MarkPackageDirty();
	}

private:

	/** The primitive this mapping represents. */
	UStaticMeshComponent* const Primitive;

	/** The LOD this mapping represents. */
	const INT LODIndex;
};

/** Represents a static mesh primitive with vertex mapped static lighting. */
class FStaticMeshStaticLightingVertexMapping : public FStaticLightingVertexMapping
{
public:

	/** Initialization constructor. */
	FStaticMeshStaticLightingVertexMapping(UStaticMeshComponent* InPrimitive,INT InLODIndex,FStaticLightingMesh* InMesh,UBOOL bPerformFullQualityBuild):
		FStaticLightingVertexMapping(
			InMesh,
			InPrimitive,
			InPrimitive->bForceDirectLightMap,
			1.0f / Square((FLOAT)InPrimitive->SubDivisionStepSize),
			!(bPerformFullQualityBuild && InPrimitive->bUseSubDivisions)
			),
		Primitive(InPrimitive),
		LODIndex(InLODIndex)
	{}

	// FStaticLightingTextureMapping interface
	virtual void Apply(FLightMapData1D* LightMapData,const TMap<ULightComponent*,FShadowMapData1D*>& ShadowMapData)
	{
		// Discard the existing static lighting data for LODs that no longer exist.
		if(Primitive->LODData.Num() > Primitive->StaticMesh->LODModels.Num())
		{
			Primitive->LODData.Remove(
				Primitive->StaticMesh->LODModels.Num(),
				Primitive->LODData.Num() - Primitive->StaticMesh->LODModels.Num()
				);
		}

		// Ensure LODData has enough entries in it.
		if(LODIndex >= Primitive->LODData.Num())
		{
			Primitive->LODData.AddZeroed(LODIndex + 1 - Primitive->LODData.Num());
		}
		FStaticMeshComponentLODInfo& ComponentLODInfo = Primitive->LODData(LODIndex);

		// Create a light-map for the primitive.
		if(LightMapData)
		{
			ComponentLODInfo.LightMap = LightMapData->Lights.Num() ? new FLightMap1D(Primitive,*LightMapData) : NULL;
			delete LightMapData;
		}

		// Create the shadow-maps for the primitive.
		ComponentLODInfo.ShadowVertexBuffers.Empty(ShadowMapData.Num());
		ComponentLODInfo.ShadowMaps.Empty();
		for(TMap<ULightComponent*,FShadowMapData1D*>::TConstIterator ShadowMapDataIt(ShadowMapData);ShadowMapDataIt;++ShadowMapDataIt)
		{
			ComponentLODInfo.ShadowVertexBuffers.AddItem(new(Owner) UShadowMap1D(ShadowMapDataIt.Key()->LightGuid,*ShadowMapDataIt.Value()));
			delete ShadowMapDataIt.Value();
		}

		// Build the list of statically irrelevant lights.
		// TODO: This should be stored per LOD.
		Primitive->IrrelevantLights.Empty();
		for(INT LightIndex = 0;LightIndex < Mesh->RelevantLights.Num();LightIndex++)
		{
			const ULightComponent* Light = Mesh->RelevantLights(LightIndex);

			// Check if the light is stored in the light-map.
			const UBOOL bIsInLightMap = ComponentLODInfo.LightMap && ComponentLODInfo.LightMap->LightGuids.ContainsItem(Light->LightmapGuid);

			// Check if the light is stored in the shadow-map.
			UBOOL bIsInShadowMap = FALSE;
			for(INT ShadowMapIndex = 0;ShadowMapIndex < ComponentLODInfo.ShadowVertexBuffers.Num();ShadowMapIndex++)
			{
				if(ComponentLODInfo.ShadowVertexBuffers(ShadowMapIndex)->GetLightGuid() == Light->LightGuid)
				{
					bIsInShadowMap = TRUE;
					break;
				}
			}

			// Add the light to the statically irrelevant light list if it is in the potentially relevant light list, but didn't contribute to the light-map or a shadow-map.
			if(!bIsInLightMap && !bIsInShadowMap)
			{	
				Primitive->IrrelevantLights.AddUniqueItem(Light->LightGuid);
			}
		}
		
		// Mark the primitive's package as dirty.
		Primitive->MarkPackageDirty();
	}

private:

	/** The primitive this mapping represents. */
	UStaticMeshComponent* const Primitive;

	/** The LOD this mapping represents. */
	const INT LODIndex;
};

void UStaticMeshComponent::GetStaticLightingInfo(FStaticLightingPrimitiveInfo& OutPrimitiveInfo,const TArray<ULightComponent*>& InRelevantLights,const FLightingBuildOptions& Options)
{
	// Determine whether this static mesh has static shadowing while it is still attached to its owner actor.
	const UBOOL bHasStaticShadowing = HasStaticShadowing();

	if( StaticMesh && bHasStaticShadowing )
	{
		// Process each LOD separately.
		TArray<FStaticMeshStaticLightingMesh*> StaticLightingMeshes;
		for(INT LODIndex = 0;LODIndex < StaticMesh->LODModels.Num();LODIndex++)
		{
			const FStaticMeshRenderData& LODRenderData = StaticMesh->LODModels(LODIndex);

			// Figure out whether we are storing the lighting/ shadowing information in a texture or vertex buffer.
			UBOOL	bUseTextureMap;
			INT		LightMapWidth	= 0;
			INT		LightMapHeight	= 0;
			GetLightMapResolution( LightMapWidth, LightMapHeight );

			if( (LightMapWidth > 0) && (LightMapHeight > 0) 
			&&	StaticMesh->LightMapCoordinateIndex >= 0 
			&&	(UINT)StaticMesh->LightMapCoordinateIndex < LODRenderData.VertexBuffer.GetNumTexCoords()
			)
			{
				bUseTextureMap = TRUE;
			}
			else
			{
				bUseTextureMap = FALSE;
			}

			// Create a static lighting mesh for the LOD.
			FStaticMeshStaticLightingMesh* StaticLightingMesh = AllocateStaticLightingMesh(LODIndex,InRelevantLights);
			OutPrimitiveInfo.Meshes.AddItem(StaticLightingMesh);
			StaticLightingMeshes.AddItem(StaticLightingMesh);

			if(bUseTextureMap)
			{
				// Create a static lighting texture mapping for the LOD.
				OutPrimitiveInfo.Mappings.AddItem(new FStaticMeshStaticLightingTextureMapping(this,LODIndex,StaticLightingMesh,LightMapWidth,LightMapHeight,StaticMesh->LightMapCoordinateIndex,Options.bPerformFullQualityBuild));
			}
			else
			{
				// Create a static lighting vertex mapping for the LOD.
				OutPrimitiveInfo.Mappings.AddItem(new FStaticMeshStaticLightingVertexMapping(this,LODIndex,StaticLightingMesh,Options.bPerformFullQualityBuild));
			}
		}

		// Give each LOD's static lighting mesh a list of the other LODs of this primitive, so they can disallow shadow casting between LODs.
		for(INT MeshIndex = 0;MeshIndex < StaticLightingMeshes.Num();MeshIndex++)
		{
			for(INT OtherMeshIndex = 0;OtherMeshIndex < StaticLightingMeshes.Num();OtherMeshIndex++)
			{
				if(MeshIndex != OtherMeshIndex)
				{
					StaticLightingMeshes(MeshIndex)->OtherLODs.AddItem(StaticLightingMeshes(OtherMeshIndex));
				}
			}
		}
	}
}

/** Allocates an implementation of FStaticLightingMesh that will handle static lighting for this component */
FStaticMeshStaticLightingMesh* UStaticMeshComponent::AllocateStaticLightingMesh(INT LODIndex, const TArray<ULightComponent*>& InRelevantLights)
{
	return new FStaticMeshStaticLightingMesh(this,LODIndex,InRelevantLights);
}

void UStaticMeshComponent::InvalidateLightingCache()
{
	UBOOL bHasStaticLightingData = FALSE;
	for(INT i = 0; i < LODData.Num() && !bHasStaticLightingData; i++)
	{
		bHasStaticLightingData = bHasStaticLightingData || LODData(i).ShadowMaps.Num() > 0;
		bHasStaticLightingData = bHasStaticLightingData || LODData(i).ShadowVertexBuffers.Num() > 0;
		bHasStaticLightingData = bHasStaticLightingData || LODData(i).LightMap != NULL;
	}
	// Save the static mesh state for transactions, force it to be marked dirty if we are going to discard any static lighting data.
	Modify(bHasStaticLightingData);

	// Mark lighting as requiring a rebuilt.
	MarkLightingRequiringRebuild();

	// Detach the component from the scene for the duration of this function.
	FComponentReattachContext ReattachContext(this);

	// Discard all cached lighting.
	IrrelevantLights.Empty();
	for(INT i=0;i<LODData.Num();i++)
	{
		LODData(i).ShadowMaps.Empty();
		LODData(i).ShadowVertexBuffers.Empty();
		LODData(i).LightMap = NULL;
	}
}

