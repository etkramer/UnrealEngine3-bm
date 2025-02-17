/*=============================================================================
	ALAudioEffects.cpp: Unreal OpenAL Audio interface object.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.

=============================================================================*/

/*------------------------------------------------------------------------------------
	Audio includes.
------------------------------------------------------------------------------------*/

#include "ALAudioPrivate.h"

#if SUPPORTS_PRAGMA_PACK
#pragma pack( push, 8 )
#endif

#include <efx.h>

#if SUPPORTS_PRAGMA_PACK
#pragma pack( pop )
#endif

/*------------------------------------------------------------------------------------
	FALAudioEffectsManager.
------------------------------------------------------------------------------------*/

FALAudioEffectsManager::FALAudioEffectsManager( UAudioDevice* InDevice, TArray<FSoundSource*> Sources )
	: FAudioEffectsManager( InDevice )
{
	UBOOL ErrorsFound = FALSE;

	MaxEffectSlots = 0;
	MaxSends = 0;
	ReverbEffect = AL_EFFECT_NULL;

	AudioDevice = ( UALAudioDevice* )InDevice;

	SUPPORTS_EFX = FALSE;
	AudioDevice->FindProcs( TRUE );		
	if( SUPPORTS_EFX )
	{
		for( MaxEffectSlots = 0; MaxEffectSlots < MAX_EFFECT_SLOTS; MaxEffectSlots++ )
		{
			alGenAuxiliaryEffectSlots( 1, EffectSlots + MaxEffectSlots );
			if( AudioDevice->alError( TEXT( "Init (creating aux effect slots)" ), 0 ) )
			{
				break;
			}
		}

		alcGetIntegerv( AudioDevice->GetDevice(), ALC_MAX_AUXILIARY_SENDS, sizeof( MaxSends ), &MaxSends );

		debugf( NAME_Init, TEXT( "Found EFX extension with %d effect slots and %d potential sends" ), MaxEffectSlots, MaxSends );

		// Check for supported EFX features that are useful
		alGenEffects( 1, &ReverbEffect );
		if( alIsEffect( ReverbEffect ) )
		{
			alEffecti( ReverbEffect, AL_EFFECT_TYPE, AL_EFFECT_REVERB );
		}

		debugf( NAME_Init, TEXT( "...'reverb' %ssupported" ), AudioDevice->alError( TEXT( "" ), 0 ) ? TEXT( "un" ) : TEXT( "" ) );

		if( MaxEffectSlots > 1 )
		{
			alGenEffects( 1, &EQEffect );
			if( alIsEffect( EQEffect ) )
			{
				alEffecti( EQEffect, AL_EFFECT_TYPE, AL_EFFECT_EQUALIZER );
			}

			debugf( NAME_Init, TEXT( "...'equalizer' %ssupported" ), AudioDevice->alError( TEXT( "" ), 0 ) ? TEXT( "un" ) : TEXT( "" ) );
		}

		// Generate a low pass filter effect for each source
		for( INT SourceId = 0; SourceId < Sources.Num(); SourceId++ )
		{
			FALSoundSource* Source = static_cast< FALSoundSource* >( Sources( SourceId ) );

			alGenFilters( 1, &Source->LowPassFilterId );
			if( alIsFilter( Source->LowPassFilterId ) )
			{
				alFilteri( Source->LowPassFilterId, AL_FILTER_TYPE, AL_FILTER_LOWPASS );

				alFilterf( Source->LowPassFilterId, AL_LOWPASS_GAIN, 1.0f );
				alFilterf( Source->LowPassFilterId, AL_LOWPASS_GAINHF, 1.0f );
			}
			else
			{
				ErrorsFound = TRUE;
				break;
			}
		}

		debugf( NAME_Init, TEXT( "...'low pass filter' %ssupported" ), AudioDevice->alError( TEXT( "" ), 0 ) ? TEXT( "un" ) : TEXT( "" ) );

		alAuxiliaryEffectSloti( EffectSlots[0], AL_EFFECTSLOT_EFFECT, ReverbEffect );
		alAuxiliaryEffectSloti( EffectSlots[0], AL_EFFECTSLOT_AUXILIARY_SEND_AUTO, AL_TRUE );

		if( MaxEffectSlots > 1 )
		{
			alAuxiliaryEffectSloti( EffectSlots[1], AL_EFFECTSLOT_EFFECT, EQEffect );
			alAuxiliaryEffectSloti( EffectSlots[1], AL_EFFECTSLOT_AUXILIARY_SEND_AUTO, AL_TRUE );
		}

		ErrorsFound |= AudioDevice->alError( TEXT( "Generating effect slots" ) );
	}

	if( ErrorsFound || MaxEffectSlots < 1 || MaxSends < 1 )
	{
		MaxEffectSlots = 0;
		MaxSends = 0;
		debugf( NAME_Init, TEXT( "...failed to initialise EFX (reverb and low pass filter)" ) );
	}
	else
	{
		bEffectsInitialised = TRUE;
	}
}

