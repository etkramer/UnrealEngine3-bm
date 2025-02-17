/*=============================================================================
	UnSkeletalRenderGPUSkin.cpp: GPU skinned skeletal mesh rendering code.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineAnimClasses.h"
#include "EnginePhysicsClasses.h"
#include "GPUSkinVertexFactory.h"
#include "UnSkeletalRenderGPUSkin.h"
#include "UnDecalRenderData.h" 

#if XBOX
#include "UnSkeletalRenderGPUXe.h"
#endif

/*-----------------------------------------------------------------------------
FMorphVertexBuffer
-----------------------------------------------------------------------------*/

/** 
* Initialize the dynamic RHI for this rendering resource 
*/
void FMorphVertexBuffer::InitDynamicRHI()
{
	// LOD of the skel mesh is used to find number of vertices in buffer
	FStaticLODModel& LodModel = SkelMesh->LODModels(LODIdx);

	// Create the buffer rendering resource
	UINT Size = LodModel.NumVertices * sizeof(FMorphGPUSkinVertex);
	VertexBufferRHI = RHICreateVertexBuffer(Size,NULL,RUF_Volatile);

	// Lock the buffer.
	FMorphGPUSkinVertex* Buffer = (FMorphGPUSkinVertex*) RHILockVertexBuffer(VertexBufferRHI,0,Size,FALSE);

	// zero all deltas (NOTE: DeltaTangentZ is FPackedNormal, so we can't just appMemzero)
	for (UINT VertIndex=0; VertIndex < LodModel.NumVertices; ++VertIndex)
	{
		Buffer[VertIndex].DeltaPosition = FVector(0,0,0);
		Buffer[VertIndex].DeltaTangentZ = FVector(0,0,0);
	}

	// Unlock the buffer.
	RHIUnlockVertexBuffer(VertexBufferRHI);
}

/** 
* Release the dynamic RHI for this rendering resource 
*/
void FMorphVertexBuffer::ReleaseDynamicRHI()
{
	VertexBufferRHI.SafeRelease();
}

/*-----------------------------------------------------------------------------
FInflucenceWeightsVertexBuffer
-----------------------------------------------------------------------------*/

/** 
* Initialize the dynamic RHI for this rendering resource 
*/
void FInflucenceWeightsVertexBuffer::InitDynamicRHI()
{
	// LOD of the skel mesh is used to find number of vertices in buffer
	FStaticLODModel& LodModel = SkelMesh->LODModels(LODIdx);

	// Create the buffer rendering resource
	UINT Size = LodModel.NumVertices * sizeof(FVertexInfluence);
	VertexBufferRHI = RHICreateVertexBuffer(Size,NULL,RUF_Dynamic);

	// just weights copy from the base skelmesh vertex buffer for defaults
	// Lock the buffer.
	FVertexInfluence* Buffer = (FVertexInfluence*) RHILockVertexBuffer(VertexBufferRHI,0,Size,FALSE);

	// zero all deltas (NOTE: DeltaTangentZ is FPackedNormal, so we can't just appMemzero)
	for (UINT VertIndex=0; VertIndex < LodModel.NumVertices; ++VertIndex)
	{
		const FGPUSkinVertexBase* BaseVert = LodModel.VertexBufferGPUSkin.GetVertexPtr(VertIndex);

		for( INT Idx=0; Idx < MAX_INFLUENCES; Idx++ )
 		{
 			Buffer[VertIndex].Weights.InfluenceWeights[Idx] = BaseVert->InfluenceWeights[Idx];
			Buffer[VertIndex].Bones.InfluenceBones[Idx] = BaseVert->InfluenceBones[Idx];
 		}
	}

	// Unlock the buffer.
	RHIUnlockVertexBuffer(VertexBufferRHI);
}

/** 
* Release the dynamic RHI for this rendering resource 
*/
void FInflucenceWeightsVertexBuffer::ReleaseDynamicRHI()
{
	VertexBufferRHI.SafeRelease();
}

/*-----------------------------------------------------------------------------
FSkeletalMeshObjectGPUSkin
-----------------------------------------------------------------------------*/

/** 
 * Constructor 
 * @param	InSkeletalMeshComponent - skeletal mesh primitive we want to render 
 */
FSkeletalMeshObjectGPUSkin::FSkeletalMeshObjectGPUSkin(USkeletalMeshComponent* InSkeletalMeshComponent) 
:	FSkeletalMeshObject(InSkeletalMeshComponent)
,	CachedShadowLOD(INDEX_NONE)
,	DynamicData(NULL)
,	bMorphResourcesInitialized(FALSE)
,	bInfluenceWeightsInitialized(FALSE)
{
	// create LODs to match the base mesh
	LODs.Empty(SkeletalMesh->LODModels.Num());
	for( INT LODIndex=0;LODIndex < SkeletalMesh->LODModels.Num();LODIndex++ )
	{
		new(LODs) FSkeletalMeshObjectLOD(SkeletalMesh,LODIndex,bDecalFactoriesEnabled);
	}

	InitResources();
}

/** 
 * Destructor 
 */
FSkeletalMeshObjectGPUSkin::~FSkeletalMeshObjectGPUSkin()
{
	delete DynamicData;
}

/** 
 * Initialize rendering resources for each LOD. 
 */
void FSkeletalMeshObjectGPUSkin::InitResources()
{
	for( INT LODIndex=0;LODIndex < LODs.Num();LODIndex++ )
	{
		FSkeletalMeshObjectLOD& SkelLOD = LODs(LODIndex);
		SkelLOD.InitResources(bUseLocalVertexFactory,bUseInstancedVertexInfluences);
	}
}

/** 
 * Release rendering resources for each LOD.
 */
void FSkeletalMeshObjectGPUSkin::ReleaseResources()
{
	for( INT LODIndex=0;LODIndex < LODs.Num();LODIndex++ )
	{
		FSkeletalMeshObjectLOD& SkelLOD = LODs(LODIndex);
		SkelLOD.ReleaseResources();
	}
	// also release morph resources
	ReleaseMorphResources();
}

/** 
* Initialize morph rendering resources for each LOD 
*/
void FSkeletalMeshObjectGPUSkin::InitMorphResources()
{
	if( bMorphResourcesInitialized )
	{
		// release first if already initialized
		ReleaseMorphResources();
	}

	for( INT LODIndex=0;LODIndex < LODs.Num();LODIndex++ )
	{
		FSkeletalMeshObjectLOD& SkelLOD = LODs(LODIndex);
		// init any morph vertex buffers for each LOD
		SkelLOD.InitMorphResources(bUseInstancedVertexInfluences);
	}
	bMorphResourcesInitialized = TRUE;
}

/** 
* Release morph rendering resources for each LOD. 
*/
void FSkeletalMeshObjectGPUSkin::ReleaseMorphResources()
{
	for( INT LODIndex=0;LODIndex < LODs.Num();LODIndex++ )
	{
		FSkeletalMeshObjectLOD& SkelLOD = LODs(LODIndex);
		// release morph vertex buffers and factories if they were created
		SkelLOD.ReleaseMorphResources();
	}
	bMorphResourcesInitialized = FALSE;
}

/**
* Called by the game thread for any dynamic data updates for this skel mesh object
* @param	LODIndex - lod level to update
* @param	InSkeletalMeshComponen - parent prim component doing the updating
* @param	ActiveMorphs - morph targets to blend with during skinning
*/
void FSkeletalMeshObjectGPUSkin::Update(INT LODIndex,USkeletalMeshComponent* InSkeletalMeshComponent,const TArray<FActiveMorph>& ActiveMorphs)
{
	// make sure morph data has been initialized for each LOD
	if( !bMorphResourcesInitialized && ActiveMorphs.Num() > 0 )
	{
		// initialized on-the-fly in order to avoid creating extra vertex streams for each skel mesh instance
		InitMorphResources();		
	}

	// create the new dynamic data for use by the rendering thread
	// this data is only deleted when another update is sent
	FDynamicSkelMeshObjectData* NewDynamicData = new FDynamicSkelMeshObjectDataGPUSkin(InSkeletalMeshComponent,LODIndex,ActiveMorphs);


	// queue a call to update this data
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		SkelMeshObjectUpdateDataCommand,
		FSkeletalMeshObject*, MeshObject, this,
		FDynamicSkelMeshObjectData*, NewDynamicData, NewDynamicData,
	{
		MeshObject->UpdateDynamicData_RenderThread(NewDynamicData);
	}
	);
}

