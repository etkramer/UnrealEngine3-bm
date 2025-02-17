/**
 * Boomshot Bomblet Weapon Projectile (emitted by GearProj_Boomshot, like a cluster bomb).
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_BoomshotBomblet extends GearProj_BoomshotBombletBase
	config(Weapon);

defaultproperties
{
	Begin Object Name=StaticMeshComponent0
    	StaticMesh=StaticMesh'Locust_Boomshot.locust_boomshot_ammo2'
    End Object

	// explosion
	Begin Object Name=ExploTemplate0
		ParticleEmitterTemplate=ParticleSystem'Locust_Boomshot.Effects.P_Boomshot_Explo_littleones'
		ExplosionSound=SoundCue'Weapon_Boomer.Impacts.BoomerExplosionCue'

		bAllowPerMaterialFX=TRUE
		bParticleSystemIsBeingOverriddenDontUsePhysMatVersion=TRUE
	End Object
}


