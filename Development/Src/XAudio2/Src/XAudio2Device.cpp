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
#include "UnAudioEffect.h"
#include "XAudio2Device.h"
#include "XAudio2Effects.h"

/*------------------------------------------------------------------------------------
	Static variables from the early init
------------------------------------------------------------------------------------*/

IXAudio2* UXAudio2Device::XAudio2						= NULL;
IXAudio2MasteringVoice* UXAudio2Device::MasteringVoice	= NULL;
XAUDIO2_DEVICE_DETAILS UXAudio2Device::DeviceDetails	= { 0 };

/*------------------------------------------------------------------------------------
	UXAudio2Device constructor and UObject interface.
------------------------------------------------------------------------------------*/

IMPLEMENT_CLASS( UXAudio2Device );

/**
 * Static constructor, used to associate .ini options with member variables.	
 */
void UXAudio2Device::StaticConstructor( void )
{
}

/**  
 * Check for errors and output a human readable string 
 */
UBOOL UXAudio2Device::ValidateAPICall( const TCHAR* Function, INT ErrorCode )
{
	if( ErrorCode != S_OK )
	{
		switch( ErrorCode )
		{
		case XAUDIO2_E_INVALID_CALL:
			debugf( NAME_DevAudio, TEXT( "%s error: Invalid Call" ), Function );
			break;

		case XAUDIO2_E_XMA_DECODER_ERROR:
			debugf( NAME_DevAudio, TEXT( "%s error: XMA Decoder Error" ), Function );
			break;

		case XAUDIO2_E_XAPO_CREATION_FAILED:
			debugf( NAME_DevAudio, TEXT( "%s error: XAPO Creation Failed" ), Function );
			break;

		case XAUDIO2_E_DEVICE_INVALIDATED:
			debugf( NAME_DevAudio, TEXT( "%s error: Device Invalidated" ), Function );
			break;
		};
		return( FALSE );
	}

	return( TRUE );
}

/**
 * Tears down audio device by stopping all sounds, removing all buffers, 
 * destroying all sources, ... Called by both Destroy and ShutdownAfterError
 * to perform the actual tear down.
 */
void UXAudio2Device::Teardown( void )
{
	// Flush stops all sources and deletes all buffers so sources can be safely deleted below.
	Flush( NULL );

	for( INT i = 0; i < Sources.Num(); i++ )
	{
		delete Sources( i );
	}

	// Clear out the EQ/Reverb/LPF effects
	delete Effects;

	Sources.Empty();
	FreeSources.Empty();

	if( MasteringVoice )
	{
		MasteringVoice->DestroyVoice();
		MasteringVoice = NULL;
	}

	if( XAudio2 )
	{
		// Force the hardware to release all references
		XAudio2->Release();
		XAudio2 = NULL;
	}

#if _WINDOWS
	CoUninitialize();
	GIsCOMInitialized = FALSE;
#endif
}

/**
 * Shuts down audio device. This will never be called with the memory image
 * codepath.
 */
void UXAudio2Device::FinishDestroy( void )
{
	if( !HasAnyFlags( RF_ClassDefaultObject ) )
	{
		Teardown();
		debugf( NAME_Exit, TEXT( "XAudio2 Device shut down." ) );
	}

	Super::FinishDestroy();
}

/**
 * Special variant of Destroy that gets called on fatal exit. Doesn't really
 * matter on the console so for now is just the same as Destroy so we can
 * verify that the code correctly cleans up everything.
 */
