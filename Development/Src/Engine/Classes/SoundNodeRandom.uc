class SoundNodeRandom extends SoundNode
    native(Sound)
    editinlinenew
    collapsecategories;

var() editfixedsize array<float> Weights;
var() bool bRandomizeWithoutReplacement;
var transient array<bool> HasBeenUsed;
var transient int NumRandomUsed;
var transient int LastRandomUsed;
var transient int NextRandomToForce;

// Export USoundNodeRandom::execPickNextNodeInAdvance(FFrame&, void* const)
native function PickNextNodeInAdvance();

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
    bRandomizeWithoutReplacement=true
    NextRandomToForce=-1
}
