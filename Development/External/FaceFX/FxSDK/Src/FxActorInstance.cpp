//------------------------------------------------------------------------------
// This class implements a FaceFx actor instance.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxActorInstance.h"
#include "FxUtil.h"
#include "FxMath.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxActorInstanceVersion 0

FX_IMPLEMENT_CLASS(FxActorInstance, kCurrentFxActorInstanceVersion, FxNamedObject)

FxActorInstance::FxActorInstance( FxActor* actor )
	: _pActor(NULL)
	, _isOpenInStudio(FxFalse)
	, _allowNonAnimTick(FxFalse)
	, _hasBeenTicked(FxFalse)
{
	SetActor(actor);
}

FxActorInstance::~FxActorInstance()
{
	// Remove this instance from the actor's instance list.  _pActor is 
	// explicitly checked here (instead of an assert) just in case the actor was
	// destroyed before its instances.  If it was, it would have called 
	// SetActor(NULL) on all of them.
	if( _pActor )
	{
		_pActor->RemoveInstance(this);
	}
}

void FxActorInstance::SetActor( FxActor* pActor )
{
	FxBool isSameActor = FxFalse;
	if( _pActor && pActor && _pActor == pActor )
	{
		isSameActor = FxTrue;
	}
	if( _pActor )
	{
		if( !isSameActor )
		{
			_pActor->RemoveInstance(this);
		}
		_registers.Clear();
		_pActor = NULL;
	}
	_pActor = pActor;
	if( _pActor )
	{
		// Add this instance to the instance list of the actor.
		if( !isSameActor )
		{
			_pActor->AddInstance(this);
		}
		// Add all the registers.
		FxCompiledFaceGraph& cg = _pActor->GetCompiledFaceGraph();
		FxSize numNodes = cg.nodes.Length();
		_registers.Reserve(numNodes);
		for( FxSize i = 0; i < numNodes; ++i )
		{
			_registers.PushBack(FxRegister());
		}
	}
}

void FxActorInstance::SetIsOpenInStudio( FxBool isOpenInStudio )
{
	_isOpenInStudio = isOpenInStudio;
}

void FxActorInstance::SetAllowNonAnimTick( FxBool allowNonAnimTick )
{
	_allowNonAnimTick = allowNonAnimTick;
}

FxBool FxActorInstance
::PlayAnim( const FxName& animName, const FxName& groupName, FxBool ignoreNegativeTime )
{
	FxAssert(_pActor);
	StopAnim();
	_animPlaybackInfo.pAnim = _pActor->GetAnimPtr(groupName, animName);
	if( _animPlaybackInfo.pAnim )
	{
		_animPlaybackInfo.currentAnimGroupName = groupName;
		_animPlaybackInfo.currentAnimName      = animName;
		_animPlaybackInfo.ignoreNegativeTime   = ignoreNegativeTime;
		_animPlaybackInfo.state                = APS_Playing;
		return FxTrue;
	}
	return FxFalse;
}

void FxActorInstance::StopAnim( void )
{
	_animPlaybackInfo.currentAnimGroupName = FxName::NullName;
	_animPlaybackInfo.currentAnimName      = FxName::NullName;
	_animPlaybackInfo.pAnim                = NULL;
	_animPlaybackInfo.startTime            = FxInvalidValue;
	_animPlaybackInfo.currentTime          = FxInvalidValue;
	_animPlaybackInfo.leadinDuration       = FxInvalidValue;
	_animPlaybackInfo.hasStartedAudio      = FxFalse;
	_animPlaybackInfo.ignoreNegativeTime   = FxFalse;
	_animPlaybackInfo.state	               = APS_Stopped;
}

void FxActorInstance::BeginFrame( void )
{	
	// Nothing to do here as of version 1.7, but it is reserved for future use.
}

void FxActorInstance::EndFrame( void )
{
	// Nothing to do here as of version 1.7, but it is reserved for future use.
}

