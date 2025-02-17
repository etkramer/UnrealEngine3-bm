
/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class UTAnimNodeCopyBoneTranslation extends AnimNodeBlendBase
	native(Animation);

/** Structure for duplicating bone information */
struct native BoneCopyInfo
{
	var()			Name	SrcBoneName;
	var()			Name	DstBoneName;
	var		const	INT		SrcBoneIndex;
	var		const	INT		DstBoneIndex;
};

var	AnimNodeAimOffset	CachedAimNode;
var	name OldAimProfileName;

var()	Array<BoneCopyInfo>	DefaultBoneCopyArray;
var()	Array<BoneCopyInfo>	DualWieldBoneCopyArray;

var		Array<BoneCopyInfo> ActiveBoneCopyArray;

/** Internal, array of required bones. Selected bones and their parents for local to component space transformation. */
var		Array<byte>			RequiredBones;

/** Cached list of UtAnimNodeSeqWeap nodes - this node will call WeapTypeChanged when weapon type changes. */
var		Array<UTAnimNodeSeqWeap>		SeqWeaps;

/** Cached list of UTAnimBlendByWeapType nodes - this node will call WeapTypeChanged when weapon type changes. */
var		Array<UTAnimBlendByWeapType>	WeapTypeBlends;

cpptext
{
	// UObject interface
	virtual void PostEditChange(UProperty* PropertyThatChanged);

	/** Update cached list of required bones, use to transform skeleton from parent space to component space. */
	void UpdateListOfRequiredBones(FName AimProfileName);

	// AnimNode interface
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);
	virtual void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );
	virtual void GetBoneAtoms(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);
}

defaultproperties
{
	Children(0)=(Name="Input",Weight=1.0)
	bFixNumChildren=TRUE
}