/**
 * Called by the rendering thread to update the current dynamic data
 * @param	InDynamicData - data that was created by the game thread for use by the rendering thread
 */
void FSkeletalMeshObjectGPUSkin::UpdateDynamicData_RenderThread(FDynamicSkelMeshObjectData* InDynamicData)
{
	UBOOL bMorphNeedsUpdate=FALSE;
	// figure out if the morphing vertex buffer needs to be updated. compare old vs new active morphs
	bMorphNeedsUpdate = DynamicData ? (DynamicData->LODIndex != ((FDynamicSkelMeshObjectDataGPUSkin*)InDynamicData)->LODIndex ||
		!DynamicData->ActiveMorphTargetsEqual(((FDynamicSkelMeshObjectDataGPUSkin*)InDynamicData)->ActiveMorphs))
		: TRUE;

	// we should be done with the old data at this point
	delete DynamicData;
	// update with new data
	DynamicData = (FDynamicSkelMeshObjectDataGPUSkin*)InDynamicData;
	checkSlow(DynamicData);

	FSkeletalMeshObjectLOD& LOD = LODs(DynamicData->LODIndex);
	const FStaticLODModel& LODModel = SkeletalMesh->LODModels(DynamicData->LODIndex);

	checkSlow( DynamicData->NumWeightedActiveMorphs == 0 || (LOD.MorphVertexFactories.Num()==LODModel.Chunks.Num()) );
	if (DynamicData->NumWeightedActiveMorphs > 0 || LOD.VertexFactories.Num() > 0)
	{
		for( INT ChunkIdx=0; ChunkIdx < LODModel.Chunks.Num(); ChunkIdx++ )
		{
			const FSkelMeshChunk& Chunk = LODModel.Chunks(ChunkIdx);
			FGPUSkinVertexFactory::ShaderDataType& ShaderData = DynamicData->NumWeightedActiveMorphs > 0 ? LOD.MorphVertexFactories(ChunkIdx).GetShaderData() : LOD.VertexFactories(ChunkIdx).GetShaderData();

			// update bone matrix shader data for the vertex factory of each chunk
			TArray<FSkinMatrix3x4>& ChunkMatrices = ShaderData.BoneMatrices;
			ChunkMatrices.Reset(); // remove all elts but leave allocated

			const INT NumBones = Chunk.BoneMap.Num();
			ChunkMatrices.Reserve( NumBones ); // we are going to keep adding data to this for each bone

			const INT NumToAdd = NumBones - ChunkMatrices.Num();
			ChunkMatrices.Add( NumToAdd );

			//FSkinMatrix3x4 is sizeof() == 48
			// CACHE_LINE_SIZE (128) / 48 = 2.6
			//  sizeof(FMatrix) == 64
			// CACHE_LINE_SIZE (128) / 64 = 2
			const INT PreFetchStride = 2; // PREFETCH stride

			TArray<FMatrix>& ReferenceToLocalMatrices = DynamicData->ReferenceToLocal;
			const INT NumReferenceToLocal = ReferenceToLocalMatrices.Num();

			for( INT BoneIdx=0; BoneIdx < NumBones; BoneIdx++ )
			{
 				if( BoneIdx+PreFetchStride < NumBones )
 				{
 					CONSOLE_PREFETCH(&ChunkMatrices(BoneIdx + PreFetchStride)); 
 					CONSOLE_PREFETCH(&ChunkMatrices(BoneIdx + PreFetchStride) + CACHE_LINE_SIZE); 
 				}

 				if( BoneIdx+PreFetchStride+PreFetchStride+PreFetchStride < NumReferenceToLocal )
 				{
 					CONSOLE_PREFETCH(&ReferenceToLocalMatrices(BoneIdx + PreFetchStride));
 					CONSOLE_PREFETCH(&ReferenceToLocalMatrices(BoneIdx + PreFetchStride) + CACHE_LINE_SIZE);
					CONSOLE_PREFETCH(&ReferenceToLocalMatrices(BoneIdx + PreFetchStride) + CACHE_LINE_SIZE + CACHE_LINE_SIZE);
				}

				FSkinMatrix3x4& BoneMat = ChunkMatrices(BoneIdx);
				BoneMat.SetMatrixTranspose(ReferenceToLocalMatrices(Chunk.BoneMap(BoneIdx)));
			}

			// update the max number of influences for the vertex factory
			ShaderData.SetMaxBoneInfluences(Chunk.MaxBoneInfluences);
		}
	}

	// Decal factories
	if ( bDecalFactoriesEnabled )
	{
		checkSlow( DynamicData->NumWeightedActiveMorphs == 0 || (LOD.MorphDecalVertexFactories.Num()==LODModel.Chunks.Num()) );
		if (DynamicData->NumWeightedActiveMorphs > 0 || LOD.DecalVertexFactories.Num() > 0)
		{
			for( INT ChunkIdx=0; ChunkIdx < LODModel.Chunks.Num(); ChunkIdx++ )
			{
				const FSkelMeshChunk& Chunk = LODModel.Chunks(ChunkIdx);
				FGPUSkinVertexFactory::ShaderDataType& ShaderData = DynamicData->NumWeightedActiveMorphs > 0 ? LOD.MorphDecalVertexFactories(ChunkIdx).GetShaderData() : LOD.DecalVertexFactories(ChunkIdx).GetShaderData();

				// update bone matrix shader data for the vertex factory of each chunk
				TArray<FSkinMatrix3x4>& ChunkMatrices = ShaderData.BoneMatrices;
				ChunkMatrices.Reset(); // remove all elts but leave allocated

				const INT NumBones = Chunk.BoneMap.Num();
				ChunkMatrices.Reserve( NumBones ); // we are going to keep adding data to this for each bone

				const INT NumToAdd = NumBones - ChunkMatrices.Num();
				ChunkMatrices.Add( NumToAdd );

				//FSkinMatrix3x4 is sizeof() == 48
				// CACHE_LINE_SIZE (128) / 48 = 2.6
				//  sizeof(FMatrix) == 64
				// CACHE_LINE_SIZE (128) / 64 = 2
				const INT PreFetchStride = 2; // PREFETCH stride

				TArray<FMatrix>& ReferenceToLocalMatrices = DynamicData->ReferenceToLocal;
				const INT NumReferenceToLocal = ReferenceToLocalMatrices.Num();

				for( INT BoneIdx=0; BoneIdx < NumBones; BoneIdx++ )
				{
 				if( BoneIdx+PreFetchStride < NumBones )
 				{
 						CONSOLE_PREFETCH(&ChunkMatrices(BoneIdx + PreFetchStride)); 
 						CONSOLE_PREFETCH(&ChunkMatrices(BoneIdx + PreFetchStride) + CACHE_LINE_SIZE); 
 				}

 				if( BoneIdx+PreFetchStride+PreFetchStride+PreFetchStride < NumReferenceToLocal )
 				{
 						CONSOLE_PREFETCH(&ReferenceToLocalMatrices(BoneIdx + PreFetchStride));
 						CONSOLE_PREFETCH(&ReferenceToLocalMatrices(BoneIdx + PreFetchStride) + CACHE_LINE_SIZE);
						CONSOLE_PREFETCH(&ReferenceToLocalMatrices(BoneIdx + PreFetchStride) + CACHE_LINE_SIZE + CACHE_LINE_SIZE);
				}
					FSkinMatrix3x4& BoneMat = ChunkMatrices(BoneIdx);
					BoneMat.SetMatrixTranspose(ReferenceToLocalMatrices(Chunk.BoneMap(BoneIdx)));
				}

				// update the max number of influences for the vertex factory
				ShaderData.SetMaxBoneInfluences(Chunk.MaxBoneInfluences);
			}
		}
	}

	// Flag the vertex cache as dirty, so potential shadow volumes can update it
	CachedShadowLOD = INDEX_NONE;

	// only update if the morph data changed and there are weighted morph targets
	if( bMorphNeedsUpdate &&
		DynamicData->NumWeightedActiveMorphs > 0 )
	{
		// update the morph data for the lod
		LOD.UpdateMorphVertexBuffer( DynamicData->ActiveMorphs );
	}
}