FxAnimPlaybackState 
FxActorInstance::Tick( const FxDReal appTime, const FxReal audioTime )
{
	FxAssert(_pActor);
	FxCompiledFaceGraph& cg = _pActor->GetCompiledFaceGraph();
	FxAnimPlaybackState currentState = APS_Stopped;
	if( APS_Stopped != _animPlaybackInfo.state )
	{
		if( _animPlaybackInfo.pAnim )
		{
			// Reset the reference bone weights.
			FxMasterBoneList& masterBoneList = _pActor->GetMasterBoneList();
			masterBoneList.ResetRefBoneWeights();
			// Set the reference bone weights for the current animation.
			FxSize numBoneWeights = _animPlaybackInfo.pAnim->GetNumBoneWeights();
			for( FxSize i = 0; i < numBoneWeights; ++i )
			{
				const FxAnimBoneWeight& boneWeight = _animPlaybackInfo.pAnim->GetBoneWeight(i);
				masterBoneList.SetRefBoneCurrentWeight(boneWeight.boneName, boneWeight.boneWeight);
			}

			if( APS_Playing == _animPlaybackInfo.state )
			{
				// Start the animation if it hasn't started yet.
				if( FxDRealEquality(_animPlaybackInfo.startTime, static_cast<FxDReal>(FxInvalidValue)) )
				{
					// Check to see if there is negative time to play through.
					FxReal animStartTime = _animPlaybackInfo.pAnim->GetStartTime();
					if( animStartTime < 0.0f && !_animPlaybackInfo.ignoreNegativeTime )
					{
						_animPlaybackInfo.leadinDuration = FxAbs(animStartTime);
					}
					else
					{
						_animPlaybackInfo.leadinDuration = 0.0f;
					}
					_animPlaybackInfo.startTime = appTime;
				}

				// See if there is audio playing (if not, audioTime should be less than 0.0)
				// and update currentTime accordingly.
				if( audioTime < 0.0f )
				{
					_animPlaybackInfo.currentTime = static_cast<FxReal>(appTime - _animPlaybackInfo.startTime) - _animPlaybackInfo.leadinDuration;
				}
				else
				{
					_animPlaybackInfo.currentTime = audioTime;
					// Keep appTime and audioTime from diverging.  This prevents situations
					// where appTime is slightly behind audioTime, so when the audio playback ends,
					// we don't go backwards in time when picking up at appTime where audioTime 
					// left off.
					_animPlaybackInfo.startTime = appTime - audioTime - _animPlaybackInfo.leadinDuration;
				}

				// Evaluate the curves.
				FxSize numCurves = _animPlaybackInfo.pAnim->GetNumAnimCurves();
				for( FxSize i = 0; i < numCurves; ++i )
				{
					const FxAnimCurve& curve = _animPlaybackInfo.pAnim->GetAnimCurve(i);
					FxSize nodeIndex = cg.FindNodeIndex(curve.GetName());
					if( FxInvalidIndex != nodeIndex )
					{
						cg.nodes[nodeIndex].trackValue = curve.EvaluateAt(_animPlaybackInfo.currentTime);
					}
				}
			}
			else if( APS_Stopped == _animPlaybackInfo.state )
			{
				// Nothing to do here for now.
			}
			else
			{
				FxAssert(!"Unknown state in FxAnimPlayer::Tick()!");
			}
			
			// Tick the Face Graph.
			cg.Tick(static_cast<FxReal>(appTime), _registers, _hasBeenTicked);

			// Check if the animation is finished.
			if( APS_Playing == _animPlaybackInfo.state && _animPlaybackInfo.currentTime >= _animPlaybackInfo.pAnim->GetEndTime() )
			{
				// The animation has stopped.
				StopAnim();
			}

			// Check if we should start the audio.
			if( APS_Playing == _animPlaybackInfo.state && !_animPlaybackInfo.hasStartedAudio && _animPlaybackInfo.currentTime >= 0.0f )
			{
				_animPlaybackInfo.hasStartedAudio = FxTrue;
				return APS_StartAudio;
			}
		}
		currentState = _animPlaybackInfo.state;
	}
	else if( _allowNonAnimTick )
	{
		cg.Tick(static_cast<FxReal>(appTime), _registers, _hasBeenTicked);
		currentState = APS_Playing;
	}
	else if( _isOpenInStudio )
	{
		cg.Tick(static_cast<FxReal>(appTime), _registers, _hasBeenTicked);
	}
	return currentState;
}

