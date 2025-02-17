/*=============================================================================
	UnTerrainFoliage.cpp: Terrain foliage code.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#include "EnginePrivate.h"
#include "UnTerrain.h"
#include "UnTerrainRender.h"

void FTerrainFoliageRenderDataPolicy::InitInstances(const UTerrainComponent* Component,const FTerrainWeightedMaterial& WeightedMaterial)
{
	const ATerrain* const Terrain = Component->GetTerrain();
	const FMatrix LocalToWorldInverseTranspose = Component->LocalToWorld.Inverse().Transpose();
	FRandomStream RandomStream(Mesh->Seed ^ Component->SectionBaseX ^ Component->SectionBaseY);

	// Calculate the fraction of the draw radius that meshes start to disappear at.
	const FLOAT MinThinningRadiusFraction = Clamp(Mesh->MinThinningRadius / Mesh->MaxDrawRadius,0.0f,1.0f);

	for(INT QuadY = 0;QuadY < Component->TrueSectionSizeY;QuadY++)
	{
		for(INT QuadX = 0;QuadX < Component->TrueSectionSizeX;QuadX++)
		{
			const INT GlobalQuadX = Component->SectionBaseX + QuadX;
			const INT GlobalQuadY = Component->SectionBaseY + QuadY;

			if (Terrain->IsTerrainQuadVisible(GlobalQuadX,GlobalQuadY) == FALSE)
			{
				// Don't populate foliage on holes...
				continue;
			}

			const FTerrainPatch Patch = Terrain->GetPatch(GlobalQuadX,GlobalQuadY);

			// Compute the vertices and material weights for this terrain quad.
			FVector QuadVertices[2][2];
			FVector QuadNormals[2][2];
			FLOAT Weights[2][2];
			for(INT SubY = 0;SubY < 2;SubY++)
			{
				for(INT SubX = 0;SubX < 2;SubX++)
				{
					static FPatchSampler PatchSampler(TERRAIN_MAXTESSELATION);

					const FLOAT SampleDerivX = PatchSampler.SampleDerivX(Patch,0,0);
					const FLOAT SampleDerivY = PatchSampler.SampleDerivY(Patch,0,0);
					const FVector LocalTangentX = FVector(1,0,SampleDerivX * TERRAIN_ZSCALE).SafeNormal();
					const FVector LocalTangentY = FVector(0,1,SampleDerivY * TERRAIN_ZSCALE).SafeNormal();
					const FVector LocalTangentZ = (LocalTangentX ^ LocalTangentY).SafeNormal();
					const FLOAT Displacement = Terrain->GetCachedDisplacement(GlobalQuadX + SubX,GlobalQuadY + SubY,0,0);

					QuadVertices[SubX][SubY] = FVector(
						GlobalQuadX + SubX,
						GlobalQuadY + SubY,
						(-32768.0f + (FLOAT)(Terrain->Height(GlobalQuadX + SubX, GlobalQuadY + SubY))) * TERRAIN_ZSCALE
						) + LocalTangentZ * Displacement;
					QuadNormals[SubX][SubY] = LocalTangentZ;
					Weights[SubX][SubY] = WeightedMaterial.Weight(GlobalQuadX + SubX,GlobalQuadY + SubY) / 255.0f;
				}
			}

			for(INT InstanceIndex = 0;InstanceIndex < Mesh->Density;InstanceIndex++)
			{
				const FLOAT X = RandomStream.GetFraction();
				const FLOAT Y = RandomStream.GetFraction();
				const FLOAT UniformScale = RandomStream.GetFraction();
				const FLOAT ScaleX = RandomStream.GetFraction();
				const FLOAT ScaleY = RandomStream.GetFraction();
				const FLOAT ScaleZ = RandomStream.GetFraction();
				const FLOAT Yaw = RandomStream.GetFraction();
				const FLOAT Random = RandomStream.GetFraction();

				const FLOAT MaterialAlpha = QuadLerp(Weights[0][0],Weights[1][0],Weights[0][1],Weights[1][1],X,Y);
				if(Random * (1.0f - Mesh->AlphaMapThreshold) <= MaterialAlpha - Mesh->AlphaMapThreshold)
				{
					FTerrainFoliageInstance FoliageInstance;

					// Calculate the instance's location in local space.
					const FVector LocalLocation = QuadLerp(QuadVertices[0][0],QuadVertices[1][0],QuadVertices[0][1],QuadVertices[1][1],X,Y) -
													FVector(Component->SectionBaseX,Component->SectionBaseY,0);
					FoliageInstance.Location = Component->LocalToWorld.TransformFVector(LocalLocation);

					// Calculate the surface normal at the instance's location.
					const FVector LocalNormal = QuadLerp(QuadNormals[0][0],QuadNormals[1][0],QuadNormals[0][1],QuadNormals[1][1],X,Y);

					// Determine the Z-axis of the foliage instance in local space.
					const FVector LocalZAxis = Lerp(FVector(0,0,1),LocalNormal,Mesh->SlopeRotationBlend);

					// Transform the Z-axis into world-space.
					const FVector WorldZAxis = LocalToWorldInverseTranspose.TransformNormal(LocalZAxis).SafeNormal();

					// Build an orthonormal basis with +Z being the surface normal.
					FVector WorldXAxis;
					FVector WorldYAxis;
					WorldZAxis.FindBestAxisVectors(WorldXAxis,WorldYAxis);

					// Calculate the instance's static lighting texture coordinate.
					FoliageInstance.StaticLightingTexCoord = FVector2D(
						((QuadX + X) * Terrain->StaticLightingResolution + 0.5f) / (Component->TrueSectionSizeX * Terrain->StaticLightingResolution + 1),
						((QuadY + Y) * Terrain->StaticLightingResolution + 0.5f) / (Component->TrueSectionSizeY * Terrain->StaticLightingResolution + 1)
						);

					// Calculate the transform from instance local space to world space.
					const FRotator Rotation(0,appTrunc(Yaw * 65535),0);
					const FMatrix NormalInstanceToWorld =
						FRotationMatrix(Rotation) *
						FMatrix(-WorldXAxis,WorldYAxis,WorldZAxis,FVector(0,0,0));
					const FMatrix InstanceToWorld =
						FScaleMatrix(Lerp(
								FVector(1,1,1) * Mesh->MinScale * Mesh->MinUniformScale,
								FVector(1,1,1) * Mesh->MaxScale * Mesh->MaxUniformScale,
								FVector(ScaleX,ScaleY,ScaleZ) * UniformScale
								)) *
						NormalInstanceToWorld *
						FTranslationMatrix(FoliageInstance.Location);

					// Set the instance's transform.
					FoliageInstance.XAxis = InstanceToWorld.GetAxis(0);
					FoliageInstance.YAxis = InstanceToWorld.GetAxis(1);
					FoliageInstance.ZAxis = InstanceToWorld.GetAxis(2);

					// Set the instance's world-space bounding radius.
					FSphere LocalBoundingSphere = Mesh->StaticMesh->Bounds.GetSphere();
					FoliageInstance.BoundingRadius = LocalBoundingSphere.TransformBy(InstanceToWorld).W;

					// Choose a random factor that is applied to the distance-based rendering of the instance.
					const FLOAT ScaleFraction = UniformScale / Mesh->MaxUniformScale;
					FoliageInstance.DistanceFactorSquared = 1.0f / Square(Lerp(MinThinningRadiusFraction,1.0f,appFrand() * ScaleFraction));

					// Add the instance to the instance octree.
					InstanceOctree.AddElement(FoliageInstance);

					// Count the instance in the total number of instances.
					NumInstances++;
				}
			}
		}
	}
}

void FTerrainFoliageRenderDataPolicy::GetVisibleFoliageInstances(const FSceneView* View,TArray<const FTerrainFoliageInstance*>& OutVisibleFoliageInstances)
{
	OutVisibleFoliageInstances.Empty(NumInstances);
	
	if (GSystemSettings.FoliageDrawRadiusMultiplier <= 0.0f)
	{
		return;
	}

	FLOAT AdjustedMaxDrawRadius = Mesh->MaxDrawRadius * GSystemSettings.FoliageDrawRadiusMultiplier;
	const FLOAT DrawRadiusSquared = Square(AdjustedMaxDrawRadius);

	// Find the foliage instances in the octree that intersect the view frustum and are within their draw radius from the view origin.
	for(FTerrainFoliageInstanceOctree::TConstIterator<> OctreeIt(InstanceOctree);OctreeIt.HasPendingNodes();OctreeIt.Advance())
	{
		const FTerrainFoliageInstanceOctree::FNode& CurrentNode = OctreeIt.GetCurrentNode();
		const FOctreeNodeContext& CurrentContext = OctreeIt.GetCurrentContext();

		// Find the octree node's children which intersect the view frustum and are within the foliage mesh's draw radius from the view origin.
		FOREACH_OCTREE_CHILD_NODE(ChildRef)
		{
			if(CurrentNode.HasChild(ChildRef))
			{
				const FOctreeNodeContext& ChildContext = OctreeIt.GetCurrentContext().GetChildContext(ChildRef);

				// Calculate the minimum squared distance to the origin of a child of this octree node which is outside the foliage mesh's draw radius.
				const FLOAT ChildCullDistanceSquared = Square(FVector(ChildContext.Bounds.Extent).Size() + AdjustedMaxDrawRadius);

				// Check that the child node is within the foliage mesh's draw radius from the view origin.
				const FLOAT ChildDistanceSquared = (FVector(ChildContext.Bounds.Center) - View->ViewOrigin).SizeSquared();
				if(ChildDistanceSquared < ChildCullDistanceSquared)
				{
					// Check that the child node intersects the view frustum.
					if(View->ViewFrustum.IntersectBox(ChildContext.Bounds.Center,ChildContext.Bounds.Extent))
					{
						OctreeIt.PushChild(ChildRef);
					}
				}
			}
		}

		// Iterate over the octree node's foliage instances.
		for(FTerrainFoliageInstanceOctree::ElementConstIt InstanceIt(CurrentNode.GetElementIt());InstanceIt;++InstanceIt)
		{
			const FTerrainFoliageInstance& Instance = *InstanceIt;

			// Check that the instance is inside the view frustum, and within its draw radius from the view origin.
			const FLOAT InstanceDistanceSquared = (Instance.Location - View->ViewOrigin).SizeSquared() * Instance.DistanceFactorSquared;
			if(	InstanceDistanceSquared < DrawRadiusSquared &&
				View->ViewFrustum.IntersectSphere(Instance.Location,Instance.BoundingRadius))
			{
				OutVisibleFoliageInstances.AddItem(&Instance);
			}
		}
	}

	// Reset the foliage mesh's render data staleness timer.
	TimeSinceLastRendered = 0.0f;
}

void FTerrainFoliageRenderDataPolicy::GetInstance(InstanceReferenceType InstanceReference,const FSceneView* View,FTerrainFoliageInstance& OutInstance,FLOAT& OutWindScale) const
{
	const InstanceType& Instance = *InstanceReference;

	// Calculate the transition scale for this instance.
	const FLOAT DistanceSquared = ((FVector)View->ViewOrigin - Instance.Location).SizeSquared() * Instance.DistanceFactorSquared;
	const FLOAT TransitionScale =
		DistanceSquared < MinTransitionRadiusSquared ?
			1.0f :
			(1.0f - (appSqrt(DistanceSquared) - Mesh->MinTransitionRadius) * InvTransitionSize);

	OutInstance.Location = Instance.Location + View->PreViewTranslation;
	OutInstance.XAxis = Instance.XAxis * TransitionScale;
	OutInstance.YAxis = Instance.YAxis * TransitionScale;
	OutInstance.ZAxis = Instance.ZAxis * TransitionScale;
	OutInstance.StaticLightingTexCoord = Instance.StaticLightingTexCoord;

	OutWindScale = Mesh->SwayScale * TransitionScale;
}

void FTerrainFoliageRenderDataPolicy::InitVertexFactoryStaticLighting(FVertexBuffer* InstanceBuffer,FFoliageVertexFactory::DataType& OutData)
{
	OutData.ShadowMapCoordinateComponent = FVertexStreamComponent(
		InstanceBuffer,
		STRUCT_OFFSET(FTerrainFoliageInstance,StaticLightingTexCoord),
		sizeof(FTerrainFoliageInstance),
		VET_Float2,
		TRUE
		);
}

void FTerrainComponentSceneProxy::DrawFoliage(FPrimitiveDrawInterface* PDI,const FSceneView* View, UINT DPGIndex)
{
	// Note: We don't need to check the view relevance for the DPGIndex, as the terrain has already done that.
	if (GSystemSettings.FoliageDrawRadiusMultiplier <= 0.0f)
	{
		return;
	}

	ATerrain* const Terrain = ComponentOwner->GetTerrain();
	const FVector& ViewOrigin = View->ViewOrigin;
	const FBoxSphereBounds& Bounds = PrimitiveSceneInfo->Bounds;

	SCOPE_CYCLE_COUNTER(STAT_FoliageRenderTime);

	// Only draw foliage in perspective viewports, not for hit testing, and on hardware that supports vertex instancing.
	const UBOOL bIsPerspectiveViewport = View->ProjectionMatrix.M[3][3] < 1.0f;
	if(	!PDI->IsHitTesting() && 
		bIsPerspectiveViewport && 
		(GSupportsVertexInstancing || GSupportsEmulatedVertexInstancing))
	{
		const FLOAT Distance = (Bounds.Origin - ViewOrigin).Size() - Bounds.SphereRadius;

		// Build batches of foliage mesh instances which are within their MaxDrawRadius.
		for(INT MaterialIndex = 0;MaterialIndex < Terrain->WeightedMaterials.Num();MaterialIndex++)
		{
			UTerrainMaterial*	TerrainMaterial = Terrain->WeightedMaterials(MaterialIndex).Material;
			for(INT MeshIndex = 0;MeshIndex < TerrainMaterial->FoliageMeshes.Num();MeshIndex++)
			{
				const FTerrainFoliageMesh* Mesh = &TerrainMaterial->FoliageMeshes(MeshIndex);

				// Verify that the foliage mesh is valid before using it.
				const UBOOL bFoliageMeshIsValid = 
					Mesh->StaticMesh &&
					Mesh->StaticMesh->LODModels(0).NumVertices > 0 && 
					Mesh->StaticMesh->LODModels(0).IndexBuffer.Indices.Num() > 0;

				if(bFoliageMeshIsValid && Distance < (Mesh->MaxDrawRadius * GSystemSettings.FoliageDrawRadiusMultiplier))
				{
					// Determine the material to use for the foliage mesh.
					const UMaterialInterface* Material = FoliageMeshToMaterialMap.FindRef(Mesh);
					check(Material);

					// Check that the material isn't ignored in this context before doing any of the work to render it.
					const FMaterialRenderProxy* MaterialRenderProxy = Material->GetRenderProxy(bSelected);
					if(!PDI->IsMaterialIgnored(MaterialRenderProxy))
					{
						TFoliageRenderData<FTerrainFoliageRenderDataPolicy>* RenderData = FoliageRenderDataSet.FindRef(Mesh);
						if (!RenderData)
						{
							RenderData = FoliageRenderDataSet.Set(Mesh,new TFoliageRenderData<FTerrainFoliageRenderDataPolicy>(FTerrainFoliageRenderDataPolicy(Mesh,Bounds.GetBox())));
							RenderData->InitInstances(ComponentOwner,Terrain->WeightedMaterials(MaterialIndex));
						}

						// Find the foliage instances visible in this view.
						TArray<const FTerrainFoliageInstance*> VisibleFoliageInstances;
						RenderData->GetVisibleFoliageInstances(View,VisibleFoliageInstances);
						INC_DWORD_STAT_BY(STAT_TerrainFoliageInstances,VisibleFoliageInstances.Num());

						// Draw the visible foliage instances.
						if(VisibleFoliageInstances.Num() > 1)
						{
							RenderData->UpdateInstances(VisibleFoliageInstances,View);

							FMeshElement MeshElement;
							MeshElement.IndexBuffer = &Mesh->StaticMesh->LODModels(0).IndexBuffer;
							MeshElement.VertexFactory = RenderData->GetVertexFactory();
							MeshElement.MaterialRenderProxy = MaterialRenderProxy;
							MeshElement.LCI = CurrentMaterialInfo->ComponentLightInfo;
							MeshElement.LocalToWorld = FMatrix::Identity;
							MeshElement.WorldToLocal = FMatrix::Identity;
							MeshElement.FirstIndex = 0;
							MeshElement.NumPrimitives = Mesh->StaticMesh->LODModels(0).IndexBuffer.Indices.Num() / 3;
							MeshElement.MinVertexIndex = 0;
							MeshElement.MaxVertexIndex = Mesh->StaticMesh->LODModels(0).NumVertices - 1;
							MeshElement.Type = PT_TriangleList;
							MeshElement.DepthPriorityGroup = DPGIndex;
							DrawRichMesh(PDI,MeshElement,FLinearColor(0,1,0),LinearLevelColor,LinearPropertyColor,PrimitiveSceneInfo,bSelected);
						}
					}
				}
			}
		}
	}
}

void FTerrainComponentSceneProxy::TickFoliage(FLOAT DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_FoliageTickTime);
	for(TMap<const FTerrainFoliageMesh*,TFoliageRenderData<FTerrainFoliageRenderDataPolicy>*>::TIterator It(FoliageRenderDataSet);It;++It)
	{
		if(It.Value()->TimeSinceLastRendered >= 10.0f)
		{
			// Free foliage data if it hasn't been rendered recently.
			delete It.Value();
			It.RemoveCurrent();
		}
		else
		{
			// Update the time since the component was last rendered.
			It.Value()->TimeSinceLastRendered += DeltaTime;
		}
	}
}

void FTerrainComponentSceneProxy::ReleaseFoliageRenderData()
{
	// Delete all foliage render data.
	for(TMap<const FTerrainFoliageMesh*,TFoliageRenderData<FTerrainFoliageRenderDataPolicy>*>::TConstIterator It(FoliageRenderDataSet);It;++It)
	{
		delete It.Value();
	}
	FoliageRenderDataSet.Empty();
}