/** 
* Called by the game thread to toggle usage for the instanced vertex weights.
* If enabled then a vertex buffer is created for this skeletal mesh instance. 
* If disabled then the existing vertex buffer is released.
* @parm bEnabled - TRUE to enable the usage of influence weights
*/
void FSkeletalMeshObjectGPUSkin::ToggleVertexInfluences(UBOOL bEnabled)
{
	if( bUseInstancedVertexInfluences != bEnabled )
	{
		bUseInstancedVertexInfluences = bEnabled;

		// re-init the vertex factories to use the influence weights vertex buffer instance
		ReleaseResources();
#if !FINAL_RELEASE
		DOUBLE Start = appSeconds();
#endif
		// NOTE: no toggling needed at runtime if you force instance weights to enabled when calling USkeletalMeshComponent::SetSkeletalMesh(..,TRUE)
		FlushRenderingCommands();
#if !FINAL_RELEASE
		debugf(TEXT("ToggleVertexInfluences: (%s) FlushRenderingCommands Took %f ms"), *SkeletalMesh->GetPathName(), (appSeconds() - Start) * 1000.f);
#endif
		InitResources();
	}
}

/**
* Called by the game thread to update the instanced vertex weights
* @param BonePairs - set of bone pairs used to find vertices that need to have their weights updated
* @param InfluenceIdx - index of skeletal mesh vertex influence entry to use (INDEX_NONE for defaults from base verts)
* @param bResetInfluences - resets the array of instanced influences using the ones from the base mesh before updating
*/
void FSkeletalMeshObjectGPUSkin::UpdateVertexInfluences(const TArray<FBoneIndexPair>& BonePairs,
									  INT InfluenceIdx,
									  UBOOL bResetInfluences)
{
	// force instance weights on so that resources get re-initialized
	ToggleVertexInfluences(TRUE);

	FDynamicUpdateVertexInfluencesData DynamicInfluencesData(
		BonePairs,
		InfluenceIdx,
		bResetInfluences
		);

	// queue a call to update this weight data
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		SkelMeshObjectUpdateWeightsCommand,
		FSkeletalMeshObject*, MeshObject, this,
		FDynamicUpdateVertexInfluencesData, DynamicInfluencesData, DynamicInfluencesData,
	{
		MeshObject->UpdateVertexInfluences_RenderThread(&DynamicInfluencesData);
	}
	);
}

/**
* Called by the rendering thread to update the current dynamic weight data
* @param	InDynamicData - data that was created by the game thread for use by the rendering thread
*/
void FSkeletalMeshObjectGPUSkin::UpdateVertexInfluences_RenderThread(FDynamicUpdateVertexInfluencesData* InDynamicData)
{
	// update instance weights for all LODs
	for( INT CurLODIdx=0; CurLODIdx < LODs.Num(); CurLODIdx++ )
	{
		const FSkeletalMeshObjectLOD& LOD = LODs(CurLODIdx);
		const FStaticLODModel& LODModel = SkeletalMesh->LODModels(CurLODIdx);

		if( IsValidRef(LOD.WeightsVertexBuffer.VertexBufferRHI) && 
			LODModel.VertexInfluences.IsValidIndex(InDynamicData->InfluenceIdx) )
		{	
			const FSkeletalMeshVertexInfluences& VertexInfluences = LODModel.VertexInfluences(InDynamicData->InfluenceIdx);
			if( VertexInfluences.Influences.Num() > 0 &&
				VertexInfluences.Influences.Num() == LODModel.NumVertices )
			{
				// Lock the buffer.
				const UINT Size = LODModel.NumVertices * sizeof(FVertexInfluence);
				FVertexInfluence* Buffer = (FVertexInfluence*)RHILockVertexBuffer(LOD.WeightsVertexBuffer.VertexBufferRHI,0,Size,FALSE);

				for( INT ChunkIdx=0; ChunkIdx < LODModel.Chunks.Num(); ChunkIdx++ )
				{
					const FSkelMeshChunk& Chunk = LODModel.Chunks(ChunkIdx);
					for( UINT VertIndex=Chunk.BaseVertexIndex; VertIndex < (Chunk.BaseVertexIndex + Chunk.GetNumVertices()); VertIndex++ )
					{
						// check default mesh vertex bones to see if they match up with any bone pairs
						const FGPUSkinVertexBase* Vertex = LODModel.VertexBufferGPUSkin.GetVertexPtr(VertIndex);
						UBOOL bMatchesBones=FALSE;
						for( INT BonePairIdx=0; BonePairIdx < InDynamicData->BonePairs.Num();  BonePairIdx++ )
						{
							const FBoneIndexPair& BonePair = InDynamicData->BonePairs(BonePairIdx);

							//assume match if invalid index (ie. no parent)
							UBOOL bCurBoneMatch[2] = 
							{	
								BonePair.BoneIdx[0]==INDEX_NONE && BonePair.BoneIdx[1]!=INDEX_NONE,	
								BonePair.BoneIdx[1]==INDEX_NONE && BonePair.BoneIdx[0]!=INDEX_NONE,	
							};

							// match bone 0
							for( INT Idx=0; Idx < MAX_INFLUENCES && !bCurBoneMatch[0]; Idx++ )
							{
								if( Vertex->InfluenceWeights[Idx] > 0 &&
									Chunk.BoneMap(Vertex->InfluenceBones[Idx]) == BonePair.BoneIdx[0] )
								{
									bCurBoneMatch[0] = TRUE;
									break;
								}
							}

							// match bone 1
							for( INT Idx=0; Idx < MAX_INFLUENCES && !bCurBoneMatch[1]; Idx++ )
							{
								if( Vertex->InfluenceWeights[Idx] > 0 &&
									Chunk.BoneMap(Vertex->InfluenceBones[Idx]) == BonePair.BoneIdx[1] )
								{
									bCurBoneMatch[1] = TRUE;
									break;
								}
							}

							// found if both bones are matched
							if( bCurBoneMatch[0] && bCurBoneMatch[1] )
							{
								bMatchesBones = TRUE;
								break;
							}
						}
						// if the vertex usage matched a bone pair then use the instanced vertex influence instead of the default
						if( bMatchesBones )
						{
							const FVertexInfluence& VertexInfluence = VertexInfluences.Influences(VertIndex);
							for( INT Idx=0; Idx < MAX_INFLUENCES; Idx++ )
							{
								Buffer[VertIndex].Weights.InfluenceWeights[Idx] = VertexInfluence.Weights.InfluenceWeights[Idx];
								Buffer[VertIndex].Bones.InfluenceBones[Idx] = VertexInfluence.Bones.InfluenceBones[Idx];
							}
						}
						else if( InDynamicData->bResetInfluences )
						{
							// reset to base weights
							const FGPUSkinVertexBase* Vertex = LODModel.VertexBufferGPUSkin.GetVertexPtr(VertIndex);
							for( INT Idx=0; Idx < MAX_INFLUENCES; Idx++ )
							{
								Buffer[VertIndex].Weights.InfluenceWeights[Idx] = Vertex->InfluenceWeights[Idx];
								Buffer[VertIndex].Bones.InfluenceBones[Idx] = Vertex->InfluenceBones[Idx];
							}
						}
					}
				}
				// Unlock the buffer.
				RHIUnlockVertexBuffer(LOD.WeightsVertexBuffer.VertexBufferRHI);
			}
		}
	}
}

