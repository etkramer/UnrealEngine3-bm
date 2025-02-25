//=============================================================================
// ExampleDamageType derives from DamageType, the base class of all damagetypes.
// this and its subclasses are never spawned, just used as information holders
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class ExampleDamageType extends DamageType
	abstract;
	
//
// Damage type sample class to show how forcefeedback can be hooked in
//
defaultproperties
{
    Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveform0
        Samples(0)=(LeftAmplitude=100,RightAmplitude=40,LeftFunction=WF_Noise,RightFunction=WF_Constant,Duration=0.5)
	End Object
    Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveform1
        Samples(0)=(LeftAmplitude=100,RightAmplitude=100,LeftFunction=WF_Constant,RightFunction=WF_Constant,Duration=0.75)
	End Object
	DamagedFFWaveform=ForceFeedbackWaveform0
	KilledFFWaveform=ForceFeedbackWaveform1

	bCausesFracture=TRUE
}


