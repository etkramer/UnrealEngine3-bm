/*=============================================================================
	XeAudioDevice.cpp: Unreal XAudio Audio interface object.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.

	Unreal is RHS with Y and Z swapped (or technically LHS with flipped axis)

=============================================================================*/

/*------------------------------------------------------------------------------------
	Audio includes.
------------------------------------------------------------------------------------*/
#include "Engine.h"
#include "EngineSoundClasses.h"
#include "EngineAudioDeviceClasses.h"
#include "UnAudioEffect.h"
#include "XeAudioDevice.h"
#include "xmp.h"

/*------------------------------------------------------------------------------------
	Local static helpers.
------------------------------------------------------------------------------------*/

FXeXMPHelper			XMPHelper;
FXeSpatializationHelper	SpatializationHelper;

FXeXMPHelper* FXeXMPHelper::GetXMPHelper( void )	{ return( &XMPHelper ); }

/*------------------------------------------------------------------------------------
	UXeAudioDevice constructor and UObject interface.
------------------------------------------------------------------------------------*/

IMPLEMENT_CLASS( UXeAudioDevice );

/**
 * Static constructor, used to associate .ini options with member varialbes.	
 */
void UXeAudioDevice::StaticConstructor( void )
{
}

/**
 * Tears down audio device by stopping all sounds, removing all buffers, 
 * destroying all sources, ... Called by both Destroy and ShutdownAfterError
 * to perform the actual tear down.
 */
void UXeAudioDevice::Teardown( void )
{
	// Flush stops all sources and deletes all buffers so sources can be safely deleted below.
	Flush( NULL );

	// Clear out the EQ/Reverb/LPF effects
	delete Effects;

	for( INT i = 0; i < Sources.Num(); i++ )
	{
		delete Sources( i );
	}

	Sources.Empty();
	FreeSources.Empty();

	XAudioShutDown();
}

/**
 * Shuts down audio device. This will never be called with the memory image
 * codepath.
 */
void UXeAudioDevice::FinishDestroy( void )
{
	if( !HasAnyFlags( RF_ClassDefaultObject ) )
	{
		Teardown();
		debugf( NAME_Exit, TEXT( "XeAudio Device shut down." ) );
	}

	Super::FinishDestroy();
}

/**
 * Special variant of Destroy that gets called on fatal exit. Doesn't really
 * matter on the console so for now is just the same as Destroy so we can
 * verify that the code correctly cleans up everything.
 */
void UXeAudioDevice::ShutdownAfterError( void )
{
	if( !HasAnyFlags( RF_ClassDefaultObject ) )
	{
		//	Teardown(); //@todo xenon: commented out as it just creates a lot of log spam
		debugf( NAME_Exit, TEXT( "UXeAudioDevice::ShutdownAfterError" ) );
	}
	Super::ShutdownAfterError();
}

/*------------------------------------------------------------------------------------
	UAudioDevice Interface.
------------------------------------------------------------------------------------*/

/**
 * Initializes the audio device and creates sources.
 *
 * @warning: Relies on XAudioInitialize already being called
 *
 * @return TRUE if initialization was successful, FALSE otherwise
 */
UBOOL UXeAudioDevice::Init( void )
{
	// Make sure no interface classes contain any garbage
	Effects = NULL;

	// No channels, no sound.
	if( MaxChannels < 1 )
	{	
		return( FALSE );
	}

	// Check for and init EFX extensions - must be on for xbox
	Effects = new FXeAudioEffectsManager( this );

	// Initialize channels.
	for( INT i = 0; i < Min( MaxChannels, MAX_AUDIOCHANNELS ); i++ )
	{
		FXeSoundSource* Source = new FXeSoundSource( this );
		Source->Voice = NULL;
		Sources.AddItem( Source );
		FreeSources.AddItem( Source );
	}

	if( !Sources.Num() )
	{
		debugf( NAME_Error, TEXT( "XeAudioDevice: couldn't allocate sources" ) );
		return FALSE;
	}

	// Update MaxChannels in case we couldn't create enough sources.
	MaxChannels = Sources.Num();
	debugf( TEXT( "XeAudioDevice: Allocated %i sources" ), MaxChannels );

	// Initialize permanent memory stack for initial & always loaded sound allocations.
	if( CommonAudioPoolSize )
	{
		debugf( TEXT( "XeAudioDevice: Allocating %g MByte for always resident audio data" ), CommonAudioPoolSize / ( 1024.0f * 1024.0f ) );
		CommonAudioPoolFreeBytes = CommonAudioPoolSize;
		CommonAudioPool = ( BYTE* )appPhysicalAlloc( CommonAudioPoolSize, CACHE_Normal );
	}
	else
	{
		debugf( TEXT( "XeAudioDevice: CommonAudioPoolSize is set to 0 - disabling persistent pool for audio data" ) );
		CommonAudioPoolFreeBytes = 0;
	}

	// Initialized.
	NextResourceID = 1;

	// Initialize base class last as it's going to precache already loaded audio.
	Super::Init();

	return( TRUE );
}

/**
 * Update the audio device and calculates the cached inverse transform later
 * on used for spatialization.
 *
 * @param	Realtime	whether we are paused or not
 */
void UXeAudioDevice::Update( UBOOL bRealtime )
{
	Super::Update( bRealtime );

	// Caches the matrix used to transform a sounds position into local space so we can just look
	// at the Y component after normalization to determine spatialization.
	InverseTransform = FMatrix( Listeners( 0 ).Up, Listeners( 0 ).Right, Listeners( 0 ).Up ^ Listeners( 0 ).Right, Listeners( 0 ).Location ).Inverse();

	// Print statistics for first non initial load allocation.
	static UBOOL bFirstTime = TRUE;
	if( bFirstTime && CommonAudioPoolSize != 0 )
	{
		bFirstTime = FALSE;
		if( CommonAudioPoolFreeBytes != 0 )
		{
			debugf( TEXT( "XeAudio: Audio pool size mismatch by %d bytes. Please update CommonAudioPoolSize ini setting to %d to avoid waste!" ),
									CommonAudioPoolFreeBytes, CommonAudioPoolSize - CommonAudioPoolFreeBytes );
		}
	}
}

