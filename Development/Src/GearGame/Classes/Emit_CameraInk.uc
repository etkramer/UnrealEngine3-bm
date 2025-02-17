/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Emit_CameraInk extends Emit_CameraLensEffectBase
	config(Weapon);

var transient protected float	LastTriggerTime;

var protected transient float	CurrentOpacity;

/** Time, in seconds, it takes to fade in. */
var() protected const float		OpacityFadeInTime;
/** Time, in seconds, it takes to fade out. */
var() protected const float		OpacityFadeOutTime;

/** How long to wait after being triggered before starting to fade out. */
var() protected const float		StartFadeDelay;


simulated function PostBeginPlay()
{
	super.PostBeginPlay();
	NotifyRetriggered();
}

function Tick(float DeltaTime)
{
	super.Tick(DeltaTime);

	if ( (WorldInfo.TimeSeconds - LastTriggerTime) < StartFadeDelay )
	{
		// ramping in
		CurrentOpacity += DeltaTime / OpacityFadeInTime;
		CurrentOpacity = FMin(CurrentOpacity, 1.f);
	}
	else
	{
		// ramping out
		CurrentOpacity -= DeltaTime / OpacityFadeOutTime;
		CurrentOpacity = FMax(CurrentOpacity, 0.f);
	}

	ParticleSystemComponent.SetFloatParameter('InkOverlayOpacity', CurrentOpacity);

	if (CurrentOpacity == 0.f)
	{
		// we're done
		Destroy();
	}
}

/** Called when this emitter is re-triggered, for bAllowMultipleInstances=FALSE emitters. */
function NotifyRetriggered()
{
	LastTriggerTime = WorldInfo.TimeSeconds;
}


defaultproperties
{
	PS_CameraEffect=ParticleSystem'Weap_Ink_Grenade.Effects.P_Ink_Grenade_Camera_Effect'
	PS_CameraEffectNonExtremeContent=ParticleSystem'Weap_Ink_Grenade.Effects.P_Ink_Grenade_Camera_Effect'

	LifeSpan=0.f		// live until explicitly destroyed

	StartFadeDelay=0.5f
	OpacityFadeInTime=0.4f
	OpacityFadeOutTime=1.5f
}



