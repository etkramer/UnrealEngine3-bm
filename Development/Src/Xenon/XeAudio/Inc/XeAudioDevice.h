/*=============================================================================
	XeAudioDevice.h: Unreal XAudio audio interface object.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _INC_XEAUDIODEVICE
#define _INC_XEAUDIODEVICE

/*------------------------------------------------------------------------------------
	Dependencies, helpers & forward declarations.
------------------------------------------------------------------------------------*/

// Unreal includes.
#include <assert.h>

enum ChannelOutputs
{
	CHANNELOUT_FRONTLEFT = 0,
	CHANNELOUT_FRONTRIGHT,
	CHANNELOUT_FRONTCENTER,
	CHANNELOUT_LOWFREQUENCY,
	CHANNELOUT_BACKLEFT,
	CHANNELOUT_BACKRIGHT,

	CHANNELOUT_REVERB,
	CHANNELOUT_RADIO,
	CHANNELOUT_VOICECENTER,
	CHANNELOUT_COUNT
};

enum ESubmixEffects
{
	EFFECT_EQ,						// Used in sound modes
	EFFECT_RADIO,					// Used by Radio EQ submix voice
	EFFECT_LOWPASSFILTER,			// Used by low pass filter source effect
	EFFECT_COUNT
};

class UXeAudioDevice;
class FXeEQEffect;

/**
 * XAudio implementation of FSoundBuffer, containing the wave data and format information.
 */
class FXeSoundBuffer
{
public:
	/** 
	 * Constructor
	 *
	 * @param AudioDevice	audio device this sound buffer is going to be attached to.
	 */
	FXeSoundBuffer( UXeAudioDevice* AudioDevice );
	
	/**
	 * Destructor 
	 * 
	 * Frees wave data and detaches itself from audio device.
	 */
	~FXeSoundBuffer();

	/**
	 * Static function used to create a buffer.
	 *
	 * @param InWave		USoundNodeWave to use as template and wave source
	 * @param AudioDevice	audio device to attach created buffer to
	 * @return FXeSoundBuffer pointer if buffer creation succeeded, NULL otherwise
	 */
	static FXeSoundBuffer* Init( USoundNodeWave* InWave, UXeAudioDevice* AudioDevice );

	/**
	 * Returns the size of this buffer in bytes.
	 *
	 * @return Size in bytes
	 */
	INT GetSize();

	// Variables.

	/** Audio device this buffer is attached to	*/
	UXeAudioDevice*				AudioDevice;
	/** XAudio structure contain information about wave format like e.g. sample rate, bitdepth, ...	*/
	XAUDIOSOURCEFORMAT			Format;
	/** Array of XAudio packets this sound is split up into. Currently only a single packet.*/
	XAUDIOSOURCEBUFFER			SourceBuffer;
	/** Resource ID of associated USoundNodeWave */
	INT							ResourceID;
	/** Human readable name of resource, most likely name of UObject associated during caching.	*/
	FString						ResourceName;
	/** Whether memory for this buffer has been allocated from permanent pool. */
	UBOOL						bAllocationInPermanentPool;
	/** Number of channels in this XMA stream */
	INT							NumChannels;
};

/**
 * XAudio implementation of FSoundSource, the interface used to play, stop and update sources
 */
class FXeSoundSource : public FSoundSource
{
public:
	
	/**
	 * Constructor
	 *
	 * @param	InAudioDevice	audio device this source is attached to
	 */
	FXeSoundSource( UAudioDevice* InAudioDevice )
	:	FSoundSource( InAudioDevice ),
		Voice( NULL ),
		Buffer( NULL ),
		CurrentBuffer( 0 )
	{}

	/**
	 * Destructor, cleaning up voice
	 */
	virtual ~FXeSoundSource( void );

	// Initialization & update.

	/**
	 * Initializes a source with a given wave instance and prepares it for playback.
	 *
	 * @param	WaveInstance	wave instance being primed for playback
	 * @return	TRUE if initialization was successful, FALSE otherwise
	 */
	virtual UBOOL Init( FWaveInstance * WaveInstance );

