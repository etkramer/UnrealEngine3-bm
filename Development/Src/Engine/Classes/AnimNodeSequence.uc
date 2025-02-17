/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AnimNodeSequence extends AnimNode
	native(Anim)
	hidecategories(Object);

/** This name will be looked for in all AnimSet's specified in the AnimSets array in the SkeletalMeshComponent. */
var()	const name		AnimSeqName;

/** Speed at which the animation will be played back. Multiplied by the RateScale in the AnimSequence. Default is 1.0 */
var()	float			Rate;

/** Whether this animation is currently playing ie. if the CurrentTime will be advanced when Tick is called. */
var()	bool			bPlaying;

/** If animation is looping. If false, animation will stop when it reaches end, otherwise will continue from beginning. */
var()	bool			bLooping;

/** Should this node call the OnAnimEnd event on its parent Actor when it reaches the end and stops. */
var()	bool			bCauseActorAnimEnd;

/** Should this node call the OnAnimPlay event on its parent Actor when PlayAnim is called on it. */
var()	bool			bCauseActorAnimPlay;

/** Always return a zero rotation (unit quaternion) for the root bone of this animation. */
var()	bool			bZeroRootRotation;
/** Always return root bone translation at the origin. */
var()	bool			bZeroRootTranslation;
/** if TRUE, don't display a warning when animation is not found. */
var()	bool			bDisableWarningWhenAnimNotFound;

/** Current position (in seconds) */
var()	const float				CurrentTime;
// Keep track of where animation was at before being ticked
var		const transient float	PreviousTime;

/** Pointer to actual AnimSequence. Found from SkeletalMeshComponent using AnimSeqName when you call SetAnim. */
var		transient const AnimSequence	AnimSeq;

/** Bone -> Track mapping info for this player node. Index into the LinkupCache array in the AnimSet. Found from AnimSet when you call SetAnim. */
var		transient const int				AnimLinkupIndex;

/** 
 * Total weight that this node must be at in the final blend for notifies to be executed.
 * This is ignored when the node is part of a group.
 */
var()				float	NotifyWeightThreshold;
/** Whether any notifies in the animation sequence should be executed for this node. */
var()				bool	bNoNotifies;
/** Forces the skeletal mesh into the ref pose by setting bForceRespose on the skelmesh comp when not playing. (Optimization) */
var()				bool	bForceRefposeWhenNotPlaying;
/**
 *	Flag that indicates if Notifies are currently being executed.
 *	Allows you to avoid doing dangerous things to this Node while this is going on.
 */
var					bool	bIsIssuingNotifies;

/** name of group this node belongs to */
var(Group) const	Name	SynchGroupName;
/** If TRUE, this node can never be a synchronization master node, always slave. */
var(Group)			bool	bForceAlwaysSlave;

/** 
 * TRUE by default. This node can be synchronized with others, when part of a SynchGroup. 
 * Set to FALSE if node shouldn't be synchronized, but still part of notification group.
 */
var(Group) const	bool	bSynchronize;
/** Relative position offset */
var(Group)			float	SynchPosOffset;

/** Display time line slider */
var(Display)		bool	bShowTimeLineSlider;

/** For debugging. Track graphic used for showing animation position. */
var	texture2D	DebugTrack;

/** For debugging. Small icon to show current position in animation. */
var texture2D	DebugCarat;

/** Reference to the CameraAnim to play in conjunction with this animation. */
var(Camera)			CameraAnim	CameraAnim;
/** Ref to the CameraAnimInst that is currently playing. */
var transient CameraAnimInst	ActiveCameraAnimInstance;
/** True to loop the CameraAnim, false for a one-off. */
var(Camera)			bool		bLoopCameraAnim;
/** True to randomize the CameraAnims start position, so it doesn't look the same every time.  Ignored if bLoopCameraAnim is false. */
var(Camera)			bool		bRandomizeCameraAnimLoopStartTime;
/** "Intensity" multiplier applied to the camera anim. */
var(Camera)			float		CameraAnimScale;
/** How fast to playback the camera anim. */
var(Camera)			float		CameraAnimPlayRate;

/**
 *	This will actually call MoveActor to move the Actor owning this SkeletalMeshComponent.
 *	You can specify the behaviour for each axis (mesh space).
 *	Doing this for multiple skeletal meshes on the same Actor does not make much sense!
 */
enum ERootBoneAxis
{
	/** the default behaviour, leave root translation from animation and do no affect owning Actor movement. */
	RBA_Default,
	/** discard any root bone movement, locking it to the first frame's location. */
	RBA_Discard,
	/** discard root movement on animation, and forward its velocity to the owning actor. */
	RBA_Translate,
};

var() ERootBoneAxis		RootBoneOption[3]; // [X, Y, Z] axes

/**
 * Root Motion Rotation.
 */
enum ERootRotationOption
{
	/** Default, leaves root rotation in the animation. Does not affect actor. */
	RRO_Default,
	/** Discards root rotation from the animation, locks to first frame rotation of animation. Does not affect actor's rotation. */
	RRO_Discard,
	/** Discard root rotation from animation, and forwards it to the actor. (to be used by it or not). */
	RRO_Extract,
};

var() ERootRotationOption	RootRotationOption[3];	// Roll (X), Pitch (Y), Yaw (Z) axes.


