/*=============================================================================
	XeAudioDevice.cpp: Unreal XAudio2 Audio interface object.
	Copyright 1998-2009 Epic Games, Inc. All Rights Reserved.

	Unreal is RHS with Y and Z swapped (or technically LHS with flipped axis)

=============================================================================*/

/*------------------------------------------------------------------------------------
	Audio includes.
------------------------------------------------------------------------------------*/

#include <xapobase.h>
#include <xapofx.h>
#include <xaudio2fx.h>

#include "Engine.h"
#include "EngineSoundClasses.h"
#include "EngineAudioDeviceClasses.h"
#include "UnAudioEffect.h"
#include "XAudio2Device.h"
#include "XAudio2Effects.h"

/*------------------------------------------------------------------------------------
	FXeAudioEffectsManager.
------------------------------------------------------------------------------------*/

/** 
 * Create voices that pipe the dry or EQ'd sound to the master output
 */
UBOOL FXAudio2EffectsManager::CreateEQPremasterVoices( UXAudio2Device* XAudio2Device )
{
	DWORD SampleRate = XAudio2Device->DeviceDetails.OutputFormat.Format.nSamplesPerSec;

	// Create the EQ effect
	if( !AudioDevice->ValidateAPICall( TEXT( "CreateFX (EQ)" ), 
		CreateFX( __uuidof( FXEQ ), &EQEffect ) ) )
	{
		return( FALSE );
	}

	XAUDIO2_EFFECT_DESCRIPTOR EQEffects[] =
	{
		{ EQEffect, TRUE, XAUDIO2_SPEAKER_COUNT }
	};

	XAUDIO2_EFFECT_CHAIN EQEffectChain =
	{
		1, EQEffects
	};

	if( !AudioDevice->ValidateAPICall( TEXT( "CreateSubmixVoice (EQPremaster)" ), 
		XAudio2Device->XAudio2->CreateSubmixVoice( &EQPremasterVoice, XAUDIO2_SPEAKER_COUNT, SampleRate, 0, STAGE_EQPREMASTER, NULL, &EQEffectChain ) ) )
	{
		return( FALSE );
	}

	if( !AudioDevice->ValidateAPICall( TEXT( "CreateSubmixVoice (DryPremaster)" ), 
		XAudio2Device->XAudio2->CreateSubmixVoice( &DryPremasterVoice, XAUDIO2_SPEAKER_COUNT, SampleRate, 0, STAGE_EQPREMASTER, NULL, NULL ) ) )
	{
		return( FALSE );
	}

	// Set the output matrix catering for a potential downmix
	const FLOAT OutputMatrixPassThrough[XAUDIO2_SPEAKER_COUNT * XAUDIO2_SPEAKER_COUNT] = 
	{ 
		1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 
		0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f 
	};

	const FLOAT OutputMatrixDownMix[XAUDIO2_SPEAKER_COUNT * 2] = 
	{ 
		1.0f, 0.0f, 0.7f, 0.0f, 0.87f, 0.49f,  
		0.0f, 1.0f, 0.7f, 0.0f, 0.49f, 0.87f 
	};

	DWORD NumChannels = XAudio2Device->DeviceDetails.OutputFormat.Format.nChannels;
	const FLOAT* OutputMatrix = ( NumChannels == 6 ) ? OutputMatrixPassThrough : OutputMatrixDownMix;

	if( !AudioDevice->ValidateAPICall( TEXT( "SetOutputMatrix (EQPremaster)" ), 
		EQPremasterVoice->SetOutputMatrix( NULL, XAUDIO2_SPEAKER_COUNT, NumChannels, OutputMatrix ) ) )
	{
		return( FALSE );
	}

	if( !AudioDevice->ValidateAPICall( TEXT( "SetOutputMatrix (DryPremaster)" ), 
		DryPremasterVoice->SetOutputMatrix( NULL, XAUDIO2_SPEAKER_COUNT, NumChannels, OutputMatrix ) ) )
	{
		return( FALSE );
	}

	return( TRUE );
}

/** 
 * Create a voice that pipes the reverb sounds to the premastering voices
 */