	/**
	 * Updates the source specific parameter like e.g. volume and pitch based on the associated
	 * wave instance.	
	 */
	virtual void Update( void );

	// Playback.

	/**
	 * Plays the current wave instance.	
	 */
	virtual void Play( void );

	/**
	 * Stops the current wave instance and detaches it from the source.	
	 */
	virtual void Stop( void );

	/**
	 * Pauses playback of current wave instance.
	 */
	virtual void Pause( void );

	// Query.

	/**
	 * Queries the status of the currently associated wave instance.
	 *
	 * @return	TRUE if the wave instance/ source has finished playback and FALSE if it is 
	 *			currently playing or paused.
	 */
	virtual UBOOL IsFinished();

	/** 
	 * Maps a sound with a given number of channels to to expected speakers
	 */
	void MapSpeakers( XAUDIOCHANNELMAPENTRY * Entries );

protected:
	/** XAudio source voice associated with this source/ channel. */
	IXAudioSourceVoice* Voice;
	/** Cached sound buffer associated with currently bound wave instance. */
	FXeSoundBuffer*		Buffer;
	/** Which sound buffer should be written to next - used for double buffering. */
	INT					CurrentBuffer;
	/** A pair of sound buffers to allow notification when a sound loops. */
	XAUDIOSOURCEBUFFER	SourceBuffers[2];

	friend class UXeAudioDevice;
};

/** 
 *
 */
class FXeAudioEffectsManager : public FAudioEffectsManager
{
public:
	FXeAudioEffectsManager( UAudioDevice* InDevice );
	~FXeAudioEffectsManager( void );

	/** 
	 * Calls the platform specific code to set the parameters that define reverb
	 */
	virtual void SetReverbEffectParameters( const FAudioReverbEffect& ReverbEffectParameters );

	/** 
	 * Calls the platform specific code to set the parameters that define EQ
	 */
	virtual void SetEQEffectParameters( const FAudioEQEffect& ReverbEffectParameters );

	/** 
	 * Platform dependent call to update the sound output with new parameters
	 */
	virtual void* UpdateEffect( FSoundSource* Source );

	/** 
	 * returns the effect id of the low pass filter
	 */
	XAUDIOFXID GetLowPassFilterEffectId( void ) 
	{ 
		return( LowPassFilterEffectId ); 
	}

private:
	/** 
	 * Convert the standard reverb effect parameters (OpenAL based) to ones the Xbox 360 can use
	 */
	void ConvertToI3DL2( const FAudioReverbEffect& ReverbEffectParameters, XAUDIOREVERBI3DL2SETTINGS& ReverbI3DL2Parameters );

	/** 
	 * Acquires effect ids so that the filters can be used by the sound engine
	 */
	void CreateCustomEffects( void );

	/** 
	 * Utility function to create a submix voice
	 */
	void CreateSubmixVoice( XAUDIOFXID EffectId, XAUDIOCHANNELMAPENTRY ChannelMapEntry[], XAUDIOCHANNEL ChannelMapEntryCount, IXAudioSubmixVoice** SubmixVoice );

	/** 
	 * Creates the submix voice that handles the reverb effect
	 */
	void CreateReverbSubmixVoice( void );

	/** 
	 * Creates the submix voice that handles the radio effect
	 */
	void CreateRadioSubmixVoice( void );

	/** 
	 * Creates the buckets that the individual voices write to
	 */
	void CreateOutputSubmixVoice( IXAudioSubmixVoice** Voice );

	XAUDIOFXID					EQEffectId;
	XAUDIOFXID					RadioEffectId;
	XAUDIOFXID					LowPassFilterEffectId;

	IXAudioSubmixVoice*			ReverbSubmixVoice;
	IXAudioSubmixVoice*			RadioSubmixVoice;

