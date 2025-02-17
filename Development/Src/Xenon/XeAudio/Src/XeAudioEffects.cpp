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
	FXeAudioEffectsManager.
------------------------------------------------------------------------------------*/

/** Work data required by the radio filter */
static FLOAT DefaultTransferTable[XAUDIOFRAMESIZE_NATIVE] = { 0 };

/**
 * Acquires effect ids so that the filters can be used by the sound engine
 */
void FXeAudioEffectsManager::CreateCustomEffects( void )
{
	XAUDIOFXTABLEENTRY Effects[EFFECT_COUNT];

	for( INT i = 0; i < XAUDIOFRAMESIZE_NATIVE; ++i )
	{
		DefaultTransferTable[i] = ( ( FLOAT ) i ) / ( FLOAT )XAUDIOFRAMESIZE_NATIVE;
	}

	// Set by call to XAudioRegisterEffects
	Effects[EFFECT_EQ].EffectId = 0;
	Effects[EFFECT_EQ].pfnCreateEffect = CreateEQEffect;
	Effects[EFFECT_EQ].pfnQueryEffectSize = QuerySizeEQEffect;

	// Set by call to XAudioRegisterEffects
	Effects[EFFECT_RADIO].EffectId = 0;
	Effects[EFFECT_RADIO].pfnCreateEffect = CreateRadioFilterEffect;
	Effects[EFFECT_RADIO].pfnQueryEffectSize = QuerySizeRadioFilterEffect;

	// Set by call to XAudioRegisterEffects
	Effects[EFFECT_LOWPASSFILTER].EffectId = 0;
	Effects[EFFECT_LOWPASSFILTER].pfnCreateEffect = CreateLowPassFilterEffect;
	Effects[EFFECT_LOWPASSFILTER].pfnQueryEffectSize = QuerySizeLowPassFilterEffect;

	XAUDIOFXREGISTER Register;
	Register.EffectCount = ARRAY_COUNT( Effects );
	Register.paEffects = Effects;
	XAudioRegisterEffects( &Register ); 

	EQEffectId = Effects[EFFECT_EQ].EffectId;
	RadioEffectId = Effects[EFFECT_RADIO].EffectId;
	LowPassFilterEffectId = Effects[EFFECT_LOWPASSFILTER].EffectId;
}

/**
 * Utility function to create a submix voice
 */
void FXeAudioEffectsManager::CreateSubmixVoice( XAUDIOFXID EffectId, XAUDIOCHANNELMAPENTRY ChannelMapEntry[], XAUDIOCHANNEL ChannelMapEntryCount, IXAudioSubmixVoice** SubmixVoice )
{
	if( *SubmixVoice )
	{
		( *SubmixVoice )->Release();
	}

	// Create a submix voice to handle an effect
	XAUDIOSUBMIXVOICEINIT SubmixVoiceInit = { 0 };

	// Single channel input
	SubmixVoiceInit.Format.ChannelCount = 1;
	// Single voice output
	SubmixVoiceInit.MaxOutputVoiceCount = 1;
	// Route to relevant speakers
	SubmixVoiceInit.MaxChannelMapEntryCount = ChannelMapEntryCount;
	// Set as processing stage 1
	SubmixVoiceInit.SubmixStage = 1;

	// Add effect to the chain
	XAUDIOFXINIT EffectInit = { EffectId, NULL };

	// Effect chain for source voice initialization
	const XAUDIOFXINIT* SourceVoiceEffectsInit[] = { &EffectInit };
	XAUDIOVOICEFXCHAIN SourceEffectsChain = { 1, SourceVoiceEffectsInit };
	SubmixVoiceInit.pEffectChain = &SourceEffectsChain;

	XAUDIOCHANNELMAP ChannelMap;
	ChannelMap.EntryCount = ChannelMapEntryCount;
	ChannelMap.paEntries = ChannelMapEntry;

	XAUDIOVOICEOUTPUTENTRY OutputEntry;
	OutputEntry.pDestVoice = NULL;
	OutputEntry.pChannelMap = &ChannelMap;

	XAUDIOVOICEOUTPUT Output;
	Output.EntryCount = 1;
	Output.paEntries = &OutputEntry;
	SubmixVoiceInit.pVoiceOutput = &Output;

	if( FAILED( XAudioCreateSubmixVoice( &SubmixVoiceInit, SubmixVoice ) ) )
	{
		debugf( NAME_Error, TEXT( "XAudioSubmixVoice for effect could not be created." ) );
	}
}

/**
 * Creates the submix voice that handles the reverb effect
 */
void FXeAudioEffectsManager::CreateReverbSubmixVoice( void )
{
	// Simple direct mapping to all speakers
	XAUDIOCHANNELMAPENTRY ChannelMapEntry[] =
	{
		{ 0, CHANNELOUT_FRONTLEFT, XAUDIOVOLUME_MAX },
		{ 1, CHANNELOUT_FRONTRIGHT, XAUDIOVOLUME_MAX },
		{ 2, CHANNELOUT_FRONTCENTER, XAUDIOVOLUME_MAX },
		{ 3, CHANNELOUT_LOWFREQUENCY, XAUDIOVOLUME_MAX },
		{ 4, CHANNELOUT_BACKLEFT, XAUDIOVOLUME_MAX },
		{ 5, CHANNELOUT_BACKRIGHT, XAUDIOVOLUME_MAX },
	};

	CreateSubmixVoice( XAUDIOFXID_REVERB, ChannelMapEntry, ARRAY_COUNT( ChannelMapEntry ), &ReverbSubmixVoice );
}

/**
 * Creates the submix voice that handles the radio effect
 */
void FXeAudioEffectsManager::CreateRadioSubmixVoice( void )
{
	// Just map the front center speakers
	XAUDIOCHANNELMAPENTRY ChannelMapEntry[] =
	{
		{ 0, CHANNELOUT_FRONTCENTER, XAUDIOVOLUME_MAX },
	};

	CreateSubmixVoice( RadioEffectId, ChannelMapEntry, ARRAY_COUNT( ChannelMapEntry ), &RadioSubmixVoice );
}

/**
 * Creates the buckets that the individual voices write to
 */
