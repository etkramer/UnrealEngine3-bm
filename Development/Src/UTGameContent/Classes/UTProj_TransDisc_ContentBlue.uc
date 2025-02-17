/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTProj_TransDisc_ContentBlue extends UTProj_TransDisc;

defaultproperties
{
	ProjFlightTemplate=ParticleSystem'WP_Translocator.Particles.P_WP_Translocator_Trail'

	BounceTemplate=ParticleSystem'WP_Translocator.Particles.P_WP_Translocator_BounceEffect_Blue'

	// Add the mesh
	Begin Object Name=ProjectileMesh
		StaticMesh=StaticMesh'WP_Translocator.Mesh.S_Translocator_Disk'
		Materials(0)=MaterialInterface'WP_Translocator.Materials.M_WP_Translocator_1PBlue_unlit'
		scale=2
	End Object

	Begin Object Class=ParticleSystemComponent Name=ConstantEffect
	Template=ParticleSystem'WP_Translocator.Particles.P_WP_Translocator_Beacon_Blue'
		bAutoActivate=false
		SecondsBeforeInactive=1.0f
	End Object
	LandEffects=ConstantEffect
	Components.Add(ConstantEffect)


	Begin Object Class=UTParticleSystemComponent Name=BrokenPCS
		Template=ParticleSystem'WP_Translocator.Particles.P_WP_Translocator_Broken_Blue'
		HiddenGame=true
		SecondsBeforeInactive=1.0f
	End Object
	DisruptedEffect=BrokenPCS
	Components.Add(BrokenPCS)
	BounceSound=SoundCue'A_Weapon_Translocator.Translocator.A_Weapon_Translocator_Bounce_Cue'
	DisruptedSound=SoundCue'A_Weapon_Translocator.Translocator.A_Weapon_Translocator_Disrupted_Cue'

	
	Begin Object Class=AudioComponent Name=DisruptionSound
		SoundCue=SoundCue'A_Weapon_Translocator.Translocator.A_Weapon_Translocator_DisruptedLoop_Cue'
	End Object
	Components.Add(DisruptionSound);
	DisruptedLoop = DisruptionSound;
	
	ProjectileLightClass=class'UTGame.UTTranslocatorLightBlue'
}