UBOOL FXAudio2EffectsManager::CreateReverbVoice( UXAudio2Device* XAudio2Device )
{
	UINT32 Flags;

	DWORD SampleRate = XAudio2Device->DeviceDetails.OutputFormat.Format.nSamplesPerSec;
#if _DEBUG
	Flags = XAUDIO2FX_DEBUG;
#else
	Flags = 0;
#endif
	// Create the reverb effect
	if( !AudioDevice->ValidateAPICall( TEXT( "CreateReverbEffect" ), 
		XAudio2CreateReverb( &ReverbEffect, Flags ) ) )
	{
		return( FALSE );
	}

	XAUDIO2_EFFECT_DESCRIPTOR ReverbEffects[] =
	{
		{ ReverbEffect, TRUE, 2 }
	};

	XAUDIO2_EFFECT_CHAIN ReverbEffectChain =
	{
		1, ReverbEffects
	};

	IXAudio2Voice* SendList[] = 
	{
		DryPremasterVoice
	};

	const XAUDIO2_VOICE_SENDS ReverbSends = 
	{
		1, ( IXAudio2Voice** )SendList
	};

	if( !AudioDevice->ValidateAPICall( TEXT( "CreateSubmixVoice (Reverb)" ), 
		XAudio2Device->XAudio2->CreateSubmixVoice( &ReverbEffectVoice, 2, SampleRate, 0, STAGE_REVERB, &ReverbSends, &ReverbEffectChain ) ) )
	{
		return( FALSE );
	}

	const FLOAT OutputMatrix[XAUDIO2_SPEAKER_COUNT * 2] = 
	{ 
		1.0f, 0.0f,
		0.0f, 1.0f, 
		0.7f, 0.7f, 
		0.0f, 0.0f, 
		1.0f, 0.0f, 
		0.0f, 1.0f 
	};

	if( !AudioDevice->ValidateAPICall( TEXT( "SetOutputMatrix (Reverb)" ), 
		ReverbEffectVoice->SetOutputMatrix( DryPremasterVoice, 2, XAUDIO2_SPEAKER_COUNT, OutputMatrix ) ) )
	{
		return( FALSE );
	}

	return( TRUE );
}

/**
 * Init all sound effect related code
 */
FXAudio2EffectsManager::FXAudio2EffectsManager( UAudioDevice* InDevice )
	: FAudioEffectsManager( InDevice )
{
	UXAudio2Device* XAudio2Device = ( UXAudio2Device* )AudioDevice;
	
	ReverbEffect = NULL;
	EQEffect = NULL;

	DryPremasterVoice = NULL;
	EQPremasterVoice = NULL;
	ReverbEffectVoice = NULL;

	// Create premaster voices for EQ and dry passes
	CreateEQPremasterVoices( XAudio2Device );

	// Create reverb voice 
	CreateReverbVoice( XAudio2Device );
}

/**
 * Clean up
 */
FXAudio2EffectsManager::~FXAudio2EffectsManager( void )
{
	if( ReverbEffectVoice )
	{
		ReverbEffectVoice->DestroyVoice();
	}

	if( DryPremasterVoice )
	{
		DryPremasterVoice->DestroyVoice();
	}

	if( EQPremasterVoice )
	{
		EQPremasterVoice->DestroyVoice();
	}

	if( ReverbEffect )
	{
		ReverbEffect->Release();
	}

	if( EQEffect )
	{
		EQEffect->Release();
	}
}

/**
 * Applies the generic reverb parameters to the XAudio2 hardware
 */
