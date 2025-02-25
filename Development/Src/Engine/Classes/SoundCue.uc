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

struct native AnimTriggerFaceFXTag
{
    var() editconst name TagName;
    var() editconst array<editconst string> TagParams;
    var() editconst AnimSet TagAnimSet;
    var() editconst float AtTime;
};

// BM1
var 						string 									FMODCategory;

/** Sound group this sound cue belongs to */
var							SoundNode								FirstNode;
var		const native		Map{USoundNode*,FSoundNodeEditorData}	EditorData;
var()						float									VolumeMultiplier;
var()						float									PitchMultiplier;
var							float									Duration;

// BM1
var() 						bool 									IgnorePitch;
var 						bool 									FMODGeneratedCue;
var 						bool 									FMODValidCue;
var() 						bool 									FMODIgnoreOcclusion;
var 						bool 									FMODRetryFail;
var 						bool 									FMODLooping;
var 						bool 									bSlowOcclusion;
var 						RFMODSound 								FMODSound;
var 						float 									OcclusionValue;
var 						float 									FMODOriginalVolume;
var 						float 									FMODOriginalWetVolume;
var 						float 									FMODOriginalDryVolume;
var 						float 									FMODOriginalMinDistance;
var 						float 									FMODOriginalMaxDistance;
var 						int 									FMODStatusFlag;

/** Reference to FaceFX AnimSet package the animation is in */
var()						FaceFXAnimSet							FaceFXAnimSetRef;
/** Name of the FaceFX Group the animation is in */
var()						string									FaceFXGroupName;
/** Name of the FaceFX Animation */
var()						string									FaceFXAnimName;

// BM1
var() 						array<AnimTriggerFaceFXTag> 			AnimTriggers;
var		const transient 	int 									FMODResourceID;

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
	VolumeMultiplier=1
	PitchMultiplier=1
}