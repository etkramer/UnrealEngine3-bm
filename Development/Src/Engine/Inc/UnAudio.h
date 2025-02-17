/*=============================================================================
	UnAudio.h: Unreal base audio.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _UNAUDIO_H_
#define _UNAUDIO_H_

/** 
 * Maximum number of channels that can be set using the ini setting
 */
#define MAX_AUDIOCHANNELS	64

/** 
 * Number of ticks an inaudible source remains alive before being stopped
 */
#define AUDIOSOURCE_TICK_LONGEVITY 60

/** 
 * Length of sound in seconds to be considered as looping forever
 */
#define INDEFINITELY_LOOPING_DURATION	10000.0f

/**
 * Audio stats
 */
enum EAudioStats
{
	STAT_AudioUpdateTime = STAT_AudioFirstStat,
	STAT_AudioComponents,
	STAT_AudioSources,
	STAT_WaveInstances,
	STAT_WavesDroppedDueToPriority,
	STAT_AudibleWavesDroppedDueToPriority,
	STAT_AudioFinishedDelegatesCalled,
	STAT_AudioMemorySize,
	STAT_AudioBufferTime,
	STAT_AudioBufferTimeChannels,
	STAT_OggWaveInstances,
	STAT_AudioGatherWaveInstances,
	STAT_AudioStartSources,
	STAT_AudioUpdateSources,
	STAT_AudioUpdateEffects,
	STAT_AudioSourceInitTime,
	STAT_AudioDecompressTime,
	STAT_VorbisDecompressTime,
	STAT_AudioPrepareDecompressionTime,
	STAT_VorbisPrepareDecompressionTime,
};

/**
 * Channel definitions for multistream waves
 *
 * These are in the sample order OpenAL expects for a 7.1 sound
 * 
 */
enum EAudioSpeakers
{							//	4.0	5.1	6.1	7.1
	SPEAKER_FrontLeft,		//	*	*	*	*
	SPEAKER_FrontRight,		//	*	*	*	*
	SPEAKER_FrontCenter,	//		*	*	*
	SPEAKER_LowFrequency,	//		*	*	*
	SPEAKER_SideLeft,		//	*	*	*	*
	SPEAKER_SideRight,		//	*	*	*	*
	SPEAKER_BackLeft,		//			*	*		If there is no BackRight channel, this is the BackCenter channel
	SPEAKER_BackRight,		//				*
	SPEAKER_Count
};

// Forward declarations.
class UAudioComponent;
struct FReverbSettings;
struct FSampleLoop;

/** 
* Removes the bulk data from any USoundNodeWave objects that were loaded 
*/
extern void appSoundNodeRemoveBulkData();

/*-----------------------------------------------------------------------------
	FSoundSource.
-----------------------------------------------------------------------------*/

class FSoundSource
{
public:
	// Constructor/ Destructor.
	FSoundSource( UAudioDevice* InAudioDevice )
	:	AudioDevice( InAudioDevice ),
		WaveInstance( NULL ),
		Playing( FALSE ),
		Paused( FALSE ),
		bReverbApplied( FALSE ),
		bEQFilterApplied( FALSE ),
		bStereoBleed( FALSE ),
		HighFrequencyGain( 1.0f ),
		LastUpdate( 0 )
	{
	}

	virtual ~FSoundSource( void )
	{
	}

	// Initialization & update.
	virtual UBOOL Init( FWaveInstance* WaveInstance ) = 0;
	virtual void Update( void ) = 0;

	// Playback.
	virtual void Play( void ) = 0;
	virtual void Stop( void );
	virtual void Pause( void ) = 0;

	// Query.
	virtual	UBOOL IsFinished( void ) = 0;

	/**
	 * Returns whether the buffer associated with this source is using CPU decompression.
	 *
	 * @return TRUE if decompressed on the CPU, FALSE otherwise
	 */
	virtual UBOOL UsesCPUDecompression( void )
	{
		return( FALSE );
	}

