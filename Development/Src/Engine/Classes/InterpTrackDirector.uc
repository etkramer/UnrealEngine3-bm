class InterpTrackDirector extends InterpTrack
	native(Interpolation);

/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * A track type used for binding the view of a Player (attached to this tracks group) to the actor of a different group.
 *
 */

cpptext
{
	// InterpTrack interface
	virtual INT GetNumKeyframes();
	virtual void GetTimeRange(FLOAT& StartTime, FLOAT& EndTime);
	virtual FLOAT GetKeyframeTime(INT KeyIndex);
	virtual INT AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode);
	virtual INT SetKeyframeTime(INT KeyIndex, FLOAT NewKeyTime, UBOOL bUpdateOrder=true);
	virtual void RemoveKeyframe(INT KeyIndex);
	virtual INT DuplicateKeyframe(INT KeyIndex, FLOAT NewKeyTime);
	virtual UBOOL GetClosestSnapPosition(FLOAT InPosition, TArray<INT> &IgnoreKeys, FLOAT& OutPosition);

	virtual void UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump);

	/** Get the name of the class used to help out when adding tracks, keys, etc. in UnrealEd.
	* @return	String name of the helper class.*/
	virtual const FString	GetEdHelperClassName() const;

	virtual class UMaterial* GetTrackIcon();
	virtual void DrawTrack( FCanvas* Canvas, const FInterpTrackDrawParams& Params );

	// InterpTrackDirector interface
	FName GetViewedGroupName(FLOAT CurrentTime, FLOAT& CutTime, FLOAT& CutTransitionTime);
}

/** Information for one cut in this track. */
struct native DirectorTrackCut
{
	/** Time to perform the cut. */
	var		float	Time;

	/** Time taken to move view to new camera. */
	var		float	TransitionTime;

	/** GroupName of InterpGroup to cut viewpoint to. */
	var()	name	TargetCamGroup;
};	

/** Array of cuts between cameras. */
var	array<DirectorTrackCut>	CutTrack;

/** True to allow clients to simulate their own camera cuts.  Can help with latency-induced timing issues. */
var() bool bSimulateCameraCutsOnClients;

defaultproperties
{
	bOnePerGroup=true
	bDirGroupOnly=true
	TrackInstClass=class'Engine.InterpTrackInstDirector'
	TrackTitle="Director"
	bSimulateCameraCutsOnClients=TRUE
}