/**
* Update the contents of the morph target vertex buffer by accumulating all 
* delta positions and delta normals from the set of active morph targets
* @param ActiveMorphs - morph targets to accumulate. assumed to be weighted and have valid targets
*/
void FSkeletalMeshObjectGPUSkin::FSkeletalMeshObjectLOD::UpdateMorphVertexBuffer( const TArray<FActiveMorph>& ActiveMorphs )
{
	if( IsValidRef(MorphVertexBuffer.VertexBufferRHI) )
	{
		// LOD of the skel mesh is used to find number of vertices in buffer
		FStaticLODModel& LodModel = SkelMesh->LODModels(LODIndex);
		UINT Size = LodModel.NumVertices * sizeof(FMorphGPUSkinVertex);

		// Lock the buffer.
		FMorphGPUSkinVertex* Buffer = (FMorphGPUSkinVertex*)RHILockVertexBuffer(MorphVertexBuffer.VertexBufferRHI,0,Size,FALSE);

		// zero all deltas (NOTE: DeltaTangentZ is FPackedNormal, so we can't just appMemzero)
		for (UINT VertIndex=0; VertIndex < LodModel.NumVertices; ++VertIndex)
		{
			Buffer[VertIndex].DeltaPosition = FVector(0,0,0);
			Buffer[VertIndex].DeltaTangentZ = FVector(0,0,0);
		}

		// iterate over all active morph targets and accumulate their vertex deltas
		for( INT MorphIdx=0; MorphIdx < ActiveMorphs.Num(); MorphIdx++ )
		{
			const FActiveMorph& Morph = ActiveMorphs(MorphIdx);
			checkSlow(Morph.Target && Morph.Target->MorphLODModels.IsValidIndex(LODIndex) && Morph.Target->MorphLODModels(LODIndex).Vertices.Num());
			checkSlow(Morph.Weight >= MinMorphBlendWeight && Morph.Weight <= MaxMorphBlendWeight);
			const FMorphTargetLODModel& MorphLODModel = Morph.Target->MorphLODModels(LODIndex);

			FLOAT ClampedMorphWeight = Min(Morph.Weight,1.0f);

			// iterate over the vertices that this lod model has changed
			for( INT MorphVertIdx=0; MorphVertIdx < MorphLODModel.Vertices.Num(); MorphVertIdx++ )
			{
				const FMorphTargetVertex& MorphVertex = MorphLODModel.Vertices(MorphVertIdx);
				checkSlow(MorphVertex.SourceIdx < LodModel.NumVertices);
				FMorphGPUSkinVertex& DestVertex = Buffer[MorphVertex.SourceIdx];

				DestVertex.DeltaPosition += MorphVertex.PositionDelta * Morph.Weight;
				DestVertex.DeltaTangentZ = FVector(DestVertex.DeltaTangentZ) + FVector(MorphVertex.TangentZDelta) * ClampedMorphWeight;
			}
		}

		// Unlock the buffer.
		RHIUnlockVertexBuffer(MorphVertexBuffer.VertexBufferRHI);
	}
}

/**
 * @param	LODIndex - each LOD has its own vertex data
 * @param	ChunkIdx - index for current mesh chunk
 * @return	vertex factory for rendering the LOD
 */
const FVertexFactory* FSkeletalMeshObjectGPUSkin::GetVertexFactory(INT LODIndex,INT ChunkIdx) const
{
	checkSlow( LODs.IsValidIndex(LODIndex) );
	checkSlow( DynamicData );

	if( DynamicData->NumWeightedActiveMorphs > 0 )
	{
		// use the morph enabled vertex factory if any active morphs are set
		return &LODs(LODIndex).MorphVertexFactories(ChunkIdx);
	}
	else if ( bUseLocalVertexFactory )
	{
		// use the local gpu vertex factory (when bForceRefpose is true)
		return LODs(LODIndex).LocalVertexFactory.GetOwnedPointer();
	}
	else
	{
		// use the default gpu skin vertex factory
		return &LODs(LODIndex).VertexFactories(ChunkIdx);
	}
}


/**
 * @param	LODIndex - each LOD has its own vertex data
 * @param	ChunkIdx - index for current mesh chunk
 * @return	Decal vertex factory for rendering the LOD
 */
FDecalVertexFactoryBase* FSkeletalMeshObjectGPUSkin::GetDecalVertexFactory(INT LODIndex,INT ChunkIdx,const FDecalInteraction* Decal)
{
	checkSlow( bDecalFactoriesEnabled );
	checkSlow( LODs.IsValidIndex(LODIndex) );

	if( DynamicData->NumWeightedActiveMorphs > 0 )
	{
		// use the morph enabled decal vertex factory if any active morphs are set
		return &LODs(LODIndex).MorphDecalVertexFactories(ChunkIdx);
	}
	else if ( LODs(LODIndex).LocalDecalVertexFactory )
	{
		// use the local decal vertex factory (when bForceRefpose is true)
		return LODs(LODIndex).LocalDecalVertexFactory.GetOwnedPointer();
	}
	else
	{
		// use the default gpu skin decal vertex factory
		return &LODs(LODIndex).DecalVertexFactories(ChunkIdx);
	}
}

/**
 * Get the shadow vertex factory for an LOD
 * @param	LODIndex - each LOD has its own vertex data
 * @return	Vertex factory for rendering the shadow volume for the LOD
 */
const FLocalShadowVertexFactory& FSkeletalMeshObjectGPUSkin::GetShadowVertexFactory( INT LODIndex ) const
{
	checkSlow( LODs.IsValidIndex(LODIndex) );
	return LODs(LODIndex).ShadowVertexBuffer.GetVertexFactory();
}

/** 
* Initialize the stream components common to all GPU skin vertex factory types 
*
* @param VertexFactoryData - context for setting the vertex factory stream components. commited later
* @param VertexBuffer - vertex buffer which contains the data and also stride info
* @param bUseInstancedVertexWeights - use instanced influence weights instead of default weights
*/
template<class VertexFactoryType>
void InitGPUSkinVertexFactoryComponents( typename VertexFactoryType::DataType* VertexFactoryData, 
										const FSkeletalMeshVertexBuffer* VertexBuffer,
										const FInflucenceWeightsVertexBuffer* WeightsVertexBuffer,
										UBOOL bUseInstancedVertexWeights )
{
	// position
	VertexFactoryData->PositionComponent = FVertexStreamComponent(
		VertexBuffer,STRUCT_OFFSET(FGPUSkinVertexBase,Position),VertexBuffer->GetStride(),VET_Float3);
	// tangents
	VertexFactoryData->TangentBasisComponents[0] = FVertexStreamComponent(
		VertexBuffer,STRUCT_OFFSET(FGPUSkinVertexBase,TangentX),VertexBuffer->GetStride(),VET_PackedNormal);
	VertexFactoryData->TangentBasisComponents[1] = FVertexStreamComponent(
		VertexBuffer,STRUCT_OFFSET(FGPUSkinVertexBase,TangentZ),VertexBuffer->GetStride(),VET_PackedNormal);
	
	if( !bUseInstancedVertexWeights )
	{
		// bone indices
		VertexFactoryData->BoneIndices = FVertexStreamComponent(
			VertexBuffer,STRUCT_OFFSET(FGPUSkinVertexBase,InfluenceBones),VertexBuffer->GetStride(),VET_UByte4);
		// bone weights
		VertexFactoryData->BoneWeights = FVertexStreamComponent(
			VertexBuffer,STRUCT_OFFSET(FGPUSkinVertexBase,InfluenceWeights),VertexBuffer->GetStride(),VET_UByte4N);
	}
	else
	{
		// use the weights/bones stored with this skeletal mesh object instance

		check(WeightsVertexBuffer);

		// instanced bones
		VertexFactoryData->BoneIndices = FVertexStreamComponent(
			WeightsVertexBuffer,STRUCT_OFFSET(FVertexInfluence,Bones.InfluenceBones),sizeof(FVertexInfluence),VET_UByte4);

		// instanced weights
		VertexFactoryData->BoneWeights = FVertexStreamComponent(
			WeightsVertexBuffer,STRUCT_OFFSET(FVertexInfluence,Weights),sizeof(FVertexInfluence),VET_UByte4N);
	}
	
	// uvs
	if( !VertexBuffer->GetUseFullPrecisionUVs() )
	{
		VertexFactoryData->TextureCoordinates.AddItem(FVertexStreamComponent(
			VertexBuffer,STRUCT_OFFSET(TGPUSkinVertexFloat16Uvs<MAX_TEXCOORDS>,UVs),VertexBuffer->GetStride(),VET_Half2));
	}
	else
	{	
		VertexFactoryData->TextureCoordinates.AddItem(FVertexStreamComponent(
			VertexBuffer,STRUCT_OFFSET(TGPUSkinVertexFloat32Uvs<MAX_TEXCOORDS>,UVs),VertexBuffer->GetStride(),VET_Float2));
	}
}

