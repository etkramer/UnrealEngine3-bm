/*=============================================================================
	XeAudioDevice.cpp: Unreal XAudio2 Audio interface object.
	Copyright 1998-2009 Epic Games, Inc. All Rights Reserved.

	Unreal is RHS with Y and Z swapped (or technically LHS with flipped axis)

=============================================================================*/

/*------------------------------------------------------------------------------------
	Audio includes.
------------------------------------------------------------------------------------*/

#include "Engine.h"
#include "EngineSoundClasses.h"
#include "EngineAudioDeviceClasses.h"
#include "UnAudioDecompress.h"
#include "UnAudioEffect.h"
#include "XAudio2Device.h"
#include "XAudio2Effects.h"

/*------------------------------------------------------------------------------------
	Local static helpers.
------------------------------------------------------------------------------------*/

#if _XBOX
#include <xmp.h>
#endif

// For muting user soundtracks during cinematics
FXMPHelper XMPHelper;
FXMPHelper* FXMPHelper::GetXMPHelper( void ) 
{ 
	return( &XMPHelper ); 
}

// For calculating spatialised volumes
FSpatializationHelper SpatializationHelper;

/*------------------------------------------------------------------------------------
	FXAudio2SoundSource.
------------------------------------------------------------------------------------*/

/**
 * Simple constructor
 */
FXAudio2SoundSource::FXAudio2SoundSource( UAudioDevice* InAudioDevice, FAudioEffectsManager* InEffects )
:	FSoundSource( InAudioDevice ),
	Source( NULL ),
	Buffer( NULL ),
	CurrentBuffer( 0 ),
	bBuffersToFlush( FALSE )
{
	AudioDevice = ( UXAudio2Device* )InAudioDevice;
	check( AudioDevice );
	Effects = ( FXAudio2EffectsManager* )InEffects;
	check( Effects );

	Destinations[DEST_DRY] = NULL;
	Destinations[DEST_REVERB] = NULL;
}

/**
 * Destructor, cleaning up voice
 */
FXAudio2SoundSource::~FXAudio2SoundSource( void )
{
	// If we're a streaming buffer...
	if( CurrentBuffer )
	{
		// ... free the buffers
		appFree( ( void* )XAudio2Buffers[0].pAudioData );
		appFree( ( void* )XAudio2Buffers[1].pAudioData );
	}

	// Release voice.
	if( Source )
	{
		Source->DestroyVoice();
	}
}

/** 
 * Submit the relevant audio buffers to the system
 */
void FXAudio2SoundSource::SubmitPCMBuffers( void )
{
	appMemzero( XAudio2Buffers, sizeof( XAUDIO2_BUFFER ) );

	CurrentBuffer = 0;

	XAudio2Buffers[0].pAudioData = Buffer->PCM.PCMData;
	XAudio2Buffers[0].AudioBytes = Buffer->PCM.PCMDataSize;

	if( WaveInstance->LoopingMode == LOOP_Forever )
	{
		XAudio2Buffers[0].LoopCount = 255;

		AudioDevice->ValidateAPICall( TEXT( "SubmitSourceBuffer - PCM - LOOP_Forever" ), 
			Source->SubmitSourceBuffer( XAudio2Buffers ) );
	}
	else if( WaveInstance->LoopingMode == LOOP_WithNotification )
	{
		AudioDevice->ValidateAPICall( TEXT( "SubmitSourceBuffer - PCM - LOOP_WithNotification" ), 
			Source->SubmitSourceBuffer( XAudio2Buffers ) );

		AudioDevice->ValidateAPICall( TEXT( "SubmitSourceBuffer - PCM - LOOP_WithNotification" ), 
			Source->SubmitSourceBuffer( XAudio2Buffers ) );	
	}
	else
	{
		XAudio2Buffers[0].Flags = XAUDIO2_END_OF_STREAM;

		AudioDevice->ValidateAPICall( TEXT( "SubmitSourceBuffer - PCM - LOOP_Never" ), 
			Source->SubmitSourceBuffer( XAudio2Buffers ) );
	}
}

/** 
 * Submit the relevant audio buffers to the system
 */
