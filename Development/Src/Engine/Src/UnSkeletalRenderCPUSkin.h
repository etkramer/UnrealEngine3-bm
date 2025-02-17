/*=============================================================================
	UnSkeletalRenderCPUSkin.h: CPU skinned mesh object and resource definitions
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _INC_SKELETALRENDERCPUSKIN
#define _INC_SKELETALRENDERCPUSKIN

#include "UnSkeletalRender.h"
#include "SkelMeshDecalVertexFactory.h"
#include "UnDecalRenderData.h"
#include "LocalDecalVertexFactory.h"

/** data for a single skinned skeletal mesh vertex */
struct FFinalSkinVertex
{
	FVector			Position;
	FPackedNormal	TangentX;
	FPackedNormal	TangentZ;
	FLOAT			U;
	FLOAT			V;
};

/**
 * Skeletal mesh vertices which have been skinned to their final positions 
 */
class FFinalSkinVertexBuffer : public FVertexBuffer
{
public:

	/** 
	 * Constructor
	 * @param	InSkelMesh - parent mesh containing the static model data for each LOD
	 * @param	InLODIdx - index of LOD model to use from the parent mesh
	 */
	FFinalSkinVertexBuffer(USkeletalMesh* InSkelMesh, INT InLODIdx)
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
	 * Cpu skinned vertex name 
	 */
	virtual FString GetFriendlyName() const { return TEXT("CPU skinned mesh vertices"); }

private:
	/** index to the SkelMesh.LODModels */
	INT	LODIdx;
	/** parent mesh containing the source data */
	USkeletalMesh* SkelMesh;
};

/**
 * Dynamic triangle indices associated with the skeletal mesh. For example used for torn cloth.
 */
class FFinalDynamicIndexBuffer : public FIndexBuffer
{
public:

	/** 
	 * Constructor
	 * @param	InSkelMesh - parent mesh containing the static model data for each LOD
	 * @param	InLODIdx - index of LOD model to use from the parent mesh
	 */
	FFinalDynamicIndexBuffer(USkeletalMesh* InSkelMesh, INT InLODIdx)
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
	 * Cpu skinned vertex name 
	 */
	virtual FString GetFriendlyName() const { return TEXT("CPU skinned dynamic indices"); }

private:
	/** index to the SkelMesh.LODModels */
	INT	LODIdx;
	/** parent mesh containing the source data */
	USkeletalMesh* SkelMesh;
};

/** 
* Stores the updated matrices needed to skin the verts.
* Created by the game thread and sent to the rendering thread as an update 
*/
class FDynamicSkelMeshObjectDataCPUSkin : public FDynamicSkelMeshObjectData
{
public:

	/**
	* Constructor
	* Updates the ReferenceToLocal matrices using the new dynamic data.
	* @param	InSkelMeshComponent - parent skel mesh component
	* @param	InLODIndex - each lod has its own bone map 
	* @param	ActiveMorphs - morph targets to blend with during skinning
	*/
	FDynamicSkelMeshObjectDataCPUSkin(
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
	/** morph targets to blend when skinning verts */
	TArray<FActiveMorph> ActiveMorphs;

#if !NX_DISABLE_CLOTH
	/** Component's transform. Just used when updating cloth vertices (have to be transformed from world to local space) */
	FMatrix	WorldToLocal;
	/** Array of cloth mesh vertex positions, passed from game thread (where simulation happens) to rendering thread (to be drawn). */
	TArray<FVector> ClothPosData;
	/** Array of cloth mesh normals, also passed from game to rendering thread. */
	TArray<FVector> ClothNormalData;
	/** Amount to blend in cloth */
	FLOAT ClothBlendWeight;

	/** Array of cloth indices for tangent calculations. */
	TArray<INT> ClothIndexData;
	
	/** Number of vertices returned by the physics SDK. Can differ from the array size due to tearing */
	INT ActualClothPosDataNum;

	TArray<INT> ClothParentIndices;
#endif

#if !NX_DISABLE_SOFTBODY

	TArray<FVector> SoftBodyTetraPosData;

#endif //!NX_DISABLE_SOFTBODY


};

/**
 * Render data for a CPU skinned mesh
 */
class FSkeletalMeshObjectCPUSkin : public FSkeletalMeshObject
{
public:

