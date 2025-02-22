
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AnimNode extends AnimObject
	native(Anim)
	hidecategories(Object)
	abstract;

/** Bone Atom definition */
struct BoneAtom
{
	var	quat	Rotation;
	var	vector	Translation;
	var float	Scale;
};

/** Enumeration for slider types */
enum ESliderType
{
	ST_1D,
	ST_2D
};

/** This node is considered 'relevant' - that is, has >0 weight in the final blend. */
var	transient const bool				bRelevant;
/** set to TRUE when this node became relevant this round of updates. Will be set to false on the next tick. */
var transient const bool				bJustBecameRelevant;
/** Allows for optimisiation. Tick is not called on this node if  */
var() bool								bSkipTickWhenZeroWeight;
/** If TRUE, this node will be ticked, even if bPauseAnims is TRUE on the SkelMeshComp. */
var() bool								bTickDuringPausedAnims;

/** Used to avoid ticking a node twice if it has multiple parents. */
var	transient const int					NodeTickTag;

/** Used to indicate whether the BoneAtom cache for this node is up-to-date or not. */
var transient const int					NodeCachedAtomsTag;

/** Total apparent weight this node has in the final blend of all animations. */
var	const float							NodeTotalWeight;

/** internal. Accumulator to calculate NodeTotalWeight */
var const transient	float				TotalWeightAccumulator;

/** Parent node of this AnimNode in the blend tree. */
var transient array<AnimNodeBlendBase>	ParentNodes;

/** This is the name used to find an AnimNode by name from a tree. */
var() name								NodeName;

/** If a node is linked to more than once in the graph, this is a cache of the results, to avoid re-evaluating the results. */
var	transient array<BoneAtom>			CachedBoneAtoms;

/** Cached root motion delta, to avoid recalculating (see above). */
var transient BoneAtom					CachedRootMotionDelta;

/** Cached bool indicating if node supplies root motion, to avoid recalculating (see above). */
var transient int						bCachedHasRootMotion;

/** Obsolete. Remove me sometime after VER_AIMOFFSET_ROT2QUAT has been long distributed. */
var	const INT							InstanceVersionNumber;

/** used when iterating over nodes via GetNodes() and related functions to skip nodes that have already been processed */
var const transient protected{protected} int SearchTag;