	/**
	 * 0 - main dry output
	 * 1 - output with an EQ filter
	 */
	IXAudioSubmixVoice*			OutputSubmixVoice[2];
};

/**
 * XAudio implementation of an Unreal audio device. Neither use XACT nor X3DAudio.
 */
class UXeAudioDevice : public UAudioDevice
{
	DECLARE_CLASS(UXeAudioDevice,UAudioDevice,CLASS_Config|CLASS_Intrinsic,XeAudio)

	// Constructor.

	/**
	 * Static constructor, used to associate .ini options with member variables.	
	 */
	void StaticConstructor( void );

	// UAudioDevice interface.

	/**
	 * Initializes the audio device and creates sources.
	 *
	 * @warning: Relies on XAudioInitialize already being called
	 *
	 * @return TRUE if initialization was successful, FALSE otherwise
	 */
	virtual UBOOL Init( void );

	/**
	 * Update the audio device and calculates the cached inverse transform later
	 * on used for spatialization.
	 *
	 * @param	Realtime	whether we are paused or not
	 */
	virtual void Update( UBOOL Realtime );

	/**
	 * Precaches the passed in sound node wave object.
	 *
	 * @param	SoundNodeWave	Resource to be precached.
	 */
	virtual void Precache( USoundNodeWave* SoundNodeWave );

	/** 
	 * Lists all the loaded sounds and their memory footprint
	 */
	virtual void ListSounds( const TCHAR* Cmd, FOutputDevice& Ar );

	/**
	 * Frees the bulk resource data associated with this SoundNodeWave.
	 *
	 * @param	SoundNodeWave	wave object to free associated bulk data
	 */
	virtual void FreeResource( USoundNodeWave* SoundNodeWave );

	// UObject interface.

	/**
	 * Shuts down audio device. This will never be called with the memory image
	 * codepath.
	 */
	virtual void FinishDestroy( void );
	
	/**
	 * Special variant of Destroy that gets called on fatal exit. Doesn't really
	 * matter on the console so for now is just the same as Destroy so we can
	 * verify that the code correctly cleans up everything.
	 */
	virtual void ShutdownAfterError( void );

	// FExec interface.

	/**
	 * Exec handler used to parse console commands.
	 *
	 * @param	Cmd		Command to parse
	 * @param	Ar		Output device to use in case the handler prints anything
	 * @return	TRUE if command was handled, FALSE otherwise
	 */
	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar );

protected:

	/**
     * Allocates memory from permanent pool. This memory will NEVER be freed.
	 *
	 * @param	Size	Size of allocation.
	 *
	 * @return pointer to a chunk of memory with size Size
	 */
	void* AllocatePermanentMemory( INT Size );
	
	/**
	 * Tears down audio device by stopping all sounds, removing all buffers, 
	 * destroying all sources, ... Called by both Destroy and ShutdownAfterError
	 * to perform the actual tear down.
	 */
	void Teardown( void );

	// Variables.

	/** Array of all created buffers associated with this audio device */
	TArray<FXeSoundBuffer*>				Buffers;
	/** Look up associating a USoundNodeWave's resource ID with low level sound buffers	*/
	TMap<INT, FXeSoundBuffer*>	WaveBufferMap;
	/** Inverse listener transformation, used for spatialization */
	FMatrix								InverseTransform;
	/** Next resource ID value used for registering USoundNodeWave objects */
	INT									NextResourceID;

	friend class FXeSoundBuffer;
	friend class FXeSoundSource;
};

/**
 * Helper class for 5.1 spatialization.
 */
class FXeSpatializationHelper
{
	/** Instance of X3D used to calculate volume multipliers.	*/
	X3DAUDIO_HANDLE		          X3DInstance;
	
    X3DAUDIO_DSP_SETTINGS         DSPSettings;
    X3DAUDIO_LISTENER             Listener;
    X3DAUDIO_EMITTER              Emitter;
    X3DAUDIO_CONE                 Cone;
	
