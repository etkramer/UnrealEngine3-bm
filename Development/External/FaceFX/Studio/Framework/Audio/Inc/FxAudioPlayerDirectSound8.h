//------------------------------------------------------------------------------
// A DirectSound8 based audio player.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxAudioPlayerDirectSound8_H__
#define FxAudioPlayerDirectSound8_H__

#include "FxPlatform.h"
#include "FxAudioPlayer.h"

#if defined(WIN32) && !defined(__UNREAL__)

#include "PreWindowsApi.h"
#include <mmsystem.h>
#include "dsound.h"
#include "PostWindowsApi.h"

namespace OC3Ent
{

namespace Face
{

// A DirectSound8 based audio player.
class FxAudioPlayerDirectSound8 : public FxAudioPlayer
{
	// Declare the class.
	FX_DECLARE_CLASS(FxAudioPlayerDirectSound8, FxAudioPlayer);
	// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxAudioPlayerDirectSound8);
public:
	// Constructor.
	FxAudioPlayerDirectSound8();
	// Destructor.
	virtual ~FxAudioPlayerDirectSound8();

	// Initializes the audio player.  This must be called with a valid pointer
	// to the application's main window.
	virtual void Initialize( wxWindow* window );

	// Returns the current audio source.
	virtual const FxAudio* GetSource( void ) const;
	// Sets the current audio source.  Initially the play range is set to
	// the full audio range.
	virtual void SetSource( FxAudio* source );

	// Returns the current play range in rangeStart and rangeEnd.
	virtual void GetPlayRange( FxReal& rangeStart, FxReal& rangeEnd ) const;
	// Sets the current play range.
	virtual void SetPlayRange( FxReal rangeStart, FxReal rangeEnd );

	// Returns the current volume.
	virtual FxReal GetVolume( void ) const;
	// Sets the current volume.
	virtual void SetVolume( FxReal volume );

	// Returns the current pitch.
	virtual FxReal GetPitch( void ) const;
	// Sets the current pitch.
	virtual void SetPitch( FxReal pitch );

	// Plays the current audio play range.
	virtual void Play( FxBool loop = FxFalse );
	// Pauses the audio.
	virtual void Pause( void );
	// Stops the audio.
	virtual void Stop( void );

	// Returns FxTrue if the audio is currently playing, FxFalse otherwise.
	virtual FxBool IsPlaying( void ) const;
	// Returns FxTrue if the audio is set to loop, FxFalse otherwise.
	virtual FxBool IsLooping( void ) const;

	// Returns the current play cursor position in seconds.
	virtual FxReal GetPlayCursorPosition( void ) const;
	// Sets the current play cursor position in seconds.  Returns FxTrue
	// if the play cursor position was set, FxFalse otherwise.
	virtual FxBool SetPlayCursorPosition( FxReal position );

protected:
	// The DirectSound interface.
	IDirectSound8* _directSoundInterface;
	// The DirectSound buffer. 
	IDirectSoundBuffer* _directSoundBuffer;
	// FxTrue if the buffer has been fully created.
	FxBool _bufferCreated;
	// Byte offset to the start of the sound buffer.
	DWORD _bufferStart;
	// Byte offset to the end of the buffer.
	DWORD _bufferEnd;

	// Clears the DirectSound buffer.
	void _clearDirectSoundBuffer( void );

	// Sets the volume in the DirectSound Buffer.
	void _setDirectSoundBufferVolume( void );
	// Sets the frequency (pitch) in the DirectSound Buffer.
	void _setDirectSoundBufferFrequency( void );

	// Returns the byte offset of the current play cursor position.
	DWORD _getPlayCursorByteOffset( void ) const;
	// Sets the byte offset of the current play cursor position.
	void _setPlayCursorByteOffset( DWORD offset );
};

} // namespace Face

} // namespace OC3Ent

#endif

#endif
