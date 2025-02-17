//------------------------------------------------------------------------------
// An abstract audio player.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxAudioPlayer_H__
#define FxAudioPlayer_H__

#include "FxPlatform.h"
#include "FxObject.h"
#include "FxAudio.h"

namespace OC3Ent
{

namespace Face
{

// An abstract audio player.
class FxAudioPlayer : public FxObject
{
	// Declare the class.
	FX_DECLARE_CLASS_NO_DYNCREATE_NO_SERIALIZE(FxAudioPlayer, FxObject);
	// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxAudioPlayer);
public:
	// Constructor.
	FxAudioPlayer();
	// Destructor.
	virtual ~FxAudioPlayer();

	// Initializes the audio player.  This must be called with a valid pointer
	// to the application's main window.
	virtual void Initialize( wxWindow* window ) = 0;

	// Returns the current audio source.
	virtual const FxAudio* GetSource( void ) const = 0;
	// Sets the current audio source.  Initially the play range is set to
	// the full audio range.
	virtual void SetSource( FxAudio* source ) = 0;

	// Returns the current play range in rangeStart and rangeEnd.
	virtual void GetPlayRange( FxReal& rangeStart, FxReal& rangeEnd ) const = 0;
	// Sets the current play range.
	virtual void SetPlayRange( FxReal rangeStart, FxReal rangeEnd ) = 0;

	// Returns the current volume.
	virtual FxReal GetVolume( void ) const = 0;
	// Sets the current volume.
	virtual void SetVolume( FxReal volume ) = 0;

	// Returns the current pitch.
	virtual FxReal GetPitch( void ) const = 0;
	// Sets the current pitch.
	virtual void SetPitch( FxReal pitch ) = 0;

	// Plays the current audio play range.
	virtual void Play( FxBool loop = FxFalse ) = 0;
	// Pauses the audio.
	virtual void Pause( void ) = 0;
	// Stops the audio.
	virtual void Stop( void ) = 0;

	// Returns FxTrue if the audio is currently playing, FxFalse otherwise.
	virtual FxBool IsPlaying( void ) const = 0;
	// Returns FxTrue if the audio is set to loop, FxFalse otherwise.
	virtual FxBool IsLooping( void ) const = 0;
	
	// Returns the current play cursor position in seconds.
	virtual FxReal GetPlayCursorPosition( void ) const = 0;
	// Sets the current play cursor position in seconds.  Returns FxTrue
	// if the play cursor position was set, FxFalse otherwise.
	virtual FxBool SetPlayCursorPosition( FxReal position ) = 0;

protected:
	// The window associated with the audio player.
	wxWindow* _window;
	// The audio source.
	FxAudio* _source;
    // The play range start time.
	FxReal _rangeStart;
	// The play range end time.
	FxReal _rangeEnd;
	// The audio player volume.
	FxReal _volume;
	// The audio player pitch.
	FxReal _pitch;
	// FxTrue if the looping is enabled.
	FxBool _loop;
	// FxTrue if the audio is paused.
	FxBool _paused;
};

} // namespace Face

} // namespace OC3Ent

#endif
