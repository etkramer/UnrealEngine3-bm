/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class SeqAct_SpectatorCameraPath extends SeqAct_Interp
	native(Sequence)
	hidecategories(SeqAct_Interp,SeqAct_Latent);

/** Time, in seconds, it takes for the camera to animate between any 2 stops on the path. */
var() const protected float TravelTimeBetweenStops;

/** Set to TRUE if you want "forward" on the path to be "reverse" in Matinee time (i.e. it was authored backwards). */
var() const protected bool	bSwapForwardAndReverse;

var() const bool			bAutomaticFraming;

/** Internal cached data. */
var private transient bool					bCachedInterpData;
var private transient CameraActor			CachedCameraActor;
var private transient InterpTrackMove		CachedMoveTrack;
var private transient InterpTrackInstMove	CachedMoveTrackInst;
var private transient InterpTrackEvent		CachedEventTrack;
var private transient InterpTrackInstEvent	CachedEventTrackInst;

var private transient int					LastStopIdx;
var private transient int					DestinationStopIdx;
var private transient float					InterpTimeToNextStop;

/** Forward one stop. */
native final function MoveToNextStop();
/** Backward one stop. */
native final function MoveToPrevStop();

/** Returns ref to the CameraActor that will be moved around by this path, or None if none is found (e.g. the Matinee isn't set up properly). */
native final function CameraActor GetAssociatedCameraActor();

/** Re-init the interp data to the beginning pos. */
native final function ResetToStartingPosition();

/** Updates camera position along the matinee curve. */
native final function UpdateCameraPosition(float DeltaTime);


cpptext
{
protected:
	void CacheInterpData();
	UBOOL IsValidCameraPath();

public:
	virtual void Activated();
	virtual UBOOL UpdateOp(FLOAT deltaTime) { return USequenceAction::UpdateOp(deltaTime); }
	virtual void DeActivated() { USequenceAction::DeActivated(); }

	virtual void UpdateConnectorsFromData();

	virtual void UpdateInterp(FLOAT NewPosition, UBOOL bPreview, UBOOL bJump);
	
	/**
	 * Activates the output for the named event.
	 */
	virtual void NotifyEventTriggered(class UInterpTrackEvent const* EventTrack, INT EventIdx);

	FLOAT GetInterpolationPercentage() const;
}

defaultproperties
{
	ObjName="Spectator Camera Path"
	ObjCategory="Gear"

	InputLinks.Empty
	InputLinks(0)=(LinkDesc="Register")

	OutputLinks.Empty
	OutputLinks(0)=(LinkDesc="Out")

	ReplicatedActorClass=None

	bLooping=TRUE
	bLatentExecution=FALSE

	TravelTimeBetweenStops=0.5f
	bAutomaticFraming=TRUE
}