void FXAudio2SoundSource::SubmitPCMRTBuffers( void )
{
	appMemzero( XAudio2Buffers, sizeof( XAUDIO2_BUFFER ) * 2 );

	// Set the buffer to be in real time mode
	CurrentBuffer = 1;

	// Set up double buffer area to decompress to
	XAudio2Buffers[0].pAudioData = ( BYTE* )appMalloc( MONO_PCM_BUFFER_SIZE );
	XAudio2Buffers[0].AudioBytes = MONO_PCM_BUFFER_SIZE;

	Buffer->ReadCompressedData( ( BYTE* )XAudio2Buffers[0].pAudioData, WaveInstance->LoopingMode != LOOP_Never );

	AudioDevice->ValidateAPICall( TEXT( "SubmitSourceBuffer - PCMRT" ), 
		Source->SubmitSourceBuffer( XAudio2Buffers ) );

	XAudio2Buffers[1].pAudioData = ( BYTE* )appMalloc( MONO_PCM_BUFFER_SIZE );
	XAudio2Buffers[1].AudioBytes = MONO_PCM_BUFFER_SIZE;

	Buffer->ReadCompressedData( ( BYTE* )XAudio2Buffers[1].pAudioData, WaveInstance->LoopingMode != LOOP_Never );

	AudioDevice->ValidateAPICall( TEXT( "SubmitSourceBuffer - PCMRT" ), 
		Source->SubmitSourceBuffer( XAudio2Buffers + 1 ) );
}

/** 
 * Submit the relevant audio buffers to the system, accounting for looping modes
 */
void FXAudio2SoundSource::SubmitXMA2Buffers( void )
{
	appMemzero( XAudio2Buffers, sizeof( XAUDIO2_BUFFER ) );
	XAudio2Buffers[0].pAudioData = Buffer->XMA2.XMA2Data;
	XAudio2Buffers[0].AudioBytes = Buffer->XMA2.XMA2DataSize;

	CurrentBuffer = 0;

	// Deal with looping behavior.
	if( WaveInstance->LoopingMode == LOOP_Forever )
	{
		// Set to reserved "infinite" value.
		XAudio2Buffers[0].LoopCount = 255;
		XAudio2Buffers[0].LoopBegin = Buffer->XMA2.XMA2Format.LoopBegin;
		XAudio2Buffers[0].LoopLength = Buffer->XMA2.XMA2Format.LoopLength;

		AudioDevice->ValidateAPICall( TEXT( "SubmitSourceBuffer - XMA2 - LOOP_Forever" ), 
			Source->SubmitSourceBuffer( XAudio2Buffers ) );
	}
	else if( WaveInstance->LoopingMode == LOOP_WithNotification )
	{
		XAudio2Buffers[0].PlayBegin = Buffer->XMA2.XMA2Format.PlayBegin;
		XAudio2Buffers[0].PlayLength = Buffer->XMA2.XMA2Format.PlayLength;

		AudioDevice->ValidateAPICall( TEXT( "SubmitSourceBuffer - XMA2 - LOOP_WithNotification" ), 
			Source->SubmitSourceBuffer( XAudio2Buffers ) );
		AudioDevice->ValidateAPICall( TEXT( "SubmitSourceBuffer - XMA2 - LOOP_WithNotification" ), 
			Source->SubmitSourceBuffer( XAudio2Buffers ) );
	}
	else
	{
		// Regular sound source, don't loop.
		XAudio2Buffers[0].Flags = XAUDIO2_END_OF_STREAM;
		XAudio2Buffers[0].PlayBegin = Buffer->XMA2.XMA2Format.PlayBegin;
		XAudio2Buffers[0].PlayLength = Buffer->XMA2.XMA2Format.PlayLength;

		AudioDevice->ValidateAPICall( TEXT( "SubmitSourceBuffer - XMA2 - LOOP_Never" ), 
			Source->SubmitSourceBuffer( XAudio2Buffers ) );
	}
}

/** 
 * Submit the relevant audio buffers to the system
 */
void FXAudio2SoundSource::SubmitXWMABuffers( void )
{
	appMemzero( XAudio2Buffers, sizeof( XAUDIO2_BUFFER ) );
	appMemzero( XAudio2BufferXWMA, sizeof( XAUDIO2_BUFFER_WMA ) );

	CurrentBuffer = 0;

	// Regular sound source, don't loop.
	XAudio2Buffers[0].pAudioData = Buffer->XWMA.XWMAData;
	XAudio2Buffers[0].AudioBytes = Buffer->XWMA.XWMADataSize;

	XAudio2BufferXWMA[0].pDecodedPacketCumulativeBytes = Buffer->XWMA.XWMASeekData;
	XAudio2BufferXWMA[0].PacketCount = Buffer->XWMA.XWMASeekDataSize / sizeof( UINT32 );

	if( WaveInstance->LoopingMode == LOOP_Forever )
	{
		XAudio2Buffers[0].LoopCount = 255;

		AudioDevice->ValidateAPICall( TEXT( "SubmitSourceBuffer - XWMA - LOOP_Forever" ), 
			Source->SubmitSourceBuffer( XAudio2Buffers, XAudio2BufferXWMA ) );

	}
	else if( WaveInstance->LoopingMode == LOOP_WithNotification )
	{
		AudioDevice->ValidateAPICall( TEXT( "SubmitSourceBuffer - XWMA - LOOP_WithNotification" ), 
			Source->SubmitSourceBuffer( XAudio2Buffers, XAudio2BufferXWMA ) );

		AudioDevice->ValidateAPICall( TEXT( "SubmitSourceBuffer - XWMA - LOOP_WithNotification" ), 
			Source->SubmitSourceBuffer( XAudio2Buffers, XAudio2BufferXWMA ) );	
	}
	else
	{
		XAudio2Buffers[0].Flags = XAUDIO2_END_OF_STREAM;

		AudioDevice->ValidateAPICall( TEXT( "SubmitSourceBuffer - XWMA - LOOP_Never" ), 
			Source->SubmitSourceBuffer( XAudio2Buffers, XAudio2BufferXWMA ) );
	}
}