/**
 * Precaches the passed in sound node wave object.
 *
 * @param	SoundNodeWave	Resource to be precached.
 */
void UXeAudioDevice::Precache( USoundNodeWave* SoundNodeWave )
{
	FXeSoundBuffer::Init( SoundNodeWave, this );

	// Size of the decompressed data
	INC_DWORD_STAT_BY( STAT_AudioMemorySize, SoundNodeWave->CompressedXbox360Data.GetBulkDataSize() );
	INC_DWORD_STAT_BY( STAT_AudioMemory, SoundNodeWave->CompressedXbox360Data.GetBulkDataSize() );
}

/**
 * Frees the bulk resource data assocated with this SoundNodeWave.
 *
 * @param	SoundNodeWave	wave object to free associated bulk data
 */
void UXeAudioDevice::FreeResource( USoundNodeWave* SoundNodeWave )
{
	// Find buffer for resident wavs
	if( SoundNodeWave->ResourceID )
	{
		// Find buffer associated with resource id.
		FXeSoundBuffer* Buffer = WaveBufferMap.FindRef( SoundNodeWave->ResourceID );
		if( Buffer )
		{
			// Remove from buffers array.
			Buffers.RemoveItem( Buffer );

			// See if it is being used by a sound source...
			UBOOL bWasReferencedBySource = FALSE;
			for( INT SrcIndex = 0; SrcIndex < Sources.Num(); SrcIndex++ )
			{
				FXeSoundSource* Src = ( FXeSoundSource* )( Sources( SrcIndex ) );
				if( Src && Src->Buffer && ( Src->Buffer == Buffer ) )
				{
					// Make sure the buffer is no longer referenced by anything
					Src->Stop();
					break;
				}
			}

			// Delete it. This will automatically remove itself from the WaveBufferMap.
			delete Buffer;
		}

		SoundNodeWave->ResourceID = 0;
	}

	// Stat housekeeping
	DEC_DWORD_STAT_BY( STAT_AudioMemorySize, SoundNodeWave->CompressedXbox360Data.GetBulkDataSize() );
	DEC_DWORD_STAT_BY( STAT_AudioMemory, SoundNodeWave->CompressedXbox360Data.GetBulkDataSize() );
}

// sort memory usage from large to small unless bAlphaSort
static UBOOL bAlphaSort = FALSE;
IMPLEMENT_COMPARE_POINTER( FXeSoundBuffer, XeAudioDevice, { return bAlphaSort ? appStricmp(*A->ResourceName,*B->ResourceName) : ( A->GetSize() > B->GetSize() ) ? -1 : 1; } );

/** 
 * Displays debug information about the loaded sounds
 *
 * @NOTE:  if you update this make certain you update PS3AudioDevice.cpp ListSounds also
 */
void UXeAudioDevice::ListSounds( const TCHAR* Cmd, FOutputDevice& Ar )
{
	bAlphaSort = ParseParam( Cmd, TEXT("ALPHASORT") );

	INT	TotalResident = 0;
	INT	ResidentCount = 0;

	Ar.Logf( TEXT( ",Size Kb,NumChannels,SoundName,bAllocationInPermanentPool" ) );

	TArray<FXeSoundBuffer*> AllSounds;
	for( INT BufferIndex = 0; BufferIndex < Buffers.Num(); BufferIndex++ )
	{
		AllSounds.AddItem( Buffers( BufferIndex ) );
	}

	Sort<USE_COMPARE_POINTER( FXeSoundBuffer, XeAudioDevice )>( &AllSounds( 0 ), AllSounds.Num() );

	for( INT i = 0; i < AllSounds.Num(); ++i )
	{
		FXeSoundBuffer* Buffer = AllSounds( i );

		Ar.Logf( TEXT( ",%8.2f, %d channel(s), %s, %d" ), Buffer->GetSize() / 1024.0f, Buffer->NumChannels, *Buffer->ResourceName, Buffer->bAllocationInPermanentPool );
		TotalResident += Buffer->GetSize();
		ResidentCount++;
	}

	Ar.Logf( TEXT( "%8.2f Kb for %d resident sounds" ), TotalResident / 1024.0f, ResidentCount );
}

/**
 * Exec handler used to parse console commands.
 *
 * @param	Cmd		Command to parse
 * @param	Ar		Output device to use in case the handler prints anything
 * @return	TRUE if command was handled, FALSE otherwise
 */
UBOOL UXeAudioDevice::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	if( Super::Exec( Cmd, Ar ) )
	{
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("ISOLATERADIO")) )
	{
		if( Effects )
		{
			Effects->SetMixDebugState( DEBUGSTATE_IsolateRadio );
		}
		return TRUE;
	}

	return FALSE;
}

/**
 * Allocates memory from permanent pool. This memory will NEVER be freed.
 *
 * @param	Size	Size of allocation.
 *
 * @return pointer to a chunk of memory with size Size
 */
void* UXeAudioDevice::AllocatePermanentMemory( INT Size )
{
	void* Allocation = NULL;
	
	// Fall back to using regular allocator if there is not enough space in permanent memory pool.
	if( Size > CommonAudioPoolFreeBytes )
	{
		Allocation = appPhysicalAlloc( Size, CACHE_Normal );
		check( Allocation );
	}
	// Allocate memory from pool.
	else
	{
		BYTE* CommonAudioPoolAddress = ( BYTE* )CommonAudioPool;
		Allocation = CommonAudioPoolAddress + ( CommonAudioPoolSize - CommonAudioPoolFreeBytes );
	}
	// Decrement available size regardless of whether we allocated from pool or used regular allocator
	// to allow us to log suggested size at the end of initial loading.
	CommonAudioPoolFreeBytes -= Size;
	
	return( Allocation );
}

/*------------------------------------------------------------------------------------
	FXeSoundSource.
------------------------------------------------------------------------------------*/