	/** 
	 * Constructor
	 * @param	InSkeletalMeshComponent - skeletal mesh primitive we want to render 
	 */
	FSkeletalMeshObjectCPUSkin(USkeletalMeshComponent* InSkeletalMeshComponent);

	/** 
	 * Destructor 
	 */
	virtual ~FSkeletalMeshObjectCPUSkin();

	/** 
	 * Initialize rendering resources for each LOD 
	 */
	virtual void InitResources();
	
	/** 
	 * Release rendering resources for each LOD 
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
	 * Compute the distance of a light from the triangle planes.
	 */
	virtual void GetPlaneDots(FLOAT* OutPlaneDots,const FVector4& LightPosition,INT LODIndex) const;

	/**
	 * Re-skin cached vertices for an LOD and update the vertex buffer. Note that this
	 * function is called from the render thread!
	 * @param	LODIndex - index to LODs
	 * @param	bForce - force update even if LOD index hasn't changed
	 * @param	bUpdateShadowVertices - whether to update the shadow volumes vertices
	 * @param	bUpdateDecalVertices - whether to update the decal vertices
	 */
	virtual void CacheVertices(INT LODIndex, UBOOL bForce, UBOOL bUpdateShadowVertices, UBOOL bUpdateDecalVertices) const;

	/**
	 * @param	LODIndex - each LOD has its own vertex data
	 * @param	ChunkIdx - not used
	 * @return	vertex factory for rendering the LOD
	 */
	virtual const FVertexFactory* GetVertexFactory(INT LODIndex,INT ChunkIdx) const;

	/**
	 * @return		Vertex factory for rendering the specified decal at the specified LOD.
	 */
	virtual FDecalVertexFactoryBase* GetDecalVertexFactory(INT LODIndex,INT ChunkIdx,const FDecalInteraction* Decal);

	/**
	 * Get the shadow vertex factory for an LOD
	 * @param	LODIndex - each LOD has its own vertex data
	 * @return	Vertex factory for rendering the shadow volume for the LOD
	 */
	virtual const FLocalShadowVertexFactory& GetShadowVertexFactory( INT LODIndex ) const;

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
		//@todo decal - fix detachment and resource cleanup of decals on CPU skinned skeletal meshes
		return FALSE;
	}

	/**
	 * Adds a decal interaction to the primitive.  This is called in the rendering thread by the skeletal mesh proxy's AddDecalInteraction_RenderingThread.
	 */
	virtual void AddDecalInteraction_RenderingThread(const FDecalInteraction& DecalInteraction);

	/**
	 * Removes a decal interaction from the primitive.  This is called in the rendering thread by the skeletal mesh proxy's RemoveDecalInteraction_RenderingThread.
	 */
	virtual void RemoveDecalInteraction_RenderingThread(UDecalComponent* DecalComponent);

	/**
	 * Transforms the decal from refpose space to local space, in preparation for application
	 * to post-skinned (ie local space) verts on the GPU.
	 */
	virtual void TransformDecalState(const FDecalState& DecalState, FMatrix& OutDecalMatrix, FVector& OutDecalLocation, FVector2D& OutDecalOffset, FMatrix& OutDecalRefToLocal);
	
	virtual const FIndexBuffer* GetDynamicIndexBuffer(INT InLOD) const
	{
		return &(LODs(InLOD).DynamicIndexBuffer);
	}
private:
	/** vertex data for rendering a single LOD */
	struct FSkeletalMeshObjectLOD
	{
		FLocalVertexFactory				VertexFactory;
		mutable FFinalSkinVertexBuffer	VertexBuffer;
		mutable FShadowVertexBuffer		ShadowVertexBuffer;

		mutable FFinalDynamicIndexBuffer DynamicIndexBuffer;

		/** Resources for a decal drawn at a particular LOD. */
		class FDecalLOD
		{
		public:
			const UDecalComponent*		DecalComponent;
			FLocalDecalVertexFactory	DecalVertexFactory;

