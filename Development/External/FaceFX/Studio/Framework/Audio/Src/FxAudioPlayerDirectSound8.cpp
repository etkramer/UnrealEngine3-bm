//------------------------------------------------------------------------------
// A DirectSound8 based audio player.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxAudioPlayerDirectSound8.h"
#include "FxMath.h"
#include "FxConsole.h"

#if defined(WIN32) && !defined(__UNREAL__)

// Link in DirectSound if compiling with DirectSound support.
#pragma comment(lib, "dsound.lib")
#pragma message("Linking DirectSound...")

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxAudioPlayerDirectSound8Version 0

FX_IMPLEMENT_CLASS(FxAudioPlayerDirectSound8, kCurrentFxAudioPlayerDirectSound8Version, FxAudioPlayer);

FxAudioPlayerDirectSound8::FxAudioPlayerDirectSound8()
	: _directSoundInterface(NULL)
	, _directSoundBuffer(NULL)
	, _bufferCreated(FxFalse)
	, _bufferStart(0)
	, _bufferEnd(0)
{
}

FxAudioPlayerDirectSound8::~FxAudioPlayerDirectSound8()
{
	// Free the DirectSound interface.
	_clearDirectSoundBuffer();

	if( _directSoundInterface )
	{
		_directSoundInterface->Release();
		_directSoundInterface = NULL;
	}
}

void FxAudioPlayerDirectSound8::Initialize( wxWindow* window )
{
	_window = window;

	if( _window )
	{
		if( !_directSoundInterface )
		{
			// Get the DirectSound interface.
			if( DS_OK != DirectSoundCreate8(NULL, &_directSoundInterface, NULL) )
			{
				// If this fails it is more than likely because of DSERR_NODRIVER.  Inform the user and
				// reset some internal variables to basically shut off the rest of the DirectX sound code.
				FxWarn( "FxAudioPlayerDirectSound8::Initialize -> DirectSoundCreate8 failed!" );
				FxWarn( "Make sure your sound card and drivers are installed properly and that no other application has the audio focus." );

				_directSoundInterface = NULL;
				_directSoundBuffer    = NULL;
				_bufferCreated        = FxFalse;
				_bufferStart          = 0;
				_bufferEnd            = 0;
			}
			else
			{
				if( DS_OK != _directSoundInterface->SetCooperativeLevel(reinterpret_cast<HWND>(_window->GetHWND()), DSSCL_PRIORITY) )
				{
					FxWarn( "FxAudioPlayerDirectSound8::Initialize -> IDirectSound8::SetCooperativeLevel failed!" );
				}
			}
		}
	}
}

const FxAudio* FxAudioPlayerDirectSound8::GetSource( void ) const
{
	return _source;
}

void FxAudioPlayerDirectSound8::SetSource( FxAudio* source )
{
	_source = source;

	// Clear the DirectSound buffer.
	_clearDirectSoundBuffer();
	_bufferCreated = FxFalse;

	if( _directSoundInterface )
	{
		if( _source )
		{
			WAVEFORMATEX wfex; 
			DSBUFFERDESC dsbdesc; 

			// Set up wave format structure.
			FxMemset(&wfex, 0, sizeof(WAVEFORMATEX));
			wfex.wFormatTag		 = WAVE_FORMAT_PCM;
			wfex.nChannels		 = _source->GetNumChannels();
			wfex.nSamplesPerSec	 = _source->GetSampleRate();
			wfex.nBlockAlign	 = ((_source->GetBitsPerSample() * _source->GetNumChannels()) / FX_CHAR_BIT);
			wfex.nAvgBytesPerSec = wfex.nSamplesPerSec * wfex.nBlockAlign;
			wfex.wBitsPerSample  = _source->GetBitsPerSample();
			wfex.cbSize			 = 0;

			// Set up DSBUFFERDESC structure. 
			FxMemset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
			dsbdesc.dwSize		  = sizeof(DSBUFFERDESC);
			dsbdesc.dwFlags		  = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
			dsbdesc.dwBufferBytes = ((_source->GetNumSamples() * _source->GetBitsPerSample()) / FX_CHAR_BIT);
			dsbdesc.lpwfxFormat   = &wfex;

			// Create the buffer. 
			HRESULT hr;
			hr = _directSoundInterface->CreateSoundBuffer(&dsbdesc, &_directSoundBuffer, NULL);
			if( DS_OK != hr )
			{
				_directSoundBuffer = NULL;
			}

			if( _directSoundBuffer )
			{
				_bufferStart = 0;
				_bufferEnd   = dsbdesc.dwBufferBytes;

				// Fill the buffer with audio.
				LPVOID buff1, buff2;
				DWORD size1, size2;

				// Lock the buffer.
				_directSoundBuffer->Lock(0, dsbdesc.dwBufferBytes, &((LPVOID)buff1), &size1, &((LPVOID)buff2), &size2, DSBLOCK_ENTIREBUFFER);

				// Get the raw sample buffer from the audio source.
				FxRawSampleBuffer rawSampleBuffer = _source->ExportRawSampleBuffer();
				
				if( rawSampleBuffer.GetSamples() )
				{
					// Copy raw sample buffer into the DirectSound buffer.
					FxMemcpy((void*)buff1, rawSampleBuffer.GetSamples(), dsbdesc.dwBufferBytes);
				}

				// Unlock the buffer.
				_directSoundBuffer->Unlock(buff1, size1, buff2, size2);
			}
			_bufferCreated = FxTrue;
		}
	}
}