	X3DAUDIO_DISTANCE_CURVE_POINT VolumeCurvePoint[2];
	X3DAUDIO_DISTANCE_CURVE       VolumeCurve;
	
	X3DAUDIO_DISTANCE_CURVE_POINT ReverbVolumeCurvePoint[2];
	X3DAUDIO_DISTANCE_CURVE       ReverbVolumeCurve;

	FLOAT                         EmitterAzimuths;
	FLOAT					      MatrixCoefficients[XAUDIOSPEAKER_COUNT];
	
public:
	/**
	 * Constructor, initializing all member variables.
	 */
	FXeSpatializationHelper();

	/**
	* Calculates the spatialized volumes for each channel.
	*
	* @param	OrientFront				The listener's facing direction.
	* @param	ListenerPosition		The position of the listener.
	* @param	EmitterPosition			The position of the emitter.
	* @param	OutVolumes				An array of floats with one volume for each output channel.
	* @param	OutReverbLevel			The reverb volume
	*/
	void CalculateDolbySurroundRate( const FVector& OrientFront, const FVector& ListenerPosition, const FVector& EmitterPosition, FLOAT* OutVolumes, FLOAT& OutReverbLevel );
};

class FXeXMPHelper
{
private:
	/** Count of current cinematic audio clips playing (used to turn on/off XMP background music, allowing for overlap) */
	INT							CinematicAudioCount;
	/** Count of current movies playing (used to turn on/off XMP background music, NOT allowing for overlap) */
	BOOL						MoviePlaying;
	/** Flag indicating whether or not XMP playback is enabled (defaults to TRUE) */
	BOOL						XMPEnabled;
	/** Flag indicating whether or not XMP playback is blocked (defaults to FALSE)
	 *	Updated when player enters single-play:
	 *		XMP is blocked if the player hasn't finished the game before
	 */
	BOOL						XMPBlocked;

public:
	/**
	 * Constructor, initializing all member variables.
	 */
	FXeXMPHelper( void );
	/**
	* Destructor, performing final cleanup.
	*/
	~FXeXMPHelper( void );

	/**
	* Accessor for getting the XMPHelper class
	*/
	static FXeXMPHelper* GetXMPHelper();

	/**
	* Records that a cinematic audio track has started playing.
	*/
	void CinematicAudioStarted();
	/**
	* Records that a cinematic audio track has stopped playing.
	*/
	void CinematicAudioStopped();
	/**
	* Records that a movie has started playing.
	*/
	void MovieStarted();
	/**
	* Records that a movie has stopped playing.
	*/
	void MovieStopped();
	/**
	* Called with every movie/cinematic change to update XMP status if necessary
	*/
	void CountsUpdated();
	/**
	* Called to block XMP playback (when the gamer hasn't yet finished the game and enters single-play)
	*/
	void BlockXMP();
	/**
	* Called to unblock XMP playback (when the gamer has finished the game or exits single-play)
	*/
	void UnblockXMP();
};

//--------------------------------------------------------------------------------------
// Constants and defines
//--------------------------------------------------------------------------------------
const XAUDIOFXPARAMID EQFX_FREQUENCY_HF_ID           = 0;
const XAUDIOFXPARAMID EQFX_GAIN_HF_ID                = 1;
const XAUDIOFXPARAMID EQFX_CUTOFFFREQUENCY_MF_ID     = 2;
const XAUDIOFXPARAMID EQFX_BANDWIDTHFFREQUENCY_MF_ID = 3;
const XAUDIOFXPARAMID EQFX_GAIN_MF_ID                = 4;
const XAUDIOFXPARAMID EQFX_FREQUENCY_LF_ID           = 5;
const XAUDIOFXPARAMID EQFX_GAIN_LF_ID                = 6;

const XAUDIOFXPARAMID LPFX_HIGH_FREQUENCY_GAIN_ID    = 0;

