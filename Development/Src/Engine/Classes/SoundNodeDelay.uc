/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/
class SoundNodeDelay extends SoundNode
	native(Sound)
	collapsecategories
	hidecategories(Object)
	editinlinenew;

var() rawdistributionfloat DelayDuration;

cpptext
{
	// USoundNode interface.
	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );

	/**
	 * Returns the maximum duration this sound node will play for. 
	 * 
	 * @return maximum duration this sound will play for
	 */
	virtual FLOAT GetDuration();
}

defaultproperties
{
	Begin Object Class=DistributionFloatUniform Name=DistributionDelayDuration
		Min=0
		Max=0
	End Object
	DelayDuration=(Distribution=DistributionDelayDuration)
}
