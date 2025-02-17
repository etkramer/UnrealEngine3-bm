/*=============================================================================
	SoundNodeAmbientNonLoop.h: Native SoundNodeAmbientNonLoop calls
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

protected:
	// USoundNode interface.
	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );

	/**
	 * Notifies the sound node that a wave instance in its subtree has finished.
	 *
	 * @param WaveInstance	WaveInstance that was finished
	 */
	virtual void NotifyWaveInstanceFinished( struct FWaveInstance* WaveInstance );

	/** 
	 * Pick which slot to play next. 
	 */
	INT PickNextSlot( void );
