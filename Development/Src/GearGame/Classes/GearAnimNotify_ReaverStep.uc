/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAnimNotify_ReaverStep extends AnimNotify
	native(Anim);

cpptext
{
	// AnimNotify interface.
	virtual void Notify( class UAnimNodeSequence* NodeSeq );
}

/**
 *	Indicates which leg is stepping.
 *	Order is AL, BL, CL, AR, BR, CR
 */
var()	int	LegIndex; 

/** How long IK will take to interpolate to new location */
var()	float	StepTime;

/** Indicates if foot is lifting or falling. */
var() const enum EReaverStepType
{
	ERST_FootDown,
	ERST_FootUp
} StepAction;


defaultproperties
{
	StepAction=ERST_FootUp
	StepTime=0.33
}