/**
 * One animation sequence of keyframes. Contains a number of tracks of data.
 * The Outer of AnimSequence is expected to be its AnimSet.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AnimSequence extends Object
	native(Anim)
	hidecategories(Object);

/*
 * Triggers an animation notify.  Each AnimNotifyEvent contains an AnimNotify object
 * which has its Notify method called and passed to the animation.
 */
struct native AnimNotifyEvent
{
	var()	float						Time;
	var()	instanced AnimNotify		Notify;
	var()	name						Comment;
};

/**
 * Raw keyframe data for one track.  Each array will contain either NumKey elements or 1 element.
 * One element is used as a simple compression scheme where if all keys are the same, they'll be
 * reduced to 1 key that is constant over the entire sequence.
 */
struct native RawAnimSequenceTrack
{
	/** Position keys. */
	var array<vector>	PosKeys;

	/** Rotation keys. */
	var array<quat>		RotKeys;

	/** Key times, in seconds. */
	var array<float>	KeyTimes;
};

/** Name of the animation sequence. Used in AnimNodeSequence. */
var		name									SequenceName;

/** Animation notifies, sorted by time (earliest notification first). */
var()	editinline array<AnimNotifyEvent>		Notifies;

/** Length (in seconds) of this AnimSequence if played back with a speed of 1.0. */
var		float									SequenceLength;

/** Number of raw frames in this sequence (not used by engine - just for informational purposes). */
var		int										NumFrames;

// BM1
var(Info) float FramesPerSecond;

/** Number for tweaking playback rate of this animation globally. */
var()	float									RateScale;

/** 
 * if TRUE, disable interpolation between last and first frame when looping.
 */
var()	bool									bNoLoopingInterpolation;

// BM1
var() bool bUseSimpleForwardYaw;
var() bool bUseSimpleFloorHeight;
var() bool bUseSimpleRootMotionXY;
var() bool DisableProportionalMotionDuringBlendOut;
var() bool AllowCheekyBlendIn;
var() bool AllowCheekyBlendOut;
var(Info) bool MeetingPointBeginRotationEnabled;
var(Info) bool MeetingPointEndRotationEnabled;
var(Info) bool MeetingPointBeginTranslationEnabled;
var(Info) bool MeetingPointEndTranslationEnabled;
var(Info) bool WeaponSwitchPointEnabled;

/**
 * Raw uncompressed keyframe data.
 */
var		const  array<RawAnimSequenceTrack>		RawAnimData;

/**
 * Keyframe position data for one track.  Pos(i) occurs at Time(i).  Pos.Num() always equals Time.Num().
 */
struct native TranslationTrack
{
	var array<vector>	PosKeys;
	var array<float>	Times;
};

/**
 * Keyframe rotation data for one track.  Rot(i) occurs at Time(i).  Rot.Num() always equals Time.Num().
 */
struct native RotationTrack
{
	var array<quat>		RotKeys;
	var array<float>	Times;
};

/**
 * Translation data post keyframe reduction.  TranslationData.Num() is zero if keyframe reduction
 * has not yet been applied.
 */
var transient const array<TranslationTrack>		TranslationData;

/**
 * Rotation data post keyframe reduction.  RotationData.Num() is zero if keyframe reduction
 * has not yet been applied.
 */
var transient const array<RotationTrack>		RotationData;

/**
 * The compression scheme that was most recently used to compress this animation.
 * May be NULL.
 */
var() editinline editconst editoronly AnimationCompressionAlgorithm	CompressionScheme;

/**
 * Indicates animation data compression format.
 */
enum AnimationCompressionFormat
{
	ACF_None,
	ACF_Float96NoW,
	ACF_Fixed48NoW,
	ACF_IntervalFixed32NoW,
	ACF_Fixed32NoW,
	ACF_Float32NoW,
	ACF_Fixed48Max,
};

/** The compression format that was used to compress translation tracks. */
var const AnimationCompressionFormat		TranslationCompressionFormat;

/** The compression format that was used to compress rotation tracks. */
var const AnimationCompressionFormat		RotationCompressionFormat;

struct native CompressedTrack
{
	var array<byte>		ByteStream;
	var array<float>	Times;
	var float			Mins[3];
	var float			Ranges[3]; 
};

/**
 * An array of 4*NumTrack ints, arranged as follows:
 *   [0] Trans0.Offset
 *   [1] Trans0.NumKeys
 *   [2] Rot0.Offset
 *   [3] Rot0.NumKeys
 *   [4] Trans1.Offset
 *   . . .
 */
var			array<int>		CompressedTrackOffsets;

/**
 * ByteStream for compressed animation data.
 * All keys are currently stored at evenly-spaced intervals (ie no explicit key times).
 *
 * For a translation track of n keys, data is packed as n uncompressed float[3]:
 *
 * For a rotation track of n>1 keys, the first 24 bytes are reserved for compression info
 * (eg Fixed32 stores float Mins[3]; float Ranges[3]), followed by n elements of the compressed type.
 * For a rotation track of n=1 keys, the single key is packed as an FQuatFloat96NoW.
 */
