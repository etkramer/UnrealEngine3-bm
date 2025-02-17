/*=============================================================================
	UnStaticMeshRender.cpp: Static mesh rendering code.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EnginePhysicsClasses.h"
#include "EngineDecalClasses.h"
#include "LevelUtils.h"

// Needed for FStaticMeshSceneProxy::DrawShadowVolumes().
#include "ScenePrivate.h"

#if XBOX
// Contains the optimized plane dots function
#include "UnStaticMeshRenderXe.h"
#endif

FDecalRenderData* UStaticMeshComponent::GenerateDecalRenderData(FDecalState* Decal) const
{
	SCOPE_CYCLE_COUNTER(STAT_DecalStaticMeshAttachTime);

	// Do nothing if the specified decal doesn't project on static meshes.
	if ( !Decal->bProjectOnStaticMeshes )
	{
		return NULL;
	}

	// Perform a kDOP query to retrieve intersecting leavesl.
	const FStaticMeshCollisionDataProvider MeshData( this );
	TArray<WORD> Leaves;
	TkDOPFrustumQuery<FStaticMeshCollisionDataProvider,WORD> kDOPQuery( static_cast<FPlane*>( Decal->Planes.GetData() ),
																	Decal->Planes.Num(),
																	Leaves,
																	MeshData );
	const UStaticMesh::kDOPTreeType& kDOPTree = StaticMesh->kDOPTree;
	const UBOOL bHit = kDOPTree.FrustumQuery( kDOPQuery );
	// Early out if there is no overlap.
	if ( !bHit )
	{
		return NULL;
	}

	FDecalRenderData* DecalRenderData = NULL;
	
	// Transform decal properties into local space.
	const FDecalLocalSpaceInfoClip DecalInfo( Decal, LocalToWorld, LocalToWorld.Inverse() );
	const FStaticMeshRenderData& StaticMeshRenderData = StaticMesh->LODModels(0);

	// Is the decal lit?
	const UBOOL bLitDecal = !Decal->bDecalMaterialHasUnlitLightingModel;

	// Static mesh light map info.
	INT LightMapWidth	= 0;
	INT LightMapHeight	= 0;
	if ( bLitDecal )
	{
		GetLightMapResolution( LightMapWidth, LightMapHeight );
	}

	// Should the decal use texture lightmapping?  FALSE if the decal is unlit.
	const UBOOL bHasLightMap =
		bLitDecal
		&& LODData.Num() > 0
		&& LODData(0).LightMap;

	// Should the decal use texture lightmapping?  FALSE if the decal has no lightmap.
	const UBOOL bUsingTextureLightmapping =
		bHasLightMap
		&& (LightMapWidth > 0)
		&& (LightMapHeight > 0) 
		&& (StaticMesh->LightMapCoordinateIndex >= 0) 
		&& ((UINT)StaticMesh->LightMapCoordinateIndex < StaticMeshRenderData.VertexBuffer.GetNumTexCoords());

	// Should the decal use vertex lightmapping?  FALSE if the decal has no lightmap.
	const UBOOL bUsingVertexLightmapping =
		bHasLightMap
		&& !bUsingTextureLightmapping;

	// Should the decal use software clipping?  FALSE if not wanted by the decal or if vertex lightmapping is used.
	// Vertex lightmapped decals MUST NOT use software clipping so that the vertex lightmap index remapping works.
	const UBOOL bUseSoftwareClipping = 
		!bUsingVertexLightmapping
		&& !Decal->bNoClip &&
		Decal->bStaticDecal &&
		!Decal->bMovableDecal;

	// We modify the incoming decal state to set whether or not we want to actually use software clipping
	Decal->bUseSoftwareClip = bUseSoftwareClipping;
	

	// Only use index buffer if we're clipping or need to remap indices.
	const UBOOL bUseIndexBuffer = bUseSoftwareClipping || bUsingVertexLightmapping || !Decal->bStaticDecal || Decal->bMovableDecal;

	// vertex lightmapping data.
	Decal->SampleRemapping.Empty();
	FLightMapData1D* LightMapData1D = NULL;

	if( !Decal->bStaticDecal || Decal->bMovableDecal )
	{
		// Allocate a FDecalRenderData object.  Use vertex factory from receiver static mesh
		DecalRenderData = new FDecalRenderData( NULL, FALSE, TRUE, &StaticMeshRenderData.VertexFactory );

		TArray<WORD>& VertexIndices = DecalRenderData->IndexBuffer.Indices;
		for( INT LeafIndex = 0 ; LeafIndex < Leaves.Num() ; ++LeafIndex )
		{
			const UStaticMesh::kDOPTreeType::NodeType& Node = kDOPQuery.Nodes(Leaves(LeafIndex));
			const WORD FirstTriangle						= Node.t.StartIndex;
			const WORD LastTriangle							= FirstTriangle + Node.t.NumTriangles;

			for( WORD TriangleIndex=FirstTriangle; TriangleIndex<LastTriangle; ++TriangleIndex )
			{
				const FkDOPCollisionTriangle<WORD>& Triangle = kDOPTree.Triangles(TriangleIndex);

				// Calculate face direction, used for backface culling.
				const FVector& V1 = StaticMeshRenderData.PositionVertexBuffer.VertexPosition(Triangle.v1);
				const FVector& V2 = StaticMeshRenderData.PositionVertexBuffer.VertexPosition(Triangle.v2);
				const FVector& V3 = StaticMeshRenderData.PositionVertexBuffer.VertexPosition(Triangle.v3);
				FVector FaceNormal = (V2 - V1 ^ V3 - V1);

				// Normalize direction, if not possible skip triangle as it's zero surface.
				if( FaceNormal.Normalize(KINDA_SMALL_NUMBER) )
				{
					// Calculate angle between look vector and decal face normal.
					const FLOAT Dot = DecalInfo.LocalLookVector | FaceNormal;
					// Determine whether decal is front facing.
					const UBOOL bIsFrontFacing = Decal->bFlipBackfaceDirection ? -Dot > Decal->DecalComponent->BackfaceAngle : Dot > Decal->DecalComponent->BackfaceAngle;

					// Even if backface culling is disabled, reject triangles that view the decal at grazing angles.
					if( bIsFrontFacing || ( Decal->bProjectOnBackfaces && Abs( Dot ) > Decal->DecalComponent->BackfaceAngle ) )
					{
						VertexIndices.AddItem( Triangle.v1 );
						VertexIndices.AddItem( Triangle.v2 );
						VertexIndices.AddItem( Triangle.v3 );
					}
				}
			}
		}
		// Set triangle count.
		DecalRenderData->NumTriangles = VertexIndices.Num()/3;
		// set the blending interval
		DecalRenderData->DecalBlendRange = Decal->DecalComponent->CalcDecalDotProductBlendRange();
	}
	else 
	{
		TArray<WORD> VertexIndices;
		VertexIndices.Empty(2000);
		for( INT LeafIndex = 0 ; LeafIndex < Leaves.Num() ; ++LeafIndex )
		{
			const UStaticMesh::kDOPTreeType::NodeType& Node = kDOPQuery.Nodes(Leaves(LeafIndex));
			const WORD FirstTriangle						= Node.t.StartIndex;
			const WORD LastTriangle							= FirstTriangle + Node.t.NumTriangles;

			for( WORD TriangleIndex=FirstTriangle; TriangleIndex<LastTriangle; ++TriangleIndex )
			{
				const FkDOPCollisionTriangle<WORD>& Triangle = kDOPTree.Triangles(TriangleIndex);

				// Calculate face direction, used for backface culling.
				const FVector& V1 = StaticMeshRenderData.PositionVertexBuffer.VertexPosition(Triangle.v1);
				const FVector& V2 = StaticMeshRenderData.PositionVertexBuffer.VertexPosition(Triangle.v2);
				const FVector& V3 = StaticMeshRenderData.PositionVertexBuffer.VertexPosition(Triangle.v3);
				FVector FaceNormal = (V2 - V1 ^ V3 - V1);

				// Normalize direction, if not possible skip triangle as it's zero surface.
				if( FaceNormal.Normalize(KINDA_SMALL_NUMBER) )
				{
					// Calculate angle between look vector and decal face normal.
					const FLOAT Dot = DecalInfo.LocalLookVector | FaceNormal;
					// Determine whether decal is front facing.
					const UBOOL bIsFrontFacing = Decal->bFlipBackfaceDirection ? -Dot > Decal->DecalComponent->BackfaceAngle : Dot > Decal->DecalComponent->BackfaceAngle;

					// Even if backface culling is disabled, reject triangles that view the decal at grazing angles.
					if( bIsFrontFacing || ( Decal->bProjectOnBackfaces && Abs( Dot ) > Decal->DecalComponent->BackfaceAngle ) )
					{
						VertexIndices.AddItem( Triangle.v1 );
						VertexIndices.AddItem( Triangle.v2 );
						VertexIndices.AddItem( Triangle.v3 );
					}
				}
			}
		}

		if( VertexIndices.Num() > 0 )
		{
			// Allocate a FDecalRenderData object.
			DecalRenderData = new FDecalRenderData( NULL, TRUE, bUseIndexBuffer );

			// Create temporary structures.
			FDecalPoly Poly;
			FVector2D TempTexCoords;

			// Presize buffers.
			DecalRenderData->Vertices.Empty(VertexIndices.Num());
			if( bUseIndexBuffer )
			{
				DecalRenderData->IndexBuffer.Indices.Empty(VertexIndices.Num());
			}

			// Iterate over all collected triangle indices and process them.
			for( INT VertexIndexIndex=0; VertexIndexIndex<VertexIndices.Num(); VertexIndexIndex+=3 )
			{
				INT V1 = VertexIndices(VertexIndexIndex+0);
				INT V2 = VertexIndices(VertexIndexIndex+1);
				INT V3 = VertexIndices(VertexIndexIndex+2);

				// Set up FDecalPoly used for clipping.
				Poly.Init();
				new(Poly.Vertices) FVector(StaticMeshRenderData.PositionVertexBuffer.VertexPosition(V1));
				new(Poly.Vertices) FVector(StaticMeshRenderData.PositionVertexBuffer.VertexPosition(V2));
				new(Poly.Vertices) FVector(StaticMeshRenderData.PositionVertexBuffer.VertexPosition(V3));
				Poly.Indices.AddItem(V1);
				Poly.Indices.AddItem(V2);
				Poly.Indices.AddItem(V3);

				if ( bUsingTextureLightmapping )
				{
					new(Poly.ShadowTexCoords) FVector2D(StaticMeshRenderData.VertexBuffer.GetVertexUV(V1,StaticMesh->LightMapCoordinateIndex));
					new(Poly.ShadowTexCoords) FVector2D(StaticMeshRenderData.VertexBuffer.GetVertexUV(V2,StaticMesh->LightMapCoordinateIndex));
					new(Poly.ShadowTexCoords) FVector2D(StaticMeshRenderData.VertexBuffer.GetVertexUV(V3,StaticMesh->LightMapCoordinateIndex));
				}
				else
				{
					new(Poly.ShadowTexCoords) FVector2D(0.f, 0.f);
					new(Poly.ShadowTexCoords) FVector2D(0.f, 0.f);
					new(Poly.ShadowTexCoords) FVector2D(0.f, 0.f);
				}

				const UBOOL bClipPassed = bUseSoftwareClipping ? Poly.ClipAgainstConvex( DecalInfo.Convex ) : TRUE;
				if( bClipPassed )
				{
					if ( bUsingVertexLightmapping )
					{
						Decal->SampleRemapping.AddItem(V1);
						Decal->SampleRemapping.AddItem(V2);
						Decal->SampleRemapping.AddItem(V3);
					}

					const INT FirstVertexIndex = DecalRenderData->GetNumVertices(); 
					for ( INT i = 0 ; i < Poly.Vertices.Num() ; ++i )
					{
						// Create decal vertex tangent basis by projecting decal tangents onto the plane defined by the vertex plane.
						const INT VertexIndex = Poly.Indices(i);
						const FPackedNormal& DecalPackedTangentX = StaticMeshRenderData.VertexBuffer.VertexTangentX(VertexIndex);
						const FPackedNormal& DecalPackedTangentZ = StaticMeshRenderData.VertexBuffer.VertexTangentZ(VertexIndex);					

						// Store the decal vertex.
						new(DecalRenderData->Vertices) FDecalVertex(Poly.Vertices( i ),
							DecalPackedTangentX,
							DecalPackedTangentZ,
							Poly.ShadowTexCoords( i ));
					}

					if( bUseIndexBuffer )
					{
						// Triangulate the polygon and add indices to the index buffer
						const INT FirstIndex = DecalRenderData->GetNumIndices();
						for ( INT i = 0 ; i < Poly.Vertices.Num() - 2 ; ++i )
						{
							DecalRenderData->AddIndex( FirstVertexIndex+0 );
							DecalRenderData->AddIndex( FirstVertexIndex+i+1 );
							DecalRenderData->AddIndex( FirstVertexIndex+i+2 );
						}
					}
				}
			}
			// Set triangle count.
			DecalRenderData->NumTriangles = bUseIndexBuffer ? DecalRenderData->GetNumIndices()/3 : VertexIndices.Num()/3;
			// set the blending interval
			DecalRenderData->DecalBlendRange = Decal->DecalComponent->CalcDecalDotProductBlendRange();
		}
	}

	return DecalRenderData;
}

IMPLEMENT_COMPARE_CONSTPOINTER( FDecalInteraction, UnStaticMeshRender,
{
	return (A->DecalState.SortOrder <= B->DecalState.SortOrder) ? -1 : 1;
} );

/** Creates a light cache for the decal if it has a lit material. */
void FStaticMeshSceneProxy::CreateDecalLightCache(const FDecalInteraction& DecalInteraction)
{
	if ( DecalInteraction.DecalState.MaterialViewRelevance.bLit )
	{
		new(DecalLightCaches) FDecalLightCache( DecalInteraction, *this );
	}
}

