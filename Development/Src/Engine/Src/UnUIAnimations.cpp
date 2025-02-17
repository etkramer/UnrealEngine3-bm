//=============================================================================// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
// Confidential.
//=============================================================================
#include "EnginePrivate.h"
#include "CanvasScene.h"

#include "EngineUserInterfaceClasses.h"
#include "EngineUIPrivateClasses.h"

#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"

IMPLEMENT_CLASS(UUIAnimation);
IMPLEMENT_CLASS(UUIAnimationSeq);

#define DEBUG_UI_ANIMATION	0

/**
 * Utility method for interpolating data from one value to another
 *
 * @param	InterpMode	the type of interpolation to use; should be one of the EUIAnimationInterpMode values
 * @param	SourceValue	the current value of the data being interpolated
 * @param	TargetValue	the value to interpolate to
 * @param	InterpPosition	a value between 0.0 and 1.0 indicating the how close to TargetValue the data should be
 * @param	InterpExponent	for interp modes which have non-linear interpolation, indicates how much the interpolation should be curved.
 *
 * @return	the new value for the data
 */
template<typename ValueType>
ValueType PerformInterpolation( /*EUIAnimationInterpMode*/BYTE InterpMode, const ValueType& SourceValue, const ValueType& TargetValue, FLOAT InterpPosition, FLOAT InterpExponent )
{
	// I think FInterpEaseInOut has a buggy implentation for ease-out; just use our own version..
	//@todo - double check this...
	if ( InterpMode == UIANIMMODE_EaseInOut )
	{
		InterpMode = InterpPosition < 0.5f ? UIANIMMODE_EaseOut : UIANIMMODE_EaseIn;
	}

	switch ( InterpMode )
	{
	case UIANIMMODE_Linear:
	default:
		return Lerp<ValueType>(SourceValue, TargetValue, InterpPosition);

// 	case UIANIMMODE_Cubic:
// 		return CubicInterp(SourceValue, SourceTangent, TargetValue, TargetTangent, InterpPosition);

	case UIANIMMODE_EaseIn:
		return Lerp<ValueType>(SourceValue, TargetValue, appPow(InterpPosition, InterpExponent));

	case UIANIMMODE_EaseOut:
		return Lerp<ValueType>(SourceValue, TargetValue, appPow(InterpPosition, 1 / InterpExponent));
	}
}

/* ==========================================================================================================
	UGameUISceneClient
========================================================================================================== */
/**
 * Attempt to find an animation in the AnimSequencePool.
 *
 * @Param SequenceName		The sequence to find
 * @returns the sequence if it was found otherwise returns none
 */
UUIAnimationSeq* UGameUISceneClient::FindUIAnimation( FName NameOfSequence ) const
{
	UUIAnimationSeq* Result = NULL;

	for ( INT PoolIndex = 0; PoolIndex < AnimSequencePool.Num(); PoolIndex++ )
	{
		if (AnimSequencePool(PoolIndex) != NULL
		&&	AnimSequencePool(PoolIndex)->SeqName == NameOfSequence)
		{
			Result = AnimSequencePool(PoolIndex);
			break;
		}
	}

	return Result;
}

/* ==========================================================================================================
	FUIAnimSequence
========================================================================================================== */
/**
 * Wrapper for verifying whether the index is a valid index for the track's keyframes array.
 *
 * @param	TrackIndex	the index [into the Tracks array] for the track to check
 * @param	FrameIndex	the index [into the KeyFrames array of the track] for the keyframe to check
 *
 * @return	TRUE if the specified track contains a keyframe at the specified index.
 */
UBOOL FUIAnimSequence::IsValidFrameIndex( INT TrackIndex, INT FrameIndex ) const
{
	UBOOL bResult = FALSE;

	if ( AnimationTracks.IsValidIndex(TrackIndex) )
	{
		const FUIAnimTrack& Track = AnimationTracks(TrackIndex);
		bResult = Track.KeyFrames.IsValidIndex(FrameIndex);
	}

	return bResult;
}

/**
 * Wrapper for getting the length of a specific frame in one of this animation sequence's tracks.
 *
 * @param	TrackIndex			the index [into the Tracks array] for the track to check
 * @param	FrameIndex			the index [into the KeyFrames array of the track] for the keyframe to check
 * @param	out_FrameLength		receives the remaining seconds for the frame specified
 *
 * @return	TRUE if the call succeeded; FALSE if an invalid track or frame index was specified.
 */