void UXAudio2Device::ShutdownAfterError( void )
{
	if( !HasAnyFlags( RF_ClassDefaultObject ) )
	{
		debugf( NAME_Exit, TEXT( "UXAudio2Device::ShutdownAfterError" ) );
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
UBOOL UXAudio2Device::Init( void )
{
	// Make sure no interface classes contain any garbage
	Effects = NULL;

	// No channels, no sound.
	if( MaxChannels < 1 )
	{	
		return( FALSE );
	}

	// Init everything in XAudio2
	if( !InitHardware() )
	{
		return( FALSE );
	}

	// Create the effects subsystem (reverb, EQ, etc.)
	Effects = new FXAudio2EffectsManager( this );

	// Initialize channels.
	for( INT i = 0; i < Min( MaxChannels, MAX_AUDIOCHANNELS ); i++ )
	{
		FXAudio2SoundSource* Source = new FXAudio2SoundSource( this, Effects );
		Sources.AddItem( Source );
		FreeSources.AddItem( Source );
	}

	if( !Sources.Num() )
	{
		debugf( NAME_Error, TEXT( "XAudio2Device: couldn't allocate sources" ) );
		return FALSE;
	}

	// Update MaxChannels in case we couldn't create enough sources.
	MaxChannels = Sources.Num();
	debugf( TEXT( "XAudio2Device: Allocated %i sources" ), MaxChannels );

	// Initialize permanent memory stack for initial & always loaded sound allocations.
	if( CommonAudioPoolSize )
	{
		debugf( TEXT( "XAudio2Device: Allocating %g MByte for always resident audio data" ), CommonAudioPoolSize / ( 1024.0f * 1024.0f ) );
		CommonAudioPoolFreeBytes = CommonAudioPoolSize;
		CommonAudioPool = ( BYTE* )appPhysicalAlloc( CommonAudioPoolSize, CACHE_Normal );
	}
	else
	{
		debugf( TEXT( "XAudio2Device: CommonAudioPoolSize is set to 0 - disabling persistent pool for audio data" ) );
		CommonAudioPoolFreeBytes = 0;
	}

	// Initialized.
	NextResourceID = 1;

	// Initialize base class last as it's going to precache already loaded audio.
	Super::Init();

	return( TRUE );
}

/** 
 * Simple init of XAudio2 device for Bink audio
 */
UBOOL UXAudio2Device::InitHardware( void )
{
	if( XAudio2 == NULL )
	{
		UINT32 NumChannels = 0;
		UINT32 SampleRate = 0;

#if _WINDOWS
		if( !GIsCOMInitialized )
		{
			CoInitialize( NULL );
			GIsCOMInitialized = TRUE;
		}
#endif

#if _DEBUG
		UINT32 Flags = XAUDIO2_DEBUG_ENGINE;
#else
		UINT32 Flags = 0;
#endif
		if( XAudio2Create( &XAudio2, Flags, AUDIO_HWTHREAD ) != S_OK )
		{
			debugf( NAME_Init, TEXT( "Failed to create XAudio2 interface" ) );
			return( FALSE );
		}

		UINT32 DeviceCount = 0;
		XAudio2->GetDeviceCount( &DeviceCount );
		if( DeviceCount < 1 )
		{
			debugf( NAME_Init, TEXT( "No audio devices found!" ) );
			XAudio2 = NULL;
			return( FALSE );		
		}

		// Get the details of the default device 0
		if( XAudio2->GetDeviceDetails( 0, &DeviceDetails ) != S_OK )
		{
			debugf( NAME_Init, TEXT( "Failed to get DeviceDetails for XAudio2" ) );
			XAudio2 = NULL;
			return( FALSE );
		}

		NumChannels = DeviceDetails.OutputFormat.Format.nChannels;
		SampleRate = DeviceDetails.OutputFormat.Format.nSamplesPerSec;

		if( NumChannels != 2 && NumChannels != 6 )
		{
			debugf( NAME_Init, TEXT( "XAudio2 requires a stereo or 5.1 output" ) );
			XAudio2 = NULL;
			return( FALSE );
		}

		debugf( NAME_Init, TEXT( "Using '%s' : %d channels at %g kHz using %d bits per sample" ), 
			DeviceDetails.DisplayName,
			NumChannels, 
			( FLOAT )SampleRate / 1000.0f, 
			DeviceDetails.OutputFormat.Format.wBitsPerSample );

		// Create the final output voice with either 2 or 6 channels
		if( XAudio2->CreateMasteringVoice( &MasteringVoice, NumChannels, SampleRate, 0, 0, NULL ) != S_OK )
		{
			debugf( NAME_Init, TEXT( "Failed to create the mastering voice for XAudio2" ) );
			XAudio2 = NULL;
			return( FALSE );
		}
	}

	return( TRUE );
}

/**
 * Update the audio device and calculates the cached inverse transform later
 * on used for spatialization.
 *
 * @param	Realtime	whether we are paused or not
 */
void UXAudio2Device::Update( UBOOL bRealtime )
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
			debugf( TEXT( "XAudio2: Audio pool size mismatch by %d bytes. Please update CommonAudioPoolSize ini setting to %d to avoid waste!" ),
									CommonAudioPoolFreeBytes, CommonAudioPoolSize - CommonAudioPoolFreeBytes );
		}
	}
}

/**
 * Precaches the passed in sound node wave object.
 *
 * @param	SoundNodeWave	Resource to be precached.
 */