/**
 * Destructor, cleaning up voice
 */
FXeSoundSource::~FXeSoundSource( void )
{
	// Release voice.
	if( Voice )
	{
		Voice->Release();
		Voice = NULL;
	}
}

/** 
 * Maps a sound with a given number of channels to to expected speakers
 */
void FXeSoundSource::MapSpeakers( XAUDIOCHANNELMAPENTRY* Entries )
{
	for( INT ChannelIndex = 0; ChannelIndex < CHANNELOUT_COUNT; ChannelIndex++ )
	{
		switch( Buffer->NumChannels )
		{
		case 1:
			// Mono file
			Entries[ChannelIndex].InputChannel = 0;
			break;

		case 2:
			// Map right channel to FR and BR speakers
			switch( ChannelIndex )
			{
			default:
			case XAUDIOSPEAKER_FRONTLEFT:
			case XAUDIOSPEAKER_BACKLEFT:
			case XAUDIOSPEAKER_FRONTCENTER:
			case XAUDIOSPEAKER_LOWFREQUENCY:
				Entries[ChannelIndex].InputChannel = 0;
				break;

			case XAUDIOSPEAKER_FRONTRIGHT:
			case XAUDIOSPEAKER_BACKRIGHT:
				Entries[ChannelIndex].InputChannel = 1;
				break;
			}
			break;

		case 4:
			// Map 4.0 sound to correct speakers
			switch( ChannelIndex )
			{
			default:
			case XAUDIOSPEAKER_FRONTLEFT:
			case XAUDIOSPEAKER_FRONTCENTER:
			case XAUDIOSPEAKER_LOWFREQUENCY:
				Entries[ChannelIndex].InputChannel = 0;
				break;

			case XAUDIOSPEAKER_FRONTRIGHT:
				Entries[ChannelIndex].InputChannel = 1;
				break;

			case XAUDIOSPEAKER_BACKLEFT:
				Entries[ChannelIndex].InputChannel = 2;
				break;

			case XAUDIOSPEAKER_BACKRIGHT:
				Entries[ChannelIndex].InputChannel = 3;
				break;
			}
			break;

		case 6:
			// Map 5.1 sound
			switch( ChannelIndex )
			{
			default:
			case XAUDIOSPEAKER_FRONTLEFT:
				Entries[ChannelIndex].InputChannel = 0;
				break;

			case XAUDIOSPEAKER_FRONTRIGHT:
				Entries[ChannelIndex].InputChannel = 1;
				break;

			case XAUDIOSPEAKER_FRONTCENTER:
				Entries[ChannelIndex].InputChannel = 2;
				break;

			case XAUDIOSPEAKER_LOWFREQUENCY:
				Entries[ChannelIndex].InputChannel = 3;
				break;

			case XAUDIOSPEAKER_BACKLEFT:
				Entries[ChannelIndex].InputChannel = 4;
				break;

			case XAUDIOSPEAKER_BACKRIGHT:
				Entries[ChannelIndex].InputChannel = 5;
				break;
			}
			break;

		case 7:
			//@TODO: Map 6.1 sound to 5.1 speakers
			break;

		case 8:
			//@TODO: Map 7.1 sound to 5.1 speakers
			break;
		}

		Entries[ChannelIndex].OutputChannel = ChannelIndex;
		Entries[ChannelIndex].Volume = 0.0f;
	}
}

/**
 * Initializes a source with a given wave instance and prepares it for playback.
 *
 * @param	WaveInstance	wave instace being primed for playback
 * @return	TRUE if initialization was successful, FALSE otherwise
 */
