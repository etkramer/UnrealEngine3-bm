//------------------------------------------------------------------------------
// An audio player widget.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxAudioPlayerWidget.h"
#include "FxConsole.h"
#include "FxStudioApp.h"
#include "FxAnimUserData.h"
#ifdef IS_FACEFX_STUDIO
	#include "FxVM.h"
#endif

#include "FxAudioPlayerOpenAL.h"

#if defined(WIN32) && !defined(__UNREAL__)
	#include "FxAudioPlayerDirectSound8.h"
#endif

namespace OC3Ent
{

namespace Face
{

FxBool FxAudioPlayerWidget::_isInitialized = FxFalse;
FxAudioPlayer* FxAudioPlayerWidget::_audioPlayer = NULL;

FxAudioPlayerWidget::FxAudioPlayerWidget( FxWidgetMediator* mediator )
	: FxWidget(mediator)
	, _isScrubbing(FxFalse)
#ifdef IS_FACEFX_STUDIO
	, g_playbackspeed(FxVM::FindConsoleVariable("g_playbackspeed"))
#else
	, g_playbackspeed(NULL)
#endif
	, _pAnim(NULL)
{
#ifdef IS_FACEFX_STUDIO
	FxAssert(g_playbackspeed);
#endif
}

FxAudioPlayerWidget::~FxAudioPlayerWidget()
{
	if( _audioPlayer )
	{
		delete _audioPlayer;
		_audioPlayer   = NULL;
		_isInitialized = FxFalse;
	}
}

void FxAudioPlayerWidget::Initialize( void )
{
	if( !_audioPlayer )
	{
#ifndef __APPLE__

#ifdef IS_FACEFX_STUDIO
		FxConsoleVariable* pAudioSystemVar = FxVM::FindConsoleVariable("g_audiosystem");
		if( pAudioSystemVar )
#endif
		{
#ifdef IS_FACEFX_STUDIO
			FxString audioSystemString = pAudioSystemVar->GetString();
#else
			FxString audioSystemString("openal");
#endif
			if( FxString("directsound") == audioSystemString )
			{
#if defined(WIN32) && !defined(__UNREAL__) && defined(IS_FACEFX_STUDIO)
				_audioPlayer = new FxAudioPlayerDirectSound8();
				FxMsg("Using DirectSound.");
#endif
			}
			else if( FxString("openal") == audioSystemString )
			{
				_audioPlayer = new FxAudioPlayerOpenAL();
				FxMsg("Using OpenAL.");
			}
			else
			{
				FxError("Invalid g_audiosystem specified!");
			}
		}
#endif
	}
	if( _audioPlayer )
	{
		if( !_isInitialized )
		{
			_audioPlayer->Initialize(FxStudioApp::GetMainWindow());
			_isInitialized = FxTrue;
			// Force a re-copy of the audio buffer from the current animation
			// into the newly created sound system (if there was a previous
			// animation).
			OnAnimChanged(NULL, FxName::NullName, _pAnim);
		}
	}
}

void FxAudioPlayerWidget::OnAudioSystemChanged( FxConsoleVariable& FxUnused(cvar) )
{
	if( _audioPlayer )
	{
		delete _audioPlayer;
		_audioPlayer   = NULL;
		_isInitialized = FxFalse;
	}
}

FxBool FxAudioPlayerWidget::IsPlaying( void ) const
{
	if( _audioPlayer )
	{
		return _audioPlayer->IsPlaying();
	}
	return FxFalse;
}

FxBool FxAudioPlayerWidget::IsScrubbing( void ) const
{
	return _isScrubbing;
}

FxReal FxAudioPlayerWidget::GetAudioOffset( void ) const
{
	if( _audioPlayer )
	{
		return _audioPlayer->GetPlayCursorPosition();
	}
	return 0.0f;
}

void FxAudioPlayerWidget::OnAppStartup( FxWidget* FxUnused(sender) )
{
}

void FxAudioPlayerWidget::OnActorChanged( FxWidget* FxUnused(sender), FxActor* FxUnused(actor) )
{
}

void FxAudioPlayerWidget::
OnAnimChanged( FxWidget* FxUnused(sender), const FxName& FxUnused(animGroupName), FxAnim* anim )
{
	_pAnim = anim;
	Initialize();
	if( _audioPlayer )
	{
		if( anim && anim->GetUserData() )
		{
			FxAnimUserData* userData = static_cast<FxAnimUserData*>(anim->GetUserData());
			_audioPlayer->SetSource(userData->GetAudioPointer());
		}
		else
		{
			_audioPlayer->SetSource(NULL);
		}
	}
}

void FxAudioPlayerWidget::OnTimeChanged( FxWidget* FxUnused(sender), FxReal FxUnused(newTime) )
{
}

void FxAudioPlayerWidget::
OnPlayAudio( FxWidget* FxUnused(sender), FxReal startTime, FxReal endTime, FxReal pitch, FxBool isScrubbing, FxBool loop )
{
	Initialize();
	if( _audioPlayer )
	{
		_isScrubbing = isScrubbing;
		const FxAudio* pSource = _audioPlayer->GetSource();
		if( pSource )
		{
			if( _isScrubbing && (startTime < 0 || endTime > pSource->GetDuration()) )
			{
				// No scrubbing beyond the bounds of the audio.
				return;
			}
		}
		_audioPlayer->SetPlayRange(startTime, endTime);
		FxReal computedPitch = pitch;
#ifdef IS_FACEFX_STUDIO
		if( g_playbackspeed && !isScrubbing )
		{
			computedPitch = static_cast<FxReal>(*g_playbackspeed);
			if( computedPitch <= 0.0f )
			{
				computedPitch = 1.0f;
			}
		}
#endif
		_audioPlayer->SetPitch(computedPitch);
		_audioPlayer->Play(loop);
	}
}

void FxAudioPlayerWidget::OnStopAudio( FxWidget* FxUnused(sender) )
{
	Initialize();
	if( _audioPlayer )
	{
		_audioPlayer->Stop();
		_isScrubbing = FxFalse;
	}
}

} // namespace Face

} // namespace OC3Ent