/** 
* Initialize the stream components for using local vertex factory with gpu skin vertex components
*
* @param VertexFactoryData - context for setting the vertex factory stream components. commited later
* @param VertexBuffer - vertex buffer which contains the data and also stride info
*/
template<class VertexFactoryType>
void InitLocalVertexFactoryComponents( typename VertexFactoryType::DataType* VertexFactoryData, const FSkeletalMeshVertexBuffer* VertexBuffer )
{
	// position
	VertexFactoryData->PositionComponent = FVertexStreamComponent(
		VertexBuffer,STRUCT_OFFSET(FGPUSkinVertexBase,Position),VertexBuffer->GetStride(),VET_Float3);
	// tangents
	VertexFactoryData->TangentBasisComponents[0] = FVertexStreamComponent(
		VertexBuffer,STRUCT_OFFSET(FGPUSkinVertexBase,TangentX),VertexBuffer->GetStride(),VET_PackedNormal);
	VertexFactoryData->TangentBasisComponents[1] = FVertexStreamComponent(
		VertexBuffer,STRUCT_OFFSET(FGPUSkinVertexBase,TangentZ),VertexBuffer->GetStride(),VET_PackedNormal);
	// uvs
	if( !VertexBuffer->GetUseFullPrecisionUVs() )
	{
		VertexFactoryData->TextureCoordinates.AddItem(FVertexStreamComponent(
			VertexBuffer,STRUCT_OFFSET(TGPUSkinVertexFloat16Uvs<MAX_TEXCOORDS>,UVs),VertexBuffer->GetStride(),VET_Half2));
	}
	else
	{	
		VertexFactoryData->TextureCoordinates.AddItem(FVertexStreamComponent(
			VertexBuffer,STRUCT_OFFSET(TGPUSkinVertexFloat32Uvs<MAX_TEXCOORDS>,UVs),VertexBuffer->GetStride(),VET_Float2));
	}
}

/** 
* Init rendering resources for this LOD 
* @param bUseLocalVertexFactory - use non-gpu skinned factory when rendering in ref pose
* @param bUseInstancedVertexWeights - use instanced influence weights instead of default weights
*/
void FSkeletalMeshObjectGPUSkin::FSkeletalMeshObjectLOD::InitResources(
	UBOOL bUseLocalVertexFactory, 
	UBOOL bUseInstancedVertexWeights
	)
{
	check(SkelMesh);
	check(SkelMesh->LODModels.IsValidIndex(LODIndex));

	// vertex buffer for each lod has already been created when skelmesh was loaded
	FStaticLODModel& LODModel = SkelMesh->LODModels(LODIndex);

	// initialize vertex buffer of weight/bone influences if needed
	if( bUseInstancedVertexWeights )
	{
		BeginInitResource(&WeightsVertexBuffer);
	}

	// one for each chunk
	VertexFactories.Empty();
	LocalVertexFactory.Reset();
	if (bUseLocalVertexFactory)
	{
		// only need one local vertex factory because this is no unique data per chunk when using the local factory.
		LocalVertexFactory.Reset(new FLocalVertexFactory());

		// update vertex factory components and sync it
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			InitGPUSkinLocalVertexFactory,
			FLocalVertexFactory*,VertexFactory,LocalVertexFactory.GetOwnedPointer(),
			FStaticLODModel*,LODModel,&LODModel,
			{
				FLocalVertexFactory::DataType Data;
				InitLocalVertexFactoryComponents<FLocalVertexFactory>(&Data,&LODModel->VertexBufferGPUSkin);
				VertexFactory->SetData(Data);
			});
		// init rendering resource	
		BeginInitResource(LocalVertexFactory.GetOwnedPointer());
	}
	else
	{
		VertexFactories.Empty(LODModel.Chunks.Num());
		for( INT FactoryIdx=0; FactoryIdx < LODModel.Chunks.Num(); FactoryIdx++ )
		{
			FGPUSkinVertexFactory* VertexFactory = new(VertexFactories) FGPUSkinVertexFactory( LODModel.Chunks(FactoryIdx).BoneMap.Num() );

			// update vertex factory components and sync it
			ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(
				InitGPUSkinVertexFactory,
				FGPUSkinVertexFactory*,VertexFactory,VertexFactory,
				FSkeletalMeshObjectLOD*,LOD,this,
				UBOOL,bUseInstancedVertexWeights,bUseInstancedVertexWeights,
				{
					FGPUSkinVertexFactory::DataType Data;
					// init default gpu skin components
					InitGPUSkinVertexFactoryComponents<FGPUSkinVertexFactory>(
						&Data,
						&LOD->SkelMesh->LODModels(LOD->LODIndex).VertexBufferGPUSkin,
						&LOD->WeightsVertexBuffer,
						bUseInstancedVertexWeights
						);
					VertexFactory->SetData(Data);
				});
			// init rendering resource	
			BeginInitResource(VertexFactory);
		}
	}

	// Decals
	if ( bDecalFactoriesEnabled )
	{
		DecalVertexFactories.Empty();
		LocalDecalVertexFactory.Reset();
		if (bUseLocalVertexFactory)
		{
			// only need one local vertex factory because this is no unique data per chunk when using the local factory.
			LocalDecalVertexFactory.Reset(new FLocalDecalVertexFactory());

			// update vertex factory components and sync it
			ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
					InitGPUSkinDecalVertexFactory,
					FLocalDecalVertexFactory*,DecalVertexFactory,LocalDecalVertexFactory.GetOwnedPointer(),
					FStaticLODModel*,LODModel,&LODModel,
				{
					FLocalDecalVertexFactory::DataType Data;
					// init default gpu skin components
					InitLocalVertexFactoryComponents<FLocalDecalVertexFactory>(&Data,&LODModel->VertexBufferGPUSkin);
					DecalVertexFactory->SetData(Data);
				});
			// init rendering resource	
			BeginInitResource(LocalDecalVertexFactory.GetOwnedPointer());
		}
		else
		{
			DecalVertexFactories.Empty(LODModel.Chunks.Num());
			for( INT FactoryIdx=0; FactoryIdx < LODModel.Chunks.Num(); FactoryIdx++ )
			{
				FGPUSkinDecalVertexFactory* DecalVertexFactory = new(DecalVertexFactories) FGPUSkinDecalVertexFactory( LODModel.Chunks(FactoryIdx).BoneMap.Num() );

				// update vertex factory components and sync it
				ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(
					InitGPUSkinDecalVertexFactory,
					FGPUSkinDecalVertexFactory*,DecalVertexFactory,DecalVertexFactory,
					FSkeletalMeshObjectLOD*,LOD,this,
					UBOOL,bUseInstancedVertexWeights,bUseInstancedVertexWeights,
					{
						FGPUSkinDecalVertexFactory::DataType Data;
						// init default gpu skin components
						InitGPUSkinVertexFactoryComponents<FGPUSkinDecalVertexFactory>(
							&Data,
							&LOD->SkelMesh->LODModels(LOD->LODIndex).VertexBufferGPUSkin,
							&LOD->WeightsVertexBuffer,
							bUseInstancedVertexWeights
							);
						DecalVertexFactory->SetData(Data);
					});
				// init rendering resource	
				BeginInitResource(DecalVertexFactory);
			}
		}
	}
	// End Decals

	if( UEngine::ShadowVolumesAllowed() )
	{
		// Initialize the shadow vertex buffer and its factory.
		BeginInitResource(&ShadowVertexBuffer);
	}
}

/** 
 * Release rendering resources for this LOD 
 */
