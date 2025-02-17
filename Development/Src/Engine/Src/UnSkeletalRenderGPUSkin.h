/*=============================================================================
	UnSkeletalRenderGPUSkin.h: GPU skinned mesh object and resource definitions
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UNSKELETALRENDERGPUSKIN_H__
#define __UNSKELETALRENDERGPUSKIN_H__

#include "UnSkeletalRender.h"
#include "GPUSkinVertexFactory.h"
#include "LocalDecalVertexFactory.h"

/** 
* Stores the updated matrices needed to skin the verts.
* Created by the game thread and sent to the rendering thread as an update 
*/
class FDynamicSkelMeshObjectDataGPUSkin : public FDynamicSkelMeshObjectData
{
public:

	/**
	* Constructor
	* Updates the ReferenceToLocal matrices using the new dynamic data.
	* @param	InSkelMeshComponent - parent skel mesh component
	* @param	InLODIndex - each lod has its own bone map 
	* @param	InActiveMorphs - morph targets active for the mesh
	*/
	FDynamicSkelMeshObjectDataGPUSkin(
		USkeletalMeshComponent* InSkelMeshComponent,
		INT InLODIndex,
		const TArray<FActiveMorph>& InActiveMorphs
		);

	/** ref pose to local space transforms */
	TArray<FMatrix> ReferenceToLocal;
#if !FINAL_RELEASE
	/** component space bone transforms*/
	TArray<FMatrix> MeshSpaceBases;
#endif
	/** currently LOD for bones being updated */
	INT LODIndex;
	/** current morph targets active on this mesh */
	TArray<FActiveMorph> ActiveMorphs;
	/** number of active morphs with weights > 0 */
	INT NumWeightedActiveMorphs;

	/**
	* Compare the given set of active morph targets with the current list to check if different
	* @param CompareActiveMorphs - array of morph targets to compare
	* @return TRUE if boths sets of active morph targets are equal
	*/
	UBOOL ActiveMorphTargetsEqual( const TArray<FActiveMorph>& CompareActiveMorphs );
};

/** morph target mesh data for a single vertex delta */
struct FMorphGPUSkinVertex
{
	FVector			DeltaPosition;
	FPackedNormal	DeltaTangentZ;
};

/**
* Morph target vertices which have been combined into single position/tangentZ deltas
*/
class FMorphVertexBuffer : public FVertexBuffer
{
public:

	/** 
	* Constructor
	* @param	InSkelMesh - parent mesh containing the static model data for each LOD
	* @param	InLODIdx - index of LOD model to use from the parent mesh
	*/
	FMorphVertexBuffer(USkeletalMesh* InSkelMesh, INT InLODIdx)
		:	LODIdx(InLODIdx)
		,	SkelMesh(InSkelMesh)
	{
		check(SkelMesh);
		check(SkelMesh->LODModels.IsValidIndex(LODIdx));
	}
	/** 
	* Initialize the dynamic RHI for this rendering resource 
	*/
	virtual void InitDynamicRHI();

	/** 
	* Release the dynamic RHI for this rendering resource 
	*/
	virtual void ReleaseDynamicRHI();

	/** 
	* Morph target vertex name 
	*/
	virtual FString GetFriendlyName() const { return TEXT("Morph target mesh vertices"); }

private:
	/** index to the SkelMesh.LODModels */
	INT	LODIdx;
	/** parent mesh containing the source data */
	USkeletalMesh* SkelMesh;
};

/** Vertex buffer that stores instanced bone influence weights */
class FInflucenceWeightsVertexBuffer : public FVertexBuffer
{
public:

	/** 
	* Constructor
	* @param	InSkelMesh - parent mesh containing the static model data for each LOD
	* @param	InLODIdx - index of LOD model to use from the parent mesh
	*/
	FInflucenceWeightsVertexBuffer(USkeletalMesh* InSkelMesh, INT InLODIdx)
		:	LODIdx(InLODIdx)
		,	SkelMesh(InSkelMesh)
	{
		check(SkelMesh);
		check(SkelMesh->LODModels.IsValidIndex(LODIdx));
	}
	/** 
	* Initialize the dynamic RHI for this rendering resource 
	*/
	virtual void InitDynamicRHI();

	/** 
	* Release the dynamic RHI for this rendering resource 
	*/
	virtual void ReleaseDynamicRHI();

	/** 
	* Morph target vertex name 
	*/
	virtual FString GetFriendlyName() const { return TEXT("Instance skel mesh weight vertices"); }

private:
	/** index to the SkelMesh.LODModels */
	INT	LODIdx;
	/** parent mesh containing the source data */
	USkeletalMesh* SkelMesh;
};
	
/**
 * Render data for a GPU skinned mesh
 */
class FSkeletalMeshObjectGPUSkin : public FSkeletalMeshObject
{
public:

	/** 
	 * Constructor
	 * @param	InSkeletalMeshComponent - skeletal mesh primitive we want to render 
	 */
	FSkeletalMeshObjectGPUSkin(USkeletalMeshComponent* InSkeletalMeshComponent);