/** Initialization constructor. */
FStaticMeshSceneProxy::FStaticMeshSceneProxy(const UStaticMeshComponent* Component):
	FPrimitiveSceneProxy(Component, Component->StaticMesh->GetFName()),
	Owner(Component->GetOwner()),
	StaticMesh(Component->StaticMesh),
	StaticMeshComponent(Component),
	ForcedLodModel(Component->ForcedLodModel),
	LevelColor(1,1,1),
	PropertyColor(1,1,1),
	bCastShadow(Component->CastShadow),
	bSelected(Component->IsOwnerSelected()),
	bShouldCollide(Component->ShouldCollide()),
	bBlockZeroExtent(Component->BlockZeroExtent),
	bBlockNonZeroExtent(Component->BlockNonZeroExtent),
	bBlockRigidBody(Component->BlockRigidBody),
	bForceStaticDecal(Component->bForceStaticDecals),
	MaterialViewRelevance(Component->GetMaterialViewRelevance()),
	WireframeColor(Component->WireframeColor)
{
	// Build the proxy's LOD data.
	LODs.Empty(StaticMesh->LODModels.Num());
	for(INT LODIndex = 0;LODIndex < StaticMesh->LODModels.Num();LODIndex++)
	{
		new(LODs) FLODInfo(Component,LODIndex);
	}

	// If the static mesh can accept decals, copy off statically irrelevant lights and light map guids.
	if( Component->bAcceptsStaticDecals ||
		Component->bAcceptsDynamicDecals )
	{
		for( INT DecalIndex = 0 ; DecalIndex < Decals.Num() ; ++DecalIndex )
		{
			// Create light cache information for any decals that were attached in the FPrimitiveSceneProxy ctor.
			// This needs to be executed on the rendering thread since the rendering thread may already be executing CreateDecalLightCache()
			ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
				CreateDecalLightCacheCommand,
				FStaticMeshSceneProxy*,StaticMeshSceneProxy,this,
				FDecalInteraction,DecalInteraction,*Decals(DecalIndex),
			{
				StaticMeshSceneProxy->CreateDecalLightCache(DecalInteraction);
			});

			// Transform the decal frustum verts into local space.  These will be transformed
			// each frame into world space for the the scissor test.
			Decals(DecalIndex)->DecalState.TransformFrustumVerts( Component->LocalToWorld.Inverse() );
		}
	}

#if !FINAL_RELEASE
	if( GIsEditor )
	{
		// Try to find a color for level coloration.
		if ( Owner )
		{
			ULevel* Level = Owner->GetLevel();
			ULevelStreaming* LevelStreaming = FLevelUtils::FindStreamingLevel( Level );
			if ( LevelStreaming )
			{
				LevelColor = LevelStreaming->DrawColor;
			}
		}

		// Get a color for property coloration.
		FColor TempPropertyColor;
		GEngine->GetPropertyColorationColor( (UObject*)Component, TempPropertyColor );
		PropertyColor = TempPropertyColor;
	}
#endif

	LastLOD = -1;
}

