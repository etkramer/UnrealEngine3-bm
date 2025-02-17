/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * This SoundNode uses UEngine::bAllowMatureLanguage to determine whether child nodes
 * that have USoundNodeWave::bMature=TRUE should be played.
 */
class SoundNodeMature extends SoundNode
	native(Sound)
	collapsecategories
	hidecategories(Object)
	editinlinenew;

cpptext
{
	// USoundNode interface.
	virtual void GetNodes( class UAudioComponent* AudioComponent, TArray<USoundNode*>& SoundNodes );
	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );

	virtual INT GetMaxChildNodes();
}

defaultproperties
{
}