	/** 
	 * Destructor 
	 */
	virtual ~FSkeletalMeshObjectGPUSkin();

	/** 
	 * Initialize rendering resources for each LOD 
	 */
	virtual void InitResources();

	/** 
	 * Release rendering resources for each LOD. 
	 */
	virtual void ReleaseResources();

	/**
	* Called by the game thread for any dynamic data updates for this skel mesh object
	* @param	LODIndex - lod level to update
	* @param	InSkeletalMeshComponen - parent prim component doing the updating
	* @param	ActiveMorphs - morph targets to blend with during skinning
	*/
	virtual void Update(INT LODIndex,USkeletalMeshComponent* InSkeletalMeshComponent,const TArray<FActiveMorph>& ActiveMorphs);

	/**
	* Called by the rendering thread to update the current dynamic data
	* @param	InDynamicData - data that was created by the game thread for use by the rendering thread
	*/
	virtual void UpdateDynamicData_RenderThread(FDynamicSkelMeshObjectData* InDynamicData);

	/** 
	* Called by the game thread to toggle usage for the instanced vertex weights.
	* @parm bEnabled - TRUE to enable the usage of influence weights
	*/
	virtual void ToggleVertexInfluences(UBOOL bEnabled);

	/**
	* Called by the game thread to update the instanced vertex weights
	* @param BonePairs - set of bone pairs used to find vertices that need to have their weights updated
	* @param InfluenceIdx - index of skeletal mesh vertex influence entry to use (INDEX_NONE for defaults from base verts)
	* @param bResetInfluences - resets the array of instanced influences using the ones from the base mesh before updating
	*/
	virtual void UpdateVertexInfluences(
		const TArray<FBoneIndexPair>& BonePairs,
		INT InfluenceIdx,
		UBOOL bResetInfluences);

	/**
	* Called by the rendering thread to update the current dynamic weight data
	* @param	InDynamicData - data that was created by the game thread for use by the rendering thread
	*/
	virtual void UpdateVertexInfluences_RenderThread(FDynamicUpdateVertexInfluencesData* InDynamicData);

	/**
	 * @param	LODIndex - each LOD has its own vertex data
	 * @param	ChunkIdx - index for current mesh chunk
	 * @return	vertex factory for rendering the LOD
	 */
	virtual const FVertexFactory* GetVertexFactory(INT LODIndex,INT ChunkIdx) const;

	/**
	 * @param	LODIndex - each LOD has its own vertex data
	 * @param	ChunkIdx - index for current mesh chunk
	 * @return	vertex factory for rendering the LOD
	 */
	virtual FDecalVertexFactoryBase* GetDecalVertexFactory(INT LODIndex,INT ChunkIdx,const FDecalInteraction* Decal);

	/**
	 * Get the shadow vertex factory for an LOD
	 * @param	LODIndex - each LOD has its own vertex data
	 * @return	Vertex factory for rendering the shadow volume for the LOD
	 */
	virtual const FLocalShadowVertexFactory& GetShadowVertexFactory( INT LODIndex ) const;

	/**
	 * Compute the distance of a light from the triangle planes.
	 */
	virtual void GetPlaneDots(FLOAT* OutPlaneDots,const FVector4& LightPosition,INT LODIndex) const;

	/**
	 * Re-skin cached vertices for an LOD and update the shadow vertex buffer. Note that this
	 * function is called from the render thread!
	 * @param	LODIndex - index to LODs
	 * @param	bForce - force update even if LOD index hasn't changed
	 * @param	bUpdateShadowVertices - whether to update the shadow volumes vertices
	 * @param	bUpdateDecalVertices - whether to update the decal vertices
	 */
	virtual void CacheVertices(INT LODIndex, UBOOL bForce, UBOOL bUpdateShadowVertices, UBOOL bUpdateDecalVertices) const;

	/** 
	 *	Get the array of component-space bone transforms. 
	 *	Not safe to hold this point between frames, because it exists in dynamic data passed from main thread.
	 */
	virtual TArray<FMatrix>* GetSpaceBases() const;

	/** Get the LOD to render this mesh at. */
	virtual INT GetLOD() const
	{
		if(DynamicData)
		{
			return DynamicData->LODIndex;
		}
		else
		{
			return 0;
		}
	}

	/** 
	 *	Get the array of tetrahedron vertex positions. 
	 *	Not safe to hold this point between frames, because it exists in dynamic data passed from main thread.
	 */
	virtual TArray<FVector>* GetSoftBodyTetraPosData() const;

	/** 
	* @return TRUE if the primitive component can render decals
	*/
	virtual UBOOL SupportsDecalRendering() const
	{
		return TRUE;
	}

	/**
	 * Allows derived types to transform decal state into a space that's appropriate for the given skinning algorithm.
	 */
	virtual void TransformDecalState(const FDecalState& DecalState, FMatrix& OutDecalMatrix, FVector& OutDecalLocation, FVector2D& OutDecalOffset, FMatrix& OutDecalRefToLocal);

