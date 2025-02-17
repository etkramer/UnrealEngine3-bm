/**
 * NOTE:  If you have a looping sound the PlaybackTime will keep increasing.  And PlaybackTime
 * is what is used to get values from the Distributions.   So the Modulation will work the first
 * time through but subsequent times will not work for distributions with have a "size" to them.
 *
 * In short using a SoundNodeModulatorContinuous for looping sounds is not advised.
 * 
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/
class SoundNodeModulatorContinuous extends SoundNode
	native(Sound)
	collapsecategories
	hidecategories(Object)
	editinlinenew;

var()	rawdistributionfloat	VolumeModulation;
var()	rawdistributionfloat	PitchModulation;

cpptext
{
	// USoundNode interface.

	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
}

defaultproperties
{
	Begin Object Class=DistributionFloatUniform Name=DistributionVolume
		Min=1
		Max=1
	End Object
	VolumeModulation=(Distribution=DistributionVolume)

	Begin Object Class=DistributionFloatUniform Name=DistributionPitch
		Min=1
		Max=1
	End Object
	PitchModulation=(Distribution=DistributionPitch)
}
