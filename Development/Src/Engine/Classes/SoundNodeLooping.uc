/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SoundNodeLooping extends SoundNode
	native(Sound)
	collapsecategories
	hidecategories(Object)
	editinlinenew;

var()	bool					bLoopIndefinitely;
var()	rawdistributionfloat	LoopCount;

cpptext
{	
	/**
	 * Notifies the sound node that a wave instance in its subtree has finished.
	 *
	 * @param WaveInstance	WaveInstance that was finished 
	 */
	virtual void NotifyWaveInstanceFinished( struct FWaveInstance* WaveInstance );

	/**
	 * Returns whether the node is finished after having been notified of buffer
	 * being finished.
	 *
	 * @param	AudioComponent	Audio component containing payload data
	 * @return	TRUE if finished, FALSE otherwise.
	 */
	virtual UBOOL IsFinished( class UAudioComponent* AudioComponent );

	/** 
	 * Returns the maximum distance this sound can be heard from. Very large for looping sounds as the
	 * player can move into the hearable range during a loop.
	 */
	virtual FLOAT MaxAudibleDistance( FLOAT CurrentMaxDistance ) { return WORLD_MAX; }
	
	/** 
	 * Returns the duration of the sound accounting for the number of loops.
	 */
	virtual FLOAT GetDuration();

	/** 
	 * Returns whether the sound is looping indefinitely or not.
	 */
	virtual UBOOL IsLoopingIndefinitely( void );

	/** 
	 * Process this node in the sound cue
	 */
	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
}


defaultproperties
{
	bLoopIndefinitely=TRUE
	Begin Object Class=DistributionFloatUniform Name=DistributionLoopCount
		Min=1000000
		Max=1000000
	End Object
	LoopCount=(Distribution=DistributionLoopCount)
}
