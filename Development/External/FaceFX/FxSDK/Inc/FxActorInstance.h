//------------------------------------------------------------------------------
// This class implements a FaceFx actor instance.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxActorInstance_H__
#define FxActorInstance_H__

#include "FxPlatform.h"
#include "FxArchive.h"
#include "FxArray.h"
#include "FxNamedObject.h"
#include "FxActor.h"
#include "FxFaceGraphNode.h"
#include "FxDeltaNode.h"

namespace OC3Ent
{

namespace Face
{

/// The various states of animation playback.
enum FxAnimPlaybackState
{
	APS_StartAudio,	 ///< The audio should be started at this time.
	APS_Playing,	 ///< The animation is playing.
	APS_Stopped      ///< The animation has finished.
};

/// A FaceFX actor instance.
/// \note Due to the way the FaceFX actor instancing system works, an actor
/// instance should always be destroyed before it's actor.
class FxActorInstance : public FxNamedObject
{
	/// Declare the class.
	FX_DECLARE_CLASS_NO_DYNCREATE_NO_SERIALIZE(FxActorInstance, FxNamedObject)
public:
	/// Construct the instance and optionally set the actor.
	FxActorInstance( FxActor* actor = NULL );
	/// Destructor.
	virtual ~FxActorInstance();

	/// Sets the actor this instance refers to.
	void SetActor( FxActor* pActor );
	/// Returns the actor this instance refers to.
	/// \note Internal use only.
	FX_INLINE FxActor* GetActor( void ) { return _pActor; }

	/// Returns FxTrue if the actor instance is currently open in FaceFX Studio.
	FX_INLINE FxBool IsOpenInStudio( void ) const { return _isOpenInStudio; }
	/// \note For internal use only.  This is called with FxTrue as the argument
	/// when the actor instance is opened in FaceFX Studio.
	void SetIsOpenInStudio( FxBool isOpenInStudio );

	/// Sets whether or not Tick() does anything if there is no currently 
	/// playing animation.  This is useful for procedural %Face Graph effects,
	/// especially those based on time.
	/// \param allowNonAnimTick Set to \p FxTrue to allow ticks regardless of the state
	/// of the animation playback, \p FxFalse to only tick when there is an animation
	/// being played.
	void SetAllowNonAnimTick( FxBool allowNonAnimTick );
	/// Returns whether or not Tick() does anything if there is no currently 
	/// playing animation.  This is useful for procedural %Face Graph effects,
	/// especially those based on time.
	FX_INLINE FxBool GetAllowNonAnimTick( void ) const { return _allowNonAnimTick; }

	/// Plays the animation 'animName' in the animation group 'groupName'.
	/// \param animName The name of the animation to play.
	/// \param groupName The name of the group in which the animation resides.
	/// \param ignoreNegativeTime If \p FxTrue, the player will skip the part of
	/// the animation before 0.0s.  This is only useful in certain circumstances,
	/// where the animation begins almost immediately at the start of the audio,
	/// and the ramp-in is undesired because of synching issues with in-game
	/// events.  It defaults to \p FxFalse, meaning the negative time will be 
	/// treated normally, and played through before sending the signal to start
	/// the audio.
	/// \return \p FxTrue if the animation started playing, \p FxFalse otherwise.
	FxBool PlayAnim( const FxName& animName, const FxName& groupName = FxAnimGroup::Default,
		             FxBool ignoreNegativeTime = FxFalse );
	/// Stops playing an animation.
	void StopAnim( void );

