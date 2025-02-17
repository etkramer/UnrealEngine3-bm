/*=============================================================================
	SoundCue.h: Native SoundCue calls
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

protected:
	/**
	 * Strip Editor-only data when cooking for console platforms.
	 */
	virtual void StripData( UE3::EPlatformType TargetPlatform );

	/** 
	 * Remap sound locations through portals
	 */
	FVector RemapLocationThroughPortals( const FVector& SourceLocation, const FVector& ListenerLocation );

	/** 
	 * Calculate the maximum audible distance accounting for every node
	 */
	void CalculateMaxAudibleDistance( void );

public:
	// UObject interface.
	virtual void Serialize( FArchive& Ar );

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
	 * Returns a description of this object that can be used as extra information in list views.
	 */
	virtual FString GetDesc( void );

	/** 
	 * Returns detailed info to populate listview columns
	 */
	virtual FString GetDetailedDescription( INT InIndex );

	/** 
	 * Does a simple range check to all listeners to test hearability
	 */
	UBOOL IsAudibleSimple( FVector* Location );

	/**
	 * Checks to see if a location is audible
	 */
	UBOOL IsAudible( const FVector& SourceLocation, const FVector& ListenerLocation, AActor* SourceActor, INT& bIsOccluded, UBOOL bCheckOcclusion );

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

	/** 
	 * Draw debug information relating to a sound cue
	 */
	void DrawCue( FCanvas* Canvas, TArray<USoundNode *>& SelectedNodes );