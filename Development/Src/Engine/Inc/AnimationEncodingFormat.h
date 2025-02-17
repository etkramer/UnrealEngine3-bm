/*=============================================================================
	AnimationEncodingFormat.h: Skeletal mesh animation compression.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#ifndef __ANIMATIONENCODINGFORMAT_H__
#define __ANIMATIONENCODINGFORMAT_H__

// switch to enable/disable the new animation codec system
#define USE_ANIMATION_CODEC_INTERFACE 1

// switch to enable/disable the new variable-key codec
#define VARIABLE_KEY_CODEC_ENABLED 1

// switches to toggle subsets of the new animation codec system
#define USE_ANIMATION_CODEC_BATCH_SOLVER (USE_ANIMATION_CODEC_INTERFACE && 1)

// all past encoding package version numbers should be listed here
#define ANIMATION_ENCODING_PACKAGE_ORIGINAL 0

// the current animation encoding package version
#define CURRENT_ANIMATION_ENCODING_PACKAGE_VERSION ANIMATION_ENCODING_PACKAGE_ORIGINAL

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interfaces For Working With Encoded Animations
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if USE_ANIMATION_CODEC_INTERFACE

/**
*	Structure to hold an Atom and Track index mapping for a requested bone. 
*	Used in the bulk-animation solving process
*/
struct BoneTrackPair
{
	INT AtomIndex;
	INT TrackIndex;

	BoneTrackPair(){}
	BoneTrackPair(INT Atom, INT Track):AtomIndex(Atom),TrackIndex(Track){}
};

/**
*	Fixed-size array of BoneTrackPair elements.
*	Used in the bulk-animation solving process.
*/
#define MAX_BONES 256 // DesiredBones is passed to the decompression routines as a TArray<BYTE>, so we know this max is appropriate
typedef TStaticArray<BoneTrackPair, MAX_BONES> BoneTrackArray;


/**
 * Extracts a single BoneAtom from an Animation Sequence.
 *
 * @param	OutAtom			The BoneAtom to fill with the extracted result.
 * @param	Seq				An Animation Sequence to extract the BoneAtom from.
 * @param	TrackIndex		The index of the track desired in the Animation Sequence.
 * @param	Time			The time (in seconds) to calculate the BoneAtom for.
 * @param	bLooping		TRUE if the animation should be played in a cyclic manner.
 */
void AnimationFormat_GetBoneAtom(	FBoneAtom& OutAtom,
									const UAnimSequence& Seq,
									INT TrackIndex,
									FLOAT Time,
									UBOOL bLooping);

#if USE_ANIMATION_CODEC_BATCH_SOLVER

/**
 * Extracts an array of BoneAtoms from an Animation Sequence representing an entire pose of the skeleton.
 *
 * @param	Atoms				The BoneAtoms to fill with the extracted result.
 * @param	RotationTracks		A BoneTrackArray element for each bone requesting rotation data. 
 * @param	TranslationTracks	A BoneTrackArray element for each bone requesting translation data. 
 * @param	Seq					An Animation Sequence to extract the BoneAtom from.
 * @param	Time				The time (in seconds) to calculate the BoneAtom for.
 * @param	bLooping			TRUE if the animation should be played in a cyclic manner.
 */
void AnimationFormat_GetAnimationPose(	
	FBoneAtomArray& Atoms, 
	const BoneTrackArray& RotationTracks,
	const BoneTrackArray& TranslationTracks,
	const UAnimSequence& Seq,
	FLOAT Time,
	UBOOL bLooping);

#endif

/**
 * Handles Byte-swapping incomming animation data from a MemoryReader
 *
 * @param	Seq					An Animation Sequence to contain the read data.
 * @param	MemoryReader		The MemoryReader object to read from.
 */
void AnimationFormat_ByteSwapIn(	UAnimSequence& Seq, 
									FMemoryReader& MemoryReader);

/**
 * Handles Byte-swapping outgoing animation data to an array of BYTEs
 *
 * @param	Seq					An Animation Sequence to write.
 * @param	SerializedData		The output buffer.
 * @param	ForceByteSwapping	TRUE is byte swapping is not optional.
 */
void AnimationFormat_ByteSwapOut(	UAnimSequence& Seq, 
									TArray<BYTE>& SerializedData, 
									UBOOL ForceByteSwapping);

