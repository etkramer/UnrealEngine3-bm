
/** 
 * GearAnim_TurnInPlace
 * Blender to handle turning in place transitions
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAnim_TurnInPlace extends AnimNodeBlend
	native(Anim);

/** Node needs to be initialized to track Pawn rotations */
var		const transient bool		bInitialized;

/** Track Pawn rotation changes */
var		const transient int			LastPawnYaw;
/** Pawn rotation rate for this frame */
var		const transient float		PawnRotationRate;

/** TurnInPlace transition animation Root Bone Rotation tracking */
var		const transient	int			LastRootBoneYaw;
var		const transient	bool		bRootRotInitialized;

/** Current Yaw offset between camera and Pawn */
var()	const transient	int			YawOffset;

/** 
 * If TRUE, this node will try to delay his parent from blending to another child,
 * So the turn in place animation can play fully.
 */
var()	bool						bDelayBlendOutToPlayAnim;

/** relative offset, applied to AimOffset nodes */
var		const transient	float		RelativeOffset;

/** Internal cached pointer to GearPawn Owner */
var		const transient GearPawn			GearPawnOwner;
var		const transient GearPawn			CachedBaseGearPawn;
var()	GearAnim_MovementNode.EOwnerType	OwnerType;

/** AimOffsets affected by rotation compensation. */
var		Array<GearAnim_AimOffset>	OffsetNodes;

struct native RotTransitionInfo
{
	var()	Float	RotationOffset;
	var()	Name	TransName;
};

var()	Array<RotTransitionInfo>	RotTransitions;

var()	float						TransitionBlendInTime;
var()	float						TransitionBlendOutTime;
var		const bool					bPlayingTurnTransition;
var		const INT					CurrentTransitionIndex;

var()	float						TransitionThresholdAngle;

/** 
 * Internal, cached arra of player nodes.
 * Used to play different types of transition animations.
 */
var		Array<GearAnim_TurnInPlace_Player>	PlayerNodes;

cpptext
{
	// AnimNode interface
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);
	virtual	void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
	virtual void GetBoneAtoms(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);
	virtual void OnBecomeRelevant();
	virtual void OnCeaseRelevant();

	/** Get an AnimNodeSequence playing a transition animation */
	UAnimNodeSequence* GetAPlayerNode();

	/** Parent node is requesting a blend out. Give node a chance to delay that. */
	virtual UBOOL	CanBlendOutFrom();

	virtual FLOAT GetSliderPosition(INT SliderIndex, INT ValueIndex);
	virtual void HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue);
	virtual FString GetSliderDrawValue(INT SliderIndex);
}

defaultproperties
{
	TransitionBlendInTime=0.15f
	TransitionBlendOutTime=0.5f
	BlendType=ABT_Cubic

	bSkipTickWhenZeroWeight=TRUE
	Children(0)=(Name="Source",Weight=1.0)
	Children(1)=(Name="TurnTransition")

	TransitionThresholdAngle=4096.f
	RotTransitions(0)=(RotationOffset=+16384.f,TransName="Rt_90")
	RotTransitions(1)=(RotationOffset=+32768.f,TransName="Rt_180")
	RotTransitions(2)=(RotationOffset=-16384.f,TransName="Lt_90")
	RotTransitions(3)=(RotationOffset=-32768.f,TransName="Lt_180")
}
