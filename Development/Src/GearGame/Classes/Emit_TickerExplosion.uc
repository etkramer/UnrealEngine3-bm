/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Emit_TickerExplosion extends Emit_SmokeGrenade
	config(Weapon)
	notplaceable;


defaultproperties
{


	bDestroyOnSystemFinish=TRUE
	Begin Object Name=ParticleSystemComponent0
		Template=ParticleSystem'COG_Smoke_Grenade.Effects.P_COG_Smoke_Grenade'
		SecondsBeforeInactive=0.f
	End Object

	bNetInitialRotation=true
	SmokeSpewStartCue=SoundCue'Weapon_Grenade.SmokeGrenade.SmokeGrenadeSmokeStartCue'
	SmokeSpewFinishCue=SoundCue'Weapon_Grenade.SmokeGrenade.SmokeGrenadeSmokeStopCue'
	SmokeSpewLoopCue=SoundCue'Weapon_Grenade.SmokeGrenade.SmokeGrenadeSmokeLoopCue'

	FogVolumeClass=class'GearFogVolume_SmokeGrenade'

	bNoDelete=false

	bDoCoughChecks=TRUE
	CoughRadius=512
	CoughCheckInterval=(X=1.f,Y=2.f)
}