UBOOL FXeSoundSource::Init( FWaveInstance* InWaveInstance )
{
	// Find matching buffer.
	Buffer = FXeSoundBuffer::Init( InWaveInstance->WaveData, ( UXeAudioDevice * )AudioDevice );
	if( Buffer )
	{
		if( Buffer->NumChannels == 0 )
		{
			return( FALSE );
		}

		SCOPE_CYCLE_COUNTER( STAT_AudioSourceInitTime );

		WaveInstance = InWaveInstance;

		// Release existing voice.
		if( Voice )
		{
			Voice->Release();
		}

		// Figure out how many channels are used.
		check( Buffer->Format.SampleType == XAUDIOSAMPLETYPE_XMA );

		XAUDIOCHANNELMAPENTRY Entries[CHANNELOUT_COUNT] = { 0 };
		MapSpeakers( Entries );

		XAUDIOCHANNELMAP ChannelMap = { 0 };
		ChannelMap.EntryCount = ARRAY_COUNT( Entries );
		ChannelMap.paEntries = Entries;

		XAUDIOVOICEOUTPUTENTRY OutputEntry = { 0 };
		bEQFilterApplied = WaveInstance->bApplyEffects;
		// This selects the output submix voice - either dry or EQ filtered
		OutputEntry.pDestVoice = ( IXAudioVoice* )AudioDevice->UpdateEffect( this );
		OutputEntry.pChannelMap = &ChannelMap;

		XAUDIOVOICEOUTPUT Output = { 0 };
		Output.EntryCount = 1;
		Output.paEntries = &OutputEntry;

		// Set up voice format.
		XAUDIOSOURCEVOICEINIT VoiceInit = { 0 };
		VoiceInit.Format = Buffer->Format;
		VoiceInit.MaxOutputVoiceCount = 1;
		VoiceInit.MaxChannelMapEntryCount = CHANNELOUT_COUNT;
		VoiceInit.MaxPacketCount = 2;
#if _XTL_VER < 2417  
		VoiceInit.MaxFreqScale = 4; // needs to be increased if we trigger an assert
#else
		VoiceInit.MaxPitchShift = 4; // needs to be increased if we trigger an assert
#endif
		VoiceInit.pVoiceOutput = &Output;

		// Flag music properly so it is automatically muted when user plays custom soundtrack (TCR #024)
		VoiceInit.Category = WaveInstance->bIsMusic ? XAUDIOVOICECATEGORY_BGMUSIC : XAUDIOVOICECATEGORY_NONE;

		// Add instance of effect to the chain
		FXeAudioEffectsManager* Effects = ( FXeAudioEffectsManager* )AudioDevice->GetEffects();
		XAUDIOFXINIT EffectInit = { Effects->GetLowPassFilterEffectId(), NULL };		

		// Effect chain for source voice initialization
		const XAUDIOFXINIT* SourceVoiceEffectsInit[] = { &EffectInit };
		XAUDIOVOICEFXCHAIN SourceEffectsChain = { 1, SourceVoiceEffectsInit };
		VoiceInit.pEffectChain = &SourceEffectsChain;	

		// Create new voice.
		verify( SUCCEEDED( XAudioCreateSourceVoice( &VoiceInit, &Voice ) ) );

		SourceBuffers[0] = Buffer->SourceBuffer;
		CurrentBuffer = 0;

		// Deal with looping behavior.
		if( WaveInstance->LoopingMode == LOOP_Forever )
		{
			// Set to reserved "infinite" value.
			SourceBuffers[0].LoopCount = INFINITE;
			// verify( SUCCEEDED( Voice->SubmitSourceBuffer( &SourceBuffers[0], XAUDIOSUBMITPACKET_DISCONTINUITY ) ) );
			Voice->SubmitSourceBuffer( &SourceBuffers[0], XAUDIOSUBMITPACKET_DISCONTINUITY );
		}
		else if( WaveInstance->LoopingMode == LOOP_WithNotification )
		{
			SourceBuffers[1] = SourceBuffers[0];

			// verify( SUCCEEDED( Voice->SubmitSourceBuffer( &SourceBuffers[0], XAUDIOSUBMITPACKET_DISCONTINUITY ) ) );
			Voice->SubmitSourceBuffer( &SourceBuffers[0], XAUDIOSUBMITPACKET_DISCONTINUITY );
			// verify( SUCCEEDED( Voice->SubmitSourceBuffer( &SourceBuffers[1], XAUDIOSUBMITPACKET_DISCONTINUITY ) ) );
			Voice->SubmitSourceBuffer( &SourceBuffers[1], XAUDIOSUBMITPACKET_DISCONTINUITY );
		}
		else
		{
			// Regular sound source, don't loop.
			// verify( SUCCEEDED( Voice->SubmitSourceBuffer( &SourceBuffers[0], XAUDIOSUBMITPACKET_DISCONTINUITY ) ) );
			Voice->SubmitSourceBuffer( &SourceBuffers[0], XAUDIOSUBMITPACKET_DISCONTINUITY );
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
 * Updates the source specific parameter like e.g. volume and pitch based on the associated
 * wave instance.	
 */
void FXeSoundSource::Update( void )
{
	SCOPE_CYCLE_COUNTER( STAT_AudioUpdateSources );

	if( !WaveInstance || Paused )
	{	
		return;
	}

	// Better safe than sorry!
	FLOAT Volume = Clamp<FLOAT>( WaveInstance->Volume * WaveInstance->VolumeMultiplier, 0.0f, 1.0f );
	const FLOAT SampleRateScalar = Clamp<FLOAT>( WaveInstance->Pitch, 0.4f, 2.0f );

	// The internal function is a ratio, actual sample rates are passed in to future proof
	XAUDIOSAMPLERATE BaseFrequency = ( XAUDIOSAMPLERATE )44100;
	XAUDIOSAMPLERATE PitchShiftedFrequency = ( XAUDIOSAMPLERATE )( 44100 * SampleRateScalar );
	XAUDIOPITCH Pitch = XAudioSampleRateToPitch( PitchShiftedFrequency, BaseFrequency );
	verify( SUCCEEDED( Voice->SetPitch( Pitch ) ) );

	// Set whether to bleed to the rear speakers
	SetStereoBleed();

	// Set whether to apply reverb
	SetReverbApplied();

	// Get the radio balance (reverse attenuated with normal volume)
	FLOAT RadioBalance = GetRadioBalance( WaveInstance, Volume );

	// Set the HighFrequencyGain value (aka low pass filter setting)
	SetHighFrequencyGain();

	// Disable attenuation with LPF if radio filter is applied
	if( RadioBalance > KINDA_SMALL_NUMBER )
	{
		HighFrequencyGain = 1.0f;
	}

	// Enable or disable the low pass filter effect
	Voice->SetEffectState( 0, XAUDIOVOICEFXSTATE_ENABLED );
	Voice->SetEffectParam( 0, LPFX_HIGH_FREQUENCY_GAIN_ID, XAUDIOFXPARAMTYPE_NUMERIC, ( XAUDIOFXPARAM* )&HighFrequencyGain );

	// Initialize channel volumes 4.0 channels.
	FLOAT ChannelVolumes[CHANNELOUT_COUNT] = { 0 };

	// Full center channel voice volume = half volume on center channel, half spatialized
	FLOAT CenterBalance = WaveInstance->VoiceCenterChannelVolume * 0.65f;
	FLOAT SpatializedBalance = 1.0f - CenterBalance;

	ChannelVolumes[CHANNELOUT_FRONTLEFT] = SpatializedBalance * Volume;
	ChannelVolumes[CHANNELOUT_FRONTRIGHT] = SpatializedBalance * Volume;
	ChannelVolumes[CHANNELOUT_FRONTCENTER] = SpatializedBalance * Volume;
	ChannelVolumes[CHANNELOUT_LOWFREQUENCY] = SpatializedBalance * Volume;
	ChannelVolumes[CHANNELOUT_BACKLEFT] = SpatializedBalance * Volume;
	ChannelVolumes[CHANNELOUT_BACKRIGHT] = SpatializedBalance * Volume;

	// Xenon reverb is very loud, account for this with a constant multiplier
	ChannelVolumes[CHANNELOUT_REVERB] = SpatializedBalance * Volume * 0.5f;
	ChannelVolumes[CHANNELOUT_RADIO] = RadioBalance;
	ChannelVolumes[CHANNELOUT_VOICECENTER] = CenterBalance * Volume;

	switch( Buffer->NumChannels )
	{
	case 1:	
		{
			// Calculate direction from listener to sound, where the sound is at the origin if unspatialized.
			FVector Direction = FVector( 0.0f, 0.0f, 0.0f );
			if( WaveInstance->bUseSpatialization )
			{
				Direction = ( ( UXeAudioDevice * )AudioDevice )->InverseTransform.TransformFVector( WaveInstance->Location ).SafeNormal();
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

			// Calculate 5.1 channel dolby surround rate/ multipliers.
			FLOAT ChannelMultipliers[XAUDIOSPEAKER_COUNT] = { 0.0f };
			FLOAT ReverbLevel = 0.0f;
			SpatializationHelper.CalculateDolbySurroundRate( OrientFront, ListenerPosition, EmitterPosition, ChannelMultipliers, ReverbLevel );

			// Convert from rate/ multiplier to final volume.
			for( INT SpeakerIndex = 0; SpeakerIndex < XAUDIOSPEAKER_COUNT; SpeakerIndex++ )
			{
				if( SpeakerIndex == CHANNELOUT_LOWFREQUENCY )
				{
					// -6dB
					ChannelVolumes[SpeakerIndex] *= 0.5f; 
				}
				else
				{
					ChannelVolumes[SpeakerIndex] *= ChannelMultipliers[SpeakerIndex];
				}
			}

			ChannelVolumes[CHANNELOUT_REVERB] *= ReverbLevel;
			break;
		}

	case 2:
		{
			// Stereo is always treated as unspatialized
			// It is bled to the surrounds at 1/4 volume to convert from 2.0 channel to simulated 4.0 channel.
			ChannelVolumes[CHANNELOUT_FRONTCENTER] = 0.0f;
			ChannelVolumes[CHANNELOUT_LOWFREQUENCY] = 0.0f;
			if( bStereoBleed )
			{
				ChannelVolumes[CHANNELOUT_BACKLEFT] *= 0.25f;
				ChannelVolumes[CHANNELOUT_BACKRIGHT] *= 0.25f;
			}
			else
			{
				ChannelVolumes[CHANNELOUT_BACKLEFT] = 0.0f;
				ChannelVolumes[CHANNELOUT_BACKRIGHT] = 0.0f;
			}
			break;
		}

	case 4:
		{
			// 4.0 unspatialised sound
			ChannelVolumes[CHANNELOUT_FRONTCENTER] = 0.0f;
			ChannelVolumes[CHANNELOUT_LOWFREQUENCY] = 0.0f;
			break;
		}

	case 6:
		{
			// 5.1 foley mixes are baked at full volume, so they need no volume modification
			break;
		}

	default:
	case 7:
	case 8:
		{
			// Currently unhandled - you may wish to add an assert here if you want to catch 6.1 or 7.1 format sounds
			break;
		}
	}

	FLOAT DryVolume = 1.0f;
	FLOAT ReverbVolume = 1.0f;
	FLOAT EQVolume = 1.0f;
	FLOAT RadioVolume = 1.0f;
	FLOAT CenterVolume = 1.0f;

	FAudioEffectsManager* Effects = ( FAudioEffectsManager* )AudioDevice->GetEffects();
	if( Effects )
	{
		Effects->GetVolumes( DryVolume, ReverbVolume, EQVolume, RadioVolume, CenterVolume );
	}

	XAUDIOCHANNELVOLUMEENTRY ChannelEntries[]= 
	{
		{ CHANNELOUT_FRONTLEFT		, ChannelVolumes[CHANNELOUT_FRONTLEFT] * DryVolume },
		{ CHANNELOUT_FRONTRIGHT		, ChannelVolumes[CHANNELOUT_FRONTRIGHT] * DryVolume },
		{ CHANNELOUT_FRONTCENTER	, ChannelVolumes[CHANNELOUT_FRONTCENTER] * DryVolume },
		{ CHANNELOUT_LOWFREQUENCY	, ChannelVolumes[CHANNELOUT_LOWFREQUENCY] * DryVolume },
		{ CHANNELOUT_BACKLEFT		, ChannelVolumes[CHANNELOUT_BACKLEFT] * DryVolume },
		{ CHANNELOUT_BACKRIGHT		, ChannelVolumes[CHANNELOUT_BACKRIGHT] * DryVolume },

		{ CHANNELOUT_REVERB			, ChannelVolumes[CHANNELOUT_REVERB] * ReverbVolume },
		{ CHANNELOUT_RADIO			, ChannelVolumes[CHANNELOUT_RADIO] * RadioVolume },
		{ CHANNELOUT_VOICECENTER	, ChannelVolumes[CHANNELOUT_VOICECENTER] * CenterVolume },
	};

	if( !bReverbApplied )
	{
		ChannelEntries[CHANNELOUT_REVERB].Volume = 0.0f;
	}

	// TODO:Reduce the number of outputs depending on whether or not voice or reverb is supported
	// Perf testing suggests this does not have a noticeable issue, so this is not high-pri
	XAUDIOCHANNELVOLUME ChannelVolume = { ARRAY_COUNT( ChannelEntries ), ChannelEntries };
	XAUDIOVOICEOUTPUTVOLUMEENTRY VoiceEntry = { 0, &ChannelVolume };
	XAUDIOVOICEOUTPUTVOLUME VoiceVolume = { 1, &VoiceEntry };

	verify( SUCCEEDED( Voice->SetVoiceOutputVolume( &VoiceVolume ) ) );
}

/**
 * Plays the current wave instance.	
 */
void FXeSoundSource::Play( void )
{
	if( WaveInstance )
	{
		if( !Playing )
		{
			if( Buffer->NumChannels == 6 )
			{
				XMPHelper.CinematicAudioStarted();
			}
		}

		verify( SUCCEEDED( Voice->Start( 0 ) ) );
		Paused = FALSE;
		Playing = TRUE;
	}
}

/**
 * Stops the current wave instance and detaches it from the source.	
 */
void FXeSoundSource::Stop( void )
{
	if( WaveInstance )
	{	
		if( Playing )
		{
			if( Buffer->NumChannels == 6 )
			{
				XMPHelper.CinematicAudioStopped();
			}
		}

		verify( SUCCEEDED( Voice->Stop( 0 ) ) );
		Paused = FALSE;
		Playing = FALSE;
		Buffer = NULL;
	}

	FSoundSource::Stop();
}

/**
 * Pauses playback of current wave instance.
 */
void FXeSoundSource::Pause( void )
{
	if( WaveInstance )
	{
		verify( SUCCEEDED( Voice->Stop( 0 ) ) );
		Paused = TRUE;
	}
}

/**
 * Queries the status of the currently associated wave instance.
 *
 * @return	TRUE if the wave instance/ source has finished playback and FALSE if it is 
 *			currently playing or paused.
 */
UBOOL FXeSoundSource::IsFinished( void )
{
	// A paused source is not finished.
	if( Paused )
	{
		return FALSE;
	}

	if( WaveInstance )
	{
		// Retrieve state source is in.
		XAUDIOSOURCESTATE SourceState;
		verify( SUCCEEDED( Voice->GetVoiceState( &SourceState ) ) );
		UBOOL bIsSourceInFinishedState = !( SourceState & XAUDIOSOURCESTATE_STARTED ) 
										|| ( SourceState & XAUDIOSOURCESTATE_STOPPING )
										|| ( SourceState & XAUDIOSOURCESTATE_STARVED );
		UBOOL bIsReadyForPacket = SourceState & XAUDIOSOURCESTATE_READYPACKET;
		UBOOL bIsPendingStart = SourceState & XAUDIOSOURCESTATE_SYNCHRONIZED;

		if( bIsSourceInFinishedState )
		{
			// Notify the wave instance that it has finished playing.
			WaveInstance->NotifyFinished();
			return( TRUE );
		}
		else if( WaveInstance->LoopingMode == LOOP_WithNotification && bIsReadyForPacket && !bIsPendingStart )
		{
			// Notify the wave instance that the current buffer has finished playing.
			WaveInstance->NotifyFinished();

			verify( SUCCEEDED( Voice->SubmitSourceBuffer( &SourceBuffers[CurrentBuffer & 1], XAUDIOSUBMITPACKET_DISCONTINUITY ) ) );
			CurrentBuffer++;
		}

		return( FALSE );
	}

	return( TRUE );
}

/*------------------------------------------------------------------------------------
	FXMAHelper.
------------------------------------------------------------------------------------*/

/**
 * Helper structure to access information in raw XMA data.
 */
struct FXMAInfo
{
	/**
	 * Constructor, parsing passed in raw data.
	 *
	 * @param RawData		raw XMA data
	 * @param RawDataSize	size of raw data in bytes
	 */
	FXMAInfo( BYTE* RawData, UINT RawDataSize )
	{
		// Check out XeTools.cpp/dll.
		UINT Offset = 0;
		appMemcpy( &EncodedBufferFormatSize, RawData + Offset, sizeof( DWORD ) );
		Offset += sizeof( DWORD );
		appMemcpy( &SeekTableSize, RawData + Offset, sizeof( DWORD ) );
		Offset += sizeof( DWORD );
		appMemcpy( &EncodedBufferSize, RawData + Offset, sizeof( DWORD ) );
		Offset += sizeof( DWORD );

		//@warning EncodedBufferFormat is NOT endian swapped.

		EncodedBufferFormat = ( XMA2WAVEFORMAT* )( RawData + Offset );
		Offset += EncodedBufferFormatSize;
		SeekTable = ( DWORD* )( RawData + Offset );
		Offset += SeekTableSize;
		EncodedBuffer = RawData + Offset;
		Offset += EncodedBufferSize;

		check( Offset == RawDataSize );
	}

	/** Encoded buffer data (allocated via malloc from within XMA encoder) */
	void*			EncodedBuffer;
	/** Size in bytes of encoded buffer */
	DWORD			EncodedBufferSize;
	/** Encoded buffer format (allocated via malloc from within XMA encoder) */
	XMA2WAVEFORMAT*	EncodedBufferFormat;
	/** Size in bytes of encoded buffer format */
	DWORD			EncodedBufferFormatSize;
	/** Seek table (allocated via malloc from within XMA encoder) */
	DWORD*			SeekTable;
	/** Size in bytes of seek table */
	DWORD			SeekTableSize;
};


/*------------------------------------------------------------------------------------
	FXeSoundBuffer.
------------------------------------------------------------------------------------*/

/** 
 * Constructor
 *
 * @param AudioDevice	audio device this sound buffer is going to be attached to.
 */
FXeSoundBuffer::FXeSoundBuffer( UXeAudioDevice* InAudioDevice )
{
	AudioDevice	= InAudioDevice;
	ResourceID = INDEX_NONE;
	bAllocationInPermanentPool = FALSE;
	SourceBuffer.pBuffer = NULL;
	SourceBuffer.BufferSize = 0;
}

/**
 * Destructor 
 * 
 * Frees wave data and detaches itself from audio device.
 */
FXeSoundBuffer::~FXeSoundBuffer( void )
{
	if( bAllocationInPermanentPool )
	{
		appErrorf( TEXT( "Can't free resource '%s' as it was allocated in permanent pool." ), *ResourceName );
	}

	if( ResourceID )
	{
		AudioDevice->WaveBufferMap.Remove( ResourceID );

		if( SourceBuffer.pBuffer )
		{
			// Wave data was kept in pBuffer so we need to free it.
			appPhysicalFree( SourceBuffer.pBuffer );
		}
	}
}

/**
 * Returns the size of this buffer in bytes.
 *
 * @return Size in bytes
 */
INT FXeSoundBuffer::GetSize( void )
{
	INT TotalSize = SourceBuffer.BufferSize;
	return TotalSize;
}

/**
 * Static function used to create a buffer.
 *
 * @param InWave		USoundNodeWave to use as template and wave source
 * @param AudioDevice	audio device to attach created buffer to
 * @return FXeSoundBuffer pointer if buffer creation succeeded, NULL otherwise
 */
FXeSoundBuffer* FXeSoundBuffer::Init( USoundNodeWave* Wave, UXeAudioDevice* AudioDevice )
{
	// Can't create a buffer without any source data
	if( Wave == NULL || Wave->NumChannels == 0 )
	{
		return( NULL );
	}

	FXeSoundBuffer* Buffer = NULL;

	// Check whether this buffer already has been initialized...
	if( Wave->ResourceID )
	{
		Buffer = AudioDevice->WaveBufferMap.FindRef( Wave->ResourceID );
	}

	if( Buffer )
	{
		// ... and return cached pointer if it has.
		return( Buffer );
	}
	else
	{
		SCOPE_CYCLE_COUNTER( STAT_AudioResourceCreationTime );

		// Create new buffer.
		Buffer = new FXeSoundBuffer( AudioDevice );
		if( !Wave->bUseTTS )
		{
			// Check whether there is any raw data.
			INT RawDataSize = Wave->CompressedXbox360Data.GetBulkDataSize();
			if( RawDataSize <= 0 )
			{
				// No data, no audio!
				debugf( NAME_DevAudio, TEXT( "%s has no data, skipping" ), *Wave->GetFullName() );
				// Delete buffer created above before returning.
				delete Buffer;
				return NULL;
			}

			// Load raw data.
			void* RawData = Wave->CompressedXbox360Data.Lock( LOCK_READ_ONLY );

			// Create XMA helper to parse raw data.
			FXMAInfo XMAInfo( ( BYTE* )RawData, RawDataSize );

			// Cache buffer format for use in FXeSoundSource::Init.
			Buffer->Format.SampleType = XAUDIOSAMPLETYPE_XMA;
			Buffer->Format.NumStreams = XMAInfo.EncodedBufferFormat->NumStreams;

			Buffer->NumChannels = 0;
			for( INT i = 0; i < Buffer->Format.NumStreams; i++ )
			{
				Buffer->Format.Stream[i].SampleRate = XMAInfo.EncodedBufferFormat->SampleRate;
				Buffer->Format.Stream[i].ChannelCount = XMAInfo.EncodedBufferFormat->Streams[i].Channels;
				Buffer->Format.Stream[i].DecodeBufferSize = XMASUBFRAME_DECODEROUTPUT_FASTEST;

				Buffer->NumChannels += XMAInfo.EncodedBufferFormat->Streams[i].Channels;
			}	

			if( Buffer->NumChannels != Wave->NumChannels )
			{
				appErrorf( TEXT( "Channel count mismatch for '%s', expected %i but got %i" ), *Wave->GetFullName(), Wave->NumChannels, Buffer->NumChannels );
			}

			XAUDIOSOURCEBUFFER* AudioSourceBuffer = &Buffer->SourceBuffer;

			if( Wave->HasAnyFlags( RF_RootSet ) )
			{
				// Allocate from permanent pool and mark buffer as non destructible.
				AudioSourceBuffer->pBuffer = AudioDevice->AllocatePermanentMemory( XMAInfo.EncodedBufferSize );
				Buffer->bAllocationInPermanentPool = TRUE;
			}
			else
			{
				// Allocate via normal allocator.
				AudioSourceBuffer->pBuffer = appPhysicalAlloc( XMAInfo.EncodedBufferSize, CACHE_Normal );
			}

			AudioSourceBuffer->BufferSize = XMAInfo.EncodedBufferSize;

			AudioSourceBuffer->LoopCount = 0;
			AudioSourceBuffer->LoopBegin = XMAInfo.EncodedBufferFormat->LoopBegin;
			AudioSourceBuffer->LoopEnd = XMAInfo.EncodedBufferFormat->LoopEnd;

			AudioSourceBuffer->PlayBegin = XMAInfo.EncodedBufferFormat->LoopBegin;
			AudioSourceBuffer->PlayEnd = XMAInfo.EncodedBufferFormat->LoopEnd;

			AudioSourceBuffer->pContext = NULL;

			appMemcpy( AudioSourceBuffer->pBuffer, XMAInfo.EncodedBuffer, XMAInfo.EncodedBufferSize );

			// Unload raw data.
			Wave->CompressedXbox360Data.Unlock();
		}
		else
		{
			Buffer->NumChannels = 0;
		}

		// Allocate new resource ID and assign to USoundNodeWave. A value of 0 (default) means not yet registered.
		INT ResourceID = AudioDevice->NextResourceID++;
		Buffer->ResourceID = ResourceID;
		Wave->ResourceID = ResourceID;

		AudioDevice->Buffers.AddItem( Buffer );
		AudioDevice->WaveBufferMap.Set( ResourceID, Buffer );

		// Keep track of associated resource name.
		Buffer->ResourceName = Wave->GetPathName();
		return( Buffer );
	}

	return( NULL );
}

/*------------------------------------------------------------------------------------
	Static linking helpers.
------------------------------------------------------------------------------------*/

/**
 * Initialize registrants, basically calling StaticClass() to create the class and also 
 * populating the lookup table.
 *
 * @param	Lookup	current index into lookup table
 */
void AutoInitializeRegistrantsXeAudio( INT& Lookup )
{
	UXeAudioDevice::StaticClass();
}

/**
 * Auto generates names.
 */
void AutoRegisterNamesXeAudio( void )
{
}

/*------------------------------------------------------------------------------------
	FXeSpatializationHelper.
------------------------------------------------------------------------------------*/

/**
 * Constructor, initializing all member variables.
 */
FXeSpatializationHelper::FXeSpatializationHelper( void )
{
	// Initialize X3DAudio
	//
	//  Speaker geometry configuration on the final mix, specifies assignment of channels
	//  to speaker positions, defined as per WAVEFORMATEXTENSIBLE.dwChannelMask
	X3DAudioInitialize( SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT,
						X3DAUDIO_SPEED_OF_SOUND, 
						X3DInstance );
	
	// Initialize 3D audio parameters
	X3DAUDIO_VECTOR ZeroVector			= { 0.0f, 0.0f, 0.0f };
	EmitterAzimuths						= 0.0f;

	// Set up listener parameters
	Listener.OrientFront.x				= 0.0f;
	Listener.OrientFront.y				= 0.0f;
	Listener.OrientFront.z				= 1.0f;
	Listener.OrientTop.x				= 0.0f;
	Listener.OrientTop.y				= 1.0f;
	Listener.OrientTop.z				= 0.0f;
	Listener.Position.x					= 0.0f;
	Listener.Position.y					= 0.0f;
	Listener.Position.z					= 0.0f;
	Listener.Velocity					= ZeroVector;

	// Set up emitter parameters
	Emitter.OrientFront.x			    = 0.0f;
	Emitter.OrientFront.y				= 0.0f;
	Emitter.OrientFront.z				= 1.0f;
	Emitter.OrientTop.x					= 0.0f;
	Emitter.OrientTop.y					= 1.0f;
	Emitter.OrientTop.z					= 0.0f;
	Emitter.Position					= ZeroVector;
	Emitter.Velocity					= ZeroVector;
	Emitter.pCone						= &Cone;
	Emitter.pCone->InnerAngle			= 0.0f; 
	Emitter.pCone->OuterAngle			= 0.0f;
	Emitter.pCone->InnerVolume			= 0.0f;
	Emitter.pCone->OuterVolume			= 1.0f;
	Emitter.pCone->InnerLPF				= 0.0f;
	Emitter.pCone->OuterLPF				= 1.0f;
	Emitter.pCone->InnerReverb			= 0.0f;
	Emitter.pCone->OuterReverb			= 1.0f;

	Emitter.ChannelCount				= 1;
	Emitter.ChannelRadius				= 0.0f;
	Emitter.pChannelAzimuths			= &EmitterAzimuths;

	// real volume -> 5.1-ch rate
	VolumeCurvePoint[0].Distance		= 0.0f;
	VolumeCurvePoint[0].DSPSetting		= 1.0f;
	VolumeCurvePoint[1].Distance		= 1.0f;
	VolumeCurvePoint[1].DSPSetting		= 1.0f;
	VolumeCurve.PointCount				= ARRAY_COUNT( VolumeCurvePoint );
	VolumeCurve.pPoints					= VolumeCurvePoint;

	ReverbVolumeCurvePoint[0].Distance		= 0.0f;
	ReverbVolumeCurvePoint[0].DSPSetting	= 0.5f;
	ReverbVolumeCurvePoint[1].Distance		= 1.0f;
	ReverbVolumeCurvePoint[1].DSPSetting	= 0.5f;
	ReverbVolumeCurve.PointCount			= ARRAY_COUNT( ReverbVolumeCurvePoint );
	ReverbVolumeCurve.pPoints				= ReverbVolumeCurvePoint;

	Emitter.pVolumeCurve				= &VolumeCurve;
	Emitter.pLFECurve					= NULL;
	Emitter.pLPFDirectCurve				= NULL;
	Emitter.pLPFReverbCurve				= NULL;
	Emitter.pReverbCurve				= &ReverbVolumeCurve;
	Emitter.CurveDistanceScaler			= 1.0f;
	Emitter.DopplerScaler				= 1.0f;

	DSPSettings.SrcChannelCount			= 1;
	DSPSettings.DstChannelCount			= XAUDIOSPEAKER_COUNT;
	DSPSettings.pMatrixCoefficients		= MatrixCoefficients;
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
void FXeSpatializationHelper::CalculateDolbySurroundRate( const FVector& OrientFront, const FVector& ListenerPosition, const FVector& EmitterPosition, FLOAT* OutVolumes, FLOAT& OutReverbLevel  )
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
	
	for( INT SpeakerIndex = 0; SpeakerIndex < XAUDIOSPEAKER_COUNT; SpeakerIndex++ )
	{
		OutVolumes[SpeakerIndex] = DSPSettings.pMatrixCoefficients[SpeakerIndex];
	}

	OutReverbLevel = DSPSettings.ReverbLevel;
}

/*------------------------------------------------------------------------------------
	FXeXMPHelper.
------------------------------------------------------------------------------------*/

/**
 * FXeXMPHelper constructor - Protected.
 */
FXeXMPHelper::FXeXMPHelper()
: CinematicAudioCount(0)
, MoviePlaying(FALSE)
, XMPEnabled(TRUE)
, XMPBlocked(FALSE)
{
}

/**
 * FXeXMPHelper destructor.
 */
FXeXMPHelper::~FXeXMPHelper()
{
}

/**
 * Records that a cinematic audio track has started playing.
 */
void FXeXMPHelper::CinematicAudioStarted()
{
	check(CinematicAudioCount >= 0);
	CinematicAudioCount++;
	CountsUpdated();
}

/**
 * Records that a cinematic audio track has stopped playing.
 */
void FXeXMPHelper::CinematicAudioStopped()
{
	check(CinematicAudioCount > 0);
	CinematicAudioCount--;
	CountsUpdated();
}

/**
 * Records that a cinematic audio track has started playing.
 */
void FXeXMPHelper::MovieStarted()
{
	MoviePlaying = TRUE;
	CountsUpdated();
}

/**
 * Records that a cinematic audio track has stopped playing.
 */
void FXeXMPHelper::MovieStopped()
{
	MoviePlaying = FALSE;
	CountsUpdated();
}

/**
 * Called every frame to update XMP status if necessary
 */
void FXeXMPHelper::CountsUpdated()
{
	if( XMPEnabled )
	{
		if( ( MoviePlaying == TRUE ) || ( CinematicAudioCount > 0 ) )
		{
			XMPOverrideBackgroundMusic();
			XMPEnabled = FALSE;
		}
	}
	else
	{
		if( ( MoviePlaying == FALSE ) && ( CinematicAudioCount == 0 ) )
		{
			XMPRestoreBackgroundMusic();
			XMPEnabled = TRUE;
		}
	}
}

// end

