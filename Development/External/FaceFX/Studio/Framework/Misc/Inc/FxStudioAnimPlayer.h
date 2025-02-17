//------------------------------------------------------------------------------
// The animation playback system for FaceFX Studio.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxStudioAnimPlayer_H__
#define FxStudioAnimPlayer_H__

#include "FxPlatform.h"
#include "FxConsoleVariable.h"

namespace OC3Ent
{

namespace Face
{

class FxTimelineWidget;
class FxAudioPlayerWidget;
class FxStudioSession;

class FxStudioAnimPlayer : public wxEvtHandler
{
public:
	// Returns the instance of the anim player.
	static FxStudioAnimPlayer* Instance();
	// Starts up the anim player.
	static void Startup();
	// Destroys the instance of the anim player.
	static void Shutdown();

	// Sets the pointer to the audio player widget.
	void SetAudioPlayerWidgetPointer( FxAudioPlayerWidget* audioPlayer );
	// Sets the pointer to the timeline widget.
	void SetTimelineWidgetPointer( FxTimelineWidget* timeline );
	// Sets the pointer to the session
	void SetSessionPointer( FxStudioSession* session );

	// Starts playback of the current window from the cursor.
	void Play(FxBool loop, FxBool ignoreCursor = FxFalse);
	// Plays a section of audio
	void PlaySection(FxReal start, FxReal end);
	// Restarts playback
	void Restart();
	// Stops playback
	void Stop( FxBool stoppedAutomatically = FxFalse );
	// Returns true if playback is in progress.
	FxBool IsPlaying();

	// Plays a segment of audio during scrubbing
	void ScrubAudio( FxReal startTime, FxReal timeDelta, FxReal pitch );
	// Stops scrubbing
	void StopScrubbing();

	// Sets the looping state.
	void SetLooping(FxBool loop);

	// Updates the state of the animation.
	void UpdateAnimation( wxTimerEvent& event );

protected:

	FxAudioPlayerWidget* _pAudioPlayer;
	FxTimelineWidget*    _pTimeline;
	FxStudioSession*	 _pSession;

	wxStopWatch _stopwatch;
	wxTimer*    _timer;

	FxReal _startTime;
	FxReal _minTime;
	FxReal _maxTime;
	FxReal _cursorTime;
	FxReal _currentTime;
	FxBool _hasStartedAudio;
	FxBool _loop;
	// The g_playbackspeed console variable.
	FxConsoleVariable* g_playbackspeed;

	DECLARE_EVENT_TABLE()

private:
	// Constructor
	FxStudioAnimPlayer();
	// Destructor
	~FxStudioAnimPlayer();

	/// Make copy construction and assignment break the compile by not providing
	/// definitions.
	FxStudioAnimPlayer( const FxStudioAnimPlayer& other );
	FxStudioAnimPlayer& operator=( const FxStudioAnimPlayer& other );

	static FxStudioAnimPlayer* _pInst;
	static FxBool _destroyed;
};

} // namespace Face

} // namespace OC3Ent

#endif
