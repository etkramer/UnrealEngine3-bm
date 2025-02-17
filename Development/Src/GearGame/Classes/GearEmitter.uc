/**
 * Gear Emitter subclass
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearEmitter extends Emitter
	native;

/** Sound to play when spawned */
var()	SoundCue		Sound;
/** Particle System to use */
var()	ParticleSystem	ParticleSystem;

simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	if( WorldInfo.NetMode != NM_DedicatedServer )
	{
		if (ParticleSystem != None)
		{
			SetTemplate( ParticleSystem, bDestroyOnSystemFinish );
		}

		if( Sound != None )
		{
			PlaySound( Sound );
		}
	}
}

/** This is mostly used by the Object pool system **/
simulated function HideSelf()
{
	ParticleSystemComponent.DeActivateSystem(); // if we are hiding this we want to deactivate also
	SetBase( None );
	SetHidden( TRUE );
	bStasis = TRUE;
	ClearTimer(nameof(HideSelf));
}


defaultproperties
{
	bDestroyOnSystemFinish=true
	bNetInitialRotation=true
	bNoDelete=false
}