void FSkeletalMeshObjectGPUSkin::FSkeletalMeshObjectLOD::ReleaseResources()
{	
	for( INT FactoryIdx=0; FactoryIdx < VertexFactories.Num(); FactoryIdx++ )
	{
		FGPUSkinVertexFactory& VertexFactory = VertexFactories(FactoryIdx);
		BeginReleaseResource(&VertexFactory);
	}
	if (LocalVertexFactory)
	{
		BeginReleaseResource(LocalVertexFactory.GetOwnedPointer());
	}

	// Decals
	for( INT FactoryIdx=0; FactoryIdx < DecalVertexFactories.Num(); FactoryIdx++ )
	{
		FGPUSkinDecalVertexFactory& DecalVertexFactory = DecalVertexFactories(FactoryIdx);
		BeginReleaseResource(&DecalVertexFactory);
	}
	if (LocalDecalVertexFactory)
	{
		BeginReleaseResource(LocalDecalVertexFactory.GetOwnedPointer());
	}
	// End Decals

	// Release the shadow vertex buffer and its factory.
	BeginReleaseResource(&ShadowVertexBuffer);

	// Release the influence weight vertex buffer
	BeginReleaseResource(&WeightsVertexBuffer);
}

/** 
* Init rendering resources for the morph stream of this LOD
* @param bUseInstancedVertexWeights - use instanced influence weights instead of default weights
*/
void FSkeletalMeshObjectGPUSkin::FSkeletalMeshObjectLOD::InitMorphResources(UBOOL bUseInstancedVertexWeights)
{
	check(SkelMesh);
	check(SkelMesh->LODModels.IsValidIndex(LODIndex));

	// vertex buffer for each lod has already been created when skelmesh was loaded
	FStaticLODModel& LODModel = SkelMesh->LODModels(LODIndex);

	// init the delta vertex buffer for this LOD
	BeginInitResource(&MorphVertexBuffer);

	// one for each chunk
	MorphVertexFactories.Empty(LODModel.Chunks.Num());
	for( INT FactoryIdx=0; FactoryIdx < LODModel.Chunks.Num(); FactoryIdx++ )
	{
		FGPUSkinMorphVertexFactory* MorphVertexFactory = new(MorphVertexFactories) FGPUSkinMorphVertexFactory( LODModel.Chunks(FactoryIdx).BoneMap.Num() );

		// update vertex factory components and sync it
		ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(
			InitGPUSkinMorphVertexFactory,
			FGPUSkinMorphVertexFactory*,VertexFactory,MorphVertexFactory,
			FSkeletalMeshObjectLOD*,LOD,this,
			UBOOL,bUseInstancedVertexWeights,bUseInstancedVertexWeights,
			{
				FGPUSkinMorphVertexFactory::DataType Data;
				// init default gpu skin components
				InitGPUSkinVertexFactoryComponents<FGPUSkinMorphVertexFactory>(
					&Data,
					&LOD->SkelMesh->LODModels(LOD->LODIndex).VertexBufferGPUSkin,
					&LOD->WeightsVertexBuffer,
					bUseInstancedVertexWeights
					);
				// delta positions
				Data.DeltaPositionComponent = FVertexStreamComponent(
					&LOD->MorphVertexBuffer,STRUCT_OFFSET(FMorphGPUSkinVertex,DeltaPosition),sizeof(FMorphGPUSkinVertex),VET_Float3);
				// delta normals
				Data.DeltaTangentZComponent = FVertexStreamComponent(
					&LOD->MorphVertexBuffer,STRUCT_OFFSET(FMorphGPUSkinVertex,DeltaTangentZ),sizeof(FMorphGPUSkinVertex),VET_PackedNormal);
				VertexFactory->SetData(Data);
			});
		// init rendering resource	
		BeginInitResource(MorphVertexFactory);
	}

	// Decals
	if ( bDecalFactoriesEnabled )
	{
		MorphDecalVertexFactories.Empty(LODModel.Chunks.Num());
		for( INT FactoryIdx=0; FactoryIdx < LODModel.Chunks.Num(); FactoryIdx++ )
		{
			FGPUSkinMorphDecalVertexFactory* MorphDecalVertexFactory = new(MorphDecalVertexFactories) FGPUSkinMorphDecalVertexFactory( LODModel.Chunks(FactoryIdx).BoneMap.Num() );

			// update vertex factory components and sync it
			ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(
				InitGPUSkinMorphDecalVertexFactory,
				FGPUSkinMorphDecalVertexFactory*,VertexFactory,MorphDecalVertexFactory,
				FSkeletalMeshObjectLOD*,LOD,this,
				UBOOL,bUseInstancedVertexWeights,bUseInstancedVertexWeights,
				{
					FGPUSkinMorphDecalVertexFactory::DataType Data;
					// init default gpu skin components
					InitGPUSkinVertexFactoryComponents<FGPUSkinMorphDecalVertexFactory>(
						&Data,
						&LOD->SkelMesh->LODModels(LOD->LODIndex).VertexBufferGPUSkin,
						&LOD->WeightsVertexBuffer,
						bUseInstancedVertexWeights
						);
					// delta positions
					Data.DeltaPositionComponent = FVertexStreamComponent(
						&LOD->MorphVertexBuffer,STRUCT_OFFSET(FMorphGPUSkinVertex,DeltaPosition),sizeof(FMorphGPUSkinVertex),VET_Float3);
					// delta normals
					Data.DeltaTangentZComponent = FVertexStreamComponent(
						&LOD->MorphVertexBuffer,STRUCT_OFFSET(FMorphGPUSkinVertex,DeltaTangentZ),sizeof(FMorphGPUSkinVertex),VET_PackedNormal);
					VertexFactory->SetData(Data);
				});
			// init rendering resource	
			BeginInitResource(MorphDecalVertexFactory);
		}
	}
	// End Decals
}

/** 
* Release rendering resources for the morph stream of this LOD
*/
void FSkeletalMeshObjectGPUSkin::FSkeletalMeshObjectLOD::ReleaseMorphResources()
{
	for( INT FactoryIdx=0; FactoryIdx < MorphVertexFactories.Num(); FactoryIdx++ )
	{
		FGPUSkinMorphVertexFactory& MorphVertexFactory = MorphVertexFactories(FactoryIdx);
		BeginReleaseResource(&MorphVertexFactory);
	}

	// Decals
	for( INT FactoryIdx=0; FactoryIdx < MorphDecalVertexFactories.Num(); FactoryIdx++ )
	{
		FGPUSkinMorphDecalVertexFactory& MorphDecalVertexFactory = MorphDecalVertexFactories(FactoryIdx);
		BeginReleaseResource(&MorphDecalVertexFactory);
	}
	// End Decals

	// release the delta vertex buffer
	BeginReleaseResource(&MorphVertexBuffer);
}

/** 
 * Update the contents of the vertex buffer with new data
 * @param	NewVertices - array of new vertex data
 * @param	NumVertices - Number of vertices
 */
void FSkeletalMeshObjectGPUSkin::FSkeletalMeshObjectLOD::UpdateShadowVertexBuffer( const FVector* NewVertices, UINT NumVertices ) const
{
	ShadowVertexBuffer.UpdateVertices( NewVertices, NumVertices, sizeof(FVector) );
}

/**
 * Compute the distance of a light from the triangle planes.
 */
void FSkeletalMeshObjectGPUSkin::GetPlaneDots(FLOAT* OutPlaneDots,const FVector4& LightPosition,INT LODIndex) const
{
	const FStaticLODModel& LODModel = SkeletalMesh->LODModels(LODIndex);
#if XBOX
	// Use VMX optimized version to calculate the plane dots
	GetPlaneDotsXbox(LODModel.ShadowIndices.Num() / 3,LightPosition,
		(FVector*)CachedVertices.GetData(),
		(WORD*)LODModel.ShadowIndices.GetData(),OutPlaneDots);
#else
	for(INT TriangleIndex = 0;TriangleIndex < LODModel.ShadowIndices.Num() / 3;TriangleIndex++)
	{
		const FVector& V1 = CachedVertices(LODModel.ShadowIndices(TriangleIndex * 3 + 0));
		const FVector& V2 = CachedVertices(LODModel.ShadowIndices(TriangleIndex * 3 + 1));
		const FVector& V3 = CachedVertices(LODModel.ShadowIndices(TriangleIndex * 3 + 2));
		*OutPlaneDots++ = ((V2-V3) ^ (V1-V3)) | (FVector(LightPosition) - V1 * LightPosition.W);
	}
#endif
}

