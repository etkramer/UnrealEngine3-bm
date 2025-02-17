/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SoundNodeMixer extends SoundNode
	native(Sound)
	collapsecategories
	hidecategories(Object)
	editinlinenew;

/** A volume for each input.  Automatically sized. */
var() export editfixedsize array<float>	InputVolume;

cpptext
{
	// USoundNode interface.

	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
	virtual INT GetMaxChildNodes() { return -1; }

	/**
	 * Mixers have two connectors by default.
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