	/// Returns the state of animation playback.
	FX_INLINE FxAnimPlaybackState GetAnimState( void ) const { return _animPlaybackInfo.state; }
	/// Returns \p FxTrue if the there is an animation currently playing, \p FxFalse
	/// otherwise.
	FX_INLINE FxBool IsPlayingAnim( void ) const { return APS_Stopped != _animPlaybackInfo.state; }
	/// Returns the current time in the animation playback.  If no animation is
	/// currently playing, -1.0f is returned.
	FX_INLINE FxReal GetCurrentAnimTime( void ) const { return _animPlaybackInfo.currentTime; }
	/// Returns the name of the animation group that contains the currently
	/// playing animation or FxName::NullName if nothing is playing.
	FX_INLINE const FxName& GetCurrentAnimGroupName( void ) const { return _animPlaybackInfo.currentAnimGroupName; }
	/// Returns the name of the currently playing animation or FxName::NulName
	/// if nothing is playing.
	FX_INLINE const FxName& GetCurrentAnimName( void ) const { return _animPlaybackInfo.currentAnimName; }
	/// Returns a pointer to the currently playing animation or \p NULL if none.
	FX_INLINE const FxAnim* GetCurrentAnim( void ) const { return _animPlaybackInfo.pAnim; }

	/// Begins a frame for this actor instance.
	/// This should only be called after FxActorInstance::Tick().  No per-frame work involving 
	/// the %Face Graph, bone blending, etc should take place outside of the 
	/// FxActorInstance::BeginFrame() / FxActorInstance::EndFrame() pair.
	virtual void BeginFrame( void );
	/// Ends a frame for this actor instance.
	/// This should only be called after FxActorInstance::Tick() and 
	/// FxActorInstance::BeginFrame().  No per-frame work involving 
	/// the %Face Graph, bone blending, etc should take place outside of the 
	/// FxActorInstance::BeginFrame() / FxActorInstance::EndFrame() pair.
	virtual void EndFrame( void );

	/// Returns the number of bones controlled by the actor instance.
	FxSize GetNumBones( void ) const;
	/// Returns the bone's index in the client animation system or FX_INT32_MAX
	/// if the bone's index in the client animation system was not set.
	/// \param index The index of the bone in the FxMasterBoneList.
	FxInt32 GetBoneClientIndex( FxSize index ) const;
	/// Returns the bone at index.
	/// \param index The index of the bone to blend in the FxMasterBoneList.
	/// \param bonePos The bone transformation's position component.
	/// \param boneRot The bone transformation's rotation component.
	/// \param boneScale The bone transformation's scale component.
	/// \param boneWeight The current weight of the bone.
	/// \note This call returns valid results only after Tick() and BeginFrame()
	/// and before EndFrame().
	void GetBone( FxSize index, FxVec3& bonePos, FxQuat& boneRot, 
		          FxVec3& boneScale, FxReal& boneWeight ) const;

	/// Ticks the actor instance.
	/// \param appTime The time (in seconds) as reckoned by the entire 
	/// application (should be ever increasing).
	/// \param audioTime The current playback time in the audio (in seconds) to 
	/// which the animation is synchronized.
	/// If \a audioTime is less than zero while the animation is in the 
	/// \p APS_Playing state, the animation is assumed to have no corresponding 
	/// audio, and thus \a appTime is used to calculate the offset into 
	/// the animation.  Tick() should be called each frame before the 
	/// FxActorInstance::BeginFrame() and FxActorInstance::EndFrame() pair.
	/// \return The current state of the animation playback.  This value should be
	/// monitored each frame for equality to \p APS_StartAudio, which indicates that
	/// the audio corresponding to the animation should be started.
	FxAnimPlaybackState Tick( const FxDReal appTime, const FxReal audioTime );
	/// Forces a tick on the actor instance at the specified time in the
	/// animation.  ForceTick() is intended for tool usage, when you know
	/// exactly what frame to display and do not need the additional services
	/// provided by Tick().  Note that appTime is \b not used here and therefore
	/// current time nodes in the %Face Graph will \b not be updated when 
	/// calling ForceTick().
	/// \param animName The name of the animation to force tick.
	/// \param groupName The group containing the animation to force tick.
	/// \param forcedAnimationTime The time (in seconds) with which to force
	/// tick the animation.  This time should be between the animation's start
	/// and end times.
	/// ForceTick() should be called each frame before the 
	/// FxActorInstance::BeginFrame() and FxActorInstance::EndFrame() pair.
	void ForceTick( const FxName& animName, const FxName& groupName, 
		            const FxReal forcedAnimationTime );
		