/** Sets up a FMeshElement for a specific LOD and element. */
UBOOL FStaticMeshSceneProxy::GetMeshElement(INT LODIndex,INT ElementIndex,INT FragmentIndex,BYTE InDepthPriorityGroup,const FMatrix& WorldToLocal,FMeshElement& OutMeshElement) const
{
	const FStaticMeshRenderData& LODModel = StaticMesh->LODModels(LODIndex);
	const FStaticMeshElement& Element = LODModel.Elements(ElementIndex);

	SetIndexSource(LODIndex, ElementIndex, FragmentIndex, OutMeshElement, FALSE);
	if(OutMeshElement.NumPrimitives > 0)
	{
		OutMeshElement.VertexFactory = &LODModel.VertexFactory;
		OutMeshElement.DynamicVertexData = NULL;
		OutMeshElement.MaterialRenderProxy = LODs(LODIndex).Elements(ElementIndex).Material->GetRenderProxy(bSelected);
		OutMeshElement.LCI = &LODs(LODIndex);
		OutMeshElement.LocalToWorld = LocalToWorld;
		OutMeshElement.WorldToLocal = WorldToLocal;
		OutMeshElement.MinVertexIndex = Element.MinVertexIndex;
		OutMeshElement.MaxVertexIndex = Element.MaxVertexIndex;
		OutMeshElement.UseDynamicData = FALSE;
		OutMeshElement.ReverseCulling = LocalToWorldDeterminant < 0.0f ? TRUE : FALSE;
		OutMeshElement.CastShadow = bCastShadow && Element.bEnableShadowCasting;
		OutMeshElement.Type = PT_TriangleList;
		OutMeshElement.DepthPriorityGroup = (ESceneDepthPriorityGroup)InDepthPriorityGroup;
		OutMeshElement.bUsePreVertexShaderCulling = TRUE;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**
 * Sets IndexBuffer, FirstIndex and NumPrimitives of OutMeshElement.
 */
void FStaticMeshSceneProxy::SetIndexSource(INT LODIndex, INT ElementIndex, INT FragmentIndex, FMeshElement& OutMeshElement, UBOOL bWireframe) const
{
	const FStaticMeshRenderData& LODModel = StaticMesh->LODModels(LODIndex);
	if (bWireframe)
	{
		if( LODModel.WireframeIndexBuffer.IsInitialized() )
		{
			OutMeshElement.Type = PT_LineList;
			OutMeshElement.FirstIndex = 0;
			OutMeshElement.IndexBuffer = &LODModel.WireframeIndexBuffer;
			OutMeshElement.NumPrimitives = LODModel.WireframeIndexBuffer.Indices.Num() / 2;
		}
		else
		{
			OutMeshElement.Type = PT_TriangleList;
			OutMeshElement.FirstIndex = 0;
			OutMeshElement.IndexBuffer = &LODModel.IndexBuffer;
			OutMeshElement.NumPrimitives = LODModel.IndexBuffer.Indices.Num() / 3;
			OutMeshElement.bWireframe = TRUE;
		}
	}
	else
	{
		const FStaticMeshElement& Element = LODModel.Elements(ElementIndex);
		OutMeshElement.IndexBuffer = &LODModel.IndexBuffer;
		OutMeshElement.FirstIndex = Element.FirstIndex;
		OutMeshElement.NumPrimitives = Element.NumTriangles;
	}
}

// FPrimitiveSceneProxy interface.
void FStaticMeshSceneProxy::DrawStaticElements(FStaticPrimitiveDrawInterface* PDI)
{
	if(!HasViewDependentDPG() && !IsMovable())
	{
		// Determine the DPG the primitive should be drawn in.
		BYTE PrimitiveDPG = GetStaticDepthPriorityGroup();
		INT NumLODs = StaticMesh->LODModels.Num();

		//check if a LOD is being forced
		if (ForcedLodModel > 0) 
		{
			INT LODIndex = ::Clamp(ForcedLodModel, 1, NumLODs) - 1;
			const FStaticMeshRenderData& LODModel = StaticMesh->LODModels(LODIndex);
			// Draw the static mesh elements.
			const FMatrix WorldToLocal = LocalToWorld.Inverse();
			for(INT ElementIndex = 0;ElementIndex < LODModel.Elements.Num();ElementIndex++)
			{
				for(INT FragmentIndex = 0;FragmentIndex < LODs(LODIndex).Elements(ElementIndex).NumFragments;FragmentIndex++)
				{
					FMeshElement MeshElement;
					if(GetMeshElement(LODIndex,ElementIndex,FragmentIndex,PrimitiveDPG,WorldToLocal,MeshElement))
					{
						PDI->DrawMesh(MeshElement, 0, FLT_MAX);
					}
				}
			}
		} 
		else //no LOD is being forced, submit them all with appropriate cull distances
		{
			for(INT LODIndex = 0;LODIndex < NumLODs;LODIndex++)
			{
				const FStaticMeshRenderData& LODModel = StaticMesh->LODModels(LODIndex);
				const FMatrix WorldToLocal = LocalToWorld.Inverse();

				FLOAT MinDist = GetMinLODDist(LODIndex);
				FLOAT MaxDist = GetMaxLODDist(LODIndex);

				// Draw the static mesh elements.
				for(INT ElementIndex = 0;ElementIndex < LODModel.Elements.Num();ElementIndex++)
				{
					for(INT FragmentIndex = 0;FragmentIndex < LODs(LODIndex).Elements(ElementIndex).NumFragments;FragmentIndex++)
					{
						FMeshElement MeshElement;
						if(GetMeshElement(LODIndex,ElementIndex,FragmentIndex,PrimitiveDPG,WorldToLocal,MeshElement))
						{
							PDI->DrawMesh(MeshElement, MinDist, MaxDist);
						}
					}
				}
			}
		}
	}
}

/** Determines if any collision should be drawn for this mesh. */
UBOOL FStaticMeshSceneProxy::ShouldDrawCollision(const FSceneView* View)
{
	if((View->Family->ShowFlags & SHOW_CollisionNonZeroExtent) && bBlockNonZeroExtent && bShouldCollide)
	{
		return TRUE;
	}

	if((View->Family->ShowFlags & SHOW_CollisionZeroExtent) && bBlockZeroExtent && bShouldCollide)
	{
		return TRUE;
	}	

	if((View->Family->ShowFlags & SHOW_CollisionRigidBody) && bBlockRigidBody)
	{
		return TRUE;
	}

	return FALSE;
}

/** Determines if the simple or complex collision should be drawn for a particular static mesh. */
UBOOL FStaticMeshSceneProxy::ShouldDrawSimpleCollision(const FSceneView* View, const UStaticMesh* Mesh)
{
	if(Mesh->UseSimpleBoxCollision && (View->Family->ShowFlags & SHOW_CollisionNonZeroExtent))
	{
		return TRUE;
	}	

	if(Mesh->UseSimpleLineCollision && (View->Family->ShowFlags & SHOW_CollisionZeroExtent))
	{
		return TRUE;
	}	

	if(Mesh->UseSimpleRigidBodyCollision && (View->Family->ShowFlags & SHOW_CollisionRigidBody))
	{
		return TRUE;
	}

	return FALSE;
}

/**
 * @return		The index into DecalLightCaches of the specified component, or INDEX_NONE if not found.
 */
INT FStaticMeshSceneProxy::FindDecalLightCacheIndex(const UDecalComponent* DecalComponent) const
{
	for( INT DecalIndex = 0 ; DecalIndex < DecalLightCaches.Num() ; ++DecalIndex )
	{
		const FDecalLightCache& DecalLightCache = DecalLightCaches(DecalIndex);
		if( DecalLightCache.GetDecalComponent() == DecalComponent )
		{
			return DecalIndex;
		}
	}
	return INDEX_NONE;
}

void FStaticMeshSceneProxy::InitLitDecalFlags(UINT InDepthPriorityGroup)
{
	// When drawing the first set of decals for this light, the blend state needs to be "set" rather
	// than "add."  Subsequent calls use "add" to accumulate color.
	for( INT DecalIndex = 0 ; DecalIndex < DecalLightCaches.Num() ; ++DecalIndex )
	{
		FDecalLightCache& DecalLightCache = DecalLightCaches(DecalIndex);
		DecalLightCache.ClearFlags();
	}
}

/**
* Draws the primitive's dynamic decal elements.  This is called from the rendering thread for each frame of each view.
* The dynamic elements will only be rendered if GetViewRelevance declares dynamic relevance.
* Called in the rendering thread.
*
* @param	PDI						The interface which receives the primitive elements.
* @param	View					The view which is being rendered.
* @param	InDepthPriorityGroup	The DPG which is being rendered.
* @param	bDynamicLightingPass	TRUE if drawing dynamic lights, FALSE if drawing static lights.
* @param	bTranslucentReceiverPass	TRUE during the decal pass for translucent receivers, FALSE for opaque receivers.
*/
void FStaticMeshSceneProxy::DrawDynamicDecalElements(
									  FPrimitiveDrawInterface* PDI,
									  const FSceneView* View,
									  UINT InDepthPriorityGroup,
									  UBOOL bDynamicLightingPass,
									  UBOOL bTranslucentReceiverPass
									  )
{
	SCOPE_CYCLE_COUNTER(STAT_DecalRenderDynamicSMTime);

	checkSlow( View->Family->ShowFlags & SHOW_Decals );

	// only render decals for translucent receiver primitives during translucent receiver pass
	if( (bTranslucentReceiverPass && !MaterialViewRelevance.bTranslucency) )
	{
		return;
	} 

#if !FINAL_RELEASE
	UBOOL bRichView = IsRichView(View);
#else
	UBOOL bRichView = FALSE;
#endif

	// Compute the set of decals in this DPG.
	FMemMark MemStackMark(GRenderingThreadMemStack);
 	TArray<FDecalInteraction*,TMemStackAllocator<GRenderingThreadMemStack> > DPGDecals;
	for ( INT DecalIndex = 0 ; DecalIndex < Decals.Num() ; ++DecalIndex )
	{
		FDecalInteraction* Interaction = Decals(DecalIndex);
		if( // only render decals that haven't been added to a static batch
			(!Interaction->DecalStaticMesh || bRichView ) &&
			// match current DPG
			InDepthPriorityGroup == Interaction->DecalState.DepthPriorityGroup &&
			// only render translucent decals during translucent receiver pass 
			((Interaction->DecalState.MaterialViewRelevance.bTranslucency && bTranslucentReceiverPass) || !bTranslucentReceiverPass) &&
			// only render lit decals during dynamic lighting pass
			((Interaction->DecalState.MaterialViewRelevance.bLit && bDynamicLightingPass) || !bDynamicLightingPass) )
		{
			DPGDecals.AddItem( Interaction );
		}
	}
	// Sort decals for the translucent receiver pass
	if( bTranslucentReceiverPass )
	{
		Sort<USE_COMPARE_CONSTPOINTER(FDecalInteraction,UnStaticMeshRender)>( DPGDecals.GetTypedData(), DPGDecals.Num() );
	}
	for ( INT DecalIndex = 0 ; DecalIndex < DPGDecals.Num() ; ++DecalIndex )
	{
		FDecalInteraction* Decal	= DPGDecals(DecalIndex);
		const FDecalState& DecalState	= Decal->DecalState;
		FDecalRenderData* RenderData = Decal->RenderData;

		if( RenderData->DecalVertexFactory &&
			RenderData->NumTriangles > 0 )
		{
			UBOOL bIsDecalVisible = TRUE;

//@todo decal - decal bounds are not getting updated, so this culling won't work
#if 0
			const FBox& DecalBoundingBox = Decal->DecalState.Bounds;

			// Distance cull using decal's CullDistance (perspective views only)
			if( bIsDecalVisible && View->ViewOrigin.W > 0.0f )
			{
				// Compute the distance between the view and the decal
				FLOAT SquaredDistance = ( DecalBoundingBox.GetCenter() - View->ViewOrigin ).SizeSquared();
			    const FLOAT SquaredCullDistance = Decal->DecalState.SquaredCullDistance;
			    if( SquaredCullDistance > 0.0f && SquaredDistance > SquaredCullDistance )
			    {
				    // Too far away to render
				    bIsDecalVisible = FALSE;
			    }
			}

			if( bIsDecalVisible )
			{
				// Make sure the decal's frustum bounds are in view
				if( !View->ViewFrustum.IntersectBox( DecalBoundingBox.GetCenter(), DecalBoundingBox.GetExtent() ) )
				{
					bIsDecalVisible = FALSE;
				}
			}
#endif

			if( bIsDecalVisible )
			{
				const UDecalComponent* DecalComponent = Decal->Decal;
				const INT DecalLightCacheIndex = FindDecalLightCacheIndex( DecalComponent );				

				FMeshElement MeshElement;
				MeshElement.IndexBuffer = RenderData->bUsesIndexResources ? &RenderData->IndexBuffer : NULL;
				MeshElement.VertexFactory = RenderData->DecalVertexFactory->CastToFVertexFactory();

				MeshElement.MaterialRenderProxy = DecalState.DecalMaterial->GetRenderProxy(FALSE);
				if( DecalState.bDecalMaterialHasStaticLightingUsage )
				{
					if( bTranslucentReceiverPass )
					{
						MeshElement.LCI = RenderData->LCI;
					}
					else
					{
						MeshElement.LCI = ( DecalLightCacheIndex != INDEX_NONE ) ? &DecalLightCaches(DecalLightCacheIndex) : NULL;
					}
				}
				else
				{
					MeshElement.LCI = NULL;
				}			
				MeshElement.LocalToWorld = LocalToWorld;
				MeshElement.WorldToLocal = LocalToWorld.Inverse();
				MeshElement.FirstIndex = 0;
				MeshElement.NumPrimitives = RenderData->NumTriangles;

				const FStaticMeshRenderData& LODModel = StaticMesh->LODModels(0);
				MeshElement.MinVertexIndex = 0;
				if( RenderData->ReceiverVertexFactory )
				{
					MeshElement.MaxVertexIndex = LODModel.NumVertices-1;
				}
				else
				{
					MeshElement.MaxVertexIndex = RenderData->DecalVertexBuffer.GetNumVertices()-1;
				}
				MeshElement.ReverseCulling = LocalToWorldDeterminant < 0.0f ? TRUE : FALSE;
				MeshElement.CastShadow = FALSE;
				MeshElement.DepthBias = DecalState.DepthBias;
				MeshElement.SlopeScaleDepthBias = DecalState.SlopeScaleDepthBias;
				MeshElement.Type = PT_TriangleList;
				MeshElement.DepthPriorityGroup = InDepthPriorityGroup;
				MeshElement.bIsDecal = TRUE;
				MeshElement.DecalState = &Decal->DecalState;

				// set decal vertex factory parameters (in local space)
				const FDecalLocalSpaceInfo DecalLocal(&DecalState,DecalState.AttachmentLocalToWorld,DecalState.AttachmentLocalToWorld.Inverse());
				RenderData->DecalVertexFactory->SetDecalMatrix(DecalLocal.TextureTransform);
				RenderData->DecalVertexFactory->SetDecalLocation(DecalLocal.LocalLocation);
				RenderData->DecalVertexFactory->SetDecalOffset(FVector2D(DecalState.OffsetX, DecalState.OffsetY));
				RenderData->DecalVertexFactory->SetDecalLocalBinormal(DecalLocal.LocalBinormal);
				RenderData->DecalVertexFactory->SetDecalLocalTangent(DecalLocal.LocalTangent);
				RenderData->DecalVertexFactory->SetDecalLocalNormal(DecalLocal.LocalNormal);

				static const FLinearColor WireColor(0.5f,1.0f,0.5f);
				const INT NumPasses = DrawRichMesh(PDI,MeshElement,WireColor,LevelColor,PropertyColor,PrimitiveSceneInfo,FALSE);

				INC_DWORD_STAT_BY(STAT_DecalTriangles,MeshElement.NumPrimitives*NumPasses);
				INC_DWORD_STAT(STAT_DecalDrawCalls);

#if 0
				if( RenderData )
				{
					RenderData->DebugDraw(PDI,DecalState,MeshElement.LocalToWorld,SDPG_World);
				}
#endif
			}
		}
	}
}

/**
* Draws the primitive's static decal elements.  This is called from the game thread whenever this primitive is attached
* as a receiver for a decal.
*
* The static elements will only be rendered if GetViewRelevance declares both static and decal relevance.
* Called in the game thread.
*
* @param PDI - The interface which receives the primitive elements.
*/
void FStaticMeshSceneProxy::DrawStaticDecalElements(FStaticPrimitiveDrawInterface* PDI,const FDecalInteraction& DecalInteraction)
{
	if( !HasViewDependentDPG() &&
		// decals should render dynamically on movable meshes 
		UseStaticDecal() &&
		// only add static decal batches for decals projecting on opaque receivers or if the decal is opaque itself
		(MaterialViewRelevance.bOpaque || DecalInteraction.DecalState.MaterialViewRelevance.bOpaque) &&
		DecalInteraction.RenderData->DecalVertexFactory &&
		DecalInteraction.RenderData->NumTriangles > 0 )
	{
		const FDecalState& DecalState	= DecalInteraction.DecalState;
		FDecalRenderData* RenderData = DecalInteraction.RenderData;

		FMeshElement MeshElement;
		MeshElement.IndexBuffer = RenderData->bUsesIndexResources ? &RenderData->IndexBuffer : NULL;		
		MeshElement.VertexFactory = RenderData->DecalVertexFactory->CastToFVertexFactory();
		MeshElement.MaterialRenderProxy = DecalState.DecalMaterial->GetRenderProxy(FALSE);

		// This makes the decal render using a scissor rect (for performance reasons).
		MeshElement.DecalState = &DecalState;

		MeshElement.LocalToWorld = LocalToWorld;
		MeshElement.WorldToLocal = LocalToWorld.Inverse();
		MeshElement.FirstIndex = 0;
		MeshElement.NumPrimitives = RenderData->NumTriangles;
		const FStaticMeshRenderData& LODModel = StaticMesh->LODModels(0);
		MeshElement.MinVertexIndex = 0;
		if( RenderData->ReceiverVertexFactory )
		{
			MeshElement.MaxVertexIndex = LODModel.NumVertices-1;
		}
		else
		{
			MeshElement.MaxVertexIndex = RenderData->DecalVertexBuffer.GetNumVertices()-1;
		}		
		MeshElement.ReverseCulling = LocalToWorldDeterminant < 0.0f ? TRUE : FALSE;
		MeshElement.CastShadow = FALSE;
		MeshElement.DepthBias = DecalState.DepthBias;
		MeshElement.SlopeScaleDepthBias = DecalState.SlopeScaleDepthBias;
		MeshElement.Type = PT_TriangleList;
		MeshElement.DepthPriorityGroup = GetStaticDepthPriorityGroup();
		MeshElement.bIsDecal = TRUE;

		// set decal vertex factory parameters (in local space)
		const FDecalLocalSpaceInfo DecalLocal(&DecalState,DecalState.AttachmentLocalToWorld,DecalState.AttachmentLocalToWorld.Inverse());
		RenderData->DecalVertexFactory->SetDecalMatrix(DecalLocal.TextureTransform);
		RenderData->DecalVertexFactory->SetDecalLocation(DecalLocal.LocalLocation);
		RenderData->DecalVertexFactory->SetDecalOffset(FVector2D(DecalState.OffsetX, DecalState.OffsetY));
		RenderData->DecalVertexFactory->SetDecalLocalBinormal(DecalLocal.LocalBinormal);
		RenderData->DecalVertexFactory->SetDecalLocalTangent(DecalLocal.LocalTangent);
		RenderData->DecalVertexFactory->SetDecalLocalNormal(DecalLocal.LocalNormal);
		
		MeshElement.LCI = NULL;
		if( DecalState.bDecalMaterialHasStaticLightingUsage )
		{
			// The decal is lit and so should have an entry in the DecalLightCaches list.
			const INT DecalLightCacheIndex = FindDecalLightCacheIndex( DecalInteraction.Decal );
			if( DecalLightCaches.IsValidIndex(DecalLightCacheIndex) )
			{
				MeshElement.LCI = &DecalLightCaches(DecalLightCacheIndex);
			}
			else
			{
				debugfSuppressed( TEXT("Missing Decal LCI Receiver=%s Decal=%s"), 
					Owner ? *Owner->GetName() : TEXT("None"),
					DecalState.DecalComponent && DecalState.DecalComponent->GetOwner() 
					? *DecalState.DecalComponent->GetOwner()->GetName() : TEXT("None") );
			}
		}

		PDI->DrawMesh(MeshElement,0,FLT_MAX);
	}	
}

/**
 * Adds a decal interaction to the primitive.  This is called in the rendering thread by AddDecalInteraction_GameThread.
 */
void FStaticMeshSceneProxy::AddDecalInteraction_RenderingThread(const FDecalInteraction& DecalInteraction)
{
	FPrimitiveSceneProxy::AddDecalInteraction_RenderingThread( DecalInteraction );

	// Transform the decal frustum verts into local space.  These will be transformed
	// each frame into world space for the the scissor test.
	FDecalInteraction& NewInteraction = *Decals(Decals.Num()-1);
	NewInteraction.DecalState.TransformFrustumVerts( LocalToWorld.Inverse() );

	// Cache any lighting information for the decal.
	CreateDecalLightCache( NewInteraction );

	// add the static mesh element for this decal interaction
	NewInteraction.CreateDecalStaticMesh(PrimitiveSceneInfo);
}

/**
 * Removes a decal interaction from the primitive.  This is called in the rendering thread by RemoveDecalInteraction_GameThread.
 */
void FStaticMeshSceneProxy::RemoveDecalInteraction_RenderingThread(UDecalComponent* DecalComponent)
{
	FPrimitiveSceneProxy::RemoveDecalInteraction_RenderingThread( DecalComponent );

	// Find the decal interaction representing the given decal component, and remove it from the interaction list.
	const INT DecalLightCacheIndex = FindDecalLightCacheIndex( DecalComponent );
	if ( DecalLightCacheIndex != INDEX_NONE )
	{
		DecalLightCaches.Remove( DecalLightCacheIndex );
	}
}

/** 
* Draw the scene proxy as a dynamic element
*
* @param	PDI - draw interface to render to
* @param	View - current view
* @param	DPGIndex - current depth priority 
* @param	Flags - optional set of flags from EDrawDynamicElementFlags
*/
void FStaticMeshSceneProxy::DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex,DWORD Flags)
{
	// Bool that defines how we should draw the collision for this mesh.
	const UBOOL bIsCollisionView = IsCollisionView(View);
	const UBOOL bDrawCollision = bIsCollisionView && ShouldDrawCollision(View);
	const UBOOL bDrawSimple = ShouldDrawSimpleCollision(View, StaticMesh);
	const UBOOL bDrawComplexCollision = (bDrawCollision && !bDrawSimple);
	const UBOOL bDrawSimpleCollision = (bDrawCollision && bDrawSimple);
	const UBOOL bDrawMesh = bIsCollisionView ?
								bDrawComplexCollision :
								IsRichView(View) || HasViewDependentDPG() || IsMovable();

	// Determine the DPG the primitive should be drawn in for this view.
	if (GetDepthPriorityGroup(View) == DPGIndex)
	{
		// Draw polygon mesh if we are either not in a collision view, or are drawing it as collision.
		if((View->Family->ShowFlags & SHOW_StaticMeshes) && bDrawMesh )
		{
			const UBOOL bLevelColorationEnabled = (View->Family->ShowFlags & SHOW_LevelColoration) ? TRUE : FALSE;
			const UBOOL bPropertyColorationEnabled = (View->Family->ShowFlags & SHOW_PropertyColoration) ? TRUE : FALSE;

			// Determine the LOD to use.
			FLOAT Distance = (FVector(View->ViewOrigin)-PrimitiveSceneInfo->Bounds.Origin).Size();
			INT LODIndex = GetLOD(Distance);
			const FStaticMeshRenderData& LODModel = StaticMesh->LODModels(LODIndex);

			if( (View->Family->ShowFlags & SHOW_Wireframe) && !(View->Family->ShowFlags & SHOW_Materials) )
			{
				FLinearColor ViewWireframeColor( bLevelColorationEnabled ? LevelColor : WireframeColor );
				if ( bPropertyColorationEnabled )
				{
					ViewWireframeColor = PropertyColor;
				}
				// Use collision color if we are drawing this as collision
				else if(bDrawComplexCollision)
				{
					ViewWireframeColor = FLinearColor(GEngine->C_ScaleBoxHi);
				}

				FColoredMaterialRenderProxy WireframeMaterialInstance(
					GEngine->WireframeMaterial->GetRenderProxy(FALSE),
					GetSelectionColor(ViewWireframeColor,!(View->Family->ShowFlags & SHOW_Selection)||bSelected)
					);

				FMeshElement Mesh;
				Mesh.VertexFactory = &LODModel.VertexFactory;
				Mesh.MaterialRenderProxy = &WireframeMaterialInstance;
				Mesh.LocalToWorld = LocalToWorld;
				Mesh.WorldToLocal = LocalToWorld.Inverse();
				Mesh.MinVertexIndex = 0;
				Mesh.MaxVertexIndex = LODModel.NumVertices - 1;
				Mesh.ReverseCulling = LocalToWorldDeterminant < 0.0f ? TRUE : FALSE;
				Mesh.CastShadow = bCastShadow;
				Mesh.DepthPriorityGroup = (ESceneDepthPriorityGroup)DPGIndex;
				
				SetIndexSource(LODIndex, 0, 0, Mesh, TRUE);

				const INT NumPasses = PDI->DrawMesh(Mesh);

				INC_DWORD_STAT_BY(STAT_StaticMeshTriangles,Mesh.NumPrimitives * NumPasses);
			}
			else
			{
				const FLinearColor UtilColor( IsCollisionView(View) ? FLinearColor(GEngine->C_ScaleBoxHi) : LevelColor );
				const FMatrix WorldToLocal = LocalToWorld.Inverse();

				// Draw the static mesh elements.
				for(INT ElementIndex = 0;ElementIndex < LODModel.Elements.Num();ElementIndex++)
				{
					for(INT FragmentIndex = 0;FragmentIndex < LODs(LODIndex).Elements(ElementIndex).NumFragments;FragmentIndex++)
					{
						FMeshElement MeshElement;
						if(GetMeshElement(LODIndex,ElementIndex,FragmentIndex,DPGIndex,WorldToLocal,MeshElement))
						{
							const INT NumPasses = DrawRichMesh(
								PDI,
								MeshElement,
								WireframeColor,
								UtilColor,
								PropertyColor,
								PrimitiveSceneInfo,
								bSelected
								);

							INC_DWORD_STAT_BY(STAT_StaticMeshTriangles,MeshElement.NumPrimitives * NumPasses);
						}
					}
				}
			}
		}
	}

	if(DPGIndex == SDPG_World && ((View->Family->ShowFlags & SHOW_Collision) && bShouldCollide) || (bDrawSimpleCollision))
	{
		if(StaticMesh->BodySetup)
		{
			// Make a material for drawing solid collision stuff
			const UMaterial* LevelColorationMaterial = (View->Family->ShowFlags & SHOW_Lighting) ? GEngine->ShadedLevelColorationLitMaterial : GEngine->ShadedLevelColorationUnlitMaterial;
			const FColoredMaterialRenderProxy CollisionMaterialInstance(
				LevelColorationMaterial->GetRenderProxy(bSelected),
				FLinearColor(GEngine->C_ScaleBoxHi)
				);

			// Draw the static mesh's body setup.

			// Get transform without scaling.
			FMatrix GeomMatrix = LocalToWorld;
			FVector RecipScale( 1.f/TotalScale3D.X, 1.f/TotalScale3D.Y, 1.f/TotalScale3D.Z );

			GeomMatrix.M[0][0] *= RecipScale.X;
			GeomMatrix.M[0][1] *= RecipScale.X;
			GeomMatrix.M[0][2] *= RecipScale.X;

			GeomMatrix.M[1][0] *= RecipScale.Y;
			GeomMatrix.M[1][1] *= RecipScale.Y;
			GeomMatrix.M[1][2] *= RecipScale.Y;

			GeomMatrix.M[2][0] *= RecipScale.Z;
			GeomMatrix.M[2][1] *= RecipScale.Z;
			GeomMatrix.M[2][2] *= RecipScale.Z;

			// Slight hack here - draw each hull in a different color if no Owner (usually in a tool like StaticMeshEditor).
			UBOOL bDrawSimpleSolid = (bDrawSimpleCollision && !(View->Family->ShowFlags & SHOW_Wireframe));

			// In old wireframe collision mode, always draw the wireframe highlighted (selected or not).
			UBOOL bDrawWireSelected = bSelected;
			if(View->Family->ShowFlags & SHOW_Collision)
			{
				bDrawWireSelected = TRUE;
			}
 
			// Differentiate the color based on bBlockNonZeroExtent.  Helps greatly with skimming a level for optimization opportunities.
			FColor collisionColor = bBlockNonZeroExtent ? FColor(223,149,157,255) : FColor(157,149,223,255);

			StaticMesh->BodySetup->AggGeom.DrawAggGeom(PDI, GeomMatrix, TotalScale3D, GetSelectionColor(collisionColor, bDrawWireSelected), &CollisionMaterialInstance, (Owner == NULL), bDrawSimpleSolid);
		}
	}

	if(DPGIndex == SDPG_Foreground && (View->Family->ShowFlags & SHOW_Bounds) && (View->Family->ShowFlags & SHOW_StaticMeshes) && (GIsGame || !Owner || bSelected))
	{
		// Draw the static mesh's bounding box and sphere.
		DrawWireBox(PDI,PrimitiveSceneInfo->Bounds.GetBox(), FColor(72,72,255),SDPG_Foreground);
		DrawCircle(PDI,PrimitiveSceneInfo->Bounds.Origin,FVector(1,0,0),FVector(0,1,0),FColor(255,255,0),PrimitiveSceneInfo->Bounds.SphereRadius,32,SDPG_Foreground);
		DrawCircle(PDI,PrimitiveSceneInfo->Bounds.Origin,FVector(1,0,0),FVector(0,0,1),FColor(255,255,0),PrimitiveSceneInfo->Bounds.SphereRadius,32,SDPG_Foreground);
		DrawCircle(PDI,PrimitiveSceneInfo->Bounds.Origin,FVector(0,1,0),FVector(0,0,1),FColor(255,255,0),PrimitiveSceneInfo->Bounds.SphereRadius,32,SDPG_Foreground);
	}
}

/**
 * Called by the rendering thread to notify the proxy when a light is no longer
 * associated with the proxy, so that it can clean up any cached resources.
 * @param Light - The light to be removed.
 */
void FStaticMeshSceneProxy::OnDetachLight(const FLightSceneInfo* Light)
{
	CachedShadowVolumes.RemoveShadowVolume(Light);
}

void FStaticMeshSceneProxy::OnTransformChanged()
{
	// Update the cached scaling.
	TotalScale3D.X = FVector(LocalToWorld.TransformNormal(FVector(1,0,0))).Size();
	TotalScale3D.Y = FVector(LocalToWorld.TransformNormal(FVector(0,1,0))).Size();
	TotalScale3D.Z = FVector(LocalToWorld.TransformNormal(FVector(0,0,1))).Size();
}

/**
 * Removes potentially cached shadow volume data for the passed in light.
 *
 * @param Light		The light for which cached shadow volume data will be removed.
 */
void FStaticMeshSceneProxy::RemoveCachedShadowVolumeData( const FLightSceneInfo* Light )
{
	CachedShadowVolumes.RemoveShadowVolume(Light);
}

/**
 * Called from the rendering thread, in the FSceneRenderer::RenderLights phase.
 */
void FStaticMeshSceneProxy::DrawShadowVolumes(FShadowVolumeDrawInterface* SVDI,const FSceneView* View,const FLightSceneInfo* Light,UINT DPGIndex)
{
	checkSlow(UEngine::ShadowVolumesAllowed());
	
	// Determine the DPG the primitive should be drawn in for this view.
	if ((GetViewRelevance(View).GetDPG(DPGIndex) == FALSE) ||
		// Don't draw shadows if we are in a collision geometry drawing mode.
		IsCollisionView(View) )
	{
		return;
	}

	SCOPE_CYCLE_COUNTER(STAT_ShadowVolumeRenderTime);

	//@todo: Use MemMark/MemPop for PlaneDots
	//@todo: Use a (not-yet-existing) dynamic IB API for IndexBuffer
	FLOAT Distance = (FVector(View->ViewOrigin)-PrimitiveSceneInfo->Bounds.Origin).Size();
	INT LODIndex = GetLOD(Distance);
	const FStaticMeshRenderData& LODModel = StaticMesh->LODModels(LODIndex);

	// Check for the shadow volume in the cache.
	const FShadowVolumeCache::FCachedShadowVolume* CachedShadowVolume = CachedShadowVolumes.GetShadowVolume(Light);
	
	//check if the LOD has changed since last cache
	if (CachedShadowVolume && LastLOD != LODIndex) 
	{
		//remove the cache
		CachedShadowVolumes.RemoveShadowVolume(Light);

		//trigger a cache rebuild
		CachedShadowVolume = NULL;

		//update the lastLOD that the cache was updated at
		LastLOD = LODIndex;
	}
	
	if (!CachedShadowVolume)
	{
		SCOPE_CYCLE_COUNTER(STAT_ShadowExtrusionTime);

		FVector4	LightPosition	= LocalToWorld.Inverse().TransformFVector4(Light->GetPosition());
		UINT		NumTriangles	= (UINT)LODModel.IndexBuffer.Indices.Num() / 3;
		FLOAT*		PlaneDots		= new FLOAT[NumTriangles];
		const WORD*	Indices			= &LODModel.IndexBuffer.Indices(0);
		WORD		FirstExtrudedVertex = (WORD)LODModel.NumVertices;
		FShadowIndexBuffer IndexBuffer;

#if XBOX
		// Use Xbox optimized function to get plane dots
		GetPlaneDotsXbox(NumTriangles,LightPosition, LODModel,(WORD*)Indices,PlaneDots);
#else
		for(UINT TriangleIndex = 0;TriangleIndex < NumTriangles;TriangleIndex++)
		{
			const FVector&	V1 = LODModel.PositionVertexBuffer.VertexPosition(Indices[TriangleIndex * 3 + 0]),
							V2 = LODModel.PositionVertexBuffer.VertexPosition(Indices[TriangleIndex * 3 + 1]),
							V3 = LODModel.PositionVertexBuffer.VertexPosition(Indices[TriangleIndex * 3 + 2]);
			PlaneDots[TriangleIndex] = (((V2-V3) ^ (V1-V3)) | (FVector(LightPosition) - V1 * LightPosition.W));
		}
#endif
		IndexBuffer.Indices.Empty(NumTriangles * 3);

		for(UINT TriangleIndex = 0;TriangleIndex < NumTriangles;TriangleIndex++)
		{
			if(IsNegativeFloat(PlaneDots[TriangleIndex]))
			{
				IndexBuffer.AddFace(
					FirstExtrudedVertex + Indices[TriangleIndex * 3 + 0],
					FirstExtrudedVertex + Indices[TriangleIndex * 3 + 1],
					FirstExtrudedVertex + Indices[TriangleIndex * 3 + 2]
					);

				IndexBuffer.AddFace(
					Indices[TriangleIndex * 3 + 2],
					Indices[TriangleIndex * 3 + 1],
					Indices[TriangleIndex * 3 + 0]
					);
			}
		}

		for(UINT EdgeIndex = 0;EdgeIndex < (UINT)LODModel.Edges.Num();EdgeIndex++)
		{
			const FMeshEdge& Edge = LODModel.Edges(EdgeIndex);
			if(	(Edge.Faces[1] == INDEX_NONE && IsNegativeFloat(PlaneDots[Edge.Faces[0]])) ||
				(Edge.Faces[1] != INDEX_NONE && IsNegativeFloat(PlaneDots[Edge.Faces[0]]) != IsNegativeFloat(PlaneDots[Edge.Faces[1]])))
			{
				IndexBuffer.AddEdge(
					Edge.Vertices[IsNegativeFloat(PlaneDots[Edge.Faces[0]]) ? 1 : 0],
					Edge.Vertices[IsNegativeFloat(PlaneDots[Edge.Faces[0]]) ? 0 : 1],
					FirstExtrudedVertex
					);
			}
		}

		delete [] PlaneDots;

		// Add the new shadow volume to the cache.
		CachedShadowVolume = CachedShadowVolumes.AddShadowVolume(Light,IndexBuffer);
	}

	// Draw the cached shadow volume.
	if(CachedShadowVolume->NumTriangles)
	{
		INC_DWORD_STAT_BY(STAT_ShadowVolumeTriangles, CachedShadowVolume->NumTriangles);
		SVDI->DrawShadowVolume( CachedShadowVolume->IndexBufferRHI, LODModel.ShadowVertexFactory, LocalToWorld, 0, CachedShadowVolume->NumTriangles, 0, LODModel.NumVertices * 2 - 1 );
	}
}

FPrimitiveViewRelevance FStaticMeshSceneProxy::GetViewRelevance(const FSceneView* View)
{   
	FPrimitiveViewRelevance Result;
	if(View->Family->ShowFlags & SHOW_StaticMeshes)
	{
		if(IsShown(View))
		{
#if !FINAL_RELEASE
			if(IsCollisionView(View))
			{
				Result.bDynamicRelevance = TRUE;
				Result.bForceDirectionalLightsDynamic = TRUE;
			}
			else 
#endif
			if(
#if !FINAL_RELEASE
				IsRichView(View) || 
#endif
				HasViewDependentDPG() ||
				IsMovable()	)
			{
				Result.bDynamicRelevance = TRUE;
			}
			else
			{
				Result.bStaticRelevance = TRUE;
			}
			Result.SetDPG(GetDepthPriorityGroup(View),TRUE);
		}

#if !FINAL_RELEASE
		if(View->Family->ShowFlags & (SHOW_Bounds|SHOW_Collision))
		{
			Result.bDynamicRelevance = TRUE;
		}

		// only add to foreground DPG for debug rendering
		if(View->Family->ShowFlags & SHOW_Bounds)
		{
			Result.SetDPG(SDPG_Foreground,TRUE);
		}
#endif
		if (IsShadowCast(View))
		{
			Result.bShadowRelevance = TRUE;
		}

		MaterialViewRelevance.SetPrimitiveViewRelevance(Result);

		Result.bDecalStaticRelevance = HasRelevantStaticDecals(View);
		Result.bDecalDynamicRelevance = HasRelevantDynamicDecals(View);
	}
	return Result;
}


/**
 *	Determines the relevance of this primitive's elements to the given light.
 *	@param	LightSceneInfo			The light to determine relevance for
 *	@param	bDynamic (output)		The light is dynamic for this primitive
 *	@param	bRelevant (output)		The light is relevant for this primitive
 *	@param	bLightMapped (output)	The light is light mapped for this primitive
 */
void FStaticMeshSceneProxy::GetLightRelevance(FLightSceneInfo* LightSceneInfo, UBOOL& bDynamic, UBOOL& bRelevant, UBOOL& bLightMapped)
{
	// Attach the light to the primitive's static meshes.
	bDynamic = TRUE;
	bRelevant = FALSE;
	bLightMapped = TRUE;

	if (LODs.Num() > 0)
	{
		for(INT LODIndex = 0; LODIndex < LODs.Num(); LODIndex++)
		{
			FLODInfo* LCI = &LODs(LODIndex);
			if (LCI)
			{
				ELightInteractionType InteractionType = LCI->GetInteraction(LightSceneInfo).GetType();
				if(InteractionType != LIT_CachedIrrelevant)
				{
					bRelevant = TRUE;
				}
				if(InteractionType != LIT_CachedLightMap && InteractionType != LIT_CachedIrrelevant)
				{
					bLightMapped = FALSE;
				}
				if(InteractionType != LIT_Uncached)
				{
					bDynamic = FALSE;
				}
			}
		}
	}
	else
	{
		bRelevant = TRUE;
		bLightMapped = FALSE;
	}
}

/** Initialization constructor. */
FStaticMeshSceneProxy::FLODInfo::FLODInfo(const UStaticMeshComponent* InComponent,INT InLODIndex):
	Component(InComponent),
	LODIndex(InLODIndex)
{
	// Determine if the LOD has static lighting.
	UBOOL bHasStaticLighting = FALSE;
	if(LODIndex < Component->LODData.Num())
	{
		const FStaticMeshComponentLODInfo& ComponentLODInfo = Component->LODData(LODIndex);
		bHasStaticLighting = ComponentLODInfo.LightMap != NULL || ComponentLODInfo.ShadowMaps.Num() || ComponentLODInfo.ShadowVertexBuffers.Num();
	}

	// Gather the materials applied to the LOD.
	Elements.Empty(Component->StaticMesh->LODModels(LODIndex).Elements.Num());
	for(INT ElementIndex = 0;ElementIndex < Component->StaticMesh->LODModels(LODIndex).Elements.Num();ElementIndex++)
	{
		const FStaticMeshElement& Element = Component->StaticMesh->LODModels(LODIndex).Elements(ElementIndex);
		FElementInfo ElementInfo;

		// Determine the material applied to this element of the LOD.
		ElementInfo.Material = Component->GetMaterial(Element.MaterialIndex,LODIndex);

		// If there isn't an applied material, or if we need static lighting and it doesn't support it, fall back to the default material.
		if(!ElementInfo.Material || (bHasStaticLighting && !ElementInfo.Material->CheckMaterialUsage(MATUSAGE_StaticLighting)))
		{
			ElementInfo.Material = GEngine->DefaultMaterial;
		}

		// Store the element info.
		Elements.AddItem(ElementInfo);
	}
}

// FLightCacheInterface.
FLightInteraction FStaticMeshSceneProxy::FLODInfo::GetInteraction(const FLightSceneInfo* LightSceneInfo) const
{
	// Check if the light has static lighting or shadowing.
	// This directly accesses the component's static lighting with the assumption that it won't be changed without synchronizing with the rendering thread.
	if(LightSceneInfo->bStaticShadowing)
	{
		if(LODIndex < Component->LODData.Num())
		{
			const FStaticMeshComponentLODInfo& LODInstanceData = Component->LODData(LODIndex);
			if(LODInstanceData.LightMap)
			{
				if(LODInstanceData.LightMap->ContainsLight(LightSceneInfo->LightmapGuid))
				{
					return FLightInteraction::LightMap();
				}
			}
			for(INT LightIndex = 0;LightIndex < LODInstanceData.ShadowVertexBuffers.Num();LightIndex++)
			{
				const UShadowMap1D* const ShadowVertexBuffer = LODInstanceData.ShadowVertexBuffers(LightIndex);
				if(ShadowVertexBuffer && ShadowVertexBuffer->GetLightGuid() == LightSceneInfo->LightGuid)
				{
					return FLightInteraction::ShadowMap1D(ShadowVertexBuffer);
				}
			}
			for(INT LightIndex = 0;LightIndex < LODInstanceData.ShadowMaps.Num();LightIndex++)
			{
				const UShadowMap2D* const ShadowMap = LODInstanceData.ShadowMaps(LightIndex);
				if(ShadowMap && ShadowMap->IsValid() && ShadowMap->GetLightGuid() == LightSceneInfo->LightGuid)
				{
					return FLightInteraction::ShadowMap2D(
						LODInstanceData.ShadowMaps(LightIndex)->GetTexture(),
						LODInstanceData.ShadowMaps(LightIndex)->GetCoordinateScale(),
						LODInstanceData.ShadowMaps(LightIndex)->GetCoordinateBias()
						);
				}
			}
		}
		
		if(Component->IrrelevantLights.ContainsItem(LightSceneInfo->LightGuid))
		{
			return FLightInteraction::Irrelevant();
		}
	}

	// Use dynamic lighting if the light doesn't have static lighting.
	return FLightInteraction::Uncached();
}

FStaticMeshSceneProxy::FDecalLightCache::FDecalLightCache(const FDecalInteraction& DecalInteraction, const FStaticMeshSceneProxy& Proxy)
	: DecalComponent( DecalInteraction.Decal )
{
	ClearFlags();

	// Build the static light interaction map.
	for( INT LightIndex = 0 ; LightIndex < Proxy.StaticMeshComponent->IrrelevantLights.Num() ; ++LightIndex )
	{
		StaticLightInteractionMap.Set( Proxy.StaticMeshComponent->IrrelevantLights(LightIndex), FLightInteraction::Irrelevant() );
	}

	// If a custom vertex lightmap does not exist but sample remapping info is present, allocate
	if( !DecalInteraction.RenderData->LightMap1D &&
		DecalInteraction.DecalState.SampleRemapping.Num() > 0 )
	{
		const FLightMap* SourceLightMap = Proxy.LODs(0).GetLightMap();
		if( SourceLightMap )
		{
			FLightMap1D* SourceLightMap1D = const_cast<FLightMap1D*>(SourceLightMap->GetLightMap1D());
			if( SourceLightMap1D )
			{
				// Create the vertex light map data.
				DecalInteraction.RenderData->LightMap1D = SourceLightMap1D->DuplicateWithRemappedVerts( DecalInteraction.DecalState.SampleRemapping );
			}
		}
	}

	// Toss the sample remapping now that a lightmap has been created.
	DecalInteraction.DecalState.SampleRemapping.Empty();

	// If a custom vertex lightmap was specified with the decal, use it.
	// Otherwise, use the mesh's lightmap texture.
	if ( DecalInteraction.RenderData->LightMap1D )
	{
		// Decal vertex lightmap.
		LightMap = DecalInteraction.RenderData->LightMap1D;
	}
	else
	{
		// Lightmap texture from the underlying mesh.
		LightMap = Proxy.LODs(0).GetLightMap();
	}
	if(LightMap)
	{
		for( INT LightIndex = 0 ; LightIndex < LightMap->LightGuids.Num() ; ++LightIndex )
		{
			StaticLightInteractionMap.Set( LightMap->LightGuids(LightIndex), FLightInteraction::LightMap() );
		}
	}
}


/**
* Clears flags used to track whether or not this decal has already been drawn for dynamic lighting.
* When drawing the first set of decals for this light, the blend state needs to be "set" rather
* than "add."  Subsequent calls use "add" to accumulate color.
*/
void FStaticMeshSceneProxy::FDecalLightCache::ClearFlags()
{
	for ( INT DPGIndex = 0 ; DPGIndex < SDPG_MAX_SceneRender ; ++DPGIndex )
	{
		Flags[DPGIndex] = FALSE;
	}
}

// FLightCacheInterface.
FLightInteraction FStaticMeshSceneProxy::FDecalLightCache::GetInteraction(const FLightSceneInfo* LightSceneInfo) const
{
	// Check for a static light interaction.
	const FLightInteraction* Interaction = StaticLightInteractionMap.Find(LightSceneInfo->LightmapGuid);
	if(!Interaction)
	{
		Interaction = StaticLightInteractionMap.Find(LightSceneInfo->LightGuid);
	}
	return Interaction ? *Interaction : FLightInteraction::Uncached();
}

/**
 * Returns the minimum distance that the given LOD should be displayed at
 *
 * @param CurrentLevel - the LOD to find the min distance for
 */
FLOAT FStaticMeshSceneProxy::GetMinLODDist(INT CurrentLevel) const 
{
	//Scale LODMaxRange by LODDistanceRatio and then split this range up by the number of LOD's
	FLOAT MinDist = CurrentLevel * StaticMesh->LODMaxRange * StaticMesh->LODDistanceRatio / StaticMesh->LODModels.Num();
	return MinDist;
}

/**
 * Returns the maximum distance that the given LOD should be displayed at
 * If the given LOD is the lowest detail LOD, then its maxDist will be FLT_MAX
 *
 * @param CurrentLevel - the LOD to find the max distance for
 */
FLOAT FStaticMeshSceneProxy::GetMaxLODDist(INT CurrentLevel) const 
{
	//This level's MaxDist is the next level's MinDist
	FLOAT MaxDist = GetMinLODDist(CurrentLevel + 1);

	//If the lowest detail LOD was passed in, set MaxDist to FLT_MAX so that it doesn't get culled
	if (CurrentLevel + 1 == StaticMesh->LODModels.Num()) 
	{
		MaxDist = FLT_MAX;
	} 
	return MaxDist;
}

/**
 * Returns the LOD that should be used at the given distance
 *
 * @param Distance - distance from the current view to the component's bound origin
 */
INT FStaticMeshSceneProxy::GetLOD(FLOAT Distance) const 
{
	//If an LOD is being forced, use that one
	if (ForcedLodModel > 0)
	{
		return ::Clamp(ForcedLodModel, 1, StaticMesh->LODModels.Num()) - 1;
	}

	//Calculate the maximum distance for the lowest detail LOD
	FLOAT EndDistance = StaticMesh->LODDistanceRatio * StaticMesh->LODMaxRange;

	//Get the percentage of the EndDistance that Distance is, then use this to choose the appropriate LOD
	INT NewLOD = appTrunc(FLOAT(StaticMesh->LODModels.Num()) * Distance / EndDistance);

	//make sure the result is valid
	NewLOD = Clamp(NewLOD, 0, StaticMesh->LODModels.Num() - 1);

	return NewLOD;
}

FPrimitiveSceneProxy* UStaticMeshComponent::CreateSceneProxy()
{
	//@todo: figure out why i need a ::new (gcc3-specific)
	return ::new FStaticMeshSceneProxy(this);
}

UBOOL UStaticMeshComponent::ShouldRecreateProxyOnUpdateTransform() const
{
	// If the primitive is movable during gameplay, it won't use static mesh elements, and the proxy doesn't need to be recreated on UpdateTransform.
	// If the primitive isn't movable during gameplay, it will use static mesh elements, and the proxy must be recreated on UpdateTransform.
	UBOOL	bMovable = FALSE;
	if( GetOwner() )
	{
		bMovable = !GetOwner()->bStatic && GetOwner()->bMovable;
	}

	return bMovable==FALSE;
}