void FxActorInstance::ForceTick( const FxName& animName, const FxName& groupName, 
							     const FxReal forcedAnimationTime )
{
	// Need to set the animation as playing to make it valid.
	PlayAnim(animName, groupName);
	// Turn off all of the registers.
	SetAllRegisters(RO_None, 0.0f);
	// Force tick it.
	if( _animPlaybackInfo.pAnim )
	{
		FxAssert(_pActor);
		// Reset the reference bone weights.
		FxMasterBoneList& masterBoneList = _pActor->GetMasterBoneList();
		masterBoneList.ResetRefBoneWeights();
		// Set the reference bone weights for the current animation.
		FxSize numBoneWeights = _animPlaybackInfo.pAnim->GetNumBoneWeights();
		for( FxSize i = 0; i < numBoneWeights; ++i )
		{
			const FxAnimBoneWeight& boneWeight = _animPlaybackInfo.pAnim->GetBoneWeight(i);
			masterBoneList.SetRefBoneCurrentWeight(boneWeight.boneName, boneWeight.boneWeight);
		}
		// Evaluate the curves.
		FxCompiledFaceGraph& cg = _pActor->GetCompiledFaceGraph();
		FxSize numCurves = _animPlaybackInfo.pAnim->GetNumAnimCurves();
		for( FxSize i = 0; i < numCurves; ++i )
		{
			const FxAnimCurve& curve = _animPlaybackInfo.pAnim->GetAnimCurve(i);
			FxSize nodeIndex = cg.FindNodeIndex(curve.GetName());
			if( FxInvalidIndex != nodeIndex )
			{
				cg.nodes[nodeIndex].trackValue = curve.EvaluateAt(forcedAnimationTime);
			}
		}
		cg.Tick(forcedAnimationTime, _registers, _hasBeenTicked);
	}
	// Stop it to clear it out.
	StopAnim();
}

FxBool 
FxActorInstance::SetRegisterEx( const FxName& regName, FxRegisterOp firstRegOp, 
							    FxReal firstValue, FxReal firstInterpDuration, 
								FxRegisterOp nextRegOp, FxReal nextValue, 
								FxReal nextInterpDuration )
{
	FxAssert(_pActor);
	FxCompiledFaceGraph& cg = _pActor->GetCompiledFaceGraph();
	FxSize regIndex = cg.FindNodeIndex(regName);
	if( FxInvalidIndex != regIndex )
	{
		_registers[regIndex].interpStartValue   = (RO_LoadAdd      == firstRegOp || 
			                                       RO_LoadMultiply == firstRegOp || 
												   RO_LoadReplace  == firstRegOp) 
			                                      ? _registers[regIndex].value 
												  : _registers[regIndex].interpLastValue;
		_registers[regIndex].interpEndValue            = firstValue;
		_registers[regIndex].interpNextEndValue        = nextValue;
#ifdef FX_XBOX_360
		_registers[regIndex].interpInverseDuration     = static_cast<FxReal>(__fsel(firstInterpDuration, 
			                                             __fsel(-firstInterpDuration, 0.0f, 1.0f / firstInterpDuration), 
														 1.0f / firstInterpDuration));
		_registers[regIndex].interpNextInverseDuration = static_cast<FxReal>(__fsel(nextInterpDuration, 
														 __fsel(-nextInterpDuration, 0.0f, 1.0f / nextInterpDuration), 
													     1.0f / firstInterpDuration));
#else // !FX_XBOX_360
		_registers[regIndex].interpInverseDuration     = (0.0f == firstInterpDuration) ? 0.0f : 1.0f / firstInterpDuration;
		_registers[regIndex].interpNextInverseDuration = (0.0f == nextInterpDuration)  ? 0.0f : 1.0f / nextInterpDuration;
#endif // FX_XBOX_360
		_registers[regIndex].interpStartTime           = 0.0f;
		_registers[regIndex].firstRegOp                = FxRegister::GetUserOpForRegOp(firstRegOp);
		_registers[regIndex].nextRegOp		           = nextRegOp;
		_registers[regIndex].isInterpolating           = FxFalse;
		return FxTrue;
	}
	return FxFalse;
}