/**
 * Create a new source voice
 */
UBOOL FXAudio2SoundSource::CreateSource( void )
{
	// Create a source that goes to the spatialisation code and reverb effect
	Destinations[DEST_DRY] = Effects->DryPremasterVoice;
	if( IsEQFilterApplied() )
	{
		Destinations[DEST_DRY] = Effects->EQPremasterVoice;
	}

	INT NumSends = 1;
	if( bReverbApplied )
	{
		Destinations[DEST_REVERB] = Effects->ReverbEffectVoice;
		NumSends = 2;
	}

	const XAUDIO2_VOICE_SENDS SourceSendList =
	{
		NumSends, ( IXAudio2Voice** )Destinations
	};

	// Mark the source as music if it is a member of the music group and allow low, band and high pass filters
	UINT32 Flags = ( WaveInstance->bIsMusic ? XAUDIO2_VOICE_MUSIC : 0 ) | XAUDIO2_VOICE_USEFILTER;

	// All sound formats start with the WAVEFORMATEX structure (PCM, XMA2, XWMA)
	if( !AudioDevice->ValidateAPICall( TEXT( "CreateSourceVoice (source)" ), 
		AudioDevice->XAudio2->CreateSourceVoice( &Source, ( WAVEFORMATEX* )&Buffer->PCM, Flags, MAX_PITCH, NULL, &SourceSendList, NULL ) ) )
	{
		return( FALSE );
	}

	return( TRUE );
}

/**
 * Initializes a source with a given wave instance and prepares it for playback.
 *
 * @param	WaveInstance	wave instace being primed for playback
 * @return	TRUE if initialization was successful, FALSE otherwise
 */
UBOOL FXAudio2SoundSource::Init( FWaveInstance* InWaveInstance )
{
	// Find matching buffer.
	Buffer = FXAudio2SoundBuffer::Init( AudioDevice, InWaveInstance->WaveData );

	// Buffer failed to be created, or there was an error with the compressed data
	if( Buffer && Buffer->NumChannels > 0 )
	{
		SCOPE_CYCLE_COUNTER( STAT_AudioSourceInitTime );

		WaveInstance = InWaveInstance;

		// Set whether to apply reverb
		SetReverbApplied();

		// Release existing voice.
		if( Source )
		{
			//OPTME: Need to recycle these
			Source->DestroyVoice();
			Source = NULL;
		}

		// Create a new source
		CreateSource();

		// Submit audio buffers
		switch( Buffer->SoundFormat )
		{
		case SoundFormat_PCM:
		case SoundFormat_PCMPreview:
			SubmitPCMBuffers();
			break;

		case SoundFormat_PCMRT:
			SubmitPCMRTBuffers();
			break;

		case SoundFormat_XMA2:
			SubmitXMA2Buffers();
			break;

		case SoundFormat_XWMA:
			SubmitXWMABuffers();
			break;
		}

		// Updates the source which e.g. sets the pitch and volume.
		Update();
		
		// Initialization succeeded.
		return( TRUE );
	}

	// Initialization failed.
	return( FALSE );
}

/**
 * Calculates the volume for each channel
 */