UBOOL FUIAnimSequence::GetFrameLength( INT TrackIndex, INT FrameIndex, FLOAT& out_FrameLength ) const
{
	UBOOL bResult = FALSE;

	if ( IsValidFrameIndex(TrackIndex, FrameIndex) )
	{
		const FUIAnimTrack& Track = AnimationTracks(TrackIndex);
		const FUIAnimationKeyFrame& Frame = Track.KeyFrames(FrameIndex);

		out_FrameLength = Frame.RemainingTime;
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Wrapper for getting the length of a specific track in this animation sequence.
 *
 * @param	TrackIndex	the index [into the Tracks array] for the track to check
 * @param	out_TrackLength		receives the remaining number of seconds for the track specified.
 *
 * @return	TRUE if the call succeeded; FALSE if an invalid track index was specified.
 */
UBOOL FUIAnimSequence::GetTrackLength( INT TrackIndex, FLOAT& out_TrackLength ) const
{
	UBOOL bResult = FALSE;

	if ( AnimationTracks.IsValidIndex(TrackIndex) )
	{
		out_TrackLength = 0.f;
		const FUIAnimTrack& Track = AnimationTracks(TrackIndex);
		for ( INT FrameIndex = 0; FrameIndex < Track.KeyFrames.Num(); FrameIndex++ )
		{
			const FUIAnimationKeyFrame& Frame = Track.KeyFrames(FrameIndex);
			out_TrackLength += Frame.RemainingTime;
		}

		bResult = TRUE;
	}

	return bResult;
}

/**
 * Wrapper for getting the length of this animation sequence.
 *
 * @return	the total number of seconds in this animation sequence.
 */
FLOAT FUIAnimSequence::GetSequenceLength() const
{
	FLOAT Result = 0.f;

	for ( INT TrackIndex = 0; TrackIndex < AnimationTracks.Num(); TrackIndex++ )
	{
		FLOAT TrackLength = 0.f;
		if ( GetTrackLength(TrackIndex, TrackLength) )
		{
			Result += TrackLength;
		}
	}

	return Result;
}

/**
 * Applies the specified track's current keyframe data to the widget.
 *
 * @param	Target		the widget to apply the update to.
 * @param	TrackIndex	the index of the track to apply data from
 * @param	DeltaTime	the time (in seconds) since the beginning of the last frame; used to determine how much to interpolate
 *						the current keyframe's value.
 *
 * @return	TRUE if the widget's state was updated with the current keyframe's data.
 */
UBOOL FUIAnimSequence::ApplyUIAnimation( UUIScreenObject* Target, INT TrackIndex, FLOAT Delta )
{
	UBOOL bResult = FALSE;

	if ( Target != NULL && AnimationTracks.IsValidIndex(TrackIndex) )
	{
		const FUIAnimTrack& Track = AnimationTracks(TrackIndex);
		if ( Track.KeyFrames.Num() > 0 )
		{
			const FUIAnimationKeyFrame& CurrentFrame = Track.KeyFrames(0);

			const UBOOL bAdvanceFrameToEnd = ARE_FLOATS_EQUAL(Delta, INDEX_NONE) || ARE_FLOATS_EQUAL(CurrentFrame.RemainingTime, 0.f);
			const FLOAT InterpPercentage = bAdvanceFrameToEnd ? 1.f : (Delta / CurrentFrame.RemainingTime);

			FUIAnimationRawData CurrentValue(EC_EventParm);
			switch ( Track.TrackType )
			{
				case EAT_Opacity:
				{
					if ( Target->Anim_GetValue(EAT_Opacity, CurrentValue) )
					{
						const FLOAT FinalOpacity = CurrentFrame.Data.DestAsFloat;
						FLOAT NewOpacity = PerformInterpolation(CurrentFrame.InterpMode, CurrentValue.DestAsFloat, FinalOpacity, InterpPercentage, CurrentFrame.InterpExponent);

#if DEBUG_UI_ANIMATION
	 					debugf(NAME_DevUIAnimation, TEXT("ApplyUIAnimation - Opacity: %f => %f  (Delta:%f   InterpPercentage:%f    TimeLeft:%f)"), CurrentValue.DestAsFloat, NewOpacity, Delta, InterpPercentage, CurrentFrame.RemainingTime);
#endif

						if ( CurrentValue.DestAsFloat > FinalOpacity )
						{
							NewOpacity = Max(NewOpacity, FinalOpacity);
						}
						else
						{
							NewOpacity = Min(NewOpacity, FinalOpacity);
						}

						CurrentValue.DestAsFloat = NewOpacity;
						Target->Anim_SetValue(Track.TrackType, CurrentValue);
					}
					bResult = TRUE;
					break;
				}

				case EAT_Visibility:
				{
					if ( InterpPercentage + DELTA >= 1.f && Target->Anim_GetValue(Track.TrackType, CurrentValue) )
					{
						CurrentValue.DestAsFloat = PerformInterpolation(CurrentFrame.InterpMode, CurrentValue.DestAsFloat, CurrentFrame.Data.DestAsFloat, InterpPercentage, CurrentFrame.InterpExponent);
						Target->Anim_SetValue(Track.TrackType, CurrentValue);
					}
					bResult = TRUE;
					break;
				}

				case EAT_Color:
				{
					if ( Target->Anim_GetValue(Track.TrackType, CurrentValue) )
					{
						CurrentValue.DestAsColor = PerformInterpolation(CurrentFrame.InterpMode, CurrentValue.DestAsColor, CurrentFrame.Data.DestAsColor, InterpPercentage, CurrentFrame.InterpExponent);
						Target->Anim_SetValue(Track.TrackType, CurrentValue);
					}
					bResult = TRUE;
					break;
				}

				case EAT_Scale:
				{
					if ( Target->Anim_GetValue(Track.TrackType, CurrentValue) )
					{
						CurrentValue.DestAsFloat = PerformInterpolation(CurrentFrame.InterpMode, CurrentValue.DestAsFloat, CurrentFrame.Data.DestAsFloat, InterpPercentage, CurrentFrame.InterpExponent);
						Target->Anim_SetValue(Track.TrackType, CurrentValue);
					}
					bResult = TRUE;
					break;
				}

				case EAT_Rotation:
				case EAT_RelRotation:
				{
					if ( Target->Anim_GetValue(Track.TrackType, CurrentValue))
					{
						CurrentValue.DestAsRotator = PerformInterpolation(CurrentFrame.InterpMode, CurrentValue.DestAsRotator, CurrentFrame.Data.DestAsRotator, InterpPercentage, CurrentFrame.InterpExponent);
						Target->Anim_SetValue(Track.TrackType, CurrentValue);
					}
					bResult = TRUE;
					break;
				}

				case EAT_Position:
				case EAT_RelPosition:
				{
					if ( Target->Anim_GetValue(Track.TrackType, CurrentValue) )
					{
						CurrentValue.DestAsVector = PerformInterpolation(CurrentFrame.InterpMode, CurrentValue.DestAsVector, CurrentFrame.Data.DestAsVector, InterpPercentage, CurrentFrame.InterpExponent);
						Target->Anim_SetValue(Track.TrackType, CurrentValue);
					}
					bResult = TRUE;
					break;
				}

				case EAT_PositionOffset:
				{
					if ( Target->Anim_GetValue(Track.TrackType, CurrentValue) )
					{
						FVector NewOffset = PerformInterpolation(CurrentFrame.InterpMode, CurrentValue.DestAsVector, CurrentFrame.Data.DestAsVector, InterpPercentage, CurrentFrame.InterpExponent);
#if DEBUG_UI_ANIMATION
						debugf(NAME_DevUIAnimation, TEXT("ApplyUIAnimation - PositionOffset: %f => %f  (Delta:%f   InterpPercentage:%f  TimeLeft:%f   Target:%f)"),
							CurrentValue.DestAsVector.Y, NewOffset.Y, Delta, InterpPercentage, CurrentFrame.RemainingTime, CurrentFrame.Data.DestAsVector.Y);
#endif
						CurrentValue.DestAsVector = NewOffset;
						Target->Anim_SetValue(Track.TrackType, CurrentValue);
					}
					bResult = TRUE;
					break;
				}

				case EAT_Left:
				case EAT_Top:
				case EAT_Right:
				case EAT_Bottom:
				{
					if ( Target->Anim_GetValue(Track.TrackType, CurrentValue) )
					{
						CurrentValue.DestAsFloat = PerformInterpolation(CurrentFrame.InterpMode, CurrentValue.DestAsFloat, CurrentFrame.Data.DestAsFloat, InterpPercentage, CurrentFrame.InterpExponent);
						Target->Anim_SetValue(Track.TrackType, CurrentValue);
					}
					bResult = TRUE;
					break;
				}

				case EAT_PPBloom:
				{
					FPostProcessSettings* CurrentSettings=NULL;
					if ( Target->AnimGetCurrentPPSettings(CurrentSettings) )
					{
						const FLOAT NewBloomScale = PerformInterpolation(CurrentFrame.InterpMode, CurrentSettings->Bloom_Scale, CurrentFrame.Data.DestAsFloat, InterpPercentage, CurrentFrame.InterpExponent);
						CurrentSettings->Bloom_Scale = NewBloomScale;
						CurrentSettings->bEnableBloom = NewBloomScale > -DELTA;
						bResult = TRUE;
					}
					break;
				}

				case EAT_PPBlurSampleSize:
				{
					FPostProcessSettings* CurrentSettings=NULL;
					if ( Target->AnimGetCurrentPPSettings(CurrentSettings) )
					{
						const FLOAT NewBlurKernelSize = PerformInterpolation(CurrentFrame.InterpMode, CurrentSettings->DOF_BlurKernelSize, CurrentFrame.Data.DestAsFloat, InterpPercentage, CurrentFrame.InterpExponent);
						CurrentSettings->DOF_BlurKernelSize = NewBlurKernelSize;
						bResult = TRUE;
					}
					break;
				}

				case EAT_PPBlurAmount:
				{
					FPostProcessSettings* CurrentSettings=NULL;
					if ( Target->AnimGetCurrentPPSettings(CurrentSettings) )
					{
						const FLOAT NewBlurAmount = PerformInterpolation(CurrentFrame.InterpMode, CurrentSettings->DOF_MaxNearBlurAmount, CurrentFrame.Data.DestAsFloat, InterpPercentage, CurrentFrame.InterpExponent);
						CurrentSettings->DOF_MaxNearBlurAmount = NewBlurAmount;
						CurrentSettings->DOF_MaxFarBlurAmount = NewBlurAmount;
						bResult = TRUE;
					}
					break;
				}

				default:
				{
					// custom animation type
					if ( Target->Anim_GetValue(Track.TrackType, CurrentValue) )
					{
						CurrentValue.DestAsFloat = PerformInterpolation(CurrentFrame.InterpMode, CurrentValue.DestAsFloat, CurrentFrame.Data.DestAsFloat, InterpPercentage, CurrentFrame.InterpExponent);
						Target->Anim_SetValue(Track.TrackType, CurrentValue);
						bResult = TRUE;
					}
					break;
				}

				// TODO - Add suport for a notify track
			}
		}
	}

	return bResult;
}

/* ==========================================================================================================
	UUIAnimationSeq
========================================================================================================== */
/**
 * Wrapper for verifying whether the index is a valid index for the track's keyframes array.
 *
 * @param	TrackIndex	the index [into the Tracks array] for the track to check
 * @param	FrameIndex	the index [into the KeyFrames array of the track] for the keyframe to check
 *
 * @return	TRUE if the specified track contains a keyframe at the specified index.
 */
UBOOL UUIAnimationSeq::IsValidFrameIndex( INT TrackIndex, INT FrameIndex ) const
{
	UBOOL bResult = FALSE;

	if ( Tracks.IsValidIndex(TrackIndex) )
	{
		const FUIAnimTrack& Track = Tracks(TrackIndex);
		bResult = Track.KeyFrames.IsValidIndex(FrameIndex);
	}

	return bResult;
}

/**
 * Wrapper for getting the length of a specific frame in one of this animation sequence's tracks.
 *
 * @param	TrackIndex			the index [into the Tracks array] for the track to check
 * @param	FrameIndex			the index [into the KeyFrames array of the track] for the keyframe to check
 * @param	out_FrameLength		receives the remaining seconds for the frame specified
 *
 * @return	TRUE if the call succeeded; FALSE if an invalid track or frame index was specified.
 */
UBOOL UUIAnimationSeq::GetFrameLength( INT TrackIndex, INT FrameIndex, FLOAT& out_FrameLength ) const
{
	UBOOL bResult = FALSE;

	if ( IsValidFrameIndex(TrackIndex, FrameIndex) )
	{
		const FUIAnimTrack& Track = Tracks(TrackIndex);
		const FUIAnimationKeyFrame& Frame = Track.KeyFrames(FrameIndex);

		out_FrameLength = Frame.RemainingTime;
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Wrapper for getting the length of a specific track in this animation sequence.
 *
 * @param	TrackIndex	the index [into the Tracks array] for the track to check
 * @param	out_TrackLength		receives the remaining number of seconds for the track specified.
 *
 * @return	TRUE if the call succeeded; FALSE if an invalid track index was specified.
 */
UBOOL UUIAnimationSeq::GetTrackLength( INT TrackIndex, FLOAT& out_TrackLength ) const
{
	UBOOL bResult = FALSE;

	if ( Tracks.IsValidIndex(TrackIndex) )
	{
		out_TrackLength = 0.f;
		const FUIAnimTrack& Track = Tracks(TrackIndex);
		for ( INT FrameIndex = 0; FrameIndex < Track.KeyFrames.Num(); FrameIndex++ )
		{
			const FUIAnimationKeyFrame& Frame = Track.KeyFrames(FrameIndex);
			out_TrackLength += Frame.RemainingTime;
		}

		bResult = TRUE;
	}

	return bResult;
}

/**
 * Wrapper for getting the length of this animation sequence.
 *
 * @return	the total number of seconds in this animation sequence.
 */
FLOAT UUIAnimationSeq::GetSequenceLength() const
{
	FLOAT Result = 0.f;

	for ( INT TrackIndex = 0; TrackIndex < Tracks.Num(); TrackIndex++ )
	{
		FLOAT TrackLength = 0.f;
		if ( GetTrackLength(TrackIndex, TrackLength) )
		{
			Result += TrackLength;
		}
	}

	return Result;
}


/* ==========================================================================================================
	UUIScreenObject
========================================================================================================== */
/**
 * Find the index [into this widget's AnimStack array] for the animation sequence that has the specified name.
 *
 * @param	SequenceName	the name of the sequence to find.
 *
 * @return	the index of the sequence, or INDEX_NONE if it's not currently active.
 */
INT UUIScreenObject::FindAnimationSequenceIndex( FName AnimName ) const
{
	INT Result = INDEX_NONE;

	for ( INT SequenceIndex = 0; SequenceIndex < AnimStack.Num(); SequenceIndex++ )
	{
		const FUIAnimSequence& AnimSeq = AnimStack(SequenceIndex);
		if (AnimSeq.SequenceRef != NULL
		&&	AnimSeq.SequenceRef->SeqName == AnimName)
		{
			Result = SequenceIndex;
			break;
		}
	}

	return Result;
}

/**
 * Play an animation on this UIObject
 *
 * @param	AnimName			name of the animation sequence to activate; only necessary if no value is provided for AnimSeq
 * @param	AnimSeq				the animation sequence to activate for this widget; if specified, overrides the value of AnimName.
 * @param	OverrideLoopMode	if specified, overrides the animation sequence's default looping behavior
 * @param	PlaybackRate		if specified, affects how fast the animation will be executed.  1.0 is 100% speed.
 * @param	InitialPosition		if specified, indicates an alternate starting position (in seconds) for the animation sequence
 * @param	bSetAnimatingFlag	specify FALSE to prevent this function from marking this widget (and its parents) as bAnimating.
 */
void UUIScreenObject::PlayUIAnimation( FName AnimName, UUIAnimationSeq* AnimSeqTemplate/*=NULL*/, /*EUIAnimationLoopMode*/BYTE OverrideLoopMode/*=UIANIMLOOP_MAX*/, FLOAT PlaybackRate/*=1.f*/, FLOAT InitialPosition/*=0.f*/, UBOOL bSetAnimatingFlag/*=TRUE*/ )
{
	UGameUISceneClient* GameSceneClient = GetSceneClient();
	if ( GameSceneClient != NULL )
	{
		if ( AnimSeqTemplate == NULL )
		{
			AnimSeqTemplate = GameSceneClient->FindUIAnimation(AnimName);
			if ( AnimSeqTemplate == NULL )
			{
				debugf(NAME_DevUIAnimation, TEXT("Failed to find animation sequence with the AnimName: %s"), *AnimName.ToString());
			}
		}
		else if ( AnimName == NAME_None )
		{
			AnimName = AnimSeqTemplate->SeqName;
		}

		debugf(NAME_DevUIAnimation, TEXT("(%s) PLAYUIANIMATION: STARTING '%s' - OverrideLoopMode:%d   PlaybackRate:%.2f   InitialPosition:%.2f    bSetAnimatingFlag:%d   TimeStamp:%.3f"),
			*GetPathName(GetOutermost()), *AnimName.ToString(), OverrideLoopMode, PlaybackRate, InitialPosition, bSetAnimatingFlag, (FLOAT)appSeconds());

// 		const UBOOL bInitialPositionSpecified = !ARE_FLOATS_EQUAL(InitialPosition,0);

		// If we are already on the stack, just reset us
		INT SequenceIndex = FindAnimationSequenceIndex(AnimName);
		if ( SequenceIndex != INDEX_NONE )
		{
// 			if ( bInitialPositionSpecified && InitialPosition < -DELTA )
// 			{
// 				// if a negative initial position was specified, this indicates that the caller wishes to allow the animation to complete
// 				// if it's already active.
// 				return;
// 			}

			StopUIAnimation(AnimName, AnimSeqTemplate, FALSE);
		}

		if ( AnimSeqTemplate != NULL )
		{
			if ( AnimSeqTemplate->Tracks.Num() > 0 )
			{
				SequenceIndex = AnimStack.AddZeroed();
				FUIAnimSequence& AnimSeq = AnimStack(SequenceIndex);

				// initialize the widget's copy of the sequence from the animation sequence template
				AnimSeq.SequenceRef = AnimSeqTemplate;
				AnimSeq.AnimationTracks = AnimSeqTemplate->Tracks;
				AnimSeq.PlaybackRate = PlaybackRate;
				AnimSeq.LoopMode = AnimSeqTemplate->LoopMode;
				if ( OverrideLoopMode != UIANIMLOOP_MAX )
				{
					AnimSeq.LoopMode = OverrideLoopMode;
				}

//				FLOAT MaxTrackLength = 0.f;
				INT TrackTypeMask=0;
				for ( INT TrackIndex = 0; TrackIndex < AnimSeq.AnimationTracks.Num(); TrackIndex++ )
				{
					FUIAnimTrack& Track = AnimSeq.AnimationTracks(TrackIndex);

					//@todo - if we add support for InitialDelay, perhaps we shouldn't automatically send notification for all tracks
					// immediately.
					TrackTypeMask |= (1 << Track.TrackType);
// 					if ( bInitialPositionSpecified )
// 					{
						// @todo ronp implement:
						// if InitialPosition == -1, it means calculate the initial position of each track based on the widget's current
						// value for that track's data...

						// calculate the percentage of the total track time which represents the InitialPosition
// 						FLOAT TotalTrackTime = 0.f;
// 						for ( INT FrameIdx = 0; FrameIdx < Track.KeyFrames.Num(); FrameIdx++ )
// 						{
// 							FUIAnimationKeyFrame& KeyFrame = Track.KeyFrames(FrameIdx);
// 							TotalTrackTime += KeyFrame.RemainingTime;
// 						}
// 
// 						MaxTrackLength = Max(MaxTrackLength, TotalTrackTime);
// 					}
				}

				eventUIAnimationStarted(this, AnimName, 0,				bSetAnimatingFlag);
				eventUIAnimationStarted(this, AnimName, TrackTypeMask,	bSetAnimatingFlag);
				UpdateAnimation(0.f, AnimSeq);

// 				if ( bInitialPositionSpecified )
// 				{
// 					UpdateAnimation(InitialPosition, AnimSeq);
// 				}
			}
			else
			{
				debugf(NAME_DevUIAnimation, TEXT("(%s) PLAYUIANIMATION - specified sequence '%s' has no tracks."), *GetPathName(GetOutermost()), *AnimName.ToString());
			}
		}
		else
		{
			warnf(TEXT("(%s) PLAYUIANIMATION - failed to find animation sequence with name '%s'."), *GetPathName(GetOutermost()), *AnimName.ToString());
		}
	}
}

/**
 * Stop an animation that is playing.
 *
 * @param	AnimName	name of the animation sequence to stop; only necessary if no value is provided for AnimSeq
 * @param	AnimSeq		the animation sequence to deactivate for this widget; if specified, overrides the value of AnimName.
 * @param	bFinalize	indicates whether the widget should apply the final frame of the animation (i.e. simulate the animation completing)
 * @param	TypeMask	a bitmask representing the type of animation tracks to stop.  The bitmask should be generated by left shifting
 *						1 by the values of the EUIAnimType enum.
 */
void UUIScreenObject::StopUIAnimation( FName AnimName, UUIAnimationSeq* AnimSeq/*=NULL*/, UBOOL bFinalize/*=TRUE*/, INT TrackTypeMask/*=0*/ )
{
	UGameUISceneClient* GameSceneClient = GetSceneClient();
	if ( GameSceneClient != NULL )
	{
		if ( AnimSeq != NULL )
		{
			AnimName = AnimSeq->SeqName;
		}

		// If we are already on the stack, just reset us
		INT SequenceIndex = FindAnimationSequenceIndex(AnimName);
		if ( SequenceIndex == INDEX_NONE && AnimSeq != NULL )
		{
			SequenceIndex = FindAnimationSequenceIndex(AnimSeq->SeqName);
		}

		debugf(NAME_DevUIAnimation, TEXT("(%s) STOPUIANIMATION: STOPPING '%s'  (bFinalize:%i   TrackTypeMask:0x%08X   TimeStamp:%.3f)"), *GetPathName(GetOutermost()), *AnimName.ToString(), bFinalize, TrackTypeMask, (FLOAT)appSeconds());
		if ( SequenceIndex != INDEX_NONE )
		{
			ClearUIAnimationLoop(SequenceIndex, TrackTypeMask);

			INT EndedTrackTypeMask = 0;
			FUIAnimSequence& AnimSeq = AnimStack(SequenceIndex);
			for ( INT TrackIndex = 0; TrackIndex < AnimSeq.AnimationTracks.Num(); TrackIndex++ )
			{
				FUIAnimTrack& AnimTrack = AnimSeq.AnimationTracks(TrackIndex);
				UBOOL bCorrectType = (TrackTypeMask == 0 || (TrackTypeMask & (1 << AnimTrack.TrackType)) != 0);
				if ( bCorrectType && AnimTrack.KeyFrames.Num() )
				{
					BYTE TrackType = AnimTrack.TrackType;
					if ( bFinalize )
					{
						while ( AnimSeq.ApplyUIAnimation(this, TrackIndex, INDEX_NONE) )
						{
							checkSlow(AnimSeq.AnimationTracks.IsValidIndex(TrackIndex));

							FUIAnimTrack& Track = AnimSeq.AnimationTracks(TrackIndex);
							checkSlow(Track.KeyFrames.Num() > 0);

							eventActivateKeyFrameCompletedDelegates(this, AnimSeq.SequenceRef->SeqName, Track.TrackType);
							Track.KeyFrames.Remove(0);
						}
					}

					AnimSeq.AnimationTracks.Remove(TrackIndex--);

					// the user might have passed in 0 for TrackTypeMask, so we can't trust it....we also don't want to re-use it
					// here because if the value WAS 0 and we change it, the remaining tracks won't be removed.
					EndedTrackTypeMask = EndedTrackTypeMask | (1 << TrackType);
				}
			}

			if ( EndedTrackTypeMask != 0 )
			{
				eventUIAnimationEnded(this, AnimName, EndedTrackTypeMask);

				//@fixme - we need to make sure that the AnimStack isn't modified during this call!
				if ( AnimSeq.AnimationTracks.Num() == 0 )
				{
					AnimStack.Remove(SequenceIndex,1);
					eventUIAnimationEnded(this, AnimName, 0);
				}
			}
		}
	}
}

/**
 * Disables the looping for an animation, without affecting the animation itself.
 *
 * @param	SequenceIndex	the index of the sequence to clear the looping for; can be retrieved using FindAnimationSequenceIndex().
 * @param	TypeMask		a bitmask representing the type of animation tracks to affect.  The bitmask should be generated by left shifting
 *							1 by the values of the EUIAnimType enum.
 */
void UUIScreenObject::ClearUIAnimationLoop( INT SequenceIndex, INT TrackTypeMask )
{
	if ( SequenceIndex >= 0 && SequenceIndex < AnimStack.Num() )
	{
		for ( INT TrackIndex = 0; TrackIndex < AnimStack(SequenceIndex).AnimationTracks.Num(); TrackIndex++ )
		{
			FUIAnimTrack& AnimTrack = AnimStack(SequenceIndex).AnimationTracks(TrackIndex);
			if ( TrackTypeMask == 0
			||	(TrackTypeMask & (1 << AnimTrack.TrackType)) != 0 )
			{
				AnimTrack.LoopFrames.Empty();
			}
		}
	}
}

/**
 * Accessor for checking whether this widget is currently animating.
 *
 * @param	AnimationSequenceName	if specified, checks whether an animation sequence with this name is currently active.
 *
 * @return	TRUE if this widget is animating and if the named animation sequence is active.
 */
UBOOL UUIScreenObject::IsAnimating( FName AnimationSequenceName/*=NAME_None*/ )
{
	UBOOL bResult = FALSE;

	if ( AnimationSequenceName == NAME_None || FindAnimationSequenceIndex(AnimationSequenceName) != INDEX_NONE )
	{
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Changes the value of bAnimationPaused
 *
 * @param	bPauseAnimation		the new value for
 */
void UUIScreenObject::PauseAnimations( UBOOL bPauseAnimation )
{
	bAnimationPaused = bPauseAnimation;
}

/**
 * Accessor for checking whether animations are currently paused.
 *
 * @return	TRUE if animations are paused for this widget.
 */
UBOOL UUIScreenObject::IsAnimationPaused() const
{
	UBOOL bResult = FALSE;

	const UUIScene* OwnerScene = GetScene();
	if ( bAnimationPaused || (OwnerScene != NULL && OwnerScene->bAnimationPaused) )
	{
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Iterate over the AnimStack and tick each active sequence
 *
 * @Param DeltaTime			How much time since the last call
 */
void UUIScreenObject::TickAnimations( FLOAT DeltaTime )
{
	if ( !IsAnimationPaused() )
	{
		for ( INT SequenceIndex = 0; SequenceIndex < AnimStack.Num(); SequenceIndex++ )
		{
			//@fixme - what if the UIAnimEnd event messes with this widget's AnimStack? (i.e. adds or removes elements)
			FUIAnimSequence& Sequence = AnimStack(SequenceIndex);
			if ( UpdateAnimation(DeltaTime, Sequence) )
			{
				AnimStack.Remove(SequenceIndex--);
			}
		}
	}
}

/**
 * Applies animations to this widget's members, and decrements the animation sequence's counter.
 *
 * @param	DeltaTime		the time (in seconds) since the last frame began
 * @param	AnimSeqRef		the animation sequence to update.
 *
 * @return	TRUE if the animation is complete.
 */
UBOOL UUIScreenObject::UpdateAnimation( FLOAT DeltaTime, FUIAnimSequence& AnimSeqRef )
{
	UBOOL bResult = FALSE;

	UUIAnimationSeq* Seq = AnimSeqRef.SequenceRef;
	INT EndedTrackTypeMask = 0;
	for ( INT TrackIndex = 0; TrackIndex < AnimSeqRef.AnimationTracks.Num(); TrackIndex++ )
	{
		FUIAnimTrack& Track = AnimSeqRef.AnimationTracks(TrackIndex);
		if ( Track.KeyFrames.Num() > 0 )
		{
			FLOAT AdjustedDelta = DeltaTime * AnimationDebugMultiplier;

			// Normalize the DeltaTime by dividing out any applied TimeDilation;  this will keep our animations running at the correct
			// speed even when the game's running at non-default speed
			if ( GWorld != NULL && GIsGame )
			{
				AWorldInfo* WI = GWorld->GetWorldInfo();
				if ( WI != NULL && WI->TimeDilation != 0 )
				{
					AdjustedDelta /= WI->TimeDilation;
				}
			}

#if !FINAL_RELEASE && !SHIPPING_PC_GAME
			if ( ARE_FLOATS_EQUAL(AnimationDebugMultiplier,1.f) )
#endif
			{
				// unless we're debugging animation (multiplier is not 1), never allow the animations to be ticked at less than 10 FPS
				AdjustedDelta = Min(AdjustedDelta, 0.1f);
			}

// #if DEBUG_UI_ANIMATION
// 			debugf(NAME_DevUIAnimation, TEXT("-->  BEGIN LOOP"));
// #endif
			FLOAT RemainingDelta = AdjustedDelta;
			while ( RemainingDelta > 0 && Track.KeyFrames.Num() > 0 )
			{
				FUIAnimationKeyFrame& KeyFrame = Track.KeyFrames(0);
				RemainingDelta = AdjustedDelta - KeyFrame.RemainingTime;

// #if DEBUG_UI_ANIMATION
// 				debugf(NAME_DevUIAnimation, TEXT("    Delta:%f  AdjustedDelta:%f  RemainingDelta:%f   RemainingTime:%f   HalfDelta:%f"),
// 					DeltaTime, AdjustedDelta, RemainingDelta, KeyFrame.RemainingTime, AdjustedDelta * 0.5f);
// #endif

				// Try to indicate arrival as close as possible to the desired time...
				// If the remaining time is less than half of RenderDelta, trigger the OnArrival
				// during this tick, otherwise, wait until next tick (we'll assume RenderDelta
				// is a constant, even though this is not the case)
				const UBOOL bFrameActive = KeyFrame.RemainingTime >= AdjustedDelta * 0.5f;
				if ( bFrameActive )
				{
					AnimSeqRef.ApplyUIAnimation(this, TrackIndex, AdjustedDelta);
				}
				else
				{
					AnimSeqRef.ApplyUIAnimation(this, TrackIndex, INDEX_NONE);

					eventActivateKeyFrameCompletedDelegates(this, AnimSeqRef.SequenceRef->SeqName, Track.TrackType);
					Track.KeyFrames.Remove(0);
					if ( Track.KeyFrames.Num() == 0 )
					{

						switch ( AnimSeqRef.LoopMode )
						{
						case UIANIMLOOP_None:
							EndedTrackTypeMask |= (1 << Track.TrackType);
							AnimSeqRef.AnimationTracks.Remove(TrackIndex--);
							if ( AnimSeqRef.AnimationTracks.Num() == 0 )
							{
								bResult = TRUE;
							}
							break;

						case UIANIMLOOP_Continuous:
							Track.KeyFrames = Track.LoopFrames;
							break;

						case UIANIMLOOP_Bounce:
							for ( INT LoopIndex = Track.LoopFrames.Num() - 1, Start = 0; LoopIndex > Start; LoopIndex--, Start++ )
							{
								Track.LoopFrames.SwapItems(LoopIndex, Start);
							}
							for ( INT LoopIndex = 1; LoopIndex < Track.LoopFrames.Num(); LoopIndex++ )
							{
								Track.KeyFrames.AddItem(Track.LoopFrames(LoopIndex));
							}
							break;
						}
					}

					AdjustedDelta = RemainingDelta;
					break;
				}

				KeyFrame.RemainingTime -= AdjustedDelta;
				AdjustedDelta = RemainingDelta;
			}
		}
	}

	if ( EndedTrackTypeMask != 0 )
	{
		debugf(NAME_DevUIAnimation, TEXT("(%s) UUIScreenObject::UpdateAnimations: END ANIM TRACK '%s'  (TrackTypeMask:0x%08X  bResult:%i  Timestamp:%.3f)"),
			*GetPathName(GetOutermost()), *Seq->SeqName.ToString(), EndedTrackTypeMask, bResult, (FLOAT)appSeconds());

		eventUIAnimationEnded(this, Seq->SeqName, EndedTrackTypeMask);
		if ( bResult )
		{
			// send a notification that the entire animation sequence has finished (this is indicated by passing 0 for TrackTypeMask)
			eventUIAnimationEnded(this, Seq->SeqName, 0);
		}
	}

	return bResult;
}

/*
switch( AnimationType )
{
case EAT_Position:
	break;
case EAT_PositionOffset:
	break;
case EAT_RelPosition:
	break;
case EAT_Rotation:
	break;
case EAT_RelRotation:
	break;
case EAT_Color:
	break;
case EAT_Opacity:
	break;
case EAT_Visibility:
	break;
case EAT_Scale:
	break;
case EAT_Left:
	break;
case EAT_Top:
	break;
case EAT_Right:
	break;
case EAT_Bottom:
	break;
case EAT_PPBloom:
	break;
case EAT_PPBlurSampleSize:
	break;
case EAT_PPBlurAmount:
	break;
default:
	break;
}
*/

/**
 * Retrieves the current value for some data currently being interpolated by this widget.
 *
 * @param	AnimationType		the type of animation data to retrieve
 * @param	out_CurrentValue	receives the current data value; animation type determines which of the fields holds the actual data value.
 *
 * @return	TRUE if the widget supports the animation type specified.
 */
UBOOL UUIScreenObject::Anim_GetValue( /*EUIAnimType*/BYTE AnimationType, FUIAnimationRawData& out_CurrentValue ) const
{
	UBOOL bResult = FALSE;

	switch( AnimationType )
	{
	case EAT_Position:
		out_CurrentValue.DestAsVector.X = GetPosition(UIFACE_Left, EVALPOS_PixelViewport);
		out_CurrentValue.DestAsVector.Y = GetPosition(UIFACE_Top, EVALPOS_PixelViewport);
		bResult = TRUE;
		break;

	case EAT_RelPosition:
		out_CurrentValue.DestAsVector.X = GetPosition(UIFACE_Left, EVALPOS_PercentageOwner);
		out_CurrentValue.DestAsVector.Y = GetPosition(UIFACE_Top, EVALPOS_PercentageOwner);
		bResult = TRUE;
		break;

	case EAT_Opacity:
		out_CurrentValue.DestAsFloat = Opacity;
		bResult = TRUE;
		break;

	case EAT_Visibility:
		out_CurrentValue.DestAsFloat = IsVisible() ? 1.f : 0.f;
		bResult = TRUE;
		break;

	case EAT_Left:
		out_CurrentValue.DestAsFloat = GetPosition(UIFACE_Left, EVALPOS_PixelViewport);
		bResult = TRUE;
		break;

	case EAT_Top:
		out_CurrentValue.DestAsFloat = GetPosition(UIFACE_Top, EVALPOS_PixelViewport);
		bResult = TRUE;
		break;

	case EAT_Right:
		out_CurrentValue.DestAsFloat = GetPosition(UIFACE_Right, EVALPOS_PixelViewport);
		bResult = TRUE;
		break;

	case EAT_Bottom:
		out_CurrentValue.DestAsFloat = GetPosition(UIFACE_Bottom, EVALPOS_PixelViewport);
		bResult = TRUE;
		break;

		// types not supported by UIScreenObject
	case EAT_PositionOffset:
	case EAT_Rotation:
	case EAT_RelRotation:
	case EAT_Color:
	case EAT_Scale:
	case EAT_PPBloom:
	case EAT_PPBlurSampleSize:
	case EAT_PPBlurAmount:
		break;

	default:
		debugf(NAME_DevUIAnimation, TEXT("UUIScreenObject::Anim_SetValue - Unsupported animation type: %d"), AnimationType);
		break;
	}

	return bResult;
}

/**
 * Updates the current value for some data currently being interpolated by this widget.
 *
 * @param	AnimationType		the type of animation data to set
 * @param	out_CurrentValue	contains the updated data value; animation type determines which of the fields holds the actual data value.
 *
 * @return	TRUE if the widget supports the animation type specified.
 */
UBOOL UUIScreenObject::Anim_SetValue( /*EUIAnimType*/BYTE AnimationType, const FUIAnimationRawData& NewValue )
{
	UBOOL bResult = FALSE;

	switch( AnimationType )
	{
	case EAT_Position:
		SetPosition(NewValue.DestAsVector.X, UIFACE_Left, EVALPOS_PixelViewport);
		SetPosition(NewValue.DestAsVector.Y, UIFACE_Top, EVALPOS_PixelViewport);
		bResult = TRUE;
		break;

	case EAT_RelPosition:
		SetPosition(NewValue.DestAsVector.X, UIFACE_Left, EVALPOS_PercentageOwner);
		SetPosition(NewValue.DestAsVector.Y, UIFACE_Top, EVALPOS_PercentageOwner);
		bResult = TRUE;
		break;


	case EAT_Opacity:
		Opacity = NewValue.DestAsFloat;
		bResult = TRUE;
		break;

	case EAT_Visibility:
		eventSetVisibility(NewValue.DestAsFloat > DELTA);
		bResult = TRUE;
		break;

	case EAT_Left:
		SetPosition(NewValue.DestAsFloat, UIFACE_Left,EVALPOS_PixelViewport);
		bResult = TRUE;
		break;

	case EAT_Top:
		SetPosition(NewValue.DestAsFloat, UIFACE_Top,EVALPOS_PixelViewport);
		bResult = TRUE;
		break;

	case EAT_Right:
		SetPosition(NewValue.DestAsFloat, UIFACE_Right,EVALPOS_PixelViewport);
		bResult = TRUE;
		break;

	case EAT_Bottom:
		SetPosition(NewValue.DestAsFloat, UIFACE_Bottom,EVALPOS_PixelViewport);
		bResult = TRUE;
		break;

	// types not supported by UIScreenObject
	case EAT_PositionOffset:
	case EAT_Rotation:
	case EAT_RelRotation:
	case EAT_Color:
	case EAT_Scale:
	case EAT_PPBloom:
	case EAT_PPBlurSampleSize:
	case EAT_PPBlurAmount:
		break;

	default:
		debugf(NAME_DevUIAnimation, TEXT("UUIScreenObject::Anim_SetValue - Unsupported animation type: %d"), AnimationType);
		break;
	}

	return bResult;
}

/* ==========================================================================================================
	UUIObject
========================================================================================================== */
/**
 * Retrieves the current value for some data currently being interpolated by this widget.
 *
 * @param	AnimationType		the type of animation data to retrieve
 * @param	out_CurrentValue	receives the current data value; animation type determines which of the fields holds the actual data value.
 *
 * @return	TRUE if the widget supports the animation type specified.
 */
UBOOL UUIObject::Anim_GetValue( /*EUIAnimType*/BYTE AnimationType, FUIAnimationRawData& out_CurrentValue ) const
{
	UBOOL bResult = FALSE;

	switch( AnimationType )
	{
	case EAT_Rotation:
		out_CurrentValue.DestAsRotator = Rotation.Rotation;
		bResult = TRUE;
		break;
	}

	return bResult || Super::Anim_GetValue(AnimationType, out_CurrentValue);
}

/**
 * Updates the current value for some data currently being interpolated by this widget.
 *
 * @param	AnimationType		the type of animation data to set
 * @param	out_CurrentValue	contains the updated data value; animation type determines which of the fields holds the actual data value.
 *
 * @return	TRUE if the widget supports the animation type specified.
 */
UBOOL UUIObject::Anim_SetValue( /*EUIAnimType*/BYTE AnimationType, const FUIAnimationRawData& NewValue )
{
	UBOOL bResult = FALSE;

	switch( AnimationType )
	{
	case EAT_Rotation:
		RotateWidget(NewValue.DestAsRotator);
		bResult = TRUE;
		break;
	}

	return bResult || Super::Anim_SetValue(AnimationType, NewValue);
}