void FXeAudioEffectsManager::CreateOutputSubmixVoice( IXAudioSubmixVoice** SubmixVoice )
{
	XAUDIOSUBMIXVOICEINIT SoundModeSubmixVoiceInit = { 0 };

	SoundModeSubmixVoiceInit.Format.ChannelCount = CHANNELOUT_COUNT;
	// One for dry, one for reverb, one for radio
	SoundModeSubmixVoiceInit.MaxOutputVoiceCount = 3;
	// Set max channel map count = Max - reverb - radio
	SoundModeSubmixVoiceInit.MaxChannelMapEntryCount = CHANNELOUT_COUNT - 2;
	SoundModeSubmixVoiceInit.SubmixStage = 0;

	// Initialize with the the proper EQ settings for this sound mode
	XAUDIOFXINIT EffectInit[1] = { 0 };
	EffectInit[0].EffectId = EQEffectId;
	EffectInit[0].pContext = NULL;

	const XAUDIOFXINIT* SourceEffectsInit[1] = { &EffectInit[0] };
	XAUDIOVOICEFXCHAIN EffectChain = { 0 };
	EffectChain.EffectCount = ( XAUDIOVOICEFXINDEX )ARRAY_COUNT( SourceEffectsInit );
	EffectChain.papEffects = SourceEffectsInit;

	SoundModeSubmixVoiceInit.pEffectChain = &EffectChain;

	XAUDIOCHANNELMAPENTRY DryChannelMapEntry[] =
	{
		{ CHANNELOUT_FRONTLEFT,		XAUDIOSPEAKER_FRONTLEFT,	XAUDIOVOLUME_MAX },
		{ CHANNELOUT_FRONTRIGHT,	XAUDIOSPEAKER_FRONTRIGHT,	XAUDIOVOLUME_MAX },
		{ CHANNELOUT_FRONTCENTER,	XAUDIOSPEAKER_FRONTCENTER,	XAUDIOVOLUME_MAX },
		{ CHANNELOUT_LOWFREQUENCY,	XAUDIOSPEAKER_LOWFREQUENCY,	XAUDIOVOLUME_MAX },
		{ CHANNELOUT_BACKLEFT,		XAUDIOSPEAKER_BACKLEFT,		XAUDIOVOLUME_MAX },
		{ CHANNELOUT_BACKRIGHT,		XAUDIOSPEAKER_BACKRIGHT,	XAUDIOVOLUME_MAX },

		{ CHANNELOUT_VOICECENTER,	XAUDIOSPEAKER_FRONTCENTER,	XAUDIOVOLUME_MAX },
	};

	XAUDIOCHANNELMAPENTRY ReverbChannelMapEntry[] =
	{
		{ CHANNELOUT_REVERB,		0,							XAUDIOVOLUME_MAX },
	};

	XAUDIOCHANNELMAPENTRY RadioChannelMapEntry[] =
	{
		{ CHANNELOUT_RADIO,			0,							XAUDIOVOLUME_MAX },
	};

	XAUDIOCHANNELMAP DryChannelMap;
	DryChannelMap.EntryCount = ARRAY_COUNT( DryChannelMapEntry );
	DryChannelMap.paEntries = DryChannelMapEntry;

	XAUDIOCHANNELMAP ReverbChannelMap;
	ReverbChannelMap.EntryCount = ARRAY_COUNT( ReverbChannelMapEntry );
	ReverbChannelMap.paEntries = ReverbChannelMapEntry;

	XAUDIOCHANNELMAP RadioChannelMap;
	RadioChannelMap.EntryCount = ARRAY_COUNT( RadioChannelMapEntry );
	RadioChannelMap.paEntries = RadioChannelMapEntry;

	XAUDIOVOICEOUTPUTENTRY OutputEntry[3];
	OutputEntry[0].pDestVoice = NULL;
	OutputEntry[0].pChannelMap = &DryChannelMap;

	OutputEntry[1].pDestVoice = ReverbSubmixVoice;
	OutputEntry[1].pChannelMap = &ReverbChannelMap;

	OutputEntry[2].pDestVoice = RadioSubmixVoice;
	OutputEntry[2].pChannelMap = &RadioChannelMap;

	XAUDIOVOICEOUTPUT Output;
	Output.EntryCount = 3;
	Output.paEntries = OutputEntry;

	SoundModeSubmixVoiceInit.pVoiceOutput = &Output;

	if( FAILED( XAudioCreateSubmixVoice( &SoundModeSubmixVoiceInit, SubmixVoice ) ) )
	{
		debugf( NAME_Error, TEXT( "XAudioSubmixVoice for sound modes could not be created." ) );
	}
}

/**
 * Platform dependent call to update the sound output with new parameters
 */
void* FXeAudioEffectsManager::UpdateEffect( FSoundSource* Source )
{
	if( Source->IsEQFilterApplied() || AudioDevice->IsTestingEQFilter() )
	{
		return( OutputSubmixVoice[1] );
	}

	return( OutputSubmixVoice[0] );
}

/**
 * Create a submix voice to apply a reverb effect
 */
FXeAudioEffectsManager::FXeAudioEffectsManager( UAudioDevice* InDevice )
	: FAudioEffectsManager( InDevice )
{
	UXeAudioDevice* AudioDevice = ( UXeAudioDevice* )InDevice;

	RadioSubmixVoice = NULL;
	ReverbSubmixVoice = NULL;
	OutputSubmixVoice[0] = NULL;
	OutputSubmixVoice[1] = NULL;

	CreateCustomEffects();

	CreateReverbSubmixVoice();
	CreateRadioSubmixVoice();

	CreateOutputSubmixVoice( &OutputSubmixVoice[0] );
	CreateOutputSubmixVoice( &OutputSubmixVoice[1] );
}

/**
 * Clean up
 */
FXeAudioEffectsManager::~FXeAudioEffectsManager( void )
{
	XAUDIOFXID Effects[EFFECT_COUNT];

	Effects[EFFECT_EQ] = EQEffectId;
	Effects[EFFECT_RADIO] = RadioEffectId;

	XAUDIOFXUNREGISTER Unregister;
	Unregister.EffectCount = ARRAY_COUNT( Effects );
	Unregister.paEffectIds = Effects;
	XAudioUnregisterEffects( &Unregister );

	OutputSubmixVoice[0]->Release();
	OutputSubmixVoice[1]->Release();

	ReverbSubmixVoice->Release();
	RadioSubmixVoice->Release();
}

/**
 * Convert the generic reverb parameters to Xbox 360 specific
 */
void FXeAudioEffectsManager::ConvertToI3DL2( const FAudioReverbEffect & ReverbEffectParameters, XAUDIOREVERBI3DL2SETTINGS & I3DL2Parameters )
{
	I3DL2Parameters.lRoom = VolumeToMilliBels( ReverbEffectParameters.Gain );
	I3DL2Parameters.lRoomHF = VolumeToMilliBels( ReverbEffectParameters.GainHF );
	I3DL2Parameters.flRoomRolloffFactor = ReverbEffectParameters.RoomRolloffFactor;
	I3DL2Parameters.flDecayTime = ReverbEffectParameters.DecayTime;
	I3DL2Parameters.flDecayHFRatio = ReverbEffectParameters.DecayHFRatio;
	I3DL2Parameters.lReflections = VolumeToMilliBels( ReverbEffectParameters.ReflectionsGain );
	I3DL2Parameters.flReflectionsDelay = ReverbEffectParameters.ReflectionsDelay;
	I3DL2Parameters.lReverb = VolumeToMilliBels( ReverbEffectParameters.LateGain );
	I3DL2Parameters.flReverbDelay = ReverbEffectParameters.LateDelay;
	I3DL2Parameters.flDiffusion = ReverbEffectParameters.Diffusion * 100.0f;
	I3DL2Parameters.flDensity = ReverbEffectParameters.Density * 100.0f;
	I3DL2Parameters.flHFReference = 5000.0f;
}

