/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class FX_GearEmitter extends Emitter
	notplaceable;

/** The Template to use for this emitter */
var ParticleSystem EmitterTemplate;


simulated function PostBeginPlay()
{
	if( EmitterTemplate != None )
	{
		SetTemplate( EmitterTemplate, bDestroyOnSystemFinish );
		ParticleSystemComponent.ActivateSystem();
	}

	super.PostBeginPlay();
}

defaultproperties
{
	bKillDuringLevelTransition=TRUE

	Components.Remove(ArrowComponent0)
	Components.Remove(Sprite)

	LifeSpan=7.0
	bDestroyOnSystemFinish=TRUE
	bNoDelete=FALSE

	bMovable=FALSE
}
