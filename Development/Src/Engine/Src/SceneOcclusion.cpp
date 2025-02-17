/*=============================================================================
	SceneRendering.cpp: Scene rendering.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"

#define NUM_CUBE_VERTICES 36

/**
* A vertex shader for rendering a texture on a simple element.
*/
template<UINT VerticesPerPrimitive>
class FOcclusionQueryVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FOcclusionQueryVertexShader,Global);
public:
	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	FOcclusionQueryVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
	}
	FOcclusionQueryVertexShader() {}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		FVertexFactory::ModifyCompilationEnvironment(Platform, OutEnvironment);
		OutEnvironment.Definitions.Set(TEXT("VERTICES_PER_PRIMITIVE_INSTANCE"),*FString::Printf(TEXT("%u"),VerticesPerPrimitive));
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
#if PS3
		//@hack - compiler bug? optimized version crashes during FShader::Serialize call
		static INT RemoveMe=0;	RemoveMe=1;
#endif
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		return bShaderHasOutdatedParameters;
	}
};

// default, non-instanced shader implementation
IMPLEMENT_SHADER_TYPE(template<>,FOcclusionQueryVertexShader<0>,TEXT("OcclusionQueryVertexShader"),TEXT("Main"),SF_Vertex,0,0);
// cube instances shader implementation
IMPLEMENT_SHADER_TYPE(template<>,FOcclusionQueryVertexShader<NUM_CUBE_VERTICES>,TEXT("OcclusionQueryVertexShader"),TEXT("Main"),SF_Vertex,0,0);

/**
* The occlusion query position-only vertex declaration resource type.
*/
class FOcclusionQueryPosOnlyVertexDeclaration : public FRenderResource
{
public:

	FVertexDeclarationRHIRef VertexDeclarationRHI;

	/** Destructor. */
	virtual ~FOcclusionQueryPosOnlyVertexDeclaration() {}

	virtual void InitRHI()
	{
		FVertexDeclarationElementList Elements;
		Elements.AddItem(FVertexElement(0,0,VET_Float3,VEU_Position,0));
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI()
	{
		VertexDeclarationRHI.SafeRelease();
	}
};
/** The occlusion query position-only vertex declaration. */
TGlobalResource<FOcclusionQueryPosOnlyVertexDeclaration> GOcclusionQueryPosOnlyVertexDeclaration;

#if XBOX
/**
* The occlusion query instanced primitives vertex declaration resource type.
*/
class FOcclusionQueryInstancingVertexDeclaration : public FRenderResource
{
public:

	FVertexDeclarationRHIRef VertexDeclarationRHI;

	/** Destructor. */
	virtual ~FOcclusionQueryInstancingVertexDeclaration() {}