cpptext
{
	// UAnimNode interface

	/** Do any initialisation, and then call InitAnim on all children. Should not discard any existing anim state though. */
	virtual void InitAnim( USkeletalMeshComponent* meshComp, UAnimNodeBlendBase* Parent );

	/** AnimSets have been updated, update all animations */
	virtual void AnimSetsUpdated() {}

	/** 
	 *	Called just after a node has been copied from its AnimTreeTemplate version. 
	 *	This is called on the copy, and SourceNode is the node within the AnimTreeTemplate.
	 */
	virtual void PostAnimNodeInstance(UAnimNode* SourceNode) {}

	/**
	 *	Update this node, then call TickAnim on all children.
	 *	@param DeltaSeconds		Amount of time to advance this node.
	 *	@param TotalWeight		The eventual weight that this node will have in the final blend. This is the multiplication of weights of all nodes above this one.
	 */
	virtual	void	TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight ) {}

	/** Parent node is requesting a blend out. Give node a chance to delay that. */
	virtual UBOOL	CanBlendOutFrom() { return TRUE; }

	/** parent node is requesting a blend in. Give node a chance to delay that. */
	virtual UBOOL	CanBlendTo() { return TRUE; }

	/** Add this node and all children to array. Node are added so a parent is always before its children in the array. */
	void GetNodes(TArray<UAnimNode*>& Nodes);

	/** Add this node and all children of the specified class to array. Node are added so a parent is always before its children in the array. */
	void GetNodesByClass(TArray<class UAnimNode*>& Nodes, class UClass* BaseClass);

	/** Return an array with all UAnimNodeSequence childs, including this node. */
	void GetAnimSeqNodes(TArray<UAnimNodeSequence*>& Nodes, FName InSynchGroupName=NAME_None);

	/** Used for building array of AnimNodes in 'tick' order - that is, all parents of a node are added to array before it. */
	virtual void BuildTickArray(TArray<UAnimNode*>& OutTickArray) {}

	/**
	 *	Get the local transform for each bone. If a blend, will recursively ask children and blend etc.
	 *	DesiredBones should be in strictly increasing order.
	 */
	virtual void GetBoneAtoms(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);

	/**
	 *	Will copy the cached results into the OutAtoms array if they are up to date and return TRUE
	 *	If cache is not up to date, does nothing and retuns FALSE.
	 */
	virtual UBOOL GetCachedResults(FBoneAtomArray& OutAtoms, FBoneAtom& OutRootMotionDelta, INT& bOutHasRootMotion);

	/** Save the supplied array of BoneAtoms in the CachedBoneAtoms. */
    virtual UBOOL ShouldSaveCachedResults();
	void SaveCachedResults(const FBoneAtomArray& NewAtoms, const FBoneAtom& NewRootMotionDelta, INT bNewHasRootMotion);

	/** Get notification that this node has become relevant for the final blend. ie TotalWeight is now > 0 */
	virtual void OnBecomeRelevant();

	/** Get notification that this node is no longer relevant for the final blend. ie TotalWeight is now == 0 */
	virtual void OnCeaseRelevant();

	/** Utility for counting the number of parents of this node that have been ticked. */
	INT CountNumParentsTicked();

	/** Returns TRUE if this node is a child of Node */
	UBOOL IsChildOf(UAnimNode* Node);

	/** Optimisation way to see if this is a UAnimTree */
	virtual UAnimTree* GetAnimTree() { return NULL; }

	virtual void SetAnim( FName SequenceName ) {}
	virtual void SetPosition( FLOAT NewTime, UBOOL bFireNotifies ) {}

	/// ANIMTREE EDITOR

	/**
	 * Draws this node in the AnimTreeEditor.
	 *
	 * @param	Canvas			The canvas to use.
	 * @param	bSelected		TRUE if this node is selected.
	 * @param	bShowWeight		If TRUE, show the global percentage weight of this node, if applicable.
	 * @param	bCurves			If TRUE, render links as splines; if FALSE, render links as straight lines.
	 */
	virtual void DrawAnimNode(FCanvas* Canvas, UBOOL bSelected, UBOOL bShowWeight, UBOOL bCurves) {}

	/** Return title to display for this Node in the AnimTree editor. */
	virtual FString GetNodeTitle() { return TEXT(""); }

	/** For editor use. */
	virtual FIntPoint GetConnectionLocation(INT ConnType, INT ConnIndex);

	/** Return the number of sliders */
	virtual INT GetNumSliders() const { return 0; }

	/** Return the slider type of slider Index */
	virtual ESliderType GetSliderType(INT InIndex) const { return ST_1D; }

	/** Return current position of slider for this node in the AnimTreeEditor. Return value should be within 0.0 to 1.0 range. */
	virtual FLOAT GetSliderPosition(INT SliderIndex, INT ValueIndex) { return 0.f; }

	/** Called when slider is moved in the AnimTreeEditor. NewSliderValue is in range 0.0 to 1.0. */
	virtual void HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue) {}

	/** Get the number to draw under the slider to show the current value being previewed. */
	virtual FString GetSliderDrawValue(INT SliderIndex) { return FString(TEXT("")); }

	/** internal code for GetNodes(); should only be called from GetNodes() or from the GetNodesInternal() of this node's parent */
	virtual void GetNodesInternal(TArray<UAnimNode*>& Nodes);

	// STATIC ANIMTREE UTILS

	/** flag to prevent calling GetNodesInternal() from anywhere besides GetNodes() or another GetNodesInternal(), since
	 * we can't make it private/protected because UAnimNodeBlendBase needs to be able to call it on its children
	 */
	static UBOOL bNodeSearching;
	/** current tag value used for SearchTag on nodes being iterated over. Incremented every time a new search is started */
	static INT CurrentSearchTag;

	/**
	 * Fills the Atoms array with the specified skeletal mesh reference pose.
	 *
	 * @param Atoms				[out] Output array of relative bone transforms. Must be the same length as RefSkel when calling function.
	 * @param DesiredBones		Indices of bones we want to modify. Parents must occur before children.
	 * @param RefSkel			Input reference skeleton to create atoms from.
	 */
	static void FillWithRefPose(TArray<FBoneAtom>& Atoms, const TArray<BYTE>& DesiredBones, const TArray<struct FMeshBone>& RefSkel);
	static void FillWithRefPose(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, const TArray<struct FMeshBone>& RefSkel);

	/** Utility for taking an array of bone indices and ensuring that all parents are present (ie. all bones between those in the array and the root are present). */
	static void EnsureParentsPresent( TArray<BYTE>& BoneIndices, USkeletalMesh* SkelMesh );
}

/** Called from InitAnim. Allows initialisation of script-side properties of this node. */
event OnInit();
/** Get notification that this node has become relevant for the final blend. ie TotalWeight is now > 0 */
event OnBecomeRelevant();
/** Get notification that this node is no longer relevant for the final blend. ie TotalWeight is now == 0 */
event OnCeaseRelevant();

/**
 * Find an Animation Node in the Animation Tree whose NodeName matches InNodeName.
 * Will search this node and all below it.
 * Warning: The search is O(n^2), so for large AnimTrees, cache result.
 */
native final function AnimNode FindAnimNode(name InNodeName);

native function PlayAnim(bool bLoop = false, float Rate = 1.0f, float StartTime = 0.0f);
native function StopAnim();