	/// Sets a register to a specific value.
	/// The operation specified by \a regOp will
	/// be used to put the register value into the %Face Graph.
	/// \param regName The name of the register.  This must be the name of a %Face Graph node.
	/// \param newValue The desired value of the register.
	/// \param regOp The operation to perform with \a newValue and the value of the node.
	/// \param interpDuration The time (in seconds) of the interpolation to \a newValue.
	/// \return \p FxTrue if the operation succeeded, or \p FxFalse if the register was not
	/// found.
	/// \note SetRegister() must be called before Tick().
	/// If \a interpDuration is 0.0f, the register value is immediately set to \a newValue.
	/// Otherwise, Hermite interpolation is performed from the current value to \a newValue
	/// over \a interpDuration seconds.
	FxBool SetRegister( const FxName& regName, FxReal newValue, 
						FxRegisterOp regOp, FxReal interpDuration = 0.0f );
	/// Sets a register to a specific value.
	/// The operations specified by \a firstRegOp and \a nextRegOp will
	/// be used to put the register value into the %Face Graph.
	/// Registers are updated during BeginFrame().
	/// \param regName The name of the register.  This must be the name of a %Face Graph node.
	/// \param firstRegOp The first operation to perform with \a firstValue and the value of the node.
	/// \param firstValue The desired first value of the register.
	/// \param firstInterpDuration The time (in seconds) of the interpolation to \a firstValue.
	/// \param nextRegOp The next operation to perform with \a nextValue and the value of the node.
	/// \param nextValue The desired next value of the register.
	/// \param nextInterpDuration The time (in seconds) of the interpolation to \a nextValue.
	/// \return \p FxTrue if the operation succeeded, or \p FxFalse if the register was not
	/// found.
	/// \note SetRegister() must be called before Tick().
	/// If \a firstInterpDuration is 0.0f, the register value is immediately set to \a firstValue.
	/// Otherwise, Hermite interpolation is performed from the current value to \a firstValue
	/// over \a firstInterpDuration seconds after which Hermite interpolation is performed from
	/// that value to \a nextValue over \a nextInterpDuration seconds.
	FxBool SetRegisterEx( const FxName& regName, FxRegisterOp firstRegOp, 
		                  FxReal firstValue, FxReal firstInterpDuration, 
						  FxRegisterOp nextRegOp, FxReal nextValue, 
						  FxReal nextInterpDuration );
	/// Sets all registers to a specific value.
	/// The operations specified by \a firstRegOp and \a nextRegOp will be used to put all register 
	/// values into the %Face Graph.
	/// Registers are updated during BeginFrame().
	/// \param firstRegOp The first operation to perform with \a firstValue and the value of the node.
	/// \param firstValue The desired first value of the register.
	/// \param firstInterpDuration The time (in seconds) of the interpolation to \a firstValue.
	/// \param nextRegOp The next operation to perform with \a nextValue and the value of the node.
	/// \param nextValue The desired next value of the register.
	/// \param nextInterpDuration The time (in seconds) of the interpolation to \a nextValue.
	/// \note SetAllRegistiers() must be called before Tick().
	/// If \a interpDuration is 0.0f, the register values are immediately set to \a newValue.
	/// Otherwise, Hermite interpolation is performed from the current values to \a newValue
	/// over \a interpDuration seconds.
	void SetAllRegisters( FxRegisterOp firstRegOp, FxReal firstValue, FxReal firstInterpDuration = 0.0f,
		                  FxRegisterOp nextRegOp = RO_Invalid, FxReal nextValue = 0.0f, 
						  FxReal nextInterpDuration = 0.0f );
	/// Returns the current value of a register.
	/// \param regName The name of the register. This should be the name of a 
	///   %Face Graph node.
	/// \return The current value of the register, or 0.0f if the register was 
	///   not found.
	FxReal GetRegister( const FxName& regName );
	/// Gets the current state of the specified register.
	/// \param regName The name of the register.  This should be the name of a
	///  %Face Graph node.
	/// \param regState The state of the specified register.
	/// \return FxTrue if the register was found (in which case regState is valid)
	///  or FxFalse if the register was not found (in which case regState is not
	///  valid).
	FxBool GetRegisterState( const FxName& regName, FxRegister& regState );

