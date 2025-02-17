/**
 * Boomer Weapon Projectile
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_ReaverCannon extends GearProj_Boomshot
	config(Weapon);

defaultproperties
{
	Speed=15000
	MaxSpeed=15000
	LifeSpan=5.000


	GravityScale=0.f
	bIgnoreInstigatorCollision=TRUE

	MyDamageType=class'GDT_ReaverCannon'

	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'Locust_Reaver.Mesh.S_Reaver_Projectile'
	End Object

	InFlightSoundTemplate=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_RocketInAirLoopCue'
	TrailTemplate=ParticleSystem'Locust_Reaver.Effects.P_Reaver_Projectile_Trail'

	// explosion point light
	Begin Object Name=ExploLight0
		Radius=800.000000
		Brightness=1000.000000
		LightColor=(B=76,G=107,R=249,A=255)
	End Object

	// explosion
	Begin Object Name=ExploTemplate0
		MyDamageType=class'GDT_ReaverCannon'
		ExploLightFadeoutTime=0.3f
		ParticleEmitterTemplate=ParticleSystem'Locust_Reaver.Effects.P_Reaver_Projectile_Impact_Explo'
		ExplosionSound=SoundCue'Weapon_TorqueBow.Weapons.Crossbow_ExplosionWallCue'

		FractureMeshRadius=500.0

		bAllowPerMaterialFX=TRUE
	End Object
}