/**
 * Applies the generic reverb parameters to the Xbox 360 hardware
 */
void FXeAudioEffectsManager::SetReverbEffectParameters( const FAudioReverbEffect& ReverbEffectParameters )
{
	// Set up the reverb parameters
	XAUDIOFXPARAM ReverbParam;
	XAUDIOREVERBI3DL2SETTINGS ReverbParameters;

	ConvertToI3DL2( ReverbEffectParameters, ReverbParameters );

	ReverbParam.Data.pBuffer = ( LPVOID )&ReverbParameters;
	ReverbParam.Data.BufferSize = sizeof( ReverbParameters );
	if( FAILED( ReverbSubmixVoice->SetEffectParam( 0, XAUDIOFXPARAMID_REVERB_I3DL2SETTINGS, XAUDIOFXPARAMTYPE_DATA, &ReverbParam ) ) )
	{
		debugf( NAME_Warning, TEXT( "In FXeReverbVoice::SetReverbPreset : SubmixVoice->SetEffectParam failed" ) );
	}

	// Set the volume of the reverb
	XAUDIOCHANNELVOLUMEENTRY ChannelVolumeEntries[XAUDIOSPEAKER_COUNT];
	XAUDIOCHANNELVOLUME ChannelVolume;
	XAUDIOVOICEOUTPUTVOLUMEENTRY VoiceEntries[1];
	XAUDIOVOICEOUTPUTVOLUME OutputVolume;

	for( INT i = 0; i < XAUDIOSPEAKER_COUNT; i++ )
	{
		ChannelVolumeEntries[i].EntryIndex = i;
		ChannelVolumeEntries[i].Volume = XAUDIOVOLUME_MAX * ReverbEffectParameters.Volume;
	}

	ChannelVolume.EntryCount = XAUDIOSPEAKER_COUNT;
	ChannelVolume.paEntries = ChannelVolumeEntries;

	VoiceEntries[0].OutputVoiceIndex = 0;
	VoiceEntries[0].pVolume = &ChannelVolume;

	OutputVolume.EntryCount = 1;
	OutputVolume.paEntries = VoiceEntries;

	ReverbSubmixVoice->SetVoiceOutputVolume( &OutputVolume );
}

void FXeAudioEffectsManager::SetEQEffectParameters( const FAudioEQEffect& EQEffectParameters )
{
	OutputSubmixVoice[1]->SetEffectParam( 0, EQFX_FREQUENCY_HF_ID, XAUDIOFXPARAMTYPE_NUMERIC, ( XAUDIOFXPARAM* )&EQEffectParameters.HFFrequency );
	FLOAT HFGain = ( FLOAT )VolumeToDeciBels( EQEffectParameters.HFGain );
	OutputSubmixVoice[1]->SetEffectParam( 0, EQFX_GAIN_HF_ID, XAUDIOFXPARAMTYPE_NUMERIC, ( XAUDIOFXPARAM* )&HFGain );
	OutputSubmixVoice[1]->SetEffectParam( 0, EQFX_CUTOFFFREQUENCY_MF_ID, XAUDIOFXPARAMTYPE_NUMERIC, ( XAUDIOFXPARAM* )&EQEffectParameters.MFCutoffFrequency );
	OutputSubmixVoice[1]->SetEffectParam( 0, EQFX_BANDWIDTHFFREQUENCY_MF_ID, XAUDIOFXPARAMTYPE_NUMERIC, ( XAUDIOFXPARAM* )&EQEffectParameters.MFBandwidthFrequency );
	FLOAT MFGain = ( FLOAT )VolumeToDeciBels( EQEffectParameters.MFGain );
	OutputSubmixVoice[1]->SetEffectParam( 0, EQFX_GAIN_MF_ID, XAUDIOFXPARAMTYPE_NUMERIC, ( XAUDIOFXPARAM* )&MFGain );
	OutputSubmixVoice[1]->SetEffectParam( 0, EQFX_FREQUENCY_LF_ID, XAUDIOFXPARAMTYPE_NUMERIC, ( XAUDIOFXPARAM* )&EQEffectParameters.LFFrequency );
	FLOAT LFGain = ( FLOAT )VolumeToDeciBels( EQEffectParameters.LFGain );
	OutputSubmixVoice[1]->SetEffectParam( 0, EQFX_GAIN_LF_ID, XAUDIOFXPARAMTYPE_NUMERIC, ( XAUDIOFXPARAM* )&LFGain );
}

/*------------------------------------------------------------------------------------
	FXeEQEffect.
------------------------------------------------------------------------------------*/

const FLOAT TWO_PI = 2.0f * PI;
const FLOAT SAMPLEPERMSEC = 48.0f;

/**
 * Increments the ref count.
 */
STDMETHODIMP_( ULONG ) FXeEQEffect::AddRef( void )
{
	return( ++RefCount );
}

/**
 * Decrements the ref count and deletes if necessary.
 */
STDMETHODIMP_( ULONG ) FXeEQEffect::Release( void )
{
	if( 0 == --RefCount )
	{
		delete this;
		return( 0 );
	}

	return( RefCount );
}

/**
 * Returns info used by XAudio - Called by XAudio during effect creation.
 *
 * @param	Info				The XAUDIOFXINFO structure containing effect configuration info.
 */
STDMETHODIMP FXeEQEffect::GetInfo( XAUDIOFXINFO* Info )
{
	check( Info );
	Info->DataFlow = XAUDIODATAFLOW_INPLACE;
	Info->TrailFrames = 0;
	return S_OK;
}

/**
 * Allows retrieval of effect parameters .
 *
 * @param	ParamId					The client-defined parameter ID.
 * @param	ParamType				The parameter type.
 * @param	Param					A pointer to the parameter variable into which to write the value.
 */
