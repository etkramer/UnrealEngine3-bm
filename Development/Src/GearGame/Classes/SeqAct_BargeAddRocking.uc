/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class SeqAct_BargeAddRocking extends SequenceAction
	native(Sequence);

var()		RawDistributionFloat	PitchVel;
var()		RawDistributionFloat	RollVel;

defaultproperties
{
	ObjName="Rock Barge"
	ObjCategory="Gear"

	Begin Object Class=DistributionFloatUniform Name=DistributionPitchVel
		Min=-1000
		Max=1000
	End Object
	PitchVel=(Distribution=DistributionPitchVel)

	Begin Object Class=DistributionFloatUniform Name=DistributionRollVel
		Min=-1000
		Max=1000
	End Object
	RollVel=(Distribution=DistributionRollVel)
}