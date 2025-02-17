/*=============================================================================
	AnimationUtils.cpp: Skeletal mesh animation utilities.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#include "EnginePrivate.h"
#include "EngineAnimClasses.h"
#include "AnimationUtils.h"
#include "AnimationCompression.h"
#include "AnimationEncodingFormat.h"

void FAnimationUtils::BuildSekeltonMetaData(const TArray<FMeshBone>& Skeleton, TArray<FBoneData>& OutBoneData)
{
	// Assemble bone data.
	OutBoneData.Empty();
	OutBoneData.AddZeroed( Skeleton.Num() );

	for ( INT BoneIndex = 0 ; BoneIndex < Skeleton.Num() ; ++BoneIndex )
	{
		FBoneData& BoneData = OutBoneData(BoneIndex);

		// Copy over data from the skeleton.
		const FMeshBone& SrcBone = Skeleton(BoneIndex);
		BoneData.Orientation = SrcBone.BonePos.Orientation;
		BoneData.Position = SrcBone.BonePos.Position;
		BoneData.Name = SrcBone.Name;

		if ( BoneIndex > 0 )
		{
			// Compute ancestry.
			INT ParentIndex = Skeleton(BoneIndex).ParentIndex;
			BoneData.BonesToRoot.AddItem( ParentIndex );
			while ( ParentIndex > 0 )
			{
				ParentIndex = Skeleton(ParentIndex).ParentIndex;
				BoneData.BonesToRoot.AddItem( ParentIndex );
			}
		}
	}

	// Enumerate children (bones that refer to this bone as parent).
	for ( INT BoneIndex = 0 ; BoneIndex < OutBoneData.Num() ; ++BoneIndex )
	{
		FBoneData& BoneData = OutBoneData(BoneIndex);
		// Exclude the root bone as it is the child of nothing.
		for ( INT BoneIndex2 = 1 ; BoneIndex2 < OutBoneData.Num() ; ++BoneIndex2 )
		{
			if ( OutBoneData(BoneIndex2).GetParent() == BoneIndex )
			{
				BoneData.Children.AddItem(BoneIndex2);
			}
		}
	}

	// Enumerate end effectors.  For each end effector, propagate its index up to all ancestors.
	for ( INT BoneIndex = 0 ; BoneIndex < OutBoneData.Num() ; ++BoneIndex )
	{
		FBoneData& BoneData = OutBoneData(BoneIndex);
		if ( BoneData.IsEndEffector() )
		{
			// End effectors have themselves as an ancestor.
			BoneData.EndEffectors.AddItem( BoneIndex );
			// Add the end effector to the list of end effectors of all ancestors.
			for ( INT i = 0 ; i < BoneData.BonesToRoot.Num() ; ++i )
			{
				const INT AncestorIndex = BoneData.BonesToRoot(i);
				OutBoneData(AncestorIndex).EndEffectors.AddItem( BoneIndex );
			}
		}
	}
#if 0
	debugf( TEXT("====END EFFECTORS:") );
	INT NumEndEffectors = 0;
	for ( INT BoneIndex = 0 ; BoneIndex < OutBoneData.Num() ; ++BoneIndex )
	{
		const FBoneData& BoneData = OutBoneData(BoneIndex);
		if ( BoneData.IsEndEffector() )
		{
			FString Message( FString::Printf(TEXT("%s(%i): "), *BoneData.Name, BoneData.GetDepth()) );
			for ( INT i = 0 ; i < BoneData.BonesToRoot.Num() ; ++i )
			{
				const INT AncestorIndex = BoneData.BonesToRoot(i);
				Message += FString::Printf( TEXT("%s "), *OutBoneData(AncestorIndex).Name );
			}
			debugf( *Message );
			++NumEndEffectors;
		}
	}
	debugf( TEXT("====NUM EFFECTORS %i(%i)"), NumEndEffectors, OutBoneData(0).Children.Num() );
	debugf( TEXT("====NON END EFFECTORS:") );
	for ( INT BoneIndex = 0 ; BoneIndex < OutBoneData.Num() ; ++BoneIndex )
	{
		const FBoneData& BoneData = OutBoneData(BoneIndex);
		if ( !BoneData.IsEndEffector() )
		{
			FString Message( FString::Printf(TEXT("%s(%i): "), *BoneData.Name, BoneData.GetDepth()) );
			Message += TEXT("Children: ");
			for ( INT i = 0 ; i < BoneData.Children.Num() ; ++i )
			{
				const INT ChildIndex = BoneData.Children(i);
				Message += FString::Printf( TEXT("%s "), *OutBoneData(ChildIndex).Name );
			}
			Message += TEXT("  EndEffectors: ");
			for ( INT i = 0 ; i < BoneData.EndEffectors.Num() ; ++i )
			{
				const INT EndEffectorIndex = BoneData.EndEffectors(i);
				Message += FString::Printf( TEXT("%s "), *OutBoneData(EndEffectorIndex).Name );
				check( OutBoneData(EndEffectorIndex).IsEndEffector() );
			}
			debugf( *Message );
		}
	}
	debugf( TEXT("===================") );
#endif
}

/**
* Builds the local-to-component transformation for all bones.
*/
void FAnimationUtils::BuildComponentSpaceTransforms(TArray<FMatrix>& OutTransforms,
												const TArray<FBoneAtom>& LocalAtoms,
												const TArray<BYTE>& RequiredBones,
												const TArray<FMeshBone>& RefSkel)
{
	OutTransforms.Empty();
	OutTransforms.Add( RefSkel.Num() );

	for ( INT i = 0 ; i < RequiredBones.Num() ; ++i )
	{
		const INT BoneIndex = RequiredBones(i);
		LocalAtoms(BoneIndex).ToTransform( OutTransforms(BoneIndex) );

		// For all bones below the root, final component-space transform is relative transform * component-space transform of parent.
		if ( BoneIndex > 0 )
		{
			const INT ParentIndex = RefSkel(BoneIndex).ParentIndex;

			// Check the precondition that parents occur before children in the RequiredBones array.
			const INT ReqBoneParentIndex = RequiredBones.FindItemIndex(ParentIndex);
			check( ReqBoneParentIndex != INDEX_NONE );
			check( ReqBoneParentIndex < i );

			OutTransforms(BoneIndex) *= OutTransforms(ParentIndex);
		}
	}
}