FALAudioEffectsManager::~FALAudioEffectsManager( void )
{
	if( bEffectsInitialised && MaxEffectSlots > 0 )
	{
		for( INT i = 0; i < MaxEffectSlots; i++ )
		{
			alAuxiliaryEffectSloti( EffectSlots[i], AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL );
			alAuxiliaryEffectSloti( EffectSlots[i], AL_EFFECTSLOT_AUXILIARY_SEND_AUTO, AL_FALSE );
		}

		if( alIsEffect( ReverbEffect ) )
		{
			alDeleteEffects( 1, &ReverbEffect );
		}

		if( MaxEffectSlots > 1 )
		{
			if( alIsEffect( EQEffect ) )
			{
				alDeleteEffects( 1, &EQEffect );
			}
		}

		alDeleteAuxiliaryEffectSlots( MaxEffectSlots, EffectSlots );
	}
}

void FALAudioEffectsManager::SetReverbEffectParameters( const FAudioReverbEffect& ReverbEffectParameters )
{
	if( bEffectsInitialised && MaxEffectSlots > 0 )
	{
		alEffectf( ReverbEffect, AL_REVERB_DENSITY, ReverbEffectParameters.Density );
		alEffectf( ReverbEffect, AL_REVERB_DIFFUSION, ReverbEffectParameters.Diffusion );
		alEffectf( ReverbEffect, AL_REVERB_GAIN, ReverbEffectParameters.Gain );
		alEffectf( ReverbEffect, AL_REVERB_GAINHF, ReverbEffectParameters.GainHF );
		alEffectf( ReverbEffect, AL_REVERB_DECAY_TIME, ReverbEffectParameters.DecayTime );
		alEffectf( ReverbEffect, AL_REVERB_DECAY_HFRATIO, ReverbEffectParameters.DecayHFRatio );
		alEffectf( ReverbEffect, AL_REVERB_REFLECTIONS_GAIN, ReverbEffectParameters.ReflectionsGain );
		alEffectf( ReverbEffect, AL_REVERB_REFLECTIONS_DELAY, ReverbEffectParameters.ReflectionsDelay );
		alEffectf( ReverbEffect, AL_REVERB_LATE_REVERB_GAIN, ReverbEffectParameters.LateGain );
		alEffectf( ReverbEffect, AL_REVERB_LATE_REVERB_DELAY, ReverbEffectParameters.LateDelay );
		alEffectf( ReverbEffect, AL_REVERB_AIR_ABSORPTION_GAINHF, ReverbEffectParameters.AirAbsorptionGainHF );
		alEffectf( ReverbEffect, AL_REVERB_ROOM_ROLLOFF_FACTOR, ReverbEffectParameters.RoomRolloffFactor );
		alEffecti( ReverbEffect, AL_REVERB_DECAY_HFLIMIT, ReverbEffectParameters.DecayHFLimit );

		alAuxiliaryEffectSlotf( EffectSlots[0], AL_EFFECTSLOT_GAIN, ReverbEffectParameters.Volume );
		alAuxiliaryEffectSloti( EffectSlots[0], AL_EFFECTSLOT_EFFECT, ReverbEffect );

		AudioDevice->alError( TEXT( "FALAudioEffectsManager::SetReverbEffectParameters" ) );
	}
}

void FALAudioEffectsManager::SetEQEffectParameters( const FAudioEQEffect& EQEffectParameters )
{
	if( bEffectsInitialised && MaxEffectSlots > 1 )
	{
		alEffectf( EQEffect, AL_EQUALIZER_MID1_GAIN, EQEffectParameters.LFGain );
		alEffectf( EQEffect, AL_EQUALIZER_MID1_CENTER, EQEffectParameters.LFFrequency );
		alEffectf( EQEffect, AL_EQUALIZER_MID1_WIDTH, 1.0f );

		alEffectf( EQEffect, AL_EQUALIZER_MID2_GAIN, EQEffectParameters.MFGain );
		alEffectf( EQEffect, AL_EQUALIZER_MID2_CENTER, EQEffectParameters.MFCutoffFrequency );
		alEffectf( EQEffect, AL_EQUALIZER_MID2_WIDTH, EQEffectParameters.MFBandwidthFrequency / EQEffectParameters.MFCutoffFrequency );

		alEffectf( EQEffect, AL_EQUALIZER_HIGH_GAIN, EQEffectParameters.HFGain );
		alEffectf( EQEffect, AL_EQUALIZER_HIGH_CUTOFF, EQEffectParameters.HFFrequency );

		alAuxiliaryEffectSlotf( EffectSlots[1], AL_EFFECTSLOT_GAIN, 1.0f );
		alAuxiliaryEffectSloti( EffectSlots[1], AL_EFFECTSLOT_EFFECT, EQEffect );

		AudioDevice->alError( TEXT( "FALAudioEffectsManager::SetEQEffectParameters" ) );
	}
}

