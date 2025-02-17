/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/** 
 * Sound node that takes a runtime parameter for the wave to play
 */
class SoundNodeWaveParam extends SoundNode
	native( Sound )
	collapsecategories
	hidecategories(Object)
	editinlinenew;

/** The name of the wave parameter to use to look up the SoundNodeWave we should play */
var() name WaveParameterName;

cpptext
{
	/** 
	 * Return the maximum number of child nodes; normally 0 to 2
	 */
	virtual INT GetMaxChildNodes()
	{
		return( 1 );
	}
	
	/** 
	 * Gets the time in seconds of the associated sound data
	 */	
	virtual FLOAT GetDuration( void );
	
	/** 
	 * USoundNode interface.
	 */
	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
	
	/** 
	 *
	 */
	virtual void GetNodes( class UAudioComponent* AudioComponent, TArray<USoundNode*>& SoundNodes );
}
