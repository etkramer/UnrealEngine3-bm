/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SpawnedGearEmitter extends GearEmitter
	native
	notplaceable;

final function HideBecauseFinished(ParticleSystemComponent FinishedComponent)
{
	HideSelf();
}

defaultproperties
{
	Begin Object Name=ParticleSystemComponent0
		SecondsBeforeInactive=0
	End Object

	Components.Remove(ArrowComponent0)
	Components.Remove(Sprite)

	LifeSpan=30.0f // safety net for any spawned emitters (caller can override if they know their effect will be last infi time or longer than 30
}
