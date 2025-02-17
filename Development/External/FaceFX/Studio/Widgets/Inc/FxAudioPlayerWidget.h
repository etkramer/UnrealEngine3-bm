//------------------------------------------------------------------------------
// An audio player widget.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxAudioPlayerWidget_H__
#define FxAudioPlayerWidget_H__

#include "FxPlatform.h"
#include "FxWidget.h"
#include "FxAudioPlayer.h"

namespace OC3Ent
{

namespace Face
{

class FxConsoleVariable;

// An audio player widget.
class FxAudioPlayerWidget : public FxWidget
{
public:
	// Constructor.
	FxAudioPlayerWidget( FxWidgetMediator* mediator = NULL );
	// Destructor.
	virtual ~FxAudioPlayerWidget();

	// Initializes the audio player widget.  This can't happen during construction
	// because the audio player widget needs a pointer to the main application
	// window, which isn't valid when widgets are created.
	void Initialize( void );
	// Callback for when the audio system console variable is changed.
	static void OnAudioSystemChanged( FxConsoleVariable& cvar );

	// Returns FxTrue if the audio player widget is currently playing audio.
	FxBool IsPlaying( void ) const;
	// Returns FxTrue if the audio player widget is currently scrubbing audio.
	FxBool IsScrubbing( void ) const;

	// Returns the current audio play offset in seconds.
	FxReal GetAudioOffset( void ) const;

	// FxWidget message handlers.
	virtual void OnAppStartup( FxWidget* sender );
	virtual void OnActorChanged( FxWidget* sender, FxActor* actor );
	// Called when the current animation has changed.
	virtual void OnAnimChanged( FxWidget* sender, const FxName& animGroupName, 
		FxAnim* anim );
	virtual void OnTimeChanged( FxWidget* sender, FxReal newTime );
	virtual void OnPlayAudio( FxWidget* sender, FxReal startTime, 
		FxReal endTime, FxReal pitch = 1.0f, FxBool isScrubbing = FxFalse,
		FxBool loop = FxFalse );
	virtual void OnStopAudio( FxWidget* sender );
    
protected:
	// FxTrue if the audio player widget has been initialized, FxFalse otherwise.
	static FxBool _isInitialized;
	// FxTrue if the audio player is currently scrubbing, FxFalse otherwise.
	FxBool _isScrubbing;
	// The audio player.
	static FxAudioPlayer* _audioPlayer;
	// The g_playbackspeed console variable.
	FxConsoleVariable* g_playbackspeed;
	// A cached pointer to the current animation.  Only used if the user changes
	// the audio system via the console variable.
	FxAnim* _pAnim;
};

} // namespace Face

} // namespace OC3Ent

#endif