STDMETHODIMP FXeEQEffect::GetParam( XAUDIOFXPARAMID ParamId, XAUDIOFXPARAMTYPE ParamType, XAUDIOFXPARAM* Param )
{
	check( Param );
	HRESULT hr = S_OK;

	XAudioLock( XAUDIOLOCKTYPE_LOCK | XAUDIOLOCKTYPE_SPINLOCK );
	switch( ParamId )
	{
	case EQFX_FREQUENCY_HF_ID:
		Param->Value = EQEffectParams.HFFrequency;
		break;
	case EQFX_GAIN_HF_ID:
		Param->Value = EQEffectParams.HFGain;
		break;
	case EQFX_CUTOFFFREQUENCY_MF_ID:
		Param->Value = EQEffectParams.MFCutoffFrequency;
		break;
	case EQFX_BANDWIDTHFFREQUENCY_MF_ID:
		Param->Value = EQEffectParams.MFBandwidthFrequency;
		break;
	case EQFX_GAIN_MF_ID:
		Param->Value = EQEffectParams.MFGain;
		break;
	case EQFX_FREQUENCY_LF_ID:
		Param->Value = EQEffectParams.LFFrequency;
		break;
	case EQFX_GAIN_LF_ID:
		Param->Value = EQEffectParams.LFGain;
		break;
	default:
		hr = E_FAIL;
		break;
	}
	XAudioLock( XAUDIOLOCKTYPE_UNLOCK | XAUDIOLOCKTYPE_SPINLOCK );

	return hr;
}

/**
 * Allows setting of effect parameters.
 *
 * @param	ParamId					The client-defined parameter ID.
 * @param	ParamType				The parameter type.
 * @param	Param					The value to set for the given parameter.
 */
STDMETHODIMP FXeEQEffect::SetParam( XAUDIOFXPARAMID ParamId, XAUDIOFXPARAMTYPE ParamType, const XAUDIOFXPARAM* Param )
{
	HRESULT hr = S_OK;

	check( Param );
	XAudioLock( XAUDIOLOCKTYPE_LOCK | XAUDIOLOCKTYPE_SPINLOCK );

	switch( ParamId )
	{
	case EQFX_FREQUENCY_HF_ID:
		if( EQEffectParams.HFFrequency != Param->Value )
		{
			EQEffectParams.HFFrequency = Param->Value;
			bDirty = TRUE;
		}
		break;
	case EQFX_GAIN_HF_ID:
		if( EQEffectParams.HFGain != Param->Value )
		{
			EQEffectParams.HFGain = Param->Value;
			bDirty = TRUE;
		}
		break;
	case EQFX_CUTOFFFREQUENCY_MF_ID:
		if( EQEffectParams.MFCutoffFrequency != Param->Value )
		{
			EQEffectParams.MFCutoffFrequency = Param->Value;
			bDirty = TRUE;
		}
		break;
	case EQFX_BANDWIDTHFFREQUENCY_MF_ID:
		if( EQEffectParams.MFBandwidthFrequency != Param->Value )
		{
			EQEffectParams.MFBandwidthFrequency = Param->Value;
			bDirty = TRUE;
		}
		break;
	case EQFX_GAIN_MF_ID:
		if( EQEffectParams.MFGain != Param->Value )
		{
			EQEffectParams.MFGain = Param->Value;
			bDirty = TRUE;
		}
		break;
	case EQFX_FREQUENCY_LF_ID:
		if( EQEffectParams.LFFrequency != Param->Value )
		{
			EQEffectParams.LFFrequency = Param->Value;
			bDirty = TRUE;
		}
		break;
	case EQFX_GAIN_LF_ID:
		if( EQEffectParams.LFGain != Param->Value)
		{
			EQEffectParams.LFGain = Param->Value;
			bDirty = TRUE;
		}
		break;
	default:
		hr = E_FAIL;
		break;
	}

	XAudioLock( XAUDIOLOCKTYPE_UNLOCK | XAUDIOLOCKTYPE_SPINLOCK );

	return hr;
}

/**
 * Returns the context this effect was created with.
 *
 * @param	OutContext				A pointer to the context structure passed into the constructor.
 */
STDMETHODIMP FXeEQEffect::GetContext( LPVOID* OutContext )
{
	check( OutContext );
	*OutContext = Context;
	return( S_OK );
}

/**
 * Constructs a new instance of FXeEQEffect.  Called not on register but upon applying the effect to a specific voice.
 *
 * @param	NewContext				Used to pass initialization info to effect constructor.  Currently unused.
 */
FXeEQEffect::FXeEQEffect( LPVOID NewContext )
{
    Context = NewContext;
    RefCount = 1;

	bDirty = TRUE;

    // Precomputed values
    ComputeHF = TRUE;
    HF_k = 0.0f;
    HF_v0 = 0.0f;
    HF_h02 = 0.0f;
    HF_a = 0.0f;
    ComputeMF = TRUE;
    MF_a = 0.0f;
    MF_k = 0.0f;
    MF_v0 = 0.0f;
    MF_h02 = 0.0f;
    MF_d = 0.0f;
    ComputeLF = TRUE;
    LF_k = 0.0f;
    LF_v0 = 0.0f;
    LF_h02 = 0.0f;
    LF_a = 0.0f;

    // Values stored across frames
    MF_xn1 = 0.0f;
    MF_xn2 = 0.0f;
    MF_y1n1 = 0.0f;
    MF_y1n2 = 0.0f;

    HFDelay = 0.0f;
    LFDelay = 0.0f;
}

/**
 * Creates the EQ effect.
 *
 * @param	Init					A pointer to the XAudio effect init data.
 * @param	Allocator				A pointer to the XAudio batch allocator.
 * @param	Effect					A double pointer into which the effect's pointer can be stored.
 */
HRESULT CreateEQEffect( const XAUDIOFXINIT* Init, IXAudioBatchAllocator* Allocator, IXAudioEffect** Effect )
{
	check( Init );
	check( Effect );
	check( Allocator );

	HRESULT hr = S_OK;
	FXeEQEffect* UserEffect = NULL;

	UserEffect = XAUDIO_BATCHALLOC_NEW( FXeEQEffect( Init->pContext ), Allocator );

	*Effect = UserEffect;
	return( hr );
}

/**
 * Returns the size of the EQ effect.
 */
HRESULT QuerySizeEQEffect( LPCXAUDIOFXINIT Init, LPDWORD EffectSize )
{
	check( Init );
	check( EffectSize );

	*EffectSize = sizeof( FXeEQEffect );
	return( S_OK );
}

/**
 * Updates the volume of the input buffer and submits it back to the output buffer.
 *
 * @param	InputBuffer				The XAudio input buffer for the effect.
 * @param	OutputBuffer			The XAudio output buffer for the effect.
 */
