/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SoundNodeOscillator extends SoundNode
	native(Sound)
	collapsecategories
	hidecategories(Object)
	editinlinenew;

var()	rawdistributionfloat	Amplitude;
var()	rawdistributionfloat	Frequency;
var()	rawdistributionfloat	Offset;
var()	rawdistributionfloat	Center;

var()	bool					bModulatePitch;
var()	bool					bModulateVolume;

cpptext
{
	// USoundNode interface.

	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
}

defaultproperties
{
	Begin Object Class=DistributionFloatUniform Name=DistributionAmplitude
		Min=0
		Max=0
	End Object
	Amplitude=(Distribution=DistributionAmplitude)

	Begin Object Class=DistributionFloatUniform Name=DistributionFrequency
		Min=0
		Max=0
	End Object
	Frequency=(Distribution=DistributionFrequency)

	Begin Object Class=DistributionFloatUniform Name=DistributionOffset
		Min=0
		Max=0
	End Object
	Offset=(Distribution=DistributionOffset)

	Begin Object Class=DistributionFloatUniform Name=DistributionCenter
		Min=0
		Max=0
	End Object
	Center=(Distribution=DistributionCenter)
}
