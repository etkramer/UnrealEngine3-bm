/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SoundNodeConcatenator extends SoundNode
	native(Sound)
	collapsecategories
	hidecategories(Object)
	editinlinenew;

/** A volume for each input.  Automatically sized. */
var() export editfixedsize array<float>	InputVolume;

cpptext
{
	// USoundNode interface.
	virtual void GetNodes( class UAudioComponent* AudioComponent, TArray<USoundNode*>& SoundNodes );
	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );

	virtual INT GetMaxChildNodes() { return -1; }	

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
	UBOOL IsFinished( class UAudioComponent* AudioComponent );

	/**
	 * Returns the maximum duration this sound node will play for. 
	 * 
	 * @return maximum duration this sound will play for
	 */
	virtual FLOAT GetDuration();

	/**
	 * Concatenators have two connectors by default.
	 */
	virtual void CreateStartingConnectors();

	/**
	 * Overloaded to add an entry to InputVolume.
	 */
	virtual void InsertChildNode( INT Index );

	/**
	 * Overloaded to remove an entry from InputVolume.
	 */
	virtual void RemoveChildNode( INT Index );
}

defaultproperties
{
}
