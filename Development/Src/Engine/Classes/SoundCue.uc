/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SoundCue extends Object
	hidecategories(object)
	native;

struct native export SoundNodeEditorData
{
	var	native const int NodePosX;
	var native const int NodePosY;
};

/** Sound group this sound cue belongs to */
var() editconst				Name									SoundGroup;
var							SoundNode								FirstNode;
var		const native		Map{USoundNode*,FSoundNodeEditorData}	EditorData;
var		transient			float									MaxAudibleDistance;
var()						float									VolumeMultiplier;
var()						float									PitchMultiplier;
var							float									Duration;

/** Reference to FaceFX AnimSet package the animation is in */
//var() editoronly notforconsole FaceFXAnimSet						FaceFXAnimSetRef;
var()						FaceFXAnimSet						FaceFXAnimSetRef;
/** Name of the FaceFX Group the animation is in */
var()						string									FaceFXGroupName;
/** Name of the FaceFX Animation */
var()						string									FaceFXAnimName;

/** Maximum number of times this cue can be played concurrently. */
var()						int										MaxConcurrentPlayCount;
/** Number of times this cue is currently being played. */
var	const transient duplicatetransient int							CurrentPlayCount;

cpptext
{
	// UObject interface.
	virtual void Serialize( FArchive& Ar );

	/**
	 * Strip Editor-only data when cooking for console platforms.
	 */
	virtual void StripData( UE3::EPlatformType TargetPlatform );

	/**
	 * Returns a description of this object that can be used as extra information in list views.
	 */
	virtual FString GetDesc( void );

	/** 
	 * Returns detailed info to populate listview columns
	 */
	virtual FString GetDetailedDescription( INT InIndex );

	/**
	 * @return		Sum of the size of waves referenced by this cue.
	 */
	virtual INT GetResourceSize( void );

	/**
	 *	@param		Platform		EPlatformType indicating the platform of interest...
	 *
	 *	@return		Sum of the size of waves referenced by this cue for the given platform.
	 */
	virtual INT GetResourceSize( UE3::EPlatformType Platform );

	/**
	 * Called when a property value from a member struct or array has been changed in the editor.
	 */
	virtual void PostEditChange( UProperty* PropertyThatChanged );
	
	/** 
	 * Remap sound locations through portals
	 */
	FVector RemapLocationThroughPortals( const FVector& SourceLocation, const FVector& ListenerLocation );
	
	/** 
	 * Calculate the maximum audible distance accounting for every node
	 */
	void CalculateMaxAudibleDistance( void );
	
	/**
	 * Checks to see if a location is audible
	 */
	UBOOL IsAudible( const FVector& SourceLocation, const FVector& ListenerLocation, AActor* SourceActor, INT& bIsOccluded, UBOOL bCheckOcclusion );
	
	/** 
	 * Does a simple range check to all listeners to test hearability
	 */
	UBOOL IsAudibleSimple( FVector* Location );

	/** 
	 * Draw debug information relating to a sound cue
	 */
	void DrawCue( FCanvas* Canvas, TArray<USoundNode *>& SelectedNodes );
	
	/**
	 * Recursively finds all attenuation nodes in a sound node tree
	 */
	void RecursiveFindAttenuation( USoundNode* Node, TArray<class USoundNodeAttenuation*> &OutAttens );

	/**
	 * Recursively finds all waves in a sound node tree.
	 */
	void RecursiveFindWaves( USoundNode* Node, TArray<USoundNodeWave *> &OutWaves );
	
	/** 
	 * Makes sure ogg vorbis data is available for all sound nodes in this cue by converting on demand
	 */
	UBOOL ValidateData( void );
}

native function float GetCueDuration();

defaultproperties
{
	VolumeMultiplier=0.75
	PitchMultiplier=1
	MaxConcurrentPlayCount=16
}