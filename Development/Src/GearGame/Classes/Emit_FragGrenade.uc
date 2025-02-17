/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Emit_FragGrenade extends Emit_SmokeGrenade
	config(Weapon)
	notplaceable;


defaultproperties
{


	bDestroyOnSystemFinish=TRUE
	Begin Object Name=ParticleSystemComponent0
	    Template=None
	End Object

	bNetInitialRotation=TRUE
	SmokeSpewStartCue=None
	SmokeSpewFinishCue=None
	SmokeSpewLoopCue=None

	FogVolumeClass=class'GearFogVolume_FragGrenade'

	bNoDelete=FALSE

	bDoCoughChecks=FALSE
	CoughRadius=512
	CoughCheckInterval=(X=1.f,Y=2.f)
}
