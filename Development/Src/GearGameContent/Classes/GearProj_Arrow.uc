/**
 * GearProj_Arrow
 * Projectile tailored for Torque Bow weapon.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_Arrow extends GearProj_ArrowBase
	config(Weapon);

defaultproperties
{
	ImpactSound=SoundCue'Weapon_TorqueBow.Weapons.Crossbow_ArrowImpactWallCue'
	ImpactFleshSound=SoundCue'Weapon_TorqueBow.Weapons.Crossbow_ArrowImpactFleshCue'
	ImpactStickSound=SoundCue'Weapon_Grenade.Actions.GrenadeStick01Cue'
	RicochetSound=SoundCue'Weapon_TorqueBow.Weapons.Crossbow_ArrowImpactRicochetCue'
	BounceSound=SoundCue'Weapon_Grenade.Impacts.GrenadeBounceCue'

	Begin Object Name=StaticMeshComponent0
    	StaticMesh=StaticMesh'Locust_Torquebow.Locust_Torquebow_Rocket'
    End Object

	Begin Object Name=CollisionCylinder
		CollideActors=TRUE
	End Object

	TrailTemplate=ParticleSystem'Locust_Torquebow.EffectS.P_Torquebow_Arrow_Trail'
	InFlightSoundTemplate=none


	PS_ArrowImpactEffect=ParticleSystem'Locust_Torquebow.Effects.PS_Torquebow_Impact'
	PS_ArrowImpactEffectHuman=ParticleSystem'Locust_Torquebow.Effects.P_Torquebow_Sparks'

	StuckInPawnExplosionEmitterTemplate=ParticleSystem'Locust_Torquebow.Effects.P_Player_Explosion'
	StuckInPawnExplosionNoGoreEmitterTemplate=ParticleSystem'Locust_Torquebow.Effects.P_Player_Explosion_NOGORE'
	StuckInPawnExplosionSound=SoundCue'Weapon_TorqueBow.Weapons.Crossbow_ExplosionBodyCue'

	InAirExplosionEmitterTemplate=ParticleSystem'Locust_Torquebow.EffectS.P_Torquebow_Impact_Explo_Stuck_In'
	InAirExplosionSound=SoundCue'Weapon_TorqueBow.Weapons.Crossbow_ExplosionCue'

	// explosion
	Begin Object Name=ExploTemplate0
		ParticleEmitterTemplate=ParticleSystem'Locust_Torquebow.EffectS.P_Torquebow_Impact_Explo_Stuck_In'
		ExplosionSound=SoundCue'Weapon_TorqueBow.Weapons.Crossbow_ExplosionWallCue'

		ExploAnimShake=(AnimBlendInTime=0.1f,bUseDirectionalAnimVariants=TRUE,Anim=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Front',Anim_Left=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Left',Anim_Right=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Right',Anim_Rear=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Back')
	End Object
}
