
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAnim_BlendByTargetingMode extends GearAnim_BlendList
	native(Anim);

/** Internal cached pointer to GearPawn Owner */
var const	transient	GearPawn		GearPawnOwner;

/** Internal cached pointer to owner's weapon */
var const	transient	GearWeapon	Weapon;

/** When Aim Fire and Idle are the same, then only Idle will be triggered for both */
var()	bool	bMergeAimFiringIntoIdle;
/** When Aim Idle and Idle Ready are the same, then only Idle Ready will be triggered for both */
var()	bool	bMergeAimIdleIntoIdleReady;
/** When Aim and Downsights poses are the same, then only Aim will be triggered for both */
var()	bool	bMergeDownSightsIntoAim;
/** When Downsights Fire and Idle are the same, then only Idle will be triggered for both */
var()	bool	bMergeDownSightsFireIntoIdle;

/** Blend Time from Aim to Idle */
var()	float	Aim2IdleBlendOutTime;


cpptext
{
	virtual void OnCeaseRelevant();
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);
	virtual void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
};


defaultproperties
{
	bFixNumChildren=TRUE
	Children(0)=(Name="Idle/Ready")
	Children(1)=(Name="Aim+Idle")
	Children(2)=(Name="Aim+Fire")
	Children(3)=(Name="DownSights+Idle")
	Children(4)=(Name="DownSights+Fire")

	bForceChildFullWeightWhenBecomingRelevant=FALSE
	BlendType=ABT_EaseInOutExponent3
	Aim2IdleBlendOutTime=1.f

	ChildBlendInTime(0)=0.25f
	ChildBlendInTime(1)=0.20f
	ChildBlendInTime(2)=0.20f
	ChildBlendInTime(3)=0.20f
	ChildBlendInTime(4)=0.20f
}