			FDecalLOD(const UDecalComponent* InDecalComponent=NULL)
				: DecalComponent( InDecalComponent )
			{}

			void InitResources_GameThread(FSkeletalMeshObjectCPUSkin::FSkeletalMeshObjectLOD* LODObject);
			void InitResources_RenderingThread(FSkeletalMeshObjectCPUSkin::FSkeletalMeshObjectLOD* LODObject);
			void ReleaseResources_GameThread()
			{
				BeginReleaseResource(&DecalVertexFactory);
			}
			void ReleaseResources_RenderingThread()
			{
				DecalVertexFactory.ReleaseResource();
			}
		};
		/** List of decals drawn at this LOD. */
		mutable TArray<FDecalLOD>	Decals;
		/** TRUE if resources for this LOD have already been initialized. */
		UBOOL						bResourcesInitialized;

		FSkeletalMeshObjectLOD(USkeletalMesh* InSkelMesh,INT InLOD)
		:	VertexBuffer(InSkelMesh,InLOD)
		,	ShadowVertexBuffer(InSkelMesh->LODModels(InLOD).NumVertices)
		,	DynamicIndexBuffer(InSkelMesh, InLOD)
		,	bResourcesInitialized( FALSE )
		{
		}
		/** 
		 * Init rendering resources for this LOD 
		 */
		void InitResources();
		/** 
		 * Release rendering resources for this LOD 
		 */
		void ReleaseResources();
		/** 
		 * Update the contents of the vertex buffer with new data
		 * @param	NewVertices - array of new vertex data
		 * @param	Size - size of new vertex data aray 
		 */
		void UpdateFinalSkinVertexBuffer(void* NewVertices, DWORD Size) const;

		/** 
		 * Update the contents of the vertex buffer with new data
		 * @param	NewVertices - array of new vertex data
		 * @param	NumVertices - Number of vertices
		 */
		void UpdateShadowVertexBuffer( const FFinalSkinVertex* NewVertices, UINT NumVertices ) const;

		/** Creates resources for drawing the specified decal at this LOD. */
		void AddDecalInteraction_RenderingThread(const FDecalInteraction& DecalInteraction);

		/** Releases resources for the specified decal at this LOD. */
		void RemoveDecalInteraction_RenderingThread(UDecalComponent* DecalComponent);

		/**
		 * @return		The vertex factory associated with the specified decal at this LOD.
		 */
		FLocalDecalVertexFactory* GetDecalVertexFactory(const UDecalComponent* Decal);

	protected:
		/**
		 * @return		The index into the decal objects list for the specified decal, or INDEX_NONE if none found.
		 */
		INT FindDecalObjectIndex(const UDecalComponent* DecalComponent) const;
	};

    /** Render data for each LOD */
	TArray<struct FSkeletalMeshObjectLOD> LODs;

	/** Data that is updated dynamically and is needed for rendering */
	class FDynamicSkelMeshObjectDataCPUSkin* DynamicData;

 	/** Index of LOD level's vertices that are currently stored in CachedFinalVertices */
 	mutable INT	CachedVertexLOD;

	/** Index of the LOD that have an up-to-date ShadowVertexBuffer */
	mutable INT CachedShadowLOD;

 	/** Cached skinned vertices. Only updated/accessed by the rendering thread */
 	mutable TArray<FFinalSkinVertex> CachedFinalVertices;

 	/** Cached tangent normals. Only updated/accessed by the rendering thread */
 	mutable TArray<FVector> CachedClothTangents;


 	/** Cached vertex positions of the soft-body surface mesh. Only updated/accessed by the rendering thread */
 	mutable TArray<FVector> CachedSoftBodyPositions;

 	/** Cached vertex normals of the soft-body surface mesh. Only updated/accessed by the rendering thread */
 	mutable TArray<FVector> CachedSoftBodyNormals;

 	/** Cached vertex tangents of the soft-body surface mesh. Only updated/accessed by the rendering thread */
 	mutable TArray<FVector> CachedSoftBodyTangents;

};

#endif