void FxAudioPlayerDirectSound8::
GetPlayRange( FxReal& rangeStart, FxReal& rangeEnd ) const
{
	rangeStart = _rangeStart;
	rangeEnd   = _rangeEnd;
}

void FxAudioPlayerDirectSound8::
SetPlayRange( FxReal rangeStart, FxReal rangeEnd )
{
	_rangeStart = rangeStart;
	_rangeEnd   = rangeEnd;

	// Swap the range end and range start if they don't make sense.
	if( _rangeEnd < _rangeStart )
	{
		FxReal temp = _rangeStart;
		_rangeStart = _rangeEnd;
		_rangeEnd   = temp;
	}

	if( !_paused )
	{
		_clearDirectSoundBuffer();
		_bufferCreated = FxFalse;

		if( _directSoundInterface )
		{
			if( _source )
			{
				//@todo At some point we'll want to do smarter range checking here.
				if( _rangeStart < 0.0f )
				{
					_rangeStart = 0.0f;
				}
				if( _rangeEnd < 0.0f )
				{
					_rangeEnd = 0.0f;
				}
				if( _rangeStart > _source->GetDuration() )
				{
					_rangeStart = _source->GetDuration();
				}
				if( _rangeEnd > _source->GetDuration() )
				{
					_rangeEnd = _source->GetDuration();
				}

				_bufferStart = ((_rangeStart * _source->GetSampleRate() * _source->GetBitsPerSample()) / FX_CHAR_BIT);

				// If there is no selection, just copy the entire rest of the buffer.
				if( _rangeStart == _rangeEnd )
				{
					_bufferEnd = ((_source->GetNumSamples() * _source->GetBitsPerSample()) / FX_CHAR_BIT);
				}
				else
				{
					_bufferEnd = ((_rangeEnd * _source->GetSampleRate() * _source->GetBitsPerSample()) / FX_CHAR_BIT);
				}

				WAVEFORMATEX wfex; 
				DSBUFFERDESC dsbdesc; 

				// Set up wave format structure. 
				FxMemset(&wfex, 0, sizeof(WAVEFORMATEX)); 
				wfex.wFormatTag		 = WAVE_FORMAT_PCM; 
				wfex.nChannels		 = _source->GetNumChannels();
				wfex.nSamplesPerSec	 = _source->GetSampleRate(); 
				wfex.nBlockAlign	 = ((_source->GetBitsPerSample() * _source->GetNumChannels()) / FX_CHAR_BIT); 
				wfex.nAvgBytesPerSec = wfex.nSamplesPerSec * wfex.nBlockAlign;
				wfex.wBitsPerSample  = _source->GetBitsPerSample();
				wfex.cbSize			 = 0;

				if( _bufferStart % wfex.nBlockAlign )
				{
					--_bufferStart;
				}

				if( _bufferEnd % wfex.nBlockAlign )
				{
					++_bufferEnd;
				}

				DWORD dwBufferBytes = _bufferEnd - _bufferStart;

				// Set up DSBUFFERDESC structure. 
				FxMemset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); 
				dsbdesc.dwSize		  = sizeof(DSBUFFERDESC); 
				dsbdesc.dwFlags		  = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
				dsbdesc.dwBufferBytes = dwBufferBytes;
				dsbdesc.lpwfxFormat   = &wfex; 

				// Create the buffer. 
				HRESULT hr;
				hr = _directSoundInterface->CreateSoundBuffer(&dsbdesc, &_directSoundBuffer, NULL);
				if( DS_OK != hr )
				{
					_directSoundBuffer = NULL;
				}

				if( _directSoundBuffer )
				{
					// Fill the buffer with audio.
					LPVOID buff1, buff2;
					DWORD size1, size2;

					// Lock the buffer.
					_directSoundBuffer->Lock(0, dsbdesc.dwBufferBytes, &((LPVOID)buff1), &size1, &((LPVOID)buff2), &size2, DSBLOCK_ENTIREBUFFER);

					// Get the raw sample buffer from the audio source.
					FxRawSampleBuffer rawSampleBuffer = _source->ExportRawSampleBuffer();

					if( rawSampleBuffer.GetSamples() )
					{
						// Copy _bufferStart through _bufferEnd from the sample buffer into the DirectSound buffer.
						FxMemcpy((void*)buff1, rawSampleBuffer.GetSamples() + _bufferStart, dsbdesc.dwBufferBytes);
					}

					// Unlock the buffer.
					_directSoundBuffer->Unlock(buff1, size1, buff2, size2);
				}
				_bufferCreated = FxTrue;
			}
		}
	}
}

