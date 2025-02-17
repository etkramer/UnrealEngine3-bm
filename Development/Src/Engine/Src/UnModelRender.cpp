/*=============================================================================
	UnModelRender.cpp: Unreal model rendering
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineDecalClasses.h"
#include "LevelUtils.h"
#include "HModel.h"

/*
 * ScenePrivate.h is needed by callbacks from the rendering thread:
 *   UModelComponent::BuildShadowVolume()
 *   UModelSceneProxy::DrawShadowVolumes()
 */
#include "ScenePrivate.h"


/*-----------------------------------------------------------------------------
FModelVertexBuffer
-----------------------------------------------------------------------------*/

FModelVertexBuffer::FModelVertexBuffer(UModel* InModel)
:	Vertices(FALSE)	// vertices are not CPU accessible
,	Model(InModel)
{}

void FModelVertexBuffer::InitRHI()
{
	// Calculate the buffer size.
	UINT Size = Vertices.GetResourceDataSize();
	if( Size > 0 )
	{
		// Create the buffer.
		VertexBufferRHI = RHICreateVertexBuffer(Size,&Vertices,RUF_Static);
	}
}

/**
* Serializer for this class
* @param Ar - archive to serialize to
* @param B - data to serialize
*/
FArchive& operator<<(FArchive& Ar,FModelVertexBuffer& B)
{
   B.Vertices.BulkSerialize(Ar);
   return Ar;
}

/*-----------------------------------------------------------------------------
FModelShadowVertexBuffer
-----------------------------------------------------------------------------*/

FModelShadowVertexBuffer::FModelShadowVertexBuffer(UModel* InModel)
:	FShadowVertexBuffer(0)
,	Model(InModel)
{
	check( Model );
}

void FModelShadowVertexBuffer::InitRHI()
{
	INT NumVertices = Model->Points.Num();
	if( NumVertices > 0)
	{
		Setup( NumVertices );
		FShadowVertexBuffer::InitRHI();
		UpdateVertices( &Model->Points(0), NumVertices, sizeof(Model->Points(0)) );
	}
}

/*-----------------------------------------------------------------------------
UModelComponent
-----------------------------------------------------------------------------*/

/**
 * Used to sort the model elements by material.
 */