STDMETHODIMP FXeEQEffect::Process( IXAudioFrameBuffer* InputBuffer, IXAudioFrameBuffer* OutputBuffer )
{
    check( InputBuffer );
    check( !OutputBuffer );

	SetEffectConstants();

	if( !ComputeHF && !ComputeMF && !ComputeLF )
	{
		return( S_OK );
	}

	const XAUDIOFRAMESIZE SampleCount = XAUDIOFRAMESIZE_NATIVE;
	const FLOAT InterpStepSize = 1.0f / ( FLOAT )SampleCount ;		// Should never be zero

    // Verify the format is "internal".  It should always be internal (float)
    XAUDIOFRAMEBUFDATA FrameBufferData;
    InputBuffer->GetProcessingData( &FrameBufferData );
    check( XAUDIOSAMPLETYPE_NATIVE == FrameBufferData.Format.SampleType );

    FLOAT HF_xhn = 0.0f;
    FLOAT MF_y1n = 0.0f;
    FLOAT MF_yn = 0.0f;
    FLOAT LF_xhn = 0.0f;

    // Go through each channel
    for( INT i = 0; i < SampleCount; ++i )
    {
		for( INT j = 0; j < FrameBufferData.Format.ChannelCount; ++j )
		{
            FLOAT* Sample = &( FrameBufferData.pSampleBuffer[j * SampleCount + i] );

            // High Frequency (Shelving Filter)
            if( ComputeHF )
            {
                HF_xhn = *Sample - ( HF_a * HFDelay );
                *Sample = HF_h02 * ( *Sample - ( ( HF_xhn * HF_a ) + HFDelay ) ) + *Sample;
                HFDelay = HF_xhn;
            }

            // Middle Frequency (Peak Filter)
            if( ComputeMF )
            {
                MF_y1n = ( -MF_a * *Sample ) + ( MF_d * MF_xn1 ) + MF_xn2 - ( MF_d * MF_y1n1 ) + ( MF_a * MF_y1n2 );
                MF_yn  = MF_h02 * ( *Sample - MF_y1n ) + *Sample;

                // Update the samples
                MF_xn2 = MF_xn1;
                MF_xn1 = *Sample;
                MF_y1n2 = MF_y1n1;
                MF_y1n1 = MF_y1n;

                // Save the result
                *Sample = MF_yn;
            }

            // Low Frequency (Shelving Filter)
            if( ComputeLF )
            {
                LF_xhn = *Sample - ( LF_a * LFDelay );
                *Sample = LF_h02 * ( *Sample + ( ( LF_xhn * LF_a ) + LFDelay ) ) + *Sample;
                LFDelay = LF_xhn;
            }
        }
    }

    return( S_OK );
}

/**
 * Applies the specified effect parameters to the current effect.  Interpolation will occur during the next Process call.
 */
void FXeEQEffect::SetEffectConstants( void )
{
	// Note that the Volume levels (0.0f to 1.0f) have already been converted to gain (dB) by this point

	if( !bDirty )
	{
		return;
	}

	bDirty = FALSE;

    // If one of the high frequency parameters was updated, recalculate it's values
	ComputeHF = ( EQEffectParams.HFGain != 0.0f );

    HF_k = appTan( PI * EQEffectParams.HFFrequency / XAUDIOSAMPLERATE_NATIVE );
    HF_v0 = appPow( 10.0f, EQEffectParams.HFGain / 20.0f );
    HF_h02 = ( HF_v0 - 1.0f ) / 2.0f;

    if( EQEffectParams.HFGain > 0.0f )
	{
		// Boost
        HF_a = ( HF_k - 1.0f ) / ( HF_k + 1.0f );
	}
    else
	{
		// Cut
        HF_a = ( ( HF_v0 * HF_k ) - 1.0f ) / ( ( HF_v0 * HF_k ) + 1.0f );
	}

	// If one of the middle frequency parameters was updated, recalculate it's values
	ComputeMF = ( EQEffectParams.MFGain != 0.0f );

	// Precalculate values
    MF_k = appTan( PI * EQEffectParams.MFBandwidthFrequency / XAUDIOSAMPLERATE_NATIVE );
    MF_v0 = appPow( 10.0f, EQEffectParams.MFGain / 20.0f );
    MF_h02 = ( MF_v0 - 1.0f ) / 2.0f;
    MF_d = -appCos( TWO_PI * EQEffectParams.MFCutoffFrequency / XAUDIOSAMPLERATE_NATIVE );

    if( EQEffectParams.MFGain > 0.0f )
	{
		// Boost
        MF_a = ( MF_k - 1.0f ) / ( MF_k + 1.0f );
	}
    else
	{
		// Cut
        MF_a = ( MF_k - MF_v0 ) / ( MF_k + MF_v0 );
	}

    // Precalculate a part of the equation
    MF_d *= ( 1.0f - MF_a );

	// If one of the low frequency parameters was updated, recalculate it's values
	ComputeLF = ( EQEffectParams.LFGain != 0.0f );

    // Precalculate values
    LF_k = appTan( PI * EQEffectParams.LFFrequency / XAUDIOSAMPLERATE_NATIVE );
    LF_v0  = appPow( 10.0f, EQEffectParams.LFGain / 20.0f );
    LF_h02 = ( LF_v0 - 1.0f ) / 2.0f;

    if( EQEffectParams.LFGain > 0.0f )
	{
		// Boost
        LF_a = ( LF_k - 1.0f ) / ( LF_k + 1.0f );
	}
    else
	{
		// Cut
        LF_a = ( LF_k - LF_v0 ) / ( LF_k + LF_v0 );
	}
}

/*------------------------------------------------------------------------------------
	FXeLowPassFilterEffect.
------------------------------------------------------------------------------------*/

/**
 * Increments the ref count.
 */
STDMETHODIMP_( ULONG ) FXeLowPassFilterEffect::AddRef( void )
{
	return( ++RefCount );
}

/**
 * Decrements the ref count and deletes if necessary.
 */
STDMETHODIMP_( ULONG ) FXeLowPassFilterEffect::Release( void )
{
	if( 0 == --RefCount )
	{
		delete this;
		return( 0 );
	}

	return( RefCount );
}

/**
* Returns info used by XAudio - Called by XAudio during effect creation.
*
* @param	Info				The XAUDIOFXINFO structure containing effect configuration info.
*/
STDMETHODIMP FXeLowPassFilterEffect::GetInfo( XAUDIOFXINFO* Info )
{
	check( Info );
	Info->DataFlow = XAUDIODATAFLOW_INPLACE;
	Info->TrailFrames = 0;
	return( S_OK );
}

/**
 * Allows retrieval of effect parameters .
 *
 * @param	ParamId					The client-defined parameter ID.
 * @param	ParamType				The parameter type.
 * @param	Param					A pointer to the parameter variable into which to write the value.
 */
STDMETHODIMP FXeLowPassFilterEffect::GetParam( XAUDIOFXPARAMID ParamId, XAUDIOFXPARAMTYPE ParamType, XAUDIOFXPARAM* Param )
{
	check( Param );
	HRESULT hr = S_OK;

	XAudioLock( XAUDIOLOCKTYPE_LOCK | XAUDIOLOCKTYPE_SPINLOCK );
	switch( ParamId )
	{
	case LPFX_HIGH_FREQUENCY_GAIN_ID:
		Param->Value = HighFrequencyGain;
		break;
	default:
		hr = E_FAIL;
		break;
	}
	XAudioLock( XAUDIOLOCKTYPE_UNLOCK | XAUDIOLOCKTYPE_SPINLOCK );

	return( hr );
}