void FXAudio2EffectsManager::SetReverbEffectParameters( const FAudioReverbEffect& ReverbEffectParameters )
{
	XAUDIO2FX_REVERB_I3DL2_PARAMETERS ReverbParameters;
	XAUDIO2FX_REVERB_PARAMETERS NativeParameters;

	ReverbParameters.WetDryMix = 100.0f;
	ReverbParameters.Room = VolumeToMilliBels( ReverbEffectParameters.Volume * ReverbEffectParameters.Gain );
	ReverbParameters.RoomHF = VolumeToMilliBels( ReverbEffectParameters.GainHF );
	ReverbParameters.RoomRolloffFactor = ReverbEffectParameters.RoomRolloffFactor;
	ReverbParameters.DecayTime = ReverbEffectParameters.DecayTime;
	ReverbParameters.DecayHFRatio = ReverbEffectParameters.DecayHFRatio;
	ReverbParameters.Reflections = VolumeToMilliBels( ReverbEffectParameters.ReflectionsGain );
	ReverbParameters.ReflectionsDelay = ReverbEffectParameters.ReflectionsDelay;
	ReverbParameters.Reverb = VolumeToMilliBels( ReverbEffectParameters.LateGain );
	ReverbParameters.ReverbDelay = ReverbEffectParameters.LateDelay;
	ReverbParameters.Diffusion = ReverbEffectParameters.Diffusion * 100.0f;
	ReverbParameters.Density = ReverbEffectParameters.Density * 100.0f;
	ReverbParameters.HFReference = DEFAULT_HIGH_FREQUENCY;

	ReverbConvertI3DL2ToNative( &ReverbParameters, &NativeParameters );

	AudioDevice->ValidateAPICall( TEXT( "SetEffectParameters (Reverb)" ), 
		ReverbEffectVoice->SetEffectParameters( 0, &NativeParameters, sizeof( NativeParameters ) ) );
}

/**
 * Applies the generic EQ parameters to the XAudio2 hardware
 */
void FXAudio2EffectsManager::SetEQEffectParameters( const FAudioEQEffect& EQEffectParameters )
{
	FXEQ_PARAMETERS NativeParameters;

	NativeParameters.FrequencyCenter0 = Clamp<FLOAT>( EQEffectParameters.LFFrequency, FXEQ_MIN_FREQUENCY_CENTER, FXEQ_MAX_FREQUENCY_CENTER );
	NativeParameters.Gain0 = Clamp<FLOAT>( EQEffectParameters.LFGain, FXEQ_MIN_GAIN, FXEQ_MAX_GAIN );
	NativeParameters.Bandwidth0 = Clamp<FLOAT>( ( EQEffectParameters.MFCutoffFrequency - EQEffectParameters.LFFrequency ) / EQEffectParameters.LFFrequency, FXEQ_MIN_BANDWIDTH, FXEQ_MAX_BANDWIDTH );

	NativeParameters.FrequencyCenter1 = Clamp<FLOAT>( EQEffectParameters.MFCutoffFrequency, FXEQ_MIN_FREQUENCY_CENTER, FXEQ_MAX_FREQUENCY_CENTER );
	NativeParameters.Gain1 = Clamp<FLOAT>( EQEffectParameters.MFGain, FXEQ_MIN_GAIN, FXEQ_MAX_GAIN );
	NativeParameters.Bandwidth1 = Clamp<FLOAT>( EQEffectParameters.MFBandwidthFrequency / EQEffectParameters.MFCutoffFrequency, FXEQ_MIN_BANDWIDTH, FXEQ_MAX_BANDWIDTH );

	NativeParameters.FrequencyCenter2 = Clamp<FLOAT>( EQEffectParameters.HFFrequency, FXEQ_MIN_FREQUENCY_CENTER, FXEQ_MAX_FREQUENCY_CENTER );
	NativeParameters.Gain2 = Clamp<FLOAT>( EQEffectParameters.HFGain, FXEQ_MIN_GAIN, FXEQ_MAX_GAIN );
	NativeParameters.Bandwidth2 = Clamp<FLOAT>( ( EQEffectParameters.HFFrequency - EQEffectParameters.MFCutoffFrequency ) / EQEffectParameters.HFFrequency, FXEQ_MIN_BANDWIDTH, FXEQ_MAX_BANDWIDTH );

	NativeParameters.FrequencyCenter3 = FXEQ_DEFAULT_FREQUENCY_CENTER_3;
	NativeParameters.Gain3 = FXEQ_DEFAULT_GAIN;
	NativeParameters.Bandwidth3 = FXEQ_DEFAULT_BANDWIDTH;

	AudioDevice->ValidateAPICall( TEXT( "SetEffectParameters (EQ)" ), 
		EQPremasterVoice->SetEffectParameters( 0, &NativeParameters, sizeof( NativeParameters ) ) );
}

// end