void UModelComponent::BuildRenderData()
{
	UModel* TheModel = GetModel();

	// Initialize the component's light-map resources.
	for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
	{
		FModelElement& Element = Elements(ElementIndex);
		if(Element.LightMap != NULL)
		{
			Element.LightMap->InitResources();
		}
	}

	// Find the edges in UModel::Edges which are adjacent to this component's nodes.
	Edges.Empty();
	for(INT EdgeIndex = 0;EdgeIndex < TheModel->Edges.Num();EdgeIndex++)
	{
		FMeshEdge& Edge = TheModel->Edges(EdgeIndex);
		if(TheModel->Nodes(Edge.Faces[0]).ComponentIndex == ComponentIndex || (Edge.Faces[1] != INDEX_NONE && TheModel->Nodes(Edge.Faces[1]).ComponentIndex == ComponentIndex))
		{
			Edges.AddItem(EdgeIndex);
		}
	}

	// Build the component's index buffer and compute each element's bounding box.
	for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
	{
		FModelElement& Element = Elements(ElementIndex);

		// Find the index buffer for the element's material.
		TScopedPointer<FRawIndexBuffer32>* IndexBufferRef = TheModel->MaterialIndexBuffers.Find(Element.Material);
		if(!IndexBufferRef)
		{
			IndexBufferRef = &TheModel->MaterialIndexBuffers.Set(Element.Material,new FRawIndexBuffer32());
		}
		FRawIndexBuffer32* const IndexBuffer = *IndexBufferRef;
		check(IndexBuffer);

		Element.IndexBuffer = IndexBuffer;
		Element.FirstIndex = IndexBuffer->Indices.Num();
		Element.NumTriangles = 0;
		Element.MinVertexIndex = 0xffffffff;
		Element.MaxVertexIndex = 0;
		Element.BoundingBox.Init();
		for(INT NodeIndex = 0;NodeIndex < Element.Nodes.Num();NodeIndex++)
		{
			FBspNode& Node = TheModel->Nodes(Element.Nodes(NodeIndex));
			FBspSurf& Surf = TheModel->Surfs(Node.iSurf);

			// Don't put portal polygons in the static index buffer.
			if(Surf.PolyFlags & PF_Portal)
				continue;

			for(UINT BackFace = 0;BackFace < (UINT)((Surf.PolyFlags & PF_TwoSided) ? 2 : 1);BackFace++)
			{
				if(Node.iZone[1-BackFace] == GetZoneIndex() || GetZoneIndex() == INDEX_NONE)
				{
					for(INT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
					{
						Element.BoundingBox += TheModel->Points(TheModel->Verts(Node.iVertPool + VertexIndex).pVertex);
					}

					for(INT VertexIndex = 2;VertexIndex < Node.NumVertices;VertexIndex++)
					{
						IndexBuffer->Indices.AddItem(Node.iVertexIndex + Node.NumVertices * BackFace);
						IndexBuffer->Indices.AddItem(Node.iVertexIndex + Node.NumVertices * BackFace + VertexIndex);
						IndexBuffer->Indices.AddItem(Node.iVertexIndex + Node.NumVertices * BackFace + VertexIndex - 1);
						Element.NumTriangles++;
					}
					Element.MinVertexIndex = Min(Node.iVertexIndex + Node.NumVertices * BackFace,Element.MinVertexIndex);
					Element.MaxVertexIndex = Max(Node.iVertexIndex + Node.NumVertices * BackFace + Node.NumVertices - 1,Element.MaxVertexIndex);
				}
			}
		}

		IndexBuffer->Indices.Shrink();
	}
}

/**
 * Called from the rendering thread to generate shadow volume indices for a particular light.
 * @param IndexBuffer	Index buffer to store the generated shadow volume indices
 * @param Light			The light to generate the shadow volume from
 */
void UModelComponent::BuildShadowVolume( FShadowIndexBuffer &IndexBuffer, const FLightSceneInfo* Light ) const
{
	FVector4 LightPosition = LocalToWorld.Inverse().TransformFVector4(Light->GetPosition());
	WORD FirstExtrudedVertex = Model->Points.Num();
	FLOAT* PlaneDots = new FLOAT[Nodes.Num()];

	IndexBuffer.Indices.Empty();

	for(INT NodeIndex = 0;NodeIndex < Nodes.Num();NodeIndex++)
		PlaneDots[NodeIndex] = Model->Nodes(Nodes(NodeIndex)).Plane | FPlane(LightPosition.X,LightPosition.Y,LightPosition.Z,-LightPosition.W);

	for(INT NodeIndex = 0;NodeIndex < Nodes.Num();NodeIndex++)
	{
		FBspNode&	Node = Model->Nodes(Nodes(NodeIndex));
		FBspSurf&	Surf = Model->Surfs(Node.iSurf);
		if(!(Surf.PolyFlags & PF_TwoSided))
		{
			if(IsNegativeFloat(PlaneDots[NodeIndex]))
			{
				for(UINT VertexIndex = 2;VertexIndex < Node.NumVertices;VertexIndex++)
				{
					IndexBuffer.AddFace(
						Model->Verts(Node.iVertPool).pVertex,
						Model->Verts(Node.iVertPool + VertexIndex - 1).pVertex,
						Model->Verts(Node.iVertPool + VertexIndex).pVertex
						);
					IndexBuffer.AddFace(
						FirstExtrudedVertex + Model->Verts(Node.iVertPool).pVertex,
						FirstExtrudedVertex + Model->Verts(Node.iVertPool + VertexIndex).pVertex,
						FirstExtrudedVertex + Model->Verts(Node.iVertPool + VertexIndex - 1).pVertex
						);
				}
			}
		}
	}

	for(INT EdgeIndex = 0;EdgeIndex < Edges.Num();EdgeIndex++)
	{
		FMeshEdge& Edge = Model->Edges(Edges(EdgeIndex));
		FBspNode& Node0 = Model->Nodes(Edge.Faces[0]);
		if(Node0.ComponentIndex == ComponentIndex)
		{
			if(Edge.Faces[1] != INDEX_NONE && Model->Nodes(Edge.Faces[1]).ComponentIndex == ComponentIndex)
			{
				if(IsNegativeFloat(PlaneDots[Node0.ComponentNodeIndex]) == IsNegativeFloat(PlaneDots[Model->Nodes(Edge.Faces[1]).ComponentNodeIndex]))
					continue;
			}
			else if(!IsNegativeFloat(PlaneDots[Node0.ComponentNodeIndex]))
				continue;

			IndexBuffer.AddEdge(
				IsNegativeFloat(PlaneDots[Node0.ComponentNodeIndex]) ? Edge.Vertices[0] : Edge.Vertices[1],
				IsNegativeFloat(PlaneDots[Node0.ComponentNodeIndex]) ? Edge.Vertices[1] : Edge.Vertices[0],
				FirstExtrudedVertex
				);
		}
		else if(Edge.Faces[1] != INDEX_NONE)
		{
			FBspNode& Node1 = Model->Nodes(Edge.Faces[1]);
			if(Node1.ComponentIndex == ComponentIndex)
			{
				if(!IsNegativeFloat(PlaneDots[Node1.ComponentNodeIndex]))
					continue;

				IndexBuffer.AddEdge(
					IsNegativeFloat(PlaneDots[Node1.ComponentNodeIndex]) ? Edge.Vertices[1] : Edge.Vertices[0],
					IsNegativeFloat(PlaneDots[Node1.ComponentNodeIndex]) ? Edge.Vertices[0] : Edge.Vertices[1],
					FirstExtrudedVertex
					);
			}
		}
	}

	delete [] PlaneDots;

    IndexBuffer.Indices.Shrink();
}


/**
 * A dynamic model index buffer.
 */
class FModelDynamicIndexBuffer : public FIndexBuffer
{
public:

	FModelDynamicIndexBuffer(UINT InTotalIndices):
		FirstIndex(0),
		NextIndex(0),
		TotalIndices(InTotalIndices)
	{
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(UINT),TotalIndices * sizeof(UINT),NULL,RUF_Static);
		Lock();
	}

	~FModelDynamicIndexBuffer()
	{
		IndexBufferRHI.SafeRelease();
	}

	void AddNode(const UModel* Model,UINT NodeIndex,INT ZoneIndex)
	{
		const FBspNode& Node = Model->Nodes(NodeIndex);
		const FBspSurf& Surf = Model->Surfs(Node.iSurf);

		for(UINT BackFace = 0;BackFace < (UINT)((Surf.PolyFlags & PF_TwoSided) ? 2 : 1);BackFace++)
		{
			if(Node.iZone[1-BackFace] == ZoneIndex || ZoneIndex == INDEX_NONE)
			{
				for(INT VertexIndex = 2;VertexIndex < Node.NumVertices;VertexIndex++)
				{
					*Indices++ = Node.iVertexIndex + Node.NumVertices * BackFace;
					*Indices++ = Node.iVertexIndex + Node.NumVertices * BackFace + VertexIndex;
					*Indices++ = Node.iVertexIndex + Node.NumVertices * BackFace + VertexIndex - 1;
					NextIndex += 3;
				}
				MinVertexIndex = Min(Node.iVertexIndex + Node.NumVertices * BackFace,MinVertexIndex);
				MaxVertexIndex = Max(Node.iVertexIndex + Node.NumVertices * BackFace + Node.NumVertices - 1,MaxVertexIndex);
			}
		}
	}

	void Draw(
		const UModelComponent* Component,
		BYTE InDepthPriorityGroup,
		const FMaterialRenderProxy* MaterialRenderProxy,
		const FLightCacheInterface* LCI,
		FPrimitiveDrawInterface* PDI,
		class FPrimitiveSceneInfo *PrimitiveSceneInfo,
		const FLinearColor& LevelColor,
		const FLinearColor& PropertyColor
		)
	{
		if(NextIndex > FirstIndex)
		{
			RHIUnlockIndexBuffer(IndexBufferRHI);

			FMeshElement MeshElement;
			MeshElement.IndexBuffer = this;
			MeshElement.VertexFactory = &Component->GetModel()->VertexFactory;
			MeshElement.MaterialRenderProxy = MaterialRenderProxy;
			MeshElement.LCI = LCI;
			MeshElement.LocalToWorld = Component->LocalToWorld;
			MeshElement.WorldToLocal = Component->LocalToWorld.Inverse();
			MeshElement.FirstIndex = FirstIndex;
			MeshElement.NumPrimitives = (NextIndex - FirstIndex) / 3;
			MeshElement.MinVertexIndex = MinVertexIndex;
			MeshElement.MaxVertexIndex = MaxVertexIndex;
			MeshElement.Type = PT_TriangleList;
			MeshElement.DepthPriorityGroup = InDepthPriorityGroup;
			DrawRichMesh(PDI,MeshElement,FLinearColor::White, LevelColor, PropertyColor, PrimitiveSceneInfo,FALSE);

			FirstIndex = NextIndex;
			Lock();
		}
	}

private:
	UINT FirstIndex;
	UINT NextIndex;
	UINT MinVertexIndex;
	UINT MaxVertexIndex;
	UINT TotalIndices;
	UINT* Indices;
	
	void Lock()
	{
		if(NextIndex < TotalIndices)
		{
			Indices = (UINT*)RHILockIndexBuffer(IndexBufferRHI,FirstIndex * sizeof(UINT),TotalIndices * sizeof(UINT) - FirstIndex * sizeof(UINT));
			MaxVertexIndex = 0;
			MinVertexIndex = MAXINT;
		}
	}
};