/**
 * Allows setting of effect parameters.
 *
 * @param	ParamId					The client-defined parameter ID.
 * @param	ParamType				The parameter type.
 * @param	Param					The value to set for the given parameter.
 */
STDMETHODIMP FXeLowPassFilterEffect::SetParam( XAUDIOFXPARAMID ParamId, XAUDIOFXPARAMTYPE ParamType, const XAUDIOFXPARAM* Param )
{
	HRESULT hr = S_OK;

	check( Param );
	XAudioLock( XAUDIOLOCKTYPE_LOCK | XAUDIOLOCKTYPE_SPINLOCK );

	switch( ParamId )
	{
	case LPFX_HIGH_FREQUENCY_GAIN_ID:
		if( HighFrequencyGain != Param->Value )
		{
			HighFrequencyGain = Param->Value;
		}
		break;
	default:
		hr = E_FAIL;
		break;
	}

	XAudioLock( XAUDIOLOCKTYPE_UNLOCK | XAUDIOLOCKTYPE_SPINLOCK );
	return( hr );
}

/**
 * Returns the context this effect was created with.
 *
 * @param	OutContext				A pointer to the context structure passed into the constructor.
 */
STDMETHODIMP FXeLowPassFilterEffect::GetContext( LPVOID* OutContext )
{
	check( OutContext );
	*OutContext = Context;
	return( S_OK );
}

/**
* Constructs a new instance of FXeLowPassFilterEffect.  Called not on register but upon applying the effect to a specific voice.
*
* @param	NewContext				Used to pass initialization info to effect constructor.  Currently unused.
*/
FXeLowPassFilterEffect::FXeLowPassFilterEffect( LPVOID NewContext )
{
	Context = NewContext;
	RefCount = 1;
}

/**
 * Creates the LowPassFilter effect.
 *
 * @param	Init					A pointer to the XAudio effect init data.
 * @param	Allocator				A pointer to the XAudio batch allocator.
 * @param	Effect					A double pointer into which the effect's pointer can be stored.
 */
HRESULT CreateLowPassFilterEffect( const XAUDIOFXINIT* Init, IXAudioBatchAllocator* Allocator, IXAudioEffect** Effect )
{
	check( Init );
	check( Effect );
	check( Allocator );

	HRESULT hr = S_OK;
	FXeLowPassFilterEffect* UserEffect = NULL;

	UserEffect = XAUDIO_BATCHALLOC_NEW( FXeLowPassFilterEffect( Init->pContext ), Allocator );

	*Effect = UserEffect;
	return( hr );
}

/**
 * Returns the size of the LowPassFilter effect.
 */
HRESULT QuerySizeLowPassFilterEffect( LPCXAUDIOFXINIT Init, LPDWORD EffectSize )
{
	check( Init );
	check( EffectSize );

	*EffectSize = sizeof( FXeLowPassFilterEffect );
	return( S_OK );
}

/**
 * Updates the volume of the input buffer and submits it back to the output buffer.
 *
 * @param	InputBuffer				The XAudio input buffer for the effect.
 * @param	OutputBuffer			The XAudio output buffer for the effect.
 */
STDMETHODIMP FXeLowPassFilterEffect::Process( IXAudioFrameBuffer* InputBuffer, IXAudioFrameBuffer* OutputBuffer )
{
	check( InputBuffer );
	check( !OutputBuffer );

	if( HighFrequencyGain == 1.0f )
	{
		return( S_OK );
	}

	const XAUDIOFRAMESIZE SampleCount = XAUDIOFRAMESIZE_NATIVE;

	// Verify the format is "internal".  It should always be internal (float)
	XAUDIOFRAMEBUFDATA FrameBufferData;
	InputBuffer->GetProcessingData( &FrameBufferData );
	check( XAUDIOSAMPLETYPE_NATIVE == FrameBufferData.Format.SampleType );

	FLOAT Cutoff = PI * 7000.0f * HighFrequencyGain / XAUDIOSAMPLERATE_NATIVE;
	FLOAT CosCutoff = appCos( Cutoff );
	FLOAT Coeff = ( 2.0f - CosCutoff ) - appSqrt( ( 2.0f - CosCutoff ) * ( 2.0f - CosCutoff ) - 1.0f );
	FLOAT Divisor = appPow( HighFrequencyGain, 0.6f );

	// Go through each channel
	for( INT i = 0; i < SampleCount; ++i )
	{
		for( INT j = 0; j < FrameBufferData.Format.ChannelCount; ++j )
		{
			FLOAT* Sample = &( FrameBufferData.pSampleBuffer[j * SampleCount + i] );

			LF_k = ( 1.0f - Coeff ) * *Sample + ( Coeff * LF_k ); 
			*Sample = LF_k / Divisor;
		}
	}

	return( S_OK );
}

/*------------------------------------------------------------------------------------
	FXeRadioFilterEffect.
------------------------------------------------------------------------------------*/

/**
 * Increments the ref count.
 */
STDMETHODIMP_( ULONG ) FXeRadioFilterEffect::AddRef( void )
{
	return( ++RefCount );
}

/**
 * Decrements the ref count and deletes if necessary.
 */
STDMETHODIMP_( ULONG ) FXeRadioFilterEffect::Release( void )
{
	if( 0 == --RefCount )
	{
		delete this;
		return( 0 );
	}

	return( RefCount );
}

/**
 * Returns info used by XAudio - Called by XAudio during effect creation.
 *
 * @param	Info				The XAUDIOFXINFO structure containing effect configuration info.
 */
STDMETHODIMP FXeRadioFilterEffect::GetInfo( XAUDIOFXINFO* Info )
{
	check( Info );
	Info->DataFlow = XAUDIODATAFLOW_INPLACE;
	Info->TrailFrames = 0;
	return( S_OK );
}

/**
 * Allows retrieval of effect parameters .
 *
 * @param	ParamId					The client-defined parameter ID.
 * @param	ParamType				The parameter type.
 * @param	Param					A pointer to the parameter variable into which to write the value.
 */
STDMETHODIMP FXeRadioFilterEffect::GetParam( XAUDIOFXPARAMID ParamId, XAUDIOFXPARAMTYPE ParamType, XAUDIOFXPARAM* Param )
{
	check( Param );
	HRESULT hr = S_OK;

	XAudioLock( XAUDIOLOCKTYPE_LOCK | XAUDIOLOCKTYPE_SPINLOCK );
	switch( ParamId )
	{
	case RADIOFX_AGC_TARGET_ID:
		Param->Value = AGCTarget;
		break;
	case RADIOFX_COMP_THRESHOLD_ID:
		Param->Value = CompThreshold;
		break;
	case RADIOFX_COMP_RATIO_ID:
		Param->Value = CompRatio;
		break;
	case RADIOFX_COMP_KNEE_ID:
		Param->Value = CompKnee;
		break;
	case RADIOFX_COMP_MAKEUP_ID:
		Param->Value = CompMakeup;
		break;
	case RADIOFX_COMP_LIMIT_ID:
		Param->Value = CompLimit;
		break;
	case RADIOFX_GAIN_ID:
		Param->Value = Gain;
		break;
	default:
		hr = E_FAIL;
		break;
	}
	XAudioLock( XAUDIOLOCKTYPE_UNLOCK | XAUDIOLOCKTYPE_SPINLOCK );

	return( hr );
}

