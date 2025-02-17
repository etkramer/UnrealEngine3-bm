
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAnim_BlendByAngularVelocity3 extends GearAnim_BlendList
	native(Anim);

/** Cached pointer to gearpawn owner */
var		const transient GearPawn	GearPawnOwner;
/** If we should use Owner's Interaction Pawn instead. */
var()	const bool					bUseInteractionPawn;

var()	bool	bUseBaseVelocity;
var()	bool	bInvertWhenMovingBackwards;
var()	FLOAT	AngularVelDeadZoneThreshold;
var()	FLOAT	YawAngularVelSpeedScale;
var()	FLOAT	MaxRateScale;
var()	FLOAT	StartRelPose;
var()	Name	ScaleBySpeedSynchGroupName;

var		transient	INT	LastYaw;

var		transient float		YawVelHistory[10];
var		transient int		YawVelSlot;

var		transient Array<AnimNodeSequence>	AnimSeqNodesRight, AnimSeqNodesLeft;

cpptext
{
	virtual void InitAnim(USkeletalMeshComponent* meshComp, UAnimNodeBlendBase* Parent);
	virtual void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
};

defaultproperties
{
	ScaleBySpeedSynchGroupName="RunWalk"
	AngularVelDeadZoneThreshold=1.f
	YawAngularVelSpeedScale=0.0067f
	MaxRateScale=1.f
	StartRelPose=0.f
	bInvertWhenMovingBackwards=TRUE

	bSkipTickWhenZeroWeight=TRUE
	Children(0)=(Name="Left")
	Children(1)=(Name="Idle")
	Children(2)=(Name="Right")

	ChildBlendInTime(0)=0.33f
	ChildBlendInTime(1)=0.33f
	ChildBlendInTime(2)=0.33f
}