void FxActorInstance::SetAllRegisters( FxRegisterOp firstRegOp, FxReal firstValue, 
									   FxReal firstInterpDuration, 
									   FxRegisterOp nextRegOp, FxReal nextValue, 
									   FxReal nextInterpDuration )
{
	FxSize numRegisters = _registers.Length();
	for( FxSize i = 0; i < numRegisters; ++i )
	{
		_registers[i].interpStartValue   = (RO_LoadAdd      == firstRegOp || 
			                                RO_LoadMultiply == firstRegOp || 
											RO_LoadReplace  == firstRegOp)
			                               ? _registers[i].value 
										   : _registers[i].interpLastValue;
		_registers[i].interpEndValue            = firstValue;
		_registers[i].interpNextEndValue        = nextValue;
#ifdef FX_XBOX_360
		_registers[i].interpInverseDuration     = static_cast<FxReal>(__fsel(firstInterpDuration, 
												  __fsel(-firstInterpDuration, 0.0f, 1.0f / firstInterpDuration), 
												  1.0f / firstInterpDuration));
		_registers[i].interpNextInverseDuration = static_cast<FxReal>(__fsel(nextInterpDuration, 
												  __fsel(-nextInterpDuration, 0.0f, 1.0f / nextInterpDuration), 
												  1.0f / firstInterpDuration));
#else // !FX_XBOX_360
		_registers[i].interpInverseDuration     = (0.0f == firstInterpDuration) ? 0.0f : 1.0f / firstInterpDuration;
		_registers[i].interpNextInverseDuration = (0.0f == nextInterpDuration)  ? 0.0f : 1.0f / nextInterpDuration;
#endif // FX_XBOX_360
		_registers[i].interpStartTime           = 0.0f;
		_registers[i].firstRegOp                = FxRegister::GetUserOpForRegOp(firstRegOp);
		_registers[i].nextRegOp		            = nextRegOp;
		_registers[i].isInterpolating           = FxFalse;
	}
}

FxReal FxActorInstance::GetRegister( const FxName& regName )
{
	FxAssert(_pActor);
	FxCompiledFaceGraph& cg = _pActor->GetCompiledFaceGraph();
	FxSize regIndex = cg.FindNodeIndex(regName);
	if( FxInvalidIndex != regIndex )
	{
		return _registers[regIndex].value;
	}
	return 0.0f;
}

FxBool FxActorInstance::GetRegisterState( const FxName& regName, FxRegister& regState )
{
	FxAssert(_pActor);
	FxCompiledFaceGraph& cg = _pActor->GetCompiledFaceGraph();
	FxSize regIndex = cg.FindNodeIndex(regName);
	if( FxInvalidIndex != regIndex )
	{
		regState = _registers[regIndex];
		return FxTrue;
	}
	return FxFalse;
}

FxReal FxActorInstance::GetOriginalRegisterValue( const FxName& regName )
{
	FxAssert(_pActor);
	FxCompiledFaceGraph& cg = _pActor->GetCompiledFaceGraph();
	FxSize regIndex = cg.FindNodeIndex(regName);
	if( FxInvalidIndex != regIndex )
	{
		return _registers[regIndex].interpEndValue;
	}
	return 0.0f;
}

} // namespace Face

} // namespace OC3Ent