IMPLEMENT_COMPARE_CONSTPOINTER( FDecalInteraction, UnModelRender,
{
	return (A->DecalState.SortOrder <= B->DecalState.SortOrder) ? -1 : 1;
} );

/**
 * A model component scene proxy.
 */
class FModelSceneProxy : public FPrimitiveSceneProxy
{
public:

	FModelSceneProxy(const UModelComponent* InComponent):
		FPrimitiveSceneProxy(InComponent),
		Component(InComponent),
		LevelColor(255,255,255),
		PropertyColor(255,255,255)
	{
		const TIndirectArray<FModelElement>& SourceElements = Component->GetElements();

		Elements.Empty(SourceElements.Num());
		for(INT ElementIndex = 0;ElementIndex < SourceElements.Num();ElementIndex++)
		{
			const FModelElement& SourceElement = SourceElements(ElementIndex);
			FElementInfo* Element = new(Elements) FElementInfo(SourceElement);
			MaterialViewRelevance |= Element->GetMaterial()->GetViewRelevance();
		}

		// Try to find a color for level coloration.
		UObject* ModelOuter = InComponent->GetModel()->GetOuter();
		ULevel* Level = Cast<ULevel>( ModelOuter );
		if ( Level )
		{
			ULevelStreaming* LevelStreaming = FLevelUtils::FindStreamingLevel( Level );
			if ( LevelStreaming )
			{
				LevelColor = LevelStreaming->DrawColor;
			}
		}

		// Get a color for property coloration.
		GEngine->GetPropertyColorationColor( (UObject*)InComponent, PropertyColor );
	}

