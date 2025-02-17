
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AnimTree extends AnimNodeBlendBase
	native(Anim)
	hidecategories(Object);

/** Definition of a group of AnimNodeSequences */
struct native AnimGroup
{
	/** Cached array of AnimNodeSequence nodes. */
	var	transient	const	Array<AnimNodeSequence> SeqNodes;
	/** Master node for synchronization. (Highest weight of the group) */
	var transient	const	AnimNodeSequence		SynchMaster;
	/** Master node for notifications. (Highest weight of the group) */
	var transient	const	AnimNodeSequence		NotifyMaster;
	/* Name of group. */
	var()			const	Name					GroupName;
	/** Rate Scale */
	var()			const	float					RateScale;
	/** Tracked Synch Position */
	var				const	float					SynchPctPosition;

	structdefaultproperties
	{
		RateScale=1.f
	}
};

/** List of animations groups */
var()	Array<AnimGroup>	AnimGroups;

/** 
 * Skeleton Branches that should be composed first.
 * This is to solve Controllers relying on bones to be updated before them. 
 */
var()			Array<Name>		PrioritizedSkelBranches;
/** Internal list of priority levels */
var				Array<BYTE>		PriorityList;

struct native SkelControlListHead
{
	/** Name of bone that this list of SkelControls will be executed after. */
	var	name			BoneName;

	/** First Control in the linked list of SkelControls to execute. */
	var editinline export SkelControlBase	ControlHead;

	/** For editor use. */
	var int				DrawY;
};

/** Root of tree of MorphNodes. */
var		editinline export array<MorphNodeBase>			RootMorphNodes;

/** Array of lists of SkelControls. Each list is executed after the bone specified using BoneName is updated with animation data. */
var		editinline export array<SkelControlListHead>	SkelControlLists;

/** Array for storing pose when bUseSavedPose is TRUE */
var		array<BoneAtom>			SavedPose;

/** If TRUE, AnimTree will always just return cached pose, rather than evaluating entire anim tree. */
var		bool					bUseSavedPose;

////// AnimTree Editor support

/** Y position of MorphNode input on AnimTree. */
var		int				MorphConnDrawY;

/** Used to avoid editing the same AnimTree in multiple AnimTreeEditors at the same time. */
var		transient bool	bBeingEdited;

/** Play rate used when previewing animations */
var()	float			PreviewPlayRate;

/** SkeletalMesh used when previewing this AnimTree in the AnimTreeEditor. */
var()	editoronly SkeletalMesh			PreviewSkelMesh;

/** previewing of socket */
var()	editoronly SkeletalMesh			SocketSkelMesh;
var()	editoronly StaticMesh			SocketStaticMesh;
var()	Name							SocketName;

/** AnimSets used when previewing this AnimTree in the AnimTreeEditor. */
var()	editoronly array<AnimSet>			PreviewAnimSets;

/** MorphTargetSets used when previewing this AnimTree in the AnimTreeEditor. */
var()	editoronly array<MorphTargetSet>	PreviewMorphSets;

/** Saved position of camera used for previewing skeletal mesh in AnimTreeEditor. */
var		vector		PreviewCamPos;

/** Saved orientation of camera used for previewing skeletal mesh in AnimTreeEditor. */
var		rotator		PreviewCamRot;

/** Saved position of floor mesh used for in AnimTreeEditor preview window. */
var		vector		PreviewFloorPos;

/** Saved yaw rotation of floor mesh used for in AnimTreeEditor preview window. */
var		int			PreviewFloorYaw;

////// End AnimTree Editor support