void FXAudio2SoundSource::GetChannelVolumes( FLOAT ChannelVolumes[CHANNELOUT_COUNT], FLOAT AttenuatedVolume )
{
	switch( Buffer->NumChannels )
	{
	case 1:
		{
			// Calculate direction from listener to sound, where the sound is at the origin if unspatialised.
			FVector Direction = FVector( 0.0f, 0.0f, 0.0f );
			if( WaveInstance->bUseSpatialization )
			{
				Direction = AudioDevice->InverseTransform.TransformFVector( WaveInstance->Location ).SafeNormal();
			}

			// Calculate 5.1 channel volume.
			FVector OrientFront;
			OrientFront.X = 0.0f;
			OrientFront.Y = 0.0f;
			OrientFront.Z = 1.0f;

			FVector ListenerPosition;
			ListenerPosition.X = 0.0f;
			ListenerPosition.Y = 0.0f;
			ListenerPosition.Z = 0.0f;

			FVector EmitterPosition;
			EmitterPosition.X = Direction.Y;
			EmitterPosition.Y = Direction.X;
			EmitterPosition.Z = -Direction.Z;

			// Calculate 5.1 channel dolby surround rate/multipliers.
			ChannelVolumes[CHANNELOUT_FRONTLEFT] = AttenuatedVolume;
			ChannelVolumes[CHANNELOUT_FRONTRIGHT] = AttenuatedVolume;
			ChannelVolumes[CHANNELOUT_FRONTCENTER] = AttenuatedVolume;
			ChannelVolumes[CHANNELOUT_LEFTSURROUND] = AttenuatedVolume;
			ChannelVolumes[CHANNELOUT_RIGHTSURROUND] = AttenuatedVolume;

			if( bReverbApplied )
			{
				ChannelVolumes[CHANNELOUT_REVERB] = AttenuatedVolume;
			}

			SpatializationHelper.CalculateDolbySurroundRate( OrientFront, ListenerPosition, EmitterPosition, ChannelVolumes );

			ChannelVolumes[CHANNELOUT_LOWFREQUENCY] = AttenuatedVolume * 0.5f;
		}
		break;

	case 2:
		{
			// Stereo is always treated as unspatialized
			ChannelVolumes[CHANNELOUT_FRONTLEFT] = AttenuatedVolume;
			ChannelVolumes[CHANNELOUT_FRONTRIGHT] = AttenuatedVolume;
			// It is bled to the surrounds at 1/4 volume to convert from 2.0 channel to simulated 4.0 channel.
			if( bStereoBleed )
			{
				ChannelVolumes[CHANNELOUT_LEFTSURROUND] = AttenuatedVolume * 0.25f;
				ChannelVolumes[CHANNELOUT_RIGHTSURROUND] = AttenuatedVolume * 0.25f;
			}

			if( bReverbApplied )
			{
				ChannelVolumes[CHANNELOUT_REVERB] = AttenuatedVolume;
			}
		}
		break;

	case 4:
		{
			ChannelVolumes[CHANNELOUT_FRONTLEFT] = AttenuatedVolume;
			ChannelVolumes[CHANNELOUT_FRONTRIGHT] = AttenuatedVolume;
			ChannelVolumes[CHANNELOUT_LEFTSURROUND] = AttenuatedVolume;
			ChannelVolumes[CHANNELOUT_RIGHTSURROUND] = AttenuatedVolume;
		}
		break;

	case 6:
		{
			ChannelVolumes[CHANNELOUT_FRONTLEFT] = AttenuatedVolume;
			ChannelVolumes[CHANNELOUT_FRONTRIGHT] = AttenuatedVolume;
			ChannelVolumes[CHANNELOUT_FRONTCENTER] = AttenuatedVolume;
			ChannelVolumes[CHANNELOUT_LOWFREQUENCY] = AttenuatedVolume;
			ChannelVolumes[CHANNELOUT_LEFTSURROUND] = AttenuatedVolume;
			ChannelVolumes[CHANNELOUT_RIGHTSURROUND] = AttenuatedVolume;
		}
		break;

	default:
		break;
	};

	// Apply any debug settings
	FLOAT DryVolume = 1.0f;
	FLOAT ReverbVolume = 1.0f;

	Effects->GetVolumes( DryVolume, ReverbVolume );

	for( INT i = 0; i < XAUDIO2_SPEAKER_COUNT; i++ )
	{
		ChannelVolumes[i] *= DryVolume;
	}

	ChannelVolumes[CHANNELOUT_REVERB] *= ReverbVolume;
}

/** 
 * Maps a sound with a given number of channels to to expected speakers
 */
