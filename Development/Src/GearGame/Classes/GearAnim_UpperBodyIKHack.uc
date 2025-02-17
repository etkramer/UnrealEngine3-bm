
/**
 * GearAnim_UpperBodyIKHack
 *
 * Copies the location of source bone to destination bone.
 * Used to fix the location of the IK bone chain when aiming with the rifle pose. So it moves along an arc
 * (FK blend of the arm), instead of a line (blend of the IK chain).
 * This basically moves the IK chain to the right hand bone locationl. Rotation remains untouched.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAnim_UpperBodyIKHack extends AnimNodeBlendBase
	native(Anim);

/** Internal cached pointer to MirrorMaster node */
var	transient GearAnim_Mirror_Master	CachedMirrorNode;
/** Internal cached pointer to GearPawn Owner. */
var transient GearPawn					GearPawnOwner;

/** Structure for duplicating bone information */
struct native BoneCopyInfo
{
	var()				Name	SrcBoneName;
	var()				Name	DstBoneName;
	var()				Vector	PositionOffset;
	var	transient const	INT		SrcBoneIndex;
	var	transient const	INT		DstBoneIndex;
};

var()	Array<BoneCopyInfo>	BoneCopyArray;
var()	Array<BoneCopyInfo>	BoneCopyArrayMirrored;

/** If TRUE, node is disabled for HeavyWeapons. */
var()	bool				bDisableForHeavyWeapons;
/** Disable for meatshield. Hostage/Kidnapper situation. */
var()	bool				bDisableForMeatShield;

/** Internal, array of required bones. Selected bones and their parents for local to component space transformation. */
var	transient	Array<byte>			RequiredBones;
var	transient	Array<byte>			RequiredBonesMirrored;

/** Internal, array of component space transform matrices */
var	transient	Array<Matrix>		BoneTM;
/** Keep track if node is disabled per weapon control. */
var transient	bool				bNodeDisabled;

/** List of nodes to get their weight from, to consider for position offset */
var()			Array<Name>			OffsetWeightNodesNamesList;
var transient	Array<AnimNode>		OffsetWeightNodesList;
var transient	bool				bUpdatedWeightNodesList;

cpptext
{
	// UObject interface
	virtual void PostEditChange(UProperty* PropertyThatChanged);

	/** Update cached list of required bones, use to transform skeleton from parent space to component space. */
	void UpdateListOfRequiredBones();
	void UpdateOffsetWeightNodesList();

	// AnimNode interface
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);
	virtual	void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
	virtual void GetBoneAtoms(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);
}

defaultproperties
{
	bFixNumChildren=TRUE
	Children(0)=(Name="Input",Weight=1.0)
	bDisableForHeavyWeapons=TRUE
	bDisableForMeatShield=TRUE
	bSkipTickWhenZeroWeight=TRUE
}