/** 
 * Platform dependent call to update the sound output with new parameters
 */
void* FALAudioEffectsManager::UpdateEffect( FSoundSource* Source )
{
	if( bEffectsInitialised && MaxEffectSlots > 0 )
	{
		FALSoundSource* ALSource = static_cast< FALSoundSource* >( Source );

		// The low pass filter is always applied unless debugging
		ALuint DryFilter = ALSource->LowPassFilterId;
		ALuint ReverbFilter = ALSource->LowPassFilterId;
		ALuint EQFilter = AL_FILTER_NULL;

		// Set up the low pass filter gain
		alFilterf( ALSource->LowPassFilterId, AL_LOWPASS_GAIN, 1.0f );
		alFilterf( ALSource->LowPassFilterId, AL_LOWPASS_GAINHF, ALSource->HighFrequencyGain );

		// Set up the reverb effect
		ALuint ReverbEffectSlot = AL_EFFECTSLOT_NULL;
		if( Source->IsReverbApplied() )
		{
			ReverbEffectSlot = EffectSlots[0];
		}

		// Set up the reverb effect
		ALuint EQEffectSlot = AL_EFFECTSLOT_NULL;
		if( ( Source->IsEQFilterApplied() || AudioDevice->IsTestingEQFilter() ) && MaxEffectSlots > 1 )
		{
			EQEffectSlot = EffectSlots[1];

			// Kill the direct path so as all output is EQ'd
			alFilterf( ALSource->LowPassFilterId, AL_LOWPASS_GAIN, 0.0f );
			alFilterf( ALSource->LowPassFilterId, AL_LOWPASS_GAINHF, 0.0f );
			DryFilter = ALSource->LowPassFilterId;
		}

		switch( DebugState )
		{
		case DEBUGSTATE_IsolateDryAudio:
			DryFilter = AL_FILTER_NULL;
			ReverbEffectSlot = AL_EFFECTSLOT_NULL;
			EQEffectSlot = AL_EFFECTSLOT_NULL;
			break;
			
		case DEBUGSTATE_IsolateReverb:
			// Kill the direct path
			alFilterf( ALSource->LowPassFilterId, AL_LOWPASS_GAIN, 0.0f );
			alFilterf( ALSource->LowPassFilterId, AL_LOWPASS_GAINHF, 0.0f );
			DryFilter = ALSource->LowPassFilterId;

			ReverbFilter = AL_FILTER_NULL;
			EQEffectSlot = AL_EFFECTSLOT_NULL;
			break;

		case DEBUGSTATE_IsolateEQ:
			// Kill the direct path
			alFilterf( ALSource->LowPassFilterId, AL_LOWPASS_GAIN, 0.0f );
			alFilterf( ALSource->LowPassFilterId, AL_LOWPASS_GAINHF, 0.0f );
			DryFilter = ALSource->LowPassFilterId;

			ReverbEffectSlot = AL_EFFECTSLOT_NULL;
			EQFilter = AL_FILTER_NULL;
			break;
		}

		// Apply filter to dry signal
		alSourcei( ALSource->SourceId, AL_DIRECT_FILTER, DryFilter );

		// Apply effect and the filter to the wet signal
		alSource3i( ALSource->SourceId, AL_AUXILIARY_SEND_FILTER, ReverbEffectSlot, 0, ReverbFilter );

		if( MaxEffectSlots > 1 )
		{
			alSource3i( ALSource->GetSourceId(), AL_AUXILIARY_SEND_FILTER, EQEffectSlot, 1, EQFilter );
		}

		AudioDevice->alError( TEXT( "FALAudioEffectsManager::UpdateEffect" ) );
	}

	return( NULL );
}

/** 
 * Platform dependent call to destroy any effect related data
 */
void FALAudioEffectsManager::DestroyEffect( FSoundSource* Source )
{
	if( bEffectsInitialised && MaxEffectSlots > 0 )
	{
		FALSoundSource* ALSource = static_cast< FALSoundSource* >( Source );

		if( alIsFilter( ALSource->LowPassFilterId ) )
		{
			alDeleteFilters( 1, &ALSource->LowPassFilterId );
		}
	}
}

// end