void FXAudio2SoundSource::RouteDryToSpeakers( FLOAT ChannelVolumes[CHANNELOUT_COUNT] )
{
	// Only need to account to the special cases that are not a simple match of channel to speaker
	switch( Buffer->NumChannels )
	{
	case 1:
		{
			// Spatialised audio maps 1 channel to 6 speakers
			FLOAT SpatialisationMatrix[XAUDIO2_SPEAKER_COUNT * 1] = 
			{			
				ChannelVolumes[CHANNELOUT_FRONTLEFT],
				ChannelVolumes[CHANNELOUT_FRONTRIGHT],
				ChannelVolumes[CHANNELOUT_FRONTCENTER],
				ChannelVolumes[CHANNELOUT_LOWFREQUENCY],
				ChannelVolumes[CHANNELOUT_LEFTSURROUND],
				ChannelVolumes[CHANNELOUT_RIGHTSURROUND]
			};

			// Update the dry output to the mastering voice
			AudioDevice->ValidateAPICall( TEXT( "SetOutputMatrix (mono)" ), 
				Source->SetOutputMatrix( Destinations[DEST_DRY], 1, XAUDIO2_SPEAKER_COUNT, SpatialisationMatrix ) );
		}
		break;

	case 2:
		{
			FLOAT SpatialisationMatrix[XAUDIO2_SPEAKER_COUNT * 2] = 
			{ 
				ChannelVolumes[CHANNELOUT_FRONTLEFT], 0.0f,
				0.0f, ChannelVolumes[CHANNELOUT_FRONTRIGHT],
				0.0f, 0.0f,
				0.0f, 0.0f,
				ChannelVolumes[CHANNELOUT_LEFTSURROUND], 0.0f,
				0.0f, ChannelVolumes[CHANNELOUT_RIGHTSURROUND]
			};

			// Stereo sounds map 2 channels to 6 speakers
			AudioDevice->ValidateAPICall( TEXT( "SetOutputMatrix (stereo)" ), 
				Source->SetOutputMatrix( Destinations[DEST_DRY], 2, XAUDIO2_SPEAKER_COUNT, SpatialisationMatrix ) );
		}
		break;

	case 4:
		{
			FLOAT SpatialisationMatrix[XAUDIO2_SPEAKER_COUNT * 4] = 
			{ 
				ChannelVolumes[CHANNELOUT_FRONTLEFT], 0.0f, 0.0f, 0.0f,
				0.0f, ChannelVolumes[CHANNELOUT_FRONTRIGHT], 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, ChannelVolumes[CHANNELOUT_LEFTSURROUND], 0.0f,
				0.0f, 0.0f, 0.0f, ChannelVolumes[CHANNELOUT_RIGHTSURROUND]
			};

			// Quad sounds map 4 channels to 6 speakers
			AudioDevice->ValidateAPICall( TEXT( "SetOutputMatrix (4 channel)" ), 
				Source->SetOutputMatrix( Destinations[DEST_DRY], 4, XAUDIO2_SPEAKER_COUNT, SpatialisationMatrix ) );
		}
		break;

	case 6:
		{
			FLOAT SpatialisationMatrix[XAUDIO2_SPEAKER_COUNT * 6] = 
			{
				ChannelVolumes[CHANNELOUT_FRONTLEFT], 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 
				0.0f, ChannelVolumes[CHANNELOUT_FRONTRIGHT], 0.0f, 0.0f, 0.0f, 0.0f, 
				0.0f, 0.0f, ChannelVolumes[CHANNELOUT_FRONTCENTER], 0.0f, 0.0f, 0.0f, 
				0.0f, 0.0f, 0.0f, ChannelVolumes[CHANNELOUT_LOWFREQUENCY], 0.0f, 0.0f, 
				0.0f, 0.0f, 0.0f, 0.0f,ChannelVolumes[CHANNELOUT_LEFTSURROUND],  0.0f, 
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, ChannelVolumes[CHANNELOUT_RIGHTSURROUND]
			};

			// 5.1 sounds map 6 channels to 6 speakers
			AudioDevice->ValidateAPICall( TEXT( "SetOutputMatrix (6 channel)" ), 
				Source->SetOutputMatrix( Destinations[DEST_DRY], 6, XAUDIO2_SPEAKER_COUNT, SpatialisationMatrix ) );
		}
		break;

	default:
		break;
	};
}

/** 
 * Maps the sound to the relevant reverb effect
 */
void FXAudio2SoundSource::RouteToReverb( FLOAT ChannelVolumes[CHANNELOUT_COUNT] )
{
	switch( Buffer->NumChannels )
	{
	case 1:
		{
			FLOAT SpatialisationMatrix[2] = 
			{			
				ChannelVolumes[CHANNELOUT_REVERB],
				ChannelVolumes[CHANNELOUT_REVERB],
			};

			// Update the dry output to the mastering voice
			AudioDevice->ValidateAPICall( TEXT( "SetOutputMatrix (Mono reverb)" ), 
				Source->SetOutputMatrix( Destinations[DEST_REVERB], 1, 2, SpatialisationMatrix ) );
		}
		break;

	case 2:
		{
			FLOAT SpatialisationMatrix[4] = 
			{ 
				ChannelVolumes[CHANNELOUT_REVERB], 0.0f,
				0.0f, ChannelVolumes[CHANNELOUT_REVERB]
			};

			// Stereo sounds map 2 channels to 6 speakers
			AudioDevice->ValidateAPICall( TEXT( "SetOutputMatrix (Stereo reverb)" ), 
				Source->SetOutputMatrix( Destinations[DEST_REVERB], 2, 2, SpatialisationMatrix ) );
		}
		break;
	}
}

/**
 * Updates the source specific parameter like e.g. volume and pitch based on the associated
 * wave instance.	
 */
