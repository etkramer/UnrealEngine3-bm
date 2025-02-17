/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Emit_Hail extends Emit_Rain
	notplaceable;


simulated protected function DropRainDropsOntoPawn()
{
	// don't do pawn-impact fx if raising shield
	if (!BasePC.IsDoingSpecialMove(SM_RaiseShieldOverHead))
	{
		super.DropRainDropsOntoPawn();
	}
}

simulated protected function Emitter GetPawnImpactEmitter( vector SpawnLocation, rotator SpawnRotation )
{
	return GearGRI(WorldInfo.GRI).GOP.GetHailImpactEmitter_Pawn( SpawnLocation, SpawnRotation );
}

defaultproperties
{	
	Begin Object Name=ParticleSystemComponent0
		Template=ParticleSystem'War_HailEffects.Effects.P_FX_Hail_NearPlayer_02'
	End Object	

	RainLoopCue=SoundCue'Ambient_Loops.Water_G2.water_hailroof01Cue'

	ZDistAbovePawn=512

	// @fixme, maybe tie this to GDT_Hail damage
	bDoRaindropsOnPawn=TRUE
}
