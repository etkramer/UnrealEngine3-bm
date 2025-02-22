/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class AmbientSoundSimpleToggleable extends AmbientSoundSimple;

/** used to update status of toggleable level placed ambient sounds on clients */
var repnotify bool bCurrentlyPlaying;

var() bool bFadeOnToggle;
var() float FadeInDuration;
var() float FadeInVolumeLevel;
var() float FadeOutDuration;
var() float FadeOutVolumeLevel;

struct CheckpointRecord
{
	var bool bCurrentlyPlaying;
};

replication
{
	if (Role == ROLE_Authority)
		bCurrentlyPlaying;
}

simulated event PostBeginPlay()
{
	Super.PostBeginPlay();

	bCurrentlyPlaying = AudioComponent.bAutoPlay;
}

simulated event ReplicatedEvent(name VarName)
{
	if (VarName == 'bCurrentlyPlaying')
	{
		if (bCurrentlyPlaying)
		{
			StartPlaying();
		}
		else
		{
			StopPlaying();
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

simulated function StartPlaying()
{
	if (bFadeOnToggle)
	{
		AudioComponent.FadeIn(FadeInDuration,FadeInVolumeLevel);
	}
	else
	{
		AudioComponent.Play();
	}
	bCurrentlyPlaying = TRUE;
}

simulated function StopPlaying()
{
	if (bFadeOnToggle)
	{
		AudioComponent.FadeOut(FadeOutDuration,FadeOutVolumeLevel);
	}
	else
	{
		AudioComponent.Stop();
	}
	bCurrentlyPlaying = FALSE;
}

/**
 * Handling Toggle event from Kismet.
 */
simulated function OnToggle(SeqAct_Toggle Action)
{
	if (Action.InputLinks[0].bHasImpulse || (Action.InputLinks[2].bHasImpulse && !AudioComponent.bWasPlaying))
	{
		StartPlaying();
	}
	else
	{
		StopPlaying();
	}
	// we now need to replicate this Actor so clients get the updated status
	ForceNetRelevant();
}

defaultproperties
{
	bAutoPlay=FALSE
	bStatic=false
	bNoDelete=true

	FadeInDuration=1.f
	FadeInVolumeLevel=1.f
	FadeOutDuration=1.f
	FadeOutVolumeLevel=0.f
}