void FXAudio2SoundSource::Update( void )
{
	SCOPE_CYCLE_COUNTER( STAT_AudioUpdateSources );

	if( !WaveInstance || Paused )
	{	
		return;
	}

	// Clamp to valid values in case the hardware doesn't like strange values
	FLOAT Volume = Clamp<FLOAT>( WaveInstance->Volume * WaveInstance->VolumeMultiplier * GVolumeMultiplier, 0.0f, 1.0f );

	const FLOAT Pitch = Clamp<FLOAT>( WaveInstance->Pitch, MIN_PITCH, MAX_PITCH );
	AudioDevice->ValidateAPICall( TEXT( "SetFrequencyRatio" ), 
		Source->SetFrequencyRatio( Pitch ) );

	// Set the HighFrequencyGain value (aka low pass filter setting)
	SetHighFrequencyGain();

	// Apply the low pass filter
	XAUDIO2_FILTER_PARAMETERS LPFParameters = { LowPassFilter, 1.0f, 1.0f };
	if( HighFrequencyGain < 1.0f - KINDA_SMALL_NUMBER )
	{
		FLOAT FilterConstant = 2.0f * appSin( PI * 6000.0f * HighFrequencyGain / 48000.0f );
		LPFParameters.Frequency = FilterConstant;
		LPFParameters.OneOverQ = 0.9f;
	}

	AudioDevice->ValidateAPICall( TEXT( "SetFilterParameters" ), 
		Source->SetFilterParameters( &LPFParameters ) );

	// Initialize channel volumes
	FLOAT ChannelVolumes[CHANNELOUT_COUNT] = { 0.0f };
	GetChannelVolumes( ChannelVolumes, Volume );

	RouteDryToSpeakers( ChannelVolumes );

	if( bReverbApplied )
	{
		RouteToReverb( ChannelVolumes );
	}
}

/**
 * Plays the current wave instance.	
 */
void FXAudio2SoundSource::Play( void )
{
	if( WaveInstance )
	{
		if( !Playing )
		{
			if( Buffer->NumChannels >= XAUDIO2_SPEAKER_COUNT )
			{
				XMPHelper.CinematicAudioStarted();
			}
		}

		AudioDevice->ValidateAPICall( TEXT( "Start" ), 
			Source->Start( 0 ) );
		Paused = FALSE;
		Playing = TRUE;
		bBuffersToFlush = FALSE;
	}
}

/**
 * Stops the current wave instance and detaches it from the source.	
 */
void FXAudio2SoundSource::Stop( void )
{
	if( WaveInstance )
	{	
		if( Playing )
		{
			if( Buffer->NumChannels >= XAUDIO2_SPEAKER_COUNT )
			{
				XMPHelper.CinematicAudioStopped();
			}
		}

		AudioDevice->ValidateAPICall( TEXT( "Stop" ), 
			Source->Stop( 0 ) );
		Paused = FALSE;
		Playing = FALSE;
		Buffer = NULL;
		bBuffersToFlush = FALSE;
	}

	FSoundSource::Stop();
}

/**
 * Pauses playback of current wave instance.
 */
void FXAudio2SoundSource::Pause( void )
{
	if( WaveInstance )
	{
		AudioDevice->ValidateAPICall( TEXT( "Stop" ), 
			Source->Stop( XAUDIO2_PLAY_TAILS ) );
		Paused = TRUE;
	}
}

/**
 * Handles feeding new data to a real time decompressed sound
 */
void FXAudio2SoundSource::HandleRealTimeSource( void )
{
	// Update the double buffer toggle
	CurrentBuffer++;

	// Get the next bit of streaming data
	UBOOL bLooped = Buffer->ReadCompressedData( ( BYTE* )XAudio2Buffers[CurrentBuffer & 1].pAudioData, WaveInstance->LoopingMode != LOOP_Never );
	
	// Have we reached the end of the compressed sound?
	if( bLooped )
	{
		switch( WaveInstance->LoopingMode )
		{
		case LOOP_Never:
			// Play out any queued buffers - once there are no buffers left, the state check at the beginning of IsFinished will fire
			bBuffersToFlush = TRUE;
			XAudio2Buffers[CurrentBuffer & 1].Flags |= XAUDIO2_END_OF_STREAM;
			break;

		case LOOP_WithNotification:
			// If we have just looped, and we are programmatically looping, send notification
			WaveInstance->NotifyFinished();
			break;

		case LOOP_Forever:
			// Let the sound loop indefinitely
			break;
		}
	}
}

/**
 * Queries the status of the currently associated wave instance.
 *
 * @return	TRUE if the wave instance/ source has finished playback and FALSE if it is 
 *			currently playing or paused.
 */
