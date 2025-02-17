
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAnim_DirectionalMove2Idle extends GearAnim_BlendAnimsByDirection
	native(Anim);

/** Directional Transition information. */
struct native DTransInfo
{
	/** Name of transition */
	var()	Name	TransitionName;
	var()	Name	AnimName_Fd;
	var()	Name	AnimName_Bd;
	var()	Name	AnimName_Lt;
	var()	Name	AnimName_Rt;
};

var() Array<DTransInfo>	DTransList;

cpptext
{
	void	SetTransition(FName TransitionName, FLOAT StartPosition);
	UBOOL	IsTransitionFinished(FLOAT DeltaTime);
	void	StopTransition();
}

defaultproperties
{
	Anims(0)=(AnimName="",Weight=1.0)
	Anims(1)=(AnimName="")
	Anims(2)=(AnimName="")
	Anims(3)=(AnimName="")

	DTransList(0)=(TransitionName="Walk_RightFoot",AnimName_Fd="AR_Walk_Fwd2Idle_RightFoot",AnimName_Bd="AR_Walk_Bwd2Idle_RightFoot",AnimName_Lt="AR_Walk_Lt2Idle_RightFoot",AnimName_Rt="AR_Walk_Rt2Idle_RightFoot")
	DTransList(1)=(TransitionName="Walk_LeftFoot",AnimName_Fd="AR_Walk_Fwd2Idle_LeftFoot",AnimName_Bd="AR_Walk_Bwd2Idle_LeftFoot",AnimName_Lt="AR_Walk_Lt2Idle_LeftFoot",AnimName_Rt="AR_Walk_Rt2Idle_LeftFoot")
	DTransList(2)=(TransitionName="Run",AnimName_Fd="AR_Run_Fwd2Idle",AnimName_Bd="AR_Run_Bwd2Idle",AnimName_Lt="AR_Run_Lt2Idle",AnimName_Rt="AR_Run_Rt2Idle")
}
