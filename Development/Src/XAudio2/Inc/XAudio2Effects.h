/*=============================================================================
	XAudio2Effects.h: Unreal XAudio2 audio effects interface object.
	Copyright 1998-2009 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _INC_XAUDIO2EFFECTS
#define _INC_XAUDIO2EFFECTS

/** 
 * XAudio2 effects manager
 */
class FXAudio2EffectsManager : public FAudioEffectsManager
{
public:
	FXAudio2EffectsManager( UAudioDevice* InDevice );
	~FXAudio2EffectsManager( void );

	/** 
     * Create voices that pipe the dry or EQ'd sound to the master output
	 */
	UBOOL CreateEQPremasterVoices( class UXAudio2Device* XAudio2Device );

	/** 
     * Create a voice that pipes the reverb sounds to the premastering voices
	 */
	UBOOL CreateReverbVoice( class UXAudio2Device* XAudio2Device );

	/** 
	 * Calls the platform specific code to set the parameters that define reverb
	 */
	virtual void SetReverbEffectParameters( const FAudioReverbEffect& ReverbEffectParameters );

	/** 
	 * Calls the platform specific code to set the parameters that define EQ
	 */
	virtual void SetEQEffectParameters( const FAudioEQEffect& ReverbEffectParameters );

private:
	/** 
	 * Creates the submix voice that handles the reverb effect
	 */
	UBOOL CreateEffectsSubmixVoices( void );

	/** Reverb effect */
	IUnknown*					ReverbEffect;
	/** EQ effect */
	IUnknown*					EQEffect;

	/** For receiving 6 channels of audio that will have no EQ applied */
	IXAudio2SubmixVoice*		DryPremasterVoice;
	/** For receiving 6 channels of audio that can have EQ applied */
	IXAudio2SubmixVoice*		EQPremasterVoice;
	/** For receiving audio that will have reverb applied */
	IXAudio2SubmixVoice*		ReverbEffectVoice;

	friend class UXAudio2Device;
	friend class FXAudio2SoundSource;
};

#endif
// end