/** 
 * EDITOR ONLY
 * Add Ref Pose to Additive Animation, so it can be viewed fully into the AnimSetViewer.
 */
var const	bool	bEditorOnlyAddRefPoseToAdditiveAnimation;

cpptext
{
protected:
	// Internal
	/** Returns the camera associated with the skelmesh's owner, if any. */
	AAnimatedCamera* GetPlayerCamera() const;

public:
	// UObject interface
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void BeginDestroy();

	// AnimNode interface
	virtual void InitAnim( USkeletalMeshComponent* meshComp, UAnimNodeBlendBase* Parent );
    virtual UBOOL GetCachedResults(FBoneAtomArray& OutAtoms, FBoneAtom& OutRootMotionDelta, INT& bOutHasRootMotion);
    virtual UBOOL ShouldSaveCachedResults();
	UBOOL ShouldSkipWhenMeshNotRendered();
	void ConditionalClearCachedData();

	/** AnimSets have been updated, update all animations */
	virtual void AnimSetsUpdated();

	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );	 // Progress the animation state, issue AnimEnd notifies.
	virtual void GetBoneAtoms(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);

	/**
	 * Draws this node in the AnimTreeEditor.
	 *
	 * @param	RI				The rendering interface to use.
	 * @param	bSelected		TRUE if this node is selected.
	 * @param	bShowWeight		If TRUE, show the global percentage weight of this node, if applicable.
	 * @param	bCurves			If TRUE, render links as splines; if FALSE, render links as straight lines.
	 */
	virtual void DrawAnimNode(FCanvas* Canvas, UBOOL bSelected, UBOOL bShowWeight, UBOOL bCurves);
	virtual FString GetNodeTitle();

	// AnimNodeSequence interface
	void GetAnimationPose(UAnimSequence* InAnimSeq, INT& InAnimLinkupIndex, FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);
	// Extract root motion fro animation.
	void ExtractRootMotion(UAnimSequence* InAnimSeq, const INT &TrackIndex, FBoneAtom& RootBoneAtom, FBoneAtom& RootBoneAtomDeltaMotion, INT& bHasRootMotion);

	/** Advance animation time. Take care of issuing notifies, looping and so on */
	void AdvanceBy(FLOAT MoveDelta, FLOAT DeltaSeconds, UBOOL bFireNotifies);

	/** Issue any notifies tha are passed when moving from the current position to DeltaSeconds in the future. Called from TickAnim. */
	void IssueNotifies(FLOAT DeltaSeconds);

	/**
	 * notification that current animation finished playing.
	 * @param	PlayedTime	Time in seconds of animation played. (play rate independant).
	 * @param	ExcessTime	Time in seconds beyond end of animation. (play rate independant).
	 */
	virtual void OnAnimEnd(FLOAT PlayedTime, FLOAT ExcessTime);

	// AnimTree editor interface
	virtual INT GetNumSliders() const { return bShowTimeLineSlider ? 1 : 0; }
	virtual FLOAT GetSliderPosition(INT SliderIndex, INT ValueIndex);
	virtual void HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue);
	virtual FString GetSliderDrawValue(INT SliderIndex);

	/** Starts playing any camera anim we want to play in conjunction with this anim. */
	void StartCameraAnim();
	/** Stops playing any active camera anim playing in conjunction with this anim. */
	void StopCameraAnim();
}

/** Change the animation this node is playing to the new name. Will be looked up in owning SkeletaMeshComponent's AnimSets array. */
native function SetAnim( name Sequence );

/** Start the current animation playing with the supplied parameters. */
native function PlayAnim(bool bLoop = false, float InRate = 1.0f, float StartTime = 0.0f);

/** Stop the current animation playing. CurrentTime will stay where it was. */
native function StopAnim();

/** Force the animation to a particular time. NewTime is in seconds. */
native function SetPosition(float NewTime, bool bFireNotifies);

/** Get normalized position, from 0.f to 1.f. */
native function float GetNormalizedPosition();

/** 
 * Finds out normalized position of a synchronized node given a relative position of a group. 
 * Takes into account node's relative SynchPosOffset.
 */
native function float FindNormalizedPositionFromGroupRelativePosition(FLOAT GroupRelativePosition);

/** Returns the global play rate of this animation. Taking into account all Rate Scales */
native function float GetGlobalPlayRate();

/** Returns the duration (in seconds) of the current animation at the current play rate. Returns 0.0 if no animation. */
native function float GetAnimPlaybackLength();

/**
 * Returns in seconds the time left until the animation is done playing.
 * This is assuming the play rate is not going to change.
 */
native function float GetTimeLeft();


defaultproperties
{
	Rate=1.0
	NotifyWeightThreshold=0.0
	bSynchronize=TRUE
	DebugTrack=Texture2D'EditorResources.AnimPlayerTrack'
	DebugCarat=Texture2D'EditorResources.AnimPlayerCarat'

	RootBoneOption[0] = RBA_Default
	RootBoneOption[1] = RBA_Default
	RootBoneOption[2] = RBA_Default

	RootRotationOption[0]=RRO_Default
	RootRotationOption[1]=RRO_Default
	RootRotationOption[2]=RRO_Default

	CameraAnimPlayRate=1.f
	CameraAnimScale=1.f
}