cpptext
{
	virtual void	InitAnim(USkeletalMeshComponent* meshComp, UAnimNodeBlendBase* Parent);
	virtual void	GetBoneAtoms(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);

	/** Optimisation way to see if this is a UAnimTree */
	virtual UAnimTree* GetAnimTree() { return this; }


	void			UpdateAnimNodeSeqGroups(FLOAT DeltaSeconds);
	void			UpdateMasterNodesForGroup(FAnimGroup& AnimGroup);
	void			RepopulateAnimGroups();

	void			PostEditChange(UProperty* PropertyThatChanged);

	/** Get all SkelControls within this AnimTree. */
	void			GetSkelControls(TArray<USkelControlBase*>& OutControls);

	/** Get all MorphNodes within this AnimTree. */
	void			GetMorphNodes(TArray<class UMorphNodeBase*>& OutNodes);

	/** Call InitMorph on all morph nodes attached to the tree. */
	void			InitTreeMorphNodes(USkeletalMeshComponent* InSkelComp);

	/** Calls GetActiveMorphs on each element of the RootMorphNodes array. */
	void			GetTreeActiveMorphs(TArray<FActiveMorph>& OutMorphs);

	/** Make a copy of entire tree, including all AnimNodes and SkelControls. */
	UAnimTree*		CopyAnimTree(UObject* NewTreeOuter);

	/** Utility function for copying a selection of nodes, maintaining any references between them, but leaving any external references. */
	static void		CopyAnimNodes(const TArray<class UAnimNode*>& SrcNodes, UObject* NewOuter, TArray<class UAnimNode*>& DestNodes, TMap<class UAnimNode*,class UAnimNode*>& SrcToDestNodeMap);

	/** Utility function for copying a selection of controls, maintaining any references between them, but leaving any external references. */
	static void		CopySkelControls(const TArray<class USkelControlBase*>& SrcControls, UObject* NewOuter, TArray<class USkelControlBase*>& DestControls, TMap<class USkelControlBase*,class USkelControlBase*>& SrcToDestControlMap);

	/** Utility function for copying a selection of morph nodes, maintaining any references between them, but leaving any external references. */
	static void		CopyMorphNodes(const TArray<class UMorphNodeBase*>& SrcNodes, UObject* NewOuter, TArray<class UMorphNodeBase*>& DestNodes, TMap<class UMorphNodeBase*,class UMorphNodeBase*>& SrcToDestNodeMap);


	// UAnimNode interface
	virtual FIntPoint GetConnectionLocation(INT ConnType, INT ConnIndex);

	/** 
	* Draws this node in the AnimTreeEditor.
	*
	* @param	Canvas			The canvas to use.
	* @param	bSelected		TRUE if this node is selected.
	* @param	bShowWeight		If TRUE, show the global percentage weight of this node, if applicable.
	* @param	bCurves			If TRUE, render links as splines; if FALSE, render links as straight lines.
	*/
	virtual void DrawAnimNode(FCanvas* Canvas, UBOOL bSelected, UBOOL bShowWeight, UBOOL bCurves);
}


native final function	SkelControlBase		FindSkelControl(name InControlName);

native final function	MorphNodeBase		FindMorphNode(name InNodeName);

/** 
 *	When passing in TRUE, will cause tree to evaluate and then save resulting pose. From then on will continue to use that saved pose instead of re-evaluating the tree 
 *	This feature is turned off when the SkeletalMesh changes
 */
native final function						SetUseSavedPose(bool bUseSaved);

//
// Anim Groups
//

/** Add a node to an existing anim group */
native final function bool				SetAnimGroupForNode(AnimNodeSequence SeqNode, Name GroupName, optional bool bCreateIfNotFound);
/** Returns the master node driving synchronization for this group. */
native final function AnimNodeSequence	GetGroupSynchMaster(Name GroupName);
/** Returns the master node driving notifications for this group. */
native final function AnimNodeSequence	GetGroupNotifyMaster(Name GroupName);
/** Force a group at a relative position. */
native final function					ForceGroupRelativePosition(Name GroupName, FLOAT RelativePosition);
/** Get the relative position of a group. */
native final function float				GetGroupRelativePosition(Name GroupName);
/** Adjust the Rate Scale of a group */
native final function					SetGroupRateScale(Name GroupName, FLOAT NewRateScale);
/** Get the Rate Scale of a group */
native final function float				GetGroupRateScale(Name GroupName);
/** 
 * Returns the index in the AnimGroups list of a given GroupName.
 * If group cannot be found, then INDEX_NONE will be returned.
 */
native final function INT				GetGroupIndex(Name GroupName);

defaultproperties
{
	Children(0)=(Name="Child",Weight=1.0)
	bFixNumChildren=TRUE
	PreviewPlayRate=1.f
}