/**
 * Allows setting of effect parameters.
 *
 * @param	ParamId					The client-defined parameter ID.
 * @param	ParamType				The parameter type.
 * @param	Param					The value to set for the given parameter.
 */
STDMETHODIMP FXeRadioFilterEffect::SetParam( XAUDIOFXPARAMID ParamId, XAUDIOFXPARAMTYPE ParamType, const XAUDIOFXPARAM* Param )
{
	HRESULT hr = S_OK;

	check( Param );
	XAudioLock( XAUDIOLOCKTYPE_LOCK | XAUDIOLOCKTYPE_SPINLOCK );

	switch( ParamId )
	{
	case RADIOFX_AGC_TARGET_ID:
		if( AGCTarget != Param->Value )
		{
			AGCTarget = Param->Value;
			bParamsUpdated = TRUE;
		}
		break;
	case RADIOFX_COMP_THRESHOLD_ID:
		if( CompThreshold != Param->Value )
		{
			CompThreshold = Param->Value;
			bParamsUpdated = TRUE;
		}
		break;
	case RADIOFX_COMP_RATIO_ID:
		if( CompRatio != Param->Value )
		{
			CompRatio = Param->Value;
			bParamsUpdated = TRUE;
		}
		break;
	case RADIOFX_COMP_KNEE_ID:
		if( CompKnee != Param->Value )
		{
			CompKnee = Param->Value;
			bParamsUpdated = TRUE;
		}
		break;
	case RADIOFX_COMP_MAKEUP_ID:
		if( CompMakeup != Param->Value )
		{
			CompMakeup = Param->Value;
			bParamsUpdated = TRUE;
		}
		break;
	case RADIOFX_COMP_LIMIT_ID:
		if( CompLimit != Param->Value )
		{
			CompLimit = Param->Value;
			bParamsUpdated = TRUE;
		}
		break;
	case RADIOFX_GAIN_ID:
		if( Gain != Param->Value )
		{
			Gain = Param->Value;
			bParamsUpdated = TRUE;
		}
		break;
	default:
		hr = E_FAIL;
		break;
	}

	XAudioLock( XAUDIOLOCKTYPE_UNLOCK | XAUDIOLOCKTYPE_SPINLOCK );
	return( hr );
}

/**
 * Returns the context this effect was created with.
 *
 * @param	OutContext				A pointer to the context structure passed into the constructor.
 */
STDMETHODIMP FXeRadioFilterEffect::GetContext( LPVOID* OutContext )
{
	check( OutContext );
	*OutContext = Context;
	return( S_OK );
}

/**
 * Constructs a new instance of FXeRadioFilterEffect.  Called not on register but upon applying the effect to a specific voice.
 *
 * @param	NewContext				Used to pass initialization info to effect constructor.  Currently unused.
 */
FXeRadioFilterEffect::FXeRadioFilterEffect( LPVOID NewContext )
{
	Context = NewContext;
	RefCount = 1;

	AGCTarget = 1.41f;
	CompThreshold = 0.46f;
	CompRatio = 0.41f;
	CompKnee = 0.58f;
	CompMakeup = 0.86f;
	CompLimit = 1.64f;
	BPFFreq = 0.09f;
	BPFQ = 4.0f;
	BPFGain = 0.8f;
	Gain = 1.0f;

	bParamsUpdated = TRUE;
	InputAmplitudeIndex = 0;
	InputAmplitudeAccumulator = 0.0f;
	OutputAmplitudeIndex = 0;
	OutputAmplitudeAccumulator = 0.0f;
}

/**
 * Creates the RadioFilter effect.
 *
 * @param	Init					A pointer to the XAudio effect init data.
 * @param	Allocator				A pointer to the XAudio batch allocator.
 * @param	Effect					A double pointer into which the effect's pointer can be stored.
 */
HRESULT CreateRadioFilterEffect( const XAUDIOFXINIT* Init, IXAudioBatchAllocator* Allocator, IXAudioEffect** Effect )
{
	check( Init );
	check( Effect );
	check( Allocator );

	HRESULT hr = S_OK;
	FXeRadioFilterEffect* UserEffect = NULL;

	UserEffect = XAUDIO_BATCHALLOC_NEW( FXeRadioFilterEffect( Init->pContext ), Allocator );

	*Effect = UserEffect;
	return( hr );
}

/**
 * Returns the size of the RadioFilter effect.
 */
HRESULT QuerySizeRadioFilterEffect( LPCXAUDIOFXINIT Init, LPDWORD EffectSize )
{
	check( Init );
	check( EffectSize );

	*EffectSize = sizeof( FXeRadioFilterEffect );
	return( S_OK );
}

/** 
 * Update the compressor coefficients
 */
void FXeRadioFilterEffect::CalcCompressorCoefficients( float t, float r, float k )
{
	InputAmplitudeAccumulator = 0.0f;
	XMemSet( InputAmplitudeWindow, 0, sizeof( InputAmplitudeWindow ) );
	OutputAmplitudeAccumulator = 0.0f;
	XMemSet( OutputAmplitudeWindow, 0, sizeof( OutputAmplitudeWindow ) );

	FLOAT denom = -8 * k * t - 3 * t * t * t + t * t * t * t - 8 * t * k * k -3 * k * t * t * t + 2 * k * k * t * t + 6 * k * k + 2 * t * t + 11 * k * t * t;
	CoeffA = -0.5f * ( ( -3 * k + 4 * t - 3 + 3 * k * r - 4 * t * r + 3 * r ) / denom ); 
	CoeffB = ( 2 * k * k * r - 2 * k * k + 2 * k * r - 2 * k - 3 * t * t * r + 3 * t * t + 2 * r - 2 ) / denom;
	CoeffC = -0.5f * ( t * ( -8 * k * k + 8 * k * k * r - 9 * r * k * t + 9 * k * t - 8 * k + 8 * k * r - 9 * t * r + 9 * t - 8 + 8 * r ) / denom );
	CoeffD = ( 9 * k * t * t - 8 * t * k * k + 2 * t * t * r + t * t * t * t * r - 3 * t * t * t * r + 2 * t * t * k * k * r + 2 * k * t * t * r - 3 * t * t * t * k * r - 8 * k * t + 6 * k * k ) / denom;
}