/**
 * Extracts statistics about a given Animation Sequence
 *
 * @param	Seq					An Animation Sequence.
 * @param	NumTransTracks		The total number of Translation Tracks found.
 * @param	NumRotTracks		The total number of Rotation Tracks found.
 * @param	TotalNumTransKeys	The total number of Translation Keys found.
 * @param	TotalNumRotKeys		The total number of Rotation Keys found.
 * @param	TranslationKeySize	The size (in BYTES) of a single Translation Key.
 * @param	RotationKeySize		The size (in BYTES) of a single Rotation Key.
 * @param	NumTransTracksWithOneKey	The total number of Translation Tracks found containing a single key.
 * @param	NumRotTracksWithOneKey		The total number of Rotation Tracks found containing a single key.
*/
void AnimationFormat_GetStats(	const UAnimSequence* Seq, 
  								INT& NumTransTracks,
								INT& NumRotTracks,
  								INT& TotalNumTransKeys,
								INT& TotalNumRotKeys,
								INT& TranslationKeySize,
								INT& RotationKeySize,
								INT& NumTransTracksWithOneKey,
								INT& NumRotTracksWithOneKey);


/**
 * Sets the internal Animation Codec Interface Links within an Animation Sequence
 *
 * @param	Seq					An Animation Sequence to setup links within.
*/
void AnimationFormat_SetInterfaceLinks(UAnimSequence& Seq);

#if !CONSOLE
#define AC_UnalignedSwap( MemoryArchive, Data, Len )		\
	MemoryArchive.ByteOrderSerialize( (Data), (Len) );		\
	(Data) += (Len);
#else
	// No need to swap on consoles, as the cooker will have ordered bytes for the target platform.
#define AC_UnalignedSwap( MemoryArchive, Data, Len )		\
	MemoryArchive.Serialize( (Data), (Len) );				\
	(Data) += (Len);
#endif // !CONSOLE

extern const INT CompressedTranslationStrides[ACF_MAX];
extern const INT CompressedTranslationNum[ACF_MAX];
extern const INT CompressedRotationStrides[ACF_MAX];
extern const INT CompressedRotationNum[ACF_MAX];

class FMemoryWriter;
class FMemoryReader;

void PadMemoryWriter(FMemoryWriter* MemoryWriter, BYTE*& TrackData, const INT Alignment);
void PadMemoryReader(FMemoryReader* MemoryReader, BYTE*& TrackData, const INT Alignment);


class AnimationEncodingFormat
{
public:
	/**
	 * Decompress the Rotation component of a BoneAtom
	 *
	 * @param	OutAtom			The FBoneAtom to fill in.
	 * @param	Seq				The animation sequence to use.
	 * @param	Stream			The compressed animation data.
	 * @param	NumKeys			The number of keys present in Stream.
	 * @param	Time			Current time to solve for.
	 * @param	RelativePos		Current position within the animation to solve for in the range [0.0,1.0].
	 * @param	bLooping		True when looping the stream in intended.
	 * @return					None. 
	 */
	virtual void GetBoneAtomRotation(	
		FBoneAtom& OutAtom,
		const UAnimSequence& Seq,
		const BYTE* RESTRICT Stream,
		INT NumKeys,
		FLOAT Time,
		FLOAT RelativePos,
		UBOOL bLooping) PURE_VIRTUAL(AnimationEncodingFormat::GetBoneAtomRotation,);

	/**
	 * Decompress the Translation component of a BoneAtom
	 *
	 * @param	OutAtom			The FBoneAtom to fill in.
	 * @param	Seq				The animation sequence to use.
	 * @param	Stream			The compressed animation data.
	 * @param	NumKeys			The number of keys present in Stream.
	 * @param	Time			Current time to solve for.
	 * @param	RelativePos		Current position within the animation to solve for in the range [0.0,1.0].
	 * @param	bLooping		True when looping the stream in intended.
	 * @return					None. 
	 */
	virtual void GetBoneAtomTranslation(	
		FBoneAtom& OutAtom,
		const UAnimSequence& Seq,
		const BYTE* RESTRICT Stream,
		INT NumKeys,
		FLOAT Time,
		FLOAT RelativePos,
		UBOOL bLooping) PURE_VIRTUAL(AnimationEncodingFormat::GetBoneAtomTranslation,);

#if USE_ANIMATION_CODEC_BATCH_SOLVER

