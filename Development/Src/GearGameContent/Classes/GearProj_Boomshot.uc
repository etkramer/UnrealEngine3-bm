/**
 * Boomer Weapon Projectile
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_Boomshot extends GearProj_BoomshotBase
	config(Weapon);

defaultproperties
{
	BombletClass=class'GearProj_BoomshotBomblet'

	TrailTemplate=ParticleSystem'Locust_Boomshot.Effects.P_Boomshot_smoke_trail'

	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'Locust_Boomshot.locust_boomshot_ammo2'
	End Object

	InFlightSoundTemplate=SoundCue'Weapon_Boomer.Firing.BoomerInAirLoopCue'
	HitPawnExplosionTemplate=ParticleSystem'Locust_Boomshot.Effects.P_Boomshot_Explo_Main_Gib'
	HitPawnExplosionNoGoreTemplate=ParticleSystem'Locust_Boomshot.Effects.P_Boomshot_Explo_Main_Gib_NOGORE'

	// explosion
	Begin Object Name=ExploTemplate0
		ParticleEmitterTemplate=ParticleSystem'Locust_Boomshot.Effects.P_Boomshot_Explo_Main'
		ExplosionSound=SoundCue'Weapon_Boomer.Impacts.BoomerExplosionCue'

		ExploAnimShake=(AnimBlendInTime=0.1f,bUseDirectionalAnimVariants=TRUE,Anim=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Front',Anim_Left=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Left',Anim_Right=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Right',Anim_Rear=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Back')
	End Object
}


