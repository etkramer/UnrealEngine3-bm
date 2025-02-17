
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class SeqAct_GearPlayerAnim extends SequenceAction
	native(Sequence);

cpptext
{
	virtual void	Activated();
	virtual UBOOL	UpdateOp(FLOAT DeltaTime);
	virtual void	DeActivated();
}

enum EPlayMode
{
	AnimPM_Rate,
	AnimPM_Duration,
};

var()	bool		bPlayUpperBodyOnly;
var()	EPlayMode	PlayMode;

var()	name	AnimName;
var()	float	StartTime;
var()	float	Duration;
var()	float	Rate;
var()	float	BlendInTime;
var()	float	BlendOutTime;
var()	bool	bLooping;
var()	bool	bOverride;

/** 
 * if TRUE, animation uses Root Motion 
 * So additionally, we're setting up the character for that.
 */
var()	bool	bUseRootMotion;

defaultproperties
{
	ObjName="Play GearPawn Anim"
	ObjCategory="Gear"

	InputLinks(0)=(LinkDesc="Play Anim")
	InputLinks(1)=(LinkDesc="Stop Anim")

	// define the base output link that this action generates (always assumed to generate at least a single output)
	OutputLinks(0)=(LinkDesc="Out")
	OutputLinks(1)=(LinkDesc="Finished")

	Rate=1
	Duration=1.0
	BlendOutTime=0.2f
	bOverride=TRUE
	bLatentExecution=TRUE
}