	/**
	 * Returns whether associated audio component is an ingame only component, aka one that will
	 * not play unless we're in game mode (not paused in the UI)
	 *
	 * @return FALSE if associated component has bIsUISound set, TRUE otherwise
	 */
	UBOOL IsGameOnly( void );

	/** 
	 * @return	The wave instance associated with the sound. 
	 */
	const FWaveInstance* GetWaveInstance() const
	{
		return( WaveInstance );
	}

	/** 
	 * @return		TRUE if the sound is playing, FALSE otherwise. 
	 */
	UBOOL IsPlaying( void ) const
	{
		return( Playing );
	}

	/** 
	 * @return		TRUE if the sound is paused, FALSE otherwise. 
	 */
	UBOOL IsPaused( void ) const
	{
		return( Paused );
	}

	/** 
	 * Returns TRUE if reverb should be applied
	 */
	UBOOL IsReverbApplied( void ) const 
	{	
		return( bReverbApplied ); 
	}

	/** 
	 * Returns TRUE if EQ should be applied
	 */
	UBOOL IsEQFilterApplied( void ) const 
	{ 
		return( bEQFilterApplied ); 
	}

	/** 
	 * Returns TRUE if this stereo sound should bleed to the rear speakers
	 */
	UBOOL IsStereoBleed( void ) const 
	{ 
		return( bStereoBleed ); 
	}

	/**
	 * Set the bReverbApplied variable
	 */
	UBOOL SetReverbApplied( void );

	/**
	 * Set the bStereoBleed variable
	 */
	UBOOL SetStereoBleed( void );

	/**
	 * Set the HighFrequencyGain value
	 */
	void SetHighFrequencyGain( void );

	/**
	 * Get the radio balance
	 */
	FLOAT GetRadioBalance( FWaveInstance* WaveInstance, FLOAT& SourceVolume );

protected:
	// Variables.	
	UAudioDevice*		AudioDevice;
	FWaveInstance*		WaveInstance;

	/** Cached status information whether we are playing or not. */
	UBOOL				Playing;
	/** Cached status information whether we are paused or not. */
	UBOOL				Paused;
	/** Cached sound mode value used to detect when to switch outputs. */
	UBOOL				bReverbApplied;
	/** Whether to apply an EQ effect to this source */
	UBOOL				bEQFilterApplied;
	/** Whether to bleed 25% of the front volume to the rear speakers for stereo sounds */
	UBOOL				bStereoBleed;
	/** Low pass filter setting */
	FLOAT				HighFrequencyGain;

	/** Last tick when this source was active */
	INT					LastUpdate;
	/** Last tick when this source was active *and* had a hearable volume */
	INT					LastHeardUpdate;

	friend class UAudioDevice;
};

/*-----------------------------------------------------------------------------
	UDrawSoundRadiusComponent. 
-----------------------------------------------------------------------------*/

class UDrawSoundRadiusComponent : public UDrawSphereComponent
{
	DECLARE_CLASS(UDrawSoundRadiusComponent,UDrawSphereComponent,CLASS_NoExport,Engine);

	// UPrimitiveComponent interface.
	/**
	 * Creates a proxy to represent the primitive to the scene manager in the rendering thread.
	 * @return The proxy object.
	 */
	virtual FPrimitiveSceneProxy* CreateSceneProxy( void );
	virtual void UpdateBounds( void );
};

/**  Hash function. Needed to avoid UObject v FResource ambiguity due to multiple inheritance */
DWORD GetTypeHash( const class USoundNodeWave* A );

/*-----------------------------------------------------------------------------
	FWaveModInfo. 
-----------------------------------------------------------------------------*/

//
// Structure for in-memory interpretation and modification of WAVE sound structures.
//
class FWaveModInfo
{
public:

	// Pointers to variables in the in-memory WAVE file.
	DWORD* pSamplesPerSec;
	DWORD* pAvgBytesPerSec;
	WORD* pBlockAlign;
	WORD* pBitsPerSample;
	WORD* pChannels;

