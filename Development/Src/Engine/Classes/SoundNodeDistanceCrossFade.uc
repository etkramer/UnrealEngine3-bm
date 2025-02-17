/**
 * SoundNodeDistanceCrossFade
 * 
 * This node's purpose is to play different sounds based on the distance to the listener.  
 * The node mixes between the N different sounds which are valid for the distance.  One should
 * think of a SoundNodeDistanceCrossFade as Mixer node which determines the set of nodes to
 * "mix in" based on their distance to the sound.
 * 
 * Example:
 * You have a gun that plays a fire sound.  At long distances you want a different sound than
 * if you were up close.   So you use a SoundNodeDistanceCrossFade which will calculate the distance
 * a listener is from the sound and play either:  short distance, long distance, mix of short and long sounds.
 *
 * A SoundNodeDistanceCrossFade differs from an SoundNodeAttenuation in that any sound is only going
 * be played if it is within the MinRadius and MaxRadius.  So if you want the short distance sound to be 
 * heard by people close to it, the MinRadius should probably be 0
 *
 * The volume curve for a SoundNodeDistanceCrossFade will look like this:
 *
 *                          Volume (of the input) 
 *    FadeInDistance.Max --> _________________ <-- FadeOutDistance.Min
 *                          /                 \
 *                         /                   \
 *                        /                     \
 * FadeInDistance.Min -->/                       \ <-- FadeOutDistance.Max
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class SoundNodeDistanceCrossFade extends SoundNode
	native(Sound)
	collapsecategories
	hidecategories(Object)
	editinlinenew;

struct native DistanceDatum
{
	/** 
	 * The FadeInDistance at which to start hearing this sound.  If you want to hear the sound 
	 * up close then setting this to 0 might be a good option.
	 **/
	var() rawdistributionfloat FadeInDistance;

	/**
	 * The FadeOutDistance is where hearing this sound will end.
	 **/
	var() rawdistributionfloat FadeOutDistance;

	/** The volume for which this Input should be played **/
	var() float Volume;

	structdefaultproperties
	{
		Volume=1.0f
	}
};

/**
 * Each input needs to have the correct data filled in so the SoundNodeDistanceCrossFade is able
 * to determine which sounds to play
 **/
var() export editfixedsize array<DistanceDatum>	CrossFadeInput;


cpptext
{
	// USoundNode interface.

	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
	virtual INT GetMaxChildNodes() { return -1; }

	/**
	 * DistanceCrossFades have two connectors by default.
	 */
	virtual void CreateStartingConnectors();
	virtual void InsertChildNode( INT Index );
	virtual void RemoveChildNode( INT Index );

	virtual FLOAT MaxAudibleDistance( FLOAT CurrentMaxDistance );
}

defaultproperties
{
}
