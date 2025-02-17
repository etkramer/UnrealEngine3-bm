/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SoundNode extends Object
	native( Sound )
	abstract
	hidecategories(Object)
	editinlinenew;

var native const int					NodeUpdateHint;
var array<SoundNode>	ChildNodes;

cpptext
{
	// USoundNode interface.
	
	/**
	 * Notifies the sound node that a wave instance in its subtree has finished.
	 *
	 * @param WaveInstance	WaveInstance that was finished 
	 */
	virtual void NotifyWaveInstanceFinished( struct FWaveInstance* WaveInstance ) {}

	/**
	 * Returns whether the node is finished after having been notified of buffer
	 * being finished.
	 *
	 * @param	AudioComponent	Audio component containing payload data
	 * @return	TRUE if finished, FALSE otherwise.
	 */
	virtual UBOOL IsFinished( class UAudioComponent* /*Unused*/) { return TRUE; }

	/** 
	 * Returns the maximum distance this sound can be heard from.
	 */
	virtual FLOAT MaxAudibleDistance(FLOAT CurrentMaxDistance) { return CurrentMaxDistance; }
	
	/** 
	 * Returns the maximum duration this sound node will play for. 
	 */
	virtual FLOAT GetDuration();

	/** 
	 * Returns whether the sound is looping indefinitely or not.
	 */
	virtual UBOOL IsLoopingIndefinitely( void ) { return( FALSE ); }

	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
	virtual void GetNodes( class UAudioComponent* AudioComponent, TArray<USoundNode*>& SoundNodes );
	virtual void GetAllNodes( TArray<USoundNode*>& SoundNodes ); // Like above but returns ALL (not just active) nodes.

	virtual INT GetMaxChildNodes() { return 1; }

	// Tool drawing
	virtual void DrawSoundNode(FCanvas* Canvas, const struct FSoundNodeEditorData& EdData, UBOOL bSelected);
	virtual FIntPoint GetConnectionLocation(FCanvas* Canvas, INT ConnType, INT ConnIndex, const struct FSoundNodeEditorData& EdData);

	/**
	 * Helper function to reset bFinished on wave instances this node has been notified of being finished.
	 *
	 * @param	AudioComponent	Audio component this node is used in.
	 */
	void ResetWaveInstances( UAudioComponent* AudioComponent );

	// Editor interface.

	/** Get the name of the class used to help out when handling events in UnrealEd.
	 * @return	String name of the helper class.
	 */
	virtual const FString	GetEdHelperClassName() const
	{
		return FString( TEXT("UnrealEd.SoundNodeHelper") );
	}

	/**
	 * Called by the Sound Cue Editor for nodes which allow children.  The default behaviour is to
	 * attach a single connector. Dervied classes can override to eg add multiple connectors.
	 */
	virtual void CreateStartingConnectors();
	virtual void InsertChildNode( INT Index );
	virtual void RemoveChildNode( INT Index );
	
	/**
	 * Called when a property value from a member struct or array has been changed in the editor.
	 */
	virtual void PostEditChange( UProperty* PropertyThatChanged );	
}
