/*=============================================================================
	UnSkeletalRender.h: Definitions and inline code for rendering SkeletalMeshComponet
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef SKEL_RENDER_HEADER
#define SKEL_RENDER_HEADER

// Forward declarations.
class FSkelMeshDecalVertexFactoryBase;
class FDecalInteraction;
class FDecalState;
class UDecalComponent;

// smallest blend weight for morph targets
extern const FLOAT MinMorphBlendWeight;
// largest blend weight for morph targets
extern const FLOAT MaxMorphBlendWeight;

/** 
* Indices for a pair of bones
*/
struct FBoneIndexPair
{
	INT BoneIdx[2];
};

/**
* Interface for mesh rendering data
*/
class FSkeletalMeshObject : public FDeferredCleanupInterface
{
public:
	FSkeletalMeshObject(USkeletalMeshComponent* InSkeletalMeshComponent) 
	:	MinDesiredLODLevel(0)
	,	MaxDistanceFactor(0.f)
	,	WorkingMinDesiredLODLevel(0)
	,	WorkingMaxDistanceFactor(0.f)
	,   bHasBeenUpdatedAtLeastOnce(FALSE)
	,	SkeletalMesh(InSkeletalMeshComponent->SkeletalMesh)
	,	LastFrameNumber(0)
	,	bDecalFactoriesEnabled(InSkeletalMeshComponent->bAcceptsStaticDecals || InSkeletalMeshComponent->bAcceptsDynamicDecals)
	,	bUseLocalVertexFactory(InSkeletalMeshComponent->bForceRefpose)
	,	bUseInstancedVertexInfluences(InSkeletalMeshComponent->bAlwaysUseInstanceWeights)
	{
		check(SkeletalMesh);
	}
	virtual ~FSkeletalMeshObject()
	{
	}
	virtual void InitResources() = 0;
	virtual void ReleaseResources() = 0;
	virtual void Update(INT LODIndex,USkeletalMeshComponent* InSkeletalMeshComponent,const TArray<FActiveMorph>& ActiveMorphs) = 0;
	virtual void UpdateDynamicData_RenderThread(class FDynamicSkelMeshObjectData* InDynamicData) = 0;
	virtual void ToggleVertexInfluences(UBOOL bEnabled) = 0;
	virtual void UpdateVertexInfluences(const TArray<FBoneIndexPair>& BonePairs,INT InfluencesIdx,UBOOL bResetInfluences) = 0;
	virtual void UpdateVertexInfluences_RenderThread(class FDynamicUpdateVertexInfluencesData* InDynamicData) = 0;
	virtual const FVertexFactory* GetVertexFactory(INT LODIndex,INT ChunkIdx) const = 0;
	virtual FDecalVertexFactoryBase* GetDecalVertexFactory(INT LODIndex,INT ChunkIdx,const FDecalInteraction* Decal) = 0;
	virtual const FLocalShadowVertexFactory& GetShadowVertexFactory( INT LODIndex ) const = 0;
	virtual void GetPlaneDots(FLOAT* OutPlaneDots,const FVector4& LightPosition,INT LODIndex) const = 0;
	virtual void CacheVertices(INT LODIndex, UBOOL bForce, UBOOL bUpdateShadowVertices, UBOOL bUpdateDecalVertices) const = 0;
	virtual TArray<FMatrix>* GetSpaceBases() const = 0;
	virtual INT GetLOD() const = 0;
	virtual TArray<FVector>* GetSoftBodyTetraPosData() const = 0;
	virtual UBOOL SupportsDecalRendering() const = 0;
	//@todo debug - remove this
	virtual UBOOL HasMorphVertexFactories() const { return FALSE; }
	
	/**
	 * Returns an index buffer used for storing dynamically generated triangle(indices). 
	 * Only valid for CPU skinned meshes.
	 */
	virtual const FIndexBuffer* GetDynamicIndexBuffer(INT Lod) const { return NULL; }

	/**
	 * Adds a decal interaction to the primitive.  This is called in the rendering thread by the skeletal mesh proxy's AddDecalInteraction_RenderingThread.
	 */
	virtual void AddDecalInteraction_RenderingThread(const FDecalInteraction& DecalInteraction) {}

	/**
	 * Removes a decal interaction from the primitive.  This is called in the rendering thread by the skeletal mesh proxy's RemoveDecalInteraction_RenderingThread.
	 */
	virtual void RemoveDecalInteraction_RenderingThread(UDecalComponent* DecalComponent) {}

	/**
	 * Allows derived types to transform decal state into a space that's appropriate for the given skinning algorithm.
	 */
	virtual void TransformDecalState(const FDecalState& Decal, FMatrix& OutDecalMatrix, FVector& OutDecalLocation, FVector2D& OutDecalOffset, FMatrix& OutDecalRefToLocal) = 0;

