/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AnimNotify_Rumble extends AnimNotify
	native(Anim);


/** A predefined WaveForm **/
var() class<WaveFormBase> PredefinedWaveForm<AllowAbstract>;


/** The waveform to play **/
var() editinline ForceFeedbackWaveform WaveForm;

cpptext
{
	// AnimNotify interface.
	virtual void Notify( class USkeletalMeshComponent* SkelComponent );
}

defaultproperties
{

}
