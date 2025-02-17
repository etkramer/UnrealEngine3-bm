/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class FracturedStaticMeshComponent extends FracturedBaseComponent
	native(Mesh);

/** Stores non-zero for each fragment whose neighbors are all visible, and 0 otherwise. */
var protected{protected} transient const array<byte> FragmentNeighborsVisible;

/** Local space bounding box of visible fragments, updated on attach. */
var protected{protected} const Box VisibleBox;

/** If true, all fragment visibility and transform information will be forwarded to SkinnedComponent, which will handle rendering. */
var protected{protected} const bool bUseSkinnedRendering;

/** 
 *	If true, the only thing considered when calculating the bounds of this component are the graphics verts current visible.
 *	Using this and having simplified collision will cause unpredictable results. 
 */
var bool	bUseVisibleVertsForBounds;

/** 
 *	Allows per-instance override of chunk support/destroyable flags. 
 *	Marks chunks at top of mesh as 'root' and 'non destroyable'.
 */
var()	bool	bTopFragmentsRootNonDestroyable;

/**  
 *	Allows per-instance override of chunk support/destroyable flags. 
 *	Marks chunks at bottom of mesh as 'root' and 'non destroyable'.
 */
var()	bool	bBottomFragmentsRootNonDestroyable;

/** Threshold distance of fragment box from top/bottom of mesh to be considered for bTop/BottomFragmentsRootNonDestroyable */
var()	float	TopBottomFragmentDistThreshold;

/** Allows overriding the LoseChunkOutsideMaterial on a per-instance basis. */
var()	MaterialInterface	LoseChunkOutsideMaterialOverride;

/** Z value of top of fractured piece bounds. */
var		float	FragmentBoundsMaxZ;

/** Z value of bottom of fractured piece bounds. */
var		float	FragmentBoundsMinZ;

/** Component which handles rendering if bUseSkinnedRendering is true. */
var transient FracturedSkinnedMeshComponent SkinnedComponent;

/**  */
struct native FragmentGroup
{
	var array<int>	FragmentIndices;
	var bool		bGroupIsRooted;
};

cpptext
{
public:
	//UObject
	virtual void Serialize(FArchive& Ar);

	//UPrimitiveComponent
	virtual void UpdateBounds();
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
	virtual UBOOL LineCheck(FCheckResult& Result, const FVector& End, const FVector& Start, const FVector& Extent, DWORD TraceFlags);
	virtual UBOOL PointCheck(FCheckResult& Result,const FVector& Location,const FVector& Extent,DWORD TraceFlags);
	virtual void CookPhysConvexDataForScale(ULevel* Level, const FVector& TotalScale3D, INT& TriByteCount, INT& TriMeshCount, INT& HullByteCount, INT& HullCount);
	virtual void CheckForErrors();
	virtual class FDecalRenderData* GenerateDecalRenderData(class FDecalState* Decal) const;

	/** Allocates an implementation of FStaticLightingMesh that will handle static lighting for this component */
	virtual class FStaticMeshStaticLightingMesh* AllocateStaticLightingMesh(INT LODIndex, const TArray<ULightComponent*>& InRelevantLights);

protected:

	/** Attaches the component to the scene, and initializes the component's resources if they have not been yet. */
	virtual void Attach();

	/** 
	* Detach the component from the scene and remove its render proxy 
	* @param bWillReattach TRUE if the detachment will be followed by an attachment
	*/
	virtual void Detach( UBOOL bWillReattach = FALSE );
	
	/**
	* @return	FALSE since fractured geometry will handle its own decal detachment
	*/
	virtual UBOOL AllowDecalRemovalOnDetach() const
	{
		return FALSE;
	}

	virtual void UpdateTransform();

	/** Update FragmentBoundsMin/MaxZ */
	void UpdateFragmentMinMaxZ();

	/** See if the bTopFragmentsSupportNonDestroyable/bBottomFragmentsSupportNonDestroyable flags indicate this chunk. */
	UBOOL FragmentInstanceIsSupportNonDestroyable(int FragmentIndex) const;

	/** Checks if the given fragment is visible. */
	virtual UBOOL IsElementFragmentVisible(INT ElementIndex, INT FragmentIndex, INT InteriorElementIndex, INT CoreFragmentIndex, UBOOL bAnyFragmentsHidden) const;

	/** 
	 * Updates the fragments of this component that are visible.  
	 * @param NewVisibleFragments - visibility factors for this component, corresponding to FracturedStaticMesh's Fragments array
	 * @param bForceUpdate - whether to update this component's resources even if no fragments have changed visibility
	 */
	virtual void UpdateVisibleFragments(const TArray<BYTE>& NewVisibleFragments, UBOOL bForceUpdate);

	friend class FFracturedStaticMeshSceneProxy;
}

/** Change the set of visible fragments. */
simulated native function SetVisibleFragments(array<byte> VisibilityFactors);

/** Returns if this fragment is destroyable. */
simulated native function bool IsFragmentDestroyable(INT FragmentIndex) const;

/** Returns if this is a supporting 'root' fragment.  */
simulated native function bool IsRootFragment(INT FragmentIndex) const;

/** Returns if this fragment should never spawn a physics object.  */
simulated native function bool IsNoPhysFragment(INT FragmentIndex) const;

/** Get the bounding box of a specific chunk, in world space. */
native function box GetFragmentBox(int FragmentIndex) const;

/** Returns average exterior normal of a particular chunk. */
native function vector GetFragmentAverageExteriorNormal(int FragmentIndex) const;

/** Gets the index that is the 'core' of this mesh. */
native function int GetCoreFragmentIndex() const;

/** 
 *	Based on the hidden state of chunks, groups which are connected.  
 *	@param IgnoreFragments	Additional fragments to ignore when finding islands. These will not end up in any groups.
 */
simulated native function array<FragmentGroup> GetFragmentGroups(array<int> IgnoreFragments, float MinConnectionArea) const;

defaultproperties
{
	OverriddenLightMapResolution=64
	TopBottomFragmentDistThreshold=0.1
	bUsePrecomputedShadows=TRUE
}