void UXAudio2Device::Precache( USoundNodeWave* SoundNodeWave )
{
	FXAudio2SoundBuffer::GetSoundFormat( SoundNodeWave, GIsEditor ? MinCompressedDurationEditor : MinCompressedDurationGame );
#if _WINDOWS
	if( SoundNodeWave->VorbisDecompressor == NULL && SoundNodeWave->DecompressionType == DTYPE_Native )
	{
		// Grab the compressed vorbis data
		SoundNodeWave->InitAudioResource( SoundNodeWave->CompressedPCData );

		// Create a worker to decompress the vorbis data
		SoundNodeWave->VorbisDecompressor = new FAsyncVorbisDecompress( SoundNodeWave );

		// Add the worker to the threadpool
		GThreadPool->AddQueuedWork( SoundNodeWave->VorbisDecompressor );
	}
	else
	{
		// If it's not native, then it will remain compressed
		INC_DWORD_STAT_BY( STAT_AudioMemorySize, SoundNodeWave->GetResourceSize() );
		INC_DWORD_STAT_BY( STAT_AudioMemory, SoundNodeWave->GetResourceSize() );
	}
#elif XBOX
	FXAudio2SoundBuffer::Init( this, SoundNodeWave );
	// Size of the decompressed data
	INC_DWORD_STAT_BY( STAT_AudioMemorySize, SoundNodeWave->GetResourceSize() );
	INC_DWORD_STAT_BY( STAT_AudioMemory, SoundNodeWave->GetResourceSize() );
#endif
}

/**
 * Frees the bulk resource data assocated with this SoundNodeWave.
 *
 * @param	SoundNodeWave	wave object to free associated bulk data
 */
void UXAudio2Device::FreeResource( USoundNodeWave* SoundNodeWave )
{
	// Find buffer for resident wavs
	if( SoundNodeWave->ResourceID )
	{
		// Just in case the data was created but never uploaded
		if( SoundNodeWave->RawPCMData )
		{
			appFree( SoundNodeWave->RawPCMData );
			SoundNodeWave->RawPCMData = NULL;
		}

		// Find buffer associated with resource id.
		FXAudio2SoundBuffer* Buffer = WaveBufferMap.FindRef( SoundNodeWave->ResourceID );
		if( Buffer )
		{
			// Remove from buffers array.
			Buffers.RemoveItem( Buffer );

			// See if it is being used by a sound source...
			UBOOL bWasReferencedBySource = FALSE;
			for( INT SrcIndex = 0; SrcIndex < Sources.Num(); SrcIndex++ )
			{
				FXAudio2SoundSource* Src = ( FXAudio2SoundSource* )( Sources( SrcIndex ) );
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
	DEC_DWORD_STAT_BY( STAT_AudioMemorySize, SoundNodeWave->GetResourceSize() );
	DEC_DWORD_STAT_BY( STAT_AudioMemory, SoundNodeWave->GetResourceSize() );
}

// sort memory usage from large to small unless bAlphaSort
static UBOOL bAlphaSort = FALSE;
IMPLEMENT_COMPARE_POINTER( FXAudio2SoundBuffer, XeAudioDevice, { return bAlphaSort ? appStricmp( *A->ResourceName,*B->ResourceName ) : ( A->GetSize() > B->GetSize() ) ? -1 : 1; } );

/** 
 * Displays debug information about the loaded sounds
 */
void UXAudio2Device::ListSounds( const TCHAR* Cmd, FOutputDevice& Ar )
{
	bAlphaSort = ParseParam( Cmd, TEXT( "ALPHASORT" ) );

	INT	TotalResident = 0;
	INT	ResidentCount = 0;

	Ar.Logf( TEXT( ",Size Kb,NumChannels,SoundName,bAllocationInPermanentPool" ) );

	TArray<FXAudio2SoundBuffer*> AllSounds;
	for( INT BufferIndex = 0; BufferIndex < Buffers.Num(); BufferIndex++ )
	{
		AllSounds.AddItem( Buffers( BufferIndex ) );
	}

	Sort<USE_COMPARE_POINTER( FXAudio2SoundBuffer, XeAudioDevice )>( &AllSounds( 0 ), AllSounds.Num() );

	for( INT i = 0; i < AllSounds.Num(); ++i )
	{
		FXAudio2SoundBuffer* Buffer = AllSounds( i );

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
UBOOL UXAudio2Device::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	if( Super::Exec( Cmd, Ar ) )
	{
		return( TRUE );
	}

	return( FALSE );
}

/**
 * Allocates memory from permanent pool. This memory will NEVER be freed.
 *
 * @param	Size	Size of allocation.
 *
 * @return pointer to a chunk of memory with size Size
 */
void* UXAudio2Device::AllocatePermanentMemory( INT Size )
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

/** 
 * Links up the resource data indices for looking up and cleaning up
 */
void UXAudio2Device::TrackResource( USoundNodeWave* Wave, FXAudio2SoundBuffer* Buffer )
{
	// Allocate new resource ID and assign to USoundNodeWave. A value of 0 (default) means not yet registered.
	INT ResourceID = NextResourceID++;
	Buffer->ResourceID = ResourceID;
	Wave->ResourceID = ResourceID;

	Buffers.AddItem( Buffer );
	WaveBufferMap.Set( ResourceID, Buffer );
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
void AutoInitializeRegistrantsXAudio2( INT& Lookup )
{
	UXAudio2Device::StaticClass();
}

/**
 * Auto generates names.
 */
void AutoRegisterNamesXAudio2( void )
{
}

// end

