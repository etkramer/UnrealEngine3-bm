/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SoundNodeRandom extends SoundNode
	native(Sound)
	collapsecategories
	hidecategories(Object)
	editinlinenew;


var()		editfixedsize	array<float>		Weights;


/** 
 * Determines whether or not this SoundNodeRandom should randomize with or without
 * replacement.  
 *
 * WithoutReplacement means that only nodes left will be valid for 
 * selection.  So with that, you are guarenteed to have only one occurrence of the
 * sound played until all of the other sounds in the set have all been played.
 *
 * WithReplacement means that a node will be chosen and then placed back into the set.
 * So one could play the same sound over and over if the probabilities don't go your way :-)
 *
 **/
var() 						bool				bRandomizeWithoutReplacement;

/**
 * Internal state of which sounds have been played.  This is only used at runtime
 * to keep track of which sounds have been played
 **/
var 		transient		array<bool>			HasBeenUsed; // we are logically treating this as an array of bools

/** Counter var so we don't have to count all of the used sounds each time we choose a sound **/
var 		transient		int					NumRandomUsed;



cpptext
{
	// USoundNode interface.
	
	virtual void GetNodes( class UAudioComponent* AudioComponent, TArray<USoundNode*>& SoundNodes );
	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );

	virtual INT GetMaxChildNodes() { return -1; }
	
	// Editor interface.
	
	virtual void InsertChildNode( INT Index );
	virtual void RemoveChildNode( INT Index );
	
	// USoundNodeRandom interface
	void FixWeightsArray();
	void FixHasBeenUsedArray();


	/**
	 * Called by the Sound Cue Editor for nodes which allow children.  The default behaviour is to
	 * attach a single connector. Dervied classes can override to e.g. add multiple connectors.
	 */
	virtual void CreateStartingConnectors();
}

defaultproperties
{
	bRandomizeWithoutReplacement=TRUE
	NumRandomUsed=0
}
