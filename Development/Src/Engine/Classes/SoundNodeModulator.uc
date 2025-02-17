/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SoundNodeModulator extends SoundNode
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
		Min=0.9
		Max=1.1
	End Object
	VolumeModulation=(Distribution=DistributionVolume)

	Begin Object Class=DistributionFloatUniform Name=DistributionPitch
		Min=0.9
		Max=1.1
	End Object
	PitchModulation=(Distribution=DistributionPitch)
}