FxReal FxAudioPlayerDirectSound8::GetVolume( void ) const
{
	return _volume;
}

void FxAudioPlayerDirectSound8::SetVolume( FxReal volume )
{
	_volume = volume;
	_setDirectSoundBufferVolume();
}

FxReal FxAudioPlayerDirectSound8::GetPitch( void ) const
{
	return _pitch;
}

void FxAudioPlayerDirectSound8::SetPitch( FxReal pitch )
{
	_pitch = pitch;
	_setDirectSoundBufferFrequency();
}

void FxAudioPlayerDirectSound8::Play( FxBool loop )
{
	if( _directSoundBuffer && _bufferCreated )
	{
		if( _paused )
		{
			_paused = FxFalse;
		}
		_loop = loop;
		if( _loop )
		{
			_directSoundBuffer->Play(0, 0, DSBPLAY_LOOPING);
		}
		else
		{
			_directSoundBuffer->Play(0, 0, 0);
		}
	}
}

void FxAudioPlayerDirectSound8::Pause( void )
{
	if( _directSoundBuffer && _bufferCreated )
	{
		_paused = !_paused;
		if( _paused )
		{
			_directSoundBuffer->Stop();
		}
		else
		{
			_directSoundBuffer->Play(0, 0, 0);
		}
	}
}

void FxAudioPlayerDirectSound8::Stop( void )
{
	if( _directSoundBuffer && _bufferCreated )
	{
		_directSoundBuffer->Stop();
		_paused = FxFalse;
		_loop   = FxFalse;
	}
}

FxBool FxAudioPlayerDirectSound8::IsPlaying( void ) const
{
	if( _directSoundBuffer && _bufferCreated )
	{
		DWORD dwStatus;
		_directSoundBuffer->GetStatus(&dwStatus);
		if( dwStatus & DSBSTATUS_PLAYING )
		{
			return FxTrue;
		}
		else
		{
			return FxFalse;
		}
	}
	else
	{
		return FxFalse;
	}
}

FxBool FxAudioPlayerDirectSound8::IsLooping( void ) const
{
	return _loop;
}

FxReal FxAudioPlayerDirectSound8::GetPlayCursorPosition( void ) const
{
	if( _source && _directSoundBuffer && _bufferCreated )
	{
		DWORD playPos = _getPlayCursorByteOffset();
		return (static_cast<FxReal>((_bufferStart + playPos) * FX_CHAR_BIT) / 
			    static_cast<FxReal>(_source->GetBitsPerSample() * _source->GetSampleRate()));
	}
	else
	{
		return 0.0f;
	}
}

FxBool FxAudioPlayerDirectSound8::SetPlayCursorPosition( FxReal position )
{
	if( _source && _directSoundBuffer && _bufferCreated )
	{
		DWORD playPos = static_cast<DWORD>(static_cast<FxReal>((position * 
			            static_cast<FxReal>((_source->GetBitsPerSample() * _source->GetSampleRate()))) / FX_CHAR_BIT));
		_setPlayCursorByteOffset(_bufferStart + playPos);
		return FxTrue;
	}
	return FxFalse;
}

void FxAudioPlayerDirectSound8::_clearDirectSoundBuffer( void )
{
	if( _directSoundBuffer && _bufferCreated )
	{
		if( IsPlaying() )
		{
			Stop();
		}

		_directSoundBuffer->Release();
		_directSoundBuffer = NULL;
		_bufferCreated     = FxFalse;
	}
}

void FxAudioPlayerDirectSound8::_setDirectSoundBufferVolume( void )
{
	if( _directSoundBuffer && _bufferCreated )
	{
		_directSoundBuffer->SetVolume(_volume * 255);
	}
}

void FxAudioPlayerDirectSound8::_setDirectSoundBufferFrequency( void )
{
	if( _directSoundBuffer && _bufferCreated )
	{
		_directSoundBuffer->SetFrequency(_source->GetSampleRate() * _pitch);
	}
}

DWORD FxAudioPlayerDirectSound8::_getPlayCursorByteOffset( void ) const
{
	if( _directSoundBuffer && _bufferCreated )
	{
		DWORD playPos;
		_directSoundBuffer->GetCurrentPosition(&playPos, NULL);
		return playPos;
	}
	else
	{
		return 0;
	}
}

void FxAudioPlayerDirectSound8::_setPlayCursorByteOffset( DWORD offset )
{
	if( _directSoundBuffer && _bufferCreated )
	{
		_directSoundBuffer->SetCurrentPosition(offset);
	}
}

} // namespace Face

} // namespace OC3Ent

#endif