/** 
 * Update the bandpass coefficients
 */
void FXeRadioFilterEffect::CalcBandPassCoefficients( float f, float q, float g )
{
	InLastSample = 0.0f;
	InLastLastSample = 0.0f;
	OutLastSample = 0.0f;
	OutLastLastSample = 0.0f;

	FLOAT w0 = PI * f;
	FLOAT sin0 = appSin( w0 );
	FLOAT cos0 = appCos( w0 );

	FLOAT alpha = sin0 / ( 2 * q );
	FLOAT a0 = alpha + 1.0f;
	FLOAT a1 = -2.0f * cos0;
	FLOAT a2 = 1.0f - alpha;
	FLOAT b0 = sin0 / 2.0f;
	FLOAT b1 = 0.0f;
	FLOAT b2 = -sin0 / 2.0f;

	CoeffAB0 = b0 / a0;
	CoeffA1 = a1 / a0;
	CoeffA2 = a2 / a0;
	CoeffB1 = b1 / a0;
	CoeffB2 = b2 / a0;
}

/** 
 * Setup the compressor transfer function
 */
void FXeRadioFilterEffect::Waveshaper( XAUDIOSAMPLE* pIn, XAUDIOSAMPLE* pOut, UBOOL GainReduction )
{
	for( INT i = 0; i < XAUDIOFRAMESIZE_NATIVE; ++i )
	{
		XAUDIOSAMPLE Sample = *pIn++;

		XAUDIOSAMPLE AbsSample = fabsf( Sample );
		XAUDIOSAMPLE OutSample = AbsSample;

		if( OutSample > CompThreshold )
		{
			FLOAT SampleSquared = OutSample * OutSample;
			FLOAT SampleCubed = OutSample * SampleSquared;
			FLOAT SampleQuad = SampleSquared * SampleSquared;
			OutSample = CoeffA * SampleQuad + CoeffB * SampleCubed + CoeffC * SampleSquared + CoeffD * OutSample;
		}
		// clamp output
		OutSample = Min( CompLimit, OutSample );

		OutSample *= ( 1.0f + CompMakeup );

		// convert to gain reduction form
		if( GainReduction )
		{
			if( AbsSample != 0.0f )
			{
				OutSample = OutSample / AbsSample;
			}
		}

		*pOut++ = OutSample;
	}
}

/**
 * Updates the volume of the input buffer and submits it back to the output buffer.
 *
 * @param	InputBuffer				The XAudio input buffer for the effect.
 * @param	OutputBuffer			The XAudio output buffer for the effect.
 */
STDMETHODIMP FXeRadioFilterEffect::Process( IXAudioFrameBuffer* InputBuffer, IXAudioFrameBuffer* OutputBuffer )
{
	check( InputBuffer );
	check( !OutputBuffer );

	if( bParamsUpdated )
	{
		CalcCompressorCoefficients( CompThreshold, CompRatio, CompKnee );

		CalcBandPassCoefficients( BPFFreq, BPFQ, BPFGain );

		Waveshaper( DefaultTransferTable, TransferFunction, FALSE );

		bParamsUpdated = FALSE;
	}

	// Get the input sample data
	XAUDIOFRAMEBUFDATA FrameBufferData;
	InputBuffer->GetProcessingData( &FrameBufferData );

	// Verify the format is "internal".  It should always be internal (float)
	check( XAUDIOSAMPLETYPE_NATIVE == FrameBufferData.Format.SampleType );

	XAUDIOSAMPLE AccumulatedAmplitude[XAUDIOFRAMESIZE_NATIVE];

	// Get the accumulated amplitude
	for( INT i = 0; i < XAUDIOFRAMESIZE_NATIVE; ++i )
	{
		FLOAT CurrentAmplitude = fabsf( FrameBufferData.pSampleBuffer[i] );

		// add the current value and at the same time subtract the last value in the window--this keeps the calculation
		// confined to the length of the window.
		InputAmplitudeAccumulator += CurrentAmplitude;
		InputAmplitudeAccumulator -= InputAmplitudeWindow[InputAmplitudeIndex & 0xff];

		InputAmplitudeWindow[InputAmplitudeIndex & 0xff] = CurrentAmplitude;
		InputAmplitudeIndex++;

		AccumulatedAmplitude[i] = InputAmplitudeAccumulator / 256.0f;
	}

	XAUDIOSAMPLE SideChain[XAUDIOFRAMESIZE_NATIVE];

	// Apply the AGC filter
	for( INT i = 0; i < XAUDIOFRAMESIZE_NATIVE; ++i )
	{
		FLOAT AGCGain = AGCTarget / ( AccumulatedAmplitude[i] + 0.0001f );
		SideChain[i] = AGCGain * FrameBufferData.pSampleBuffer[i];
	}

	XAUDIOSAMPLE GainReduction[XAUDIOFRAMESIZE_NATIVE];

	// Apply the compression filter
	Waveshaper( SideChain, GainReduction, TRUE );

	// Apply the gain reduction
	for( INT i = 0; i < XAUDIOFRAMESIZE_NATIVE; ++i )
	{
		FLOAT OutSample = FrameBufferData.pSampleBuffer[i] * GainReduction[i];
		FLOAT AbsSample = fabsf( OutSample );

		OutputAmplitudeAccumulator += AbsSample;
		OutputAmplitudeAccumulator -= OutputAmplitudeWindow[OutputAmplitudeIndex & 0xff];

		OutputAmplitudeWindow[OutputAmplitudeIndex & 0xff] = AbsSample;
		OutputAmplitudeIndex++;

		FLOAT OutAmp = OutputAmplitudeAccumulator / 256.0f;
		if( OutAmp > 0.0f )
		{
			FLOAT Loss = ( AccumulatedAmplitude[i] - OutAmp ) / OutAmp;
			FrameBufferData.pSampleBuffer[i] = OutSample * ( 1.0f + Loss );
		}
	}

	// Apply the bandpass filter
	for( INT i = 0; i < XAUDIOFRAMESIZE_NATIVE; ++i )
	{
		FLOAT InSample = FrameBufferData.pSampleBuffer[i];

		FLOAT WorkSample = ( CoeffAB0 * InSample ) + ( CoeffB1 * InLastSample ) + ( CoeffB2 * InLastLastSample ) - ( CoeffA1 * OutLastSample ) - ( CoeffA2 * OutLastLastSample );

		FLOAT OutSample = ( ( 1.0f + BPFGain ) * WorkSample + ( 1.0f - BPFGain ) * InSample ) / 2.0f;
		FrameBufferData.pSampleBuffer[i] = OutSample;

		InLastLastSample = InLastSample;
		InLastSample = InSample;

		OutLastLastSample = OutLastSample;
		OutLastSample = OutSample;
	}

	return( S_OK );
}

// end