	/**
	 * Decompress all requested rotation components from an Animation Sequence
	 *
	 * @param	Atoms			The FBoneAtom array to fill in.
	 * @param	DesiredPairs	Array of requested bone information
	 * @param	Seq				The animation sequence to use.
	 * @param	Time			Current time to solve for.
	 * @param	bLooping		True when looping the stream in intended.
	 * @return					None. 
	 */
	virtual void GetPoseRotations(	
		FBoneAtomArray& Atoms, 
		const BoneTrackArray& DesiredPairs,
		const UAnimSequence& Seq,
		FLOAT Time,
		UBOOL bLooping) PURE_VIRTUAL(AnimationEncodingFormat::GetPoseRotations,);

	/**
	 * Decompress all requested translation components from an Animation Sequence
	 *
	 * @param	Atoms			The FBoneAtom array to fill in.
	 * @param	DesiredPairs	Array of requested bone information
	 * @param	Seq				The animation sequence to use.
	 * @param	Time			Current time to solve for.
	 * @param	bLooping		True when looping the stream in intended.
	 * @return					None. 
	 */
	virtual void GetPoseTranslations(	
		FBoneAtomArray& Atoms, 
		const BoneTrackArray& DesiredPairs,
		const UAnimSequence& Seq,
		FLOAT Time,
		UBOOL bLooping) PURE_VIRTUAL(AnimationEncodingFormat::GetPoseTranslations,);
#endif

	/**
	 * Handles the ByteSwap of compressed animation data on import
	 *
	 * @param	Seq				The UAnimSequence container.
	 * @param	MemoryReader	The FMemoryReader to read from.
	 * @param	Stream			The compressed animation data.
	 * @param	NumKeys			The number of keys present in Stream.
	 * @return					The adjusted Stream position after import. 
	 */
	virtual void ByteSwapRotationIn(
		UAnimSequence& Seq, 
		FMemoryReader& MemoryReader,
		BYTE*& Stream,
		INT NumKeys) PURE_VIRTUAL(AnimationEncodingFormat::ByteSwapRotationIn,);

	/**
	 * Handles the ByteSwap of compressed animation data on import
	 *
	 * @param	Seq				The UAnimSequence container.
	 * @param	MemoryReader	The FMemoryReader to read from.
	 * @param	Stream			The compressed animation data.
	 * @param	NumKeys			The number of keys present in Stream.
	 * @return					The adjusted Stream position after import. 
	 */
	virtual void ByteSwapTranslationIn(
		UAnimSequence& Seq, 
		FMemoryReader& MemoryReader,
		BYTE*& Stream,
		INT NumKeys) PURE_VIRTUAL(AnimationEncodingFormat::ByteSwapTranslationIn,);

	/**
	 * Handles the ByteSwap of compressed animation data on export
	 *
	 * @param	Seq				The UAnimSequence container.
	 * @param	MemoryWriter	The FMemoryReader to write to.
	 * @param	Stream			The compressed animation data.
	 * @param	NumKeys			The number of keys present in Stream.
	 * @return					The adjusted Stream position after export. 
	 */
	virtual void ByteSwapRotationOut(
		UAnimSequence& Seq, 
		FMemoryWriter& MemoryWriter,
		BYTE*& Stream,
		INT NumKeys) PURE_VIRTUAL(AnimationEncodingFormat::ByteSwapRotationOut,);

	/**
	 * Handles the ByteSwap of compressed animation data on export
	 *
	 * @param	Seq				The UAnimSequence container.
	 * @param	MemoryWriter	The FMemoryReader to write to.
	 * @param	Stream			The compressed animation data.
	 * @param	NumKeys			The number of keys present in Stream.
	 * @return					The adjusted Stream position after export. 
	 */
	virtual void ByteSwapTranslationOut(
		UAnimSequence& Seq, 
		FMemoryWriter& MemoryWriter,
		BYTE*& Stream,
		INT NumKeys) PURE_VIRTUAL(AnimationEncodingFormat::ByteSwapTranslationOut,);
};

#endif

#endif // __ANIMATIONENCODINGFORMAT_H__