/**
 * Re-skin cached vertices for an LOD and update the vertex buffer. Note that this
 * function is called from the render thread!
 * @param	LODIndex - index to LODs
 * @param	bForce - force update even if LOD index hasn't changed
 * @param	bUpdateShadowVertices - whether to update the shadow volumes vertices
 * @param	bUpdateDecalVertices - whether to update the decal vertices
 */
void FSkeletalMeshObjectGPUSkin::CacheVertices(INT LODIndex, UBOOL bForce, UBOOL bUpdateShadowVertices, UBOOL bUpdateDecalVertices) const
{
	// Get the destination mesh LOD.
	const FSkeletalMeshObjectLOD& MeshLOD = LODs(LODIndex);

	// source skel mesh and static lod model
	FStaticLODModel& LOD = SkeletalMesh->LODModels(LODIndex);

	// only recache if lod changed and we need it for a shadow volume
	if ( (LODIndex != CachedShadowLOD || bForce) && 
		bUpdateShadowVertices &&
		DynamicData && 
		IsValidRef(MeshLOD.ShadowVertexBuffer.VertexBufferRHI) )
	{
		// bone matrices
		const TArray<FMatrix>& ReferenceToLocal = DynamicData->ReferenceToLocal;

		// final cached verts
		CachedVertices.Empty(LOD.NumVertices);
		CachedVertices.Add(LOD.NumVertices);
		FVector* DestVertex = &CachedVertices(0);

		// Do the actual skinning
		//@todo: skin straight to the shadow vertex buffer?
		SkinVertices( DestVertex, ReferenceToLocal, LOD );

		// copy to the shadow vertex buffer
		check(LOD.NumVertices == CachedVertices.Num());
		MeshLOD.UpdateShadowVertexBuffer( &CachedVertices(0), LOD.NumVertices );

		CachedShadowLOD = LODIndex;
	}
}

/** optimized skinning */
FORCEINLINE void FSkeletalMeshObjectGPUSkin::SkinVertices( FVector* DestVertex, const TArray<FMatrix>& ReferenceToLocal, FStaticLODModel& LOD ) const
{
	//@todo sz - handle morph targets when updating vertices for shadow volumes

	DWORD StatusRegister = VectorGetControlRegister();
	VectorSetControlRegister( StatusRegister | VECTOR_ROUND_TOWARD_ZERO );

	// For each chunk in the LOD
	for (INT ChunkIndex = 0; ChunkIndex < LOD.Chunks.Num(); ChunkIndex++)
	{
		const FSkelMeshChunk& Chunk = LOD.Chunks(ChunkIndex);
		// For reading the verts
		FGPUSkinVertexBase* RESTRICT SrcRigidVertex = (Chunk.GetNumRigidVertices() > 0) ? LOD.VertexBufferGPUSkin.GetVertexPtr(Chunk.GetRigidVertexBufferIndex()) : NULL;
		// For each rigid vert transform from reference pose to local space
		for (INT VertexIndex = 0; VertexIndex < Chunk.GetNumRigidVertices(); VertexIndex++)
		{
			const FMatrix& RefToLocal = ReferenceToLocal(Chunk.BoneMap(SrcRigidVertex->InfluenceBones[RIGID_INFLUENCE_INDEX]));
			// Load VMX registers
			VectorRegister Position = VectorLoadFloat3( &SrcRigidVertex->Position );
			VectorRegister M3 = VectorLoadAligned( &RefToLocal.M[3][0] );
			VectorRegister M2 = VectorLoadAligned( &RefToLocal.M[2][0] );
			VectorRegister M1 = VectorLoadAligned( &RefToLocal.M[1][0] );
			VectorRegister M0 = VectorLoadAligned( &RefToLocal.M[0][0] );
			// Move to next source
			SrcRigidVertex = (FGPUSkinVertexBase*)( ((BYTE*)SrcRigidVertex) + LOD.VertexBufferGPUSkin.GetStride() );
			// Splat position (transformed position starts as Z splat)
			VectorRegister TransformedPosition = VectorReplicate(Position, 2);
			VectorRegister SplatY = VectorReplicate(Position, 1);
			VectorRegister SplatX = VectorReplicate(Position, 0);
			// Now transform
			TransformedPosition = VectorMultiplyAdd(M2,TransformedPosition,M3);
			TransformedPosition = VectorMultiplyAdd(SplatY,M1,TransformedPosition);
			TransformedPosition = VectorMultiplyAdd(SplatX,M0,TransformedPosition);
			// Write out 3 part vector
			VectorStoreFloat3( TransformedPosition, DestVertex );
			// Move to next dest
			DestVertex++;
		}
		// Source vertices for transformation
		FGPUSkinVertexBase* RESTRICT SrcSoftVertex = (Chunk.GetNumSoftVertices() > 0) ? LOD.VertexBufferGPUSkin.GetVertexPtr(Chunk.GetSoftVertexBufferIndex()) : NULL;
		// Transform each skinned vert
		for (INT VertexIndex = 0; VertexIndex < Chunk.GetNumSoftVertices(); VertexIndex++)
		{
			// Load VMX registers
			VectorRegister Position = VectorLoadFloat3( &SrcSoftVertex->Position );
			VectorRegister WeightVector = VectorMultiply( VectorLoadByte4(SrcSoftVertex->InfluenceWeights), VECTOR_INV_255 );
			VectorResetFloatRegisters();	// Must call this after VectorLoadByte4 in order to use scalar floats again.

			// Zero the out going position
			VectorRegister TransformedPosition = VectorZero();
			// Influence 0
			{
				// Get the matrix for this influence
				const FMatrix& RefToLocal = ReferenceToLocal(Chunk.BoneMap(SrcSoftVertex->InfluenceBones[0]));
				// Load the matrix
				VectorRegister M3 = VectorLoadAligned(&RefToLocal.M[3][0]);
				VectorRegister M2 = VectorLoadAligned(&RefToLocal.M[2][0]);
				VectorRegister M1 = VectorLoadAligned(&RefToLocal.M[1][0]);
				VectorRegister M0 = VectorLoadAligned(&RefToLocal.M[0][0]);
				// Splat position (influenced position starts as Z splat)
				VectorRegister InflPosition = VectorReplicate(Position, 2);
				VectorRegister SplatY = VectorReplicate(Position, 1);
				VectorRegister SplatX = VectorReplicate(Position, 0);
				// Splat the weight
				VectorRegister Weight = VectorReplicate(WeightVector, 0);
				// Now transform
				InflPosition = VectorMultiplyAdd(M2,InflPosition,M3);
				InflPosition = VectorMultiplyAdd(SplatY,M1,InflPosition);
				InflPosition = VectorMultiplyAdd(SplatX,M0,InflPosition);
				// Multipy by the weight and accumulate the results
				TransformedPosition = VectorMultiplyAdd(Weight,InflPosition,TransformedPosition);
			}
			// Influence 1
			{
				// Get the matrix for this influence
				const FMatrix& RefToLocal = ReferenceToLocal(Chunk.BoneMap(SrcSoftVertex->InfluenceBones[1]));
				// Load the matrix
				VectorRegister M3 = VectorLoadAligned(&RefToLocal.M[3][0]);
				VectorRegister M2 = VectorLoadAligned(&RefToLocal.M[2][0]);
				VectorRegister M1 = VectorLoadAligned(&RefToLocal.M[1][0]);
				VectorRegister M0 = VectorLoadAligned(&RefToLocal.M[0][0]);
				// Splat position (influenced position starts as Z splat)
				VectorRegister InflPosition = VectorReplicate(Position, 2);
				VectorRegister SplatY = VectorReplicate(Position, 1);
				VectorRegister SplatX = VectorReplicate(Position, 0);
				// Splat the weight
				VectorRegister Weight = VectorReplicate(WeightVector, 1);
				// Now transform
				InflPosition = VectorMultiplyAdd(M2,InflPosition,M3);
				InflPosition = VectorMultiplyAdd(SplatY,M1,InflPosition);
				InflPosition = VectorMultiplyAdd(SplatX,M0,InflPosition);
				// Multipy by the weight and accumulate the results
				TransformedPosition = VectorMultiplyAdd(Weight,InflPosition,TransformedPosition);
			}
			// Influence 2
			{
				// Get the matrix for this influence
				const FMatrix& RefToLocal = ReferenceToLocal(Chunk.BoneMap(SrcSoftVertex->InfluenceBones[2]));
				// Load the matrix
				VectorRegister M3 = VectorLoadAligned(&RefToLocal.M[3][0]);
				VectorRegister M2 = VectorLoadAligned(&RefToLocal.M[2][0]);
				VectorRegister M1 = VectorLoadAligned(&RefToLocal.M[1][0]);
				VectorRegister M0 = VectorLoadAligned(&RefToLocal.M[0][0]);
				// Splat position (influenced position starts as Z splat)
				VectorRegister InflPosition = VectorReplicate(Position, 2);
				VectorRegister SplatY = VectorReplicate(Position, 1);
				VectorRegister SplatX = VectorReplicate(Position, 0);
				// Splat the weight
				VectorRegister Weight = VectorReplicate(WeightVector, 2);
				// Now transform
				InflPosition = VectorMultiplyAdd(M2,InflPosition,M3);
				InflPosition = VectorMultiplyAdd(SplatY,M1,InflPosition);
				InflPosition = VectorMultiplyAdd(SplatX,M0,InflPosition);
				// Multipy by the weight and accumulate the results
				TransformedPosition = VectorMultiplyAdd(Weight,InflPosition,TransformedPosition);
			}
			// Influence 3
			{
				// Get the matrix for this influence
				const FMatrix& RefToLocal = ReferenceToLocal(Chunk.BoneMap(SrcSoftVertex->InfluenceBones[3]));
				// Load the matrix
				VectorRegister M3 = VectorLoadAligned(&RefToLocal.M[3][0]);
				VectorRegister M2 = VectorLoadAligned(&RefToLocal.M[2][0]);
				VectorRegister M1 = VectorLoadAligned(&RefToLocal.M[1][0]);
				VectorRegister M0 = VectorLoadAligned(&RefToLocal.M[0][0]);
				// Splat position (influenced position starts as Z splat)
				VectorRegister InflPosition = VectorReplicate(Position, 2);
				VectorRegister SplatY = VectorReplicate(Position, 1);
				VectorRegister SplatX = VectorReplicate(Position, 0);
				// Splat the weight
				VectorRegister Weight = VectorReplicate(WeightVector, 3);
				// Now transform
				InflPosition = VectorMultiplyAdd(M2,InflPosition,M3);
				InflPosition = VectorMultiplyAdd(SplatY,M1,InflPosition);
				InflPosition = VectorMultiplyAdd(SplatX,M0,InflPosition);
				// Multipy by the weight and accumulate the results
				TransformedPosition = VectorMultiplyAdd(Weight,InflPosition,TransformedPosition);
			}
			// Move to next source vert
			SrcSoftVertex = (FGPUSkinVertexBase*)( ((BYTE*)SrcSoftVertex) + LOD.VertexBufferGPUSkin.GetStride() );
			// Write out 3 part vector
			VectorStoreFloat3( TransformedPosition, DestVertex );
			// Move to next dest
			DestVertex++;
		}
	}
	VectorSetControlRegister( StatusRegister );
}

