//------------------------------------------------------------------------------
// The animation playback system for FaceFX Studio.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxAudioPlayerWidget.h"
#include "FxStudioSession.h"
#include "FxTimelineWidget.h"
#include "FxStudioAnimPlayer.h"
#include "FxMath.h"
#include "FxVM.h"

namespace OC3Ent
{

namespace Face
{

FxStudioAnimPlayer* FxStudioAnimPlayer::_pInst = NULL;
FxBool FxStudioAnimPlayer::_destroyed = FxFalse;

#define ANIMPLAYER_TIMER_ID 1025

BEGIN_EVENT_TABLE(FxStudioAnimPlayer, wxEvtHandler)
	EVT_TIMER(ANIMPLAYER_TIMER_ID, FxStudioAnimPlayer::UpdateAnimation)
END_EVENT_TABLE()

// Returns the instance of the anim player.
FxStudioAnimPlayer* FxStudioAnimPlayer::Instance()
{
	if( !_pInst && !_destroyed )
	{
		_pInst = new FxStudioAnimPlayer;
	}
	return _pInst;
}

void FxStudioAnimPlayer::Startup()
{
	_destroyed = FxFalse;
	_pInst = NULL;
	Instance();
}

// Destroys the instance of the anim player.
void FxStudioAnimPlayer::Shutdown()
{
	delete _pInst;
	_pInst = NULL;
	_destroyed = FxTrue;
}

FxStudioAnimPlayer::FxStudioAnimPlayer()
	: _pAudioPlayer(NULL)
	, _pTimeline(NULL)
	, _pSession(NULL)
	, _timer(NULL)
	, _startTime(FxInvalidValue)
	, _minTime(FxInvalidValue)
	, _maxTime(FxInvalidValue)
	, _cursorTime(FxInvalidValue)
	, _currentTime(FxInvalidValue)
	, _hasStartedAudio(FxFalse)
	, _loop(FxFalse)
	, g_playbackspeed(FxVM::FindConsoleVariable("g_playbackspeed"))
{
	FxAssert(g_playbackspeed);
	_timer = new wxTimer(this, ANIMPLAYER_TIMER_ID);
}

FxStudioAnimPlayer::~FxStudioAnimPlayer()
{
	if( _timer )
	{
		delete _timer;
	}
}

void FxStudioAnimPlayer::SetAudioPlayerWidgetPointer( FxAudioPlayerWidget* audioPlayer )
{
	_pAudioPlayer = audioPlayer;
}

void FxStudioAnimPlayer::SetTimelineWidgetPointer( FxTimelineWidget* timeline )
{
	_pTimeline = timeline;
}
	
void FxStudioAnimPlayer::SetSessionPointer( FxStudioSession* session )
{
	_pSession = session;
}

void FxStudioAnimPlayer::Play( FxBool loop, FxBool ignoreCursor )
{
	if( _pAudioPlayer )
	{
		if( _pAudioPlayer->IsPlaying() )
		{
			_pAudioPlayer->OnStopAudio(NULL);
		}
	}
	_timer->Stop();

	_stopwatch.Start();
	_startTime = static_cast<FxReal>(_stopwatch.Time())/1000.0f;
	if( _pTimeline )
	{
		_maxTime = _pTimeline->GetTimeRangeEnd();
		if( ignoreCursor )
		{
			_cursorTime = _pTimeline->GetTimeRangeStart();
		}
		else
		{
			_cursorTime = _pTimeline->GetCurrentTime();
		}
	}
	_minTime = _cursorTime;
	_hasStartedAudio = FxFalse;
	SetLooping(loop);

	_timer->Start(1);

	if( _pTimeline )
	{
		_pTimeline->OnPlaybackStarted();
	}
}

// Plays a section of audio
void FxStudioAnimPlayer::PlaySection( FxReal start, FxReal end )
{
	if( _pAudioPlayer )
	{
		if( _pAudioPlayer->IsPlaying() )
		{
			_pAudioPlayer->OnStopAudio(NULL);
		}
	}
	_timer->Stop();
	_stopwatch.Start();

	_startTime = static_cast<FxReal>(_stopwatch.Time())/1000.0f;
	_maxTime = end;
	_cursorTime = start;
	_minTime = _cursorTime;
	_hasStartedAudio = FxFalse;
	SetLooping(FxFalse);

	_timer->Start(1);

	if( _pTimeline )
	{
		_pTimeline->OnPlaybackStarted();
	}
}

void FxStudioAnimPlayer::Restart()
{
	_timer->Stop();
	if( _pAudioPlayer )
	{
		if( _pAudioPlayer->IsPlaying() )
		{
			_pAudioPlayer->OnStopAudio(NULL);
		}
	}
	_stopwatch.Start();
	_hasStartedAudio = FxFalse;
	_startTime = static_cast<FxReal>(_stopwatch.Time())/1000.0f;
	_timer->Start(1);
}

void FxStudioAnimPlayer::Stop(  FxBool stoppedAutomatically )
{
	if( IsPlaying() )
	{
		_timer->Stop();
		if( _pAudioPlayer )
		{
			if( _pAudioPlayer->IsPlaying() )
			{
				_pAudioPlayer->OnStopAudio(NULL);
			}
		}
		_stopwatch.Pause();
		if( _pTimeline )
		{
			_pTimeline->OnPlaybackFinished();
			_pTimeline->SetCurrentTime(_cursorTime, FxFalse, FxTrue);
		}
		_startTime = FxInvalidValue;
		_cursorTime = FxInvalidValue;

		// Alert anyone listening
		if( _pSession )
		{
			if( stoppedAutomatically )
			{
				_pSession->OnAnimationStopped(NULL);
			}
			else
			{
				_pSession->OnStopAnimation(NULL);
			}
		}
	}
}

FxBool FxStudioAnimPlayer::IsPlaying()
{
	return _startTime != FxInvalidValue;
}

void FxStudioAnimPlayer::ScrubAudio( FxReal startTime, FxReal timeDelta, FxReal pitch )
{
	if( _pAudioPlayer )
	{
		_pAudioPlayer->OnPlayAudio(NULL, startTime, startTime+timeDelta, pitch, FxTrue);
	}
}

void FxStudioAnimPlayer::StopScrubbing()
{
	if( _pAudioPlayer )
	{
		_pAudioPlayer->OnStopAudio(NULL);
	}
}

void FxStudioAnimPlayer::SetLooping(FxBool loop)
{
	_loop = loop;
}

void FxStudioAnimPlayer::UpdateAnimation( wxTimerEvent& FxUnused(event) )
{
	FxReal audioTime = -1.0f;
	if( _pAudioPlayer )
	{
		if( _pAudioPlayer->IsPlaying() )
		{
			audioTime = _pAudioPlayer->GetAudioOffset();
		}
	}
	FxReal appTime = static_cast<FxReal>(_stopwatch.Time())/1000.0f;

	FxReal playbackSpeed = 1.0f;
	if( g_playbackspeed )
	{
		playbackSpeed = static_cast<FxReal>(*g_playbackspeed);
		if( playbackSpeed <= 0.0f )
		{
			playbackSpeed = 1.0f;
		}
	}

	// See if there is audio playing (if not, audioTime should be less than 0.0)
	// and update _currentTime accordingly.
	if( audioTime < 0.0f )
	{
		_currentTime = static_cast<FxReal>((appTime - _startTime) * playbackSpeed) + _cursorTime;
	}
	else
	{
		// Note:  audioTime already takes g_playbackspeed into account.
		_currentTime = audioTime;
		// Keep appTime and audioTime from diverging.  This prevents situations
		// where appTime is slightly behind audioTime, so when the audio playback ends,
		// we don't go backwards in time when picking up at appTime where audioTime 
		// left off.
		_startTime = appTime - ((audioTime - _cursorTime) / playbackSpeed);
	}

	// Update the facial animation.
	if( _currentTime <= _maxTime )
	{
		// We're still playing.
		_pSession->OnTimeChanged(NULL, _currentTime);
	}
	else
	{
		if( _loop )
		{
			Restart();
			return;
		}
		else
		{
			// We're finished playing.
			Stop(FxTrue);
		}
	}

	// If we've reached the time when we should start the audio, do so.
	if( _currentTime >= 0.0f && !_hasStartedAudio )
	{
		_hasStartedAudio = FxTrue;
		if( _pAudioPlayer )
		{
			_pAudioPlayer->OnPlayAudio(NULL, _currentTime, _maxTime);
		}
	}
}

} // namespace Face

} // namespace OC3Ent
