//------------------------------------------------------------------------------
// An OpenAL based audio player.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxAudioPlayerOpenAL_H__
#define FxAudioPlayerOpenAL_H__

#include "FxPlatform.h"
#include "FxAudioPlayer.h"

#ifdef __UNREAL__
	#if SUPPORTS_PRAGMA_PACK
		#pragma pack (push,8)
	#endif
	#define AL_NO_PROTOTYPES 1
	#define ALC_NO_PROTOTYPES 1
#endif

#ifdef __APPLE__
	#include "OpenAL/al.h"
	#include "OpenAL/alc.h"
#else
	#include "al.h"
	#include "alc.h"
#endif

#ifdef __UNREAL__
	#if SUPPORTS_PRAGMA_PACK
		#pragma pack (pop)
	#endif
#endif

namespace OC3Ent
{

namespace Face
{

// An OpenAL based audio player.
class FxAudioPlayerOpenAL : public FxAudioPlayer
{
	// Declare the class.
	FX_DECLARE_CLASS(FxAudioPlayerOpenAL, FxAudioPlayer);
	// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxAudioPlayerOpenAL);
public:
	// Constructor.
	FxAudioPlayerOpenAL();
	// Destructor.
	virtual ~FxAudioPlayerOpenAL();

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
	// The OpenAL device.
	ALCdevice* _pDevice;
	// The OpenAL device context.
	ALCcontext* _pContext;
	// The OpenAL audio buffer.
	ALuint _alBuffer;
	// FxTrue if the OpenAL audio buffer has been created and is valid.
	FxBool _alBufferIsValid;
	// The OpenAL audio source.
	ALuint _alSource;
	// FxTrue if the OpenAL audio source has been created and is valid.
	FxBool _alSourceIsValid;
	// Byte offset to the start of the sound buffer.
	FxSize _bufferStart;
	// Byte offset to the end of the buffer.
	FxSize _bufferEnd;
	// FxTrue if OpenAL is "OK" (all function pointers are non-NULL).
	FxBool _isOK;

	// Clears the OpenAL sound buffer.
	void _clearOpenALBuffer( void );
	// Creates the OpenAL sound buffer.
	void _createOpenALBuffer( void );

	// OpenAL function signatures.
	ALboolean		(*Fx_alcMakeContextCurrent)( ALCcontext* context );
	ALvoid			(*Fx_alDeleteSources)( ALsizei n, const ALuint* sources );
	ALvoid			(*Fx_alcDestroyContext)( ALCcontext* context );
	ALboolean		(*Fx_alcCloseDevice)( ALCdevice* device );
	ALCdevice*		(*Fx_alcOpenDevice)( const ALbyte* deviceName );
	ALCcontext*		(*Fx_alcCreateContext)( ALCdevice* device, const ALint* attrList );
	ALenum			(*Fx_alGetError)( ALvoid );
	ALenum			(*Fx_alcGetError)( ALCdevice* device );
	ALvoid			(*Fx_alcGetIntegerv)( ALCdevice* device, ALenum param, ALsizei size, ALint* data );
	ALvoid			(*Fx_alListenerfv)( ALenum param, const ALfloat* values );
	ALvoid			(*Fx_alGenSources)( ALsizei n, ALuint* sources );
	ALvoid			(*Fx_alSourcef)( ALuint source, ALenum param, ALfloat value );
	ALvoid			(*Fx_alBufferData)( ALuint buffer, ALenum format, const ALvoid* data, ALsizei size, ALsizei freq );
	ALvoid			(*Fx_alSourcei)( ALuint source, ALenum param, ALint value );
	ALvoid			(*Fx_alSourcePlay)( ALuint source );
	ALvoid			(*Fx_alSourcePause)( ALuint source );
	ALvoid			(*Fx_alSourceStop)( ALuint source );
	ALvoid			(*Fx_alGetSourcei)( ALuint source, ALenum param, ALint* value );
	ALvoid			(*Fx_alGetSourcef)( ALuint source, ALenum param, ALfloat* value );
	ALvoid			(*Fx_alDeleteBuffers)( ALsizei n, const ALuint* buffers );
	const ALbyte*	(*Fx_alcGetString)( ALCdevice* device, ALenum param );
	ALvoid			(*Fx_alGenBuffers)( ALsizei n, ALuint* buffers );
};

} // namespace Face

} // namespace OC3Ent

#endif