/** 
 *	Get the array of component-space bone transforms. 
 *	Not safe to hold this point between frames, because it exists in dynamic data passed from main thread.
 */
TArray<FMatrix>* FSkeletalMeshObjectGPUSkin::GetSpaceBases() const
{
#if !FINAL_RELEASE
	if(DynamicData)
	{
		return &(DynamicData->MeshSpaceBases);
	}
	else
#endif
	{
		return NULL;
	}
}

TArray<FVector>* FSkeletalMeshObjectGPUSkin::GetSoftBodyTetraPosData() const
{
	// Soft-Bodies only work with CPU skinning currently.
	return NULL;	
}

/**
 * Allows derived types to transform decal state into a space that's appropriate for the given skinning algorithm.
 */
void FSkeletalMeshObjectGPUSkin::TransformDecalState(const FDecalState& DecalState,
													 FMatrix& OutDecalMatrix,
													 FVector& OutDecalLocation,
													 FVector2D& OutDecalOffset,
													 FMatrix& OutDecalRefToLocal)
{
	// The decal is already in the 'reference pose' space; just pass values along.
	OutDecalMatrix = DecalState.WorldTexCoordMtx;
	OutDecalLocation = DecalState.HitLocation;
	OutDecalOffset = FVector2D(DecalState.OffsetX, DecalState.OffsetY);
	
	if( DecalState.HitBoneIndex != INDEX_NONE  && DynamicData )
	{
		if( DynamicData->ReferenceToLocal.IsValidIndex(DecalState.HitBoneIndex) )
		{
			OutDecalRefToLocal = DynamicData->ReferenceToLocal(DecalState.HitBoneIndex);
		}
		else
		{
//			debugf(NAME_Warning,TEXT("Invalid bone index specified for decal: HitBoneIndex=%d"),DecalState.HitBoneIndex);
			OutDecalRefToLocal = FMatrix::Identity;
		}
	}
	else
	{
		OutDecalRefToLocal = FMatrix::Identity; 	// default
	}
}

/*-----------------------------------------------------------------------------
FDynamicSkelMeshObjectDataGPUSkin
-----------------------------------------------------------------------------*/

/**
* Constructor
* Updates the ReferenceToLocal matrices using the new dynamic data.
* @param	InSkelMeshComponent - parent skel mesh component
* @param	InLODIndex - each lod has its own bone map 
* @param	InActiveMorphs - morph targets active for the mesh
*/
FDynamicSkelMeshObjectDataGPUSkin::FDynamicSkelMeshObjectDataGPUSkin(
	USkeletalMeshComponent* InSkelMeshComponent,
	INT InLODIndex,
	const TArray<FActiveMorph>& InActiveMorphs
	)
	:	LODIndex(InLODIndex)
	,	ActiveMorphs(InActiveMorphs)
	,	NumWeightedActiveMorphs(0)
{
	UpdateRefToLocalMatrices( ReferenceToLocal, InSkelMeshComponent, LODIndex );

#if !FINAL_RELEASE
	MeshSpaceBases = InSkelMeshComponent->SpaceBases;
#endif

	// find number of morphs that are currently weighted and will affect the mesh
	for( INT MorphIdx=ActiveMorphs.Num()-1; MorphIdx >= 0; MorphIdx-- )
	{
		const FActiveMorph& Morph = ActiveMorphs(MorphIdx);
		if( Morph.Weight >= MinMorphBlendWeight &&
			Morph.Weight <= MaxMorphBlendWeight &&
			Morph.Target &&
			Morph.Target->MorphLODModels.IsValidIndex(LODIndex) &&
			Morph.Target->MorphLODModels(LODIndex).Vertices.Num() )
		{
			NumWeightedActiveMorphs++;
		}
		else
		{
			ActiveMorphs.Remove(MorphIdx);
		}
	}
}

/**
* Compare the given set of active morph targets with the current list to check if different
* @param CompareActiveMorphs - array of morph targets to compare
* @return TRUE if boths sets of active morph targets are equal
*/
UBOOL FDynamicSkelMeshObjectDataGPUSkin::ActiveMorphTargetsEqual( const TArray<FActiveMorph>& CompareActiveMorphs )
{
	UBOOL Result=TRUE;
	if( CompareActiveMorphs.Num() == ActiveMorphs.Num() )
	{
		for( INT MorphIdx=0; MorphIdx < ActiveMorphs.Num(); MorphIdx++ )
		{
			const FActiveMorph& Morph = ActiveMorphs(MorphIdx);
			const FActiveMorph& CompMorph = CompareActiveMorphs(MorphIdx);
			const FLOAT MorphWeightThreshold = 0.001f;

			if( Morph.Target != CompMorph.Target ||
				Abs(Morph.Weight - CompMorph.Weight) >= MorphWeightThreshold )
			{
				Result=FALSE;
				break;
			}
		}
	}
	else
	{
		Result = FALSE;
	}
	return Result;
}