UBOOL FXAudio2SoundSource::IsFinished( void )
{
	// A paused source is not finished.
	if( Paused )
	{
		return( FALSE );
	}

	if( WaveInstance )
	{
		// Retrieve state source is in.
		XAUDIO2_VOICE_STATE SourceState;
		Source->GetState( &SourceState );

		if( SourceState.BuffersQueued == 0 )
		{
			// Notify the wave instance that it has finished playing.
			WaveInstance->NotifyFinished();
			return( TRUE );
		}

		// Early out for non looping and non realtime sounds
		if( !CurrentBuffer && WaveInstance->LoopingMode == LOOP_Never )
		{
			return( FALSE );
		}

		if( SourceState.BuffersQueued == 1 )
		{
			// Feed in new data if required
			switch( Buffer->SoundFormat )
			{
			case SoundFormat_PCMRT:
				// Continue feeding new sound data (unless we are waiting for the sound to finish)
				if( !bBuffersToFlush )
				{
					HandleRealTimeSource();

					AudioDevice->ValidateAPICall( TEXT( "SubmitSourceBuffer - IsFinished" ), 
						Source->SubmitSourceBuffer( &XAudio2Buffers[CurrentBuffer & 1] ) );
				}
				break;

			case SoundFormat_PCM:
			case SoundFormat_PCMPreview:
			case SoundFormat_XMA2:
				AudioDevice->ValidateAPICall( TEXT( "SubmitSourceBuffer - IsFinished" ), 
					Source->SubmitSourceBuffer( XAudio2Buffers ) );
				break;

			case SoundFormat_XWMA:
				AudioDevice->ValidateAPICall( TEXT( "SubmitSourceBuffer - IsFinished" ), 
					Source->SubmitSourceBuffer( XAudio2Buffers, XAudio2BufferXWMA ) );
				break;
			}

			if( !CurrentBuffer && WaveInstance->LoopingMode == LOOP_WithNotification )
			{
				// Notify the wave instance that the current buffer has finished playing.
				WaveInstance->NotifyFinished();
			}
		}

		return( FALSE );
	}

	return( TRUE );
}


/*------------------------------------------------------------------------------------
	FSpatializationHelper.
------------------------------------------------------------------------------------*/

/**
 * Constructor, initializing all member variables.
 */
FSpatializationHelper::FSpatializationHelper( void )
{
	// Initialize X3DAudio
	//
	//  Speaker geometry configuration on the final mix, specifies assignment of channels
	//  to speaker positions, defined as per WAVEFORMATEXTENSIBLE.dwChannelMask
	X3DAudioInitialize( SPEAKER_5POINT1, X3DAUDIO_SPEED_OF_SOUND, X3DInstance );

	// Initialize 3D audio parameters
	X3DAUDIO_VECTOR ZeroVector = { 0.0f, 0.0f, 0.0f };
	EmitterAzimuths = 0.0f;

	// Set up listener parameters
	Listener.OrientFront.x = 0.0f;
	Listener.OrientFront.y = 0.0f;
	Listener.OrientFront.z = 1.0f;
	Listener.OrientTop.x = 0.0f;
	Listener.OrientTop.y = 1.0f;
	Listener.OrientTop.z = 0.0f;
	Listener.Position.x	= 0.0f;
	Listener.Position.y = 0.0f;
	Listener.Position.z = 0.0f;
	Listener.Velocity = ZeroVector;

	// Set up emitter parameters
	Emitter.OrientFront.x = 0.0f;
	Emitter.OrientFront.y = 0.0f;
	Emitter.OrientFront.z = 1.0f;
	Emitter.OrientTop.x = 0.0f;
	Emitter.OrientTop.y = 1.0f;
	Emitter.OrientTop.z = 0.0f;
	Emitter.Position = ZeroVector;
	Emitter.Velocity = ZeroVector;
	Emitter.pCone = &Cone;
	Emitter.pCone->InnerAngle = 0.0f; 
	Emitter.pCone->OuterAngle = 0.0f;
	Emitter.pCone->InnerVolume = 0.0f;
	Emitter.pCone->OuterVolume = 1.0f;
	Emitter.pCone->InnerLPF = 0.0f;
	Emitter.pCone->OuterLPF = 1.0f;
	Emitter.pCone->InnerReverb = 0.0f;
	Emitter.pCone->OuterReverb = 1.0f;

	Emitter.ChannelCount = 1;
	Emitter.ChannelRadius = 0.0f;
	Emitter.pChannelAzimuths = &EmitterAzimuths;

	// real volume -> 5.1-ch rate
	VolumeCurvePoint[0].Distance = 0.0f;
	VolumeCurvePoint[0].DSPSetting = 1.0f;
	VolumeCurvePoint[1].Distance = 1.0f;
	VolumeCurvePoint[1].DSPSetting = 1.0f;
	VolumeCurve.PointCount = ARRAY_COUNT( VolumeCurvePoint );
	VolumeCurve.pPoints = VolumeCurvePoint;

	ReverbVolumeCurvePoint[0].Distance = 0.0f;
	ReverbVolumeCurvePoint[0].DSPSetting = 0.5f;
	ReverbVolumeCurvePoint[1].Distance = 1.0f;
	ReverbVolumeCurvePoint[1].DSPSetting = 0.5f;
	ReverbVolumeCurve.PointCount = ARRAY_COUNT( ReverbVolumeCurvePoint );
	ReverbVolumeCurve.pPoints = ReverbVolumeCurvePoint;

	Emitter.pVolumeCurve = &VolumeCurve;
	Emitter.pLFECurve = NULL;
	Emitter.pLPFDirectCurve = NULL;
	Emitter.pLPFReverbCurve = NULL;
	Emitter.pReverbCurve = &ReverbVolumeCurve;
	Emitter.CurveDistanceScaler = 1.0f;
	Emitter.DopplerScaler = 1.0f;

	DSPSettings.SrcChannelCount = 1;
	DSPSettings.DstChannelCount = XAUDIO2_SPEAKER_COUNT;
	DSPSettings.pMatrixCoefficients = MatrixCoefficients;
}

