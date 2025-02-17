
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAnim_MovementNode extends AnimNodeBlendList
		native(Anim);

enum ESpeedType
{	
	/** Velocity drives everything. */
	EST_Velocity,

	/** 
	 * Acceleration tells if Pawn is moving or not. 
	 * MaxSpeed tells which movement channel to play. 
	 */
	EST_AccelAndMaxSpeed,

	/** Driven by Root Motion */
	EST_RootMotion,
};

/** How should this node monitor speed? */
var()	ESpeedType	SpeedType;

enum EOwnerType
{
	/** Use Owner. */
	EOT_Owner,
	/** Use Owner's Base. */
	EOT_Base,
};
/** Which owner should we use */
var()	EOwnerType	OwnerType;

/** Scale Animations Play Rate by Speed */
var()	bool		bScaleAnimationsPlayRateBySpeed;

/** 
 * If Pawn's base speed is scaled (GroundSpeed), then scale all speed constraints the same way. 
 * For instance COG and Locusts have different GroundSpeed defaults, but use the same tree.
 */
var()	bool		bScaleConstraintsByBaseSpeed;

/** 
 * If TRUE, this node should handle transitions.
 * Set FALSE, if this node should be passive about transitions
 * And not cancel the special move.
 */
var()	bool		bShouldHandleTransitions;

/** Internal cached pointer to GearPawn Owner */
var	const transient GearPawn	GearPawnOwner;
/** Cached pointer to GearPawn to use for monitoring movement. */
var const transient	GearPawn	CachedWorkingGearPawn;

/** How fast owner is moving this frame */
var		float	Speed;

/** Time to blend out of idle */
var()	float	IdleBlendOutTime;
/** How fast to blend when going up */
var()	float	BlendUpTime;		
/** How fast to blend when going down */
var()	float	BlendDownTime;
/** When should we start blending back down */
var()	float	BlendDownPerc;

/** 
 * Start position of the movement cycle.
 * Used to synchronize movement group when starting to move.
 */
var()	float	MoveCycleFirstStepStartPosition;

/** Synchronization node info. To access normalized movement info */
var		const transient	AnimTree	RootNode;
var()	Name						Name_SynchGroupName;

var()			float		TransitionBlendOutTime;

/** Internal flag to tell if node is player a movement 2 idle transition */
var				bool		bPlayingTransitionToIdle;

/** list of nodes that can be used for transitions */
var		Array<GearAnim_DirectionalMove2Idle>		TransitionNodes;

// internal timing variables. To find out when transitions can be triggered
var	const transient	float	PrevGroupRelPos;
var	const transient	float	GroupRelPos;

enum EMoveTransChannel
{
	EMTC_Idle,
	EMTC_Transition,
	EMTC_Walk,
	EMTC_Run,
	EMTC_RoadieRun
};

/** Transition Range information. */
struct native TransInfo
{
	/** Name of transition */
	var()	Name		TransName;
	
	/** Range of transition */
	var()	vector2d	Range;
	
	/** Custom transition blend time */
	var()	float		BlendTime;
	
	/** 
	 * Adjust transition anim start position by offset from 
	 * effective transition point to ideal transition point.
	 * if transition happens late, then this corrects the 
	 * transition start pos to offert a smoother transition.
	 */
	var()	bool		bAdjustAnimPos;

	/** There is no transition animation, so blend to idle pose instead. */
	var()	bool		bNoTransitionAnim;

	/** Should this transition force the player to aim (raise his gun). */
	var()	bool		bForcePlayerToAim;
};

struct native MovementDef
{
	/** Default Speed of movement */
	var()	float					BaseSpeed;

	/** List of potential transitions from movement to idle */
	var()	Array<TransInfo>		Move2IdleTransitions;

	/** Cached list of seq nodes playing animations. To scale their play rate based on speed */
	var		Array<AnimNodeSequence>	SeqNodes;
};

/** Definition of movements */
var() Array<MovementDef> Movements;

/** Last Transition Played, to resume transitions. */
var const transient native pointer LastTransInfo{FTransInfo};

var()	float	TransWeightResumeTheshold;

cpptext
{
	/** Perform node initialization. */
	void InitializeNode();

	/** 
	 * Returns the Pawn's current speed.
	 * Takes into account the SpeedType.
	 */
	FLOAT GetCurrentSpeed();

	/** 
	 * See if movement has reached a transition point in the given cycle, 
	 * If that's the case, the transition information is returned. Otherwise NULL is returned.
	 */
	FTransInfo*	GetTransitionInfo(TArray<FTransInfo> &CycleTransInfoList);

	/** Get WorkingPawn */
	AGearPawn*	GetWorkingPawn();
	void		StartTransition(FTransInfo* Info);
	void		ResumeTransition(FTransInfo* Info);
	UBOOL		IsTransitionFinished(FLOAT DeltaTime);
	void		StopTransition(UBOOL bAbortTransition);

	// AnimNode interface
	virtual void InitAnim(USkeletalMeshComponent* meshComp, UAnimNodeBlendBase* Parent);

	/**
	 */
	virtual	void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);

	virtual INT GetNumSliders() const { return 1; }
	virtual FLOAT GetSliderPosition(INT SliderIndex, INT ValueIndex);
	virtual void HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue);
	virtual FString GetSliderDrawValue(INT SliderIndex);
}


defaultproperties
{
	bFixNumChildren=TRUE
	Children(EMTC_Idle)			=(Name="Idle")
	Children(EMTC_Transition)	=(Name="Transition")
	Children(EMTC_Walk)			=(Name="Walk")
	Children(EMTC_Run)			=(Name="Run")
	Children(EMTC_RoadieRun)	=(Name="RoadieRun")

	Movements(EMTC_Walk)={(	
		BaseSpeed=100,
		Move2IdleTransitions=(	(TransName="Walk_RightFoot",Range=(X=0,Y=0.01),BlendTime=0.1,bAdjustAnimPos=TRUE),
								(TransName="Walk_LeftFoot",Range=(X=0.51,Y=0.52),BlendTime=0.1,bAdjustAnimPos=TRUE)	)
	)}

	Movements(EMTC_Run)={(	
		BaseSpeed=300,
		Move2IdleTransitions=((TransName="Run",Range=(X=0,Y=1),BlendTime=0.4,bForcePlayerToAim=TRUE))
	)}

	Movements(EMTC_RoadieRun)={(
		BaseSpeed=450,
		Move2IdleTransitions=((TransName="Run",Range=(X=0,Y=1),BlendTime=0.4,bForcePlayerToAim=TRUE))
	)}

	SpeedType=EST_Velocity
	OwnerType=EOT_Owner
	bScaleAnimationsPlayRateBySpeed=TRUE
	bScaleConstraintsByBaseSpeed=TRUE
	bShouldHandleTransitions=TRUE

	IdleBlendOutTime=0.15f
	BlendUpTime=0.4f
    BlendDownTime=0.3f
    BlendDownPerc=0.33f
	MoveCycleFirstStepStartPosition=0.3f

	TransWeightResumeTheshold=0.5f

	TransitionBlendOutTime=0.15f

	Name_SynchGroupName="RunWalk"
}
	