	/// Returns the requested value of the register. 
	/// \param regName The name of the register. This should be the name of a 
	///   %Face Graph node.
	/// \return The requested value of the register, or 0.0f if the register was 
	///   not found.
	FxReal GetOriginalRegisterValue( const FxName& regName );

protected:
	/// An animation player.
	/// Each FxActorInstance has its own animation player.
	struct FxAnimPlaybackInfo
	{
		/// Constructor.
		FxAnimPlaybackInfo()
			: pAnim(NULL)
			, startTime(FxInvalidValue)
			, currentTime(FxInvalidValue)
			, leadinDuration(FxInvalidValue)
			, hasStartedAudio(FxFalse)
			, ignoreNegativeTime(FxFalse)
			, state(APS_Stopped) {}
		/// The name of the animation group that contains the currently playing
		/// animation or FxName::NullName if nothing is playing.
		FxName currentAnimGroupName;
		/// The name of the currently playing animation or FxName::NullName if 
		/// nothing is playing.
		FxName currentAnimName;
		/// The animation loaded in the player.
		FxAnim* pAnim;
		/// The start time of the animation in app time.
		/// (Used for soundless animations)
		FxDReal startTime;
		/// The current time of the animation player.
		FxReal currentTime;
		/// The lead-in duration of the animation
		FxReal leadinDuration;
		/// Whether or not an audio start has been triggered for this animation.
		FxBool hasStartedAudio;
		/// Whether to ignore the negative time in an animation.
		FxBool ignoreNegativeTime;
		/// The current state of this instance.
		FxAnimPlaybackState state;
	};

	/// The actor this instance refers to.
	FxActor* _pActor;

	/// This is FxTrue if the actor instance is currently open in FaceFX Studio.
	FxBool _isOpenInStudio;
	
	/// Determines whether or not calling Tick() does anything if there is no
	/// currently playing animation.  This is useful for procedural %Face Graph 
	/// effects, especially those based on time.  Defaults to \p FxFalse.
	FxBool _allowNonAnimTick;

	/// The animation playback information.
	FxAnimPlaybackInfo _animPlaybackInfo;

	/// Registers.
	FxArray<FxRegister> _registers;
	/// Whether or not the instance has been ticked yet.
	FxBool _hasBeenTicked;
};

FX_INLINE FxSize FxActorInstance::GetNumBones( void ) const
{
	FxAssert(_pActor);
	return _pActor->GetMasterBoneList().GetNumRefBones();
}

FX_INLINE FxInt32 FxActorInstance::GetBoneClientIndex( FxSize index ) const
{
	FxAssert(_pActor);
	return _pActor->GetMasterBoneList().GetRefBoneClientIndex(index);
}

FX_INLINE void FxActorInstance::GetBone( FxSize index, FxVec3& bonePos, 
							             FxQuat& boneRot, FxVec3& boneScale, 
							             FxReal& boneWeight ) const
{
	FxAssert(_pActor);
	return _pActor->GetMasterBoneList().GetBlendedBone(index, _pActor->GetCompiledFaceGraph(), bonePos, boneRot, boneScale, boneWeight);
}

FX_INLINE FxBool FxActorInstance::SetRegister( const FxName& regName, 
									           FxReal newValue, FxRegisterOp regOp, 
									           FxReal interpDuration )
{
	return SetRegisterEx(regName, regOp, newValue, interpDuration, RO_Invalid, 0.0f, 0.0f);
}

} // namespace Face

} // namespace OC3Ent

#endif