/**
* Builds the local-to-component matrix for the specified bone.
*/
void FAnimationUtils::BuildComponentSpaceTransform(FMatrix& OutTransform,
											   INT BoneIndex,
											   const TArray<FBoneAtom>& LocalAtoms,
											   const TArray<FBoneData>& BoneData)
{
	// Put root-to-component in OutTransform.
	LocalAtoms(0).ToTransform( OutTransform );

	if ( BoneIndex > 0 )
	{
		const FBoneData& Bone = BoneData(BoneIndex);

		checkSlow( Bone.BonesToRoot.Num()-1 == 0 );

		// Compose BoneData.BonesToRoot down.
		FMatrix ToParent;
		for ( INT i = Bone.BonesToRoot.Num()-2 ; i >=0 ; --i )
		{
			const INT AncestorIndex = Bone.BonesToRoot(i);
			LocalAtoms(AncestorIndex).ToTransform( ToParent );
			ToParent *= OutTransform;
			OutTransform = ToParent;
		}

		// Finally, include the bone's local-to-parent.
		LocalAtoms(BoneIndex).ToTransform( ToParent );
		ToParent *= OutTransform;
		OutTransform = ToParent;
	}
}

void FAnimationUtils::BuildPoseFromRawSequenceData(TArray<FBoneAtom>& OutAtoms,
											   UAnimSequence* AnimSeq,
											   const TArray<BYTE>& RequiredBones,
											   USkeletalMesh* SkelMesh,
											   FLOAT Time,
											   UBOOL bLooping)
{
	// Get/create a linkup cache for the mesh to compress against.
	UAnimSet* AnimSet						= AnimSeq->GetAnimSet();
	const INT AnimLinkupIndex				= AnimSet->GetMeshLinkupIndex( SkelMesh );
	check( AnimLinkupIndex != INDEX_NONE );
	check( AnimLinkupIndex < AnimSet->LinkupCache.Num() );

	const FAnimSetMeshLinkup& AnimLinkup	= AnimSet->LinkupCache( AnimLinkupIndex );
	const TArray<FMeshBone>& RefSkel		= SkelMesh->RefSkeleton;
	check( AnimLinkup.BoneToTrackTable.Num() == RefSkel.Num() );

	for ( INT i = 0 ; i < RequiredBones.Num() ; ++i )
	{
		// Map the bone index to sequence track.
		const INT BoneIndex					= RequiredBones(i);
		const INT TrackIndex				= AnimLinkup.BoneToTrackTable(BoneIndex);
		if ( TrackIndex == INDEX_NONE )
		{
			// No track for the bone was found, so use the reference pose.
			OutAtoms(BoneIndex)	= FBoneAtom( RefSkel(BoneIndex).BonePos.Orientation, RefSkel(BoneIndex).BonePos.Position, 1.f );
		}
		else
		{
			// Get pose information using the raw data.
			AnimSeq->GetBoneAtom( OutAtoms(BoneIndex), TrackIndex, Time, bLooping, TRUE );
		}

		// Apply quaternion fix for ActorX-exported quaternions.
		if( BoneIndex > 0 ) 
		{
			OutAtoms(BoneIndex).Rotation.W *= -1.0f;
		}
	}
}