var native	array<byte>		CompressedByteStream;

// BM1
enum EForwardYawDirection
{
    FYD_Clockwise,
    FYD_AntiClockwise
};

// BM1
struct native AnimReferenceOptions
{
    var() bool AutomaticFloorHeight;
    var() bool EaseInProportionalMotion;
    var() float FloorHeight;
    var() EForwardYawDirection ForwardYawDirection;
    var() float ForwardYaw;
};

// BM1
struct native AnimReferencePeriods
{
    var() AnimReferenceOptions Start;
    var() AnimReferenceOptions End;
    var() bool EnforceMinimumFloorHeight;
    var() float MinimumFloorHeight;
};

// BM1
var(Info) editoronly string MaxFilePath;
var() Vector ReferencePoint;
var() float ReferencePointYaw;
var() AnimReferencePeriods ReferenceOptions;
var() float ProportionalMotionDistanceCap;
var() float BlendInDuration;
var() float BlendOutDuration;

// BM1
var(Info) float BlendInPoint;
var(Info) float BlendOutPoint;
var(Info) float ClippedStartPoint;
var(Info) float ClippedEndPoint;
var(Info) float CanCancelBeforeHerePoint;
var(Info) float CanCancelAfterHerePoint;
var(Info) float CanCorrectAfterHerePoint;
var(Info) float ForwardYawInPoint;
var(Info) float ForwardYawOutPoint;
var(Info) float ForwardYawStartOffset;
var(Info) float ForwardYawEndOffset;
var(Info) float FloorHeightInPoint;
var(Info) float FloorHeightOutPoint;
var(Info) float FloorHeightStartOffset;
var(Info) float FloorHeightEndOffset;
var(Info) float CollisionOptionsOutPoint;
var(Info) float ClipRootMotionInPoint;
var(Info) float ClipRootMotionOutPoint;
var(Info) Vector LinearCentre;
var(Info) Vector LinearSpan;
var(Info) float MeetingPointBeginRotation;
var(Info) float MeetingPointEndRotation;
var(Info) float MeetingPointBeginTranslation;
var(Info) float MeetingPointEndTranslation;
var(Info) float WeaponSwitchPoint;

/**
 * Indicates animation data compression format.
 */
enum AnimationKeyFormat
{
    AKF_ConstantKeyLerp,
	AKF_VariableKeyLerp,
};

var const AnimationKeyFormat		KeyEncodingFormat;

/**
 * The runtime interface to decode and byte swap the compressed animation
 * May be NULL. Set at runtime - does not exist in editor
 */
var private transient native pointer	TranslationCodec;
var private transient native pointer	RotationCodec;

// Additive Animation Support
/** TRUE if this is an Additive Animation */
var	const	bool			bIsAdditive;
/** Reference pose for additive animation. */
var	const	Array<BoneAtom>	AdditiveRefPose;
/** Reference animation name */
var	const	Name			AdditiveRefName;

// Versioning Support
/** The version of the global encoding package used at the time of import */
var	const	int				EncodingPkgVersion;

/** 
 * Do not attempt to override compression scheme when running CompressAnimations commandlet. 
 * Some high frequency animations are too sensitive and shouldn't be changed.
 */
var() const bool			bDoNotOverrideCompression;

// BM1
enum EAnimPhysics
{
    APHYS_Walking,
    APHYS_Flying,
    APHYS_Floating,
    APHYS_Falling
};

// BM1
enum ERootMotionRotationOption
{
    RMRO_On,
    RMRO_NoExtraction,
    RMRO_Off
};

// BM1
enum ERootMotionTranslationOption
{
    RMTO_On,
    RMTO_NoExtraction,
    RMTO_Off
};

// BM1
struct AnimCollisionOptions
{
    var() EAnimPhysics Physics;
    var() bool BlockActors;
    var() bool CollideWorld;
    var() bool DisableLegIK;
    var() ERootMotionRotationOption RootMotionRotationOption;
    var() ERootMotionTranslationOption RootMotionTranslationOption;

    structdefaultproperties
    {
        Physics=APHYS_Walking
        BlockActors=true
        CollideWorld=true
        DisableLegIK=false
        RootMotionRotationOption=RMRO_On
        RootMotionTranslationOption=RMTO_On
    }
};

