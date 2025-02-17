
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 * Node to play a transition animation referenced by transition type.
 */

class GearAnim_TurnInPlace_Player extends AnimNodeSequenceBlendBase
	native(Anim);

struct native TIP_Transition
{
	var()	Name	TransName;
	var()	Name	AnimName;
};

var()	Array<TIP_Transition>	TIP_Transitions;

/** Transition blend time */
var()	FLOAT			TransitionBlendTime;
var		transient INT	ActiveChildIndex;
var		transient FLOAT	BlendTimeToGo;

cpptext
{
	/** Play a turn in place transition */
	void PlayTransition(FName TransitionName, FLOAT BlendTime);
	void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
	void SetActiveChild(INT ChildIndex, FLOAT BlendTime);
}

defaultproperties
{
	TIP_Transitions(0)=(TransName="Rt_90",	AnimName="AR_Idle_Ready_Turn_Rt_90")
	TIP_Transitions(1)=(TransName="Rt_180",	AnimName="AR_Idle_Ready_Turn_Rt_180")
	TIP_Transitions(2)=(TransName="Lt_90",	AnimName="AR_Idle_Ready_Turn_Lt_90")
	TIP_Transitions(3)=(TransName="Lt_180",	AnimName="AR_Idle_Ready_Turn_Lt_180")

	ActiveChildIndex=0
	Anims(0)=(Weight=1.0)
	Anims(1)=()

	TransitionBlendTime=0.33f

	RootRotationOption[0]=RRO_Discard
	RootRotationOption[1]=RRO_Discard
	RootRotationOption[2]=RRO_Discard
	bSkipTickWhenZeroWeight=TRUE
}