	virtual void InitRHI()
	{
		FVertexDeclarationElementList Elements;
		Elements.AddItem(FVertexElement(0,0,VET_Float3,VEU_Position,0));
		Elements.AddItem(FVertexElement(1,STRUCT_OFFSET(FOcclusionPrimitive,Origin),VET_Float3,VEU_Position,1));
		Elements.AddItem(FVertexElement(1,STRUCT_OFFSET(FOcclusionPrimitive,Extent),VET_Float3,VEU_Position,2));
		Elements.AddItem(FVertexElement(2,0,VET_UByte4,VEU_Position,3));
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI()
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

/** 
* Simple vertex buffer class for an occlusion query box
*/
class FOcclusionQueryBoxVertexBuffer : public FVertexBuffer
{
public:
	/** 
	* Initialize the RHI for this rendering resource 
	*/
	virtual void InitRHI()
	{
		// create a static vertex buffer
		VertexBufferRHI = RHICreateVertexBuffer(8 * sizeof(FVector), NULL, RUF_Static);
		FVector* Vertices = (FVector*)RHILockVertexBuffer(VertexBufferRHI, 0, 8 * sizeof(FVector), FALSE);
		Vertices[0] = FVector(-1, -1, -1);
		Vertices[1] = FVector(-1, -1, +1);
		Vertices[2] = FVector(-1, +1, -1);
		Vertices[3] = FVector(-1, +1, +1);
		Vertices[4] = FVector(+1, -1, -1);
		Vertices[5] = FVector(+1, -1, +1);
		Vertices[6] = FVector(+1, +1, -1);
		Vertices[7] = FVector(+1, +1, +1);
		RHIUnlockVertexBuffer(VertexBufferRHI);
	}
};

/** 
* Simple vertex buffer class for the occlusion query primitives
*/
class FOcclusionQueryPrimitivesVertexBuffer : public FVertexBuffer
{
public:
	FOcclusionQueryPrimitivesVertexBuffer() : Primitives(NULL), Size(0) {}

	/** 
	* Initialize the dynamic RHI for this rendering resource 
	*/
	virtual void InitDynamicRHI()
	{
		if(Size)
		{
			// create a vertex buffer
			VertexBufferRHI = RHICreateVertexBuffer(Size, NULL, RUF_Dynamic);
			if(Primitives)
			{
				// Lock the buffer.
				void* Buffer = RHILockVertexBuffer(VertexBufferRHI,0,Size,FALSE);
				// copy over the primitive data
				appMemcpy(Buffer, (void*)Primitives, Size);
				// Unlock the buffer.
				RHIUnlockVertexBuffer(VertexBufferRHI);
			}
		}
	}

	/** 
	* Release the dynamic RHI for this rendering resource 
	*/
	virtual void ReleaseDynamicRHI()
	{
		VertexBufferRHI.SafeRelease();
	}

	void SetPrimitives(	FOcclusionPrimitive *InPrimitives, UINT InCount)
	{
		Primitives = InPrimitives;
		Size = InCount * sizeof(FOcclusionPrimitive);
	}

private:
	FOcclusionPrimitive *Primitives;
	UINT Size;
};

/** 
* Simple vertex buffer class for the occlusion query Indices
*/
class FOcclusionQueryIndicesVertexBuffer : public FVertexBuffer
{
public:
	/** 
	* Initialize the RHI for this rendering resource 
	*/
	virtual void InitRHI()
	{
		VertexBufferRHI = RHICreateVertexBuffer(NUM_CUBE_VERTICES, NULL, RUF_Static);
		BYTE* Indices = (BYTE*)RHILockVertexBuffer(VertexBufferRHI, 0, NUM_CUBE_VERTICES, FALSE);
		// pack the cube indices to UBYTE4 for Xbox 
		for(int Index=0; Index < (NUM_CUBE_VERTICES - 3); Index+=4)
		{
			Indices[Index+3]	=	(BYTE)GCubeIndices[Index];
			Indices[Index+2]	=	(BYTE)GCubeIndices[Index+1];
			Indices[Index+1]	=	(BYTE)GCubeIndices[Index+2];
			Indices[Index]		=	(BYTE)GCubeIndices[Index+3];
		}
		RHIUnlockVertexBuffer(VertexBufferRHI);
	}
};

/**
 * FOcclusionQueryDrawResource
 * 
 * Encapsulates global occlusion drawing resources.
 */
class FOcclusionQueryDrawResource : public FRenderResource
{
public:
	void DrawShared( FOcclusionPrimitive *InPrimitives, UINT InCount)
	{
		if( !IsValidRef(BoundShaderState) )
		{
			TShaderMapRef<FOcclusionQueryVertexShader<NUM_CUBE_VERTICES>> VertexShader(GetGlobalShaderMap());
			DWORD Strides[MaxVertexElementCount];
			appMemzero(Strides, sizeof(Strides));
			Strides[0] = sizeof(FVector);
			Strides[1] = sizeof(FOcclusionPrimitive);
			Strides[2] = sizeof(DWORD);
			BoundShaderState = RHICreateBoundShaderState(
				VertexDeclaration.VertexDeclarationRHI, 
				Strides, 
				VertexShader->GetVertexShader(), 
				NULL
				);
		}
		PrimitivesVertexBuffer.SetPrimitives(InPrimitives, InCount);
		PrimitivesVertexBuffer.UpdateRHI();
		// setup the streams
		RHISetStreamSource(0, BoxVertexBuffer.VertexBufferRHI, sizeof(FVector),FALSE,0,1);
		RHISetStreamSource(1, PrimitivesVertexBuffer.VertexBufferRHI, sizeof(FOcclusionPrimitive),FALSE,0,1);
		RHISetStreamSource(2, IndicesVertexBuffer.VertexBufferRHI, sizeof(DWORD),FALSE,0,1);
		// set the bound shader state
		RHISetBoundShaderState( BoundShaderState );
	}

	// FRenderResource interface.
	virtual void InitResource()
	{
		FRenderResource::InitResource();
		BoxVertexBuffer.InitResource();
		PrimitivesVertexBuffer.InitResource();;
		IndicesVertexBuffer.InitResource();;
		VertexDeclaration.InitResource();;
	}
	// FRenderResource interface.
	virtual void ReleaseResource()
	{
		FRenderResource::ReleaseResource();
		BoxVertexBuffer.ReleaseResource();
		PrimitivesVertexBuffer.ReleaseResource();
		IndicesVertexBuffer.ReleaseResource();
		VertexDeclaration.ReleaseResource();

		BoundShaderState.SafeRelease();
	}

private:
	FOcclusionQueryBoxVertexBuffer BoxVertexBuffer;
	FOcclusionQueryPrimitivesVertexBuffer PrimitivesVertexBuffer;
	FOcclusionQueryIndicesVertexBuffer IndicesVertexBuffer;
	FOcclusionQueryInstancingVertexDeclaration VertexDeclaration;
	FBoundShaderStateRHIRef BoundShaderState;

};
TGlobalResource<FOcclusionQueryDrawResource> GOcclusionDrawer;

#endif // XBOX


FOcclusionQueryPool::~FOcclusionQueryPool()
{
	Release();
}

void FOcclusionQueryPool::Release()
{
	OcclusionQueries.Empty();
}

INT GNumQueriesAllocated = 0;
INT GNumQueriesInPools = 0;
INT GNumQueriesOutstanding = 0;
FOcclusionQueryRHIRef FOcclusionQueryPool::AllocateQuery()
{
	GNumQueriesOutstanding++;

	// Are we out of available occlusion queries?
	if ( OcclusionQueries.Num() == 0 )
	{
		++GNumQueriesAllocated;
		// Create a new occlusion query.
		return RHICreateOcclusionQuery();
	}

	GNumQueriesInPools--;
	return OcclusionQueries.Pop();
}

void FOcclusionQueryPool::ReleaseQuery( FOcclusionQueryRHIRef &Query )
{
	if ( IsValidRef(Query) )
	{
		// Is no one else keeping a refcount to the query?
		if ( Query.GetRefCount() == 1 )
		{
			// Return it to the pool.
			OcclusionQueries.AddItem( Query );
			GNumQueriesInPools++;
			GNumQueriesOutstanding--;

			// Tell RHI we don't need the result anymore.
			RHIResetOcclusionQuery( Query );
		}

		// De-ref without deleting.
		Query = NULL;
	}
}


/**
* Expands a primitive bounds slightly to prevent the primitive from occluding it.
* @param Bounds - The base bounds of the primitive.
* @return The expanded bounds.
*/
static FBoxSphereBounds GetOcclusionBounds(const FBoxSphereBounds& Bounds)
{
	// Scale the box proportional to it's size. This is to prevent false occlusion reports.
	static const FLOAT BoundsOffset = 1.0f;
	static const FLOAT BoundsScale = 1.1f;
	static const FLOAT BoundsScaledOffset = BoundsOffset * BoundsScale;
	static const FVector BoundsScaledOffsetVector = FVector(1,1,1) * BoundsScaledOffset;
	return FBoxSphereBounds(
		Bounds.Origin,
		Bounds.BoxExtent * BoundsScale + BoundsScaledOffsetVector,
		Bounds.SphereRadius * BoundsScale + BoundsScaledOffset
		);
}

FGlobalBoundShaderState FSceneRenderer::OcclusionTestBoundShaderState;

void FSceneViewState::TrimOcclusionHistory(FLOAT MinHistoryTime,FLOAT MinQueryTime)
{
	for(TSet<FPrimitiveOcclusionHistory,FPrimitiveOcclusionHistoryKeyFuncs>::TIterator PrimitiveIt(PrimitiveOcclusionHistorySet);
		PrimitiveIt;
		++PrimitiveIt
		)
	{
		// If the primitive has an old pending occlusion query, release it.
		if(PrimitiveIt->LastConsideredTime < MinQueryTime)
		{
			OcclusionQueryPool.ReleaseQuery( PrimitiveIt->PendingOcclusionQuery );
		}

		// If the primitive hasn't been considered for visibility recently, remove its history from the set.
		if(PrimitiveIt->LastConsideredTime < MinHistoryTime)
		{
			PrimitiveIt.RemoveCurrent();
		}
	}
}

UBOOL FSceneViewState::UpdatePrimitiveOcclusion(
	const FPrimitiveSceneInfoCompact& CompactPrimitiveSceneInfo,
	FViewInfo& View,
	FLOAT CurrentRealTime,
	UBOOL& bOutPrimitiveIsDefinitelyUnoccluded
	)
{
	// Only allow primitives that are in the world DPG to be occluded.
	const UINT DepthPriorityGroup = CompactPrimitiveSceneInfo.bHasViewDependentDPG ?
		CompactPrimitiveSceneInfo.Proxy->GetDepthPriorityGroup(&View) :
		CompactPrimitiveSceneInfo.StaticDepthPriorityGroup;
	const UBOOL bIsOccludable = DepthPriorityGroup == SDPG_World && CompactPrimitiveSceneInfo.Proxy->RequiresOcclusion(&View);

	// Find the primitive's occlusion history.
	FPrimitiveOcclusionHistory* PrimitiveOcclusionHistory = PrimitiveOcclusionHistorySet.Find(CompactPrimitiveSceneInfo.Component);
	UBOOL bIsOccluded = FALSE;
	UBOOL bOcclusionStateIsDefinite = FALSE;
	if(!PrimitiveOcclusionHistory)
	{
		// If the primitive doesn't have an occlusion history yet, create it.
		PrimitiveOcclusionHistory = &PrimitiveOcclusionHistorySet(
			PrimitiveOcclusionHistorySet.Add(FPrimitiveOcclusionHistory(CompactPrimitiveSceneInfo.Component))
			);

		// If the primitive hasn't been visible recently enough to have a history, treat it as unoccluded this frame so it will be rendered as an occluder and its true occlusion state can be determined.
		bIsOccluded = FALSE;

		// Flag the primitive's occlusion state as indefinite, which will force it to be queried this frame.
		// The exception is if the primitive isn't occludable, in which case we know that it's definitely unoccluded.
		bOcclusionStateIsDefinite = bIsOccludable ? FALSE : TRUE;
	}
	else
	{
		if(View.bIgnoreExistingQueries)
		{
			// If the view is ignoring occlusion queries, the primitive is definitely unoccluded.
			bIsOccluded = FALSE;
			bOcclusionStateIsDefinite = View.bDisableQuerySubmissions;
		}
		else if(bIsOccludable)
		{
			// Read the occlusion query results.
			DWORD NumPixels = 0;
			if(	IsValidRef(PrimitiveOcclusionHistory->PendingOcclusionQuery) )
			{
				// NOTE: RHIGetOcclusionQueryResult should never fail when using a blocking call, rendering artifacts may show up.
				if ( RHIGetOcclusionQueryResult(PrimitiveOcclusionHistory->PendingOcclusionQuery,NumPixels,TRUE) )
				{
					// The primitive is occluded if non of its bounding box's pixels were visible in the previous frame's occlusion query.
					bIsOccluded = (NumPixels == 0);

					PrimitiveOcclusionHistory->LastPixelsPercentage = NumPixels / (View.SizeX * View.SizeY);

					// Flag the primitive's occlusion state as definite if it wasn't grouped.
					bOcclusionStateIsDefinite = !PrimitiveOcclusionHistory->bGroupedQuery;
				}
				else
				{
					// If the occlusion query failed, treat the primitive as visible.  
					bIsOccluded = FALSE;
				}
			}
			else
			{
				// If there's no occlusion query for the primitive, set it's visibility state to whether it has been unoccluded recently.
				bIsOccluded = (PrimitiveOcclusionHistory->LastVisibleTime + GEngine->PrimitiveProbablyVisibleTime < CurrentRealTime);
				if (bIsOccluded)
				{
					PrimitiveOcclusionHistory->LastPixelsPercentage = 0.0f;
				}
				else
				{
					PrimitiveOcclusionHistory->LastPixelsPercentage = CompactPrimitiveSceneInfo.bFirstFrameOcclusion ? 0.0f : GEngine->MaxOcclusionPixelsFraction;
				}
				// the state was definite last frame, otherwise we would have ran a query
				bOcclusionStateIsDefinite = TRUE;
			}
		}
		else
		{
			// Primitives that aren't occludable are considered definitely unoccluded.
			bIsOccluded = FALSE;
			bOcclusionStateIsDefinite = TRUE;
		}

		// Clear the primitive's pending occlusion query.
		OcclusionQueryPool.ReleaseQuery( PrimitiveOcclusionHistory->PendingOcclusionQuery );
	}

	// Set the primitive's considered time to keep its occlusion history from being trimmed.
	PrimitiveOcclusionHistory->LastConsideredTime = CurrentRealTime;

	// Enqueue the next frame's occlusion query for the primitive.
	if (!View.bDisableQuerySubmissions
		&& bIsOccludable
#if !FINAL_RELEASE
		&& !HasViewParent() && !bIsFrozen
#endif
		)
	{
		const FBoxSphereBounds OcclusionBounds = GetOcclusionBounds(CompactPrimitiveSceneInfo.Bounds);

		// Don't query primitives whose bounding boxes possibly intersect the viewer's near clipping plane.
		const FLOAT BoundsPushOut = FBoxPushOut(View.NearClippingPlane,OcclusionBounds.BoxExtent);

		// If the primitive is ignoring the near-plane check, we must check whether the camera is inside the occlusion box.
		UBOOL bAllowBoundsTest = TRUE;
		if ( View.bHasNearClippingPlane && CompactPrimitiveSceneInfo.bIgnoreNearPlaneIntersection )
		{
			// Disallow occlusion bound testing if the camera is inside, since we're not doing double-sided testing.
			FBox Bounds = OcclusionBounds.GetBox();
			bAllowBoundsTest = !Bounds.IsInside(View.ViewOrigin);
		}

		const FLOAT ClipDistance = View.NearClippingPlane.PlaneDot(OcclusionBounds.Origin);
		if ( bAllowBoundsTest &&
			(View.bHasNearClippingPlane && (ClipDistance < -BoundsPushOut || CompactPrimitiveSceneInfo.bIgnoreNearPlaneIntersection) ||
			!View.bHasNearClippingPlane && OcclusionBounds.SphereRadius < HALF_WORLD_MAX) )
		{
			// decide if a query should be run this frame
			UBOOL bRunQuery = FALSE;
			if(CompactPrimitiveSceneInfo.bAllowApproximateOcclusion)
			{
				if(bIsOccluded)
				{
					// Primitives that were occluded the previous frame use grouped queries.
					bRunQuery = TRUE;
					PrimitiveOcclusionHistory->bGroupedQuery = TRUE;
				}
				else if(bOcclusionStateIsDefinite)
				{
					// If the primitive's is definitely unoccluded, only requery it occasionally.
					FLOAT FractionMultiplier = Max(PrimitiveOcclusionHistory->LastPixelsPercentage/GEngine->MaxOcclusionPixelsFraction, 1.0f);
					static FRandomStream UnoccludedQueryRandomStream(0);
					bRunQuery = (FractionMultiplier * UnoccludedQueryRandomStream.GetFraction() < GEngine->MaxOcclusionPixelsFraction);
					PrimitiveOcclusionHistory->bGroupedQuery = FALSE;
				}
				else
				{
					bRunQuery = TRUE;
					PrimitiveOcclusionHistory->bGroupedQuery = FALSE;
				}
			}
			else
			{
				// Primitives that need precise occlusion results use individual queries.
				bRunQuery = TRUE;
				PrimitiveOcclusionHistory->bGroupedQuery = FALSE;
			}

			// Don't actually queue the primitive's occlusion query in wireframe, since it will not be submitted.
			if (bRunQuery && !(View.Family->ShowFlags & SHOW_Wireframe))
			{
				// Translate the occlusion bounds to the translated world-space used for rendering.
				const FBoxSphereBounds TranslatedOcclusionBounds(
					OcclusionBounds.Origin + View.PreViewTranslation,
					OcclusionBounds.BoxExtent,
					OcclusionBounds.SphereRadius
					);
				// Create the primitive's occlusion query in the appropriate batch.
				if(!PrimitiveOcclusionHistory->bGroupedQuery)
				{
					PrimitiveOcclusionHistory->PendingOcclusionQuery = View.IndividualOcclusionQueries.BatchPrimitive(TranslatedOcclusionBounds);	
				}
				else
				{
					PrimitiveOcclusionHistory->PendingOcclusionQuery = View.GroupedOcclusionQueries.BatchPrimitive(TranslatedOcclusionBounds);	
				}
			}
		}
		else
		{
			// If the primitive's bounding box intersects the near clipping plane, treat it as definitely unoccluded.
			bIsOccluded = FALSE;
			bOcclusionStateIsDefinite = TRUE;
		}
	}

	if((bOcclusionStateIsDefinite 
#if !FINAL_RELEASE
		|| bIsFrozen
#endif
		) && !bIsOccluded)
	{
		// Update the primitive's visibility time in the occlusion history.
		// This is only done when we have definite occlusion results for the primitive.
		PrimitiveOcclusionHistory->LastVisibleTime = CurrentRealTime;

		bOutPrimitiveIsDefinitelyUnoccluded = TRUE;
	}
	else
	{
		bOutPrimitiveIsDefinitelyUnoccluded = FALSE;
	}

	return bIsOccluded;
}

UBOOL FSceneViewState::IsShadowOccluded(const UPrimitiveComponent* Primitive,const ULightComponent* Light) const
{
	// Find the shadow's occlusion query from the previous frame.
	const FSceneViewState::FProjectedShadowKey Key(Primitive,Light);
	const FOcclusionQueryRHIRef* Query = ShadowOcclusionQueryMap.Find(Key);

	// Read the occlusion query results.
	DWORD NumPixels = 0;
	if(Query && RHIGetOcclusionQueryResult(*Query,NumPixels,TRUE))
	{
		// If the shadow's occlusion query didn't have any pixels visible the previous frame, it's occluded.
		return NumPixels == 0;
	}
	else
	{
		// If the shadow wasn't queried the previous frame, it isn't occluded.8

		return FALSE;
	}
}

/**
 *	Retrieves the percentage of the views render target the primitive touched last time it was rendered.
 *
 *	@param	Primitive				The primitive of interest.
 *	@param	OutCoveragePercentage	REFERENCE: The screen coverate percentage. (OUTPUT)
 *	@return	UBOOL					TRUE if the primitive was found and the results are valid, FALSE is not
 */
UBOOL FSceneViewState::GetPrimitiveCoveragePercentage(const UPrimitiveComponent* Primitive, FLOAT& OutCoveragePercentage)
{
	FPrimitiveOcclusionHistory* PrimitiveOcclusionHistory = PrimitiveOcclusionHistorySet.Find(Primitive);
	if (PrimitiveOcclusionHistory)
	{
		OutCoveragePercentage = PrimitiveOcclusionHistory->LastPixelsPercentage;
		return TRUE;
	}

	return FALSE;
}

FOcclusionQueryBatcher::FOcclusionQueryBatcher(class FSceneViewState* ViewState,UINT InMaxBatchedPrimitives)
:	CurrentBatchOcclusionQuery(FOcclusionQueryRHIRef())
,	MaxBatchedPrimitives(InMaxBatchedPrimitives)
,	NumBatchedPrimitives(0)
,	OcclusionQueryPool(ViewState ? &ViewState->OcclusionQueryPool : NULL)
{}

FOcclusionQueryBatcher::~FOcclusionQueryBatcher()
{
	check(!Primitives.Num());
}

void FOcclusionQueryBatcher::Flush()
{
	if(BatchOcclusionQueries.Num())
	{
#if !XBOX
		FMemMark MemStackMark(GRenderingThreadMemStack);

		// Create the indices for MaxBatchedPrimitives boxes.
		WORD* BakedIndices = new(GRenderingThreadMemStack) WORD[MaxBatchedPrimitives * 12 * 3];
		for(UINT PrimitiveIndex = 0;PrimitiveIndex < MaxBatchedPrimitives;PrimitiveIndex++)
		{
			for(INT Index = 0;Index < NUM_CUBE_VERTICES;Index++)
			{
				BakedIndices[PrimitiveIndex * NUM_CUBE_VERTICES + Index] = PrimitiveIndex * 8 + GCubeIndices[Index];
			}
		}

		// Draw the batches.
		for(INT BatchIndex = 0;BatchIndex < BatchOcclusionQueries.Num();BatchIndex++)
		{
			FOcclusionQueryRHIParamRef BatchOcclusionQuery = BatchOcclusionQueries(BatchIndex);
			const INT NumPrimitivesInBatch = Clamp<INT>( Primitives.Num() - BatchIndex * MaxBatchedPrimitives, 0, MaxBatchedPrimitives );
				
			RHIBeginOcclusionQuery(BatchOcclusionQuery);

			FLOAT* RESTRICT Vertices;
			WORD* RESTRICT Indices;
			RHIBeginDrawIndexedPrimitiveUP(PT_TriangleList,NumPrimitivesInBatch * 12,NumPrimitivesInBatch * 8,sizeof(FVector),*(void**)&Vertices,0,NumPrimitivesInBatch * 12 * 3,sizeof(WORD),*(void**)&Indices);

			for(INT PrimitiveIndex = 0;PrimitiveIndex < NumPrimitivesInBatch;PrimitiveIndex++)
			{
				const FOcclusionPrimitive& Primitive = Primitives(BatchIndex * MaxBatchedPrimitives + PrimitiveIndex);
				const UINT BaseVertexIndex = PrimitiveIndex * 8;
				const FVector PrimitiveBoxMin = Primitive.Origin - Primitive.Extent;
				const FVector PrimitiveBoxMax = Primitive.Origin + Primitive.Extent;

				Vertices[ 0] = PrimitiveBoxMin.X; Vertices[ 1] = PrimitiveBoxMin.Y; Vertices[ 2] = PrimitiveBoxMin.Z;
				Vertices[ 3] = PrimitiveBoxMin.X; Vertices[ 4] = PrimitiveBoxMin.Y; Vertices[ 5] = PrimitiveBoxMax.Z;
				Vertices[ 6] = PrimitiveBoxMin.X; Vertices[ 7] = PrimitiveBoxMax.Y; Vertices[ 8] = PrimitiveBoxMin.Z;
				Vertices[ 9] = PrimitiveBoxMin.X; Vertices[10] = PrimitiveBoxMax.Y; Vertices[11] = PrimitiveBoxMax.Z;
				Vertices[12] = PrimitiveBoxMax.X; Vertices[13] = PrimitiveBoxMin.Y; Vertices[14] = PrimitiveBoxMin.Z;
				Vertices[15] = PrimitiveBoxMax.X; Vertices[16] = PrimitiveBoxMin.Y; Vertices[17] = PrimitiveBoxMax.Z;
				Vertices[18] = PrimitiveBoxMax.X; Vertices[19] = PrimitiveBoxMax.Y; Vertices[20] = PrimitiveBoxMin.Z;
				Vertices[21] = PrimitiveBoxMax.X; Vertices[22] = PrimitiveBoxMax.Y; Vertices[23] = PrimitiveBoxMax.Z;

				Vertices += 24;
			}

			appMemcpy(Indices,BakedIndices,sizeof(WORD) * NumPrimitivesInBatch * 12 * 3);

			RHIEndDrawIndexedPrimitiveUP();

			RHIEndOcclusionQuery(BatchOcclusionQuery);
		}
#else // XBOX
		// prepare RHI for drawing
		GOcclusionDrawer.DrawShared(Primitives.GetTypedData(), Primitives.Num());
		// Draw the batches.
		for(INT BatchIndex = 0;BatchIndex < BatchOcclusionQueries.Num();BatchIndex++)
		{
			FOcclusionQueryRHIParamRef BatchOcclusionQuery = BatchOcclusionQueries(BatchIndex);
			FOcclusionPrimitive *BatchPrimitives = Primitives.GetTypedData() + BatchIndex * MaxBatchedPrimitives;
			INT BatchVertexIndex = NUM_CUBE_VERTICES * BatchIndex * MaxBatchedPrimitives;
			INT NumPrimitivesInBatch = 12 * Clamp<INT>( Primitives.Num() - BatchIndex * MaxBatchedPrimitives, 0, MaxBatchedPrimitives );
			RHIBeginOcclusionQuery(BatchOcclusionQuery);
			RHIDrawPrimitive(PT_TriangleList,BatchVertexIndex,NumPrimitivesInBatch);
			RHIEndOcclusionQuery(BatchOcclusionQuery);
		}
#endif
		INC_DWORD_STAT_BY(STAT_OcclusionQueries,BatchOcclusionQueries.Num());

		// Reset the batch state.
		BatchOcclusionQueries.Empty(BatchOcclusionQueries.Num());
		Primitives.Empty(Primitives.Num());
		CurrentBatchOcclusionQuery = FOcclusionQueryRHIRef();
	}
}

FOcclusionQueryRHIParamRef FOcclusionQueryBatcher::BatchPrimitive(const FBoxSphereBounds& Bounds)
{
	// Check if the current batch is full.
#if PS3 && USE_NULL_RHI
	if(NumBatchedPrimitives >= MaxBatchedPrimitives)
#else
	if(!IsValidRef(CurrentBatchOcclusionQuery) || NumBatchedPrimitives >= MaxBatchedPrimitives)
#endif
	{
		check(OcclusionQueryPool);
		INT Index = BatchOcclusionQueries.AddItem( OcclusionQueryPool->AllocateQuery() );
		CurrentBatchOcclusionQuery = BatchOcclusionQueries( Index );
		NumBatchedPrimitives = 0;
	}

	// Add the primitive to the current batch.
	FOcclusionPrimitive* const Primitive = new(Primitives) FOcclusionPrimitive;
	Primitive->Origin = Bounds.Origin;
	Primitive->Extent = Bounds.BoxExtent;
	NumBatchedPrimitives++;

	return CurrentBatchOcclusionQuery;
}

void FSceneRenderer::BeginOcclusionTests()
{
	// Perform occlusion queries for each view
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		SCOPED_DRAW_EVENT(EventBeginOcclude)(DEC_SCENE_ITEMS,TEXT("BeginOcclusionTests"));
		FViewInfo& View = Views(ViewIndex);

		RHISetViewParameters( &View, View.TranslatedViewProjectionMatrix, View.ViewOrigin );

		RHISetViewport(View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);
    
	    FSceneViewState* ViewState = (FSceneViewState*)View.State;
    
	    if(ViewState && !View.bDisableQuerySubmissions)
	    {
			{
				SCOPED_DRAW_EVENT(EventShadowQueries)(DEC_SCENE_ITEMS,TEXT("ShadowFrustumQueries"));
				SCOPE_CYCLE_COUNTER(STAT_BeginOcclusionTestsTime);

				// Lookup the vertex shader.
				TShaderMapRef<FOcclusionQueryVertexShader<0> > VertexShader(GetGlobalShaderMap());

				// Issue this frame's occlusion queries (occlusion queries from last frame may still be in flight)
				typedef TMap<FSceneViewState::FProjectedShadowKey, FOcclusionQueryRHIRef> TShadowOcclusionQueryMap;
				TShadowOcclusionQueryMap& ShadowOcclusionQueryMap = ViewState->ShadowOcclusionQueryMap;

				// Clear primitives which haven't been visible recently out of the occlusion history, and reset old pending occlusion queries.
				ViewState->TrimOcclusionHistory(ViewFamily.CurrentRealTime - GEngine->PrimitiveProbablyVisibleTime,ViewFamily.CurrentRealTime);

				// Depth tests, no depth writes, no color writes, opaque, solid rasterization wo/ backface culling.
				RHISetDepthState(TStaticDepthState<FALSE,CF_LessEqual>::GetRHI());
				RHISetColorWriteEnable(FALSE);
				// We only need to render the front-faces of the culling geometry (this halves the amount of pixels we touch)
				RHISetRasterizerState(
					View.bReverseCulling ? TStaticRasterizerState<FM_Solid,CM_CCW>::GetRHI() : TStaticRasterizerState<FM_Solid,CM_CW>::GetRHI()); 
				RHISetBlendState(TStaticBlendState<>::GetRHI());

				SetGlobalBoundShaderState(OcclusionTestBoundShaderState,GOcclusionQueryPosOnlyVertexDeclaration.VertexDeclarationRHI,*VertexShader,NULL,sizeof(FVector));

				// Give back all these occlusion queries to the pool.
				for ( TShadowOcclusionQueryMap::TIterator QueryIt(ShadowOcclusionQueryMap); QueryIt; ++QueryIt )
				{
					//FOcclusionQueryRHIParamRef Query = QueryIt.Value();
					//check( Query.GetRefCount() == 1 );
					ViewState->OcclusionQueryPool.ReleaseQuery( QueryIt.Value() );
				}
				ShadowOcclusionQueryMap.Reset();

				for(TSparseArray<FLightSceneInfoCompact>::TConstIterator LightIt(Scene->Lights);LightIt;++LightIt)
				{
					const FVisibleLightInfo& VisibleLightInfo = VisibleLightInfos(LightIt.GetIndex());
					for(INT ShadowIndex = 0;ShadowIndex < VisibleLightInfo.ProjectedShadows.Num();ShadowIndex++)
					{
						const FProjectedShadowInfo& ProjectedShadowInfo = *VisibleLightInfo.ProjectedShadows(ShadowIndex);

						// Don't query preshadows, since they are culled if their subject is occluded.
						// Also don't query if any subjects are visible because the shadow frustum will be definitely unoccluded
						if(!ProjectedShadowInfo.SubjectsVisible(View) && !ProjectedShadowInfo.bPreShadow)
						{
							// The shadow transforms and view transforms are relative to different origins, so the world coordinates need to
							// be translated.
							const FVector4 PreShadowToPreViewTranslation(View.PreViewTranslation - ProjectedShadowInfo.PreShadowTranslation,0);

							// If the shadow frustum is farther from the view origin than the near clipping plane,
							// it can't intersect the near clipping plane.
							const UBOOL bIntersectsNearClippingPlane = ProjectedShadowInfo.ReceiverFrustum.IntersectSphere(
								-View.PreViewTranslation + ProjectedShadowInfo.PreShadowTranslation,
								View.NearClippingDistance * appSqrt(3.0f)
								);
							if( !bIntersectsNearClippingPlane )
							{
								// Allocate an occlusion query for the primitive from the occlusion query pool.
								const FOcclusionQueryRHIRef ShadowOcclusionQuery = ViewState->OcclusionQueryPool.AllocateQuery();

								// Draw the primitive's bounding box, using the occlusion query.
								RHIBeginOcclusionQuery(ShadowOcclusionQuery);

								void* VerticesPtr;
								void* IndicesPtr;
								// preallocate memory to fill out with vertices and indices
								RHIBeginDrawIndexedPrimitiveUP( PT_TriangleList, 12, 8, sizeof(FVector), VerticesPtr, 0, NUM_CUBE_VERTICES, sizeof(WORD), IndicesPtr);
								FVector* Vertices = (FVector*)VerticesPtr;
								WORD* Indices = (WORD*)IndicesPtr;

								// Generate vertices for the shadow's frustum.
								for(UINT Z = 0;Z < 2;Z++)
								{
									for(UINT Y = 0;Y < 2;Y++)
									{
										for(UINT X = 0;X < 2;X++)
										{
											const FVector4 UnprojectedVertex = ProjectedShadowInfo.InvReceiverMatrix.TransformFVector4(
												FVector4(
												(X ? -1.0f : 1.0f),
												(Y ? -1.0f : 1.0f),
												(Z ?  1.0f : 0.0f),
												1.0f
												)
												);
											const FVector ProjectedVertex = UnprojectedVertex / UnprojectedVertex.W + PreShadowToPreViewTranslation;
											Vertices[GetCubeVertexIndex(X,Y,Z)] = ProjectedVertex;
										}
									}
								}

								// we just copy the indices right in
								appMemcpy(Indices, GCubeIndices, sizeof(GCubeIndices));

								FSceneViewState::FProjectedShadowKey Key(
									ProjectedShadowInfo.ParentSceneInfo ? 
									ProjectedShadowInfo.ParentSceneInfo->Component :
								NULL,
									ProjectedShadowInfo.LightSceneInfo->LightComponent
									);
								checkSlow(ShadowOcclusionQueryMap.Find(Key) == NULL);
								ShadowOcclusionQueryMap.Set(Key, ShadowOcclusionQuery);

								RHIEndDrawIndexedPrimitiveUP();
								RHIEndOcclusionQuery(ShadowOcclusionQuery);
							}
						}
					}
				}
			}
    
		    // Don't do primitive occlusion if we have a view parent or are frozen.
    #if !FINAL_RELEASE
		    if ( !ViewState->HasViewParent() && !ViewState->bIsFrozen )
    #endif
		    {
				{
					SCOPED_DRAW_EVENT(EventIndividualQueries)(DEC_SCENE_ITEMS,TEXT("IndividualQueries"));
					View.IndividualOcclusionQueries.Flush();
				}
				{
					SCOPED_DRAW_EVENT(EventGroupedQueries)(DEC_SCENE_ITEMS,TEXT("GroupedQueries"));
					View.GroupedOcclusionQueries.Flush();
				}
		    }
		    
		    // Reenable color writes.
		    RHISetColorWriteEnable(TRUE);
    
		    // Kick the commands.
		    RHIKickCommandBuffer();
	    }
    }
}
