
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAnim_BlendByTransition extends GearAnim_BlendList
	native(Anim);

/** Internal cached pointer to GearPawn Owner */
var const	transient	GearPawn		GearPawnOwner;
/** If set, will bypass intro animation if previous cover action matches what is set here. */
var()	Array<ECoverAction>	PrevCoverActionBypassIntro;
/** If set, will bypass outro animation if CoverAction matches what is set here. */
var()	Array<ECoverACtion>	CoverActionBypassOutro;

var()	bool	b360AimingBypassIntro;
var()	bool	b360AimingBypassOutro;

// Internal
var transient	bool	bPlayingIntro;
var	transient	bool	bPlayingOutro;
var	transient	bool	bPlayedOutro;

var()			bool	bUpdatePawnActionFiringFlag;

cpptext
{
	/** Parent node is requesting a blend out. Give node a chance to delay that. */
	virtual UBOOL	CanBlendOutFrom();

	/** parent node is requesting a blend in. Give node a chance to delay that. */
	virtual UBOOL	CanBlendTo();

	/** Starts all intro nodes, and sets blend target on intro child */
	UBOOL	PlayIntro();
	UBOOL	PlayOutro();
	void	PlayMain();

	/** Stops all Intro nodes */
	void	StopIntro();
	void	StopOutro();

	/** Returns TRUE if Child is an Intro node */
	UBOOL	IsAnIntroNode(UAnimNodeSequence* Child);
	UBOOL	IsAnOutroNode(UAnimNodeSequence* Child);

	/** Get notification that this node has become relevant for the final blend. ie TotalWeight is now > 0 */
	virtual void OnBecomeRelevant();
	virtual void OnCeaseRelevant();

	/** Notification to this blend that a child UAnimNodeSequence has reached the end and stopped playing. Not called if child has bLooping set to true or if user calls StopAnim. */
	virtual void OnChildAnimEnd(UAnimNodeSequence* Child, FLOAT PlayedTime, FLOAT ExcessTime);

	virtual void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);
	virtual	void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
};


defaultproperties
{
	bSkipTickWhenZeroWeight=TRUE
	bUpdatePawnActionFiringFlag=TRUE
	bFixNumChildren=TRUE
	Children(0)=(Name="Intro")
	Children(1)=(Name="Anim")
	Children(2)=(Name="Outro")

	ChildBlendInTime(0)=0.f
	ChildBlendInTime(1)=0.15f
	ChildBlendInTime(2)=0.15f
}
