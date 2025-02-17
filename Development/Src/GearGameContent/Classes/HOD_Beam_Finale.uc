/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class HOD_Beam_Finale extends HOD_Beam;

defaultproperties
{
	bZeroExtentBeam=TRUE
	bHurtBelowBeam=TRUE

	// sound overrides
	CoolDownSound=SoundCue'Weapon_HammerDawn.HODCopter.HODBeamEndCue'
	ImpactSound=SoundCue'Weapon_HammerDawn.HODCopter.HoDStartImpactCue'

	Begin Object Name=FireLoopSound0
		SoundCue=SoundCue'Weapon_HammerDawn.HODCopter.HODBeamLoopCue'
	End Object

	Begin Object Name=RippingEarthLoopSound0
		SoundCue=SoundCue'Weapon_HammerDawn.HODCopter.HODRippingEarthLoopCue'
	End Object

	Begin Object Name=WarmupSound0
		SoundCue=SoundCue'Weapon_HammerDawn.HODCopter.HoDStartWarmupCue'
	End Object



	Begin Object Name=PSC_MainBeam0
		Template=ParticleSystem'COG_HOD.Effects.COG_HOD_BOSS_Main_Beam'
	End Object

	Begin Object Name=PSC_BerserkerImpact0
		Template=ParticleSystem'COG_HOD.Effects.COG_HOD_Main_Beam_BOSS_Impact'
	End Object


	PS_EntryEffect=ParticleSystem'COG_HOD.Effects.COG_HOD_BOSS_Main_IN'
	PS_ExitEffect=ParticleSystem'COG_HOD.Effects.COG_HOD_BOSS_Main_OUT'
}