/**
 * Calculates the spatialized volumes for each channel.
 *
 * @param	OrientFront				The listener's facing direction.
 * @param	ListenerPosition		The position of the listener.
 * @param	EmitterPosition			The position of the emitter.
 * @param	OutVolumes				An array of floats with one volume for each output channel.
 * @param	OutReverbLevel			The reverb volume
 */
void FSpatializationHelper::CalculateDolbySurroundRate( const FVector& OrientFront, const FVector& ListenerPosition, const FVector& EmitterPosition, FLOAT* OutVolumes  )
{
	DWORD CalculateFlags = X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_REVERB;

	Listener.OrientFront.x = OrientFront.X;
	Listener.OrientFront.y = OrientFront.Y;
	Listener.OrientFront.z = OrientFront.Z;
	Listener.Position.x = ListenerPosition.X;
	Listener.Position.y = ListenerPosition.Y;
	Listener.Position.z = ListenerPosition.Z;
	Emitter.Position.x = EmitterPosition.X;
	Emitter.Position.y = EmitterPosition.Y;
	Emitter.Position.z = EmitterPosition.Z;

	X3DAudioCalculate( X3DInstance, &Listener, &Emitter, CalculateFlags, &DSPSettings );

	for( INT SpeakerIndex = 0; SpeakerIndex < XAUDIO2_SPEAKER_COUNT; SpeakerIndex++ )
	{
		OutVolumes[SpeakerIndex] *= DSPSettings.pMatrixCoefficients[SpeakerIndex];
	}

	OutVolumes[CHANNELOUT_REVERB] *= DSPSettings.ReverbLevel;
}

/*------------------------------------------------------------------------------------
	FXMPHelper.
------------------------------------------------------------------------------------*/

/**
 * FXMPHelper constructor - Protected.
 */
FXMPHelper::FXMPHelper( void )
	: CinematicAudioCount( 0 )
, MoviePlaying( FALSE )
, XMPEnabled( TRUE )
, XMPBlocked( FALSE )
{
}

/**
 * FXMPHelper destructor.
 */
FXMPHelper::~FXMPHelper( void )
{
}

/**
 * Records that a cinematic audio track has started playing.
 */
void FXMPHelper::CinematicAudioStarted( void )
{
	check( CinematicAudioCount >= 0 );
	CinematicAudioCount++;
	CountsUpdated();
}

/**
 * Records that a cinematic audio track has stopped playing.
 */
void FXMPHelper::CinematicAudioStopped( void )
{
	check( CinematicAudioCount > 0 );
	CinematicAudioCount--;
	CountsUpdated();
}

/**
 * Records that a cinematic audio track has started playing.
 */
void FXMPHelper::MovieStarted( void )
{
	MoviePlaying = TRUE;
	CountsUpdated();
}

/**
 * Records that a cinematic audio track has stopped playing.
 */
void FXMPHelper::MovieStopped( void )
{
	MoviePlaying = FALSE;
	CountsUpdated();
}

/**
 * Called every frame to update XMP status if necessary
 */
void FXMPHelper::CountsUpdated( void )
{
	if( XMPEnabled )
	{
		if( ( MoviePlaying == TRUE ) || ( CinematicAudioCount > 0 ) )
		{
#if _XBOX
			XMPOverrideBackgroundMusic();
#endif
			XMPEnabled = FALSE;
		}
	}
	else
	{
		if( ( MoviePlaying == FALSE ) && ( CinematicAudioCount == 0 ) )
		{
#if _XBOX
			XMPRestoreBackgroundMusic();
#endif
			XMPEnabled = TRUE;
		}
	}
}

// end