const XAUDIOFXPARAMID RADIOFX_AGC_TARGET_ID			 = 0;
const XAUDIOFXPARAMID RADIOFX_COMP_THRESHOLD_ID		 = 1;
const XAUDIOFXPARAMID RADIOFX_COMP_RATIO_ID			 = 2;
const XAUDIOFXPARAMID RADIOFX_COMP_KNEE_ID			 = 3;
const XAUDIOFXPARAMID RADIOFX_COMP_MAKEUP_ID		 = 4;
const XAUDIOFXPARAMID RADIOFX_COMP_LIMIT_ID			 = 5;
const XAUDIOFXPARAMID RADIOFX_GAIN_ID				 = 6;

#ifndef ASSERT
#define ASSERT assert       // For XAUDIO_USE_BATCHALLOC_NEWDELETE macro
#endif

//--------------------------------------------------------------------------------------
// Declarations
//--------------------------------------------------------------------------------------

// Size functions.
HRESULT QuerySizeEQEffect( LPCXAUDIOFXINIT pInit, LPDWORD pEffectSize );
HRESULT QuerySizeLowPassFilterEffect( LPCXAUDIOFXINIT Init, LPDWORD EffectSize );
HRESULT QuerySizeRadioFilterEffect( LPCXAUDIOFXINIT Init, LPDWORD EffectSize );

// Effect creation functions.  Part of the data needed by XAudioInitialize().
HRESULT CreateEQEffect( const XAUDIOFXINIT* pInit, IXAudioBatchAllocator* pAllocator, IXAudioEffect** ppEffect );
HRESULT CreateLowPassFilterEffect( const XAUDIOFXINIT* Init, IXAudioBatchAllocator* Allocator, IXAudioEffect** Effect );
HRESULT CreateRadioFilterEffect( const XAUDIOFXINIT* Init, IXAudioBatchAllocator* Allocator, IXAudioEffect** Effect );

class FXeEQEffect : public IXAudioEffect
{
    XAUDIO_USE_BATCHALLOC_NEWDELETE();

    public:
        // IXAudioRefCount
        STDMETHOD_( ULONG, AddRef )( void );
        STDMETHOD_( ULONG, Release )( void );

        // IXAudioEffect    
        STDMETHOD( GetInfo )    ( XAUDIOFXINFO* Info );
        STDMETHOD( GetParam )   ( XAUDIOFXPARAMID ParamId, XAUDIOFXPARAMTYPE ParamType, XAUDIOFXPARAM* Param );
        STDMETHOD( SetParam )   ( XAUDIOFXPARAMID ParamId, XAUDIOFXPARAMTYPE ParamType, const XAUDIOFXPARAM* Param );
        STDMETHOD( GetContext ) ( LPVOID* OutContext );
        STDMETHOD( Process )    ( IXAudioFrameBuffer* InputBuffer, IXAudioFrameBuffer* OutputBuffer );

        // FXeEQEffect
        FXeEQEffect( LPVOID Context );
		/**
		 * Applies the specified effect parameters to the current effect.  Interpolation will occur during the next Process call.
		 */
		void SetEffectConstants( void );

    private:
        LPVOID  Context;
        DWORD   RefCount;

        // Effect Parameters
		FAudioEQEffect	EQEffectParams;

		UBOOL	bDirty;

        // Precomputed values
        UBOOL   ComputeHF;
        FLOAT   HF_k;
        FLOAT   HF_v0;
        FLOAT   HF_h02;
        FLOAT   HF_a;
    
        UBOOL   ComputeMF;
        FLOAT   MF_a;
        FLOAT   MF_k;
        FLOAT   MF_v0;
        FLOAT   MF_h02;
        FLOAT   MF_d;

        UBOOL   ComputeLF;
        FLOAT   LF_k;
        FLOAT   LF_v0;
        FLOAT   LF_h02;
        FLOAT   LF_a;

        // Values stored across frames
        FLOAT   MF_xn1;
        FLOAT   MF_xn2;
        FLOAT   MF_y1n1;
        FLOAT   MF_y1n2;

        FLOAT   HFDelay;
        FLOAT   LFDelay;
};