/*
void FAnimationUtils::BuildPoseFromReducedKeys(TArray<FBoneAtom>& OutAtoms,
										   const FAnimSetMeshLinkup& AnimLinkup,
										   const TArray<FTranslationTrack>& TranslationData,
										   const TArray<FRotationTrack>& RotationData,
										   const TArray<BYTE>& RequiredBones,
										   USkeletalMesh* SkelMesh,
										   FLOAT SequenceLength,
										   FLOAT Time,
										   UBOOL bLooping)
{
	const TArray<FMeshBone>& RefSkel		= SkelMesh->RefSkeleton;
	check( AnimLinkup.BoneToTrackTable.Num() == RefSkel.Num() );

	for ( INT i = 0 ; i < RequiredBones.Num() ; ++i )
	{
		// Map the bone index to sequence track.
		const INT BoneIndex					= RequiredBones(i);
		const INT TrackIndex				= AnimLinkup.BoneToTrackTable(BoneIndex);
		if ( TrackIndex == INDEX_NONE )
		{
			// No track for the bone was found, so use the reference pose.
			OutAtoms(BoneIndex)	= FBoneAtom( RefSkel(BoneIndex).BonePos.Orientation, RefSkel(BoneIndex).BonePos.Position, 1.f );
		}
		else
		{
			// Build the bone pose using the key data.
			UAnimSequence::ReconstructBoneAtom( OutAtoms(BoneIndex), TranslationData(TrackIndex), RotationData(TrackIndex), SequenceLength, Time, bLooping );
		}

		// Apply quaternion fix for ActorX-exported quaternions.
		if( BoneIndex > 0 ) 
		{
			OutAtoms(BoneIndex).Rotation.W *= -1.0f;
		}
	}
}

void FAnimationUtils::BuildPoseFromReducedKeys(TArray<FBoneAtom>& OutAtoms,
										   UAnimSequence* AnimSeq,
										   const TArray<BYTE>& RequiredBones,
										   USkeletalMesh* SkelMesh,
										   FLOAT Time,
										   UBOOL bLooping)
{
	// Get/create a linkup cache for the mesh to compress against.
	UAnimSet* AnimSet						= AnimSeq->GetAnimSet();
	const INT AnimLinkupIndex				= AnimSet->GetMeshLinkupIndex( SkelMesh );
	check( AnimLinkupIndex != INDEX_NONE );
	check( AnimLinkupIndex < AnimSet->LinkupCache.Num() );

	FAnimationUtils::BuildPoseFromReducedKeys( OutAtoms,
		AnimSet->LinkupCache( AnimLinkupIndex ),
		AnimSeq->TranslationData,
		AnimSeq->RotationData,
		RequiredBones,
		SkelMesh,
		AnimSeq->SequenceLength,
		Time,
		bLooping );
}
*/

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
void FAnimationUtils::ComputeCompressionError(const UAnimSequence* AnimSeq, USkeletalMesh* SkelMesh, const TArray<FBoneData>& BoneData, AnimationErrorStats& ErrorStats)
{
	ErrorStats.AverageError = 0.0f;
	ErrorStats.MaxError = 0.0f;
	ErrorStats.MaxErrorBone = 0;
	ErrorStats.MaxErrorTime = 0.0f;

#if USE_ANIMATION_CODEC_INTERFACE
	if (AnimSeq->NumFrames > 0)
	{
		INT NumTransTracks = 0;
		INT NumRotTracks = 0;
		INT TotalNumTransKeys = 0;
		INT TotalNumRotKeys = 0;
		INT TranslationKeySize = 0;
		INT RotationKeySize = 0;
		INT NumTransTracksWithOneKey = 0;
		INT NumRotTracksWithOneKey = 0;

		AnimationFormat_GetStats(	AnimSeq, 
						  				NumTransTracks,
										NumRotTracks,
						  				TotalNumTransKeys,
										TotalNumRotKeys,
										TranslationKeySize,
										RotationKeySize,
										NumTransTracksWithOneKey,
										NumRotTracksWithOneKey);

		const FLOAT TimeStep = (FLOAT)AnimSeq->SequenceLength/(FLOAT)AnimSeq->NumFrames;
		const INT NumBones = BoneData.Num();
		
		FLOAT ErrorCount = 0.0f;
		FLOAT ErrorTotal = 0.0f;

		UAnimSet* AnimSet						= AnimSeq->GetAnimSet();
		const INT AnimLinkupIndex				= AnimSet->GetMeshLinkupIndex( SkelMesh );
		check( AnimLinkupIndex != INDEX_NONE );
		check( AnimLinkupIndex < AnimSet->LinkupCache.Num() );

		const FAnimSetMeshLinkup& AnimLinkup	= AnimSet->LinkupCache( AnimLinkupIndex );
		const TArray<FMeshBone>& RefSkel		= SkelMesh->RefSkeleton;
		check( AnimLinkup.BoneToTrackTable.Num() == RefSkel.Num() );

		TArray<FBoneAtom> RawAtoms;
		TArray<FBoneAtom> NewAtoms;
		TArray<FMatrix> RawTransforms;
		TArray<FMatrix> NewTransforms;

		RawAtoms.AddZeroed(NumBones);
		NewAtoms.AddZeroed(NumBones);
		RawTransforms.AddZeroed(NumBones);
		NewTransforms.AddZeroed(NumBones);

		// for each whole increment of time (frame stepping)
		for ( FLOAT Time = 0.0f; Time < AnimSeq->SequenceLength; Time+= TimeStep )
		{
			// get the raw and compressed atom for each bone
			for ( INT BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex )
			{
				const INT TrackIndex			= AnimLinkup.BoneToTrackTable(BoneIndex);

				if ( TrackIndex == INDEX_NONE )
				{
					// No track for the bone was found, so use the reference pose.
					RawAtoms(BoneIndex)	= FBoneAtom( RefSkel(BoneIndex).BonePos.Orientation, RefSkel(BoneIndex).BonePos.Position, 1.f );
					NewAtoms(BoneIndex) = RawAtoms(BoneIndex);
				}
				else
				{
					AnimSeq->GetBoneAtom(RawAtoms(BoneIndex), TrackIndex, Time, FALSE, TRUE);
					AnimSeq->GetBoneAtom(NewAtoms(BoneIndex), TrackIndex, Time, FALSE, FALSE);

					// Apply quaternion fix for ActorX-exported quaternions.
					if( BoneIndex > 0 ) 
					{
						RawAtoms(BoneIndex).Rotation.W *= -1.0f;
						NewAtoms(BoneIndex).Rotation.W *= -1.0f;
					}

					// apply the reference bone atom (if needed)
					if (AnimSeq->bIsAdditive)
					{
						FBoneAtom RefBoneAtom = AnimSeq->AdditiveRefPose(TrackIndex);
						
						// Apply quaternion fix for ActorX-exported quaternions.
						if( BoneIndex > 0 )
						{
							RefBoneAtom.Rotation.W *= -1.0f;
						}

						// apply additive amount to the reference pose
						RawAtoms(BoneIndex).Translation += RefBoneAtom.Translation;
						NewAtoms(BoneIndex).Translation += RefBoneAtom.Translation;

						// Add ref pose relative animation to base animation, only if rotation is significant.
						if( Square(RawAtoms(BoneIndex).Rotation.W) < 1.f - DELTA * DELTA )
						{
							RawAtoms(BoneIndex).Rotation = RawAtoms(BoneIndex).Rotation * RefBoneAtom.Rotation;
						}
						else
						{
							RawAtoms(BoneIndex).Rotation = RefBoneAtom.Rotation;
						}
						if( Square(NewAtoms(BoneIndex).Rotation.W) < 1.f - DELTA * DELTA )
						{
							NewAtoms(BoneIndex).Rotation = NewAtoms(BoneIndex).Rotation * RefBoneAtom.Rotation;
						}
						else
						{
							NewAtoms(BoneIndex).Rotation = RefBoneAtom.Rotation;
						}
					}

				}

				RawAtoms(BoneIndex).ToTransform( RawTransforms(BoneIndex) );
				NewAtoms(BoneIndex).ToTransform( NewTransforms(BoneIndex) );

				// For all bones below the root, final component-space transform is relative transform * component-space transform of parent.
				if ( BoneIndex > 0 )
				{
					const INT ParentIndex = RefSkel(BoneIndex).ParentIndex;

					// Check the precondition that parents occur before children in the RequiredBones array.
					check( ParentIndex != INDEX_NONE );
					check( ParentIndex < BoneIndex );

					RawTransforms(BoneIndex) *= RawTransforms(ParentIndex);
					NewTransforms(BoneIndex) *= NewTransforms(ParentIndex);
				}
				
				if (BoneData(BoneIndex).IsEndEffector())
				{
					FLOAT Error = (RawTransforms(BoneIndex).GetOrigin()-NewTransforms(BoneIndex).GetOrigin()).Size();

					ErrorTotal += Error;
					ErrorCount += 1.0f;

					if( Error > ErrorStats.MaxError )
					{
						ErrorStats.MaxError		= Error;
						ErrorStats.MaxErrorBone = BoneIndex;
						ErrorStats.MaxErrorTime = Time;
					}
				}
			}
		}

		if (ErrorCount > 0.0f)
		{
			ErrorStats.AverageError = ErrorTotal / ErrorCount;
		}

// 		// That's a big error, log out some information!
// 		if( ErrorStats.MaxError > 5.f )
// 		{
// 			debugf(TEXT("!!! Big error found: %f, Time: %f, BoneIndex: %d, CompressionScheme: %s, additive: %d"), 
// 				ErrorStats.MaxError, ErrorStats.MaxErrorTime, ErrorStats.MaxErrorBone, AnimSeq->CompressionScheme ? *AnimSeq->CompressionScheme->GetFName().ToString() : TEXT("NULL"), AnimSeq->bIsAdditive );
// 			debugf(TEXT("   RawOrigin: %s, NormalOrigin: %s"), *RawTransforms(ErrorStats.MaxErrorBone).GetOrigin().ToString(), *NewTransforms(ErrorStats.MaxErrorBone).GetOrigin().ToString());
// 
// 			// We shouldn't have a big error with no compression.
// 			if( !AnimSeq->CompressionScheme )
// 			{
// 				assert(FALSE);
// 			}
// 		}	
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Defualt animation compression algorithm.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {

/**
 * @return		A new instance of the default animation compression algorithm singleton, attached to the root set.
 */
static inline UAnimationCompressionAlgorithm* ConstructDefaultCompressionAlgorithm()
{
	// Algorithm.
	FString DefaultCompressionAlgorithm( TEXT("AnimationCompressionAlgorithm_BitwiseCompressOnly") );
	GConfig->GetString( TEXT("AnimationCompression"), TEXT("DefaultCompressionAlgorithm"), DefaultCompressionAlgorithm, GEngineIni );

	// Rotation compression format.
	AnimationCompressionFormat RotationCompressionFormat = ACF_Float96NoW;
	GConfig->GetInt( TEXT("AnimationCompression"), TEXT("RotationCompressionFormat"), (INT&)RotationCompressionFormat, GEngineIni );
	Clamp( RotationCompressionFormat, ACF_None, ACF_MAX );

	// Translation compression format.
	AnimationCompressionFormat TranslationCompressionFormat = ACF_None;
	GConfig->GetInt( TEXT("AnimationCompression"), TEXT("TranslationCompressionFormat"), (INT&)TranslationCompressionFormat, GEngineIni );
	Clamp( TranslationCompressionFormat, ACF_None, ACF_MAX );

	// Find a class that inherits
	UClass* CompressionAlgorithmClass = NULL;
	for ( TObjectIterator<UClass> It ; It ; ++It )
	{
		UClass* Class = *It;
		if( !(Class->ClassFlags & CLASS_Abstract) && !(Class->ClassFlags & CLASS_Deprecated) )
		{
			if ( Class->IsChildOf(UAnimationCompressionAlgorithm::StaticClass()) && DefaultCompressionAlgorithm == Class->GetName() )
			{
				CompressionAlgorithmClass = Class;
				break;

			}
		}
	}

	if ( !CompressionAlgorithmClass )
	{
		appErrorf( TEXT("Couldn't find animation compression algorithm named %s"), *DefaultCompressionAlgorithm );
	}

	UAnimationCompressionAlgorithm* NewAlgorithm = ConstructObject<UAnimationCompressionAlgorithm>( CompressionAlgorithmClass );
	NewAlgorithm->RotationCompressionFormat = RotationCompressionFormat;
	NewAlgorithm->TranslationCompressionFormat = TranslationCompressionFormat;
	NewAlgorithm->AddToRoot();
	return NewAlgorithm;
}

} // namespace

/**
 * @return		The default animation compression algorithm singleton, instantiating it if necessary.
 */
UAnimationCompressionAlgorithm* FAnimationUtils::GetDefaultAnimationCompressionAlgorithm()
{
	static UAnimationCompressionAlgorithm* SAlgorithm = ConstructDefaultCompressionAlgorithm();
	return SAlgorithm;
}

/**
 * Determines the current setting for world-space error tolerance in the animation compressor.
 * When requested, animation being compressed will also consider an alternative compression
 * method if the end result of that method produces less error than the AlternativeCompressionThreshold.
 * The default tolerance value is 0.0f (no alternatives allowed) but may be overriden using a field in the base engine INI file.
 *
 * @return				World-space error tolerance for considering an alternative compression method
 */
FLOAT FAnimationUtils::GetAlternativeCompressionThreshold()
{
	// Allow the Engine INI file to provide a new override
	FLOAT AlternativeCompressionThreshold = 0.0f;
	GConfig->GetFloat( TEXT("AnimationCompression"), TEXT("AlternativeCompressionThreshold"), (FLOAT&)AlternativeCompressionThreshold, GEngineIni );

	return AlternativeCompressionThreshold;
}

/**
 * Determines the current setting for recompressing all animations upon load. The default value 
 * is False, but may be overridden by an optional field in the base engine INI file. 
 *
 * @return				TRUE if the engine settings request that all animations be recompiled
 */
UBOOL FAnimationUtils::GetForcedRecompressionSetting()
{
	// Allow the Engine INI file to provide a new override
	UBOOL ForcedRecompressionSetting = FALSE;
	GConfig->GetBool( TEXT("AnimationCompression"), TEXT("ForceRecompression"), (UBOOL&)ForcedRecompressionSetting, GEngineIni );

	return ForcedRecompressionSetting;
}

#if CONSOLE
	#define TRYCOMPRESSION(compressionname,winningcompressor,compressionalgorithm)	
#else
	#define TRYCOMPRESSION(compressionname,winningcompressor,compressionalgorithm)																			\
{																																							\
	/* assume we'll be sticking with the original format (for now)*/																						\
	UBOOL KeepNewCompressionMethod = FALSE;																													\
																																							\
	/* try the alternative compressor	*/																													\
	(##compressionalgorithm)->Reduce( AnimSeq, DefaultSkeletalMesh, bOutput );																				\
	INT NewSize = AnimSeq->GetResourceSize();																												\
																																							\
	/* compute the savings and compression error*/																											\
	INT MemorySavingsFromOriginal = OriginalSize - NewSize;																									\
	INT MemorySavingsFromPrevious = CurrentSize - NewSize;																									\
	PctSaving = 0.f;																																		\
	if( MemorySavingsFromPrevious > 0 )																														\
	{																																						\
		/* figure out our new compression error*/																											\
		FAnimationUtils::ComputeCompressionError(AnimSeq, DefaultSkeletalMesh, BoneData, NewErrorStats);													\
																																							\
		PctSaving = OriginalSize > 0 ? 100.f - (100.f * FLOAT(NewSize) / FLOAT(OriginalSize)) : 0.f;														\
		warnf(TEXT("- %s - bytes saved: %i (%3.1f%% saved), error: %f"), ##compressionname, MemorySavingsFromOriginal, PctSaving, NewErrorStats.MaxError);	\
																																							\
		/* if the error has been reduced, we'll use the new compression format */																			\
		if( NewErrorStats.MaxError <= MasterTolerance || NewErrorStats.MaxError < OriginalErrorStats.MaxError )												\
		{																																					\
			KeepNewCompressionMethod = TRUE;																												\
			WinningCompressor = &(##winningcompressor);																										\
			CurrentSize = NewSize;																															\
			WinningCompressorSavings = MemorySavingsFromOriginal;																							\
		}																																					\
	}																																						\
	else																																					\
	{																																						\
		PctSaving = OriginalSize > 0 ? 100.f - (100.f * FLOAT(NewSize) / FLOAT(OriginalSize)) : 0.f;														\
		warnf(TEXT("- %s - bytes saved: %i (%3.1f%% saved)"), ##compressionname, MemorySavingsFromOriginal,PctSaving );										\
	}																																						\
																																							\
	if( !KeepNewCompressionMethod )																															\
	{																																						\
		/* revert back to the old method by copying back the data we cached */																				\
		AnimSeq->TranslationData = SavedTranslationData;																									\
		AnimSeq->RotationData = SavedRotationData;																											\
		AnimSeq->CompressionScheme = SavedCompressionScheme;																								\
		AnimSeq->TranslationCompressionFormat = SavedTranslationCompressionFormat;																			\
		AnimSeq->RotationCompressionFormat = SavedRotationCompressionFormat;																				\
		AnimSeq->KeyEncodingFormat = SavedKeyEncodingFormat;																								\
		AnimSeq->CompressedTrackOffsets = SavedCompressedTrackOffsets;																						\
		AnimSeq->CompressedByteStream = SavedCompressedByteStream;																							\
		AnimSeq->TranslationCodec = SavedTranslationCodec;																									\
		AnimSeq->RotationCodec = SavedRotationCodec;																										\
																																							\
		INT RestoredSize = AnimSeq->GetResourceSize();																										\
		check(RestoredSize == CurrentSize);																													\
	}																																						\
	else																																					\
	{																																						\
		/* backup key information from the sequence */																										\
		SavedTranslationData				= AnimSeq->TranslationData;																						\
		SavedRotationData					= AnimSeq->RotationData;																						\
		SavedCompressionScheme				= AnimSeq->CompressionScheme;																					\
		SavedTranslationCompressionFormat	= AnimSeq->TranslationCompressionFormat;																		\
		SavedRotationCompressionFormat		= AnimSeq->RotationCompressionFormat;																			\
		SavedKeyEncodingFormat				= AnimSeq->KeyEncodingFormat;																					\
		SavedCompressedTrackOffsets			= AnimSeq->CompressedTrackOffsets;																				\
		SavedCompressedByteStream			= AnimSeq->CompressedByteStream;																				\
		SavedTranslationCodec				= AnimSeq->TranslationCodec;																					\
		SavedRotationCodec					= AnimSeq->RotationCodec;																						\
	}																																						\
}
#endif

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
void FAnimationUtils::CompressAnimSequence(UAnimSequence* AnimSeq, USkeletalMesh* SkelMesh, UBOOL AllowAlternateCompressor, UBOOL bOutput)
{
	static INT TotalRecompressions = 0;
	static INT LinearACF_Float96NoWCompressorWins = 0;
	static INT LinearACF_Fixed48NoWCompressorWins = 0;
	static INT AlternativeCompressorLossesFromSize = 0;
	static INT AlternativeCompressorLossesFromError = 0;
	static INT AlternativeCompressorSavings = 0;
	static INT TotalSizeBefore = 0;
	static INT TotalSizeNow = 0;
	static INT HadNoCompression = 0;
	static INT MissingSkelMesh = 0;
	static TArray<FString>	MissingSkelMeshArray;

	// get the master tolerance we will use to guide recompression
	static FLOAT MasterTolerance = GetAlternativeCompressionThreshold(); 

	UBOOL bOnlyCheckForMissingSkeletalMeshes = FALSE;
	GConfig->GetBool( TEXT("AnimationCompression"), TEXT("bOnlyCheckForMissingSkeletalMeshes"), (UBOOL&)bOnlyCheckForMissingSkeletalMeshes, GEngineIni );

	check(AnimSeq != NULL);
	UAnimSet* AnimSet = AnimSeq->GetAnimSet();
	check(AnimSet != NULL);

	// we must have raw data to continue
	if( AnimSeq->RawAnimData.Num() > 0 )
	{
		// attempt to find the default skeletal mesh associated with this sequence
		USkeletalMesh* DefaultSkeletalMesh = SkelMesh;
		if( !DefaultSkeletalMesh && AnimSet->PreviewSkelMeshName != NAME_None )
		{
			DefaultSkeletalMesh = LoadObject<USkeletalMesh>(NULL, *AnimSet->PreviewSkelMeshName.ToString(), NULL, LOAD_None, NULL);
		}

		// Make sure that animation can be played on that skeletal mesh
		if( DefaultSkeletalMesh && !AnimSet->CanPlayOnSkeletalMesh(DefaultSkeletalMesh) )
		{
			warnf(TEXT("%s cannot be played on %s (%s)."), *AnimSeq->SequenceName.ToString(), *DefaultSkeletalMesh->GetFName().ToString(), *AnimSet->GetFullName());
			DefaultSkeletalMesh = NULL;
		}

		// start with the current technique, or the default if none exists.
		// this will serve as our fallback if no better technique can be found
		UAnimationCompressionAlgorithm* CompressionAlgorithm = AnimSeq->CompressionScheme ? AnimSeq->CompressionScheme : FAnimationUtils::GetDefaultAnimationCompressionAlgorithm();
		CompressionAlgorithm->Reduce( AnimSeq, DefaultSkeletalMesh, bOutput );

#if USE_ANIMATION_CODEC_INTERFACE
#if VARIABLE_KEY_CODEC_ENABLED

		// Check for global permission to try an alternative compressor
		if( AllowAlternateCompressor &&		// alternative compressors are allowed
			MasterTolerance > 0.0f )		// we have a non-zero tolerance for the alternative compressor
		{
			// check this sequence for compliance with the alternative compressor
			if( DefaultSkeletalMesh	&&									// there must be a skeletal mesh to guide compression
				DefaultSkeletalMesh->SkelMeshGUID.IsValid() &&			// Make sure GUID is valid. That could be false is this is trigger before SkelMesh::PostLoad() is called.
				AnimSeq->NumFrames > 2 &&								// there must be more than a single frame of animation
				AnimSeq->KeyEncodingFormat != AKF_VariableKeyLerp &&	// make sure we're not already using the alternative compressor
				!AnimSeq->bDoNotOverrideCompression )
			{
				// If only checking for missing skeletal meshes, abort here.
				// This is useful for displaying only the list of animations which couldn't be recompressed because they had no default skeletal mesh.
				if( bOnlyCheckForMissingSkeletalMeshes )
				{
					return;
				}

				// Get the current size
				INT OriginalSize = AnimSeq->GetResourceSize();
				TotalSizeBefore += OriginalSize;

				// backup key information from the sequence
				TArrayNoInit<struct FTranslationTrack> SavedTranslationData = AnimSeq->TranslationData;
				TArrayNoInit<struct FRotationTrack> SavedRotationData = AnimSeq->RotationData;
				class UAnimationCompressionAlgorithm* SavedCompressionScheme = AnimSeq->CompressionScheme;
				BYTE SavedTranslationCompressionFormat = AnimSeq->TranslationCompressionFormat;
				BYTE SavedRotationCompressionFormat = AnimSeq->RotationCompressionFormat;
				BYTE SavedKeyEncodingFormat = AnimSeq->KeyEncodingFormat;
				TArrayNoInit<INT> SavedCompressedTrackOffsets = AnimSeq->CompressedTrackOffsets;
				TArrayNoInit<BYTE> SavedCompressedByteStream = AnimSeq->CompressedByteStream;
				FPointer SavedTranslationCodec = AnimSeq->TranslationCodec;
				FPointer SavedRotationCodec = AnimSeq->RotationCodec;

				// count all attempts for debugging
				++TotalRecompressions;

				// Build skeleton metadata to use during the key reduction.
				TArray<FBoneData> BoneData;
				FAnimationUtils::BuildSekeltonMetaData( DefaultSkeletalMesh->RefSkeleton, BoneData );
				
				// figure out our current compression error
				AnimationErrorStats OriginalErrorStats;
				FAnimationUtils::ComputeCompressionError(AnimSeq, DefaultSkeletalMesh, BoneData, OriginalErrorStats);
				AnimationErrorStats NewErrorStats = OriginalErrorStats;

				// Prepare to compress
				INT CurrentSize = OriginalSize;
				INT *WinningCompressor = NULL;
				INT WinningCompressorSavings = 0;
				FLOAT PctSaving = 0.f;
				warnf(TEXT("Compressing %s (%s), SkelMesh: %s, size: %i, actual error: %f."), *AnimSeq->SequenceName.ToString(), *AnimSet->GetFullName(), DefaultSkeletalMesh ? *DefaultSkeletalMesh->GetFName().ToString() : TEXT("NULL"), OriginalSize, OriginalErrorStats.MaxError );

				// construct the proposed compressor		
				UAnimationCompressionAlgorithm_RemoveLinearKeys* LinearKeyRemover =
					ConstructObject<UAnimationCompressionAlgorithm_RemoveLinearKeys>( UAnimationCompressionAlgorithm_RemoveLinearKeys::StaticClass() );

				// Defaults to use...
				LinearKeyRemover->MaxPosDiff = 0.1f;
				LinearKeyRemover->MaxAngleDiff = 0.025f;
				LinearKeyRemover->MaxEffectorDiff = 0.01f;
				LinearKeyRemover->MinEffectorDiff = 0.02f;
				LinearKeyRemover->ParentKeyScale = 2.0f;
				LinearKeyRemover->bRetarget = TRUE;

				// Try ACF_Float96NoW
				LinearKeyRemover->RotationCompressionFormat = ACF_Float96NoW;	//RotationCompressionFormat;
				LinearKeyRemover->TranslationCompressionFormat = ACF_None;		//TranslationCompressionFormat;
				TRYCOMPRESSION(TEXT("LinearFixed96"),LinearACF_Float96NoWCompressorWins,LinearKeyRemover)

				// If first compression reduced error and beat size reduction by 70%, skip second pass for speed.
				// This is generally true for cinematics which are *very* long to compress!
				if( TRUE || !(NewErrorStats.MaxError < OriginalErrorStats.MaxError && PctSaving > 70.f) )
				{
					// Try ACF_Fixed48NoW
					LinearKeyRemover->RotationCompressionFormat = ACF_Fixed48NoW;	//RotationCompressionFormat;
					LinearKeyRemover->TranslationCompressionFormat = ACF_None;		//TranslationCompressionFormat;
					TRYCOMPRESSION(TEXT("LinearFixed48"),LinearACF_Fixed48NoWCompressorWins,LinearKeyRemover)
				}

				// Increase winning compressor.
				if( CurrentSize != OriginalSize )
				{
					if( WinningCompressor )
					{
						++(*WinningCompressor);
					}
					AlternativeCompressorSavings += WinningCompressorSavings;
				}
				TotalSizeNow += CurrentSize;

				PctSaving = TotalSizeBefore > 0 ? 100.f - (100.f * FLOAT(TotalSizeNow) / FLOAT(TotalSizeBefore)) : 0.f;
				warnf(TEXT("Compression Stats Summary [%i total, %i Original, %i LinearFloat96, %i LinearFixed48, %i saved, %i before, %i now, %3.1f%% total savings]"), 
					TotalRecompressions,
					HadNoCompression,
					LinearACF_Float96NoWCompressorWins,
					LinearACF_Fixed48NoWCompressorWins,
					AlternativeCompressorSavings,
					TotalSizeBefore, 
					TotalSizeNow,
					PctSaving);
			}
			else
			{
				if( !DefaultSkeletalMesh || !DefaultSkeletalMesh->SkelMeshGUID.IsValid() )
				{
					++MissingSkelMesh;
					if( !DefaultSkeletalMesh )
					{
						warnf(TEXT("%s %s couldn't be compressed! Default Mesh is NULL! PreviewSkelMeshName: %s"), *AnimSet->GetFullName(), *AnimSeq->SequenceName.ToString(), *AnimSet->PreviewSkelMeshName.ToString());
					}
					else
					{
						warnf(TEXT("%s %s couldn't be compressed! Default Mesh is %s, GUID valid? %d, PreviewSkelMeshName: %s"), *AnimSet->GetFullName(), *DefaultSkeletalMesh->GetFullName(), DefaultSkeletalMesh->SkelMeshGUID.IsValid(), *AnimSeq->SequenceName.ToString(), *AnimSet->PreviewSkelMeshName.ToString());
					}

					if( bOnlyCheckForMissingSkeletalMeshes )
					{
						MissingSkelMeshArray.AddUniqueItem(AnimSeq->GetOuter()->GetFullName());
						for(INT i=0; i<MissingSkelMeshArray.Num(); i++)
						{
							warnf(TEXT("[%i] %s"), i, *MissingSkelMeshArray(i));
						}
					}
				}
			}

			warnf(TEXT("Stats [Errors: %i]"), MissingSkelMesh);
		}

#endif
#endif

	}
	else
	{
		warnf(TEXT("Compression Requested for Empty Animation %s"), *AnimSeq->SequenceName.ToString() );
	}
}