	virtual HHitProxy* CreateHitProxies(const UPrimitiveComponent*,TArray<TRefCountPtr<HHitProxy> >& OutHitProxies)
	{
		HHitProxy* ModelHitProxy = new HModel(Component->GetModel());
		OutHitProxies.AddItem(ModelHitProxy);
		return ModelHitProxy;
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
	virtual void DrawDynamicDecalElements(
		FPrimitiveDrawInterface* PDI,
		const FSceneView* View,
		UINT InDepthPriorityGroup,
		UBOOL bDynamicLightingPass,
		UBOOL bTranslucentReceiverPass
		)
	{
		SCOPE_CYCLE_COUNTER(STAT_DecalRenderDynamicBSPTime);

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
			Sort<USE_COMPARE_CONSTPOINTER(FDecalInteraction,UnModelRender)>( DPGDecals.GetTypedData(), DPGDecals.Num() );
		}
		for ( INT DecalIndex = 0 ; DecalIndex < DPGDecals.Num() ; ++DecalIndex )
		{
			const FDecalInteraction* Decal	= DPGDecals(DecalIndex);
			FDecalRenderData* RenderData = Decal->RenderData;

			if( RenderData->DecalVertexFactory &&
				RenderData->NumTriangles > 0 )
			{
				const FDecalState& DecalState	= Decal->DecalState;
				const FBox& DecalBoundingBox = Decal->DecalState.Bounds;

				UBOOL bIsDecalVisible = TRUE;

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

				if( bIsDecalVisible )
				{
					FMeshElement MeshElement;
					MeshElement.IndexBuffer = RenderData->bUsesIndexResources ? &RenderData->IndexBuffer : NULL;
					MeshElement.VertexFactory = RenderData->DecalVertexFactory->CastToFVertexFactory();
					MeshElement.MaterialRenderProxy = DecalState.DecalMaterial->GetRenderProxy(FALSE);

					MeshElement.LCI = NULL;
					if( DecalState.bDecalMaterialHasStaticLightingUsage )
					{
						if( Elements.IsValidIndex(RenderData->Data) )
						{
							// BSP uses the render data's Data field to store the model element index.
							MeshElement.LCI = &Elements(RenderData->Data);
						}
						else
						{
							debugfSuppressed( TEXT("Missing Decal LCI ComponentElementIndex=%d Decal=%s"), 
								RenderData->Data,
								DecalState.DecalComponent && DecalState.DecalComponent->GetOwner() 
								? *DecalState.DecalComponent->GetOwner()->GetName() : TEXT("None") );
						}
					}

					MeshElement.LocalToWorld = Component->LocalToWorld;
					MeshElement.WorldToLocal = Component->LocalToWorld.Inverse();
					MeshElement.FirstIndex = 0;
					MeshElement.NumPrimitives = RenderData->NumTriangles;
					MeshElement.MinVertexIndex = 0;
					MeshElement.MaxVertexIndex = RenderData->DecalVertexBuffer.GetNumVertices()-1;
					MeshElement.CastShadow = FALSE;
					MeshElement.DepthBias = DecalState.DepthBias;
					MeshElement.SlopeScaleDepthBias = DecalState.SlopeScaleDepthBias;
					MeshElement.Type = PT_TriangleList;
					MeshElement.DepthPriorityGroup = InDepthPriorityGroup;
					MeshElement.bIsDecal = TRUE;

					// set decal vertex factory parameters (in local space)
					const FDecalLocalSpaceInfo DecalLocal(&DecalState,MeshElement.LocalToWorld,MeshElement.WorldToLocal);
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
					if(RenderData)
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
	virtual void DrawStaticDecalElements(FStaticPrimitiveDrawInterface* PDI,const FDecalInteraction& DecalInteraction)
	{
		if( !HasViewDependentDPG() &&
			// only add static decal batches for decals projecting on opaque receivers or if the decal is opaque itself
			(MaterialViewRelevance.bOpaque || DecalInteraction.DecalState.MaterialViewRelevance.bOpaque) &&
			DecalInteraction.RenderData->DecalVertexFactory &&
			DecalInteraction.RenderData->NumTriangles > 0 )
		{
			const FDecalState& DecalState = DecalInteraction.DecalState;
			FDecalRenderData* RenderData = DecalInteraction.RenderData;

			FMeshElement MeshElement;
			MeshElement.IndexBuffer = RenderData->bUsesIndexResources ? &RenderData->IndexBuffer : NULL;
			MeshElement.VertexFactory = RenderData->DecalVertexFactory->CastToFVertexFactory();
			MeshElement.MaterialRenderProxy = DecalState.DecalMaterial->GetRenderProxy(FALSE);
			MeshElement.LocalToWorld = Component->LocalToWorld;
			MeshElement.WorldToLocal = Component->LocalToWorld.Inverse();
			MeshElement.FirstIndex = 0;
			MeshElement.NumPrimitives = RenderData->NumTriangles;
			MeshElement.MinVertexIndex = 0;
			MeshElement.MaxVertexIndex = RenderData->DecalVertexBuffer.GetNumVertices()-1;
			MeshElement.CastShadow = FALSE;
			MeshElement.DepthBias = DecalState.DepthBias;
			MeshElement.SlopeScaleDepthBias = DecalState.SlopeScaleDepthBias;
			MeshElement.Type = PT_TriangleList;
			MeshElement.DepthPriorityGroup = GetStaticDepthPriorityGroup();
			MeshElement.bIsDecal = TRUE;

			// This makes the decal render using a scissor rect (for performance reasons).
			MeshElement.DecalState = &DecalState;

			// set decal vertex factory parameters (in local space)
			const FDecalLocalSpaceInfo DecalLocal(&DecalState,MeshElement.LocalToWorld,MeshElement.WorldToLocal);
			RenderData->DecalVertexFactory->SetDecalMatrix(DecalLocal.TextureTransform);
			RenderData->DecalVertexFactory->SetDecalLocation(DecalLocal.LocalLocation);
			RenderData->DecalVertexFactory->SetDecalOffset(FVector2D(DecalState.OffsetX, DecalState.OffsetY));
			RenderData->DecalVertexFactory->SetDecalLocalBinormal(DecalLocal.LocalBinormal);
			RenderData->DecalVertexFactory->SetDecalLocalTangent(DecalLocal.LocalTangent);
			RenderData->DecalVertexFactory->SetDecalLocalNormal(DecalLocal.LocalNormal);

			MeshElement.LCI = NULL;
			if( DecalState.bDecalMaterialHasStaticLightingUsage )
			{
				if( Elements.IsValidIndex(RenderData->Data) )
				{
					// BSP uses the render data's Data field to store the model element index.
					MeshElement.LCI = &Elements(RenderData->Data);
				}
				else
				{
					debugfSuppressed( TEXT("Missing Decal LCI ComponentElementIndex=%d Decal=%s"), 
						RenderData->Data,
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
	virtual void AddDecalInteraction_RenderingThread(const FDecalInteraction& DecalInteraction)
	{
		FPrimitiveSceneProxy::AddDecalInteraction_RenderingThread( DecalInteraction );
		FDecalInteraction& NewInteraction = *Decals(Decals.Num()-1);

		// add the static mesh element for this decal interaction
		NewInteraction.CreateDecalStaticMesh(PrimitiveSceneInfo);
	}

	/** 
	* Draw the scene proxy as a dynamic element
	*
	* @param	PDI - draw interface to render to
	* @param	View - current view
	* @param	DPGIndex - current depth priority 
	* @param	Flags - optional set of flags from EDrawDynamicElementFlags
	*/
	virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex,DWORD Flags)
	{
		const UBOOL bDynamicBSPTriangles = (View->Family->ShowFlags & SHOW_Selection) || IsRichView(View) || IsCollisionView(View);
		
		// Determine the DPG the primitive should be drawn in for this view.
		if (GetViewRelevance(View).GetDPG(DPGIndex) == FALSE)
		{
			return;
		}

		// Draw the BSP triangles.
		if((View->Family->ShowFlags & SHOW_BSPTriangles) && (View->Family->ShowFlags & SHOW_BSP) && bDynamicBSPTriangles)
		{
			FLinearColor UtilColor = LevelColor;
			if( IsCollisionView(View) )
			{
				UtilColor = GEngine->C_BSPCollision;
			}

			if(View->Family->ShowFlags & SHOW_Selection)
			{
				UINT TotalIndices = 0;
				for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
				{
					const FModelElement& ModelElement = Component->GetElements()(ElementIndex);
					TotalIndices += ModelElement.NumTriangles * 3;
				}

				if(TotalIndices > 0)
				{
					FModelDynamicIndexBuffer IndexBuffer(TotalIndices);
					for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
					{
						const FModelElement& ModelElement = Component->GetElements()(ElementIndex);
						if(ModelElement.NumTriangles > 0)
						{
							const FElementInfo& ProxyElementInfo = Elements(ElementIndex);
							for(UINT bSelected = 0;bSelected < 2;bSelected++)
							{
								for(INT NodeIndex = 0;NodeIndex < ModelElement.Nodes.Num();NodeIndex++)
								{
									FBspNode& Node = Component->GetModel()->Nodes(ModelElement.Nodes(NodeIndex));
									FBspSurf& Surf = Component->GetModel()->Surfs(Node.iSurf);

									// Don't draw portal polygons.
									if(Surf.PolyFlags & PF_Portal)
										continue;

									if(((Surf.PolyFlags & PF_Selected) ? TRUE : FALSE) == bSelected)
									{
										IndexBuffer.AddNode(Component->GetModel(),ModelElement.Nodes(NodeIndex),Component->GetZoneIndex());
									}
								}
								IndexBuffer.Draw(
									Component,
									DPGIndex,
									ProxyElementInfo.GetMaterial()->GetRenderProxy(bSelected),
									&ProxyElementInfo,
									PDI,
									PrimitiveSceneInfo,
									UtilColor,
									PropertyColor
									);
							}
						}
					}
				}
			}
			else
			{
				for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
				{
					const FModelElement& ModelElement = Component->GetElements()(ElementIndex);
					if(ModelElement.NumTriangles > 0)
					{
						FMeshElement MeshElement;
						MeshElement.IndexBuffer = ModelElement.IndexBuffer;
						MeshElement.VertexFactory = &Component->GetModel()->VertexFactory;
						MeshElement.MaterialRenderProxy = Elements(ElementIndex).GetMaterial()->GetRenderProxy(FALSE);
						MeshElement.LCI = &Elements(ElementIndex);
						MeshElement.LocalToWorld = Component->LocalToWorld;
						MeshElement.WorldToLocal = Component->LocalToWorld.Inverse();
						MeshElement.FirstIndex = ModelElement.FirstIndex;
						MeshElement.NumPrimitives = ModelElement.NumTriangles;
						MeshElement.MinVertexIndex = ModelElement.MinVertexIndex;
						MeshElement.MaxVertexIndex = ModelElement.MaxVertexIndex;
						MeshElement.Type = PT_TriangleList;
						MeshElement.DepthPriorityGroup = DPGIndex;
						DrawRichMesh(PDI,MeshElement,FLinearColor::White,UtilColor,FLinearColor::White,PrimitiveSceneInfo,FALSE);
					}
				}
			}
		}
	}

	virtual void DrawStaticElements(FStaticPrimitiveDrawInterface* PDI)
	{
		if(!HasViewDependentDPG())
		{
			// Determine the DPG the primitive should be drawn in.
			BYTE PrimitiveDPG = GetStaticDepthPriorityGroup();

			for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
			{
				const FModelElement& ModelElement = Component->GetElements()(ElementIndex);
				if(ModelElement.NumTriangles > 0)
				{
					FMeshElement MeshElement;
					MeshElement.IndexBuffer = ModelElement.IndexBuffer;
					MeshElement.VertexFactory = &Component->GetModel()->VertexFactory;
					MeshElement.MaterialRenderProxy = Elements(ElementIndex).GetMaterial()->GetRenderProxy(FALSE);
					MeshElement.LCI = &Elements(ElementIndex);
					MeshElement.LocalToWorld = Component->LocalToWorld;
					MeshElement.WorldToLocal = Component->LocalToWorld.Inverse();
					MeshElement.FirstIndex = ModelElement.FirstIndex;
					MeshElement.NumPrimitives = ModelElement.NumTriangles;
					MeshElement.MinVertexIndex = ModelElement.MinVertexIndex;
					MeshElement.MaxVertexIndex = ModelElement.MaxVertexIndex;
					MeshElement.Type = PT_TriangleList;
					MeshElement.DepthPriorityGroup = PrimitiveDPG;
					PDI->DrawMesh(MeshElement,0,FLT_MAX);
				}
			}
		}
	}

	/**
	 * Draws the primitive's shadow volumes.  This is called from the rendering thread,
	 * in the FSceneRenderer::RenderLights phase.
	 * @param SVDI - The interface which performs the actual rendering of a shadow volume.
	 * @param View - The view which is being rendered.
	 * @param Light - The light for which shadows should be drawn.
	 * @param DPGIndex - The depth priority group the light is being drawn for.
	 */
	virtual void DrawShadowVolumes(FShadowVolumeDrawInterface* SVDI, const FSceneView* View, const FLightSceneInfo* Light,UINT DPGIndex)
	{
		checkSlow(UEngine::ShadowVolumesAllowed());

		// Determine the DPG the primitive should be drawn in for this view.
		if (GetViewRelevance(View).GetDPG(DPGIndex) == FALSE)
		{
			return;
		}

		SCOPE_CYCLE_COUNTER(STAT_ShadowVolumeRenderTime);

		// Check for the shadow volume in the cache.
		const FShadowVolumeCache::FCachedShadowVolume* CachedShadowVolume = CachedShadowVolumes.GetShadowVolume(Light);
		if (!CachedShadowVolume)
		{
			SCOPE_CYCLE_COUNTER(STAT_ShadowExtrusionTime);

			FShadowIndexBuffer IndexBuffer;
			Component->BuildShadowVolume( IndexBuffer, Light );

			// Add the new shadow volume to the cache.
			CachedShadowVolume = CachedShadowVolumes.AddShadowVolume(Light,IndexBuffer);
		}

		// Draw the cached shadow volume.
		if(CachedShadowVolume->NumTriangles)
		{
			INC_DWORD_STAT_BY(STAT_ShadowVolumeTriangles, CachedShadowVolume->NumTriangles);
			INT NumVertices = Component->GetModel()->Points.Num();
			const FLocalShadowVertexFactory &ShadowVertexFactory = Component->GetModel()->ShadowVertexBuffer.GetVertexFactory();
			SVDI->DrawShadowVolume( CachedShadowVolume->IndexBufferRHI, ShadowVertexFactory, Component->LocalToWorld, 0, CachedShadowVolume->NumTriangles, 0, NumVertices * 2 - 1 );
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View)
	{
		FPrimitiveViewRelevance Result;
		if((View->Family->ShowFlags & SHOW_BSPTriangles) && (View->Family->ShowFlags & SHOW_BSP))
		{
			if(IsShown(View))
			{
				if((View->Family->ShowFlags & SHOW_Selection) || IsRichView(View) || IsCollisionView(View) || HasViewDependentDPG())
				{
					Result.bDynamicRelevance = TRUE;
				}
				else
				{
					Result.bStaticRelevance = TRUE;
				}
				Result.SetDPG(GetDepthPriorityGroup(View),TRUE);
				Result.bDecalStaticRelevance = HasRelevantStaticDecals(View);
				Result.bDecalDynamicRelevance = HasRelevantDynamicDecals(View);
			}
			if (IsShadowCast(View))
			{
				Result.bShadowRelevance = TRUE;
			}
			MaterialViewRelevance.SetPrimitiveViewRelevance(Result);
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
	virtual void GetLightRelevance(FLightSceneInfo* LightSceneInfo, UBOOL& bDynamic, UBOOL& bRelevant, UBOOL& bLightMapped)
	{
		// Attach the light to the primitive's static meshes.
		bDynamic = TRUE;
		bRelevant = FALSE;
		bLightMapped = TRUE;

		if (Elements.Num() > 0)
		{
			for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
			{
				FElementInfo* LCI = &Elements(ElementIndex);
				if (LCI)
				{
					ELightInteractionType InteractionType = LCI->GetInteraction(LightSceneInfo).GetType();
					if(InteractionType != LIT_CachedIrrelevant)
					{
						bRelevant = TRUE;
						if(InteractionType != LIT_CachedLightMap)
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
		}
		else
		{
			bRelevant = TRUE;
			bLightMapped = FALSE;
		}
	}

	/**
	 * Called by the rendering thread to notify the proxy when a light is no longer
	 * associated with the proxy, so that it can clean up any cached resources.
	 * @param Light - The light to be removed.
	 */
	virtual void OnDetachLight(const FLightSceneInfo* Light)
	{
		CachedShadowVolumes.RemoveShadowVolume(Light);
	}
	virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocOther ); }
	virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	DWORD GetAllocatedSize( void ) const 
	{ 
		DWORD AdditionalSize = FPrimitiveSceneProxy::GetAllocatedSize();

		AdditionalSize += Elements.GetAllocatedSize();

		return( AdditionalSize ); 
	}

private:

	/** Cached shadow volumes. */
	FShadowVolumeCache CachedShadowVolumes;

	const UModelComponent* Component;

	class FElementInfo: public FLightCacheInterface
	{
	public:

		/** Initialization constructor. */
		FElementInfo(const FModelElement& InModelElement)
		:	ModelElement(InModelElement)
		,	Bounds(ModelElement.BoundingBox)
		{
			const UBOOL bHasStaticLighting = ModelElement.LightMap != NULL || ModelElement.ShadowMaps.Num();

			// Determine the material applied to the model element.
			Material = ModelElement.Material;

			// If there isn't an applied material, or if we need static lighting and it doesn't support it, fall back to the default material.
			if(!ModelElement.Material || (bHasStaticLighting && !ModelElement.Material->CheckMaterialUsage(MATUSAGE_StaticLighting)))
			{
				Material = GEngine->DefaultMaterial;
			}

			LightMap = ModelElement.LightMap;
		}
		
		// FLightCacheInterface.
		virtual FLightInteraction GetInteraction(const FLightSceneInfo* LightSceneInfo) const
		{
			if( LightSceneInfo->bStaticShadowing )
			{
				if( ModelElement.IrrelevantLights.ContainsItem( LightSceneInfo->LightGuid ) )
				{
					return FLightInteraction::Irrelevant();
				}

				if( LightMap && LightMap->ContainsLight( LightSceneInfo->LightmapGuid ) )
				{
					return FLightInteraction::LightMap();
				}

				for( INT LightIndex=0; LightIndex<ModelElement.ShadowMaps.Num(); LightIndex++ )
				{
					UShadowMap2D* ShadowMap = ModelElement.ShadowMaps(LightIndex);
					if( ShadowMap && ShadowMap->IsValid() && ShadowMap->GetLightGuid() == LightSceneInfo->LightGuid )
					{
						return	FLightInteraction::ShadowMap2D(
									ShadowMap->GetTexture(),
									ShadowMap->GetCoordinateScale(),
									ShadowMap->GetCoordinateBias() );
					}
				}
			}

			// Cull the uncached light against the bounding box of the element.
			if(	LightSceneInfo->AffectsBounds(Bounds) )
			{
				return FLightInteraction::Uncached();
			}
			else
			{
				return FLightInteraction::Irrelevant();
			}
		}
		virtual FLightMapInteraction GetLightMapInteraction() const
		{
			return LightMap ? LightMap->GetInteraction() : FLightMapInteraction();
		}

		// Accessors.
		UMaterialInterface* GetMaterial() const { return Material; }

	private:

		/** The element's material. */
		UMaterialInterface* Material;

		/** Associated model element. */
		const FModelElement& ModelElement;

		/** The light-map used by the element. */
		const FLightMap* LightMap;

		/** The element's bounding volume. */
		FBoxSphereBounds Bounds;
	};

	TArray<FElementInfo> Elements;

	FColor LevelColor;
	FColor PropertyColor;

	FMaterialViewRelevance MaterialViewRelevance;
};

FPrimitiveSceneProxy* UModelComponent::CreateSceneProxy()
{
	return ::new FModelSceneProxy(this);
}

UBOOL UModelComponent::ShouldRecreateProxyOnUpdateTransform() const
{
	return TRUE;
}

void UModelComponent::UpdateBounds()
{
	if(Model)
	{
		FBox	BoundingBox(0);
		for(INT NodeIndex = 0;NodeIndex < Nodes.Num();NodeIndex++)
		{
			FBspNode& Node = Model->Nodes(Nodes(NodeIndex));
			for(INT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
			{
				BoundingBox += Model->Points(Model->Verts(Node.iVertPool + VertexIndex).pVertex);
			}
		}
		Bounds = FBoxSphereBounds(BoundingBox.TransformBy(LocalToWorld));
	}
	else
	{
		Super::UpdateBounds();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Decals on BSP.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////

FDecalRenderData* UModelComponent::GenerateDecalRenderData(FDecalState* Decal) const
{
	SCOPE_CYCLE_COUNTER(STAT_DecalBSPAttachTime);

	// Do nothing if the specified decal doesn't project on BSP.
	if ( !Decal->bProjectOnBSP )
	{
		return NULL;
	}

	// Return value.
	FDecalRenderData* DecalRenderData = NULL;

	// Temporaries. Moved out of loop to avoid reallocations for Poly case and costly recomputation.
	FDecalPoly Poly;
	const FDecalLocalSpaceInfoClip DecalInfo( Decal, LocalToWorld, LocalToWorld.Inverse() );

	// Iterate over all potential nodes.
	for( INT HitNodeIndexIndex=0; HitNodeIndexIndex<Decal->HitNodeIndices.Num(); HitNodeIndexIndex++ )
	{
		INT HitNodeIndex = Decal->HitNodeIndices(HitNodeIndexIndex);

		const FBspNode& Node	= Model->Nodes( HitNodeIndex );
		const ULevel*	Level	= CastChecked<ULevel>(GetOuter());

		// Nothing to do if requested node is not part of this model component
		if( Level->ModelComponents( Node.ComponentIndex ) != this )
		{
			continue;
		}

		// Don't attach to invisible or portal surfaces.
		const FBspSurf& Surf = Model->Surfs( Node.iSurf );
		if ( (Surf.PolyFlags & PF_Portal) || (Surf.PolyFlags & PF_Invisible) )
		{
			continue;
		}

		INT ComponentElementIndex = Node.ComponentElementIndex;

		// BSP built before the addition of ComponentElementIndex needs some fixups.
		if( !Elements.IsValidIndex(ComponentElementIndex) )
		{
			for( INT ElementIndex=0; ElementIndex<Elements.Num(); ElementIndex++ )
			{
				if( Elements(ElementIndex).Nodes.FindItemIndex( HitNodeIndex ) != INDEX_NONE )
				{
					ComponentElementIndex = ElementIndex;
					break;
				}
			}
		}
		check( ComponentElementIndex != INDEX_NONE );
	
		// Don't perform software clipping for dynamic decals or when requested.
		const UBOOL bNoClip = Decal->bNoClip; //!Decal->DecalComponent->bStaticDecal || Decal->bNoClip;

		// Discard if the polygon faces away from the decal.
		// The dot product is inverted because BSP polygon winding is clockwise.
		const FLOAT Dot = -(DecalInfo.LocalLookVector | Node.Plane);
		const UBOOL bIsFrontFacing = Decal->bFlipBackfaceDirection ? -Dot > Decal->DecalComponent->BackfaceAngle : Dot > Decal->DecalComponent->BackfaceAngle;

		// Even if backface culling is disabled, reject triangles that view the decal at grazing angles.
		if ( bIsFrontFacing || ( Decal->bProjectOnBackfaces && Abs( Dot ) > Decal->DecalComponent->BackfaceAngle ) )
		{
			// Copy off the vertices into a temporary poly for clipping.
			Poly.Init();
			const INT FirstVertexIndex = Node.iVertPool;
			for ( INT VertexIndex = 0 ; VertexIndex < Node.NumVertices ; ++VertexIndex )
			{
				const FVert& ModelVert = Model->Verts( FirstVertexIndex + VertexIndex );
				new(Poly.Vertices) FVector( Model->Points( ModelVert.pVertex ) );
				new(Poly.ShadowTexCoords) FVector2D( ModelVert.ShadowTexCoord );
				Poly.Indices.AddItem( FirstVertexIndex + VertexIndex );
			}

			// Clip against the decal. Don't need to check in the 
			const UBOOL bClipPassed = bNoClip ? TRUE : Poly.ClipAgainstConvex( DecalInfo.Convex );
			if ( bClipPassed )
			{
				// Allocate a FDecalRenderData object if we haven't already.
				if ( !DecalRenderData )
				{
					DecalRenderData = new FDecalRenderData( NULL, TRUE, TRUE );
					// Store the model element index.
					DecalRenderData->Data = ComponentElementIndex;
				}

				// surface normal of bsp triangle
				const FVector TangentZ(Node.Plane);
				const FVector TangentX(Model->Vectors(Surf.vTextureU).SafeNormal());
				const FVector TangentY(Model->Vectors(Surf.vTextureV).SafeNormal());
				
				// store sign of determinant in normal w component
				FPackedNormal DecalPackedTangentZ(TangentZ);
				FPackedNormal DecalPackedTangentX(TangentX);
				DecalPackedTangentZ.Vector.W = GetBasisDeterminantSign(TangentX,TangentY,TangentZ) < 0 ? 0 : 255;

				const DWORD DecalFirstVertexIndex = DecalRenderData->GetNumVertices();
				checkSlow( Poly.Vertices.Num() == Poly.ShadowTexCoords.Num() );
				for ( INT i = 0 ; i < Poly.Vertices.Num() ; ++i )
				{
					// Store the decal vertex.
					new(DecalRenderData->Vertices) FDecalVertex(Poly.Vertices(i),
																DecalPackedTangentX,
																DecalPackedTangentZ,
																Poly.ShadowTexCoords( i ));
				}

				// Triangulate the polygon and add indices to the index buffer				
				for ( INT i = 0 ; i < Poly.Vertices.Num() - 2 ; ++i )
				{
					DecalRenderData->AddIndex( DecalFirstVertexIndex+0 );
					DecalRenderData->AddIndex( DecalFirstVertexIndex+i+2 );
					DecalRenderData->AddIndex( DecalFirstVertexIndex+i+1 );
				}
			}
		}
	}

	// Finalize the data.
	if( DecalRenderData )
	{
		DecalRenderData->NumTriangles = DecalRenderData->GetNumIndices()/3;
		// set the blending interval - clamp to 
		DecalRenderData->DecalBlendRange = Decal->DecalComponent->CalcDecalDotProductBlendRange();
	}

	return DecalRenderData;
}