	/** 
	 *	Given a set of views, update the MinDesiredLODLevel member to indicate the minimum (ie best) LOD we would like to use to render this mesh. 
	 *	This is called from the rendering thread (PreRender) so be very careful what you read/write to.
	 */
	void UpdateMinDesiredLODLevel(const FSceneView* View, const FBoxSphereBounds& Bounds, INT FrameNumber);

	// allow access to mesh component
	friend class FDynamicSkelMeshObjectDataCPUSkin;
	friend class FDynamicSkelMeshObjectDataGPUSkin;
	friend class FSkeletalMeshSceneProxy;

	// FDeferredCleanupInterface
	virtual void FinishCleanup()
	{
		delete this;
	}

	/** 
	 *	Lowest (best) LOD that was desired for rendering this SkeletalMesh last frame. 
	 *	This should only ever be WRITTEN by the RENDER thread (in FSkeletalMeshProxy::PreRenderView) and READ by the GAME thread (in USkeletalMeshComponent::UpdateSkelPose).
	 */
	INT MinDesiredLODLevel;

	/** 
	 *	High (best) DistanceFactor that was desired for rendering this SkeletalMesh last frame. Represents how big this mesh was in screen space  
	 *	This should only ever be WRITTEN by the RENDER thread (in FSkeletalMeshProxy::PreRenderView) and READ by the GAME thread (in USkeletalMeshComponent::UpdateSkelPose).
	 */
	FLOAT MaxDistanceFactor;

	/** This frames min desired LOD level. This is copied (flipped) to MinDesiredLODLevel at the beginning of the next frame. */
	INT WorkingMinDesiredLODLevel;

	/** This frames max distance factor. This is copied (flipped) to MaxDistanceFactor at the beginning of the next frame. */
	FLOAT WorkingMaxDistanceFactor;

    /** This is set to TRUE when we have sent our Mesh data to the rendering thread at least once as it needs to have have a datastructure created there for each MeshObject **/
	UBOOL           bHasBeenUpdatedAtLeastOnce;

protected:
	USkeletalMesh*	SkeletalMesh;

	/** Used to keep track of the first call to UpdateMinDesiredLODLevel each frame. */
	UINT			LastFrameNumber;

	/** TRUE if decal vertex factories are enabled (beacuse this skeletal mesh can accept decals). */
	UBOOL			bDecalFactoriesEnabled;

	/** TRUE if we are only using local vertex factories for this mesh (when bForceRefpose = TRUE) */
	UBOOL			bUseLocalVertexFactory;

	/** 
	* TRUE if the instanced vertex buffer containing influence weights/bones should be used instead of 
	* the default weights form the USkeletalMesh vertex buffer.  
	*/
	UBOOL			bUseInstancedVertexInfluences;
};

/** Dynamic data updates needed by the rendering thread are sent with this */
class FDynamicSkelMeshObjectData
{
public:
	FDynamicSkelMeshObjectData(){}
	virtual ~FDynamicSkelMeshObjectData(){}
};

/** 
* Stores the data for updating instanced weights
* Created by the game thread and sent to the rendering thread as an update 
*/
class FDynamicUpdateVertexInfluencesData
{
public:
	/**
	* Constructor
	*/
	FDynamicUpdateVertexInfluencesData(
		const TArray<FBoneIndexPair>& InBonePairs,
		INT InInfluenceIdx,
		UBOOL InbResetInfluences )
		:	BonePairs(InBonePairs)
		,	InfluenceIdx(InInfluenceIdx)
		,	bResetInfluences(InbResetInfluences)
	{
	}

	/** set of bone pairs used to find vertices that need to have their weights updated */
	TArray<FBoneIndexPair> BonePairs;
	/** index of skeletal mesh vertex influence entry to use (INDEX_NONE for defaults from base verts) */
	INT InfluenceIdx;
	/** resets the array of instanced weights/bones using the ones from the base mesh defaults before udpating */
	UBOOL bResetInfluences;
};

/**
* Utility function that fills in the array of ref-pose to local-space matrices using 
* the mesh component's updated space bases
* @param	ReferenceToLocal - matrices to update
* @param	SkeletalMeshComponent - mesh primitive with updated bone matrices
* @param	LODIndex - each LOD has its own mapping of bones to update
*/
void UpdateRefToLocalMatrices( TArray<FMatrix>& ReferenceToLocal, USkeletalMeshComponent* SkeletalMeshComponent, INT LODIndex );

#endif //SKEL_RENDER_HEADER
