/*=============================================================================
	AnimationUtils.h: Skeletal mesh animation utilities.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#ifndef __ANIMATIONUTILS_H__
#define __ANIMATIONUTILS_H__

// Forward declarations.
class UAnimationCompressionAlgorithm;
class UAnimSequence;
class UAnimSet;
class USkeletalMesh;
struct FAnimSetMeshLinkup;
struct FBoneAtom;
struct FMeshBone;
struct FRotationTrack;
struct FTranslationTrack;

/**
 * Encapsulates commonly useful data about bones.
 */
class FBoneData
{
public:
	FQuat		Orientation;
	FVector		Position;
	/** Bone name. */
	FName		Name;
	/** Direct descendants.  Empty for end effectors. */
	TArray<INT> Children;
	/** List of bone indices from parent up to root. */
	TArray<INT>	BonesToRoot;
	/** List of end effectors for which this bone is an ancestor.  End effectors have only one element in this list, themselves. */
	TArray<INT>	EndEffectors;

	/**	@return		Index of parent bone; -1 for the root. */
	INT GetParent() const
	{
		return GetDepth() ? BonesToRoot(0) : -1;
	}
	/**	@return		Distance to root; 0 for the root. */
	INT GetDepth() const
	{
		return BonesToRoot.Num();
	}
	/** @return		TRUE if this bone is an end effector (has no children). */
	UBOOL IsEndEffector() const
	{
		return Children.Num() == 0;
	}
};

/**
 * A set of error statistics for an animation, gathered by FAnimationUtils::ComputeCompressionError
 */
struct AnimationErrorStats
{
	/** Average world-space translation error across all end-effectors **/
	FLOAT AverageError;
	/** The worst error encountered across all end effectors **/
	FLOAT MaxError;
	/** Time at which the worst error occurred */
	FLOAT MaxErrorTime;
	/** Bone on which the worst error occurred */
	INT MaxErrorBone;
};

/**
 * A collection of useful functions for skeletal mesh animation.
 */
class FAnimationUtils
{
public:
	/**
	 * Builds the local-to-component transformation for all bones.
	 */
	static void BuildComponentSpaceTransforms(TArray<FMatrix>& OutTransforms,
												const TArray<FBoneAtom>& LocalAtoms,
												const TArray<BYTE>& RequiredBones,
												const TArray<FMeshBone>& BoneData);

	/**
	* Builds the local-to-component matrix for the specified bone.
	*/
	static void BuildComponentSpaceTransform(FMatrix& OutTransform,
												INT BoneIndex,
												const TArray<FBoneAtom>& LocalAtoms,
												const TArray<FBoneData>& BoneData);

	static void BuildPoseFromRawSequenceData(TArray<FBoneAtom>& OutAtoms,
												UAnimSequence* AnimSeq,
												const TArray<BYTE>& RequiredBones,
												USkeletalMesh* SkelMesh,
												FLOAT Time,
												UBOOL bLooping);

/*	static void BuildPoseFromReducedKeys(TArray<FBoneAtom>& OutAtoms,
											const FAnimSetMeshLinkup& AnimLinkup,
											const TArray<FTranslationTrack>& TranslationData,
											const TArray<FRotationTrack>& RotationData,
											const TArray<BYTE>& RequiredBones,
											USkeletalMesh* SkelMesh,
											FLOAT SequenceLength,
											FLOAT Time,
											UBOOL bLooping);

	static void BuildPoseFromReducedKeys(TArray<FBoneAtom>& OutAtoms,
											UAnimSequence* AnimSeq,
											const TArray<BYTE>& RequiredBones,
											USkeletalMesh* SkelMesh,
											FLOAT Time,
											UBOOL bLooping);
*/
	static void BuildSekeltonMetaData(const TArray<FMeshBone>& Skeleton, TArray<FBoneData>& OutBoneData);


	/**
	 * Utility function to measure the accuracy of a compressed animation. Each end-effector is checked for 
	 * world-space movement as a result of compression.
	 *
	 * @param	AnimSet		The animset to calculate compression error for.
	 * @param	SkelMesh	The skeletal mesh to use to check for world-space error (required)
	 * @param	BoneData	BoneData array describing the hierarchy of the animated skeleton
	 * @param	ErrorStats	Output structure containing the final compression error values
	 * @return				None.
	 */
	static void ComputeCompressionError(const UAnimSequence* AnimSeq, USkeletalMesh* SkelMesh, const TArray<FBoneData>& BoneData, AnimationErrorStats& ErrorStats);

	/**
	 * Utility function to compress an animation. If the animation is currently associated with a codec, it will be used to 
	 * compress the animation. Otherwise, the default codec will be used. If AllowAlternateCompressor is TRUE, an
	 * alternative compression codec will also be tested. If the alternative codec produces better compression and 
	 * the accuracy of the compressed animation remains within tolerances, the alternative codec will be used. 
	 * See GetAlternativeCompressionThreshold for information on the tolerance value used.
	 *
	 * @param	AnimSet		The animset to compress.
	 * @param	SkelMesh	The skeletal mesh against which to compress the animation.  Not needed by all compression schemes.
	 * @param	AllowAlternateCompressor	TRUE if an alternative compressor is permitted.
	 * @param	bOutput		If FALSE don't generate output or compute memory savings.
	 * @return				None.
	 */
	static void CompressAnimSequence(UAnimSequence* AnimSeq, USkeletalMesh* SkelMesh = NULL, UBOOL AllowAlternateCompressor = FALSE, UBOOL bOutput = FALSE);

	/**
	 * Determines the current setting for world-space error tolerance in the animation compressor.
	 * When requested, animation being compressed will also consider an alternative compression
	 * method if the end result of that method produces less error than the AlternativeCompressionThreshold.
	 * The default tolerance value is 0.0f (no alternatives allowed) but may be overriden using a field in the base engine INI file.
	 *
	 * @return				World-space error tolerance for considering an alternative compression method
	 */
	static FLOAT GetAlternativeCompressionThreshold();
	
	/**
	 * Determines the current setting for recompressing all animations upon load. The default value 
	 * is False, but may be overridden by an optional field in the base engine INI file. 
	 *
	 * @return				TRUE if the engine settings request that all animations be recompiled
	 */
	static UBOOL GetForcedRecompressionSetting();

private:
	/**
	 * @return		The default animation compression algorithm singleton, instantiating it if necessary.
	 */
	static UAnimationCompressionAlgorithm* GetDefaultAnimationCompressionAlgorithm();

};


#endif // __ANIMATIONUTILS_H__