	DWORD  OldBitsPerSample;

	DWORD* pWaveDataSize;
	DWORD* pMasterSize;
	BYTE*  SampleDataStart;
	BYTE*  SampleDataEnd;
	DWORD  SampleDataSize;
	BYTE*  WaveDataEnd;

	INT	   SampleLoopsNum;
	FSampleLoop*  pSampleLoop;

	DWORD  NewDataSize;
	UBOOL  NoiseGate;

	// Constructor.
	FWaveModInfo()
	{
		NoiseGate   = false;
		SampleLoopsNum = 0;
	}
	
	// 16-bit padding.
	DWORD Pad16Bit( DWORD InDW )
	{
		return ((InDW + 1)& ~1);
	}

	// Read headers and load all info pointers in WaveModInfo. 
	// Returns 0 if invalid data encountered.
	UBOOL ReadWaveInfo( BYTE* WaveData, INT WaveDataSize );
};

#if WITH_TTS
/*-----------------------------------------------------------------------------
	FTextToSpeech.
-----------------------------------------------------------------------------*/

#define TTS_CHUNK_SIZE	71

class FTextToSpeech
{
public:
	FTextToSpeech( void )
	{
		bInit = FALSE;
	}

	~FTextToSpeech( void );

	/** Static callback from TTS engine to feed PCM data to sound system */
	static SWORD* StaticCallback( SWORD* AudioData, long Flags );

	/** Table used to convert UE3 language extension to TTS language index */
	static TCHAR* LanguageConvert[];

	/** Function to convert UE3 language extension to TTS language index */
	INT GetLanguageIndex( FString& Language );

	/** Initialise the TTS and set the language to the current one */
	void Init( void );

	/** Write a chunk of data to a buffer */
	void WriteChunk( SWORD* AudioData );

	/** Convert a line of text into PCM data */
	void CreatePCMData( USoundNodeWave* Wave );

private:
	/** Current speaker index (FnxDECtalkVoiceId) */
	BYTE			CurrentSpeaker;

	UBOOL			bInit;

	SWORD			TTSBuffer[TTS_CHUNK_SIZE];
	TArray<SWORD>	PCMData;

};
#endif // WITH TTS

/*-----------------------------------------------------------------------------
	USoundNode helper macros. 
-----------------------------------------------------------------------------*/

#define DECLARE_SOUNDNODE_ELEMENT(Type,Name)													\
	Type& Name = *((Type*)(Payload));															\
	Payload += sizeof(Type);														

#define DECLARE_SOUNDNODE_ELEMENT_PTR(Type,Name)												\
	Type* Name = (Type*)(Payload);																\
	Payload += sizeof(Type);														

#define	RETRIEVE_SOUNDNODE_PAYLOAD( Size )														\
		BYTE*	Payload					= NULL;													\
		UBOOL*	RequiresInitialization	= NULL;													\
		{																						\
			UINT* TempOffset = AudioComponent->SoundNodeOffsetMap.Find( this );					\
			UINT Offset;																		\
			if( !TempOffset )																	\
			{																					\
				Offset = AudioComponent->SoundNodeData.AddZeroed( Size + sizeof(UBOOL));		\
				AudioComponent->SoundNodeOffsetMap.Set( this, Offset );							\
				RequiresInitialization = (UBOOL*) &AudioComponent->SoundNodeData(Offset);		\
				*RequiresInitialization = 1;													\
				Offset += sizeof(UBOOL);														\
			}																					\
			else																				\
			{																					\
				RequiresInitialization = (UBOOL*) &AudioComponent->SoundNodeData(*TempOffset);	\
				Offset = *TempOffset + sizeof(UBOOL);											\
			}																					\
			Payload = &AudioComponent->SoundNodeData(Offset);									\
		}


#endif //_UNAUDIO_H_