cpptext
{
	// UObject interface

	virtual void Serialize(FArchive& Ar);
	virtual void PreSave();
	virtual void PostLoad();
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void BeginDestroy();

	/**
	 * Used by various commandlets to purge Editor only data from the object.
	 * 
	 * @param TargetPlatform Platform the object will be saved for (ie PC vs console cooking, etc)
	 */
	virtual void StripData(UE3::EPlatformType TargetPlatform);

	// AnimSequence interface

	/**
	 * Reconstructs a bone atom from key-reduced tracks.
	 */
	static void ReconstructBoneAtom(FBoneAtom& OutAtom,
									const FTranslationTrack& TranslationTrack,
									const FRotationTrack& RotationTrack,
									FLOAT SequenceLength,
									FLOAT Time,
									UBOOL bLooping);

	/**
	 * Reconstructs a bone atom from compressed tracks.
	 */
	static void ReconstructBoneAtom(FBoneAtom& OutAtom,
									const FCompressedTrack& TranslationTrack,
									const FCompressedTrack& RotationTrack,
									AnimationCompressionFormat TranslationCompressionFormat,
									AnimationCompressionFormat RotationCompressionFormat,
									FLOAT SequenceLength,
									FLOAT Time,
									UBOOL bLooping);

	/**
	 * Reconstructs a bone atom from compressed tracks.
	 */
	static void ReconstructBoneAtom(FBoneAtom& OutAtom,
									const BYTE* TransStream,
									INT NumTransKeys,
									const BYTE* RotStream,
									INT NumRotKeys,
									AnimationCompressionFormat TranslationCompressionFormat,
									AnimationCompressionFormat RotationCompressionFormat,
									FLOAT SequenceLength,
									FLOAT Time,
									UBOOL bLooping);

	/**
	 * Decompresses a translation key from the specified compressed translation track.
	 */
	static void ReconstructTranslation(class FVector& Out, const BYTE* Stream, INT KeyIndex, AnimationCompressionFormat TranslationCompressionFormat);

	/**
	 * Decompresses a rotation key from the specified compressed rotation track.
	 */
	static void ReconstructRotation(class FQuat& Out, const BYTE* Stream, INT KeyIndex, AnimationCompressionFormat RotationCompressionFormat, const FLOAT *Mins, const FLOAT *Ranges);

	/**
	 * Decompresses a translation key from the specified compressed translation track.
	 */
	static void ReconstructTranslation(class FVector& Out, const BYTE* Stream, INT KeyIndex);

	/**
	 * Decompresses a rotation key from the specified compressed rotation track.
	 */
	static void ReconstructRotation(class FQuat& Out, const BYTE* Stream, INT KeyIndex, UBOOL bTrackHasCompressionInfo, AnimationCompressionFormat RotationCompressionFormat);

	/**
	 * Populates the key reduced arrays from raw animation data.
	 */
	static void SeparateRawDataToTracks(const TArray<FRawAnimSequenceTrack>& RawAnimData,
										FLOAT SequenceLength,
										TArray<FTranslationTrack>& OutTranslationData,
										TArray<FRotationTrack>& OutRotationData);

	/**
	 * Interpolate keyframes in this sequence to find the bone transform (relative to parent).
	 * 
	 * @param	OutAtom			[out] Output bone transform.
	 * @param	TrackIndex		Index of track to interpolate.
	 * @param	Time			Time on track to interpolate to.
	 * @param	bLooping		TRUE if the animation is looping.
	 * @param	bUseRawData		If TRUE, use raw animation data instead of compressed data.
	 */
	void GetBoneAtom(FBoneAtom& OutAtom, INT TrackIndex, FLOAT Time, UBOOL bLooping, UBOOL bUseRawData) const;

	/** Sort the Notifies array by time, earliest first. */
	void SortNotifies();

	/**
	 * @return		A reference to the AnimSet this sequence belongs to.
	 */
	UAnimSet* GetAnimSet() const;

	/**
	 * Returns the size of the object/ resource for display to artists/ LDs in the Editor.
	 *
	 * @return size of resource as to be displayed to artists/ LDs in the Editor.
	 */
	virtual INT GetResourceSize();

	/**
	 * @return		The approximate size of raw animation data.
	 */
	INT GetApproxRawSize() const;

	/**
	 * @return		The approximate size of key-reduced animation data.
	 */
	INT GetApproxReducedSize() const;

	/**
	 * @return		The approximate size of compressed animation data.
	 */
	INT GetApproxCompressedSize() const;
	
	/**
	 * Crops the raw anim data either from Start to CurrentTime or CurrentTime to End depending on 
	 * value of bFromStart.  Can't be called against cooked data.
	 *
	 * @param	CurrentTime		marker for cropping (either beginning or end)
	 * @param	bFromStart		whether marker is begin or end marker
	 * @return					TRUE if the operation was successful.
	 */
	UBOOL CropRawAnimData( FLOAT CurrentTime, UBOOL bFromStart );

	/** Clears any data in the AnimSequence, so it can be recycled when importing a new animation with same name over it. */
	void RecycleAnimSequence();
}

defaultproperties
{
	RateScale=1.0
	AllowCheekyBlendIn=true
    AllowCheekyBlendOut=true
	BlendInDuration=0.2
    BlendOutDuration=0.2
    BlendOutPoint=1.0
    ClippedEndPoint=1.0
    CanCancelBeforeHerePoint=1.0
    CanCancelAfterHerePoint=1.0
    CanCorrectAfterHerePoint=1.0
    ForwardYawOutPoint=1.0
    FloorHeightOutPoint=1.0
    CollisionOptionsOutPoint=1.0
    ClipRootMotionOutPoint=1.0
}
