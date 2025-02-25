class InterpTrackSound extends InterpTrackVectorBase
	native(Interpolation);

/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * 
 *	A track that plays sounds on the groups Actor.
 */

cpptext
{
	virtual void PostLoad();

	// InterpTrack interface
	virtual INT GetNumKeyframes();
	virtual void GetTimeRange(FLOAT& StartTime, FLOAT& EndTime);
	virtual FLOAT GetKeyframeTime(INT KeyIndex);
	virtual INT AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode);
	virtual INT SetKeyframeTime(INT KeyIndex, FLOAT NewKeyTime, UBOOL bUpdateOrder=true);
	virtual void RemoveKeyframe(INT KeyIndex);
	virtual INT DuplicateKeyframe(INT KeyIndex, FLOAT NewKeyTime);
	virtual UBOOL GetClosestSnapPosition(FLOAT InPosition, TArray<INT> &IgnoreKeys, FLOAT& OutPosition);

	virtual void PreviewUpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst);
	virtual void UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump);
	virtual void PreviewStopPlayback(class UInterpTrackInst* TrInst);

	/** Get the name of the class used to help out when adding tracks, keys, etc. in UnrealEd.
	* @return	String name of the helper class.*/
	virtual const FString	GetEdHelperClassName() const;

	virtual class UMaterial* GetTrackIcon();
	virtual void DrawTrack( FCanvas* Canvas, const FInterpTrackDrawParams& Params );
	
	/** Whether or not this track is allowed to be used on static actors. */
	virtual UBOOL AllowStaticActors() { return TRUE; }

	// InterpTrackSound interface
	/**
	 * Returns the key at the specified position in the track.
	 */
	struct FSoundTrackKey& GetSoundTrackKeyAtPosition(FLOAT InPosition);

	virtual void SetTrackToSensibleDefault();
}

/** Information for one sound in the track. */
struct native SoundTrackKey
{
	var		float		Time;
	var		float		Volume;
	var		float		Pitch;
	var()	SoundCue	Sound;

	structdefaultproperties
	{
		Volume=1.f
		Pitch=1.f
	}
};	

/** Array of sounds to play at specific times. */
var	array<SoundTrackKey>	Sounds;

/** If true, sounds on this track will not be forced to finish when the matinee sequence finishes. */
var()	bool			bContinueSoundOnMatineeEnd;

/** If TRUE, don't show subtitles for sounds played by this track. */
var()	bool			bSuppressSubtitles;

// BM1
var() bool bStopSoundOnMatineeSkip;

defaultproperties
{
	TrackInstClass=class'Engine.InterpTrackInstSound'
	TrackTitle="Sound"
}
