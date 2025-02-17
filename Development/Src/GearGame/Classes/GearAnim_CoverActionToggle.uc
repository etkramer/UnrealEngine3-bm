
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAnim_CoverActionToggle extends GearAnim_BlendList
	native(Anim);

/** PreviousCoverActions to consider. */
var()	Array<ECoverAction>	PrevCoverAction;
/** CoverActions to consider. */
var()	Array<ECoverACtion>	CoverAction;
/** Cached pointer to GearPawnOwner */
var transient GearPawn	GearPawnOwner;
/** Cached values when node becomes relevant */
var ECoverAction		CachedPrevCoverAction, CachedCoverAction;

cpptext
{
	virtual void OnBecomeRelevant();
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);
	virtual	void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
	/** Parent node is requesting a blend out. Give node a chance to delay that. */
	virtual UBOOL		CanBlendOutFrom();
	/** parent node is requesting a blend in. Give node a chance to delay that. */
	virtual UBOOL		CanBlendTo();
};

defaultproperties
{
	bSkipTickWhenZeroWeight=TRUE
	bFixNumChildren=TRUE
	Children(0)=(Name="OFF")
	Children(1)=(Name="ON")

	ChildBlendInTime(0)=0.f
	ChildBlendInTime(1)=0.f
}