	//@todo debug - remove this
	virtual UBOOL HasMorphVertexFactories() const 
	{ 
		for( INT LODIndex=0;LODIndex < LODs.Num();LODIndex++ )
		{
			const FSkeletalMeshObjectLOD& SkelLOD = LODs(LODIndex);
			if( SkelLOD.MorphVertexFactories.Num() > 0 )
			{
				return TRUE;
			}
		}
		return FALSE;
	}

private:
	/** vertex data for rendering a single LOD */
	struct FSkeletalMeshObjectLOD
	{
		FSkeletalMeshObjectLOD(USkeletalMesh* InSkelMesh,INT InLOD,UBOOL bInDecalFactoriesEnabled)
		:	SkelMesh(InSkelMesh)
		,	LODIndex(InLOD)
		,	bDecalFactoriesEnabled(bInDecalFactoriesEnabled)
		,	ShadowVertexBuffer(InSkelMesh->LODModels(InLOD).NumVertices, 1.0f)
		,	MorphVertexBuffer(InSkelMesh,LODIndex)
		,	WeightsVertexBuffer(InSkelMesh,LODIndex)
		{
		}

		/** 
		 * Init rendering resources for this LOD 
		 * @param bUseLocalVertexFactory - use non-gpu skinned factory when rendering in ref pose
		 * @param bUseInstancedVertexWeights - use instanced influence weights instead of default weights
		 */
		void InitResources(UBOOL bUseLocalVertexFactory, UBOOL bUseInstancedVertexWeights);

		/** 
		 * Release rendering resources for this LOD 
		 */
		void ReleaseResources();

		/** 
		* Init rendering resources for the morph stream of this LOD
		* @param bUseInstancedVertexWeights - use instanced influence weights instead of default weights
		*/
		void InitMorphResources(UBOOL bUseInstancedVertexWeights);

		/** 
		* Release rendering resources for the morph stream of this LOD
		*/
		void ReleaseMorphResources();

		/** 
		 * Update the contents of the vertex buffer with new data. Note that this
		 * function is called from the render thread.
		 * @param	NewVertices - array of new vertex data
		 * @param	NumVertices - Number of vertices
		 */
		void UpdateShadowVertexBuffer( const FVector* NewVertices, UINT NumVertices ) const;

		USkeletalMesh* SkelMesh;
		INT LODIndex;

		/** TRUE if the owning skeletal mesh data object accepts decals.  If FALSE, DecalVertexFactories are not updated. */
		UBOOL bDecalFactoriesEnabled;

		/** The vertex buffer containing the CPU skinned vertices used for shadow volume rendering. */
		mutable FShadowVertexBuffer ShadowVertexBuffer;

		/** Vertex buffer that stores the morph target vertex deltas. Updated on the CPU */
		FMorphVertexBuffer MorphVertexBuffer;

		/** Vertex buffer that stores instanced influence weights and bones */
		FInflucenceWeightsVertexBuffer	WeightsVertexBuffer;

		/** one vertex factory for each chunk */
		TIndirectArray<FGPUSkinVertexFactory> VertexFactories;
		TIndirectArray<FGPUSkinDecalVertexFactory> DecalVertexFactories;

		/** Vertex factory defining both the base mesh as well as the morph delta vertex decls */
		TIndirectArray<FGPUSkinMorphVertexFactory> MorphVertexFactories;
		TIndirectArray<FGPUSkinMorphDecalVertexFactory> MorphDecalVertexFactories;

		/** one vertex factory for each chunk */
		TScopedPointer<FLocalVertexFactory> LocalVertexFactory;
		TScopedPointer<FLocalDecalVertexFactory> LocalDecalVertexFactory;

		/**
		* Update the contents of the morph target vertex buffer by accumulating all 
		* delta positions and delta normals from the set of active morph targets
		* @param ActiveMorphs - morph targets to accumulate. assumed to be weighted and have valid targets
		*/
		void UpdateMorphVertexBuffer( const TArray<FActiveMorph>& ActiveMorphs );
	};

	/** 
	* Initialize morph rendering resources for each LOD 
	*/
	void InitMorphResources();

	/** 
	* Release morph rendering resources for each LOD. 
	*/
	void ReleaseMorphResources();

	/** Render data for each LOD */
	TArray<struct FSkeletalMeshObjectLOD> LODs;

	/** Index of the LOD that have an up-to-date ShadowVertexBuffer */
	mutable INT CachedShadowLOD;

	/** A cache of skinned, local space vertex positions, used for Shadow Volumes. */
	mutable TArray<FVector> CachedVertices;

	/** Data that is updated dynamically and is needed for rendering */
	FDynamicSkelMeshObjectDataGPUSkin* DynamicData;

	/** TRUE if the morph resources have been initialized */
	UBOOL bMorphResourcesInitialized;

	/** TRUE if the instanced vertex influence weights have been initialized */
	UBOOL bInfluenceWeightsInitialized;

	/** optimized skinning */
	FORCEINLINE void SkinVertices( FVector* DestVertex, const TArray<FMatrix>& ReferenceToLocal, FStaticLODModel& LOD ) const;	
};

#endif // __UNSKELETALRENDERGPUSKIN_H__