class FXeLowPassFilterEffect: public IXAudioEffect
{
	XAUDIO_USE_BATCHALLOC_NEWDELETE();

public:
	// IXAudioRefCount
	STDMETHOD_( ULONG, AddRef )( void );
	STDMETHOD_( ULONG, Release )( void );

	// IXAudioEffect    
	STDMETHOD( GetInfo )    ( XAUDIOFXINFO* Info );
	STDMETHOD( GetParam )   ( XAUDIOFXPARAMID ParamId, XAUDIOFXPARAMTYPE ParamType, XAUDIOFXPARAM* Param );
	STDMETHOD( SetParam )   ( XAUDIOFXPARAMID ParamId, XAUDIOFXPARAMTYPE ParamType, const XAUDIOFXPARAM* Param );
	STDMETHOD( GetContext ) ( LPVOID* OutContext );
	STDMETHOD( Process )    ( IXAudioFrameBuffer* InputBuffer, IXAudioFrameBuffer* OutputBuffer );

	// FXeEQEffect
	FXeLowPassFilterEffect( LPVOID Context );

private:
	LPVOID  Context;
	DWORD   RefCount;

	FLOAT	HighFrequencyGain;

	FLOAT   LF_k;
};

class FXeRadioFilterEffect: public IXAudioEffect
{
	XAUDIO_USE_BATCHALLOC_NEWDELETE();

public:
	// IXAudioRefCount
	STDMETHOD_( ULONG, AddRef )( void );
	STDMETHOD_( ULONG, Release )( void );

	// IXAudioEffect    
	STDMETHOD( GetInfo )    ( XAUDIOFXINFO* Info );
	STDMETHOD( GetParam )   ( XAUDIOFXPARAMID ParamId, XAUDIOFXPARAMTYPE ParamType, XAUDIOFXPARAM* Param );
	STDMETHOD( SetParam )   ( XAUDIOFXPARAMID ParamId, XAUDIOFXPARAMTYPE ParamType, const XAUDIOFXPARAM* Param );
	STDMETHOD( GetContext ) ( LPVOID* OutContext );
	STDMETHOD( Process )    ( IXAudioFrameBuffer* InputBuffer, IXAudioFrameBuffer* OutputBuffer );

	// FXeRadioFilterEffect
	FXeRadioFilterEffect( LPVOID Context );

	void CalcCompressorCoefficients( float t, float r, float k );
	void Waveshaper( XAUDIOSAMPLE* pIn, XAUDIOSAMPLE* pOut, UBOOL GainReduction );
	void CalcBandPassCoefficients( float f, float q, float g );

private:
	LPVOID  Context;
	DWORD   RefCount;

	FLOAT	AGCTarget;
	FLOAT	CompThreshold;
	FLOAT	CompRatio;
	FLOAT	CompKnee;
	FLOAT	CompMakeup;
	FLOAT	CompLimit;
	FLOAT	BPFFreq;
	FLOAT	BPFQ;
	FLOAT	BPFGain;
	FLOAT	Gain;

	UBOOL	bParamsUpdated;

	FLOAT	CoeffA;
	FLOAT	CoeffB;
	FLOAT	CoeffC;
	FLOAT	CoeffD;

	FLOAT	CoeffAB0;
	FLOAT	CoeffA1;
	FLOAT	CoeffA2;
	FLOAT	CoeffB1;
	FLOAT	CoeffB2;

	FLOAT	InLastSample;
	FLOAT	InLastLastSample;
	FLOAT	OutLastSample;
	FLOAT	OutLastLastSample;

	INT		InputAmplitudeIndex;
	FLOAT	InputAmplitudeAccumulator;
	FLOAT	InputAmplitudeWindow[256];

	INT		OutputAmplitudeIndex;
	FLOAT	OutputAmplitudeAccumulator;
	FLOAT	OutputAmplitudeWindow[256];

	FLOAT	TransferFunction[XAUDIOFRAMESIZE_NATIVE];
};

#endif
